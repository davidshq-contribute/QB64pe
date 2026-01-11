#ifndef INCLUDE_LIBQB_QBS_H
#define INCLUDE_LIBQB_QBS_H

#include <stdint.h>

/**
 * @file qbs.h
 * @brief QB64 string (qbs) management functions
 * 
 * This header provides the core string handling API for QB64-PE.
 * All QB64 strings are represented as qbs (QB64 String) structures.
 */

/**
 * @struct qbs_field
 * @brief Represents a field within a file for file-based string operations
 */
struct qbs_field {
    int32_t fileno;    ///< File number
    int64_t fileid;    ///< File identifier
    int64_t size;      ///< Size of the field
    int64_t offset;    ///< Offset within the file
};

/**
 * @struct qbs
 * @brief QB64 string descriptor structure
 * 
 * This structure represents a QB64 string with its data and metadata.
 * Strings are reference-counted and automatically managed.
 */
struct qbs {
    uint8_t *chr;      ///< Pointer to the string's data (32-bit pointer)
    int32_t len;       ///< Length of the string (must be signed for comparisons)

    uint8_t in_cmem;    ///< Set to 1 if string is in conventional memory DBLOCK
    uint16_t *cmem_descriptor;
    uint16_t cmem_descriptor_offset;

    uint32_t listi;     ///< Index in the list of strings that references it

    uint8_t tmp;        ///< Set to 1 if string can be deleted immediately after processing
    uint32_t tmplisti;  ///< Index in the temporary string list

    uint8_t fixed;      ///< Indicates a fixed-length string
    uint8_t readonly;   ///< Set to 1 if string is read-only

    qbs_field *field;   ///< Pointer to file field information (if applicable)
};

/**
 * @brief Creates a new qbs string with the specified length
 * @param len Length of the string to create
 * @param tmp Temporary flag (1 = temporary, 0 = permanent)
 * @return Pointer to the newly created qbs string, or NULL on failure
 * @note The string is initialized with zeros. Caller is responsible for freeing with qbs_free()
 */
qbs *qbs_new(int32_t len, uint8_t tmp);

/**
 * @brief Creates a new qbs string from a C null-terminated string
 * @param txt Null-terminated C string to copy
 * @return Pointer to the newly created qbs string, or NULL on failure
 * @note The string is copied. Caller is responsible for freeing with qbs_free()
 */
qbs *qbs_new_txt(const char *txt);

/**
 * @brief Creates a new qbs string in conventional memory
 * @param size Size of the string to create
 * @param tmp Temporary flag (1 = temporary, 0 = permanent)
 * @return Pointer to the newly created qbs string, or NULL on failure
 * @note The string is allocated in conventional memory. Caller is responsible for freeing with qbs_free()
 */
qbs *qbs_new_cmem(int32_t size, uint8_t tmp);

/**
 * @brief Creates a new qbs string from a C string with explicit length
 * @param txt C string to copy (may contain null bytes)
 * @param len Length of the string to copy
 * @return Pointer to the newly created qbs string, or NULL on failure
 * @note The string is copied with the specified length. Caller is responsible for freeing with qbs_free()
 */
qbs *qbs_new_txt_len(const char *txt, int32_t len);

/**
 * @brief Creates a new qbs string from a fixed memory location
 * @param offset Pointer to the fixed memory location
 * @param size Size of the fixed string
 * @param tmp Temporary flag (1 = temporary, 0 = permanent)
 * @return Pointer to the newly created qbs string, or NULL on failure
 * @note The string uses the provided memory location directly. Do not free the underlying memory.
 */
qbs *qbs_new_fixed(uint8_t *offset, uint32_t size, uint8_t tmp);

/**
 * @brief Concatenates two qbs strings
 * @param str1 First string
 * @param str2 Second string
 * @return Pointer to a new qbs string containing the concatenation, or NULL on failure
 * @note Creates a new string. Caller is responsible for freeing with qbs_free()
 */
qbs *qbs_add(qbs *str1, qbs *str2);

/**
 * @brief Sets the contents of a qbs string to match another
 * @param dest Destination string (will be modified)
 * @param src Source string to copy from
 * @return Pointer to the destination string
 * @note The destination string is reallocated if necessary to fit the source
 */
