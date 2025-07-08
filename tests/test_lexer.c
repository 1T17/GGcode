#include "Unity/src/unity.h"
#include "../src/lexer/lexer.h"
#include "../src/lexer/token_utils.h"
#include "../src/lexer/token_types.h"
#include <stdlib.h>
#include <stdio.h>

int print_tokens_logs = 0;

typedef struct
{
    Token *tokens;
    int count;
    int capacity;
} TokenList;

// Local helper to tokenize full input string
TokenList lex_source(const char *source)
{
    Lexer *lexer = lexer_new(source);
    TokenList list;
    list.count = 0;
    list.capacity = 8;
    list.tokens = malloc(sizeof(Token) * list.capacity);

    while (1)
    {
        Token token = lexer_next_token(lexer);
        if (list.count >= list.capacity)
        {
            list.capacity *= 2;
            list.tokens = realloc(list.tokens, sizeof(Token) * list.capacity);
        }
        list.tokens[list.count++] = token;
        if (token.type == TOKEN_EOF)
            break;
    }

    lexer_free(lexer);
    return list;
}

void free_token_list(TokenList *list)
{
    for (int i = 0; i < list->count; i++)
    {
        token_free(list->tokens[i]);
    }
    free(list->tokens);
    list->tokens = NULL;
    list->count = 0;
    list->capacity = 0;
}

void assert_token(Token token, Token_Type expected_type, const char *expected_value)
{
    TEST_ASSERT_EQUAL(expected_type, token.type);
    if (expected_value)
    {
        TEST_ASSERT_EQUAL_STRING(expected_value, token.value);
    }
}

void print_tokens(TokenList *tokens)
{

    if (print_tokens_logs == 1)
    {

        printf("\n%-4s %-10s %-20s %-6s %-6s\n", "Idx", "Type", "Value", "Line", "Col");
        printf("---------------------------------------------------------------\n");

        for (int i = 0; i < tokens->count; i++)
        {
            Token t = tokens->tokens[i];
            printf("%-4d %-10d %-20s %-6d %-6d\n",
                   i,
                   t.type,
                   t.value ? t.value : "NULL",
                   t.line,
                   t.column);
        }

        printf("---------------------------------------------------------------\n");
        printf("Total tokens: %d\n\n", tokens->count);
    }
}

void test_keywords_and_identifiers()
{
    TokenList tokens = lex_source("let while for function note if else G0 M3 myVar");
    print_tokens(&tokens);
    TEST_ASSERT_EQUAL(11, tokens.count); // 9 tokens + EOF

    assert_token(tokens.tokens[0], TOKEN_LET, "let");
    assert_token(tokens.tokens[1], TOKEN_WHILE, "while");
    assert_token(tokens.tokens[2], TOKEN_FOR, "for");
    assert_token(tokens.tokens[3], TOKEN_FUNCTION, "function");
    assert_token(tokens.tokens[4], TOKEN_NOTE, "note");
    assert_token(tokens.tokens[5], TOKEN_IF, "if");
    assert_token(tokens.tokens[6], TOKEN_ELSE, "else");
    assert_token(tokens.tokens[7], TOKEN_GCODE_WORD, "G0");
    assert_token(tokens.tokens[8], TOKEN_GCODE_WORD, "M3");
    assert_token(tokens.tokens[9], TOKEN_IDENTIFIER, "myVar");

    free_token_list(&tokens);
}

void test_symbols_and_operators()
{
    TokenList tokens = lex_source("+ - * / = == != < <= > >= ( ) { } [ ] , ; . .. !");
    print_tokens(&tokens);
    TEST_ASSERT_EQUAL(23, tokens.count); // 23 tokens + EOF

    assert_token(tokens.tokens[0], TOKEN_PLUS, "+");
    assert_token(tokens.tokens[1], TOKEN_MINUS, "-");
    assert_token(tokens.tokens[2], TOKEN_STAR, "*");
    assert_token(tokens.tokens[3], TOKEN_SLASH, "/");
    assert_token(tokens.tokens[4], TOKEN_EQUAL, "=");
    assert_token(tokens.tokens[5], TOKEN_EQUAL_EQUAL, "==");
    assert_token(tokens.tokens[6], TOKEN_BANG_EQUAL, "!=");
    assert_token(tokens.tokens[7], TOKEN_LESS, "<");
    assert_token(tokens.tokens[8], TOKEN_LESS_EQUAL, "<=");
    assert_token(tokens.tokens[9], TOKEN_GREATER, ">");
    assert_token(tokens.tokens[10], TOKEN_GREATER_EQUAL, ">=");
    assert_token(tokens.tokens[11], TOKEN_LPAREN, "(");
    assert_token(tokens.tokens[12], TOKEN_RPAREN, ")");
    assert_token(tokens.tokens[13], TOKEN_LBRACE, "{");
    assert_token(tokens.tokens[14], TOKEN_RBRACE, "}");
    assert_token(tokens.tokens[15], TOKEN_LBRACKET, "[");
    assert_token(tokens.tokens[16], TOKEN_RBRACKET, "]");
    assert_token(tokens.tokens[17], TOKEN_COMMA, ",");
    assert_token(tokens.tokens[18], TOKEN_SEMICOLON, ";");
    assert_token(tokens.tokens[19], TOKEN_DOT, ".");
    assert_token(tokens.tokens[20], TOKEN_DOTDOT, "..");
    assert_token(tokens.tokens[21], TOKEN_BANG, "!");

    free_token_list(&tokens);
}

