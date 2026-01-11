#pragma once

#include <stdint.h>

/**
 * @file cmem.h
 * @brief Conventional memory declarations for QB64-PE
 * 
 * This header provides declarations for conventional memory used by QB64.
 * The memory size is calculated as: 16*65535+65535+3 (enough for highest
 * referenceable dword in conventional memory).
 */

/**
 * @brief Conventional memory array
 * @note Size: 16*65535+65535+3 bytes (enough for highest referenceable dword)
 */
extern uint8_t cmem[1114099];

/**
 * @brief Data block pointer
 * @note Required for Play() function. Declaration location not found elsewhere.
 */
extern intptr_t dblock;
