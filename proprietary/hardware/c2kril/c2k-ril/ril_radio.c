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
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <termios.h>
#include "atchannel.h"
#include "ril_callbacks.h"
#include <utils/Log.h>

#include <cutils/properties.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/route.h>

#include "hardware/ccci_intf.h"
#include "ril_radio.h"
#include "ril_wp.h"
#include "misc.h"
#include "at_tok.h"
#include "c2k_ril_data.h"

#define RADIO_CHANNEL_CTX getRILChannelCtxFromToken(t)

extern int s_ps_on_desired;
extern RIL_UNSOL_Msg_Cache s_unsol_msg_cache;
extern int no_service_times;

extern int getCdmaModemSlot();
extern RUIM_Status getRUIMStatus(RADIO_STATUS_UPDATE isReportRadioStatus);
extern void resetUnsolMsgCache();
extern int waitUimModualInitialized(void);

static int last_flight_mode = 0;

int s_md3_off = 0;

int s_md3_vpon = 0;

//Add for radio request in different channel.
static pthread_mutex_t s_md3_radio_mutex = PTHREAD_MUTEX_INITIALIZER;
static const struct timeval TIMEVAL_0 = {0, 0};

static void requestRadioPower(void *data, size_t datalen, RIL_Token t);
static void requestModemPowerOn(void *data, size_t datalen, RIL_Token t);
static void requestModemPowerOff(void *data, size_t datalen, RIL_Token t);
static void requestModemPowerOffSyncC2K(void *data, size_t datalen, RIL_Token t);
static void requestSetThermalModem(void *data, size_t datalen, RIL_Token t);
static void requestResetRadio(void * data, size_t datalen, RIL_Token t);
static void requestShutdown(void * data,size_t datalen,RIL_Token t);
static void updateRadioStatus(void *param);
static void requestConfigModemStatus(void *data, size_t datalen, RIL_Token t);

extern int rilRadioMain(int request, void * data, size_t datalen, RIL_Token t) {
    switch(request) {
        case RIL_REQUEST_MODEM_POWEROFF:
            requestModemPowerOff(data, datalen, t);
            break;
        case RIL_LOCAL_REQUEST_MODEM_POWEROFF_SYNC_C2K:
            requestModemPowerOffSyncC2K(data, datalen, t);
            break;
        case RIL_REQUEST_MODEM_POWERON:
            requestModemPowerOn(data, datalen, t);
            break;
        case RIL_REQUEST_RADIO_POWER:
        case RIL_REQUEST_ENABLE_MODEM:
            requestRadioPower(data, datalen, t);
            break;
        case RIL_REQUEST_SHUTDOWN:
            requestShutdown(data, datalen, t);
            break;
        case RIL_REQUEST_SET_MODEM_THERMAL:
            requestSetThermalModem(data, datalen, t);
            break;
        case RIL_REQUEST_RESET_RADIO:
        case RIL_REQUEST_RESTART_RILD:
            requestResetRadio(data, datalen, t);
            break;
        case RIL_REQUEST_CONFIG_MODEM_STATUS_C2K:
            requestConfigModemStatus(data, datalen, t);
            break;
         default:
            // not support
            return 0;
    }
    return 1;
}

extern int rilRadioUnsolicited(const char *s, const char *sms_pdu __unused) {
    if (strStartsWith(s, "+VPON:1")) {
        LOGD("+VPON: 1, old: %d", s_md3_vpon);
        if (s_md3_vpon == 0) {
            s_md3_vpon = 1;
            RIL_requestProxyTimedCallback(updateRadioStatus, NULL, &TIMEVAL_0,
                    getChannelCtxbyProxy()->id);
    }
        return 1;
    } else if (strStartsWith(s, "+VPON:0")) {
        LOGD("+VPON: 0, old: %d", s_md3_vpon);
        s_md3_vpon = 0;
        return 1;
    }
    return 0;
}

int isRadioOn()
{
    return s_md3_vpon;
}
void updateRadioStatus(void *param __unused) {
    LOGD("updateRadioStatus");
    getRUIMStatus(UPDATE_RADIO_STATUS);
}

