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


#define MTK_LOG_ENABLE 1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <log/log.h>
#include <sys/prctl.h>
#include <sys/capability.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cutils/properties.h>
#include <private/android_filesystem_config.h>
#include "config.h"
#include "global_var.h"
#include "mlog.h"
#include "bootmode.h"
#include "daemon.h"
#include "libfunc.h"


/* for aee */
AEE_SYS_FUNC *aee_sys_exp;
AEE_SYS_FUNC *aee_sys_war;
extern void load_dynamic_shared_libs(void);

/* for mobile_log_d status and global logging path  */
MBLOGSTATUS g_mblog_status;
char last_logging_path[256];
char cur_logging_path[256];


#define TRACE_INSTANCE "/sys/kernel/debug/tracing/instances/"
#define TRACE_BUFFER_SIZE   1024
#define MAX_SEPRATE_EVENT   3

typedef enum ftrace_buf_type {
    BUF_TYPE_MMEDIA = 0,
    BUF_TYPE_BSP = 1,
    BUF_TYPE_MAX
} ftrace_buf_type;

typedef struct __ftrace_buf {
    int buf_size;           // tracepoint buffer size
    char name[16];          // tracepoint name
    char root_path[128];    // instance root path
    char enable_path[MAX_SEPRATE_EVENT][128];   // enable/disable control file path
    char size_path[128];    // size control file path
    char log_path[128];     // log output file path
} ftrace_buf_t;

static ftrace_buf_t ftrace_buf[BUF_TYPE_MAX];

char *BUF_NAME[BUF_TYPE_MAX] = {
    [BUF_TYPE_MMEDIA] = "mmedia",
    [BUF_TYPE_BSP] = "bsp",
};

int ftrace_buf_init(ftrace_buf_type type) {
    char cmd[512];
    int i, status, e, count = 20;
    char *name = BUF_NAME[type];

    ALOGI("%s ftrace buffer initializing...", name);

    ftrace_buf[type].buf_size = TRACE_BUFFER_SIZE;
    memset(ftrace_buf[type].name, 0x0, sizeof(ftrace_buf[type].name));
    snprintf(ftrace_buf[type].name, sizeof(ftrace_buf[type].name), "%s", name);

    memset(ftrace_buf[type].root_path, 0x0, sizeof(ftrace_buf[type].root_path));
    snprintf(ftrace_buf[type].root_path, sizeof(ftrace_buf[type].root_path),
            "%s%s", TRACE_INSTANCE, name);

    /*for mmedia buff, there is just one enable path*/
    for (i = 0; i< MAX_SEPRATE_EVENT; i++)
        memset(ftrace_buf[type].enable_path[i], 0x0, sizeof(ftrace_buf[type].enable_path[i]));
    if (type == BUF_TYPE_MMEDIA) {
        for (i = 0; i < MAX_SEPRATE_EVENT; i++)
            snprintf(ftrace_buf[type].enable_path[i], sizeof(ftrace_buf[type].enable_path[i]),
                    "%s%s", ftrace_buf[type].root_path, "/events/fileop/enable");
    } else if (type == BUF_TYPE_BSP) {
        snprintf(ftrace_buf[type].enable_path[0], sizeof(ftrace_buf[type].enable_path[0]),
                "%s%s", ftrace_buf[type].root_path, "/events/signal/enable");
        snprintf(ftrace_buf[type].enable_path[1], sizeof(ftrace_buf[type].enable_path[1]),
                "%s%s", ftrace_buf[type].root_path, "/events/sched/sched_fork_time/enable");
        snprintf(ftrace_buf[type].enable_path[2], sizeof(ftrace_buf[type].enable_path[2]),
                "%s%s", ftrace_buf[type].root_path, "/events/sched/sched_process_exit/enable");
    }

    memset(ftrace_buf[type].size_path, 0x0, sizeof(ftrace_buf[type].size_path));
    snprintf(ftrace_buf[type].size_path, sizeof(ftrace_buf[type].size_path),
            "%s%s", ftrace_buf[type].root_path, "/buffer_size_kb");

    memset(ftrace_buf[type].log_path, 0x0, sizeof(ftrace_buf[type].log_path));
    snprintf(ftrace_buf[type].log_path, sizeof(ftrace_buf[type].log_path),
            "%s%s", ftrace_buf[type].root_path, "/trace_pipe");

    status = mkdir(ftrace_buf[type].root_path, S_IRWXU | S_IRWXG| S_IROTH | S_IXOTH);
    e = errno;
    if (status < 0) {
        ALOGE("create %s failed(%s)", ftrace_buf[type].root_path, strerror(errno));
        return -1;
    }
    status = chown(ftrace_buf[type].root_path, AID_SHELL, AID_SYSTEM);
    e = errno;
    if (status < 0) {
        ALOGE("chown %s failed(%s)", ftrace_buf[type].root_path, strerror(errno));
        return -1;
    }
    // wait at most 2s
    for (i = 0; i< MAX_SEPRATE_EVENT; i++) {
        count = 20;
        while (count) {
            if (access(ftrace_buf[type].enable_path[i], F_OK)) {
                int r = usleep(100000);
                if (r < 0) {
                    ALOGE("usleep error %s", strerror(errno));
                }
                count--;
                continue;
            }
            break;
        }
    }
    for (i = 0; i < MAX_SEPRATE_EVENT; i++) {
        status = access(ftrace_buf[type].enable_path[i], F_OK);
        e = errno;
        if (status < 0) {
            ALOGE("access %s failed(%s)", ftrace_buf[type].enable_path[i], strerror(errno));
        }
    }
    status = access(ftrace_buf[type].size_path, F_OK);
    e = errno;
    if (status < 0) {
        ALOGE("access %s failed(%s)", ftrace_buf[type].size_path, strerror(errno));
        return -1;
    }
    status = access(ftrace_buf[type].log_path, F_OK);
    e = errno;
    if (status < 0) {
        ALOGE("access %s failed(%s)", ftrace_buf[type].log_path, strerror(errno));
        return -1;
    }
    status = chown(ftrace_buf[type].log_path, AID_SYSTEM, AID_SYSTEM);
    e = errno;
    if (status < 0) {
        ALOGE("chown %s failed(%s)", ftrace_buf[type].log_path, strerror(errno));
        return -1;
    }
    memset(cmd, 0, sizeof(cmd));
    for (i = 0; i < MAX_SEPRATE_EVENT; i++) {
        snprintf(cmd, sizeof(cmd), "echo 1 > %s", ftrace_buf[type].enable_path[i]);
        system(cmd);
    }
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "echo %d > %s", ftrace_buf[type].buf_size, ftrace_buf[type].size_path);
    system(cmd);
    ALOGI("%s ftrace buffer initialize done", ftrace_buf[type].name);
    return 0;
}

