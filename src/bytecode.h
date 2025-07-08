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
 * Byte codes are either standalone or signal how many values to
 * pop and/or push.
 *
 * Little endian is used for multi-byte values.
 *
 * Primitive values like nil/true/false are stored as only the op code.
 * No pop is required. Specific types like F64 require multiple bytes
 * to be popped. Constants require bytes to be popped and those bytes
 * refer to an index into a constants table.
 *
 * Arithmetic operators pop their operands, evaluate and push the result.
 *
 * Function calls are performed by first popping the argument count,
 * and then popping the argument runtime values.
 *
 * The cons cell op code indicates that there are head/tail values on the stack.
 * Those are popped and a new cons cell value with that head/tail is pushed.
 *
 * Conditional jump op codes are followed by a truthy/falsey value.
 * Jumping is done by popping a byte code offset and updating
 * the program counter.
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
    OP_F64, // pop next 8 bytes
    OP_CONSTANT_16, // pop next 2 bytes for the index
    // arithmetic operators
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,
    // functions
    OP_FUNCTION_CALL, // pop next byte for argc, then pop that amount
    // lists
    OP_CONS_CELL,
    // control flow
    OP_JUMP_IF_TRUE, // pop next byte for the condition
    OP_JUMP_IF_FALSE,
    OP_JUMP,
    // explicit VM stack pop
    OP_POP,
    // built in functions
    OP_PRINT,
} OpCode;

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
