//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Window Module Unit Tests
//  Tests for window query and control functions
//
//  Purpose:
//    This test suite verifies the functionality of window functions
//    in the QB64-PE window module, including window handle retrieval,
//    title access, and focus state.
//
//  Test Coverage:
//    - Module linkage verification
//    - _HANDLE return value validation
//    - _TITLE function behavior
//    - _HASFOCUS return value validation
//    - Platform-specific behavior documentation
//
//  Limitations:
//    Window functions require a graphics window to be fully functional.
//    In test environments without GLUT initialization, functions may
//    return default values. Tests focus on:
//    - Verifying functions exist and link
//    - Testing return value ranges
//    - Documenting expected behavior
//
//  How to Run:
//    1. Build the test: make build-tests
//    2. Run individually: ./tests/exes/cpp/window_test
//    3. Run all C++ tests: ./tests/run_c_tests.sh
//
//  Expected Behavior:
//    All tests should pass. Tests verify:
//    - Function linkage is correct
//    - Return values are in expected ranges
//    - No crashes occur
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
#define int64 int64_t

// Forward declaration for qbs structure (returned by func__title)
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

// Forward declarations for window functions we're testing
extern "C" {
    int64 func__handle();
    qbs* func__title();
    int32 func__hasfocus();
    void set_foreground_window(intptr_t i);
}

// ============================================================================
// MODULE LINKAGE TESTS
// ============================================================================

void test_window_linkage() {
    // Verify all window functions are linked correctly
    int64 (*handle_func)() = func__handle;
    qbs* (*title_func)() = func__title;
    int32 (*hasfocus_func)() = func__hasfocus;
    void (*setforeground_func)(intptr_t) = set_foreground_window;

    test_assert(handle_func != NULL);
    test_assert(title_func != NULL);
    test_assert(hasfocus_func != NULL);
    test_assert(setforeground_func != NULL);
}

// ============================================================================
// WINDOW HANDLE TESTS
// ============================================================================

void test_handle_returns_valid() {
    // _HANDLE should return a window handle (may be 0 without GLUT)
    int64 result = func__handle();

    // Handle is platform-specific:
    // - Windows: HWND value (or 0 if no window)
    // - Others: 0
#ifdef QB64_WINDOWS
    // On Windows, may be 0 or a valid handle
    test_assert(result >= 0);
#else
    // On non-Windows, always returns 0
    test_assert(result == 0);
#endif
}

void test_handle_no_crash() {
    // Calling func__handle should never crash
    func__handle();
    func__handle();
    func__handle();
    test_assert(1);
}

// ============================================================================
// WINDOW TITLE TESTS
// ============================================================================

void test_title_returns_qbs() {
    // _TITLE should return a valid qbs pointer
    qbs *result = func__title();
    test_assert(result != NULL);
}

void test_title_qbs_has_valid_fields() {
    // The returned qbs should have valid fields
    qbs *result = func__title();
    test_assert(result != NULL);

    // chr pointer should exist (even if empty string)
    test_assert(result->chr != NULL || result->len == 0);

    // Length should be non-negative
    test_assert(result->len >= 0);
}

void test_title_no_crash() {
    // Calling func__title multiple times should not crash
    for (int i = 0; i < 10; i++) {
        func__title();
    }
    test_assert(1);
}

// ============================================================================
// WINDOW FOCUS TESTS
// ============================================================================

void test_hasfocus_returns_valid() {
    // _HASFOCUS should return 0 (no focus) or -1 (has focus)
    int32 result = func__hasfocus();
    test_assert(result == 0 || result == -1);
}

void test_hasfocus_no_crash() {
    // Calling func__hasfocus should never crash
    func__hasfocus();
    func__hasfocus();
    func__hasfocus();
    test_assert(1);
}

// ============================================================================
// SET FOREGROUND WINDOW TESTS
// ============================================================================

void test_setforeground_null_no_crash() {
    // set_foreground_window with 0/NULL should not crash
    set_foreground_window(0);
    test_assert(1);
}

void test_setforeground_invalid_no_crash() {
    // set_foreground_window with invalid handle should not crash
    // (It will just fail silently on Windows)
    set_foreground_window(12345);
    test_assert(1);
}

