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
static char error_messages[MAX_ERRORS][512];  // Increased buffer size for longer lines
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
    // Create detailed error message with line info
    if (error_count < MAX_ERRORS) {
        char base_msg[200];  // Reduced size to ensure we have room
        
        // First format the base message
        va_list args;
        va_start(args, format);
        vsnprintf(base_msg, sizeof(base_msg), format, args);
        va_end(args);
        
        // Add location info
        if (source && line > 0 && column > 0) {
            // Extract the source line
            const char *line_start = source;
            const char *p = source;
            int current_line = 1;

            // Find the start of the target line
            while (*p && current_line < line) {
                if (*p == '\n') {
                    current_line++;
                    if (current_line <= line) {
                        line_start = p + 1;  // Start after the newline
                    }
                }
                p++;
            }

            // Find the end of the line
            const char *line_end = line_start;
            while (*line_end && *line_end != '\n' && *line_end != '\r') {
                line_end++;
            }

            int line_len = (int)(line_end - line_start);
            
            // Create detailed message with source line and better formatting
            if (line_len > 0) {
                // Show full line for professional output - limit to 120 chars to leave room for caret
                int max_line_len = line_len > 120 ? 120 : line_len;
                
                // Create the caret indicator line
                char caret_line[150] = {0};
                int caret_pos = column - 1; // Convert to 0-based
                if (caret_pos < 0) caret_pos = 0;
                if (caret_pos > max_line_len) caret_pos = max_line_len;
                
                // Add spaces to align with "  → Source: " (12 characters for exact alignment)
                strcpy(caret_line, "            "); // 12 spaces to match "  → Source: "
                
                // Fill with spaces up to the error position, then add caret
                for (int i = 0; i < caret_pos && i < 120; i++) {
                    // Handle tabs properly by using tab character
                    if (i < line_len && line_start[i] == '\t') {
                        caret_line[12 + i] = '\t';
                    } else {
                        caret_line[12 + i] = ' ';
                    }
                }
                caret_line[12 + caret_pos] = '^';
                caret_line[12 + caret_pos + 1] = '\0';
                
                snprintf(error_messages[error_count], 511, 
                    "\n%.180s\n  → Location: %d:%d\n  → Source: %.*s%s\n%s", 
                    base_msg, 
                    line, column,
                    max_line_len, line_start,
                    line_len > 120 ? "..." : "",  // Add ... if truncated
                    caret_line
                );
            } else {
                // No line content found, just show location
                snprintf(error_messages[error_count], 511, 
                    "\n%.200s\n  → Location: %d:%d", 
                    base_msg, 
                    line, column
                );
            }
        } else {
            snprintf(error_messages[error_count], 511, "\n%.200s", base_msg);
        }
        
        error_messages[error_count][511] = '\0';  // Ensure null termination
        error_count++;
    }
    
    // Show header with location
    fprintf(stderr, "\n🛑 [Fatal Error] ");

    va_list args2;
    va_start(args2, format);
    vfprintf(stderr, format, args2);
    va_end(args2);

    // Show file:line:column info (if source provided)
    if (source && line > 0 && column > 0) {
        fprintf(stderr, "  (at %d:%d)\n", line, column);

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
    // Don't clear errors here - let the caller handle it
    // clear_errors();


    // Set fatal error flag and jump to error handler
fatal_error_triggered = 1;
longjmp(fatal_error_jump_buffer, 1);

}



































void report_error(const char *format, ...) {
    if (error_count >= MAX_ERRORS) return;
    va_list args;
    va_start(args, format);
    vsnprintf(error_messages[error_count], 512, format, args);
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

const char* get_error_messages() {
    if (error_count == 0) {
        return strdup("No errors");
    }
    
    // Calculate total length needed
    size_t total_len = 0;
    for (int i = 0; i < error_count; i++) {
        total_len += strlen(error_messages[i]) + 15; // +15 for "; ERROR:" and formatting
    }
    total_len += 50; // Extra buffer for formatting
    
    char* result = malloc(total_len);
    if (!result) {
        return strdup("; ERROR: Memory allocation failed");
    }
    
    strcpy(result, "GGcode Compiler Error:");
    for (int i = 0; i < error_count; i++) {
        strcat(result, error_messages[i]);
        if (i < error_count - 1) {
            strcat(result, "\nGGcode Compiler Error:");
        }
    }
    strcat(result, "\n");
    
    return result;
}