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

// Test basic buffer overflow detection
void test_basic_buffer_overflow_detection(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Allocate a small buffer
    char* buffer = (char*)SAFE_MALLOC(100);
    TEST_ASSERT_NOT_NULL(buffer);
    
    // Write within bounds (should be safe)
    for (int i = 0; i < 100; i++) {
        buffer[i] = 'A';
    }
    
    // Check integrity (should pass)
    TEST_ASSERT_TRUE(check_allocation_integrity(buffer));
    
    // Simulate buffer overflow by writing past the end
    buffer[100] = 'X'; // This should corrupt the guard area
    buffer[101] = 'Y';
    
    // Check for buffer overflow detection
    TEST_ASSERT_TRUE(detect_buffer_overflow(buffer));
    
    SAFE_FREE(buffer);
    
    // Verify buffer overflow was detected in stats
    MemoryStats* stats = memory_safety_get_stats();
    TEST_ASSERT_GREATER_THAN(0, stats->buffer_overflows_detected);
    
    MEMORY_SAFETY_TEST_END();
}

// Test buffer underflow detection
void test_buffer_underflow_detection(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Allocate buffer
    char* buffer = (char*)SAFE_MALLOC(100);
    TEST_ASSERT_NOT_NULL(buffer);
    
    // Write before the buffer start (underflow)
    buffer[-1] = 'X'; // This should corrupt the header guard
    buffer[-2] = 'Y';
    
    // Check for buffer corruption
    TEST_ASSERT_TRUE(detect_buffer_overflow(buffer));
    
    SAFE_FREE(buffer);
    
    // Verify detection in stats
    MemoryStats* stats = memory_safety_get_stats();
    TEST_ASSERT_GREATER_THAN(0, stats->buffer_overflows_detected);
    
    MEMORY_SAFETY_TEST_END();
}

// Test large buffer overflow
void test_large_buffer_overflow(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Allocate a larger buffer
    char* buffer = (char*)SAFE_MALLOC(1000);
    TEST_ASSERT_NOT_NULL(buffer);
    
    // Fill buffer normally
    memset(buffer, 'A', 1000);
    
    // Check integrity (should be good)
    TEST_ASSERT_TRUE(check_allocation_integrity(buffer));
    
    // Cause large overflow
    memset(buffer + 1000, 'X', 50); // Write 50 bytes past end
    
    // Should detect overflow
    TEST_ASSERT_TRUE(detect_buffer_overflow(buffer));
    
    SAFE_FREE(buffer);
    
    MEMORY_SAFETY_TEST_END();
}

// Test multiple buffer overflows
void test_multiple_buffer_overflows(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    const int num_buffers = 10;
    char* buffers[num_buffers];
    
    // Allocate multiple buffers
    for (int i = 0; i < num_buffers; i++) {
        buffers[i] = (char*)SAFE_MALLOC(100 + i * 10);
        TEST_ASSERT_NOT_NULL(buffers[i]);
    }
    
    // Corrupt some buffers
    for (int i = 0; i < num_buffers; i += 2) {
        buffers[i][100 + i * 10] = 'X'; // Overflow
    }
    
    // Check integrity of all buffers
    int overflows_detected = 0;
    for (int i = 0; i < num_buffers; i++) {
        if (detect_buffer_overflow(buffers[i])) {
            overflows_detected++;
        }
    }
    
    // Should detect overflows in every other buffer
    TEST_ASSERT_EQUAL(5, overflows_detected);
    
    // Free all buffers
    for (int i = 0; i < num_buffers; i++) {
        SAFE_FREE(buffers[i]);
    }
    
    MEMORY_SAFETY_TEST_END();
}

// Test string buffer overflow
void test_string_buffer_overflow(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Allocate buffer for string
    char* str_buffer = (char*)SAFE_MALLOC(50);
    TEST_ASSERT_NOT_NULL(str_buffer);
    
    // Safe string operations
    strcpy(str_buffer, "Hello");
    strcat(str_buffer, " World");
    
    // Check integrity (should be fine)
    TEST_ASSERT_TRUE(check_allocation_integrity(str_buffer));
    
    // Unsafe string operation that causes overflow
    char long_string[100];
    memset(long_string, 'A', 99);
    long_string[99] = '\0';
    
    strcpy(str_buffer, long_string); // This should overflow
    
    // Should detect the overflow
    TEST_ASSERT_TRUE(detect_buffer_overflow(str_buffer));
    
    SAFE_FREE(str_buffer);
    
    MEMORY_SAFETY_TEST_END();
}

