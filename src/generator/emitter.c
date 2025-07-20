/// emitter.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "emitter.h"
#include "runtime/evaluator.h"

void free_value(Value *val); // Forward declaration
Value *copy_value(Value *val); // Forward declaration
#include "utils/output_buffer.h"
#include "config/config.h"
#include "error/error.h"
#define FATAL_ERROR(msg, ...) fatal_error(NULL, 0, 0, msg, ##__VA_ARGS__)



#define MAX_WHILE_ITERATIONS 1000000

Value *make_number_value(double num);

double get_number(Value *val);

#include <math.h>

Value *make_number_value(double num)
{
    Value *val = malloc(sizeof(Value));
    if (!val)
    {
        report_error("[make_number_value] malloc failed for Value");
        return NULL;
    }

    // Comprehensive floating-point precision handling
    // 1. Handle NaN and Infinity
    if (isnan(num) || isinf(num)) {
        num = 0.0;
    }
    // 2. Clamp extremely small values to 0 (prevents scientific notation)
    else if (fabs(num) < 1e-5) {
        num = 0.0;
    }
    // 3. Round very small values to prevent floating-point noise
    else if (fabs(num) < 1e-4) {
        num = round(num * 1e4) / 1e4;
    }
    // 4. Round medium values to prevent excessive precision
    else if (fabs(num) < 1e-2) {
        num = round(num * 1e6) / 1e6;
    }

    val->type = VAL_NUMBER;
    val->number = num;
    return val;
}

int get_statement_count()
{
    Runtime *rt = get_runtime();
    return rt->statement_count;
}

// Global flag to reset emitter state
static int emitter_reset_flag = 0;

// Reset emitter state between compilations
void reset_emitter_state()
{
    Runtime *rt = get_runtime();
    rt->statement_count = 0;
    emitter_reset_flag = 1;  // Set flag to reset last_code on next emit_gcode_stmt call
}



