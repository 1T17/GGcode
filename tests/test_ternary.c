#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/runtime/evaluator.h"
#include "../src/parser/parser.h"
#include "../src/runtime/runtime_state.h"
#include "Unity/src/unity.h"

void setUp(void) {
    reset_runtime_state();
}

void tearDown(void) {
    reset_runtime_state();
}

// Test basic ternary operator
void test_basic_ternary(void) {
    const char* source = "let result = 5 > 3 ? 10 : 20";
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    // Execute the script
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    // Check result
    Value* result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL(VAL_NUMBER, result->type);
    TEST_ASSERT_EQUAL_FLOAT(10.0, result->number);
}

// Test ternary with false condition
void test_ternary_false_condition(void) {
    const char* source = "let result = 2 > 5 ? 100 : 200";
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_FLOAT(200.0, result->number);
}

// Test nested ternary operators
void test_nested_ternary(void) {
    const char* source = "let result = 1 > 0 ? (2 > 1 ? 30 : 40) : 50";
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_FLOAT(30.0, result->number);
}

// Test ternary with variables
void test_ternary_with_variables(void) {
    const char* source = "let maximum = 15 > 10 ? 15 : 10";
    
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* maximum = get_var("maximum");
    TEST_ASSERT_NOT_NULL(maximum);
    TEST_ASSERT_EQUAL_FLOAT(15.0, maximum->number);
}

// Test ternary with function calls
void test_ternary_with_functions(void) {
    const char* source = "let result = abs(-5) > 3 ? sqrt(16) : pow(2, 3)";
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_FLOAT(4.0, result->number); // sqrt(16) = 4
}

// Test chained ternary operators (right-associative)
void test_chained_ternary(void) {
    // Since we don't have strings, let's use numbers: A=4, B=3, C=2, F=1
    const char* source_numeric = "let grade = 85 >= 90 ? 4 : 85 >= 80 ? 3 : 85 >= 70 ? 2 : 1";
    
    ASTNode* ast = parse_script_from_string(source_numeric);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* grade = get_var("grade");
    TEST_ASSERT_NOT_NULL(grade);
    TEST_ASSERT_EQUAL_FLOAT(3.0, grade->number); // Should be 'B' (3)
}

// Test ternary with zero/non-zero conditions
void test_ternary_zero_nonzero(void) {
    const char* source = 
        "let a = 0 ? 100 : 200\n"
        "let b = 1 ? 300 : 400\n"
        "let c = -1 ? 500 : 600";
    
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* a = get_var("a");
    Value* b = get_var("b");
    Value* c = get_var("c");
    
    TEST_ASSERT_EQUAL_FLOAT(200.0, a->number); // 0 is false
    TEST_ASSERT_EQUAL_FLOAT(300.0, b->number); // 1 is true
    TEST_ASSERT_EQUAL_FLOAT(500.0, c->number); // -1 is true
}

// Test ternary operator precedence
void test_ternary_precedence(void) {
    const char* source = "let result = 2 + 3 > 4 ? 1 * 2 : 3 + 4";
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    // 2 + 3 = 5, 5 > 4 is true, so result should be 1 * 2 = 2
    TEST_ASSERT_EQUAL_FLOAT(2.0, result->number);
}

// Test ternary in expressions
void test_ternary_in_expressions(void) {
    const char* source = "let result = (5 > 3 ? 10 : 5) + (2 < 1 ? 20 : 30)";
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    // (5 > 3 ? 10 : 5) = 10, (2 < 1 ? 20 : 30) = 30, so 10 + 30 = 40
    TEST_ASSERT_EQUAL_FLOAT(40.0, result->number);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_basic_ternary);
    RUN_TEST(test_ternary_false_condition);
    RUN_TEST(test_nested_ternary);
    RUN_TEST(test_ternary_with_variables);
    RUN_TEST(test_ternary_with_functions);
    RUN_TEST(test_chained_ternary);
    RUN_TEST(test_ternary_zero_nonzero);
    RUN_TEST(test_ternary_precedence);
    RUN_TEST(test_ternary_in_expressions);
    
    return UNITY_END();
}