#pragma once

#include "libqb-common.h"
#include <limits.h>
#include <stdint.h>

/**
 * @file bitops.h
 * @brief Bit manipulation functions for QB64-PE
 * 
 * This header provides functions for reading, writing, and manipulating bits
 * in memory, as well as bitwise rotation and shift operations.
 */

/**
 * @brief Gets an unsigned integer value from a bit field
 * @param bsize Size of the integer type in bits (8, 16, 32, or 64)
 * @param base Base pointer to the memory containing the bit field
 * @param i Index/offset of the value to read
 * @return Unsigned integer value read from the bit field
 * @note Reads a value of the specified bit size from memory at the given index
 */
uint64_t getubits(uint32_t bsize, uint8_t *base, intptr_t i);

/**
 * @brief Gets a signed integer value from a bit field
 * @param bsize Size of the integer type in bits (8, 16, 32, or 64)
 * @param base Base pointer to the memory containing the bit field
 * @param i Index/offset of the value to read
 * @return Signed integer value read from the bit field
 * @note Reads a signed value of the specified bit size from memory at the given index
 */
int64_t getbits(uint32_t bsize, uint8_t *base, intptr_t i);

/**
 * @brief Sets a value in a bit field
 * @param bsize Size of the integer type in bits (8, 16, 32, or 64)
 * @param base Base pointer to the memory containing the bit field
 * @param i Index/offset of the value to write
 * @param val Value to write to the bit field
 * @note Writes a value of the specified bit size to memory at the given index
 */
void setbits(uint32_t bsize, uint8_t *base, intptr_t i, int64_t val);

/**
 * @brief Rotates a value left (QB64 _ROL function)
 * @tparam T Integer type to rotate
 * @param value Value to rotate
 * @param count Number of bits to rotate left
 * @return Value rotated left by count bits
 * @note Performs a circular left rotation. Bits shifted out on the left wrap around to the right.
 */
template <typename T> static inline constexpr T func__rol(T value, unsigned int count) {
    const unsigned int mask = CHAR_BIT * sizeof(T) - 1;
    count &= mask;
    return (value << count) | (value >> (-count & mask));
}

/**
 * @brief Rotates a value right (QB64 _ROR function)
 * @tparam T Integer type to rotate
 * @param value Value to rotate
 * @param count Number of bits to rotate right
 * @return Value rotated right by count bits
 * @note Performs a circular right rotation. Bits shifted out on the right wrap around to the left.
 */
template <typename T> static inline constexpr T func__ror(T value, unsigned int count) {
    const unsigned int mask = CHAR_BIT * sizeof(T) - 1;
    count &= mask;
    return (value >> count) | (value << (-count & mask));
}

/**
 * @brief Shifts a value left (QB64 _SHL function)
 * @param a1 Value to shift
 * @param b1 Number of bits to shift left
 * @return Value shifted left by b1 bits
 * @note Performs a logical left shift. Zeros are shifted in from the right.
 */
static inline constexpr uint64_t func__shl(uint64_t a1, int b1) {
    return a1 << b1;
}

/**
 * @brief Shifts a value right (QB64 _SHR function)
 * @param a1 Value to shift
 * @param b1 Number of bits to shift right
 * @return Value shifted right by b1 bits
 * @note Performs a logical right shift. Zeros are shifted in from the left.
 */
static inline constexpr uint64_t func__shr(uint64_t a1, int b1) {
    return a1 >> b1;
}

/**
 * @brief Reads a bit at a specific position (QB64 _READBIT function)
 * @param a1 Value to read the bit from
 * @param b1 Bit position to read (0-based, least significant bit is 0)
 * @return QB_TRUE if the bit is set, QB_FALSE if the bit is clear
 * @note Returns the state of the bit at the specified position
 */
static inline constexpr int64_t func__readbit(uint64_t a1, int b1) {
    return (a1 & 1ull << b1) ? QB_TRUE : QB_FALSE;
}

/**
 * @brief Sets a bit at a specific position (QB64 _SETBIT function)
 * @param a1 Value to modify
 * @param b1 Bit position to set (0-based, least significant bit is 0)
 * @return Value with the specified bit set to 1
 * @note Returns a new value with the bit at position b1 set to 1
 */
static inline constexpr uint64_t func__setbit(uint64_t a1, int b1) {
    return a1 | 1ull << b1;
}

/**
 * @brief Clears a bit at a specific position (QB64 _RESETBIT function)
 * @param a1 Value to modify
 * @param b1 Bit position to clear (0-based, least significant bit is 0)
 * @return Value with the specified bit set to 0
 * @note Returns a new value with the bit at position b1 set to 0
 */
static inline constexpr uint64_t func__resetbit(uint64_t a1, int b1) {
    return a1 & ~(1ull << b1);
}

/**
 * @brief Toggles a bit at a specific position (QB64 _TOGGLEBIT function)
 * @param a1 Value to modify
 * @param b1 Bit position to toggle (0-based, least significant bit is 0)
 * @return Value with the specified bit toggled (0 becomes 1, 1 becomes 0)
 * @note Returns a new value with the bit at position b1 flipped
 */
static inline constexpr uint64_t func__togglebit(uint64_t a1, int b1) {
    return a1 ^ 1ull << b1;
}
