// Comprehensive Unit Tests for QB64 String (qbs) System
// Tests string creation, manipulation, conversion, and memory management

#include "test.h"
#include "qbs.h"
#include <string.h>

// ============================================================================
// STRING CREATION TESTS
// ============================================================================

static void test_qbs_new_empty()
{
    qbs *str = qbs_new(0, 0);
    test_assert(str != NULL);
    test_assert_ints(0, str->len);
    qbs_free(str);
}

static void test_qbs_new_length()
{
    qbs *str = qbs_new(10, 0);
    test_assert(str != NULL);
    test_assert_ints(10, str->len);
    test_assert(str->chr != NULL);
    qbs_free(str);
}

static void test_qbs_new_txt_basic()
{
    qbs *str = qbs_new_txt("Hello, World!");
    test_assert(str != NULL);
    test_assert_ints(13, str->len);
    test_assert(str->chr != NULL);
    test_assert_buffers("Hello, World!", (char*)str->chr, 13);
    qbs_free(str);
}

static void test_qbs_new_txt_empty()
{
    qbs *str = qbs_new_txt("");
    test_assert(str != NULL);
    test_assert_ints(0, str->len);
    qbs_free(str);
}

static void test_qbs_new_txt_len()
{
    qbs *str = qbs_new_txt_len("Hello, World!", 5);  // Only "Hello"
    test_assert(str != NULL);
    test_assert_ints(5, str->len);
    test_assert_buffers("Hello", (char*)str->chr, 5);
    qbs_free(str);
}

static void test_qbs_new_txt_len_with_nulls()
{
    // Test that qbs can contain null bytes
    const char data[] = {'A', '\0', 'B', '\0', 'C'};
    qbs *str = qbs_new_txt_len(data, 5);
    test_assert(str != NULL);
    test_assert_ints(5, str->len);
    test_assert_buffers(data, (char*)str->chr, 5);
    qbs_free(str);
}

// ============================================================================
// STRING CONCATENATION AND ASSIGNMENT
// ============================================================================

static void test_qbs_add_basic()
{
    qbs *str1 = qbs_new_txt("Hello, ");
    qbs *str2 = qbs_new_txt("World!");
    qbs *result = qbs_add(str1, str2);

    test_assert(result != NULL);
    test_assert_ints(13, result->len);
    test_assert_buffers("Hello, World!", (char*)result->chr, 13);

    qbs_free(str1);
    qbs_free(str2);
    qbs_free(result);
}

static void test_qbs_add_empty_strings()
{
    qbs *str1 = qbs_new_txt("");
    qbs *str2 = qbs_new_txt("Test");
    qbs *result = qbs_add(str1, str2);

    test_assert(result != NULL);
    test_assert_ints(4, result->len);
    test_assert_buffers("Test", (char*)result->chr, 4);

    qbs_free(str1);
    qbs_free(str2);
    qbs_free(result);
}

static void test_qbs_set_basic()
{
    qbs *str1 = qbs_new_txt("Original");
    qbs *str2 = qbs_new_txt("New");
    qbs *result = qbs_set(str1, str2);

    test_assert(result != NULL);
    test_assert_ints(3, result->len);
    test_assert_buffers("New", (char*)result->chr, 3);

    qbs_free(str1);
    qbs_free(str2);
}

static void test_qbs_set_longer()
{
    qbs *str1 = qbs_new_txt("Short");
    qbs *str2 = qbs_new_txt("Much Longer String");
    qbs *result = qbs_set(str1, str2);

    test_assert(result != NULL);
    test_assert_ints(18, result->len);
    test_assert_buffers("Much Longer String", (char*)result->chr, 18);

    qbs_free(str1);
    qbs_free(str2);
}

// ============================================================================
// CASE CONVERSION TESTS
// ============================================================================

