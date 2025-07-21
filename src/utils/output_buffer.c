#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "output_buffer.h"

#include <time.h>
#include "config/config.h"
#include "runtime/evaluator.h"


// Internal buffer
static char* gcode_output = NULL;
static size_t gcode_output_capacity = 0;
static size_t gcode_output_length = 0;

void init_output_buffer() {
    gcode_output_capacity = 8192;
    gcode_output = calloc(gcode_output_capacity, 1);
    gcode_output_length = 0;
}

void free_output_buffer() {
    free(gcode_output);
    gcode_output = NULL;
    gcode_output_capacity = 0;
    gcode_output_length = 0;
}

void write_to_output(const char* line) {
    size_t len = strlen(line);
    if (gcode_output_length + len + 2 > gcode_output_capacity) {
        gcode_output_capacity = (gcode_output_capacity + len + 256) * 2;
        char *tmp = realloc(gcode_output, gcode_output_capacity);
        if (!tmp) { /* handle error */ } else { gcode_output = tmp; }
    }
    memcpy(gcode_output + gcode_output_length, line, len);
    gcode_output_length += len;
    gcode_output[gcode_output_length++] = '\n';
    gcode_output[gcode_output_length] = '\0';
}

const char* get_output_buffer() {
    return gcode_output;
}

size_t get_output_length() {
    return gcode_output_length;
}


void prepend_to_output_buffer(const char* prefix) {
    size_t prefix_len = strlen(prefix);

    if (prefix_len + gcode_output_length >= gcode_output_capacity) {
        gcode_output_capacity = (gcode_output_capacity + prefix_len + 256) * 2;
        char *tmp = realloc(gcode_output, gcode_output_capacity);
        if (!tmp) { /* handle error */ } else { gcode_output = tmp; }
    }

    // Shift current contents to the right
    memmove(gcode_output + prefix_len, gcode_output, gcode_output_length + 1);
    memcpy(gcode_output, prefix, prefix_len);
    gcode_output_length += prefix_len;
}


void emit_gcode_preamble(const char* default_filename) {
    char id_line[64];
    if (var_exists("id")) {
        const Value *id_val = get_var("id");
        if (id_val && id_val->type == VAL_NUMBER) {
            snprintf(id_line, sizeof(id_line), "%.0f", id_val->number);
        } else {
            snprintf(id_line, sizeof(id_line), "000");
        }
    } else {
        snprintf(id_line, sizeof(id_line), "000");
    }

    // Set RUNTIME_TIME
    char time_line[64];
    time_t now = time(NULL);
    const struct tm *t = localtime(&now);
    strftime(time_line, sizeof(time_line), "%Y-%m-%d %H:%M:%S", t);
    strncpy(RUNTIME_TIME, time_line, sizeof(RUNTIME_TIME) - 1);
    RUNTIME_TIME[sizeof(RUNTIME_TIME) - 1] = '\0';

    // Set RUNTIME_FILENAME
    if (default_filename) {
        strncpy(RUNTIME_FILENAME, default_filename, sizeof(RUNTIME_FILENAME) - 1);
        RUNTIME_FILENAME[sizeof(RUNTIME_FILENAME) - 1] = '\0';
    }

    // Also update runtime state variables for note block parsing
    Runtime *rt = get_runtime();
    strncpy(rt->RUNTIME_TIME, time_line, sizeof(rt->RUNTIME_TIME) - 1);
    rt->RUNTIME_TIME[sizeof(rt->RUNTIME_TIME) - 1] = '\0';
    
    if (default_filename) {
        strncpy(rt->RUNTIME_FILENAME, default_filename, sizeof(rt->RUNTIME_FILENAME) - 1);
        rt->RUNTIME_FILENAME[sizeof(rt->RUNTIME_FILENAME) - 1] = '\0';
    }

    char preamble[128] = "%\n";
    strcat(preamble, id_line);
    strcat(preamble, "\n");
    prepend_to_output_buffer(preamble);


}


