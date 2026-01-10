// Unit tests for hexoctbin module
// Tests hexadecimal, octal, and binary number conversion functions

#include "test.h"
#include "hexoctbin.h"
#include <string.h>

static void test_func__bin() {
    // Test binary conversion for integers
    qbs *bin1 = func__bin(0, 0);
    test_assert(bin1 != NULL);
    test_assert(bin1->len > 0);
    test_assert_buffers("0", (const char *)bin1->chr, 1);
    qbs_free(bin1);
    
    qbs *bin2 = func__bin(5, 0);
    test_assert(bin2 != NULL);
    test_assert(bin2->len > 0);
    test_assert_buffers("101", (const char *)bin2->chr, 3);
    qbs_free(bin2);
    
    qbs *bin3 = func__bin(255, 0);
    test_assert(bin3 != NULL);
    test_assert(bin3->len > 0);
    qbs_free(bin3);
}

static void test_func__bin_float() {
    // Test binary conversion for floating point
    qbs *bin = func__bin_float(3.14);
    test_assert(bin != NULL);
    test_assert(bin->len > 0);
    qbs_free(bin);
}

static void test_func_oct() {
    // Test octal conversion for integers
    qbs *oct1 = func_oct(0, 0);
    test_assert(oct1 != NULL);
    test_assert(oct1->len > 0);
    test_assert_buffers("0", (const char *)oct1->chr, 1);
    qbs_free(oct1);
    
    qbs *oct2 = func_oct(8, 0);
    test_assert(oct2 != NULL);
    test_assert(oct2->len > 0);
    test_assert_buffers("10", (const char *)oct2->chr, 2);
    qbs_free(oct2);
    
    qbs *oct3 = func_oct(64, 0);
    test_assert(oct3 != NULL);
    test_assert(oct3->len > 0);
    test_assert_buffers("100", (const char *)oct3->chr, 3);
    qbs_free(oct3);
}

static void test_func_oct_float() {
    // Test octal conversion for floating point
    qbs *oct = func_oct_float(3.14);
    test_assert(oct != NULL);
    test_assert(oct->len > 0);
    qbs_free(oct);
}

static void test_func_hex() {
    // Test hexadecimal conversion for integers
    qbs *hex1 = func_hex(0, 0);
    test_assert(hex1 != NULL);
    test_assert(hex1->len > 0);
    test_assert_buffers("0", (const char *)hex1->chr, 1);
    qbs_free(hex1);
    
    qbs *hex2 = func_hex(10, 0);
    test_assert(hex2 != NULL);
    test_assert(hex2->len > 0);
    test_assert_buffers("A", (const char *)hex2->chr, 1);
    qbs_free(hex2);
    
    qbs *hex3 = func_hex(255, 0);
    test_assert(hex3 != NULL);
    test_assert(hex3->len > 0);
    test_assert_buffers("FF", (const char *)hex3->chr, 2);
    qbs_free(hex3);
    
    qbs *hex4 = func_hex(16, 0);
    test_assert(hex4 != NULL);
    test_assert(hex4->len > 0);
    test_assert_buffers("10", (const char *)hex4->chr, 2);
    qbs_free(hex4);
}

static void test_func_hex_float() {
    // Test hexadecimal conversion for floating point
    qbs *hex = func_hex_float(3.14);
    test_assert(hex != NULL);
    test_assert(hex->len > 0);
    qbs_free(hex);
}

static void test_negative_values() {
    // Test negative value conversions
    qbs *bin_neg = func__bin(-5, 8);
    test_assert(bin_neg != NULL);
    test_assert(bin_neg->len > 0);
    qbs_free(bin_neg);
    
    qbs *oct_neg = func_oct(-8, 8);
    test_assert(oct_neg != NULL);
    test_assert(oct_neg->len > 0);
    qbs_free(oct_neg);
    
    qbs *hex_neg = func_hex(-16, 8);
    test_assert(hex_neg != NULL);
    test_assert(hex_neg->len > 0);
    qbs_free(hex_neg);
}

int main() {
    struct unit_test tests[] = {
        {test_func__bin, "func__bin"},
        {test_func__bin_float, "func__bin_float"},
        {test_func_oct, "func_oct"},
        {test_func_oct_float, "func_oct_float"},
        {test_func_hex, "func_hex"},
        {test_func_hex_float, "func_hex_float"},
        {test_negative_values, "negative_values"},
    };
    
    return run_tests("hexoctbin", tests, sizeof(tests) / sizeof(tests[0]));
}
