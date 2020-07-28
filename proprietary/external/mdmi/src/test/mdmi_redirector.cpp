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

#include "mdmi_redirector.h"
#include "CommandCode.h"
#include "CommandProcessor.h"
#include "NetworkSocketManager.h"
#include "Mdmi_defs.h"

//TODO: #include "ComPortManager.h"

#include <dlfcn.h>
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

#undef TAG
#define TAG "MDMI-MdmiRedirector"

//
// ==================== Commands functions prototype ======================
//
bool CreateSession(size_t argsLen, const uint8_t *pArgsBuffer);
bool CloseSession(size_t argsLen, const uint8_t *pArgsBuffer);
bool Get(size_t argsLen, const uint8_t *pArgsBuffer);
bool Set(size_t argsLen, const uint8_t *pArgsBuffer);
bool Invoke(size_t argsLen, const uint8_t *pArgsBuffer);
bool Subscribe(size_t argsLen, const uint8_t *pArgsBuffer);
bool Unsubscribe(size_t argsLen, const uint8_t *pArgsBuffer);
bool GetSessionStats(size_t argsLen, const uint8_t *pArgsBuffer);

//
// ==================== Global variable ======================
//
volatile sig_atomic_t gbTerminate = false;
// Communication
static IoChannel *pChannel = NULL;

static MdmiLibFp_t g_libFpTable[MDMI_LIB_MAX];
static MdmiLibInfo_t g_libInfo[MDMI_LIB_MAX] = {
    {MDMI_LIB_NAME_DEBUG},
    {MDMI_LIB_NAME_LTE},
    {MDMI_LIB_NAME_GSM},
    {MDMI_LIB_NAME_UMTS},
    {MDMI_LIB_NAME_WIFI},
    {MDMI_LIB_NAME_IMS},
    {MDMI_LIB_NAME_COMMANDS},
    {MDMI_LIB_NAME_WCDMA},
    {MDMI_LIB_NAME_HSUPA},
    {MDMI_LIB_NAME_HSDPA},
    {MDMI_LIB_NAME_EMBMS}
};

// The command code neet to align with mdi_redirector_ctrl.
const struct redirectorCmdEntry g_redirectorCmdTable[] = {
    {MDMI_CMD_CODE_CREATE_SESSION, CreateSession},
    {MDMI_CMD_CODE_CLOSE_SESSION, CloseSession},
    {MDMI_CMD_CODE_GET, Get},
    {MDMI_CMD_CODE_SET, Set},
    {MDMI_CMD_CODE_INVOKE, Invoke},
    {MDMI_CMD_CODE_SUBSCRIBE, Subscribe},
    {MDMI_CMD_CODE_UNSUBSCRIBE, Unsubscribe},
    {MDMI_CMD_CODE_GET_SESSION_STATS, GetSessionStats}
};

static pthread_mutex_t gMutex;

