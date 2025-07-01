#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../parser/ast_nodes.h"

#define MAX_VARIABLES 100

typedef struct
{
    char *name;
    double value;
} Variable;

static Variable variables[MAX_VARIABLES];
static int var_count = 0;

// Set or update variable
void set_var(const char *name, double value)
{
    for (int i = 0; i < var_count; i++)
    {
        if (variables[i].name && strcmp(variables[i].name, name) == 0)
        {
            variables[i].value = value;
            return;
        }
    }

    if (var_count >= MAX_VARIABLES)
    {
        fprintf(stderr, "[set_var] ERROR: variable limit reached.\n");
        exit(1);
    }

    variables[var_count].name = strdup(name);
    variables[var_count].value = value;

    // fprintf(stderr, "[set_var] New variable: %s = %.5f (slot %d)\n", name, value, var_count);
    var_count++;
}

// Check if variable exists
int var_exists(const char *name)
{
    for (int i = 0; i < var_count; i++)
    {
        if (variables[i].name && strcmp(variables[i].name, name) == 0)
        {
            return 1;
        }
    }
    return 0;
}

// Retrieve variable value
double get_var(const char *name)
{

    for (int i = 0; i < var_count; i++)
    {
        if (strcmp(variables[i].name, name) == 0)
        {
            return variables[i].value;
        }
    }
    fprintf(stderr, "[Runtime] Undefined variable: %s\n", name);
    exit(1);
}

// Evaluate expressions recursively
double eval_expr(ASTNode *node)
{
    if (!node)
    {
        fprintf(stderr, "[Runtime] NULL node passed to eval_expr\n");
        exit(1);
    }

    switch (node->type)
    {

    case AST_NUMBER:
        return node->number.value;

    case AST_VAR:
        return get_var(node->var.name);

    case AST_LET:
        fprintf(stderr, "[Runtime] ERROR: 'let' is a statement, not an expression.\n");
        exit(1);

    case AST_UNARY:
    {
        double operand = eval_expr(node->unary_expr.operand);

        switch (node->unary_expr.op)
        {
        case TOKEN_BANG:
            return !operand;
        default:
            fprintf(stderr, "[Runtime] Unknown unary operator: %d\n", node->unary_expr.op);
            exit(1);
        }
    }

    case AST_BINARY:
    {
        double left = eval_expr(node->binary_expr.left);
        double right = eval_expr(node->binary_expr.right);
        TokenType op = node->binary_expr.op;

        switch (op)
        {
        case TOKEN_PLUS:
            return left + right;
        case TOKEN_MINUS:
            return left - right;
        case TOKEN_STAR:
            return left * right;
        case TOKEN_SLASH:
            if (right == 0)
            {
                fprintf(stderr, "[Runtime] Division by zero (%.5f / %.5f)\n", left, right);
                exit(1);
            }
            return left / right;
        case TOKEN_LESS:
            return left < right;
        case TOKEN_LESS_EQUAL:
            return left <= right;
        case TOKEN_GREATER:
            return left > right;
        case TOKEN_GREATER_EQUAL:
            return left >= right;
        case TOKEN_EQUAL_EQUAL:
            return left == right;
        case TOKEN_BANG_EQUAL:
           // fprintf(stderr, "[DEBUG] Evaluating: %.2f != %.2f → %s\n", left, right, (left != right) ? "true" : "false");
            return left != right;

        // ✅ Add these new lines:
        case TOKEN_AND:
            return (left != 0.0 && right != 0.0) ? 1.0 : 0.0;
        case TOKEN_OR:
            return (left != 0.0 || right != 0.0) ? 1.0 : 0.0;

        default:
            fprintf(stderr, "[Runtime] Unknown binary operator: %d\n", op);
            exit(1);
        }
    }

    case AST_INDEX:
    {
        double array = eval_expr(node->index_expr.array);
        double index = eval_expr(node->index_expr.index);
        fprintf(stderr, "[Runtime] Warning: Using 'index' operator [%.2f][%.2f] — not fully implemented\n", array, index);
        return index;
    }

    default:
        fprintf(stderr, "[Runtime] Unsupported expression type: %d\n", node->type);
        exit(1);
    }
}
