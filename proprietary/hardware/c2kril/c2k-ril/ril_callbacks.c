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

/* //hardware/viatelecom/ril/viatelecom-withuim-ril/viatelecom-withuim-ril.c
**
** Copyright 2009, Viatelecom Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <telephony/ril_cdma_sms.h>
#include "librilutils.h"
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <alloca.h>
#include "atchannel.h"
#include "at_tok.h"
#include "misc.h"
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <termios.h>
#include <sys/system_properties.h>
#include <libmtkrilutils.h>
#include <mal.h>

#include <qemu_pipe.h>

#define LOG_TAG "RIL"
#include <utils/Log.h>

#include "hardware/ccci_intf.h"
#include <cutils/properties.h>
#ifdef C2K_RIL_LEGACY
  #include <com_intf.h>
#endif
#include <linux/serial.h>
#include <hardware/ril/librilutils/proto/sap-api.pb.h>
#include <dlfcn.h>
#include <ctype.h>

#include "ril_callbacks.h"
#include "atchannel.h"
#include "at_tok.h"
#include "misc.h"
#include "hardware/ccci_intf.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "atchannel_config.h"
#include <ratconfig.h>
#include "ril_cc.h"
#include "ril_nw.h"
#include "ril_sms.h"
#include "ril_wp.h"
#include <libmtkrilutils.h>
#include "ril_radio.h"
#include "c2k_ril_data.h"
#include "ril_sim.h"

#ifndef UNUSED
// Eliminate "warning: unused parameter"
#define UNUSED(x) ((void)(x))
#endif

#ifdef  RIL_SHLIB
const struct RIL_Env *s_rilenv;
#endif  /* RIL_SHLIB */

#define SYS_DATA_CHANNEL "/sys/class/usb_rawbulk/data/enable"
#define SYS_ETS_CHANNEL  "/sys/class/usb_rawbulk/ets/enable"
#define SYS_AT_CHANNEL   "/sys/class/usb_rawbulk/atc/enable"
#define SYS_PCV_CHANNEL  "/sys/class/usb_rawbulk/pcv/enable"
#define SYS_GPS_CHANNEL  "/sys/class/usb_rawbulk/gps/enable"

#define SYSFS_USB_DRIVER_OLD    "/sys/bus/usb/drivers/option/"
#define SYSFS_USB_DRIVER_NEW    "/sys/bus/usb/drivers/via-cbp/"

#define GW_SIGNAL_DEFAULT          0
#define GW_BIT_ERR_RATE_DEFAULT    0
#define CDMA_DBM_DEFAULT           55
#define EVDO_DBM_DEFAULT           65

#ifdef ANDROID_KK

ModemInfo *sMdmInfo;

static int is3gpp2(int radioTech) {
    switch (radioTech) {
        case RADIO_TECH_IS95A:
        case RADIO_TECH_IS95B:
        case RADIO_TECH_1xRTT:
        case RADIO_TECH_EVDO_0:
        case RADIO_TECH_EVDO_A:
        case RADIO_TECH_EVDO_B:
        case RADIO_TECH_EHRPD:
            return 1;
        default:
            return 0;
    }
}
#endif /* ANDROID_KK */

// M: For multi channel support
#define DEFAULT_CHANNEL_CTX getRILChannelCtxFromToken(t)
// All channels' device sys property list

/** Dlopen handling function **/
void *dlopenHandlerForMalApi;
void *getDlopenHandler(const char *libPath);
void *getAPIbyDlopenHandler(void *dlHandler, const char *apiName);
void initMalApi();
int requestedTlvMemoryOfMalApi[MALRIL_API_INDEX_MAX];

static char s_mux_path[RIL_SUPPORT_CHANNELS][32];

int inemergency = 0;

#define EVDO_ECIO_DEFAULT 750
#define EVDO_SNR_DEFAULT 8
#define CDMA_ECIO_DEFAULT 90

/* cache for unsolicited message from BP */
RIL_UNSOL_Msg_Cache s_unsol_msg_cache = {
    0, 0, 0, 1, 1, 1,
    1,
    "", "",
    "",
    {
        { GW_SIGNAL_DEFAULT, GW_BIT_ERR_RATE_DEFAULT, },
        { CDMA_DBM_DEFAULT, CDMA_ECIO_DEFAULT, },
        { EVDO_DBM_DEFAULT, EVDO_ECIO_DEFAULT, EVDO_SNR_DEFAULT, },
        { 0, 44, 20, 0, 0, },
    },
    0, 0,
    2, 0, 0, 0
};

int uimInsertedStatus = UIM_STATUS_INITIALIZER;


int cta_no_uim = 0;
// framework wants to enable ps
int s_ps_on_desired = 0;
//0: not yet, 1: initialized
int uim_modual_initialized = 0;
//0: not yet, 1: initialized
int iccid_readable = 0;
static pthread_mutex_t s_vser_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_signal_repoll_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_signal_change_mutex = PTHREAD_MUTEX_INITIALIZER;
int s_wait_repoll_flag = 0; /*1: waiting for repoll, so return a fake value*/
const struct timeval TIMEVAL_NETWORK_REPOLL = { 4, 0 }; /* 4 second */
static int s_port = -1;
static const char * s_device_path = NULL;
const char s_data_device_path[32];
static char atpath[32];
static char datapath[32];

#ifdef FLASHLESS_SUPPORT
static char* atDevPath = NULL;
static char* dataDevPath = NULL;

#ifdef AT_AUTO_SCRIPT_RESET
static int noLoop = 0;
#endif /* AT_AUTO_SCRIPT_RESET */

#endif /* FLASHLESS_SUPPORT */

extern const char * requestToString(int request);

int voicetype = 0;
int invoicecall = 0;
/* trigger change to this with s_state_cond */
int s_closed = 0;

//TODO: SIM/Nw both access this variabe, should extract set/get APIs
/*Indicate how many  times registration queried since last time no service*/
int no_service_times = 0;

/*fix HANDROID#2225,if unlock pin-code more than 3 times, service_state should set to 0(out of service)*/
int unlock_pin_outtimes = 0;

#ifdef RIL_SHLIB
const struct RIL_Env *s_rilenv;
#endif

#define RIL_onRequestComplete(t, e, response, responselen) s_rilenv->OnRequestComplete(t,e, response, responselen)
#define RIL_onUnsolicitedResponse(a,b,c) s_rilenv->OnUnsolicitedResponse(a,b,c)
#define RIL_requestTimedCallback(a,b,c) s_rilenv->RequestTimedCallback(a,b,c)


static RIL_RadioState sState = RADIO_STATE_UNAVAILABLE;

static pthread_mutex_t s_state_mutex = PTHREAD_MUTEX_INITIALIZER;
static const struct timeval TIMEVAL_UPDATE_CACHE = {10, 0};   /* 10second */
static pthread_cond_t s_state_cond = PTHREAD_COND_INITIALIZER;
static int sRuimState = RUIM_NOT_READY;
static int          s_device_socket = 0;

/*Indicate current card type, initialized by "UIMST:" URC in onUnsolicited function*/
int cardtype = 0;

static const struct timeval TIMEVAL_SIMPOLL = {1,0};
static const struct timeval TIMEVAL_CALLSTATEPOLL = {0,500000};
static const struct timeval TIMEVAL_0 = {0,0};

int arsithrehd = -1; //arsi threshold value, anywhere sent to set cp should use this value
static int arsithrehddef = 2; //default arsi threshold value, only used in atcommand_init() func

int deactivedata_inprogress = 0;

#ifdef ANDROID_KK
static int s_ims_registered  = 0;        // 0==unregistered
static int s_ims_services    = 1;        // & 0x1 == sms over ims supported
static int s_ims_format    = 1;          // FORMAT_3GPP(1) vs FORMAT_3GPP2(2);
static int s_ims_cause_retry = 0;        // 1==causes sms over ims to temp fail
static int s_ims_cause_perm_failure = 0; // 1==causes sms over ims to permanent fail
static int s_ims_gsm_retry   = 0;        // 1==causes sms over gsm to temp fail
static int s_ims_gsm_fail    = 0;        // 1==causes sms over gsm to permanent fail
#endif

static int s_cell_info_rate_ms = INT_MAX;
static int s_mcc = 0;
static int s_mnc = 0;
static int s_lac = 0;
static int s_cid = 0;

static UrcList* pendedUrcList = NULL;  // World Phone cache Urc list when World Phone is changing

void setRadioState(RIL_RadioState newState);
static void setRadioTechnology(ModemInfo *mdm, int newtech);
static int query_ctec(ModemInfo *mdm, int *current, int32_t *preferred);
static int parse_technology_response(const char *response, int *current, int32_t *preferred);

/**
 * M: Fix RILD NE after reset: AT channel init handling in main looper, and RIL Request
 * handling in proxy looper, if both looper send AT command, conflict will happen
 */
pthread_mutex_t s_at_init_Mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t s_at_init_Cond = PTHREAD_COND_INITIALIZER;
int s_at_init_finish = 0;
pthread_t mainloop_thread_id;

#ifndef C2K_RIL_LEGACY
/**
 * M: RILD should send AT command after MD3 +VPUP URC reported; add a wait/notify mechanism
 * to make sure this sequence flow.
 */
pthread_mutex_t s_md3_init_Mutex;
pthread_cond_t s_md3_init_Cond;
int s_md3_init_finish = 0;
#endif

extern int s_closed;
extern int processing_md5_cmd;
extern int s_md3_off;

int s_isUserLoad = 0;

extern void trigger_update_cache();

// For multi channel support
extern void initRILChannels(void);
extern void openRILChannels(char mux_path[][32], ATUnsolHandler h);

static void onRequest (int request, void *data, size_t datalen, RIL_Token t);
#if defined(ANDROID_MULTI_SIM)
static void onSapRequest(int request, void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID socket_id);
#else
static void onSapRequest(int request, void *data, size_t datalen, RIL_Token t);
#endif

static RIL_RadioState currentState();
static int onSupports (int requestCode);
static void onCancel (RIL_Token t);
static const char *getVersion();

/*** Static Variables ***/
static RIL_RadioFunctions s_callbacks = {
    RIL_VERSION,
    onRequest,
    currentState,
    onSupports,
    onCancel,
    getVersion
};

