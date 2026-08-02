#include <criterion/criterion.h>

#include "histrio/coroutine.h"

#include "support/fixtures.h"
#include "support/stack_alloc.h"

TestSuite(coroutine_multiple, .init = coroutine_test_suite_init, .fini = coroutine_test_suite_fini);

static int a_log[8], a_log_n;
static int b_log[8], b_log_n;

static void a_entry(void *arg) {
    (void)arg;
    for (int i = 0; i < 4; i++) {
        a_log[a_log_n++] = 100 + i;
        coroutine_yield();
    }
}

static void b_entry(void *arg) {
    (void)arg;
    for (int i = 0; i < 4; i++) {
        b_log[b_log_n++] = 200 + i;
        coroutine_yield();
    }
}

// Two flat, independently-driven coroutines (the normal, non-nested
// multi-coroutine pattern) interleaved by the test driver, verifying
// neither's state/stack leaks into the other's.
Test(coroutine_multiple, two_interleaved_coroutines_dont_cross_contaminate, .timeout = 2.0) {
    coroutine_t a = {0}, b = {0};
    a.stack = coroutine_test_stack_alloc(COROUTINE_TEST_STACK_SIZE);
    a.stack_size = COROUTINE_TEST_STACK_SIZE;
    a.entry = a_entry;
    a.cleanup = coroutine_test_stack_cleanup;

    b.stack = coroutine_test_stack_alloc(COROUTINE_TEST_STACK_SIZE);
    b.stack_size = COROUTINE_TEST_STACK_SIZE;
    b.entry = b_entry;
    b.cleanup = coroutine_test_stack_cleanup;

    a_log_n = 0;
    b_log_n = 0;
    cr_assert_eq(coroutine_create(&a).code, HISTRIO_OK);
    cr_assert_eq(coroutine_create(&b).code, HISTRIO_OK);

    // 4 yields each means 5 resumes each to actually reach DEAD (the 5th
    // resume is what lets entry() return after its final yield).
    while (a.state != COROUTINE_DEAD || b.state != COROUTINE_DEAD) {
        if (a.state != COROUTINE_DEAD) coroutine_resume(&a);
        if (b.state != COROUTINE_DEAD) coroutine_resume(&b);
    }

    cr_assert_eq(a.state, COROUTINE_DEAD);
    cr_assert_eq(b.state, COROUTINE_DEAD);
    cr_assert_eq(a_log_n, 4);
    cr_assert_eq(b_log_n, 4);
    for (int i = 0; i < 4; i++) {
        cr_assert_eq(a_log[i], 100 + i);
        cr_assert_eq(b_log[i], 200 + i);
    }

    coroutine_stack_free(&a);
    coroutine_stack_free(&b);
}

#define NUM_CO 5
static long counters[NUM_CO];

static void counting_entry(void *arg) {
    long *counter = (long *)arg;
    for (int i = 0; i < 50; i++) {
        (*counter)++;
        coroutine_yield();
    }
}

Test(coroutine_multiple, many_coroutines_maintain_independent_counters, .timeout = 3.0) {
    coroutine_t cos[NUM_CO];
    for (int i = 0; i < NUM_CO; i++) {
        counters[i] = 0;
        cos[i] = (coroutine_t){0};
        cos[i].stack = coroutine_test_stack_alloc(COROUTINE_TEST_STACK_SIZE);
        cos[i].stack_size = COROUTINE_TEST_STACK_SIZE;
        cos[i].entry = counting_entry;
        cos[i].arg = &counters[i];
        cos[i].cleanup = coroutine_test_stack_cleanup;
        cr_assert_eq(coroutine_create(&cos[i]).code, HISTRIO_OK);
    }

    int any_alive = 1;
    while (any_alive) {
        any_alive = 0;
        for (int i = 0; i < NUM_CO; i++) {
            if (cos[i].state != COROUTINE_DEAD) {
                coroutine_resume(&cos[i]);
                any_alive = 1;
            }
        }
    }

    for (int i = 0; i < NUM_CO; i++) {
        cr_assert_eq(counters[i], 50);
        coroutine_stack_free(&cos[i]);
    }
}
