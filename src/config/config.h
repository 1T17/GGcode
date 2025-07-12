#ifndef CONFIG_H
#define CONFIG_H

#include "../parser/ast_nodes.h"  // ⬅️ Add this line for ASTNode




// Default configuration
#define DEFAULT_DEBUG           0   // 1 = enable debug logging
#define DEFAULT_OUTPUT_TO_FILE  1   // 1 = output to file, 0 = print to stdout
#define DEFAULT_LINE_NUMBER     10
#define DEFAULT_ENABLE_N_LINES  1   // 1 = enabled, 0 = disabled

void set_input_file(const char* filename);


// ✅ Declarations only — use 'extern'
extern char RUNTIME_TIME[64];
extern char RUNTIME_FILENAME[256];



extern ASTNode *global_root_ast;
extern char *global_source_buffer;



// Function declarations
const char* get_input_file();
const char* get_output_file();
int get_debug();
int get_output_to_file();
int get_line_number();
int get_enable_n_lines();

void reset_line_number();
void increment_line_number();


#endif // CONFIG_H
