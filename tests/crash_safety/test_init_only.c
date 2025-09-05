#include <stdio.h>
#include <stdlib.h>
#include "framework/memory_safety.h"

int main(void) {
    printf("Testing memory_safety_init only...\n");
    
    printf("About to call memory_safety_init...\n");
    memory_safety_init();
    printf("memory_safety_init completed\n");
    
    printf("About to call memory_safety_cleanup...\n");
    memory_safety_cleanup();
    printf("memory_safety_cleanup completed\n");
    
    printf("All operations completed successfully\n");
    return 0;
}