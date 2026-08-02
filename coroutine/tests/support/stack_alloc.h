#ifndef HISTRIO_COROUTINE_TEST_STACK_ALLOC_H
#define HISTRIO_COROUTINE_TEST_STACK_ALLOC_H

#include <stddef.h>

void *coroutine_test_stack_alloc(size_t size);
void coroutine_test_stack_cleanup(void *stack, size_t stack_size);

#endif // HISTRIO_COROUTINE_TEST_STACK_ALLOC_H
