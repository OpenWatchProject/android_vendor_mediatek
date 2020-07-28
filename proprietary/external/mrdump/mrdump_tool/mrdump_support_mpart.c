/* uint64_t ...*/
#include <inttypes.h>
#include <stdbool.h>

/* struct stat, lstat, open, execl */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

/* malloc */
#include <stdlib.h>

/* strlen */
#include <string.h>

/* strerror */
#include <string.h>
/* errno */
#include <errno.h>

/* bzero */
#include <strings.h>

/* ctime */
#include <time.h>

/* crc32, Z_NULL */
#include <zlib.h>

/* mrdump related */
#include "mrdump_log.h"
#include "mrdump_common.h"
#include "mrdump_support_mpart.h"
#include "mrdump_support_ext4.h"

/* remember to free the fullpath when it is no longer required. */
static char *get_partition_fullpath(char *linkfile)
{
    struct stat sa;
    if (lstat(linkfile, &sa) == -1) {
        MD_LOGE("%s: no mrdump partition.\n", __func__);
        return NULL;
    }

    char *fullpath = malloc(sa.st_size + 1);
    if (fullpath == NULL) {
        MD_LOGE("%s: insufficient memory\n", __func__);
        free(fullpath);
        return NULL;
    }

    ssize_t r = readlink(linkfile, fullpath, sa.st_size + 1);
    if (r == -1) {
        MD_LOGE("%s: lstat\n", __func__);
        free(fullpath);
        return NULL;
    }
    if (r > sa.st_size) {
        MD_LOGE("%s: symlink increased in size "
                    "between lstat() and readlink()\n", __func__);
        free(fullpath);
        return NULL;
    }
    fullpath[r] = '\0';

    return fullpath;
}

/* remember to free the pname when it is no longer required. */
static char *get_partition_name(char *fullpath)
{
    char *pfile = fullpath;
    if (!pfile) {
        MD_LOGE("%s: no such full path\n", __func__);
        return NULL;
    }
    int len = strlen(pfile);

    char *pname = malloc(len + 1);
    if (pname == NULL) {
        MD_LOGE("%s: malloc failed\n", __func__);
        return NULL;
    }

    char *delim="\x2f";
    char *p = strtok(pfile, delim);
    while (p) {
        p = strtok(NULL, delim);
        if (p) {
            bzero(pname, (len + 1));
            strlcpy(pname, p, len);
        }
    }

    return pname;
}

int mrdump_check_partition(void)
{
    /* get real path from symbolic file */
    char *fullpath = get_partition_fullpath(MRDUMP_MPART_PARTITION);
    if (!fullpath) {
        MD_LOGE("%s: no such fullpath\n", __func__);
        return 0;
    }

    /* get device node name */
    char *realpath = strdup(fullpath);
    char *pname = get_partition_name(realpath);
    if (!pname) {
        MD_LOGE("%s: cannot get the partition name\n", __func__);
        free(fullpath);
        free(realpath);
        return 0;
    }
    free(realpath);

    /* get partition size */
    uint64_t psize = mrdump_get_partition_size(fullpath);

    /* get stat */
    struct stat sb;
    if (stat(fullpath, &sb) == -1) {
        MD_LOGE("%s: no mrdump partition.\n", __func__);
        free(fullpath);
        free(pname);
        return 0;
    }

    /* parition informations, reference to EXAMPLE of manpage STAT(2) */
    MD_LOGI("PARTNAME:                  mrdump\n");
    MD_LOGI("LINK:                      %s\n", MRDUMP_MPART_PARTITION);
    MD_LOGI("Partition size:            %" PRIu64 " bytes\n", psize);
    MD_LOGI("DEVNAME:                   %s\n", pname);
    MD_LOGI("FILE:                      %s\n", fullpath);
    MD_LOGI("File type:                 ");
    switch (sb.st_mode & S_IFMT) {
        case S_IFBLK:  MD_LOGI("block device\n");            break;
        case S_IFCHR:  MD_LOGI("character device\n");        break;
        case S_IFDIR:  MD_LOGI("directory\n");               break;
        case S_IFIFO:  MD_LOGI("FIFO/pipe\n");               break;
        case S_IFLNK:  MD_LOGI("symlink\n");                 break;
        case S_IFREG:  MD_LOGI("regular file\n");            break;
        case S_IFSOCK: MD_LOGI("socket\n");                  break;
        default:       MD_LOGI("unknown?\n");                break;
    }
    MD_LOGI("Ownership:                 UID=%ld   GID=%ld\n",
            (long) sb.st_uid, (long) sb.st_gid);
    MD_LOGI("I-node number:             %ld\n", (long) sb.st_ino);
    MD_LOGI("Mode:                      %lo (octal)\n", (unsigned long) sb.st_mode);
    MD_LOGI("Link count:                %ld\n", (long) sb.st_nlink);
    MD_LOGI("Last status change:        %s\n", ctime(&sb.st_ctime));
    MD_LOGI("Last file access:          %s\n", ctime(&sb.st_atime));
    MD_LOGI("Last file modification:    %s\n", ctime(&sb.st_mtime));

    /* remember to free fullpath and pname */
    free(fullpath);
    free(pname);

    return 1;
}

