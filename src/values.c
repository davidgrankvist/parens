#include "values.h"

Object* CreateStringObject(String s, Allocator* allocator) {
    Object* obj = AllocatorAlloc(sizeof(Object), allocator);
    *obj = (Object) {
        .type = OBJECT_STRING,
        .as.string = s,
    };

    return obj;
}

Object* CreateSymbolObject(String s, Allocator* allocator) {
    Object* obj = AllocatorAlloc(sizeof(Object), allocator);
    *obj = (Object) {
        .type = OBJECT_SYMBOL,
        .as.symbol = s,
    };

    return obj;
}

Object* CreateConsCellObject(Value* head, Value* tail, Allocator* allocator) {
    Object* obj = AllocatorAlloc(sizeof(Object), allocator);
    ConsCell cell = {
        .head = head,
        .tail = tail,
    };

    *obj = (Object) {
        .type = OBJECT_CONS,
        .as.cons = cell,
    };

    return obj;
}
