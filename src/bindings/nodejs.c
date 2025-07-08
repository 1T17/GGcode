#include <stdio.h>
#include <stdlib.h>
#include <string.h>




#include "../include/config.h"
#include "../parser/parser.h"
#include "../runtime/evaluator.h"
#include "../utils/output_buffer.h"
#include "../generator/emitter.h"
#include "../error/error.h"





extern int statement_count;

const char* compile_ggcode_from_string(const char* source_code, int debug) {


if (!source_code || source_code[0] == '\0') {
    return strdup("; EMPTY OR NULL INPUT\n");
}



          statement_count = 0;

    reset_runtime_state();
    init_output_buffer();

    statement_count = 0;

    ASTNode* root = parse_script_from_string(source_code);
    if (!root || has_errors()) {
        print_errors();
        clear_errors();
        return strdup("; ERROR\n");
    }

    emit_gcode(root, debug);

    // === Insert G-code header with % and ID ===
    char preamble[128] = "%\n";
    char id_line[64];

    if (var_exists("id")) {
        printf("[DEBUG] Variable 'id' exists\n");

        Value *id_val = get_var("id");
        if (id_val) {
            printf("[DEBUG] id_val is not NULL, type = %d\n", id_val->type);
        } else {
            printf("[DEBUG] id_val is NULL\n");
        }

        if (id_val && id_val->type == VAL_NUMBER) {
            printf("[DEBUG] id_val->number = %f\n", id_val->number);
            snprintf(id_line, sizeof(id_line), "%.0f", id_val->number);
        } else {
            printf("[DEBUG] id_val is not a number, using default '000'\n");
            snprintf(id_line, sizeof(id_line), "000");
        }
    } else {
        printf("[DEBUG] Variable 'id' does not exist, using default '000'\n");
        snprintf(id_line, sizeof(id_line), "000");
    }


    strcat(preamble, id_line);
    strcat(preamble, "\n");

    prepend_to_output_buffer(preamble);

    const char* output = strdup(get_output_buffer());

    free_ast(root);


    return output;
}

