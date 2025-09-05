#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../Unity/src/unity.h"
#include "framework/memory_safety.h"
#include "framework/crash_detection.h"
#include "config/crash_test_config.h"

void setUp(void) {
    memory_safety_init();
    setup_crash_handlers();
}

void tearDown(void) {
    memory_safety_cleanup();
    cleanup_crash_handlers();
}

// Test basic double-free detection
void test_basic_double_free(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Allocate memory
    char* buffer = (char*)SAFE_MALLOC(100);
    TEST_ASSERT_NOT_NULL(buffer);
    
    // Use memory normally
    strcpy(buffer, "Test data");
    
    // Free the memory once (normal)
    SAFE_FREE(buffer);
    
    // Try to free again (double-free)
    SAFE_FREE(buffer);
    
    // Check if double-free was detected
    MemoryStats* stats = memory_safety_get_stats();
    TEST_ASSERT_GREATER_THAN(0, stats->double_free_detected);
    
    MEMORY_SAFETY_TEST_END();
}

// Test double-free with multiple allocations
void test_multiple_double_free(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    const int num_buffers = 5;
    void* buffers[num_buffers];
    
    // Allocate multiple buffers
    for (int i = 0; i < num_buffers; i++) {
        buffers[i] = SAFE_MALLOC(100 + i * 10);
        TEST_ASSERT_NOT_NULL(buffers[i]);
    }
    
    // Free all buffers normally
    for (int i = 0; i < num_buffers; i++) {
        SAFE_FREE(buffers[i]);
    }
    
    // Try to free some buffers again (double-free)
    for (int i = 0; i < num_buffers; i += 2) {
        SAFE_FREE(buffers[i]); // Double-free every other buffer
    }
    
    // Should detect multiple double-frees
    MemoryStats* stats = memory_safety_get_stats();
    TEST_ASSERT_GREATER_OR_EQUAL(3, stats->double_free_detected); // At least 3 double-frees
    
    MEMORY_SAFETY_TEST_END();
}

// Test double-free with NULL pointer
void test_double_free_null_pointer(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    char* buffer = NULL;
    
    // Freeing NULL should be safe (no double-free)
    SAFE_FREE(buffer);
    SAFE_FREE(buffer);
    
    // Should not detect double-free for NULL
    MemoryStats* stats = memory_safety_get_stats();
    TEST_ASSERT_EQUAL(0, stats->double_free_detected);
    
    MEMORY_SAFETY_TEST_END();
}

// Test double-free after realloc
void test_double_free_after_realloc(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Initial allocation
    char* buffer = (char*)SAFE_MALLOC(50);
    TEST_ASSERT_NOT_NULL(buffer);
    
    strcpy(buffer, "Initial data");
    char* old_buffer = buffer;
    
    // Realloc (may free old buffer internally)
    buffer = (char*)SAFE_REALLOC(buffer, 200);
    TEST_ASSERT_NOT_NULL(buffer);
    
    // Free the new buffer
    SAFE_FREE(buffer);
    
    // Try to free the old buffer (may be double-free if realloc freed it)
    if (old_buffer != buffer) {
        SAFE_FREE(old_buffer); // Potential double-free
        
        // Check if detected
        MemoryStats* stats = memory_safety_get_stats();
        // Note: This may or may not be detected depending on realloc implementation
    }
    
    MEMORY_SAFETY_TEST_END();
}

// Test triple-free (multiple double-frees)
void test_triple_free(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Allocate memory
    int* numbers = (int*)SAFE_MALLOC(10 * sizeof(int));
    TEST_ASSERT_NOT_NULL(numbers);
    
    // Initialize
    for (int i = 0; i < 10; i++) {
        numbers[i] = i;
    }
    
    // Free once (normal)
    SAFE_FREE(numbers);
    
    // Free twice (first double-free)
    SAFE_FREE(numbers);
    
    // Free thrice (second double-free)
    SAFE_FREE(numbers);
    
    // Should detect multiple double-frees
    MemoryStats* stats = memory_safety_get_stats();
    TEST_ASSERT_GREATER_OR_EQUAL(2, stats->double_free_detected);
    
    MEMORY_SAFETY_TEST_END();
}

// Test double-free with different pointer types
void test_double_free_different_types(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Test with different data types
    char* char_ptr = (char*)SAFE_MALLOC(100);
    int* int_ptr = (int*)SAFE_MALLOC(10 * sizeof(int));
    double* double_ptr = (double*)SAFE_MALLOC(5 * sizeof(double));
    
    TEST_ASSERT_NOT_NULL(char_ptr);
    TEST_ASSERT_NOT_NULL(int_ptr);
    TEST_ASSERT_NOT_NULL(double_ptr);
    
    // Use the memory
    strcpy(char_ptr, "Test");
    int_ptr[0] = 42;
    double_ptr[0] = 3.14159;
    
    // Free all once
    SAFE_FREE(char_ptr);
    SAFE_FREE(int_ptr);
    SAFE_FREE(double_ptr);
    
    // Try to free all again (double-free)
    SAFE_FREE(char_ptr);
    SAFE_FREE(int_ptr);
    SAFE_FREE(double_ptr);
    
    // Should detect 3 double-frees
    MemoryStats* stats = memory_safety_get_stats();
    TEST_ASSERT_GREATER_OR_EQUAL(3, stats->double_free_detected);
    
    MEMORY_SAFETY_TEST_END();
}

