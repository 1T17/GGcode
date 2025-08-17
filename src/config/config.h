#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>
#include "../parser/ast_nodes.h"
#include "../runtime/runtime_state.h"

// Global variables
extern ASTNode *global_root_ast;
extern char *global_source_buffer;

// Runtime management
extern Runtime g_runtime;
void init_runtime(void);
Runtime* get_runtime(void);

// Legacy global variables (for backward compatibility)
extern char RUNTIME_TIME[64];
extern char RUNTIME_FILENAME[256];

// Default values

#define DEFAULT_OUTPUT_TO_FILE 1
#define DEFAULT_LINE_NUMBER 10
#define DEFAULT_ENABLE_N_LINES 1

// Function declarations
void set_input_file(const char* filename);
const char* get_input_file(void);

int get_output_to_file(void);
int get_line_number(void);
void increment_line_number(void);
void reset_line_number(void);
int get_enable_n_lines(void);

// Configuration variable support
void set_decimal_places(int places);
int get_decimal_places(void);
void set_enable_n_lines_from_var(int enable);
const char* get_decimal_format(void);
void reset_config_state(void);

#endif // CONFIG_H
