

#include "Unity/src/unity.h"
#include "../src/lexer/lexer.h"
#include "../src/parser/parser.h"
#include "../src/lexer/token_utils.h"
#include "../include/config.h"
#include <string.h>

int print_parser_logs = 0;

typedef struct {
    ASTNode* root;
} ASTResult;



ASTResult parse_source(const char* source) {
    ASTResult result;
    result.root = parse_script(source);
    return result;
}



void print_indent(int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
}

void print_ast_node(ASTNode* node, int indent) {
    if (!node) return;

    print_indent(indent);

    switch (node->type) {
        case AST_NOP:
            printf("NOP\n");
            break;

        case AST_NUMBER:
            printf("Number: %g\n", node->number.value);
            break;

        case AST_VAR:
            printf("Var: %s\n", node->var.name);
            break;

        case AST_LET:
            printf("Let: %s =\n", node->let_stmt.name);
            print_ast_node(node->let_stmt.expr, indent + 1);
            break;

        case AST_BINARY:
            printf("BinaryExpr: %d\n", node->binary_expr.op);
            print_ast_node(node->binary_expr.left, indent + 1);
            print_ast_node(node->binary_expr.right, indent + 1);
            break;

        case AST_UNARY:
            printf("UnaryExpr: %d\n", node->unary_expr.op);
            print_ast_node(node->unary_expr.operand, indent + 1);
            break;

        case AST_INDEX:
            printf("IndexExpr:\n");
            print_ast_node(node->index_expr.array, indent + 1);
            print_ast_node(node->index_expr.index, indent + 1);
            break;

        case AST_GCODE:
            printf("GCode: %s\n", node->gcode_stmt.code);
            for (int i = 0; i < node->gcode_stmt.argCount; i++) {
                print_indent(indent + 1);
                printf("Arg %s:\n", node->gcode_stmt.args[i].key);
                print_ast_node(node->gcode_stmt.args[i].indexExpr, indent + 2);
            }
            break;

        case AST_NOTE:
            printf("Note: %s\n", node->note.content);
            break;

        case AST_BLOCK:
            printf("Block:\n");
            for (int i = 0; i < node->block.count; i++) {
                print_ast_node(node->block.statements[i], indent + 1);
            }
            break;

        case AST_IF:
            printf("If:\n");
            print_indent(indent + 1); printf("Condition:\n");
            print_ast_node(node->if_stmt.condition, indent + 2);
            print_indent(indent + 1); printf("Then:\n");
            print_ast_node(node->if_stmt.then_branch, indent + 2);
            if (node->if_stmt.else_branch) {
                print_indent(indent + 1); printf("Else:\n");
                print_ast_node(node->if_stmt.else_branch, indent + 2);
            }
            break;

        case AST_FOR:
            printf("For %s = %g..%g\n", node->for_stmt.var, node->for_stmt.from, node->for_stmt.to);
            print_ast_node(node->for_stmt.body, indent + 1);
            break;

        case AST_WHILE:
            printf("While:\n");
            print_indent(indent + 1); printf("Condition:\n");
            print_ast_node(node->while_stmt.condition, indent + 2);
            print_indent(indent + 1); printf("Body:\n");
            print_ast_node(node->while_stmt.body, indent + 2);
            break;

        default:
            printf("Unknown AST node type %d\n", node->type);
            break;
    }
}

void free_ast_result(ASTResult* result) {
    if (result->root) {
        free_ast(result->root);
        result->root = NULL;
    }
}

void print_ast_if_enabled(ASTResult* result, const char* label) {
    if (print_parser_logs && result->root) {
        printf("\n=== AST Output: %s ===\n", label);
        print_ast_node(result->root, 0);
        printf("====================================\n");
    }
}

void setUp(void) {}
void tearDown(void) {}





void test_parse_negative_number(void) {
    ASTResult result = parse_source("let x = -42");

    print_ast_if_enabled(&result, "test_parse_negative_number");

    ASTNode* let_stmt = result.root->block.statements[0];
    ASTNode* expr = let_stmt->let_stmt.expr;

    TEST_ASSERT_EQUAL(AST_NUMBER, expr->type);
    TEST_ASSERT_EQUAL_DOUBLE(-42.0, expr->number.value);

    free_ast_result(&result);
}

