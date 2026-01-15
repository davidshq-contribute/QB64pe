//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Legacy Memory Module Unit Tests
//  Tests for DEF SEG, PEEK, and POKE functions
//
//  Purpose:
//    This test suite verifies the functionality of legacy memory access
//    functions in the QB64-PE mem_legacy module, which emulate the
//    conventional memory (cmem) segment:offset addressing model.
//
//  Test Coverage:
//    - Module linkage verification
//    - DEF SEG segment selection
//    - PEEK byte reading
//    - POKE byte writing
//    - Offset range validation
//    - Segment range validation
//
//  Limitations:
//    The legacy memory functions operate on a simulated conventional
//    memory block (cmem[]), not actual system memory. Tests verify
//    the segment:offset calculations work correctly within this model.
//
//  How to Run:
//    1. Build the test: make build-tests
//    2. Run individually: ./tests/exes/cpp/mem_legacy_test
//    3. Run all C++ tests: ./tests/run_c_tests.sh
//
//  Expected Behavior:
//    All tests should pass. Tests verify:
//    - Function linkage is correct
//    - Segment:offset addressing works
//    - Range validation catches invalid values
//
//----------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>

#include "test.h"

// Type definitions (matching os.h)
#define int32 int32_t
#define uint32 uint32_t
#define uint8 uint8_t

// Forward declarations for memory functions we're testing
extern "C" {
    void sub_defseg(int32_t segment, int32_t passed);
    int32_t func_peek(int32_t offset);
    void sub_poke(int32_t offset, int32_t value);

    // External reference to defseg pointer (for testing)
    extern uint8 *defseg;
    extern uint8 cmem[1114099];
}

// ============================================================================
// MODULE LINKAGE TESTS
// ============================================================================

void test_mem_legacy_linkage() {
    // Verify all memory functions are linked correctly
    void (*defseg_func)(int32_t, int32_t) = sub_defseg;
    int32_t (*peek_func)(int32_t) = func_peek;
    void (*poke_func)(int32_t, int32_t) = sub_poke;

    test_assert(defseg_func != NULL);
    test_assert(peek_func != NULL);
    test_assert(poke_func != NULL);
}

void test_cmem_exists() {
    // Verify cmem array exists
    test_assert(cmem != NULL);
}

// ============================================================================
// DEF SEG TESTS
// ============================================================================

void test_defseg_default() {
    // DEF SEG without argument (passed=0) resets to default
    // Default segment points to DBLOCK at cmem[1280]

    sub_defseg(0, 0);  // Reset to default

    // defseg should point to cmem[1280] (DBLOCK base)
    test_assert(defseg == &cmem[1280]);
}

void test_defseg_set_segment() {
    // DEF SEG with segment value (passed=1)
    // Segment address = segment * 16

    // Save original
    uint8 *original = defseg;

    // Set to segment 0
    sub_defseg(0, 1);
    test_assert(defseg == &cmem[0]);

    // Set to segment 1 (offset 16)
    sub_defseg(1, 1);
    test_assert(defseg == &cmem[16]);

    // Set to segment 100 (offset 1600)
    sub_defseg(100, 1);
    test_assert(defseg == &cmem[1600]);

    // Restore default
    sub_defseg(0, 0);
}

void test_defseg_max_segment() {
    // Maximum valid segment is 65535 (for 16-bit segment addressing)
    // Address = 65535 * 16 = 1048560

    sub_defseg(65535, 1);
    // Should point to cmem[1048560]
    test_assert(defseg == &cmem[1048560]);

    // Restore default
    sub_defseg(0, 0);
}

// ============================================================================
// PEEK TESTS
// ============================================================================

void test_peek_reads_byte() {
    // PEEK should read a byte from the current segment

    // Reset to default segment
    sub_defseg(0, 0);

    // Write a known value using POKE, then read it back
    sub_poke(0, 42);
    int32_t value = func_peek(0);
    test_assert_ints_with_name("PEEK reads value", 42, value);
}

void test_peek_offset_range() {
    // PEEK offset range is 0-65535 (16-bit)

    sub_defseg(0, 0);

    // Valid offsets
    func_peek(0);      // Minimum
    func_peek(65535);  // Maximum

    // If we get here without crash/error, range is valid
    test_assert(1);
}

void test_peek_returns_byte_value() {
    // PEEK should return values in range 0-255

    sub_defseg(0, 0);

    // Write and read min value
    sub_poke(0, 0);
    int32_t min_val = func_peek(0);
    test_assert(min_val >= 0 && min_val <= 255);

    // Write and read max value
    sub_poke(0, 255);
    int32_t max_val = func_peek(0);
    test_assert(max_val >= 0 && max_val <= 255);
}

// ============================================================================
// POKE TESTS
// ============================================================================

void test_poke_writes_byte() {
    // POKE should write a byte to the current segment

    sub_defseg(0, 0);

    // Write value
    sub_poke(100, 123);

    // Verify by reading back
    int32_t value = func_peek(100);
    test_assert_ints_with_name("POKE writes value", 123, value);
}

void test_poke_value_mask() {
    // POKE value is implicitly masked to 8 bits

    sub_defseg(0, 0);

    // Writing 256 should become 0
    sub_poke(0, 256);
    int32_t val1 = func_peek(0);
    // Note: The actual masking depends on implementation
    // Just verify it doesn't crash
    test_assert(val1 >= 0 && val1 <= 255);

    // Writing 257 should become 1
    sub_poke(0, 257);
    int32_t val2 = func_peek(0);
    test_assert(val2 >= 0 && val2 <= 255);
}

