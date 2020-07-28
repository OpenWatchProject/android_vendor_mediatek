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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
//#include <linux/capability.h>
#include <sys/capability.h>

#include <private/android_filesystem_config.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <dirent.h>
#include <cutils/log.h>
#include "commandlistening.h"
//#include "commandcontrol.h"

//#define LOG_TAG "NetworkingDiagnose"
#define MAX 128
#ifdef __cplusplus
extern "C" {
#endif
int drvb_f0(void);
#ifdef __cplusplus
}
#endif
char network_service[16] = "netdiag";

void sig_handler(int s);

/*bool ChipSecurityCheck(void)
 {
 int ret;
 ret = drvb_f0();
 if(ret != 0)
 {
 LOGE("mobilel_log_d the chip dismatch: %d\n", ret);
 return false;
 }
 return true;
 }*/
/* Set UID to inet w/ CAP_NET_RAW, CAP_NET_ADMIN and CAP_NET_BIND_SERVICE
 * (Android's init.rc does not yet support applying linux capabilities) */
void netdiag_set_aid_and_cap() {
    /* We ignore errors here since errors are normal if we
     * are already running as non-root.
     */
    gid_t groups[] = { AID_ADB, AID_LOG, AID_INPUT, AID_INET, AID_NET_ADMIN,
            AID_NET_BT, AID_SYSTEM, AID_SDCARD_RW, AID_MOUNT,
            /***/AID_NET_BW_STATS, AID_NET_RAW, AID_SDCARD_R, AID_READPROC, AID_MEDIA_RW };
    setgroups(sizeof(groups) / sizeof(groups[0]), groups);
    prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0);
    if (setgid(AID_SYSTEM) == 0)
        LOGD("setgid AID_SYSTEM successfully!");
    else
        LOGD("setgid AID_SYSTEM failed!");
    if (setuid(AID_SHELL) == 0)
        LOGD("setuid SHELL successfully!");
    else
        LOGD("setuid SHELL failed!");
    struct __user_cap_header_struct header;
    struct __user_cap_data_struct cap;
    header.version = _LINUX_CAPABILITY_VERSION;
    header.pid = 0;
    cap.effective = cap.permitted = 1 << CAP_NET_RAW | 1 << CAP_NET_ADMIN
            | 1 << CAP_NET_BIND_SERVICE | 1 << CAP_SYS_BOOT;
    cap.inheritable = 0;
    if (capset(&header, &cap) < 0) {
        LOGE("capset return value below 0!");
    }
}

int main() {
    umask(000);
    netdiag_set_aid_and_cap();
    /*if(!ChipSecurityCheck())
     return 0;*/LOGI("network_dignose_daemon doing");

    struct sigaction act, oact;
    act.sa_handler = sig_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (sigaction(SIGCHLD, &act, &oact) < 0) {
        LOGE("sigaction failed");
        exit(1);
    }

    commandlistening * c1;

    c1 = new commandlistening(network_service);

    c1->startlistening();

    while (1) {
        LOGD("network_dignose_daemon loop");
        sleep(2000);
    }
    LOGI("network_dignose_daemon exiting");
    return 0;
}

void sig_handler(int s) {
    pid_t pid;
    int stat;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        LOGE("child %d terminated. Receive signal%d in main.\n", pid, s);
        return;
    }
}