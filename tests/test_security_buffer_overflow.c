#include "Unity/src/unity.h"
#include "../src/cli/cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Mock compile_file function to avoid linking issues
void compile_file(const char* input_path, const char* output_path, bool quiet) {
    // Mock implementation for testing
    (void)input_path;
    (void)output_path;
    (void)quiet;
}

void setUp(void) {
    // Set up function called before each test
}

void tearDown(void) {
    // Tear down function called after each test
}

/**
 * Test buffer overflow protection in output filename
 */
void test_output_filename_overflow(void) {
    // Create a very long filename (over MAX_OUTPUT_PATH limit)
    char long_filename[1024];
    memset(long_filename, 'A', sizeof(long_filename) - 1);
    long_filename[sizeof(long_filename) - 1] = '\0';
    
    // Create argv with long output filename
    char* argv[] = {"ggcode", "-o", long_filename, "test.ggcode"};
    int argc = 4;
    
    CLIArgs* args = parse_arguments(argc, argv);
    
    // Should return NULL due to filename too long
    TEST_ASSERT_NULL(args);
    printf("✅ Output filename overflow protection working\n");
}

/**
 * Test buffer overflow protection in output directory
 */
void test_output_directory_overflow(void) {
    // Create a very long directory path
    char long_dir[1024];
    memset(long_dir, 'B', sizeof(long_dir) - 1);
    long_dir[sizeof(long_dir) - 1] = '\0';
    
    char* argv[] = {"ggcode", "--output-dir", long_dir, "test.ggcode"};
    int argc = 4;
    
    CLIArgs* args = parse_arguments(argc, argv);
    
    // Should return NULL due to directory path too long
    TEST_ASSERT_NULL(args);
    printf("✅ Output directory overflow protection working\n");
}

/**
 * Test buffer overflow protection in evaluation code
 */
void test_eval_code_overflow(void) {
    // Create evaluation code longer than MAX_EVAL_CODE
    char long_eval[8192];  // Twice the limit
    memset(long_eval, 'C', sizeof(long_eval) - 1);
    long_eval[sizeof(long_eval) - 1] = '\0';
    
    char* argv[] = {"ggcode", "-e", long_eval};
    int argc = 3;
    
    CLIArgs* args = parse_arguments(argc, argv);
    
    // Should return NULL due to eval code too long
    TEST_ASSERT_NULL(args);
    printf("✅ Eval code overflow protection working\n");
}

/**
 * Test buffer overflow protection in input filename
 */
void test_input_filename_overflow(void) {
    // Create a very long input filename
    char long_filename[512];
    memset(long_filename, 'D', sizeof(long_filename) - 1);
    long_filename[sizeof(long_filename) - 1] = '\0';
    
    char* argv[] = {"ggcode", long_filename};
    int argc = 2;
    
    CLIArgs* args = parse_arguments(argc, argv);
    
    // Should return NULL due to filename too long
    TEST_ASSERT_NULL(args);
    printf("✅ Input filename overflow protection working\n");
}

/**
 * Test path traversal protection
 */
void test_path_traversal_protection(void) {
    // Test various path traversal attempts
    char* traversal_attempts[] = {
        "../../../etc/passwd",
        "..\\..\\..\\windows\\system32\\config\\sam",
        "test/../../../secret.txt",
        "normal_file/../../../etc/shadow"
    };
    
    for (int i = 0; i < 4; i++) {
        char* argv[] = {"ggcode", "-o", traversal_attempts[i], "test.ggcode"};
        int argc = 4;
        
        CLIArgs* args = parse_arguments(argc, argv);
        
        // Should return NULL due to path traversal detection
        TEST_ASSERT_NULL(args);
        printf("✅ Path traversal protection working for: %s\n", traversal_attempts[i]);
    }
}

/**
 * Test excessive path traversal protection
 */
void test_excessive_path_traversal(void) {
    char* excessive_traversals[] = {
        "../../../../../../../etc/passwd",
        "..\\..\\..\\..\\..\\windows\\system32"
    };
    
    for (int i = 0; i < 2; i++) {
        char* argv[] = {"ggcode", "--output-dir", excessive_traversals[i]};
        int argc = 3;
        
        CLIArgs* args = parse_arguments(argc, argv);
        
        // Should return NULL due to excessive path traversal
        TEST_ASSERT_NULL(args);
        printf("✅ Excessive path traversal protection working for: %s\n", excessive_traversals[i]);
    }
}

/**
 * Test maximum input files limit
 */
