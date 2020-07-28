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

#include "ApmSession.h"
#include "apm_msg_defs.h"
#include "Logs.h"

#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <cutils/properties.h>

using vendor::mediatek::hardware::apmonitor::V2_0::apmKpiHeader_t;
using android::hardware::hidl_death_recipient;

static const char* TAG = "APM-SessionN";

// Single instance for a process
ApmSession *g_pSession = NULL;

// Listen to DMC service status
struct ApmDeathRecipient : public hidl_death_recipient {
    void serviceDied(uint64_t cookie, const wp<IBase>& who);
};

void ApmDeathRecipient::serviceDied(uint64_t cookie, const wp<IBase>& who) {
    UNUSED(who);
    if (g_pSession != NULL) {
        APM_LOGD(TAG, "APM service died with SID = %" PRIu64", reset!", cookie);
        g_pSession->reset();
    }
}
static android::sp<ApmDeathRecipient> g_ApmDeathRecipient = nullptr;


ApmSession::ApmSession(): mSessionId(INVALID_SESSION_ID), mSessionParam(0),
        m_pConnectionBase_V2_0(nullptr), m_pControlCallback(nullptr),
        mIsServiceReady(false), mIsSubscriptionStateInitialized(false) {
    APM_LOGD(TAG, "constructor");
    pthread_mutex_init(&mMutex, NULL);
}

ApmSession::~ApmSession() {
    disconnect("destructor");
    pthread_mutex_destroy(&mMutex);
}

bool ApmSession::shouldSubmitKpi(APM_MSGID msgId) {
    // TODO: Check DMC session created or subscribed any APM KPIs
    //pthread_mutex_lock(&mMutex);
    APM_MSGID msgIdToCheck = (APM_MSGID)msgId;
    std::vector<APM_MSGID>::iterator itor =
            std::find(mSubscribedMsgList.begin(), mSubscribedMsgList.end(), msgIdToCheck);

    bool ret = (itor != mSubscribedMsgList.end())? true: false;
    //pthread_mutex_unlock(&mMutex);

    return ret;
}

bool ApmSession::isSubscriptionStateInitialized(void) {
    return mIsSubscriptionStateInitialized;
}

bool ApmSession::isApmServiceReady(void) {
    bool ret = false;
    //pthread_mutex_lock(&mMutex);
    if (mIsServiceReady && m_pConnectionBase_V2_0 != nullptr) {
        ret = true;
    }
    //pthread_mutex_unlock(&mMutex);
    return ret;
}

bool ApmSession::isApmServiceActive() {
    char buffer[PROPERTY_VALUE_MAX] = {'\0'};
    property_get(APM_SERVICE_ACTIVE_PROP, buffer, "0");
    int activated = atoi(buffer);
    return (activated != 1) ? false : true;
}

int64_t ApmSession::getSessionParam() {
    return mSessionParam;
}

