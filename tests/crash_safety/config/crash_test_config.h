#ifndef CRASH_TEST_CONFIG_H
#define CRASH_TEST_CONFIG_H

#include <stddef.h>
#include <stdbool.h>

// Test configuration structure
typedef struct {
    // Stress testing limits
    size_t max_file_size_mb;
    int max_nesting_depth;
    size_t max_array_elements;
    
    // Memory testing parameters
    bool enable_valgrind;
    bool enable_asan;
    size_t memory_limit_mb;
    
    // Performance thresholds
    double max_compilation_time_sec;
    size_t max_memory_usage_mb;
    
    // Platform-specific settings
    bool cross_platform_mode;
    char platform_name[64];
    
    // Test execution settings
    bool verbose_output;
    bool stop_on_first_failure;
    int timeout_seconds;
} CrashTestConfig;

// Test result structure
typedef struct {
    char test_name[128];
    bool passed;
    bool crashed;
    bool memory_leak_detected;
    bool buffer_overflow_detected;
    double execution_time_sec;
    size_t memory_usage_kb;
    char error_message[512];
    int exit_code;
    int signal_received;
} TestResult;

// Test summary structure
typedef struct {
    TestResult* results;
    size_t result_count;
    size_t passed_count;
    size_t failed_count;
    size_t crashed_count;
    double total_execution_time;
    size_t total_memory_usage;
} TestSummary;

// Error types for crash detection
typedef enum {
    ERROR_NONE = 0,
    ERROR_CRASH,
    ERROR_MEMORY_LEAK,
    ERROR_BUFFER_OVERFLOW,
    ERROR_PERFORMANCE_REGRESSION,
    ERROR_TIMEOUT,
    ERROR_SIGNAL_RECEIVED,
    ERROR_INVALID_EXIT_CODE
} ErrorType;

// Default configuration values
#define DEFAULT_MAX_FILE_SIZE_MB 50
#define DEFAULT_MAX_NESTING_DEPTH 1000
#define DEFAULT_MAX_ARRAY_ELEMENTS 100000
#define DEFAULT_MEMORY_LIMIT_MB 512
#define DEFAULT_MAX_COMPILATION_TIME_SEC 30.0
#define DEFAULT_MAX_MEMORY_USAGE_MB 256
#define DEFAULT_TIMEOUT_SECONDS 60

// Function declarations
CrashTestConfig* create_default_config(void);
void free_config(CrashTestConfig* config);
bool load_config_from_file(const char* filename, CrashTestConfig* config);
bool save_config_to_file(const char* filename, const CrashTestConfig* config);

TestResult* create_test_result(const char* test_name);
void free_test_result(TestResult* result);
void reset_test_result(TestResult* result);

TestSummary* create_test_summary(void);
void free_test_summary(TestSummary* summary);
void add_result_to_summary(TestSummary* summary, const TestResult* result);
void print_test_summary(const TestSummary* summary);

#endif // CRASH_TEST_CONFIG_H