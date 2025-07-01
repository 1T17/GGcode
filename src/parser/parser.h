#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"
#include "ast_nodes.h"

typedef struct {
    Lexer* lexer;
    Token current;
    Token previous;
} Parser;


ASTNode* parse_inline_expression(const char* expr);

ASTNode* parse_script(const char* source);
void free_ast(ASTNode* node);

#endif
