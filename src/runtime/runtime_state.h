#ifndef RUNTIME_STATE_H
#define RUNTIME_STATE_H

#include "../parser/ast_nodes.h"
#include "../parser/parser.h"
#include <stddef.h>

#define MAX_VARIABLES 1024
#define MAX_FUNCTIONS 64

// Forward declarations
struct Value;

// --- Variable ---
typedef struct {
    char *name;
    int scope_level;
    struct Value *val;
} Variable;

// --- Function ---
typedef struct {
    char *name;
    ASTNode *node; // AST_FUNCTION node
} FunctionEntry;

// --- Runtime State ---
typedef struct Runtime {
    int statement_count;
    char RUNTIME_TIME[64];
    char RUNTIME_FILENAME[256];
    Variable variables[MAX_VARIABLES];
    int var_count;
    FunctionEntry function_table[MAX_FUNCTIONS];
    int function_count;
    int current_scope_level;
    int debug;  // Add debug field to runtime state
    Parser parser;  // Parser state moved from global to runtime
    // Add more fields as needed (error state, output buffer, etc.)
} Runtime;

#endif // RUNTIME_STATE_H 