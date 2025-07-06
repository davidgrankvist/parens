#include "tests.h"

int main() {
    DynamicArrayTests();
    BumpAllocatorTests();
    TokenizerTests();
    ParserTests();
    printf("--------------\n");
    printf("SUCCESS\n");
}
