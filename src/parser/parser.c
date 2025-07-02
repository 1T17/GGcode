#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parser.h"
#include "../runtime/evaluator.h"



#define M_PI 3.14159265358979323846




static int gcode_mode_active = 0;
static Parser parser;

double eval_expr(ASTNode *node);

static ASTNode *parse_binary_expression();
static ASTNode *parse_block();
static ASTNode *parse_while();
static ASTNode *parse_for();
static ASTNode *parse_if();
static ASTNode *parse_let();
static ASTNode *parse_note();
static ASTNode *parse_gcode();
static ASTNode *parse_gcode_coord_only();
static ASTNode *parse_primary();
static ASTNode *parse_function();
static ASTNode *parse_return();
static ASTNode *parse_assignment();

// Forward declaration
Token lexer_peek_token(Lexer *lexer);




int get_precedence(TokenType op)
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
        return 0; // lowest precedence
    default:
        return -1;
    }
}

static void advance()
{
    parser.previous = parser.current;
    parser.current = lexer_next_token(parser.lexer);
}

static int match(TokenType type)
{
    if (parser.current.type == type)
    {
        advance();
        return 1;
    }
    return 0;
}

static ASTNode *parse_unary()
{
    if (parser.current.type == TOKEN_BANG)
    {
        TokenType op = parser.current.type;
        advance(); // consume '!'

        ASTNode *operand = parse_unary(); // recursive for !!x

        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = AST_UNARY;
        node->unary_expr.op = op;
        node->unary_expr.operand = operand;
        return node;
    }

    return parse_primary();
}

static ASTNode *parse_primary() {
    // Handle parentheses
    if (parser.current.type == TOKEN_LPAREN) {
        advance(); // consume '('
        ASTNode *expr = parse_binary_expression();
        if (!match(TOKEN_RPAREN)) {
            printf("[Parser] Expected ')' after expression\n");
            exit(1);
        }
        return expr;
    }

    // Handle unary minus
    if (parser.current.type == TOKEN_MINUS) {
        advance(); // consume '-'
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
        parser.current.type == TOKEN_FUNC_RAD_TO_DEG
    ) {
        double val = 0.0;
        switch (parser.current.type) {
            case TOKEN_FUNC_PI:         val = M_PI; break;
            case TOKEN_FUNC_TAU:        val = 2 * M_PI; break;
            case TOKEN_FUNC_EU:         val = 2.718281828459045; break;
            case TOKEN_FUNC_DEG_TO_RAD: val = M_PI / 180.0; break;
            case TOKEN_FUNC_RAD_TO_DEG: val = 180.0 / M_PI; break;
            default: break;
        }

        advance(); // consume the constant
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = AST_NUMBER;
        node->number.value = val;
        return node;
    }

    // Handle function calls and variables
    if (parser.current.type == TOKEN_IDENTIFIER ||
        (parser.current.type >= TOKEN_FUNC_ABS && parser.current.type <= TOKEN_FUNC_EXP)) {

        char *name = strdup(parser.current.value);
        advance(); // consume function or variable name

        if (parser.current.type == TOKEN_LPAREN) {
            advance(); // consume '('

            ASTNode **args = NULL;
            int arg_count = 0, arg_capacity = 0;

            while (parser.current.type != TOKEN_RPAREN) {
                if (arg_count >= arg_capacity) {
                    arg_capacity = (arg_capacity == 0) ? 4 : arg_capacity * 2;
                    args = realloc(args, arg_capacity * sizeof(ASTNode *));
                }
                args[arg_count++] = parse_binary_expression();
                if (parser.current.type == TOKEN_COMMA)
                    advance(); // consume comma
            }

            match(TOKEN_RPAREN); // consume ')'

            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = AST_CALL;
            node->call_expr.name = name;
            node->call_expr.args = args;
            node->call_expr.arg_count = arg_count;
            return node;
        } else {
            // Variable reference
            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = AST_VAR;
            node->var.name = name;
            return node;
        }
    }

    // Handle numeric constants
    if (parser.current.type == TOKEN_NUMBER) {
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = AST_NUMBER;
        node->number.value = atof(parser.current.value);
        advance(); // consume number
        return node;
    }

    // Fallback error
    printf("[Parser] Unexpected token in expression: %s\n", parser.current.value);
    exit(1);
}









