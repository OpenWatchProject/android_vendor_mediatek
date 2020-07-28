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


#include "mdi_redirector.h"
#include "CommandProcessor.h"
#include "NetworkSocketManager.h"

//TODO: #include "ComPortManager.h"

#include <fcntl.h>
#include <inttypes.h>
#include <sys/types.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#define __STDC_FORMAT_MACROS   // use PRIu64 for printing uint64
#include <vector>
#include <memory>

#include <vendor/mediatek/hardware/dmc/1.1/IDmcService.h>

#undef TAG
#define TAG "MAPI-MdiRedirector"

using namespace vendor::mediatek::hardware::dmc;
using V1_1::IDmcService;
using V1_0::IDmcEventCallback;
using V1_0::dmcCommand_e;
using V1_0::dmcResult_e;
using V1_0::dmcSessionConfig_t;
using V1_0::dmcObjectId_t;
using V1_0::dmcValue_t;

using android::hardware::Return;
using android::hardware::hidl_death_recipient;
using android::hidl::base::V1_0::IBase;
using android::sp;
using android::wp;

typedef int32_t sessionId;

#define PRINT_OUTPUT_BUFFER 0

//
// ==================== Commands functions prototype ======================
//
bool EnableTrap(size_t argsLen, const uint8_t *pArgsBuffer);
bool DisableTrap(size_t argsLen, const uint8_t *pArgsBuffer);
bool SubscribeObject(size_t argsLen, const uint8_t *pArgsBuffer);
bool UnsubscribeObject(size_t argsLen, const uint8_t *pArgsBuffer);
bool QueryObject(size_t argsLen, const uint8_t *pArgsBuffer);
bool SetPayloadSize(size_t argsLen, const uint8_t *pArgsBuffer);
bool SetPacketType(size_t argsLen, const uint8_t *pArgsBuffer);

//
// ==================== Global variable ======================
//
volatile sig_atomic_t gbTerminate = false;
// Communication
static IoChannel *pChannel = NULL;
static sessionId g_sessionId = INVALID_DMC_SESSION_ID;
static android::sp<V1_0::IDmcService> g_pDmcConnection = nullptr;
static android::sp<IDmcEventCallback> g_pDmcEventCallback = nullptr;
static bool g_supportAsyncQuery = false;

// Listen to DMC service status
struct DmcDeathRecipient : public hidl_death_recipient {
    void serviceDied(uint64_t cookie, const wp<IBase>& who);
};

void DmcDeathRecipient::serviceDied(uint64_t cookie, const wp<IBase>& who) {
    UNUSED(who);
    g_sessionId = INVALID_DMC_SESSION_ID;
    g_pDmcEventCallback = nullptr;
    g_pDmcConnection = nullptr;
    MDI_LOGE(TAG, "DMC Core died! SID = %" PRIu64, cookie);
}

// The command code neet to align with mdi_redirector_ctrl.
const struct redirectorCmdEntry g_redirectorCmdTable[] = {
    {0, EnableTrap},
    {1, DisableTrap},
    {2, SubscribeObject},
    {3, UnsubscribeObject},
    {4, SetPayloadSize},
    {5, SetPacketType},
    {6, QueryObject}
};

static pthread_mutex_t gMutex;

// ==================== Internal Functions ======================
// V1.1
sp<V1_1::IDmcService> tryCastDmcService_V1_1(sp<V1_0::IDmcService> current) {
    return V1_1::IDmcService::castFrom(current);
}

void SigHandler(int value) {
    switch (value)
    {
    case SIGTERM:
        pthread_mutex_lock(&gMutex);
        gbTerminate = true;
        pthread_mutex_unlock(&gMutex);
        MDI_LOGI(TAG, "Receive SIGTERM signal [%d]", value);
        break;
    default:
        MDI_LOGD(TAG, "Receive signal [%d]", value);
        break;
    }
}

void SetupSigHandler() {
    struct sigaction actions;

    actions.sa_flags = 0;
    sigemptyset(&actions.sa_mask);
    actions.sa_handler = SigHandler;
    sigaction(SIGTERM, &actions, NULL);
}

