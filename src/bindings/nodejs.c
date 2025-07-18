#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "../config/config.h"
#include "../parser/parser.h"
#include "../runtime/evaluator.h"
#include "../utils/output_buffer.h"
#include "../generator/emitter.h"
#include "../error/error.h"
#include <time.h>


extern int statement_count;


// In your function later:


const char* compile_ggcode_from_string(const char* source_code) {

    int debug = get_debug();

    // Initialize runtime state
    init_runtime();
    Runtime* runtime = get_runtime();
    runtime->debug = debug;

if (!source_code || source_code[0] == '\0') {
    return strdup("; EMPTY OR NULL INPUT\n");
}

    statement_count = 0;

    // reset_runtime_state();
    init_output_buffer();


    
    ASTNode* root = parse_script_from_string(source_code);


    emit_gcode(root, -1);  // Use runtime state for debug

    // === Insert G-code header with % and ID ===

    char ggcode_file_name[64];
    snprintf(ggcode_file_name, sizeof(ggcode_file_name), "nodejs.ggcode");

    emit_gcode_preamble(debug, ggcode_file_name);





// const char* buffer = get_output_buffer();


// printf("DEBUG RAW OUTPUT: '%s'\n", buffer);  // See what's really there



    const char* output = strdup(get_output_buffer());



    if (!root || has_errors()) {
        report_error("[NodeJS] Compilation failed or errors detected");
        print_errors();
        clear_errors();
        return strdup("; ERROR\n");
    }



   reset_runtime_state(); 


    free_ast(root);




    free_output_buffer();


    return output;
}

