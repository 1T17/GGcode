# Compiler and flags
CC = gcc
CFLAGS = -Wall -Werror -Wpedantic -Wextra \
         -DUNITY_SUPPORT_64 -DUNITY_INCLUDE_DOUBLE \
         -I./include -Isrc -Isrc/lexer -Isrc/parser -Isrc/runtime -Isrc/semantic -Isrc/generator -Isrc/utils

# Windows cross-compiler
CC_WIN = x86_64-w64-mingw32-gcc

# Main source files
SRC = $(wildcard src/*.c src/lexer/*.c src/parser/*.c src/semantic/*.c src/generator/*.c src/runtime/*.c src/utils/*.c)
OUT = ggcode

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

# Windows build target
.PHONY: win
win:
	$(CC_WIN) $(CFLAGS) -o $(OUT).exe $(SRC) -lm

# Build all test binaries (excluding src/main.c to avoid duplicate main)
tests: unity $(TEST_BINS)

bin/%: tests/%.c $(filter-out src/main.c, $(SRC)) $(UNITY)
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

# Clean build and test artifacts
.PHONY: clean
clean:
	@mkdir -p bin win
	rm -f $(OUT) bin/* win/*.exe .testlog.tmp
