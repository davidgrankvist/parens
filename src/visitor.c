#include "ast.h"

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