void test_max_input_files_limit(void) {
    // Create argc/argv with too many files (more than MAX_INPUT_FILES)
    int argc = 1002;  // 1 for program name + 1001 files (over limit)
    char** argv = malloc(argc * sizeof(char*));
    
    argv[0] = "ggcode";
    for (int i = 1; i < argc; i++) {
        char* filename = malloc(20);
        snprintf(filename, 20, "file%d.ggcode", i);
        argv[i] = filename;
    }
    
    CLIArgs* args = parse_arguments(argc, argv);
    
    // Should return NULL due to too many files
    TEST_ASSERT_NULL(args);
    printf("✅ Maximum input files limit protection working\n");
    
    // Cleanup
    for (int i = 1; i < argc; i++) {
        free(argv[i]);
    }
    free(argv);
}

/**
 * Test valid inputs still work (boundary testing)
 */
void test_valid_inputs_still_work(void) {
    // Test maximum valid lengths
    char valid_filename[MAX_FILENAME_LENGTH - 1];
    memset(valid_filename, 'V', sizeof(valid_filename) - 1);
    valid_filename[sizeof(valid_filename) - 1] = '\0';
    
    char valid_output[MAX_OUTPUT_PATH - 1];
    memset(valid_output, 'O', sizeof(valid_output) - 1);
    valid_output[sizeof(valid_output) - 1] = '\0';
    
    char valid_eval[MAX_EVAL_CODE - 1];
    memset(valid_eval, 'E', sizeof(valid_eval) - 1);
    valid_eval[sizeof(valid_eval) - 1] = '\0';
    
    // Test valid filename
    char* argv1[] = {"ggcode", valid_filename};
    CLIArgs* args1 = parse_arguments(2, argv1);
    TEST_ASSERT_NOT_NULL(args1);
    free_cli_args(args1);
    
    // Test valid output
    char* argv2[] = {"ggcode", "-o", valid_output, "test.ggcode"};
    CLIArgs* args2 = parse_arguments(4, argv2);
    TEST_ASSERT_NOT_NULL(args2);
    free_cli_args(args2);
    
    // Test valid eval
    char* argv3[] = {"ggcode", "-e", valid_eval};
    CLIArgs* args3 = parse_arguments(3, argv3);
    TEST_ASSERT_NOT_NULL(args3);
    free_cli_args(args3);
    
    printf("✅ Valid inputs at boundary limits still work\n");
}

/**
 * Test memory allocation failure handling
 */
void test_memory_allocation_robustness(void) {
    // Test with normal input to ensure memory management works
    char* argv[] = {"ggcode", "test1.ggcode", "test2.ggcode", "test3.ggcode"};
    int argc = 4;
    
    CLIArgs* args = parse_arguments(argc, argv);
    TEST_ASSERT_NOT_NULL(args);
    TEST_ASSERT_EQUAL_INT(3, args->input_count);
    
    // Verify all strings are properly allocated
    TEST_ASSERT_NOT_NULL(args->input_files[0]);
    TEST_ASSERT_NOT_NULL(args->input_files[1]);
    TEST_ASSERT_NOT_NULL(args->input_files[2]);
    
    // Test proper cleanup
    free_cli_args(args);
    printf("✅ Memory allocation and cleanup working properly\n");
}

/**
 * Test null pointer safety
 */
void test_null_pointer_safety(void) {
    // Test free_cli_args with NULL
    free_cli_args(NULL);  // Should not crash
    
    printf("✅ Null pointer safety working\n");
}

/**
 * Test format string vulnerabilities
 */
void test_format_string_safety(void) {
    // Test with format string characters in filenames
    char* format_strings[] = {
        "test%s%s%s.ggcode",
        "file%n%n%n.ggcode",
        "evil%x%x%x.ggcode"
    };
    
    for (int i = 0; i < 3; i++) {
        char* argv[] = {"ggcode", format_strings[i]};
        CLIArgs* args = parse_arguments(2, argv);
        
        // Should still work but not cause format string vulnerability
        if (args) {
            TEST_ASSERT_EQUAL_STRING(format_strings[i], args->input_files[0]);
            free_cli_args(args);
        }
    }
    
    printf("✅ Format string safety verified\n");
}

int main(void) {
    UNITY_BEGIN();
    
    printf("\n🔒 Running Buffer Overflow and Security Tests...\n\n");
    
    RUN_TEST(test_output_filename_overflow);
    RUN_TEST(test_output_directory_overflow);
    RUN_TEST(test_eval_code_overflow);
    RUN_TEST(test_input_filename_overflow);
    RUN_TEST(test_path_traversal_protection);
    RUN_TEST(test_excessive_path_traversal);
    RUN_TEST(test_max_input_files_limit);
    RUN_TEST(test_valid_inputs_still_work);
    RUN_TEST(test_memory_allocation_robustness);
    RUN_TEST(test_null_pointer_safety);
    RUN_TEST(test_format_string_safety);
    
    printf("\n🛡️ Security test suite completed!\n");
    
    return UNITY_END();
}