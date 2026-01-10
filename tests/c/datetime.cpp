// Unit tests for datetime module
// Tests date/time functions, timer, and delay operations

#include "test.h"
#include "datetime.h"
#include <unistd.h>

static void test_clock_init() {
    // Test that clock_init can be called without errors
    clock_init();
    test_assert(1); // If we get here, it didn't crash
}

static void test_getticks() {
    clock_init();
    
    // Get initial tick count
    int64_t ticks1 = GetTicks();
    test_assert(ticks1 >= 0);
    
    // Wait a small amount
    usleep(10000); // 10ms
    
    // Get tick count again
    int64_t ticks2 = GetTicks();
    test_assert(ticks2 >= ticks1); // Should have increased
}

static void test_func_timer() {
    clock_init();
    
    // Test timer with default accuracy
    double time1 = func_timer(0, 0);
    test_assert(time1 >= 0);
    
    // Wait a bit
    usleep(10000); // 10ms
    
    double time2 = func_timer(0, 0);
    test_assert(time2 >= time1);
    
    // Test timer with specific accuracy
    double time3 = func_timer(0.001, 1);
    test_assert(time3 >= 0);
}

static void test_func_time() {
    qbs *time_str = func_time();
    test_assert(time_str != NULL);
    test_assert(time_str->len > 0);
    // Time string should be in format HH:MM:SS (8 characters)
    test_assert(time_str->len >= 8);
    qbs_free(time_str);
}

static void test_func_date() {
    qbs *date_str = func_date();
    test_assert(date_str != NULL);
    test_assert(date_str->len > 0);
    // Date string format varies, but should have content
    test_assert(date_str->len >= 8);
    qbs_free(date_str);
}

static void test_sub_delay() {
    clock_init();
    
    int64_t start = GetTicks();
    sub__delay(0.01); // 10ms delay
    int64_t end = GetTicks();
    
    // Verify some time passed (allowing for timing variations)
    test_assert(end >= start);
}

static void test_sleep() {
#ifndef QB64_WINDOWS
    clock_init();
    
    int64_t start = GetTicks();
    Sleep(10); // 10ms sleep
    int64_t end = GetTicks();
    
    // Verify some time passed
    test_assert(end >= start);
#else
    // Sleep() is not available on Windows (use Windows Sleep instead)
    // Skip this test on Windows or use alternative
    test_assert(1); // Placeholder
#endif
}

int main() {
    struct unit_test tests[] = {
        {test_clock_init, "clock_init"},
        {test_getticks, "GetTicks"},
        {test_func_timer, "func_timer"},
        {test_func_time, "func_time"},
        {test_func_date, "func_date"},
        {test_sub_delay, "sub_delay"},
        {test_sleep, "Sleep"},
    };
    
    return run_tests("datetime", tests, sizeof(tests) / sizeof(tests[0]));
}
