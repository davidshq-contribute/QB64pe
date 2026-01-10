// Unit tests for qblist module
// Tests list data structure operations

#include "test.h"
#include "qblist.h"

static void test_list_new() {
    // Test creating a new list
    list *L = list_new(16); // 16-byte structures
    test_assert(L != NULL);
    test_assert_ints(16, L->user_structure_size);
    
    list_destroy(L);
}

static void test_list_new_threadsafe() {
    // Test creating a new threadsafe list
    list *L = list_new_threadsafe(32); // 32-byte structures
    test_assert(L != NULL);
    test_assert_ints(32, L->user_structure_size);
    test_assert(L->lock_add != NULL);
    test_assert(L->lock_remove != NULL);
    
    list_destroy(L);
}

static void test_list_add() {
    // Test adding items to list
    list *L = list_new(8);
    test_assert(L != NULL);
    
    intptr_t idx1 = list_add(L);
    test_assert(idx1 > 0); // List doesn't use index 0
    
    intptr_t idx2 = list_add(L);
    test_assert(idx2 > 0);
    test_assert(idx2 != idx1);
    
    intptr_t idx3 = list_add(L);
    test_assert(idx3 > 0);
    test_assert(idx3 != idx1);
    test_assert(idx3 != idx2);
    
    list_destroy(L);
}

static void test_list_get() {
    // Test getting items from list
    list *L = list_new(8);
    test_assert(L != NULL);
    
    intptr_t idx = list_add(L);
    test_assert(idx > 0);
    
    void *item = list_get(L, idx);
    test_assert(item != NULL);
    
    list_destroy(L);
}

static void test_list_get_index() {
    // Test getting index from structure pointer
    list *L = list_new(8);
    test_assert(L != NULL);
    
    intptr_t idx = list_add(L);
    test_assert(idx > 0);
    
    void *item = list_get(L, idx);
    test_assert(item != NULL);
    
    intptr_t retrieved_idx = list_get_index(L, item);
    test_assert_ints(idx, retrieved_idx);
    
    list_destroy(L);
}

static void test_list_remove() {
    // Test removing items from list
    list *L = list_new(8);
    test_assert(L != NULL);
    
    intptr_t idx1 = list_add(L);
    intptr_t idx2 = list_add(L);
    intptr_t idx3 = list_add(L);
    
    // Remove middle item
    intptr_t result = list_remove(L, idx2);
    test_assert_ints(-1, result); // -1 indicates success
    
    // Verify item is removed
    void *item = list_get(L, idx2);
    test_assert(item == NULL);
    
    // Verify other items still exist
    item = list_get(L, idx1);
    test_assert(item != NULL);
    
    item = list_get(L, idx3);
    test_assert(item != NULL);
    
    list_destroy(L);
}

static void test_list_reuse_removed() {
    // Test that removed indices can be reused
    list *L = list_new(8);
    test_assert(L != NULL);
    
    intptr_t idx1 = list_add(L);
    list_remove(L, idx1);
    
    // Add new item - might reuse the removed index
    intptr_t idx2 = list_add(L);
    test_assert(idx2 > 0);
    
    void *item = list_get(L, idx2);
    test_assert(item != NULL);
    
    list_destroy(L);
}

int main() {
    struct unit_test tests[] = {
        {test_list_new, "list_new"},
        {test_list_new_threadsafe, "list_new_threadsafe"},
        {test_list_add, "list_add"},
        {test_list_get, "list_get"},
        {test_list_get_index, "list_get_index"},
        {test_list_remove, "list_remove"},
        {test_list_reuse_removed, "list_reuse_removed"},
    };
    
    return run_tests("qblist", tests, sizeof(tests) / sizeof(tests[0]));
}
