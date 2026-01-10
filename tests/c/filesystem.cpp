// Unit tests for filesystem module
// Tests file system operations like directory existence, file operations, etc.

#include "test.h"
#include "filesystem.h"
#include "qbs.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

static void test_fs_savestartdirectory() {
    // Test saving start directory
    FS_SaveStartDirectory();
    test_assert(1); // If we get here, it didn't crash
}

static void test_fs_directoryexists() {
    // Test checking if directory exists
    bool exists = FS_DirectoryExists("/");
    test_assert(exists); // Root directory should exist
    
    bool not_exists = FS_DirectoryExists("/nonexistent_directory_12345");
    test_assert(!not_exists);
}

static void test_fs_fileexists() {
    // Test checking if file exists
    // Create a temporary test file
    FILE *tmp = fopen("test_file_exists.tmp", "w");
    if (tmp) {
        fclose(tmp);
        
        bool exists = FS_FileExists("test_file_exists.tmp");
        test_assert(exists);
        
        // Clean up
        unlink("test_file_exists.tmp");
    }
    
    bool not_exists = FS_FileExists("nonexistent_file_12345.tmp");
    test_assert(!not_exists);
}

static void test_func__cwd() {
    // Test getting current working directory
    qbs *cwd = func__cwd();
    test_assert(cwd != NULL);
    test_assert(cwd->len > 0);
    qbs_free(cwd);
}

static void test_func__direxists() {
    // Test checking directory existence via QB64 function
    qbs *root = qbs_new_txt("/");
    int32_t exists = func__direxists(root);
    test_assert_ints(1, exists); // Should exist
    qbs_free(root);
    
    qbs *nonexistent = qbs_new_txt("/nonexistent_dir_12345");
    int32_t not_exists = func__direxists(nonexistent);
    test_assert_ints(0, not_exists); // Should not exist
    qbs_free(nonexistent);
}

static void test_func__fileexists() {
    // Test checking file existence via QB64 function
    // Create a temporary test file
    FILE *tmp = fopen("test_fileexists.tmp", "w");
    if (tmp) {
        fclose(tmp);
        
        qbs *filename = qbs_new_txt("test_fileexists.tmp");
        int32_t exists = func__fileexists(filename);
        test_assert_ints(1, exists);
        qbs_free(filename);
        
        // Clean up
        unlink("test_fileexists.tmp");
    }
    
    qbs *nonexistent = qbs_new_txt("nonexistent_file_12345.tmp");
    int32_t not_exists = func__fileexists(nonexistent);
    test_assert_ints(0, not_exists);
    qbs_free(nonexistent);
}

static void test_func__startdir() {
    // Test getting start directory
    FS_SaveStartDirectory();
    qbs *startdir = func__startdir();
    test_assert(startdir != NULL);
    test_assert(startdir->len > 0);
    qbs_free(startdir);
}

static void test_func__fullpath() {
    // Test getting full path
    qbs *relative = qbs_new_txt(".");
    qbs *fullpath = func__fullpath(relative);
    test_assert(fullpath != NULL);
    test_assert(fullpath->len > 0);
    qbs_free(relative);
    qbs_free(fullpath);
}

static void test_sub_mkdir() {
    // Test creating directory
    qbs *dirname = qbs_new_txt("test_mkdir_dir");
    sub_mkdir(dirname);
    
    // Verify it was created
    bool exists = FS_DirectoryExists("test_mkdir_dir");
    test_assert(exists);
    
    // Clean up
    rmdir("test_mkdir_dir");
    qbs_free(dirname);
}

static void test_sub_rmdir() {
    // Test removing directory
    // First create a directory
    mkdir("test_rmdir_dir", 0755);
    
    qbs *dirname = qbs_new_txt("test_rmdir_dir");
    sub_rmdir(dirname);
    
    // Verify it was removed
    bool exists = FS_DirectoryExists("test_rmdir_dir");
    test_assert(!exists);
    
    qbs_free(dirname);
}

int main() {
    struct unit_test tests[] = {
        {test_fs_savestartdirectory, "FS_SaveStartDirectory"},
        {test_fs_directoryexists, "FS_DirectoryExists"},
        {test_fs_fileexists, "FS_FileExists"},
        {test_func__cwd, "func__cwd"},
        {test_func__direxists, "func__direxists"},
        {test_func__fileexists, "func__fileexists"},
        {test_func__startdir, "func__startdir"},
        {test_func__fullpath, "func__fullpath"},
        {test_sub_mkdir, "sub_mkdir"},
        {test_sub_rmdir, "sub_rmdir"},
    };
    
    return run_tests("filesystem", tests, sizeof(tests) / sizeof(tests[0]));
}