void test_comments_skipped()
{
    TokenList tokens = lex_source("let // comment\n while /* multi \n line */ for \n if");
    print_tokens(&tokens);
    TEST_ASSERT_EQUAL(5, tokens.count); // let, while, for, EOF

    assert_token(tokens.tokens[0], TOKEN_LET, "let");
    assert_token(tokens.tokens[1], TOKEN_WHILE, "while");
    assert_token(tokens.tokens[2], TOKEN_FOR, "for");
    assert_token(tokens.tokens[3], TOKEN_IF, "if");

    free_token_list(&tokens);
}

void test_unknown_token()
{
    TokenList tokens = lex_source("@");
    print_tokens(&tokens);
    TEST_ASSERT_EQUAL(2, tokens.count); // @ + EOF
    TEST_ASSERT_EQUAL(TOKEN_UNKNOWN, tokens.tokens[0].type);
    free_token_list(&tokens);
}

void test_reserved_word_as_identifier()
{
    TokenList tokens = lex_source("let let = 5;");
    print_tokens(&tokens);
    TEST_ASSERT_EQUAL(6, tokens.count);
    assert_token(tokens.tokens[0], TOKEN_LET, "let");
    assert_token(tokens.tokens[1], TOKEN_LET, "let"); // or TOKEN_IDENTIFIER
    assert_token(tokens.tokens[2], TOKEN_EQUAL, "=");
    assert_token(tokens.tokens[3], TOKEN_NUMBER, "5");
    free_token_list(&tokens);
}

void test_underscored_identifiers()
{
    TokenList tokens = lex_source("_abc _123 foo_bar");
    print_tokens(&tokens);
    TEST_ASSERT_EQUAL(4, tokens.count); // 3 identifiers + EOF
    assert_token(tokens.tokens[0], TOKEN_IDENTIFIER, "_abc");
    assert_token(tokens.tokens[1], TOKEN_IDENTIFIER, "_123");
    assert_token(tokens.tokens[2], TOKEN_IDENTIFIER, "foo_bar");
    free_token_list(&tokens);
}

void test_invalid_number_formats()
{
    TokenList tokens = lex_source("123abc 1.2.3 --5");
    print_tokens(&tokens);
    TEST_ASSERT_EQUAL(7, tokens.count); // 6 real tokens + EOF

    assert_token(tokens.tokens[0], TOKEN_NUMBER, "123");
    assert_token(tokens.tokens[1], TOKEN_IDENTIFIER, "abc");
    assert_token(tokens.tokens[2], TOKEN_NUMBER, "1.23");
    assert_token(tokens.tokens[3], TOKEN_MINUS, "-");
    assert_token(tokens.tokens[4], TOKEN_MINUS, "-");
    assert_token(tokens.tokens[5], TOKEN_NUMBER, "5");
    assert_token(tokens.tokens[6], TOKEN_EOF, "EOF");

    free_token_list(&tokens);
}

void test_unterminated_multiline_comment()
{
    TokenList tokens = lex_source("let /* unclosed comment");
    print_tokens(&tokens);
    TEST_ASSERT_EQUAL(2, tokens.count); // let + EOF
    assert_token(tokens.tokens[0], TOKEN_LET, "let");
    free_token_list(&tokens);
}

