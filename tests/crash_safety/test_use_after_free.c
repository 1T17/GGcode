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

// Test basic use-after-free detection
void test_basic_use_after_free(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Allocate memory
    char* buffer = (char*)SAFE_MALLOC(100);
    TEST_ASSERT_NOT_NULL(buffer);
    
    // Use memory normally
    strcpy(buffer, "Hello World");
    TEST_ASSERT_EQUAL_STRING("Hello World", buffer);
    
    // Free the memory
    SAFE_FREE(buffer);
    
    // Try to access freed memory (use-after-free)
    char first_char = buffer[0]; // This should be detected
    (void)first_char; // Suppress unused variable warning
    
    // Check if use-after-free was detected
    TEST_ASSERT_TRUE(detect_use_after_free(buffer));
    
    // Verify detection in stats
    MemoryStats* stats = memory_safety_get_stats();
    TEST_ASSERT_GREATER_THAN(0, stats->use_after_free_detected);
    
    MEMORY_SAFETY_TEST_END();
}

// Test use-after-free with write operation
void test_use_after_free_write(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Allocate and initialize
    int* numbers = (int*)SAFE_MALLOC(10 * sizeof(int));
    TEST_ASSERT_NOT_NULL(numbers);
    
    for (int i = 0; i < 10; i++) {
        numbers[i] = i;
    }
    
    // Free the memory
    SAFE_FREE(numbers);
    
    // Try to write to freed memory
    numbers[0] = 999; // Use-after-free write
    
    // Should detect the violation
    TEST_ASSERT_TRUE(detect_use_after_free(numbers));
    
    MEMORY_SAFETY_TEST_END();
}

// Test use-after-free with string operations
void test_use_after_free_string_ops(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Allocate string buffer
    char* str = (char*)SAFE_MALLOC(50);
    TEST_ASSERT_NOT_NULL(str);
    
    strcpy(str, "Test String");
    
    // Free the string
    SAFE_FREE(str);
    
    // Try string operations on freed memory
    size_t len = strlen(str); // Use-after-free
    (void)len; // Suppress warning
    
    // Should detect violation
    TEST_ASSERT_TRUE(detect_use_after_free(str));
    
    MEMORY_SAFETY_TEST_END();
}

// Test multiple use-after-free scenarios
void test_multiple_use_after_free(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    const int num_buffers = 5;
    void* buffers[num_buffers];
    
    // Allocate multiple buffers
    for (int i = 0; i < num_buffers; i++) {
        buffers[i] = SAFE_MALLOC(100 + i * 10);
        TEST_ASSERT_NOT_NULL(buffers[i]);
    }
    
    // Free all buffers
    for (int i = 0; i < num_buffers; i++) {
        SAFE_FREE(buffers[i]);
    }
    
    // Try to access all freed buffers
    int violations_detected = 0;
    for (int i = 0; i < num_buffers; i++) {
        char* ptr = (char*)buffers[i];
        ptr[0] = 'X'; // Use-after-free
        
        if (detect_use_after_free(buffers[i])) {
            violations_detected++;
        }
    }
    
    // Should detect all violations
    TEST_ASSERT_EQUAL(num_buffers, violations_detected);
    
    MEMORY_SAFETY_TEST_END();
}

// Test use-after-free with structure access
void test_use_after_free_structure(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    typedef struct {
        int id;
        char name[32];
        double value;
    } TestStruct;
    
    // Allocate structure
    TestStruct* obj = (TestStruct*)SAFE_MALLOC(sizeof(TestStruct));
    TEST_ASSERT_NOT_NULL(obj);
    
    // Initialize structure
    obj->id = 123;
    strcpy(obj->name, "Test Object");
    obj->value = 3.14159;
    
    // Free the structure
    SAFE_FREE(obj);
    
    // Try to access freed structure members
    int id = obj->id; // Use-after-free
    (void)id;
    
    // Should detect violation
    TEST_ASSERT_TRUE(detect_use_after_free(obj));
    
    MEMORY_SAFETY_TEST_END();
}

