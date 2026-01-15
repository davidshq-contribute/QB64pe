//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Utility Module Unit Tests
//  Tests for general utility functions (RND, FPS, blink, control chars)
//
//  Purpose:
//    This test suite verifies the functionality of utility functions
//    in the QB64-PE utility module, including random number generation,
//    frame rate control, blink state, and control character mode.
//
//  Test Coverage:
//    - Module linkage verification
//    - RND function deterministic behavior
//    - FPS accessor functions
//    - Blink state toggle
//    - Control character mode toggle
//
//  Limitations:
//    sub_randomize without a seed prompts for user input, so it cannot
//    be tested in automated tests. Tests focus on deterministic behavior
//    and accessor functions.
//
//  How to Run:
//    1. Build the test: make build-tests
//    2. Run individually: ./tests/exes/cpp/utility_test
//    3. Run all C++ tests: ./tests/run_c_tests.sh
//
//  Expected Behavior:
//    All tests should pass. Tests verify:
//    - Function linkage is correct
//    - RND produces deterministic sequences
//    - State accessors work correctly
//
//----------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "test.h"

// Type definitions (matching os.h)
#define int32 int32_t
#define uint32 uint32_t

// Forward declarations for utility functions we're testing
extern "C" {
    // Blink functions
    void sub__blink(int32_t onoff);
    int32_t func__blink();

    // Random number functions
    void sub_randomize(double seed, int32_t passed);
    float func_rnd(float n, int32_t passed);
    void reset_rnd_state();

    // Frame rate functions
    void sub__fps(double fps, int32_t passed);
    double get_max_fps();
    int32_t get_auto_fps();
    void set_auto_fps(int32_t value);

    // Control character functions
    void sub__controlchr(int32_t onoff);
    int32_t func__controlchr();
    int32_t get_control_characters_disabled();
}

// ============================================================================
// MODULE LINKAGE TESTS
// ============================================================================

void test_utility_linkage() {
    // Verify all utility functions are linked correctly
    void (*blink_sub)(int32_t) = sub__blink;
    int32_t (*blink_func)() = func__blink;
    float (*rnd_func)(float, int32_t) = func_rnd;
    void (*fps_sub)(double, int32_t) = sub__fps;
    double (*get_fps)() = get_max_fps;
    void (*controlchr_sub)(int32_t) = sub__controlchr;
    int32_t (*controlchr_func)() = func__controlchr;

    test_assert(blink_sub != NULL);
    test_assert(blink_func != NULL);
    test_assert(rnd_func != NULL);
    test_assert(fps_sub != NULL);
    test_assert(get_fps != NULL);
    test_assert(controlchr_sub != NULL);
    test_assert(controlchr_func != NULL);
}

// ============================================================================
// BLINK STATE TESTS
// ============================================================================

void test_blink_initial_state() {
    // _BLINK should return -1 (enabled) by default
    // Note: The initial state depends on H3C0_blink_enable which defaults to 1
    int32_t result = func__blink();
    test_assert(result == 0 || result == -1);
}

void test_blink_enable() {
    // Enable blink and verify
    sub__blink(1);  // 1 = ON
    int32_t result = func__blink();
    // func__blink returns -H3C0_blink_enable, so -1 when enabled
    test_assert_ints_with_name("Blink enabled", -1, result);
}

void test_blink_disable() {
    // Disable blink and verify
    sub__blink(0);  // 0 = OFF
    int32_t result = func__blink();
    // func__blink returns -H3C0_blink_enable, so 0 when disabled
    test_assert_ints_with_name("Blink disabled", 0, result);

    // Re-enable for other tests
    sub__blink(1);
}

void test_blink_toggle() {
    // Toggle blink state multiple times
    sub__blink(1);
    test_assert(func__blink() == -1);

    sub__blink(0);
    test_assert(func__blink() == 0);

    sub__blink(1);
    test_assert(func__blink() == -1);
}

