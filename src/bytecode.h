#ifndef bytecode_h
#define bytecode_h

#include "tokens.h"
#include "stringz.h"
#include "memory.h"
#include "da.h"
#include "ast.h"

// -- OP codes --

typedef enum {
    OP_NIL,
    // add more OP codes here
} OpCode;

typedef char OpByte;
DA_DECLARE(OpByte);

const char* MapOpCodeToStr(OpCode code);

// -- Bytecode generator --

typedef enum {
    BYTECODE_GENERATE_SUCCESS,
    BYTECODE_GENERATE_ERROR,
} ByteCodeResultType;

typedef struct {
    OpByteDa bytes;
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
