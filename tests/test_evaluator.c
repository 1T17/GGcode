#include "Unity/src/unity.h"
#include "parser.h"
#include "lexer/lexer.h"
#include "../runtime/evaluator.h"
#include "../generator/emitter.h"
#include "../runtime/runtime_state.h"
#include "../config/config.h"

double get_number(Value *val); 

void setUp(void) {}
void tearDown(void) {}

void test_eval_simple_addition(void)
{
    ASTNode *root = parse_script_from_string("let x = 2 + 3");
    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(5.0, get_number(get_var("x")));  // ✅ Ensure this is correct based on Value type
    free_ast(root);
}

void test_eval_nested_expression(void)
{
    ASTNode *root = parse_script_from_string("let y = (4 + 1) * 2");
    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("y", val);

    Value *retrieved = get_var("y");
    TEST_ASSERT_NOT_NULL(retrieved);
TEST_ASSERT_EQUAL_DOUBLE(10.0, retrieved->number);


    free_ast(root);
}

void test_eval_variables_and_arithmetic(void)
{
    ASTNode *root = parse_script_from_string("let a = 10\nlet b = a - 3");

    Value *a_val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("a", a_val);

    Value *b_val = eval_expr(root->block.statements[1]->let_stmt.expr);
    set_var("b", b_val);

    Value *retrieved = get_var("b");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(7.0, retrieved->number);

    free_ast(root);
}

void test_eval_comparison_operators(void)
{
    ASTNode *root = parse_script_from_string(
        "let r1 = 5 < 10\n"
        "let r2 = 5 == 5\n"
        "let r3 = 6 > 9"
    );

    Value *r1_val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("r1", r1_val);
    Value *r2_val = eval_expr(root->block.statements[1]->let_stmt.expr);
    set_var("r2", r2_val);
    Value *r3_val = eval_expr(root->block.statements[2]->let_stmt.expr);
    set_var("r3", r3_val);

    Value *v1 = get_var("r1");
    Value *v2 = get_var("r2");
    Value *v3 = get_var("r3");

    TEST_ASSERT_NOT_NULL(v1);
    TEST_ASSERT_NOT_NULL(v2);
    TEST_ASSERT_NOT_NULL(v3);

    TEST_ASSERT_EQUAL_DOUBLE(1.0, v1->number);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, v2->number);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, v3->number);

    free_ast(root);
}

void test_eval_unary_bang(void)
{
    ASTNode *root = parse_script_from_string("let v = !!0");

    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("v", val);

    Value *retrieved = get_var("v");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, retrieved->number);

    free_ast(root);
}

void test_eval_not_equal_operator(void)
{
    ASTNode *root = parse_script_from_string("let neq = 4 != 5");

    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("neq", val);

    Value *retrieved = get_var("neq");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, retrieved->number);

    free_ast(root);
}

void test_eval_variable_dependencies(void)
{
    ASTNode *root = parse_script_from_string("let a = 3\nlet b = a + 2\nlet c = b * 3");

    Value *val_a = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("a", val_a);

    Value *val_b = eval_expr(root->block.statements[1]->let_stmt.expr);
    set_var("b", val_b);

    Value *val_c = eval_expr(root->block.statements[2]->let_stmt.expr);
    set_var("c", val_c);

    Value *retrieved = get_var("c");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(15.0, retrieved->number); // (3 + 2) * 3

    free_ast(root);
}

void test_eval_unary_bang_nonzero(void)
{
    ASTNode *root = parse_script_from_string("let val = !3");

    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("val", val);

    Value *retrieved = get_var("val");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, retrieved->number); // !3 → false → 0

    free_ast(root);
}

void test_eval_operator_precedence(void)
{
    ASTNode *root = parse_script_from_string("let x = 2 + 3 * 4");

    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("x", val);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(14.0, retrieved->number); // 2 + (3 * 4)

    free_ast(root);
}

void test_eval_chained_arithmetic(void)
{
    ASTNode *root = parse_script_from_string("let result = 10 - 2 + 1");

    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("result", val);

    Value *retrieved = get_var("result");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(9.0, retrieved->number); // (10 - 2) + 1

    free_ast(root);
}

void test_eval_parentheses_override_precedence(void)
{
    ASTNode *root = parse_script_from_string("let x = (2 + 3) * 4");

    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("x", val);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(20.0, retrieved->number); // (2 + 3) * 4

    free_ast(root);
}

