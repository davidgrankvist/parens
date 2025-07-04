#include <string.h>
#include "common.h"

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

