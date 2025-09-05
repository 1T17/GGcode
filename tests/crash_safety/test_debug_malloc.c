#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple test without any framework
int main(void) {
    printf("Testing basic malloc...\n");
    
    char* buffer = malloc(100);
    if (!buffer) {
        printf("Malloc failed\n");
        return 1;
    }
    
    printf("Malloc succeeded, buffer at %p\n", (void*)buffer);
    
    strcpy(buffer, "Hello");
    printf("String copy succeeded: %s\n", buffer);
    
    free(buffer);
    printf("Free succeeded\n");
    
    printf("All basic operations completed successfully\n");
    return 0;
}