/** returns 1 if on, 0 if off, and -1 on error */
int getRadioStateInternal(RILChannelCtx *p_channel)
{
    ATResponse *p_response = NULL;
    int err;
    char *line;
    char display_en, ps_en;
    LOGD("getRadioStateInternal");
    err = at_send_command_singleline("AT+VPON?", "+VPON:", &p_response, p_channel);

    if (err < 0 || p_response == NULL || p_response->success == 0) {
        // assume radio is off
        goto error;
    }

    line = p_response->p_intermediates->line;

    err = at_tok_start(&line);
    if (err < 0) goto error;

    err = at_tok_nextbool(&line, &display_en);
    if (err < 0) goto error;

    err = at_tok_nextbool(&line, &ps_en);
    s_md3_vpon = (int)ps_en;
    if (err < 0) goto error;

    at_response_free(p_response);

    return (int)ps_en;

error:

    at_response_free(p_response);
    return -1;
}

// Add 2 functions for radio reqeust in different channels.
void grabRadioPowerLock(void)
{
    pthread_mutex_lock(&s_md3_radio_mutex);
}

void releaseRadioPowerLock(void)
{
    pthread_mutex_unlock(&s_md3_radio_mutex);
}

void requestRadioPower(void *data, size_t datalen, RIL_Token t)
{
    int onOff;

    int err;
    ATResponse *p_response = NULL;
    RUIM_Status uimStatus = RUIM_ABSENT;
    #ifdef OPEN_PS_ON_CARD_ABSENT
    int emergencyOpen = 0;
    #endif
    const long long CPOF_RETRY_TIMES = 3 * 60 * 1000LL;
    const long long CPON_RETRY_TIMES = 60 * 1000LL;

    assert (datalen >= sizeof(int *));
    onOff = ((int *)data)[0];

    LOGD("%s: onOff is %d, sState is %d", __FUNCTION__, onOff, getRadioState());
    if (onOff == 0 && getRadioState() != RADIO_STATE_OFF) {
        s_ps_on_desired = 0;
        LOGD("%s: s_ps_on_desired = %d", __FUNCTION__, s_ps_on_desired);
        // fix CPOF timing issue ALPS02546229
        // MD3 may report VPON status cause AP missing power off MD3
        // if(isRadioOn() == 1) {
            err = at_send_command_with_specified_timeout("AT+CPOF", CPOF_RETRY_TIMES, &p_response, RADIO_CHANNEL_CTX);
            if (err < 0 || p_response == NULL || p_response->success == 0) {
                int errCode = 0;
                if (p_response != NULL) {
                    errCode = at_get_cme_error(p_response);
                }
                if (errCode == CME_RADIO_NOT_ALLOWED_DURING_RAT_SWITCH){//during rat mode switch
                    at_response_free(p_response);
                    RIL_onRequestComplete(t, RIL_E_OP_NOT_ALLOWED_DURING_VOICE_CALL, NULL, 0);
                    return;
                } else if (errCode != 3) {// 3:already off
                    goto error;
                }
            }
            resetUnsolMsgCache();
        // }
        last_flight_mode = 1;
        setRadioState(RADIO_STATE_OFF);
    } else if (onOff > 0) {
        last_flight_mode = 0;
        s_ps_on_desired = 1;
        LOGD("%s: s_ps_on_desired = %d", __FUNCTION__, s_ps_on_desired);
        if (getRadioState() != RADIO_STATE_ON) {
            if (isRadioOn() != 1) {
                no_service_times = 1; /*Fix HREF#17811, do not return a fake value when PS openning*/

                /*Actually it should be done before CPIN? (getRUIMStatus)*/
                waitUimModualInitialized();
                uimStatus = getRUIMStatus(DO_NOT_UPDATE);
                if ((RUIM_PIN == uimStatus) || (RUIM_PUK == uimStatus)
                        || (RUIM_ABSENT == uimStatus)) {
                    //s_ps_on_desired = 1;
                    //LOGD("%s: s_ps_on_desired = %d", __FUNCTION__, s_ps_on_desired);
#ifdef OPEN_PS_ON_CARD_ABSENT
                    emergencyOpen = 1;
                    //setRadioState(RADIO_STATE_RUIM_LOCKED_OR_ABSENT);
#else
                    goto error;
#endif
                }
                s_unsol_msg_cache.service_state = 2;

                // lock frequency for lab test. @{
                char lockFreqProp[PROPERTY_VALUE_MAX] = { 0 };
                property_get("persist.vendor.sys.lock.freq.c2k", lockFreqProp, "0");
                int nLockFrep = atoi(lockFreqProp);
                LOGD("Lock C2K frequency: lockFreqProp = %s, nLockFrep = %d.", lockFreqProp, nLockFrep);
                if (nLockFrep == 1) {
                    at_send_command("AT+ELOCKCH=0,1,293", NULL, RADIO_CHANNEL_CTX);
                    at_send_command("AT+ELOCKCH=1,1,47", NULL, RADIO_CHANNEL_CTX);
                }
                // lock frequency for lab test. @}

                /// M:attach c2k if needed.
                combineDataAttach(RADIO_CHANNEL_CTX);

                err = at_send_command_with_specified_timeout("AT+CPON", CPON_RETRY_TIMES, &p_response, RADIO_CHANNEL_CTX);

                // MD3 will return CME_SIM_FAILURE when SIM not init, handle it in RilProxy
                if (p_response != NULL) {
                    int errCode = at_get_cme_error(p_response);
                    if (errCode == CME_SIM_FAILURE) {
                        LOGD("%s: failed to send CPON, errCode is CME_SIM_FAILURE", __FUNCTION__);
                        at_response_free(p_response);
                        RIL_onRequestComplete(t, RIL_E_POWER_ON_UIM_NOT_READY, NULL, 0);
                        return;
                    }
                }

                if (err < 0 || (p_response != NULL && p_response->success == 0)) {
                    LOGD("%s: failed to send CPON, err is %d", __FUNCTION__, err);
                    // Some stacks return an error when there is no SIM,
                    // but they really turn the RF portion on
                    // So, if we get an error, let's check to see if it
                    // turned on anyway
                    if (getRadioStateInternal(RADIO_CHANNEL_CTX) != 1) {
                        goto error;
                    }
                }
                // Update here to keep s_md3_vpon updated
                s_md3_vpon = 1;
                LOGD("%s: set s_md3_vpon = 1", __FUNCTION__);

                setRadioState(RADIO_STATE_ON);
            }
        }
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestModemPowerOffSyncC2K(void *data __unused, size_t datalen __unused, RIL_Token t)
{
    if (s_md3_off == 1) {
        LOGD("Flight mode power off modem but already modem powered off");
        property_set("vendor.ril.cdma.offsync", "1");
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        return;
    }
    s_md3_off = 1;

    setRadioState(RADIO_STATE_OFF);
    LOGD("%s: setRadioState to RADIO_STATE_OFF.", __FUNCTION__);

    property_set("vendor.ril.cdma.offsync", "1");

    // Add a checking for world mode switch, if modem off, it need to stop waiting.
    if (bWorldModeSwitching) {
        LOGD("Power off , onWorldModeChanged 1");
        onWorldModeChanged(1);
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

void requestModemPowerOff(void *data __unused, size_t datalen __unused, RIL_Token t)
{
    int err;
    ATResponse *p_response = NULL;
    ATResponse *p_rfs_response = NULL;
    ATResponse *p_cbp_version_response = NULL;
    char *cbp_version = NULL;
    char* line = NULL;
    const long long POWEROFF_RETRY_TIMES = (4 * 60 + 30) * 1000LL;

    /*sync rfs image only in ipo/normal power off cause force rfs image sync removed from CPOF in CBP8.2*/
    do
    {
        err = at_send_command_singleline("AT+VCGMM", "+VCGMM", &p_cbp_version_response, RADIO_CHANNEL_CTX);
        if (err < 0 || p_cbp_version_response == NULL || p_cbp_version_response->success == 0)
        {
            LOGD("%s: failed to send VCGMM, err is %d", __FUNCTION__, err);
            break;
        }
        line = p_cbp_version_response->p_intermediates->line;
        err = at_tok_start(&line);
        if (err < 0)
        {
            LOGD("%s: failed to at_tok_start", __FUNCTION__);
            break;
        }
        err = at_tok_nextstr(&line, &cbp_version);
        if (err < 0)
        {
            LOGD("%s: failed to at_tok_nextstr", __FUNCTION__);
            break;
        }
        if((NULL != cbp_version) &&
                (!strncmp(cbp_version, "CBP8", 4) || !strncmp(cbp_version, "MT6735", 6) || !strncmp(cbp_version, "MT6753", 6)))
        {
            err = at_send_command("AT+RFSSYNC", &p_rfs_response, RADIO_CHANNEL_CTX);
            if (err < 0 || p_rfs_response == NULL || p_rfs_response->success == 0)
            {
                LOGD("%s: failed to send RFSSYNC, err is %d", __FUNCTION__, err);
                break;
            }
        }
    } while (0);

    if (s_md3_off == 1)
    {
        LOGD("Flight mode power off modem but already modem powered off");
        at_response_free(p_response);
        at_response_free(p_cbp_version_response);
        at_response_free(p_rfs_response);
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        return;
    }
    s_md3_off = 1;

    int slotid = getCdmaModemSlot();
    switch (slotid) {
    case 1:
        property_set("vendor.ril.ipo.radiooff", "-1");
        LOGD("C2K set vendor.ril.ipo.radiooff to -1");
        break;
    case 2:
        property_set("vendor.ril.ipo.radiooff.2", "-1");
        LOGD("C2K set vendor.ril.ipo.radiooff.2 to -1");
        break;
    }

    LOGD("%s: sState is %d.", __FUNCTION__, getRadioState());
    /* Work around for ALPS02344902, 2015/10/19 {*/
    // To expand AT+EPOF timeout to 4min30sec + default 30sec = 5mins
    err = at_send_command_with_specified_timeout("AT+EPOF",
            POWEROFF_RETRY_TIMES, &p_response, RADIO_CHANNEL_CTX);
    /* Work around for ALPS02344902, 2015/10/19 }*/
    if (err < 0 || p_response == NULL || p_response->success == 0) goto error;
    LOGD("%s: AT+EPOF, err=%d.", __FUNCTION__, err);

    property_set("vendor.ril.cdma.emdstatus.send", "0");
    RLOGD("power off modem, set vendor.ril.cdma.emdstatus.send to 0");

    switch (slotid) {
        case 1:
            property_set("vendor.ril.ipo.radiooff", "1");
            LOGD("C2K set vendor.ril.ipo.radiooff to 1");
            break;
        case 2:
            property_set("vendor.ril.ipo.radiooff.2", "1");
            LOGD("C2K set vendor.ril.ipo.radiooff.2 to 1");
            break;
        default:
            LOGE("unsupport slot id %d", slotid);
            break;
    }

    last_flight_mode = 1;
    setRadioState(RADIO_STATE_OFF);
    LOGD("%s: setRadioState to RADIO_STATE_OFF.", __FUNCTION__);


    LOGD("%s: enter flight mode.", __FUNCTION__);
    triggerIoctl(CCCI_IOC_ENTER_DEEP_FLIGHT);

    // Add a checking for world mode switch, if modem off, it need to stop waiting.
    if (bWorldModeSwitching) {
        LOGD("Power off , onWorldModeChanged 1");
        onWorldModeChanged(1);
    }

    at_response_free(p_response);
    at_response_free(p_cbp_version_response);
    at_response_free(p_rfs_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    at_response_free(p_response);
    at_response_free(p_cbp_version_response);
    at_response_free(p_rfs_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestModemPowerOn(void *data __unused, size_t datalen __unused, RIL_Token t)
{
    LOGD("%s: leave flight mode.", __FUNCTION__);

    if (s_md3_off == 0)
    {
        LOGD("Flight mode power on modem but modem is already power on");
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        return;
    }

    LOGD("%s: leave flight mode.", __FUNCTION__);
    triggerIoctl(CCCI_IOC_LEAVE_DEEP_FLIGHT);
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
            LOGE("requestModemPowerOn, unsupport slot id %d", slotid);
            break;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
}

void requestShutdown(void *data __unused, size_t datalen __unused, RIL_Token t) {
    int err;
    ATResponse *p_response = NULL;
    ATResponse *p_rfs_response = NULL;
    ATResponse *p_cbp_version_response = NULL;
    char *cbp_version = NULL;
    char* line = NULL;
    const long long POWEROFF_RETRY_TIMES = (4 * 60 + 30) * 1000LL;

    if (s_md3_off == 1)
    {
        LOGD("Flight mode power off modem but already modem powered off");
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        return;
    }

    /*sync rfs image only in ipo/normal power off cause force rfs image sync removed from CPOF in CBP8.2*/
    do
    {
        err = at_send_command_singleline("AT+VCGMM", "+VCGMM", &p_cbp_version_response,
                getRILChannelCtxFromToken(t));
        if (err < 0 || p_cbp_version_response == NULL || p_cbp_version_response->success == 0)
        {
            LOGD("%s: failed to send VCGMM, err is %d", __FUNCTION__, err);
            break;
        }
        line = p_cbp_version_response->p_intermediates->line;
        err = at_tok_start(&line);
        if (err < 0)
        {
            LOGD("%s: failed to at_tok_start", __FUNCTION__);
            break;
        }
        err = at_tok_nextstr(&line, &cbp_version);
        if (err < 0)
        {
            LOGD("%s: failed to at_tok_nextstr", __FUNCTION__);
            break;
        }
        if((NULL != cbp_version) && (!strncmp(cbp_version, "CBP8", 4) ||
                !strncmp(cbp_version, "MT6735", 6) || !strncmp(cbp_version, "MT6753", 6)))
        {
            err = at_send_command("AT+RFSSYNC", &p_rfs_response, getRILChannelCtxFromToken(t));
            if (err < 0 || p_rfs_response == NULL || p_rfs_response->success == 0)
            {
                LOGD("%s: failed to send RFSSYNC, err is %d", __FUNCTION__, err);
                break;
            }
        }
    } while (0);

    s_md3_off = 1;

    int slotid = getCdmaModemSlot();
    switch (slotid) {
    case 1:
        property_set("vendor.ril.ipo.radiooff", "-1");
        LOGD("C2K set vendor.ril.ipo.radiooff to -1");
        break;
    case 2:
        property_set("vendor.ril.ipo.radiooff.2", "-1");
        LOGD("C2K set vendor.ril.ipo.radiooff.2 to -1");
        break;
    }

    LOGD("%s: sState is %d.", __FUNCTION__, getRadioState());
    /* Work around for ALPS02344902, 2015/10/19 {*/
    // To expand AT+EPOF timeout to 4min30sec + default 30sec = 5mins
    err = at_send_command_with_specified_timeout("AT+EPOF",
            POWEROFF_RETRY_TIMES, &p_response, getRILChannelCtxFromToken(t));
    /* Work around for ALPS02344902, 2015/10/19 }*/
    if (err < 0 || p_response == NULL || p_response->success == 0) goto error;
    LOGD("%s: AT+EPOF, err=%d.", __FUNCTION__, err);

    property_set("vendor.ril.cdma.emdstatus.send", "0");
    RLOGD("power off modem, set vendor.ril.cdma.emdstatus.send to 0");

    switch (slotid) {
        case 1:
            property_set("vendor.ril.ipo.radiooff", "1");
            LOGD("C2K set vendor.ril.ipo.radiooff to 1");
            break;
        case 2:
            property_set("vendor.ril.ipo.radiooff.2", "1");
            LOGD("C2K set vendor.ril.ipo.radiooff.2 to 1");
            break;
        default:
            LOGE("unsupport slot id %d", slotid);
            break;
    }

    // last_flight_mode = 1;
    setRadioState(RADIO_STATE_UNAVAILABLE);
    LOGD("%s: setRadioState to RADIO_STATE_UNAVAILABLE.", __FUNCTION__);


    LOGD("%s: enter flight mode.", __FUNCTION__);
    triggerIoctl(CCCI_IOC_ENTER_DEEP_FLIGHT);
    at_response_free(p_response);
    at_response_free(p_cbp_version_response);
    at_response_free(p_rfs_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    at_response_free(p_response);
    at_response_free(p_cbp_version_response);
    at_response_free(p_rfs_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestSetThermalModem(void *data, size_t datalen, RIL_Token t)
{
    assert(datalen >= sizeof(int *));
    int modem_on = ((int*) data)[0];
    char enhanceMDVersion[PROPERTY_VALUE_MAX] = { 0 };
    int enhanceMDVersionVal = 0;
    property_get("vendor.ril.cdma.enhance.version", enhanceMDVersion, "0");
    enhanceMDVersionVal = atoi(enhanceMDVersion);

    LOGD("%s: modem_on = %d, vendor.ril.cdma.enhance.version = %d.",
            __FUNCTION__, modem_on, enhanceMDVersionVal);
    if (modem_on) {
        if (enhanceMDVersionVal == 1) {
            LOGD("No need to power on c2k, return.");
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        } else {
            requestModemPowerOn(NULL, 0, t);
        }
    } else {
        if (enhanceMDVersionVal == 1) {
            requestModemPowerOffSyncC2K(NULL, 0, t);
        } else {
            requestModemPowerOff(NULL, 0, t);
        }
    }
}

void requestResetRadio(void * data __unused, size_t datalen __unused, RIL_Token t)
{
    int err;
    char* line = NULL;
    char *cbp_version = NULL;
    ATResponse *p_response = NULL;
    ATResponse *p_rfs_response = NULL;
    ATResponse *p_cbp_version_response = NULL;
    const long long POWEROFF_RETRY_TIMES = (4 * 60 + 30) * 1000LL;

    /*Reset Radio is trigger from GSM RILD, in C2K rild we need to turn of Modem before TRM.*/
    RLOGI("reset radio");

    /*sync rfs image only in ipo/normal power off cause force rfs image sync removed from CPOF in CBP8.2*/
    do
    {
        err = at_send_command_singleline("AT+VCGMM", "+VCGMM", &p_cbp_version_response, RADIO_CHANNEL_CTX);
        if (err < 0 || p_cbp_version_response == NULL || p_cbp_version_response->success == 0)
        {
            RLOGI("%s: failed to send VCGMM, err is %d", __FUNCTION__, err);
            break;
        }
        line = p_cbp_version_response->p_intermediates->line;
        err = at_tok_start(&line);
        if (err < 0)
        {
            RLOGI("%s: failed to at_tok_start", __FUNCTION__);
            break;
        }
        err = at_tok_nextstr(&line, &cbp_version);
        if (err < 0)
        {
            RLOGI("%s: failed to at_tok_nextstr", __FUNCTION__);
            break;
        }
        if((NULL != cbp_version) &&
                (!strncmp(cbp_version, "CBP8", 4) || !strncmp(cbp_version, "MT6735", 6) || !strncmp(cbp_version, "MT6753", 6)))
        {
            err = at_send_command("AT+RFSSYNC", &p_rfs_response, RADIO_CHANNEL_CTX);
            if (err < 0 || p_rfs_response == NULL ||p_rfs_response->success == 0)
            {
                RLOGI("%s: failed to send RFSSYNC, err is %d", __FUNCTION__, err);
                break;
            }
        }
    } while (0);

    RLOGI("%s: s_md3_off is %d", __FUNCTION__, s_md3_off);

    if (s_md3_off != 1) {
        s_md3_off = 1;

        RLOGI("%s: sState is %d.", __FUNCTION__, getRadioState());

        err = at_send_command_with_specified_timeout("AT+EPOF",
                POWEROFF_RETRY_TIMES, &p_response, RADIO_CHANNEL_CTX);
        if (err < 0 || p_response == NULL ||p_response->success == 0) goto error;

        property_set("vendor.ril.cdma.emdstatus.send", "0");
        property_set("vendor.cdma.ril.eboot", "1");
        RLOGI("%s: power off modem, set vendor.ril.cdma.emdstatus.send to 0, vendor.cdma.ril.eboot to 1", __FUNCTION__);

        setRadioState(RADIO_STATE_OFF);
        RLOGI("%s: setRadioState to RADIO_STATE_OFF.", __FUNCTION__);
    } else {
        RLOGI("%s: Flight mode power off modem but already modem powered off", __FUNCTION__);
    }

    at_response_free(p_response);
    at_response_free(p_cbp_version_response);
    at_response_free(p_rfs_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    RLOGI("error");
    at_response_free(p_response);
    at_response_free(p_cbp_version_response);
    at_response_free(p_rfs_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void requestConfigModemStatus(void *data, size_t datalen __unused, RIL_Token t)
{
    int modemStatus, remoteSimProtocol;
    int err;
    ATResponse *p_response = NULL;
    char *cmd = NULL;
    RIL_Errno ril_errno = RIL_E_GENERIC_FAILURE;

    modemStatus = ((int *)data)[0];
    remoteSimProtocol = ((int *)data)[1];

    if ((modemStatus < 0 || modemStatus > 2) || (remoteSimProtocol < 0 || remoteSimProtocol > 2)) {
        LOGE("requestConfigModemStatus para error!");
    } else {
        err = asprintf(&cmd, "AT+EMDSTATUS=%d, %d", modemStatus, remoteSimProtocol);
        if(err >= 0) {
            err = at_send_command(cmd, &p_response, RADIO_CHANNEL_CTX);
            if ( !(err < 0 || p_response == NULL || p_response->success == 0) ) {
                ril_errno = RIL_E_SUCCESS;
            }
            property_set("vendor.ril.cdma.emdstatus.send", "1");
            LOGD("c2k rild set vendor.ril.cdma.emdstatus.send to 1");
        }
        if (cmd != NULL) {
            free(cmd);
        }
    }
    RIL_onRequestComplete(t, ril_errno, NULL, 0);
    at_response_free(p_response);
    return;
}
