#include "Unity/src/unity.h"
#include "../src/parser/parser.h"
#include "../src/lexer/lexer.h"
#include "../src/runtime/evaluator.h"
#include "../src/generator/emitter.h"
#include "../src/runtime/runtime_state.h"
#include "../src/config/config.h"

void setUp(void) {
    reset_runtime_state();
}

void tearDown(void) {
    reset_runtime_state();
}

// Test that string variables work with declare_var() and get_var()
void test_string_declare_var_get_var(void)
{
    // Test declare_var with string value
    Value *str_val = make_string_value("test string");
    declare_var("test_var", str_val);
    
    // Test get_var retrieves the string correctly
    Value *retrieved = get_var("test_var");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_INT(VAL_STRING, retrieved->type);
    TEST_ASSERT_EQUAL_STRING("test string", retrieved->string);
    
    // Test with empty string
    Value *empty_val = make_string_value("");
    declare_var("empty_var", empty_val);
    
    Value *empty_retrieved = get_var("empty_var");
    TEST_ASSERT_NOT_NULL(empty_retrieved);
    TEST_ASSERT_EQUAL_INT(VAL_STRING, empty_retrieved->type);
    TEST_ASSERT_EQUAL_STRING("", empty_retrieved->string);
    
    // Test with NULL string (should create empty string)
    Value *null_val = make_string_value(NULL);
    declare_var("null_var", null_val);
    
    Value *null_retrieved = get_var("null_var");
    TEST_ASSERT_NOT_NULL(null_retrieved);
    TEST_ASSERT_EQUAL_INT(VAL_STRING, null_retrieved->type);
    TEST_ASSERT_EQUAL_STRING("", null_retrieved->string);
}

// Test string variable assignment and retrieval
void test_string_variable_assignment_retrieval(void)
{
    ASTNode *root = parse_script_from_string("let message = \"Hello, World!\"");
    
    // Execute the let statement
    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("message", val);
    
    // Retrieve and verify
    Value *retrieved = get_var("message");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_INT(VAL_STRING, retrieved->type);
    TEST_ASSERT_EQUAL_STRING("Hello, World!", retrieved->string);
    
    free_ast(root);
}

// Test string variable reassignment
void test_string_variable_reassignment(void)
{
    ASTNode *root = parse_script_from_string("let greeting = \"Hello\"\ngreeting = \"Updated\"");
    
    // Execute initial assignment
    Value *initial_val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("greeting", initial_val);
    
    // Verify initial value
    Value *initial_retrieved = get_var("greeting");
    TEST_ASSERT_NOT_NULL(initial_retrieved);
    TEST_ASSERT_EQUAL_STRING("Hello", initial_retrieved->string);
    
    // Execute reassignment
    Value *new_val = eval_expr(root->block.statements[1]->assign_stmt.expr);
    set_var("greeting", new_val);
    
    // Verify updated value
    Value *updated_retrieved = get_var("greeting");
    TEST_ASSERT_NOT_NULL(updated_retrieved);
    TEST_ASSERT_EQUAL_STRING("Updated", updated_retrieved->string);
    
    free_ast(root);
}

// Test string variable scoping behavior
void test_string_variable_scoping(void)
{
    // Test global scope string
    Value *global_str = make_string_value("global string");
    declare_var("global_var", global_str);
    
    // Enter new scope
    enter_scope();
    
    // Declare string in inner scope
    Value *local_str = make_string_value("local string");
    declare_var("local_var", local_str);
    
    // Both should be accessible
    Value *global_retrieved = get_var("global_var");
    Value *local_retrieved = get_var("local_var");
    
    TEST_ASSERT_NOT_NULL(global_retrieved);
    TEST_ASSERT_NOT_NULL(local_retrieved);
    TEST_ASSERT_EQUAL_STRING("global string", global_retrieved->string);
    TEST_ASSERT_EQUAL_STRING("local string", local_retrieved->string);
    
    // Shadow global variable with string
    Value *shadow_str = make_string_value("shadowed string");
    declare_var("global_var", shadow_str);
    
    Value *shadowed_retrieved = get_var("global_var");
    TEST_ASSERT_NOT_NULL(shadowed_retrieved);
    TEST_ASSERT_EQUAL_STRING("shadowed string", shadowed_retrieved->string);
    
    // Exit scope
    exit_scope();
    
    // Global should be restored, local should be gone
    Value *restored_global = get_var("global_var");
    TEST_ASSERT_NOT_NULL(restored_global);
    TEST_ASSERT_EQUAL_STRING("global string", restored_global->string);
    
    // Local variable should no longer exist (this will generate an error, but that's expected)
    // We can't easily test this without modifying get_var to not report errors
}

