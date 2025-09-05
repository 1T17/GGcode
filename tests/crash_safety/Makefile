# Makefile for Crash Safety Tests
# This makefile builds and runs comprehensive memory corruption detection tests

# Project configuration
PROJECT_ROOT = ../..
SRC_DIR = $(PROJECT_ROOT)/src
UNITY_DIR = ../Unity/src
FRAMEWORK_DIR = framework
CONFIG_DIR = config
SCRIPTS_DIR = scripts

# Compiler configuration
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O0 -fno-omit-frame-pointer
INCLUDES = -I$(PROJECT_ROOT) -I$(UNITY_DIR) -I$(FRAMEWORK_DIR) -I$(CONFIG_DIR) -I$(SRC_DIR)
LIBS = -lm -lpthread

# AddressSanitizer configuration
ASAN_CFLAGS = $(CFLAGS) -fsanitize=address -fno-optimize-sibling-calls
ASAN_LDFLAGS = -fsanitize=address

# Valgrind configuration
VALGRIND_FLAGS = --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1

# Source files
FRAMEWORK_SOURCES = $(FRAMEWORK_DIR)/memory_safety.c \
                   $(FRAMEWORK_DIR)/crash_detection.c \
                   $(FRAMEWORK_DIR)/test_runner.c

CONFIG_SOURCES = $(CONFIG_DIR)/crash_test_config.c

UNITY_SOURCES = $(UNITY_DIR)/unity.c

# GGcode compiler sources (needed for integration tests)
GGCODE_SOURCES = $(SRC_DIR)/lexer/lexer.c \
                $(SRC_DIR)/lexer/token_utils.c \
                $(SRC_DIR)/parser/parser.c \
                $(SRC_DIR)/parser/ast_helpers.c \
                $(SRC_DIR)/generator/emitter.c \
                $(SRC_DIR)/runtime/evaluator.c \
                $(SRC_DIR)/utils/output_buffer.c \
                $(SRC_DIR)/utils/file_utils.c \
                $(SRC_DIR)/utils/math_utils.c \
                $(SRC_DIR)/utils/report.c \
                $(SRC_DIR)/config/config.c \
                $(SRC_DIR)/error/error.c

# Test executables
TEST_EXECUTABLES = test_buffer_overflow \
                  test_use_after_free \
                  test_double_free \
                  test_memory_corruption_integration \
                  test_stack_overflow_protection

# ASAN test executables
ASAN_TEST_EXECUTABLES = $(TEST_EXECUTABLES:%=%_asan)

# Results directories
RESULTS_DIR = results
VALGRIND_RESULTS_DIR = $(RESULTS_DIR)/valgrind
ASAN_RESULTS_DIR = $(RESULTS_DIR)/asan
REPORTS_DIR = $(RESULTS_DIR)/reports

# Default target
.PHONY: all
all: setup build

# Setup directories
.PHONY: setup
setup:
	@echo "🔧 Setting up crash safety test environment..."
	@mkdir -p $(RESULTS_DIR) $(VALGRIND_RESULTS_DIR) $(ASAN_RESULTS_DIR) $(REPORTS_DIR)
	@mkdir -p data
	@echo "✅ Environment setup complete"

# Build all tests
.PHONY: build
build: $(TEST_EXECUTABLES)
	@echo "✅ All crash safety tests built successfully"

# Build ASAN tests
.PHONY: build-asan
build-asan: $(ASAN_TEST_EXECUTABLES)
	@echo "✅ All AddressSanitizer tests built successfully"

# Individual test targets
test_buffer_overflow: test_buffer_overflow.c $(FRAMEWORK_SOURCES) $(CONFIG_SOURCES) $(UNITY_SOURCES)
	@echo "🔨 Building buffer overflow tests..."
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

test_use_after_free: test_use_after_free.c $(FRAMEWORK_SOURCES) $(CONFIG_SOURCES) $(UNITY_SOURCES)
	@echo "🔨 Building use-after-free tests..."
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

test_double_free: test_double_free.c $(FRAMEWORK_SOURCES) $(CONFIG_SOURCES) $(UNITY_SOURCES)
	@echo "🔨 Building double-free tests..."
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

test_memory_corruption_integration: test_memory_corruption_integration.c $(FRAMEWORK_SOURCES) $(CONFIG_SOURCES) $(UNITY_SOURCES) $(GGCODE_SOURCES)
	@echo "🔨 Building memory corruption integration tests..."
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

test_stack_overflow_protection: test_stack_overflow_protection.c $(FRAMEWORK_SOURCES) $(CONFIG_SOURCES) $(UNITY_SOURCES) $(GGCODE_SOURCES)
	@echo "🔨 Building stack overflow protection tests..."
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

