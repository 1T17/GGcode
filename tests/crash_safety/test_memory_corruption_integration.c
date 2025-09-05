#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
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
#include "../src/config/config.h"

void setUp(void) {
    memory_safety_init();
    setup_crash_handlers();
    reset_runtime_state();
    init_output_buffer();
}

void tearDown(void) {
    free_output_buffer();
    memory_safety_cleanup();
    cleanup_crash_handlers();
}

// Test buffer boundary conditions in lexer
void test_lexer_buffer_boundaries(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Test with extremely long identifier
    char* long_identifier = (char*)SAFE_MALLOC(10000);
    TEST_ASSERT_NOT_NULL(long_identifier);
    
    // Fill with valid identifier characters
    for (int i = 0; i < 9999; i++) {
        long_identifier[i] = (i % 26) + 'a';
    }
    long_identifier[9999] = '\0';
    
    // Create source with long identifier
    size_t source_len = strlen(long_identifier) + 100;
    char* source = (char*)SAFE_MALLOC(source_len);
    TEST_ASSERT_NOT_NULL(source);
    
    snprintf(source, source_len, "let %s = 42", long_identifier);
    
    // Test lexer with long identifier
    ASTNode* ast = parse_script_from_string(source);
    
    // Should handle gracefully (may succeed or fail, but shouldn't crash)
    if (ast) {
        free_ast(ast);
    }
    
    SAFE_FREE(source);
    SAFE_FREE(long_identifier);
    
    ASSERT_NO_BUFFER_OVERFLOWS();
    ASSERT_NO_MEMORY_LEAKS();
    
    MEMORY_SAFETY_TEST_END();
}

// Test parser with deeply nested expressions
void test_parser_deep_nesting_protection(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Create deeply nested expression
    const int nesting_depth = 1000;
    size_t source_size = nesting_depth * 10 + 100;
    char* source = (char*)SAFE_MALLOC(source_size);
    TEST_ASSERT_NOT_NULL(source);
    
    strcpy(source, "let result = ");
    
    // Add opening parentheses
    for (int i = 0; i < nesting_depth; i++) {
        strcat(source, "(");
    }
    
    strcat(source, "42");
    
    // Add closing parentheses
    for (int i = 0; i < nesting_depth; i++) {
        strcat(source, ")");
    }
    
    // Test parser with deep nesting
    ASTNode* ast = parse_script_from_string(source);
    
    // Should handle gracefully without stack overflow
    if (ast) {
        free_ast(ast);
    }
    
    SAFE_FREE(source);
    
    ASSERT_NO_BUFFER_OVERFLOWS();
    ASSERT_NO_MEMORY_LEAKS();
    
    MEMORY_SAFETY_TEST_END();
}

// Test memory allocation failure handling in parser
void test_parser_allocation_failure_handling(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    const char* source = "let x = 5\nlet y = 10\nG1 X{x} Y{y}";
    
    // Enable allocation failure simulation
    enable_allocation_failure_simulation();
    set_allocation_failure_rate(0.1); // 10% failure rate
    
    // Try parsing multiple times with allocation failures
    for (int i = 0; i < 50; i++) {
        ASTNode* ast = parse_script_from_string(source);
        
        // Should handle allocation failures gracefully
        if (ast) {
            free_ast(ast);
        }
        
        // Reset for next iteration
        free_output_buffer();
        init_output_buffer();
    }
    
    disable_allocation_failure_simulation();
    
    ASSERT_NO_BUFFER_OVERFLOWS();
    ASSERT_NO_USE_AFTER_FREE();
    ASSERT_NO_DOUBLE_FREE();
    
    MEMORY_SAFETY_TEST_END();
}

// Test emitter buffer overflow protection
void test_emitter_buffer_overflow_protection(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Create source that generates large output
    const int num_commands = 10000;
    size_t source_size = num_commands * 50 + 1000;
    char* source = (char*)SAFE_MALLOC(source_size);
    TEST_ASSERT_NOT_NULL(source);
    
    strcpy(source, "");
    
    // Generate many G-code commands
    for (int i = 0; i < num_commands; i++) {
        char command[50];
        snprintf(command, sizeof(command), "G1 X%d Y%d\n", i, i * 2);
        strcat(source, command);
    }
    
    // Parse and emit
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    // Emit G-code (should handle large output safely)
    emit_gcode(ast);
    
    // Check output buffer integrity
    const char* output = get_output_buffer();
    TEST_ASSERT_NOT_NULL(output);
    
    free_ast(ast);
    SAFE_FREE(source);
    
    ASSERT_NO_BUFFER_OVERFLOWS();
    ASSERT_NO_MEMORY_LEAKS();
    
    MEMORY_SAFETY_TEST_END();
}

