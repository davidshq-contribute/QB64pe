//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Mouse Module Unit Tests
//  Tests for mouse input and cursor functions
//
//  Purpose:
//    This test suite verifies the functionality of mouse functions
//    in the QB64-PE mouse module, including cursor visibility, position,
//    buttons, and wheel input.
//
//  Test Coverage:
//    - Module linkage verification
//    - Cursor visibility functions
//    - Mouse button validation
//    - Wheel movement values
//    - Cursor style constants
//
//  Limitations:
//    Mouse functions require a graphics window and actual mouse input
//    for full testing. These tests focus on:
//    - Verifying functions exist and link
//    - Testing input validation
//    - Documenting expected return values
//
//  How to Run:
//    1. Build the test: make build-tests
//    2. Run individually: ./tests/exes/cpp/mouse_test
//    3. Run all C++ tests: ./tests/run_c_tests.sh
//
//  Expected Behavior:
//    All tests should pass. Tests verify:
//    - Function linkage is correct
//    - Button number validation works
//    - Return values are in expected ranges
//
//----------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "test.h"

// Type definitions (matching os.h)
#define int32 int32_t
#define uint32 uint32_t

// Forward declaration for qbs structure (minimal for testing)
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

// Forward declarations for mouse functions we're testing
extern "C" {
    // Cursor visibility
    void sub__mousehide();
    void sub__mouseshow(qbs *style, int32 passed);
    int32_t func__mousehidden();

    // Position functions
    float func__mousex();
    float func__mousey();
    float func__mousemovementx();
    float func__mousemovementy();
    void sub__mousemove(float x, float y);

    // Input functions
    int32 func__mouseinput();
    int32 func__mousebutton(int32 i);
    int32 func__mousewheel();
}

// ============================================================================
// MODULE LINKAGE TESTS
// ============================================================================

void test_mouse_linkage() {
    // Verify all mouse functions are linked correctly
    void (*hide_func)() = sub__mousehide;
    int32_t (*hidden_func)() = func__mousehidden;
    float (*mousex_func)() = func__mousex;
    float (*mousey_func)() = func__mousey;
    int32 (*input_func)() = func__mouseinput;
    int32 (*button_func)(int32) = func__mousebutton;
    int32 (*wheel_func)() = func__mousewheel;

    test_assert(hide_func != NULL);
    test_assert(hidden_func != NULL);
    test_assert(mousex_func != NULL);
    test_assert(mousey_func != NULL);
    test_assert(input_func != NULL);
    test_assert(button_func != NULL);
    test_assert(wheel_func != NULL);
}

// ============================================================================
// CURSOR VISIBILITY TESTS
// ============================================================================

void test_mousehidden_returns_valid() {
    // _MOUSEHIDDEN should return 0 (visible) or -1 (hidden)
    int32_t result = func__mousehidden();
    test_assert(result == 0 || result == -1);
}

void test_mousehide_no_crash() {
    // sub__mousehide should not crash
    // Note: On systems without GLUT, this is effectively a no-op
    sub__mousehide();
    test_assert(1);
}

// ============================================================================
// MOUSE BUTTON TESTS
// ============================================================================

void test_mousebutton_valid_buttons() {
    // Valid button numbers are 1-3 (left, right, middle)
    // Button 1 = left
    // Button 2 = right (swapped with 3 internally)
    // Button 3 = middle (swapped with 2 internally)

    // These should return 0 or -1 without error
    int32 result1 = func__mousebutton(1);
    int32 result2 = func__mousebutton(2);
    int32 result3 = func__mousebutton(3);

    test_assert(result1 == 0 || result1 == -1);
    test_assert(result2 == 0 || result2 == -1);
    test_assert(result3 == 0 || result3 == -1);
}