void test_eval_exponentiation_basic(void)
{
    ASTNode *root = parse_script_from_string("let x = 2^3");

    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("x", val);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(8.0, retrieved->number); // 2^3 = 8

    free_ast(root);
}

void test_eval_exponentiation_precedence(void)
{
    ASTNode *root = parse_script_from_string("let x = 2 + 3^2");

    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("x", val);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(11.0, retrieved->number); // 2 + (3^2) = 2 + 9 = 11

    free_ast(root);
}

void test_eval_exponentiation_right_associative(void)
{
    ASTNode *root = parse_script_from_string("let x = 2^3^2");

    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("x", val);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(512.0, retrieved->number); // 2^(3^2) = 2^9 = 512

    free_ast(root);
}

void test_eval_exponentiation_with_multiplication(void)
{
    ASTNode *root = parse_script_from_string("let x = 2^3 * 4");

    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("x", val);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(32.0, retrieved->number); // (2^3) * 4 = 8 * 4 = 32

    free_ast(root);
}

void test_eval_deep_variable_chain(void)
{
    ASTNode *root = parse_script_from_string("let a = 2\nlet b = a + 2\nlet c = b * 2\nlet d = c - a");

    set_var("a", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("b", eval_expr(root->block.statements[1]->let_stmt.expr));
    set_var("c", eval_expr(root->block.statements[2]->let_stmt.expr));
    set_var("d", eval_expr(root->block.statements[3]->let_stmt.expr));

    Value *retrieved = get_var("d");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(6.0, retrieved->number); // ((2 + 2) * 2) - 2 = 6

    free_ast(root);
}

void test_eval_multiple_comparisons(void)
{
    ASTNode *root = parse_script_from_string("let x = 3 < 5\nlet y = 5 <= 5\nlet z = 6 > 4");

    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("y", eval_expr(root->block.statements[1]->let_stmt.expr));
    set_var("z", eval_expr(root->block.statements[2]->let_stmt.expr));

    Value *vx = get_var("x");
    Value *vy = get_var("y");
    Value *vz = get_var("z");

    TEST_ASSERT_NOT_NULL(vx);
    TEST_ASSERT_NOT_NULL(vy);
    TEST_ASSERT_NOT_NULL(vz);

    TEST_ASSERT_EQUAL_DOUBLE(1.0, vx->number);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, vy->number);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, vz->number);

    free_ast(root);
}

void test_eval_all_operators_combo(void)
{
    ASTNode *root = parse_script_from_string("let result = (5 + 3) * 2 - 4 / 2");

    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("result", val);

    Value *retrieved = get_var("result");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(14.0, retrieved->number); // ((5 + 3) * 2) - 2

    free_ast(root);
}

void test_eval_division(void)
{
    ASTNode *root = parse_script_from_string("let x = 20 / 4");
    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("x", val);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(5.0, retrieved->number);

    free_ast(root);
}

void test_eval_logical_and_or(void)
{
    ASTNode *root = parse_script_from_string("let x = 1 < 2 && 2 < 3\nlet y = 1 > 2 || 3 > 2");

    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("y", eval_expr(root->block.statements[1]->let_stmt.expr));

    Value *vx = get_var("x");
    Value *vy = get_var("y");

    TEST_ASSERT_NOT_NULL(vx);
    TEST_ASSERT_NOT_NULL(vy);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, vx->number);  // true && true
    TEST_ASSERT_EQUAL_DOUBLE(1.0, vy->number);  // false || true

    free_ast(root);
}

void test_eval_arith_and_compare_mix(void)
{
    ASTNode *root = parse_script_from_string("let r = (2 + 3) == (1 + 4)");
    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("r", val);

    Value *retrieved = get_var("r");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, retrieved->number);

    free_ast(root);
}

void test_eval_bitwise_and(void)
{
    ASTNode *root = parse_script_from_string("let x = 6 & 3");
    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("x", val);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(2.0, retrieved->number); // 6 & 3 == 2

    free_ast(root);
}

