#include "tests.h"

int main() {
    DynamicArrayTests();
    BumpAllocatorTests();
    TokenizerTests();
    ParserTests();
    BytecodeGeneratorTests();
    printf("--------------\n");
    printf("SUCCESS\n");
}
