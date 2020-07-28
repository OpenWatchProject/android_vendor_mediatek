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

#include "CoreSession.h"
#include "Mdmi_utils.h"
#include <stdio.h>
#include <inttypes.h>

const static char *TAG = "MDMI-CoreSession";

static char gObjName[OID_NAME_LEN_MAX + 1];

// Static member prototype
SessionInfo_t CoreSession::sSessionInfo[MDMI_LIB_MAX];
CoreSessionId CoreSession::sCoreSessionId;
sp<IDmcService> CoreSession::s_pDmcConnection;
sp<IDmcEventCallback> CoreSession::s_pDmcEventCallback;

CoreSession::CoreSession() {
    resetDmcConnection();

    for (int libType = MDMI_LIB_BEGIN; libType < MDMI_LIB_MAX; libType++) {
        sSessionInfo[libType].sessionId = INVALID_SESSION_ID;
        sSessionInfo[libType].callback = NULL;
        sSessionInfo[libType].param = NULL;
    }
}

CoreSession::~CoreSession() {
    resetDmcConnection();
}
void CoreSession::resetDmcConnection() {
    sCoreSessionId = INVALID_SESSION_ID;
    s_pDmcConnection = nullptr;
    s_pDmcEventCallback = nullptr;
}

void CoreSession::sessionCallback(const MdmiObjectName* eventName, const MdmiValue* eventValue, void* param, MDMI_LIB_TYPE libType) {
    CoreSession *me = (CoreSession *)param;

    if (me->sSessionInfo[libType].callback != NULL) {
        me->sSessionInfo[libType].callback(
                CoreSession::sCoreSessionId,
                eventName, eventValue,
                me->getSessionParam(libType));
    }
}

MdmiErrorCode CoreSession::createCoreSession(void) {
    dmcResult_e result = dmcResult_e::DMC_FAILED;
    dmcSessionConfig_t sessionConfig = {0};

    if (!tryGetDmcService("CreateSession")) {
        return MDMI_ERROR_GENERIC;
    }

    sessionConfig.identity = MDMI_IDENTITY;
    sessionConfig.version = MDMI_VERSION;
    sessionConfig.decryptPacket = false;

    auto check = s_pDmcConnection->createSession(sessionConfig, [&](auto ret, auto session) {
        sCoreSessionId = session;
        result = ret;
    });
    if (!check.isOk()) {
        MDMI_LOGE(TAG, "DMC HIDL call(createSession) failed!");
        resetDmcConnection();
        return MDMI_ERROR_GENERIC;
    }

    if (sCoreSessionId > 0 && result == dmcResult_e::DMC_SUCCESS) {
        if (s_pDmcEventCallback == nullptr) {
            s_pDmcEventCallback = new DmcEventCallback();
            auto check = s_pDmcConnection->registerEventCallback(sCoreSessionId, s_pDmcEventCallback);
            if (!check.isOk()) {
                MDMI_LOGE(TAG, "DMC HIDL call(registerEventCallback) failed!");
                resetDmcConnection();
                return MDMI_ERROR_GENERIC;
            }
            result = (dmcResult_e)check;
            if (result != dmcResult_e::DMC_SUCCESS) {
                MDMI_LOGE(TAG, "registerEventCallback() failed, result = %d", result);
                closeCoreSession();
            } else {
                MDMI_LOGD(TAG, "registerEventCallback() success with SID = %d", sCoreSessionId);
            }
        }
        dmcObjectId_t dummyOid = {0};
        auto check = s_pDmcConnection->invokeInt(
                sCoreSessionId, dmcCommand_e::DMC_CMD_START_REPORT_EVENT, dummyOid, 0);
        if (!check.isOk()) {
            MDMI_LOGE(TAG, "DMC HIDL call(invokeInt %d) failed!",
                    dmcCommand_e::DMC_CMD_START_REPORT_EVENT);
            resetDmcConnection();
            return MDMI_ERROR_GENERIC;
        }
        result = (dmcResult_e)check;
        if (result != dmcResult_e::DMC_SUCCESS) {
            MDMI_LOGE(TAG, "invoke(DMC_CMD_START_REPORT_EVENT) failed with SID = %d, result = %d",
                    sCoreSessionId, result);
            closeCoreSession();
        }
        sp<DmcDeathRecipient> pDmcRecipient = new DmcDeathRecipient();
        s_pDmcConnection->linkToDeath(pDmcRecipient, sCoreSessionId);
    } else {
        MDMI_LOGE(TAG, "createSession() failed! SID = %d, result = %d", sCoreSessionId, result);
        return MDMI_ERROR_GENERIC;
    }

    MDMI_LOGI(TAG, "createCoreSession() for SID = %d successfully!", sCoreSessionId);

    return MDMI_NO_ERROR;
}

