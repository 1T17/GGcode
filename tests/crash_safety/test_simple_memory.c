#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Unity/src/unity.h"

void setUp(void) {
    // Simple setup
}

void tearDown(void) {
    // Simple teardown
}

// Test basic memory allocation without our framework
void test_basic_malloc_free(void) {
    printf("Testing basic malloc/free...\n");
    
    char* buffer = malloc(100);
    TEST_ASSERT_NOT_NULL(buffer);
    
    strcpy(buffer, "Hello World");
    TEST_ASSERT_EQUAL_STRING("Hello World", buffer);
    
    free(buffer);
    
    printf("Basic malloc/free test passed\n");
}

// Test Unity framework itself
void test_unity_framework(void) {
    printf("Testing Unity framework...\n");
    
    TEST_ASSERT_EQUAL(42, 42);
    TEST_ASSERT_TRUE(1 == 1);
    TEST_ASSERT_FALSE(1 == 0);
    
    printf("Unity framework test passed\n");
}

int main(void) {
    UNITY_BEGIN();
    
    printf("🧪 Running Simple Memory Tests...\n");
    
    RUN_TEST(test_basic_malloc_free);
    RUN_TEST(test_unity_framework);
    
    printf("✅ Simple memory tests completed\n");
    
    return UNITY_END();
}