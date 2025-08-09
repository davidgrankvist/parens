#ifndef bytecode_h
#define bytecode_h

#include "tokens.h"
#include "common.h"
#include "memory.h"
#include "da.h"
#include "ast.h"

// -- OP codes --

typedef enum {
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_F64, // read next 8 bytes as little endian
    OP_CONSTANT_16, // read next 2 bytes for the index
    OP_BUILTIN_FN, // read next 1 byte for the built in operator/function
    OP_GLOBAL,
    OP_SET_GLOBAL,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,
    OP_FUNCTION_CALL, // pop function definition and args from the stack
    OP_CONS_CELL,
    OP_JUMP_IF_TRUE, // pop one byte for the condition
    OP_JUMP_IF_FALSE,
    OP_JUMP,
    OP_POP,
    OP_PRINT,
    OP_ENUM_COUNT,
} OpCode;

// -- Bytecode generator --

typedef struct {
    ByteDa byteCode;
} ByteCodeGenerateSuccess;

typedef struct {
    String message;
    Token* token;
} ByteCodeGenerateError;

typedef struct {
    ResultType type;
    union {
        ByteCodeGenerateSuccess success;
        ByteCodeGenerateError error;
    } as;
} ByteCodeResult;

ByteCodeResult GenerateByteCode(Ast* ast, Allocator* allocator);

void PrintByteCodeResult(ByteCodeResult result);

double ReadDoubleFromLittleEndian8(Byte* bytes);

#endif
