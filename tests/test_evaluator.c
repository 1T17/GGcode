#include "Unity/src/unity.h"
#include "parser.h"
#include "lexer.h"
#include "../runtime/evaluator.h"

void setUp(void) {}
void tearDown(void) {}

void test_eval_simple_addition(void) {
    ASTNode* root = parse_script("let x = 2 + 3");
    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(5.0, get_var("x"));
    free_ast(root);
}

void test_eval_nested_expression(void) {
    ASTNode* root = parse_script("let y = (4 + 1) * 2");
    set_var("y", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(10.0, get_var("y"));
    free_ast(root);
}

void test_eval_variables_and_arithmetic(void) {
    ASTNode* root = parse_script("let a = 10\nlet b = a - 3");
    set_var("a", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("b", eval_expr(root->block.statements[1]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(7.0, get_var("b"));
    free_ast(root);
}

void test_eval_comparison_operators(void) {
    ASTNode* root = parse_script("let r1 = 5 < 10\nlet r2 = 5 == 5\nlet r3 = 6 > 9");
    set_var("r1", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("r2", eval_expr(root->block.statements[1]->let_stmt.expr));
    set_var("r3", eval_expr(root->block.statements[2]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("r1"));
    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("r2"));
    TEST_ASSERT_EQUAL_DOUBLE(0.0, get_var("r3"));
    free_ast(root);
}

void test_eval_unary_bang(void) {
    ASTNode* root = parse_script("let v = !!0");
    set_var("v", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(0.0, get_var("v"));
    free_ast(root);
}








void test_eval_not_equal_operator(void) {
    ASTNode* root = parse_script("let neq = 4 != 5");
    set_var("neq", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("neq"));
    free_ast(root);
}




void test_eval_variable_dependencies(void) {
    ASTNode* root = parse_script("let a = 3\nlet b = a + 2\nlet c = b * 3");
    set_var("a", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("b", eval_expr(root->block.statements[1]->let_stmt.expr));
    set_var("c", eval_expr(root->block.statements[2]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(15.0, get_var("c"));  // (3 + 2) * 3
    free_ast(root);
}


void test_eval_unary_bang_nonzero(void) {
    ASTNode* root = parse_script("let val = !3");
    set_var("val", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(0.0, get_var("val"));  // !3 → false → 0
    free_ast(root);
}

void test_eval_operator_precedence(void) {
    ASTNode* root = parse_script("let x = 2 + 3 * 4");
    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(14.0, get_var("x"));  // 2 + (3 * 4)
    free_ast(root);
}




void test_eval_chained_arithmetic(void) {
    ASTNode* root = parse_script("let result = 10 - 2 + 1");
    set_var("result", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(9.0, get_var("result"));  // (10 - 2) + 1
    free_ast(root);
}



void test_eval_parentheses_override_precedence(void) {
    ASTNode* root = parse_script("let x = (2 + 3) * 4");
    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(20.0, get_var("x"));
    free_ast(root);
}




void test_eval_deep_variable_chain(void) {
    ASTNode* root = parse_script("let a = 2\nlet b = a + 2\nlet c = b * 2\nlet d = c - a");
    set_var("a", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("b", eval_expr(root->block.statements[1]->let_stmt.expr));
    set_var("c", eval_expr(root->block.statements[2]->let_stmt.expr));
    set_var("d", eval_expr(root->block.statements[3]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(6.0, get_var("d"));  // ((2 + 2) * 2) - 2 = 6
    free_ast(root);
}












void test_eval_multiple_comparisons(void) {
    ASTNode* root = parse_script("let x = 3 < 5\nlet y = 5 <= 5\nlet z = 6 > 4");
    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("y", eval_expr(root->block.statements[1]->let_stmt.expr));
    set_var("z", eval_expr(root->block.statements[2]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("x"));
    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("y"));
    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("z"));
    free_ast(root);
}





void test_eval_all_operators_combo(void) {
    ASTNode* root = parse_script("let result = (5 + 3) * 2 - 4 / 2");
    set_var("result", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(14.0, get_var("result"));  // ((5+3)*2)-2 = 14
    free_ast(root);
}







void test_eval_division(void) {
    ASTNode* root = parse_script("let x = 20 / 4");
    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(5.0, get_var("x"));
    free_ast(root);
}






void test_eval_logical_and_or(void) {
    ASTNode* root = parse_script("let x = 1 < 2 && 2 < 3\nlet y = 1 > 2 || 3 > 2");
    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    set_var("y", eval_expr(root->block.statements[1]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("x"));  // true && true
    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("y"));  // false || true
    free_ast(root);
}



void test_eval_manual_while_loop(void) {
    set_var("i", 0);
    double result = 0;
    while (get_var("i") < 3) {
        result += get_var("i");
        set_var("i", get_var("i") + 1);
    }
    TEST_ASSERT_EQUAL_DOUBLE(3.0, result);  // 0 + 1 + 2
}


void test_eval_negative_and_unary(void) {
    ASTNode* root = parse_script("let x = -(-5)");
    set_var("x", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(5.0, get_var("x"));
    free_ast(root);
}


void test_eval_arith_and_compare_mix(void) {
    ASTNode* root = parse_script("let r = (2 + 3) == (1 + 4)");
    set_var("r", eval_expr(root->block.statements[0]->let_stmt.expr));
    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("r"));
    free_ast(root);
}



















int main(void) {
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






    return UNITY_END();
} 


