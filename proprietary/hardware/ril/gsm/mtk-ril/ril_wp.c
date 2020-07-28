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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <ratconfig.h>
#include <dirent.h>
#include <telephony/mtk_ril.h>
#include <libmtkrilutils.h>
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
#include <ril_callbacks.h>
#include <ril_wp.h>
#include <ril_sim.h>
#include <ril_radio.h>
#include <ril_oem.h>
#include <ril_nw.h>
#include "ratconfig.h"

int bUbinSupport = 1; //support Universal BIN(worldmode)
int bWorldModeSwitching = 0;  //[ALPS02277365]
int bWorldModeSetRadioState = -1;
extern void setStkServiceRunningFlag(RIL_SOCKET_ID rid, bool flag);
extern void clearAllPdnInfoInDetail();
extern int SetModeToKernal(int modemMode, int iRat);
extern void queryActiveMode(RIL_SOCKET_ID rid, RILChannelCtx *p_channel);
extern void setRadioState(RIL_RadioState newState, RIL_SOCKET_ID rid);
extern void grabRadioPowerLock(void);
extern void releaseRadioPowerLock(void);
extern void updateRadioCapability(void);
extern void handleWorldModeChangedNoSimReset(void *param);
extern void sendATWithRetry(int times, int interval, const char* at_cmd, RILChannelCtx* p_channel);

static const struct timeval TIMEVAL_1S = {1, 0};
static int mRetryResumeCount = 0;
static const struct timeval TIMEVAL_0 = {0,0};
static RIL_SOCKET_ID worldModeBackupSocketRid = RIL_SOCKET_1;

int ecsraUrcParams[5] = {0};
RIL_RadioState radioStateBackup[MAX_SIM_COUNT] = {0};

extern int inCallNumber;
static const struct timeval TIMEVAL_5S = {5, 0};

#define WP_COMMAND_MAX_RETRY_COUNT (30)
#define WP_RELOAD_MODEM_TYPE (1)
#define WP_STORE_MODEM_TYPE (2)

bool isWorldModeRemoveSimReset() {
    char removeSimResetState[PROPERTY_VALUE_MAX] = {0};
    property_get("vendor.ril.nw.wm.no_reset_support", removeSimResetState, "0");
    LOGD("[isWorldModeRemoveSimReset] state= %s", removeSimResetState);
    if (0 == strcmp(removeSimResetState, "1")) {
        return true;
    }
    return false;
}

void onWorldModePrepareTRM(int mode) {
    int rid;
    if ( bWorldModeSwitching == 0 ) {
        return;
    }
    rid = RIL_get3GSIM() - 1;
    switch ( mode ) {
        case 1:
        case 2:
        case 19:  // Only for hot-plug
        {
            int status[1] = {0};
            status[0] = 1;  // notify world mode change as end
            RIL_UNSOL_RESPONSE(RIL_UNSOL_WORLD_MODE_CHANGED,
                    status, sizeof(status), RIL_SOCKET_1 + rid);
            break;
        }
        default:
            break;
    }
}

void onWorldModeStateChanged(int state) {
    if (true == isWorldModeRemoveSimReset()) {
        LOGD("[onWorldModeStateChanged]-remove sim reset");
        return;
    }
    if (state == 1){
        int i;

        LOGD("[onWorldModeStateChanged]-start");
        clearAllPdnInfoInDetail();

        // reset aIs_stk_service_running[] flag and clear queued Proactive Command
        // when world mode changed start
        for (i = 0; i < SIM_COUNT; i++) {
            setStkServiceRunningFlag(RIL_SOCKET_1 + i, false);
            // freeStkQueuedProactivCmdByRid(RIL_SOCKET_1 + i);
        }
    } else if (state == 0){
        int i;
        LOGD("[onWorldModeStateChanged]-end");
        for (i = 0; i < SIM_COUNT; i++) {
            setStkServiceRunningFlag(RIL_SOCKET_1 + i, false);
            // freeStkQueuedProactivCmdByRid(RIL_SOCKET_1 + i);
        }
    } else if (state == 2){
        int i;

        RLOGD("[onWorldModeStateChanged]-before_available");
        for (i = 0; i < SIM_COUNT; i++) {
            setStkServiceRunningFlag(RIL_SOCKET_1 + i, false);
            // freeStkQueuedProactivCmdByRid(RIL_SOCKET_1 + i);
            resetAidInfo(RIL_SOCKET_1 + i);
        }
    }
}

