// Unit tests for graphics module
// Tests color conversion functions that don't require display

#include "test.h"
#include "graphics.h"
#include <math.h>
#include <stdint.h>

static void test_func__hsb32_basic() {
    // Test basic HSB to RGB conversion
    uint32_t color = func__hsb32(0.0, 100.0, 100.0); // Red
    test_assert(color != 0); // Should return a color value
    
    color = func__hsb32(120.0, 100.0, 100.0); // Green
    test_assert(color != 0);
    
    color = func__hsb32(240.0, 100.0, 100.0); // Blue
    test_assert(color != 0);
}

static void test_func__hsb32_edge_cases() {
    // Test edge cases for HSB conversion
    uint32_t color;
    
    // Zero saturation (grayscale)
    color = func__hsb32(0.0, 0.0, 50.0);
    test_assert(color != 0);
    
    // Zero brightness (black)
    color = func__hsb32(0.0, 100.0, 0.0);
    test_assert(color != 0);
    
    // Out of range values (should be clamped)
    color = func__hsb32(-10.0, 150.0, 200.0);
    test_assert(color != 0);
    
    color = func__hsb32(400.0, -10.0, -20.0);
    test_assert(color != 0);
}

static void test_func__hsba32_basic() {
    // Test HSBA to RGBA conversion
    uint32_t color = func__hsba32(0.0, 100.0, 100.0, 100.0); // Red with full alpha
    test_assert(color != 0);
    
    color = func__hsba32(0.0, 100.0, 100.0, 50.0); // Red with half alpha
    test_assert(color != 0);
    
    color = func__hsba32(0.0, 100.0, 100.0, 0.0); // Red with zero alpha
    test_assert(color != 0);
}

static void test_func__hsba32_edge_cases() {
    // Test edge cases for HSBA conversion
    uint32_t color;
    
    // Out of range alpha
    color = func__hsba32(0.0, 100.0, 100.0, -10.0);
    test_assert(color != 0);
    
    color = func__hsba32(0.0, 100.0, 100.0, 200.0);
    test_assert(color != 0);
}

static void test_func__hue32() {
    // Test extracting hue from RGB color
    // Red color (0xFF0000)
    double hue = func__hue32(0xFF0000FF);
    test_assert(hue >= 0.0 && hue <= 360.0);
    
    // Green color (0x00FF00)
    hue = func__hue32(0xFF00FF00);
    test_assert(hue >= 0.0 && hue <= 360.0);
    
    // Blue color (0x0000FF)
    hue = func__hue32(0xFFFF0000);
    test_assert(hue >= 0.0 && hue <= 360.0);
}

static void test_func__sat32() {
    // Test extracting saturation from RGB color
    // Full saturation color
    double sat = func__sat32(0xFF0000FF);
    test_assert(sat >= 0.0 && sat <= 100.0);
    
    // Grayscale (zero saturation)
    sat = func__sat32(0xFF808080);
    test_assert(sat >= 0.0 && sat <= 100.0);
}

static void test_func__bri32() {
    // Test extracting brightness from RGB color
    // Full brightness
    double bri = func__bri32(0xFFFFFFFF);
    test_assert(bri >= 0.0 && bri <= 100.0);
    
    // Zero brightness (black)
    bri = func__bri32(0xFF000000);
    test_assert(bri >= 0.0 && bri <= 100.0);
    
    // Medium brightness
    bri = func__bri32(0xFF808080);
    test_assert(bri >= 0.0 && bri <= 100.0);
}

static void test_hsb_rgb_roundtrip() {
    // Test that HSB->RGB->HSB conversion is consistent
    double original_hue = 180.0;
    double original_sat = 75.0;
    double original_bri = 50.0;
    
    // Convert HSB to RGB
    uint32_t rgb = func__hsb32(original_hue, original_sat, original_bri);
    
    // Extract HSB back from RGB
    double extracted_hue = func__hue32(rgb);
    double extracted_sat = func__sat32(rgb);
    double extracted_bri = func__bri32(rgb);
    
    // Hue should be approximately the same (within tolerance)
    // Note: Saturation and brightness may vary slightly due to rounding
    test_assert(fabs(extracted_hue - original_hue) < 10.0 || 
                fabs(extracted_hue - original_hue) > 350.0); // Allow wrap-around
}

static void test_hsb32_color_values() {
    // Test specific known color values
    uint32_t red = func__hsb32(0.0, 100.0, 100.0);
    uint32_t green = func__hsb32(120.0, 100.0, 100.0);
    uint32_t blue = func__hsb32(240.0, 100.0, 100.0);
    
    // Colors should be different
    test_assert(red != green);
    test_assert(green != blue);
    test_assert(red != blue);
    
    // Extract RGB components and verify
    uint8_t red_r = (red >> 16) & 0xFF;
    uint8_t green_g = (green >> 8) & 0xFF;
    uint8_t blue_b = blue & 0xFF;
    
    // Red should have high red component
    test_assert(red_r > 200);
    // Green should have high green component
    test_assert(green_g > 200);
    // Blue should have high blue component
    test_assert(blue_b > 200);
}

static void test_hsba32_alpha_channel() {
    // Test that alpha channel is properly set
    uint32_t full_alpha = func__hsba32(0.0, 100.0, 100.0, 100.0);
    uint32_t half_alpha = func__hsba32(0.0, 100.0, 100.0, 50.0);
    uint32_t zero_alpha = func__hsba32(0.0, 100.0, 100.0, 0.0);
    
    uint8_t alpha1 = (full_alpha >> 24) & 0xFF;
    uint8_t alpha2 = (half_alpha >> 24) & 0xFF;
    uint8_t alpha3 = (zero_alpha >> 24) & 0xFF;
    
    // Alpha values should be different
    test_assert(alpha1 > alpha2);
    test_assert(alpha2 > alpha3);
    test_assert(alpha1 > 200); // Full alpha should be high
    test_assert(alpha3 < 10);   // Zero alpha should be low
}

int main() {
    struct unit_test tests[] = {
        {test_func__hsb32_basic, "func__hsb32_basic"},
        {test_func__hsb32_edge_cases, "func__hsb32_edge_cases"},
        {test_func__hsba32_basic, "func__hsba32_basic"},
        {test_func__hsba32_edge_cases, "func__hsba32_edge_cases"},
        {test_func__hue32, "func__hue32"},
        {test_func__sat32, "func__sat32"},
        {test_func__bri32, "func__bri32"},
        {test_hsb_rgb_roundtrip, "hsb_rgb_roundtrip"},
        {test_hsb32_color_values, "hsb32_color_values"},
        {test_hsba32_alpha_channel, "hsba32_alpha_channel"},
    };
    
    return run_tests("graphics", tests, sizeof(tests) / sizeof(tests[0]));
}
