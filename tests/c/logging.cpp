// Unit tests for logging module
// Tests logging functionality, levels, scopes, and handlers

#include "test.h"
#include "logging.h"
#include "qbs.h"
#include <stdlib.h>
#include <string.h>

static void test_logging_init() {
    // Test logging initialization
    // This should not crash
    libqb_log_init();
    test_assert(1); // If we get here, init succeeded
}

static void test_func__logminlevel() {
    // Test getting minimum log level
    // Note: libqb_log_init() already called in previous test
    uint32_t level = func__logminlevel();
    // Should return 1-5 (1=Trace, 2=Information, 3=Warning, 4=Error, 5=disabled)
    test_assert(level >= 1 && level <= 5);
}

static void test_libqb_log_basic() {
    // Test basic logging (should not crash even if no handlers)
    // Note: libqb_log_init() already called in previous test
    
    // These should not crash even if logging is disabled
    libqb_log(loglevel::Trace, logscope::Libqb, __FILE__, __func__, __LINE__, "Test trace message");
    libqb_log(loglevel::Information, logscope::Libqb, __FILE__, __func__, __LINE__, "Test info message");
    libqb_log(loglevel::Warning, logscope::Libqb, __FILE__, __func__, __LINE__, "Test warning message");
    libqb_log(loglevel::Error, logscope::Libqb, __FILE__, __func__, __LINE__, "Test error message");
    
    test_assert(1); // If we get here, logging didn't crash
}

static void test_libqb_log_with_format() {
    // Test logging with format strings
    // Note: libqb_log_init() already called in previous test
    
    libqb_log(loglevel::Information, logscope::Libqb, __FILE__, __func__, __LINE__, 
              "Test with number: %d", 42);
    libqb_log(loglevel::Information, logscope::Libqb, __FILE__, __func__, __LINE__, 
              "Test with string: %s", "test_string");
    
    test_assert(1); // If we get here, logging didn't crash
}

static void test_libqb_log_qbs() {
    // Test logging with qbs string
    // Note: libqb_log_init() already called in previous test
    
    qbs *test_str = qbs_new_txt("Test qbs log message");
    libqb_log_qbs(loglevel::Information, logscope::Libqb, __FILE__, __func__, __LINE__, test_str);
    qbs_free(test_str);
    
    test_assert(1); // If we get here, logging didn't crash
}

static void test_libqb_log_different_scopes() {
    // Test logging to different scopes
    // Note: libqb_log_init() already called in previous test
    
    libqb_log(loglevel::Information, logscope::Runtime, __FILE__, __func__, __LINE__, "Runtime scope");
    libqb_log(loglevel::Information, logscope::QB64, __FILE__, __func__, __LINE__, "QB64 scope");
    libqb_log(loglevel::Information, logscope::Libqb, __FILE__, __func__, __LINE__, "Libqb scope");
    libqb_log(loglevel::Information, logscope::Audio, __FILE__, __func__, __LINE__, "Audio scope");
    libqb_log(loglevel::Information, logscope::Image, __FILE__, __func__, __LINE__, "Image scope");
    
    test_assert(1); // If we get here, logging didn't crash
}

static void test_libqb_log_different_levels() {
    // Test logging at different levels
    // Note: libqb_log_init() already called in previous test
    
    libqb_log(loglevel::Trace, logscope::Libqb, __FILE__, __func__, __LINE__, "Trace level");
    libqb_log(loglevel::Information, logscope::Libqb, __FILE__, __func__, __LINE__, "Information level");
    libqb_log(loglevel::Warning, logscope::Libqb, __FILE__, __func__, __LINE__, "Warning level");
    libqb_log(loglevel::Error, logscope::Libqb, __FILE__, __func__, __LINE__, "Error level");
    
    test_assert(1); // If we get here, logging didn't crash
}

static void test_libqb_log_qb64() {
    // Test QB64-specific logging
    // Note: libqb_log_init() already called in previous test
    
    libqb_log_qb64(loglevel::Information, logscope::QB64, __FILE__, __func__, __LINE__, 
                   "QB64 log message: %d", 123);
    
    test_assert(1); // If we get here, logging didn't crash
}

static void test_logging_multiple_calls() {
    // Test multiple logging calls
    // Note: libqb_log_init() already called in previous test
    
    for (int i = 0; i < 10; i++) {
        libqb_log(loglevel::Information, logscope::Libqb, __FILE__, __func__, __LINE__, 
                  "Message %d", i);
    }
    
    test_assert(1); // If we get here, multiple calls worked
}

int main() {
    struct unit_test tests[] = {
        {test_logging_init, "logging_init"},
        {test_func__logminlevel, "func__logminlevel"},
        {test_libqb_log_basic, "libqb_log_basic"},
        {test_libqb_log_with_format, "libqb_log_with_format"},
        {test_libqb_log_qbs, "libqb_log_qbs"},
        {test_libqb_log_different_scopes, "libqb_log_different_scopes"},
        {test_libqb_log_different_levels, "libqb_log_different_levels"},
        {test_libqb_log_qb64, "libqb_log_qb64"},
        {test_logging_multiple_calls, "logging_multiple_calls"},
    };
    
    return run_tests("logging", tests, sizeof(tests) / sizeof(tests[0]));
}
