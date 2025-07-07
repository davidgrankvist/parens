#include <string.h>
#include "common.h"
#include "asserts.h"

bool StringEquals(String s1, String s2) {
    return s1.length == s2.length
        && (s1.start == s2.start || memcmp(s1.start, s2.start, s1.length) == 0);
}

String MakeString(const char* str) {
    String s = {
        .start = str,
        .length = strlen(str),
    };
    return s;
}

static bool IsDigit(char c) {
    return c >= '0' && c <= '9';
}

static int ToDigit(char c) {
    return c - '0';
}

// use custom parsing since the stdlib relies on C strings
double ParseStringAsDouble(String str) {
    double num = 0;
    int i = 0;
    // integer part
    for (; i < str.length && IsDigit(str.start[i]); i++) {
        int digit = ToDigit(str.start[i]);
        num = num * 10 + digit;
    }

    // decimal part
    if (i + 1 < str.length && str.start[i] == '.') {
       i++;
       double denom = 10;
       for (; i < str.length && IsDigit(str.start[i]); i++) {
           int digit = ToDigit(str.start[i]);
           num += digit / denom;
           denom *= 10;
       }
    }

    /*
     * abort() here is fine as in the parsing stage we already know
     * that we're parsing a valid token.
     */
    Assert(i == str.length, "Failed to parse as double. "
            "The string contains invalid characters.");

    return num;
}