static ASTNode *parse_binary_expression_prec(int min_prec)
{
    ASTNode *left = parse_unary();

    while (1)
    {
        TokenType op = parser.current.type;
        int prec = get_precedence(op);

        if (prec < min_prec)
            break;

        advance(); // consume operator

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
    advance(); // skip 'function'

    if (parser.current.type != TOKEN_IDENTIFIER)
    {
        printf("[Parser] Expected function name after 'function'\n");
        exit(1);
    }

    char *name = strdup(parser.current.value);
    advance();

    if (!match(TOKEN_LPAREN))
    {
        printf("[Parser] Expected '(' after function name\n");
        exit(1);
    }

    char **params = NULL;
    int param_count = 0, param_capacity = 0;

    while (parser.current.type != TOKEN_RPAREN)
    {
        if (parser.current.type != TOKEN_IDENTIFIER)
        {
            printf("[Parser] Expected parameter name\n");
            exit(1);
        }

        if (param_count >= param_capacity)
        {
            param_capacity = param_capacity == 0 ? 4 : param_capacity * 2;
            params = realloc(params, param_capacity * sizeof(char *));
        }
        params[param_count++] = strdup(parser.current.value);
        advance();

        if (parser.current.type == TOKEN_COMMA)
            advance();
    }
    advance(); // consume ')'

    ASTNode *body = parse_block();

    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_FUNCTION;
    node->function_stmt.name = name;
    node->function_stmt.params = params;
    node->function_stmt.param_count = param_count;
    node->function_stmt.body = body;
    return node;
}

static ASTNode *parse_return()
{
    advance(); // skip 'return'
    ASTNode *expr = parse_binary_expression();
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_RETURN;
    node->return_stmt.expr = expr;
    return node;
}



static ASTNode *parse_assignment() {
    // Assume current token is TOKEN_IDENTIFIER
    char *name = strdup(parser.current.value);
    advance(); // consume identifier

    if (!match(TOKEN_EQUAL)) {
        printf("[Parser] Expected '=' after identifier in assignment\n");
        exit(1);
    }

    // ❌ DO NOT add another advance here

    ASTNode *expr = parse_binary_expression();

    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_ASSIGN;
    node->assign_stmt.name = name;
    node->assign_stmt.expr = expr;
    return node;
}



static ASTNode *parse_statement()
{
    //printf("[DEBUG] parse_statement: token=%s (type=%d) line=%d\n",
     //      parser.current.value, parser.current.type, parser.current.line);

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

    // Identifier-based logic
    if (parser.current.type == TOKEN_IDENTIFIER) {
     //   printf("[DEBUG] parse_statement: identifier '%s', gcode_mode_active=%d\n",
            //   parser.current.value, gcode_mode_active);

        Token lookahead = lexer_peek_token(parser.lexer);  // ✅ Use peek, not advance

        if (lookahead.type == TOKEN_EQUAL) {
        //    printf("[DEBUG] parse_statement: Detected assignment for '%s'\n", parser.current.value);
            return parse_assignment();
        } else if (gcode_mode_active) {
        //    printf("[DEBUG] parse_statement: Detected G-code coordinate for '%s'\n", parser.current.value);
            return parse_gcode_coord_only();
        } else {
         //   printf("[DEBUG] parse_statement: Detected primary/expression for '%s'\n", parser.current.value);
            return parse_primary();
        }
    }

    // G-code command like G0, G1, etc.
    if (parser.current.type == TOKEN_GCODE_WORD) {
      //  printf("[DEBUG] parse_statement: Detected G-code word '%s'\n", parser.current.value);
        return parse_gcode();
    }

    // Fallback error
    printf("[Parser] Unexpected token: %s (type %d) at line %d\n",
           parser.current.value, parser.current.type, parser.current.line);
    exit(1);
}



static ASTNode *parse_block()
{
    if (!match(TOKEN_LBRACE))
    {
        printf("[Parser] Expected '{'\n");
        exit(1);
    }

    ASTNode **statements = NULL;
    int capacity = 0, count = 0;

    while (!match(TOKEN_RBRACE))
    {
        if (parser.current.type == TOKEN_EOF)
        {
            printf("[Parser] Unexpected EOF in block\n");
            exit(1);
        }

        if (count >= capacity)
        {
            capacity = capacity == 0 ? 4 : capacity * 2;
            statements = realloc(statements, capacity * sizeof(ASTNode *));
        }

        statements[count++] = parse_statement();

        // Skip any newlines between statements in a block
        while (parser.current.type == TOKEN_NEWLINE)
            advance();
    }

    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_BLOCK;
    node->block.statements = statements;
    node->block.count = count;
    return node;
}

static ASTNode *parse_while()
{

    // printf("[DEBUG] Inside parse_while. Current token before advance: type=%d, value='%s'\n",parser.current.type, parser.current.value);

    advance(); // skip 'while'

    ASTNode *condition = parse_binary_expression(); // allow full expression

    ASTNode *body = parse_block(); // parse block after condition

    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_WHILE;
    node->while_stmt.condition = condition;
    node->while_stmt.body = body;

    return node;
}

static ASTNode *parse_let()
{
    advance(); // skip 'let'

    if (parser.current.type != TOKEN_IDENTIFIER)
    {
        printf("[Parser] Expected identifier after 'let'\n");
        exit(1);
    }

    char *name = strdup(parser.current.value);
    advance();

    if (!match(TOKEN_EQUAL))
    {
        printf("[Parser] Expected '=' after variable name\n");
        exit(1);
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
    advance(); // skip 'for'

    if (parser.current.type != TOKEN_IDENTIFIER)
    {
        printf("[Parser] Expected identifier after 'for'\n");
        exit(1);
    }

    char *var = strdup(parser.current.value);
    advance();

    if (!match(TOKEN_EQUAL))
    {
        printf("[Parser] Expected '=' after variable name in for loop\n");
        exit(1);
    }

    if (parser.current.type != TOKEN_NUMBER)
    {
        printf("[Parser] Expected number after '=' in for loop\n");
        exit(1);
    }

    double from = atof(parser.current.value);
    advance();

    if (!match(TOKEN_DOTDOT))
    {
        printf("[Parser] Expected '..' in for loop range\n");
        exit(1);
    }

    if (parser.current.type != TOKEN_NUMBER)
    {
        printf("[Parser] Expected number after '..' in for loop\n");
        exit(1);
    }

    double to = atof(parser.current.value);
    advance();

    ASTNode *body = parse_block();

    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_FOR;
    node->for_stmt.var = var;
    node->for_stmt.from = from;
    node->for_stmt.to = to;
    node->for_stmt.body = body;
    return node;
}

static ASTNode *parse_note()
{
    advance(); // skip 'note'

    // printf("[Parser] Saw 'note'\n");

    if (parser.current.type != TOKEN_LBRACE)
    {
        printf("[Parser] Expected '{' after 'note'\n");
        exit(1);
    }

    // Manually store brace start before advancing

    const char *start = parser.lexer->source + parser.lexer->pos;

    int start_pos = parser.lexer->pos;

    // printf("[Parser] Found '{' at position %d\n", parser.lexer->pos);

    advance(); // consume '{'
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
    char *content = strndup(start, len);

    // printf("[Parser] Raw captured string (%d bytes): \"", len);
    // fwrite(content, 1, len, stdout);
    // printf("\"\n");

    advance(); // consume final '}'

    //  printf("[Parser] Consumed '}'\n");

    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_NOTE;
    node->note.content = content;
    return node;
}











// Add this to lexer.c or a shared header
Token lexer_peek_token(Lexer *lexer) {
    int saved_pos = lexer->pos;
    int saved_column = lexer->column;

    Token next = lexer_next_token(lexer);

    lexer->pos = saved_pos;
    lexer->column = saved_column;
    return next;
}









static ASTNode *parse_gcode()
{
    // Start grouping multiple GCODE words from the same line
    char line[256] = {0};
    int line_pos = 0;

    while (parser.current.type == TOKEN_GCODE_WORD)
    {
        if (line_pos > 0)
            line[line_pos++] = ' '; // add space between tokens
        int len = snprintf(line + line_pos, sizeof(line) - line_pos, "%s", parser.current.value);
        line_pos += len;
        advance();
    }
    char *code = strdup(line);

    GArg *args = NULL;
    int count = 0, capacity = 0;

    while (parser.current.type == TOKEN_IDENTIFIER)
    {
        Token lookahead = lexer_peek_token(parser.lexer);
        if (lookahead.type == TOKEN_EQUAL)
            break;

        char *key = strdup(parser.current.value);
        advance();

        ASTNode *index = NULL;
        if (parser.current.type == TOKEN_LBRACKET)
        {
            advance();
            index = parse_binary_expression();
            if (!match(TOKEN_RBRACKET))
            {
                printf("[Parser] Expected ']'\n");
                exit(1);
            }
        }

        if (count >= capacity)
        {
            capacity = capacity == 0 ? 4 : capacity * 2;
            args = realloc(args, capacity * sizeof(GArg));
        }
        args[count++] = (GArg){key, index};

        if (parser.current.type == TOKEN_NEWLINE || parser.current.type == TOKEN_EOF)
            break;
    }

    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_GCODE;
    node->gcode_stmt.code = code;
    node->gcode_stmt.args = args;
    node->gcode_stmt.argCount = count;
    gcode_mode_active = 1;

    return node;
}








static ASTNode *parse_gcode_coord_only()
{
    GArg *args = NULL;
    int count = 0, capacity = 0;

    while (parser.current.type == TOKEN_IDENTIFIER)
    {
        Token lookahead = lexer_peek_token(parser.lexer);
        if (lookahead.type == TOKEN_EQUAL)
            break;

        char *key = strdup(parser.current.value);
        advance();

        ASTNode *index = NULL;
        if (parser.current.type == TOKEN_LBRACKET)
        {
            advance();
            index = parse_binary_expression();
            if (!match(TOKEN_RBRACKET))
            {
                printf("[Parser] Expected ']'\n");
                exit(1);
            }
        }

        if (count >= capacity)
        {
            capacity = capacity == 0 ? 4 : capacity * 2;
            args = realloc(args, capacity * sizeof(GArg));
        }
        args[count++] = (GArg){key, index};

        if (parser.current.type == TOKEN_NEWLINE || parser.current.type == TOKEN_EOF)
            break;
    }

    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_GCODE;
    node->gcode_stmt.code = strdup("G1");
    node->gcode_stmt.args = args;
    node->gcode_stmt.argCount = count;
    return node;
}















static ASTNode *parse_if()
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_IF;

    advance(); // skip 'if'

    // Parse condition
    node->if_stmt.condition = parse_binary_expression();

    // Expect '{'
    if (parser.current.type != TOKEN_LBRACE)
    {
        fprintf(stderr, "[Parser] Expected '{' after if condition\n");
        exit(1);
    }

    node->if_stmt.then_branch = parse_block(); // handles { ... }

    // Optional else
    if (parser.current.type == TOKEN_ELSE)
    {
        advance(); // consume 'else'
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
            fprintf(stderr, "[Parser] Expected '{' or 'if' after else\n");
            exit(1);
        }
    }

    else
    {
        node->if_stmt.else_branch = NULL;
    }

    return node;
}

