/*
 * Copyright (c) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_RIL_INTERNAL_H
#define ANDROID_RIL_INTERNAL_H

namespace android {
// for RILJ - AOSP APIs
#define RIL_SERVICE_NAME_BASE "slot"
#define RIL1_SERVICE_NAME "slot1"
#define RIL2_SERVICE_NAME "slot2"
#define RIL3_SERVICE_NAME "slot3"
#define RIL4_SERVICE_NAME "slot4"
// for RILJ - MTK APIs
#define MTK_RIL_SERVICE_NAME_BASE "mtkSlot"
#define MTK_RIL1_SERVICE_NAME "mtkSlot1"
#define MTK_RIL2_SERVICE_NAME "mtkSlot2"
#define MTK_RIL3_SERVICE_NAME "mtkSlot3"
#define MTK_RIL4_SERVICE_NAME "mtkSlot4"
// for IMS - AOSP APIs
#define IMS_AOSP_RIL_SERVICE_NAME_BASE "imsAospSlot"
#define IMS_AOSP_RIL1_SERVICE_NAME "imsAospSlot1"
#define IMS_AOSP_RIL2_SERVICE_NAME "imsAospSlot2"
#define IMS_AOSP_RIL3_SERVICE_NAME "imsAospSlot3"
#define IMS_AOSP_RIL4_SERVICE_NAME "imsAospSlot4"

// for IMS - MTK APIs
#define IMS_RIL_SERVICE_NAME_BASE "imsSlot"
#define IMS_RIL1_SERVICE_NAME "imsSlot1"
#define IMS_RIL2_SERVICE_NAME "imsSlot2"
#define IMS_RIL3_SERVICE_NAME "imsSlot3"
#define IMS_RIL4_SERVICE_NAME "imsSlot4"


/// MTK: ForSE @{
// for SE - AOSP APIs
#define SE_RIL_SERVICE_NAME_BASE "se"
#define SE_RIL1_SERVICE_NAME "se1"
#define SE_RIL2_SERVICE_NAME "se2"
#define SE_RIL3_SERVICE_NAME "se3"
#define SE_RIL4_SERVICE_NAME "se4"
// for SE - MTK APIs
#define MTK_SE_RIL_SERVICE_NAME_BASE "mtkSe"
#define MTK_SE_RIL1_SERVICE_NAME "mtkSe1"
#define MTK_SE_RIL2_SERVICE_NAME "mtkSe2"
#define MTK_SE_RIL3_SERVICE_NAME "mtkSe3"
#define MTK_SE_RIL4_SERVICE_NAME "mtkSe4"
/// MTK: ForSE @}

// for EM - AOSP APIs
#define EM_RIL_SERVICE_NAME_BASE "em"
#define EM_RIL1_SERVICE_NAME "em1"
#define EM_RIL2_SERVICE_NAME "em2"
#define EM_RIL3_SERVICE_NAME "em3"
#define EM_RIL4_SERVICE_NAME "em4"

// for EM - MTK APIs
#define MTK_EM_RIL_SERVICE_NAME_BASE "mtkEm"
#define MTK_EM_RIL1_SERVICE_NAME "mtkEm1"
#define MTK_EM_RIL2_SERVICE_NAME "mtkEm2"
#define MTK_EM_RIL3_SERVICE_NAME "mtkEm3"
#define MTK_EM_RIL4_SERVICE_NAME "mtkEm4"

// For telephony assist - MTK APIs
#define MTK_ASSIST_RIL_SERVICE_NAME_BASE "mtkAssit"
#define MTK_ASSIST_RIL1_SERVICE_NAME "mtkAssist1"
#define MTK_ASSIST_RIL2_SERVICE_NAME "mtkAssist2"
#define MTK_ASSIST_RIL3_SERVICE_NAME "mtkAssist3"
#define MTK_ASSIST_RIL4_SERVICE_NAME "mtkAssist4"

// For RCS
#define MTK_RCS_RIL_SERVICE_NAME_BASE "mtkRcs"
#define MTK_RCS_RIL1_SERVICE_NAME "mtkRcs1"
#define MTK_RCS_RIL2_SERVICE_NAME "mtkRcs2"
#define MTK_RCS_RIL3_SERVICE_NAME "mtkRcs3"
#define MTK_RCS_RIL4_SERVICE_NAME "mtkRcs4"

// For CapCtrl
#define MTK_CAP_RIL_SERVICE_NAME_BASE "mtkCap"
#define MTK_CAP_RIL1_SERVICE_NAME "mtkCap1"
#define MTK_CAP_RIL2_SERVICE_NAME "mtkCap2"
#define MTK_CAP_RIL3_SERVICE_NAME "mtkCap3"
#define MTK_CAP_RIL4_SERVICE_NAME "mtkCap4"

// Define Multiple Client of Radio HIDL
enum ClientId {
  CLIENT_RILJ = 0,
  CLIENT_IMS,
  CLIENT_SE,
  CLIENT_EM,
  CLIENT_ASSIST,
  CLIENT_RCS,
  CLIENT_CAP,
  CLIENT_COUNT
};

typedef enum {
    RADIO_AOSP,
    RADIO_MTK,
    IMS_AOSP,
    IMS_MTK,
    IMS_WFC,
    SE_MTK,
    EM_AOSP,
    EM_MTK,
    ASSIST_MTK,
    RCS_MTK,
    CAP_MTK,
    CLIENT_TYPE_COUNT
} ClientType;

/* Constants for response types */
#define RESPONSE_SOLICITED 0
#define RESPONSE_UNSOLICITED 1
#define RESPONSE_SOLICITED_ACK 2
#define RESPONSE_SOLICITED_ACK_EXP 3
#define RESPONSE_UNSOLICITED_ACK_EXP 4

