#ifndef INCLUDE_LIBQB_FILEPATH_H
#define INCLUDE_LIBQB_FILEPATH_H

#include <string>

struct qbs;

/**
 * @file filepath.h
 * @brief File path manipulation utilities
 * 
 * This header provides functions for working with file paths, including
 * extracting components, normalizing separators, and splitting/joining paths.
 */

/**
 * @brief Extracts the filename portion from a full path
 * @param path Full file path
 * @return Pointer to the filename portion, or NULL/empty string if path has no filename
 * @note The returned pointer points into the original path string. Do not free it.
 * @example filepath_get_filename("/path/to/file.txt") returns "file.txt"
 */
const char *filepath_get_filename(const char *path);

/**
 * @brief Extracts the file extension from a filename
 * @param filename Filename (may include path)
 * @return Pointer to the extension (including the dot), or NULL/empty string if no extension
 * @note The returned pointer points into the original filename string. Do not free it.
 * @example filepath_get_extension("file.txt") returns ".txt"
 */
const char *filepath_get_extension(const char *filename);

/**
 * @brief Checks if a path has a specific file extension
 * @param path File path to check
 * @param extension Extension to check for (with or without leading dot)
 * @return true if the path has the specified extension, false otherwise
 * @note Comparison is case-insensitive. Extension can be provided with or without leading dot.
 * @example filepath_has_extension("file.txt", "txt") returns true
 */
bool filepath_has_extension(const char *path, const char *extension);

/**
 * @name Path Separator Normalization Functions
 * @brief Normalizes path separators to match the current operating system
 * @param path Path to normalize (modified in place)
 * @return Pointer to the modified path (same as input)
 * @note Converts path separators (/, \) to the appropriate separator for the current OS.
 *       The path is modified in place.
 */
///@{
const char *filepath_fix_directory(char *path);
const char *filepath_fix_directory(qbs *path);
const char *filepath_fix_directory(std::string &path);
///@}

/**
 * @brief Splits a file path into directory and filename components
 * @param filePath Full file path to split
 * @param[out] directory Output parameter for the directory portion
 * @param[out] fileName Output parameter for the filename portion
 * @note Both directory and fileName are cleared before being set.
 * @example filepath_split("/path/to/file.txt", dir, name) sets dir="/path/to/" and name="file.txt"
 */
void filepath_split(const std::string &filePath, std::string &directory, std::string &fileName);

/**
 * @brief Joins a directory and filename into a full file path
 * @param[out] filePath Output parameter for the joined path
 * @param directory Directory path
 * @param fileName Filename
 * @note Handles path separator insertion automatically. filePath is cleared before being set.
 * @example filepath_join(path, "/path/to", "file.txt") sets path="/path/to/file.txt"
 */
void filepath_join(std::string &filePath, const std::string &directory, const std::string &fileName);

#endif