void test_eval_basic_functions(void)
{
    ASTNode *root = parse_script_from_string(
        "let a1 = abs(-5)\n"
        "let a2 = mod(10, 3)\n"
        "let a3 = floor(3.9)\n"
        "let a4 = ceil(3.1)\n"
        "let a5 = round(2.6)");

    set_var("a1", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("a2", eval_expr(root->block.statements[1]->let_stmt.expr));
    set_var("a3", eval_expr(root->block.statements[2]->let_stmt.expr));
    set_var("a4", eval_expr(root->block.statements[3]->let_stmt.expr));
    set_var("a5", eval_expr(root->block.statements[4]->let_stmt.expr));

    TEST_ASSERT_EQUAL_DOUBLE(5, get_var("a1")->number);
    TEST_ASSERT_EQUAL_DOUBLE(1, get_var("a2")->number);
    TEST_ASSERT_EQUAL_DOUBLE(3, get_var("a3")->number);
    TEST_ASSERT_EQUAL_DOUBLE(4, get_var("a4")->number);
    TEST_ASSERT_EQUAL_DOUBLE(3, get_var("a5")->number);

    free_ast(root);
}

void test_eval_geometry_helpers(void)
{
    ASTNode *root = parse_script_from_string(
        "let sq = sqrt(9)\n"
        "let pw = pow(2, 3)\n"
        "let hy = hypot(3, 4)\n"
        "let lp = lerp(0, 10, 0.5)\n"
        "let mp = map(5, 0, 10, 0, 100)\n"
        "let ds = distance(0, 0, 3, 4)");

    set_var("sq", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("pw", eval_expr(root->block.statements[1]->let_stmt.expr));
    set_var("hy", eval_expr(root->block.statements[2]->let_stmt.expr));
    set_var("lp", eval_expr(root->block.statements[3]->let_stmt.expr));
    set_var("mp", eval_expr(root->block.statements[4]->let_stmt.expr));
    set_var("ds", eval_expr(root->block.statements[5]->let_stmt.expr));

    TEST_ASSERT_EQUAL_DOUBLE(3, get_var("sq")->number);
    TEST_ASSERT_EQUAL_DOUBLE(8, get_var("pw")->number);
    TEST_ASSERT_EQUAL_DOUBLE(5, get_var("hy")->number);
    TEST_ASSERT_EQUAL_DOUBLE(5, get_var("lp")->number);
    TEST_ASSERT_EQUAL_DOUBLE(50, get_var("mp")->number);
    TEST_ASSERT_EQUAL_DOUBLE(5, get_var("ds")->number);

    free_ast(root);
}

void test_eval_advanced_functions(void)
{
    ASTNode *root = parse_script_from_string(
        "let sg = sign(-42)\n"
        "let lg = log(10)\n"
        "let ex = exp(1)");

    set_var("sg", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("lg", eval_expr(root->block.statements[1]->let_stmt.expr));
    set_var("ex", eval_expr(root->block.statements[2]->let_stmt.expr));

    TEST_ASSERT_EQUAL_DOUBLE(-1, get_var("sg")->number);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 2.302, get_var("lg")->number);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 2.718, get_var("ex")->number);

    free_ast(root);
}

void test_eval_manual_while_loop(void)
{
    set_var("i", make_number_value(0));
    double result = 0;

    while (get_var("i")->number < 3)
    {
        result += get_var("i")->number;
        set_var("i", make_number_value(get_var("i")->number + 1));
    }

    TEST_ASSERT_EQUAL_DOUBLE(3.0, result); // 0 + 1 + 2
}

void test_eval_constants(void)
{
    ASTNode *root = parse_script_from_string(
        "let pi = PI\n"
        "let tau = TAU\n"
        "let eu = EU\n"
        "let d2r = DEG_TO_RAD\n"
        "let r2d = RAD_TO_DEG");

    set_var("pi", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("tau", eval_expr(root->block.statements[1]->let_stmt.expr));
    set_var("eu", eval_expr(root->block.statements[2]->let_stmt.expr));
    set_var("d2r", eval_expr(root->block.statements[3]->let_stmt.expr));
    set_var("r2d", eval_expr(root->block.statements[4]->let_stmt.expr));

    TEST_ASSERT_FLOAT_WITHIN(0.0001, 3.14159, get_var("pi")->number);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 6.28318, get_var("tau")->number);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 2.71828, get_var("eu")->number);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 0.01745, get_var("d2r")->number);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 57.2958, get_var("r2d")->number);

    free_ast(root);
}

