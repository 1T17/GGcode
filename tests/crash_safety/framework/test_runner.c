#define _GNU_SOURCE
#include "test_runner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

// Static variables
static ProgressCallback progress_callback = NULL;

// Create test suite
TestSuite* create_test_suite(const char* suite_name, CrashTestConfig* config) {
    if (!suite_name) {
        return NULL;
    }
    
    TestSuite* suite = malloc(sizeof(TestSuite));
    if (!suite) {
        return NULL;
    }
    
    suite->suite_name = strdup(suite_name);
    suite->test_cases = NULL;
    suite->test_count = 0;
    suite->config = config ? config : create_default_config();
    suite->summary = create_test_summary();
    
    if (!suite->summary) {
        free((void*)suite->suite_name);
        free(suite);
        return NULL;
    }
    
    return suite;
}

// Free test suite
void free_test_suite(TestSuite* suite) {
    if (!suite) {
        return;
    }
    
    if (suite->test_cases) {
        free(suite->test_cases);
    }
    
    if (suite->summary) {
        free_test_summary(suite->summary);
    }
    
    if (suite->config) {
        free_config(suite->config);
    }
    
    free((void*)suite->suite_name);
    free(suite);
}

// Add test case to suite
bool add_test_case(TestSuite* suite, const TestCase* test_case) {
    if (!suite || !test_case) {
        return false;
    }
    
    // Reallocate test cases array
    TestCase* new_cases = realloc(suite->test_cases, 
                                 (suite->test_count + 1) * sizeof(TestCase));
    if (!new_cases) {
        return false;
    }
    
    suite->test_cases = new_cases;
    
    // Copy test case (shallow copy for function pointers and strings)
    memcpy(&suite->test_cases[suite->test_count], test_case, sizeof(TestCase));
    suite->test_count++;
    
    return true;
}

// Find test case by name
TestCase* find_test_case(TestSuite* suite, const char* test_name) {
    if (!suite || !test_name) {
        return NULL;
    }
    
    for (size_t i = 0; i < suite->test_count; i++) {
        if (strcmp(suite->test_cases[i].name, test_name) == 0) {
            return &suite->test_cases[i];
        }
    }
    
    return NULL;
}

