#include "Unity/src/unity.h"
#include "utils/output_buffer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Move save_output_to_file implementation here for testing only
static void save_output_to_file(const char* filename) {
    const char *buffer = get_output_buffer();
    size_t length = get_output_length();
    FILE* out = fopen(filename, "w");
    if (!out) {
        perror("Failed to write output file");
        return;
    }
    fwrite(buffer, 1, length, out);
    fclose(out);
}

void setUp(void)
{
    init_output_buffer();
}

void tearDown(void)
{
    free_output_buffer();
}

void test_write_and_read_output_buffer(void)
{
    write_to_output("G1 X10 Y20");
    write_to_output("G1 X30 Y40");

    const char *buffer = get_output_buffer();
    size_t length = get_output_length();

    printf("[TEST] Buffer:\n%s\n", buffer);
    printf("[TEST] Length: %zu\n", length);

    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT_TRUE(length > 0);
    TEST_ASSERT_NOT_NULL(strstr(buffer, "G1 X10 Y20"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "G1 X30 Y40"));
}

void test_output_buffer_length(void)
{
    write_to_output("ABC");
    write_to_output("DEF");

    size_t len = get_output_length();
    printf("[TEST] Expected length: 8, Got: %zu\n", len);
    TEST_ASSERT_EQUAL_UINT(8, len); // ABC\nDEF\n = 8
}

void test_prepend_to_output_buffer(void)
{
    write_to_output("XYZ");
    prepend_to_output_buffer("START-\n");

    const char *buffer = get_output_buffer();
    printf("[TEST] Buffer after prepend:\n%s\n", buffer);
    TEST_ASSERT_TRUE(strstr(buffer, "START-\n") == buffer); // Must be at beginning
}

void test_save_output_to_file(void)
{
    write_to_output("File line A");
    write_to_output("File line B");

    const char *filename = "test_output_buffer.tmp";
    save_output_to_file(filename);

    FILE *f = fopen(filename, "r");
    TEST_ASSERT_NOT_NULL(f);

    char content[256] = {0};
    fread(content, 1, sizeof(content) - 1, f);
    fclose(f);
    remove(filename); // cleanup

    printf("[TEST] File content:\n%s\n", content);

    TEST_ASSERT_NOT_EQUAL(0, strlen(content));
    TEST_ASSERT_NOT_NULL(strstr(content, "File line A"));
    TEST_ASSERT_NOT_NULL(strstr(content, "File line B"));
}

void test_empty_buffer_should_be_null_or_empty(void)
{
    const char *buffer = get_output_buffer();
    size_t length = get_output_length();

    printf("[TEST] Empty buffer length: %zu\n", length);

    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT_EQUAL_UINT(0, length);
    TEST_ASSERT_EQUAL_STRING("", buffer);
}

void test_very_long_line_reallocation(void)
{
    char *long_line = malloc(10000);
    memset(long_line, 'X', 9999);
    long_line[9999] = '\0';

    write_to_output(long_line);

    const char *buffer = get_output_buffer();
    TEST_ASSERT_EQUAL_UINT(strlen(long_line) + 1, get_output_length()); // +1 for newline

    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT_TRUE(strstr(buffer, "XXXXX") != NULL); // crude sanity check

    free(long_line);
}

void test_multiple_prepends_order(void)
{
    write_to_output("MAIN");
    prepend_to_output_buffer("P2\n");
    prepend_to_output_buffer("P1\n");
    const char *buffer = get_output_buffer();
    printf("[TEST] Buffer with multiple prepends:\n%s\n", buffer);
    TEST_ASSERT_TRUE(strstr(buffer, "P1\nP2\nMAIN") == buffer);
}

void test_prepend_after_large_write(void)
{
    char *huge = malloc(5000);
    memset(huge, 'A', 4999);
    huge[4999] = '\0';

    write_to_output(huge);
    prepend_to_output_buffer("START\n");

    const char *buffer = get_output_buffer();
    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT_TRUE(strstr(buffer, "START\n") == buffer);

    free(huge);
}

void test_no_trailing_newline_write(void)
{
    const char *raw = "G0 X5 Y5 Z5";
    write_to_output(raw);

    size_t len = get_output_length();
    const char *buffer = get_output_buffer();

    printf("[TEST] Raw length: %zu, With newline: %zu\n", strlen(raw), len);

    TEST_ASSERT_EQUAL_UINT(strlen(raw) + 1, len);
    TEST_ASSERT_EQUAL_CHAR('\n', buffer[len - 1]);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_write_and_read_output_buffer);         // 1
    
    RUN_TEST(test_output_buffer_length);                 // 2
    RUN_TEST(test_prepend_to_output_buffer);             // 3
    RUN_TEST(test_save_output_to_file);                  // 4
    RUN_TEST(test_empty_buffer_should_be_null_or_empty); // 5
    RUN_TEST(test_very_long_line_reallocation);          // 6
    RUN_TEST(test_multiple_prepends_order);              // 7
    RUN_TEST(test_prepend_after_large_write);            // 8
    RUN_TEST(test_no_trailing_newline_write);            // 9
    return UNITY_END();
}