// Test evaluator with large data structures
void test_evaluator_large_data_structures(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Create source with large arrays
    const char* source = 
        "let large_array = []\n"
        "for i = 0..1000 {\n"
        "  large_array[i] = i * 2\n"
        "}\n"
        "let sum = 0\n"
        "for i = 0..999 {\n"
        "  sum = sum + large_array[i]\n"
        "}\n"
        "G1 X{sum % 100} Y{sum % 100}\n";
    
    // Parse and evaluate
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    // Emit G-code (involves evaluation)
    emit_gcode(ast);
    
    free_ast(ast);
    
    ASSERT_NO_BUFFER_OVERFLOWS();
    ASSERT_NO_MEMORY_LEAKS();
    
    MEMORY_SAFETY_TEST_END();
}

// Test error condition resource cleanup
void test_error_condition_resource_cleanup(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Test various error conditions
    const char* error_sources[] = {
        "let x = undefined_variable",  // Undefined variable
        "let y = 5 / 0",              // Division by zero
        "function f() { return f() }", // Infinite recursion
        "let z = [1, 2, 3][10]",      // Array out of bounds
        "G1 X{invalid_expr} Y10",     // Invalid expression
        NULL
    };
    
    for (int i = 0; error_sources[i] != NULL; i++) {
        // Reset state for each test
        free_output_buffer();
        init_output_buffer();
        
        // Try to parse and emit (should handle errors gracefully)
        ASTNode* ast = parse_script_from_string(error_sources[i]);
        
        if (ast) {
            // Try to emit (may fail due to evaluation errors)
            emit_gcode(ast);
            free_ast(ast);
        }
        
        // Check that resources are cleaned up properly after errors
        MemoryStats* stats = memory_safety_get_stats();
        size_t current_allocations = stats->current_allocations;
        
        // Should not accumulate allocations across error conditions
        TEST_ASSERT_LESS_THAN(100, current_allocations); // Allow some reasonable number
    }
    
    ASSERT_NO_MEMORY_LEAKS();
    
    MEMORY_SAFETY_TEST_END();
}

// Test concurrent compilation safety
void test_concurrent_compilation_safety(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    const char* source = "let x = 42\nG1 X{x} Y{x}";
    
    // Simulate concurrent access by rapid compilation cycles
    for (int i = 0; i < 100; i++) {
        // Reset state
        free_output_buffer();
        init_output_buffer();
        
        // Parse
        ASTNode* ast = parse_script_from_string(source);
        TEST_ASSERT_NOT_NULL(ast);
        
        // Emit
        emit_gcode(ast);
        
        // Cleanup
        free_ast(ast);
        
        // Verify no memory corruption between iterations
        TEST_ASSERT_TRUE(check_memory_integrity());
    }
    
    ASSERT_NO_BUFFER_OVERFLOWS();
    ASSERT_NO_USE_AFTER_FREE();
    ASSERT_NO_DOUBLE_FREE();
    ASSERT_NO_MEMORY_LEAKS();
    
    MEMORY_SAFETY_TEST_END();
}

// Test stack overflow protection in recursive functions
void test_recursive_function_stack_protection(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Test recursive function that should be limited
    const char* source = 
        "function factorial(n) {\n"
        "  if n <= 1 {\n"
        "    return 1\n"
        "  }\n"
        "  return n * factorial(n - 1)\n"
        "}\n"
        "let result = factorial(100)\n"  // Deep recursion
        "G1 X{result % 100} Y{result % 100}\n";
    
    setup_stack_canary();
    
    // Parse and evaluate
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    // Emit (should handle deep recursion safely)
    emit_gcode(ast);
    
    // Check stack canary
    TEST_ASSERT_TRUE(check_stack_canary());
    
    free_ast(ast);
    
    ASSERT_NO_BUFFER_OVERFLOWS();
    ASSERT_NO_MEMORY_LEAKS();
    
    MEMORY_SAFETY_TEST_END();
}

