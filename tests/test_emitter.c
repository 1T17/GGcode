// File: tests/test_emitter.c
#include "Unity/src/unity.h"
#include "parser/parser.h"
#include "runtime/evaluator.h"
#include "generator/emitter.h"
#include "error/error.h"

#include "config/config.h"

extern int statement_count;
extern void reset_runtime_state(void);

void setUp(void)
{
    statement_count = 0;
    reset_runtime_state();
}

void tearDown(void) {}

void test_emit_simple_gcode_block(void)
{
    const char *code =
        "let x = 10\n"
        "note { simple emit }\n"
        "if (x > 5) {\n"
        "  G1 X[x] Y[0]\n"
        "} else {\n"
        "  G1 X[0] Y[0]\n"
        "}\n";

    ASTNode *root = parse_script_from_string(code); // <-- updated line

    emit_gcode(root, get_debug());

    // We expect:
    // - let x
    // - note
    // - if condition evaluation and G1 inside the "then" branch
    TEST_ASSERT_EQUAL_INT(4, statement_count);
    Value *val = get_var("x");
    TEST_ASSERT_NOT_NULL(val);
    TEST_ASSERT_EQUAL_DOUBLE(10.0, val->number);

    free_ast(root);
}

void test_emit_loop_and_conditionals(void)
{
    const char *code =
        "let z = 0\n"
        "for i = 1..3 {\n"
        "  let z = z + i\n"
        "  if (z > 2) { G1 X[z] }\n"
        "}";

    ASTNode *root = parse_script_from_string(code);

    emit_gcode(root, get_debug());

    Value *z = get_var("z");
    TEST_ASSERT_NOT_NULL(z);
    TEST_ASSERT_EQUAL_DOUBLE(6.0, z->number); // z = 1 + 2 + 3

    TEST_ASSERT_EQUAL_INT(10, statement_count); // let + for + 3 lets + 2 G1s + 3 ifs = 10

    free_ast(root);
}

void test_emit_nested_if_inside_loop(void)
{
    const char *code =
        "let count = 0\n"
        "for i = 1..4 {\n"
        "  if (i == 2 || i == 3) {\n"
        "     count = count + 1\n"
        "    G1 X[i]\n"
        "  }\n"
        "}\n";

    ASTNode *root = parse_script_from_string(code);

    statement_count = 0;
    reset_runtime_state();

    emit_gcode(root, get_debug());

    Value *val = get_var("count");
    TEST_ASSERT_NOT_NULL(val);
    TEST_ASSERT_EQUAL_DOUBLE(2.0, val->number); // ✅ Only i == 2 and 3 increment

    free_ast(root);
}

void test_emit_while_loop_basic(void)
{
    const char *code =
        "let a = 0\n"
        "while (a < 3) {\n"
        "  a = a + 1\n"
        "  G1 X[a]\n"
        "}";

    ASTNode *root = parse_script_from_string(code);

    emit_gcode(root, get_debug());

    Value *a = get_var("a");
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_EQUAL_DOUBLE(3.0, a->number); // should reach 3 after 3 iterations

    // 1 let + 1 while + (3x assign) + (3x G1) = 8 statements total
    TEST_ASSERT_EQUAL_INT(8, statement_count);

    free_ast(root);
}

void test_emit_nested_if_inside_loop2(void)
{
    const char *code =
        "let count = 0\n"
        "for i = 1..4 {\n"
        "  if (i == 2 || i == 4) {\n"
        "    let count = count + 1\n"
        "    G1 X[i]\n"
        "  }\n"
        "}";

    ASTNode *root = parse_script_from_string(code);

    statement_count = 0;
    reset_runtime_state();

    emit_gcode(root, get_debug());

    Value *count = get_var("count");
    TEST_ASSERT_NOT_NULL(count);
    TEST_ASSERT_EQUAL_DOUBLE(2.0, count->number); // should increment twice
    TEST_ASSERT_EQUAL_INT(10, statement_count);

    free_ast(root);
}