void test_line_and_column_tracking()
{
    TokenList tokens = lex_source("let\nx =\n123");
    print_tokens(&tokens);
    TEST_ASSERT_EQUAL(5, tokens.count);          // let, x, =, 123, EOF
    TEST_ASSERT_EQUAL(1, tokens.tokens[0].line); // "let"
    TEST_ASSERT_EQUAL(2, tokens.tokens[1].line); // "x"
    TEST_ASSERT_EQUAL(2, tokens.tokens[2].line); // "="
    TEST_ASSERT_EQUAL(3, tokens.tokens[3].line); // "123"
    TEST_ASSERT_EQUAL(3, tokens.tokens[4].line); // "EOF"

    free_token_list(&tokens);
}

void test_line_gcode()
{
    TokenList tokens = lex_source("G45 X[66]");

    print_tokens(&tokens);
    TEST_ASSERT_EQUAL(6, tokens.count); // 5 tokens + EOF

    TEST_ASSERT_EQUAL(1, tokens.tokens[0].line); // G45
    TEST_ASSERT_EQUAL(1, tokens.tokens[1].line); // X
    TEST_ASSERT_EQUAL(1, tokens.tokens[2].line); // [
    TEST_ASSERT_EQUAL(1, tokens.tokens[3].line); // 66
    TEST_ASSERT_EQUAL(1, tokens.tokens[4].line); // ]
    TEST_ASSERT_EQUAL(1, tokens.tokens[5].line); // EOF

    free_token_list(&tokens);
}

void test_line_gcode_multi_line()
{
    TokenList tokens = lex_source("G45 X[66]\nG5");

    print_tokens(&tokens);
    TEST_ASSERT_EQUAL(7, tokens.count); // 6 tokens + EOF

    TEST_ASSERT_EQUAL(1, tokens.tokens[0].line); // G45
    TEST_ASSERT_EQUAL(1, tokens.tokens[1].line); // X
    TEST_ASSERT_EQUAL(1, tokens.tokens[2].line); // [
    TEST_ASSERT_EQUAL(1, tokens.tokens[3].line); // 66
    TEST_ASSERT_EQUAL(1, tokens.tokens[4].line); // ]
    TEST_ASSERT_EQUAL(2, tokens.tokens[5].line); // G5
    TEST_ASSERT_EQUAL(2, tokens.tokens[6].line); // EOF

    free_token_list(&tokens);
}

void test_long_identifier_and_number()
{
    const char *input = "averyveryverylongidentifier123 12345678901234567890";
    TokenList tokens = lex_source(input);
    print_tokens(&tokens);
    TEST_ASSERT_EQUAL(3, tokens.count); // identifier, number, EOF
    assert_token(tokens.tokens[0], TOKEN_IDENTIFIER, "averyveryverylongidentifier123");
    assert_token(tokens.tokens[1], TOKEN_NUMBER, "12345678901234567890");
    free_token_list(&tokens);
}

void test_long_number_literal()
{
    TokenList tokens = lex_source("99999999999999999999999999999999");
    print_tokens(&tokens);
    TEST_ASSERT_EQUAL(2, tokens.count); // number + EOF

    assert_token(tokens.tokens[0], TOKEN_NUMBER, "99999999999999999999999999999999");

    free_token_list(&tokens);
}

void test_weird_identifiers()
{
    TokenList tokens = lex_source("_ _x _abc_ ABC_123");
    print_tokens(&tokens);
    TEST_ASSERT_EQUAL(5, tokens.count); // 4 idents + EOF

    assert_token(tokens.tokens[0], TOKEN_IDENTIFIER, "_");
    assert_token(tokens.tokens[1], TOKEN_IDENTIFIER, "_x");
    assert_token(tokens.tokens[2], TOKEN_IDENTIFIER, "_abc_");
    assert_token(tokens.tokens[3], TOKEN_IDENTIFIER, "ABC_123");

    free_token_list(&tokens);
}

void test_unicode_invalid_input()
{
    TokenList tokens = lex_source("let π = 5");
    print_tokens(&tokens);

    TEST_ASSERT_EQUAL(6, tokens.count); // 5 real + EOF
    assert_token(tokens.tokens[0], TOKEN_LET, "let");
    TEST_ASSERT_EQUAL(TOKEN_UNKNOWN, tokens.tokens[1].type); // first byte of π
    TEST_ASSERT_EQUAL(TOKEN_UNKNOWN, tokens.tokens[2].type); // second byte of π
    assert_token(tokens.tokens[3], TOKEN_EQUAL, "=");
    assert_token(tokens.tokens[4], TOKEN_NUMBER, "5");
    assert_token(tokens.tokens[5], TOKEN_EOF, "EOF");

    free_token_list(&tokens);
}

