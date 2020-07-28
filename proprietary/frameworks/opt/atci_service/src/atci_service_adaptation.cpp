/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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
#include <hidl/HidlTransportSupport.h>
#include <hwbinder/IPCThreadState.h>
#include <hwbinder/ProcessState.h>
#include <sys/socket.h>
#include <utils/Mutex.h>
#include <vendor/mediatek/hardware/atci/1.0/IAtcid.h>
#include <vendor/mediatek/hardware/atci/1.0/IAtcidCommandHandler.h>
#include "atci_service.h"
#include "atci_service_adaptation.h"

using namespace vendor::mediatek::hardware::atci::V1_0;
using ::android::Mutex;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::configureRpcThreadpool;
using ::android::hardware::hidl_death_recipient;
using ::android::hardware::hidl_string;
using ::android::hidl::base::V1_0::IBase;
using ::android::sp;
using ::android::wp;

class AtciServiceAdaptation {
private:
    sp<IAtcid> mAtcid;
    sp<IAtcidCommandHandler> mAtcidCommandHandler;
    sp<hidl_death_recipient> mDeathRecipient;
    int mFd = -1;
    Mutex mLock;
    static AtciServiceAdaptation* sInstance;

public:
    static AtciServiceAdaptation *getInstance();
    void setSocketFd(int fd) { mFd = fd; }
    void sendCommandToAtciService(const char *data, int size);
    bool sendResponseToAtcid(const char *data, int size);
    void serviceDied();
    void setUp();

private:
    AtciServiceAdaptation();
};

class AtcidCommandHandler : public IAtcidCommandHandler {
private:
    AtciServiceAdaptation *mAtciService;

public:
    explicit AtcidCommandHandler(AtciServiceAdaptation *atci) : mAtciService(atci) {}

    virtual Return<void> sendCommand(const hidl_string& data) override {
        const char *str = data.c_str();
        ALOGD("sendCommand %s", str);
        mAtciService->sendCommandToAtciService((const char *)str, data.size());
        return Void();
    }
};

class DeathRecipient : public hidl_death_recipient {
private:
    AtciServiceAdaptation *mAtciService;

public:
    explicit DeathRecipient(AtciServiceAdaptation *atci) : mAtciService(atci) {}

    virtual void serviceDied(uint64_t cookie, const wp<IBase>& who) override {
        UNUSED(cookie);
        UNUSED(who);
        ALOGD("serviceDied");
        mAtciService->serviceDied();
    }
};

AtciServiceAdaptation* AtciServiceAdaptation::sInstance = NULL;

AtciServiceAdaptation* AtciServiceAdaptation::getInstance() {
    if (sInstance == NULL) {
        sInstance = new AtciServiceAdaptation();
    }
    return sInstance;
}

AtciServiceAdaptation::AtciServiceAdaptation() {
    mAtcidCommandHandler = new AtcidCommandHandler(this);
    mDeathRecipient = new DeathRecipient(this);
}

void AtciServiceAdaptation::setUp() {
    mAtcid = IAtcid::getService("default");
    if (mAtcid != NULL) {
        mAtcid->setCommandHandler(mAtcidCommandHandler);
        mAtcid->linkToDeath(mDeathRecipient, 0);
    } else {
        ALOGD("setUp error");
    }
}

void AtciServiceAdaptation::serviceDied() {
    Mutex::Autolock autoLock(mLock);
    mAtcid = NULL;
    setUp();
}

bool AtciServiceAdaptation::sendResponseToAtcid(const char *data, int size) {
    ALOGD("sendResponseToAtcid %s", data);
    Mutex::Autolock autoLock(mLock);
    if (mAtcid == NULL) {
        setUp();
    }
    if (mAtcid != NULL) {
        hidl_string str;
        str.setToExternal(data, size);
        Return<void> ret = mAtcid->sendCommandResponse(data);
        if (ret.isOk()) {
            return true;
        }
    }
    return false;
}

void AtciServiceAdaptation::sendCommandToAtciService(const char *data, int size) {
    ALOGD("sendCommandToAtciService %s", data);
    Mutex::Autolock autoLock(mLock);
    if (data != NULL) {
        int sendLen = send(mFd, data, size, 0);
        ALOGD("sendLen = %d", sendLen);
    }
}

void setSocketFdForAdaptation(int fd) {
    AtciServiceAdaptation::getInstance()->setUp();
    AtciServiceAdaptation::getInstance()->setSocketFd(fd);
}

bool sendResponseToAtcid(const char *data, int size) {
    return AtciServiceAdaptation::getInstance()->sendResponseToAtcid(data, size);
}
