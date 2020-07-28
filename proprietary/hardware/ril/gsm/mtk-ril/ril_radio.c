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

#include <telephony/mtk_ril.h>
#include "ril_radio.h"
#include "atchannels.h"
#include "hardware/ccci_intf.h"
#include <cutils/properties.h>
#include <ril_callbacks.h>
#include <libmtkrilutils.h>
#include "ril_nw.h"
#include "ril_oem.h"
#include "ratconfig.h"

#undef LOG_TAG
#define LOG_TAG "RilRadio"

extern int s_enhanceModemOffVersion;
extern RIL_RadioState getRadioState(RIL_SOCKET_ID rid);
extern void setRadioState(RIL_RadioState newState, RIL_SOCKET_ID rid);
extern void sendATWithRetry(int times, int interval, const char* at_cmd, RILChannelCtx* p_channel);
extern int ril_nw_cur_state[];

int s_md_off = 0;
int s_md_on = 0;
int s_main_loop = 1;

int isModemResetStarted = 0;

/* radio power to calcalate value of EFUN */
int m_msim_mode[RIL_SOCKET_NUM] = {0};

//Add for radio request in different channel.
static pthread_mutex_t s_md1_radio_mutex = PTHREAD_MUTEX_INITIALIZER;

// md status reader
pthread_t mdStatsuReaderThread = 0;

// modem status reader
static void *mdStatusReaderLoop(void *param);

/*
  * struct md_status_event {
  *     struct timeval time_stamp;
  *     int md_id;
  *     int event_type;
  *     char reason[32];
  * };
  * define at ccci_inft.h
  */
typedef struct md_status_event MdStatusEvent;

extern int rilRadioMain(int request, void * data, size_t datalen, RIL_Token t) {
    switch(request) {
        case RIL_REQUEST_MODEM_POWERON:
            requestModemPowerOn(data, datalen, t);
            break;
        case RIL_REQUEST_MODEM_POWEROFF:
            requestModemPowerOff(data, datalen, t);
            break;
        case RIL_REQUEST_CONFIG_MODEM_STATUS:
            requestConfigModemStatus(data, datalen, t);
            break;
        case RIL_REQUEST_ENTER_RESTRICT_MODEM:
        case RIL_REQUEST_LEAVE_RESTRICT_MODEM:
            requestSetRestrictModemState(data, datalen, t);
            break;
        case RIL_LOCAL_REQUEST_MODE_SWITCH_GSM_SET_TRM:
            requestModeSwitchSetTrm(data, datalen, t);
            break;
        case RIL_REQUEST_RADIO_POWER:
        case RIL_REQUEST_ENABLE_MODEM:
            requestRadioPower(data, datalen, t);
            break;
        case RIL_REQUEST_SHUTDOWN:
            requestModemPowerOff(data, datalen, t);
            break;
        case RIL_REQUEST_RESET_RADIO:
        case RIL_REQUEST_RESTART_RILD:
            requestResetRadio(data, datalen, t);
            break;
        default:
            // not support
            return 0;
    }
    return 1;
}

