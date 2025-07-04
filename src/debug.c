#include <stdio.h>
#include "common.h"

static const char* MapTokenTypeToStr(TokenType type) {
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

void PrintToken(Token token) {
    if (token.type == TOKEN_ERROR) {
        fprintf(stderr, "Error at %d,%d - ", token.line, token.col);
        fwrite(token.str.start, 1, token.str.length, stderr);
        fprintf(stderr, "\n");
        return;
    }
    const char* typeStr = MapTokenTypeToStr(token.type);
    printf("Token %s at %d,%d", typeStr, token.line, token.col);
    if (token.type != TOKEN_EOF) {
        printf(" - ");
        fwrite(token.str.start, 1, token.str.length, stdout);
    }
    printf("\n");
}
