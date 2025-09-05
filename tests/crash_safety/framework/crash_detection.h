#ifndef CRASH_DETECTION_H
#define CRASH_DETECTION_H

#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include "../config/crash_test_config.h"

// Signal handling context
typedef struct {
    bool crash_detected;
    int signal_number;
    char crash_message[256];
    void* crash_address;
    bool in_signal_handler;
} CrashContext;

// Crash detection callbacks
typedef void (*CrashCallback)(int signal, const char* message);
typedef void (*TestCleanupCallback)(void);

// Signal handler setup and management
void setup_crash_handlers(void);
void cleanup_crash_handlers(void);
void reset_crash_detection(void);

// Crash detection state
bool is_crash_detected(void);
int get_crash_signal(void);
const char* get_crash_message(void);
void* get_crash_address(void);

// Signal handling utilities
const char* signal_to_string(int signal);
bool is_fatal_signal(int signal);
void install_signal_handler(int signal);
void restore_default_handler(int signal);

// Test execution with crash protection
typedef struct {
    int exit_code;
    bool crashed;
    int signal_received;
    double execution_time;
    char error_output[512];
} TestExecutionResult;

TestExecutionResult execute_test_with_protection(
    const char* test_command,
    int timeout_seconds,
    TestCleanupCallback cleanup_func
);

// Memory tracking for crash detection
void enable_memory_tracking(void);
void disable_memory_tracking(void);
bool check_memory_corruption(void);
size_t get_current_memory_usage(void);

// Stack overflow detection
void setup_stack_overflow_detection(void);
bool check_stack_overflow(void);
size_t get_stack_usage(void);

// Timeout handling
void setup_timeout_handler(int timeout_seconds);
void cancel_timeout_handler(void);
bool is_timeout_exceeded(void);

// Test isolation utilities
pid_t fork_test_process(void);
bool wait_for_test_completion(pid_t pid, int* exit_code, int* signal);
void kill_test_process(pid_t pid);

// Crash report generation
void generate_crash_report(const TestResult* result, const char* output_file);
void log_crash_event(const char* test_name, int signal, const char* message);

// Global crash context (internal use)
extern CrashContext g_crash_context;

#endif // CRASH_DETECTION_H