/* Handle +ECSRA URC */
void onWorldModeChanged(char* urc, const RIL_SOCKET_ID rid) {
    int err;
    int* ecsraParams = &ecsraUrcParams[0];
    int status[1] = {0};

    TimedCallbackParam* mParam = (TimedCallbackParam*)malloc(sizeof(TimedCallbackParam));
    if (mParam == NULL) {
        LOGD("[onWorldModeChanged] malloc memory fail");
        return;
    }

    // AT+ECSRA:<UTRAN-FDD>,<UTRAN-TDD-LCR>,<E-UTRAN-FDD>,<E-UTRAN-TDD>,<cause>
    // Parse URC parameters.
    err = at_tok_start(&urc);
    if (err < 0) goto error;
    err = at_tok_nextint(&urc, &ecsraParams[0]);//<UTRAN-FDD>
    if (err < 0) goto error;
    err = at_tok_nextint(&urc, &ecsraParams[1]);//<UTRAN-TDD-LCR>
    if (err < 0) goto error;
    err = at_tok_nextint(&urc, &ecsraParams[2]);//<E-UTRAN-FDD>
    if (err < 0) goto error;
    err = at_tok_nextint(&urc, &ecsraParams[3]);//<E-UTRAN-TDD>
    if (err < 0) goto error;
    err = at_tok_nextint(&urc, &ecsraParams[4]);//<cause>
    if (err < 0) goto error;

    mParam->rid = rid;
    mParam->urc = NULL;

    worldModeBackupSocketRid = rid;
    if (false == isWorldModeRemoveSimReset()) {
        RIL_requestProxyTimedCallback(handleWorldModeChanged, mParam, &TIMEVAL_0,
                getRILChannelId(RIL_SIM, getMainProtocolRid()), "handleWorldModeChanged");
    } else {
        // notify world mode changed start
        status[0] = 0;
        LOGD("[onWorldModeChanged] state= %d", status[0]);
        property_set("persist.vendor.radio.wm_state", "0");
        RIL_UNSOL_RESPONSE(RIL_UNSOL_WORLD_MODE_CHANGED, status, sizeof(status), rid);

        // [ALPS02277365] set flag when world mode changed start
        bWorldModeSwitching = 1;
        onWorldModeStateChanged(bWorldModeSwitching);

        if (RatConfig_isC2kSupported()) {
            free(mParam->urc);
            free(mParam);
        } else {
            RIL_requestProxyTimedCallback(handleWorldModeChangedNoSimReset, mParam, &TIMEVAL_0,
                    getRILChannelId(RIL_SIM, getMainProtocolRid()),
                    "handleWorldModeChangedNoSimReset");
        }
    }
    return;
error:
    if (mParam != NULL) {
        free(mParam);
    }
}

void requestResumeWorldModeChanged(void * data, size_t datelen, RIL_Token t) {
    RIL_SOCKET_ID rid = worldModeBackupSocketRid;

    int err;
    int i;
    int wm = -1;
    char* cmd = NULL;
    int* params = &ecsraUrcParams[0];
    int status = {0};
    ATResponse *p_response = NULL;

    const int switchingDone = 0;
    const int beforeRestoreRadio = 2;

    // return the latest reported +ECRA URC
    err = asprintf(&cmd, "AT+ECSRA=2,%d,%d,%d,%d,%d", params[0], params[1], params[2], params[3], params[4]);
    LOGD("handleWorldModeChanged(), send command %s", cmd);

    err = at_send_command(cmd, &p_response, getChannelCtxbyProxy());

    queryActiveMode(rid, getChannelCtxbyProxy());

    // [ALPS02277365] reset flag when world mode changed end
    bWorldModeSwitching = 0;

    onWorldModeStateChanged(switchingDone);

    // Re-read World Mode after world mode switch
    triggerCCCIIoctlEx(CCCI_IOC_GET_MD_TYPE, &wm);
    setActiveModemType(wm);

    LOGD("[handleWorldModeChanged] new world mode = %d", wm);

    if (err < 0 || p_response->success == 0) {
        // notify world mode changed done but modem failure
        status = -1;
        LOGD("[handleWorldModeChanged] state = %d", status);
        RIL_onRequestComplete (t, RIL_E_GENERIC_FAILURE, &status, sizeof(int));
    } else {
        // notify world mode changed done
        status = 1;
        LOGD("[handleWorldModeChanged] state = %d", status);
        RIL_onRequestComplete (t, RIL_E_SUCCESS, &status, sizeof(int));
    }
    RIL_UNSOL_RESPONSE (RIL_UNSOL_WORLD_MODE_CHANGED, &status, sizeof(status), rid);
    updateRadioCapability();
    property_set("persist.vendor.radio.wm_state", "1");
    at_response_free(p_response);
    p_response = NULL;
    free(cmd);
}

