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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RmcDcMiscImpl.h"
#include "RmcDataDefs.h"
#include "RmcCapabilitySwitchUtil.h"
#include "RfxRilUtils.h"

#define RFX_LOG_TAG "RmcDcMiscImpl"

/*****************************************************************************
 * Class RmcDcMiscImpl
 * Send AT Command through this class. Created by RmcDcMiscHandler.
 *****************************************************************************/

RmcDcMiscImpl::RmcDcMiscImpl(RfxBaseHandler* handler, int slotId) :
        mHandler(handler), mSlotId(slotId) {
}

RmcDcMiscImpl::~RmcDcMiscImpl() {
}

/**
 * Method for initialization module.
 */
void RmcDcMiscImpl::init() {
    if (isFastDormancySupport() == 1) {
        initializeFastDormancy();
    } else {
        RFX_LOG_I(RFX_LOG_TAG, "Fast dormancy is configured disabled");
        atSendCommand(String8::format("AT+EFD=0"));
    }
    syncEpdgConfigToMd();
    syncIwlanOperationModeToMd();
}

void RmcDcMiscImpl::syncEpdgConfigToMd() {
    int wfcSupport = 0;
    if (RfxRilUtils::isWfcSupport()) {
        wfcSupport = 1;
    }
    mHandler->atSendCommand(String8::format("AT+EEPDG=%d", wfcSupport));
}

void RmcDcMiscImpl::syncIwlanOperationModeToMd() {
    sp<RfxAtResponse> responseFromModem;

    // for Inter Wireless LAN Priority List Enable
    // AT+EIWLPLEN = <on/off>, 1:on, 0:off
    responseFromModem = mHandler->atSendCommand(String8::format("AT+EIWLPLEN =%d", 1));
    if (responseFromModem != NULL && !responseFromModem->isAtResponseFail()) {
        RFX_LOG_D(RFX_LOG_TAG, "[%s] enable IWLAN Priority List update", __FUNCTION__);
    } else {
        int error = responseFromModem->atGetCmeError();
        RFX_LOG_E(RFX_LOG_TAG, "[%s] fail to enable IWLAN Priority List update! err=%d",
                __FUNCTION__, error);
    }
}

void RmcDcMiscImpl::requestStartLce(const sp<RfxMclMessage>& msg) {
    RfxAtLine *pLine = NULL;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> responseMsg;
    int *pInt = (int *)msg->getData()->getData();
    int desiredInterval = pInt[0]; // The desired reporting interval in ms.
    int lceMode = pInt[1] + 1; // LCE mode : start with PUSH mode = 1; start with PULL mode = 2.
    char lceStatus = 0xFF; // LCE status : not supported = 0xFF; stopped = 0; active = 1.
    unsigned int actualInterval = 0; //The actual reporting interval in ms.
    int err = 0;

    /* Initialize LCE status response */
    RIL_LceStatusInfo* response = (RIL_LceStatusInfo*)calloc(1, sizeof(RIL_LceStatusInfo));
    RFX_ASSERT(response != NULL);

    /* Use AT+ELCE=<lce_mode>[,<desired_interval>] to start LCE service */
    p_response = atSendCommandSingleline(String8::format("AT+ELCE=%d,%u",
            lceMode, desiredInterval), "+ELCE:");

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response!",
                mSlotId, __FUNCTION__);
        goto error;
    }

    if (p_response->isAtResponseFail()) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+ELCE=%d,%u responce ERROR:%d",
                mSlotId, __FUNCTION__, lceMode, desiredInterval, p_response->atGetCmeError());
        goto error;
    }

    /* pLine => +ELCE:<lce_status>,<actual_interval> */
    pLine = p_response->getIntermediates();

    if (pLine == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get intermediate results!",
                mSlotId, __FUNCTION__);
        goto error;
    }

    pLine->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start", mSlotId, __FUNCTION__);
        goto error;
    }

    /* Get 1st parameter: lceStatus */
    lceStatus = (char)pLine->atTokNextint(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing lce status",
                mSlotId, __FUNCTION__);
        goto error;
    }

    /* Get 2nd parameter: actualInterval */
    if (pLine->atTokHasmore()) {
        actualInterval = pLine->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing actual interval",
                    mSlotId, __FUNCTION__);
            goto error;
        }
    }

