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
#include <stdio.h>
#include <fcntl.h>
#include <log/log.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <cutils/properties.h>

#include "global_var.h"
#include "mlog.h"
#include "config.h"
#include "bootmode.h"
#include "daemon.h"
#include "dump.h"
#include "libfunc.h"
#include "logging.h"
#include "debug_config.h"


BOOTMODE g_bootmode;
int meta_mblogenable = 0;

/*
 * Read this time boot mode
 */

void set_status(MBLOGSTATUS status) {
    g_mblog_status = status;
}

#define BOOT_MODE_INFO_FILE "/sys/class/BOOT/BOOT/boot/boot_mode"
#define BOOT_MODE_STR_LEN 1
int get_bootmode(void) {
    int mode = -1;
    char buf[BOOT_MODE_STR_LEN + 1];
    int bytes_read = 0;
    int fd, res = 0;

    fd = open(BOOT_MODE_INFO_FILE, O_RDONLY);
    if (fd != -1) {
        memset(buf, 0, BOOT_MODE_STR_LEN + 1);
        while (bytes_read < BOOT_MODE_STR_LEN) {
            res = read(fd, buf + bytes_read, BOOT_MODE_STR_LEN);
            if (res > 0)
                bytes_read += res;
            else
                break;
        }
        close(fd);
        mode = atoi(buf);
    } else {
        MLOGE_DATA("open %s fail,%s", BOOT_MODE_INFO_FILE, strerror(errno));
        return -1;
    }

    MLOGI_DATA("boot mode is %d", mode);
    return mode;
}

int init_bootmode_and_config() {
    int bootmode;
    const char *customer = NULL;

    bootmode = get_bootmode();
    if (bootmode == -1) {
        MLOGE_DATA("Get bootmode fail");
        g_bootmode = UNKNOWN_BOOT;
        return 0;
    }

    g_bootmode = (BOOTMODE)bootmode;

    if (access(CONFIG_FILE, F_OK) != 0) {
        MLOGI_DATA("Can not access %s, %s", CONFIG_FILE, strerror(errno));
        create_config_file();
    }

    customer = read_bootmode_config(KEY_SPEC_CUSTOM);
    snprintf(spec_customer, sizeof(spec_customer), "%s", customer);
    MLOGI_DATA("customer is: %s", spec_customer);
    init_custom_id(spec_customer);

    gAllMode = ifAllModeConfigUnified();

    return 1;
}

/*
 */
const char* read_bootmode_config(const char *key) {
    if (!strcmp(key, KEY_SIZE) && g_mblog_status != SAVE_TO_SDCARD)
        return DATA_SIZE_DEFAULT;
    return read_config_value(key);
}

/*
* Attention Please:
*
* This function may be invoked by root. Do not use MLOG API during this function,
* MLOGx may create a root:root mblog_history which cannot access later
*/
int check_internal_sdcard() {
    struct stat exlink;
    int ret, e;
    int waitSeconds = 120; /*120s timeout*/

    while (waitSeconds--) {
        ret = lstat(PATH_DEFAULT, &exlink);
        e = errno;
        if (ret == -1) {
            ALOGI("check '%s' fail(%s)", PATH_DEFAULT, strerror(e));
            usleep(1000000);  // sleep 1 seoncd
            continue;
        } else {
            ALOGE("Inter sdcard can access!");
            return 1;
        }
    }
    ALOGE("check internal sdcard timeout!");
    return 0;
}

int check_external_sdcard() {
    struct timeval begin, current;
    struct stat exlink;
    int ret, e;

    char ext_path[255];
    char uuid[20];
    DIR *src_dir;
    struct dirent *entry;

    bool found = false;
    #define ROOTDIR  PATH_EXTSD_PARENT

    // check inter-sdcard
    gettimeofday(&begin, NULL);
    suseconds_t timeout = 60000000; /*60s timeout*/
    while (1) {
        gettimeofday(&current, NULL);
        if (1000000 * (current.tv_sec - begin.tv_sec) + (current.tv_usec - begin.tv_usec) > timeout) {
            MLOGE_BOTH("check internal sdcard timeout!");
            break;
        }

        ret = lstat(PATH_DEFAULT, &exlink);
        e = errno;
        if (ret == -1) {
            int r;
            MLOGE_BOTH("check '%s' fail(%s)", PATH_DEFAULT, strerror(e));
            r = usleep(1000000);
            if (r < 0) {
                MLOGE_BOTH("usleep error %s", strerror(errno));
            }
            continue;
        } else
            MLOGI_BOTH("Inter sdcard has been mounted !");
        break;
    }

    // Ensure inter-sdcard mounted, then check ext-sdcard
    int i = 8;
    while (i--) {
        src_dir = opendir(ROOTDIR);
        if (src_dir == NULL) {
            MLOGE_BOTH("Failed to opendir %s (%s)", ROOTDIR, strerror(errno));
            return 0;
        }

        while ((entry = readdir(src_dir)) != NULL) {
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")
                || entry->d_type != DT_DIR) {
                continue;
            }
            if (strlen(entry->d_name) == 9) {
                snprintf(uuid, sizeof(uuid), "%s", entry->d_name);
                if (uuid[4] == '-') {
                    found = true;
                    break;
                }
            }
        }

        if (closedir(src_dir)) {
            MLOGE_BOTH("Failed to closedir %s, (%s)", ROOTDIR, strerror(errno));
        }

        if (found == true) {
            snprintf(ext_path, sizeof(ext_path), "%s%s/", ROOTDIR, uuid);
            update_sd_context(ext_path);
            MLOGI_BOTH("Ext sdcard path: %s", ext_path);
            break;
        } else
            usleep(1000000);
    }

    if (found == false) {
        MLOGI_BOTH("can't find external sdcard !");
        return 0;
    }

    return 1;
}

