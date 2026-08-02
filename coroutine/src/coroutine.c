
#include "histrio/coroutine.h"

#include "stdint.h"
#include "stdlib.h"

#include "histrio/telemetry/log.h"

static _Thread_local coroutine_t *current_coroutine = NULL;
static _Thread_local coroutine_cpu_context_t scheduler_cpu_context;

extern void coroutine_trampoline(void);
extern void coroutine_context_switch(coroutine_cpu_context_t *from, coroutine_cpu_context_t *to);

histrio_result_t coroutine_create(coroutine_t* co) {
    if (!co)
        return HISTRIO_FAIL(HISTRIO_ERR_INVALID_ARGUMENT, "co must not be NULL");
    if (!co->stack)
        return HISTRIO_FAIL(HISTRIO_ERR_INVALID_ARGUMENT, "co->stack must not be NULL");
    if (!co->entry)
        return HISTRIO_FAIL(HISTRIO_ERR_INVALID_ARGUMENT, "co->entry must not be NULL");
    if (!co->cleanup)
        return HISTRIO_FAIL(HISTRIO_ERR_INVALID_ARGUMENT, "co->cleanup must not be NULL");
    if (co->stack_size <= 128)
        return HISTRIO_FAIL(HISTRIO_ERR_INVALID_ARGUMENT, "co->stack_size should be greater than 128 (red zone reservation)");

    co->state = COROUTINE_READY; // Ensure correct state

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

    HISTRIO_TRACE("coroutine_create: created co=%p stack=[%p,%p)", (void *)co, co->stack, top);

    return HISTRIO_SUCCESS();
}

_Noreturn void coroutine_finished(void) {
    coroutine_t *self = current_coroutine;
    self->state = COROUTINE_DEAD;
    HISTRIO_TRACE("coroutine_finished: co=%p", (void *)self);
    coroutine_context_switch(&self->cpu_context, &scheduler_cpu_context);
    abort();
}

void coroutine_yield(void) {
    coroutine_t *self = current_coroutine;
    if (!self) {
        HISTRIO_ERROR("coroutine_yield: called outside of any running coroutine ignoring");
        return;
    }
    self->state = COROUTINE_SUSPENDED;
    coroutine_context_switch(&self->cpu_context, &scheduler_cpu_context);
}

histrio_status_t coroutine_resume(coroutine_t *co) {
    if (!co) {
        HISTRIO_WARN("coroutine_resume: co must not be NULL");
        return HISTRIO_ERR_INVALID_ARGUMENT;
    }
    if (current_coroutine != NULL) {
        HISTRIO_WARN("coroutine_resume: co=%p rejected. a running coroutine (co=%p) attempted to call coroutine_resume.", (void *)co, (void *)current_coroutine);
        return HISTRIO_ERR_FAILED_PRECONDITION;
    }
    if (co->state == COROUTINE_RUNNING || co->state == COROUTINE_DEAD) {
        HISTRIO_WARN("coroutine_resume: co=%p cannot be resumed from state=%d", (void *)co, co->state);
        return HISTRIO_ERR_FAILED_PRECONDITION;
    }

    current_coroutine = co;
    co->state = COROUTINE_RUNNING;

    coroutine_context_switch(&scheduler_cpu_context, &co->cpu_context);

    current_coroutine = NULL;
    return HISTRIO_OK;
}

void coroutine_stack_free(coroutine_t *co) {
    if (!co) return;
    if (co->cleanup) {
        co->cleanup(co->stack, co->stack_size);
    }

    HISTRIO_TRACE("coroutine_stack_free: co=%p", (void *)co);
}