void test_emit_function_declaration_and_call(void)
{
    const char *code =
        "function square(x) {\n"
        "  return x * x\n"
        "}\n"
        "let a = 3\n"
        "let b = 4\n"
        "note { function exists but not called }\n";

    ASTNode *root = parse_script_from_string(code);

    statement_count = 0;
    reset_runtime_state();

    emit_gcode(root, get_debug());

    Value *a = get_var("a");
    Value *b = get_var("b");

    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);
    TEST_ASSERT_EQUAL_DOUBLE(3.0, a->number);
    TEST_ASSERT_EQUAL_DOUBLE(4.0, b->number);
    TEST_ASSERT_EQUAL_INT(3, statement_count); // function, 2 lets, 1 note

    free_ast(root);
}

void test_emit_function_call_returns_value(void)
{
    const char *code =
        "function square(x) {\n"
        "  return x * x\n"
        "}\n"
        "let r = 2\n"
        "let b = square(2)\n"
        "let bb = square(2)\n"
        "note { test_emit_function_call_returns_value }\n";

    ASTNode *root = parse_script_from_string(code);

    statement_count = 0;
    reset_runtime_state();

    emit_gcode(root, get_debug());

    Value *r = get_var("r");
    Value *bb = get_var("bb");

    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT_NOT_NULL(bb);

    TEST_ASSERT_EQUAL_DOUBLE(2.0, r->number);  // r = 2
    TEST_ASSERT_EQUAL_DOUBLE(4.0, bb->number); // bb = square(2)
    TEST_ASSERT_EQUAL_INT(4, statement_count); // 1 function + 3 lets + 1 note

    free_ast(root);
}

void test_emit_function_call_inside_expression(void)
{
    const char *code =
        "function inc(x) {\n"
        "  return x + 1\n"
        "}\n"
        "let a = 2\n"
        "let b = inc(a) + inc(3)\n"; // 3 + 4 = 7

    ASTNode *root = parse_script_from_string(code);
    statement_count = 0;
    reset_runtime_state();

    emit_gcode(root, get_debug());

    Value *b = get_var("b");
    TEST_ASSERT_NOT_NULL(b);
    TEST_ASSERT_EQUAL_DOUBLE(7.0, b->number);
    TEST_ASSERT_EQUAL_INT(2, statement_count); // function + 2 lets

    free_ast(root);
}

void test_emit_function_with_two_params(void)
{
    const char *code =
        "function add(x, y) {\n"
        "  return x + y\n"
        "}\n"
        "let sum = add(3, 7)\n";

    ASTNode *root = parse_script_from_string(code);
    statement_count = 0;
    reset_runtime_state();

    emit_gcode(root, get_debug());

    Value *sum = get_var("sum");
    TEST_ASSERT_NOT_NULL(sum);
    TEST_ASSERT_EQUAL_DOUBLE(10.0, sum->number);
    TEST_ASSERT_EQUAL_INT(1, statement_count); // function + let

    free_ast(root);
}

void test_emit_function_recursion(void)
{
    const char *code =
        "function fact(n) {\n"
        "  if (n <= 1) { return 1 }\n"
        "  return n * fact(n - 1)\n"
        "}\n"

        "let a = fact(4)\n"; // 4*3*2*1 = 24

    ASTNode *root = parse_script_from_string(code);
    statement_count = 0;
    reset_runtime_state();

    emit_gcode(root, get_debug());

    Value *a = get_var("a");
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_EQUAL_DOUBLE(24.0, a->number);
    TEST_ASSERT_EQUAL_INT(1, statement_count); //  let

    free_ast(root);
}

void test_emit_function_expr_args(void)
{
    const char *code =
        "function mul(a, b) { return a * b }\n"
        "let x = 2\n"
        "let y = 3\n"
        "let z = mul(x + 1, y + 2)\n"; // 3 * 5 = 15

    ASTNode *root = parse_script_from_string(code);
    statement_count = 0;
    reset_runtime_state();

    emit_gcode(root, get_debug());

    Value *z = get_var("z");
    TEST_ASSERT_NOT_NULL(z);
    TEST_ASSERT_EQUAL_DOUBLE(15.0, z->number);

    // ✅ Function declarations are NOT counted anymore
    TEST_ASSERT_EQUAL_INT(3, statement_count); // only x, y, and z

    free_ast(root);
}

