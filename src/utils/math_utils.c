#include "math_utils.h"
#include "../error/error.h"
#include <math.h>

// Modern IEEE 754 compliant division with proper error handling
double safe_divide(double numerator, double denominator) {
    if (denominator == 0.0) {
        if (numerator > 0.0) {
            report_error("[Math Utils] Division by zero: %.5f / 0.0 = +∞", numerator);
            return INFINITY;
        } else if (numerator < 0.0) {
            report_error("[Math Utils] Division by zero: %.5f / 0.0 = -∞", numerator);
            return -INFINITY;
        } else {
            report_error("[Math Utils] Indeterminate form: 0.0 / 0.0 = NaN");
            return NAN;
        }
    }
    
    double result = numerator / denominator;
    
    // Check for overflow/underflow
    if (isinf(result)) {
        report_error("[Math Utils] Division overflow: %.5f / %.5f = %s∞", 
                    numerator, denominator, (result > 0) ? "+" : "-");
    } else if (isnan(result)) {
        report_error("[Math Utils] Division resulted in NaN: %.5f / %.5f", 
                    numerator, denominator);
    }
    
    return result;
}

// Check if a floating-point number is safe for calculations
int is_safe_number(double value) {
    return !isnan(value) && !isinf(value);
}

// Clamp a value to prevent overflow/underflow
double clamp_safe(double value, double min_val, double max_val) {
    if (isnan(value)) {
        report_error("[Math Utils] Attempting to clamp NaN value, returning 0.0");
        return 0.0;
    }
    
    if (isinf(value)) {
        if (value > 0) {
            report_error("[Math Utils] Clamping +∞ to maximum value %.5f", max_val);
            return max_val;
        } else {
            report_error("[Math Utils] Clamping -∞ to minimum value %.5f", min_val);
            return min_val;
        }
    }
    
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}