/* uint64_t ...*/
#include <inttypes.h>
#include <stdbool.h>

/* __u32 */
#include <linux/types.h>

/* struct stat, open */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

/* malloc */
#include <stdlib.h>

/* strerror */
#include <string.h>
/* errno */
#include <errno.h>

/* figetbsz */
#include <linux/fs.h>
#include <sys/ioctl.h>

/* Property */
#include <cutils/properties.h>
#include <sys/system_properties.h>

/* statfs() and statvfs() */
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <sys/vfs.h>

/* crc32, Z_NULL */
#include <zlib.h>

/* syscall, __NR_fallocate */
#include <sys/syscall.h>

/* time */
#include <time.h>

/* sysenv */
#include <sysenv_utils.h>

/* mrdump related */
#include "mrdump_log.h"
#include "mrdump_common.h"
#include "mrdump_status.h"
#include "mrdump_support_fiemap.h"
#include "mrdump_support_ext4.h"
#include "mrdump_support_f2fs.h"
#include "mrdump_support_mpart.h"

/* add for block verification */
static uint64_t lba_marker_time;
static struct marked_block_data bdata;

struct palloc_file {
    int fd;
    int blksize;
    int lbaooo;
};

static struct palloc_file *palloc_file_open(const char *allocfile)
{
    struct stat statinfo;

    struct palloc_file *pfile = malloc(sizeof(struct palloc_file));
    if (pfile == NULL) {
        MD_LOGE("%s: malloc return NULL\n", __func__);
        return NULL;
    }
    pfile->fd = -1;
    pfile->blksize = 0;
    pfile->lbaooo = 0;

    /////////////////////////////////////////////////////
    // open file handle
    pfile->fd = open(allocfile, O_RDONLY);
    if(0 > pfile->fd) {
        MD_LOGE("%s: file(%s) marker open failed(%d), %s\n", __func__, allocfile, errno, strerror(errno));
        free(pfile);
        return NULL;
    }
    // Get file statinfo
    if(0 > fstat(pfile->fd, &statinfo)) {
        MD_LOGE("%s: marker stat failed(%d), %s\n", __func__, errno, strerror(errno));
        goto cleanup;
    }

    if(0 > ioctl(pfile->fd, FIGETBSZ, &pfile->blksize)) {
        MD_LOGE("%s: FIGETGSZ failed(%d), %s\n", __func__, errno, strerror(errno));
        goto cleanup;
    }

    pfile->lbaooo = mrdump_fiemap_get_entry_lba(pfile->fd, pfile->blksize, 0);
    if (pfile->lbaooo <= 0) {
        MD_LOGE("%s: get lba failed\n", __func__);
        goto cleanup;
    }

    MD_LOGI("%s: get lbaooo(%d)\n", __func__, pfile->lbaooo);
    return pfile;

  cleanup:
    mrdump_close(pfile->fd);
    pfile->fd = -1;
    pfile->blksize = 0;
    pfile->lbaooo = 0;
    free(pfile);
    return NULL;
}

void palloc_file_close(struct palloc_file *pfile)
{
    mrdump_close(pfile->fd);
    free(pfile);
}

static int fop_file_write_string(const char *path, const char *content, ...)
{
    int ret_val = 0;

    int fd = open(path, O_WRONLY);
    if (fd >= 0) {
        char content_buf[1024];
        va_list ap;

        va_start(ap, content);
        int n = vsnprintf(content_buf, sizeof(content_buf), content, ap);
        va_end(ap);
        int val = write(fd, content_buf, n);
        if (val != n) {
            if (val < 0) {
                ret_val = -errno;
            }
            else {
                ret_val = -EIO;
            }
        }
        mrdump_close(fd);
        if (ret_val == 0) {
            ret_val = n;
        }
    }
    else {
        ret_val = -errno;
    }
    return ret_val;
}

////////////////////////////////////////////
// common function (static in this file)  //
////////////////////////////////////////////
static bool ext4_setup_attr(char const *allocfile, bool enabled)
{
    int fd;
    unsigned int myflags;

    if(allocfile == NULL) {
        allocfile = MRDUMP_EXT4_ALLOCATE_FILE;
    }

    fd = open(allocfile, O_RDONLY);
    if(0 > fd) {
        MD_LOGE("%s: open allocfile failed(%d), %s\n", __func__, errno, strerror(errno));
        return false;
    }

    if(0 > ioctl(fd, FS_IOC_GETFLAGS, &myflags)) {
        MD_LOGE("%s: FS_IOC_GETFLAGS failed(%d), %s\n", __func__, errno, strerror(errno));
        mrdump_close(fd);
        return false;
    }

    if (enabled) {
        myflags |= (FS_SECRM_FL | FS_IMMUTABLE_FL);
    }
    else {
        myflags &= ~(FS_SECRM_FL | FS_IMMUTABLE_FL);
    }
    if(0 > ioctl(fd, FS_IOC_SETFLAGS, &myflags)) {
        MD_LOGE("%s: FS_IOC_SETFLAGS failed(%d), %s\n", __func__, errno, strerror(errno));
        mrdump_close(fd);
        return false;
    }

    mrdump_close(fd);
    return true;
}

static off64_t ext4_block_lseek(int fd, unsigned lba, int blksize)
{
    unsigned long long location = (unsigned long long) lba * (unsigned long long) blksize;
    return lseek64(fd, location, SEEK_SET);
}

static int ext4_open_device_node(const char *mountp)
{
    char *devicenode = mrdump_get_device_node(mountp);
    if(devicenode == NULL) {
        MD_LOGE("%s: Get devicenode return null\n", __func__);
        return -1;
    }
    int fd = open(devicenode, O_RDWR);
    if(0 > fd) {
        MD_LOGE("%s: open devicenode failed(%d), %s\n", __func__, errno, strerror(errno));
        free(devicenode);
        return -1;
    }
    free(devicenode);
    return fd;
}

bool mount_as_ext4(const char *mountp)
{
    struct statfs fs;

    /* initialization */
    memset((void *)&fs, 0, sizeof(fs));

    if(statfs(mountp, &fs) == 0) {
        /*
         * bionic header didn't define EXT4_SUPER_MAGIC
         * use EXT3_SUPER_MAGIC instead
         */
        if(fs.f_type == EXT3_SUPER_MAGIC)
            return true;
        return false;
    }
    MD_LOGE("%s: %s statfs error.\n", __func__, mountp);
    return false;
}

