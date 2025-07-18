#ifndef DEBUG_HELPERS_H
#define DEBUG_HELPERS_H

#include <stdio.h>

// Helper macros for debug output patterns
#define DEBUG_PRINT(context, format, ...) \
    printf("[%s] " format "\n", context, ##__VA_ARGS__); \
    fflush(stdout)

#define DEBUG_PRINT_COND(condition, context, format, ...) \
    if (condition) { \
        printf("[%s] " format "\n", context, ##__VA_ARGS__); \
        fflush(stdout); \
    }

#define DEBUG_PRINT_VALUE(context, name, value) \
    printf("[%s] %s = %.6f\n", context, name, value); \
    fflush(stdout)

#define DEBUG_PRINT_ARRAY(context, name, count) \
    printf("[%s] %s = [array with %zu items]\n", context, name, count); \
    fflush(stdout)

// Helper macro for statement counting
#define INCREMENT_STATEMENT_COUNT(rt) \
    (rt)->statement_count++

#endif // DEBUG_HELPERS_H 