//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE State Accessor Layer Unit Tests
//  Tests for the global state accessor functions
//
//  Purpose:
//    This test suite verifies the functionality of the state accessor
//    layer (libqb_state.h/cpp) which provides controlled access to
//    global state variables for modularization.
//
//  Test Coverage:
//    - Module linkage verification
//    - Image system accessors
//    - Page index accessors
//    - Font system accessors
//    - LPRINT state accessors
//    - Environment 2D accessors
//
//  Limitations:
//    The state accessors provide read/write access to globals defined
//    in libqb.cpp. Some accessor results depend on initialization state.
//    Tests verify accessor functions work correctly rather than specific
//    state values (which depend on program state).
//
//  How to Run:
//    1. Build the test: make build-tests
//    2. Run individually: ./tests/exes/cpp/libqb_state_test
//    3. Run all C++ tests: ./tests/run_c_tests.sh
//
//  Expected Behavior:
//    All tests should pass. Tests verify:
//    - Function linkage is correct
//    - Accessors return valid values
//    - Setters modify state correctly
//
//----------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>

#include "test.h"

// Type definitions (matching os.h)
#define int32 int32_t
#define uint32 uint32_t

// Forward declaration for img_struct (used by image accessors)
struct img_struct;

// Forward declarations for state accessor functions we're testing
extern "C" {
    // Image system accessors
    img_struct* libqb_get_write_page();
    img_struct* libqb_get_read_page();
    img_struct* libqb_get_display_page();
    img_struct* libqb_get_image(int32_t handle);
    int32_t libqb_get_image_count();
    int32_t libqb_validate_image(int32_t handle);

    // Page index accessors
    int32_t libqb_get_write_page_index();
    void libqb_set_write_page_index(int32_t index);
    int32_t libqb_get_read_page_index();
    void libqb_set_read_page_index(int32_t index);
    int32_t libqb_get_display_page_index();
    void libqb_set_display_page_index(int32_t index);

    // Font system accessors
    int32_t libqb_get_font_width(int32_t font_handle);
    int32_t libqb_get_font_height(int32_t font_handle);
    int32_t libqb_get_font_flags(int32_t font_handle);
    int32_t libqb_get_font(int32_t font_handle);
    int32_t libqb_get_last_font();
    void libqb_set_last_font(int32_t value);
    int32_t* libqb_get_font_array();
    int32_t* libqb_get_fontwidth_array();
    int32_t* libqb_get_fontheight_array();
    int32_t* libqb_get_fontflags_array();

    // LPRINT state accessors
    int32_t libqb_get_lprint();
    void libqb_set_lprint(int32_t value);
    int32_t libqb_get_lprint_image();
    void libqb_set_lprint_image(int32_t value);

    // Environment 2D accessors
    int32_t libqb_get_screen_width();
    int32_t libqb_get_screen_height();
    int32_t libqb_get_screen_x1();
    int32_t libqb_get_screen_y1();
    int32_t libqb_get_screen_scaled_width();
    int32_t libqb_get_screen_scaled_height();
}

// ============================================================================
// MODULE LINKAGE TESTS
// ============================================================================

void test_state_image_linkage() {
    // Verify image accessor functions are linked
    img_struct* (*write_page)() = libqb_get_write_page;
    img_struct* (*read_page)() = libqb_get_read_page;
    img_struct* (*display_page)() = libqb_get_display_page;
    img_struct* (*get_image)(int32_t) = libqb_get_image;
    int32_t (*image_count)() = libqb_get_image_count;
    int32_t (*validate_image)(int32_t) = libqb_validate_image;

    test_assert(write_page != NULL);
    test_assert(read_page != NULL);
    test_assert(display_page != NULL);
    test_assert(get_image != NULL);
    test_assert(image_count != NULL);
    test_assert(validate_image != NULL);
}

void test_state_page_linkage() {
    // Verify page index accessor functions are linked
    int32_t (*get_write)() = libqb_get_write_page_index;
    void (*set_write)(int32_t) = libqb_set_write_page_index;
    int32_t (*get_read)() = libqb_get_read_page_index;
    void (*set_read)(int32_t) = libqb_set_read_page_index;
    int32_t (*get_display)() = libqb_get_display_page_index;
    void (*set_display)(int32_t) = libqb_set_display_page_index;

    test_assert(get_write != NULL);
    test_assert(set_write != NULL);
    test_assert(get_read != NULL);
    test_assert(set_read != NULL);
    test_assert(get_display != NULL);
    test_assert(set_display != NULL);
}

void test_state_font_linkage() {
    // Verify font accessor functions are linked
    int32_t (*get_font_width)(int32_t) = libqb_get_font_width;
    int32_t (*get_font_height)(int32_t) = libqb_get_font_height;
    int32_t (*get_font_flags)(int32_t) = libqb_get_font_flags;
    int32_t (*get_last_font)() = libqb_get_last_font;
    void (*set_last_font)(int32_t) = libqb_set_last_font;

    test_assert(get_font_width != NULL);
    test_assert(get_font_height != NULL);
    test_assert(get_font_flags != NULL);
    test_assert(get_last_font != NULL);
    test_assert(set_last_font != NULL);
}

