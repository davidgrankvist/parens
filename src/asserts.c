
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "asserts.h"

static bool shouldAbort = true;
static bool shouldAssert = true;

#ifdef IS_RUNNING_TESTS
void SetAssertAbortFromTest(bool enabled) {
    shouldAbort = enabled;
}
void SetAssertEnabledFromTest(bool enabled) {
    shouldAssert = enabled;
}
#endif

void AssertFunction(bool b, const char* file, int line, const char* func, const char* cond, const char* format, ...) {
    if (b || !shouldAssert) {
        return;
    }

    if (shouldAbort) {
        fflush(stdout);
        fflush(stderr);
    }

    va_list args;
    va_start(args, format);

    fprintf(stderr, "ASSERT FAILED %s:%d: %s: %s\n\t", file, line, func, cond);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

    va_end(args);

    if (shouldAbort) {
        abort();
    }
}