// Run single test with crash protection
static bool run_single_test_protected(TestSuite* suite, TestCase* test_case) {
    if (!suite || !test_case || !test_case->test_func) {
        return false;
    }
    
    TestResult* result = create_test_result(test_case->name);
    if (!result) {
        return false;
    }
    
    if (suite->config->verbose_output) {
        printf("Running test: %s\n", test_case->name);
    }
    
    // Record start time
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    
    // Setup crash detection
    setup_crash_handlers();
    reset_crash_detection();
    
    // Fork process for test execution
    pid_t pid = fork();
    if (pid == -1) {
        strcpy(result->error_message, "Failed to fork test process");
        result->passed = false;
        record_test_result(suite, result);
        free_test_result(result);
        return false;
    }
    
    if (pid == 0) {
        // Child process - run the test
        // Execute test function
        test_case->test_func();
        
        // If we get here, test passed
        exit(0);
    } else {
        // Parent process - wait for completion
        int status;
        int wait_result = waitpid(pid, &status, 0);
        
        // Calculate execution time
        gettimeofday(&end_time, NULL);
        result->execution_time_sec = (end_time.tv_sec - start_time.tv_sec) +
                                    (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
        
        if (wait_result == -1) {
            strcpy(result->error_message, "Failed to wait for test process");
            result->passed = false;
        } else if (WIFEXITED(status)) {
            result->exit_code = WEXITSTATUS(status);
            result->passed = (result->exit_code == 0);
        } else if (WIFSIGNALED(status)) {
            result->crashed = true;
            result->signal_received = WTERMSIG(status);
            result->exit_code = 128 + result->signal_received;
            result->passed = false;
            
            snprintf(result->error_message, sizeof(result->error_message),
                     "Test crashed with signal %d (%s)",
                     result->signal_received,
                     signal_to_string(result->signal_received));
        }
    }
    
    // Check for timeout
    if (result->execution_time_sec > suite->config->max_compilation_time_sec) {
        strcpy(result->error_message, "Test execution timeout");
        result->passed = false;
    }
    
    // Record result
    record_test_result(suite, result);
    
    if (suite->config->verbose_output) {
        printf("  Result: %s (%.3fs)\n", 
               result->passed ? "PASS" : "FAIL", 
               result->execution_time_sec);
        
        if (!result->passed && strlen(result->error_message) > 0) {
            printf("  Error: %s\n", result->error_message);
        }
    }
    
    bool test_passed = result->passed;
    free_test_result(result);
    
    return test_passed;
}

// Run entire test suite
bool run_test_suite(TestSuite* suite) {
    if (!suite) {
        return false;
    }
    
    printf("Running test suite: %s\n", suite->suite_name);
    printf("Total tests: %zu\n\n", suite->test_count);
    
    setup_test_environment();
    
    bool all_passed = true;
    size_t tests_run = 0;
    
    for (size_t i = 0; i < suite->test_count; i++) {
        TestCase* test_case = &suite->test_cases[i];
        
        if (!test_case->enabled) {
            continue;
        }
        
        tests_run++;
        
        // Report progress
        if (progress_callback) {
            progress_callback(test_case->name, tests_run, suite->test_count);
        }
        
        bool test_passed = run_single_test_protected(suite, test_case);
        
        if (!test_passed) {
            all_passed = false;
            
            if (suite->config->stop_on_first_failure) {
                printf("Stopping on first failure as requested\n");
                break;
            }
        }
    }
    
    cleanup_test_environment();
    
    printf("\n");
    print_test_results(suite);
    
    return all_passed;
}

// Run single test by name
bool run_single_test(TestSuite* suite, const char* test_name) {
    if (!suite || !test_name) {
        return false;
    }
    
    TestCase* test_case = find_test_case(suite, test_name);
    if (!test_case) {
        printf("Test not found: %s\n", test_name);
        return false;
    }
    
    if (!test_case->enabled) {
        printf("Test is disabled: %s\n", test_name);
        return false;
    }
    
    setup_test_environment();
    bool result = run_single_test_protected(suite, test_case);
    cleanup_test_environment();
    
    return result;
}

// Record test result
void record_test_result(TestSuite* suite, const TestResult* result) {
    if (!suite || !result || !suite->summary) {
        return;
    }
    
    add_result_to_summary(suite->summary, result);
}

// Print test results
void print_test_results(const TestSuite* suite) {
    if (!suite || !suite->summary) {
        return;
    }
    
    print_test_summary(suite->summary);
    
    // Print failed tests details
    if (suite->summary->failed_count > 0 || suite->summary->crashed_count > 0) {
        printf("Failed Tests:\n");
        for (size_t i = 0; i < suite->summary->result_count; i++) {
            const TestResult* result = &suite->summary->results[i];
            if (!result->passed) {
                printf("  - %s", result->test_name);
                if (result->crashed) {
                    printf(" (CRASHED - signal %d)", result->signal_received);
                }
                if (strlen(result->error_message) > 0) {
                    printf(": %s", result->error_message);
                }
                printf("\n");
            }
        }
        printf("\n");
    }
}

// Enable all tests
void enable_all_tests(TestSuite* suite) {
    if (!suite) {
        return;
    }
    
    for (size_t i = 0; i < suite->test_count; i++) {
        suite->test_cases[i].enabled = true;
    }
}

// Disable all tests
void disable_all_tests(TestSuite* suite) {
    if (!suite) {
        return;
    }
    
    for (size_t i = 0; i < suite->test_count; i++) {
        suite->test_cases[i].enabled = false;
    }
}

// Enable tests by priority
void enable_tests_by_priority(TestSuite* suite, int priority) {
    if (!suite) {
        return;
    }
    
    for (size_t i = 0; i < suite->test_count; i++) {
        suite->test_cases[i].enabled = (suite->test_cases[i].priority <= priority);
    }
}

// Setup test environment
void setup_test_environment(void) {
    // Create results directory if it doesn't exist
    system("mkdir -p tests/crash_safety/results");
    
    // Setup crash detection
    setup_crash_handlers();
    
    // Initialize any global test state
    reset_crash_detection();
}

// Cleanup test environment
void cleanup_test_environment(void) {
    cleanup_crash_handlers();
}

// Verify test prerequisites
bool verify_test_prerequisites(void) {
    // Check if we can create test directories
    if (access("tests/crash_safety", F_OK) != 0) {
        fprintf(stderr, "Error: crash_safety test directory not found\n");
        return false;
    }
    
    // Check if we can write to results directory
    system("mkdir -p tests/crash_safety/results");
    if (access("tests/crash_safety/results", W_OK) != 0) {
        fprintf(stderr, "Error: Cannot write to results directory\n");
        return false;
    }
    
    return true;
}

// Set progress callback
void set_progress_callback(ProgressCallback callback) {
    progress_callback = callback;
}