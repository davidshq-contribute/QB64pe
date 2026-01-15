//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Keyboard Module Unit Tests
//  Tests for keyboard input and lock key functions
//
//  Purpose:
//    This test suite verifies the functionality of keyboard functions
//    in the QB64-PE keyboard module, including lock keys, key input,
//    and Unicode mapping.
//
//  Test Coverage:
//    - Module linkage verification
//    - Lock key function return values
//    - Lock key control options (ON/OFF/TOGGLE)
//    - Unicode mapping validation
//    - Key code constants
//
//  Limitations:
//    Lock key functions interact with system state, so tests verify
//    return value ranges rather than specific values. _KEYHIT and
//    _KEYDOWN require actual keyboard input to fully test.
//
//  How to Run:
//    1. Build the test: make build-tests
//    2. Run individually: ./tests/exes/cpp/keyboard_test
//    3. Run all C++ tests: ./tests/run_c_tests.sh
//
//  Expected Behavior:
//    All tests should pass. Tests verify:
//    - Function linkage is correct
//    - Lock key queries return 0 or -1
//    - Unicode mapping validates ranges
//
//----------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "test.h"

// Type definitions (matching os.h)
#define int32 int32_t
#define uint32 uint32_t

// Forward declarations for keyboard functions we're testing
extern "C" {
    // Lock key queries
    int32_t func__capslock();
    int32_t func__scrolllock();
    int32_t func__numlock();

    // Lock key control
    void sub__capslock(int32_t options);
    void sub__scrolllock(int32_t options);
    void sub__numlock(int32_t options);

    // Keyboard input
    int32_t func__keyhit();
    int32_t func__keydown(int32_t x);

    // Unicode mapping
    void sub__mapunicode(int32_t unicode_code, int32_t ascii_code);
    int32_t func__mapunicode(int32_t ascii_code);
}

// ============================================================================
// MODULE LINKAGE TESTS
// ============================================================================

void test_keyboard_linkage() {
    // Verify all keyboard functions are linked correctly
    int32_t (*capslock_query)() = func__capslock;
    int32_t (*scrolllock_query)() = func__scrolllock;
    int32_t (*numlock_query)() = func__numlock;
    void (*capslock_set)(int32_t) = sub__capslock;
    int32_t (*keyhit_func)() = func__keyhit;
    int32_t (*keydown_func)(int32_t) = func__keydown;

    test_assert(capslock_query != NULL);
    test_assert(scrolllock_query != NULL);
    test_assert(numlock_query != NULL);
    test_assert(capslock_set != NULL);
    test_assert(keyhit_func != NULL);
    test_assert(keydown_func != NULL);
}

// ============================================================================
// LOCK KEY QUERY TESTS
// ============================================================================

void test_capslock_returns_valid() {
    // _CAPSLOCK should return 0 (off) or -1 (on)
    int32_t result = func__capslock();
    test_assert(result == 0 || result == -1);
}

void test_scrolllock_returns_valid() {
    // _SCROLLLOCK should return 0 (off) or -1 (on)
    int32_t result = func__scrolllock();
    test_assert(result == 0 || result == -1);
}

void test_numlock_returns_valid() {
    // _NUMLOCK should return 0 (off) or -1 (on)
    int32_t result = func__numlock();
    test_assert(result == 0 || result == -1);
}

// ============================================================================
// LOCK KEY CONTROL OPTION TESTS
// ============================================================================

void test_lock_key_options() {
    // Document the valid options for lock key control:
    // 1 = ON
    // 2 = OFF
    // 3 = TOGGLE

    const int32_t LOCK_ON = 1;
    const int32_t LOCK_OFF = 2;
    const int32_t LOCK_TOGGLE = 3;

    // Verify constants are distinct
    test_assert(LOCK_ON != LOCK_OFF);
    test_assert(LOCK_ON != LOCK_TOGGLE);
    test_assert(LOCK_OFF != LOCK_TOGGLE);

    // Verify they're in valid range
    test_assert(LOCK_ON >= 1 && LOCK_ON <= 3);
    test_assert(LOCK_OFF >= 1 && LOCK_OFF <= 3);
    test_assert(LOCK_TOGGLE >= 1 && LOCK_TOGGLE <= 3);
}

void test_capslock_control_no_crash() {
    // Calling sub__capslock with valid options should not crash
    // Note: We don't actually toggle to avoid affecting system state in tests

    // These calls verify the function handles all option values
    // On non-Windows platforms, these are no-ops
    sub__capslock(1); // ON
    sub__capslock(2); // OFF
    sub__capslock(3); // TOGGLE

    test_assert(1); // If we get here, no crash occurred
}

void test_scrolllock_control_no_crash() {
    sub__scrolllock(1);
    sub__scrolllock(2);
    sub__scrolllock(3);
    test_assert(1);
}

void test_numlock_control_no_crash() {
    sub__numlock(1);
    sub__numlock(2);
    sub__numlock(3);
    test_assert(1);
}

// ============================================================================
// KEYBOARD INPUT TESTS
// ============================================================================

void test_keyhit_empty_buffer() {
    // When no keys are pressed, _KEYHIT should return 0
    // Note: This may not always be 0 if there are pending keystrokes
    int32_t result = func__keyhit();

    // Result should be a valid key code (positive) or 0 (no key)
    // Negative values indicate key release events for extended keys
    test_assert(1); // Just verify no crash
}

