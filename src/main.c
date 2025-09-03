#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>

#ifdef _WIN32
#include <tchar.h>
#include <windows.h>
#include <psapi.h>
#else
#include <dirent.h>
#endif
#ifndef DT_REG
#define DT_REG 8
#endif

#ifdef __linux__
#include <sys/wait.h>
#include <sys/resource.h>
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "config/config.h"
#include "parser/parser.h"
#include "runtime/evaluator.h"
#include "utils/output_buffer.h"
#include "generator/emitter.h"
#include "utils/file_utils.h"
#include "utils/report.h"
#include "error/error.h"
#include "utils/time_utils.h"
#include "cli/cli.h"


#define FATAL_ERROR(msg, ...) fatal_error(NULL, 0, 0, msg, ##__VA_ARGS__)

// Declare the runtime variable lookup functions from evaluator.c
const char* GGCODE_INPUT_FILENAME = NULL;
void compile_file(const char* input_path, const char* output_path, bool quiet);
void compile_eval(const char* code);
void compile_all_files_cli(const CLIArgs* args);


void make_g_gcode_filename(const char *src, char *dst, size_t dst_size) {
    size_t len = strlen(src);
    if (len > 7 && strcmp(src + len - 7, ".ggcode") == 0) {
        snprintf(dst, dst_size, "%.*s.g.gcode", (int)(len - 7), src);
    } else {
        snprintf(dst, dst_size, "%s.g.gcode", src);
    }
}

// Helper to check if a string ends with .ggcode
int ends_with_ggcode(const char *filename) {
    size_t len = strlen(filename);
    return len > 7 && strcmp(filename + len - 7, ".ggcode") == 0;
}


void compile_file(const char* input_path, const char* output_path, bool quiet) {
    // Initialize runtime state
    init_runtime();
    Runtime* runtime = get_runtime();

    // Reset configuration state for clean compilation
    reset_config_state();

    GGCODE_INPUT_FILENAME = input_path;
    long input_size_bytes = 0;
    runtime->statement_count = 0;
    reset_runtime_state();

    // Store filename only (no path)
#if defined(_WIN32)
    const char* last_slash = strrchr(input_path, '\\');
    const char* filename = last_slash ? last_slash + 1 : input_path;
#else
    const char* filename = basename((char*)input_path);
#endif

    // Update runtime state
    strncpy(runtime->RUNTIME_FILENAME, filename, sizeof(runtime->RUNTIME_FILENAME) - 1);
    runtime->RUNTIME_FILENAME[sizeof(runtime->RUNTIME_FILENAME) - 1] = '\0';

    // Also update legacy globals for backward compatibility
    strncpy(RUNTIME_FILENAME, filename, sizeof(RUNTIME_FILENAME) - 1);
    RUNTIME_FILENAME[sizeof(RUNTIME_FILENAME) - 1] = '\0';

    // Get current time string
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(runtime->RUNTIME_TIME, sizeof(runtime->RUNTIME_TIME), "%Y-%m-%d %H:%M:%S", tm_info);

    // Also update legacy globals for backward compatibility
    strftime(RUNTIME_TIME, sizeof(RUNTIME_TIME), "%Y-%m-%d %H:%M:%S", tm_info);

    // Load source
    char* source = read_file_to_buffer(input_path, &input_size_bytes);
    if (!source) {
        if (!quiet) {
            fprintf(stderr, "Error: Failed to read input file '%s': %s\n", input_path, strerror(errno));
        }
        return;
    }

    init_output_buffer();

    // Parse timing
    Timer parse_timer;
    start_timer(&parse_timer);

    ASTNode* root = parse_script_from_string(source);
    double parse_time = end_timer(&parse_timer);

    // Emit timing
    Timer emit_timer;
    start_timer(&emit_timer);

 //   reset_line_number();
    emit_gcode(root);
    double emit_time = end_timer(&emit_timer);

    // ➤ Insert G-code header at the beginning AFTER emit
    emit_gcode_preamble(filename);

// Measure memory usage (Linux/macOS/Windows)
long memory_kb = 0;

#if defined(__linux__) || defined(__APPLE__)
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    memory_kb = usage.ru_maxrss;

#elif defined(_WIN32)
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        memory_kb = pmc.PeakWorkingSetSize / 1024;
    }