// Enable verbose logging
#define VDBG 0

#ifndef MIN
#define MIN(a,b) ((a)<(b) ? (a) : (b))
#endif

// Enable RILC log
#define RILC_LOG 0

#if RILC_LOG
    #define startRequest           sprintf(printBuf, "(")
    #define closeRequest           sprintf(printBuf, "%s)", printBuf)
    #define printRequest(token, req)           \
            RFX_LOG_D(LOG_TAG, "[%04d]> %s %s", token, requestToString(req), printBuf)

    #define startResponse           sprintf(printBuf, "%s {", printBuf)
    #define closeResponse           sprintf(printBuf, "%s}", printBuf)
    #define printResponse           RFX_LOG_D(LOG_TAG, "%s", printBuf)

    #define clearPrintBuf           printBuf[0] = 0
    #define removeLastChar          printBuf[strlen(printBuf)-1] = 0
    #define appendPrintBuf(x...)    snprintf(printBuf, PRINTBUF_SIZE, x)
#else
    #define startRequest
    #define closeRequest
    #define printRequest(token, req)
    #define startResponse
    #define closeResponse
    #define printResponse
    #define clearPrintBuf
    #define removeLastChar
    #define appendPrintBuf(x...)
#endif

typedef struct CommandInfo CommandInfo;

extern "C" const char * requestToString(int request);

typedef struct RequestInfo {
    int32_t token;      //this is not RIL_Token
    CommandInfo *pCI;
    struct RequestInfo *p_next;
    char cancelled;
    char local;         // responses to local commands do not go back to command process
    RIL_SOCKET_ID socket_id;
    int wasAckSent;    // Indicates whether an ack was sent earlier
} RequestInfo;

typedef struct CommandInfo {
    int requestNumber;
    int(*responseFunction) (int slotId, ClientId clientId, int responseType, int token,
            RIL_Errno e, void *response, size_t responselen);
} CommandInfo;

enum WakeType {DONT_WAKE, WAKE_PARTIAL};

typedef struct {
    int requestNumber;
    int (*responseFunction) (int slotId, int responseType, int token,
            RIL_Errno e, void *response, size_t responselen);
    WakeType wakeType;
} UnsolResponseInfo;

RequestInfo * addRequestToList(int serial, int slotId, int request);

char * RIL_getServiceName();

void releaseWakeLock();

void onNewCommandConnect(RIL_SOCKET_ID socket_id, ClientType clientType);

void onCommandDisconnect(RIL_SOCKET_ID socket_id, ClientType clientType);

}   // namespace android

#endif //ANDROID_RIL_INTERNAL_H
