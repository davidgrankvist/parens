#include "ast.h"

Ast* CreateAtom(Value value, Token* token, Allocator* allocator) {
    Ast* ast = AllocatorAlloc(sizeof(Ast), allocator);
    *ast = (Ast) {
        .type = AST_ATOM,
        .token = token,
        .as.atom = (AstAtom) {
            .value = value,
        },
    };

    return ast;
}

Ast* CreateCons(Ast* head, Ast* tail, Allocator* allocator) {
    Ast* ast = AllocatorAlloc(sizeof(Ast), allocator);
    *ast = (Ast) {
        .type = AST_CONS,
            .token = head->token,
            .as.cons = (AstCons) {
                .head = head,
                .tail = tail,
            },
    };
    return ast;
}

Ast* CreateSymbolAtom(String s, Token* token, Allocator* allocator) {
    Object* obj = CreateSymbolObject(s, allocator);
    Value val = MAKE_VALUE_OBJECT(obj);
    Ast* ast = CreateAtom(val, token, allocator);
    return ast;
}

Ast* CreateStringAtom(String s, Token* token, Allocator* allocator) {
    Object* obj = CreateStringObject(s, allocator);
    Value val = MAKE_VALUE_OBJECT(obj);
    Ast* ast = CreateAtom(val, token, allocator);
    return ast;
}

void VisitAst(Ast* ast, AstVisitor* visitor, void* ctx) {
    switch(ast->type) {
        case AST_ATOM:
            visitor->VisitAtom(ast, ctx);
            break;
        case AST_CONS:
            visitor->VisitCons(ast, ctx);
            break;
        default: break;
    }
}