MdmiErrorCode CoreSession::closeCoreSession(void) {
    if (!tryGetDmcService("CloseSession")) {
        resetDmcConnection();
        return MDMI_ERROR_GENERIC;
    }

    dmcResult_e ret = dmcResult_e::DMC_FAILED;
    dmcObjectId_t dummyOid = {0};
    auto check1 = CoreSession::s_pDmcConnection->invokeInt(
            CoreSession::sCoreSessionId, dmcCommand_e::DMC_CMD_STOP_REPORT_EVENT, dummyOid, 0);
    if (!check1.isOk()) {
        MDMI_LOGE(TAG, "DMC HIDL call(invokeInt %d) failed!",
                dmcCommand_e::DMC_CMD_STOP_REPORT_EVENT);
        resetDmcConnection();
        return MDMI_ERROR_GENERIC;
    }
    MDMI_LOGD(TAG, "Invoke(DMC_CMD_STOP_REPORT_EVENT) SID = %d, ret = %d", sCoreSessionId, ret);
    auto check2 = CoreSession::s_pDmcConnection->closeSession(sCoreSessionId);
    if (!check2.isOk()) {
        MDMI_LOGE(TAG, "DMC HIDL call(closeSession) failed!");
        resetDmcConnection();
        return MDMI_ERROR_GENERIC;
    }
    ret = (dmcResult_e)check2;

    MDMI_LOGD(TAG, "closeCoreSession() SID = %d, ret = %d", sCoreSessionId, ret);
    resetDmcConnection();

    return MDMI_NO_ERROR;
}

CoreSessionId CoreSession::getCoreSessionId(void) {
    if (sCoreSessionId == INVALID_SESSION_ID) {
        return MDMI_ERROR_NOT_INITIALIZED;
    }
    return sCoreSessionId;
}

LibSessionId CoreSession::mapLibSessionId(MDMI_LIB_TYPE libType) {
    return (libType + 1) * 1000 + 1;
}

MdmiErrorCode CoreSession::createLibSession(MDMI_LIB_TYPE libType, LibSessionId *sessionId) {
    if (sSessionInfo[libType].sessionId) {
        return MDMI_ERROR_TOO_MANY_SESSIONS;
    }

    *sessionId = sSessionInfo[libType].sessionId = mapLibSessionId(libType);
    MDMI_LOGD(TAG, "createLibSession() libType = %d, id = %d", libType, *sessionId);
    return MDMI_NO_ERROR;
}

MdmiErrorCode CoreSession::closeLibSession(MDMI_LIB_TYPE libType, LibSessionId sessionId) {
    if (sSessionInfo[libType].sessionId != sessionId) {
        return MDMI_ERROR_INVALID_SESSION;
    }
    sSessionInfo[libType].sessionId = INVALID_SESSION_ID;
    sSessionInfo[libType].callback = NULL;
    sSessionInfo[libType].param = NULL;
    MDMI_LOGD(TAG, "closeLibSession() libType = %d, id = %d", libType, sessionId);

    return MDMI_NO_ERROR;
}

int CoreSession::getCurrentLibSessionCount(void) {
    int count = 0;

    for (int libType = MDMI_LIB_BEGIN; libType < MDMI_LIB_MAX; libType++) {
        if (sSessionInfo[libType].sessionId == mapLibSessionId((MDMI_LIB_TYPE)libType)) {
            count++;
        }
    }
    return count;
}

bool CoreSession::isLibSessionValid(MDMI_LIB_TYPE libType, LibSessionId sessionId) {
    return (sSessionInfo[libType].sessionId == sessionId) ? true : false;
}

MdmiErrorCode CoreSession::subscribe(MDMI_LIB_TYPE libType, const MdmiObjectName* eventName) {
    if (!tryGetDmcService("Subscribe")) {
        return false;
    }

    dmcObjectId_t dmcOid = {0};
    objNameCoreToHidl(eventName, &dmcOid);
    auto check = s_pDmcConnection->subscribe(sCoreSessionId, dmcOid);
    if (!check.isOk()) {
        MDMI_LOGE(TAG, "DMC HIDL call(subscribe) failed!");
        resetDmcConnection();
        return MDMI_ERROR_GENERIC;
    }
    dmcResult_e ret = (dmcResult_e)check;

    getOidNameString(eventName, gObjName);
    MDMI_LOGD(TAG, "subscribe() SID = %d, OID = %s, libType = %d, ret = %d",
            sCoreSessionId, gObjName, libType, ret);

    return (ret != dmcResult_e::DMC_SUCCESS)? MDMI_ERROR_INVALID_OID : MDMI_NO_ERROR;
}

