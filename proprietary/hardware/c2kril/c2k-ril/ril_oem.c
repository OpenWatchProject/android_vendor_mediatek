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
#include "at_tok.h"
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
#include <libmtkrilutils.h>
#include <ratconfig.h>
#include "ril_nw.h"
#include "c2k_ril_data.h"

#define LOG_TAG "C2K_RIL-OEM"

#define OEM_CHANNEL_CTX getRILChannelCtxFromToken(t)

#define RESET_MODE_ASSERT 31
#define RESET_MODE_RESET 32
#define TRIGGER_CDMA_RILD_NE 103

static void *noopRemoveOemWarning( void *a ) { return a; }
#define RIL_OEM_UNUSED_PARM(a) noopRemoveOemWarning((void *)&(a));

extern int arsithrehd;
extern int s_md3_off;
extern int s_isUserLoad;


int processing_md5_cmd = 0;

extern void requestSetRadioCapability(void * data, size_t datalen, RIL_Token t);
extern void requestGetRadioCapability(void * data, size_t datalen, RIL_Token t);
extern void resetUnsolMsgCache();
extern int at_send_command_raw(const char *command, ATResponse **pp_outResponse,
        RILChannelCtx *p_channel);
extern void notifyMd3InitDone();
extern void trigger_update_cache();
extern void onWorldModePrepareTRM(int resetType);
static void requestGetIMEI(void *data, size_t datalen, RIL_Token t);
static void requestOemHookRaw(void * data, size_t datalen, RIL_Token t);
static void requestOemHookStrings(void * data, size_t datalen, RIL_Token t);
static void requestScreenState(void *data, size_t datalen, RIL_Token t);
static void requestIdentity(void *data, size_t datalen, RIL_Token t);
/// M: [C2K 6M][NW] Config EVDO mode @{
static void requestConfigEvdoMode(void * data, size_t datalen, RIL_Token t);
static void requestSetRestrictModemState(void *data, size_t datalen, RIL_Token t);
static int configEiratMode();
static int isEratExtSupport();
static int isOp016mSupportExt();
/// M: [C2K 6M][NW] Config EVDO mode @}
static void requestBasebandVersion(void *data, size_t datalen, RIL_Token t);
/// M: Mode switch TRM feature. @{
static void requestModeSwitchSetTrm(void * data, size_t datalen, RIL_Token t);
/// @}
static void requestSetTrm(void *data, size_t datalen, RIL_Token t);
static void requestSetUnsolResponseFilter(void *data, size_t datalen, RIL_Token t);

static void requestSendDeviceState(void *data, size_t datalen, RIL_Token t);

static void requestAtCmdWithProxyCdma(void * data, size_t datalen, RIL_Token t);
/// M: C2k agps command
static void requestAgpsTcpConnected(void *data, size_t datalen, RIL_Token t);
// Lock the p_channelMutex so that the at command sending will be pended
static void lockAtChannel();
// Unlock the p_channelMutex and resume at command sending
static void unlockAtChannel();
// Set the val to s_simInitState with thread safe
static void simSwitchSetSimInitState(int val);
// Wait for sim init done or time out
static void simSwitchTimedWaitSimInit(long long msec);
/*Indicate current screen state, update in requestScreenState*/
int s_screenState = 1;
// Used for sync sim init state with thread safe
static pthread_mutex_t s_simInitStateMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_simInitStateCond = PTHREAD_COND_INITIALIZER;
static int s_simInitState = 0;
static const struct timeval TIMEVAL_0 = {0, 0};

int rilOemMain(int request, void *data, size_t datalen, RIL_Token t)
{
    switch (request) {
        case RIL_REQUEST_BASEBAND_VERSION:
            requestBasebandVersion(data, datalen, t);
            break;
        case RIL_REQUEST_GET_RADIO_CAPABILITY:
            requestGetRadioCapability(data, datalen, t);
            break;
        case RIL_REQUEST_SET_RADIO_CAPABILITY:
            requestSetRadioCapability(data, datalen, t);
            break;
        case RIL_REQUEST_SCREEN_STATE:
            requestScreenState(data, datalen, t);
            break;
        case RIL_REQUEST_GET_IMEI:
            requestGetIMEI(data, datalen, t);
            break;
        case RIL_REQUEST_OEM_HOOK_RAW:
            // echo back data
            LOGD("rilOemMain, RIL_REQUEST_OEM_HOOK_RAW");
            requestOemHookRaw(data, datalen, t);
            break;
        case RIL_REQUEST_OEM_HOOK_STRINGS: {
            int i;
            const char ** cur;
            LOGD("got OEM_HOOK_STRINGS: 0x%8p %lu", data, (long) datalen);
            for (i = (datalen / sizeof(char *)), cur = (const char **) data; i > 0;
                    cur++, i--) {
                LOGD("> '%s'", *cur);
            }
            requestOemHookStrings(data, datalen, t);
            }
            break;
        case RIL_REQUEST_DEVICE_IDENTITY:
            requestIdentity(data, datalen, t);
            break;
        /// M: [C2K 6M][NW] Config EVDO mode @{
        case RIL_LOCAL_C2K_REQUEST_CONFIG_EVDO_MODE:
            requestConfigEvdoMode(data, datalen, t);
            break;
        /// M: [C2K 6M][NW] Config EVDO mode @}
        /// M: [C2K]Dynamic switch support. @{
        case RIL_REQUEST_ENTER_RESTRICT_MODEM_C2K:
        case RIL_REQUEST_LEAVE_RESTRICT_MODEM_C2K:
            requestSetRestrictModemState(data, datalen, t);
            break;
        /// @}
        /// M: Mode switch TRM feature. @{
        case RIL_LOCAL_REQUEST_MODE_SWITCH_C2K_SET_TRM:
            requestModeSwitchSetTrm(data, datalen, t);
            break;
        /// @}
        case RIL_REQUEST_SET_TRM:
            requestSetTrm(data, datalen, t);
            break;
        case RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER:
            requestSetUnsolResponseFilter(data, datalen, t);
            break;
        case RIL_REQUEST_SEND_DEVICE_STATE:
            requestSendDeviceState(data, datalen, t);
            break;
        case RIL_LOCAL_C2K_REQUEST_AT_COMMAND_WITH_PROXY_CDMA:
            requestAtCmdWithProxyCdma(data, datalen, t);
            break;
        case RIL_LOCAL_C2K_REQUEST_AGPS_TCP_CONNIND:
            requestAgpsTcpConnected(data, datalen, t);
            break;
    default:
            return 0; /* no matched request */
    }
    return 1;
}

