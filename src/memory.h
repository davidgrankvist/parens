#ifndef memory_h
#define memory_h

// -- General purpose allocation --

void* AllocateArray(void* ptr, size_t count, size_t elementSize);
void* AllocateZeros(size_t count, size_t elementSize);
void FreeMemory(void* ptr);

#define ALLOCATE_OBJ(type) (type*)AllocateZeros(1, sizeof(type))
#define ALLOCATE_ARR(type, items, count) (type*)AllocateArray(items, count, sizeof(type))
#define ALLOCATE_NEW_ARR(type, count) ALLOCATE_ARR(type, NULL, count)
#define RESIZE_ARR(items, newCount) AllocateArray(items, newCount, sizeof(items[0]))

#endif
