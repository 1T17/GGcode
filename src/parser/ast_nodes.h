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
    AST_COMPOUND_ASSIGN,
    AST_VAR,
    AST_ARRAY_LITERAL, 
    AST_INDEX,
    AST_ASSIGN_INDEX,
    AST_FUNCTION,
    AST_CALL,
    AST_EXPR_STMT, // <-- ADD THIS
    AST_RETURN,
    AST_NUMBER,
    AST_STRING,
    AST_BINARY,
    AST_TERNARY,
    AST_GCODE,
    AST_WHILE,
    AST_FOR,
    AST_BLOCK,
    AST_NOTE,
    AST_IF,
    AST_UNARY,
     AST_EMPTY ,
    
    
    AST_NODE_TYPE_COUNT // <- this must always be the last
} ASTNodeType;

struct ASTNode
{
    struct ASTNode *parent;
    ASTNodeType type;

    union
    {



struct {
    ASTNode *expr;
} expr_stmt;



struct {
    char *name;
    ASTNode *expr;
} assign_stmt;

struct {
    char *name;
    Token_Type op;  // TOKEN_PLUS_EQUAL, TOKEN_MINUS_EQUAL, etc.
    ASTNode *expr;
} compound_assign;


struct {
    ASTNode *target;  // should be AST_INDEX node
    ASTNode *value;   // right-hand side
} assign_index;



        

struct {
    ASTNode **elements;  // array of ASTNode*
    int count;           // number of elements
} array_literal;


struct {
    char *var;
    char *index_var;  // For (char, index) syntax - optional
    ASTNode *from;
    ASTNode *to;
    ASTNode *step;
    ASTNode *iterable; // For string iteration: for char in string_var
    int exclusive;
    int is_string_iteration; // Flag to distinguish string vs numeric iteration
    ASTNode *body;
} for_stmt;





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
        { // string literal constant
            char *value;
        } string_literal;

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

        struct
        { // ternary expression: condition ? true_expr : false_expr
            ASTNode *condition;
            ASTNode *true_expr;
            ASTNode *false_expr;
        } ternary_expr;
    };
};

// Entry point
ASTNode *parse_script();
ASTNode *parse_expression_from_string(const char *expr_text);
void free_ast(ASTNode *node);

#endif // AST_NODES_H
