#ifndef common_h
#define common_h

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

typedef enum {
    RESULT_SUCCESS,
    RESULT_ERROR,
} ResultType;

const char* MapResultTypeToStr(ResultType result);

#endif
