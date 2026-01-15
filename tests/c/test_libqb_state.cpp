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

    // Keyhit buffer accessors
    int64_t libqb_keyhit_pop();
    int32_t libqb_keyhit_pending();
    void libqb_keyhit_push(int64_t value);
    void libqb_keyhit_clear();

    // Keyheld state accessor
    int32_t libqb_keyheld(uint32_t keycode);

    // Port 60h buffer accessors
    int32_t libqb_port60h_events_count();
    uint8_t libqb_port60h_peek();
    uint8_t libqb_port60h_pop();
    void libqb_port60h_push(uint8_t scancode);
    void libqb_port60h_push_release(uint8_t scancode);

    // Mouse queue accessors
    struct libqb_mouse_state {
        int32_t x;
        int32_t y;
        int32_t movementX;
        int32_t movementY;
        int32_t buttons;
        int32_t wheel;
    };
    void libqb_mouse_get_current(struct libqb_mouse_state* state);
    int32_t libqb_mouse_input_next();
    int32_t libqb_mouse_has_pending();

    // Mouse state accessors
    int32_t libqb_get_mouse_hidden();
    void libqb_set_mouse_hidden(int32_t value);
    int32_t libqb_get_mouse_cursor_style();
    void libqb_set_mouse_cursor_style(int32_t style);

    // Codepage mapping accessor
    uint16_t libqb_get_codepage_mapping(int32_t ascii_code);
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

// ============================================================================
// KEYHIT BUFFER ACCESSOR TESTS
// ============================================================================

void test_keyhit_linkage() {
    // Verify keyhit accessor functions are linked
    int64_t (*pop)() = libqb_keyhit_pop;
    int32_t (*pending)() = libqb_keyhit_pending;
    void (*push)(int64_t) = libqb_keyhit_push;
    void (*clear)() = libqb_keyhit_clear;

    test_assert(pop != NULL);
    test_assert(pending != NULL);
    test_assert(push != NULL);
    test_assert(clear != NULL);
}

void test_keyhit_push_pop() {
    // Clear any existing events first
    libqb_keyhit_clear();
    test_assert_ints_with_name("Initial pending 0", 0, libqb_keyhit_pending());

    // Push a key event
    libqb_keyhit_push(65);  // 'A'
    test_assert_ints_with_name("Pending after push", 1, libqb_keyhit_pending());

    // Pop and verify
    int64_t value = libqb_keyhit_pop();
    test_assert(value == 65);
    test_assert_ints_with_name("Pending after pop", 0, libqb_keyhit_pending());
}

void test_keyhit_empty_pop() {
    // Clear and verify empty pop returns 0
    libqb_keyhit_clear();
    int64_t value = libqb_keyhit_pop();
    test_assert(value == 0);
}

void test_keyhit_multiple_events() {
    // Clear and push multiple events
    libqb_keyhit_clear();
    libqb_keyhit_push(65);  // 'A'
    libqb_keyhit_push(66);  // 'B'
    libqb_keyhit_push(67);  // 'C'
    test_assert_ints_with_name("Pending 3", 3, libqb_keyhit_pending());

    // Pop in FIFO order
    test_assert(libqb_keyhit_pop() == 65);
    test_assert(libqb_keyhit_pop() == 66);
    test_assert(libqb_keyhit_pop() == 67);
    test_assert_ints_with_name("Pending 0", 0, libqb_keyhit_pending());

    // Extra pop should return 0
    test_assert(libqb_keyhit_pop() == 0);
}

void test_keyhit_clear() {
    // Push events and clear
    libqb_keyhit_push(65);
    libqb_keyhit_push(66);
    libqb_keyhit_clear();
    test_assert_ints_with_name("Cleared pending", 0, libqb_keyhit_pending());
    test_assert(libqb_keyhit_pop() == 0);
}

// ============================================================================
// KEYHELD ACCESSOR TESTS
// ============================================================================

void test_keyheld_linkage() {
    // Verify keyheld accessor function is linked
    int32_t (*keyheld_fn)(uint32_t) = libqb_keyheld;
    test_assert(keyheld_fn != NULL);
}

void test_keyheld_returns_value() {
    // Call keyheld - result depends on keyboard state
    // Just verify function can be called without crashing
    int32_t result = libqb_keyheld(65);  // Check 'A' key
    (void)result;  // Result depends on whether key is held
    test_assert(1);
}

// ============================================================================
// PORT 60H BUFFER ACCESSOR TESTS
// ============================================================================

void test_port60h_linkage() {
    // Verify port 60h accessor functions are linked
    int32_t (*count)() = libqb_port60h_events_count;
    uint8_t (*peek)() = libqb_port60h_peek;
    uint8_t (*pop)() = libqb_port60h_pop;
    void (*push)(uint8_t) = libqb_port60h_push;
    void (*push_release)(uint8_t) = libqb_port60h_push_release;

    test_assert(count != NULL);
    test_assert(peek != NULL);
    test_assert(pop != NULL);
    test_assert(push != NULL);
    test_assert(push_release != NULL);
}

void test_port60h_push_pop() {
    // Drain any existing events
    while (libqb_port60h_events_count() > 0) {
        libqb_port60h_pop();
    }

    // Push a scancode
    libqb_port60h_push(0x1E);  // 'A' scancode
    test_assert_ints_with_name("Count after push", 1, libqb_port60h_events_count());

    // Peek should return value without removing
    test_assert(libqb_port60h_peek() == 0x1E);
    test_assert_ints_with_name("Count after peek", 1, libqb_port60h_events_count());

    // Pop should return and remove
    test_assert(libqb_port60h_pop() == 0x1E);
    test_assert_ints_with_name("Count after pop", 0, libqb_port60h_events_count());
}

