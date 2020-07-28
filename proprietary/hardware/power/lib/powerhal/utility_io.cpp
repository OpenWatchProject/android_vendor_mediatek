/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "libPowerHal"

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <utils/Log.h>
#include <errno.h>
#include <sys/stat.h>

#include "common.h"
#include "utility_io.h"

#define PATH_BLKDEV_UFS_USER      "/sys/block/sdc/queue/read_ahead_kb"
#define PATH_BLKDEV_DM_USER       "/sys/block/dm-2/queue/read_ahead_kb"
#define PATH_BLKDEV_EMMC_USER     "/sys/block/mmcblk0/queue/read_ahead_kb"

static int blkdev_init = 0;
static int blkdev_ufsSupport = 0;
static int blkdev_dmSupport = 0;
static int blkdev_emmcSupport = 0;
static int blkdev_ufsDefault = 0;
static int blkdev_dmDefault = 0;
static int blkdev_emmDefault = 0;

/* static function */
static void check_blkDevSupport(void)
{
    struct stat stat_buf;

    if (0 == stat(PATH_BLKDEV_UFS_USER, &stat_buf)) {
        blkdev_ufsSupport = 1;
        blkdev_ufsDefault = get_int_value(PATH_BLKDEV_UFS_USER);
    }
    if (0 == stat(PATH_BLKDEV_DM_USER, &stat_buf)) {
        blkdev_dmSupport = 1;
        blkdev_dmDefault = get_int_value(PATH_BLKDEV_DM_USER);
    }
    if (0 == stat(PATH_BLKDEV_EMMC_USER, &stat_buf)) {
        blkdev_emmcSupport = 1;
        blkdev_emmDefault = get_int_value(PATH_BLKDEV_EMMC_USER);
    }
    ALOGI("check_blkDevSupport: %d, %d, %d", blkdev_ufsDefault, blkdev_dmDefault, blkdev_emmDefault);
}

int setBlkDev_readAhead(int value, void *scn)
{
    ALOGV("setBlkDev_readAhead: %p", scn);
    if (!blkdev_init) {
        check_blkDevSupport();
        blkdev_init = 1;
    }

    if (value != -1) {
        if (blkdev_ufsSupport)
            set_value(PATH_BLKDEV_UFS_USER, value);
        if (blkdev_dmSupport)
            set_value(PATH_BLKDEV_DM_USER, value);
        if (blkdev_emmcSupport)
            set_value(PATH_BLKDEV_EMMC_USER, value);
    } else {
        if (blkdev_ufsSupport)
            set_value(PATH_BLKDEV_UFS_USER, blkdev_ufsDefault);
        if (blkdev_dmSupport)
            set_value(PATH_BLKDEV_DM_USER, blkdev_dmDefault);
        if (blkdev_emmcSupport)
            set_value(PATH_BLKDEV_EMMC_USER, blkdev_emmDefault);
    }
    ALOGI("setBlkDev_readAhead: value:%d", value);
    return 0;
}