int rilOemUnsolicited(const char *s, const char *sms_pdu)
{
    UNUSED(sms_pdu);
    if (strStartsWith(s, "+VPUP"))
    {
        // Notify +VPUP received from MD3 to mainloop.
        notifyMd3InitDone();
        return 1;
    }
    else if (strStartsWith(s, "^GPSTCPCONNREQ"))
    {
        VIA_GPS_EVENT_Data gps_event_data;
        gps_event_data.event = REQUEST_DATA_CONNECTION;
        gps_event_data.gps_status = 0;
        RIL_onUnsolicitedResponse(RIL_LOCAL_C2K_UNSOL_VIA_GPS_EVENT, &gps_event_data,
                sizeof(gps_event_data));
        return 1;
    }
    else if (strStartsWith(s, "^GPSTCPCLOSEREQ"))
    {
        VIA_GPS_EVENT_Data gps_event_data;
        gps_event_data.event = CLOSE_DATA_CONNECTION;
        gps_event_data.gps_status = 0;
        RIL_onUnsolicitedResponse(RIL_LOCAL_C2K_UNSOL_VIA_GPS_EVENT, &gps_event_data,
                sizeof(gps_event_data));
        return 1;
    }
    return 0;
}

void triggerIoctl(int param){
    RLOGD("triggerIoctl, param=%d", param);

#ifdef MTK_ECCCI_C2K
    char ret[32];
    int fd = -1;
    snprintf(ret, 32, "%s", ccci_get_node_name(USR_RILD_IOCTL, MD_SYS3));
    fd = open(ret, O_RDWR);
    if(fd >= 0) {
        ioctl(fd, param);
        close(fd);
    } else {
        LOGE("open ccci_get_node_name USR_RILD_IOCTL failed, errno: %d", errno);
    }
#else /* MTK_ECCCI_C2K */
    switch (param)
    {
    case CCCI_IOC_MD_RESET:
        C2KReset();
        break;
    case CCCI_IOC_FORCE_MD_ASSERT:
        C2KForceAssert();
        break;
    case CCCI_IOC_ENTER_DEEP_FLIGHT:
        C2KEnterFlightMode();
        break;
    case CCCI_IOC_LEAVE_DEEP_FLIGHT:
        C2KLeaveFlightMode();
        break;
    default:
        exit(0);
        break;
    }
#endif
}

void initRadioCapabilityResponse(RIL_RadioCapability* rc, RIL_RadioCapability* copyFromRC) {
    memset(rc, 0, sizeof(RIL_RadioCapability));
    rc->version = RIL_RADIO_CAPABILITY_VERSION;
    rc->session = copyFromRC->session;
    rc->phase = copyFromRC->phase;
    rc->rat = copyFromRC->rat;
    strncpy(rc->logicalModemUuid, copyFromRC->logicalModemUuid, MAX_UUID_LENGTH - 1);
    rc->status = copyFromRC->status;
}

void getMSimProperty(int phoneId, char *pPropertyName, char *pPropertyValue) {
    char prop[PROPERTY_VALUE_MAX] = {0};
    char value[PROPERTY_VALUE_MAX] = {0};
    int count = 0;
    int propLen = 0;
    int i = 0;
    int j = 0;
    property_get(pPropertyName, prop, "");
    //LOGD("getMSimProperty pPropertyName=%s, prop=%s", pPropertyName, prop);
    propLen = strlen(prop);
    for (i = 0; i < propLen; i++) {
        if (prop[i] == ',') {
            count++;
            if ((count-1) == phoneId) {
                // return current buffer
                //LOGD("getMSimProperty found! phoneId=%d, value =%s", phoneId, value);
                strncpy(pPropertyValue, value, PROPERTY_VALUE_MAX - 1);
                pPropertyValue[PROPERTY_VALUE_MAX - 1] = '\0';
                return;
            } else {
                // clear current buffer
                j = 0;
                memset(value, 0, sizeof(char) * PROPERTY_VALUE_MAX);
            }
        } else {
            value[j] = prop[i];
            j++;
        }
    }
    if (count == phoneId) {
        strncpy(pPropertyValue, value, PROPERTY_VALUE_MAX - 1);
        pPropertyValue[PROPERTY_VALUE_MAX - 1] = '\0';
        //LOGD("getMSimProperty found at end! phoneId=%d, value =%s", phoneId, value);
    }
}

int isExternalSimOnlySlot(RIL_SOCKET_ID rid) {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    property_get("ro.vendor.mtk_external_sim_only_slots", property_value, "0");
    int supported = atoi(property_value) & (1 << rid);

    RLOGI("[isExternalSimOnlySlot] vsimOnlySlots:%d, supported:%d", atoi(property_value), supported);
    return ((supported > 0) ? 1 : 0);
}

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

