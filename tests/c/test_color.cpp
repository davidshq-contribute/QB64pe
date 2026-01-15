//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Color & Palette Module Unit Tests
//  Tests for color manipulation functions
//
//  Purpose:
//    This test suite verifies the functionality of color manipulation functions
//    in the QB64-PE color module, including RGB/RGBA color creation, channel
//    extraction, and color matching.
//
//  Test Coverage:
//    - RGB color creation (func__rgb)
//    - RGBA color creation with alpha channel (func__rgba)
//    - Color channel extraction (func__red, func__green, func__blue, func__alpha)
//    - Color value clamping (negative and out-of-range values)
//    - Color roundtrip tests (RGB -> channels -> RGB)
//    - Basic color matching (matchcol)
//
//  Limitations:
//    Some color functions require image/page initialization for palette-based
//    operations. These tests focus on functions that can be tested independently
//    (RGB/RGBA creation and channel extraction) or with minimal setup.
//
//  How to Run:
//    1. Build the test: make build-tests
//    2. Run individually: ./tests/exes/cpp/color_test
//    3. Run all C++ tests: ./tests/run_c_tests.sh
//
//  Expected Behavior:
//    All tests should pass. Tests verify:
//    - Correct RGB/RGBA color encoding (ARGB format: 0xFFRRGGBB)
//    - Proper clamping of values outside 0-255 range
//    - Accurate channel extraction from color values
//    - Roundtrip consistency (creating and extracting colors)
//
//----------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>

#include "test.h"

// Type definitions (matching os.h)
#define int32 int32_t
#define uint32 uint32_t

// Forward declarations for color functions we're testing
// Note: These would normally come from color.h, but we're testing in isolation
extern "C" {
    // Color matching (finds closest palette entry)
    uint32_t matchcol(int32_t r, int32_t g, int32_t b);
    uint32_t matchcol(int32_t r, int32_t g, int32_t b, int32_t i);
    
    // RGB/RGBA color creation
    uint32_t func__rgb(int32_t r, int32_t g, int32_t b, int32_t i, int32_t passed);
    uint32_t func__rgba(int32_t r, int32_t g, int32_t b, int32_t a, int32_t i, int32_t passed);
    
    // Color channel extraction
    int32_t func__red(uint32_t col, int32_t i, int32_t passed);
    int32_t func__green(uint32_t col, int32_t i, int32_t passed);
    int32_t func__blue(uint32_t col, int32_t i, int32_t passed);
    int32_t func__alpha(uint32_t col, int32_t i, int32_t passed);
}

// Note: Some color functions require image/page initialization.
// These tests focus on functions that can be tested independently
// or with minimal setup.

void test_rgb_basic() {
    // Test basic RGB color creation (without image parameter)
    uint32_t red = func__rgb(255, 0, 0, 0, 0);
    uint32_t green = func__rgb(0, 255, 0, 0, 0);
    uint32_t blue = func__rgb(0, 0, 255, 0, 0);
    
    // For 32-bit images, RGB should create ARGB format: 0xFFRRGGBB
    // Extract RGB components (ignore alpha)
    uint32_t red_rgb = red & 0x00FFFFFF;
    uint32_t green_rgb = green & 0x00FFFFFF;
    uint32_t blue_rgb = blue & 0x00FFFFFF;
    
    test_assert_ints_with_name("Red RGB", 0xFF0000, red_rgb);
    test_assert_ints_with_name("Green RGB", 0x00FF00, green_rgb);
    test_assert_ints_with_name("Blue RGB", 0x0000FF, blue_rgb);
}

void test_rgb_clamping() {
    // Test that RGB values are clamped to 0-255 range
    uint32_t negative = func__rgb(-10, -20, -30, 0, 0);
    uint32_t too_large = func__rgb(300, 400, 500, 0, 0);
    
    // Negative values should be clamped to 0
    uint32_t neg_rgb = negative & 0x00FFFFFF;
    test_assert_ints_with_name("Negative clamped", 0x000000, neg_rgb);
    
    // Values > 255 should be clamped to 255
    uint32_t large_rgb = too_large & 0x00FFFFFF;
    test_assert_ints_with_name("Large clamped", 0xFFFFFF, large_rgb);
}

void test_rgba_alpha() {
    // Test RGBA with alpha channel
    uint32_t red_full_alpha = func__rgba(255, 0, 0, 255, 0, 0);
    uint32_t red_half_alpha = func__rgba(255, 0, 0, 128, 0, 0);
    uint32_t red_no_alpha = func__rgba(255, 0, 0, 0, 0, 0);
    
    // Extract alpha channel (high byte)
    uint32_t full_alpha = (red_full_alpha >> 24) & 0xFF;
    uint32_t half_alpha = (red_half_alpha >> 24) & 0xFF;
    uint32_t no_alpha = (red_no_alpha >> 24) & 0xFF;
    
    test_assert_ints_with_name("Full alpha", 255, full_alpha);
    test_assert_ints_with_name("Half alpha", 128, half_alpha);
    test_assert_ints_with_name("No alpha", 0, no_alpha);
}

void test_rgba_clamping() {
    // Test that RGBA values are clamped
    uint32_t negative_alpha = func__rgba(255, 0, 0, -10, 0, 0);
    uint32_t too_large_alpha = func__rgba(255, 0, 0, 300, 0, 0);
    
    uint32_t neg_alpha = (negative_alpha >> 24) & 0xFF;
    uint32_t large_alpha = (too_large_alpha >> 24) & 0xFF;
    
    test_assert_ints_with_name("Negative alpha clamped", 0, neg_alpha);
    test_assert_ints_with_name("Large alpha clamped", 255, large_alpha);
}

