#include <criterion/criterion.h>
#include <stdint.h>

#include "histrio/coroutine.h"

#include "support/fixtures.h"
#include "support/stack_alloc.h"

TestSuite(coroutine_context_switch, .init = coroutine_test_suite_init, .fini = coroutine_test_suite_fini);

static int reg_failures = 0;

static void register_probe_entry(void *arg) {
    (void)arg;

    register int64_t rbx_v __asm__("rbx") = 0x1111111111111111LL;
    register int64_t r12_v __asm__("r12") = 0x1212121212121212LL;
    register int64_t r13_v __asm__("r13") = 0x1313131313131313LL;
    register int64_t r14_v __asm__("r14") = 0x1414141414141414LL;
    register int64_t r15_v __asm__("r15") = 0x1515151515151515LL;
    // Force the compiler to actually materialize the sentinels into the
    // named registers before the switch happens.
    __asm__ volatile("" :: "r"(rbx_v), "r"(r12_v), "r"(r13_v), "r"(r14_v), "r"(r15_v));

    coroutine_yield();

    __asm__ volatile("" : "=r"(rbx_v), "=r"(r12_v), "=r"(r13_v), "=r"(r14_v), "=r"(r15_v));

    reg_failures = 0;
    if (rbx_v != 0x1111111111111111LL) reg_failures++;
    if (r12_v != 0x1212121212121212LL) reg_failures++;
    if (r13_v != 0x1313131313131313LL) reg_failures++;
    if (r14_v != 0x1414141414141414LL) reg_failures++;
    if (r15_v != 0x1515151515151515LL) reg_failures++;
}

Test(coroutine_context_switch, callee_saved_registers_survive_yield, .timeout = 2.0) {
    coroutine_t co = {0};
    co.stack = coroutine_test_stack_alloc(COROUTINE_TEST_STACK_SIZE);
    co.stack_size = COROUTINE_TEST_STACK_SIZE;
    co.entry = register_probe_entry;
    co.cleanup = coroutine_test_stack_cleanup;

    cr_assert_eq(coroutine_create(&co).code, HISTRIO_OK);

    coroutine_resume(&co); // runs up to the yield
    cr_assert_eq(co.state, COROUTINE_SUSPENDED);

    coroutine_resume(&co); // resumes, checks registers, runs to completion
    cr_assert_eq(co.state, COROUTINE_DEAD);

    cr_assert_eq(reg_failures, 0);

    coroutine_stack_free(&co);
}
