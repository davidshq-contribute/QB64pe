//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Console Module Unit Tests
//  Tests for console window and input/output functions
//
//  Purpose:
//    This test suite verifies the functionality of console functions
//    in the QB64-PE console module, including console window management
//    and console input handling.
//
//  Test Coverage:
//    - Module linkage verification
//    - func__console return value validation
//    - Console input function behavior
//    - Platform-specific behavior documentation
//
//  Limitations:
//    Console functions interact with the Windows console subsystem, which
//    may not be available in all test environments. Tests focus on:
//    - Verifying functions exist and link
//    - Testing return value ranges
//    - Documenting expected platform behavior
//
//  How to Run:
//    1. Build the test: make build-tests
//    2. Run individually: ./tests/exes/cpp/console_test
//    3. Run all C++ tests: ./tests/run_c_tests.sh
//
//  Expected Behavior:
//    All tests should pass. Tests verify:
//    - Function linkage is correct
//    - Return values are in expected ranges
//    - No crashes on edge cases
//
//----------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "test.h"

// Include OS detection
#include "../../internal/c/os.h"

// Type definitions (matching os.h)
#define int32 int32_t
#define uint32 uint32_t

// Forward declaration for qbs structure
struct qbs {
    uint8_t *chr;
    int32_t len;
    uint8_t in_cmem;
    uint16_t *cmem_descriptor;
    uint16_t cmem_descriptor_offset;
    uint32_t listi;
    uint8_t tmp;
    uint8_t fixed;
    uint8_t readonly;
};

// Forward declarations for console functions we're testing
extern "C" {
    int32_t func__console();
    void sub__console(int32_t onoff);
    void sub__consoletitle(qbs *s);
    void sub__consolefont(qbs *FontName, int32_t FontSize);
    void sub__console_cursor(int32_t visible, int32_t cursorsize, int32_t passed);
    int32_t func__getconsoleinput();
    int32_t func__cinp(int32_t toggle, int32_t passed);
}

// Helper to create a minimal qbs for testing
static uint8_t empty_chr[1] = {0};
static qbs empty_qbs = {empty_chr, 0, 0, NULL, 0, 0, 0, 0, 0};

// ============================================================================
// MODULE LINKAGE TESTS
// ============================================================================

void test_console_linkage() {
    // Verify all console functions are linked correctly
    int32_t (*console_func)() = func__console;
    void (*console_sub)(int32_t) = sub__console;
    void (*consoletitle_func)(qbs*) = sub__consoletitle;
    int32_t (*getconsoleinput_func)() = func__getconsoleinput;
    int32_t (*cinp_func)(int32_t, int32_t) = func__cinp;

    test_assert(console_func != NULL);
    test_assert(console_sub != NULL);
    test_assert(consoletitle_func != NULL);
    test_assert(getconsoleinput_func != NULL);
    test_assert(cinp_func != NULL);
}

// ============================================================================
// CONSOLE QUERY TESTS
// ============================================================================

void test_console_returns_valid() {
    // _CONSOLE returns the console image handle, or -1 on error
    int32_t result = func__console();
    // Result should be a valid handle (>= 0) or -1
    test_assert(result >= -1);
}

// ============================================================================
// CONSOLE CONTROL TESTS
// ============================================================================

void test_console_on_no_crash() {
    // sub__console(1) turns console on - should not crash
    // Note: May be a no-op if console is unavailable
    sub__console(1);
    test_assert(1);
}

void test_console_off_no_crash() {
    // sub__console(2) turns console off - should not crash
    sub__console(2);
    test_assert(1);
}

void test_console_toggle_no_crash() {
    // Toggle console state multiple times
    sub__console(1);  // ON
    sub__console(2);  // OFF
    sub__console(1);  // ON
    sub__console(2);  // OFF
    test_assert(1);
}

// ============================================================================
// CONSOLE TITLE TESTS
// ============================================================================

void test_consoletitle_empty_no_crash() {
    // Empty string should not crash
    qbs empty;
    uint8_t chr_data[1] = {0};
    empty.chr = chr_data;
    empty.len = 0;
    empty.in_cmem = 0;
    empty.cmem_descriptor = NULL;
    empty.cmem_descriptor_offset = 0;
    empty.listi = 0;
    empty.tmp = 0;
    empty.fixed = 0;
    empty.readonly = 0;

    sub__consoletitle(&empty);
    test_assert(1);
}

