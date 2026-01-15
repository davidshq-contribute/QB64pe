//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Graphics Module Unit Tests
//  Tests for graphics functions that can be tested in isolation
//
//  Purpose:
//    This test suite verifies HSB (Hue, Saturation, Brightness) to RGB color
//    conversion functions and related graphics utilities in the QB64-PE graphics
//    module.
//
//  Test Coverage:
//    - HSB to RGB conversion (func__hsb32)
//    - HSBA to RGBA conversion with alpha (func__hsba32)
//    - Hue extraction from RGB colors (func__hue32)
//    - Saturation extraction (func__sat32)
//    - Brightness extraction (func__bri32)
//    - Edge cases (zero saturation, zero brightness, hue wrapping)
//    - Roundtrip tests (HSB -> RGB -> HSB)
//
//  Important Notes:
//    - HSB functions expect saturation and brightness as percentages (0-100),
//      not normalized values (0.0-1.0)
//    - Hue values are in degrees (0-360), with wrapping at 360 degrees
//    - Alpha values in HSBA are also percentages (0-100)
//
//  Limitations:
//    Some tests may require QB64 runtime initialization or additional
//    dependencies. The HSB/RGB conversion functions should work independently,
//    but other graphics functions may need image/page setup.
//
//  How to Run:
//    1. Build the test: make build-tests
//    2. Run individually: ./tests/exes/cpp/graphics_test
//    3. Run all C++ tests: ./tests/run_c_tests.sh
//
//  Expected Behavior:
//    All tests should pass. Tests verify:
//    - Correct HSB to RGB conversion for primary colors
//    - Proper handling of edge cases (grayscale, black, white)
//    - Hue wrapping (360 degrees = 0 degrees)
//    - Accurate channel extraction from RGB colors
//    - Roundtrip consistency (HSB -> RGB -> HSB conversion)
//
//----------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <float.h>

#include "test.h"

// Type definitions (matching os.h)
#define int32 int32_t
#define uint32 uint32_t

// Forward declarations for graphics functions we're testing
// Note: These would normally come from graphics.h, but we're testing in isolation
extern "C" {
    // HSB/RGB Color Conversion functions
    uint32_t func__hsb32(double hue, double sat, double bri);
    uint32_t func__hsba32(double hue, double sat, double bri, double alf);
    double func__hue32(uint32_t argb);
    double func__sat32(uint32_t argb);
    double func__bri32(uint32_t argb);
}

// Helper function to compare floating point values with tolerance
static int double_approx_equal(double a, double b, double tolerance) {
    return (fabs(a - b) < tolerance) ? 1 : 0;
}

// Test HSB to RGB conversion
// Note: func__hsb32 expects saturation and brightness as percentages (0-100), not normalized (0.0-1.0)
void test_hsb32_basic() {
    // Test basic colors
    // Red: hue=0, sat=100%, bri=100% -> RGB(255, 0, 0)
    uint32_t red = func__hsb32(0.0, 100.0, 100.0);
    uint32_t expected_red = 0xFF0000; // ARGB format: 0xFF000000
    test_assert_ints_with_name("Red hue=0", expected_red, red & 0x00FFFFFF);
    
    // Green: hue=120, sat=100%, bri=100% -> RGB(0, 255, 0)
    uint32_t green = func__hsb32(120.0, 100.0, 100.0);
    uint32_t expected_green = 0x00FF00;
    test_assert_ints_with_name("Green hue=120", expected_green, green & 0x00FFFFFF);
    
    // Blue: hue=240, sat=100%, bri=100% -> RGB(0, 0, 255)
    uint32_t blue = func__hsb32(240.0, 100.0, 100.0);
    uint32_t expected_blue = 0x0000FF;
    test_assert_ints_with_name("Blue hue=240", expected_blue, blue & 0x00FFFFFF);
}

void test_hsb32_edge_cases() {
    // Test zero saturation (should be grayscale)
    // 50% brightness = 50.0 (percentage)
    uint32_t gray = func__hsb32(0.0, 0.0, 50.0);
    uint32_t expected_gray = 0x808080; // 50% brightness = 128
    test_assert_ints_with_name("Gray zero saturation", expected_gray, gray & 0x00FFFFFF);
    
    // Test zero brightness (should be black)
    uint32_t black = func__hsb32(0.0, 100.0, 0.0);
    uint32_t expected_black = 0x000000;
    test_assert_ints_with_name("Black zero brightness", expected_black, black & 0x00FFFFFF);
    
    // Test full brightness, zero saturation (should be white)
    uint32_t white = func__hsb32(0.0, 0.0, 100.0);
    uint32_t expected_white = 0xFFFFFF;
    test_assert_ints_with_name("White full brightness", expected_white, white & 0x00FFFFFF);
}

void test_hsb32_hue_wrapping() {
    // Test hue wrapping (360 = 0)
    uint32_t red1 = func__hsb32(0.0, 100.0, 100.0);
    uint32_t red2 = func__hsb32(360.0, 100.0, 100.0);
    test_assert_ints_with_name("Hue 0 == 360", red1 & 0x00FFFFFF, red2 & 0x00FFFFFF);
    
    // Test negative hue (should be clamped to 0)
    uint32_t red3 = func__hsb32(-360.0, 100.0, 100.0);
    test_assert_ints_with_name("Hue -360 clamped to 0", red1 & 0x00FFFFFF, red3 & 0x00FFFFFF);
}

