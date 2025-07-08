#include <stdio.h>
#include "da.h"
#include "bytecode.h"
#include "asserts.h"

// -- Bytecode generator --
/*
 * Emits byte code for a stack based VM.
 *
 * -- Traversal / Byte order --
 *
 * The VM pops values from a stack and assumes the order head/tail/current.
 * That means this code generator must emit code in tail/head/current order
 * to set up the stack correctly.
 *
 * For example, (- 1 2) should be emitted as
 *
 * PUSH 2
 * PUSH 1
 * OP_SUBTRACT
 *
 * Which causes the VM to do this when it reaches the OP_SUBTRACT instruction
 *
 * first = pop()
 * second = pop()
 * push(first - second)
 *
 * -- Endianness --
 *
 * The VM pops multi-byte values and expects little endian.
 * That means the byte code generator must emit in big endian.
 * The number of bytes for a given type is also fixed for portability
 *
 * For example, a double should be emitted as this (where b8 denotes the MSB).
 *
 * PUSH b8
 * PUSH b7
 * ...
 * PUSH b1
 * OP_F64
 *
 * And the VM reads it like this
 *
 * b1 = pop()
 * ..
 */

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

static ByteCodeResult CreateGeneratorError(const char* message) {
    ByteCodeResult result = {
        .type = BYTECODE_GENERATE_ERROR,
        .as.error = {
            .message = MakeString(message),
            .token = NULL,
        }
    };
    return result;
}

static void ReportError(const char* message, void* ctx) {
    ByteCodeResult* result = (ByteCodeResult*)ctx;
    *result = CreateGeneratorError(message);
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

static void EmitBigEndian(Byte* bytes, size_t count) {
    if (!IsLittleEndian()) {
        EmitBytes(bytes, count);
        return;
    }
    for (ssize_t i = count - 1; i >= 0; i--) {
        EmitByte(bytes[i]);
    }
}

static void EmitF64(double d) {
    // VM reads little endian, but emit order is reversed
    EmitBigEndian((Byte*)&doubleDefault, DOUBLE_SIZE - sizeof(double)); // padding
    EmitBigEndian((Byte*)&d, sizeof(double));
    EmitByte(OP_F64);
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
        default:
            ReportError("Unsupported value type", ctx);
            break;
    }
}

static void EmitFunctionCall(Ast* ast, void* ctx) {
    ReportError("Function calls are not implemented", ctx);
}

static void EmitConsCell(Ast* ast, void* ctx) {
    ByteCodeResult* result = (ByteCodeResult*)ctx;
    AstCons cons = ast->as.cons;
    Ast* tail = cons.tail;
    Ast* head = cons.head;

    EmitAstHelper(tail, result);
    if (result->type == BYTECODE_GENERATE_ERROR) {
        return;
    }

    EmitAstHelper(head, result);
    if (result->type == BYTECODE_GENERATE_ERROR) {
        return;
    }

    EmitByte(OP_CONS_CELL);
}

static void EmitCons(Ast* ast, void* ctx) {
    /*
     * TODO(incomplete): add more information to the AST
     * to determine if something is a function call or a list.
     */
    bool headIsFunction = false;
    bool tailIsProperlist = false;
    if (headIsFunction && tailIsProperlist) {
        EmitFunctionCall(ast, ctx);
    } else {
        EmitConsCell(ast, ctx);
    }
}

static ByteCodeResult EmitAst(Ast* ast) {
    ByteCodeResult result = {0};
    EmitAstHelper(ast, &result);

    if (result.type == BYTECODE_GENERATE_SUCCESS) {
        result.as.success.byteCode = byteCode;
    }
    return result;
}

ByteCodeResult GenerateByteCode(Ast* ast, Allocator* allocator) {
    // TODO(memory): allocator is ignored for now
    byteCode = DA_MAKE_CAPACITY(Byte, 1337);
    return EmitAst(ast);
}

// -- Printing --

static bool IsOpCode(OpCode op) {
    return op >= OP_NIL && op <= OP_PRINT;
}
static const char* MapOpCodeToStr(OpCode op) {
    switch (op) {
        case OP_NIL: return "OP_NIL";
        case OP_TRUE: return "OP_TRUE";
        case OP_FALSE: return "OP_FALSE";
        case OP_F64: return "OP_F64";
        case OP_CONSTANT_16: return "OP_CONSTANT_16";
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

/*
 * Consumes the number of bytes for a given operation and returns
 * the offset to the next byte.
 *
 * The bytes are read in reverse order to match the read order
 * of the bytecode VM.
 */
size_t DisasOp(Byte* bytes, size_t line) {
    OpCode op = bytes[0];
    size_t offset = 0;
    printf("%3ld: ", line++);
    switch (op) {
        case OP_F64: {
            // look ahead to construct double from little endian bytes
            Byte buf[DOUBLE_SIZE];
            for (ssize_t i = 0; i < DOUBLE_SIZE; i++) {
                // -1 to look beyond the op code
                Byte b = *(bytes - i - 1);
                buf[i] = b;
            }

            // TODO(portability): add proper conversion
            Assert(DOUBLE_SIZE == sizeof(double) && IsLittleEndian(),
                "Unable to read double with evil pointer magic");
            double d = *((double*)buf);

            printf("%s: %f\n", MapOpCodeToStr(op), d);
            offset++;

            for (int i = 0; i < DOUBLE_SIZE; i++) {
                printf("%3ld: %d\n", line++, buf[i]);
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
    /*
     * Dissassemble in the VM read order in order to distinguish
     * between op codes and data.
     */
    printf("Dissassembling in VM read order. Count = %ld\n", count);
    ssize_t current = count - 1;
    for (ssize_t guard = count - 1; current >= 0 && guard >= 0; guard--) {
        ssize_t line = (ssize_t)count - current - 1;
        current -= DisasOp(&bytes[current], line);
    }
    Assertf(current == -1,
        "Unexpected final byte count. Expected %ld, but received %ld",
        count, current);
}

const char* MapByteCodeResultTypeToStr(ByteCodeResultType type) {
    switch(type) {
        case BYTECODE_GENERATE_SUCCESS: return "BYTECODE_GENERATE_SUCCESS";
        case BYTECODE_GENERATE_ERROR: return "BYTECODE_GENERATE_ERROR";
        default: return NULL;
    }
}

void PrintByteCodeResult(ByteCodeResult result) {
    if (result.type == BYTECODE_GENERATE_ERROR) {
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
