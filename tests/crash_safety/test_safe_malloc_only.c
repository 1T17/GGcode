#include <stdio.h>
#include <stdlib.h>
#include "framework/memory_safety.h"

int main(void) {
    printf("Testing SAFE_MALLOC only...\n");
    
    printf("Initializing memory safety...\n");
    memory_safety_init();
    printf("Memory safety initialized\n");
    
    printf("About to call SAFE_MALLOC...\n");
    void* ptr = SAFE_MALLOC(100);
    printf("SAFE_MALLOC returned: %p\n", ptr);
    
    if (ptr) {
        printf("About to call SAFE_FREE...\n");
        SAFE_FREE(ptr);
        printf("SAFE_FREE completed\n");
    }
    
    printf("Cleaning up...\n");
    memory_safety_cleanup();
    printf("All operations completed successfully\n");
    return 0;
}