void InitGlobalVariable() {
    // NULL now
    pthread_mutex_init(&gMutex, NULL);
}

void SetupCommandReceiver() {
    int cmdTableSize = sizeof(g_redirectorCmdTable)/sizeof(g_redirectorCmdTable[0]);
    static SocketListener socketListener(REDIRECTOR_COMMAND_RECEIVER_NAME);
    static CommandProcessor cmdProcessor(g_redirectorCmdTable, cmdTableSize);

    socketListener.SetConnectionHandler(&cmdProcessor);
}

static std::string GetStringAttributeStringValue(std::string &inputString, const char * attr) {
    if (attr == NULL) {
        return "";
    }
    std::size_t attri_pos = inputString.find(attr);
    if (attri_pos == std::string::npos) {
        return "";
    }
    std::size_t attri_value_spos = attri_pos + strlen(attr) + 1; // +1 = "="
    std::size_t attri_value_epos = inputString.find(" ", attri_value_spos ); // search next " "

    //MDI_LOGE(TAG, "GetStringAttributeStringValue(): search \"%s\" in \"%s\", value spos = %d, epos = %d.",attr,inputString.c_str(),attri_value_spos,attri_value_epos);

    if(attri_pos != std::string::npos &&
       attri_value_spos != std::string::npos &&
       attri_value_epos != std::string::npos )
    {
        std::string ret = inputString.substr(attri_value_spos, attri_value_epos - attri_value_spos);
        inputString.erase(0, attri_value_epos);
        return ret;
    }
    inputString.erase(0, attri_value_spos);
    return "";
}

