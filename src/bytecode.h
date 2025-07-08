#ifndef bytecode_h
#define bytecode_h

#include "tokens.h"
#include "stringz.h"
#include "memory.h"
#include "da.h"
#include "ast.h"

// -- OP codes --

/*
 * Single byte instructions to run on a stack based bytecode VM.
 *
 * The value op codes are followed by values that are pushed as runtime values
 * to the stack. The constant op code use an index to look up the value to push.
 *
 * Arithmetic operators pop their operands, evaluate and push the result.
 *
 * Function calls are performed by first popping the argument count,
 * and then popping the argument runtime values.
 *
 * The cons cell op code indicates that there are head/tail values on the stack.
 * Those are popped and a new cons cell value with that head/tail is pushed.
 *
 * Conditional jump op codes are followed by a truthy/falsey value. Jumping is done by popping
 * a byte code offset and updating the program counter.
 *
 * The pop op code discards a value from the stack.
 *
 * Print has a dedicated operation for now to make development easier.
 */
typedef enum {
    // values
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_F64,
    OP_CONSTANT_16, // index to constants table for other value types
    // arithmetic operators
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,
    // functions
    OP_FUNCTION_CALL,
    // lists
    OP_CONS_CELL,
    // control flow
    OP_JUMP_IF_TRUE,
    OP_JUMP_IF_FALSE,
    OP_JUMP,
    // explicit VM stack pop
    OP_POP,
    // built in functions
    OP_PRINT,
} OpCode;

const char* MapOpCodeToStr(OpCode code);

// -- Bytecode generator --

typedef enum {
    BYTECODE_GENERATE_SUCCESS,
    BYTECODE_GENERATE_ERROR,
} ByteCodeResultType;

typedef struct {
    ByteDa byteCode;
} ByteCodeGenerateSuccess;

typedef struct {
    String message;
    Token* token;
} ByteCodeGenerateError;

typedef struct {
    ByteCodeResultType type;
    union {
        ByteCodeGenerateSuccess success;
        ByteCodeGenerateError error;
    } as;
} ByteCodeResult;

ByteCodeResult GenerateByteCode(Ast* ast, Allocator* allocator);

const char* MapByteCodeResultTypeToStr(ByteCodeResultType type);
void PrintByteCodeResult(ByteCodeResult result);

#endif
