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

#include "WpfaDriver.h"

#define WPFA_D_LOG_TAG "WpfaDriver"

WpfaDriver *WpfaDriver::sInstance = NULL;
Mutex WpfaDriver::sWpfaDriverInitMutex;

WpfaDriver::WpfaDriver() {
    mtkLogD(WPFA_D_LOG_TAG, "-new()");
    mWpfaDriverAdapter = NULL;
    mWpfaShmSynchronizer = NULL;
    mDriverTid = DRIVER_TID_START;
    init();
}

void WpfaDriver::init() {
    int ret = 0;
    int ccciHandler = -1;
    ret = checkShmControllerState();
    if (ret == 0) {
        ccciHandler = mWpfaShmSynchronizer->getCcciHandler();
        mtkLogD(WPFA_D_LOG_TAG, "init getCcciHandler:%d", ccciHandler);
        mWpfaDriverAdapter->setCcciHandler(ccciHandler);
        ret = checkDriverAdapterState();
    }
    mtkLogD(WPFA_D_LOG_TAG, "-init() ret=%d", ret);
}

WpfaDriver::~WpfaDriver() {
    mtkLogD(WPFA_D_LOG_TAG, "-del()");
}

WpfaDriver* WpfaDriver::getInstance() {
    if (sInstance != NULL) {
       return sInstance;
    } else {
       sWpfaDriverInitMutex.lock();
       sInstance = new WpfaDriver();
       if (sInstance == NULL) {
          mtkLogE(WPFA_D_LOG_TAG, "new WpfaDriver fail");
       }
       sWpfaDriverInitMutex.unlock();
       return sInstance;
    }
}

int WpfaDriver::registerCallback(event_id_enum eventId,
                                CallbackFunc callbackFunc) {
    std::map<int, CallbackStruc>::iterator iter;

    iter = mapCallbackFunc.find(eventId);
    if (iter != mapCallbackFunc.end()) {
        mtkLogE(WPFA_D_LOG_TAG, "[%s] callback already existed!", __FUNCTION__);
        return 1;
    }
    addCallback(eventId, callbackFunc);
    return 0;
}

int WpfaDriver::unregisterCallback(event_id_enum eventId) {
    std::map<int, CallbackStruc>::iterator iter;

    iter = mapCallbackFunc.find(eventId);
    if (iter != mapCallbackFunc.end()) {
        mapCallbackFunc.erase(iter);
        mtkLogD(WPFA_D_LOG_TAG, "[%s] remove callback", __FUNCTION__);
        return 0;
    } else {
        mtkLogE(WPFA_D_LOG_TAG, "[%s] callback not found!", __FUNCTION__);
        return 1;
    }
}

int WpfaDriver::notifyCallback(event_id_enum eventId, void *notifyArg) {
    int ret = 0;
    std::map<int, CallbackStruc>::iterator iter;

    iter = mapCallbackFunc.find(eventId);
    if (iter != mapCallbackFunc.end()) {
        //mtkLogD(WPFA_D_LOG_TAG, "[%s] invoke callback", __FUNCTION__);
        ret = (mapCallbackFunc[eventId].cb)((void *)notifyArg);
        return ret;
    } else {
        mtkLogE(WPFA_D_LOG_TAG, "[%s] callback not found!", __FUNCTION__);
    }
    return ret;
}

int WpfaDriver::addCallback(event_id_enum eventId,
                                CallbackFunc callbackFunc) {
    struct CallbackStruc callback;
    callback.cb = callbackFunc;
    mapCallbackFunc[eventId] = callback;
    mtkLogD(WPFA_D_LOG_TAG, "[%s] eventId=%d success", __FUNCTION__, eventId);
    return 0;
}

int WpfaDriver::notifyWpfaInit(){
    int retValue = checkDriverAdapterState();
    if (retValue == 0) {
        sp<WpfaDriverMessage> msg = WpfaDriverMessage::obtainMessage(
        MSG_A2M_WPFA_INIT,
        generateDriverTid(),
        CCCI_IP_TABLE_MSG,
        0);
        retValue = mWpfaDriverAdapter->sendMsgToControlMsgDispatcher(msg);
    }
    return retValue;
}