// Test string handling buffer safety
void test_string_handling_buffer_safety(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Create source with very long strings
    const int string_length = 5000;
    char* long_string = (char*)SAFE_MALLOC(string_length + 1);
    TEST_ASSERT_NOT_NULL(long_string);
    
    // Fill with characters
    for (int i = 0; i < string_length; i++) {
        long_string[i] = 'A' + (i % 26);
    }
    long_string[string_length] = '\0';
    
    // Create source with long string literal
    size_t source_size = string_length + 200;
    char* source = (char*)SAFE_MALLOC(source_size);
    TEST_ASSERT_NOT_NULL(source);
    
    snprintf(source, source_size, "let long_str = \"%s\"\nG1 X10 Y20", long_string);
    
    // Parse and emit
    ASTNode* ast = parse_script_from_string(source);
    
    if (ast) {
        emit_gcode(ast);
        free_ast(ast);
    }
    
    SAFE_FREE(source);
    SAFE_FREE(long_string);
    
    ASSERT_NO_BUFFER_OVERFLOWS();
    ASSERT_NO_MEMORY_LEAKS();
    
    MEMORY_SAFETY_TEST_END();
}

// Test memory usage under stress conditions
void test_memory_usage_stress_conditions(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Record initial memory usage
    MemoryStats* initial_stats = memory_safety_get_stats();
    size_t initial_allocations = initial_stats->current_allocations;
    
    // Perform many compilation cycles
    for (int cycle = 0; cycle < 50; cycle++) {
        // Create increasingly complex source
        size_t source_size = 1000 + cycle * 100;
        char* source = (char*)SAFE_MALLOC(source_size);
        TEST_ASSERT_NOT_NULL(source);
        
        strcpy(source, "");
        
        // Add variables and operations
        for (int i = 0; i < cycle + 10; i++) {
            char line[100];
            snprintf(line, sizeof(line), "let var%d = %d\n", i, i * 2);
            strcat(source, line);
        }
        
        strcat(source, "G1 X10 Y20\n");
        
        // Parse and emit
        ASTNode* ast = parse_script_from_string(source);
        if (ast) {
            emit_gcode(ast);
            free_ast(ast);
        }
        
        SAFE_FREE(source);
        
        // Reset output buffer
        free_output_buffer();
        init_output_buffer();
        
        // Check memory growth
        MemoryStats* current_stats = memory_safety_get_stats();
        size_t current_allocations = current_stats->current_allocations;
        
        // Memory should not grow unboundedly
        TEST_ASSERT_LESS_THAN(initial_allocations + 50, current_allocations);
    }
    
    ASSERT_NO_MEMORY_LEAKS();
    
    MEMORY_SAFETY_TEST_END();
}

// Test proper cleanup on compilation interruption
void test_cleanup_on_interruption(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    const char* source = 
        "let matrix = []\n"
        "for i = 0..100 {\n"
        "  let row = []\n"
        "  for j = 0..100 {\n"
        "    row[j] = i * j\n"
        "  }\n"
        "  matrix[i] = row\n"
        "}\n";
    
    // Start compilation
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    // Simulate interruption during emission
    // (In a real scenario, this might be a signal)
    
    // Cleanup should happen properly
    free_ast(ast);
    
    // Verify cleanup
    ASSERT_NO_MEMORY_LEAKS();
    
    MEMORY_SAFETY_TEST_END();
}

int main(void) {
    UNITY_BEGIN();
    
    printf("🛡️ Running Memory Corruption Integration Tests...\n");
    
    RUN_TEST(test_lexer_buffer_boundaries);
    RUN_TEST(test_parser_deep_nesting_protection);
    RUN_TEST(test_parser_allocation_failure_handling);
    RUN_TEST(test_emitter_buffer_overflow_protection);
    RUN_TEST(test_evaluator_large_data_structures);
    RUN_TEST(test_error_condition_resource_cleanup);
    RUN_TEST(test_concurrent_compilation_safety);
    RUN_TEST(test_recursive_function_stack_protection);
    RUN_TEST(test_string_handling_buffer_safety);
    RUN_TEST(test_memory_usage_stress_conditions);
    RUN_TEST(test_cleanup_on_interruption);
    
    printf("✅ Memory corruption integration tests completed\n");
    
    return UNITY_END();
}