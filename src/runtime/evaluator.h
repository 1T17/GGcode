#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "../parser/ast_nodes.h"
#include "runtime_state.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.71828182845904523536
#endif

void enter_scope(void);
void exit_scope(void);

// --- Value types ---
typedef enum {
    VAL_NUMBER,
    VAL_ARRAY
} ValueType;

// --- Value ---
typedef struct Value {
    ValueType type;
    union {
        double number;  // VAL_NUMBER
        struct {
            struct Value **items;
            size_t count;
        } array;        // VAL_ARRAY
    };
} Value;

// --- Function declarations ---
Value *make_number_value(double x);
Value *copy_value(Value *val);
void free_value(Value *val);

// Core API
void set_var(const char *name, Value *val);
Value *get_var(const char *name);
int var_exists(const char *name);
void declare_var(const char *name, Value *val);

// Scalar shortcuts
void set_scalar(const char *name, double value);

double get_scalar(ASTNode *node);

// Arrays
void declare_array(const char *name, Value **items, size_t count);
Value *get_array_item(Value *arr, int index);
void set_array_item(Value *arr, int index, Value *item);

// Scope control
void enter_scope(void);
void exit_scope(void);

// Evaluation
Value *eval(ASTNode *node);

Value *eval_expr(ASTNode *node);
void eval_block(ASTNode *block);

// Function system
void register_function(ASTNode *node);
void reset_runtime_state(void); // test/reset
void reset_parser_state(void);

ASTNode *parse_script_from_string(const char *source);

// Configuration variable detection
void check_config_variable(const char* name, Value* val);

#endif // EVALUATOR_H