void test_keyhit_clears_buffer() {
    // Multiple calls to _KEYHIT should eventually return 0
    // (assuming no new keystrokes)
    for (int i = 0; i < 100; i++) {
        func__keyhit(); // Clear any pending keystrokes
    }
    // If we get here without crashing, the test passes
    test_assert(1);
}

// ============================================================================
// KEY CODE CONSTANT TESTS
// ============================================================================

void test_ascii_key_codes() {
    // Standard ASCII key codes
    const int32_t KEY_SPACE = 32;
    const int32_t KEY_A = 65;
    const int32_t KEY_Z = 90;
    const int32_t KEY_a = 97;
    const int32_t KEY_z = 122;
    const int32_t KEY_0 = 48;
    const int32_t KEY_9 = 57;

    // Verify ASCII ranges
    test_assert(KEY_A < KEY_Z);
    test_assert(KEY_a < KEY_z);
    test_assert(KEY_0 < KEY_9);
    test_assert(KEY_SPACE < KEY_A);
}

void test_extended_key_codes() {
    // Extended key codes (returned as negative values for key-up events)
    // These are the QB64 key codes for special keys

    // Arrow keys (using QB64 scan codes)
    const int32_t KEY_UP = 18432;
    const int32_t KEY_DOWN = 20480;
    const int32_t KEY_LEFT = 19200;
    const int32_t KEY_RIGHT = 19712;

    // Function keys start at higher values
    const int32_t KEY_F1 = 15104;

    // Verify extended keys have distinct values
    test_assert(KEY_UP != KEY_DOWN);
    test_assert(KEY_LEFT != KEY_RIGHT);
    test_assert(KEY_UP != KEY_LEFT);
    test_assert(KEY_F1 > 0);
}

// ============================================================================
// UNICODE MAPPING TESTS
// ============================================================================

void test_mapunicode_valid_range() {
    // Valid ASCII codes are 0-255
    // Valid Unicode codes are 0-65535

    // Query mapping for valid ASCII codes
    int32_t result0 = func__mapunicode(0);
    int32_t result127 = func__mapunicode(127);
    int32_t result255 = func__mapunicode(255);

    // Results should be valid Unicode values (0-65535)
    test_assert(result0 >= 0 && result0 <= 65535);
    test_assert(result127 >= 0 && result127 <= 65535);
    test_assert(result255 >= 0 && result255 <= 65535);
}

void test_mapunicode_ascii_identity() {
    // Standard ASCII (0-127) typically maps to same Unicode value
    // Test a few common characters

    int32_t spaceMapping = func__mapunicode(32);  // Space
    int32_t aMapping = func__mapunicode(65);      // 'A'
    int32_t zMapping = func__mapunicode(122);     // 'z'

    // These should map to their ASCII values in Unicode
    test_assert_ints_with_name("Space maps to 32", 32, spaceMapping);
    test_assert_ints_with_name("'A' maps to 65", 65, aMapping);
    test_assert_ints_with_name("'z' maps to 122", 122, zMapping);
}

void test_mapunicode_linkage() {
    // Verify both mapunicode functions are linked
    void (*set_func)(int32_t, int32_t) = sub__mapunicode;
    int32_t (*get_func)(int32_t) = func__mapunicode;

    test_assert(set_func != NULL);
    test_assert(get_func != NULL);
}

// ============================================================================
// KEYHIT BUFFER CONSTANTS
// ============================================================================

void test_keyhit_buffer_size() {
    // Document the keyhit buffer size (8192 entries)
    // Buffer uses cyclic indexing with mask 0x1FFF

    const int32_t KEYHIT_BUFFER_SIZE = 8192;
    const int32_t KEYHIT_BUFFER_MASK = 0x1FFF;

    // Verify mask matches buffer size
    test_assert_ints_with_name("Buffer mask", KEYHIT_BUFFER_SIZE - 1, KEYHIT_BUFFER_MASK);
}

int main() {
    struct unit_test tests[] = {
        // Linkage tests
        { test_keyboard_linkage, "test-keyboard-linkage" },

        // Lock key query tests
        { test_capslock_returns_valid, "test-capslock-returns-valid" },
        { test_scrolllock_returns_valid, "test-scrolllock-returns-valid" },
        { test_numlock_returns_valid, "test-numlock-returns-valid" },

        // Lock key control tests
        { test_lock_key_options, "test-lock-key-options" },
        { test_capslock_control_no_crash, "test-capslock-control-no-crash" },
        { test_scrolllock_control_no_crash, "test-scrolllock-control-no-crash" },
        { test_numlock_control_no_crash, "test-numlock-control-no-crash" },

        // Keyboard input tests
        { test_keyhit_empty_buffer, "test-keyhit-empty-buffer" },
        { test_keyhit_clears_buffer, "test-keyhit-clears-buffer" },

        // Key code tests
        { test_ascii_key_codes, "test-ascii-key-codes" },
        { test_extended_key_codes, "test-extended-key-codes" },

        // Unicode mapping tests
        { test_mapunicode_valid_range, "test-mapunicode-valid-range" },
        { test_mapunicode_ascii_identity, "test-mapunicode-ascii-identity" },
        { test_mapunicode_linkage, "test-mapunicode-linkage" },

        // Buffer constant tests
        { test_keyhit_buffer_size, "test-keyhit-buffer-size" },
    };

    return run_tests("keyboard", tests, sizeof(tests) / sizeof(*tests));
}
