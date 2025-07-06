#include <stdlib.h>
#include "memory.h"
#include "asserts.h"
#include "common.h"

// -- General purpose allocation --

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

// -- Arena allocation --

typedef struct Allocator {
    void* (*AllocateBytes)(size_t bytes, struct Allocator* self);
    void (*Reset)(struct Allocator* self);
    void (*Free)(struct Allocator* self);
    void (*Debug)(struct Allocator* self);
} Allocator;

void* ArenaAllocate(size_t bytes, Allocator* allocator) {
    return allocator->AllocateBytes(bytes, allocator);
}

void ArenaReset(Allocator* allocator) {
    allocator->Reset(allocator);
}

void ArenaFree(Allocator* allocator) {
    allocator->Free(allocator);
}

void ArenaDebug(Allocator* allocator) {
    allocator->Debug(allocator);
}

// -- Bump allocator --

// Page = DA of bytes
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

static void* AllocateBytesBump(size_t bytes, Allocator* self) {
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

        DA_APPEND_RESIZE(arena, newPage, arena->capacity + 1);
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
        DA_APPEND(&arena, page);
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

static void DebugBump(Allocator* self) {
    BumpAllocator* allocator = (BumpAllocator*)self;
    // inspect things here..
}

Allocator* CreateBumpAllocator(size_t pageSize, size_t initialNumPages) {
    BumpAllocator* allocator = ALLOCATE_OBJ(BumpAllocator);
    *allocator = (BumpAllocator) {
        .base.AllocateBytes = &AllocateBytesBump,
        .base.Reset = &ResetBump,
        .base.Free = &FreeBump,
        .base.Debug = &DebugBump,
        .pageSize = pageSize,
        .initialNumPages = initialNumPages,
        .currentPage = 0,
    };
    InitArenaBump(allocator);

    return (Allocator*)allocator;
}