// Test string variables with let statements (integration test)
void test_string_let_statements_integration(void)
{
    ASTNode *root = parse_script_from_string(
        "let name = \"Alice\"\n"
        "let greeting = \"Hello\"\n"
        "let message = name\n"
        "let empty = \"\""
    );
    
    // Execute all let statements
    for (int i = 0; i < root->block.count; i++) {
        ASTNode *stmt = root->block.statements[i];
        if (stmt->type == AST_LET) {
            Value *val = eval_expr(stmt->let_stmt.expr);
            set_var(stmt->let_stmt.name, val);
        }
    }
    
    // Verify all variables
    Value *name = get_var("name");
    Value *greeting = get_var("greeting");
    Value *message = get_var("message");
    Value *empty = get_var("empty");
    
    TEST_ASSERT_NOT_NULL(name);
    TEST_ASSERT_NOT_NULL(greeting);
    TEST_ASSERT_NOT_NULL(message);
    TEST_ASSERT_NOT_NULL(empty);
    
    TEST_ASSERT_EQUAL_INT(VAL_STRING, name->type);
    TEST_ASSERT_EQUAL_INT(VAL_STRING, greeting->type);
    TEST_ASSERT_EQUAL_INT(VAL_STRING, message->type);
    TEST_ASSERT_EQUAL_INT(VAL_STRING, empty->type);
    
    TEST_ASSERT_EQUAL_STRING("Alice", name->string);
    TEST_ASSERT_EQUAL_STRING("Hello", greeting->string);
    TEST_ASSERT_EQUAL_STRING("Alice", message->string);
    TEST_ASSERT_EQUAL_STRING("", empty->string);
    
    free_ast(root);
}

// Test string variables with escape sequences
void test_string_escape_sequences_integration(void)
{
    ASTNode *root = parse_script_from_string("let escaped = \"He said \\\"hello\\\"\\nNext line\\tTabbed\\\\path\"");
    
    Value *val = eval_expr(root->block.statements[0]->let_stmt.expr);
    set_var("escaped", val);
    
    Value *retrieved = get_var("escaped");
    TEST_ASSERT_NOT_NULL(retrieved);
    TEST_ASSERT_EQUAL_INT(VAL_STRING, retrieved->type);
    TEST_ASSERT_EQUAL_STRING("He said \"hello\"\nNext line\tTabbed\\path", retrieved->string);
    
    free_ast(root);
}

// Test mixed variable types (string, number, array) in same scope
void test_mixed_variable_types_integration(void)
{
    ASTNode *root = parse_script_from_string(
        "let str_var = \"text\"\n"
        "let num_var = 42.5\n"
        "let arr_var = [1, 2, 3]"
    );
    
    // Execute all statements
    for (int i = 0; i < root->block.count; i++) {
        ASTNode *stmt = root->block.statements[i];
        if (stmt->type == AST_LET) {
            Value *val = eval_expr(stmt->let_stmt.expr);
            set_var(stmt->let_stmt.name, val);
        }
    }
    
    // Verify all variables have correct types
    Value *str_var = get_var("str_var");
    Value *num_var = get_var("num_var");
    Value *arr_var = get_var("arr_var");
    
    TEST_ASSERT_NOT_NULL(str_var);
    TEST_ASSERT_NOT_NULL(num_var);
    TEST_ASSERT_NOT_NULL(arr_var);
    
    TEST_ASSERT_EQUAL_INT(VAL_STRING, str_var->type);
    TEST_ASSERT_EQUAL_INT(VAL_NUMBER, num_var->type);
    TEST_ASSERT_EQUAL_INT(VAL_ARRAY, arr_var->type);
    
    TEST_ASSERT_EQUAL_STRING("text", str_var->string);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 42.5, num_var->number);
    TEST_ASSERT_EQUAL_INT(3, arr_var->array.count);
    
    free_ast(root);
}

