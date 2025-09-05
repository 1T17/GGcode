#ifndef MEMORY_SAFETY_H
#define MEMORY_SAFETY_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

// Memory allocation tracking structure
typedef struct MemoryAllocation {
    void* ptr;
    size_t size;
    const char* file;
    int line;
    const char* function;
    struct MemoryAllocation* next;
    uint32_t magic_header;
    uint32_t magic_footer;
} MemoryAllocation;

// Memory safety statistics
typedef struct {
    size_t total_allocations;
    size_t total_deallocations;
    size_t current_allocations;
    size_t peak_allocations;
    size_t total_bytes_allocated;
    size_t current_bytes_allocated;
    size_t peak_bytes_allocated;
    size_t buffer_overflows_detected;
    size_t use_after_free_detected;
    size_t double_free_detected;
    size_t memory_leaks_detected;
} MemoryStats;

// Memory corruption detection constants
#define MEMORY_MAGIC_HEADER 0xDEADBEEF
#define MEMORY_MAGIC_FOOTER 0xCAFEBABE
#define MEMORY_GUARD_SIZE 16
#define MEMORY_FILL_PATTERN 0xAA
#define MEMORY_FREE_PATTERN 0xDD

// Memory safety test framework functions
void memory_safety_init(void);
void memory_safety_cleanup(void);
void memory_safety_reset_stats(void);
MemoryStats* memory_safety_get_stats(void);

// Memory allocation wrappers with tracking
void* safe_malloc(size_t size, const char* file, int line, const char* function);
void* safe_calloc(size_t count, size_t size, const char* file, int line, const char* function);
void* safe_realloc(void* ptr, size_t size, const char* file, int line, const char* function);
void safe_free(void* ptr, const char* file, int line, const char* function);
char* safe_strdup(const char* str, const char* file, int line, const char* function);

// Memory corruption detection
bool check_memory_integrity(void);
bool check_allocation_integrity(void* ptr);
bool detect_buffer_overflow(void* ptr);
bool detect_use_after_free(void* ptr);
void mark_memory_as_freed(void* ptr);

// Memory leak detection
size_t detect_memory_leaks(void);
void print_memory_leaks(void);
void print_memory_stats(void);

// Buffer overflow test utilities
void* create_buffer_with_guards(size_t size);
bool check_buffer_guards(void* buffer);
void destroy_guarded_buffer(void* buffer);

// Use-after-free test utilities
void* allocate_tracked_memory(size_t size);
void free_tracked_memory(void* ptr);
bool is_memory_freed(void* ptr);

// Double-free test utilities
bool is_double_free_attempt(void* ptr);
void record_free_attempt(void* ptr);

// Stack overflow detection
void setup_stack_canary(void);
bool check_stack_canary(void);
size_t get_stack_depth(void);
void push_stack_frame(const char* function_name);
void pop_stack_frame(void);

// Memory allocation failure simulation
void enable_allocation_failure_simulation(void);
void disable_allocation_failure_simulation(void);
void set_allocation_failure_rate(double failure_rate);
void set_allocation_failure_after_count(size_t count);

// Macros for convenient memory tracking
#define SAFE_MALLOC(size) safe_malloc(size, __FILE__, __LINE__, __func__)
#define SAFE_CALLOC(count, size) safe_calloc(count, size, __FILE__, __LINE__, __func__)
#define SAFE_REALLOC(ptr, size) safe_realloc(ptr, size, __FILE__, __LINE__, __func__)
#define SAFE_FREE(ptr) safe_free(ptr, __FILE__, __LINE__, __func__)
#define SAFE_STRDUP(str) safe_strdup(str, __FILE__, __LINE__, __func__)

// Memory safety test macros
#define MEMORY_SAFETY_TEST_BEGIN() \
    do { \
        memory_safety_init(); \
        memory_safety_reset_stats(); \
    } while(0)

#define MEMORY_SAFETY_TEST_END() \
    do { \
        size_t leaks = detect_memory_leaks(); \
        if (leaks > 0) { \
            print_memory_leaks(); \
        } \
        memory_safety_cleanup(); \
    } while(0)

#define ASSERT_NO_MEMORY_LEAKS() \
    do { \
        size_t leaks = detect_memory_leaks(); \
        if (leaks > 0) { \
            print_memory_leaks(); \
            TEST_FAIL_MESSAGE("Memory leaks detected"); \
        } \
    } while(0)

#define ASSERT_NO_BUFFER_OVERFLOWS() \
    do { \
        MemoryStats* stats = memory_safety_get_stats(); \
        if (stats->buffer_overflows_detected > 0) { \
            TEST_FAIL_MESSAGE("Buffer overflows detected"); \
        } \
    } while(0)

#define ASSERT_NO_USE_AFTER_FREE() \
    do { \
        MemoryStats* stats = memory_safety_get_stats(); \
        if (stats->use_after_free_detected > 0) { \
            TEST_FAIL_MESSAGE("Use-after-free detected"); \
        } \
    } while(0)

#define ASSERT_NO_DOUBLE_FREE() \
    do { \
        MemoryStats* stats = memory_safety_get_stats(); \
        if (stats->double_free_detected > 0) { \
            TEST_FAIL_MESSAGE("Double-free detected"); \
        } \
    } while(0)

// Memory corruption test scenarios
typedef enum {
    CORRUPTION_BUFFER_OVERFLOW,
    CORRUPTION_BUFFER_UNDERFLOW,
    CORRUPTION_USE_AFTER_FREE,
    CORRUPTION_DOUBLE_FREE,
    CORRUPTION_UNINITIALIZED_READ,
    CORRUPTION_STACK_OVERFLOW,
    CORRUPTION_HEAP_CORRUPTION
} CorruptionType;

// Test scenario execution
bool execute_corruption_test(CorruptionType type, void* test_data);
void simulate_buffer_overflow(void* buffer, size_t buffer_size, size_t overflow_size);
void simulate_use_after_free(void* ptr);
void simulate_double_free(void* ptr);
void simulate_stack_overflow(int depth);

#endif // MEMORY_SAFETY_H