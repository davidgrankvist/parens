#include "tests.h"
#include "da.h"
#include "bytecode.h"
#include "parser.h"

typedef struct {
    char* desc;
    char* input;
    ByteCodeResult expected;
} BytecodeGeneratorTestCase;

#define BYTECODE_GENERATOR_TEST_PAGE_SIZE 100
#define BYTECODE_GENERATOR_TEST_TOKEN_MAX 100

static bool BytecodeResultEquals(ByteCodeResult expected, ByteCodeResult actual) {
    if (expected.type != actual.type) {
        return false;
    }

    if (expected.type == BYTECODE_GENERATE_ERROR) {
        return true;
    }

    ByteDa expectedBytes = expected.as.success.byteCode;
    ByteDa actualBytes = actual.as.success.byteCode;

    if (expectedBytes.count != actualBytes.count) {
        return false;
    }

    for (int i = 0; i < expectedBytes.count; i++) {
        if (expectedBytes.items[i] != actualBytes.items[i]) {
            return false;
        }
    }

    return true;
}

static void RunTestCase(BytecodeGeneratorTestCase testCase) {
    printf("%s\n", testCase.desc);

    InitTokenizer(testCase.input);

    TokenDa tokens = DA_MAKE_CAPACITY(Token, BYTECODE_GENERATOR_TEST_TOKEN_MAX);

    Token token = {0};
    do {
        token = ConsumeToken();
        DA_APPEND(&tokens, token);
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);

    Assert(token.type != TOKEN_ERROR, "Failed to tokenize");

    Allocator* allocator = CreateBumpAllocator(BYTECODE_GENERATOR_TEST_PAGE_SIZE, 1);
    ParseResult parseResult = ParseTokens(tokens, allocator);

    Assert(parseResult.type == PARSE_SUCCESS, "Failed to parse");

    ByteCodeResult result = GenerateByteCode(parseResult.as.success.ast, allocator);

    if (!BytecodeResultEquals(testCase.expected, result)) {
        PRINT_TEST_FAILURE();
        printf("Expected:\n");
        PrintByteCodeResult(testCase.expected);
        printf("Actual:\n");
        PrintByteCodeResult(result);

        AssertFail("Unexpected bytecode result.");
    }

    AllocatorFree(allocator);
    DA_FREE(&tokens);
}

static ByteCodeResult MakeSuccess(Byte* bytes, size_t count) {
    ByteDa byteDa = (ByteDa) {
        .count = count,
        .capacity = count,
        .items = bytes,
    };
    ByteCodeResult result = (ByteCodeResult) {
        .type = BYTECODE_GENERATE_SUCCESS,
        .as.success = byteDa,
    };

    return result;
}

#define F64_BIG_ENDIAN_1 63, 240, 0, 0, 0, 0, 0, 0
#define F64_BIG_ENDIAN_2 64, 0, 0, 0, 0, 0, 0, 0

void BytecodeGeneratorTests() {
    PRINT_TEST_TITLE();

    RunTestCase((BytecodeGeneratorTestCase) {
        .desc = "Only nil",
        .input = "()",
        .expected = MakeSuccess((Byte[]){ OP_NIL }, 1),
    });

    RunTestCase((BytecodeGeneratorTestCase) {
        .desc = "Only number",
        .input = "1",
        .expected = MakeSuccess((Byte[]){ F64_BIG_ENDIAN_1, OP_F64 }, 9),
    });

    RunTestCase((BytecodeGeneratorTestCase) {
        .desc = "Simple cons",
        .input = "'(1 . 2)",
        .expected = MakeSuccess((Byte[]){
                F64_BIG_ENDIAN_2,
                OP_F64,
                F64_BIG_ENDIAN_1,
                OP_F64,
                OP_CONS_CELL
        }, 19),
    });

    RunTestCase((BytecodeGeneratorTestCase) {
        .desc = "Simple proper list",
        .input = "'(1 2)",
        .expected = MakeSuccess((Byte[]){
                OP_NIL,
                F64_BIG_ENDIAN_2,
                OP_F64,
                OP_CONS_CELL,
                F64_BIG_ENDIAN_1,
                OP_F64,
                OP_CONS_CELL
        }, 21),
    });

    RunTestCase((BytecodeGeneratorTestCase) {
        .desc = "Simple add",
        .input = "(+ 1 2)",
        .expected = MakeSuccess((Byte[]){
                F64_BIG_ENDIAN_2,
                OP_F64,
                OP_CONS_CELL,
                F64_BIG_ENDIAN_1,
                OP_F64,
                OP_CONS_CELL
        }, 19),
    });

}

