// error.h
#ifndef ERROR_H
#define ERROR_H

extern struct ASTNode *global_root_ast;
extern char *global_source_buffer;
#include "../parser/ast_nodes.h"  // For ASTNode*







#include <setjmp.h>

extern jmp_buf fatal_error_jump_buffer;
extern int fatal_error_triggered;


void clear_fatal_state(void);







void report_error(const char *format, ...);

void fatal_error(const char *source, int line, int column, const char *format, ...);


void clear_errors();
int has_errors();
void print_errors();
const char* get_error_messages(); // New function to get error messages as string
const char *get_ast_type_name(int type);

#endif