// Test string variable type replacement (string -> number -> string)
void test_string_variable_type_replacement(void)
{
    // Start with string
    Value *str_val = make_string_value("initial string");
    declare_var("dynamic_var", str_val);
    
    Value *retrieved = get_var("dynamic_var");
    TEST_ASSERT_EQUAL_INT(VAL_STRING, retrieved->type);
    TEST_ASSERT_EQUAL_STRING("initial string", retrieved->string);
    
    // Replace with number
    Value *num_val = make_number_value(123.45);
    set_var("dynamic_var", num_val);
    
    retrieved = get_var("dynamic_var");
    TEST_ASSERT_EQUAL_INT(VAL_NUMBER, retrieved->type);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 123.45, retrieved->number);
    
    // Replace back with string
    Value *new_str_val = make_string_value("final string");
    set_var("dynamic_var", new_str_val);
    
    retrieved = get_var("dynamic_var");
    TEST_ASSERT_EQUAL_INT(VAL_STRING, retrieved->type);
    TEST_ASSERT_EQUAL_STRING("final string", retrieved->string);
}

// Test string memory management during variable operations
void test_string_memory_management_integration(void)
{
    // Test multiple string assignments to same variable
    Value *str1 = make_string_value("first string");
    declare_var("mem_test", str1);
    
    // Reassign multiple times (should free previous strings)
    for (int i = 0; i < 5; i++) {
        char buffer[50];
        snprintf(buffer, sizeof(buffer), "string number %d", i);
        Value *str = make_string_value(buffer);
        set_var("mem_test", str);
        
        Value *retrieved = get_var("mem_test");
        TEST_ASSERT_NOT_NULL(retrieved);
        TEST_ASSERT_EQUAL_STRING(buffer, retrieved->string);
    }
    
    // Test string copying behavior
    Value *original = make_string_value("original string");
    declare_var("original_var", original);
    
    Value *retrieved_original = get_var("original_var");
    declare_var("copy_var", retrieved_original);
    
    Value *retrieved_copy = get_var("copy_var");
    TEST_ASSERT_EQUAL_STRING("original string", retrieved_copy->string);
    
    // Modify original - copy should remain unchanged
    Value *modified = make_string_value("modified string");
    set_var("original_var", modified);
    
    Value *check_original = get_var("original_var");
    Value *check_copy = get_var("copy_var");
    
    TEST_ASSERT_EQUAL_STRING("modified string", check_original->string);
    TEST_ASSERT_EQUAL_STRING("original string", check_copy->string);
}

// Test var_exists function with string variables
void test_string_var_exists(void)
{
    // Test non-existent variable
    TEST_ASSERT_FALSE(var_exists("nonexistent"));
    
    // Declare string variable
    Value *str_val = make_string_value("test");
    declare_var("exists_test", str_val);
    
    // Test existing variable
    TEST_ASSERT_TRUE(var_exists("exists_test"));
    
    // Test after scope exit
    enter_scope();
    Value *local_str = make_string_value("local");
    declare_var("local_exists", local_str);
    
    TEST_ASSERT_TRUE(var_exists("local_exists"));
    TEST_ASSERT_TRUE(var_exists("exists_test")); // Should still exist
    
    exit_scope();
    
    TEST_ASSERT_FALSE(var_exists("local_exists")); // Should be gone
    TEST_ASSERT_TRUE(var_exists("exists_test"));   // Should still exist
}

// Main test runner
int main(void)
{
    UNITY_BEGIN();
    
    RUN_TEST(test_string_declare_var_get_var);
    RUN_TEST(test_string_variable_assignment_retrieval);
    RUN_TEST(test_string_variable_reassignment);
    RUN_TEST(test_string_variable_scoping);
    RUN_TEST(test_string_let_statements_integration);
    RUN_TEST(test_string_escape_sequences_integration);
    RUN_TEST(test_mixed_variable_types_integration);
    RUN_TEST(test_string_variable_type_replacement);
    RUN_TEST(test_string_memory_management_integration);
    RUN_TEST(test_string_var_exists);
    
    return UNITY_END();
}