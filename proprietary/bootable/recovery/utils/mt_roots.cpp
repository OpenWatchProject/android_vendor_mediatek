/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
#include <unistd.h>
#include <stdlib.h>
#include <fs_mgr.h>
#include <errno.h>
#include <string.h>
#include <sys/mount.h>
#include "otautil/roots.h"
#include "mt_roots.h"
#include "common.h"
#include "mt_partition.h"

using android::fs_mgr::Fstab;
using android::fs_mgr::FstabEntry;
using android::fs_mgr::ReadDefaultFstab;

static Fstab mt_fstab;

void mt_load_volume_table(Fstab* fstab)
{
    if (mt_get_phone_type() == FS_TYPE_EMMC) {
#if defined(MTK_GMO_ROM_OPTIMIZE)
        fstab->emplace_back(FstabEntry{
             .mount_point = "/sdcard", .fs_type = "vfat", .blk_device = "/dev/block/mmcblk1p1", .length = 0 });

        fstab->emplace_back(FstabEntry{
             .mount_point = "/sdcard_dev2", .fs_type = "vfat", .blk_device = "/dev/block/mmcblk1", .length = 0 });

#else

        fstab->emplace_back(FstabEntry{
             .mount_point = "/sdcard", .fs_type = "vfat", .blk_device = "/dev/block/mmcblk1p1", .length = 0 });

#endif
    } else { // for non emmc platform
        fstab->emplace_back(FstabEntry{
             .mount_point = "/sdcard", .fs_type = "vfat", .blk_device = "/dev/block/mmcblk0p1", .length = 0 });

        fstab->emplace_back(FstabEntry{
             .mount_point = "/sdcard_dev2", .fs_type = "vfat", .blk_device = "/dev/block/mmcblk0", .length = 0 });
    }

     mt_ensure_dev_ready("/misc");
     mt_ensure_dev_ready("/cache");
}

void mt_ensure_dev_ready(const char *mount_point) {
    int count=0;

    if (mt_fstab.empty()) {
        if (!ReadDefaultFstab(&mt_fstab)) {
            printf("Failed to read default fstab in mt_ensure_dev_ready\n");
        }
    }

    Volume* v = GetEntryForMountPoint(&mt_fstab, mount_point);
    if (v) {
        while ((count++ < 5) && (access(v->blk_device.c_str(), R_OK) != 0)) {
            printf("no %s entry %s, count = %d\n", mount_point, v->blk_device.c_str(), count);
            sleep(1);
        }
    } else {
        printf("Error: Retry fail %s partition not found\n",mount_point);
    }
}