const struct RIL_Env *s_rilsapenv;
static const RIL_RadioFunctions s_sapcallbacks = {
    RIL_VERSION,
    onSapRequest,
    currentState,
    onSupports,
    onCancel,
    getVersion
};

static void atcommand_init();
static int isRadioOn();
RUIM_Status getRUIMStatus(RADIO_STATUS_UPDATE isReportRadioStatus);
int waitUimModualInitialized(void);
static void onATReaderClosed(RILChannelId channel_id);
static void autoEnterPinCode(void);
void setIccidProperty(VIA_ICCID_TYPE type, char* pIccid);
static void onRadioPowerOn();
void waitAtInitDone();
void pollRUIMState (void *param);
void sendSapResponseComplete(RIL_Token t, RIL_Errno ret, MsgId msgId, void *data);
extern RILChannelCtx *getChannelCtxbyProxy();
// M: [VzW] Data Framework
void syncDisabledApnToMd();

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
static void
onRequest (int request, void *data, size_t datalen, RIL_Token t)
{
    // Wait until AT channel initialization finished
    waitAtInitDone();

    if (s_md3_off && request != RIL_REQUEST_MODEM_POWERON
            && request != RIL_REQUEST_SET_MODEM_THERMAL
            && request != RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE
            && request != RIL_REQUEST_RESET_RADIO
            && request != RIL_REQUEST_SHUTDOWN
            && request != RIL_REQUEST_MODEM_POWEROFF
            && request != RIL_LOCAL_REQUEST_MODEM_POWEROFF_SYNC_C2K
            /// M: [C2K]Dynamic switch support. @{
            && request != RIL_REQUEST_ENTER_RESTRICT_MODEM_C2K
            && request != RIL_REQUEST_LEAVE_RESTRICT_MODEM_C2K
            /// @}
            && request != RIL_LOCAL_C2K_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE
            && request != RIL_REQUEST_RESTART_RILD
            ){
        RLOGD("MD off and ignore %s", requestToString(request));
        RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
        return;
    }

    LOGD("C2K onRequest: %s, sState= %d",
            requestToString(request), getRadioState());

    /* Ignore all requests except RIL_REQUEST_GET_SIM_STATUS
     * when RADIO_STATE_UNAVAILABLE.
     */
    if (getRadioState() == RADIO_STATE_UNAVAILABLE
        && !(request == RIL_REQUEST_GET_SIM_STATUS
                    || request == RIL_REQUEST_ENTER_SIM_PIN
                    || request == RIL_REQUEST_ENTER_SIM_PUK
                    || request == RIL_REQUEST_SET_RADIO_CAPABILITY
                    || request == RIL_REQUEST_GET_RADIO_CAPABILITY
                    || request == RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE
                    || request == RIL_LOCAL_C2K_REQUEST_SWITCH_CARD_TYPE
                    /// M: [C2K]Dynamic switch support. @{
                    || request == RIL_REQUEST_ENTER_RESTRICT_MODEM_C2K
                    || request == RIL_REQUEST_LEAVE_RESTRICT_MODEM_C2K
                    /// @}
                    || request == RIL_REQUEST_SET_MODEM_THERMAL
                    || request == RIL_REQUEST_RESET_RADIO
                    || request == RIL_REQUEST_SHUTDOWN
                    || request == RIL_REQUEST_MODEM_POWEROFF
                    || request == RIL_REQUEST_MODEM_POWERON
                    || request == RIL_LOCAL_REQUEST_MODEM_POWEROFF_SYNC_C2K
                    || request == RIL_REQUEST_CONFIG_MODEM_STATUS_C2K
                    || request == RIL_REQUEST_ALLOW_DATA
                    || request == RIL_LOCAL_C2K_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE
                    /// M: [C2K][IR] Support SVLTE IR feature. @{
                    || request == RIL_REQUEST_SET_REG_SUSPEND_ENABLED_CDMA
                    || request == RIL_REQUEST_RESUME_REGISTRATION_CDMA
                    /// M: [C2K][IR] Support SVLTE IR feature. @}
                    || request == RIL_REQUEST_SET_DATA_PROFILE
                    || request == RIL_LOCAL_REQUEST_MODE_SWITCH_C2K_SET_TRM
                    || request == RIL_REQUEST_SET_TRM
                    || request == RIL_REQUEST_OEM_HOOK_RAW
                    || request == RIL_REQUEST_DEVICE_IDENTITY
                    /// M: World Phone(91/92) switching, need pass special request. @{
                    || allowForWorldModeSwitching(request)
                    /// @}
                    /// M: CC: Switch HPF
                    || request == RIL_LOCAL_C2K_REQUEST_SWITCH_HPF
                    /// M: AGPSD APN
                    || request == RIL_LOCAL_C2K_UNSOL_VIA_GPS_EVENT
                    || request == RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER
                    || request == RIL_REQUEST_RESTART_RILD
                    )
    ) {
        RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
        return;
    }

    /* Ignore all non-power requests when RADIO_STATE_OFF
     * (except RIL_REQUEST_GET_SIM_STATUS)
     */
    if (getRadioState() == RADIO_STATE_OFF
            && !(request == RIL_REQUEST_RADIO_POWER
                    || request == RIL_REQUEST_GET_SIM_STATUS
                    || request == RIL_REQUEST_ENTER_SIM_PUK
                    || request == RIL_REQUEST_CDMA_SUBSCRIPTION
                    || request == RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE
                    || request == RIL_REQUEST_GET_IMSI
                    || request == RIL_REQUEST_SIM_OPEN_CHANNEL
                    || request == RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC
                    || request == RIL_REQUEST_SIM_CLOSE_CHANNEL
                    || request == RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL
                    || request == RIL_REQUEST_SIM_GET_ATR
                    || request == RIL_REQUEST_SET_RADIO_CAPABILITY
                    || request == RIL_REQUEST_GET_RADIO_CAPABILITY
                    || request == RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE
                    /// M: [C2K 6M][NW] Config EVDO mode @{
                    || request == RIL_LOCAL_C2K_REQUEST_CONFIG_EVDO_MODE
                    /// M: [C2K 6M][NW] Config EVDO mode @}
                    /// M: [Network][C2K] For handle roaming preference. @{
                    || request == RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE
                    || request == RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE
                    /// @}
                    || request == RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM
                    || request == RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM
                    || request == RIL_REQUEST_REPORT_SMS_MEMORY_STATUS
                    || request == RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION
                    || request == RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG
                    || request == RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG
                    || request == RIL_REQUEST_GET_SMSC_ADDRESS
                    || request == RIL_REQUEST_SET_SMSC_ADDRESS
                    || request == RIL_REQUEST_GET_SMS_RUIM_MEM_STATUS
                    /// M: [C2K][IR] Support CT 3g dual Mode card IR feature. @{
                    || request == RIL_LOCAL_C2K_REQUEST_SET_SVLTE_RAT_MODE
                    /// M: [C2K][IR] Support CT 3g dual Mode card IR feature. @}
                    || request == RIL_REQUEST_SIM_IO
                    || request == RIL_REQUEST_ENTER_SIM_PIN
                    || request == RIL_REQUEST_ENTER_SIM_PUK
                    || request == RIL_REQUEST_ENTER_SIM_PIN2
                    || request == RIL_REQUEST_ENTER_SIM_PUK2
                    || request == RIL_REQUEST_CHANGE_SIM_PIN
                    || request == RIL_REQUEST_CHANGE_SIM_PIN2
                    || request == RIL_REQUEST_QUERY_FACILITY_LOCK
                    || request == RIL_REQUEST_SET_FACILITY_LOCK
                    || request == RIL_REQUEST_GET_ALLOWED_CARRIERS
                    || request == RIL_REQUEST_SET_ALLOWED_CARRIERS
                    || request == RIL_REQUEST_ALLOW_DATA
                    || request == RIL_LOCAL_C2K_REQUEST_CONFIRM_INTER_3GPP_IRAT_CHANGE
                    || request == RIL_REQUEST_START_LCE
                    || request == RIL_REQUEST_STOP_LCE
                    || request == RIL_REQUEST_PULL_LCEDATA
                    || request == RIL_REQUEST_SET_LINK_CAPACITY_REPORTING_CRITERIA
                    || request == RIL_LOCAL_C2K_REQUEST_SWITCH_CARD_TYPE
                    /// M: [C2K]Dynamic switch support. @{
                    || request == RIL_REQUEST_ENTER_RESTRICT_MODEM_C2K
                    || request == RIL_REQUEST_LEAVE_RESTRICT_MODEM_C2K
                    /// @}
                    || request == RIL_REQUEST_SET_MODEM_THERMAL
                    || request == RIL_REQUEST_RESET_RADIO
                    || request == RIL_REQUEST_SHUTDOWN
                    || request == RIL_REQUEST_MODEM_POWEROFF
                    || request == RIL_REQUEST_MODEM_POWERON
                    || request == RIL_LOCAL_REQUEST_MODEM_POWEROFF_SYNC_C2K
                    || request == RIL_REQUEST_CONFIG_MODEM_STATUS_C2K
                    /// M: [C2K][IR] Support SVLTE IR feature. @{
                    || request == RIL_REQUEST_SET_REG_SUSPEND_ENABLED_CDMA
                    || request == RIL_REQUEST_RESUME_REGISTRATION_CDMA
                    /// M: [C2K][IR] Support SVLTE IR feature. @}
                    || request == RIL_REQUEST_SET_DATA_PROFILE
                    || request == RIL_LOCAL_REQUEST_MODE_SWITCH_C2K_SET_TRM
                    || request == RIL_REQUEST_SET_TRM
                    || request == RIL_REQUEST_OEM_HOOK_RAW
                    || request == RIL_REQUEST_DEVICE_IDENTITY
                    || request == RIL_REQUEST_BASEBAND_VERSION
                    || request == RIL_REQUEST_OEM_HOOK_STRINGS
                    || request == RIL_LOCAL_C2K_REQUEST_AT_COMMAND_WITH_PROXY_CDMA
                    /// M: CC: Switch HPF
                    || request == RIL_LOCAL_C2K_REQUEST_SWITCH_HPF
                    /// M: [Network][C2K] Sprint roaming control @{
                    || request == RIL_REQUEST_SET_ROAMING_ENABLE
                    || request == RIL_REQUEST_GET_ROAMING_ENABLE
                    /// @}
                    /// M: STK. {
                    || request == RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND
                    || request == RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE
                    || request == RIL_REQUEST_STK_SET_PROFILE
                    || request == RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING
                    || request == RIL_LOCAL_C2K_REQUEST_SET_UTK_MODE
                    /// M: STK. }
                    || request == RIL_REQUEST_SEND_DEVICE_STATE
                    /// M: PHB start
                    || request == RIL_REQUEST_QUERY_PHB_STORAGE_INFO
                    || request == RIL_REQUEST_READ_PHB_ENTRY
                    || request == RIL_REQUEST_WRITE_PHB_ENTRY
                    /// M: PHB end
                    || request == RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER
                    || request == RIL_REQUEST_ENABLE_MODEM
                    || request == RIL_REQUEST_RESTART_RILD
                    )
    ) {
        RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
        return;
    }
    /*If now is processing md5 commands, ignore other ril commands*/
    if (processing_md5_cmd)
    {
        LOGD("processing md5 command = %d, ignore other ril commands except md5!!!",
                processing_md5_cmd);
        RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
        return;
    }

    if (!(rilNwMain(request, data, datalen, t)
            || rilDataMain(request, data, datalen, t)
            || rilCcMain(request, data, datalen, t)
            || rilSimMain(request, data, datalen, t)
            || rilSmsMain(request, data, datalen, t)
            || rilUtkMain(request, data, datalen, t)
            || rilOemMain(request, data, datalen, t)
            || rilRadioMain(request, data, datalen, t)
            || rilPhbMain(request, data, datalen, t)))
    {
        RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
    }
}

