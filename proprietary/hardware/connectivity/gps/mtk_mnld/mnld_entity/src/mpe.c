/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#include "mtk_lbs_utility.h"
#include "mnld.h"
#include "mtk_gps.h"
#include "mpe.h"
#include "gps_dbg_log.h"
#include "mpe_common.h"
#include "data_coder.h"
#include "gps_controller.h"

#ifdef LOGD
#undef LOGD
#endif
#ifdef LOGW
#undef LOGW
#endif
#ifdef LOGE
#undef LOGE
#endif
#if 0
#define LOGD(...) tag_log(1, "[mpe]", __VA_ARGS__);
#define LOGW(...) tag_log(1, "[mpe] WARNING: ", __VA_ARGS__);
#define LOGE(...) tag_log(1, "[mpe] ERR: ", __VA_ARGS__);
#else
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "mpe"

#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)
#ifdef CONFIG_GPS_ENG_LOAD
#define LOGD_ENG(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#else
#define LOGD_ENG(fmt, arg ...) NULL
#endif
#endif

MPECallBack gMpeCallBackFunc = NULL;
static int g_fd_mpe;
extern unsigned char gMpeThreadExist;

int mnl2mpe_set_log_path(char* path, int status_flag, int mode_flag) {
    char mnl2mpe_buff[MNL_MPE_MAX_BUFF_SIZE] = {0};
    int offset = 0;

    put_int(mnl2mpe_buff, &offset, CMD_SEND_FROM_MNLD);
    put_int(mnl2mpe_buff, &offset, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN + 2*sizeof(INT32));
    put_int(mnl2mpe_buff, &offset, status_flag);
    put_int(mnl2mpe_buff, &offset, mode_flag);
    put_binary(mnl2mpe_buff, &offset, (const char*)path, GPS_DEBUG_LOG_FILE_NAME_MAX_LEN);

    if (!(mpe_sys_get_mpe_conf_flag() & MPE_CONF_MPE_ENABLE)) {
        LOGD("MPE not enable\n");
        return MTK_GPS_ERROR;
    }
    if(safe_sendto(MNLD_MPE_SOCKET, mnl2mpe_buff, MNL_MPE_MAX_BUFF_SIZE) == -1) {
        LOGE("safe_sendto fail:[%s]%d", strerror(errno), errno);
        return MTK_GPS_ERROR;
    }
    return 0;
}

int mpe2mnl_hdlr(char *buff) {
    int type, length;
    int offset = 0;

    type = get_int(buff, &offset, MNL_MPE_MAX_BUFF_SIZE);
    length = get_int(buff, &offset, MNL_MPE_MAX_BUFF_SIZE);

    LOGD("type=%d length=%d\n", type, length);

    switch (type) {
        case CMD_MPED_REBOOT_DONE: {
            mtklogger_mped_reboot_message_update();
            break;
        }
        case CMD_START_MPE_RES:
        case CMD_STOP_MPE_RES:
        case CMD_SEND_SENSOR_RAW_RES:
        case CMD_SEND_SENSOR_CALIBRATION_RES:
        case CMD_SEND_SENSOR_FUSION_RES:
        case CMD_SEND_GPS_AIDING_RES:
        case CMD_SEND_ADR_STATUS_RES:
        case CMD_SEND_GPS_TIME_REQ: {
            if (mnld_is_gps_started_done()) {
                mtk_gps_mnl_get_sensor_info((UINT8 *)buff, length + sizeof(MPE_MSG));
            }
            break;
        }
        default: {
           LOGE("unknown cmd=%d\n", type);
           break;
       }
    }
    return 0;
}

