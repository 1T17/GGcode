#ifndef RUNTIME_STATE_H
#define RUNTIME_STATE_H

#include "../parser/ast_nodes.h"
#include "../parser/parser.h"
#include <stddef.h>

#define MAX_VARIABLES 1024
#define MAX_FUNCTIONS 64
#define MAX_FUNCTION_STACK_DEPTH 32

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

// --- Function Context for tracking function call stack ---
typedef struct {
    char *function_name;
    int scope_level;
    ASTNode *function_node;
} FunctionContext;

// --- Function Stack for managing nested function calls ---
typedef struct {
    FunctionContext contexts[MAX_FUNCTION_STACK_DEPTH];
    int count;
} FunctionStack;

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
    
    // Function context tracking for return statement validation
    FunctionStack function_stack;
    
    // Recursion protection
    int recursion_depth;        // Current function call depth
    int max_recursion_depth;    // Maximum allowed depth (default: 100)

    Parser parser;  // Parser state moved from global to runtime
    // Add more fields as needed (error state, output buffer, etc.)
} Runtime;

#endif // RUNTIME_STATE_H 