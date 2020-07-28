/*
 * vim:ts=4:sw=4:expandtab
 *
 * memtester - customized memtester-4.3.0
 *
 * util.c: Utilities for memtester
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "types.h"

ull virt2phys(ull vaddr)
{
    ull frame, paddr, pfnmask, pagemask;
    int pagesize = sysconf(_SC_PAGESIZE);
    off64_t off = ((uintptr_t)vaddr) / pagesize * 8;
    int fd = open("/proc/self/pagemap", O_RDONLY);

    if (fd < 0)
        return 0;

    if (lseek64(fd, off, SEEK_SET) != off || read(fd, &frame, 8) != 8) {
        int err = errno;
        fprintf(stderr, "FAILURE: failed to access /proc/self/pagemap with errno %d (%s)\n", err, strerror(err));
        if (fd >= 0)
            close(fd);
        return 0;
    }
    close(fd);

    /* Check if page is present and not swapped. */
    if (!(frame & (1ULL << 63)) || (frame & (1ULL << 62)))
        return 0;

    /* pfn is bits 0-54. */
    pfnmask = ((1ULL << 55) - 1);
    /* Pagesize had better be a power of 2. */
    pagemask = pagesize - 1;

    paddr = ((frame & pfnmask) * pagesize) | ((uintptr_t)vaddr & pagemask);
    return paddr;
}

int find_byteinvert(ul v1, ul v2)
{
    size_t i;
    ul t1, t2;

    for (i = 0; i < sizeof(ul); i++) {
        t1 = (v1 >> (i << 3)) & 0xFFUL;
        t2 = (v2 >> (i << 3)) & 0xFFUL;

        if ((t1 + t2) == 0xFF) {
            return 1;
        }
    }

    return 0;
}

int find_bitflips(ul v1, ul v2)
{
    size_t i;
    ul t1, t2;

    for (i = 0; i < sizeof(ul); i++) {
        t1 = (v1 >> (i << 3)) & 0xFFUL;
        t2 = (v2 >> (i << 3)) & 0xFFUL;

        if (t1 ^ t2) {
            return 1;
        }
    }

    return 0;
}