static void test_qbs_ucase_basic()
{
    qbs *str = qbs_new_txt("hello world");
    qbs *upper = qbs_ucase(str);

    test_assert(upper != NULL);
    test_assert_ints(11, upper->len);
    test_assert_buffers("HELLO WORLD", (char*)upper->chr, 11);

    qbs_free(str);
    qbs_free(upper);
}

static void test_qbs_ucase_mixed()
{
    qbs *str = qbs_new_txt("HeLLo WoRLd");
    qbs *upper = qbs_ucase(str);

    test_assert(upper != NULL);
    test_assert_buffers("HELLO WORLD", (char*)upper->chr, 11);

    qbs_free(str);
    qbs_free(upper);
}

static void test_qbs_lcase_basic()
{
    qbs *str = qbs_new_txt("HELLO WORLD");
    qbs *lower = qbs_lcase(str);

    test_assert(lower != NULL);
    test_assert_ints(11, lower->len);
    test_assert_buffers("hello world", (char*)lower->chr, 11);

    qbs_free(str);
    qbs_free(lower);
}

// ============================================================================
// SUBSTRING TESTS
// ============================================================================

static void test_qbs_left_basic()
{
    qbs *str = qbs_new_txt("Hello, World!");
    qbs *left = qbs_left(str, 5);

    test_assert(left != NULL);
    test_assert_ints(5, left->len);
    test_assert_buffers("Hello", (char*)left->chr, 5);

    qbs_free(str);
    qbs_free(left);
}

static void test_qbs_left_full_string()
{
    qbs *str = qbs_new_txt("Test");
    qbs *left = qbs_left(str, 10);  // Request more than available

    test_assert(left != NULL);
    test_assert_ints(4, left->len);  // Should return full string
    test_assert_buffers("Test", (char*)left->chr, 4);

    qbs_free(str);
    qbs_free(left);
}

static void test_qbs_right_basic()
{
    qbs *str = qbs_new_txt("Hello, World!");
    qbs *right = qbs_right(str, 6);

    test_assert(right != NULL);
    test_assert_ints(6, right->len);
    test_assert_buffers("World!", (char*)right->chr, 6);

    qbs_free(str);
    qbs_free(right);
}

static void test_func_mid_basic()
{
    qbs *str = qbs_new_txt("Hello, World!");
    qbs *mid = func_mid(str, 8, 5, 1);  // Start at 8, length 5

    test_assert(mid != NULL);
    test_assert_ints(5, mid->len);
    test_assert_buffers("World", (char*)mid->chr, 5);

    qbs_free(str);
    qbs_free(mid);
}

static void test_func_mid_to_end()
{
    qbs *str = qbs_new_txt("Hello, World!");
    qbs *mid = func_mid(str, 8, 0, 0);  // Start at 8, to end

    test_assert(mid != NULL);
    test_assert_ints(6, mid->len);
    test_assert_buffers("World!", (char*)mid->chr, 6);

    qbs_free(str);
    qbs_free(mid);
}

// ============================================================================
// STRING TRIMMING TESTS
// ============================================================================

static void test_qbs_ltrim_spaces()
{
    qbs *str = qbs_new_txt("   Hello");
    qbs *trimmed = qbs_ltrim(str);

    test_assert(trimmed != NULL);
    test_assert_ints(5, trimmed->len);
    test_assert_buffers("Hello", (char*)trimmed->chr, 5);

    qbs_free(str);
    qbs_free(trimmed);
}

static void test_qbs_ltrim_tabs()
{
    qbs *str = qbs_new_txt("\t\tTest");
    qbs *trimmed = qbs_ltrim(str);

    test_assert(trimmed != NULL);
    test_assert_ints(4, trimmed->len);
    test_assert_buffers("Test", (char*)trimmed->chr, 4);

    qbs_free(str);
    qbs_free(trimmed);
}

