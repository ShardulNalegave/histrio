
#ifndef HISTRIO_TELEMETRY_LOG_H
#define HISTRIO_TELEMETRY_LOG_H

#include "histrio/status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum histrio_log_level {
    HISTRIO_LOG_TRACE,
    HISTRIO_LOG_DEBUG,
    HISTRIO_LOG_INFO,
    HISTRIO_LOG_WARN,
    HISTRIO_LOG_ERROR,
    HISTRIO_LOG_CRITICAL,
    HISTRIO_LOG_OFF
} histrio_log_level_t;

[[nodiscard]]
histrio_status_t histrio_log_init(histrio_log_level_t initial_level);

void histrio_log_shutdown(void);

void histrio_log_set_level(histrio_log_level_t level);

void histrio_log(histrio_log_level_t level, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

#ifdef __cplusplus
}
#endif

#define HISTRIO_TRACE(...)    histrio_log(HISTRIO_LOG_TRACE, __VA_ARGS__)
#define HISTRIO_DEBUG(...)    histrio_log(HISTRIO_LOG_DEBUG, __VA_ARGS__)
#define HISTRIO_INFO(...)     histrio_log(HISTRIO_LOG_INFO, __VA_ARGS__)
#define HISTRIO_WARN(...)     histrio_log(HISTRIO_LOG_WARN, __VA_ARGS__)
#define HISTRIO_ERROR(...)    histrio_log(HISTRIO_LOG_ERROR, __VA_ARGS__)
#define HISTRIO_CRITICAL(...) histrio_log(HISTRIO_LOG_CRITICAL, __VA_ARGS__)

#endif // HISTRIO_TELEMETRY_LOG_H