void handleWorldModeChangedNoSimReset(void *param) {
    TimedCallbackParam* mParam = (TimedCallbackParam*)param;
    char* urc = mParam->urc;
    RIL_SOCKET_ID rid = (RIL_SOCKET_ID)(mParam->rid);
    int err;
    int i;
    int wm = -1;
    char* cmd = NULL;
    char* worldModeCmd = NULL;
    int* params = &ecsraUrcParams[0];
    int status[1] = {0};
    RIL_RadioState* backup_radio_state = &radioStateBackup[0];
    ATResponse *p_response = NULL;
    const int switchingDone = 0;
    const int beforeRestoreRadio = 2;
    const int RETRY_TIMES = 20;
    const int RETRY_INTERVAL = 1 * 1000;
    RILChannelCtx* p_channel = getChannelCtxbyProxy();

    grabRadioPowerLock();
    if (getSimCount() >= 2) {
        err = asprintf(&cmd, "AT+EFUN=0");
    } else {
        err = asprintf(&cmd, "AT+CFUN=4");
    }
    LOGD("handleWorldModeChanged(), poweroff %s", cmd);
    sendATWithRetry(RETRY_TIMES, RETRY_INTERVAL, cmd, p_channel);
    free(cmd);
    // backup radio state and set to RADIO_STATE_OFF
    for ( i=0; i < getSimCount(); i++ ) {
        backup_radio_state[i] = getRadioState(RIL_SOCKET_1 + i);
        LOGD("[onWorldModeChanged]backup_radio_state[%d]=%d", i, backup_radio_state[i]);
        setRadioState(RADIO_STATE_OFF, RIL_SOCKET_1 +i);
    }

    // return the latest reported +ECRA URC
    err = asprintf(&worldModeCmd, "AT+ECSRA=2,%d,%d,%d,%d,%d",
            params[0], params[1], params[2], params[3], params[4]);
    LOGD("handleWorldModeChanged(), send command %s", worldModeCmd);

    err = at_send_command(worldModeCmd, &p_response, getChannelCtxbyProxy());

    queryActiveMode(rid, getChannelCtxbyProxy());

    // [ALPS02277365] reset flag when world mode changed end
    bWorldModeSwitching = 0;

    onWorldModeStateChanged(switchingDone);
    releaseRadioPowerLock();
    // Re-read World Mode after world mode switch
    triggerCCCIIoctlEx(CCCI_IOC_GET_MD_TYPE, &wm);
    setActiveModemType(wm);

    LOGD("[handleWorldModeChanged] new world mode = %d", wm);

    if (err < 0 || p_response->success == 0) {
        // notify world mode changed done but modem failure
        status[0] = -1;
        LOGD("[handleWorldModeChanged] state = %d", status[0]);
        RIL_UNSOL_RESPONSE (RIL_UNSOL_WORLD_MODE_CHANGED, status, sizeof(status), rid);
    } else {
        // notify world mode changed done
        status[0] = 1;
        LOGD("[handleWorldModeChanged] state = %d", status[0]);
        RIL_UNSOL_RESPONSE (RIL_UNSOL_WORLD_MODE_CHANGED, status, sizeof(status), rid);
    }
    updateRadioCapability();
    property_set("persist.vendor.radio.wm_state", "1");
    at_response_free(p_response);
    p_response = NULL;
    free(worldModeCmd);
    free(mParam->urc);
    free(mParam);
}

