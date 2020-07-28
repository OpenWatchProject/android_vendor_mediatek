/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include "RmcWpRequestHandler.h"
#include "RmcWpURCHandler.h"
#include <telephony/mtk_ril.h>
#include "RfxLog.h"
#include "rfx_properties.h"
#include "RfxRilUtils.h"
#include <ratconfig.h>

#define WP_LOG_TAG "RmcWp"
// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcWpRequestHandler, RIL_CMD_PROXY_1);

int RmcWpRequestHandler::bWorldModeSwitching = 0;
int RmcWpRequestHandler::ecsraUrcParams[5] = {0};
RIL_RadioState RmcWpRequestHandler::radioStateBackup[MAX_SIM_COUNT] ={RADIO_STATE_OFF};

RmcWpRequestHandler::RmcWpRequestHandler(int slot_id, int channel_id):RfxBaseHandler(slot_id, channel_id) {
    int majorSIM = RfxRilUtils::getMajorSim();
    const int request[] = {
        RFX_MSG_REQUEST_WORLD_MODE_RESUMING,
    };

    registerToHandleRequest(request, sizeof(request)/sizeof(int));
    if (slot_id == (majorSIM - 1)) {
        setActiveModemType();
        queryActiveMode();
    }
    worldPhoneInitialize(slot_id);
}

RmcWpRequestHandler::~RmcWpRequestHandler() {
}

void RmcWpRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    logD(WP_LOG_TAG, "onHandleRequest: %d", msg->getId());
    int request = msg->getId();
    switch(request) {
        case RFX_MSG_REQUEST_WORLD_MODE_RESUMING:
            requestResumingWorldMode(msg);
            break;
        default:
            assert(0);
            break;
    }
}

void RmcWpRequestHandler::requestResumingWorldMode(const sp<RfxMclMessage>& msg) {
    for (int i = 0; i < 5; i++) {
        ecsraUrcParams[i] = RmcWpURCHandler::getEcsraUrcParam(i);
    }
    handleWorldModeChanged(msg);
    return;
}

void RmcWpRequestHandler::handleWorldModeChanged(const sp<RfxMclMessage>& msg) {
    int i;
    int err;
    int reterr[2] = {0};
    const int beforeRestoreRadio = 2;
    const int switchingDone = 0;
    sp<RfxAtResponse> p_response;
    RIL_Errno err_no = RIL_E_SUCCESS;

    // switching start
    bWorldModeSwitching = 1;
    rfx_property_set("persist.vendor.radio.wm_state", "0");

    // return the latest reported +ECSRA URC
    logD(WP_LOG_TAG, "AT+ECSRA=2,%d,%d,%d,%d,%d",
            ecsraUrcParams[0], ecsraUrcParams[1], ecsraUrcParams[2], ecsraUrcParams[3], ecsraUrcParams[4]);
    p_response = atSendCommand(String8::format("AT+ECSRA=2,%d,%d,%d,%d,%d",
            ecsraUrcParams[0], ecsraUrcParams[1], ecsraUrcParams[2], ecsraUrcParams[3], ecsraUrcParams[4]));

    // re-read world mode after world mode switch
    setActiveModemType();
    // switch success end
    bWorldModeSwitching = 0;
    rfx_property_set("persist.vendor.radio.wm_state", "1");

    // check error
    err = p_response->getError();
    logD(WP_LOG_TAG, "handleWorldModeChanged AT+ECSRA=2 err = %d, success = %d", err, p_response->getSuccess());
    if (err != 0 || p_response->getSuccess() != 1) {
        err_no = RIL_E_GENERIC_FAILURE;
        reterr[0] = err;
    }

    // requery duplex mode
    reterr[1] = queryActiveMode();

    // response request
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(),
            err_no, RfxIntsData(reterr, 2), msg, false);
    // response to TeleCore
    responseToTelCore(response);
    return;
}

int RmcWpRequestHandler::isWorldModeSwitching() {
    if (1 == bWorldModeSwitching) {
        //switching
        return 1;
    }
    //no switching
    return 0;
}

void RmcWpRequestHandler::onWorldModeStateChanged(int state) {
    if (1 == state) {
        logD(WP_LOG_TAG, "[onWorldModeStateChanged]-start");
    } else if (0 == state) {
        logD(WP_LOG_TAG, "[onWorldModeStateChanged]-end");
    } else if (2 == state) {
        logD(WP_LOG_TAG, "[onWorldModeStateChanged]-before_available");
    }
}

