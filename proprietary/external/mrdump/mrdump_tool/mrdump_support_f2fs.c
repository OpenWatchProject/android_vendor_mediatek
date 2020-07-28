/* uint64_t ...*/
#include <inttypes.h>
#include <stdbool.h>

/* statfs() */
#include <sys/statfs.h>

/* mmap, munmap */
#include <sys/mman.h>

/* fsync(), close(), write(), unlink() */
#include <unistd.h>

/* __u32 */
#include <linux/types.h>

/* ioctl */
//#include <linux/fs.h>
#include <sys/ioctl.h>

/* struct stat, open, posix_fadvise */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* strerror, memset */
#include <string.h>
/* errno */
#include <errno.h>

/* mrdump related */
#include "mrdump_log.h"
#include "mrdump_common.h"
#include "mrdump_support_f2fs.h"

bool mount_as_f2fs(const char *mountp)
{
    struct statfs fs;

    /* initialization */
    memset((void *)&fs, 0, sizeof(fs));

    if(statfs(mountp, &fs) == 0) {
        if(fs.f_type == F2FS_SUPER_MAGIC)
            return true;

        return false;
    }
    MD_LOGE("%s: %s statfs error.\n", __func__, mountp);
    return false;
}

bool f2fs_fallocate(const char *allocfile, uint64_t allocsize)
{
    int i;
    int zQ = (allocsize + F2FS_MAPSIZE - 1) / F2FS_MAPSIZE;
    unsigned int val;

    if((allocfile == NULL) || (allocsize == 0)) {
        MD_LOGE("%s: allocfile is NULL or allocsize = %" PRIu64 "\n", __func__, allocsize);
        return false;
    }

    void *ZeroPage = mmap(NULL, F2FS_MAPSIZE, PROT_READ, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if(ZeroPage == NULL) {
        MD_LOGE("%s: ZeroPage map failed.\n", __func__);
        return false;
    }

    int fd = open(allocfile, O_RDWR | O_CREAT, 0400);
    if(0 > fd) {
        MD_LOGE("%s: open fd failed.\n", __func__);
        munmap(ZeroPage, F2FS_MAPSIZE);
        return false;
    }

    val = 31337;
    if(0 > ioctl(fd, F2FS_IOC_SET_PIN_FILE, &val)) {
        MD_LOGE("%s: F2FS_IOC_SET_PIN_FILE(%u) failed(%d), %s\n", __func__, val, errno, strerror(errno));
    }

    uint64_t delaylen = 0;
    off_t offset = 0;
    int res;
    for(i=0; i<zQ; i++) {
        if(0 >= write(fd,ZeroPage, F2FS_MAPSIZE)) {
            MD_LOGE("%s: ZeroPage write failed\n", __func__);
            close(fd);
            unlink(allocfile);
            munmap(ZeroPage, F2FS_MAPSIZE);
            return false;
        }

        /* flow control and drop caches */
        offset += F2FS_MAPSIZE;
        delaylen += F2FS_MAPSIZE;
        if (delaylen >= MRDUMP_MAX_BANDWIDTH) {
            delaylen = 0;
            sleep(1);
            res = posix_fadvise(fd, 0, offset, POSIX_FADV_DONTNEED);
            if (res != 0) {
                MD_LOGI("%s: posix_fadvise failed(%d, %dMB written)\n",
                        __func__, res, (i*F2FS_MAPUNIT));
            }
        }
    }

    fsync(fd);

    close(fd);
    munmap(ZeroPage, F2FS_MAPSIZE);

    return true;
}
