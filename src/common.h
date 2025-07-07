#ifndef common_h
#define common_h

#include <stddef.h>
#include <stdbool.h>
#include "memory.h"

// -- Dynamic arrays --
/*
 * Common dynamic arrays utilites. Uses macros to operate
 * on generic types.
 *
 * The make/append calls use standard heap allocation.
 */

// Declares a DA type with the suffix "Da".
#define DA_DECLARE(type) \
    typedef struct { \
        type* items; \
        size_t count; \
        size_t capacity; \
    } type##Da

// Capacity when calling DA_MAKE_DEFAULT
#define DA_DEFAULT_CAPACITY 8
// Capacity multiplier when calling DA_APPEND
#define DA_GROW_FACTOR 2

#define DA_MAKE_CAPACITY(type, cap) \
    (type##Da){ \
        .items = AllocateArray(NULL, cap, sizeof(type)), \
        .count = 0,  \
        .capacity = cap \
    }

// Create a DA with the default capacity
#define DA_MAKE_DEFAULT(type) DA_MAKE_CAPACITY(type, DA_DEFAULT_CAPACITY)

/*
 * Helper for other append macros. Do not use directly.
 *
 * Appends an item and grows to the given capacity if needed.
 * The caller is expected to pass in a greater new capacity.
 */
#define DA_APPEND_RESIZE(da, item, newCap) \
    do { \
        if ((da)->count + 1 > (da)->capacity) { \
            (da)->capacity = newCap; \
            (da)->items = AllocateArray( \
                (da)->items, (da)->capacity, sizeof((da)->items[0]) \
            ); \
        } \
        (da)->items[(da)->count++] = item; \
    } while(0)

// Appends an item, growing the capacity by DA_GROW_FACTOR if needed.
#define DA_APPEND(da, item) \
    DA_APPEND_RESIZE(da, item, (da)->capacity * DA_GROW_FACTOR)

// Appends an item, growing the capacity by 1 if needed.
#define DA_APPEND_GROW_ONE(da, item) \
    DA_APPEND_RESIZE(da, item, (da)->capacity + 1)

/*
 * Removes an item at a given index. The original
 * item order is not preserved.
 */
#define DA_REMOVE_UNORDERED(da, index) \
    do { \
        (da)->items[index] = (da)->items[--(da)->count]; \
    } while(0)

// Frees the DA. It can not be re-used.
#define DA_FREE(da) \
    do { \
        (da)->count = 0; \
        (da)->capacity = 0; \
        FreeMemory((da)->items); \
        (da)->items = NULL; \
    } while(0);

// -- Strings --
/*
 * Custom string type with an attached length.
 *
 * This type is useful for making substrings without
 * copying the original data.
 */

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

#endif
