#include "Unity/src/unity.h"
#include "../src/lexer/lexer.h"
#include "../src/parser/parser.h"
#include "../src/runtime/evaluator.h"
#include "../src/lexer/token_utils.h"
#include "../src/config/config.h"
#include "../src/error/error.h"
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>

// External variables for error handling
extern jmp_buf fatal_error_jump_buffer;
extern int fatal_error_triggered;

int print_parser_logs = 0; // Set to 1 to enable debug output

typedef struct {
    ASTNode *root;
} ASTResult;

typedef struct {
    int error_occurred;
    char error_message[1024];
    ASTNode* result;
} ErrorTestResult;

extern Parser parser; // declare global parser object

ASTResult parse_source(const char *source) {
    ASTResult result;

    // Initialize runtime and set up parser
    init_runtime();
    Runtime *rt = get_runtime();
    
    Lexer *lexer = lexer_new(source);
    rt->parser.lexer = lexer;
    parser_advance();

    result.root = parse_script();
    lexer_free(lexer);
    return result;
}

ErrorTestResult test_parse_with_error_handling(const char* source) {
    ErrorTestResult result = {0};
    
    clear_errors();
    clear_fatal_state();
    
    // Parse the source - parse_script has its own setjmp that catches fatal errors
    result.result = parse_script_from_string(source);
    
    // Check if parsing failed (NULL result indicates error)
    if (result.result == NULL) {
        result.error_occurred = 1;
        const char* error_msg = get_error_messages();
        if (error_msg) {
            strncpy(result.error_message, error_msg, sizeof(result.error_message) - 1);
            result.error_message[sizeof(result.error_message) - 1] = '\0';
            free((void*)error_msg);
        }
    } else {
        result.error_occurred = 0;
    }
    
    return result;
}

void free_ast_result(ASTResult *result) {
    if (result->root) {
        free_ast(result->root);
        result->root = NULL;
    }
}

void setUp(void) {
    clear_errors();
    clear_fatal_state();
}

void tearDown(void) {
    clear_errors();
    clear_fatal_state();
}

// ============================================================================
// Test 1: Simple return statements with various expression types
// ============================================================================

void test_simple_return_with_number(void) {
    const char *source = "function test() { return 42 }";
    ASTResult result = parse_source(source);

    TEST_ASSERT_NOT_NULL(result.root);
    TEST_ASSERT_EQUAL(AST_BLOCK, result.root->type);
    TEST_ASSERT_EQUAL(1, result.root->block.count);

    ASTNode *func = result.root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_FUNCTION, func->type);
    TEST_ASSERT_EQUAL_STRING("test", func->function_def.name);

    ASTNode *body = func->function_def.body;
    TEST_ASSERT_EQUAL(AST_BLOCK, body->type);
    TEST_ASSERT_EQUAL(1, body->block.count);

    ASTNode *return_stmt = body->block.statements[0];
    TEST_ASSERT_EQUAL(AST_RETURN, return_stmt->type);
    TEST_ASSERT_NOT_NULL(return_stmt->return_stmt.expr);
    TEST_ASSERT_EQUAL(AST_NUMBER, return_stmt->return_stmt.expr->type);
    TEST_ASSERT_EQUAL_DOUBLE(42.0, return_stmt->return_stmt.expr->number.value);

    free_ast_result(&result);
}

void test_simple_return_with_variable(void) {
    const char *source = "function test() { return x }";
    ASTResult result = parse_source(source);

    TEST_ASSERT_NOT_NULL(result.root);
    ASTNode *func = result.root->block.statements[0];
    ASTNode *body = func->function_def.body;
    ASTNode *return_stmt = body->block.statements[0];
    
    TEST_ASSERT_EQUAL(AST_RETURN, return_stmt->type);
    TEST_ASSERT_NOT_NULL(return_stmt->return_stmt.expr);
    TEST_ASSERT_EQUAL(AST_VAR, return_stmt->return_stmt.expr->type);
    TEST_ASSERT_EQUAL_STRING("x", return_stmt->return_stmt.expr->var.name);

    free_ast_result(&result);
}

