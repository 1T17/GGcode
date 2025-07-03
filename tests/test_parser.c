

#include "Unity/src/unity.h"
#include "../src/lexer/lexer.h"
#include "../src/parser/parser.h"
#include "../src/lexer/token_utils.h"
#include "../include/config.h"
#include <string.h>

int print_parser_logs = 0;

typedef struct
{
    ASTNode *root;
} ASTResult;

ASTResult parse_source(const char *source)
{
    ASTResult result;
    result.root = parse_script(source);
    return result;
}

void print_indent(int indent)
{
    for (int i = 0; i < indent; i++)
        printf("  ");
}

void print_ast_node(ASTNode *node, int indent)
{
    if (!node)
        return;

    print_indent(indent);

    switch (node->type)
    {
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
        for (int i = 0; i < node->gcode_stmt.argCount; i++)
        {
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
        for (int i = 0; i < node->block.count; i++)
        {
            print_ast_node(node->block.statements[i], indent + 1);
        }
        break;

    case AST_IF:
        printf("If:\n");
        print_indent(indent + 1);
        printf("Condition:\n");
        print_ast_node(node->if_stmt.condition, indent + 2);
        print_indent(indent + 1);
        printf("Then:\n");
        print_ast_node(node->if_stmt.then_branch, indent + 2);
        if (node->if_stmt.else_branch)
        {
            print_indent(indent + 1);
            printf("Else:\n");
            print_ast_node(node->if_stmt.else_branch, indent + 2);
        }
        break;

    case AST_FOR:
        if (node->for_stmt.from->type == AST_NUMBER && node->for_stmt.to->type == AST_NUMBER)
        {
            printf("For %s = %g..%g\n", node->for_stmt.var,
                   node->for_stmt.from->number.value,
                   node->for_stmt.to->number.value);
        }
        else
        {
            printf("For %s = <expr>..<expr>\n", node->for_stmt.var);
        }
        print_ast_node(node->for_stmt.body, indent + 1);
        break;

    case AST_WHILE:
        printf("While:\n");
        print_indent(indent + 1);
        printf("Condition:\n");
        print_ast_node(node->while_stmt.condition, indent + 2);
        print_indent(indent + 1);
        printf("Body:\n");
        print_ast_node(node->while_stmt.body, indent + 2);
        break;

    default:
        printf("Unknown AST node type %d\n", node->type);
        break;
    }
}

void free_ast_result(ASTResult *result)
{
    if (result->root)
    {
        free_ast(result->root);
        result->root = NULL;
    }
}

void print_ast_if_enabled(ASTResult *result, const char *label)
{
    if (print_parser_logs && result->root)
    {
        printf("\n=== AST Output: %s ===\n", label);
        print_ast_node(result->root, 0);
        printf("====================================\n");
    }
}

void setUp(void) {}
void tearDown(void) {}

void test_parse_negative_number(void)
{
    ASTResult result = parse_source("let x = -42");

    print_ast_if_enabled(&result, "test_parse_negative_number");

    ASTNode *let_stmt = result.root->block.statements[0];
    ASTNode *expr = let_stmt->let_stmt.expr;

    TEST_ASSERT_EQUAL(AST_NUMBER, expr->type);
    TEST_ASSERT_EQUAL_DOUBLE(-42.0, expr->number.value);

    free_ast_result(&result);
}

void test_parse_let_statement(void)
{
    const char *source = "let a = 5 + 3";
    ASTNode *root = parse_script(source);

    TEST_ASSERT_NOT_NULL(root);
    TEST_ASSERT_EQUAL(AST_BLOCK, root->type);
    TEST_ASSERT_EQUAL(1, root->block.count);

    ASTNode *let_stmt = root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_LET, let_stmt->type);
    TEST_ASSERT_EQUAL_STRING("a", let_stmt->let_stmt.name);

    ASTNode *expr = let_stmt->let_stmt.expr;
    TEST_ASSERT_EQUAL(AST_BINARY, expr->type);
    TEST_ASSERT_EQUAL(TOKEN_PLUS, expr->binary_expr.op);

    free_ast(root);
}

