// utils/report.c
#include <stdio.h>
#include "report.h"

void print_compilation_report(long input_size, long output_size, double parse_time, double emit_time, long mem_kb, int statement_count) {
    printf("\n=== GGcode Compilation Report ===\n");
    printf("Input File Size : %ld bytes (%.2f KB)\n", input_size, input_size / 1024.0);
    printf("G-code Output   : %ld bytes (%.2f KB)\n", output_size, output_size / 1024.0);
    printf("Parse Time      : %.4f sec\n", parse_time);
    printf("Emit Time       : %.4f sec\n", emit_time);
    printf("Memory Used     : %ld KB\n", mem_kb);
    printf("Statements      : %d\n", statement_count);
}
