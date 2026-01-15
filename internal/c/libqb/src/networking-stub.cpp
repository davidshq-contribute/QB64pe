//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Networking Stub Module
//  Stub implementation of networking functions when sockets are not available
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#include "libqb-common.h"

#include "networking.h"
#include "handles.h"
#include "qblist.h"
#include "qbs.h"

#include "../../os.h"

#include <cstdlib>

// ============================================================================
// MODULE STATE
// ============================================================================

// Connection handles list (must be defined even when sockets disabled)
list *connection_handles = NULL;

// ============================================================================
// NETWORKING INITIALIZATION (STUB)
// ============================================================================

void networking_init() {
    // Initialize empty connection handles list
    connection_handles = list_new(sizeof(int)); // Minimal size since not used
}

// ============================================================================
// TCP FUNCTIONS (STUBS)
// ============================================================================

void tcp_init() {
    // No-op when sockets disabled
}

void tcp_done() {
    // No-op when sockets disabled
}

int32 tcp_connected(void *connection) {
    (void)connection;
    return 0; // Not connected
}

// ============================================================================
// CONNECTION FUNCTIONS (STUBS)
// ============================================================================

int32 connection_new(int32 method, qbs *info, int32 value) {
    (void)method;
    (void)info;
    (void)value;
    return 0; // Failure
}

int32 func__openclient(qbs *info) {
    (void)info;
    return 0; // Failure
}

int32 func__openhost(qbs *info) {
    (void)info;
    return 0; // Failure
}

int32 func__openconnection(int32 i) {
    (void)i;
    return 0; // No connections available
}

qbs *func__connectionaddress(int32 i) {
    (void)i;
    qbs *tqbs = qbs_new(0, 1);
    return tqbs; // Return empty string
}

int32 func__connected(int32 i) {
    (void)i;
    return 0; // Not connected
}

// ============================================================================
// STREAM FUNCTIONS (STUBS)
// ============================================================================

void stream_free(stream_struct *st) {
    if (st->in_limit)
        free(st->in);
    list_remove(stream_handles, list_get_index(stream_handles, st));
}

void stream_out(stream_struct *st, void *offset, ptrszint bytes) {
    (void)st;
    (void)offset;
    (void)bytes;
    // No-op when sockets disabled
}

void stream_update(stream_struct *stream) {
    (void)stream;
    // No-op when sockets disabled
}

void connection_close(ptrszint i) {
    (void)i;
    // No-op when sockets disabled
}
