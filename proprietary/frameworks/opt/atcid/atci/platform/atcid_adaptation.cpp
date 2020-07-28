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

//#include <cutils/properties.h>
#include <hidl/HidlTransportSupport.h>
#include <hwbinder/IPCThreadState.h>
#include <hwbinder/ProcessState.h>
#include <utils/Mutex.h>
#include <vendor/mediatek/hardware/atci/1.0/IAtcid.h>
#include <vendor/mediatek/hardware/atci/1.0/IAtcidCommandHandler.h>
#include <vendor/mediatek/hardware/atci/1.0/IAtcidResponse.h>
#include "atcid.h"
#include "atcid_adaptation.h"

#define MAX_SLOT_NUM 4
#define RIL1_SERVICE_NAME "slot1"
#define RIL2_SERVICE_NAME "slot2"
#define RIL3_SERVICE_NAME "slot3"
#define RIL4_SERVICE_NAME "slot4"
#define NOMODEM_RSP "ERROR:No Modem"
#define NOUE_RSP "ERROR:UE is not ready"

using namespace vendor::mediatek::hardware::atci::V1_0;
using ::android::Mutex;
using ::android::hardware::Return;
using ::android::hardware::configureRpcThreadpool;
using ::android::hardware::hidl_death_recipient;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Void;
using ::android::hidl::base::V1_0::IBase;
using ::android::sp;
using ::android::wp;

class AtcidImpl : public vendor::mediatek::hardware::atci::V1_0::IAtcid {
private:
    std::vector<sp<IAtcidCommandHandler> > mAtcidCommandHandler;
    sp<IAtcidResponse> mAtcidResponse;
    int mFdAtciService = -1;
    int mFd = -1;
    static AtcidImpl* sInstance;

public:
    static AtcidImpl* getInstance();

    void setUp(int fd) { mFd = fd; }

    void setSocketFdForAtciService(int fd) { mFdAtciService = fd; }

    Return<void> setCommandHandler(const ::android::sp<IAtcidCommandHandler>& atciCommandHandler);

    Return<void> sendCommandResponse(const hidl_string& data);

    Return<void> setResponseFunction(const ::android::sp<IAtcidResponse>& atcidResponse);

    Return<void> sendCommand(const hidl_string& data);

    bool sendCommandToAtciService(const char *data, int size);

    bool sendResponse(const char *data, int size);
};

AtcidImpl* AtcidImpl::sInstance = NULL;

AtcidImpl* AtcidImpl::getInstance() {
    if (sInstance == NULL) {
        configureRpcThreadpool(2, false);
        sInstance = new AtcidImpl;
        android::status_t status = sInstance->registerAsService("default");
        LOGATCI(LOG_INFO, "registerAsService %d", status);
    }
    return sInstance;
}

Return<void> AtcidImpl::setCommandHandler(const ::android::sp<IAtcidCommandHandler>& atciCommandHandler) {
    LOGATCI(LOG_INFO, "setCommandHandler");
    mAtcidCommandHandler.push_back(atciCommandHandler);
    return Void();
}

/*hidle API for atci_service_sys and atciservice.apk*/
Return<void> AtcidImpl::sendCommandResponse(const hidl_string& data) {
    const char *str = data.c_str();
    int sendLen = 0;
    int len = 0;
    if (str != NULL) {
        len = strlen(str);
        sendLen = send(mFdAtciService, str, data.size(), 0);
        if (sendLen != len) {
            LOGATCI(LOG_ERR, "lose data when sendCommandResponse to atcid. errno = %d, sendLen = %d, len = %d", errno, sendLen, len);
        }
    }
    return Void();
}

//for swift tool only
Return<void> AtcidImpl::setResponseFunction(const ::android::sp<IAtcidResponse>& atcidResponse) {
    LOGATCI(LOG_INFO, "setResponseFunction");
    mAtcidResponse = atcidResponse;
    return Void();
}

//for swift tool only
Return<void> AtcidImpl::sendCommand(const hidl_string& data) {
    const char *str = data.c_str();
    int sendLen = 0;
    int len = 0;
    if (str != NULL) {
        len = strlen(str);
        sendLen = send(mFd, str, len, 0);
        if (sendLen != len) {
            LOGATCI(LOG_ERR, "lose data when sendCommand to atcid. errno = %d, sendLen = %d, len = %d", errno, sendLen, len);
        }
    }
    return Void();
}

bool AtcidImpl::sendCommandToAtciService(const char *data, int size) {
    if (mAtcidCommandHandler.size() > 0 && data != NULL) {
        LOGATCI(LOG_ERR, "sendCommandToAtciService mAtcidCommandHandler != null");
        hidl_string str;
        str.setToExternal(data, size);
        for (unsigned int i = 0; i < mAtcidCommandHandler.size(); i++) {
            Return<void> ret = mAtcidCommandHandler[i]->sendCommand(str);
            if (!ret.isOk()) {
                LOGATCI(LOG_ERR, "sendCommandToAtciService sendCommand error");
            }
        }
        return true;
    } else {
        LOGATCI(LOG_ERR, "sendCommandToAtciService mAtcidCommandHandler == null");
    }
    return false;
}

/*send response to swift tool*/
bool AtcidImpl::sendResponse(const char *data, int size) {
    if (mAtcidResponse != NULL && data != NULL) {
        LOGATCI(LOG_ERR, "sendResponse mAtcidResponse != null");
        hidl_string str;
        str.setToExternal(data, size);
        Return<void> ret = mAtcidResponse->sendCommandResponse(str);
        if (ret.isOk()) {
            return true;
        }
    } else {
        LOGATCI(LOG_ERR, "sendResponse mAtcidResponse == null");
    }
    return false;
}

void initAtcidHidlService(int fd) {
    AtcidImpl::getInstance()->setUp(fd);
}

void setSocketFdForAtciService(int fd) {
    AtcidImpl::getInstance()->setSocketFdForAtciService(fd);
}

bool sendCommandToSystemAtciService(const char *data, int size) {
    return AtcidImpl::getInstance()->sendCommandToAtciService(data, size);
}

/*when response receiver is swift tool*/
bool sendResponse(const char *data, int size) {
    return AtcidImpl::getInstance()->sendResponse(data, size);
}

int connectToSystemAtciService() {
    int fd[2] = {0};
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) >= 0) {
        setSocketFdForAtciService(fd[1]);
        return fd[0];
    } else {
        LOGATCI(LOG_ERR, "socketpair failed. errno:%d", errno);
        return -1;
    }
}