void test_eval_trig_functions(void)
{
    ASTNode *root = parse_script_from_string(
        "let s = sin(0)\n"
        "let c = cos(0)\n"
        "let t = tan(0)\n"
        "let a = asin(0)\n"
        "let o = acos(1)\n"
        "let n = atan(1)\n"
        "let p = atan2(1, 1)");

    set_var("s", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("c", eval_expr(root->block.statements[1]->let_stmt.expr));
    set_var("t", eval_expr(root->block.statements[2]->let_stmt.expr));
    set_var("a", eval_expr(root->block.statements[3]->let_stmt.expr));
    set_var("o", eval_expr(root->block.statements[4]->let_stmt.expr));
    set_var("n", eval_expr(root->block.statements[5]->let_stmt.expr));
    set_var("p", eval_expr(root->block.statements[6]->let_stmt.expr));

    TEST_ASSERT_EQUAL_DOUBLE(0, get_var("s")->number);
    TEST_ASSERT_EQUAL_DOUBLE(1, get_var("c")->number);
    TEST_ASSERT_EQUAL_DOUBLE(0, get_var("t")->number);
    TEST_ASSERT_EQUAL_DOUBLE(0, get_var("a")->number);
    TEST_ASSERT_EQUAL_DOUBLE(0, get_var("o")->number);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.785, get_var("n")->number); // ~π/4
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.785, get_var("p")->number);

    free_ast(root);
}

void test_eval_negative_and_unary(void)
{
    ASTNode *root = parse_script_from_string("let x = -(-5)");
    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("x", val);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(5.0, retrieved->number);

    free_ast(root);
}

void test_eval_exponentiation_complex_chain(void)
{
    // Test 2^3^4 = 2^(3^4) = 2^81 = 2417851639229258349412352
    // This is a very large number, so let's test a smaller chain: 2^2^3 = 2^(2^3) = 2^8 = 256
    ASTNode *root = parse_script_from_string("let x = 2^2^3");

    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("x", val);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(256.0, retrieved->number); // 2^(2^3) = 2^8 = 256

    free_ast(root);
}

void test_eval_exponentiation_four_levels(void)
{
    // Test 2^2^2^2 = 2^(2^(2^2)) = 2^(2^4) = 2^16 = 65536
    ASTNode *root = parse_script_from_string("let x = 2^2^2^2");

    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("x", val);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(65536.0, retrieved->number); // 2^(2^(2^2)) = 2^16 = 65536

    free_ast(root);
}

void test_eval_exponentiation_with_parentheses(void)
{
    // Test (2^3)^2 = 8^2 = 64 vs 2^(3^2) = 2^9 = 512
    ASTNode *root1 = parse_script_from_string("let x = (2^3)^2");
    ASTNode *root2 = parse_script_from_string("let y = 2^(3^2)");

    Value *val1 = eval_expr(root1->block.statements[0]->let_stmt.expr);
    set_var("x", val1);
    Value *val2 = eval_expr(root2->block.statements[0]->let_stmt.expr);
    set_var("y", val2);

    Value *x = get_var("x");
    Value *y = get_var("y");
    TEST_ASSERT_NOT_NULL(x);
    TEST_ASSERT_NOT_NULL(y);
    TEST_ASSERT_EQUAL_DOUBLE(64.0, x->number);   // (2^3)^2 = 8^2 = 64
    TEST_ASSERT_EQUAL_DOUBLE(512.0, y->number);  // 2^(3^2) = 2^9 = 512

    free_ast(root1);
    free_ast(root2);
}

void test_eval_exponentiation_mixed_operations(void)
{
    // Test complex expression: 3 + 2^3^2 * 4 - 1
    // Should be: 3 + (2^(3^2)) * 4 - 1 = 3 + (2^9) * 4 - 1 = 3 + 512 * 4 - 1 = 3 + 2048 - 1 = 2050
    ASTNode *root = parse_script_from_string("let x = 3 + 2^3^2 * 4 - 1");

    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("x", val);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(2050.0, retrieved->number);

    free_ast(root);
}

void test_eval_exponentiation_fractional_base(void)
{
    // Test 0.5^3 = 0.125
    ASTNode *root = parse_script_from_string("let x = 0.5^3");

    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("x", val);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(0.125, retrieved->number);

    free_ast(root);
}

void test_eval_exponentiation_fractional_exponent(void)
{
    // Test 4^0.5 = sqrt(4) = 2
    ASTNode *root = parse_script_from_string("let x = 4^0.5");

    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("x", val);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(2.0, retrieved->number);

    free_ast(root);
}

