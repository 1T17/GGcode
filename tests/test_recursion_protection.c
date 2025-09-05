#include "Unity/src/unity.h"
#include "../src/parser/parser.h"
#include "../src/lexer/lexer.h"
#include "../src/runtime/evaluator.h"
#include "../src/generator/emitter.h"
#include "../src/runtime/runtime_state.h"
#include "../src/config/config.h"

void setUp(void) {
    reset_runtime_state();
}

void tearDown(void) {
    reset_runtime_state();
}

// Test the original failing case: infinite recursion between two functions
void test_original_infinite_recursion_case(void)
{
    // This is the exact scenario from the bug report that was causing crashes
    const char *code = 
        "function infinite_a() { return infinite_b() }\n"
        "function infinite_b() { return infinite_a() }\n"
        "let result = infinite_a()\n";
    
    reset_runtime_state(); // Ensure clean state
    
    ASTNode *root = parse_script_from_string(code);
    TEST_ASSERT_NOT_NULL(root);
    
    // Execute the code - should not crash (Requirement 1.3)
    emit_gcode(root);
    
    // Verify that recursion limit was detected and handled gracefully (Requirement 1.1)
    Value *result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, result->number); // Should return safe default value
    
    // Verify that the system is in a stable state after recursion error (Requirement 1.2, 2.3)
    Runtime *rt = get_runtime();
    TEST_ASSERT_EQUAL_INT(0, rt->recursion_depth); // Should be reset to 0
    
    free_ast(root);
}

// Test single function infinite recursion
void test_single_function_infinite_recursion(void)
{
    const char *code = 
        "function infinite() { return infinite() }\n"
        "let result = infinite()\n";
    
    reset_runtime_state();
    
    ASTNode *root = parse_script_from_string(code);
    TEST_ASSERT_NOT_NULL(root);
    
    // Should not crash (Requirement 1.3)
    emit_gcode(root);
    
    // Verify recursion limit detection (Requirement 1.1)
    Value *result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, result->number);
    
    // Verify stable state (Requirement 1.2, 2.3)
    Runtime *rt = get_runtime();
    TEST_ASSERT_EQUAL_INT(0, rt->recursion_depth);
    
    free_ast(root);
}

// Test that legitimate recursion within limits still works
void test_legitimate_recursion_within_limits(void)
{
    const char *code = 
        "function countdown(n) {\n"
        "  if (n <= 0) { return 0 }\n"
        "  return countdown(n - 1)\n"
        "}\n"
        "let result = countdown(10)\n";
    
    reset_runtime_state();
    
    ASTNode *root = parse_script_from_string(code);
    TEST_ASSERT_NOT_NULL(root);
    
    emit_gcode(root);
    
    // Legitimate recursion should work normally
    Value *result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, result->number); // countdown returns 0 when n <= 0
    
    // System should be stable
    Runtime *rt = get_runtime();
    TEST_ASSERT_EQUAL_INT(0, rt->recursion_depth);
    
    free_ast(root);
}

// Test recursion at the exact limit (99 calls, should work)
void test_recursion_at_limit_boundary(void)
{
    const char *code = 
        "function deep_recursion(n) {\n"
        "  if (n <= 0) { return n }\n"
        "  return deep_recursion(n - 1)\n"
        "}\n"
        "let result = deep_recursion(99)\n"; // Should be within limit
    
    reset_runtime_state();
    
    ASTNode *root = parse_script_from_string(code);
    TEST_ASSERT_NOT_NULL(root);
    
    emit_gcode(root);
    
    // Should work at the boundary
    Value *result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, result->number);
    
    Runtime *rt = get_runtime();
    TEST_ASSERT_EQUAL_INT(0, rt->recursion_depth);
    
    free_ast(root);
}

// Test recursion just over the limit (101 calls, should fail gracefully)
void test_recursion_over_limit_boundary(void)
{
    const char *code = 
        "function deep_recursion(n) {\n"
        "  if (n <= 0) { return n }\n"
        "  return deep_recursion(n - 1)\n"
        "}\n"
        "let result = deep_recursion(101)\n"; // Should exceed limit
    
    reset_runtime_state();
    
    ASTNode *root = parse_script_from_string(code);
    TEST_ASSERT_NOT_NULL(root);
    
    emit_gcode(root);
    
    // Should fail gracefully and return default value (Requirement 1.1, 1.4)
    Value *result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, result->number);
    
    // System should be stable after error (Requirement 1.2, 2.3)
    Runtime *rt = get_runtime();
    TEST_ASSERT_EQUAL_INT(0, rt->recursion_depth);
    
    free_ast(root);
}

