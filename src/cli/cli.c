/**
 * @file cli.c
 * @brief Command Line Interface Implementation for GGcode Compiler
 * 
 * This file implements the complete CLI functionality for the GGcode compiler,
 * including argument parsing, interactive file selection, help system, and
 * smart output path generation with cross-platform support.
 * 
 * Key Features:
 * - Interactive file selection menu when no arguments provided
 * - Comprehensive argument parsing with validation
 * - Smart output path generation with fallback logic
 * - Cross-platform directory operations (Windows/Unix)
 * - Detailed help system with examples and workflows
 * 
 * @author GGcode Development Team
 * @version 1.0.9
 */

#include "cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <libgen.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

#ifndef DT_REG
#define DT_REG 8  /**< Regular file type for systems that don't define it */
#endif

/**
 * @brief Print comprehensive help message to stdout
 * 
 * Displays detailed usage information including:
 * - Command syntax and basic usage patterns
 * - All available options with descriptions
 * - Output control mechanisms
 * - Practical examples for common use cases
 * - Development and production workflows
 * 
 * The help is formatted for readability with clear sections and examples.
 */
void print_help(void) {
    printf("GGcode - Parametric G-code Compiler v%s\n\n", GGCODE_VERSION);
    printf("USAGE:\n");
    printf("    ggcode                                      # Interactive file selection menu\n");
    printf("    ggcode [OPTIONS] [FILES...]                # Compile specific files\n");
    printf("    ggcode -e|--eval \"GGCODE_EXPRESSION\"       # Direct evaluation\n\n");
    
    printf("ARGUMENTS:\n");
    printf("    FILES...    GGcode source files to compile (.ggcode extension)\n\n");
    
    printf("OPTIONS:\n");
    printf("    -a, --all               Compile all .ggcode files in current directory\n");
    printf("    -o, --output FILE       Specify exact output file (single file mode only)\n");
    printf("    --output-dir DIR        Set output directory (default: ./Gcode)\n");
    printf("    -e, --eval \"CODE\"       Execute GGcode directly to terminal (no files)\n");
    printf("    -q, --quiet             Suppress compilation reports and progress\n");
    printf("    -V, --verbose           Show detailed compilation information\n");
    printf("    -h, --help              Show this help message\n");
    printf("    -v, --version           Show version information\n\n");
    
    printf("OUTPUT CONTROL:\n");
    printf("    Default: Files output to ./Gcode/ directory (auto-created)\n");
    printf("    Fallback: Current directory if ./Gcode/ cannot be created\n");
    printf("    Override: Use -o for specific file or --output-dir for custom directory\n");
    printf("    Format: input.ggcode → output.g.gcode\n\n");
    
    printf("BASIC EXAMPLES:\n");
    printf("    ggcode                                      → Interactive menu to select files\n");
    printf("    ggcode part.ggcode                          → ./Gcode/part.g.gcode\n");
    printf("    ggcode part1.ggcode part2.ggcode            → Multiple files to ./Gcode/\n");
    printf("    ggcode *.ggcode                             → All matching files\n\n");
    
    printf("OUTPUT CONTROL EXAMPLES:\n");
    printf("    ggcode -o custom.gcode part.ggcode          → custom.gcode\n");
    printf("    ggcode --output-dir ./build part.ggcode     → ./build/part.g.gcode\n");
    printf("    ggcode -a --output-dir ./output             → All files to ./output/\n");
    printf("    ggcode -q part.ggcode                       → Silent compilation\n\n");
    
    printf("DIRECT EVALUATION (Testing & Development):\n");
    printf("    ggcode -e \"G1 X10 Y20 F300\"                → Test G-code commands\n");
    printf("    ggcode -e \"let r=5; note {Radius: [r]}\"     → Test variables & expressions\n");
    printf("    ggcode -e \"for i=1..5 { G1 X[i*10] Y0 }\"   → Test loops and math\n");
    printf("    ggcode -e \"let x=cos(45*DEG_TO_RAD)*10\"     → Test math functions\n\n");
    
    printf("COMMON WORKFLOWS:\n");
    printf("    # Development workflow\n");
    printf("    ggcode -e \"test_expression\"                → Quick testing\n");
    printf("    ggcode mypart.ggcode                        → Compile & check output\n");
    printf("    ggcode -q -a                               → Batch compile silently\n\n");
    
    printf("    # Production workflow\n");
    printf("    ggcode --output-dir ./production *.ggcode  → Organized output\n");
    printf("    ggcode -o final.gcode optimized.ggcode     → Specific naming\n\n");
    
    printf("For more information, visit: https://github.com/1T17/GGcode\n");
}

