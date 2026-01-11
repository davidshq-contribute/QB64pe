
#include <curl/curl.h>
#include <list>
#include <queue>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unordered_map>

#include "buffer.h"
#include "completion.h"
#include "condvar.h"
#include "http.h"
#include "mutex.h"
#include "thread.h"

/**
 * @file http.cpp
 * @brief Implementation of HTTP client functions for QB64-PE
 * 
 * This file implements HTTP client functionality using libcurl, providing
 * asynchronous HTTP operations with thread-safe data access.
 */

/**
 * @brief HTTP connection handle structure
 * @note Contains CURL connection, I/O buffer, status information, and synchronization primitives.
 */
struct handle {
    int id = 0;

    CURL *con = NULL;

    // Lock protects all the below members
    struct libqb_mutex *io_lock;

    struct libqb_buffer out;
    int closed = 0;
    int err = 0;

    struct completion *response_started = NULL;

    // Reports whether the below info is valid. It won't be if the response
    // hasn't started yet.
    bool has_info = false;

    bool has_content_length = false;
    uint64_t content_length = 0;
    char *url = NULL;

    int status_code = -1;

    handle() {
        io_lock = libqb_mutex_new();
        libqb_buffer_init(&out);
    }

    ~handle() {
        libqb_mutex_free(io_lock);
        libqb_buffer_clear(&out);

        if (url)
            free(url);
    }
};

/**
 * @brief Message to add a new HTTP handle
 * @note Signals the curl thread that a new handle was added, its CURL connection should be started.
 */
struct add_handle {
    int handle;
    int err;

    struct completion added;
};

/**
 * @brief Message to close an HTTP handle
 * @note Signals to the curl thread that a handle is finished and should be closed.
 */
struct close_handle {
    int handle;

    struct completion closed;
};

/**
 * @brief Global CURL multi-handle state
 * @note Manages all HTTP connections and handles thread-safe operations.
 */
struct curl_state {
    CURLM *multi;

    // Lock protects all the below members
    struct libqb_mutex *lock;

    struct std::unordered_map<int, struct handle *> handle_table;
    std::queue<struct add_handle *> add_handle_queue;
    std::queue<struct close_handle *> close_handle_queue;
    int stop_curl;

    curl_state() {
        lock = libqb_mutex_new();
    }
};

/**
 * @brief Fills out all of the 'info' fields in the handle
 * @param handle Handle to fill info for
 * @note Sets the has_info flag and triggers the response_started completion if it exists.
 *       Handle should be locked when calling this function.
 *       Extracts content length, URL, and status code from CURL.
 */
static void __fillout_curl_info(struct handle *handle) {
    if (handle->has_info)
        return;

    handle->has_info = true;

    uint64_t cl;

    // MacOS High Sierra has a version a bit lower than this
#if CURL_AT_LEAST_VERSION(7, 55, 0)
    curl_off_t curloff;
    CURLcode res = curl_easy_getinfo(handle->con, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &curloff);

    cl = (uint64_t)curloff;
#else
    double dbl = 0;
    CURLcode res = curl_easy_getinfo(handle->con, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &dbl);

    cl = (uint64_t)dbl;
#endif

    if (res != CURLE_OK || cl == -1) {
        handle->has_content_length = false;
    } else {
        handle->has_content_length = true;
        handle->content_length = cl;
    }

    char *urlp = NULL;

    res = curl_easy_getinfo(handle->con, CURLINFO_EFFECTIVE_URL, &urlp);

    if (urlp)
        handle->url = strdup(urlp);

    res = curl_easy_getinfo(handle->con, CURLINFO_RESPONSE_CODE, &handle->status_code);

    // If someone is waiting for the info then we let them know
    if (handle->response_started) {
        completion_finish(handle->response_started);
        handle->response_started = NULL;
    }
}

/**
 * @brief Processes the handle addition and deletion lists
 * @param state CURL state structure
 * @note Thread-safe: processes queued handle additions and deletions.
 *       Removes connections from multi-handle and cleans them up.
 */
static void process_handles(struct curl_state *state) {
    std::list<CURL *> connectionsToDrop;

    {
        libqb_mutex_guard guard(state->lock);

        for (; !state->add_handle_queue.empty(); state->add_handle_queue.pop()) {
            struct add_handle *add = state->add_handle_queue.front();

            curl_multi_add_handle(state->multi, state->handle_table[add->handle]->con);

            add->err = 0;
            completion_finish(&add->added);
        }

        for (; !state->close_handle_queue.empty(); state->close_handle_queue.pop()) {
            struct close_handle *close = state->close_handle_queue.front();
            struct handle *handle = state->handle_table[close->handle];

            // If this was already finished, then con will be NULL
            if (handle->con)
                connectionsToDrop.push_back(handle->con);

            state->handle_table.erase(close->handle);

            delete handle;

            completion_finish(&close->closed);
        }
    }

    for (CURL *const &con : connectionsToDrop) {
        // Removing the connection can trigger the callbacks to get run. Due to
        // that we have to call it without holding the lock, or we could
        // deadlock.
        curl_multi_remove_handle(state->multi, con);
        curl_easy_cleanup(con);
    }
}

