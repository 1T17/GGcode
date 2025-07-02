#ifndef GGCODE_TIME_UTILS_H
#define GGCODE_TIME_UTILS_H

#ifdef _WIN32
#include <windows.h>

typedef struct {
    LARGE_INTEGER freq;
    LARGE_INTEGER start;
} Timer;

static void start_timer(Timer* t) {
    QueryPerformanceFrequency(&t->freq);
    QueryPerformanceCounter(&t->start);
}

static double end_timer(Timer* t) {
    LARGE_INTEGER end;
    QueryPerformanceCounter(&end);
    return (double)(end.QuadPart - t->start.QuadPart) / t->freq.QuadPart;
}

#else
#include <time.h>

typedef struct {
    struct timespec start;
} Timer;

static void start_timer(Timer* t) {
    clock_gettime(CLOCK_MONOTONIC, &t->start);
}

static double end_timer(Timer* t) {
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    return (end.tv_sec - t->start.tv_sec) + (end.tv_nsec - t->start.tv_nsec) / 1e9;
}
#endif

#endif // GGCODE_TIME_UTILS_H
