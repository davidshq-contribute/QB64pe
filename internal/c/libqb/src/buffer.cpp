
#include "libqb-common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

/**
 * @file buffer.cpp
 * @brief Implementation of buffer management for QB64-PE
 * 
 * This file implements a linked-list buffer system for efficient data buffering,
 * used for network I/O and other streaming operations.
 */

/**
 * @brief Initializes a buffer structure
 * @param buffer Buffer to initialize
 * @note Sets up the buffer with empty head and tail pointers.
 */
void libqb_buffer_init(struct libqb_buffer *buffer) {
    memset(buffer, 0, sizeof(*buffer));

    buffer->tail = &buffer->head;
}

/**
 * @brief Frees a buffer entry
 * @param ent Buffer entry to free
 * @note Frees both the entry structure and its data.
 */
static void libqb_buffer_entry_free(struct libqb_buffer_entry *ent) {
    free(ent->data);
    free(ent);
}

/**
 * @brief Clears all data from a buffer
 * @param buffer Buffer to clear
 * @note Frees all entries and reinitializes the buffer.
 */
void libqb_buffer_clear(struct libqb_buffer *buffer) {
    struct libqb_buffer_entry *entry = buffer->head;

    while (entry) {
        struct libqb_buffer_entry *nxt = entry->next;

        libqb_buffer_entry_free(entry);

        entry = nxt;
    }

    libqb_buffer_init(buffer);
}

/**
 * @brief Gets the total length of data in the buffer
 * @param buffer Buffer to query
 * @return Total number of bytes in the buffer
 */
size_t libqb_buffer_length(struct libqb_buffer *buffer) {
    return buffer->total_length;
}

/**
 * @brief Reads data from the buffer
 * @param buffer Buffer to read from
 * @param out Output buffer to write to
 * @param length Maximum number of bytes to read
 * @return Actual number of bytes read
 * @note Reads from the head of the buffer, removing entries as they are consumed.
 *       Partial reads are supported - if not all data from an entry is read,
 *       the entry is kept with an updated offset.
 */
size_t libqb_buffer_read(struct libqb_buffer *buffer, char *out, size_t length) {
    size_t actual_length = 0;

    while (buffer->head && length) {
        struct libqb_buffer_entry *entry = buffer->head;
        size_t offset = buffer->cur_entry_offset;

        size_t len = entry->length - offset;
        if (len > length)
            len = length;

        memcpy(out, entry->data + offset, len);

        out += len;
        length -= len;
        actual_length += len;

        if (len == entry->length - offset) {
            // This buffer is done, drop it
            buffer->head = buffer->head->next;
            buffer->cur_entry_offset = 0;

            libqb_buffer_entry_free(entry);
        } else {
            // We didn't use the whole buffer, length == 0, loop will end
            buffer->cur_entry_offset = offset + len;
        }
    }

    // If the list is now empty, we need to reset the tail pointer
    if (!buffer->head)
        buffer->tail = &buffer->head;

    buffer->total_length -= actual_length;

    return actual_length;
}

/**
 * @brief Writes data to the buffer
 * @param buffer Buffer to write to
 * @param in Input data to write
 * @param length Number of bytes to write
 * @note Allocates a new buffer entry and copies the data. Appends to the tail
 *       of the buffer. The data is copied, so the input buffer can be freed
 *       after this call.
 */
void libqb_buffer_write(struct libqb_buffer *buffer, const char *in, size_t length) {
    struct libqb_buffer_entry *new_ent = (struct libqb_buffer_entry *)malloc(sizeof(*new_ent));

    new_ent->length = length;
    new_ent->next = NULL;
    new_ent->data = (char *)malloc(length);

    memcpy(new_ent->data, in, length);

    *buffer->tail = new_ent;
    buffer->tail = &new_ent->next;
    buffer->total_length += length;
}