/**
 * @brief Handles CURL multi-handle messages
 * @param state CURL state structure
 * @note Processes completed connections, fills out connection info, and marks handles as closed.
 *       Thread-safe: uses mutex guards for handle access.
 */
static void handle_messages(struct curl_state *state) {
    CURLMsg *msg;
    int left;

    while (msg = curl_multi_info_read(state->multi, &left), msg) {
        if (msg->msg != CURLMSG_DONE)
            continue;

        CURL *e = msg->easy_handle;

        struct handle *handle;
        curl_easy_getinfo(e, CURLINFO_PRIVATE, &handle);

        handle->con = NULL;

        {
            libqb_mutex_guard guard(handle->io_lock);

            handle->closed = 1;

            // In the event this connection had no data, we want to make sure
            // to fill this out and trigger the completion if there is one.
            __fillout_curl_info(handle);

            switch (msg->data.result) {
            case CURLE_OK:
                break;

            default:
                handle->err = 1;
                break;
            }
        }

        curl_multi_remove_handle(state->multi, e);
        curl_easy_cleanup(e);
    }
}

#if CURL_AT_LEAST_VERSION(7, 68, 0)
/**
 * @brief Polls CURL multi-handle for activity (modern libcurl)
 * @param state CURL state structure
 * @note Uses curl_multi_poll with a one second timeout to avoid accidental deadlocks.
 */
static void curl_state_poll(struct curl_state *state) {
    // We use a one second timeout to avoid any accidental deadlocks if we
    // don't wakeup the thread.
    curl_multi_poll(state->multi, NULL, 0, 1000, NULL);
}

/**
 * @brief Wakes up CURL multi-handle (modern libcurl)
 * @param state CURL state structure
 * @note Uses curl_multi_wakeup to immediately wake the polling thread.
 */
static void curl_state_wakeup(struct curl_state *state) {
    curl_multi_wakeup(state->multi);
}
#else
/**
 * @brief Polls CURL multi-handle for activity (legacy libcurl)
 * @param state CURL state structure
 * @note This is a workaround for libcurl version lacking the curl_multi_poll() and
 *       curl_multi_wakeup() functions. Unfortunately this old version is on OS X,
 *       so we need to support it.
 *       We use curl_multi_wait() with a small timeout, and don't support wakeup (so
 *       commands have to wait for the timeout to trigger).
 *       If numfds from curl_multi_wait() is zero, then we have to do the timeout
 *       manually via usleep().
 */
static void curl_state_poll(struct curl_state *state) {
    int numfds = 0;

    curl_multi_wait(state->multi, NULL, 0, 50, &numfds);

    if (!numfds)
        usleep(50 * 1000);
}

/**
 * @brief Wakes up CURL multi-handle (legacy libcurl)
 * @param state CURL state structure
 * @note NOP for legacy versions - curl_state_poll will timeout automatically.
 */
static void curl_state_wakeup(struct curl_state *state) {
    // NOP, curl_state_poll will timeout automatically
}
#endif

/**
 * @brief Main CURL thread handler
 * @param arg Pointer to curl_state structure
 * @note Runs in a separate thread, continuously polling and processing HTTP connections.
 *       Handles connection lifecycle, data transfer, and cleanup.
 */
static void libqb_curl_thread_handler(void *arg) {
    struct curl_state *state = (struct curl_state *)arg;
    int running_transfers = 0;

    while (!state->stop_curl) {
        curl_state_poll(state);

        // Process handle additions and calls to close()
        process_handles(state);

        // Process requests, performs any read/write operations
        curl_multi_perform(state->multi, &running_transfers);

        // Handle any requests that finished
        handle_messages(state);
    }

    // FIXME: This should do graceful closing for uploads, but because we only support
    // downloads at the moment throwing the data away doesn't matter.
}

/**
 * @brief Global CURL state
 */
static struct curl_state curl_state;

/**
 * @brief CURL thread handle
 */
static struct libqb_thread *curl_thread;

/**
 * @brief CURL write callback - services data received from HTTP connection
 * @param ptr Pointer to received data
 * @param size Size of each element
 * @param nmemb Number of elements
 * @param data User data pointer (handle structure)
 * @return Number of bytes processed
 * @note Writes received data to the handle's buffer. Fills out connection info
 *       on first data received. Thread-safe: uses mutex guard.
 */
