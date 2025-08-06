#include <stdio.h>
#include "da.h"
#include "bytecode.h"
#include "asserts.h"

// -- Bytecode generator --

ByteDa byteCode = {0};

static double doubleDefault = 0;
#define DOUBLE_SIZE 8

static void EmitAtom(Ast* ast, void* ctx);
static void EmitCons(Ast* ast, void* ctx);

static AstVisitor emitterVisitor = {
    .VisitAtom = &EmitAtom,
    .VisitCons = &EmitCons,
};

static void EmitAstHelper(Ast* ast, void* ctx) {
    VisitAst(ast, &emitterVisitor, ctx);
}

static ByteCodeResult CreateGeneratorError(const char* message, Token* token) {
    ByteCodeResult result = {
        .type = RESULT_ERROR,
        .as.error = {
            .message = MakeString(message),
            .token = token,
        }
    };
    return result;
}

static void ReportError(const char* message, Ast* ast, void* ctx) {
    ByteCodeResult* result = (ByteCodeResult*)ctx;
    *result = CreateGeneratorError(message, ast->token);
}

static bool IsLittleEndian() {
    unsigned short sh = 1;
    Byte* bp = (Byte*)&sh;
    return bp[0] == 1;
}


static void EmitByte(Byte byte) {
    DA_APPEND(&byteCode, byte);
}

static void EmitBytes(Byte* bytes, size_t count) {
    for (size_t i = 0; i < count; i++) {
        EmitByte(bytes[i]);
    }
}

static void EmitLittleEndian(Byte* bytes, size_t count) {
    if (IsLittleEndian()) {
        EmitBytes(bytes, count);
        return;
    }

    for (ssize_t i = count - 1; i >= 0; i--) {
        EmitByte(bytes[i]);
    }
}

static void EmitF64(double d) {
    EmitByte(OP_F64);
    EmitLittleEndian((Byte*)&d, sizeof(double));
    EmitLittleEndian((Byte*)&doubleDefault, DOUBLE_SIZE - sizeof(double)); // padding
}

static void EmitOperator(OperatorType operator, Ast* ast, void* ctx) {
    switch(operator) {
        case OPERATOR_ADD:
            EmitByte(OP_ADD);
            break;
        case OPERATOR_SUBTRACT:
            EmitByte(OP_SUBTRACT);
            break;
        case OPERATOR_MULTIPLY:
            EmitByte(OP_MULTIPLY);
            break;
        case OPERATOR_DIVIDE:
            EmitByte(OP_DIVIDE);
            break;
        case OPERATOR_PRINT:
            EmitByte(OP_PRINT);
            break;
        default:
            ReportError("Unsupported operator type", ast, ctx);
            break;
    }
}

static void EmitAtom(Ast* ast, void* ctx) {
    Value val = ast->as.atom.value;
    switch (val.type) {
        case VALUE_NIL:
            EmitByte(OP_NIL);
            break;
        case VALUE_F64:
            EmitF64(val.as.f64);
            break;
        case VALUE_OPERATOR:
            EmitByte(OP_BUILTIN_FN); // indicate that the operator is passed as a value
            EmitOperator(val.as.operator, ast, ctx);
            return;
        default:
            ReportError("Unsupported value type", ast, ctx);
            break;
    }
}

static bool IsNilAtom(Ast* ast) {
    return ast->type == AST_ATOM && ast->as.atom.value.type == VALUE_NIL;
}

// Evaluate each list element individually, tail first. Skip the terminating nil.
static void EmitProperListElements(Ast* ast, void* ctx) {
    if (IsNilAtom(ast)) {
        return;
    } else if (ast->type == AST_ATOM) {
        ReportError("A proper list was unexpectedly terminated by a non-nil atom.", ast, ctx);
        return;
    }

    ByteCodeResult* result = (ByteCodeResult*)ctx;
    AstCons cons = ast->as.cons;
    Ast* tail = cons.tail;
    Ast* head = cons.head;

    EmitProperListElements(tail, result);
    if (result->type == RESULT_ERROR) {
        return;
    }

    EmitAstHelper(head, result);
    if (result->type == RESULT_ERROR) {
        return;
    }
}

static void EmitFunctionCall(Ast* ast, void* ctx) {
    EmitProperListElements(ast, ctx);
    ByteCodeResult* result = (ByteCodeResult*)ctx;
    if (result->type == RESULT_ERROR) {
        return;
    }

    bool isBuiltin = byteCode.count >= 2 && byteCode.items[byteCode.count - 2] == OP_BUILTIN_FN;
    if (isBuiltin) {
        /*
         * Instead of pushing the operator/builtin to the stack, call it directly.
         * For example the atom + is emitted as OP_BUILTIN_FN OP_ADD
         * but in a direct invocation we can use OP_ADD right away.
         */
        byteCode.items[byteCode.count - 2] = byteCode.items[byteCode.count - 1];
        byteCode.count--;
    } else {
        // The expression may or may not evaluate to a callable. Defer the check to runtime.
        // TODO(optimize): check for known callables such as resolved symbols at compile time
        EmitByte(OP_FUNCTION_CALL);
    }
}

