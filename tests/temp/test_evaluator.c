#include "Unity/src/unity.h"
#include "parser.h"
#include "lexer.h"
#include "../runtime/evaluator.h"
ASTNode* eval_block(ASTNode* root);

void setUp(void) {}
void tearDown(void) {}

void test_eval_simple_addition(void)
{
    ASTNode *root = parse_script("let x = 2 + 3");
    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(5.0, get_var("x"));
    free_ast(root);
}

void test_eval_nested_expression(void)
{
    ASTNode *root = parse_script("let y = (4 + 1) * 2");
    set_var("y", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(10.0, get_var("y"));
    free_ast(root);
}

void test_eval_variables_and_arithmetic(void)
{
    ASTNode *root = parse_script("let a = 10\nlet b = a - 3");
    set_var("a", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("b", eval_expr(root->block.statements[1]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(7.0, get_var("b"));
    free_ast(root);
}

void test_eval_comparison_operators(void)
{
    ASTNode *root = parse_script("let r1 = 5 < 10\nlet r2 = 5 == 5\nlet r3 = 6 > 9");
    set_var("r1", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("r2", eval_expr(root->block.statements[1]->let_stmt.expr));
    set_var("r3", eval_expr(root->block.statements[2]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("r1"));
    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("r2"));
    TEST_ASSERT_EQUAL_DOUBLE(0.0, get_var("r3"));
    free_ast(root);
}

void test_eval_unary_bang(void)
{
    ASTNode *root = parse_script("let v = !!0");
    set_var("v", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(0.0, get_var("v"));
    free_ast(root);
}

void test_eval_not_equal_operator(void)
{
    ASTNode *root = parse_script("let neq = 4 != 5");
    set_var("neq", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("neq"));
    free_ast(root);
}

void test_eval_variable_dependencies(void)
{
    ASTNode *root = parse_script("let a = 3\nlet b = a + 2\nlet c = b * 3");
    set_var("a", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("b", eval_expr(root->block.statements[1]->let_stmt.expr));
    set_var("c", eval_expr(root->block.statements[2]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(15.0, get_var("c")); // (3 + 2) * 3
    free_ast(root);
}

void test_eval_unary_bang_nonzero(void)
{
    ASTNode *root = parse_script("let val = !3");
    set_var("val", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(0.0, get_var("val")); // !3 → false → 0
    free_ast(root);
}

void test_eval_operator_precedence(void)
{
    ASTNode *root = parse_script("let x = 2 + 3 * 4");
    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(14.0, get_var("x")); // 2 + (3 * 4)
    free_ast(root);
}

void test_eval_chained_arithmetic(void)
{
    ASTNode *root = parse_script("let result = 10 - 2 + 1");
    set_var("result", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(9.0, get_var("result")); // (10 - 2) + 1
    free_ast(root);
}

void test_eval_parentheses_override_precedence(void)
{
    ASTNode *root = parse_script("let x = (2 + 3) * 4");
    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(20.0, get_var("x"));
    free_ast(root);
}

void test_eval_deep_variable_chain(void)
{
    ASTNode *root = parse_script("let a = 2\nlet b = a + 2\nlet c = b * 2\nlet d = c - a");
    set_var("a", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("b", eval_expr(root->block.statements[1]->let_stmt.expr));
    set_var("c", eval_expr(root->block.statements[2]->let_stmt.expr));
    set_var("d", eval_expr(root->block.statements[3]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(6.0, get_var("d")); // ((2 + 2) * 2) - 2 = 6
    free_ast(root);
}

void test_eval_multiple_comparisons(void)
{
    ASTNode *root = parse_script("let x = 3 < 5\nlet y = 5 <= 5\nlet z = 6 > 4");
    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("y", eval_expr(root->block.statements[1]->let_stmt.expr));
    set_var("z", eval_expr(root->block.statements[2]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("x"));
    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("y"));
    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("z"));
    free_ast(root);
}

void test_eval_all_operators_combo(void)
{
    ASTNode *root = parse_script("let result = (5 + 3) * 2 - 4 / 2");
    set_var("result", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(14.0, get_var("result")); // ((5+3)*2)-2 = 14
    free_ast(root);
}

void test_eval_division(void)
{
    ASTNode *root = parse_script("let x = 20 / 4");
    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(5.0, get_var("x"));
    free_ast(root);
}

void test_eval_logical_and_or(void)
{
    ASTNode *root = parse_script("let x = 1 < 2 && 2 < 3\nlet y = 1 > 2 || 3 > 2");
    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("y", eval_expr(root->block.statements[1]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("x")); // true && true
    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("y")); // false || true
    free_ast(root);
}

void test_eval_manual_while_loop(void)
{
    set_var("i", 0);
    double result = 0;
    while (get_var("i") < 3)
    {
        result += get_var("i");
        set_var("i", get_var("i") + 1);
    }
    TEST_ASSERT_EQUAL_DOUBLE(3.0, result); // 0 + 1 + 2
}

void test_eval_negative_and_unary(void)
{
    ASTNode *root = parse_script("let x = -(-5)");
    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(5.0, get_var("x"));
    free_ast(root);
}

void test_eval_arith_and_compare_mix(void)
{
    ASTNode *root = parse_script("let r = (2 + 3) == (1 + 4)");
    set_var("r", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("r"));
    free_ast(root);
}

void test_eval_bitwise_and(void)
{
    ASTNode *root = parse_script("let x = 6 & 3");
    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(2.0, get_var("x")); // 6 & 3 == 2
    free_ast(root);
}



























void test_eval_constants(void)
{
    ASTNode *root = parse_script(
        "let pi = PI\n"
        "let tau = TAU\n"
        "let eu = EU\n"
        "let d2r = DEG_TO_RAD\n"
        "let r2d = RAD_TO_DEG"
    );

    eval_block(root);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 3.14159, get_var("pi"));
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 6.28318, get_var("tau"));
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 2.71828, get_var("eu"));
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 0.01745, get_var("d2r"));
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 57.2958, get_var("r2d"));

    free_ast(root);
}

void test_eval_basic_functions(void)
{
    ASTNode *root = parse_script(
        "let a1 = abs(-5)\n"
        "let a2 = mod(10, 3)\n"
        "let a3 = floor(3.9)\n"
        "let a4 = ceil(3.1)\n"
        "let a5 = round(2.6)"
    );

    eval_block(root);
    TEST_ASSERT_EQUAL_DOUBLE(5, get_var("a1"));
    TEST_ASSERT_EQUAL_DOUBLE(1, get_var("a2"));
    TEST_ASSERT_EQUAL_DOUBLE(3, get_var("a3"));
    TEST_ASSERT_EQUAL_DOUBLE(4, get_var("a4"));
    TEST_ASSERT_EQUAL_DOUBLE(3, get_var("a5"));

    free_ast(root);
}

void test_eval_trig_functions(void)
{
    ASTNode *root = parse_script(
        "let s = sin(0)\n"
        "let c = cos(0)\n"
        "let t = tan(0)\n"
        "let a = asin(0)\n"
        "let o = acos(1)\n"
        "let n = atan(1)\n"
        "let p = atan2(1, 1)"
    );

    eval_block(root);
    TEST_ASSERT_EQUAL_DOUBLE(0, get_var("s"));
    TEST_ASSERT_EQUAL_DOUBLE(1, get_var("c"));
    TEST_ASSERT_EQUAL_DOUBLE(0, get_var("t"));
    TEST_ASSERT_EQUAL_DOUBLE(0, get_var("a"));
    TEST_ASSERT_EQUAL_DOUBLE(0, get_var("o"));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.785, get_var("n")); // ~π/4
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.785, get_var("p"));

    free_ast(root);
}

void test_eval_geometry_helpers(void)
{
    ASTNode *root = parse_script(
        "let sq = sqrt(9)\n"
        "let pw = pow(2, 3)\n"
        "let hy = hypot(3, 4)\n"
        "let lp = lerp(0, 10, 0.5)\n"
        "let mp = map(5, 0, 10, 0, 100)\n"
        "let ds = distance(0, 0, 3, 4)"
    );

    eval_block(root);
    TEST_ASSERT_EQUAL_DOUBLE(3, get_var("sq"));
    TEST_ASSERT_EQUAL_DOUBLE(8, get_var("pw"));
    TEST_ASSERT_EQUAL_DOUBLE(5, get_var("hy"));
    TEST_ASSERT_EQUAL_DOUBLE(5, get_var("lp"));
    TEST_ASSERT_EQUAL_DOUBLE(50, get_var("mp"));
    TEST_ASSERT_EQUAL_DOUBLE(5, get_var("ds"));

    free_ast(root);
}

void test_eval_advanced_functions(void)
{
    ASTNode *root = parse_script(
        "let sg = sign(-42)\n"
        "let lg = log(10)\n"
        "let ex = exp(1)"
    );

    eval_block(root);
    TEST_ASSERT_EQUAL_DOUBLE(-1, get_var("sg"));
    TEST_ASSERT_FLOAT_WITHIN(0.01, 2.302, get_var("lg"));
    TEST_ASSERT_FLOAT_WITHIN(0.01, 2.718, get_var("ex"));

    free_ast(root);
}














int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_eval_simple_addition);
    RUN_TEST(test_eval_nested_expression);
    RUN_TEST(test_eval_variables_and_arithmetic);
    RUN_TEST(test_eval_comparison_operators);
    RUN_TEST(test_eval_unary_bang);
    RUN_TEST(test_eval_not_equal_operator);
    RUN_TEST(test_eval_chained_arithmetic);
    RUN_TEST(test_eval_variable_dependencies);
    RUN_TEST(test_eval_unary_bang_nonzero);
    RUN_TEST(test_eval_operator_precedence);
    RUN_TEST(test_eval_division);
    RUN_TEST(test_eval_parentheses_override_precedence);
    RUN_TEST(test_eval_deep_variable_chain);
    RUN_TEST(test_eval_multiple_comparisons);
    RUN_TEST(test_eval_all_operators_combo);
    RUN_TEST(test_eval_manual_while_loop);
    RUN_TEST(test_eval_negative_and_unary);
    RUN_TEST(test_eval_arith_and_compare_mix);
    RUN_TEST(test_eval_logical_and_or);
    RUN_TEST(test_eval_bitwise_and);
    RUN_TEST(test_eval_constants);
    RUN_TEST(test_eval_basic_functions);
    RUN_TEST(test_eval_trig_functions);
    RUN_TEST(test_eval_geometry_helpers);
    RUN_TEST(test_eval_advanced_functions);


    return UNITY_END();
}