/**
 * @brief Print version information to stdout
 * 
 * Displays:
 * - Current version number from GGCODE_VERSION
 * - Program description
 * - Build timestamp (compile date and time)
 */
void print_version(void) {
    printf("GGcode v%s\n", GGCODE_VERSION);
    printf("Parametric G-code Compiler\n");
    printf("Built: %s %s\n", __DATE__, __TIME__);
}

/**
 * @brief Create directory if it doesn't exist
 * 
 * Attempts to create the specified directory with permissions 0755.
 * Uses stat() to check if directory already exists before attempting creation.
 * 
 * @param path Directory path to create (must not be NULL)
 * @return 0 on success or if directory already exists, -1 on failure
 * 
 * @note This function is safe to call on existing directories
 * @note Uses standard Unix permissions (0755) for created directories
 */
int create_directory_if_needed(const char* path) {
    struct stat st = {0};
    
    if (stat(path, &st) == -1) {
        // Directory doesn't exist, try to create it
        if (mkdir(path, 0755) == -1) {
            return -1; // Failed to create
        }
    }
    return 0; // Success or already exists
}

/**
 * @brief Generate smart output path for input file
 * 
 * Creates an appropriate output path based on input file and CLI arguments.
 * Implements intelligent path resolution with the following priority:
 * 1. Use explicit output file if specified (-o option)
 * 2. Use custom output directory if specified (--output-dir)
 * 3. Default to ./Gcode/ directory (created if needed)
 * 4. Fallback to current directory if ./Gcode/ cannot be created
 * 
 * File naming convention:
 * - input.ggcode → output.g.gcode
 * - other.ext → other.ext.g.gcode
 * 
 * @param input_file Path to input .ggcode file (must not be NULL)
 * @param args Parsed CLI arguments containing output preferences (must not be NULL)
 * @return Allocated string containing output path, or NULL on memory allocation failure
 * 
 * @note Caller is responsible for freeing the returned string
 * @note Performs write permission test and falls back to current directory if needed
 */
char* get_smart_output_path(const char* input_file, const CLIArgs* args) {
    char* output_path = malloc(MAX_OUTPUT_PATH);
    if (!output_path) return NULL;
    
    // If specific output file is given, use it
    if (args->output_file) {
        strncpy(output_path, args->output_file, MAX_OUTPUT_PATH - 1);
        output_path[MAX_OUTPUT_PATH - 1] = '\0';
        return output_path;
    }
    
    // Determine output directory
    const char* output_dir = args->output_dir ? args->output_dir : "./Gcode";
    
    // Try to create output directory, fallback to current directory
    if (create_directory_if_needed(output_dir) != 0) {
        output_dir = ".";
    }
    
    // Generate output filename
    char base_name[256];
    strncpy(base_name, basename((char*)input_file), sizeof(base_name) - 1);
    base_name[sizeof(base_name) - 1] = '\0';
    
    // Convert .ggcode to .g.gcode
    size_t len = strlen(base_name);
    if (len > 7 && strcmp(base_name + len - 7, ".ggcode") == 0) {
        snprintf(output_path, MAX_OUTPUT_PATH, "%s/%.*s.g.gcode", 
                output_dir, (int)(len - 7), base_name);
    } else {
        snprintf(output_path, MAX_OUTPUT_PATH, "%s/%s.g.gcode", 
                output_dir, base_name);
    }
    
    // Test if we can actually write to this path by trying to create a temp file
    char test_path[MAX_OUTPUT_PATH];
    snprintf(test_path, sizeof(test_path), "%s/.ggcode_test", output_dir);
    FILE* test_file = fopen(test_path, "w");
    if (test_file) {
        fclose(test_file);
        unlink(test_path);  // Remove test file
    } else if (strcmp(output_dir, ".") != 0) {
        // If we can't write to the intended directory and it's not current dir, fallback
        output_dir = ".";
        if (len > 7 && strcmp(base_name + len - 7, ".ggcode") == 0) {
            snprintf(output_path, MAX_OUTPUT_PATH, "%s/%.*s.g.gcode", 
                    output_dir, (int)(len - 7), base_name);
        } else {
            snprintf(output_path, MAX_OUTPUT_PATH, "%s/%s.g.gcode", 
                    output_dir, base_name);
        }
    }
    
    return output_path;
}

