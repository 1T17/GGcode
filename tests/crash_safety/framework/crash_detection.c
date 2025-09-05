#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include "crash_detection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

// Global crash context
CrashContext g_crash_context = {0};

// Static variables for signal handling
static struct sigaction old_handlers[32];
static bool handlers_installed = false;
static CrashCallback crash_callback = NULL;
static TestCleanupCallback cleanup_callback = NULL;
static volatile sig_atomic_t timeout_flag = 0;

// Signal handler function
static void crash_signal_handler(int signal, siginfo_t* info, void* context __attribute__((unused))) {
    // Prevent recursive signal handling
    if (g_crash_context.in_signal_handler) {
        _exit(128 + signal);
    }
    
    g_crash_context.in_signal_handler = true;
    g_crash_context.crash_detected = true;
    g_crash_context.signal_number = signal;
    
    // Get crash address if available
    if (info) {
        g_crash_context.crash_address = info->si_addr;
    }
    
    // Generate crash message
    const char* signal_name = signal_to_string(signal);
    snprintf(g_crash_context.crash_message, sizeof(g_crash_context.crash_message),
             "Crash detected: %s (signal %d)", signal_name, signal);
    
    // Call cleanup callback if set
    if (cleanup_callback) {
        cleanup_callback();
    }
    
    // Call crash callback if set
    if (crash_callback) {
        crash_callback(signal, g_crash_context.crash_message);
    }
    
    // Log crash event
    fprintf(stderr, "CRASH: %s at address %p\n", 
            g_crash_context.crash_message, g_crash_context.crash_address);
    
    // Exit with signal-specific code
    _exit(128 + signal);
}

// Timeout signal handler
static void timeout_signal_handler(int signal __attribute__((unused))) {
    timeout_flag = 1;
}

// Setup crash handlers for common fatal signals
void setup_crash_handlers(void) {
    if (handlers_installed) {
        return;
    }
    
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = crash_signal_handler;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset(&sa.sa_mask);
    
    // Install handlers for fatal signals
    int fatal_signals[] = {
        SIGSEGV,  // Segmentation fault
        SIGABRT,  // Abort signal
        SIGFPE,   // Floating point exception
        SIGILL,   // Illegal instruction
        SIGBUS,   // Bus error
        SIGSYS,   // Bad system call
        0
    };
    
    for (int i = 0; fatal_signals[i] != 0; i++) {
        int sig = fatal_signals[i];
        if (sigaction(sig, &sa, &old_handlers[sig]) == -1) {
            fprintf(stderr, "Warning: Failed to install handler for signal %d\n", sig);
        }
    }
    
    handlers_installed = true;
    reset_crash_detection();
}

// Cleanup crash handlers
void cleanup_crash_handlers(void) {
    if (!handlers_installed) {
        return;
    }
    
    int fatal_signals[] = {SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGBUS, SIGSYS, 0};
    
    for (int i = 0; fatal_signals[i] != 0; i++) {
        int sig = fatal_signals[i];
        sigaction(sig, &old_handlers[sig], NULL);
    }
    
    handlers_installed = false;
}

// Reset crash detection state
void reset_crash_detection(void) {
    memset(&g_crash_context, 0, sizeof(g_crash_context));
    timeout_flag = 0;
}

// Crash detection state queries
bool is_crash_detected(void) {
    return g_crash_context.crash_detected;
}

int get_crash_signal(void) {
    return g_crash_context.signal_number;
}

const char* get_crash_message(void) {
    return g_crash_context.crash_message;
}

void* get_crash_address(void) {
    return g_crash_context.crash_address;
}

// Convert signal number to string
const char* signal_to_string(int signal) {
    switch (signal) {
        case SIGSEGV: return "SIGSEGV (Segmentation fault)";
        case SIGABRT: return "SIGABRT (Abort)";
        case SIGFPE:  return "SIGFPE (Floating point exception)";
        case SIGILL:  return "SIGILL (Illegal instruction)";
        case SIGBUS:  return "SIGBUS (Bus error)";
        case SIGSYS:  return "SIGSYS (Bad system call)";
        case SIGALRM: return "SIGALRM (Timeout)";
        case SIGTERM: return "SIGTERM (Termination)";
        case SIGINT:  return "SIGINT (Interrupt)";
        default:      return "Unknown signal";
    }
}

// Check if signal is fatal
bool is_fatal_signal(int signal) {
    switch (signal) {
        case SIGSEGV:
        case SIGABRT:
        case SIGFPE:
        case SIGILL:
        case SIGBUS:
        case SIGSYS:
            return true;
        default:
            return false;
    }
}

