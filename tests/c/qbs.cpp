// Unit tests for QB64 String (qbs) system
// Tests string operations, memory management, and reference counting

#include "test.h"
#include "../c/include/qbs.h"

static void test_qbs_new()
{
    qbs *str = qbs_new_txt("Hello, World!");
    test_assert(str != NULL);
    test_assert_ints(13, str->len);
    test_assert(str->chr != NULL);
    qbs_free(str);
}

static void test_qbs_add()
{
    qbs *str1 = qbs_new_txt("Hello, ");
    qbs *str2 = qbs_new_txt("World!");
    qbs *result = qbs_add(str1, str2);
    
    test_assert(result != NULL);
    test_assert_ints(13, result->len);
    
    qbs_free(str1);
    qbs_free(str2);
    qbs_free(result);
}

static void test_qbs_set()
{
    qbs *str1 = qbs_new_txt("Original");
    qbs *str2 = qbs_new_txt("New");
    qbs *result = qbs_set(str1, str2);
    
    test_assert(result != NULL);
    test_assert_ints(3, result->len);
    
    qbs_free(str1);
    qbs_free(str2);
    qbs_free(result);
}

static void test_qbs_str()
{
    qbs *str = qbs_str(42);
    test_assert(str != NULL);
    test_assert(str->len > 0);
    qbs_free(str);
    
    str = qbs_str(-42);
    test_assert(str != NULL);
    qbs_free(str);
}

struct unit_test qbs_tests[] = {
    {test_qbs_new, "qbs_new"},
    {test_qbs_add, "qbs_add"},
    {test_qbs_set, "qbs_set"},
    {test_qbs_str, "qbs_str"},
};

int main()
{
    return run_tests("qbs", qbs_tests, sizeof(qbs_tests) / sizeof(qbs_tests[0]));
}