bool ApmSession::connect() {
    //pthread_mutex_lock(&mMutex);

    // Active when at least APM message being subscribed
    if (!isApmServiceActive()) {
        // TODO; Remove me
        // APM_LOGD(TAG, "ApmService not in active state, skip~");
        return false;
    }

    if (isApmServiceReady()) {
        //pthread_mutex_unlock(&mMutex);
        return true;
    }

    // Get DMC HIDL service from base version V2_0
    m_pConnectionBase_V2_0 = V2_0::IApmService::getService(APM_HIDL_SERVER);

    if (m_pConnectionBase_V2_0 == nullptr) {
        APM_LOGE(TAG, "getService(IApmService) failed!");
        //pthread_mutex_unlock(&mMutex);
        return false;
    }

    // Create a DMC client session
    apmResult_e result = apmResult_e::APM_FAILED;

    if (INVALID_SESSION_ID == mSessionId) {
        m_pConnectionBase_V2_0->createSession([&](auto ret, auto sessionId, auto param) {
                    mSessionId = sessionId;
                    mSessionParam = param;
                    result = ret;
                    APM_LOGD(TAG, "createSession() SID = %d, param = %" PRId64", ret = %d",
                            mSessionId, param, ret);
        });
        if (INVALID_SESSION_ID == mSessionId) {
            APM_LOGE(TAG, "createSession() failed! SID = %d, ret = %d", mSessionId, result);
            //pthread_mutex_unlock(&mMutex);
            return false;
        }
    } else {
        APM_LOGD(TAG, "Found session with SID = %d", mSessionId);
    }

    // Get subscribed APM messages
    m_pConnectionBase_V2_0->getSubscribedMessages(mSessionId, [&](auto ret, auto subscribedMessages) {
                mSubscribedMsgList = subscribedMessages;
                result = ret;
                APM_LOGD(TAG, "[%d]getSubscribedMessages(), ret = %d", mSessionId, ret);
                dumpSubscribedMsgList("getSubscribedMessages");
    });
    if (apmResult_e::APM_SUCCESS != result) {
        APM_LOGE(TAG, "[%d]getSubscribedMessages() failed!, ret = %d", mSessionId, result);
        m_pConnectionBase_V2_0->closeSession(mSessionId);
        mSessionId = INVALID_SESSION_ID;
        mSessionParam = 0;
        //pthread_mutex_unlock(&mMutex);
        return false;
    }
    mIsSubscriptionStateInitialized = true;

    // Death notify for DMC HIDL server
    if (g_ApmDeathRecipient == nullptr) {
        g_ApmDeathRecipient = new ApmDeathRecipient();
        m_pConnectionBase_V2_0->linkToDeath(g_ApmDeathRecipient, mSessionId);
    }

    // Register APM control callback
    if (m_pControlCallback == nullptr) {
        m_pControlCallback = new ApmControlCallback();
        m_pControlCallback->registerUpdateKpiSubStateListener(
                onUpdateKpiSubStateListener, (void *)this);
        m_pControlCallback->registerQueryKpiListener(
                onQueryKpiListener, (void *)this);
    }

    APM_LOGD(TAG, "[%d]registerControlCallback()", mSessionId);
    m_pConnectionBase_V2_0->registerControlCallback(mSessionId, m_pControlCallback);

    mIsServiceReady = true;
    APM_LOGD(TAG, "[%d]connect IApmService successfully!", mSessionId);

    //pthread_mutex_unlock(&mMutex);

    return true;
}

void ApmSession::disconnect(const char *cause) {
    APM_LOGD(TAG, "[%d]APM session disconnect(%s)", mSessionId, cause);

    //pthread_mutex_lock(&mMutex);
    reset();
    //pthread_mutex_unlock(&mMutex);
}

uint64_t ApmSession::getCurrentTimestamp(void) {
    uint64_t timestampMs = 0;
    //pthread_mutex_lock(&mMutex);
    if (m_pConnectionBase_V2_0 == NULL) {
        APM_LOGE(TAG, "[%d]getCurrentTimestamp: no server connection", mSessionId);
        //pthread_mutex_unlock(&mMutex);
        return 0;
    }
    timestampMs = m_pConnectionBase_V2_0->getCurrentTimestamp();
    //pthread_mutex_unlock(&mMutex);
    return timestampMs;
}

bool ApmSession::submitKpi(APM_MSGID msgId, APM_SIMID simId, timeStamp timestampMs,
        uint32_t len, const void *data) {
    APM_LOGD(TAG, "[%d]submitKpi msgId = %d, simId = %d, len = %" PRIu32, mSessionId, msgId, simId, len);

    //pthread_mutex_lock(&mMutex);
    if (m_pConnectionBase_V2_0 == NULL) {
        APM_LOGE(TAG, "[%d]submitKpi: no server connection", mSessionId);
        //pthread_mutex_unlock(&mMutex);
        return false;
    }

    apmKpiHeader_t kpiHeader = {.msgId = 0, .simId = 0, .timestampMs = 0};
    kpiHeader.msgId = msgId;
    kpiHeader.simId = simId;
    kpiHeader.timestampMs = timestampMs;

    hidl_vec<uint8_t> kpiData;
    kpiData.setToExternal((uint8_t *)data, len);
    m_pConnectionBase_V2_0->submitRawEvent(mSessionId, kpiHeader, len, kpiData);

    //pthread_mutex_unlock(&mMutex);
    return false;
}

