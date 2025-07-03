#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../parser/ast_nodes.h"
#include <math.h>


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

#define MAX_VARIABLES 1024
#define MAX_FUNCTIONS 64

void enter_scope(void);
void exit_scope(void);
void register_function(ASTNode *node);
void declare_var(const char *name, double value);

static ASTNode* find_function(const char *name);
static int runtime_has_returned = 0;
static double runtime_return_value = 0.0;
static int current_scope_level = 0;

typedef struct
{
    char *name;
    double value;
        int scope_level; // <-- add this
} Variable;

typedef struct {
    char *name;
    ASTNode *node; // AST_FUNCTION node
} FunctionEntry;

static Variable variables[MAX_VARIABLES];
static int var_count = 0;

static FunctionEntry function_table[MAX_FUNCTIONS];
static int function_count = 0;








#include "evaluator.h"
#include <stdio.h>

void eval_block(ASTNode *block) {
    if (!block || block->type != AST_BLOCK) {
        printf("[Runtime] Invalid block node\n");
        return;
    }
    enter_scope();
    for (int i = 0; i < block->block.count; ++i) {
        ASTNode *stmt = block->block.statements[i];
        switch (stmt->type) {
            case AST_LET:
                set_var(stmt->let_stmt.name, eval_expr(stmt->let_stmt.expr));
                break;
            case AST_NOTE:
                // Optional: support runtime notes/logs if needed
                break;
            default:
                printf("[Runtime] Unsupported statement in block at index %d\n", i);
                break;
        }
    }
    exit_scope();
}




// Set or update variable
void set_var(const char *name, double value)
{
    // Search from the end for an existing variable with this name
    for (int i = var_count - 1; i >= 0; i--) {
        if (variables[i].name && strcmp(variables[i].name, name) == 0) {
            variables[i].value = value;
            return;
        }
    }
    // Not found, create new
    if (var_count >= MAX_VARIABLES) {
        fprintf(stderr, "[set_var] ERROR: variable limit reached.aaaaaaaaaaaaaaaaaaaaaa\n");
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
        Token_Type op = node->binary_expr.op;

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

        case AST_FUNCTION:
    register_function(node);
    return 0.0;

    case AST_ASSIGN:
        set_var(node->assign_stmt.name, eval_expr(node->assign_stmt.expr));
        return 0.0;


    default:
        fprintf(stderr, "[Runtime] Unsupported expression type: %d\n", node->type);
        exit(1);
    }
}



#include <math.h>
#include <string.h>

