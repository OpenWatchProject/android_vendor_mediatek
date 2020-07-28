/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#include <telephony/mtk_ril.h>
#include <libmtkrilutils.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <alloca.h>
#include "atchannels.h"
#include "at_tok.h"
#include "misc.h"
#include <getopt.h>
#include <sys/socket.h>
#include <compiler_utils.h>
#include <cutils/sockets.h>
#include <termios.h>
#include <cutils/properties.h>
#include <ctype.h>

#include <ril_callbacks.h>
#include "usim_fcp_parser.h"
#include "base64.h"
// MTK-START: BT SIM Access Profile
#include "pb_decode.h"
#include "pb_encode.h"
// MTK-END
#include <hardware_legacy/power.h>
// MTK-START: BT SIM Access Profile
#include <hardware/ril/librilutils/proto/sap-api.pb.h>
// MTK-END

#undef LOG_TAG
#define LOG_TAG "RIL-SIM"

#include <log/log.h>
#include "ratconfig.h"

#include <ril_cc.h>

//---Definition---//
#define SIM_CHANNEL_CTX getRILChannelCtxFromToken(t)
#define CMD_GET_RESPONSE 192

//---Static Variables---//
static const struct timeval TIMEVAL_SIMPOLL = {1,0};

//---Exported Variables--//
int sim_inserted_status = -1;

// Use to protect the function which is called from
// many functions in different threads
static pthread_mutex_t simStatusMutex1 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t simStatusMutex2 = PTHREAD_MUTEX_INITIALIZER;

//---Extern Functions---//
extern SIM_Status getSIMStatus(RIL_SOCKET_ID rid);
extern void upadteSystemPropertyByCurrentModeGemini(int rid, char* key1, char* key2, char* key3, char* key4, char* value);
extern void detectTestSim(RIL_SOCKET_ID rid);
extern RIL_RadioState getRadioState(RIL_SOCKET_ID rid);
extern int isCdmaLteDcSupport();
extern void setupOpProperty(RIL_SOCKET_ID rid);
extern int hexStringToByteArrayEx(unsigned char* hexString, int hexStringLen,
        unsigned char ** byte);
// Dynamic-SBP: Run-time configure SBP-ID according to SIM MCC/MNC.
extern void setupDynamicSBP(RIL_SOCKET_ID rid);
extern void setStkServiceRunningFlag(RIL_SOCKET_ID rid, bool flag);
extern void freeStkQueuedProactivCmdByRid(RIL_SOCKET_ID rid);
extern void freeStkCacheMenuByRid(RIL_SOCKET_ID rid);


// MTK-START: BT SIM Access Profile
static void requestBtSapConnect(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid);
static void requestLocalSapConnect(void *param);
static void requestBtSapDisconnect(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid);
static void requestLocalBtSapDisconnect(void *param);
static void requestBtSapTransferApdu(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid);
static void requestLocalBtSapTransferApdu(void *param);
static void requestBtSapTransferAtr(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid);
static void requestBtSapPower(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid);
static void requestLocalBtSapPower(void *param);
static void requestBtSapResetSim(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid);
static void requestLocalBtSapResetSim(void *param);
static void requestBtSapSetTransferProtocol(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid);

static void sendBtSapResponseComplete(RIL_Token t, RIL_Errno ret, MsgId msgId,
        void *data);

static char* btSapMsgIdToString(MsgId msgId);

extern int inCallNumberPerSim[MAX_SIM_COUNT] ;
// MTK-END

static void requestSimGetEfdir(void *data, size_t datalen, RIL_Token t);

extern void onSimInsertChangedForData(RIL_SOCKET_ID rid, const char* urc);
extern void setRadioState(RIL_RadioState newState, RIL_SOCKET_ID rid);
static int isCdmaCard(RIL_SOCKET_ID rid);
static void configModemStatus(RIL_SOCKET_ID rid);

// External SIM [START]
#define VSIM_TRIGGER_RESET      0
#define VSIM_TRIGGER_PLUG_IN    1
#define VSIM_TRIGGER_PLUG_OUT   2
int isVsimEnabled();
int isVsimEnabledByRid(int rid);
int isRecoverySimState[4]={0, 0, 0, 0};
int getVsimPlugInOutEvent(int slotId);
void setVsimPlugInOutEvent(int slotId, int state);
// External SIM [END]

bool serviceActivationStatus(int fileId, int appTypeId, RIL_Token t);
int queryAppTypeByAid(char* aid);

/* -1: invalid, 0: sim, 1: usim, 2: none*/
int isUsimDetect[4]={-1,-1,-1,-1};
/* -1: invalid, 0: uim, 1: csim, 2: none, 3: csim and ruim */
int isCDMADetect[4]={-1,-1,-1,-1};
/* -1: invalid, 0: uim, 1: csim, 2: none, 3: csim and ruim */
int cdmaCardType[4]={-1,-1,-1,-1};

bool aIs_sim_off_by_IMEI_lock[4] = {false, false, false, false};

/* Subsidy lock feature only */
static int pinDefer[4] = {-1, -1, -1, -1};
// For STK/UTK switch
typedef struct {
    int mode;
    RIL_SOCKET_ID rid;
} SwitchStkUtkModeParam;

// The prefix of AID which support on our platform
// Support USIM, SIM, ISIM currently
const static char* aid_support_list[AID_MAX] =
{
    "A0000000871002", // USIM
    //"A0000000090001", // SIM
    "A0000000871004"  // ISIM
};

static int read_ef_dir_complete[MAX_SIM_COUNT] = {0};
//MTK-START: ISIM
static SessionInfo isim_session_info[MAX_SIM_COUNT] = {{0,0}};
//MTK-END
static AidInfo aid_info_array[MAX_SIM_COUNT][AID_MAX];

/// M: OP01 6M @{
static char *g_eusim[MAX_SIM_COUNT] = {NULL};

/// @}

// MTK-START: Slot lock
static bool smlSbpConfigureChecked = false;
static bool smlSbpUrcReported = false;
// MTK-END

/// M: OP01 6M @{
static void onOp016mCardDetected(RIL_SOCKET_ID rid);
static void modifyCdmaCardType(int simId);
static int hasCdmaCapability(const char *s);
static void cacheCardTypeUrc(const char *s, RIL_SOCKET_ID rid);
static void clearCardTypeUrc(int simId);
static void checkPinState(const char *s, RIL_SOCKET_ID rid);
static void checkPin(void *param);
static int eraseCdmaCapabilityMode();
static int isOp096mSupport();
int isOp016mSupport();
/// @}

static void onTestSimDetectedInternal(const char *s, RIL_SOCKET_ID rid, int isUrc);

extern int s_md_off;

/* To check if new SS service class feature is supported or not */
extern int bNewSsServiceClassFeature;

// External SIM [Start]
#ifdef MTK_EXTERNAL_SIM_SUPPORT
extern void requestSwitchExternalSim(RIL_SOCKET_ID rid);
extern void onVsimEventDetected(const char *s, RIL_SOCKET_ID rid);
extern void onVsimAuthEventDetected(const char *s, RIL_SOCKET_ID rid);
extern void requestVsimNotification(void *data, size_t datalen, RIL_Token t);
extern void requestVsimOperation(void *data, size_t datalen, RIL_Token t);
extern int isVsimEnabledByRid(int rid);
#endif
// External SIM [End]
extern bool bDssNoResetSupport;
extern void simSwitchNotifySIMInitDone(int slot_id);
void onHandleEsimInd(const char *s, RIL_SOCKET_ID rid);
void onSmlSbpConfigureCheck();
void onSmlStatusChanged(const char *urc, RIL_SOCKET_ID rid);

#define PROPERTY_RIL_SIM_READY              "vendor.ril.ready.sim"

int isSimSlotLockSupport() {
    int isSimSlotLockSupport = 0;

    char lockMode[PROPERTY_VALUE_MAX] = {0};
    property_get("ro.vendor.sim_me_lock_mode", lockMode, "");

    return (strcmp("3", lockMode) == 0) ? 1 : 0;
}

bool isRjioRsuSupport() {
    char property_value[PROPERTY_VALUE_MAX] = {0};
    char property_value_standalone[PROPERTY_VALUE_MAX] = {0};

    property_get("vendor.gsm.sim.slot.lock.policy", property_value, "0");
    property_get("ro.vendor.mtk_subsidy_lock_support", property_value_standalone, "0");

    LOGD("[SUBSIDY_LOCK] isRjioRsuSupport = %d, %d ", atoi(property_value),
            atoi(property_value_standalone));
    if ((atoi(property_value) == 10) || (atoi(property_value_standalone) == 1)) {
        return true;
    }
    return false;
}

int isSimInserted(RIL_SOCKET_ID rid) {
    int pivot = 1;
    int pivotSim = rid;
    int simInserted = 0;

    if (SIM_COUNT >= 2) {
        pivotSim = pivot << rid;

        if ((sim_inserted_status & pivotSim) == pivotSim ) {
            simInserted = 1;
        }
    } else {
        //[ALPS01631073]-START : "sim_inserted_status" is 0 when MTK_GEMINI=false
        char iccid[PROPERTY_VALUE_MAX] = {0};
        property_get(PROPERTY_ICCID_SIM[getMappingSIMByCurrentMode(rid)], iccid, "");
        if ((strlen(iccid) > 0) && (strcmp(iccid, "N/A") != 0)){
            simInserted = 1;
        }
        //[ALPS01631073]-END
    }

    return simInserted;
}


void setSimStatusChanged(RIL_SOCKET_ID rid)
{
    RIL_UNSOL_RESPONSE (
        RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED,
        NULL, 0, rid);
}

static void setPINretryCount(SimPinCount *result,  RIL_SOCKET_ID rid)
{
    char oldPin1[PROPERTY_VALUE_MAX] = {0};
    char oldPin2[PROPERTY_VALUE_MAX] = {0};
    char oldPuk1[PROPERTY_VALUE_MAX] = {0};
    char oldPuk2[PROPERTY_VALUE_MAX] = {0};
    char *count = NULL;
    int simStatusChanged = 0;
    int forceToUpdate = 0;
    int simId = getMappingSIMByCurrentMode(rid);
    const char *propPin1 = PROPERTY_RIL_SIM_PIN1[simId];
    const char *propPin2 = PROPERTY_RIL_SIM_PIN2[simId];
    const char *propPuk1 = PROPERTY_RIL_SIM_PUK1[simId];
    const char *propPuk2 = PROPERTY_RIL_SIM_PUK2[simId];

    property_get(propPin1,oldPin1,NULL);

    if (!(property_get(propPin1,oldPin1,NULL)
         &&property_get(propPin2,oldPin2,NULL)
         &&property_get(propPuk1,oldPuk1,NULL)
         &&property_get(propPuk2,oldPuk2,NULL))) {
        RLOGE("Load property fail!");
        forceToUpdate = 1;
    } else {

        RLOGD("oldpin1:%s, oldpin2:%s, oldpuk1:%s, oldpuk2:%s",
                oldPin1,oldPin2,oldPuk1,oldPuk2);
    }

    /* PIN1 */
    asprintf(&count,"%d",result->pin1);
    if (forceToUpdate) {

        property_set(propPin1,count);

    } else if (strcmp((const char*)oldPin1,(const char*) count)) {
        /* PIN1 has been changed */
        property_set(propPin1,count);

        if (result->pin1 == 0) {
            /* SIM goto PUK state*/
            RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0, rid);
            simStatusChanged++;
        } else if ((result->pin1 > 0) && (0 == strcmp("0",(const char*) oldPin1))) {
            /* SIM recovered from PUK state */
            SIM_Status sim_status = getSIMStatus(rid);
            if ( USIM_READY == sim_status || SIM_READY == sim_status ) {
                RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0, rid);
            }
            simStatusChanged++;
        }
    }
    free(count);

    /* PIN2 */
    asprintf(&count,"%d",result->pin2);
    if (forceToUpdate) {
        property_set(propPin2,count);

    } else if (strcmp((const char*)oldPin2,(const char*) count)) {
        /* PIN2 has been changed */
        property_set(propPin2,count);

        if (result->pin2 == 0) {
            /* PIN2 Blocked */
            simStatusChanged++;
        } else if ((result->pin2 > 0) && (0 == strcmp("0",(const char*) oldPin2))) {
            /* PIN2 unBlocked */
            simStatusChanged++;
        }
    }
    free(count);

    /* PUK1 */
    asprintf(&count,"%d",result->puk1);
    if (forceToUpdate) {
        property_set(propPuk1,count);

    } else if (strcmp((const char*)oldPuk1,(const char*) count)) {
        /* PUK1 has been changed */
        property_set(propPuk1,count);

        if (result->puk1 == 0) {
            /* PUK1 Blocked */
            simStatusChanged++;
        }
    }
    free(count);

    /* PUK2 */
    asprintf(&count,"%d",result->puk2);
    if (forceToUpdate) {
        property_set(propPuk2,count);

    } else if (strcmp((const char*)oldPuk2,(const char*) count)) {
        /* PUK2 has been changed */
        property_set(propPuk2,count);

        if (result->puk2 == 0) {
            /* PUK2 Blocked */
            simStatusChanged++;
        }
    }
    free(count);

    if (simStatusChanged > 0) {
        setSimStatusChanged(rid);
    }
}

extern void getPINretryCount(SimPinCount *result, RIL_Token t, RIL_SOCKET_ID rid)
{
    ATResponse *p_response = NULL;
    int err;
    int ret;
    char *line;

    result->pin1 = -1;
    result->pin2 = -1;
    result->puk1 = -1;
    result->puk2 = -1;


    if (NULL != t) {
        err = at_send_command_singleline("AT+EPINC", "+EPINC:", &p_response, SIM_CHANNEL_CTX);
    } else {
        /* called by pollSIMState */
        err = at_send_command_singleline("AT+EPINC", "+EPINC:", &p_response, getChannelCtxbyProxy());
    }

    if (err < 0) {
        RLOGE("getPINretryCount Fail:%d",err);
         goto done;
    }

    if (p_response->success == 0) {
        goto done;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);

    if (err < 0) {
        RLOGE("get token error");
        goto done;
    }

    err = at_tok_nextint(&line, &(result->pin1));

    if (err < 0) {
        RLOGE("get pin1 fail");
        goto done;
    }

    err = at_tok_nextint(&line, &(result->pin2));

    if (err < 0) {
        RLOGE("get pin2 fail");
        goto done;
    }

    err = at_tok_nextint(&line, &(result->puk1));

    if (err < 0) {
        RLOGE("get puk1 fail");
        goto done;
    }

    err = at_tok_nextint(&line, &(result->puk2));

    if (err < 0) {
        RLOGE("get puk2 fail");
        goto done;
    }

    setPINretryCount(result,rid);

    RLOGD("pin1:%d, pin2:%d, puk1:%d, puk2:%d",
            result->pin1,result->pin2,result->puk1,result->puk2);

done:
    at_response_free(p_response);
}

// SIM ME LOCK SML SBP start
static void setSimSlotLockDeviceLockInfoProp(SimSlotLockDeviceLockInfo *result,
        RIL_SOCKET_ID rid) {
    RIL_SIM_UNUSED_PARM(rid);
    char *property_lock_policy = NULL;

    // Set SIM slot lock device lock remain count property
    asprintf(&property_lock_policy, "%d", result->remain);
    property_set(PROPERTY_SIM_SLOT_LOCK_DEVICE_LOCK_REMAIN_COUNT, property_lock_policy);

    free(property_lock_policy);
}

static void getSimSlotLockDeviceLockInfo(SimSlotLockDeviceLockInfo *result, RIL_Token t,
        RIL_SOCKET_ID rid) {
    ATResponse *p_response = NULL;
    int err;
    char *line;

    result->lock_state = -1;
    result->algo = -1;
    result->maximum = -1;
    result->remain = -1;

    if (NULL != t) {
        err = at_send_command_singleline("AT+ESLBLOB=5", "+ESLBLOB:", &p_response, SIM_CHANNEL_CTX);
    } else {
        err = at_send_command_singleline("AT+ESLBLOB=5", "+ESLBLOB:", &p_response, getChannelCtxbyProxy());
    }

    if (err < 0) {
        RLOGE("getSimSlotLockDeviceLockInfo Fail:%d", err);
         goto done;
    }

    if (p_response->success == 0) {
        goto done;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);

    if (err < 0) {
        RLOGE("getSimSlotLockDeviceLockInfo, get token error");
        goto done;
    }

    err = at_tok_nextint(&line, &(result->lock_state));

    if (err < 0) {
        RLOGE("getSimSlotLockDeviceLockInfo, get lock_state fail");
        goto done;
    }

    err = at_tok_nextint(&line, &(result->algo));

    if (err < 0) {
        RLOGE("getSimSlotLockDeviceLockInfo, get algo fail");
        goto done;
    }

    err = at_tok_nextint(&line, &(result->maximum));

    if (err < 0) {
        RLOGE("getSimSlotLockDeviceLockInfo, get maximum fail");
        goto done;
    }

    err = at_tok_nextint(&line, &(result->remain));

    if (err < 0) {
        RLOGE("getSimSlotLockDeviceLockInfo, get remain fail");
        goto done;
    }

    setSimSlotLockDeviceLockInfoProp(result, rid);

    RLOGD("lock_state:%d, algo:%d, maximum:%d, remain:%d",
            result->lock_state, result->algo, result->maximum, result->remain);

done:
    at_response_free(p_response);
}
// SIM ME LOCK SML SBP end

static SIM_Status getSIMStatusWithRetry(RIL_SOCKET_ID rid) {
    SIM_Status sim_status = SIM_ABSENT;
    int count = 0;
    do {
       sim_status = getSIMStatus(rid);
       if (SIM_BUSY == sim_status) {
           sleepMsec(200);
           count++;     // to avoid block; if the busy time is too long; need to check modem.
           if (count == 30) break;
       }
    } while (SIM_BUSY == sim_status);
    return sim_status;
}

/** Returns SIM_NOT_READY or USIM_NOT_READY on error */
SIM_Status getSIMStatus(RIL_SOCKET_ID rid)
{
    //MTK-START [03182012][APLS00252888] This function is called
    // from many functions in different threads.
    if (RIL_SOCKET_1 == rid)
        pthread_mutex_lock(&simStatusMutex1);
    else
        pthread_mutex_lock(&simStatusMutex2);
    //MTK-END  [03182012][APLS00252888]

    ATResponse *p_response = NULL;
    int err;
    int ret;
    char *cpinLine;
    char *cpinResult;
    int isUsim = isUsimDetect[rid];
    SimPinCount retryCounts;
    // MTK-START: AOSP SIM PLUG IN/OUT
    int esimsValue = -1;
    char propEsimsCause[PROPERTY_VALUE_MAX] = { 0 };

    property_get(PROPERTY_ESIMS_CAUSE, propEsimsCause, "-1");
    esimsValue = atoi(propEsimsCause);
    RLOGD("esimsValue: %s,%d", propEsimsCause,esimsValue);
    // MTK-END
    // JB MR1, it will request sim status after receiver iccStatusChangedRegistrants,
    // but MD is off in the mean time, so it will get the exception result of CPIN.
    // For this special case, handle it specially.
    // check md off and sim inserted status, then return the result directly instead of request CPIN.
    // not insert: return SIM_ABSENT, insert: return SIM_NOT_READY or USIM_NOT_READY
    if (s_md_off) {
        int slot = (1 << getMappingSIMByCurrentMode(rid));
        RLOGI("getSIMStatus s_md_off: %d slot: %d rid: %d", s_md_off, slot, rid);
        if (!(slot & sim_inserted_status)) {
            ret = SIM_ABSENT;
            goto done;
        } else {
            if(isUsim == 1) {
                ret = USIM_NOT_READY;
            } else {
                ret = SIM_NOT_READY;
            }
            goto done;
        }
    }

    if (getRadioState(rid) == RADIO_STATE_UNAVAILABLE) {
        if(isUsim == 1) {
            ret = USIM_NOT_READY;
        } else {
            ret = SIM_NOT_READY;
        }
        goto done;
    }

    err = at_send_command_singleline("AT+CPIN?", "+CPIN:", &p_response, getChannelCtxbyProxy());

    if (err != 0) {
        if (err == AT_ERROR_INVALID_THREAD) {
            ret = SIM_BUSY;
        } else {
            if(isUsim == 1) {
                ret = USIM_NOT_READY;
            } else {
                ret = SIM_NOT_READY;
            }
        }
        goto done;
    }

    if (p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            case CME_SIM_BUSY:
                RLOGD("getSIMStatus: CME_SIM_BUSY");
                ret = SIM_BUSY;
                goto done;
                break;
            case CME_SIM_NOT_INSERTED:
                RLOGD("getSIMStatus: CME_SIM_NOT_INSERTED");
                // MTK-START: AOSP SIM PLUG IN/OUT
                if (ESIMS_CAUSE_SIM_NO_INIT == esimsValue) {
                    ret = UICC_NO_INIT;
                } else {
                    ret = SIM_ABSENT;
                }
                // MTK-END
                goto done;
                break;
            case CME_SIM_FAILURE:
                ret = SIM_ABSENT;
                goto done;
                break;
            case CME_SIM_WRONG:
                getPINretryCount(&retryCounts, NULL, rid);
                RLOGD("SIM wrong: pin1:%d, pin2:%d, puk1:%d, puk2:%d",
                retryCounts.pin1,retryCounts.pin2,retryCounts.puk1,retryCounts.puk2);
                if (retryCounts.pin1 == 0 && retryCounts.puk1 == 0)
                {
                    if (isUsim == 1) {
                        ret = USIM_PERM_BLOCKED; // PERM_DISABLED
                    } else {
                        ret = SIM_PERM_BLOCKED; // PERM_DISABLED
                    }
                } else if (retryCounts.pin1 == -1 && retryCounts.puk1 == -1 &&
                        retryCounts.pin2 == -1 && retryCounts.puk2 == -1 &&
                        aIs_sim_off_by_IMEI_lock[rid] == false) {
                    /* Only for FTA mode */
                    char property_value[PROPERTY_VALUE_MAX] = { 0 };
                    property_get("vendor.gsm.gcf.testmode", property_value, "0");
                    RLOGD("property_value:%s",property_value);
                    if (atoi(property_value) == 2) {
                        if (isUsim == 1) {
                            ret = USIM_NOT_READY;
                        } else {
                            ret = SIM_NOT_READY;
                        }
                    } else {
                        ret = SIM_ABSENT;
                    }
                } else {
                    if(isUsim == 1) {
                        ret = USIM_NOT_READY;
                    } else {
                        ret = SIM_NOT_READY;
                    }
                }
                goto done;
                break;
            default:
                if(isUsim == 1) {
                    ret = USIM_NOT_READY;
                } else {
                    ret = SIM_NOT_READY;
                }
                goto done;
        }
    }
    /* CPIN? has succeeded, now look at the result */

    cpinLine = p_response->p_intermediates->line;
    err = at_tok_start (&cpinLine);

    if (err < 0) {
        if(isUsim == 1) {
            ret = USIM_NOT_READY;
        } else {
            ret = SIM_NOT_READY;
        }
        goto done;
    }

    err = at_tok_nextstr(&cpinLine, &cpinResult);

    if (err < 0) {
        if(isUsim == 1) {
            ret = USIM_NOT_READY;
        } else {
            ret = SIM_NOT_READY;
        }
        goto done;
    }

    if (0 == strcmp (cpinResult, "SIM PIN")) {
        if (isUsim == 1) {
            ret = USIM_PIN;
        } else {
            ret = SIM_PIN;
        }
        goto done;
    } else if (0 == strcmp (cpinResult, "SIM PUK")) {
        if (isUsim == 1) {
            ret = USIM_PUK;
        } else {
            ret = SIM_PUK;
        }
        goto done;
    } else if (0 == strcmp (cpinResult, "PH-NET PIN") ||
               0 == strcmp (cpinResult, "PH-NET PUK")) {
        if (isUsim == 1) {
            ret = USIM_NP;
        } else {
            ret = SIM_NP;
        }
        goto done;
    } else if (0 == strcmp (cpinResult, "PH-NETSUB PIN") ||
               0 == strcmp (cpinResult, "PH-NETSUB PUK")) {
        if (isUsim == 1) {
            ret = USIM_NSP;
        } else {
            ret = SIM_NSP;
        }
        goto done;
    } else if (0 == strcmp (cpinResult, "PH-SP PIN") ||
               0 == strcmp (cpinResult, "PH-SP PUK")) {
        if (isUsim == 1) {
            ret = USIM_SP;
        } else {
            ret = SIM_SP;
        }
        goto done;
    } else if (0 == strcmp (cpinResult, "PH-CORP PIN") ||
               0 == strcmp (cpinResult, "PH-CORP PUK")) {
        if (isUsim == 1) {
            ret = USIM_CP;
        } else {
            ret = SIM_CP;
        }
        goto done;
    } else if (0 == strcmp (cpinResult, "PH-FSIM PIN") ||
               0 == strcmp (cpinResult, "PH-FSIM PUK")) {
        if (isUsim == 1) {
            ret = USIM_SIMP;
        } else {
            ret = SIM_SIMP;
        }
        goto done;
    } else if (isRjioRsuSupport() && 0 == strcmp(cpinResult, "SIM PIN-DEFER")) {
        /* Subsidy lock feature only */
        if (isUsim == 1) {
            ret = USIM_NOT_READY;
        } else {
            ret = SIM_NOT_READY;
        }
        pinDefer[rid] = 1;
        goto done;
    } else if (0 == strcmp(cpinResult, "CARD_REBOOT")) {
        ret = CARD_REBOOT;
        goto done;
    } else if (0 == strcmp(cpinResult, "CARD_RESTRICTED")) {
        if (isUsim == 1) {
            ret = USIM_RESTRICTED;
        } else {
            ret = SIM_RESTRICTED;
        }
        goto done;
    } else if (0 != strcmp(cpinResult, "READY"))  {
        /* we're treating unsupported lock types as "sim absent" */
        ret = SIM_ABSENT;
        goto done;
    }

    at_response_free(p_response);
    p_response = NULL;
    cpinResult = NULL;

    if (isUsim == 1) {
        ret = USIM_READY;
    } else {
        ret = SIM_READY;
    }

done:
    RLOGD("getSIMStatus: ret %d", ret);
    at_response_free(p_response);
    //MTK-START [03182012][APLS00252888] This function is called
    // from many functions in different threads.
    if (RIL_SOCKET_1 == rid)
        pthread_mutex_unlock(&simStatusMutex1);
    else
        pthread_mutex_unlock(&simStatusMutex2);
    //MTK-END  [03182012][APLS00252888]
    return ret;
}

int correctPinStatus(int sim_state, char *fac, RIL_SOCKET_ID rid) {
    ATResponse*     p_response = NULL;
    int             err = -1;
    int             status = -1, result = RIL_PINSTATE_UNKNOWN;
    char*           cmd = NULL;
    char*           line = NULL;
    SimPinCount retryCounts;

    asprintf(&cmd, "AT+CLCK=\"%s\",2", fac);
    err = at_send_command_singleline(cmd, "+CLCK:", &p_response, getChannelCtxbyProxy());

    free(cmd);

    if (err < 0 || p_response == NULL || (p_response != NULL && p_response->success == 0)) {
        RLOGE("[correctPinStatus] Fail to get facility lock");
    } else {
        line = p_response->p_intermediates->line;
        err = at_tok_start(&line);
        if (err < 0) goto error;
        err = at_tok_nextint(&line, &status);
        if (err < 0) goto error;
    }

    if (status == 0) {
        result = RIL_PINSTATE_DISABLED;
    } else if (status == 1 && (sim_state == SIM_READY || sim_state == USIM_READY)){
        getPINretryCount(&retryCounts, NULL, rid);
        if (strcmp(fac, "SC") == 0) {
            result = RIL_PINSTATE_ENABLED_VERIFIED;
        } else {
            if (retryCounts.pin2 == 0 && retryCounts.puk2 > 0) {
                result = RIL_PINSTATE_ENABLED_BLOCKED;
            } else if (retryCounts.puk2 == 0) {
                result = RIL_PINSTATE_ENABLED_PERM_BLOCKED;
            } else {
                result = RIL_PINSTATE_ENABLED_NOT_VERIFIED;
            }
        }
    }
    at_response_free(p_response);
    return result;
error:
    RLOGE("[correctPinStatus] CLCK error!");
    at_response_free(p_response);
    return RIL_PINSTATE_UNKNOWN;
}

bool getIccIdbySimSlot(char *iccid, RIL_SOCKET_ID rid) {
    bool isIccIdReady = false;
    char iccIdProp[PROPERTY_VALUE_MAX] = {0};
    property_get(PROPERTY_ICCID_SIM[getMappingSIMByCurrentMode(rid)], iccIdProp, "");

    if (strlen(iccIdProp) > 0) {
        isIccIdReady = true;

        if (strcmp(iccIdProp, "N/A") != 0) {
            property_get(PROPERTY_ICCID_SIM[getMappingSIMByCurrentMode(rid)], iccid, "");
        }
    }

    return isIccIdReady;
}

bool getIccAtrBySimSlot(char *iccid, RIL_SOCKET_ID rid) {
    // TODO: didn't implement currently
    return true;
}

/**
 * Get the current card status.
 *
 * This must be freed using freeCardStatus.
 * @return: On success returns RIL_E_SUCCESS
 */
int getCardStatus(RIL_CardStatus_v8 **pp_card_status, RIL_SOCKET_ID rid) {
    static RIL_AppStatus app_status_array[] = {
        // SIM_ABSENT = 0
        { RIL_APPTYPE_UNKNOWN, RIL_APPSTATE_UNKNOWN, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // SIM_NOT_READY = 1
        { RIL_APPTYPE_SIM, RIL_APPSTATE_DETECTED, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // SIM_READY = 2
        { RIL_APPTYPE_SIM, RIL_APPSTATE_READY, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // SIM_PIN = 3
        { RIL_APPTYPE_SIM, RIL_APPSTATE_PIN, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // SIM_PUK = 4
        { RIL_APPTYPE_SIM, RIL_APPSTATE_PUK, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_BLOCKED, RIL_PINSTATE_UNKNOWN },
        // SIM_NETWORK_PERSONALIZATION = 5
        { RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_NETWORK,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // USIM_READY = 6
        { RIL_APPTYPE_USIM, RIL_APPSTATE_READY, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // USIM_PIN = 7
        { RIL_APPTYPE_USIM, RIL_APPSTATE_PIN, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // USIM_PUK = 8
        { RIL_APPTYPE_USIM, RIL_APPSTATE_PUK, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_BLOCKED, RIL_PINSTATE_UNKNOWN },
        // SIM_BUSY = 9
        { RIL_APPTYPE_UNKNOWN, RIL_APPSTATE_UNKNOWN, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // SIM_NP = 10,
        { RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_NETWORK,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // SIM_NSP = 11,
        { RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_NETWORK_SUBSET,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // SIM_SP = 12,
        { RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_SERVICE_PROVIDER,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // SIM_CP = 13,
        { RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_CORPORATE,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // SIM_SIMP =14
        { RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_SIM,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // SIM_PERM_BLOCKED = 15 // PERM_DISABLED
        { RIL_APPTYPE_SIM, RIL_APPSTATE_PIN, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_PERM_BLOCKED, RIL_PINSTATE_UNKNOWN },
        // USIM_PERM_BLOCKED = 16 // PERM_DISABLED
        { RIL_APPTYPE_USIM, RIL_APPSTATE_PIN, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_PERM_BLOCKED, RIL_PINSTATE_UNKNOWN },
        // RUIM_ABSENT = 17
        { RIL_APPTYPE_UNKNOWN, RIL_APPSTATE_UNKNOWN, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // RUIM_NOT_READY = 18
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_DETECTED, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // RUIM_READY = 19
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_READY, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // RUIM_PIN = 20
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_PIN, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // RUIM_PUK = 21
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_PUK, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_BLOCKED, RIL_PINSTATE_UNKNOWN },
        // RUIM_NETWORK_PERSONALIZATION = 22
        { RIL_APPTYPE_RUIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_NETWORK,
           NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // USIM_NP = 23,
        { RIL_APPTYPE_USIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_NETWORK,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // USIM_NSP = 24,
        { RIL_APPTYPE_USIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_NETWORK_SUBSET,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // USIM_SP = 25,
        { RIL_APPTYPE_USIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_SERVICE_PROVIDER,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // USIM_CP = 26,
        { RIL_APPTYPE_USIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_CORPORATE,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // USIM_SIMP =27
        { RIL_APPTYPE_USIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_SIM,
           NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // USIM_NOT_READY =28
        { RIL_APPTYPE_USIM, RIL_APPSTATE_DETECTED, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN }
    };
    RIL_CardState card_state;
    int num_apps;

//MTK-START [mtkXXXXX][120208][APLS00109092] Replace "RIL_UNSOL_SIM_MISSING in RIL.java" with "acively query SIM missing status"
//MTK-END [mtkXXXXX][120208][APLS00109092] Replace "RIL_UNSOL_SIM_MISSING in RIL.java" with "acively query SIM missing status"

    SIM_Status sim_status = SIM_ABSENT;
    int count = 0;
    int isUsim = isUsimDetect[rid];
    int isCDMA = isCDMADetect[rid];
    char iccId[PROPERTY_VALUE_MAX] = {0};

    RLOGD("getCardStatus isUsim %d, isCDMA %d", isUsim, isCDMA);

    #ifdef MTK_TC1_FEATURE
    if(isUsim == -1)
    {
        requestGetIccType(rid);
        isUsim = isUsimDetect[rid];
    }
    #endif

    if (isUsim == 2) {
        // No USIM and SIM so no SIM card for MD1
        sim_status = SIM_ABSENT;

        // MTK-START: AOSP SIM PLUG IN/OUT
        int esimsValue = -1;
        char propEsimsCause[PROPERTY_VALUE_MAX] = { 0 };

        property_get(PROPERTY_ESIMS_CAUSE, propEsimsCause, "-1");
        esimsValue = atoi(propEsimsCause);
        // MTK-END

        // MTK-START: AOSP SIM PLUG IN/OUT
        if (ESIMS_CAUSE_SIM_NO_INIT == esimsValue) {
            sim_status = UICC_NO_INIT;
        }
        // MTK-END
    } else {
        do{
            sim_status = getSIMStatus(rid);
           if (SIM_BUSY == sim_status )
           {
               sleepMsec(200);
               count++;     //to avoid block; if the busy time is too long; need to check modem.
               // Since VSIM might take more time to initilaization, we stop retry continuously to
               // to avoid the occuppy the channel too long.
               // Reference CR: ALPS02414879
               if(count == 30 || isVsimEnabledByRid(rid))
               {
                    if(isUsim == 1) {
                        sim_status = USIM_NOT_READY; //to avoid exception in RILD
                    } else {
                        sim_status = SIM_NOT_READY; //to avoid exception in RILD
                    }
                    // External SIM [Start]
                    RLOGE("Error in getSIM Status, isVsimEnabled()=%d", isVsimEnabledByRid(rid));
                    if (isVsimEnabledByRid(rid)) {
                        isRecoverySimState[rid] = 1;
                    }
                    // External SIM [End]
                    break;
               }
           }
           if ( USIM_READY == sim_status || SIM_READY == sim_status ) {
                detectTestSim(rid);
                //setRadioState(RADIO_STATE_SIM_READY,rid);
                /* the following property is used for auto test */
                property_set((const char *)PROPERTY_RIL_SIM_READY, (const char *)"true");
                if (isRjioRsuSupport()) {
                    /* Subsidy lock feature only */
                    for (int i = 0; i < getSimCount(); i++) {
                        if (i != rid && pinDefer[i] == 1) {
                            setSimStatusChanged(i);
                            RLOGD("Send SIM status changed due to SIM PIN DEFER.");
                            pinDefer[i] = 0;
                        }
                    }
                }
            } else if ((USIM_NP == sim_status || SIM_NP == sim_status )  // CPIN  PH-NET PIN or PH-NET PUK
                    || (USIM_PIN == sim_status || SIM_PIN == sim_status)  // CPIN PIN
                    || (USIM_PUK == sim_status || SIM_PUK == sim_status) ) {  // CPIN PUK
                property_set((const char *)PROPERTY_RIL_SIM_READY, (const char *)"false");
                if (isRjioRsuSupport()) {
                    /* Subsidy lock feature only */
                    for (int i = 0; i < getSimCount(); i++) {
                        if (i != rid && pinDefer[i] == 1) {
                            setSimStatusChanged(i);
                            RLOGD("Send SIM status changed due to SIM PIN DEFER.");
                            pinDefer[i] = 0;
                        }
                    }
                }
           } else if (SIM_BUSY != sim_status) {
                /* the following property is used for auto test */
                RLOGD("getCardStatus, SIM is neither not ready nor busy");
                property_set((const char *)PROPERTY_RIL_SIM_READY, (const char *)"false");
           }
        }while(SIM_BUSY == sim_status);
    }

    // From radio hidl v1.2, we shall ensure get Icc id before notify SIM card is present
    if (sim_status == USIM_NOT_READY || sim_status == SIM_NOT_READY || !getIccIdbySimSlot(iccId, rid)) {
        // SIM not ready so do nothing!
        RLOGE("fail to get card status in rid %d", rid);
        return RIL_E_GENERIC_FAILURE;
    }
    // MTK-START: AOSP SIM PLUG IN/OUT
    if (sim_status != UICC_NO_INIT && sim_status != SIM_ABSENT && sim_status != USIM_NOT_READY &&
            sim_status != SIM_NOT_READY) {
    // MTK-END
        queryEfDir(rid);
    }

    if (sim_status == SIM_ABSENT) {
        card_state = RIL_CARDSTATE_ABSENT;
        num_apps = 0;
    // MTK-START: AOSP SIM PLUG IN/OUT
    } else if (sim_status == UICC_NO_INIT || sim_status == CARD_REBOOT) {
        card_state = RIL_CARDSTATE_PRESENT;
        num_apps = 0;
    // MTK-END
    } else if (sim_status == USIM_RESTRICTED || sim_status == SIM_RESTRICTED) {
        card_state = RIL_CARDSTATE_RESTRICTED;
        num_apps = 1;
    } else {
        card_state = RIL_CARDSTATE_PRESENT;
        num_apps = 1;
    }

    // Allocate and initialize base card status.
    RIL_CardStatus_v8 *p_card_status = calloc(1, sizeof(RIL_CardStatus_v8));
    assert(p_card_status != NULL);
    p_card_status->card_state = card_state;
    p_card_status->universal_pin_state = RIL_PINSTATE_UNKNOWN;
    // Fot VTS test. Set the default index as -1.
    p_card_status->gsm_umts_subscription_app_index = -1;
    p_card_status->cdma_subscription_app_index = -1;
    p_card_status->ims_subscription_app_index = -1;
    p_card_status->num_applications = num_apps;

    // Initialize application status
    int i;
    for (i = 0; i < RIL_CARD_MAX_APPS; i++) {
        p_card_status->applications[i] = app_status_array[SIM_ABSENT];
    }

    // Pickup the appropriate application status
    // that reflects sim_status for gsm.
    if (num_apps != 0) {
        // TODO, MR1 Migration
        // UIM appstatus array may out of bounds on mtk solution
        // Only support one app, gsm
        //p_card_status->num_applications = 2;
        //p_card_status->cdma_subscription_app_index = 1;
        //p_card_status->applications[1] = app_status_array[sim_status + RUIM_ABSENT];

        p_card_status->num_applications = 1;
        p_card_status->gsm_umts_subscription_app_index = 0;

        // Get the correct app status
        p_card_status->applications[0] = app_status_array[sim_status];
        if (sim_status == SIM_READY || sim_status == USIM_READY) {
            p_card_status->applications[0].pin1 = correctPinStatus(sim_status, "SC", rid);
            p_card_status->applications[0].pin2 = correctPinStatus(sim_status, "FD", rid);
        }
        if (isUsim == 1) {
            // Set USIM AID and application label
            AidInfo* pAidInfo = getAidInfo(rid, AID_USIM);
            p_card_status->applications[0].aid_ptr = pAidInfo->aid;
            p_card_status->applications[0].app_label_ptr = pAidInfo->appLabel;
        }
    }

    // Parameters add from radio hidl v1.2.
    p_card_status->physicalSlotId = rid; //un-used
    p_card_status->atr = NULL;  // not support

    // Parameters add from radio hidl v1.4.
    p_card_status->eid = NULL;  // not support

    p_card_status->iccId = (char *)calloc(1, PROPERTY_VALUE_MAX);
    strncpy(p_card_status->iccId, iccId, strlen(iccId));
    (p_card_status->iccId)[strlen(iccId)] = '\0';

    *pp_card_status = p_card_status;
    return RIL_E_SUCCESS;
}

#ifdef MTK_TC1_FEATURE
void requestGetIccType(RIL_SOCKET_ID rid)
{
    ATResponse *    p_response  = NULL;
    int             err         = 0;
    char *          line        = NULL;
    int             ret         = 0;
    int             simId       = getMappingSIMByCurrentMode(rid);
    const char *    propUicc    = PROPERTY_RIL_UICC_TYPE[simId];

    err = at_send_command_singleline("AT+EUSIM?", "+EUSIM:", &p_response, getChannelCtxbyProxy());

    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &ret);
    if (err < 0) goto error;

    isUsimDetect[rid] = ret;

    if (ret == 1) {
        property_set(propUicc,"USIM");
    }
    else {
        property_set(propUicc,"SIM");
    }

    LOGD("[RIL%d] AT+EUSIM? return %d, isUsimDetect[%d] = %d", rid, ret, rid, isUsimDetect[rid]);
    at_response_free(p_response);
    return;
error:
    LOGE("[RIL%d] AT+EUSIM? return ERROR", rid);
    at_response_free(p_response);
}
#endif

/**
 * Free the card status returned by getCardStatus
 */
static void freeCardStatus(RIL_CardStatus_v8 *p_card_status) {
    free(p_card_status);
}

/**
 * SIM ready means any commands that access the SIM will work, including:
 *  AT+CPIN, AT+CSMS, AT+CNMI, AT+CRSM
 *  (all SMS-related commands)
 */
extern void pollSIMState (void *param)
{
    ATResponse *p_response;
    int ret;
    RIL_SOCKET_ID rid = *((RIL_SOCKET_ID *) param);
    SimPinCount retryCounts;

    switch(getSIMStatus(rid)) {
        case SIM_PIN:
        case SIM_PUK:
        case SIM_NP:
        case SIM_NSP:
        case SIM_SP:
        case SIM_CP:
        case SIM_SIMP:
        case USIM_NP:
        case USIM_NSP:
        case USIM_SP:
        case USIM_CP:
        case USIM_SIMP:
        case USIM_PIN:
        case USIM_PUK:
        case SIM_PERM_BLOCKED: // PERM_DISABLED due to CME_SIM_WRONG
        case USIM_PERM_BLOCKED: // PERM_DISABLED due to CME_SIM_WRONG
            getPINretryCount(&retryCounts, NULL, rid);
        case SIM_ABSENT:
            RLOGI("SIM ABSENT or LOCKED");
            RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0, rid);
        return;

        case SIM_BUSY:
            if (rid == RIL_SOCKET_1)
            {
                RIL_requestProxyTimedCallback (pollSIMState, param, &TIMEVAL_SIMPOLL,
                RIL_CMD_3, "pollSIMState");
            }
            else if (SIM_COUNT >= 2) {
                RIL_requestProxyTimedCallback (pollSIMState, param, &TIMEVAL_SIMPOLL,
                RIL_CMD2_3, "pollSIMState");
            }
            return;

        case SIM_READY:
        case USIM_READY:
            getPINretryCount(&retryCounts, NULL, rid);
            RLOGI("SIM_READY");
            RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0, rid);
            return;
        default:
            RLOGI("SIM state: default case");
            RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0, rid);
            return;
    }
}

static void requestGetSimStatus(void *data, size_t datalen, RIL_Token t)
{
    RIL_CardStatus_v8 *p_card_status = NULL;
    char *p_buffer;
    int buffer_size;
    int appId = -1, session = -1;
    int iResult = RIL_E_GENERIC_FAILURE;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    SimPinCount retryCounts;
    AidInfo *pAidInfo = NULL;
    SessionInfo *pSession = NULL;

    RIL_SIM_UNUSED_PARM(data);
    RIL_SIM_UNUSED_PARM(datalen);

    int result = getCardStatus(&p_card_status,rid);
    if (result == RIL_E_SUCCESS) {
        p_buffer = (char *)p_card_status;
        buffer_size = sizeof(*p_card_status);

        //ALPS01021099: We get card status from getCardStatus. In that function, we will know the
        //SIM card is present or not no matter the modem is on or off. But we can not send AT command
        //to modem when modem is off. Thus, we need to check modem status.
        RLOGD("card status = %d, modem status = %d",p_card_status->card_state , !s_md_off);
        if (RIL_CARDSTATE_PRESENT == p_card_status->card_state && !s_md_off) {
            getPINretryCount(&retryCounts, t, rid);
            // MTK-START: ISIM
            if (p_card_status->applications[0].pin1 != RIL_PINSTATE_ENABLED_NOT_VERIFIED) {
                // To ensure get correct application status in case of ISIM application has been initialized
                // Query if the channel has been opened on indicated application
                // The application session may be closed by modem so we have to check it everytime.
                int sessionId = queryIccApplicationChannel(0, t);
                if (sessionId > 0) {
                    RLOGD("requestGetSimStatus, ISIM has been initialized before.");
                    iResult = RIL_E_SUCCESS;
                } else {
                    // Turn on ISIM application
                    iResult = turnOnIsimApplication(t, rid);
                    if (iResult == RIL_E_SUCCESS) {
                        pSession = getIsimSessionInfo(rid);
                        sessionId = pSession->session;
                    }
                }
                if (iResult == RIL_E_SUCCESS) {
                    iResult = getIccApplicationStatus(&p_card_status, rid, sessionId);
                    if (iResult == RIL_E_SUCCESS) {
                        // FIXME: if need to check CDMA type and adjust num_application value?
                        p_buffer = (char *)p_card_status;
                        buffer_size = sizeof(*p_card_status);
                    }
                }
            }
            // MTK-END
        }
    } else {
        p_buffer = NULL;
        buffer_size = 0;
    }
    RIL_onRequestComplete(t, result, p_buffer, buffer_size);
    if (p_card_status != NULL) {
        freeCardStatus(p_card_status);
    }
}

static void simSecurityOperation(void *data, size_t datalen, RIL_Token t, SIM_Operation op)
{
    ATResponse   *p_response = NULL;
    int           err = -1;
    char*         cmd = NULL;
    char*         line = NULL;
    const char**  strings = (const char**)data;;
    RIL_Errno     ret = RIL_E_GENERIC_FAILURE;
    SimPinCount   retryCount;
    int           response;
    RIL_SOCKET_ID         rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    SIM_Status    sim_status;
    int           i = 0;
    int           bIgnoreCmd = FALSE;
    int           opRetryCount = -1;
    int           isSimPinEnabled = 0;

    switch (op) {
        case ENTER_PIN1:
        case ENTER_PUK1:
            if ( datalen == 2*sizeof(char*) ) {
                asprintf(&cmd, "AT+CPIN=\"%s\"", strings[0]);
            } else if ( datalen == 3*sizeof(char*) ) {
                asprintf(&cmd, "AT+EPIN1=\"%s\",\"%s\"", strings[0], strings[1]);
            } else
                goto done;

            err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);

            break;
        case ENTER_PIN2:
        case ENTER_PUK2:
            if ( datalen == 2*sizeof(char*) ) {
                asprintf(&cmd, "AT+EPIN2=\"%s\"", strings[0]);
            } else if ( datalen == 3*sizeof(char*) ) {
                asprintf(&cmd, "AT+EPIN2=\"%s\",\"%s\"", strings[0], strings[1]);
            } else
                goto done;

            err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);

            break;
        case CHANGE_PIN1:
            /* Check if SIM pin has enabled */
            asprintf(&cmd, "AT+CLCK=\"SC\",2");
            err = at_send_command_singleline(cmd, "+CLCK:", &p_response, SIM_CHANNEL_CTX);
            free(cmd);

            if (err < 0) {
                RLOGE("query SIM PIN lock:%d",err);
                 goto done;
            }

            if (p_response->success == 0) {
                goto done;
            }

            line = p_response->p_intermediates->line;

            err = at_tok_start(&line);
            if (err < 0) {
                RLOGE("query SIM PIN lock, get token error");
                goto done;
            }

            err = at_tok_nextint(&line, &(isSimPinEnabled));
            if (err < 0) {
                RLOGE("query SIM PIN lock, get result fail");
                goto done;
            }

            RLOGE("query SIM PIN lock, isSimPinEnabled: %d", isSimPinEnabled);
            at_response_free(p_response);
            p_response = NULL;

            if (isSimPinEnabled == 0) {
                ret = RIL_E_REQUEST_NOT_SUPPORTED;
                goto done;
            }
            // Fall through
            TELEPHONYWARE_FALLTHROUGH;

        case CHANGE_PIN2:
            /*  UI shall handle CHANGE PIN only operated on PIN which is enabled and nonblocking state. */
            if ( datalen == 3*sizeof(char*) ) {
                if (CHANGE_PIN1 == op) {
                    SIM_Status sim_status = getSIMStatus(rid);
                    if ((sim_status == SIM_PIN)||(sim_status == USIM_PIN)) {
                        op = ENTER_PIN1;
                        /* Solve CR - ALPS00260076 PIN can not be changed by using SS string when PIN is dismissed
                           Modem does not support multiple AT commands, so input PIN1 first */
                        asprintf(&cmd, "AT+CPIN=\"%s\"", strings[0]);
                        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
                        bIgnoreCmd = TRUE;
                        if (p_response->success > 0)
                        {
                           bIgnoreCmd = FALSE;
                           free(cmd);
                           asprintf(&cmd, "AT+CPWD=\"SC\",\"%s\",\"%s\"", strings[0], strings[1]);
                           /* Wait for SIM ready before changing PIN password
                              Used the same retry scenario with getCardStatus */
                           for (i = 0; i < 30; i++) {
                               RLOGD("Wait for SIM ready, try count:%d", i+1);
                               sleepMsec(200);
                               sim_status = getSIMStatus(rid);
                               if ((sim_status == SIM_READY) || (sim_status == USIM_READY)) {
                                  RLOGD("SIM ready");
                                  break;
                               }
                           }/* End of for */
                        }
                    } else {
                        asprintf(&cmd, "AT+CPWD=\"SC\",\"%s\",\"%s\"", strings[0], strings[1]);
                    }
                } else {
                    asprintf(&cmd, "AT+CPWD=\"P2\",\"%s\",\"%s\"", strings[0], strings[1]);
                }
            } else
                goto done;

            if (!bIgnoreCmd) {
                // [mtk02772] p_response should be free before use it, or it will be leakage
                at_response_free(p_response);
                p_response = NULL;
               err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
            }
            break;

        default:
            goto done;
            break;
    }
    free(cmd);

    if (err != 0) {
        /* AT runtime error */
        at_response_free(p_response);
        p_response = NULL;
        goto done;
    }

    getPINretryCount(&retryCount, t, rid);

    if ((ENTER_PIN1 == op) || (CHANGE_PIN1 == op)) {
        opRetryCount = retryCount.pin1;
    } else if ((ENTER_PIN2 == op) || (CHANGE_PIN2 == op)) {
        opRetryCount = retryCount.pin2;
    } else if (ENTER_PUK1 == op) {
        opRetryCount = retryCount.puk1;
    } else if (ENTER_PUK2 == op) {
        opRetryCount = retryCount.puk2;
    }
    RLOGD("simSecurityOperation: op=%d, RetryCount=%d", op, opRetryCount);

    if (p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            case CME_INCORRECT_PASSWORD:
                ret = RIL_E_PASSWORD_INCORRECT;
                break;
            case CME_SIM_PIN_REQUIRED:
            case CME_SIM_PUK_REQUIRED:
                ret = RIL_E_PASSWORD_INCORRECT;
                break;
            case CME_SIM_PIN2_REQUIRED:
                ret = RIL_E_SIM_PIN2;
                break;
            case CME_SIM_PUK2_REQUIRED:
                ret = RIL_E_SIM_PUK2;
                break;
            case CME_SIM_FAILURE:
            case CME_UNKNOWN:
                if ((ENTER_PUK1 == op) || (ENTER_PUK2 == op)) {
                    ret = RIL_E_INVALID_SIM_STATE;
                } else {
                    ret = RIL_E_REQUEST_NOT_SUPPORTED;
                }
                break;
            default:
                ret = RIL_E_REQUEST_NOT_SUPPORTED;
                break;
        }
    } else {
        /* Only for FTA mode */
        char property_value[PROPERTY_VALUE_MAX] = { 0 };
        property_get("vendor.gsm.gcf.testmode", property_value, "0");

        if (atoi(property_value) == 2) {
            // 1. Since modem need to init after unlock SIM PIN, we try to query SIM state to check
            //    if modem has complted initialization.
            int count = 0;
            do{
               sim_status = getSIMStatus(rid);
               if (SIM_BUSY == sim_status) {
                   sleepMsec(200);
                   count++;     //to avoid block; if the busy time is too long; need to check modem.
                   if(count == 30) break;
               }
            } while(SIM_BUSY == sim_status);

            // 2. Check if there is techinal problem in SIM card to support lab test PTCRB 27.16
            //    Read any file to ensure there is no technical problem.
            sleepMsec(200);
            ATResponse   *p_responseIccid = NULL;
            asprintf(&cmd, "AT+CRSM=%d,%d,%d,%d,%d", 0xb2, 0x2fe2, 1 , 4, 10);
            err = at_send_command_singleline(cmd, "+CRSM:", &p_responseIccid,
                    getChannelCtxbyProxy());
            free(cmd);
            if (err < 0 || p_responseIccid->success == 0) {
                RLOGE("Fail to read EF_ICCID");
                if (at_get_cme_error(p_responseIccid) == CME_SIM_TECHNICAL_PROBLEM) {
                    RLOGE("Fail to read EF_ICCID, got CME_SIM_TECHNICAL_PROBLEM.");
                    ret = RIL_E_GENERIC_FAILURE;
                }
            } else {
                RLOGD("success to read EF_ICCID");
                ret = RIL_E_SUCCESS;
            }
            at_response_free(p_responseIccid);
            p_responseIccid = NULL;
        } else {
            ret = RIL_E_SUCCESS;
        }
    }
    /* release resource for getPINretryCount usage */
    at_response_free(p_response);
    p_response = NULL;


    if ((ret == RIL_E_SUCCESS)&&((ENTER_PIN1 == op) || (ENTER_PUK1 == op))) {
        if (op == ENTER_PIN1) {
            RLOGD("ENTER_PIN1 Success!");
        } else {
            RLOGD("ENTER_PUK1 Success!");
        }

        setSimStatusChanged(rid);
        setupOpProperty(rid);
        // Dynamic-SBP feature
        RLOGD("PIN unlocked! Setup Dynamic-SBP for RIL:%d", rid);
        setupDynamicSBP(rid);
    }

done:
    RIL_onRequestComplete(t, ret, &opRetryCount, sizeof(opRetryCount));

    if (p_response != NULL) {
        at_response_free(p_response);
        p_response = NULL;
    }
}

static void requestEnterDeviceNetworkDepersonalization(void *data, size_t datalen, RIL_Token t)
{
    ATResponse* p_response = NULL;
    char* cmd = NULL;
    const char** strings = (const char**)data;
    int err = -1;
    int new_state = -1;
    int op = -1;
    int op_remain_count = 0;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    SimSlotLockDeviceLockInfo deviceLockInfo;

    RIL_SIM_UNUSED_PARM(datalen);

    if (strings == NULL || strings[0] == NULL) {
        ret = RIL_E_INVALID_ARGUMENTS;
        RLOGE("handleEnterDepersonalization invalid arguments.");
        goto done;
    }

    op = SML_LOCK_OR_UNLOCK;
    new_state = SML_UNLOCK_DEVICE;
    asprintf(&cmd, "AT+ESLBLOB=%d,\"%s\",%d", op, strings[0], new_state);
    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);

    free(cmd);
    if (err != 0) {
        /* AT runtime error */
        RLOGE("requestEnterDeviceNetworkDepersonalization: err=%d.", err);
        goto done;
    }

    if (p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            case CME_INCORRECT_PASSWORD:
                ret = RIL_E_PASSWORD_INCORRECT;
                break;
            default:
                ret = RIL_E_GENERIC_FAILURE;
                break;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }
    // SIM ME LOCK SBP feature
    getSimSlotLockDeviceLockInfo(&deviceLockInfo, t, rid);
    op_remain_count = deviceLockInfo.remain;

    RLOGD("requestEnterDeviceNetworkDepersonalization: ret=%d.", ret);

done:
    RIL_onRequestComplete(t, ret, &op_remain_count, sizeof(op_remain_count));

    if (p_response != NULL) {
        at_response_free(p_response);
        p_response = NULL;
    }
}

// TODO: requestEnterNetworkDepersonalization
static void requestEnterNetworkDepersonalization(void *data, size_t datalen, RIL_Token t)
{
    ATResponse*     p_response = NULL;
    int             err = -1;
    char*           cmd = NULL;
    const char**    strings = (const char**)data;
    RIL_SOCKET_ID           rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    SIM_Status  sim_status = getSIMStatus(rid);
    RIL_Errno     ret = RIL_E_GENERIC_FAILURE;

    RIL_SIM_UNUSED_PARM(datalen);

    if (SIM_ABSENT == sim_status) {
        ret = RIL_E_SIM_ABSENT;
        goto error;
    }
    if (SIM_NP == sim_status ||
        SIM_NSP == sim_status ||
        SIM_SP == sim_status ||
        SIM_CP == sim_status ||
        SIM_SIMP == sim_status ||
        USIM_NP == sim_status ||
        USIM_NSP == sim_status ||
        USIM_SP == sim_status ||
        USIM_CP == sim_status ||
        USIM_SIMP == sim_status
    ) {
        asprintf(&cmd, "AT+CPIN=\"%s\"", strings[0]);
        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX );

        free(cmd);
        if (p_response->success == 0) {
            switch (at_get_cme_error(p_response)) {
                case CME_NETWORK_PERSONALIZATION_PUK_REQUIRED:
                case CME_INCORRECT_PASSWORD:
                    ret = RIL_E_PASSWORD_INCORRECT;
                    goto error;
                    break;
                case CME_NETWORK_SUBSET_PERSONALIZATION_PUK_REQUIRED:
                    if (SIM_NP == sim_status || USIM_NP == sim_status )
                    {
                        ret = RIL_E_SUCCESS;
                        break;
                    }
                    else if (SIM_NSP == sim_status || USIM_NSP == sim_status )
                    {
                        ret = RIL_E_PASSWORD_INCORRECT;
                        goto error;
                    }else {
                        goto error;
                    }
                case CME_SERVICE_PROVIDER_PERSONALIZATION_PUK_REQUIRED:
                    if (SIM_NP == sim_status || SIM_NSP == sim_status ||
                        USIM_NP == sim_status || USIM_NSP == sim_status )
                    {
                        ret = RIL_E_SUCCESS;
                        break;
                    }
                    else if (SIM_SP == sim_status || USIM_SP == sim_status)
                    {
                        ret = RIL_E_PASSWORD_INCORRECT;
                        goto error;
                    }else {
                        goto error;
                    }
                case CME_CORPORATE_PERSONALIZATION_PUK_REQUIRED:
                    if (SIM_NP == sim_status ||
                        SIM_NSP == sim_status ||
                        SIM_SP == sim_status ||
                        USIM_NP == sim_status ||
                        USIM_NSP == sim_status ||
                        USIM_SP == sim_status
                    )
                    {
                        ret = RIL_E_SUCCESS;
                        break;
                    }
                    else if (SIM_CP == sim_status || USIM_CP == sim_status)
                    {
                        ret = RIL_E_PASSWORD_INCORRECT;
                        goto error;
                    }else {
                        goto error;
                    }
                case CME_PH_FSIM_PUK_REQUIRED:
                    if (SIM_SIMP == sim_status || USIM_SIMP == sim_status)
                    {
                        ret = RIL_E_PASSWORD_INCORRECT;
                        goto error;
                    }
                    else
                    {
                        ret = RIL_E_SUCCESS;
                        break;
                    }
                case CME_OPERATION_NOT_ALLOWED_ERR:
                case CME_OPERATION_NOT_SUPPORTED:
                case CME_UNKNOWN:
                    RLOGE("Unknow error");
                    goto error;
                    break;
                default:
                    RLOGD("Not wrong password or not allowed.");
                    ret = RIL_E_SUCCESS;
                    break;
            }
        } else {
            ret = RIL_E_SUCCESS;
            RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0, rid);
        }
    }
    at_response_free(p_response);
    p_response = NULL;

    setSimStatusChanged(rid);
    // Dynamic-SBP feature: ALPS02832079
    RLOGD("SIM NP unlocked! Setup Dynamic-SBP for RIL:%d", rid);
    setupDynamicSBP(rid);
    RIL_onRequestComplete(t, ret, NULL, 0);
    return;
error:
    at_response_free(p_response);
    p_response = NULL;
    RIL_onRequestComplete(t, ret, NULL, 0);
}

static void requestGetImsi(void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err = -1;
    int count = 0;
    int isSimBusy = 0;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int isSimInsert = 0;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    RIL_SIM_UNUSED_PARM(data);
    RIL_SIM_UNUSED_PARM(datalen);

    do {
        if (0 == isSimInserted(rid)) {
            RLOGE("requestGetImsi. no sim: %d", rid);
            ret = RIL_E_SIM_ABSENT;
            break;
        }
        err = at_send_command_numeric("AT+CIMI", &p_response, SIM_CHANNEL_CTX);
        isSimBusy = 0;
        if (p_response != NULL && p_response->success == 0) {
            switch (at_get_cme_error(p_response)) {
                case CME_SIM_BUSY:
                    RLOGD("AT+CIMI: CME_SIM_BUSY. has sim: %d", isSimInserted(rid));
                    if (0 == isSimInserted(rid)) {
                        break;
                    }
                    sleepMsec(200);
                    count++;  // to avoid block; if the busy time is too long; need to check modem.
                    if (count == 30) break;
                    isSimBusy = 1;
            }
        }
    } while (isSimBusy == 1);

    if (err < 0 || p_response == NULL || p_response->success == 0) {
        RIL_onRequestComplete(t, ret, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS,
            p_response->p_intermediates->line, sizeof(char *));
    }
    at_response_free(p_response);

}

#define CMD_READ_BINARY 176
#define CMD_READ_RECORD 178
#define CMD_GET_RESPONSE 192
#define CMD_UPDATE_BINARY 214
#define CMD_UPDATE_RECORD 220
#define CMD_STATUS 242
#define USIM_TRANSPARENT_EF_MAX_SIZE (128 * 256)

static RIL_Errno sendSimAccessCommand(const char *cmd, ATResponse **pp_response, RIL_SIM_IO_Response *p_sr, RIL_Token t, int sessionId)
{
    int err;
    char *line;

    if (sessionId == 0) {
        // Since modem didn't support CRLA in all version.
        // For compatibility purpose, we use CRSM in case of session Id equal to 0.
        err = at_send_command_singleline(cmd, "+CRSM:", pp_response, SIM_CHANNEL_CTX);
    } else {
        // For session id is not equal to 0,
        //we should use CRLA to indicated target session id.
        err = at_send_command_singleline(cmd, "+CRLA:", pp_response, SIM_CHANNEL_CTX);
    }

    if (err < 0 || (*pp_response)->success == 0) {
        assert(0);
        goto error;
    }

    line = (*pp_response)->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &(p_sr->sw1));
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &(p_sr->sw2));
    if (err < 0) goto error;

    if (at_tok_hasmore(&line)) {
        err = at_tok_nextstr(&line, &(p_sr->simResponse));
        if (err < 0) goto error;
    }

    return RIL_E_SUCCESS;

error:
    return RIL_E_GENERIC_FAILURE;
}

static RIL_Errno usimGetResponse(RIL_SIM_IO_EX_v6 *p_args, RIL_SIM_IO_Response *p_res,RIL_Token t)
{
    int pathlen = 0, no_path = 0, orig_pathLen = 0;
    size_t len;
    RIL_SIM_IO_Response sr;
    char * line = NULL;
    char * cmd = NULL;
    ATResponse * p_response = NULL;
    RIL_Errno ret = RIL_E_SUCCESS;
    char path[PROPERTY_VALUE_MAX] = {0};

    memset(&sr, 0, sizeof(RIL_SIM_IO_Response));

    /* For read command */
    line = (char *) calloc(1, 2*256+1);
    if (NULL == line) {
        RLOGE("usimGetResponse, alloc OOM");
        ret = RIL_E_NO_MEMORY;
        goto done;
    }
    p_res->simResponse = line; /* assume data out */

    if (p_args->path != NULL) {
        orig_pathLen = pathlen = strlen(p_args->path);
        //if ((pathlen > 4) && ((strncmp(p_args->path, "3F00", 4) == 0) || (strncmp(p_args->path, "3f00", 4) == 0))) {
        if ((strncmp(p_args->path, "3F00", 4) == 0) || (strncmp(p_args->path, "3f00", 4) == 0)) {
            if (pathlen == 4) {
                no_path = 1;
            } else {
                pathlen = (((pathlen-4) < PROPERTY_VALUE_MAX)? (pathlen-4) : (PROPERTY_VALUE_MAX-1));
                strncpy(path, ((p_args->path)+4), pathlen);
            }
        } else {
            pathlen = ((pathlen < PROPERTY_VALUE_MAX)? pathlen : (PROPERTY_VALUE_MAX-1));
            strncpy(path, p_args->path, pathlen);
        }
    }

    if (p_args->sessionId == 0) {
        // Since modem didn't support CRLA in all version.
        // For compatibility purpose, we use CRSM in case of session Id equal to 0.
        if (p_args->path == NULL || no_path == 1) {
            asprintf(&cmd, "AT+CRSM=%d,%d,%d,%d,%d",
                     p_args->command, p_args->fileid,
                     0 , 0, 0);
        } else {
            asprintf(&cmd, "AT+CRSM=%d,%d,%d,%d,%d,,\"%s\"",
                    p_args->command, p_args->fileid,
                    0 , 0, 0, path);
        }
    } else {
        // For session id is not equal to 0,
        //we should use CRLA to indicated target session id.
        if (p_args->path == NULL || no_path == 1) {
            asprintf(&cmd, "AT+CRLA=%d, %d,%d,%d,%d,%d",
                     p_args->sessionId, p_args->command, p_args->fileid,
                     0 , 0, 0);
        } else {
            asprintf(&cmd, "AT+CRLA=%d,%d,%d,%d,%d,%d,,\"%s\"",
                    p_args->sessionId, p_args->command, p_args->fileid,
                    0 , 0, 0, path);
        }
    }

    memset(&sr,0,sizeof(RIL_SIM_IO_Response));

    if (RIL_E_SUCCESS == sendSimAccessCommand((const char *) cmd, &p_response, &sr, t, p_args->sessionId)) {
        if (sr.simResponse) {

            len = strlen(sr.simResponse);

            if (len > 512) {
                RLOGE("Invalid len:%zu",len);
                ret = RIL_E_GENERIC_FAILURE;
                p_res->simResponse = NULL;
                goto done;
            }

            memcpy(line,sr.simResponse,len);

        } else {
            /* no data e.g. 0x94 0x04 */
            p_res->simResponse = NULL;
            free(line);
            line = NULL;
            goto done;
        }
    } else {
        ret = RIL_E_GENERIC_FAILURE;
        goto done;
    }


done:
    p_res->sw1 = sr.sw1;
    p_res->sw2 = sr.sw2;
    if (ret != RIL_E_SUCCESS) {
        RLOGE("simIo Fail!");
        if (line) free(line);
    }
    if (cmd) free(cmd);
    at_response_free(p_response);
    return ret;
}
static RIL_Errno simIo(RIL_SIM_IO_EX_v6 *p_args, RIL_SIM_IO_Response *p_res,RIL_Token t)
{
    size_t offset = p_args->p1 * (1 << 8) + p_args->p2;
    int remain = p_args->p3;
    int pathlen = 0, no_path = 0, orig_pathLen = 0;
    size_t len;
    RIL_SIM_IO_Response sr;
    char * line = NULL;
    char * cmd = NULL;
    ATResponse * p_response = NULL;
    RIL_Errno ret = RIL_E_SUCCESS;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    char path[PROPERTY_VALUE_MAX] = {0};

    if((p_args->path != NULL) && (0 == strcmp (p_args->path,""))) {
        p_args->path = NULL;
    }

    if (p_args->path != NULL) {
        orig_pathLen = pathlen = strlen(p_args->path);
        if ((pathlen > 4) && ((strncmp(p_args->path, "3F00", 4) == 0) || (strncmp(p_args->path, "3f00", 4) == 0))) {
            if (pathlen == 4) {
                no_path = 1;
            } else {
                pathlen = (((pathlen-4) < PROPERTY_VALUE_MAX)? (pathlen-4) : (PROPERTY_VALUE_MAX-1));
                strncpy(path, ((p_args->path)+4), pathlen);
            }
        } else if ((isUsimDetect[rid] == 1) && (pathlen == 4) && ((strncmp(p_args->path, "3F00", 4) == 0) || (strncmp(p_args->path, "3f00", 4) == 0))) {
            RLOGD("simIo, but it is USIM");
            no_path = 1;
        } else {
            pathlen = ((pathlen < PROPERTY_VALUE_MAX)? pathlen : (PROPERTY_VALUE_MAX-1));
            strncpy(path, p_args->path, pathlen);
        }
    }

    if ((p_args->command == CMD_STATUS) && (remain <= 0)) {
        remain = 256;
    }

    if (p_args->data == NULL) {
        /* For read command */
        line = (char *) calloc(1, 2*remain+1);
        if (NULL == line) {
            RLOGE("simIo error, alloc OOM");
            ret = RIL_E_NO_MEMORY;
            goto done;
        }
        len = ((remain < 256)?remain:256);

        p_res->simResponse = line; /* assume data out */

        // According to USIM spec, if P1>= 128, SFI shall be used.
        // return error due to modem does not support it.
        if ((p_args->command == CMD_READ_BINARY)
                && (isUsimDetect[rid] == 1)
                && (remain >= USIM_TRANSPARENT_EF_MAX_SIZE)) {
            RLOGD("simIo error, the file of transparent EF is error");
            ret = RIL_E_GENERIC_FAILURE;
            goto done;
        }

        int round = 0;
        while (remain > 0) {
            RLOGD("[simio]Round %d: remain %d: len %zu\n", round++, remain, len);
            if (p_args->sessionId == 0) {
                // Since modem didn't support CRLA in all version.
                // For compatibility purpose, we use CRSM in case of session Id equal to 0.
                if (p_args->path == NULL || no_path == 1) {
                    asprintf(&cmd, "AT+CRSM=%d,%d,%zu,%zu,%d",
                          p_args->command, p_args->fileid,
                         (0xFF & (offset >> 8)) , (0xFF & offset),
                         ((remain < 256)?remain:0));
                } else if (p_args->command == CMD_STATUS) {
                    asprintf(&cmd, "AT+CRSM=%d,%d,%zu,%zu,%d",
                          p_args->command, p_args->fileid,
                         (0xFF & (offset >> 8)), (0xFF & offset), 0);
                } else {
                    asprintf(&cmd, "AT+CRSM=%d,%d,%zu,%zu,%d,,\"%s\"",
                         p_args->command, p_args->fileid,
                        (0xFF & (offset >> 8)) , (0xFF & offset),((remain < 256)?remain:0), path);
                }
            } else {
                // For session id is not equal to 0,
                //we should use CRLA to indicated target session id.
                if (p_args->path == NULL || no_path == 1) {
                    asprintf(&cmd, "AT+CRLA=%d,%d,%d,%zu,%zu,%d",
                          p_args->sessionId, p_args->command, p_args->fileid,
                         (0xFF & (offset >> 8)) , (0xFF & offset),((remain < 256)?remain:0));
                } else {
                    asprintf(&cmd, "AT+CRLA=%d,%d,%d,%zu,%zu,%d,,\"%s\"",
                          p_args->sessionId, p_args->command, p_args->fileid,
                          (0xFF & (offset >> 8)) , (0xFF & offset),
                          ((remain < 256)?remain:0), path);
                }
            }

            memset(&sr,0,sizeof(RIL_SIM_IO_Response));

            if (RIL_E_SUCCESS == sendSimAccessCommand((const char *) cmd, &p_response, &sr, t, p_args->sessionId)) {
                if (sr.simResponse) {
                    if (p_args->command == CMD_STATUS) {
                        remain = 0;
                        memcpy(line, sr.simResponse, strlen(sr.simResponse));
                    } else {
                        memcpy(line + 2*(p_args->p3 - remain), sr.simResponse, 2 * len);
                        offset += len;
                        remain -= len;
                        len = ((remain < 256) ? remain : 256);
                    }
                    free(cmd);
                    at_response_free(p_response);
                    p_response = NULL;
                } else {
                    /* no data e.g. 0x94 0x04 */
                    RLOGI("[simio]Null response\n");
                    p_res->simResponse = NULL;
                    free(cmd);
                    free(line);
                    line = NULL;
                    goto done;
                }
            } else {
                ret = RIL_E_GENERIC_FAILURE;
                free(cmd);
                goto done;
            }

        }


    } else {
        /* For write command */
        line = (char *) calloc(1, 512);
        if (NULL == line) {
            RLOGE("simIo error, alloc OOM");
            ret = RIL_E_NO_MEMORY;
            goto done;
        }
        len = (remain > 255)?255:remain;

        while (remain > 0) {
            strncpy(line,p_args->data + 2*(p_args->p3 - remain),2*len);

            if (p_args->sessionId == 0) {
                // Since modem didn't support CRLA in all version.
                // For compatibility purpose, we use CRSM in case of session Id equal to 0.
                if (p_args->path == NULL || no_path == 1) {
                    asprintf(&cmd, "AT+CRSM=%d,%d,%zu,%zu,%zu,\"%s\"",
                        p_args->command, p_args->fileid,
                        (0xFF & (offset >> 8)), (0xFF & offset), len, line);
                } else {
                    asprintf(&cmd, "AT+CRSM=%d,%d,%zu,%zu,%zu,\"%s\",\"%s\"",
                        p_args->command, p_args->fileid,
                        (0xFF & (offset >> 8)), (0xFF & offset), len, line, path);
                }
            } else {
                // For session id is not equal to 0,
                //we should use CRLA to indicated target session id.
                if (p_args->path == NULL || no_path == 1) {
                    asprintf(&cmd, "AT+CRLA=%d,%d,%d,%zu,%zu,%zu,\"%s\"",
                        p_args->sessionId, p_args->command, p_args->fileid,
                        (0xFF & (offset >> 8)), (0xFF & offset), len, line);
                } else {
                    asprintf(&cmd, "AT+CRLA=%d,%d,%d,%zu,%zu,%zu,\"%s\",\"%s\"",
                        p_args->sessionId, p_args->command, p_args->fileid,
                        (0xFF & (offset >> 8)), (0xFF & offset), len, line, path);
                }
            }

            if (RIL_E_SUCCESS == sendSimAccessCommand((const char *) cmd, &p_response, &sr, t, p_args->sessionId)) {
                offset += len;
                remain -= len;
                len = ((remain < 256)?remain:256);
                free(cmd);
                at_response_free(p_response);
                p_response = NULL;
            } else {
                ret = RIL_E_GENERIC_FAILURE;
                free(cmd);
                goto done;
            }
        }
        free(line); /* free line here because no response data needed */
        line = NULL;
        p_res->simResponse = NULL;
    }

done:
    p_res->sw1 = sr.sw1;
    p_res->sw2 = sr.sw2;

    if (p_args->sessionId == 0) {
        if ((p_res->sw1 == 0x00) && (p_res->sw2 == 0x00)) {
            ret = RIL_E_REQUEST_NOT_SUPPORTED;
        }
    }

    if (ret != RIL_E_SUCCESS) {
        RLOGE("simIo Fail! p3:%d, offset:%zu, remain:%d",p_args->p3,offset,remain);
        if (line) {
            free(line);
        }
    }
    at_response_free(p_response);
    return ret;
}

void makeSimRspFromUsimFcp(unsigned char ** simResponse)
{
    int format_wrong = 0;
    unsigned char * fcpByte = NULL;
    unsigned short  fcpLen = 0;
    usim_file_descriptor_struct fDescriptor = {0,0,0,0};
    usim_file_size_struct fSize  = {0};
    unsigned char simRspByte[GET_RESPONSE_EF_SIZE_BYTES] = {0};

    fcpLen = hexStringToByteArray(*simResponse, &fcpByte);

    if (FALSE == usim_fcp_query_tag(fcpByte, fcpLen, FCP_FILE_DES_T, &fDescriptor)) {
        RLOGE("USIM FD Parse fail:%s", *simResponse);
        format_wrong = 1;
        goto done;
    }

    if ((!IS_DF_ADF(fDescriptor.fd)) && (FALSE == usim_fcp_query_tag(fcpByte, fcpLen, FCP_FILE_SIZE_T,&fSize))) {
        RLOGW("USIM File Size fail:%s", *simResponse);
        format_wrong = 1;
        goto done;
    }

    if (IS_DF_ADF(fDescriptor.fd)) {
        simRspByte[RESPONSE_DATA_FILE_TYPE] = TYPE_DF;
        goto done;
    } else {
        simRspByte[RESPONSE_DATA_FILE_TYPE] = TYPE_EF;
    }

    simRspByte[RESPONSE_DATA_FILE_SIZE_1] = (fSize.file_size & 0xFF00) >> 8;
    simRspByte[RESPONSE_DATA_FILE_SIZE_2] = fSize.file_size & 0xFF;

    if (IS_LINEAR_FIXED_EF(fDescriptor.fd)) {
        simRspByte[RESPONSE_DATA_STRUCTURE] = EF_TYPE_LINEAR_FIXED;
        simRspByte[RESPONSE_DATA_RECORD_LENGTH] = fDescriptor.rec_len;
    } else if (IS_TRANSPARENT_EF(fDescriptor.fd)) {
        simRspByte[RESPONSE_DATA_STRUCTURE] = EF_TYPE_TRANSPARENT;

    } else if (IS_CYCLIC_EF(fDescriptor.fd)) {
        simRspByte[RESPONSE_DATA_STRUCTURE] = EF_TYPE_CYCLIC;
        simRspByte[RESPONSE_DATA_RECORD_LENGTH] = fDescriptor.rec_len;
    }


done:
    free(*simResponse);
    free(fcpByte);
    if (format_wrong != 1) {
        *simResponse = byteArrayToHexString(simRspByte, GET_RESPONSE_EF_SIZE_BYTES);
        RLOGD("simRsp done:%s", *simResponse);
    } else {
        *simResponse = NULL;
        RLOGE("simRsp done, but simRsp is null because command format may be wrong");
    }

}

static void handleSimIo(RIL_SIM_IO_EX_v6 *data, RIL_Token t) {
    ATResponse *p_response = NULL;
    RIL_SIM_IO_Response sr;
    int err;
    char *cmd = NULL;
    RIL_SIM_IO_EX_v6 *p_args;
    char *line;
    char *cpinResult;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    SimPinCount retryCounts;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int appTypeId = -1;

    memset(&sr, 0, sizeof(sr));

    p_args = (RIL_SIM_IO_EX_v6 *)data;

    if (p_args == NULL || data == NULL) {
        RLOGD("handleSimIo start, p_args is %d, data is %d", ((p_args != NULL)? 1 : 0), ((data != NULL)? 1 : 0));
        ret = RIL_E_INVALID_ARGUMENTS;
        goto error;
    }

    appTypeId = queryAppTypeByAid(p_args->aidPtr);

    if (!serviceActivationStatus(p_args->fileid, appTypeId, t)) {
        ret = RIL_E_REQUEST_NOT_SUPPORTED;
        goto done;
    }

    /* Handle if there is a PIN2 */
    if (NULL != p_args->pin2) {
        asprintf(&cmd, "AT+EPIN2=\"%s\"", (const char *) p_args->pin2);
        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);

        /* AT+EPIN2 fail and check the cause */
        if (err != 0) {
            /* AT runtime error */
            assert(0);
            goto done;
        }

        if (p_response->success == 0) {
            switch (at_get_cme_error(p_response)) {
                case CME_SIM_PIN2_REQUIRED:
                    ret = RIL_E_SIM_PIN2;
                    goto done;
                    break;
                case CME_SIM_PUK2_REQUIRED:
                    ret = RIL_E_SIM_PUK2;
                    goto done;
                    break;
                default:
                    ret = RIL_E_GENERIC_FAILURE;
                    goto done;
                    break;
            }
        } else {
            ret = RIL_E_SUCCESS;
        }
    }

    if ((isUsimDetect[rid] == 1) && (p_args->command == CMD_GET_RESPONSE)) {

        ret = usimGetResponse(p_args, &sr, t);

        /* Map USIM FCP to SIM RSP */
        if ((RIL_E_SUCCESS == ret) && (sr.simResponse != NULL)) {
            makeSimRspFromUsimFcp((unsigned char **) &(sr.simResponse));
            if (sr.simResponse == NULL) {
                RLOGE("The response format may be wrong");
                ret = RIL_E_GENERIC_FAILURE;
            }
        }

    } else {
       ret = simIo(p_args, &sr, t);
    }

done:
    if (NULL != p_args->pin2) {
        getPINretryCount(&retryCounts, t, rid);
    }

    if (ret == RIL_E_SUCCESS) {
        RIL_onRequestComplete(t, ret, &sr, sizeof(sr));
        if (sr.simResponse) {
            free(sr.simResponse);
            sr.simResponse = NULL;
        }
    } else {
        RIL_onRequestComplete(t, ret, NULL, 0);
    }
    at_response_free(p_response);
    if (cmd) {
        free(cmd);
    }
    return;
error:
    RIL_onRequestComplete(t, ret, NULL, 0);
}

static void requestSimIo(void *data, size_t datalen, RIL_Token t)
{
    RIL_SIM_IO_v6 *p_args;
    RIL_SIM_IO_EX_v6 p_args_ex;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    RIL_SIM_UNUSED_PARM(datalen);

    memset(&p_args_ex, 0, sizeof(RIL_SIM_IO_EX_v6));

    p_args = (RIL_SIM_IO_v6 *)data;

    p_args_ex.command = p_args->command;
    p_args_ex.fileid = p_args->fileid;
    p_args_ex.path = p_args->path;
    p_args_ex.p1 = p_args->p1;
    p_args_ex.p2 = p_args->p2;
    p_args_ex.p3 = p_args->p3;
    p_args_ex.data = p_args->data;
    p_args_ex.pin2 = p_args->pin2;
    p_args_ex.aidPtr = p_args->aidPtr;
    if (p_args_ex.aidPtr != NULL && (queryAppType(p_args_ex.aidPtr) == AID_ISIM)) {
        SessionInfo* pSession = getIsimSessionInfo(rid);
        p_args_ex.sessionId = pSession->session;
    } else {
        p_args_ex.sessionId = 0;
    }

    RLOGD("requestSimIo %d, %d, %s, %d, %d, %d %d!!", p_args_ex.command, p_args_ex.fileid, p_args_ex.path,
            p_args_ex.p1, p_args_ex.p2, p_args_ex.p3, p_args_ex.sessionId);

    handleSimIo(&p_args_ex, t);
}

static void simFacilityLock(void *data, size_t datalen, RIL_Token t)
{
    ATResponse*     p_response = NULL;
    int             fdnServiceResult = -1;
    int             err = -1;
    char*           cmd = NULL;
    const char**    strings = (const char**)data;;
    int             response = -1;
    char*           line = NULL;
    SimPinCodeE     simOperationSetLock = PINUNKNOWN;
    SimPinCount     retryCount;
    int             attemptsRemaining[4] = {0};
    const char * p_serviceClass = NULL;
    RIL_Errno       ret = RIL_E_GENERIC_FAILURE;
    RIL_SOCKET_ID           rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    int           resLength = 0;
    int*          p_res = NULL;
    int allResponse = -1;

    // [ALPS00451149][MTK02772]
    // CLCK is query before MSG_ID_SIM_MMI_READY_IND
    // FD's flag is ready after receive this msg
    // solution: request again if modem response busy, max 2.5s
    int isSimBusy = 0;
    int count = 0;

    do{
         //ALPS00839044: Modem needs more time for some special cards.
         //The detail of the time 2.5s is in the note of this CR.
         if( count == 13 ) {
             RLOGE("Set Facility Lock: CME_SIM_BUSY and time out.");
             goto error;
         }

        if (datalen <= 0 || strings == NULL || strings[0] == NULL) {
            RLOGE("Set Facility Lock: Null parameters.");
            ret = RIL_E_INVALID_ARGUMENTS;
            goto error;
        }
    // For call barring part.
    if ((0 == strcmp (strings[0],"AO")) ||
        (0 == strcmp (strings[0],"OI")) ||
        (0 == strcmp (strings[0],"OX")) ||
        (0 == strcmp (strings[0],"AI")) ||
        (0 == strcmp (strings[0],"IR")) ||
        (0 == strcmp (strings[0],"AB")) ||
        (0 == strcmp (strings[0],"AG")) ||
        (0 == strcmp (strings[0],"AC")))
    {
        requestCallBarring(data, datalen, t);
        return;
    }

    if ( datalen == 4*sizeof(char*) ) {
        /* Query Facility Lock */
        /* No network related query. CLASS is unnecessary */
        if (0 == strcmp (strings[0],"FD")) {
            // Use AT+ESSTQ=<app>,<service> to query service table
            // 0:  Service not supported
            // 1:  Service supported
            // 2:  Service allocated but not activated
            if (isUsimDetect[rid] == 1) {
                asprintf(&cmd, "AT+ESSTQ=%d,%d", 1, 2);
            } else {
                asprintf(&cmd, "AT+ESSTQ=%d,%d", 0, 3);
            }
            err = at_send_command_singleline(cmd, "+ESSTQ:", &p_response, SIM_CHANNEL_CTX);

            free(cmd);
            cmd = NULL;

            // The same as AOSP. 0 - Available & Disabled, 1-Available & Enabled, 2-Unavailable.
            if (err < 0 || NULL == p_response) {
                RLOGE("Fail to query service table");
            } else if (p_response->success == 0) {
                RLOGE("Fail to query service table");
            } else {
                line = p_response->p_intermediates->line;

                err = at_tok_start(&line);
                if (err < 0) goto error;
                err = at_tok_nextint(&line, &fdnServiceResult);
                if (err < 0) goto error;

                if (fdnServiceResult == 0) {
                    fdnServiceResult = 2;
                } else if (fdnServiceResult == 2) {
                    fdnServiceResult = 0;
                }

                RLOGD("FDN available: %d", fdnServiceResult);
            }
            at_response_free(p_response);
            p_response = NULL;
        }
        asprintf(&cmd, "AT+CLCK=\"%s\",2", strings[0]);

        err = at_send_command_singleline(cmd, "+CLCK:", &p_response, SIM_CHANNEL_CTX);
    } else if ( datalen == 5*sizeof(char*) ) {

        if ( 0 == strcmp (strings[0],"SC") ) {
            simOperationSetLock = PINCODE1;
        } else if ( 0 == strcmp (strings[0],"FD") ) {
            simOperationSetLock = PINCODE2;
        }

        if(NULL == strings[2]) {

            RLOGE("Set Facility Lock: Pwd cannot be null!");
            ret = RIL_E_PASSWORD_INCORRECT;
            goto error;
        }

        /* Set Facility Lock */
        /* Local operation. PASSWD is necessary */
        asprintf(&cmd, "AT+CLCK=\"%s\",%s,\"%s\"", strings[0], strings[1], strings[2]);

        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    } else
        goto error;

    free(cmd);
        cmd = NULL;

        if (err < 0 || NULL == p_response) {
            RLOGE("getFacilityLock Fail");
            goto error;
        }
        switch (at_get_cme_error(p_response)) {
            case CME_SIM_BUSY:
                RLOGD("simFacilityLock: CME_SIM_BUSY");
                sleepMsec(200);
                count++;
                isSimBusy = 1;
                at_response_free(p_response);
                p_response = NULL;
                break;
            default:
                isSimBusy = 0;
                break;
        }
    } while (isSimBusy == 1);

    if (PINUNKNOWN != simOperationSetLock) {
        getPINretryCount(&retryCount, t, rid);
    }

    if (p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            case CME_SIM_PIN_REQUIRED:
            case CME_SIM_PUK_REQUIRED:
                ret = RIL_E_PASSWORD_INCORRECT;
                break;
            case CME_SIM_PIN2_REQUIRED:
                ret = RIL_E_SIM_PIN2;
                break;
            case CME_SIM_PUK2_REQUIRED:
                ret = RIL_E_SIM_PUK2;
                break;
            case CME_INCORRECT_PASSWORD:
                ret = RIL_E_PASSWORD_INCORRECT;
                if (PINUNKNOWN == simOperationSetLock) {
                    /* Non PIN operation! Goto Error directly */
                    goto error;
                }
                break;
            case CME_CALL_BARRED:
            case CME_OPR_DTR_BARRING:
                ret = RIL_E_GENERIC_FAILURE;
                goto error;
                break;
            case CME_PHB_FDN_BLOCKED:
                ret = RIL_E_FDN_CHECK_FAILURE;
                goto error;
                break;
            default:
                RLOGD("simFacilityLock() retryPIN2 = %d, simOperationSetLock = %d", retryCount.pin2, simOperationSetLock);
                if (simOperationSetLock == PINCODE2 && retryCount.pin2 != 3) {
                    ret = ((retryCount.pin2 == 0) ? RIL_E_SIM_PUK2 : RIL_E_PASSWORD_INCORRECT);
                }
                goto error;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }



    /* For Query command only */
    if ( p_response->p_intermediates != NULL ) {
        ATLine * p_cur;
        int serviceClass;

        if (bNewSsServiceClassFeature == 0) {
            for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
                resLength++;
            }
            LOGI("%d of +CLCK: received!",resLength);

            p_res = alloca(resLength * sizeof(int));
            if (NULL == p_res) {
                RLOGE("CLCK: OOM!");
                ret = RIL_E_NO_MEMORY;
                goto error;
            }
            resLength = 0; /* reset resLength for decoding */
        }

        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            char *line = p_cur->line;
            assert(line);

            err = at_tok_start(&line);

            if (err < 0) {
                    RLOGE("CLCK: decode error 1!");
                goto error;
            }

            err = at_tok_nextint(&line, &response); /* 0 disable 1 enable */

            if (bNewSsServiceClassFeature == 0) {
                p_res[resLength] = 0; /* Set Init value to 0 */
            }

            if (at_tok_hasmore(&line)) {
                if (bNewSsServiceClassFeature == 0) {
                    err = at_tok_nextint(&line, &p_res[resLength]); /* enabled service code */
                    LOGD("Status:%d, BSCode:%d\n",response,p_res[resLength]);
                }
            }

            if (err < 0) {
                RLOGE("CLCK: decode error 2!");
                goto error;
            }

            if (bNewSsServiceClassFeature == 0) {
                resLength++;
            }
        }
    }


    if (err < 0) {
         goto error;
    } else {
        if (PINUNKNOWN != simOperationSetLock) {
            /* SIM operation we shall return pin retry counts */
            attemptsRemaining[0] = retryCount.pin1;
            attemptsRemaining[1] = retryCount.pin2;
            attemptsRemaining[2] = retryCount.puk1;
            attemptsRemaining[3] = retryCount.puk2;
            if (simOperationSetLock == PINCODE2) {
                attemptsRemaining[0] = retryCount.pin2;
            }
            // HIDL: the response length should be sizeof(int).
            RIL_onRequestComplete(t, ret,
                attemptsRemaining, sizeof(int));

        } else {
            RLOGD("resLength = %d",resLength);

            /* For those non NW (Call Barring) results*/
            if (fdnServiceResult == -1) {
                RIL_onRequestComplete(t, RIL_E_SUCCESS,
                        &response, sizeof(int));
            } else {
                if (fdnServiceResult == 1 && response == 0) {
                    fdnServiceResult = 0;
                }
                RLOGD("final FDN result: %d", fdnServiceResult);
                RIL_onRequestComplete(t, RIL_E_SUCCESS,
                        &fdnServiceResult, sizeof(int));
            }
        }

        at_response_free(p_response);
        p_response = NULL;
        return;
    }

error:
    if (PINUNKNOWN != simOperationSetLock) {
        /* SIM operation we shall return pin retry counts */
        attemptsRemaining[0] = retryCount.pin1;
        attemptsRemaining[1] = retryCount.pin2;
        attemptsRemaining[2] = retryCount.puk1;
        attemptsRemaining[3] = retryCount.puk2;
        if (simOperationSetLock == PINCODE2) {
            RLOGE("[simFacilityLock] fail to set FDN");
            attemptsRemaining[0] = retryCount.pin2;
        }
        // HIDL: the response length should be sizeof(int).
        RIL_onRequestComplete(t, ret,
            attemptsRemaining, sizeof(int));

    } else {
        RIL_onRequestComplete(t, ret, NULL, 0);
    }
    at_response_free(p_response);
}

// MTK-START: ISIM
static void requestIsimAuthentication(void *data, size_t datalen, RIL_Token t) {
    char *nonce;

    ATResponse *p_response = NULL;
    int err = 0;
    char* cmd = NULL;
    char *line;
    RIL_SIM_IO_Response sr;
    int sessionId;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    RIL_SIM_UNUSED_PARM(datalen);

    nonce = (char *) data;

    memset(&sr, 0, sizeof(sr));

    if (!isSimInserted(rid)) {
        ret = RIL_E_RADIO_NOT_AVAILABLE;
    }
    // Get ISIM session id
    sessionId = queryIccApplicationChannel(0, t);
    if (sessionId <= 0) {
        LOGE("requestIsimAuthentication : ISIM application has not been initialized.");
        goto error;
    }

    asprintf(&cmd, "AT+ESIMAUTH=%d,%d,\"%s\"", sessionId, 0, nonce);

    err = at_send_command_singleline(cmd, "+ESIMAUTH:", &p_response, SIM_CHANNEL_CTX);

    if (err < 0 || NULL == p_response) {
        LOGE("requestIsimAuthentication Fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        RLOGD("p_response = %d /n", at_get_cme_error(p_response) );
        case CME_SUCCESS:
            break;
        case CME_UNKNOWN:
        RLOGE("p_response: CME_UNKNOWN");
            break;
        default:
            goto error;
    }

    if (err < 0 || p_response->success == 0) {
        goto error;
    } else {

        line = p_response->p_intermediates->line;
        err = at_tok_start(&line);
        if(err < 0) goto error;

        err = at_tok_nextint(&line, &sr.sw1);
        if(err < 0) goto error;

        err = at_tok_nextint(&line, &sr.sw2);
        if(err < 0) goto error;

        if(at_tok_hasmore(&line)){
            err = at_tok_nextstr(&line, &(sr.simResponse));
            if(err < 0) goto error;
            RLOGD("requestIsimAuthentication len = %zu %02x, %02x", strlen(sr.simResponse), sr.sw1, sr.sw2);
        } else {
            RLOGD("No response data");
        }
        #ifdef MTK_TC1_FEATURE
          char auth_rsp[MAX_AUTH_RSP] = {0,};
          strcpy(auth_rsp, sr.simResponse);
          RLOGD("requestIsimAuthentication - Response = %s", auth_rsp);
          RIL_onRequestComplete(t, RIL_E_SUCCESS, &auth_rsp, sizeof(auth_rsp));
        #else
          RIL_onRequestComplete(t, RIL_E_SUCCESS, &sr.simResponse, sizeof(char*));
        #endif
        at_response_free(p_response);
    }
    return;

error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);

}

// MTK-START: SIM AUTH
static void requestSimAuthentication(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    RIL_SIM_IO_Response sim_auth_response;
    RIL_SimAuthentication *sim_auth_data = (RIL_SimAuthentication *)data;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    SessionInfo* pSession = getIsimSessionInfo(rid);
    int err;
    int channel, closeChannelLater = 0, len = 0;
    int appType = -1;
    char *cmd = NULL;
    char *line;
    unsigned char *out_put = NULL;
    char *payload = NULL, *pByteArray = NULL;
    char tmp[256] = {0};

    RIL_SIM_UNUSED_PARM(datalen);

    RLOGD("[requestSimAuthentication] enter, aid %s, authContext %d", sim_auth_data->aid,
            sim_auth_data->authContext);

    // Step1. Open channel with indicated AID to obtain channel id
    appType = queryAppType(sim_auth_data->aid);
    if (appType == AID_ISIM) {
        RLOGD("[requestSimAuthentication] Go for ISIM!");
        if (pSession->session <= 0) {
            err = turnOnIsimApplication(t, rid);
            if (err != RIL_E_SUCCESS) {
                goto error;
            }
        }
        channel = pSession->session;
    } else if (appType == AID_USIM || sim_auth_data->aid == NULL) {
        RLOGD("[requestSimAuthentication] Go for USIM or SIM!");
        channel = 0;
    } else if (sim_auth_data->aid != NULL) {
        RLOGD("[requestSimAuthentication] Go for the other AID!");
        asprintf(&cmd, "AT+CCHO=\"%s\"", (char *)sim_auth_data->aid);
        err = at_send_command_numeric(cmd, &p_response, SIM_CHANNEL_CTX);
        free(cmd);

        if (err < 0 || NULL == p_response || p_response->success == 0) {
            RLOGE("[requestSimAuthentication] open channel fail");
            goto error;
        }

        switch (at_get_cme_error(p_response)) {
            RLOGD("p_response = %d /n", at_get_cme_error(p_response) );
            case CME_SUCCESS:
                break;
            case CME_MEMORY_FULL:
                err = RIL_E_MISSING_RESOURCE;
                break;
            case CME_NOT_FOUND:
                err = RIL_E_NO_SUCH_ELEMENT;
                RLOGE("p_response: CME_UNKNOWN");
                break;
            default:
                err = RIL_E_GENERIC_FAILURE;
                goto error;
        }

        channel = atoi(p_response->p_intermediates->line);

        if(channel <= 0 || channel > 19) {
            RLOGE("[requestSimAuthentication] fail to open channel = %d", channel);
            err = RIL_E_GENERIC_FAILURE;
            goto error;
        }
        at_response_free(p_response);
        p_response = NULL;
        closeChannelLater = 1;
    } else {
        RLOGE("[requestSimAuthentication] No AID!");
        err = RIL_E_GENERIC_FAILURE;
        goto error;
    }

    if (NULL == sim_auth_data->authData) {
        err = RIL_E_INVALID_ARGUMENTS;
        RLOGE("[requestSimAuthentication] Send authentication APDU Fail");
        goto error;
    }

    // decode the data
    out_put = base64_decode_to_str((unsigned char *)sim_auth_data->authData,
            strlen(sim_auth_data->authData));
    RLOGD("[requestSimAuthentication] decode output %s, %d, %d", out_put,
            sim_auth_data->authContext, isUsimDetect[rid]);
    // Step2. Send authentication APDU
    // AT+CGLA="channel id","len","CLA+INS+P1+P2+Lc+Data+Le"
    // CLA: if channel id 0~3, then CLA=0X; otherwise, if channel id 4~19, then CLA=4X
    // INS: 88 for authentication command
    // P1:00
    // P2 is decided by user's parameter, will be 8X according authentication context type
    // Lc:length of authentication context
    // Data: authentication context decided by user's parameter
    // Le: max length of data expected in response. use 00 represent unknown.
    if (isUsimDetect[rid] == 1) {
        asprintf(&cmd, "AT+CGLA=%d,%zu,\"%02x%02x%02x%02x%02zx%s00\"",
                channel, (12 + strlen((char*)out_put)),
                ((channel <= 3) ? channel : (4 * 16 + channel - 4)), 0x88, 0, sim_auth_data->authContext,
                (strlen((char*)out_put)/2), out_put);
    } else {
        asprintf(&cmd, "AT+CGLA=%d,%zu,\"%02x%02x%02x%02x%s0C\"",
                channel, (10 + strlen((char*)out_put)),
                0xA0, 0x88, 0x00, 0x00, out_put);
    }
    err = at_send_command_singleline(cmd, "+CGLA:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    free(out_put);
    out_put = NULL;

    if (err < 0 || (NULL != p_response && p_response->success == 0)) {
        err = RIL_E_GENERIC_FAILURE;
        RLOGE("[requestSimAuthentication] Send authentication APDU Fail");
        goto error;
    }

    // Process authentication response data
    if (err < 0 || NULL == p_response || p_response->success == 0) {
        RLOGE("[requestSimAuthentication] authentication fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        RLOGD("p_response = %d /n", at_get_cme_error(p_response) );
        case CME_SUCCESS:
            break;
        default:
            err = RIL_E_GENERIC_FAILURE;
            goto error;
    }

    memset(&sim_auth_response, 0, sizeof(RIL_SIM_IO_Response));

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) goto error;
    err = at_tok_nextint(&line, &len);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line, &payload);
    if (err < 0) goto error;
    //err = at_tok_nextstr(&line, &sim_auth_response.simResponse);
    //if (err < 0) goto error;

    /*
    sscanf(&(sim_auth_response.simResponse[len - 4]), "%02x%02x", &(sim_auth_response.sw1),
        &(sim_auth_response.sw2));
    sim_auth_response.simResponse[len - 4] = '\0';
    */

    sscanf(&(payload[len - 4]), "%02x%02x", &(sim_auth_response.sw1),
        &(sim_auth_response.sw2));
    payload[len - 4] = '\0';

    if (sim_auth_response.sw1 != 0x90 && sim_auth_response.sw1 != 0x91) {
        RLOGD("Fail to do SIM authentication");
        err = RIL_E_INVALID_ARGUMENTS;
        goto error;
    }

    if (len > 4) {
        strncpy(tmp, payload, 255);

        RLOGD("[requestSimAuthentication] tmp = %s, %zu",
                tmp, strlen(tmp));
        out_put = base64_encode_to_str((unsigned char*)tmp, strlen(tmp));
    }

    sim_auth_response.simResponse = (char*)out_put;

    RLOGD("[requestSimAuthentication] payload = %s %02x, %02x",
            sim_auth_response.simResponse, sim_auth_response.sw1, sim_auth_response.sw2);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &sim_auth_response, sizeof(sim_auth_response));

    at_response_free(p_response);
    p_response = NULL;

    if (out_put != NULL) {
        free(out_put);
    }

    RLOGD("[requestSimAuthentication] end");
    if (closeChannelLater) {
        // Step3. Close channel to avoid open too much unuseless channels
        asprintf(&cmd, "AT+CCHC=%d", channel);
        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
        free(cmd);

        if (err < 0 || NULL == p_response || p_response->success == 0) {
            RLOGE("[requestSimAuthentication] close channel fail");
            goto error;
        }

        switch (at_get_cme_error(p_response)) {
            RLOGD("p_response = %d /n", at_get_cme_error(p_response) );
            case CME_SUCCESS:
                break;
            default:
                err = RIL_E_GENERIC_FAILURE;
                goto error;
        }
        at_response_free(p_response);
        p_response = NULL;
    }
    return;
error:
    if (closeChannelLater) {
        // Step3. Close channel to avoid open too much unuseless channels
        at_response_free(p_response);
        p_response = NULL;

        asprintf(&cmd, "AT+CCHC=%d", channel);
        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
        free(cmd);

        if (err < 0 || NULL == p_response || p_response->success == 0) {
            RLOGE("[requestSimAuthentication] close channel fail");
        }
    }
    RIL_onRequestComplete(t, err, NULL, 0);
    at_response_free(p_response);

}
// MTK-END

static void requestGetAllowedCarriers(void *data, size_t datalen, RIL_Token t) {
    ATResponse*     p_response = NULL;
    ATLine          *p_cur;
    int             err = -1;
    RIL_CarrierRestrictionsWithPriority *result = NULL;
    char            *line;
    int             lockstate = 0;  // 1: locked state, 2: unlocked state, 3: disable state
    int             listtype = -1;  // 0: white list, 1: black list
    int             category = -1;
    int             tmp_len = 0;
    int             len[RIL_MATCH_MAX] = {RIL_MATCH_MAX};
    char            *tmp;
    int             i, j;
    int             k = 0;
    int             total_allowed_len = 0;
    int             total_excluded_len = 0;
    RIL_Carrier *allowed = NULL;
    RIL_Carrier **allowed_tmp = (RIL_Carrier **)calloc(RIL_MATCH_MAX, sizeof(RIL_Carrier*));
    RIL_Carrier *excluded = NULL;
    RIL_Carrier **excluded_tmp = (RIL_Carrier **)calloc(RIL_MATCH_MAX, sizeof(RIL_Carrier*));

    RIL_SIM_UNUSED_PARM(datalen);
    RIL_SIM_UNUSED_PARM(data);

    LOGI("requestGetAllowedCarriers IN!");
    err = at_send_command_multiline("AT+ECRRST?", "+ECRRST:", &p_response, SIM_CHANNEL_CTX);
    if (err < 0 || p_response->success == 0) {
            goto error;
    }

    result = (RIL_CarrierRestrictionsWithPriority *)calloc(1, sizeof(
            RIL_CarrierRestrictionsWithPriority));
    assert(result != NULL);

    p_cur = p_response->p_intermediates;
    line = p_cur->line;
    err = at_tok_start(&line);
    if (err < 0) {
        goto error;
    }
    LOGI("First Line Start");

    err = at_tok_nextint(&line, &lockstate);
    if (err < 0) {
        goto error;
    }
    LOGI("lockstate: %d", lockstate);

    err = at_tok_nextint(&line, &(result->simLockMultiSimPolicy));
    if (err < 0) {
        goto error;
    }
    LOGI("simLockMultiSimPolicy: %d", result->simLockMultiSimPolicy);

    err = at_tok_nextint(&line, &(result->allowedCarriersPrioritized));
    if (err < 0) {
        goto error;
    }
    LOGI("allowedCarriersPrioritized: %d", result->allowedCarriersPrioritized);

    // White list
    p_cur = p_cur->p_next;
    line = p_cur->line;
    err = at_tok_start(&line);
    if (err < 0) {
        goto error;
    }
    LOGI("Second Line Start");

    err = at_tok_nextint(&line, &listtype);
    if (err < 0) {
        goto error;
    }
    if (listtype != 0 && listtype != 1) {
        goto error;
    }
    LOGI("listtype: %d", listtype);

    // result->len_allowed_carriers = total_allowed_len;
    // allowed = calloc(total_allowed_len, sizeof(RIL_Carrier));
    for (i = 0; i < RIL_MATCH_MAX; i++) {
        err = at_tok_nextint(&line, &category);
        if (err < 0) {
            goto error;
        }
        LOGI("category: %d", category);

        err = at_tok_nextint(&line, &tmp_len);
        if (err < 0) {
            goto error;
        }
        LOGI("tmp_len: %d", tmp_len);

        len[i] = tmp_len;
        allowed_tmp[i] = (RIL_Carrier *)calloc(tmp_len, sizeof(RIL_Carrier));
        for (j = 0; j < tmp_len; j++) {
            RIL_Carrier *p_cr = allowed_tmp[i] + j;

            err = at_tok_nextstr(&line, &tmp);
            if (err < 0) {
                goto error;
            }
            LOGI("mccmnc: %s", tmp);

            char* tmp_mcc = (char *)alloca(4);
            memset(tmp_mcc, '\0', 4);
            strncpy(tmp_mcc, tmp, 3);
            // strcat(tmp_mcc, "\0");
            LOGI("tmp_mcc: %s, %zu", tmp_mcc, strlen(tmp_mcc));
            asprintf(&p_cr->mcc, "%s", tmp_mcc);
            LOGI("mcc: %s", p_cr->mcc);

            char* tmp_mnc = (char *)alloca(4);
            memset(tmp_mnc, '\0', 4);
            strncpy(tmp_mnc, tmp + 3, strlen(tmp) - 3);
            LOGI("tmp_mnc: %s, %zu", tmp_mcc, strlen(tmp_mcc));
            asprintf(&p_cr->mnc, "%s", tmp_mnc);
            LOGI("mnc: %s", p_cr->mnc);
            p_cr->match_type = (RIL_CarrierMatchType) category;
            if (category != 0) {
                err = at_tok_nextstr(&line, &tmp);
                if (err < 0) {
                    goto error;
                }
                LOGI("match_data: %s", tmp);
                asprintf(&p_cr->match_data, "%s", tmp);
            }
        }
        total_allowed_len += tmp_len;
    }

    result->len_allowed_carriers = total_allowed_len;
    result->allowed_carriers = allowed = (RIL_Carrier *)calloc(total_allowed_len,
            sizeof(RIL_Carrier));
    for (i = 0; i < RIL_MATCH_MAX; i++) {
        for (j = 0; j < len[i]; j++) {
            LOGI("i:%d, j:%d", i, j);
            RIL_Carrier *p_cr = allowed_tmp[i] + j;
            LOGI("*p_cur OK");
            RIL_Carrier *p_cr2 = allowed + (k++);
            LOGI("*p_cur2 OK");
            asprintf(&p_cr2->mcc, "%s", p_cr->mcc);
            LOGI("copy mcc OK");
            asprintf(&p_cr2->mnc, "%s", p_cr->mnc);
            LOGI("copy mnc OK");
            p_cr2->match_type = p_cr->match_type;
            asprintf(&p_cr2->match_data, "%s", p_cr->match_data);
        }
    }

    if (allowed_tmp != NULL) {
        for (i = 0; i < RIL_MATCH_MAX; i++) {
           if (allowed_tmp[i] != NULL) {
               free(allowed_tmp[i]);
               allowed_tmp[i] = NULL;
           }
        }
        free(allowed_tmp);
        allowed_tmp = NULL;
    }
    LOGI("Free allowed_tmp OK");

    // Black list.
    p_cur = p_cur->p_next;
    line = p_cur->line;
    err = at_tok_start(&line);
    if (err < 0) {
        goto error;
    }
    LOGI("Third Line Start");

    err = at_tok_nextint(&line, &listtype);
    if (err < 0) {
        goto error;
    }
    if (listtype != 0 && listtype != 1) {
        goto error;
    }
    LOGI("listtype: %d", listtype);

    // result->len_allowed_carriers = total_allowed_len;
    // allowed = calloc(total_allowed_len, sizeof(RIL_Carrier));
    for (i = 0; i < RIL_MATCH_MAX; i++) {
        err = at_tok_nextint(&line, &category);
        if (err < 0) {
            goto error;
        }
        LOGI("category: %d", category);

        err = at_tok_nextint(&line, &tmp_len);
        if (err < 0) {
            goto error;
        }
        LOGI("tmp_len: %d", tmp_len);

        len[i] = tmp_len;
        excluded_tmp[i] = (RIL_Carrier *)calloc(tmp_len, sizeof(RIL_Carrier));
        for (j = 0; j < tmp_len; j++) {
            RIL_Carrier *p_cr = excluded_tmp[i] + j;

            err = at_tok_nextstr(&line, &tmp);
            if (err < 0) {
                goto error;
            }
            LOGI("mccmnc: %s", tmp);

            char* tmp_mcc = (char *)alloca(4);
            memset(tmp_mcc, '\0', 4);
            strncpy(tmp_mcc, tmp, 3);
            // strcat(tmp_mcc, "\0");
            LOGI("tmp_mcc: %s, %zu", tmp_mcc, strlen(tmp_mcc));
            asprintf(&p_cr->mcc, "%s", tmp_mcc);
            LOGI("mcc: %s", p_cr->mcc);

            char* tmp_mnc = (char *)alloca(4);
            memset(tmp_mnc, '\0', 4);
            strncpy(tmp_mnc, tmp + 3, strlen(tmp) - 3);
            // strcat(tmp_mnc, "\0");
            LOGI("tmp_mnc: %s, %zu", tmp_mcc, strlen(tmp_mcc));
            asprintf(&p_cr->mnc, "%s", tmp_mnc);
            LOGI("mnc: %s", p_cr->mnc);
            p_cr->match_type = (RIL_CarrierMatchType) category;
            if (category != 0) {
                err = at_tok_nextstr(&line, &tmp);
                if (err < 0) {
                    goto error;
                }
                LOGI("match_data: %s", tmp);
                asprintf(&p_cr->match_data, "%s", tmp);
            }
        }
        total_excluded_len += tmp_len;
    }

    result->len_excluded_carriers = total_excluded_len;
    result->excluded_carriers = excluded = (RIL_Carrier *) calloc(total_excluded_len,
            sizeof(RIL_Carrier));
    for (i = 0; i < RIL_MATCH_MAX; i++) {
        for (j = 0; j < len[i]; j++) {
            LOGI("i:%d, j:%d", i, j);
            RIL_Carrier *p_cr = excluded_tmp[i] + j;
            LOGI("*p_cur OK");
            RIL_Carrier *p_cr2 = excluded + (k++);
            LOGI("*p_cur2 OK");
            asprintf(&p_cr2->mcc, "%s", p_cr->mcc);
            LOGI("copy mcc OK");
            asprintf(&p_cr2->mnc, "%s", p_cr->mnc);
            LOGI("copy mnc OK");
            p_cr2->match_type = p_cr->match_type;
            asprintf(&p_cr2->match_data, "%s", p_cr->match_data);
        }
    }

    if (excluded_tmp != NULL) {
        for (i = 0; i < RIL_MATCH_MAX; i++) {
           if (excluded_tmp[i] != NULL) {
               free(excluded_tmp[i]);
               excluded_tmp[i] = NULL;
           }
        }
        free(excluded_tmp);
        excluded_tmp = NULL;
    }
    LOGI("Free excluded_tmp OK");
    LOGI("Black list OK");

    LOGI("requestGetAllowedCarriers: allowed_len %d, excluded len %d",
         result->len_allowed_carriers, result->len_excluded_carriers);
    for (int i = 0; i < result->len_allowed_carriers; i++) {
        allowed = result->allowed_carriers + i;
        LOGI("allowed mcc %s, mnc %s, match_type %d, match_data %s",
                allowed->mcc, allowed->mnc, allowed->match_type, allowed->match_data);
    }
    for (int i = 0; i < result->len_excluded_carriers; i++) {
        excluded = result->excluded_carriers + i;
        LOGI("excluded mcc %s, mnc %s, match_type %d, match_data %s",
                excluded->mcc, excluded->mnc, excluded->match_type, excluded->match_data);
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, result, sizeof(RIL_CarrierRestrictionsWithPriority));
    free(result);
    result = NULL;
    return;

error:
    if (allowed_tmp != NULL) {
        for (i = 0; i < RIL_MATCH_MAX; i++) {
           if (allowed_tmp[i] != NULL) {
               free(allowed_tmp[i]);
               allowed_tmp[i] = NULL;
           }
        }
        free(allowed_tmp);
        allowed_tmp = NULL;
    }
    if (excluded_tmp != NULL) {
        for (i = 0; i < RIL_MATCH_MAX; i++) {
           if (excluded_tmp[i] != NULL) {
               free(excluded_tmp[i]);
               excluded_tmp[i] = NULL;
           }
        }
        free(excluded_tmp);
        excluded_tmp = NULL;
    }
    if (result != NULL) {
        free(result);
        result = NULL;
    }

    LOGI("requestGetAllowedCarriers Fail!");
    RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
    at_response_free(p_response);
}

static void requestSetAllowedCarriers(void *data, size_t datalen, RIL_Token t) {
    // sp<RfxAtResponse> p_response;
    // sp<RfxMclMessage> response;
    ATResponse*     p_response = NULL;
    char*           line;
    char*           cmd = NULL;
    int err = -1;
    // String8 cmd("");
    RIL_CarrierRestrictionsWithPriority *crp = (RIL_CarrierRestrictionsWithPriority*)data;
    RIL_Carrier *allowed = NULL;
    RIL_Carrier *excluded = NULL;
    int allowedCount = 0;
    int excludedCount = 0;
    int totalCount = 0;
    // UICC_Status sim_status = UICC_ABSENT;
    SIM_Status sim_status = SIM_ABSENT;
    RIL_SOCKET_ID   rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    // 1.wait sim status
    sim_status = getSIMStatusWithRetry(rid);

    // 2.unlock
    /* Comment as modem issue.
    err = at_send_command("AT+ECRRST=0,\"12345678\"", &p_response, SIM_CHANNEL_CTX);
    if (err < 0 || p_response->success == 0) {
        logE(mTag, "requestSetAllowedCarriers AT+ECRRST=0 Fail, e= %d", err);
        err = RIL_E_GENERIC_FAILURE;
        goto error;
    }
    at_response_free(p_response);
    p_response = NULL; */

    // 3.Clear all
    err = at_send_command("AT+ECRRST=6", &p_response, SIM_CHANNEL_CTX);
    if (err < 0 || p_response->success == 0) {
        LOGE("requestSetAllowedCarriers AT+ECRRST=6 Fail, e= %d", err);
        goto error;
    }
    at_response_free(p_response);
    p_response = NULL;

    // 4.set multi SIM policy and prioritized.
    LOGI("requestSetAllowedCarriers MultiSimPolicy %d,  Prioritized %d",
            crp->simLockMultiSimPolicy, crp->allowedCarriersPrioritized);
    asprintf(&cmd, "AT+ECRRST=9,\"12345678\",%d,%d",
            crp->simLockMultiSimPolicy, crp->allowedCarriersPrioritized);
    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    cmd = NULL;
    if (err < 0 || p_response->success == 0) {
        LOGE("requestSetAllowedCarriers AT+ECRRST=9 Fail, e= %d", err);
        goto error;
    }
    at_response_free(p_response);
    p_response = NULL;

    // 5.set allowed list
    for (int i = 0; i < crp->len_allowed_carriers; i++) {
        allowed = crp->allowed_carriers + i;

        if (strcmp(allowed->mcc, "") == 0 && strcmp(allowed->mnc, "") == 0) {
            asprintf(&cmd, "AT+ECRRST=7");
        } else if (allowed->match_type == RIL_MATCH_ALL) {
            LOGI("requestSetAllowedCarriers match_type: %d, mcc mnc %s %s",
                    allowed->match_type, allowed->mcc, allowed->mnc);
            asprintf(&cmd, "AT+ECRRST=2,,0,%d,\"%s%s\"",
                    allowed->match_type, allowed->mcc, allowed->mnc);
        } else {
            LOGI("requestSetAllowedCarriers match_type: %d, mcc mnc %s %s, match_data: %s",
                    allowed->match_type, allowed->mcc, allowed->mnc, allowed->match_data);
            asprintf(&cmd, "AT+ECRRST=2,,0,%d,\"%s%s\",\"%s\"",
                    allowed->match_type, allowed->mcc, allowed->mnc, allowed->match_data);
        }

        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
        free(cmd);
        cmd = NULL;
        if (err < 0 || p_response->success == 0) {
            LOGE("requestSetAllowedCarriers set white list %d Fail, e= %d", i, err);
        } else {
            allowedCount++;
        }
        at_response_free(p_response);
        p_response = NULL;
    }

    // 6.set excluded list
    for (int i = 0; i < crp->len_excluded_carriers; i++) {
        excluded = crp->excluded_carriers + i;

        if (excluded->match_type == RIL_MATCH_ALL) {
            asprintf(&cmd, "AT+ECRRST=2,,1,%d,\"%s%s\"",
                    excluded->match_type, excluded->mcc, excluded->mnc);
        } else {
            asprintf(&cmd, "AT+ECRRST=2,,1,%d,\"%s%s\",\"%s\"",
                    excluded->match_type, excluded->mcc, excluded->mnc, excluded->match_data);
        }

        err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
        free(cmd);
        cmd = NULL;
        if (err < 0 || p_response->success == 0) {
            LOGE("requestSetAllowedCarriers set black list %d Fail, e= %d", i, err);
        } else {
            excludedCount++;
        }
        at_response_free(p_response);
        p_response = NULL;
    }

    /* 7.lock
    err = at_send_command("AT+ECRRST=1", &p_response, SIM_CHANNEL_CTX);
    if (err < 0 || p_response->success == 0) {
        logE(mTag, "requestSetAllowedCarriers AT+ECRRST=1 Fail, e= %d", err);
        err = RIL_E_GENERIC_FAILURE;
        goto error;
    }
    at_response_free(p_response);
    p_response = NULL; */

    // 8.wait reboot
    sim_status = getSIMStatusWithRetry(rid);

    // 9.Rset sim
    if ((sim_status == CARD_REBOOT) || (sim_status == SIM_RESTRICTED)
            || (sim_status == USIM_RESTRICTED)) {
        err = at_send_command_singleline("AT+EBTSAP=0", "+EBTSAP:", &p_response, SIM_CHANNEL_CTX);
        if (err < 0 || NULL == p_response || at_get_cme_error(p_response) != CME_SUCCESS) {
            LOGE("requestSetAllowedCarriers AT+EBTSAP=0 Fail, e= %d", err);
            goto error;
        }
        at_response_free(p_response);
        p_response = NULL;

        err = at_send_command("AT+EBTSAP=1", &p_response, SIM_CHANNEL_CTX);
        if (err < 0 || NULL == p_response || at_get_cme_error(p_response) != CME_SUCCESS) {
            LOGE("requestSetAllowedCarriers AT+EBTSAP=1 Fail, e= %d", err);
            goto error;
        }
        at_response_free(p_response);
        p_response = NULL;
    }

    // check if all sucess
    if (allowedCount < crp->len_allowed_carriers || excludedCount < crp->len_excluded_carriers) {
        goto error;
    }

    // AOSP justs request allowedCount when hidl version < 1.4.
    // totalCount = allowedCount + excludedCount;
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &allowedCount, sizeof(int));
    return;

error:

    LOGE("requestSetAllowedCarriers Fail");
    RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
    at_response_free(p_response);
}

// MTK-START: SIM TMO RSU
void onSimMeLockEvent(const char *s, RIL_SOCKET_ID rid) {
    int eventId = 0;
    if(strStartsWith(s, "+ETMOEVT: 0")) { // +ETMOEVT
        LOGD("onSimMeLockEvent: notify need to reboot phone");
        eventId = 0;
    }

    LOGD("onSimMeLockEvent: eventId = %d", eventId);
    RIL_UNSOL_RESPONSE(RIL_UNSOL_MELOCK_NOTIFICATION, &eventId, sizeof(int), rid);
}

void requestSimMeLockGetSharedKey(void *data, size_t datalen, RIL_Token t) {
    LOGD("[SIM ME Lock] requestSimMeLockGetSharedKey");

    ATResponse *p_response = NULL;
    int err, key_len;
    char *cmd, *line, *key = NULL;
    RIL_Errno ret = RIL_E_SUCCESS;

    LOGD("[SIM ME Lock] requestSimMeLockGetSharedKey");
    RIL_SIM_UNUSED_PARM(data);
    RIL_SIM_UNUSED_PARM(datalen);

    asprintf(&cmd, "AT+ETMOSK=0");
    err = at_send_command_singleline(cmd, "+ETMOSK:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || NULL == p_response || NULL == p_response->p_intermediates ||
            0 == p_response->success) {
        LOGE("[SIM ME Lock] requestSimMeLockGetSharedKey Fail");
        ret = RIL_E_GENERIC_FAILURE;
        goto done;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start (&line);
    if (err < 0) {
        ret = RIL_E_GENERIC_FAILURE;
        goto done;
    }

    //Get shared key len
    err = at_tok_nextint(&line, &key_len);
    if (err < 0) {
        ret = RIL_E_GENERIC_FAILURE;
        goto done;
    }

    //Get shared key
    err = at_tok_nextstr(&line, &key);
    if (err < 0) {
        ret = RIL_E_GENERIC_FAILURE;
        goto done;
    }

done:
    RIL_onRequestComplete(t, ret, key, ((key == NULL) ? 0 : strlen(key)));
    at_response_free(p_response);

}

void requestSimMeLockUpdateSettings(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err;
    //const char** strings = (const char**)data;
    char *cmd;
    RIL_Errno ret = RIL_E_SUCCESS;

    LOGD("[SIM ME Lock] requestSimMeLockUpdateSettings: data=%s", ((char*)data));
    RIL_SIM_UNUSED_PARM(datalen);

    asprintf(&cmd, "AT+ETMOSLB=1,%zu,\"%s\"", ((strlen(data) + 1)/2), ((char*)data));
    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || NULL == p_response) {
        LOGE("[SIM ME Lock] requestSimMeLockUpdateSettings Fail");
        ret = RIL_E_GENERIC_FAILURE;
        goto done;
    }

    if (0 == p_response->success) {
        switch (at_get_cme_error(p_response)) {
            LOGD("p_response = %d /n", at_get_cme_error(p_response) );
            case CME_VERIFICATION_FAILED:
                ret = RIL_E_VERIFICATION_FAILED;
                LOGD("p_response: CME_SIM_LOCK_BLOB_VERIFICATION_FAILED");
                break;
            case CME_REBOOT_REQUEST:
                ret = RIL_E_REBOOT_REQUEST;
                LOGD("p_response: CME_REBOOT_REQUEST");
                //int eventId = 0;
                //RIL_UNSOL_RESPONSE(RIL_UNSOL_MELOCK_NOTIFICATION, &eventId, sizeof(int), 0);
                break;
            default:
                ret = RIL_E_GENERIC_FAILURE;
                goto done;
        }
    } else {
        setSimStatusChanged(getRILIdByChannelCtx(getRILChannelCtxFromToken(t)));
    }

done:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

void requestSimMeLockGetSettingsInfo(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err, info_len;
    char *cmd, *line, *info = NULL;
    RIL_Errno ret = RIL_E_SUCCESS;

    LOGD("[SIM ME Lock] requestSimMeLockGetSettingsInfo");
    RIL_SIM_UNUSED_PARM(data);
    RIL_SIM_UNUSED_PARM(datalen);

    asprintf(&cmd, "AT+ETMOSLB=3");
    err = at_send_command_singleline(cmd, "+ETMOSLB:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || NULL == p_response || NULL == p_response->p_intermediates ||
            0 == p_response->success) {
        LOGE("[SIM ME Lock] requestSimMeLockGetSettingsInfo Fail");
        ret = RIL_E_GENERIC_FAILURE;
        goto done;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start (&line);
    if (err < 0) {
        ret = RIL_E_GENERIC_FAILURE;
        goto done;
    }

    //Get info len
    err = at_tok_nextint(&line, &info_len);
    if (err < 0) {
        ret = RIL_E_GENERIC_FAILURE;
        goto done;
    }

    //Get info
    err = at_tok_nextstr(&line, &info);
    if (err < 0) {
        ret = RIL_E_GENERIC_FAILURE;
        goto done;
    }

done:
    RIL_onRequestComplete(t, ret, info, ((info==NULL) ? 0 : strlen(info)));
    at_response_free(p_response);

}

void requestSimMeLockResetSetting(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err;
    char *cmd;
    RIL_Errno ret = RIL_E_SUCCESS;

    LOGD("[SIM ME Lock] requestSimMeLockResetSetting");
    RIL_SIM_UNUSED_PARM(data);
    RIL_SIM_UNUSED_PARM(datalen);

    asprintf(&cmd, "AT+ETMOSLB=2");
    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || NULL == p_response) {
        LOGE("[SIM ME Lock] requestSimMeLockResetSetting Fail");
        ret = RIL_E_GENERIC_FAILURE;
        goto done;
    }

    if (0 == p_response->success) {
        switch (at_get_cme_error(p_response)) {
            LOGD("p_response = %d /n", at_get_cme_error(p_response) );
            case CME_VERIFICATION_FAILED:
                ret = RIL_E_VERIFICATION_FAILED;
                LOGD("p_response: CME_SIM_LOCK_BLOB_VERIFICATION_FAILED");
                break;
            case CME_REBOOT_REQUEST:
                ret = RIL_E_REBOOT_REQUEST;
                LOGD("p_response: CME_REBOOT_REQUEST");
                //int eventId = 0;
                //RIL_UNSOL_RESPONSE(RIL_UNSOL_MELOCK_NOTIFICATION, &eventId, sizeof(int), 0);
                break;
            default:
                ret = RIL_E_GENERIC_FAILURE;
                goto done;
        }
    } else {
        setSimStatusChanged(getRILIdByChannelCtx(getRILChannelCtxFromToken(t)));
    }

done:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

void requestSimMeLockGetModemStatus(void *data, size_t datalen, RIL_Token t) {
    LOGD("[SIM ME Lock] requestSimMeLockGetModemStatus");

    ATResponse *p_response = NULL;
    int err, status_len;
    char *cmd, *line, *status = NULL;
    RIL_Errno ret = RIL_E_SUCCESS;

    LOGD("[SIM ME Lock] requestSimMeLockGetModemStatus");
    RIL_SIM_UNUSED_PARM(data);
    RIL_SIM_UNUSED_PARM(datalen);

    asprintf(&cmd, "AT+ETMOSLB=4");
    err = at_send_command_singleline(cmd, "+ETMOSLB:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || NULL == p_response || NULL == p_response->p_intermediates ||
            0 == p_response->success) {
        LOGE("[SIM ME Lock] requestSimMeLockGetModemStatus Fail");
        ret = RIL_E_GENERIC_FAILURE;
        goto done;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start (&line);
    if (err < 0) {
        ret = RIL_E_GENERIC_FAILURE;
        goto done;
    }

    //Get modem status len
    err = at_tok_nextint(&line, &status_len);
    if (err < 0) {
        ret = RIL_E_GENERIC_FAILURE;
        goto done;
    }

    //Get modem status
    err = at_tok_nextstr(&line, &status);
    if (err < 0) {
        ret = RIL_E_GENERIC_FAILURE;
        goto done;
    }

done:
    RIL_onRequestComplete(t, ret, status, ((status == NULL) ? 0 : strlen(status)));
    at_response_free(p_response);

}
// MTK-END

extern int rilSimMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request)
    {
        case RIL_REQUEST_GET_SIM_STATUS:
            requestGetSimStatus(data,datalen,t);
        break;
        case RIL_REQUEST_GET_IMSI:
            requestGetImsi(data,datalen,t);
        break;
        case RIL_REQUEST_SIM_IO:
            requestSimIo(data,datalen,t);
        break;
        case RIL_REQUEST_ENTER_SIM_PIN:
            simSecurityOperation(data,datalen,t,ENTER_PIN1);
        break;
        case RIL_REQUEST_ENTER_SIM_PUK:
            simSecurityOperation(data,datalen,t,ENTER_PUK1);
        break;
        case RIL_REQUEST_ENTER_SIM_PIN2:
            simSecurityOperation(data,datalen,t,ENTER_PIN2);
        break;
        case RIL_REQUEST_ENTER_SIM_PUK2:
            simSecurityOperation(data,datalen,t,ENTER_PUK2);
        break;
        case RIL_REQUEST_CHANGE_SIM_PIN:
            simSecurityOperation(data,datalen,t,CHANGE_PIN1);
        break;
        case RIL_REQUEST_CHANGE_SIM_PIN2:
            simSecurityOperation(data,datalen,t,CHANGE_PIN2);
        break;
        case RIL_REQUEST_ENTER_NETWORK_DEPERSONALIZATION:
            requestEnterNetworkDepersonalization(data,datalen,t);
        break;
        case RIL_REQUEST_QUERY_FACILITY_LOCK:
        case RIL_REQUEST_SET_FACILITY_LOCK:
            simFacilityLock(data,datalen,t);
        break;

        // MTK-START: SIM ME LOCK
        case RIL_REQUEST_QUERY_SIM_NETWORK_LOCK:
            queryNetworkLock(data, datalen, t);
        break;
        case RIL_REQUEST_SET_SIM_NETWORK_LOCK:
            simNetworkLock(data,datalen,t);
        break;
        // MTK-END
        // NFC SEEK start
        case RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC:
            requestSIM_TransmitBasic(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_OPEN_CHANNEL:
            requestSIM_OpenChannelWithSw(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_CLOSE_CHANNEL:
            requestSIM_CloseChannel(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL:
            requestSIM_TransmitChannel(data, datalen, t);
            break;
        case RIL_REQUEST_SIM_GET_ATR:
            requestSIM_GetATR(data, datalen, t);
            break;
        // NFC SEEK end
        case RIL_REQUEST_SIM_AUTHENTICATION:
            requestSimAuthentication(data, datalen, t);
            return 0;
        case RIL_REQUEST_SET_SIM_CARD_POWER:
            requestSetSimCardPower(data, datalen, t);
            return 0;
        // MTK-START: ISIM
        case RIL_REQUEST_ISIM_AUTHENTICATION:
            requestIsimAuthentication(data, datalen, t);
            break;
        // MTK-END
        case RIL_LOCAL_GSM_REQUEST_SWITCH_CARD_TYPE:
            requestSwitchCardType(data, datalen, t);
            break;
        // SIM power [Start]
        case RIL_REQUEST_SET_SIM_POWER:
            requestSetSimPower(data, datalen, t);
            break;
        // SIM power [End]
        // MTK-START: SIM GBA
        case RIL_REQUEST_GENERAL_SIM_AUTH:
            requestGeneralSimAuth(data, datalen, t);
            break;
        // MTK-END
        // MTK-START: SIM TMO RSU
        case RIL_LOCAL_REQUEST_GET_SHARED_KEY:
            requestSimMeLockGetSharedKey(data, datalen, t);
            break;
        case RIL_LOCAL_REQUEST_UPDATE_SIM_LOCK_SETTINGS:
            requestSimMeLockUpdateSettings(data, datalen, t);
            break;
        case RIL_LOCAL_REQUEST_GET_SIM_LOCK_INFO:
            requestSimMeLockGetSettingsInfo(data, datalen, t);
            break;
        case RIL_LOCAL_REQUEST_RESET_SIM_LOCK_SETTINGS:
            requestSimMeLockResetSetting(data, datalen, t);
            break;
        case RIL_LOCAL_REQUEST_GET_MODEM_STATUS:
            requestSimMeLockGetModemStatus(data, datalen, t);
            break;
        // MTK-END
        // Exteranl SIM [Start]
    #ifdef MTK_EXTERNAL_SIM_SUPPORT
        case RIL_REQUEST_VSIM_NOTIFICATION:
            requestVsimNotification(data, datalen, t);
            break;
        case RIL_REQUEST_VSIM_OPERATION:
            requestVsimOperation(data, datalen, t);
            break;
    #endif
        // External SIM [End]
        case RIL_LOCAL_REQUEST_SIM_GET_EFDIR:
            requestSimGetEfdir(data, datalen, t);
            break;
        // MTK-START:  SIM SLOT LOCK
        case RIL_REQUEST_ENTER_DEVICE_NETWORK_DEPERSONALIZATION:
            requestEnterDeviceNetworkDepersonalization(data,datalen,t);
            break;
        // MTK-END
        case RIL_REQUEST_SET_ALLOWED_CARRIERS:
            requestSetAllowedCarriers(data, datalen, t);
            break;
        case RIL_REQUEST_GET_ALLOWED_CARRIERS:
            requestGetAllowedCarriers(data, datalen, t);
            break;
        default:
            return 0; /* no match */
        break;
    }
    return 1; /* request find */
}

extern int rilSimUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel)
{
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    RIL_SIM_UNUSED_PARM(sms_pdu);

    if (strStartsWith(s, "+EUSIM:")) {
        RLOGD("EUSIM URC:%s",s);
        onUsimDetected(s,rid);
    } else if (strStartsWith(s, "+ETESTSIM:")) {
        RLOGD("ETESTSIM URC:%s",s);
        onTestSimDetected(s, rid);
    } else if (strStartsWith(s, "+ESIMS:")) {
        onSimInsertChanged(s, rid);
    } else if (strStartsWith(s, "+ESIMAPP:")) {
        RLOGD("ESIMAPP URC:%s",s);
        onSessionIdChanged(s, rid);
    // MTK-START: SIM TMO RSU
    } else if (strStartsWith(s, "+ETMOEVT")) {
        RLOGD("ETMOEVT URC:%s",s);
        onSimMeLockEvent(s, rid);
    // MTK-END
    } else if (strStartsWith(s, "+ECT3G:")) {
        onCt3gDetected(s, rid);
    // External SIM [Start]
#ifdef MTK_EXTERNAL_SIM_SUPPORT
    } else if (strStartsWith(s, "+EIND: 32")) {
        // Add for slow SIM card error handling.
        // Backgournd: get sim status try 30 times with 0.2 second sleep duration.
        // If the card can't get non-busy status within 6 second, then Java layer always can't
        // get correct sim status and broadcast them.
        // Solution: we need a sim status changed unsolicited message to trigger java layer once
        // modem complete SIM card initialization.
        // FIXME: Actually, we need an event represent the SIM card complete initialization.
        // Modem suggest AP use phonebook start to initialization event as a SIM card
        // initialization done. Might change to other exactly URC in the further.
        // Reference CR: ALPS02408560
        if (isVsimEnabledByRid(rid) && isRecoverySimState[rid] == 1) {
            isRecoverySimState[rid] = 0;
            setSimStatusChanged(rid);
        }
        return 0;
    } else if (strStartsWith(s, "+ERSAIND:")) {
        onVsimEventDetected(s, rid);
    } else if (strStartsWith(s, "+ERSAAUTH:") || strStartsWith(s, "+ERSIMAUTH:")) {
        onVsimAuthEventDetected(s, rid);
#endif
    // External SIM [End]
    } else if (strStartsWith(s, "+ESIMIND:")) {
        onHandleEsimInd(s, rid);
    } else if (strStartsWith(s, "+ESMLSTATUS:")) {
        onSmlStatusChanged(s, rid);
    } else {
        return 0;
    }

    return 1;
}

/* ALPS01962408, workaround to reset ICCID, 2015/03/01 {*/
static void resetSimIccid(const char *p_iccid_sim) {
    RLOGD("%s: Reset p_iccid_sim(%s).", __FUNCTION__, p_iccid_sim);
    property_set(p_iccid_sim, "");
}
/* ALPS01962408, workaround to reset ICCID, 2015/03/01 }*/

void resetSIMProperties(RIL_SOCKET_ID rid) {
    property_set(PROPERTY_RIL_SIM_PIN1[rid], NULL);
    property_set(PROPERTY_RIL_SIM_PUK1[rid], NULL);
    property_set(PROPERTY_RIL_SIM_PIN2[rid], NULL);
    property_set(PROPERTY_RIL_SIM_PUK2[rid], NULL);
    property_set(PROPERTY_RIL_UICC_TYPE[rid],NULL);
    property_set(PROPERTY_ICCID_SIM[rid], NULL);
    /* ALPS01962408, workaround to reset ICCID, 2015/03/01 {*/
    if (isCdmaLteDcSupport()) {
        resetSimIccid(PROPERTY_ICCID_SIM[rid]);
        property_set(PROPERTY_RIL_FULL_UICC_TYPE[rid], NULL);
    }
    /* ALPS01962408, workaround to reset ICCID, 2015/03/01 }*/

    // MTK-START:  SIM SLOT LOCK
    if (isSimSlotLockSupport() == 1) {
        LOGI("Slot lock on and reset related properties at rid: %d", rid);
        property_set(PROPERTY_SIM_SLOT_LOCK_POLICY, "-1");
        property_set(PROPERTY_SIM_SLOT_LOCK_STATE, "-1");
        property_set(PROPERTY_SIM_SLOT_LOCK_SERVICE_CAPABILITY[rid], "-1");
        property_set(PROPERTY_SIM_SLOT_LOCK_CARD_VALID[rid], "-1");
    } else {
        property_set(PROPERTY_SIM_SLOT_LOCK_POLICY, "0");
        property_set(PROPERTY_SIM_SLOT_LOCK_STATE, "1");
        property_set(PROPERTY_SIM_SLOT_LOCK_SERVICE_CAPABILITY[rid], "0");
        property_set(PROPERTY_SIM_SLOT_LOCK_CARD_VALID[rid], "0");
    }
    //MTK-END
}

// MTK-START: BT SIM Access Profile
extern int rilBtSapMain(int request, void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    switch (request) {
        case MsgId_RIL_SIM_SAP_CONNECT:
            requestBtSapConnect(data, datalen, t, rid);
            break;
        case MsgId_RIL_SIM_SAP_DISCONNECT:
            requestBtSapDisconnect(data, datalen, t, rid);
            break;
        case MsgId_RIL_SIM_SAP_APDU:
            requestBtSapTransferApdu(data, datalen, t, rid);
            break;
        case MsgId_RIL_SIM_SAP_TRANSFER_ATR:
            requestBtSapTransferAtr(data, datalen, t, rid);
            break;
        case MsgId_RIL_SIM_SAP_POWER:
            requestBtSapPower(data, datalen, t, rid);
            break;
        case MsgId_RIL_SIM_SAP_RESET_SIM:
            requestBtSapResetSim(data, datalen, t, rid);
            break;
        case MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL:
            requestBtSapSetTransferProtocol(data, datalen, t, rid);
            break;
        default:
            return 0;
            break;
    }

    return 1;
}

BtSapStatus mBtSapStatus[SIM_COUNT] = {BT_SAP_INIT};
char* mBtSapAtr[SIM_COUNT] = {NULL};
int mBtSapCurrentProtocol[SIM_COUNT] = {0};
int mBtSapSupportProtocol[SIM_COUNT] = {0};


void resetBtSapContext(RIL_SOCKET_ID rid) {
    RLOGD("[BTSAP] resetBtSapAtr (%d)", rid);
    mBtSapStatus[rid] = BT_SAP_INIT;
    mBtSapCurrentProtocol[rid] = 0;
    mBtSapSupportProtocol[rid] = 0;
    if (mBtSapAtr[rid] != NULL) {
        free(mBtSapAtr[rid]);
        mBtSapAtr[rid] = NULL;
    }
}

BtSapStatus queryBtSapStatus(RIL_SOCKET_ID rid) {
    RLOGD("[BTSAP] mBtSapStatus (%d, %d)", rid, mBtSapStatus[rid]);
    return mBtSapStatus[rid];
}

static LocalBtSapMsgHeader* allocateLocalBtSapMsgHeader(void *param, RIL_Token t,
        RIL_SOCKET_ID rid) {
    LocalBtSapMsgHeader *localMsgHdr = (LocalBtSapMsgHeader*)calloc(1,sizeof(LocalBtSapMsgHeader));
    assert(localMsgHdr != NULL);
    localMsgHdr->t = t;
    localMsgHdr->param = param;
    localMsgHdr->socket_id = rid;

    return localMsgHdr;
}

static void releaseLocalBtSapMsgHeader(LocalBtSapMsgHeader *localMsgHdr) {
    if (localMsgHdr != NULL) {
        if (localMsgHdr->param != NULL) {
            free(localMsgHdr->param);
        }
        free(localMsgHdr);
    }
}

static void decodeBtSapPayload(MsgId msgId, void *src, size_t srclen, void *dst) {
    pb_istream_t stream;
    const pb_field_t *fields = NULL;

    RLOGD("[BTSAP] decodeBtSapPayload start (%s), srclen:%zu", btSapMsgIdToString(msgId), srclen);
    if (dst == NULL || src == NULL) {
        RLOGE("[BTSAP] decodeBtSapPayload, dst or src is NULL!!");
        return;
    }

    switch (msgId) {
        case MsgId_RIL_SIM_SAP_CONNECT:
            fields = RIL_SIM_SAP_CONNECT_REQ_fields;
            break;
        case MsgId_RIL_SIM_SAP_DISCONNECT:
            fields = RIL_SIM_SAP_DISCONNECT_REQ_fields;
            break;
        case MsgId_RIL_SIM_SAP_APDU:
            fields = RIL_SIM_SAP_APDU_REQ_fields;
            break;
        case MsgId_RIL_SIM_SAP_TRANSFER_ATR:
            fields = RIL_SIM_SAP_TRANSFER_ATR_REQ_fields;
            break;
        case MsgId_RIL_SIM_SAP_POWER:
            fields = RIL_SIM_SAP_POWER_REQ_fields;
            break;
        case MsgId_RIL_SIM_SAP_RESET_SIM:
            fields = RIL_SIM_SAP_RESET_SIM_REQ_fields;
            break;
        case MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL:
            fields = RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_REQ_fields;
            break;
        default:
            RLOGE("[BTSAP] decodeBtSapPayload, MsgId is mistake!");
            return;
    }

    stream = pb_istream_from_buffer((uint8_t *)src, srclen);
    if (!pb_decode(&stream, fields, dst) ) {
        RLOGE("[BTSAP] decodeBtSapPayload, Error decoding protobuf buffer : %s", PB_GET_ERROR(&stream));
    } else {
        RLOGD("[BTSAP] decodeBtSapPayload, Success!");
    }
}

static void requestBtSapConnect(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    RIL_SIM_SAP_CONNECT_REQ *req = NULL;
    RIL_SIM_SAP_CONNECT_RSP rsp;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RILChannelCtx *p_cctx = getRILChannelCtxFromToken(t);
    //FIXME: To get from system property
    int sysMaxSize = 32767;

    req = (RIL_SIM_SAP_CONNECT_REQ*)calloc(1, sizeof(RIL_SIM_SAP_CONNECT_REQ));
    assert(req != NULL);
    decodeBtSapPayload(MsgId_RIL_SIM_SAP_CONNECT, data, datalen, req);

    RLOGD("[BTSAP] requestBtSapConnect (%d,%d,%d)", rid, req->max_message_size, sysMaxSize);
    do {
        if (req->max_message_size > sysMaxSize) {
            // Max message size from request > project setting
            rsp.response = RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_MSG_SIZE_TOO_LARGE;
            rsp.max_message_size = sysMaxSize;
            rsp.has_max_message_size = TRUE;
        //} else if (req->max_message_size < sysMaxSize) {
            // Max message size from request < project setting
        //    rsp.response = RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_MSG_SIZE_TOO_SMALL;
        //    rsp.max_message_size = sysMaxSize;
        //    rsp.has_max_message_size = TRUE;
        } else {
            // Send connect request
            RIL_SIM_SAP_CONNECT_REQ *local_req =
                (RIL_SIM_SAP_CONNECT_REQ*)calloc(1, sizeof(RIL_SIM_SAP_CONNECT_REQ));
            assert(local_req != NULL);
            memcpy(local_req, req, sizeof(RIL_SIM_SAP_CONNECT_REQ));
            LocalBtSapMsgHeader *localMsgHeader = allocateLocalBtSapMsgHeader(local_req, t, rid);
            if (p_cctx == NULL) {
                RLOGE("[BTSAP] requestBtSapDisconnect p_cctx == NULL");
            } else {
                RIL_requestProxyTimedCallback (requestLocalSapConnect, localMsgHeader, NULL,
                        p_cctx->id, "requestLocalSapConnect");
                break;
            }
        }

        // Send CONNECTION_RESP directly because max message size from request is wrong
        //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
        sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_CONNECT, &rsp);
    } while(0);

    free(req);

    RLOGD("[BTSAP] requestBtSapConnect end");
}

static void requestLocalSapConnect(void *param) {
    LocalBtSapMsgHeader *localMsgHdr = (LocalBtSapMsgHeader*)param;
    RIL_Token t = localMsgHdr->t;
    RIL_SOCKET_ID rid = localMsgHdr->socket_id;
    RIL_SIM_SAP_CONNECT_REQ *req = localMsgHdr->param;
    RIL_SIM_SAP_CONNECT_RSP rsp;
    ATResponse *p_response = NULL;
    int err, type = -1;
    char *line = NULL, *pAtr = NULL;
    RILChannelCtx *p_cctx = getChannelCtxbyProxy();
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RIL_SIM_SAP_STATUS_IND_Status unsolMsg = RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_UNKNOWN_ERROR;

    RLOGD("[BTSAP] requestLocalSapConnect start");
    err = at_send_command_singleline("AT+EBTSAP=0", "+EBTSAP:", &p_response, p_cctx);

    if (err < 0 || NULL == p_response) {
        rsp.response = RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_CONNECT_FAILURE;
        goto format_error;
    }

    if (p_response->success == 0) {
        RLOGE("[BTSAP] CME ERROR = %d", at_get_cme_error(p_response));
        rsp.response = RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_CONNECT_FAILURE;
        switch (at_get_cme_error(p_response)) {
            case CME_BT_SAP_UNDEFINED:
                ret = RIL_E_BT_SAP_UNDEFINED;
                break;
            case CME_BT_SAP_NOT_ACCESSIBLE:
                ret = RIL_E_BT_SAP_NOT_ACCESSIBLE;
                unsolMsg = RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_CARD_NOT_ACCESSIBLE;
                break;
            case CME_BT_SAP_CARD_REMOVED:
            case CME_SIM_NOT_INSERTED:
                ret = RIL_E_BT_SAP_CARD_REMOVED;
                unsolMsg = RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_CARD_REMOVED;
                break;
            default:
                break;
        }
        goto format_error;
    } else {
        line = p_response->p_intermediates->line;
        rsp.response = RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_CONNECT_FAILURE;

        err = at_tok_start(&line);
        if (err < 0) goto format_error;

        err = at_tok_nextint(&line, &type);
        if (err < 0) goto format_error;

        mBtSapCurrentProtocol[rid] = type;

        err = at_tok_nextint(&line, &type);
        if (err < 0) goto  format_error;

        mBtSapSupportProtocol[rid] = type;

        err = at_tok_nextstr(&line, &pAtr);
        if (err < 0) goto  format_error;

        int atrLen = 0;
        if (pAtr != NULL) {
            atrLen = strlen(pAtr);
            mBtSapAtr[rid] = (char*)calloc(1, (atrLen+1)*sizeof(char));
            if (NULL == mBtSapAtr[rid]){
                RLOGE("[BTSAP] mBtSapAtr alloc OOM.");
                ret = RIL_E_NO_MEMORY;
                goto format_error;
            }
            memcpy(mBtSapAtr[rid], pAtr, atrLen);
        }

        RLOGD("[BTSAP] requestLocalSapConnect, cur_type: %d, supp_type: %d, size: %d",
                mBtSapCurrentProtocol[rid], mBtSapSupportProtocol[rid], atrLen);

        ret = RIL_E_SUCCESS;
        // For AOSP BT SAP UT case, to set the flag has_max_message_size as true.
        //rsp.has_max_message_size = false;
        rsp.has_max_message_size = true;
        rsp.max_message_size = 0;
        if (inCallNumberPerSim[rid] > 0) {
            // There is an ongoing call so do not notify STATUS_IND now
            RLOGD("[BTSAP] Connection Success but there is ongoing call");
            rsp.response = RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_CONNECT_OK_CALL_ONGOING;
        } else {
            RLOGD("[BTSAP] Connection Success");
            rsp.response = RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SUCCESS;
            unsolMsg = RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_CARD_RESET;
        }
        mBtSapStatus[rid] = BT_SAP_CONNECTION_SETUP;
    }

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_CONNECT, &rsp);

    at_response_free(p_response);
    releaseLocalBtSapMsgHeader(localMsgHdr);

    if (unsolMsg != RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_UNKNOWN_ERROR) {
        notifyBtSapStatusInd(unsolMsg, rid);
    }
    RLOGD("[BTSAP] requestLocalSapConnect end");
    return;
format_error:
    RLOGE("[BTSAP] Connection Fail");
    rsp.has_max_message_size = false;
    rsp.max_message_size = 0;

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_CONNECT, &rsp);

    at_response_free(p_response);
    releaseLocalBtSapMsgHeader(localMsgHdr);
    RLOGD("[BTSAP] requestLocalSapConnect end");
}

static void requestBtSapDisconnect(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    RIL_SIM_SAP_DISCONNECT_REQ *req = NULL;
    RIL_SIM_SAP_DISCONNECT_RSP rsp;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RILChannelCtx *p_cctx = getRILChannelCtxFromToken(t);
    if (p_cctx == NULL) {
        RLOGE("[BTSAP] requestBtSapDisconnect p_cctx == NULL");
        return;
    }
    RLOGE("[BTSAP] requestBtSapDisconnect start");
    req = (RIL_SIM_SAP_DISCONNECT_REQ*)calloc(1, sizeof(RIL_SIM_SAP_DISCONNECT_REQ));
    assert(req != NULL);
    decodeBtSapPayload(MsgId_RIL_SIM_SAP_DISCONNECT, data, datalen, req);

    mBtSapStatus[rid] = BT_SAP_DISCONNECT;
    if (isSimInserted(rid)) {
        // Send disconnect request
        RIL_SIM_SAP_DISCONNECT_REQ *local_req =
            (RIL_SIM_SAP_DISCONNECT_REQ*)calloc(1, sizeof(RIL_SIM_SAP_DISCONNECT_REQ));
        assert(local_req != NULL);
        memcpy(local_req, req, sizeof(RIL_SIM_SAP_DISCONNECT_REQ));
        LocalBtSapMsgHeader *localMsgHeader = allocateLocalBtSapMsgHeader(local_req, t, rid);
        RLOGE("[BTSAP] requestBtSapDisconnect start disconnect");
        RIL_requestProxyTimedCallback (requestLocalBtSapDisconnect, localMsgHeader, NULL,
                p_cctx->id, "requestLocalBtSapDisconnect");
    } else {
        RLOGE("[BTSAP] requestBtSapDisconnect but card was removed");
        rsp.dummy_field = 1;
        mBtSapStatus[rid] = BT_SAP_INIT;
        ret = RIL_E_SUCCESS;
        // FIXME: Send response immediately
        //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
        sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_DISCONNECT, &rsp);
    }
    free(req);
    RLOGE("[BTSAP] requestBtSapDisconnect end");
}

static void requestLocalBtSapDisconnect(void *param) {
    LocalBtSapMsgHeader *localMsgHdr = (LocalBtSapMsgHeader*)param;
    RIL_Token t = localMsgHdr->t;
    RIL_SOCKET_ID rid = localMsgHdr->socket_id;
    RIL_SIM_SAP_DISCONNECT_REQ *req = localMsgHdr->param;
    RIL_SIM_SAP_DISCONNECT_RSP rsp;
    ATResponse *p_response = NULL;
    int err;
    RILChannelCtx *p_cctx = getChannelCtxbyProxy();
    char *line = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    RLOGD("[BTSAP] requestLocalBtSapDisconnect start");
    err = at_send_command("AT+EBTSAP=1", &p_response, p_cctx);

    if (err < 0 || NULL == p_response) {
        goto format_error;
    }

    if (p_response->success == 0) {
        RLOGE("[BTSAP] CME ERROR = %d", at_get_cme_error(p_response));
        switch (at_get_cme_error(p_response)) {
            case CME_BT_SAP_UNDEFINED:
                ret = RIL_E_BT_SAP_UNDEFINED;
                break;
            case CME_BT_SAP_NOT_ACCESSIBLE:
                ret = RIL_E_BT_SAP_NOT_ACCESSIBLE;
                break;
            case CME_BT_SAP_CARD_REMOVED:
            case CME_SIM_NOT_INSERTED:
                ret = RIL_E_BT_SAP_CARD_REMOVED;
                break;
            default:
                break;
        }
        goto format_error;
    } else {
        resetBtSapContext(rid);
        ret = RIL_E_SUCCESS;
        rsp.dummy_field = 1;
    }

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_DISCONNECT, &rsp);

    at_response_free(p_response);
    releaseLocalBtSapMsgHeader(localMsgHdr);

    RLOGD("[BTSAP] requestLocalBtSapDisconnect end");
    return;
format_error:
    RLOGE("[BTSAP] Fail to disconnect");
    mBtSapStatus[rid] = BT_SAP_INIT;
    rsp.dummy_field = 0;

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_DISCONNECT, &rsp);

    at_response_free(p_response);
    releaseLocalBtSapMsgHeader(localMsgHdr);

    RLOGD("[BTSAP] requestLocalBtSapDisconnect end");
}

static char* convertBtSapIntToHexString(uint8_t *data, size_t datalen) {
    char* output = NULL, *pOut = NULL;
    size_t i = 0;

    if (data == NULL || datalen <= 0) {
        return output;
    }

    output = (char*)calloc(1, (sizeof(char)*datalen+1)*2);
    if (NULL == output) {
        return NULL;
    }
    pOut = output;

    for (i = 0; i < datalen; i++) {
        pOut = &output[2*i];
        sprintf(pOut, "%02x", data[i]);
    }

    RLOGD("[BTSAP] convert string (%zu, %s)", datalen, output);
    return output;
}

static void requestBtSapTransferApdu(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    RIL_SIM_SAP_APDU_REQ *req = NULL;
    RIL_SIM_SAP_APDU_RSP rsp;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RILChannelCtx *p_cctx = getRILChannelCtxFromToken(t);

    req = (RIL_SIM_SAP_APDU_REQ*)calloc(1, sizeof(RIL_SIM_SAP_APDU_REQ));
    assert(req != NULL);
    decodeBtSapPayload(MsgId_RIL_SIM_SAP_APDU, data, datalen, req);
    RLOGD("[BTSAP] requestBtSapTransferApdu start, (%d,%d)", rid, req->type);
    do {
        if (!isSimInserted(rid)) {
            RLOGD("[BTSAP] requestBtSapTransferApdu but card was removed");
            rsp.response = RIL_SIM_SAP_APDU_RSP_Response_RIL_E_SIM_ABSENT;
            ret = RIL_E_BT_SAP_CARD_REMOVED;
        } else if (queryBtSapStatus(rid) == BT_SAP_POWER_OFF) {
            RLOGD("[BTSAP] requestBtSapTransferApdu but card was already power off");
            rsp.response = RIL_SIM_SAP_APDU_RSP_Response_RIL_E_SIM_ALREADY_POWERED_OFF;
        } else {
            // Send disconnect request
            RIL_SIM_SAP_APDU_REQ *local_req =
                (RIL_SIM_SAP_APDU_REQ*)calloc(1, sizeof(RIL_SIM_SAP_APDU_REQ));
            assert(local_req != NULL);
            pb_bytes_array_t *payload = (pb_bytes_array_t *)calloc(
                    1,sizeof(pb_bytes_array_t) + req->command->size);
            assert(payload != NULL);
            memcpy(payload->bytes, req->command->bytes, req->command->size);
            payload->size = req->command->size;
            local_req->command = payload;
            local_req->type = req->type;
            LocalBtSapMsgHeader *localMsgHeader = allocateLocalBtSapMsgHeader(local_req, t, rid);
            RIL_requestProxyTimedCallback (requestLocalBtSapTransferApdu, localMsgHeader, NULL,
                    p_cctx->id, "requestLocalBtSapTransferApdu");
            break;
        }

        // FIXME: Send response immediately
        //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
        sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_APDU, &rsp);
    } while(0);

    free(req);

    RLOGD("[BTSAP] requestBtSapTransferApdu end");
}

static void requestLocalBtSapTransferApdu(void *param) {
    LocalBtSapMsgHeader *localMsgHdr = (LocalBtSapMsgHeader*)param;
    RIL_Token t = localMsgHdr->t;
    RIL_SOCKET_ID rid = localMsgHdr->socket_id;
    RIL_SIM_SAP_APDU_REQ *req = localMsgHdr->param;
    RIL_SIM_SAP_APDU_RSP rsp;
    ATResponse *p_response = NULL;
    int err;
    char* cmd = NULL;
    char *line = NULL;
    char *pApduResponse = NULL;
    char *hexStr = NULL;
    RILChannelCtx *p_cctx = getChannelCtxbyProxy();
    pb_bytes_array_t *apduResponse = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    hexStr = convertBtSapIntToHexString(req->command->bytes, req->command->size);
    if (NULL == hexStr) {
        RLOGE("[BTSAP] hexStr OOM.");
        ret = RIL_E_NO_MEMORY;
        goto format_error;
    }
    RLOGD("[BTSAP] requestLocalBtSapTransferApdu start, (%d,%d,%zu,%s)", rid, req->type,
            req->command->size, hexStr);
    memset(&rsp, 0, sizeof(RIL_SIM_SAP_APDU_RSP));
    asprintf(&cmd, "AT+EBTSAP=5,0,\"%s\"", hexStr);
    err = at_send_command_singleline(cmd, "+EBTSAP:", &p_response, p_cctx);

    if (hexStr != NULL) {
        free(hexStr);
    }
    free(cmd);

    if (err < 0 || NULL == p_response) {
        rsp.response = RIL_SIM_SAP_APDU_RSP_Response_RIL_E_GENERIC_FAILURE;
        goto format_error;
    }

    if (p_response->success == 0) {
        RLOGE("[BTSAP] CME ERROR = %d", at_get_cme_error(p_response));
        rsp.response = RIL_SIM_SAP_APDU_RSP_Response_RIL_E_GENERIC_FAILURE;
        switch (at_get_cme_error(p_response)) {
            case CME_BT_SAP_UNDEFINED:
                ret = RIL_E_BT_SAP_UNDEFINED;
                rsp.response = RIL_SIM_SAP_APDU_RSP_Response_RIL_E_GENERIC_FAILURE;
                break;
            case CME_BT_SAP_NOT_ACCESSIBLE:
                ret = RIL_E_BT_SAP_NOT_ACCESSIBLE;
                rsp.response = RIL_SIM_SAP_APDU_RSP_Response_RIL_E_SIM_NOT_READY;
                break;
            case CME_BT_SAP_CARD_REMOVED:
            case CME_SIM_NOT_INSERTED:
                ret = RIL_E_BT_SAP_CARD_REMOVED;
                rsp.response = RIL_SIM_SAP_APDU_RSP_Response_RIL_E_SIM_ABSENT;
                break;
            default:
                break;
        }
        goto format_error;
    } else {
        char *pRes = NULL;

        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if (err < 0) goto format_error;

        err = at_tok_nextstr(&line, &pApduResponse);
        if (err < 0) goto  format_error;

        int apduLen = strlen(pApduResponse);
        int reslen = hexStringToByteArrayEx((unsigned char*)pApduResponse, apduLen,
                (unsigned char**)&pRes);
        apduResponse = (pb_bytes_array_t *)calloc(1,sizeof(pb_bytes_array_t) + reslen);
        if (NULL == apduResponse) {
            RLOGE("[BTSAP] apduResponse OOM.");
            ret = RIL_E_NO_MEMORY;
            goto format_error;
        }
        memcpy(apduResponse->bytes, pRes, reslen);
        apduResponse->size = reslen;
        rsp.apduResponse = apduResponse;
        rsp.response = RIL_SIM_SAP_APDU_RSP_Response_RIL_E_SUCCESS;
        ret = RIL_E_SUCCESS;
        free(pRes);
    }

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_APDU, &rsp);

    if (apduResponse != NULL) {
        free(apduResponse);
    }
    at_response_free(p_response);
    if (req->command != NULL) {
        free(req->command);
    }
    releaseLocalBtSapMsgHeader(localMsgHdr);
    RLOGD("[BTSAP] requestLocalBtSapTransferApdu end");
    return;
format_error:
    RLOGE("[BTSAP] Fail to send APDU");

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_APDU, &rsp);

    at_response_free(p_response);
    if (req->command != NULL) {
        free(req->command);
    }
    releaseLocalBtSapMsgHeader(localMsgHdr);
    RLOGD("[BTSAP] requestLocalBtSapTransferApdu end");
}

static void requestBtSapTransferAtr(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    RIL_SIM_SAP_TRANSFER_ATR_REQ *req = NULL;
    RIL_SIM_SAP_TRANSFER_ATR_RSP rsp;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    BtSapStatus status = -1;
    pb_bytes_array_t *atrResponse = NULL;
    RILChannelCtx *p_cctx = getRILChannelCtxFromToken(t);

    RLOGD("[BTSAP] requestBtSapTransferAtr start, (%d)", rid);
    req = (RIL_SIM_SAP_TRANSFER_ATR_REQ*)calloc(1, sizeof(RIL_SIM_SAP_TRANSFER_ATR_REQ));
    assert(req != NULL);
    decodeBtSapPayload(MsgId_RIL_SIM_SAP_TRANSFER_ATR, data, datalen, req);

    status = queryBtSapStatus(rid);
    if (!isSimInserted(rid)) {
        RLOGD("[BTSAP] requestBtSapTransferAtr but card was removed");
        rsp.response = RIL_SIM_SAP_TRANSFER_ATR_RSP_Response_RIL_E_SIM_ABSENT;
        rsp.atr = NULL;
        ret = RIL_E_BT_SAP_CARD_REMOVED;
    } else if (status == BT_SAP_POWER_OFF) {
        RLOGD("[BTSAP] requestBtSapTransferAtr but card was already power off");
        rsp.response = RIL_SIM_SAP_TRANSFER_ATR_RSP_Response_RIL_E_SIM_ALREADY_POWERED_OFF;
        rsp.atr = NULL;
    } else if (status == BT_SAP_POWER_ON ||
                status == BT_SAP_CONNECTION_SETUP ||
                status == BT_SAP_ONGOING_CONNECTION) {
        char *pRes = NULL;
        rsp.response = RIL_SIM_SAP_TRANSFER_ATR_RSP_Response_RIL_E_SUCCESS;
        int atrLen = strlen(mBtSapAtr[rid]);
        int reslen = hexStringToByteArrayEx((unsigned char *)mBtSapAtr[rid], atrLen,
                (unsigned char **)&pRes);
        atrResponse = (pb_bytes_array_t *)calloc(1,sizeof(pb_bytes_array_t) + reslen);
        assert(atrResponse != NULL);
        memcpy(atrResponse->bytes, pRes, reslen);
        atrResponse->size = reslen;
        rsp.atr = atrResponse;
        if (pRes != NULL) {
            free(pRes);
        }

        RLOGD("[BTSAP] requestBtSapTransferAtr, status: %d, size: %d", status, atrLen);

        if (status == BT_SAP_CONNECTION_SETUP) {
            mBtSapStatus[rid] = BT_SAP_ONGOING_CONNECTION;
        }
    } else {
        rsp.response = RIL_SIM_SAP_TRANSFER_ATR_RSP_Response_RIL_E_GENERIC_FAILURE;
        rsp.atr = NULL;
    }
    // FIXME: Send response immediately
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_TRANSFER_ATR, &rsp);

    if (atrResponse != NULL) {
        free(atrResponse);
    }

    free(req);
    RLOGD("[BTSAP] requestBtSapTransferAtr end");
}

static void requestLocalBtSapTransferAtr(void *data, size_t datalen, RIL_Token t) {
    RIL_SIM_UNUSED_PARM(data);
    RIL_SIM_UNUSED_PARM(datalen);
    RIL_SIM_UNUSED_PARM(t);
}

static void requestBtSapPower(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    RIL_SIM_SAP_POWER_REQ *req = NULL;
    RIL_SIM_SAP_POWER_RSP rsp;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    BtSapStatus status = -1;
    RILChannelCtx *p_cctx = getRILChannelCtxFromToken(t);

    req = (RIL_SIM_SAP_POWER_REQ*)calloc(1, sizeof(RIL_SIM_SAP_POWER_REQ));
    assert(req != NULL);
    decodeBtSapPayload(MsgId_RIL_SIM_SAP_POWER, data, datalen, req);

    RLOGD("[BTSAP] requestBtSapPower start, (%d,%d)", rid, req->state);
    status = queryBtSapStatus(rid);
    do {
        if (!isSimInserted(rid)) {
            RLOGD("[BTSAP] requestBtSapPower but card was removed");
            rsp.response = RIL_SIM_SAP_POWER_RSP_Response_RIL_E_SIM_ABSENT;
            ret = RIL_E_BT_SAP_CARD_REMOVED;
        } else if (req->state == TRUE && status == BT_SAP_POWER_ON) {
            RLOGD("[BTSAP] requestBtSapPower on but card was already power on");
            rsp.response = RIL_SIM_SAP_POWER_RSP_Response_RIL_E_SIM_ALREADY_POWERED_ON;
        } else if (req->state == FALSE && status == BT_SAP_POWER_OFF) {
            RLOGD("[BTSAP] requestBtSapPower off but card was already power off");
            rsp.response = RIL_SIM_SAP_POWER_RSP_Response_RIL_E_SIM_ALREADY_POWERED_OFF;
        } else {
            // Send disconnect request
            RIL_SIM_SAP_POWER_REQ *local_req =
                (RIL_SIM_SAP_POWER_REQ*)calloc(1, sizeof(RIL_SIM_SAP_POWER_REQ));
            assert(local_req != NULL);
            memcpy(local_req, req, sizeof(RIL_SIM_SAP_POWER_REQ));
            LocalBtSapMsgHeader *localMsgHeader = allocateLocalBtSapMsgHeader(local_req, t, rid);
            RIL_requestProxyTimedCallback (requestLocalBtSapPower, localMsgHeader, NULL,
                    p_cctx->id, "requestLocalBtSapPower");
            break;
        }

        // FIXME: Send response immediately
        //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
        sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_POWER, &rsp);
    } while(0);

    free(req);

    RLOGD("[BTSAP] requestBtSapPower end");
}

static void requestLocalBtSapPower(void *param) {
    LocalBtSapMsgHeader *localMsgHdr = (LocalBtSapMsgHeader*)param;
    RIL_Token t = localMsgHdr->t;
    RIL_SOCKET_ID rid = localMsgHdr->socket_id;
    RIL_SIM_SAP_POWER_REQ *req = localMsgHdr->param;
    RIL_SIM_SAP_POWER_RSP rsp;
    ATResponse *p_response = NULL;
    int err;
    int type = 0;
    char* cmd = NULL;
    char *line = NULL, *pAtr = NULL;
    RILChannelCtx *p_cctx = getChannelCtxbyProxy();
    pb_bytes_array_t *atrResponse = NULL;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    RLOGD("[BTSAP] requestLocalBtSapPower start, (%d,%d)", rid, req->state);
    memset(&rsp, 0, sizeof(RIL_SIM_SAP_POWER_RSP));
    if (req->state == TRUE) {
        asprintf(&cmd, "AT+EBTSAP=2,%d", mBtSapCurrentProtocol[rid]);
        err = at_send_command_singleline(cmd, "+EBTSAP:", &p_response, p_cctx);
        free(cmd);
    } else {
        err = at_send_command("AT+EBTSAP=3", &p_response, p_cctx);
    }

    if (err < 0 || NULL == p_response) {
        rsp.response = RIL_SIM_SAP_POWER_RSP_Response_RIL_E_GENERIC_FAILURE;
        goto format_error;
    }

    if (p_response->success == 0) {
        RLOGE("[BTSAP] CME ERROR = %d", at_get_cme_error(p_response));
        rsp.response = RIL_SIM_SAP_POWER_RSP_Response_RIL_E_GENERIC_FAILURE;
        switch (at_get_cme_error(p_response)) {
            case CME_BT_SAP_CARD_REMOVED:
            case CME_SIM_NOT_INSERTED:
                ret = RIL_E_BT_SAP_CARD_REMOVED;
                rsp.response = RIL_SIM_SAP_POWER_RSP_Response_RIL_E_SIM_ABSENT;
                break;
            default:
                break;
        }
        goto format_error;
    } else {
        if (req->state == TRUE) {
            line = p_response->p_intermediates->line;

            err = at_tok_start(&line);
            if (err < 0) goto format_error;

            err = at_tok_nextint(&line, &type);
            if (err < 0) goto  format_error;

            mBtSapCurrentProtocol[rid] = type;

            err = at_tok_nextstr(&line, &pAtr);
            if (err < 0) goto  format_error;

            int atrLen = 0;
            if (pAtr != NULL) {
                // Update ATR
                if (mBtSapAtr[rid] != NULL) {
                    free(mBtSapAtr[rid]);
                }
                atrLen = strlen(pAtr);
                mBtSapAtr[rid] = (char*)calloc(1, (atrLen+1)*sizeof(char));
                if (NULL == mBtSapAtr[rid]){
                    RLOGE("[BTSAP] mBtSapAtr alloc OOM.");
                    ret = RIL_E_NO_MEMORY;
                    goto format_error;
                }
                memcpy(mBtSapAtr[rid], pAtr, atrLen);
            }
            RLOGD("[BTSAP] requestLocalBtSapPower, cur_type: %d, size: %d",
                    mBtSapCurrentProtocol[rid], atrLen);
            mBtSapStatus[rid] = BT_SAP_POWER_ON;
        } else {
            mBtSapStatus[rid] = BT_SAP_POWER_OFF;
        }
        RLOGD("[BTSAP] requestLocalBtSapPower, success! status: %d", mBtSapStatus[rid]);
        ret = RIL_E_SUCCESS;
        rsp.response = RIL_SIM_SAP_POWER_RSP_Response_RIL_E_SUCCESS;
    }

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_POWER, &rsp);

    at_response_free(p_response);
    releaseLocalBtSapMsgHeader(localMsgHdr);
    RLOGD("[BTSAP] requestLocalBtSapPower end");
    return;
format_error:
    RLOGE("[BTSAP] Fail to Set Power");

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_POWER, &rsp);

    at_response_free(p_response);
    releaseLocalBtSapMsgHeader(localMsgHdr);
    RLOGD("[BTSAP] requestLocalBtSapPower end");
}

static void requestBtSapResetSim(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    RIL_SIM_SAP_RESET_SIM_REQ *req = NULL;
    RIL_SIM_SAP_RESET_SIM_RSP rsp;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    BtSapStatus status = -1;
    RILChannelCtx *p_cctx = getRILChannelCtxFromToken(t);

    RLOGD("[BTSAP] requestBtSapResetSim start, (%d)", rid);
    req = (RIL_SIM_SAP_RESET_SIM_REQ*)calloc(1, sizeof(RIL_SIM_SAP_RESET_SIM_REQ));
    assert(req != NULL);
    decodeBtSapPayload(MsgId_RIL_SIM_SAP_RESET_SIM, data, datalen, req);

    status = queryBtSapStatus(rid);
    do {
        if (!isSimInserted(rid)) {
            RLOGD("[BTSAP] requestBtSapResetSim but card was removed");
            rsp.response = RIL_SIM_SAP_RESET_SIM_RSP_Response_RIL_E_SIM_ABSENT;
        } else if (status == BT_SAP_POWER_OFF) {
            RLOGD("[BTSAP] requestBtSapResetSim off but card was already power off");
            rsp.response = RIL_SIM_SAP_RESET_SIM_RSP_Response_RIL_E_SIM_ALREADY_POWERED_OFF;
        } else {
            // Send disconnect request
            RIL_SIM_SAP_RESET_SIM_REQ *local_req =
                (RIL_SIM_SAP_RESET_SIM_REQ*)calloc(1, sizeof(RIL_SIM_SAP_RESET_SIM_REQ));
            assert(local_req != NULL);
            memcpy(local_req, req, sizeof(RIL_SIM_SAP_RESET_SIM_REQ));
            LocalBtSapMsgHeader *localMsgHeader = allocateLocalBtSapMsgHeader(local_req, t, rid);
            RIL_requestProxyTimedCallback (requestLocalBtSapResetSim, localMsgHeader, NULL,
                    p_cctx->id, "requestLocalBtSapResetSim");
            break;
        }

        // FIXME: Send response immediately
        //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
        sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_RESET_SIM, &rsp);
    } while(0);

    free(req);

    RLOGD("[BTSAP] requestBtSapResetSim end");
}

static void requestLocalBtSapResetSim(void *param) {
    LocalBtSapMsgHeader *localMsgHdr = (LocalBtSapMsgHeader*)param;
    RIL_Token t = localMsgHdr->t;
    RIL_SOCKET_ID rid = localMsgHdr->socket_id;
    RIL_SIM_SAP_RESET_SIM_REQ *req = localMsgHdr->param;
    RIL_SIM_SAP_RESET_SIM_RSP rsp;
    ATResponse *p_response = NULL;
    int err;
    int type = 0;
    char* cmd = NULL;
    char *line = NULL, *pAtr = NULL;
    pb_bytes_array_t *atrResponse = NULL;
    RILChannelCtx *p_cctx = getChannelCtxbyProxy();
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    RLOGD("[BTSAP] requestLocalBtSapResetSim start, (%d)", rid);
    memset(&rsp, 0, sizeof(RIL_SIM_SAP_RESET_SIM_RSP));
    asprintf(&cmd, "AT+EBTSAP=4,%d", mBtSapCurrentProtocol[rid]);
    err = at_send_command_singleline(cmd, "+EBTSAP:", &p_response, p_cctx);
    free(cmd);

    if (err < 0 || NULL == p_response) {
        rsp.response = RIL_SIM_SAP_RESET_SIM_RSP_Response_RIL_E_GENERIC_FAILURE;
        goto format_error;
    }

    if (p_response->success == 0) {
        RLOGE("[BTSAP] CME ERROR = %d", at_get_cme_error(p_response));
        rsp.response = RIL_SIM_SAP_RESET_SIM_RSP_Response_RIL_E_GENERIC_FAILURE;
        switch (at_get_cme_error(p_response)) {
            case CME_BT_SAP_CARD_REMOVED:
            case CME_SIM_NOT_INSERTED:
                ret = RIL_E_BT_SAP_CARD_REMOVED;
                rsp.response = RIL_SIM_SAP_RESET_SIM_RSP_Response_RIL_E_SIM_ABSENT;
                break;
            default:
                break;
        }
        goto format_error;
    } else {
        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if (err < 0) goto format_error;

        err = at_tok_nextint(&line, &type);
        if (err < 0) goto  format_error;

        mBtSapCurrentProtocol[rid] = type;

        err = at_tok_nextstr(&line, &pAtr);
        if (err < 0) goto  format_error;

        int atrLen = 0;

        if (pAtr != NULL) {
            // Update ATR
            if (mBtSapAtr[rid] != NULL) {
                free(mBtSapAtr[rid]);
            }
            atrLen = strlen(pAtr);
            mBtSapAtr[rid] = (char*)calloc(1, (atrLen+1)*sizeof(char));
            if (NULL == mBtSapAtr[rid]){
                RLOGE("[BTSAP] mBtSapAtr alloc OOM.");
                ret = RIL_E_NO_MEMORY;
                goto format_error;
            }
            memcpy(mBtSapAtr[rid], pAtr, atrLen);
        }

        RLOGD("[BTSAP] requestLocalBtSapResetSim, cur_type: %d, size: %d",
                mBtSapCurrentProtocol[rid], atrLen);
        ret = RIL_E_SUCCESS;
        rsp.response = RIL_SIM_SAP_RESET_SIM_RSP_Response_RIL_E_SUCCESS;
    }

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_RESET_SIM, &rsp);

    at_response_free(p_response);
    releaseLocalBtSapMsgHeader(localMsgHdr);
    RLOGD("[BTSAP] requestLocalBtSapResetSim end");
    return;
format_error:
    RLOGE("[BTSAP] Fail to Reset SIM");

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_RESET_SIM, &rsp);

    at_response_free(p_response);
    releaseLocalBtSapMsgHeader(localMsgHdr);
    RLOGD("[BTSAP] requestLocalBtSapResetSim end");
}

static void requestBtSapSetTransferProtocol(void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID rid) {
    RIL_SIM_SAP_ERROR_RSP rsp;
    RIL_Errno ret = RIL_E_BT_SAP_UNDEFINED;

    RIL_SIM_UNUSED_PARM(data);
    RIL_SIM_UNUSED_PARM(datalen);

    RLOGD("[BTSAP] requestBtSapSetTransferProtocol start, (%d)", rid);
    rsp.dummy_field = 1;

    // Send response
    //RIL_SAP_onRequestComplete(t, ret, &rsp, sizeof(rsp));
    sendBtSapResponseComplete(t, ret, MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL, &rsp);
}

bool isBtSapConnectionSetup(RIL_SOCKET_ID rid) {
    bool result = ((queryBtSapStatus(rid) == BT_SAP_CONNECTION_SETUP)? TRUE : FALSE);
    RLOGD("[BTSAP] isBtSapConnectionSetup %d", result);
    return result;
}

void notifyBtSapStatusInd(RIL_SIM_SAP_STATUS_IND_Status message, RIL_SOCKET_ID rid) {
    int msgId = MsgId_RIL_SIM_SAP_STATUS;
    RIL_SIM_SAP_STATUS_IND unsolMsg;
    size_t encoded_size = 0;
    size_t buffer_size = 0;
    pb_ostream_t ostream;
    bool success = false;
    ssize_t written_bytes;

    RLOGD("[BTSAP] notifyBtSapStatusInd, (%d, %d)", rid, message);
    unsolMsg.statusChange = message;

    if (message == RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_CARD_RESET &&
        queryBtSapStatus(rid) == BT_SAP_CONNECTION_SETUP) {
        mBtSapStatus[rid] = BT_SAP_ONGOING_CONNECTION;
    }

    // send unsolicited message STATUS_IND
    if ((success = pb_get_encoded_size(&encoded_size, RIL_SIM_SAP_STATUS_IND_fields,
            &unsolMsg)) && encoded_size <= INT32_MAX) {
        //buffer_size = encoded_size + sizeof(uint32_t);
        buffer_size = encoded_size;
        uint8_t buffer[buffer_size];
        //written_size = htonl((uint32_t) encoded_size);
        ostream = pb_ostream_from_buffer(buffer, buffer_size);
        //pb_write(&ostream, (uint8_t *)&written_size, sizeof(written_size));
        success = pb_encode(&ostream, RIL_SIM_SAP_STATUS_IND_fields, &unsolMsg);

        if(success) {
            RLOGD("[BTSAP] notifyBtSapStatusInd, Size: %zu (0x%zx)", encoded_size, encoded_size);
            // Send response
            //RIL_SAP_onUnsolicitedResponse(MsgId_RIL_SIM_SAP_STATUS, buffer, buffer_size, rid);
            RIL_SAP_onUnsolicitedResponse(MsgId_RIL_SIM_SAP_STATUS, buffer, buffer_size, rid);
        } else {
            RLOGE("[BTSAP] notifyBtSapStatusInd, Encode failed!");
        }
    } else {
        RLOGE("Not sending response type %d: encoded_size: %zu. encoded size result: %d",
        msgId, encoded_size, success);
    }

}

static void sendBtSapResponseComplete(RIL_Token t, RIL_Errno ret, MsgId msgId,
        void *data) {
    const pb_field_t *fields = NULL;
    size_t encoded_size = 0;
    size_t buffer_size = 0;
    pb_ostream_t ostream;
    bool success = false;
    ssize_t written_bytes;
    int i = 0;

    RLOGD("[BTSAP] sendBtSapResponseComplete, start (%s)", btSapMsgIdToString(msgId));

    switch (msgId) {
        case MsgId_RIL_SIM_SAP_CONNECT:
            fields = RIL_SIM_SAP_CONNECT_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_DISCONNECT:
            fields = RIL_SIM_SAP_DISCONNECT_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_APDU:
            fields = RIL_SIM_SAP_APDU_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_TRANSFER_ATR:
            fields = RIL_SIM_SAP_TRANSFER_ATR_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_POWER:
            fields = RIL_SIM_SAP_POWER_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_RESET_SIM:
            fields = RIL_SIM_SAP_RESET_SIM_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL:
            fields = RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP_fields;
            break;
        case MsgId_RIL_SIM_SAP_ERROR_RESP:
            fields = RIL_SIM_SAP_ERROR_RSP_fields;
            break;
        default:
            RLOGE("[BTSAP] sendBtSapResponseComplete, MsgId is mistake!");
            return;
    }

    if ((success = pb_get_encoded_size(&encoded_size, fields, data)) &&
            encoded_size <= INT32_MAX) {
        //buffer_size = encoded_size + sizeof(uint32_t);
        buffer_size = encoded_size;
        uint8_t buffer[buffer_size];
        //written_size = htonl((uint32_t) encoded_size);
        ostream = pb_ostream_from_buffer(buffer, buffer_size);
        //pb_write(&ostream, (uint8_t *)&written_size, sizeof(written_size));
        success = pb_encode(&ostream, fields, data);

        if(success) {
            RLOGD("[BTSAP] sendBtSapResponseComplete, Size: %zu (0x%zx)", encoded_size, encoded_size);
            // Send response
            //RIL_SAP_onRequestComplete(t, ret, buffer, buffer_size);
            RIL_SAP_onRequestComplete(t, ret, buffer, buffer_size);
        } else {
            RLOGE("[BTSAP] sendBtSapResponseComplete, Encode failed!");
        }
    } else {
        RLOGE("Not sending response type %d: encoded_size: %zu. encoded size result: %d",
        msgId, encoded_size, success);
    }
}

static char* btSapMsgIdToString(MsgId msgId) {
    switch (msgId) {
        case MsgId_RIL_SIM_SAP_CONNECT:
            return "BT_SAP_CONNECT";
        case MsgId_RIL_SIM_SAP_DISCONNECT:
            return "BT_SAP_DISCONNECT";
        case MsgId_RIL_SIM_SAP_APDU:
            return "BT_SAP_TRANSFER_APDU";
        case MsgId_RIL_SIM_SAP_TRANSFER_ATR:
            return "BT_SAP_TRANSFER_ATR";
        case MsgId_RIL_SIM_SAP_POWER:
            return "BT_SAP_POWER";
        case MsgId_RIL_SIM_SAP_RESET_SIM:
            return "BT_SAP_RESET_SIM";
        case MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL:
            return "BT_SAP_TRANSFER_PROTOCOL";
        case MsgId_RIL_SIM_SAP_ERROR_RESP:
            return "BT_SAP_ERROR_RESP";
        case MsgId_RIL_SIM_SAP_STATUS:
            return "BT_SAP_STATUS_IND";
        default:
            return "BT_SAP_UNKNOWN_MSG_ID";
    }
}
// MTK-END

void resetAidInfo(RIL_SOCKET_ID rid) {
    int i = 0;

    for (i = 0; i < AID_MAX; i++) {
        memset(&aid_info_array[rid][i], 0, sizeof(AidInfo));
    }
    // MTK-START: ISIM
    memset(&isim_session_info[rid], 0, sizeof(SessionInfo));
    // MTK-END
    read_ef_dir_complete[rid] = 0;
}

// MTK-START: ISIM
SessionInfo* getIsimSessionInfo(RIL_SOCKET_ID rid) {
    RLOGD("getIsimSession, rid %d, appId %d, session %d", rid, isim_session_info[rid].appId,
            isim_session_info[rid].session);
    return &isim_session_info[rid];
}
// MTK-END

AidInfo* getAidInfo(RIL_SOCKET_ID rid, AidIndex index) {
    RLOGD("getAidInfo, rid %d, aid_len %d, applabel_len %d", rid, aid_info_array[rid][index].aid_len,
            aid_info_array[rid][index].app_label_len);
    return &aid_info_array[rid][index];
}

int queryAppType(char* pAid) {
    int match = -1, i = 0;
    for (i = 0; i < AID_MAX; i++) {
        if (pAid != NULL && (memcmp(aid_support_list[i], pAid, AID_PREFIX_LEN) == 0)) {
            match = i;
        }
    }
    if (match == -1) {
        RLOGI("matchAppAid, not match");
    }
    return match;
}

bool bIsTc1()
{
    static int siTc1 = -1;

    if (siTc1 < 0)
    {
        char cTc1[PROPERTY_VALUE_MAX] = { 0 };

        property_get("ro.vendor.mtk_tc1_feature", cTc1, "0");
        siTc1 = atoi(cTc1);
    }

    return ((siTc1 > 0) ? true : false);
}

void setSimInsertedStatus(RIL_SOCKET_ID rid, int isInserted){
    //int simId3G = RIL_get3GSIM();
    int pivot = 1;
    int pivotSim = rid;

    int notPivotSimStatus;

    if(isInserted == 1) {
        // 3G switch
        pivotSim = pivot << rid;
        sim_inserted_status = sim_inserted_status | pivotSim;
    } else {
        pivotSim = pivot << rid;
        notPivotSimStatus = ~pivotSim;
        sim_inserted_status =  (sim_inserted_status & notPivotSimStatus );
    }
}

void requestIccId(void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err;
    const char**    strings = (const char**)data;
    char *line;
    char *iccId;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    RIL_SIM_UNUSED_PARM(datalen);

    err = at_send_command_singleline("AT+ICCID?", "+ICCID:", &p_response, SIM_CHANNEL_CTX);

    if (err < 0 || NULL == p_response) {
        RLOGE("requestIccId Fail");
        goto error;
    }

    if (p_response->success == 0) {
        RLOGD("p_response = %d /n", at_get_cme_error(p_response) );
        switch (at_get_cme_error(p_response)) {
            case CME_SUCCESS:
                ret = RIL_E_GENERIC_FAILURE;
                break;
            case CME_UNKNOWN:
                RLOGE("p_response: CME_UNKNOWN");
                break;
            default:
                break;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }

    if (ret != RIL_E_SUCCESS) {
        RLOGE("requestIccId: Error or no SIM inserted!");
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start (&line);
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &iccId);
    if (err < 0) goto error;

    RLOGD("requestIccId: strlen of response is %zu", strlen(iccId) );
    // Convert all the lower case characters in iccid to upper case
    stringToUpper(iccId);
    property_set(PROPERTY_ICCID_SIM[rid], iccId);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, iccId, strlen(iccId));

    at_response_free(p_response);
    return;
error:
    property_set(PROPERTY_ICCID_SIM[rid], "N/A");
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

// NFC SEEK start
void requestSIM_OpenChannel(void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err;
    char *cmd = NULL;
    char *line;
    int channel;

    RIL_SIM_UNUSED_PARM(datalen);

    asprintf(&cmd, "AT+CCHO=\"%s\"", (char *)data);
    err = at_send_command_numeric(cmd, &p_response, SIM_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0) {
        err = RIL_E_GENERIC_FAILURE;
        if (p_response != NULL && p_response->finalResponse != NULL) {
            if (!strcmp(p_response->finalResponse,
                    "+CME ERROR: MEMORY FULL"))
                err = RIL_E_MISSING_RESOURCE;
            if (!strcmp(p_response->finalResponse,
                    "+CME ERROR: NOT FOUND"))
                err = RIL_E_NO_SUCH_ELEMENT;
        }
        goto error;
    }

    line = p_response->p_intermediates->line;
    channel = atoi(p_response->p_intermediates->line);
    if(channel < 0 || channel > 19){
        LOGD("requestSIM_OpenChannel only support 20 channels at the same time, channel = %d", channel);
        err = RIL_E_GENERIC_FAILURE;
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &channel, sizeof(int));
    at_response_free(p_response);
    free(cmd);

    return;
error:
    RIL_onRequestComplete(t, err, NULL, 0);
    at_response_free(p_response);
    free(cmd);
}

void requestSIM_CloseChannel(void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err;
    char *cmd = NULL;
    char *line;
    int channel;

    RIL_SIM_UNUSED_PARM(datalen);

    asprintf(&cmd, "AT+CCHC=%d", *(int *)data);
    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);

    if (err < 0 || p_response->success == 0) {
        err = RIL_E_GENERIC_FAILURE;
        if (p_response != NULL && p_response->finalResponse != NULL) {
            if (!strcmp(p_response->finalResponse,
                    "+CME ERROR: INCORRECT PARAMETERS"))
                err = RIL_E_GENERIC_FAILURE;
            if (*(int *)data == 0) {
                err = RIL_E_INVALID_ARGUMENTS;
            }
        }
        goto error;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    free(cmd);

    return;
error:
    RIL_onRequestComplete(t, err, NULL, 0);
    at_response_free(p_response);
    free(cmd);
}

void requestSIM_TransmitBasic(void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    RIL_SIM_IO_Response sr;
    int err;
    char *cmd = NULL;
    RIL_SIM_APDU *p_args;
    char *line;
    int len;
    RILChannelCtx *p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    RIL_SIM_UNUSED_PARM(datalen);

    memset(&sr, 0, sizeof(sr));

    p_args = (RIL_SIM_APDU *)data;

    if ((p_args->data == NULL) || (strlen(p_args->data) == 0)) {
        LOGD("requestSIM_TransmitBasic p3 = %d", p_args->p3);

        if (p_args->p3 < 0) {
            asprintf(&cmd, "AT+CGLA=0,%d,\"%02x%02x%02x%02x\"",
                    8, p_args->cla, p_args->instruction,
                    p_args->p1, p_args->p2);
        } else {
            asprintf(&cmd, "AT+CGLA=0,%d,\"%02x%02x%02x%02x%02x\"",
                    10, p_args->cla, p_args->instruction,
                    p_args->p1, p_args->p2, p_args->p3);
        }
    } else {
        asprintf(&cmd, "AT+CGLA=0,%zu,\"%02x%02x%02x%02x%02x%s\"",
                10 + strlen(p_args->data), p_args->cla, p_args->instruction,
                p_args->p1, p_args->p2, p_args->p3,
                p_args->data);
    }
    err = at_send_command_singleline(cmd, "+CGLA:", &p_response, p_channel);

    if (err < 0 || p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &len);
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &(sr.simResponse));
    if (err < 0) goto error;

    sscanf(&(sr.simResponse[len - 4]), "%02x%02x", &(sr.sw1), &(sr.sw2));
    sr.simResponse[len - 4] = '\0';
    LOGD("requestSIM_TransmitBasic len = %d %02x, %02x", len, sr.sw1, sr.sw2);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &sr, sizeof(sr));
    goto done;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);

done:
    at_response_free(p_response);
    free(cmd);
}

/**
 * Returns a copy of the given CLA byte where the channel number bits are
 * set as specified by the given channel number See GlobalPlatform Card
 * Specification 2.2.0.7: 11.1.4 Class Byte Coding.
 *
 * @param cla the CLA byte. Won't be modified
 * @param channelNumber within [0..3] (for first interindustry class byte
 *            coding) or [4..19] (for further interindustry class byte
 *            coding)
 * @return the CLA byte with set channel number bits. The seventh bit
 *         indicating the used coding (first/further interindustry class
 *         byte coding) might be modified
 */
 int setChannelToClassByte(int cla, int channelNumber) {
    if (channelNumber < 4) {
        // b7 = 0 indicates the first interindustry class byte coding
        cla = ((cla & 0xBC) | channelNumber);

    } else if (channelNumber < 20) {
        // b7 = 1 indicates the further interindustry class byte coding
        bool isSM = (cla & 0x0C) != 0;
        cla = ((cla & 0xB0) | 0x40 | (channelNumber - 4));
        if (isSM) {
            cla |= 0x20;
        }
    }
    return cla;
}

void requestSIM_TransmitChannel(void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    RIL_SIM_IO_Response sr;
    int err;
    char *cmd = NULL;
    RIL_SIM_APDU *p_args;
    char *line;
    int len;
    RILChannelCtx *p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    RIL_SIM_UNUSED_PARM(datalen);

    memset(&sr, 0, sizeof(sr));

    p_args = (RIL_SIM_APDU *)data;

    if (bIsTc1() == true) {
        p_args->cla = setChannelToClassByte(p_args->cla, p_args->sessionid);
        LOGD("requestSIM_TransmitChannel cla = %02x", p_args->cla);
    }

    if ((p_args->data == NULL) || (strlen(p_args->data) == 0)) {
       LOGD("requestSIM_TransmitChannel p3 = %d", p_args->p3);
       if (p_args->p3 < 0) {
           asprintf(&cmd, "AT+CGLA=%d,%d,\"%02x%02x%02x%02x\"",
                    p_args->sessionid,
                    8, p_args->cla, p_args->instruction,
                    p_args->p1, p_args->p2);
       } else {
           asprintf(&cmd, "AT+CGLA=%d,%d,\"%02x%02x%02x%02x%02x\"",
                    p_args->sessionid,
                    10, p_args->cla, p_args->instruction,
                    p_args->p1, p_args->p2, p_args->p3);
       }
    } else {
        asprintf(&cmd, "AT+CGLA=%d,%zu,\"%02x%02x%02x%02x%02x%s\"",
                p_args->sessionid,
                10 + strlen(p_args->data), p_args->cla, p_args->instruction,
                p_args->p1, p_args->p2, p_args->p3,
                p_args->data);
    }
    err = at_send_command_singleline(cmd, "+CGLA:", &p_response, p_channel);

    if (err < 0 || p_response->success == 0) {
        err = RIL_E_GENERIC_FAILURE;
        if (p_response != NULL && p_response->finalResponse != NULL) {
            if (!strcmp(p_response->finalResponse,
                    "+CME ERROR: INCORRECT PARAMETERS"))
                err = RIL_E_GENERIC_FAILURE;
        }
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = RIL_E_GENERIC_FAILURE;
    if (at_tok_start(&line) < 0) goto error;
    if (at_tok_nextint(&line, &len) < 0) goto error;
    if (at_tok_nextstr(&line, &(sr.simResponse)) < 0) goto error;

    sscanf(&(sr.simResponse[len - 4]), "%02x%02x", &(sr.sw1), &(sr.sw2));
    sr.simResponse[len - 4] = '\0';
    LOGD("requestSIM_TransmitChannel len = %d %02x, %02x", len, sr.sw1, sr.sw2);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &sr, sizeof(sr));
    goto done;

error:
    RIL_onRequestComplete(t, err, NULL, 0);

done:
    at_response_free(p_response);
    free(cmd);
}


int requrstSimChannelAccess(RILChannelCtx *p_channel, int sessionid, char* senddata, RIL_SIM_IO_Response* output){
    ATResponse *p_response = NULL;
    char *cmd = NULL;
    char *line;
    int len = 0;
    int err = 0;

    RLOGD("requrstSimChannelAccess sessionid  = %d, senddata = %s", sessionid, senddata);

    if(senddata == NULL || p_channel == NULL){
        err = AT_ERROR_GENERIC;
        goto error;
    }

    len = strlen(senddata);

    asprintf(&cmd, "AT+CGLA=%d,%d,\"%s\"", sessionid, len, senddata);
    err = at_send_command_singleline(cmd, "+CGLA:", &p_response, p_channel);

    line = p_response->p_intermediates->line;
    free(cmd);

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &len);
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &(output->simResponse));
    if (err < 0) goto error;

    sscanf(&(output->simResponse[len - 4]), "%02x%02x", &(output->sw1), &(output->sw2));
    output->simResponse[len - 4] = '\0';

    at_response_free(p_response);
    return err;

error:
    at_response_free(p_response);

    RLOGE("requrstSimChannelAccess error");
    return err;

}

void requestSIM_GetATR(void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err;
    const char**    strings = (const char**)data;
    char *line, *tmp;
    char *atr;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    RIL_SIM_UNUSED_PARM(datalen);

    err = at_send_command_singleline("AT+ESIMINFO=0", "+ESIMINFO:", &p_response, SIM_CHANNEL_CTX);

    if (err < 0 || NULL == p_response) {
        LOGE("requestSIM_GetATR Fail");
        goto error;
    }

    if (p_response->success == 0) {
        goto error;
    }

    // +ESIMINFO:<mode>, <data> => if<mode>=0,<data>=ATR Hex String
    line = p_response->p_intermediates->line;
    err = at_tok_start (&line);
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &tmp);
    if(err < 0) goto error;

    err = at_tok_nextstr(&line, &atr);
    if (err < 0) goto error;

    LOGD("requestSIM_GetATR: strlen of response is %zu", strlen(atr) );
    RIL_onRequestComplete(t, RIL_E_SUCCESS, atr, strlen(atr));


    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

// MTK-START: SIM ME LOCK
void queryNetworkLock(void *data, size_t datalen, RIL_Token t)
{
    ATResponse*     p_response = NULL;
    int             err = -1;
    char*           cmd = NULL;
    RIL_Errno       ret = RIL_E_GENERIC_FAILURE;
    char            *line;
    SimLockInfo     result;
    char            *tmp;
    int             i = 0;
    int*            n = (int *) data;
    RIL_SimMeLockCatInfo     lockstate;

    RIL_SIM_UNUSED_PARM(datalen);

    err = at_send_command_singleline("AT+ESMLCK", "+ESMLCK:", &p_response, SIM_CHANNEL_CTX);
    if (err < 0 || p_response->success == 0) {
            goto error;
        }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if(err < 0) goto error;

    for (i=0; i<7; i++)
    {
        err = at_tok_char(&line);
        if(err < 0) goto error;

        err = at_tok_nextint(&line, &(result.catagory[i].catagory));
        if(err < 0) goto error;

        err = at_tok_nextint(&line, &(result.catagory[i].state));
        if(err < 0) goto error;

        err = at_tok_nextint(&line, &(result.catagory[i].retry_cnt));
        if(err < 0) goto error;

        err = at_tok_nextint(&line, &(result.catagory[i].autolock_cnt));
        if(err < 0) goto error;

        err = at_tok_nextint(&line, &(result.catagory[i].num_set));
        if(err < 0) goto error;

        err = at_tok_nextint(&line, &(result.catagory[i].total_set));
        if(err < 0) goto error;

        err = at_tok_nextint(&line, &(result.catagory[i].key_state));
        if(err < 0) goto error;

    }

    err = at_tok_nextstr(&line, &tmp);
    if(err < 0) goto error;
    strlcpy(result.imsi, tmp, sizeof(result.imsi));

    err = at_tok_nextint(&line, &(result.isgid1));
    if(err < 0) goto error;

    err = at_tok_nextstr(&line, &tmp);
    if(err < 0) goto error;
    strlcpy(result.gid1, tmp, sizeof(result.gid1));

    err = at_tok_nextint(&line, &(result.isgid2));
    if(err < 0) goto error;

    err = at_tok_nextstr(&line, &tmp);
    if(err < 0) goto error;
    strlcpy(result.gid2, tmp, sizeof(result.gid2));

    err = at_tok_nextint(&line, &(result.mnclength));
    if(err < 0) goto error;


    at_response_free(p_response);

    i = n[0];
    lockstate.catagory= result.catagory[i].catagory;
    lockstate.state = result.catagory[i].state;
    lockstate.retry_cnt= result.catagory[i].retry_cnt;
    lockstate.autolock_cnt = result.catagory[i].autolock_cnt;
    lockstate.num_set = result.catagory[i].num_set;
    lockstate.total_set = result.catagory[i].total_set;
    lockstate.key_state = result.catagory[i].key_state;

    RIL_onRequestComplete(t, RIL_E_SUCCESS, &lockstate, sizeof(lockstate));
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

SIM_Status getSIMLockStatus(RIL_SOCKET_ID rid)
{
    ATResponse *p_response = NULL;
    int err;
    int ret;
    char *cpinLine;
    char *cpinResult;
    int isUsim = isUsimDetect[rid];
    if (getRadioState(rid) == RADIO_STATE_OFF || getRadioState(rid) == RADIO_STATE_UNAVAILABLE) {
        ret = SIM_NOT_READY;
        goto done;
    }
    RLOGD("UICC Type: %d", isUsim);
    err = at_send_command_singleline("AT+CPIN?", "+CPIN:", &p_response, getChannelCtxbyProxy());

    if (err != 0) {
        ret = SIM_NOT_READY;
        goto done;
    }
    if (p_response->success == 0) {
        switch (at_get_cme_error(p_response)) {
            case CME_SIM_BUSY:
                RLOGD("getSIMStatus: CME_SIM_BUSY");
                ret = SIM_BUSY;
                goto done;
                break;
            case CME_SIM_NOT_INSERTED:
            case CME_SIM_WRONG:
            case CME_SIM_FAILURE:
                ret = SIM_ABSENT;
                goto done;

            default:
                ret = SIM_NOT_READY;
                goto done;
        }
    }
    cpinLine = p_response->p_intermediates->line;
    err = at_tok_start (&cpinLine);

    if (err < 0) {
        ret = SIM_NOT_READY;
        goto done;
    }
    err = at_tok_nextstr(&cpinLine, &cpinResult);

    if (err < 0) {
        ret = SIM_NOT_READY;
        goto done;
    }
    if (0 == strcmp (cpinResult, "SIM PIN")) {
        if (isUsim == 1) {
            ret = USIM_PIN;
        } else {
        ret = SIM_PIN;
        }
        goto done;
    } else if (0 == strcmp (cpinResult, "SIM PUK")) {
        if (isUsim == 1) {
            ret = USIM_PUK;
        } else {
        ret = SIM_PUK;
        }
        goto done;
    } else if (0 == strcmp (cpinResult, "PH-NET PIN") ||
               0 == strcmp (cpinResult, "PH-NET PUK")) {
        ret = SIM_NP;
        goto done;
    }else if ( 0 == strcmp (cpinResult, "PH-NETSUB PIN") ||
               0 == strcmp (cpinResult, "PH-NETSUB PUK")) {
        ret = SIM_NSP;
        goto done;
    }else if ( 0 == strcmp (cpinResult, "PH-SP PIN") ||
               0 == strcmp (cpinResult, "PH-SP PUK")) {
        ret = SIM_SP;
        goto done;
    }else if ( 0 == strcmp (cpinResult, "PH-CORP PIN") ||
               0 == strcmp (cpinResult, "PH-CORP PUK")) {
        ret = SIM_CP;
        goto done;
    }else if ( 0 == strcmp (cpinResult, "PH-FSIM PIN") ||
               0 == strcmp (cpinResult, "PH-FSIM PUK") ) {
        ret = SIM_SIMP;
        goto done;
    } else if (0 != strcmp (cpinResult, "READY"))  {
        ret = SIM_ABSENT;
        goto done;
    }
    at_response_free(p_response);
    p_response = NULL;
    cpinResult = NULL;
    if (isUsim == 1) {
        ret = USIM_READY;
    } else {
    ret = SIM_READY;
    }
done:
        RLOGE("ret = %d", ret);
    at_response_free(p_response);
    return ret;
}

void simNetworkLock(void *data, size_t datalen, RIL_Token t)
{
    ATResponse*     p_response = NULL;
    int             err = -1;
    char*           cmd = NULL;
    char**          strings = (char**)data;;
    char*           line;
    RIL_Errno       ret = RIL_E_GENERIC_FAILURE;
    int             resLength = 0;
    int*            p_res = NULL;
    RIL_SOCKET_ID   rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    char* key = NULL;
    char* imsi = NULL;
    char* gid1 = NULL;
    char* gid2 = NULL;

    if (strings == NULL || strings[0] == NULL ||
            strings[1] == NULL) {
         ret = RIL_E_INVALID_ARGUMENTS;
         RLOGE("simNetworkLock invalid arguments.");
         goto error;
    }
    // strings[0]: cat
    // strings[1]: op
    key = (strings[2] == NULL) ? "" : strings[2];
    imsi = (strings[3] == NULL) ? "" : strings[3];
    gid1 = (strings[4] == NULL) ? "" : strings[4];
    gid2 = (strings[5] == NULL) ? "" : strings[5];

    RLOGI("simNetworkLock: %s,%s,%s,%s,%s,%s datalen=%zu/n",
         strings[0], strings[1], key, imsi, gid1, gid2, datalen);

    if( 0 == strcmp (strings[1],"2")) //add data
    {
        if (0 == strcmp(strings[0],"2")) {
            asprintf(&cmd, "AT+ESMLCK=%s,%s,\"%s\",\"%s\",\"%s\"",
                    strings[0], strings[1], key, imsi, gid1);
        } else if (0 == strcmp(strings[0], "3")) {
            asprintf(&cmd, "AT+ESMLCK=%s,%s,\"%s\",\"%s\",\"%s\",\"%s\"",
                    strings[0], strings[1], key, imsi, gid1, gid2);
        }else {
            asprintf(&cmd, "AT+ESMLCK=%s,%s,\"%s\",\"%s\"",
                    strings[0], strings[1], key, imsi);
        }
    }
    else if (0 == strcmp (strings[1],"3") || //remove data
             0 == strcmp (strings[1],"4")) //disable data
    {
        asprintf(&cmd, "AT+ESMLCK=%s,%s", strings[0], strings[1]);
    }else if (0 == strcmp (strings[1],"0") ) //unlock
    {
        SIM_Status sim_status = getSIMLockStatus(rid);

        if ( (SIM_NP==sim_status && (0==strcmp (strings[0],"0"))) ||
                 (SIM_NSP==sim_status && (0==strcmp (strings[0],"1")))  ||
                 (SIM_SP==sim_status && (0==strcmp (strings[0],"2"))) ||
                 (SIM_CP==sim_status && (0==strcmp (strings[0],"3"))) ||
                 (SIM_SIMP==sim_status && (0==strcmp (strings[0],"4")))
             ) {
                RLOGE("simsatus = %d, category = %s", sim_status, strings[0] );
            asprintf(&cmd, "AT+CPIN=\"%s\"", key);
            err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX );
            free(cmd);
            if (err < 0 || NULL == p_response) {
                RLOGE("simNetworkLock Fail");
                 goto error;
            }

            if (p_response->success == 0) {
                switch (at_get_cme_error(p_response)) {
                case CME_INCORRECT_PASSWORD:
                    goto error;
                    break;
                case CME_SUCCESS:
                    // While p_response->success is 0, the CME_SUCCESS means CME ERROR:0 =>
                    // it is phone failure
                    goto error;
                    break;
                default:
                    at_response_free(p_response);
                    setSimStatusChanged(rid);
                    asprintf(&cmd, "AT+ESMLCK=%s,%s,\"%s\"", strings[0], strings[1], key);
                    break;
                }
            } else {
                at_response_free(p_response);
                setSimStatusChanged(rid);
                //setRadioState(RADIO_STATE_SIM_READY,rid);
                RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0, rid);
                asprintf(&cmd, "AT+ESMLCK=%s,%s,\"%s\"", strings[0], strings[1], key);
            }
        }
        else
        {
            asprintf(&cmd, "AT+ESMLCK=%s,%s,\"%s\"", strings[0], strings[1], key);
        }
    }
    else if (0 == strcmp (strings[1],"1")) //lock
    {
        asprintf(&cmd, "AT+ESMLCK=%s,%s,\"%s\"", strings[0], strings[1], key);
    }

    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);

    if (err < 0 || NULL == p_response) {
         goto error;
    }

    if (p_response->success == 0) {
        RLOGE("p_response = %d /n", at_get_cme_error(p_response) );
        switch (at_get_cme_error(p_response)) {
            case CME_SUCCESS:
                ret = RIL_E_GENERIC_FAILURE;
                break;
            case CME_UNKNOWN:
                break;
            default:
                goto error;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }
    at_response_free(p_response);
    p_response = NULL;
    RIL_onRequestComplete(t, ret, NULL, 0);
    return;

error:
    RLOGE("simNetworkLock Fail");
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}
// MTK-END


static int checkRetryFCI(int sw1, int sw2) {
    int retry = 0;
    if (sw1 != 0x61 && sw1 != 0x91 && (sw1 != 0x63 && sw2 != 0x10) && (sw1 != 0x92 && sw2 != 0x40)
        && sw1 != 0x9F && (sw1 != 0x90 && sw2 != 0x00)) {
        retry = 1;
    }
    RLOGD("checkRetryFCI, sw1 %d sw2 %d retry %d", sw1, sw2, retry);
    return retry;
}

// Hexa To Int
unsigned int htoi(char* szHex) {
    unsigned int hex = 0;
    int nibble;
    while (*szHex) {
        hex <<= 4;

        if (*szHex >= '0' && *szHex <= '9') {
            nibble = *szHex - '0';
        } else if (*szHex >= 'a' && *szHex <= 'f') {
            nibble = *szHex - 'a' + 10;
        } else if (*szHex >= 'A' && *szHex <= 'F') {
            nibble = *szHex - 'A' + 10;
        } else {
            nibble = 0;
        }

        hex |= nibble;

        szHex++;
    }

    return hex;
}

int requestCGLA_OpenChannelWithSw(RIL_SIM_IO_Response *sr, int *len,
                                 int channel, int length, int cla, int ins, int p1, int p2, int p3, char **data, RIL_Token t) {
    ATResponse *p_response = NULL;
    char *cmd = NULL;
    int err;
    char *line;
    char *srResponse;

    srResponse = (char*)calloc(1, sizeof(char) * 515);
    if (NULL == srResponse) {
        RLOGE("requestCGLA_OpenChannelWithSw: alloc OOM");
        goto error;
    }
redo:
    *len = 0;
    memset(srResponse, 0, 515);
    memset(sr, 0, sizeof(*sr));

    if(data == NULL) {
        if(ins == 0xA4 && p2==0x04) {
            asprintf(&cmd, "AT+CGLA=%d,%d,\"%02x%02x%02x%02x%02x00\"",
                            channel, length+2, cla, ins, p1, p2, p3);
        } else {
            asprintf(&cmd, "AT+CGLA=%d,%d,\"%02x%02x%02x%02x%02x\"",
                            channel, length, cla, ins, p1, p2, p3);
        }
    } else {
        if(ins == 0xA4 && p2==0x04) {
            asprintf(&cmd, "AT+CGLA=%d,%d,\"%02x%02x%02x%02x%02x%s00\"",
                            channel, length+2, cla, ins, p1, p2, p3, *data);
        } else {
            asprintf(&cmd, "AT+CGLA=%d,%d,\"%02x%02x%02x%02x%02x%s\"",
                            channel, length, cla, ins, p1, p2, p3, *data);
        }
    }

    err = at_send_command_singleline(cmd, "+CGLA:", &p_response, SIM_CHANNEL_CTX);
    if (err < 0 || p_response->success == 0) {
        RLOGD("requestCGLA_OpenChannelWithSw: send AT-CMD fail");
        err = RIL_E_GENERIC_FAILURE;
        if (p_response != NULL && p_response->finalResponse != NULL) {
            if (!strcmp(p_response->finalResponse, "+CME ERROR: MEMORY FULL")) {
                err = RIL_E_MISSING_RESOURCE;
            }
            if (!strcmp(p_response->finalResponse, "+CME ERROR: NOT FOUND")) {
                err = RIL_E_NO_SUCH_ELEMENT;
            }
        }
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) goto error;
    err = at_tok_nextint(&line, len);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line, &(sr->simResponse));
    if (err < 0) goto error;

    sscanf(&(sr->simResponse[*len - 4]), "%02x%02x", &(sr->sw1), &(sr->sw2));
    sr->simResponse[*len - 4] = '\0';
    strlcpy(srResponse, sr->simResponse, sizeof(char) * 515);
    sr->simResponse = srResponse;

    at_response_free(p_response);
    p_response = NULL;
    free(cmd);
    cmd = NULL;

    if(sr->sw1 == 0x61) { // GET RESPONSE setting p3 = sr.sw2
        // GET RESPONSE for selecting AID if T=0
        // AT+CGLA=<sessionid>,<length>,<command>
        // <length> = 10
        // <command> = 0X C0 00 00 sr.sw2
        // X = channel
        length = 10;
        ins = 0xC0;
        p1 = 0x00;
        p2 = 0x00;
        p3 = sr->sw2;
        data = NULL;
        goto redo;
    } else if(sr->sw1 == 0x6C) { // resend previous command header setting p3 = sr.sw2
        p3 = sr->sw2;
        goto redo;
    } else if((sr->sw1 == 0x6A && sr->sw2 == 0x82) ||
              (sr->sw1 == 0x69 && sr->sw2 == 0x85) ||
              (sr->sw1 == 0x69 && sr->sw2 == 0x99)){
        RLOGE("File not found");
        return RIL_E_NO_SUCH_ELEMENT;
    } else if ((sr->sw1 == 0x6A && sr->sw2 == 0x84) ||
               (sr->sw1 == 0x6A && sr->sw2 == 0x81) ||
               (sr->sw1 == 0x68 && sr->sw2 == 0x81)) {
        RLOGE("Not enough memory space in the file");
        return RIL_E_MISSING_RESOURCE;
    } else if (sr->sw1 != 0x90 && sr->sw1 != 0x91) { // wrong sw1, sw2
        if (ins == 0xA4 && data != NULL && p2 == 0x04) {
            // Retry only for "Select AID"
            if (checkRetryFCI(sr->sw1, sr->sw2) == 1) {
                p2 = 0x00;
                goto redo;
            }
        }
        // GSMA NFC test: We should treat warning code 0x62 or 0x63 as
        // success.
        if (sr->sw1 == 0x62 || sr->sw1 == 0x63) {
            LOGD("requestCGLA_OpenChannelWithSw: 0x62 || 0x63 warning code.");
        } else {
            return RIL_E_GENERIC_FAILURE;
        }
    }

    return RIL_E_SUCCESS;

error:
    sr->sw1 = 0x6f;
    sr->sw2 = 0x00;
    free(srResponse);
    sr->simResponse = NULL;

    at_response_free(p_response);
    free(cmd);
    return RIL_E_GENERIC_FAILURE;
}

int checkUserLoad() {
    int isUserLoad = 0;
    char property_value_emulation[PROPERTY_VALUE_MAX] = { 0 };
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("vendor.ril.emulation.userload", property_value_emulation, "0");
    if(strcmp("1", property_value_emulation) == 0) {
        return 1;
    }
    property_get("ro.build.type", property_value, "");
    isUserLoad = (strcmp("user", property_value) == 0);
    return isUserLoad;
}

void requestSIM_OpenChannelWithSw(void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    char *cmd = NULL;
    RIL_SIM_IO_Response sr;
    int err;
    int channel = 0;
    int len;
    int status[3] = {0};
    int *fullResponse = NULL;
    int i = 0;
    bool isFree = false;
    RIL_OpenChannelParams *openChannelParams = NULL;
    char *pAid = NULL;
    int p2 = 0x00;

    RIL_SIM_UNUSED_PARM(datalen);

    openChannelParams = (RIL_OpenChannelParams *)data;

    if (checkUserLoad() == 1) {
        RLOGD("requestSIM_OpenChannelWithSw entering...");
    } else {
        RLOGD("requestSIM_OpenChannelWithSw entering... %s, %d",
                (char*)(openChannelParams->aidPtr), openChannelParams->p2);
    }
    pAid = openChannelParams->aidPtr;
    if (openChannelParams->p2 > 0) {
        p2 = openChannelParams->p2;
    }

    // If p2 is invalid as TS 102 221 Table 11.2 defined.
    if ((pAid == NULL) && (p2 != 0) && ((p2 & 0x80) > 0 || ((p2 & 0x40) > 0 && (p2 & 0x20) > 0)
            || ((p2 & 0x40) == 0 && (p2 & 0x20) > 0) || (p2 & 0x10) > 0 || (p2 & 0x04) == 0)) {
        LOGI("requestSIM_OpenChannelWithSw, invalid p2");
        sr.sw1 = 0xff;
        sr.sw2 = 0xff;
        sr.simResponse = NULL;
        goto error;
    }

    // Open logical Channel
    // AT+CGLA=<sessionid>,<length>,<command>
    // <sessionid> = 0
    // <command> = 00 70 00 00 01
    err = requestCGLA_OpenChannelWithSw(&sr, &len, 0, 10, 0x00, 0x70, 0x00, 0x00, 0x01, NULL, t);
    LOGD("requestSIM_OpenChannelWithSw: Open logical Channel: err = %d, len = %d %02x, %02x",
            err, len, sr.sw1, sr.sw2);

    // if sw1 == sw2 == 0xff, it means that open logical channel fail
    if(err == RIL_E_SUCCESS) {
        // Support Extended Channel - 4 to 19 (0x40 ~ 0x4F)
        if (sr.simResponse != NULL) {
            channel = htoi(sr.simResponse);
        }
        LOGD("requestSIM_OpenChannelWithSw - channel = %x", channel);
        status[0] = channel;
        if(channel < 1 || channel > 19) {
            channel = 0;
            status[0] = 0;
            sr.sw1 = 0xff;
            sr.sw2 = 0xff;
            goto error;
        }
    } else if (err == RIL_E_NO_SUCH_ELEMENT || err == RIL_E_MISSING_RESOURCE) {
        status[1] = sr.sw1;
        status[2] = sr.sw2;
        goto error;
    } else {
        sr.sw1 = 0xff;
        sr.sw2 = 0xff;
        goto error;
    }

    // Select AID
    // AT+CGLA=<sessionid>,<length>,<command>
    // <command> = 0X A4 04 00 length DATA
    // X = channel
    if(sr.simResponse != NULL) {
        free(sr.simResponse);
        sr.simResponse = NULL;
    }
    // GSMA NFC test:
    // 1. Ignore NULL object and empty string and retrun success directly.
    // 2. Assign p2 value as 0x00.(Recommanded by OpenMobileAPI3_0).
    if (pAid != NULL && (int)(strlen((char *)pAid)/2) > 0) { // To check NULL AID
        if(channel > 3) {
            LOGD("channel > 3");
            err = requestCGLA_OpenChannelWithSw(&sr, &len,
                    channel, 10 + strlen((char *)pAid), 0x40|(channel-4), 0xA4, 0x04, p2,
                    (int)(strlen((char *)pAid)/2), (char **)(&pAid), t);
        } else {
            LOGD("channel <= 3");
            err = requestCGLA_OpenChannelWithSw(&sr, &len,
                    channel, 10 + strlen((char *)pAid), channel, 0xA4, 0x04, p2,
                    (int)(strlen((char *)pAid)/2), (char **)(&pAid), t);
        }
    } else {
        LOGD("requestSIM_OpenChannelWithSw - channel = %x with NULL AID", channel);
        err = RIL_E_SUCCESS;
    }

    if (err != RIL_E_SUCCESS) {
        LOGD("requestSIM_OpenChannelWithSw: Select AID AT-CMD failed, and then close logical channel %d", channel);
        goto error_select_aid;
    } else if(err == RIL_E_SUCCESS) {
        if ((sr.sw1 == 0x6A && sr.sw2 == 0x82) ||
            (sr.sw1 == 0x6A && sr.sw2 == 0x81) ||
            (sr.sw1 == 0x69 && sr.sw2 == 0x85) ||
            (sr.sw1 == 0x69 && sr.sw2 == 0x99)) {
            RLOGE("Select AID, File not found");
            if(sr.simResponse != NULL) {
                free(sr.simResponse);
                sr.simResponse = NULL;
            }
            RIL_onRequestComplete(t, RIL_E_NO_SUCH_ELEMENT, NULL, 0);
            return;
        } else if ((sr.sw1 == 0x6A && sr.sw2 == 0x84) ||
                   (sr.sw1 == 0x68 && sr.sw2 == 0x81)) {
            RLOGE("Select AID, Not enough memory space in the file");
            if(sr.simResponse != NULL) {
                free(sr.simResponse);
                sr.simResponse = NULL;
            }
            RIL_onRequestComplete(t, RIL_E_MISSING_RESOURCE, NULL, 0);
            return;
        }
        goto no_error;
    }

error_select_aid:
    // Close logical Channel if selecting AID is failed
    // AT+CGLA=<sessionid>,<length>,<command>
    // <command> = 00 70 80 0X
    // X = channel
    asprintf(&cmd, "AT+CGLA=0,%d,\"%02x%02x%02x%02x\"",
                    8, 0x00, 0x70, 0x80, channel);
    err = at_send_command_singleline(cmd, "+CGLA:", &p_response, SIM_CHANNEL_CTX);
    LOGD("requestSIM_OpenChannelWithSw: close logical channel %d", channel);
    if (err < 0 || p_response->success == 0) {
        LOGD("requestSIM_OpenChannelWithSw: Close logical Channel failed");
        err = RIL_E_GENERIC_FAILURE;
    } else if (p_response->success > 0) {
        channel = 0;
        status[0] = 0;
    }
    goto error;

no_error:
    RLOGD("no_error, simResponse = %s ",sr.simResponse);
    if (sr.simResponse != NULL && strlen(sr.simResponse) > 0) {
        fullResponse = calloc(1, sizeof(int) * ((len / 2) + 3));
        if (NULL != fullResponse) {
            isFree = true;
            for(i = 0; i < (len / 2 - 2); i++) {
                sscanf(&(sr.simResponse[i * 2]), "%02x", &fullResponse[i + 1]);
            }
        } else {
            LOGD("requestSIM_OpenChannelWithSw, calloc fail.");
            fullResponse = status;
        }
    } else {
        LOGD("requestSIM_OpenChannelWithSw, simResponse is null.");
        fullResponse = status;
    }
    fullResponse[0] = channel;
    fullResponse[i + 1] = sr.sw1;
    fullResponse[i + 2] = sr.sw2;
    RLOGD("requestSIM_OpenChannelWithSw, Pass, channel = %d %02x, %02x",
            fullResponse[0], fullResponse[i + 1], fullResponse[i + 2]);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, fullResponse, (i + 3) * sizeof(int));

    if (sr.simResponse != NULL) {
        free(sr.simResponse);
        sr.simResponse = NULL;
    }
    if (isFree == true && NULL != fullResponse) {
        free(fullResponse);
    }
    return;
error:
    if (checkUserLoad() == 1) {
        LOGE("requestCGLA_OpenChannelWithSw, error %d, sw1 %x, sw2 %x",
            err, sr.sw1, sr.sw2);
    } else {
        LOGE("requestCGLA_OpenChannelWithSw, error %d, sw1 %x, sw2 %x, sr.simresponse %s",
            err, sr.sw1, sr.sw2, sr.simResponse);
    }
    if (sr.simResponse != NULL && strlen(sr.simResponse) > 0) {
        fullResponse = calloc(1, sizeof(int) * ((len / 2) + 3));
        if (NULL != fullResponse) {
            isFree = true;
            for(i = 0; i < (len / 2 - 2); i++) {
                sscanf(&(sr.simResponse[i * 2]), "%02x", &fullResponse[i + 1]);
            }
        } else {
            LOGE("requestCGLA_OpenChannelWithSw, calloc fail.");
            fullResponse = status;
        }
    } else {
        LOGE("requestSIM_OpenChannelWithSw, simResponse is null.");
        fullResponse = status;
    }
    fullResponse[0] = channel;
    fullResponse[i + 1] = sr.sw1;
    fullResponse[i + 2] = sr.sw2;
    if ((sr.sw1 == 0x6A && sr.sw2 == 0x82) ||
        (sr.sw1 == 0x69 && sr.sw2 == 0x85) ||
        (sr.sw1 == 0x69 && sr.sw2 == 0x99)) {
        RIL_onRequestComplete(t, RIL_E_NO_SUCH_ELEMENT, fullResponse,
                (i + 3) * sizeof(int));
    } else if ((sr.sw1 == 0x6A && sr.sw2 == 0x84) ||
               (sr.sw1 == 0x6A && sr.sw2 == 0x81) ||
               (sr.sw1 == 0x68 && sr.sw2 == 0x81)) {
        RIL_onRequestComplete(t, RIL_E_MISSING_RESOURCE, fullResponse,
                (i + 3) * sizeof(int));
    } else {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, fullResponse,
                (i + 3) * sizeof(int));
    }
    if (sr.simResponse != NULL) {
        free(sr.simResponse);
        sr.simResponse = NULL;
    }
    if (isFree == true && NULL != fullResponse) {
        free(fullResponse);
    }

    // free for close logical channel
    if(p_response != NULL) {
        at_response_free(p_response);
    }

    // free for close logical channel
    if(cmd != NULL) {
        free(cmd);
    }
    return;
}

// NFC SEEK end

void detectTestSim(RIL_SOCKET_ID rid)
{
    ATResponse *p_response = NULL;

    int err = at_send_command_singleline("AT+ETESTSIM?", "+ETESTSIM:", &p_response, getChannelCtxbyProxy());
    if (err < 0 || p_response->success == 0)
        RLOGE("detect test SIM failed");
    else
        onTestSimDetectedInternal(p_response->p_intermediates->line, rid, 0);

    at_response_free(p_response);
}

bool RIL_isTestSim(RIL_SOCKET_ID rid) {
    if((int)rid < getSimCount()) {
        return (isTestSim[rid]>0) ? true : false;
    } else {
        return false;
    }
}

// The request of feature comes from kernel! Due to HW limitation, MD will pause LTE via CCCI. But there is risk for some lab certification.
// So the kernel module want to avoid pausing LTE when using test SIM.
// We deliver the result of ETESTSIM to kernel here
static void pass_sim_type_to_vcorefs(RIL_SOCKET_ID rid, int testsim)
{
    int fd, len, r;
    char cmd[32];
    static unsigned int simtype = 0;

    if (testsim)
        simtype |= 1 << rid;
    else
        simtype &= ~(1 << rid);

    fd = open("/sys/power/vcorefs/pwr_ctrl", O_WRONLY);
    if (fd < 0) {
        RLOGE("failed to open vcorefs pwr_ctrl (%d)", errno);
        return;
    }

    len = snprintf(cmd, sizeof(cmd), "test_sim_prot %x", simtype);
    r = write(fd, cmd, len);
    if (r < 0)
        RLOGE("failed to write vcorefs pwr_ctrl (%d)", errno);

    close(fd);
}

void onTestSimDetected(const char *s, RIL_SOCKET_ID rid)
{
    onTestSimDetectedInternal(s, rid, 1);
}

void onTestSimDetectedInternal(const char *s, RIL_SOCKET_ID rid, int isUrc)
{
    int err = 0, para = 0;
    int simId = getMappingSIMByCurrentMode(rid);

    err = at_tok_start((char **)&s);
    if (err < 0) goto error;

    err = at_tok_nextint((char **)&s, &para);
    if (err < 0) goto error;

    switch(para) {
        case 0:
            RLOGD("onTestSimDetected(): set 0 to isTestSim[%d].", simId);
            if (isTestSim[rid] != para) {
                property_set(PROPERTY_RIL_TEST_SIM[simId], "0");
            }
            isTestSim[simId] = 0;
            pass_sim_type_to_vcorefs(simId, 0);
            break;
        case 1:
            RLOGD("onTestSimDetected(): set 1 to isTestSim[%d].", simId);
            if (isTestSim[rid] != para) {
                property_set(PROPERTY_RIL_TEST_SIM[simId], "1");
            }
            isTestSim[simId] = 1;
            pass_sim_type_to_vcorefs(simId, 1);
            break;
        default:
            RLOGE("onTestSimDetected() error, para: %d", para);
            break;
    }

    /// M: op01 6M @{
    if (isUrc) {
        onOp016mCardDetected(rid);
    }
    /// @}
    return;

error:
    RLOGE("There is something wrong with the URC: +ETESTSIM");
}

#define CCCI_IOC_MAGIC 'C'
#define CCCI_IOC_SIM_SWITCH      _IOW(CCCI_IOC_MAGIC, 20, unsigned int)

#define PHONE_MODE_DUALTALK    1
#define PHONE_MODE_GEMINI      0

#define SIM_SWITCH_MODE_CDMA    0x010001
#define SIM_SWITCH_MODE_GSM      0x010000

void requestSimInterfaceSwitch(void *data, size_t datalen, RIL_Token t)
{
    LOGD("[SimSw] requestSimInterfaceSwitch");

    int err;
    char *line;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    int* params = (int *) data;

    int ret_ioctl_val = -1;
    int ccci_sys_fd = -1;
    int switchMode;

    RIL_SIM_UNUSED_PARM(datalen);

    ccci_sys_fd = open("/dev/ccci_ioctl0", O_RDWR | O_NONBLOCK);
    if (ccci_sys_fd < 0) {
        LOGD("[SimSw]  id=%d", ccci_sys_fd);
        goto error;
    }

    if (*params == PHONE_MODE_DUALTALK)
        switchMode = SIM_SWITCH_MODE_CDMA;
    else
        switchMode = SIM_SWITCH_MODE_GSM;

    LOGD("[SimSw] requestSimInterfaceSwitch switchMode=%d", switchMode);

    ret_ioctl_val = ioctl(ccci_sys_fd, CCCI_IOC_SIM_SWITCH, &switchMode);
    LOGD("[SimSw] ret_ioctl_val = %d ", ret_ioctl_val);

    if (ret_ioctl_val  == -1) {
        LOGD("[SimSw]  strerror(errno)=%s", strerror(errno));
        goto error;
    }

    ret = RIL_E_SUCCESS;
error:
    if (ccci_sys_fd >= 0) {
        close(ccci_sys_fd);
    }
    RIL_onRequestComplete(t, ret, NULL, 0);
}

static int readEfDirResponseParser(unsigned char ** simResponse, int *record_length)
{
    int record_num = -1;
    unsigned char * fcpByte = NULL;
    unsigned short  fcpLen = 0;
    usim_file_descriptor_struct fDescriptor = {0,0,0,0};

    fcpLen = hexStringToByteArray(*simResponse, &fcpByte);

    do {
        if (FALSE == usim_fcp_query_tag(fcpByte, fcpLen, FCP_FILE_DES_T, &fDescriptor)) {
            RLOGE("USIM FD Parse fail");
            break;
        }
        record_num = fDescriptor.num_rec;
        (*record_length) = fDescriptor.rec_len;
    } while(0);

    free(fcpByte);
    RLOGE("readEfDirResponseParser, num: %d, length: %d", record_num, (*record_length));
    return record_num;
}

static char* queryEfDirReadLine(char* line) {
    int err;
    RIL_SIM_IO_Response sr;

    memset(&sr, 0, sizeof(RIL_SIM_IO_Response));

    do {
        err = at_tok_start(&line);
        if (err < 0) {
            break;
        }

        err = at_tok_nextint(&line, &(sr.sw1));
        if (err < 0) {
            break;
        }

        err = at_tok_nextint(&line, &(sr.sw2));
        if (err < 0) {
            break;
        }

        if (at_tok_hasmore(&line)) {
            err = at_tok_nextstr(&line, &(sr.simResponse));
            if (err < 0) {
                break;
            }
        }
    } while(0);

    RLOGD("queryEfDirReadLine, result: %d", ((sr.simResponse != NULL)? 1 : 0));
    return sr.simResponse;
}

static int toByte(char c)
{
    if (c >= '0' && c <= '9') return (c - '0');
    if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
    if (c >= 'a' && c <= 'f') return (c - 'a' + 10);

    RLOGE("toByte Error: %c",c);
    return 0;
}

int hexStringToByteArrayEx(unsigned char* hexString, int hexStringLen, unsigned char ** byte)
{
    int length = hexStringLen/2;
    int i = 0;
    unsigned char* buffer = calloc(1, (length + 1)*sizeof(char));
    assert(buffer != NULL);

    for (i = 0 ; i < hexStringLen ; i += 2)
    {
        buffer[i / 2] = (unsigned char)((toByte(hexString[i]) << 4) | toByte(hexString[i+1]));
    }

    *byte = buffer;

    return length;
}


void queryEfDir(RIL_SOCKET_ID rid) {
    size_t len;
    int err;
    int record_num = -1, i = 0, j = 0, record_length = 0;
    int app_info_len = 0, aid_len = 0, app_label_len = 0;
    int fcpLen = 0;
    char * line = NULL;
    char * cmd = NULL;
    unsigned char * fcpByte = NULL;
    unsigned char * aidHexStr = NULL;
    ATResponse * p_response = NULL;
    ATResponse * p_response2 = NULL;
    RIL_Errno ret = RIL_E_SUCCESS;
    char aid_len_str[3] = {0};

    RLOGD("queryEfDir, rid: %d, USIM: %d, flag: %d", rid, isUsimDetect[rid], read_ef_dir_complete[rid]);
    if (read_ef_dir_complete[rid] != 0) {
        RLOGE("return queryEfDir, rid: %d, USIM: %d, flag: %d", rid,
                isUsimDetect[rid], read_ef_dir_complete[rid]);
        return;
    }
    read_ef_dir_complete[rid] = 1;

    asprintf(&cmd, "AT+CRSM=%d,%d,%d,%d,%d", 0xc0, 0x2f00, 0 , 0, 0);

    do {
        err = at_send_command_singleline(cmd, "+CRSM:", &p_response, getChannelCtxbyProxy());
        if (err < 0 || p_response->success == 0) {
            RLOGE("Fail to GET_RESPONSE EfDir");
        } else {
            if ((line = queryEfDirReadLine(p_response->p_intermediates->line)) != NULL) {
                if ((record_num = readEfDirResponseParser((unsigned char **)&line, &record_length)) >= 0) {
                    // To read applicaion records
                    int matched = 0;
                    for (i = 0; i < record_num; i++) {
                        free(cmd);
                        cmd = NULL;
                        asprintf(&cmd, "AT+CRSM=%d,%d,%d,%d,%d", 0xb2, 0x2f00, (i+1) , 4, record_length);
                        err = at_send_command_singleline(cmd, "+CRSM:", &p_response2,
                                getChannelCtxbyProxy());
                        if (err < 0 || p_response2->success == 0) {
                            RLOGE("Fail to READ_RECORD EfDir");
                        } else {
                            if ((line = queryEfDirReadLine(p_response2->p_intermediates->line)) != NULL) {
                                fcpLen = hexStringToByteArrayEx((unsigned char*)line, 8,
                                        (unsigned char**)&fcpByte);
                                // Refer to in TS 102.221 13.1, the 2nd byte denotes the length of application
                                app_info_len = (int)fcpByte[1];
                                // Refer to in TS 102.221 13.1, the 4th byte denotes the length of AID
                                aid_len = (int)fcpByte[3];
                                free(fcpByte);
                                fcpByte = NULL;
                                // The range of AID len and application len are defined in TS 102.221 13.1
                                RLOGD("app len %d, AID len %d", app_info_len, aid_len);
                                if ((app_info_len < 3 || app_info_len > 127) ||
                                    (aid_len < 1 || aid_len > 16)) {
                                    RLOGE("Parameter are wrong!");
                                    break;
                                }
                                for (j = 0; j < AID_MAX; j++) {
                                    if (memcmp(aid_support_list[j], &line[8], AID_PREFIX_LEN) == 0) {
                                        memcpy(aid_info_array[rid][j].aid, &line[8], (aid_len*2));
                                        aid_info_array[rid][j].aid_len = aid_len;
                                        matched++;
                                        if ((app_info_len-(aid_len+1)) > 2) {
                                            // There is application label
                                            fcpLen = hexStringToByteArrayEx((unsigned char*)&line[8+(aid_len*2)],
                                                    4, &fcpByte);
                                            app_label_len = (int)fcpByte[1];
                                            aid_info_array[rid][j].app_label_len = app_label_len;
                                            free(fcpByte);
                                            fcpByte = NULL;
                                            RLOGD("app_label_len %d", app_label_len);
                                            memcpy(aid_info_array[rid][j].appLabel, &line[12+(aid_len*2)],
                                                    (app_label_len*2));
                                        }
                                        RLOGD("aid: %s, app_label: %s", aid_info_array[rid][j].aid,
                                                aid_info_array[rid][j].appLabel);
                                        break;
                                    }
                                };
                                if (matched == AID_MAX) {
                                    break;
                                }
                            }
                        }
                        at_response_free(p_response2);
                        p_response2 = NULL;
                    }
                }
            }
        }
    } while(0);

    at_response_free(p_response);
    free(cmd);
}

static const struct timeval TIMEVAL_PIN_SIM = { 0, 100000 };
static const unsigned int DETECT_SIM[MAX_SIM_COUNT] =
{
RIL_SOCKET_1,
RIL_SOCKET_2,
RIL_SOCKET_3,
RIL_SOCKET_4,
};

static void queryIccId(void *param) {
    TimedCallbackParam* mParam = (TimedCallbackParam*)param;
    char* urc = mParam->urc;
    RIL_SOCKET_ID rid = (RIL_SOCKET_ID)(mParam->rid);
    ATResponse *p_response = NULL;
    int err;
    char *line;
    char *iccId;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;
    RILChannelCtx *p_cctx = getChannelCtxbyProxy();
    char tmp[PROPERTY_VALUE_MAX] = {0};
    // SIM ME LOCK SBP feature
    SimSlotLockDeviceLockInfo deviceLockInfo;

    if (isSimSlotLockSupport() == 1) {
        getSimSlotLockDeviceLockInfo(&deviceLockInfo, NULL, rid);
    }

    RLOGD("[Socket %d] queryIccId: entering, urc:%s", rid, urc);

    err = at_send_command_singleline("AT+ICCID?", "+ICCID:", &p_response, p_cctx);

    if (err < 0 || NULL == p_response) {
        RLOGE("queryIccId Fail");
        goto error;
    }

    if (p_response->success == 0) {
        RLOGD("p_response = %d /n", at_get_cme_error(p_response) );
        switch (at_get_cme_error(p_response)) {
            case CME_SUCCESS:
                ret = RIL_E_GENERIC_FAILURE;
                break;
            case CME_SIM_BUSY:
                ret = RIL_E_REQUEST_NOT_SUPPORTED;
                RLOGD("p_response: CME_SIM_BUSY  ret = %d", ret);
                break;
            case CME_UNKNOWN:
                RLOGE("p_response: CME_UNKNOWN");
                break;
            default:
                break;
        }
    } else {
        ret = RIL_E_SUCCESS;
    }

    if (ret != RIL_E_SUCCESS) {
        RLOGE("queryIccId: Error or no SIM inserted!");
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start (&line);
    if (err < 0) goto error;

    err = at_tok_nextstr(&line, &iccId);
    if (err < 0) goto error;

    RLOGD("queryIccId: strlen of response is %zu", strlen(iccId) );
    // Convert all the lower case characters in iccid to upper case
    stringToUpper(iccId);
    property_set(PROPERTY_ICCID_SIM[rid], iccId);
    setSimInsertedStatus(rid, 1);
    if (strStartsWith(urc, "+ESIMS: 1,9")) {
        RLOGD("RILd,RIL_UNSOL_VIRTUAL_SIM_STATUS_CHANGED");
        int inserted = 1;
        RIL_UNSOL_RESPONSE(RIL_UNSOL_VIRTUAL_SIM_STATUS_CHANGED, &inserted, sizeof(int), rid);
    } else if (strStartsWith(urc, "+ESIMS: 1,14") || strStartsWith(urc, "+ESIMS: 1,12")) {
        RLOGD("To notify RIL_UNSOL_SIM_PLUG_IN");
        // MTK-START: BT SIM Access Profile
        if (strStartsWith(urc, "+ESIMS: 1,12")) {
            notifyBtSapStatusInd(RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_CARD_INSERTED,
                    rid);
        } else if (strStartsWith(urc, "+ESIMS: 1,14")) {
            notifyBtSapStatusInd(RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_RECOVERED, rid);
        }
        // MTK-END
        // MTK-START: SIM COMMON SLOT
        // External SIM [Start]
        if ((isSupportCommonSlot() == 1) &&
                (getVsimPlugInOutEvent(rid) == VSIM_TRIGGER_PLUG_IN)) {
            RLOGD("vsim trigger plug in on common slot project.");
            setVsimPlugInOutEvent(rid, VSIM_TRIGGER_RESET);
        } else if ((isSupportCommonSlot() == 1) && isVsimEnabled()) {
            RLOGD("Ingore plug in event during vsim enabled on common slot project.");
        } else {
        // External SIM [End]
            if (isSupportCommonSlot()) {
                if (strStartsWith(urc, "+ESIMS: 1,12")) {
                    RIL_UNSOL_RESPONSE(RIL_UNSOL_SIM_PLUG_IN, NULL, 0, rid);
                }
            } else {
                RIL_UNSOL_RESPONSE(RIL_UNSOL_SIM_PLUG_IN, NULL, 0, rid);
            }
        }
        // MTK-END
    }
    RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0, rid);
    free(mParam->urc);
    free(mParam);
    at_response_free(p_response);

    release_wake_lock("sim_hot_plug");
    return;
error:
    release_wake_lock("sim_hot_plug");
    if (ret == RIL_E_REQUEST_NOT_SUPPORTED) {
        RLOGD("SIM busy, failed to get ICCID, do nothing");
    } else if ((isSvlteSupport() || isSrlteSupport()) && (isUsimDetect[rid] == 2 &&
            (isCDMADetect[rid] == 0 || isCDMADetect[rid] == 1 || isCDMADetect[rid] == 3))) {
        property_set(PROPERTY_ICCID_SIM[rid], NULL);
        RLOGD("queryIccId: RUIM, CSIM, or RUIM & CSIM card, set %s to NULL",
                PROPERTY_ICCID_SIM[rid]);
    } else if (!s_md_off && (ret != RIL_E_REQUEST_NOT_SUPPORTED)) {
        property_set(PROPERTY_ICCID_SIM[rid], "N/A");
    }
    RLOGE("Got the event plug in, but failed to get ICCID");
    RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0, rid);
    free(mParam->urc);
    free(mParam);
    at_response_free(p_response);
}

static void detectSim(void *param) {
    TimedCallbackParam* mParam = (TimedCallbackParam*)param;
    char* urc = mParam->urc;
    RIL_SOCKET_ID rid = (RIL_SOCKET_ID)(mParam->rid);
    ATResponse *p_response = NULL;
    int err;
    int cmeError;
    int cardPresent = 0;
    //int count = 0;
    char *cpinLine;
    char *cpinResult;
    RILChannelCtx *p_cctx = getChannelCtxbyProxy();
    RILChannelId cid = p_cctx->id;

    RLOGD("[Socket %d] detectSim: entering, urc:%s", rid, urc);

    if (getRadioState(rid) == RADIO_STATE_UNAVAILABLE) {
        cardPresent = 1;
        goto detectSimDone;
    }

    err = at_send_command_singleline("AT+CPIN?", "+CPIN:", &p_response, p_cctx);
    if (err != 0) {
        RLOGE("detectSim: err: %d", err);
        if (err == AT_ERROR_INVALID_THREAD) {
            cardPresent = 1;
        } else {
            cardPresent = 0;
        }
        goto detectSimDone;
    }

    if (p_response->success == 0) {
        cmeError = at_get_cme_error(p_response);
        RLOGD("detectSim: cme_ERROR: %d", cmeError);
        switch (cmeError) {
            case CME_SIM_BUSY: // try to do again until more than 20 times
                cardPresent = 14; // SIM busy, framework needs to query again
                at_response_free(p_response);
                p_response = NULL;
                break;
            case CME_SIM_FAILURE:
            case CME_SIM_NOT_INSERTED:
                cardPresent = 0;
                goto detectSimDone;
            case CME_SIM_WRONG:
                cardPresent = 1;
                goto detectSimDone;
            default:
                cardPresent = 1; // NOT_READY
                goto detectSimDone;
        }
    } else {
        RLOGD("detectSim: success");
        cpinLine = p_response->p_intermediates->line;
        err = at_tok_start (&cpinLine);
        if(err < 0) {
            RLOGD("detectSim: CME_SUCCESS: err1: %d", err);
            cardPresent = 1;
            goto detectSimDone;
        }
        err = at_tok_nextstr(&cpinLine, &cpinResult);
        if(err < 0) {
            RLOGD("detectSim: CME_SUCCESS: err2: %d", err);
            cardPresent = 1;
            goto detectSimDone;
        }

        if (strcmp(cpinResult, "SIM PIN") && strcmp(cpinResult, "SIM PUK")
                && strcmp(cpinResult, "PH-NET PIN") && strcmp(cpinResult, "PH-NET PUK")
                && strcmp(cpinResult, "PH-NETSUB PIN") && strcmp(cpinResult, "PH-NETSUB PUK")
                && strcmp(cpinResult, "PH-SP PIN") && strcmp(cpinResult, "PH-SP PUK")
                && strcmp(cpinResult, "PH-CORP PIN") && strcmp(cpinResult, "PH-CORP PUK")
                && strcmp(cpinResult, "PH-FSIM PIN") && strcmp(cpinResult, "PH-FSIM PUK")
                && strcmp(cpinResult, "READY") && strcmp(cpinResult, "SIM PIN-DEFER")) {
            RLOGD("detectSim: out of strcmp: %s", cpinResult);
            cardPresent = 0;
        }
        else {
            cardPresent = 1;
        }
        goto detectSimDone;
    }

detectSimDone:
    release_wake_lock("sim_hot_plug");
    switch (cardPresent) {
        case 1:
            /* Card inserted */
            if (isSupportC2kSim()) {
                RLOGD("Ready to check EUSIM (%d,%d) in the case of event plug in",
                        isUsimDetect[rid], isCDMADetect[rid]);
                if (isUsimDetect[rid] == 0 || isUsimDetect[rid] == 1) {
                    RLOGD("To notify RIL_UNSOL_SIM_PLUG_IN");
                    if (strStartsWith(urc, "+ESIMS: 1,9")) {
                        RLOGD("RILd,RIL_UNSOL_VIRTUAL_SIM_STATUS_CHANGED");
                        int inserted = 1;
                        RIL_UNSOL_RESPONSE(RIL_UNSOL_VIRTUAL_SIM_STATUS_CHANGED,
                                &inserted, sizeof(int), rid);
                    // MTK-START: BT SIM Access Profile
                    } else if (strStartsWith(urc, "+ESIMS: 1,12")) {
                        notifyBtSapStatusInd(
                                RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_CARD_INSERTED, rid);
                    } else if (strStartsWith(urc, "+ESIMS: 1,14")) {
                        notifyBtSapStatusInd(
                                RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_RECOVERED, rid);
                    }
                    // MTK-END

                    // External SIM [Start]
                    if ((isSupportCommonSlot() == 1) &&
                            (getVsimPlugInOutEvent(rid) == VSIM_TRIGGER_PLUG_IN)) {
                        RLOGD("vsim trigger plug in on common slot project.");
                        setVsimPlugInOutEvent(rid, VSIM_TRIGGER_RESET);
                    } else if ((isSupportCommonSlot() == 1) && isVsimEnabled()) {
                        RLOGD("Ingore plug in event during vsim enabled on common slot project.");
                    } else {
                    // External SIM [End]
                        RIL_UNSOL_RESPONSE(RIL_UNSOL_SIM_PLUG_IN, NULL, 0, rid);
                    }
                } else if (isCDMADetect[rid] == -1) {
                    // Wait for +EUSIM
                    RIL_requestProxyTimedCallback (detectSim, mParam, &TIMEVAL_PIN_SIM,
                            getRILChannelId(RIL_SIM, rid), "detectSim");
                    acquire_wake_lock(PARTIAL_WAKE_LOCK, "sim_hot_plug");
                }
            } else {
                RLOGD("To query iccid");
                RIL_requestProxyTimedCallback (queryIccId, mParam, &TIMEVAL_PIN_SIM,
                        getRILChannelId(RIL_SIM, rid), "queryIccId");
                acquire_wake_lock(PARTIAL_WAKE_LOCK, "sim_hot_plug");
            }
            break;
        case 0:
            if (isSupportC2kSim()) {
                RLOGD("Ready to check EUSIM (%d,%d) in the case of event plug in but no SIM",
                        isUsimDetect[rid], isCDMADetect[rid]);
                if (isCDMADetect[rid] == -1) {
                    // Wait for +EUSIM
                    RIL_requestProxyTimedCallback (detectSim, mParam, &TIMEVAL_PIN_SIM,
                            getRILChannelId(RIL_SIM, rid), "detectSim");
                    acquire_wake_lock(PARTIAL_WAKE_LOCK, "sim_hot_plug");
                    break;
                } else if (isCDMADetect[rid] == 2) {
                    // MTK-START: SIM COMMON SLOT
                    if (isSupportCommonSlot()) {
                        RLOGD("notify RIL_UNSOL_SIM_COMMON_SLOT_NO_CHANGED");
                        property_set(PROPERTY_ICCID_SIM[rid], "N/A");
                        // External SIM [Start]
                        if ((getVsimPlugInOutEvent(rid) == VSIM_TRIGGER_PLUG_IN)) {
                            RLOGD("vsim trigger no change on common slot project.");
                            setVsimPlugInOutEvent(rid, VSIM_TRIGGER_RESET);
                        } else if (isVsimEnabled()) {
                            RLOGD("Ingore no change event during vsim enabled on common slot project.");
                        } else {
                        // External SIM [End]
                            RIL_UNSOL_RESPONSE (RIL_UNSOL_SIM_COMMON_SLOT_NO_CHANGED, NULL, 0, rid);
                        }
                    }
                    // MTK-END
                } else if (isCDMADetect[rid] == 0 || isCDMADetect[rid] == 1 ||
                        isCDMADetect[rid] == 3) {
                    // External SIM [Start]
                    if ((isSupportCommonSlot() == 1) &&
                            (getVsimPlugInOutEvent(rid) == VSIM_TRIGGER_PLUG_IN)) {
                        RLOGD("vsim trigger plug in on common slot project.");
                        setVsimPlugInOutEvent(rid, VSIM_TRIGGER_RESET);
                    } else if ((isSupportCommonSlot() == 1) && isVsimEnabled()) {
                        RLOGD("Ingore plug in event during vsim enabled on common slot project.");
                    } else {
                    // External SIM [End]
                        RIL_UNSOL_RESPONSE(RIL_UNSOL_SIM_PLUG_IN, NULL, 0, rid);
                    }
                    RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL,
                            0, rid);
                }
            } else {
                RLOGE("Event plug in was happened, but there is no SIM inserted actually.");
                RIL_UNSOL_RESPONSE (RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0, rid);
                // MTK-START: SIM COMMON SLOT
                if (isSupportCommonSlot()) {
                    RLOGD("notify RIL_UNSOL_SIM_COMMON_SLOT_NO_CHANGED");
                    property_set(PROPERTY_ICCID_SIM[rid], "N/A");

                    if (isSupportCommonSlot()) {
                        // External SIM [Start]
                        if ((getVsimPlugInOutEvent(rid) == VSIM_TRIGGER_PLUG_IN)) {
                            RLOGD("vsim trigger no change on common slot project.");
                            setVsimPlugInOutEvent(rid, VSIM_TRIGGER_RESET);
                        } else if (isVsimEnabled()) {
                            RLOGD("Ingore no change event during vsim enabled on common slot project.");
                        } else {
                        // External SIM [End]
                            RIL_UNSOL_RESPONSE (RIL_UNSOL_SIM_COMMON_SLOT_NO_CHANGED, NULL, 0, rid);
                        }
                    }
                }
                // MTK-END
            }
            free(mParam->urc);
            free(mParam);
            break;
        case 14:
            RLOGD("detect sim and sim busy so try again");
            RIL_requestProxyTimedCallback (detectSim, mParam, &TIMEVAL_PIN_SIM,
                    getRILChannelId(RIL_SIM, rid), "detectSim");
            acquire_wake_lock(PARTIAL_WAKE_LOCK, "sim_hot_plug");
            break;
        default:
            RLOGE("Event plug in was happened, but CPIN? response");
            free(mParam->urc);
            free(mParam);
            break;
    }
    at_response_free(p_response);
    return;
}

static void onCdmaDetected(const char *s, RIL_SOCKET_ID rid) {
    int cdma_type = -1, card_type_len = 19;
    char* line = (char*)s;
    int simId = getMappingSIMByCurrentMode(rid);
    // RILChannelCtx *p_cctx = getChannelCtxbyProxy();
    const char *propUicc = PROPERTY_RIL_UICC_TYPE[simId];
    const char *fullUiccType = PROPERTY_RIL_FULL_UICC_TYPE[simId];
    char card_type[20] = {0};
    char tmp[PROPERTY_VALUE_MAX] = {0};

    if (at_tok_start(&line) < 0) {
        goto error;
    }

    if (at_tok_nextint(&line, &(isUsimDetect[rid])) < 0) {
        RLOGE("Get EUSIM type fail!");
        goto error;
    }

    if (isUsimDetect[rid] == 1) {
        property_set(propUicc, "USIM");
        snprintf(card_type, 19, "USIM");
    } else if (isUsimDetect[rid] == 0) {
        property_set(propUicc, "SIM");
        snprintf(card_type, 19, "SIM");
    } else {
        RLOGD("The SIM card is neither USIM nor SIM!");
    }

    card_type_len = card_type_len - strlen(card_type);
    RLOGD("UICC %d Type identified as %d, card type len %d", rid, isUsimDetect[rid], card_type_len);

    /* Check CDMA card type */
    if (at_tok_hasmore(&line)) {
        if (at_tok_nextint(&line, &cdma_type) < 0) {
            RLOGE("Get CDMA type fail!");
        } else {
            isCDMADetect[rid] = cdma_type;
            RLOGD("CDMA %d Type identified as %d", rid, isCDMADetect[rid]);
            if (card_type_len != 19 && isCDMADetect[rid] != 2) {
                strncat(card_type, ",", 1);
                card_type_len = card_type_len - 1;
            }

            switch (cdma_type) {
                case 0:
                    if (isUsimDetect[rid] == 2) {
                        snprintf(card_type, 19, "RUIM");
                    } else {
                        strncat(card_type, "RUIM", card_type_len);
                    }
                    property_set(propUicc, "RUIM");
                    break;
                case 1:
                    if (isUsimDetect[rid] == 2) {
                        snprintf(card_type, 19, "CSIM");
                    } else {
                        strncat(card_type, "CSIM", card_type_len);
                    }
                    property_set(propUicc, "CSIM");
                    break;
                case 3:
                    if (isUsimDetect[rid] == 2) {
                        snprintf(card_type, 19, "RUIM,CSIM");
                    } else {
                        strncat(card_type, "RUIM,CSIM", card_type_len);
                    }
                    property_set(propUicc, "CSIM");
                    break;
                default:
                    RLOGE("The SIM card is neither RUIM nor CSIM!");
                    break;
            }
        }
    }

    if ((isUsimDetect[rid] == 2 && isCDMADetect[rid] == 2) ||
        (isUsimDetect[rid] == 2 && isCDMADetect[rid] == -1)) {
        RLOGE("There is no card type!!!");
        memset(card_type, 0, 20*sizeof(char));
        strncpy(card_type, "N/A", 19);
        property_set(fullUiccType, "");
        property_set(propUicc, "");
        RIL_UNSOL_RESPONSE(RIL_LOCAL_GSM_UNSOL_CARD_TYPE_NOTIFY, card_type,
            strlen(card_type), rid);
    } else {
        RLOGD("The card type is %s", card_type);
        // property_set(propUicc, card_type);
        property_set(fullUiccType, card_type);
        //Set sim inserted status by +EUSIM. Because
        //modem will report +ESIMS: 1,12 if AT+ESIMPOWER=1 even no SIM inserted.
        setSimInsertedStatus(rid, 1);
        RIL_UNSOL_RESPONSE(RIL_LOCAL_GSM_UNSOL_CARD_TYPE_NOTIFY , card_type,
            strlen(card_type), rid);
    }

    if ((isSvlteSupport() || isSrlteSupport()) && (isUsimDetect[rid] == 2 &&
            (isCDMADetect[rid] == 0 || isCDMADetect[rid] == 1 || isCDMADetect[rid] == 3))) {
        property_set(PROPERTY_ICCID_SIM[rid], NULL);
        RLOGD("onCdmaDetected RUIM, CSIM, or RUIM & CSIM card, set %s to NULL",
                PROPERTY_ICCID_SIM[rid]);
    } else {
        char* urc = NULL;
        asprintf(&urc, "%s", s);
        RLOGD("[Socket %d] onCdmaDetected: to queryIccid, urc:%s", rid, urc);
        TimedCallbackParam* param = (TimedCallbackParam*)calloc(1, sizeof(TimedCallbackParam));
        assert(param != NULL);
        param->rid = rid;
        param->urc = urc;
        RIL_requestProxyTimedCallback(queryIccId, (void *)param, &TIMEVAL_PIN_SIM,
                getRILChannelId(RIL_SIM, rid), "queryIccId");
    }

    property_get(PROPERTY_RIL_CT3G_ROAMING[rid], tmp, "0");
    RLOGD("%s: %s", PROPERTY_RIL_CT3G_ROAMING[rid], tmp);
    if (strcmp("1", tmp) == 0) {
        property_set(PROPERTY_RIL_CT3G_ROAMING[rid], "0");
    }
    return;
error:
    RLOGE("Parse EUSIM fail: %s/n", s);
}

// For STK/UTK mode switch start
static void switchStkUtkMode(void *param) {
    SwitchStkUtkModeParam *temp = (SwitchStkUtkModeParam*)param;
    int mode = temp->mode;
    RIL_SOCKET_ID rid = temp->rid;
    RILChannelCtx* p_channel = getChannelCtxbyProxy();

    int err = 0;
    ATResponse *p_response = NULL;
    LOGD("switchStkUtkMode(), mode=%d.", mode);

    // AT+EUTK
    char* cmd;
    err = asprintf(&cmd, "AT+EUTK=%d", mode);
    LOGD("switchStkUtkMode(), send command %s.", cmd);
    err = at_send_command(cmd, &p_response, p_channel);
    free(cmd);
    at_response_free(p_response);
    p_response = NULL;
    free(param);
}

void switchStkUtkModeByCardType(RIL_SOCKET_ID rid) {
    int simId = getMappingSIMByCurrentMode(rid);
    const char *propUicc = PROPERTY_RIL_UICC_TYPE[simId];
    char tmp[PROPERTY_VALUE_MAX] = {0};
    SwitchStkUtkModeParam *param = (SwitchStkUtkModeParam*)malloc(sizeof(SwitchStkUtkModeParam));
    assert(param != NULL);

    // RILChannelCtx* p_channel = getChannelCtxbyProxy();
    // RILChannelCtx *p_cctx = getChannelCtxbyProxy();
    // LOGD("switchStkUtkMode(), socket_id %d.",rid );

    property_get(propUicc, tmp, "1");
    if (strcmp(tmp, "SIM") == 0 || strcmp(tmp, "USIM") == 0) {
        param->mode = 0;
        param->rid = rid;
        char* funcString = "switchStkUtkMode";
        RIL_requestProxyTimedCallback(switchStkUtkMode, param, NULL, getRILChannelId(RIL_SIM, rid),
                funcString);
    } else if (strcmp(tmp, "RUIM") == 0 ||strcmp(tmp, "CSIM") == 0) {
        param->mode = 1;
        param->rid = rid;
        char* funcString = "switchStkUtkMode";
        RIL_requestProxyTimedCallback(switchStkUtkMode, param, NULL, getRILChannelId(RIL_SIM, rid),
                funcString);
    }
}
// For STK/UTK mode switch end

static void handleCardTypeUrc(const char *s, RIL_SOCKET_ID rid) {
    int supportC2k = isSupportC2kSim();
    int simId = getMappingSIMByCurrentMode(rid);

    RLOGD("handleCardTypeUrc,rid: %d, %s", rid, s);

    if (supportC2k == 1) {
        onCdmaDetected(s, rid);
        // switchStkUtkModeByCardType(rid);
    } else {
        char* line = (char*)s;
        char* urc = NULL;
        const char *propUicc = PROPERTY_RIL_UICC_TYPE[simId];
        // RILChannelCtx *p_cctx = getChannelCtxbyProxy();
        TimedCallbackParam* param = NULL;

        if (at_tok_start(&line) < 0) {
            goto error;
        }

        if (at_tok_nextint(&line, &(isUsimDetect[rid])) < 0) {
            RLOGE("Get EUSIM type fail!");
            goto error;
        }

        RLOGD("UICC %d Type identified as %d", rid,isUsimDetect[rid]);

        if (isUsimDetect[rid]) {
            property_set(propUicc, "USIM");
        } else {
            property_set(propUicc, "SIM");
        }

        if (isUsimDetect[rid] != 2 && isUsimDetect[rid] != -1) {
            //Set sim inserted status by +EUSIM. Because
            //modem will report +ESIMS: 1,12 if AT+ESIMPOWER=1 even no SIM inserted.
            setSimInsertedStatus(rid, 1);
        }
        asprintf(&urc, "%s", s);
        param = (TimedCallbackParam*)calloc(1, sizeof(TimedCallbackParam));
        assert(param != NULL);
        param->rid = rid;
        param->urc = urc;
        RIL_requestProxyTimedCallback(queryIccId, (void *)param, &TIMEVAL_PIN_SIM,
                getRILChannelId(RIL_SIM, rid), "queryIccId");
    }
    setupOpProperty(rid);
    // Dynamic-SBP: Run-time configure SBP-ID according to SIM MCC/MNC
    RLOGD("SIM detected! Setup Dynamic-SBP for RIL:%d", rid);
    setupDynamicSBP(rid);
    return;

error:
    RLOGE("Parse EUSIM fail: %s/n", s);
}

void onUsimDetected(const char *s, RIL_SOCKET_ID rid) {

    /// M: OP01 6M @{
    /*
     * ALPS02402386, For OP01 6M test, need to remove CDMA capability for TEST SIM card only.
     * 1. It has USIM app,
     * 2. and it has CDMA capability
     * So cache EUSIM wait for ETESTSIM.
     */
    int simId = getMappingSIMByCurrentMode(rid);

    onSmlSbpConfigureCheck();

    /*
     * ALPS03022529, according to property set in engineer mode, test sim card works as
     * "0": Default
     * "1": CDMA card
     * "2": GSM only card
     */
    char tmp[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.vendor.radio.testsim.cardtype", tmp, "0");
    LOGD("onUsimDetected persist.vendor.radio.testsim.cardtype : %s", tmp);

    if (0 == strcmp(tmp, "0")) {
        // Default
        if ((isOp016mSupport() || isOp096mSupport()) && hasCdmaCapability(s)) {
            if (g_eusim[simId] == NULL) {
                cacheCardTypeUrc(s, rid);
                checkPinState(s, rid);
            }
            return;
        }
    } else if (0 == strcmp(tmp, "1")) {
        // Works normally
    } else if (0 == strcmp(tmp, "2")) {
        // GSM only card
        if (hasCdmaCapability(s)) {
            if (g_eusim[simId] == NULL) {
                isUsimDetect[simId] = -1;
                isCDMADetect[simId] = -1;
                cacheCardTypeUrc(s, rid);
                checkPinState(s, rid);
            }
            return;
        }
    }
    /// @}

    handleCardTypeUrc(s, rid);
}

static void setPropertyCt3g(int value, RIL_SOCKET_ID rid) {
    RLOGD("setPropertyCt3g value = %d, rid = %d", value, rid);
    if (value == 0) {
        property_set(PROPERTY_RIL_CT3G[rid], "0");
    } else {
        property_set(PROPERTY_RIL_CT3G[rid], "1");
    }
}

void onCt3gDetected(const char *s, RIL_SOCKET_ID rid) {
    int ret = 0;
    int err = 0;

    RLOGD("onCt3gDetected rid = %d", rid);

    err = at_tok_start((char **)&s);
    if (err < 0) {
        RLOGE("Unknown command!");
        goto error;
    }

    err = at_tok_nextint((char **)&s, &ret);
    if (err < 0) {
        RLOGE("Unknown parameter!");
        goto error;
    }

    setPropertyCt3g(ret, rid);
    RIL_UNSOL_RESPONSE(RIL_LOCAL_GSM_UNSOL_CT3G_DUALMODE_CARD, &ret, sizeof(int), rid);
    return;
error:
    setPropertyCt3g(0, rid);
}

void onSimInsertChanged(const char *s,RIL_SOCKET_ID rid)
{
    RILChannelCtx *p_cctx = getChannelCtxbyProxy();
    RILChannelId cid = p_cctx->id;
    char card_type[20] = {0};

    RLOGD("onSimInsertChanged rid = %d, cid = %d", rid, cid);

    if (aIs_sim_off_by_IMEI_lock[rid] == true) {
        aIs_sim_off_by_IMEI_lock[rid] = false;
        RLOGD("aIs_sim_off_by_IMEI_lock[%d] value:%d", rid, aIs_sim_off_by_IMEI_lock[rid]);
    }

    // MTK-START: AOSP SIM PLUG IN/OUT
    char* p_esim_cause = NULL;
    int esim_cause = -1;

    p_esim_cause = strchr(s, ',');
    if (p_esim_cause != NULL) {
        RLOGD("onSimInsertChanged esim_cause = %s", p_esim_cause + 1);
        property_set(PROPERTY_ESIMS_CAUSE, p_esim_cause + 1);
    }

    // MTK-END
    int supportC2k = isSupportC2kSim();
    if ((supportC2k == 1) && strStartsWith(s, "+ESIMS: 0,11")) {
        RLOGD("SIM plug out");
        SwitchStkUtkModeParam *param = (SwitchStkUtkModeParam*)malloc(sizeof(SwitchStkUtkModeParam));
        param->mode = 1;
        param->rid = rid;
        char* funcString = "switchStkUtkMode";
        RIL_requestProxyTimedCallback(switchStkUtkMode, param, NULL,
                getRILChannelId(RIL_SIM, rid), funcString);
    }

    if (strStartsWith(s, "+ESIMS: 0,0") || strStartsWith(s, "+ESIMS: 0,13") || strStartsWith(s, "+ESIMS: 0,10") || strStartsWith(s, "+ESIMS: 0,11") || strStartsWith(s, "+ESIMS: 0,15")) {
        //+ESIMS: 0,0: SIM Missing
        //+ESIMS: 0,13: Recovery start
        //+ESIMS: 0,10: Virtual SIM off
        //+ESIMS: 0,11: SIM plug out
        //+ESIMS: 0,15: ATT IMEI Lock

        /// M: CC: reset NW ECC when SIM plug out
        if (strStartsWith(s, "+ESIMS: 0,11")) {
            // ALPS02880266, clear ECC number and service category when sim plugged out
            int simId = getMappingSIMByCurrentMode(rid);
            property_set(PROPERTY_NW_ECC[simId], "");
            RLOGD("%s: reset %s. SIM removed", __FUNCTION__, PROPERTY_NW_ECC[simId]);
        }

        // MTK-START: SIM COMMON SLOT
        int common_slot_no_changed = 0;
        // MTK-END
        if (getSimCount()>= 2) {
            // MTK-START: SIM COMMON SLOT
            if (strStartsWith(s, "+ESIMS: 0,11") && (isSupportCommonSlot() == 1)) {
                // Check if it is no sim -> no sim
                if (!isSimInserted(rid) &&
                        ((isUsimDetect[rid] == 2 && isCDMADetect[rid] == 2) ||
                        (isUsimDetect[rid] == 2 && isCDMADetect[rid] == -1))) {
                    common_slot_no_changed = 1;
                }
                RLOGD("Common slot plug out, no changed: %d", common_slot_no_changed);
            }
            // MTK-END
            RLOGD("onSimInsertChanged [%d]", sim_inserted_status);
            setSimInsertedStatus(rid, 0);
        } else {
            sim_inserted_status = 0;
        }

        if (strStartsWith(s, "+ESIMS: 0,11") || strStartsWith(s, "+ESIMS: 0,13") || strStartsWith(s, "+ESIMS: 0,10")) {
            RLOGD("onSimInsertChanged, SIM Plug out, SIM Missing or Virtual SIM off, no_changed %d",
                    common_slot_no_changed);
            resetSIMProperties(rid);
            property_set(PROPERTY_ICCID_SIM[rid], "N/A");
            // Reset the stk service running flag and clear queued Proactive Command.
            // Set to true in 'ril_stk.c' when CatService is ready and report ready again later.
            setStkServiceRunningFlag(rid, false);
            freeStkQueuedProactivCmdByRid(rid);
            freeStkCacheMenuByRid(rid);
            resetAidInfo(rid);
            // MTK-START: BT SIM Access Profile
            if (queryBtSapStatus(rid) != BT_SAP_INIT && common_slot_no_changed == 0) {
                // During BT SAP connection
                if (strStartsWith(s, "+ESIMS: 0,11")) {
                    notifyBtSapStatusInd(RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_CARD_REMOVED,
                            rid);
                } else if (strStartsWith(s, "+ESIMS: 0,13")) {
                    notifyBtSapStatusInd(RIL_SIM_SAP_STATUS_IND_Status_RIL_SIM_STATUS_CARD_NOT_ACCESSIBLE,
                            rid);
                }
            }
            resetBtSapContext(rid);
            // MTK-END
            if (isSupportC2kSim()) {
                RLOGD("Plug out on C2K project (%d, %d)", isUsimDetect[rid], isCDMADetect[rid]);
                property_set(PROPERTY_RIL_FULL_UICC_TYPE[rid], NULL);
                property_set(PROPERTY_RIL_CT3G_ROAMING[rid], NULL);
                property_set(PROPERTY_RIL_CT3G_ROAMING2[rid], NULL);
                strncpy(card_type, "N/A", 19);
                RIL_UNSOL_RESPONSE(RIL_LOCAL_GSM_UNSOL_CARD_TYPE_NOTIFY, card_type,
                        strlen(card_type), rid);
                setPropertyCt3g(0, rid);
                // MTK-START: SIM COMMON SLOT
                if (common_slot_no_changed == 1) {
                    // External SIM [Start]
                    if (isVsimEnabled()) {
                        RLOGD("Ingore no changed event during vsim enabled on common slot project.");
                    } else {
                    // External SIM [End]
                        RIL_UNSOL_RESPONSE(RIL_UNSOL_SIM_COMMON_SLOT_NO_CHANGED, NULL, 0, rid);
                    }
                } else {
                    // External SIM [Start]
                    if ((isSupportCommonSlot() == 1) && (getVsimPlugInOutEvent(rid) == VSIM_TRIGGER_PLUG_OUT)) {
                        RLOGD("vsim trigger plug out on common slot project.");
                        setVsimPlugInOutEvent(rid, VSIM_TRIGGER_RESET);
                    } else if ((isSupportCommonSlot() == 1) && isVsimEnabled()) {
                        RLOGD("Ingore plug out event during vsim enabled on common slot project.");
                    } else {
                    // External SIM [End]
                        if (isSupportCommonSlot() == 1) {
                            // To send the event plug out only when card removed in common slot project
                            if (strStartsWith(s, "+ESIMS: 0,11")) {
                                RIL_UNSOL_RESPONSE(RIL_UNSOL_SIM_PLUG_OUT, NULL, 0, rid);
                            }
                        } else {
                            RIL_UNSOL_RESPONSE(RIL_UNSOL_SIM_PLUG_OUT, NULL, 0, rid);
                        }
                    }
                    isUsimDetect[rid] = 2;
                    isCDMADetect[rid] = -1;
                    RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0, rid);
                }
                // MTK-END
            } else {
                // MTK-START: SIM COMMON SLOT
                if (common_slot_no_changed == 1) {
                    // External SIM [Start]
                    if (isVsimEnabled()) {
                        RLOGD("Ingore no changed event during vsim enabled on common slot project.");
                    } else {
                    // External SIM [End]
                        RIL_UNSOL_RESPONSE(RIL_UNSOL_SIM_COMMON_SLOT_NO_CHANGED, NULL, 0, rid);
                    }
                } else {
                    // External SIM [Start]
                    if ((isSupportCommonSlot() == 1) && (getVsimPlugInOutEvent(rid) == VSIM_TRIGGER_PLUG_OUT)) {
                        RLOGD("vsim trigger plug out on common slot project.");
                        setVsimPlugInOutEvent(rid, VSIM_TRIGGER_RESET);
                    } else if ((isSupportCommonSlot() == 1) && isVsimEnabled()) {
                        RLOGD("Ingore plug out event during vsim enabled on common slot project.");
                    } else {
                        // External SIM [End]
                        if (isSupportCommonSlot() == 1) {
                            // To send the event plug out only when card removed in common slot project
                            if (strStartsWith(s, "+ESIMS: 0,11")) {
                                RIL_UNSOL_RESPONSE(RIL_UNSOL_SIM_PLUG_OUT, NULL, 0, rid);
                            }
                        } else {
                            RIL_UNSOL_RESPONSE(RIL_UNSOL_SIM_PLUG_OUT, NULL, 0, rid);
                        }
                    }
                    RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0, rid);
                }
                // MTK-END
            }

            char* prop = NULL;
            char* propCdma = NULL;
            // +ESIMAPP URC. Reset mcc mnc property.
            if (rid == RIL_SOCKET_1) {
                asprintf(&prop, "%s", PROPERTY_MCC_MNC);
                asprintf(&propCdma, "%s", PROPERTY_MCC_MNC_CDMA);
            } else {
                asprintf(&prop, "%s.%c", PROPERTY_MCC_MNC, rid + '0');
                asprintf(&propCdma, "%s.%c", PROPERTY_MCC_MNC_CDMA, rid + '0');
            }
            property_set(prop, "N/A");
            property_set(propCdma, "N/A");
            free(prop);
            free(propCdma);
        } else if(strStartsWith(s, "+ESIMS: 0,15")) {
            RIL_UNSOL_RESPONSE(RIL_UNSOL_IMEI_LOCK, NULL, 0, rid);
        } else if (strStartsWith(s, "+ESIMS: 0,10")) {
            RLOGD("RILd,RIL_UNSOL_VIRTUAL_SIM_STATUS_CHANGED");
            resetSIMProperties(rid);
            resetAidInfo(rid);
            int inserted = 0;
            RIL_UNSOL_RESPONSE(RIL_UNSOL_VIRTUAL_SIM_STATUS_CHANGED, &inserted, sizeof(int), rid);
            RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0, rid);
        } else {
            RLOGD("onSimInsertChanged the other");
            RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0, rid);
        }
    // MTK-START: AOSP SIM PLUG IN/OUT
    } else if (strStartsWith(s, "+ESIMS: 0,26")) {
        RLOGD("SIM Plug in but no init.");
        RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0, rid);
    // MTK-END
    } else if (strStartsWith(s, "+ESIMS: 1,9") || strStartsWith(s, "+ESIMS: 1,14") || strStartsWith(s, "+ESIMS: 1,12")) {
        //+ESIMS: 1,9: Virtual SIM on
        //+ESIMS: 1,14: Recovery end
        //+ESIMS: 1,12: SIM plug in
        if (getSimCount()>= 2) {
            // MTK-START: SIM COMMON SLOT
            if(isSupportCommonSlot() == 1 && (strStartsWith(s, "+ESIMS: 1,12"))) {
                // External SIM [Start]
                if ((isSupportCommonSlot() == 1) && (getVsimPlugInOutEvent(rid) == VSIM_TRIGGER_PLUG_IN)) {
                    RLOGD("vsim trigger tray plug in on common slot project.");
                    //setVsimPlugInOutEvent(m_slot_id, VSIM_TRIGGER_RESET);
                } else if ((isSupportCommonSlot() == 1) && isVsimEnabled()) {
                    RLOGD("Ingore tray plug in event during vsim enabled on common slot project.");
                } else {
                // External SIM [End]
                    // In this feature, when we receive "ESIMS: 1, 12", it does not mean SIM card plug,
                    // but means slot plug in. That is, it might be no SIM card in this slot.
                    // Thus, we need to query SIM state when detect SIM missing and update flag at that time.
                    RLOGD("Receive plug in(common slot project)so do not set sim inserted status here");
                    property_set(PROPERTY_ICCID_SIM[rid], NULL);
                    RIL_UNSOL_RESPONSE(RIL_UNSOL_TRAY_PLUG_IN, NULL, 0, rid);
                }
            } else {
            // MTK-END
            //    setSimInsertedStatus(rid, 1);
            }
        }

        RLOGD("onSimInsertChanged, SIM Plug in, SIM Recovery or Virtual SIM on");
        char* urc = NULL;
        asprintf(&urc, "%s", s);
        TimedCallbackParam* param = (TimedCallbackParam*)calloc(1, sizeof(TimedCallbackParam));
        assert(param != NULL);
        param->rid = rid;
        param->urc = urc;
        RIL_requestProxyTimedCallback (detectSim, param, &TIMEVAL_PIN_SIM,
                getRILChannelId(RIL_SIM, rid), "detectSim");
        acquire_wake_lock(PARTIAL_WAKE_LOCK, "sim_hot_plug");
    } else if (strStartsWith(s, "+ESIMS: 1,2")) {
        RLOGD("SIM_REFRESH_DONE");
        RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0, rid);
        RIL_UNSOL_RESPONSE(RIL_UNSOL_IMSI_REFRESH_DONE, NULL, 0, rid);
        // Dynamic-SBP: Run-time configure SBP-ID according to SIM MCC/MNC: ALPS03039871
        RLOGD("SIM_REFRESH_DONE: SIM detected! Setup Dynamic-SBP for RIL:%d", rid);
        setupDynamicSBP(rid);
    } else if (strStartsWith(s, "+ESIMS: 0,5")) {
        if (isCdmaLteDcSupport()) {
            RLOGD("SIM Access Profile On");
            setPropertyCt3g(0, rid);
        }
    } else if (strStartsWith(s, "+ESIMS: 2,0")) {
        if (isCdmaLteDcSupport()) {
            property_set(PROPERTY_ICCID_SIM[rid], "");
            RLOGD("Reset %s to null", PROPERTY_ICCID_SIM[rid]);
        }
    } else if (strStartsWith(s, "+ESIMS: 0,19")) {
        //+ESIMS: 0,19: SPR IMEI Lock
        RLOGD("IMEI lock is activated");
        aIs_sim_off_by_IMEI_lock[rid] = true;
        RLOGD("aIs_sim_off_by_IMEI_lock[%d] value:%d", rid, aIs_sim_off_by_IMEI_lock[rid]);
        RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0, rid);
    }
    onSimInsertChangedForData(rid, s);
}
//MTK-START: ISIM
//MTK-START Support Multi-Application
int queryIccApplicationChannel(int appId, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int err;
    int resData[2];
    char *line, *cmd = NULL;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    /* Query if the channel has been opened on the indicated application
     * AT+ESIMAPP=<application_id>[, <mode>]
     * <mode> = 0 (query)
     * success: +ESIMAPP: <application_id>, <channel_id>
     * fail: ERROR
     */

    asprintf(&cmd, "AT+ESIMAPP=%d, 0", appId);

    err = at_send_command_singleline(cmd, "+ESIMAPP:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    if (err < 0 || NULL == p_response) {
        RLOGE("queryIccApplicationChannel query application fail");
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        RLOGD("p_response = %d /n", at_get_cme_error(p_response) );
        case CME_SUCCESS:
            if (err < 0 || p_response->success == 0) {
                goto error;
            } else {
                line = p_response->p_intermediates->line;

                err = at_tok_start(&line);
                if(err < 0) goto error;

                err = at_tok_nextint(&line, &resData[0]);
                if(err < 0) goto error;

                err = at_tok_nextint(&line, &resData[1]);
                if(err < 0) goto error;

                if (resData[0] != appId)  {
                    RLOGE("queryIccApplicationChannel: query ERROR, Response AP not equals request AP");
                    goto error;
                } else {
                    RLOGD("queryIccApplicationChannel: (socket %d) APP has been initialized, App is %d, Session is %d ", rid, resData[0], resData[1]);
                    isim_session_info[rid].appId = resData[0];
                    isim_session_info[rid].session = resData[1];

                }
                at_response_free(p_response);
                p_response = NULL;
            }
            return resData[1];
        case CME_UNKNOWN:
            break;
        default:
            LOGD("queryIccApplicationChannel query application fail");
    }

error:
    if (NULL != p_response) {
        at_response_free(p_response);
        p_response = NULL;
    }
    LOGE("queryIccApplicationChannel Error");
    return 0;
}

int turnOnIsimApplication(RIL_Token t, RIL_SOCKET_ID rid) {
    ATResponse *p_response = NULL;
    int err = RIL_E_SUCCESS;
    int id = -1;
    char *line, *cmd = NULL;

    if (aid_info_array[rid][AID_ISIM].aid_len <= 0) {
        RLOGE("Not get ISIM AID yet");
        err = RIL_E_GENERIC_FAILURE;
        goto error;
    }

    // Open application in case of the application has not been initialzed
    asprintf(&cmd, "AT+ESIMAPP=%d", 0);

    err = at_send_command_singleline(cmd, "+ESIMAPP:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    if (err < 0 || NULL == p_response) {
        goto error;
    }

    switch (at_get_cme_error(p_response)) {
        RLOGD("p_response = %d /n", at_get_cme_error(p_response) );
        case CME_SUCCESS:
            break;
        case CME_UNKNOWN:
            break;
        default:
            goto error;
    }

    if (err < 0 || p_response->success == 0) {
        goto error;

    } else {
        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if(err < 0) goto error;

        err = at_tok_nextint(&line, &id);
        if(err < 0) goto error;

        err = at_tok_nextint(&line, &isim_session_info[rid].session);
        if(err < 0) goto error;

        if (id != 0)  {
            RLOGE("requestOpenIccApplication: ERROR, Response AP not equals request AP");
            goto error;
        } else {
            isim_session_info[rid].appId = 0;
            RLOGD("requestOpenIccApplication: App is %d, Session is %d ", id, isim_session_info[rid].session);
        }
        at_response_free(p_response);
    }
    return RIL_E_SUCCESS;
error:
    RLOGE("requestOpenIccApplication Fail");
    if(p_response != NULL)
        at_response_free(p_response);
    return RIL_E_GENERIC_FAILURE;
}

void requestOpenIccApplication(void *data, size_t datalen, RIL_Token t)
{
    ATResponse *p_response = NULL;
    int appId, err;
    int resData[2];
    char *line, *cmd = NULL;
    int sessionId = 0;

    RIL_SIM_UNUSED_PARM(datalen);

    appId = ((int *)data)[0];

    // Query if the channel has been opened on the indicated application
    sessionId = queryIccApplicationChannel(appId, t);
    if (sessionId > 0) {
        resData[0] = appId;
        resData[1] = sessionId;
        RIL_onRequestComplete(t, RIL_E_SUCCESS, resData, 2*sizeof(int));
        return;
    }

    // Open application in case of the application has not been initialzed
    asprintf(&cmd, "AT+ESIMAPP=%d", appId);

    err = at_send_command_singleline(cmd, "+ESIMAPP:", &p_response, SIM_CHANNEL_CTX);
    free(cmd);
    if (err < 0 || NULL == p_response) {
        LOGE("requestOpenIccApplication Fail");
         goto error;
    }

    switch (at_get_cme_error(p_response)) {
        LOGD("p_response = %d /n", at_get_cme_error(p_response) );
        case CME_SUCCESS:
            break;
        case CME_UNKNOWN:
            LOGD("p_response: CME_UNKNOWN");
            break;
        default:
            goto error;
    }

    if (err < 0 || p_response->success == 0) {
        goto error;

    } else {
        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if(err < 0) goto error;

        err = at_tok_nextint(&line, &resData[0]);
        if(err < 0) goto error;

        err = at_tok_nextint(&line, &resData[1]);
        if(err < 0) goto error;

        if (resData[0] != appId)  {
            LOGD("requestOpenIccApplication: ERROR, Response AP not equals request AP");
            goto error;
        } else {
            LOGD("requestOpenIccApplication: App is %d, Session is %d ", resData[0], resData[1]);
            RIL_onRequestComplete(t, RIL_E_SUCCESS, resData, 2*sizeof(int));
        }
        at_response_free(p_response);
    }
      return;

error:
    LOGD("requestOpenIccApplication Error");

    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    if(p_response != NULL)
        at_response_free(p_response);
}

void onSessionIdChanged(const char *urc, RIL_SOCKET_ID rid)
{
    int err = 0;
    int resData[2];
    char *line = (char*) urc;
    char *pMcc = NULL, *pMnc = NULL;
    char* propTemp = NULL;
    char* prop = NULL;
    char* pMccMnc = NULL;
    int appId = -1, session = -1;

    if (rid < RIL_SOCKET_1 || rid >= RIL_SOCKET_NUM) {
        goto error;
    }

    //+ESIMAPP: <ApplicationId>, <SessionId>, ''mcc", "mnc"
    RLOGD("onSessionIdChanged receive %s", urc);
    err = at_tok_start(&line);
    if (err < 0) goto error;

    if (strlen(urc) > 0) {
        err = at_tok_nextint((char **)&line, &appId);
        if(err < 0) goto error;

        err = at_tok_nextint((char **)&line, &session);
        if(err < 0) goto error;

        err = at_tok_nextstr((char **)&line, &pMcc);
        if(err < 0) goto error;

        err = at_tok_nextstr((char **)&line, &pMnc);
        if(err < 0) goto error;

        if (UICC_APP_ISIM == appId) {
            isim_session_info[rid].appId = UICC_APP_ISIM;
            isim_session_info[rid].session = session;
        } else {
            if (UICC_APP_USIM == appId || UICC_APP_SIM == appId) {
                asprintf(&propTemp, "%s", PROPERTY_MCC_MNC);
            } else if (UICC_APP_CSIM == appId || UICC_APP_RUIM == appId) {
                asprintf(&propTemp, "%s", PROPERTY_MCC_MNC_CDMA);
            }
            if (propTemp != NULL) {
                if (rid == RIL_SOCKET_1) {
                    asprintf(&prop, "%s", propTemp);
                } else {
                    asprintf(&prop, "%s.%c", propTemp, rid + '0');
                }
                asprintf(&pMccMnc, "%s%s", pMcc, pMnc);
                property_set(prop, pMccMnc);
                /// Notify CC to refresh emergency list @{
                RLOGD("[%s][%d] Notify CC to refresh ecc list", __FUNCTION__, rid);
                onMccMncIndication(rid, pMccMnc);
                /// @}
                free(propTemp);
                free(prop);
                free(pMccMnc);
            } else {
                RLOGD("onSessionIdChanged ignore %d", appId);
            }
        }
    } else {
        return;
    }

    return;

error:
    RLOGE("onSessionIdChanged Error: %d", err);

}

ISIM_Status getIsimStatus(RIL_SOCKET_ID rid, int sessionId)
{

    ATResponse *p_response = NULL;
    int err;
    int ret;
    int sId, mode;
    char *cpinLine;
    char *cpinResult;
    char *cmd = NULL;
    SimPinCount retryCounts;

    // JB MR1, it will request sim status after receiver iccStatusChangedRegistrants,
    // but MD is off in the mean time, so it will get the exception result of CPIN.
    // For this special case, handle it specially.
    // check md off and sim inserted status, then return the result directly instead of request CPIN.
    // not insert: return SIM_ABSENT, insert: return SIM_NOT_READY or USIM_NOT_READY
    if (s_md_off) {
        int slot = (1 << getMappingSIMByCurrentMode(rid));
        RLOGI("getSIMStatus s_md_off: %d slot: %d", s_md_off, slot);
        if (!(slot & sim_inserted_status)) {
            ret = ISIM_ABSENT;
            goto done;
        } else {
            ret = ISIM_NOT_READY;
            goto done;
        }
    }

//MTK-START [mtkXXXXX][120208][APLS00109092] Replace "RIL_UNSOL_SIM_MISSING in RIL.java" with "acively query SIM missing status"
    //if (getRadioState(rid) == RADIO_STATE_OFF || getRadioState(rid) == RADIO_STATE_UNAVAILABLE) {
    if (getRadioState(rid) == RADIO_STATE_UNAVAILABLE) {
        ret = ISIM_NOT_READY;
        goto done;
    }

//MTK-END [mtkXXXXX][120208][APLS00109092] Replace "RIL_UNSOL_SIM_MISSING in RIL.java" with "acively query SIM missing status"

    asprintf(&cmd, "AT+EPIN=%d,0,0", sessionId);

    err = at_send_command_singleline(cmd, "+EPIN:", &p_response, getChannelCtxbyProxy());
    free(cmd);

    if (err != 0) {
        if (err == AT_ERROR_INVALID_THREAD) {
            ret = ISIM_BUSY;
        } else {
            ret = ISIM_NOT_READY;
        }
        goto done;
    }

    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;
        case CME_SIM_BUSY:
            RLOGD("getSIMStatus: CME_SIM_BUSY");
            ret = ISIM_BUSY;
            goto done;
            break;
        case CME_SIM_NOT_INSERTED:
        case CME_SIM_FAILURE:
            ret = ISIM_ABSENT;
            goto done;
            break;

        default:
            ret = ISIM_NOT_READY;
            goto done;
    }

    cpinLine = p_response->p_intermediates->line;
    err = at_tok_start (&cpinLine);

    if (err < 0) {
        ret = ISIM_NOT_READY;
        goto done;
    }

    err = at_tok_nextint(&cpinLine, &sId);
    if (err < 0 || sessionId != sId) {
        ret = ISIM_NOT_READY;
        goto done;
    }

    err = at_tok_nextint(&cpinLine, &mode);
    if (err < 0 || mode != 0) {
        ret = ISIM_NOT_READY;
        goto done;
    }

    err = at_tok_nextstr(&cpinLine, &cpinResult);
    if (err < 0) {
        ret = ISIM_NOT_READY;
        goto done;
    }

    if (0 == strcmp (cpinResult, "SIM PIN")) {
        ret = ISIM_PIN;
        goto done;
    } else if (0 == strcmp (cpinResult, "SIM PUK")) {
        ret = ISIM_PUK;
        goto done;
    } else if (0 != strcmp (cpinResult, "READY"))  {
        /* we're treating unsupported lock types as "sim absent" */
        ret = ISIM_ABSENT;
        goto done;
    }

    cpinResult = NULL;
    ret = ISIM_READY;


done:
    if(p_response != NULL)
        at_response_free(p_response);

    return ret;
}

int getIccApplicationStatus(RIL_CardStatus_v8 **pp_card_status, RIL_SOCKET_ID rid, int sessionId) {
    static RIL_AppStatus app_status_array[] = {
        // ISIM_ABSENT = 0
        { RIL_APPTYPE_UNKNOWN, RIL_APPSTATE_UNKNOWN, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // ISIM_NOT_READY = 1
        { RIL_APPTYPE_ISIM, RIL_APPSTATE_DETECTED, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // ISIM_READY = 2
        { RIL_APPTYPE_ISIM, RIL_APPSTATE_READY, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // ISIM_PIN = 3
        { RIL_APPTYPE_ISIM, RIL_APPSTATE_PIN, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
        // ISIM_PUK = 4
        { RIL_APPTYPE_ISIM, RIL_APPSTATE_PUK, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_BLOCKED, RIL_PINSTATE_UNKNOWN },
        // ISIM_BUSY = 5
        { RIL_APPTYPE_UNKNOWN, RIL_APPSTATE_UNKNOWN, RIL_PERSOSUBSTATE_READY,
          NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
        // ISIM_PERM_BLOCKED = 6 // PERM_DISABLED
        { RIL_APPTYPE_ISIM, RIL_APPSTATE_DETECTED, RIL_PERSOSUBSTATE_UNKNOWN,
          NULL, NULL, 0, RIL_PINSTATE_ENABLED_PERM_BLOCKED, RIL_PINSTATE_UNKNOWN },
    };
    RIL_CardState card_state;
    int num_apps;

    RLOGD("getIccApplicationStatus: (%d, %d) entering ", sessionId, rid);

    ISIM_Status sim_status = ISIM_ABSENT;
    int count = 0;

    do{
       sim_status = getIsimStatus(rid, sessionId);
       if (ISIM_BUSY == sim_status)
       {
           sleepMsec(200);
           count++;     //to avoid block; if the busy time is too long; need to check modem.
           if(count == 30)
           {
                RLOGE("Error in getSIM Status");
                sim_status = ISIM_NOT_READY; //to avoid exception in RILD
                break;
           }
       }
    }while(ISIM_BUSY == sim_status);
    if (sim_status == ISIM_ABSENT) {
        return RIL_E_GENERIC_FAILURE;
    }


    // Allocate and initialize base card status.
    RIL_CardStatus_v8 *p_card_status;
    AidInfo *pAidInfo = NULL;
    p_card_status = *pp_card_status;

    p_card_status->num_applications++;
    p_card_status->ims_subscription_app_index = 1;
    p_card_status->applications[p_card_status->ims_subscription_app_index] = app_status_array[sim_status];
    pAidInfo = getAidInfo(rid, AID_ISIM);
    p_card_status->applications[p_card_status->ims_subscription_app_index].aid_ptr = pAidInfo->aid;
    p_card_status->applications[p_card_status->ims_subscription_app_index].app_label_ptr =
            pAidInfo->appLabel;

    return RIL_E_SUCCESS;
}

void requestGetIccApplicationStatus(void *data, size_t datalen, RIL_Token t)
{
    RIL_CardStatus_v8 *p_card_status;
    char *p_buffer;
    int buffer_size;
    int sessionId;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    RIL_SIM_UNUSED_PARM(datalen);

    sessionId = ((int *)data)[0];

    int result = getCardStatus(&p_card_status,rid);
    if (result != RIL_E_SUCCESS) {
        p_buffer = NULL;
        buffer_size = 0;
        goto finish;
    }

    result = getIccApplicationStatus(&p_card_status,rid,sessionId);
    if (result == RIL_E_SUCCESS) {
        p_buffer = (char *)p_card_status;
        buffer_size = sizeof(*p_card_status);
    } else {
        p_buffer = NULL;
        buffer_size = 0;
    }

finish:
    RIL_onRequestComplete(t, result, p_buffer, buffer_size);
    free(p_card_status);
}
//MTK-END Support Multi-Application
//MTK-END

void requestSetSimCardPower(void *data, size_t datalen, RIL_Token t) {
    int mode = ((int *)data)[0];
    ATResponse *p_response = NULL;
    int err;
    int ret = RIL_E_SUCCESS;
    char* cmd = NULL;

    RIL_SIM_UNUSED_PARM(datalen);

    LOGD("requestSetSimCardPower: mode = %d", mode);

    asprintf(&cmd, "AT+ESIMPOWER=%d", mode);

    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    if (err < 0 || NULL == p_response || at_get_cme_error(p_response) != CME_SUCCESS) {
        LOGE("requestSetSimCardPower AT+ESIMPOWER=1 Fail, e= %d", err);
        ret = RIL_E_GENERIC_FAILURE;
    }
    if (p_response != NULL) {
        at_response_free(p_response);
        p_response = NULL;
    }
    free(cmd);
    RIL_onRequestComplete(t, ret, NULL, 0);
}

// SIM power [Start]
void requestSetSimPower(void *data, size_t datalen, RIL_Token t) {
    int mode = ((int *)data)[0];
    ATResponse *p_response = NULL;
    int err;
    int ret = RIL_E_SUCCESS;

    LOGD("requestSetSimPower: mode = %d", mode);

    RIL_SIM_UNUSED_PARM(datalen);

    switch (mode) {
        case 0: // Power off
            // TODO: not support sim power off currently
            break;
        case 1: // Power on
            // TODO: not support sim power on currently
            break;
        case 2: // Rest SIM
            err = at_send_command_singleline("AT+EBTSAP=0", "+EBTSAP:", &p_response, SIM_CHANNEL_CTX);
            if (err < 0 || NULL == p_response || at_get_cme_error(p_response) != CME_SUCCESS) {
                LOGE("requestSetSimPower AT+EBTSAP=0 Fail , e= %d", err);
                ret = RIL_E_GENERIC_FAILURE;
                goto done;
            }
            at_response_free(p_response);
            p_response = NULL;

            err = at_send_command("AT+EBTSAP=1", &p_response, SIM_CHANNEL_CTX);

            if (err < 0 || NULL == p_response || at_get_cme_error(p_response) != CME_SUCCESS) {
                LOGE("requestSetSimPower AT+EBTSAP=1 Fail, e= %d", err);
                ret = RIL_E_GENERIC_FAILURE;
                if (err == AT_ERROR_RADIO_UNAVAILABLE) {
                    ret = RIL_E_RADIO_NOT_AVAILABLE;
                    LOGD("requestSetSimPower() IVSR fail due to modem not available");
                }
                goto done;
            }

            at_response_free(p_response);
            p_response = NULL;
            break;
    }

done:
    if (p_response != NULL) {
        at_response_free(p_response);
    }
    RIL_onRequestComplete(t, ret, NULL, 0);

    LOGD("requestSetSimPower ret:%d", ret);
}
// SIM power [End]

void resetSimForCt3g(RIL_SOCKET_ID rid) {
    char* propMccMnc = NULL;
    char* propMccMncCdma = NULL;
    RLOGD("resetSimForCt3g rid %d", rid);

    property_set(PROPERTY_RIL_CT3G_ROAMING[rid], "1");
    property_set(PROPERTY_RIL_CT3G_ROAMING2[rid], "1");
    property_set(PROPERTY_RIL_UICC_TYPE[rid], NULL);
    property_set(PROPERTY_RIL_FULL_UICC_TYPE[rid], NULL);
    RIL_UNSOL_RESPONSE(RIL_LOCAL_GSM_UNSOL_CARD_TYPE_NOTIFY , "", 0, rid);
    // +ESIMAPP URC. Reset mcc mnc property.
    if (rid == RIL_SOCKET_1) {
        asprintf(&propMccMnc, "%s", PROPERTY_MCC_MNC);
        asprintf(&propMccMncCdma, "%s", PROPERTY_MCC_MNC_CDMA);
    } else {
        asprintf(&propMccMnc, "%s.%c", PROPERTY_MCC_MNC, rid + '0');
        asprintf(&propMccMncCdma, "%s.%c", PROPERTY_MCC_MNC_CDMA, rid + '0');
    }
    property_set(propMccMnc, "");
    property_set(propMccMncCdma, "");
    free(propMccMnc);
    free(propMccMncCdma);
    requestSimReset(rid);
}

void requestSimReset(RIL_SOCKET_ID rid)
{
    ATResponse *p_response = NULL;
    char *line;
    int ret;
    //int simId3G = RIL_get3GSIM();
    int pivot = 1;
    int pivotSim;;
    int err;
    err = at_send_command_singleline("AT+ESIMS", "+ESIMS:", &p_response, getChannelCtxbyProxy());

    if (err < 0 || p_response->success == 0) {
        // assume radio is off
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &ret);
    if (err < 0) goto error;

    if (ret == 1) {
        if (sim_inserted_status == -1)
            sim_inserted_status = NO_SIM_INSERTED;

        pivotSim = pivot << rid;
        sim_inserted_status = sim_inserted_status | pivotSim;
    } else if (sim_inserted_status == -1) {
        sim_inserted_status = NO_SIM_INSERTED;
    }

    LOGD("[RIL%d]AT+ESIMS return %d sim_inserted_status = %d", rid+1 , ret, sim_inserted_status );

    at_response_free(p_response);
    return;

error:
    LOGE("[RIL%d]AT+ESIMS return ERROR", rid+1 );
    at_response_free(p_response);
}

void requestSimInsertStatus(RIL_SOCKET_ID rid)
{
    ATResponse *p_response = NULL;
    int err;
    char *line;
    int ret;
    //int simId3G = RIL_get3GSIM();
    int pivot = 1;
    int pivotSim;

    err = at_send_command_singleline("AT+ESIMS?", "+ESIMS:", &p_response, getChannelCtxbyProxy());

    if (err < 0 || p_response->success == 0) {
        // assume radio is off
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &ret);
    if (err < 0) goto error;

    if (ret == 1) {
        if (sim_inserted_status == -1)
            sim_inserted_status = NO_SIM_INSERTED;

        pivotSim = pivot << rid;
        sim_inserted_status = sim_inserted_status | pivotSim;
    } else if (sim_inserted_status == -1) {
        sim_inserted_status = NO_SIM_INSERTED;
    }

    LOGD("[RIL%d]AT+ESIMS? return %d sim_inserted_status = %d", rid, ret, sim_inserted_status);
    at_response_free(p_response);
    return;
error:
    LOGE("[RIL%d]AT+ESIMS? return ERROR", rid);
    at_response_free(p_response);
}

// MTK-START: SIM GBA
void requestGeneralSimAuth(void *data, size_t datalen, RIL_Token t)
{
    RIL_SimAuthStructure *auth;

    ATResponse *p_response = NULL;
    int err = 0;
    char* cmd = NULL;
    char *line;
    RIL_SIM_IO_Response sr;
    int ret = RIL_E_GENERIC_FAILURE;
    char *param1 = NULL;
    char *param2 = NULL;

    RIL_SIM_UNUSED_PARM(datalen);

    auth = (RIL_SimAuthStructure *) data;

    memset(&sr, 0, sizeof(sr));

    if (auth == NULL) {
        LOGE("requestGeneralSimAuth invalid arg");
        ret = RIL_E_INVALID_ARGUMENTS;
        goto error;
    }
    param1 = (auth->param1 == NULL) ? "" : auth->param1;
    param2 = (auth->param2 == NULL) ? "" : auth->param2;

    if(auth->mode == 0) {
        // mode 0 is represent AKA authentication mode
        if (auth->sessionId == 0) {
            // Session id is equal to 0, for backward compability, we use old AT command
            if (param2 != NULL && strlen(param2) > 0) {
                // There is param2 means that caller except to use USIM AUTH
                asprintf(&cmd, "AT+EAUTH=\"%s\",\"%s\"", param1, param2);
            } else {
                // There is no param2 means that caller except to use SIM AUTH
                asprintf(&cmd, "AT+EAUTH=\"%s\"", param1);
            }
        } else {
            // Session id is not equal to 0, means we can use new AT command
            asprintf(&cmd, "AT+ESIMAUTH=%d,%d,\"%s%s\"", auth->sessionId, auth->mode, param1, param2);
        }
    } else if (auth->mode == 1) {
        // mode 1 is represent GBA authentication mode
        asprintf(&cmd, "AT+ESIMAUTH=%d,%d,\"%02x%s%s\"", auth->sessionId, auth->mode, auth->tag, param1, param2);
    }

    if (auth->mode == 0 && auth->sessionId == 0) {
        err = at_send_command_singleline(cmd, "+EAUTH:", &p_response, SIM_CHANNEL_CTX);
    } else {
        err = at_send_command_singleline(cmd, "+ESIMAUTH:", &p_response, SIM_CHANNEL_CTX);
    }
    free(cmd);

    if (err < 0 || NULL == p_response) {
        LOGE("requestGeneralSimAuth Fail");
         goto error;
    }

    switch (at_get_cme_error(p_response)) {
        RLOGD("p_response = %d /n", at_get_cme_error(p_response) );
        case CME_SUCCESS:
            break;
        case CME_UNKNOWN:
        RLOGE("p_response: CME_UNKNOWN");
            break;
        default:
            goto error;
    }


    if (err < 0 || p_response->success == 0) {
        goto error;
    } else {

        line = p_response->p_intermediates->line;
        err = at_tok_start(&line);
        if(err < 0) goto error;

        err = at_tok_nextint(&line, &sr.sw1);
        if(err < 0) goto error;

        err = at_tok_nextint(&line, &sr.sw2);
        if(err < 0) goto error;

        if(at_tok_hasmore(&line)){
            err = at_tok_nextstr(&line, &(sr.simResponse));
            if(err < 0) goto error;
            RLOGD("requestGeneralSimAuth response len = %zu %02x, %02x", strlen(sr.simResponse), sr.sw1, sr.sw2);
        } else {
            RLOGD("No response data");
        }
        RIL_onRequestComplete(t, RIL_E_SUCCESS, &sr, sizeof(sr));
        at_response_free(p_response);
    }
    return;

error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}
// MTK-END

static void configModemStatus(RIL_SOCKET_ID rid) {
    int typeCount = 0;
    int cardTypeSim = 0x01;
    int cardTypeUsim = 0x02;
    int cardTypeRuim = 0x04;
    int cardTypeCsim = 0x08;
    char cardType[PROPERTY_VALUE_MAX] = {0};
    property_get("vendor.gsm.ril.fulluicctype", cardType, "");
    LOGD("mtk ril configModeStatus cardType is:%s", cardType);

    char* type = cardType;
    char* out;
    int err = 0;
    while (at_tok_hasmore(&type)) {
        err = at_tok_nextstr(&type, &out);
        if (err < 0) {
            continue;
        }
        LOGD("mtk ril configModeStatus out is:%s", out);
        if (strcmp("SIM", out) == 0) {
            typeCount|= cardTypeSim;
            continue;
        } else if (strcmp("USIM", out) == 0) {
            typeCount|= cardTypeUsim;
            continue;
        } else if (strcmp("RUIM", out) == 0) {
            typeCount|= cardTypeRuim;
            continue;
        } else if (strcmp("CSIM", out) == 0) {
            typeCount|= cardTypeCsim;
            continue;
        }
    }
    LOGD("mtk ril configModeStatus typeCount is:%d", typeCount);

    int invalidType = 0;
    int modemStatus = 0;
    int remoteSimProtocol = 0;
    char *cmd;

    if (typeCount == 0) {
        // no card
        modemStatus = 1;
        remoteSimProtocol = 1;
    } else if ((typeCount & cardTypeRuim) == 0 && (typeCount & cardTypeCsim) == 0) {
        // GSM only
        modemStatus = 0;
        remoteSimProtocol = 1;
    } else if (((typeCount & cardTypeSim) == 0 && (typeCount & cardTypeUsim) == 0)) {
        // CDMA only
        modemStatus = 1;
        remoteSimProtocol = 1;
    } else if ((typeCount & cardTypeUsim) == cardTypeUsim && (typeCount & cardTypeCsim) == cardTypeCsim) {
        // CT LTE
        modemStatus = 2;
        remoteSimProtocol = 1;
    } else {
        //other case, may not happen!
        invalidType = 1;
    }
    asprintf(&cmd, "AT+EMDSTATUS=%d, %d", modemStatus, remoteSimProtocol);
    LOGD("mtk ril configModeStatus modemStatus:%d   remoteSimProtocol:%d", modemStatus, remoteSimProtocol);
    if (invalidType == 0) {
        at_send_command(cmd, NULL, getChannelCtxbyProxy());
    }
    free(cmd);
}

static int isCdmaCard(RIL_SOCKET_ID rid) {
    LOGD("isCdmaCard RID :%d  isCDMADetect[rid] : %d", rid, isCDMADetect[rid]);
    return (isCDMADetect[rid] == 0 || isCDMADetect[rid] == 1 || isCDMADetect[rid] == 3);
}


void sendATWithRetry(int times, int interval,
        const char* at_cmd, RILChannelCtx* p_channel) {
    ATResponse *p_response = NULL;
    int err = 0;
    int retry = 0;

    assert(at_cmd && p_channel);

    do {
        err = at_send_command(at_cmd, &p_response, p_channel);
        LOGD("%s(): times: %d, interval: %d, at_cmd: %s, retry: %d.",
                __FUNCTION__, times, interval, at_cmd, retry);
        if (!isATCmdRspErr(err, p_response)) {
            LOGD("%s(): at_cmd: %s, %d, %s.",
                    __FUNCTION__, at_cmd,
                    p_response->success, p_response->finalResponse);
            break;
        }
        usleep(1000 * interval);
        // Avoid memory leak
        if (p_response) {
            at_response_free(p_response);
            p_response = NULL;
        }
    } while (++retry < times);

    if (p_response) {
        at_response_free(p_response);
    }
}

/**
 * Switch RUIM card to SIM or switch SIM to RUIM.
 */
void requestSwitchCardType(void *data, size_t datalen, RIL_Token t) {
    char* cmd;
    ATResponse *p_response = NULL;
    int err = 0;
    const int RETRY_TIMES = 20;
    const int RETRY_INTERVAL = 1 * 1000;
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));

    RIL_SIM_UNUSED_PARM(datalen);

    // switch UTK/STK mode.
    SwitchStkUtkModeParam *param = (SwitchStkUtkModeParam*)malloc(sizeof(SwitchStkUtkModeParam));
    assert(param != NULL);
    param->mode = ((int *)data)[0];
    param->rid = rid;
    LOGD("requestSwitchCardType(), param->mode=%d, rid=%d", param->mode, param->rid);
    switchStkUtkMode(param);

    // Send cfun
    err = asprintf(&cmd, "AT+CFUN=0");
    LOGD("requestSwitchCardType(), send command %s.", cmd);
    // err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    sendATWithRetry(RETRY_TIMES, RETRY_INTERVAL, cmd, SIM_CHANNEL_CTX);
    free(cmd);

    resetSimForCt3g(getRILIdByChannelCtx(getRILChannelCtxFromToken(t)));
    // Send eboot
    err = asprintf(&cmd, "AT+EBOOT=1");
    LOGD("requestSwitchCardType(), send command %s.", cmd);
    err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
    free(cmd);

    // Reset the radio.
    if (err >= 0 && p_response != NULL && p_response->success != 0) {
        at_response_free(p_response);
        p_response = NULL;
        err = asprintf(&cmd, "AT+CFUN=4");
        LOGD("requestSwitchCardType(), send command %s.", cmd);
        // err = at_send_command(cmd, &p_response, SIM_CHANNEL_CTX);
        sendATWithRetry(RETRY_TIMES, RETRY_INTERVAL, cmd, SIM_CHANNEL_CTX);
        free(cmd);
    }
    setRadioState(RADIO_STATE_OFF, rid);
    at_response_free(p_response);
    LOGD("requestSwitchCardType(), err=%d.", err);
    if (err < 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
}

/// M: OP01 6M @{
void cacheCardTypeUrc(const char *s, RIL_SOCKET_ID rid)
{
    int simId = getMappingSIMByCurrentMode(rid);

    if (simId < 0 || simId >= MAX_SIM_COUNT) {
        return;
    }

    if (s != NULL) {
        int len = strlen(s);
        isTestSim[simId] = -1;
        g_eusim[simId] = calloc(1, (len + 1) * sizeof(char));
        strncpy(g_eusim[simId], s, (len + 1) * sizeof(char));
        LOGD("[op016m] cache eusim: %s", g_eusim[simId]);
    }
}

void clearCardTypeUrc(int simId) {
    if (g_eusim[simId] != NULL) {
        free(g_eusim[simId]);
        g_eusim[simId] = NULL;
    }
}

static void handleOp016mCardType(RIL_SOCKET_ID rid)
{
    int simId = getMappingSIMByCurrentMode(rid);

    if (simId < 0 || simId >= MAX_SIM_COUNT) {
        return;
    }

    LOGD("[op016m] handleOp016mCardType,slot:%d, cache is:%s", simId, g_eusim[simId]);

    if (g_eusim[simId] == NULL) {
        return;
    }

    handleCardTypeUrc(g_eusim[simId], rid);
    clearCardTypeUrc(simId);
}

void modifyCdmaCardType(int simId)
{
    char *p_cur = g_eusim[simId];
    if (p_cur == NULL) {
        return;
    }

    while (*p_cur != '\0' && *p_cur != ',') {
        p_cur++;
    }

    if (*p_cur == ',') {
        p_cur++;
    }

    while (*p_cur != '\0' && *p_cur == ' ') {
        p_cur++;
    }

    if (*p_cur != '\0') {
        *p_cur = '2';
    }

    LOGD("[op016m] modifyCdmaCardType: %s", g_eusim[simId]);
}

int hasCdmaCapability(const char *s)
{
    int value = -1;
    int len = strlen(s);
    char **pline;
    char *line = NULL;
    char *cmd = NULL;

    cmd = line = (char *)malloc((len + 1) * sizeof(char));
    assert(cmd != NULL);
    strncpy(line, s, (len + 1) * sizeof(char));

    pline = &line;

    LOGD("[op016m] hasCdmaCapability: %s", line);
    if ((at_tok_start(&line) >= 0) && (at_tok_nextint(&line, &value) >= 0)) {
        LOGD("[op016m] at_tok_start: %d", value);
        if (value != 1) {
            free(cmd);
            return 0;
        }

        if ((at_tok_hasmore(&line) >= 0) && (at_tok_nextint(&line, &value) >= 0)) {
            LOGD("[op016m] at_tok_hasmore: %d", value);
            if (value != 2) {
                free(cmd);
                return 1;
            }
        }
    }

    LOGD("[op016m] non cdma capability");

    free(cmd);
    return 0;
}

static const char sOp01TestSim[][6] = {
    "46000",
    "00101",
    "00102",
    "46001",
    "00431" //R&S PCT 6.1.1.4a
};

static int sGetImsiCounter[MAX_SIM_COUNT] = {0};
#define VALUE_GET_ISMI_MAX 3

static void getImsi(void *param)
{
    TimedCallbackParam* param_t = (TimedCallbackParam *)param;
    RILChannelCtx *p_cctx = getChannelCtxbyProxy();
    ATResponse *p_response = NULL;
    int simId = getMappingSIMByCurrentMode(param_t->rid);
    char mccmnc[6] = {'\0'};
    int err;
    int num = sizeof(sOp01TestSim) / sizeof(sOp01TestSim[0]);
    int i = 0;
    int testFlag = 0;

    RLOGD("[op016m] [Socket %d] getImsi: entering", param_t->rid);
    if (simId < 0 || simId >= MAX_SIM_COUNT) {
        return;
    }

    err = at_send_command_numeric("AT+CIMI", &p_response, p_cctx);

    if (err < 0 || p_response->success == 0) {
        RLOGD("[op016m] getImsi err:%d", err);
        if (sGetImsiCounter[simId] < VALUE_GET_ISMI_MAX) {
            sGetImsiCounter[simId] ++;
            RIL_requestProxyTimedCallback(getImsi,
                    (void *)param, &TIMEVAL_PIN_SIM, p_cctx->id, "getImsi");
            goto done;
        }
        RLOGE("[op016m] getImsi error, do not handle EUSIM");
        goto error;
    }

    RLOGD("[op016m] Imsi: %.6s", p_response->p_intermediates->line);
    strncpy(mccmnc, p_response->p_intermediates->line, 5);

    for (; i< num; i++) {
        if (strcmp(mccmnc, sOp01TestSim[i]) == 0) {
            testFlag = 1;
        }
    }
    RLOGD("[op016m] getImsi, testFlag: %d, %d", testFlag, simId);

    if (testFlag) {
        modifyCdmaCardType(simId);
    }
    handleOp016mCardType(param_t->rid);

error:
    sGetImsiCounter[simId] = 0;

    if (param_t->urc != NULL) {
        free(param_t->urc);
    }
    free(param_t);

done:
    at_response_free(p_response);
}

void onOp016mCardDetected(RIL_SOCKET_ID rid)
{
    int simId = getMappingSIMByCurrentMode(rid);
    char erasecdma[PROPERTY_VALUE_MAX] = {0};

    if (simId < 0 || simId >= MAX_SIM_COUNT) {
        return;
    }

    LOGD("[op016m] onOp016mCardDetected, slot: %d, cache is:%s", simId, g_eusim[simId]);

    if (g_eusim[simId] == NULL) {
        return;
    }

    if (eraseCdmaCapabilityMode() == 1) {
        modifyCdmaCardType(simId);
        handleOp016mCardType(rid);

        return;
    }

    if (isTestSim[simId] == 1) {
        if (eraseCdmaCapabilityMode() != 5) {
            // RILChannelCtx *p_cctx = getChannelCtxbyProxy();
            TimedCallbackParam* param = (TimedCallbackParam*)malloc(sizeof(TimedCallbackParam));
            param->rid = rid;
            param->urc = NULL;
            LOGD("[op016m] onOp016mCardDetected, getImsi for %d", simId);

            RIL_requestProxyTimedCallback(getImsi,
                    (void *)param, &TIMEVAL_PIN_SIM, getRILChannelId(RIL_SIM, rid), "getImsi");
        } else {
            modifyCdmaCardType(simId);
            handleOp016mCardType(rid);
        }
    } else {
        handleOp016mCardType(rid);
    }
}

/**
 * 1: ignore if it is test sim
 * 5: disable query imsi
 */
int eraseCdmaCapabilityMode()
{
    char erasecdma[PROPERTY_VALUE_MAX] = {0};

    property_get("persist.vendor.op016m.cdma.erase", erasecdma, "-1");
    return atoi(erasecdma);
}

void checkPin(void *param)
{
    TimedCallbackParam* param_t = (TimedCallbackParam *)param;
    int simId = getMappingSIMByCurrentMode(param_t->rid);
    RILChannelCtx *p_cctx = getChannelCtxbyProxy();
    ATResponse *p_response = NULL;
    int err;
    int ret = -1;
    int cmeError;
    char *cpinLine;
    char *cpinResult;

    RLOGD("[op016m] checkPin for OP01 6M: entering %d", simId);
    if (simId < 0 || simId >= MAX_SIM_COUNT) {
        return;
    }

    err = at_send_command_singleline("AT+CPIN?", "+CPIN:", &p_response, p_cctx);

    do {
        if (err < 0 || p_response->success == 0) {
            RLOGD("[op016m] checkPin err:%d", err);
            if (at_get_cme_error(p_response) == CME_SIM_BUSY) {
                ret = SIM_BUSY;
            }
            break;
        }

        cpinLine = p_response->p_intermediates->line;
        err = at_tok_start(&cpinLine);
        if (err < 0) {
            ret = SIM_NOT_READY;
            break;
        }

        err = at_tok_nextstr(&cpinLine, &cpinResult);
        if (0 == strcmp(cpinResult, "SIM PIN")) {
            ret = USIM_PIN;
            break;
        } else if (0 == strcmp(cpinResult, "SIM PUK")) {
            ret = USIM_PUK;
            break;
        }
    } while (0);

    if (ret == USIM_PIN || ret == USIM_PUK) {
        // If locked, just report EUSIM context.
        handleOp016mCardType(param_t->rid);
    } else if (ret == SIM_BUSY && isTestSim[simId] == -1) {
        RIL_requestProxyTimedCallback(checkPin,
            param, &TIMEVAL_PIN_SIM, p_cctx->id, "checkPin");
        goto done;
    }

    if (param_t->urc != NULL) {
        free(param_t->urc);
    }
    free(param_t);

done:
    at_response_free(p_response);
}

void checkPinState(const char *s, RIL_SOCKET_ID rid)
{
    if (hasCdmaCapability(s)) {
        // RILChannelCtx *p_cctx = getChannelCtxbyProxy();
        TimedCallbackParam* param = (TimedCallbackParam*)malloc(sizeof(TimedCallbackParam));
        assert(param != NULL);
        param->rid = rid;
        param->urc = NULL;
        LOGD("[op016m] checkPinState, getIsim for %d", rid);

        RIL_requestProxyTimedCallback(checkPin,
            (void *)param, &TIMEVAL_PIN_SIM, getRILChannelId(RIL_SIM, rid), "checkPin");
    }
}
/// @}

static int isOp096mSupport() {
    int isCtaSupport = 0;
    char cta[PROPERTY_VALUE_MAX] = {0};
    char optr[PROPERTY_VALUE_MAX] = {0};
    char seg[PROPERTY_VALUE_MAX] = {0};

    property_get("ro.vendor.mtk_cta_support", cta, "0");
    isCtaSupport = atoi(cta);
    property_get("persist.vendor.operator.optr", optr, "");
    property_get("persist.vendor.operator.seg", seg, "");
    LOGD("isOp096mSupport isC2KSupport: %d, isCtaSupport: %d, optr: %s, seg: %s",
            RatConfig_isC2kSupported(), isCtaSupport, optr, seg);
    return ((RatConfig_isC2kSupported() == 1) && (isCtaSupport != 1)
            && ((strcmp("OP09", optr) == 0) ? 1 : 0)
            && ((strcmp("SEGC", seg) == 0) ? 1 : 0));
}

/// M: OP01 6M @{
int isOp016mSupport() {
    char optr[PROPERTY_VALUE_MAX] = {0};

    property_get("persist.vendor.operator.optr", optr, "");

    return (RatConfig_isC2kSupported() == 1) && ((strcmp("OP01", optr) == 0) ? 1 : 0);
}
/// @}

// External SIM [START]
int vsim_plug_in_out[MAX_SIM_COUNT] = {0,0,0,0};

extern int isExternalSimOnlySlot(RIL_SOCKET_ID rid);

int isPersistVsim()
{
    int rid = RIL_SOCKET_1;
    int persist = 0;
    char persist_vsim_inserted_prop[PROPERTY_VALUE_MAX] = {0};

    for (rid = RIL_SOCKET_1; rid < RIL_SOCKET_NUM; rid++) {
        getMSimProperty(rid, "persist.vendor.radio.external.sim", persist_vsim_inserted_prop);

        if (atoi(persist_vsim_inserted_prop) > 0) {
            persist = 1;
            break;
        }
    }
    LOGD("[VSIM] isPersistVsim is %d.", persist);
    return persist;
}

int isVsimEnabled()
{
    int rid = RIL_SOCKET_1;
    // Since swtich will disable the channels, AP can't receive URC from Modem.
    // It will cause AP can't handle SIM power off request and fail to do capability
    // switch then NE occurred.
    // Reference issue: [ALPS02399092].
    int enabled = 0;

    for (rid = RIL_SOCKET_1; rid < RIL_SOCKET_NUM; rid++) {
        if (isVsimEnabledByRid(rid) == 1) {
            enabled = 1;
            break;
        }
    }

    LOGD("[VSIM] isVsimEnabled is %d.", enabled);

    return enabled;
}

int isVsimEnabledByRid(int rid)
{
    int enabled = 0;
    char vsim_enabled_prop[PROPERTY_VALUE_MAX] = {0};
    char vsim_inserted_prop[PROPERTY_VALUE_MAX] = {0};

    getMSimProperty(rid, "vendor.gsm.external.sim.enabled", vsim_enabled_prop);
    getMSimProperty(rid, "vendor.gsm.external.sim.inserted", vsim_inserted_prop);

    if ((atoi(vsim_enabled_prop) > 0 && atoi(vsim_inserted_prop) > 0) || isExternalSimOnlySlot(rid)) {
        enabled = 1;
    }

    LOGD("[VSIM] isVsimEnabled rid:%d is %d.", rid, enabled);

    return enabled;
}

void setVsimPlugInOutEvent(int slotId, int state) {
    vsim_plug_in_out[slotId] = state;
}

int getVsimPlugInOutEvent(int slotId) {
    return vsim_plug_in_out[slotId];
}
// External SIM [END]

/*
*This method will be called before init to release the wakelock
*which has been called before reboot.
*/
void releaseExtraWakeLock() {
    release_wake_lock("sim_hot_plug");
}

int queryAppTypeByAid(char* aid) {
    int appTypeId = -1;

    if (NULL == aid) {
        return appTypeId;
    }
    if (strncmp(aid, "A0000000871002", 14) == 0) {
        // USIM
        appTypeId = UICC_APP_USIM; // USIM
    } else if (strncmp(aid, "A0000000871004", 14) == 0) {
        // ISIM
        appTypeId = UICC_APP_ISIM;
    } else if (strncmp(aid, "A0000003431002", 14) == 0) {
        // CSIM
        appTypeId = UICC_APP_CSIM;
    } else {
        RLOGE("Not support the aid %s", aid);
        appTypeId = -1;
    }

    return appTypeId;
}

bool serviceActivationStatus(int fileId, int appTypeId, RIL_Token t) {
    int index = -1, err = 0, support = 0;
    char *cmd = NULL;
    char *line;
    ATResponse *p_response = NULL;
    bool result = true;

    switch(fileId) {
        case 28486: // 0x6F46 EF_SPN
            // SPN
            if (appTypeId == UICC_APP_USIM) {
                index = 19;
            } else if (appTypeId == UICC_APP_SIM) {
                index = 17;
            }
            break;
        case 28617: // 0x6FC9 EF_MBI
            // MBI. Check Mailbox Dialling Numbers
            if (appTypeId == UICC_APP_USIM) {
                index = 47;
            } else if (appTypeId == UICC_APP_SIM) {
                index = 53;
            }
            break;
        case 28618: // 0x6FCA EF_MWIS
            // MWIS
            if (appTypeId == UICC_APP_USIM) {
                index = 48;
            } else if (appTypeId == UICC_APP_SIM) {
                index = 54;
            }
            break;
    }

    if (index >= 0) {
        // Use AT+ESSTQ=<app>,<service> to query service table
        // 0:  Service not supported
        // 1:  Service supported
        // 2:  Service allocated but not activated
        asprintf(&cmd, "AT+ESSTQ=%d,%d", appTypeId, index);
        err = at_send_command_singleline(cmd, "+ESSTQ:", &p_response,
                SIM_CHANNEL_CTX);
        free(cmd);
        // The same as AOSP. 0 - Available & Disabled, 1-Available & Enabled, 2-Unavailable.
        if (err < 0 || NULL == p_response || p_response->success == 0) {
            RLOGE("Fail to query service table");
            goto done;
        } else {
            line = p_response->p_intermediates->line;

            err = at_tok_start(&line);
            if (err < 0) goto done;

            err = at_tok_nextint(&line, &support);
            if (err < 0) goto done;

            if (support != 1) {
                result = false;
            }
        }
        RLOGD("serviceActivationStatus result: %d", result);
    }

done:
    p_response = NULL;
    return result;
}

void onHandleEsimInd(const char *s, RIL_SOCKET_ID rid) {
    int indEvent;
    int err;
    char* line = (char*)s;
    int applist = 0;
    RLOGD("onHandleEsimInd[%d]", rid);

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &indEvent);
    if (err < 0) goto error;

    switch (indEvent) {
        case 1:
            RLOGD("Notify SIM initialization done, slotId:%d", rid);
            // Currently capability switch without RADIO_UNAVAILABLE, but modem SIM task
            // still has to do SIM initialization again.
            // After modem SIM task initialize SIM done, AP will get the URC "+ESIMINIT".
            // We have to notify the capability switch module the event and the proxy channels
            // will unlock
            if (bDssNoResetSupport == TRUE) {
                simSwitchNotifySIMInitDone(rid);
            } else {
                RLOGE("Not support DSS without reset");
            }
            break;
        case 2:
            RLOGD("Notify uicc app list, slotId:%d", rid);
            // +ESIMIND: 2, <uicc_app_list>
            // uicc_app_list = is_csim_exist | is_usim_exist | is_isim_exist (currently isim always 0)
            // is_usim_exist:2 is_csim_exist:4 (is_csim_exist | is_usim_exist): 6
            // For icc card uicc_app_list:0
            err = at_tok_nextint(&line, &applist);
            if (err < 0) {
                goto error;
            }
            RIL_UNSOL_RESPONSE(RIL_LOCAL_GSM_UNSOL_ESIMIND_APPLIST,  &applist, sizeof(int), rid);
            return;
        default:
            RLOGD("Not support the SIM indication event %d", indEvent);
            break;
    }

error:
    RLOGE("handleSimIndication, Invalid parameters");
}

void onSmlStatusChanged(const char *urc, RIL_SOCKET_ID rid) {
    bool is_cap_changed = false;
    char *line = (char*) urc;
    char *property_lock_policy = NULL;
    char *property_lock_state = NULL;
    char *property_service = NULL;
    char *property_valid_card = NULL;
    char property_old_service[PROPERTY_VALUE_MAX] = { 0 };
    char property_old_valid_card[PROPERTY_VALUE_MAX] = { 0 };
    unsigned char slot_lock_rule = 0;
    unsigned char capability_rule = 0;
    int err = 0;
    int rule = 0;
    int sub_rule = 0;
    int lock_state = 0;
    int service = 0;
    int valid_card = 0;
    int old_service = 0;
    int old_valid_info = 0;
    int sml_status[4] = {0};

    if (!smlSbpUrcReported) {
        smlSbpUrcReported = true;
    }

    if (isSimSlotLockSupport() == 0) {
        RLOGE("onSmlStatusChanged: not support SML so return!");
        return;
    }
    if (rid < RIL_SOCKET_1 || rid >= RIL_SOCKET_NUM) {
        goto error;
    }

    //+ESMLSTATUS: <rule>, <subRule>, <lockState>, <sercice>,<validCard>
    RLOGD("onSmlStatusChanged receive %s", urc);
    err = at_tok_start(&line);
    if (err < 0) goto error;

    if (strlen(urc) > 0) {
        err = at_tok_nextint((char **)&line, &rule);
        if(err < 0) goto error;

        err = at_tok_nextint((char **)&line, &sub_rule);
        if(err < 0) goto error;

        err = at_tok_nextint((char **)&line, &lock_state);
        if(err < 0) goto error;

        err = at_tok_nextint((char **)&line, &service);
        if(err < 0) goto error;

        err = at_tok_nextint((char **)&line, &valid_card);
        if(err < 0) goto error;

        RLOGD("onSmlStatusChanged rule: %d, sub_rule: %d, lock_state: %d, service: %d, \
            valid_card: %d", rule, sub_rule, lock_state, service, valid_card);

        //Parse and update lock policy rule into sml_status[0], currently MD only uses 4 bits of sub_rule
        slot_lock_rule = sub_rule & SML_BITMASK_SLOT_LOCK_CONTEXT;
        capability_rule = (sub_rule & SML_BITMASK_INVALID_SIM_CAP) >> 1;

        if (SML_NO_RULE == rule) {
            if ((SML_SLOT_LOCK_USE_SAME_CONTEXT == slot_lock_rule) &&
                 (SML_INVALID_SIM_FULL_SERVICE == capability_rule)) {
                sml_status[0] = SML_SLOT_LOCK_POLICY_NONE;
            }
        } else if (SML_SIM1_SLOT_LOCK_ONLY == rule) {
            if ((SML_SLOT_LOCK_USE_SAME_CONTEXT == slot_lock_rule) &&
                (SML_INVALID_SIM_FULL_SERVICE == capability_rule)) {
                sml_status[0] = SML_SLOT_LOCK_POLICY_ONLY_SLOT1;
            }
        } else if (SML_SIM2_SLOT_LOCK_ONLY == rule) {
            if ((SML_SLOT_LOCK_USE_SAME_CONTEXT == slot_lock_rule) &&
                (SML_INVALID_SIM_FULL_SERVICE == capability_rule)) {
                sml_status[0] = SML_SLOT_LOCK_POLICY_ONLY_SLOT2;
            }
        } else if (SML_ALL_SIM_LOCK == rule) {
            if ((SML_SLOT_LOCK_NOT_USE_SAME_CONTEXT == slot_lock_rule) &&
                (SML_INVALID_SIM_FULL_SERVICE == capability_rule)) {
                sml_status[0] = SML_SLOT_LOCK_POLICY_ALL_SLOTS_INDIVIDUAL;
            }
        } else if (SML_LINK_LOCK_WITH_SIM1_SLOT == rule) {
            if ((SML_SLOT_LOCK_USE_SAME_CONTEXT == slot_lock_rule) &&
                (SML_INVALID_SIM_FULL_SERVICE == capability_rule)) {
                sml_status[0] = SML_SLOT_LOCK_POLICY_LK_SLOT1;
            }
        } else if (SML_LINK_LOCK_WITH_SIM2_SLOT == rule) {
            if ((SML_SLOT_LOCK_USE_SAME_CONTEXT == slot_lock_rule) &&
                (SML_INVALID_SIM_FULL_SERVICE == capability_rule)) {
                sml_status[0] = SML_SLOT_LOCK_POLICY_LK_SLOT2;
            }
        } else if (SML_LINK_LOCK_WITH_ANY_SLOT == rule) {
            if ((SML_SLOT_LOCK_USE_SAME_CONTEXT == slot_lock_rule) &&
                (SML_INVALID_SIM_CS_ONLY == capability_rule)) {
                sml_status[0] = SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_CS;
            } else if ((SML_SLOT_LOCK_USE_SAME_CONTEXT == slot_lock_rule) &&
                (SML_INVALID_SIM_FULL_SERVICE == capability_rule)){
                sml_status[0] = SML_SLOT_LOCK_POLICY_LK_SLOTA;
            }
        } else if (SML_SLOTA_RESTRICT_INVALID_VOICE == rule) {
            if ((SML_SLOT_LOCK_USE_SAME_CONTEXT == slot_lock_rule) &&
                (SML_INVALID_VOICE == capability_rule)) {
                sml_status[0] = SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_VOICE;
            }
        } else if (SML_SLOTA_RESTRICT_REVERSE == rule) {
            if ((SML_SLOT_LOCK_USE_SAME_CONTEXT == slot_lock_rule) &&
                (SML_INVALID_SIM_FULL_SERVICE == capability_rule)) {
                sml_status[0] = SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_REVERSE;
            }
        } else if (SML_RESTRICT_INVALID_ECC_FOR_VALID_NO_SERVICE == rule) {
            if ((SML_SLOT_LOCK_USE_SAME_CONTEXT == slot_lock_rule) &&
                (SML_INVALID_SIM_FULL_SERVICE == capability_rule)) {
                sml_status[0] =
                    SML_SLOT_LOCK_POLICY_LK_SLOTA_RESTRICT_INVALID_ECC_FOR_VALID_NO_SERVICE;
            }
        } else if (SML_ALL_SIM_LOCK_AND_RSU_VZW == rule) {
            if ((SML_SLOT_LOCK_NOT_USE_SAME_CONTEXT == slot_lock_rule) &&
                (SML_INVALID_SIM_FULL_SERVICE == capability_rule)) {
                sml_status[0] = SML_SLOT_LOCK_POLICY_ALL_SLOTS_INDIVIDUAL_AND_RSU_VZW;
            }
        } else if (SML_LEGACY_MODE == rule) {
            if ((SML_SLOT_LOCK_USE_SAME_CONTEXT == slot_lock_rule) &&
                (SML_INVALID_SIM_FULL_SERVICE == capability_rule)) {
                sml_status[0] = SML_SLOT_LOCK_POLICY_LEGACY;
            }
        } else {
            RLOGE("onSmlStatusChanged Error: not support lock policy!");
            return;
        }

        //Update other slot lock related status
        sml_status[1] = lock_state;
        sml_status[2] = service - 1;
        sml_status[3] = valid_card - 1;

        //Check whether the service and valid info changed
        property_get(PROPERTY_SIM_SLOT_LOCK_SERVICE_CAPABILITY[rid], property_old_service, "-1");
        property_get(PROPERTY_SIM_SLOT_LOCK_CARD_VALID[rid], property_old_valid_card, "-1");
        old_service = atoi(property_old_service);
        old_valid_info = atoi(property_old_valid_card);
        if (old_service != sml_status[2] || old_valid_info != sml_status[3]) {
            is_cap_changed = true;
            RLOGD("onSmlStatusChanged: capability changed!");
        }

        //Set SIM slot lock related properties
        asprintf(&property_lock_policy, "%d", sml_status[0]);
        property_set(PROPERTY_SIM_SLOT_LOCK_POLICY, property_lock_policy);
        asprintf(&property_lock_state, "%d", sml_status[1]);
        property_set(PROPERTY_SIM_SLOT_LOCK_STATE, property_lock_state);
        asprintf(&property_service, "%d", sml_status[2]);
        property_set(PROPERTY_SIM_SLOT_LOCK_SERVICE_CAPABILITY[rid], property_service);
        asprintf(&property_valid_card, "%d", sml_status[3]);
        property_set(PROPERTY_SIM_SLOT_LOCK_CARD_VALID[rid], property_valid_card);

        RIL_onUnsolicitedResponse(
            RIL_UNSOL_SIM_SLOT_LOCK_POLICY_NOTIFY, sml_status, sizeof(sml_status), rid);

        if (is_cap_changed) {
            setSimStatusChanged(rid);
        }

        free(property_lock_policy);
        free(property_lock_state);
        free(property_service);
        free(property_valid_card);
        return;
    } else {
        RLOGE("onSmlStatusChanged Error: urc length is inValid!");
        return;
    }
error:
    RLOGE("onSmlStatusChanged Error: %d", err);
}

void onSmlSbpConfigureCheck() {
    if (!smlSbpConfigureChecked) {
        smlSbpConfigureChecked = true;
        if (smlSbpUrcReported) {
            if (isSimSlotLockSupport() == 0) {
                RLOGE("MD supports SBP SML, AP does not support, please help configure rightly!");
                // assert(0); // customer enable it to avoid maliciously falsify, like change image
            }
        } else {
             if (isSimSlotLockSupport() == 1) {
                RLOGE("MD does not support SBP SML, AP supports, please help configure rightly!");
                // assert(0); // customer enable it to avoid maliciously falsify, like change image
            }
        }
    }
}

static void requestSimGetEfdir(void *data, size_t datalen, RIL_Token t) {
    ATResponse *p_response = NULL;
    int err = 0;
    char *line, *cmd = NULL;
    char *efdir;
    RIL_Errno ret = RIL_E_GENERIC_FAILURE;

    // AT+CUAD[=<option>]
    // <option>: integer type.
    // 0:   no parameters requested in addition to <response>.
    // 1    include <active_application>.
    // +CUAD: <response>[,<active_application>[,<AID>]]
    // <response>: string type in hexadecimal character format.The response is the content of the
    // EFDIR.
    // <active_application>: integer type.
    // 0    no SIM or USIM active.
    // 1    active application is SIM.
    // 2    active application is USIM, followed by <AID>.
    // <AID>: string type in hexadecimal character format. AID of active USIM.

    asprintf(&cmd, "AT+CUAD=0");
    err = at_send_command_singleline(cmd, "+CUAD:", &p_response, SIM_CHANNEL_CTX);
    RLOGD("requestSimGetEfdir(), send command %s", cmd);
    free(cmd);
    if (err < 0 || NULL == p_response) {
        goto error;
    }

    if (p_response->success == 0) {
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0) {
        goto error;
    }

    err = at_tok_nextstr(&line, &efdir);
    if (err < 0) {
        goto error;
    }
    RLOGD("requestSimGetEfdir(), efdir: %s", efdir);

    RIL_onRequestComplete(t, RIL_E_SUCCESS, efdir, ((efdir == NULL) ? 0 : strlen(efdir)));
    at_response_free(p_response);
    return;

error:
    RLOGE("requestSimGetEfdir(), fail to get efdir!");

    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
}

/// M: GSMA TS.27 13.3.7 @{
bool isUiccCLFSupport() {
    char uiccCLF[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.vendor.mtk_uicc_clf", uiccCLF, "0");
    return (atoi(uiccCLF) == 1);
}
/// M: GSMA TS.27 13.3.7 @}

char* stringToUpper(char *str) {
    char *p = str;
    for (; *p != '\0'; p++) {
        *p = toupper(*p);
    }
    return str;
}
