#ifndef ast_h
#define ast_h

#include "tokens.h"
#include "memory.h"
#include "values.h"

typedef enum {
    AST_ATOM,
    AST_CONS,
} AstType;

typedef struct {
    Value value;
} AstAtom;

typedef struct Ast Ast;

typedef struct {
    Ast* head;
    Ast* tail;
} AstCons;

struct Ast {
    AstType type;
    Token* token;
    bool isQuoted;
    union {
        AstAtom atom;
        AstCons cons;
    } as;
};

Ast* CreateAtom(Value value, Token* token, Allocator* allocator);
Ast* CreateCons(Ast* head, Ast* tail, Allocator* allocator);
Ast* CreateStringAtom(String s, Token* token, Allocator* allocator);
Ast* CreateSymbolAtom(String s, Token* token, Allocator* allocator);

typedef struct {
    void (*VisitAtom)(Ast* ast, void* ctx);
    void (*VisitCons)(Ast* ast, void* ctx);
} AstVisitor;

void VisitAst(Ast* ast, AstVisitor* visitor, void* ctx);
void PrintAst(Ast* ast);

#endif
