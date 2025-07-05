#ifndef ast_h
#define ast_h

#include "common.h"

// -- Values --

typedef enum {
    OBJECT_STRING,
} ObjectType;

typedef struct {
   ObjectType type;
   union {
       String string;
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

#define MAKE_NIL() (Value) \
    { .type = VALUE_NIL }
#define MAKE_F64(x) \
    (Value) { .type = VALUE_F64, .as = { .f64 = x } }

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

#endif