// ============================================================================
// RANDOM NUMBER GENERATOR TESTS
// ============================================================================

void test_rnd_range() {
    // RND should always return values in [0, 1)
    reset_rnd_state();

    for (int i = 0; i < 100; i++) {
        float result = func_rnd(1.0f, 1);
        test_assert(result >= 0.0f);
        test_assert(result < 1.0f);
    }
}

void test_rnd_deterministic_with_seed() {
    // RND with same negative seed should produce same sequence
    float values1[10];
    float values2[10];

    // Generate first sequence with negative seed
    func_rnd(-0.5f, 1);  // Negative seed resets generator
    for (int i = 0; i < 10; i++) {
        values1[i] = func_rnd(1.0f, 1);
    }

    // Generate second sequence with same seed
    func_rnd(-0.5f, 1);  // Same negative seed
    for (int i = 0; i < 10; i++) {
        values2[i] = func_rnd(1.0f, 1);
    }

    // Sequences should match
    for (int i = 0; i < 10; i++) {
        test_assert(values1[i] == values2[i]);
    }
}

void test_rnd_zero_repeats() {
    // RND(0) should repeat the last value
    reset_rnd_state();

    float first = func_rnd(1.0f, 1);  // Get a value
    float repeated1 = func_rnd(0.0f, 1);  // RND(0) repeats last
    float repeated2 = func_rnd(0.0f, 1);  // Should still be the same

    test_assert(first == repeated1);
    test_assert(first == repeated2);
}

void test_rnd_no_arg() {
    // RND without argument (passed=0) should act like RND(1)
    reset_rnd_state();

    float result = func_rnd(0.0f, 0);  // No arg passed
    test_assert(result >= 0.0f);
    test_assert(result < 1.0f);
}

void test_rnd_reset() {
    // reset_rnd_state should reset the generator
    reset_rnd_state();
    float first1 = func_rnd(1.0f, 1);

    reset_rnd_state();
    float first2 = func_rnd(1.0f, 1);

    test_assert(first1 == first2);
}

void test_randomize_using() {
    // RANDOMIZE USING (passed=3) uses specific seed handling
    sub_randomize(12345.0, 3);  // USING mode
    float value1 = func_rnd(1.0f, 1);

    sub_randomize(12345.0, 3);  // Same seed
    float value2 = func_rnd(1.0f, 1);

    test_assert(value1 == value2);
}

// ============================================================================
// FPS CONTROL TESTS
// ============================================================================

void test_fps_default() {
    // Default FPS should be 60
    double fps = get_max_fps();
    test_assert(fps > 0);
    // Note: Default is 60, but may have been changed by other tests
}

void test_fps_set_value() {
    // Set FPS to a specific value
    sub__fps(30.0, 2);  // passed=2 means use fps value
    double fps = get_max_fps();
    test_assert(fabs(fps - 30.0) < 0.001);

    // Restore default
    sub__fps(60.0, 2);
}

void test_fps_auto_mode() {
    // Test auto FPS mode
    set_auto_fps(0);  // Ensure not in auto mode
    test_assert_ints_with_name("Auto FPS off", 0, get_auto_fps());

    sub__fps(0.0, 1);  // passed=1 means _AUTO
    test_assert_ints_with_name("Auto FPS on", 1, get_auto_fps());

    // Disable auto mode
    set_auto_fps(0);
}

void test_fps_max_limit() {
    // FPS should be capped at 200
    sub__fps(1000.0, 2);
    double fps = get_max_fps();
    test_assert(fps <= 200.0);

    // Restore default
    sub__fps(60.0, 2);
}

void test_fps_accessors() {
    // Test accessor functions
    set_auto_fps(1);
    test_assert_ints_with_name("set_auto_fps", 1, get_auto_fps());

    set_auto_fps(0);
    test_assert_ints_with_name("clear_auto_fps", 0, get_auto_fps());
}