static int check_a_data_block(int myfds, int mybs, uint32_t lba)
{
    unsigned long long offset = (unsigned long long)lba * (unsigned long long)mybs;
    struct marked_block_data output_data;

    /* initialization */
    memset((void *)&output_data, 0, sizeof(output_data));

    unsigned int mycrc = crc32(0, Z_NULL, 0);
    mycrc = crc32(mycrc, (void *)&bdata, (mybs-4));
    bdata.crc = mycrc;
    bdata.lba = lba;

    if(0 > pread64(myfds, (void *)&output_data, mybs, offset)) {
        MD_LOGE("%s: write block error!(%d), %s\n", __func__, errno, strerror(errno));
        return -1;
    };

    // check lba
    if (bdata.lba != output_data.lba) {
        MD_LOGE("Read BlockData LBA failed (c:%u, v:%u)\n", bdata.lba, output_data.lba);
        return -1;
    }
    // check timestamp
    if (bdata.timestamp != output_data.timestamp) {
        MD_LOGE("Read BlockData timestamp failed (c:%" PRIu64 ", v:%" PRIu64 ")\n", bdata.timestamp, output_data.timestamp);
        return -1;
    }
    // check crc
    if (bdata.crc != output_data.crc) {
        MD_LOGE("Read BlockData crc32 failed (c:%u, v:%u)\n", bdata.crc, output_data.crc);
        return -1;
    }

    return 0;
}

static int format_a_data_block(int myfds, int mybs, uint32_t lba)
{
    unsigned long long offset = (unsigned long long)lba * (unsigned long long)mybs;

    bdata.lba = lba;
    // (no change) bdata.timestamp = lba_marker_time;
    unsigned int mycrc = crc32(0, Z_NULL, 0);
    mycrc = crc32(mycrc, (void *)&bdata, (mybs-4));
    bdata.crc = mycrc;

    if(0 > pwrite64(myfds, (void *)&bdata, mybs, offset)) {
        MD_LOGE("%s: write block error!(%d), %s\n", __func__, errno, strerror(errno));
        return -1;
    };

    return 0;
}

static int ext4_get_next_bidx(int myfd, unsigned int *pBlock, unsigned int bbidx, unsigned int bs, unsigned int moves)
{
    unsigned int rlba, mycrc;

    if(bbidx > MRDUMP_EXT4_LBA_PER_BLOCK)
        return -1;

    if(bbidx == MRDUMP_EXT4_LBA_PER_BLOCK) {
        rlba = pBlock[bbidx];
        // seek to location
        if(-1 == ext4_block_lseek(myfd, rlba, bs)) {
            MD_LOGE("%s: lseek64 location failed(%d), %s\n", __func__, errno, strerror(errno));
            return -1;
        }
        // read to pBlock
        ssize_t read_size = read(myfd, pBlock, bs);
        if (read_size < 0) {
            MD_LOGE("%s: read lbaooo error!(%d), %s\n", __func__, errno, strerror(errno));
            return -1;
        }
        if (read_size != bs) {
            MD_LOGE("%s: read size %zd != bs %u\n", __func__, read_size, bs);
            return -1;
        }

        // check crc32
        mycrc = crc32(0, Z_NULL, 0);
        mycrc = crc32(mycrc, (void *)pBlock, (bs-4));
        if(mycrc != pBlock[1023]) {
            MD_LOGE("%s: crc32 error!(%d), %s\n", __func__, errno, strerror(errno));
            return -1;
        }
        bbidx = 0;
    } else {
        bbidx+=moves;
    }
    return bbidx;
}

static unsigned int ext4_num_to_join(unsigned int *pBlock, unsigned int bbidx)
{
    unsigned int i, j;
    for(i=0, j=0; i<MRDUMP_EXT4_MAX_CONTINUE; i++) {
        if((pBlock[bbidx+i] - pBlock[bbidx]) == i) {
            j++;
            continue;
        }
        break;
    }
    return j;
}

static int construct_a_block(int myfds, int mybs, int myloop, int myoffset, struct fiemap_info *mapinfo, unsigned int rows)
{
    unsigned int i, lba, num, mycrc;
    unsigned int block[1024]; // assume max BLOCKSIZE=4096, int(4byte) for addressing

    // Zero blocks
    memset(block, 0, sizeof(block));

    // number of address per block
    // 1024 = Normal_LBA[1022] + Next_LBA + crc32
    num = (mybs+sizeof(int)-1) / sizeof(int);

    // Indexing by 0. --> (i-1)
    // i     = 1~MRDUMP_EXT4_LBA_PER_BLOCK(1022)
    // index = 0~1021
    i = 1;
    while(i < (num - 1)) {
        lba = myoffset + (i-1);
        lba = mrdump_fiemap_get_lba_of_block(mapinfo, rows, lba);
        if(0 > lba) {
            MD_LOGE("%s: mrdump_fiemap_get_lba_of_block(1021) failed(%d), %s\n", __func__, errno, strerror(errno));
            return -1;
        }
        block[(i-1)] = lba;
        i++;
    }

    /* coverity: to avoid while(i < (num - 1)) taking false branch */
    if (i < 2) {
        MD_LOGE("%s: number of address per block should be more than 2\n", __func__);
        return -1;
    }

    // i     = 1023
    // index = MRDUMP_EXT4_LBA_PER_BLOCK(1022)
    if (block[(i-2)] == 0) {
        // End of File
        block[(i-1)] = 0;
    } else {
        // lba = (myloop+1); --> Need ++ For InfoLBA[EXT4_LBA_INFO_NUM];
        // block starts from 2nd Block
        lba = myloop + 2;
        lba = mrdump_fiemap_get_lba_of_block(mapinfo, rows, lba);
        if(0 > lba) {
            MD_LOGE("%s: mrdump_fiemap_get_lba_of_block(1022) failed(%d), %s\n", __func__, errno, strerror(errno));
            return -1;
        }
        block[(i-1)] = lba;
    }

    // i     = 1023: crc32
    // index = 1023: (NO block[i+1] now...)
    // PS: index = 1023 now... i takes no changes.
    mycrc = crc32(0, Z_NULL, 0);
    mycrc = crc32(mycrc, (void *)block, (mybs-4));
    block[i] = mycrc;

    // Prepare to write onto the Blocks in the front ...
    //
    // the address (LBA) where to write on.
    // lba = myloop; --> Need ++ For InfoLBA[EXT4_LBA_INFO_NUM];
    lba = myloop+1;
    lba = mrdump_fiemap_get_lba_of_block(mapinfo, rows, lba);
    if(0 > lba) {
        MD_LOGE("%s: mrdump_fiemap_get_lba_of_block(write) failed(%d), %s\n", __func__, errno, strerror(errno));
        return -1;
    }
    // Write block onto location;
    if (-1 == ext4_block_lseek(myfds, lba, mybs)) {
        MD_LOGE("%s: lseek64 location failed(%d), %s\n", __func__, errno, strerror(errno));
        return -1;
    }
    if(0 > write(myfds, (void *)block, sizeof(block))) {
        MD_LOGE("%s: write block error!(%d), %s\n", __func__, errno, strerror(errno));
        return -1;
    }

    // return the new offset
    // i = 1023
    if (block[(i-1)] == 0) {
        myoffset = 0;
    } else {
        myoffset = myoffset + (i - 1);
    }

    return (int)myoffset;
}

