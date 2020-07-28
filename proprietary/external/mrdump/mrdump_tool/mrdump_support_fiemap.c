/* uint64_t ...*/
#include <inttypes.h>
#include <stdbool.h>

/* malloc */
#include <stdlib.h>

/* ioctl, fibmap, fiemap */
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <linux/fiemap.h>

/* strerror */
#include <string.h>
/* errno */
#include <errno.h>

/* mrdump related */
#include "mrdump_log.h"
#include "mrdump_common.h"
#include "mrdump_support_fiemap.h"

static unsigned int fiemap_total_entries(int fd)
{
    int is_last, rows = 0;

    // Preparing for fiemap work
    struct fiemap *fiemap = (struct fiemap *)malloc(sizeof(struct fiemap) + sizeof(struct fiemap_extent));
    if (fiemap == NULL) {
        MD_LOGE("%s: fiemap malloc failed!(%d), %s\n", __func__, errno, strerror(errno));
        return 0;
    }
    unsigned long long lstart = 0;             // logical input mapping star
#if (__LP64__)
    unsigned long long llength = ~0ULL;        // logical input mapping length
#else
    unsigned long long llength = 0xFFFFFFFF;
#endif

    fiemap->fm_start = lstart;
    fiemap->fm_length = llength;
    fiemap->fm_flags = 0;
    fiemap->fm_extent_count = 1;
    fiemap->fm_mapped_extents = 0;   // output only

    do {
        if(0 > ioctl(fd, FS_IOC_FIEMAP, (unsigned long)fiemap)) {
            MD_LOGE("%s: FIEMAP ioctl failed!(%d), %s\n", __func__, errno, strerror(errno));
            free(fiemap);
            return 0;
        }

        if(fiemap->fm_mapped_extents == 0) {
            MD_LOGE("%s: FIEMAP: fm_mapped_extents = 0(%d), %s\n", __func__, errno, strerror(errno));
            free(fiemap);
            return 0;
        }

        // check if the last extent
        is_last = fiemap->fm_extents[0].fe_flags & FIEMAP_EXTENT_LAST;

        // Set up the next call arguments
        if(!is_last) {
            unsigned long long foo = fiemap->fm_extents[0].fe_logical + fiemap->fm_extents[0].fe_length;
            fiemap->fm_start = foo;
            fiemap->fm_length = lstart + llength - foo;
            fiemap->fm_flags = 0;
            fiemap->fm_extent_count = 1;
        }

        rows++;

    } while(!is_last);

    free(fiemap);
    return rows;

}

static bool fiemap_get_entries(int fd, unsigned int blksize, struct fiemap_info *mapinfo, unsigned int rows)
{
    struct fiemap_info *myinfo = mapinfo;

    unsigned long long lstart = 0;             // logical input mapping star
#if (__LP64__)
    unsigned long long llength = ~0ULL;        // logical input mapping length
#else
    unsigned long long llength = 0xFFFFFFFF;
#endif

    // Preparing for fiemap work
    struct fiemap *fiemap = (struct fiemap *)malloc(sizeof(struct fiemap) + sizeof(struct fiemap_extent));
    if (fiemap == NULL) {
        MD_LOGE("%s: malloc failed\n", __func__);
        return false;
    }

    fiemap->fm_start = lstart;
    fiemap->fm_length = llength;
    fiemap->fm_flags = 0;
    fiemap->fm_extent_count = 1;
    fiemap->fm_mapped_extents = 0;   // output only

    unsigned int i, is_last;
    for(i=0; i<rows; i++)
    {
        if(0 > ioctl(fd, FS_IOC_FIEMAP, (unsigned long)fiemap)) {
            printf("%s: FIEMAP ioctl failed!(%d), %s\n", __func__, errno, strerror(errno));
            free(fiemap);
            return false;
        }

        // check if the last extent
        is_last = fiemap->fm_extents[0].fe_flags & FIEMAP_EXTENT_LAST;

        // basic parameters
        myinfo->lba = (unsigned int)(fiemap->fm_extents[0].fe_physical/blksize);
        myinfo->tot = (unsigned int)(fiemap->fm_extents[0].fe_length / blksize);

        // Set up the next call arguments
        if(!is_last) {

            unsigned long long foo = fiemap->fm_extents[0].fe_logical + fiemap->fm_extents[0].fe_length;
            fiemap->fm_start = foo;
            fiemap->fm_length = lstart + llength - foo;
            fiemap->fm_flags = 0;
            fiemap->fm_extent_count = 1;
        }
        myinfo++;
    }
    free(fiemap);
    return true;
}

static int fiemap_get_entry_lba(int fd, unsigned int blksize, unsigned int rows)
{
    unsigned int lba = 0, num = (unsigned int)fiemap_total_entries(fd);
    if(num > 0) {
        struct fiemap_info *myinfo = malloc(num * sizeof(struct fiemap_info));
        if(fiemap_get_entries(fd, blksize, myinfo, num)) {
            lba = myinfo[rows].lba;
            free(myinfo);
            return (int)lba;
        }
        free(myinfo);
    }
    return -1;
}

static int fiemap_get_entry_tot(int fd, unsigned int blksize, unsigned int rows)
{
    unsigned int tot = 0, num = (unsigned int)fiemap_total_entries(fd);
    if(num > 0) {
        struct fiemap_info *myinfo = malloc(num * sizeof(struct fiemap_info));
        if(fiemap_get_entries(fd, blksize, myinfo, num)) {
            tot = myinfo[rows].tot;
            free(myinfo);
            return (int)tot;
        }
        free(myinfo);
    }
    return -1;
}

static unsigned int fiemap_get_lba_of_block(struct fiemap_info *myinfo, unsigned int rows, unsigned int block)
{
    unsigned int i, lba = 0, tot = 0;
    for(i=0; i<rows; i++) {
        tot = myinfo[i].tot;
        if(block < tot) {
            lba = myinfo[i].lba + block;
            break;
        } else {
            block = block - tot;
        }
    }
    return lba;
}

unsigned int mrdump_fiemap_total_entries(int fd)
{
    return fiemap_total_entries(fd);
}

bool mrdump_fiemap_get_entries(int fd, unsigned int blksize, struct fiemap_info *mapinfo, unsigned int rows)
{
    return fiemap_get_entries(fd, blksize, mapinfo, rows);
}

unsigned int mrdump_fiemap_get_lba_of_block(struct fiemap_info *myinfo, unsigned int num, unsigned int block)
{
    return fiemap_get_lba_of_block(myinfo, num, block);
}

int mrdump_fiemap_get_entry_tot(int fd, unsigned int blksize, unsigned int rows)
{
    return fiemap_get_entry_tot(fd, blksize, rows);
}

int mrdump_fiemap_get_entry_lba(int fd, unsigned int blksize, unsigned int rows)
{
    return fiemap_get_entry_lba(fd, blksize, rows);
}
