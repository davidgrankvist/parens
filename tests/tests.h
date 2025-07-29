#ifndef tests_h
#define tests_h

#include <stdio.h>
#include <stdlib.h>
#include "asserts.h"

void DynamicArrayTests();
void BumpAllocatorTests();
void TokenizerTests();
void ParserTests();
void BytecodeGeneratorTests();
void VmTests();

#define PRINT_TEST_TITLE() printf("--- %s ---\n", __func__)
#define PRINT_TEST_FAILURE() printf("=== TEST FAILURE ===\n")

#endif
