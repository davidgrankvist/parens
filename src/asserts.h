#ifndef asserts_h
#define asserts_h

#include <stdbool.h>

#ifdef IS_RUNNING_TESTS
void SetAssertAbortFromTest(bool enabled);
void SetAssertEnabledFromTest(bool enabled);
#endif

// Helper function. Use the macros to capture file name, etc.
void AssertFn(bool b,
    const char* file, int line,
    const char* func, const char* cond,
    const char* format, ...);

#define Assertf(b, format, ...) \
    AssertFn((b), __FILE__, __LINE__, __func__, #b,  format,  __VA_ARGS__)
#define Assert(b, msg) Assertf(b, "%s",  msg)

#define AssertFailf(format, ...) \
    AssertFn(false, __FILE__, __LINE__, __func__, "false", format, __VA_ARGS__)
#define AssertFail(msg) AssertFailf("%s",  msg)

#endif
