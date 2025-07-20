#include "evaluator.h"
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
// Parser moved to runtime state - no more global parser

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
static int runtime_has_returned = 0;

double get_number(Value *val)
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
    Value *v = eval_expr(node);
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
    Runtime *rt = get_runtime();
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

Value *get_array_item(Value *arr, int index)
{
    if (!arr || arr->type != VAL_ARRAY)
    {
        report_error("[get_array_item] Not an array");
        return NULL;
    }
    if (index < 0 || index >= (int)arr->array.count)
    {
        report_error("[get_array_item] Index %d out of bounds (len=%zu)", index, arr->array.count);
        return NULL;
    }

    return arr->array.items[index];
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
        case AST_UNARY:
            set_parents_recursive(node->unary_expr.operand, node);
            break;
        case AST_BINARY:
            set_parents_recursive(node->binary_expr.left, node);
            set_parents_recursive(node->binary_expr.right, node);
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
            set_parents_recursive(node->for_stmt.from, node);
            set_parents_recursive(node->for_stmt.to, node);
            if (node->for_stmt.step)
                set_parents_recursive(node->for_stmt.step, node);
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
        double left = get_number(eval_expr(node->binary_expr.left));
        double right = get_number(eval_expr(node->binary_expr.right));
        Token_Type op = node->binary_expr.op;

        switch (op)
        {
        case TOKEN_PLUS:
            return make_number_value(left + right);
        case TOKEN_MINUS:
            return make_number_value(left - right);
        case TOKEN_STAR:
            return make_number_value(left * right);
        case TOKEN_SLASH:
            if (right == 0)
            {
                report_error("[Runtime evaluator] Division by zero (%.5f / %.5f)", left, right);
                return make_number_value(0.0);
            }

            return make_number_value(left / right);
        case TOKEN_LESS:
            return make_number_value(left < right ? 1.0 : 0.0);
        case TOKEN_LESS_EQUAL:
            return make_number_value(left <= right ? 1.0 : 0.0);
        case TOKEN_GREATER:
            return make_number_value(left > right ? 1.0 : 0.0);
        case TOKEN_GREATER_EQUAL:
            return make_number_value(left >= right ? 1.0 : 0.0);

        case TOKEN_EQUAL_EQUAL:
        {
            double result = left == right ? 1.0 : 0.0;
            //printf("[Eval evaluator] %.2f == %.2f => %.2f\n", left, right, result);
            return make_number_value(result);
        }

        case TOKEN_BANG_EQUAL:
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
        double cond = get_number(eval_expr(node->if_stmt.condition));
        if (cond)
        {
            return eval_expr(node->if_stmt.then_branch);
        }
        else if (node->if_stmt.else_branch)
        {
            return eval_expr(node->if_stmt.else_branch);
        }
        return make_number_value(0.0);
    }

case AST_RETURN:
{
runtime_has_returned = 1;
Value *ret = eval_expr(node->return_stmt.expr);
runtime_return_value = copy_value(ret);   // <- COPY IMMEDIATELY
return ret;
}


    case AST_FUNCTION:
        register_function(node);
        return make_number_value(0.0);

    case AST_ASSIGN:
        set_var(node->assign_stmt.name, eval_expr(node->assign_stmt.expr));
        return make_number_value(0.0);



case AST_INDEX:
{
    Value *target = eval_expr(node->index_expr.array);
    Value *index_val = eval_expr(node->index_expr.index);

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
        eval_expr(node->while_stmt.body);

        if (runtime_has_returned) {
            break;
        }
    }
    return make_number_value(0.0);
}





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
        return make_number_value(fmod(SCALAR(0), SCALAR(1)));
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
        return make_number_value(pow(SCALAR(0), SCALAR(1)));
    if (strcmp(name, "hypot") == 0 && argc == 2)
        return make_number_value(hypot(SCALAR(0), SCALAR(1)));
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
        return make_number_value(hypot(x2 - x1, y2 - y1));
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
        return make_number_value(log(SCALAR(0)));
    }

    if (strcmp(name, "exp") == 0 && argc == 1)
        return make_number_value(exp(SCALAR(0)));
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






    // ✅ Clear return state before executing function
    runtime_has_returned = 0;
    runtime_return_value = NULL;

    enter_scope();

    int param_count = func->function_stmt.param_count;
    for (int i = 0; i < param_count; i++)
    {
        Value *arg_val = make_number_value(0.0);
if (i < argc)
    arg_val = eval_expr(args[i]);

if (!arg_val || arg_val->type != VAL_NUMBER) {
    fprintf(stderr, "🚨 ERROR: Failed to evaluate argument %d for function '%s'\n", i, name);
    FATAL_ERROR("🚨 ERROR: Failed to evaluate argument %d for function '%s'", i, name);
}



        declare_var(func->function_stmt.params[i], arg_val);
    }

ASTNode *body = func->function_stmt.body;