void test_hsba32_alpha() {
    // Test RGBA with alpha channel
    // Note: func__hsba32 expects alpha as percentage (0-100), not normalized (0.0-1.0)
    uint32_t red_alpha = func__hsba32(0.0, 100.0, 100.0, 50.0);
    // Alpha should be in the high byte: 0x80 = 128 = 50% of 255
    uint32_t expected_alpha = 0x80;
    uint32_t actual_alpha = (red_alpha >> 24) & 0xFF;
    test_assert_ints_with_name("Alpha 50%", expected_alpha, actual_alpha);
    
    // Test full alpha
    uint32_t red_full = func__hsba32(0.0, 100.0, 100.0, 100.0);
    uint32_t expected_full_alpha = 0xFF;
    uint32_t actual_full_alpha = (red_full >> 24) & 0xFF;
    test_assert_ints_with_name("Alpha 100%", expected_full_alpha, actual_full_alpha);
    
    // Test zero alpha
    uint32_t red_zero = func__hsba32(0.0, 100.0, 100.0, 0.0);
    uint32_t expected_zero_alpha = 0x00;
    uint32_t actual_zero_alpha = (red_zero >> 24) & 0xFF;
    test_assert_ints_with_name("Alpha 0%", expected_zero_alpha, actual_zero_alpha);
}

void test_hue32_extraction() {
    // Test hue extraction from RGB colors
    uint32_t red = 0xFF0000;
    double red_hue = func__hue32(red);
    test_assert(double_approx_equal(red_hue, 0.0, 1.0) || double_approx_equal(red_hue, 360.0, 1.0));
    
    uint32_t green = 0x00FF00;
    double green_hue = func__hue32(green);
    test_assert(double_approx_equal(green_hue, 120.0, 1.0));
    
    uint32_t blue = 0x0000FF;
    double blue_hue = func__hue32(blue);
    test_assert(double_approx_equal(blue_hue, 240.0, 1.0));
}

void test_sat32_extraction() {
    // Test saturation extraction
    // Note: func__sat32 returns saturation as percentage (0-100), not normalized (0.0-1.0)
    uint32_t full_color = 0xFF0000; // Full red, should have saturation = 100%
    double sat = func__sat32(full_color);
    test_assert(double_approx_equal(sat, 100.0, 1.0));
    
    uint32_t gray = 0x808080; // Gray, should have saturation = 0%
    double gray_sat = func__sat32(gray);
    test_assert(double_approx_equal(gray_sat, 0.0, 1.0));
}

void test_bri32_extraction() {
    // Test brightness extraction
    // Note: func__bri32 returns brightness as percentage (0-100), not normalized (0.0-1.0)
    uint32_t white = 0xFFFFFF; // White, should have brightness = 100%
    double bri = func__bri32(white);
    test_assert(double_approx_equal(bri, 100.0, 1.0));
    
    uint32_t black = 0x000000; // Black, should have brightness = 0%
    double black_bri = func__bri32(black);
    test_assert(double_approx_equal(black_bri, 0.0, 1.0));
    
    uint32_t gray = 0x808080; // 50% gray, should have brightness ≈ 50%
    double gray_bri = func__bri32(gray);
    test_assert(double_approx_equal(gray_bri, 50.0, 5.0));
}

void test_hsb_roundtrip() {
    // Test that HSB -> RGB -> HSB conversion preserves values (approximately)
    // Note: func__hsb32 expects percentages (0-100), func__sat32/func__bri32 return percentages
    double test_hue = 180.0;
    double test_sat = 75.0;  // 75%
    double test_bri = 60.0;  // 60%
    
    uint32_t rgb = func__hsb32(test_hue, test_sat, test_bri);
    double extracted_hue = func__hue32(rgb);
    double extracted_sat = func__sat32(rgb);
    double extracted_bri = func__bri32(rgb);
    
    // Hue can wrap, so check if it's close to original or original ± 360
    int hue_match = (double_approx_equal(extracted_hue, test_hue, 2.0) ||
                     double_approx_equal(extracted_hue, test_hue - 360.0, 2.0) ||
                     double_approx_equal(extracted_hue, test_hue + 360.0, 2.0)) ? 1 : 0;
    test_assert_with_name("HSB roundtrip hue", hue_match);
    test_assert(double_approx_equal(extracted_sat, test_sat, 5.0));
    test_assert(double_approx_equal(extracted_bri, test_bri, 5.0));
}

int main() {
    struct unit_test tests[] = {
        { test_hsb32_basic, "test-hsb32-basic" },
        { test_hsb32_edge_cases, "test-hsb32-edge-cases" },
        { test_hsb32_hue_wrapping, "test-hsb32-hue-wrapping" },
        { test_hsba32_alpha, "test-hsba32-alpha" },
        { test_hue32_extraction, "test-hue32-extraction" },
        { test_sat32_extraction, "test-sat32-extraction" },
        { test_bri32_extraction, "test-bri32-extraction" },
        { test_hsb_roundtrip, "test-hsb-roundtrip" },
    };

    return run_tests("graphics", tests, sizeof(tests) / sizeof(*tests));
}
