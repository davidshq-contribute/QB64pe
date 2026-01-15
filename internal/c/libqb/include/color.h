//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Color & Palette Module
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_COLOR_H
#define INCLUDE_LIBQB_COLOR_H

// ============================================================================
// DEPENDENCIES
// ============================================================================

#include <stdint.h>

// ============================================================================
// PUBLIC API DECLARATIONS
// ============================================================================

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

// Palette operations
uint32_t func__palettecolor(int32_t n, int32_t i, int32_t passed);
void sub__palettecolor(int32_t n, uint32_t c, int32_t i, int32_t passed);
void sub__copypalette(int32_t src, int32_t dst, int32_t passed);

// Clear/transparent color
void sub__clearcolor(uint32_t c, int32_t i, int32_t passed);
int32_t func__clearcolor(int32_t i, int32_t passed);

// Default colors
uint32_t func__defaultcolor(int32_t i, int32_t passed);
uint32_t func__backgroundcolor(int32_t i, int32_t passed);

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

// This module provides color management functionality including:
// - RGB/RGBA color creation and manipulation
// - Palette management for indexed color modes
// - Color channel extraction utilities
// - Default color handling

#endif // INCLUDE_LIBQB_COLOR_H
