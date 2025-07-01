#ifndef FILE_UTILS_H
#define FILE_UTILS_H

char* read_file_to_buffer(const char* filename, long* out_size);

const char* get_input_file();
const char* get_output_file();
int get_debug();

#endif
