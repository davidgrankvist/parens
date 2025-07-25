#include "tests.h"
#include "da.h"
#include "tokens.h"
#include "parser.h"

typedef struct {
    char* desc;
    char* input;
    ParseResult expected;
    // configure allocator
    bool shouldUseCustomAllocator;
    Allocator* allocator;
    // callback to inspect raw memory before freeing
    bool shouldInspectAllocator;
    void (*InspectAllocator)(Allocator* allocator, ParseResult result);
} ParserTestCase;

static bool AstAtomEquals(Ast* first, Ast* second) {
    Value v1 = first->as.atom.value;
    Value v2 = second->as.atom.value;

    if (v1.type != v2.type) {
        return false;
    }

    switch (v1.type) {
        case VALUE_NIL:
            return true; // already checked the type
        case VALUE_F64:
            return v1.as.f64 == v2.as.f64;
        case VALUE_OBJECT: {
            Object* o1 = v1.as.object;
            Object* o2 = v2.as.object;
            if (o1->type != o2->type) {
                return false;
            }
            if (o1->type == OBJECT_STRING) {
                return StringEquals(o1->as.string, o2->as.string);
            } else if (o1->type == OBJECT_SYMBOL) {
                return StringEquals(o1->as.symbol, o2->as.symbol);
            }
            break;
        }
        default:
            break;
    }
    return false;
}

static bool AstEquals(Ast* first, Ast* second) {
    if (first == NULL || second == NULL) {
        return first == second;
    }

    if (first->type != second->type) {
        return false;
    }

    switch(first->type) {
        case AST_ATOM:
            return AstAtomEquals(first, second);
        case AST_CONS:
            return AstEquals(first->as.cons.head, second->as.cons.head)
                && AstEquals(first->as.cons.tail, second->as.cons.tail);
        default: break;
    }

    return false;
}

#define PARSE_TEST_TOKEN_MAX 100
#define PARSE_TEST_AST_PAGE_SIZE 100

static void RunTestCase(ParserTestCase testCase) {
    printf("%s\n", testCase.desc);

    InitTokenizer(testCase.input);

    TokenDa tokens = DA_MAKE_CAPACITY(Token, PARSE_TEST_TOKEN_MAX);

    Token token = {0};
    do {
        token = ConsumeToken();
        DA_APPEND(&tokens, token);
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);

    Assert(token.type != TOKEN_ERROR, "Failed to tokenize");

    Allocator* allocator = testCase.shouldUseCustomAllocator ?
        testCase.allocator : CreateBumpAllocator(PARSE_TEST_AST_PAGE_SIZE, 1);
    ParseResult result = ParseTokens(tokens, allocator);

    if (result.type != testCase.expected.type) {
        PRINT_TEST_FAILURE();
        if (result.type == PARSE_ERROR) {
            PrintParseResult(result);
        }
        Assertf(false, "Unexpected parse result type. Expected %s, but received %s.",
            MapParseResultTypeToStr(testCase.expected.type),
            MapParseResultTypeToStr(result.type)
        );
    }

    if (testCase.expected.type == PARSE_SUCCESS
            && !AstEquals(testCase.expected.as.success.ast, result.as.success.ast)) {
        PRINT_TEST_FAILURE();
        printf("Expected:\n");
        PrintAst(testCase.expected.as.success.ast);
        printf("Actual:\n");
        PrintAst(result.as.success.ast);

        AssertFail("Unexpected AST.");
    }

    if (testCase.shouldInspectAllocator) {
        testCase.InspectAllocator(allocator, result);
    }

    AllocatorFree(allocator);
    DA_FREE(&tokens);
}

static void AssertExpectedSymbol(Object* symbol, const char* cs, size_t length) {
    Assert(symbol->type == OBJECT_SYMBOL, "Expected a symbol object");
    Assertf(symbol->as.symbol.length == length, "Expected string length %ld, but received %ld\n", 1, symbol->as.symbol.length);
    if (!StringEquals(symbol->as.symbol, MakeString(cs))) {
        printf("Unexpected string:");
        PrintString(symbol->as.symbol);
        printf("\n");
        AssertFailf("Expected symbol string \"%s\"", cs);
    }
}

static void AssertExpectedSymbolAtom(Ast* ast, Object* expected) {
    Assertf(ast->type == AST_ATOM, "Expected atom, but received %d", ast->type);
    AstAtom* atom = &ast->as.atom;
    Assertf(atom->value.type == VALUE_OBJECT, "Expected object value, but received %d", atom->value.type);
    Assertf(atom->value.as.object == expected, "Expected object reference to be %ld, but received %ld", expected, atom->value.as.object);
}