int getItemReverse(FILE* fp, char item[], int len, int no) {
    long fileLen = 0;
    long offset = 0;
    if (fp == NULL || item == NULL || len <= 0 || no > BOOTMAX || no <= 0) {
        MLOGE_BOTH("Require Item with invalid args.");
        return -1;
    }

    if (fseek(fp, 0L, SEEK_END) != 0) {
        MLOGE_BOTH("Error fseeking END of bootFileTree, %s", strerror(errno));
        return -1;
    }
    if ((fileLen = ftell(fp)) < 0) {
        MLOGE_BOTH("Error ftelling of bootFileTree, %s", strerror(errno));
        return -1;
    }

    offset = no * (ItemMaxLen + 1);
    MLOGI_BOTH("file length: %ld, offset-to-End: %ld", fileLen, offset);
    if (offset > fileLen) {
        MLOGE_BOTH("Required Item is out of bounds.");
        return -1;
    }

    if (fseek(fp, -1 * offset, SEEK_END) != 0) {
        MLOGE_BOTH("Error fseeking offset of bootFileTree, %s", strerror(errno));
        return -1;
    }
    if (fgets(item, len, fp) != NULL) {
        *(item + len -2) = '\0';  // NULL replace '\n'

        /* test::
                int flags = O_RDWR | O_CREAT | O_APPEND;
                int mode = S_IRUSR | S_IWUSR;
                int fd;
                fd = TEMP_FAILURE_RETRY(open(TEST_FILE_TREE, flags, mode));
                if (fd < 0)
                    MLOGE_DATA("open testfileTree errro, %s", strerror(errno));
                else {
                    write(fd, item, len);
                }
                *(item + len -2) = '\0'; // NULL replace '\n'
                if (fd < 0)
                    MLOGE_DATA("open testfileTree errro, %s", strerror(errno));
                else {
                    write(fd, item, len);
                    close(fd);
                }
        */
    } else {
        MLOGE_BOTH("Require Item failed.");
        return 0;
    }
    return 1;
}

/*
 * after start, different bootmode may do different works
 *
 */
