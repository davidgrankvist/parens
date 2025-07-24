#ifndef values_h
#define values_h

#include "memory.h"
#include "stringz.h"

typedef enum {
    OBJECT_STRING,
    OBJECT_SYMBOL,
} ObjectType;

typedef struct {
   ObjectType type;
   union {
       String string;
       String symbol;
   } as;
} Object;

typedef enum {
    VALUE_NIL,
    VALUE_F64,
    VALUE_OBJECT,
} ValueType;

typedef struct {
    ValueType type;
    union {
        double f64;
        Object* object;
    } as;
} Value;

#define MAKE_VALUE_NIL() (Value) { .type = VALUE_NIL }
#define MAKE_VALUE_F64(x) (Value) { .type = VALUE_F64, .as.f64 = x }
#define MAKE_VALUE_OBJECT(obj) \
    (Value) { .type = VALUE_OBJECT, .as.object = obj }

Object* CreateStringObject(String s, Allocator* allocator);
Object* CreateSymbolObject(String s, Allocator* allocator);

#endif
