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
#include <dirent.h>
#include <stdlib.h>
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
#include <cutils/sockets.h>
#include <termios.h>
#include "hardware/ccci_intf.h"
#include <cutils/properties.h>
/// M: eMBMS feature
#include <ril_embms.h>

#include <hardware/ril/librilutils/proto/sap-api.pb.h>
#include "pb_decode.h"
#include "pb_encode.h"

#include <ril_callbacks.h>
#include <ril_wp.h>
#include <ril_data.h>
#include "usim_fcp_parser.h"
#include "sysenv_utils.h"
#include <mal.h>
#include <limits.h>

#ifdef HAVE_AEE_FEATURE
#include "aee.h"
#endif
#undef LOG_TAG
#define LOG_TAG "RIL"

#include <log/log.h>

#include <cutils/properties.h>

#include <ratconfig.h>
#include <time.h>
#include "ratconfig.h"

#define PROPERTY_RIL_SIM_READY              "vendor.ril.ready.sim"

//[New R8 modem FD]
#define PROPERTY_FD_SCREEN_ON_TIMER     "persist.vendor.radio.fd.counter"
#define PROPERTY_FD_SCREEN_ON_R8_TIMER  "persist.vendor.radio.fd.r8.counter"
#define PROPERTY_FD_SCREEN_OFF_TIMER    "persist.vendor.radio.fd.off.counter"
#define PROPERTY_FD_SCREEN_OFF_R8_TIMER    "persist.vendor.radio.fd.off.r8.counter"
#define PROPERTY_FD_ON_ONLY_R8_NETWORK  "persist.vendor.radio.fd.on.only.r8"
#define PROPERTY_RIL_FD_MODE    "vendor.ril.fd.mode"
/* FD related timer: units: 0.1 sec */
#define DEFAULT_FD_SCREEN_ON_TIMER "150"
#define DEFAULT_FD_SCREEN_ON_R8_TIMER "150"
#define DEFAULT_FD_SCREEN_OFF_TIMER "50"
#define DEFAULT_FD_SCREEN_OFF_R8_TIMER "50"
#define DEFAULT_FD_ON_ONLY_R8_NETWORK "0"

// max int: 2147483647
#define INT_MAX_LENGTH 11
static void onRequest(int request, void *data, size_t datalen, RIL_Token t, RIL_SOCKET_ID socket_id);
static void updateConnectionState(RIL_SOCKET_ID socketId, int isConnected);

#if defined(ANDROID_MULTI_SIM)
static void onSapRequest(int request, void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID socket_id);
#else
static void onSapRequest(int request, void *data, size_t datalen, RIL_Token t);
#endif

static RIL_RadioState currentState(RIL_SOCKET_ID rid);
static int onSupports(int requestCode);
static void onCancel(RIL_Token t);
static const char *getVersion();
static void sendSapResponseComplete(RIL_Token t, RIL_Errno ret, MsgId msgId, void *data);

static void syncModemProtocolCapability();
static void setDynamicMsimConfig();

void determineSSServiceClassFeature(RIL_SOCKET_ID rid, RILChannelCtx* p_channel);
static int isATCommandRawShouldBeHandled(const int request, char* s, size_t len);

extern const char *requestToString(int request);
extern void initRILChannels(void);
extern void initialCidTable();
extern int isATCmdRspErr(int err, const ATResponse *p_response);
extern void setRadioState(RIL_RadioState newState, RIL_SOCKET_ID rid);
extern RIL_RadioState getRadioState(RIL_SOCKET_ID rid);
extern void getLogicModemId();
extern void initNetworkMode(const RIL_SOCKET_ID rid, RILChannelCtx *p_channel);
extern void setStkServiceRunningFlag(RIL_SOCKET_ID rid, bool flag);
/// M: Ensure that channels have created and init AT command send to MD @{
int isInitDone = 0;
/// @}

// External SIM [Start]
#ifdef MTK_EXTERNAL_SIM_SUPPORT
extern void initVsimConfiguration(RIL_SOCKET_ID rid);
extern void requestSetAkaSim(RIL_SOCKET_ID rid);
extern void requestSwitchExternalSim(RIL_SOCKET_ID rid);
extern void queryModemVsimCapability(RIL_SOCKET_ID rid);
extern void initializeResetVsim(void *param);
extern int isVsimEnabledByRid(int rid);
extern int isVsimEnabled();
#endif
// External SIM [End]

/*** Static Variables ***/
static const RIL_RadioFunctions s_callbacks = {
    RIL_VERSION,
    onRequest,
    currentState,
    onSupports,
    onCancel,
    getVersion,
    updateConnectionState
};

#ifdef  RIL_SHLIB
const struct RIL_Env *s_rilenv;
#endif  /* RIL_SHLIB */

const struct RIL_Env *s_rilsapenv;
static const RIL_RadioFunctions s_sapcallbacks = {
    RIL_VERSION,
    onSapRequest,
    currentState,
    onSupports,
    onCancel,
    getVersion,
    updateConnectionState
};

static RIL_RadioState sState = RADIO_STATE_UNAVAILABLE;
static RIL_RadioState sState2 = RADIO_STATE_UNAVAILABLE;
static RIL_RadioState sState3 = RADIO_STATE_UNAVAILABLE;
static RIL_RadioState sState4 = RADIO_STATE_UNAVAILABLE;

static pthread_mutex_t s_state_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_state_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_init_mutex = PTHREAD_MUTEX_INITIALIZER;

static int s_port = -1;
static const char *s_device_path = NULL;
static int s_device_socket = 0;

static int s_device_range_begin = -1;
static int s_device_range_end = -1;

/* trigger change to this with s_state_cond */
static int s_closed = 0;

static int sEactsSupport = 0;

#ifdef  MTK_RIL
static const RIL_SOCKET_ID s_pollSimId[] = {RIL_SOCKET_1, RIL_SOCKET_2, RIL_SOCKET_3, RIL_SOCKET_4};
#endif  /* MTK_RIL */

//BEGIN mtk03923 [ALPS00061979 - MS Lost network after sending many DTMF]
extern int inCallNumber;
//END   mtk03923 [ALPS00061979 - MS Lost network after sending many DTMF]
extern int setPlmnListFormat(RIL_SOCKET_ID rid, int format);

static const struct timeval TIMEVAL_0 = { 0, 0 };

/* Commands pending in AT channel */
static pthread_mutex_t s_pendinglist_mutex = PTHREAD_MUTEX_INITIALIZER;
static int pendinglist[RIL_SUPPORT_CHANNELS] = { 0 };

bool bDssNoResetSupport = false;

int s_isSimSwitched = 0;
int s_targetSim = 0;
static int s_telephonyMode = -1;
extern int bPSBEARERSupport;

extern int s_md_off;
extern int s_main_loop;

extern int bCREGType3Support;
extern int bEopsSupport;

// common data @{
extern int pdnFailCauseSupportVer;
extern int pdnReasonSupportVer;
extern int pdnMdVersion;
// @}

//Add log level for ALPS01270573 reproduce
int mtk_ril_log_level = 0;

/* To check if new SS service class feature is supported or not */
int bNewSsServiceClassFeature = 0;

/* logic modem id
 **/
char s_logicalModemId[SIM_COUNT][MAX_UUID_LENGTH];

/* capability switch verion
 * value : 1 for legacy sim switch
 *         2 for no MD reset sim switch
 **/
int s_simSwitchVersion = 1;

/* check if under sim switching
 * value : 0 for idle state
 *         1 for under sim switching
 **/
int s_isSimSwitching = 0;

/* modem capability
 * value(bit mask) : RAF_GSM, RAF_UMTS, RAF_LTE...etc
 **/
int s_modemCapability[RIL_SOCKET_NUM] = {0};

int s_isUserLoad = 0;

/* modem off version
  * value: 0 for old version (send EPOF from GSM and C2K RILD)
  *          1 for new version (Send EPOF form GSM RILD only)
**/
int s_enhanceModemOffVersion = 0;
static int s_worldmode_activemode = 1;
extern void updateTplusWVersion(RIL_SOCKET_ID rid);


/// M: world mode switching, pending special URC. @{
static UrcList* pendedUrcList = NULL;

/// MAL interface URC support @{
typedef struct MalUrc{
    int type;
    int rid;
    char* line;
    struct MalUrc* nextUrc;
} MalUrc;

void notifyMalRawData(const char *s, const char *sms_pdu, void *pChannel);
void cacheRawUrcToMal();
static void *malUrcLoop(void *param);
MalUrc* initMalUrc(int type, int rid, char* line);
MalUrc* malUrcList = NULL;
pthread_mutex_t malUrcListMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t mal_urc_dispatch_thread = 0;
pthread_cond_t malUrcLlistNotEmptyCond;
/// @}

/*
 * If found the request in pendinglist, return list index else retrun RIL_SUPPORT_CHANNELS
 */
static int findPendingRequest(int request)
{
    int i;
    for (i = 0; i < getSupportChannels(); i++) {
        if (*(pendinglist + i) == request)
            return i;
    }

    return i;
}

static void setRequest(int request)
{
    pthread_mutex_lock(&s_pendinglist_mutex);
    /* find an empty slot */
    pendinglist[findPendingRequest(0)] = request;
    assert(i < getSupportChannels());
    pthread_mutex_unlock(&s_pendinglist_mutex);
}

static void resetRequest(int request)
{
    pthread_mutex_lock(&s_pendinglist_mutex);
    pendinglist[findPendingRequest(request)] = 0;
    assert(i < getSupportChannels());
    pthread_mutex_unlock(&s_pendinglist_mutex);
}

extern int RILcheckPendingRequest(int request)
{
    return (getSupportChannels()== findPendingRequest(request)) ? 0 : 1;
}

/** do post-AT+CFUN=1 initialization */
static void onRadioPowerOn(RIL_SOCKET_ID rid)
{
    const RIL_SOCKET_ID *p_rilId = &s_pollSimId[0];

    if (RIL_SOCKET_2 == rid) {
        p_rilId = &s_pollSimId[1];
    } else if (RIL_SOCKET_3 == rid) {
        p_rilId = &s_pollSimId[2];
    } else if (RIL_SOCKET_4 == rid) {
        p_rilId = &s_pollSimId[3];
    }

    pollSIMState((void *)p_rilId);
}

/** do post- SIM ready initialization */
static void onSIMReady(RIL_SOCKET_ID rid __unused)
{
}

/*** Callback methods from the RIL library to us ***/

/**
 * Call from RIL to us to make a RIL_REQUEST
 *
 * Must be completed with a call to RIL_onRequestComplete()
 *
 * RIL_onRequestComplete() may be called from any thread, before or after
 * this function returns.
 *
 * Will always be called from the same thread, so returning here implies
 * that the radio is ready to process another command (whether or not
 * the previous command has completed).
 */
