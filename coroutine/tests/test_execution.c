#include <criterion/criterion.h>
#include <stdint.h>

#include "histrio/coroutine.h"

#include "support/fixtures.h"
#include "support/stack_alloc.h"

TestSuite(coroutine_execution, .init = coroutine_test_suite_init, .fini = coroutine_test_suite_fini);

static void *received_arg = NULL;
static void arg_entry(void *arg) { received_arg = arg; }

Test(coroutine_execution, entry_receives_exact_arg, .timeout = 2.0) {
    int sentinel = 42;
    coroutine_t co = {0};
    co.stack = coroutine_test_stack_alloc(COROUTINE_TEST_STACK_SIZE);
    co.stack_size = COROUTINE_TEST_STACK_SIZE;
    co.entry = arg_entry;
    co.arg = &sentinel;
    co.cleanup = coroutine_test_stack_cleanup;

    cr_assert_eq(coroutine_create(&co).code, HISTRIO_OK);
    coroutine_resume(&co);
    cr_assert_eq(received_arg, &sentinel);

    coroutine_stack_free(&co);
}

static int corruption_detected = 0;
static void data_integrity_entry(void *arg) {
    (void)arg;
    int values[32];
    for (int i = 0; i < 32; i++) values[i] = i * 7;

    for (int iter = 0; iter < 10; iter++) {
        coroutine_yield();
        for (int i = 0; i < 32; i++) {
            if (values[i] != i * 7) corruption_detected = 1;
        }
    }
}

Test(coroutine_execution, data_survives_multiple_yields, .timeout = 2.0) {
    coroutine_t co = {0};
    co.stack = coroutine_test_stack_alloc(COROUTINE_TEST_STACK_SIZE);
    co.stack_size = COROUTINE_TEST_STACK_SIZE;
    co.entry = data_integrity_entry;
    co.cleanup = coroutine_test_stack_cleanup;
    corruption_detected = 0;

    cr_assert_eq(coroutine_create(&co).code, HISTRIO_OK);
    while (co.state != COROUTINE_DEAD) {
        coroutine_resume(&co);
    }

    cr_assert_eq(corruption_detected, 0);
    coroutine_stack_free(&co);
}

static void *local_addr = NULL;
static void stack_isolation_entry(void *arg) {
    (void)arg;
    int local_var;
    local_addr = &local_var;
    coroutine_yield();
}

Test(coroutine_execution, runs_on_its_own_stack, .timeout = 2.0) {
    void *stack = coroutine_test_stack_alloc(COROUTINE_TEST_STACK_SIZE);
    coroutine_t co = {0};
    co.stack = stack;
    co.stack_size = COROUTINE_TEST_STACK_SIZE;
    co.entry = stack_isolation_entry;
    co.cleanup = coroutine_test_stack_cleanup;
    cr_assert_eq(coroutine_create(&co).code, HISTRIO_OK);

    int caller_local;
    coroutine_resume(&co);

    uintptr_t lo = (uintptr_t)stack;
    uintptr_t hi = lo + COROUTINE_TEST_STACK_SIZE;
    uintptr_t addr = (uintptr_t)local_addr;
    cr_assert(addr >= lo && addr < hi,
        "coroutine-local variable at %p not within assigned stack [%p,%p)",
        (void *)addr, (void *)lo, (void *)hi);

    uintptr_t caller_addr = (uintptr_t)&caller_local;
    cr_assert(caller_addr < lo || caller_addr >= hi,
        "caller's own stack unexpectedly overlaps the coroutine's stack region");

    coroutine_resume(&co); // let it finish
    coroutine_stack_free(&co);
}

static long stress_counter = 0;
static void stress_entry(void *arg) {
    long n = *(long *)arg;
    for (long i = 0; i < n; i++) {
        stress_counter++;
        coroutine_yield();
    }
}

Test(coroutine_execution, survives_many_yield_resume_cycles, .timeout = 5.0) {
    long n = 20000;
    coroutine_t co = {0};
    co.stack = coroutine_test_stack_alloc(COROUTINE_TEST_STACK_SIZE);
    co.stack_size = COROUTINE_TEST_STACK_SIZE;
    co.entry = stress_entry;
    co.arg = &n;
    co.cleanup = coroutine_test_stack_cleanup;
    stress_counter = 0;

    cr_assert_eq(coroutine_create(&co).code, HISTRIO_OK);
    while (co.state != COROUTINE_DEAD) {
        coroutine_resume(&co);
    }

    cr_assert_eq(stress_counter, n);
    coroutine_stack_free(&co);
}
