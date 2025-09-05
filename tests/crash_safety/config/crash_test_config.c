#include "crash_test_config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Create default configuration
CrashTestConfig* create_default_config(void) {
    CrashTestConfig* config = malloc(sizeof(CrashTestConfig));
    if (!config) {
        return NULL;
    }
    
    // Set default values
    config->max_file_size_mb = DEFAULT_MAX_FILE_SIZE_MB;
    config->max_nesting_depth = DEFAULT_MAX_NESTING_DEPTH;
    config->max_array_elements = DEFAULT_MAX_ARRAY_ELEMENTS;
    config->enable_valgrind = false;
    config->enable_asan = false;
    config->memory_limit_mb = DEFAULT_MEMORY_LIMIT_MB;
    config->max_compilation_time_sec = DEFAULT_MAX_COMPILATION_TIME_SEC;
    config->max_memory_usage_mb = DEFAULT_MAX_MEMORY_USAGE_MB;
    config->cross_platform_mode = false;
    config->verbose_output = false;
    config->stop_on_first_failure = false;
    config->timeout_seconds = DEFAULT_TIMEOUT_SECONDS;
    
    // Set platform name
    strncpy(config->platform_name, "unknown", sizeof(config->platform_name) - 1);
    config->platform_name[sizeof(config->platform_name) - 1] = '\0';
    
    return config;
}

// Free configuration
void free_config(CrashTestConfig* config) {
    if (config) {
        free(config);
    }
}

// Load configuration from file (basic implementation)
bool load_config_from_file(const char* filename, CrashTestConfig* config) {
    if (!filename || !config) {
        return false;
    }
    
    FILE* file = fopen(filename, "r");
    if (!file) {
        return false;
    }
    
    // Simple key-value parser
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n') {
            continue;
        }
        
        // Parse key=value pairs
        char* equals = strchr(line, '=');
        if (!equals) {
            continue;
        }
        
        *equals = '\0';
        char* key = line;
        char* value = equals + 1;
        
        // Remove trailing newline
        char* newline = strchr(value, '\n');
        if (newline) {
            *newline = '\0';
        }
        
        // Set configuration values
        if (strcmp(key, "max_file_size_mb") == 0) {
            config->max_file_size_mb = (size_t)atoi(value);
        } else if (strcmp(key, "max_nesting_depth") == 0) {
            config->max_nesting_depth = atoi(value);
        } else if (strcmp(key, "enable_valgrind") == 0) {
            config->enable_valgrind = (strcmp(value, "true") == 0);
        } else if (strcmp(key, "verbose_output") == 0) {
            config->verbose_output = (strcmp(value, "true") == 0);
        }
    }
    
    fclose(file);
    return true;
}

// Save configuration to file
bool save_config_to_file(const char* filename, const CrashTestConfig* config) {
    if (!filename || !config) {
        return false;
    }
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        return false;
    }
    
    fprintf(file, "# Crash Safety Test Configuration\n");
    fprintf(file, "max_file_size_mb=%zu\n", config->max_file_size_mb);
    fprintf(file, "max_nesting_depth=%d\n", config->max_nesting_depth);
    fprintf(file, "max_array_elements=%zu\n", config->max_array_elements);
    fprintf(file, "enable_valgrind=%s\n", config->enable_valgrind ? "true" : "false");
    fprintf(file, "enable_asan=%s\n", config->enable_asan ? "true" : "false");
    fprintf(file, "memory_limit_mb=%zu\n", config->memory_limit_mb);
    fprintf(file, "max_compilation_time_sec=%.2f\n", config->max_compilation_time_sec);
    fprintf(file, "verbose_output=%s\n", config->verbose_output ? "true" : "false");
    fprintf(file, "timeout_seconds=%d\n", config->timeout_seconds);
    
    fclose(file);
    return true;
}

// Create test result
TestResult* create_test_result(const char* test_name) {
    TestResult* result = malloc(sizeof(TestResult));
    if (!result) {
        return NULL;
    }
    
    // Initialize result
    strncpy(result->test_name, test_name ? test_name : "unknown", 
            sizeof(result->test_name) - 1);
    result->test_name[sizeof(result->test_name) - 1] = '\0';
    
    reset_test_result(result);
    return result;
}

// Free test result
void free_test_result(TestResult* result) {
    if (result) {
        free(result);
    }
}

// Reset test result to initial state
void reset_test_result(TestResult* result) {
    if (!result) {
        return;
    }
    
    result->passed = false;
    result->crashed = false;
    result->memory_leak_detected = false;
    result->buffer_overflow_detected = false;
    result->execution_time_sec = 0.0;
    result->memory_usage_kb = 0;
    result->exit_code = 0;
    result->signal_received = 0;
    result->error_message[0] = '\0';
}

// Create test summary
TestSummary* create_test_summary(void) {
    TestSummary* summary = malloc(sizeof(TestSummary));
    if (!summary) {
        return NULL;
    }
    
    summary->results = NULL;
    summary->result_count = 0;
    summary->passed_count = 0;
    summary->failed_count = 0;
    summary->crashed_count = 0;
    summary->total_execution_time = 0.0;
    summary->total_memory_usage = 0;
    
    return summary;
}

// Free test summary
void free_test_summary(TestSummary* summary) {
    if (!summary) {
        return;
    }
    
    if (summary->results) {
        for (size_t i = 0; i < summary->result_count; i++) {
            // Results are not individually allocated, just free the array
        }
        free(summary->results);
    }
    
    free(summary);
}

// Add result to summary
void add_result_to_summary(TestSummary* summary, const TestResult* result) {
    if (!summary || !result) {
        return;
    }
    
    // Reallocate results array
    TestResult* new_results = realloc(summary->results, 
                                     (summary->result_count + 1) * sizeof(TestResult));
    if (!new_results) {
        return;
    }
    
    summary->results = new_results;
    
    // Copy result
    memcpy(&summary->results[summary->result_count], result, sizeof(TestResult));
    summary->result_count++;
    
    // Update counters
    if (result->passed) {
        summary->passed_count++;
    } else {
        summary->failed_count++;
    }
    
    if (result->crashed) {
        summary->crashed_count++;
    }
    
    summary->total_execution_time += result->execution_time_sec;
    summary->total_memory_usage += result->memory_usage_kb;
}

// Print test summary
void print_test_summary(const TestSummary* summary) {
    if (!summary) {
        printf("No test summary available\n");
        return;
    }
    
    printf("\n=== CRASH SAFETY TEST SUMMARY ===\n");
    printf("Total Tests: %zu\n", summary->result_count);
    printf("Passed: %zu\n", summary->passed_count);
    printf("Failed: %zu\n", summary->failed_count);
    printf("Crashed: %zu\n", summary->crashed_count);
    printf("Total Execution Time: %.2f seconds\n", summary->total_execution_time);
    printf("Total Memory Usage: %zu KB\n", summary->total_memory_usage);
    
    if (summary->result_count > 0) {
        double pass_rate = (double)summary->passed_count / summary->result_count * 100.0;
        printf("Pass Rate: %.1f%%\n", pass_rate);
    }
    
    printf("================================\n\n");
}