void test_empty_input()
{
    TokenList tokens = lex_source("");
    print_tokens(&tokens);
    TEST_ASSERT_EQUAL(1, tokens.count);
    assert_token(tokens.tokens[0], TOKEN_EOF, "EOF");
    free_token_list(&tokens);
}

void test_only_whitespace()
{
    TokenList tokens = lex_source(" \n\t  ");
    print_tokens(&tokens);
    TEST_ASSERT_EQUAL(1, tokens.count);
    assert_token(tokens.tokens[0], TOKEN_EOF, "EOF");
    free_token_list(&tokens);
}

void test_gcode_long()
{
    TokenList tokens = lex_source("G9999");
    print_tokens(&tokens);
    assert_token(tokens.tokens[0], TOKEN_GCODE_WORD, "G9999");
    free_token_list(&tokens);
}

void test_dot_and_dotdot_disambiguation()
{
    TokenList tokens = lex_source(". .. ... .");
    print_tokens(&tokens);
    TEST_ASSERT_EQUAL(5, tokens.count); // 4 tokens + EOF

    assert_token(tokens.tokens[0], TOKEN_DOT, ".");
    assert_token(tokens.tokens[1], TOKEN_DOTDOT, "..");
    assert_token(tokens.tokens[2], TOKEN_DOTDOTDOT, "...");
    assert_token(tokens.tokens[3], TOKEN_DOT, ".");
    assert_token(tokens.tokens[4], TOKEN_EOF, "EOF");

    free_token_list(&tokens);
}

void test_numbers()
{
    TokenList tokens = lex_source("123 -456 78.9 -0.12 .5 -.5");
    print_tokens(&tokens);
    TEST_ASSERT_EQUAL(10, tokens.count);

    assert_token(tokens.tokens[0], TOKEN_NUMBER, "123");
    assert_token(tokens.tokens[1], TOKEN_MINUS, "-");
    assert_token(tokens.tokens[2], TOKEN_NUMBER, "456");
    assert_token(tokens.tokens[3], TOKEN_NUMBER, "78.9");
    assert_token(tokens.tokens[4], TOKEN_MINUS, "-");
    assert_token(tokens.tokens[5], TOKEN_NUMBER, "0.12");
    assert_token(tokens.tokens[6], TOKEN_NUMBER, ".5");
    assert_token(tokens.tokens[7], TOKEN_MINUS, "-");
    assert_token(tokens.tokens[8], TOKEN_NUMBER, ".5");

    free_token_list(&tokens);
}

void test_dot_prefixed_and_negative_floats()
{
    TokenList tokens = lex_source(".4 .. -5.6");

    print_tokens(&tokens);

    TEST_ASSERT_EQUAL(5, tokens.count); // .4, .., -5.6, EOF

    assert_token(tokens.tokens[0], TOKEN_NUMBER, ".4");
    assert_token(tokens.tokens[1], TOKEN_DOTDOT, "..");
    assert_token(tokens.tokens[2], TOKEN_MINUS, "-");
    assert_token(tokens.tokens[3], TOKEN_NUMBER, "5.6");
    assert_token(tokens.tokens[4], TOKEN_EOF, "EOF");

    free_token_list(&tokens);
}

void test_ampersand_operator()
{
    TokenList tokens = lex_source("& &&");
    print_tokens(&tokens);
    TEST_ASSERT_EQUAL(3, tokens.count); // & , && , EOF

    assert_token(tokens.tokens[0], TOKEN_AMPERSAND, "&");
    assert_token(tokens.tokens[1], TOKEN_AND, "&&");
    assert_token(tokens.tokens[2], TOKEN_EOF, "EOF");

    free_token_list(&tokens);
}

