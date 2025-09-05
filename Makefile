# Compiler and flags
CC = gcc
CFLAGS = -g -Wall -Werror -Wpedantic -Wextra \
         -DUNITY_SUPPORT_64 -DUNITY_INCLUDE_DOUBLE \
         -I./include -Isrc -Isrc/lexer -Isrc/parser -Isrc/runtime -Isrc/semantic -Isrc/generator -Isrc/utils

# Windows cross-compiler
CC_WIN = x86_64-w64-mingw32-gcc

# Main source files
SRC = $(wildcard src/*.c src/lexer/*.c src/error/*.c src/parser/*.c src/semantic/*.c src/generator/*.c src/runtime/*.c src/utils/*.c src/config/*.c src/cli/*.c)
OUT = GGCODE/ggcode

# Test discovery
TEST_SRC := $(wildcard tests/test_*.c)
TEST_BINS := $(patsubst tests/%.c,bin/%,$(TEST_SRC))
UNITY = tests/Unity/src/unity.c

UNITY_URL = https://github.com/ThrowTheSwitch/Unity/archive/refs/heads/master.zip
UNITY_DIR = tests/Unity

# Main target
all: $(OUT)

# Build main program
$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ -lm
	@echo "✅ Build complete: $(OUT)"
	@$(MAKE) -s prompt-install


# Windows build target with psapi for memory info
.PHONY: win
win:
	$(CC_WIN) $(CFLAGS) -o $(OUT).exe $(SRC) -lm -lpsapi

# Build all test binaries (excluding src/main.c to avoid duplicate main)
tests: unity $(TEST_BINS)

# Special rule for security test that needs CLI functions
bin/test_security_buffer_overflow: tests/test_security_buffer_overflow.c $(filter-out src/main.c, $(SRC)) $(UNITY)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^ -lm

# General rule for other tests (excludes CLI to avoid compile_file dependency)
bin/%: tests/%.c $(filter-out src/main.c src/cli/cli.c, $(SRC)) $(UNITY)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^ -lm

# Run all tests with final summary
.PHONY: test
test: tests
	@echo ""
	@PASS_TOTAL=0; FAIL_TOTAL=0; \
	for t in $(TEST_BINS); do \
		echo "🧪 Running $$t..."; \
		./$$t > .testlog.tmp; cat .testlog.tmp; \
		PASS=$$(grep -c ":PASS" .testlog.tmp); \
		FAIL=$$(grep -c ":FAIL" .testlog.tmp); \
		PASS_TOTAL=$$((PASS_TOTAL + PASS)); \
		FAIL_TOTAL=$$((FAIL_TOTAL + FAIL)); \
		echo "→ Summary for $$t: $$PASS Pass, $$FAIL Fail"; \
		echo ""; \
		if [ $$FAIL -gt 0 ]; then exit 1; fi; \
	done; \
	rm -f .testlog.tmp; \
	echo "============================="; \
	echo "✅ All Tests Done"; \
	echo "🧪 Total: $$PASS_TOTAL Pass, $$FAIL_TOTAL Fail"; \
	echo "============================="

# Download and setup Unity framework
.PHONY: unity
unity:
	@if [ ! -d "$(UNITY_DIR)" ]; then \
		echo "Downloading Unity test framework..."; \
		mkdir -p tests; \
		curl -L $(UNITY_URL) -o tests/unity.zip; \
		unzip -q tests/unity.zip -d tests; \
		mv tests/Unity-master $(UNITY_DIR); \
		rm tests/unity.zip; \
		echo "Unity downloaded to $(UNITY_DIR)"; \
	else \
		echo "Unity already present."; \
	fi



.PHONY: node
node:
	@mkdir -p node
	$(CC) -shared -fPIC -o node/libggcode.so \
	    src/bindings/nodejs.c $(SRC) \
	    $(CFLAGS) -lm





# Interactive global installation prompt
.PHONY: prompt-install
prompt-install:
	@echo ""
	@echo "🌍 Install ggcode globally? (y/N): "; \
	read -r response; \
	case "$$response" in \
		[yY][eE][sS]|[yY]) \
			$(MAKE) -s install-global; \
			;; \
		*) \
			echo "Skipped global installation. Use 'make install' later if needed."; \
			echo "Or run: export PATH=\"\$$PATH:\$$(pwd)/GGCODE\""; \
			;; \
	esac

# Global installation options
.PHONY: install-global install
install-global install:
	@echo "Choose installation method:"
	@echo "  1) Symlink to /usr/local/bin (recommended)"
	@echo "  2) Copy to /usr/local/bin"
	@echo "  3) Add to PATH in ~/.bashrc"
	@echo "Enter choice (1-3): "; \
	read -r choice; \
	case "$$choice" in \
		1) \
			if [ -L /usr/local/bin/ggcode ]; then \
				echo "🔄 Updating existing symlink..."; \
				sudo rm /usr/local/bin/ggcode; \
			fi; \
			sudo ln -sf $$(pwd)/$(OUT) /usr/local/bin/ggcode; \
			echo "✅ Symlink created: /usr/local/bin/ggcode -> $$(pwd)/$(OUT)"; \
			;; \
		2) \
			sudo cp $(OUT) /usr/local/bin/ggcode; \
			sudo chmod +x /usr/local/bin/ggcode; \
			echo "✅ Binary copied to /usr/local/bin/ggcode"; \
			;; \
		3) \
			if ! grep -q "GGCODE" ~/.bashrc; then \
				echo "export PATH=\"\$$PATH:$$(pwd)/GGCODE\"" >> ~/.bashrc; \
				echo "✅ Added to ~/.bashrc"; \
				echo "Run: source ~/.bashrc"; \
			else \
				echo "⚠️  PATH already contains GGCODE directory"; \
			fi; \
			;; \
		*) \
			echo "Invalid choice. Skipped installation."; \
			;; \
	esac
	@echo ""
	@echo "🧪 Test global installation:"
	@echo "  ggcode --version"
	@echo "  ggcode -e \"G1 X10 Y20 F300\""

# Uninstall global installation
.PHONY: uninstall
uninstall:
	@echo "🗑️  Removing global ggcode installation..."
	@if [ -f /usr/local/bin/ggcode ] || [ -L /usr/local/bin/ggcode ]; then \
		sudo rm /usr/local/bin/ggcode; \
		echo "✅ Removed /usr/local/bin/ggcode"; \
	else \
		echo "No global installation found in /usr/local/bin/"; \
	fi
	@if grep -q "GGCODE" ~/.bashrc 2>/dev/null; then \
		echo "⚠️  Found GGCODE in ~/.bashrc - remove manually if needed"; \
	fi

# Crash Safety Testing
CRASH_TEST_DIR = tests/crash_safety
CRASH_TEST_SRC = $(wildcard $(CRASH_TEST_DIR)/config/*.c $(CRASH_TEST_DIR)/framework/*.c)
CRASH_TEST_INCLUDES = -I$(CRASH_TEST_DIR)/config -I$(CRASH_TEST_DIR)/framework

# Build crash safety test infrastructure
.PHONY: crash-safety-build
crash-safety-build: unity
	@echo "🛡️ Building crash safety test infrastructure..."
	@mkdir -p bin/crash_safety
	$(CC) $(CFLAGS) $(CRASH_TEST_INCLUDES) -I$(UNITY_DIR)/src \
		-o bin/crash_safety/test_infrastructure \
		$(CRASH_TEST_DIR)/test_infrastructure.c \
		$(CRASH_TEST_SRC) $(UNITY) -lm
	@echo "✅ Crash safety infrastructure built"

# Run crash safety infrastructure test
.PHONY: crash-safety-test-infra
crash-safety-test-infra: crash-safety-build
	@echo "🧪 Testing crash safety infrastructure..."
	@./bin/crash_safety/test_infrastructure

# Run all crash safety tests
.PHONY: crash-safety-tests
crash-safety-tests: crash-safety-test-infra
	@echo "🛡️ Running comprehensive crash safety tests..."
	@$(CRASH_TEST_DIR)/scripts/run_crash_tests.sh

# Run quick crash safety tests (high priority only)
.PHONY: crash-safety-quick
crash-safety-quick: crash-safety-test-infra
	@echo "🛡️ Running quick crash safety tests..."
	@$(CRASH_TEST_DIR)/scripts/run_crash_tests.sh --quick

# Run full crash safety tests with memory checking
.PHONY: crash-safety-full
crash-safety-full: crash-safety-test-infra
	@echo "🛡️ Running full crash safety tests with memory checking..."
	@$(CRASH_TEST_DIR)/scripts/run_crash_tests.sh --full --valgrind

# Clean crash safety test artifacts
.PHONY: crash-safety-clean
crash-safety-clean:
	@echo "🧹 Cleaning crash safety test artifacts..."
	@rm -rf bin/crash_safety
	@rm -rf $(CRASH_TEST_DIR)/results/crash_test_*
	@rm -f $(CRASH_TEST_DIR)/Makefile
	@rm -f $(CRASH_TEST_DIR)/crash_test_runner

# Clean build and test artifacts
.PHONY: clean
clean:
	@mkdir -p bin
	rm -f $(OUT) win/*.exe .testlog.tmp
	@find bin -name "test_*" -type f -delete 2>/dev/null || true