void ApmSession::onUpdateKpiSubStateListener(const std::vector<APM_MSGID> &subscribedMessages, void *param) {
    ApmSession *me = (ApmSession *)param;
    //pthread_mutex_lock(&me->mMutex);
    me->mSubscribedMsgList = subscribedMessages;
    me->dumpSubscribedMsgList("onUpdateKpiSubStateListener");
    //pthread_mutex_unlock(&me->mMutex);
}

void ApmSession::onQueryKpiListener(APM_MSGID msgId, void *param) {
    UNUSED(msgId);
    UNUSED(param);

    //(&me->mMutex);
    // TODO: notify submission layer to submit APM KPI
    //pthread_mutex_unlock(&me->mMutex);
}

void ApmSession::reset(void) {
    mSubscribedMsgList.clear();
    mIsSubscriptionStateInitialized = false;
    mIsServiceReady = false;
    mSessionId = INVALID_SESSION_ID;
    m_pConnectionBase_V2_0 = nullptr;
    mSessionParam = 0;

    if (g_ApmDeathRecipient != nullptr) {
        g_ApmDeathRecipient = nullptr;
    }
    if (m_pControlCallback != nullptr) {
        m_pControlCallback->registerUpdateKpiSubStateListener(NULL, NULL);
        m_pControlCallback->registerQueryKpiListener(NULL, NULL);
        m_pControlCallback = nullptr;
    }
}

void ApmSession::dumpSubscribedMsgList(const char *cause) {
    //pthread_mutex_lock(&mMutex);
    if (mSubscribedMsgList.size() == 0) {
        APM_LOGD(TAG, "[%d]dumpSubscribedMsgList(%s): No APM KPI being subscribed!", mSessionId, cause);
    } else {
        char *out = NULL;
        appendString(&out, "[%d]dumpSubscribedMsgList(%s): {", mSessionId, cause);
        for (unsigned int i = 0; i < mSubscribedMsgList.size(); i++) {
            if (i == 0) {
                appendString(&out, "%" PRIu16, mSubscribedMsgList[i]);
            } else {
                appendString(&out, ", %" PRIu16, mSubscribedMsgList[i]);
            }
        }
        appendString(&out, "}");
        if (out != NULL) {
            APM_LOGD(TAG, "%s", out);
            free(out);
        }
    }
    //pthread_mutex_unlock(&mMutex);
}

void ApmSession::appendString(char **outStr, const char *format, ...) {
    char *inputStr = NULL;
    char *oldStr = NULL;
    char *newStr = NULL;
    unsigned int len = 0;

    va_list arg_ptr;
    va_start(arg_ptr, format);
    vasprintf(&inputStr, format, arg_ptr);
    if (inputStr == NULL) {
        goto FreeAndReturn;
    }

    // save old string
    asprintf(&oldStr, "%s", (*outStr == NULL ? "" : *outStr));
    if (oldStr == NULL) {
        goto FreeAndReturn;
    }

    // calloc new string memory
    len = strlen(oldStr) + strlen(inputStr);
    newStr = (char *)calloc(len + 1, sizeof(char));
    if (newStr == NULL) {
         goto FreeAndReturn;
    }
    strncat(newStr, oldStr, len);
    strncat(newStr, inputStr, len - strlen(oldStr));

    if (*outStr) {
        free(*outStr);
    }
    *outStr = newStr;

FreeAndReturn:
    if (oldStr != NULL) free(oldStr);
    if (inputStr != NULL) free(inputStr);
    va_end(arg_ptr);
}

