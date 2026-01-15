//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Screen Management Module Unit Tests
//  Tests for screen management functions
//
//  Purpose:
//    This test suite verifies screen management and display control functions
//    in the QB64-PE screen module, including fullscreen control, window resizing,
//    and display updates.
//
//  Test Coverage:
//    - Fullscreen control (func__fullscreen, func__fullscreensmooth, sub__fullscreen)
//    - Window resizing (func__resize, func__resizewidth, func__resizeheight, sub__resize)
//    - Scaled dimensions (func__scaledwidth, func__scaledheight)
//    - Screen position (func__screenx, func__screeny)
//    - Display control (sub__display, sub__autodisplay)
//
//  Limitations:
//    Many screen functions require GUI/display initialization (FreeGLUT).
//    These tests focus on functions that can be tested with minimal setup or
//    verify function signatures and basic behavior without full initialization.
//    Some tests may be no-ops if the display is not initialized, but they should
//    not crash.
//
//  How to Run:
//    1. Build the test: make build-tests
//    2. Run individually: ./tests/exes/cpp/screen_test
//    3. Run all C++ tests: ./tests/run_c_tests.sh
//
//  Expected Behavior:
//    All tests should pass. Tests verify:
//    - Functions return valid values (even without display initialization)
//    - Setters can be called without crashing
//    - Getters return consistent, reasonable values
//    - Boolean-like values are 0 or 1
//    - Dimension values are non-negative
//
//----------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>

#include "test.h"

// Type definitions (matching os.h)
#define int32 int32_t

// Forward declarations for screen functions we're testing
// Note: These would normally come from screen.h, but we're testing in isolation
extern "C" {
    // Display control
    void sub__display();
    void sub__autodisplay();
    
    // Fullscreen control
    void sub__fullscreen(int32_t method, int32_t passed);
    int32_t func__fullscreen();
    int32_t func__fullscreensmooth();
    void sub__allowfullscreen(int32_t method, int32_t smooth);
    
    // Resize handling
    void sub__resize(int32_t on_off, int32_t stretch_smooth);
    int32_t func__resize();
    int32_t func__resizewidth();
    int32_t func__resizeheight();
    
    // Scaled dimensions
    int32_t func__scaledwidth();
    int32_t func__scaledheight();
    
    // Screen position
    int32_t func__screenx();
    int32_t func__screeny();
}

// Note: Many screen functions require GUI/display initialization.
// These tests focus on functions that can be tested with minimal setup
// or test the function signatures and basic behavior.

void test_fullscreen_getters() {
    // Test that fullscreen getters return valid values (even if display not initialized)
    // These should not crash and should return consistent values
    int32_t fullscreen = func__fullscreen();
    int32_t fullscreen_smooth = func__fullscreensmooth();
    
    // Values should be 0 or 1 (boolean-like)
    test_assert(fullscreen == 0 || fullscreen == 1);
    test_assert(fullscreen_smooth == 0 || fullscreen_smooth == 1);
}

void test_resize_getters() {
    // Test resize getters return valid values
    int32_t resize = func__resize();
    int32_t resize_width = func__resizewidth();
    int32_t resize_height = func__resizeheight();
    
    // These should return valid integer values (not crash)
    // resize should be 0 or 1 (boolean-like)
    test_assert(resize == 0 || resize == 1);
    
    // Width and height should be non-negative
    test_assert(resize_width >= 0);
    test_assert(resize_height >= 0);
}

void test_scaled_dimensions() {
    // Test scaled dimension getters
    int32_t scaled_width = func__scaledwidth();
    int32_t scaled_height = func__scaledheight();
    
    // Should return non-negative values
    test_assert(scaled_width >= 0);
    test_assert(scaled_height >= 0);
}

void test_screen_position() {
    // Test screen position getters
    int32_t screen_x = func__screenx();
    int32_t screen_y = func__screeny();
    
    // Should return valid integer values (may be negative if off-screen)
    // Just verify they don't crash and return reasonable values
    test_assert(screen_x >= -10000 && screen_x <= 10000);
    test_assert(screen_y >= -10000 && screen_y <= 10000);
}

void test_resize_setters() {
    // Test resize setters with various values
    // These should not crash even if display not initialized
    
    // Test enabling resize
    sub__resize(1, 0);
    int32_t resize_after_enable = func__resize();
    test_assert(resize_after_enable == 1);
    
    // Test disabling resize
    sub__resize(0, 0);
    int32_t resize_after_disable = func__resize();
    test_assert(resize_after_disable == 0);
}

void test_fullscreen_setters() {
    // Test fullscreen setters
    // These should not crash even if display not initialized
    
    // Test setting fullscreen mode
    sub__fullscreen(1, 1);
    int32_t fullscreen_after_set = func__fullscreen();
    // Note: May not actually change if display not initialized, but shouldn't crash
    // Verify it returns a valid value (0 or 1)
    test_assert(fullscreen_after_set == 0 || fullscreen_after_set == 1);
    
    // Test allowing fullscreen
    sub__allowfullscreen(1, 1);
    // Should not crash
}

void test_display_control() {
    // Test display control functions
    // These should not crash even if display not initialized
    
    // Test autodisplay
    sub__autodisplay();
    // Should not crash
    
    // Test manual display
    sub__display();
    // Should not crash (may be no-op if screen hidden)
}

int main() {
    struct unit_test tests[] = {
        { test_fullscreen_getters, "test-fullscreen-getters" },
        { test_resize_getters, "test-resize-getters" },
        { test_scaled_dimensions, "test-scaled-dimensions" },
        { test_screen_position, "test-screen-position" },
        { test_resize_setters, "test-resize-setters" },
        { test_fullscreen_setters, "test-fullscreen-setters" },
        { test_display_control, "test-display-control" },
    };

    return run_tests("screen", tests, sizeof(tests) / sizeof(*tests));
}
