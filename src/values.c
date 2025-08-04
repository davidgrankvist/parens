#include <stdio.h>
#include "values.h"
#include "asserts.h"

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

Object* CreateConsCellObject(Value head, Value tail, Allocator* allocator) {
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

static const char* MapOperatorTypeToStr(OperatorType operator) {
    switch(operator) {
        case OPERATOR_ADD: return "+";
        case OPERATOR_SUBTRACT: return "-";
        case OPERATOR_MULTIPLY: return "*";
        case OPERATOR_DIVIDE: return "/";
        case OPERATOR_PRINT: return "print";
        default: return NULL;
    }
}

static void PrintOperator(OperatorType operator) {
    const char* str = MapOperatorTypeToStr(operator);

    Assertf(str != NULL, "Not implemented for type %d", operator);
    printf("%s", str);
}

static const char* MapObjectTypeToStr(ObjectType object) {
    switch(object) {
        case OBJECT_STRING: return "OBJECT_STRING";
        case OBJECT_SYMBOL: return "OBJECT_SYMBOL";
        case OBJECT_CONS: return "OBJECT_CONS";
        default: return NULL;
    }
}

static void PrintObject(Object* obj) {
    switch (obj->type) {
        case OBJECT_STRING: {
            printf("\"");
            PrintString(obj->as.string);
            printf("\"");
            break;
        }
        case OBJECT_SYMBOL:
            printf("<symbol ");
            PrintString(obj->as.symbol);
            printf(">");
            break;
        case OBJECT_CONS: {
            ConsCell cons = obj->as.cons;
            Value head = cons.head;
            Value tail = cons.tail;
            printf("(");
            PrintValue(head);
            printf(" . ");
            PrintValue(tail);
            printf(")");
            break;
        }
        default:
            AssertFailf("Not implemented for object type %s", MapObjectTypeToStr(obj->type));
            break;
    }
}

static const char* MapValueTypeToStr(ValueType valueType) {
    switch(valueType) {
        case VALUE_NIL: return "VALUE_NIL";
        case VALUE_F64: return "VALUE_F64";
        case VALUE_BOOL: return "VALUE_BOOL";
        case VALUE_OBJECT: return "VALUE_OBJECT";
        case VALUE_OPERATOR: return "VALUE_OPERATOR";
        default: return NULL;
    }
}

void PrintValue(Value value) {
    switch(value.type) {
        case VALUE_NIL:
            printf("()");
            break;
        case VALUE_F64:
            printf("%g", value.as.f64);
            break;
        case VALUE_OBJECT:
            PrintObject(value.as.object);
            break;
        case VALUE_OPERATOR:
            PrintOperator(value.as.operator);
            break;
        default:
            AssertFailf("Not implemented for value type %s", MapValueTypeToStr(value.type));
            break;
    }
}
