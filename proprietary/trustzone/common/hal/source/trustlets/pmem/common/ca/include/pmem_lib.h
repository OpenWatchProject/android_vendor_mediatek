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

#ifndef PMEM_LIB_H
#define PMEM_LIB_H

#include <sys/ioctl.h>

#define PMEM_DEV "/proc/pmem0"

#define MAX_NAME_SIZE 32

struct pmem_param {
    uint32_t size;      /* INOUT */
    uint32_t alignment; /* IN */
    uint32_t handle;    /* INOUT */
    uint32_t refcount;  /* OUT */
    uint32_t id;        /* IN */
    uint32_t flags;     /* IN */
    uint8_t owner[MAX_NAME_SIZE];
    uint32_t owner_len;
};

// clang-format off
#define PMEM_IOC_MAGIC          'P'
#define PMEM_MEM_ALLOC          _IOWR(PMEM_IOC_MAGIC, 1, struct pmem_param)
#define PMEM_MEM_UNREF          _IOWR(PMEM_IOC_MAGIC, 2, struct pmem_param)
#define PMEM_IOC_MAXNR          (2)
// clang-format on

#define ION_FLAG_MM_HEAP_INIT_ZERO (1 << 16)

int pmem_open();
int pmem_close(int fd);
int pmem_alloc(int fd, uint32_t size, uint32_t align, uint32_t flags,
               uint32_t* handle);
int pmem_free(int fd, uint32_t handle);
int pmem_read(int pmem_fd, char* buf, int buf_len);

#endif /* end of PMEM_LIB_H */
