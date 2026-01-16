//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Buffer Management Module
//  Provides efficient data buffering utilities for stream operations
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_BUFFER_H
#define INCLUDE_LIBQB_BUFFER_H

#include <stdint.h>

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/// Represents a single entry in a linked buffer chain
/// Each entry holds a chunk of data and links to the next entry
struct libqb_buffer_entry {
    size_t length;                              ///< Length of data in this entry
    char *data;                                 ///< Pointer to the data buffer
    struct libqb_buffer_entry *next;            ///< Next entry in the linked list
};

/// Main buffer structure for efficient data streaming
/// Uses a linked list of entries to handle large data streams
struct libqb_buffer {
    size_t total_length;                        ///< Total length of all data in buffer
    size_t cur_entry_offset;                    ///< Current read offset within current entry
    
    struct libqb_buffer_entry *head;            ///< First entry in the buffer chain
    struct libqb_buffer_entry **tail;           ///< Pointer to the last entry's next pointer
};

// ============================================================================
// BUFFER MANAGEMENT FUNCTIONS
// ============================================================================

/// Initializes a new buffer structure
/// Sets up the buffer to an empty state ready for use
/// @param buffer Pointer to the buffer structure to initialize
void libqb_buffer_init(struct libqb_buffer *);

/// Frees all data used by the buffer and resets it to empty state
/// Releases all memory allocated for buffer entries and resets counters
/// @param buffer Pointer to the buffer to clear
void libqb_buffer_clear(struct libqb_buffer *);

/// Returns the current total length of data in the buffer
/// @param buffer Pointer to the buffer to query
/// @return Total number of bytes stored in the buffer
size_t libqb_buffer_length(struct libqb_buffer *);

/// Reads data from the buffer and removes it from the buffer
/// Data is read from the current position and the read pointer is advanced
/// @param buffer Pointer to the buffer to read from
/// @param data Pointer to destination buffer to store read data
/// @param length Maximum number of bytes to read
/// @return Actual number of bytes read (may be less than requested if buffer is empty)
size_t libqb_buffer_read(struct libqb_buffer *, char *, size_t length);

/// Writes data into the buffer
/// Appends the specified data to the end of the buffer
/// @param buffer Pointer to the buffer to write to
/// @param data Pointer to source data to write
/// @param length Number of bytes to write
void libqb_buffer_write(struct libqb_buffer *, const char *, size_t length);

#endif
