//----------------------------------------------------------------------------------------------------------------------
//    ___  ___   __ _  _  _  ___   ___
//   / _ \| _ ) / /| || || || _ \ / _ \
//  | (_) | _ \/ _ \__ | || ||  _/|  __/
//   \__\_\___/\___/|_||_||_||_|   \___|
//
//  QB64-PE Handle Types Module
//  Common types for special handles, streams, and related structures
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_HANDLES_H
#define INCLUDE_LIBQB_HANDLES_H

#include "../../os.h"
#include "qblist.h"

// Special Handle Types
enum class special_handle_type {
    Invalid,
    Stream,
    Host,
    Http,
};

// Special Handle system
// Purpose: Manage handles to custom QB64 interfaces alongside the standard QB file handle indexing
// Method:  Uses negative values for custom interface handles
struct special_handle_struct {
    special_handle_type type;
    // an index or pointer to the type's object
    //
    // Http streams use this as an EOF flag
    ptrszint index;
};

// Stream Types
enum class stream_type {
    Tcp,
};

// Stream system
// Purpose: Unify access to the input and/or output of streamed data
struct stream_struct {
    uint8 *in;
    ptrszint in_size;  // current size in bytes
    ptrszint in_limit; // size before reallocation of buffer is required
    int8 eof;          // user attempted to read past end of stream
    // Note: 'out' is unrequired because data can be sent directly to the interface
    //-----------------------------------------
    stream_type type;

    ptrszint index; // an index or pointer to the type's object
};

// External declarations for global handle lists
extern list *special_handles;
extern list *stream_handles;

// Stream functions
void stream_free(stream_struct *st);
void stream_update(stream_struct *stream);
void stream_out(stream_struct *st, void *offset, ptrszint bytes);

// Connection functions
void connection_close(ptrszint i);

#endif // INCLUDE_LIBQB_HANDLES_H