static void onRequest(int request, void *data, size_t datalen, RIL_Token t, RIL_SOCKET_ID socket_id __unused)
{
    /// M: wait initialization done @{
    if (!isInitDone &&
            (request == RIL_REQUEST_VSIM_NOTIFICATION ||
            request == RIL_REQUEST_VSIM_OPERATION)) {
        RLOGI("init undone : whitelist request");
    } else {
        while (!isInitDone) {
            usleep(100*1000);
            RLOGE("init undone");
        }
    }
    /// @}

    RIL_RadioState radioState = sState;

    if (RIL_CHANNEL_SET4_OFFSET <= RIL_queryMyChannelId(t)) {
        radioState = sState4;
    } else if (RIL_CHANNEL_SET3_OFFSET <= RIL_queryMyChannelId(t)) {
        radioState = sState3;
    } else if (RIL_CHANNEL_OFFSET <= RIL_queryMyChannelId(t)) {
        radioState = sState2;
    }
    RLOGD("onRequest: %s, datalen = %zu, radioState:%d", requestToString(request), datalen,
            radioState);

    if (s_md_off &&
        request != RIL_REQUEST_RADIO_POWER &&
        request != RIL_REQUEST_GET_RADIO_CAPABILITY &&
        request != RIL_REQUEST_DEVICE_IDENTITY &&
        request != RIL_REQUEST_GET_IMEI &&
        request != RIL_REQUEST_GET_IMEISV &&
        request != RIL_REQUEST_BASEBAND_VERSION &&
        request != RIL_REQUEST_GET_SIM_STATUS &&
        // External SIM [Start]
        request != RIL_REQUEST_VSIM_NOTIFICATION &&
        request != RIL_REQUEST_VSIM_OPERATION &&
        // External SIM [End]
        request != RIL_REQUEST_RESET_RADIO &&
        request != RIL_REQUEST_SET_RADIO_CAPABILITY &&
        request != RIL_REQUEST_CONFIG_MODEM_STATUS &&
        /// M: [C2K]Dynamic switch support. @{
        request != RIL_REQUEST_ENTER_RESTRICT_MODEM &&
        request != RIL_REQUEST_LEAVE_RESTRICT_MODEM &&
        /// @}
        request != RIL_REQUEST_GSM_DEVICE_IDENTITY &&
        request != RIL_REQUEST_MODEM_POWERON &&
        request != RIL_REQUEST_MODEM_POWEROFF &&
        // MTK-START: SIM GBA
        request != RIL_REQUEST_GENERAL_SIM_AUTH &&
        // MTK-END
        /// M: Deactivate Data Call for VoWiFi while turning off the modem. @{
        (!isWfcSupport() || request != RIL_REQUEST_DEACTIVATE_DATA_CALL) &&
        /// @}
        /// M: world mode switching, need pass special request. @{
        request != RIL_REQUEST_MODIFY_MODEM_TYPE &&
        request != RIL_LOCAL_REQUEST_STORE_MODEM_TYPE &&
        request != RIL_LOCAL_REQUEST_RELOAD_MODEM_TYPE &&
        request != RIL_LOCAL_REQUEST_SET_MODEM_THERMAL &&
        /// IMS:
        request != RIL_REQUEST_SET_IMS_ENABLE &&
        request != RIL_REQUEST_SET_VOLTE_ENABLE &&
        request != RIL_REQUEST_SET_WFC_ENABLE &&
        request != RIL_REQUEST_SET_IMS_VIDEO_ENABLE &&
        request != RIL_REQUEST_IMS_DEREG_NOTIFICATION &&
        /// @}
        request != RIL_REQUEST_SET_SS_PROPERTY &&
        request != RIL_REQUEST_SEND_USSD &&
        request != RIL_REQUEST_SEND_USSI &&
        request != RIL_REQUEST_CANCEL_USSD &&
        request != RIL_REQUEST_CANCEL_USSI &&
        request != RIL_REQUEST_SET_PHONEBOOK_READY &&
        request != RIL_LOCAL_GSM_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE &&
        request != RIL_REQUEST_ENABLE_MODEM &&
        request != RIL_REQUEST_RESTART_RILD &&
        !isATCommandRawShouldBeHandled(request, data, datalen)
    ) {
        RLOGD("MD off and ignore %s", requestToString(request));
        RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
        return;
    }

    /* Ignore all requests except RIL_REQUEST_GET_SIM_STATUS,RIL_REQUEST_DETECT_SIM_MISSING when RADIO_STATE_UNAVAILABLE. */
    // [ALPS00958313] Pass RADIO_POWERON when radio not available that is caused by muxd shutdown device before ccci ioctl.
    if (radioState == RADIO_STATE_UNAVAILABLE &&
        // External SIM [Start]
        request != RIL_REQUEST_VSIM_NOTIFICATION &&
        request != RIL_REQUEST_VSIM_OPERATION &&
        // External SIM [End]
        request != RIL_REQUEST_OEM_HOOK_RAW && //This is for ATCI
        request != RIL_REQUEST_OEM_HOOK_STRINGS &&
        request != RIL_REQUEST_GET_RADIO_CAPABILITY &&
        request != RIL_REQUEST_SET_RADIO_CAPABILITY &&
        request != RIL_REQUEST_RESET_RADIO &&
        request != RIL_REQUEST_ALLOW_DATA &&
        request != RIL_REQUEST_SET_PREFERRED_DATA_MODEM &&
        request != RIL_REQUEST_SET_PS_REGISTRATION &&
        request != RIL_LOCAL_GSM_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE &&
        /// M: [C2K][IR] Support SVLTE IR feature. @{
        request != RIL_REQUEST_SET_REG_SUSPEND_ENABLED &&
        request != RIL_REQUEST_RESUME_REGISTRATION &&
        request != RIL_LOCAL_REQUEST_RESET_SUSPEND &&
        /// M: [C2K][IR] Support SVLTE IR feature. @}
        request != RIL_REQUEST_CONFIG_MODEM_STATUS &&
        /// M: [C2K]Dynamic switch support. @{
        request != RIL_REQUEST_ENTER_RESTRICT_MODEM &&
        request != RIL_REQUEST_LEAVE_RESTRICT_MODEM &&
        /// @}
        request != RIL_REQUEST_GSM_DEVICE_IDENTITY &&
        /// M: Mode switch TRM feature. @{
        request != RIL_LOCAL_REQUEST_MODE_SWITCH_GSM_SET_TRM &&
        /// @}
        /// M: eMBMS
        request != RIL_LOCAL_REQUEST_EMBMS_AT_CMD &&
        /// @}
        request != RIL_REQUEST_SET_TRM &&
        request != RIL_REQUEST_MODEM_POWERON &&
        request != RIL_REQUEST_MODEM_POWEROFF &&
        request != RIL_REQUEST_SET_DATA_PROFILE &&
        request != RIL_REQUEST_SET_ECC_LIST &&
        /// M: world mode switching, need pass special request. @{
        request != RIL_REQUEST_MODIFY_MODEM_TYPE &&
        request != RIL_LOCAL_REQUEST_STORE_MODEM_TYPE &&
        request != RIL_LOCAL_REQUEST_RELOAD_MODEM_TYPE &&
        request != RIL_REQUEST_GET_HARDWARE_CONFIG &&
        !allowForWorldModeSwitching(request) &&
        /// @}
        // M: Data Framework - common part enhancement
        request != RIL_REQUEST_SYNC_DATA_SETTINGS_TO_MD &&
        /// M: IMS [Start]
        request != RIL_REQUEST_SET_IMS_ENABLE &&
        request != RIL_REQUEST_SET_VOLTE_ENABLE &&
        request != RIL_REQUEST_SET_WFC_ENABLE &&
        request != RIL_REQUEST_SET_IMS_VIDEO_ENABLE &&
        request != RIL_REQUEST_SET_WFC_PROFILE &&
        request != RIL_REQUEST_IMS_DEREG_NOTIFICATION &&
        request != RIL_REQUEST_IMS_REGISTRATION_STATE &&
        request != RIL_REQUEST_SET_IMS_REGISTRATION_REPORT &&
        /// M: IMS Provisioning. @{
        request != RIL_REQUEST_GET_PROVISION_VALUE &&
        request != RIL_REQUEST_SET_PROVISION_VALUE &&
        /// @}
        /// M: IMS [End]
        request != RIL_REQUEST_SIM_AUTHENTICATION &&
        // / M :STK start
        request != RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING &&
        request != RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE &&
        request != RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM &&
        request != RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE &&
        request != RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND &&
        request != RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS &&
        request != RIL_REQUEST_SET_STK_UTK_MODE &&
        // / M :STK end
        request != RIL_LOCAL_REQUEST_SET_MODEM_THERMAL &&
        request != RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER &&
        request != RIL_REQUEST_SHUTDOWN &&
        request != RIL_REQUEST_SET_PHONEBOOK_READY &&
        request != RIL_REQUEST_GET_PHONE_CAPABILITY &&
        request != RIL_REQUEST_SET_SS_PROPERTY &&
        request != RIL_REQUEST_SEND_USSD &&
        request != RIL_REQUEST_SEND_USSI &&
        request != RIL_REQUEST_CANCEL_USSD &&
        request != RIL_REQUEST_CANCEL_USSI &&
        request != RIL_REQUEST_RESTART_RILD
       ) {
        RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
        return;
    }

    /* Ignore all non-power requests when RADIO_STATE_OFF
     * (except RIL_REQUEST_GET_SIM_STATUS, RIL_REQUEST_DETECT_SIM_MISSING)
     */
    if (radioState == RADIO_STATE_OFF &&
        !(request == RIL_REQUEST_RADIO_POWER ||
          request == RIL_REQUEST_GET_SIM_STATUS ||
          request == RIL_REQUEST_DEVICE_IDENTITY ||
          request == RIL_REQUEST_GET_IMEI ||
          request == RIL_REQUEST_GET_IMEISV ||
          request == RIL_REQUEST_BASEBAND_VERSION ||
          request == RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE ||
          request == RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE ||
          request == RIL_REQUEST_SET_LOCATION_UPDATES ||
          request == RIL_REQUEST_GET_CELL_INFO_LIST ||
          request == RIL_REQUEST_WRITE_SMS_TO_SIM ||
          request == RIL_REQUEST_DELETE_SMS_ON_SIM ||
          request == RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG ||
          request == RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG ||
          request == RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION ||
          request == RIL_REQUEST_GSM_SET_BROADCAST_LANGUAGE ||
          request == RIL_REQUEST_GSM_GET_BROADCAST_LANGUAGE ||
          request == RIL_REQUEST_GET_GSM_SMS_BROADCAST_ACTIVATION ||
          request == RIL_REQUEST_GET_SMS_SIM_MEM_STATUS ||
          request == RIL_REQUEST_GET_SMSC_ADDRESS ||
          request == RIL_REQUEST_SET_SMSC_ADDRESS ||
          request == RIL_REQUEST_REPORT_SMS_MEMORY_STATUS ||
          request == RIL_REQUEST_SMS_ACKNOWLEDGE ||
          request == RIL_REQUEST_SMS_ACKNOWLEDGE_EX ||
          request == RIL_REQUEST_SCREEN_STATE ||
          request == RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER ||
          request == RIL_REQUEST_RESET_RADIO ||
          request == RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE ||
          request == RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND ||
          request == RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS ||
          request == RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM ||
          request == RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE||
          request == RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING ||
          request == RIL_REQUEST_SET_STK_UTK_MODE ||
          request == RIL_REQUEST_SET_TTY_MODE ||
          request == RIL_REQUEST_QUERY_TTY_MODE ||
          request == RIL_REQUEST_SET_MUTE ||
          request == RIL_REQUEST_GET_MUTE ||
          request == RIL_REQUEST_GET_CURRENT_CALLS ||
          request == RIL_REQUEST_OEM_HOOK_RAW ||
          request == RIL_REQUEST_OEM_HOOK_STRINGS ||
          request == RIL_REQUEST_SIM_IO ||
          request == RIL_REQUEST_GET_IMSI ||
          request == RIL_REQUEST_SIM_OPEN_CHANNEL ||
          request == RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC ||
          request == RIL_REQUEST_SIM_CLOSE_CHANNEL ||
          request == RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL ||
          request == RIL_REQUEST_SIM_GET_ATR ||
          // SIM power [Start]
          request == RIL_REQUEST_SET_SIM_POWER ||
          // SIM power [End]
          request == RIL_REQUEST_SET_SIM_CARD_POWER ||
          // MTK-START: SIM GBA
          request == RIL_REQUEST_GENERAL_SIM_AUTH ||
          // MTK-END
          // MTK-START: SIM ME LOCK
          request == RIL_REQUEST_ENTER_NETWORK_DEPERSONALIZATION ||
          request == RIL_REQUEST_QUERY_SIM_NETWORK_LOCK ||
          request == RIL_REQUEST_SET_SIM_NETWORK_LOCK ||
          // MTK-END
          // MTK-START: SIM TMO RSU
          request == RIL_LOCAL_REQUEST_GET_SHARED_KEY ||
          request == RIL_LOCAL_REQUEST_UPDATE_SIM_LOCK_SETTINGS ||
          request == RIL_LOCAL_REQUEST_GET_SIM_LOCK_INFO ||
          request == RIL_LOCAL_REQUEST_RESET_SIM_LOCK_SETTINGS ||
          request == RIL_LOCAL_REQUEST_GET_MODEM_STATUS ||
          // RJIL Sub lock start.
          request == RIL_REQUEST_GET_SUBLOCK_MODEM_STATUS ||
          request == RIL_REQUEST_UPDATE_SUBLOCK_SETTINGS ||
          // RJIL Sub lock End
          // MTK-END
          // External SIM [Start]
          request == RIL_REQUEST_VSIM_NOTIFICATION ||
          request == RIL_REQUEST_VSIM_OPERATION ||
          // External SIM [End]
          request == RIL_REQUEST_SIM_AUTHENTICATION ||
          request == RIL_REQUEST_GET_RADIO_CAPABILITY ||
          request == RIL_REQUEST_SET_RADIO_CAPABILITY ||
          request == RIL_REQUEST_ENTER_SIM_PIN ||
          request == RIL_REQUEST_ENTER_SIM_PUK ||
          request == RIL_REQUEST_ENTER_SIM_PIN2 ||
          request == RIL_REQUEST_ENTER_SIM_PUK2 ||
          request == RIL_REQUEST_CHANGE_SIM_PIN ||
          request == RIL_REQUEST_CHANGE_SIM_PIN2 ||
          request == RIL_REQUEST_QUERY_FACILITY_LOCK ||
          request == RIL_REQUEST_SET_FACILITY_LOCK ||
          request == RIL_REQUEST_ALLOW_DATA ||
          request == RIL_REQUEST_SET_PREFERRED_DATA_MODEM ||
          request == RIL_REQUEST_SET_PS_REGISTRATION ||
          request == RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE ||
          request == RIL_LOCAL_GSM_REQUEST_SET_ACTIVE_PS_SLOT ||
          request == RIL_LOCAL_GSM_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE ||
          request == RIL_REQUEST_START_LCE ||
          request == RIL_REQUEST_STOP_LCE ||
          request == RIL_REQUEST_PULL_LCEDATA ||
          request == RIL_REQUEST_SET_LINK_CAPACITY_REPORTING_CRITERIA ||
          request == RIL_LOCAL_GSM_REQUEST_SWITCH_CARD_TYPE ||
          /// M:[Network][C2K] add for band8 desense feature. @{
          (isSvlteSupport() && request == RIL_REQUEST_SET_BAND_MODE) ||
          /// M @}
          request == RIL_REQUEST_SET_REG_SUSPEND_ENABLED ||
          request == RIL_REQUEST_RESUME_REGISTRATION ||
          /// M:  EPDG feature. Update PS state from MAL
          request == RIL_LOCAL_REQUEST_MAL_PS_RGEGISTRATION_STATE ||
          request == RIL_REQUEST_CONFIG_MODEM_STATUS ||
          /// M: [Network][C2K] Set the SVLTE RAT mode. @{
          request == RIL_LOCAL_REQUEST_SET_SVLTE_RAT_MODE ||
          /// M: [Network][C2K] Set the SVLTE RAT mode. @}
          /// M: [Network][C2K] Set the init FDD TDD mode. @{
          request == RIL_LOCAL_REQUEST_SET_FDD_TDD_MODE ||
          /// M: [Network][C2K] Set the init FDD TDD mode. @}
          /// M: [C2K]Dynamic switch support. @{
          request == RIL_REQUEST_ENTER_RESTRICT_MODEM ||
          request == RIL_REQUEST_LEAVE_RESTRICT_MODEM ||
          /// @}
          request == RIL_REQUEST_GSM_DEVICE_IDENTITY ||
          request == RIL_LOCAL_REQUEST_CDMA_SMS_SPECIFIC_TO_GSM ||
          /// M: eMBMS
          request == RIL_LOCAL_REQUEST_EMBMS_AT_CMD ||
          /// @}
          /// M: Mode switch TRM feature. @{
          request == RIL_LOCAL_REQUEST_MODE_SWITCH_GSM_SET_TRM ||
          /// @}
          request == RIL_REQUEST_SET_INITIAL_ATTACH_APN ||
          /// M: IMS [Start]
          request == RIL_REQUEST_SET_IMS_ENABLE ||
          request == RIL_REQUEST_SET_VOLTE_ENABLE ||
          request == RIL_REQUEST_SET_WFC_ENABLE ||
          request == RIL_REQUEST_SET_IMS_VIDEO_ENABLE ||
          request == RIL_REQUEST_SET_WFC_PROFILE ||
          request == RIL_REQUEST_IMS_DEREG_NOTIFICATION ||
          request == RIL_REQUEST_IMS_REGISTRATION_STATE ||
          request == RIL_REQUEST_SET_IMS_REGISTRATION_REPORT ||
          /// M: IMS Provisioning. @{
          request == RIL_REQUEST_GET_PROVISION_VALUE ||
          request == RIL_REQUEST_SET_PROVISION_VALUE ||
          /// @}
          /// M: IMS [End]
          request == RIL_REQUEST_SET_TRM ||
          request == RIL_REQUEST_MODEM_POWERON ||
          request == RIL_REQUEST_MODEM_POWEROFF ||
          request == RIL_REQUEST_SET_DATA_PROFILE ||
          request == RIL_REQUEST_SET_PSEUDO_CELL_MODE ||
          request == RIL_REQUEST_SET_ECC_LIST ||
          /// M: CC: Switch antenna
          request == RIL_LOCAL_REQUEST_SWITCH_ANTENNA ||
          // PHB Start
          request == RIL_REQUEST_QUERY_PHB_STORAGE_INFO ||
          request == RIL_REQUEST_WRITE_PHB_ENTRY ||
          request == RIL_REQUEST_READ_PHB_ENTRY ||
          request == RIL_REQUEST_GET_PHB_STRING_LENGTH ||
          request == RIL_REQUEST_GET_PHB_MEM_STORAGE ||
          request == RIL_REQUEST_SET_PHB_MEM_STORAGE ||
          request == RIL_REQUEST_READ_PHB_ENTRY_EXT ||
          request == RIL_REQUEST_WRITE_PHB_ENTRY_EXT ||
          request == RIL_REQUEST_QUERY_UPB_CAPABILITY ||
          request == RIL_REQUEST_EDIT_UPB_ENTRY ||
          request == RIL_REQUEST_DELETE_UPB_ENTRY ||
          request == RIL_REQUEST_READ_UPB_GAS_LIST ||
          request == RIL_REQUEST_READ_UPB_GRP ||
          request == RIL_REQUEST_WRITE_UPB_GRP ||
          request == RIL_REQUEST_QUERY_UPB_AVAILABLE ||
          request == RIL_REQUEST_READ_EMAIL_ENTRY ||
          request == RIL_REQUEST_READ_SNE_ENTRY ||
          request == RIL_REQUEST_READ_ANR_ENTRY ||
          request == RIL_REQUEST_READ_UPB_AAS_LIST ||
          request == RIL_REQUEST_SET_PHONEBOOK_READY ||
          // PHB End
          // M: For SS, when VoWifi registered and TBCLIR support
          request == RIL_REQUEST_SET_CLIR ||
          request == RIL_REQUEST_SET_SS_PROPERTY ||
          request == RIL_REQUEST_SEND_USSD ||
          request == RIL_REQUEST_SEND_USSI ||
          request == RIL_REQUEST_CANCEL_USSD ||
          request == RIL_REQUEST_CANCEL_USSI ||
          // M: Data Framework - common part enhancement
          request == RIL_REQUEST_SYNC_DATA_SETTINGS_TO_MD ||
          // M: Data Framework - CC 33
          request == RIL_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE ||
          // M: Data Framework - keepalive
          request == RIL_REQUEST_START_KEEPALIVE ||
          request == RIL_REQUEST_STOP_KEEPALIVE ||
          /// M: world mode switching, need pass special request. @{
          request == RIL_REQUEST_MODIFY_MODEM_TYPE ||
          request == RIL_LOCAL_REQUEST_STORE_MODEM_TYPE ||
          request == RIL_LOCAL_REQUEST_RELOAD_MODEM_TYPE ||
          /// M: CC: Need to inform MD1 the current Emergency session status. @{
          request == RIL_LOCAL_REQUEST_EMERGENCY_REDIAL ||
          request == RIL_LOCAL_REQUEST_EMERGENCY_SESSION_BEGIN ||
          request == RIL_LOCAL_REQUEST_EMERGENCY_SESSION_END ||
          request == RIL_LOCAL_REQUEST_SET_HVOLTE_MODE ||
          /// @}
          request == RIL_LOCAL_REQUEST_EMERGENCY_DIAL ||
          // Verizon E911
          request == RIL_REQUEST_SET_ECC_MODE ||
          request == RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE ||
          request == RIL_REQUEST_GET_HARDWARE_CONFIG ||
          request == RIL_REQUEST_GET_ACTIVITY_INFO ||
          request == RIL_LOCAL_REQUEST_AT_COMMAND_WITH_PROXY ||
          /// @}
          // MD should responsive even radio off.
          // ril_nw responses when MD is off.
          request == RIL_REQUEST_VOICE_REGISTRATION_STATE ||
          request == RIL_REQUEST_DATA_REGISTRATION_STATE ||
          request == RIL_REQUEST_OPERATOR ||
          request == RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE ||
          request == RIL_REQUEST_SET_BAND_MODE ||
          request == RIL_REQUEST_GET_NEIGHBORING_CELL_IDS ||
          request == RIL_REQUEST_VOICE_RADIO_TECH ||
          request == RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE ||
          request == RIL_LOCAL_REQUEST_SET_MODEM_THERMAL ||
          request == RIL_REQUEST_GET_ALLOWED_CARRIERS ||
          request == RIL_REQUEST_SET_ALLOWED_CARRIERS ||
          request == RIL_REQUEST_SEND_DEVICE_STATE ||
          request == RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER ||
          // World Mode
          request == RIL_LOCAL_REQUEST_RESUME_WORLD_MODE ||
          request == RIL_REQUEST_SHUTDOWN ||
          request == RIL_LOCAL_REQUEST_SIM_GET_EFDIR ||
          /// M: [IR][C2K] Reset Suspend Mode @{
          request == RIL_LOCAL_REQUEST_RESET_SUSPEND ||
          /// @}
          // MTK-START: SIM SLOT LOCK
          request == RIL_REQUEST_ENTER_DEVICE_NETWORK_DEPERSONALIZATION ||
          // MTK-END
          request == RIL_REQUEST_GET_PHONE_CAPABILITY ||
          request == RIL_REQUEST_ENABLE_MODEM ||
          request == RIL_REQUEST_RESTART_RILD
         )) {
         /// M: [WFC]Wifi calling can be done even if the radio is unavailable.
         /// If WFC support then Epdg must be supported.
         if (isWfcSupport() &&
              (request == RIL_REQUEST_DIAL ||
               request == RIL_REQUEST_HANGUP ||
               request == RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND ||
               request == RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND ||
               request == RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE ||
               request == RIL_REQUEST_ANSWER ||
               request == RIL_REQUEST_CONFERENCE ||
               request == RIL_REQUEST_SEPARATE_CONNECTION ||
               request == RIL_REQUEST_LAST_CALL_FAIL_CAUSE ||
               request == RIL_REQUEST_DTMF ||
               request == RIL_REQUEST_DTMF_START ||
               request == RIL_REQUEST_DTMF_STOP ||
               request == RIL_REQUEST_HANGUP_ALL ||
               request == RIL_REQUEST_HANGUP_WITH_REASON ||
               request == RIL_REQUEST_FORCE_RELEASE_CALL ||
               request == RIL_REQUEST_SET_CALL_INDICATION ||
               request == RIL_REQUEST_HOLD_CALL ||
               request == RIL_REQUEST_RESUME_CALL ||
               request == RIL_REQUEST_SETUP_DATA_CALL ||
               request == RIL_REQUEST_DEACTIVATE_DATA_CALL ||
               request == RIL_REQUEST_VIDEO_CALL_ACCEPT ||
               request == RIL_REQUEST_ECC_REDIAL_APPROVE ||
               request == RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER ||
               request == RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER ||
               request == RIL_REQUEST_DIAL_WITH_SIP_URI ||
               request == RIL_REQUEST_VT_DIAL_WITH_SIP_URI ||
               request == RIL_REQUEST_IMS_ECT ||
               request == RIL_REQUEST_SET_CLIR ||
               request == RIL_LOCAL_REQUEST_SETUP_DATA_CALL_ALT ||
               request == RIL_REQUEST_SET_IMS_RTP_REPORT ||
               request == RIL_REQUEST_IMS_DIAL ||
               request == RIL_REQUEST_IMS_VT_DIAL ||
               request == RIL_LOCAL_REQUEST_IMS_EMERGENCY_DIAL ||
               request == RIL_LOCAL_REQUEST_SET_ECC_SERVICE_CATEGORY ||
               // SMS over wifi is able to send SMS even if radio is off
               request == RIL_REQUEST_SEND_SMS ||
               request == RIL_REQUEST_SEND_SMS_EXPECT_MORE ||
               request == RIL_REQUEST_SMS_ACKNOWLEDGE ||
               request == RIL_REQUEST_SMS_ACKNOWLEDGE_EX ||
               request == RIL_REQUEST_IMS_SEND_SMS ||
               request == RIL_REQUEST_IMS_SEND_SMS_EX ||
               request == RIL_REQUEST_SET_FD_MODE ||
               request == RIL_REQUEST_SEND_DEVICE_STATE)) {
             RLOGD("call command accept in radio off if wfc is support");
         }
         /// M: If Epdg support only case.
         else if (isEpdgSupport() &&
               (request == RIL_LOCAL_REQUEST_WIFI_DISCONNECT_IND)) {
             RLOGD("call command accept in radio off if epdg is support");
         } else {
             RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
             return;
         }
    }

    //BEGIN mtk03923 [ALPS00061979 - MS Lost network after sending many DTMF]
    if (inCallNumber == 0 &&
        (request == RIL_REQUEST_DTMF ||
         request == RIL_REQUEST_DTMF_START ||
         request == RIL_REQUEST_DTMF_STOP)) {
        RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0); // RIL_E_GENERIC_FAILURE
        return;
    }
    //END   mtk03923 [ALPS00061979 - MS Lost network after sending many DTMF]

    /* set pending RIL request */
    setRequest(request);

    if (!(rilSimMain(request, data, datalen, t) ||
          rilNwMain(request, data, datalen, t) ||
          rilCcMain(request, data, datalen, t) ||
          rilSsMain(request, data, datalen, t) ||
          rilSmsMain(request, data, datalen, t) ||
          rilC2kSmsMain(request, data, datalen, t) ||
          rilStkMain(request, data, datalen, t) ||
          rilOemMain(request, data, datalen, t) ||
          /// M: eMBMS feature
          rilEmbmsMain(request, data, datalen, t)||
          /// @}
          rilDataMain(request, data, datalen, t) ||
          rilPhbMain(request, data, datalen, t) ||
          rilImsMain(request, data, datalen, t) ||
          rilRadioMain(request, data, datalen, t) ||
          rilImsCcMain(request, data, datalen, t) ||
          rilSimLockMain(request, data, datalen, t)))
        RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);

    /* Reset pending RIL request */
    resetRequest(request);
}

