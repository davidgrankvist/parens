#include "tests.h"

int main() {
    DynamicArrayTests();
    BumpAllocatorTests();
    TokenizerTests();
    ParserTests();
    BytecodeGeneratorTests();
    VmTests();
    printf("--------------\n");
    printf("SUCCESS\n");
}
