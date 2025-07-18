#ifndef MEMORY_HELPERS_H
#define MEMORY_HELPERS_H

#include <stdlib.h>
#include "../error/error.h"

// Helper macros for common memory allocation patterns
#define SAFE_MALLOC(type, context) \
    type *safe_malloc_ptr = malloc(sizeof(type)); \
    if (!safe_malloc_ptr) { \
        report_error("[%s] malloc failed for " #type, context); \
        return NULL; \
    }

#define SAFE_MALLOC_ARRAY(type, count, context) \
    type *safe_malloc_array_ptr = malloc(sizeof(type) * (count)); \
    if (!safe_malloc_array_ptr) { \
        report_error("[%s] malloc failed for " #type " array of size %d", context, count); \
        return NULL; \
    }

#define SAFE_STRDUP(str, context) \
    char *safe_strdup_copy = strdup(str); \
    if (!safe_strdup_copy) { \
        report_error("[%s] strdup failed", context); \
        return NULL; \
    }

#define SAFE_FREE(ptr) \
    if (ptr) { \
        free(ptr); \
        ptr = NULL; \
    }

// Helper macros for dynamic array growth
#define GROW_ARRAY(array, count, capacity, type, context) \
    if (count >= capacity) { \
        int new_capacity = (capacity == 0) ? 4 : capacity * 2; \
        type **new_array = malloc(new_capacity * sizeof(type *)); \
        if (!new_array) { \
            report_error("[%s] malloc failed for array growth", context); \
            return NULL; \
        } \
        if (array) { \
            memcpy(new_array, array, count * sizeof(type *)); \
            free(array); \
        } \
        array = new_array; \
        capacity = new_capacity; \
    }

// Helper macro for AST node allocation
#define SAFE_MALLOC_AST_NODE(context) \
    ASTNode *safe_ast_node = malloc(sizeof(ASTNode)); \
    if (!safe_ast_node) { \
        report_error("[%s] malloc failed for ASTNode", context); \
        return NULL; \
    }

// Helper macro for string duplication from parser.current.value
#define SAFE_STRDUP_CURRENT(context) \
    char *safe_current_copy = strdup(parser.current.value); \
    if (!safe_current_copy) { \
        report_error("[%s] strdup failed for current token", context); \
        return NULL; \
    }

// Helper macro for dynamic array growth with realloc
#define GROW_ARRAY_REALLOC(array, count, capacity, type, context) \
    if (count >= capacity) { \
        capacity = (capacity == 0) ? 4 : capacity * 2; \
        type *new_array = realloc(array, capacity * sizeof(type)); \
        if (!new_array) { \
            report_error("[%s] realloc failed for array growth", context); \
            return NULL; \
        } \
        array = new_array; \
    }



#endif // MEMORY_HELPERS_H 