qbs *qbs_set(qbs *dest, qbs *src);

/**
 * @brief Frees a qbs string
 * @param str Pointer to the qbs string to free
 * @note This function handles reference counting. The string is only freed when the reference count reaches zero.
 * @warning Do not use the string pointer after calling this function
 */
void qbs_free(qbs *str);

/**
 * @brief Converts a qbs string to a numeric value
 * @tparam T Numeric type to convert to (int, float, double, etc.)
 * @param s qbs string to convert
 * @return Numeric value extracted from the string
 * @note The string is parsed as a number. Returns 0 if parsing fails.
 */
template <typename T> T qbs_val(qbs *s);

/**
 * @name Legacy STR$ function prototypes
 * @brief Convert numeric values to qbs strings (legacy QB64 STR$ function)
 * @param value Numeric value to convert
 * @return Pointer to a new qbs string containing the string representation
 * @note These functions use the legacy QB64 STR$ format. Caller must free with qbs_free()
 */
///@{
qbs *qbs_str(int64_t value);
qbs *qbs_str(int32_t value);
qbs *qbs_str(int16_t value);
qbs *qbs_str(int8_t value);
qbs *qbs_str(uint64_t value);
qbs *qbs_str(uint32_t value);
qbs *qbs_str(uint16_t value);
qbs *qbs_str(uint8_t value);
qbs *qbs_str(float value);
qbs *qbs_str(double value);
qbs *qbs_str(long double value);
///@}

/**
 * @name Modern _TOSTR$ function prototypes
 * @brief Convert numeric values to qbs strings with formatting control
 * @param value Numeric value to convert
 * @param digits Number of digits to display (if passed > 0)
 * @param passed Flag indicating if digits parameter was provided
 * @return Pointer to a new qbs string containing the formatted string representation
 * @note These functions provide more control over formatting than qbs_str(). Caller must free with qbs_free()
 */
///@{
qbs *qbs__tostr(int64_t value, int32_t digits, int32_t passed);
qbs *qbs__tostr(int32_t value, int32_t digits, int32_t passed);
qbs *qbs__tostr(int16_t value, int32_t digits, int32_t passed);
qbs *qbs__tostr(int8_t value, int32_t digits, int32_t passed);
qbs *qbs__tostr(uint64_t value, int32_t digits, int32_t passed);
qbs *qbs__tostr(uint32_t value, int32_t digits, int32_t passed);
qbs *qbs__tostr(uint16_t value, int32_t digits, int32_t passed);
qbs *qbs__tostr(uint8_t value, int32_t digits, int32_t passed);
qbs *qbs__tostr(float value, int32_t digits, int32_t passed);
qbs *qbs__tostr(double value, int32_t digits, int32_t passed);
qbs *qbs__tostr(long double value, int32_t digits, int32_t passed);
///@}

/**
 * @brief Creates a qbs string containing a single character from an ASCII value
 * @param value ASCII value (0-255)
 * @return Pointer to a new qbs string containing the character, or NULL on failure
 * @note Equivalent to QB64 CHR$ function. Caller must free with qbs_free()
 */
qbs *func_chr(int32_t value);

/**
 * @brief Converts a string to uppercase
 * @param str Source string
 * @return Pointer to a new qbs string with uppercase characters, or NULL on failure
 * @note Creates a new string. Caller must free with qbs_free()
 */
qbs *qbs_ucase(qbs *str);

/**
 * @brief Converts a string to lowercase
 * @param str Source string
 * @return Pointer to a new qbs string with lowercase characters, or NULL on failure
 * @note Creates a new string. Caller must free with qbs_free()
 */
qbs *qbs_lcase(qbs *str);

/**
 * @brief Extracts the leftmost characters from a string
 * @param str Source string
 * @param l Number of characters to extract
 * @return Pointer to a new qbs string containing the leftmost l characters, or NULL on failure
 * @note If l exceeds the string length, returns the entire string. Caller must free with qbs_free()
 */
qbs *qbs_left(qbs *str, int32_t l);

/**
 * @brief Extracts the rightmost characters from a string
 * @param str Source string
 * @param l Number of characters to extract
 * @return Pointer to a new qbs string containing the rightmost l characters, or NULL on failure
 * @note If l exceeds the string length, returns the entire string. Caller must free with qbs_free()
 */
