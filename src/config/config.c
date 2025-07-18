#include "config.h"
#include <stddef.h>
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

// Initialize runtime state
void init_runtime() {
    memset(&g_runtime, 0, sizeof(Runtime));
    g_runtime.debug = get_debug();  // Use current debug setting
    g_runtime.statement_count = 0;
    g_runtime.var_count = 0;
    g_runtime.function_count = 0;
    g_runtime.current_scope_level = 0;
}

// Get runtime instance
Runtime* get_runtime() {
    return &g_runtime;
}

void set_input_file(const char* filename) {
    input_file = filename;
}

const char* get_input_file() {
    return input_file;
}


int get_debug() {
    return DEFAULT_DEBUG;
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