void test_parse_if_else(void)
{
    ASTResult result = parse_source("if 1 < 2 { let x = 5 } else { let x = 6 }");
    print_ast_if_enabled(&result, "test_parse_if_else");

    TEST_ASSERT_EQUAL(AST_BLOCK, result.root->type);
    TEST_ASSERT_EQUAL(1, result.root->block.count);
    ASTNode *if_stmt = result.root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_IF, if_stmt->type);

    TEST_ASSERT_EQUAL(AST_BINARY, if_stmt->if_stmt.condition->type);
    TEST_ASSERT_NOT_NULL(if_stmt->if_stmt.then_branch);
    TEST_ASSERT_NOT_NULL(if_stmt->if_stmt.else_branch);

    free_ast_result(&result);
}

void test_parse_for_loop(void)
{
    const char *source = "for i = 0..5 { let x = i }";
    ASTResult result = parse_source(source);

    print_ast_if_enabled(&result, "test_parse_for_loop");

    ASTNode *for_stmt = result.root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_FOR, for_stmt->type);
    TEST_ASSERT_EQUAL_STRING("i", for_stmt->for_stmt.var);

    TEST_ASSERT_EQUAL(AST_NUMBER, for_stmt->for_stmt.from->type);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, for_stmt->for_stmt.from->number.value);

    TEST_ASSERT_EQUAL(AST_NUMBER, for_stmt->for_stmt.to->type);
    TEST_ASSERT_EQUAL_DOUBLE(5.0, for_stmt->for_stmt.to->number.value);

    free_ast_result(&result);
}

void test_parse_while_loop(void)
{
    const char *source = "while 1 < 10 { let x = 1 }";
    ASTResult result = parse_source(source);

    print_ast_if_enabled(&result, "test_parse_while_loop");

    ASTNode *while_stmt = result.root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_WHILE, while_stmt->type);
    TEST_ASSERT_EQUAL(AST_BINARY, while_stmt->while_stmt.condition->type);

    free_ast_result(&result);
}

void test_nested_if_else_if(void)
{
    const char *source = "if 1 < 2 { let a = 1 } else if 2 < 3 { let b = 2 } else { let c = 3 }";
    ASTResult result = parse_source(source);

    print_ast_if_enabled(&result, "test_nested_if_else_if");

    ASTNode *if_stmt = result.root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_IF, if_stmt->type);
    TEST_ASSERT_NOT_NULL(if_stmt->if_stmt.else_branch);
    TEST_ASSERT_EQUAL(AST_IF, if_stmt->if_stmt.else_branch->type);

    free_ast_result(&result);
}

void test_parse_gcode_expression(void)
{
    const char *source = "G1 X[1+2] Y[i]";
    ASTResult result = parse_source(source);

    print_ast_if_enabled(&result, "test_parse_gcode_expression");

    ASTNode *gcode = result.root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_GCODE, gcode->type);
    TEST_ASSERT_EQUAL_STRING("G1", gcode->gcode_stmt.code);
    TEST_ASSERT_EQUAL(2, gcode->gcode_stmt.argCount);

    free_ast_result(&result);
}

void test_parse_note_block(void)
{
    const char *source = "note {This is a note with {nested} braces}";
    ASTResult result = parse_source(source);

    print_ast_if_enabled(&result, "test_parse_note_block");

    ASTNode *note_stmt = result.root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_NOTE, note_stmt->type);
    TEST_ASSERT_NOT_NULL(note_stmt->note.content);
    TEST_ASSERT_TRUE(strstr(note_stmt->note.content, "nested") != NULL);

    free_ast_result(&result);
}

