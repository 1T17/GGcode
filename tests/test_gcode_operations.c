#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/runtime/evaluator.h"
#include "../src/parser/parser.h"
#include "../src/runtime/runtime_state.h"
#include "../src/generator/emitter.h"
#include "Unity/src/unity.h"

void setUp(void) {
    reset_runtime_state();
}

void tearDown(void) {
    reset_runtime_state();
}

// Test G-code generation with ternary operations
void test_gcode_with_ternary_operations(void) {
    const char* source = 
        "let material = 2\n"
        "let x_pos = 50\n"
        "let y_pos = 30\n"
        "let feed_rate = material == 1 ? 1500 : material == 2 ? 1000 : 800\n"
        "let safe_x = x_pos > 100 ? 100 : x_pos\n"
        "let safe_y = y_pos > 100 ? 100 : y_pos\n"
        "G[1] X[safe_x] Y[safe_y] F[feed_rate]\n";
    
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    // Check that variables were set correctly
    Value* feed_rate = get_var("feed_rate");
    Value* safe_x = get_var("safe_x");
    Value* safe_y = get_var("safe_y");
    
    TEST_ASSERT_NOT_NULL(feed_rate);
    TEST_ASSERT_NOT_NULL(safe_x);
    TEST_ASSERT_NOT_NULL(safe_y);
    TEST_ASSERT_EQUAL_FLOAT(1000.0, feed_rate->number); // material == 2
    TEST_ASSERT_EQUAL_FLOAT(50.0, safe_x->number);      // x_pos <= 100
    TEST_ASSERT_EQUAL_FLOAT(30.0, safe_y->number);      // y_pos <= 100
    
    free_ast(ast);
}

// Test M-code generation with bitwise operations
void test_mcode_with_bitwise_operations(void) {
    const char* source = 
        "let base_speed = 1000\n"
        "let speed_flags = 5\n"      // 101 in binary
        "let precision_flag = 4\n"   // 100 in binary
        "let final_speed = (speed_flags & precision_flag) != 0 ? base_speed >> 1 : base_speed\n"
        "M[3] S[final_speed]\n";
    
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* final_speed = get_var("final_speed");
    TEST_ASSERT_NOT_NULL(final_speed);
    // speed_flags & precision_flag = 5 & 4 = 4 (not zero), so final_speed = 1000 >> 1 = 500
    TEST_ASSERT_EQUAL_FLOAT(500.0, final_speed->number);
    
    free_ast(ast);
}

// Test tool selection with compound assignments
void test_tool_selection_with_compound_assignments(void) {
    const char* source = 
        "let material_hardness = 7\n"
        "let tool_wear = 2\n"
        "let base_tool = 1\n"
        "base_tool += material_hardness > 5 ? 2 : 1\n"  // base_tool = 1 + 2 = 3
        "base_tool += tool_wear\n"                       // base_tool = 3 + 2 = 5
        "T[base_tool]\n"
        "M[6]\n";  // Tool change
    
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* base_tool = get_var("base_tool");
    TEST_ASSERT_NOT_NULL(base_tool);
    TEST_ASSERT_EQUAL_FLOAT(5.0, base_tool->number);
    
    free_ast(ast);
}

// Test complex G-code generation with functions and operations
void test_complex_gcode_with_functions(void) {
    const char* source = 
        "function calculate_arc_params(radius, angle, precision) {\n"
        "    let steps = precision ? radius << 2 : radius << 1\n"  // More steps for precision
        "    return steps > 32 ? 32 : steps\n"  // Clamp to maximum
        "}\n"
        "function get_feed_for_arc(radius) {\n"
        "    return radius < 5 ? 800 : radius < 10 ? 1000 : 1200\n"
        "}\n"
        "let arc_radius = 8\n"
        "let precision_mode = 1\n"
        "let steps = calculate_arc_params(arc_radius, 90, precision_mode)\n"
        "let arc_feed = get_feed_for_arc(arc_radius)\n"
        "G[2] X[50] Y[50] R[arc_radius] F[arc_feed]\n";
    
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* steps = get_var("steps");
    Value* arc_feed = get_var("arc_feed");
    TEST_ASSERT_NOT_NULL(steps);
    TEST_ASSERT_NOT_NULL(arc_feed);
    // steps = precision ? 8 << 2 : 8 << 1 = 32 (clamped to 32)
    TEST_ASSERT_EQUAL_FLOAT(32.0, steps->number);
    // arc_feed = radius < 10 ? 1000 : 1200 = 1000
    TEST_ASSERT_EQUAL_FLOAT(1000.0, arc_feed->number);
    
    free_ast(ast);
}