static void mrdump_dump_pafile_info(const uint8_t lba[MRDUMP_PAF_TOTAL_SIZE])
{
    uint32_t version = *(uint32_t *)(lba);
    uint32_t info_lba = *(uint32_t *)(lba + MRDUMP_PAF_INFO_LBA);
    uint32_t addr_lba = *(uint32_t *)(lba + MRDUMP_PAF_ADDR_LBA);
    uint64_t filesize = *(uint64_t *)(lba + MRDUMP_PAF_ALLOCSIZE);
    uint64_t coredump_size = *(uint64_t *)(lba + MRDUMP_PAF_COREDUMPSIZE);
    uint64_t timestamp = *(uint64_t *)(lba + MRDUMP_PAF_TIMESTAMP);
    uint32_t crcval = *(uint32_t *)(lba + MRDUMP_PAF_CRC32);

    MD_LOGD("PAFILE-INFO version %" PRIu32 " info-lba %" PRIu32 " addr-lba %" PRIu32 " file-size "
            "%" PRIu64 " coredump-size %" PRIu64 " timestamp %" PRIx64
            " crc %" PRIx32 "\n",
            version, info_lba, addr_lba, filesize, coredump_size, timestamp, crcval);
}

/* lba format
 *  0: version      (2 bytes)
 *  2:reserved      (2 bytes)
 *  4:info lba      (4 bytes)
 *  8:addr lba      (4 bytes)
 * 12:allocate size (8 bytes)
 * 20:coredump size (8 bytes)
 * 28:timestamp     (8 bytes)
 * 36:CRC32         (4 bytes)
 */
static bool fs_lba_mark_header(const struct palloc_file *pfile)
{
    int num_blocks, rsv_blocks;
    struct stat statinfo;
    unsigned int mycrc, block;
    uint8_t InfoLBA[MRDUMP_PAF_TOTAL_SIZE], *pinfo;

    // Get file statinfo
    if(0 > fstat(pfile->fd, &statinfo)) {
        MD_LOGE("%s: marker stat failed(%d), %s\n", __func__, errno, strerror(errno));
        return false;
    }
    // Get Blocksize and counting blocks
    uint64_t blksize = pfile->blksize;

    // Counting blocks
    num_blocks = (statinfo.st_size + blksize - 1) / blksize;

    /////////////////////////////////////////////////////
    // Zero InfoLBA ...
    memset(InfoLBA, 0, sizeof(InfoLBA));
    pinfo = &InfoLBA[0];

    /* Setup pafile info version */
    *(uint16_t *)pinfo = MRDUMP_PAF_VERSION;

    /* Flags */
    *(uint16_t *)(pinfo + 2) = 0;

    // Get info LBA
    block = mrdump_fiemap_get_entry_lba(pfile->fd, blksize, 0);
    *(uint32_t *)(pinfo + MRDUMP_PAF_INFO_LBA) = block;

    // Get 2nd LBA
    if(2 > mrdump_fiemap_get_entry_tot(pfile->fd, blksize, 0))
        *(uint32_t *)(pinfo + MRDUMP_PAF_ADDR_LBA) = mrdump_fiemap_get_entry_lba(pfile->fd, blksize, 1);
    else
        *(uint32_t *)(pinfo + MRDUMP_PAF_ADDR_LBA) = block + 1;

    // Reserved space for Header and Tailer
    rsv_blocks = num_blocks/MRDUMP_EXT4_LBA_PER_BLOCK;   // Header
    rsv_blocks+= 2;                 // Tailer
    *(uint64_t *)(pinfo + MRDUMP_PAF_ALLOCSIZE) = (num_blocks - rsv_blocks) * blksize;

    // Set core dump size to zero
    *(uint64_t *)(pinfo + MRDUMP_PAF_COREDUMPSIZE) = 0UL;

    // Set timestamp to lba_marker_time
    *(uint64_t *)(pinfo + MRDUMP_PAF_TIMESTAMP) = lba_marker_time;

    // check crc32 of InfoLBA
    mycrc = crc32(0, Z_NULL, 0);
    mycrc = crc32(mycrc, (void *)InfoLBA, MRDUMP_LBA_DATAONLY);
    *(uint32_t *)(pinfo + MRDUMP_PAF_CRC32) = mycrc;

    int fd = ext4_open_device_node(MRDUMP_EXT4_MOUNT_POINT);
    if (fd < 0) {
        return false;
    }
    // Save InfoLBA First
    if(-1 == ext4_block_lseek(fd, block, blksize)) {
        MD_LOGE("%s: marker header lseek64 InfoLBA failed(%d), %s\n", __func__, errno, strerror(errno));
        mrdump_close(fd);
        return false;
    }
    if(0 > write(fd, (void *)InfoLBA, (sizeof(InfoLBA)))) {
        MD_LOGE("%s: marker header write InfoLBA error!(%d), %s\n", __func__, errno, strerror(errno));
        mrdump_close(fd);
        return false;
    }

    mrdump_close(fd);
    return true;
}