MdmiErrorCode CoreSession::unsubscribe(const MdmiObjectName* eventName) {
    if (!tryGetDmcService("Unsubscribe")) {
        return false;
    }

    dmcObjectId_t dmcOid = {0};
    objNameCoreToHidl(eventName, &dmcOid);
    auto check = s_pDmcConnection->unsubscribe(sCoreSessionId, dmcOid);
    if (!check.isOk()) {
        MDMI_LOGE(TAG, "DMC HIDL call(unsubscribe) failed!");
        resetDmcConnection();
        return MDMI_ERROR_GENERIC;
    }
    dmcResult_e ret = (dmcResult_e)check;

    getOidNameString(eventName, gObjName);
    MDMI_LOGD(TAG, "unsubscribe() SID = %d, OID = %s, ret = %d", sCoreSessionId, gObjName, ret);

    return (ret != dmcResult_e::DMC_SUCCESS)? MDMI_ERROR_INVALID_OID : MDMI_NO_ERROR;
}

void CoreSession::setSessionCallback(MDMI_LIB_TYPE libType, MdmiEventCallback callback, void *param) {
    if (callback == NULL) {
        MDMI_LOGD(TAG, "setSessionCallback() libType = %d");
    } else {
        MDMI_LOGD(TAG, "setSessionCallback(NULL) libType = %d");
    }

    sSessionInfo[libType].callback = callback;
    sSessionInfo[libType].param = param;
}

void *CoreSession::getSessionParam(MDMI_LIB_TYPE libType) {
    return sSessionInfo[libType].param;
}

MdmiErrorCode CoreSession::getLibSessionStats(MDMI_LIB_TYPE libType, MdmiSessionStats *statistics) {
    dmcResult_e result = dmcResult_e::DMC_FAILED;

    auto check = s_pDmcConnection->getSessionStats(sCoreSessionId, libType,
            [&](auto ret, dmcSessionStats_t stats){
        statistics->size = sizeof(MdmiSessionStats);
        statistics->totalEvents = stats.totalEvents;
        statistics->droppedEvents = stats.droppedEvents;
        statistics->eventBufferCapacity = stats.eventBufferCapacity;
        statistics->eventBufferInUse = stats.eventBufferInUse;
        result = ret;
    });
    if (!check.isOk()) {
        MDMI_LOGE(TAG, "DMC HIDL call(getLibSessionStats) failed!");
        resetDmcConnection();
        return MDMI_ERROR_GENERIC;
    }

    MDMI_LOGD(TAG, "Feed: %d Session: len: %u, Total/Dropped: %u/%u, Size/InUse: %u/%u",
            libType, statistics->size,
            statistics->totalEvents, statistics->droppedEvents,
            statistics->eventBufferCapacity, statistics->eventBufferInUse);

    return (result != dmcResult_e::DMC_SUCCESS)? MDMI_ERROR_GENERIC : MDMI_NO_ERROR;
}

// ==================== DMC Interface ========================
void DmcDeathRecipient::serviceDied(uint64_t cookie, const wp<IBase>& who) {
    UNUSED(who);
    CoreSession::sCoreSessionId = INVALID_SESSION_ID;
    CoreSession::s_pDmcEventCallback = nullptr;
    CoreSession::s_pDmcConnection = nullptr;

    MDMI_LOGE(TAG, "DMC Core died! SID = %" PRIu64, cookie);
}

