#ifndef OUTPUT_BUFFER_H
#define OUTPUT_BUFFER_H

#include <stddef.h>

#define OUTPUT_BUFFER_SIZE 65536

extern char output_buffer[];  // <-- this is the declaration

void init_output_buffer();
void write_to_output(const char* line);
void free_output_buffer();
const char* get_output_buffer();
size_t get_output_length();
void prepend_to_output_buffer(const char* prefix);  // <-- your prepend function
void save_output_to_file(const char* filename); 
void emit_gcode_preamble(const char* default_filename); 

#endif // OUTPUT_BUFFER_H