static bool fs_lba_mark_body(const struct palloc_file *pfile)
{
    int             fds;
    int             i, blksize, num_blocks, loop, offset;
    struct stat     statinfo;

    // Get file statinfo
    if(0 > fstat(pfile->fd, &statinfo)) {
        MD_LOGE("%s: marker stat failed(%d), %s\n", __func__, errno, strerror(errno));
        return false;
    }
    // Get Blocksize and counting blocks
    blksize = pfile->blksize;
    num_blocks = (statinfo.st_size + blksize - 1) / blksize;

    /////////////////////////////////////////////////////
    // begin to record (2-layer block writing)
    char *devicenode = mrdump_get_device_node(MRDUMP_EXT4_MOUNT_POINT);
    if(devicenode == NULL) {
        MD_LOGE("%s: get devicenode failed(%d), %s\n", __func__, errno, strerror(errno));
        return false;
    } else {
        fds = open(devicenode, O_RDWR);
        if(0 > fds) {
            free(devicenode);
            MD_LOGE("%s: open devicenode failed(%d), %s\n", __func__, errno, strerror(errno));
            return false;
        }
    }
    free(devicenode);

    // for performance, we must finish fiemap here... but not in the loop
    unsigned int my_num = mrdump_fiemap_total_entries(pfile->fd);
    if(my_num == 0) {
        MD_LOGE("%s: mrdump_fiemap_total_entries error!(%d), %s\n", __func__, errno, strerror(errno));
        mrdump_close(fds);
        return false;
    }

#ifdef MRDUMP_DEBUG
    MD_LOGI("%s: my_num=(%06u)\n", __func__, my_num);
#endif

    struct fiemap_info *myinfo = malloc(my_num * sizeof(struct fiemap_info));
    if(!mrdump_fiemap_get_entries(pfile->fd, blksize, myinfo, my_num)) {
        MD_LOGE("%s: mrdump_fiemap_get_entries error!(%d), %s\n", __func__, errno, strerror(errno));
        mrdump_close(fds);
        return false;
    }

#ifdef MRDUMP_DEBUG
    for(i=0; i<(int)my_num; i++) {
        MD_LOGI("%s: i=%03d, LBA=%06u, TOT=%06u\n", __func__, i, myinfo[i].lba, myinfo[i].tot);
    }
#endif

    // go
    loop = (blksize / sizeof(unsigned int)) - 2;
    loop = (num_blocks + loop - 1) / loop;
#ifdef MRDUMP_DEBUG
    MD_LOGI("%s: num_blocks=%d, my_num=%u\n", __func__, num_blocks, my_num);
#endif
    offset = loop + 1;
    for(i=0; i<loop; i++) {
        offset = construct_a_block(fds, blksize, i, offset, myinfo, my_num);
        if (offset == -1) {
            MD_LOGE("%s: marker construct block error!(%d), %s\n", __func__, errno, strerror(errno));
            mrdump_close(fds);
            free(myinfo);
            return false;
        }
    }
    mrdump_close(fds);
    free(myinfo);
    return true;
}

static bool ext4_fallocate(const char *allocfile, uint64_t allocsize)
{
    int fd;

    if((allocfile == NULL) || (allocsize == 0)) {
        MD_LOGE("%s: allocfile is NULL or allocsize = %" PRIu64 "\n", __func__, allocsize);
        return false;
    }

    while(!mrdump_file_is_exist(AE_DUMPSYS_DATA_PATH)) {
        MD_LOGI("%s: wait until %s ready.\n", __func__, AE_DUMPSYS_DATA_PATH);
        sleep(1);
    }

    if(mount_as_f2fs(MRDUMP_EXT4_MOUNT_POINT)) {

        allocsize = ((allocsize + F2FS_MAPSIZE - 1) / F2FS_MAPSIZE) * F2FS_MAPSIZE;
        if(!f2fs_fallocate(allocfile, allocsize)) {
            MD_LOGE("%s: f2fs_fallocate failed (allocfile=%s, allocsize=%" PRIu64 ")\n", __func__, allocfile, allocsize);
            return false;
        }

        fd = open(allocfile, O_RDONLY);
        if(0 > fd) {
            MD_LOGE("%s: open fd failed.\n", __func__);
            return false;
        }

        unsigned int val = 1234;
        if(0 > ioctl(fd, F2FS_IOC_GET_PIN_FILE , &val)) {
            MD_LOGE("%s: F2FS_IOC_GET_PIN_FILE(%u) failed(%d), %s\n", __func__, val, errno, strerror(errno));
        }
        MD_LOGI("%s: file(%s:%" PRIu64 ") Skip GC (%u times)\n", __func__, allocfile, allocsize, val);

        mrdump_close(fd);

    } else {

        fd = open(allocfile, O_RDWR | O_CREAT, 0400);
        if(0 > fd) {
            MD_LOGE("%s: open allocfile failed(%d), %s\n", __func__, errno, strerror(errno));
            return false;
        }

#if (!__LP64__)
        uint32_t low = allocsize & 0xffffffff;
        uint32_t high = allocsize >> 32;

        if(0 > syscall(__NR_fallocate, fd, 0, 0, 0, low, high)) {
            MD_LOGE("%s: fallocate32 failed(allocfile=%s, allocsize=%" PRIu64 ")\n", __func__, allocfile, allocsize);
            mrdump_close(fd);
            unlink(allocfile);
            return false;
        }
#else
        if(0 > syscall(__NR_fallocate, fd, 0, 0, allocsize)) {
            MD_LOGE("%s: fallocate64 failed(allocfile=%s, allocsize=%" PRIu64 ")\n", __func__, allocfile, allocsize);
            mrdump_close(fd);
            unlink(allocfile);
            return false;
        }
#endif
        mrdump_close(fd);
    }

    return true;
}

static bool palloc_file_remove(const char *allocfile)
{
    /* Ignore attribute setting failed */
    fop_file_write_string(MRDUMP_EXT4_PARA_LBAOOO, "0");

    if (mrdump_file_is_exist(allocfile)) {
        ext4_setup_attr(allocfile, false);
        if (unlink(allocfile) != 0) {
            MD_LOGE("%s: Cannot unlink %s(%d), %s\n", __func__, allocfile, errno, strerror(errno));
            return false;
        }
    }
    return true;
}

static bool mrdump_read_pafile_info(const struct palloc_file *pfile, int device_fd,
                                    struct mrdump_pafile_info *info)
{
    memset(info, 0, sizeof(struct mrdump_pafile_info));

    uint8_t block0[MRDUMP_PAF_TOTAL_SIZE];
    memset(block0, 0, sizeof(block0));

    if (-1 == ext4_block_lseek(device_fd, pfile->lbaooo, pfile->blksize)) {
        MD_LOGE("%s: lseek64 InfoLBA failed(%d), %s\n", __func__, errno, strerror(errno));
        return false;
    }

