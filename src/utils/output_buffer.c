#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "output_buffer.h"



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
        gcode_output = realloc(gcode_output, gcode_output_capacity);
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

void save_output_to_file(const char* filename) {
    FILE* out = fopen(filename, "w");
    if (!out) {
        perror("Failed to write output file");
        return;
    }
    fwrite(gcode_output, 1, gcode_output_length, out);
    fclose(out);
}


void prepend_to_output_buffer(const char* prefix) {
    size_t prefix_len = strlen(prefix);

    if (prefix_len + gcode_output_length >= gcode_output_capacity) {
        gcode_output_capacity = (gcode_output_capacity + prefix_len + 256) * 2;
        gcode_output = realloc(gcode_output, gcode_output_capacity);
    }

    // Shift current contents to the right
    memmove(gcode_output + prefix_len, gcode_output, gcode_output_length + 1);
    memcpy(gcode_output, prefix, prefix_len);
    gcode_output_length += prefix_len;
}


