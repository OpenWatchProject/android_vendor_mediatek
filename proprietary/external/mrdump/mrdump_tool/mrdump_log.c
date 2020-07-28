/* vprintf, vsnprintf */
#include <stdio.h>

/* exit(), atoi() */
#include <stdlib.h>

/* mrdump related */
#include "mrdump_log.h"


static int mdlog_level = LOG_INFO;
static int mdlog_write_syslog = 0;

void mdlog_init(int level, int write_syslog)
{
    mdlog_level = level;
    mdlog_write_syslog = write_syslog;
    openlog("mrdump_tool", 0, 0);
}

void mdlog_printf(int log_prio, const char *fmt, ...)
{
    va_list ap;

    if (log_prio > mdlog_level) {
        return;
    }

    va_start(ap, fmt);
    if (mdlog_write_syslog) {
        vsyslog(log_prio, fmt, ap);
    }
    else {
        char prefix;
        if (log_prio <= LOG_ERR) {
            prefix = 'E';
        } else if (log_prio == LOG_WARNING) {
            prefix = 'W';
        } else if (log_prio <= LOG_INFO) {
            prefix = 'I';
        } else {
            prefix = 'D';
        }
        putc(prefix, stderr);
        putc(':', stderr);
        vfprintf(stderr, fmt, ap);
    }
    va_end(ap);
}

void error(const char *msg, ...)
{
    va_list ap;
    char msgbuf[128];

    va_start(ap, msg);
    if (mdlog_write_syslog) {
        vsyslog(LOG_ERR, msg, ap);
    }
    else {
        vsnprintf(msgbuf, sizeof(msgbuf), msg, ap);
        fprintf(stderr, "Error: %s", msgbuf);
    }
    va_end(ap);

    exit(2);
}
