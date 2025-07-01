#include <stdlib.h>
#include <stdio.h>

char* read_file_to_buffer(const char* filename, long* out_size) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char* buffer = malloc(size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, size, file);
    buffer[size] = '\0';
    fclose(file);

    if (out_size) *out_size = size;
    return buffer;
}
