// utils/report.c
#include <stdio.h>
#include "report.h"

void print_compilation_report(long input_size, long output_size, double parse_time, double emit_time, long mem_kb, int statement_count) {


#if defined(_WIN32)
  printf("\n   ______________________  ____  ______    \n");
    printf("  / ____/ ____/ ____/ __ \\/ __ \\/ ____/  \n");
    printf(" / / __/ / __/ /   / / / / / / / __/       \n");
    printf("/ /_/ / /_/ / /___/ /_/ / /_/ / /___       \n");
    printf("\\____/\\____/\\____/\\____/_____/_____/   \n");
    printf("                                           \n");
    printf("Input      : %ld bytes (%.2f KB)\n", input_size, input_size / 1024.0);
    printf("Output     : %ld bytes (%.2f KB)\n", output_size, output_size / 1024.0);
    printf("Parse      : %.4f sec   Emit: %.4f sec\n", parse_time, emit_time);
    printf("Memory     : %ld KB     Statements: %d\n", mem_kb, statement_count);
    printf("-----------------------------------------------\n");
#else
  printf("\n┏┓┏┓┏┓┏┓┳┓┏┓  ┏┓       •┓   •      ┳┓         \n");
    printf("┃┓┃┓┃ ┃┃┃┃┣   ┃ ┏┓┏┳┓┏┓┓┃┏┓╋┓┏┓┏┓  ┣┫┏┓┏┓┏┓┏┓╋\n");
    printf("┗┛┗┛┗┛┗┛┻┛┗┛  ┗┛┗┛┛┗┗┣┛┗┗┗┻┗┗┗┛┛┗  ┛┗┗ ┣┛┗┛┛ ┗\n");
    printf("                     ┛                 ┛      \n");
printf("\033[1;37mInput  \033[0m : \033[1;33m%ld bytes\033[1;22m (%.2f KB)\n", input_size, input_size / 1024.0);
printf("\033[1;37mOutput \033[0m : \033[1;33m%ld bytes\033[1;22m  (%.2f KB)\n", output_size, output_size / 1024.0);
printf("\033[1;37mParse  \033[0m : \033[1;36m%.4f sec\033[0m   \033[1;37mEmit\033[0m: \033[1;36m%.4f sec\033[0m\n", parse_time, emit_time);
printf("\033[1;37mMemory \033[0m : \033[1;33m%ld KB\033[0m     \033[1;37mStatements\033[0m: \033[1;33m%d\033[0m\n", mem_kb, statement_count);

    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
#endif





}
