#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Unity/src/unity.h"
#include "framework/memory_safety.h"

void setUp(void) {
    printf("Setting up memory safety...\n");
    memory_safety_init();
    printf("Memory safety initialized\n");
}

void tearDown(void) {
    printf("Cleaning up memory safety...\n");
    memory_safety_cleanup();
    printf("Memory safety cleaned up\n");
}

// Test basic memory safety framework
void test_memory_safety_init(void) {
    printf("Testing memory safety initialization...\n");
    
    MemoryStats* stats = memory_safety_get_stats();
    TEST_ASSERT_NOT_NULL(stats);
    
    printf("Memory safety init test passed\n");
}

// Test safe allocation
void test_safe_allocation(void) {
    printf("Testing safe allocation...\n");
    
    char* buffer = (char*)SAFE_MALLOC(100);
    TEST_ASSERT_NOT_NULL(buffer);
    
    strcpy(buffer, "Test");
    TEST_ASSERT_EQUAL_STRING("Test", buffer);
    
    SAFE_FREE(buffer);
    
    printf("Safe allocation test passed\n");
}

int main(void) {
    UNITY_BEGIN();
    
    printf("🧪 Running Memory Framework Tests...\n");
    
    RUN_TEST(test_memory_safety_init);
    RUN_TEST(test_safe_allocation);
    
    printf("✅ Memory framework tests completed\n");
    
    return UNITY_END();
}