static size_t receive_http_block(void *ptr, size_t size, size_t nmemb, void *data) {
    struct handle *handle = (struct handle *)data;
    size_t length = size * nmemb;

    libqb_mutex_guard guard(handle->io_lock);

    libqb_buffer_write(&handle->out, (const char *)ptr, length);

    // The first time this connection starts to receive data we fill out the
    // connection info.
    __fillout_curl_info(handle);

    return length;
}

/**
 * @brief Checks if an HTTP handle ID is valid
 * @param id Handle ID to check
 * @return true if handle exists, false otherwise
 */
static bool is_valid_http_id(int id) {
    return curl_state.handle_table.find(id) != curl_state.handle_table.end();
}

/**
 * @brief Gets the available data length from an HTTP connection
 * @param id Handle ID
 * @param[out] length Output length in bytes
 * @return 0 on success, -1 if handle is invalid
 * @note Thread-safe: uses mutex guard for buffer access.
 */
int libqb_http_get_length(int id, size_t *length) {
    if (!is_valid_http_id(id))
        return -1;

    struct handle *handle = curl_state.handle_table[id];

    libqb_mutex_guard guard(handle->io_lock);

    *length = libqb_buffer_length(&handle->out);

    return 0;
}

/**
 * @brief Waits for handle to have valid info
 * @param handle Handle to wait for
 * @note Waits for connection headers to be received. Info is available after
 *       the connection headers have been received. Uses completion for synchronization.
 */
static void wait_for_info(struct handle *handle) {
    struct completion comp;

    {
        libqb_mutex_guard guard(handle->io_lock);

        if (handle->has_info)
            return;

        completion_init(&comp);
        handle->response_started = &comp;
    }

    completion_wait(&comp);
    completion_clear(&comp);
}

/**
 * @brief Gets the content length from HTTP response headers
 * @param id Handle ID
 * @param[out] length Output content length
 * @return 0 on success, -1 if handle is invalid or content length not available
 * @note Waits for response headers before returning. Thread-safe.
 */
int libqb_http_get_content_length(int id, uint64_t *length) {
    if (!is_valid_http_id(id))
        return -1;

    struct handle *handle = curl_state.handle_table[id];

    wait_for_info(handle);

    {
        libqb_mutex_guard guard(handle->io_lock);

        if (!handle->has_content_length)
            return -1;

        *length = handle->content_length;
        return 0;
    }
}

/**
 * @brief Gets the HTTP status code from response headers
 * @param id Handle ID
 * @return HTTP status code, or -1 if handle is invalid
 * @note Waits for response headers before returning. Thread-safe.
 */
int libqb_http_get_status_code(int id) {
    if (!is_valid_http_id(id))
        return -1;

    struct handle *handle = curl_state.handle_table[id];

    wait_for_info(handle);

    {
        libqb_mutex_guard guard(handle->io_lock);

        return handle->status_code;
    }
}

/**
 * @brief Gets the effective URL from HTTP connection
 * @param id Handle ID
 * @return Effective URL string, or NULL if handle is invalid
 * @note Waits for response headers before returning. Returns the final URL after redirects.
 *       Thread-safe. The returned pointer is valid until the handle is closed.
 */
const char *libqb_http_get_url(int id) {
    if (!is_valid_http_id(id))
        return NULL;

    struct handle *handle = curl_state.handle_table[id];

    wait_for_info(handle);

    {
        libqb_mutex_guard guard(handle->io_lock);

        return handle->url;
    }
}

/**
 * @brief Reads available data from HTTP connection
 * @param id Handle ID
 * @param[out] buf Buffer to read into
 * @param[in,out] length Input: buffer size, Output: bytes actually read
 * @return 0 on success, -1 if handle is invalid
 * @note Reads up to *length bytes. Updates *length with actual bytes read.
 *       Thread-safe: uses mutex guard.
 */
int libqb_http_get(int id, char *buf, size_t *length) {
    if (!is_valid_http_id(id))
        return -1;

    struct handle *handle = curl_state.handle_table[id];

    libqb_mutex_guard guard(handle->io_lock);

    *length = libqb_buffer_read(&handle->out, buf, *length);

    return 0;
}

/**
 * @brief Reads a fixed amount of data from HTTP connection
 * @param id Handle ID
 * @param[out] buf Buffer to read into
 * @param length Exact number of bytes to read
 * @return 0 on success, -1 if handle is invalid or insufficient data available
 * @note Requires exactly 'length' bytes to be available. Thread-safe: uses mutex guard.
 */