void test_parse_gcode_implicit_G1(void)
{
    const char *source = "X[10] Y[20]";
    ASTResult result = parse_source(source);

    print_ast_if_enabled(&result, "test_parse_gcode_implicit_G1");

    ASTNode *gcode = result.root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_GCODE, gcode->type);
    TEST_ASSERT_EQUAL_STRING("G1", gcode->gcode_stmt.code);
    TEST_ASSERT_EQUAL(2, gcode->gcode_stmt.argCount);

    free_ast_result(&result);
}

void test_parse_unary_expression(void)
{
    const char *source = "let a = !!1";
    ASTResult result = parse_source(source);

    print_ast_if_enabled(&result, "test_parse_unary_expression");

    ASTNode *let_stmt = result.root->block.statements[0];
    ASTNode *expr = let_stmt->let_stmt.expr;
    TEST_ASSERT_EQUAL(AST_UNARY, expr->type);
    TEST_ASSERT_EQUAL(TOKEN_BANG, expr->unary_expr.op);
    TEST_ASSERT_EQUAL(AST_UNARY, expr->unary_expr.operand->type);

    free_ast_result(&result);
}

void test_invalid_token_should_fail(void)
{
    const char *source = "unexpected_token";
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

void test_deeply_nested_if_else(void)
{
    const char *source =
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

void test_nested_loops_and_conditionals(void)
{
    const char *source =
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

void test_complex_nested_gcode(void)
{
    const char *source = "G1 X[(i+1)*2] Y[(j-3)/(k+4)]";

    ASTResult result = parse_source(source);
    print_ast_if_enabled(&result, "test_complex_nested_gcode");
    TEST_ASSERT_NOT_NULL(result.root);
    free_ast_result(&result);
}

void test_multiple_statements_in_block(void)
{
    const char *source =
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

void test_builtin_constants_expression(void)
{
    const char *source =
        "let a = PI\n"
        "let b = TAU\n"
        "let c = EU";

    printf("Parsing source:\n%s\n", source);

    ASTResult result = parse_source(source);
    print_ast_if_enabled(&result, "test_builtin_constants_expression");

    TEST_ASSERT_NOT_NULL(result.root);
    TEST_ASSERT_EQUAL(AST_BLOCK, result.root->type);
    TEST_ASSERT_EQUAL(3, result.root->block.count);

    ASTNode *let_a = result.root->block.statements[0];
    ASTNode *let_b = result.root->block.statements[1];
    ASTNode *let_c = result.root->block.statements[2];

    printf("a = %f\n", let_a->let_stmt.expr->number.value);
    printf("b = %f\n", let_b->let_stmt.expr->number.value);
    printf("c = %f\n", let_c->let_stmt.expr->number.value);

    TEST_ASSERT_EQUAL(AST_LET, let_a->type);
    TEST_ASSERT_EQUAL(AST_NUMBER, let_a->let_stmt.expr->type);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 3.14159, let_a->let_stmt.expr->number.value);

    TEST_ASSERT_EQUAL(AST_LET, let_b->type);
    TEST_ASSERT_EQUAL(AST_NUMBER, let_b->let_stmt.expr->type);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 6.28318, let_b->let_stmt.expr->number.value);

    TEST_ASSERT_EQUAL(AST_LET, let_c->type);
    TEST_ASSERT_EQUAL(AST_NUMBER, let_c->let_stmt.expr->type);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 2.71828, let_c->let_stmt.expr->number.value);

    free_ast_result(&result);
}

void test_builtin_constants_only(void)
{
    const char *source =
        "let pi = PI\n"
        "let tau = TAU\n"
        "let eu = EU\n"
        "let d2r = DEG_TO_RAD\n"
        "let r2d = RAD_TO_DEG";

    ASTResult result = parse_source(source);
    print_ast_if_enabled(&result, "test_builtin_constants_only");

    TEST_ASSERT_NOT_NULL(result.root);
    TEST_ASSERT_EQUAL(AST_BLOCK, result.root->type);
    TEST_ASSERT_EQUAL(5, result.root->block.count);

    for (int i = 0; i < 5; i++)
    {
        ASTNode *stmt = result.root->block.statements[i];
        TEST_ASSERT_EQUAL(AST_LET, stmt->type);
        TEST_ASSERT_EQUAL(AST_NUMBER, stmt->let_stmt.expr->type);
        printf("[Const %d] = %f\n", i, stmt->let_stmt.expr->number.value);
    }

    free_ast_result(&result);
}

void test_basic_arithmetic_functions(void)
{
    const char *source =
        "let a1 = abs(-5)\n"
        "let a2 = mod(10, 3)\n"
        "let a3 = floor(3.9)\n"
        "let a4 = ceil(3.1)\n"
        "let a5 = round(2.6)\n"
        "let a6 = min(3, 4)\n"
        "let a7 = max(4, 5)\n"
        "let a8 = clamp(7, 0, 10)";

    ASTResult result = parse_source(source);
    print_ast_if_enabled(&result, "test_basic_arithmetic_functions");

    TEST_ASSERT_NOT_NULL(result.root);
    TEST_ASSERT_EQUAL(AST_BLOCK, result.root->type);
    TEST_ASSERT_EQUAL(8, result.root->block.count);

    for (int i = 0; i < 8; i++)
    {
        ASTNode *stmt = result.root->block.statements[i];
        TEST_ASSERT_EQUAL(AST_LET, stmt->type);
        TEST_ASSERT_EQUAL(AST_CALL, stmt->let_stmt.expr->type);
        printf("[Func %d] %s\n", i, stmt->let_stmt.expr->call_expr.name);
    }

    free_ast_result(&result);
}

void test_trigonometry_functions(void)
{
    const char *source =
        "let t1 = sin(0)\n"
        "let t2 = cos(0)\n"
        "let t3 = tan(0)\n"
        "let t4 = asin(0)\n"
        "let t5 = acos(1)\n"
        "let t6 = atan(1)\n"
        "let t7 = atan2(1, 1)\n"
        "let t8 = deg(3.14)\n"
        "let t9 = rad(180)";

    ASTResult result = parse_source(source);
    print_ast_if_enabled(&result, "test_trigonometry_functions");

    TEST_ASSERT_NOT_NULL(result.root);
    TEST_ASSERT_EQUAL(AST_BLOCK, result.root->type);
    TEST_ASSERT_EQUAL(9, result.root->block.count);

    for (int i = 0; i < 9; i++)
    {
        ASTNode *stmt = result.root->block.statements[i];
        TEST_ASSERT_EQUAL(AST_LET, stmt->type);
        TEST_ASSERT_EQUAL(AST_CALL, stmt->let_stmt.expr->type);
        printf("[Trig %d] %s\n", i, stmt->let_stmt.expr->call_expr.name);
    }

    free_ast_result(&result);
}

void test_geometry_functions(void)
{
    const char *source =
        "let g1 = sqrt(9)\n"
        "let g2 = pow(2, 3)\n"
        "let g3 = hypot(3, 4)\n"
        "let g4 = lerp(0, 10, 0.5)\n"
        "let g5 = map(5, 0, 10, 0, 100)\n"
        "let g6 = distance(0, 0, 3, 4)";

    ASTResult result = parse_source(source);
    print_ast_if_enabled(&result, "test_geometry_functions");

    TEST_ASSERT_NOT_NULL(result.root);
    TEST_ASSERT_EQUAL(AST_BLOCK, result.root->type);
    TEST_ASSERT_EQUAL(6, result.root->block.count);

    for (int i = 0; i < 6; i++)
    {
        ASTNode *stmt = result.root->block.statements[i];
        TEST_ASSERT_EQUAL(AST_LET, stmt->type);
        TEST_ASSERT_EQUAL(AST_CALL, stmt->let_stmt.expr->type);
        printf("[Geom %d] %s\n", i, stmt->let_stmt.expr->call_expr.name);
    }

    free_ast_result(&result);
}

void test_optional_advanced_functions(void)
{
    const char *source =
        "let o1 = noise(1)\n"
        "let o2 = sign(-42)\n"
        "let o3 = log(10)\n"
        "let o4 = exp(1)";

    ASTResult result = parse_source(source);
    print_ast_if_enabled(&result, "test_optional_advanced_functions");

    TEST_ASSERT_NOT_NULL(result.root);
    TEST_ASSERT_EQUAL(AST_BLOCK, result.root->type);
    TEST_ASSERT_EQUAL(4, result.root->block.count);

    for (int i = 0; i < 4; i++)
    {
        ASTNode *stmt = result.root->block.statements[i];
        TEST_ASSERT_EQUAL(AST_LET, stmt->type);
        TEST_ASSERT_EQUAL(AST_CALL, stmt->let_stmt.expr->type);
        printf("[Adv %d] %s\n", i, stmt->let_stmt.expr->call_expr.name);
    }

    free_ast_result(&result);
}

void test_parse_for_loop_with_step_and_dotdotlt(void)
{
    const char *source = "for i = 1..<10 step 2 { let x = i }";
    ASTResult result = parse_source(source);

    print_ast_if_enabled(&result, "test_parse_for_loop_with_step_and_dotdotlt");

    ASTNode *for_stmt = result.root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_FOR, for_stmt->type);
    TEST_ASSERT_EQUAL_STRING("i", for_stmt->for_stmt.var);

    // Check from, to, step are numbers and have correct values
    TEST_ASSERT_EQUAL(AST_NUMBER, for_stmt->for_stmt.from->type);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, for_stmt->for_stmt.from->number.value);

    TEST_ASSERT_EQUAL(AST_NUMBER, for_stmt->for_stmt.to->type);
    TEST_ASSERT_EQUAL_DOUBLE(10.0, for_stmt->for_stmt.to->number.value);

    TEST_ASSERT_NOT_NULL(for_stmt->for_stmt.step);
    TEST_ASSERT_EQUAL(AST_NUMBER, for_stmt->for_stmt.step->type);
    TEST_ASSERT_EQUAL_DOUBLE(2.0, for_stmt->for_stmt.step->number.value);

    TEST_ASSERT_EQUAL(1, for_stmt->for_stmt.exclusive); // 1 means '..<' exclusive

    // Check the body is a block with one let statement
    ASTNode *body = for_stmt->for_stmt.body;
    TEST_ASSERT_EQUAL(AST_BLOCK, body->type);
    TEST_ASSERT_EQUAL(1, body->block.count);
    TEST_ASSERT_EQUAL(AST_LET, body->block.statements[0]->type);

    free_ast_result(&result);
}