// (a . b) = 2 symbols, 2 atoms, 1 cons
#define PARSE_TEST_SIMPLE_CONS_SIZE (sizeof(Object) * 2 + sizeof(Ast) * 3)

/*
 * This is a memory layout test for parsing with a bump allocator.
 * It makes sure that AST nodes are allocated sequentially in memory
 * and in the expected order.
 *
 * This test assumes a single allocator page, so that all nodes
 * are in a contiguous block.
 *
 * Input = "(a . b)"
 * Output = CONS(SYMBOL("a"), SYMBOL("b"))
 *
 * The traversal is in post-order (head, tail, parent).
 * Symbol values are allocated before their atom wrappers are.
 * This gives the following memory layout.
 *
 *                     PAGE 1
 * | symbol a, atom a, symbol b, atom b cons(a, b) |
 *                                      ^
 *                                      |
 *                                      Result AST pointer
 *
 * The parse result points to the beginning of the final CONS.
 */
static void TestSimpleConsIsSequentialSinglePage(Allocator* allocator, ParseResult result) {
    Assert(result.type == PARSE_SUCCESS, "Expected parse success");

    size_t totalSize = PARSE_TEST_SIMPLE_CONS_SIZE;

    Byte* lastNodeStart = (Byte*)result.as.success.ast;
    Byte* lastNodeEnd = lastNodeStart + sizeof(Ast);
    Byte* firstNodeStart = lastNodeEnd - totalSize;

    Byte* current = firstNodeStart;

    AssertExpectedSymbol((Object*)current, "a", 1);

    Object* prevSymbol = (Object*)current;
    current += sizeof(Object);
    Ast* head = (Ast*)current;
    AssertExpectedSymbolAtom((Ast*)current, prevSymbol);

    current += sizeof(Ast);
    AssertExpectedSymbol((Object*)current, "b", 1);

    prevSymbol = (Object*)current;
    current += sizeof(Object);
    Ast* tail = (Ast*)current;
    AssertExpectedSymbolAtom((Ast*)current, prevSymbol);

    current += sizeof(Ast);
    Ast* ast = (Ast*)current;
    Assertf(ast->type == AST_CONS, "Expected Cons, but received %d", ast->type);
    Assertf(ast->as.cons.head == head, "Expected head to be %ld, but received %ld", (size_t)head, (size_t)ast->as.cons.head);
    Assertf(ast->as.cons.tail == tail, "Expected tail to be %ld, but received %ld", (size_t)tail, (size_t)ast->as.cons.tail);
}

/*
 * See TestSimpleConsIsSequentialSinglePage.
 *
 * This is a modified version where the final CONS ends up on a new page.
 * In this scenario the start address is not immediately known, but can
 * be found via the head of the CONS.
 *
 * This is the memory layout.
 *
 *               PAGE 1                     PAGE 2
 * | symbol a, atom a, symbol b, atom b | cons(a, b) |
 *             ^                          ^
 *             |                          |
 *             this is the cons head      Result AST pointer
 *
 */
static void TestSimpleConsIsSequentialMultiPage(Allocator* allocator, ParseResult result) {
    Assert(result.type == PARSE_SUCCESS, "Expected parse success");

    Byte* lastNodeStart = (Byte*)result.as.success.ast; // page 2 start
    Byte* current = lastNodeStart;

    Ast* ast = (Ast*)current;
    Assertf(ast->type == AST_CONS, "Expected Cons, but received %d", ast->type);

    Byte* headStart = (Byte*)ast->as.cons.head;
    Byte* firstNodeStart = headStart - sizeof(Object); // page 1 start

    current = firstNodeStart;

    // -- Verify page 1 --

    AssertExpectedSymbol((Object*)current, "a", 1);

    Object* prevSymbol = (Object*)current;
    current += sizeof(Object);
    Ast* head = (Ast*)current;
    AssertExpectedSymbolAtom((Ast*)current, prevSymbol);

    current += sizeof(Ast);
    AssertExpectedSymbol((Object*)current, "b", 1);

    prevSymbol = (Object*)current;
    current += sizeof(Object);
    Ast* tail = (Ast*)current;
    AssertExpectedSymbolAtom((Ast*)current, prevSymbol);

    // -- Verify page 2 --

    current = lastNodeStart;
    ast = (Ast*)current;
    Assertf(ast->type == AST_CONS, "Expected Cons, but received %d", ast->type);
    Assertf(ast->as.cons.head == head, "Expected head to be %ld, but received %ld", (size_t)head, (size_t)ast->as.cons.head);
    Assertf(ast->as.cons.tail == tail, "Expected tail to be %ld, but received %ld", (size_t)tail, (size_t)ast->as.cons.tail);
}

