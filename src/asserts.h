#ifndef asserts_h
#define asserts_h

#include <stdbool.h>

// Helper function. Use the macros to capture file name, etc.
void AssertFunction(bool b,
    const char* file, int line,
    const char* func, const char* cond,
    const char* format, ...);

#define Assertf(b, format, ...) \
    AssertFunction((b), __FILE__, __LINE__, __func__, #b,  format,  __VA_ARGS__)
#define Assert(b, msg) Assertf(b, "%s",  msg)

#define AssertFailf(format, ...) \
    AssertFunction(false, __FILE__, __LINE__, __func__, "false", format, __VA_ARGS__)
#define AssertFail(msg) AssertFailf("%s",  msg)

#ifdef IS_RUNNING_TESTS
void SetAssertAbortFromTest(bool enabled);
void SetAssertEnabledFromTest(bool enabled);
#endif

#endif
