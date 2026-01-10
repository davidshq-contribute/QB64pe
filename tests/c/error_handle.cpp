// Unit tests for error_handle module
// Tests error handling, error codes, and error state management

#include "test.h"
#include "error_handle.h"

static void test_error_codes() {
    // Test that error codes are defined correctly
    test_assert_ints(1, QB_ERROR_NEXT_WITHOUT_FOR);
    test_assert_ints(2, QB_ERROR_SYNTAX_ERROR);
    test_assert_ints(5, QB_ERROR_ILLEGAL_FUNCTION_CALL);
    test_assert_ints(11, QB_ERROR_DIVISION_BY_ZERO);
    test_assert_ints(53, QB_ERROR_FILE_NOT_FOUND);
}

static void test_is_error_pending() {
    // Initially, no error should be pending
    clear_error();
    test_assert(!is_error_pending());
}

static void test_clear_error() {
    // Clear error should reset error state
    clear_error();
    test_assert(!is_error_pending());
    test_assert_ints(0, new_error);
}

static void test_get_error_erl() {
    // Test getting error line (should return 0 when no error)
    clear_error();
    double erl = get_error_erl();
    test_assert(erl >= 0);
}

static void test_get_error_err() {
    // Test getting error code (should return 0 when no error)
    clear_error();
    uint32_t err = get_error_err();
    test_assert_ints(0, err);
}

static void test_func__errorline() {
    // Test getting error line number
    clear_error();
    int32_t line = func__errorline();
    test_assert(line >= 0);
}

static void test_func__inclerrorline() {
    // Test getting include error line number
    clear_error();
    int32_t line = func__inclerrorline();
    test_assert(line >= 0);
}

static void test_func__inclerrorfile() {
    // Test getting include error file
    clear_error();
    qbs *file = func__inclerrorfile();
    test_assert(file != NULL);
    qbs_free(file);
}

static void test_func__errormessage() {
    // Test getting error message for a specific error code
    qbs *msg = func__errormessage(QB_ERROR_SYNTAX_ERROR, 1);
    test_assert(msg != NULL);
    test_assert(msg->len > 0);
    qbs_free(msg);
    
    // Test with another error code
    msg = func__errormessage(QB_ERROR_DIVISION_BY_ZERO, 1);
    test_assert(msg != NULL);
    test_assert(msg->len > 0);
    qbs_free(msg);
}

static void test_error_set_line() {
    // Test setting error line information
    error_set_line(42, 10, "test_file.bas");
    
    int32_t line = func__errorline();
    test_assert_ints(42, line);
    
    int32_t incl_line = func__inclerrorline();
    test_assert_ints(10, incl_line);
    
    qbs *file = func__inclerrorfile();
    test_assert(file != NULL);
    test_assert_buffers("test_file.bas", (const char *)file->chr, file->len);
    qbs_free(file);
    
    clear_error();
}

int main() {
    struct unit_test tests[] = {
        {test_error_codes, "error_codes"},
        {test_is_error_pending, "is_error_pending"},
        {test_clear_error, "clear_error"},
        {test_get_error_erl, "get_error_erl"},
        {test_get_error_err, "get_error_err"},
        {test_func__errorline, "func__errorline"},
        {test_func__inclerrorline, "func__inclerrorline"},
        {test_func__inclerrorfile, "func__inclerrorfile"},
        {test_func__errormessage, "func__errormessage"},
        {test_error_set_line, "error_set_line"},
    };
    
    return run_tests("error_handle", tests, sizeof(tests) / sizeof(tests[0]));
}
