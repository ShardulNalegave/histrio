#include "criterion/criterion.h"

#include "histrio/coroutine.h"

#include "support/fixtures.h"
#include "support/stack_alloc.h"

TestSuite(coroutine_create, .init = coroutine_test_suite_init, .fini = coroutine_test_suite_fini);

static void noop_entry(void *arg) { (void)arg; }

Test(coroutine_create, null_co) {
    histrio_result_t r = coroutine_create(NULL);
    cr_assert_eq(r.code, HISTRIO_ERR_INVALID_ARGUMENT);
}

Test(coroutine_create, null_stack) {
    coroutine_t co = {0};
    co.entry = noop_entry;
    co.cleanup = coroutine_test_stack_cleanup;
    co.stack_size = COROUTINE_TEST_STACK_SIZE;

    histrio_result_t r = coroutine_create(&co);
    cr_assert_eq(r.code, HISTRIO_ERR_INVALID_ARGUMENT);
}

Test(coroutine_create, null_entry) {
    coroutine_t co = {0};
    co.stack = coroutine_test_stack_alloc(COROUTINE_TEST_STACK_SIZE);
    co.cleanup = coroutine_test_stack_cleanup;
    co.stack_size = COROUTINE_TEST_STACK_SIZE;

    histrio_result_t r = coroutine_create(&co);
    cr_assert_eq(r.code, HISTRIO_ERR_INVALID_ARGUMENT);

    coroutine_test_stack_cleanup(co.stack, COROUTINE_TEST_STACK_SIZE);
}

Test(coroutine_create, null_cleanup) {
    coroutine_t co = {0};
    co.stack = coroutine_test_stack_alloc(COROUTINE_TEST_STACK_SIZE);
    co.entry = noop_entry;
    co.stack_size = COROUTINE_TEST_STACK_SIZE;

    histrio_result_t r = coroutine_create(&co);
    cr_assert_eq(r.code, HISTRIO_ERR_INVALID_ARGUMENT);

    coroutine_test_stack_cleanup(co.stack, COROUTINE_TEST_STACK_SIZE);
}

// stack_size <= 128 is rejected: 128 bytes are reserved entirely for the
// red zone, leaving nothing usable.
Test(coroutine_create, stack_size_at_boundary_fails) {
    coroutine_t co = {0};
    co.stack = coroutine_test_stack_alloc(256);
    co.entry = noop_entry;
    co.cleanup = coroutine_test_stack_cleanup;
    co.stack_size = 128;

    histrio_result_t r = coroutine_create(&co);
    cr_assert_eq(r.code, HISTRIO_ERR_INVALID_ARGUMENT);

    coroutine_test_stack_cleanup(co.stack, 256);
}

Test(coroutine_create, stack_size_just_above_boundary_succeeds) {
    coroutine_t co = {0};
    co.stack = coroutine_test_stack_alloc(256);
    co.entry = noop_entry;
    co.cleanup = coroutine_test_stack_cleanup;
    co.stack_size = 129;

    histrio_result_t r = coroutine_create(&co);
    cr_assert_eq(r.code, HISTRIO_OK);
    cr_assert_eq(co.state, COROUTINE_READY);

    coroutine_test_stack_cleanup(co.stack, 256);
}

Test(coroutine_create, success_sets_ready_state) {
    coroutine_t co = {0};
    co.stack = coroutine_test_stack_alloc(COROUTINE_TEST_STACK_SIZE);
    co.entry = noop_entry;
    co.cleanup = coroutine_test_stack_cleanup;
    co.stack_size = COROUTINE_TEST_STACK_SIZE;

    histrio_result_t r = coroutine_create(&co);
    cr_assert_eq(r.code, HISTRIO_OK);
    cr_assert_null(r.message);
    cr_assert_eq(co.state, COROUTINE_READY);

    coroutine_stack_free(&co);
}