void RmcWpRequestHandler::setActiveModemType() {
    int mdType = -1;
    char activeModem[RFX_PROPERTY_VALUE_MAX] = {0};

    RfxRilUtils::triggerCCCIIoctlEx(CCCI_IOC_GET_MD_TYPE, &mdType);
    sprintf(activeModem, "%d", mdType);
    rfx_property_set("vendor.ril.active.md", activeModem);
    logD(WP_LOG_TAG, "[setActiveModemType] activeModem = %s", activeModem);

    return;
}

int RmcWpRequestHandler::getActiveModemType() {
    int activeModem = 0;
    char prop_val[RFX_PROPERTY_VALUE_MAX] = {0};

    rfx_property_get("vendor.ril.active.md", prop_val, "0");
    activeModem = atoi(prop_val);
    logD(WP_LOG_TAG, "[getActiveModemType] activeModem = %d", activeModem);

    return activeModem;
}

int RmcWpRequestHandler::queryActiveMode() {
    sp<RfxAtResponse> p_response;
    int err;
    char* activeMode = NULL;
    int csraaResponse[3] = {0};
    int mode = 0;

    p_response = atSendCommandSingleline("AT+CSRA?", "+CSRAA:");
    err = p_response->getError();
    if (err == 0 && p_response->getSuccess() != 0 && p_response->getIntermediates() != NULL) {
        RfxAtLine* line = p_response->getIntermediates();
        mode = 0;
        /*+CSRAA:<GERAN-TDMA>,<UTRANFDD>,<UTRAN-TDD-LCR>,<UTRAN-TDD-HCR>,<UTRANTDD-VHCR>,<E-UTRAN-FDD>,<E-UTRAN-TDD>*/
        // go to start position
        line->atTokStart(&err);
        if (err >= 0) {
            for (int i = 0; i <= 2; i++) {
                csraaResponse[i] = line->atTokNextint(&err);
                if (err < 0) {
                    logD(WP_LOG_TAG, "queryActiveMode CSRA? fail %d %d %d", err, i, csraaResponse[i]);
                    break;
                }
            }
        }

        if ((csraaResponse[1] == 1) && (csraaResponse[2] == 0)) {
            // FDD mode
            mode = 1;
        } else if ((csraaResponse[1] == 0) && (csraaResponse[2] == 1)) {
            // TDD mode
            mode = 2;
        }
        asprintf(&activeMode, "%d", mode);
        logD(WP_LOG_TAG, "+CSRAA:<UTRANFDD>=%d, <UTRAN-TDD-LCR>=%d, update ril.nw.worldmode.active.mode to %s",
                csraaResponse[1], csraaResponse[2], activeMode);
        rfx_property_set("vendor.ril.nw.worldmode.activemode", activeMode);

        if (NULL != activeMode) {
            free(activeMode);
            activeMode = NULL;
        }
    }
    return mode;
}

WorldPhonePolicyType RmcWpRequestHandler::getWorldPhonePolicy() {
    WorldPhonePolicyType policy = WORLD_PHONE_POLYCY_OM;
    int op01_policy = 0;
    char optr[RFX_PROPERTY_VALUE_MAX] = {0};
    char prop_val[RFX_PROPERTY_VALUE_MAX] = {0};

    rfx_property_get("persist.vendor.operator.optr", optr, "0");
    if (strcmp("OP01", optr) == 0) {
        rfx_property_get("ro.vendor.mtk_world_phone_policy", prop_val, "0");
        op01_policy = atoi(prop_val);
        logD(WP_LOG_TAG, "[getWorldPhonePolicy] op01_policy=%d", op01_policy);
        if (op01_policy == 0) {
            policy = WORLD_PHONE_POLYCY_OP01A;
        } else {
            policy = WORLD_PHONE_POLYCY_OP01B;
        }
    } else if (strcmp("OP02", optr) == 0) {
        policy = WORLD_PHONE_POLYCY_OP02;
    } else {
        policy = WORLD_PHONE_POLYCY_OM;
    }
    logD(WP_LOG_TAG, "[getWorldPhonePolicy] WorldPhonePolicy=%d", policy);
    return policy;
}