Return<dmcResult_e> DmcEventCallback::onHandleEvent(
            CoreSessionId session, const dmcObjectId_t &objectId, const dmcValue_t &eventValue) {
    MdmiObjectName mdmiObjName = {0};
    mdmiObjName.length = objectId.length;
    mdmiObjName.ids = (MdmiTypeUInt32 *)calloc(objectId.length, sizeof(MdmiTypeUInt32));
    if (mdmiObjName.ids == NULL) {
        MDMI_LOGE(TAG, "calloc MdmiObjectName ids failed!");
        return dmcResult_e::DMC_FAILED;
    }
    CoreSession::objNameHidlToCore(&objectId, &mdmiObjName);
    CoreSession::getOidNameString(&mdmiObjName, gObjName);

    // Notify MDMI callback
    unsigned int length = eventValue.length;
    const uint8_t *data = eventValue.data.data();
    MDMI_LIB_TYPE libType = (MDMI_LIB_TYPE)objectId.group;
    LibSessionId libSessionId = 0;
    MdmiValue mdmiValue = {0};

    if (libType == INVALID_GROUP_ID || libType < 0) {
        MDMI_LOGE(TAG, "Invalid libType = %d", libType);
        goto errorAndFree;
    }

    mdmiValue.length = length;
    mdmiValue.data = (MdmiTypeUInt8 *)calloc(length, sizeof(MdmiTypeUInt8));
    if (mdmiValue.data == NULL) {
        MDMI_LOGE(TAG, "calloc mdmiValue data failed!");
        goto errorAndFree;
    }
    for (unsigned int i = 0; i < length; i++) {
        mdmiValue.data[i] = (MdmiTypeUInt8)eventValue.data[i];
    }

    libSessionId = CoreSession::sSessionInfo[libType].sessionId;
    MDMI_LOGD(TAG, "onHandleEvent() Core SID = %d, libType = %d, Lib SID = %d, OID = %s, datalen = %d",
            session, libType, libSessionId, gObjName, length);

    if (CoreSession::sSessionInfo[libType].callback == NULL) {
        MDMI_LOGE(TAG, "Null callback for libType = %d", libType);
        goto errorAndFree;
    }

    CoreSession::sSessionInfo[libType].callback(
            (MdmiSession)libSessionId,
            (const MdmiObjectName *)&mdmiObjName,
            (const MdmiValue *)&mdmiValue,
            (void *)CoreSession::sSessionInfo[libType].param);

    if (PRINT_OUTPUT_BUFFER) {
        CoreSession::printKpiBuf(gObjName, data, length);
    }

    free(mdmiValue.data);
    free(mdmiObjName.ids);

    return dmcResult_e::DMC_SUCCESS;

errorAndFree:
    if (mdmiValue.data) free(mdmiValue.data);
    if (mdmiObjName.ids) free(mdmiObjName.ids);

    return dmcResult_e::DMC_FAILED;
}

bool CoreSession::tryGetDmcService(const char *caller) {
    CoreSession::s_pDmcConnection = IDmcService::getService(DMC_HIDL_SERVER_NAME);
    if (CoreSession::s_pDmcConnection == nullptr) {
        MDMI_LOGE(TAG, "Failed to get IDmcService service, caller = %s", caller);
        return false;
    }

    MDMI_LOGD(TAG, "Found IDmcService service, caller = %s", caller);
    return true;
}

void CoreSession::objNameCoreToHidl(const MdmiObjectName *oidIn, dmcObjectId_t *oidOut) {
    oidOut->length = oidIn->length;
    oidOut->ids.setToExternal((uint32_t *)oidIn->ids, oidIn->length);
}

void CoreSession::objNameHidlToCore(const dmcObjectId_t *oidIn, MdmiObjectName *oidOut) {
    unsigned int len = oidIn->length;
    std::vector<uint32_t> idsVec = oidIn->ids;
    if (oidOut->ids == NULL) {
        // Allocate memory by the caller
        return;
    }
    oidOut->length = len;
    for (unsigned int i = 0; i < len; i++) {
       oidOut->ids[i] = (MdmiTypeUInt32)idsVec[i];
    }
}

void CoreSession::printKpiBuf(const char *prefix, const unsigned char *buf, unsigned int buf_len) {
    char *result = NULL;

    printBufHex(buf, buf_len, &result);
    MDMI_LOGD(TAG, "buff[%s] = %s", prefix, result);
    free(result);
}

void CoreSession::printBufHex(const unsigned char *bin, unsigned int binsz, char **result) {
    char hex_str[]= "0123456789ABCDEF";
    unsigned int i;

    *result = (char *)calloc(binsz * 2 + 1, sizeof(char));
    if (*result == NULL) {
        MDMI_LOGE(TAG, "printBufHex() calloc failed!");
        return;
    }

    (*result)[binsz * 2] = 0;

    if (!binsz) {
        return;
    }

    for (i = 0; i < binsz; i++) {
        (*result)[i * 2 + 0] = hex_str[(bin[i] >> 4) & 0x0F];
        (*result)[i * 2 + 1] = hex_str[(bin[i]     ) & 0x0F];
    }
}

void CoreSession::getOidNameString(const MdmiObjectName *eventName, char *result) {
    // Reset the buffer first
    memset(result, 0, OID_NAME_LEN_MAX);

    unsigned int len = eventName->length;

    // -1 for end char
    unsigned int remain_len = OID_NAME_LEN_MAX - 1;
    char id[10] = {0};

    for (unsigned int i = 0; i < len; i++) {
        snprintf(id, sizeof(id), "%d", eventName->ids[i]);
        unsigned int id_len = strlen(id);

        if (len > 1 && i > 0) {
            if (remain_len > 1) {
                strncat(result, ".", remain_len);
                remain_len -= 1;
            } else {
                remain_len = 0;
                return;
            }
        }

        if (remain_len > id_len) {
            strncat(result, id, remain_len);
            remain_len -= id_len;
        } else {
            remain_len = 0;
            return;
        }
    }
}
