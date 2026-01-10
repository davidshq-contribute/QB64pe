// Unit tests for shell module
// Tests shell command execution functions

#include "test.h"
#include "shell.h"
#include <unistd.h>

static void test_shell_call_in_progress() {
    // Test that shell_call_in_progress is accessible
    // Initially should be 0 (no shell call in progress)
    test_assert_ints(0, shell_call_in_progress);
}

static void test_func_shell() {
    // Test executing a shell command
    // Use a simple command that should work on all platforms
    qbs *cmd = qbs_new_txt("echo test");
    int64_t result = func_shell(cmd);
    
    // Result should be non-negative (exit code)
    test_assert(result >= 0);
    
    qbs_free(cmd);
}

static void test_func__shellhide() {
    // Test executing a shell command with hidden output
    qbs *cmd = qbs_new_txt("echo test");
    int64_t result = func__shellhide(cmd);
    
    // Result should be non-negative (exit code)
    test_assert(result >= 0);
    
    qbs_free(cmd);
}

static void test_sub_shell() {
    // Test shell subroutine (no return value)
    qbs *cmd = qbs_new_txt("echo test");
    sub_shell(cmd, 0);
    
    // If we get here, it didn't crash
    test_assert(1);
    
    qbs_free(cmd);
}

static void test_sub_shell2() {
    // Test shell subroutine variant 2
    qbs *cmd = qbs_new_txt("echo test");
    sub_shell2(cmd, 0);
    
    // If we get here, it didn't crash
    test_assert(1);
    
    qbs_free(cmd);
}

static void test_sub_shell3() {
    // Test shell subroutine variant 3
    qbs *cmd = qbs_new_txt("echo test");
    sub_shell3(cmd, 0);
    
    // If we get here, it didn't crash
    test_assert(1);
    
    qbs_free(cmd);
}

static void test_sub_shell4() {
    // Test shell subroutine variant 4
    qbs *cmd = qbs_new_txt("echo test");
    sub_shell4(cmd, 0);
    
    // If we get here, it didn't crash
    test_assert(1);
    
    qbs_free(cmd);
}

int main() {
    struct unit_test tests[] = {
        {test_shell_call_in_progress, "shell_call_in_progress"},
        {test_func_shell, "func_shell"},
        {test_func__shellhide, "func__shellhide"},
        {test_sub_shell, "sub_shell"},
        {test_sub_shell2, "sub_shell2"},
        {test_sub_shell3, "sub_shell3"},
        {test_sub_shell4, "sub_shell4"},
    };
    
    return run_tests("shell", tests, sizeof(tests) / sizeof(tests[0]));
}
