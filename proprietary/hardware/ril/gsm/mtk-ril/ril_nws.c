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
#include <libmtkrilutils.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "at_tok.h"
#include "atchannels.h"
#include <ril_callbacks.h>
#include <ril_nws.h>
#include <log/log.h>
#include <cutils/properties.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#ifdef MTK_RIL_MD1
#define LOG_TAG "RIL-NWS"
#else
#define LOG_TAG "RILMD2-NWS"
#endif

extern void setRadioState(RIL_RadioState newState, RIL_SOCKET_ID rid);
extern bool isWorldPhoneOldVersion();

int g_regSuspendEnabled[RIL_SOCKET_NUM] = {0};
int g_ectMode[RIL_SOCKET_NUM] = {0};
int g_volteState[RIL_SOCKET_NUM] = {0};

void setEctMode(RIL_SOCKET_ID rid, int ectMode) {
    LOGD("setEctMode(), rid=%d, ectmode=%d", rid, ectMode);
    g_ectMode[rid] = ectMode;
}

int getEctMode(RIL_SOCKET_ID rid) {
    return g_ectMode[rid];
}

void onGmssRat(char* urc, const RIL_SOCKET_ID rid) {
    /* +EGMSS:<rat>,<mcc>,<status>[,<mspl>,<home>[,<is_srlte>]]
     * <rat>
     *     0: Any RAT in 3GPP2 RAT group
     *     1: Any RAT in 3GPP RAT group
     *     2: CDMA2000 1x
     *     3: CDMA2000 HRPD
     *     4: GERAN
     *     5: UTRAN
     *     6: EUTRAN
     * <mcc>
     *     460: CT
     * <status>
     *     0: selected
     *     1: attached
     *     2: ECC retry
     * <mspl>
     *     Combination  of LWCG
     * <home>
     *     0: unknow
     *     1: home/ehome
     * <is_srlte>
     *     0: not srlte
     *     1: srlte
     */
    int err = 0;
    int rat = 0;
    int mcc = 0;
    int attached = 0;
    int mspl = -1;
    int is_home = -1;
    int is_srlte = -1;
    char * mcc_str = NULL;
    err = at_tok_start(&urc);
    if (err < 0) return;
    err = at_tok_nextint(&urc, &rat);
    if (err < 0) {
        LOGD("onGmssRat has error rat");
        return;
    }
    err = at_tok_nextstr(&urc, &mcc_str);
    if (err < 0) {
        LOGD("onGmssRat has error mcc");
        return;
    }
    err = at_tok_nextint(&urc, &attached);
    if (err < 0) {
        LOGD("onGmssRat has error attached");
        return;
    }
    err = at_tok_nextint(&urc, &mspl);
    if (err < 0) {
        LOGD("onGmssRat no mspl");
    }
    err = at_tok_nextint(&urc, &is_home);
    if (err < 0) {
        LOGD("onGmssRat no is_home");
    }
    err = at_tok_nextint(&urc, &is_srlte);
    if (err < 0) {
        LOGD("onGmssRat no is_srlte");
    }
    if (mcc_str != NULL) {
        mcc = atoi(mcc_str);
    }

    const int count = 6;
    int response[count] = {0};
    response[0] = rat;
    response[1] = mcc;
    response[2] = attached;
    response[3] = mspl;
    response[4] = is_home;
    response[5] = is_srlte;
    LOGD("onGmssRat, rat:%d, mcc:%d, att:%d, mspl:%d, ish:%d, is_srlte:%d",
            rat, mcc, attached, mspl, is_home, is_srlte);
    RIL_UNSOL_RESPONSE(RIL_UNSOL_GMSS_RAT_CHANGED, response, sizeof(int) * count, rid);
}

void resetRegSuspend(void * data, size_t datalen, RIL_Token t) {
    char * cmd1 = NULL;
    char * cmd2 = NULL;
    int err;
    ATResponse *p_response = NULL;
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    asprintf(&cmd1, "AT+EMSR=0,0");
    err = at_send_command(cmd1, &p_response, p_channel);
    free(cmd1);

    if (err < 0 || p_response->success == 0) {
        LOGD("onResetRegSuspend(), send AT+EMSR=0,0 return error.");
        goto error;
    } else {
        LOGD("onResetRegSuspend, reset Reg Suspend flag for index %d", rid);
        g_regSuspendEnabled[rid] = 0;
        at_response_free(p_response);
        p_response = NULL;
        asprintf(&cmd2, "AT+ECOPS=0");
        err = at_send_command(cmd2, &p_response, p_channel);
        free(cmd2);

        if (err < 0 || p_response->success == 0) {
            LOGD("onResetRegSuspend(), AT+ECOPS=0 return error.");
            goto error;
        }
    }
    RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    at_response_free(p_response);
    p_response = NULL;
    return;

error:
    RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);
    p_response = NULL;
    return;
}

void requestSetRegSuspendEnabled(void * data, size_t datalen, RIL_Token t) {
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(getRILChannelCtxFromToken(t));
    g_regSuspendEnabled[rid] = ((int *)data)[0];
    if (datalen / sizeof(int32_t) == 2) {
        g_volteState[rid] = ((int *)data)[1];
    }
    setRegSuspend(t);
}