void test_eval_exponentiation_negative_base(void)
{
    // Test (-2)^3 = -8
    ASTNode *root = parse_script_from_string("let x = (-2)^3");

    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("x", val);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(-8.0, retrieved->number);

    free_ast(root);
}

void test_eval_exponentiation_zero_and_one(void)
{
    // Test edge cases: 0^5 = 0, 5^0 = 1, 1^100 = 1
    ASTNode *root = parse_script_from_string("let a = 0^5\nlet b = 5^0\nlet c = 1^100");

    set_var("a", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("b", eval_expr(root->block.statements[1]->let_stmt.expr));
    set_var("c", eval_expr(root->block.statements[2]->let_stmt.expr));

    Value *a = get_var("a");
    Value *b = get_var("b");
    Value *c = get_var("c");
    
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);
    TEST_ASSERT_NOT_NULL(c);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, a->number);   // 0^5 = 0
    TEST_ASSERT_EQUAL_DOUBLE(1.0, b->number);   // 5^0 = 1
    TEST_ASSERT_EQUAL_DOUBLE(1.0, c->number);   // 1^100 = 1

    free_ast(root);
}

void test_eval_exponentiation_vs_multiplication_precedence(void)
{
    // Test that ^ has higher precedence than *: 2 * 3^2 = 2 * 9 = 18 (not (2*3)^2 = 36)
    ASTNode *root = parse_script_from_string("let x = 2 * 3^2");

    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("x", val);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(18.0, retrieved->number); // 2 * (3^2) = 2 * 9 = 18

    free_ast(root);
}

void test_eval_compound_assignment_plus_equal(void)
{
    ASTNode *root = parse_script_from_string("let x = 10\nx += 5");

    // Execute let statement
    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    
    // Execute compound assignment
    eval_expr(root->block.statements[1]);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(15.0, retrieved->number); // 10 + 5 = 15

    free_ast(root);
}

void test_eval_compound_assignment_minus_equal(void)
{
    ASTNode *root = parse_script_from_string("let y = 20\ny -= 3");

    set_var("y", eval_expr(root->block.statements[0]->let_stmt.expr));
    eval_expr(root->block.statements[1]);

    Value *retrieved = get_var("y");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(17.0, retrieved->number); // 20 - 3 = 17

    free_ast(root);
}

void test_eval_compound_assignment_star_equal(void)
{
    ASTNode *root = parse_script_from_string("let z = 4\nz *= 2");

    set_var("z", eval_expr(root->block.statements[0]->let_stmt.expr));
    eval_expr(root->block.statements[1]);

    Value *retrieved = get_var("z");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(8.0, retrieved->number); // 4 * 2 = 8

    free_ast(root);
}

void test_eval_compound_assignment_slash_equal(void)
{
    ASTNode *root = parse_script_from_string("let w = 16\nw /= 4");

    set_var("w", eval_expr(root->block.statements[0]->let_stmt.expr));
    eval_expr(root->block.statements[1]);

    Value *retrieved = get_var("w");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(4.0, retrieved->number); // 16 / 4 = 4

    free_ast(root);
}

void test_eval_compound_assignment_caret_equal(void)
{
    ASTNode *root = parse_script_from_string("let p = 2\np ^= 3");

    set_var("p", eval_expr(root->block.statements[0]->let_stmt.expr));
    eval_expr(root->block.statements[1]);

    Value *retrieved = get_var("p");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(8.0, retrieved->number); // 2^3 = 8

    free_ast(root);
}

void test_eval_compound_assignment_complex_expression(void)
{
    ASTNode *root = parse_script_from_string("let a = 5\na += 2 * 3");

    set_var("a", eval_expr(root->block.statements[0]->let_stmt.expr));
    eval_expr(root->block.statements[1]);

    Value *retrieved = get_var("a");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(11.0, retrieved->number); // 5 + (2 * 3) = 11

    free_ast(root);
}

void test_eval_compound_assignment_bitwise_and_equal(void)
{
    ASTNode *root = parse_script_from_string("let x = 12\nx &= 10");

    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    eval_expr(root->block.statements[1]);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(8.0, retrieved->number); // 12 & 10 = 8 (1100 & 1010 = 1000)

    free_ast(root);
}

