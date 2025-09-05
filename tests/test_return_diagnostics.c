#include "Unity/src/unity.h"
#include "../src/lexer/lexer.h"
#include "../src/parser/parser.h"
#include "../src/runtime/evaluator.h"
#include "../src/error/error.h"
#include "../src/config/config.h"
#include <string.h>

// Forward declarations
void reset_runtime_state(void);

// External variables from evaluator
extern int runtime_has_returned;
extern Value *runtime_return_value;

void setUp(void) {
    init_runtime();
}

void tearDown(void) {
    reset_runtime_state();
}

// Helper function to parse and evaluate code, capturing any errors
typedef struct {
    int success;
    char error_message[512];
    Value* result;
} TestResult;

TestResult test_code(const char* source) {
    TestResult result = {0};
    
    // Try to parse the code
    ASTNode *root = parse_script_from_string(source);
    if (!root) {
        strcpy(result.error_message, "Parse failed - NULL AST returned");
        return result;
    }
    
    // Evaluate the code without creating a block scope
    // If it's a block, evaluate statements individually at global scope
    if (root->type == AST_BLOCK) {
        for (int i = 0; i < root->block.count; i++) {
            ASTNode *stmt = root->block.statements[i];
            result.result = eval_expr(stmt);
            
            // Reset return state after each statement (return only applies within functions)
            runtime_has_returned = 0;
            if (runtime_return_value) {
                free_value(runtime_return_value);
                runtime_return_value = NULL;
            }
        }
        result.success = 1;
    } else {
        // Single statement
        result.result = eval_expr(root);
        if (result.result) {
            result.success = 1;
        } else {
            strcpy(result.error_message, "Evaluation failed - NULL value returned");
        }
    }
    
    free_ast(root);
    return result;
}

// Test 1: Simple return statement in function
void test_simple_return_in_function(void) {
    const char* source = 
        "function simple_test() {\n"
        "    return 42\n"
        "}\n"
        "let result = simple_test()\n";
    
    TestResult result = test_code(source);
    
    if (result.success) {
        Value* var_result = get_var("result");
        if (var_result) {
            TEST_ASSERT_EQUAL_DOUBLE(42.0, var_result->number);
            printf("✅ Simple return test passed\n");
        } else {
            printf("❌ Variable 'result' not found in runtime\n");
            TEST_FAIL_MESSAGE("Variable 'result' not found");
        }
    } else {
        printf("❌ Simple return test failed: %s\n", result.error_message);
        TEST_FAIL_MESSAGE(result.error_message);
    }
}

// Test 2: Return statement in conditional block
void test_return_in_conditional(void) {
    const char* source = 
        "function conditional_return(x) {\n"
        "    if (x > 0) {\n"
        "        return 1\n"
        "    }\n"
        "    return 0\n"
        "}\n"
        "let result = conditional_return(5)\n";
    
    TestResult result = test_code(source);
    
    if (result.success) {
        Value* var_result = get_var("result");
        TEST_ASSERT_NOT_NULL(var_result);
        TEST_ASSERT_EQUAL_DOUBLE(1.0, var_result->number);
        printf("✅ Conditional return test passed\n");
    } else {
        printf("❌ Conditional return test failed: %s\n", result.error_message);
        TEST_FAIL_MESSAGE(result.error_message);
    }
}

// Test 3: Return with complex mathematical expression
void test_return_complex_expression(void) {
    const char* source = 
        "function complex_calc(x, y) {\n"
        "    return x * y + sqrt(x * x + y * y)\n"
        "}\n"
        "let result = complex_calc(3, 4)\n";
    
    TestResult result = test_code(source);
    
    if (result.success) {
        Value* var_result = get_var("result");
        TEST_ASSERT_NOT_NULL(var_result);
        // 3 * 4 + sqrt(3*3 + 4*4) = 12 + sqrt(25) = 12 + 5 = 17
        TEST_ASSERT_EQUAL_DOUBLE(17.0, var_result->number);
        printf("✅ Complex expression return test passed\n");
    } else {
        printf("❌ Complex expression return test failed: %s\n", result.error_message);
        TEST_FAIL_MESSAGE(result.error_message);
    }
}

// Test 4: Nested return statements (return inside nested if)
void test_nested_return_statements(void) {
    const char* source = 
        "function nested_test(x, y) {\n"
        "    if (x > 0) {\n"
        "        if (y > 0) {\n"
        "            return x + y\n"
        "        } else {\n"
        "            return x - y\n"
        "        }\n"
        "    }\n"
        "    return 0\n"
        "}\n"
        "let result = nested_test(5, 3)\n";
    
    TestResult result = test_code(source);
    
    if (result.success) {
        Value* var_result = get_var("result");
        TEST_ASSERT_NOT_NULL(var_result);
        TEST_ASSERT_EQUAL_DOUBLE(8.0, var_result->number);
        printf("✅ Nested return test passed\n");
    } else {
        printf("❌ Nested return test failed: %s\n", result.error_message);
        TEST_FAIL_MESSAGE(result.error_message);
    }
}

