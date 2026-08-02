#ifndef HISTRIO_COROUTINE_TEST_FIXTURES_H
#define HISTRIO_COROUTINE_TEST_FIXTURES_H

#include "histrio/telemetry/log.h"

#define COROUTINE_TEST_STACK_SIZE ((size_t)(64 * 1024))

static inline void coroutine_test_suite_init(void) {
    (void)histrio_log_init(HISTRIO_LOG_WARN);
}

static inline void coroutine_test_suite_fini(void) {
    histrio_log_shutdown();
}

#endif // HISTRIO_COROUTINE_TEST_FIXTURES_H
