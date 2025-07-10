#include "config.h"
#include <stddef.h>

#include "../parser/ast_nodes.h"  // Needed for ASTNode
ASTNode *global_root_ast = NULL;
char *global_source_buffer = NULL;

char RUNTIME_TIME[64] = "";
 char RUNTIME_FILENAME[256] = "";

static const char* input_file = NULL;

// Internal static variables
static int line_number = DEFAULT_LINE_NUMBER;
static int enable_n_lines = DEFAULT_ENABLE_N_LINES;

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