ASTNode *parse_script(const char *source)
{
    parser.lexer = lexer_new(source);
    advance();

    ASTNode **statements = NULL;
    int capacity = 0, count = 0;

    while (parser.current.type != TOKEN_EOF)
    {
        if (count >= capacity)
        {
            capacity = capacity == 0 ? 4 : capacity * 2;
            statements = realloc(statements, capacity * sizeof(ASTNode *));
        }

        statements[count++] = parse_statement();

        // Skip any newlines between statements
        while (parser.current.type == TOKEN_NEWLINE)
            advance();
    }

    ASTNode *root = malloc(sizeof(ASTNode));
    root->type = AST_BLOCK;
    root->block.statements = statements;
    root->block.count = count;
    return root;
}




void free_ast(ASTNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case AST_BINARY:
        free_ast(node->binary_expr.left);
        free_ast(node->binary_expr.right);
        break;

    case AST_FUNCTION:
        free(node->function_stmt.name);
        for (int i = 0; i < node->function_stmt.param_count; i++)
        {
            free(node->function_stmt.params[i]);
        }
        free(node->function_stmt.params);
        free_ast(node->function_stmt.body);
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
        for (int i = 0; i < node->call_expr.arg_count; i++) {
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
            free_ast(node->block.statements[i]);
        }
        free(node->block.statements);
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
     fprintf(stderr, "[free_ast] WARNING: Unhandled node type %d\n", node->type);
        break;
    }

    free(node);
}
