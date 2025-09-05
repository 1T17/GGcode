#include "config.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "../runtime/runtime_state.h"

#include "../parser/ast_nodes.h"  // Needed for ASTNode
ASTNode *global_root_ast = NULL;
char *global_source_buffer = NULL;

// Global runtime instance
Runtime g_runtime = {0};

// Legacy global variables (for backward compatibility)
char RUNTIME_TIME[64] = "";
char RUNTIME_FILENAME[256] = "";

static const char* input_file = NULL;

// Internal static variables
static int line_number = DEFAULT_LINE_NUMBER;
static int enable_n_lines = DEFAULT_ENABLE_N_LINES;
static int decimal_places = 3;  // Default decimal places for G-code coordinates

// Initialize runtime state
void init_runtime() {
    memset(&g_runtime, 0, sizeof(Runtime));

    // statement_count is now managed in runtime state
    g_runtime.var_count = 0;
    g_runtime.function_count = 0;
    g_runtime.current_scope_level = 0;
    
    // Initialize recursion protection
    g_runtime.recursion_depth = 0;
    g_runtime.max_recursion_depth = 100;  // Default limit
}

// Get runtime instance
Runtime* get_runtime() {
    return &g_runtime;
}

const char* get_input_file() {
    return input_file;
}




int get_output_to_file() {
    return DEFAULT_OUTPUT_TO_FILE;
}

int get_line_number() {
    return line_number;
}

void increment_line_number() {
    line_number += 5;
}

void reset_line_number() {
    line_number = DEFAULT_LINE_NUMBER;
}

int get_enable_n_lines() {
    return enable_n_lines;
}

// Configuration variable support
void set_decimal_places(int places) {
    if (places >= 0 && places <= 6) {
        decimal_places = places;
    }
}

int get_decimal_places(void) {
    return decimal_places;
}

void set_enable_n_lines_from_var(int enable) {
    enable_n_lines = (enable != 0);
}

void reset_config_state(void) {
    decimal_places = 3;  // Reset to default
    enable_n_lines = DEFAULT_ENABLE_N_LINES;
    line_number = DEFAULT_LINE_NUMBER;
}

const char* get_decimal_format(void) {
    static char format[8];
    snprintf(format, sizeof(format), "%%.%df", decimal_places);
    return format;
}