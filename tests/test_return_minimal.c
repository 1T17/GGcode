#include "Unity/src/unity.h"
#include "../src/lexer/lexer.h"
#include "../src/parser/parser.h"
#include "../src/runtime/evaluator.h"
#include "../src/config/config.h"
#include <string.h>

// Forward declarations
void reset_runtime_state(void);

void setUp(void) {
    init_runtime();
    reset_runtime_state();
}

void tearDown(void) {
    reset_runtime_state();
}

// Test 1: Simple function without return (baseline)
void test_function_without_return(void) {
    const char* source = 
        "function simple() {\n"
        "    let x = 5\n"
        "}\n"
        "let result = simple()\n";
    
    printf("\n=== Testing function without return ===\n");
    printf("Source code:\n%s\n", source);
    
    ASTNode *root = parse_script_from_string(source);
    if (!root) {
        printf("❌ Parse failed\n");
        TEST_FAIL_MESSAGE("Parse failed");
        return;
    }
    
    printf("✅ Parse succeeded\n");
    
    // Try to evaluate without creating a block scope
    if (root->type == AST_BLOCK) {
        for (int i = 0; i < root->block.count; i++) {
            ASTNode *stmt = root->block.statements[i];
            eval_expr(stmt);
        }
    } else {
        eval_expr(root);
    }
    
    Value* var_result = get_var("result");
    if (var_result) {
        printf("✅ Variable 'result' found with value: %f\n", var_result->number);
    } else {
        printf("❌ Variable 'result' not found\n");
    }
    
    free_ast(root);
}

// Test 2: Simple function with return
void test_function_with_return(void) {
    const char* source = 
        "function simple() {\n"
        "    return 42\n"
        "}\n"
        "let result = simple()\n";
    
    printf("\n=== Testing function with return ===\n");
    printf("Source code:\n%s\n", source);
    
    ASTNode *root = parse_script_from_string(source);
    if (!root) {
        printf("❌ Parse failed\n");
        TEST_FAIL_MESSAGE("Parse failed");
        return;
    }
    
    printf("✅ Parse succeeded\n");
    
    // Try to evaluate without creating a block scope
    if (root->type == AST_BLOCK) {
        for (int i = 0; i < root->block.count; i++) {
            ASTNode *stmt = root->block.statements[i];
            eval_expr(stmt);
        }
    } else {
        eval_expr(root);
    }
    
    Value* var_result = get_var("result");
    if (var_result) {
        printf("✅ Variable 'result' found with value: %f\n", var_result->number);
        TEST_ASSERT_EQUAL_DOUBLE(42.0, var_result->number);
    } else {
        printf("❌ Variable 'result' not found\n");
        TEST_FAIL_MESSAGE("Variable 'result' not found");
    }
    
    free_ast(root);
}

// Test 3: Return outside function
void test_return_outside_function(void) {
    const char* source = 
        "let x = 5\n"
        "return x\n";
    
    printf("\n=== Testing return outside function ===\n");
    printf("Source code:\n%s\n", source);
    
    ASTNode *root = parse_script_from_string(source);
    if (!root) {
        printf("❌ Parse failed (this might be expected)\n");
        return;
    }
    
    printf("✅ Parse succeeded (this should NOT happen)\n");
    
    // Try to evaluate
    eval_expr(root);
    
    printf("❌ Return outside function was allowed - this is a bug!\n");
    
    free_ast(root);
}

// Test 4: Bare return statement
void test_bare_return(void) {
    const char* source = 
        "function test() {\n"
        "    return\n"
        "}\n";
    
    printf("\n=== Testing bare return ===\n");
    printf("Source code:\n%s\n", source);
    
    ASTNode *root = parse_script_from_string(source);
    if (!root) {
        printf("❌ Parse failed for bare return\n");
        return;
    }
    
    printf("✅ Parse succeeded for bare return\n");
    
    free_ast(root);
}

int main(void) {
    UNITY_BEGIN();
    
    printf("\n=== Minimal Return Statement Diagnostic Tests ===\n");
    
    RUN_TEST(test_function_without_return);
    RUN_TEST(test_function_with_return);
    RUN_TEST(test_return_outside_function);
    RUN_TEST(test_bare_return);
    
    printf("\n=== Minimal Diagnostic Tests Complete ===\n");
    
    return UNITY_END();
}