
#include "histrio/coroutine.h"

#include "stdint.h"
#include "stdlib.h"
#include "assert.h"

static _Thread_local coroutine_t *current_coroutine = NULL;
extern _Thread_local coroutine_cpu_context_t scheduler_cpu_context;

extern void coroutine_trampoline(void);
extern void coroutine_context_switch(coroutine_cpu_context_t *from, coroutine_cpu_context_t *to);

coroutine_t* coroutine_create(
    void *stack,
    size_t stack_size,
    void (*entry)(void *arg),
    void *arg,
    void (*cleanup)(void *stack, size_t stack_size)
) {
    coroutine_t *co = calloc(1, sizeof(coroutine_t));
    if (!co) return NULL;

    co->stack = stack;
    co->stack_size = stack_size;
    co->entry = entry;
    co->arg = arg;
    co->cleanup = cleanup;
    co->state = COROUTINE_READY;

    // Note: We assume the provided stack is 16-byte aligned

    // Reserve the red zone (128 bytes)
    // We decrement the stack size itself by 128 and compute a correct stack top pointer
    // as this region is unusable except by leaf functions who use it as a scratch pad.
    co->stack_size -= 128;
    void *top = (uint8_t *)co->stack + co->stack_size;

    co->cpu_context.rip  = (void *)coroutine_trampoline;
    co->cpu_context.rsp  = top; 
    co->cpu_context.rbp  = NULL;
    co->cpu_context.rbx  = NULL;
    co->cpu_context.r12 = (void *)co->entry;
    co->cpu_context.r13 = (void *)co->arg;
    co->cpu_context.r14 = NULL;
    co->cpu_context.r15 = NULL;

    return co;
}

_Noreturn void coroutine_finished(void) {
    coroutine_t *self = current_coroutine;
    self->state = COROUTINE_DEAD;
    coroutine_context_switch(&self->cpu_context, &scheduler_cpu_context);
    abort();
}

void coroutine_yield(void) {
    coroutine_t *self = current_coroutine;
    self->state = COROUTINE_SUSPENDED;
    coroutine_context_switch(&self->cpu_context, &scheduler_cpu_context);
}

void reap(coroutine_t *co) {
    if (co->cleanup) {
        co->cleanup(co->stack, co->stack_size);
    }

    free(co);
}

bool coroutine_resume(coroutine_t *co) {
    assert(co->state == COROUTINE_READY || co->state == COROUTINE_SUSPENDED);

    current_coroutine = co;
    co->state = COROUTINE_RUNNING;

    coroutine_context_switch(&scheduler_cpu_context, &co->cpu_context);

    current_coroutine = NULL;

    if (co->state == COROUTINE_DEAD) {
        reap(co);
        return true;
    }

    return false;
}
