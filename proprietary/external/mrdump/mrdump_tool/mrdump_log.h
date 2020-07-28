#pragma once

#include <syslog.h>

void mdlog_init(int level, int write_syslog);

void mdlog_printf(int log_prio, const char *fmt, ...);

void error(const char *msg, ...) __attribute__((noreturn));

/* LOG macro support */
#define MD_LOGV(...) mdlog_printf(LOG_DEBUG, __VA_ARGS__)
#define MD_LOGD(...) mdlog_printf(LOG_DEBUG, __VA_ARGS__)
#define MD_LOGI(...) mdlog_printf(LOG_INFO, __VA_ARGS__)
#define MD_LOGW(...) mdlog_printf(LOG_WARNING, __VA_ARGS__)
#define MD_LOGE(...) mdlog_printf(LOG_ERR, __VA_ARGS__)