#if defined(ANDROID_MULTI_SIM)
static void onSapRequest(int request, void *data, size_t datalen, RIL_Token t,
        RIL_SOCKET_ID socket_id) {
#else
static void onSapRequest(int request, void *data, size_t datalen, RIL_Token t) {
#endif
    RLOGD("onSapRequest: %d", request);
    if (request < MsgId_RIL_SIM_SAP_CONNECT /* MsgId_UNKNOWN_REQ */ ||
            request > MsgId_RIL_SIM_SAP_SET_TRANSFER_PROTOCOL) {
        RLOGD("invalid request");
        RIL_SIM_SAP_ERROR_RSP rsp;
        rsp.dummy_field = 1;
        sendSapResponseComplete(t, (RIL_Errno)Error_RIL_E_REQUEST_NOT_SUPPORTED,
                MsgId_RIL_SIM_SAP_ERROR_RESP, &rsp);
        return;
    }

    if (s_md3_off) {
        RLOGD("MD off and reply failure to Sap message");
        RIL_SIM_SAP_CONNECT_RSP rsp;
        rsp.response = RIL_SIM_SAP_CONNECT_RSP_Response_RIL_E_SAP_CONNECT_FAILURE;
        rsp.has_max_message_size = false;
        rsp.max_message_size = 0;
        sendSapResponseComplete(t, (RIL_Errno)Error_RIL_E_RADIO_NOT_AVAILABLE,
                MsgId_RIL_SIM_SAP_CONNECT, &rsp);
        return;
    }
#if defined(ANDROID_MULTI_SIM)
    if (!(rilBtSapMain(request, data, datalen, t, socket_id) ||
            rilStkBtSapMain(request, data, datalen, t, socket_id))) {
#else
    if (!(rilBtSapMain(request, data, datalen, t, 0) ||
            rilStkBtSapMain(request, data, datalen, t, 0))) {
#endif

        RIL_SIM_SAP_ERROR_RSP rsp;
        rsp.dummy_field = 1;
        sendSapResponseComplete(t, (RIL_Errno)Error_RIL_E_REQUEST_NOT_SUPPORTED,
                MsgId_RIL_SIM_SAP_ERROR_RESP, &rsp);
    }
}

/**
 * Synchronous call from the RIL to us to return current radio state.
 * RADIO_STATE_UNAVAILABLE should be the initial state.
 */
static RIL_RadioState
currentState()
{
    return getRadioState();
}
/**
 * Call from RIL to us to find out whether a specific request code
 * is supported by this implementation.
 *
 * Return 1 for "supported" and 0 for "unsupported"
 */

static int
onSupports (int requestCode __unused)
{
    //@@@ todo

    return 1;
}

static void onCancel (RIL_Token t __unused)
{
    //@@@todo

}

#ifdef ANDROID_KK
/**
 * Parse the response generated by a +CTEC AT command
 * The values read from the response are stored in current and preferred.
 * Both current and preferred may be null. The corresponding value is ignored in that case.
 *
 * @return: -1 if some error occurs (or if the modem doesn't understand the +CTEC command)
 *          1 if the response includes the current technology only
 *          0 if the response includes both current technology and preferred mode
 */
int parse_technology_response( const char *response, int *current, int32_t *preferred )
{
    int err;
    char *line, *p;
    int ct;
    int32_t pt = 0;
    char *str_pt;

    line = p = strdup(response);
    LOGD("Response: %s", line);
    err = at_tok_start(&p);
    if (err || !at_tok_hasmore(&p)) {
        LOGD("err: %d. p: %s", err, p);
        free(line);
        return -1;
    }

    err = at_tok_nextint(&p, &ct);
    if (err) {
        free(line);
        return -1;
    }
    if (current) *current = ct;

    RLOGD("line remaining after int: %s", p);

    err = at_tok_nexthexint(&p, &pt);
    if (err) {
        free(line);
        return 1;
    }
    if (preferred) {
        *preferred = pt;
    }
    free(line);

    return 0;
}

int query_supported_techs( ModemInfo *mdm __unused, int *supported )
{
    ATResponse *p_response;
    int err, val, techs = 0;
    char *tok;
    char *line;

    LOGD("query_supported_techs");
    err = at_send_command_singleline("AT+CTEC=?", "+CTEC:", &p_response, DEFAULT_CHANNEL_CTX);
    if (err || !p_response->success)
        goto error;
    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err || !at_tok_hasmore(&line))
        goto error;
    while (!at_tok_nextint(&line, &val)) {
        techs |= ( 1 << val );
    }
    if (supported) *supported = techs;
    return 0;
error:
    at_response_free(p_response);
    return -1;
}

/**
 * query_ctec. Send the +CTEC AT command to the modem to query the current
 * and preferred modes. It leaves values in the addresses pointed to by
 * current and preferred. If any of those pointers are NULL, the corresponding value
 * is ignored, but the return value will still reflect if retreiving and parsing of the
 * values suceeded.
 *
 * @mdm Currently unused
 * @current A pointer to store the current mode returned by the modem. May be null.
 * @preferred A pointer to store the preferred mode returned by the modem. May be null.
 * @return -1 on error (or failure to parse)
 *         1 if only the current mode was returned by modem (or failed to parse preferred)
 *         0 if both current and preferred were returned correctly
 */
int query_ctec(ModemInfo *mdm __unused, int *current, int32_t *preferred)
{
    ATResponse *response = NULL;
    int err;
    int res;

    LOGD("query_ctec. current: %d, preferred: %d", (int)current, (int) preferred);
    err = at_send_command_singleline("AT+CTEC?", "+CTEC:", &response, DEFAULT_CHANNEL_CTX);
    if (!err && response->success) {
        res = parse_technology_response(response->p_intermediates->line, current, preferred);
        at_response_free(response);
        return res;
    }
    LOGE("Error executing command: %d. response: %x. status: %d", err, (int)response, response? response->success : -1);
    at_response_free(response);
    return -1;
}

int is_multimode_modem(ModemInfo *mdm)
{
    ATResponse *response;
    int err;
    char *line;
    int tech;
    int32_t preferred;

    if (query_ctec(mdm, &tech, &preferred) == 0) {
        mdm->currentTech = tech;
        mdm->preferredNetworkMode = preferred;
        if (query_supported_techs(mdm, &mdm->supportedTechs)) {
            return 0;
        }
        return 1;
    }
    return 0;
}

/**
 * Find out if our modem is GSM, CDMA or both (Multimode)
 */
static void probeForModemMode(ModemInfo *info)
{
    ATResponse *response;
    int err;
    assert (info);
    // Currently, our only known multimode modem is qemu's android modem,
    // which implements the AT+CTEC command to query and set mode.
    // Try that first

    if (is_multimode_modem(info)) {
        LOGI("Found Multimode Modem. Supported techs mask: %8.8x. Current tech: %d",
            info->supportedTechs, info->currentTech);
        return;
    }

    /* Being here means that our modem is not multimode */
    info->isMultimode = 0;

    /* CDMA Modems implement the AT+WNAM command */
    err = at_send_command_singleline("AT+WNAM","+WNAM:", &response, DEFAULT_CHANNEL_CTX);
    if (!err && response->success) {
        at_response_free(response);
        // TODO: find out if we really support EvDo
        info->supportedTechs = MDM_CDMA | MDM_EVDO;
        info->currentTech = MDM_CDMA;
        LOGI("Found CDMA Modem");
        return;
    }
    if (!err) at_response_free(response);
    // TODO: find out if modem really supports WCDMA/LTE
    info->supportedTechs = MDM_GSM | MDM_WCDMA | MDM_LTE;
    info->currentTech = MDM_GSM;
    LOGI("Found GSM Modem");
}
#endif

static const char * getVersion(void)
{
    static char version_info[50] = { 0 };
    sprintf(version_info, "Viatelecom-cdma-ril suffix Version-%s",
            VIA_SUFFIX_VERSION);
    return version_info;
}

void trigger_update_cache()
{
    LOGD("trigger_update_cache");
    at_send_command("AT^SYSINFO", NULL, getChannelCtxbyProxy());
    at_send_command("AT+CSQ?", NULL, getChannelCtxbyProxy());
    at_send_command("AT+HDRCSQ?", NULL, getChannelCtxbyProxy());
    at_send_command("AT+CREG?", NULL, getChannelCtxbyProxy());
    at_send_command("AT+VMCCMNC?", NULL, getChannelCtxbyProxy());
    at_send_command("AT+VSER?", NULL, getChannelCtxbyProxy());
    at_send_command("AT+EIPRL?", NULL, getChannelCtxbyProxy());
    at_send_command("AT+EFCELL?", NULL, getChannelCtxbyProxy());
}

static bool isChipTestMode() {
    char prop_val[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.vendor.chiptest.enable", prop_val, "0");
    int chipMode = atoi(prop_val);

    if (chipMode == 1) {
        RLOGD("chipmode: %d", chipMode);
        RLOGD("MAL socket is set to ril-proxy-mal");
        return true;
    }
    return false;
}

int isMalSupported()
{
    static int is_mal_supported = -1;
    if (is_mal_supported == -1) {
        char ims_prop[PROPERTY_VALUE_MAX] = {0};
        char epdg_prop[PROPERTY_VALUE_MAX] = {0};
        //MAL is depended on ims or epdg
        property_get("persist.vendor.ims_support", ims_prop, "0");
        property_get("persist.vendor.mtk_wfc_support", epdg_prop, "0");
        if (!strcmp(ims_prop, "1") || !strcmp(epdg_prop, "1")) {
            is_mal_supported = 1;
        } else {
            is_mal_supported = 0;
        }
    }
    return is_mal_supported;
}

int getRUIMState(void) {
    return sRuimState;
}

/** Returns SIM_NOT_READY on error */
RUIM_Status getRUIMStatus(RADIO_STATUS_UPDATE isReportRadioStatus)
{
    UNUSED(isReportRadioStatus);
    ATResponse *p_response = NULL;
    int err;
    int ret;
    char *cpinLine;
    char *cpinResult = NULL;
    PS_PARAM psParam;
    memset(&psParam, 0, sizeof(psParam));
    char boottimes[PROPERTY_VALUE_MAX] = { 0 };

    property_get("vendor.net.cdma.boottimes", boottimes, "0");
    if ((getRadioState() == RADIO_STATE_UNAVAILABLE) && (strcmp(boottimes, "2") != 0)) {
        ret = RUIM_NOT_READY;
        goto done;
    }

    err = at_send_command_singleline("AT+CPIN?", "+CPIN:", &p_response, getChannelCtxbyProxy());
    if (err != 0)
    {
        ret = RUIM_NOT_READY;
        goto done;
    }
    if (cta_no_uim)
    {
        LOGD("fake value uim status");
        // if (isChipTestMode()) {
        //    RLOGD("chip test mode, report radio state changed");
        //    setRadioState(RADIO_STATE_RUIM_LOCKED_OR_ABSENT);
        // }
        ret = RUIM_ABSENT;
        goto done;
    }
    switch (at_get_cme_error(p_response)) {
        case CME_SUCCESS:
            break;

        case CME_SIM_NOT_INSERTED:
            ret = RUIM_ABSENT;
#ifndef OPEN_PS_ON_CARD_ABSENT
            // psParam.enable = 0;
            // turnPSEnable((void *)&psParam);
#endif /* OPEN_PS_ON_CARD_ABSENT */
            goto done;
        case CME_SIM_FAILURE:
            ret = RUIM_NOT_READY;
            goto done;
        case CME_SIM_BUSY:
            ret = RUIM_BUSY;
            goto done;
        default:
            ret = RUIM_NOT_READY;
            goto done;
    }

    /* CPIN? has succeeded, now look at the result */

    cpinLine = p_response->p_intermediates->line;
    err = at_tok_start(&cpinLine);

    if (err < 0)
    {
        ret = RUIM_NOT_READY;
        goto done;
    }

    err = at_tok_nextstr(&cpinLine, &cpinResult);

    if (err < 0)
    {
        ret = RUIM_NOT_READY;
        goto done;
    }

    if (0 == strcmp(cpinResult, "SIM PIN"))
    {
        ret = RUIM_PIN;
        //LOGD("sState = %d",sState);
        //setRadioState(RADIO_STATE_RUIM_LOCKED_OR_ABSENT);
#ifndef OPEN_PS_ON_CARD_ABSENT
        // if (getRadioState() == RADIO_STATE_RUIM_LOCKED_OR_ABSENT)
        // {
        //     psParam.enable = 0;
        //     turnPSEnable((void *)&psParam);
        // }
#endif /* OPEN_PS_ON_CARD_ABSENT */
        goto done;
    }
    else if (0 == strcmp(cpinResult, "SIM PUK"))
    {
#ifndef OPEN_PS_ON_CARD_ABSENT
        // if (getRadioState() == RADIO_STATE_RUIM_LOCKED_OR_ABSENT)
        // {
        //     psParam.enable = 0;
        //     turnPSEnable((void *)&psParam);
        // }
#endif /* OPEN_PS_ON_CARD_ABSENT */
        ret = RUIM_PUK;
        goto done;
    }
    else if (0 == strcmp(cpinResult, "UIM Error"))
    {
        LOGD("UIM Error");
        ret = RUIM_NOT_READY; //cwen add uim error notification that UI not display no card
        goto done;
    } else if (0 == strcmp(cpinResult, "CARD_REBOOT")) {
        LOGD("CARD_REBOOT");
        ret = RUIM_CARD_REBOOT;
        goto done;
    } else if (0 == strcmp(cpinResult, "CARD_RESTRICTED")) {
        LOGD("CARD_RESTRICTED");
        ret = RUIM_RESTRICTED;
        goto done;
    } else if (0 != strcmp(cpinResult, "READY")) {
        /* we're treating unsupported lock types as "sim absent" */
        ret = RUIM_ABSENT;
        goto done;
    }
    at_response_free(p_response);
    p_response = NULL;
    cpinResult = NULL;

    /* ALPS01977096: To align with GSM to set READY if Radio is NOT UNAVAILABLE, 2015/03/12 {*/
    //TODO: To complete align with GSM to remove Radio ON check

    if (1 == isRadioOn()) {
        // LOGD("getUIMStatus isReportRadioStatus = %d",  isReportRadioStatus);
        // if (UPDATE_RADIO_STATUS == isReportRadioStatus) {
               // cwen add CPOF before pin is checked so that it can find the net work after
               // check pin
        //     setRadioState(RADIO_STATE_RUIM_READY);
        // }
        ret = RUIM_READY;
    } else if (RADIO_STATE_UNAVAILABLE == getRadioState()) {
        LOGD("%s: Radio state: %d", __FUNCTION__, getRadioState());
        ret = RUIM_NOT_READY;
    } else {
        LOGD("%s:UIM_READY", __FUNCTION__);
        ret = RUIM_READY;
    }
    /*  ALPS01977096: To align with GSM to set READY if Radio is NOT UNAVAILABLE, 2015/03/12 }*/

done:
    at_response_free(p_response);
    LOGD("%s: ret = %d", __FUNCTION__, ret);
    sRuimState = ret;
    return ret;
}

static void atcommand_init()
{
    ATResponse *p_response = NULL;

    char *cmd = NULL;
    char boottimes[PROPERTY_VALUE_MAX] = { 0 };
    int err;
    int flight_mode = -1;
    int testSilentReboot = -1;
    int modemEE  = -1;
    PS_PARAM psParam;
    memset(&psParam, 0, sizeof(psParam));
    /**
     * note: we don't check errors here. Everything important will
     * be handled in onATTimeout and onATReaderClosed
     */

    /*  No auto-answer */
    at_send_command("ATS0=0", NULL, getChannelCtxbyProxy());

    /*  Extended errors */
    at_send_command("AT+CMEE=1", NULL, getChannelCtxbyProxy());

    /*  Network registration events */
    err = at_send_command("AT+CREG=2", &p_response, getChannelCtxbyProxy());

    /* some handsets -- in tethered mode -- don't support CREG=2 */
    if (err < 0 || p_response->success == 0) {
        at_send_command("AT+CREG=1", NULL, getChannelCtxbyProxy());
    }

    at_response_free(p_response);

    if (arsithrehd < 0)
    {
        arsithrehd = arsithrehddef;
    }
    LOGD("ARSI value:%d", arsithrehd);
    asprintf(&cmd, "AT+ARSI=1,%d", arsithrehd);
    at_send_command(cmd, NULL, getChannelCtxbyProxy());

    /*Send URC +VMCCMNC when MCC/MNC changed in network */
    at_send_command("AT+VMCCMNC=1", NULL, getChannelCtxbyProxy());

    /*disable CBP wakeup AP because of ETS message*/
    at_send_command("AT+VUSBETS=0", NULL, getChannelCtxbyProxy());

    /* set cp rm interface protocol - Relay Layer Rm interface, PPP */
    at_send_command("AT+CRM=1", NULL, getChannelCtxbyProxy());


    /* Config IRAT mode: 0 AP IRAT(default), 1 MD IRAT, 2 MD IRAT LCG */
    if (getSvlteProjectType() >= SVLTE_PROJ_SC_3M
            && getSvlteProjectType() <= SVLTE_PROJ_SC_6M) {
        at_send_command("AT+EIRATMODE=0", NULL, getChannelCtxbyProxy());
    } else if (isCdmaLteDcSupport()) {
        if (isSvlteCdmaOnlySetFromEngMode() == 1) {
            at_send_command("AT+EIRATMODE=3", NULL, getChannelCtxbyProxy());
            // M: [VzW] Data Framework
            // Try to disable VZW APN if it needs
            syncDisabledApnToMd();
        } else if (isSvlteLcgSupport()) {
            at_send_command("AT+EIRATMODE=2", NULL, getChannelCtxbyProxy());
        } else {
            at_send_command("AT+EIRATMODE=1", NULL, getChannelCtxbyProxy());
        }
    }

    /*enable +VSER URC*/
    at_send_command("AT+VSER=1", NULL, getChannelCtxbyProxy());

    /*add for new service_state mechanism*/
    at_send_command("AT^PREFMODE?", NULL, getChannelCtxbyProxy());

    property_get("vendor.net.cdma.boottimes", boottimes, "0");
    LOGD("%s: boottimes is %s", __FUNCTION__, boottimes);
    if(!strcmp(boottimes,"2")) {
        if (isCdmaLteDcSupport() == 1) {
        LOGD("C2K rild init, svlte boottimes=2 skip autoEnterPinCode");
        } else {
            autoEnterPinCode();
        }
    }

    ///for test broadcast when user trigger the modem ,send EBOOT command
    property_get("vendor.ril.cdma.report.case", boottimes, "0");
    testSilentReboot = atoi(boottimes);

    ///for modem EE
    property_get("vendor.ril.cdma.report", boottimes, "0");
    modemEE = atoi(boottimes);

    ///for when user unlock the sim pin and enter flight mode
    property_get("vendor.cdma.ril.eboot", boottimes, "0");
    flight_mode = atoi(boottimes);

    LOGD("%s: testSilentReboot:%d, flight_mode:%d, modemEE:%d.",
            __FUNCTION__, testSilentReboot, flight_mode, modemEE);
    if (testSilentReboot == 1 || flight_mode == 1 || modemEE == 1) {
        LOGD("vendor.cdma.ril.eboot = 1 ,send command AT+EBOOT=1");
        at_send_command("AT+EBOOT=1", NULL, getChannelCtxbyProxy());
    } else {
        LOGD("vendor.cdma.ril.eboot = 0 ,send command AT+EBOOT=0");
        at_send_command("AT+EBOOT=0", NULL, getChannelCtxbyProxy());
    }
    property_set("vendor.cdma.ril.eboot", "0");
    property_set("vendor.ril.cdma.report.case", "0");
    ///for modem EE
    property_set("vendor.ril.cdma.report", "0");

    // RIL data initialization.
    rilDataInitialization(getChannelCtxbyProxy());

    /// M: CC: GSA HD Voice for 2/3G network support @{
    char hdVoiceEnabled[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.radio.hd.voice", hdVoiceEnabled, "1");
    if ((atoi(hdVoiceEnabled) == 1)) {
        at_send_command("AT+EVOCD=1", NULL, getChannelCtxbyProxy());
    } else {
        at_send_command("AT+EVOCD=0", NULL, getChannelCtxbyProxy());
    }
    /// @}

    /*enable femtocell URC*/
    if (isFemtocellSupport()) {
        at_send_command("AT+EFCELL=1", NULL, getChannelCtxbyProxy());
    }
}

// M: [VzW] Data Framework @{
void syncDisabledApnToMd() {
    FILE* fPtr = fopen("/mobile_info/vzwapn_info.txt", "r");
    char apnName[32];
    char *cmd = NULL;
    int err = 0;
    ATResponse *p_response = NULL;
    RILChannelCtx* pChannel = getChannelCtxbyProxy();
    int failCause = PDP_FAIL_NONE;

    if (fPtr == NULL) {
        LOGE("syncDisabledApnToMd(), open file failed");
    } else {
        LOGD("syncDisabledApnToMd(), start");
        while (fgets(apnName, sizeof(apnName), fPtr) != NULL) {
            // AT+VZWAPNE=<wapn>,<apncl>,<apnni>,<apntype>,<apnb>,<apned>,<apntime>
            if (strcmp(apnName,"VZWADMIN") == 0) {
                asprintf(&cmd, "AT+VZWAPNE=0,2,\"%s\",\"IPV4V6\",\"LTE\",\"Disabled\",0",apnName);
            } else if (strcmp(apnName,"VZWIMS") == 0) {
                asprintf(&cmd, "AT+VZWAPNE=1,1,\"%s\",\"IPV4V6\",\"LTE\",\"Disabled\",0",apnName);
            } else {
                LOGD("syncDisabledApnToMd(), No need to disabled VZWADMIN / VZWIMS");
            }

            if (cmd != NULL) {
                err = at_send_command(cmd, &p_response, pChannel);
                free(cmd);

                if((err != 0) || (p_response->success == 0)) {
                    failCause = at_get_cme_error(p_response);
                    LOGE("syncDisabledApnToMd(), AT+VZWAPNE err=%d", failCause);
                }
                at_response_free(p_response);
            }
        }
        fclose(fPtr);
    }
}
// M: [VzW] Data Framework @}

//ALPS02110463: Check and reset EMDSTATUS, 2015/06/12
static void resetRemoteSimProperties() {
    const char *PROPERTY_CONFIG_EMDSTATUS_SEND = "vendor.ril.cdma.emdstatus.send";
    property_set(PROPERTY_CONFIG_EMDSTATUS_SEND, "0");
    RLOGD("%s: Reset %s.", __FUNCTION__, PROPERTY_CONFIG_EMDSTATUS_SEND);
}

static void resetSystemProperties()
{
    LOGD("[C2K RIL_CALLBACK] resetSystemProperties");
    int slotid = getCdmaModemSlot();
    switch (slotid) {
        case 1:
            property_set("vendor.ril.ipo.radiooff", "0");
            LOGD("C2K set vendor.ril.ipo.radiooff to 0");
            break;
        case 2:
            property_set("vendor.ril.ipo.radiooff.2", "0");
            LOGD("C2K set vendor.ril.ipo.radiooff.2 to 0");
            break;
        default:
            LOGE("C2K resetSystemProperties, unsupport slot id %d", slotid);
            break;
    }
}

/**
 * Initialize everything that can be configured while we're still in
 * AT+CFUN=0
 */
static void initializeCallback(void *param __unused)
{
    /* M: set radio state off to make the URC from readloop can be handled by onUnsolicited */
    setRadioState (RADIO_STATE_OFF);

    resetSystemProperties();

    //setRadioState (RADIO_STATE_OFF);
    atcommand_init();


    /* assume radio is off on error */
    if (isRadioOn() > 0) {
        setRadioState (RADIO_STATE_ON);
    }
}

void waitAtInitDone() {
    if (s_at_init_finish == 0) {
        LOGD("C2K onRequest wait");
        int ret = pthread_mutex_lock(&s_at_init_Mutex);
        LOGD("C2K onRequest get lock");
        assert(ret == 0);
        if (s_at_init_finish == 0) {
            LOGD("C2K onRequest s_at_init_finish still 0 and wait for ready");
            pthread_cond_wait(&s_at_init_Cond, &s_at_init_Mutex);
            LOGD("C2K onRequest finish wait");
        }
        ret = pthread_mutex_unlock(&s_at_init_Mutex);
        assert(ret == 0);
        LOGD("C2K onRequest finish unlock and ready to send");
    }
}

void notifyAtInitDone() {
    if (s_at_init_finish == 0) {
        // Only main loop modify s_at_init_finish, do not need to check its value again
        LOGD("at channel initialization done and try get lock");
        int ret = pthread_mutex_lock(&s_at_init_Mutex);
        assert(ret == 0);
        LOGD("at channel initialization lock and get lock");
        s_at_init_finish = 1;
        pthread_cond_broadcast(&s_at_init_Cond);
        LOGD("at channel initialization broadcast done");
        ret = pthread_mutex_unlock(&s_at_init_Mutex);
        assert(ret == 0);
        LOGD("at channel initialization lock and unlock");
    }
}

static void setTimespecRelative(struct timespec *p_ts, long long msec)
{
    /// M: Revise gettime to fix system time jump cause timer incorrect issue in android N. @{
    clock_gettime(CLOCK_MONOTONIC, p_ts);
    p_ts->tv_sec += (msec / 1000);
    p_ts->tv_nsec += (msec % 1000) * 1000L * 1000L;
    /// @
}

void waitMd3InitDone() {
#ifdef C2K_RIL_LEGACY
    return;
#else
    int ret = pthread_mutex_lock(&s_md3_init_Mutex);
    assert(ret == 0);
    LOGD("waitMd3InitDone start");
    if (s_md3_init_finish == 0) {
        struct timespec ts;
        setTimespecRelative(&ts, VPUP_TIMEOUT_MSEC);
        // Wait +VPUP 15s and trigger assert if timeout
        int err = pthread_cond_timedwait(&s_md3_init_Cond, &s_md3_init_Mutex, &ts);
        if (err == ETIMEDOUT) {
            char modemException[PROPERTY_VALUE_MAX] = { 0 };
            property_get(PROPERTY_MODEM_EE, modemException, "");
            if (isInternalLoad() || !isUserLoad()) {
                if (strcmp(modemException, "exception") != 0) {
                    LOG_ALWAYS_FATAL("waitMd3InitDone Wait +VPUP timeout");
                } else {
                    LOGE("waitMd3InitDone Wait +VPUP timeout with modem EE");
                }
            } else {
                if (!isModemPoweredOff()) {
                    LOGE("waitMd3InitDone Wait +VPUP timeout, reset modem");
                    triggerIoctl(CCCI_IOC_MD_RESET);
                } else {
                    // If modem power on, rild process will restart
                    LOGE("waitMd3InitDone Modem already powered off, ignore");
                }
            }
        } else {
            LOGD("waitMd3InitDone get signal");
        }
    }
    LOGD("waitMd3InitDone done");
    ret = pthread_mutex_unlock(&s_md3_init_Mutex);
    assert(ret == 0);
#endif
}

void notifyMd3InitDone() {
#ifdef C2K_RIL_LEGACY
    return;
#else
    int ret = pthread_mutex_lock(&s_md3_init_Mutex);
    assert(ret == 0);
    if (s_md3_init_finish == 0) {
        s_md3_init_finish = 1;
        pthread_cond_signal(&s_md3_init_Cond);
        LOGD("notifyMd3InitDone +VPUP received, notify mainloop");
    } else {
        LOGD("notifyMd3InitDone MD3 already initialized");
    }
    ret = pthread_mutex_unlock(&s_md3_init_Mutex);
    assert(ret == 0);
#endif
}

void sendPincode()
{
    char boottimes[PROPERTY_VALUE_MAX] = { 0 };

    property_get("vendor.net.cdma.boottimes", boottimes, "0");
    LOGD("%s: boottimes is %s", __FUNCTION__, boottimes);
    if (strcmp(boottimes, "0") == 0)
    {
        property_set("vendor.net.cdma.boottimes", "1");
    }
    else if (strcmp(boottimes, "1") == 0)
    {
        property_set("vendor.net.cdma.boottimes", "2");
    }
}

static void sendUnsolImsNetworkStateChanged()
{
#if 0 // to be used when unsol is changed to return data.
    int reply[2];
    reply[0] = s_ims_registered;
    reply[1] = s_ims_services;
    reply[1] = s_ims_format;
#endif
    RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED,
            NULL, 0);
}

/**
 * Called by atchannel when an unsolicited line appears
 * This is called on atchannel's reader thread. AT commands may
 * not be issued here
 */
static void onUnsolicited (const char *s, const char *sms_pdu)
{
    char *line = NULL;
    int urcToMalLength = 0;

    /* Ignore unsolicited responses until we're initialized.
     * This is OK because the RIL library will poll for initial state
     */
    if (getRadioState() == RADIO_STATE_UNAVAILABLE && !strStartsWith(s, "+VPUP")
            && !strStartsWith(s, "+VPON")  && isWorldModeSwitching(s)) {
        /// M: world mode switching, pending special URC. @{
        if (bWorldModeSwitching == 1) {
            cacheUrcIfNeeded(s, sms_pdu);
        }
        /// @}
        LOGD("onUnsolicited return for radio");
        return;
    }
    char atci_urc_enable[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.service.atci_urc.enable", atci_urc_enable, "0");
    if ((NULL != s) && (atoi(atci_urc_enable) == 1)) {
        RIL_onUnsolicitedResponse(RIL_UNSOL_ATCI_RESPONSE, s, strlen(s));
    }
    if (s != NULL && isMalSupported() && !strStartsWith(s, "+VPUP")) {
        if (strStartsWith(s, "+CMGR:")){
            // 5 for 2 \r\n and 1 \0
            urcToMalLength = strlen(s) + strlen(sms_pdu) + 9;
            line = (char *) calloc(1, urcToMalLength);
            assert(line);
            strlcpy(line, "\r\n", urcToMalLength);
            strlcat(line, s, urcToMalLength);
            strlcat(line, "\r\n", urcToMalLength);
            strlcat(line, "\r\n", urcToMalLength);
            strlcat(line, sms_pdu, urcToMalLength);
            strlcat(line, "\r\n\0", urcToMalLength);
        } else {
            urcToMalLength = strlen(s) + 5;
            line = (char *) calloc(1, urcToMalLength);
            assert(line);
            strlcpy(line, "\r\n", urcToMalLength);
            strlcat(line, s, urcToMalLength);
            strlcat(line, "\r\n\0", urcToMalLength);
        }

        // VzW IMS Data
        if (!skipIratUrcToMal(s)) {
            if (mal_mdmngr_send_urc != NULL
                    && mal_set != NULL
                    && mal_reset != NULL) {
                int ret = mal_mdmngr_send_urc(mal_once(2, mal_cfg_type_md_id, MAL_MD_ID_C2K,
                        mal_cfg_type_sim_id, MAL_SIM_ID_NONE), line);
                RLOGD("[%s] Call mal_mdmngr_send_urc success and ret = %d", __FUNCTION__, ret);
            } else {
                RLOGE("[%s] mal_mdmngr_send_urc is null", __FUNCTION__);
            }
        }

        free(line);
    }

    if (!(rilNwUnsolicited(s, sms_pdu)
            || rilSmsUnsolicited(s, sms_pdu)
            || rilPhbUnsolicited(s, sms_pdu)
            || rilDataUnsolicited(s, sms_pdu)
            || rilCcUnsolicited(s, sms_pdu)
            || rilSimUnsolicited(s, sms_pdu)
            || rilUtkUnsolicited(s, sms_pdu)
            || rilOemUnsolicited(s, sms_pdu)
            || rilRadioUnsolicited(s, sms_pdu)))
    {
        LOGE("Unhandled unsolicited result code: %s\n", s);
    }
}

/* Called on command or reader thread */
static void onATReaderClosed(RILChannelId channel_id)
{
    setIccidProperty(CLEAN_ICCID, NULL);
    LOGE("AT channel closed\n");
    at_close(channel_id);
    s_closed = 1;

    setRadioState (RADIO_STATE_UNAVAILABLE);
}

/* Called on command thread */
static void onATTimeout(RILChannelId channelId)
{

    LOGE("AT channel timeout; closing\n");
    at_close(channelId);

    s_closed = 1;
    if (!CmpBypassMode())
    {
        LOGE("reset CP, not in cp bypass mode");
        reset_cbp(AT_CHANNEL_TIMEOUT);
    }
    else
    {
        LOGD("timed out but not caused by bypass");
    }

    /* FIXME cause a radio reset here */

    setRadioState (RADIO_STATE_UNAVAILABLE);
}

/* Called to pass hardware configuration information to telephony
 * framework.
 */
static void setHardwareConfiguration(int num, RIL_HardwareConfig *cfg)
{
   RIL_onUnsolicitedResponse(RIL_UNSOL_HARDWARE_CONFIG_CHANGED, cfg, num*sizeof(*cfg));
}

static void usage(char *s __unused)
{
#ifdef RIL_SHLIB
    fprintf(stderr, "reference-ril requires: -p <tcp port> or -d /dev/tty_device\n");
#else /* RIL_SHLIB */
    fprintf(stderr, "usage: %s [-p <tcp port>] [-d /dev/tty_device]\n", s);
    exit(-1);
#endif /* RIL_SHLIB */
}

#ifdef C2K_RIL_LEGACY
static int ril_com_callback(char *src_module,char *dst_module,char *data_type,int *data_len,unsigned char *buffer)
{
    int i = 0;
    char type=*data_type;
    unsigned char event=buffer[0];

    LOGD("msg from %s:%s\n", name_inquery(*src_module),type_inquery(event));
    if((type == STATUS_DATATYPE_CMD) && (event==CMD_KILL_CLIENT))
    {
        LOGD("set ril to exit");
        onATReaderClosed(DEFAULT_CHANNEL);
    }
    return 0;
}
#endif

void initATChannelPathes()
{
    initChannelConfiguration(RIL_SUPPORT_CHANNELS);
    int i = 0;
    for (i = 0; i < RIL_SUPPORT_CHANNELS; i++)
    {
        getPseudoChannelPathes(i, s_mux_path[i],
                sizeof(s_mux_path[i])/sizeof(s_mux_path[i][0]));
    }
}

extern void resetWakelock(void);

static void *
mainLoop(void *param __unused)
{
    int fd;
    int ret;
    int i = 0;

    s_isUserLoad = isUserLoad();
    AT_DUMP("== ", "entering mainLoop()", -1);
#ifdef C2K_RIL_LEGACY
    /**
     * Tell statusd ril is on the way
     */
    ret = statusd_c2ssend_cmd(MODULE_TYPE_RIL,MODULE_TYPE_SR,CMD_CLIENT_INIT);
    if(ret < 1)
    {
        LOGD("CLIENT %s::send CMD_CLIENT_INIT cmd failed ret=%d\n",LOG_TAG,ret);
    }
#endif
    sendPincode();

    /* ALPS02110463: Check and reset EMDSTATUS, 2015/06/12 { */
    if (isCdmaLteDcSupport()) {
        resetRemoteSimProperties();
    } else {
        LOGD("Non SVLTE, by pass reset REMOTE SIM properties.");
    }
    /* ALPS02110463: Check and reset EMDSTATUS, 2015/06/12 } */

    AT_DUMP("== ", "entering mainLoop()", -1 );
    at_set_on_reader_closed(onATReaderClosed);
    at_set_on_timeout(onATTimeout);
    resetWakelock();

    /// M: Save main loop thread id. Used to lock and wait until init done before sending AT to MD
    ret = pthread_mutex_lock(&s_at_init_Mutex);
    LOGD("C2K mainloop get lock");
    assert(ret == 0);
    mainloop_thread_id = pthread_self();
    ret = pthread_mutex_unlock(&s_at_init_Mutex);
    assert(ret == 0);
    LOGD("C2K mainloop update thread_id and unlock");

    // If condition for google default workflow
    if (s_port > 0 || s_device_socket) {
        for (;;) {
            fd = -1;
            while  (fd < 0) {
                if (isEmulatorRunning()) {
                    fd = qemu_pipe_open("pipe:qemud:gsm");
                } else if (s_port > 0) {
                    fd = socket_network_client("localhost", s_port, SOCK_STREAM);
                } else if (s_device_socket) {
                    fd = socket_local_client(s_device_path,
                                             ANDROID_SOCKET_NAMESPACE_FILESYSTEM,
                                             SOCK_STREAM);
                } else if (s_device_path != NULL) {
                    fd = open (s_device_path, O_RDWR);
                    if ( fd >= 0 && !memcmp( s_device_path, "/dev/ttyS", 9 ) ) {
                        /* disable echo on serial ports */
                        struct termios  ios;
                        tcgetattr( fd, &ios );
                        ios.c_lflag = 0;  /* disable ECHO, ICANON, etc... */
                        tcsetattr( fd, TCSANOW, &ios );
                    }
                }

                if (fd < 0) {
                    perror ("opening AT interface. retrying...");
                    sleep(10);
                    /* never returns */
                }
            }

            RLOGD("FD: %d", fd);

            s_closed = 0;
            ret = at_open_old(fd, onUnsolicited);

            if (ret < 0) {
                RLOGE ("AT error %d on at_open\n", ret);
                return 0;
            }

            RIL_requestTimedCallback(initializeCallback, NULL, &TIMEVAL_0);

            // Give initializeCallback a chance to dispatched, since
            // we don't presently have a cancellation mechanism
            sleep(1);

            waitForClose();
            RLOGI("Re-opening after close");
        }
    } else {
        initRILChannels();
        initATChannelPathes();
        openRILChannels(s_mux_path, onUnsolicited);
        s_closed = 0;
        // AT channels initialization done, notify other thread to begin send request
        notifyAtInitDone();

        RIL_requestProxyTimedCallback(initializeCallback, NULL, &TIMEVAL_0, DEFAULT_CHANNEL);

#ifdef C2K_RIL_LEGACY
        /*tell statusd ril is ready*/
        ret = statusd_c2ssend_cmd(MODULE_TYPE_RIL,MODULE_TYPE_SR,CMD_CLIENT_READY);
        if(ret < 1) {
            LOGD("CLIENT %s::send CMD_CLIENT_READY cmd failed ret=%d\n",LOG_TAG,ret);
        } else {
            LOGD("CLIENT %s::send CMD_CLIENT_READY\n",LOG_TAG,ret);
        }
#endif
        // Give initializeCallback a chance to dispatched, since
        // we don't presently have a cancellation mechanism
        sleep(1);

        waitForClose();
#if defined(FLASHLESS_SUPPORT) && defined(AT_AUTO_SCRIPT_RESET)
        LOGD("wait to die"); //wait to killed by flashlessd
        while(noLoop);
#endif

#ifdef C2K_RIL_LEGACY
        /*wait to receive kill msg from and be killed by statusd*/
        //sleep(2);
        /*deregister interface which is used for communicating with statusd*/

        statusd_deregister_cominf(MODULE_TYPE_RIL);
#endif
    }

    return NULL;
}

#ifdef RIL_SHLIB

pthread_t s_tid_mainloop;

void RIL_setRilEnvForGT(const struct RIL_Env *env) {
    s_rilenv = env;
}


const RIL_RadioFunctions *RIL_Init(const struct RIL_Env *env, int argc, char **argv)
{
    int ret;
    int opt;
    pthread_attr_t attr;

    s_rilenv = env;

    while ( -1 != (opt = getopt(argc, argv, "p:d:s:c:"))) {
        switch (opt) {
            case 'p':
                s_port = atoi(optarg);
                if (s_port == 0) {
                    usage(argv[0]);
                    return NULL;
                }
                LOGI("Opening loopback port %d\n", s_port);
            break;

            case 'd':
                s_device_path = optarg;
                LOGI("Opening tty device %s\n", s_device_path);
            break;

            case 's':
                s_device_path = optarg;
                s_device_socket = 1;
                LOGI("Opening socket %s\n", s_device_path);
            break;

            case 'c':
                LOGI("c2k skip the argument");
            break;

            default:
                usage(argv[0]);
                LOGI("RIL_Init return null");
                return NULL;
        }
    }

    if (s_port < 0 && !strcmp(s_device_path, ""))
    {
        usage(argv[0]);
        return NULL;
    }

#ifdef C2K_RIL_LEGACY
    /*register interface which is used for communicating with statusd*/
    ret = statusd_register_cominf(MODULE_TYPE_RIL,(statusd_data_callback)ril_com_callback);
    if(ret)
    {
        LOGD("CLIENT %s :: register com interface failed\n",LOG_TAG);
    }
#endif

#ifdef ANDROID_KK
    sMdmInfo = calloc(1, sizeof(ModemInfo));
    if (!sMdmInfo)
    {
        LOGE("Unable to alloc memory for ModemInfo");
        return NULL;
    }
#endif /* ANDROID_KK */

    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&s_tid_mainloop, &attr, mainLoop, NULL);
    initMalApi();
    if (mal_datamngr_get_api_tlv_buff_len != NULL) {
        mal_datamngr_get_api_tlv_buff_len(mal_once(0), sizeof(requestedTlvMemoryOfMalApi), requestedTlvMemoryOfMalApi);
        LOGD("[%s] Call mal_datamngr_get_api_tlv_buff_len success", __FUNCTION__);
        for (int i = 0; i < MALRIL_API_INDEX_MAX; ++i) {
            LOGD("requestedTlvMemoryOfMalApi[%d] is %d", i, requestedTlvMemoryOfMalApi[i]);
        }
    } else {
        LOGE("[%s] mal_datamngr_get_api_tlv_buff_len is null", __FUNCTION__);
    }

    return &s_callbacks;
}

const RIL_RadioFunctions *RIL_SAP_Init(const struct RIL_Env *env, int argc,
        char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    s_rilsapenv = env;
    return &s_sapcallbacks;
}

#else /* RIL_SHLIB */
int main (int argc, char **argv)
{
    int ret;
    int fd = -1;
    int opt;

    while ( -1 != (opt = getopt(argc, argv, "p:d:"))) {
        switch (opt) {
            case 'p':
                s_port = atoi(optarg);
                if (s_port == 0) {
                    usage(argv[0]);
                }
                LOGI("Opening loopback port %d\n", s_port);
            break;

            case 'd':
                s_device_path = optarg;
                LOGI("Opening tty device %s\n", s_device_path);
            break;

            case 's':
                s_device_path   = optarg;
                s_device_socket = 1;
                LOGI("Opening socket %s\n", s_device_path);
            break;

            default:
                usage(argv[0]);
            break;
        }
    }

    if (s_port < 0 && s_device_path == NULL) {
        usage(argv[0]);
    }

    RIL_register(&s_callbacks);

#ifdef ANDROID_KK
    sMdmInfo = calloc(1, sizeof(ModemInfo));
    if (!sMdmInfo) {
        LOGE("Unable to alloc memory for ModemInfo");
        return NULL;
    }
#endif /* ANDROID_KK */

    mainLoop(NULL);

    return 0;
}

#endif /* RIL_SHLIB */

int waitUimModualInitialized(void)
{
    int ret = 0;
    int i = 0;
    int waitInterval = 200; //ms
    int maxWaitCount = 50; //waitInterval*maxWaitCount = 10s;

    for (i = 0; i < maxWaitCount; i++) {
        if (uim_modual_initialized) {
            break;
        }
        usleep(waitInterval * 1000);
    }
    LOGD("%s: uim_modual_initialized = %d, wait %d ms",
            __FUNCTION__, uim_modual_initialized,
            i * waitInterval);
    return ret;
}

static void autoEnterPinCode(){
}

void setIccidProperty(VIA_ICCID_TYPE type, char* pIccid)
{
    char* iccidStr = NULL;
    char maskIccid[PROPERTY_VALUE_MAX] = {0};

    switch (type) {
        case SET_VALID_ICCID:
            asprintf(&iccidStr, "%s", pIccid);
            break;
        case SET_TO_NA:
            asprintf(&iccidStr, "%s", "N/A");
            break;
        case CLEAN_ICCID:
            asprintf(&iccidStr, "%s", "");
            break;
        default:
            LOGD("%s: unsupport type is %d", __FUNCTION__, type);
            break;
    }

    // Convert all the lower case characters in iccid to upper case
    if (NULL != iccidStr) {
        for (char *p = iccidStr; *p != '\0'; p++) {
            *p = toupper(*p);
        }
    }

    givePrintableIccid(iccidStr, maskIccid);
    LOGD("%s: iccidStr is %s", __FUNCTION__, maskIccid);
    if (getActiveSvlteModeSlotId() == 1) {
        property_set("vendor.ril.iccid.sim1", iccidStr);
        LOGD("set true iccid %s to vendor.ril.iccid.sim1", maskIccid);
    } else if (getActiveSvlteModeSlotId() == 2) {
        property_set("vendor.ril.iccid.sim2", iccidStr);
        LOGD("set true iccid %s to vendor.ril.iccid.sim2", maskIccid);
    } else {
        LOGD("can not get right value from getActiveSvlteModeSlotId");
    }

    if (NULL != iccidStr) {
        free(iccidStr);
        iccidStr = NULL;
    }
}

RIL_RadioState getRadioState(void)
{
    return sState;
}

void setRadioState(RIL_RadioState newState)
{
    RIL_RadioState oldState;
    LOGD("%s: newState = %d, sState = %d, s_closed = %d",
            __FUNCTION__, newState, sState, s_closed);
    pthread_mutex_lock(&s_state_mutex);

    oldState = sState;

    if (s_closed > 0)
    {
        // If we're closed, the only reasonable state is
        // RADIO_STATE_UNAVAILABLE
        // This is here because things on the main thread
        // may attempt to change the radio state after the closed
        // event happened in another thread
        newState = RADIO_STATE_UNAVAILABLE;
    }

    if (sState != newState || s_closed > 0)
    {
        sState = newState;
        /* when radio changed to on and RUIM_READY state, read subscription automaticlly */
        if (sState == RADIO_STATE_ON && RUIM_READY == getRUIMState())
        {
#if 0
            if(!s_triggerNitztime)        //If nitztime callback is inprogress, don't trigger again
                trigger_nitztime_report();
#endif
           RIL_requestProxyTimedCallback(trigger_update_cache, NULL,
                   &TIMEVAL_UPDATE_CACHE, DEFAULT_CHANNEL);
        }
        pthread_cond_broadcast (&s_state_cond);
    }

    pthread_mutex_unlock(&s_state_mutex);

    /* do these outside of the mutex */
    if (sState != oldState)
    {
        LOGD("setRadioState: before RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED,sState = %d",sState);
        RIL_onUnsolicitedResponse (RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED,
                NULL, 0);

        /* FIXME onRadioPowerOn() cannot be called
         * from the AT reader thread
         * Currently, this doesn't happen, but if that changes then these
         * will need to be dispatched on the request thread
         */
        if (sState == RADIO_STATE_ON && RUIM_NOT_READY == getRUIMState())
        {
            onRadioPowerOn();
        }
        else if (sState == RADIO_STATE_UNAVAILABLE)
        {
            resetUtkStatus();
        }
    }
}

void waitForClose()
{
    pthread_mutex_lock(&s_state_mutex);

    while (s_closed == 0) {
        pthread_cond_wait(&s_state_cond, &s_state_mutex);
    }

    pthread_mutex_unlock(&s_state_mutex);
}

#ifdef C2K_RIL_LEGACY
void reset_cbp(CBP_RESET_CASE type)
{
    int fd = -1;
    int i = 0;
    char *errstr = NULL;
#ifdef USE_STATUSD
    int ret = -1;
    if( type == AT_CHANNEL_TIMEOUT)
    {
        ret = statusd_c2ssend_cmd(MODULE_TYPE_RIL,MODULE_TYPE_SR,CMD_CLIENT_ERROR);
        LOGD("CLIENT %s::send CMD_CLIENT_ERROR cmd ret=%d\n",LOG_TAG,ret);
        if(ret != 1)
        {
            LOGD("CLIENT %s::send CMD_CLIENT_ERROR cmd failed ret=%d\n",LOG_TAG,ret);
        }
    }
#elif defined(FLASHLESS_SUPPORT)
    LOGD("reset_cbp start");
    /*Let flashlessd to handle the other type of CBP_RESET_CASE*/
    if( type == AT_CHANNEL_TIMEOUT)
    {
        viatelModemReset();
    }
    LOGD("reset_cbp stop");
    return ;
#else
    fd = open("/sys/power/modem_reset", O_RDWR);
    if(fd < 0)
    {
        errstr = strerror(errno);
        LOGE("Open modem_reset error, errno=%d, %s\n", errno, errstr);
    }
    else
    {
        i = write(fd, "reset", sizeof("reset"));
        if(i < 0)
        {
            errstr = strerror(errno);
            LOGE("write modem_reset errror, errno=%d, %s\n", errno, errstr);
        }
        else
        {
            LOGD("Reset the modem...\n");
        }
    }
    if(fd >=0)
    {
        close(fd);
    }
#endif
}
#else
void reset_cbp(CBP_RESET_CASE type)
{
    UNUSED(type);
    triggerIoctl(CCCI_IOC_MD_RESET);
}
#endif

#ifdef USB_FS_EXCEPTION
void powerCbp(int type)
{
    UNUSED(type);
#ifndef FLASHLESS_SUPPORT
    int fd = -1;
    int len = 0;
    char buf[8];
    char *errstr = NULL;

    memset(buf , 0, sizeof(buf));
    if(type)
    {
        strcpy(buf, "on");
    }
    else
    {
        strcpy(buf, "off");
    }

    do
    {
        fd = open("/sys/power/modem_power", O_RDWR);
        if( fd < 0)
        {
            errstr = strerror(errno);
            LOGE("Open modem_power error, errno=%d, %s\n", errno, errstr);
            break;
        }

        len = strlen(buf);
        if(len == write(fd, buf, len))
        {
            LOGD("Power %s the modem...\n", buf);
        }
        else
        {
            errstr = strerror(errno);
            LOGE("write modem_reset errror, errno=%d, %s\n", errno, errstr);
            break;
        }
    } while(0);

    if(fd >= 0)
    {
        close(fd);
        fd = -1;
    }
#endif
    return;
}

int CmpBypassMode(void)
{
    int bypassed = 0;
    char tempstr = 0;
    int ret;


    int sys_fd_at = open(SYS_AT_CHANNEL, O_RDONLY);
    if (sys_fd_at == -1)
    {
        LOGE("fail to open SYS_AT_CHANNEL!");
        return -1;
    }

    ret = read(sys_fd_at, &tempstr, 1);
    if (ret != 1)
    {
        LOGE("fail to read SYS_AT_CHANNEL!");
        goto exit_at;
    }

    bypassed = atoi(&tempstr);
    if (bypassed)
    {
        LOGD("CmpBypassMode at channel is bypassed");
        goto exit_at;
    }
#if 0 //RIL Recovery mechanism should work even  either  DATA or  ETS  is bypassed.

    int sys_fd_data = open(SYS_DATA_CHANNEL, O_RDONLY);
    if (sys_fd_data == -1)
    {
        LOGE("fail to open SYS_DATA_CHANNEL!");
        goto exit_at;
    }

    ret = read(sys_fd_data, &tempstr, 1);
    if (ret != 1)
    {
        LOGE("fail to read SYS_DATA_CHANNEL!");
        goto exit_data;
    }

    bypassed = atoi(&tempstr);
    if (bypassed)
    {
        LOGD("CmpBypassMode data port is bypassed");
        goto exit_data;
    }

    int sys_fd_ets = open(SYS_ETS_CHANNEL, O_RDONLY);
    if (sys_fd_ets == -1)
    {
        LOGE("fail to open SYS_ETS_CHANNEL!");
        goto exit_data;
    }

    ret = read(sys_fd_ets, &tempstr, 1);
    if (ret != 1)
    {
        LOGE("fail to read SYS_ETS_CHANNEL!");
        goto exit_ets;
    }

    bypassed = atoi(&tempstr);
    if (bypassed)
        LOGD("CmpBypassMode ets port is bypassed");

exit_ets:
    close(sys_fd_ets);

exit_data:
    close(sys_fd_data);
#endif

exit_at:
    close(sys_fd_at);

    return bypassed;
}
#endif /* USB_FS_EXCEPTION */

/** do post-AT+CFUN=1 initialization */
static void onRadioPowerOn()
{
    LOGD("onRadioPowerOn");

    pollRUIMState(NULL);
}

/**
 * RUIM ready means any commands that access the SIM will work, including:
 *  AT+CPIN, AT+CSMS?, AT+CNMI, AT+CRSM
 *  (all SMS-related commands)
 */
void pollRUIMState (void *param)
{
    UNUSED(param);
    LOGD("pollUIMState");
    // if (getRadioState() != RADIO_STATE_RUIM_NOT_READY ) {
        // no longer valid to poll
    //     return;
    // }

    switch(getRUIMStatus(UPDATE_RADIO_STATUS)) {
        case RUIM_NOT_READY:
        RIL_requestProxyTimedCallback(pollRUIMState, NULL, &TIMEVAL_SIMPOLL,
                DEFAULT_CHANNEL);
            break;
        case RUIM_READY:
            // setRadioState(RADIO_STATE_RUIM_READY);
            RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);
            break;
        case RUIM_ABSENT:
        case RUIM_PIN:
        case RUIM_PUK:
        case RUIM_NETWORK_PERSONALIZATION:
        default:
            // setRadioState(RADIO_STATE_RUIM_LOCKED_OR_ABSENT);
            RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);
            break;
    }
}