// ⚠️ Fix: Use emit_gcode instead of eval_expr for statements
for (int i = 0; i < body->block.count; i++) {
    ASTNode *stmt = body->block.statements[i];

    // ✅ Always evaluate control flow and expressions
    if (stmt->type == AST_GCODE || stmt->type == AST_NOTE || stmt->type == AST_WHILE || stmt->type == AST_FOR ) {
        emit_gcode(stmt);
    } else {
        eval_expr(stmt);
    }

    if (runtime_has_returned)
        break;
}







// for (int i = 0; i < body->block.count; i++) {
//     ASTNode *stmt = body->block.statements[i];

//     // Evaluate expressions (e.g., return), emit G-code lines
//     if (stmt->type == AST_GCODE || stmt->type == AST_NOTE || stmt->type == AST_EXPR_STMT || stmt->type == AST_IF || stmt->type == AST_FOR || stmt->type == AST_WHILE)
//         emit_gcode(stmt, 0);
//     else
//         eval_expr(stmt);

//     if (runtime_has_returned)
//         break;
// }


















    exit_scope();

    // ✅ Return the result if set, or 0 otherwise
    if (runtime_return_value)
        return copy_value(runtime_return_value);
    else
        return make_number_value(0.0);

#undef SCALAR
}

void declare_array(const char *name, Value **items, size_t count)
{
    Runtime *rt = get_runtime();
    if (rt->var_count >= MAX_VARIABLES)
    {
        fprintf(stderr, "[declare_array evaluator] ERROR: variable limit reached.\n");
        FATAL_ERROR("[declare_array evaluator] ERROR: variable limit reached.");
    }

    Value *arr_val = malloc(sizeof(Value));
    if (!arr_val) {
        report_error("[declare_array] malloc failed for arr_val");
        return;
    }
    arr_val->type = VAL_ARRAY;
    arr_val->array.items = items;
    arr_val->array.count = count;

    rt->variables[rt->var_count].name = strdup(name);
    rt->variables[rt->var_count].val = arr_val;
    rt->variables[rt->var_count].scope_level = rt->current_scope_level;
    rt->var_count++;
}

int var_exists_in_current_scope(const char *name)
{
    Runtime *rt = get_runtime();
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

            Runtime *rt = get_runtime();
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

        case AST_IF:
            eval_if(stmt);
            break;

        case AST_FOR:
            eval_for(stmt);
            break;

        case AST_WHILE:
            printf("[Runtime evaluator] WHILE encountered\n");
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
    Value *cond = eval_expr(stmt->if_stmt.condition);
    if (cond && cond->type == VAL_NUMBER)
    {
        printf("[Eval_IF evaluator] condition evaluates to: %.2f\n", cond->number);
    }
    else
    {
        printf("[Eval_IF evaluator] condition is NULL or non-number\n");
    }

    if (cond && cond->type == VAL_NUMBER && cond->number != 0)
    {
        eval_block(stmt->if_stmt.then_branch);
    }
    else if (stmt->if_stmt.else_branch)
    {
        eval_block(stmt->if_stmt.else_branch);
    }
}

void eval_for(ASTNode *stmt)
{
    Value *v_from = eval_expr(stmt->for_stmt.from);
    Value *v_to = eval_expr(stmt->for_stmt.to);
    Value *v_step = stmt->for_stmt.step ? eval_expr(stmt->for_stmt.step) : make_number_value(1.0);

    if (!v_from || !v_to || !v_step ||
        v_from->type != VAL_NUMBER || v_to->type != VAL_NUMBER || v_step->type != VAL_NUMBER)
    {
        printf("[Runtime evaluator] ERROR: FOR loop expects numeric values\n");
        return;
    }

    double from = v_from->number;
    double to = v_to->number;
    double step = v_step->number;
    int exclusive = stmt->for_stmt.exclusive;

    double end = exclusive ? to : (step > 0 ? to + 1e-9 : to - 1e-9);

    enter_scope();
    for (double i = from;
         (step > 0 && i < end) || (step < 0 && i > end);
         i += step)
    {
        set_var(stmt->for_stmt.var, make_number_value(i));
        eval_block(stmt->for_stmt.body);
    }
    exit_scope();
}





































void eval_while(ASTNode *stmt)
{
    enter_scope();
    int iter = 0;

    while (1)
    {
        Value *cond = eval_expr(stmt->while_stmt.condition);

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

        printf("[EVAL evaluator] WHILE ITERATION %d, cond = %.2f\n", iter, cond->number);
        fflush(stdout);

        if (cond->number == 0)
        {
            break;
        }

        eval_block(stmt->while_stmt.body);
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
}




// Check if variable exists
int var_exists(const char *name)
{
    Runtime *rt = get_runtime();
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
    Runtime *rt = get_runtime();
    for (int i = 0; i < rt->function_count; i++)
    {
        if (strcmp(rt->function_table[i].name, name) == 0)
            return rt->function_table[i].node;
    }
    return NULL;
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
    }

    val->type = FREED_MAGIC;  // poison to catch reuse
    free(val);
    val = NULL;
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
            return;
        }
    }
    // Not found, declare new variable (copy already made)
    declare_var(name, copy);
}