void requestModemPowerOn(void * data __unused, size_t datalen __unused, RIL_Token t) {
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    int err=0;
    ATResponse *p_response = NULL;
    RIL_RadioState state = getRadioState(rid);
    RIL_RadioState new_state;
    RIL_Errno err_no = RIL_E_SUCCESS;

    LOGD("ENTER requestRadio PowerOn");
    property_set("vendor.ril.getccci.response","0");

    if (s_md_on) {
        LOGD("requestRadioPowerOn but already modem powered on");
        RIL_onRequestComplete(t, err_no, NULL, 0);
        return;
    }
    s_md_on = 1;

    // Reset property when we need to reset modem(ex:EPON).
    if (!isCCCIPowerOffModem() || s_md_off){
        int i = 0;
        for (i=0; i < getSimCount(); i++)
        {
            /*reset all sim properties*/
            LOGD("resetSIMProperties, rid = %d", i);
            resetSIMProperties((RIL_SOCKET_ID) (RIL_SOCKET_1+i));
        }
    }

    //Deu to the connection might be disconnected soon after AT+EPON, we set property first.
    LOGD("SET vendor.ril.ipo.radiooff to 0");

    // isDualTalkMode return 0 in c2k project after revert isEVDODTSupport.
    if (!isDualTalkMode() && !RatConfig_isC2kSupported()) {
        property_set("vendor.ril.ipo.radiooff","0");
    }else{
        for (int i = 0; i < SIM_COUNT; i++) {
            upadteSystemPropertyByCurrentMode(i,"vendor.ril.ipo.radiooff","vendor.ril.ipo.radiooff.2","0");
        }
    }
    if (isCCCIPowerOffModem()) {
        if (!s_md_off) {
            LOGD("Flight mode power on modem but modem is already power on");
        } else {
            if (s_enhanceModemOffVersion == 1) {
                LOGD("Flight mode power on modem, trigger CCCI power on modem (new versio)");
                triggerCCCIIoctl(CCCI_IOC_LEAVE_DEEP_FLIGHT_ENHANCED);
            } else {
                LOGD("Flight mode power on modem, trigger CCCI power on modem");
                triggerCCCIIoctl(CCCI_IOC_LEAVE_DEEP_FLIGHT);
            }
        }
    } else {
        if (getMdMaxSimCount() >= 2) {
            if (RIL_queryMyChannelId(t) >= RIL_CHANNEL_OFFSET)
                p_channel = getRILChannelCtxFromToken(t);
        }
        err = at_send_command("AT+EPON", &p_response, p_channel);
        new_state = RADIO_STATE_UNAVAILABLE;

        setRadioState(new_state,rid);

        at_response_free(p_response);
    }
    s_main_loop = 0;
    RIL_onRequestComplete(t, err_no, NULL, 0);
    property_set("vendor.ril.getccci.response","1");
}

void requestModemPowerOff(void * data __unused, size_t datalen __unused, RIL_Token t) {
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);
    int err=0;
    ATResponse *p_response = NULL;
    RIL_RadioState state = getRadioState(rid);
    RIL_RadioState new_state;
    RIL_Errno err_no = RIL_E_SUCCESS;
    int i=0;

    if (getMdMaxSimCount() >= 2) {
        p_channel = getRILChannelCtxFromToken(t);
    }

    LOGD("%s(): s_md_off:%d, rid:%d.", __FUNCTION__, s_md_off, rid);

    if (s_enhanceModemOffVersion == 1 && RatConfig_isC2kSupported()) {
        char c2kPowerOffSync[PROPERTY_VALUE_MAX] = { 0 };
        int c2kPowerOffSyncVal = 0;
        property_get("vendor.ril.cdma.offsync", c2kPowerOffSync, "0");
        c2kPowerOffSyncVal = atoi(c2kPowerOffSync);
        while (c2kPowerOffSyncVal != 1) {
            usleep(100 * 1000);
            property_get("vendor.ril.cdma.offsync", c2kPowerOffSync, "0");
            c2kPowerOffSyncVal = atoi(c2kPowerOffSync);
        }
    } else {
        LOGE("no need to sync vendor.ril.cdma.offsync");
    }

    if (isCCCIPowerOffModem()) {
        if (s_md_off) {
            LOGD("Flight mode power off modem but already modem powered off");
            RIL_onRequestComplete(t, err_no, NULL, 0);
            return;
        }
        // Normal AT may be blocked if s_md_off is 1
        s_md_off = 1;
    }

    LOGD("ENTER requestRadio PowerOff");
    LOGD("requestRadioPowerOff SET vendor.ril.ipo.radiooff to -1");
    // isDualTalkMode return 0 in c2k project after revert isEVDODTSupport. @{
    if (!isDualTalkMode() && !RatConfig_isC2kSupported()) {
        property_set("vendor.ril.ipo.radiooff","-1");
    }else{
        for (int i = 0; i < SIM_COUNT; i++) {
            upadteSystemPropertyByCurrentMode(i,"vendor.ril.ipo.radiooff","vendor.ril.ipo.radiooff.2","-1");
        }
    }

    // For SVLTE project optimization: MD1 EPOF execution will wait until MD3 EPOF finished,
    // send AT+EFUN=0 to turn off radio first which may save some time
    // put efun & epof after ipo flag to avoid IPOD wrongly killing PhoneApp
    if (isCdmaLteDcSupport()) {
        const int RETRY_TIMES = 20;
        const int RETRY_INTERVAL = 1 * 1000;
        LOGD("%s(): set radio off before power off MD1.", __FUNCTION__);
        sendATWithRetry(RETRY_TIMES, RETRY_INTERVAL, "AT+EFUN=0", p_channel);
    }

    //Wait here for sim switch done, or the following AT-cmd will not to be sent due to mutex lock
    pthread_mutex_lock(&p_channel->restartMutex);
    pthread_mutex_unlock(&p_channel->restartMutex);

    err = at_send_command("AT+ECUSD=2,2", &p_response, p_channel);
    at_response_free(p_response);
    p_response = NULL;

    sendATBeforeEPOF("AT+EMDT=0", "+EMDT:", 0 /* retry times */, 0 /* interval */, p_channel);

    err = at_send_command("AT+EPOF", &p_response, p_channel);
    if (isCCCIPowerOffModem()) {
        if (s_enhanceModemOffVersion == 1) {
            LOGD("Flight mode power off modem, trigger CCCI level 2 power off (new version)");
            triggerCCCIIoctl(CCCI_IOC_ENTER_DEEP_FLIGHT_ENHANCED);
        } else {
            LOGD("Flight mode power off modem, trigger CCCI level 2 power off");
            triggerCCCIIoctl(CCCI_IOC_ENTER_DEEP_FLIGHT);
        }
    }

    // MD response AT+EPOF until all radio and sim is off
    LOGD("requestRadioPowerOff SET vendor.ril.ipo.radiooff to 1");

    // isDualTalkMode return 0 in c2k project after revert isEVDODTSupport.
    if (!isDualTalkMode() && !RatConfig_isC2kSupported()) {
        property_set("vendor.ril.ipo.radiooff","1");
    }else{
        for (int i = 0; i < SIM_COUNT; i++) {
            upadteSystemPropertyByCurrentMode(i,"vendor.ril.ipo.radiooff","vendor.ril.ipo.radiooff.2","1");
        }
    }

    // enhance modem off
    if (s_enhanceModemOffVersion == 1) {
        property_set("vendor.ril.cdma.emdstatus.send", "0");
        property_set("vendor.ril.cdma.offsync", "0");
        RLOGD("requestRadioPowerOff set vendor.ril.cdma.emdstatus.send to 0");
    }

    new_state = RADIO_STATE_OFF;
    for(i=0; i<getSimCount(); i++){
        setRadioState(RADIO_STATE_OFF, i);
        resetPhbReady(RIL_SOCKET_1 + i);
    }

    at_response_free(p_response);
    p_response = NULL;

    RIL_onRequestComplete(t, err_no, NULL, 0);

    s_md_on = 0;
}

