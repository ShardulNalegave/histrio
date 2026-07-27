
#ifndef HISTRIO_COROUTINES_H
#define HISTRIO_COROUTINES_H

#include "stddef.h"

typedef enum coroutine_state {
    COROUTINE_READY,
    COROUTINE_RUNNING,
    COROUTINE_SUSPENDED,
    COROUTINE_DEAD
} coroutine_state_t;

typedef struct coroutine_cpu_context {
    void *rip; // offset  0 : instruction pointer (resume address)
    void *rsp; // offset  8 : stack pointer
    void *rbp; // offset 16 : base/frame pointer
    void *rbx; // offset 24 : callee-saved general purpose
    void *r12; // offset 32 : callee-saved general purpose
    void *r13; // offset 40 : callee-saved general purpose
    void *r14; // offset 48 : callee-saved general purpose
    void *r15; // offset 56 : callee-saved general purpose
} coroutine_cpu_context_t;

typedef struct coroutine {
    coroutine_state_t state;
    coroutine_cpu_context_t cpu_context;

    void *stack;
    size_t stack_size;
    void (*cleanup)(void *stack, size_t stack_size);

    void *arg;
    void (*entry)(void *arg);
} coroutine_t;

coroutine_t* coroutine_create(
    void *stack,
    size_t stack_size,
    void (*entry)(void *arg),
    void *arg,
    void (*cleanup)(void *stack, size_t stack_size)
);

void coroutine_yield(void);
bool coroutine_resume(coroutine_t *co);

#endif // HISTRIO_COROUTINES_H
