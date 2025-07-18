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


#define FATAL_ERROR(msg, ...) fatal_error(NULL, 0, 0, msg, ##__VA_ARGS__)

// Declare the runtime variable lookup functions from evaluator.c
const char* GGCODE_INPUT_FILENAME = NULL;
void compile_file(const char* input_path, const char* output_path, int debug);


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


void compile_file(const char* input_path, const char* output_path, int debug) {
    // Initialize runtime state
    init_runtime();
    Runtime* runtime = get_runtime();
    runtime->debug = debug;
    
    GGCODE_INPUT_FILENAME = input_path;
    long input_size_bytes = 0;
    statement_count = 0;
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
        report_error("Failed to read input file: %s", strerror(errno));
        FATAL_ERROR("Failed to read input file: %s", strerror(errno));
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
    emit_gcode(root, -1);  // Use runtime state for debug
    double emit_time = end_timer(&emit_timer);

    // ➤ Insert G-code header at the beginning AFTER emit
    emit_gcode_preamble(debug, filename);

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
            report_error("Failed to write output file: %s", strerror(errno));
        } else {
            fwrite(get_output_buffer(), 1, get_output_length(), out);
            fclose(out);
        }
    } else {
        printf("%s", get_output_buffer());
    }

    long gcode_size_bytes = get_output_length();


    print_compilation_report(input_size_bytes, gcode_size_bytes, parse_time, emit_time, memory_kb,statement_count);

    free_ast(root);
    free(source);
    free_output_buffer();


if (has_errors()) {
    report_error("⚠️ Compilation finished with errors");
    print_errors();
    clear_errors();  // Reset for next file (if compiling multiple)
}


}










































void compile_all_gg_files(int debug) {
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

compile_file(findFileData.cFileName, out_name, debug);

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
    compile_file(entry->d_name, out_name, debug);
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








int main() {
//int x =0 ;

//scanf("%d",&x);

    const char *input_file = get_input_file();
    int debug = get_debug();
if (input_file && strlen(input_file) > 0) {
    char out_name[272];
    make_g_gcode_filename(input_file, out_name, sizeof(out_name));
    compile_file(input_file, out_name, debug);
} else {
    compile_all_gg_files(debug);
}



    return 0;
}