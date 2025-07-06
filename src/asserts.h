#ifndef asserts_h
#define asserts_h

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

bool ShouldAssertAbort();
bool ShouldAssert();

#ifdef IS_RUNNING_TESTS
void SetAssertAbortFromTest(bool enabled);
void SetAssertEnabledFromTest(bool enabled);
#endif

#define Assertf(cond, fmt, ...) \
    do { \
        if (!(cond)) { \
            if (!ShouldAssert()) { \
                break; \
            } \
            fprintf(stderr, "ASSERT FAILED %s:%d: %s: %s\n\t", \
                    __FILE__, __LINE__, __func__, #cond); \
            fprintf(stderr, fmt, __VA_ARGS__); \
            fprintf(stderr, "\n"); \
            if (ShouldAssertAbort()) { \
                abort(); \
            } \
        } \
    } while (0)

#define Assert(cond, msg) Assertf(cond, "%s", msg)
#define AssertFailf(msg) Assertf(0, "%s", msg)
#define AssertFail(msg) AssertFailf(msg)

#endif
