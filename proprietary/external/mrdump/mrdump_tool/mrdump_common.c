/* uint64_t ...*/
#include <inttypes.h>
#include <stdbool.h>

/* struct stat, open */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* fsync(), close() */
#include <unistd.h>

/* strerror */
#include <string.h>
/* errno */
#include <errno.h>

/* statfs() and statvfs() */
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <sys/vfs.h>

/* ioctl, BLKGETSIZE64 */
#include <linux/fs.h>
#include <sys/ioctl.h>

/* Property */
#include <cutils/properties.h>
#include <sys/system_properties.h>

/* mrdump related */
#include "mrdump_log.h"
#include "mrdump_common.h"
#include "mrdump_support_ext4.h"
#include "mrdump_support_f2fs.h"

void mrdump_close(int fd)
{
    fsync(fd);
    close(fd);
}

int mrdump_file_is_exist(const char *path)
{
    struct stat s;
    if (!path)
        return 0;
    if (stat(path, &s) != 0)
        return 0;
    return 1;
}

int mrdump_get_data_os(void)
{
    if (mount_as_ext4(MRDUMP_EXT4_MOUNT_POINT))
        return MRDUMP_DATA_FS_EXT4;

    if (mount_as_f2fs(MRDUMP_EXT4_MOUNT_POINT))
        return MRDUMP_DATA_FS_F2FS;

    MD_LOGI("%s: unknown os\n", __func__);
    return MRDUMP_DATA_FS_NONE;
}

uint64_t mrdump_get_partition_free_size(const char *mountp)
{
    struct statvfs vfs;
    uint64_t psize;

    /* initialization */
    memset((void *)&vfs, 0, sizeof(vfs));

    if(statvfs(mountp, &vfs) == 0) {
        psize = (uint64_t) vfs.f_frsize * (uint64_t) vfs.f_bfree;
        MD_LOGD("%s: size of %s: %" PRId64 " bytes.\n", __func__, mountp, psize);
        return psize;
    }

    MD_LOGE("%s: statvfs of %s got failed(%d), %s\n", __func__, mountp, errno, strerror(errno));
    return 0;
}

uint64_t mrdump_get_partition_size(char *fullpath)
{
    int fd = open(fullpath, O_RDONLY);
    if (0 > fd) {
        MD_LOGE("%s: open fullpath failed. (%s)\n", __func__, fullpath);
        return 0;
    }

    uint64_t psize;
    int ret = ioctl(fd, BLKGETSIZE64, &psize);
    if (0 > ret) {
        MD_LOGE("%s: ioctl BLKGETSIZE64 failed. (%s)\n", __func__, fullpath);
        mrdump_close(fd);
        return 0;
    }

    mrdump_close(fd);

    return psize;
}

static char *mrdump_get_device_node_from_fstab(const char *fstab, const char *mountp)
{
    int ret;
    FILE *fp;
    char c, myline[1024];
    char *delim="\x09\x20";
    char *DeviceNode, *MountPoint;

    DeviceNode = NULL;
    fp = fopen(fstab, "r");
    if(fp == NULL)
        return NULL;

    while(!feof(fp)) {

        // getline
        ret = fscanf(fp, "%[^\n]", myline);

        // strtok strings
        if(ret > 0) {
            if(myline[0] == '/') {
                DeviceNode = strtok(myline, delim);
                if (DeviceNode == NULL)
                    continue;
                MountPoint = strtok(NULL, delim);
                if(MountPoint != NULL) {
                    if(!strcmp(MountPoint, mountp)) {
                        fclose(fp);
                        return strdup(DeviceNode);
                    }
                }
            }
        }

        /* clear newline character */
        ret = fscanf(fp, "%c", &c);
        if (ret != 1) {
            MD_LOGE("%s: not EOL.", __func__);
            fclose(fp);
            return NULL;
        }
    }
    fclose(fp);
    return NULL;
}

static const char *fstab_path_prefix[] = {
    "/vendor/etc/fstab",
    "/fstab",
    NULL
};

char *mrdump_get_device_node(const char *mountp)
{
    int i;
    char *DeviceNode = NULL;
    char fstab_filename[PROPERTY_VALUE_MAX];

    /* get hardware parts */
    char propbuf[PROPERTY_VALUE_MAX];
    if(property_get("ro.hardware", propbuf, NULL) == 0)
        property_get("ro.board.platform", propbuf, "");

    for(i = 0; fstab_path_prefix[i] != NULL; i++) {
        snprintf(fstab_filename, sizeof(fstab_filename), "%s.%s", fstab_path_prefix[i], propbuf);
        if(mrdump_file_is_exist(fstab_filename))
            DeviceNode =  mrdump_get_device_node_from_fstab(fstab_filename, mountp);
        if(DeviceNode)
            return DeviceNode;
    }

    /* search for path: /fstab */
    DeviceNode =  mrdump_get_device_node_from_fstab("/fstab", mountp);
    return DeviceNode;
}
