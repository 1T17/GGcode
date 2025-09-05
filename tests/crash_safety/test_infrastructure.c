#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "config/crash_test_config.h"
#include "framework/crash_detection.h"
#include "framework/test_runner.h"

// Unity framework functions
void setUp(void) {
    // Setup for each test
}

void tearDown(void) {
    // Cleanup after each test
}

// Test functions for infrastructure verification
void test_config_creation(void) {
    CrashTestConfig* config = create_default_config();
    CRASH_SAFE_ASSERT_NOT_NULL(config, "Config creation failed");
    
    CRASH_SAFE_ASSERT_EQUAL(DEFAULT_MAX_FILE_SIZE_MB, config->max_file_size_mb, 
                            "Default file size incorrect");
    CRASH_SAFE_ASSERT_EQUAL(DEFAULT_MAX_NESTING_DEPTH, config->max_nesting_depth,
                            "Default nesting depth incorrect");
    
    free_config(config);
}

void test_crash_detection_setup(void) {
    // Test crash handler setup
    setup_crash_handlers();
    
    // Verify initial state
    CRASH_SAFE_ASSERT(!is_crash_detected(), "Crash should not be detected initially");
    CRASH_SAFE_ASSERT_EQUAL(0, get_crash_signal(), "Initial signal should be 0");
    
    cleanup_crash_handlers();
}

void test_result_tracking(void) {
    TestResult* result = create_test_result("test_infrastructure");
    CRASH_SAFE_ASSERT_NOT_NULL(result, "Test result creation failed");
    
    CRASH_SAFE_ASSERT(!result->passed, "Initial result should be false");
    CRASH_SAFE_ASSERT(!result->crashed, "Initial crashed should be false");
    CRASH_SAFE_ASSERT_EQUAL(0, result->exit_code, "Initial exit code should be 0");
    
    free_test_result(result);
}

void test_summary_functionality(void) {
    TestSummary* summary = create_test_summary();
    CRASH_SAFE_ASSERT_NOT_NULL(summary, "Test summary creation failed");
    
    CRASH_SAFE_ASSERT_EQUAL(0, summary->result_count, "Initial result count should be 0");
    CRASH_SAFE_ASSERT_EQUAL(0, summary->passed_count, "Initial passed count should be 0");
    
    // Add a test result
    TestResult result = {0};
    strcpy(result.test_name, "dummy_test");
    result.passed = true;
    result.execution_time_sec = 1.5;
    
    add_result_to_summary(summary, &result);
    
    CRASH_SAFE_ASSERT_EQUAL(1, summary->result_count, "Result count should be 1");
    CRASH_SAFE_ASSERT_EQUAL(1, summary->passed_count, "Passed count should be 1");
    
    free_test_summary(summary);
}

void test_signal_utilities(void) {
    // Test signal to string conversion
    const char* sigsegv_str = signal_to_string(SIGSEGV);
    CRASH_SAFE_ASSERT_NOT_NULL(sigsegv_str, "Signal string should not be NULL");
    
    // Test fatal signal detection
    CRASH_SAFE_ASSERT(is_fatal_signal(SIGSEGV), "SIGSEGV should be fatal");
    CRASH_SAFE_ASSERT(is_fatal_signal(SIGABRT), "SIGABRT should be fatal");
    CRASH_SAFE_ASSERT(!is_fatal_signal(SIGTERM), "SIGTERM should not be fatal");
}

// Test suite setup
int main(void) {
    printf("=== Crash Safety Infrastructure Test ===\n");
    
    // Verify test prerequisites
    if (!verify_test_prerequisites()) {
        printf("FAILED: Test prerequisites not met\n");
        return 1;
    }
    
    printf("✓ Test prerequisites verified\n");
    
    // Create test suite
    CrashTestConfig* config = create_default_config();
    config->verbose_output = true;
    
    TestSuite* suite = create_test_suite("Infrastructure Tests", config);
    if (!suite) {
        printf("FAILED: Could not create test suite\n");
        return 1;
    }
    
    // Add test cases
    TestCase test_cases[] = {
        DEFINE_TEST_CASE("config_creation", "Test configuration creation", 
                        test_config_creation, HIGH_PRIORITY, "8.1"),
        DEFINE_TEST_CASE("crash_detection", "Test crash detection setup", 
                        test_crash_detection_setup, HIGH_PRIORITY, "8.2"),
        DEFINE_TEST_CASE("result_tracking", "Test result tracking", 
                        test_result_tracking, HIGH_PRIORITY, "8.3"),
        DEFINE_TEST_CASE("summary_functionality", "Test summary functionality", 
                        test_summary_functionality, HIGH_PRIORITY, "8.1"),
        DEFINE_TEST_CASE("signal_utilities", "Test signal utilities", 
                        test_signal_utilities, MEDIUM_PRIORITY, "8.2")
    };
    
    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        if (!add_test_case(suite, &test_cases[i])) {
            printf("FAILED: Could not add test case: %s\n", test_cases[i].name);
            free_test_suite(suite);
            return 1;
        }
    }
    
    printf("✓ Test cases added to suite\n");
    
    // Run tests
    printf("\nRunning infrastructure tests...\n");
    bool all_passed = run_test_suite(suite);
    
    // Cleanup
    free_test_suite(suite);
    
    if (all_passed) {
        printf("\n🎉 All infrastructure tests PASSED!\n");
        printf("Crash safety testing infrastructure is ready.\n");
        return 0;
    } else {
        printf("\n❌ Some infrastructure tests FAILED!\n");
        return 1;
    }
}