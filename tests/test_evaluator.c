#include "Unity/src/unity.h"
#include "parser.h"
#include "lexer/lexer.h"
#include "../runtime/evaluator.h"

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
     RUN_TEST(test_eval_all_operators_combo);              //12
     RUN_TEST(test_eval_deep_variable_chain);              //13
     RUN_TEST(test_eval_multiple_comparisons);             //14
     RUN_TEST(test_eval_division);                         //15      
     RUN_TEST(test_eval_arith_and_compare_mix);            //16
     RUN_TEST(test_eval_logical_and_or);                   //17
     RUN_TEST(test_eval_bitwise_and);                      //18
     RUN_TEST(test_eval_basic_functions);                  //19
     RUN_TEST(test_eval_geometry_helpers);                 //20
     RUN_TEST(test_eval_advanced_functions);               //21
     RUN_TEST(test_eval_constants);                        //22
     RUN_TEST(test_eval_manual_while_loop);                //23
     RUN_TEST(test_eval_trig_functions);                   //24 
     RUN_TEST(test_eval_negative_and_unary);               //25
    return UNITY_END();
}
