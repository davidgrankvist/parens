#include "tests.h"
#include "da.h"
#include "tokens.h"
#include "parser.h"
#include "bytecode.h"
#include "vm.h"

typedef struct {
    char* input;
    char* desc;
    VmResult expected;
} VmTestCase;

#define VM_TEST_TOKEN_MAX 100
#define VM_TEST_PAGE_SIZE 256

static bool ValueEquals(Value first, Value second) {
    if (first.type != second.type) {
        return false;
    }

    bool result = false;
    switch (first.type) {
        case VALUE_NIL:
            result = true;
            break;
        case VALUE_F64:
            result = first.as.f64 == second.as.f64;
            break;
        case VALUE_BOOL:
            result = first.as.boolValue == second.as.boolValue;
            break;
        case VALUE_OBJECT:
            result = first.as.object == second.as.object;
            break;
        case VALUE_OPERATOR:
            result = first.as.operator == second.as.operator;
            break;
        default:
            break;
    }
    return result;
}

static void PrintVmTestResult(VmResult result) {
    if (result.type == VM_ERROR) {
        VmError error = result.as.error;
        fprintf(stderr, "VM error: ");
        PrintString(error.message);
        fprintf(stderr, "\n");

        return;
    }

    printf("VM success. Printing value stack.\n");
    ValueDa values = result.as.success.values;
    for (ssize_t i = values.count - 1; i >= 0; i--) {
        PrintValue(values.items[i]);
    }
}

static bool VmResultEquals(VmResult expected, VmResult actual) {
    if (expected.type != actual.type) {
        return false;
    }

    if (expected.type == VM_ERROR) {
        return true;
    }

    ValueDa expectedValues = expected.as.success.values;
    ValueDa actualValues = actual.as.success.values;

    if (expectedValues.count != actualValues.count) {
        return false;
    }

    for (int i = 0; i < expectedValues.count; i++) {
        if (!ValueEquals(expectedValues.items[i], actualValues.items[i])) {
            return false;
        }
    }

    return true;
}

static void RunTestCase(VmTestCase testCase) {
    printf("%s\n", testCase.desc);

    InitTokenizer(testCase.input);

    TokenDa tokens = DA_MAKE_CAPACITY(Token, VM_TEST_TOKEN_MAX);

    Token token = {0};
    do {
        token = ConsumeToken();
        DA_APPEND(&tokens, token);
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);

    Assert(token.type != TOKEN_ERROR, "Failed to tokenize");

    Allocator* allocator = CreateBumpAllocator(VM_TEST_PAGE_SIZE, 1);
    ParseResult parseResult = ParseTokens(tokens, allocator);

    Assert(parseResult.type == PARSE_SUCCESS, "Failed to parse");

    ByteCodeResult byteCodeResult = GenerateByteCode(parseResult.as.success.ast, allocator);
    Assert(byteCodeResult.type == BYTECODE_GENERATE_SUCCESS, "Failed to generate bytecode");

    VmResult result = ExecuteByteCode(byteCodeResult.as.success.byteCode, allocator);
    if (!VmResultEquals(testCase.expected, result)) {
        PRINT_TEST_FAILURE();
        printf("Expected:\n");
        PrintVmTestResult(testCase.expected);
        printf("Actual:\n");
        PrintVmTestResult(result);

        AssertFail("Unexpected VM result.");
    }

    AllocatorFree(allocator);
    DA_FREE(&tokens);
}

static VmResult MakeSuccess(Value* values, size_t count) {
    ValueDa valueDa = (ValueDa) {
        .count = count,
        .capacity = count,
        .items = values,
    };

    VmResult result = (VmResult) {
        .type = VM_SUCCESS,
        .as.success = valueDa,
    };

    return result;
}

void VmTests() {
    PRINT_TEST_TITLE();

    RunTestCase((VmTestCase) {
        .desc = "Only nil",
        .input = "()",
        .expected = MakeSuccess((Value[]) { MAKE_VALUE_NIL() }, 1),
    });

   RunTestCase((VmTestCase) {
       .desc = "Only number",
       .input = "1",
       .expected = MakeSuccess((Value[]) { MAKE_VALUE_F64(1) }, 1),
   });

}
