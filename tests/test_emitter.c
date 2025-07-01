// File: tests/test_emitter.c
#include "Unity/src/unity.h"
#include "parser.h"
#include "runtime/evaluator.h"
#include "generator/gcode_emitter.h"

 int debug = 1;

extern int statement_count;
extern void reset_runtime_state(void);

void setUp(void) {
    statement_count = 0;
    reset_runtime_state();
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
        "let a = square(3)\n"
        "let b = square(2)\n"
        "note { test_emit_function_call_returns_value }\n";

    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);

    TEST_ASSERT_EQUAL_DOUBLE(9.0, get_var("a"));
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

void test_emit_function_no_return(void) {
    const char* code =
        "function doNothing(x) {\n"
        "  let y = x + 1\n"
        "}\n"
        "let a = doNothing(5)\n";

    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);

    // If no return, expect 0.0 or your default return value
    TEST_ASSERT_EQUAL_DOUBLE(0.0, get_var("a"));
    TEST_ASSERT_EQUAL_INT(2, statement_count);

    free_ast(root);
}




void test_emit_function_recursion(void) {
    const char* code =
        "function fact(n) {\n"
        "  if (n <= 1) { return 1 }\n"
        "  return n * fact(n - 1)\n"
        "}\n"
        "let a = fact(4)\n"; // 4*3*2*1 = 24

    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);

    TEST_ASSERT_EQUAL_DOUBLE(24.0, get_var("a"));
    TEST_ASSERT_EQUAL_INT(2, statement_count);

    free_ast(root);
}








void test_emit_function_many_args(void) {
    const char* code =
        "function sum5(a, b, c, d, e) {\n"
        "  return a + b + c + d + e\n"
        "}\n"
        "let s = sum5(1,2,3,4,5)\n";

    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);

    TEST_ASSERT_EQUAL_DOUBLE(15.0, get_var("s"));
    TEST_ASSERT_EQUAL_INT(2, statement_count);

    free_ast(root);
}



void test_emit_function_expr_args(void) {
    const char* code =
        "function mul(a, b) { return a * b }\n"
        "let x = 2\n"
        "let y = 3\n"
        "let z = mul(x + 1, y + 2)\n"; // 3 * 5 = 15

    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);

    TEST_ASSERT_EQUAL_DOUBLE(15.0, get_var("z"));
    TEST_ASSERT_EQUAL_INT(4, statement_count);

    free_ast(root);
}

void test_emit_function_in_condition(void) {
    const char* code =
        "function isEven(x) {\n"
        "  return (x & 1) == 0\n"
        "}\n"
        "let a = 0\n"
        "if (isEven(4)) { let a = 1 }\n";

    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);

    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("a"));
    TEST_ASSERT_EQUAL_INT(4, statement_count);

    free_ast(root);
}





void test_emit_bitwise_and(void) {
    const char* code =
        "let a = 6 & 3\n"; // 6 & 3 == 2

    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);

    TEST_ASSERT_EQUAL_DOUBLE(2.0, get_var("a"));
    TEST_ASSERT_EQUAL_INT(1, statement_count); // only one let statement

    free_ast(root);
}





void test_emit_function_overwrite_var(void) {
    const char* code =
        "let x = 10\n"
        "function setx(val) {\n"
        "  let x = val\n"
        "  return x\n"
        "}\n"
        "let y = setx(5)\n";

    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);

    TEST_ASSERT_EQUAL_DOUBLE(10.0, get_var("x")); // global x should remain unchanged
    TEST_ASSERT_EQUAL_DOUBLE(5.0, get_var("y"));
    TEST_ASSERT_EQUAL_INT(3, statement_count);

    free_ast(root);
}

void test_emit_function_no_params_no_return(void) {
    const char* code =
        "function foo() { let x = 1 }\n"
        "let y = foo()\n";
    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, get_var("y"));
    free_ast(root);
}



void test_emit_function_only_return(void) {
    const char* code =
        "function ret42() { return 42 }\n"
        "let x = ret42()\n";
    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);
    TEST_ASSERT_EQUAL_DOUBLE(42.0, get_var("x"));
    free_ast(root);
}

void test_emit_function_unused_param(void) {
    const char* code =
        "function foo(a, b) { return a }\n"
        "let x = foo(5, 99)\n";
    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);
    TEST_ASSERT_EQUAL_DOUBLE(5.0, get_var("x"));
    free_ast(root);
}



void test_emit_function_calls_function(void) {
    const char* code =
        "function add1(x) { return x + 1 }\n"
        "function add2(x) { return add1(x) + 1 }\n"
        "let y = add2(3)\n";
    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);
    TEST_ASSERT_EQUAL_DOUBLE(5.0, get_var("y"));
    free_ast(root);
}

void test_emit_function_early_return(void) {
    const char* code =
        "function test(x) { if (x > 0) { return 1 } return 2 }\n"
        "let a = test(5)\n"
        "let b = test(-1)\n";
    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, get_var("a"));
    TEST_ASSERT_EQUAL_DOUBLE(2.0, get_var("b"));
    free_ast(root);
}


void test_emit_function_empty_body(void) {
    const char* code =
        "function empty() {}\n"
        "let x = empty()\n";
    ASTNode* root = parse_script(code);
    emit_gcode(root, debug);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, get_var("x"));
    free_ast(root);
}










int main(void) {
    UNITY_BEGIN();


// all pass do not remuve

    RUN_TEST(test_emit_function_declaration_and_call);
    RUN_TEST(test_emit_simple_gcode_block);
    RUN_TEST(test_emit_loop_and_conditionals);
    RUN_TEST(test_emit_nested_if_inside_loop);
    RUN_TEST(test_emit_while_loop_basic);
    RUN_TEST(test_emit_nested_if_inside_loop2);
    RUN_TEST(test_emit_function_call_returns_value);
    RUN_TEST(test_emit_bitwise_and);
    RUN_TEST(test_emit_function_call_inside_expression);
    RUN_TEST(test_emit_function_with_two_params);
    RUN_TEST(test_emit_function_many_args);
    RUN_TEST(test_emit_function_expr_args);
    RUN_TEST(test_emit_function_in_condition);
    RUN_TEST(test_emit_function_recursion);
    RUN_TEST(test_emit_function_no_return);
    RUN_TEST(test_emit_function_overwrite_var);


    // Add these for full function coverage:
    RUN_TEST(test_emit_function_no_params_no_return);
    RUN_TEST(test_emit_function_only_return);
    RUN_TEST(test_emit_function_unused_param);
    RUN_TEST(test_emit_function_calls_function);
    RUN_TEST(test_emit_function_early_return);
    RUN_TEST(test_emit_function_empty_body);





    return UNITY_END();
}
