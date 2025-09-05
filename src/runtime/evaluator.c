#include "evaluator.h"
#include <math.h>

// Compatibility math functions embedded directly
static double compat_hypot_impl(double x, double y) {
    return sqrt(x * x + y * y);
}

static double compat_exp_impl(double x) {
    if (x == 0.0) return 1.0;
    if (x < -700.0) return 0.0;  // Underflow protection
    if (x > 700.0) return INFINITY;  // Overflow protection
    
    // Taylor series: e^x = 1 + x + x^2/2! + x^3/3! + ...
    double result = 1.0;
    double term = 1.0;
    
    for (int i = 1; i < 50; i++) {
        term *= x / i;
        result += term;
        if (fabs(term) < 1e-15) break;  // Convergence check
    }
    
    return result;
}

static double compat_log_impl(double x) {
    if (x <= 0.0) return NAN;
    if (x == 1.0) return 0.0;
    
    // For x close to 1, use ln(1+u) = u - u^2/2 + u^3/3 - ...
    if (x > 0.5 && x < 1.5) {
        double u = x - 1.0;
        double result = 0.0;
        double term = u;
        
        for (int i = 1; i < 50; i++) {
            result += (i % 2 == 1 ? term : -term) / i;
            term *= u;
            if (fabs(term) < 1e-15) break;
        }
        return result;
    }
    
    // For other values, use change of base and recursion
    if (x > 1.5) {
        return compat_log_impl(x / 2.0) + 0.693147180559945309417;  // ln(2)
    } else {
        return -compat_log_impl(1.0 / x);
    }
}

static double compat_pow_impl(double x, double y) {
    if (y == 0.0) return 1.0;
    if (x == 0.0) return (y > 0.0) ? 0.0 : INFINITY;
    if (x == 1.0) return 1.0;
    if (y == 1.0) return x;
    
    // Handle integer powers efficiently
    if (y == (int)y) {
        int n = (int)y;
        if (n < 0) {
            return 1.0 / compat_pow_impl(x, -n);
        }
        
        double result = 1.0;
        double base = x;
        while (n > 0) {
            if (n % 2 == 1) result *= base;
            base *= base;
            n /= 2;
        }
        return result;
    }
    
    // For non-integer powers: x^y = e^(y * ln(x))
    if (x > 0.0) {
        return compat_exp_impl(y * compat_log_impl(x));
    }
    
    return NAN;  // Negative base with non-integer exponent
}

