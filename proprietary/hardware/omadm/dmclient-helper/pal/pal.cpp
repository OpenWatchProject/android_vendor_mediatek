/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*
* MediaTek Inc. (C) 2017. All rights reserved.
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

//#include <private/android_filesystem_config.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <sys/cdefs.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>

#include "../pal/pal_internal.h"
#include "omadm_hidl.h"
#include "omadm_service_api.h"
#include "pal.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "PAL"
#define DEBUG
using namespace android;
using namespace omadm_service_api;

// Shared mutex objects
Mutex OmadmServiceAPI::mPalLock;
Mutex OmadmServiceAPI::mPalOmadmControllerLock;

// Initialize static fields
KeyedVector<omadmCallback_id, omadmCallback> * OmadmServiceAPI::mOmadmCallbackData = NULL;
OmadmListener* OmadmListener::mOmadmListener = NULL;
bool OmadmServiceAPI::mInitialized = false;

int pal_init() {
    ALOGI("pal_init");
    AutoMutex _l(OmadmServiceAPI::mPalLock);
    int ret = OmadmServiceAPI::initBinder(true);   

    if (ret != NO_ERR) {
        ALOGE("pal_init() fault with error %d", ret);
    }

    return ret;
}

void pal_fini() {
    AutoMutex _l(OmadmServiceAPI::mPalLock);
    pal_fini_event_handler();
}

int pal_register_omadm_callback(omadmCallback_id omadmCallbackId, omadmCallback omadmcallback) {
    AutoMutex _l(OmadmServiceAPI::mPalLock);

    if(!OmadmServiceAPI::isPalInitialized())
        return ERR_INIT;

    int reply = OmadmServiceAPI::registerOmadmCallback(omadmCallbackId, omadmcallback);
#ifdef DEBUG
    ALOGD("pal_register_omadm_callback return: %d", NO_ERR);
#endif
    return NO_ERR;
}

int pal_unregister_omadm_callback(omadmCallback_id omadmCallbackId) {
    AutoMutex _l(OmadmServiceAPI::mPalLock);

    if(!OmadmServiceAPI::isPalInitialized())
        return ERR_INIT;

    int reply = OmadmServiceAPI::unregisterOmadmCallback(omadmCallbackId);
#ifdef DEBUG
    ALOGD("pal_unregister_omadm_callback return: %d", NO_ERR);
#endif
    return NO_ERR;
}

