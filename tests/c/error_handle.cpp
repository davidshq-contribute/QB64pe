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

// Tests for is_critical_oom_error()
static void test_is_critical_oom_error_257() {
    // Error 257 is the generic critical OOM error
    test_assert(is_critical_oom_error(257));
}

static void test_is_critical_oom_error_502_518() {
    // Errors 502-518 are traceable critical OOM errors
    test_assert(is_critical_oom_error(502));
    test_assert(is_critical_oom_error(503));
    test_assert(is_critical_oom_error(504));
    test_assert(is_critical_oom_error(505));
    test_assert(is_critical_oom_error(506));
    test_assert(is_critical_oom_error(507));
    test_assert(is_critical_oom_error(508));
    test_assert(is_critical_oom_error(509));
    test_assert(is_critical_oom_error(510));
    test_assert(is_critical_oom_error(511));
    test_assert(is_critical_oom_error(512));
    test_assert(is_critical_oom_error(513));
    test_assert(is_critical_oom_error(514));
    test_assert(is_critical_oom_error(515));
    test_assert(is_critical_oom_error(516));
    test_assert(is_critical_oom_error(517));
    test_assert(is_critical_oom_error(518));
}

static void test_is_critical_oom_error_boundaries() {
    // Test boundary values - these should NOT be critical OOM errors
    test_assert(!is_critical_oom_error(256));  // Just before 257
    test_assert(!is_critical_oom_error(501));  // Just before 502
    test_assert(!is_critical_oom_error(519));  // Just after 518
}

static void test_is_critical_oom_error_other_errors() {
    // Test that other error codes are not considered critical OOM
    test_assert(!is_critical_oom_error(0));
    test_assert(!is_critical_oom_error(1));
    test_assert(!is_critical_oom_error(11));   // Division by zero
    test_assert(!is_critical_oom_error(53));   // File not found
    test_assert(!is_critical_oom_error(258));  // Invalid handle
    test_assert(!is_critical_oom_error(259));  // Cannot find DLL
    test_assert(!is_critical_oom_error(300));  // Memory region OOR
}

static void test_is_critical_oom_error_negative() {
    // Test negative values (should not be critical OOM)
    test_assert(!is_critical_oom_error(-1));
    test_assert(!is_critical_oom_error(-257));
}

// Tests for get_critical_oom_error_index()
static void test_get_critical_oom_error_index_257() {
    // Error 257 should map to index 1
    test_assert_ints(1, get_critical_oom_error_index(257));
}

static void test_get_critical_oom_error_index_502() {
    // Error 502 should map to index 2
    test_assert_ints(2, get_critical_oom_error_index(502));
}

static void test_get_critical_oom_error_index_518() {
    // Error 518 should map to index 18
    test_assert_ints(18, get_critical_oom_error_index(518));
}

static void test_get_critical_oom_error_index_all() {
    // Test all traceable OOM error codes (502-518) map to correct indices (2-18)
    for (int32_t error_code = 502; error_code <= 518; error_code++) {
        int expected_index = error_code - 500;
        test_assert_ints(expected_index, get_critical_oom_error_index(error_code));
    }
}

static void test_get_critical_oom_error_index_sequential() {
    // Verify indices are sequential from 1 to 18
    int index_257 = get_critical_oom_error_index(257);
    test_assert_ints(1, index_257);

    // Verify 502-518 produce sequential indices 2-18
    for (int i = 0; i < 17; i++) {
        int32_t error_code = 502 + i;
        int expected_index = 2 + i;
        test_assert_ints(expected_index, get_critical_oom_error_index(error_code));
    }
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
        // Critical OOM error detection tests
        {test_is_critical_oom_error_257, "is_critical_oom_error_257"},
        {test_is_critical_oom_error_502_518, "is_critical_oom_error_502_518"},
        {test_is_critical_oom_error_boundaries, "is_critical_oom_error_boundaries"},
        {test_is_critical_oom_error_other_errors, "is_critical_oom_error_other_errors"},
        {test_is_critical_oom_error_negative, "is_critical_oom_error_negative"},
        // Critical OOM error index tests
        {test_get_critical_oom_error_index_257, "get_critical_oom_error_index_257"},
        {test_get_critical_oom_error_index_502, "get_critical_oom_error_index_502"},
        {test_get_critical_oom_error_index_518, "get_critical_oom_error_index_518"},
        {test_get_critical_oom_error_index_all, "get_critical_oom_error_index_all"},
        {test_get_critical_oom_error_index_sequential, "get_critical_oom_error_index_sequential"},
    };

    return run_tests("error_handle", tests, sizeof(tests) / sizeof(tests[0]));
}