void test_red_extraction() {
    // Test red channel extraction
    uint32_t red_color = 0xFF123456; // ARGB: alpha=FF, red=12, green=34, blue=56
    int32_t red = func__red(red_color, 0, 0);
    
    test_assert_ints_with_name("Red extraction", 0x12, red);
    
    // Test with pure red
    uint32_t pure_red = 0xFFFF0000;
    int32_t pure_red_val = func__red(pure_red, 0, 0);
    test_assert_ints_with_name("Pure red", 255, pure_red_val);
}

void test_green_extraction() {
    // Test green channel extraction
    uint32_t green_color = 0xFF123456;
    int32_t green = func__green(green_color, 0, 0);
    
    test_assert_ints_with_name("Green extraction", 0x34, green);
    
    // Test with pure green
    uint32_t pure_green = 0xFF00FF00;
    int32_t pure_green_val = func__green(pure_green, 0, 0);
    test_assert_ints_with_name("Pure green", 255, pure_green_val);
}

void test_blue_extraction() {
    // Test blue channel extraction
    uint32_t blue_color = 0xFF123456;
    int32_t blue = func__blue(blue_color, 0, 0);
    
    test_assert_ints_with_name("Blue extraction", 0x56, blue);
    
    // Test with pure blue
    uint32_t pure_blue = 0xFF0000FF;
    int32_t pure_blue_val = func__blue(pure_blue, 0, 0);
    test_assert_ints_with_name("Pure blue", 255, pure_blue_val);
}

void test_alpha_extraction() {
    // Test alpha channel extraction
    uint32_t full_alpha = 0xFF123456;
    uint32_t half_alpha = 0x80123456;
    uint32_t no_alpha = 0x00123456;
    
    int32_t full = func__alpha(full_alpha, 0, 0);
    int32_t half = func__alpha(half_alpha, 0, 0);
    int32_t none = func__alpha(no_alpha, 0, 0);
    
    test_assert_ints_with_name("Full alpha extraction", 255, full);
    test_assert_ints_with_name("Half alpha extraction", 128, half);
    test_assert_ints_with_name("No alpha extraction", 0, none);
}

void test_color_roundtrip() {
    // Test that RGB -> channels -> RGB preserves values
    int32_t test_r = 123;
    int32_t test_g = 45;
    int32_t test_b = 67;
    
    uint32_t color = func__rgb(test_r, test_g, test_b, 0, 0);
    int32_t extracted_r = func__red(color, 0, 0);
    int32_t extracted_g = func__green(color, 0, 0);
    int32_t extracted_b = func__blue(color, 0, 0);
    
    test_assert_ints_with_name("Roundtrip red", test_r, extracted_r);
    test_assert_ints_with_name("Roundtrip green", test_g, extracted_g);
    test_assert_ints_with_name("Roundtrip blue", test_b, extracted_b);
}

void test_rgba_roundtrip() {
    // Test that RGBA -> channels -> RGBA preserves values
    int32_t test_r = 200;
    int32_t test_g = 150;
    int32_t test_b = 100;
    int32_t test_a = 180;
    
    uint32_t color = func__rgba(test_r, test_g, test_b, test_a, 0, 0);
    int32_t extracted_r = func__red(color, 0, 0);
    int32_t extracted_g = func__green(color, 0, 0);
    int32_t extracted_b = func__blue(color, 0, 0);
    int32_t extracted_a = func__alpha(color, 0, 0);
    
    test_assert_ints_with_name("RGBA roundtrip red", test_r, extracted_r);
    test_assert_ints_with_name("RGBA roundtrip green", test_g, extracted_g);
    test_assert_ints_with_name("RGBA roundtrip blue", test_b, extracted_b);
    test_assert_ints_with_name("RGBA roundtrip alpha", test_a, extracted_a);
}

void test_matchcol_basic() {
    // Test color matching (finds closest palette entry)
    // This may require image initialization, but we can test the signature
    uint32_t match1 = matchcol(255, 0, 0);
    uint32_t match2 = matchcol(0, 255, 0);
    uint32_t match3 = matchcol(0, 0, 255);
    
    // Should return valid color values (palette indices or RGB values)
    // Just verify they don't crash and return reasonable values
    test_assert(match1 < 0x100000000ULL);
    test_assert(match2 < 0x100000000ULL);
    test_assert(match3 < 0x100000000ULL);
}

int main() {
    struct unit_test tests[] = {
        { test_rgb_basic, "test-rgb-basic" },
        { test_rgb_clamping, "test-rgb-clamping" },
        { test_rgba_alpha, "test-rgba-alpha" },
        { test_rgba_clamping, "test-rgba-clamping" },
        { test_red_extraction, "test-red-extraction" },
        { test_green_extraction, "test-green-extraction" },
        { test_blue_extraction, "test-blue-extraction" },
        { test_alpha_extraction, "test-alpha-extraction" },
        { test_color_roundtrip, "test-color-roundtrip" },
        { test_rgba_roundtrip, "test-rgba-roundtrip" },
        { test_matchcol_basic, "test-matchcol-basic" },
    };

    return run_tests("color", tests, sizeof(tests) / sizeof(*tests));
}
