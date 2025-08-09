/*
 * These value types represent two things:
 * 1. compile time values attached to AST atoms
 * 2. runtime values
 */
#ifndef values_h
#define values_h

#include <stdint.h>

#include "memory.h"
#include "common.h"
#include "da.h"

typedef enum {
    OPERATOR_ADD,
    OPERATOR_SUBTRACT,
    OPERATOR_MULTIPLY,
    OPERATOR_DIVIDE,
    OPERATOR_PRINT,
    OPERATOR_SET_GLOBAL,
} OperatorType;

typedef enum {
    COMPTIME_OPERATOR_NONE,
    COMPTIME_OPERATOR_FUN,
    COMPTIME_OPERATOR_ENUM_COUNT,
} ComptimeOperatorType;

typedef enum {
    VALUE_NIL,
    VALUE_F64,
    VALUE_BOOL,
    VALUE_OBJECT,
    VALUE_OPERATOR,
    VALUE_COMPTIME_OPERATOR,
    VALUE_FUNCTION,
} ValueType;

typedef struct Object Object;

typedef struct {
    uint32_t location;
} Function;

typedef struct {
    ValueType type;
    union {
        double f64;
        bool boolValue;
        Object* object;
        OperatorType operator;
        ComptimeOperatorType comptimeOperator;
        Function function;
    } as;
} Value;

typedef enum {
    OBJECT_STRING,
    OBJECT_SYMBOL,
    OBJECT_CONS,
} ObjectType;

typedef struct {
    Value head;
    Value tail;
} ConsCell;

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
#define MAKE_VALUE_COMPTIME_OPERATOR(op) (Value) { .type = VALUE_COMPTIME_OPERATOR, .as.comptimeOperator = op }

Object* CreateStringObject(String s, Allocator* allocator);
Object* CreateSymbolObject(String s, Allocator* allocator);
Object* CreateConsCellObject(Value head, Value tail, Allocator* allocator);

DA_DECLARE(Value);

void PrintValue(Value v);

#endif