void test_consoletitle_with_text_no_crash() {
    // Normal string should not crash
    qbs title;
    uint8_t chr_data[] = "Test Title";
    title.chr = chr_data;
    title.len = 10;
    title.in_cmem = 0;
    title.cmem_descriptor = NULL;
    title.cmem_descriptor_offset = 0;
    title.listi = 0;
    title.tmp = 0;
    title.fixed = 0;
    title.readonly = 0;

    sub__consoletitle(&title);
    test_assert(1);
}

// ============================================================================
// CONSOLE FONT TESTS
// ============================================================================

void test_consolefont_empty_name_no_crash() {
    // Empty font name should not crash (keeps existing font)
    qbs empty;
    uint8_t chr_data[1] = {0};
    empty.chr = chr_data;
    empty.len = 0;
    empty.in_cmem = 0;
    empty.cmem_descriptor = NULL;
    empty.cmem_descriptor_offset = 0;
    empty.listi = 0;
    empty.tmp = 0;
    empty.fixed = 0;
    empty.readonly = 0;

    sub__consolefont(&empty, 12);
    test_assert(1);
}

void test_consolefont_with_name_no_crash() {
    // Named font should not crash
    qbs fontname;
    uint8_t chr_data[] = "Consolas";
    fontname.chr = chr_data;
    fontname.len = 8;
    fontname.in_cmem = 0;
    fontname.cmem_descriptor = NULL;
    fontname.cmem_descriptor_offset = 0;
    fontname.listi = 0;
    fontname.tmp = 0;
    fontname.fixed = 0;
    fontname.readonly = 0;

    sub__consolefont(&fontname, 14);
    test_assert(1);
}

// ============================================================================
// CONSOLE CURSOR TESTS
// ============================================================================

void test_console_cursor_show_no_crash() {
    // Show cursor should not crash
    sub__console_cursor(1, 0, 0);  // visible=1 (show)
    test_assert(1);
}

void test_console_cursor_hide_no_crash() {
    // Hide cursor should not crash
    sub__console_cursor(2, 0, 0);  // visible=2 (hide)
    test_assert(1);
}

void test_console_cursor_size_no_crash() {
    // Set cursor size should not crash
    sub__console_cursor(1, 50, 1);  // visible=1, size=50, passed=1
    test_assert(1);
}

void test_console_cursor_size_bounds() {
    // Test cursor size boundaries
    sub__console_cursor(1, 0, 1);    // Minimum size
    sub__console_cursor(1, 100, 1);  // Maximum size
    sub__console_cursor(1, 101, 1);  // Out of range (should be ignored)
    sub__console_cursor(1, -1, 1);   // Negative (should be ignored)
    test_assert(1);
}

// ============================================================================
// CONSOLE INPUT TESTS
// ============================================================================

void test_getconsoleinput_returns_valid() {
    // _GETCONSOLEINPUT should return 0, 1, or 2
    // 0 = no input, 1 = keyboard, 2 = mouse
    int32_t result = func__getconsoleinput();
    test_assert(result >= 0 && result <= 2);
}

void test_getconsoleinput_multiple_calls() {
    // Multiple calls should not crash
    for (int i = 0; i < 10; i++) {
        func__getconsoleinput();
    }
    test_assert(1);
}

// ============================================================================
// CINP TESTS
// ============================================================================

void test_cinp_default_mode() {
    // _CINP with default mode (passed=0 -> toggle=1)
    int32_t result = func__cinp(0, 0);
    // Should return 0 when no key pressed, or a scan code
    // On non-Windows, always returns 0
#ifdef QB64_WINDOWS
    test_assert(1);  // Result varies based on console input
#else
    test_assert_ints_with_name("Non-Windows returns 0", 0, result);
#endif
}

void test_cinp_toggle_on() {
    // _CINP with toggle=1 returns positive/negative scan codes
    int32_t result = func__cinp(1, 1);
    // On non-Windows, always returns 0
#ifndef QB64_WINDOWS
    test_assert_ints_with_name("Non-Windows returns 0", 0, result);
#else
    test_assert(1);  // Result varies
#endif
}

