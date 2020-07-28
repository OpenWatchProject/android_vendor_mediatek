#if !defined(__MRDUMP_USER_PRIVATE_H__)
#define __MRDUMP_USER_PRIVATE_H__

#include <stdbool.h>
#include <stdint.h>

/* DRAM KLOG at MRDUMP area of expdb, offset from bottom = 3145728 - 16384 = 3129344 */
#define MRDUMP_OFFSET 3145728
#define MRDUMP_EXPDB_NAME "/expdb"
#define MRDUMP_KVER "/sys/module/mrdump/version"
#define MRDUMP_LVER "/sys/module/mrdump/parameters/lk"
#define MRDUMP_SIG "MRDUMP08"

struct __attribute__((__packed__)) mrdump_cblock_result {
    char sig[9];
    char status[128];
    char log_buf[2048];
};

struct partinfo {
    int fd;
    uint64_t size;
    uint32_t blksize;
};

#endif
