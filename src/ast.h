#ifndef ast_h
#define ast_h

#include "common.h"
#include "memory.h"

// -- Values --

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

#define MAKE_NIL() (Value) { .type = VALUE_NIL }
#define MAKE_F64(x) (Value) { .type = VALUE_F64, .as.f64 = x }
#define MAKE_OBJECT(obj) (Value) { .type = VALUE_OBJECT, .as.object = obj }
#define MAKE_STRING(s, allocator) MAKE_OBJECT(CreateStringObject(s, allocator))
#define MAKE_SYMBOL(s, allocator) MAKE_OBJECT(CreateSymbolObject(s, allocator))

#define MAKE_STRING_CHARS(cs, allocator) MAKE_STRING(MakeString(cs), allocator)
#define MAKE_SYMBOL_CHARS(cs, allocator) MAKE_SYMBOL(MakeString(cs), allocator)

Object* CreateStringObject(String s, Allocator* allocator);
Object* CreateSymbolObject(String s, Allocator* allocator);


// -- AST --

typedef enum {
    AST_ATOM,
    AST_CONS,
} AstType;

typedef struct Ast Ast;

typedef struct {
    Value value;
} AstAtom;

typedef struct {
    Ast* head;
    Ast* tail;
} AstCons;

struct Ast {
    AstType type;
    Token token;
    union {
        AstAtom atom;
        AstCons cons;
    } as;
};

typedef struct {
    void (*VisitAtom)(Ast* ast, void* ctx);
    void (*VisitCons)(Ast* ast, void* ctx);
} AstVisitor;

void VisitAst(Ast* ast, AstVisitor* visitor, void* ctx);

Ast* CreateAtom(Value value, Allocator* allocator);
Ast* CreateCons(Ast* head, Ast* tail, Allocator* allocator);

#endif
