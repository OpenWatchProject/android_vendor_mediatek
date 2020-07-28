/*
 * Copyright (c) 2018 MediaTek Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define LOG_TAG "PMEM"
/* LOG_TAG need to be defined before include log.h */
#include <android/log.h>

#include "pmem_dbg.h"
#include "pmem_lib.h"

static int pmem_ioctl(int pmem_fd, unsigned int cmd, struct pmem_param* param)
{
    char const* owner = LOG_TAG;

    strncpy((char*)param->owner, owner, strlen(owner));
    param->owner_len = strlen((char*)param->owner) + 1;
    param->id = pmem_fd;

    return ioctl(pmem_fd, cmd, param);
}

int pmem_read(int pmem_fd, char* buf, int buf_len)
{
    int ret = PMEM_SUCCESS;

    if ((ret = read(pmem_fd, buf, buf_len)) < 0) {
        PMEM_ERROR("read device '%s'(R) failed: (%s)!", PMEM_DEV,
                   strerror(errno));
        return ret;
    }

    return PMEM_SUCCESS;
}

int pmem_open()
{
    int pmem_fd = -1;

    pmem_fd = open(PMEM_DEV, O_RDWR, 0);
    if (pmem_fd < 0) {
        PMEM_WARN("open device '%s'(RW) failed: (%s)!", PMEM_DEV,
                  strerror(errno));

        pmem_fd = open(PMEM_DEV, O_RDONLY, 0);
        if (pmem_fd < 0) {
            PMEM_ERROR("open device '%s'(R) failed: (%s)!", PMEM_DEV,
                       strerror(errno));
        }
    }

    return pmem_fd;
}

int pmem_close(int fd)
{
    return close(fd);
}

int pmem_alloc(int fd, uint32_t size, uint32_t align, uint32_t flags,
               uint32_t* handle)
{
    struct pmem_param param;
    int ret;

    memset(&param, 0, sizeof(struct pmem_param));
    param.size = size;
    param.alignment = align;
    param.flags = flags;
    if ((ret = pmem_ioctl(fd, PMEM_MEM_ALLOC, &param)) != 0) {
        PMEM_ERROR("alloc protect memory failed: (%s)!", strerror(errno));
        return ret;
    }

    PMEM_DEBUG("allocated handle: 0x%x", param.handle);

    *handle = param.handle;
    return PMEM_SUCCESS;
}

int pmem_free(int fd, uint32_t handle)
{
    struct pmem_param param;
    int ret;

    PMEM_DEBUG("free handle: 0x%x", handle);

    memset(&param, 0, sizeof(struct pmem_param));
    param.handle = handle;
    if ((ret = pmem_ioctl(fd, PMEM_MEM_UNREF, &param)) != 0) {
        PMEM_ERROR("free protect memory failed: (%s)!", strerror(errno));
        return ret;
    }

    return PMEM_SUCCESS;
}
