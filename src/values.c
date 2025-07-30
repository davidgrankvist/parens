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

Object* CreateConsAtomObject(Value* value, Allocator* allocator) {
    Object* obj = AllocatorAlloc(sizeof(Object), allocator);
    Cons atom = {
        .type = CONS_ATOM,
        .as.atom.value = value,
    };

    *obj = (Object) {
        .type = OBJECT_CONS,
        .as.cons = atom,
    };

    return obj;
}

Object* CreateConsCellObject(Cons* head, Cons* tail, Allocator* allocator) {
    Object* obj = AllocatorAlloc(sizeof(Object), allocator);
    Cons cell = {
        .type = CONS_CELL,
        .as.cons = {
            .head = head,
            .tail = tail,
        },
    };

    *obj = (Object) {
        .type = OBJECT_CONS,
        .as.cons = cell,
    };

    return obj;
}
