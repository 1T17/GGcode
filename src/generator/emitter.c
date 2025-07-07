///emitter.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "emitter.h"
#include "runtime/evaluator.h"
#include "utils/output_buffer.h"
#include "config.h"
#include "error/error.h"

// Forward declarations of utility functions
extern char RUNTIME_TIME[64];
// Forward declaration of the runtime filename
extern char RUNTIME_FILENAME[256];

#define MAX_WHILE_ITERATIONS 1000000  // adjust this as needed

// Value constructors
Value *make_number_value(double num);

double get_number(Value *val); 

Value *make_number_value(double num) {
    Value *val = malloc(sizeof(Value));
    if (!val) {

        report_error("[make_number_value] Out of memory");

        exit(1);
    }
    val->type = VAL_NUMBER;
    val->number = num;
    return val;
}

int statement_count = 0;



int get_statement_count()
{
    return statement_count;
}



static void emit_note_stmt(ASTNode *node, int debug) {
    statement_count++;
    const char *content = node->note.content;
    if (!content) {

        report_error("[Emit] NOTE content is NULL");

        return;
    }

    if (debug) {
        printf("[Emit] NOTE (raw content): %s\n", content);
        fflush(stdout);
    }

    char *copy = strdup(content);
    if (!copy) {
        report_error("[Emit] strdup failed for note content");

        return;
    }

    char *line = strtok(copy, "\n");
    while (line) {
        char parsed[256] = {0}, *out = parsed;
        const char *p = line;

        while (*p) {
            if (*p == '[') {
                char varname[64] = {0};
                p++; int vi = 0;
                while (*p && *p != ']' && vi < 63) varname[vi++] = *p++;
                varname[vi] = '\0';
                if (*p == ']') p++;

                const char *replacement = NULL;
                if (strcmp(varname, "time") == 0)
                    replacement = RUNTIME_TIME;
                else if (strcmp(varname, "ggcode_file_name") == 0)
                    replacement = RUNTIME_FILENAME;

                if (replacement) {
                    out += sprintf(out, "%s", replacement);
                } else {
Value *val = get_var(varname);
if (val && val->type == VAL_NUMBER) {
    out += sprintf(out, "%.6g", val->number);
} else {
    out += sprintf(out, "0");  // fallback for missing or non-number
}
                }
            } else {
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
}

static void emit_let_stmt(ASTNode *node, int debug) {
    statement_count++;

    if (!node->let_stmt.name) {
        report_error("[Emit] LET statement missing name");
        return;
    }

    Value *val = eval(node->let_stmt.expr);
    if (debug) {
        if (val) {
            if (val->type == VAL_NUMBER) {
                printf("[Emit] LET %s = %.5f\n", node->let_stmt.name, val->number);
            } else if (val->type == VAL_ARRAY) {
                printf("[Emit] LET %s = [array with %zu items]\n", node->let_stmt.name, val->array.count);
            }
        } else {
            printf("[Emit] LET %s = NULL\n", node->let_stmt.name);
        }
        fflush(stdout);
    }

    if (!val) {
        report_error("[Emit] LET %s = NULL (defaulting to 0)", node->let_stmt.name);
        val = make_number_value(0);
    }

    set_var(node->let_stmt.name, val);
}

static void emit_gcode_stmt(ASTNode *node, int debug) {
        statement_count++;
        if (!node->gcode_stmt.code)
        {
            report_error("[Emit] GCODE missing command code");

            return;
        }

        if (debug)
        {
            printf("[Emit] GCODE %s\n", node->gcode_stmt.code);
            fflush(stdout);
        }

        static char last_code[16] = "";
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
                strcat(line, "G1");
                strcpy(last_code, "G1");
            }
        }
        else
        {
            strcat(line, node->gcode_stmt.code);
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
if (!v || v->type != VAL_NUMBER) {
   report_error("[Emit] GCODE arg[%d] is not a number", i);

    val = 0.0;
} else {
    val = v->number;
}

            }

            snprintf(segment, sizeof(segment), " %s%.6g", node->gcode_stmt.args[i].key, val);
            strcat(line, segment);
        }

        if (debug)
        {
            printf("[Line] Final G-code: %s\n", line);
            fflush(stdout);
        }

        write_to_output(line);
}