#define DUMMY_TOKEN NULL
#define CONS(h, t) CreateCons(h, t, inputAllocator)
#define NIL() CreateAtom(MAKE_VALUE_NIL(), DUMMY_TOKEN, inputAllocator)
#define F64(x) CreateAtom(MAKE_VALUE_F64(x), DUMMY_TOKEN, inputAllocator)
#define SYMBOL(cs) CreateSymbolAtom(MakeString(cs), DUMMY_TOKEN, inputAllocator)
#define STRING(cs) CreateStringAtom(MakeString(cs), DUMMY_TOKEN, inputAllocator)

void ParserTests() {
    PRINT_TEST_TITLE();

    Allocator* inputAllocator = CreateHeapAllocator();

    RunTestCase((ParserTestCase) {
        .desc = "Empty",
        .input = "",
        .expected = {
            .type = PARSE_ERROR,
        },
    });

    RunTestCase((ParserTestCase) {
        .desc = "Only nil",
        .input = "()",
        .expected = {
            .type = PARSE_SUCCESS,
            .as.success.ast = NIL(),
        },
    });

    RunTestCase((ParserTestCase) {
        .desc = "Only number",
        .input = "1.23",
        .expected = {
            .as.success.ast = F64(1.23),
        },
    });

    RunTestCase((ParserTestCase) {
        .desc = "Only symbol",
        .input = "a",
        .expected = {
            .type = PARSE_SUCCESS,
            .as.success.ast = SYMBOL("a"),
        },
    });

    RunTestCase((ParserTestCase) {
        .desc = "Only string",
        .input = "\"hello\"",
        .expected = {
            .type = PARSE_SUCCESS,
            .as.success.ast = STRING("hello"),
        },
    });

    RunTestCase((ParserTestCase) {
        .desc = "Simple cons",
        .input = "(a . b)",
        .expected = {
            .type = PARSE_SUCCESS,
            .as.success.ast = CONS(SYMBOL("a"), SYMBOL("b")),
        },
    });

    RunTestCase((ParserTestCase) {
        .desc = "Nested cons",
        .input = "((1 . 2) . (3 . (4 . 5)))",
        .expected = {
            .type = PARSE_SUCCESS,
            .as.success.ast = CONS(
                CONS(F64(1), F64(2)),
                CONS(
                    F64(3),
                    CONS(F64(4), F64(5))
                )
            ),
        },
    });

    RunTestCase((ParserTestCase) {
        .desc = "Simple proper list",
        .input = "(1 2)",
        .expected = {
            .type = PARSE_SUCCESS,
            .as.success.ast = CONS(F64(1), CONS(F64(2), NIL())),
        },
    });

    RunTestCase((ParserTestCase) {
        .desc = "Nested proper list",
        .input = "((1 2) (3 (4 5)))",
        .expected = {
            .type = PARSE_SUCCESS,
            .as.success.ast = CONS(
                /*
                 * (1 2) = (1 . (2 . nil))
                 */
                CONS(F64(1), CONS(F64(2), NIL())),
                // append nil after (3 (4 5))
                CONS(
                    /*
                     * (3 (4 5)) = (3 . ((4 . (5 . nil)) . nil)
                     */
                    CONS(
                        F64(3),
                        CONS(
                            CONS(F64(4), CONS(F64(5), NIL())),
                            NIL()
                        )
                    ),
                    NIL()
                )
            ),
        },
    });

    RunTestCase((ParserTestCase) {
        .desc = "Inspect arena memory - Simple cons, single page",
        .input = "(a . b)",
        .expected = {
            .type = PARSE_SUCCESS,
            .as.success.ast = CONS(SYMBOL("a"), SYMBOL("b")),
        },
        .shouldUseCustomAllocator = true,
        .allocator = CreateBumpAllocator(PARSE_TEST_SIMPLE_CONS_SIZE, 1),
        .shouldInspectAllocator = true,
        .InspectAllocator = &TestSimpleConsIsSequentialSinglePage,
    });

    RunTestCase((ParserTestCase) {
        .desc = "Inspect arena memory - Simple cons, two pages",
        .input = "(a . b)",
        .expected = {
            .type = PARSE_SUCCESS,
            .as.success.ast = CONS(SYMBOL("a"), SYMBOL("b")),
        },
        .shouldUseCustomAllocator = true,
        // -1 to put the last allocated node on a new page
        .allocator = CreateBumpAllocator(PARSE_TEST_SIMPLE_CONS_SIZE - 1, 2),
        .shouldInspectAllocator = true,
        .InspectAllocator = &TestSimpleConsIsSequentialMultiPage,
    });

    AllocatorFree(inputAllocator);
}

#undef CONS
#undef NIL
#undef F64
#undef SYMBOL
#undef STRING
