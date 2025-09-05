#include "Unity/src/unity.h"
#include "../src/lexer/lexer.h"
#include "../src/parser/parser.h"
#include "../src/runtime/evaluator.h"
#include "../src/runtime/runtime_state.h"
#include "../src/generator/emitter.h"
#include "../src/error/error.h"
#include <math.h>

void setUp(void) {
    reset_runtime_state();
    clear_errors();
}

void tearDown(void) {
    reset_runtime_state();
    clear_errors();
}

void test_positive_division_by_zero(void) {
    const char* source = "let result = 5.0 / 0.0";
    
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    emit_gcode(ast);
    
    Value* result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL(VAL_NUMBER, result->type);
    TEST_ASSERT_TRUE(isinf(result->number));
    TEST_ASSERT_TRUE(result->number > 0); // Should be +∞
    
    free_ast(ast);
}

void test_negative_division_by_zero(void) {
    const char* source = "let result = -3.0 / 0.0";
    
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    emit_gcode(ast);
    
    Value* result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL(VAL_NUMBER, result->type);
    TEST_ASSERT_TRUE(isinf(result->number));
    TEST_ASSERT_TRUE(result->number < 0); // Should be -∞
    
    free_ast(ast);
}

void test_zero_division_by_zero(void) {
    const char* source = "let result = 0.0 / 0.0";
    
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    emit_gcode(ast);
    
    Value* result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL(VAL_NUMBER, result->type);
    TEST_ASSERT_TRUE(isnan(result->number)); // Should be NaN
    
    free_ast(ast);
}

void test_compound_division_by_zero(void) {
    const char* source = 
        "let x = 10.0\n"
        "x /= 0.0";
    
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    emit_gcode(ast);
    
    Value* x = get_var("x");
    TEST_ASSERT_NOT_NULL(x);
    TEST_ASSERT_EQUAL(VAL_NUMBER, x->type);
    TEST_ASSERT_TRUE(isinf(x->number));
    TEST_ASSERT_TRUE(x->number > 0); // Should be +∞
    
    free_ast(ast);
}

void test_safe_divide_function(void) {
    const char* source = "let result = safe_divide(8.0, 2.0)";
    
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    emit_gcode(ast);
    
    Value* result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL(VAL_NUMBER, result->type);
    TEST_ASSERT_EQUAL_FLOAT(4.0, result->number);
    
    free_ast(ast);
}

void test_safe_divide_by_zero_function(void) {
    const char* source = "let result = safe_divide(6.0, 0.0)";
    
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    emit_gcode(ast);
    
    Value* result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL(VAL_NUMBER, result->type);
    TEST_ASSERT_TRUE(isinf(result->number));
    TEST_ASSERT_TRUE(result->number > 0); // Should be +∞
    
    free_ast(ast);
}

void test_is_finite_function(void) {
    const char* source = 
        "let normal = is_finite(42.0)\n"
        "let inf_val = is_finite(1.0 / 0.0)\n"
        "let nan_val = is_finite(0.0 / 0.0)";
    
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    emit_gcode(ast);
    
    Value* normal = get_var("normal");
    Value* inf_val = get_var("inf_val");
    Value* nan_val = get_var("nan_val");
    
    TEST_ASSERT_NOT_NULL(normal);
    TEST_ASSERT_NOT_NULL(inf_val);
    TEST_ASSERT_NOT_NULL(nan_val);
    
    TEST_ASSERT_EQUAL_FLOAT(1.0, normal->number);  // 42.0 is finite
    TEST_ASSERT_EQUAL_FLOAT(0.0, inf_val->number); // ∞ is not finite
    TEST_ASSERT_EQUAL_FLOAT(0.0, nan_val->number); // NaN is not finite
    
    free_ast(ast);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_positive_division_by_zero);
    RUN_TEST(test_negative_division_by_zero);
    RUN_TEST(test_zero_division_by_zero);
    RUN_TEST(test_compound_division_by_zero);
    RUN_TEST(test_safe_divide_function);
    RUN_TEST(test_safe_divide_by_zero_function);
    RUN_TEST(test_is_finite_function);
    
    return UNITY_END();
}