# ASAN test targets
test_buffer_overflow_asan: test_buffer_overflow.c $(FRAMEWORK_SOURCES) $(CONFIG_SOURCES) $(UNITY_SOURCES)
	@echo "🔨 Building buffer overflow tests with AddressSanitizer..."
	$(CC) $(ASAN_CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS) $(ASAN_LDFLAGS)

test_use_after_free_asan: test_use_after_free.c $(FRAMEWORK_SOURCES) $(CONFIG_SOURCES) $(UNITY_SOURCES)
	@echo "🔨 Building use-after-free tests with AddressSanitizer..."
	$(CC) $(ASAN_CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS) $(ASAN_LDFLAGS)

test_double_free_asan: test_double_free.c $(FRAMEWORK_SOURCES) $(CONFIG_SOURCES) $(UNITY_SOURCES)
	@echo "🔨 Building double-free tests with AddressSanitizer..."
	$(CC) $(ASAN_CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS) $(ASAN_LDFLAGS)

test_memory_corruption_integration_asan: test_memory_corruption_integration.c $(FRAMEWORK_SOURCES) $(CONFIG_SOURCES) $(UNITY_SOURCES) $(GGCODE_SOURCES)
	@echo "🔨 Building memory corruption integration tests with AddressSanitizer..."
	$(CC) $(ASAN_CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS) $(ASAN_LDFLAGS)

test_stack_overflow_protection_asan: test_stack_overflow_protection.c $(FRAMEWORK_SOURCES) $(CONFIG_SOURCES) $(UNITY_SOURCES) $(GGCODE_SOURCES)
	@echo "🔨 Building stack overflow protection tests with AddressSanitizer..."
	$(CC) $(ASAN_CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS) $(ASAN_LDFLAGS)

# Run tests
.PHONY: test
test: build
	@echo "🧪 Running crash safety tests..."
	@for test in $(TEST_EXECUTABLES); do \
		echo "Running $$test..."; \
		./$$test || echo "❌ $$test failed"; \
	done
	@echo "✅ Crash safety tests completed"

# Run tests with Valgrind
.PHONY: test-valgrind
test-valgrind: build
	@echo "🧪 Running crash safety tests with Valgrind..."
	@mkdir -p $(VALGRIND_RESULTS_DIR)/manual_run_$(shell date +%Y%m%d_%H%M%S)
	@for test in $(TEST_EXECUTABLES); do \
		echo "Running $$test with Valgrind..."; \
		valgrind $(VALGRIND_FLAGS) --log-file=$(VALGRIND_RESULTS_DIR)/manual_run_$(shell date +%Y%m%d_%H%M%S)/$$test.log ./$$test || echo "❌ $$test failed with Valgrind"; \
	done
	@echo "✅ Valgrind tests completed"

# Run tests with AddressSanitizer
.PHONY: test-asan
test-asan: build-asan
	@echo "🧪 Running crash safety tests with AddressSanitizer..."
	@mkdir -p $(ASAN_RESULTS_DIR)/manual_run_$(shell date +%Y%m%d_%H%M%S)
	@export ASAN_OPTIONS="abort_on_error=1:halt_on_error=1:check_initialization_order=1:detect_leaks=1:log_path=$(ASAN_RESULTS_DIR)/manual_run_$(shell date +%Y%m%d_%H%M%S)/asan_log"; \
	for test in $(ASAN_TEST_EXECUTABLES); do \
		echo "Running $$test with AddressSanitizer..."; \
		./$$test || echo "❌ $$test failed with AddressSanitizer"; \
	done
	@echo "✅ AddressSanitizer tests completed"

# Run comprehensive memory analysis
.PHONY: test-comprehensive
test-comprehensive: build build-asan
	@echo "🛡️ Running comprehensive memory safety analysis..."
	@$(SCRIPTS_DIR)/valgrind_integration.sh
	@$(SCRIPTS_DIR)/asan_integration.sh
	@$(SCRIPTS_DIR)/memory_analysis_report.sh
	@echo "✅ Comprehensive memory analysis completed"

# Quick smoke test
.PHONY: test-quick
test-quick: build
	@echo "🚀 Running quick memory safety smoke test..."
	@./test_buffer_overflow
	@./test_use_after_free
	@echo "✅ Quick smoke test completed"

# Performance test
.PHONY: test-performance
test-performance: build
	@echo "⚡ Running memory safety performance tests..."
	@time ./test_memory_corruption_integration
	@echo "✅ Performance tests completed"