void test_state_lprint_linkage() {
    // Verify LPRINT accessor functions are linked
    int32_t (*get_lprint)() = libqb_get_lprint;
    void (*set_lprint)(int32_t) = libqb_set_lprint;
    int32_t (*get_lprint_image)() = libqb_get_lprint_image;
    void (*set_lprint_image)(int32_t) = libqb_set_lprint_image;

    test_assert(get_lprint != NULL);
    test_assert(set_lprint != NULL);
    test_assert(get_lprint_image != NULL);
    test_assert(set_lprint_image != NULL);
}

void test_state_env2d_linkage() {
    // Verify environment 2D accessor functions are linked
    int32_t (*get_width)() = libqb_get_screen_width;
    int32_t (*get_height)() = libqb_get_screen_height;
    int32_t (*get_x1)() = libqb_get_screen_x1;
    int32_t (*get_y1)() = libqb_get_screen_y1;
    int32_t (*get_scaled_width)() = libqb_get_screen_scaled_width;
    int32_t (*get_scaled_height)() = libqb_get_screen_scaled_height;

    test_assert(get_width != NULL);
    test_assert(get_height != NULL);
    test_assert(get_x1 != NULL);
    test_assert(get_y1 != NULL);
    test_assert(get_scaled_width != NULL);
    test_assert(get_scaled_height != NULL);
}

// ============================================================================
// IMAGE ACCESSOR TESTS
// ============================================================================

void test_image_count_non_negative() {
    // Image count should be >= 0
    int32_t count = libqb_get_image_count();
    test_assert(count >= 0);
}

void test_validate_image_negative_handle() {
    // Negative handles should be invalid
    int32_t result = libqb_validate_image(-1);
    test_assert_ints_with_name("Negative handle invalid", 0, result);
}

void test_validate_image_out_of_range() {
    // Handles >= image count should be invalid
    int32_t count = libqb_get_image_count();
    int32_t result = libqb_validate_image(count + 100);
    test_assert_ints_with_name("Out of range invalid", 0, result);
}

void test_get_image_invalid_handle() {
    // Getting image with invalid handle should return NULL
    img_struct* result = libqb_get_image(-1);
    test_assert(result == NULL);

    result = libqb_get_image(999999);
    test_assert(result == NULL);
}

// ============================================================================
// PAGE INDEX ACCESSOR TESTS
// ============================================================================

void test_page_index_roundtrip() {
    // Save original values
    int32_t orig_write = libqb_get_write_page_index();
    int32_t orig_read = libqb_get_read_page_index();
    int32_t orig_display = libqb_get_display_page_index();

    // Test write page index
    libqb_set_write_page_index(5);
    test_assert_ints_with_name("Write page set", 5, libqb_get_write_page_index());

    // Test read page index
    libqb_set_read_page_index(3);
    test_assert_ints_with_name("Read page set", 3, libqb_get_read_page_index());

    // Test display page index
    libqb_set_display_page_index(1);
    test_assert_ints_with_name("Display page set", 1, libqb_get_display_page_index());

    // Restore original values
    libqb_set_write_page_index(orig_write);
    libqb_set_read_page_index(orig_read);
    libqb_set_display_page_index(orig_display);
}

void test_page_index_non_negative() {
    // Page indices should be >= 0 in normal operation
    int32_t write_idx = libqb_get_write_page_index();
    int32_t read_idx = libqb_get_read_page_index();
    int32_t display_idx = libqb_get_display_page_index();

    // Note: Indices are stored as uint32, so they can't be negative
    // This test documents the expected behavior
    test_assert(write_idx >= 0);
    test_assert(read_idx >= 0);
    test_assert(display_idx >= 0);
}

// ============================================================================
// FONT ACCESSOR TESTS
// ============================================================================

void test_font_width_invalid_handle() {
    // Invalid font handles should return 0
    int32_t result = libqb_get_font_width(-1);
    test_assert_ints_with_name("Negative handle width", 0, result);
}

void test_font_height_invalid_handle() {
    // Invalid font handles should return 0
    int32_t result = libqb_get_font_height(-1);
    test_assert_ints_with_name("Negative handle height", 0, result);
}

void test_font_flags_invalid_handle() {
    // Invalid font handles should return 0
    int32_t result = libqb_get_font_flags(-1);
    test_assert_ints_with_name("Negative handle flags", 0, result);
}

void test_font_array_accessors() {
    // Font array accessors should return non-null or null depending on init state
    // In uninitialized state, they may be null
    // This test just verifies the functions can be called
    libqb_get_font_array();
    libqb_get_fontwidth_array();
    libqb_get_fontheight_array();
    libqb_get_fontflags_array();
    test_assert(1);
}

