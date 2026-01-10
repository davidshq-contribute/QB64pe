// Unit tests for command module
// Tests command-line argument parsing and retrieval

#include "test.h"
#include "command.h"
#include "qbs.h"
#include <stdlib.h>
#include <string.h>

static void test_command_initialize() {
    // Initialize with test arguments
    char *argv[] = {"test_program", "arg1", "arg2", "arg3", NULL};
    int argc = 4;
    
    command_initialize(argc, argv);
    
    // Verify command count
    int32_t count = func__commandcount();
    test_assert_ints(3, count); // 3 arguments (excluding program name)
}

static void test_func_command() {
    char *argv[] = {"test_program", "first", "second", "third", NULL};
    int argc = 4;
    
    command_initialize(argc, argv);
    
    // Test retrieving first command (index 1)
    qbs *cmd1 = func_command(1, 1);
    test_assert(cmd1 != NULL);
    test_assert_ints(5, cmd1->len); // "first" is 5 characters
    test_assert_buffers("first", (const char *)cmd1->chr, 5);
    qbs_free(cmd1);
    
    // Test retrieving second command (index 2)
    qbs *cmd2 = func_command(2, 1);
    test_assert(cmd2 != NULL);
    test_assert_ints(6, cmd2->len); // "second" is 6 characters
    test_assert_buffers("second", (const char *)cmd2->chr, 6);
    qbs_free(cmd2);
    
    // Test retrieving third command (index 3)
    qbs *cmd3 = func_command(3, 1);
    test_assert(cmd3 != NULL);
    test_assert_ints(5, cmd3->len); // "third" is 5 characters
    test_assert_buffers("third", (const char *)cmd3->chr, 5);
    qbs_free(cmd3);
}

static void test_func_command_out_of_range() {
    char *argv[] = {"test_program", "arg1", NULL};
    int argc = 2;
    
    command_initialize(argc, argv);
    
    // Test retrieving command beyond range
    qbs *cmd = func_command(10, 1);
    // Should return empty string or handle gracefully
    test_assert(cmd != NULL);
    qbs_free(cmd);
}

static void test_func_commandcount() {
    char *argv[] = {"test_program", NULL};
    int argc = 1;
    
    command_initialize(argc, argv);
    int32_t count = func__commandcount();
    test_assert_ints(0, count); // No arguments
    
    char *argv2[] = {"test_program", "one", "two", "three", NULL};
    int argc2 = 4;
    
    command_initialize(argc2, argv2);
    count = func__commandcount();
    test_assert_ints(3, count); // Three arguments
}

int main() {
    struct unit_test tests[] = {
        {test_command_initialize, "command_initialize"},
        {test_func_command, "func_command"},
        {test_func_command_out_of_range, "func_command_out_of_range"},
        {test_func_commandcount, "func_commandcount"},
    };
    
    return run_tests("command", tests, sizeof(tests) / sizeof(tests[0]));
}