#endif

    // Output
    if (get_output_to_file()) {
        FILE* out = fopen(output_path, "w");
        if (!out) {
            if (!quiet) {
                fprintf(stderr, "Error: Failed to write output file '%s': %s\n", output_path, strerror(errno));
            }
        } else {
            fwrite(get_output_buffer(), 1, get_output_length(), out);
            fclose(out);
        }
    } else {
        printf("%s", get_output_buffer());
    }

    long gcode_size_bytes = get_output_length();

    if (!quiet) {
        print_compilation_report(input_size_bytes, gcode_size_bytes, parse_time, emit_time, memory_kb, runtime->statement_count);
    }

    free_ast(root);
    free(source);
    free_output_buffer();


if (has_errors()) {
    if (!quiet) {
        report_error("⚠️ Compilation finished with errors");
        print_errors();
    }
    clear_errors();  // Reset for next file (if compiling multiple)
}
}

void compile_eval(const char* code) {
    // Initialize runtime state
    init_runtime();
    Runtime* runtime = get_runtime();
    reset_config_state();
    
    GGCODE_INPUT_FILENAME = "<eval>";
    runtime->statement_count = 0;
    reset_runtime_state();
    
    // Set runtime info for eval mode
    strncpy(runtime->RUNTIME_FILENAME, "<eval>", sizeof(runtime->RUNTIME_FILENAME) - 1);
    runtime->RUNTIME_FILENAME[sizeof(runtime->RUNTIME_FILENAME) - 1] = '\0';
    strncpy(RUNTIME_FILENAME, "<eval>", sizeof(RUNTIME_FILENAME) - 1);
    RUNTIME_FILENAME[sizeof(RUNTIME_FILENAME) - 1] = '\0';
    
    // Get current time
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(runtime->RUNTIME_TIME, sizeof(runtime->RUNTIME_TIME), "%Y-%m-%d %H:%M:%S", tm_info);
    strftime(RUNTIME_TIME, sizeof(RUNTIME_TIME), "%Y-%m-%d %H:%M:%S", tm_info);
    
    init_output_buffer();
    
    // Parse and emit
    ASTNode* root = parse_script_from_string(code);
    if (root) {
        emit_gcode(root);
        
        // Output directly to terminal (no file)
        printf("%s", get_output_buffer());
        
        free_ast(root);
    }
    
    if (has_errors()) {
        fprintf(stderr, "\nErrors during evaluation:\n");
        print_errors();
        clear_errors();
    }
    
    free_output_buffer();
}

void compile_all_files_cli(const CLIArgs* args) {
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile("*.ggcode", &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "No .ggcode files found in current directory\n");
        return;
    }
    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            char* output_path = get_smart_output_path(findFileData.cFileName, args);
            if (!args->quiet) {
                printf("\033[38;5;208mGGCODE Compiling\033[0m \033[1m%s\033[0m → \033[1;32m%s\033[0m\n", 
                       findFileData.cFileName, output_path);
            }
            compile_file(findFileData.cFileName, output_path, args->quiet);
            free(output_path);
        }
    } while (FindNextFile(hFind, &findFileData) != 0);
    FindClose(hFind);