static bool CheckStringIsDigital(const std::string &s) {
    std::string::const_iterator it = s.begin();
    while(it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

bool GetObjectIDFromStringBuffer(size_t argsLen, const uint8_t *pArgsBuffer, std::vector<ObjId> &ObjID) {
    // parse "OID="
    std::string argvString((const char *)pArgsBuffer, argsLen);

    while (STRING_ATTR_MATCH(argvString, "OID")) {
        std::string idString = GetStringAttributeStringValue(argvString, "OID");
        if (idString.length() == 0) {
            MDI_LOGE(TAG, "GetObjectIDFromStringBuffer() : oid string is NULL.");
            continue;
        }
        MDI_LOGD(TAG, "OID message : Setup OID string = %s.", idString.c_str());

        // set id & push to vector
        ObjId oid;
        oid.setIdString(idString.c_str());
        ObjID.push_back(oid);
    }
    if (ObjID.size() == 0) {
        MDI_LOGE(TAG, "OID message : Not found OID assigment.");
        return false;
    }
    else {
        MDI_LOGD(TAG, "OID message : %lu OID in assignment.", ObjID.size());
    }

    return true;
}

static void GetSimIdFromStringBuffer(size_t argsLen, const uint8_t *pArgsBuffer, std::vector<unsigned int> &objSize)
{
    // parse "max_raw_size="   (optional for TRAP_TYPE_IP)
    std::string argvString((const char *)pArgsBuffer, argsLen);

    while(STRING_ATTR_MATCH(argvString, "SIMID")) {
        std::string lenString = GetStringAttributeStringValue(argvString, "SIMID");

        if(lenString.length() != 0 && CheckStringIsDigital(lenString)) {
            unsigned int simId = atoi(lenString.c_str());
            objSize.push_back(simId);
        } else {
            objSize.push_back(0);
        }
    }

    if (objSize.size() == 0) {
        MDI_LOGE(TAG, "GetSimIdFromStringBuffer: No SIMID found");
    }

    return;  // default length
}

static void GetTrapMaxRawSizeFromStringBuffer(size_t argsLen, const uint8_t *pArgsBuffer,
        std::vector<unsigned int> &objSize) {
    // parse "max_raw_size="   (optional for TRAP_TYPE_IP)
    std::string argvString((const char *)pArgsBuffer, argsLen);

    while (STRING_ATTR_MATCH(argvString, "size")) {
        std::string lenString = GetStringAttributeStringValue(argvString, "size");

        if (lenString.length() != 0 && CheckStringIsDigital(lenString)) {
            unsigned int size = atoi(lenString.c_str());
            objSize.push_back(size);
        } else {
            unsigned int size = 128;
            objSize.push_back(size);
        }
    }

    if (objSize.size() == 0) {
        MDI_LOGE(TAG, "GetTrapMaxRawSizeFromStringBuffer: No size found");
    }

    return;  // default length
}

static bool GetPacketTypeFromStringBuffer(size_t argsLen,
        const uint8_t *pArgsBuffer, std::vector<unsigned int> &packetType) {
    // parse "type="
    std::string argvString((const char *)pArgsBuffer, argsLen);

    while (STRING_ATTR_MATCH(argvString, "type")) {
        std::string typeString = GetStringAttributeStringValue(argvString, "type");

        if (typeString.length() != 0 && CheckStringIsDigital(typeString)) {
            unsigned int type = atoi(typeString.c_str());
            packetType.push_back(type);
        } else {
            MDI_LOGE(TAG, "GetPacketTypeFromStringBuffer: Invalid type: %s", typeString.c_str());
            return false;
        }
    }

    if (packetType.size() == 0) {
        MDI_LOGE(TAG, "GetPacketTypeFromStringBuffer: No type found!");
        return false;
    }

    return true;
}

uint32_t GetIntFromBuffer(const uint8_t *data, int width) {
    uint32_t ret = 0;

    for(int i = width - 1 ; i >= 0 ; --i) {
        ret += *(data+i) & 0xFF;
        if(i != 0) {
            ret <<= 8;
        }
    }

    return ret;
}

bool WriteKpiObjectMessage(const uint8_t *buf, size_t len) {
    if (gbTerminate) {
        return false;
    }

    pthread_mutex_lock(&gMutex);

    size_t data_size = len;

    MDI_LOGI(TAG, "SendOneFrameWithSyncToken: obj data len = %zu", data_size);

    if (buf != NULL) {
        // Send Data
        if (!pChannel->Write(buf, data_size)) {
            MDI_LOGE(TAG, "Failed to write 'Data' to IoChannel.");
            pthread_mutex_unlock(&gMutex);
            return false;
        }

    } else {
        MDI_LOGI(TAG, "SendOneFrameWithSyncToken: ignore sending data");
    }
    pthread_mutex_unlock(&gMutex);

    MDI_LOGI(TAG, "SendOneFrameWithSyncToken: data write done");
    return true;
}

static void objIdCoreToHidl(const ObjId *oidIn, dmcObjectId_t *oidOut) {
    uint32_t *oid = oidIn->getObjId();
    oidOut->length = oidIn->getObjLength();
    oidOut->ids.setToExternal(oid, oidOut->length);
}

static void objIdHidlToCore(const dmcObjectId_t *oidIn, ObjId *oidOut) {
    unsigned int len = oidIn->length;
    std::vector<uint32_t> idsVec = oidIn->ids;
    oidOut->setIdsVec(len, idsVec);
}

static void printBufHex(const unsigned char *bin, unsigned int binsz, char **result) {
    char hex_str[]= "0123456789ABCDEF";
    unsigned int i;

    *result = (char *)calloc(binsz * 2 + 1, sizeof(char));
    if (*result == NULL) {
        MDI_LOGE(TAG, "printBufHex() calloc failed!");
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

static void printBuf(unsigned char *prefix, const unsigned char *buf, unsigned int buf_len) {
    char *result = NULL;

    printBufHex(buf, buf_len, &result);
    MDI_LOGE(TAG, "buff[%s] = %s", prefix, result);
    free(result);
}

static bool tryGetDmcService(const char *caller) {
    g_pDmcConnection = V1_0::IDmcService::getService(DMC_HIDL_SERVER_NAME);
    if (g_pDmcConnection == nullptr) {
        MDI_LOGE(TAG, "Failed to get IDmcService service, caller = %s", caller);
        return false;
    }

    sp<V1_1::IDmcService> service = tryCastDmcService_V1_1(g_pDmcConnection);
    if (service != nullptr) {
        MDI_LOGD(TAG, "Found IDmcService service(V1_1), caller = %s", caller);
        g_supportAsyncQuery = true;
    } else {
        MDI_LOGD(TAG, "Found IDmcService service(V1_0), caller = %s", caller);
        g_supportAsyncQuery = false;
    }

    return true;
}

class DmcEventCallback : public IDmcEventCallback {
    Return<dmcResult_e> onHandleEvent(
            sessionId session, const dmcObjectId_t &objectId, const dmcValue_t &eventValue) {
        ObjId *oid = new ObjId();
        if (oid == NULL) {
            MDI_LOGE(TAG, "new ObjId failed!");
            return dmcResult_e::DMC_FAILED;
        }
        objIdHidlToCore(&objectId, oid);

        unsigned int length = eventValue.length;
        const uint8_t *data = eventValue.data.data();

        bool ret = WriteKpiObjectMessage(data, length);
        MDI_LOGD(TAG, "onHandleEvent() SID = %d, OID = %s, datalen = %d, ret = %d",
                session, oid->getOidString(), length, ret);

        if (PRINT_OUTPUT_BUFFER) {
            printBuf((unsigned char *)oid->getOidString(), data, length);
        }
        delete oid;

        if (!ret) {
            return dmcResult_e::DMC_FAILED;
        }
        return dmcResult_e::DMC_SUCCESS;
    }
};

// ==================== Public Functions ========================
void Reset() {
    g_sessionId = INVALID_DMC_SESSION_ID;
    g_pDmcConnection = nullptr;
    g_pDmcEventCallback = nullptr;
}
bool EnableTrap(size_t argsLen, const uint8_t *pArgsBuffer) {
    UNUSED(argsLen);
    UNUSED(pArgsBuffer);

    dmcResult_e result = dmcResult_e::DMC_FAILED;
    dmcSessionConfig_t sessionConfig;

    MDI_LOGD(TAG, "EnableTrap()");

    if (!tryGetDmcService("EnableTrap")) {
        return false;
    }

    sessionConfig.identity = MAPI_IDENTITY;
    sessionConfig.version = MAPI_VERSION;
    sessionConfig.decryptPacket = false;
    auto check = g_pDmcConnection->createSession(sessionConfig, [&](auto ret, auto session) {
        g_sessionId = session;
        result = ret;
        MDI_LOGD(TAG, "createSession() SID = %d, ret = %d", g_sessionId, ret);
    });
    if (!check.isOk()) {
        MDI_LOGE(TAG, "DMC HIDL call(createSession) failed!");
        Reset();
        return false;
    }

    if (g_sessionId > INVALID_DMC_SESSION_ID && result == dmcResult_e::DMC_SUCCESS) {
        // TODO: Fix memory leak
        if (g_pDmcEventCallback == nullptr) {
            android::sp<IDmcEventCallback> g_pDmcEventCallback = new DmcEventCallback();
            auto check = g_pDmcConnection->registerEventCallback(g_sessionId, g_pDmcEventCallback);
            if (!check.isOk()) {
                MDI_LOGE(TAG, "DMC HIDL call(registerEventCallback) failed!");
                Reset();
                return false;
            }
            result = (dmcResult_e)check;
            if (result != dmcResult_e::DMC_SUCCESS) {
                MDI_LOGE(TAG,"registerEventCallback() failed, result = %d", result);
                DisableTrap(0, NULL);
            } else {
                MDI_LOGD(TAG,"registerEventCallback() success with SID = %d", g_sessionId);
            }
        }
        dmcObjectId_t dummyOid;
        auto check = g_pDmcConnection->invokeInt(
                g_sessionId, dmcCommand_e::DMC_CMD_START_REPORT_EVENT, dummyOid, 0);
        if (!check.isOk()) {
            MDI_LOGE(TAG, "DMC HIDL call(invokeInt %d) failed!",
                    dmcCommand_e::DMC_CMD_START_REPORT_EVENT);
            Reset();
            return false;
        }
        result = (dmcResult_e)check;
        if (result != dmcResult_e::DMC_SUCCESS) {
            MDI_LOGE(TAG,"invoke(DMC_CMD_START_REPORT_EVENT) failed with SID = %d, result = %d",
                    g_sessionId, result);
            DisableTrap(0, NULL);
        }
        sp<DmcDeathRecipient> pDmcRecipient = new DmcDeathRecipient();
        g_pDmcConnection->linkToDeath(pDmcRecipient, g_sessionId);
    } else {
        MDI_LOGE(TAG,"createSession() failed! SID = %d, result = %d", g_sessionId, result);
        Reset();
        return false;
    }

    MDI_LOGI(TAG, "Traps has been enabled for SID = %d", g_sessionId);
    return true;
}

bool DisableTrap(size_t argsLen, const uint8_t *pArgsBuffer) {
    UNUSED(argsLen);
    UNUSED(pArgsBuffer);

    if (!tryGetDmcService("DisableTrap")) {
        Reset();
        return false;
    }

    dmcResult_e ret = dmcResult_e::DMC_FAILED;
    dmcObjectId_t dummyOid;
    auto check1 = g_pDmcConnection->invokeInt(
            g_sessionId, dmcCommand_e::DMC_CMD_STOP_REPORT_EVENT, dummyOid, 0);
    if (!check1.isOk()) {
        MDI_LOGE(TAG, "DMC HIDL call(invokeInt %d) failed!",
                dmcCommand_e::DMC_CMD_STOP_REPORT_EVENT);
        Reset();
        return false;
    }
    MDI_LOGD(TAG, "Invoke(DMC_CMD_STOP_REPORT_EVENT) SID = %d, ret = %d", g_sessionId, ret);
    auto check2 = g_pDmcConnection->closeSession(g_sessionId);
    if (!check2.isOk()) {
        MDI_LOGE(TAG, "DMC HIDL call(closeSession) failed!");
        Reset();
        return false;
    }
    ret = (dmcResult_e)check2;
    MDI_LOGD(TAG, "closeSession() SID = %d, ret = %d", g_sessionId, ret);

    MDI_LOGI(TAG, "Traps has been disabled");
    Reset();
    return true;
}

bool SubscribeObject(size_t argsLen, const uint8_t *pArgsBuffer) {
    if(argsLen == 0 || pArgsBuffer == NULL) {
        MDI_LOGE(TAG, "Failed to subscribe traps, arg is invalid. argsLen(%zu), pArgsBuffer(0x%08x)  ", argsLen, pArgsBuffer);
        return false;
    }
    std::string ArgString((const char *)pArgsBuffer,argsLen);  // for debug
    //MDI_LOGE(TAG, "SubscribeTrap(): args = \"%s\".", ArgString.c_str());
    std::vector<ObjId> objID;
    GetObjectIDFromStringBuffer(argsLen,pArgsBuffer,objID);

    if (!tryGetDmcService("SubscribeObject")) {
        return false;
    }

    for (size_t i = 0 ; i < objID.size(); ++i) {
        dmcObjectId_t dmcOid = {0};
        objIdCoreToHidl(&objID[i], &dmcOid);
        auto check = g_pDmcConnection->subscribe(g_sessionId, dmcOid);
        if (!check.isOk()) {
            MDI_LOGE(TAG, "DMC HIDL call(subscribe) failed!");
            Reset();
            return false;
        }
        dmcResult_e ret = (dmcResult_e)check;
        MDI_LOGD(TAG, "subscribe SID=%d, OID=%s, ret=%d", g_sessionId, objID[i].getOidString(), ret);
    }

    return true;
}

bool UnsubscribeObject(size_t argsLen, const uint8_t *pArgsBuffer) {
    if(argsLen == 0 || pArgsBuffer == NULL) {
        MDI_LOGE(TAG, "Failed to subscribe traps, arg is invalid. argsLen(%zu), pArgsBuffer(0x%08x)  ", argsLen, pArgsBuffer);
        return false;
    }

    if (!tryGetDmcService("UnsubscribeObject")) {
        return false;
    }

    std::string ArgString((const char *)pArgsBuffer,argsLen);  // for debug
    //MDI_LOGE(TAG, "SubscribeTrap(): args = \"%s\".", ArgString.c_str());
    std::vector<ObjId> objID;
    GetObjectIDFromStringBuffer(argsLen,pArgsBuffer,objID);

    for (size_t i = 0 ; i < objID.size() ; ++i) {
        dmcObjectId_t dmcOid = {0};
        objIdCoreToHidl(&objID[i], &dmcOid);
        auto check = g_pDmcConnection->unsubscribe(g_sessionId, dmcOid);
        if (!check.isOk()) {
            MDI_LOGE(TAG, "DMC HIDL call(unsubscribe) failed!");
            Reset();
            return false;
        }
        dmcResult_e ret = (dmcResult_e)check;
        MDI_LOGD(TAG, "unsubscribe SID=%d, OID=%s, ret=%d", g_sessionId, objID[i].getOidString(), ret);
    }

    return true;
}

bool QueryObject(size_t argsLen, const uint8_t *pArgsBuffer) {
    if (!g_supportAsyncQuery) {
        MDI_LOGE(TAG, "QueryObject only supported after DMC V1.1");
        return false;
    }

    MDI_LOGD(TAG, "QueryObject");
    sp<V1_1::IDmcService> service = tryCastDmcService_V1_1(g_pDmcConnection);

    if (argsLen == 0 || pArgsBuffer == NULL) {
        MDI_LOGE(TAG, "Failed to query object, arg is invalid. argsLen(%d), pArgsBuffer(0x%08x)",
                argsLen, pArgsBuffer);
        return false;
    }

    std::vector<ObjId> objID;
    GetObjectIDFromStringBuffer(argsLen, pArgsBuffer, objID);

    std::vector<unsigned int> simID;
    GetSimIdFromStringBuffer(argsLen, pArgsBuffer, simID);

    if (!objID.size() || !simID.size() || objID.size() != simID.size()) {
        MDI_LOGE(TAG, "SetPayloadSize: Invalid argument (%d, %d)", objID.size(), simID.size());
        return false;
    }

    for(size_t i = 0 ; i < objID.size() ; ++i) {
        dmcObjectId_t dmcOid = {0};
        objIdCoreToHidl(&objID[i], &dmcOid);
        service->queryEventAsync(g_sessionId, (uint8_t)simID[i], dmcOid);
    }

    return true;
}

bool SetPayloadSize(size_t argsLen, const uint8_t *pArgsBuffer) {
    MDI_LOGD(TAG, "SetPayloadSize");

    if (argsLen == 0 || pArgsBuffer == NULL) {
        MDI_LOGE(TAG, "Failed to setup IP raw size, arg is invalid. argsLen(%zu), pArgsBuffer(0x%08x)  ", argsLen, pArgsBuffer);
        return false;
    }

    if (!tryGetDmcService("SetPayloadSize")) {
        return false;
    }

    std::string ArgString((const char *)pArgsBuffer, argsLen);
    std::vector<ObjId> objID;
    GetObjectIDFromStringBuffer(argsLen, pArgsBuffer,objID);
    std::vector<unsigned int> objSize;
    GetTrapMaxRawSizeFromStringBuffer(argsLen ,pArgsBuffer, objSize);

    if (!objID.size() || !objSize.size() || objID.size() != objSize.size()) {
        MDI_LOGE(TAG, "SetPayloadSize: Invalid argument (%lu, %lu)", objID.size(), objSize.size());
        return false;
    }

    for(size_t i = 0 ; i < objID.size() ; ++i) {
        dmcObjectId_t dmcOid = {0};
        objIdCoreToHidl(&objID[i], &dmcOid);
        auto check = g_pDmcConnection->invokeInt(
                g_sessionId, dmcCommand_e::DMC_CMD_SET_MAX_PAYLOAD_SIZE, dmcOid, (int)objSize[i]);
        if (!check.isOk()) {
            MDI_LOGE(TAG, "DMC HIDL call(invokeInt %d) failed!",
                    dmcCommand_e::DMC_CMD_SET_MAX_PAYLOAD_SIZE);
            Reset();
            return false;
        }
        dmcResult_e ret = (dmcResult_e)check;
        if (ret != dmcResult_e::DMC_SUCCESS) {
            MDI_LOGE(TAG, "SetPayloadSize(%u) to OID %s failed, ret = %d",
                    objSize[i], objID[i].getOidString(), ret);
        }
    }

    return true;
}

bool SetPacketType(size_t argsLen, const uint8_t *pArgsBuffer) {
    MDI_LOGD(TAG, "SetPacketType");

    if (argsLen == 0 || pArgsBuffer == NULL) {
        MDI_LOGE(TAG, "Failed to setup IP raw size, arg is invalid. argsLen(%zu), pArgsBuffer(0x%08x)  ", argsLen, pArgsBuffer);
        return false;
    }

    if (!tryGetDmcService("SetPacketType")) {
        return false;
    }

    std::string ArgString((const char *)pArgsBuffer, argsLen);
    std::vector<ObjId> objID;
    GetObjectIDFromStringBuffer(argsLen, pArgsBuffer, objID);

    std::vector<unsigned int> packetType;
    if (!GetPacketTypeFromStringBuffer(argsLen, pArgsBuffer, packetType)) {
        MDI_LOGE(TAG, "SetPacketType: Invalid argument of packet type");
        return false;
    }

    if (!objID.size() || !packetType.size() || objID.size() != packetType.size()) {
        MDI_LOGE(TAG, "SetPacketType: Invalid argument (%lu, %lu)", objID.size(), packetType.size());
        return false;
    }

    for (size_t i = 0 ; i < objID.size() ; ++i) {
        if (packetType[i] != CMD_PACKET_TYPE_HEADER_ONLY &&
                packetType[i] != CMD_PACKET_TYPE_WITH_PAYLOAD) {
            MDI_LOGE(TAG, "SetPacketType(): Invalid argument packet type: %u", packetType[i]);
            continue;
        }
        dmcObjectId_t dmcOid = {0};
        objIdCoreToHidl(&objID[i], &dmcOid);
        auto check = g_pDmcConnection->invokeInt(
                g_sessionId, dmcCommand_e::DMC_CMD_SET_PACKET_TYPE, dmcOid, (int)packetType[i]);
        if (!check.isOk()) {
            MDI_LOGE(TAG, "DMC HIDL call(invokeInt %d) failed!",
                    dmcCommand_e::DMC_CMD_SET_PACKET_TYPE);
            Reset();
            return false;
        }
        dmcResult_e ret = (dmcResult_e)check;
        if (ret != dmcResult_e::DMC_SUCCESS) {
            MDI_LOGE(TAG, "SetPacketType(%u) to OID %s failed, ret = %d",
                    packetType[i], objID[i].getOidString(), ret);
        }
    }
    return true;
}

// ==================== Entry ======================
int main(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    int err = -1;

    SetupSigHandler();
    InitGlobalVariable();

#if 0
    // Use USB COM port to coommunication with PC-side
    if (NULL == (pChannel = new ComPortManager)) {
        MDI_LOGE(TAG, "Failed to new ComPortManager.");
        goto End;
    }
#endif

    // Use TCP/IP socket (forward by ADB) to communication with PC-side
    if (NULL == (pChannel = new NetworkSocketManager(PORT_NUM))) {
        MDI_LOGE(TAG, "Failed to new NetworkSocketManager.");
        goto End;
    }

    /* Ready to accept commands */
    SetupCommandReceiver();

    /* Loop */
    while (!gbTerminate) {
        //sleep(1);
        //WriteKpiObjectMessage(NULL, buf, 6);
    }

    err = 0;
End:
    /* Cleanup */
    if (NULL != pChannel) {
        delete pChannel;
        pChannel = NULL;
    }

    MDI_LOGD(TAG, "Cleaning done");

    pthread_mutex_destroy(&gMutex);

    return err;
}

