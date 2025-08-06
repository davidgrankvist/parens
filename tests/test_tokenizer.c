#include "tests.h"
#include "tokens.h"

typedef struct {
    char* desc;
    char* input;
    TokenType* expected;
    size_t numExpected;
} TokenizerTestCase;

static void RunTestCase(TokenizerTestCase testCase) {
    printf("%s\n", testCase.desc);

    InitTokenizer(testCase.input);
    Token token = {0};

    int iToken = 0;
    do {
        token = ConsumeToken();
        TokenType expectedType = testCase.expected[iToken];

        Assertf(expectedType == token.type, "Expected token %d to be %s, but received %s.", iToken, MapTokenTypeToStr(expectedType), MapTokenTypeToStr(token.type));

        iToken++;
    } while (token.type != TOKEN_EOF
            && iToken < testCase.numExpected);
    int actualNumTokens = iToken;

    Assertf(actualNumTokens == testCase.numExpected, "Expected %ld tokens, but received %d.", testCase.numExpected, actualNumTokens);
}

void TokenizerTests() {
    PRINT_TEST_TITLE();

    RunTestCase((TokenizerTestCase){
        .desc = "Only EOF",
        .input = "",
        .expected = (TokenType[]){ TOKEN_EOF },
        .numExpected = 1,
    });

    RunTestCase((TokenizerTestCase){
        .desc = "Only nil",
        .input = "()",
        .expected = (TokenType[]){ TOKEN_NIL, TOKEN_EOF },
        .numExpected = 2,
    });

    RunTestCase((TokenizerTestCase){
        .desc = "Only quote",
        .input = "'",
        .expected = (TokenType[]){ TOKEN_QUOTE, TOKEN_EOF },
        .numExpected = 2,
    });

    RunTestCase((TokenizerTestCase){
        .desc = "List of numbers",
        .input = "(1 2 3)",
        .expected = (TokenType[]){
            TOKEN_PAREN_START, TOKEN_NUMBER, TOKEN_NUMBER,
            TOKEN_NUMBER, TOKEN_PAREN_END, TOKEN_EOF
        },
        .numExpected = 6,
    });

    RunTestCase((TokenizerTestCase){
        .desc = "Mixed list",
        .input = "(1 asdf ())",
        .expected = (TokenType[]){
            TOKEN_PAREN_START, TOKEN_NUMBER, TOKEN_SYMBOL,
            TOKEN_NIL, TOKEN_PAREN_END, TOKEN_EOF
        },
        .numExpected = 6,
    });

    RunTestCase((TokenizerTestCase){
        .desc = "Cons",
        .input = "a . b",
        .expected = (TokenType[]){
            TOKEN_SYMBOL, TOKEN_CONS, TOKEN_SYMBOL, TOKEN_EOF },
        .numExpected = 4,
    });

    RunTestCase((TokenizerTestCase){
        .desc = "List with string",
        .input = "(1 \"hello\" ())",
        .expected = (TokenType[]){
            TOKEN_PAREN_START, TOKEN_NUMBER, TOKEN_STRING,
            TOKEN_NIL, TOKEN_PAREN_END, TOKEN_EOF
        },
        .numExpected = 6,
    });

    RunTestCase((TokenizerTestCase){
        .desc = "Unexpected character",
        .input = "a 1 , 2 3",
        .expected = (TokenType[]){
            TOKEN_SYMBOL, TOKEN_NUMBER, TOKEN_ERROR
        },
        .numExpected = 3,
    });

    RunTestCase((TokenizerTestCase){
        .desc = "Unterminated string",
        .input = "(1 \"hello ())",
        .expected = (TokenType[]){
            TOKEN_PAREN_START, TOKEN_NUMBER, TOKEN_ERROR
        },
        .numExpected = 3,
    });

    RunTestCase((TokenizerTestCase){
        .desc = "Set variable",
        .input = "(set x 1)",
        .expected = (TokenType[]){
            TOKEN_PAREN_START, TOKEN_SET, TOKEN_SYMBOL, TOKEN_NUMBER, TOKEN_PAREN_END
        },
        .numExpected = 5,
    });

    RunTestCase((TokenizerTestCase){
        .desc = "Anonymous function",
        .input = "(fun (1) (2))",
        .expected = (TokenType[]){
            TOKEN_PAREN_START,
            TOKEN_FUN,
            TOKEN_PAREN_START, TOKEN_NUMBER, TOKEN_PAREN_END,
            TOKEN_PAREN_START, TOKEN_NUMBER, TOKEN_PAREN_END,
            TOKEN_PAREN_END,
        },
        .numExpected = 9,
    });

    RunTestCase((TokenizerTestCase){
        .desc = "Function declaration",
        .input = "(defun (1) (2))",
        .expected = (TokenType[]){
            TOKEN_PAREN_START,
            TOKEN_DEFUN,
            TOKEN_PAREN_START, TOKEN_NUMBER, TOKEN_PAREN_END,
            TOKEN_PAREN_START, TOKEN_NUMBER, TOKEN_PAREN_END,
            TOKEN_PAREN_END,
        },
        .numExpected = 9,
    });
}
