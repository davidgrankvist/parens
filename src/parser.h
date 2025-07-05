#ifndef parser_h
#define parser_h

#include "ast.h"

typedef enum {
    PARSE_SUCCESS,
    PARSE_ERROR,
} ParseResultType;

typedef struct {
    Ast* ast;
} ParseSuccess;

typedef struct {
    String message;
    Token* token;
} ParseError;

typedef struct {
    ParseResultType type;
    union {
        ParseSuccess success;
        ParseError error;
    } as;
} ParseResult;

ParseResult ParseTokens(TokenDa tokens);

void PrintParseResult(ParseResult result);

#endif