//
static void emit_note_stmt(ASTNode *node)
{
    Runtime *rt = get_runtime();
    rt->statement_count++;
    const char *content = node->note.content;
    if (!content)
    {
        report_error("[Emit] NOTE content is NULL");
        return;
    }



    char *copy = strdup(content);
    if (!copy)
    {
        report_error("[Emit] strdup failed for note content");
        return;
    }

    char *line = strtok(copy, "\n");
    while (line)
    {
        // Strip trailing \r if present (Windows CRLF line endings)
        size_t len = strlen(line);

        if (len > 0 && line[len - 1] == '\r')
        {
            line[len - 1] = ' ';
        }

        char parsed[256] = {0}, *out = parsed;
        const char *p = line;

        while (*p)
        {
            if (*p == '[')
            {
                char varname[64] = {0};
                p++;
                int vi = 0;
                while (*p && *p != ']' && vi < 63)
                    varname[vi++] = *p++;
                varname[vi] = '\0';
                if (*p == ']')
                    p++;

                Runtime *rt = get_runtime();
                const char *replacement = NULL;
                if (strcmp(varname, "time") == 0)
                    replacement = rt->RUNTIME_TIME;
                else if (strcmp(varname, "ggcode_file_name") == 0)
                    replacement = rt->RUNTIME_FILENAME;

                if (replacement)
                {
                    out += sprintf(out, "%s", replacement);
                }
                else
                {
                    Value *val = get_var(varname);
                    if (val && val->type == VAL_NUMBER)
                    {
                        out += sprintf(out, "%.6g", val->number);
                    }
                    else
                    {
                        out += sprintf(out, "0");
                    }
                }
            }
            else
            {
                *out++ = *p++;
            }
        }
        *out = '\0';

        char gcode_comment[300];
        snprintf(gcode_comment, sizeof(gcode_comment), "(%s)", parsed);
        write_to_output(gcode_comment);

        line = strtok(NULL, "\n");
    }

    free(copy);
    copy = NULL;
}
//
static void emit_let_stmt(ASTNode *node)
{
    Runtime *rt = get_runtime();
    rt->statement_count++;

    if (!node->let_stmt.name)
    {
        report_error("[Emit] LET statement missing name");
        return;
    }

    Value *val = eval(node->let_stmt.expr);


    if (!val)
    {
        report_error("[Emit] LET %s = NULL (defaulting to 0)", node->let_stmt.name);
        val = make_number_value(0);
    }

    set_var(node->let_stmt.name, val);
}
//
static void emit_gcode_stmt(ASTNode *node)
{
    Runtime *rt = get_runtime();
    rt->statement_count++;
    if (!node->gcode_stmt.code)
    {
        report_error("[Emit] GCODE missing command code");

        return;
    }



    static char last_code[16] = "";

    // Reset last_code when emitter_reset_flag is set
    if (emitter_reset_flag) {
        memset(last_code, 0, sizeof(last_code));
        emitter_reset_flag = 0;
    }

    char line[256] = {0};

    if (get_enable_n_lines())
    {
        snprintf(line, sizeof(line), "N%d ", get_line_number());
        increment_line_number();
    }

    if (strcmp(node->gcode_stmt.code, "G1") == 0)
    {
        if (strcmp(last_code, "G1") != 0)
        {
            size_t len = strlen(line);
            snprintf(line + len, sizeof(line) - len, "%s", "G1");
            strncpy(last_code, "G1", sizeof(last_code) - 1);
            last_code[sizeof(last_code) - 1] = '\0';
        }
    }
    else
    {
        size_t len = strlen(line);
        snprintf(line + len, sizeof(line) - len, "%s", node->gcode_stmt.code);
        strncpy(last_code, node->gcode_stmt.code, sizeof(last_code) - 1);
        last_code[sizeof(last_code) - 1] = '\0';
    }

    for (int i = 0; i < node->gcode_stmt.argCount; i++)
    {
        char segment[64];
        double val = 0.0;

        if (node->gcode_stmt.args[i].indexExpr)
        {
            Value *v = eval_expr(node->gcode_stmt.args[i].indexExpr);
            if (!v || v->type != VAL_NUMBER)
            {
                report_error("[Emit] GCODE arg[%d] is not a number", i);
                val = 0.0;
            }
            else
            {
                val = v->number;
            }
        }

        snprintf(segment, sizeof(segment), " %s%.6g", node->gcode_stmt.args[i].key, val);

        size_t len = strlen(line);
        strncat(line, segment, sizeof(line) - len - 1);
    }



    write_to_output(line);
}
//
static void emit_while_stmt(ASTNode *node)
{
    Runtime *rt = get_runtime();
    rt->statement_count++;



    int iteration = 0;
    while (1)
    {
        Value *cond = eval_expr(node->while_stmt.condition);
        if (!cond || cond->type != VAL_NUMBER)
        {
            report_error("[Emit] WHILE condition is invalid (null or not a number)");
            break;
        }

        if (cond->number == 0)
            break;

        if (iteration >= MAX_WHILE_ITERATIONS)
        {
            report_error("[Emit] WHILE loop exceeded maximum iterations (%d)", MAX_WHILE_ITERATIONS);
            break;
        }

   

        emit_gcode(node->while_stmt.body);
        iteration++;
    }

}
//
static void emit_for_stmt(ASTNode *node)
{
    Runtime *rt = get_runtime();
    rt->statement_count++;

    if (!node->for_stmt.var || !node->for_stmt.body)
    {

        report_error("[Emit] FOR loop missing variable or body");

        return;
    }

    Value *v_from = eval_expr(node->for_stmt.from);
    Value *v_to = eval_expr(node->for_stmt.to);
    Value *v_step = node->for_stmt.step ? eval_expr(node->for_stmt.step) : make_number_value(1.0);

    if (!v_from || !v_to || !v_step ||
        v_from->type != VAL_NUMBER || v_to->type != VAL_NUMBER || v_step->type != VAL_NUMBER)
    {

        report_error("[Emit] FOR loop expects numeric values");

        return;
    }

    double from = v_from->number;
    double to = v_to->number;
    double step = v_step->number;
    int exclusive = node->for_stmt.exclusive;



    if (step == 0)
    {

        report_error("[Emit] FOR loop step cannot be zero");

        return;
    }

    if (step > 0)
    {
        double end = exclusive ? to : to + 1e-9;
        for (double i = from; i < end; i += step)
        {
            set_var(node->for_stmt.var, make_number_value(i));
            emit_gcode(node->for_stmt.body);
        }
    }
    else
    {
        double end = exclusive ? to : to - 1e-9;
        for (double i = from; i > end; i += step)
        {
            set_var(node->for_stmt.var, make_number_value(i));
            emit_gcode(node->for_stmt.body);
        }
    }
}
//
void emit_block_stmt(ASTNode *node)
{


    for (int i = 0; i < node->block.count; i++)
    {
        if (!node->block.statements[i])
        {

            report_error("[Emit] NULL statement in block index %d", i);

            continue;
        }

        emit_gcode(node->block.statements[i]);
    }
}
//
static void emit_function_stmt(ASTNode *node)
{

    extern void register_function(ASTNode * node);
    register_function(node);
}
//
static void emit_if_stmt(ASTNode *node)
{
    Runtime *rt = get_runtime();
    rt->statement_count++;



    Value *cond_val = eval_expr(node->if_stmt.condition);
    if (!cond_val)
    {

        report_error("[Emit] IF condition evaluated to NULL");

        return; // Or exit(1) if this should be fatal
    }

    if (cond_val->type != VAL_NUMBER)
    {

        report_error("[Emit] IF condition did not return a number (type: %d)", cond_val->type);

        return;
    }

    double cond = cond_val->number;

    if (cond)
    {

        emit_gcode(node->if_stmt.then_branch);
    }
    else if (node->if_stmt.else_branch)
    {

        emit_gcode(node->if_stmt.else_branch);
    }
    else
    {

    }
}






