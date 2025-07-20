#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../config/config.h"
#include "../parser/parser.h"
#include "../runtime/evaluator.h"
#include "../utils/output_buffer.h"
#include "../generator/emitter.h"
#include "../error/error.h"
#include <time.h>

// Maximum input size to prevent buffer overflows
#define MAX_INPUT_SIZE (1024 * 1024) // 1MB limit

// Fixed function signature to match Node.js FFI binding
const char* compile_ggcode_from_string(const char* source_code) {
    clock_t start_time = clock();

    // Input validation
    if (!source_code) {
        return strdup("; ERROR: NULL input\n");
    }
    
    // Check input length to prevent buffer overflows
    size_t input_len = strlen(source_code);
    if (input_len == 0) {
        return strdup("; EMPTY INPUT\n");
    }
    
    if (input_len > MAX_INPUT_SIZE) {
        return strdup("; ERROR: Input too large (max 1MB)\n");
    }

    // Initialize runtime state properly (no global contamination)
    init_runtime();
    Runtime* runtime = get_runtime();

    runtime->statement_count = 0;  // Use runtime state instead of global

    // Initialize output buffer
    init_output_buffer();
    
    // Parse and compile
    ASTNode* root = parse_script_from_string(source_code);
    
    if (!root) {
        reset_runtime_state();
        free_output_buffer();
        return strdup("; ERROR: Parsing failed\n");
    }
    
    // Set runtime state variables for note block parsing BEFORE emit_gcode
    char ggcode_file_name[64];
    snprintf(ggcode_file_name, sizeof(ggcode_file_name), "nodejs.ggcode");
    
    Runtime *rt = get_runtime();
    strncpy(rt->RUNTIME_FILENAME, ggcode_file_name, sizeof(rt->RUNTIME_FILENAME) - 1);
    rt->RUNTIME_FILENAME[sizeof(rt->RUNTIME_FILENAME) - 1] = '\0';
    
    // Set current time
    char time_line[64];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(time_line, sizeof(time_line), "%Y-%m-%d %H:%M:%S", t);
    strncpy(rt->RUNTIME_TIME, time_line, sizeof(rt->RUNTIME_TIME) - 1);
    rt->RUNTIME_TIME[sizeof(rt->RUNTIME_TIME) - 1] = '\0';
    
    emit_gcode(root);

    // Generate G-code header
    emit_gcode_preamble(ggcode_file_name);

    // Get output and check for errors
    const char* output = strdup(get_output_buffer());
    long output_size = get_output_length();
    
    if (has_errors()) {
        report_error("[NodeJS] Compilation failed or errors detected");
        print_errors();
        clear_errors();
        free((void*)output);  // Free the output we just allocated
        reset_runtime_state();
        free_ast(root);
        free_output_buffer();
        return strdup("; ERROR: Compilation failed\n");
    }

    // Print basic compilation report for successful compilation
    clock_t end_time = clock();
    double elapsed = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000.0;
    fprintf(stderr, "GGcode: Compiled %zu bytes → %ld bytes in %.2fms\n", 
           input_len, output_size, elapsed);

    // Cleanup
    reset_runtime_state();
    free_ast(root);
    free_output_buffer();
    
    return output;
}

