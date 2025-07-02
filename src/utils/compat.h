#ifndef GGCODE_COMPAT_H
#define GGCODE_COMPAT_H

#if defined(_WIN32) || defined(__MINGW32__)
#include <string.h>
#include <stdlib.h>

// Provide fallback for strndup if not available
static char* strndup(const char* s, size_t n) {
    size_t len = strnlen(s, n);
    char* result = (char*)malloc(len + 1);
    if (result) {
        memcpy(result, s, len);
        result[len] = '\0';
    }
    return result;
}
#endif

#endif // GGCODE_COMPAT_H
