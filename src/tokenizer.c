#include "tokenizer.h"

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
        case 'n': return TryEmitKeyword(1, (String){ "il", 2 }, TOKEN_NIL);
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
        case '(':
            return EmitToken(TOKEN_PAREN_START);
        case ')':
            return EmitToken(TOKEN_PAREN_END);
        case '.':
            return EmitToken(TOKEN_CONS);
        case '"':
            return ConsumeString();
        default: break;
    }

    return EmitError("Unexpected character.");
}
