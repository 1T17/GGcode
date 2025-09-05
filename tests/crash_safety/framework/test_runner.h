#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include "../config/crash_test_config.h"
#include "crash_detection.h"

// Test function pointer type
typedef void (*TestFunction)(void);

// Test case structure
typedef struct {
    const char* name;
    const char* description;
    TestFunction test_func;
    bool enabled;
    int priority;  // 1=high, 2=medium, 3=low
    const char* requirements[8];  // Referenced requirements
} TestCase;

// Test suite structure
typedef struct {
    const char* suite_name;
    TestCase* test_cases;
    size_t test_count;
    CrashTestConfig* config;
    TestSummary* summary;
} TestSuite;

// Test runner functions
TestSuite* create_test_suite(const char* suite_name, CrashTestConfig* config);
void free_test_suite(TestSuite* suite);

bool add_test_case(TestSuite* suite, const TestCase* test_case);
bool remove_test_case(TestSuite* suite, const char* test_name);
TestCase* find_test_case(TestSuite* suite, const char* test_name);

// Test execution
bool run_test_suite(TestSuite* suite);
bool run_single_test(TestSuite* suite, const char* test_name);
bool run_tests_by_priority(TestSuite* suite, int max_priority);

// Test result management
void record_test_result(TestSuite* suite, const TestResult* result);
void print_test_results(const TestSuite* suite);
bool save_test_results(const TestSuite* suite, const char* filename);

// Test filtering and selection
void enable_all_tests(TestSuite* suite);
void disable_all_tests(TestSuite* suite);
void enable_tests_by_priority(TestSuite* suite, int priority);
void enable_tests_by_pattern(TestSuite* suite, const char* pattern);

// Test execution utilities
void setup_test_environment(void);
void cleanup_test_environment(void);
bool verify_test_prerequisites(void);

// Progress reporting
typedef void (*ProgressCallback)(const char* test_name, int current, int total);
void set_progress_callback(ProgressCallback callback);

// Test macros for easier test definition
#define DEFINE_TEST_CASE(name, desc, func, prio, ...) \
    { name, desc, func, true, prio, {__VA_ARGS__, NULL} }

#define HIGH_PRIORITY 1
#define MEDIUM_PRIORITY 2
#define LOW_PRIORITY 3

// Standard test assertions with crash safety
#define CRASH_SAFE_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "ASSERTION FAILED: %s at %s:%d\n", \
                    message, __FILE__, __LINE__); \
            exit(1); \
        } \
    } while(0)

#define CRASH_SAFE_ASSERT_NOT_NULL(ptr, message) \
    CRASH_SAFE_ASSERT((ptr) != NULL, message)

#define CRASH_SAFE_ASSERT_EQUAL(expected, actual, message) \
    CRASH_SAFE_ASSERT((expected) == (actual), message)

// Memory tracking macros
#define TRACK_MEMORY_START() \
    size_t start_memory = get_current_memory_usage()

#define TRACK_MEMORY_END(result) \
    do { \
        size_t end_memory = get_current_memory_usage(); \
        (result)->memory_usage_kb = end_memory - start_memory; \
    } while(0)

#endif // TEST_RUNNER_H