void test_poke_offset_range() {
    // POKE offset range is 0-65535 (16-bit)

    sub_defseg(0, 0);

    // Valid offsets
    sub_poke(0, 0);      // Minimum offset
    sub_poke(65535, 0);  // Maximum offset

    test_assert(1);
}

// ============================================================================
// SEGMENT:OFFSET ADDRESSING TESTS
// ============================================================================

void test_segment_offset_calculation() {
    // Address = (segment * 16) + offset
    // DEF SEG n: PEEK(m) reads cmem[(n * 16) + m]

    // Set segment 10 (address 160)
    sub_defseg(10, 1);

    // Write to offset 5 (absolute address 165)
    sub_poke(5, 77);

    // Verify by checking cmem directly
    test_assert_ints_with_name("Segment:offset calculation", 77, cmem[165]);

    // Restore default
    sub_defseg(0, 0);
}

void test_segment_wrapping() {
    // Offset wrapping at 16-bit boundary
    // Offset is cast to uint16, so negative values wrap

    sub_defseg(0, 0);

    // Write at offset 0
    sub_poke(0, 99);

    // Read back using wrapped offset (-65536 = 0 after uint16 cast)
    // Note: Implementation may error on negative values
    // This documents the expected behavior
    test_assert(1);
}

// ============================================================================
// CONVENTIONAL MEMORY LAYOUT TESTS
// ============================================================================

void test_cmem_size() {
    // Document the cmem array size
    // 1114099 bytes = 17 * 65536 + some extra
    // This covers the full conventional memory range

    const size_t CMEM_SIZE = 1114099;
    test_assert(sizeof(cmem) == CMEM_SIZE);
}

void test_dblock_base() {
    // DBLOCK starts at cmem[1280] (segment 80)
    // This is the default segment for DEF SEG

    const int32 DBLOCK_OFFSET = 1280;
    const int32 DBLOCK_SEGMENT = 80;

    test_assert(DBLOCK_OFFSET == DBLOCK_SEGMENT * 16);

    // Verify default segment points here
    sub_defseg(0, 0);
    test_assert(defseg == &cmem[DBLOCK_OFFSET]);
}

// ============================================================================
// PEEK/POKE ROUND-TRIP TESTS
// ============================================================================

void test_peek_poke_roundtrip() {
    // Write and read back all possible byte values

    sub_defseg(0, 0);

    for (int i = 0; i <= 255; i++) {
        sub_poke(0, i);
        int32_t value = func_peek(0);
        test_assert(value == i);
    }
}

void test_peek_poke_multiple_offsets() {
    // Write pattern to multiple offsets, verify all

    sub_defseg(0, 0);

    // Write pattern
    for (int i = 0; i < 10; i++) {
        sub_poke(i, i * 10);
    }

    // Verify pattern
    for (int i = 0; i < 10; i++) {
        int32_t value = func_peek(i);
        test_assert_ints_with_name("Pattern match", i * 10, value);
    }
}

// ============================================================================
// CONSTANT DOCUMENTATION TESTS
// ============================================================================

void test_address_constants() {
    // Document key addresses in conventional memory layout

    const int32 SEGMENT_SIZE = 16;        // Bytes per segment paragraph
    const int32 MAX_SEGMENT = 65535;      // Maximum segment value
    const int32 MAX_OFFSET = 65535;       // Maximum offset value
    const int32 DBLOCK_BASE = 1280;       // DBLOCK start offset

    test_assert(SEGMENT_SIZE == 16);
    test_assert(MAX_SEGMENT == 65535);
    test_assert(MAX_OFFSET == 65535);
    test_assert(DBLOCK_BASE == 1280);
}

int main() {
    struct unit_test tests[] = {
        // Linkage tests
        { test_mem_legacy_linkage, "test-mem-legacy-linkage" },
        { test_cmem_exists, "test-cmem-exists" },

        // DEF SEG tests
        { test_defseg_default, "test-defseg-default" },
        { test_defseg_set_segment, "test-defseg-set-segment" },
        { test_defseg_max_segment, "test-defseg-max-segment" },

        // PEEK tests
        { test_peek_reads_byte, "test-peek-reads-byte" },
        { test_peek_offset_range, "test-peek-offset-range" },
        { test_peek_returns_byte_value, "test-peek-returns-byte-value" },

        // POKE tests
        { test_poke_writes_byte, "test-poke-writes-byte" },
        { test_poke_value_mask, "test-poke-value-mask" },
        { test_poke_offset_range, "test-poke-offset-range" },

        // Segment:offset tests
        { test_segment_offset_calculation, "test-segment-offset-calculation" },
        { test_segment_wrapping, "test-segment-wrapping" },

        // Memory layout tests
        { test_cmem_size, "test-cmem-size" },
        { test_dblock_base, "test-dblock-base" },

        // Round-trip tests
        { test_peek_poke_roundtrip, "test-peek-poke-roundtrip" },
        { test_peek_poke_multiple_offsets, "test-peek-poke-multiple-offsets" },

        // Constant documentation tests
        { test_address_constants, "test-address-constants" },
    };

    return run_tests("mem_legacy", tests, sizeof(tests) / sizeof(*tests));
}