void RmcWpRequestHandler::setWorldPhonePolicy() {
    switch (getWorldPhonePolicy()) {
        case WORLD_PHONE_POLYCY_OP01A:
            atSendCommand("AT+EWMPOLICY=1");
            break;
        case WORLD_PHONE_POLYCY_OP01B:
            atSendCommand("AT+EWMPOLICY=2");
            break;
        case WORLD_PHONE_POLYCY_OM:
            atSendCommand("AT+EWMPOLICY=3");
            break;
        case WORLD_PHONE_POLYCY_OP02:
            atSendCommand("AT+EWMPOLICY=4");
            break;
        default:
            logD(WP_LOG_TAG, "WorldPhone Policy unknown!");
    }
}

int RmcWpRequestHandler::isWorldPhoneSupport() {
    if (RatConfig_isWcdmaSupported() && RatConfig_isTdscdmaSupported()) {
        logD(WP_LOG_TAG, "isWorldPhoneSupport() true");
        return 1;
    } else {
        logD(WP_LOG_TAG, "isWorldPhoneSupport() false");
        return 0;
    }
}

int RmcWpRequestHandler::isSvlteLcgSupport() {
    int isSvlteLcgSupport = 0;
    if (RatConfig_isLteFddSupported() == 1 && RatConfig_isLteTddSupported() == 1
            && RatConfig_isC2kSupported() == 1 && RatConfig_isGsmSupported() ==1
            && RatConfig_isWcdmaSupported() == 0 && RatConfig_isTdscdmaSupported() == 0) {
        isSvlteLcgSupport = 1;
    }
    return isSvlteLcgSupport;
}

SVLTE_PROJ_TYPE RmcWpRequestHandler::getSvlteProjectType() {
    SVLTE_PROJ_TYPE type = SVLTE_PROJ_INVALID;

    if (RatConfig_isLteFddSupported() && RatConfig_isLteTddSupported()
            && RatConfig_isC2kSupported() && RatConfig_isWcdmaSupported()
            && RatConfig_isTdscdmaSupported()
            && RatConfig_isGsmSupported()) {
        type = SVLTE_PROJ_DC_6M;
    } else if (RatConfig_isLteFddSupported()
            && RatConfig_isLteTddSupported() && RatConfig_isC2kSupported()
            && RatConfig_isWcdmaSupported() && RatConfig_isGsmSupported()) {
        type = SVLTE_PROJ_DC_5M;
    } else if (RatConfig_isLteFddSupported()
            && RatConfig_isLteTddSupported() && RatConfig_isC2kSupported()
            && RatConfig_isGsmSupported()) {
        type = SVLTE_PROJ_DC_4M;
    } else if (RatConfig_isC2kSupported() && RatConfig_isGsmSupported()
            && RatConfig_isWcdmaSupported()) {
        type = SVLTE_PROJ_DC_3M;
    }

    logD(WP_LOG_TAG, "getSvlteWwopType(): %d", type);
    return type;
}

void RmcWpRequestHandler::worldPhoneInitialize(int slot_id) {
    if (isWorldPhoneSupport()) {
        atSendCommand("AT+ECSRA=1");
        logD(WP_LOG_TAG, "AT+ECSRA=1 sent");
        setWorldPhonePolicy();
    } else {
        // 4M & 5M setWorldPhonePolicy.
        SVLTE_PROJ_TYPE svlteProjectType = getSvlteProjectType();
        if (isSvlteLcgSupport() || svlteProjectType == SVLTE_PROJ_DC_5M || svlteProjectType == SVLTE_PROJ_SC_5M) {
            setWorldPhonePolicy();
        }
    }
    // notify world mode change done
    char worldModeState[RFX_PROPERTY_VALUE_MAX] = {0};
    rfx_property_get("persist.vendor.radio.wm_state", worldModeState, "1");
    logD(WP_LOG_TAG, "World Mode State = %s", worldModeState);
    if (strcmp("0", worldModeState) == 0) {
        int status[2] = {1, -1};
        logD(WP_LOG_TAG, "[worldPhoneInitialize] status[0] = %d, slot_id = %d", status[0], slot_id);
        // inform telcore to world mode change done
        sp<RfxMclMessage> urc = RfxMclMessage::obtainUrc(
                RFX_MSG_URC_WORLD_MODE_CHANGED, slot_id, RfxIntsData(status, 2));
        responseToTelCore(urc);
        rfx_property_set("persist.vendor.radio.wm_state", "1");
    }
}