static bool mrdump_read_pafile_info(int device_fd,
                                    struct mrdump_pafile_info *info)
{
    memset(info, 0, sizeof(struct mrdump_pafile_info));

    uint8_t block0[MRDUMP_PAF_TOTAL_SIZE];
    memset(block0, 0, sizeof(block0));

    ssize_t readsize = read(device_fd, block0, sizeof(block0));
    if((readsize < 0) || (readsize > sizeof(block0))) {
        MD_LOGE("%s: read InfoLBA error (%d), %s\n", __func__, errno, strerror(errno));
        return false;
    }

    uint8_t *bufp = &block0[0];
    unsigned int crcval = crc32(0, Z_NULL, 0);
    crcval = crc32(crcval, (void *)block0, MRDUMP_LBA_DATAONLY);
    if (crcval != *(uint32_t *)&block0[MRDUMP_PAF_CRC32]) {
        MD_LOGE("%s: LBA info CRC error (c:%08x, v:%08x)\n", __func__, crcval,
                *(uint32_t *)&block0[MRDUMP_PAF_CRC32]);
        return false;
    }

    info->coredump_size = *(uint64_t *)(bufp + MRDUMP_PAF_COREDUMPSIZE);
    return true;
}

bool mrdump_file_fetch_zip_coredump_partition(const char *outfile)
{
    struct mrdump_pafile_info lbainfo;
    unsigned int       read_len, mylen;
    unsigned char      MyData[MRDUMP_PARTITION_EXSPACE];
    int                fpRead, fpWrite;

    // outfile
    if(outfile == NULL) {
        MD_LOGE("%s: outfile is NULL! (%d), %s\n", __func__, errno, strerror(errno));
        return false;
    }

    fpRead = open(MRDUMP_MPART_PARTITION, O_RDWR);
    if (fpRead < 0) {
        MD_LOGE("mrdump partition %s open failed (%d), %s\n",
                MRDUMP_MPART_PARTITION, errno, strerror(errno));
        return false;
    }

    if (!mrdump_read_pafile_info(fpRead, &lbainfo)) {
        mrdump_close(fpRead);
        return false;
    }

    if (lbainfo.coredump_size == 0) {
        MD_LOGI("mrdump partition have 0 size dump, no data to dump\n");
        mrdump_close(fpRead);
        return false;
    }

    // Write handle
    fpWrite = open(outfile, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if(0 > fpWrite) {
        MD_LOGE("%s: open Write handle open failed(%d), %s\n", __func__, errno, strerror(errno));
        mrdump_close(fpRead);
        goto cleanup0;
    }

    uint64_t delaylen = 0;
    uint64_t coresize = lbainfo.coredump_size;

    if (lseek64(fpRead, BLK_SIZE, SEEK_SET) == (off64_t) -1) {
        MD_LOGE("%s: lseek64 Read MyData failed(%d), %s", __func__, errno, strerror(errno));
        mrdump_close(fpRead);
        mrdump_close(fpWrite);
        unlink(outfile);
        goto cleanup0;
    }

    while(coresize > 0) {

        // counting coutinue datas...
        read_len = MAX_READ_BLK * BLK_SIZE;

        if(coresize < read_len) {
            mylen = coresize;
        } else {
            mylen = read_len;
        }

        ssize_t readsize = read(fpRead, MyData, mylen);
        if((readsize < 0) || (readsize > mylen)) {
            MD_LOGE("%s: fetch MyData error!(%d), %s", __func__, errno, strerror(errno));
            mrdump_close(fpRead);
            mrdump_close(fpWrite);
            unlink(outfile);
            goto cleanup0;
        }
        if(0 > write(fpWrite, MyData, mylen)) {
            MD_LOGE("%s: fetch MyData error!(%d), %s", __func__, errno, strerror(errno));
            mrdump_close(fpRead);
            mrdump_close(fpWrite);
            unlink(outfile);
            goto cleanup0;
        }

        coresize -= mylen;

        /* flow control */
        delaylen += mylen;
        if (delaylen >= MRDUMP_MAX_BANDWIDTH) {
            delaylen = 0;
            fdatasync(fpWrite);
            sleep(1);
        }
    }

    mrdump_close(fpRead);
    mrdump_close(fpWrite);

    MD_LOGI("Ramdump write to %s size %" PRId64 "\n", outfile, lbainfo.coredump_size);
    return true;

  cleanup0:
    return false;
}
