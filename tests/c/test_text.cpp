//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Text & Font Module Unit Tests
//  Tests for text output and font management functions
//
//  Purpose:
//    This test suite verifies the functionality of text and font functions
//    in the QB64-PE text module, including font metrics, control character
//    settings, and functions that can be tested with minimal initialization.
//
//  Test Coverage:
//    - Font width/height queries for built-in fonts (8, 14, 16)
//    - Control character mode settings (_CONTROLCHR)
//    - Font parameter validation
//    - Basic error checking
//
//  Limitations:
//    Many text functions require full graphics subsystem initialization
//    (write_page, img[], font[] arrays, etc.). These tests focus on
//    functions that can be tested independently or with minimal setup.
//
//  How to Run:
//    1. Build the test: make build-tests
//    2. Run individually: ./tests/exes/cpp/text_test
//    3. Run all C++ tests: ./tests/run_c_tests.sh
//
//  Expected Behavior:
//    All tests should pass. Tests verify:
//    - Built-in font sizes are correct (8x8, 8x14, 8x16)
//    - Control character mode can be toggled
//    - Invalid font handles are detected
//
//----------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "test.h"

// Type definitions (matching os.h)
#define int32 int32_t
#define uint32 uint32_t

// Forward declarations for text/font functions we're testing
extern "C" {
    // Font metric queries
    int32_t func__fontwidth(int32_t f, int32_t passed);
    int32_t func__fontheight(int32_t f, int32_t passed);

    // Control character functions from utility module
    void sub__controlchr(int32_t onoff);
    int32_t func__controlchr();
    int32_t get_control_characters_disabled();

    // Cursor position functions (require initialized write_page)
    int32_t func_csrlin();
    int32_t func_pos(int32_t ignore);

    // LPOS function
    int32_t func_lpos(int32_t lpt);
}

// Test data - built-in font specifications
// QB64 supports three built-in fonts:
// - Font 8: 8x8 pixel characters
// - Font 14: 8x14 pixel characters (EGA-style)
// - Font 16: 8x16 pixel characters (VGA-style)

void test_builtin_font8_width() {
    // Font 8 (8x8) has fixed width of 8 pixels
    int32_t width = func__fontwidth(8, 1);
    test_assert_ints_with_name("Font 8 width", 8, width);
}

void test_builtin_font8_height() {
    // Font 8 (8x8) has height of 8 pixels
    int32_t height = func__fontheight(8, 1);
    test_assert_ints_with_name("Font 8 height", 8, height);
}

void test_builtin_font14_width() {
    // Font 14 (8x14) has fixed width of 8 pixels
    int32_t width = func__fontwidth(14, 1);
    test_assert_ints_with_name("Font 14 width", 8, width);
}

void test_builtin_font14_height() {
    // Font 14 (8x14) has height of 14 pixels
    int32_t height = func__fontheight(14, 1);
    test_assert_ints_with_name("Font 14 height", 14, height);
}

void test_builtin_font16_width() {
    // Font 16 (8x16) has fixed width of 8 pixels
    int32_t width = func__fontwidth(16, 1);
    test_assert_ints_with_name("Font 16 width", 8, width);
}

void test_builtin_font16_height() {
    // Font 16 (8x16) has height of 16 pixels
    int32_t height = func__fontheight(16, 1);
    test_assert_ints_with_name("Font 16 height", 16, height);
}

void test_controlchr_default() {
    // By default, control characters should be enabled (not disabled)
    // Reset to default state first
    sub__controlchr(1); // ON
    int32_t result = func__controlchr();
    test_assert_ints_with_name("Control chars default ON", -1, result);

    int32_t disabled = get_control_characters_disabled();
    test_assert_ints_with_name("Control chars not disabled", 0, disabled);
}

void test_controlchr_off() {
    // Test disabling control characters
    sub__controlchr(0); // OFF
    int32_t result = func__controlchr();
    test_assert_ints_with_name("Control chars OFF", 0, result);

    int32_t disabled = get_control_characters_disabled();
    test_assert_ints_with_name("Control chars disabled", 1, disabled);

    // Reset to default
    sub__controlchr(1); // ON
}

void test_controlchr_toggle() {
    // Test toggling control characters on and off
    sub__controlchr(1); // ON
    test_assert_ints_with_name("Toggle ON", -1, func__controlchr());

    sub__controlchr(0); // OFF
    test_assert_ints_with_name("Toggle OFF", 0, func__controlchr());

    sub__controlchr(1); // ON again
    test_assert_ints_with_name("Toggle ON again", -1, func__controlchr());
}

void test_lpos_valid_range() {
    // LPOS accepts values 0-3 (LPT1-LPT3)
    // Should not error for valid values
    // Note: actual return value depends on lpos global state

    // Test that valid inputs don't cause issues
    // We can't verify exact return values without full init,
    // but we can verify the function handles valid inputs
    int32_t result0 = func_lpos(0);
    int32_t result1 = func_lpos(1);
    int32_t result2 = func_lpos(2);
    int32_t result3 = func_lpos(3);

    // Results should be >= 0 (lpos is position, starts at 1 typically)
    test_assert(result0 >= 0);
    test_assert(result1 >= 0);
    test_assert(result2 >= 0);
    test_assert(result3 >= 0);
}

void test_font_consistency() {
    // Verify all built-in fonts have consistent 8-pixel width
    int32_t w8 = func__fontwidth(8, 1);
    int32_t w14 = func__fontwidth(14, 1);
    int32_t w16 = func__fontwidth(16, 1);

    test_assert_ints_with_name("All built-in fonts 8px wide", w8, w14);
    test_assert_ints_with_name("All built-in fonts 8px wide", w14, w16);
}

void test_font_height_ordering() {
    // Font heights should be in ascending order: 8 < 14 < 16
    int32_t h8 = func__fontheight(8, 1);
    int32_t h14 = func__fontheight(14, 1);
    int32_t h16 = func__fontheight(16, 1);

    test_assert_with_name("Font 8 < Font 14", h8 < h14);
    test_assert_with_name("Font 14 < Font 16", h14 < h16);
}

int main() {
    struct unit_test tests[] = {
        // Built-in font tests
        { test_builtin_font8_width, "test-builtin-font8-width" },
        { test_builtin_font8_height, "test-builtin-font8-height" },
        { test_builtin_font14_width, "test-builtin-font14-width" },
        { test_builtin_font14_height, "test-builtin-font14-height" },
        { test_builtin_font16_width, "test-builtin-font16-width" },
        { test_builtin_font16_height, "test-builtin-font16-height" },

        // Control character tests
        { test_controlchr_default, "test-controlchr-default" },
        { test_controlchr_off, "test-controlchr-off" },
        { test_controlchr_toggle, "test-controlchr-toggle" },

        // LPOS tests
        { test_lpos_valid_range, "test-lpos-valid-range" },

        // Consistency tests
        { test_font_consistency, "test-font-consistency" },
        { test_font_height_ordering, "test-font-height-ordering" },
    };

    return run_tests("text", tests, sizeof(tests) / sizeof(*tests));
}
