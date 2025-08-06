#include "da.h"
#include "tokens.h"
#include "parser.h"
#include "memory.h"
#include "bytecode.h"
#include "vm.h"

// Some small values for now
#define TOKENS_DEFAULT_CAPACITY 256
#define AST_PAGE_SIZE 256
#define AST_NUM_PAGES 1

// TODO(incomplete): Consider repl vs not repl. Currently not repl.
int main() {
    char* program = "'(1 2 3)";

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
    if (parseResult.type == RESULT_ERROR) {
        PrintParseResult(parseResult);
        return 1;
    }

    Ast* ast = parseResult.as.success.ast;
    ByteCodeResult byteCodeResult = GenerateByteCode(ast, allocator);
    if (byteCodeResult.type == RESULT_ERROR) {
        PrintByteCodeResult(byteCodeResult);
        return 1;
    }

    ByteDa byteCode = byteCodeResult.as.success.byteCode;
    VmResult vmResult = ExecuteByteCode(byteCode, allocator);
    if (vmResult.type == RESULT_ERROR) {
        PrintVmResult(vmResult);
        return 1;
    }
}