void test_emit_function_in_condition(void)
{
    const char *code =
        "function isEven(x) {\n"
        "  return (x & 1) == 0\n"
        "}\n"
        "let a = 0\n"
        "if (isEven(4)) { let a = 1 }\n";

    ASTNode *root = parse_script_from_string(code);
    statement_count = 0;
    reset_runtime_state();

    emit_gcode(root, get_debug());

    Value *a = get_var("a");
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, a->number);
    TEST_ASSERT_EQUAL_INT(3, statement_count); // + let + if + inner let

    free_ast(root);
}

void test_emit_bitwise_and(void)
{
    const char *code =
        "let a = 6 & 3\n"; // 6 & 3 == 2

    ASTNode *root = parse_script_from_string(code);
    statement_count = 0;
    reset_runtime_state();

    emit_gcode(root, get_debug());

    Value *a = get_var("a");
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_EQUAL_DOUBLE(2.0, a->number);
    TEST_ASSERT_EQUAL_INT(1, statement_count); // just one let

    free_ast(root);
}

void test_emit_function_overwrite_var(void)
{
    const char *code =
        "let x = 10\n"
        "function setx(aaa) {\n"
        "   x = aaa\n"
        "   return x\n"
        "}\n"
        "let y = setx(5)\n";

    ASTNode *root = parse_script_from_string(code);
    statement_count = 0;
    reset_runtime_state();

    emit_gcode(root, get_debug());

    if (has_errors())
    {
        print_errors();
    }

    Value *x = get_var("x");
    Value *y = get_var("y");

    TEST_ASSERT_NOT_NULL(x);
    TEST_ASSERT_NOT_NULL(y);

    TEST_ASSERT_EQUAL_DOUBLE(5.0, x->number); // Global x must remain untouched
    TEST_ASSERT_EQUAL_DOUBLE(5.0, y->number);
    TEST_ASSERT_EQUAL_INT(2, statement_count); // let x, let y

    free_ast(root);
}

void test_emit_function_no_params_no_return(void)
{
    const char *code =
        "function foo() { let x = 1 }\n"
        "let y = foo()\n";
    ASTNode *root = parse_script_from_string(code);

    statement_count = 0;
    reset_runtime_state();

    emit_gcode(root, get_debug());

    Value *y = get_var("y");
    TEST_ASSERT_NOT_NULL(y);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, y->number); // No return value defaults to 0

    free_ast(root);
}

void test_emit_function_only_return(void)
{
    const char *code =
        "function ret42() { return 42 }\n"
        "let x = ret42()\n";
    ASTNode *root = parse_script_from_string(code);

    statement_count = 0;
    reset_runtime_state();

    emit_gcode(root, get_debug());

    Value *x = get_var("x");
    TEST_ASSERT_NOT_NULL(x);
    TEST_ASSERT_EQUAL_DOUBLE(42.0, x->number);

    free_ast(root);
}

void test_emit_function_unused_param(void)
{
    const char *code =
        "function foo(a, b) { return a }\n"
        "let x = foo(5, 99)\n";
    ASTNode *root = parse_script_from_string(code);

    statement_count = 0;
    reset_runtime_state();

    emit_gcode(root, get_debug());

    Value *x = get_var("x");
    TEST_ASSERT_NOT_NULL(x);
    TEST_ASSERT_EQUAL_DOUBLE(5.0, x->number);

    free_ast(root);
}

void test_emit_function_calls_function(void)
{
    const char *code =
        "function add1(x) { return x + 1 }\n"
        "function add2(x) { return add1(x) + 1 }\n"
        "let y = add2(3)\n";
    ASTNode *root = parse_script_from_string(code);

    statement_count = 0;
    reset_runtime_state();
    emit_gcode(root, get_debug());

    Value *y = get_var("y");
    TEST_ASSERT_NOT_NULL(y);
    TEST_ASSERT_EQUAL_DOUBLE(5.0, y->number); // 3 + 1 + 1 = 5

    free_ast(root);
}

void test_emit_function_early_return(void)
{
    const char *code =
        "function test(x) { if (x > 0) { return 1 } return 2 }\n"
        "let a = test(5)\n"
        "let b = test(-1)\n";
    ASTNode *root = parse_script_from_string(code);

    statement_count = 0;
    reset_runtime_state();
    emit_gcode(root, get_debug());

    Value *a = get_var("a");
    Value *b = get_var("b");

    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);

    TEST_ASSERT_EQUAL_DOUBLE(1.0, a->number);
    TEST_ASSERT_EQUAL_DOUBLE(2.0, b->number);

    free_ast(root);
}