void handleWorldModeChanged(void *param) {
    TimedCallbackParam* mParam = (TimedCallbackParam*)param;
    char* urc = mParam->urc;
    RIL_SOCKET_ID rid = (RIL_SOCKET_ID)(mParam->rid);
    int err;
    int i;
    int wm = -1;
    char* cmd = NULL;
    int* params = &ecsraUrcParams[0];
    int status[1] = {0};
    RIL_RadioState* backup_radio_state = &radioStateBackup[0];
    ATResponse *p_response = NULL;
    const int switchingDone = 0;
    const int beforeRestoreRadio = 2;
    char modemOffState[PROPERTY_VALUE_MAX] = {0};

    // pending world mode switch request in calling state
    if (inCallNumber > 0) {
        LOGD("[onWorldModeChanged] calling state= %d", inCallNumber);
        RIL_requestProxyTimedCallback(handleWorldModeChanged, param, &TIMEVAL_5S,
                getRILChannelId(RIL_SIM, getMainProtocolRid()), "handleWorldModeReryChanged");
        return;
    }

    property_get("vendor.ril.ipo.radiooff", modemOffState, "0");
    LOGD("worldmode vendor.ril.ipo.radiooff state= %s", modemOffState);
    // backup radio state and set to RADIO_STATE_UNAVAILABLE
    if (mRetryResumeCount == 0) {
        // before set radio unavailable, no need to notify world mode state
        if (strcmp("1", modemOffState) == 0) {
            free(mParam->urc);
            free(mParam);
            return;
        }
        // notify world mode changed start
        status[0] = 0;
        LOGD("[onWorldModeChanged] state= %d", status[0]);
        property_set("persist.vendor.radio.wm_state", "0");
        RIL_UNSOL_RESPONSE(RIL_UNSOL_WORLD_MODE_CHANGED, status, sizeof(status), rid);

        // [ALPS02277365] set flag when world mode changed start
        bWorldModeSwitching = 1;
        bWorldModeSetRadioState = 1;
        onWorldModeStateChanged(bWorldModeSwitching);

        // ALPS03274303
        grabRadioPowerLock();
        // backup radio state and set to RADIO_STATE_UNAVAILABLE
        for (i = 0; i < getSimCount(); i++) {
            backup_radio_state[i] = getRadioState(RIL_SOCKET_1 + i);
            LOGD("[onWorldModeChanged]backup_radio_state[%d]=%d",
                    i, backup_radio_state[i]);
            setRadioState(RADIO_STATE_UNAVAILABLE, RIL_SOCKET_1 +i);
        }
    }

    // return the latest reported +ECRA URC
    err = asprintf(&cmd, "AT+ECSRA=2,%d,%d,%d,%d,%d", params[0], params[1], params[2], params[3], params[4]);
    LOGD("handleWorldModeChanged(), send command %s", cmd);

    err = at_send_command(cmd, &p_response, getChannelCtxbyProxy());

    if (err < 0 || p_response->success == 0) {
        // notify world mode changed done but modem failure
        mRetryResumeCount++;
        status[0] = -1;
        LOGD("[handleWorldModeChanged] state = %d, retryCount = %d", status[0], mRetryResumeCount);
        if (strcmp("1", modemOffState) == 0) {
            // if radio set modem off, world mode no need to retry again, end world mode change.
            LOGD("[handleWorldModeChanged] not expected end since modemOffState is 1");
        } else if (mRetryResumeCount <= WP_COMMAND_MAX_RETRY_COUNT) {
            at_response_free(p_response);
            p_response = NULL;
            free(cmd);
            cmd = NULL;
            RIL_requestProxyTimedCallback(handleWorldModeChanged, param, &TIMEVAL_1S,
                    getRILChannelId(RIL_SIM, getMainProtocolRid()), "handleWorldModeReryChanged");
            return;
        }
    } else {
        status[0] = 1;
    }

    mRetryResumeCount = 0;
    onWorldModeStateChanged(beforeRestoreRadio);
    queryActiveMode(rid, getChannelCtxbyProxy());
    // store radio state and notify event
    for (i = 0; i < getSimCount(); i++) {
        LOGD("[handleWorldModeChanged]restore radio_state[%d]=%d",i, backup_radio_state[i]);
        setRadioState(backup_radio_state[i], RIL_SOCKET_1+i);
    }
    releaseRadioPowerLock();
    // [ALPS02277365] reset flag when world mode changed end
    bWorldModeSwitching = 0;
    bWorldModeSetRadioState = 0;

    // world mode switching, pending special URC.
    sendPendedUrcs();

    onWorldModeStateChanged(switchingDone);

    // Re-read World Mode after world mode switch
    triggerCCCIIoctlEx(CCCI_IOC_GET_MD_TYPE, &wm);
    setActiveModemType(wm);
    LOGD("[handleWorldModeChanged] new world mode = %d", wm);

    // notify world mode changed done
    LOGD("[handleWorldModeChanged] state = %d", status[0]);
    RIL_UNSOL_RESPONSE(RIL_UNSOL_WORLD_MODE_CHANGED, status, sizeof(status), rid);

    property_set("persist.vendor.radio.wm_state", "1");
    at_response_free(p_response);
    p_response = NULL;
    free(cmd);
    free(mParam->urc);
    free(mParam);
    RLOGI("&&&& wm end");
}