double eval_function_call(ASTNode *node) {
    const char *name = node->call_expr.name;
    int argc = node->call_expr.arg_count;
    ASTNode **args = node->call_expr.args;

    // --- Constants ---
    if (strcmp(name, "PI") == 0) return M_PI;
    if (strcmp(name, "TAU") == 0) return 2.0 * M_PI;
    if (strcmp(name, "EU") == 0) return M_E;
    if (strcmp(name, "DEG_TO_RAD") == 0) return M_PI / 180.0;
    if (strcmp(name, "RAD_TO_DEG") == 0) return 180.0 / M_PI;

    // --- Basic Math ---
    if (strcmp(name, "abs") == 0 && argc == 1) return fabs(eval_expr(args[0]));
    if (strcmp(name, "mod") == 0 && argc == 2) return fmod(eval_expr(args[0]), eval_expr(args[1]));
    if (strcmp(name, "floor") == 0 && argc == 1) return floor(eval_expr(args[0]));
    if (strcmp(name, "ceil") == 0 && argc == 1) return ceil(eval_expr(args[0]));
    if (strcmp(name, "round") == 0 && argc == 1) return round(eval_expr(args[0]));
    if (strcmp(name, "min") == 0 && argc == 2) return fmin(eval_expr(args[0]), eval_expr(args[1]));
    if (strcmp(name, "max") == 0 && argc == 2) return fmax(eval_expr(args[0]), eval_expr(args[1]));
    if (strcmp(name, "clamp") == 0 && argc == 3) {
        double v = eval_expr(args[0]);
        double lo = eval_expr(args[1]);
        double hi = eval_expr(args[2]);
        return fmin(fmax(v, lo), hi);
    }

    // --- Trigonometry ---
    if (strcmp(name, "sin") == 0 && argc == 1) return sin(eval_expr(args[0]));
    if (strcmp(name, "cos") == 0 && argc == 1) return cos(eval_expr(args[0]));
    if (strcmp(name, "tan") == 0 && argc == 1) return tan(eval_expr(args[0]));
    if (strcmp(name, "asin") == 0 && argc == 1) return asin(eval_expr(args[0]));
    if (strcmp(name, "acos") == 0 && argc == 1) return acos(eval_expr(args[0]));
    if (strcmp(name, "atan") == 0 && argc == 1) return atan(eval_expr(args[0]));
    if (strcmp(name, "atan2") == 0 && argc == 2) return atan2(eval_expr(args[0]), eval_expr(args[1]));
    if (strcmp(name, "deg") == 0 && argc == 1) return eval_expr(args[0]) * (180.0 / M_PI);
    if (strcmp(name, "rad") == 0 && argc == 1) return eval_expr(args[0]) * (M_PI / 180.0);

    // --- Geometry / Vector ---
    if (strcmp(name, "sqrt") == 0 && argc == 1) return sqrt(eval_expr(args[0]));
    if (strcmp(name, "pow") == 0 && argc == 2) return pow(eval_expr(args[0]), eval_expr(args[1]));
    if (strcmp(name, "hypot") == 0 && argc == 2) return hypot(eval_expr(args[0]), eval_expr(args[1]));
    if (strcmp(name, "lerp") == 0 && argc == 3) {
        double a = eval_expr(args[0]);
        double b = eval_expr(args[1]);
        double t = eval_expr(args[2]);
        return a + t * (b - a);
    }
    if (strcmp(name, "map") == 0 && argc == 5) {
        double v = eval_expr(args[0]);
        double in_min = eval_expr(args[1]);
        double in_max = eval_expr(args[2]);
        double out_min = eval_expr(args[3]);
        double out_max = eval_expr(args[4]);
        return out_min + ((v - in_min) * (out_max - out_min)) / (in_max - in_min);
    }
    if (strcmp(name, "distance") == 0 && argc == 4) {
        double x1 = eval_expr(args[0]);
        double y1 = eval_expr(args[1]);
        double x2 = eval_expr(args[2]);
        double y2 = eval_expr(args[3]);
        return hypot(x2 - x1, y2 - y1);
    }

    // --- Optional / Advanced ---
    if (strcmp(name, "sign") == 0 && argc == 1) {
        double x = eval_expr(args[0]);
        return (x > 0) - (x < 0);
    }
    if (strcmp(name, "log") == 0 && argc == 1) return log(eval_expr(args[0]));
    if (strcmp(name, "exp") == 0 && argc == 1) return exp(eval_expr(args[0]));
    if (strcmp(name, "noise") == 0 && argc == 1) {
        // TODO: Replace with real noise function
        return sin(eval_expr(args[0])); // Placeholder
    }

    // --- User-defined function fallback ---
    ASTNode *func = find_function(name);
    if (!func) {
        fprintf(stderr, "[Runtime] Function not found: %s\n", name);
        exit(1);
    }

    // Save old variable values for parameters
     enter_scope();

    int param_count = func->function_stmt.param_count;
    for (int i = 0; i < param_count; i++) {
        double arg_val = 0.0;
        if (i < node->call_expr.arg_count)
            arg_val = eval_expr(node->call_expr.args[i]);
        declare_var(func->function_stmt.params[i], arg_val);
    }

    runtime_has_returned = 0;
    runtime_return_value = 0.0;
    ASTNode *body = func->function_stmt.body;
    for (int i = 0; i < body->block.count; i++) {
        eval_expr(body->block.statements[i]);
        if (runtime_has_returned) break;
    }

    exit_scope();

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
    variables[var_count].scope_level = current_scope_level; // <-- set scope
    var_count++;
}



















void enter_scope() {
    current_scope_level++;
}

void exit_scope() {
    // Remove all variables at the current scope level
    for (int i = var_count - 1; i >= 0; --i) {
        if (variables[i].scope_level == current_scope_level) {
            free(variables[i].name);
            // Shift down
            for (int j = i; j < var_count - 1; ++j) {
                variables[j] = variables[j + 1];
            }
            var_count--;
        }
    }
    current_scope_level--;
}