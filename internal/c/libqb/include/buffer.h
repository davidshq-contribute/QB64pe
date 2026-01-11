#ifndef INCLUDE_LIBQB_BUFFER_H
#define INCLUDE_LIBQB_BUFFER_H

#include <stdint.h>

/**
 * @file buffer.h
 * @brief Linked-list buffer implementation for QB64-PE
 * 
 * This header provides a buffer structure that uses a linked list of entries
 * to store data. This allows efficient appending and reading of data.
 */

/**
 * @struct libqb_buffer_entry
 * @brief A single entry in the linked-list buffer
 */
struct libqb_buffer_entry {
    size_t length;                      ///< Length of data in this entry
    char *data;                         ///< Pointer to the data
    struct libqb_buffer_entry *next;   ///< Pointer to the next entry
};

/**
 * @struct libqb_buffer
 * @brief Main buffer structure using a linked list of entries
 */
struct libqb_buffer {
    size_t total_length;                ///< Total length of all data in the buffer
    size_t cur_entry_offset;           ///< Current read offset within the first entry

    struct libqb_buffer_entry *head;   ///< Pointer to the first entry
    struct libqb_buffer_entry **tail;  ///< Pointer to the pointer to the last entry
};

/**
 * @brief Initializes a buffer structure
 * @param buffer Buffer structure to initialize
 * @note Must be called before using the buffer. Sets all fields to zero/NULL.
 */
void libqb_buffer_init(struct libqb_buffer *buffer);

/**
 * @brief Frees all data used by the buffer
 * @param buffer Buffer to clear
 * @note Frees all entries and their data. The buffer can be reused after calling libqb_buffer_init().
 */
void libqb_buffer_clear(struct libqb_buffer *buffer);

/**
 * @brief Gets the current total length of data in the buffer
 * @param buffer Buffer to query
 * @return Total number of bytes stored in the buffer
 */
size_t libqb_buffer_length(struct libqb_buffer *buffer);

/**
 * @brief Reads data from the buffer
 * @param buffer Buffer to read from
 * @param[out] dest Destination buffer to write data to
 * @param length Maximum number of bytes to read
 * @return Number of bytes actually read (may be less than length if buffer has less data)
 * @note The data read is removed from the buffer. Reading advances the read position.
 */
size_t libqb_buffer_read(struct libqb_buffer *buffer, char *dest, size_t length);

/**
 * @brief Writes data into the buffer
 * @param buffer Buffer to write to
 * @param src Source data to write
 * @param length Number of bytes to write
 * @note Appends data to the end of the buffer. The data is copied.
 */
void libqb_buffer_write(struct libqb_buffer *buffer, const char *src, size_t length);

#endif
