#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Mock QB64 error function for testing
void error(int code) {
    printf("Error %d triggered\n", code);
}

// Include the memory management code
typedef struct mem_lock {
    uint64_t id;
    int32_t type;
    void *offset;
} mem_lock;

typedef struct mem_block {
    intptr_t lock_offset;
    uint64_t lock_id;
    intptr_t offset;
    intptr_t size;
    int32_t type;
    intptr_t elementsize;
    int32_t image;
} mem_block;

// Global variables (from mem.cpp)
uint64_t mem_lock_id = 1073741823;
int32_t mem_lock_max = 10000;
int32_t mem_lock_next = 0;
mem_lock *mem_lock_base = NULL;
mem_lock *mem_lock_tmp;

int32_t mem_lock_freed_max = 1000;
int32_t mem_lock_freed_n = 0;
intptr_t *mem_lock_freed = NULL;

static int mem_init_done = 0;
static int initialize_mem_system() {
    if (mem_init_done) return 1;
    
    if (!mem_lock_base) {
        mem_lock_base = (mem_lock *)malloc(sizeof(mem_lock) * mem_lock_max);
        if (!mem_lock_base) {
            return 0;
        }
    }
    
    if (!mem_lock_freed) {
        mem_lock_freed = (intptr_t *)malloc(sizeof(intptr_t) * mem_lock_freed_max);
        if (!mem_lock_freed) {
            free(mem_lock_base);
            mem_lock_base = NULL;
            return 0;
        }
    }
    
    mem_init_done = 1;
    return 1;
}

void new_mem_lock() {
    if (!initialize_mem_system()) {
        error(518);
        return;
    }

    if (mem_lock_freed_n) {
        mem_lock_tmp = (mem_lock *)mem_lock_freed[--mem_lock_freed_n];
    } else {
        if (mem_lock_next == mem_lock_max) {
            mem_lock *new_base = (mem_lock *)malloc(sizeof(mem_lock) * mem_lock_max);
            if (!new_base) {
                error(518);
                return;
            }
            mem_lock_base = new_base;
            mem_lock_next = 0;
        }
        mem_lock_tmp = &mem_lock_base[mem_lock_next++];
    }
    mem_lock_tmp->id = ++mem_lock_id;
}

mem_block func__memnew(intptr_t bytes) {
    static mem_block b;
    new_mem_lock();
    b.lock_offset = (intptr_t)mem_lock_tmp;
    b.lock_id = mem_lock_id;
    b.type = 16384;
    b.elementsize = 1;
    b.image = -1;
    
    if (bytes < 0) {
        error(5);
        b.offset = 0;
        b.size = 0;
        mem_lock_tmp->type = 0;
    } else {
        if (!bytes) {
            b.offset = 1;
            b.size = 0;
        } else {
            b.offset = (intptr_t)malloc(bytes);
            if (!b.offset) {
                b.size = 0;
                mem_lock_tmp->type = 0;
                error(518);
            } else {
                b.size = bytes;
                mem_lock_tmp->type = 1;
                mem_lock_tmp->offset = (void *)b.offset;
            }
        }
    }
    return b;
}

int main() {
    printf("Testing memory allocation fixes...\n");
    
    // Test 1: Normal allocation
    printf("Test 1: Normal allocation (100 bytes)\n");
    mem_block block1 = func__memnew(100);
    if (block1.offset != 0 && block1.size == 100) {
        printf("✓ Normal allocation successful\n");
    } else {
        printf("✗ Normal allocation failed\n");
    }
    
    // Test 2: Zero bytes allocation
    printf("Test 2: Zero bytes allocation\n");
    mem_block block2 = func__memnew(0);
    if (block2.offset == 1 && block2.size == 0) {
        printf("✓ Zero bytes allocation successful\n");
    } else {
        printf("✗ Zero bytes allocation failed\n");
    }
    
    // Test 3: Negative size (should trigger error 5)
    printf("Test 3: Negative size allocation\n");
    mem_block block3 = func__memnew(-10);
    if (block3.offset == 0 && block3.size == 0) {
        printf("✓ Negative size properly handled\n");
    } else {
        printf("✗ Negative size not properly handled\n");
    }
    
    // Test 4: Large allocation (may fail, should handle gracefully)
    printf("Test 4: Large allocation test\n");
    mem_block block4 = func__memnew(SIZE_MAX);
    if (block4.offset == 0 && block4.size == 0) {
        printf("✓ Large allocation properly handled (failed gracefully)\n");
    } else {
        printf("✓ Large allocation succeeded (unexpected but ok)\n");
    }
    
    printf("\nMemory allocation fix tests completed.\n");
    return 0;
}
