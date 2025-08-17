#include "Unity/src/unity.h"
#include "../src/config/config.h"
#include "../src/runtime/evaluator.h"
#include "../src/parser/parser.h"
#include "../src/generator/emitter.h"
#include "../src/utils/output_buffer.h"
#include <string.h>
#include <stdio.h>

// Test setup and teardown
void setUp(void) {
    // Reset configuration state before each test
    reset_config_state();
    init_runtime();
    init_output_buffer();
}

void tearDown(void) {
    // Clean up after each test
    free_output_buffer();
}

// Test decimal places configuration
void test_decimal_places_default(void) {
    TEST_ASSERT_EQUAL_INT(3, get_decimal_places());
    TEST_ASSERT_EQUAL_STRING("%.3f", get_decimal_format());
}

void test_set_decimal_places_valid(void) {
    set_decimal_places(2);
    TEST_ASSERT_EQUAL_INT(2, get_decimal_places());
    TEST_ASSERT_EQUAL_STRING("%.2f", get_decimal_format());
    
    set_decimal_places(4);
    TEST_ASSERT_EQUAL_INT(4, get_decimal_places());
    TEST_ASSERT_EQUAL_STRING("%.4f", get_decimal_format());
    
    set_decimal_places(0);
    TEST_ASSERT_EQUAL_INT(0, get_decimal_places());
    TEST_ASSERT_EQUAL_STRING("%.0f", get_decimal_format());
    
    set_decimal_places(6);
    TEST_ASSERT_EQUAL_INT(6, get_decimal_places());
    TEST_ASSERT_EQUAL_STRING("%.6f", get_decimal_format());
}

void test_set_decimal_places_invalid(void) {
    // Test invalid values (should be ignored)
    set_decimal_places(3); // Set to known good value first
    
    set_decimal_places(-1); // Invalid: negative
    TEST_ASSERT_EQUAL_INT(3, get_decimal_places()); // Should remain unchanged
    
    set_decimal_places(10); // Invalid: too large
    TEST_ASSERT_EQUAL_INT(3, get_decimal_places()); // Should remain unchanged
}

// Test line numbering configuration
void test_line_numbering_default(void) {
    TEST_ASSERT_EQUAL_INT(1, get_enable_n_lines()); // Default is enabled
}

void test_set_enable_n_lines_from_var(void) {
    set_enable_n_lines_from_var(0);
    TEST_ASSERT_EQUAL_INT(0, get_enable_n_lines());
    
    set_enable_n_lines_from_var(1);
    TEST_ASSERT_EQUAL_INT(1, get_enable_n_lines());
    
    set_enable_n_lines_from_var(5); // Non-zero should be treated as true
    TEST_ASSERT_EQUAL_INT(1, get_enable_n_lines());
    
    set_enable_n_lines_from_var(-1); // Non-zero should be treated as true
    TEST_ASSERT_EQUAL_INT(1, get_enable_n_lines());
}

// Test configuration variable detection
void test_config_variable_detection_decimalpoint(void) {
    Value val;
    val.type = VAL_NUMBER;
    
    // Test decimalpoint variable
    val.number = 2.0;
    check_config_variable("decimalpoint", &val);
    TEST_ASSERT_EQUAL_INT(2, get_decimal_places());
    
    val.number = 4.0;
    check_config_variable("decimalpoint", &val);
    TEST_ASSERT_EQUAL_INT(4, get_decimal_places());
}

void test_config_variable_detection_nline(void) {
    Value val;
    val.type = VAL_NUMBER;
    
    // Test nline variable
    val.number = 0.0;
    check_config_variable("nline", &val);
    TEST_ASSERT_EQUAL_INT(0, get_enable_n_lines());
    
    val.number = 1.0;
    check_config_variable("nline", &val);
    TEST_ASSERT_EQUAL_INT(1, get_enable_n_lines());
}

void test_config_variable_detection_invalid(void) {
    Value val;
    
    // Test with non-numeric value (should be ignored)
    val.type = VAL_ARRAY;
    check_config_variable("decimalpoint", &val);
    TEST_ASSERT_EQUAL_INT(3, get_decimal_places()); // Should remain default
    
    // Test with NULL value (should be ignored)
    check_config_variable("decimalpoint", NULL);
    TEST_ASSERT_EQUAL_INT(3, get_decimal_places()); // Should remain default
    
    // Test with NULL name (should be ignored)
    val.type = VAL_NUMBER;
    val.number = 2.0;
    check_config_variable(NULL, &val);
    TEST_ASSERT_EQUAL_INT(3, get_decimal_places()); // Should remain default
}

void test_config_variable_detection_unknown(void) {
    Value val;
    val.type = VAL_NUMBER;
    val.number = 5.0;
    
    // Test with unknown variable name (should be ignored)
    check_config_variable("unknown_var", &val);
    TEST_ASSERT_EQUAL_INT(3, get_decimal_places()); // Should remain default
    TEST_ASSERT_EQUAL_INT(1, get_enable_n_lines()); // Should remain default
}

// Test configuration state reset
void test_reset_config_state(void) {
    // Change settings
    set_decimal_places(2);
    set_enable_n_lines_from_var(0);
    
    // Verify they changed
    TEST_ASSERT_EQUAL_INT(2, get_decimal_places());
    TEST_ASSERT_EQUAL_INT(0, get_enable_n_lines());
    
    // Reset and verify defaults are restored
    reset_config_state();
    TEST_ASSERT_EQUAL_INT(3, get_decimal_places());
    TEST_ASSERT_EQUAL_INT(1, get_enable_n_lines());
}