int isVsimEnabledByRid(int rid) {
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

int isVsimEnabled() {
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

    LOGD("[VSIM] isVsimEnabled is 1.");

    return enabled;
}

static void simSwitchSetSimInitState(int val) {
    pthread_mutex_lock(&s_simInitStateMutex);
    s_simInitState = val;
    pthread_mutex_unlock(&s_simInitStateMutex);
}

void simSwitchNotifySimInitDone() {
    pthread_mutex_lock(&s_simInitStateMutex);
    LOGI("notifySimInitDone, state=0x%x", s_simInitState);
    s_simInitState = 1;
    pthread_cond_signal(&s_simInitStateCond);
    pthread_mutex_unlock(&s_simInitStateMutex);
}

#define NS_PER_S 1000000000
static void simSwitchTimedWaitSimInit(long long msec) {
    pthread_mutex_lock(&s_simInitStateMutex);
    if (s_simInitState == 0) {
        struct timeval tv;
        struct timespec ts;
        gettimeofday(&tv, (struct timezone *) NULL);
        ts.tv_sec = tv.tv_sec + (msec / 1000);
        ts.tv_nsec = (tv.tv_usec + (msec % 1000) * 1000L ) * 1000L;
        /* assuming tv.tv_usec < 10^6 */
        if (ts.tv_nsec >= NS_PER_S) {
            ts.tv_sec++;
            ts.tv_nsec -= NS_PER_S;
        }
        if (ETIMEDOUT == pthread_cond_timedwait(&s_simInitStateCond, &s_simInitStateMutex, &ts)) {
            LOGI("wait sim init timeout");
        }
    }
    pthread_mutex_unlock(&s_simInitStateMutex);
}

extern void requestSetRadioCapability(void * data, size_t datalen, RIL_Token t)
{
    RIL_OEM_UNUSED_PARM(datalen);
    char sRcSessionId[32] = {0};
    RIL_RadioCapability rc;
    int err = 0;
    ATResponse *p_rfs_response = NULL;
    char *cbp_version = NULL;
    ATResponse *p_cbp_version_response = NULL;
    char *line = NULL;
    memcpy(&rc, data, sizeof(RIL_RadioCapability));
    LOGD("requestSetRadioCapability : %d, %d, %d, %d, %s, %d",
        rc.version, rc.session, rc.phase, rc.rat, rc.logicalModemUuid, rc.status);

    memset(sRcSessionId, 0, sizeof(sRcSessionId));
    sprintf(sRcSessionId,"%d",rc.session);

    RIL_RadioCapability* responseRc = (RIL_RadioCapability*) malloc(sizeof(RIL_RadioCapability));
    assert(responseRc);
    initRadioCapabilityResponse(responseRc, &rc);
    switch (rc.phase) {
        case RC_PHASE_START:
            LOGD("requestSetRadioCapability RC_PHASE_START");
            if (isSvlteSupport() != 1) {
                LOGD("requestSetRadioCapability not svlte project");
            }

            responseRc->status = RC_STATUS_SUCCESS;
            RIL_onRequestComplete(t, RIL_E_SUCCESS, responseRc, sizeof(RIL_RadioCapability));
            break;
        case RC_PHASE_APPLY:
            if (isSvlteSupport() == 1 || isVsimEnabled() == 1 || isPersistVsim() == 1) {
                err = at_send_command_singleline("AT+VCGMM", "+VCGMM",
                    &p_cbp_version_response, getRILChannelCtxFromToken(t));
                if (err < 0 || p_cbp_version_response->success == 0) {
                    LOGD("%s: failed to send VCGMM, err is %d", __FUNCTION__, err);
                    break;
                }
                line = p_cbp_version_response->p_intermediates->line;
                err = at_tok_start(&line);
                if (err < 0) {
                    LOGD("%s: failed to at_tok_start", __FUNCTION__);
                    break;
                }
                err = at_tok_nextstr(&line, &cbp_version);
                if (err < 0) {
                    LOGD("%s: failed to at_tok_nextstr", __FUNCTION__);
                    break;
                }
                // there is no need to do this before CBP8.X
                if ((NULL != cbp_version) && (!strncmp(cbp_version, "CBP8", 4))) {
                    err = at_send_command("AT+RFSSYNC", &p_rfs_response, getRILChannelCtxFromToken(t));
                    if (err < 0 || p_rfs_response->success == 0) {
                        LOGD("%s: failed to send RFSSYNC, err is %d", __FUNCTION__, err);
                        break;
                    }
                }
                if (s_md3_off != 1) {
                    s_md3_off = 1;
                    LOGD("%s: sState is %d.", __FUNCTION__, getRadioState());
                    err = at_send_command("AT+EPOF", NULL, getRILChannelCtxFromToken(t));
                    LOGD("%s: AT+EPOF, err=%d.", __FUNCTION__, err);
                    property_set("vendor.cdma.ril.eboot", "1");
                    setRadioState(RADIO_STATE_OFF);
                }
            } else {
                LOGD("requestSetRadioCapability RC_PHASE_APPLY");
                err = at_send_command("AT+EBOOT=1", NULL, getRILChannelCtxFromToken(t));
                LOGD("%s: AT+EBOOT=1, err=%d.", __FUNCTION__, err);
                resetUnsolMsgCache();
                err = at_send_command("AT+ERMS=1", NULL, getRILChannelCtxFromToken(t));
                LOGD("%s: AT+ERMS=1, err=%d.", __FUNCTION__, err);
                if (isDssNoResetSupport() && hasCdmaCard()) {
                    lockAtChannel(t);
                    responseRc->status = RC_STATUS_SUCCESS;
                    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseRc,
                                          sizeof(RIL_RadioCapability));
                    simSwitchSetSimInitState(0);
                    simSwitchTimedWaitSimInit(60000);
                    unlockAtChannel(t);
                    break;
                }
            }
            responseRc->status = RC_STATUS_SUCCESS;
            RIL_onRequestComplete(t, RIL_E_SUCCESS, responseRc, sizeof(RIL_RadioCapability));
            break;

        default:
            LOGD("requestSetRadioCapability default, phase %d", rc.phase);
            responseRc->status = RC_STATUS_FAIL;
            RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
            break;
    }
    free(responseRc);
    at_response_free(p_cbp_version_response);
    at_response_free(p_rfs_response);
}

extern void requestGetRadioCapability(void * data, size_t datalen, RIL_Token t)
{
    RIL_OEM_UNUSED_PARM(data);
    RIL_OEM_UNUSED_PARM(datalen);
    RILChannelId rid = RIL_queryMyChannelId(t);
    LOGD("current RID : %d, %lu", rid, datalen);

    RIL_RadioCapability* rc = (RIL_RadioCapability*) calloc(1, sizeof(RIL_RadioCapability));
    assert(rc);
    rc->version = RIL_RADIO_CAPABILITY_VERSION;
    rc->session = 0;
    rc->phase = RC_PHASE_CONFIGURED;
    rc->rat = (1 << RADIO_TECH_GSM);
    rc->status = RC_STATUS_NONE;

    LOGD("requestGetRadioCapability : %d, %d, %d, %d, %s, %d, rild:%d",
            rc->version, rc->session, rc->phase, rc->rat, rc->logicalModemUuid, rc->status, rid);

    //C2K rild always return the same capability
    rc->rat = ((1 << RADIO_TECH_GSM)) | (1 << RADIO_TECH_UMTS);
    //C2K rild always return this string,because MD3 not has uid
    strncpy(rc->logicalModemUuid, "modem_sys3" , MAX_UUID_LENGTH - 1);
    rc->logicalModemUuid[MAX_UUID_LENGTH - 1] = '\0';
    RIL_onRequestComplete(t, RIL_E_SUCCESS, rc, sizeof(RIL_RadioCapability));
}