void test_parse_for_loop_with_step_and_dotdot(void)
{
    const char *source = "for i = 1..10 step 2 { let x = i }";
    ASTResult result = parse_source(source);

    ASTNode *for_stmt = result.root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_FOR, for_stmt->type);
    TEST_ASSERT_EQUAL_STRING("i", for_stmt->for_stmt.var);

    TEST_ASSERT_EQUAL(AST_NUMBER, for_stmt->for_stmt.from->type);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, for_stmt->for_stmt.from->number.value);

    TEST_ASSERT_EQUAL(AST_NUMBER, for_stmt->for_stmt.to->type);
    TEST_ASSERT_EQUAL_DOUBLE(10.0, for_stmt->for_stmt.to->number.value);

    TEST_ASSERT_NOT_NULL(for_stmt->for_stmt.step);
    TEST_ASSERT_EQUAL(AST_NUMBER, for_stmt->for_stmt.step->type);
    TEST_ASSERT_EQUAL_DOUBLE(2.0, for_stmt->for_stmt.step->number.value);

    TEST_ASSERT_EQUAL(0, for_stmt->for_stmt.exclusive); // 0 means '..' inclusive

    free_ast_result(&result);
}

void test_parse_for_loop_without_step_and_dotdotlt(void)
{
    const char *source = "for i = 1..<10 { let x = i }";
    ASTResult result = parse_source(source);

    ASTNode *for_stmt = result.root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_FOR, for_stmt->type);
    TEST_ASSERT_EQUAL_STRING("i", for_stmt->for_stmt.var);

    TEST_ASSERT_EQUAL(AST_NUMBER, for_stmt->for_stmt.from->type);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, for_stmt->for_stmt.from->number.value);

    TEST_ASSERT_EQUAL(AST_NUMBER, for_stmt->for_stmt.to->type);
    TEST_ASSERT_EQUAL_DOUBLE(10.0, for_stmt->for_stmt.to->number.value);

    // Default step is 1.0 if not specified
    TEST_ASSERT_NOT_NULL(for_stmt->for_stmt.step);
    TEST_ASSERT_EQUAL(AST_NUMBER, for_stmt->for_stmt.step->type);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, for_stmt->for_stmt.step->number.value);

    TEST_ASSERT_EQUAL(1, for_stmt->for_stmt.exclusive); // 1 means '..<' exclusive

    free_ast_result(&result);
}