static void updateConnectionState(RIL_SOCKET_ID socketId, int isConnected) {
}

#if defined(ANDROID_MULTI_SIM)
static void onSapRequest(int request, void *data __unused, size_t datalen __unused, RIL_Token t,
        RIL_SOCKET_ID socket_id) {
#else
static void onSapRequest(int request, void *data, size_t datalen, RIL_Token t) {
#endif
    RLOGD("onSapRequest: %d", request);

    RIL_SOCKET_ID socketId = RIL_SOCKET_1;
    #if defined(ANDROID_MULTI_SIM)
    socketId = socket_id;
    #endif

    if (request < MsgId_RIL_SIM_SAP_CONNECT /* MsgId_UNKNOWN_REQ */ ||
            request > MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL) {
        RLOGD("invalid request");
        RIL_SIM_SAP_ERROR_RSP rsp;
        rsp.dummy_field = 1;
        sendSapResponseComplete(t, (RIL_Errno) Error_RIL_E_REQUEST_NOT_SUPPORTED,
                MsgId_RIL_SIM_SAP_ERROR_RESP, &rsp);
        return;
    }

    if (s_md_off) {
        RLOGD("MD off and reply failure to Sap message");
        RIL_SIM_SAP_CONNECT_RSP rsp;
        rsp.response = RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_CONNECT_FAILURE;
        rsp.has_max_message_size = false;
        rsp.max_message_size = 0;
        sendSapResponseComplete(t, (RIL_Errno) Error_RIL_E_RADIO_NOT_AVAILABLE,
                MsgId_RIL_SIM_SAP_CONNECT, &rsp);
        return;
    }

    /* set pending RIL request */
    setRequest(request);

    if (!(rilBtSapMain(request, data, datalen, t, socketId) ||
            rilStkBtSapMain(request, data, datalen, t, socketId))) {
        RIL_SIM_SAP_ERROR_RSP rsp;
        rsp.dummy_field = 1;
        sendSapResponseComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, MsgId_RIL_SIM_SAP_ERROR_RESP
                , &rsp);
    }

    /* Reset pending RIL request */
    resetRequest(request);
}

/**
 * Synchronous call from the RIL to us to return current radio state.
 * RADIO_STATE_UNAVAILABLE should be the initial state.
 */
static RIL_RadioState currentState(RIL_SOCKET_ID rid)
{
    return getRadioState(rid);
}

/**
 * Call from RIL to us to find out whether a specific request code
 * is supported by this implementation.
 *
 * Return 1 for "supported" and 0 for "unsupported"
 */
static int onSupports(int requestCode __unused)
{
    //@@@ todo
    return 1;
}

static void onCancel(RIL_Token t __unused)
{
    //@@@todo
}

static const char *getVersion(void)
{
    switch (SIM_COUNT) {
        case 2:
            return "mtk gemini ril 1.0";
        case 3:
            return "mtk gemini+ 3 SIM ril 1.0";
        case 4:
            return "mtk gemini+ 4 SIM ril 1.0";
        case 1:
        default:
            return "mtk ril w10.20";
    }
}


static int isReadMccMncForBootAnimation()
{
    char prop[PROPERTY_VALUE_MAX] = {0};

    property_get("ro.vendor.mtk_rild_read_imsi", prop, "");
    if (!strcmp(prop, "1")) {
        return 1;
    }
    property_get("ro.vendor.mtk_disable_cap_switch", prop, "0");
    if (!strcmp(prop, "0")) {
        return 1;
    }
    property_get("persist.vendor.operator.optr", prop, "");

    return (!strcmp(prop, "OP01") || !strcmp(prop, "OP02")) ? 1 : 0;
}

static int isATCommandRawShouldBeHandled(const int request, char* s, size_t len) {

    if (len <= 0) {
        return 0;
    }

    if (request == RIL_LOCAL_REQUEST_AT_COMMAND_WITH_PROXY) {
        if (strStartsWith(s, "AT+ESIMAUTH")||
            strStartsWith(s, "AT+EAUTH") ||
            strStartsWith(s, "AT+EIMSVOICE") ||
            strStartsWith(s, "AT+EIMSCCP") ||
            strStartsWith(s, "AT+EIMSWFC") ||
            strStartsWith(s, "AT+EIMSVOLTE") ||
            strStartsWith(s, "AT+EWIFISTA") ||
            strStartsWith(s, "AT+EIMSREGCON") ||
            strStartsWith(s, "AT+EIMSDEREG ") ||
            strStartsWith(s, "AT+EIMS")) {
            // RLOGD("[MAL] Special Handling for ESIMAUTH related command");
            return 1;
        }
    }
    return 0;
}

static void resetSystemProperties(RIL_SOCKET_ID rid)
{
    char* propMccMnc = NULL;
    char* propMccMncCdma = NULL;

    RLOGI("[RIL_CALLBACK] resetSystemProperties");
    upadteSystemPropertyByCurrentMode(rid,"vendor.ril.ipo.radiooff",
            "vendor.ril.ipo.radiooff.2","0");
    resetSIMProperties(rid);

    // +ESIMAPP URC. Reset mcc mnc property.
    if (rid == RIL_SOCKET_1) {
        asprintf(&propMccMnc, "%s", PROPERTY_MCC_MNC);
        asprintf(&propMccMncCdma, "%s", PROPERTY_MCC_MNC_CDMA);
    } else {
        asprintf(&propMccMnc, "%s.%c", PROPERTY_MCC_MNC, rid + '0');
        asprintf(&propMccMncCdma, "%s.%c", PROPERTY_MCC_MNC_CDMA, rid + '0');
    }
    property_set(propMccMnc, "N/A");
    property_set(propMccMncCdma, "N/A");
    free(propMccMnc);
    free(propMccMncCdma);
}

void queryActiveMode(RIL_SOCKET_ID rid, RILChannelCtx *p_channel) {
    ATResponse *p_response = NULL;
    char* line;
    char* activeMode = NULL;
    int csraaResponse[3] = {0};
    int err;

    if (p_channel == NULL){
        err = at_send_command_multiline(
                "AT+CSRA?", "+CSRAA:", &p_response, getChannelCtxbyProxy());
    } else {
        err = at_send_command_multiline("AT+CSRA?", "+CSRAA:", &p_response, p_channel);
    }

    if (err == 0 && p_response->success != 0 && p_response->p_intermediates != NULL){
        line = p_response->p_intermediates->line;
        asprintf(&activeMode, "%d", 0);
        /* +CSRAA: <GERAN-TDMA>,<UTRANFDD>,<UTRAN-TDD-LCR>,<UTRAN-TDD-HCR>,
                                        <UTRANTDD-VHCR>,<E-UTRAN-FDD>,<E-UTRAN-TDD>*/
        err = at_tok_start(&line);
        if (err >= 0){
            err = at_tok_nextint(&line, &csraaResponse[0]);//<GERAN-TDMA>
            if (err >= 0){
                err = at_tok_nextint(&line, &csraaResponse[1]);//<UTRANFDD>
                if (err >= 0){
                    LOGD("+CSRAA:<UTRANFDD> = %d", csraaResponse[1]);
                    err = at_tok_nextint(&line, &csraaResponse[2]);//<UTRAN-TDD-LCR>
                    if (err >= 0){
                        LOGD("+CSRAA:<UTRAN-TDD-LCR> = %d", csraaResponse[2]);
                        if ((csraaResponse[1] == 1) && (csraaResponse[2] == 0)){
                            //FDD mode
                            if (NULL != activeMode){
                                free(activeMode);
                                activeMode = NULL;
                            }
                            asprintf(&activeMode, "%d", 1);
                        }
                        if ((csraaResponse[1] == 0) && (csraaResponse[2] == 1)){
                            //TDD mode
                            if (NULL != activeMode){
                                free(activeMode);
                                activeMode = NULL;
                            }
                            asprintf(&activeMode, "%d", 2);
                        }
                        LOGD("update property vendor.ril.nw.worldmode.activemode to %s", activeMode);
                        property_set("vendor.ril.nw.worldmode.activemode", activeMode);
                        s_worldmode_activemode = atoi(activeMode);
                    }
                }
            }
        }
        if (NULL != activeMode){
            free(activeMode);
            activeMode = NULL;
        }
    }
    at_response_free(p_response);
}