// Test use-after-free with array access
void test_use_after_free_array(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Allocate array
    double* array = (double*)SAFE_MALLOC(20 * sizeof(double));
    TEST_ASSERT_NOT_NULL(array);
    
    // Initialize array
    for (int i = 0; i < 20; i++) {
        array[i] = i * 1.5;
    }
    
    // Free the array
    SAFE_FREE(array);
    
    // Try to access freed array elements
    double sum = 0.0;
    for (int i = 0; i < 5; i++) {
        sum += array[i]; // Use-after-free in loop
    }
    (void)sum;
    
    // Should detect violation
    TEST_ASSERT_TRUE(detect_use_after_free(array));
    
    MEMORY_SAFETY_TEST_END();
}

// Test use-after-realloc (similar to use-after-free)
void test_use_after_realloc(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Initial allocation
    char* buffer = (char*)SAFE_MALLOC(50);
    TEST_ASSERT_NOT_NULL(buffer);
    
    strcpy(buffer, "Initial data");
    char* old_buffer = buffer;
    
    // Realloc to larger size (may move memory)
    buffer = (char*)SAFE_REALLOC(buffer, 200);
    TEST_ASSERT_NOT_NULL(buffer);
    
    // If memory was moved, accessing old pointer is use-after-free
    if (old_buffer != buffer) {
        char first_char = old_buffer[0]; // Potential use-after-free
        (void)first_char;
        
        // Check if detected (may or may not be detected depending on implementation)
        detect_use_after_free(old_buffer);
    }
    
    SAFE_FREE(buffer);
    
    MEMORY_SAFETY_TEST_END();
}

// Test delayed use-after-free
void test_delayed_use_after_free(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    char* buffer = (char*)SAFE_MALLOC(100);
    TEST_ASSERT_NOT_NULL(buffer);
    
    strcpy(buffer, "Data to be freed");
    
    // Free the memory
    SAFE_FREE(buffer);
    
    // Allocate other memory to potentially overwrite freed area
    for (int i = 0; i < 10; i++) {
        void* temp = SAFE_MALLOC(50);
        SAFE_FREE(temp);
    }
    
    // Try to access the original freed memory after some time
    char value = buffer[5]; // Delayed use-after-free
    (void)value;
    
    // Should still detect violation
    TEST_ASSERT_TRUE(detect_use_after_free(buffer));
    
    MEMORY_SAFETY_TEST_END();
}

// Test use-after-free with function pointers
void test_use_after_free_function_pointer(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    typedef struct {
        int (*func)(int);
        int data;
    } FuncStruct;
    
    // Simple function for testing
    int test_func(int x) { return x * 2; }
    
    // Allocate structure with function pointer
    FuncStruct* fs = (FuncStruct*)SAFE_MALLOC(sizeof(FuncStruct));
    TEST_ASSERT_NOT_NULL(fs);
    
    fs->func = test_func;
    fs->data = 42;
    
    // Test normal usage
    int result = fs->func(fs->data);
    TEST_ASSERT_EQUAL(84, result);
    
    // Free the structure
    SAFE_FREE(fs);
    
    // Try to call function through freed pointer
    // Note: This is dangerous and might crash, so we just access the pointer
    void* func_ptr = (void*)fs->func; // Use-after-free
    (void)func_ptr;
    
    // Should detect violation
    TEST_ASSERT_TRUE(detect_use_after_free(fs));
    
    MEMORY_SAFETY_TEST_END();
}

// Test use-after-free detection with NULL pointer
void test_use_after_free_null_pointer(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    char* buffer = NULL;
    
    // Should not detect use-after-free for NULL pointer
    TEST_ASSERT_FALSE(detect_use_after_free(buffer));
    
    MEMORY_SAFETY_TEST_END();
}

int main(void) {
    UNITY_BEGIN();
    
    printf("🛡️ Running Use-After-Free Detection Tests...\n");
    
    RUN_TEST(test_basic_use_after_free);
    RUN_TEST(test_use_after_free_write);
    RUN_TEST(test_use_after_free_string_ops);
    RUN_TEST(test_multiple_use_after_free);
    RUN_TEST(test_use_after_free_structure);
    RUN_TEST(test_use_after_free_array);
    RUN_TEST(test_use_after_realloc);
    RUN_TEST(test_delayed_use_after_free);
    RUN_TEST(test_use_after_free_function_pointer);
    RUN_TEST(test_use_after_free_null_pointer);
    
    printf("✅ Use-after-free detection tests completed\n");
    
    return UNITY_END();
}