#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>
#include <string.h>

#include "config.h"
#include "parser/parser.h"
#include "runtime/evaluator.h"
#include "utils/output_buffer.h"
#include "generator/gcode_emitter.h"
#include "utils/file_utils.h"
#include "utils/report.h"


#include <libgen.h>  // for basename()






// Declare the runtime variable lookup functions from evaluator.c
extern double get_var(const char* name);
extern int var_exists(const char* name);

const char* GGCODE_INPUT_FILENAME = NULL;


void compile_file(const char* input_path, const char* output_path, int debug) {

     GGCODE_INPUT_FILENAME = input_path;  // Save for later use
    long input_size_bytes = 0;

        GGCODE_INPUT_FILENAME = input_path;

    // Store filename only (no path)
    strncpy(RUNTIME_FILENAME, basename((char*)input_path), sizeof(RUNTIME_FILENAME) - 1);
    RUNTIME_FILENAME[sizeof(RUNTIME_FILENAME) - 1] = '\0';

    // Get current time string
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(RUNTIME_TIME, sizeof(RUNTIME_TIME), "%Y-%m-%d %H:%M:%S", tm_info);


    char* source = read_file_to_buffer(input_path, &input_size_bytes);
    if (!source) {
        perror("Failed to read input file");
        exit(1);
    }

    init_output_buffer();

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    ASTNode* root = parse_script(source);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double parse_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // ➤ Emit G-code (this defines variables like 'id')
    clock_gettime(CLOCK_MONOTONIC, &start);
    emit_gcode(root, debug);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double emit_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

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

    // ➤ Insert preamble at start of output buffer
    prepend_to_output_buffer(preamble);

    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    long memory_kb = usage.ru_maxrss;

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