int WpfaDriver::notifyWpfaVersion(uint16_t apVer, uint16_t mdVer){
    uint32_t maxDataBufferSize = 0;
    uint32_t realDataBufferSize = 0;
    int retValue = checkDriverAdapterState();
    if (retValue == 0) {
        retValue = checkShmControllerState();
        if (retValue == 0) {
            wifiproxy_ap_md_filter_ver_t *pVersionData = (wifiproxy_ap_md_filter_ver_t *)calloc(1,
                        sizeof(wifiproxy_ap_md_filter_ver_t));

            if (pVersionData == NULL) {
                mtkLogD(WPFA_D_LOG_TAG, "[%s] pVersionData is NULL, return failed.", __FUNCTION__);
                return -1;
            }
            pVersionData->ap_filter_ver = apVer;
            pVersionData->md_filter_ver = mdVer;

            // set Total data buffer size for UL and DL
            maxDataBufferSize = mWpfaShmSynchronizer->getMaxDataBufferSize();
            realDataBufferSize = mWpfaShmSynchronizer->getRealDataBufferSize();
            // TODO: handle data buffer size not sync
            switch (CURRENT_SHM_CONFIG_MODE) {
                case SHM_CONFIG_DL_ONLY:
                    pVersionData->dl_buffer_size = maxDataBufferSize;
                    pVersionData->ul_buffer_size = 0;
                    break;

                case SHM_CONFIG_UL_ONLY:
                    /* not supported */
                    pVersionData->dl_buffer_size = 0;
                    pVersionData->ul_buffer_size = maxDataBufferSize;
                    break;

                case SHM_CONFIG_SHARE:
                    pVersionData->dl_buffer_size = maxDataBufferSize / 2;
                    pVersionData->ul_buffer_size = maxDataBufferSize / 2;
                    break;

                default:
                    mtkLogE(WPFA_D_LOG_TAG, "Error: unknown CURRENT_SHM_CONFIG_MODE");
                    WPFA_D_ASSERT(0);
                    return 0;
            }

            mtkLogD(WPFA_D_LOG_TAG, "pVersionData->dl_buffer_size:%d, ul_buffer_size:%d",
                    pVersionData->dl_buffer_size, pVersionData->ul_buffer_size);

            sp<WpfaDriverMessage> msg = WpfaDriverMessage::obtainMessage(
                    MSG_A2M_WPFA_VERSION,
                    generateDriverTid(),
                    CCCI_IP_TABLE_MSG,
                    0,
                    WpfaDriverVersionData(pVersionData, 1));
            retValue = mWpfaDriverAdapter->sendMsgToControlMsgDispatcher(msg);
            free(pVersionData);
        }
    }
    return retValue;
}

int WpfaDriver::sendDataPackageToModem(WpfaRingBuffer *ringBuffer) {
    int retValue = checkShmControllerState();
    if (retValue == 0) {
        retValue = mWpfaShmSynchronizer->wirteDataToShm(ringBuffer);
    }
    return retValue;
}

int WpfaDriver::checkDriverAdapterState() {
    int retValue = 0;
    if (mWpfaDriverAdapter != NULL) {
        return retValue;
    } else {
        mWpfaDriverAdapter = WpfaDriverAdapter::getInstance();
        if (mWpfaDriverAdapter == NULL) {
            mtkLogE(WPFA_D_LOG_TAG, "getInstance of WpfaDriverAdapter fail");
            retValue = -1;
        }
    }
    return retValue;
}

int WpfaDriver::checkShmControllerState() {
    int retValue = 0;
    if (mWpfaShmSynchronizer != NULL) {
        return retValue;
    } else {
        mWpfaShmSynchronizer = WpfaShmSynchronizer::getInstance();
        if (mWpfaShmSynchronizer == NULL) {
            mtkLogE(WPFA_D_LOG_TAG, "getInstance of checkShmControllerState fail");
            retValue = -1;
        }
    }
    return retValue;
}

uint16_t WpfaDriver::generateDriverTid() {
    uint16_t newTid = mDriverTid;
    newTid = newTid + 1;
    if (newTid >= DRIVER_TID_END) {
        newTid = DRIVER_TID_START;
    }
    mDriverTid = newTid;
    mtkLogD(WPFA_D_LOG_TAG, "generateDriverTid() newTid=%d", newTid);
    return newTid;
}
