#ifndef GGCODE_COMPAT_H
#define GGCODE_COMPAT_H

#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(__MINGW32__)

// Windows doesn't have strndup, so we provide our own
static char* strndup_portable(const char* s, size_t n) {
    size_t len = strnlen(s, n);
    char* result = (char*)malloc(len + 1);
    if (result) {
        memcpy(result, s, len);
        result[len] = '\0';
    }
    return result;
}

#else

// On Linux/macOS just use strndup directly
#define strndup_portable strndup

#endif

#endif // GGCODE_COMPAT_H
