
// Implementation of these functions was pulled from miniaudio.h (MIT)

#include "libqb-common.h"

#include <algorithm>
#include <string.h>

#include "filepath.h"
#include "qbs.h"

/**
 * @file filepath.cpp
 * @brief Implementation of file path manipulation functions for QB64-PE
 * 
 * This file implements cross-platform file path utilities including filename extraction,
 * extension handling, and path normalization. Some functions are based on miniaudio.h (MIT).
 */

/**
 * @brief Extracts the filename from a file path
 * @param path File path string
 * @return Pointer to the filename portion of the path, or NULL if path is NULL
 * @note Finds the last occurrence of '/' or '\\' and returns the portion after it.
 *       Returns the original path if no separator is found.
 */
const char *filepath_get_filename(const char *path) {
    const char *fileName;

    if (path == NULL) {
        return NULL;
    }

    fileName = path;

    /* We just loop through the path until we find the last slash. */
    while (path[0] != '\0') {
        if (path[0] == '/' || path[0] == '\\') {
            fileName = path;
        }

        path += 1;
    }

    /* At this point the file name is sitting on a slash, so just move forward. */
    while (fileName[0] != '\0' && (fileName[0] == '/' || fileName[0] == '\\')) {
        fileName += 1;
    }

    return fileName;
}

/**
 * @brief Extracts the file extension from a file path
 * @param path File path string
 * @return Pointer to the extension (without the dot), or pointer to end of string if no extension
 * @note Finds the last '.' in the filename and returns the portion after it.
 *       If path is NULL, treats it as empty string.
 */
const char *filepath_get_extension(const char *path) {
    const char *extension;
    const char *lastOccurance;

    if (path == NULL) {
        path = "";
    }

    extension = filepath_get_filename(path);
    lastOccurance = NULL;

    /* Just find the last '.' and return. */
    while (extension[0] != '\0') {
        if (extension[0] == '.') {
            extension += 1;
            lastOccurance = extension;
        }

        extension += 1;
    }

    return (lastOccurance != NULL) ? lastOccurance : extension;
}

/**
 * @brief Checks if a file path has a specific extension
 * @param path File path to check
 * @param extension Extension to check for (without the dot)
 * @return true if the path has the specified extension, false otherwise
 * @note Case-insensitive comparison. Returns false if either parameter is NULL.
 */
bool filepath_has_extension(const char *path, const char *extension) {
    const char *ext1;
    const char *ext2;

    if (path == NULL || extension == NULL) {
        return false;
    }

    ext1 = extension;
    ext2 = filepath_get_extension(path);

#if defined(_MSC_VER) || defined(__DMC__)
    return _stricmp(ext1, ext2) == 0;
#else
    return strcasecmp(ext1, ext2) == 0;
#endif
}

/**
 * @brief Normalizes path separators for the current OS (char* overload)
 * @param path Path to normalize (contents may be modified)
 * @return Pointer to the modified path string
 * @note On Windows, converts '/' to '\\'. On POSIX, converts '\\' to '/'.
 */
const char *filepath_fix_directory(char *path) {
    auto len = strlen(path);

    for (size_t i = 0; i < len; i++) {
#ifdef QB64_WINDOWS
        if (path[i] == '/')
            path[i] = '\\';
#else
        if (path[i] == '\\')
            path[i] = '/';
#endif
    }

    return path;
}

/**
 * @brief Normalizes path separators for the current OS (qbs* overload)
 * @param path Path to normalize (contents may be modified)
 * @return Pointer to the modified path string
 * @note On Windows, converts '/' to '\\'. On POSIX, converts '\\' to '/'.
 */
const char *filepath_fix_directory(qbs *path) {
    for (size_t i = 0; i < size_t(path->len); i++) {
#ifdef QB64_WINDOWS
        if (path->chr[i] == '/')
            path->chr[i] = '\\';
#else
        if (path->chr[i] == '\\')
            path->chr[i] = '/';
#endif
    }

    return reinterpret_cast<char *>(path->chr);
}

/**
 * @brief Normalizes path separators for the current OS (std::string overload)
 * @param path Path to normalize (contents may be modified)
 * @return Pointer to the modified path string
 * @note On Windows, converts '/' to '\\'. On POSIX, converts '\\' to '/'.
 */
const char *filepath_fix_directory(std::string &path) {
    std::transform(path.begin(), path.end(), path.begin(), [](unsigned char c) {
#ifdef QB64_WINDOWS
        return c == '/' ? '\\' : c;
#else
        return c == '\\' ? '/' : c;
#endif
    });

    return path.c_str();
}

/**
 * @brief Splits a file path into directory and filename components
 * @param filePath Full file path to split
 * @param[out] directory Output directory path (includes trailing separator)
 * @param[out] fileName Output filename
 * @note Finds the last occurrence of '/' or '\\' and splits the path at that point.
 *       If no separator is found, directory is empty and fileName is the full path.
 */
void filepath_split(const std::string &filePath, std::string &directory, std::string &fileName) {
    // Find the last occurrence of either '/' or '\\'
    size_t lastSlash = filePath.find_last_of("/\\");

    if (lastSlash != std::string::npos) {
        directory = filePath.substr(0, lastSlash + 1); // include the trailing separator
        fileName = filePath.substr(lastSlash + 1);
    } else {
        // No directory separator found
        directory.clear();
        fileName = filePath;
    }
}

/**
 * @brief Joins a directory and filename into a complete file path
 * @param[out] filePath Output complete file path
 * @param directory Directory path
 * @param fileName Filename
 * @note Adds a trailing separator to directory if it doesn't have one.
 *       Uses platform-appropriate separator (\\ on Windows, / on POSIX).
 */
void filepath_join(std::string &filePath, const std::string &directory, const std::string &fileName) {
    // Check if the directory has a trailing separator, and add one if not
    filePath = directory;

    if (!filePath.empty() && filePath.back() != '/' && filePath.back() != '\\') {
#ifdef QB64_WINDOWS
        filePath += '\\';
#else
        filePath += '/';
#endif
    }

    // Append the file name to the directory
    filePath += fileName;
}