done:
    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] lceStatus=%d, actualInterval=%u",
            mSlotId, __FUNCTION__, lceStatus, actualInterval);
    response->lce_status = lceStatus;
    response->actual_interval_ms = actualInterval;
    responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS,
            RfxLceStatusResponseData(response, sizeof(RIL_LceStatusInfo)), msg);
    responseToTelCore(responseMsg);
    FREEIF(response);
    return;

error:
    responseMsg = RfxMclMessage::obtainResponse(RIL_E_GENERIC_FAILURE, RfxVoidData(), msg);
    responseToTelCore(responseMsg);
    FREEIF(response);
}

void RmcDcMiscImpl::requestStopLce(const sp<RfxMclMessage>& msg) {
    RFX_UNUSED(msg);
    RfxAtLine *pLine = NULL;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> responseMsg;
    char lceStatus = 0xFF; // LCE status : not supported = 0xFF; stopped = 0; active = 1.
    unsigned int actualInterval = 0; //The actual reporting interval in ms.
    int err = 0;

    /* Initialize LCE status response */
    RIL_LceStatusInfo* response = (RIL_LceStatusInfo*)calloc(1, sizeof(RIL_LceStatusInfo));
    RFX_ASSERT(response != NULL);

    /* Use AT+ELCE=0 to stop LCE service */
    p_response = atSendCommandSingleline(String8::format("AT+ELCE=0"), "+ELCE:");

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response!",
                mSlotId, __FUNCTION__);
        goto error;
    }

    if (p_response->isAtResponseFail()) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+ELCE=0 responce ERROR:%d",
                mSlotId, __FUNCTION__, p_response->atGetCmeError());
        goto error;
    }

    /* pLine => +ELCE:<lce_status>,<actual_interval> */
    pLine = p_response->getIntermediates();

    if (pLine == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get intermediate results!",
                mSlotId, __FUNCTION__);
        goto error;
    }

    pLine->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start", mSlotId, __FUNCTION__);
        goto error;
    }

    /* Get 1st parameter: lceStatus */
    lceStatus = (char)pLine->atTokNextint(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing lce status",
                mSlotId, __FUNCTION__);
        goto error;
    }

    /* Get 2nd parameter: actualInterval */
    if (pLine->atTokHasmore()) {
        actualInterval = pLine->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing actual interval",
                    mSlotId, __FUNCTION__);
            goto error;
        }
    }

done:
    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] lceStatus=%d, actualInterval=%u",
            mSlotId, __FUNCTION__, lceStatus, actualInterval);
    response->lce_status = lceStatus;
    response->actual_interval_ms = actualInterval;
    responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS,
            RfxLceStatusResponseData(response, sizeof(RIL_LceStatusInfo)), msg);
    responseToTelCore(responseMsg);
    FREEIF(response);
    return;

error:
    responseMsg = RfxMclMessage::obtainResponse(RIL_E_GENERIC_FAILURE, RfxVoidData(), msg);
    responseToTelCore(responseMsg);
    FREEIF(response);
}