void test_builtin_constants_and_math_functions()
{
    const char *input =
        "PI TAU EU DEG_TO_RAD RAD_TO_DEG "
        "abs mod floor ceil round min max clamp "
        "sin cos tan asin acos atan atan2 deg rad "
        "sqrt pow hypot lerp map distance "
        "noise sign log exp";

    TokenList tokens = lex_source(input);

    const Token_Type expected_types[] = {
        TOKEN_FUNC_PI, TOKEN_FUNC_TAU, TOKEN_FUNC_EU,
        TOKEN_FUNC_DEG_TO_RAD, TOKEN_FUNC_RAD_TO_DEG,

        TOKEN_FUNC_ABS, TOKEN_FUNC_MOD, TOKEN_FUNC_FLOOR, TOKEN_FUNC_CEIL, TOKEN_FUNC_ROUND,
        TOKEN_FUNC_MIN, TOKEN_FUNC_MAX, TOKEN_FUNC_CLAMP,

        TOKEN_FUNC_SIN, TOKEN_FUNC_COS, TOKEN_FUNC_TAN,
        TOKEN_FUNC_ASIN, TOKEN_FUNC_ACOS, TOKEN_FUNC_ATAN, TOKEN_FUNC_ATAN2,
        TOKEN_FUNC_DEG, TOKEN_FUNC_RAD,

        TOKEN_FUNC_SQRT, TOKEN_FUNC_POW, TOKEN_FUNC_HYPOT,
        TOKEN_FUNC_LERP, TOKEN_FUNC_MAP, TOKEN_FUNC_DISTANCE,

        TOKEN_FUNC_NOISE, TOKEN_FUNC_SIGN, TOKEN_FUNC_LOG, TOKEN_FUNC_EXP};

    const char *expected_values[] = {
        "PI", "TAU", "EU", "DEG_TO_RAD", "RAD_TO_DEG",
        "abs", "mod", "floor", "ceil", "round",
        "min", "max", "clamp",
        "sin", "cos", "tan", "asin", "acos", "atan", "atan2", "deg", "rad",
        "sqrt", "pow", "hypot", "lerp", "map", "distance",
        "noise", "sign", "log", "exp"};

    int expected_count = sizeof(expected_types) / sizeof(expected_types[0]);

    // +1 for EOF
    TEST_ASSERT_EQUAL(expected_count + 1, tokens.count);

    for (int i = 0; i < expected_count; ++i)
    {
        assert_token(tokens.tokens[i], expected_types[i], expected_values[i]);
    }

    // Final EOF check
    assert_token(tokens.tokens[expected_count], TOKEN_EOF, "EOF");

    free_token_list(&tokens);
}

void test_step_and_dotdotlt()
{
    TokenList tokens = lex_source("for i = 1..<10 step 2 {}");
    print_tokens(&tokens);
    TEST_ASSERT_EQUAL(11, tokens.count); // for, i, =, 1, ..<, 10, step, 2, {, }, }, EOF

    assert_token(tokens.tokens[0], TOKEN_FOR, "for");
    assert_token(tokens.tokens[1], TOKEN_IDENTIFIER, "i");
    assert_token(tokens.tokens[2], TOKEN_EQUAL, "=");
    assert_token(tokens.tokens[3], TOKEN_NUMBER, "1");
    assert_token(tokens.tokens[4], TOKEN_DOTDOT_LT, "..<");
    assert_token(tokens.tokens[5], TOKEN_NUMBER, "10");
    assert_token(tokens.tokens[6], TOKEN_STEP, "step");
    assert_token(tokens.tokens[7], TOKEN_NUMBER, "2");
    assert_token(tokens.tokens[8], TOKEN_LBRACE, "{");
    assert_token(tokens.tokens[9], TOKEN_RBRACE, "}");
    assert_token(tokens.tokens[10], TOKEN_EOF, "EOF");

    free_token_list(&tokens);
}

// === UNITY HOOKS ===

void setUp(void) {}
void tearDown(void) {}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_keywords_and_identifiers);             // 1
    RUN_TEST(test_symbols_and_operators);                // 2
    RUN_TEST(test_comments_skipped);                     // 3
    RUN_TEST(test_unknown_token);                        // 4
    RUN_TEST(test_reserved_word_as_identifier);          // 5
    RUN_TEST(test_underscored_identifiers);              // 6
    RUN_TEST(test_line_gcode);                           // 7
    RUN_TEST(test_line_gcode_multi_line);                // 8
    RUN_TEST(test_unterminated_multiline_comment);       // 9
    RUN_TEST(test_line_and_column_tracking);             // 10
    RUN_TEST(test_long_identifier_and_number);           // 11
    RUN_TEST(test_invalid_number_formats);               // 12
    RUN_TEST(test_long_number_literal);                  // 13
    RUN_TEST(test_weird_identifiers);                    // 14
    RUN_TEST(test_unicode_invalid_input);                // 15
    RUN_TEST(test_empty_input);                          // 16
    RUN_TEST(test_only_whitespace);                      // 17
    RUN_TEST(test_dot_and_dotdot_disambiguation);        // 18
    RUN_TEST(test_gcode_long);                           // 19
    RUN_TEST(test_numbers);                              // 20
    RUN_TEST(test_dot_prefixed_and_negative_floats);     // 21
    RUN_TEST(test_ampersand_operator);                   // 22
    RUN_TEST(test_builtin_constants_and_math_functions); // 23
    RUN_TEST(test_step_and_dotdotlt);                    /// 24
    return UNITY_END();
}
