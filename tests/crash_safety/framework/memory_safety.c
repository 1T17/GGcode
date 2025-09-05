#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include "memory_safety.h"
#include "crash_detection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>

// Global memory tracking state
static MemoryAllocation* g_allocations = NULL;
static MemoryStats g_memory_stats = {0};
static pthread_mutex_t g_memory_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool g_memory_tracking_enabled = false;
static bool g_allocation_failure_simulation = false;
static double g_allocation_failure_rate = 0.0;
static size_t g_allocation_failure_after_count = 0;
static size_t g_allocation_count = 0;

// Stack canary for stack overflow detection
static uint32_t g_stack_canary = 0x12345678;
static size_t g_stack_depth = 0;
static char g_stack_frames[1000][64];

// Forward declaration
static bool check_allocation_integrity_internal(void* ptr, MemoryAllocation* allocations);

// Initialize memory safety framework
void memory_safety_init(void) {
    pthread_mutex_lock(&g_memory_mutex);
    
    g_allocations = NULL;
    memset(&g_memory_stats, 0, sizeof(g_memory_stats));
    g_memory_tracking_enabled = true;
    g_allocation_failure_simulation = false;
    g_allocation_failure_rate = 0.0;
    g_allocation_failure_after_count = 0;
    g_allocation_count = 0;
    g_stack_depth = 0;
    
    // Setup stack canary
    setup_stack_canary();
    
    pthread_mutex_unlock(&g_memory_mutex);
}

// Cleanup memory safety framework
void memory_safety_cleanup(void) {
    pthread_mutex_lock(&g_memory_mutex);
    
    // Free all tracked allocations
    MemoryAllocation* current = g_allocations;
    while (current) {
        MemoryAllocation* next = current->next;
        
        // Mark as leaked
        g_memory_stats.memory_leaks_detected++;
        
        // Free the actual memory
        free(current->ptr);
        free(current);
        
        current = next;
    }
    
    g_allocations = NULL;
    g_memory_tracking_enabled = false;
    
    pthread_mutex_unlock(&g_memory_mutex);
}

// Reset memory statistics
void memory_safety_reset_stats(void) {
    pthread_mutex_lock(&g_memory_mutex);
    memset(&g_memory_stats, 0, sizeof(g_memory_stats));
    g_allocation_count = 0;
    pthread_mutex_unlock(&g_memory_mutex);
}

// Get memory statistics
MemoryStats* memory_safety_get_stats(void) {
    return &g_memory_stats;
}

// Safe malloc with tracking
void* safe_malloc(size_t size, const char* file, int line, const char* function) {
    if (!g_memory_tracking_enabled) {
        return malloc(size);
    }
    
    pthread_mutex_lock(&g_memory_mutex);
    
    // Simulate allocation failure if enabled
    if (g_allocation_failure_simulation) {
        g_allocation_count++;
        
        bool should_fail = false;
        if (g_allocation_failure_after_count > 0 && 
            g_allocation_count >= g_allocation_failure_after_count) {
            should_fail = true;
        } else if (g_allocation_failure_rate > 0.0) {
            double random_val = (double)rand() / RAND_MAX;
            should_fail = (random_val < g_allocation_failure_rate);
        }
        
        if (should_fail) {
            pthread_mutex_unlock(&g_memory_mutex);
            return NULL;
        }
    }
    
    // Allocate memory with guard bytes
    size_t total_size = size + 2 * MEMORY_GUARD_SIZE + sizeof(uint32_t) * 2;
    void* raw_ptr = malloc(total_size);
    
    if (!raw_ptr) {
        pthread_mutex_unlock(&g_memory_mutex);
        return NULL;
    }
    
    // Setup guard bytes and magic numbers
    uint32_t* header_magic = (uint32_t*)raw_ptr;
    *header_magic = MEMORY_MAGIC_HEADER;
    
    void* user_ptr = (char*)raw_ptr + sizeof(uint32_t) + MEMORY_GUARD_SIZE;
    
    uint32_t* footer_magic = (uint32_t*)((char*)user_ptr + size + MEMORY_GUARD_SIZE);
    *footer_magic = MEMORY_MAGIC_FOOTER;
    
    // Fill guard areas with pattern
    memset((char*)raw_ptr + sizeof(uint32_t), MEMORY_FILL_PATTERN, MEMORY_GUARD_SIZE);
    memset((char*)user_ptr + size, MEMORY_FILL_PATTERN, MEMORY_GUARD_SIZE);
    
    // Create allocation record
    MemoryAllocation* allocation = malloc(sizeof(MemoryAllocation));
    if (allocation) {
        allocation->ptr = user_ptr;
        allocation->size = size;
        allocation->file = file;
        allocation->line = line;
        allocation->function = function;
        allocation->magic_header = MEMORY_MAGIC_HEADER;
        allocation->magic_footer = MEMORY_MAGIC_FOOTER;
        allocation->next = g_allocations;
        g_allocations = allocation;
        
        // Update statistics
        g_memory_stats.total_allocations++;
        g_memory_stats.current_allocations++;
        g_memory_stats.total_bytes_allocated += size;
        g_memory_stats.current_bytes_allocated += size;
        
        if (g_memory_stats.current_allocations > g_memory_stats.peak_allocations) {
            g_memory_stats.peak_allocations = g_memory_stats.current_allocations;
        }
        
        if (g_memory_stats.current_bytes_allocated > g_memory_stats.peak_bytes_allocated) {
            g_memory_stats.peak_bytes_allocated = g_memory_stats.current_bytes_allocated;
        }
    }
    
    pthread_mutex_unlock(&g_memory_mutex);
    return user_ptr;
}

