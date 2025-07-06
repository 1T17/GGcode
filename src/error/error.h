// error.h
#ifndef ERROR_H
#define ERROR_H

void report_error(const char *format, ...);
void clear_errors();
int has_errors();
void print_errors();
const char *get_ast_type_name(int type);

#endif