// Test 5: Return statement outside function (should fail)
void test_return_outside_function(void) {
    const char* source = 
        "let x = 5\n"
        "return x\n";  // This should cause an error
    
    TestResult result = test_code(source);
    
    // This test should fail - return outside function is invalid
    if (!result.success) {
        printf("✅ Return outside function correctly failed: %s\n", result.error_message);
        // This is expected behavior
    } else {
        printf("❌ Return outside function should have failed but didn't\n");
        TEST_FAIL_MESSAGE("Return outside function should have failed");
    }
}

// Test 6: Bare return statement (no expression)
void test_bare_return_statement(void) {
    const char* source = 
        "function bare_return_test() {\n"
        "    let x = 5\n"
        "    return\n"  // No expression after return
        "}\n"
        "let result = bare_return_test()\n";
    
    TestResult result = test_code(source);
    
    if (result.success) {
        Value* var_result = get_var("result");
        TEST_ASSERT_NOT_NULL(var_result);
        // Bare return should return 0 or undefined value
        printf("✅ Bare return test passed, result: %f\n", var_result->number);
    } else {
        printf("❌ Bare return test failed: %s\n", result.error_message);
        TEST_FAIL_MESSAGE(result.error_message);
    }
}

// Test 7: SVG-like elliptical arc function (simplified version)
void test_svg_arc_like_function(void) {
    const char* source = 
        "function elliptical_arc(rx, ry, x_axis_rotation, large_arc_flag, sweep_flag, x, y) {\n"
        "    if (rx == 0 || ry == 0) {\n"
        "        return 0\n"  // Degenerate case
        "    }\n"
        "    \n"
        "    let cos_phi = cos(x_axis_rotation)\n"
        "    let sin_phi = sin(x_axis_rotation)\n"
        "    \n"
        "    if (x == 0 && y == 0) {\n"
        "        return 1\n"  // Another early return
        "    }\n"
        "    \n"
        "    let result = rx * cos_phi + ry * sin_phi\n"
        "    return result\n"
        "}\n"
        "let arc_result = elliptical_arc(10, 5, 0, 0, 0, 1, 1)\n";
    
    TestResult result = test_code(source);
    
    if (result.success) {
        Value* var_result = get_var("arc_result");
        TEST_ASSERT_NOT_NULL(var_result);
        // rx * cos(0) + ry * sin(0) = 10 * 1 + 5 * 0 = 10
        TEST_ASSERT_EQUAL_DOUBLE(10.0, var_result->number);
        printf("✅ SVG arc-like function test passed\n");
    } else {
        printf("❌ SVG arc-like function test failed: %s\n", result.error_message);
        TEST_FAIL_MESSAGE(result.error_message);
    }
}

// Test 8: Multiple return paths with different expressions
void test_multiple_return_paths(void) {
    const char* source = 
        "function multi_return(mode) {\n"
        "    if (mode == 1) {\n"
        "        return 2 + 3 * 4\n"  // Should be 14
        "    } else if (mode == 2) {\n"
        "        return pow(2, 3)\n"   // Should be 8
        "    } else if (mode == 3) {\n"
        "        return sqrt(16)\n"    // Should be 4
        "    }\n"
        "    return -1\n"  // Default case
        "}\n"
        "let r1 = multi_return(1)\n"
        "let r2 = multi_return(2)\n"
        "let r3 = multi_return(3)\n"
        "let r4 = multi_return(99)\n";
    
    TestResult result = test_code(source);
    
    if (result.success) {
        Value* r1 = get_var("r1");
        Value* r2 = get_var("r2");
        Value* r3 = get_var("r3");
        Value* r4 = get_var("r4");
        
        TEST_ASSERT_NOT_NULL(r1);
        TEST_ASSERT_NOT_NULL(r2);
        TEST_ASSERT_NOT_NULL(r3);
        TEST_ASSERT_NOT_NULL(r4);
        
        TEST_ASSERT_EQUAL_DOUBLE(14.0, r1->number);
        TEST_ASSERT_EQUAL_DOUBLE(8.0, r2->number);
        TEST_ASSERT_EQUAL_DOUBLE(4.0, r3->number);
        TEST_ASSERT_EQUAL_DOUBLE(-1.0, r4->number);
        
        printf("✅ Multiple return paths test passed\n");
    } else {
        printf("❌ Multiple return paths test failed: %s\n", result.error_message);
        TEST_FAIL_MESSAGE(result.error_message);
    }
}

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== Return Statement Diagnostic Tests ===\n");
    
    RUN_TEST(test_simple_return_in_function);
    RUN_TEST(test_return_in_conditional);
    RUN_TEST(test_return_complex_expression);
    RUN_TEST(test_nested_return_statements);
    RUN_TEST(test_return_outside_function);
    RUN_TEST(test_bare_return_statement);
    RUN_TEST(test_svg_arc_like_function);
    RUN_TEST(test_multiple_return_paths);
    
    printf("\n=== Diagnostic Tests Complete ===\n");
    
    return UNITY_END();
}