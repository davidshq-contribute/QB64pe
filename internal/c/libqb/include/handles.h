//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Handle Types Module
//  Common types for special handles, streams, and related structures
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_HANDLES_H
#define INCLUDE_LIBQB_HANDLES_H

// ============================================================================
// DEPENDENCIES
// ============================================================================

#include "../../os.h"
#include "qblist.h"

// ============================================================================
// TYPE DEFINITIONS
// ============================================================================

/// Special handle types for custom QB64 interfaces.
/// These handles use negative values to distinguish them from standard file handles.
enum class special_handle_type {
    Invalid,  ///< Invalid or uninitialized handle
    Stream,   ///< Stream handle (TCP, etc.)
    Host,     ///< Host connection handle
    Http,     ///< HTTP connection handle
};

/// Special handle structure for managing custom QB64 interfaces.
/// Uses negative handle values to coexist with standard QB file handle indexing.
struct special_handle_struct {
    special_handle_type type;  ///< Type of special handle
    ptrszint index;            ///< Index or pointer to the type's object
                               ///< For HTTP streams, this is also used as an EOF flag
};

/// Stream types supported by the stream system.
enum class stream_type {
    Tcp,  ///< TCP network stream
};

/// Stream structure for unified access to streamed input/output data.
/// Provides buffering and state management for streamed data sources.
struct stream_struct {
    uint8 *in;           ///< Input buffer pointer
    ptrszint in_size;    ///< Current buffer size in bytes
    ptrszint in_limit;   ///< Size limit before buffer reallocation is required
    int8 eof;            ///< End-of-stream flag (set when user reads past end)
    stream_type type;    ///< Type of stream
    ptrszint index;      ///< Index or pointer to the type's object
    // Note: 'out' buffer is not required as data can be sent directly to the interface
};

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

/// Global list of special handles (manages all special handle instances).
extern list *special_handles;

/// Global list of stream handles (manages all stream instances).
extern list *stream_handles;

// ============================================================================
// PUBLIC API DECLARATIONS
// ============================================================================

// Stream Management Functions

/// Frees a stream structure and releases its resources.
/// @param st Pointer to stream structure to free
void stream_free(stream_struct *st);

/// Updates a stream's input buffer (reads new data if available).
/// @param stream Pointer to stream structure to update
void stream_update(stream_struct *stream);

/// Sends output data through a stream.
/// @param st Pointer to stream structure
/// @param offset Pointer to data to send
/// @param bytes Number of bytes to send
void stream_out(stream_struct *st, void *offset, ptrszint bytes);

// Connection Management Functions

/// Closes a connection and cleans up associated resources.
/// @param i Connection handle/index to close
void connection_close(ptrszint i);

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

// This module provides handle management for custom QB64 interfaces including:
// - Special handle system for non-file handles (uses negative values)
// - Stream management for network and other streamed data sources
// - Unified input/output buffering for streams
// - Connection lifecycle management
//
// The special handle system allows QB64 to manage custom interfaces (like
// network streams) alongside standard QB file handles by using negative
// handle values to distinguish them.

#endif // INCLUDE_LIBQB_HANDLES_H