void test_emit_function_empty_body(void)
{
    const char *code =
        "function empty() {}\n"
        "let x = empty()\n";
    ASTNode *root = parse_script_from_string(code);

    statement_count = 0;
    reset_runtime_state();
    emit_gcode(root, get_debug());

    Value *x = get_var("x");
    TEST_ASSERT_NOT_NULL(x);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, x->number); // No return = 0.0

    free_ast(root);
}

void test_builtin_math_functions_and_constants(void)
{
    const char *code =
        "let a = PI\n"
        "let b = EU\n"
        "let s = sin(PI / 2)\n"
        "let c = cos(0)\n"
        "let r = round(3.6)\n"
        "let f = floor(3.6)\n"
        "let u = ceil(3.1)\n"
        "let m = mod(10, 3)\n"
        "let ab = abs(-7)\n"
        "let sq = sqrt(16)";

    ASTNode *root = parse_script_from_string(code);

    statement_count = 0;
    reset_runtime_state();
    emit_gcode(root, get_debug());

    Value *a = get_var("a");
    Value *b = get_var("b");
    Value *s = get_var("s");
    Value *c = get_var("c");
    Value *r = get_var("r");
    Value *f = get_var("f");
    Value *u = get_var("u");
    Value *m = get_var("m");
    Value *ab = get_var("ab");
    Value *sq = get_var("sq");

    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_NOT_NULL(c);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_NOT_NULL(u);
    TEST_ASSERT_NOT_NULL(m);
    TEST_ASSERT_NOT_NULL(ab);
    TEST_ASSERT_NOT_NULL(sq);

    TEST_ASSERT_FLOAT_WITHIN(0.0001, 3.1415, a->number);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 2.7182, b->number);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 1.0, s->number);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, 1.0, c->number);
    TEST_ASSERT_EQUAL_DOUBLE(4.0, r->number);
    TEST_ASSERT_EQUAL_DOUBLE(3.0, f->number);
    TEST_ASSERT_EQUAL_DOUBLE(4.0, u->number);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, m->number);
    TEST_ASSERT_EQUAL_DOUBLE(7.0, ab->number);
    TEST_ASSERT_EQUAL_DOUBLE(4.0, sq->number);

    free_ast(root);
}

void test_builtin_min_max(void)
{
    const char *code =
        "let a = min(3, 5)\n"
        "let b = max(3, 5)\n";

    ASTNode *root = parse_script_from_string(code);

    statement_count = 0;
    reset_runtime_state();
    emit_gcode(root, get_debug());

    Value *a = get_var("a");
    Value *b = get_var("b");

    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);

    TEST_ASSERT_FLOAT_WITHIN(0.001, 3.0, a->number);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 5.0, b->number);

    free_ast(root);
}

void test_builtin_clamp(void)
{
    const char *code =
        "let clamp_low = clamp(2, 3, 5)\n"
        "let clamp_high = clamp(6, 3, 5)\n"
        "let clamp_mid = clamp(4, 3, 5)\n";

    ASTNode *root = parse_script_from_string(code);

    statement_count = 0;
    reset_runtime_state();
    emit_gcode(root, get_debug());

    Value *low = get_var("clamp_low");
    Value *high = get_var("clamp_high");
    Value *mid = get_var("clamp_mid");

    TEST_ASSERT_NOT_NULL(low);
    TEST_ASSERT_NOT_NULL(high);
    TEST_ASSERT_NOT_NULL(mid);

    TEST_ASSERT_FLOAT_WITHIN(0.001, 3.0, low->number);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 5.0, high->number);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 4.0, mid->number);

    free_ast(root);
}

