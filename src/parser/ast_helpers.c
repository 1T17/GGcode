#include "ast_nodes.h"
#include <stdlib.h>
#include <string.h>
#include "../error/error.h"

// Helper functions for common AST node creation patterns

ASTNode* create_number_node(double value) {
    ASTNode *ast_node = malloc(sizeof(ASTNode));
    if (!ast_node) {
        report_error("[AST] malloc failed for ASTNode");
        return NULL;
    }
    ast_node->type = AST_NUMBER;
    ast_node->parent = NULL;
    ast_node->number.value = value;
    return ast_node;
}

ASTNode* create_var_node(const char* name) {
    ASTNode *ast_node = malloc(sizeof(ASTNode));
    if (!ast_node) {
        report_error("[AST] malloc failed for ASTNode");
        return NULL;
    }
    ast_node->type = AST_VAR;
    ast_node->parent = NULL;
    ast_node->var.name = strdup(name);
    if (!ast_node->var.name) {
        report_error("[AST] strdup failed for variable name");
        free(ast_node);
        return NULL;
    }
    return ast_node;
}

ASTNode* create_binary_node(Token_Type op, ASTNode* left, ASTNode* right) {
    ASTNode *ast_node = malloc(sizeof(ASTNode));
    if (!ast_node) {
        report_error("[AST] malloc failed for ASTNode");
        return NULL;
    }
    ast_node->type = AST_BINARY;
    ast_node->parent = NULL;
    ast_node->binary_expr.op = op;
    ast_node->binary_expr.left = left;
    ast_node->binary_expr.right = right;
    return ast_node;
}

ASTNode* create_unary_node(Token_Type op, ASTNode* operand) {
    ASTNode *ast_node = malloc(sizeof(ASTNode));
    if (!ast_node) {
        report_error("[AST] malloc failed for ASTNode");
        return NULL;
    }
    ast_node->type = AST_UNARY;
    ast_node->parent = NULL;
    ast_node->unary_expr.op = op;
    ast_node->unary_expr.operand = operand;
    return ast_node;
}

ASTNode* create_call_node(const char* name, ASTNode** args, int arg_count) {
    ASTNode *ast_node = malloc(sizeof(ASTNode));
    if (!ast_node) {
        report_error("[AST] malloc failed for ASTNode");
        return NULL;
    }
    ast_node->type = AST_CALL;
    ast_node->parent = NULL;
    ast_node->call_expr.name = strdup(name);
    if (!ast_node->call_expr.name) {
        report_error("[AST] strdup failed for function name");
        free(ast_node);
        return NULL;
    }
    ast_node->call_expr.args = args;
    ast_node->call_expr.arg_count = arg_count;
    return ast_node;
}

ASTNode* create_array_literal_node(ASTNode** elements, int count) {
    ASTNode *ast_node = malloc(sizeof(ASTNode));
    if (!ast_node) {
        report_error("[AST] malloc failed for ASTNode");
        return NULL;
    }
    ast_node->type = AST_ARRAY_LITERAL;
    ast_node->parent = NULL;
    ast_node->array_literal.elements = elements;
    ast_node->array_literal.count = count;
    return ast_node;
}

ASTNode* create_block_node(ASTNode** statements, int count) {
    ASTNode *ast_node = malloc(sizeof(ASTNode));
    if (!ast_node) {
        report_error("[AST] malloc failed for ASTNode");
        return NULL;
    }
    ast_node->type = AST_BLOCK;
    ast_node->parent = NULL;
    ast_node->block.statements = statements;
    ast_node->block.count = count;
    return ast_node;
}