void RmcDcMiscImpl::requestPullLceData(const sp<RfxMclMessage>& msg) {
    RFX_UNUSED(msg);
    RfxAtLine *pLine = NULL;
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> responseMsg;
    int lceMode = 0; // LCE service mode : stop mode = 0; start with PUSH mode = 1; start with PULL mode = 2.
    int err = 0;

    /* Initialize LCE data response */
    RIL_LceDataInfo* response = (RIL_LceDataInfo*)calloc(1, sizeof(RIL_LceDataInfo));
    RFX_ASSERT(response != NULL);

    /* Use AT+ELCE? to pull LCE service for capacity information */
    p_response = atSendCommandSingleline(String8::format("AT+ELCE?"), "+ELCE:");

    if (p_response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get p_response!",
                mSlotId, __FUNCTION__);
        goto error;
    }

    if (p_response->isAtResponseFail()) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] AT+ELCE? responce ERROR", mSlotId, __FUNCTION__);
        goto error;
    }

    /* pLine => +ELCE:<lce_mode>[,<last_hop_capacity_kbps>,<confidence_level>,<lce_suspended>] */
    pLine = p_response->getIntermediates();

    if (pLine == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get intermediate results!",
                mSlotId, __FUNCTION__);
        goto error;
    }

    pLine->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start", mSlotId, __FUNCTION__);
        goto error;
    }

    /* Get 1st parameter: lceMode(unused) */
    lceMode = pLine->atTokNextint(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing lce mode",
                mSlotId, __FUNCTION__);
        goto error;
    }

    if (pLine->atTokHasmore()) {
        /* Get 2nd parameter: last_hop_capacity_kbps */
        response->last_hop_capacity_kbps = pLine->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing last hop capacity",
                    mSlotId, __FUNCTION__);
            goto error;
        }

        if (pLine->atTokHasmore()) {
            /* Get 3rd parameter: confidence_level */
            response->confidence_level = (unsigned char)pLine->atTokNextint(&err);
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing confidence level",
                        mSlotId, __FUNCTION__);
                goto error;
            }

            if (pLine->atTokHasmore()) {
                /* Get 4th parameter: lce_suspended */
                response->lce_suspended = (unsigned char)pLine->atTokNextint(&err);
                if (err < 0) {
                    RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing lce suspended",
                            mSlotId, __FUNCTION__);
                    goto error;
                }
            }
        }
    }

    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] last_hop_capacity_kbps=%u, confidence_level=%u, "
            "lce_suspended=%u", mSlotId, __FUNCTION__, response->last_hop_capacity_kbps,
            response->confidence_level, response->lce_suspended);
    responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS,
            RfxLceDataResponseData(response, sizeof(RIL_LceDataInfo)), msg);
    responseToTelCore(responseMsg);
    FREEIF(response);
    return;

error:
    responseMsg = RfxMclMessage::obtainResponse(RIL_E_GENERIC_FAILURE, RfxVoidData(), msg);
    responseToTelCore(responseMsg);
    FREEIF(response);
}

void RmcDcMiscImpl::onLceStatusChanged(const sp<RfxMclMessage>& msg) {
    //+ELCE: <last_hop_capacity_kbps>,<confidence_level>,<lce_suspended>
    char *urc = (char*)msg->getData()->getData();
    int err = 0;
    RfxAtLine *pLine = NULL;
    sp<RfxMclMessage> urcMsg;

    /* Initialize LCE data response */
    RIL_LceDataInfo* response = (RIL_LceDataInfo*)calloc(1, sizeof(RIL_LceDataInfo));
    RFX_ASSERT(response != NULL);

    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] urc=%s", mSlotId, __FUNCTION__, urc);

    pLine = new RfxAtLine(urc, NULL);

    if (pLine == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to new pLine!",
                mSlotId, __FUNCTION__);
        goto error;
    }

    pLine->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start", mSlotId, __FUNCTION__);
        goto error;
    }

    if (pLine->atTokHasmore()) {
        /* Get 1st parameter: last_hop_capacity_kbps */
        response->last_hop_capacity_kbps = pLine->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing last hop capacity",
                    mSlotId, __FUNCTION__);
            goto error;
        }

        if (pLine->atTokHasmore()) {
            /* Get 2nd parameter: confidence_level */
            response->confidence_level = (unsigned char)pLine->atTokNextint(&err);
            if (err < 0) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing confidence level",
                        mSlotId, __FUNCTION__);
                goto error;
            }

            if (pLine->atTokHasmore()) {
                /* Get 3rd parameter: lce_suspended */
                response->lce_suspended = (unsigned char)pLine->atTokNextint(&err);
                if (err < 0) {
                    RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing lce suspended",
                            mSlotId, __FUNCTION__);
                    goto error;
                }
            }
        }
    }

    urcMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_LCEDATA_RECV, mSlotId,
                    RfxLceDataResponseData(response, sizeof(RIL_LceDataInfo)));
    responseToTelCore(urcMsg);
    AT_LINE_FREE(pLine);
    FREEIF(response);
    return;

error:
    AT_LINE_FREE(pLine);
    FREEIF(response);
}

