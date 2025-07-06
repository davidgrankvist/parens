#include <stdio.h>
#include "common.h"
#include "tokenizer.h"
#include "parser.h"

int main() {
    char* program = "(1 2 3)";
    printf("Tokenizing %s\n", program);

    InitTokenizer(program);

    //TODO(memory): Never freed. Use arena?
    TokenDa tokens = DA_MAKE_CAPACITY(Token, 100);

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