    ssize_t readsize = read(device_fd, block0, sizeof(block0));
    if((readsize < 0) || (readsize != sizeof(block0))) {
        MD_LOGE("%s: readsize=%zd, expected_size=%zu (%d), %s\n", __func__, readsize, sizeof(block0),
                errno, strerror(errno));
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

    uint16_t version = *(uint16_t *)&block0[0];
    if (version != MRDUMP_PAF_VERSION) {
        MD_LOGE("%s: LBA version mismatch (c:%d, v:%d)\n", __func__, MRDUMP_PAF_VERSION, version);
        mrdump_dump_pafile_info(block0);
        return false;
    }
    info->info_lba = *(uint32_t *)(bufp + MRDUMP_PAF_INFO_LBA);
    info->addr_lba = *(uint32_t *)(bufp + MRDUMP_PAF_ADDR_LBA);
    info->filesize = *(uint64_t *)(bufp + MRDUMP_PAF_ALLOCSIZE);
    info->coredump_size = *(uint64_t *)(bufp + MRDUMP_PAF_COREDUMPSIZE);
    info->timestamp = *(uint64_t *)(bufp + MRDUMP_PAF_TIMESTAMP);
    return true;
}

static bool mark_data_block(const struct palloc_file *pfile)
{
    int             fds, fdd; /* fd of src and dst */
    int             i, j, blksize, num_blocks, loop;
    struct stat     statinfo;
    unsigned int    lba, BlockLBA[1024] = {0};

    // Get file statinfo
    if(0 > fstat(pfile->fd, &statinfo)) {
        MD_LOGE("%s: marker stat failed(%d), %s\n", __func__, errno, strerror(errno));
        return false;
    }
    // Get Blocksize and counting blocks
    blksize = pfile->blksize;
    num_blocks = (statinfo.st_size + blksize - 1) / blksize;

    /////////////////////////////////////////////////////
    // begin to record (2-layer block writing)
    char *devicenode = mrdump_get_device_node(MRDUMP_EXT4_MOUNT_POINT);
    if(devicenode == NULL) {
        MD_LOGE("%s: get devicenode failed(%d), %s\n", __func__, errno, strerror(errno));
        return false;
    } else {
        fds = open(devicenode, O_RDONLY);
        if(0 > fds) {
            free(devicenode);
            MD_LOGE("%s: open devicenode failed(%d), %s\n", __func__, errno, strerror(errno));
            return false;
        }
        fdd = open(devicenode, O_WRONLY);
        if(0 > fdd) {
            mrdump_close(fds);
            free(devicenode);
            MD_LOGE("%s: open devicenode failed(%d), %s\n", __func__, errno, strerror(errno));
            return false;
        }
    }
    free(devicenode);

    // Check each Block (Now: InfoLBA + Address Block)
    // 1. InfoLBA blocks
    struct mrdump_pafile_info info;
    if (!mrdump_read_pafile_info(pfile, fds, &info)) {
        mrdump_close(fds);
        mrdump_close(fdd);
        return false;
    }

    // go for Address Area
    bool stop_here = false;
    lba = (unsigned int)info.addr_lba;
    bdata.timestamp = info.timestamp;
    loop = (blksize / sizeof(unsigned int)) - 2;    // 1022
    loop = (num_blocks + loop - 1) / loop;          // num of blocks which Address Area really occupied

    for(i=0; i<loop; i++) {
#ifdef MRDUMMP_DEBUG
        MD_LOGE("%s: i(%03d) loop(%03d)\n", __func__, i, loop);
#endif
        if(-1 == ext4_block_lseek(fds, lba, blksize)) {
            MD_LOGE("%s: bdata lseek64 BlockLBA failed(%d), %s\n", __func__, errno, strerror(errno));
            mrdump_close(fds);
            mrdump_close(fdd);
            return false;
        }
        ssize_t readsize = read(fds, (void *)BlockLBA, blksize);
        if((readsize < 0) || (readsize != blksize)) {
            MD_LOGE("%s: readsize=%zd, expected_size=%d (%d), %s\n", __func__, readsize, blksize,
                    errno, strerror(errno));
            mrdump_close(fds);
            mrdump_close(fdd);
            return false;
        }

        for(j=0; j<MRDUMP_EXT4_LBA_PER_BLOCK; j++) {
            lba = BlockLBA[j];

            if(lba == 0)
                stop_here = true;

            if(format_a_data_block(fdd, blksize, lba) < 0) {
                MD_LOGE("%s: format_data_block error! j=%d (%d), %s\n", __func__, j, errno, strerror(errno));
                mrdump_close(fds);
                mrdump_close(fdd);
                return false;
            }
        }
        lba = BlockLBA[j];

        if(stop_here)
            break;
    }
    mrdump_close(fds);
    mrdump_close(fdd);
    return true;
}

static bool fs_lba_maker(const struct palloc_file *pfile, bool need_to_reinit)
{
#ifdef MRDUMP_DEBUG
    MD_LOGI("%s start...\n", __func__);
#endif
    if (need_to_reinit) {
        lba_marker_time = (uint64_t)time(NULL);
        MD_LOGI("%s: lba_marker_time(%" PRIx64 ")\n", __func__, lba_marker_time);

        if (!fs_lba_mark_header(pfile)) {
            MD_LOGE("%s: mark header failed(%d), %s\n", __func__, errno, strerror(errno));
            return false;
        }

        if (!fs_lba_mark_body(pfile)) {
            MD_LOGE("%s: mark body failed(%d), %s\n", __func__, errno, strerror(errno));
            return false;
        }
    }

    if (MRDUMP_DATA_FS_F2FS == mrdump_get_data_os()) {
        if(!mark_data_block(pfile)) {
            MD_LOGE("%s: mark data block failed(%d), %s\n", __func__, errno, strerror(errno));
            return false;
        }
    }

    sync();

    /* drop buffer cache */
    fop_file_write_string("/proc/sys/vm/drop_caches", "1");

#ifdef MRDUMP_DEBUG
    MD_LOGI("%s end...\n", __func__);
#endif
    return true;
}

static struct palloc_file *ext4_new_fallocfile(const char *allocfile, uint64_t allocsize)
{
    uint64_t psize = mrdump_get_partition_free_size(MRDUMP_EXT4_MOUNT_POINT);
    if (psize < (allocsize + MRDUMP_REST_SPACE)) {
        MD_LOGE("Error: Partition %s has no enough free space(%" PRIu64 "MB), allocate size %" PRIu64 "MB\n",
                MRDUMP_EXT4_MOUNT_POINT, psize / (1024 * 1024), allocsize / (1024 * 1024));
        return NULL;
    }

    if (!ext4_fallocate(allocfile, allocsize)) {
        MD_LOGE("%s: new fallocate failed(%d), %s\n", __func__, errno, strerror(errno));
        return NULL;
    }

    if (!ext4_setup_attr(allocfile, true)) {
        MD_LOGE("%s: ext4_setup_attr failed(%d), %s\n", __func__, errno, strerror(errno));
        return NULL;
    }

    struct palloc_file *pfile = palloc_file_open(allocfile);
    if (pfile == NULL) {
        MD_LOGE("%s: palloc_file_open failed(%d), %s\n", __func__, errno, strerror(errno));
        return NULL;
    }

    if (!fs_lba_maker(pfile, true)) {
        palloc_file_close(pfile);
        MD_LOGE("%s: lba marker failed(%d), %s\n", __func__, errno, strerror(errno));
        return NULL;
    }

    // sync when finish lba_marker in new_fallocate.
    sync();