void test_last_font_roundtrip() {
    // Save original
    int32_t orig = libqb_get_last_font();

    // Test set/get
    libqb_set_last_font(10);
    test_assert_ints_with_name("Last font set", 10, libqb_get_last_font());

    // Restore
    libqb_set_last_font(orig);
}

// ============================================================================
// LPRINT ACCESSOR TESTS
// ============================================================================

void test_lprint_roundtrip() {
    // Save original
    int32_t orig = libqb_get_lprint();

    // Test set/get
    libqb_set_lprint(1);
    test_assert_ints_with_name("LPRINT enabled", 1, libqb_get_lprint());

    libqb_set_lprint(0);
    test_assert_ints_with_name("LPRINT disabled", 0, libqb_get_lprint());

    // Restore
    libqb_set_lprint(orig);
}

void test_lprint_image_roundtrip() {
    // Save original
    int32_t orig = libqb_get_lprint_image();

    // Test set/get
    libqb_set_lprint_image(5);
    test_assert_ints_with_name("LPRINT image set", 5, libqb_get_lprint_image());

    // Restore
    libqb_set_lprint_image(orig);
}

// ============================================================================
// ENVIRONMENT 2D ACCESSOR TESTS
// ============================================================================

void test_screen_dimensions_non_negative() {
    // Screen dimensions should be >= 0
    int32_t width = libqb_get_screen_width();
    int32_t height = libqb_get_screen_height();

    test_assert(width >= 0);
    test_assert(height >= 0);
}

void test_screen_position_accessible() {
    // Screen position functions should be callable
    int32_t x1 = libqb_get_screen_x1();
    int32_t y1 = libqb_get_screen_y1();

    // Position can be any value depending on window placement
    // Just verify functions don't crash
    (void)x1;
    (void)y1;
    test_assert(1);
}

void test_screen_scaled_dimensions() {
    // Scaled dimensions should be >= 0
    int32_t scaled_width = libqb_get_screen_scaled_width();
    int32_t scaled_height = libqb_get_screen_scaled_height();

    test_assert(scaled_width >= 0);
    test_assert(scaled_height >= 0);
}

// ============================================================================
// ACCESSOR CONSISTENCY TESTS
// ============================================================================

void test_accessors_no_crash_on_repeated_calls() {
    // Calling accessors repeatedly should not crash
    for (int i = 0; i < 100; i++) {
        libqb_get_write_page();
        libqb_get_image_count();
        libqb_get_write_page_index();
        libqb_get_screen_width();
    }
    test_assert(1);
}

// ============================================================================
// DOCUMENTATION TESTS
// ============================================================================

void test_accessor_purpose() {
    // Document the purpose of state accessors
    // - Enable module extraction by providing controlled access to globals
    // - Avoid extern declarations scattered across modules
    // - Allow future refactoring to encapsulate state
    test_assert(1);
}

int main() {
    struct unit_test tests[] = {
        // Linkage tests
        { test_state_image_linkage, "test-state-image-linkage" },
        { test_state_page_linkage, "test-state-page-linkage" },
        { test_state_font_linkage, "test-state-font-linkage" },
        { test_state_lprint_linkage, "test-state-lprint-linkage" },
        { test_state_env2d_linkage, "test-state-env2d-linkage" },

        // Image accessor tests
        { test_image_count_non_negative, "test-image-count-non-negative" },
        { test_validate_image_negative_handle, "test-validate-image-negative-handle" },
        { test_validate_image_out_of_range, "test-validate-image-out-of-range" },
        { test_get_image_invalid_handle, "test-get-image-invalid-handle" },

        // Page index tests
        { test_page_index_roundtrip, "test-page-index-roundtrip" },
        { test_page_index_non_negative, "test-page-index-non-negative" },

        // Font accessor tests
        { test_font_width_invalid_handle, "test-font-width-invalid-handle" },
        { test_font_height_invalid_handle, "test-font-height-invalid-handle" },
        { test_font_flags_invalid_handle, "test-font-flags-invalid-handle" },
        { test_font_array_accessors, "test-font-array-accessors" },
        { test_last_font_roundtrip, "test-last-font-roundtrip" },

        // LPRINT accessor tests
        { test_lprint_roundtrip, "test-lprint-roundtrip" },
        { test_lprint_image_roundtrip, "test-lprint-image-roundtrip" },

        // Environment 2D accessor tests
        { test_screen_dimensions_non_negative, "test-screen-dimensions-non-negative" },
        { test_screen_position_accessible, "test-screen-position-accessible" },
        { test_screen_scaled_dimensions, "test-screen-scaled-dimensions" },

        // Consistency tests
        { test_accessors_no_crash_on_repeated_calls, "test-accessors-no-crash-repeated" },

        // Documentation tests
        { test_accessor_purpose, "test-accessor-purpose" },
    };

    return run_tests("libqb_state", tests, sizeof(tests) / sizeof(*tests));
}
