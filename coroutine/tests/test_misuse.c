#include <criterion/criterion.h>
#include <signal.h>
#include <sys/mman.h>

#include "histrio/coroutine.h"

#include "support/fixtures.h"
#include "support/stack_alloc.h"

TestSuite(coroutine_misuse, .init = coroutine_test_suite_init, .fini = coroutine_test_suite_fini);

static void noop_entry(void *arg) { (void)arg; }

Test(coroutine_misuse, yield_outside_coroutine_is_safe_noop, .timeout = 2.0) {
    coroutine_yield();

    coroutine_t co = {0};
    co.stack = coroutine_test_stack_alloc(COROUTINE_TEST_STACK_SIZE);
    co.stack_size = COROUTINE_TEST_STACK_SIZE;
    co.entry = noop_entry;
    co.cleanup = coroutine_test_stack_cleanup;

    cr_assert_eq(coroutine_create(&co).code, HISTRIO_OK);
    cr_assert_eq(coroutine_resume(&co), HISTRIO_OK);
    cr_assert_eq(co.state, COROUTINE_DEAD);

    coroutine_stack_free(&co);
}

// A coroutine_t that was never passed through coroutine_create(): state
// reads as COROUTINE_READY (enum value 0, matching zero-init) so it
// passes the precondition check in coroutine_resume(), then the context
// switch jumps to a NULL rip with a NULL rsp.
Test(coroutine_misuse, resume_never_created_coroutine_crashes, .signal = SIGSEGV, .timeout = 2.0) {
    coroutine_t co = {0};
    coroutine_resume(&co);
}

// Resuming a still-SUSPENDED coroutine whose stack has been released out
// from under it. This does NOT use coroutine_test_stack_alloc() /
// coroutine_stack_free(): a plain heap free() here is UB that plain ASan
// does not reliably catch either, since a hand-rolled context switch that
// resumes execution on freed memory hits ordinary, uninstrumented
// local-variable stack accesses (raw ret/jmp, not a pointer dereference
// ASan's compiler pass would ever see) - confirmed empirically before
// writing this test: an ASan build ran straight through a resume into a
// freed stack with no report at all. So instead of relying on malloc's
// heap poisoning, the stack is mmap'd and its access is revoked with
// mprotect(PROT_NONE) before the second resume - enforced by the MMU, so
// it deterministically SIGSEGVs on the very next access regardless of
// sanitizer configuration.
static void yield_once_entry(void *arg) {
    (void)arg;
    coroutine_yield();
}

static void mmap_stack_cleanup(void *stack, size_t stack_size) {
    munmap(stack, stack_size);
}

Test(coroutine_misuse, resume_after_stack_access_revoked_crashes, .signal = SIGSEGV, .timeout = 2.0) {
    size_t size = COROUTINE_TEST_STACK_SIZE;
    void *stack = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    cr_assert_neq(stack, MAP_FAILED);

    coroutine_t co = {0};
    co.stack = stack;
    co.stack_size = size;
    co.entry = yield_once_entry;
    co.cleanup = mmap_stack_cleanup;

    cr_assert_eq(coroutine_create(&co).code, HISTRIO_OK);
    coroutine_resume(&co);
    cr_assert_eq(co.state, COROUTINE_SUSPENDED);

    cr_assert_eq(mprotect(stack, size, PROT_NONE), 0);

    coroutine_resume(&co); // use-after-free of the stack: must crash
}
