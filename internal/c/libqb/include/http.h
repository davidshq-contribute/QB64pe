#ifndef INCLUDE_LIBQB_HTTP_H
#define INCLUDE_LIBQB_HTTP_H

#include <stdint.h>

/**
 * @file http.h
 * @brief HTTP client functions for QB64-PE
 * 
 * This header provides functions for making HTTP requests and reading responses.
 * All functions return 0 on success and a negative error code on failure.
 */

/**
 * @brief Initializes the HTTP system
 * @note Must be called before using any HTTP functions. Sets up the HTTP client infrastructure.
 */
void libqb_http_init();

/**
 * @brief Stops the HTTP system
 * @note Cleans up HTTP resources. Should be called when HTTP functionality is no longer needed.
 */
void libqb_http_stop();

/**
 * @brief Opens an HTTP connection
 * @param url URL to connect to
 * @param handle Unique handle identifier for this connection
 * @return 0 on success, negative error code on failure
 * @note The handle must be unique and is used to identify this connection in subsequent calls.
 */
int libqb_http_open(const char *url, int handle);

/**
 * @brief Closes an HTTP connection
 * @param handle Handle of the connection to close
 * @return 0 on success, negative error code on failure
 * @note Releases resources associated with the connection. Do not use the handle after closing.
 */
int libqb_http_close(int handle);

/**
 * @brief Checks if an HTTP connection is established
 * @param handle Handle of the connection to check
 * @return Non-zero if connected, 0 if not connected
 * @note Returns non-zero if the connection is established and ready for data transfer.
 */
int libqb_http_connected(int handle);

/**
 * @brief Gets the length of bytes waiting to be read
 * @param handle Handle of the connection
 * @param[out] length Pointer to store the length of available bytes
 * @return 0 on success, negative error code on failure
 * @note More bytes may arrive after calling this function, but you're guaranteed to have
 *       at least this many bytes available to read.
 */
int libqb_http_get_length(int handle, size_t *length);

/**
 * @brief Gets the Content-Length value from the HTTP header
 * @param handle Handle of the connection
 * @param[out] length Pointer to store the content length
 * @return 0 on success, negative error code on failure (e.g., if header was not provided)
 * @note Returns an error if the Content-Length header was not provided in the response.
 */
int libqb_http_get_content_length(int handle, uint64_t *length);

/**
 * @brief Gets the HTTP status code from the response
 * @param handle Handle of the connection
 * @return Positive status code (e.g., 200, 404), or -1 if no status code was available
 * @note Returns -1 if the connection was unsuccessful or no status code was received.
 */
int libqb_http_get_status_code(int handle);

/**
 * @brief Gets the effective URL after redirects
 * @param handle Handle of the connection
 * @return Pointer to the effective URL string, or NULL if it could not be resolved
 * @note The returned string is only valid for the lifetime of this handle.
 *       This is the actual URL connected to after following redirects.
 */
const char *libqb_http_get_url(int handle);

/**
 * @brief Reads up to length bytes from the HTTP response
 * @param handle Handle of the connection
 * @param[out] buf Buffer to store the read data
 * @param[in,out] length On input: maximum bytes to read. On output: actual bytes read.
 * @return 0 on success, negative error code on failure
 * @note The length parameter is modified to reflect the actual number of bytes read,
 *       which may be less than requested if fewer bytes are available.
 */
int libqb_http_get(int handle, char *buf, size_t *length);

/**
 * @brief Reads exactly length bytes from the HTTP response
 * @param handle Handle of the connection
 * @param[out] buf Buffer to store the read data
 * @param length Exact number of bytes to read
 * @return 0 on success, negative error code on failure
 * @note Returns an error if fewer than length bytes are available to read.
 *       This function blocks until exactly length bytes are read or an error occurs.
 */
int libqb_http_get_fixed(int handle, char *buf, size_t length);

#endif
