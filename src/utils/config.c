#include "config.h"


char RUNTIME_TIME[64] = "";
char RUNTIME_FILENAME[256] = "";


// Internal static variables
static int line_number = DEFAULT_LINE_NUMBER;
static int enable_n_lines = DEFAULT_ENABLE_N_LINES;

const char* get_input_file() {
    return DEFAULT_INPUT_FILE;
}

const char* get_output_file() {
    return DEFAULT_OUTPUT_FILE;
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