void test_eval_compound_assignment_bitwise_or_equal(void)
{
    ASTNode *root = parse_script_from_string("let x = 12\nx |= 10");

    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    eval_expr(root->block.statements[1]);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(14.0, retrieved->number); // 12 | 10 = 14 (1100 | 1010 = 1110)

    free_ast(root);
}

void test_eval_compound_assignment_left_shift_equal(void)
{
    ASTNode *root = parse_script_from_string("let x = 5\nx <<= 2");

    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    eval_expr(root->block.statements[1]);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(20.0, retrieved->number); // 5 << 2 = 20 (101 << 2 = 10100)

    free_ast(root);
}

void test_eval_compound_assignment_right_shift_equal(void)
{
    ASTNode *root = parse_script_from_string("let x = 20\nx >>= 2");

    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    eval_expr(root->block.statements[1]);

    Value *retrieved = get_var("x");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_DOUBLE(5.0, retrieved->number); // 20 >> 2 = 5 (10100 >> 2 = 101)

    free_ast(root);
}

void test_eval_recursion_limit_protection(void)
{
    // Test that infinite recursion is detected and handled gracefully
    const char *code = 
        "function infinite_a() { return infinite_b() }\n"
        "function infinite_b() { return infinite_a() }\n"
        "let result = infinite_a()\n";
    
    reset_runtime_state(); // Ensure clean state
    
    ASTNode *root = parse_script_from_string(code);
    TEST_ASSERT_NOT_NULL(root);
    
    // Execute the code - should not crash
    emit_gcode(root);
    
    // Verify that the result is the safe default value (0.0)
    Value *result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, result->number);
    
    // Verify that the system is in a stable state after recursion error
    Runtime *rt = get_runtime();
    TEST_ASSERT_EQUAL_INT(0, rt->recursion_depth); // Should be reset to 0
    
    free_ast(root);
}

void test_eval_recursion_recovery_and_stability(void)
{
    // Test that the system remains stable after recursion errors
    const char *infinite_code = 
        "function infinite() { return infinite() }\n"
        "let bad_result = infinite()\n";
    
    const char *normal_code = 
        "function countdown(n) {\n"
        "  if (n <= 0) { return 0 }\n"
        "  return countdown(n - 1)\n"
        "}\n"
        "let good_result = countdown(5)\n";
    
    reset_runtime_state(); // Ensure clean state
    
    // First, trigger recursion limit
    ASTNode *infinite_root = parse_script_from_string(infinite_code);
    TEST_ASSERT_NOT_NULL(infinite_root);
    emit_gcode(infinite_root);
    
    Value *bad_result = get_var("bad_result");
    TEST_ASSERT_NOT_NULL(bad_result);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, bad_result->number);
    
    free_ast(infinite_root);
    
    // Then, verify normal recursion still works
    ASTNode *normal_root = parse_script_from_string(normal_code);
    TEST_ASSERT_NOT_NULL(normal_root);
    emit_gcode(normal_root);
    
    Value *good_result = get_var("good_result");
    TEST_ASSERT_NOT_NULL(good_result);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, good_result->number); // countdown(5) should return 0
    
    // Verify system is still stable
    Runtime *rt = get_runtime();
    TEST_ASSERT_EQUAL_INT(0, rt->recursion_depth);
    
    free_ast(normal_root);
}

// String literal evaluation tests
void test_eval_string_literal_basic(void)
{
    ASTNode *root = parse_script_from_string("let message = \"hello world\"");
    
    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("message", val);
    
    Value *retrieved = get_var("message");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_INT(VAL_STRING, retrieved->type);
    TEST_ASSERT_EQUAL_STRING("hello world", retrieved->string);
    
    free_ast(root);
}

void test_eval_string_literal_empty(void)
{
    ASTNode *root = parse_script_from_string("let empty = \"\"");
    
    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("empty", val);
    
    Value *retrieved = get_var("empty");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_INT(VAL_STRING, retrieved->type);
    TEST_ASSERT_EQUAL_STRING("", retrieved->string);
    
    free_ast(root);
}

void test_eval_string_literal_with_escapes(void)
{
    ASTNode *root = parse_script_from_string("let escaped = \"He said \\\"hello\\\"\\nNext line\\tTabbed\"");
    
    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("escaped", val);
    
    Value *retrieved = get_var("escaped");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_INT(VAL_STRING, retrieved->type);
    TEST_ASSERT_EQUAL_STRING("He said \"hello\"\nNext line\tTabbed", retrieved->string);
    
    free_ast(root);
}

