// Unit tests for bit operations module
// Tests bit manipulation functions including getbits, setbits, and bitwise operations

#include "test.h"
#include "bitops.h"
#include <string.h>

static void test_getubits() {
    uint8_t buffer[16];
    memset(buffer, 0, sizeof(buffer));
    
    // Set some bits manually
    buffer[0] = 0xAA; // 10101010
    buffer[1] = 0x55; // 01010101
    
    // Test reading 1-bit values
    uint64_t val1 = getubits(1, buffer, 0);
    test_assert_ints(1, val1); // First bit should be 1
    
    uint64_t val2 = getubits(1, buffer, 1);
    test_assert_ints(0, val2); // Second bit should be 0
    
    // Test reading 4-bit values
    uint64_t val4 = getubits(4, buffer, 0);
    test_assert_ints(10, val4); // First 4 bits: 1010 = 10
    
    // Test reading 8-bit values
    uint64_t val8 = getubits(8, buffer, 0);
    test_assert_ints(0xAA, val8);
}

static void test_getbits() {
    uint8_t buffer[16];
    memset(buffer, 0, sizeof(buffer));
    
    // Set a signed value (negative in 4-bit representation)
    buffer[0] = 0xF0; // 11110000 - negative in 4-bit
    
    // Test reading signed 4-bit value
    int64_t val = getbits(4, buffer, 0);
    // Should be sign-extended negative value
    test_assert(val < 0);
}

static void test_setbits() {
    uint8_t buffer[16];
    memset(buffer, 0, sizeof(buffer));
    
    // Set a 4-bit value
    setbits(4, buffer, 0, 10); // 1010
    uint64_t val = getubits(4, buffer, 0);
    test_assert_ints(10, val);
    
    // Set another 4-bit value
    setbits(4, buffer, 1, 5); // 0101
    val = getubits(4, buffer, 1);
    test_assert_ints(5, val);
    
    // Verify both values are in the buffer
    test_assert_ints(0xA5, buffer[0]); // 1010 0101
}

static void test_bit_operations_rol() {
    uint64_t value = 0x123456789ABCDEF0;
    uint64_t result = func__rol(value, 4);
    // Rotate left by 4 bits
    test_assert(result != value);
    test_assert(result == ((value << 4) | (value >> 60)));
}

static void test_bit_operations_ror() {
    uint64_t value = 0x123456789ABCDEF0;
    uint64_t result = func__ror(value, 4);
    // Rotate right by 4 bits
    test_assert(result != value);
    test_assert(result == ((value >> 4) | (value << 60)));
}

static void test_bit_operations_shl() {
    uint64_t value = 0x1;
    uint64_t result = func__shl(value, 4);
    test_assert_ints(16, result); // 1 << 4 = 16
}

static void test_bit_operations_shr() {
    uint64_t value = 16;
    uint64_t result = func__shr(value, 4);
    test_assert_ints(1, result); // 16 >> 4 = 1
}

static void test_readbit() {
    uint64_t value = 0x10; // Bit 4 is set
    int64_t result = func__readbit(value, 4);
    test_assert_ints(1, result); // Bit 4 should be 1
    
    result = func__readbit(value, 3);
    test_assert_ints(0, result); // Bit 3 should be 0
}

static void test_setbit() {
    uint64_t value = 0;
    uint64_t result = func__setbit(value, 5);
    test_assert_ints(32, result); // 2^5 = 32
}

static void test_resetbit() {
    uint64_t value = 0xFF;
    uint64_t result = func__resetbit(value, 3);
    test_assert_ints(0xF7, result); // 0xFF & ~(1<<3) = 0xF7
}

static void test_togglebit() {
    uint64_t value = 0x10;
    uint64_t result = func__togglebit(value, 4);
    test_assert_ints(0, result); // Toggle bit 4: 0x10 ^ 0x10 = 0
    
    result = func__togglebit(value, 3);
    test_assert_ints(0x18, result); // Toggle bit 3: 0x10 ^ 0x08 = 0x18
}

int main() {
    struct unit_test tests[] = {
        {test_getubits, "getubits"},
        {test_getbits, "getbits"},
        {test_setbits, "setbits"},
        {test_bit_operations_rol, "rol"},
        {test_bit_operations_ror, "ror"},
        {test_bit_operations_shl, "shl"},
        {test_bit_operations_shr, "shr"},
        {test_readbit, "readbit"},
        {test_setbit, "setbit"},
        {test_resetbit, "resetbit"},
        {test_togglebit, "togglebit"},
    };
    
    return run_tests("bitops", tests, sizeof(tests) / sizeof(tests[0]));
}