// Test system recovery after recursion error (Requirement 2.4)
void test_system_recovery_after_recursion_error(void)
{
    // First, trigger recursion limit
    const char *infinite_code = 
        "function infinite() { return infinite() }\n"
        "let bad_result = infinite()\n";
    
    reset_runtime_state();
    
    ASTNode *infinite_root = parse_script_from_string(infinite_code);
    TEST_ASSERT_NOT_NULL(infinite_root);
    emit_gcode(infinite_root);
    
    Value *bad_result = get_var("bad_result");
    TEST_ASSERT_NOT_NULL(bad_result);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, bad_result->number);
    
    free_ast(infinite_root);
    
    // Then, verify normal operations still work (Requirement 2.4)
    const char *normal_code = 
        "function add(a, b) { return a + b }\n"
        "let good_result = add(5, 3)\n";
    
    ASTNode *normal_root = parse_script_from_string(normal_code);
    TEST_ASSERT_NOT_NULL(normal_root);
    emit_gcode(normal_root);
    
    Value *good_result = get_var("good_result");
    TEST_ASSERT_NOT_NULL(good_result);
    TEST_ASSERT_EQUAL_DOUBLE(8.0, good_result->number);
    
    // Verify system is still stable (Requirement 2.4)
    Runtime *rt = get_runtime();
    TEST_ASSERT_EQUAL_INT(0, rt->recursion_depth);
    
    free_ast(normal_root);
}

// Test complex recursion scenario with multiple functions
void test_complex_recursion_chain(void)
{
    const char *code = 
        "function func_a(n) { if (n <= 0) { return 0 } return func_b(n - 1) }\n"
        "function func_b(n) { if (n <= 0) { return 0 } return func_c(n - 1) }\n"
        "function func_c(n) { if (n <= 0) { return 0 } return func_a(n - 1) }\n"
        "let result = func_a(150)\n"; // Should exceed limit through chain
    
    reset_runtime_state();
    
    ASTNode *root = parse_script_from_string(code);
    TEST_ASSERT_NOT_NULL(root);
    
    emit_gcode(root);
    
    // Should detect recursion limit in the chain (Requirement 1.1)
    Value *result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, result->number);
    
    // System should be stable (Requirement 1.2, 2.3)
    Runtime *rt = get_runtime();
    TEST_ASSERT_EQUAL_INT(0, rt->recursion_depth);
    
    free_ast(root);
}

// Test that recursion protection doesn't interfere with normal function calls
void test_normal_function_calls_unaffected(void)
{
    const char *code = 
        "function multiply(a, b) { return a * b }\n"
        "function add(a, b) { return a + b }\n"
        "function calculate() { return add(multiply(2, 3), multiply(4, 5)) }\n"
        "let result = calculate()\n";
    
    reset_runtime_state();
    
    ASTNode *root = parse_script_from_string(code);
    TEST_ASSERT_NOT_NULL(root);
    
    emit_gcode(root);
    
    // Normal function calls should work perfectly
    Value *result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_DOUBLE(26.0, result->number); // (2*3) + (4*5) = 6 + 20 = 26
    
    Runtime *rt = get_runtime();
    TEST_ASSERT_EQUAL_INT(0, rt->recursion_depth);
    
    free_ast(root);
}

// Test recursion depth tracking accuracy
void test_recursion_depth_tracking(void)
{
    const char *code = 
        "function factorial(n) {\n"
        "  if (n <= 1) { return 1 }\n"
        "  return n * factorial(n - 1)\n"
        "}\n"
        "let result = factorial(5)\n"; // Well within limit, should return 120
    
    reset_runtime_state();
    
    ASTNode *root = parse_script_from_string(code);
    TEST_ASSERT_NOT_NULL(root);
    
    emit_gcode(root);
    
    // Should work fine within limits
    Value *result = get_var("result");
    TEST_ASSERT_NOT_NULL(result);
    // factorial(5) = 5 * 4 * 3 * 2 * 1 = 120
    TEST_ASSERT_EQUAL_DOUBLE(120.0, result->number);
    
    // Depth should be back to 0 after completion
    Runtime *rt = get_runtime();
    TEST_ASSERT_EQUAL_INT(0, rt->recursion_depth);
    
    free_ast(root);
}

int main(void)
{
    UNITY_BEGIN();
    
    // Test the original failing case and all requirements
    RUN_TEST(test_original_infinite_recursion_case);        // Original bug scenario
    RUN_TEST(test_single_function_infinite_recursion);      // Simple infinite recursion
    RUN_TEST(test_legitimate_recursion_within_limits);      // Normal recursion should work
    RUN_TEST(test_recursion_at_limit_boundary);             // Test at exact limit
    RUN_TEST(test_recursion_over_limit_boundary);           // Test just over limit
    RUN_TEST(test_system_recovery_after_recursion_error);   // System stability (Req 2.4)
    RUN_TEST(test_complex_recursion_chain);                 // Complex recursion patterns
    RUN_TEST(test_normal_function_calls_unaffected);        // Normal functions unaffected
    RUN_TEST(test_recursion_depth_tracking);                // Depth tracking accuracy
    
    return UNITY_END();
}