/**
 * @brief Parse command line arguments into CLIArgs structure
 * 
 * Parses argc/argv into a structured format, handling all supported options:
 * - Help and version flags (-h, --help, -v, --version)
 * - Compilation modes (-a, --all for all files)
 * - Output control (-o, --output, --output-dir)
 * - Evaluation mode (-e, --eval)
 * - Verbosity control (-q, --quiet, -V, --verbose)
 * - Input file arguments
 * 
 * Performs validation of arguments and provides helpful error messages.
 * 
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return Pointer to allocated CLIArgs structure, or NULL on error
 * 
 * @note Caller is responsible for freeing returned structure with free_cli_args()
 * @note Prints error messages to stderr for invalid arguments
 * @note Returns NULL and prints usage hint for unknown options
 */
CLIArgs* parse_arguments(int argc, char* argv[]) {
    CLIArgs* args = calloc(1, sizeof(CLIArgs));
    if (!args) return NULL;
    
    // Initialize defaults
    args->input_files = malloc(argc * sizeof(char*));
    if (!args->input_files) {
        free(args);
        return NULL;
    }
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            args->show_help = true;
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            args->show_version = true;
        }
        else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0) {
            args->compile_all = true;
        }
        else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0) {
            args->quiet = true;
        }
        else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--verbose") == 0) {
            args->verbose = true;
        }
        else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                // Validate output filename length and characters
                if (strlen(argv[i + 1]) >= MAX_OUTPUT_PATH) {
                    fprintf(stderr, "Error: Output filename too long (max %d characters)\n", MAX_OUTPUT_PATH - 1);
                    free_cli_args(args);
                    return NULL;
                }
                // Check for path traversal attempts
                if (strstr(argv[i + 1], "../") || strstr(argv[i + 1], "..\\")) {
                    fprintf(stderr, "Error: Path traversal not allowed in output filename\n");
                    free_cli_args(args);
                    return NULL;
                }
                args->output_file = strdup(argv[++i]);
                args->has_custom_output = true;
            } else {
                fprintf(stderr, "Error: -o/--output requires a filename\n");
                free_cli_args(args);
                return NULL;
            }
        }
        else if (strcmp(argv[i], "--output-dir") == 0) {
            if (i + 1 < argc) {
                // Validate directory path length
                if (strlen(argv[i + 1]) >= MAX_OUTPUT_PATH) {
                    fprintf(stderr, "Error: Directory path too long (max %d characters)\n", MAX_OUTPUT_PATH - 1);
                    free_cli_args(args);
                    return NULL;
                }
                // Check for excessive path traversal attempts
                if (strstr(argv[i + 1], "../../../") || strstr(argv[i + 1], "..\\..\\..\\")) {
                    fprintf(stderr, "Error: Excessive path traversal not allowed\n");
                    free_cli_args(args);
                    return NULL;
                }
                args->output_dir = strdup(argv[++i]);
            } else {
                fprintf(stderr, "Error: --output-dir requires a directory path\n");
                free_cli_args(args);
                return NULL;
            }
        }
        else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--eval") == 0) {
            if (i + 1 < argc) {
                // Validate eval code length
                if (strlen(argv[i + 1]) >= MAX_EVAL_CODE) {
                    fprintf(stderr, "Error: Evaluation code too long (max %d characters)\n", MAX_EVAL_CODE - 1);
                    free_cli_args(args);
                    return NULL;
                }
                args->eval_mode = true;
                args->eval_code = strdup(argv[++i]);
            } else {
                fprintf(stderr, "Error: -e/--eval requires GGcode expression\n");
                free_cli_args(args);
                return NULL;
            }
        }
        else if (argv[i][0] == '-') {
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
            fprintf(stderr, "Use 'ggcode --help' for usage information\n");
            free_cli_args(args);
            return NULL;
        }
        else {
            // Input file - validate before adding
            if (args->input_count >= MAX_INPUT_FILES) {
                fprintf(stderr, "Error: Too many input files (max %d)\n", MAX_INPUT_FILES);
                free_cli_args(args);
                return NULL;
            }
            
            // Validate filename length
            if (strlen(argv[i]) >= MAX_FILENAME_LENGTH) {
                fprintf(stderr, "Error: Filename too long: %s (max %d characters)\n", 
                        argv[i], MAX_FILENAME_LENGTH - 1);
                free_cli_args(args);
                return NULL;
            }
            
            // Check for path traversal attempts
            if (strstr(argv[i], "../") || strstr(argv[i], "..\\")) {
                fprintf(stderr, "Error: Path traversal not allowed in filename: %s\n", argv[i]);
                free_cli_args(args);
                return NULL;
            }
            
            args->input_files[args->input_count++] = strdup(argv[i]);
        }
    }
    
    return args;
}

