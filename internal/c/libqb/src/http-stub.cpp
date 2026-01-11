
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "http.h"

/**
 * @file http-stub.cpp
 * @brief Stub implementation of HTTP functions for QB64-PE
 * 
 * This file provides stub implementations of HTTP functions when libcurl is not available.
 * All functions return error codes or NULL to indicate HTTP is not supported.
 */

/**
 * @brief Initializes HTTP subsystem (stub)
 * @note No-op for stub implementation.
 */
void libqb_http_init() {}

/**
 * @brief Stops HTTP subsystem (stub)
 * @note No-op for stub implementation.
 */
void libqb_http_stop() {}

/**
 * @brief Opens an HTTP connection (stub)
 * @param url URL to connect to (unused)
 * @param handle Handle ID (unused)
 * @return Always -1 (not supported)
 */
int libqb_http_open(const char *url, int handle) {
    (void)url;
    (void)handle;
    return -1;
}

/**
 * @brief Closes an HTTP connection (stub)
 * @param handle Handle ID (unused)
 * @return Always -1 (not supported)
 */
int libqb_http_close(int handle) {
    (void)handle;
    return -1;
}

/**
 * @brief Checks if HTTP connection is connected (stub)
 * @param handle Handle ID (unused)
 * @return Always -1 (not supported)
 */
int libqb_http_connected(int handle) {
    (void)handle;
    return -1;
}

/**
 * @brief Gets available data length (stub)
 * @param handle Handle ID (unused)
 * @param[out] length Output length (set to 0)
 * @return Always -1 (not supported)
 */
int libqb_http_get_length(int handle, size_t *length) {
    (void)handle;
    *length = 0;
    return -1;
}

/**
 * @brief Reads data from HTTP connection (stub)
 * @param handle Handle ID (unused)
 * @param buf Buffer to read into (unused)
 * @param[in,out] length Input: buffer size, Output: bytes read (unused)
 * @return Always -1 (not supported)
 */
int libqb_http_get(int handle, char *buf, size_t *length) {
    (void)handle;
    (void)buf;
    (void)length;
    return -1;
}

/**
 * @brief Reads fixed amount of data from HTTP connection (stub)
 * @param id Handle ID (unused)
 * @param buf Buffer to read into (unused)
 * @param length Number of bytes to read (unused)
 * @return Always -1 (not supported)
 */
int libqb_http_get_fixed(int id, char *buf, size_t length) {
    (void)id;
    (void)buf;
    (void)length;
    return -1;
}

/**
 * @brief Gets content length from HTTP response (stub)
 * @param id Handle ID (unused)
 * @param[out] ptr Output pointer for content length (unused)
 * @return Always -1 (not supported)
 */
int libqb_http_get_content_length(int id, uint64_t *ptr) {
    (void)id;
    (void)ptr;
    return -1;
}

/**
 * @brief Gets HTTP status code (stub)
 * @param id Handle ID (unused)
 * @return Always -1 (not supported)
 */
int libqb_http_get_status_code(int id) {
    (void)id;
    return -1;
}

/**
 * @brief Gets URL of HTTP connection (stub)
 * @param handle Handle ID (unused)
 * @return Always NULL (not supported)
 */
const char *libqb_http_get_url(int handle) {
    (void)handle;
    return NULL;
}
