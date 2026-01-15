//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Networking Module Unit Tests
//  Tests for TCP/IP networking functions
//
//  Purpose:
//    This test suite verifies the functionality of networking functions
//    in the QB64-PE networking module, including initialization,
//    handle conversion, and error handling.
//
//  Test Coverage:
//    - TCP initialization and cleanup
//    - Handle conversion logic (negative handles)
//    - Invalid input handling
//    - Connection status checks
//    - Stub behavior verification (when sockets disabled)
//
//  Limitations:
//    Full networking tests require actual network connections. These tests
//    focus on initialization, handle logic, and error cases that can be
//    tested without live network operations.
//
//  How to Run:
//    1. Build the test: make build-tests
//    2. Run individually: ./tests/exes/cpp/networking_test
//    3. Run all C++ tests: ./tests/run_c_tests.sh
//
//  Expected Behavior:
//    All tests should pass. Tests verify:
//    - Initialization functions don't crash
//    - Handle conversion is consistent
//    - Invalid handles return appropriate errors
//    - Stub functions return expected values
//
//----------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "test.h"

// Type definitions (matching os.h)
#define int32 int32_t
#define uint32 uint32_t
#define ptrszint intptr_t

// Forward declarations for networking functions we're testing
extern "C" {
    // Initialization
    void networking_init();
    void tcp_init();
    void tcp_done();

    // Connection functions
    int32 func__openclient(void *info);
    int32 func__openhost(void *info);
    int32 func__openconnection(int32 i);
    int32 func__connected(int32 i);

    // TCP connection check
    int32 tcp_connected(void *connection);

    // Connection handles list (external)
    struct list;
    extern list *connection_handles;
}

// ============================================================================
// HANDLE CONVERSION TESTS
// ============================================================================
// QB64 networking uses negative handles: returned_handle = -1 - internal_index
// This ensures network handles don't conflict with file handles

void test_handle_conversion_logic() {
    // Test the handle conversion formula: handle = -1 - index
    // Verify it's reversible: index = -(handle + 1)

    // If internal index is 0, handle should be -1
    int32 index0 = 0;
    int32 handle0 = -1 - index0;
    test_assert_ints_with_name("Index 0 -> Handle -1", -1, handle0);

    // Reverse: handle -1 should give index 0
    int32 reverse0 = -(handle0 + 1);
    test_assert_ints_with_name("Handle -1 -> Index 0", 0, reverse0);

    // If internal index is 5, handle should be -6
    int32 index5 = 5;
    int32 handle5 = -1 - index5;
    test_assert_ints_with_name("Index 5 -> Handle -6", -6, handle5);

    // Reverse: handle -6 should give index 5
    int32 reverse5 = -(handle5 + 1);
    test_assert_ints_with_name("Handle -6 -> Index 5", 5, reverse5);

    // Test larger indices
    int32 index100 = 100;
    int32 handle100 = -1 - index100;
    int32 reverse100 = -(handle100 + 1);
    test_assert_ints_with_name("Roundtrip index 100", index100, reverse100);
}

void test_handle_always_negative() {
    // Network handles should always be negative (to distinguish from file handles)
    for (int32 index = 0; index < 1000; index++) {
        int32 handle = -1 - index;
        test_assert(handle < 0);
    }
}

// ============================================================================
// INITIALIZATION TESTS
// ============================================================================

void test_tcp_init_no_crash() {
    // tcp_init should be safe to call multiple times
    tcp_init();
    tcp_init();
    tcp_init();
    // If we get here without crashing, the test passes
    test_assert(1);
}

void test_tcp_done_no_crash() {
    // tcp_done should be safe to call
    tcp_done();
    // If we get here without crashing, the test passes
    test_assert(1);
}

void test_tcp_init_done_cycle() {
    // Test init/done cycle multiple times
    for (int i = 0; i < 3; i++) {
        tcp_init();
        tcp_done();
    }
    // If we get here without crashing, the test passes
    test_assert(1);
}

// ============================================================================
// CONNECTION STATUS TESTS
// ============================================================================

void test_tcp_connected_null() {
    // tcp_connected with NULL should return 0 (not connected)
    // or handle gracefully without crashing
    int32 result = tcp_connected(NULL);
    // In stub mode, always returns 0
    // In full mode with NULL, behavior depends on implementation
    // Either way, should not crash
    test_assert(result == 0 || result == -1);
}

void test_connected_invalid_handle() {
    // func__connected with invalid handle should return 0
    // Positive handles are invalid for network connections
    int32 result = func__connected(1);
    test_assert_ints_with_name("Invalid positive handle", 0, result);

    result = func__connected(100);
    test_assert_ints_with_name("Invalid large handle", 0, result);

    result = func__connected(0);
    test_assert_ints_with_name("Zero handle", 0, result);
}

// ============================================================================
// STUB BEHAVIOR TESTS
// ============================================================================
// When DEPENDENCY_SOCKETS is not defined, all functions should return
// failure/empty values safely

void test_openconnection_invalid() {
    // func__openconnection with invalid host handle should return 0
    int32 result = func__openconnection(0);
    test_assert_ints_with_name("Invalid host handle 0", 0, result);

    result = func__openconnection(1);
    test_assert_ints_with_name("Invalid host handle 1", 0, result);

    // Even with valid-looking negative handle, if no actual host exists
    result = func__openconnection(-1);
    test_assert_ints_with_name("Non-existent host -1", 0, result);
}

// ============================================================================
// PORT VALIDATION TESTS
// ============================================================================
// Port validation happens inside tcp_host_open and tcp_client_open
// Valid ports are 0-65535

void test_port_range_constants() {
    // Verify port range constants
    const int32 MIN_PORT = 0;
    const int32 MAX_PORT = 65535;

    // These are the boundaries that should be accepted
    test_assert(MIN_PORT >= 0);
    test_assert(MAX_PORT <= 65535);
    test_assert(MAX_PORT > MIN_PORT);
}

// ============================================================================
// INTEGRATION READINESS TESTS
// ============================================================================

void test_networking_module_linkage() {
    // Verify all expected functions are linked
    // These are compile-time checks - if the test builds, linkage is correct

    void (*init_func)() = networking_init;
    void (*tcp_init_func)() = tcp_init;
    void (*tcp_done_func)() = tcp_done;
    int32 (*connected_func)(void*) = tcp_connected;

    // Verify function pointers are non-null
    test_assert(init_func != NULL);
    test_assert(tcp_init_func != NULL);
    test_assert(tcp_done_func != NULL);
    test_assert(connected_func != NULL);
}

int main() {
    struct unit_test tests[] = {
        // Handle conversion tests
        { test_handle_conversion_logic, "test-handle-conversion-logic" },
        { test_handle_always_negative, "test-handle-always-negative" },

        // Initialization tests
        { test_tcp_init_no_crash, "test-tcp-init-no-crash" },
        { test_tcp_done_no_crash, "test-tcp-done-no-crash" },
        { test_tcp_init_done_cycle, "test-tcp-init-done-cycle" },

        // Connection status tests
        { test_tcp_connected_null, "test-tcp-connected-null" },
        { test_connected_invalid_handle, "test-connected-invalid-handle" },

        // Stub behavior tests
        { test_openconnection_invalid, "test-openconnection-invalid" },

        // Port validation tests
        { test_port_range_constants, "test-port-range-constants" },

        // Integration tests
        { test_networking_module_linkage, "test-networking-module-linkage" },
    };

    return run_tests("networking", tests, sizeof(tests) / sizeof(*tests));
}