// ==================== Internal Functions ======================
void SigHandler(int value) {
    switch (value) {
        case SIGTERM:
            pthread_mutex_lock(&gMutex);
            gbTerminate = true;
            pthread_mutex_unlock(&gMutex);
            MDMI_LOGI(TAG, "Receive SIGTERM signal [%d]", value);
            break;
        default:
            MDMI_LOGD(TAG, "Receive signal [%d]", value);
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

int loadMdmiLibraries() {
    void *refLibMdmi = NULL;
    int validLibCount = 0;

    for (int type = MDMI_LIB_BEGIN; type < MDMI_LIB_MAX; type++) {
        MDMI_LOGD(TAG, "Start load MDMI lib %s", g_libInfo[type].libName);

        refLibMdmi = dlopen(g_libInfo[type].libName, RTLD_NOW | RTLD_LOCAL);
        if (refLibMdmi == 0) {
            MDMI_LOGE(TAG, "open lib %s failed", g_libInfo[type].libName);
            continue;
        }
        g_libFpTable[type].type = (MDMI_LIB_TYPE)type;

        g_libFpTable[type].fpCreateSession =
                (FP_MDMI_CREATE_SESSION)dlsym(refLibMdmi, FN_NAME_CREATE_SESSION);
        if (g_libFpTable[type].fpCreateSession == 0) {
            MDMI_LOGE(TAG, "dlsym %s failed", FN_NAME_CREATE_SESSION);
            continue;
        }
        g_libFpTable[type].fpCloseSession =
                (FP_MDMI_CLOSE_SESSION)dlsym(refLibMdmi, FN_NAME_CLOSE_SESSION);
        if (g_libFpTable[type].fpCloseSession == 0) {
            MDMI_LOGE(TAG, "dlsym %s failed", FN_NAME_CLOSE_SESSION);
            continue;
        }
        g_libFpTable[type].fpGet =
                (FP_MDMI_GET)dlsym(refLibMdmi, FN_NAME_GET);
        if (g_libFpTable[type].fpGet == 0) {
            MDMI_LOGE(TAG, "dlsym %s failed", FN_NAME_GET);
            continue;
        }
        g_libFpTable[type].fpSet =
                (FP_MDMI_SET)dlsym(refLibMdmi, FN_NAME_SET);
        if (g_libFpTable[type].fpSet == 0) {
            MDMI_LOGE(TAG, "dlsym %s failed", FN_NAME_SET);
            continue;
        }
        g_libFpTable[type].fpInvoke =
                (FP_MDMI_INVOKE)dlsym(refLibMdmi, FN_NAME_INVOKE);
        if (g_libFpTable[type].fpInvoke == 0) {
            MDMI_LOGE(TAG, "dlsym %s failed", FN_NAME_INVOKE);
            continue;
        }
        g_libFpTable[type].fpSetEventCallback =
                (FP_MDMI_SET_EVENT_CALLBACK)dlsym(refLibMdmi, FN_NAME_SET_EVENT_CALLBACK);
        if (g_libFpTable[type].fpSetEventCallback == 0) {
            MDMI_LOGE(TAG, "dlsym %s failed", FN_NAME_SET_EVENT_CALLBACK);
            continue;
        }
        g_libFpTable[type].fpSubscribe =
                (FP_MDMI_SUBSCRIBE)dlsym(refLibMdmi, FN_NAME_SUBSCRIBE);
        if (g_libFpTable[type].fpSubscribe == 0) {
            MDMI_LOGE(TAG, "dlsym %s failed", FN_NAME_SUBSCRIBE);
            continue;
        }
        g_libFpTable[type].fpUnsubscribe =
                (FP_MDMI_UNSUBSCRIBE)dlsym(refLibMdmi, FN_NAME_UNSUBSCRIBE);
        if (g_libFpTable[type].fpUnsubscribe == 0) {
            MDMI_LOGE(TAG, "dlsym %s failed", FN_NAME_UNSUBSCRIBE);
            continue;
        }
        g_libFpTable[type].fpGetSessionStats =
                (FP_MDMI_GET_SESSION_STATS)dlsym(refLibMdmi, FN_NAME_GET_SESSION_STATS);
        if (g_libFpTable[type].fpGetSessionStats == 0) {
            MDMI_LOGE(TAG, "dlsym %s failed", FN_NAME_GET_SESSION_STATS);
            continue;
        }
        MDMI_LOGD(TAG, "Load MDMI lib %s successfully!", g_libInfo[type].libName);
        validLibCount++;
    }
    return validLibCount;
}

void SetupCommandReceiver() {
    int cmdTableSize = sizeof(g_redirectorCmdTable)/sizeof(g_redirectorCmdTable[0]);
    static SocketListener socketListener(REDIRECTOR_COMMAND_RECEIVER_NAME);
    static CommandProcessor cmdProcessor(g_redirectorCmdTable, cmdTableSize);

    socketListener.SetConnectionHandler(&cmdProcessor);
}

static std::string GetStringAttributeStringValue(std::string &inputString, const char * attr) {
    std::size_t attri_pos = inputString.find(attr);
    std::size_t attri_value_spos = attri_pos + strlen(attr) + 1; // +1 = "="
    std::size_t attri_value_epos = inputString.find(" ", attri_value_spos ); // search next " "

    //MDMI_LOGE(TAG, "GetStringAttributeStringValue(): search \"%s\" in \"%s\", value spos = %d, epos = %d.",attr,inputString.c_str(),attri_value_spos,attri_value_epos);

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

static bool CheckValidLibType(int libType) {
    if (libType <= MDMI_LIB_UNDEFINED || libType >= MDMI_LIB_MAX) {
        MDMI_LOGE(TAG, "CheckValidLibType: Invalid libtype: %d", libType);
        return false;
    }
    return true;
}

static bool GetLibTypeFromStringBuffer(size_t argsLen,
        const uint8_t *pArgsBuffer, int &libType) {
    // parse "libType="
    bool bFound = false;
    std::string argvString((const char *)pArgsBuffer, argsLen);

    while (STRING_ATTR_MATCH(argvString, "libtype")) {
        std::string typeString = GetStringAttributeStringValue(argvString, "libtype");

        if (typeString.length() != 0 && CheckStringIsDigital(typeString)) {
            libType = atoi(typeString.c_str());
            if (!CheckValidLibType(libType)) {
                return false;
            }

            bFound = true;
            break;
        } else {
            MDMI_LOGE(TAG, "GetLibTypeFromStringBuffer: Invalid libtype: %s", typeString.c_str());
            return false;
        }
    }

    if (!bFound) {
        MDMI_LOGE(TAG, "GetLibTypeFromStringBuffer: No libtype found!");
        return false;
    }

    return true;
}


bool GetObjectIDFromStringBuffer(size_t argsLen, const uint8_t *pArgsBuffer, std::vector<ObjId> &ObjID) {
    // parse "OID="
    std::string argvString((const char *)pArgsBuffer, argsLen);

    while (STRING_ATTR_MATCH(argvString, "OID")) {
        std::string idString = GetStringAttributeStringValue(argvString, "OID");
        if (idString.length() == 0) {
            MDMI_LOGE(TAG, "GetObjectIDFromStringBuffer() : oid string is NULL.");
            continue;
        }
        MDMI_LOGD(TAG, "OID message : Setup OID string = %s.", idString.c_str());

        // set id & push to vector
        ObjId oid;
        oid.setIdString(idString.c_str());
        ObjID.push_back(oid);
    }
    if (ObjID.size() == 0) {
        MDMI_LOGE(TAG, "OID message : Not found OID assigment.");
        return false;
    } else {
        MDMI_LOGD(TAG, "OID message : %lu OID in assignment.", ObjID.size());
    }

    return true;
}

uint32_t GetIntFromBuffer(const uint8_t *data, int width) {
    uint32_t ret = 0;

    for (int i = width - 1 ; i >= 0 ; --i) {
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

    MDMI_LOGD(TAG, "SendOneFrameWithSyncToken: obj data len = %zu", data_size);

    if (buf != NULL) {
        // Send Data
        if (!pChannel->Write(buf, data_size)) {
            MDMI_LOGE(TAG, "Failed to write 'Data' to IoChannel.");
            pthread_mutex_unlock(&gMutex);
            return false;
        }

    } else {
        MDMI_LOGD(TAG, "SendOneFrameWithSyncToken: ignore sending data");
    }
    pthread_mutex_unlock(&gMutex);

    MDMI_LOGD(TAG, "SendOneFrameWithSyncToken: data write done");
    return true;
}

#define OID_NAME_LEN_MAX (50)
void GetOidNameString(const MdmiObjectName *eventName, char *result) {
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

MdmiSession MapMdmiLibSessionId(int libType) {
    MdmiSession session = INVALID_SESSION_ID;

    if (libType >= 0) {
        session = (MdmiSession)((libType + 1) * 1000 + 1);
    } else {
        MDMI_LOGE(TAG, "MapMdmiLibSessionId() failed with invalid libType = %d", libType);        
    }
    
    return session;
}

// ==================== Public Functions ========================
void MdmiEventCallbackImpl(MdmiSession session, const MdmiObjectName *eventName,
        const MdmiValue* eventValue, void* state) {
    char buffer[OID_NAME_LEN_MAX] = {0};
    GetOidNameString(eventName, buffer);
    MDMI_LOGD(TAG, "MdmiEventCallbackImpl() Lib SID = %d, eventName = %s, data len = %d",
            session, buffer, eventValue->length);

    unsigned int length = eventValue->length;
    const uint8_t *data = (uint8_t *)eventValue->data;

    bool ret = WriteKpiObjectMessage(data, length);
}

bool CreateSession(size_t argsLen, const uint8_t *pArgsBuffer) {
    if (argsLen == 0 || pArgsBuffer == NULL) {
        MDMI_LOGE(TAG, "Failed to CreateSession, arg is invalid. argsLen(%zu), pArgsBuffer(0x%08x)",
                argsLen, pArgsBuffer);
        return false;
    }

    int libType = MDMI_LIB_UNDEFINED;
    if (!GetLibTypeFromStringBuffer(argsLen, pArgsBuffer, libType)) {
        return false;
    }

    MdmiSession session = INVALID_SESSION_ID;
    MdmiError ret = g_libFpTable[libType].fpCreateSession(NULL, &session);
    MDMI_LOGD(TAG, "CreateSession() libType = %d, ret = %d", libType, ret);

    // Regist event callbacks 
    ret = g_libFpTable[libType].fpSetEventCallback(session, MdmiEventCallbackImpl, NULL);
    MDMI_LOGD(TAG, "SetEventCallback() ret = %d", ret);

    return true;
}

bool CloseSession(size_t argsLen, const uint8_t *pArgsBuffer) {
    if (argsLen == 0 || pArgsBuffer == NULL) {
        MDMI_LOGE(TAG, "Failed to CreateSession, arg is invalid. argsLen(%zu), pArgsBuffer(0x%08x)",
                argsLen, pArgsBuffer);
        return false;
    }

    int libType = MDMI_LIB_UNDEFINED;
    if (!GetLibTypeFromStringBuffer(argsLen, pArgsBuffer, libType)) {
        return false;
    }

    MdmiSession session = MapMdmiLibSessionId(libType);
    MdmiError ret = g_libFpTable[libType].fpCloseSession(session);
    MDMI_LOGD(TAG, "CloseSession() ret = %d", ret);

    return true;
}

bool Get(size_t argsLen, const uint8_t *pArgsBuffer) {
    UNUSED(argsLen);
    UNUSED(pArgsBuffer);
    // TODO: Complete me
    return false;
}

bool Set(size_t argsLen, const uint8_t *pArgsBuffer) {
    UNUSED(argsLen);
    UNUSED(pArgsBuffer);

    MDMI_LOGE(TAG, "Not support MDMI Set()");
    return false;
}

bool Invoke(size_t argsLen, const uint8_t *pArgsBuffer) {
    UNUSED(argsLen);
    UNUSED(pArgsBuffer);

    MDMI_LOGE(TAG, "Not support MDMI Invoke()");
    return false;
}

bool Subscribe(size_t argsLen, const uint8_t *pArgsBuffer) {
    if (argsLen == 0 || pArgsBuffer == NULL) {
        MDMI_LOGE(TAG, "Failed to subscribe traps, arg is invalid. argsLen(%zu), pArgsBuffer(0x%08x)",
                argsLen, pArgsBuffer);
        return false;
    }

    int libType = MDMI_LIB_UNDEFINED;
    if (!GetLibTypeFromStringBuffer(argsLen, pArgsBuffer, libType)) {
        return false;
    }

    std::vector<ObjId> objID;
    GetObjectIDFromStringBuffer(argsLen, pArgsBuffer, objID);

    MdmiSession session = MapMdmiLibSessionId(libType);
    MdmiObjectName objName = {0};
    for (size_t i = 0 ; i < objID.size(); ++i) {
        objName.length = (MdmiTypeUInt32)objID[i].getObjLength();
        objName.ids = (MdmiTypeUInt32 *)calloc(objName.length, sizeof(MdmiTypeUInt32));
        if (objName.ids == NULL) {
            return false;
        }
        uint32_t *ids = objID[i].getObjId();
        for (MdmiTypeUInt32 j = 0; j < objName.length; j++) {
            objName.ids[j] = (MdmiTypeUInt32)ids[j];
        }
        MdmiError ret = g_libFpTable[libType].fpSubscribe(session, &objName);
        MDMI_LOGD(TAG, "Subscribe(%s) ret = %d", objID[i].getOidString(), ret);

        free(objName.ids);
    }

    return true;
}

bool Unsubscribe(size_t argsLen, const uint8_t *pArgsBuffer) {
    if(argsLen == 0 || pArgsBuffer == NULL) {
        MDMI_LOGE(TAG, "Failed to subscribe traps, arg is invalid. argsLen(%zu), pArgsBuffer(0x%08x)",
                argsLen, pArgsBuffer);
        return false;
    }

    int libType = MDMI_LIB_UNDEFINED;
    if (!GetLibTypeFromStringBuffer(argsLen, pArgsBuffer, libType)) {
        return false;
    }

    std::vector<ObjId> objID;
    GetObjectIDFromStringBuffer(argsLen, pArgsBuffer, objID);

    MdmiSession session = MapMdmiLibSessionId(libType);
    MdmiObjectName objName = {0};
    for (size_t i = 0 ; i < objID.size(); ++i) {
        objName.length = (MdmiTypeUInt32)objID[i].getObjLength();
        objName.ids = (MdmiTypeUInt32 *)calloc(objName.length, sizeof(MdmiTypeUInt32));
        if (objName.ids == NULL) {
            return false;
        }
        uint32_t *ids = objID[i].getObjId();
        for (MdmiTypeUInt32 j = 0; j < objName.length; j++) {
            objName.ids[j] = (MdmiTypeUInt32)ids[j];
        }
        MdmiError ret = g_libFpTable[libType].fpUnsubscribe(session, &objName);
        MDMI_LOGD(TAG, "Unsubscribe(%s) ret = %d", objID[i].getOidString(), ret);

        free(objName.ids);
    }

    return true;
}

bool GetSessionStats(size_t argsLen, const uint8_t *pArgsBuffer) {
    MdmiSessionStats sessionStats = {
            .size = 0,
            .totalEvents = 0,
            .droppedEvents = 0,
            .eventBufferCapacity = 0,
            .eventBufferInUse = 0
    };
    if(argsLen == 0 || pArgsBuffer == NULL) {
        MDMI_LOGE(TAG, "Failed to GetSessionStats, arg is invalid. argsLen(%zu), pArgsBuffer(0x%08x)",
                argsLen, pArgsBuffer);
        return false;
    }

    int libType = MDMI_LIB_UNDEFINED;
    if (!GetLibTypeFromStringBuffer(argsLen, pArgsBuffer, libType)) {
        return false;
    }
    MdmiSession session = MapMdmiLibSessionId(libType);
    MdmiError ret = g_libFpTable[libType].fpGetSessionStats(session, &sessionStats);
    MDMI_LOGD(TAG, "GetSessionStats(%d) ret = %d", libType, ret);

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
        MDMI_LOGE(TAG, "Failed to new ComPortManager.");
        goto End;
    }
#endif

    if (loadMdmiLibraries() == 0) {
        MDMI_LOGE(TAG, "NO MDMI library exist!");
        goto End;
    }

    // Use TCP/IP socket (forward by ADB) to communication with PC-side
    if (NULL == (pChannel = new NetworkSocketManager(PORT_NUM))) {
        MDMI_LOGE(TAG, "Failed to new NetworkSocketManager.");
        goto End;
    }

    /* Ready to accept commands */
    SetupCommandReceiver();

    /* Loop */
    while (!gbTerminate) {
        //sleep(1);
    }

    err = 0;
End:
    /* Cleanup */
    if (NULL != pChannel) {
        delete pChannel;
        pChannel = NULL;
    }

    MDMI_LOGD(TAG, "Cleaning done");

    pthread_mutex_destroy(&gMutex);

    return err;
}

