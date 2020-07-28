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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <dlfcn.h>
#include "utility_ux.h"
#include "utility_netd.h"
#include <utils/Log.h>

/*****************************************************************************
 * ux_utility
 *****************************************************************************/
#undef LOG_TAG
#define LOG_TAG "UxUtility"
#define UX_RIL_PATH "libnwk_opt_halwrap_vendor.so"
void* sDlOpHandler = NULL;
void* func = NULL;

static int (*UxGameEvent)(int, int) = NULL;
static int (*notifyAppState)(const char*, const char*, uint32_t) = NULL;
static bool initlib = false;

static void initUxLibrary() {
    if (initlib) return;
    initlib = true;
    sDlOpHandler = dlopen(UX_RIL_PATH, RTLD_NOW);
    if (sDlOpHandler == NULL) {
        ALOGE("[%s] dlopen failed in %s: %s",
                __FUNCTION__, UX_RIL_PATH, dlerror());
        return;
    }
    func = dlsym(sDlOpHandler, "NwkOptHal_wrap_switch_game_mode_S");
    UxGameEvent = (power_notify_ux_game_event)(func);
    const char* dlsym_error = dlerror();

    if (UxGameEvent == NULL) {
        ALOGE("UxGameEvent error: %s", dlsym_error);
        return;
    }
    dlerror();
    func = dlsym(sDlOpHandler, "NwkOptHal_Wrap_notifyAppState_S");
    notifyAppState = (power_notify_AppState)(func);
    dlsym_error = dlerror();

    if (notifyAppState == NULL) {
        ALOGE("notifyAppState error: %s", dlsym_error);
        return;
    }

    // reset errors
    dlerror();

    ALOGD("[%s] completed", __FUNCTION__);
}

static int notifyUxGameEvent(int mode) {
    initUxLibrary();
    if (UxGameEvent == NULL) {
        ALOGE("notifyUxGameEvent error = NULL");
        return -1;
    }
    return UxGameEvent(mode, -1);
}

static void notifyNetdBoostUid(int enable, int uid) /* -1: don't care */
{
    static int netd_boost = -1;
    static int boost_uid = -1;
    int last_boost_uid = -1;

    ALOGD("notifyNetdBoostUid enable:%d -> %d, uid:%d -> %d",
        netd_boost, enable, boost_uid, uid);

    if (enable != -1 && enable != netd_boost) { /* -1: don't care */
        netd_boost = enable;

        if (netd_boost == 1) {
            if (boost_uid != -1) {
                netd_set_priority_uid(boost_uid, NULL);
            }
        } else {
            if (boost_uid != -1) {
                netd_clear_priority_uid(boost_uid, NULL);
            }
        }
    }

    if (uid != -1 && uid != boost_uid) { /* -1: don't care */
        last_boost_uid = boost_uid;
        boost_uid = uid;

        if (netd_boost == 1) {
            if (last_boost_uid != -1) {
                netd_clear_priority_uid(last_boost_uid, NULL);
            }

            if (boost_uid != -1) {
                netd_set_priority_uid(boost_uid, NULL);
            }
        }
    }
}

void notify_APPState(const char *packname, int32_t uid) {
    initUxLibrary();
    if (notifyAppState == NULL) {
        ALOGE("notifyAppState error = NULL");
        return;
    }
    notifyAppState(packname, NULL, uid);
    ALOGI("notifyAppState pack:%s, uid:%d", packname, uid);
}

void notifyForegroundApp(const char *packname, int32_t uid) {
    initUxLibrary();
    if (notifyAppState == NULL) {
        ALOGE("notifyAppState error = NULL");
        return;
    }
    notifyAppState(NULL, packname, uid);
    notifyNetdBoostUid(-1, uid);
    ALOGI("notifyForegroundApp pack:%s, uid:%d", packname, uid);
}

int notifyLowLatencyMode(int lowLatencyMode, void *scn) {
    ALOGV("notifyLowLatencyMode: %p", scn);
    if (lowLatencyMode == -1) return 0;
    if (lowLatencyMode == 1) {
        notifyUxGameEvent(GAME_EVENT_LOW_LATENCY);
        notifyNetdBoostUid(1, -1);
    } else if (lowLatencyMode == 0) {
        notifyUxGameEvent(GAME_EVENT_IDLE);
        notifyNetdBoostUid(0, -1);
    }
    ALOGD("notifyUxLatencyMode latency:%d", lowLatencyMode);
    return 0;
}

int resetUtilityUx(int poweron_init)
{
    if(poweron_init != 1) {
        notifyLowLatencyMode(0, NULL);
    }
    return 0;
}

void notifyCmdMode(int mode) {
    notifyUxGameEvent(mode | 0x10);
}


