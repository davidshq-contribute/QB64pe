//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE File Path Utilities Module
//  Provides cross-platform file path manipulation and analysis functions
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_FILEPATH_H
#define INCLUDE_LIBQB_FILEPATH_H

#include <string>

struct qbs;

// ============================================================================
// PATH EXTRACTION FUNCTIONS
// ============================================================================

/// Extracts filename from full path
/// Takes a path + filename, and returns just the filename portion
/// Returns either NULL or empty string if it has none.
/// @param path Full file path to extract filename from
/// @return Pointer to filename portion (do not free), or NULL/empty if none
const char *filepath_get_filename(const char *path);

/// Extracts file extension from filename
/// Takes a filename and returns just the extension at the end
/// Returns either NULL or empty string if it has none.
/// @param filename Filename to extract extension from
/// @return Pointer to extension portion (do not free), or NULL/empty if none
const char *filepath_get_extension(const char *filename);

// ============================================================================
// PATH VALIDATION FUNCTIONS
// ============================================================================

/// Checks if file path has specified extension
/// Returns true if the path is to a file that matches the provided extension
/// @param path File path to check
/// @param extension Extension to match (without dot, e.g., "txt")
/// @return True if extension matches, false otherwise
bool filepath_has_extension(const char *path, const char *extension);

// ============================================================================
// PATH NORMALIZATION FUNCTIONS
// ============================================================================

/// Fixes directory separators in path based on current OS
/// The following overloaded functions change path separators in path based on the OS (path is modified)
/// Converts forward slashes to backslashes on Windows, ensures proper format
/// @param path Path string to normalize (modified in-place)
/// @return Pointer to normalized path string
const char *filepath_fix_directory(char *path);

/// QB64 string version of path normalization
/// @param path QB64 string path to normalize (modified in-place)
/// @return Pointer to normalized path string
const char *filepath_fix_directory(qbs *path);

/// C++ std::string version of path normalization
/// @param path String path to normalize (modified in-place)
/// @return Pointer to normalized path string
const char *filepath_fix_directory(std::string &path);

// ============================================================================
// PATH MANIPULATION FUNCTIONS
// ============================================================================

/// Splits a file path into directory and file name components
/// Separates full path into directory portion and filename portion
/// @param filePath Full file path to split
/// @param directory Output string for directory portion
/// @param fileName Output string for filename portion
void filepath_split(const std::string &filePath, std::string &directory, std::string &fileName);

/// Joins a directory and file name into a complete file path
/// Combines directory and filename with appropriate separator
/// @param filePath Output string for combined path
/// @param directory Directory portion
/// @param fileName Filename portion
void filepath_join(std::string &filePath, const std::string &directory, const std::string &fileName);

#endif
