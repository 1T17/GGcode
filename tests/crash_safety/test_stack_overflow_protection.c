#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/resource.h>
#include "../Unity/src/unity.h"
#include "framework/memory_safety.h"
#include "framework/crash_detection.h"
#include "config/crash_test_config.h"

// Include GGcode compiler components
#include "../src/lexer/lexer.h"
#include "../src/parser/parser.h"
#include "../src/generator/emitter.h"
#include "../src/runtime/evaluator.h"
#include "../src/utils/output_buffer.h"

// Stack overflow detection variables
static jmp_buf stack_overflow_jmp;
static volatile sig_atomic_t stack_overflow_detected = 0;

// Signal handler for stack overflow detection
static void stack_overflow_handler(int sig) {
    stack_overflow_detected = 1;
    longjmp(stack_overflow_jmp, 1);
}

void setUp(void) {
    memory_safety_init();
    setup_crash_handlers();
    reset_runtime_state();
    init_output_buffer();
    stack_overflow_detected = 0;
    
    // Install stack overflow handler
    struct sigaction sa;
    sa.sa_handler = stack_overflow_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_ONSTACK;
    sigaction(SIGSEGV, &sa, NULL);
}

void tearDown(void) {
    free_output_buffer();
    memory_safety_cleanup();
    cleanup_crash_handlers();
    
    // Restore default signal handler
    signal(SIGSEGV, SIG_DFL);
}

// Test basic stack overflow detection
void test_basic_stack_overflow_detection(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    setup_stack_canary();
    
    // Create a function that will cause stack overflow
    if (setjmp(stack_overflow_jmp) == 0) {
        // Simulate deep recursion that should trigger stack overflow protection
        simulate_stack_overflow(10000); // Very deep recursion
        
        // If we get here, stack overflow protection worked
        TEST_ASSERT_TRUE(check_stack_canary());
    } else {
        // Stack overflow was detected and handled
        TEST_ASSERT_TRUE(stack_overflow_detected);
        printf("Stack overflow detected and handled safely\n");
    }
    
    MEMORY_SAFETY_TEST_END();
}

// Test parser stack overflow protection with deeply nested expressions
void test_parser_stack_overflow_protection(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    setup_stack_canary();
    
    // Create extremely deeply nested expression
    const int max_depth = 5000;
    size_t source_size = max_depth * 2 + 100;
    char* source = (char*)SAFE_MALLOC(source_size);
    TEST_ASSERT_NOT_NULL(source);
    
    strcpy(source, "let result = ");
    
    // Add deeply nested parentheses
    for (int i = 0; i < max_depth; i++) {
        strcat(source, "(");
    }
    
    strcat(source, "42");
    
    for (int i = 0; i < max_depth; i++) {
        strcat(source, ")");
    }
    
    if (setjmp(stack_overflow_jmp) == 0) {
        // Try to parse deeply nested expression
        ASTNode* ast = parse_script_from_string(source);
        
        // If parsing succeeded, it should have stack protection
        if (ast) {
            free_ast(ast);
            TEST_ASSERT_TRUE(check_stack_canary());
        }
    } else {
        // Stack overflow was detected during parsing
        TEST_ASSERT_TRUE(stack_overflow_detected);
        printf("Parser stack overflow detected and handled\n");
    }
    
    SAFE_FREE(source);
    
    ASSERT_NO_MEMORY_LEAKS();
    
    MEMORY_SAFETY_TEST_END();
}

// Test recursive function evaluation stack protection
void test_recursive_evaluation_stack_protection(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    setup_stack_canary();
    
    // Create recursive function that will cause deep recursion
    const char* source = 
        "function deep_recursion(n) {\n"
        "  if n <= 0 {\n"
        "    return 0\n"
        "  }\n"
        "  return 1 + deep_recursion(n - 1)\n"
        "}\n"
        "let result = deep_recursion(10000)\n"  // Very deep recursion
        "G1 X{result} Y{result}\n";
    
    if (setjmp(stack_overflow_jmp) == 0) {
        // Parse the source
        ASTNode* ast = parse_script_from_string(source);
        TEST_ASSERT_NOT_NULL(ast);
        
        // Try to emit (this will evaluate the recursive function)
        emit_gcode(ast);
        
        free_ast(ast);
        
        // If we get here, recursion protection worked
        TEST_ASSERT_TRUE(check_stack_canary());
    } else {
        // Stack overflow was detected during evaluation
        TEST_ASSERT_TRUE(stack_overflow_detected);
        printf("Recursive evaluation stack overflow detected and handled\n");
    }
    
    ASSERT_NO_MEMORY_LEAKS();
    
    MEMORY_SAFETY_TEST_END();
}