void test_eval_string_variable_assignment(void)
{
    ASTNode *root = parse_script_from_string("let name = \"Alice\"\nlet greeting = name");
    
    // Set first variable
    Value *name_val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("name", name_val);
    
    // Set second variable to reference first
    Value *greeting_val = eval_expr(root->block.statements[1]->let_stmt.expr);
    set_var("greeting", greeting_val);
    
    Value *name = get_var("name");
    Value *greeting = get_var("greeting");
    
    TEST_ASSERT_NOT_NULL(name);
    TEST_ASSERT_NOT_NULL(greeting);
    TEST_ASSERT_EQUAL_INT(VAL_STRING, name->type);
    TEST_ASSERT_EQUAL_INT(VAL_STRING, greeting->type);
    TEST_ASSERT_EQUAL_STRING("Alice", name->string);
    TEST_ASSERT_EQUAL_STRING("Alice", greeting->string);
    
    free_ast(root);
}

void test_eval_string_variable_reassignment(void)
{
    ASTNode *root = parse_script_from_string("let text = \"first\"\ntext = \"second\"");
    
    // Set initial value
    Value *initial_val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("text", initial_val);
    
    // Reassign
    Value *new_val = eval_expr(root->block.statements[1]->assign_stmt.expr);
    set_var("text", new_val);
    
    Value *retrieved = get_var("text");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_INT(VAL_STRING, retrieved->type);
    TEST_ASSERT_EQUAL_STRING("second", retrieved->string);
    
    free_ast(root);
}

void test_eval_string_memory_management(void)
{
    // Test that string memory is properly managed during operations
    ASTNode *root = parse_script_from_string("let str1 = \"test string\"\nlet str2 = \"another string\"");
    
    Value *val1 = eval_expr(root->block.statements[0]->let_stmt.expr);
    Value *val2 = eval_expr(root->block.statements[1]->let_stmt.expr);
    
    set_var("str1", val1);
    set_var("str2", val2);
    
    // Verify both strings are properly stored
    Value *retrieved1 = get_var("str1");
    Value *retrieved2 = get_var("str2");
    
    TEST_ASSERT_NOT_NULL(retrieved1);
    TEST_ASSERT_NOT_NULL(retrieved2);
    TEST_ASSERT_EQUAL_INT(VAL_STRING, retrieved1->type);
    TEST_ASSERT_EQUAL_INT(VAL_STRING, retrieved2->type);
    TEST_ASSERT_EQUAL_STRING("test string", retrieved1->string);
    TEST_ASSERT_EQUAL_STRING("another string", retrieved2->string);
    
    // Verify strings have different memory addresses (independent copies)
    TEST_ASSERT_NOT_EQUAL(retrieved1->string, retrieved2->string);
    
    free_ast(root);
}

void test_eval_string_copy_independence(void)
{
    // Test that copying string values creates independent copies
    Value *original = make_string_value("original text");
    Value *copy = copy_value(original);
    
    TEST_ASSERT_NOT_NULL(original);
    TEST_ASSERT_NOT_NULL(copy);
    TEST_ASSERT_EQUAL_INT(VAL_STRING, original->type);
    TEST_ASSERT_EQUAL_INT(VAL_STRING, copy->type);
    TEST_ASSERT_EQUAL_STRING("original text", original->string);
    TEST_ASSERT_EQUAL_STRING("original text", copy->string);
    
    // Verify they have different memory addresses
    TEST_ASSERT_NOT_EQUAL(original->string, copy->string);
    
    // Cleanup
    free_value(original);
    free_value(copy);
}

void test_eval_string_mixed_content(void)
{
    ASTNode *root = parse_script_from_string("let mixed = \"text with 123 numbers and symbols!@#\"");
    
    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("mixed", val);
    
    Value *retrieved = get_var("mixed");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_INT(VAL_STRING, retrieved->type);
    TEST_ASSERT_EQUAL_STRING("text with 123 numbers and symbols!@#", retrieved->string);
    
    free_ast(root);
}