    return pfile;
}

static int ext4_bdata_is_ok(const struct palloc_file *pfile)
{
    int fds, num_blocks, j;
    struct stat statinfo;
    unsigned int mycrc, BlockLBA[1024];
    unsigned int i;
    bool stop_here = false;
    ssize_t readsize;

    // Get file statinfo
    if(0 > fstat(pfile->fd, &statinfo)) {
        MD_LOGE("%s: bdata stat failed(%d), %s\n", __func__, errno, strerror(errno));
        return BDATA_STATE_FILE_ACCESS_ERROR;
    }

    // Counting blocks
    num_blocks = (statinfo.st_size + statinfo.st_blksize - 1) / statinfo.st_blksize;
    num_blocks = (num_blocks + MRDUMP_EXT4_LBA_PER_BLOCK - 1) / MRDUMP_EXT4_LBA_PER_BLOCK;

    // open
    char *devicenode = mrdump_get_device_node(MRDUMP_EXT4_MOUNT_POINT);
    if(devicenode == NULL) {
        MD_LOGE("%s: get devicenode failed(%d), %s\n", __func__, errno, strerror(errno));
        return BDATA_STATE_FILE_ACCESS_ERROR;
    } else {
        fds = open(devicenode, O_RDWR);
        if(0 > fds) {
            free(devicenode);
            MD_LOGE("%s: open devicenode failed(%d), %s\n", __func__, errno, strerror(errno));
            return BDATA_STATE_FILE_ACCESS_ERROR;
        }
    }
    free(devicenode);

    // Check each Block (Now: InfoLBA + Address Block)
    // 1. InfoLBA blocks
    struct mrdump_pafile_info info;
    if (!mrdump_read_pafile_info(pfile, fds, &info)) {
        mrdump_close(fds);
        return BDATA_STATE_BLOCK_HEADER_ERROR;
    }

    //get timestamp
    bdata.timestamp = info.timestamp;

    // for performance, we must finish fiemap here... but not in the loop
    unsigned int my_num = mrdump_fiemap_total_entries(pfile->fd);
    if(my_num == 0) {
        MD_LOGE("%s: mrdump_fiemap_total_entries error!(%d), %s\n", __func__, errno, strerror(errno));
        mrdump_close(fds);
        return BDATA_STATE_BLOCK_HEADER_ERROR;
    }
    struct fiemap_info *myinfo = malloc(my_num * sizeof(struct fiemap_info));
    if(!mrdump_fiemap_get_entries(pfile->fd, pfile->blksize, myinfo, my_num)) {
        MD_LOGE("%s: mrdump_fiemap_get_entries error!(%d), %s\n", __func__, errno, strerror(errno));
        mrdump_close(fds);
        return BDATA_STATE_BLOCK_HEADER_ERROR;
    }

    // 2. Address Block sector wait until patch in lk

    if (mount_as_ext4(MRDUMP_EXT4_MOUNT_POINT) && info.coredump_size == ULLONG_MAX) {
        MD_LOGE("coredump size is incorrect\n");
        return BDATA_STATE_BLOCK_DATA_ERROR;
    }
    for (i=1; i<(unsigned int)num_blocks; i++) {
        unsigned int lba = (unsigned int)mrdump_fiemap_get_lba_of_block(myinfo, my_num, i);
        if(-1 == ext4_block_lseek(fds, lba, pfile->blksize)) {
            MD_LOGE("%s: bdata lseek64 BlockLBA failed(%d), %s\n", __func__, errno, strerror(errno));
            free(myinfo);
            mrdump_close(fds);
            return BDATA_STATE_BLOCK_HEADER_ERROR;
        }
        readsize = read(fds, (void *)BlockLBA, pfile->blksize);
        if((readsize < 0) || (readsize != pfile->blksize)) {
            MD_LOGE("%s: readsize=%zd, expected_size=%d (%d), %s\n", __func__, readsize, pfile->blksize,
                    errno, strerror(errno));
            free(myinfo);
            mrdump_close(fds);
            return BDATA_STATE_BLOCK_HEADER_ERROR;
        }
        mycrc = crc32(0, Z_NULL, 0);
        mycrc = crc32(mycrc, (void *)BlockLBA, (pfile->blksize-4));
        if (mycrc != BlockLBA[1023]) {
            MD_LOGE("%s: bdata BlockLBA %d CRC error(%08x, %08x)\n", __func__, i, mycrc, BlockLBA[1023]);
            free(myinfo);
            mrdump_close(fds);
            return BDATA_STATE_BLOCK_HEADER_ERROR;
        }
    }
    // data block vaildation for F2FS
    if (mount_as_f2fs(MRDUMP_EXT4_MOUNT_POINT) && info.coredump_size == ULLONG_MAX) {
        for(i=1; i<(unsigned int)num_blocks; i++) {
            unsigned int lba = (unsigned int)mrdump_fiemap_get_lba_of_block(myinfo, my_num, i);
            if(-1 == ext4_block_lseek(fds, lba, pfile->blksize)) {
                MD_LOGE("%s: bdata lseek64 BlockLBA failed(%d), %s\n", __func__, errno, strerror(errno));
                mrdump_close(fds);
                return BDATA_STATE_BLOCK_DATA_ERROR;
            }
            readsize = read(fds, (void *)BlockLBA, pfile->blksize);
            if((readsize < 0) || (readsize != pfile->blksize)) {
                MD_LOGE("%s: readsize=%zd, expected_size=%d (%d), %s\n", __func__, readsize, pfile->blksize,
                        errno, strerror(errno));
                mrdump_close(fds);
                return BDATA_STATE_BLOCK_DATA_ERROR;
            }

            for(j=0; j<MRDUMP_EXT4_LBA_PER_BLOCK; j++) {
                lba = BlockLBA[j];

                if(lba == 0)
                    stop_here = true;

                if(check_a_data_block(fds, pfile->blksize, lba) < 0) {
                    MD_LOGE("%s: format_data_block error! j=%d \n", __func__, j);
                    mrdump_close(fds);
                    return BDATA_STATE_BLOCK_DATA_ERROR;
                }
            }
            lba = BlockLBA[j];

            if(stop_here)
                break;
        }
    }

    mrdump_close(fds);
    free(myinfo);
    return BDATA_STATE_CHECK_PASS;
}

////////////////////////////////////////////
// export function (extern of other files)//
////////////////////////////////////////////

bool mrdump_file_get_info(const char *allocfile, struct mrdump_pafile_info *info)
{
    struct palloc_file *pfile = palloc_file_open(allocfile);
    if (pfile == NULL) {
        return false;
    }
    int device_fd = ext4_open_device_node(MRDUMP_EXT4_MOUNT_POINT);
    if (device_fd < 0) {
        palloc_file_close(pfile);
        return false;
    }
    bool retval = mrdump_read_pafile_info(pfile, device_fd, info);
    palloc_file_close(pfile);
    mrdump_close(device_fd);
    return retval;
}

static uint64_t ext4_default_filesize(int memsize)
{
    int ret = 0;
    FILE *fp;
    char c, myline[1024];
    char *delim="\x09\x20";
    uint64_t MySize;

    fp = fopen("/proc/meminfo", "r");
    if(fp == NULL)
        return ret;

    while(!feof(fp)) {

        // getline
        ret = fscanf(fp, "%[^\n]", myline);

        // strtok strings
        MySize = 0;
        if(ret > 0) {
            char *mystr = strtok(myline, delim);
            if(mystr != NULL) {
                if(!strcmp(mystr, "MemTotal:")) {
                    mystr = strtok(NULL, delim);
                    if(mystr != NULL) {
                        MySize = atol(mystr);
                    }
                    break;
                }
            }
        }

        /* clear newline character */
        ret = fscanf(fp, "%c", &c);
        if (ret != 1) {
            MD_LOGE("%s: not EOL.\n", __func__);
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);

    // Count proper MySize about half size of MemTotal;
    ret = ((MySize*1000) + (1000*1000) - 1)/(1000000);
    if(memsize == DEFAULT_HALFMEM)
        ret/= 2;
    MySize = (uint64_t)ret*1024*1024;

    return (MySize);
}

uint64_t mrdump_file_default_filesize(void)
{
    uint64_t fsize = 0;
    int  f_mem = USERDEFINED_MEM;
    const char *mrdump_allocate_size = sysenv_get("mrdump_allocate_size");

    if ((mrdump_allocate_size == NULL) || !strncmp(mrdump_allocate_size, "fullmem", 7)) {
        fsize = ext4_default_filesize(DEFAULT_FULLMEM);
        f_mem = DEFAULT_FULLMEM;
    }
    else if ((!strncmp(mrdump_allocate_size, "0", 1))) {
        fsize = 0;
        f_mem = DEFAULT_DISABLE;
    }
    else if (!strncmp(mrdump_allocate_size, "halfmem", 7)) {
        fsize = ext4_default_filesize(DEFAULT_HALFMEM);
        f_mem = DEFAULT_HALFMEM;
    }
    // nnn or others
    else if(f_mem == USERDEFINED_MEM) {
        int value = atoi(mrdump_allocate_size);
        if(value > 0) {
            fsize = (uint64_t)value*1024*1024;
        }
        else {
            fsize = 0;
        }
    }

    return fsize;
}

////////////////////////////////////////////
// User API                                  //
////////////////////////////////////////////
static bool mrdump_ext4_reinit_allocfile(const char *allocfile, uint64_t realsize, bool reset)
{
    int check_state = 0;

    /* pre-allocate only in internal-storage:ext4 */
    const char *output_dev = sysenv_get("mrdump_output");
    MD_LOGI("%s: output-dev(%s)\n", __func__, output_dev);
    if  ((output_dev != NULL) && (strcmp(output_dev, "internal-storage") != 0)) {
        palloc_file_remove(allocfile);
        return false;
    }

    if (realsize > 0) {
        struct palloc_file *pfile = NULL;
        if (!mrdump_file_is_exist(allocfile)) {
            pfile = ext4_new_fallocfile(allocfile, realsize);
            if (pfile == NULL) {
                MD_LOGE("%s: fallocate failed at new creation.\n", __func__);
                return false;
            }
        } else {
            pfile = palloc_file_open(allocfile);
            if (pfile == NULL) {
                MD_LOGE("%s: palloc_file_open failed.\n", __func__);
                goto cleanup;
            }

            // existed allocate
            // Check Block Data validity
            check_state = ext4_bdata_is_ok(pfile);
            if ((check_state == BDATA_STATE_FILE_ACCESS_ERROR) || (check_state == BDATA_STATE_BLOCK_HEADER_ERROR)) {
                MD_LOGI("%s: Address Blocks checked: incorrect.\n", __func__);
                palloc_file_close(pfile);
                palloc_file_remove(allocfile);
                pfile = ext4_new_fallocfile(allocfile, realsize);
                if (pfile == NULL) {
                    MD_LOGE("%s: Address Blocks not correct, re-fallocate.\n", __func__);
                    return false;
                }
            } else if (check_state == BDATA_STATE_BLOCK_DATA_ERROR) {
                MD_LOGE("BlockData Verification failed\n");
                if (!fs_lba_maker(pfile, true)) {
                    MD_LOGE("%s: lba marker failed, removing pre-allocate-file.\n", __func__);
                    palloc_file_close(pfile);
                    goto cleanup;
                }
            } else {
                MD_LOGI("%s: Address Blocks checked: correct.\n", __func__);
                if (reset) {
                    if (!fs_lba_maker(pfile, true)) {
                        MD_LOGE("%s: lba marker failed, removing pre-allocate-file.\n", __func__);
                        palloc_file_close(pfile);
                        goto cleanup;
                    }
                }
            }
        }

        int retval = fop_file_write_string(MRDUMP_EXT4_PARA_LBAOOO, "%d\n", pfile->lbaooo);
        if (retval < 0) {
            MD_LOGE("%s: write %s failed(%d), %s\n", __func__, MRDUMP_EXT4_PARA_LBAOOO,
                    -retval, strerror(-retval));
            palloc_file_close(pfile);
            goto cleanup;
        }
        MD_LOGI("%s: LBAOOO(%d) ready\n", __func__, pfile->lbaooo);
        palloc_file_close(pfile);
        return true;
    }
    MD_LOGI("Allocate size %" PRIx64 " set to zero, remove preallocate file\n",
            realsize);
  cleanup:
    palloc_file_remove(allocfile);
    return false;
}

bool mrdump_file_fetch_zip_coredump(const char *outfile)
{
    struct mrdump_pafile_info lbainfo;
    unsigned int       blknum;
    unsigned int       rlba, bidx, BlockLBA[1024];
    unsigned int       len, mylen;
    unsigned char      MyData[MRDUMP_EXT4_EXSPACE];
    int       fpRead, fpWrite, ret;
    ssize_t readsize;

    // outfile
    if(outfile == NULL) {
        MD_LOGE("%s: outfile is NULL! (%d), %s\n", __func__, errno, strerror(errno));
        return false;
    }

    struct palloc_file *pfile = palloc_file_open(MRDUMP_EXT4_ALLOCATE_FILE);
    if (pfile == 0) {
        return false;
    }

    fpRead = ext4_open_device_node(MRDUMP_EXT4_MOUNT_POINT);
    if (fpRead < -1) {
        goto cleanup0;
    }

    if (!mrdump_read_pafile_info(pfile, fpRead, &lbainfo)) {
        mrdump_close(fpRead);
        goto cleanup0;
    }
    if (lbainfo.coredump_size == 0) {
        MD_LOGI("Ramdump size is 0, no data to dump\n");
        mrdump_close(fpRead);
        goto cleanup0;
    }

    // Write handle
    fpWrite = open(outfile, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if(0 > fpWrite) {
        MD_LOGE("%s: open Write handle open failed(%d), %s\n", __func__, errno, strerror(errno));
        mrdump_close(fpRead);
        goto cleanup0;
    }

    // Init BlockLBA
    rlba = lbainfo.addr_lba;
    if(-1 == ext4_block_lseek(fpRead, rlba, pfile->blksize)) {
        MD_LOGE("%s: lseek64 Init BlockLBA failed(%d), %s\n", __func__, errno, strerror(errno));
        mrdump_close(fpRead);
        mrdump_close(fpWrite);
        unlink(outfile);
        goto cleanup0;
    }
    readsize = read(fpRead, BlockLBA, sizeof(BlockLBA));
    if((readsize < 0) || (readsize != sizeof(BlockLBA))) {
        MD_LOGE("%s: readsize=%zd, expected_size=%zu (%d), %s\n", __func__, readsize, sizeof(BlockLBA),
                errno, strerror(errno));
        mrdump_close(fpRead);
        mrdump_close(fpWrite);
        unlink(outfile);
        goto cleanup0;
    }

    // Fetching data
    bidx    = 0;
    rlba    =  BlockLBA[bidx];

    uint64_t delaylen = 0;
    uint64_t coresize = lbainfo.coredump_size;
    while(coresize > 0) {

        // counting coutinue datas...
        blknum = ext4_num_to_join((unsigned int *)BlockLBA, bidx);
        len = blknum * pfile->blksize;
        if(coresize < len) {
            mylen = coresize;
        } else {
            mylen = len;
        }

        // Reading data to MyData
        if(-1 == ext4_block_lseek(fpRead, rlba, pfile->blksize)) {
            MD_LOGE("%s: lseek64 Read MyData failed(%d), %s\n", __func__, errno, strerror(errno));
            mrdump_close(fpRead);
            mrdump_close(fpWrite);
            unlink(outfile);
            goto cleanup0;
        }
        readsize = read(fpRead, MyData, mylen);
        if((readsize < 0) || (readsize != mylen)) {
            MD_LOGE("%s: readsize=%zd, expected_size=%d (%d), %s\n", __func__, readsize, mylen,
                    errno, strerror(errno));
            mrdump_close(fpRead);
            mrdump_close(fpWrite);
            unlink(outfile);
            goto cleanup0;
        }
        if(0 > write(fpWrite, MyData, mylen)) {
            MD_LOGE("%s: fetch MyData error!(%d), %s\n", __func__, errno, strerror(errno));
            mrdump_close(fpRead);
            mrdump_close(fpWrite);
            unlink(outfile);
            goto cleanup0;
        }

        ret = ext4_get_next_bidx(fpRead, (unsigned int *)BlockLBA, bidx, pfile->blksize, blknum);
        if(ret < 0){
            MD_LOGE("%s: fpRead failed to get next block idx(%d), %s\n", __func__, errno, strerror(errno));
            mrdump_close(fpRead);
            mrdump_close(fpWrite);
            unlink(outfile);
            goto cleanup0;
        }
        bidx = (unsigned int)ret;

        if(bidx > MRDUMP_EXT4_LBA_PER_BLOCK) {
            mrdump_close(fpRead);
            mrdump_close(fpWrite);
            unlink(outfile);
            goto cleanup0;
        }
        if(bidx == MRDUMP_EXT4_LBA_PER_BLOCK) {
            ret = ext4_get_next_bidx(fpRead, (unsigned int *)BlockLBA, bidx, pfile->blksize, blknum);
            if(ret < 0) {
                MD_LOGE("%s: bidx(1022) failed to get next block idx(%d), %s\n", __func__, errno, strerror(errno));
                mrdump_close(fpRead);
                mrdump_close(fpWrite);
                unlink(outfile);
                goto cleanup0;
            }
            bidx = (unsigned int)ret;
        }
        rlba = BlockLBA[bidx];
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
    palloc_file_close(pfile);
    return false;
}

void mrdump_file_set_maxsize(int mrdump_size)
{
    palloc_file_remove(MRDUMP_EXT4_ALLOCATE_FILE);

    char s[64];
    uint64_t fsize = 0;

    if(mrdump_size < 0)
        mrdump_size = 0;

    switch(mrdump_size) {
    case DEFAULT_DISABLE:
        sysenv_set("mrdump_allocate_size", "0");
        fsize = 0;
        break;
    case DEFAULT_HALFMEM:
        sysenv_set("mrdump_allocate_size", "halfmem");
        fsize = mrdump_file_default_filesize();
        break;
    case DEFAULT_FULLMEM:
        sysenv_set("mrdump_allocate_size", "fullmem");
        fsize = mrdump_file_default_filesize();
        break;
    default:
        snprintf(s, sizeof(s), "%d", mrdump_size);
        sysenv_set("mrdump_allocate_size", s);
        fsize = (uint64_t)mrdump_size*1024*1024;
        break;
    }

    if (mrdump_ext4_reinit_allocfile(MRDUMP_EXT4_ALLOCATE_FILE, fsize, true)) {
        printf("set MT-RAMDUMP allocated file size => %" PRIu64 " MB].\n", (fsize/1024/1024));
    }
}

/* System startup setup
 * Sanity check
 * 1. Check if /data partition filesystem is supported
 * 2. Check if lk/kernel MT-RAMDUMP support is enabled
 * 3. Check if mrdump file size is set to non-zero
 * If above condition is failed, remove pre-allocated file and disable MRDUMP.
 * Try to pre-allocated file
 * 1. If file doesn't exist, create a new file
 * 2. If file exist and header/body corrupted, re-create a new file
 */
void mrdump_file_setup(bool reset)
{
    /* check if mount as ext4 partition or f2fs */
    if(MRDUMP_DATA_FS_NONE == mrdump_get_data_os()) {
        MD_LOGE("Unsupport file system type.\n");
        return;
    }

    if (!mrdump_is_supported()) {
        MD_LOGE("MRDUMP is not supported\n");
        palloc_file_remove(MRDUMP_EXT4_ALLOCATE_FILE);
        return;
    }

    mrdump_ext4_reinit_allocfile(MRDUMP_EXT4_ALLOCATE_FILE, mrdump_file_default_filesize(), reset);
}