void requestResumeRegistration(void * data, size_t datalen, RIL_Token t) {
    RIL_NWS_UNUSED_PARM(datalen);
    char * cmd = NULL;
    ATResponse *p_response = NULL;
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    int err;

    int sessionId = ((int *)data)[0];

    asprintf(&cmd, "AT+EMSR=1, %d", sessionId);
    err = at_send_command(cmd, &p_response, p_channel);
    free(cmd);

    if (err < 0 || p_response->success == 0) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    } else {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
}

void setRegSuspend(RIL_Token t) {
    setRegSuspendFunction(t, true);
}

void setRegSuspendFunction(RIL_Token t, bool req_complete) {
    char * cmd1 = NULL;
    char * cmd2 = NULL;
    ATResponse *p_response = NULL;
    int err;
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rid = getRILIdByChannelCtx(p_channel);

    LOGD("setRegSuspendFunction(), g_regSuspendEnabled[%d] = %d", rid, g_regSuspendEnabled[rid]);
    if (g_regSuspendEnabled[rid] == 0) {
        int beWorldPhoneOldVersion = isWorldPhoneOldVersion() ? 1 : 0;
        LOGD("setRegSuspendFunction(), isWorldPhoneOldVersion = %d", beWorldPhoneOldVersion);
        if (beWorldPhoneOldVersion == 1) {
            LOGD("setRegSuspendFunction(), world phone old version, do not turn of EMSR and ECOPS");
            return;
        }
        asprintf(&cmd1, "AT+EMSR=0,0");
        asprintf(&cmd2, "AT+ECOPS=0");
    } else {
        asprintf(&cmd1, "AT+ECOPS=%d", g_regSuspendEnabled[rid]);
        asprintf(&cmd2, "AT+EMSR=0,%d", g_regSuspendEnabled[rid]);
    }

    err = at_send_command(cmd1, &p_response, p_channel);
    free(cmd1);

    if (err < 0 || p_response->success == 0) {
        free(cmd2);
        goto error;
    } else {
        at_response_free(p_response);
        p_response = NULL;

        err = at_send_command(cmd2, &p_response, p_channel);
        free(cmd2);

        if (err < 0 || p_response->success == 0) {
            goto error;
        }
    }

    if (req_complete) {
        RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
    }
    at_response_free(p_response);
    p_response = NULL;
    return;

error:
    LOGE("setRegSuspendFunction error");
    if (req_complete) {
        RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    }
    at_response_free(p_response);
    p_response = NULL;
    return;
}

void onPlmnListChanged(char *urc, const RIL_SOCKET_ID rid) {
    int err = 0, i, plmnCount;
    char** response = NULL;

    err = at_tok_start(&urc);
    if (err < 0) goto error;

    err = at_tok_nextint(&urc, &plmnCount);
    if (err < 0) goto error;

    response = (char **) calloc(1, sizeof(char*) * plmnCount);
    assert(response != NULL);

    for (i = 0; i < plmnCount; i++) {
        err = at_tok_nextstr(&urc, &response[i]);
        if (err < 0) goto error;
    }


    LOGD("onPlmnListChanged= %d, %s", plmnCount, response[0]);

    RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_PLMN_CHANGED,
            response, sizeof(char*) * plmnCount, rid);
    free(response);
    return;

error:
    LOGE("There is something wrong with the onPlmnListChanged URC");
    if (response != NULL)
        free(response);
}

void onRegistrationSuspended(char *urc, const RIL_SOCKET_ID rid) {
    int err = 0;
    int sessionId[1] = {0};

    err = at_tok_start(&urc);
    if (err < 0) goto error;

    err = at_tok_nextint(&urc, &sessionId[0]);
    if (err < 0) goto error;

    LOGD("onRegistrationSuspended= %d", sessionId[0]);

    RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_REGISTRATION_SUSPENDED,
            sessionId, sizeof(sessionId), rid);
    return;

error:
    LOGE("There is something wrong with the onRegistrationSuspended URC");
}

void setBootUpMode(RIL_Token t) {
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    LOGD("setBootUpMode(), send command AT+EBOOT=1.");
    at_send_command("AT+EBOOT=1", NULL, p_channel);
}

