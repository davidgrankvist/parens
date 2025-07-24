#ifndef bytecode_h
#define bytecode_h

#include "tokens.h"
#include "stringz.h"
#include "memory.h"
#include "da.h"
#include "ast.h"

// -- OP codes --

typedef enum {
    // values
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_F64, // read next 8 bytes as little endian
    OP_CONSTANT_16, // read next 2 bytes for the index
    OP_BUILTIN_FN, // read next 1 byte for the built in operator/function
    // arithmetic operators
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,
    // arbirary functions
    OP_FUNCTION_CALL, // pop function definition and args from the stack
    // lists
    OP_CONS_CELL,
    // control flow
    OP_JUMP_IF_TRUE, // pop one byte for the condition
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
