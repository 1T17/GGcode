/**
 * @file cli.h
 * @brief Command Line Interface for GGcode Compiler
 * 
 * This module provides comprehensive CLI functionality including argument parsing,
 * interactive file selection, help system, and output path management.
 */

#ifndef CLI_H
#define CLI_H

#include <stdbool.h>

/** @brief Current version of the GGcode compiler */
#define GGCODE_VERSION "1.0.0"

/** @brief Maximum length for evaluation code strings */
#define MAX_EVAL_CODE 4096

/** @brief Maximum length for output file paths */
#define MAX_OUTPUT_PATH 512

/** @brief Maximum length for input filenames */
#define MAX_FILENAME_LENGTH 256

/** @brief Maximum number of input files */
#define MAX_INPUT_FILES 1000

/**
 * @brief Structure containing all parsed command line arguments and options
 * 
 * This structure holds all the configuration options parsed from command line
 * arguments, including flags, file paths, and input files.
 */
typedef struct {
    // Options
    bool show_help;         /**< Show help message and exit */
    bool show_version;      /**< Show version information and exit */
    bool compile_all;       /**< Compile all .ggcode files in directory */
    bool quiet;             /**< Suppress compilation reports */
    bool verbose;           /**< Show detailed compilation information */
    bool eval_mode;         /**< Direct evaluation mode */
    
    // Paths
    char* output_file;      /**< Specific output file path (single file mode) */
    char* output_dir;       /**< Output directory path */
    char* eval_code;        /**< Code string for direct evaluation */
    
    // Input files
    char** input_files;     /**< Array of input file paths */
    int input_count;        /**< Number of input files */
    
    // Internal
    bool has_custom_output; /**< Flag indicating custom output file specified */
} CLIArgs;

/**
 * @brief Parse command line arguments into CLIArgs structure
 * 
 * Parses argc/argv into a structured format, handling all supported options
 * and flags. Validates arguments and returns NULL on error.
 * 
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return Pointer to allocated CLIArgs structure, or NULL on error
 */
CLIArgs* parse_arguments(int argc, char* argv[]);

/**
 * @brief Free memory allocated for CLIArgs structure
 * 
 * Properly deallocates all memory associated with a CLIArgs structure,
 * including dynamically allocated strings and arrays.
 * 
 * @param args Pointer to CLIArgs structure to free (can be NULL)
 */
void free_cli_args(CLIArgs* args);

/**
 * @brief Print comprehensive help message to stdout
 * 
 * Displays detailed usage information including all options, examples,
 * and common workflows for the GGcode compiler.
 */
void print_help(void);

/**
 * @brief Print version information to stdout
 * 
 * Displays version number, build date, and basic program information.
 */
void print_version(void);

/**
 * @brief Generate smart output path for input file
 * 
 * Creates an appropriate output path based on input file and CLI arguments.
 * Handles custom output files, output directories, and fallback logic.
 * 
 * @param input_file Path to input .ggcode file
 * @param args Parsed CLI arguments containing output preferences
 * @return Allocated string containing output path, or NULL on error
 */
char* get_smart_output_path(const char* input_file, const CLIArgs* args);

/**
 * @brief Create directory if it doesn't exist
 * 
 * Attempts to create the specified directory with appropriate permissions.
 * Does nothing if directory already exists.
 * 
 * @param path Directory path to create
 * @return 0 on success or if directory exists, -1 on failure
 */
int create_directory_if_needed(const char* path);

/**
 * @brief Display interactive file selection menu
 * 
 * Scans current directory for .ggcode files and presents an interactive
 * menu allowing user to select individual files or compile all files.
 * Handles user input and initiates compilation based on selection.
 * 
 * @param args Parsed CLI arguments for compilation options
 */
void interactive_file_selection(const CLIArgs* args);

#endif // CLI_H