static double compat_fmod_impl(double x, double y) {
    if (y == 0.0) return NAN;
    if (x == 0.0) return 0.0;
    
    // Simple fmod implementation
    double quotient = x / y;
    int int_quotient = (int)quotient;
    if (quotient < 0 && quotient != int_quotient) {
        int_quotient--;  // Floor for negative numbers
    }
    
    return x - int_quotient * y;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../parser/ast_nodes.h"
#include <math.h>
#include "../parser/parser.h"
#include "error/error.h"
#define FATAL_ERROR(msg, ...) fatal_error(NULL, 0, 0, msg, ##__VA_ARGS__)
#include "config/config.h"
#include "generator/emitter.h"
#include "../utils/math_utils.h"
// Parser moved to runtime state - no more global parser

// Configuration variable detection
void check_config_variable(const char* name, Value* val) {
    if (!name || !val || val->type != VAL_NUMBER) {
        return;
    }
    
    if (strcmp(name, "nline") == 0) {
        set_enable_n_lines_from_var((int)val->number);
    }
    else if (strcmp(name, "decimalpoint") == 0) {
        set_decimal_places((int)val->number);
    }
}

#define MAX_VARIABLES 1024
#define MAX_FUNCTIONS 64

Value *eval_let(ASTNode *node);
Value *eval_function_call(ASTNode *node);
Value *runtime_return_value;

void free_value(Value *val); // Forward declaration
void register_function(ASTNode *node);
void eval_if(ASTNode *stmt);
void eval_for(ASTNode *stmt);
void eval_while(ASTNode *stmt);

static ASTNode *find_function(const char *name);
int runtime_has_returned = 0;

double get_number(const Value *val)
{
    if (!val || val->type != VAL_NUMBER)
    {
        report_error("[Runtime evaluator] Expected number value");
        FATAL_ERROR("[Runtime evaluator] Expected number value");
    }
    return val->number;
}

double get_scalar(ASTNode *node)
{
    const Value *v = eval_expr(node);
    if (!v || v->type != VAL_NUMBER)
    {
        report_error("[Runtime evaluator] get_scalar() expected number");
        return 0.0;
    }
    return v->number;
}

Value *eval_let(ASTNode *node)
{
    const char *name = node->let_stmt.name;
    Value *value = eval_expr(node->let_stmt.expr);
    declare_var(name, value);
    return make_number_value(0.0); // Return a default value, since LET is not an expression
}

Value *eval(ASTNode *node)
{
    return eval_expr(node); // delegate to eval_expr
}

Value *get_var(const char *name)
{
    const Runtime *rt = get_runtime();
    for (int i = rt->var_count - 1; i >= 0; --i)
    {
        if (strcmp(rt->variables[i].name, name) == 0)
        {
            return rt->variables[i].val;
        }
    }
    report_error("[Runtime evaluator] Variable not found: %s", name);
    return NULL;
}

void set_parents_recursive(ASTNode *node, ASTNode *parent) {
    if (!node) return;

    node->parent = parent;

    switch (node->type) {
        case AST_LET:
            set_parents_recursive(node->let_stmt.expr, node);
            break;
        case AST_ASSIGN:
            set_parents_recursive(node->assign_stmt.expr, node);
            break;
        case AST_COMPOUND_ASSIGN:
            set_parents_recursive(node->compound_assign.expr, node);
            break;
        case AST_UNARY:
            set_parents_recursive(node->unary_expr.operand, node);
            break;
        case AST_BINARY:
            set_parents_recursive(node->binary_expr.left, node);
            set_parents_recursive(node->binary_expr.right, node);
            break;
        case AST_TERNARY:
            set_parents_recursive(node->ternary_expr.condition, node);
            set_parents_recursive(node->ternary_expr.true_expr, node);
            set_parents_recursive(node->ternary_expr.false_expr, node);
            break;
        case AST_IF:
            set_parents_recursive(node->if_stmt.condition, node);
            set_parents_recursive(node->if_stmt.then_branch, node);
            if (node->if_stmt.else_branch)
                set_parents_recursive(node->if_stmt.else_branch, node);
            break;
        case AST_WHILE:
            set_parents_recursive(node->while_stmt.condition, node);
            set_parents_recursive(node->while_stmt.body, node);
            break;
        case AST_FOR:
            if (node->for_stmt.from)
                set_parents_recursive(node->for_stmt.from, node);
            if (node->for_stmt.to)
                set_parents_recursive(node->for_stmt.to, node);
            if (node->for_stmt.step)
                set_parents_recursive(node->for_stmt.step, node);
            if (node->for_stmt.iterable)
                set_parents_recursive(node->for_stmt.iterable, node);
            set_parents_recursive(node->for_stmt.body, node);
            break;
        case AST_FUNCTION:
            set_parents_recursive(node->function_stmt.body, node);
            break;
        case AST_CALL:
            for (int i = 0; i < node->call_expr.arg_count; i++)
                set_parents_recursive(node->call_expr.args[i], node);
            break;
        case AST_ARRAY_LITERAL:
            for (int i = 0; i < node->array_literal.count; i++)
                set_parents_recursive(node->array_literal.elements[i], node);
            break;
        case AST_BLOCK:
            for (int i = 0; i < node->block.count; i++)
                set_parents_recursive(node->block.statements[i], node);
            break;
        case AST_INDEX:
            set_parents_recursive(node->index_expr.array, node);
            set_parents_recursive(node->index_expr.index, node);
            break;
        case AST_RETURN:
            set_parents_recursive(node->return_stmt.expr, node);
            break;
        default:
            // AST_VAR, AST_NUMBER, etc. do not have children
            break;
    }
}

// Cleanup function for error recovery - ensures system returns to stable state
void cleanup_recursion_error_state(void)
{
    Runtime *rt = get_runtime();
    
    // Reset recursion depth to 0 to prevent cascading errors
    rt->recursion_depth = 0;
    
    // Clear return state to prevent stale return values
    runtime_has_returned = 0;
    if (runtime_return_value) {
        free_value(runtime_return_value);
        runtime_return_value = NULL;
    }
    
    // Clean up function stack to prevent stack corruption
    function_stack_init();
    
    // Clean up any open scopes to prevent memory leaks
    while (rt->current_scope_level > 0) {
        exit_scope();
    }
}

void reset_runtime_state(void)
{
    Runtime *rt = get_runtime();



    // Clean up all scopes using exit_scope
    while (rt->current_scope_level > 0) {
        exit_scope();
    }
    // If any variables remain (e.g., global scope), clean them up too
    if (rt->var_count > 0) {
        exit_scope();
    }

    rt->function_count = 0;
    runtime_has_returned = 0;

    if (runtime_return_value) {
        free_value(runtime_return_value);
        runtime_return_value = NULL;
    }

    // Reset runtime state
    memset(rt, 0, sizeof(Runtime));

    // Initialize recursion protection
    rt->recursion_depth = 0;
    rt->max_recursion_depth = 100;  // Default limit for recursion protection

    // Initialize function stack
    function_stack_init();

    // Reset emitter state
    extern void reset_emitter_state(void);
    reset_emitter_state();

    // Reset parser state
    reset_parser_state();


}

void enter_scope()
{
    Runtime *rt = get_runtime();
    rt->current_scope_level++;

}

void reset_parser_state() {
    Runtime *rt = get_runtime();
    // Free the lexer if it exists
    if (rt->parser.lexer) {
        lexer_free(rt->parser.lexer);
        rt->parser.lexer = NULL;
    }
    // Reset the rest of the parser state
    memset(&rt->parser, 0, sizeof(Parser));  // full reset
}

//step 1
ASTNode *parse_script_from_string(const char *source)
{
    Runtime *rt = get_runtime();

    enter_scope();
    runtime_has_returned = 0;
    rt->current_scope_level = 0;

    rt->parser.lexer = lexer_new(source);

    parser_advance();
    ASTNode *root = parse_script();
    set_parents_recursive(root, NULL);
    reset_parser_state();
    reset_line_number();

    return root;
}

Value *copy_value(Value *val)
{
    if (!val) {

        return NULL;
    }

    Value *copy = malloc(sizeof(Value));
    if (!copy) {

        FATAL_ERROR("[copy_value] malloc failed for Value");
    }

    copy->type = val->type;

    if (val->type == VAL_NUMBER)
    {

        copy->number = val->number;
    }
else if (val->type == VAL_ARRAY)
{

    copy->array.count = val->array.count;
    copy->array.items = malloc(sizeof(Value *) * val->array.count);
    if (!copy->array.items) {
        free(copy);
        FATAL_ERROR("[copy_value] malloc failed for array Value");
    }

    for (size_t i = 0; i < val->array.count; ++i)
    {

        copy->array.items[i] = copy_value(val->array.items[i]);
        if (!copy->array.items[i]) {
            report_error("[copy_value] Failed to copy element %zu — cleaning up", i);
            // Free previously allocated items
            for (size_t j = 0; j < i; ++j) {
                if (copy->array.items[j]) free(copy->array.items[j]);
            }
            free(copy->array.items);
            free(copy);
            FATAL_ERROR("[copy_value] Failed to copy element %zu — cleaning up", i);
        }
    }
}
    else if (val->type == VAL_STRING)
    {
        if (val->string) {
            copy->string = strdup(val->string);
            if (!copy->string) {
                free(copy);
                FATAL_ERROR("[copy_value] strdup failed for string Value");
            }
        } else {
            copy->string = strdup("");
            if (!copy->string) {
                free(copy);
                FATAL_ERROR("[copy_value] strdup failed for empty string Value");
            }
        }
    }
    else
    {
        printf("[copy_value] Unknown Value type: %d\n", val->type);
    }

    return copy;
}

// Evaluate expressions
Value *eval_expr(ASTNode *node)
{
    if (!node)
    {
        report_error("[eval_expr] NULL node passed to eval_expr");
        return NULL;
    }

    switch (node->type)
    {
    case AST_NUMBER:
        return make_number_value(node->number.value);

    case AST_STRING:
        return make_string_value(node->string_literal.value);

case AST_VAR:
{
    Value *val = get_var(node->var.name);
    if (!val)
    {
        report_error("[Runtime evaluator] Variable '%s' is undefined", node->var.name);
        return make_number_value(0.0);
    }
    return val;  // ✅ Return original value, whether number or array
}


    case AST_LET:
        eval_let(node);                // <-- Actually execute the let
        return make_number_value(0.0); // Return dummy value to satisfy eval_expr()

    case AST_UNARY:
    {
        double operand = get_number(eval_expr(node->unary_expr.operand));
        switch (node->unary_expr.op)
        {
        case TOKEN_BANG:
            return make_number_value(!operand);
        case TOKEN_MINUS:
            return make_number_value(-operand);
        default:

            report_error("[Runtime evaluator] Unknown unary operator: %d", node->unary_expr.op);
            return make_number_value(0.0);
        }
    }

    case AST_BINARY:
    {
        Value *left_val = eval_expr(node->binary_expr.left);
        Value *right_val = eval_expr(node->binary_expr.right);
        Token_Type op = node->binary_expr.op;

        if (!left_val || !right_val) {
            report_error("[Runtime evaluator] Failed to evaluate binary expression operands");
            return make_number_value(0.0);
        }

        // Handle comparison operators that can work with different types
        if (op == TOKEN_EQUAL_EQUAL || op == TOKEN_BANG_EQUAL) {
            int result = 0;
            
            // Both are strings - compare string values
            if (left_val->type == VAL_STRING && right_val->type == VAL_STRING) {
                result = strcmp(left_val->string, right_val->string) == 0;
            }
            // Both are numbers - compare numeric values
            else if (left_val->type == VAL_NUMBER && right_val->type == VAL_NUMBER) {
                result = left_val->number == right_val->number;
            }
            // Different types - not equal
            else {
                result = 0;
            }
            
            // Apply negation for != operator
            if (op == TOKEN_BANG_EQUAL) {
                result = !result;
            }
            
            return make_number_value(result ? 1.0 : 0.0);
        }

        // For all other operations, we need numbers
        if (left_val->type != VAL_NUMBER || right_val->type != VAL_NUMBER) {
            report_error("[Runtime evaluator] Arithmetic operations require numeric operands");
            return make_number_value(0.0);
        }

        double left = left_val->number;
        double right = right_val->number;

        switch (op)
        {
        case TOKEN_PLUS:
            return make_number_value(left + right);
        case TOKEN_MINUS:
            return make_number_value(left - right);
        case TOKEN_STAR:
            return make_number_value(left * right);
        case TOKEN_SLASH:
            if (right == 0.0)
            {
                // Modern IEEE 754 compliant division by zero handling
                if (left > 0.0) {
                    report_error("[Runtime evaluator] Division by zero: %.5f / 0.0 = +∞", left);
                    return make_raw_number_value(INFINITY);
                } else if (left < 0.0) {
                    report_error("[Runtime evaluator] Division by zero: %.5f / 0.0 = -∞", left);
                    return make_raw_number_value(-INFINITY);
                } else {
                    report_error("[Runtime evaluator] Indeterminate form: 0.0 / 0.0 = NaN");
                    return make_raw_number_value(NAN);
                }
            }
            return make_number_value(left / right);
        case TOKEN_CARET:
            return make_number_value(compat_pow_impl(left, right));
        case TOKEN_LESS:
            return make_number_value(left < right ? 1.0 : 0.0);
        case TOKEN_LESS_EQUAL:
            return make_number_value(left <= right ? 1.0 : 0.0);
        case TOKEN_GREATER:
            return make_number_value(left > right ? 1.0 : 0.0);
        case TOKEN_GREATER_EQUAL:
            return make_number_value(left >= right ? 1.0 : 0.0);

        case TOKEN_BANG_EQUAL:
            // This case is handled above with TOKEN_EQUAL_EQUAL
            return make_number_value(left != right ? 1.0 : 0.0);
        case TOKEN_AND:
            return make_number_value((left != 0.0 && right != 0.0) ? 1.0 : 0.0);

        case TOKEN_OR:
        {
            double result = (left != 0.0 || right != 0.0) ? 1.0 : 0.0;
            //printf("[Eval] OR: %.2f || %.2f => %.2f\n", left, right, result);
            return make_number_value(result);
        }

        case TOKEN_AMPERSAND:
            return make_number_value((double)((int)left & (int)right));
        
        case TOKEN_PIPE:
            return make_number_value((double)((int)left | (int)right));
        
        case TOKEN_LSHIFT:
            return make_number_value((double)((int)left << (int)right));
        
        case TOKEN_RSHIFT:
            return make_number_value((double)((int)left >> (int)right));
        
        default:

            report_error("[Runtime evaluator] Unknown binary operator: %d", op);
            return make_number_value(0.0);
        }
    }

    case AST_ARRAY_LITERAL:
    {
        int count = node->array_literal.count;

        Value **items = malloc(sizeof(Value *) * count);
        if (!items)
        {

            report_error("[Runtime evaluator] malloc failed for array literal");
            FATAL_ERROR("[Runtime evaluator] malloc failed for array literal");
        }

        for (int i = 0; i < count; i++)
        {
            ASTNode *element = node->array_literal.elements[i];

            // Recursively evaluate (handles nested arrays naturally)
            Value *v = eval_expr(element);
            items[i] = v;
        }

        Value *arr_val = malloc(sizeof(Value));

        if (!arr_val)
        {
            report_error("[Runtime evaluator] malloc failed for array Value");
            FATAL_ERROR("[Runtime evaluator] malloc failed for array Value");
        }

        arr_val->type = VAL_ARRAY;
        arr_val->array.items = items;
        arr_val->array.count = count;

        // Return the array value (no parent access to prevent crashes)
        return arr_val;

    }

    case AST_FOR:
        return eval_function_call(node);

    case AST_CALL:
        return eval_function_call(node);

    case AST_IF:
    {
        const Value *cond = eval_expr(node->if_stmt.condition);
        if (cond && cond->type == VAL_NUMBER)
        {
            if (cond->number != 0)
            {
                Value *result = eval_expr(node->if_stmt.then_branch);
                // Return statements in if expressions are handled by the branch evaluation
                return result;
            }
            else if (node->if_stmt.else_branch)
            {
                Value *result = eval_expr(node->if_stmt.else_branch);
                // Return statements in else expressions are handled by the branch evaluation
                return result;
            }
        }
        return make_number_value(0.0);
    }

    case AST_TERNARY:
    {
        const Value *cond = eval_expr(node->ternary_expr.condition);
        if (cond && cond->type == VAL_NUMBER)
        {
            if (cond->number != 0)
            {
                return eval_expr(node->ternary_expr.true_expr);
            }
            else
            {
                return eval_expr(node->ternary_expr.false_expr);
            }
        }
        // If condition is not a number, default to false branch
        return eval_expr(node->ternary_expr.false_expr);
    }

case AST_RETURN:
{
    runtime_has_returned = 1;
    
    // Handle null expressions (bare return statements)
    if (node->return_stmt.expr == NULL) {
        // Clean up any existing return value
        if (runtime_return_value) {
            free_value(runtime_return_value);
        }
        runtime_return_value = make_number_value(0.0);
        return runtime_return_value;
    }
    
    // Evaluate the return expression
    Value *ret = eval_expr(node->return_stmt.expr);
    if (!ret) {
        // If expression evaluation failed, return 0
        if (runtime_return_value) {
            free_value(runtime_return_value);
        }
        runtime_return_value = make_number_value(0.0);
        return runtime_return_value;
    }
    
    // Clean up any existing return value before setting new one
    if (runtime_return_value) {
        free_value(runtime_return_value);
    }
    
    // Copy the return value for later use
    runtime_return_value = copy_value(ret);
    return ret;
}


    case AST_FUNCTION:
        register_function(node);
        return make_number_value(0.0);

    case AST_ASSIGN:
        set_var(node->assign_stmt.name, eval_expr(node->assign_stmt.expr));
        return make_number_value(0.0);

    case AST_COMPOUND_ASSIGN:
    {
        // Get current value of the variable
        Value *current = get_var(node->compound_assign.name);
        if (!current) {
            report_error("[eval_expr] Variable '%s' not found for compound assignment", node->compound_assign.name);
            return make_number_value(0.0);
        }
        
        // Evaluate the right-hand side expression
        Value *rhs = eval_expr(node->compound_assign.expr);
        if (!rhs) {
            report_error("[eval_expr] Failed to evaluate RHS of compound assignment");
            return make_number_value(0.0);
        }

        // Perform the compound operation
        double result = 0.0;
        switch (node->compound_assign.op) {
            case TOKEN_PLUS_EQUAL:
                result = current->number + rhs->number;
                break;
            case TOKEN_MINUS_EQUAL:
                result = current->number - rhs->number;
                break;
            case TOKEN_STAR_EQUAL:
                result = current->number * rhs->number;
                break;
            case TOKEN_SLASH_EQUAL:
                if (rhs->number == 0.0) {
                    // Modern IEEE 754 compliant division by zero handling for compound assignment
                    if (current->number > 0.0) {
                        report_error("[eval_expr] Division by zero in /= operation: %.5f /= 0.0 = +∞", current->number);
                        result = INFINITY;
                    } else if (current->number < 0.0) {
                        report_error("[eval_expr] Division by zero in /= operation: %.5f /= 0.0 = -∞", current->number);
                        result = -INFINITY;
                    } else {
                        report_error("[eval_expr] Indeterminate form in /= operation: 0.0 /= 0.0 = NaN");
                        result = NAN;
                    }
                } else {
                    result = current->number / rhs->number;
                }
                break;
            case TOKEN_CARET_EQUAL:
                result = compat_pow_impl(current->number, rhs->number);
                break;
            case TOKEN_AMPERSAND_EQUAL:
                result = (double)((int)current->number & (int)rhs->number);
                break;
            case TOKEN_PIPE_EQUAL:
                result = (double)((int)current->number | (int)rhs->number);
                break;
            case TOKEN_LSHIFT_EQUAL:
                result = (double)((int)current->number << (int)rhs->number);
                break;
            case TOKEN_RSHIFT_EQUAL:
                result = (double)((int)current->number >> (int)rhs->number);
                break;
            default:
                report_error("[eval_expr] Unknown compound assignment operator");
                return make_number_value(0.0);
        }

        // Set the new value (use raw if it contains special values)
        if (isnan(result) || isinf(result)) {
            set_var(node->compound_assign.name, make_raw_number_value(result));
        } else {
            set_var(node->compound_assign.name, make_number_value(result));
        }
        return make_number_value(0.0);
    }



case AST_INDEX:
{
    Value *target = eval_expr(node->index_expr.array);
    const Value *index_val = eval_expr(node->index_expr.index);

    if (!target || target->type != VAL_ARRAY) {
        report_error("[Runtime] AST_INDEX: Not an array");
        return make_number_value(0.0);
    }

    if (!index_val || index_val->type != VAL_NUMBER) {
        report_error("[Runtime] AST_INDEX: Index is not a number");
        return make_number_value(0.0);
    }

    int index = (int)index_val->number;
    if (index < 0 || index >= (int)target->array.count) {
        report_error("[Runtime] AST_INDEX: Index %d out of bounds (size = %zu)", index, target->array.count);
        return make_number_value(0.0);
    }

    Value *result = target->array.items[index];
    if (!result) {
        report_error("[Runtime] AST_INDEX: NULL element at index %d", index);
        return make_number_value(0.0);
    }

    // ✅ Return the value directly (even if it’s another array)
    return result;
}



case AST_BLOCK:
    eval_block(node);
    return make_number_value(0.0);





case AST_WHILE: {
    while (get_number(eval_expr(node->while_stmt.condition))) {
        // While loop body should be evaluated as a block, not an expression
        if (node->while_stmt.body->type == AST_BLOCK) {
            eval_block(node->while_stmt.body);
        } else {
            eval_expr(node->while_stmt.body);
        }

        if (runtime_has_returned) {
            break;
        }
    }
    return make_number_value(0.0);
}





    case AST_EXPR_STMT:
        // Expression statements should evaluate their inner expression
        if (node->expr_stmt.expr) {
            return eval_expr(node->expr_stmt.expr);
        }
        return make_number_value(0.0);

    default:

        report_error("[Runtime evaluator] Unsupported expression type: %d (%s)", node->type, get_ast_type_name(node->type));

        return make_number_value(0.0);
    }
}

Value *eval_function_call(ASTNode *node)
{
    const char *name = node->call_expr.name;
    int argc = node->call_expr.arg_count;
    ASTNode **args = node->call_expr.args;

// Helper to extract double
#define SCALAR(i) get_scalar(args[i])

    // --- Constants ---
    if (strcmp(name, "PI") == 0)
        return make_number_value(M_PI);
    if (strcmp(name, "TAU") == 0)
        return make_number_value(2.0 * M_PI);
    if (strcmp(name, "EU") == 0)
        return make_number_value(M_E);
    if (strcmp(name, "DEG_TO_RAD") == 0)
        return make_number_value(M_PI / 180.0);
    if (strcmp(name, "RAD_TO_DEG") == 0)
        return make_number_value(180.0 / M_PI);

    // --- Basic Math ---
    if (strcmp(name, "abs") == 0 && argc == 1)
        return make_number_value(fabs(SCALAR(0)));
    if (strcmp(name, "mod") == 0 && argc == 2)
        return make_number_value(compat_fmod_impl(SCALAR(0), SCALAR(1)));
    if (strcmp(name, "floor") == 0 && argc == 1)
        return make_number_value(floor(SCALAR(0)));
    if (strcmp(name, "ceil") == 0 && argc == 1)
        return make_number_value(ceil(SCALAR(0)));
    if (strcmp(name, "round") == 0 && argc == 1)
        return make_number_value(round(SCALAR(0)));
    if (strcmp(name, "min") == 0 && argc == 2)
        return make_number_value(fmin(SCALAR(0), SCALAR(1)));
    if (strcmp(name, "max") == 0 && argc == 2)
        return make_number_value(fmax(SCALAR(0), SCALAR(1)));
    
    // --- Safe Math Functions ---
    if (strcmp(name, "safe_divide") == 0 && argc == 2) {
        double result = safe_divide(SCALAR(0), SCALAR(1));
        return (isnan(result) || isinf(result)) ? make_raw_number_value(result) : make_number_value(result);
    }
    if (strcmp(name, "is_finite") == 0 && argc == 1)
        return make_number_value(is_safe_number(SCALAR(0)) ? 1.0 : 0.0);
    if (strcmp(name, "is_nan") == 0 && argc == 1)
        return make_number_value(isnan(SCALAR(0)) ? 1.0 : 0.0);
    if (strcmp(name, "is_inf") == 0 && argc == 1)
        return make_number_value(isinf(SCALAR(0)) ? 1.0 : 0.0);
    if (strcmp(name, "clamp") == 0 && argc == 3)
    {
        double v = SCALAR(0);
        double lo = SCALAR(1);
        double hi = SCALAR(2);
        return make_number_value(fmin(fmax(v, lo), hi));
    }

    // --- Trig ---
    if (strcmp(name, "tan") == 0 && argc == 1)
        return make_number_value(tan(SCALAR(0)));
    if (strcmp(name, "asin") == 0 && argc == 1)
        return make_number_value(asin(SCALAR(0)));
    if (strcmp(name, "acos") == 0 && argc == 1)
        return make_number_value(acos(SCALAR(0)));
    if (strcmp(name, "atan") == 0 && argc == 1)
        return make_number_value(atan(SCALAR(0)));
    if (strcmp(name, "atan2") == 0 && argc == 2)
        return make_number_value(atan2(SCALAR(0), SCALAR(1)));

    if (strcmp(name, "sin") == 0 && argc == 1)
        return make_number_value(sin(SCALAR(0)));
    if (strcmp(name, "cos") == 0 && argc == 1)
        return make_number_value(cos(SCALAR(0)));
    // Add tan, atan, etc. as needed...

    // --- Geometry / Vector ---
    if (strcmp(name, "sqrt") == 0 && argc == 1)
        return make_number_value(sqrt(SCALAR(0)));
    if (strcmp(name, "pow") == 0 && argc == 2)
        return make_number_value(compat_pow_impl(SCALAR(0), SCALAR(1)));
    if (strcmp(name, "hypot") == 0 && argc == 2)
        return make_number_value(compat_hypot_impl(SCALAR(0), SCALAR(1)));
    if (strcmp(name, "lerp") == 0 && argc == 3)
    {
        double a = SCALAR(0), b = SCALAR(1), t = SCALAR(2);
        return make_number_value(a + t * (b - a));
    }
    if (strcmp(name, "map") == 0 && argc == 5)
    {
        double v = SCALAR(0), in_min = SCALAR(1), in_max = SCALAR(2);
        double out_min = SCALAR(3), out_max = SCALAR(4);
        return make_number_value(out_min + ((v - in_min) * (out_max - out_min)) / (in_max - in_min));
    }
    if (strcmp(name, "distance") == 0 && argc == 4)
    {
        double x1 = SCALAR(0), y1 = SCALAR(1), x2 = SCALAR(2), y2 = SCALAR(3);
        return make_number_value(compat_hypot_impl(x2 - x1, y2 - y1));
    }

    // --- Optional / parser_advanced ---
    if (strcmp(name, "sign") == 0 && argc == 1)
    {
        double x = SCALAR(0);
        return make_number_value((x > 0) - (x < 0));
    }

    if (strcmp(name, "deg") == 0 && argc == 1)
    {
        return make_number_value(SCALAR(0) * (180.0 / M_PI));
    }

    if (strcmp(name, "rad") == 0 && argc == 1)
    {
        return make_number_value(SCALAR(0) * (M_PI / 180.0));
    }

    if (strcmp(name, "log") == 0 && argc == 1)
    {
        return make_number_value(compat_log_impl(SCALAR(0)));
    }

    if (strcmp(name, "exp") == 0 && argc == 1)
        return make_number_value(compat_exp_impl(SCALAR(0)));
    if (strcmp(name, "noise") == 0 && argc == 1)
    {
        return make_number_value(sin(SCALAR(0))); // Placeholder
    }

    // --- User-defined function ---
    ASTNode *func = find_function(name);
    if (!func)
    {
        report_error("[Runtime] Function not found: %s", name);
        return make_number_value(0.0);
    }

    // Check recursion depth before entering function scope
    Runtime *rt = get_runtime();
    if (rt->recursion_depth >= rt->max_recursion_depth) {
        report_error("Recursion limit exceeded (%d calls). Possible infinite recursion in function '%s'", 
                     rt->max_recursion_depth, name);
        
        // Perform proper cleanup when recursion limit is exceeded
        // Reset recursion depth to prevent further issues
        rt->recursion_depth = 0;
        
        // Clear any pending return state to ensure clean state
        runtime_has_returned = 0;
        if (runtime_return_value) {
            free_value(runtime_return_value);
            runtime_return_value = NULL;
        }
        
        // Return safe default value instead of crashing
        return make_number_value(0.0);
    }

    // Increment recursion depth when entering function execution
    rt->recursion_depth++;

    // ✅ Clear return state before executing function
    runtime_has_returned = 0;
    runtime_return_value = NULL;

    enter_scope();
    
    // Push function context for return statement validation
    if (!function_stack_push(name, func)) {
        // Function stack push failed, clean up and return
        rt->recursion_depth--;
        exit_scope();
        return make_number_value(0.0);
    }

    int param_count = func->function_stmt.param_count;
    for (int i = 0; i < param_count; i++)
    {
        Value *arg_val = make_number_value(0.0);
if (i < argc)
    arg_val = eval_expr(args[i]);

if (!arg_val) {
    fprintf(stderr, "🚨 ERROR: Failed to evaluate argument %d for function '%s'\n", i, name);
    // Proper cleanup before fatal error
    rt->recursion_depth--;
    function_stack_pop();
    exit_scope();
    
    // Clear return state to prevent stale values
    runtime_has_returned = 0;
    if (runtime_return_value) {
        free_value(runtime_return_value);
        runtime_return_value = NULL;
    }
    
    FATAL_ERROR("🚨 ERROR: Failed to evaluate argument %d for function '%s'", i, name);
}



        declare_var(func->function_stmt.params[i], arg_val);
    }

ASTNode *body = func->function_stmt.body;

// ⚠️ Fix: Use emit_gcode instead of eval_expr for statements
for (int i = 0; i < body->block.count; i++) {
    ASTNode *stmt = body->block.statements[i];

    // ✅ Use emit_gcode for statements that need proper G-code emission
    // Use eval_expr for everything else to maintain proper execution semantics
    if (stmt->type == AST_GCODE || stmt->type == AST_NOTE || stmt->type == AST_FOR) {
        emit_gcode(stmt);
    } else {
        // Handle expressions, assignments, conditionals, and returns through eval_expr
        eval_expr(stmt);
    }

    if (runtime_has_returned)
        break;
}


























    // Pop function context before exiting scope
    function_stack_pop();
    
    exit_scope();

    // Decrement recursion depth when function returns normally
    rt->recursion_depth--;

    // ✅ Return the result if set, or 0 otherwise
    if (runtime_return_value)
        return copy_value(runtime_return_value);
    else
        return make_number_value(0.0);

#undef SCALAR
}

int var_exists_in_current_scope(const char *name)
{
    const Runtime *rt = get_runtime();
    for (int i = rt->var_count - 1; i >= 0; --i)
    {
        if (rt->variables[i].name && strcmp(rt->variables[i].name, name) == 0)
        {
            return rt->variables[i].scope_level == rt->current_scope_level;
        }
    }
    return 0;
}







void eval_block(ASTNode *block)
{
    if (!block || block->type != AST_BLOCK)
    {
        report_error("[Runtime evaluator] Invalid block node (null or wrong type)");
        return;
    }

    enter_scope();

    for (int i = 0; i < block->block.count; ++i)
    {
        ASTNode *stmt = block->block.statements[i];

        switch (stmt->type)
        {
        case AST_LET:
        {
            Value *val = eval_expr(stmt->let_stmt.expr);
            if (!val) {
                report_error("[Runtime evaluator] Failed to evaluate expression in LET at block index %d", i);
                break;
            }

            const Runtime *rt = get_runtime();
            if (!var_exists_in_current_scope(stmt->let_stmt.name))
            {
                printf("[Runtime evaluator] DECLARE '%s' = %.4f (scope level %d)\n",
                       stmt->let_stmt.name,
                       val->type == VAL_NUMBER ? val->number : -9999,
                       rt->current_scope_level);
                declare_var(stmt->let_stmt.name, val);
            }
            else
            {
                printf("[Runtime evaluator] UPDATE '%s' = %.4f (already in scope level %d)\n",
                       stmt->let_stmt.name,
                       val->type == VAL_NUMBER ? val->number : -9999,
                       rt->current_scope_level);
                set_var(stmt->let_stmt.name, val);
            }
            break;
        }

        case AST_ASSIGN:
            eval_expr(stmt);
            break;

        case AST_COMPOUND_ASSIGN:
            eval_expr(stmt);
            break;

        case AST_IF:
            eval_if(stmt);
            break;

        case AST_FOR:
            eval_for(stmt);
            break;

        case AST_WHILE:
            eval_while(stmt);
            break;

        case AST_BLOCK:
            eval_block(stmt); // Nested block
            break;

        case AST_FUNCTION:
            register_function(stmt);
            break;

        case AST_RETURN:
            eval_expr(stmt);  // Sets runtime_return_value internally
            break;

        case AST_CALL:
            eval_expr(stmt);  // Might have side effects
            break;

        case AST_NOTE:

            break;



        case AST_GCODE:

            break;



        case AST_EXPR_STMT:
            // Expression statements should evaluate their inner expression
            if (stmt->expr_stmt.expr) {
                eval_expr(stmt->expr_stmt.expr);
            }
            break;


        default:
            report_error("[Runtime evaluator] Unsupported stmt type: %d (%s) at block index %d",
                         stmt->type, get_ast_type_name(stmt->type), i);
            break;
        }

        // ✅ Short-circuit if return was hit
        if (runtime_has_returned) {
            break;
        }
    }

    exit_scope();
}

void eval_if(ASTNode *stmt)
{
    const Value *cond = eval_expr(stmt->if_stmt.condition);
    if (cond && cond->type == VAL_NUMBER)
    {
        //printf("[Eval_IF evaluator] condition evaluates to: %.2f\n", cond->number);
    }
    else
    {
        //printf("[Eval_IF evaluator] condition is NULL or non-number\n");
    }

    if (cond && cond->type == VAL_NUMBER && cond->number != 0)
    {
        eval_block(stmt->if_stmt.then_branch);
        // Return statements in if blocks are handled by eval_block
        // No need to check runtime_has_returned here as it propagates up
    }
    else if (stmt->if_stmt.else_branch)
    {
        eval_block(stmt->if_stmt.else_branch);
        // Return statements in else blocks are handled by eval_block
        // No need to check runtime_has_returned here as it propagates up
    }
}

void eval_for(ASTNode *stmt)
{
    // Handle string iteration: for char in string_var or for (char, index) in string_var
    if (stmt->for_stmt.is_string_iteration) {
        const Value *iterable_val = eval_expr(stmt->for_stmt.iterable);
        
        if (!iterable_val || iterable_val->type != VAL_STRING) {
            report_error("[Runtime evaluator] FOR-IN loop expects string value");
            return;
        }
        
        const char *str = iterable_val->string;
        if (!str) {
            return; // Empty string, nothing to iterate
        }
        
        // Iterate through each character
        for (int i = 0; str[i] != '\0'; i++) {
            // Create a single-character string for the loop variable
            char char_str[2] = {str[i], '\0'};
            set_var(stmt->for_stmt.var, make_string_value(char_str));
            
            // If index variable is specified, set it too
            if (stmt->for_stmt.index_var) {
                set_var(stmt->for_stmt.index_var, make_number_value((double)i));
            }
            
            eval_block(stmt->for_stmt.body);
            
            // Check if return was encountered in the loop body
            if (runtime_has_returned) {
                break;
            }
        }
        return;
    }

    // Traditional numeric for loop: for i = 1..10
    const Value *v_from = eval_expr(stmt->for_stmt.from);
    const Value *v_to = eval_expr(stmt->for_stmt.to);
    const Value *v_step = stmt->for_stmt.step ? eval_expr(stmt->for_stmt.step) : make_number_value(1.0);

    if (!v_from || !v_to || !v_step ||
        v_from->type != VAL_NUMBER || v_to->type != VAL_NUMBER || v_step->type != VAL_NUMBER)
    {
        //printf("[Runtime evaluator] ERROR: FOR loop expects numeric values\n");
        return;
    }

    double from = v_from->number;
    double to = v_to->number;
    double step = v_step->number;
    int exclusive = stmt->for_stmt.exclusive;

    double end = exclusive ? to : (step > 0 ? to + 1e-9 : to - 1e-9);

    for (double i = from;
         (step > 0 && i < end) || (step < 0 && i > end);
         i += step)
    {
        set_var(stmt->for_stmt.var, make_number_value(i));
        eval_block(stmt->for_stmt.body);
        
        // Check if return was encountered in the loop body
        if (runtime_has_returned) {
            break;
        }
    }
}





































void eval_while(ASTNode *stmt)
{
    enter_scope();
    int iter = 0;

    while (1)
    {
        const Value *cond = eval_expr(stmt->while_stmt.condition);

        if (!cond)
        {
            fprintf(stderr, "[EVAL evaluator] WHILE condition is NULL!\n");
            break;
        }

        if (cond->type != VAL_NUMBER)
        {
            fprintf(stderr, "[EVAL evaluator] WHILE condition not a number!\n");
            break;
        }

        //printf("[EVAL evaluator] WHILE ITERATION %d, cond = %.2f\n", iter, cond->number);
        //fflush(stdout);

        if (cond->number == 0)
        {
            break;
        }

        eval_block(stmt->while_stmt.body);
        
        // Check if return was encountered in the loop body
        if (runtime_has_returned) {
            break;
        }
        
        iter++;

        if (iter > 100)
        {
            fprintf(stderr, "[EVAL evaluator] WHILE loop exceeded 1000 iterations — breaking.\n");
            break;
        }
    }

    exit_scope();
}






void declare_var(const char *name, Value *val)
{
    Runtime *rt = get_runtime();

    // Prevent duplicate variable names in the same scope
    for (int i = rt->var_count - 1; i >= 0; --i) {
        if (rt->variables[i].scope_level != rt->current_scope_level)
            break;
        if (rt->variables[i].name && strcmp(rt->variables[i].name, name) == 0) {
            report_error("[declare_var] ERROR: variable '%s' already declared in current scope.", name);
            FATAL_ERROR("[declare_var] ERROR: variable '%s' already declared in current scope.", name);
        }
    }
    if (rt->var_count >= MAX_VARIABLES)
    {
        report_error("[declare_var] ERROR: variable limit reached.");
        FATAL_ERROR("[declare_var] ERROR: variable limit reached.");
    }
    rt->variables[rt->var_count].name = strdup(name);
    // Always copy the value for safety
    Value *copy = copy_value(val);
    if (!copy) {
        report_error("[declare_var] ERROR: failed to copy value for '%s'", name);
        FATAL_ERROR("[declare_var] ERROR: failed to copy value for '%s'", name);
    }
    rt->variables[rt->var_count].val = copy;
    rt->variables[rt->var_count].scope_level = rt->current_scope_level;
    rt->var_count++;
    
    // Check for configuration variables
    check_config_variable(name, val);
}




// Check if variable exists
int var_exists(const char *name)
{
    const Runtime *rt = get_runtime();
    for (int i = 0; i < rt->var_count; i++)
    {
        if (rt->variables[i].name && strcmp(rt->variables[i].name, name) == 0)
        {
            return 1;
        }
    }
    return 0;
}

void register_function(ASTNode *node)
{
    Runtime *rt = get_runtime();
    if (rt->function_count >= MAX_FUNCTIONS)
    {
        report_error("[Runtime] Too many functions");
        FATAL_ERROR("[Runtime] Too many functions");
    }
    rt->function_table[rt->function_count].name = node->function_stmt.name;
    rt->function_table[rt->function_count].node = node;
    rt->function_count++;
}

static ASTNode *find_function(const char *name)
{
    const Runtime *rt = get_runtime();
    for (int i = 0; i < rt->function_count; i++)
    {
        if (strcmp(rt->function_table[i].name, name) == 0)
            return rt->function_table[i].node;
    }
    return NULL;
}

// Function context stack management functions
void function_stack_init(void)
{
    Runtime *rt = get_runtime();
    rt->function_stack.count = 0;
    memset(rt->function_stack.contexts, 0, sizeof(rt->function_stack.contexts));
}

int function_stack_push(const char *function_name, ASTNode *function_node)
{
    Runtime *rt = get_runtime();
    
    if (rt->function_stack.count >= MAX_FUNCTION_STACK_DEPTH) {
        report_error("[Function Stack] Maximum function call depth exceeded (%d)", MAX_FUNCTION_STACK_DEPTH);
        return 0; // Failure
    }
    
    FunctionContext *ctx = &rt->function_stack.contexts[rt->function_stack.count];
    ctx->function_name = strdup(function_name);  // Make a copy of the name
    ctx->scope_level = rt->current_scope_level;
    ctx->function_node = function_node;
    
    rt->function_stack.count++;
    return 1; // Success
}

void function_stack_pop(void)
{
    Runtime *rt = get_runtime();
    
    if (rt->function_stack.count <= 0) {
        report_error("[Function Stack] Attempted to pop from empty function stack");
        return;
    }
    
    rt->function_stack.count--;
    FunctionContext *ctx = &rt->function_stack.contexts[rt->function_stack.count];
    
    // Free the duplicated function name
    if (ctx->function_name) {
        free(ctx->function_name);
        ctx->function_name = NULL;
    }
    ctx->function_node = NULL;
    ctx->scope_level = 0;
}

FunctionContext *function_stack_peek(void)
{
    Runtime *rt = get_runtime();
    
    if (rt->function_stack.count <= 0) {
        return NULL;
    }
    
    return &rt->function_stack.contexts[rt->function_stack.count - 1];
}

int is_inside_function(void)
{
    Runtime *rt = get_runtime();
    return rt->function_stack.count > 0;
}




#define FREED_MAGIC 0xDEAD1234

void free_value(Value *val) {
    if (!val) return;

    if (val->type == FREED_MAGIC) {
        report_error("[FreeValue] Double free detected for %p", (void*)val);
        return;
    }

    if (val->type == VAL_ARRAY) {
        for (size_t j = 0; j < val->array.count; j++) {
            free_value(val->array.items[j]);
            val->array.items[j] = NULL;
        }
        free(val->array.items);
        val->array.items = NULL;
    } else if (val->type == VAL_STRING) {
        if (val->string) {
            free(val->string);
            val->string = NULL;
        }
    }

    val->type = FREED_MAGIC;  // poison to catch reuse
    free(val);
}
























void exit_scope()
{
    Runtime *rt = get_runtime();


    for (int i = rt->var_count - 1; i >= 0; --i)
    {
        if (rt->variables[i].scope_level == rt->current_scope_level)
        {


            free(rt->variables[i].name);
            rt->variables[i].name = NULL;

            // Use free_value for all value types
            if (rt->variables[i].val)
            {
                free_value(rt->variables[i].val);
            }

            // Shift remaining variables down
            for (int j = i; j < rt->var_count - 1; ++j)
            {
                rt->variables[j] = rt->variables[j + 1];
            }

            rt->var_count--;
        }
    }

    rt->current_scope_level--;

}













void set_var(const char *name, Value *val)
{
    Runtime *rt = get_runtime();
    if (!val) {
        report_error("[set_var] Warning: null value for '%s'", name);
        return;
    }
    Value *copy = copy_value(val);
    if (!copy) {
        report_error("[set_var] Error: failed to copy value for '%s'", name);
        return;
    }
    for (int i = rt->var_count - 1; i >= 0; i--) {
        if (rt->variables[i].name && strcmp(rt->variables[i].name, name) == 0) {
            free_value(rt->variables[i].val);     // Free old value
            rt->variables[i].val = copy; // Assign new copy
            
            // Check for configuration variables
            check_config_variable(name, val);
            return;
        }
    }
    // Not found, declare new variable (copy already made)
    declare_var(name, copy);
}