void requestConfigModemStatus(void *data, size_t datalen, RIL_Token t)
{
    int modemStatus, remoteSimProtocol;
    int err;
    ATResponse *p_response = NULL;
    char *cmd = NULL;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;
    RILChannelCtx *p_channel = NULL;

    modemStatus = ((int *)data)[0];
    remoteSimProtocol = ((int *)data)[1];

    if ((modemStatus < 0 || modemStatus > 2) || (remoteSimProtocol < 0 || remoteSimProtocol > 2)) {
        LOGE("requestConfigModemStatus para error!");
    } else {
        p_channel = getRILChannelCtxFromToken(t);
        LOGD("%s: force dispatch, p_channel:%s", __FUNCTION__, p_channel->myName);
        err = asprintf(&cmd, "AT+EMDSTATUS=%d, %d", modemStatus, remoteSimProtocol);
        if(err >= 0) {
            err = at_send_command(cmd, &p_response, p_channel);

            if ( !(err < 0 || p_response->success == 0) ) {
                ril_errno = RIL_E_SUCCESS;
                int cdmaslot = getActiveSvlteModeSlotId();
                char* lastProtocol = NULL;
                asprintf(&lastProtocol, "%d,%d", remoteSimProtocol, cdmaslot);
                property_set("vendor.ril.cdma.emdstatus.protocol", lastProtocol);
                LOGD("vendor.ril.cdma.emdstatus.protocol set to %d,%d", remoteSimProtocol, cdmaslot);
                free(lastProtocol);
            }
        }
        if (cmd != NULL) {
            free(cmd);
        }
    }
    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);
}