void requestGetIMEI(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    int err = 0, count, i;
    ATResponse *p_response = NULL;
    char *line = NULL;
    char ** responseStr = NULL;
    ATLine *p_cur;

    err = at_send_command_multiline("AT+GSN", "+GSN:", &p_response, OEM_CHANNEL_CTX);
    if ((err < 0) || (p_response != NULL && p_response->success == 0))
    {
        goto error;
    }

    /* count the entries */
    for (count = 0, p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
        count++;
    }
    responseStr = (char **) calloc(1, sizeof(char *) * count);
    if (responseStr == NULL) goto error;

    for (i = 0, p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
        line = p_cur->line;
        LOGD("requestGetIMEI:  before at_tok_start line = %s",line);
        err = at_tok_start(&line);
        if (err < 0)
        {
            goto error;
        }

        err = at_tok_nextstr(&line, &responseStr[i]);
        LOGD("requestGetIMEI: responseStr = %s",responseStr[i]);
        i++;
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, responseStr, strlen(responseStr));
    at_response_free(p_response);
    free(responseStr);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    if (responseStr != NULL) free(responseStr);
}


static void requestScreenState(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    int err = 0;
    char *cmd;

    assert (datalen >= sizeof(int *));
    s_screenState = ((int*)data)[0];
    LOGD("requestScreenState");
    if (s_screenState == 1) {
        LOGD("requestScreenState on");
        // system("echo LCD > /sys/class/power_supply/twl4030_bci_bk_battery/device/status_on");
        // err = at_send_command("AT+ARSI=1,4", NULL, OEM_CHANNEL_CTX);
        asprintf(&cmd, "AT+ARSI=1,%d", arsithrehd);
        err = at_send_command(cmd, NULL, OEM_CHANNEL_CTX);
        if (err < 0) goto error;
        err = at_send_command("AT+VMCCMNC=1", NULL, OEM_CHANNEL_CTX);
        if (err < 0) goto error;
        // err = at_send_command("AT+SAMEVSER=1", NULL);
        // if (err < 0) goto error;
        err = at_send_command("AT+CREG=1", NULL, OEM_CHANNEL_CTX);
        if (err < 0) goto error;
        // err = at_send_command("AT+SAMECREG=1", NULL);
        // if (err < 0) goto error;
        // err = at_send_command("AT+SAMEVMCCMNC=1", NULL);
        // if (err < 0) goto error;
        err = at_send_command("AT+MODE=1", NULL, OEM_CHANNEL_CTX);
        if (err < 0) goto error;
        err = at_send_command("AT+DORMONOFF=1", NULL, OEM_CHANNEL_CTX);
        if (err < 0) goto error;
        err = at_send_command("AT+CIEV=1,106", NULL, OEM_CHANNEL_CTX);
        if (err < 0) goto error;
        err = at_send_command("AT+VSER=1", NULL, OEM_CHANNEL_CTX);
        if (err < 0) goto error;

        // Enable presentation of "+EIPRL" URC
        err = at_send_command("AT+EIPRL=1", NULL, OEM_CHANNEL_CTX);
        if (err < 0) {
          LOGD("requestScreenState(), send AT+EIPRL fail");
        }

        err = at_send_command("AT+EFCELL=1", NULL, OEM_CHANNEL_CTX);
        if (err < 0) {
          LOGD("requestScreenState(), enable AT+EFCELL fail");
        }

        RIL_requestProxyTimedCallback(trigger_update_cache, NULL, &TIMEVAL_0, DEFAULT_CHANNEL);

        // by Hyan, query network status again once screen is on
        RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
    } else if (s_screenState == 0) {
        LOGD("requestScreenState off");
        // system("echo LCD > /sys/class/power_supply/twl4030_bci_bk_battery/device/status_off");

        // err = at_send_command("AT+ARSI=0,4", NULL);
        asprintf(&cmd, "AT+ARSI=0,%d", arsithrehd);
        err = at_send_command(cmd, NULL, OEM_CHANNEL_CTX);
        if (err < 0) goto error;
        err = at_send_command("AT+VSER=0", NULL, OEM_CHANNEL_CTX);
        if (err < 0) goto error;
        // err = at_send_command("AT+SAMEVSER=0", NULL);
        // if (err < 0) goto error;
        err = at_send_command("AT+CREG=0", NULL, OEM_CHANNEL_CTX);
        if (err < 0) goto error;
        // err = at_send_command("AT+SAMECREG=0", NULL);
        // if (err < 0) goto error;
        err = at_send_command("AT+VMCCMNC=0", NULL, OEM_CHANNEL_CTX);
        if (err < 0) goto error;
        err = at_send_command("AT+CIEV=0,106", NULL, OEM_CHANNEL_CTX);
        if (err < 0) goto error;
        // err = at_send_command("AT+SAMEVMCCMNC=0", NULL);
        // if (err < 0) goto error;
        err = at_send_command("AT+MODE=0", NULL, OEM_CHANNEL_CTX);
        if (err < 0) goto error;
        err = at_send_command("AT+DORMONOFF=0", NULL, OEM_CHANNEL_CTX);
        if (err < 0) goto error;

        // disable presentation of "+EIPRL" URC
        err = at_send_command("AT+EIPRL=0", NULL, OEM_CHANNEL_CTX);
        if (err < 0) {
          LOGD("requestScreenState(), send AT+EIPRL fail");
        }

        err = at_send_command("AT+EFCELL=0", NULL, OEM_CHANNEL_CTX);
        if (err < 0) {
          LOGD("requestScreenState(), disable AT+EFCELL fail");
        }
    } else {
        /* Not a defined value - error */
        goto error;
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);

    return;

error:
    LOGE("ERROR: requestScreenState failed");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestIdentity(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    RIL_Identity response;
    int err = 0;
    ATResponse *p_response = NULL;
    ATResponse *p_response_imei = NULL;
    ATResponse *p_responsenext = NULL;
    ATResponse *p_responseuimid = NULL;
    ATLine *p_cur;
    char *line;
    char *skip = NULL;
    char *line_imei;
    char *linenext;
    char *lineuimid;
    char *tmp = NULL;
    memset(&response, 0, sizeof(response));

    err = at_send_command_singleline("AT+VGMUID?", "+VGMUID:", &p_response_imei, OEM_CHANNEL_CTX);
    if ((err != 0) || (p_response_imei->success == 0))
    {
        goto error;
    }

    line_imei = p_response_imei->p_intermediates->line;
    err = at_tok_start(&line_imei);
    if (err < 0)
    {
        goto error;
    }
    err = at_tok_nextstr(&line_imei, &skip);
    if (err < 0)
    {
        goto error;
    }
    err = at_tok_nextstr(&line_imei, &response.imei);
    if(err < 0)
    {
        goto error;
    }

    if(!strncmp(response.imei, "0x", 2))
    {
        response.imei = response.imei + 2;
    }


    err = at_send_command_multiline ("AT+GSN", "+GSN:", &p_response, OEM_CHANNEL_CTX);
    if ((err != 0) || (p_response->success == 0))
    {
        goto error;
    }

    // New modem response order: Dec->Hex->OK. But new rild + old medem, rild would receive Hex only
    p_cur = p_response->p_intermediates;
    line = p_cur->line;
    err = at_tok_start(&line);
    if (err < 0)
    {
        goto error;
    }
    err = at_tok_nextstr(&line, &tmp);
    if(err < 0)
    {
        goto error;
    }
    if (strstr(tmp, "x") != NULL) {
        response.esnHex = tmp;
        if (!s_isUserLoad) RLOGD("response.esnHex = %s", response.esnHex);
    } else {
        response.esnDec = tmp;
        if (!s_isUserLoad) RLOGD("response.esnDec = %s", response.esnDec);
    }

    if (p_cur->p_next != NULL) {
        line = p_cur->p_next->line;
        err = at_tok_start(&line);
        if (err < 0)
        {
            goto error;
        }
        err = at_tok_nextstr(&line, &tmp);
        if(err < 0)
        {
            goto error;
        }
        if (strstr(tmp, "x") != NULL) {
            response.esnHex = tmp;
            if (!s_isUserLoad) RLOGD("response.esnHex = %s", response.esnHex);
        } else {
            response.esnDec = tmp;
            if (!s_isUserLoad) RLOGD("response.esnDec = %s", response.esnDec);
        }
    }

    err = at_send_command_multiline ("AT^MEID", "^MEID:", &p_responsenext, OEM_CHANNEL_CTX);
    if ((err != 0) || (p_responsenext->success == 0))
    {
        goto error;
    }

    p_cur = p_responsenext->p_intermediates;
    linenext = p_cur->line;
    err = at_tok_start(&linenext);
    if (err < 0)
    {
        goto error;
    }
    err = at_tok_nextstr(&linenext, &tmp);
    if (err < 0)
    {
        goto error;
    }
    if (strstr(tmp, "x") != NULL) {
        response.meidHex = tmp;
        if (!s_isUserLoad) RLOGD("response.meidHex = %s", response.meidHex);
    } else {
        response.meidDec = tmp;
        if (!s_isUserLoad) RLOGD("response.meidDec = %s", response.meidDec);
    }

    if (p_cur->p_next != NULL) {
        linenext = p_cur->p_next->line;
        err = at_tok_start(&linenext);
        if (err < 0)
        {
            goto error;
        }
        err = at_tok_nextstr(&linenext, &tmp);
        if (err < 0)
        {
            goto error;
        }
        if (strstr(tmp, "x") != NULL) {
            response.meidHex = tmp;
            if (!s_isUserLoad) RLOGD("response.meidHex = %s", response.meidHex);
        } else {
            response.meidDec = tmp;
            if (!s_isUserLoad) RLOGD("response.meidDec = %s", response.meidDec);
        }
    }
    if(!strncmp(response.meidHex, "0x", 2))
    {
        response.meidHex = response.meidHex + 2;
    }
    err = at_send_command_singleline ("AT+CCID?", "+CCID:", &p_responseuimid, OEM_CHANNEL_CTX);
    if ((err != 0) || (p_responseuimid->success == 0))
    {
        goto error;
    }
    lineuimid = p_responseuimid->p_intermediates->line;
    err = at_tok_start(&lineuimid);
    if (err < 0)
    {
        goto error;
    }
    err = at_tok_nextstr(&lineuimid, &response.uimid);
    if (err < 0)
    {
        goto error;
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, &response, sizeof(response));
    at_response_free(p_response);
    at_response_free(p_response_imei);
    at_response_free(p_responsenext);
    at_response_free(p_responseuimid);
    return;
error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    at_response_free(p_response_imei);
    at_response_free(p_responsenext);
    at_response_free(p_responseuimid);
}


static void requestAgpsTcpConnected(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    int err = 0;
    ATResponse *p_response = NULL;
    char * cmd = NULL;
    int connected = ((int*) data)[0];

    asprintf(&cmd, "AT^GPSTCPCONNIND=%d", connected);
    err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);
    free(cmd);
    if (err < 0 || p_response == NULL || p_response->success == 0)
    {
        goto error;
    }

    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
error:
    at_response_free(p_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestOemHookRaw(void * data, size_t datalen, RIL_Token t)
{
    /* atci start */
    ATResponse * p_response = NULL;
    ATLine* p_cur = NULL;
    const char* buffer = (char*)data;
    char* line;
    int i;
    int strLength = 0;
    int err = -1;

    RLOGD("data = %s, length = %lu", buffer, datalen);

    err = at_send_command_raw(buffer, &p_response, OEM_CHANNEL_CTX);

    if (err < 0) {
        RLOGE("OEM_HOOK_RAW fail");
        goto error;
    }

    RLOGD("p_response->success = %d", p_response->success);
    RLOGD("p_response->finalResponse = %s", p_response->finalResponse);

    ///M: If the final response is CDMA call id assign URC, then notify rilCcUnsolicited. @{
    // handleCdmaCallIdAssignUrc(p_response->finalResponse);
    /// @}

    strLength += 2; //for the pre tag of the first string in response.

    for (p_cur = p_response->p_intermediates; p_cur != NULL;
        p_cur = p_cur->p_next) {
        RLOGD("p_response->p_intermediates = <%s>", p_cur->line);
        strLength += (strlen(p_cur->line) + 2); //M:To append \r\n
    }
    strLength += (strlen(p_response->finalResponse) + 2);
    RLOGD("strLength = %d", strLength);

    int size = strLength * sizeof(char) + 1;
    line = (char *) alloca(size);
    assert(line);
    memset(line, 0, size);

    strlcpy(line, "\r\n", size);

    for (i = 0, p_cur = p_response->p_intermediates; p_cur != NULL;
        p_cur = p_cur->p_next, i++)
    {
       strlcat(line, p_cur->line, size);
       strlcat(line, "\r\n", size);
       RLOGD("line[%d] = <%s>", i, line);
    }
    strlcat(line, p_response->finalResponse, size);
    strlcat(line, "\r\n", size);
    RLOGD("line = <%s>", line);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, size);

     at_response_free(p_response);
     return;

error:
    line = (char *) alloca(10);
    assert(line);
    memset(line, 0, 10);
    strlcpy(line, "\r\nERROR\r\n", 10);
    RLOGD("line = <%s>", line);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, line, strlen(line));

    at_response_free(p_response);
    return;
}

