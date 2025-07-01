/// gcode_emitter.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gcode_emitter.h"
#include "runtime/evaluator.h"
#include "utils/output_buffer.h"
#include "config.h"

extern char RUNTIME_TIME[64];
extern char RUNTIME_FILENAME[256];

int statement_count = 0;

/// Returns the number of statements emitted so far.
/// @return Total emitted AST statements.
int get_statement_count()
{
    return statement_count;
}

/// Emits a NOTE comment block as G-code comments.
/// Replaces [vars] like [time], [ggcode_file_name], and user variables.
/// @param node NOTE AST node.
/// @param debug Enables debug output.
static void emit_note_stmt(ASTNode *node, int debug) {
    statement_count++;
    const char *content = node->note.content;
    if (!content) {
        fprintf(stderr, "[Emit] WARNING: NOTE content is NULL\n");
        return;
    }

    if (debug) {
        printf("[Emit] NOTE (raw content): %s\n", content);
        fflush(stdout);
    }

    char *copy = strdup(content);
    if (!copy) {
        fprintf(stderr, "[Emit] ERROR: strdup failed for note content\n");
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
                    double val = get_var(varname);
                    out += sprintf(out, "%.6g", val);
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

/// Emits a LET assignment, setting a variable from an expression.
/// @param node LET AST node.
/// @param debug Enables debug output.
static void emit_let_stmt(ASTNode *node, int debug) {
    statement_count++;

    if (!node->let_stmt.name) {
        fprintf(stderr, "[Emit] ERROR: LET statement missing name\n");
        return;
    }

    double val = eval_expr(node->let_stmt.expr);
    if (debug) {
        printf("[Emit] LET %s = %.5f\n", node->let_stmt.name, val);
        fflush(stdout);
    }

    set_var(node->let_stmt.name, val);
}

/// Emits a raw G-code instruction, including optional axes/params.
/// Adds line numbers and compresses repeated G1 codes.
/// @param node GCODE AST node.
/// @param debug Enables debug output.
static void emit_gcode_stmt(ASTNode *node, int debug) {
        statement_count++;
        if (!node->gcode_stmt.code)
        {
            fprintf(stderr, "[Emit] ERROR: GCODE missing command code\n");
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
                val = eval_expr(node->gcode_stmt.args[i].indexExpr);
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

/// Emits a WHILE loop, repeatedly emitting the body while the condition is true.
/// @param node WHILE AST node.
/// @param debug Enables debug output.
static void emit_while_stmt(ASTNode *node, int debug) {
    statement_count++;

    if (debug) {
        printf("[Emit] WHILE loop starting\n");
        fflush(stdout);
    }

    while (eval_expr(node->while_stmt.condition)) {
        emit_gcode(node->while_stmt.body, debug);
    }
}

/// Emits a FOR loop from `from` to `to`, assigning the loop variable and emitting the body.
/// @param node FOR AST node.
/// @param debug Enables debug output.
static void emit_for_stmt(ASTNode *node, int debug) {
    statement_count++;

    if (!node->for_stmt.var || !node->for_stmt.body) {
        fprintf(stderr, "[Emit] ERROR: FOR loop missing variable or body\n");
        return;
    }

    if (debug) {
        printf("[Emit] FOR %s = %.2f .. %.2f\n", node->for_stmt.var, node->for_stmt.from, node->for_stmt.to);
        fflush(stdout);
    }

    for (int i = node->for_stmt.from; i <= node->for_stmt.to; i++) {
        set_var(node->for_stmt.var, i);
        emit_gcode(node->for_stmt.body, debug);
    }
}

/// Emits a block of sequential AST statements.
/// @param node BLOCK AST node.
/// @param debug Enables debug output.
static void emit_block_stmt(ASTNode *node, int debug) {
    if (debug) {
        printf("[Emit] BLOCK with %d statements\n", node->block.count);
        fflush(stdout);
    }

    for (int i = 0; i < node->block.count; i++) {
        if (!node->block.statements[i]) {
            fprintf(stderr, "[Emit] WARNING: NULL statement in block index %d\n", i);
            continue;
        }

        emit_gcode(node->block.statements[i], debug);
    }
}

/// Emits a FUNCTION declaration (not executed, only registered/logged).
/// @param node FUNCTION AST node.
/// @param debug Enables debug output.
static void emit_function_stmt(ASTNode *node, int debug) {
    statement_count++;

    if (debug) {
        printf("[Emit] FUNCTION declared: %s with %d params\n",
               node->function_stmt.name,
               node->function_stmt.param_count);
        fflush(stdout);
    }

    // Optional: You can store the function in a function table here if implementing function calls.
}

/// Emits a function call expression by evaluating it.
/// @param node CALL AST node.
/// @param debug Enables debug output.
static void emit_call_stmt(ASTNode *node, int debug) {
    statement_count++;

    // Evaluate the function call for its return value or side effects
    eval_expr(node);

    if (debug) {
        printf("[Emit] Function call: %s()\n", node->call_expr.name);
        fflush(stdout);
    }
}

/// Emits an IF/ELSE conditional branch depending on the evaluated condition.
/// @param node IF AST node.
/// @param debug Enables debug output.
static void emit_if_stmt(ASTNode *node, int debug) {
    statement_count++;

    if (debug) {
        printf("[Emit] IF condition...\n");
        fflush(stdout);
    }

    if (eval_expr(node->if_stmt.condition)) {
        if (debug) {
            printf("[Emit] IF condition TRUE, executing THEN branch\n");
        }
        emit_gcode(node->if_stmt.then_branch, debug);
    } else if (node->if_stmt.else_branch) {
        if (debug) {
            printf("[Emit] IF condition FALSE, executing ELSE branch\n");
        }
        emit_gcode(node->if_stmt.else_branch, debug);
    }
}

/// Dispatches AST nodes to their respective emit_* handlers.
/// Logs errors for unknown or unsupported types.
/// @param node Any AST node.
/// @param debug Enables debug output.
void emit_gcode(ASTNode *node, int debug) {
    if (!node) {
        fprintf(stderr, "[Emit] ERROR: NULL node passed to emit_gcode\n");
        return;
    }

    if (node->type < 0 || node->type >= AST_NODE_TYPE_COUNT) {
        fprintf(stderr, "[Emit] ERROR: Invalid AST node type: %d\n", node->type);
        return;
    }

    switch (node->type) {
        case AST_NOTE:     emit_note_stmt(node, debug); break;
        case AST_LET:      emit_let_stmt(node, debug); break;
        case AST_GCODE:    emit_gcode_stmt(node, debug); break;
        case AST_WHILE:    emit_while_stmt(node, debug); break;
        case AST_FOR:      emit_for_stmt(node, debug); break;
        case AST_BLOCK:    emit_block_stmt(node, debug); break;
        case AST_FUNCTION: emit_function_stmt(node, debug); break;
        case AST_CALL:     emit_call_stmt(node, debug); break;
        case AST_IF:       emit_if_stmt(node, debug); break;
        default:
            fprintf(stderr, "[Emit] WARNING: Unrecognized node type: %d\n", node->type);
            break;
    }
}