#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parser.h"
#include "../utils/compat.h"
#include "../runtime/evaluator.h"
#include "error/error.h"
#include <ctype.h>
#include <setjmp.h>
#define M_PI 3.14159265358979323846
#define PARSE_ERROR(msg, ...) \
    fatal_error(parser.lexer->source, parser.current.line, parser.current.column, msg, ##__VA_ARGS__)

static int gcode_mode_active = 0;
Parser parser;

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
Token lexer_peek_token(Lexer *lexer);


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

    //printf("[DEBUG parse_script step] Entering parse_script()\n");

    while (parser.current.type != TOKEN_EOF) {
        if (parser.current.type == TOKEN_NEWLINE) {
            parser_advance();
            continue;
        }



       /// step 3

        ASTNode *stmt = parse_statement();

        if (!stmt) {
            report_error("[parse_script] Skipping NULL stmt");
            continue;
        }

        //printf("[DEBUG] Parsed stmt type: %d\n", stmt->type);

if (stmt->type == AST_EMPTY || stmt->type == AST_NOP)
    continue;


        if (count >= capacity) {
            int new_capacity = (capacity == 0) ? 4 : capacity * 2;
         //   //printf("[DEBUG] Growing statement array: %d → %d\n", capacity, new_capacity);
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

  //printf("[DEBUG] Finished loop, count = %d\n", count);

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
    case TOKEN_STAR:
    case TOKEN_SLASH:
        return 3;
    case TOKEN_PLUS:
    case TOKEN_MINUS:
        return 2;
    case TOKEN_LESS:
    case TOKEN_LESS_EQUAL:
    case TOKEN_GREATER:
    case TOKEN_GREATER_EQUAL:
    case TOKEN_EQUAL_EQUAL:
    case TOKEN_BANG_EQUAL:
        return 1;
    case TOKEN_AND:
    case TOKEN_OR:
    case TOKEN_AMPERSAND: // <-- Add this line
        return 0;         // lowest precedence
    default:
        return -1;
    }
}

void parser_advance()
{
    // printf("[Parser parser_advance()] calling lexer_next_token...\n");
    parser.previous = parser.current;
    parser.current = lexer_next_token(parser.lexer);
    // printf("[Parser parser_advance()] got token: type=%d, value=%s\n", parser.current.type, parser.current.value);
}

static int match(Token_Type type)
{
    if (parser.current.type == type)
    {
        parser_advance();
        return 1;
    }
    return 0;
}

static ASTNode *parse_unary()
{
    if (parser.current.type == TOKEN_BANG || parser.current.type == TOKEN_MINUS)
    {
        Token op = parser.current;
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


    // Handle parentheses
if (parser.current.type == TOKEN_LPAREN)
{
    //printf("[parse_primary] 🌀 Detected '('\n");
    parser_advance(); // consume '('
    //printf("[parse_primary] ⏬ Parsing inner expression...\n");

    // Handle empty parentheses like: ()
    if (parser.current.type == TOKEN_RPAREN) {
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
        report_error("[parse_primary] Expected ')' after expression, but got '%s'", parser.current.value);
        return NULL;
    }

    //printf("[parse_primary] ✅ Matched closing ')', returning inner expression AST\n");
    return expr;
}








    // Handle unary minus
    if (parser.current.type == TOKEN_MINUS)
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
        parser.current.type == TOKEN_FUNC_PI ||
        parser.current.type == TOKEN_FUNC_TAU ||
        parser.current.type == TOKEN_FUNC_EU ||
        parser.current.type == TOKEN_FUNC_DEG_TO_RAD ||
        parser.current.type == TOKEN_FUNC_RAD_TO_DEG)
    {
        double val = 0.0;
        switch (parser.current.type)
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
    if (parser.current.type == TOKEN_IDENTIFIER ||
        (parser.current.type >= TOKEN_FUNC_ABS && parser.current.type <= TOKEN_FUNC_EXP))
    {

        char *name = strdup(parser.current.value);
        //fprintf(stderr, "[Parser] Detected identifier or function name: '%s'\n", name);

        parser_advance(); // consume function or variable name

        // Check for function call
        if (parser.current.type == TOKEN_LPAREN)
        {
           // fprintf(stderr, "[Parser] Detected function call: '%s(...)'\n", name);
            parser_advance(); // consume '('

            ASTNode **args = NULL;
            int arg_count = 0, arg_capacity = 0;

            while (parser.current.type != TOKEN_RPAREN)
            {
                if (arg_count >= arg_capacity)
                {
                    arg_capacity = (arg_capacity == 0) ? 4 : arg_capacity * 2;
                    args = realloc(args, arg_capacity * sizeof(ASTNode *));
                }

                //fprintf(stderr, "[Parser] Parsing argument %d for '%s'\n", arg_count + 1, name);
                args[arg_count++] = parse_binary_expression();

                if (parser.current.type == TOKEN_COMMA)
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
    if (parser.current.type == TOKEN_NUMBER)
    {
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = AST_NUMBER;
        node->number.value = atof(parser.current.value);
        parser_advance(); // consume number
        return node;
    }

    // Handle array literal: [1, 2, 3] or [[1, 2], [3, 4]]
    if (parser.current.type == TOKEN_LBRACKET)
    {
        parser_advance(); // consume '['

        ASTNode **elements = NULL;
        int count = 0, capacity = 0;

        // Handle non-empty array
        if (parser.current.type != TOKEN_RBRACKET)
        {
            while (1)
            {
                if (count >= capacity)
                {
                    capacity = capacity == 0 ? 4 : capacity * 2;
                    elements = realloc(elements, capacity * sizeof(ASTNode *));
                }

                elements[count++] = parse_binary_expression();

                if (parser.current.type == TOKEN_COMMA)
                {
                    parser_advance(); // consume comma and continue
                }
                else if (parser.current.type == TOKEN_RBRACKET)
                {
                    break; // closing bracket
                }
                else
                {
                    report_error("[Parser] Expected ',' or ']' in array literal, found '%s'", parser.current.value);
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
        while (parser.current.type == TOKEN_LBRACKET)
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

            PARSE_ERROR("[parse_primary end] Unexpected token in expression: '%s'", parser.current.value);




    // Return dummy NOP node to avoid crashing
    ASTNode *dummy = malloc(sizeof(ASTNode));
    dummy->type = AST_NOP;
    return dummy;
}

static ASTNode *parse_postfix_expression()
{
    ASTNode *expr = parse_primary();

    while (parser.current.type == TOKEN_LBRACKET)
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

    ASTNode *left = parse_unary();

    while (1)
    {
        Token_Type op = parser.current.type;
        int prec = get_precedence(op);

        if (prec < min_prec)
            break;

        parser_advance(); // consume operator

        ASTNode *right = parse_binary_expression_prec(prec + 1);

        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = AST_BINARY;
        node->binary_expr.op = op;
        node->binary_expr.left = left;
        node->binary_expr.right = right;

        left = node;
    }

    return left;
}

static ASTNode *parse_binary_expression()
{
    return parse_binary_expression_prec(0);
}

static ASTNode *parse_function()
{
    parser_advance(); // Skip 'function' keyword

    if (parser.current.type != TOKEN_IDENTIFIER)
    {
        PARSE_ERROR("Expected function name after 'function'");
    }

    char *name = strdup(parser.current.value); // Save function name
    parser_advance();                          // Consume name

    if (!match(TOKEN_LPAREN))
    {
        PARSE_ERROR("Expected '(' after function name");
    }

    // Handle parameter list
    char **params = NULL;
    int param_count = 0, param_capacity = 0;

    while (parser.current.type != TOKEN_RPAREN)
    {
        if (parser.current.type != TOKEN_IDENTIFIER)
        {
            PARSE_ERROR("Expected parameter name, got '%s'", parser.current.value);
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

        params[param_count++] = strdup(parser.current.value);
        parser_advance(); // Consume parameter name

        if (parser.current.type == TOKEN_COMMA)
        {
            parser_advance(); // Consume comma
        }
        else if (parser.current.type != TOKEN_RPAREN)
        {
            PARSE_ERROR("Expected ',' or ')' in parameter list");
        }
    }

    parser_advance(); // Consume ')'

    // Parse function body block
    ASTNode *body = parse_block();

    // Build function node
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = AST_FUNCTION;
    node->function_stmt.name = name;
    node->function_stmt.params = params;
    node->function_stmt.param_count = param_count;
    node->function_stmt.body = body;

    if (body)
        body->parent = node;

#ifdef PARSER_DEBUG
    printf("[Parser] Parsed function: %s with %d params\n", name, param_count);
#endif

    return node;
}

static ASTNode *parse_return()
{
    parser_advance(); // skip 'return'
    ASTNode *expr = parse_binary_expression();
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_RETURN;
    node->return_stmt.expr = expr;
    return node;
}

static ASTNode *parse_statement()
{
    if (parser.current.type == TOKEN_NEWLINE || parser.current.type == TOKEN_EOF)
        return NULL;

    // Keyword-based statements
    if (parser.current.type == TOKEN_FUNCTION)
        return parse_function();
    if (parser.current.type == TOKEN_RETURN)
        return parse_return();
    if (parser.current.type == TOKEN_WHILE)
        return parse_while();
    if (parser.current.type == TOKEN_FOR)
        return parse_for();
    if (parser.current.type == TOKEN_IF)
        return parse_if();
    if (parser.current.type == TOKEN_LET)
        return parse_let();
    if (parser.current.type == TOKEN_NOTE)
        return parse_note();

    // Identifier-based logic (e.g., x = 1, maze[i][j] = 0, or foo(1))
    if (parser.current.type == TOKEN_IDENTIFIER)
    {
        return parse_identifier_statement();
    }

    // G-code commands like G1, G0, M3, etc.
    if (parser.current.type == TOKEN_GCODE_WORD)
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
    if (parser.current.type == TOKEN_EQUAL)
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

    while (!match(TOKEN_RBRACE))
    {

        if (parser.current.type == TOKEN_EOF)
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
            report_error("[DEBUG] Skipping NULL stmt");
        }

        // Skip any newlines between statements in a block
        while (parser.current.type == TOKEN_NEWLINE)
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
    parser_advance(); // skip 'let'

    if (parser.current.type != TOKEN_IDENTIFIER)
    {
        PARSE_ERROR("[parse_let] Expected identifier after 'let'");
    }

    char *name = strdup(parser.current.value);
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
    parser_advance(); // skip 'for'

    if (parser.current.type != TOKEN_IDENTIFIER)
    {
        PARSE_ERROR("[parse_for] Expected identifier after 'for', but got '%s'", parser.current.value);
    }

    char *var = strdup(parser.current.value);
    parser_advance();

    if (!match(TOKEN_EQUAL))
    {
        PARSE_ERROR("[parse_for] Expected '=' after variable name in for loop, but got '%s'", parser.current.value);
    }

    // Parse 'from' expression
    ASTNode *from = parse_binary_expression();

    int exclusive = 0;
    if (parser.current.type == TOKEN_DOTDOT_LT)
    {
        exclusive = 1;
        parser_advance();
    }
    else if (parser.current.type == TOKEN_DOTDOT)
    {
        parser_advance();
    }
    else
    {
        PARSE_ERROR("[parse_for] Expected '..' or '..<' in for loop range, but got '%s'", parser.current.value);
    }

    // Parse 'to' expression
    ASTNode *to = parse_binary_expression();

    // Optional: support 'step' keyword
    ASTNode *step = NULL;
    if (parser.current.type == TOKEN_STEP)
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
    node->for_stmt.from = from;
    node->for_stmt.to = to;
    node->for_stmt.step = step; // can be NULL (default to 1 later)
    node->for_stmt.exclusive = exclusive;
    node->for_stmt.body = body;
    return node;
}

static ASTNode *parse_while()
{

    // printf("[DEBUG] Inside parse_while. Current token before parser_advance: type=%d, value='%s'\n",parser.current.type, parser.current.value);

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
    parser_advance(); // skip 'note'

    // printf("[Parser] Saw 'note'\n");

    if (parser.current.type != TOKEN_LBRACE)
    {
        PARSE_ERROR("[parse_note] Expected '{' after 'note'");
    }

    // Manually store brace start before advancing

    const char *start = parser.lexer->source + parser.lexer->pos;

    int start_pos = parser.lexer->pos;

    // printf("[Parser] Found '{' at position %d\n", parser.lexer->pos);

    parser_advance(); // consume '{'
    int brace_depth = 1;

    while (parser.current.type != TOKEN_EOF && brace_depth > 0)
    {
        char c = parser.lexer->source[parser.lexer->pos];
        if (c == '{')
            brace_depth++;
        else if (c == '}')
            brace_depth--;
        parser.lexer->pos++;
        parser.lexer->column++;
    }

    int len = parser.lexer->pos - start_pos - 1; // exclude final '}'
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
    // Group GCODE words (like G1, G90, etc.)
    char line[256] = {0};
    int line_pos = 0;

    while (parser.current.type == TOKEN_GCODE_WORD)
    {
        if (line_pos > 0)
            line[line_pos++] = ' '; // space between codes
        int len = snprintf(line + line_pos, sizeof(line) - line_pos, "%s", parser.current.value);
        line_pos += len;
        parser_advance();
    }

    char *code = strdup(line);
    GArg *args = NULL;
    int count = 0, capacity = 0;

    while (parser.current.type == TOKEN_IDENTIFIER)
    {
        // Allow only classic G-code letters (1-letter, uppercase: X, Y, Z, F, S, etc.)
        const char *key = parser.current.value;

        if (strlen(key) != 1 || !isupper(key[0]))
        {
            // Not a classic G-code key, stop argument parsing
            //fprintf(stderr, "[parse_gcode] ⛔ Identifier '%s' not valid G-code key. Stopping G-code line.\n", key);
            break;
        }

        char *key_copy = strdup(key);
        parser_advance();

        ASTNode *index = NULL;
        if (parser.current.type == TOKEN_LBRACKET)
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
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_IF;

    parser_advance(); // skip 'if'

    //  fprintf(stderr, "[Parser IF]\n");

    // Parse condition
    node->if_stmt.condition = parse_binary_expression();

    // Expect '{'
    if (parser.current.type != TOKEN_LBRACE)
    {
        PARSE_ERROR("[parse_if] Expected '{' after if condition");
    }

    node->if_stmt.then_branch = parse_block(); // handles { ... }

    // Optional else
    if (parser.current.type == TOKEN_ELSE)
    {
        parser_advance(); // consume 'else'
        if (parser.current.type == TOKEN_IF)
        {
            node->if_stmt.else_branch = parse_if(); // recursively parse chained if
        }
        else if (parser.current.type == TOKEN_LBRACE)
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
        //fprintf(stderr, "[free_ast] Skipping function: %s\n", node->function_stmt.name);
        return;

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
        free_ast(node->for_stmt.body);
        break;

    case AST_BLOCK:
        for (int i = 0; i < node->block.count; i++)
        {
            ASTNode *stmt = node->block.statements[i];
            if (stmt->type != AST_FUNCTION)
            {
                free_ast(stmt); // only free non-function nodes
            }
            // else: skip function — it's owned by the registry and already freed
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

    default:
        report_error("[free_ast] WARNING: Unhandled node type %d", node->type);
        break;
    }

    free(node);
}

Token lexer_peek_token(Lexer *lexer)
{
    int saved_pos = lexer->pos;
    int saved_column = lexer->column;

    Token next = lexer_next_token(lexer);

    lexer->pos = saved_pos;
    lexer->column = saved_column;
    return next;
}
