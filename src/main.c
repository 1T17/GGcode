#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <string.h>

#include "config.h"
#include "parser/parser.h"
#include "runtime/evaluator.h"
#include "utils/output_buffer.h"
#include "generator/gcode_emitter.h"
#include "utils/file_utils.h"
#include "utils/report.h"

#include "utils/time_utils.h"



#include <libgen.h>  // for basename()

#ifdef __linux__
#include <sys/resource.h>
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


// Declare the runtime variable lookup functions from evaluator.c
extern double get_var(const char* name);
extern int var_exists(const char* name);

const char* GGCODE_INPUT_FILENAME = NULL;


void compile_file(const char* input_path, const char* output_path, int debug) {
    GGCODE_INPUT_FILENAME = input_path;
    long input_size_bytes = 0;

    // Store filename only (no path)
#if defined(_WIN32)
    const char* last_slash = strrchr(input_path, '\\');
    const char* filename = last_slash ? last_slash + 1 : input_path;
#else
    const char* filename = basename((char*)input_path);
#endif
    strncpy(RUNTIME_FILENAME, filename, sizeof(RUNTIME_FILENAME) - 1);
    RUNTIME_FILENAME[sizeof(RUNTIME_FILENAME) - 1] = '\0';

    // Get current time string
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(RUNTIME_TIME, sizeof(RUNTIME_TIME), "%Y-%m-%d %H:%M:%S", tm_info);

    // Load source
    char* source = read_file_to_buffer(input_path, &input_size_bytes);
    if (!source) {
        perror("Failed to read input file");
        exit(1);
    }

    init_output_buffer();

    // Parse timing
    Timer parse_timer;
    start_timer(&parse_timer);
    ASTNode* root = parse_script(source);
    double parse_time = end_timer(&parse_timer);

    // Emit timing
    Timer emit_timer;
    start_timer(&emit_timer);
    emit_gcode(root, debug);
    double emit_time = end_timer(&emit_timer);

    // ➤ Insert G-code header at the beginning AFTER emit
    char preamble[128] = "%\n";
    char id_line[64];
    if (var_exists("id")) {
        snprintf(id_line, sizeof(id_line), "%.0f", get_var("id"));
    } else {
        snprintf(id_line, sizeof(id_line), "000");
    }
    strcat(preamble, id_line);
    strcat(preamble, "\n");
    prepend_to_output_buffer(preamble);

    // Measure memory usage (only supported on Linux/macOS)
    long memory_kb = 0;
#if defined(__linux__) || defined(__APPLE__)
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    memory_kb = usage.ru_maxrss;
#endif

    // Output
    if (get_output_to_file()) {
        FILE* out = fopen(output_path, "w");
        if (!out) {
            perror("Failed to write output file");
        } else {
            fwrite(get_output_buffer(), 1, get_output_length(), out);
            fclose(out);
        }
    } else {
        printf("%s", get_output_buffer());
    }

    long gcode_size_bytes = get_output_length();
    print_compilation_report(input_size_bytes, gcode_size_bytes, parse_time, emit_time, memory_kb, get_statement_count());

    free_ast(root);
    free(source);
    free_output_buffer();
}






int main() {
    compile_file(get_input_file(), get_output_file(), get_debug());
    return 0;
}
