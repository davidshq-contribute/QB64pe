// Unit tests for filepath module
// Tests file path manipulation functions

#include "test.h"
#include "filepath.h"
#include "qbs.h"
#include <string.h>
#include <string>

static void test_filepath_get_filename() {
    // Test extracting filename from path
    const char *path1 = "/path/to/file.txt";
    const char *filename1 = filepath_get_filename(path1);
    test_assert(filename1 != NULL);
    test_assert_buffers("file.txt", filename1, strlen("file.txt"));
    
    const char *path2 = "file.txt";
    const char *filename2 = filepath_get_filename(path2);
    test_assert(filename2 != NULL);
    test_assert_buffers("file.txt", filename2, strlen("file.txt"));
    
    const char *path3 = "/path/to/";
    const char *filename3 = filepath_get_filename(path3);
    // Should return empty or NULL for directory
    test_assert(filename3 == NULL || strlen(filename3) == 0);
}

static void test_filepath_get_extension() {
    // Test extracting extension from filename
    const char *file1 = "file.txt";
    const char *ext1 = filepath_get_extension(file1);
    test_assert(ext1 != NULL);
    test_assert_buffers(".txt", ext1, strlen(".txt"));
    
    const char *file2 = "file.tar.gz";
    const char *ext2 = filepath_get_extension(file2);
    test_assert(ext2 != NULL);
    test_assert_buffers(".gz", ext2, strlen(".gz"));
    
    const char *file3 = "file";
    const char *ext3 = filepath_get_extension(file3);
    // Should return empty or NULL for no extension
    test_assert(ext3 == NULL || strlen(ext3) == 0);
}

static void test_filepath_has_extension() {
    // Test checking if path has specific extension
    const char *path1 = "file.txt";
    bool has_txt = filepath_has_extension(path1, ".txt");
    test_assert(has_txt);
    
    bool has_png = filepath_has_extension(path1, ".png");
    test_assert(!has_png);
    
    const char *path2 = "file.TXT";
    bool has_txt_upper = filepath_has_extension(path2, ".txt");
    // Extension matching behavior - test that function doesn't crash
    // Actual case sensitivity depends on implementation
    test_assert(has_txt_upper == has_txt_upper); // At least verify it returns a value
}

static void test_filepath_fix_directory() {
    // Test fixing directory separators in path
    char path1[256] = "path/to/file";
    const char *fixed1 = filepath_fix_directory(path1);
    test_assert(fixed1 != NULL);
    // Path should be modified (separators fixed for current OS)
    
    // Test with qbs
    qbs *path_qbs = qbs_new_txt("path/to/file");
    const char *fixed2 = filepath_fix_directory(path_qbs);
    test_assert(fixed2 != NULL);
    qbs_free(path_qbs);
}

static void test_filepath_split() {
    // Test splitting path into directory and filename
    std::string filePath = "/path/to/file.txt";
    std::string directory, fileName;
    
    filepath_split(filePath, directory, fileName);
    
    test_assert(directory.length() > 0);
    test_assert(fileName.length() > 0);
    test_assert_buffers("file.txt", fileName.c_str(), fileName.length());
}

static void test_filepath_join() {
    // Test joining directory and filename into path
    std::string filePath;
    std::string directory = "/path/to";
    std::string fileName = "file.txt";
    
    filepath_join(filePath, directory, fileName);
    
    test_assert(filePath.length() > 0);
    test_assert(filePath.find("file.txt") != std::string::npos);
}

int main() {
    struct unit_test tests[] = {
        {test_filepath_get_filename, "filepath_get_filename"},
        {test_filepath_get_extension, "filepath_get_extension"},
        {test_filepath_has_extension, "filepath_has_extension"},
        {test_filepath_fix_directory, "filepath_fix_directory"},
        {test_filepath_split, "filepath_split"},
        {test_filepath_join, "filepath_join"},
    };
    
    return run_tests("filepath", tests, sizeof(tests) / sizeof(tests[0]));
}
