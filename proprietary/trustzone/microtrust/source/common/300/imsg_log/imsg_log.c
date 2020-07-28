/*
 * Copyright (c) 2015-2016 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#include <time.h>
#include <imsg_log.h>
#include <android/log.h>

#define KMSG_DEV_FILE       "/dev/kmsg"
#define LOG_BUF_MAX  512

static int kernel_log_fd = -1;

static void kernel_log_init()
{
    if (kernel_log_fd >= 0)
        return;

    kernel_log_fd = open(KMSG_DEV_FILE, O_WRONLY);

    if (kernel_log_fd < 0) {
        IMSG_PRINT_MAINLOG("fail to open %s!\n", KMSG_DEV_FILE);
        return;
    }
    fcntl(kernel_log_fd, F_SETFD, FD_CLOEXEC);
}

void kernel_log_write(const char *fmt, ...)
{
    va_list ap;
    char buf[LOG_BUF_MAX] = {0};

    kernel_log_init();

    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_MAX, fmt, ap);
    buf[LOG_BUF_MAX - 1] = 0;
    write(kernel_log_fd, buf, strlen(buf));
    va_end(ap);
}

int imsg_log_write(int prio, const char *tag,
                   const char *fmt, ...)
{
    va_list ap;
    char buf[LOG_BUF_MAX];

    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_MAX, fmt, ap);
    va_end(ap);

    return __android_log_write(prio, tag, buf);
}
