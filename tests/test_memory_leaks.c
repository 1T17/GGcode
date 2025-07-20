#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <math.h>
#include "Unity/src/unity.h"
#include "../src/lexer/lexer.h"
#include "../src/parser/parser.h"
#include "../src/generator/emitter.h"
#include "../src/runtime/evaluator.h"
#include "../src/utils/output_buffer.h"
#include "../src/config/config.h"

void setUp(void) {
    // Initialize runtime state for each test
    reset_runtime_state();
    init_output_buffer();
}

void tearDown(void) {
    // Clean up after each test
    free_output_buffer();
}

// Helper function to get current memory usage in KB
long get_memory_usage() {
    FILE *file = fopen("/proc/self/status", "r");
    if (!file) return -1;
    
    long memory = -1;
    char line[128];
    while (fgets(line, 128, file) != NULL) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line, "VmRSS: %ld", &memory);
            break;
        }
    }
    fclose(file);
    return memory;
}

// Test basic compilation without memory leaks
void test_basic_compilation_no_leaks() {
    const char *source = "G1 X10 Y20\nlet x = 5\nlet y = x + 3";
    
    // Get initial memory
    long initial_memory = get_memory_usage();
    TEST_ASSERT_GREATER_THAN(0, initial_memory);
    
    // Run multiple compilations
    for (int i = 0; i < 100; i++) {
        // Initialize output buffer
        init_output_buffer();
        
        // Parse
        ASTNode *ast = parse_script_from_string(source);
        TEST_ASSERT_NOT_NULL(ast);
        
        // Emit G-code
        emit_gcode(ast);
        
        // Clean up
        free_ast(ast);
        free_output_buffer();
    }
    
    // Get final memory
    long final_memory = get_memory_usage();
    TEST_ASSERT_GREATER_THAN(0, final_memory);
    
    // Check memory difference (should be minimal)
    long memory_diff = final_memory - initial_memory;
    printf("Memory usage: %ld KB -> %ld KB (diff: %ld KB)\n", 
           initial_memory, final_memory, memory_diff);
    
    // Allow for some small memory growth (within 1MB)
    TEST_ASSERT_LESS_THAN(1024, labs(memory_diff));
}

// Test complex compilation with arrays and functions
void test_complex_compilation_no_leaks() {
    const char *source = 
        "let maze = []\n"
        "for y = 0..10 {\n"
        "  let row = []\n"
        "  for x = 0..10 {\n"
        "    row[x] = 1\n"
        "  }\n"
        "  maze[y] = row\n"
        "}\n"
        "function test(x, y) {\n"
        "  return x + y\n"
        "}\n"
        "let result = test(5, 3)\n"
        "G1 X10 Y20\n";
    
    // Get initial memory
    long initial_memory = get_memory_usage();
    TEST_ASSERT_GREATER_THAN(0, initial_memory);
    
    // Run multiple compilations
    for (int i = 0; i < 50; i++) {
        // Initialize output buffer
        init_output_buffer();
        
        // Parse
        ASTNode *ast = parse_script_from_string(source);
        TEST_ASSERT_NOT_NULL(ast);
        
        // Emit G-code
        emit_gcode(ast);
        
        // Clean up
        free_ast(ast);
        free_output_buffer();
    }
    
    // Get final memory
    long final_memory = get_memory_usage();
    TEST_ASSERT_GREATER_THAN(0, final_memory);
    
    // Check memory difference
    long memory_diff = final_memory - initial_memory;
    printf("Complex compilation memory: %ld KB -> %ld KB (diff: %ld KB)\n", 
           initial_memory, final_memory, memory_diff);
    
    // Allow for some small memory growth (within 1MB)
    TEST_ASSERT_LESS_THAN(1024, labs(memory_diff));
}

// Test error handling without memory leaks
void test_error_handling_no_leaks() {
    const char *invalid_source = "let x = { invalid syntax }";
    
    // Get initial memory
    long initial_memory = get_memory_usage();
    TEST_ASSERT_GREATER_THAN(0, initial_memory);
    
    // Run multiple error cases
    for (int i = 0; i < 100; i++) {
        // Initialize output buffer
        init_output_buffer();
        
        // Try to parse (should fail)
        ASTNode *ast = parse_script_from_string(invalid_source);
        // AST might be NULL or partial, that's okay
        
        // Try to emit (might fail)
        if (ast) {
            emit_gcode(ast);
            free_ast(ast);
        }
        
        // Clean up
        free_output_buffer();
    }
    
    // Get final memory
    long final_memory = get_memory_usage();
    TEST_ASSERT_GREATER_THAN(0, final_memory);
    
    // Check memory difference
    long memory_diff = final_memory - initial_memory;
    printf("Error handling memory: %ld KB -> %ld KB (diff: %ld KB)\n", 
           initial_memory, final_memory, memory_diff);
    
    // Allow for some small memory growth (within 1MB)
    TEST_ASSERT_LESS_THAN(1024, labs(memory_diff));
}

