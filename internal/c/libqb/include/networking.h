//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Networking Module
//  TCP/IP socket networking functions
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_NETWORKING_H
#define INCLUDE_LIBQB_NETWORKING_H

// ============================================================================
// DEPENDENCIES
// ============================================================================

#include "../../os.h"

// Forward declarations
struct qbs;

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

struct list;

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

/// Connection handles list (managed by networking module)
extern list *connection_handles;

// ============================================================================
// PUBLIC API DECLARATIONS
// ============================================================================

// Networking Initialization

/// Initializes the networking subsystem (call during program startup).
void networking_init();

// TCP Initialization/Cleanup

/// Initializes the TCP/IP networking subsystem.
/// Called automatically on first network operation.
void tcp_init();

/// Cleans up the TCP/IP networking subsystem.
/// Should be called during program shutdown.
void tcp_done();

// Connection Management

/// Creates a new network connection.
/// @param method Connection method: 0=_OPENCLIENT, 1=_OPENHOST, 2=_OPENCONNECTION
/// @param info Connection info string (e.g., "TCP/IP:12345:host.example.com")
/// @param value Host handle for _OPENCONNECTION method
/// @returns Handle (>0), 0 on failure, -1 on invalid arguments
int32 connection_new(int32 method, qbs *info, int32 value);

// QB64 Network Functions

/// Opens a client connection to a server.
/// @param info Connection string (e.g., "TCP/IP:12345:host.example.com")
/// @returns Negative handle on success, 0 on failure
int32 func__openclient(qbs *info);

/// Opens a host (server) listening on a port.
/// @param info Host string (e.g., "TCP/IP:12345")
/// @returns Negative handle on success, 0 on failure
int32 func__openhost(qbs *info);

/// Accepts a connection from a listening host.
/// @param i Host handle
/// @returns Negative handle on success, 0 if no pending connections
int32 func__openconnection(int32 i);

/// Gets the address information for a connection.
/// @param i Connection handle
/// @returns String with connection address info
qbs *func__connectionaddress(int32 i);

/// Checks if a connection is still active.
/// @param i Connection handle
/// @returns -1 (true) if connected, 0 if disconnected
int32 func__connected(int32 i);

// Low-level TCP Functions (used internally)

/// Checks if a TCP connection is active.
/// @param connection TCP connection pointer
/// @returns Non-zero if connected
int32 tcp_connected(void *connection);

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

// This module provides TCP/IP networking functionality for QB64 programs:
// - Client connections (_OPENCLIENT)
// - Server/host connections (_OPENHOST)
// - Connection acceptance (_OPENCONNECTION)
// - Connection status checking (_CONNECTED)
// - Address information (_CONNECTIONADDRESS$)
//
// Connections use negative handle values to distinguish them from
// standard QB64 file handles.
//
// Platform support:
// - Windows: Uses Winsock2 API
// - Unix/Linux/macOS: Uses BSD sockets
//
// HTTP connections are also supported via libcurl (separate module).

#endif // INCLUDE_LIBQB_NETWORKING_H
