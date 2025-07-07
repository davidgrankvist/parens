#ifndef parser_h
#define parser_h

#include "ast.h"
#include "memory.h"

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

ParseResult ParseTokens(TokenDa ts, Allocator* allocator);

void PrintParseResult(ParseResult result);
void PrintAst(Ast* ast);
const char* MapParseResultTypeToStr(ParseResultType type);

#endif