// Test double-free with structure
void test_double_free_structure(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    typedef struct {
        int id;
        char name[50];
        struct {
            double x, y, z;
        } position;
    } ComplexStruct;
    
    // Allocate structure
    ComplexStruct* obj = (ComplexStruct*)SAFE_MALLOC(sizeof(ComplexStruct));
    TEST_ASSERT_NOT_NULL(obj);
    
    // Initialize structure
    obj->id = 123;
    strcpy(obj->name, "Test Object");
    obj->position.x = 1.0;
    obj->position.y = 2.0;
    obj->position.z = 3.0;
    
    // Free once
    SAFE_FREE(obj);
    
    // Try to free again
    SAFE_FREE(obj);
    
    // Should detect double-free
    MemoryStats* stats = memory_safety_get_stats();
    TEST_ASSERT_GREATER_THAN(0, stats->double_free_detected);
    
    MEMORY_SAFETY_TEST_END();
}

// Test double-free in error handling path
void test_double_free_error_path(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    char* buffer1 = NULL;
    char* buffer2 = NULL;
    char* buffer3 = NULL;
    
    // Simulate error handling that might lead to double-free
    buffer1 = (char*)SAFE_MALLOC(100);
    if (!buffer1) goto cleanup;
    
    buffer2 = (char*)SAFE_MALLOC(200);
    if (!buffer2) goto cleanup;
    
    buffer3 = (char*)SAFE_MALLOC(300);
    if (!buffer3) goto cleanup;
    
    // Simulate some work
    strcpy(buffer1, "Buffer 1");
    strcpy(buffer2, "Buffer 2");
    strcpy(buffer3, "Buffer 3");
    
    // Normal cleanup
    SAFE_FREE(buffer1);
    SAFE_FREE(buffer2);
    SAFE_FREE(buffer3);
    
    // Simulate buggy error handling that frees again
cleanup:
    SAFE_FREE(buffer1); // Potential double-free
    SAFE_FREE(buffer2); // Potential double-free
    SAFE_FREE(buffer3); // Potential double-free
    
    // Should detect double-frees
    MemoryStats* stats = memory_safety_get_stats();
    TEST_ASSERT_GREATER_THAN(0, stats->double_free_detected);
    
    MEMORY_SAFETY_TEST_END();
}

// Test double-free with array of pointers
void test_double_free_pointer_array(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    const int array_size = 5;
    char* string_array[array_size];
    
    // Allocate strings
    for (int i = 0; i < array_size; i++) {
        string_array[i] = (char*)SAFE_MALLOC(50);
        TEST_ASSERT_NOT_NULL(string_array[i]);
        snprintf(string_array[i], 50, "String %d", i);
    }
    
    // Free all strings
    for (int i = 0; i < array_size; i++) {
        SAFE_FREE(string_array[i]);
        string_array[i] = NULL; // Good practice to avoid double-free
    }
    
    // Simulate buggy code that tries to free again
    for (int i = 0; i < array_size; i++) {
        if (string_array[i] != NULL) { // This check should prevent double-free
            SAFE_FREE(string_array[i]);
        }
    }
    
    // Should not detect double-free due to NULL check
    MemoryStats* stats = memory_safety_get_stats();
    TEST_ASSERT_EQUAL(0, stats->double_free_detected);
    
    MEMORY_SAFETY_TEST_END();
}

// Test double-free detection with invalid pointer
void test_double_free_invalid_pointer(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Create an invalid pointer (not allocated by our system)
    char stack_buffer[100];
    char* invalid_ptr = stack_buffer;
    
    // Try to free invalid pointer (should be detected as error)
    SAFE_FREE(invalid_ptr);
    
    // Try to free again (double-free of invalid pointer)
    SAFE_FREE(invalid_ptr);
    
    // Should detect the attempts to free invalid pointer
    MemoryStats* stats = memory_safety_get_stats();
    TEST_ASSERT_GREATER_THAN(0, stats->double_free_detected);
    
    MEMORY_SAFETY_TEST_END();
}

// Test double-free with memory pattern checking
void test_double_free_pattern_check(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Allocate and initialize
    char* buffer = (char*)SAFE_MALLOC(100);
    TEST_ASSERT_NOT_NULL(buffer);
    
    memset(buffer, 'A', 100);
    
    // Free once (should fill with free pattern)
    SAFE_FREE(buffer);
    
    // Try to free again
    SAFE_FREE(buffer);
    
    // Should detect double-free
    MemoryStats* stats = memory_safety_get_stats();
    TEST_ASSERT_GREATER_THAN(0, stats->double_free_detected);
    
    MEMORY_SAFETY_TEST_END();
}

int main(void) {
    UNITY_BEGIN();
    
    printf("🛡️ Running Double-Free Detection Tests...\n");
    
    RUN_TEST(test_basic_double_free);
    RUN_TEST(test_multiple_double_free);
    RUN_TEST(test_double_free_null_pointer);
    RUN_TEST(test_double_free_after_realloc);
    RUN_TEST(test_triple_free);
    RUN_TEST(test_double_free_different_types);
    RUN_TEST(test_double_free_structure);
    RUN_TEST(test_double_free_error_path);
    RUN_TEST(test_double_free_pointer_array);
    RUN_TEST(test_double_free_invalid_pointer);
    RUN_TEST(test_double_free_pattern_check);
    
    printf("✅ Double-free detection tests completed\n");
    
    return UNITY_END();
}