// Test large program compilation
void test_large_program_no_leaks() {
    // Create a large program
    char large_source[10000];
    int pos = 0;
    
    // Generate a large program with many variables and operations
    for (int i = 0; i < 100; i++) {
        pos += snprintf(large_source + pos, sizeof(large_source) - pos,
                       "let var%d = %d\n", i, i);
    }
    
    // Add some G-code
    pos += snprintf(large_source + pos, sizeof(large_source) - pos,
                   "G1 X10 Y20\nG1 X30 Y40\n");
    
    // Get initial memory
    long initial_memory = get_memory_usage();
    TEST_ASSERT_GREATER_THAN(0, initial_memory);
    
    // Run multiple compilations
    for (int i = 0; i < 20; i++) {
        // Initialize output buffer
        init_output_buffer();
        
        // Parse
        ASTNode *ast = parse_script_from_string(large_source);
        TEST_ASSERT_NOT_NULL(ast);
        
        // Emit G-code
        emit_gcode(ast);
        
        // Clean up
        free_ast(ast);
        free_output_buffer();
    }
    
    // Get final memory
    long final_memory = get_memory_usage();
    TEST_ASSERT_GREATER_THAN(0, final_memory);
    
    // Check memory difference
    long memory_diff = final_memory - initial_memory;
    printf("Large program memory: %ld KB -> %ld KB (diff: %ld KB)\n", 
           initial_memory, final_memory, memory_diff);
    
    // Allow for some small memory growth (within 1MB)
    TEST_ASSERT_LESS_THAN(1024, labs(memory_diff));
}

// Test memory usage with different input sizes
void test_memory_scaling() {
    const char *small_source = "G1 X10 Y20";
    const char *medium_source = "let x = 5\nlet y = 10\nG1 X{x} Y{y}";
    const char *large_source = 
        "let maze = []\n"
        "for y = 0..10 {\n"
        "  let row = []\n"
        "  for x = 0..10 {\n"
        "    row[x] = 1\n"
        "  }\n"
        "  maze[y] = row\n"
        "}\n"
        "G1 X10 Y20\n";
    
    long initial_memory = get_memory_usage();
    TEST_ASSERT_GREATER_THAN(0, initial_memory);
    
    // Test small source
    for (int i = 0; i < 50; i++) {
        init_output_buffer();
        ASTNode *ast = parse_script_from_string(small_source);
        emit_gcode(ast);
        free_ast(ast);
        free_output_buffer();
    }
    
    long small_memory = get_memory_usage();
    
    // Test medium source
    for (int i = 0; i < 50; i++) {
        init_output_buffer();
        ASTNode *ast = parse_script_from_string(medium_source);
        emit_gcode(ast);
        free_ast(ast);
        free_output_buffer();
    }
    
    long medium_memory = get_memory_usage();
    
    // Test large source
    for (int i = 0; i < 50; i++) {
        init_output_buffer();
        ASTNode *ast = parse_script_from_string(large_source);
        emit_gcode(ast);
        free_ast(ast);
        free_output_buffer();
    }
    
    long large_memory = get_memory_usage();
    
    printf("Memory scaling: %ld KB -> %ld KB -> %ld KB -> %ld KB\n",
           initial_memory, small_memory, medium_memory, large_memory);
    
    // Memory should not grow significantly
    long total_growth = large_memory - initial_memory;
    TEST_ASSERT_LESS_THAN(2048, total_growth); // Allow 2MB growth max
}

int main() {
    UNITY_BEGIN();
    
    printf("🧪 Running Memory Leak Tests...\n");
    
    RUN_TEST(test_basic_compilation_no_leaks);
    RUN_TEST(test_complex_compilation_no_leaks);
    RUN_TEST(test_error_handling_no_leaks);
    RUN_TEST(test_large_program_no_leaks);
    RUN_TEST(test_memory_scaling);
    
    printf("✅ Memory leak tests completed\n");
    
    return UNITY_END();
} 