// Test dwell command with calculated timing
void test_dwell_with_calculated_timing(void) {
    const char* source = 
        "let spindle_speed = 2000\n"
        "let material_density = 3\n"  // 1=soft, 2=medium, 3=hard
        "let base_dwell = 1.0\n"
        "let dwell_time = base_dwell * (material_density > 2 ? 2.5 : 1.5)\n"
        "dwell_time += spindle_speed > 1500 ? 0.5 : 0.0\n"
        "M[3] S[spindle_speed]\n"
        "G[4] P[dwell_time]\n";
    
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* dwell_time = get_var("dwell_time");
    TEST_ASSERT_NOT_NULL(dwell_time);
    // dwell_time = 1.0 * 2.5 + 0.5 = 3.0
    TEST_ASSERT_EQUAL_FLOAT(3.0, dwell_time->number);
    
    free_ast(ast);
}

// Test coordinate transformation with bitwise flags
void test_coordinate_transformation_with_flags(void) {
    const char* source = 
        "let transform_flags = 6\n"  // 110 in binary (mirror_x=1, mirror_y=1, rotate=0)
        "let x_coord = 25\n"
        "let y_coord = 15\n"
        "let mirror_x_flag = 2\n"    // 010 in binary
        "let mirror_y_flag = 4\n"    // 100 in binary
        "let final_x = (transform_flags & mirror_x_flag) != 0 ? -x_coord : x_coord\n"
        "let final_y = (transform_flags & mirror_y_flag) != 0 ? -y_coord : y_coord\n"
        "G[0] X[final_x] Y[final_y]\n";
    
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* final_x = get_var("final_x");
    Value* final_y = get_var("final_y");
    TEST_ASSERT_NOT_NULL(final_x);
    TEST_ASSERT_NOT_NULL(final_y);
    // transform_flags & mirror_x_flag = 6 & 2 = 2 (not zero), so final_x = -25
    // transform_flags & mirror_y_flag = 6 & 4 = 4 (not zero), so final_y = -15
    TEST_ASSERT_EQUAL_FLOAT(-25.0, final_x->number);
    TEST_ASSERT_EQUAL_FLOAT(-15.0, final_y->number);
    
    free_ast(ast);
}

// Test loop-based G-code generation with operations
void test_loop_gcode_generation_with_operations(void) {
    const char* source = 
        "let step_size = 5\n"
        "let precision = 1\n"
        "let actual_step = precision ? step_size >> 1 : step_size\n"  // Half step for precision
        "let feed_rate = 1000\n"
        "for i = 0..3 {\n"
        "    let x = i * actual_step\n"
        "    let y = (i & 1) != 0 ? 10 : 0\n"  // Alternate between y=10 and y=0
        "    let current_feed = feed_rate + (i << 6)\n"  // Increase feed by 64 each step
        "    G[1] X[x] Y[y] F[current_feed]\n"
        "}\n";
    
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* actual_step = get_var("actual_step");
    TEST_ASSERT_NOT_NULL(actual_step);
    // precision=1, so actual_step = 5 >> 1 = 2 (integer division)
    TEST_ASSERT_EQUAL_FLOAT(2.0, actual_step->number);
    
    free_ast(ast);
}

// Test conditional G-code generation with nested operations
void test_conditional_gcode_with_nested_operations(void) {
    const char* source = 
        "function select_operation(material, thickness) {\n"
        "    let operation = material << 2\n"  // Base operation code
        "    operation |= thickness > 5 ? 2 : 1\n"  // Add thickness flag
        "    return operation\n"
        "}\n"
        "let material_type = 2\n"
        "let part_thickness = 8\n"
        "let operation_code = select_operation(material_type, part_thickness)\n"
        "if (operation_code & 8) != 0 {\n"  // Check if material_type >= 2
        "    let drill_speed = (operation_code & 2) != 0 ? 800 : 1200\n"  // Check thickness flag
        "    M[3] S[drill_speed]\n"
        "    G[81] X[0] Y[0] Z[-5] R[2] F[100]\n"  // Drilling cycle
        "} else {\n"
        "    M[3] S[1500]\n"
        "    G[1] X[0] Y[0] F[1000]\n"  // Regular move
        "}\n";
    
    ASTNode* ast = parse_script_from_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    for (int i = 0; i < ast->block.count; i++) {
        eval_expr(ast->block.statements[i]);
    }
    
    Value* operation_code = get_var("operation_code");
    TEST_ASSERT_NOT_NULL(operation_code);
    // operation = 2 << 2 = 8, operation |= 8 > 5 ? 2 : 1 = 8 | 2 = 10
    TEST_ASSERT_EQUAL_FLOAT(10.0, operation_code->number);
    
    free_ast(ast);
}

// Unity test runner setup
int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_gcode_with_ternary_operations);
    RUN_TEST(test_mcode_with_bitwise_operations);
    RUN_TEST(test_tool_selection_with_compound_assignments);
    RUN_TEST(test_complex_gcode_with_functions);
    RUN_TEST(test_dwell_with_calculated_timing);
    RUN_TEST(test_coordinate_transformation_with_flags);
    RUN_TEST(test_loop_gcode_generation_with_operations);
    RUN_TEST(test_conditional_gcode_with_nested_operations);
    
    return UNITY_END();
}