int libqb_http_get_fixed(int id, char *buf, size_t length) {
    if (!is_valid_http_id(id))
        return -1;

    struct handle *handle = curl_state.handle_table[id];

    libqb_mutex_guard guard(handle->io_lock);

    size_t total_length = libqb_buffer_length(&handle->out);

    if (total_length < length)
        return -1;

    libqb_buffer_read(&handle->out, buf, length);

    return 0;
}

/**
 * @brief Checks if HTTP connection is still connected
 * @param id Handle ID
 * @return Non-zero if connected, 0 if closed, -1 if handle is invalid
 * @note Thread-safe: uses mutex guard.
 */
int libqb_http_connected(int id) {
    if (!is_valid_http_id(id))
        return -1;

    struct handle *handle = curl_state.handle_table[id];

    libqb_mutex_guard guard(handle->io_lock);

    return !handle->closed;
}

/**
 * @brief Opens an HTTP connection
 * @param url URL to connect to
 * @param id Handle ID to assign
 * @return 0 on success, -1 on error
 * @note Creates a new HTTP connection, configures CURL options, and starts the transfer.
 *       Only allows HTTP and HTTPS protocols. Follows redirects automatically.
 *       Waits for connection to start and response headers to be received.
 *       Thread-safe: uses mutex guards and completion synchronization.
 */
int libqb_http_open(const char *url, int id) {
    struct handle *handle = new struct handle();

    handle->id = id;

    handle->con = curl_easy_init();
    curl_easy_setopt(handle->con, CURLOPT_PRIVATE, handle);
    curl_easy_setopt(handle->con, CURLOPT_URL, url);

    curl_easy_setopt(handle->con, CURLOPT_WRITEFUNCTION, receive_http_block);
    curl_easy_setopt(handle->con, CURLOPT_WRITEDATA, handle);

    // Allow redirects to be followed
    curl_easy_setopt(handle->con, CURLOPT_FOLLOWLOCATION, 1);

    // Only allow HTTP and HTTPS to be used
    curl_easy_setopt(handle->con, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
    curl_easy_setopt(handle->con, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);

    struct add_handle add;
    struct completion info_comp;

    add.handle = id;
    completion_init(&add.added);
    completion_init(&info_comp);

    handle->response_started = &info_comp;

    {
        libqb_mutex_guard guard(curl_state.lock);

        curl_state.handle_table.insert(std::make_pair(id, handle));
        curl_state.add_handle_queue.push(&add);
    }

    // Kick the curl thread so that it processes our new handle
    curl_state_wakeup(&curl_state);

    completion_wait(&add.added);
    completion_clear(&add.added);

    // Wait for the Server's response to begin
    completion_wait(&info_comp);
    completion_clear(&info_comp);

    int errored = 0;

    {
        libqb_mutex_guard guard(handle->io_lock);

        // If this is set, the connection bombed out with some kind of error
        errored = handle->err;
    }

    if (errored) {
        libqb_http_close(id);
        return -1;
    }

    return 0;
}

/**
 * @brief Closes an HTTP connection
 * @param id Handle ID to close
 * @return 0 on success, -1 if handle is invalid
 * @note Queues a close request and waits for it to complete. Cleans up all
 *       resources associated with the handle. Thread-safe: uses mutex guards
 *       and completion synchronization.
 */
int libqb_http_close(int id) {
    if (!is_valid_http_id(id))
        return -1;

    struct close_handle close;

    close.handle = id;
    completion_init(&close.closed);

    {
        libqb_mutex_guard guard(curl_state.lock);

        curl_state.close_handle_queue.push(&close);
    }

    // Kick the curl thread so that it processes our close request
    curl_state_wakeup(&curl_state);

    completion_wait(&close.closed);
    completion_clear(&close.closed);

    return 0;
}

/**
 * @brief Initializes the HTTP subsystem
 * @note Initializes libcurl and starts the CURL thread. Must be called before
 *       any other HTTP functions. Thread-safe initialization.
 */
void libqb_http_init() {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl_state.multi = curl_multi_init();

    curl_thread = libqb_thread_new();
    libqb_thread_start(curl_thread, libqb_curl_thread_handler, &curl_state);
}

/**
 * @brief Stops the HTTP subsystem
 * @note Signals the CURL thread to stop and waits for it to complete.
 *       Cleans up all CURL resources. Thread-safe shutdown.
 */
void libqb_http_stop() {
    {
        libqb_mutex_guard guard(curl_state.lock);

        // Tell CURL to finish up requests
        curl_state.stop_curl = 1;
    }

    curl_state_wakeup(&curl_state);

    // Wait for curl to finish
    libqb_thread_join(curl_thread);

    libqb_thread_free(curl_thread);
}