static void requestOemHookStrings(void * data, size_t datalen, RIL_Token t)
{
    int i;
    const char ** cur;
    ATResponse *    p_response = NULL;
    int             err = -1;
    ATLine*         p_cur = NULL;
    char**          line;
    int             strLength = datalen / sizeof(char *);
    RIL_Errno       ret = RIL_E_GENERIC_FAILURE;

    LOGD("got OEM_HOOK_STRINGS: 0x%8p %lu", data, (long)datalen);
    for (i = strLength, cur = (const char **)data ;
         i > 0 ; cur++, i --) {
        LOGD("> '%s'", *cur);
    }

    if (strLength != 2) {
        /* Non proietary. Loopback! */
        RIL_onRequestComplete(t, RIL_E_SUCCESS, data, datalen);
        return;
    }

    /* For AT command access */
    cur = (const char **)data;
    if (NULL != cur[1] && strlen(cur[1]) != 0) {
        if ((strncmp(cur[1],"+CIMI",5) == 0) ||(strncmp(cur[1],"+CGSN",5) == 0)) {
            err = at_send_command_numeric(cur[0], &p_response, OEM_CHANNEL_CTX);
        } else {
            err = at_send_command_multiline(cur[0],cur[1], &p_response, OEM_CHANNEL_CTX);
        }
    } else {
        err = at_send_command(cur[0],&p_response, OEM_CHANNEL_CTX);
    }

    if (err < 0 || NULL == p_response) {
        LOGE("OEM_HOOK_STRINGS fail");
        goto error;
    }

    /* Count response length */
    strLength = 0;

    for (p_cur = p_response->p_intermediates; p_cur != NULL;
        p_cur = p_cur->p_next)
        strLength++;

    if (strLength == 0) {
        char *pRet = strdup("OK");
        RIL_onRequestComplete(t, RIL_E_SUCCESS, &pRet, sizeof(char *));
        free(pRet);
    } else {
        LOGI("%d of %s received!",strLength, cur[1]);
        line = (char **) alloca(strLength * sizeof(char *));
        assert(line);
        for (i = 0, p_cur = p_response->p_intermediates; p_cur != NULL;
            p_cur = p_cur->p_next, i++)
        {
            line[i] = p_cur->line;
        }
        RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strLength * sizeof(char *));
    }
    at_response_free(p_response);
    return;