int ftrace_buf_deinit() {
    int status, i, e;
    char rootpath[128];
    ALOGI("ftrace buffer deinitializing...");
    for (i = 0; i < BUF_TYPE_MAX; i++) {
        snprintf(rootpath, sizeof(rootpath), "%s%s", TRACE_INSTANCE, BUF_NAME[i]);
        if (!access(rootpath, F_OK)) {
            ALOGI("removing: %s", rootpath);
            status = remove(rootpath);
            e = errno;
            if (status < 0) {
                ALOGE("remove %s failed(%s)", rootpath, strerror(errno));
                return -1;
            }
            ALOGI("%s ftrace buffer deinit completely.", BUF_NAME[i]);
        }
    }
    return 0;
}

static int drop_privs() {
    if (prctl(PR_SET_KEEPCAPS, 1) < 0) {
        return -1;
    }

    gid_t groups[] = { AID_LOG, AID_ROOT, AID_SDCARD_RW, AID_MEDIA_RW,
            AID_MISC, AID_READPROC, AID_RADIO, AID_INET };

    if (setgroups(sizeof(groups) / sizeof(groups[0]), groups) == -1) {
        ALOGE("setgroup fail");
        return -1;
    }

    if (setgid(AID_SYSTEM) != 0) {
        ALOGE("setgid fail");
        return -1;
    }

    if (setuid(AID_SYSTEM) != 0) {
        ALOGE("setuid fail");
        return -1;
    }

    struct __user_cap_header_struct capheader;
    struct __user_cap_data_struct capdata[2];
    memset(&capheader, 0, sizeof(capheader));
    memset(&capdata, 0, sizeof(capdata));
    capheader.version = _LINUX_CAPABILITY_VERSION_3;
    capheader.pid = 0;

    // CAP_SYS_NICE for I/O scheduling class and priority
    capdata[CAP_TO_INDEX(CAP_SYS_NICE)].permitted = CAP_TO_MASK(CAP_SYS_NICE);
    // CAP_CHOWN for change file ownership
    capdata[CAP_TO_INDEX(CAP_CHOWN)].permitted  |= CAP_TO_MASK(CAP_CHOWN);
    // CAP_SYSLOG for /proc/kmsg
    capdata[CAP_TO_INDEX(CAP_SYSLOG)].permitted |= CAP_TO_MASK(CAP_SYSLOG);

    capdata[0].inheritable = capdata[0].effective = capdata[0].permitted;
    capdata[1].inheritable = capdata[1].effective = capdata[1].permitted;

    if (capset(&capheader, &capdata[0]) < 0) {
        return -1;
    }


    return 0;
}

