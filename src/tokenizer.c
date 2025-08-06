#include <stdio.h>
#include "tokens.h"

static char* current = NULL;
static char* tokenStart = NULL;
static int line;
static int col;

static char Peek() {
    return *current;
}

static char Advance() {
    char c = Peek();
    if (c != '\0') {
        current++;
        col++;
    }
    return c;
}

void InitTokenizer(char* str) {
    current = str;
    tokenStart = str;
    line = 1;
    col = 0;
}

static void SkipWhiteSpace() {
    while (true) {
        char c = Peek();
        switch(c) {
            case ' ':
            case '\t':
            case '\r':
                Advance();
                break;
            case '\n':
                Advance();
                line++;
                col = 0;
                break;
            default: return;
        }
    }
}

static bool IsAtEnd() {
    return Peek() == '\0';
}

static Token EmitToken(TokenType type) {
    Token token = {
        .type = type,
        .str = {
            .start = tokenStart,
            .length = current - tokenStart,
        },
        .line = line,
        .col = col,
    };

    return token;
}

static Token EmitError(const char* msg) {
    Token token = {
        .type = TOKEN_ERROR,
        .str = MakeString(msg),
        .line = line,
        .col = col,
    };

    return token;
}

static bool IsDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool IsAlpha(char c) {
    return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

static Token ConsumeNumber() {
    while (IsDigit(Peek())) {
        Advance();
    }

    if (Peek() == '.') {
        Advance();
        while (IsDigit(Peek())) {
            Advance();
        }
    }

    return EmitToken(TOKEN_NUMBER);
}

static TokenType TryEmitKeyword(size_t startOffset, String keywordPart, TokenType keywordType) {
    String subTokenString = {
        .start = tokenStart + startOffset,
        .length = current - tokenStart + - startOffset,
    };
    if (StringEquals(subTokenString, keywordPart)) {
        return keywordType;
    }

    return TOKEN_SYMBOL;
}

static TokenType EmitKeywordOrSymbolType() {
    switch(tokenStart[0]) {
        case 'p': return TryEmitKeyword(1, (String){ "rint", 4 }, TOKEN_PRINT);
        case 's': return TryEmitKeyword(1, (String){ "et", 2 }, TOKEN_SET);
        case 'f': return TryEmitKeyword(1, (String){ "un", 2 }, TOKEN_FUN);
        case 'd': return TryEmitKeyword(1, (String){ "efun", 4 }, TOKEN_DEFUN);
        default: return TOKEN_SYMBOL;
    }
}

static Token ConsumeKeywordOrSymbol() {
    while (IsAlpha(Peek()) || IsDigit(Peek())) {
        Advance();
    }

    return EmitToken(EmitKeywordOrSymbolType());
}

static Token ConsumeString() {
    while(Peek() != '"' && !IsAtEnd()) {
        Advance();
    }

    if (Peek() != '"') {
        return EmitError("Unterminated string.");
    }
    Advance();

    return EmitToken(TOKEN_STRING);
}

Token ConsumeToken() {
    SkipWhiteSpace();
    if (IsAtEnd()) {
       return EmitToken(TOKEN_EOF);
    }
    tokenStart = current;
    char c = Advance();

    if (IsDigit(c)) {
        return ConsumeNumber();
    }

    if (IsAlpha(c)) {
        return ConsumeKeywordOrSymbol();
    }

    switch(c) {
        case '(': {
            if (Peek() == ')') {
                Advance();
                return EmitToken(TOKEN_NIL);
            }
            return EmitToken(TOKEN_PAREN_START);
        }
        case ')':
            return EmitToken(TOKEN_PAREN_END);
        case '.':
            return EmitToken(TOKEN_CONS);
        case '"':
            return ConsumeString();
        case '\'':
            return EmitToken(TOKEN_QUOTE);
        case '+':
            return EmitToken(TOKEN_PLUS);
        case '-':
            return EmitToken(TOKEN_MINUS);
        case '*':
            return EmitToken(TOKEN_STAR);
        case '/':
            return EmitToken(TOKEN_SLASH);
        default: break;
    }

    return EmitError("Unexpected character.");
}


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
        case TOKEN_SET: return "TOKEN_SET";
        case TOKEN_FUN: return "TOKEN_FUN";
        case TOKEN_DEFUN: return "TOKEN_DEFUN";
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