error:
    RIL_onRequestComplete(t, ret, NULL, 0);
    at_response_free(p_response);
}

/// M: [C2K 6M][NW] Config EVDO mode @{

/// M: [C2K]Dynamic switch support. @{
static int configEiratMode() {
    int ril_errno = 0;
    int cdmaslotid = getCdmaSocketSlotId();
    // get 4g switch capability id
    char tempstr[PROPERTY_VALUE_MAX];
    int currMajorSim = 1;
    memset(tempstr, 0, sizeof(tempstr));
    property_get("persist.vendor.radio.simswitch", tempstr, "1");
    currMajorSim = atoi(tempstr);
    LOGD("configEiratMode cdmaslotid =  %d, currMajorSim =  %d", cdmaslotid, currMajorSim);

    if (cdmaslotid == currMajorSim) {
        LOGD("configEiratMode LC");
        if (isSvlteCdmaOnlySetFromEngMode() == 1) {
            ril_errno = at_send_command("AT+EIRATMODE=3", NULL, getChannelCtxbyProxy());
        } else if (isSvlteLcgSupport()) {
            ril_errno = at_send_command("AT+EIRATMODE=2", NULL, getChannelCtxbyProxy());
        } else {
            ril_errno = at_send_command("AT+EIRATMODE=1", NULL, getChannelCtxbyProxy());
        }
        if (ril_errno < 0) {
            return ril_errno;
        }
        if (!isEratExtSupport()) {
            ril_errno = at_send_command("AT^PREFMODE=8", NULL, getChannelCtxbyProxy());
        }
        if (ril_errno < 0) {
            return ril_errno;
        }
    } else {
        LOGD("configEiratMode c");
        ril_errno = at_send_command("AT+EIRATMODE=0", NULL, getChannelCtxbyProxy());
        if (ril_errno < 0) {
            return ril_errno;
        }
        if (!isEratExtSupport()) {
            ril_errno = at_send_command("AT^PREFMODE=2", NULL, getChannelCtxbyProxy());
            if (ril_errno < 0) {
                return ril_errno;
            }
        }
    }
    return ril_errno;
}
/// @}

static int isEratExtSupport() {
    int isEratExtSupport = 0;
    char eratext[PROPERTY_VALUE_MAX] = { 0 };
    property_get("vendor.ril.nw.erat.ext.support", eratext, "-1");
    if (0 == strcmp(eratext, "1")) {
        isEratExtSupport = 1;
    }
    return isEratExtSupport;
}

