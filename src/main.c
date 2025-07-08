#include <stdio.h>
#include "da.h"
#include "tokens.h"
#include "parser.h"
#include "memory.h"
#include "bytecode.h"

// Some small values for now
#define TOKENS_DEFAULT_CAPACITY 256
#define AST_PAGE_SIZE 256
#define AST_NUM_PAGES 1

int main() {
    char* program = "(1 2 3)";
    printf("Input = %s\n", program);

    InitTokenizer(program);
    TokenDa tokens = DA_MAKE_CAPACITY(Token, TOKENS_DEFAULT_CAPACITY);
    Token token = {0};
    do {
        token = ConsumeToken();
        DA_APPEND(&tokens, token);
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);

    if (token.type == TOKEN_ERROR) {
        PrintToken(token);
        return 1;
    }
    Allocator* allocator = CreateBumpAllocator(AST_PAGE_SIZE, AST_NUM_PAGES);
    ParseResult parseResult = ParseTokens(tokens, allocator);
    PrintParseResult(parseResult);

    if (parseResult.type == PARSE_ERROR) {
        return 1;
    }

    Ast* ast = parseResult.as.success.ast;
    ByteCodeResult bytecodeResult = GenerateByteCode(ast, allocator);
    PrintByteCodeResult(bytecodeResult);
}
