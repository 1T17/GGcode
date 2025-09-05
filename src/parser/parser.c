#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parser.h"
#include "../utils/compat.h"
#include "../runtime/evaluator.h"
#include "../runtime/runtime_state.h"
#include "../config/config.h"
#include "error/error.h"
#include <ctype.h>
#include <setjmp.h>
#define M_PI 3.14159265358979323846
#define PARSE_ERROR(msg, ...) \
    fatal_error(get_runtime()->parser.lexer->source, get_runtime()->parser.current.line, get_runtime()->parser.current.column, msg, ##__VA_ARGS__)

// Enhanced error reporting macro specifically for return statements
#define RETURN_ERROR(context, msg, ...) \
    report_return_error(get_runtime()->parser.lexer->source, get_runtime()->parser.current.line, get_runtime()->parser.current.column, context, msg, ##__VA_ARGS__)

// Non-fatal error reporting that allows parsing to continue
#define PARSE_WARNING(msg, ...) \
    report_error("Warning at %d:%d: " msg, get_runtime()->parser.current.line, get_runtime()->parser.current.column, ##__VA_ARGS__)

// Parser moved to runtime state - no more global parser

static ASTNode *parse_binary_expression();
static ASTNode *parse_block();
static ASTNode *parse_while();
static ASTNode *parse_for();
static ASTNode *parse_if();
static ASTNode *parse_let();
static ASTNode *parse_note();
static ASTNode *parse_gcode();
static ASTNode *parse_identifier_statement();
static ASTNode *parse_statement();
static ASTNode *parse_primary();
static ASTNode *parse_function();
static ASTNode *parse_return();
// static ASTNode *parse_assignment();
static ASTNode *parse_postfix_expression(); // <-- add this

// Forward declaration

// Restore static variable needed by parse_gcode
static int gcode_mode_active = 0;

/// @brief step 2
/// @return
ASTNode *parse_script() {


    if (setjmp(fatal_error_jump_buffer)) {
        // ⛔ Fatal error triggered, return NULL cleanly

        fatal_error_triggered = 0;
        return NULL;
    }

    ASTNode **statements = NULL;
    int count = 0;
    int capacity = 0;

    

    Runtime *rt = get_runtime();
    while (rt->parser.current.type != TOKEN_EOF) {
        if (rt->parser.current.type == TOKEN_NEWLINE) {
            parser_advance();
            continue;
        }



       /// step 3

        ASTNode *stmt = parse_statement();

        if (!stmt) {
            report_error("[parse_script] Skipping NULL stmt");
            continue;
        }

       

if (stmt->type == AST_EMPTY || stmt->type == AST_NOP)
    continue;


                if (count >= capacity) {
            int new_capacity = (capacity == 0) ? 4 : capacity * 2;
            ASTNode **new_array = malloc(new_capacity * sizeof(ASTNode *));
if (!new_array) {
    PARSE_ERROR("[parse_script] Memory allocation failed for statement array");
}

            if (statements) {
                memcpy(new_array, statements, count * sizeof(ASTNode *));
                free(statements);
            }
            statements = new_array;
            capacity = new_capacity;
        }

        statements[count++] = stmt;
        }

    ASTNode *block = malloc(sizeof(ASTNode));
if (!block) {
    PARSE_ERROR("[parse_script] Memory allocation failed for block");
}


    block->type = AST_BLOCK;
    block->block.statements = statements;
    block->block.count = count;
    return block;
}

int get_precedence(Token_Type op)
{
    switch (op)
    {
    case TOKEN_CARET:
        return 5; // highest precedence for exponentiation
    case TOKEN_STAR:
    case TOKEN_SLASH:
        return 4;
    case TOKEN_PLUS:
    case TOKEN_MINUS:
        return 3;
    case TOKEN_LSHIFT:
    case TOKEN_RSHIFT:
        return 3; // Same as addition/subtraction
    case TOKEN_LESS:
    case TOKEN_LESS_EQUAL:
    case TOKEN_GREATER:
    case TOKEN_GREATER_EQUAL:
    case TOKEN_EQUAL_EQUAL:
    case TOKEN_BANG_EQUAL:
        return 2;
    case TOKEN_AMPERSAND: // Bitwise AND
        return 1; // Lower precedence than comparison
    case TOKEN_PIPE:      // Bitwise OR
        return 1; // Same as bitwise AND
    case TOKEN_AND:       // Logical AND
    case TOKEN_OR:        // Logical OR
        return 1;         
    case TOKEN_QUESTION:  // Ternary operator
        return 0;         // lowest precedence (right-associative)
    default:
        return -1;
    }
}

void parser_advance()
{
    Runtime *rt = get_runtime();
    // printf("[Parser parser_advance()] calling lexer_next_token...\n");
    rt->parser.previous = rt->parser.current;
    rt->parser.current = lexer_next_token(rt->parser.lexer);
    // printf("[Parser parser_advance()] got token: type=%d, value=%s\n", rt->parser.current.type, rt->parser.current.value);
}

// Function definition context tracking for parser
void parser_enter_function_definition(void)
{
    Runtime *rt = get_runtime();
    rt->parser.function_definition_depth++;
}

void parser_exit_function_definition(void)
{
    Runtime *rt = get_runtime();
    if (rt->parser.function_definition_depth > 0) {
        rt->parser.function_definition_depth--;
    }
}

int parser_is_inside_function_definition(void)
{
    Runtime *rt = get_runtime();
    return rt->parser.function_definition_depth > 0;
}

static int match(Token_Type type)
{
    Runtime *rt = get_runtime();
    if (rt->parser.current.type == type)
    {
        parser_advance();
        return 1;
    }
    return 0;
}

static ASTNode *parse_unary()
{
    Runtime *rt = get_runtime();
    if (rt->parser.current.type == TOKEN_BANG || rt->parser.current.type == TOKEN_MINUS)
    {
        Token op = rt->parser.current;
        parser_advance();

        ASTNode *operand = parse_unary(); // recursive for !! or -- etc.

        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = AST_UNARY;
        node->unary_expr.op = op.type;
        node->unary_expr.operand = operand;
        return node;
    }

    return parse_postfix_expression(); // fallback to normal expression
}

static ASTNode *parse_primary()
{

    //printf("[parse_primary]---------------------------------------\n");


    Runtime *rt = get_runtime();
    // Handle parentheses
if (rt->parser.current.type == TOKEN_LPAREN)
{
    //printf("[parse_primary] 🌀 Detected '('\n");
    parser_advance(); // consume '('
    //printf("[parse_primary] ⏬ Parsing inner expression...\n");

    // Handle empty parentheses like: ()
    if (rt->parser.current.type == TOKEN_RPAREN) {
        //printf("[parse_primary] ⚠️ Empty parentheses detected — unexpected in primary expression.\n");
        report_error("[parse_primary] Unexpected empty '()' expression.");
        return NULL;
    }

    ASTNode *expr = parse_binary_expression();

    if (!expr) {
        report_error("[parse_primary] Failed to parse inner expression inside parentheses");
        return NULL;
    }

    //printf("[parse_primary] ✅ Finished parsing inner expression. Expecting ')'\n");

    if (!match(TOKEN_RPAREN))
    {
        report_error("[parse_primary] Expected ')' after expression, but got '%s'", rt->parser.current.value);
        return NULL;
    }

    //printf("[parse_primary] ✅ Matched closing ')', returning inner expression AST\n");
    return expr;
}








    // Handle unary minus
    if (rt->parser.current.type == TOKEN_MINUS)
    {
        parser_advance(); // consume '-'
        ASTNode *right = parse_primary();

        ASTNode *zero = malloc(sizeof(ASTNode));
        zero->type = AST_NUMBER;
        zero->number.value = 0;

        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = AST_BINARY;
        node->binary_expr.op = TOKEN_MINUS;
        node->binary_expr.left = zero;
        node->binary_expr.right = right;
        return node;
    }

    // Handle built-in math constants
    if (
        rt->parser.current.type == TOKEN_FUNC_PI ||
        rt->parser.current.type == TOKEN_FUNC_TAU ||
        rt->parser.current.type == TOKEN_FUNC_EU ||
        rt->parser.current.type == TOKEN_FUNC_DEG_TO_RAD ||
        rt->parser.current.type == TOKEN_FUNC_RAD_TO_DEG)
    {
        double val = 0.0;
        switch (rt->parser.current.type)
        {
        case TOKEN_FUNC_PI:
            val = M_PI;
            break;
        case TOKEN_FUNC_TAU:
            val = 2 * M_PI;
            break;
        case TOKEN_FUNC_EU:
            val = 2.718281828459045;
            break;
        case TOKEN_FUNC_DEG_TO_RAD:
            val = M_PI / 180.0;
            break;
        case TOKEN_FUNC_RAD_TO_DEG:
            val = 180.0 / M_PI;
            break;
        default:
            break;
        }

        parser_advance(); // consume the constant
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = AST_NUMBER;
        node->number.value = val;
        return node;
    }

    // Handle function calls and variables
    if (rt->parser.current.type == TOKEN_IDENTIFIER ||
        (rt->parser.current.type >= TOKEN_FUNC_ABS && rt->parser.current.type <= TOKEN_FUNC_EXP))
    {

        char *name = strdup(rt->parser.current.value);
        //fprintf(stderr, "[Parser] Detected identifier or function name: '%s'\n", name);

        parser_advance(); // consume function or variable name

        // Check for function call
        if (rt->parser.current.type == TOKEN_LPAREN)
        {
           // fprintf(stderr, "[Parser] Detected function call: '%s(...)'\n", name);
            parser_advance(); // consume '('

            ASTNode **args = NULL;
            int arg_count = 0, arg_capacity = 0;

            while (rt->parser.current.type != TOKEN_RPAREN)
            {
                if (arg_count >= arg_capacity)
                {
                    arg_capacity = (arg_capacity == 0) ? 4 : arg_capacity * 2;
                    args = realloc(args, arg_capacity * sizeof(ASTNode *));
                }

                //fprintf(stderr, "[Parser] Parsing argument %d for '%s'\n", arg_count + 1, name);
                args[arg_count++] = parse_binary_expression();

                if (rt->parser.current.type == TOKEN_COMMA)
                {
                    //fprintf(stderr, "[Parser] Found ',' – continuing to next argument\n");
                    parser_advance(); // consume comma
                }
            }

            if (match(TOKEN_RPAREN))
            {
               // fprintf(stderr, "[Parser] Successfully closed argument list for '%s' with %d args\n", name, arg_count);
            }
            else
            {
               // fprintf(stderr, "[Parser] ERROR: Expected ')' after function arguments for '%s'\n", name);
            }

            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = AST_CALL;
            node->call_expr.name = name;
            node->call_expr.args = args;
            node->call_expr.arg_count = arg_count;

            //fprintf(stderr, "[Parser] Created AST_CALL node for function '%s' with %d arguments\n", name, arg_count);
            return node;
        }

        else
        {
            // Variable reference
            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = AST_VAR;
            node->var.name = name;
            return node;
        }
    }

    // Handle numeric constants
    if (rt->parser.current.type == TOKEN_NUMBER)
    {
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = AST_NUMBER;
        node->number.value = atof(rt->parser.current.value);
        parser_advance(); // consume number
        return node;
    }

    // Handle string literals
    if (rt->parser.current.type == TOKEN_STRING)
    {
        ASTNode *node = malloc(sizeof(ASTNode));
        if (!node) {
            PARSE_ERROR("Memory allocation failed for string literal");
        }
        
        node->type = AST_STRING;
        node->string_literal.value = strdup(rt->parser.current.value);
        if (!node->string_literal.value) {
            free(node);
            PARSE_ERROR("Memory allocation failed for string literal value");
        }
        
        parser_advance(); // consume string
        return node;
    }

    // Handle array literal: [1, 2, 3] or [[1, 2], [3, 4]]
    if (rt->parser.current.type == TOKEN_LBRACKET)
    {
        parser_advance(); // consume '['

        ASTNode **elements = NULL;
        int count = 0, capacity = 0;

        // Handle non-empty array
        if (rt->parser.current.type != TOKEN_RBRACKET)
        {
            while (1)
            {
                if (count >= capacity)
                {
                    capacity = capacity == 0 ? 4 : capacity * 2;
                    elements = realloc(elements, capacity * sizeof(ASTNode *));
                }

                elements[count++] = parse_binary_expression();

                if (rt->parser.current.type == TOKEN_COMMA)
                {
                    parser_advance(); // consume comma and continue
                }
                else if (rt->parser.current.type == TOKEN_RBRACKET)
                {
                    break; // closing bracket
                }
                else
                {
                    report_error("[Parser] Expected ',' or ']' in array literal, found '%s'", rt->parser.current.value);
                    return NULL;
                }
            }
        }

        match(TOKEN_RBRACKET); // consume final ']'

        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = AST_ARRAY_LITERAL;
        node->array_literal.elements = elements;
        node->array_literal.count = count;

        // Now parse chained index access like: a[1][2]
        while (rt->parser.current.type == TOKEN_LBRACKET)
        {
            parser_advance(); // consume '['
            ASTNode *index = parse_binary_expression();

            if (!match(TOKEN_RBRACKET))
            {
                PARSE_ERROR("Expected ']' after index");
            }

            ASTNode *index_node = malloc(sizeof(ASTNode));
            index_node->type = AST_INDEX;
            index_node->index_expr.array = node;
            index_node->index_expr.index = index;
            node = index_node; // chain it
        }

        return node;
    }

            PARSE_ERROR("[parse_primary end] Unexpected token in expression: '%s'", rt->parser.current.value);




    // Return dummy NOP node to avoid crashing
    ASTNode *dummy = malloc(sizeof(ASTNode));
    dummy->type = AST_NOP;
    return dummy;
}

static ASTNode *parse_postfix_expression()
{
    Runtime *rt = get_runtime();
    ASTNode *expr = parse_primary();

    while (rt->parser.current.type == TOKEN_LBRACKET)
    {
        parser_advance(); // consume '['
        ASTNode *index = parse_binary_expression();
        if (!match(TOKEN_RBRACKET))
        {
            PARSE_ERROR("Expected ']' after index");
        }

        ASTNode *index_node = malloc(sizeof(ASTNode));
        index_node->type = AST_INDEX;
        index_node->index_expr.array = expr;
        index_node->index_expr.index = index;
        expr = index_node;
    }

    return expr;
}

static ASTNode *parse_binary_expression_prec(int min_prec)
{
    Runtime *rt = get_runtime();
    ASTNode *left = parse_unary();

    while (1)
    {
        Token_Type op = rt->parser.current.type;
        int prec = get_precedence(op);

        if (prec < min_prec)
            break;

        // Handle ternary operator specially
        if (op == TOKEN_QUESTION)
        {
            parser_advance(); // consume '?'
            
            ASTNode *true_expr = parse_binary_expression_prec(0); // Parse true expression
            
            if (rt->parser.current.type != TOKEN_COLON)
            {
                PARSE_ERROR("Expected ':' after '?' in ternary expression");
            }
            
            parser_advance(); // consume ':'
            
            ASTNode *false_expr = parse_binary_expression_prec(prec); // Right-associative
            
            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = AST_TERNARY;
            node->ternary_expr.condition = left;
            node->ternary_expr.true_expr = true_expr;
            node->ternary_expr.false_expr = false_expr;
            
            left = node;
        }
        else
        {
            parser_advance(); // consume operator

            // Right-associative for exponentiation, left-associative for others
            int next_prec = (op == TOKEN_CARET) ? prec : prec + 1;
            ASTNode *right = parse_binary_expression_prec(next_prec);

            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = AST_BINARY;
            node->binary_expr.op = op;
            node->binary_expr.left = left;
            node->binary_expr.right = right;

            left = node;
        }
    }

    return left;
}

static ASTNode *parse_binary_expression()
{
    return parse_binary_expression_prec(0);
}

static ASTNode *parse_function()
{
    Runtime *rt = get_runtime();
    parser_advance(); // Skip 'function' keyword

    if (rt->parser.current.type != TOKEN_IDENTIFIER)
    {
        PARSE_ERROR("Expected function name after 'function'");
    }

    char *name = strdup(rt->parser.current.value); // Save function name
    parser_advance();                          // Consume name

    if (!match(TOKEN_LPAREN))
    {
        PARSE_ERROR("Expected '(' after function name");
    }

    // Handle parameter list
    char **params = NULL;
    int param_count = 0, param_capacity = 0;

    while (rt->parser.current.type != TOKEN_RPAREN)
    {
        if (rt->parser.current.type != TOKEN_IDENTIFIER)
        {
            PARSE_ERROR("Expected parameter name, got '%s'", rt->parser.current.value);
        }

        // Grow parameter list if needed
        if (param_count >= param_capacity)
        {
            param_capacity = param_capacity == 0 ? 4 : param_capacity * 2;
            params = realloc(params, param_capacity * sizeof(char *));
            if (!params)
            {
                PARSE_ERROR("Out of memory while parsing parameters");
            }
        }

        params[param_count++] = strdup(rt->parser.current.value);
        parser_advance(); // Consume parameter name

        if (rt->parser.current.type == TOKEN_COMMA)
        {
            parser_advance(); // Consume comma
        }
        else if (rt->parser.current.type != TOKEN_RPAREN)
        {
            PARSE_ERROR("Expected ',' or ')' in parameter list");
        }
    }

    parser_advance(); // Consume ')'

    // Enter function definition context for return statement validation
    parser_enter_function_definition();
    
    // Parse function body block
    ASTNode *body = parse_block();
    
    // Exit function definition context
    parser_exit_function_definition();

    // Build function node
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = AST_FUNCTION;
    node->function_stmt.name = name;
    node->function_stmt.params = params;
    node->function_stmt.param_count = param_count;
    node->function_stmt.body = body;

    if (body)
        body->parent = node;



    return node;
}

static ASTNode *parse_return()
{
    Runtime *rt = get_runtime();
    
    // Store the return token position for detailed error reporting
    Token return_token = rt->parser.current;
    parser_advance(); // skip 'return'
    
    // Enhanced function context validation with detailed error messages
    if (!parser_is_inside_function_definition()) {
        // Provide context-specific error messages based on current parsing context
        if (rt->parser.lexer->line == 1) {
            PARSE_ERROR("Return statement at global scope (line %d:%d) - return statements are only valid inside functions.\n"
                       "  → Suggestion: Wrap your code in a function definition:\n"
                       "    function main() {\n"
                       "        return %s\n"
                       "    }", 
                       return_token.line, return_token.column,
                       rt->parser.current.type != TOKEN_NEWLINE && rt->parser.current.type != TOKEN_EOF ? "your_expression" : "");
        } else {
            PARSE_ERROR("Return statement outside function context (line %d:%d) - return statements must be inside function definitions.\n"
                       "  → Current context: Global scope\n"
                       "  → Suggestion: Move this return statement inside a function", 
                       return_token.line, return_token.column);
        }
        
        // Error recovery: create a dummy return node to continue parsing
        ASTNode *error_node = malloc(sizeof(ASTNode));
        if (error_node) {
            error_node->type = AST_NOP;  // Use NOP to indicate error recovery
        }
        return error_node;
    }
    
    ASTNode *expr = NULL;
    
    // Enhanced handling of optional return expressions (bare 'return' statements)
    // Check if the next token indicates end of statement or block
    if (rt->parser.current.type != TOKEN_NEWLINE && 
        rt->parser.current.type != TOKEN_EOF &&
        rt->parser.current.type != TOKEN_RBRACE &&
        rt->parser.current.type != TOKEN_SEMICOLON) {
        
        // Save current parser state for comprehensive error recovery
        Token saved_current = rt->parser.current;
        
        // Parse the return expression with enhanced error recovery
        expr = parse_binary_expression();
        
        // Comprehensive validation for complex mathematical expressions
        if (!expr) {
            // Enhanced error recovery with detailed context and suggestions
            switch (saved_current.type) {
                case TOKEN_PLUS:
                case TOKEN_MINUS:
                case TOKEN_STAR:
                case TOKEN_SLASH:
                case TOKEN_CARET:
                    PARSE_ERROR("Invalid operator '%s' at start of return expression (line %d:%d).\n"
                               "  → Error: Operators cannot appear at the beginning of expressions\n"
                               "  → Suggestion: Add a value before the operator, e.g., 'return 0 %s expression'", 
                               saved_current.value, saved_current.line, saved_current.column, saved_current.value);
                    break;
                case TOKEN_RPAREN:
                    PARSE_ERROR("Unexpected ')' in return expression (line %d:%d) - missing opening parenthesis.\n"
                               "  → Error: Unmatched closing parenthesis\n"
                               "  → Suggestion: Add opening '(' or remove the closing ')'", 
                               saved_current.line, saved_current.column);
                    break;
                case TOKEN_COMMA:
                    PARSE_ERROR("Unexpected ',' in return expression (line %d:%d) - return statements accept only single expressions.\n"
                               "  → Error: Multiple values in return statement\n"
                               "  → Suggestion: Return a single value or use an array: 'return [value1, value2]'", 
                               saved_current.line, saved_current.column);
                    break;
                case TOKEN_RBRACKET:
                    PARSE_ERROR("Unexpected ']' in return expression (line %d:%d) - missing opening bracket.\n"
                               "  → Error: Unmatched closing bracket\n"
                               "  → Suggestion: Add opening '[' or remove the closing ']'", 
                               saved_current.line, saved_current.column);
                    break;
                case TOKEN_EQUAL:
                    PARSE_ERROR("Unexpected '=' in return expression (line %d:%d) - assignment not allowed in return statements.\n"
                               "  → Error: Cannot assign values in return expressions\n"
                               "  → Suggestion: Use comparison '==' or move assignment before return", 
                               saved_current.line, saved_current.column);
                    break;
                default:
                    PARSE_ERROR("Invalid or malformed expression in return statement (line %d:%d) starting with '%s'.\n"
                               "  → Error: Unexpected token '%s' of type %d\n"
                               "  → Suggestion: Check expression syntax and ensure proper operators/operands", 
                               saved_current.line, saved_current.column, saved_current.value, 
                               saved_current.value, saved_current.type);
                    break;
            }
            
            // Error recovery: try to continue parsing by skipping problematic tokens
            while (rt->parser.current.type != TOKEN_NEWLINE && 
                   rt->parser.current.type != TOKEN_EOF &&
                   rt->parser.current.type != TOKEN_RBRACE &&
                   rt->parser.current.type != TOKEN_SEMICOLON) {
                parser_advance();
            }
        }
        
        // Enhanced validation for successfully parsed expressions
        if (expr) {
            // Check for common expression errors that parse_binary_expression might miss
            if (expr->type == AST_NOP) {
                PARSE_ERROR("Empty or invalid expression in return statement (line %d:%d).\n"
                           "  → Error: Expression evaluated to no-operation\n"
                           "  → Suggestion: Provide a valid expression after 'return'", 
                           return_token.line, return_token.column);
            }
            
            // Enhanced validation for trailing invalid tokens with specific error messages
            if (rt->parser.current.type != TOKEN_NEWLINE && 
                rt->parser.current.type != TOKEN_EOF &&
                rt->parser.current.type != TOKEN_RBRACE &&
                rt->parser.current.type != TOKEN_SEMICOLON &&
                rt->parser.current.type != TOKEN_ELSE) {
                
                // Check for common mistakes like multiple expressions or missing operators
                if (rt->parser.current.type == TOKEN_NUMBER ||
                    rt->parser.current.type == TOKEN_IDENTIFIER) {
                    PARSE_ERROR("Multiple expressions in return statement (line %d:%d) - found '%s' after '%s'.\n"
                               "  → Error: Return statements can only contain one expression\n"
                               "  → Suggestion: Use an operator between values: 'return %s + %s' or 'return %s * %s'", 
                               rt->parser.current.line, rt->parser.current.column,
                               rt->parser.current.value, rt->parser.previous.value,
                               rt->parser.previous.value, rt->parser.current.value,
                               rt->parser.previous.value, rt->parser.current.value);
                } else if (rt->parser.current.type == TOKEN_LPAREN) {
                    PARSE_ERROR("Unexpected '(' after return expression (line %d:%d).\n"
                               "  → Error: Function call syntax after return expression\n"
                               "  → Suggestion: Move function call inside return: 'return function_name(args)'", 
                               rt->parser.current.line, rt->parser.current.column);
                } else {
                    PARSE_ERROR("Unexpected token '%s' after return expression (line %d:%d).\n"
                               "  → Error: Invalid syntax following return expression\n"
                               "  → Suggestion: End return statement with newline or '}'", 
                               rt->parser.current.value, rt->parser.current.line, rt->parser.current.column);
                }
                
                // Error recovery: skip the problematic token and continue
                parser_advance();
            }
        }
    } else {
        // Bare return statement - provide informational context
        // This is valid, but we can provide helpful context in debug mode
        #ifdef DEBUG_PARSER
        printf("INFO: Bare return statement at line %d:%d (no expression)\n", 
               return_token.line, return_token.column);
        #endif
    }
    
    // Create return AST node with enhanced error checking
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        PARSE_ERROR("Memory allocation failed for return statement (line %d:%d).\n"
                   "  → Error: Out of memory while creating AST node\n"
                   "  → Suggestion: Check available memory or simplify code structure", 
                   return_token.line, return_token.column);
        return NULL;  // Return NULL to indicate failure
    }
    
    node->type = AST_RETURN;
    node->return_stmt.expr = expr;  // Can be NULL for bare returns
    
    // Store line information in the AST node for runtime error reporting
    // Note: This would require extending the AST node structure, but for now we rely on the parser's line tracking
    
    return node;
}

static ASTNode *parse_statement()
{
    Runtime *rt = get_runtime();
    if (rt->parser.current.type == TOKEN_NEWLINE || rt->parser.current.type == TOKEN_EOF)
        return NULL;

    // Keyword-based statements
    if (rt->parser.current.type == TOKEN_FUNCTION)
        return parse_function();
    if (rt->parser.current.type == TOKEN_RETURN)
        return parse_return();
    if (rt->parser.current.type == TOKEN_WHILE)
        return parse_while();
    if (rt->parser.current.type == TOKEN_FOR)
        return parse_for();
    if (rt->parser.current.type == TOKEN_IF)
        return parse_if();
    if (rt->parser.current.type == TOKEN_LET)
        return parse_let();
    if (rt->parser.current.type == TOKEN_NOTE)
        return parse_note();

    // Identifier-based logic (e.g., x = 1, maze[i][j] = 0, or foo(1))
    if (rt->parser.current.type == TOKEN_IDENTIFIER)
    {
        return parse_identifier_statement();
    }

    // G-code commands like G1, G0, M3, etc.
    if (rt->parser.current.type == TOKEN_GCODE_WORD)
    {
        return parse_gcode();
    }

    // Fallback: try to parse as expression and wrap as expression-statement
    ASTNode *expr = parse_binary_expression();
    if (expr && expr->type != AST_NOP)
    {
        ASTNode *stmt = malloc(sizeof(ASTNode));
        stmt->type = AST_EXPR_STMT;
        stmt->expr_stmt.expr = expr;
        return stmt;
    }

    return NULL;
}

static ASTNode *parse_identifier_statement()
{
    Runtime *rt = get_runtime();
    ASTNode *node = parse_postfix_expression();
    if (!node)
        return NULL;

    // Case 1: Function call like foo(1,2)
    if (node->type == AST_CALL)
    {
        ASTNode *stmt = malloc(sizeof(ASTNode));
        stmt->type = AST_EXPR_STMT;
        stmt->expr_stmt.expr = node;
        return stmt;
    }

    // Case 2: Assignment like foo = 123 or maze[i][j] = 7
    if (rt->parser.current.type == TOKEN_EQUAL)
    {
        ASTNode *assign = malloc(sizeof(ASTNode));
        ASTNode *rhs = NULL;
        parser_advance(); // consume '='

        rhs = parse_binary_expression();

        if (node->type == AST_VAR)
        {
            assign->type = AST_ASSIGN;
            assign->assign_stmt.name = node->var.name;
            assign->assign_stmt.expr = rhs;
        }
        else if (node->type == AST_INDEX)
        {
            assign->type = AST_ASSIGN_INDEX;
            assign->assign_index.target = node;
            assign->assign_index.value = rhs;
        }

        return assign;
    }

    // Case 2b: Compound assignment like foo += 123, foo -= 5, etc.
    if (rt->parser.current.type == TOKEN_PLUS_EQUAL ||
        rt->parser.current.type == TOKEN_MINUS_EQUAL ||
        rt->parser.current.type == TOKEN_STAR_EQUAL ||
        rt->parser.current.type == TOKEN_SLASH_EQUAL ||
        rt->parser.current.type == TOKEN_CARET_EQUAL ||
        rt->parser.current.type == TOKEN_AMPERSAND_EQUAL ||
        rt->parser.current.type == TOKEN_PIPE_EQUAL ||
        rt->parser.current.type == TOKEN_LSHIFT_EQUAL ||
        rt->parser.current.type == TOKEN_RSHIFT_EQUAL)
    {
        if (node->type != AST_VAR)
        {
            PARSE_ERROR("[parse_identifier_statement] Compound assignment only supported for variables, not array indices");
        }

        ASTNode *compound = malloc(sizeof(ASTNode));
        compound->type = AST_COMPOUND_ASSIGN;
        compound->compound_assign.name = node->var.name;
        compound->compound_assign.op = rt->parser.current.type;
        
        parser_advance(); // consume compound operator
        compound->compound_assign.expr = parse_binary_expression();

        return compound;
    }

    // Case 3: Implicit assignment like `foo` → `foo = 0`
    if (node->type == AST_VAR)
    {
        ASTNode *assign = malloc(sizeof(ASTNode));
        assign->type = AST_ASSIGN;
        assign->assign_stmt.name = node->var.name;

        ASTNode *rhs = malloc(sizeof(ASTNode));
        rhs->type = AST_NUMBER;
        rhs->number.value = 0.0;

        assign->assign_stmt.expr = rhs;
        return assign;
    }

    // Case 4: Fallback to raw expression
    ASTNode *stmt = malloc(sizeof(ASTNode));
    stmt->type = AST_EXPR_STMT;
    stmt->expr_stmt.expr = node;
    return stmt;
}

static ASTNode *parse_block()
{
    if (!match(TOKEN_LBRACE))
    {
        PARSE_ERROR("Expected '{' to start block");
    }

    ASTNode **statements = NULL;
    int capacity = 0, count = 0;

    Runtime *rt = get_runtime();
    while (!match(TOKEN_RBRACE))
    {

        if (rt->parser.current.type == TOKEN_EOF)
        {
            PARSE_ERROR("Unexpected EOF in block");
        }
////step 6
        ASTNode *stmt = parse_statement();
        if (stmt != NULL)
        {
            if (count >= capacity)
            {
                capacity = capacity == 0 ? 4 : capacity * 2;
                statements = realloc(statements, capacity * sizeof(ASTNode *));
            }

            statements[count++] = stmt;
        }
        else
        {
            report_error("Skipping NULL stmt");
        }

        // Skip any newlines between statements in a block
        while (rt->parser.current.type == TOKEN_NEWLINE)
            parser_advance();
    }

    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_BLOCK;
    node->block.statements = statements;
    node->block.count = count;
    return node;
}

static ASTNode *parse_let()
{
    Runtime *rt = get_runtime();
    parser_advance(); // skip 'let'

    if (rt->parser.current.type != TOKEN_IDENTIFIER)
    {
        PARSE_ERROR("[parse_let] Expected identifier after 'let'");
    }

    char *name = strdup(rt->parser.current.value);
    parser_advance();

    if (!match(TOKEN_EQUAL))
    {
        PARSE_ERROR("[parse_let] Expected '=' after variable name");
    }

    ASTNode *expr = parse_binary_expression(); // Save the raw expression

    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_LET;
    node->let_stmt.name = name;
    node->let_stmt.expr = expr;
    return node;
}

static ASTNode *parse_for()
{
    Runtime *rt = get_runtime();
    parser_advance(); // skip 'for'

    // Check for tuple syntax first: for (char, index) in string_var
    if (rt->parser.current.type == TOKEN_LPAREN)
    {
        parser_advance(); // skip '('
        
        if (rt->parser.current.type != TOKEN_IDENTIFIER)
        {
            PARSE_ERROR("[parse_for] Expected identifier for character variable, but got '%s'", rt->parser.current.value);
        }
        
        char *var = strdup(rt->parser.current.value);
        parser_advance();
        
        if (rt->parser.current.type != TOKEN_COMMA)
        {
            PARSE_ERROR("[parse_for] Expected ',' in tuple syntax, but got '%s'", rt->parser.current.value);
        }
        parser_advance(); // skip ','
        
        if (rt->parser.current.type != TOKEN_IDENTIFIER)
        {
            PARSE_ERROR("[parse_for] Expected identifier for index variable, but got '%s'", rt->parser.current.value);
        }
        
        char *index_var = strdup(rt->parser.current.value);
        parser_advance();
        
        if (rt->parser.current.type != TOKEN_RPAREN)
        {
            PARSE_ERROR("[parse_for] Expected ')' after tuple variables, but got '%s'", rt->parser.current.value);
        }
        parser_advance(); // skip ')'
        
        if (rt->parser.current.type != TOKEN_IN)
        {
            PARSE_ERROR("[parse_for] Expected 'in' after tuple syntax, but got '%s'", rt->parser.current.value);
        }
        parser_advance(); // skip 'in'
        
        // Parse the iterable expression
        ASTNode *iterable = parse_binary_expression();
        ASTNode *body = parse_block();

        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = AST_FOR;
        node->for_stmt.var = var;
        node->for_stmt.index_var = index_var;
        node->for_stmt.from = NULL;
        node->for_stmt.to = NULL;
        node->for_stmt.step = NULL;
        node->for_stmt.iterable = iterable;
        node->for_stmt.exclusive = 0;
        node->for_stmt.is_string_iteration = 1;
        node->for_stmt.body = body;
        return node;
    }

    // Regular identifier-based syntax
    if (rt->parser.current.type != TOKEN_IDENTIFIER)
    {
        PARSE_ERROR("[parse_for] Expected identifier after 'for', but got '%s'", rt->parser.current.value);
    }

    char *var = strdup(rt->parser.current.value);
    parser_advance();

    // Check for string iteration syntax: for char in string_var
    if (rt->parser.current.type == TOKEN_IN)
    {
        parser_advance(); // skip 'in'
        
        // Parse the iterable expression (should be a string variable)
        ASTNode *iterable = parse_binary_expression();
        ASTNode *body = parse_block();

        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = AST_FOR;
        node->for_stmt.var = var;
        node->for_stmt.index_var = NULL;
        node->for_stmt.from = NULL;
        node->for_stmt.to = NULL;
        node->for_stmt.step = NULL;
        node->for_stmt.iterable = iterable;
        node->for_stmt.exclusive = 0;
        node->for_stmt.is_string_iteration = 1;
        node->for_stmt.body = body;
        return node;
    }

    // Traditional numeric for loop: for i = 1..10
    if (!match(TOKEN_EQUAL))
    {
        PARSE_ERROR("[parse_for] Expected '=' after variable name in for loop, but got '%s'", rt->parser.current.value);
    }

    // Parse 'from' expression
    ASTNode *from = parse_binary_expression();

    int exclusive = 0;
    if (rt->parser.current.type == TOKEN_DOTDOT_LT)
    {
        exclusive = 1;
        parser_advance();
    }
    else if (rt->parser.current.type == TOKEN_DOTDOT)
    {
        parser_advance();
    }
    else
    {
        PARSE_ERROR("[parse_for] Expected '..' or '..<' in for loop range, but got '%s'", rt->parser.current.value);
    }

    // Parse 'to' expression
    ASTNode *to = parse_binary_expression();

    // Optional: support 'step' keyword
    ASTNode *step = NULL;
    if (rt->parser.current.type == TOKEN_STEP)
    {
        parser_advance();
        step = parse_binary_expression();
    }
    if (!step)
    {
        step = malloc(sizeof(ASTNode));
        step->type = AST_NUMBER;
        step->number.value = 1.0;
    }

    ///step5
    ASTNode *body = parse_block();

    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_FOR;
    node->for_stmt.var = var;
    node->for_stmt.index_var = NULL;
    node->for_stmt.from = from;
    node->for_stmt.to = to;
    node->for_stmt.step = step;
    node->for_stmt.iterable = NULL;
    node->for_stmt.exclusive = exclusive;
    node->for_stmt.is_string_iteration = 0;
    node->for_stmt.body = body;
    return node;
}

static ASTNode *parse_while()
{
    parser_advance(); // skip 'while'

    ASTNode *condition = parse_binary_expression(); // allow full expression

    ASTNode *body = parse_block(); // parse block after condition

    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_WHILE;
    node->while_stmt.condition = condition;
    node->while_stmt.body = body;

    return node;
}

static ASTNode *parse_note()
{
    Runtime *rt = get_runtime();
    parser_advance(); // skip 'note'

    // printf("[Parser] Saw 'note'\n");

    if (rt->parser.current.type != TOKEN_LBRACE)
    {
        PARSE_ERROR("[parse_note] Expected '{' after 'note'");
    }

    // Manually store brace start before advancing

    const char *start = rt->parser.lexer->source + rt->parser.lexer->pos;

    int start_pos = rt->parser.lexer->pos;

    // printf("[Parser] Found '{' at position %d\n", rt->parser.lexer->pos);

    parser_advance(); // consume '{'
    int brace_depth = 1;

    while (rt->parser.current.type != TOKEN_EOF && brace_depth > 0)
    {
        char c = rt->parser.lexer->source[rt->parser.lexer->pos];
        if (c == '{')
            brace_depth++;
        else if (c == '}')
            brace_depth--;
        
        // Properly track line numbers when encountering newlines
        if (c == '\n') {
            rt->parser.lexer->line++;
            rt->parser.lexer->column = 1;
        } else {
            rt->parser.lexer->column++;
        }
        
        rt->parser.lexer->pos++;
    }

    int len = rt->parser.lexer->pos - start_pos - 1; // exclude final '}'
    char *content = strndup_portable(start, len);

    // //printf("[Parser] Raw captured string (%d bytes): \"", len);
    // fwrite(content, 1, len, stdout);
    // //printf("\"\n");

    parser_advance(); // consume final '}'

    //  //printf("[Parser] Consumed '}'\n");

    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_NOTE;
    node->note.content = content;
    return node;
}

static ASTNode *parse_gcode()
{
    Runtime *rt = get_runtime();
    // Group GCODE words (like G1, G90, etc.)
    char line[256] = {0};
    int line_pos = 0;

    while (rt->parser.current.type == TOKEN_GCODE_WORD)
    {
        if (line_pos > 0)
            line[line_pos++] = ' '; // space between codes
        int len = snprintf(line + line_pos, sizeof(line) - line_pos, "%s", rt->parser.current.value);
        line_pos += len;
        parser_advance();
    }

    char *code = strdup(line);
    GArg *args = NULL;
    int count = 0, capacity = 0;

    while (rt->parser.current.type == TOKEN_IDENTIFIER)
    {
        // Allow only classic G-code letters (1-letter, uppercase: X, Y, Z, F, S, etc.)
        const char *key = rt->parser.current.value;

        if (strlen(key) != 1 || !isupper(key[0]))
        {
            // Not a classic G-code key, stop argument parsing
            //fprintf(stderr, "[parse_gcode] ⛔ Identifier '%s' not valid G-code key. Stopping G-code line.\n", key);
            break;
        }

        char *key_copy = strdup(key);
        parser_advance();

        ASTNode *index = NULL;
        if (rt->parser.current.type == TOKEN_LBRACKET)
        {
            parser_advance();
            index = parse_binary_expression();
            if (!match(TOKEN_RBRACKET))
            {
                PARSE_ERROR("[parse_gcode] Expected closing ']' for argument");
            }
        }

        // Expand storage
        if (count >= capacity)
        {
            capacity = capacity == 0 ? 4 : capacity * 2;
            args = realloc(args, capacity * sizeof(GArg));
        }

        args[count++] = (GArg){key_copy, index};

        // We stop adding args once we hit anything that’s not a valid G-code arg
    }

    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_GCODE;
    node->gcode_stmt.code = code;
    node->gcode_stmt.args = args;
    node->gcode_stmt.argCount = count;
    gcode_mode_active = 1;

    return node;
}

static ASTNode *parse_if()
{
    Runtime *rt = get_runtime();
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_IF;

    parser_advance(); // skip 'if'

    //  fprintf(stderr, "[Parser IF]\n");

    // Parse condition
    node->if_stmt.condition = parse_binary_expression();

    // Expect '{'
    if (rt->parser.current.type != TOKEN_LBRACE)
    {
        PARSE_ERROR("[parse_if] Expected '{' after if condition");
    }

    node->if_stmt.then_branch = parse_block(); // handles { ... }

    // Optional else
    if (rt->parser.current.type == TOKEN_ELSE)
    {
        parser_advance(); // consume 'else'
        if (rt->parser.current.type == TOKEN_IF)
        {
            node->if_stmt.else_branch = parse_if(); // recursively parse chained if
        }
        else if (rt->parser.current.type == TOKEN_LBRACE)
        {
            node->if_stmt.else_branch = parse_block();
        }
        else
        {
            PARSE_ERROR("[parse_if] Expected '{' or 'if' after else");
        }
    }

    else
    {
        node->if_stmt.else_branch = NULL;
    }

    return node;
}

void free_ast(ASTNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {

    case AST_NOP:
        // Nothing to free
        break;

    case AST_EMPTY:
        // Nothing to free
        break;


case AST_EXPR_STMT:
    free_ast(node->expr_stmt.expr);
    break;



    case AST_BINARY:
        free_ast(node->binary_expr.left);
        free_ast(node->binary_expr.right);
        break;

    case AST_ASSIGN_INDEX:
        free_ast(node->assign_index.target);
        free_ast(node->assign_index.value);
        break;

    case AST_FUNCTION:
        // Free function name
        if (node->function_stmt.name) {
            free(node->function_stmt.name);
        }
        // Free parameter names
        if (node->function_stmt.params) {
            for (int i = 0; i < node->function_stmt.param_count; i++) {
                if (node->function_stmt.params[i]) {
                    free(node->function_stmt.params[i]);
                }
            }
            free(node->function_stmt.params);
        }
        // Free function body
        if (node->function_stmt.body) {
            free_ast(node->function_stmt.body);
        }
        break;

    case AST_RETURN:
        free_ast(node->return_stmt.expr);
        break;
    case AST_LET:
        free(node->let_stmt.name);
        break;
    case AST_VAR:
        free(node->var.name);
        break;
    case AST_GCODE:
        free(node->gcode_stmt.code);
        for (int i = 0; i < node->gcode_stmt.argCount; i++)
        {
            free(node->gcode_stmt.args[i].key);
            free_ast(node->gcode_stmt.args[i].indexExpr);
        }
        free(node->gcode_stmt.args);
        break;
    case AST_CALL:
        free(node->call_expr.name);
        for (int i = 0; i < node->call_expr.arg_count; i++)
        {
            free_ast(node->call_expr.args[i]);
        }
        free(node->call_expr.args);
        break;

    case AST_NUMBER:
        // nothing to free
        break;
    case AST_STRING:
        free(node->string_literal.value);
        break;
    case AST_UNARY:
        free_ast(node->unary_expr.operand);
        break;

    case AST_WHILE:
        free_ast(node->while_stmt.condition);
        free_ast(node->while_stmt.body);
        break;

    case AST_NOTE:
        free(node->note.content);
        break;

    case AST_FOR:
        free(node->for_stmt.var);
        if (node->for_stmt.index_var) {
            free(node->for_stmt.index_var);
        }
        if (node->for_stmt.from) {
            free_ast(node->for_stmt.from);
        }
        if (node->for_stmt.to) {
            free_ast(node->for_stmt.to);
        }
        if (node->for_stmt.step) {
            free_ast(node->for_stmt.step);
        }
        if (node->for_stmt.iterable) {
            free_ast(node->for_stmt.iterable);
        }
        free_ast(node->for_stmt.body);
        break;

    case AST_BLOCK:
        for (int i = 0; i < node->block.count; i++)
        {
            ASTNode *stmt = node->block.statements[i];
            free_ast(stmt); // Free all statements including functions
        }
        free(node->block.statements);
        break;

    case AST_ARRAY_LITERAL:
        for (int i = 0; i < node->array_literal.count; i++)
        {
            free_ast(node->array_literal.elements[i]);
        }
        free(node->array_literal.elements);
        break;

    case AST_INDEX:
        free_ast(node->index_expr.array);
        free_ast(node->index_expr.index);
        break;

    case AST_IF:
        free_ast(node->if_stmt.condition);
        free_ast(node->if_stmt.then_branch);
        if (node->if_stmt.else_branch)
            free_ast(node->if_stmt.else_branch);
        break;
    case AST_ASSIGN:
        free(node->assign_stmt.name);
        free_ast(node->assign_stmt.expr);
        break;

    case AST_COMPOUND_ASSIGN:
        free(node->compound_assign.name);
        free_ast(node->compound_assign.expr);
        break;

    default:
        report_error("[free_ast] WARNING: Unhandled node type %d", node->type);
        break;
    }

    free(node);
}
// Parse an expression from a string while preserving current runtime context
ASTNode *parse_expression_from_string(const char *expr_text) {
    if (!expr_text || strlen(expr_text) == 0) {
        return NULL;
    }
    
    Runtime *rt = get_runtime();
    
    // Save current parser state
    Lexer *saved_lexer = rt->parser.lexer;
    Token saved_current = rt->parser.current;
    
    // Create temporary lexer for the expression
    rt->parser.lexer = lexer_new(expr_text);
    if (!rt->parser.lexer) {
        // Restore parser state
        rt->parser.lexer = saved_lexer;
        rt->parser.current = saved_current;
        return NULL;
    }
    
    // Advance to first token
    parser_advance();
    
    // Parse the expression
    ASTNode *expr = parse_binary_expression();
    
    // Clean up temporary lexer
    lexer_free(rt->parser.lexer);
    
    // Restore parser state
    rt->parser.lexer = saved_lexer;
    rt->parser.current = saved_current;
    
    return expr;
}