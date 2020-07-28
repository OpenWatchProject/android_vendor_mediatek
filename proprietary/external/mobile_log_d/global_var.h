/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#ifndef GLOBAL_VAR_H
#define GLOBAL_VAR_H

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "MobileLogD"
#endif

#define LIB_AED "libaed.so"
typedef int (AEE_SYS_FUNC)(const char *module, const char* path, unsigned int flags, const char *msg, ...);

#define CONFIG_DIR          "/data/misc/mblog/"
#define CONFIG_FILE         CONFIG_DIR"mblog_config"
#define MBLOG_HISTORY       CONFIG_DIR"mblog_history"
#define INTER_DIR_FILE      CONFIG_DIR"inter_sd_folder"
#define OUTER_DIR_FILE      CONFIG_DIR"outer_sd_folder"
#define BOOTUP_FILE_TREE    CONFIG_DIR"bootup_file_tree"
#define LOG_DIR_PACKED      "/data/misc/mblog/packlog/"

#define BUILD_TYPE_ENG      "eng"
#define BUILD_TYPE_USER     "user"


#define CUSTOM_DEFAULT      "MTK_Internal"
#define STORAGE_PATH        "set_storage_path"

#define PATH_INTERNAL       "internal_sd"
#define PATH_EXTERNAL       "external_sd"
#define PATH_DEFAULT        "/sdcard/"
#define PATH_EXTSD_PARENT   "/storage/"
#define PATH_DATA           "/data/"

#define PATH_TEMP           "/data/log_temp/"
/*
#define PATH_NORMAL         PATH_TEMP"boot/"
#define PATH_META           PATH_TEMP"meta/"
#define PATH_FACTORY        PATH_TEMP"factory/"
#define PATH_OTHER_BOOT     PATH_TEMP"otherboot/"
*/

#define PATH_SUFFIX         "debuglogger/mobilelog/"

#define PROP_PREPARE         "vendor.MB.prepare"
#define PROP_RUNNING         "vendor.MB.running"
#define PROP_CONTROL         "vendor.MB.control"
#define PROP_PATH            "vendor.MB.realpath"
#define PROP_OLDPATH         "vendor.MB.oldpath"
#define PROP_PACKED          "vendor.MB.packed"
#define PROP_VERSION         "vendor.MB.version"
#define PROP_SUBLOG          "vendor.MB.sublog"
#define PROP_SUBLOG_CONFIG   "vendor.MB.sublog_config"


#define QUOTA_RATIO         10
#define REMAIN_SIZE         (70 * 1024 * 1024ULL)  // unit:MB
#define STORAGE_FULL        101
#define DELAY_TIME          30


#ifndef TEMP_FAILURE_RETRY
/* Used to retry syscalls that can return EINTR. */
#define TEMP_FAILURE_RETRY(exp)({                 \
    typeof(exp) _rc;                             \
    do {                                          \
        _rc = (exp);                              \
    } while (_rc == -1 && errno == EINTR);        \
    _rc;})
#endif

typedef enum {
    SAVE_TO_BOOTUP = 0,
    SAVE_TO_SDCARD,
    STOPPING,

    STOPPED = 999,
} MBLOGSTATUS;

extern MBLOGSTATUS g_mblog_status;
extern char last_logging_path[256];
extern char cur_logging_path[256];

// log current path change flag
extern int g_redirect_flag;
extern int g_copy_wait;

typedef enum mobilog_id {
    MOBILOG_ID_MIN = 0,

    MOBILOG_ID_MAIN = 0,
    MOBILOG_ID_RADIO = 1,
    MOBILOG_ID_EVENTS = 2,
    MOBILOG_ID_SYSTEM = 3,
    MOBILOG_ID_CRASH = 4,
    MOBILOG_ID_STATS = 5,
    MOBILOG_ID_SECURITY = 6,
    MOBILOG_ID_KERNEL = 7, /* Third-parties can not use it */

    MOBILOG_ID_ATF = 8,
    MOBILOG_ID_GZ = 9,
    MOBILOG_ID_BSP = 10,
    MOBILOG_ID_MMEDIA = 11,
    MOBILOG_ID_SCP = 12,
    MOBILOG_ID_SCP_B = 13,
    MOBILOG_ID_SSPM = 14,
    MOBILOG_ID_ADSP = 15,
    MOBILOG_ID_MCUPM = 16,

    MOBILOG_ID_MAX
} mobilog_id_t;

#endif
