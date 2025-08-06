#ifndef tokens_h
#define tokens_h

#include "common.h"
#define DA_WITH_ONLY_DECLARE
#include "da.h"
#undef DA_WITH_ONLY_DECLARE

typedef enum {
    TOKEN_PAREN_START,
    TOKEN_PAREN_END,
    TOKEN_CONS,
    TOKEN_NIL,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_SYMBOL,
    TOKEN_QUOTE,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PRINT,
    TOKEN_SET,
    TOKEN_FUN,
    TOKEN_DEFUN,
    /*
     * This marker is added to signal the end of the
     * token stream.
     */
    TOKEN_EOF,
    /*
     * Not an actual token, but makes error reporting
     * simple when tokenizing.
     */
    TOKEN_ERROR,
} TokenType;

typedef struct {
    TokenType type;
    String str;
    int line;
    int col;
} Token;

DA_DECLARE(Token);

const char* MapTokenTypeToStr(TokenType type);
void PrintTokenType(TokenType type);
void PrintToken(Token token);

void InitTokenizer(char* str);
Token ConsumeToken();

#endif