qbs *qbs_right(qbs *str, int32_t l);

/**
 * @brief Compares two strings for equality
 * @param str1 First string
 * @param str2 Second string
 * @return Non-zero if strings are equal, 0 otherwise
 */
int32_t qbs_equal(qbs *str1, qbs *str2);

/**
 * @brief Compares two strings for inequality
 * @param str1 First string
 * @param str2 Second string
 * @return Non-zero if strings are not equal, 0 otherwise
 */
int32_t qbs_notequal(qbs *str1, qbs *str2);

/**
 * @brief Checks if str2 is greater than str1 (lexicographically)
 * @param str2 Second string
 * @param str1 First string
 * @return Non-zero if str2 > str1, 0 otherwise
 */
int32_t qbs_greaterthan(qbs *str2, qbs *str1);

/**
 * @brief Checks if str1 is less than str2 (lexicographically)
 * @param str1 First string
 * @param str2 Second string
 * @return Non-zero if str1 < str2, 0 otherwise
 */
int32_t qbs_lessthan(qbs *str1, qbs *str2);

/**
 * @brief Checks if str1 is less than or equal to str2 (lexicographically)
 * @param str1 First string
 * @param str2 Second string
 * @return Non-zero if str1 <= str2, 0 otherwise
 */
int32_t qbs_lessorequal(qbs *str1, qbs *str2);

/**
 * @brief Checks if str2 is greater than or equal to str1 (lexicographically)
 * @param str2 Second string
 * @param str1 First string
 * @return Non-zero if str2 >= str1, 0 otherwise
 */
int32_t qbs_greaterorequal(qbs *str2, qbs *str1);

/**
 * @brief Gets the ASCII value of a character at a specific position
 * @param str Source string
 * @param i Character position (1-based)
 * @return ASCII value of the character at position i, or 0 if position is invalid
 */
int32_t qbs_asc(qbs *str, uint32_t i);

/**
 * @brief Gets the ASCII value of the first character in a string
 * @param str Source string
 * @return ASCII value of the first character, or 0 if string is empty
 */
int32_t qbs_asc(qbs *str);

/**
 * @brief Gets the length of a qbs string
 * @param str Source string
 * @return Length of the string in characters
 * @note This is an inline function that directly accesses the len field
 */
static inline int32_t qbs_len(qbs *str) {
    return str->len;
}

// FIXME: Usages of these outside of qbx.c (and qbs_cleanup()) need to be removed.
extern intptr_t *qbs_tmp_list;
extern uint32_t qbs_tmp_list_lasti;
extern uint32_t qbs_tmp_list_nexti;

/**
 * @brief Cleans up temporary strings created since a base index
 * @tparam T Type of value to return
 * @param base Base index in the temporary string list
 * @param passvalue Value to return after cleanup
 * @return The passvalue parameter
 * @note Frees all temporary strings created after the base index. Used for automatic cleanup.
 */
template <typename T> static T qbs_cleanup(uint32_t base, T passvalue) {

    while (qbs_tmp_list_nexti > base) {
        qbs_tmp_list_nexti--;
        if (qbs_tmp_list[qbs_tmp_list_nexti] != -1)
            qbs_free((qbs *)qbs_tmp_list[qbs_tmp_list_nexti]);
    } // clear any temp. strings created

    return passvalue;
}

/**
 * @brief Left-aligns a string in a destination (QB64 LSET statement)
 * @param dest Destination string (will be modified)
 * @param source Source string to copy from
 * @note If source is shorter than dest, dest is padded with spaces on the right.
 *       If source is longer, it is truncated to fit dest.
 */
void sub_lset(qbs *dest, qbs *source);

/**
 * @brief Right-aligns a string in a destination (QB64 RSET statement)
 * @param dest Destination string (will be modified)
 * @param source Source string to copy from
 * @note If source is shorter than dest, dest is padded with spaces on the left.
 *       If source is longer, it is truncated to fit dest.
 */
void sub_rset(qbs *dest, qbs *source);

/**
 * @brief Creates a string filled with spaces
 * @param spaces Number of spaces
 * @return Pointer to a new qbs string containing spaces, or NULL on failure
 * @note Equivalent to QB64 SPACE$ function. Caller must free with qbs_free()
 */