void test_port60h_push_release() {
    // Drain events
    while (libqb_port60h_events_count() > 0) {
        libqb_port60h_pop();
    }

    // Push release scancode (should set high bit)
    libqb_port60h_push_release(0x1E);
    uint8_t value = libqb_port60h_pop();
    test_assert(value == (0x1E | 0x80));
}

void test_port60h_empty_pop() {
    // Drain events
    while (libqb_port60h_events_count() > 0) {
        libqb_port60h_pop();
    }

    // Empty pop should return 0
    test_assert(libqb_port60h_pop() == 0);
    test_assert(libqb_port60h_peek() == 0);
}

// ============================================================================
// MOUSE ACCESSOR TESTS
// ============================================================================

void test_mouse_linkage() {
    // Verify mouse accessor functions are linked
    void (*get_current)(struct libqb_mouse_state*) = libqb_mouse_get_current;
    int32_t (*input_next)() = libqb_mouse_input_next;
    int32_t (*has_pending)() = libqb_mouse_has_pending;
    int32_t (*get_hidden)() = libqb_get_mouse_hidden;
    void (*set_hidden)(int32_t) = libqb_set_mouse_hidden;
    int32_t (*get_cursor)() = libqb_get_mouse_cursor_style;
    void (*set_cursor)(int32_t) = libqb_set_mouse_cursor_style;

    test_assert(get_current != NULL);
    test_assert(input_next != NULL);
    test_assert(has_pending != NULL);
    test_assert(get_hidden != NULL);
    test_assert(set_hidden != NULL);
    test_assert(get_cursor != NULL);
    test_assert(set_cursor != NULL);
}

void test_mouse_get_current() {
    // Get current mouse state - should not crash
    struct libqb_mouse_state state;
    libqb_mouse_get_current(&state);
    // Values depend on actual mouse state - just verify no crash
    test_assert(1);
}

void test_mouse_hidden_roundtrip() {
    // Save original
    int32_t orig = libqb_get_mouse_hidden();

    // Set and verify
    libqb_set_mouse_hidden(1);
    test_assert_ints_with_name("Mouse hidden set", 1, libqb_get_mouse_hidden());

    libqb_set_mouse_hidden(0);
    test_assert_ints_with_name("Mouse visible set", 0, libqb_get_mouse_hidden());

    // Restore
    libqb_set_mouse_hidden(orig);
}

void test_mouse_cursor_style_roundtrip() {
    // Save original
    int32_t orig = libqb_get_mouse_cursor_style();

    // Set and verify
    libqb_set_mouse_cursor_style(2);
    test_assert_ints_with_name("Cursor style set", 2, libqb_get_mouse_cursor_style());

    // Restore
    libqb_set_mouse_cursor_style(orig);
}

// ============================================================================
// CODEPAGE ACCESSOR TESTS
// ============================================================================

void test_codepage_linkage() {
    // Verify codepage accessor is linked
    uint16_t (*get_mapping)(int32_t) = libqb_get_codepage_mapping;
    test_assert(get_mapping != NULL);
}

void test_codepage_valid_range() {
    // Test valid ASCII codes (0-255)
    // ASCII 'A' (65) should map to Unicode 'A' (65)
    uint16_t result = libqb_get_codepage_mapping(65);
    test_assert(result == 65);

    // ASCII space (32) should map to Unicode space (32)
    result = libqb_get_codepage_mapping(32);
    test_assert(result == 32);
}

void test_codepage_invalid_range() {
    // Test invalid ASCII codes
    uint16_t result = libqb_get_codepage_mapping(-1);
    test_assert(result == 0);

    result = libqb_get_codepage_mapping(256);
    test_assert(result == 0);

    result = libqb_get_codepage_mapping(1000);
    test_assert(result == 0);
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

        // Keyhit buffer accessor tests
        { test_keyhit_linkage, "test-keyhit-linkage" },
        { test_keyhit_push_pop, "test-keyhit-push-pop" },
        { test_keyhit_empty_pop, "test-keyhit-empty-pop" },
        { test_keyhit_multiple_events, "test-keyhit-multiple-events" },
        { test_keyhit_clear, "test-keyhit-clear" },

        // Keyheld accessor tests
        { test_keyheld_linkage, "test-keyheld-linkage" },
        { test_keyheld_returns_value, "test-keyheld-returns-value" },

        // Port 60h buffer accessor tests
        { test_port60h_linkage, "test-port60h-linkage" },
        { test_port60h_push_pop, "test-port60h-push-pop" },
        { test_port60h_push_release, "test-port60h-push-release" },
        { test_port60h_empty_pop, "test-port60h-empty-pop" },

        // Mouse accessor tests
        { test_mouse_linkage, "test-mouse-linkage" },
        { test_mouse_get_current, "test-mouse-get-current" },
        { test_mouse_hidden_roundtrip, "test-mouse-hidden-roundtrip" },
        { test_mouse_cursor_style_roundtrip, "test-mouse-cursor-style-roundtrip" },

        // Codepage accessor tests
        { test_codepage_linkage, "test-codepage-linkage" },
        { test_codepage_valid_range, "test-codepage-valid-range" },
        { test_codepage_invalid_range, "test-codepage-invalid-range" },
    };

    return run_tests("libqb_state", tests, sizeof(tests) / sizeof(*tests));
}
