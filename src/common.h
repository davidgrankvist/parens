#ifndef common_h
#define common_h

#include <stddef.h>
#include <stdbool.h>

// -- Strings --

typedef struct {
    const char* start;
    size_t length;
} String;

bool StringEquals(String s1, String s2);
String MakeString(const char* str);

// -- Tokens --

typedef enum {
    TOKEN_PAREN_START,
    TOKEN_PAREN_END,
    TOKEN_CONS,
    TOKEN_NIL,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_SYMBOL,
    TOKEN_EOF,
    TOKEN_ERROR,
} TokenType;

typedef struct {
    TokenType type;
    String str;
    int line;
    int col;
} Token;

void PrintTokenType(TokenType type);
void PrintToken(Token token);

#endif
