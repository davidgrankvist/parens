/*
 * These value types represent two things:
 * 1. compile time values attached to AST atoms
 * 2. runtime values
 */
#ifndef values_h
#define values_h

#include "memory.h"
#include "common.h"
#include "da.h"

typedef enum {
    OPERATOR_ADD,
    OPERATOR_SUBTRACT,
    OPERATOR_MULTIPLY,
    OPERATOR_DIVIDE,
    OPERATOR_PRINT,
} OperatorType;

typedef enum {
    VALUE_NIL,
    VALUE_F64,
    VALUE_BOOL,
    VALUE_OBJECT,
    VALUE_OPERATOR,
} ValueType;

typedef struct Object Object;

typedef struct {
    ValueType type;
    union {
        double f64;
        bool boolValue;
        Object* object;
        OperatorType operator;
    } as;
} Value;

typedef struct {
    Value head;
    Value tail;
} ConsCell;

typedef enum {
    OBJECT_STRING,
    OBJECT_SYMBOL,
    OBJECT_CONS,
} ObjectType;

struct Object {
    ObjectType type;
    int refCount;
    union {
        String string;
        String symbol;
        ConsCell cons;
    } as;
};


#define MAKE_VALUE_NIL() (Value) { .type = VALUE_NIL }
#define MAKE_VALUE_F64(x) (Value) { .type = VALUE_F64, .as.f64 = x }
#define MAKE_VALUE_BOOL(b) (Value) { .type = VALUE_BOOL, .as.boolValue = b }
#define MAKE_VALUE_OBJECT(obj) (Value) { .type = VALUE_OBJECT, .as.object = obj }
#define MAKE_VALUE_OPERATOR(op) (Value) { .type = VALUE_OPERATOR, .as.operator = op }

Object* CreateStringObject(String s, Allocator* allocator);
Object* CreateSymbolObject(String s, Allocator* allocator);
Object* CreateConsCellObject(Value head, Value tail, Allocator* allocator);

DA_DECLARE(Value);

void PrintValue(Value v);

#endif
