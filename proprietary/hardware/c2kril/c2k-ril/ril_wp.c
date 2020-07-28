/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <getopt.h>
#include "atchannel.h"
#include "ril_callbacks.h"
#include "misc.h"
#include "ril_sim.h"
#include <utils/Log.h>
#include <cutils/properties.h>

extern void sendPendedUrcs();

int bWorldModeSwitching = 0;


int isWorldModeSwitching(const char *s) {
    int needIgnore = 0;
    if (bWorldModeSwitching == 1) {
        if (strStartsWith(s, "+CIEV:12")
            || strStartsWith(s, "+CIEV:107")
            || strStartsWith(s, "+CIEV:109")
            || strStartsWith(s, "+CIEV:101")
            || strStartsWith(s, "+CIEV:110")
            || strStartsWith(s, "+CIEV:130")
            || strStartsWith(s, "+CIEV:131")
            || strStartsWith(s, "^SIMST:")
            || strStartsWith(s, "+UIMST:")
            || strStartsWith(s, "+VCPCOMPILE:")) {
            RLOGD("WorldModeSwitching=true, don't ignore URC%s", s);
            needIgnore = 0;
        } else {
            needIgnore = 1;
        }
    }
    return needIgnore;
}

/// M: world mode switching, need pass special request. @{
int allowForWorldModeSwitching(const int request) {
    if (bWorldModeSwitching == 1) {
        if (request == RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE) {
            RLOGD("WorldModeSwitching=true, don't ignore RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE");
            return 1;
        } else if (request == RIL_REQUEST_RADIO_POWER) {
            RLOGD("[WM]WorldModeSwitching=true, don't ignore RIL_REQUEST_RADIO_POWER");
            return 1;
        }  else if (request == RIL_REQUEST_ENABLE_MODEM) {
            RLOGD("WorldModeSwitching=true, don't ignore RIL_REQUEST_ENABLE_MODEM");
            return 1;
        }
    }
    return 0;
}
/// @}

/// M: world mode switch. @{
int isWorldModeSwitch(int cievVal) {
    LOGD("[WM][%s]bSwitching = %d, val = %d", __func__, bWorldModeSwitching, cievVal);
    if ((bWorldModeSwitching == 0 && cievVal == 1) || (bWorldModeSwitching == 1 && cievVal == 0)) {
        return 0;
    } else if (bWorldModeSwitching == 1) {
        return 1;
    } else {
        char worldModeState[PROPERTY_VALUE_MAX] = { 0 };
        property_get("persist.vendor.radio.wm_state", worldModeState, "1");
        LOGD("[WM][%s]GSM World Mode State = %s", __func__, worldModeState);
        if (strcmp("0", worldModeState) == 0) {
            return 1;
        } else {
            return 0;
        }
    }
}

void onWorldModeChanged(int wmswitch) {
    int status[2] = { 0, 3 };
    status[0] = wmswitch;
    LOGD("[WM][%s]state (%d)", __func__, status[0]);

    if (status[0] == 0) {
        LOGD("[WM][onWorldModeChanged]-start");
        // Set flag when world mode changed start
        bWorldModeSwitching = 1;
        // Reset aid info
        resetAidInfo();
        // notify world mode changed start
        RIL_onUnsolicitedResponse(RIL_UNSOL_WORLD_MODE_CHANGED, status, sizeof(status));
    } else {
        // Set flag when world mode changed end
        bWorldModeSwitching = 0;
        // Reset aid info
        resetAidInfo();
        // notify world mode changed done
        RIL_onUnsolicitedResponse(RIL_UNSOL_WORLD_MODE_CHANGED, status, sizeof(status));
        // send pending special URC when world mode changed end
        sendPendedUrcs();
        LOGD("[WM][onWorldModeChanged]-end");
    }
}

void onWorldModePrepareTRM(int resetType) {
    if (bWorldModeSwitching == 0) {
        return;
    }
    switch (resetType) {
        case 1:
        case 2:
        case 19:  // Only for hot-plug
        {
            LOGD("[WM][%s] (%d)", __func__, resetType);
            bWorldModeSwitching = 0;
            int status[2] = { 0, 3 };
            status[0] = 1;  // notify world mode change as end
            RIL_onUnsolicitedResponse(RIL_UNSOL_WORLD_MODE_CHANGED, status, sizeof(status));
            break;
        }
        default:
        break;
    }
}
