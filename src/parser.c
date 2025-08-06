#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "parser.h"
#include "memory.h"

static TokenDa tokens = {0};
static size_t currentIndex = 0;
static Allocator* astAllocator = NULL;

static Token* Peek() {
    return &tokens.items[currentIndex];
}

static Token* Previos() {
    return &tokens.items[currentIndex - 1];
}

static bool IsDone() {
    return currentIndex >= tokens.count || Peek()->type == TOKEN_EOF;
}

static Token* Advance() {
    if (IsDone()) {
        return Previos();
    }
    return &tokens.items[currentIndex++];
}

static bool Match(TokenType type) {
    Token* token = Peek();
    if (token->type == type) {
        Advance();
        return true;
    }
    return false;
}

static bool Check(TokenType type) {
    Token* token = Peek();
    return token->type == type;
}

static ParseResult EmitParseError(const char* message) {
    ParseError error = {
        .message = MakeString(message),
        .token = &tokens.items[currentIndex],
    };

    ParseResult result = {
        .type = RESULT_ERROR,
        .as.error = error,
    };

    return result;
}

static ParseResult EmitParseSuccess(Ast* ast) {
    ParseSuccess success = {
        .ast = ast,
    };

    ParseResult result = {
        .type = RESULT_SUCCESS,
        .as.success = success,
    };

    return result;
}

static ParseResult ParseExpr();

static ParseResult ParseF64() {
    String str = Peek()->str;
    double num = ParseStringAsDouble(str);
    Value val = MAKE_VALUE_F64(num);
    Token* token = Peek();

    return EmitParseSuccess(CreateAtom(val, token, astAllocator));
}

static ParseResult ParseSymbol() {
    Token* token = Peek();
    String s = token->str;
    Ast* ast = CreateSymbolAtom(s, token, astAllocator);
    return EmitParseSuccess(ast);
}

static ParseResult ParseString() {
    Token* token = Peek();
    String quotedStr = token->str;
    String unquotedStr = {
        .start = &quotedStr.start[1],
        .length = quotedStr.length - 2,
    };
    Ast* ast = CreateStringAtom(unquotedStr, token, astAllocator);
    return EmitParseSuccess(ast);
}

static ParseResult ParseOperator(OperatorType op) {
    Token* token = Peek();
    Value val = MAKE_VALUE_OPERATOR(op);
    Ast* ast = CreateAtom(val, token, astAllocator);
    return EmitParseSuccess(ast);
}

static ParseResult ParseAtom() {
    ParseResult result = {0};
    Ast* ast = NULL;
    Token* token = Peek();
    switch (token->type) {
        case TOKEN_NIL:
            ast = CreateAtom(MAKE_VALUE_NIL(), token, astAllocator);
            result = EmitParseSuccess(ast);
            break;
        case TOKEN_NUMBER:
            result = ParseF64();
            break;
        case TOKEN_STRING:
            result = ParseString();
            break;
        case TOKEN_SYMBOL:
            result = ParseSymbol();
            break;
        case TOKEN_PLUS:
            result = ParseOperator(OPERATOR_ADD);
            break;
        case TOKEN_MINUS:
            //TODO(incomplete): consider unary minus
            result = ParseOperator(OPERATOR_SUBTRACT);
            break;
        case TOKEN_STAR:
            result = ParseOperator(OPERATOR_MULTIPLY);
            break;
        case TOKEN_SLASH:
            result = ParseOperator(OPERATOR_DIVIDE);
            break;
        case TOKEN_PRINT:
            result = ParseOperator(OPERATOR_PRINT);
            break;
        default:
            result = EmitParseError("Unexpected token while parsing atom");
            break;
    }

    Advance();
    return result;
}

/*
 * Parses the tail in proper list syntax.
 * Adds the implicit nil.
 *
 * For example, given (1 2 3) this function
 * will be called for the 2 3 part and return
 * (2 . (3 . nil))
 */
static ParseResult ParseListElements() {
    // add implicit nil if we reached the end
    if (Check(TOKEN_PAREN_END)) {
        Token* token = Peek();
        return EmitParseSuccess(CreateAtom(MAKE_VALUE_NIL(), token, astAllocator));
    }

    ParseResult head = ParseExpr();
    if (head.type == RESULT_ERROR) {
        return head;
    }
    ParseResult tail = ParseListElements();
    if (tail.type == RESULT_ERROR) {
        return tail;
    }

    Ast* cons = CreateCons(head.as.success.ast, tail.as.success.ast, astAllocator);
    return EmitParseSuccess(cons);
}

/*
 * Parses either an explicit cons list like
 * (1 . (2 . 3))
 *
 * Or a proper list with implicit nil
 * (1 2 3) = (1 . (2 . (3 . nil))
 */
static ParseResult ParseList() {
    ParseResult head = ParseExpr();
    if (head.type == RESULT_ERROR) {
        return head;
    }

    ParseResult tail = {0};
    if (Match(TOKEN_CONS)) {
        tail = ParseExpr();
    } else {
        tail = ParseListElements();
    }

    if (tail.type == RESULT_ERROR) {
        return tail;
    }

    if (!Match(TOKEN_PAREN_END)) {
        return EmitParseError("Unterminated list parentheses");
    }

    Ast* cons = CreateCons(head.as.success.ast, tail.as.success.ast, astAllocator);
    return EmitParseSuccess(cons);
}

static ParseResult ParseExpr() {
    bool isQuoted = Match(TOKEN_QUOTE);
    ParseResult result = {0};
    if (Match(TOKEN_PAREN_START)) {
        result = ParseList();
    } else {
        result = ParseAtom();
    }

    if (result.type == RESULT_SUCCESS && result.as.success.ast != NULL) {
        result.as.success.ast->isQuoted = isQuoted;
    }

    return result;
}

ParseResult ParseTokens(TokenDa ts, Allocator* allocator) {
    tokens = ts;
    currentIndex = 0;
    astAllocator = allocator;

    if (IsDone()) {
        return EmitParseError("Nothing to parse.");
    }

    return ParseExpr();
}

static void PrintAstHelper(Ast* ast, void* ctx);

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
    if (result.type == RESULT_ERROR) {
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
