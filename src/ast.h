#ifndef ast_h
#define ast_h

#include "common.h"

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
#define MAKE_STRING(s) MAKE_OBJECT(CreateStringObject(s))
#define MAKE_SYMBOL(s) MAKE_OBJECT(CreateSymbolObject(s))

#define MAKE_STRING_CHARS(cs) MAKE_STRING(MakeString(cs))
#define MAKE_SYMBOL_CHARS(cs) MAKE_SYMBOL(MakeString(cs))

Object* CreateStringObject(String s);
Object* CreateSymbolObject(String s);


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

Ast* CreateAtom(Value value);
Ast* CreateCons(Ast* head, Ast* tail);

#endif
