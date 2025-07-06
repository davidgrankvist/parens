#include <stdlib.h>
#include "memory.h"
#include "asserts.h"

void* AllocateArray(void* ptr, size_t count,  size_t newSize) {
    void* result = realloc(ptr, count * newSize);
    Assert(result != NULL, "Failed to allocate array.");

    return result;
}

void* AllocateZeros(size_t count, size_t elementSize) {
    void* result = calloc(count, elementSize);
    Assert(result != NULL, "Failed to allocate zeros.");

    return result;
}

void FreeMemory(void* ptr) {
    free(ptr);
}
