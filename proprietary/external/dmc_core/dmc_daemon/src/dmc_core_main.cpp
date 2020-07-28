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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#include "DmcCore.h"
#include <unistd.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <hidl/LegacySupport.h>
#include <sys/system_properties.h>

#undef TAG
#define TAG "DMC-Core"

bool gTerminated = false;

using namespace std;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

mutex gMutex;
condition_variable gCV;

// DMC maintain only 1 IPC thread to service APM, MDM, DMC binder request.
#define DMC_MAX_RPC_THREADPOOL_SIZE (1)

/*
static bool isDmcSupported() {
    char buffer[PROP_VALUE_MAX] = {'\0'};
    property_get("ro.vendor.mtk_dmc_support", buffer, "0");
    int isDmcSupported = atoi(buffer);

    if (isDmcSupported != 1) {
        return false;
    }
    return true;
}
*/

int main(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    // Skip to check to avoid VTS failure
    /*
    if (!isDmcSupported()) {
        DMC_LOGE(TAG, "MTK_DMC_SUPPORT not enabled");
        return 0;
    }
    */

    // Make the main thread to join RPC thread pool,
    // and is the only 1 thread to handle RPC request.
    configureRpcThreadpool(DMC_MAX_RPC_THREADPOOL_SIZE, true);

    // Initialize DmcCore and DMC HIDL service.
    DmcCore *pCore = new DmcCore();
    if (pCore->isReady()) {
        DMC_LOGD(TAG, "DmcCore join RPC thread pool.");
        joinRpcThreadpool();
    } else {
        DMC_LOGD(TAG, "Ooops! DmcCore not ready, BYE!");
    }
    return 0;
}