/// M: SAP start
void sendSapResponseComplete(RIL_Token t, RIL_Errno ret, MsgId msgId, void *data) {
    const pb_field_t *fields = NULL;
    size_t encoded_size = 0;
    uint32_t written_size = 0;
    size_t buffer_size = 0;
    pb_ostream_t ostream;
    bool success = false;

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
            RLOGD("sendSapResponseComplete, Size: %lu Size as written: 0x%x",
                    encoded_size, written_size);
            // Send response
            RIL_SAP_onRequestComplete(t, ret, buffer, buffer_size);
        } else {
            RLOGE("sendSapResponseComplete, Encode failed!");
        }
    } else {
        RLOGE("Not sending response type %d: encoded_size: %lu. encoded size result: %d",
                msgId, encoded_size, success);
    }
}
/// SAP end

int isImsSupport() {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.ims_support", property_value, "0");
    return atoi(property_value);
}

void cacheUrcIfNeeded(const char *s, const char *sms_pdu) {
    // Only the URC list we wanted.
    if (strStartsWith(s, "+REGPAUSEIND:")) {
        const int socket_id = DEFAULT_CHANNEL;
        RLOGD("cacheUrc: RILD:%d, %s", socket_id, s);

        UrcList* urcCur = NULL;
        UrcList* urcPrev = NULL;
        int pendedUrcCount = 0;

        urcCur = pendedUrcList;
        while (urcCur != NULL) {
            RLOGD("cacheUrc: Pended URC:%d, RILD:%d, %s", pendedUrcCount, urcCur->rid, urcCur->urc);
            urcPrev = urcCur;
            urcCur = (UrcList*)(urcCur->pNext);
            pendedUrcCount++;
        }
        urcCur = (UrcList*)calloc(1, sizeof(UrcList));
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

    const int rid = (int) (urclist->rid);
    RLOGD("sendUrc: RILD:%d %s, %s", rid, urclist->urc, urclist->sms_pdu);

    onUnsolicited(urclist->urc, urclist->sms_pdu);

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
            RLOGD("sendUrcs: RILD:%d, %s", urc->rid, urc->urc);
            RIL_requestProxyTimedCallback(sendUrc, urc, &TIMEVAL_0, DEFAULT_CHANNEL);
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
/// @}


void *getDlopenHandler(const char *libPath) {
    void *dlHandler;
    dlHandler = dlopen(libPath, RTLD_NOW);
    if (dlHandler == NULL) {
        LOGE("dlopen failed: %s", dlerror());
    }
    return dlHandler;
}


void *getAPIbyDlopenHandler(void *dlHandler, const char *apiName){
    if (dlHandler == NULL) {
        LOGE("dlopen Handler is null");
        return NULL;
    }
    void *outputFun;
    outputFun = dlsym(dlHandler, apiName);
    if (outputFun == NULL) {
        LOGE("%s is not defined or exported: %s", apiName, dlerror());
    } else {
        LOGD("Dlopen api success: %s", apiName);
    }
    return outputFun;
}


void initMalApi() {
    LOGD("[%s] Dlopen mal APIs start",__FUNCTION__);
    dlopenHandlerForMalApi = getDlopenHandler("libmal.so");
    if (dlopenHandlerForMalApi != NULL) {
        mal_datamngr_get_ims_info = (int (*)(mal_ptr_t , mal_datamngr_ims_info_req_ptr_t , mal_datamngr_ims_info_rsp_ptr_t))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_datamngr_get_ims_info");
        mal_datamngr_set_data_call_info = (int (*)(mal_ptr_t, mal_datamngr_data_call_info_req_ptr_t, mal_datamngr_data_call_info_rsp_ptr_t))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_datamngr_set_data_call_info");
        mal_datamngr_set_pdn_state = (int (*)(mal_ptr_t, mal_datamngr_set_pdn_state_req_ptr_t, mal_datamngr_set_pdn_state_rsp_ptr_t))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_datamngr_set_pdn_state");
        mal_mdmngr_send_urc = (int (*)(mal_ptr_t, const char *))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_mdmngr_send_urc");
        mal_set = (mal_ptr_t (*)(mal_ptr_t, unsigned int, ...))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_set");
        mal_reset = (mal_ptr_t (*)(mal_ptr_t))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_reset");
        mal_datamngr_notify_data_call_list_tlv = (int (*)(mal_ptr_t mal_ptr, int num_pdn, mal_datamngr_data_call_info_req_ptr_t req_ptr[], mal_datamngr_data_call_info_rsp_ptr_t rsp_ptr[],
                int num_req_tlv[], void *req_tlv[], int *num_rsp_tlv[], void *rsp_tlv[]))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_datamngr_notify_data_call_list_tlv");
        mal_datamngr_set_pdn_state_tlv = (int (*)(mal_ptr_t mal_ptr, mal_datamngr_set_pdn_state_req_ptr_t req_ptr, mal_datamngr_set_pdn_state_rsp_ptr_t rsp_ptr,
                int num_req_tlv, void *req_tlv, int *num_rsp_tlv, void *rsp_tlv))getAPIbyDlopenHandler(dlopenHandlerForMalApi, "mal_datamngr_set_pdn_state_tlv");
        LOGD("[%s] Dlopen mal APIs success",__FUNCTION__);
    } else {
        LOGE("[%s] Dlopen mal APIs failed: dlopenHandlerForMalApi is null",__FUNCTION__);
    }

}