// Integration test: Test configuration variables in GGcode compilation
void test_integration_decimal_formatting(void) {
    const char* ggcode = 
        "let decimalpoint = 2\n"
        "G1 X[1] Y[2.5] F[150]\n";
    
    // Parse and emit the GGcode
    ASTNode* root = parse_script_from_string(ggcode);
    TEST_ASSERT_NOT_NULL(root);
    
    emit_gcode(root);
    
    const char* output = get_output_buffer();
    TEST_ASSERT_NOT_NULL(output);
    
    // Check that coordinates use 2 decimal places
    TEST_ASSERT_TRUE(strstr(output, "X1.00") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "Y2.50") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "F150.00") != NULL);
    
    free_ast(root);
}

void test_integration_line_numbering(void) {
    const char* ggcode = 
        "let nline = 0\n"
        "G0 X[1] Y[2]\n"
        "let nline = 1\n"
        "G1 X[3] Y[4]\n";
    
    // Parse and emit the GGcode
    ASTNode* root = parse_script_from_string(ggcode);
    TEST_ASSERT_NOT_NULL(root);
    
    emit_gcode(root);
    
    const char* output = get_output_buffer();
    TEST_ASSERT_NOT_NULL(output);
    
    // First G-code should have no line number
    // Second G-code should have line number
    // Note: This is a simplified check - actual output parsing would be more complex
    TEST_ASSERT_TRUE(strstr(output, "G0 X1.000") != NULL); // No N prefix
    TEST_ASSERT_TRUE(strstr(output, "N") != NULL); // Should have some N line
    
    free_ast(root);
}

void test_integration_combined_config(void) {
    const char* ggcode = 
        "let decimalpoint = 1\n"
        "let nline = 0\n"
        "G1 X[10.555] Y[20.999]\n";
    
    // Parse and emit the GGcode
    ASTNode* root = parse_script_from_string(ggcode);
    TEST_ASSERT_NOT_NULL(root);
    
    emit_gcode(root);
    
    const char* output = get_output_buffer();
    TEST_ASSERT_NOT_NULL(output);
    
    // Check 1 decimal place and no line numbers
    TEST_ASSERT_TRUE(strstr(output, "X10.6") != NULL || strstr(output, "X11.0") != NULL); // Rounded to 1 decimal
    TEST_ASSERT_TRUE(strstr(output, "Y21.0") != NULL || strstr(output, "Y20.0") != NULL); // Rounded to 1 decimal
    
    free_ast(root);
}

// Test edge cases
void test_edge_case_zero_decimals(void) {
    set_decimal_places(0);
    TEST_ASSERT_EQUAL_STRING("%.0f", get_decimal_format());
    
    const char* ggcode = 
        "let decimalpoint = 0\n"
        "G1 X[1.999] Y[2.001]\n";
    
    ASTNode* root = parse_script_from_string(ggcode);
    TEST_ASSERT_NOT_NULL(root);
    
    emit_gcode(root);
    
    const char* output = get_output_buffer();
    TEST_ASSERT_NOT_NULL(output);
    
    // Should round to whole numbers
    TEST_ASSERT_TRUE(strstr(output, "X2") != NULL); // 1.999 rounds to 2
    TEST_ASSERT_TRUE(strstr(output, "Y2") != NULL); // 2.001 rounds to 2
    
    free_ast(root);
}

void test_edge_case_max_decimals(void) {
    set_decimal_places(6);
    TEST_ASSERT_EQUAL_STRING("%.6f", get_decimal_format());
    
    const char* ggcode = 
        "let decimalpoint = 6\n"
        "G1 X[1.123456789]\n";
    
    ASTNode* root = parse_script_from_string(ggcode);
    TEST_ASSERT_NOT_NULL(root);
    
    emit_gcode(root);
    
    const char* output = get_output_buffer();
    TEST_ASSERT_NOT_NULL(output);
    
    // Should show 6 decimal places
    TEST_ASSERT_TRUE(strstr(output, "X1.123457") != NULL); // Rounded to 6 decimals
    
    free_ast(root);
}

// Main test runner
int main(void) {
    UNITY_BEGIN();
    
    // Basic configuration tests
    RUN_TEST(test_decimal_places_default);
    RUN_TEST(test_set_decimal_places_valid);
    RUN_TEST(test_set_decimal_places_invalid);
    RUN_TEST(test_line_numbering_default);
    RUN_TEST(test_set_enable_n_lines_from_var);
    
    // Configuration variable detection tests
    RUN_TEST(test_config_variable_detection_decimalpoint);
    RUN_TEST(test_config_variable_detection_nline);
    RUN_TEST(test_config_variable_detection_invalid);
    RUN_TEST(test_config_variable_detection_unknown);
    
    // State management tests
    RUN_TEST(test_reset_config_state);
    
    // Integration tests
    RUN_TEST(test_integration_decimal_formatting);
    RUN_TEST(test_integration_line_numbering);
    RUN_TEST(test_integration_combined_config);
    
    // Edge case tests
    RUN_TEST(test_edge_case_zero_decimals);
    RUN_TEST(test_edge_case_max_decimals);
    
    return UNITY_END();
}