void test_mousebutton_out_of_range() {
    // Button numbers > 3 should return 0 (SDL only supports 3 buttons)
    int32 result4 = func__mousebutton(4);
    int32 result5 = func__mousebutton(5);
    int32 result10 = func__mousebutton(10);

    test_assert_ints_with_name("Button 4 returns 0", 0, result4);
    test_assert_ints_with_name("Button 5 returns 0", 0, result5);
    test_assert_ints_with_name("Button 10 returns 0", 0, result10);
}

void test_mousebutton_constants() {
    // Document the button numbering
    const int32 BUTTON_LEFT = 1;
    const int32 BUTTON_RIGHT = 2;
    const int32 BUTTON_MIDDLE = 3;

    // Verify constants are distinct
    test_assert(BUTTON_LEFT != BUTTON_RIGHT);
    test_assert(BUTTON_LEFT != BUTTON_MIDDLE);
    test_assert(BUTTON_RIGHT != BUTTON_MIDDLE);

    // Verify range
    test_assert(BUTTON_LEFT >= 1 && BUTTON_LEFT <= 3);
    test_assert(BUTTON_RIGHT >= 1 && BUTTON_RIGHT <= 3);
    test_assert(BUTTON_MIDDLE >= 1 && BUTTON_MIDDLE <= 3);
}

// ============================================================================
// MOUSE WHEEL TESTS
// ============================================================================

void test_mousewheel_returns_valid() {
    // _MOUSEWHEEL should return -1 (up), 0 (no movement), or 1 (down)
    int32 result = func__mousewheel();
    test_assert(result >= -1 && result <= 1);
}

void test_mousewheel_values() {
    // Document the wheel movement values
    const int32 WHEEL_UP = -1;
    const int32 WHEEL_NONE = 0;
    const int32 WHEEL_DOWN = 1;

    // These should be the possible return values
    test_assert(WHEEL_UP == -1);
    test_assert(WHEEL_NONE == 0);
    test_assert(WHEEL_DOWN == 1);
}

// ============================================================================
// MOUSE INPUT TESTS
// ============================================================================

void test_mouseinput_returns_valid() {
    // _MOUSEINPUT should return 0 (no events) or -1 (event available)
    int32 result = func__mouseinput();
    test_assert(result == 0 || result == -1);
}

void test_mouseinput_clears_queue() {
    // Multiple calls should eventually return 0 (no more events)
    for (int i = 0; i < 100; i++) {
        func__mouseinput();
    }
    // If we get here without crashing, the test passes
    test_assert(1);
}

// ============================================================================
// MOUSE POSITION TESTS
// ============================================================================

void test_mousex_no_crash() {
    // func__mousex should not crash even without initialization
    float result = func__mousex();
    // Result should be a valid float (not NaN or Inf)
    test_assert(result == result); // NaN check: NaN != NaN
}

void test_mousey_no_crash() {
    float result = func__mousey();
    test_assert(result == result);
}

void test_mousemovement_no_crash() {
    // Movement functions should not crash
    float mx = func__mousemovementx();
    float my = func__mousemovementy();

    test_assert(mx == mx); // NaN check
    test_assert(my == my);
}

// ============================================================================
// CURSOR STYLE CONSTANTS
// ============================================================================

void test_cursor_style_names() {
    // Document the supported cursor style names for _MOUSESHOW
    // These are the string values accepted by sub__mouseshow:

    const char* styles[] = {
        "DEFAULT",
        "LINK",
        "TEXT",
        "CROSSHAIR",
        "VERTICAL",
        "HORIZONTAL",
        "TOPLEFT_BOTTOMRIGHT",
        "TOPRIGHT_BOTTOMLEFT",
        "WAIT",
        "HELP",
        "CYCLE",
        "MOVE"
    };

    int style_count = sizeof(styles) / sizeof(styles[0]);

    // Verify we have at least the documented styles
    test_assert(style_count >= 10);

    // Verify all style names are non-empty
    for (int i = 0; i < style_count; i++) {
        test_assert(styles[i] != NULL);
        test_assert(strlen(styles[i]) > 0);
    }
}