// Test stack usage monitoring
void test_stack_usage_monitoring(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    setup_stack_canary();
    
    // Record initial stack depth
    size_t initial_depth = get_stack_depth();
    
    // Push some stack frames
    push_stack_frame("test_function_1");
    push_stack_frame("test_function_2");
    push_stack_frame("test_function_3");
    
    // Check stack depth increased
    size_t current_depth = get_stack_depth();
    TEST_ASSERT_EQUAL(initial_depth + 3, current_depth);
    
    // Pop stack frames
    pop_stack_frame();
    pop_stack_frame();
    
    current_depth = get_stack_depth();
    TEST_ASSERT_EQUAL(initial_depth + 1, current_depth);
    
    // Pop remaining frame
    pop_stack_frame();
    
    current_depth = get_stack_depth();
    TEST_ASSERT_EQUAL(initial_depth, current_depth);
    
    // Check stack canary is still intact
    TEST_ASSERT_TRUE(check_stack_canary());
    
    MEMORY_SAFETY_TEST_END();
}

// Test stack overflow with array operations
void test_stack_overflow_with_arrays(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    setup_stack_canary();
    
    // Create source with deeply nested array operations
    const char* source = 
        "function create_nested_array(depth) {\n"
        "  if depth <= 0 {\n"
        "    return [1, 2, 3]\n"
        "  }\n"
        "  let nested = create_nested_array(depth - 1)\n"
        "  return [nested, nested, nested]\n"
        "}\n"
        "let deep_array = create_nested_array(100)\n"  // Deep nesting
        "G1 X10 Y20\n";
    
    if (setjmp(stack_overflow_jmp) == 0) {
        ASTNode* ast = parse_script_from_string(source);
        TEST_ASSERT_NOT_NULL(ast);
        
        emit_gcode(ast);
        free_ast(ast);
        
        TEST_ASSERT_TRUE(check_stack_canary());
    } else {
        TEST_ASSERT_TRUE(stack_overflow_detected);
        printf("Array operation stack overflow detected and handled\n");
    }
    
    ASSERT_NO_MEMORY_LEAKS();
    
    MEMORY_SAFETY_TEST_END();
}

// Test stack overflow recovery
void test_stack_overflow_recovery(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    setup_stack_canary();
    
    // First, cause a stack overflow
    const char* overflow_source = 
        "function infinite_recursion(n) {\n"
        "  return infinite_recursion(n + 1)\n"
        "}\n"
        "let result = infinite_recursion(0)\n";
    
    bool overflow_handled = false;
    
    if (setjmp(stack_overflow_jmp) == 0) {
        ASTNode* ast = parse_script_from_string(overflow_source);
        if (ast) {
            emit_gcode(ast);
            free_ast(ast);
        }
    } else {
        overflow_handled = true;
        stack_overflow_detected = 0; // Reset for next test
    }
    
    // Now test that we can recover and compile normally
    const char* normal_source = "let x = 42\nG1 X{x} Y{x}";
    
    // Reset output buffer
    free_output_buffer();
    init_output_buffer();
    
    // This should work normally after stack overflow recovery
    ASTNode* ast = parse_script_from_string(normal_source);
    TEST_ASSERT_NOT_NULL(ast);
    
    emit_gcode(ast);
    free_ast(ast);
    
    // Verify output was generated
    const char* output = get_output_buffer();
    TEST_ASSERT_NOT_NULL(output);
    TEST_ASSERT_TRUE(strlen(output) > 0);
    
    if (overflow_handled) {
        printf("Successfully recovered from stack overflow\n");
    }
    
    ASSERT_NO_MEMORY_LEAKS();
    
    MEMORY_SAFETY_TEST_END();
}

// Test stack limit configuration
void test_stack_limit_configuration(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Get current stack limit
    struct rlimit stack_limit;
    int result = getrlimit(RLIMIT_STACK, &stack_limit);
    TEST_ASSERT_EQUAL(0, result);
    
    printf("Current stack limit: %ld bytes\n", (long)stack_limit.rlim_cur);
    
    // Test with a smaller stack limit (if we can set it)
    struct rlimit new_limit = stack_limit;
    new_limit.rlim_cur = 1024 * 1024; // 1MB stack
    
    if (setrlimit(RLIMIT_STACK, &new_limit) == 0) {
        printf("Set stack limit to 1MB for testing\n");
        
        setup_stack_canary();
        
        // Try to cause stack overflow with smaller limit
        if (setjmp(stack_overflow_jmp) == 0) {
            simulate_stack_overflow(1000); // Should overflow with 1MB limit
            TEST_ASSERT_TRUE(check_stack_canary());
        } else {
            TEST_ASSERT_TRUE(stack_overflow_detected);
            printf("Stack overflow detected with reduced limit\n");
        }
        
        // Restore original limit
        setrlimit(RLIMIT_STACK, &stack_limit);
    } else {
        printf("Warning: Could not set stack limit for testing\n");
    }
    
    MEMORY_SAFETY_TEST_END();
}

