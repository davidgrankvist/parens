#ifndef memory_h
#define memory_h

// -- General purpose allocation --

/*
 * Creates or resizes an array. Does not zero memory.
 * Pass in NULL to create a new array.
 */
void* AllocateArray(void* ptr, size_t count, size_t elementSize);
void* AllocateZeros(size_t bytes);
void FreeMemory(void* ptr);

// -- Allocators --

typedef struct Allocator Allocator;

void* AllocatorAlloc(size_t bytes, Allocator* allocator);

// Reset allocator state, for example memory arenas. The allocator can be re-used.
void AllocatorReset(Allocator* allocator);
// Free allocator state. The allocator can not be re-used.
void AllocatorFree(Allocator* allocator);
// Helper to inspect internal state when debugging.
void AllocatorDebug(Allocator* allocator);

/*
 * HEAP ALLOCATOR
 *
 * General purpose allocator. Uses regular stdlib calls.
 * The caller is responsible for freeing.
 *
 * Does not implement AllocatorReset.
 */
Allocator* CreateHeapAllocator();

/*
 * BUMP ALLOCATOR
 *
 * This allocator uses an append-only strategy. Bytes are grouped into
 * pages. When a page is full, the next one is used. When all pages are full,
 * the arena grows by one page.
 *
 * There is some fragmentation, because all bytes of an object are always
 * allocated within the same page. If there is not enough room in
 * the current page to allocate N bytes, then all of those N bytes end up
 * in the next page, leaving the end of the previos page unused.
 *
 * When resetting the allocator, it returns to having the initial
 * number of pages and any extra pages that were created.
 */
Allocator* CreateBumpAllocator(size_t pageSize, size_t initialNumPages);

#endif