// Safe calloc with tracking
void* safe_calloc(size_t count, size_t size, const char* file, int line, const char* function) {
    size_t total_size = count * size;
    void* ptr = safe_malloc(total_size, file, line, function);
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

// Safe realloc with tracking
void* safe_realloc(void* ptr, size_t size, const char* file, int line, const char* function) {
    if (!ptr) {
        return safe_malloc(size, file, line, function);
    }
    
    if (size == 0) {
        safe_free(ptr, file, line, function);
        return NULL;
    }
    
    // Allocate new memory
    void* new_ptr = safe_malloc(size, file, line, function);
    if (!new_ptr) {
        return NULL;
    }
    
    // Find old allocation to get size
    pthread_mutex_lock(&g_memory_mutex);
    MemoryAllocation* allocation = g_allocations;
    size_t old_size = 0;
    
    while (allocation) {
        if (allocation->ptr == ptr) {
            old_size = allocation->size;
            break;
        }
        allocation = allocation->next;
    }
    pthread_mutex_unlock(&g_memory_mutex);
    
    // Copy data
    if (old_size > 0) {
        size_t copy_size = (old_size < size) ? old_size : size;
        memcpy(new_ptr, ptr, copy_size);
    }
    
    // Free old memory
    safe_free(ptr, file, line, function);
    
    return new_ptr;
}

// Safe free with tracking
void safe_free(void* ptr, const char* file, int line, const char* function) {
    if (!ptr) {
        return;
    }
    
    if (!g_memory_tracking_enabled) {
        free(ptr);
        return;
    }
    
    pthread_mutex_lock(&g_memory_mutex);
    
    // Find allocation record
    MemoryAllocation* prev = NULL;
    MemoryAllocation* current = g_allocations;
    
    while (current) {
        if (current->ptr == ptr) {
            break;
        }
        prev = current;
        current = current->next;
    }
    
    if (!current) {
        // Double-free or invalid pointer
        g_memory_stats.double_free_detected++;
        pthread_mutex_unlock(&g_memory_mutex);
        return;
    }
    
    // Check for buffer overflows (use internal version since we already hold the mutex)
    if (!check_allocation_integrity_internal(ptr, g_allocations)) {
        g_memory_stats.buffer_overflows_detected++;
    }
    
    // Remove from allocation list
    if (prev) {
        prev->next = current->next;
    } else {
        g_allocations = current->next;
    }
    
    // Update statistics
    g_memory_stats.total_deallocations++;
    g_memory_stats.current_allocations--;
    g_memory_stats.current_bytes_allocated -= current->size;
    
    // Mark memory as freed (fill with pattern)
    memset(ptr, MEMORY_FREE_PATTERN, current->size);
    
    // Free the actual memory (including guards)
    void* raw_ptr = (char*)ptr - sizeof(uint32_t) - MEMORY_GUARD_SIZE;
    free(raw_ptr);
    
    // Free allocation record
    free(current);
    
    pthread_mutex_unlock(&g_memory_mutex);
}

// Safe strdup with tracking
char* safe_strdup(const char* str, const char* file, int line, const char* function) {
    if (!str) {
        return NULL;
    }
    
    size_t len = strlen(str) + 1;
    char* copy = safe_malloc(len, file, line, function);
    if (copy) {
        strcpy(copy, str);
    }
    return copy;
}

// Check memory integrity
bool check_memory_integrity(void) {
    if (!g_memory_tracking_enabled) {
        return true;
    }
    
    pthread_mutex_lock(&g_memory_mutex);
    
    bool integrity_ok = true;
    MemoryAllocation* current = g_allocations;
    
    while (current) {
        if (!check_allocation_integrity(current->ptr)) {
            integrity_ok = false;
            g_memory_stats.buffer_overflows_detected++;
        }
        current = current->next;
    }
    
    pthread_mutex_unlock(&g_memory_mutex);
    return integrity_ok;
}

// Internal function to check allocation integrity without mutex (assumes mutex is already held)
static bool check_allocation_integrity_internal(void* ptr, MemoryAllocation* allocations) {
    if (!ptr) {
        return false;
    }
    
    // Get raw pointer
    void* raw_ptr = (char*)ptr - sizeof(uint32_t) - MEMORY_GUARD_SIZE;
    
    // Check header magic
    uint32_t* header_magic = (uint32_t*)raw_ptr;
    if (*header_magic != MEMORY_MAGIC_HEADER) {
        return false;
    }
    
    // Find allocation record to get size
    MemoryAllocation* allocation = allocations;
    size_t size = 0;
    
    while (allocation) {
        if (allocation->ptr == ptr) {
            size = allocation->size;
            break;
        }
        allocation = allocation->next;
    }
    
    if (size == 0) {
        return false;
    }
    
    // Check footer magic
    uint32_t* footer_magic = (uint32_t*)((char*)ptr + size + MEMORY_GUARD_SIZE);
    if (*footer_magic != MEMORY_MAGIC_FOOTER) {
        return false;
    }
    
    // Check guard areas
    char* header_guard = (char*)raw_ptr + sizeof(uint32_t);
    char* footer_guard = (char*)ptr + size;
    
    for (size_t i = 0; i < MEMORY_GUARD_SIZE; i++) {
        if (header_guard[i] != (char)MEMORY_FILL_PATTERN ||
            footer_guard[i] != (char)MEMORY_FILL_PATTERN) {
            return false;
        }
    }
    
    return true;
}

// Check specific allocation integrity
bool check_allocation_integrity(void* ptr) {
    if (!ptr) {
        return false;
    }
    
    pthread_mutex_lock(&g_memory_mutex);
    bool result = check_allocation_integrity_internal(ptr, g_allocations);
    pthread_mutex_unlock(&g_memory_mutex);
    
    return result;
}

// Detect buffer overflow
bool detect_buffer_overflow(void* ptr) {
    return !check_allocation_integrity(ptr);
}

// Detect use after free
bool detect_use_after_free(void* ptr) {
    if (!ptr) {
        return false;
    }
    
    pthread_mutex_lock(&g_memory_mutex);
    
    // Check if pointer is in active allocations
    MemoryAllocation* current = g_allocations;
    while (current) {
        if (current->ptr == ptr) {
            pthread_mutex_unlock(&g_memory_mutex);
            return false; // Not freed
        }
        current = current->next;
    }
    
    pthread_mutex_unlock(&g_memory_mutex);
    
    // Check if memory contains free pattern
    char* mem = (char*)ptr;
    for (size_t i = 0; i < 16; i++) { // Check first 16 bytes
        if (mem[i] == (char)MEMORY_FREE_PATTERN) {
            g_memory_stats.use_after_free_detected++;
            return true;
        }
    }
    
    return false;
}

// Detect memory leaks
size_t detect_memory_leaks(void) {
    if (!g_memory_tracking_enabled) {
        return 0;
    }
    
    pthread_mutex_lock(&g_memory_mutex);
    
    size_t leak_count = 0;
    MemoryAllocation* current = g_allocations;
    
    while (current) {
        leak_count++;
        current = current->next;
    }
    
    g_memory_stats.memory_leaks_detected = leak_count;
    
    pthread_mutex_unlock(&g_memory_mutex);
    return leak_count;
}

// Print memory leaks
void print_memory_leaks(void) {
    if (!g_memory_tracking_enabled) {
        return;
    }
    
    pthread_mutex_lock(&g_memory_mutex);
    
    printf("\n=== MEMORY LEAKS DETECTED ===\n");
    
    MemoryAllocation* current = g_allocations;
    size_t leak_count = 0;
    size_t total_leaked_bytes = 0;
    
    while (current) {
        printf("Leak #%zu: %zu bytes allocated at %s:%d in %s()\n",
               ++leak_count, current->size, current->file, current->line, current->function);
        total_leaked_bytes += current->size;
        current = current->next;
    }
    
    printf("Total: %zu leaks, %zu bytes\n", leak_count, total_leaked_bytes);
    printf("=============================\n\n");
    
    pthread_mutex_unlock(&g_memory_mutex);
}

// Print memory statistics
void print_memory_stats(void) {
    pthread_mutex_lock(&g_memory_mutex);
    
    printf("\n=== MEMORY STATISTICS ===\n");
    printf("Total allocations: %zu\n", g_memory_stats.total_allocations);
    printf("Total deallocations: %zu\n", g_memory_stats.total_deallocations);
    printf("Current allocations: %zu\n", g_memory_stats.current_allocations);
    printf("Peak allocations: %zu\n", g_memory_stats.peak_allocations);
    printf("Total bytes allocated: %zu\n", g_memory_stats.total_bytes_allocated);
    printf("Current bytes allocated: %zu\n", g_memory_stats.current_bytes_allocated);
    printf("Peak bytes allocated: %zu\n", g_memory_stats.peak_bytes_allocated);
    printf("Buffer overflows detected: %zu\n", g_memory_stats.buffer_overflows_detected);
    printf("Use-after-free detected: %zu\n", g_memory_stats.use_after_free_detected);
    printf("Double-free detected: %zu\n", g_memory_stats.double_free_detected);
    printf("Memory leaks detected: %zu\n", g_memory_stats.memory_leaks_detected);
    printf("=========================\n\n");
    
    pthread_mutex_unlock(&g_memory_mutex);
}

// Stack overflow detection
void setup_stack_canary(void) {
    g_stack_canary = 0x12345678;
    g_stack_depth = 0;
}

bool check_stack_canary(void) {
    return g_stack_canary == 0x12345678;
}

size_t get_stack_depth(void) {
    return g_stack_depth;
}

void push_stack_frame(const char* function_name) {
    if (g_stack_depth < 1000) {
        strncpy(g_stack_frames[g_stack_depth], function_name, 63);
        g_stack_frames[g_stack_depth][63] = '\0';
        g_stack_depth++;
    }
}

void pop_stack_frame(void) {
    if (g_stack_depth > 0) {
        g_stack_depth--;
    }
}

// Allocation failure simulation
void enable_allocation_failure_simulation(void) {
    g_allocation_failure_simulation = true;
}

void disable_allocation_failure_simulation(void) {
    g_allocation_failure_simulation = false;
}

void set_allocation_failure_rate(double failure_rate) {
    g_allocation_failure_rate = failure_rate;
}

void set_allocation_failure_after_count(size_t count) {
    g_allocation_failure_after_count = count;
}

// Test scenario execution
bool execute_corruption_test(CorruptionType type, void* test_data) {
    switch (type) {
        case CORRUPTION_BUFFER_OVERFLOW: {
            char* buffer = (char*)test_data;
            // Intentionally write past buffer end
            buffer[1000] = 'X'; // This should be caught
            return detect_buffer_overflow(buffer);
        }
        
        case CORRUPTION_USE_AFTER_FREE: {
            void* ptr = test_data;
            safe_free(ptr, __FILE__, __LINE__, __func__);
            // Try to access freed memory
            char* freed_mem = (char*)ptr;
            char value = freed_mem[0]; // This should be detected
            (void)value; // Suppress unused variable warning
            return detect_use_after_free(ptr);
        }
        
        case CORRUPTION_DOUBLE_FREE: {
            void* ptr = test_data;
            safe_free(ptr, __FILE__, __LINE__, __func__);
            safe_free(ptr, __FILE__, __LINE__, __func__); // Double free
            return g_memory_stats.double_free_detected > 0;
        }
        
        default:
            return false;
    }
}

// Simulate buffer overflow
void simulate_buffer_overflow(void* buffer, size_t buffer_size, size_t overflow_size) {
    if (!buffer || overflow_size == 0) {
        return;
    }
    
    char* char_buffer = (char*)buffer;
    // Write past the end of the buffer
    for (size_t i = 0; i < overflow_size; i++) {
        char_buffer[buffer_size + i] = 'X';
    }
}

// Simulate use after free
void simulate_use_after_free(void* ptr) {
    if (!ptr) {
        return;
    }
    
    // Free the memory
    safe_free(ptr, __FILE__, __LINE__, __func__);
    
    // Try to access freed memory
    char* freed_mem = (char*)ptr;
    freed_mem[0] = 'X'; // This should trigger use-after-free detection
}

// Simulate double free
void simulate_double_free(void* ptr) {
    if (!ptr) {
        return;
    }
    
    // Free the memory twice
    safe_free(ptr, __FILE__, __LINE__, __func__);
    safe_free(ptr, __FILE__, __LINE__, __func__);
}

// Simulate stack overflow
void simulate_stack_overflow(int depth) {
    if (depth <= 0) {
        return;
    }
    
    // Create a large local array to consume stack space
    char large_array[10000];
    memset(large_array, 'A', sizeof(large_array));
    
    push_stack_frame(__func__);
    
    // Recursive call
    simulate_stack_overflow(depth - 1);
    
    pop_stack_frame();
}