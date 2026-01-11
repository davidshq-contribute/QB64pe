//-----------------------------------------------------------------------------------------------------
//  QB64-PE filesystem related functions
//-----------------------------------------------------------------------------------------------------

#pragma once

#include <stdint.h>

struct qbs;

/**
 * @file filesystem.h
 * @brief Filesystem operations for QB64-PE
 * 
 * This header provides functions for file and directory operations,
 * including existence checks, directory navigation, and file management.
 */

/**
 * @brief Saves the current working directory as the start directory
 * @note This should be called at program startup to remember the initial directory.
 *       Used by func__startdir() to return to the program's starting directory.
 */
void FS_SaveStartDirectory();

/**
 * @brief Checks if a directory exists
 * @param path Directory path to check
 * @return true if directory exists, false otherwise
 */
bool FS_DirectoryExists(const char *path);

/**
 * @brief Checks if a file exists
 * @param path File path to check
 * @return true if file exists, false otherwise
 */
bool FS_FileExists(const char *path);

/**
 * @brief Gets the current working directory (QB64 _CWD$ function)
 * @return qbs string containing the current working directory, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func__cwd();

/**
 * @brief Gets directory listing (QB64 _DIR$ function)
 * @param qbsContext Context string for directory listing
 * @return qbs string containing directory listing, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func__dir(qbs *qbsContext);

/**
 * @brief Checks if a directory exists (QB64 _DIREXISTS function)
 * @param path Directory path to check
 * @return Non-zero if directory exists, 0 otherwise
 */
int32_t func__direxists(qbs *path);

/**
 * @brief Checks if a file exists (QB64 _FILEEXISTS function)
 * @param path File path to check
 * @return Non-zero if file exists, 0 otherwise
 */
int32_t func__fileexists(qbs *path);

/**
 * @brief Gets the starting directory (QB64 _STARTDIR$ function)
 * @return qbs string containing the program's starting directory, or NULL on failure
 * @note Returns the directory where the program was launched. Caller must free with qbs_free()
 */
qbs *func__startdir();

/**
 * @brief Changes the current working directory (QB64 CHDIR statement)
 * @param str Directory path to change to
 * @note Changes the process's current working directory. May throw an error if directory doesn't exist.
 */
void sub_chdir(qbs *str);

/**
 * @brief Gets file listing matching a pattern (QB64 _FILES$ function)
 * @param qbsFileSpec File specification pattern (e.g., "*.txt")
 * @param passed Flag indicating if pattern was provided
 * @return qbs string containing file listing, or NULL on failure
 * @note If passed is 0, lists all files. Caller must free with qbs_free()
 */
qbs *func__files(qbs *qbsFileSpec, int32_t passed);

/**
 * @brief Gets the full absolute path of a file (QB64 _FULLPATH$ function)
 * @param qbsPathName File path (may be relative)
 * @return qbs string containing the full absolute path, or NULL on failure
 * @note Converts relative paths to absolute paths. Caller must free with qbs_free()
 */
qbs *func__fullpath(qbs *qbsPathName);

/**
 * @brief Lists files matching a pattern (QB64 FILES statement)
 * @param str File specification pattern (e.g., "*.txt")
 * @param passed Flag indicating if pattern was provided
 * @note Outputs file listing to console. If passed is 0, lists all files.
 */
void sub_files(qbs *str, int32_t passed);

/**
 * @brief Deletes a file (QB64 KILL statement)
 * @param str File path to delete
 * @note Permanently deletes the file. May throw an error if file doesn't exist or can't be deleted.
 */
void sub_kill(qbs *str);

/**
 * @brief Creates a directory (QB64 MKDIR statement)
 * @param str Directory path to create
 * @note Creates the directory and any necessary parent directories. May throw an error on failure.
 */
void sub_mkdir(qbs *str);

/**
 * @brief Renames a file or directory (QB64 NAME statement)
 * @param oldname Current name/path of the file
 * @param newname New name/path for the file
 * @note Can be used to rename or move files. May throw an error on failure.
 */
void sub_name(qbs *oldname, qbs *newname);

/**
 * @brief Removes a directory (QB64 RMDIR statement)
 * @param str Directory path to remove
 * @note Removes an empty directory. May throw an error if directory doesn't exist or is not empty.
 */
void sub_rmdir(qbs *str);