void test_parse_let_statement(void) {
    const char* source = "let a = 5 + 3";
    ASTNode* root = parse_script(source);

    TEST_ASSERT_NOT_NULL(root);
    TEST_ASSERT_EQUAL(AST_BLOCK, root->type);
    TEST_ASSERT_EQUAL(1, root->block.count);

    ASTNode* let_stmt = root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_LET, let_stmt->type);
    TEST_ASSERT_EQUAL_STRING("a", let_stmt->let_stmt.name);

    ASTNode* expr = let_stmt->let_stmt.expr;
    TEST_ASSERT_EQUAL(AST_BINARY, expr->type);
    TEST_ASSERT_EQUAL(TOKEN_PLUS, expr->binary_expr.op);

    free_ast(root);
}

void test_parse_if_else(void) {
    ASTResult result = parse_source("if 1 < 2 { let x = 5 } else { let x = 6 }");
    print_ast_if_enabled(&result, "test_parse_if_else");

    TEST_ASSERT_EQUAL(AST_BLOCK, result.root->type);
    TEST_ASSERT_EQUAL(1, result.root->block.count);
    ASTNode* if_stmt = result.root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_IF, if_stmt->type);

    TEST_ASSERT_EQUAL(AST_BINARY, if_stmt->if_stmt.condition->type);
    TEST_ASSERT_NOT_NULL(if_stmt->if_stmt.then_branch);
    TEST_ASSERT_NOT_NULL(if_stmt->if_stmt.else_branch);

    free_ast_result(&result);
}


void test_parse_for_loop(void) {
    const char* source = "for i = 0..5 { let x = i }";
    ASTResult result = parse_source(source);

    print_ast_if_enabled(&result, "test_parse_for_loop");

    ASTNode* for_stmt = result.root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_FOR, for_stmt->type);
    TEST_ASSERT_EQUAL_STRING("i", for_stmt->for_stmt.var);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, for_stmt->for_stmt.from);
    TEST_ASSERT_EQUAL_DOUBLE(5.0, for_stmt->for_stmt.to);

    free_ast_result(&result);
}

void test_parse_while_loop(void) {
    const char* source = "while 1 < 10 { let x = 1 }";
    ASTResult result = parse_source(source);

    print_ast_if_enabled(&result, "test_parse_while_loop");

    ASTNode* while_stmt = result.root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_WHILE, while_stmt->type);
    TEST_ASSERT_EQUAL(AST_BINARY, while_stmt->while_stmt.condition->type);

    free_ast_result(&result);
}

void test_nested_if_else_if(void) {
    const char* source = "if 1 < 2 { let a = 1 } else if 2 < 3 { let b = 2 } else { let c = 3 }";
    ASTResult result = parse_source(source);

    print_ast_if_enabled(&result, "test_nested_if_else_if");

    ASTNode* if_stmt = result.root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_IF, if_stmt->type);
    TEST_ASSERT_NOT_NULL(if_stmt->if_stmt.else_branch);
    TEST_ASSERT_EQUAL(AST_IF, if_stmt->if_stmt.else_branch->type);

    free_ast_result(&result);
}

void test_parse_gcode_expression(void) {
    const char* source = "G1 X[1+2] Y[i]";
    ASTResult result = parse_source(source);

    print_ast_if_enabled(&result, "test_parse_gcode_expression");

    ASTNode* gcode = result.root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_GCODE, gcode->type);
    TEST_ASSERT_EQUAL_STRING("G1", gcode->gcode_stmt.code);
    TEST_ASSERT_EQUAL(2, gcode->gcode_stmt.argCount);

    free_ast_result(&result);
}

void test_parse_note_block(void) {
    const char* source = "note {This is a note with {nested} braces}";
    ASTResult result = parse_source(source);

    print_ast_if_enabled(&result, "test_parse_note_block");

    ASTNode* note_stmt = result.root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_NOTE, note_stmt->type);
    TEST_ASSERT_NOT_NULL(note_stmt->note.content);
    TEST_ASSERT_TRUE(strstr(note_stmt->note.content, "nested") != NULL);

    free_ast_result(&result);
}

void test_parse_gcode_implicit_G1(void) {
    const char* source = "X[10] Y[20]";
    ASTResult result = parse_source(source);

    print_ast_if_enabled(&result, "test_parse_gcode_implicit_G1");

    ASTNode* gcode = result.root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_GCODE, gcode->type);
    TEST_ASSERT_EQUAL_STRING("G1", gcode->gcode_stmt.code);
    TEST_ASSERT_EQUAL(2, gcode->gcode_stmt.argCount);

    free_ast_result(&result);
}