void test_parse_for_loop_without_step_and_dotdot(void)
{
    const char *source = "for i = 1..10 { let x = i }";
    ASTResult result = parse_source(source);

    ASTNode *for_stmt = result.root->block.statements[0];
    TEST_ASSERT_EQUAL(AST_FOR, for_stmt->type);
    TEST_ASSERT_EQUAL_STRING("i", for_stmt->for_stmt.var);

    TEST_ASSERT_EQUAL(AST_NUMBER, for_stmt->for_stmt.from->type);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, for_stmt->for_stmt.from->number.value);

    TEST_ASSERT_EQUAL(AST_NUMBER, for_stmt->for_stmt.to->type);
    TEST_ASSERT_EQUAL_DOUBLE(10.0, for_stmt->for_stmt.to->number.value);

    // Default step is 1.0 if not specified
    TEST_ASSERT_NOT_NULL(for_stmt->for_stmt.step);
    TEST_ASSERT_EQUAL(AST_NUMBER, for_stmt->for_stmt.step->type);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, for_stmt->for_stmt.step->number.value);

    TEST_ASSERT_EQUAL(0, for_stmt->for_stmt.exclusive); // 0 means '..' inclusive

    free_ast_result(&result);
}

void test_parse_for_loop_all_edge_cases(void)
{
    const char *source =
        "let x = 5\n"
        "let y = 10\n"
        "let s = 2\n"
        // Numeric bounds, inclusive, no step
        "for i = 0..5 { let a = i }\n"
        // Numeric bounds, inclusive, with step
        "for i = 0..5 step 2 { let b = i }\n"
        // Numeric bounds, exclusive, no step
        "for i = 0..<5 { let c = i }\n"
        // Numeric bounds, exclusive, with step
        "for i = 0..<5 step 2 { let d = i }\n"
        // Variable upper bound, inclusive, no step
        "for i = 0..x { let e = i }\n"
        // Variable upper bound, inclusive, with step
        "for i = 0..x step s { let f = i }\n"
        // Variable upper bound, exclusive, no step
        "for i = 0..<x { let g = i }\n"
        // Variable upper bound, exclusive, with step
        "for i = 0..<x step s { let h = i }\n"
        // Variable lower and upper bound, inclusive, with step
        "for i = x..y step s { let j = i }\n"
        // Variable lower and upper bound, exclusive, with step
        "for i = x..<y step s { let k = i }\n"
        // Expression as upper bound, inclusive, with step
        "for i = 0..(x + y) step (s + 1) { let m = i }\n"
        // Expression as lower and upper bound, exclusive, with step
        "for i = (x - 2)..<(y - 3) step (s * 2) { let n = i }\n";

    ASTResult result = parse_source(source);

    // There are 12 for-loops after the 3 let statements
    TEST_ASSERT_NOT_NULL(result.root);
    TEST_ASSERT_EQUAL(AST_BLOCK, result.root->type);
    TEST_ASSERT_EQUAL(15, result.root->block.count);

// Helper macro to check for-loop structure
#define CHECK_FOR_LOOP(idx, from_type, to_type, step_type, excl)     \
    do                                                               \
    {                                                                \
        ASTNode *for_stmt = result.root->block.statements[idx];      \
        TEST_ASSERT_EQUAL(AST_FOR, for_stmt->type);                  \
        TEST_ASSERT_EQUAL(from_type, for_stmt->for_stmt.from->type); \
        TEST_ASSERT_EQUAL(to_type, for_stmt->for_stmt.to->type);     \
        TEST_ASSERT_NOT_NULL(for_stmt->for_stmt.step);               \
        TEST_ASSERT_EQUAL(step_type, for_stmt->for_stmt.step->type); \
        TEST_ASSERT_EQUAL(excl, for_stmt->for_stmt.exclusive);       \
    } while (0)

    // 0: let x = 5
    // 1: let y = 10
    // 2: let s = 2
    // 3: for i = 0..5 { let a = i }
    CHECK_FOR_LOOP(3, AST_NUMBER, AST_NUMBER, AST_NUMBER, 0);
    // 4: for i = 0..5 step 2 { let b = i }
    CHECK_FOR_LOOP(4, AST_NUMBER, AST_NUMBER, AST_NUMBER, 0);
    // 5: for i = 0..<5 { let c = i }
    CHECK_FOR_LOOP(5, AST_NUMBER, AST_NUMBER, AST_NUMBER, 1);
    // 6: for i = 0..<5 step 2 { let d = i }
    CHECK_FOR_LOOP(6, AST_NUMBER, AST_NUMBER, AST_NUMBER, 1);
    // 7: for i = 0..x { let e = i }
    CHECK_FOR_LOOP(7, AST_NUMBER, AST_VAR, AST_NUMBER, 0);
    // 8: for i = 0..x step s { let f = i }
    CHECK_FOR_LOOP(8, AST_NUMBER, AST_VAR, AST_VAR, 0);
    // 9: for i = 0..<x { let g = i }
    CHECK_FOR_LOOP(9, AST_NUMBER, AST_VAR, AST_NUMBER, 1);
    // 10: for i = 0..<x step s { let h = i }
    CHECK_FOR_LOOP(10, AST_NUMBER, AST_VAR, AST_VAR, 1);
    // 11: for i = x..y step s { let j = i }
    CHECK_FOR_LOOP(11, AST_VAR, AST_VAR, AST_VAR, 0);
    // 12: for i = x..<y step s { let k = i }
    CHECK_FOR_LOOP(12, AST_VAR, AST_VAR, AST_VAR, 1);
    // 13: for i = 0..(x + y) step (s + 1) { let m = i }
    CHECK_FOR_LOOP(13, AST_NUMBER, AST_BINARY, AST_BINARY, 0);
    // 14: for i = (x - 2)..<(y - 3) step (s * 2) { let n = i }
    CHECK_FOR_LOOP(14, AST_BINARY, AST_BINARY, AST_BINARY, 1);

#undef CHECK_FOR_LOOP

    free_ast_result(&result);
}

int main(void)
{
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
    RUN_TEST(test_multiple_statements_in_block);
    RUN_TEST(test_builtin_constants_expression);
    RUN_TEST(test_builtin_constants_only);
    RUN_TEST(test_basic_arithmetic_functions);
    RUN_TEST(test_trigonometry_functions);
    RUN_TEST(test_geometry_functions);
    RUN_TEST(test_optional_advanced_functions);
    RUN_TEST(test_parse_for_loop_with_step_and_dotdotlt);
    RUN_TEST(test_parse_for_loop_with_step_and_dotdot);
    RUN_TEST(test_parse_for_loop_without_step_and_dotdotlt);
    RUN_TEST(test_parse_for_loop_without_step_and_dotdot);
    RUN_TEST(test_parse_for_loop_all_edge_cases);

    return UNITY_END();
}