void test_simple_return_with_binary_expression(void) {
    const char *source = "function test() { return x + y }";
    ASTResult result = parse_source(source);

    TEST_ASSERT_NOT_NULL(result.root);
    ASTNode *func = result.root->block.statements[0];
    ASTNode *body = func->function_def.body;
    ASTNode *return_stmt = body->block.statements[0];
    
    TEST_ASSERT_EQUAL(AST_RETURN, return_stmt->type);
    TEST_ASSERT_NOT_NULL(return_stmt->return_stmt.expr);
    TEST_ASSERT_EQUAL(AST_BINARY, return_stmt->return_stmt.expr->type);
    TEST_ASSERT_EQUAL(TOKEN_PLUS, return_stmt->return_stmt.expr->binary_expr.op);

    free_ast_result(&result);
}

void test_simple_return_with_complex_expression(void) {
    const char *source = "function test() { return (x * y + z) / 2 }";
    ASTResult result = parse_source(source);

    TEST_ASSERT_NOT_NULL(result.root);
    ASTNode *func = result.root->block.statements[0];
    ASTNode *body = func->function_def.body;
    ASTNode *return_stmt = body->block.statements[0];
    
    TEST_ASSERT_EQUAL(AST_RETURN, return_stmt->type);
    TEST_ASSERT_NOT_NULL(return_stmt->return_stmt.expr);
    TEST_ASSERT_EQUAL(AST_BINARY, return_stmt->return_stmt.expr->type);

    free_ast_result(&result);
}

void test_simple_return_with_function_call(void) {
    const char *source = "function test() { return sqrt(x) }";
    ASTResult result = parse_source(source);

    TEST_ASSERT_NOT_NULL(result.root);
    ASTNode *func = result.root->block.statements[0];
    ASTNode *body = func->function_def.body;
    ASTNode *return_stmt = body->block.statements[0];
    
    TEST_ASSERT_EQUAL(AST_RETURN, return_stmt->type);
    TEST_ASSERT_NOT_NULL(return_stmt->return_stmt.expr);
    TEST_ASSERT_EQUAL(AST_CALL, return_stmt->return_stmt.expr->type);
    TEST_ASSERT_EQUAL_STRING("sqrt", return_stmt->return_stmt.expr->call_expr.name);

    free_ast_result(&result);
}

void test_bare_return_statement(void) {
    const char *source = "function test() { return }";
    ASTResult result = parse_source(source);

    TEST_ASSERT_NOT_NULL(result.root);
    ASTNode *func = result.root->block.statements[0];
    ASTNode *body = func->function_def.body;
    ASTNode *return_stmt = body->block.statements[0];
    
    TEST_ASSERT_EQUAL(AST_RETURN, return_stmt->type);
    TEST_ASSERT_NULL(return_stmt->return_stmt.expr); // Bare return has no expression

    free_ast_result(&result);
}

// ============================================================================
// Test 2: Return statements in different contexts (valid and invalid)
// ============================================================================

void test_return_in_if_statement(void) {
    const char *source = "function test() { if (x > 0) { return x } }";
    ASTResult result = parse_source(source);

    TEST_ASSERT_NOT_NULL(result.root);
    ASTNode *func = result.root->block.statements[0];
    ASTNode *body = func->function_def.body;
    ASTNode *if_stmt = body->block.statements[0];
    
    TEST_ASSERT_EQUAL(AST_IF, if_stmt->type);
    ASTNode *then_branch = if_stmt->if_stmt.then_branch;
    TEST_ASSERT_EQUAL(AST_BLOCK, then_branch->type);
    
    ASTNode *return_stmt = then_branch->block.statements[0];
    TEST_ASSERT_EQUAL(AST_RETURN, return_stmt->type);

    free_ast_result(&result);
}