// Execute test with crash protection
TestExecutionResult execute_test_with_protection(
    const char* test_command,
    int timeout_seconds,
    TestCleanupCallback cleanup_func) {
    
    TestExecutionResult result = {0};
    
    if (!test_command) {
        result.exit_code = -1;
        strcpy(result.error_output, "Invalid test command");
        return result;
    }
    
    // Record start time
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    
    // Fork process for test execution
    pid_t pid = fork();
    if (pid == -1) {
        result.exit_code = -1;
        snprintf(result.error_output, sizeof(result.error_output),
                 "Failed to fork process: %s", strerror(errno));
        return result;
    }
    
    if (pid == 0) {
        // Child process - execute test
        setup_crash_handlers();
        
        if (cleanup_func) {
            cleanup_callback = cleanup_func;
        }
        
        // Execute the test command
        int exec_result = system(test_command);
        exit(WEXITSTATUS(exec_result));
    } else {
        // Parent process - wait for completion with timeout
        setup_timeout_handler(timeout_seconds);
        
        int status;
        pid_t wait_result = waitpid(pid, &status, 0);
        
        cancel_timeout_handler();
        
        // Calculate execution time
        gettimeofday(&end_time, NULL);
        result.execution_time = (end_time.tv_sec - start_time.tv_sec) +
                               (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
        
        if (wait_result == -1) {
            if (errno == EINTR && timeout_flag) {
                // Timeout occurred
                kill(pid, SIGKILL);
                waitpid(pid, NULL, 0);
                result.exit_code = -1;
                result.crashed = true;
                strcpy(result.error_output, "Test timed out");
            } else {
                result.exit_code = -1;
                snprintf(result.error_output, sizeof(result.error_output),
                         "Wait failed: %s", strerror(errno));
            }
        } else {
            if (WIFEXITED(status)) {
                result.exit_code = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                result.crashed = true;
                result.signal_received = WTERMSIG(status);
                result.exit_code = 128 + result.signal_received;
                snprintf(result.error_output, sizeof(result.error_output),
                         "Process terminated by signal %d (%s)",
                         result.signal_received, 
                         signal_to_string(result.signal_received));
            }
        }
    }
    
    return result;
}

// Setup timeout handler
void setup_timeout_handler(int timeout_seconds) {
    if (timeout_seconds <= 0) {
        return;
    }
    
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = timeout_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    sigaction(SIGALRM, &sa, NULL);
    alarm(timeout_seconds);
    timeout_flag = 0;
}

// Cancel timeout handler
void cancel_timeout_handler(void) {
    alarm(0);
    signal(SIGALRM, SIG_DFL);
}

// Check if timeout exceeded
bool is_timeout_exceeded(void) {
    return timeout_flag != 0;
}

// Generate crash report
void generate_crash_report(const TestResult* result, const char* output_file) {
    if (!result || !output_file) {
        return;
    }
    
    FILE* file = fopen(output_file, "w");
    if (!file) {
        fprintf(stderr, "Failed to create crash report file: %s\n", output_file);
        return;
    }
    
    fprintf(file, "=== CRASH SAFETY TEST REPORT ===\n");
    fprintf(file, "Test Name: %s\n", result->test_name);
    fprintf(file, "Timestamp: %s", ctime(&(time_t){time(NULL)}));
    fprintf(file, "Status: %s\n", result->passed ? "PASSED" : "FAILED");
    fprintf(file, "Crashed: %s\n", result->crashed ? "YES" : "NO");
    
    if (result->crashed) {
        fprintf(file, "Signal Received: %d (%s)\n", 
                result->signal_received,
                signal_to_string(result->signal_received));
    }
    
    fprintf(file, "Exit Code: %d\n", result->exit_code);
    fprintf(file, "Execution Time: %.3f seconds\n", result->execution_time_sec);
    fprintf(file, "Memory Usage: %zu KB\n", result->memory_usage_kb);
    
    if (result->memory_leak_detected) {
        fprintf(file, "Memory Leak: DETECTED\n");
    }
    
    if (result->buffer_overflow_detected) {
        fprintf(file, "Buffer Overflow: DETECTED\n");
    }
    
    if (strlen(result->error_message) > 0) {
        fprintf(file, "Error Message: %s\n", result->error_message);
    }
    
    fprintf(file, "================================\n");
    
    fclose(file);
}

// Memory tracking (basic implementation)
void enable_memory_tracking(void) {
    // Placeholder - would integrate with memory profiling tools
}

void disable_memory_tracking(void) {
    // Placeholder - would disable memory profiling
}

bool check_memory_corruption(void) {
    // Placeholder - would check for memory corruption
    return false;
}

size_t get_current_memory_usage(void) {
    // Placeholder - would return actual memory usage
    // In a real implementation, this would read from /proc/self/status or similar
    return 0;
}

// Stack overflow detection (basic implementation)
void setup_stack_overflow_detection(void) {
    // Placeholder - would setup stack monitoring
}

bool check_stack_overflow(void) {
    // Placeholder - would check stack usage
    return false;
}

size_t get_stack_usage(void) {
    // Placeholder - would return stack usage
    return 0;
}

// Log crash event
void log_crash_event(const char* test_name, int signal, const char* message) {
    FILE* log_file = fopen("tests/crash_safety/results/crash_log.txt", "a");
    if (!log_file) {
        return;
    }
    
    time_t now = time(NULL);
    fprintf(log_file, "[%s] Test: %s, Signal: %d, Message: %s\n",
            ctime(&now), test_name ? test_name : "unknown", signal, 
            message ? message : "no message");
    
    fclose(log_file);
}