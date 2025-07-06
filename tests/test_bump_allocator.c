#include "tests.h"
#include "memory.h"

typedef struct {
    int i;
    char c;
    float f;
    double d;
} BumpMixedTestData;

typedef void (*BumpTestCaseFunc)();

typedef struct {
    char* desc;
    BumpTestCaseFunc testFn;
    size_t initialNumPages;
    size_t pageSize;
} BumpTestCase;

#define ASSERT_BUMP_STRUCT_EQUALS(expected, actual, msg) \
    Assertf(expected.i == actual.i && expected.c == actual.c \
        && expected.f == actual.f && expected.d == actual.d, \
        "%s: Expected { %d, %c, %f, %f } but received { %d, %c, %f, %f }", \
        msg, expected.i, expected.c, expected.f, expected.d, \
        actual.i, actual.c, actual.f, actual.d)

static void TestAllocateOneByte(Allocator* allocator) {
    char* b = ArenaAllocate(1, allocator);
    *b = 'a';
}

static void TestAllocateTwoBytesSequentially(Allocator* allocator) {
    char* b1 = ArenaAllocate(1, allocator);
    *b1 = 'a';
    char* b2 = ArenaAllocate(1, allocator);
    *b2 = 'b';

    size_t diff = (b2 - b1) * sizeof(char);
    Assertf(diff == sizeof(char), "Expected bytes to be sequential, but the diff was %ld bytes.", diff);
    Assert(*b1 == 'a', "Expected initial data to not be modified after second allocation.");
}

static void TestAllocateTwoStructsSequentially(Allocator* allocator) {
    BumpMixedTestData first = (BumpMixedTestData){ .i = 1, .c = 'a', .f = 1.2, .d = 3.4 };
    BumpMixedTestData* s1 = ArenaAllocate(sizeof(BumpMixedTestData), allocator);
    *s1 = first;
    BumpMixedTestData* s2 = ArenaAllocate(sizeof(BumpMixedTestData), allocator);
    *s2 = (BumpMixedTestData){ .i = 2, .c = 'b', .f = 4.5, .d = 6.7 };

    size_t diff = (s2 - s1) * sizeof(BumpMixedTestData);
    Assertf(diff == sizeof(BumpMixedTestData), "Expected structs to be sequential, but the diff was %ld bytes.", diff);

    BumpMixedTestData expected = first;
    BumpMixedTestData actual = *s1;

    ASSERT_BUMP_STRUCT_EQUALS(expected, actual, "Expected initial data to not be modified after second allocation");
}

static void TestAllocateNewPage(Allocator* allocator) {
    // page size 1
    char* b1 = ArenaAllocate(1, allocator);
    *b1 = 'a';
    char* b2 = ArenaAllocate(1, allocator);
    *b2 = 'b';

    Assert(*b1 == 'a', "Expected initial data to not be modified after second allocation.");
}

static void TestAllocateNewPageStruct(Allocator* allocator) {
    BumpMixedTestData first = (BumpMixedTestData){ .i = 1, .c = 'a', .f = 1.2, .d = 3.4 };
    BumpMixedTestData* s1 = ArenaAllocate(sizeof(BumpMixedTestData), allocator);
    *s1 = first;
    BumpMixedTestData* s2 = ArenaAllocate(sizeof(BumpMixedTestData), allocator);
    *s2 = (BumpMixedTestData){ .i = 2, .c = 'b', .f = 4.5, .d = 6.7 };

    size_t diff = (s2 - s1) * sizeof(BumpMixedTestData);
    Assertf(diff != sizeof(BumpMixedTestData), "Expected structs to NOT be sequential, but the diff was %ld bytes.", diff);

    BumpMixedTestData expected = first;
    BumpMixedTestData actual = *s1;

    ASSERT_BUMP_STRUCT_EQUALS(expected, actual, "Expected initial data to not be modified after second allocation");
}

static void TestShouldNotExceedPageSize(Allocator* allocator) {
    // page size 1
    SetAssertEnabledFromTest(false);
    void* result = ArenaAllocate(2, allocator);
    SetAssertEnabledFromTest(true);
    Assert(result == NULL, "Cannot allocate more than a page in a single call.");
}

static void TestResetAndReuse(Allocator* allocator) {
    char* b1 = ArenaAllocate(1, allocator);
    *b1 = 'a';
    char* b2 = ArenaAllocate(1, allocator);
    *b2 = 'b';

    ArenaReset(allocator);

    char* b3 = ArenaAllocate(1, allocator);
    char* b4 = ArenaAllocate(1, allocator);

    Assert(b1 == b3, "Expected first byte address to be re-used");
    Assert(b2 == b4, "Expected second byte address to be re-used");
}

static void TestResetAndReuseMultiPage(Allocator* allocator) {
    char* b1 = ArenaAllocate(1, allocator);
    *b1 = 'a';
    char* b2 = ArenaAllocate(1, allocator);
    *b2 = 'b';

    ArenaReset(allocator);

    char* b3 = ArenaAllocate(1, allocator);
    char* b4 = ArenaAllocate(1, allocator);

    Assert(b1 == b3, "Expected first byte address to be re-used");
    // second byte is in a brand new page
}

static void RunTestCase(BumpTestCase testCase) {
    printf("%s\n", testCase.desc);
    Allocator* allocator = CreateBumpAllocator(testCase.pageSize, testCase.initialNumPages);
    testCase.testFn(allocator);
    ArenaFree(allocator);
}

void BumpAllocatorTests() {
    PRINT_TEST_TITLE();

    RunTestCase((BumpTestCase) {
        .desc = "One byte",
        .testFn = &TestAllocateOneByte,
        .initialNumPages = 1,
        .pageSize = 1,
    });
    RunTestCase((BumpTestCase) {
        .desc = "Two bytes sequentially",
        .testFn = &TestAllocateTwoBytesSequentially,
        .initialNumPages = 1,
        .pageSize = 10,
    });
    RunTestCase((BumpTestCase) {
        .desc = "Two structs sequentially",
        .testFn = &TestAllocateTwoStructsSequentially,
        .initialNumPages = 1,
        .pageSize = 100,
    });
    RunTestCase((BumpTestCase) {
        .desc = "Grow by one page",
        .testFn = &TestAllocateNewPage,
        .initialNumPages = 1,
        .pageSize = 1
    });
    RunTestCase((BumpTestCase) {
        .desc = "Grow by one page - struct",
        .testFn = &TestAllocateNewPageStruct,
        .initialNumPages = 1,
        .pageSize = sizeof(BumpMixedTestData) * 2 - 1,
    });
    RunTestCase((BumpTestCase) {
        .desc = "Fail to allocate more than page size",
        .testFn = &TestShouldNotExceedPageSize,
        .initialNumPages = 1,
        .pageSize = 1
    });
    RunTestCase((BumpTestCase) {
        .desc = "Re-use reset memory",
        .testFn = &TestResetAndReuse,
        .initialNumPages = 1,
        .pageSize = 2
    });
    RunTestCase((BumpTestCase) {
        .desc = "Re-use reset memory when one page was added",
        .testFn = &TestResetAndReuseMultiPage,
        .initialNumPages = 1,
        .pageSize = 1
    });
}