int getWorldPhonePolicy() {
    int  policy = 0;
    int  op01_policy = 0;
    char optr[PROPERTY_VALUE_MAX] = {0};
    char prop_val[PROPERTY_VALUE_MAX] = {0};

    property_get("persist.vendor.operator.optr", optr, "");

    if (strcmp("OP01", optr) == 0) {
        property_get("ro.vendor.mtk_world_phone_policy", prop_val, "0");
        op01_policy = atoi(prop_val);
        LOGD("[getWorldPhonePolicy] op01_policy=%d", op01_policy);
        if (op01_policy == 0) {
            policy = 1;
        } else {
            policy = 2;
        }
    } else if (strcmp("OP02", optr) == 0) {
        policy = 4;
    } else {
        policy = 3;
    }

    LOGD("[getWorldPhonePolicy] WorldPhonePolicy=%d", policy);

    return policy;
}

void setWorldPhonePolicy(RILChannelCtx *pChannel) {
    switch (getWorldPhonePolicy()) {
        case 1:
            at_send_command("AT+EWMPOLICY=1", NULL, pChannel);
            break;
        case 2:
            at_send_command("AT+EWMPOLICY=2", NULL, pChannel);
            break;
        case 3:
            at_send_command("AT+EWMPOLICY=3", NULL, pChannel);
            break;
        case 4:
            at_send_command("AT+EWMPOLICY=4", NULL, pChannel);
            break;
        default:
            RLOGD("WorldPhone Policy unknown!");
    }
}

int isWorldModeSwitching(const char *s) {
    int needIgnore = 0;
    if (bWorldModeSwitching) {
        if (strStartsWith(s, "+EUSIM:")) {
            RLOGD("WorldModeSwitching=true, don't ignore URC+EUSIM");
            needIgnore = 0;
        } else if (strStartsWith(s, "+ERSAIND:")){
            RLOGD("WorldModeSwitching=true, don't ignore URC+ERSAIND");
            needIgnore = 0;
        } else if (strStartsWith(s, "+STKPCI:")){
            RLOGD("WorldModeSwitching=true, don't ignore URC+STKPCI");
            needIgnore = 0;
        } else if (strStartsWith(s, "+ETESTSIM:")) {
            RLOGD("WorldModeSwitching=true, don't ignore URC+ETESTSIM");
            needIgnore = 0;
        } else if (strStartsWith(s, "+ECT3G:")) {
            RLOGD("WorldModeSwitching=true, don't ignore URC+ECT3G");
            needIgnore = 0;
        } else if (strStartsWith(s, "+EIND: 2") || strStartsWith(s, "+EIND: 32")) {
            RLOGD("WorldModeSwitching=true, don't ignore URC for PHB status");
            needIgnore = 0;
        } else if ( strStartsWith(s, "+ERPRAT:") ){
            RLOGD("WorldModeSwitching=true, don't ignore URC+ERPRAT");
            needIgnore = 0;
        } else if (strStartsWith(s, "+CGEV: NW PDN DEACT")) {
            RLOGD("WorldModeSwitching=true, don't ignore URC %s", s);
            needIgnore = 0;
        } else {
            needIgnore = 1;
        }
    }
    if ( strStartsWith(s, "+ECSRA:") ) {
            RLOGD("WorldModeSwitching=%d, don't ignore URC+ECSRA", bWorldModeSwitching);
            needIgnore = 0;
    }
    return needIgnore;
}

/// M: world mode switching, need pass special request. @{
int allowForWorldModeSwitching(const int request) {
    if (bWorldModeSwitching == 1) {
        if (request == RIL_REQUEST_RADIO_POWER) {
            RLOGD("WorldModeSwitching=true, don't ignore RIL_REQUEST_RADIO_POWER");
            return 1;
        } else if ((request == RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE) &&
                RatConfig_isC2kSupported()) {
            RLOGD("WorldModeSwitching=true, don't ignore RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE");
            return 1;
        } /* else if (request == RIL_REQUEST_SET_SVLTE_RAT_MODE) {
            RLOGD("WorldModeSwitching=true, don't ignore RIL_REQUEST_SET_SVLTE_RAT_MODE");
            return 1;
        } */
    }
    return 0;
}

