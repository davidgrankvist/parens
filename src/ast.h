#ifndef ast_h
#define ast_h

#include "tokens.h"
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

#define MAKE_VALUE_NIL() (Value) { .type = VALUE_NIL }
#define MAKE_VALUE_F64(x) (Value) { .type = VALUE_F64, .as.f64 = x }
#define MAKE_VALUE_OBJECT(obj) \
    (Value) { .type = VALUE_OBJECT, .as.object = obj }

Object* CreateStringObject(String s, Allocator* allocator);
Object* CreateSymbolObject(String s, Allocator* allocator);

// -- AST --

typedef enum {
    AST_ATOM,
    AST_CONS,
} AstType;

// Forward declare for recursive AstCons definition
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
    Token* token;
    union {
        AstAtom atom;
        AstCons cons;
    } as;
};

Ast* CreateAtom(Value value, Token* token, Allocator* allocator);
Ast* CreateCons(Ast* head, Ast* tail, Allocator* allocator);
Ast* CreateStringAtom(String s, Token* token, Allocator* allocator);
Ast* CreateSymbolAtom(String s, Token* token, Allocator* allocator);

// -- Visitor --

typedef struct {
    void (*VisitAtom)(Ast* ast, void* ctx);
    void (*VisitCons)(Ast* ast, void* ctx);
} AstVisitor;

void VisitAst(Ast* ast, AstVisitor* visitor, void* ctx);
void PrintAst(Ast* ast);

#endif