void RmcDcMiscImpl::setFdMode(const sp<RfxMclMessage>& msg) {
    int *pReqInt = (int *)msg->getData()->getData();
    sp<RfxMclMessage> responseToTcl;

    if (pReqInt == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "Weird, Request msg null!");
        responseToTcl = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                RfxVoidData(), msg, true);
        responseToTelCore(responseToTcl);
        return;
    }

    int argsNum = pReqInt[0];
    sp<RfxAtResponse> responseFromModem;

    if (argsNum == 1) {
        // AT+EFD=1: Enable modem fast dormancy.
        // AT+EFD=0: Disable modem fast dormancy.
        responseFromModem = atSendCommand(String8::format("AT+EFD=%d", pReqInt[1]));
    } else if (argsNum == 2) {
        // Format: AT+EFD=3,screen_status(0:screen off, 1:screen on).
        responseFromModem = atSendCommand(String8::format("AT+EFD=%d,%d",
                pReqInt[1], pReqInt[2]));
    } else if (argsNum == 3) {
        // Format: AT+EFD=2,timer_id,timerValue (unit:0.1 sec)
        rfx_property_set(PROPERTY_FD_TIMER[pReqInt[2]], String8::format("%d", pReqInt[3]).string());
        responseFromModem = atSendCommand(String8::format("AT+EFD=%d,%d,%d",
                pReqInt[1], pReqInt[2], pReqInt[3]));
    } else {
        RFX_LOG_E(RFX_LOG_TAG, "Weird, should never be here!");
    }

    if (responseFromModem != NULL && !responseFromModem->isAtResponseFail()) {
        responseToTcl = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                RfxVoidData(), msg, true);
    } else {
        responseToTcl = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_MODEM_ERR,
                RfxVoidData(), msg, true);
    }

    responseToTelCore(responseToTcl);
}

void RmcDcMiscImpl::initializeFastDormancy() {
    // Fast Dormancy is only available on 3G protocol set, so when maxRadio is 2G, disable it.
    int radioCapability = getIntValue(mSlotId, RFX_STATUS_KEY_SLOT_CAPABILITY, 0);
    int maxCapability = RmcCapabilitySwitchUtil::getMaxRadioGeneration(radioCapability);

    if (maxCapability == RADIO_GENERATION_2G) {
        rfx_property_set(PROPERTY_RIL_FD_MODE, "0");
        return;
    }

    sp<RfxAtResponse> response;
    char propertyValue[RFX_PROPERTY_VALUE_MAX] = { 0 };

    // [Step#01] Query if the new FD mechanism is supported by modem or not.
    response = atSendCommandSingleline(String8::format("AT+EFD=?"), "+EFD:");

    if (response != NULL && !response->isAtResponseFail()) {
        // Set PROPERTY_RIL_FD_MODE. Framework can query this to know if AP side is necessary to
        // execute FD or not.
        rfx_property_set(PROPERTY_RIL_FD_MODE, "1");

        // [Step#02] Set default FD related timers for mode:
        //           format => AT+EFD=2, timer_id, timerValue(unit:0.1 sec)
        // timerId=0: Screen Off + Legacy FD
        memset(propertyValue, 0, sizeof(propertyValue));
        rfx_property_get(PROPERTY_FD_SCREEN_OFF_TIMER, propertyValue, DEFAULT_FD_SCREEN_OFF_TIMER);
        atSendCommand(String8::format("AT+EFD=2,0,%d", (int)(atof(propertyValue))));

        // timerId=2: Screen Off + R8 FD
        memset(propertyValue, 0, sizeof(propertyValue));
        rfx_property_get(PROPERTY_FD_SCREEN_OFF_R8_TIMER, propertyValue,
                DEFAULT_FD_SCREEN_OFF_R8_TIMER);
        atSendCommand(String8::format("AT+EFD=2,2,%d", (int)(atof(propertyValue))));

        // timerId=1: Screen On + Legacy FD
        memset(propertyValue, 0, sizeof(propertyValue));
        rfx_property_get(PROPERTY_FD_SCREEN_ON_TIMER, propertyValue, DEFAULT_FD_SCREEN_ON_TIMER);
        atSendCommand(String8::format("AT+EFD=2,1,%d", (int)(atof(propertyValue))));

        // timerId=3: Screen On + R8 FD
        memset(propertyValue, 0, sizeof(propertyValue));
        rfx_property_get(PROPERTY_FD_SCREEN_ON_R8_TIMER, propertyValue,
                DEFAULT_FD_SCREEN_ON_R8_TIMER);
        atSendCommand(String8::format("AT+EFD=2,3,%d", (int)(atof(propertyValue))));

        // For special operator request.
        memset(propertyValue, 0, sizeof(propertyValue));
        rfx_property_get(PROPERTY_FD_ON_ONLY_R8_NETWORK, propertyValue, "0");
        if (atoi(propertyValue) == 1) {
            disableLegacyFastDormancy();
        }
    }
}