/**
 *  ModeController uses it to let Modem enter or exit restrict mode. In restrict mode, it's not
 *  allowed to radio on Modem.
 */
void requestSetRestrictModemState(void * data, size_t datalen, RIL_Token t) {
    int err = 0;
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    int value = ((int *)data)[0];
    LOGD("%s: value is %d", __FUNCTION__, value);
    RILChannelCtx *p_channel = getRILChannelCtxFromToken(t);
    asprintf(&cmd, "AT+ERMS=%d", value);
    LOGD("%s: force dispatch, p_channel:%s", __FUNCTION__, p_channel->myName);
    err = at_send_command(cmd, &p_response, p_channel);
    free(cmd);
    if ((err < 0) || (p_response->success == 0)) {
        LOGE("%s: send at error", __FUNCTION__);
        goto error;
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

/**
 *  ModeController uses it to reset Modem in SVLTE project when CDMA capability SIM slot changed.
 */
void requestModeSwitchSetTrm(void * data, size_t datalen, RIL_Token t) {
    const int RETRY_TIMES = 20;
    const int RETRY_INTERVAL = 1 * 1000;
    int err = 0;
    ATResponse *p_response = NULL;
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    if (getMdMaxSimCount()>= 2) {
        p_channel = getRILChannelCtxFromToken(t);
    }
    LOGD("%s(): s_md_off = %d.", __FUNCTION__, s_md_off);
    if (s_md_off != 1) {
        LOGD("%s(): set radio off before power off MD1.", __FUNCTION__);
        sendATWithRetry(RETRY_TIMES, RETRY_INTERVAL, "AT+EFUN=0", p_channel);
        err = at_send_command("AT+EMDT=0", &p_response, p_channel);
        at_response_free(p_response);
        p_response = NULL;
        //power off modem
        if (isCCCIPowerOffModem()) {
            s_md_off = 1;
        }
        err = at_send_command("AT+EPOF", &p_response, p_channel);
        if (err != 0 || p_response->success == 0) {
            LOGW("There is something wrong with the exectution of AT+EPOF");
        }
        at_response_free(p_response);
    }
    if (isCdmaLteDcSupport()) {
        property_set("vendor.ril.cdma.report.case", "1");
    }
    property_set("vendor.ril.mux.report.case", "2");
    property_set("vendor.ril.muxreport", "1");
    s_main_loop = 0;
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

// Add 2 functions for radio reqeust in different channels.
void grabRadioPowerLock(void)
{
    pthread_mutex_lock(&s_md1_radio_mutex);
}

void releaseRadioPowerLock(void)
{
    pthread_mutex_unlock(&s_md1_radio_mutex);
}

void requestRadioPower(void * data, size_t datalen __unused, RIL_Token t)
{
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelId(RIL_queryCommandChannelId(t));
    int onOff, not_sent = 0, targetMode = 0;
    int err=0, is_radio_on, i;
    ATResponse *p_response = NULL;
    RIL_RadioState state = getRadioState(rid);
    RIL_RadioState new_state;
    AT_CME_Error cause;
    RIL_Errno err_no = RIL_E_SUCCESS;
    int old_state = m_msim_mode[rid];
    int simId3G = RIL_get3GSIM()-1;

    grabRadioPowerLock();

    if (getMdMaxSimCount() >= 2) {
        // force dispatch request radio power channel to protocol 1
        p_channel = getRILChannelCtxFromToken(t);
        LOGD("force dispatch, requestRadio Power, p_channel:%s", p_channel->myName);
    } else {
        LOGD("ENTER requestRadio Power, p_channel:%s", p_channel->myName);
    }

    onOff = ((int *)data)[0];

    m_msim_mode[rid] = onOff;
    for(i=0; i<getSimCount(); i++) {
        targetMode |= m_msim_mode[i] << i;
    }
    LOGD("requestRadioPower, onOff: %d, targetMode: %d", onOff, targetMode);

    if (targetMode == 0 && !s_md_off)
    {
        if (getMdMaxSimCount() >= 2) {
            err = at_send_command("AT+EFUN=0", &p_response, p_channel);
        } else {
            err = at_send_command("AT+CFUN=4", &p_response, p_channel);
        }
        new_state = RADIO_STATE_OFF;
    }
    else if (targetMode > 0 /*&& state == RADIO_STATE_OFF*/)
    {
        if (s_md_off &&(isCCCIPowerOffModem())){
            if (RatConfig_isC2kSupported()) {
                LOGD("In Cdma project, not trigger CCCI power on modem when set radio power on");
                RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
                releaseRadioPowerLock();
                return;
            }
            LOGD("Flight mode power on modem, trigger CCCI power on modem");
            LOGD("SET vendor.ril.ipo.radiooff to 0");
            property_set("vendor.ril.ipo.radiooff","0");
            property_set("vendor.ril.getccci.response","0");
            if (s_enhanceModemOffVersion == 1) {
                LOGD("Flight mode power on modem, trigger CCCI power on modem (new versio)");
                triggerCCCIIoctl(CCCI_IOC_LEAVE_DEEP_FLIGHT_ENHANCED);
            } else {
                LOGD("Flight mode power on modem, trigger CCCI power on modem");
                triggerCCCIIoctl(CCCI_IOC_LEAVE_DEEP_FLIGHT);
            }
            RIL_onRequestComplete(t, err_no, NULL, 0);
            property_set("vendor.ril.getccci.response","1");
            releaseRadioPowerLock();
            return;
        }

        if (isMdFallbackSupport()) {
            // enable MD fallback for AT&T requirement
            err = at_send_command("AT+EGFB=1,1", &p_response, p_channel);
        } else {
            //before turn on radio, turn off fallback since CID0 does not support fallback
            //AT+EGFB=<fallback_enable>,<ipv4_first>
            err = at_send_command("AT+EGFB=0,1", &p_response, p_channel);
        }
        at_response_free(p_response);
        p_response = NULL;

        if(RIL_get3GSIM() - 1) {
            int sim1mode = targetMode & RADIO_MODE_SIM1_ONLY;
            int sim3Gmode = ((targetMode & (RADIO_MODE_SIM1_ONLY<< simId3G)) > 0) ?1 :0;
            LOGD("switched : original mode=%d, sim1mode=%d,sim3Gmode=%d , need switch", targetMode,sim1mode,sim3Gmode);

            targetMode &= ~(RADIO_MODE_SIM1_ONLY);
            targetMode &= ~(RADIO_MODE_SIM1_ONLY << simId3G);
            targetMode |= (sim1mode << simId3G);
            targetMode |= sim3Gmode;
        }

        // for data attach, EGTYPE needs applied before CFUN
        doAttachBeforeRadioOn(rid, onOff, p_channel);
        defineAttachApnBeforeRadioOn(rid, p_channel);

        if (getMdMaxSimCount() >= 2) {
            char *cmd;
            err = asprintf(&cmd, "AT+EFUN=%d", targetMode);
            err = at_send_command(cmd, &p_response, p_channel);
            free(cmd);
        } else {
            err = at_send_command("AT+CFUN=1", &p_response, p_channel);
        }
        //new_state = RADIO_STATE_SIM_NOT_READY;
        if(onOff == 0) {
            new_state = RADIO_STATE_OFF;
        } else {
            new_state = RADIO_STATE_ON;
        }
    } else {
        new_state = state;
        not_sent = 1;
    }

    // Handle Dyanmic switch radio off error 619
    // this may cause by OEM HOOK EFUN or ATCI, we will retry force EFUN=0
    if (isSrlteSupport() && !not_sent && p_response != NULL && p_response->success == 0) {
        cause = at_get_cme_error(p_response);
        if (cause ==  CME_NOT_ALLOW_RADIO_ON) {
            LOGD("Dynamic switch radio off error:619, retry EFUN=0");
            at_response_free(p_response);
            p_response = NULL;
            //operation failed, so need Reset m_msim_mode[rid];
            m_msim_mode[rid] = old_state;
            if (onOff == 0) {
                err = at_send_command("AT+EFUN=0", &p_response, p_channel);
                targetMode = 0;
                cause = 0;
                new_state = RADIO_STATE_OFF;
            }
        }
    }

    if( !not_sent && p_response != NULL && p_response->success == 0)
    {
        cause = at_get_cme_error(p_response);
        err_no = RIL_E_GENERIC_FAILURE;
        new_state = state;

        LOGD("Get error cause: %d",cause);

        if ( cause ==  CME_ERROR_NON_CME)
        {
            LOGW("There is something wrong with the returned CME ERROR, please help to check");
        }
        else if (cause == CME_OPERATION_NOT_ALLOWED_ERR)
        {
            err_no = RIL_E_OP_NOT_ALLOWED_DURING_VOICE_CALL;
        }
        else
        {
            /*********************************
            * The reason of the error:
            * 1. the radio state of the modem and rild is not sync.
            * 2. There is any action or call existed in the modem
            **********************************/

            is_radio_on = isRadioOn(rid);
            if ( is_radio_on == -1)
            {
                LOGD("AT+CFUN? can't be executed normally");
                goto error;
            }
            else if( !is_radio_on && state != RADIO_STATE_OFF)
            {
                // phone off
                LOGW("The state of the modem is not synchronized with the state in the RILD: phone off");
                new_state = RADIO_STATE_OFF;
                err_no = RIL_E_SUCCESS;
            }
            else if ( is_radio_on && state == RADIO_STATE_OFF)
            {
                // phone on
                LOGW("The state of the modem is not synchronized with the state in the RILD: phone on");
                new_state = RADIO_STATE_ON;
                err_no = RIL_E_SUCCESS;
            }
            else
            {
                // The execution of the AT+CFUN is not success.
                LOGD("AT+CFUN=<fun> can't be executed");
            }
        }
    }

    if (p_response != NULL) {
        at_response_free(p_response);
        p_response = NULL;
    }

    /// M: [C2K]Dynamic switch support. @{
    if (cause != CME_NOT_ALLOW_RADIO_ON) {
        setRadioState(new_state, rid);
        // Update the other rild radio state for C2K refactoring
        if (isSrlteSupport()) {
        for (i = 0; i < getSimCount(); i++) {
            RIL_RadioState state;
            if (i != rid) {
                if (simId3G == 0) {
                    int radio = targetMode & (RADIO_MODE_SIM1_ONLY << i);
                    if (radio > 0) {
                        LOGD("set another rild=%d radio on", i);
                        setRadioState(RADIO_STATE_ON, i);
                    } else if (radio == 0) {
                        state = getRadioState(i);
                        if (RADIO_STATE_UNAVAILABLE != state) {
                            LOGD("set another rild=%d radio off", i);
                            setRadioState(RADIO_STATE_OFF, i);
                        }
                    } else {
                        LOGD("No need to set another rild=%d state", i);
                    }
                } else {
                    int protcolId = (i == simId3G) ? 0 : simId3G;
                    int radio = targetMode & (RADIO_MODE_SIM1_ONLY << protcolId);
                    if (radio > 0) {
                        LOGD("set another rild=%d radio on", i);
                        setRadioState(RADIO_STATE_ON, i);
                    } else if (radio == 0) {
                        state = getRadioState(i);
                        if (RADIO_STATE_UNAVAILABLE != state) {
                            LOGD("set another rild=%d radio off", i);
                            setRadioState(RADIO_STATE_OFF, i);
                        }
                    } else {
                        LOGD("No need to set another rild=%d state", i);
                    }
                }
            }
        }
        }
    } else {
        LOGD("Not allow radio on");
    }
    /// @}

    RIL_onRequestComplete(t, err_no, NULL, 0);

    if (!not_sent && err_no == RIL_E_SUCCESS)
    {
        if (new_state == RADIO_STATE_OFF)
        {
            cleanCurrentRestrictionState(RIL_NW_ALL_RESTRICTIONS, rid);
            setCurrentRestrictionState(RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS, rid);
        }
        else
        {
            cleanCurrentRestrictionState(RIL_NW_ALL_RESTRICTIONS, rid);
        }
        //onRestrictedStateChanged(rid);
    }
    releaseRadioPowerLock();
    return;

error:
    if (p_response != NULL)
        at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    releaseRadioPowerLock();
}

void requestResetRadio(void * data __unused, size_t datalen __unused, RIL_Token t) {
    property_set("vendor.ril.getccci.response","0");
    RIL_Errno err;
    if (isCdmaLteDcSupport()) {
        err = resetRadioForSvlte();
    } else {
        err = resetRadio();
    }
    RIL_onRequestComplete(t, err, NULL, 0);
    property_set("vendor.ril.getccci.response","1");
}

RIL_Errno resetRadio() {
    int err;
    ATResponse *p_response = NULL;
    RILChannelCtx* p_channel = NULL;
    int i =0;
    p_channel = getChannelCtxbyProxy();
    int cur3GSim = queryMainProtocol(RIL_SIM);

    RLOGI("start to reset radio");
    RLOGD("update 3G Sim property : %d", cur3GSim);
    setSimSwitchProp(cur3GSim);

    // only do power off when it is on
    if (s_md_off != 1)
    {
        //power off modem
        if (isCCCIPowerOffModem()) {
            s_md_off = 1;
        }

        /* Reset the modem, we will do the following steps
         * 1. AT+EPOF,
         *      do network detach and power-off the SIM
         *      By this way, we can protect the SIM card
         * 2. AT+EPON
         *      do the normal procedure of boot up
         * 3. stop muxd
         *      because we will re-construct the MUX channels
         * 4. The responsibility of Telephony Framework
         *    i. stop rild
         *    ii. start muxd to re-construct the MUX channels and start rild
         *    iii. make sure that it is OK if there is any request in the request queue
         */
        if (SIM_COUNT >= 2) {
           isModemResetStarted = 1;
        }
        sendATBeforeEPOF("AT+EMDT=0", "+EMDT:", 0 /* retry times */, 0 /* interval */,
                p_channel);
        err = at_send_command("AT+EPOF", &p_response, p_channel);

        if (err != 0 || p_response->success == 0) {
            RLOGE("There is something wrong with the exectution of AT+EPOF");
        }
        at_response_free(p_response);

        if (isCCCIPowerOffModem()) {
            if (s_enhanceModemOffVersion == 1) {
                LOGD("Flight mode power off modem, trigger CCCI level 2 power off (new version)");
                triggerCCCIIoctl(CCCI_IOC_ENTER_DEEP_FLIGHT_ENHANCED);
            } else {
                LOGD("Flight mode power off modem, trigger CCCI level 2 power off");
                triggerCCCIIoctl(CCCI_IOC_ENTER_DEEP_FLIGHT);
            }
        }
    }
    for(i = 0; i < getSimCount(); i++) {
        resetPhbReady(RIL_SOCKET_1 + i);
        RIL_onUnsolicitedResponse(RIL_UNSOL_STK_SETUP_MENU_RESET, NULL, 0, RIL_SOCKET_1+i);
    }

    //power on modem
    if (isCCCIPowerOffModem()) {
        if (s_enhanceModemOffVersion == 1) {
            LOGD("Flight mode power on modem, trigger CCCI power on modem (new versio)");
            triggerCCCIIoctl(CCCI_IOC_LEAVE_DEEP_FLIGHT_ENHANCED);
        } else {
            LOGD("Flight mode power on modem, trigger CCCI power on modem");
            triggerCCCIIoctl(CCCI_IOC_LEAVE_DEEP_FLIGHT);
        }
        if (SIM_COUNT >= 2) {
            isModemResetStarted = 0;
        }
    } else {
        err = at_send_command("AT+EPON", &p_response, p_channel);
        if (SIM_COUNT >= 2) {
            isModemResetStarted = 0;
        }

        if (err != 0 || p_response->success == 0) {
            LOGW("There is something wrong with the exectution of AT+EPON");
        }
        at_response_free(p_response);
    }
    s_main_loop = 0;
    return RIL_E_SUCCESS;
}

RIL_Errno resetRadioForSvlte() {
    int cur3GSim = queryMainProtocol(RIL_SIM);
    ATResponse *p_response = NULL;
    RILChannelCtx* p_channel = NULL;
    int err = -1;
    p_channel = getChannelCtxbyProxy();

    RLOGI("start to reset radio - resetRadioForSvlte");
    // only do power off when it is on
    if (s_enhanceModemOffVersion == 1 && RatConfig_isC2kSupported()) {
        property_set("vendor.ril.cdma.emdstatus.send", "0");
        property_set("vendor.cdma.ril.eboot", "1");
        RLOGI("%s: set vendor.ril.cdma.emdstatus.send to 0, vendor.cdma.ril.eboot to 1", __FUNCTION__);
    }
    if (getSimCount() >= 2) {
        RLOGD("update 3G Sim property : %d", cur3GSim);
        setSimSwitchProp(cur3GSim);
    }
    if (s_md_off != 1)
    {
        const int RETRY_TIMES = 20;
        const int RETRY_INTERVAL = 1 * 1000;
        LOGD("%s(): set radio off before power off MD1.", __FUNCTION__);
        sendATWithRetry(RETRY_TIMES, RETRY_INTERVAL, "AT+EFUN=0", p_channel);
        sendATBeforeEPOF("AT+EMDT=0", "+EMDT:", 0 /* retry times */, 0 /* interval */,
                p_channel);
        if (isCCCIPowerOffModem()) {
            s_md_off = 1;
        }

        isModemResetStarted = 1;
        err = at_send_command("AT+EPOF", &p_response, p_channel);
        if (err != 0 || p_response->success == 0) {
            RLOGW("There is something wrong with the exectution of AT+EPOF");
        }
        at_response_free(p_response);
    }

    property_set("vendor.ril.mux.report.case", "2");
    property_set("vendor.ril.muxreport", "1");
    s_main_loop = 0;

    return RIL_E_SUCCESS;
}

void initMdStatusReader() {
    LOGD("initMdStatusReader");
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    int result = pthread_create(&mdStatsuReaderThread, &attr, mdStatusReaderLoop, NULL);
    if (result != 0) {
        RLOGE("initMdStatusReader: Failed to create dispatch thread: %s", strerror(result));
        return;
    }
}

static int sMdStatusFd = -1;
static void *mdStatusReaderLoop(void* params __unused) {
    RLOGD("mdStatusReaderLoop - md status working thread");
    int count = -1;

    /* /dev/ccci_mdx_sta: contains all modem status
    *  /dev/ccci_md1_sta: contains gsm modem status
    *  /dev/ccci_md3_sta: contains c2k modem status
    */
    sMdStatusFd = open("/dev/ccci_mdx_sta", O_RDWR);
    if (sMdStatusFd < 0) {
        // Avoid reading unexpected data
        RLOGE("mdStatusReaderLoop: open node fail");
        return NULL;
    }
    RLOGD("mdStatusReaderLoop: fd = %d", sMdStatusFd);
    getResetLock();
    MdStatusEvent *buffer = (MdStatusEvent *) calloc(1, sizeof(MdStatusEvent));
    if (buffer == NULL) {
        RLOGE("OOM");
        releaseResetLock();
        return NULL;
    }

    do {
        count = read(sMdStatusFd, buffer, sizeof(MdStatusEvent));
        RLOGE("mdStatusReaderLoop: count: %d", count);
        RLOGE("mdStatusReaderLoop: modem id: %d, event type: %d, reason: %s",
                buffer->md_id, buffer->event_type, buffer->reason);

        if (MD_STA_EV_RESET_REQUEST == buffer->event_type
                || MD_STA_EV_STOP_REQUEST == buffer->event_type
                || MD_STA_EV_ENTER_FLIGHT_REQUEST == buffer->event_type
                || MD_STA_EV_ENTER_FLIGHT_E_REQUEST == buffer->event_type) {
            RLOGE("mdStatusReaderLoop: RESET/STOP/ENTER_FLIGHT, break");
            break;
        }
    } while (1);

    if (MD_STA_EV_RESET_REQUEST == buffer->event_type) {
        LOGE("mdStatusReaderLoop: update status to framework");
        int i;
        for(i = 0; i < getSimCount(); i++) {
            RIL_UNSOL_RESPONSE(RIL_UNSOL_MODEM_RESTART,
                    buffer->reason, sizeof(buffer->reason), i);
        }
        sleep(1);
    }
    releaseResetLock();
    free(buffer);
    close(sMdStatusFd);

    return NULL;
}

// get reset lock at bootup
void getResetLock() {
    LOGE("getResetLock");
    int ret = ioctl(sMdStatusFd, CCCI_IOC_HOLD_RST_LOCK, -1);
}

// release reset lock at following case
// 1. Leave flight mode
// 2. Receive MD_STA_EV_RESET_REQUEST
// 3. Receive MD_STA_EV_STOP_REQUEST
void releaseResetLock() {
    LOGE("releaseResetLock");
    int ret = ioctl(sMdStatusFd, CCCI_IOC_FREE_RST_LOCK, -1);
}
