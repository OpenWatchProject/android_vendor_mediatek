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

#include <CamPostAlgoProviderMgr.h>

namespace com {
namespace mediatek {
namespace campostalgo {

static Mutex providerLock;

android::sp<CamPostAlgoProviderMgr> CamPostAlgoProviderMgr::getInstance() {
    Mutex::Autolock _l(providerLock);
    static android::sp<CamPostAlgoProviderMgr> camPostAlgoProviderMgr;
    if (camPostAlgoProviderMgr == nullptr) {
        camPostAlgoProviderMgr = new CamPostAlgoProviderMgr();
        status_t res = camPostAlgoProviderMgr->initialize();
        if (res != android::OK) {
            ALOGE("%s: Unable to initialize provider manager: %s (%d)",
                    __FUNCTION__, strerror(-res), res);
        }
    }
    return camPostAlgoProviderMgr;
}

CamPostAlgoProviderMgr::CamPostAlgoProviderMgr() {

}

int32_t CamPostAlgoProviderMgr::getMaxInterfaceNum() {
    return MAX_INTERFACE_NUMBER;
}

CamPostAlgoProviderMgr::~CamPostAlgoProviderMgr() {
    Mutex::Autolock _l(mInterfaceMutex);
    mInterfaces.clear();
    ALOGD("CamPostAlgoProvider destroyed!");
}

status_t CamPostAlgoProviderMgr::initialize() {
    //get the interface
    //add interfaces
    Mutex::Autolock _l(mInterfaceMutex);
    int32_t num = getMaxInterfaceNum();
    for (int i = 0; i < num; i++) {

        android::sp<CamPostAlgoInterface> interface = new CamPostAlgoInterface(
                i);
        if (interface->initialize() == android::OK) {
            mInterfaces.push_back(interface);
        } else {
            ALOGE("CamPostAlgoProviderMgr Error init %d",
                    interface->getInterfaceId());
            return android::UNKNOWN_ERROR;
        }
    }
    return android::OK;
}

  sp<CamPostAlgoInterface> CamPostAlgoProviderMgr::getInterface() {
      Mutex::Autolock _l(mInterfaceMutex);
    if(mInterfaces.size() > 0) {
        ALOGD("CamPostAlgoProviderMgr getInterface size %zu", mInterfaces.size());
        return mInterfaces.at(0);
    } else {
        ALOGE("CamPostAlgoProviderMgr getInterface size 0");
        return nullptr;
    }
}

} /* namespace campostalgo */
} /* namespace mediatek */
} /* namespace com */