void emit_gcode(ASTNode *node)
{
    if (!node)
        return;



    switch (node->type)
    {



case AST_RETURN:

    break;


case AST_EXPR_STMT:
{


    if (node->expr_stmt.expr)
    {
        emit_gcode(node->expr_stmt.expr);
        Runtime *rt = get_runtime();
        rt->statement_count++;
    }
    else
    {
        report_error("[Emit AST_EXPR_STMT] EXPR_STMT has NULL expr");
    }
    break;
}



    case AST_NOP:
        break;

    case AST_NOTE:
        emit_note_stmt(node);
        break;
    case AST_LET:
        emit_let_stmt(node);
        break;

    case AST_ASSIGN:
    {
        Runtime *rt = get_runtime();
        rt->statement_count++;
        Value *val = eval_expr(node->assign_stmt.expr);
        if (!val || val->type != VAL_NUMBER)
        {
            report_error("[Emit] ASSIGN %s failed, invalid expression", node->assign_stmt.name);
            val = make_number_value(0); // fallback
        }

        set_var(node->assign_stmt.name, val);


        break;
    }

    case AST_GCODE:
        emit_gcode_stmt(node);
        break;


    case AST_FOR:
        emit_for_stmt(node);
        break;
    case AST_WHILE:
        emit_while_stmt(node);
        break;
    case AST_BLOCK:
        emit_block_stmt(node);
        break;
    case AST_FUNCTION:
        emit_function_stmt(node);
        break;
    case AST_IF:
        emit_if_stmt(node);
        break;




case AST_CALL:
{


    // Execute function and ignore the return value
    eval_expr(node);



    Runtime *rt = get_runtime();
    rt->statement_count++;
    break;
}





    case AST_ASSIGN_INDEX:
    {
        Runtime *rt = get_runtime();
        rt->statement_count++;

        ASTNode *index_node = node->assign_index.target;

        if (!index_node || index_node->type != AST_INDEX)
        {
            report_error("[Emit] ASSIGN_INDEX: target is not an index expression");
            break;
        }

        Value *array = eval_expr(index_node->index_expr.array);
        Value *index_val = eval_expr(index_node->index_expr.index);
        Value *value = eval_expr(node->assign_index.value);

        if (!array || array->type != VAL_ARRAY)
        {
            report_error("[Emit] ASSIGN_INDEX: array is not valid");
            break;
        }

        if (!index_val || index_val->type != VAL_NUMBER)
        {
            report_error("[Emit] ASSIGN_INDEX: index is not a number");
            break;
        }

        if (!value)
        {
            report_error("[Emit] ASSIGN_INDEX: value is NULL");
            value = make_number_value(0); // fallback
        }

        int i = (int)(index_val->number);
        if (i < 0)
        {
            report_error("[Emit] ASSIGN_INDEX: index %d is negative", i);
            break;
        }

        if ((size_t)i >= array->array.count)
        {
            // Expand array
            size_t new_count = i + 1;
            Value **new_items = realloc(array->array.items, sizeof(Value *) * new_count);
            if (!new_items)
            {
                fprintf(stderr, "[ASSIGN_INDEX] realloc failed for array items\n");
                return;
            }

            // Initialize new elements
            for (size_t j = array->array.count; j < new_count; j++)
            {
                new_items[j] = make_number_value(0); // or NULL if you want sparse arrays
            }

            array->array.items = new_items;
            array->array.count = new_count;
        }

        if (array->array.items[i])
        {
            free_value(array->array.items[i]);
        }
        array->array.items[i] = copy_value(value);



        break;
    }

    case AST_ARRAY_LITERAL:
    {
        Runtime *rt = get_runtime();
        rt->statement_count++;

        break;
    }

    default:

        report_error("[Emit] Unrecognized node type: %d", node->type);

        break;
    }
}