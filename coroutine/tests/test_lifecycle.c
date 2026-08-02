#include <criterion/criterion.h>
#include <stdlib.h>

#include "histrio/coroutine.h"

#include "support/fixtures.h"
#include "support/stack_alloc.h"

TestSuite(coroutine_lifecycle, .init = coroutine_test_suite_init, .fini = coroutine_test_suite_fini);

static void noop_entry(void *arg) { (void)arg; }

static int step_marker = 0;
static void multi_yield_entry(void *arg) {
    (void)arg;
    step_marker = 1;
    coroutine_yield();
    step_marker = 2;
    coroutine_yield();
    step_marker = 3;
}

Test(coroutine_lifecycle, state_machine_full_cycle, .timeout = 2.0) {
    coroutine_t co = {0};
    co.stack = coroutine_test_stack_alloc(COROUTINE_TEST_STACK_SIZE);
    co.stack_size = COROUTINE_TEST_STACK_SIZE;
    co.entry = multi_yield_entry;
    co.cleanup = coroutine_test_stack_cleanup;

    cr_assert_eq(coroutine_create(&co).code, HISTRIO_OK);
    cr_assert_eq(co.state, COROUTINE_READY);

    cr_assert_eq(coroutine_resume(&co), HISTRIO_OK);
    cr_assert_eq(step_marker, 1);
    cr_assert_eq(co.state, COROUTINE_SUSPENDED);

    cr_assert_eq(coroutine_resume(&co), HISTRIO_OK);
    cr_assert_eq(step_marker, 2);
    cr_assert_eq(co.state, COROUTINE_SUSPENDED);

    cr_assert_eq(coroutine_resume(&co), HISTRIO_OK);
    cr_assert_eq(step_marker, 3);
    cr_assert_eq(co.state, COROUTINE_DEAD);

    coroutine_stack_free(&co);
}

Test(coroutine_lifecycle, resume_after_dead_fails, .timeout = 2.0) {
    coroutine_t co = {0};
    co.stack = coroutine_test_stack_alloc(COROUTINE_TEST_STACK_SIZE);
    co.stack_size = COROUTINE_TEST_STACK_SIZE;
    co.entry = noop_entry;
    co.cleanup = coroutine_test_stack_cleanup;
    cr_assert_eq(coroutine_create(&co).code, HISTRIO_OK);

    coroutine_resume(&co); // runs to completion
    cr_assert_eq(co.state, COROUTINE_DEAD);

    cr_assert_eq(coroutine_resume(&co), HISTRIO_ERR_FAILED_PRECONDITION);

    coroutine_stack_free(&co);
}

static coroutine_t *self_ptr;
static histrio_status_t self_resume_status;
static void self_resume_entry(void *arg) {
    (void)arg;
    self_resume_status = coroutine_resume(self_ptr); // co is RUNNING; must be rejected, not crash
    coroutine_yield();
}

Test(coroutine_lifecycle, resume_self_while_running_fails, .timeout = 2.0) {
    coroutine_t co = {0};
    co.stack = coroutine_test_stack_alloc(COROUTINE_TEST_STACK_SIZE);
    co.stack_size = COROUTINE_TEST_STACK_SIZE;
    co.entry = self_resume_entry;
    co.cleanup = coroutine_test_stack_cleanup;
    cr_assert_eq(coroutine_create(&co).code, HISTRIO_OK);
    self_ptr = &co;

    coroutine_resume(&co);
    cr_assert_eq(self_resume_status, HISTRIO_ERR_FAILED_PRECONDITION);
    cr_assert_eq(co.state, COROUTINE_SUSPENDED);

    coroutine_resume(&co); // let it finish
    coroutine_stack_free(&co);
}

static coroutine_t *nested_target_ptr;
static histrio_status_t nested_resume_status;
static void resume_other_entry(void *arg) {
    (void)arg;
    // Scheduling decisions belong exclusively to the scheduler: A is
    // RUNNING, so this must be rejected exactly like resuming itself
    // would be, not just when co == the running coroutine.
    nested_resume_status = coroutine_resume(nested_target_ptr);
    coroutine_yield();
}

Test(coroutine_lifecycle, resume_different_coroutine_while_running_fails, .timeout = 2.0) {
    coroutine_t a = {0}, b = {0};
    a.stack = coroutine_test_stack_alloc(COROUTINE_TEST_STACK_SIZE);
    a.stack_size = COROUTINE_TEST_STACK_SIZE;
    a.entry = resume_other_entry;
    a.cleanup = coroutine_test_stack_cleanup;

    b.stack = coroutine_test_stack_alloc(COROUTINE_TEST_STACK_SIZE);
    b.stack_size = COROUTINE_TEST_STACK_SIZE;
    b.entry = noop_entry;
    b.cleanup = coroutine_test_stack_cleanup;

    cr_assert_eq(coroutine_create(&a).code, HISTRIO_OK);
    cr_assert_eq(coroutine_create(&b).code, HISTRIO_OK);
    nested_target_ptr = &b;

    coroutine_resume(&a);
    cr_assert_eq(nested_resume_status, HISTRIO_ERR_FAILED_PRECONDITION);
    cr_assert_eq(b.state, COROUTINE_READY); // b was never touched, let alone run
    cr_assert_eq(a.state, COROUTINE_SUSPENDED); // a carried on normally past the rejected call

    coroutine_resume(&a); // let a finish
    cr_assert_eq(a.state, COROUTINE_DEAD);

    coroutine_stack_free(&a);
    coroutine_stack_free(&b);
}

Test(coroutine_lifecycle, resume_null_returns_invalid_argument) {
    cr_assert_eq(coroutine_resume(NULL), HISTRIO_ERR_INVALID_ARGUMENT);
}

static int cleanup_calls = 0;
static void *cleanup_stack_arg = NULL;
static size_t cleanup_size_arg = 0;
static void tracking_cleanup(void *stack, size_t stack_size) {
    cleanup_calls++;
    cleanup_stack_arg = stack;
    cleanup_size_arg = stack_size;
    free(stack);
}

Test(coroutine_lifecycle, stack_free_invokes_cleanup_once_with_reduced_size, .timeout = 2.0) {
    void *stack = coroutine_test_stack_alloc(COROUTINE_TEST_STACK_SIZE);
    coroutine_t co = {0};
    co.stack = stack;
    co.stack_size = COROUTINE_TEST_STACK_SIZE;
    co.entry = noop_entry;
    co.cleanup = tracking_cleanup;

    cr_assert_eq(coroutine_create(&co).code, HISTRIO_OK);
    // coroutine_create() reserves 128 bytes for the red zone by reducing
    // co->stack_size in place; coroutine_stack_free() must pass that
    // reduced size straight through to cleanup(), not the original.
    size_t expected_size = COROUTINE_TEST_STACK_SIZE - 128;
    cr_assert_eq(co.stack_size, expected_size);

    coroutine_resume(&co);
    coroutine_stack_free(&co);

    cr_assert_eq(cleanup_calls, 1);
    cr_assert_eq(cleanup_stack_arg, stack);
    cr_assert_eq(cleanup_size_arg, expected_size);
}

Test(coroutine_lifecycle, stack_free_null_is_noop) {
    coroutine_stack_free(NULL);
    cr_assert(1);
}