void test_return_in_else_statement(void) {
    const char *source = "function test() { if (x > 0) { return x } else { return 0 } }";
    ASTResult result = parse_source(source);

    TEST_ASSERT_NOT_NULL(result.root);
    ASTNode *func = result.root->block.statements[0];
    ASTNode *body = func->function_def.body;
    ASTNode *if_stmt = body->block.statements[0];
    
    TEST_ASSERT_EQUAL(AST_IF, if_stmt->type);
    ASTNode *else_branch = if_stmt->if_stmt.else_branch;
    TEST_ASSERT_EQUAL(AST_BLOCK, else_branch->type);
    
    ASTNode *return_stmt = else_branch->block.statements[0];
    TEST_ASSERT_EQUAL(AST_RETURN, return_stmt->type);

    free_ast_result(&result);
}

void test_return_in_while_loop(void) {
    const char *source = "function test() { while (x > 0) { return x } }";
    ASTResult result = parse_source(source);

    TEST_ASSERT_NOT_NULL(result.root);
    ASTNode *func = result.root->block.statements[0];
    ASTNode *body = func->function_def.body;
    ASTNode *while_stmt = body->block.statements[0];
    
    TEST_ASSERT_EQUAL(AST_WHILE, while_stmt->type);
    ASTNode *while_body = while_stmt->while_stmt.body;
    TEST_ASSERT_EQUAL(AST_BLOCK, while_body->type);
    
    ASTNode *return_stmt = while_body->block.statements[0];
    TEST_ASSERT_EQUAL(AST_RETURN, return_stmt->type);

    free_ast_result(&result);
}

void test_return_in_for_loop(void) {
    const char *source = "function test() { for i = 0..10 { return i } }";
    ASTResult result = parse_source(source);

    TEST_ASSERT_NOT_NULL(result.root);
    ASTNode *func = result.root->block.statements[0];
    ASTNode *body = func->function_def.body;
    ASTNode *for_stmt = body->block.statements[0];
    
    TEST_ASSERT_EQUAL(AST_FOR, for_stmt->type);
    ASTNode *for_body = for_stmt->for_stmt.body;
    TEST_ASSERT_EQUAL(AST_BLOCK, for_body->type);
    
    ASTNode *return_stmt = for_body->block.statements[0];
    TEST_ASSERT_EQUAL(AST_RETURN, return_stmt->type);

    free_ast_result(&result);
}

void test_multiple_return_statements(void) {
    const char *source = 
        "function test() {\n"
        "  if (x > 0) {\n"
        "    return x\n"
        "  }\n"
        "  return 0\n"
        "}";
    ASTResult result = parse_source(source);

    TEST_ASSERT_NOT_NULL(result.root);
    ASTNode *func = result.root->block.statements[0];
    ASTNode *body = func->function_def.body;
    
    // Should have if statement and return statement
    TEST_ASSERT_EQUAL(2, body->block.count);
    
    ASTNode *if_stmt = body->block.statements[0];
    TEST_ASSERT_EQUAL(AST_IF, if_stmt->type);
    
    ASTNode *final_return = body->block.statements[1];
    TEST_ASSERT_EQUAL(AST_RETURN, final_return->type);

    free_ast_result(&result);
}

// ============================================================================
// Test 3: Error cases and error message accuracy
// ============================================================================

void test_return_outside_function_error(void) {
    const char *source = "return 42";
    ErrorTestResult result = test_parse_with_error_handling(source);

    TEST_ASSERT_TRUE(result.error_occurred);
    TEST_ASSERT_TRUE(strstr(result.error_message, "Return statement") != NULL);
    TEST_ASSERT_TRUE(strstr(result.error_message, "function") != NULL);
    
    if (result.result) {
        free_ast(result.result);
    }
}

void test_return_outside_function_with_context_error(void) {
    const char *source = 
        "let x = 5\n"
        "return x";
    ErrorTestResult result = test_parse_with_error_handling(source);

    TEST_ASSERT_TRUE(result.error_occurred);
    TEST_ASSERT_TRUE(strstr(result.error_message, "Return statement") != NULL);
    TEST_ASSERT_TRUE(strstr(result.error_message, "function") != NULL);
    
    if (result.result) {
        free_ast(result.result);
    }
}

