#include "stack_alloc.h"

#include <stdlib.h>

void *coroutine_test_stack_alloc(size_t size) {
    void *stack = NULL;
    if (posix_memalign(&stack, 16, size) != 0) return NULL;
    return stack;
}

void coroutine_test_stack_cleanup(void *stack, size_t stack_size) {
    (void)stack_size;
    free(stack);
}