/// M: OP01 6M @{
static int isOp016mSupportExt() {
    char optr[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.vendor.operator.optr", optr, "");

    return (RatConfig_isC2kSupported() == 1) && ((strcmp("OP01", optr) == 0) ? 1 : 0);
}
/// @}

static void requestConfigEvdoMode(void * data, size_t datalen, RIL_Token t) {
    UNUSED(datalen);
    int err = 0;
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    int value = ((int *) data)[0];
    LOGD("%s: value is %d", __FUNCTION__, value);
    /// M: [C2K]Dynamic switch support. @{
    if (isSrlteSupport() || isOp016mSupportExt()) {
        err = configEiratMode();
        if (err < 0) {
            goto error;
        }
    }
    /// @}
    if (!isEratExtSupport()) {
        asprintf(&cmd, "AT+EVDOMODE=%d", value);
        err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);
        free(cmd);
        if ((err < 0) || (p_response->success == 0)) {
            LOGE("%s: send at error", __FUNCTION__);
            goto error;
        }
    }
    if (p_response != NULL) {
        at_response_free(p_response);
        p_response = NULL;
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    if (p_response != NULL) {
        at_response_free(p_response);
        p_response = NULL;
    }
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}
/// M: [C2K 6M][NW] Config EVDO mode @}


/// M: [C2K]Dynamic switch support. @{
static void requestSetRestrictModemState(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(datalen);
    int err = 0;
    char *cmd = NULL;
    ATResponse *p_response = NULL;
    int value = ((int *)data)[0];
    LOGD("%s: value is %d", __FUNCTION__, value);
    asprintf(&cmd, "AT+ERMS=%d", value);
    err = at_send_command(cmd, &p_response, OEM_CHANNEL_CTX);
    free(cmd);
    if ((err < 0) || (p_response->success == 0))
    {
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
/// @}

static void requestBasebandVersion(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    int err = 0;
    ATResponse *p_response = NULL;
    char * response = NULL;
    char *response_hard = NULL;
    char* line = NULL;
    ATResponse *p_response_soft = NULL;
    char * response_sof = NULL;
    char * response_string1 = NULL;
    char * response_string2 = NULL;
    char * response_string3 = NULL;
    char * response_string4 = NULL;
    char * response_string5 = NULL;
    char * response_string6 = NULL;
    char * response_string7 = NULL;
    char * response_string8 = NULL;
    char * response_string11 = NULL;
    char * line_sof = NULL;

    err = at_send_command_singleline("AT+VCGMM", "+VCGMM", &p_response, OEM_CHANNEL_CTX);
    if ((err != 0) || (p_response != NULL && p_response->success == 0))
    {
        goto error;
    }

    line = p_response->p_intermediates->line;
    err = at_tok_start(&line);
    if (err < 0)
    {
        goto error;
    }
    err = at_tok_nextstr(&line, &response_hard);
    if (err < 0)
    {
        goto error;
    }

    err = at_send_command_singleline("AT+VCUSVER?", "+VCUSVER", &p_response_soft, OEM_CHANNEL_CTX);
    if ((err != 0) || (p_response_soft != NULL && p_response_soft->success == 0))
    {
        goto error;
    }

    line_sof = p_response_soft->p_intermediates->line;
    err = at_tok_start(&line_sof);

    if (err < 0) {
        goto error;
    }

    err = at_tok_nextstr(&line_sof, &response_string1);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_sof, &response_string2);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_sof, &response_string3);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_sof, &response_string4);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_sof, &response_string5);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_sof, &response_string6);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_sof, &response_string7);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_sof, &response_string8);
    if (err < 0) goto error;
    err = at_tok_nextstr(&line_sof, &response_string11);
    if (err < 0) goto error;

    asprintf(&response, "%s, 20%s/%s/%s %s:%s",
                response_string11, response_string6,
                response_string4, response_string5, response_string7,
                response_string8);
    LOGD("SVER response = %s, %s",response, response_string11);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(char *));
    at_response_free(p_response);
    at_response_free(p_response_soft);
    free(response);
    return;

error:
    at_response_free(p_response);
    at_response_free(p_response_soft);
    LOGE("ERROR: requestBasebandVersion failed\n");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

/// M: Mode switch TRM feature. @{
void requestModeSwitchSetTrm(void *data, size_t datalen, RIL_Token t)
{
    UNUSED(data);
    UNUSED(datalen);
    int err;
    ATResponse *p_response = NULL;
    ATResponse *p_rfs_response = NULL;
    ATResponse *p_cbp_version_response = NULL;
    char *cbp_version = NULL;
    char* line = NULL;

    /*sync rfs image only in ipo/normal power off cause force rfs image sync removed from CPOF in CBP8.2*/
    do
    {
        err = at_send_command_singleline("AT+VCGMM", "+VCGMM",
                &p_cbp_version_response, getRILChannelCtxFromToken(t));
        if (err < 0 || (p_cbp_version_response != NULL
                && p_cbp_version_response->success == 0))
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
                (!strncmp(cbp_version, "CBP8", 4)
                || !strncmp(cbp_version, "MT6735", 6) || !strncmp(cbp_version, "MT6753", 6)))
        {
            err = at_send_command("AT+RFSSYNC", &p_rfs_response, getRILChannelCtxFromToken(t));
            if (err < 0 || (p_rfs_response != NULL
                    && p_rfs_response->success == 0))
            {
                LOGD("%s: failed to send RFSSYNC, err is %d", __FUNCTION__, err);
                break;
            }
        }
    } while (0);

    if (s_md3_off == 1)
    {
        LOGD("%s: power off modem but already modem powered off", __FUNCTION__);
        at_response_free(p_cbp_version_response);
        at_response_free(p_rfs_response);
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
        return;
    }
    s_md3_off = 1;

    LOGD("%s: sState is %d.", __FUNCTION__, getRadioState());
    err = at_send_command("AT+EPOF", &p_response, getRILChannelCtxFromToken(t));
    LOGD("%s: AT+EPOF, err=%d.", __FUNCTION__, err);
    if (err < 0 || (p_response != NULL && p_response->success == 0)) {
        goto error;
    }

    property_set("vendor.ril.cdma.emdstatus.send", "0");
    RLOGD("power off modem, set vendor.ril.cdma.emdstatus.send to 0");

    at_response_free(p_response);
    at_response_free(p_cbp_version_response);
    at_response_free(p_rfs_response);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    LOGD("%s: error", __FUNCTION__);
    at_response_free(p_response);
    at_response_free(p_cbp_version_response);
    at_response_free(p_rfs_response);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}
