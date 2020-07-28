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
#include "NetAgentService.h"

#include "RmcDcDefaultReqHandler.h"
#include "RfxIntsData.h"
#include "RfxVoidData.h"

#define RFX_LOG_TAG "RmcDcDefault"

/*****************************************************************************
 * Class RmcDcDefaultReqHandler
 *****************************************************************************/
RmcDcDefaultReqHandler::RmcDcDefaultReqHandler(int slot_id, int channel_id, RmcDcPdnManager* pdnManager)
: RmcDcCommonReqHandler(slot_id, channel_id, pdnManager) {
}

RmcDcDefaultReqHandler::~RmcDcDefaultReqHandler() {
}

void RmcDcDefaultReqHandler::requestSetupDataCall(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::requestSetupDataCall(msg);
}

void RmcDcDefaultReqHandler::requestDeactivateDataCall(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::requestDeactivateDataCall(msg);
}

void RmcDcDefaultReqHandler::onNwPdnAct(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::onNwPdnAct(msg);
}

void RmcDcDefaultReqHandler::onNwPdnDeact(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::onNwPdnDeact(msg);
}

void RmcDcDefaultReqHandler::onNwModify(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::onNwModify(msg);
}

void RmcDcDefaultReqHandler::onNwReact(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::onNwReact(msg);
}

void RmcDcDefaultReqHandler::onMePdnAct(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::onMePdnAct(msg);
}

void RmcDcDefaultReqHandler::onMePdnDeact(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::onMePdnDeact(msg);
}

void RmcDcDefaultReqHandler::onPdnChange(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::onPdnChange(msg);
}

void RmcDcDefaultReqHandler::onQualifiedNetworkTypeChanged(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::onQualifiedNetworkTypeChanged(msg);
}

void RmcDcDefaultReqHandler::requestSyncApnTable(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::requestSyncApnTable(msg);
}

void RmcDcDefaultReqHandler::requestSyncDataSettingsToMd(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::requestSyncDataSettingsToMd(msg);
}

void RmcDcDefaultReqHandler::requestResetMdDataRetryCount(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::requestResetMdDataRetryCount(msg);
}

void RmcDcDefaultReqHandler::requestLastFailCause(const sp<RfxMclMessage>& msg) {
    int cause = getLastFailCause();
    sp<RfxMclMessage> responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS,
            RfxIntsData(&cause, 1), msg);
    responseToTelCore(responseMsg);
}

void RmcDcDefaultReqHandler::requestOrSendDataCallList(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::requestOrSendDataCallList(msg);
}

void RmcDcDefaultReqHandler::requestOrSendDataCallList(const sp<RfxMclMessage>& msg, Vector<int> *vAidList) {
    RmcDcCommonReqHandler::requestOrSendDataCallList(msg, vAidList);
}

void RmcDcDefaultReqHandler::requestOrSendDataCallList(const sp<RfxMclMessage>& msg, int aid) {
    RmcDcCommonReqHandler::requestOrSendDataCallList(msg, aid);
}

void RmcDcDefaultReqHandler::requestQueryPco(int aid, int ia, const char* apn, const char* iptype) {
    RmcDcCommonReqHandler::requestQueryPco(aid, ia, apn, iptype);
}

void RmcDcDefaultReqHandler::requestSetLteAccessStratumReport(const sp<RfxMclMessage>& msg) {
    // AT+EDRB=<mode>
    // <mode>
    //     0   Turn off URC +EDRBSTATE
    //     1   Turn on URC +EDRBSTATE
    sp<RfxAtResponse> p_response;
    sp<RfxMclMessage> response;
    int *pReqData = (int *)msg->getData()->getData();
    int enabled = pReqData[0];

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] enabled: %d", m_slot_id, __FUNCTION__, enabled);

    p_response = atSendCommand(String8::format("AT+EDRB=%d", enabled));

    if (p_response->isAtResponseFail()) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s]AT+EDRB command returns ERROR", m_slot_id, __FUNCTION__);
        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                RfxVoidData(), msg, false);
    } else {
        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                RfxVoidData(), msg, false);
    }
    responseToTelCore(response);
}

