#ifndef AST_NODES_H
#define AST_NODES_H

#include "../lexer/token_types.h"

typedef struct ASTNode ASTNode;

// G-code argument (like X[i] or Y[10])
typedef struct
{
    char *key;          // e.g., "X", "Y"
    ASTNode *indexExpr; // expression inside []
} GArg;

typedef enum
{
    AST_NOP,
    AST_LET,
    AST_ASSIGN,
    AST_VAR,
    AST_INDEX,
    AST_FUNCTION,
    AST_CALL,
    AST_RETURN,
    AST_NUMBER,
    AST_BINARY,
    AST_GCODE,
    AST_WHILE,
    AST_FOR,
    AST_BLOCK,
    AST_NOTE,
    AST_IF,
    AST_UNARY,
    
    AST_NODE_TYPE_COUNT // <- this must always be the last
} ASTNodeType;

struct ASTNode
{
    ASTNodeType type;

    union
    {





struct {
    char *var;
    ASTNode *from;
    ASTNode *to;
    ASTNode *step;
    int exclusive;
    ASTNode *body;
} for_stmt;






struct
{
    char *name;
    ASTNode *expr;
} assign_stmt;

        struct
        {
            char *name;
            ASTNode **args;
            int arg_count;
        } call_expr;

        struct
        { // function definition: function name(args...) { body }
            char *name;
            char **params; // list of parameter names
            int param_count;
            ASTNode *body;
        } function_def;

        struct
        {
            ASTNode *expr;
        } return_stmt;

        struct
        {
            char *name;
            char **params;
            int param_count;
            ASTNode *body;
        } function_stmt;

        struct
        { // binary expression: left + right
            Token_Type op;
            ASTNode *left;
            ASTNode *right;
        } binary_expr;

        struct
        { // let x = expr
            char *name;
            ASTNode *expr; // ✅ store the expression instead of value
        } let_stmt;

        struct
        { // variable reference: e.g., x
            char *name;
        } var;

        struct
        { // numeric constant
            double value;
        } number;

        struct
        { // array-style access: e.g., X[i]
            ASTNode *array;
            ASTNode *index;
        } index_expr;

        struct
        {               // G-code: e.g., G1 X[i] Y[0]
            char *code; // "G1", "M3", etc.
            GArg *args; // array of arguments
            int argCount;
        } gcode_stmt;

        struct
        { // while condition { ... }
            ASTNode *condition;
            ASTNode *body;
        } while_stmt;



        struct
        { // block: { ... }
            ASTNode **statements;
            int count;
        } block;

        struct
        { // note { ... }
            char *content;
        } note;

        struct
        { // unary expression: !x
            Token_Type op;
            ASTNode *operand;
        } unary_expr;

        struct
        {
            ASTNode *condition;
            ASTNode *then_branch;
            ASTNode *else_branch; // NULL if no else
        } if_stmt;
    };
};

// Entry point
ASTNode *parse_script(const char *source);
void free_ast(ASTNode *node);

#endif // AST_NODES_H