/// M: workaround for world phone lib test @{
int isWorldPhoneSupport() {
  if (RatConfig_isWcdmaSupported() && RatConfig_isTdscdmaSupported()) {
    RLOGI("isWorldPhoneSupport() true");
    return 1;
  } else {
    return 0;
  }
}

void setActiveModemType(int mdType) {
    char activeModem[PROPERTY_VALUE_MAX] = {0};

    sprintf(activeModem, "%d", mdType);
    property_set("vendor.ril.active.md", activeModem);
    LOGD("[LteMSH][setActiveModemType] activeModem=%s", activeModem);

    return ;
}

int getActiveModemType() {
    int  activeModem = 0;
    char prop_val[PROPERTY_VALUE_MAX] = {0};

    property_get("vendor.ril.active.md", prop_val, "0");
    activeModem = atoi(prop_val);
    LOGD("[LteMSH][getActiveModemType] activeModem=%d", activeModem);

    return activeModem;
}

bool isWorldPhoneOldVersion() {
    char worldmode[PROPERTY_VALUE_MAX] = {0};
    int worldphone = isWorldPhoneSupport();
    property_get("ro.vendor.mtk_md_world_mode_support", worldmode, "0");
    LOGD("isWorldPhoneOldVersion(), worldphone = %d, worldmode = %s", worldphone, worldmode);
    if (1 == worldphone) {
        if (0 == strcmp(worldmode, "1")) {
            // support 91 world mode
        } else {
            // support 90 modem world phone
            return true;
        }
    }
    return false;
}

void requestStoreModem(void *data, size_t datalen, RIL_Token t) {
    WORLDPHONE_UNUSED(datalen);
    int modemType = (((int *)data)[0]) & 0x00FF;

    triggerCCCIIoctlEx(CCCI_IOC_STORE_MD_TYPE, &modemType);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    LOGD("requestStoreModem complete");

    return;
}

void requestReloadModem(void *data, size_t datalen, RIL_Token t) {
    WORLDPHONE_UNUSED(datalen);
    int modemType = ((int *)data)[0];
    if (true == isWorldPhoneOldVersion()) {
        if (modemType == 0 || modemType == 1 || modemType == 255) {
            char activeModem[PROPERTY_VALUE_MAX] = {0};
            sprintf(activeModem, "%d", modemType);
            property_set(PROPERTY_SWITCH_MODEM_CAUSE_TYPE, activeModem);
            RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
            LOGD("requestReloadModem complete: %d", modemType);
            return;
        } else {
            setActiveModemType(modemType);
        }
    }
    triggerCCCIIoctlEx(CCCI_IOC_RELOAD_MD_TYPE, &modemType);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    LOGD("requestReloadModem complete: %d", modemType);
    return;
}


void requestModifyModem(void *data, size_t datalen, RIL_Token t) {
    WORLDPHONE_UNUSED(datalen);
    int response[1] = {0};
    int applyType = ((int *)data)[0];
    int modemType = ((int *)data)[1];
    response[0] = applyType;
    if (applyType == WP_RELOAD_MODEM_TYPE) {
        if (true == isWorldPhoneOldVersion()) {
            if (modemType == 0 || modemType == 1 || modemType == 255) {
                char activeModem[PROPERTY_VALUE_MAX] = {0};
                sprintf(activeModem, "%d", modemType);
                property_set(PROPERTY_SWITCH_MODEM_CAUSE_TYPE, activeModem);
            } else {
                setActiveModemType(modemType);
            }
        }
        triggerCCCIIoctlEx(CCCI_IOC_RELOAD_MD_TYPE, &modemType);
    } else if (applyType == WP_STORE_MODEM_TYPE) {
        triggerCCCIIoctlEx(CCCI_IOC_STORE_MD_TYPE, &modemType);
    }
    LOGD("requestReloadModem complete: %d, %d", applyType, modemType);
    RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(int));
    return;
}
void onSwitchModemCauseInformation(char *urc, const RIL_SOCKET_ID rid) {
    WORLDPHONE_UNUSED(rid);
    int err = 0;
    err = at_tok_start(&urc);
    if (err < 0) {
        goto error;
    }

    // store delay information to property
    property_set(PROPERTY_SWITCH_MODEM_DELAY_INFO, urc);
    LOGD("onSwitchModemCauseInformation, delay information:%s", urc);
    return;

    error:
        LOGE("There is something wrong with the onSwitchModemCauseInformation URC");
}