qbs *func_space(int32_t spaces);

/**
 * @brief Creates a string filled with a repeated character
 * @param characters Number of characters
 * @param asciivalue ASCII value of the character to repeat
 * @return Pointer to a new qbs string containing the repeated character, or NULL on failure
 * @note Equivalent to QB64 STRING$ function. Caller must free with qbs_free()
 */
qbs *func_string(int32_t characters, int32_t asciivalue);

/**
 * @brief Searches for a substring within a string
 * @param start Starting position for search (1-based, if passed > 0)
 * @param str String to search in
 * @param substr Substring to search for
 * @param passed Flag indicating if start parameter was provided
 * @return Position of substring (1-based), or 0 if not found
 * @note Equivalent to QB64 INSTR function. Search is case-sensitive.
 */
int32_t func_instr(int32_t start, qbs *str, qbs *substr, int32_t passed);

/**
 * @brief Searches for a substring within a string from the end
 * @param start Starting position for reverse search (1-based, if passed > 0)
 * @param str String to search in
 * @param substr Substring to search for
 * @param passed Flag indicating if start parameter was provided
 * @return Position of substring (1-based), or 0 if not found
 * @note Searches backwards from the end of the string. Search is case-sensitive.
 */
int32_t func__instrrev(int32_t start, qbs *str, qbs *substr, int32_t passed);

/**
 * @brief Extracts a substring and assigns it to a destination (QB64 MID$ statement)
 * @param dest Destination string (will be modified)
 * @param start Starting position (1-based)
 * @param l Length of substring to extract
 * @param src Source string
 * @param passed Flag indicating if length parameter was provided
 * @note If passed is 0, extracts from start to end of string. If l is 0, clears dest.
 */
void sub_mid(qbs *dest, int32_t start, int32_t l, qbs *src, int32_t passed);

/**
 * @brief Extracts a substring from a string (QB64 MID$ function)
 * @param str Source string
 * @param start Starting position (1-based)
 * @param l Length of substring to extract
 * @param passed Flag indicating if length parameter was provided
 * @return Pointer to a new qbs string containing the substring, or NULL on failure
 * @note If passed is 0, extracts from start to end of string. Caller must free with qbs_free()
 */
qbs *func_mid(qbs *str, int32_t start, int32_t l, int32_t passed);

/**
 * @brief Removes leading whitespace from a string
 * @param str Source string
 * @return Pointer to a new qbs string with leading whitespace removed, or NULL on failure
 * @note Creates a new string. Caller must free with qbs_free()
 */
qbs *qbs_ltrim(qbs *str);

/**
 * @brief Removes trailing whitespace from a string
 * @param str Source string
 * @return Pointer to a new qbs string with trailing whitespace removed, or NULL on failure
 * @note Creates a new string. Caller must free with qbs_free()
 */
qbs *qbs_rtrim(qbs *str);

/**
 * @brief Removes leading and trailing whitespace from a string
 * @param str Source string
 * @return Pointer to a new qbs string with leading and trailing whitespace removed, or NULL on failure
 * @note Creates a new string. Caller must free with qbs_free()
 */
qbs *qbs__trim(qbs *str);

/**
 * @brief Compares two strings case-insensitively
 * @param s1 First string
 * @param s2 Second string
 * @return Negative if s1 < s2, 0 if equal, positive if s1 > s2
 * @note Comparison is case-insensitive
 */
int32_t func__str_nc_compare(qbs *s1, qbs *s2);

/**
 * @brief Compares two strings case-sensitively
 * @param s1 First string
 * @param s2 Second string
 * @return Negative if s1 < s2, 0 if equal, positive if s1 > s2
 * @note Comparison is case-sensitive
 */
int32_t func__str_compare(qbs *s1, qbs *s2);

/**
 * @brief Sets the size of a qbs string (used by vWatch debugger)
 * @param target_qbs Pointer to a pointer to the target qbs string
 * @param newlength New length for the string
 * @note This function is called by the vWatch debugger to resize strings during debugging
 */
static inline void set_qbs_size(intptr_t *target_qbs, int32_t newlength) {
    qbs_set((qbs *)(*target_qbs), func_space(newlength));
}

#endif