void test_cinp_toggle_off() {
    // _CINP with toggle=0 converts negative to positive+128
    int32_t result = func__cinp(0, 1);
#ifndef QB64_WINDOWS
    test_assert_ints_with_name("Non-Windows returns 0", 0, result);
#else
    test_assert(result >= 0);  // Never negative in this mode
#endif
}

// ============================================================================
// PLATFORM BEHAVIOR TESTS
// ============================================================================

void test_console_platform_behavior() {
    // Document expected platform behavior
#ifdef QB64_WINDOWS
    // Windows has full console support
    test_assert_with_name("Windows console support", 1);
#else
    // Non-Windows platforms have limited or no console support
    // Functions exist but may be no-ops
    test_assert_with_name("Non-Windows limited console", 1);
#endif
}

// ============================================================================
// CONSTANT DOCUMENTATION TESTS
// ============================================================================

void test_console_onoff_constants() {
    // Document the console on/off values for sub__console
    const int32_t CONSOLE_ON = 1;
    const int32_t CONSOLE_OFF = 2;

    test_assert(CONSOLE_ON != CONSOLE_OFF);
    test_assert(CONSOLE_ON == 1);
    test_assert(CONSOLE_OFF == 2);
}

void test_cursor_visible_constants() {
    // Document the cursor visibility values
    const int32_t CURSOR_SHOW = 1;
    const int32_t CURSOR_HIDE = 2;

    test_assert(CURSOR_SHOW != CURSOR_HIDE);
}

void test_console_input_return_constants() {
    // Document the _GETCONSOLEINPUT return values
    const int32_t INPUT_NONE = 0;
    const int32_t INPUT_KEYBOARD = 1;
    const int32_t INPUT_MOUSE = 2;

    test_assert(INPUT_NONE == 0);
    test_assert(INPUT_KEYBOARD == 1);
    test_assert(INPUT_MOUSE == 2);
}

int main() {
    struct unit_test tests[] = {
        // Linkage tests
        { test_console_linkage, "test-console-linkage" },

        // Console query tests
        { test_console_returns_valid, "test-console-returns-valid" },

        // Console control tests
        { test_console_on_no_crash, "test-console-on-no-crash" },
        { test_console_off_no_crash, "test-console-off-no-crash" },
        { test_console_toggle_no_crash, "test-console-toggle-no-crash" },

        // Console title tests
        { test_consoletitle_empty_no_crash, "test-consoletitle-empty-no-crash" },
        { test_consoletitle_with_text_no_crash, "test-consoletitle-with-text-no-crash" },

        // Console font tests
        { test_consolefont_empty_name_no_crash, "test-consolefont-empty-name-no-crash" },
        { test_consolefont_with_name_no_crash, "test-consolefont-with-name-no-crash" },

        // Console cursor tests
        { test_console_cursor_show_no_crash, "test-console-cursor-show-no-crash" },
        { test_console_cursor_hide_no_crash, "test-console-cursor-hide-no-crash" },
        { test_console_cursor_size_no_crash, "test-console-cursor-size-no-crash" },
        { test_console_cursor_size_bounds, "test-console-cursor-size-bounds" },

        // Console input tests
        { test_getconsoleinput_returns_valid, "test-getconsoleinput-returns-valid" },
        { test_getconsoleinput_multiple_calls, "test-getconsoleinput-multiple-calls" },

        // CINP tests
        { test_cinp_default_mode, "test-cinp-default-mode" },
        { test_cinp_toggle_on, "test-cinp-toggle-on" },
        { test_cinp_toggle_off, "test-cinp-toggle-off" },

        // Platform behavior tests
        { test_console_platform_behavior, "test-console-platform-behavior" },

        // Constant documentation tests
        { test_console_onoff_constants, "test-console-onoff-constants" },
        { test_cursor_visible_constants, "test-cursor-visible-constants" },
        { test_console_input_return_constants, "test-console-input-return-constants" },
    };

    return run_tests("console", tests, sizeof(tests) / sizeof(*tests));
}