int RmcDcMiscImpl::isFastDormancySupport() {
    int isFdSupport = 0;
    char propertyValue[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get(PROPERTY_MTK_FD_SUPPORT, propertyValue, "0");
    isFdSupport = atoi(propertyValue);
    return isFdSupport ? 1 : 0;
}

int RmcDcMiscImpl::getIntValue(int slotId, const RfxStatusKeyEnum key, int default_value) {
    RFX_ASSERT(getMclStatusManager(slotId) != NULL);
    return getMclStatusManager(slotId)->getIntValue(key, default_value);
}

RfxMclStatusManager* RmcDcMiscImpl::getMclStatusManager(int slotId) {
    RFX_ASSERT(mHandler != NULL);
    return mHandler->getMclStatusManager(slotId);
}

sp<RfxAtResponse> RmcDcMiscImpl::atSendCommand(const String8 cmd) {
    RFX_ASSERT(mHandler != NULL);
    return mHandler->atSendCommand(cmd.string());
}

sp<RfxAtResponse> RmcDcMiscImpl::atSendCommandSingleline(const String8 cmd, const char *rspPrefix) {
    RFX_ASSERT(mHandler != NULL);
    return mHandler->atSendCommandSingleline(cmd.string(), rspPrefix);
}

void RmcDcMiscImpl::responseToTelCore(const sp<RfxMclMessage> msg) {
    RFX_ASSERT(mHandler != NULL);
    mHandler->responseToTelCore(msg);
}

void RmcDcMiscImpl::requestSetLinkCapacityReportingCriteria(const sp<RfxMclMessage>& msg) {
    sp<RfxAtResponse> pAtResponse;
    sp<RfxMclMessage> pMclResponse;
    RIL_LinkCapacityReportingCriteria *data = NULL;
    int numOfDlThreshold = 0;
    int numOfUlThreshold = 0;
    String8 cmd("");

    data = (RIL_LinkCapacityReportingCriteria *)msg->getData()->getData();
    if (data == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] data is null, return!", mSlotId, __FUNCTION__);
        pMclResponse = RfxMclMessage::obtainResponse(RIL_E_NO_SUCH_ENTRY, RfxVoidData(), msg);
        responseToTelCore(pMclResponse);
        return;
    }

    // AT+EXLCE = <mode>[,<histerisis_millis>,<dl_hysterisis_kbps>,<ul_hysterisis_kbps>,
    // <num_of_dl_threshold>[,<dl_threshold1>,<dl_threshold2>,<dl_threshold3>,...],
    // <num_of_ul_threshold>[,<ul_threshold1>,<ul_threshold2>,<ul_threshold3>,...],<RAT>]
    // Ex: AT+EXLCE=2,3000,50,50,5,500,1000,5000,10000,20000,5,100,500,1000,5000,10000,1
    // <mode> = 0: disable; 1: enable; 2: set LCE reporting criteria.
    cmd.append(String8::format("AT+EXLCE=2,%d,%d,%d", data->hysteresisMs, data->hysteresisDlKbps,
            data->hysteresisUlKbps));

    numOfDlThreshold = data->thresholdDlKbpsNumber > MAX_LCE_THRESHOLD_NUMBER ?
            MAX_LCE_THRESHOLD_NUMBER : data->thresholdDlKbpsNumber;
    cmd.append(String8::format(",%d", numOfDlThreshold));
    for (int i = 0; i < numOfDlThreshold; i++) {
        cmd.append(String8::format(",%d", data->thresholdDlKbpsList[i]));
    }

    numOfUlThreshold = data->thresholdUlKbpsNumber > MAX_LCE_THRESHOLD_NUMBER ?
            MAX_LCE_THRESHOLD_NUMBER : data->thresholdUlKbpsNumber;
    cmd.append(String8::format(",%d", numOfUlThreshold));
    for (int i = 0; i < numOfUlThreshold; i++) {
        cmd.append(String8::format(",%d", (data->thresholdUlKbpsList)[i]));
    }

    cmd.append(String8::format(",%d", data->accessNetwork));

    pAtResponse = atSendCommand(cmd);
    if (pAtResponse != NULL && !pAtResponse->isAtResponseFail()) {
        pMclResponse = RfxMclMessage::obtainResponse(RIL_E_SUCCESS, RfxVoidData(), msg);
    } else {
        pMclResponse = RfxMclMessage::obtainResponse(RIL_E_MODEM_ERR, RfxVoidData(), msg);
    }

    responseToTelCore(pMclResponse);
}

