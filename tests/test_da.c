#include "tests.h"
#include "da.h"

DA_DECLARE(int);
typedef intDa IntDa;

typedef enum {
    DA_TEST_OP_ADD,
    DA_TEST_OP_REMOVE,
    DA_TEST_OP_FREE,
} TestDaOperationType;

typedef struct {
    TestDaOperationType type;
    union {
        int toAdd;
        size_t removeIndex;
    } as;
} TestDaOperation;

typedef struct {
    char* desc;
    TestDaOperation* operations;
    size_t opCount;
    size_t initCapacity;
    bool checkCapacity;
    bool hasCustomCapacity;
    IntDa expected;
} DaTestCase;

#define DA_TEST_DEFAULT_CAPACITY 10

static bool IntDaEquals(IntDa expected, IntDa actual, bool checkCapacity) {
    if (expected.count != actual.count) {
        return false;
    }
    if (checkCapacity && expected.capacity != actual.capacity) {
        return false;
    }
    if (expected.items == NULL || actual.items == NULL) {
        return expected.items == actual.items;
    }
    for (int i = 0; i < expected.count; i++) {
        if (expected.items[i] != actual.items[i]) {
            return false;
        }
    }
    return true;
}

static void PrintIntDa(const char* prefix, IntDa da) {
    printf("%s: count=%ld, capacity=%ld, items=", prefix, da.count, da.capacity);
    if (da.items == NULL) {
       printf("NULL ");
    }
    for (int i = 0; i < da.count && da.items != NULL; i++) {
        printf("%d ", da.items[i]);
    }
}

static void RunTestCase(DaTestCase testCase) {
    printf("%s\n", testCase.desc);

    size_t cap = testCase.hasCustomCapacity ? testCase.initCapacity : DA_TEST_DEFAULT_CAPACITY;
    IntDa da = DA_MAKE_CAPACITY(int, cap);

    for (int i = 0; i < testCase.opCount; i++) {
        TestDaOperation op = testCase.operations[i];
        switch (op.type) {
            case DA_TEST_OP_ADD:
                DA_APPEND(&da, op.as.toAdd);
                break;
            case DA_TEST_OP_REMOVE:
                DA_REMOVE_UNORDERED(&da, op.as.removeIndex);
                break;
            case DA_TEST_OP_FREE:
                DA_FREE(&da);
                break;
        }
    }

    if (IntDaEquals(testCase.expected, da, testCase.checkCapacity)) {
        return;
    }
    PRINT_TEST_FAILURE();

    PrintIntDa("Expected", testCase.expected);
    printf("\n");
    PrintIntDa("Actual", da);
    printf("\n");
    AssertFail("Unexpected DA result");
}

void DynamicArrayTests() {
    PRINT_TEST_TITLE();

    RunTestCase((DaTestCase) {
        .desc = "Empty",
        .expected = {
            .items = (int[]) {},
            .count = 0,
            .capacity = DA_TEST_DEFAULT_CAPACITY,
        }
    });

    RunTestCase((DaTestCase) {
        .desc = "Add single",
        .operations = (TestDaOperation[]) {
            { .type = DA_TEST_OP_ADD, .as.toAdd = 1234, }
        },
        .opCount = 1,
        .expected = {
            .items = (int[]) { 1234 },
            .count = 1,
            .capacity = DA_TEST_DEFAULT_CAPACITY,
        }
    });

    RunTestCase((DaTestCase) {
        .desc = "Add multiple",
        .operations = (TestDaOperation[]) {
            { .type = DA_TEST_OP_ADD, .as.toAdd = 1, },
            { .type = DA_TEST_OP_ADD, .as.toAdd = 2, },
            { .type = DA_TEST_OP_ADD, .as.toAdd = 3, },
        },
        .opCount = 3,
        .expected = {
            .items = (int[]) { 1, 2, 3 },
            .count = 3,
            .capacity = DA_TEST_DEFAULT_CAPACITY,
        }
    });

    RunTestCase((DaTestCase) {
        .desc = "Add beyond capacity",
        .operations = (TestDaOperation[]) {
            { .type = DA_TEST_OP_ADD, .as.toAdd = 1, },
            { .type = DA_TEST_OP_ADD, .as.toAdd = 2, },
            { .type = DA_TEST_OP_ADD, .as.toAdd = 3, },
        },
        .opCount = 3,
        .initCapacity = 2,
        .hasCustomCapacity = true,
        .expected = {
            .items = (int[]) { 1, 2, 3 },
            .count = 3,
            .capacity = DA_TEST_DEFAULT_CAPACITY,
        }
    });

    RunTestCase((DaTestCase) {
        .desc = "Remove unordered",
        .operations = (TestDaOperation[]) {
            { .type = DA_TEST_OP_ADD, .as.toAdd = 1, },
            { .type = DA_TEST_OP_ADD, .as.toAdd = 2, },
            { .type = DA_TEST_OP_ADD, .as.toAdd = 3, },
            { .type = DA_TEST_OP_REMOVE, .as.removeIndex = 0, },
        },
        .opCount = 4,
        .expected = {
            .items = (int[]) { 3, 2 },
            .count = 2,
            .capacity = DA_TEST_DEFAULT_CAPACITY,
        }
    });

    RunTestCase((DaTestCase) {
        .desc = "Free memory",
        .operations = (TestDaOperation[]) {
            { .type = DA_TEST_OP_ADD, .as.toAdd = 1234, },
            { .type = DA_TEST_OP_FREE, },
        },
        .opCount = 2,
        .checkCapacity = true,
        .expected = {
            .items = NULL,
            .count = 0,
            .capacity = 0,
        }
    });
}
