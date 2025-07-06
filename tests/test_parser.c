#include "tests.h"
#include "tokenizer.h"
#include "parser.h"

typedef struct {
    char* desc;
    char* input;
    ParseResult expected;
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
static Token tokenBuf[PARSE_TEST_TOKEN_MAX] = {0};

static void RunTestCase(ParserTestCase testCase) {
    printf("%s\n", testCase.desc);

    InitTokenizer(testCase.input);

    TokenDa tokens = {
        .items = tokenBuf,
        .capacity = PARSE_TEST_TOKEN_MAX,
        .count = 0,
    };

    Token token = {0};
    do {
        token = ConsumeToken();
        DA_APPEND(&tokens, token);
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);

    Assert(token.type != TOKEN_ERROR, "Failed to tokenize");

    ParseResult result = ParseTokens(tokens);

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
}

#define CONS(h, t) CreateCons(h, t)
#define NIL() CreateAtom(MAKE_NIL())
#define F64(x) CreateAtom(MAKE_F64(x))
#define SYMBOL(cs) CreateAtom(MAKE_SYMBOL_CHARS(cs))
#define STRING(cs) CreateAtom(MAKE_STRING_CHARS(cs))

void ParserTests() {
    PRINT_TEST_TITLE();

    RunTestCase((ParserTestCase) {
        .desc = "Empty",
        .input = "",
        .expected = {
            .type = PARSE_ERROR,
        },
    });

    RunTestCase((ParserTestCase) {
        .desc = "Only nil",
        .input = "nil",
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
}

#undef CONS
#undef NIL
#undef F64
#undef SYMBOL
#undef STRING