void RmcDcMiscImpl::onLinkCapacityEstimate(const sp<RfxMclMessage>& msg) {
    // +EXLCE: <downlinkCapacityKbps>,<uplinkCapacityKbps>
    char *urc = (char*)msg->getData()->getData();
    int err = 0;
    RfxAtLine *pLine = NULL;
    RIL_LinkCapacityEstimate *response = NULL;
    sp<RfxMclMessage> urcMsg;

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] urc=%s", mSlotId, __FUNCTION__, urc);

    /* Initialize LCE data response */
    response = (RIL_LinkCapacityEstimate *)calloc(1, sizeof(RIL_LinkCapacityEstimate));
    if (response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] response is null, return!", mSlotId, __FUNCTION__);
        return;
    }

    pLine = new RfxAtLine(urc, NULL);

    if (pLine == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to new pLine!", mSlotId, __FUNCTION__);
        goto error;
    }

    pLine->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start", mSlotId, __FUNCTION__);
        goto error;
    }

    /* Get 1st parameter: downlinkCapacityKbps */
    response->downlinkCapacityKbps = (unsigned int)pLine->atTokNextint(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing downlinkCapacityKbps",
                mSlotId, __FUNCTION__);
        goto error;
    }

    /* Get 2nd parameter: uplinkCapacityKbps */
    response->uplinkCapacityKbps = (unsigned int)pLine->atTokNextint(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing uplinkCapacityKbps",
                mSlotId, __FUNCTION__);
        goto error;
    }

    urcMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_LINK_CAPACITY_ESTIMATE, mSlotId,
            RfxLinkCapacityEstimateData(response, sizeof(RIL_LinkCapacityEstimate)));
    responseToTelCore(urcMsg);
    AT_LINE_FREE(pLine);
    FREEIF(response);
    return;

error:
    AT_LINE_FREE(pLine);
    FREEIF(response);
}

void RmcDcMiscImpl::disableLegacyFastDormancy() {
    sp<RfxAtResponse> response;
    RfxAtLine *pLine;
    int err = 0;
    int mode = 0;
    int parameter = 0;

    response = atSendCommandSingleline(String8::format("AT+EPCT?"), "+EPCT:");

    if (response == NULL || response->isAtResponseFail()) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] fail to get response!", mSlotId, __FUNCTION__);
        return;
    }

    pLine = response->getIntermediates();
    if (pLine == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] pLine is null", mSlotId, __FUNCTION__);
        return;
    }

    pLine->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                mSlotId, __FUNCTION__);
        return;
    }

    mode = pLine->atTokNextint(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing mode",
                mSlotId, __FUNCTION__);
        return;
    }

    if (pLine->atTokHasmore()) {
        parameter = pLine->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing parameter",
                    mSlotId, __FUNCTION__);
            return;
        }
    } else {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] no parameter", mSlotId, __FUNCTION__);
    }

    atSendCommand(String8::format("AT+EPCT=%d,%d",
            mode, ((parameter & FASTDORMANCY_ON) | FASTDORMANCY_LEGACY_OFF)));
}