static void emit_while_stmt(ASTNode *node, int debug) {
    statement_count++;

    if (debug) {
        printf("[Emit] WHILE loop starting\n");
        fflush(stdout);
    }

    int iteration = 0;
    while (1) {
        Value *cond = eval_expr(node->while_stmt.condition);
        if (!cond || cond->type != VAL_NUMBER) {
            report_error("[Emit] WHILE condition is invalid (null or not a number)");
            break;
        }

        if (cond->number == 0) break;

        if (iteration >= MAX_WHILE_ITERATIONS) {
            report_error("[Emit] WHILE loop exceeded maximum iterations (%d)", MAX_WHILE_ITERATIONS);
            break;
        }

        if (debug) {
            printf("[Emit] WHILE iteration %d\n", iteration);
            fflush(stdout);
        }

        emit_gcode(node->while_stmt.body, debug);
        iteration++;
    }

    if (debug) {
        printf("[Emit] WHILE loop completed after %d iteration(s)\n", iteration);
        fflush(stdout);
    }
}

/// Emits a FOR loop from `from` to `to`, assigning the loop variable and emitting the body.
/// @param node FOR AST node.
static void emit_for_stmt(ASTNode *node, int debug) {
    statement_count++;

    if (!node->for_stmt.var || !node->for_stmt.body) {

        report_error("[Emit] FOR loop missing variable or body");

        return;
    }

    Value *v_from = eval_expr(node->for_stmt.from);
    Value *v_to = eval_expr(node->for_stmt.to);
    Value *v_step = node->for_stmt.step ? eval_expr(node->for_stmt.step) : make_number_value(1.0);

    if (!v_from || !v_to || !v_step ||
        v_from->type != VAL_NUMBER || v_to->type != VAL_NUMBER || v_step->type != VAL_NUMBER) {

       report_error("[Emit] FOR loop expects numeric values");

        return;
    }

    double from = v_from->number;
    double to = v_to->number;
    double step = v_step->number;
    int exclusive = node->for_stmt.exclusive;

    if (debug) {
        printf("[Emit] FOR %s = %.2f %s %.2f step %.2f\n",
               node->for_stmt.var,
               from,
               exclusive ? "..<" : "..",
               to,
               step);
    }

    if (step == 0) {

       report_error("[Emit] FOR loop step cannot be zero");

        return;
    }

    if (step > 0) {
        double end = exclusive ? to : to + 1e-9;
        for (double i = from; i < end; i += step) {
            set_var(node->for_stmt.var, make_number_value(i));
            emit_gcode(node->for_stmt.body, debug);
        }
    } else {
        double end = exclusive ? to : to - 1e-9;
        for (double i = from; i > end; i += step) {
            set_var(node->for_stmt.var, make_number_value(i));
            emit_gcode(node->for_stmt.body, debug);
        }
    }
}

/// Emits a block of sequential AST statements.
/// @param node BLOCK AST node.
static void emit_block_stmt(ASTNode *node, int debug) {
    if (debug) {
        printf("[Emit] BLOCK with %d statements\n", node->block.count);
        fflush(stdout);
    }

    for (int i = 0; i < node->block.count; i++) {
        if (!node->block.statements[i]) {

           report_error("[Emit] NULL statement in block index %d", i);

            continue;
        }

        emit_gcode(node->block.statements[i], debug);
    }
}

/// Emits a FUNCTION declaration (not executed, only registered/logged).
/// @param node FUNCTION AST node.
static void emit_function_stmt(ASTNode *node, int debug) {
    statement_count++;
    extern void register_function(ASTNode *node);
    register_function(node);

    if (debug) {
        printf("[Emit] FUNCTION declared: %s with %d params\n",
               node->function_stmt.name,
               node->function_stmt.param_count);
        fflush(stdout);
    }
}


/// Emits a function call expression by evaluating it.
/// @param node CALL AST node.
/// @param debug Enables debug output.
// static void emit_call_stmt(ASTNode *node, int debug) {
//     statement_count++;

//     // Evaluate the function call for its return value or side effects
//     eval_expr(node);

//     if (debug) {
//         printf("[Emit] Function call: %s()\n", node->call_expr.name);
//         fflush(stdout);
//     }
// }

/// Emits an IF/ELSE conditional branch depending on the evaluated condition.
/// @param node IF AST node.
static void emit_if_stmt(ASTNode *node, int debug) {
    statement_count++;

    if (debug) {
        printf("[Emit] IF condition...\n");
        fflush(stdout);
    }

    Value *cond_val = eval_expr(node->if_stmt.condition);
    if (!cond_val) {

        report_error("[Emit] IF condition evaluated to NULL");

        return; // Or exit(1) if this should be fatal
    }

    if (cond_val->type != VAL_NUMBER) {

        report_error("[Emit] IF condition did not return a number (type: %d)", cond_val->type);

        return;
    }

    double cond = cond_val->number;

    if (cond) {
        if (debug) {
            printf("[Emit] IF condition TRUE, executing THEN branch\n");
            fflush(stdout);
        }
        emit_gcode(node->if_stmt.then_branch, debug);
    } else if (node->if_stmt.else_branch) {
        if (debug) {
            printf("[Emit] IF condition FALSE, executing ELSE branch\n");
            fflush(stdout);
        }
        emit_gcode(node->if_stmt.else_branch, debug);
    } else {
        if (debug) {
            printf("[Emit] IF condition FALSE, no ELSE branch\n");
            fflush(stdout);
        }
    }
}