// Test array bounds checking
void test_array_bounds_overflow(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Allocate array
    int* array = (int*)SAFE_MALLOC(10 * sizeof(int));
    TEST_ASSERT_NOT_NULL(array);
    
    // Fill array normally
    for (int i = 0; i < 10; i++) {
        array[i] = i;
    }
    
    // Check integrity
    TEST_ASSERT_TRUE(check_allocation_integrity(array));
    
    // Write past array bounds
    array[10] = 999; // Index 10 is out of bounds for 10-element array
    array[11] = 888;
    
    // Should detect overflow
    TEST_ASSERT_TRUE(detect_buffer_overflow(array));
    
    SAFE_FREE(array);
    
    MEMORY_SAFETY_TEST_END();
}

// Test structure buffer overflow
void test_structure_buffer_overflow(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    typedef struct {
        char name[32];
        int age;
        double salary;
    } Person;
    
    // Allocate structure
    Person* person = (Person*)SAFE_MALLOC(sizeof(Person));
    TEST_ASSERT_NOT_NULL(person);
    
    // Normal usage
    strcpy(person->name, "John Doe");
    person->age = 30;
    person->salary = 50000.0;
    
    // Check integrity
    TEST_ASSERT_TRUE(check_allocation_integrity(person));
    
    // Overflow the name field
    char long_name[100];
    memset(long_name, 'X', 99);
    long_name[99] = '\0';
    strcpy(person->name, long_name); // This overflows the name field
    
    // Should detect overflow
    TEST_ASSERT_TRUE(detect_buffer_overflow(person));
    
    SAFE_FREE(person);
    
    MEMORY_SAFETY_TEST_END();
}

// Test off-by-one buffer overflow
void test_off_by_one_overflow(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Allocate exact size buffer
    char* buffer = (char*)SAFE_MALLOC(10);
    TEST_ASSERT_NOT_NULL(buffer);
    
    // Fill exactly to capacity
    for (int i = 0; i < 10; i++) {
        buffer[i] = 'A' + i;
    }
    
    // Check integrity (should be fine)
    TEST_ASSERT_TRUE(check_allocation_integrity(buffer));
    
    // Off-by-one error
    buffer[10] = '\0'; // Writing null terminator past end
    
    // Should detect the overflow
    TEST_ASSERT_TRUE(detect_buffer_overflow(buffer));
    
    SAFE_FREE(buffer);
    
    MEMORY_SAFETY_TEST_END();
}

// Test buffer overflow with realloc
void test_realloc_buffer_overflow(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    // Initial allocation
    char* buffer = (char*)SAFE_MALLOC(50);
    TEST_ASSERT_NOT_NULL(buffer);
    
    // Fill initial buffer
    memset(buffer, 'A', 50);
    
    // Reallocate to larger size
    buffer = (char*)SAFE_REALLOC(buffer, 100);
    TEST_ASSERT_NOT_NULL(buffer);
    
    // Fill new area
    memset(buffer + 50, 'B', 50);
    
    // Check integrity (should be fine)
    TEST_ASSERT_TRUE(check_allocation_integrity(buffer));
    
    // Overflow the reallocated buffer
    buffer[100] = 'X';
    
    // Should detect overflow
    TEST_ASSERT_TRUE(detect_buffer_overflow(buffer));
    
    SAFE_FREE(buffer);
    
    MEMORY_SAFETY_TEST_END();
}

// Test buffer overflow in loop
void test_loop_buffer_overflow(void) {
    MEMORY_SAFETY_TEST_BEGIN();
    
    char* buffer = (char*)SAFE_MALLOC(100);
    TEST_ASSERT_NOT_NULL(buffer);
    
    // Intentional loop that goes past buffer end
    for (int i = 0; i <= 100; i++) { // Note: <= instead of <
        buffer[i] = 'A';
    }
    
    // Should detect overflow
    TEST_ASSERT_TRUE(detect_buffer_overflow(buffer));
    
    SAFE_FREE(buffer);
    
    MEMORY_SAFETY_TEST_END();
}

int main(void) {
    UNITY_BEGIN();
    
    printf("🛡️ Running Buffer Overflow Detection Tests...\n");
    
    RUN_TEST(test_basic_buffer_overflow_detection);
    RUN_TEST(test_buffer_underflow_detection);
    RUN_TEST(test_large_buffer_overflow);
    RUN_TEST(test_multiple_buffer_overflows);
    RUN_TEST(test_string_buffer_overflow);
    RUN_TEST(test_array_bounds_overflow);
    RUN_TEST(test_structure_buffer_overflow);
    RUN_TEST(test_off_by_one_overflow);
    RUN_TEST(test_realloc_buffer_overflow);
    RUN_TEST(test_loop_buffer_overflow);
    
    printf("✅ Buffer overflow detection tests completed\n");
    
    return UNITY_END();
}