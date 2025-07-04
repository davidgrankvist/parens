#include <stdio.h>
#include "common.h"
#include "tokenizer.h"

int main() {
    char* program = "(1 2 3)";
    printf("Tokenizing %s\n", program);

    InitTokenizer(program);

    Token token = {0};
    do {
        token = ConsumeToken();
        PrintToken(token);
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);
}