int main(void)
{
    UNITY_BEGIN();
     RUN_TEST(test_eval_simple_addition);                  //1
     RUN_TEST(test_eval_nested_expression);                //2
     RUN_TEST(test_eval_variables_and_arithmetic);         //3
     RUN_TEST(test_eval_comparison_operators);             //4
     RUN_TEST(test_eval_unary_bang);                       //5
     RUN_TEST(test_eval_not_equal_operator);               //6
     RUN_TEST(test_eval_variable_dependencies);            //7
     RUN_TEST(test_eval_unary_bang_nonzero);               //8
     RUN_TEST(test_eval_chained_arithmetic);               //9
     RUN_TEST(test_eval_operator_precedence);              //10
     RUN_TEST(test_eval_parentheses_override_precedence);  //11
     RUN_TEST(test_eval_exponentiation_basic);             //12
     RUN_TEST(test_eval_exponentiation_precedence);        //13
     RUN_TEST(test_eval_exponentiation_right_associative); //14
     RUN_TEST(test_eval_exponentiation_with_multiplication);//15
     RUN_TEST(test_eval_exponentiation_complex_chain);     //16 - NEW
     RUN_TEST(test_eval_exponentiation_four_levels);       //17 - NEW
     RUN_TEST(test_eval_exponentiation_with_parentheses);  //18 - NEW
     RUN_TEST(test_eval_exponentiation_mixed_operations);  //19 - NEW
     RUN_TEST(test_eval_exponentiation_fractional_base);   //20 - NEW
     RUN_TEST(test_eval_exponentiation_fractional_exponent);//21 - NEW
     RUN_TEST(test_eval_exponentiation_negative_base);     //22 - NEW
     RUN_TEST(test_eval_exponentiation_zero_and_one);      //23 - NEW
     RUN_TEST(test_eval_exponentiation_vs_multiplication_precedence);//24 - NEW
     RUN_TEST(test_eval_compound_assignment_plus_equal);   //25 - NEW
     RUN_TEST(test_eval_compound_assignment_minus_equal);  //26 - NEW
     RUN_TEST(test_eval_compound_assignment_star_equal);   //27 - NEW
     RUN_TEST(test_eval_compound_assignment_slash_equal);  //28 - NEW
     RUN_TEST(test_eval_compound_assignment_caret_equal);  //29 - NEW
     RUN_TEST(test_eval_compound_assignment_complex_expression); //30 - NEW
     RUN_TEST(test_eval_compound_assignment_bitwise_and_equal); //31 - NEW
     RUN_TEST(test_eval_compound_assignment_bitwise_or_equal);  //32 - NEW
     RUN_TEST(test_eval_compound_assignment_left_shift_equal);  //33 - NEW
     RUN_TEST(test_eval_compound_assignment_right_shift_equal); //34 - NEW
     RUN_TEST(test_eval_all_operators_combo);              //35
     RUN_TEST(test_eval_deep_variable_chain);              //36
     RUN_TEST(test_eval_multiple_comparisons);             //37
     RUN_TEST(test_eval_division);                         //38      
     RUN_TEST(test_eval_arith_and_compare_mix);            //39
     RUN_TEST(test_eval_logical_and_or);                   //40
     RUN_TEST(test_eval_bitwise_and);                      //41
     RUN_TEST(test_eval_basic_functions);                  //42
     RUN_TEST(test_eval_geometry_helpers);                 //43
     RUN_TEST(test_eval_advanced_functions);               //44
     RUN_TEST(test_eval_constants);                        //45
     RUN_TEST(test_eval_manual_while_loop);                //46
     RUN_TEST(test_eval_trig_functions);                   //47 
     RUN_TEST(test_eval_negative_and_unary);               //48
     RUN_TEST(test_eval_recursion_limit_protection);       //49
     RUN_TEST(test_eval_recursion_recovery_and_stability); //50
     
     // String literal tests
     RUN_TEST(test_eval_string_literal_basic);             //51
     RUN_TEST(test_eval_string_literal_empty);             //52
     RUN_TEST(test_eval_string_literal_with_escapes);      //53
     RUN_TEST(test_eval_string_variable_assignment);       //54
     RUN_TEST(test_eval_string_variable_reassignment);     //55
     RUN_TEST(test_eval_string_memory_management);         //56
     RUN_TEST(test_eval_string_copy_independence);         //57
     RUN_TEST(test_eval_string_mixed_content);             //58tion);       //49 - NEW
     RUN_TEST(test_eval_recursion_recovery_and_stability); //50 - NEW
    return UNITY_END();
}