void test_malformed_return_expression_operator_start(void) {
    const char *source = "function test() { return + }";
    ErrorTestResult result = test_parse_with_error_handling(source);

    TEST_ASSERT_TRUE(result.error_occurred);
    TEST_ASSERT_TRUE(strstr(result.error_message, "Invalid operator") != NULL ||
                     strstr(result.error_message, "operator") != NULL ||
                     strstr(result.error_message, "Unexpected token") != NULL);
    
    if (result.result) {
        free_ast(result.result);
    }
}

void test_malformed_return_expression_unmatched_paren(void) {
    const char *source = "function test() { return ) }";
    ErrorTestResult result = test_parse_with_error_handling(source);

    TEST_ASSERT_TRUE(result.error_occurred);
    TEST_ASSERT_TRUE(strstr(result.error_message, "Unexpected ')'") != NULL ||
                     strstr(result.error_message, "parenthesis") != NULL ||
                     strstr(result.error_message, "Unexpected token") != NULL);
    
    if (result.result) {
        free_ast(result.result);
    }
}

void test_malformed_return_expression_comma(void) {
    const char *source = "function test() { return , }";
    ErrorTestResult result = test_parse_with_error_handling(source);

    TEST_ASSERT_TRUE(result.error_occurred);
    TEST_ASSERT_TRUE(strstr(result.error_message, "Unexpected ','") != NULL ||
                     strstr(result.error_message, "comma") != NULL ||
                     strstr(result.error_message, "Unexpected token") != NULL);
    
    if (result.result) {
        free_ast(result.result);
    }
}

void test_malformed_return_expression_assignment(void) {
    const char *source = "function test() { return = }";
    ErrorTestResult result = test_parse_with_error_handling(source);

    TEST_ASSERT_TRUE(result.error_occurred);
    TEST_ASSERT_TRUE(strstr(result.error_message, "Unexpected '='") != NULL ||
                     strstr(result.error_message, "assignment") != NULL ||
                     strstr(result.error_message, "Unexpected token") != NULL);
    
    if (result.result) {
        free_ast(result.result);
    }
}

void test_multiple_expressions_in_return(void) {
    const char *source = "function test() { return 1 2 }";
    ErrorTestResult result = test_parse_with_error_handling(source);

    TEST_ASSERT_TRUE(result.error_occurred);
    TEST_ASSERT_TRUE(strstr(result.error_message, "Multiple expressions") != NULL ||
                     strstr(result.error_message, "multiple") != NULL);
    
    if (result.result) {
        free_ast(result.result);
    }
}

void test_return_with_line_number_accuracy(void) {
    const char *source = 
        "function test() {\n"    // Line 1
        "  let x = 5\n"          // Line 2
        "  return +\n"           // Line 3 - Error here
        "  let y = 10\n"         // Line 4
        "}";
    ErrorTestResult result = test_parse_with_error_handling(source);

    TEST_ASSERT_TRUE(result.error_occurred);
    // Check that line number 3 is mentioned in the error
    TEST_ASSERT_TRUE(strstr(result.error_message, "3:") != NULL ||
                     strstr(result.error_message, "line 3") != NULL);
    
    if (result.result) {
        free_ast(result.result);
    }
}

// ============================================================================
// Test 4: Complex return expressions similar to SVG arc function
// ============================================================================

void test_return_complex_math_expression(void) {
    const char *source = 
        "function test() {\n"
        "  return sqrt(rx * rx * sin_phi * sin_phi + ry * ry * cos_phi * cos_phi)\n"
        "}";
    ASTResult result = parse_source(source);

    TEST_ASSERT_NOT_NULL(result.root);
    ASTNode *func = result.root->block.statements[0];
    ASTNode *body = func->function_def.body;
    ASTNode *return_stmt = body->block.statements[0];
    
    TEST_ASSERT_EQUAL(AST_RETURN, return_stmt->type);
    TEST_ASSERT_NOT_NULL(return_stmt->return_stmt.expr);
    TEST_ASSERT_EQUAL(AST_CALL, return_stmt->return_stmt.expr->type);

    free_ast_result(&result);
}

