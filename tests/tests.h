#ifndef tests_h
#define tests_h

#include <stdio.h>
#include <stdlib.h>

void TokenizerTests();

#define Assertf(cond, fmt, ...) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "ASSERT FAILED %s:%d: %s: %s\n\t", \
                    __FILE__, __LINE__, __func__, #cond); \
            fprintf(stderr, fmt, __VA_ARGS__); \
            fprintf(stderr, "\n"); \
            abort(); \
        } \
    } while (0)

#define Assert(cond, msg) Assertf(cond, "%s", msg)

#endif
