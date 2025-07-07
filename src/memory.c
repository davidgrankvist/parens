#include <stdlib.h>
#include "memory.h"
#include "da.h"
#include "asserts.h"

// -- General purpose allocation --

void* AllocateArray(void* ptr, size_t count,  size_t newSize) {
    void* result = realloc(ptr, count * newSize);
    Assert(result != NULL, "Failed to allocate array.");

    return result;
}

void* AllocateZeros(size_t bytes) {
    void* result = calloc(1, bytes);
    Assert(result != NULL, "Failed to allocate zeros.");

    return result;
}

void FreeMemory(void* ptr) {
    free(ptr);
}

// -- Arena allocation --

typedef struct Allocator {
    void* (*Alloc)(size_t bytes, struct Allocator* self);
    void (*Reset)(struct Allocator* self);
    void (*Free)(struct Allocator* self);
    void (*Debug)(struct Allocator* self);
} Allocator;

void* AllocatorAlloc(size_t bytes, Allocator* allocator) {
    return allocator->Alloc(bytes, allocator);
}

void AllocatorReset(Allocator* allocator) {
    allocator->Reset(allocator);
}

void AllocatorFree(Allocator* allocator) {
    allocator->Free(allocator);
}

void AllocatorDebug(Allocator* allocator) {
    allocator->Debug(allocator);
}

static void AllocatorStub(Allocator* self) {
}

// -- Heap allocator --

static void* AllocHeap(size_t bytes, Allocator* self) {
    return AllocateZeros(bytes);
}

static void FreeHeap(Allocator* self) {
    *self = (Allocator){0};
    FreeMemory(self);
}

Allocator* CreateHeapAllocator() {
    Allocator* allocator = AllocateZeros(sizeof(Allocator));
    *allocator = (Allocator) {
        .Alloc = &AllocHeap,
        .Reset = &AllocatorStub,
        .Free = &FreeHeap,
        .Debug = &AllocatorStub,
    };

    return allocator;
}

// -- Bump allocator --

// ArenaPage = DA of bytes
typedef char ArenaByte;
DA_DECLARE(ArenaByte);
typedef ArenaByteDa ArenaPage;
// Arena = DA of pages
DA_DECLARE(ArenaPage);
typedef ArenaPageDa Arena;

typedef struct {
    Allocator base;
    Arena arena;
    size_t pageSize;
    size_t initialNumPages;
    size_t currentPage;
} BumpAllocator;

static void* AllocBump(size_t bytes, Allocator* self) {
    BumpAllocator* allocator = (BumpAllocator*)self;
    Assert(bytes <= allocator->pageSize, "Cannot allocate objects greater than the page size");

    // this return is for when asserts are disabled from tests
    if (bytes > allocator->pageSize) {
        return NULL;
    }

    Arena* arena = &allocator->arena;

    ArenaPage* lastPage = &allocator->arena.items[allocator->currentPage];
    size_t numAvailable = lastPage->capacity - lastPage->count;
    size_t bytesAvailable = numAvailable * sizeof(ArenaByte);

    if (bytes > bytesAvailable) {
        ArenaPage newPage = DA_MAKE_CAPACITY(ArenaByte, allocator->pageSize);

        DA_APPEND_GROW_ONE(arena, newPage);
        allocator->currentPage++;

        lastPage = &arena->items[allocator->currentPage];
    }

    void* bytesStart = &lastPage->items[lastPage->count];
    lastPage->count += bytes;
    return bytesStart;
}

static void ResetBump(Allocator* self) {
    BumpAllocator* allocator = (BumpAllocator*)self;
    Arena* arena = &allocator->arena;

    // free the extra pages
    for (int i = allocator->initialNumPages; i < arena->count; i++) {
        DA_FREE(&arena->items[i]);
    }
    // shrink capacity
    if (arena->count > allocator->initialNumPages) {
        arena->items = AllocateArray(arena->items, allocator->initialNumPages, sizeof(ArenaPage));
    }

    // reset counters
    arena->count = allocator->initialNumPages;
    arena->capacity = allocator->initialNumPages;
    for (int i = 0; i < allocator->initialNumPages; i++) {
        arena->items[i].count = 0;
    }
    allocator->currentPage = 0;
}

static void InitArenaBump(BumpAllocator* allocator) {
    Arena arena = DA_MAKE_CAPACITY(ArenaPage, allocator->initialNumPages);
    for (int i = 0; i < allocator->initialNumPages; i++) {
        ArenaPage page = DA_MAKE_CAPACITY(ArenaByte, allocator->pageSize);
        // avoid DA_APPEND utils to make sure there's no resizing
        arena.items[arena.count++] = page;
    }

    allocator->arena = arena;
}

static void FreeBump(Allocator* self) {
    BumpAllocator* allocator = (BumpAllocator*)self;
    Arena* arena = &allocator->arena;

    for (int i = 0; i < arena->capacity; i++) {
        DA_FREE(&arena->items[i]);
    }
    DA_FREE(arena);

    *allocator = (BumpAllocator) {0};
    FreeMemory(allocator);
}

Allocator* CreateBumpAllocator(size_t pageSize, size_t initialNumPages) {
    BumpAllocator* allocator = AllocateZeros(sizeof(BumpAllocator));
    *allocator = (BumpAllocator) {
        .base.Alloc = &AllocBump,
        .base.Reset = &ResetBump,
        .base.Free = &FreeBump,
        .base.Debug = &AllocatorStub,
        .pageSize = pageSize,
        .initialNumPages = initialNumPages,
        .currentPage = 0,
    };
    InitArenaBump(allocator);

    return (Allocator*)allocator;
}