/// Dispatches AST nodes to their respective emit_* handlers.
/// Logs errors for unknown or unsupported types.
/// @param node Any AST node.
void emit_gcode(ASTNode *node, int debug) {

    if (!node) return;
    switch (node->type) {


case AST_NOP:
    // Do nothing, this is a placeholder node
    break;



        case AST_NOTE:     emit_note_stmt(node, debug); break;
        case AST_LET:      emit_let_stmt(node, debug); break;






case AST_ASSIGN: {
    statement_count++;
    Value *val = eval_expr(node->assign_stmt.expr);
    if (!val || val->type != VAL_NUMBER) {
        report_error("[Emit] ASSIGN %s failed, invalid expression", node->assign_stmt.name);
        val = make_number_value(0);  // fallback
    }

    set_var(node->assign_stmt.name, val);

    if (debug) {
        printf("[Emit] ASSIGN %s = %.6f\n", node->assign_stmt.name, val->number);
        fflush(stdout);
    }
    break;
}




        case AST_GCODE:    emit_gcode_stmt(node, debug); break;
      


// case AST_WHILE:
//     printf("[Emit] WHILE node executing...\n");
//     emit_while_stmt(node, debug);  // or just: eval_while(node); if you merged logic
//     break;

        case AST_FOR:      emit_for_stmt(node, debug); break;
              case AST_WHILE:    emit_while_stmt(node, debug); break;
        case AST_BLOCK:    emit_block_stmt(node, debug); break;
        case AST_FUNCTION: emit_function_stmt(node, debug); break;
        case AST_IF:       emit_if_stmt(node, debug); break;
        case AST_CALL:     eval_expr(node); break; // <-- Add this line




case AST_ASSIGN_INDEX: {
    statement_count++;

    ASTNode *index_node = node->assign_index.target;

    if (!index_node || index_node->type != AST_INDEX) {
        report_error("[Emit] ASSIGN_INDEX: target is not an index expression");
        break;
    }

    Value *array = eval_expr(index_node->index_expr.array);
    Value *index_val = eval_expr(index_node->index_expr.index);
    Value *value = eval_expr(node->assign_index.value);

    if (!array || array->type != VAL_ARRAY) {
        report_error("[Emit] ASSIGN_INDEX: array is not valid");
        break;
    }

    if (!index_val || index_val->type != VAL_NUMBER) {
        report_error("[Emit] ASSIGN_INDEX: index is not a number");
        break;
    }

    if (!value) {
        report_error("[Emit] ASSIGN_INDEX: value is NULL");
        value = make_number_value(0);  // fallback
    }

    int i = (int)(index_val->number);
    if (i < 0) {
        report_error("[Emit] ASSIGN_INDEX: index %d is negative", i);
        break;
    }

    if ((size_t)i >= array->array.count) {
        // Expand array
        size_t new_count = i + 1;
        Value **new_items = realloc(array->array.items, sizeof(Value *) * new_count);
        if (!new_items) {
            report_error("[Emit] ASSIGN_INDEX: realloc failed");
            break;
        }

        // Initialize new elements
        for (size_t j = array->array.count; j < new_count; j++) {
            new_items[j] = make_number_value(0); // or NULL if you want sparse arrays
        }

        array->array.items = new_items;
        array->array.count = new_count;
    }

if (array->array.items[i]) {
    free(array->array.items[i]);
}
array->array.items[i] = value;


    if (debug) {
        printf("[Emit] ASSIGN_INDEX [%d] = %.6f\n", i, value->type == VAL_NUMBER ? value->number : 0);
        fflush(stdout);
    }

    break;
}

        

        case AST_ARRAY_LITERAL:
           statement_count++;
            if (debug) {
                printf("[Emit] ARRAY_LITERAL with %d elements\n", node->array_literal.count);
                for (int i = 0; i < node->array_literal.count; ++i) {
                    printf(" - Element [%d]:\n", i);
                    emit_gcode(node->array_literal.elements[i], debug);
                }
                
            }
            break;


        default:

            report_error("[Emit] Unrecognized node type: %d", node->type);

            break;
    }


    
}