#else
    DIR *dir = opendir(".");
    if (!dir) {
        fprintf(stderr, "Error: Cannot open current directory: %s\n", strerror(errno));
        return;
    }
    
    struct dirent *entry;
    int file_count = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && ends_with_ggcode(entry->d_name)) {
            char* output_path = get_smart_output_path(entry->d_name, args);
            
            if (!args->quiet) {
                printf("\033[38;5;208mGGCODE Compiling\033[0m \033[1m%s\033[0m → \033[1;32m%s\033[0m\n", 
                       entry->d_name, output_path);
            }
            
            pid_t pid = fork();
            if (pid == 0) {
                // Child process
                compile_file(entry->d_name, output_path, args->quiet);
                exit(0);
            } else if (pid > 0) {
                // Parent process - wait for child
                int status;
                waitpid(pid, &status, 0);
                file_count++;
            } else {
                fprintf(stderr, "Error: fork failed: %s\n", strerror(errno));
            }
            
            free(output_path);
        }
    }
    closedir(dir);
    
    if (file_count == 0) {
        fprintf(stderr, "No .ggcode files found in current directory\n");
    } else if (!args->quiet) {
        printf("\nCompiled %d file%s\n", file_count, file_count == 1 ? "" : "s");
    }
#endif
}










































void compile_all_gg_files() {
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile("*.ggcode", &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        report_error("No .ggcode files found");
        return;
    }
    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
char out_name[MAX_PATH];
make_g_gcode_filename(findFileData.cFileName, out_name, sizeof(out_name));
printf("\nCompiling %s -> %s\n", findFileData.cFileName, out_name);

        compile_file(findFileData.cFileName, out_name);

        }
    } while (FindNextFile(hFind, &findFileData) != 0);
    FindClose(hFind);
#else
    DIR *dir = opendir(".");
    if (!dir) {
        report_error("opendir failed: %s", strerror(errno));
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && ends_with_ggcode(entry->d_name)) {

char out_name[272];
make_g_gcode_filename(entry->d_name, out_name, sizeof(out_name));
printf("\033[38;5;208m\nGGCODE Compiling\033[0m \033[1m%s\033[0m → \033[1;32m%s\033[0m\n", entry->d_name, out_name);




pid_t pid = fork();
if (pid == 0) {
    // 👶 Child process
            compile_file(entry->d_name, out_name, false);
    exit(0);  // Exit cleanly so parent doesn't run more compiles
} else if (pid > 0) {
    // 👴 Parent process — wait for the child
    int status;
    waitpid(pid, &status, 0);
} else {
    report_error("fork failed: %s", strerror(errno));
}





        }
    }
    closedir(dir);
#endif
}








int main(int argc, char* argv[]) {
    // Parse command line arguments
    CLIArgs* args = parse_arguments(argc, argv);
    if (!args) {
        return 1;
    }
    
    // Handle help and version
    if (args->show_help) {
        print_help();
        free_cli_args(args);
        return 0;
    }
    
    if (args->show_version) {
        print_version();
        free_cli_args(args);
        return 0;
    }
    
    // Handle eval mode
    if (args->eval_mode) {
        if (!args->eval_code) {
            fprintf(stderr, "Error: No code provided for evaluation\n");
            free_cli_args(args);
            return 1;
        }
        compile_eval(args->eval_code);
        free_cli_args(args);
        return 0;
    }
    
    // Handle compile all mode
    if (args->compile_all) {
        compile_all_files_cli(args);
        free_cli_args(args);
        return 0;
    }
    
    // Handle specific input files
    if (args->input_count > 0) {
        for (int i = 0; i < args->input_count; i++) {
            char* output_path = get_smart_output_path(args->input_files[i], args);
            
            if (!args->quiet && args->input_count > 1) {
                printf("\033[38;5;208mGGCODE Compiling\033[0m \033[1m%s\033[0m → \033[1;32m%s\033[0m\n", 
                       args->input_files[i], output_path);
            }
            
            compile_file(args->input_files[i], output_path, args->quiet);
            free(output_path);
        }
        free_cli_args(args);
        return 0;
    }
    
    // Default behavior: interactive file selection
    if (argc == 1) {
        interactive_file_selection(args);
        free_cli_args(args);
        return 0;
    }
    
    // If we get here, show help
    fprintf(stderr, "Error: No input files specified\n");
    fprintf(stderr, "Use 'ggcode --help' for usage information\n");
    free_cli_args(args);
    return 1;
}