/// @}

/**
 *description:support framework to reset modem both active & inactive ways
 *parameter 1:means framework just want an exception from modem
 *parameter 2:means framework want reset modem
 */
static void requestSetTrm(void *data, size_t datalen, RIL_Token t) {
    UNUSED(datalen);
    int resetType = 0;

    resetType = ((int *) data)[0];
    LOGD("requestSetTrm resetType is %d", resetType);

    onWorldModePrepareTRM(resetType);

    /// for sim pin test
    property_set("vendor.ril.cdma.report.case", "1");

    /// for svlte,c card is also need G modem unlock sim pin
    if (isCdmaLteDcSupport()) {
        property_set("vendor.ril.mux.report.case", "2");
    }

    if (RESET_MODE_ASSERT == resetType) {  // MD3 assert(active mode)
        LOGD("Request CDMA MD assert new.");

        triggerIoctl(CCCI_IOC_FORCE_MD_ASSERT);

    } else if (RESET_MODE_RESET == resetType) {  // MD3 reset(inactive mode)
        LOGD("Request CDMA MD reset new.");
        triggerIoctl(CCCI_IOC_MD_RESET);

    } else if (TRIGGER_CDMA_RILD_NE == resetType) {  // RILD NE
        LOGD("Request CDMA RILD NE.");
        // Make RILD NE here, %s format with int value caused NE.
        LOGD("requestSetTrm resetType is %d", resetType);
    }

    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;
}

void requestSendDeviceState(void *data, size_t datalen, RIL_Token t) {
    UNUSED(datalen);
    RIL_DeviceStateType stateType = ((RIL_DeviceStateType *)data)[0];

    switch (stateType) {
        case RIL_DST_LOW_DATA_EXPECTED:
            // To be implemented by data module
            // Remove the below when implement done
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            break;
        default:
        case RIL_DST_CHARGING_STATE:
        case RIL_DST_POWER_SAVE_MODE:
            // do nothing
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            break;
    }
}

static void requestSetUnsolResponseFilter(void *data, size_t datalen,
        RIL_Token t) {
    UNUSED(datalen);
    int err = 0;
    RIL_UnsolicitedResponseFilter filter
            = ((RIL_UnsolicitedResponseFilter *)data)[0];
    if ((filter & RIL_UR_SIGNAL_STRENGTH) == RIL_UR_SIGNAL_STRENGTH) {
        // enable
        err = setUnsolResponseFilterSignalStrength(true, t);
        if (err < 0) goto error;
    } else {
        // disable
        err = setUnsolResponseFilterSignalStrength(false, t);
        if (err < 0) goto error;
    }
    if ((filter & RIL_UR_FULL_NETWORK_STATE) == RIL_UR_FULL_NETWORK_STATE) {
        // enable
        err = setUnsolResponseFilterNetworkState(true, t);
        if (err < 0) goto error;
    } else {
        // disable
        err = setUnsolResponseFilterNetworkState(false, t);
        if (err < 0) goto error;
    }
    if ((filter & RIL_UR_DATA_CALL_DORMANCY_CHANGED)
            == RIL_UR_DATA_CALL_DORMANCY_CHANGED) {
        // enable
    } else {
        // disable
    }
    if ((filter & LINK_CAPACITY_ESTIMATE) == LINK_CAPACITY_ESTIMATE) {
        // enable
        err = setUnsolResponseFilterLinkCapacityEstimate(true, t);
        if (err < 0) goto error;
    } else {
        // disable
        err = setUnsolResponseFilterLinkCapacityEstimate(false, t);
        if (err < 0) goto error;
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    return;

error:
    LOGE("ERROR: requestSetUnsolResponseFilter failed");
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

static void requestAtCmdWithProxyCdma(void * data,
        size_t datalen, RIL_Token t) {
    ATResponse * p_response = NULL;
    ATLine* p_cur = NULL;
    const char* buffer = (char*)data;
    char* line = NULL;
    int i;
    int strLength = 0;
    int err = -1;

    err = at_send_command_raw(buffer, &p_response, getRILChannelCtxFromToken(t));

    if (err < 0 || p_response == NULL) {
        RLOGE("AT_CMD_P_CDMA: AT_COMMAND_WITH_PROXY fail");
        goto error;
    }

    for (p_cur = p_response->p_intermediates; p_cur != NULL;
            p_cur = p_cur->p_next) {
        strLength += (strlen(p_cur->line) + 4);
    }
    strLength += (strlen(p_response->finalResponse)+ 4);

    int size = strLength + 1;  // +1 for '\0'
    line = (char *)calloc(1, size);
    assert(line);

    for (i = 0, p_cur = p_response->p_intermediates; p_cur != NULL;
            p_cur = p_cur->p_next, i++) {
        strlcat(line, "\r\n", size);
        strlcat(line, p_cur->line, size);
        strlcat(line, "\r\n", size);
    }

    strlcat(line, "\r\n", size);
    strlcat(line, p_response->finalResponse, size);
    strlcat(line, "\r\n", size);

    RLOGD("AT_CMD_P_CDMA: "
            " data = %s, length = %lu ,"
            " p_response->success = %d ,"
            " p_response->finalResponse = %s ,"
            " strLength = %d , line = <%s>",
            buffer, datalen,
            p_response->success,
            p_response->finalResponse,
            strLength, line);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, line, strLength);
    at_response_free(p_response);
    if (line) {
        free(line);
    }
    return;

error:
    line = (char *) calloc(1, 10);
    assert(line);
    strlcpy(line, "\r\nERROR\r\n", 10);
    RLOGD("AT_CMD_P_CDMA: line = <%s>", line);
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, line, strlen(line));
    at_response_free(p_response);
    if (line) {
        free(line);
    }
    return;
}
