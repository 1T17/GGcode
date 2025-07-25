#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"
#include "ast_nodes.h"

typedef struct {
    Lexer* lexer;
    Token current;
    Token previous;
} Parser;

void parser_advance(void); 
ASTNode* parse_inline_expression(const char* expr);
ASTNode* parse_script(void);
void free_ast(ASTNode* node);
void reset_parser_static_vars(void);


#endif
