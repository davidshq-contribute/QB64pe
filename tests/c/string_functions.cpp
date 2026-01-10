// Unit tests for string_functions module
// Tests string manipulation functions like lset, rset, space, string, instr, mid, etc.

#include "test.h"
#include "qbs.h"
#include <string.h>

// String functions are declared in qbs.h, no need for forward declarations

static void test_sub_lset() {
    // Test left-justified string assignment
    qbs *dest = qbs_new(10, 1);
    qbs *source = qbs_new_txt("Hello");
    
    sub_lset(dest, source);
    
    test_assert_buffers("Hello     ", (const char *)dest->chr, 10);
    
    qbs_free(dest);
    qbs_free(source);
}

static void test_sub_rset() {
    // Test right-justified string assignment
    qbs *dest = qbs_new(10, 1);
    qbs *source = qbs_new_txt("Hello");
    
    sub_rset(dest, source);
    
    test_assert_buffers("     Hello", (const char *)dest->chr, 10);
    
    qbs_free(dest);
    qbs_free(source);
}

static void test_func_space() {
    // Test space string generation
    // Note: func_space returns a static pointer, but we free it here for test isolation
    qbs *spaces = func_space(5);
    test_assert(spaces != NULL);
    test_assert_ints(5, spaces->len);
    test_assert_buffers("     ", (const char *)spaces->chr, 5);
    qbs_free(spaces);
    
    // Test zero spaces
    qbs *zero = func_space(0);
    test_assert(zero != NULL);
    test_assert_ints(0, zero->len);
    qbs_free(zero);
    
    // Test negative spaces (should be treated as 0)
    qbs *neg = func_space(-5);
    test_assert(neg != NULL);
    test_assert_ints(0, neg->len);
    qbs_free(neg);
}

static void test_func_string() {
    // Test string generation with specific character
    // Note: func_string returns a static pointer, but we free it here for test isolation
    qbs *str1 = func_string(5, 65); // 5 'A' characters
    test_assert(str1 != NULL);
    test_assert_ints(5, str1->len);
    test_assert_buffers("AAAAA", (const char *)str1->chr, 5);
    qbs_free(str1);
    
    qbs *str2 = func_string(3, 48); // 3 '0' characters
    test_assert(str2 != NULL);
    test_assert_ints(3, str2->len);
    test_assert_buffers("000", (const char *)str2->chr, 3);
    qbs_free(str2);
}

static void test_func_instr() {
    // Test finding substring in string
    qbs *str = qbs_new_txt("Hello World");
    qbs *substr1 = qbs_new_txt("World");
    
    int32_t pos1 = func_instr(1, str, substr1, 1);
    test_assert_ints(7, pos1); // "World" starts at position 7
    
    qbs *substr2 = qbs_new_txt("llo");
    int32_t pos2 = func_instr(1, str, substr2, 1);
    test_assert_ints(3, pos2); // "llo" starts at position 3
    
    qbs *substr3 = qbs_new_txt("xyz");
    int32_t pos3 = func_instr(1, str, substr3, 1);
    test_assert_ints(0, pos3); // Not found
    
    qbs_free(str);
    qbs_free(substr1);
    qbs_free(substr2);
    qbs_free(substr3);
}

static void test_func__instrrev() {
    // Test reverse string search
    qbs *str = qbs_new_txt("Hello World Hello");
    qbs *substr = qbs_new_txt("Hello");
    
    int32_t pos = func__instrrev(1, str, substr, 1);
    // Should find the last occurrence
    test_assert(pos > 0);
    
    qbs_free(str);
    qbs_free(substr);
}

static void test_func_mid() {
    // Test extracting substring
    qbs *str = qbs_new_txt("Hello World");
    
    qbs *mid1 = func_mid(str, 1, 5, 2);
    test_assert(mid1 != NULL);
    test_assert_ints(5, mid1->len);
    test_assert_buffers("Hello", (const char *)mid1->chr, 5);
    qbs_free(mid1);
    
    qbs *mid2 = func_mid(str, 7, 5, 2);
    test_assert(mid2 != NULL);
    test_assert_ints(5, mid2->len);
    test_assert_buffers("World", (const char *)mid2->chr, 5);
    qbs_free(mid2);
    
    qbs_free(str);
}

static void test_sub_mid() {
    // Test modifying substring
    qbs *dest = qbs_new_txt("Hello World");
    qbs *src = qbs_new_txt("QB64");
    
    sub_mid(dest, 1, 4, src, 2);
    // Should replace first 4 characters with "QB64"
    test_assert_buffers("QB64o World", (const char *)dest->chr, 11);
    
    qbs_free(dest);
    qbs_free(src);
}

static void test_qbs_ltrim() {
    // Test left trim
    qbs *str1 = qbs_new_txt("   Hello");
    qbs *trimmed1 = qbs_ltrim(str1);
    test_assert(trimmed1 != NULL);
    test_assert_buffers("Hello", (const char *)trimmed1->chr, 5);
    qbs_free(str1);
    qbs_free(trimmed1);
    
    qbs *str2 = qbs_new_txt("Hello");
    qbs *trimmed2 = qbs_ltrim(str2);
    test_assert(trimmed2 != NULL);
    test_assert_buffers("Hello", (const char *)trimmed2->chr, 5);
    qbs_free(str2);
    qbs_free(trimmed2);
}

static void test_qbs_rtrim() {
    // Test right trim
    qbs *str1 = qbs_new_txt("Hello   ");
    qbs *trimmed1 = qbs_rtrim(str1);
    test_assert(trimmed1 != NULL);
    test_assert_buffers("Hello", (const char *)trimmed1->chr, 5);
    qbs_free(str1);
    qbs_free(trimmed1);
}

static void test_qbs__trim() {
    // Test trim both sides
    qbs *str1 = qbs_new_txt("   Hello   ");
    qbs *trimmed1 = qbs__trim(str1);
    test_assert(trimmed1 != NULL);
    test_assert_buffers("Hello", (const char *)trimmed1->chr, 5);
    qbs_free(str1);
    qbs_free(trimmed1);
}

int main() {
    struct unit_test tests[] = {
        {test_sub_lset, "sub_lset"},
        {test_sub_rset, "sub_rset"},
        {test_func_space, "func_space"},
        {test_func_string, "func_string"},
        {test_func_instr, "func_instr"},
        {test_func__instrrev, "func__instrrev"},
        {test_func_mid, "func_mid"},
        {test_sub_mid, "sub_mid"},
        {test_qbs_ltrim, "qbs_ltrim"},
        {test_qbs_rtrim, "qbs_rtrim"},
        {test_qbs__trim, "qbs__trim"},
    };
    
    return run_tests("string_functions", tests, sizeof(tests) / sizeof(tests[0]));
}