/**
 * @brief Free memory allocated for CLIArgs structure
 * 
 * Properly deallocates all memory associated with a CLIArgs structure:
 * - Dynamic strings (output_file, output_dir, eval_code)
 * - Input files array and individual file strings
 * - The CLIArgs structure itself
 * 
 * @param args Pointer to CLIArgs structure to free (can be NULL)
 * 
 * @note Safe to call with NULL pointer
 * @note Handles partial initialization gracefully
 */
void free_cli_args(CLIArgs* args) {
    if (!args) return;
    
    if (args->output_file) free(args->output_file);
    if (args->output_dir) free(args->output_dir);
    if (args->eval_code) free(args->eval_code);
    
    if (args->input_files) {
        for (int i = 0; i < args->input_count; i++) {
            if (args->input_files[i]) free(args->input_files[i]);
        }
        free(args->input_files);
    }
    
    free(args);
}

/**
 * @brief Display interactive file selection menu
 * 
 * Scans current directory for .ggcode files and presents an interactive menu:
 * 1. Lists all found .ggcode files with numbered options
 * 2. Provides "Compile all files" option
 * 3. Provides "Exit" option
 * 4. Shows helpful usage instructions
 * 
 * User can select:
 * - Individual files by number (1, 2, 3, ...)
 * - All files option (highest number)
 * - Exit option (0)
 * 
 * Cross-platform implementation:
 * - Windows: Uses FindFirstFile/FindNextFile API
 * - Unix/Linux: Uses opendir/readdir POSIX functions
 * 
 * @param args Parsed CLI arguments for compilation options (must not be NULL)
 * 
 * @note Handles memory allocation failures gracefully
 * @note Validates user input and provides error messages for invalid selections
 * @note Uses external compile_file() function for actual compilation
 * @note Displays colorized output for better user experience
 */
void interactive_file_selection(const CLIArgs* args) {
    // Step 1: Find all .ggcode files in current directory
    char** ggcode_files = NULL;
    int file_count = 0;
    int capacity = 10;
    
    ggcode_files = malloc(capacity * sizeof(char*));
    if (!ggcode_files) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return;
    }
    
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile("*.ggcode", &findFileData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                if (file_count >= MAX_INPUT_FILES) {
                    fprintf(stderr, "Warning: Too many .ggcode files in directory (max %d), showing first %d\n", 
                            MAX_INPUT_FILES, MAX_INPUT_FILES);
                    break;
                }
                if (file_count >= capacity) {
                    capacity *= 2;
                    ggcode_files = realloc(ggcode_files, capacity * sizeof(char*));
                    if (!ggcode_files) {
                        fprintf(stderr, "Error: Memory reallocation failed\n");
                        return;
                    }
                }
                ggcode_files[file_count] = strdup(findFileData.cFileName);
                file_count++;
            }
        } while (FindNextFile(hFind, &findFileData) != 0);
        FindClose(hFind);
    }
#else
    DIR *dir = opendir(".");
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) {
                size_t len = strlen(entry->d_name);
                if (len > 7 && strcmp(entry->d_name + len - 7, ".ggcode") == 0) {
                    if (file_count >= MAX_INPUT_FILES) {
                        fprintf(stderr, "Warning: Too many .ggcode files in directory (max %d), showing first %d\n", 
                                MAX_INPUT_FILES, MAX_INPUT_FILES);
                        break;
                    }
                    if (file_count >= capacity) {
                        capacity *= 2;
                        ggcode_files = realloc(ggcode_files, capacity * sizeof(char*));
                        if (!ggcode_files) {
                            fprintf(stderr, "Error: Memory reallocation failed\n");
                            closedir(dir);
                            return;
                        }
                    }
                    ggcode_files[file_count] = strdup(entry->d_name);
                    file_count++;
                }
            }
        }
        closedir(dir);
    }
