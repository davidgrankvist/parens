#ifndef common_h
#define common_h

#include <stddef.h>
#include <stdbool.h>
#include "asserts.h"

// -- Dynamic arrays --
// TODO(incomplete): Make dynamic

#define DA_DECLARE(type) \
    typedef struct { \
        type* items; \
        size_t count; \
        size_t capacity; \
    } type##Da

#define DA_DEFAULT_CAPACITY 0

#define DA_MAKE_CAPACITY(type, capacity) \
    (type##Da){ NULL, 0, capacity }

#define DA_MAKE(type) \
    DA_MAKE(type, DA_DEFAULT_CAPACITY)

#define DA_APPEND(da, item) \
    do { \
        if ((da)->count + 1 > (da)->capacity) { \
            AssertFail("Resizing is not implemented"); \
        } \
        (da)->items[(da)->count++] = item; \
    } while(0)

// -- Strings --

typedef struct {
    const char* start;
    size_t length;
} String;

bool StringEquals(String s1, String s2);
String MakeString(const char* str);

void PrintString(String s);
void PrintStringErr(String s);

double ParseStringAsDouble(String str);

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

DA_DECLARE(Token);

const char* MapTokenTypeToStr(TokenType type);
void PrintTokenType(TokenType type);
void PrintToken(Token token);

#endif