static int worldPhoneGetSvlteProjectType() {
    int type = -1;

    if (RatConfig_isLteFddSupported() && RatConfig_isLteTddSupported()
            && RatConfig_isC2kSupported() && RatConfig_isWcdmaSupported()
            && RatConfig_isTdscdmaSupported()
            && RatConfig_isGsmSupported()) {
        type = WP_SVLTE_PROJ_DC_6M;
    } else if (RatConfig_isLteFddSupported()
            && RatConfig_isLteTddSupported() && RatConfig_isC2kSupported()
            && RatConfig_isWcdmaSupported() && RatConfig_isGsmSupported()) {
        type = WP_SVLTE_PROJ_DC_5M;
    } else if (RatConfig_isLteFddSupported()
            && RatConfig_isLteTddSupported() && RatConfig_isC2kSupported()
            && RatConfig_isGsmSupported()) {
        type = WP_SVLTE_PROJ_DC_4M;
    } else if (RatConfig_isC2kSupported() && RatConfig_isGsmSupported()
            && RatConfig_isWcdmaSupported()) {
        type = WP_SVLTE_PROJ_DC_3M;
    }
    RLOGI("getSvlteWwopType(): %d", type);

    return type;
}

static int worldPhoneIsSvlteLcgSupport() {
    int isSvlteLcgSupport = 0;

    if (RatConfig_isLteFddSupported() == 1 && RatConfig_isLteTddSupported() == 1
            && RatConfig_isC2kSupported() == 1 && RatConfig_isGsmSupported() ==1
            && RatConfig_isWcdmaSupported() == 0 && RatConfig_isTdscdmaSupported() == 0) {
        isSvlteLcgSupport = 1;
    }

    return isSvlteLcgSupport;
}

void worldPhoneinitialize(const RIL_SOCKET_ID rid, RILChannelCtx *p_channel) {
    int err;
    int mdType = -1;
    ATResponse *p_response = NULL;
    RILChannelCtx *pChannel = p_channel;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    /* check if modem support +ECSRA? */
    if (getMainProtocolRid() == rid) {
        err = at_send_command_singleline("AT+ECSRA?", "+ECSRA:", &p_response, pChannel);
        if (err < 0 || p_response->success == 0) {
            bUbinSupport = 0;
        }
        at_response_free(p_response);
    }

    if (isWorldPhoneSupport()) {
        // WorldMode(+ECSRA? is not support)
        if (true == isWorldPhoneOldVersion()) {
            char product_brand[PROPERTY_VALUE_MAX] = { 0 };
            char build_id[PROPERTY_VALUE_MAX] = { 0 };
            char build_desc[PROPERTY_VALUE_MAX] = { 0 };
            property_get("ro.product.brand", product_brand, "0");
            property_get("ro.build.display.id", build_id, "0");
            property_get("ro.build.description", build_desc, "0");
            LOGD("worldPhoneinitialize(), brand = %s, %s, %s", product_brand, build_id, build_desc);
            if (!(strcmp("Android", product_brand) == 0) &&
                    !(strcmp("android", product_brand) == 0) &&
                    !(strncmp("aosp", build_id, 4) == 0) &&
                    !(strncmp("aosp", build_desc, 4) == 0)) {
                at_send_command("AT+ECOPS=1", NULL, pChannel);
                RLOGD("AT+ECOPS=1 sent");
                at_send_command("AT+EMSR=0,1", NULL, pChannel);
                RLOGD("AT+EMSR=0,1 sent");

                // world phone optimization
                if (getMainProtocolRid() == rid) {
                    // cause type
                    property_get(PROPERTY_SWITCH_MODEM_CAUSE_TYPE,
                        property_value,
                        PROPERTY_SWITCH_MODEM_CAUSE_TYPE_DEFAULT_VALUE);
                    // delay info
                    char property_value2[PROPERTY_VALUE_MAX] = { 0 };
                    property_get(PROPERTY_SWITCH_MODEM_DELAY_INFO,
                        property_value2,
                        PROPERTY_SWITCH_MODEM_DELAY_INFO_DEFAULT_VALUE);
                    char *smciCmd = NULL;
                    asprintf(&smciCmd, "AT+ESMCI=%s,%s", property_value, property_value2);
                    if (smciCmd != NULL) {
                        at_send_command(smciCmd, NULL, pChannel);
                        RLOGD("%s sent", smciCmd);
                        free(smciCmd);
                    }
                    // set default value
                    property_set(PROPERTY_SWITCH_MODEM_CAUSE_TYPE,
                        PROPERTY_SWITCH_MODEM_CAUSE_TYPE_DEFAULT_VALUE);
                    property_set(PROPERTY_SWITCH_MODEM_DELAY_INFO,
                        PROPERTY_SWITCH_MODEM_DELAY_INFO_DEFAULT_VALUE);
                }
            }  // strcmp alps
        } else {
            at_send_command("AT+ECSRA=1", NULL, pChannel);
            RLOGD("AT+ECSRA=1 sent");
            setWorldPhonePolicy(pChannel);
        }
    } else {
        // 4M & 5M setWorldPhonePolicy.
        int svlteProjectType = worldPhoneGetSvlteProjectType();
        if (worldPhoneIsSvlteLcgSupport() || svlteProjectType == WP_SVLTE_PROJ_DC_5M ||
                svlteProjectType == WP_SVLTE_PROJ_SC_5M) {
            setWorldPhonePolicy(pChannel);
        }
    }
    // notify world mode changed done
    char worldModeState[PROPERTY_VALUE_MAX];
    property_get("persist.vendor.radio.wm_state", worldModeState, "1");
    LOGD("World Mode State = %s", worldModeState);
    if (strcmp("0", worldModeState) == 0) {
        int status[1] = {2};
        LOGD("[handleWorldModeChanged] state = %d", status[0]);
        RIL_UNSOL_RESPONSE (RIL_UNSOL_WORLD_MODE_CHANGED, status, sizeof(status), rid);
        property_set("persist.vendor.radio.wm_state", "1");
    }

    // get active mode
    if (getMainProtocolRid() == rid) {
        triggerCCCIIoctlEx(CCCI_IOC_GET_MD_TYPE, &mdType);
        setActiveModemType(mdType);
        LOGD("[WPO][initCallback] Set vendor.ril.active.md = %d %d", mdType, rid);
        if (bUbinSupport) {
            queryActiveMode(rid, pChannel);
        }
    }
}