# Memory leak specific tests
.PHONY: test-leaks
test-leaks: build
	@echo "🔍 Running memory leak detection tests..."
	@valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./test_memory_corruption_integration
	@echo "✅ Memory leak tests completed"

# Buffer overflow specific tests
.PHONY: test-buffer-overflow
test-buffer-overflow: build
	@echo "💥 Running buffer overflow detection tests..."
	@./test_buffer_overflow
	@echo "✅ Buffer overflow tests completed"

# Use-after-free specific tests
.PHONY: test-use-after-free
test-use-after-free: build
	@echo "🔄 Running use-after-free detection tests..."
	@./test_use_after_free
	@echo "✅ Use-after-free tests completed"

# Double-free specific tests
.PHONY: test-double-free
test-double-free: build
	@echo "🔂 Running double-free detection tests..."
	@./test_double_free
	@echo "✅ Double-free tests completed"

# Stack overflow specific tests
.PHONY: test-stack-overflow
test-stack-overflow: build
	@echo "📚 Running stack overflow protection tests..."
	@./test_stack_overflow_protection
	@echo "✅ Stack overflow tests completed"

# Integration tests
.PHONY: test-integration
test-integration: build
	@echo "🔗 Running memory corruption integration tests..."
	@./test_memory_corruption_integration
	@echo "✅ Integration tests completed"

# Generate test report
.PHONY: report
report:
	@echo "📊 Generating memory safety test report..."
	@$(SCRIPTS_DIR)/memory_analysis_report.sh
	@echo "✅ Report generated"

# Clean build artifacts
.PHONY: clean
clean:
	@echo "🧹 Cleaning crash safety test artifacts..."
	@rm -f $(TEST_EXECUTABLES) $(ASAN_TEST_EXECUTABLES)
	@rm -f *.o *.log *.tmp
	@echo "✅ Clean completed"

# Clean all results
.PHONY: clean-results
clean-results:
	@echo "🧹 Cleaning test results..."
	@rm -rf $(RESULTS_DIR)/*
	@echo "✅ Results cleaned"

# Full clean
.PHONY: clean-all
clean-all: clean clean-results
	@echo "🧹 Full clean completed"

# Install dependencies (Ubuntu/Debian)
.PHONY: install-deps
install-deps:
	@echo "📦 Installing crash safety test dependencies..."
	@sudo apt-get update
	@sudo apt-get install -y valgrind gcc libc6-dev build-essential
	@echo "✅ Dependencies installed"

# Check environment
.PHONY: check-env
check-env:
	@echo "🔍 Checking crash safety test environment..."
	@echo "GCC version: $$(gcc --version | head -n1)"
	@echo "Valgrind available: $$(command -v valgrind >/dev/null && echo 'Yes' || echo 'No')"
	@echo "AddressSanitizer support: $$(gcc -fsanitize=address -x c /dev/null -o /dev/null 2>/dev/null && echo 'Yes' || echo 'No')"
	@echo "Unity framework: $$(test -f $(UNITY_DIR)/unity.c && echo 'Available' || echo 'Missing')"
	@echo "✅ Environment check completed"

# Help target
.PHONY: help
help:
	@echo "🛡️ Crash Safety Test Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all                    - Setup and build all tests"
	@echo "  build                  - Build all test executables"
	@echo "  build-asan             - Build tests with AddressSanitizer"
	@echo "  test                   - Run all crash safety tests"
	@echo "  test-valgrind          - Run tests with Valgrind"
	@echo "  test-asan              - Run tests with AddressSanitizer"
	@echo "  test-comprehensive     - Run full memory analysis suite"
	@echo "  test-quick             - Run quick smoke tests"
	@echo "  test-performance       - Run performance tests"
	@echo "  test-leaks             - Run memory leak tests"
	@echo "  test-buffer-overflow   - Run buffer overflow tests"
	@echo "  test-use-after-free    - Run use-after-free tests"
	@echo "  test-double-free       - Run double-free tests"
	@echo "  test-stack-overflow    - Run stack overflow tests"
	@echo "  test-integration       - Run integration tests"
	@echo "  report                 - Generate test report"
	@echo "  clean                  - Clean build artifacts"
	@echo "  clean-results          - Clean test results"
	@echo "  clean-all              - Full clean"
	@echo "  install-deps           - Install dependencies"
	@echo "  check-env              - Check environment"
	@echo "  help                   - Show this help"

# Phony targets
.PHONY: all setup build build-asan test test-valgrind test-asan test-comprehensive \
        test-quick test-performance test-leaks test-buffer-overflow test-use-after-free \
        test-double-free test-stack-overflow test-integration report clean clean-results \
        clean-all install-deps check-env help