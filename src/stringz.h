/*
 * Custom string type with an attached length.
 *
 * This type is useful for making substrings without
 * copying the original data.
 */

#ifndef stringz_h
#define stringz_h

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    const char* start;
    size_t length;
} String;

bool StringEquals(String s1, String s2);
String MakeString(const char* str);
double ParseStringAsDouble(String str);

void PrintString(String s);
void PrintStringErr(String s);

#endif
