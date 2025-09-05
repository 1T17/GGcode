# Crash Safety Testing Suite

This directory contains comprehensive crash safety tests for the GGcode compiler.

## Directory Structure

- `config/` - Test configuration files and settings
- `framework/` - Core testing framework and utilities
- `results/` - Test execution results and reports
- `data/` - Generated test data files
- `scripts/` - Test execution and automation scripts

## Test Categories

1. **Memory Safety Tests** - Buffer overflow, memory leaks, use-after-free
2. **Stress Tests** - Large files, deep nesting, resource exhaustion
3. **Malformed Input Tests** - Binary data, invalid UTF-8, truncated files
4. **Numerical Edge Cases** - Extreme values, NaN, infinity handling
5. **Parser Robustness** - Deep expressions, syntax error recovery
6. **Concurrent Access** - Multi-process compilation, signal handling
7. **File System Edge Cases** - Permissions, long paths, symbolic links
8. **Performance Regression** - Compilation time, memory usage monitoring

## Usage

Run all crash safety tests:
```bash
make crash-safety-tests
```

Run specific test category:
```bash
./scripts/run_memory_tests.sh
./scripts/run_stress_tests.sh
```

Generate test report:
```bash
./scripts/generate_report.sh
```