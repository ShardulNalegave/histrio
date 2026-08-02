
#include "histrio/telemetry/log.h"

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <cstdarg>
#include <cstdio>
#include <vector>

namespace {

spdlog::level::level_enum to_spdlog_level(histrio_log_level_t level) {
    switch (level) {
        case HISTRIO_LOG_TRACE:    return spdlog::level::trace;
        case HISTRIO_LOG_DEBUG:    return spdlog::level::debug;
        case HISTRIO_LOG_INFO:     return spdlog::level::info;
        case HISTRIO_LOG_WARN:     return spdlog::level::warn;
        case HISTRIO_LOG_ERROR:    return spdlog::level::err;
        case HISTRIO_LOG_CRITICAL: return spdlog::level::critical;
        default:                   return spdlog::level::off;
    }
}

void log_formatted(spdlog::level::level_enum level, const char *fmt, va_list args) {
    try {
        auto logger = spdlog::default_logger();
        if (!logger || !logger->should_log(level)) {
            return;
        }

        char stack_buf[256];
        va_list args_copy;
        va_copy(args_copy, args);
        int needed = std::vsnprintf(stack_buf, sizeof(stack_buf), fmt, args_copy);
        va_end(args_copy);

        if (needed < 0) {
            return;
        }

        if (static_cast<size_t>(needed) < sizeof(stack_buf)) {
            logger->log(level, stack_buf);
            return;
        }

        std::vector<char> heap_buf(static_cast<size_t>(needed) + 1);
        std::vsnprintf(heap_buf.data(), heap_buf.size(), fmt, args);
        logger->log(level, heap_buf.data());
    } catch (...) {
        // logging is best-effort
    }
}

}  // namespace

extern "C" histrio_status_t histrio_log_init(histrio_log_level_t initial_level) {
    try {
        spdlog::init_thread_pool(8192, 1);
        auto logger = spdlog::create_async_nb<spdlog::sinks::stdout_color_sink_mt>("histrio");

        logger->set_level(to_spdlog_level(initial_level));
        logger->flush_on(spdlog::level::err);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

        spdlog::set_default_logger(logger);
        return HISTRIO_OK;
    } catch (...) {
        return HISTRIO_ERR_INTERNAL;
    }
}

extern "C" void histrio_log_shutdown(void) {
    try {
        spdlog::shutdown();
    } catch (...) {
        // best-effort — nothing left to do if shutdown itself fails
    }
}

extern "C" void histrio_log_set_level(histrio_log_level_t level) {
    try {
        spdlog::set_level(to_spdlog_level(level));
    } catch (...) {
    }
}

extern "C" void histrio_log(histrio_log_level_t level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_formatted(to_spdlog_level(level), fmt, args);
    va_end(args);
}
