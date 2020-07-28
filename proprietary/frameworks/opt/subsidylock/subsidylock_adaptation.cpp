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

#include <cutils/properties.h>
#include <utils/Mutex.h>
#include <vendor/mediatek/hardware/mtkradioex/1.0/IMtkRadioEx.h>
#include <vendor/mediatek/hardware/mtkradioex/1.0/ISubsidyLockIndication.h>
#include <vendor/mediatek/hardware/mtkradioex/1.0/ISubsidyLockResponse.h>
#include "subsidylock_ipc.h"
#include "subsidylock_adaptation.h"

#define MAX_SLOT_NUM 4
#define RIL1_SERVICE_NAME "mtkSlot1"
#define RIL2_SERVICE_NAME "mtkSlot2"
#define RIL3_SERVICE_NAME "mtkSlot3"
#define RIL4_SERVICE_NAME "mtkSlot4"

using namespace android::hardware::radio::V1_0;
using namespace vendor::mediatek::hardware::mtkradioex::V1_0;
using ::android::Mutex;
using ::android::hardware::Return;
using ::android::hardware::hidl_death_recipient;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Void;
using ::android::hidl::base::V1_0::IBase;
using ::android::sp;
using ::android::wp;
namespace NS_VENDOR = vendor::mediatek::hardware::mtkradioex::V1_0;

class SubsidyLockAdaptation {
private:
    sp<NS_VENDOR::IMtkRadioEx> mRadio[MAX_SLOT_NUM];
    sp<ISubsidyLockResponse> mSubsidyLockResponse;
    sp<ISubsidyLockIndication> mSubsidyLockIndication;
    int mFd = -1;
    static SubsidyLockAdaptation* sInstance;

public:
    static SubsidyLockAdaptation *getInstance();
    void setSocketFd(int fd) { mFd = fd; }
    bool sendRequest(int reqType, const char *data, int size);
    void sendResponse(const char *data, int size);

private:
    SubsidyLockAdaptation();
    void setUp(int slotId);
    int getSlotId();
};

class SubsidyLockResponse : public ISubsidyLockResponse {
private:
    SubsidyLockAdaptation *mSubsidyLock;

public:
    SubsidyLockResponse(SubsidyLockAdaptation *sublock) : mSubsidyLock(sublock) {}

    virtual Return<void> sendSubsidyLockResponse(const RadioResponseInfo& info,
            const hidl_vec<uint32_t>& data) override {
        const uint8_t *resp = (uint8_t*)data.data();
        int dataSize = (sizeof(uint32_t) / sizeof(uint8_t)) * data.size();

        LOGI("sendSubsidyLockResponse: info.error = %d, %d", info.error, data.size());
        if (data.size() > 0) {
            LOGD("sendSubsidyLockResponse: reqType = %d, status = %d", data[0], data[1]);
        }

        mSubsidyLock->sendResponse((const char *)resp, dataSize);
        return Void();
    }
};

SubsidyLockAdaptation* SubsidyLockAdaptation::sInstance = NULL;

SubsidyLockAdaptation* SubsidyLockAdaptation::getInstance() {
    if (sInstance == NULL) {
        sInstance = new SubsidyLockAdaptation();
    }
    return sInstance;
}

SubsidyLockAdaptation::SubsidyLockAdaptation() {
    mSubsidyLockResponse = new SubsidyLockResponse(this);
}

void SubsidyLockAdaptation::setUp(int slotId) {
    const char *serviceNames[MAX_SLOT_NUM] = {
        RIL1_SERVICE_NAME,
        RIL2_SERVICE_NAME,
        RIL3_SERVICE_NAME,
        RIL4_SERVICE_NAME
    };
    if (slotId >= 0 && slotId < MAX_SLOT_NUM) {
        mRadio[slotId] = NS_VENDOR::IMtkRadioEx::getService(serviceNames[slotId]);
        if (mRadio[slotId] != NULL) {
            mRadio[slotId]->setResponseFunctionsSubsidyLock(mSubsidyLockResponse, mSubsidyLockIndication);
        } else {
            LOGE("setUp: getService %s failed", serviceNames[slotId]);
        }
    }
}

bool SubsidyLockAdaptation::sendRequest(int reqType, const char *data, int size) {
    LOGI("sendRequest: reqType = %d, data = %s", reqType, data);
    int slotId = getSlotId();
    if (slotId >= 0 && slotId < MAX_SLOT_NUM) {
        if (mRadio[slotId] == NULL) {
            setUp(slotId);
        }
        if (mRadio[slotId] != NULL) {
            hidl_vec<uint8_t> vec;
            vec.setToExternal((uint8_t *)data, size);
            Return<void> ret = mRadio[slotId]->sendSubsidyLockRequest(0xFFFFFFFF, reqType, vec);
            if (ret.isOk()) {
                LOGI("sendRequest success");
                return true;
            } else {
                LOGI("sendRequest failed");
                mRadio[slotId] = NULL;
            }
        }
    }
    return false;
}

void SubsidyLockAdaptation::sendResponse(const char *data, int size) {
    LOGI("sendResponse: %s", data);
    //Mutex::Autolock autoLock(mLock);
    if (data != NULL) {
        int sendLen = send(mFd, data, size, 0);
        LOGI("SubsidyLockAdaptation::sendResponse: sendLen = %d", sendLen);
    }
}

int SubsidyLockAdaptation::getSlotId() {
    char simNo[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.vendor.radio.simswitch", simNo, "0");
    int slotId = atoi(simNo) - 1;
    LOGD("getSlotId: simNo: %s, slotId = %d", simNo, slotId);
    if (slotId >= 0 && slotId < MAX_SLOT_NUM) {
        return slotId;
    }
    LOGE("SubsidyLockAdaptation::getSlotId(): unsupported slot number.");
    return -1;
}

bool sendRequest(int reqType, const char *data, int size) {
    return SubsidyLockAdaptation::getInstance()->sendRequest(reqType, data, size);
}

void setSocketFdForAdaptation(int fd) {
    SubsidyLockAdaptation::getInstance()->setSocketFd(fd);
}