int mtk_gps_mnl_trigger_mpe(void) {
    UINT16 mpe_len;
    char mnl2mpe_buff[MNL_MPE_MAX_BUFF_SIZE] = {0};
    int ret = MTK_GPS_ERROR;

    mpe_len = mtk_gps_set_mpe_info((UINT8 *)mnl2mpe_buff);
    LOGD_ENG("mpemsg len=%d\n", mpe_len);

    if (mpe_len > 0) {
        if (!(mpe_sys_get_mpe_conf_flag() & MPE_CONF_MPE_ENABLE)) {
            LOGD("MPE not enable\n");
            return MTK_GPS_ERROR;
        }
        safe_sendto(MNLD_MPE_SOCKET, mnl2mpe_buff, MNL_MPE_MAX_BUFF_SIZE);
        ret = MTK_GPS_SUCCESS;
    }
    return ret;
}
#if 0
static void mpe_thread_timeout() {
    if (mnld_timeout_ne_enabled() == false) {
        LOGE("mpe_thread_timeout() dump and exit.");
        gps_dbg_log_exit_flush(0);
        mnld_block_exit();
    } else {
        LOGE("mpe_thread_timeout() crash here for debugging");
        CRASH_TO_DEBUG();
    }
}
#endif
int mnl_mpe_thread_init() {
    int ret;
    LOGD("mpe enabled");

    gMpeCallBackFunc = mtk_gps_mnl_trigger_mpe;
    ret = mtk_gps_mnl_mpe_callback_reg((MPECallBack *)gMpeCallBackFunc);
    LOGD("register mpe cb %d,gMpeCallBackFunc= %p,mtk_gps_mnl_trigger_mpe=%p\n",
        ret, gMpeCallBackFunc, mtk_gps_mnl_trigger_mpe);
    return 0;
}

unsigned char mpe_sys_sensor_threads_create(void)
{
    pthread_t sensor_thread_handle;

    if (gMpeThreadExist == 0) {
        if(pthread_create(&sensor_thread_handle, NULL, mpe_sensor_thread, NULL)) {
            LOGD("MPE sensor thread init failed - pthread_create");
            return FALSE;
        }
    } else {
        LOGD("MPE sensor thread init failed - Thread exist");
        return FALSE;
    }
    return TRUE;
}

static void* mpe_main_thread(void *arg) {
    #define MAX_EPOLL_EVENT 50
    struct epoll_event events[MAX_EPOLL_EVENT];
    UNUSED(arg);
    pthread_detach(pthread_self());

    mnl2mpe_hdlr_init();
    int epfd = epoll_create(MAX_EPOLL_EVENT);
    if (epfd == -1) {
        LOGE("epoll_create failure reason=[%s]%d\n",
            strerror(errno), errno);
        return 0;
    }

    if (epoll_add_fd(epfd, g_fd_mpe) == -1) {
        LOGE("epoll_add_fd() failed for g_fd_epo failed");
        return 0;
    }
    while (1) {
        int i;
        int n;
        LOGD("wait");
        n = epoll_wait(epfd, events, MAX_EPOLL_EVENT , -1);
        if (n == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                LOGE("epoll_wait failure reason=[%s]%d",
                    strerror(errno), errno);
                return 0;
            }
        }
        for (i = 0; i < n; i++) {
            if (events[i].data.fd == g_fd_mpe) {
                if (events[i].events & EPOLLIN) {
                    mnl2mpe_hdlr(g_fd_mpe);
                }
            } else {
                LOGE("unknown fd=%d",
                    events[i].data.fd);
            }
        }
    }

    LOGE("exit");
    pthread_exit(NULL);
    return 0;
}

int mpe_function_init(void) {
    pthread_t calib_thread_handle;
    pthread_t main_thread_handle;

    mpe_sys_read_mpe_conf_flag();
    if (!(mpe_sys_get_mpe_conf_flag() & MPE_CONF_MPE_ENABLE)) {
        LOGD("MPE not enable\n");
        return MTK_GPS_SUCCESS;
    }

    if (mpe_sys_get_mpe_conf_flag() & MPE_CONF_AUTO_CALIB) {
        if(pthread_create(&calib_thread_handle, NULL, mpe_calib_thread, NULL)) {
            LOGE("MPE calib thread init failed");
        }
    }

    g_fd_mpe = socket_bind_udp(MNLD_MPE_SOCKET);

    if(pthread_create(&main_thread_handle, NULL, mpe_main_thread, NULL)) {
        LOGE("MPE main thread init failed");
    }

    return MTK_GPS_SUCCESS;
}