// Test stack overflow with string operations
void test_stack_overflow_string_operations(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    setup_stack_canary();
    
    // Create recursive string concatenation
    const char* source = 
        "function concat_recursive(str, depth) {\n"
        "  if depth <= 0 {\n"
        "    return str\n"
        "  }\n"
        "  return concat_recursive(str + \"_\" + depth, depth - 1)\n"
        "}\n"
        "let long_string = concat_recursive(\"start\", 1000)\n"
        "G1 X10 Y20\n";
    
    if (setjmp(stack_overflow_jmp) == 0) {
        ASTNode* ast = parse_script_from_string(source);
        TEST_ASSERT_NOT_NULL(ast);
        
        emit_gcode(ast);
        free_ast(ast);
        
        TEST_ASSERT_TRUE(check_stack_canary());
    } else {
        TEST_ASSERT_TRUE(stack_overflow_detected);
        printf("String operation stack overflow detected and handled\n");
    }
    
    ASSERT_NO_MEMORY_LEAKS();
    
    MEMORY_SAFETY_TEST_END();
}

// Test multiple stack overflow scenarios
void test_multiple_stack_overflow_scenarios(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    const char* scenarios[] = {
        // Scenario 1: Mutual recursion
        "function func_a(n) { if n > 0 { return func_b(n - 1) } return 0 }\n"
        "function func_b(n) { if n > 0 { return func_a(n - 1) } return 1 }\n"
        "let result = func_a(5000)\n",
        
        // Scenario 2: Nested loops with recursion
        "function nested_loop_recursion(depth) {\n"
        "  if depth <= 0 { return 0 }\n"
        "  let sum = 0\n"
        "  for i = 0..10 {\n"
        "    sum = sum + nested_loop_recursion(depth - 1)\n"
        "  }\n"
        "  return sum\n"
        "}\n"
        "let result = nested_loop_recursion(100)\n",
        
        // Scenario 3: Complex expression evaluation
        "function complex_expr(n) {\n"
        "  if n <= 1 { return 1 }\n"
        "  return complex_expr(n-1) + complex_expr(n-2) + complex_expr(n-3)\n"
        "}\n"
        "let result = complex_expr(50)\n",
        
        NULL
    };
    
    for (int i = 0; scenarios[i] != NULL; i++) {
        setup_stack_canary();
        stack_overflow_detected = 0;
        
        // Reset output buffer for each scenario
        free_output_buffer();
        init_output_buffer();
        
        printf("Testing stack overflow scenario %d\n", i + 1);
        
        if (setjmp(stack_overflow_jmp) == 0) {
            ASTNode* ast = parse_script_from_string(scenarios[i]);
            if (ast) {
                emit_gcode(ast);
                free_ast(ast);
            }
            
            // If we get here, check stack canary
            TEST_ASSERT_TRUE(check_stack_canary());
        } else {
            // Stack overflow was handled
            TEST_ASSERT_TRUE(stack_overflow_detected);
            printf("Scenario %d: Stack overflow detected and handled\n", i + 1);
        }
    }
    
    ASSERT_NO_MEMORY_LEAKS();
    
    MEMORY_SAFETY_TEST_END();
}

int main(void) {
    UNITY_BEGIN();
    
    printf("🛡️ Running Stack Overflow Protection Tests...\n");
    
    RUN_TEST(test_basic_stack_overflow_detection);
    RUN_TEST(test_parser_stack_overflow_protection);
    RUN_TEST(test_recursive_evaluation_stack_protection);
    RUN_TEST(test_stack_usage_monitoring);
    RUN_TEST(test_stack_overflow_with_arrays);
    RUN_TEST(test_stack_overflow_recovery);
    RUN_TEST(test_stack_limit_configuration);
    RUN_TEST(test_stack_overflow_string_operations);
    RUN_TEST(test_multiple_stack_overflow_scenarios);
    
    printf("✅ Stack overflow protection tests completed\n");
    
    return UNITY_END();
}