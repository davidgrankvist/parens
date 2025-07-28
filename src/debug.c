#include <stdio.h>
#include <stdint.h>
#include "tokens.h"
#include "parser.h"

const char* MapTokenTypeToStr(TokenType type) {
    switch(type) {
        case TOKEN_PAREN_START: return "TOKEN_PAREN_START";
        case TOKEN_PAREN_END: return "TOKEN_PAREN_END";
        case TOKEN_CONS: return "TOKEN_CONS";
        case TOKEN_NIL: return "TOKEN_NIL";
        case TOKEN_NUMBER: return "TOKEN_NUMBER";
        case TOKEN_STRING: return "TOKEN_STRING";
        case TOKEN_SYMBOL: return "TOKEN_SYMBOL";
        case TOKEN_EOF: return "TOKEN_EOF";
        case TOKEN_ERROR: return "TOKEN_ERROR";
        case TOKEN_PLUS: return "TOKEN_PLUS";
        case TOKEN_MINUS: return "TOKEN_MINUS";
        case TOKEN_STAR: return "TOKEN_STAR";
        case TOKEN_SLASH: return "TOKEN_SLASH";
        case TOKEN_PRINT: return "TOKEN_PRINT";
        default: return NULL;
    }
}

void PrintTokenType(TokenType type) {
    const char* typeStr = MapTokenTypeToStr(type);
    printf("%s", typeStr);
}

void PrintToken(Token token) {
    if (token.type == TOKEN_ERROR) {
        fprintf(stderr, "Error at %d,%d - ", token.line, token.col);
        PrintStringErr(token.str);
        fprintf(stderr, "\n");
        return;
    }
    const char* typeStr = MapTokenTypeToStr(token.type);
    printf("Token %s at %d,%d", typeStr, token.line, token.col);
    if (token.type != TOKEN_EOF) {
        printf(" - ");
        PrintString(token.str);
    }
    printf("\n");
}

static void PrintAstHelper(Ast* ast, void* ctx);

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
    if (str == NULL) {
        printf("UNKNOWN");
    } else {
        printf("%s", str);
    }
}

static void PrintValue(Value value) {
    switch(value.type) {
        case VALUE_NIL:
            printf("()");
            break;
        case VALUE_F64:
            printf("%g", value.as.f64);
            break;
        case VALUE_OBJECT:
            if (value.as.object->type == OBJECT_STRING) {
                printf("\"");
                PrintString(value.as.object->as.string);
                printf("\"");
                break;
            } else if (value.as.object->type == OBJECT_SYMBOL) {
                PrintString(value.as.object->as.symbol);
                break;
            }
            printf("UNKNOWN");
            break;
        case VALUE_OPERATOR:
            PrintOperator(value.as.operator);
            break;
        default:
            printf("UNKNOWN");
            break;
    }
}

static void PrintAstAtom(Ast* ast, void* ctx) {
    int indent = (int)(intptr_t)ctx;
    AstAtom atom = ast->as.atom;
    printf("%*s", indent, "");
    PrintValue(atom.value);
    printf("\n");
}

static void PrintAstCons(Ast* ast, void* ctx) {
    int indent = (int)(intptr_t)ctx;
    AstCons cons = ast->as.cons;
    printf("%*s(\n", indent, "");
    PrintAstHelper(cons.head, (void*)(intptr_t)(indent+2));
    PrintAstHelper(cons.tail, (void*)(intptr_t)(indent+2));
    printf("%*s)\n", indent, "");
}

static AstVisitor printVisitor = {
    .VisitAtom = &PrintAstAtom,
    .VisitCons = &PrintAstCons,
};

static void PrintAstHelper(Ast* ast, void* ctx) {
    if (ast->isQuoted) {
        printf("'");
    }
    VisitAst(ast, &printVisitor, ctx);
}

void PrintAst(Ast* ast) {
    if (ast == NULL) {
        printf("NULL\n");
        return;
    }
    PrintAstHelper(ast, (void*)0);
}

void PrintParseResult(ParseResult result) {
    if (result.type == PARSE_ERROR) {
        ParseError error = result.as.error;
        fprintf(stderr, "Parse error: ");
        PrintStringErr(error.message);
        fprintf(stderr, "\n");

        if (error.token != NULL) {
            PrintToken(*(error.token));
        }
        return;
    }
    printf("Parsed AST successfully\n");
    PrintAst(result.as.success.ast);
}

const char* MapParseResultTypeToStr(ParseResultType type) {
    switch(type) {
        case PARSE_SUCCESS: return "PARSE_SUCCESS";
        case PARSE_ERROR: return "PARSE_ERROR";
        default: return NULL;
    }
}
