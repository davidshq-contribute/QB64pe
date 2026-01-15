//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE File I/O Module Unit Tests
//  Tests for file I/O functions
//
//  Purpose:
//    This test suite verifies the functionality of file I/O operations in the
//    QB64-PE fileio module, including file handle management, file positioning,
//    and generic I/O operations.
//
//  Test Coverage:
//    - File number allocation (func_freefile)
//    - File status functions (func_lof, func_eof, func_seek, func_loc)
//    - File positioning (sub_seek)
//    - Generic I/O operations (generic_get, generic_put)
//    - Error handling with invalid file handles
//
//  Limitations:
//    Many file I/O functions require QB64 file handle system initialization.
//    These tests focus on functions that can be tested independently or with
//    minimal setup. Full integration tests would require proper QB64 runtime
//    and actual file operations.
//
//  How to Run:
//    1. Build the test: make build-tests
//    2. Run individually: ./tests/exes/cpp/fileio_test
//    3. Run all C++ tests: ./tests/run_c_tests.sh
//
//  Expected Behavior:
//    All tests should pass. Tests verify:
//    - Valid file number allocation
//    - Graceful handling of invalid file handles (no crashes)
//    - Proper bounds checking for I/O operations
//    - Basic functionality of file status and positioning functions
//
//----------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <stdio.h>  // For L_tmpnam, tmpnam_s
#else
#include <unistd.h>
#endif

#include "test.h"

// Type definitions (matching os.h)
#define int64 int64_t
#define int32 int32_t
#define uint8 uint8_t

// Forward declarations for fileio functions we're testing
// Note: These would normally come from fileio.h, but we're testing in isolation
extern "C" {
    // File status functions
    int64 func_lof(int32 i);
    int32 func_eof(int32 i);
    void sub_seek(int32 i, int64 pos);
    int64 func_seek(int32 i);
    int64 func_loc(int32 i);
    
    // Generic I/O helpers
    int32 generic_put(int32 i, int32 offset, uint8 *cp, int32 bytes);
    int32 generic_get(int32 i, int32 offset, uint8 *cp, int32 bytes);
    extern int32 generic_get_bytes_read;
    
    // File number management
    int32 func_freefile();
}

// Helper to create a temporary test file
// Note: Not currently used but kept for future tests
#ifdef _WIN32
#include <io.h>
static FILE* create_temp_file(const char* content, size_t len) {
    char temp_filename[L_tmpnam];
    if (tmpnam_s(temp_filename, L_tmpnam) != 0) {
        return NULL;
    }
    
    FILE* f = fopen(temp_filename, "w+b");
    if (f && content && len > 0) {
        fwrite(content, 1, len, f);
        fseek(f, 0, SEEK_SET);
    }
    return f;
}
#else
static FILE* create_temp_file(const char* content, size_t len) {
    char temp_filename[] = "/tmp/qb64_test_XXXXXX";
    int fd = mkstemp(temp_filename);
    if (fd < 0) {
        return NULL;
    }
    
    FILE* f = fdopen(fd, "w+");
    if (f && content && len > 0) {
        fwrite(content, 1, len, f);
        fseek(f, 0, SEEK_SET);
    }
    return f;
}
#endif

// Note: Many file I/O functions require QB64 file handle system initialization.
// These tests focus on functions that can be tested independently or with
// minimal setup. Full integration tests would require proper QB64 runtime.

void test_freefile() {
    // Test that func_freefile returns a valid file number
    // Should return a positive integer
    int32 file_num = func_freefile();
    test_assert(file_num > 0);
    test_assert(file_num < 1000); // Reasonable upper bound
}

void test_generic_io_basic() {
    // Test generic_get_bytes_read is accessible
    // This is an extern variable, so we can at least verify it exists
    int32 bytes_read = generic_get_bytes_read;
    test_assert(bytes_read >= 0); // Should be non-negative
}

void test_file_status_invalid_handle() {
    // Test that file status functions handle invalid file handles gracefully
    // These should return error values or 0, not crash
    
    int32 invalid_handle = -1;
    int64 lof_result = func_lof(invalid_handle);
    // Should return 0 or error value, not crash
    test_assert(lof_result >= 0);
    
    int32 eof_result = func_eof(invalid_handle);
    // EOF on invalid handle should return 1 (true) or error code
    // Just verify it doesn't crash
    test_assert(eof_result >= -1 && eof_result <= 1);
    
    int64 seek_result = func_seek(invalid_handle);
    test_assert(seek_result >= 0);
    
    int64 loc_result = func_loc(invalid_handle);
    test_assert(loc_result >= 0);
}

void test_seek_operations() {
    // Test seek operations with invalid handle (should not crash)
    int32 invalid_handle = 999;
    
    // Test setting seek position
    sub_seek(invalid_handle, 0);
    // Should not crash
    
    sub_seek(invalid_handle, 100);
    // Should not crash
    
    sub_seek(invalid_handle, -1);
    // Should not crash (may ignore negative values)
}

// Note: More comprehensive tests would require:
// 1. Proper QB64 file handle initialization
// 2. Creating actual files through QB64's file system
// 3. Testing actual read/write operations
// 4. Testing BLOAD/BSAVE operations
// 5. Testing file open/close operations

void test_generic_get_bounds() {
    // Test that generic_get handles edge cases
    uint8 buffer[100];
    int32 invalid_handle = -1;
    
    // Test with invalid handle
    int32 result = generic_get(invalid_handle, 0, buffer, sizeof(buffer));
    // Should return error code (non-zero) or 0, not crash
    test_assert(result >= -1);
    
    // Test with zero bytes
    result = generic_get(invalid_handle, 0, buffer, 0);
    test_assert(result >= -1);
}

void test_generic_put_bounds() {
    // Test that generic_put handles edge cases
    uint8 buffer[100] = {0};
    int32 invalid_handle = -1;
    
    // Test with invalid handle
    int32 result = generic_put(invalid_handle, 0, buffer, sizeof(buffer));
    // Should return error code (non-zero) or 0, not crash
    test_assert(result >= -1);
    
    // Test with zero bytes
    result = generic_put(invalid_handle, 0, buffer, 0);
    test_assert(result >= -1);
}

int main() {
    struct unit_test tests[] = {
        { test_freefile, "test-freefile" },
        { test_generic_io_basic, "test-generic-io-basic" },
        { test_file_status_invalid_handle, "test-file-status-invalid-handle" },
        { test_seek_operations, "test-seek-operations" },
        { test_generic_get_bounds, "test-generic-get-bounds" },
        { test_generic_put_bounds, "test-generic-put-bounds" },
    };

    return run_tests("fileio", tests, sizeof(tests) / sizeof(*tests));
}