void RmcDcDefaultReqHandler::onLteAccessStratumStateChange(const sp<RfxMclMessage>& msg) {
    // +EDRBSTATE:<drb_state>[,<AcT>]
    // <drb_state>
    //     1   DRB is exist
    //     0   DRB is not existed
    // <AcT> only reported when drb_state = 0, report the current PS access technology
    int err = 0;
    int response[DRB_NUM] = {0};
    char *urc = (char*)msg->getData()->getData();
    RfxAtLine *pLine = new RfxAtLine(urc, NULL);
    sp<RfxMclMessage> urcMsg;

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] urc=%s", m_slot_id, __FUNCTION__, urc);

    pLine->atTokStart(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when token start",
                m_slot_id, __FUNCTION__);
        goto error;
    }

    response[DRB_STATE] = pLine->atTokNextint(&err);
    if (err < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing <drb_state>",
                m_slot_id, __FUNCTION__);
        goto error;
    }

    if (DRB_NOT_EXISTED == response[DRB_STATE]) {  // 2G/3G/4G connected but no DRB exists/4G AS idle
        response[DRB_ACT] = pLine->atTokNextint(&err);
        if (err < 0) {
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] ERROR occurs when parsing <AcT>",
                    m_slot_id, __FUNCTION__);
            goto error;
        }
        response[DRB_ACT] = convertNetworkType(response[DRB_ACT]);
    }
    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] <drb_state>: %d, <AcT>: %d",
            m_slot_id, __FUNCTION__, response[DRB_STATE], response[DRB_ACT]);

    if (DRB_NOT_EXISTED == response[DRB_STATE]) {
        urcMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_LTE_ACCESS_STRATUM_STATE_CHANGE,
                m_slot_id, RfxIntsData(response, DRB_NUM));
        responseToTelCore(urcMsg);
    } else {
        urcMsg = RfxMclMessage::obtainUrc(RFX_MSG_URC_LTE_ACCESS_STRATUM_STATE_CHANGE,
                m_slot_id, RfxIntsData(&response[DRB_STATE], DRB_NUM - 1));
        responseToTelCore(urcMsg);
    }

error:
    AT_LINE_FREE(pLine);
}

void RmcDcDefaultReqHandler::requestSetLteUplinkDataTransfer(const sp<RfxMclMessage>& msg) {
    // Tclose timer(sec.) << 16 | 0 : stop uplink data transfer with Tclose timer
    // 1 : start uplink data transfer
    int *pReqData = (int *)msg->getData()->getData();
    int state = pReqData[0];
    int transIntfId = pReqData[1];
    NetAgentService *pNetAgentService = NetAgentService::getInstance();
    sp<NetActionBase> action;

    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] state: %d, transIntfId: %d",
            m_slot_id, __FUNCTION__, state, transIntfId);

    if (NULL == pNetAgentService) {
        RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] NetAgentService is NULL, return error",
                m_slot_id, __FUNCTION__);
        requestSetLteUplinkDataTransferCompleted(false, msg);
    } else {
        action = new NetAction<const sp<RfxMclMessage>>(this,
                &RmcDcDefaultReqHandler::requestSetLteUplinkDataTransferCompleted, msg);
        pNetAgentService->setNetworkTransmitState(state, transIntfId, action);
    }
}

void RmcDcDefaultReqHandler::requestSetLteUplinkDataTransferCompleted(
        bool bSuccess, const sp<RfxMclMessage> msg) {
    sp<RfxMclMessage> response;
    if (bSuccess) {
        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
                RfxVoidData(), msg, false);
    } else {
        response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_GENERIC_FAILURE,
                RfxVoidData(), msg, false);
    }
    responseToTelCore(response);
}

int RmcDcDefaultReqHandler::convertNetworkType(int nAct) {
    int nRet = 0;
    switch (nAct) {
        case PS_ACT_GSM:
        case PS_ACT_GSM_COMPACT:
            nRet = RADIO_TECH_GPRS;
            break;
        case PS_ACT_UTRAN:
            nRet = RADIO_TECH_UMTS;
            break;
        case PS_ACT_GSM_W_EGPRS:
            nRet = RADIO_TECH_EDGE;
            break;
        case PS_ACT_UTRAN_W_HSDPA:
            nRet = RADIO_TECH_HSDPA;
            break;
        case PS_ACT_UTRAN_W_HSUPA:
            nRet = RADIO_TECH_HSUPA;
            break;
        case PS_ACT_UTRAN_W_HSDPA_AND_HSUPA:
            nRet = RADIO_TECH_HSPA;
            break;
        case PS_ACT_E_UTRAN:
            nRet = RADIO_TECH_LTE;
            break;
        default:
            nRet = RADIO_TECH_UNKNOWN;
            break;
    }
    return nRet;
}

void RmcDcDefaultReqHandler::requestClearAllPdnInfo(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::requestClearAllPdnInfo(msg);
}

void RmcDcDefaultReqHandler::requestResendSyncDataSettingsToMd(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::requestResendSyncDataSettingsToMd(msg);
}

void RmcDcDefaultReqHandler::handleResetAllConnections(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::handleResetAllConnections(msg);
}

void RmcDcDefaultReqHandler::requestSetPreferredDataModem(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::requestSetPreferredDataModem(msg);
}

void RmcDcDefaultReqHandler::requestGetDataContextIds(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::requestGetDataContextIds(msg);
}

void RmcDcDefaultReqHandler::requestSendQualifiedNetworkTypesChanged(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::requestSendQualifiedNetworkTypesChanged(msg);
}

void RmcDcDefaultReqHandler::mobileDataUsageNotify(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::mobileDataUsageNotify(msg);
}

void RmcDcDefaultReqHandler::requestStartKeepalive(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::requestStartKeepalive(msg);
}

void RmcDcDefaultReqHandler::requestStopKeepalive(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::requestStopKeepalive(msg);
}

void RmcDcDefaultReqHandler::updateKeepaliveStatus(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::updateKeepaliveStatus(msg);
}

void RmcDcDefaultReqHandler::onNwLimitNotify(const sp<RfxMclMessage>& msg) {
    RmcDcCommonReqHandler::onNwLimitNotify(msg);
}
