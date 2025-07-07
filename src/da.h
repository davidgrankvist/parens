/*
 * Common dynamic arrays utilites. Uses macros to operate
 * on generic types.
 *
 * The make/append calls use standard heap allocation.
 *
 * Define the macro DA_WITH_ONLY_DECLARE if you only need
 * the DA_DECLARE macro for type declarations.
 */

#ifndef da_h
#define da_h

#ifndef DA_WITH_ONLY_DECLARE
#include "memory.h"
#endif

// Declares a DA type with the suffix "Da".
#define DA_DECLARE(type) \
    typedef struct { \
        type* items; \
        size_t count; \
        size_t capacity; \
    } type##Da

#ifndef DA_WITH_ONLY_DECLARE

// Capacity when calling DA_MAKE_DEFAULT
#define DA_DEFAULT_CAPACITY 8
// Capacity multiplier when calling DA_APPEND
#define DA_GROW_FACTOR 2

#define DA_MAKE_CAPACITY(type, cap) \
    (type##Da){ \
        .items = AllocateArray(NULL, cap, sizeof(type)), \
        .count = 0,  \
        .capacity = cap \
    }

// Create a DA with the default capacity
#define DA_MAKE_DEFAULT(type) DA_MAKE_CAPACITY(type, DA_DEFAULT_CAPACITY)

/*
 * Helper for other append macros. Do not use directly.
 *
 * Appends an item and grows to the given capacity if needed.
 * The caller is expected to pass in a greater new capacity.
 */
#define DA_APPEND_RESIZE(da, item, newCap) \
    do { \
        if ((da)->count + 1 > (da)->capacity) { \
            (da)->capacity = newCap; \
            (da)->items = AllocateArray( \
                (da)->items, (da)->capacity, sizeof((da)->items[0]) \
            ); \
        } \
        (da)->items[(da)->count++] = item; \
    } while(0)

// Appends an item, growing the capacity by DA_GROW_FACTOR if needed.
#define DA_APPEND(da, item) \
    DA_APPEND_RESIZE(da, item, (da)->capacity * DA_GROW_FACTOR)

// Appends an item, growing the capacity by 1 if needed.
#define DA_APPEND_GROW_ONE(da, item) \
    DA_APPEND_RESIZE(da, item, (da)->capacity + 1)

/*
 * Removes an item at a given index. The original
 * item order is not preserved.
 */
#define DA_REMOVE_UNORDERED(da, index) \
    do { \
        (da)->items[index] = (da)->items[--(da)->count]; \
    } while(0)

// Frees the DA. It can not be re-used.
#define DA_FREE(da) \
    do { \
        (da)->count = 0; \
        (da)->capacity = 0; \
        FreeMemory((da)->items); \
        (da)->items = NULL; \
    } while(0);

#endif // DA_WITH_ONLY_DECLARE

#endif
