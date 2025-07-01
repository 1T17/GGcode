#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../parser/ast_nodes.h"

#define MAX_VARIABLES 100
#define MAX_FUNCTIONS 64

void register_function(ASTNode *node);
void declare_var(const char *name, double value);

static ASTNode* find_function(const char *name);
static int runtime_has_returned = 0;
static double runtime_return_value = 0.0;


typedef struct
{
    char *name;
    double value;
} Variable;

typedef struct {
    char *name;
    ASTNode *node; // AST_FUNCTION node
} FunctionEntry;

static Variable variables[MAX_VARIABLES];
static int var_count = 0;

static FunctionEntry function_table[MAX_FUNCTIONS];
static int function_count = 0;

// Set or update variable
void set_var(const char *name, double value)
{
    if (var_count >= MAX_VARIABLES)
    {
        fprintf(stderr, "[set_var] ERROR: variable limit reached.\n");
        exit(1);
    }

    variables[var_count].name = strdup(name);
    variables[var_count].value = value;
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

    for (int i = var_count - 1; i >= 0; i--)
    {
        if (variables[i].name && strcmp(variables[i].name, name) == 0)
        {
            return variables[i].value;
        }
    }
    fprintf(stderr, "[get_var] ERROR: variable '%s' not found.\n", name);
    exit(1);
}

// Forward declaration
double eval_function_call(ASTNode *node);

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
        declare_var(node->let_stmt.name, eval_expr(node->let_stmt.expr));
        return 0.0;

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
            return left != right;
        case TOKEN_AND:
            return (left != 0.0 && right != 0.0) ? 1.0 : 0.0;
        case TOKEN_OR:
            return (left != 0.0 || right != 0.0) ? 1.0 : 0.0;
        case TOKEN_AMPERSAND:
            return (double)((int)left & (int)right);
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

    case AST_CALL:
        // Evaluate the function call and return its value
        // If you have a function call evaluator, call it here.
        return eval_function_call(node);

    case AST_IF: {
        double cond = eval_expr(node->if_stmt.condition);
        if (cond) {
            return eval_expr(node->if_stmt.then_branch);
        } else if (node->if_stmt.else_branch) {
            return eval_expr(node->if_stmt.else_branch);
        }
        return 0.0;
    }

    case AST_BLOCK: {
        double result = 0.0;
        for (int i = 0; i < node->block.count; i++) {
            result = eval_expr(node->block.statements[i]);
        }
        return result;
    }

    case AST_RETURN:
        runtime_has_returned = 1;
        runtime_return_value = eval_expr(node->return_stmt.expr);
        return runtime_return_value;


    default:
        fprintf(stderr, "[Runtime] Unsupported expression type: %d\n", node->type);
        exit(1);
    }
}

double eval_function_call(ASTNode *node) {
    ASTNode *func = find_function(node->call_expr.name);
    if (!func) {
        fprintf(stderr, "[Runtime] Function not found: %s\n", node->call_expr.name);
        exit(1);
    }

    // Save old variable values for parameters
    double old_vars[32];
    char *param_names[32];
    int param_count = func->function_stmt.param_count;
    for (int i = 0; i < param_count; i++) {
        param_names[i] = func->function_stmt.params[i];
        if (var_exists(param_names[i]))
            old_vars[i] = get_var(param_names[i]);
        else
            old_vars[i] = 0.0;
        double arg_val = 0.0;
        if (i < node->call_expr.arg_count)
            arg_val = eval_expr(node->call_expr.args[i]);
        declare_var(param_names[i], arg_val); // <-- use declare_var here
    }

    int old_var_count = var_count; // Save before declaring params

    // Evaluate function body and capture return value
    runtime_has_returned = 0;
    runtime_return_value = 0.0;
    ASTNode *body = func->function_stmt.body;
    for (int i = 0; i < body->block.count; i++) {
        eval_expr(body->block.statements[i]);
        if (runtime_has_returned) break;
    }

    var_count = old_var_count; // Remove locals after function call

    // Restore old variable values
    for (int i = 0; i < param_count; i++) {
        set_var(param_names[i], old_vars[i]);
    }

    return runtime_return_value;
}

void register_function(ASTNode *node) {
    if (function_count >= MAX_FUNCTIONS) {
        fprintf(stderr, "[Runtime] Too many functions\n");
        exit(1);
    }
    function_table[function_count].name = node->function_stmt.name;
    function_table[function_count].node = node;
    function_count++;
}

static ASTNode* find_function(const char *name) {
    for (int i = 0; i < function_count; i++) {
        if (strcmp(function_table[i].name, name) == 0)
            return function_table[i].node;
    }
    return NULL;
}

void reset_runtime_state(void) {
    for (int i = 0; i < var_count; i++) {
        free(variables[i].name);
    }
    var_count = 0;

    // If you dynamically allocate function names, free them here too
    function_count = 0;
}

void declare_var(const char *name, double value)
{
    if (var_count >= MAX_VARIABLES) {
        fprintf(stderr, "[declare_var] ERROR: variable limit reached.\n");
        exit(1);
    }
    variables[var_count].name = strdup(name);
    variables[var_count].value = value;
    var_count++;
}

