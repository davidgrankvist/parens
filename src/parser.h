#ifndef parser_h
#define parser_h

#include "ast.h"

typedef struct {
    Ast* ast;
} ParseSuccess;

typedef struct {
    String message;
    Token* token;
} ParseError;

typedef struct {
    ResultType type;
    union {
        ParseSuccess success;
        ParseError error;
    } as;
} ParseResult;

ParseResult ParseTokens(TokenDa ts, Allocator* allocator);

void PrintParseResult(ParseResult result);

#endif