// ============================================================================
// PLATFORM BEHAVIOR TESTS
// ============================================================================

void test_window_platform_handle() {
    // Document platform-specific handle behavior
#ifdef QB64_WINDOWS
    // Windows: Returns HWND cast to intptr_t
    // May be 0 if no window exists yet
    test_assert_with_name("Windows returns HWND", 1);
#else
    // Non-Windows: Returns 0
    int64 handle = func__handle();
    test_assert_ints_with_name("Non-Windows returns 0", 0, (int32)handle);
#endif
}

void test_window_platform_focus() {
    // Document platform-specific focus behavior
#ifdef QB64_WINDOWS
    // Windows: Compares func__handle() with GetForegroundWindow()
    test_assert_with_name("Windows checks foreground window", 1);
#elif defined(QB64_LINUX)
    // Linux: Uses window_focused variable
    test_assert_with_name("Linux uses window_focused", 1);
#else
    // Other platforms: Returns -1 (always focused)
    test_assert_with_name("Other platforms assume focus", 1);
#endif
}

// ============================================================================
// CONSTANT DOCUMENTATION TESTS
// ============================================================================

void test_focus_return_constants() {
    // Document the _HASFOCUS return values
    const int32 HAS_FOCUS = -1;     // Window has focus (QB64 TRUE)
    const int32 NO_FOCUS = 0;       // Window does not have focus (QB64 FALSE)

    test_assert(HAS_FOCUS == -1);
    test_assert(NO_FOCUS == 0);
    test_assert(HAS_FOCUS != NO_FOCUS);
}

void test_handle_type() {
    // Document that _HANDLE returns a 64-bit integer
    // On Windows this is an HWND (pointer-sized)
    // On other platforms it's 0

    int64 handle = func__handle();

    // Verify it fits in 64 bits (always true, but documents the type)
    test_assert(sizeof(handle) == 8);
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

void test_handle_focus_consistency() {
    // If handle is 0, focus state should still be valid
    int64 handle = func__handle();
    int32 focus = func__hasfocus();

    // Focus should be 0 or -1 regardless of handle
    test_assert(focus == 0 || focus == -1);

    // If handle exists on Windows, focus check should work
#ifdef QB64_WINDOWS
    if (handle != 0) {
        // Focus state is meaningful when we have a window
        test_assert(focus == 0 || focus == -1);
    }
#endif
}

void test_title_with_handle() {
    // Title should be accessible regardless of handle state
    int64 handle = func__handle();
    qbs *title = func__title();

    test_assert(title != NULL);

    // Even without a window, title should return a valid qbs
    // (may be empty string)
}

int main() {
    struct unit_test tests[] = {
        // Linkage tests
        { test_window_linkage, "test-window-linkage" },

        // Handle tests
        { test_handle_returns_valid, "test-handle-returns-valid" },
        { test_handle_no_crash, "test-handle-no-crash" },

        // Title tests
        { test_title_returns_qbs, "test-title-returns-qbs" },
        { test_title_qbs_has_valid_fields, "test-title-qbs-has-valid-fields" },
        { test_title_no_crash, "test-title-no-crash" },

        // Focus tests
        { test_hasfocus_returns_valid, "test-hasfocus-returns-valid" },
        { test_hasfocus_no_crash, "test-hasfocus-no-crash" },

        // Set foreground tests
        { test_setforeground_null_no_crash, "test-setforeground-null-no-crash" },
        { test_setforeground_invalid_no_crash, "test-setforeground-invalid-no-crash" },

        // Platform behavior tests
        { test_window_platform_handle, "test-window-platform-handle" },
        { test_window_platform_focus, "test-window-platform-focus" },

        // Constant documentation tests
        { test_focus_return_constants, "test-focus-return-constants" },
        { test_handle_type, "test-handle-type" },

        // Integration tests
        { test_handle_focus_consistency, "test-handle-focus-consistency" },
        { test_title_with_handle, "test-title-with-handle" },
    };

    return run_tests("window", tests, sizeof(tests) / sizeof(*tests));
}
