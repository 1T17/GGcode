// error.c
#include "error.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "../lexer/token_types.h"  // Needed for Token_Type enum
#include "../parser/ast_nodes.h"   // Needed for ASTNodeType enum



#define MAX_ERRORS 100
static char error_messages[MAX_ERRORS][256];
static int error_count = 0;





const char *get_ast_type_name(int type) {
    switch (type) {
        case AST_NOP: return "AST_NOP";
        case AST_LET: return "AST_LET";
        case AST_ASSIGN: return "AST_ASSIGN";
        case AST_VAR: return "AST_VAR";
        case AST_ARRAY_LITERAL: return "AST_ARRAY_LITERAL";
        case AST_INDEX: return "AST_INDEX";
        case AST_FUNCTION: return "AST_FUNCTION";
        case AST_CALL: return "AST_CALL";
        case AST_RETURN: return "AST_RETURN";
        case AST_NUMBER: return "AST_NUMBER";
        case AST_BINARY: return "AST_BINARY";
        case AST_GCODE: return "AST_GCODE";
        case AST_WHILE: return "AST_WHILE";
        case AST_FOR: return "AST_FOR";
        case AST_BLOCK: return "AST_BLOCK";
        case AST_NOTE: return "AST_NOTE";
        case AST_IF: return "AST_IF";
        case AST_UNARY: return "AST_UNARY";
        case AST_EMPTY: return "AST_EMPTY";
        default: return "UNKNOWN_AST_TYPE";
    }
}







void report_error(const char *format, ...) {
    if (error_count >= MAX_ERRORS) return;
    va_list args;
    va_start(args, format);
    vsnprintf(error_messages[error_count], 256, format, args);
    va_end(args);
    error_count++;
}

void clear_errors() {
    error_count = 0;
}

int has_errors() {
    return error_count > 0;
}


void print_errors() {

    for (int i = 0; i < error_count; ++i)
        fprintf(stderr, "\n💥 [Error] %s\n", error_messages[i]);
    clear_errors();
}