static void EmitConsCell(Ast* ast, void* ctx) {
    if (ast->type == AST_ATOM) {
        EmitAstHelper(ast, ctx);
        return;
    }

    ByteCodeResult* result = (ByteCodeResult*)ctx;
    AstCons cons = ast->as.cons;
    Ast* tail = cons.tail;
    Ast* head = cons.head;

    EmitConsCell(tail, result);
    if (result->type == RESULT_ERROR) {
        return;
    }

    EmitAstHelper(head, result);
    if (result->type == RESULT_ERROR) {
        return;
    }

    EmitByte(OP_CONS_CELL);
}

static void EmitCons(Ast* ast, void* ctx) {
    if (ast->isQuoted) {
        EmitConsCell(ast, ctx);
    } else {
        EmitFunctionCall(ast, ctx);
    }
}

static ByteCodeResult EmitAst(Ast* ast) {
    ByteCodeResult result = {0};
    EmitAstHelper(ast, &result);

    if (result.type == RESULT_SUCCESS) {
        result.as.success.byteCode = byteCode;
    }
    return result;
}

ByteCodeResult GenerateByteCode(Ast* ast, Allocator* allocator) {
    // TODO(memory): allocator is ignored for now
    byteCode = DA_MAKE_CAPACITY(Byte, 1337);
    return EmitAst(ast);
}


double ReadDoubleFromLittleEndian8(Byte* bytes) {
    // TODO(portability): add proper conversion
    Assert(DOUBLE_SIZE == sizeof(double) && IsLittleEndian(),
           "Unable to read double with evil pointer magic");
    double d = *((double*)bytes);

    return d;
}

// -- Printing --

static bool IsOpCode(OpCode op) {
    return op >= OP_NIL && op < OP_ENUM_COUNT;
}
static const char* MapOpCodeToStr(OpCode op) {
    switch (op) {
        case OP_NIL: return "OP_NIL";
        case OP_TRUE: return "OP_TRUE";
        case OP_FALSE: return "OP_FALSE";
        case OP_F64: return "OP_F64";
        case OP_CONSTANT_16: return "OP_CONSTANT_16";
        case OP_BUILTIN_FN: return "OP_BUILTIN_FN";
        case OP_ADD: return "OP_ADD";
        case OP_SUBTRACT: return "OP_SUBTRACT";
        case OP_MULTIPLY: return "OP_MULTIPLY";
        case OP_DIVIDE: return "OP_DIVIDE";
        case OP_NEGATE: return "OP_NEGATE";
        case OP_FUNCTION_CALL: return "OP_FUNCTION_CALL";
        case OP_CONS_CELL: return "OP_CONS_CELL";
        case OP_JUMP_IF_TRUE: return "OP_JUMP_IF_TRUE";
        case OP_JUMP_IF_FALSE: return "OP_JUMP_IF_FALSE";
        case OP_JUMP: return "OP_JUMP";
        case OP_POP: return "OP_POP";
        case OP_PRINT: return "OP_PRINT";
        default: break;
    }
    Assertf(!IsOpCode(op), "Missing string for op code %d", op);
    return NULL;
}

size_t DisasOp(Byte* bytes, size_t line) {
    OpCode op = bytes[0];
    size_t offset = 0;
    printf("%3ld: ", line++);
    switch (op) {
        case OP_F64: {
            // look ahead to read the resulting f64 value
            double d = ReadDoubleFromLittleEndian8(&bytes[offset + 1]);

            printf("%s: %f\n", MapOpCodeToStr(op), d);
            offset++;

            for (int i = 0; i < DOUBLE_SIZE; i++) {
                printf("%3ld: %d\n", line++, bytes[offset + i]);
            }

            offset += DOUBLE_SIZE;
            break;
        }
        default: {
            if (IsOpCode(op)) {
                printf("%s\n", MapOpCodeToStr(op));
            } else {
                printf("%d\n", op);
            }
            offset++;
            break;
        }
    }

    return offset;
}

static void DisasByteCode(Byte* bytes, size_t count) {
     printf("Dissassembling bytecode. Count = %ld\n", count);
     size_t current = 0;
     for (size_t guard = 0; current < count && guard < count; guard++) {
         current += DisasOp(&bytes[current], current);
     }
     Assertf(current == count,
             "Unexpected final byte count. Expected %ld, but received %ld",
             count, current);
}

void PrintByteCodeResult(ByteCodeResult result) {
    if (result.type == RESULT_ERROR) {
        ByteCodeGenerateError error = result.as.error;
        fprintf(stderr, "Byte code generator error: ");
        PrintStringErr(error.message);
        fprintf(stderr, "\n");

        if (error.token != NULL) {
            PrintToken(*(error.token));
        }
        return;
    }
    printf("Generated bytecode successfully\n");
    ByteDa code = result.as.success.byteCode;

    DisasByteCode(code.items, code.count);
}