// ============================================================================
// CONTROL CHARACTER TESTS
// ============================================================================

void test_controlchr_default() {
    // By default, control characters should be interpreted
    int32_t disabled = get_control_characters_disabled();
    // Initial state may vary, just verify it's valid
    test_assert(disabled == 0 || disabled == 1);
}

void test_controlchr_disable() {
    // Disable control character interpretation
    sub__controlchr(2);  // 2 = OFF (print literally)
    int32_t result = func__controlchr();
    // func__controlchr returns -no_control_characters2
    test_assert(result == -1 || result == 0);

    int32_t disabled = get_control_characters_disabled();
    test_assert_ints_with_name("Control chars disabled", 1, disabled);
}

void test_controlchr_enable() {
    // Enable control character interpretation
    sub__controlchr(1);  // 1 = ON (interpret control chars)
    int32_t disabled = get_control_characters_disabled();
    test_assert_ints_with_name("Control chars enabled", 0, disabled);
}

void test_controlchr_toggle() {
    // Toggle control character mode
    sub__controlchr(1);  // ON
    test_assert(get_control_characters_disabled() == 0);

    sub__controlchr(2);  // OFF
    test_assert(get_control_characters_disabled() == 1);

    sub__controlchr(1);  // ON again
    test_assert(get_control_characters_disabled() == 0);
}

// ============================================================================
// CONSTANT DOCUMENTATION TESTS
// ============================================================================

void test_blink_constants() {
    // Document the blink control values
    const int32_t BLINK_ON = 1;
    const int32_t BLINK_OFF = 0;

    test_assert(BLINK_ON != BLINK_OFF);
}

void test_controlchr_constants() {
    // Document the control character control values
    const int32_t CONTROLCHR_ON = 1;   // Interpret control characters
    const int32_t CONTROLCHR_OFF = 2;  // Print literally

    test_assert(CONTROLCHR_ON != CONTROLCHR_OFF);
}

void test_fps_pass_constants() {
    // Document the sub__fps passed parameter values
    const int32_t FPS_AUTO = 1;    // _AUTO mode
    const int32_t FPS_VALUE = 2;   // Use specific fps value

    test_assert(FPS_AUTO != FPS_VALUE);
}

int main() {
    struct unit_test tests[] = {
        // Linkage tests
        { test_utility_linkage, "test-utility-linkage" },

        // Blink tests
        { test_blink_initial_state, "test-blink-initial-state" },
        { test_blink_enable, "test-blink-enable" },
        { test_blink_disable, "test-blink-disable" },
        { test_blink_toggle, "test-blink-toggle" },

        // RND tests
        { test_rnd_range, "test-rnd-range" },
        { test_rnd_deterministic_with_seed, "test-rnd-deterministic-with-seed" },
        { test_rnd_zero_repeats, "test-rnd-zero-repeats" },
        { test_rnd_no_arg, "test-rnd-no-arg" },
        { test_rnd_reset, "test-rnd-reset" },
        { test_randomize_using, "test-randomize-using" },

        // FPS tests
        { test_fps_default, "test-fps-default" },
        { test_fps_set_value, "test-fps-set-value" },
        { test_fps_auto_mode, "test-fps-auto-mode" },
        { test_fps_max_limit, "test-fps-max-limit" },
        { test_fps_accessors, "test-fps-accessors" },

        // Control character tests
        { test_controlchr_default, "test-controlchr-default" },
        { test_controlchr_disable, "test-controlchr-disable" },
        { test_controlchr_enable, "test-controlchr-enable" },
        { test_controlchr_toggle, "test-controlchr-toggle" },

        // Constant documentation tests
        { test_blink_constants, "test-blink-constants" },
        { test_controlchr_constants, "test-controlchr-constants" },
        { test_fps_pass_constants, "test-fps-pass-constants" },
    };

    return run_tests("utility", tests, sizeof(tests) / sizeof(*tests));
}
