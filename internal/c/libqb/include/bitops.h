//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Bit Operations Module
//  Provides bit manipulation, rotation, and extraction utilities
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "libqb-common.h"
#include <limits.h>
#include <stdint.h>

// ============================================================================
// BIT FIELD EXTRACTION AND MANIPULATION
// ============================================================================

/// Extracts unsigned bits from memory at specified position
/// Reads a bit field of specified size from memory location
/// @param bsize Size of bit field in bits (1-64)
/// @param base Base address of memory to read from
/// @param i Bit position to start extraction from (0-based)
/// @return Unsigned value of extracted bit field
uint64_t getubits(uint32_t bsize, uint8_t *base, intptr_t i);

/// Extracts signed bits from memory at specified position
/// Reads a signed bit field of specified size from memory location
/// @param bsize Size of bit field in bits (1-64)
/// @param base Base address of memory to read from
/// @param i Bit position to start extraction from (0-based)
/// @return Signed value of extracted bit field
int64_t getbits(uint32_t bsize, uint8_t *base, intptr_t i);

/// Sets bits in memory at specified position
/// Writes a bit field of specified size to memory location
/// @param bsize Size of bit field in bits (1-64)
/// @param base Base address of memory to write to
/// @param i Bit position to start writing from (0-based)
/// @param val Value to write (will be truncated to bsize bits)
void setbits(uint32_t bsize, uint8_t *base, intptr_t i, int64_t val);

// ============================================================================
// BIT ROTATION OPERATIONS (TEMPLATES)
// ============================================================================

/// Rotates bits left (circular shift)
/// Bits that shift out of the left side reappear on the right side
/// @param value Value to rotate
/// @param count Number of bit positions to rotate left
/// @return Rotated value
template <typename T> static inline constexpr T func__rol(T value, unsigned int count) {
    const unsigned int mask = CHAR_BIT * sizeof(T) - 1;
    count &= mask;
    return (value << count) | (value >> (-count & mask));
}

/// Rotates bits right (circular shift)
/// Bits that shift out of the right side reappear on the left side
/// @param value Value to rotate
/// @param count Number of bit positions to rotate right
/// @return Rotated value
template <typename T> static inline constexpr T func__ror(T value, unsigned int count) {
    const unsigned int mask = CHAR_BIT * sizeof(T) - 1;
    count &= mask;
    return (value >> count) | (value << (-count & mask));
}

// ============================================================================
// BIT SHIFT OPERATIONS
// ============================================================================

/// Shifts bits left (logical shift)
/// Performs left shift with zero fill on right side
/// @param a1 Value to shift
/// @param b1 Number of positions to shift left
/// @return Shifted value
static inline constexpr uint64_t func__shl(uint64_t a1, int b1) {
    return a1 << b1;
}

/// Shifts bits right (logical shift)
/// Performs right shift with zero fill on left side
/// @param a1 Value to shift
/// @param b1 Number of positions to shift right
/// @return Shifted value
static inline constexpr uint64_t func__shr(uint64_t a1, int b1) {
    return a1 >> b1;
}

// ============================================================================
// INDIVIDUAL BIT OPERATIONS
// ============================================================================

/// Reads a single bit from value
/// Tests if specified bit is set (1) or clear (0)
/// @param a1 Value to test
/// @param b1 Bit position to test (0-based)
/// @return QB_TRUE if bit is set, QB_FALSE if bit is clear
static inline constexpr int64_t func__readbit(uint64_t a1, int b1) {
    return (a1 & 1ull << b1) ? QB_TRUE : QB_FALSE;
}

/// Sets a single bit in value
/// Sets specified bit to 1
/// @param a1 Value to modify
/// @param b1 Bit position to set (0-based)
/// @return Value with specified bit set
static inline constexpr uint64_t func__setbit(uint64_t a1, int b1) {
    return a1 | 1ull << b1;
}

/// Clears a single bit in value
/// Sets specified bit to 0
/// @param a1 Value to modify
/// @param b1 Bit position to clear (0-based)
/// @return Value with specified bit cleared
static inline constexpr uint64_t func__resetbit(uint64_t a1, int b1) {
    return a1 & ~(1ull << b1);
}

/// Toggles a single bit in value
/// Flips specified bit (0 becomes 1, 1 becomes 0)
/// @param a1 Value to modify
/// @param b1 Bit position to toggle (0-based)
/// @return Value with specified bit toggled
static inline constexpr uint64_t func__togglebit(uint64_t a1, int b1) {
    return a1 ^ 1ull << b1;
}