void queryBearer(RILSubSystemId subsystem, RIL_SOCKET_ID rid)
{
    ATResponse *p_response = NULL;
    int err;
    char *line;
    int ret;
    char propValue[PROPERTY_VALUE_MAX] = {0};
    char cdmaSlot_prop[PROPERTY_VALUE_MAX] = {0};
    int worldMode = 0;
    int mainRid = RIL_get3GSIM() - 1;
    int ps1MaxRat = RAF_GPRS;
    int ps2MaxRat = RAF_GSM | RAF_GPRS;
    int ps3MaxRat = RAF_GSM | RAF_GPRS;
    int i = 0;
    int isFinished = 1;
    int max_rid = 0;
    int hasTDCapability = 0;
    int cdmaSlot = -1;
    int mode = 0;

    /* modem capability(modem report directly)
     * value(bit mask) : GPRS, EDGE, WCDMA, TD-SCDMA...etc
     **/
    int modemReport[RIL_SOCKET_NUM] = {0};

    memset(propValue, 0, PROPERTY_VALUE_MAX);

    // the max capability of sim1
    if (RatConfig_isGsmSupported()) {
        ps1MaxRat |= (RAF_GSM | RAF_GPRS);
    }
    if (RatConfig_isWcdmaSupported()) {
        ps1MaxRat |= RAF_UMTS;
    }
    if (RatConfig_isTdscdmaSupported()) {
        ps1MaxRat |= RAF_TD_SCDMA;
    }
    // the max capability of sim2
    property_get("persist.vendor.radio.mtk_ps2_rat", propValue, "G");
    if (strchr(propValue, 'W') != NULL) {
        ps2MaxRat |= RAF_UMTS;
    }
    if (strchr(propValue, 'L') != NULL) {
        ps2MaxRat |= RAF_LTE;
    }
    // the max capability of sim3
    property_get("persist.vendor.radio.mtk_ps3_rat", propValue, "G");
    if (strchr(propValue, 'W') != NULL) {
        ps3MaxRat |= RAF_UMTS;
    }
    if (strchr(propValue, 'L') != NULL) {
        ps3MaxRat |= RAF_LTE;
    }

    // query modem capability
    err = at_send_command_singleline("AT+EPSB?", "+EPSB:", &p_response, getChannelCtxbyProxy());

    if (err < 0 || p_response->success == 0) {
        LOGE("[RIL%d] AT+EPSB return ERROR", rid+1);
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextint(&line, &ret);
    if (err < 0) goto error;

    /* +EPSB: <bearer>
     * GPRS: 0x0001
     * EDGE: 0x0002
     * WCDMA: 0x0004
     * TD-SCDMA: 0x0008
     * HSDPA: 0x0010
     * HSUPA: 0x0020
     * HSPA+: 0x0040
     * FDD LTE: 0x0080
     * TDD LTE: 0x0100
     */
    // reset capability
    s_modemCapability[rid] = RAF_GSM;

    // only main protocol has TD-SCDMA
    if ((ret & 0x0008) > 0) {
        s_modemCapability[rid] |= RAF_GPRS;
        hasTDCapability = 1;
    }
    // if modem is not support TD-SCDMA on all rids, set the main protocol
    if (hasTDCapability == 0) {
        for (i = 0; i < SIM_COUNT; i++) {
            if (modemReport[i] == 0) {
                isFinished = 0;
                break;
            }
            if (modemReport[i] >= modemReport[max_rid]) {
                s_modemCapability[max_rid] &= (~RAF_GPRS);
                max_rid = i;
            }
        }

        if (isFinished == 1) {
            s_modemCapability[max_rid] |= RAF_GPRS;
        }
    }

    // set 4G capability
    if ((RatConfig_isLteTddSupported() || RatConfig_isLteFddSupported()) &&
        (((ret & 0x0100) > 0) || ((ret & 0x0080) > 0))) {
        s_modemCapability[rid] |= RAF_LTE;
        ps1MaxRat |= RAF_LTE;
    }

    // set 3G capability
    if (s_simSwitchVersion == 2) {
        // property_get("ril.nw.worldmode.activemode", propValue, "1");
        // worldMode = atoi(propValue);
        worldMode = s_worldmode_activemode;
        if ((ret & 0x0008) > 0 && worldMode == 2) {
            // TDD modem
            s_modemCapability[rid] |= RAF_TD_SCDMA;
        }
        if ((ret & 0x0004) > 0 && worldMode == 1) {
            // FDD modem
            s_modemCapability[rid] |= RAF_UMTS;
        }
    } else {
        property_get("vendor.ril.active.md", propValue, "0");
        worldMode = atoi(propValue);
        if (worldMode == 4 || worldMode == 6 || worldMode == 7) {
            // TDD modem
            if ((ret & 0x0008) > 0) {
                s_modemCapability[rid] |= RAF_TD_SCDMA;
            }
        } else {
            // FDD modem
            if ((ret & 0x0004) > 0) {
                s_modemCapability[rid] |= RAF_UMTS;
            }
        }
    }

    // check the priority of WCDMA and TD_SCDMA
    memset(propValue, 0, PROPERTY_VALUE_MAX);
    property_get("vendor.ril.nw.worldmode.keep_3g_mode", propValue, "0");
    mode = atoi(propValue);
    LOGD("[RIL%d] keep_3g_mode = %d", rid+1, mode);
    if (mode == 0) {
        //check if support tplusw
        memset(propValue, 0, PROPERTY_VALUE_MAX);
        property_get("vendor.ril.simswitch.tpluswsupport", propValue, "0");
        mode = atoi(propValue);
        LOGD("[RIL%d] tpluswsupport = %d", rid+1, mode);
    }
    if (mode == 1) {
        // TDD modem
        if ((ret & 0x0008) > 0) {
            s_modemCapability[rid] |= RAF_TD_SCDMA;
        }
        // FDD modem
        if ((ret & 0x0004) > 0) {
            s_modemCapability[rid] |= RAF_UMTS;
        }
    }

    if (mainRid == 0) {
        // major SIM is SIM1
        if (rid == 0) {
            s_modemCapability[rid] = s_modemCapability[rid] & ps1MaxRat;
        } else if (rid == 1) {
            s_modemCapability[rid] = s_modemCapability[rid] & ps2MaxRat;
        } else if (rid == 2) {
            s_modemCapability[rid] = s_modemCapability[rid] & ps3MaxRat;
        }
    } else if (mainRid == 1) {
        // major SIM is SIM2
        if (rid == 1) {
            s_modemCapability[rid] = s_modemCapability[rid] & ps1MaxRat;
        } else if (rid == 0) {
            s_modemCapability[rid] = s_modemCapability[rid] & ps2MaxRat;
        } else if (rid == 2) {
            s_modemCapability[rid] = s_modemCapability[rid] & ps3MaxRat;
        }
    } else if (mainRid == 2) {
        // major SIM is SIM3
        if (rid == 2) {
            s_modemCapability[rid] = s_modemCapability[rid] & ps1MaxRat;
        } else if (rid == 1) {
            s_modemCapability[rid] = s_modemCapability[rid] & ps2MaxRat;
        } else if (rid == 0) {
            s_modemCapability[rid] = s_modemCapability[rid] & ps3MaxRat;
        }
    }

    // add cdma capability
    if (RatConfig_isC2kSupported()) {
        if (isDualCTCard()) {
            if (hasTDCapability) {
                s_modemCapability[rid] |= RAF_IS95A | RAF_IS95B | RAF_1xRTT | RAF_EVDO_0
                    | RAF_EVDO_A | RAF_EHRPD;
            }
        } else {
            property_get("persist.vendor.radio.cdma_slot", cdmaSlot_prop, "1");
            cdmaSlot = atoi(cdmaSlot_prop);
            if (cdmaSlot == (rid+1)) {
                // MD3 capability
                s_modemCapability[rid] |= RAF_IS95A | RAF_IS95B | RAF_1xRTT | RAF_EVDO_0
                      | RAF_EVDO_A | RAF_EHRPD;
            }
        }
    }

    LOGD("[RIL%d] capability %d, worldmode %d, mainRid %d, isDualCTCard %d, cdmaSlot %d",
        rid+1, s_modemCapability[rid], worldMode, mainRid, isDualCTCard(), cdmaSlot);

error:
    at_response_free(p_response);
}


void updateRadioCapability(void) {
    RLOGI("updateRadioCapability, old:  %d, %d", s_modemCapability[0], s_modemCapability[1]);

    queryBearer(RIL_SIM, RIL_SOCKET_1);
    if (getSimCount() >= 2) {
        queryBearer(RIL_SIM, RIL_SOCKET_2);
        if (getSimCount() >= 3) {
            queryBearer(RIL_SIM, RIL_SOCKET_3);
            if (getSimCount() >= 4) {
                queryBearer(RIL_SIM, RIL_SOCKET_4);
            }
        }
    }

    RLOGI("updateRadioCapability, new:  %d, %d", s_modemCapability[0], s_modemCapability[1]);
    sendRadioCapabilityDoneIfNeeded();
}

void initMsimConfiguration(void *param, RILChannelCtx *pChannel){
    RIL_SOCKET_ID rid = *((RIL_SOCKET_ID *)param);
    int current_share_modem = 0;

    if (getMainProtocolRid() == rid) {
        current_share_modem = getSimCount();
        switch (current_share_modem) {
            case 1:
                at_send_command("AT+ESADM=1", NULL, pChannel);
                break;
            case 2:
                at_send_command("AT+ESADM=3", NULL, pChannel);
                break;
            case 3:
                at_send_command("AT+ESADM=7", NULL, pChannel);
                break;
            case 4:
                at_send_command("AT+ESADM=15", NULL, pChannel);
                break;
        }
    }
}

void setSilentReboot(void *param, RILChannelCtx *pChannel) {
    RIL_SOCKET_ID rid = *((RIL_SOCKET_ID *)param);
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    int muxreport_case = 0;
    int auto_unlock_pin = -1;
    int isSilentReboot = -1;

    /********************************
    * AT+EBOOT=<mode>
    *
    * 0: Normal boot up
    * 1: Silent boot up (Verify PIN by modem internally)
    *********************************/
    if (getMainProtocolRid() == rid) {
        property_get("vendor.ril.mux.report.case", property_value, "0");
        muxreport_case = atoi(property_value);
        LOGD("getprop vendor.ril.mux.report.case %d", muxreport_case);
        switch (muxreport_case) {
            case 0:
                isSilentReboot = 0;
                break;
            case 1:
            case 2:
            case 5:
            case 6:
                isSilentReboot = 1;
                break;
        }
        property_set("vendor.ril.mux.report.case", "0");

        property_get("vendor.gsm.ril.eboot", property_value, "-1");
        auto_unlock_pin = atoi(property_value);
        RLOGD("getprop vendor.gsm.ril.eboot %d", auto_unlock_pin);

         // eboot property will be set to 0 when ipo shutdown, no needs to silent reboot in this case
         // ebbot property will be set to 1 when flight mode turn on, and 3g switch reset modem
        if(auto_unlock_pin == 0) {
            isSilentReboot = 0;
        } else if (auto_unlock_pin == 1) {
            isSilentReboot = 1;
        }

        RLOGD("isSilentReboot %d", isSilentReboot);
        switch (isSilentReboot) {
            case 0:
                at_send_command("AT+EBOOT=0", NULL, pChannel);
                break;
            case 1:
                at_send_command("AT+EBOOT=1", NULL, pChannel);
                break;
            default:
                at_send_command("AT+EBOOT=0", NULL, pChannel);
                break;
        }
        property_set("vendor.gsm.ril.eboot", "-1");
    }
}

/**
 * Initialize everything that can be configured while we're still in
 * AT+CFUN=0
 */
#ifdef  MTK_RIL
static void initializeCallback(void *param)
{
    ATResponse *p_response = NULL;
    int err;
    RIL_SOCKET_ID rid = *((RIL_SOCKET_ID *)param);
    int nRadioState = -1;
    //[New R8 modem FD] Enlarge array size of property_value
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    int current_share_modem = 0;
    int mdType = -1;
    int i=0;
    char *cmd = NULL;
#ifdef MTK_MUX_CHANNEL_64
    RILSubSystemId subSysId = RIL_RADIO;
#else
    RILSubSystemId subSysId = RIL_DEFAULT;
#endif
    RILChannelCtx *pChannel = getChannelCtxbyProxy();
    bool vsimEnabledByRid = false;
    bool anyVsimEnabled = false;

    if (getMainProtocolRid() == rid) {
        pthread_mutex_lock(&s_init_mutex);
        // trigger other slots' initialcallback
        int i = 0;
        int curr_share_modem = getSimCount();
        if (curr_share_modem >= 2) {
            for (i = 0; i < curr_share_modem; i++) {
                if (s_pollSimId[i] != rid) {
                    RLOGI("Start initialized callback (MSMS - Others, %d)", s_pollSimId[i]);
                    RIL_requestProxyTimedCallback(initializeCallback, (void *)&s_pollSimId[i],
                            &TIMEVAL_0,
                            getRILChannelId(subSysId, s_pollSimId[i]), "initializeCallback");
                }
            }
        }
    }

    resetSystemProperties(rid);
    err = at_handshake(pChannel);
    RLOGI("AT handshake: %d", err);
    setRadioState(RADIO_STATE_OFF, rid);

    /* note: we don't check errors here. Everything important will
     * be handled in onATTimeout and onATReaderClosed */

    /*  atchannel is tolerant of echo but it must */
    /*  have verbose result codes */
    at_send_command("ATE0Q0V1", NULL, pChannel);

    /*  No auto-answer */
    at_send_command("ATS0=0", NULL, pChannel);

    /*  Extended errors */
    at_send_command("AT+CMEE=1", NULL, pChannel);

    /*  Disable CFU query */
    err = at_send_command("AT+ESSP=1", &p_response, pChannel);

    /* check if modem support +CREG=3 */
    err = at_send_command("AT+CREG=3", &p_response, pChannel);
    if (err < 0 || p_response->success == 0) {
        bCREGType3Support = 0;

        /*  check if modem support +CREG=2 */
        err = at_send_command("AT+CREG=2", &p_response, pChannel);

        /* some handsets -- in tethered mode -- don't support CREG=2 */
        if (err < 0 || p_response->success == 0)
            at_send_command("AT+CREG=1", NULL, pChannel);
    }
    at_response_free(p_response);

    // M: OP07 RAT filter
    if (isDisable2G()) {
        at_send_command("AT+EDRAT=1", NULL, pChannel);
    }

    if (isFemtocellSupport()) {
        err = at_send_command("AT+ECSG=4,1", NULL, pChannel);
    }

    // M: [VzW] Data Framework
    // Try to disable VZW APN if it needs
    syncDisabledApnToMd(pChannel);

    if (isImsSupport()) {
        err = at_send_command("AT+CIREG=2", &p_response, pChannel);
        at_response_free(p_response);
    }

    /*  GPRS registration events */
    err = at_send_command("AT+ECGREG=1", &p_response, pChannel);
    if (err != 0 || p_response->success == 0) {
        at_send_command("AT+CGREG=1", NULL, pChannel);
    } else {
        at_response_free(p_response);
        p_response = NULL;
        err = at_send_command("AT+CGREG=3", &p_response, pChannel);
        if (err != 0 || p_response->success == 0) {
            at_response_free(p_response);
            p_response = NULL;
            err = at_send_command("AT+CGREG=2", &p_response, pChannel);
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+CGREG=2");
        }
    }
    at_response_free(p_response);
    p_response = NULL;

    err = at_send_command("AT+ECEREG=1", &p_response, pChannel);
    if (err != 0 || p_response->success == 0) {
        at_send_command("AT+CEREG=1", NULL, pChannel);
    } else {
        at_response_free(p_response);
        p_response = NULL;
        err = at_send_command("AT+CEREG=3", &p_response, pChannel);
        if (err != 0 || p_response->success == 0) {
            at_response_free(p_response);
            p_response = NULL;
            err = at_send_command("AT+CEREG=2", &p_response, pChannel);
            if (err != 0 || p_response->success == 0)
                LOGW("There is something wrong with the exectution of AT+CEREG=2");
        }
    }
    at_response_free(p_response);
    p_response = NULL;
    err = at_send_command("AT+PSBEARER=1", NULL, pChannel);
    if (err < 0) {
        bPSBEARERSupport = 0;
    }

    enableLteBand8(rid, pChannel);

   /*  Call Waiting notifications */
    at_send_command("AT+CCWA=1", NULL, pChannel);

    /*  mtk00924: enable Call Progress notifications */
    at_send_command("AT+ECPI=4294967295", NULL, pChannel);

    /*  Alternating voice/data off */
    /*
     * at_send_command("AT+CMOD=0", NULL, pChannel);
     */

    /*  Not muted */
    /*
     * at_send_command("AT+CMUT=0", NULL, pChannel);
     */

    /*  +CSSU unsolicited supp service notifications */
    at_send_command("AT+CSSN=1,1", NULL, pChannel);

    /*  connected line identification on */
    at_send_command("AT+COLP=1", NULL, pChannel);

    /*  HEX character set */
    at_send_command("AT+CSCS=\"UCS2\"", NULL, pChannel);

    /*  USSD unsolicited */
    at_send_command("AT+CUSD=1", NULL, pChannel);

    /*  Enable USSI URC */
    err = at_send_command("AT+EIUSD=2,4,1,\"\",\"\",0",  &p_response, pChannel);
    RLOGD("AT+EIUSD, got err= %d, success=%d, finalResponse = %s", err, p_response->success,
            p_response->finalResponse);
    at_response_free(p_response);

    /*  Enable +CGEV GPRS event notifications, but don't buffer */
    at_send_command("AT+CGEREP=1,0", NULL, pChannel);

    /*  SMS PDU mode */
    at_send_command("AT+CMGF=0", NULL, pChannel);

    /* Enable getting NITZ, include TZ and Operator Name*/
    /* To Receive +CIEV: 9 and +CIEV: 10*/
    at_send_command("AT+CTZR=1", NULL, pChannel);

    // common data @{
    for (i = 0; i < TOTAL_FEATURE_NUMBER; i++) {
        bool isATCmdEgmrRspErr = false;
        int tempMDSuptVer = MD_NOT_SUPPORTED;

        if (DATA_L4C_LAST_PDN_ERROR_CAUSE == i) {
            /* Modem support version for last PDN fail cause */
            asprintf(&cmd, "AT+EGMR=0,17,%d", DATA_L4C_LAST_PDN_ERROR_CAUSE);
        } else if (DATA_L4C_PDN_CAUSE_REASON == i) {
            /* Modem support version for PDN cause reason */
            asprintf(&cmd, "AT+EGMR=0,17,%d", DATA_L4C_PDN_CAUSE_REASON);
        } else if (DATA_L4C_MD_VERSION == i) {
            /* Modem support version */
            asprintf(&cmd, "AT+EGMR=0,17");
        }

        err = at_send_command_singleline(cmd, "+EGMR:", &p_response, pChannel);
        free(cmd);
        if (err != 0 || p_response->success == 0 || p_response->p_intermediates == NULL) {
            RLOGE("EGMR=0,17,%d got error response", i);
            isATCmdEgmrRspErr = true;
        } else {
            char* line;
            int featureId;
            int supportVersion;
            line = p_response->p_intermediates->line;

            err = at_tok_start(&line);
            if (err >= 0) {
                if (DATA_L4C_MD_VERSION == i) {
                    err = at_tok_nextint(&line, &supportVersion);
                    if (err >= 0) {
                        RLOGD("+EGMR:<support_version> = %d", supportVersion);
                        tempMDSuptVer = supportVersion;
                    }
                } else {
                    err = at_tok_nextint(&line, &featureId);
                    if (err >= 0) {
                        RLOGD("+EGMR:<feature_id> = %d", featureId);
                        err = at_tok_nextint(&line, &supportVersion);
                        if (err >= 0) {
                            RLOGD("+EGMR:<support_version> = %d", supportVersion);
                            tempMDSuptVer = supportVersion;
                        }
                    }
                }
            }
        }
        at_response_free(p_response);

        if (isATCmdEgmrRspErr) {
            int cmdCeppiRsp = MD_NOT_SUPPORTED;

            // For distinguishing between LR11 and before.
            err = at_send_command("AT+CEPPI=?", &p_response, pChannel);
            if (isATCmdRspErr(err, p_response)) {
                cmdCeppiRsp = MD_LR9;
            } else {
                cmdCeppiRsp = MD_LR11;
            }
            at_response_free(p_response);

            RLOGD("+CEPPI:<feature_id> = %d, <support_version> = %d", i, cmdCeppiRsp);
            tempMDSuptVer = cmdCeppiRsp;
        }

        if (DATA_L4C_LAST_PDN_ERROR_CAUSE == i) {
            pdnFailCauseSupportVer = tempMDSuptVer;
        } else if (DATA_L4C_PDN_CAUSE_REASON == i) {
            pdnReasonSupportVer = tempMDSuptVer;
        } else if (DATA_L4C_MD_VERSION == i) {
            pdnMdVersion = tempMDSuptVer;
            char* mdVer = NULL;
            asprintf(&mdVer, "%d", pdnMdVersion);
            property_set("vendor.ril.md.version", mdVer);
            free(mdVer);
        }
    }
    // common data @}

    /*  Enable getting CFU info +ECFU and speech info +ESPEECH*/
    int einfo_value;
    einfo_value = 50;//default value.

    /*  Enable getting CFU info +ECFU and speech info +ESPEECH and modem warning +EWARNING(0x100) */
    char modemWarningProperty[PROPERTY_VALUE_MAX];
    char einfoStr[32];
    property_get("persist.vendor.radio.modem.warning", modemWarningProperty, 0);
    if (strcmp(modemWarningProperty, "1") == 0) {
        /* Enable "+EWARNING" */
       einfo_value |= 512;
    }
    /* Enable response message of call ctrl by sim. */
    einfo_value |= 1024;
    sprintf(einfoStr, "AT+EINFO=%d", einfo_value);
    at_send_command(einfoStr, NULL, pChannel);

/* M: Start - abnormal event logging for logger */
    einfo_value |= 8;
    sprintf(einfoStr, "AT+EINFO=%d,401,0", einfo_value);
    /* Enable smart logging no service notification +ENWINFO */
    at_send_command(einfoStr, NULL, pChannel);
/* M: End - abnormal event logging for logger */

    at_send_command("AT+ECSQ=1", NULL, pChannel);

    /*  Enable get +CIEV:7 URC to receive SMS SIM Storage Status*/
    at_send_command("AT+CMER=1,0,0,2,0", NULL, pChannel);

    // Disable 3G VT by default
    at_send_command("AT+ECCP=1", NULL, pChannel);

    // M: OP17 IA Start
    char optr[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.vendor.operator.optr", optr, "");
    bool isOp17 = (strcmp("OP17", optr) == 0)? true : false;
    if (isOp17) {
        const char *propDataOpSusRsmMd = "vendor.gsm.ril.data.op.suspendmd";
        property_set(propDataOpSusRsmMd, "1");
        at_send_command("AT+ECOPS=1", NULL, pChannel);
        at_send_command("AT+EMSR=0,1", NULL, pChannel);
    }
    // M: OP17 IA End

    // M: Check MD support +ERAT=?, e.g. AT< +ERAT: (0-7, 11, 14)
    if (getMainProtocolRid() == rid) {
        err = at_send_command_singleline("AT+ERAT=?", "+ERAT:", &p_response, pChannel);
        if (err == 0 && p_response->success != 0 && p_response->p_intermediates != NULL) {
            property_set("vendor.ril.nw.erat.ext.support", "1");
        } else {
            RLOGD("AT+ERAT=? not support");
            property_set("vendor.ril.nw.erat.ext.support", "0");
        }
        at_response_free(p_response);
    }

    /* check if modem support +EAPC?  send EAPC setting to modem if support APC */
    err = at_send_command_singleline("AT+EAPC?", "+EAPC:", &p_response, pChannel);
    if (err >= 0 && p_response->success != 0) {
        // set property if modem support APC, EM will check this property to show APC setting
        property_set("vendor.ril.apc.support", "1");
        // check if the APC mode was set before, if yes, send the same at command again
        // AT+EAPC? was apc query command, if return it, means APC mode was not set before
        char apcModeCmd[PROPERTY_VALUE_MAX] = {0};
        char *property;
        asprintf(&property, "persist.vendor.radio.apc.mode%d", rid);
        property_get(property, apcModeCmd, "AT+EAPC?");
        RLOGD("APC Mode State: %s = %s", property, apcModeCmd);
        free(property);
        if (strcmp("AT+EAPC?", apcModeCmd) != 0) {
            at_send_command(apcModeCmd, NULL, pChannel);
        }
    }
    at_response_free(p_response);

    // world phone initialize
    worldPhoneinitialize(rid, pChannel);

    queryBearer(subSysId, rid);

    //[Fast dormancy]
    memset(property_value, 0, sizeof(property_value));
    property_get("ro.vendor.mtk_fd_support", property_value, "0");
    // MTK_FD_SUPPORT is turned on single sim 3G Project, WG/G Gemini Project
    // => For EVDO_DT_SUPPORT: Turn off MTK_FD_DORMANCY
    if (atoi(property_value) == 1) {
        if ((s_modemCapability[rid] & RAF_LTE) == RAF_LTE
                || (s_modemCapability[rid] & RAF_UMTS) == RAF_UMTS) {
            /* Fast Dormancy is only available on 3G Protocol Set */
            /* [Step#01] Query if the new FD mechanism is supported by modem or not */
            err = at_send_command_singleline("AT+EFD=?", "+EFD:", &p_response, pChannel);

            //[New R8 modem FD for test purpose]
            if ((err == 0 && p_response->success == 1)
                    && (strncmp(p_response->finalResponse, "OK", 2) == 0)) {
                /* TEL FW can query this variable to know if AP side
                 * is necessary to execute FD or not
                 */
                property_set(PROPERTY_RIL_FD_MODE, "1");

                /* [Step#02] Set default FD related timers for mode:
                 * format => AT+EFD=2, timer_id, timer_value (unit:0.1sec)
                 */
                char *timer_value;
                memset(property_value, 0, sizeof(property_value));
                property_get(PROPERTY_FD_SCREEN_OFF_TIMER,
                        property_value, DEFAULT_FD_SCREEN_OFF_TIMER);
                RLOGD("Screen Off FD Timer=%s", property_value);
                /* timerId=0: Screen Off + Legancy FD */
                asprintf(&timer_value, "AT+EFD=2,0,%d", (int)(atof(property_value)));
                at_send_command(timer_value, NULL, pChannel);
                free(timer_value);

                /* timerId=2: Screen Off + R8 FD */
                memset(property_value, 0, sizeof(property_value));
                property_get(PROPERTY_FD_SCREEN_OFF_R8_TIMER,
                        property_value, DEFAULT_FD_SCREEN_OFF_R8_TIMER);
                asprintf(&timer_value, "AT+EFD=2,2,%d", (int)(atof(property_value)));
                at_send_command(timer_value, NULL, pChannel);
                free(timer_value);

                memset(property_value, 0, sizeof(property_value));
                property_get(PROPERTY_FD_SCREEN_ON_TIMER,
                        property_value, DEFAULT_FD_SCREEN_ON_TIMER);
                RLOGD("Screen On FD Timer=%s", property_value);
                /* timerId=1: Screen On + Legancy FD */
                asprintf(&timer_value, "AT+EFD=2,1,%d", (int)(atof(property_value)));
                at_send_command(timer_value, NULL, pChannel);
                free(timer_value);

                /* timerId=3: Screen On + R8 FD */
                memset(property_value, 0, sizeof(property_value));
                property_get(PROPERTY_FD_SCREEN_ON_R8_TIMER,
                        property_value, DEFAULT_FD_SCREEN_ON_R8_TIMER);
                asprintf(&timer_value, "AT+EFD=2,3,%d", (int)(atof(property_value)));
                at_send_command(timer_value, NULL, pChannel);
                free(timer_value);

                memset(property_value, 0, sizeof(property_value));
                property_get(PROPERTY_FD_ON_ONLY_R8_NETWORK,
                        property_value, DEFAULT_FD_ON_ONLY_R8_NETWORK);
                RLOGD("%s = %s", PROPERTY_FD_ON_ONLY_R8_NETWORK, property_value);
                if (atoi(property_value) == 1) {
                    at_send_command("AT+EPCT=0,4194304", NULL, pChannel);
                }
            }
        }
    } else {
        RLOGI("Fast dormancy is configured disabled");
        at_send_command("AT+EFD=0", NULL, pChannel);
    }

    at_send_command("AT+EAIC=2", NULL, pChannel);

    at_send_command("AT+CLIP=1", NULL, pChannel);

    at_send_command("AT+CNAP=1", NULL, pChannel);

    /// M: CC: GSA HD Voice for 2/3G network support
    at_send_command("AT+EVOCD=1", NULL, pChannel);

    // set data/call prefer
    // 0 : call prefer
    // 1 : data prefer
    char gprsPrefer[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.radio.gprs.prefer", gprsPrefer, "0");
    if ((atoi(gprsPrefer) == 0)) {
        // call prefer
        at_send_command("AT+EGTP=1", NULL, pChannel);
        at_send_command("AT+EMPPCH=1", NULL, pChannel);
    } else {
        // data prefer
        at_send_command("AT+EGTP=0", NULL, pChannel);
        at_send_command("AT+EMPPCH=0", NULL, pChannel);
    }

    /* ALPS00574862 Remove redundant +COPS=3,2;+COPS? multiple cmd in REQUEST_OPERATOR */
    at_send_command("AT+COPS=3,2", NULL, pChannel);

    // ALPS00353868 START
    err = at_send_command("AT+COPS=3,3",  &p_response, pChannel);
    RLOGI("AT+COPS=3,3 got err= %d,success=%d", err, p_response->success);

    if (err >= 0 && p_response->success != 0) {
        setPlmnListFormat(rid, 1);
    }
    at_response_free(p_response);
    // ALPS00353868 END

    /* To support get PLMN when not registered to network via AT+EOPS?  START */
    err = at_send_command("AT+EOPS=3,2",  &p_response, pChannel);
    RLOGI("AT+EOPS=3,2 got err= %d,success=%d", err, p_response->success);

    if (err >= 0 && p_response->success != 0){
        bEopsSupport = 1;
    }
    at_response_free(p_response);
    /* To support get PLMN when not registered to network via AT+EOPS?  END */

    // MTK-START: Enable +ESIMAPP URC to report mcc mnc.
    at_send_command("AT+ESIMAPP=0,1", NULL, pChannel);
    // MTK-END

    /// M: GSMA TS.27 13.3.7 @{
    if (isUiccCLFSupport()) {
        /* TS26_NFC_REQ_166:
         * Send Terminal Capability command to the UICC indicating that the
         * UICC-CLF interface (SWP) is supported as per ETSI TS 102 221. It
         * should send before sending AT+ESIMS and AT+EFUN.
         */
        err = at_send_command("AT+ESBP=5,\"SBP_TERMINAL_CAPABILITY_FLEX\",1", &p_response, pChannel);
        if (err < 0 || p_response->success == 0) {
            at_send_command("AT+ESBP=1,191,1", NULL, pChannel); /* Before LR11 */
        }
        at_response_free(p_response);
    }
    /// M: GSMA TS.27 13.3.7 @}

    // MTK-START: SIM HOT SWAP
    at_send_command("AT+ESIMS=1", NULL, pChannel);
    resetAidInfo(rid);
    // MTK-END

    //ALPS01228632
#ifdef MTK_SIM_RECOVERY
    at_send_command("AT+ESIMREC=1", NULL, pChannel);
#else
    at_send_command("AT+ESIMREC=0", NULL, pChannel);
#endif
    // MTK-START:[C2K]Dynamic Capability Switch support.
    if (isSrlteSupport() && rid == getMainProtocolRid()) {
        at_send_command("AT+EMDSTATUS=0,0,1", NULL, pChannel);
    }
    // MTK-END

    initMsimConfiguration(param, pChannel);
    ///M: [Network][C2K] add network C2K init network mode flow. @{
    initNetworkMode(rid, pChannel);
    /// @}
    setSilentReboot(param, pChannel);

    releaseExtraWakeLock();

    current_share_modem = getSimCount();

    nRadioState = queryRadioState(subSysId, rid);

    at_send_command("AT+EREGINFO=1", NULL, pChannel);

    if (isEnableModulationReport()) {
        at_send_command("AT+EMODCFG=1", NULL, pChannel);
    }

    // External SIM [Start]
#ifdef MTK_EXTERNAL_SIM_SUPPORT
    if (isExternalSimSupport()) {
        initVsimConfiguration(rid);
        queryModemVsimCapability(rid);
        requestSetAkaSim(rid);
        requestSwitchExternalSim(rid);
        vsimEnabledByRid = (isVsimEnabledByRid(rid) == 1);
        anyVsimEnabled = (isVsimEnabled() == 1);
    }
    if (vsimEnabledByRid) {
        RIL_requestProxyTimedCallback(initializeResetVsim, (void *)param,
                &TIMEVAL_0, getRILChannelId(RIL_SIM, rid), "initializeResetVsim");
    } else
#endif
    // External SIM [End]
    {
        if (0 == nRadioState) {
            // SIM reset if modem haven't reset SIM card
            requestSimReset(rid);
        } else {
            // Query SIM inserted status
            requestSimInsertStatus(rid);
        }
    }
    RLOGD("start rild bootup flow [%d, %d, %d]", rid, s_isSimSwitched, current_share_modem);

    if (getMainProtocolRid() == rid) {
        int i = 0;
        if (!anyVsimEnabled) {
            flightModeBoot(pChannel);
        }
        // unlock the init mutex
        pthread_mutex_unlock(&s_init_mutex);
        // we should get iccid after EFUN=0 for each rid
        // get iccid of main protocol first for data requirement
        // needs to decide EGTYP arguments for auto attached
        if (!anyVsimEnabled) {
            bootupGetIccid(pChannel);  // main protocol
        }
        bootupGetCalData(pChannel);

        const char *logmuch_detect = "vendor.logmuch.delay";
        if ((isInternalLoad() == 1) && (isEngLoad() == 1)) {
            char logDelay[PROPERTY_VALUE_MAX] = { 0 };
            property_get(logmuch_detect, logDelay,"0");
            int delay = atoi(logDelay);
            if (delay < INT_MAX) {
                char strDelay[INT_MAX_LENGTH];
                snprintf(strDelay, INT_MAX_LENGTH, "%d", ++delay);
                property_set(logmuch_detect, strDelay);
            } else {
                property_set(logmuch_detect, "1");
            }
        }
    } else {
        RLOGD("wait main protocol init done...main %d, cur %d", getMainProtocolRid(), rid);
        pthread_mutex_lock(&s_init_mutex);
        RLOGD("wait main protocol init done...finish");
        if (!anyVsimEnabled) {
            bootupGetIccid(pChannel);
        }
        pthread_mutex_unlock(&s_init_mutex);
    }
    if (getMultiImsSupportValue() == 1) {
        if (getMainProtocolRid() == rid) {
            imsInit(subSysId, rid);
        }
    } else {
        imsInit(subSysId, rid);
    }

    bootupGetImei(pChannel);
    bootupGetImeisv(pChannel);
    bootupGetBasebandVersion(pChannel);

    updateNitzOperInfo(rid);

    // This is used for wifi-onlg version load
    // Since RIL is not connected to RILD in wifi-only version
    // we query it and stored into a system property
    // note: since this patch has no impact to nomal load, do this in normal initial procedure
    requestSN(pChannel);

    if (getMainProtocolRid() == rid) {
        /* To check if new SS service class feature is supported or not */
        determineSSServiceClassFeature(rid, pChannel);
        // GCG switcher feature
        requestGetGcfMode(rid);
        // GCG switcher feature
    }
    /* assume radio is off on error */
    if (queryRadioState(subSysId, rid) == 1) {
        setRadioState(RADIO_STATE_ON, rid);
    }
    //bootupSetRadio(rid);

    // query EACTS support or not.
    err = at_send_command("AT+EACTS=?", &p_response, pChannel);
    if (isATCmdRspErr(err, p_response) != 0) {
        LOGD("EACTS command not support.");
        sEactsSupport = 0;
    } else {
        LOGD("EACTS command support.");
        sEactsSupport = 1;
    }
    at_response_free(p_response);

    RLOGI("init rid %d done", rid);
    if (getMainProtocolRid() == rid) {
        /// M: send condition signal when finishing initcallback @{
        isInitDone = 1;
        RLOGE("isInitDone = %d", isInitDone);
        /// @}
    }
}
#else   /* MTK_RIL */
static void initializeCallback(void *param)
{
    ATResponse *p_response = NULL;
    int err;

    setRadioState(RADIO_STATE_OFF);

    at_handshake();

    /* note: we don't check errors here. Everything important will
     * be handled in onATTimeout and onATReaderClosed */

    /*  atchannel is tolerant of echo but it must */
    /*  have verbose result codes */
    at_send_command("ATE0Q0V1", NULL);

    /*  No auto-answer */
    at_send_command("ATS0=0", NULL);

    /*  Extended errors */
    at_send_command("AT+CMEE=1", NULL);

    /*  Network registration events */
    err = at_send_command("AT+CREG=2", &p_response);

    /* some handsets -- in tethered mode -- don't support CREG=2 */
    if (err < 0 || p_response->success == 0)
        at_send_command("AT+CREG=1", NULL);

    at_response_free(p_response);

    /*  GPRS registration events */
    at_send_command("AT+CGREG=1", NULL);

    /*  Call Waiting notifications */
    at_send_command("AT+CCWA=1", NULL);

    /*  Alternating voice/data off */
    at_send_command("AT+CMOD=0", NULL);

    /*  Not muted */
    at_send_command("AT+CMUT=0", NULL);

    /*  +CSSU unsolicited supp service notifications */
    at_send_command("AT+CSSN=0,1", NULL);

    /*  no connected line identification */
    at_send_command("AT+COLP=0", NULL);

    /*  HEX character set */
    at_send_command("AT+CSCS=\"HEX\"", NULL);

    /*  USSD unsolicited */
    at_send_command("AT+CUSD=1", NULL);

    /*  Enable +CGEV GPRS event notifications, but don't buffer */
    at_send_command("AT+CGEREP=1,0", NULL);

    /*  SMS PDU mode */
    at_send_command("AT+CMGF=0", NULL);

#ifdef  USE_TI_COMMANDS
    at_send_command("AT%CPI=3", NULL);

    /*  TI specific -- notifications when SMS is ready (currently ignored) */
    at_send_command("AT%CSTAT=1", NULL);
#endif  /* USE_TI_COMMANDS */

    /* assume radio is off on error */
    if (isRadioOn() > 0) {
        setRadioState (RADIO_STATE_ON);
    }
}
#endif  /* MTK_RIL */

static void waitForClose()
{
    pthread_mutex_lock(&s_state_mutex);

    while (s_closed == 0)
        pthread_cond_wait(&s_state_cond, &s_state_mutex);

    pthread_mutex_unlock(&s_state_mutex);
}


/**
 * Called by atchannel when an unsolicited line appears
 * This is called on atchannel's reader thread. AT commands may
 * not be issued here
 */
static void onUnsolicited(const char *s, const char *sms_pdu, void *pChannel)
{
    char *line = NULL;
    int err;
    RIL_RadioState radioState = sState;
    RILChannelCtx *p_channel = (RILChannelCtx *)pChannel;

    if (RIL_SOCKET_2 == getRILIdByChannelCtx(p_channel)) {
        radioState = sState2;
    } else if (RIL_SOCKET_3 == getRILIdByChannelCtx(p_channel)) {
        radioState = sState3;
    } else if (RIL_SOCKET_4 == getRILIdByChannelCtx(p_channel)) {
        radioState = sState4;
    }

    /* Ignore unsolicited responses until we're initialized.
     * This is OK because the RIL library will poll for initial state
     */
    if ((radioState == RADIO_STATE_UNAVAILABLE) && isWorldModeSwitching(s)) {
        /// M: world mode switching, pending special URC. @{
        if (bWorldModeSwitching == 1) {
            cacheUrcIfNeeded(s, sms_pdu, p_channel);
        }
        /// @}
        return;
    }

    /// M: eMBMS feature
    if ((NULL != s)) {
        if (isEmbmsCommonUnsolicited(s)) {
            rilEmbmsCommonUnsolicited(s, p_channel);
        } else if (isEmbmsUnsolicited(s)) {
            rilEmbmsUnsolicited(s, p_channel);
            return;
        }
    }

    /* ATCI for unsolicited response */
    char atci_urc_enable[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.service.atci_urc.enable", atci_urc_enable, "0");
    if ((NULL != s) && (atoi(atci_urc_enable) == 1)) {
        RIL_onUnsolicitedResponse(RIL_UNSOL_ATCI_RESPONSE, s, strlen(s),
                getRILIdByChannelCtx(p_channel));
        if (isSMSUnsolicited(s)) {
            RIL_onUnsolicitedResponse(RIL_UNSOL_ATCI_RESPONSE, sms_pdu, strlen(sms_pdu),
                    getRILIdByChannelCtx(p_channel));
        }
    }

    /// notify MAL each raw data @{
    notifyMalRawData(s, sms_pdu, pChannel);
    /// @}

    if (!(rilNwUnsolicited(s, sms_pdu, p_channel) ||
          rilCcUnsolicited(s, sms_pdu, p_channel) ||
          rilSsUnsolicited(s, sms_pdu, p_channel) ||
          rilSmsUnsolicited(s, sms_pdu, p_channel) ||
          rilC2kSmsUnsolicited(s, sms_pdu, p_channel) ||
          rilStkUnsolicited(s, sms_pdu, p_channel) ||
          rilOemUnsolicited(s, sms_pdu, p_channel) ||
          rilImsUnsolicited(s, sms_pdu, p_channel) ||
          rilDataUnsolicited(s, sms_pdu, p_channel) ||
          rilSimUnsolicited(s, sms_pdu, p_channel) ||
          rilPhbUnsolicited(s, sms_pdu, p_channel) ||
          rilImsCcUnsolicited(s, p_channel) ))
        RLOGE("Unhandled unsolicited result code: %s\n", s);
}

#ifdef  MTK_RIL
/* Called on command or reader thread */
static void onATReaderClosed(RILChannelCtx *p_channel)
{
    RLOGI("AT channel closed\n");
    at_close(p_channel);
    assert(0);
    s_closed = 1;

    setRadioState(RADIO_STATE_UNAVAILABLE, getRILIdByChannelCtx(p_channel));
}

/* Called on command thread */
static void onATTimeout(RILChannelCtx *p_channel)
{
    RLOGI("AT channel timeout; closing\n");
    at_close(p_channel);
    assert(0);
    s_closed = 1;

    /* FIXME cause a radio reset here */

    setRadioState(RADIO_STATE_UNAVAILABLE, getRILIdByChannelCtx(p_channel));
}
#else   /* MTK_RIL */
/* Called on command or reader thread */
static void onATReaderClosed()
{
    RLOGI("AT channel closed\n");
    at_close();
    s_closed = 1;

    setRadioState(RADIO_STATE_UNAVAILABLE);
}

/* Called on command thread */
static void onATTimeout()
{
    RLOGI("AT channel timeout; closing\n");
    at_close();

    s_closed = 1;

    /* FIXME cause a radio reset here */

    setRadioState(RADIO_STATE_UNAVAILABLE);
}
#endif  /* MTK_RIL */

static void usage(char *s)
{
#ifdef  RIL_SHLIB
    fprintf(stderr, "reference-ril requires: %s -p <tcp port> or -d /dev/tty_device\n", s);
#else   /* RIL_SHLIB */
    fprintf(stderr, "usage: %s [-p <tcp port>] [-d /dev/tty_device]\n", s);
    exit(-1);
#endif  /* RIL_SHLIB */
}


#ifdef  MTK_RIL
/* These nodes are created by gsm0710muxd */
#define RIL_SUPPORT_CHANNELS_MAX_NAME_LEN 32
char s_mux_path[RIL_SUPPORT_CHANNELS][RIL_SUPPORT_CHANNELS_MAX_NAME_LEN];
char s_mux_path_init[RIL_SUPPORT_CHANNELS][RIL_SUPPORT_CHANNELS_MAX_NAME_LEN] = {
    "/dev/radio/pttynoti",
    "/dev/radio/pttycmd1",
    "/dev/radio/pttycmd2",
    "/dev/radio/pttycmd3",
    "/dev/radio/pttycmd4",
    "/dev/radio/atci1",
#ifdef MTK_MUX_CHANNEL_64
    "/dev/radio/pttycmd7",
    "/dev/radio/pttycmd8",
    "/dev/radio/pttycmd9",
    "/dev/radio/pttycmd10",
    "/dev/radio/pttycmd11",
#endif

    "/dev/radio/ptty2noti",
    "/dev/radio/ptty2cmd1",
    "/dev/radio/ptty2cmd2",
    "/dev/radio/ptty2cmd3",
    "/dev/radio/ptty2cmd4",
    "/dev/radio/atci2",
#ifdef MTK_MUX_CHANNEL_64
    "/dev/radio/ptty2cmd7",
    "/dev/radio/ptty2cmd8",
    "/dev/radio/ptty2cmd9",
    "/dev/radio/ptty2cmd10",
    "/dev/radio/ptty2cmd11",
#endif

    "/dev/radio/ptty3noti",
    "/dev/radio/ptty3cmd1",
    "/dev/radio/ptty3cmd2",
    "/dev/radio/ptty3cmd3",
    "/dev/radio/ptty3cmd4",
    "/dev/radio/atci3",
#ifdef MTK_MUX_CHANNEL_64
    "/dev/radio/ptty3cmd7",
    "/dev/radio/ptty3cmd8",
    "/dev/radio/ptty3cmd9",
    "/dev/radio/ptty3cmd10",
    "/dev/radio/ptty3cmd11",
#endif

    "/dev/radio/ptty4noti",
    "/dev/radio/ptty4cmd1",
    "/dev/radio/ptty4cmd2",
    "/dev/radio/ptty4cmd3",
    "/dev/radio/ptty4cmd4",
    "/dev/radio/atci4",
#ifdef MTK_MUX_CHANNEL_64
    "/dev/radio/ptty4cmd7",
    "/dev/radio/ptty4cmd8",
    "/dev/radio/ptty4cmd9",
    "/dev/radio/ptty4cmd10",
    "/dev/radio/ptty4cmd11",
#endif
};

void switchMuxPath() {
    RLOGD("switchMuxPath start");
    // restore muxpath
    memcpy(s_mux_path, s_mux_path_init, RIL_SUPPORT_CHANNELS*RIL_SUPPORT_CHANNELS_MAX_NAME_LEN);
    if (SIM_COUNT >= 2) {
        char prop_value[PROPERTY_VALUE_MAX] = { 0 };
        char s_mux_path_tmp[RIL_CHANNEL_OFFSET][RIL_SUPPORT_CHANNELS_MAX_NAME_LEN] = {{0}};
        int i;
        int buf_size = RIL_SUPPORT_CHANNELS_MAX_NAME_LEN;

        property_get(PROPERTY_3G_SIM, prop_value, "0");

        s_targetSim = atoi(prop_value);
        if (s_targetSim == 0) {
            s_targetSim = 1;
            property_set(PROPERTY_3G_SIM, "1");
        }
        RLOGD("targetSim : %d", s_targetSim);
        if (s_targetSim*RIL_CHANNEL_OFFSET > getSupportChannels()) {
            RLOGD("!!!! targetSim*RIL_CHANNEL_OFFSET > RIL_SUPPORT_CHANNELS");
        } else if (s_targetSim != 1) {
            // exchange mux channel for SIM switch
            for(i = 0; i < RIL_CHANNEL_OFFSET; i++) {
                int temp = (s_targetSim - 1) * RIL_CHANNEL_OFFSET + i;
                strncpy(s_mux_path_tmp[i], s_mux_path[temp], buf_size - 1);
                s_mux_path_tmp[i][buf_size - 1] = '\0';
                strncpy(s_mux_path[temp], s_mux_path[i], buf_size - 1);
                s_mux_path[temp][buf_size - 1] = '\0';
                strncpy(s_mux_path[i], s_mux_path_tmp[i], buf_size - 1);
                s_mux_path[i][buf_size - 1] = '\0';
            }
        }
        for(i = 0; i < getSupportChannels(); i++) {
            RLOGD("s_mux_path[%d] = %s", i, s_mux_path[i]);
        }
    }
    RLOGD("switchMuxPath end");
}

static void updateSimSwitchVersion() {
    if(SIM_COUNT > 1) {
        ATResponse *p_response = NULL;
        int err;

        err = at_send_command_singleline("AT+ESIMMAP?", "+ESIMMAP:", &p_response, getChannelCtxbyProxy());

        if (err < 0 || p_response->success == 0) {
            // do nothing
        } else {
            s_simSwitchVersion = 2;
        }

        at_response_free(p_response);
    }
}

static void updateEnhanceModemOffVersion() {
    ATResponse *p_response = NULL;
    int err = 0;
    char *line = NULL;

    err = at_send_command_multiline("AT+EPOF=?", "+EPOF:", &p_response,
            getChannelCtxbyProxy());
    if (p_response != NULL && p_response->p_intermediates != NULL) {
        line = p_response->p_intermediates->line;

        err = at_tok_start(&line);
        if (err < 0) goto error;

        err = at_tok_nextint(&line, &s_enhanceModemOffVersion);
        if (err < 0) goto error;
        RLOGD("updateEnhanceModemOffVersion: %d", s_enhanceModemOffVersion);
    }

    if (s_enhanceModemOffVersion == 0) {
        property_set("vendor.ril.cdma.enhance.version", "0");
    } else {
        property_set("vendor.ril.cdma.enhance.version", "1");
    }
    RIL_onUnsolicitedResponse(RIL_LOCAL_GSM_UNSOL_ENHANCED_MODEM_POWER,
            &s_enhanceModemOffVersion, sizeof(int), RIL_SOCKET_1);
error:
    at_response_free(p_response);
}

void sendRadioCapabilityDoneIfNeeded () {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    int sessionId = 0;
    int i = 0;

    property_get(PROPERTY_SET_RC_SESSION_ID[0], property_value, "-1");
    sessionId = atoi(property_value);
    RLOGI("sendRadioCapabilityDoneIfNeeded sessionId:%d", sessionId);
    //if (sessionId != 0) {
        RIL_RadioCapability* rc = (RIL_RadioCapability*) malloc(sizeof(RIL_RadioCapability));
        assert(rc != NULL);
        memset(rc, 0, sizeof(RIL_RadioCapability));
        rc->version = RIL_RADIO_CAPABILITY_VERSION;
        rc->session = sessionId;
        rc->phase = RC_PHASE_UNSOL_RSP;
        rc->status = RC_STATUS_SUCCESS;

        for (i = 0; i < SIM_COUNT; i++) {
            RLOGI("sendRadioCapabilityDoneIfNeeded, capability[%d] = %d", i, s_modemCapability[i]);
            rc->rat = s_modemCapability[i];
            strncpy(rc->logicalModemUuid, s_logicalModemId[i], MAX_UUID_LENGTH - 1);
            rc->logicalModemUuid[MAX_UUID_LENGTH - 1] = '\0';
            RIL_UNSOL_RESPONSE(RIL_UNSOL_RADIO_CAPABILITY, rc, sizeof(RIL_RadioCapability), i);
        }
    //}
}

void openChannelFds() {
    for (int i = 0; i < getSupportChannels(); i++) {
        openChannel(i, s_mux_path[i]);
    }
}

void openChannelFdsByChannelCtx(RILChannelCtx *p_channel, char *path) {
    struct timeval start;
    struct timeval end;
    int ret;
    while (p_channel->fd < 0) {
        do {
            p_channel->fd = open(path, O_RDWR);
        } while (p_channel->fd < 0 && errno == EINTR);

        if (p_channel->fd < 0) {
            perror("opening AT interface. retrying...");
            RLOGE("could not connect to %s: %s",
                  path, strerror(errno));
            // sleep(10);
            gettimeofday(&start, (struct timezone *) NULL);
            do {
                gettimeofday(&end, (struct timezone *) NULL);
            } while ((end.tv_sec - start.tv_sec) < 10);
            /* never returns */
        } else {
            struct termios ios;
            tcgetattr(p_channel->fd, &ios);
            ios.c_lflag = 0; /* disable ECHO, ICANON, etc... */
            ios.c_iflag = 0;
            tcsetattr(p_channel->fd, TCSANOW, &ios);
        }
    }
    s_closed = 0;
    ret = at_open(p_channel->fd, onUnsolicited, p_channel);
    if (ret < 0) {
        RLOGE("AT error %d on at_open\n", ret);
        return;
    }
}

extern void resetWakelock(void);

static void queryModemFeatureVersion(void)
{
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    ATLine *p_cur = NULL;
    char *line = NULL;
    int err;
    char *feature = NULL;
    int version;
    char *property_value = NULL;

    asprintf(&cmd, "AT+EMDVER?");
    err = at_send_command_multiline(cmd, "+EMDVER:", &p_response, getChannelCtxbyProxy());
    free(cmd);
    if (err == 0 && p_response->success != 0 && p_response->p_intermediates != NULL) {
        p_cur = p_response->p_intermediates;
        while (p_cur != NULL) {
            line = p_cur->line;
            err = at_tok_start(&line);
            err = at_tok_nextstr(&line, &feature);
            err = at_tok_nextint(&line, &version);
            if (0 == strcmp(feature, "DSS_NO_RESET")) {
                    if (version == 1) {
                        bDssNoResetSupport = true;
                    }
                    asprintf(&property_value, "%d", version);
                    property_set("vendor.ril.simswitch.no_reset_support", property_value);
                    free(property_value);
            } else if (0 == strcmp(feature, "WP Duplex Mode NO SIM RESET")) {
                asprintf(&property_value, "%d", version);
                property_set("ril.nw.wm.no_reset_support", property_value);
                free(property_value);
            }
            p_cur = p_cur->p_next;
        }
    }
    at_response_free(p_response);
}

static void *mainLoop(void *param __unused)
{
    int curr_share_modem = 0;
    char prop_value[PROPERTY_VALUE_MAX] = { 0 };
#ifdef MTK_MUX_CHANNEL_64
    RILSubSystemId subSysId = RIL_RADIO;
#else
    RILSubSystemId subSysId = RIL_DEFAULT;
#endif

    s_isUserLoad = isUserLoad();
    AT_DUMP("== ", "entering mainLoop()", -1);
    property_get("vendor.mux.debuglog.enable", prop_value, NULL);
    if (prop_value[0] == '1') {
        RLOGI("enable full log of mtk-ril.so");
            mtk_ril_log_level = 1;
    }

    at_set_on_reader_closed(onATReaderClosed);
    at_set_on_timeout(onATTimeout);
    resetWakelock();
    initRILChannels();
    getLogicModemId();
    syncModemProtocolCapability();
    switchMuxPath();
    setDynamicMsimConfig();
    setRildInterfaceCtrlSupport();
    initMalApi();
    registerMalRestartCallbacks();
    initMdStatusReader();
    while (s_main_loop) {
        openChannelFds();

        queryModemFeatureVersion();
        int sim3G = queryMainProtocol(subSysId);
        if ((sim3G != RIL_get3GSIM()) || (sim3G != s_targetSim)) {
            RLOGE("sync main protocol and resetradio, s_targetSim = %d", s_targetSim);
            setSimSwitchProp(sim3G);

            if (isCdmaLteDcSupport()) {
                char tmp[PROPERTY_VALUE_MAX] = {0};
                property_get("vendor.net.cdma.mdmstat", tmp, "not");
                //Wait for MD3 ready
                while(strncmp(tmp, "ready", 5) != 0) {
                    LOGI("vendor.net.cdma.mdmstat: %s", tmp);
                    sleep(1);
                    property_get("vendor.net.cdma.mdmstat", tmp, "not");
                }
                property_set("vendor.ril.mux.report.case", "2");
                property_set("vendor.ril.cdma.report.case", "1");
                property_set("vendor.ril.muxreport", "1");
            } else {
                resetRadio();
            }
            return 0;
        }

        /* restart MAL if modem reset while sim switching */
        property_get(PROPERTY_SIM_SWITCH_CONTROL_MAL, prop_value, "0");

        if (atoi(prop_value) == 0) {
            RLOGI("restart MAL if modem reset while sim switching");
            property_set(PROPERTY_SIM_SWITCH_CONTROL_MAL, "1");
        }

        updateSimSwitchVersion();
        updateEnhanceModemOffVersion();
        if (isDisableCapabilitySwitch() == 1) {
            if (sim3G != 1) {
                setSimSwitchProp(1);
                if (isCdmaLteDcSupport()) {
                    property_set("vendor.ril.mux.report.case", "2");
                    property_set("vendor.ril.cdma.report.case", "1");
                    property_set("vendor.ril.muxreport", "1");
                } else {
                    resetRadio();
                }
                return 0;
            }
        }

        // update T+W version
        updateTplusWVersion(getMainProtocolRid());
        queryKeep3GMode(getMainProtocolRid(), NULL);

        // update sim switch status when booting
        setSimSwitchProp(sim3G);
        // update bearer and send radio capability urc
        if (s_simSwitchVersion == 2) {
            queryActiveMode(getMainProtocolRid(), NULL);
        } else {
            int mdType = -1;
            triggerCCCIIoctlEx(CCCI_IOC_GET_MD_TYPE, &mdType);
            setActiveModemType(mdType);
        }
        queryBearer(subSysId, RIL_SOCKET_1);
        if (SIM_COUNT >= 2) {
            queryBearer(subSysId, RIL_SOCKET_2);
            if (SIM_COUNT >= 3) {
                queryBearer(subSysId, RIL_SOCKET_3);
                if (SIM_COUNT >= 4) {
                    queryBearer(subSysId, RIL_SOCKET_4);
                }
            }
        }
        sendRadioCapabilityDoneIfNeeded();
        if (sim3G >= CAPABILITY_3G_SIM2)
            s_isSimSwitched = 1;
        else
            s_isSimSwitched = 0;

        initialCidTable();
        //the first time use AT command to get the capability
        RLOGI("Is Sim Switch now: [%d], Sim SIM mapped for framework: [SIM%d]", s_isSimSwitched, sim3G);
        char* value = NULL;
        asprintf(&value, "%d", sim3G);
        property_set(PROPERTY_3G_SIM, value);
        free(value);
        curr_share_modem = getSimCount();
        const RIL_SOCKET_ID *pMainSimId;
        if (curr_share_modem >= 2) {
            // always init non-main protocol first
            switch (sim3G) {
                case CAPABILITY_3G_SIM2:
                    pMainSimId = &s_pollSimId[1];
                    break;
                case CAPABILITY_3G_SIM3:
                    pMainSimId = &s_pollSimId[2];
                    break;
                case CAPABILITY_3G_SIM4:
                    pMainSimId = &s_pollSimId[3];
                    break;
                default:
                case CAPABILITY_3G_SIM1:
                    pMainSimId = &s_pollSimId[0];
                    break;
            }
            RLOGI("Start initialized callback (MSMS - Main, %d)", *pMainSimId);
            RIL_requestProxyTimedCallback(initializeCallback, (void *)pMainSimId,
                    &TIMEVAL_0, getRILChannelId(subSysId, *pMainSimId), "initializeCallback");
            /* trigger other slots in the initializeCallback of main slot
            */
        } else {
            RLOGI("Start initialized callback (SS)");
            RIL_requestProxyTimedCallback(initializeCallback, (void *)&s_pollSimId[0],
                    &TIMEVAL_0, getRILChannelId(subSysId, s_pollSimId[0]), "initializeCallback");
        }

        // Give initializeCallback a chance to dispatched, since
        // we don't presently have a cancellation mechanism
        sleep(1);

        RLOGI("ril-proxy start");
        property_set("vendor.ril.rilproxy", "1");
        waitForClose();
        RLOGI("Re-opening after close");
    }
    RLOGI("Main loop exit");
    return 0;
}
#else   /* MTK_RIL */
static void *mainLoop(void *param)
{
    int fd;
    int ret;
    char path[50];
    int ttys_index;

    AT_DUMP("== ", "entering mainLoop()", -1);
    at_set_on_reader_closed(onATReaderClosed);
    at_set_on_timeout(onATTimeout);

    for (;; ) {
        fd = -1;
        while (fd < 0) {
            if (s_port > 0) {
                fd = socket_loopback_client(s_port, SOCK_STREAM);
            } else if (s_device_socket) {
                if (!strcmp(s_device_path, "/dev/socket/qemud")) {
                    /* Qemu-specific control socket */
                    fd = socket_local_client("qemud", ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
                    if (fd >= 0) {
                        char answer[2];

                        if (write(fd, "gsm", 3) != 3 ||
                            read(fd, answer, 2) != 2 ||
                            memcmp(answer, "OK", 2) != 0) {
                            close(fd);
                            fd = -1;
                        }
                    }
                } else {
                    fd = socket_local_client(s_device_path, ANDROID_SOCKET_NAMESPACE_FILESYSTEM, SOCK_STREAM);
                }
            } else if (s_device_path != NULL) {
                fd = open(s_device_path, O_RDWR);
                if (fd >= 0 && !memcmp(s_device_path, "/dev/ttyS", 9)) {
                    /* disable echo on serial ports */
                    struct termios ios;
                    tcgetattr(fd, &ios);
                    ios.c_lflag = 0; /* disable ECHO, ICANON, etc... */
                    ios.c_iflag = 0;
                    tcsetattr(fd, TCSANOW, &ios);
                }
            }
#if 0
            else if (s_device_range_begin >= 0 && s_device_range_end >= 0) {
                RLOGD("Open ttyS....");
                ttys_index = s_device_range_begin;
                while (ttys_index <= s_device_range_end) {
                    sprintf(path, "/dev/ttyS%d", ttys_index);
                    fd = open(path, O_RDWR);
                    if (fd >= 0) {
                        /* disable echo on serial ports */
                        struct termios ios;
                        tcgetattr(fd, &ios);
                        ios.c_lflag = 0; /* disable ECHO, ICANON, etc... */
                        ios.c_iflag = 0;
                        tcsetattr(fd, TCSANOW, &ios);
                    } else {
                        RLOGE("Can't open the device /dev/ttyS%d: %s", ttys_index, strerror(errno));
                    }
                    ttys_index++;
                }
            }
#endif

            if (fd < 0) {
                perror("opening AT interface. retrying...");
                sleep(10);
                /* never returns */
            }
        }

        RLOGD("FD: %d", fd);

        s_closed = 0;
        ret = at_open(fd, onUnsolicited);

        if (ret < 0) {
            RLOGE("AT error %d on at_open\n", ret);
            return 0;
        }

        RIL_requestTimedCallback(initializeCallback, NULL, &TIMEVAL_0);

        // Give initializeCallback a chance to dispatched, since we don't presently have a cancellation mechanism
        sleep(1);

        waitForClose();
        RLOGI("Re-opening after close");
    }
}
#endif  /* MTK_RIL */

#ifdef  MTK_RIL
RIL_RadioState getRadioState(RIL_SOCKET_ID rid)
{
    RIL_RadioState radioState = sState;

    if (RIL_SOCKET_2 == rid) {
        radioState = sState2;
    } else if (RIL_SOCKET_3 == rid) {
        radioState = sState3;
    } else if (RIL_SOCKET_4 == rid) {
        radioState = sState4;
    }

    RLOGI("getRadioState(): radioState=%d\n", radioState);

    return radioState;
}
#else   /* MTK_RIL */
RIL_RadioState getRadioState(void)
{
    return sState;
}
#endif  /* MTK_RIL */


#ifdef  MTK_RIL
void setRadioState(RIL_RadioState newState, RIL_SOCKET_ID rid)
#else
void setRadioState(RIL_RadioState newState)
#endif  /* MTK_RIL */
{
    RIL_RadioState oldState;
    RIL_RadioState *pState = NULL;

    pthread_mutex_lock(&s_state_mutex);

    oldState = sState;
    pState = &sState;
    if (RIL_SOCKET_2 == rid) {
        oldState = sState2;
        pState = &sState2;
    } else if (RIL_SOCKET_3 == rid) {
        oldState = sState3;
        pState = &sState3;
    } else if (RIL_SOCKET_4 == rid) {
        oldState = sState4;
        pState = &sState4;
    }

    if (s_closed > 0) {
        // If we're closed, the only reasonable state is
        // RADIO_STATE_UNAVAILABLE
        // This is here because things on the main thread
        // may attempt to change the radio state after the closed
        // event happened in another thread
        assert(0);
        newState = RADIO_STATE_UNAVAILABLE;
    }

    if (*pState != newState || s_closed > 0) {
        *pState = newState;
        assert(0);
        pthread_cond_broadcast(&s_state_cond);
    }

    pthread_mutex_unlock(&s_state_mutex);

    RLOGI("setRadioState(%d): newState=%d, oldState=%d, *pState=%d\n", rid, newState, oldState, *pState);

    /* do these outside of the mutex */
    if (*pState != oldState) {
        // Radio state changed to RADIO_STATE_UNAVAILABLE
        // Clear "STK service is running" flag in STK RIL
        if(*pState == RADIO_STATE_UNAVAILABLE) {
            setStkServiceRunningFlag(rid, false);
        }
        RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, NULL, 0, rid);

        // Sim state can change as result of radio state change
        RIL_UNSOL_RESPONSE(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0, rid);

        if (*pState == RADIO_STATE_ON) {
            onRadioPowerOn(rid);
        }
    }
}


#ifdef  RIL_SHLIB
pthread_t s_tid_mainloop;

const RIL_RadioFunctions *RIL_Init(const struct RIL_Env *env, int argc, char **argv)
{
    int ret;
    int fd = -1;
    int opt;
    int index;
    char *tmp;
    pthread_attr_t attr;
    char path[50];
    int ttys_index, i;

    s_rilenv = env;

    while (-1 != (opt = getopt(argc, argv, "p:d:s:m:"))) {
        switch (opt) {
            case 'p':
                s_port = atoi(optarg);
                if (s_port == 0) {
                    usage(argv[0]);
                    return NULL;
                }
                RLOGI("Opening loopback port %d\n", s_port);
                break;

            case 'd':
                s_device_path = optarg;
                RLOGI("Opening tty device %s\n", s_device_path);
                break;

            case 's':
                s_device_path = optarg;
                s_device_socket = 1;
                RLOGI("Opening socket %s\n", s_device_path);
                RLOGD("Got device!");
                break;
            case 'c':
                RLOGD("skip the argument");
                break;

#ifdef  MTK_RIL
            case 'm':
                RLOGD("Input range: %s %s %s", optarg, argv[1], argv[2]);

                tmp = strtok(optarg, " ");
                s_device_range_begin = atoi(tmp);
                tmp = strtok(NULL, " ");
                s_device_range_end = atoi(tmp);

                if ((s_device_range_end - s_device_range_begin + 1) != getSupportChannels()) {
                    RLOGE("We can't accept the input configuration for muliple channel since we need %d COM ports", getSupportChannels());
                    return NULL;
                }

                RLOGD("Open the ttyS%d to ttyS%d", s_device_range_begin, s_device_range_end);

                RLOGD("Link ttyS....");
                ttys_index = s_device_range_begin;
                i = 0;
                switchMuxPath();
                while (ttys_index <= s_device_range_end) {
                    sprintf(path, "/dev/ttyS%d", ttys_index);
                    RLOGD("Unlock %s on Link %s", path, s_mux_path[i]);
                    /*if(chmod(path, 0666) < 0 )
                     * {
                     *  RLOGD("chomod: system-error: '%s' (code: %d)", strerror(errno), errno);
                     *  return NULL;
                     * }*/
                    if (symlink(path, s_mux_path[i]) < 0) {
                        RLOGD("symlink: system-error: '%s' (code: %d)", strerror(errno), errno);
                        return NULL;
                    }
                    ttys_index++;
                    i++;
                }
                break;
#endif  /* MTK_RIL */

            default:
                usage(argv[0]);
                RLOGD("1- return null, %c", opt);
                //return NULL;
        }
    }

    if (s_port < 0 && s_device_path == NULL &&
        (s_device_range_begin < 0 || s_device_range_end < 0)) {
        usage(argv[0]);
        return NULL;
    }

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&s_tid_mainloop, &attr, mainLoop, NULL);

    return &s_callbacks;
}

const RIL_RadioFunctions *RIL_SAP_Init(const struct RIL_Env *env, int argc __unused,
        char **argv __unused) {
    s_rilsapenv = env;
    return &s_sapcallbacks;
}
#else   /* RIL_SHLIB */
int main(int argc, char **argv)
{
    int ret;
    int fd = -1;
    int opt;

    while (-1 != (opt = getopt(argc, argv, "p:d:"))) {
        switch (opt) {
            case 'p':
                s_port = atoi(optarg);
                if (s_port == 0)
                    usage(argv[0]);
                RLOGI("Opening loopback port %d\n", s_port);
                break;

            case 'd':
                s_device_path = optarg;
                RLOGI("Opening tty device %s\n", s_device_path);
                break;

            case 's':
                s_device_path = optarg;
                s_device_socket = 1;
                RLOGI("Opening socket %s\n", s_device_path);
                break;

            default:
                usage(argv[0]);
        }
    }

    if (s_port < 0 && s_device_path == NULL)
        usage(argv[0]);

    RIL_register(&s_callbacks);

    mainLoop(NULL);

    return 0;
}
#endif  /* RIL_SHLIB */

void setSimSwitchProp(int SimId)
{
    char* simSwitchValue = NULL;
    int tmpCapability;
    int oldMajorSimRid = RIL_get3GSIM() - 1;
    char tmpLogicalModemId[MAX_UUID_LENGTH];
    int info = SimId-1;

    if (SimId != 1) {
        s_isSimSwitched = 1;
    } else {
        s_isSimSwitched = 0;
    }

    if ((oldMajorSimRid != (SimId - 1)) && (oldMajorSimRid >= 0)) {
        // switch modem capability
        // restore to non-switch status
        if (oldMajorSimRid != 0) {
            tmpCapability = s_modemCapability[0];
            s_modemCapability[0] = s_modemCapability[oldMajorSimRid];
            s_modemCapability[oldMajorSimRid] = tmpCapability;
        }

        // switch to new status
        if (SimId != 1) {
            tmpCapability = s_modemCapability[0];
            s_modemCapability[0] = s_modemCapability[SimId-1];
            s_modemCapability[SimId-1] = tmpCapability;
        }

        // switch modem ID
        // restore to non-switch status
        if (oldMajorSimRid != 0) {
            strncpy(tmpLogicalModemId, s_logicalModemId[0], MAX_UUID_LENGTH - 1);
            tmpLogicalModemId[MAX_UUID_LENGTH - 1] = '\0';
            strncpy(s_logicalModemId[0], s_logicalModemId[oldMajorSimRid], MAX_UUID_LENGTH - 1);
            s_logicalModemId[0][MAX_UUID_LENGTH - 1] = '\0';
            strncpy(s_logicalModemId[oldMajorSimRid], tmpLogicalModemId, MAX_UUID_LENGTH - 1);
            s_logicalModemId[oldMajorSimRid][MAX_UUID_LENGTH - 1] = '\0';
        }

        // switch to new status
        if (SimId != 1) {
            strncpy(tmpLogicalModemId, s_logicalModemId[0], MAX_UUID_LENGTH - 1);
            tmpLogicalModemId[MAX_UUID_LENGTH - 1] = '\0';
            strncpy(s_logicalModemId[0], s_logicalModemId[SimId - 1], MAX_UUID_LENGTH - 1);
            s_logicalModemId[0][MAX_UUID_LENGTH - 1] = '\0';
            strncpy(s_logicalModemId[SimId - 1], tmpLogicalModemId, MAX_UUID_LENGTH - 1);
            s_logicalModemId[SimId - 1][MAX_UUID_LENGTH - 1] = '\0';
        }
    }

    asprintf(&simSwitchValue, "%d", SimId);
    property_set(PROPERTY_3G_SIM, simSwitchValue);
    free(simSwitchValue);

    //notify the MAIN SIM status change by RIL_SOCKET_1
    // feature phase, TBD
    //RIL_UNSOL_RESPONSE(RIL_UNSOL_MAIN_SIM_INFO, &info, sizeof(int), RIL_SOCKET_1);
}

static char *gettaskName(char *buf, int size, FILE *file)
{
    int cnt = 0;
    int eof = 0;
    int eol = 0;
    int c;

    if (size < 1) {
        return NULL;
    }

    while (cnt < (size - 1)) {
        c = getc(file);
        if (c == EOF) {
            eof = 1;
            break;
        }

        *(buf + cnt) = c;
        cnt++;

        if (c == '\n') {
            eol = 1;
            break;
        }
    }

    /* Null terminate what we've read */
    *(buf + cnt) = '\0';

    if (eof) {
        if (cnt) {
            return buf;
        } else {
            return NULL;
        }
    } else if (eol) {
        return buf;
    } else {
        /* The line is too long.  Read till a newline or EOF.
         * If EOF, return null, if newline, return an empty buffer.
         */
        while(1) {
            c = getc(file);
            if (c == EOF) {
                return NULL;
            } else if (c == '\n') {
                *buf = '\0';
                return buf;
            }
        }
    }
    return NULL;
}


int findPid(char* name)
{
    int pid = -1;

    if (name != NULL) {
        RLOGI("Find PID of %s <process name>", name);
    } else {
        return pid;
    }

    const char* directory = "/proc";
    size_t      taskNameSize = 256;
    char*       taskName = calloc(taskNameSize, sizeof(char));
    if (taskName == NULL) {
        RLOGE("OOM");
        return 0;
    }
    DIR* dir = opendir(directory);

    if (dir) {
        struct dirent* de = 0;

        while ((de = readdir(dir)) != 0) {
            if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
                continue;

            int res = sscanf(de->d_name, "%d", &pid);

            if (res == 1) {
                // we have a valid pid

                // open the cmdline file to determine what's the name of the process running
                char cmdline_file[256] = {0};
                sprintf(cmdline_file, "%s/%d/cmdline", directory, pid);

                FILE* cmdline = fopen(cmdline_file, "r");

                if (gettaskName(taskName, taskNameSize, cmdline) != NULL) {
                    // is it the process we care about?
                    if (strstr(taskName, name) != 0) {
                        RLOGI("%s process, with PID %d, has been detected.", name, pid);
                        // just let the OS free this process' memory!
                        fclose(cmdline);
                        free(taskName);
                        closedir(dir);
                        return pid;
                    }
                }

                fclose(cmdline);
            }
        }

        closedir(dir);
    }

    // just let the OS free this process' memory!
    free(taskName);

    return -1;
}

/* To check if new SS service class feature is supported or not */
void determineSSServiceClassFeature(RIL_SOCKET_ID rid __unused, RILChannelCtx* p_channel) {
    ATResponse *p_response = NULL;
    int err;

    bNewSsServiceClassFeature = 0;

    /* Temporary solution */
    /*err = at_send_command("AT+CESQ=?",  &p_response, getDefaultChannelCtx(rid));
    LOGI("AT+CESQ=?, got err= %d, success=%d, finalResponse = %s", err, p_response->success, p_response->finalResponse);

    if ((err == 0 && p_response->success == 1) && (strncmp(p_response->finalResponse,"OK", 2) == 0)) {
       bNewSsServiceClassFeature = 1;
    }
    at_response_free(p_response);
    */


    err = at_send_command("AT+ECUSD?",  &p_response, p_channel);
    if (p_response != NULL) {
        RLOGI("AT+ECUSD?, got err= %d, success=%d, finalResponse = %s", err, p_response->success, p_response->finalResponse);

        if ((err == 0 && p_response->success == 1) && (strncmp(p_response->finalResponse,"OK", 2) == 0)) {
           bNewSsServiceClassFeature = 1;
        }
        at_response_free(p_response);
    } else {
        RLOGD("Could not get result of AT+ECUSD?.");
    }

    RLOGI("bNewSsServiceClassFeature = %d", bNewSsServiceClassFeature);
}
/// @}

int handleAee(const char *modem_warning, const char *modem_version) {
#ifdef HAVE_AEE_FEATURE
    return aee_modem_warning("Modem Warning", NULL, DB_OPT_DUMMY_DUMP, modem_warning, modem_version);
#else
    LOGD("[handleOemUnsolicited]HAVE_AEE_FEATURE is not defined");
    return 1;
#endif
}

/// MAL interface URC support @{
void notifyMalRawData(const char *s, const char *sms_pdu, void *pChannel) {
    int urcToMalLength = 0;
    char *line = NULL;
    RILChannelCtx *p_channel = (RILChannelCtx *)pChannel;

    int rid = getRILIdByChannelCtx(p_channel);
    if (isMalSupported()) {
        // M: dual ims or dual ps => send raw urc to each slot
        bool sendRawUrc = false;
        if (isMultipleImsSupport() || isMultiPsAttachSupport()) {
            sendRawUrc = true;
        } else {
            if (rid == getMainProtocolRid()) {
                sendRawUrc = true;
            }
        }
        if(sendRawUrc && s != NULL && !strStartsWith(s, "+EIND: 128")) {
            if (isSMSUnsolicited(s)){
                // 5 for 2 \r\n and 1 \0
                urcToMalLength = strlen(s) + strlen(sms_pdu) + 9;
                line = (char *) calloc(1, urcToMalLength);
                if (line == NULL) {
                    RLOGE("OOM");
                    return;
                }
                strncpy(line, "\r\n", 2);
                strncat(line, s, strlen(s));
                strncat(line, "\r\n", 2);
                strncat(line, "\r\n", 2);
                strncat(line, sms_pdu, strlen(sms_pdu));
                strncat(line, "\r\n\0", 3);
            } else {
                urcToMalLength = strlen(s) + 5;
                line = (char *) calloc(1, urcToMalLength);
                if (line == NULL) {
                    RLOGE("OOM");
                    return;
                }
                strncpy(line, "\r\n", 2);
                strncat(line, s, strlen(s));
                strncat(line, "\r\n\0", 3);
            }
            cacheRawUrcToMal(MAL_MD_ID_GSM, rid, line);
        }
    }
}

MalUrc* initMalUrc(int type, int rid, char* line) {
    MalUrc* urc = (MalUrc*)malloc(sizeof(MalUrc));
    if (urc == NULL) {
        RLOGE("OOM");
        return NULL;
    }
    urc -> type = type;
    urc -> rid = rid;
    urc -> line = line;
    urc -> nextUrc = NULL;
    return urc;
}

static void *malUrcLoop(void* params __unused) {
    MalUrc* urcHead = malUrcList;
    int ret = -1;
    while(true) {

        pthread_mutex_lock(&malUrcListMutex);

        if ((urcHead->nextUrc) != NULL) {
            // always send out the first urc in the list
            MalUrc* urcSendToMal = urcHead -> nextUrc;
            if (!skipIratUrcToMal(urcSendToMal->line)) {
                // This function should not be blocking
                if (mal_mdmngr_send_urc != NULL) {
                    ret = mal_mdmngr_send_urc(mal_once(2, mal_cfg_type_md_id, urcSendToMal->type,
                            mal_cfg_type_sim_id, urcSendToMal->rid), urcSendToMal->line);
                    RLOGD("[%s] Call mal_mdmngr_send_urc success",__FUNCTION__);
                } else {
                    RLOGE("[%s] mal_mdmngr_send_urc is null",__FUNCTION__);
                }

                if (ret == MAL_SUCC) {
                    // RLOGI("[GSM MAL] URC send success, rid %d, ret %d: %s", urcSendToMal->rid,
                    //       ret, urcSendToMal->line);

                    urcHead -> nextUrc = urcSendToMal -> nextUrc;
                    if (urcSendToMal -> line != NULL) {
                        free(urcSendToMal-> line);
                    }
                    free(urcSendToMal);
                } else {
                    RLOGI("[GSM MAL] URC send fail, rid %d, ret %d: %s", urcSendToMal->rid,
                           ret, urcSendToMal->line);
                }
            }
        } else {
            // RLOGV("[GSM MAL] list empty now, wait for condition");
            pthread_cond_wait(&malUrcLlistNotEmptyCond, &malUrcListMutex);
        }

        pthread_mutex_unlock(&malUrcListMutex);

        if (ret != MAL_SUCC) {
            RLOGI("[GSM MAL] send fail, rest for 100ms");
            usleep(100000);
        }
    }
    return NULL;
}

void cacheRawUrcToMal(int type, int rid, char* line) {

    // lazily initialize the linked list and thread
    if (malUrcList == NULL) {
        malUrcList = initMalUrc(-1, -1, NULL);
    }

    if (mal_urc_dispatch_thread == 0) {
        RLOGI("[GSM MAL] init mal working thread");
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        int result = pthread_create(&mal_urc_dispatch_thread, &attr, malUrcLoop, NULL);
        if (result != 0) {
            RLOGE("[GSM MAL] Failed to create dispatch thread: %s", strerror(result));
            return;
        }
    }

    pthread_mutex_lock(&malUrcListMutex);
    // RLOGV("[GSM MAL] cache URC rid %d, %s", rid, line);


    MalUrc* urcListTail = malUrcList;

    while ((urcListTail->nextUrc) != NULL){
        urcListTail = urcListTail -> nextUrc;
    }

    MalUrc* newUrc = initMalUrc(type, rid, line);
    urcListTail -> nextUrc = newUrc;

    pthread_cond_broadcast(&malUrcLlistNotEmptyCond);
    pthread_mutex_unlock(&malUrcListMutex);

}
/// @}

/// M: SAP start
static void sendSapResponseComplete(RIL_Token t __unused, RIL_Errno ret __unused
        , MsgId msgId __unused, void *data __unused) {
    const pb_field_t *fields = NULL;
    size_t encoded_size = 0;
    uint32_t written_size = 0;
    size_t buffer_size = 0;
    pb_ostream_t ostream;
    bool success = false;
    ssize_t written_bytes;
    int i = 0;

    RLOGD("sendSapResponseComplete, start (%d)", msgId);

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
            RLOGE("sendSapResponseComplete, MsgId is mistake!");
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
            RLOGD("sendSapResponseComplete, Size: %zu (0x%zx) Size as written: 0x%x",
                encoded_size, encoded_size, written_size);
            // Send response
            RIL_SAP_onRequestComplete(t, ret, buffer, buffer_size);
        } else {
            RLOGE("sendSapResponseComplete, Encode failed!");
        }
    } else {
        RLOGE("Not sending response type %d: encoded_size: %zu. encoded size result: %d",
        msgId, encoded_size, success);
    }
}
/// SAP end

static void syncModemProtocolCapability() {
    char prop_value[PROPERTY_VALUE_MAX] = { 0 };
    // If ps1 doesn't support W, we need to remove it in ps2,3,4
    if(!RatConfig_isWcdmaSupported()) {
        property_get("persist.vendor.radio.mtk_ps2_rat", prop_value, "G");
        if (strchr(prop_value, 'W') != NULL) {
            if (strchr(prop_value, 'L') != NULL) {
                property_set("persist.vendor.radio.mtk_ps2_rat", "L/G");
            } else {
                property_set("persist.vendor.radio.mtk_ps2_rat", "G");
            }
        }
        property_get("persist.vendor.radio.mtk_ps3_rat", prop_value, "G");
        if (strchr(prop_value, 'W') != NULL) {
            if (strchr(prop_value, 'L') != NULL) {
                property_set("persist.vendor.radio.mtk_ps3_rat", "L/G");
            } else {
                property_set("persist.vendor.radio.mtk_ps3_rat", "G");
            }
        }
        property_get("persist.vendor.radio.mtk_ps4_rat", prop_value, "G");
        if (strchr(prop_value, 'W') != NULL) {
            if (strchr(prop_value, 'L') != NULL) {
                property_set("persist.vendor.radio.mtk_ps4_rat", "L/G");
            } else {
                property_set("persist.vendor.radio.mtk_ps4_rat", "G");
            }
        }
    }
}


/// M: world mode switching, pending special URC. @{
void cacheUrcIfNeeded(const char *s, const char *sms_pdu, RILChannelCtx *p_channel) {
    // Only the URC list we wanted.
    if (strStartsWith(s, "+ECOPS:") || strStartsWith(s, "+EMSR:")
            || strStartsWith(s, "+ESIMAPP:")) {
        const RIL_SOCKET_ID socket_id = getRILIdByChannelCtx(p_channel);
        RLOGD("cacheUrc: RILD:%d, %s", socket_id, s);

        UrcList* urcCur = NULL;
        UrcList* urcPrev = NULL;
        int pendedUrcCount = 0;

        urcCur = pendedUrcList;
        while (urcCur != NULL) {
            // RLOGD("cacheUrc: Pended URC:%d, RILD:%d, %s", pendedUrcCount, urcCur->rid, urcCur->urc);
            urcPrev = urcCur;
            urcCur = (UrcList*)(urcCur->pNext);
            pendedUrcCount++;
        }
        urcCur = (UrcList*)calloc(1, sizeof(UrcList));
        if (urcCur == NULL) {
            RLOGE("OOM");
            return;
        }
        if (urcPrev != NULL) {
            urcPrev->pNext = urcCur;
        }
        urcCur->pNext = NULL;
        urcCur->rid = socket_id;
        urcCur->urc = strdup(s);
        if (sms_pdu != NULL) {
            urcCur->sms_pdu = strdup(sms_pdu);
        } else {
            urcCur->sms_pdu = NULL;
        }
        if (pendedUrcCount == 0) {
            pendedUrcList = urcCur;
        }
        RLOGD("cacheUrc: Current pendedUrcCount = %d", pendedUrcCount + 1);
    }
}

static void sendUrc(void *param) {
    UrcList* urclist = (UrcList*)param;

    const RIL_SOCKET_ID rid = (RIL_SOCKET_ID) (urclist->rid);
    RILChannelCtx* p_channel = getChannelCtxbyProxy();
    RLOGD("sendUrc: RILD:%d %s, %s", rid, urclist->urc, urclist->sms_pdu);

    onUnsolicited(urclist->urc, urclist->sms_pdu, p_channel);

    free(urclist->urc);
    free(urclist->sms_pdu);
    free(urclist);
}

void sendPendedUrcs() {
    RLOGD("sendPendedUrcs: bWorldModeSwitching=%d", bWorldModeSwitching);
    if (bWorldModeSwitching == 0 && pendedUrcList != NULL) {
        RLOGD("sendPendedUrcs: Ready to send pended URCs");
        UrcList* urc = pendedUrcList;
        UrcList* urc_next;
        while (urc != NULL) {
            urc_next = (UrcList*)(urc->pNext);
            // RLOGD("sendUrcs: RILD:%d, %s", urc->rid, urc->urc);
            RIL_requestProxyTimedCallback(sendUrc, urc, &TIMEVAL_0,
                    getChannelCtxbyProxy()->id, "sendUrc");
            urc = urc_next;
            if (urc == NULL) {
                RLOGD("sendUrcs: urc->pNext == NULL");
            }
        }
        pendedUrcList = NULL;
    } else if (pendedUrcList == NULL) {
        RLOGD("sendPendedUrcs: pendedUrcList == NULL");
    }
}

static void setDynamicMsimConfig() {
    char msim_old[PROPERTY_VALUE_MAX] = { 0 };
    char prop_value[PROPERTY_VALUE_MAX] = { 0 };
    // update dynamic multisim config

    // sync persist.radio.multisim.config and persist.vendor.radio.msimmode
    property_get("persist.radio.multisim.config", msim_old, "dsds");
    property_set("persist.vendor.radio.msimmode", msim_old);

/*
    const char *pSysenv = sysenv_get_static("msim_config");

    RLOGD("sysenv_get_static: %s\n", pSysenv);
    if (pSysenv != NULL) {
        // for META tool
        if (strcmp(pSysenv, "dsds") == 0) {
            property_set("persist.vendor.radio.msimmode", "dsds");
            property_set("persist.vendor.radio.multisimslot", "2");
        } else if (strcmp(pSysenv, "ss") == 0) {
            property_set("persist.vendor.radio.msimmode", "ss");
            property_set("persist.vendor.radio.multisimslot", "1");
        }
    } else
*/
    {
        // for EM
        if (strcmp(msim_old, "dsda") != 0) {
            // get ccci setting first, if not set then get EM setting.
            property_get("ro.boot.opt_sim_count", prop_value, "0");
            if (prop_value[0] == '0') {
                property_get("persist.vendor.radio.multisimslot", prop_value, "0");
            }
            if (prop_value[0] == '1') {
                property_set("persist.vendor.radio.msimmode", "ss");
            } else if (prop_value[0] == '2') {
                property_set("persist.vendor.radio.msimmode", "dsds");
            } else if (prop_value[0] == '3') {
                property_set("persist.vendor.radio.msimmode", "tsts");
            } else if (prop_value[0] == '4') {
                property_set("persist.vendor.radio.msimmode", "qsqs");
            }
        }
    }
}

int isEactsSupport() {
    return sEactsSupport;
}

void updateTplusWVersion(RIL_SOCKET_ID rid) {
    ATResponse *p_response = NULL;
    int err;
    char *cmd = NULL;
    char* line = 0;
    int mode = 0;

    // check if support TplusW
    if (getMainProtocolRid() == rid) {
        asprintf(&cmd, "AT+ESBP=7,\"SBP_T_PLUS_W\"");
        err = at_send_command_singleline(cmd, "+ESBP:", &p_response, getChannelCtxbyProxy());
        if (err == 0 && p_response->success != 0 && p_response->p_intermediates != NULL) {
            line = p_response->p_intermediates->line;
            err = at_tok_start(&line);
            if (err >= 0) {
                err = at_tok_nextint(&line, &mode);
                LOGD("tplusw mode = %d", mode);
                if (0 == mode) {
                    property_set("vendor.ril.simswitch.tpluswsupport", "0");
                    LOGD("don't support tplusw mode");
                } else if (1 == mode) {
                    property_set("vendor.ril.simswitch.tpluswsupport", "1");
                    LOGD("support tplusw mode");
                } else {
                    LOGD("AT+ESBP=7,\"SBP_T_PLUS_W\", tpluswmode error : %d", mode);
                }
            } else {
                LOGD("AT+ESBP=7 response value error: %d", err);
                property_set("vendor.ril.simswitch.tpluswsupport", "-1");
            }
        } else {
            LOGD("does not support SBP_T_PLUS_W, error info: %d, %d, %d",
                 err,p_response->success, p_response->p_intermediates);
            property_set("vendor.ril.simswitch.tpluswsupport", "-1");
        }
        free(cmd);
        at_response_free(p_response);
    }
}