void test_complex_gcode_logic(void)
{
    const char *code =
        "let x = 0\n"
        "let flag = 0\n"
        "function double(n) { return n * 2 }\n"
        "function my_clamp(ggh) {\n"
        "  if (ggh > 0) { return 0 }\n"
        "  if (ggh < 10) { return 10 }\n"
        "  return 55\n"
        "}\n"
        "x = double(double(1)) + my_clamp(-5) + my_clamp(25)\n"
        "if (x > 5) {\n"
        "  flag = 1\n"
        "  note {Flag raised, x = [x]}\n"
        "} else {\n"
        "  flag = -1\n"
        "  note {Flag lowered, x = [x]}\n"
        "}\n"
        "for i = -2..4 {\n"
        "  let z = my_clamp(i * 3 - 1)\n"
        "  G1 X[i] Y[z]\n"
        "  if (z > 0) {\n"
        "    note {Z hit lower bound}\n"
        "  } else if (z == 10) {\n"
        "    note {Z hit upper bound}\n"
        "  }\n"
        "}\n"
        "note {Final X = [x], Flag = [flag]}\n";
    ASTNode *root = parse_script_from_string(code);
    statement_count = 0;
    reset_runtime_state();
    emit_gcode(root, get_debug());
    Value *x = get_var("x");
    Value *flag = get_var("flag");
    TEST_ASSERT_NOT_NULL(x);
    TEST_ASSERT_NOT_NULL(flag);
    TEST_ASSERT_EQUAL_DOUBLE(14.0, x->number);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, flag->number);
    free_ast(root);
}

void test_builtin_trig_pow_hypot(void)
{
    const char *code =
        "let atan2_d = atan2(1, 1)\n"
        "let pow_d = pow(2, 3)\n"
        "let hypot_d = hypot(3, 4)\n";

    ASTNode *root = parse_script_from_string(code);
    statement_count = 0;
    reset_runtime_state();
    emit_gcode(root, get_debug());

    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.7853, get_var("atan2_d")->number);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 8.0, get_var("pow_d")->number);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 5.0, get_var("hypot_d")->number);

    free_ast(root);
}

void test_builtin_deg_rad_sign(void)
{
    const char *code =
        "let deg_d = deg(PI)\n"
        "let rad_d = rad(180)\n"
        "let sign_neg = sign(-20)\n"
        "let sign_pos = sign(20)\n"
        "let sign_zero = sign(0)\n";

    ASTNode *root = parse_script_from_string(code);
    statement_count = 0;
    reset_runtime_state();
    emit_gcode(root, get_debug());

    TEST_ASSERT_FLOAT_WITHIN(0.001, 180.0, get_var("deg_d")->number);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 3.1415, get_var("rad_d")->number);
    TEST_ASSERT_FLOAT_WITHIN(0.001, -1.0, get_var("sign_neg")->number);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 1.0, get_var("sign_pos")->number);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, get_var("sign_zero")->number);

    free_ast(root);
}

void test_builtin_log(void)
{
    const char *code = "let val_log = round(log(EU))\n";

    ASTNode *root = parse_script_from_string(code);
    statement_count = 0;
    reset_runtime_state();
    emit_gcode(root, get_debug());

    TEST_ASSERT_FLOAT_WITHIN(0.001, 1.0, get_var("val_log")->number); // log(E) ≈ 1

    free_ast(root);
}

void test_builtin_exp(void)
{
    const char *code = "let val_exp = round(exp(1))\n";

    ASTNode *root = parse_script_from_string(code);
    statement_count = 0;
    reset_runtime_state();
    emit_gcode(root, get_debug());

    TEST_ASSERT_FLOAT_WITHIN(0.001, 3.0, get_var("val_exp")->number); // round(2.718)

    free_ast(root);
}

void test_builtin_noise_zero(void)
{
    const char *code = "let noise_val = noise(0)\n";

    ASTNode *root = parse_script_from_string(code);
    statement_count = 0;
    reset_runtime_state();
    emit_gcode(root, get_debug());

    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, get_var("noise_val")->number); // Expect 0 at seed 0

    free_ast(root);
}

void test_emit_function_no_return(void)
{
    const char *code =
        "function doNothing(x) {\n"
        " let  y = x + 1\n"
        "}\n"
        "let abc = doNothing(5)\n";

    ASTNode *root = parse_script_from_string(code);
    statement_count = 0;
    reset_runtime_state();
    emit_gcode(root, get_debug());

    Value *abc = get_var("abc");
    TEST_ASSERT_NOT_NULL(abc);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, abc->number); // No return = 0.0 by convention
    TEST_ASSERT_EQUAL_INT(1, statement_count);  //  let abc

    free_ast(root);
}

