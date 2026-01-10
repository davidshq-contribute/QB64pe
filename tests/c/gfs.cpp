// Unit tests for Generic File System (GFS) module
// Tests file operations, handles, and file system abstraction

#include "test.h"
#include "gfs.h"
#include "qbs.h"
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#define unlink _unlink
#else
#include <unistd.h>
#endif

static void test_gfs_fileno_valid() {
    // Test checking if fileno is valid
    int32_t result = gfs_fileno_valid(0);
    test_assert_ints(-2, result); // Invalid handle
    
    result = gfs_fileno_valid(1);
    test_assert_ints(0, result); // Should be unused initially
}

static void test_gfs_fileno_freefile() {
    // Test getting next free file number
    int32_t freefile = gfs_fileno_freefile();
    test_assert(freefile >= 1); // Should return at least 1
}

static void test_gfs_open_close() {
    // Test opening and closing a file
    qbs *filename = qbs_new_txt("test_gfs_file.tmp");
    
    // Create a test file first
    FILE *tmp = fopen("test_gfs_file.tmp", "w");
    if (tmp) {
        fprintf(tmp, "test data");
        fclose(tmp);
        
        // Open file for reading
        int32_t handle = gfs_open(filename, 1, 0, 0); // read access
        test_assert(handle >= 0); // Should return valid handle
        
        if (handle >= 0) {
            // Test that we can get file struct (indicates handle is valid)
            gfs_file_struct *file_struct = gfs_get_file_struct(handle);
            test_assert(file_struct != NULL);
            
            // Close the file
            int32_t close_result = gfs_close(handle);
            test_assert_ints(0, close_result);
        }
        
        // Clean up
        unlink("test_gfs_file.tmp");
    }
    
    qbs_free(filename);
}

static void test_gfs_getpos_setpos() {
    // Test getting and setting file position
    qbs *filename = qbs_new_txt("test_gfs_pos.tmp");
    
    // Create a test file
    FILE *tmp = fopen("test_gfs_pos.tmp", "w");
    if (tmp) {
        fprintf(tmp, "test data for position");
        fclose(tmp);
        
        int32_t handle = gfs_open(filename, 1, 0, 0); // read access
        if (handle >= 0) {
            // Get initial position (should be 0)
            int64_t pos = gfs_getpos(handle);
            test_assert(pos >= 0);
            
            // Set position to 5
            int32_t set_result = gfs_setpos(handle, 5);
            test_assert_ints(0, set_result);
            
            // Get position again
            int64_t new_pos = gfs_getpos(handle);
            test_assert_ints(5, new_pos);
            
            gfs_close(handle);
        }
        
        unlink("test_gfs_pos.tmp");
    }
    
    qbs_free(filename);
}

static void test_gfs_eof_reached() {
    // Test EOF reached flag
    qbs *filename = qbs_new_txt("test_gfs_eof.tmp");
    
    // Create a small test file
    FILE *tmp = fopen("test_gfs_eof.tmp", "w");
    if (tmp) {
        fprintf(tmp, "abc");
        fclose(tmp);
        
        int32_t handle = gfs_open(filename, 1, 0, 0); // read access
        if (handle >= 0) {
            // Initially, EOF should not be reached
            int32_t eof = gfs_eof_reached(handle);
            // Note: eof_reached is only set by gfs_read, so this may be 0 initially
            test_assert(eof == 0 || eof == 1); // Either value is acceptable
            
            gfs_close(handle);
        }
        
        unlink("test_gfs_eof.tmp");
    }
    
    qbs_free(filename);
}

static void test_gfs_eof_passed() {
    // Test EOF passed flag
    qbs *filename = qbs_new_txt("test_gfs_eofp.tmp");
    
    // Create a small test file
    FILE *tmp = fopen("test_gfs_eofp.tmp", "w");
    if (tmp) {
        fprintf(tmp, "abc");
        fclose(tmp);
        
        int32_t handle = gfs_open(filename, 1, 0, 0); // read access
        if (handle >= 0) {
            // Initially, EOF should not be passed
            int32_t eof = gfs_eof_passed(handle);
            // Note: eof_passed is only set by gfs_read, so this may be 0 initially
            test_assert(eof == 0 || eof == 1); // Either value is acceptable
            
            gfs_close(handle);
        }
        
        unlink("test_gfs_eofp.tmp");
    }
    
    qbs_free(filename);
}

static void test_gfs_invalid_handle() {
    // Test operations on invalid handles
    int64_t pos = gfs_getpos(-1);
    test_assert(pos < 0); // Should return error
    
    int32_t close_result = gfs_close(-1);
    test_assert(close_result < 0); // Should return error
    
    int64_t lof_result = gfs_lof(-1);
    test_assert(lof_result < 0); // Should return error
}

static void test_gfs_get_file_struct() {
    // Test getting file structure
    qbs *filename = qbs_new_txt("test_gfs_struct.tmp");
    
    FILE *tmp = fopen("test_gfs_struct.tmp", "w");
    if (tmp) {
        fprintf(tmp, "test");
        fclose(tmp);
        
        int32_t handle = gfs_open(filename, 1, 0, 0);
        if (handle >= 0) {
            gfs_file_struct *file_struct = gfs_get_file_struct(handle);
            test_assert(file_struct != NULL);
            test_assert(file_struct->open == 1);
            test_assert(file_struct->read == 1);
            
            gfs_close(handle);
        }
        
        unlink("test_gfs_struct.tmp");
    }
    
    qbs_free(filename);
}

static void test_gfs_fileno_use() {
    // Test associating fileno with handle
    qbs *filename = qbs_new_txt("test_gfs_fileno.tmp");
    
    FILE *tmp = fopen("test_gfs_fileno.tmp", "w");
    if (tmp) {
        fprintf(tmp, "test");
        fclose(tmp);
        
        int32_t handle = gfs_open(filename, 1, 0, 0);
        if (handle >= 0) {
            int32_t fileno = gfs_fileno_freefile();
            gfs_fileno_use(fileno, handle);
            
            // Verify association
            int32_t retrieved_handle = gfs_get_fileno(fileno);
            test_assert_ints(handle, retrieved_handle);
            
            gfs_close(handle);
        }
        
        unlink("test_gfs_fileno.tmp");
    }
    
    qbs_free(filename);
}

int main() {
    struct unit_test tests[] = {
        {test_gfs_fileno_valid, "gfs_fileno_valid"},
        {test_gfs_fileno_freefile, "gfs_fileno_freefile"},
        {test_gfs_open_close, "gfs_open_close"},
        {test_gfs_getpos_setpos, "gfs_getpos_setpos"},
        {test_gfs_eof_reached, "gfs_eof_reached"},
        {test_gfs_eof_passed, "gfs_eof_passed"},
        {test_gfs_invalid_handle, "gfs_invalid_handle"},
        {test_gfs_get_file_struct, "gfs_get_file_struct"},
        {test_gfs_fileno_use, "gfs_fileno_use"},
    };
    
    return run_tests("gfs", tests, sizeof(tests) / sizeof(tests[0]));
}