#endif
    
    if (file_count == 0) {
        printf("No .ggcode files found in current directory.\n");
        free(ggcode_files);
        return;
    }
    
    // Step 2: Display interactive menu with found files
    printf("┏┓┏┓┏┓┏┓┳┓┏┓  ┏┓       •┓      ┏┓┓ ┳\n");
    printf("┃┓┃┓┃ ┃┃┃┃┣   ┃ ┏┓┏┳┓┏┓┓┃┏┓┏┓  ┃ ┃ ┃\n");
    printf("┗┛┗┛┗┛┗┛┻┛┗┛  ┗┛┗┛┛┗┗┣┛┗┗┗ ┛   ┗┛┗┛┻   v1.0.9\n\n");




    printf("Found %d .ggcode file%s:\n\n", file_count, file_count == 1 ? "" : "s");
    
    for (int i = 0; i < file_count; i++) {
        printf("  \033[1;33m%d)\033[0m %s\n", i + 1, ggcode_files[i]);
    }
    printf("  \033[1;33m%d)\033[0m \033[1;32mCompile all files\033[0m\n", file_count + 1);
    printf("  \033[1;33mh)\033[0m \033[1;36mShow help\033[0m\n");
    printf("  \033[1;33m0)\033[0m \033[1;31mExit\033[0m\n\n");
    // Step 3: Get and process user selection
    printf("Select option (0-%d, h): ", file_count + 1);
    fflush(stdout);
    
    char input[10];
    if (!fgets(input, sizeof(input), stdin)) {
        printf("\nExiting...\n");
        goto cleanup;
    }
    
    // Remove newline from input and validate
    input[strcspn(input, "\n")] = 0;
    
    // Basic input sanitization - only allow alphanumeric and basic chars
    for (int i = 0; input[i] != '\0'; i++) {
        if (!((input[i] >= '0' && input[i] <= '9') || 
              (input[i] >= 'a' && input[i] <= 'z') || 
              (input[i] >= 'A' && input[i] <= 'Z'))) {
            fprintf(stderr, "Invalid input character. Use numbers 0-%d or 'h' for help.\n", file_count + 1);
            goto cleanup;
        }
    }
    
    // Check for help option first
    if (strcmp(input, "h") == 0 || strcmp(input, "H") == 0) {
        printf("\n");
        print_help();
        goto cleanup;
    }
    
    int choice = atoi(input);
    
    if (choice == 0) {
        printf("Exiting...\n");
        goto cleanup;
    } else if (choice == file_count + 1) {
        // Compile all files
        printf("\n\033[1;32mCompiling all files...\033[0m\n");
        for (int i = 0; i < file_count; i++) {
            char* output_path = get_smart_output_path(ggcode_files[i], args);
            if (!args->quiet) {
                printf("\033[38;5;208mGGCODE Compiling\033[0m \033[1m%s\033[0m → \033[1;32m%s\033[0m\n", 
                       ggcode_files[i], output_path);
            }
            
            // Forward declare compile_file function
            extern void compile_file(const char* input_path, const char* output_path, bool quiet);
            compile_file(ggcode_files[i], output_path, args->quiet);
            free(output_path);
        }
        if (!args->quiet) {
            printf("\nCompiled %d file%s successfully!\n", file_count, file_count == 1 ? "" : "s");
        }
    } else if (choice >= 1 && choice <= file_count) {
        // Compile selected file
        char* selected_file = ggcode_files[choice - 1];
        char* output_path = get_smart_output_path(selected_file, args);
        
        if (!args->quiet) {
            printf("\n\033[38;5;208mGGCODE Compiling\033[0m \033[1m%s\033[0m → \033[1;32m%s\033[0m\n", 
                   selected_file, output_path);
        }
        
        // Forward declare compile_file function
        extern void compile_file(const char* input_path, const char* output_path, bool quiet);
        compile_file(selected_file, output_path, args->quiet);
        free(output_path);
        
        if (!args->quiet) {
            printf("Compilation completed!\n");
        }
    } else {
        printf("Invalid selection. Exiting...\n");
    }
    
cleanup:
    // Step 4: Clean up allocated memory
    for (int i = 0; i < file_count; i++) {
        free(ggcode_files[i]);
    }
    free(ggcode_files);
}