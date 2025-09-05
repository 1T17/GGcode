#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"
#include "ast_nodes.h"

typedef struct {
    Lexer* lexer;
    Token current;
    Token previous;
    int function_definition_depth;  // Track nested function definitions during parsing
} Parser;

void parser_advance(void); 
ASTNode* parse_inline_expression(const char* expr);
ASTNode* parse_script(void);
void free_ast(ASTNode* node);
void reset_parser_static_vars(void);

// Function definition context tracking for parser
void parser_enter_function_definition(void);
void parser_exit_function_definition(void);
int parser_is_inside_function_definition(void);


#endif
