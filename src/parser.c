#include <stddef.h>
#include <stdbool.h>
#include "parser.h"
#include "asserts.h"

static TokenDa tokens = {0};
static size_t currentIndex = 0;

static Token Previos() {
    return tokens.items[currentIndex - 1];
}

static Token Peek() {
    return tokens.items[currentIndex];
}

static bool IsDone() {
    return currentIndex >= tokens.count || Peek().type == TOKEN_EOF;
}

static Token Advance() {
    if (IsDone()) {
        return Previos();
    }
    return tokens.items[currentIndex++];
}

static bool Match(TokenType type) {
    Token token = Peek();
    if (token.type == type) {
        Advance();
        return true;
    }
    return false;
}

static bool Check(TokenType type) {
    Token token = Peek();
    return token.type == type;
}

static ParseResult EmitParseError(const char* message) {
    ParseError error = {
        .message = MakeString(message),
        .token = &tokens.items[currentIndex],
    };

    ParseResult result = {
        .type = PARSE_ERROR,
        .as.error = error,
    };

    return result;
}

static ParseResult EmitParseSuccess(Ast* ast) {
    ParseSuccess success = {
        .ast = ast,
    };

    ParseResult result = {
        .type = PARSE_SUCCESS,
        .as.success = success,
    };

    return result;
}

static Ast* CreateAtom(Value value) {
    //TODO(memory): Never freed. Use arena?
    Ast* ast = calloc(1, sizeof(Ast));
    *ast = (Ast) {
        .type = AST_ATOM,
        .token = Peek(),
        .as.atom = (AstAtom) {
            .value = value,
        },
    };

    return ast;
}

static Ast* CreateCons(Ast* head, Ast* tail) {
    //TODO(memory): Never freed. Use arena?
    Ast* ast = calloc(1, sizeof(Ast));
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

static ParseResult ParseExpr();

static ParseResult ParseF64() {
    String str = Peek().str;
    double num = ParseStringAsDouble(str);
    Value val = MAKE_F64(num);

    return EmitParseSuccess(CreateAtom(val));
}

static ParseResult ParseAtom() {
    ParseResult result = {0};
    Ast* ast = NULL;
    switch (Peek().type) {
        case TOKEN_NIL:
            ast = CreateAtom(MAKE_NIL());
            result = EmitParseSuccess(ast);
            break;
        case TOKEN_NUMBER:
            result = ParseF64();
            break;
        case TOKEN_SYMBOL:
            result = EmitParseError("Symbol atoms are not implemented");
            break;
        case TOKEN_STRING:
            result = EmitParseError("String atoms are not implemented");
            break;
        default:
            result = EmitParseError("Unexpected token while parsing atom");
            break;
    }

    return result;
}

/*
 * Parses a cons list. Proper list syntax has an implicit nil.
 * (1 2) -> (1 . (2 . nil))
 */
static ParseResult ParseList() {
    ParseResult head = ParseExpr();
    if (head.type == PARSE_ERROR) {
        return head;
    }
    Ast* cons = NULL;

    /*
     * single value list
     * (1) -> (1 . nil)
     */
    if (Match(TOKEN_PAREN_END)) {
        cons = CreateCons(head.as.success.ast, CreateAtom(MAKE_NIL()));
        return EmitParseSuccess(cons);
    }

    bool shouldAppendNil = !Match(TOKEN_CONS);

    ParseResult expr = ParseExpr();
    if (expr.type == PARSE_ERROR) {
        return expr;
    }

    Ast* tail = expr.as.success.ast;
    while (!Check(TOKEN_PAREN_END) && !IsDone()) {
        expr = ParseExpr();
        if (expr.type == PARSE_ERROR) {
            return expr;
        }

        tail = CreateCons(tail, expr.as.success.ast);
        Advance();
    }

    if (shouldAppendNil) {
        tail = CreateCons(tail, CreateAtom(MAKE_NIL()));
    }

    if (!Match(TOKEN_PAREN_END)) {
        return EmitParseError("Unterminated list parentheses");
    }

    cons = CreateCons(head.as.success.ast, tail);
    return EmitParseSuccess(cons);
}

static ParseResult ParseExpr() {
    if (Match(TOKEN_PAREN_START)) {
        return ParseList();
    } else {
        return ParseAtom();
    }
}

ParseResult ParseTokens(TokenDa ts) {
    tokens = ts;
    currentIndex = 0;

    return ParseExpr();
}