void test_array_assignment_and_access(void)
{

    const char *code =
        "let grid = [[1, 2], [3, 4]]\n"
        "let val = grid[1][0]\n";

    ASTNode *root = parse_script_from_string(code);
    reset_runtime_state();
    emit_gcode(root, 0);

    if (has_errors())
        print_errors();

    Value *val = get_var("val");
    TEST_ASSERT_NOT_NULL(val);
    TEST_ASSERT_EQUAL_DOUBLE(3.0, val->number);

    free_ast(root);
}

void test_emit_maze_generator_program(void)
{

    const char *code =
        "let maze = []\n"
        "let row = []\n"
        "row[0] = 1\n"
        "row[1] = 1\n"
        "row[2] = 1\n"
        "maze[0] = row\n"
        "let px = 1\n"
        "let py = 0\n"
        "maze[py][px] = 0\n";

    ASTNode *root = parse_script_from_string(code);
    emit_gcode(root, 0);

    Value *maze = get_var("maze");
    TEST_ASSERT_NOT_NULL(maze);
    TEST_ASSERT_EQUAL(VAL_ARRAY, maze->type);
    TEST_ASSERT_TRUE(maze->array.count > 0);

    Value *row0 = maze->array.items[0];
    TEST_ASSERT_NOT_NULL(row0);
    TEST_ASSERT_EQUAL(VAL_ARRAY, row0->type);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, row0->array.items[1]->number);

    free_ast(root);
}

void test_emit_function_statement_and_expression(void)
{
    const char *code =
        "function foo(a) {\n"
        "  G1 X[a]\n"
        "}\n"
        "foo(5)\n"                // standalone call — should emit G-code
        "let result = foo(10)\n"; // expression call — should return 0.0

    ASTNode *root = parse_script_from_string(code);
    statement_count = 0;
    reset_runtime_state();
    emit_gcode(root, get_debug());

    // Validate variable assignment
    Value *res = get_var("result");
    TEST_ASSERT_NOT_NULL(res);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, res->number); // No return = 0.0

    // ✅ Only G1 and let should count
    TEST_ASSERT_EQUAL_INT(5, statement_count);

    free_ast(root);
}

int main(void)
{

    UNITY_BEGIN();

       RUN_TEST(test_array_assignment_and_access);          // 1
    RUN_TEST(test_emit_maze_generator_program);            // 2
    RUN_TEST(test_emit_function_overwrite_var);            // 3
    RUN_TEST(test_emit_function_unused_param);             // 4
    RUN_TEST(test_emit_function_only_return);              // 5
    RUN_TEST(test_emit_function_no_params_no_return);      // 6
    RUN_TEST(test_emit_loop_and_conditionals);             // 7
    RUN_TEST(test_emit_while_loop_basic);                  // 8
    RUN_TEST(test_emit_nested_if_inside_loop);             // 9
    RUN_TEST(test_emit_simple_gcode_block);                // 10
    RUN_TEST(test_builtin_clamp);                          // 11
    RUN_TEST(test_builtin_math_functions_and_constants);   // 12
    RUN_TEST(test_emit_nested_if_inside_loop2);            // 13
    RUN_TEST(test_emit_function_declaration_and_call);     // 14
    RUN_TEST(test_emit_function_call_returns_value);       // 15
    RUN_TEST(test_emit_function_call_inside_expression);   // 16
    RUN_TEST(test_emit_function_with_two_params);          // 17
    RUN_TEST(test_emit_function_recursion);                // 18
    RUN_TEST(test_emit_function_in_condition);             // 19
    RUN_TEST(test_emit_bitwise_and);                       // 20
    RUN_TEST(test_emit_function_calls_function);           // 21
    RUN_TEST(test_emit_function_early_return);             // 22
    RUN_TEST(test_complex_gcode_logic);                    // 23
    RUN_TEST(test_builtin_exp);                            // 24
    RUN_TEST(test_builtin_deg_rad_sign);                   // 25
    RUN_TEST(test_emit_function_no_return);                // 26
    RUN_TEST(test_builtin_log);                            // 27
    RUN_TEST(test_builtin_noise_zero);                     // 28
    RUN_TEST(test_builtin_trig_pow_hypot);                 // 29
    RUN_TEST(test_emit_function_expr_args);                // 30
    RUN_TEST(test_builtin_min_max);                        // 31
    RUN_TEST(test_emit_function_empty_body);               // 32
    RUN_TEST(test_emit_function_statement_and_expression); // 33

    ////333

    return UNITY_END();
}
