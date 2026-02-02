# NETWORKING.md

Documentation of the QB64pe networking system implementation.

## Overview

QB64pe provides TCP/IP networking via `_OPENCLIENT`, `_OPENHOST`, and `_OPENCONNECTION` functions, plus HTTP support through libcurl integration.

## TCP/IP Architecture

### Connection Types

| Type | Function | Description |
|------|----------|-------------|
| Client | `_OPENCLIENT` | Connect to remote server |
| Host | `_OPENHOST` | Listen for connections |
| Connection | `_OPENCONNECTION` | Accept incoming connection |

### Handle System

Network handles use negative file numbers:
- Stored in `special_handles` list
- Compatible with GET/PUT/CLOSE
- Range: -2 to -MAX_HANDLES

## Socket Structure

```c
struct connection_struct {
    int32 handle;                // Socket descriptor
    int32 type;                  // 1=client, 2=host, 3=connection
    int32 connected;             // Connection status
    int32 protocol;              // TCP=1, UDP=2
    char *host;                  // Host name/IP
    int32 port;                  // Port number
    uint8 *buffer;               // Read buffer
    int32 buffer_size;           // Buffer allocation
    int32 buffer_used;           // Data in buffer
};

connection_struct connections[256];
int32 connection_count;
```

## Client Connections

### _OPENCLIENT

```c
int32 func__openclient(qbs *connection_string);
```

**Syntax:**
```basic
handle& = _OPENCLIENT("TCP/IP:port:host")
```

**Implementation:**
1. Parse connection string
2. Resolve hostname via `gethostbyname()`
3. Create socket with `socket()`
4. Connect with `connect()`
5. Set non-blocking mode
6. Return negative handle

**Example:**
```basic
client& = _OPENCLIENT("TCP/IP:80:www.example.com")
IF client& = 0 THEN PRINT "Connection failed"
```

## Host (Server) Connections

### _OPENHOST

```c
int32 func__openhost(qbs *connection_string);
```

**Syntax:**
```basic
handle& = _OPENHOST("TCP/IP:port")
```

**Implementation:**
1. Parse connection string
2. Create socket
3. Set SO_REUSEADDR option
4. Bind to port with `bind()`
5. Start listening with `listen()`
6. Return negative handle

**Example:**
```basic
host& = _OPENHOST("TCP/IP:12345")
IF host& = 0 THEN PRINT "Could not start server"
```

### _OPENCONNECTION

```c
int32 func__openconnection(int32 host_handle);
```

**Non-blocking accept:**
1. Call `accept()` on host socket
2. If connection pending, return new handle
3. If no connection, return 0

**Example:**
```basic
DO
    newconn& = _OPENCONNECTION(host&)
    IF newconn& THEN
        PRINT "New client connected"
    END IF
    _LIMIT 60
LOOP
```

## Data Transfer

### GET Statement (Read)

```c
void sub_get(int32 handle, int64 offset, void *element, int32 passed);
```

For network handles:
1. Checks if data available via `select()`
2. Reads into element via `recv()`
3. Returns actual bytes read

### PUT Statement (Write)

```c
void sub_put(int32 handle, int64 offset, void *element, int32 passed);
```

For network handles:
1. Sends data via `send()`
2. Handles partial sends
3. Returns on completion or error

### LOF Function

```c
int64 func_lof(int32 handle);
```

For network handles:
- Returns bytes available to read
- Uses `ioctl(FIONREAD)` or `ioctlsocket()`

### EOF Function

```c
int32 func_eof(int32 handle);
```

For network handles:
- Returns -1 if connection closed
- Returns 0 if still connected

## Connection Status

### _CONNECTED Function

```c
int32 func__connected(int32 handle);
```

**Returns:**
- -1: Connected
- 0: Not connected or invalid handle

**Implementation:**
```c
int32 func__connected(int32 handle) {
    // Check socket state via select() or getsockopt()
    int error = 0;
    socklen_t len = sizeof(error);
    getsockopt(socket, SOL_SOCKET, SO_ERROR, &error, &len);
    return (error == 0) ? -1 : 0;
}
```

## HTTP Support

### libcurl Integration

**Location:** `internal/c/parts/network/`

```c
#include <curl/curl.h>

CURL *curl_handle;
```

### _OPENCLIENT with HTTP

```basic
handle& = _OPENCLIENT("HTTP:http://example.com/page")
```

**Implementation:**
1. Initialize libcurl
2. Set URL and options
3. Perform request
4. Buffer response
5. Return file-like handle

### HTTP Methods

