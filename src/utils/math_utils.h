#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <math.h>

// Modern IEEE 754 compliant division with proper error handling
double safe_divide(double numerator, double denominator);

// Check if a floating-point number is safe for calculations
int is_safe_number(double value);

// Clamp a value to prevent overflow/underflow
double clamp_safe(double value, double min_val, double max_val);

#endif // MATH_UTILS_H