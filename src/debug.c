#include <stdio.h>
#include "common.h"
#include "parser.h"

const char* MapTokenTypeToStr(TokenType type) {
    switch(type) {
        case TOKEN_PAREN_START: return "TOKEN_PAREN_START";
        case TOKEN_PAREN_END: return "TOKEN_PAREN_END";
        case TOKEN_CONS: return "TOKEN_CONS";
        case TOKEN_NIL: return "TOKEN_NIL";
        case TOKEN_NUMBER: return "TOKEN_NUMBER";
        case TOKEN_STRING: return "TOKEN_STRING";
        case TOKEN_SYMBOL: return "TOKEN_SYMBOL";
        case TOKEN_EOF: return "TOKEN_EOF";
        case TOKEN_ERROR: return "TOKEN_ERROR";
        default: return NULL;
    }
}

void PrintTokenType(TokenType type) {
    const char* typeStr = MapTokenTypeToStr(type);
    printf("%s", typeStr);
}

void PrintString(String s) {
    fwrite(s.start, 1, s.length, stdout);
}

void PrintStringErr(String s) {
    fwrite(s.start, 1, s.length, stderr);
}

void PrintToken(Token token) {
    if (token.type == TOKEN_ERROR) {
        fprintf(stderr, "Error at %d,%d - ", token.line, token.col);
        PrintStringErr(token.str);
        fprintf(stderr, "\n");
        return;
    }
    const char* typeStr = MapTokenTypeToStr(token.type);
    printf("Token %s at %d,%d", typeStr, token.line, token.col);
    if (token.type != TOKEN_EOF) {
        printf(" - ");
        PrintString(token.str);
    }
    printf("\n");
}

void PrintParseResult(ParseResult result) {
    if (result.type == PARSE_ERROR) {
        ParseError error = result.as.error;
        fprintf(stderr, "Parse error: ");
        PrintStringErr(error.message);
        fprintf(stderr, "\n");

        if (error.token != NULL) {
            PrintToken(*(error.token));
        }
        return;
    }
    printf("Parsed AST successfully\n");
}