```c
void http_get(const char *url, uint8 **data, int32 *size);
void http_post(const char *url, const char *post_data,
               uint8 **data, int32 *size);
```

## Platform Differences

### Windows

```c
#include <winsock2.h>
#include <ws2tcpip.h>

WSADATA wsaData;
WSAStartup(MAKEWORD(2, 2), &wsaData);

#define close(s) closesocket(s)
#define ioctl(s, cmd, arg) ioctlsocket(s, cmd, arg)
```

### Unix/Linux/macOS

```c
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
```

### Non-Blocking Mode

**Windows:**
```c
u_long mode = 1;
ioctlsocket(socket, FIONBIO, &mode);
```

**Unix:**
```c
int flags = fcntl(socket, F_GETFL, 0);
fcntl(socket, F_SETFL, flags | O_NONBLOCK);
```

## Protocol Details

### TCP Connection String Format

```
TCP/IP:port:hostname
```

| Component | Required | Description |
|-----------|----------|-------------|
| TCP/IP | Yes | Protocol identifier |
| port | Yes | Port number (1-65535) |
| hostname | Client only | Host name or IP address |

### Address Resolution

```c
struct hostent *host = gethostbyname(hostname);
struct sockaddr_in addr;
addr.sin_family = AF_INET;
addr.sin_port = htons(port);
addr.sin_addr = *((struct in_addr *)host->h_addr);
```

## Buffer Management

### Read Buffer

```c
#define NET_BUFFER_SIZE 65536

uint8 net_buffer[NET_BUFFER_SIZE];
int32 net_buffer_pos;
int32 net_buffer_len;
```

### Buffered Read

```c
int32 net_read(int32 handle, void *data, int32 size) {
    // First check local buffer
    if (buffer_len > 0) {
        // Return from buffer
    }
    // Then check socket
    return recv(socket, data, size, 0);
}
```

## Error Handling

### Socket Errors

| Error | Cause |
|-------|-------|
| 5 | Illegal function call |
| 52 | Bad file name or number |
| 53 | File not found (connection failed) |
| 54 | Bad file mode |
| 57 | Device I/O error |
| 68 | Device unavailable |

### Error Checking

```c
int32 check_socket_error(int socket) {
    int error = 0;
    socklen_t len = sizeof(error);
    getsockopt(socket, SOL_SOCKET, SO_ERROR, &error, &len);
    return error;
}
```

## Closing Connections

### CLOSE Statement

```c
void sub_close(int32 handle, int32 passed);
```

For network handles:
1. Shutdown socket with `shutdown()`
2. Close socket with `close()`/`closesocket()`
3. Free connection structure
4. Remove from special_handles

## Key Source Files

| File | Content |
|------|---------|
| `internal/c/libqb.cpp` | Network functions |
| `internal/c/parts/network/` | libcurl integration |
| `source/subs_functions/subs_functions.bas` | Function registration |

## Example Usage

### Simple Client

```basic
client& = _OPENCLIENT("TCP/IP:80:www.example.com")
IF client& THEN
    request$ = "GET / HTTP/1.0" + CHR$(13) + CHR$(10) + CHR$(13) + CHR$(10)
    PUT #client&, , request$

    DO UNTIL EOF(client&)
        GET #client&, , response$
        PRINT response$;
    LOOP
    CLOSE client&
END IF
```

### Simple Server

```basic
host& = _OPENHOST("TCP/IP:12345")
IF host& = 0 THEN END

PRINT "Waiting for connections..."

DO
    client& = _OPENCONNECTION(host&)
    IF client& THEN
        PRINT "Client connected!"
        msg$ = "Hello from QB64pe!"
        PUT #client&, , msg$
        CLOSE client&
    END IF
    _LIMIT 60
LOOP UNTIL INKEY$ = CHR$(27)

CLOSE host&
```

### Echo Server

```basic
host& = _OPENHOST("TCP/IP:7")  ' Echo port

DO
    newclient& = _OPENCONNECTION(host&)
    IF newclient& THEN
        DO UNTIL EOF(newclient&)
            IF LOF(newclient&) > 0 THEN
                GET #newclient&, , data$
                PUT #newclient&, , data$  ' Echo back
            END IF
            _LIMIT 100
        LOOP
        CLOSE newclient&
    END IF
    _LIMIT 60
LOOP
```

### Check Connection Status

```basic
client& = _OPENCLIENT("TCP/IP:80:example.com")

DO
    IF _CONNECTED(client&) THEN
        PRINT "Connected"
    ELSE
        PRINT "Disconnected"
        EXIT DO
    END IF
    _LIMIT 1
LOOP

CLOSE client&
```