static void test_qbs_rtrim_spaces()
{
    qbs *str = qbs_new_txt("Hello   ");
    qbs *trimmed = qbs_rtrim(str);

    test_assert(trimmed != NULL);
    test_assert_ints(5, trimmed->len);
    test_assert_buffers("Hello", (char*)trimmed->chr, 5);

    qbs_free(str);
    qbs_free(trimmed);
}

static void test_qbs__trim_both()
{
    qbs *str = qbs_new_txt("  Hello  ");
    qbs *trimmed = qbs__trim(str);

    test_assert(trimmed != NULL);
    test_assert_ints(5, trimmed->len);
    test_assert_buffers("Hello", (char*)trimmed->chr, 5);

    qbs_free(str);
    qbs_free(trimmed);
}

// ============================================================================
// NUMERIC CONVERSION TESTS
// ============================================================================

static void test_qbs_str_int32_positive()
{
    qbs *str = qbs_str((int32_t)42);
    test_assert(str != NULL);
    test_assert(str->len > 0);
    // STR$ adds leading space for positive numbers
    qbs_free(str);
}

static void test_qbs_str_int32_negative()
{
    qbs *str = qbs_str((int32_t)-42);
    test_assert(str != NULL);
    test_assert(str->len > 0);
    qbs_free(str);
}

static void test_qbs_str_int64()
{
    qbs *str = qbs_str((int64_t)123456789);
    test_assert(str != NULL);
    test_assert(str->len > 0);
    qbs_free(str);
}

static void test_qbs_str_uint64()
{
    qbs *str = qbs_str((uint64_t)18446744073709551615ULL);  // Max uint64
    test_assert(str != NULL);
    test_assert(str->len > 0);
    qbs_free(str);
}

static void test_qbs_str_float()
{
    qbs *str = qbs_str(3.14f);
    test_assert(str != NULL);
    test_assert(str->len > 0);
    qbs_free(str);
}

static void test_qbs_str_double()
{
    qbs *str = qbs_str(2.718281828);
    test_assert(str != NULL);
    test_assert(str->len > 0);
    qbs_free(str);
}

static void test_qbs_str_zero()
{
    qbs *str = qbs_str((int32_t)0);
    test_assert(str != NULL);
    test_assert(str->len > 0);
    qbs_free(str);
}

// ============================================================================
// STRING UTILITY TESTS
// ============================================================================

static void test_func_chr_basic()
{
    qbs *str = func_chr(65);  // 'A'
    test_assert(str != NULL);
    test_assert_ints(1, str->len);
    test_assert_ints('A', str->chr[0]);
    qbs_free(str);
}

static void test_func_chr_zero()
{
    qbs *str = func_chr(0);
    test_assert(str != NULL);
    test_assert_ints(1, str->len);
    test_assert_ints(0, str->chr[0]);
    qbs_free(str);
}

static void test_func_space_basic()
{
    qbs *str = func_space(5);
    test_assert(str != NULL);
    test_assert_ints(5, str->len);
    for (int i = 0; i < 5; i++) {
        test_assert_ints(' ', str->chr[i]);
    }
    qbs_free(str);
}

static void test_func_space_zero()
{
    qbs *str = func_space(0);
    test_assert(str != NULL);
    test_assert_ints(0, str->len);
    qbs_free(str);
}

static void test_func_string_basic()
{
    qbs *str = func_string(3, 65);  // "AAA"
    test_assert(str != NULL);
    test_assert_ints(3, str->len);
    test_assert_buffers("AAA", (char*)str->chr, 3);
    qbs_free(str);
}

static void test_qbs_asc_basic()
{
    qbs *str = qbs_new_txt("ABC");
    test_assert_ints(65, qbs_asc(str));  // 'A'
    qbs_free(str);
}

static void test_qbs_asc_at_position()
{
    qbs *str = qbs_new_txt("ABC");
    test_assert_ints(66, qbs_asc(str, 2));  // 'B' (1-indexed in QB64)
    test_assert_ints(67, qbs_asc(str, 3));  // 'C'
    qbs_free(str);
}

