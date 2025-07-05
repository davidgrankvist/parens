#include <stdio.h>
#include "common.h"
#include "tokenizer.h"
#include "parser.h"

int main() {
    char* program = "(1 2 3)";
    printf("Tokenizing %s\n", program);

    InitTokenizer(program);

    // use external buffer while DA implementation is unfinished
    Token tokenBuf[100] = {0};
    TokenDa tokens = DA_MAKE_CAPACITY(Token, 100);
    tokens.items = tokenBuf;

    Token token = {0};
    do {
        token = ConsumeToken();
        DA_APPEND(&tokens, token);
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);

    if (token.type == TOKEN_ERROR) {
        PrintToken(token);
        return 1;
    }
    printf("Tokenization success! Found %ld tokens\n", tokens.count);

    printf("Parsing tokens\n");
    ParseResult result = ParseTokens(tokens);
    PrintParseResult(result);
}