void queryKeep3GMode(RIL_SOCKET_ID rid, RILChannelCtx *p_channel) {
    ATResponse *p_response = NULL;
    int err;
    char *cmd = NULL;
    char* line;
    int mode = 0;

    asprintf(&cmd, "AT+ESBP=7,\"SBP_GEMINI_LG_WG_MODE\"");
    err = at_send_command_singleline(cmd, "+ESBP:", &p_response, getChannelCtxbyProxy());
    if (err == 0 && p_response->success != 0 && p_response->p_intermediates != NULL) {
        line = p_response->p_intermediates->line;
        err = at_tok_start(&line);
        if (err >= 0) {
            err = at_tok_nextint(&line, &mode);
            LOGD("keep 3G mode mode = %d", mode);
            if (0 == mode) {
                property_set("vendor.ril.nw.worldmode.keep_3g_mode", "0");
                LOGD("keep TD-SCDMA 3G mode mode");
            } else if (1 == mode) {
                property_set("vendor.ril.nw.worldmode.keep_3g_mode", "1");
                LOGD("keep WCDMA 3G mode mode");
            } else {
                LOGD("AT+ESBP=7,\"SBP_GEMINI_LG_WG_MODE\", mode value error : %d", mode);
            }
        } else {
            LOGD("AT+ESBP=7 response value error: %d", err);
        }
    } else {
        LOGD("Not support SBP_GEMINI_LG_WG_MODE, error info: %d, %d", err, p_response->success);
        property_set("vendor.ril.nw.worldmode.keep_3g_mode", "-1");
    }
    free(cmd);
    at_response_free(p_response);
}

/* ALPS03212986 check if world phone has set radio state or if world phone has handled in sim channel*/
int checkWorldPhoneHadSetRadioState() {
    LOGD("checkWorldPhoneHadSetRadioState %d %d", bWorldModeSetRadioState, bWorldModeSwitching);
    if (-1 == bWorldModeSetRadioState) {
        return bWorldModeSwitching;
    }
    return bWorldModeSetRadioState;
}