// ============================================================================
// EDGE CASE TESTS
// ============================================================================

static void test_empty_string_operations()
{
    qbs *empty = qbs_new_txt("");

    // Test operations on empty strings
    qbs *upper = qbs_ucase(empty);
    test_assert_ints(0, upper->len);
    qbs_free(upper);

    qbs *lower = qbs_lcase(empty);
    test_assert_ints(0, lower->len);
    qbs_free(lower);

    qbs *trimmed = qbs__trim(empty);
    test_assert_ints(0, trimmed->len);
    qbs_free(trimmed);

    qbs_free(empty);
}

static void test_very_long_string()
{
    // Test with a 1000-character string
    qbs *str = qbs_new(1000, 0);
    test_assert(str != NULL);
    test_assert_ints(1000, str->len);
    qbs_free(str);
}

// ============================================================================
// TEST RUNNER
// ============================================================================

struct unit_test qbs_tests[] = {
    // String creation
    {test_qbs_new_empty, "qbs_new - empty string"},
    {test_qbs_new_length, "qbs_new - with length"},
    {test_qbs_new_txt_basic, "qbs_new_txt - basic"},
    {test_qbs_new_txt_empty, "qbs_new_txt - empty"},
    {test_qbs_new_txt_len, "qbs_new_txt_len - truncated"},
    {test_qbs_new_txt_len_with_nulls, "qbs_new_txt_len - with null bytes"},

    // Concatenation and assignment
    {test_qbs_add_basic, "qbs_add - basic concatenation"},
    {test_qbs_add_empty_strings, "qbs_add - with empty string"},
    {test_qbs_set_basic, "qbs_set - basic assignment"},
    {test_qbs_set_longer, "qbs_set - expand string"},

    // Case conversion
    {test_qbs_ucase_basic, "qbs_ucase - basic"},
    {test_qbs_ucase_mixed, "qbs_ucase - mixed case"},
    {test_qbs_lcase_basic, "qbs_lcase - basic"},

    // Substrings
    {test_qbs_left_basic, "qbs_left - basic"},
    {test_qbs_left_full_string, "qbs_left - full string"},
    {test_qbs_right_basic, "qbs_right - basic"},
    {test_func_mid_basic, "func_mid - basic"},
    {test_func_mid_to_end, "func_mid - to end"},

    // Trimming
    {test_qbs_ltrim_spaces, "qbs_ltrim - spaces"},
    {test_qbs_ltrim_tabs, "qbs_ltrim - tabs"},
    {test_qbs_rtrim_spaces, "qbs_rtrim - spaces"},
    {test_qbs__trim_both, "qbs__trim - both sides"},

    // Numeric conversion
    {test_qbs_str_int32_positive, "qbs_str - int32 positive"},
    {test_qbs_str_int32_negative, "qbs_str - int32 negative"},
    {test_qbs_str_int64, "qbs_str - int64"},
    {test_qbs_str_uint64, "qbs_str - uint64"},
    {test_qbs_str_float, "qbs_str - float"},
    {test_qbs_str_double, "qbs_str - double"},
    {test_qbs_str_zero, "qbs_str - zero"},

    // String utilities
    {test_func_chr_basic, "func_chr - basic"},
    {test_func_chr_zero, "func_chr - null byte"},
    {test_func_space_basic, "func_space - basic"},
    {test_func_space_zero, "func_space - zero"},
    {test_func_string_basic, "func_string - basic"},
    {test_qbs_asc_basic, "qbs_asc - first char"},
    {test_qbs_asc_at_position, "qbs_asc - at position"},

    // Edge cases
    {test_empty_string_operations, "empty string operations"},
    {test_very_long_string, "very long string (1000 chars)"},
};

int main()
{
    return run_tests("qbs_comprehensive", qbs_tests, sizeof(qbs_tests) / sizeof(qbs_tests[0]));
}
