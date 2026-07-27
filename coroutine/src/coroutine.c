
#include "histrio/coroutine.h"

#include "stdint.h"
#include "stdlib.h"

extern void coroutine_trampoline(void);
extern void coroutine_context_switch(coroutine_cpu_context_t *from, coroutine_cpu_context_t *to);

void coroutine_make(coroutine_t *co) {
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
}

_Noreturn void coroutine_finished(void) {
    abort();
}