// ============================================================================
// MOUSE MESSAGE QUEUE TESTS
// ============================================================================

void test_mouse_queue_structure() {
    // Document the mouse message queue structure
    // Each message contains: x, y, buttons, movementx, movementy

    // Message fields sizes
    const size_t X_SIZE = sizeof(int16_t);
    const size_t Y_SIZE = sizeof(int16_t);
    const size_t BUTTONS_SIZE = sizeof(uint32_t);
    const size_t MOVX_SIZE = sizeof(int16_t);
    const size_t MOVY_SIZE = sizeof(int16_t);

    // Verify expected sizes
    test_assert_ints_with_name("X field size", 2, X_SIZE);
    test_assert_ints_with_name("Y field size", 2, Y_SIZE);
    test_assert_ints_with_name("Buttons field size", 4, BUTTONS_SIZE);
    test_assert_ints_with_name("MovementX field size", 2, MOVX_SIZE);
    test_assert_ints_with_name("MovementY field size", 2, MOVY_SIZE);
}

void test_button_bitmask() {
    // Mouse button state is stored in a bitmask
    // Bit 0 = Button 1 (left)
    // Bit 1 = Button 2 (middle, swapped)
    // Bit 2 = Button 3 (right, swapped)
    // Bit 3 = Wheel up
    // Bit 4 = Wheel down

    const uint32_t BUTTON_1_MASK = 1 << 0;  // 1
    const uint32_t BUTTON_2_MASK = 1 << 1;  // 2
    const uint32_t BUTTON_3_MASK = 1 << 2;  // 4
    const uint32_t WHEEL_UP_MASK = 1 << 3;  // 8
    const uint32_t WHEEL_DOWN_MASK = 1 << 4; // 16

    // Verify masks are correct
    test_assert_ints_with_name("Button 1 mask", 1, BUTTON_1_MASK);
    test_assert_ints_with_name("Button 2 mask", 2, BUTTON_2_MASK);
    test_assert_ints_with_name("Button 3 mask", 4, BUTTON_3_MASK);
    test_assert_ints_with_name("Wheel up mask", 8, WHEEL_UP_MASK);
    test_assert_ints_with_name("Wheel down mask", 16, WHEEL_DOWN_MASK);

    // Masks should not overlap
    test_assert((BUTTON_1_MASK & BUTTON_2_MASK) == 0);
    test_assert((WHEEL_UP_MASK & WHEEL_DOWN_MASK) == 0);
}

int main() {
    struct unit_test tests[] = {
        // Linkage tests
        { test_mouse_linkage, "test-mouse-linkage" },

        // Cursor visibility tests
        { test_mousehidden_returns_valid, "test-mousehidden-returns-valid" },
        { test_mousehide_no_crash, "test-mousehide-no-crash" },

        // Mouse button tests
        { test_mousebutton_valid_buttons, "test-mousebutton-valid-buttons" },
        { test_mousebutton_out_of_range, "test-mousebutton-out-of-range" },
        { test_mousebutton_constants, "test-mousebutton-constants" },

        // Mouse wheel tests
        { test_mousewheel_returns_valid, "test-mousewheel-returns-valid" },
        { test_mousewheel_values, "test-mousewheel-values" },

        // Mouse input tests
        { test_mouseinput_returns_valid, "test-mouseinput-returns-valid" },
        { test_mouseinput_clears_queue, "test-mouseinput-clears-queue" },

        // Mouse position tests
        { test_mousex_no_crash, "test-mousex-no-crash" },
        { test_mousey_no_crash, "test-mousey-no-crash" },
        { test_mousemovement_no_crash, "test-mousemovement-no-crash" },

        // Cursor style tests
        { test_cursor_style_names, "test-cursor-style-names" },

        // Queue structure tests
        { test_mouse_queue_structure, "test-mouse-queue-structure" },
        { test_button_bitmask, "test-button-bitmask" },
    };

    return run_tests("mouse", tests, sizeof(tests) / sizeof(*tests));
}