void test_parse_unary_expression(void) {
    const char* source = "let a = !!1";
    ASTResult result = parse_source(source);

    print_ast_if_enabled(&result, "test_parse_unary_expression");

    ASTNode* let_stmt = result.root->block.statements[0];
    ASTNode* expr = let_stmt->let_stmt.expr;
    TEST_ASSERT_EQUAL(AST_UNARY, expr->type);
    TEST_ASSERT_EQUAL(TOKEN_BANG, expr->unary_expr.op);
    TEST_ASSERT_EQUAL(AST_UNARY, expr->unary_expr.operand->type);

    free_ast_result(&result);
}

void test_invalid_token_should_fail(void) {
    const char* source = "unexpected_token";
    ASTResult result = parse_source(source);

    print_ast_if_enabled(&result, "test_invalid_token_should_fail");

    TEST_ASSERT_NOT_NULL(result.root);
    TEST_ASSERT_EQUAL(AST_BLOCK, result.root->type);
    TEST_ASSERT_EQUAL(1, result.root->block.count);
    TEST_ASSERT_EQUAL(AST_GCODE, result.root->block.statements[0]->type);
    TEST_ASSERT_EQUAL_STRING("G1", result.root->block.statements[0]->gcode_stmt.code);
    TEST_ASSERT_EQUAL_STRING("unexpected_token", result.root->block.statements[0]->gcode_stmt.args[0].key);

    free_ast_result(&result);
}






void test_deeply_nested_if_else(void) {
    const char* source =
        "if 1 < 2 {"
        "   if 2 < 3 {"
        "       if 3 < 4 {"
        "           let a = 10"
        "       } else {"
        "           let a = 11"
        "       }"
        "   } else {"
        "       let b = 12"
        "   }"
        "} else {"
        "   let c = 13"
        "}";

    ASTResult result = parse_source(source);
    print_ast_if_enabled(&result, "test_deeply_nested_if_else");
    TEST_ASSERT_NOT_NULL(result.root);
    free_ast_result(&result);
}

void test_nested_loops_and_conditionals(void) {
    const char* source =
        "for i = 0..3 {"
        "   while i < 2 {"
        "       if i == 1 {"
        "           let x = i"
        "       }"
        "   }"
        "}";

    ASTResult result = parse_source(source);
    print_ast_if_enabled(&result, "test_nested_loops_and_conditionals");
    TEST_ASSERT_NOT_NULL(result.root);
    free_ast_result(&result);
}

void test_complex_nested_gcode(void) {
    const char* source = "G1 X[(i+1)*2] Y[(j-3)/(k+4)]";

    ASTResult result = parse_source(source);
    print_ast_if_enabled(&result, "test_complex_nested_gcode");
    TEST_ASSERT_NOT_NULL(result.root);
    free_ast_result(&result);
}

void test_multiple_statements_in_block(void) {
    const char* source =
        "if 1 < 2 {"
        "   let a = 1"
        "   let b = 2"
        "   let c = 3"
        "}";

    ASTResult result = parse_source(source);
    print_ast_if_enabled(&result, "test_multiple_statements_in_block");
    TEST_ASSERT_NOT_NULL(result.root);
    free_ast_result(&result);
}



int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_parse_let_statement);
    RUN_TEST(test_parse_if_else);
    RUN_TEST(test_parse_for_loop);
    RUN_TEST(test_parse_while_loop);
    RUN_TEST(test_nested_if_else_if);
    RUN_TEST(test_parse_gcode_expression);


    RUN_TEST(test_parse_note_block);
    RUN_TEST(test_parse_gcode_implicit_G1);
    RUN_TEST(test_parse_unary_expression);
    RUN_TEST(test_parse_negative_number);
    RUN_TEST(test_invalid_token_should_fail); // Optional: only if running in a crash-test environment


    RUN_TEST(test_deeply_nested_if_else);
    RUN_TEST(test_nested_loops_and_conditionals);
    RUN_TEST(test_complex_nested_gcode);
    RUN_TEST(test_multiple_statements_in_block);

    return UNITY_END();
}