void test_return_conditional_expression(void) {
    const char *source = 
        "function test() {\n"
        "  return x > 0 ? x : -x\n"
        "}";
    ASTResult result = parse_source(source);

    TEST_ASSERT_NOT_NULL(result.root);
    ASTNode *func = result.root->block.statements[0];
    ASTNode *body = func->function_def.body;
    ASTNode *return_stmt = body->block.statements[0];
    
    TEST_ASSERT_EQUAL(AST_RETURN, return_stmt->type);
    TEST_ASSERT_NOT_NULL(return_stmt->return_stmt.expr);
    TEST_ASSERT_EQUAL(AST_TERNARY, return_stmt->return_stmt.expr->type);

    free_ast_result(&result);
}

void test_return_array_access(void) {
    const char *source = 
        "function test() {\n"
        "  return points[i][0] + points[i][1]\n"
        "}";
    ASTResult result = parse_source(source);

    TEST_ASSERT_NOT_NULL(result.root);
    ASTNode *func = result.root->block.statements[0];
    ASTNode *body = func->function_def.body;
    ASTNode *return_stmt = body->block.statements[0];
    
    TEST_ASSERT_EQUAL(AST_RETURN, return_stmt->type);
    TEST_ASSERT_NOT_NULL(return_stmt->return_stmt.expr);
    TEST_ASSERT_EQUAL(AST_BINARY, return_stmt->return_stmt.expr->type);

    free_ast_result(&result);
}

void test_return_nested_function_calls(void) {
    const char *source = 
        "function test() {\n"
        "  return atan2(sin(angle), cos(angle))\n"
        "}";
    ASTResult result = parse_source(source);

    TEST_ASSERT_NOT_NULL(result.root);
    ASTNode *func = result.root->block.statements[0];
    ASTNode *body = func->function_def.body;
    ASTNode *return_stmt = body->block.statements[0];
    
    TEST_ASSERT_EQUAL(AST_RETURN, return_stmt->type);
    TEST_ASSERT_NOT_NULL(return_stmt->return_stmt.expr);
    TEST_ASSERT_EQUAL(AST_CALL, return_stmt->return_stmt.expr->type);
    TEST_ASSERT_EQUAL_STRING("atan2", return_stmt->return_stmt.expr->call_expr.name);

    free_ast_result(&result);
}

// ============================================================================
// Main test runner
// ============================================================================

int main(void) {
    UNITY_BEGIN();

    // Test 1: Simple return statements with various expression types
    RUN_TEST(test_simple_return_with_number);
    RUN_TEST(test_simple_return_with_variable);
    RUN_TEST(test_simple_return_with_binary_expression);
    RUN_TEST(test_simple_return_with_complex_expression);
    RUN_TEST(test_simple_return_with_function_call);
    RUN_TEST(test_bare_return_statement);

    // Test 2: Return statements in different contexts
    RUN_TEST(test_return_in_if_statement);
    RUN_TEST(test_return_in_else_statement);
    RUN_TEST(test_return_in_while_loop);
    RUN_TEST(test_return_in_for_loop);
    RUN_TEST(test_multiple_return_statements);

    // Test 3: Error cases and error message accuracy
    RUN_TEST(test_return_outside_function_error);
    RUN_TEST(test_return_outside_function_with_context_error);
    RUN_TEST(test_malformed_return_expression_operator_start);
    RUN_TEST(test_malformed_return_expression_unmatched_paren);
    RUN_TEST(test_malformed_return_expression_comma);
    RUN_TEST(test_malformed_return_expression_assignment);
    RUN_TEST(test_multiple_expressions_in_return);
    RUN_TEST(test_return_with_line_number_accuracy);

    // Test 4: Complex return expressions
    RUN_TEST(test_return_complex_math_expression);
    RUN_TEST(test_return_conditional_expression);
    RUN_TEST(test_return_array_access);
    RUN_TEST(test_return_nested_function_calls);

    return UNITY_END();
}