/*
 * These value types represent two things:
 * 1. compile time values attached to AST atoms
 * 2. runtime values
 */
#ifndef values_h
#define values_h

#include "memory.h"
#include "stringz.h"
#include "da.h"

typedef enum {
    CONS_ATOM,
    CONS_CELL,
} ConsType;

typedef struct Value Value;

typedef struct {
    Value* value;
} ConsAtom;

typedef struct Cons Cons;

typedef struct {
    Cons* head;
    Cons* tail;
} ConsCell;

struct Cons {
    ConsType type;
    union {
        ConsAtom atom;
        ConsCell cons;
    } as;
};

typedef enum {
    OBJECT_STRING,
    OBJECT_SYMBOL,
    OBJECT_CONS,
} ObjectType;

typedef struct {
   ObjectType type;
   union {
       String string;
       String symbol;
       Cons cons;
   } as;
} Object;

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

struct Value {
    ValueType type;
    union {
        double f64;
        bool boolValue;
        Object* object;
        OperatorType operator;
    } as;
};

#define MAKE_VALUE_NIL() (Value) { .type = VALUE_NIL }
#define MAKE_VALUE_F64(x) (Value) { .type = VALUE_F64, .as.f64 = x }
#define MAKE_VALUE_BOOL(b) (Value) { .type = VALUE_BOOL, .as.boolValue = b }
#define MAKE_VALUE_OBJECT(obj) (Value) { .type = VALUE_OBJECT, .as.object = obj }
#define MAKE_VALUE_OPERATOR(op) (Value) { .type = VALUE_OPERATOR, .as.operator = op }

Object* CreateStringObject(String s, Allocator* allocator);
Object* CreateSymbolObject(String s, Allocator* allocator);
Object* CreateConsAtomObject(Value* value, Allocator* allocator);
Object* CreateConsCellObject(Cons* head, Cons* tail, Allocator* allocator);

DA_DECLARE(Value);

void PrintValue(Value v);

#endif