void resetRadioForRoaming(RIL_Token t) {
    char* cmd = NULL;
    ATResponse *p_response = NULL;
    RILChannelCtx* p_channel = getRILChannelCtxFromToken(t);
    RIL_SOCKET_ID rsId = getRILIdByChannelCtx(p_channel);
    int err = 0;

    // Reset the radio.
    bool bSlot0Off = RADIO_STATE_OFF == getRadioState(RIL_SOCKET_1);
    int nSlot0Off = bSlot0Off ? 1 : 0;
    bool bSlot1Off = RADIO_STATE_OFF == getRadioState(RIL_SOCKET_2);
    int nSlot1Off = bSlot1Off ? 1 : 0;
    LOGD("requestSetSvlteRatMode(), slot 0 off = %d, slot 1 off = %d.", nSlot0Off, nSlot1Off);

    if (getMainProtocolRid() == RIL_SOCKET_1) {
        LOGD("requestSetSvlteRatMode(), MainProtocolRid = 0, rsId = %d.", rsId);
        if (rsId == RIL_SOCKET_1) {
            if (bSlot1Off) {
                err = asprintf(&cmd, "AT+EFUN=0");
                err = at_send_command(cmd, &p_response, p_channel);
                setRadioState(RADIO_STATE_OFF, RIL_SOCKET_1);
                setRadioState(RADIO_STATE_OFF, RIL_SOCKET_2);
            } else {
                err = asprintf(&cmd, "AT+EFUN=2");
                err = at_send_command(cmd, &p_response, p_channel);
                setRadioState(RADIO_STATE_OFF, RIL_SOCKET_1);
            }
            LOGD("requestSetSvlteRatMode(), send command %s.", cmd);
        } else {
            if (bSlot0Off) {
                err = asprintf(&cmd, "AT+EFUN=0");
                err = at_send_command(cmd, &p_response, p_channel);
                setRadioState(RADIO_STATE_OFF, RIL_SOCKET_1);
                setRadioState(RADIO_STATE_OFF, RIL_SOCKET_2);
            } else {
                err = asprintf(&cmd, "AT+EFUN=1");
                err = at_send_command(cmd, &p_response, p_channel);
                setRadioState(RADIO_STATE_OFF, RIL_SOCKET_2);
            }
            LOGD("requestSetSvlteRatMode(), send command %s.", cmd);
        }
    } else if (getMainProtocolRid() == RIL_SOCKET_2) {
        LOGD("requestSetSvlteRatMode(), MainProtocolRid = 1, rsId = %d.", rsId);
        if (rsId == RIL_SOCKET_1) {
            if (bSlot1Off) {
                err = asprintf(&cmd, "AT+EFUN=0");
                err = at_send_command(cmd, &p_response, p_channel);
                setRadioState(RADIO_STATE_OFF, RIL_SOCKET_1);
                setRadioState(RADIO_STATE_OFF, RIL_SOCKET_2);
            } else {
                err = asprintf(&cmd, "AT+EFUN=1");
                err = at_send_command(cmd, &p_response, p_channel);
                setRadioState(RADIO_STATE_OFF, RIL_SOCKET_1);
            }
            LOGD("requestSetSvlteRatMode(), send command %s.", cmd);
        } else {
            if (bSlot0Off) {
                err = asprintf(&cmd, "AT+EFUN=0");
                err = at_send_command(cmd, &p_response, p_channel);
                setRadioState(RADIO_STATE_OFF, RIL_SOCKET_1);
                setRadioState(RADIO_STATE_OFF, RIL_SOCKET_2);
            } else {
                err = asprintf(&cmd, "AT+EFUN=2");
                err = at_send_command(cmd, &p_response, p_channel);
                setRadioState(RADIO_STATE_OFF, RIL_SOCKET_2);
            }
            LOGD("requestSetSvlteRatMode(), send command %s.", cmd);
        }
    } else {
        LOGW("requestSetSvlteRatMode(), invalid main protocol rid.");
        err = asprintf(&cmd, "AT+EFUN=0");
        err = at_send_command(cmd, &p_response, p_channel);
        setRadioState(RADIO_STATE_OFF, RIL_SOCKET_1);
        setRadioState(RADIO_STATE_OFF, RIL_SOCKET_2);
    }

    free(cmd);
    at_response_free(p_response);
}

int isCtDualModeCardInGsmOnlyMode(RIL_SOCKET_ID rid) {
    char ect3gProp[PROPERTY_VALUE_MAX] = {0};
    char uiccProp[PROPERTY_VALUE_MAX] = {0};
    property_get(PROPERTY_RIL_CT3G[rid], ect3gProp, "");
    property_get(PROPERTY_RIL_UICC_TYPE[rid], uiccProp, "");
    RLOGD("isCtDualModeCardInGsmOnlyMode: rid %d, ECT3G %s, uicc type: %s", rid, ect3gProp, uiccProp);
    if ((strcmp("1", ect3gProp) == 0) && ((strcmp("CSIM", uiccProp) != 0) && (strcmp("RUIM", uiccProp) != 0))) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * M: [C2K][IR] To check if it is necessary to send AT+CFUN=0/AT+ESUIMS for switching CT 3G dual mode card.
 * AT+ESUIMS will cause MD to reset SIM, and finally trigger SvlteRatController to config rat mode again,
 * we need to break the cycle.
 */
int needModeSwitchForCtDualModeCard(RIL_SOCKET_ID rid, int roamingMode) {
    if (roamingMode == ROAMING_MODE_NORMAL_ROAMING && isCtDualModeCardInGsmOnlyMode(rid)) {
        return 0;
    } else if (roamingMode != ROAMING_MODE_NORMAL_ROAMING && !isCtDualModeCardInGsmOnlyMode(rid)) {
        return 0;
    } else {
        return 1;
    }
}

