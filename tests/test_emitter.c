// File: tests/test_emitter.c
#include "Unity/src/unity.h"
#include "parser.h"
#include "runtime/evaluator.h"
#include "generator/gcode_emitter.h"

 int debug = 1;

extern int statement_count;

void setUp(void) {
    statement_count = 0;
}
void tearDown(void) {}

void test_emit_simple_gcode_block(void) {
    const char* code =
        "let x = 10\n"
        "note { simple emit }\n"
        "if (x > 5) {\n"
        "  G1 X[x] Y[0]\n"
        "} else {\n"
        "  G1 X[0] Y[0]\n"
        "}\n";

    ASTNode* root = parse_script(code);
   

    emit_gcode(root, debug);

    // We expect:
    // - let x
    // - note
    // - G1 (from if true)
    TEST_ASSERT_EQUAL_INT(4, statement_count);  // let, note, G1

    TEST_ASSERT_EQUAL_DOUBLE(10.0, get_var("x"));  // Confirm 'let x = 10' was evaluated

    free_ast(root);
}

void test_emit_loop_and_conditionals(void) {
    
    const char* code =
        "let z = 0\n"
        "for i = 1..3 {\n"
        "  let z = z + i\n"
        "  if (z > 2) { G1 X[z] }\n"
        "}";

    ASTNode* root = parse_script(code);


    emit_gcode(root, debug);

    // Expect:
    // - for loop (3 runs)
    //   - 3 let z assignments
    //   - 2 G1 calls (when z > 2)
    TEST_ASSERT_EQUAL_DOUBLE(6.0, get_var("z"));  // z = 1+2+3

TEST_ASSERT_EQUAL_INT(10, statement_count);
  // 1 initial let + 3 inner lets + 2 G1s = 6


    free_ast(root);
}

void test_emit_nested_if_inside_loop(void) {
    const char* code =
        "let count = 0\n"
        "for i = 1..4 {\n"
        "  if (i == 2) {\n"
        "    let count = count + 1\n"
        "    G1 X[i]\n"
        "  }\n"
        "}";

    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);

    // Loop i = 1..4, only even i (2 and 4) should match condition
    // → 2 lets + 2 G1s + 1 outer let + 1 for + 2 if = 8 statements
    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("count"));  // incremented twice
    TEST_ASSERT_EQUAL_INT(8, statement_count);

    free_ast(root);
}

void test_emit_while_loop_basic(void) {
    const char* code =
        "let a = 0\n"
        "while (a < 3) {\n"
        "  let a = a + 1\n"
        "  G1 X[a]\n"
        "}";

    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);

    // a goes from 0 → 1 → 2 → 3 (3 loops)
    // → 1 let (init) + 1 while + (3 lets + 3 G1s + 3 ifs) = 8
    TEST_ASSERT_EQUAL_DOUBLE(3.0, get_var("a"));  // final a
    TEST_ASSERT_EQUAL_INT(8, statement_count);    // 1 let + 1 while + 3 lets + 3 G1

    free_ast(root);
}

void test_emit_nested_if_inside_loop2(void) {
    const char* code =
        "let count = 0\n"
        "for i = 1..4 {\n"
        "  if (i == 2 || i == 4) {\n"
        "    let count = count + 1\n"
        "    G1 X[i]\n"
        "  }\n"
        "}";

    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);

    TEST_ASSERT_EQUAL_DOUBLE(2.0, get_var("count"));  // should increment twice
    TEST_ASSERT_EQUAL_INT(10, statement_count);  


    free_ast(root);
}


void test_emit_function_declaration_and_call(void) {
    const char* code =
        "function square(x) {\n"
        "  return x * x\n"
        "}\n"
        "let a = 3\n"
        "let b = 4\n"
        "note { function exists but not called }\n";

    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);

    // Expecting 1 function + 2 let + 1 note = 4
    TEST_ASSERT_EQUAL_DOUBLE(3.0, get_var("a"));
    TEST_ASSERT_EQUAL_DOUBLE(4.0, get_var("b"));
    TEST_ASSERT_EQUAL_INT(4, statement_count);

    free_ast(root);
}


void test_emit_function_call_returns_value(void) {
    const char* code =
        "function square(x) {\n"
        "  return x * x\n"
        "}\n"
        "let a = 5\n"
        "square(10)\n"
        "note { test_emit_function_call_returns_value }\n";

    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);

    TEST_ASSERT_EQUAL_DOUBLE(3.0, get_var("a"));
    TEST_ASSERT_EQUAL_DOUBLE(4.0, get_var("b"));
    TEST_ASSERT_EQUAL_INT(4, statement_count);

    free_ast(root);
}



void test_emit_function_call_inside_expression(void) {
    const char* code =
        "function inc(x) {\n"
        "  return x + 1\n"
        "}\n"
        "let a = 2\n"
        "let b = inc(a) + inc(3)\n"; // 3 + 4 = 7

    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);

    TEST_ASSERT_EQUAL_DOUBLE(7.0, get_var("b"));
    TEST_ASSERT_EQUAL_INT(3, statement_count);

    free_ast(root);
}



void test_emit_function_with_two_params(void) {
    const char* code =
        "function add(x, y) {\n"
        "  return x + y\n"
        "}\n"
        "let sum = add(3, 7)\n";

    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);

    TEST_ASSERT_EQUAL_DOUBLE(10.0, get_var("sum"));
    TEST_ASSERT_EQUAL_INT(2, statement_count);   // function + let

    free_ast(root);
}








int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_emit_simple_gcode_block);
    RUN_TEST(test_emit_loop_and_conditionals);
    RUN_TEST(test_emit_nested_if_inside_loop);
    RUN_TEST(test_emit_while_loop_basic);
    RUN_TEST(test_emit_nested_if_inside_loop2);
    RUN_TEST(test_emit_function_declaration_and_call);

    RUN_TEST(test_emit_function_call_returns_value);
  //  RUN_TEST(test_emit_function_call_inside_expression);
  //  RUN_TEST(test_emit_function_with_two_params);

    return UNITY_END();
}
