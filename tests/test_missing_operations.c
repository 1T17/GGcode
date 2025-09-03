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

// Test basic bitwise OR operator
void test_bitwise_or_operator(void) {
    const char* source = "let result = 12 | 10";
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL(VAL_NUMBER, result->type);
    // 12 (1100) | 10 (1010) = 14 (1110)
    TEST_ASSERT_EQUAL_FLOAT(14.0, result->number);
}

// Test left shift operator
void test_left_shift_operator(void) {
    const char* source = "let result = 5 << 2";
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    // 5 (101) << 2 = 20 (10100)
    TEST_ASSERT_EQUAL_FLOAT(20.0, result->number);
}

// Test right shift operator
void test_right_shift_operator(void) {
    const char* source = "let result = 20 >> 2";
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    // 20 (10100) >> 2 = 5 (101)
    TEST_ASSERT_EQUAL_FLOAT(5.0, result->number);
}

// Test bitwise operations with variables
void test_bitwise_operations_with_variables(void) {
    const char* source = 
        "let a = 15\n"
        "let b = 7\n"
        "let and_result = a & b\n"
        "let or_result = a | b\n"
        "let left_shift = a << 1\n"
        "let right_shift = a >> 1";
    
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* and_result = get_var("and_result");
    Value* or_result = get_var("or_result");
    Value* left_shift = get_var("left_shift");
    Value* right_shift = get_var("right_shift");
    
    TEST_ASSERT_NOT_NULL(and_result);
    TEST_ASSERT_NOT_NULL(or_result);
    TEST_ASSERT_NOT_NULL(left_shift);
    TEST_ASSERT_NOT_NULL(right_shift);
    
    // 15 (1111) & 7 (0111) = 7 (0111)
    TEST_ASSERT_EQUAL_FLOAT(7.0, and_result->number);
    // 15 (1111) | 7 (0111) = 15 (1111)
    TEST_ASSERT_EQUAL_FLOAT(15.0, or_result->number);
    // 15 << 1 = 30
    TEST_ASSERT_EQUAL_FLOAT(30.0, left_shift->number);
    // 15 >> 1 = 7
    TEST_ASSERT_EQUAL_FLOAT(7.0, right_shift->number);
}

// Test bitwise operations precedence
void test_bitwise_operations_precedence(void) {
    const char* source = "let result = 8 + 4 & 12";
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    // Should be (8 + 4) & 12 = 12 & 12 = 12
    TEST_ASSERT_EQUAL_FLOAT(12.0, result->number);
}

// Test shift operations with edge cases
void test_shift_operations_edge_cases(void) {
    const char* source = 
        "let zero_shift = 10 << 0\n"
        "let large_shift = 1 << 4\n"
        "let zero_right = 0 >> 2";
    
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* zero_shift = get_var("zero_shift");
    Value* large_shift = get_var("large_shift");
    Value* zero_right = get_var("zero_right");
    
    TEST_ASSERT_NOT_NULL(zero_shift);
    TEST_ASSERT_NOT_NULL(large_shift);
    TEST_ASSERT_NOT_NULL(zero_right);
    
    TEST_ASSERT_EQUAL_FLOAT(10.0, zero_shift->number);  // 10 << 0 = 10
    TEST_ASSERT_EQUAL_FLOAT(16.0, large_shift->number); // 1 << 4 = 16
    TEST_ASSERT_EQUAL_FLOAT(0.0, zero_right->number);   // 0 >> 2 = 0
}

// Test complex bitwise expressions
void test_complex_bitwise_expressions(void) {
    const char* source = "let result = (8 | 4) & (15 | 0)";
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    // (8 | 4) = 12, (15 | 0) = 15, 12 & 15 = 12
    TEST_ASSERT_EQUAL_FLOAT(12.0, result->number);
}

// Test bitwise operations in ternary expressions
void test_bitwise_in_ternary(void) {
    const char* source = "let result = (5 & 3) > 0 ? (8 | 4) : (16 >> 2)";
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    // (5 & 3) = 1, 1 > 0 is true, so result = (8 | 4) = 12
    TEST_ASSERT_EQUAL_FLOAT(12.0, result->number);
}

// Test all compound assignments together
void test_all_compound_assignments(void) {
    const char* source = 
        "let a = 10\n"
        "a += 5\n"    // a = 15
        "a -= 3\n"    // a = 12
        "a *= 2\n"    // a = 24
        "a /= 3\n"    // a = 8
        "a ^= 2\n"    // a = 64
        "a &= 15\n"   // a = 0 (64 & 15 = 0)
        "a |= 7\n"    // a = 7
        "a <<= 1\n"   // a = 14
        "a >>= 2";    // a = 3
    
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* a = get_var("a");
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_EQUAL_FLOAT(3.0, a->number);
}

// Test operator precedence with all operators
void test_comprehensive_operator_precedence(void) {
    const char* source = "let result = 2 + 3 * 4 > 10 ? 1 << 2 : 8 >> 1";
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    // 2 + 3 * 4 = 2 + 12 = 14, 14 > 10 is true, so result = 1 << 2 = 4
    TEST_ASSERT_EQUAL_FLOAT(4.0, result->number);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_bitwise_or_operator);
    RUN_TEST(test_left_shift_operator);
    RUN_TEST(test_right_shift_operator);
    RUN_TEST(test_bitwise_operations_with_variables);
    RUN_TEST(test_bitwise_operations_precedence);
    RUN_TEST(test_shift_operations_edge_cases);
    RUN_TEST(test_complex_bitwise_expressions);
    RUN_TEST(test_bitwise_in_ternary);
    RUN_TEST(test_all_compound_assignments);
    RUN_TEST(test_comprehensive_operator_precedence);
    
    return UNITY_END();
}