void check_if_firstboot() {
    int is_firstboot = 1;

    char first_start[PROPERTY_VALUE_MAX];
    char old_path[PROPERTY_VALUE_MAX];

    if (property_get(PROP_RUNNING, first_start, "")) {
        MLOGI_BOTH("maybe not first start, and property is %s", first_start);
        is_firstboot = 0;

        // capture the sdcard path which was set on last runtime
        property_get(PROP_OLDPATH, old_path, "");
        update_sd_context(old_path);
    }
}

static void sig_handler(int signo) {
    uint64_t u = SIG_STOPLOG_EVENT;

    MLOGI_BOTH("sig_handler for: %d", signo);
    int ret = TEMP_FAILURE_RETRY(write(event_fd, &u, sizeof(u)));
    if (ret < 0) {
        MLOGE_BOTH("Sig_handler, Write %llu to event_fd fail (%s)",
                (unsigned long long)u, strerror(errno));
    }
}

extern int send_to_control(char* buf, size_t len);

static int control_handler(const char *cmd) {
    char cmdMsg[256];
    ALOGI("control: %s", cmd);

    if (!strcmp(cmd, "copy")) {
        char path[200];

        if (get_custom_mobilog_path(path, sizeof(path)) < 0) {
            ALOGE("mobilelog --control: get_custom_mobilog_path fail!");
            return 0;
        }
        // set_storage_path,xxxxx/xxxx
        snprintf(cmdMsg, sizeof(cmdMsg), "%s,%s", STORAGE_PATH, path);
        ALOGI("send mobilogpath: %s", cmdMsg);
        send_to_control(cmdMsg, sizeof(cmdMsg));

        snprintf(cmdMsg, sizeof(cmdMsg), "%s", cmd);
        if (check_internal_sdcard())
            ALOGI("sdcard ready, send auto copy and dump.");
        else
            ALOGE("sdcard Not ready, Anyway! send auto copy and dump.");
        // for special customer, alway need seed copy at the end
        send_to_control(cmdMsg, sizeof(cmdMsg));
    } else if (!strcmp(cmd, "sublog_config")) {
        char mask[PROPERTY_VALUE_MAX];
        // get sublog_config mask, eg, 0x0000DD81
        property_get(PROP_SUBLOG_CONFIG, mask, "");
        snprintf(cmdMsg, sizeof(cmdMsg), "%s=%s", "sublog_ALL", mask);
        ALOGI("send sublog_ALL: %s", cmdMsg);

        send_to_control(cmdMsg, sizeof(cmdMsg));

        if (is_mblog_running()) {
            snprintf(cmdMsg, sizeof(cmdMsg), "%s", "stop");
            send_to_control(cmdMsg, sizeof(cmdMsg));
            snprintf(cmdMsg, sizeof(cmdMsg), "%s", "start");
            send_to_control(cmdMsg, sizeof(cmdMsg));
        }
    }
    else
        ALOGE("Error control: %s", cmd);
    return 0;
}

int main(int argc, char **argv) {
    /*the third argument is ftrace buffer name or 'off'*/
    /*
     * !!!Do not use MLOG API during this phase,
     * MLOGx will create a root:root mblog_history which cannot access later!!!
     */
    if (argc > 1 && argv[1] && argv[2] && !strcmp(argv[1], "--ftrace") && !strcmp(argv[2], "mmedia")) {
        if (-1 == ftrace_buf_init(BUF_TYPE_MMEDIA))
            ALOGE("%s ftrace buffer init error", BUF_NAME[BUF_TYPE_MMEDIA]);
        return 0;
    }

    if (argc > 1 && argv[1] && argv[2] && !strcmp(argv[1], "--ftrace") && !strcmp(argv[2], "bsp")) {
        if (-1 == ftrace_buf_init(BUF_TYPE_BSP))
            ALOGE("%s ftrace buffer init error", BUF_NAME[BUF_TYPE_BSP]);
        return 0;
    }

    if (argc > 1 && argv[1] && argv[2] && !strcmp(argv[1], "--ftrace") && !strcmp(argv[2], "off")) {
        if (-1 == ftrace_buf_deinit())
            ALOGE("ftrace buffer deinit error");
        return 0;
    }

    // auto copy if no MTKLogger
    if (argc > 1 && argv[1] && argv[2] && !strcmp(argv[1], "--control")) {
        control_handler(argv[2]);
        return 0;
    }

    signal(SIGPIPE, SIG_IGN);
    drop_privs();

    /* process first history log */
    MLOGI_DATA("=====MOBILELOG START=======");

    check_if_firstboot();
    umask(000);
    property_set(PROP_RUNNING, "0");

    /* load lib --libaed.so */
    load_dynamic_shared_libs();

    init_bootmode_and_config();
    setup_socket();
    follow_up_work();

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    TEMP_FAILURE_RETRY(pause());
    exit(0);
}
