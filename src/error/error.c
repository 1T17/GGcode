// error.c
#include "error.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "../lexer/token_types.h"  // Needed for Token_Type enum
#include "../parser/ast_nodes.h"   // Needed for ASTNodeType enum
#include "../utils/output_buffer.h"

#include "config/config.h"

#define MAX_ERRORS 100
static char error_messages[MAX_ERRORS][256];
static int error_count = 0;



jmp_buf fatal_error_jump_buffer;
int fatal_error_triggered = 0;



const char *get_ast_type_name(int type) {
    switch (type) {
        case AST_NOP: return "AST_NOP";
        case AST_LET: return "AST_LET";
        case AST_ASSIGN: return "AST_ASSIGN";
        case AST_VAR: return "AST_VAR";
        case AST_ASSIGN_INDEX: return "AST_ASSIGN_INDEX";
        case AST_ARRAY_LITERAL: return "AST_ARRAY_LITERAL";
        case AST_INDEX: return "AST_INDEX";
        case AST_FUNCTION: return "AST_FUNCTION";
        case AST_CALL: return "AST_CALL";
        case AST_RETURN: return "AST_RETURN";
        case AST_NUMBER: return "AST_NUMBER";
        case AST_BINARY: return "AST_BINARY";
        case AST_EXPR_STMT: return  "AST_EXPR_STMT";
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



#include "../generator/emitter.h"   // for free_output_buffer()
#include "../runtime/evaluator.h"   // for reset_runtime_state()


// These should be declared as extern in error.h or at the top of error.c if not already
extern ASTNode *global_root_ast;
extern char *global_source_buffer;


void fatal_error(const char *source, int line, int column, const char *format, ...) {
    // Show header with location
    fprintf(stderr, "\n🛑 [Fatal Error] ");



    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    // Show file:line:column info (if source provided)
    if (source && line > 0 && column > 0) {
        fprintf(stderr, "  (at %s:%d:%d)\n", RUNTIME_FILENAME[0] ? RUNTIME_FILENAME : "<input>", line, column);

        // Extract and print the relevant source line
        const char *line_start = source;
        const char *p = source;
        int current_line = 1;

        // Go to the line
        while (*p && current_line < line) {
            if (*p == '\n') current_line++;
            if (current_line < line) line_start = ++p;
            else break;
        }

        const char *line_end = line_start;
        while (*line_end && *line_end != '\n') line_end++;

        int line_len = (int)(line_end - line_start);
        fprintf(stderr, "  %.*s\n  ", line_len, line_start);

        for (int i = 1; i < column && i <= line_len; i++) {
            fprintf(stderr, (line_start[i - 1] == '\t') ? "\t" : " ");
        }
        fprintf(stderr, "^\n");
    } else {
        fprintf(stderr, "\n");
    }

    // Cleanup
    if (global_root_ast) {
        free_ast(global_root_ast);
        global_root_ast = NULL;
    }

    if (global_source_buffer) {
        free(global_source_buffer);
        global_source_buffer = NULL;
    }

    free_output_buffer();
    clear_errors();


    // Set fatal error flag and jump to error handler
fatal_error_triggered = 1;
longjmp(fatal_error_jump_buffer, 1);

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