void follow_up_work() {
    int isboot;
    char buf[PROPERTY_VALUE_MAX];
    char buf_1[PROPERTY_VALUE_MAX];

    FILE *fp;
    char logF[ItemMaxLen + 2];  // Item + '\n' + '\0'
    char *st;
    long index = 0;
    char cmd[300];

    char bootLogDir[ItemMaxLen * 4];
    char logFolder[ItemMaxLen];

    if ((g_bootmode != META_BOOT) && (g_bootmode != FACTORY_BOOT) && (g_bootmode != ATE_FACTORY_BOOT)) {
        isboot = atoi(read_bootmode_config(KEY_BOOT));
        if (!isboot) {
            MLOGE_DATA("no start logging");
            set_status(STOPPED);
            property_set(PROP_RUNNING, "0");
            return;
        }
    }
    // bootup log saving
    set_status(SAVE_TO_BOOTUP);
    memset(cur_logging_path, 0x0, sizeof(cur_logging_path));

    // remove oldest path
    memset(logF, 0x0, sizeof(logF));
    fp = fopen(BOOTUP_FILE_TREE, "r");
    if (fp != NULL && getItemReverse(fp, logF, sizeof(logF), BOOTMAX) > 0
        && logF[0] != '\0' && logF[0] != '*') {
        snprintf(cmd, sizeof(cmd), "rm %s%s -rf", PATH_TEMP, logF);
        MLOGI_DATA("cmd: %s", cmd);
        system(cmd);
    }
    if (fp != NULL)
        fclose(fp);

    #define KEY "boot_"
    fp = fopen(BOOTUP_FILE_TREE, "r");
    if (fp != NULL && getItemReverse(fp, logF, sizeof(logF), 1) > 0) {
        st = strstr(logF, KEY);
        if (st != NULL) {
            index = strtol(st + strlen(KEY), NULL, 10);
            index = (index > 0 && index < BOOTMAX) ? (index + 1) : 1;
            MLOGI_DATA("item: %s, set index: %ld", logF, index);
        } else {
            index = 1;
            MLOGE_DATA("The name of item(%s) is invalid, reset index to %ld", logF, index);
        }
    } else {
        index = 1;
        MLOGE_DATA("Can not get item, set index: %ld", index);
    }
    if (fp != NULL) {
        fclose(fp);
    }

    memset(logFolder, 0x00, sizeof(logFolder));
    switch (g_bootmode) {
        case NORMAL_BOOT:
            snprintf(logFolder, sizeof(logFolder), "%s%ld%s", KEY, index, "__normal/");
            break;
        case META_BOOT:
            snprintf(logFolder, sizeof(logFolder), "%s%ld%s", KEY, index, "__meta/");
            break;
        case FACTORY_BOOT:
        case ATE_FACTORY_BOOT:
            snprintf(logFolder, sizeof(logFolder), "%s%ld%s", KEY, index, "__factory/");
            break;
        default:
            snprintf(logFolder, sizeof(logFolder), "%s%ld%s", KEY, index, "__other/");
    }
    snprintf(bootLogDir, sizeof(bootLogDir), "%s%s", PATH_TEMP, logFolder);
    if (access(bootLogDir, F_OK) == 0) {  // the folder exists, remove it
        snprintf(cmd, sizeof(cmd), "rm %s%s -rf", PATH_TEMP, logFolder);
        MLOGI_DATA("Before create, rm existed: %s", cmd);
        system(cmd);
    }
    if (!create_dir(bootLogDir)) {
        MLOGE_DATA("creat %s fail (%s)", bootLogDir, strerror(errno));
        set_status(STOPPED);
        property_set(PROP_RUNNING, "0");
        return;
    }

    /* Record 'logFolder' to bootup_file_tree */
    int flags = O_RDWR | O_CREAT | O_APPEND;
    int mode = S_IRUSR | S_IWUSR;
    int fd;
    fd = TEMP_FAILURE_RETRY(open(BOOTUP_FILE_TREE, flags, mode));
    if (fd < 0)
        MLOGE_DATA("Fail to open Bootup file tree, %s", strerror(errno));
    else {
        TEMP_FAILURE_RETRY(write(fd, logFolder, sizeof(logFolder)));
        TEMP_FAILURE_RETRY(write(fd, "\n", 1));
        close(fd);
    }

    switch (g_bootmode) {
        case NORMAL_BOOT:
            set_cur_logpath(bootLogDir);
            prepare_logging();
            return;
        case META_BOOT:
            set_cur_logpath(bootLogDir);
            prepare_logging();
            /*
            *   meta: ro.boot.mblogenable should be detected
            *   fast meta: ro.boot.meta_log_disable
            */
            property_get("ro.boot.mblogenable", buf, "0");
            property_get("ro.boot.meta_log_disable", buf_1, "1");
            if (buf[0] == '1' || buf_1[0] == '0')
                meta_mblogenable = 1;
            break;
        case FACTORY_BOOT:
        case ATE_FACTORY_BOOT:
            set_cur_logpath(bootLogDir);
            prepare_logging();
            break;
        default:
            set_cur_logpath(bootLogDir);
            prepare_logging();
            return;
    }

    // for Meta, factory, ate_factory
    if ((meta_mblogenable && g_bootmode == META_BOOT) ||
            g_bootmode == FACTORY_BOOT || g_bootmode == ATE_FACTORY_BOOT) {
        // Save the log in /data/debuglog
        update_sd_context(PATH_DATA);
        // If set delay timer fail, send auto copy event directly,
        // else waiting for meta start or time out.
        int result = set_delay_timer();
        MLOGI_BOTH("not normal mode, set_delay_timer, ret = %d", result);
        if (result == -1) {
            // no 'cmd' coming, send auto copy and dump event cmd
            if (g_mblog_status == SAVE_TO_BOOTUP) {
                uint64_t u = AUTO_COPY_EVENT;
                int ret = TEMP_FAILURE_RETRY(write(event_fd, &u, sizeof(u)));
                if (ret < 0) {
                    MLOGE_BOTH("Auto copy, Write %llu to event_fd fail (%s)",
                            (unsigned long long)u, strerror(errno));
                }
            }
        }
    } else {
        set_status(STOPPED);
    }

    return;
}

/*
 * The follow two functions will be call when start/stop
 */
int init_debug_config() {
    if (g_bootmode == NORMAL_BOOT) {
        MLOGI_BOTH("init debug");
        InitDebugConfig();
    }

    return 1;
}

int deinit_debug_config() {
    if (g_bootmode == NORMAL_BOOT) {
        MLOGI_BOTH("deinit debug");
        DeInitDebugConfig();
    }

    return 1;
}

/*
 * Select() should timeout in NORMAL_BOOT and META_BOOT FACTORY_BOOT
 * if sdcard has not mount for a long time
 */
int timeout_in_this_boot() {
    int isboot = atoi(read_bootmode_config(KEY_BOOT));

    if (!isboot)
        return 0;

    switch (g_bootmode) {
        case NORMAL_BOOT:
            return 1;
            break;

        default:
            return 0;
            break;
    }
}
