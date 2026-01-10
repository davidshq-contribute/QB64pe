// Unit tests for environ module
// Tests environment variable access and manipulation

#include "test.h"
#include "environ.h"
#include "qbs.h"
#include <stdlib.h>
#include <string.h>

static void test_func_environcount() {
    // Get count of environment variables
    int32_t count = func__environcount();
    test_assert(count >= 0);
    // Should have at least some environment variables
    test_assert(count > 0);
}

static void test_func_environ_by_name() {
    // Test retrieving environment variable by name
    // Use a common environment variable that should exist
    qbs *path_name = qbs_new_txt("PATH");
    qbs *path_value = func_environ(path_name);
    
    test_assert(path_value != NULL);
    // PATH should exist and have content
    test_assert(path_value->len > 0);
    
    qbs_free(path_name);
    qbs_free(path_value);
}

static void test_func_environ_by_number() {
    int32_t count = func__environcount();
    test_assert(count > 0);
    
    // Test retrieving first environment variable by number
    qbs *env1 = func_environ(1);
    test_assert(env1 != NULL);
    // Should have format "NAME=VALUE"
    test_assert(env1->len > 0);
    qbs_free(env1);
    
    // Test retrieving last environment variable
    if (count > 1) {
        qbs *env_last = func_environ(count);
        test_assert(env_last != NULL);
        test_assert(env_last->len > 0);
        qbs_free(env_last);
    }
}

static void test_func_environ_nonexistent() {
    // Test retrieving non-existent environment variable
    qbs *name = qbs_new_txt("NONEXISTENT_VAR_12345");
    qbs *value = func_environ(name);
    
    // Should return empty string or NULL
    test_assert(value != NULL);
    // Non-existent variable should be empty
    test_assert_ints(0, value->len);
    
    qbs_free(name);
    qbs_free(value);
}

static void test_sub_environ() {
    // Test setting an environment variable
    qbs *env_str = qbs_new_txt("TEST_VAR=test_value");
    sub_environ(env_str);
    
    // Verify it was set by retrieving it
    qbs *name = qbs_new_txt("TEST_VAR");
    qbs *value = func_environ(name);
    
    test_assert(value != NULL);
    test_assert(value->len > 0);
    test_assert_buffers("test_value", (const char *)value->chr, value->len);
    
    qbs_free(env_str);
    qbs_free(name);
    qbs_free(value);
    
    // Clean up - remove the test variable
    qbs *unset_str = qbs_new_txt("TEST_VAR=");
    sub_environ(unset_str);
    qbs_free(unset_str);
}

int main() {
    struct unit_test tests[] = {
        {test_func_environcount, "func_environcount"},
        {test_func_environ_by_name, "func_environ_by_name"},
        {test_func_environ_by_number, "func_environ_by_number"},
        {test_func_environ_nonexistent, "func_environ_nonexistent"},
        {test_sub_environ, "sub_environ"},
    };
    
    return run_tests("environ", tests, sizeof(tests) / sizeof(tests[0]));
}
