/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#include "RmcModemRequestHandler.h"
#include "RfxVoidData.h"

#define RFX_LOG_TAG "RmcModemRequestHandler"

// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcModemRequestHandler, RIL_CMD_PROXY_9);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData,
        RFX_MSG_REQUEST_MODEM_POWERON);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData,
        RFX_MSG_REQUEST_MODEM_POWEROFF);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData,
        RFX_MSG_REQUEST_RESET_RADIO);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData,
        RFX_MSG_REQUEST_RESTART_RILD);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData,
        RFX_MSG_REQUEST_SHUTDOWN);

RmcModemRequestHandler::RmcModemRequestHandler(int slotId, int channelId) :
        RfxBaseHandler (slotId, channelId) {
    logD(RFX_LOG_TAG, "RmcModemRequestHandler constructor");
    const int request[] = {
        RFX_MSG_REQUEST_MODEM_POWERON,
        RFX_MSG_REQUEST_MODEM_POWEROFF,
        RFX_MSG_REQUEST_RESET_RADIO,
        RFX_MSG_REQUEST_RESTART_RILD,
        RFX_MSG_REQUEST_SHUTDOWN,
    };
    registerToHandleRequest(request, sizeof(request)/sizeof(int));
}

RmcModemRequestHandler::~RmcModemRequestHandler() {
}

void RmcModemRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    int id = msg->getId();
    logD(RFX_LOG_TAG, "onHandleRequest: %s(%d)", idToString(id), id);
    switch(id) {
        case RFX_MSG_REQUEST_MODEM_POWERON:
            requestModemPowerOn(msg);
            break;
        case RFX_MSG_REQUEST_MODEM_POWEROFF:
        case RFX_MSG_REQUEST_SHUTDOWN:
            requestModemPowerOff(msg);
            break;
        case RFX_MSG_REQUEST_RESET_RADIO:
        case RFX_MSG_REQUEST_RESTART_RILD:
            requestResetRadio(msg);
            break;
        default:
            logE(RFX_LOG_TAG, "Should not be here");
            break;
    }
}

void RmcModemRequestHandler::requestModemPowerOn(const sp<RfxMclMessage> &msg) {
    logD(RFX_LOG_TAG, "%s", __FUNCTION__);
    rfx_property_set("ril.getccci.response","0");

    bool mdOn = !getNonSlotMclStatusManager()->getBoolValue(RFX_STATUS_KEY_MODEM_POWER_OFF, false);
    if (mdOn) {
        logD(RFX_LOG_TAG, "requestModemPowerOn but already modem powered on");
        sp<RfxMclMessage> responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS,
                RfxVoidData(), msg);
        responseToTelCore(responseMsg);
        return;
    }
    getNonSlotMclStatusManager()->setBoolValue(RFX_STATUS_KEY_MODEM_POWER_OFF, false);

    logD(RFX_LOG_TAG, "SET vendor.ril.ipo.radiooff to 0");
    rfx_property_set("vendor.ril.ipo.radiooff","0");

    logD(RFX_LOG_TAG, "Flight mode power on modem, trigger CCCI power on modem (new versio)");
    RfxRilUtils::triggerCCCIIoctl(CCCI_IOC_LEAVE_DEEP_FLIGHT_ENHANCED);

    sp<RfxMclMessage> responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS,
            RfxVoidData(), msg);
    responseToTelCore(responseMsg);
    rfx_property_set("ril.getccci.response","1");
}

void RmcModemRequestHandler::requestModemPowerOff(const sp<RfxMclMessage> &msg) {
        int mdOff = getNonSlotMclStatusManager()->getBoolValue(RFX_STATUS_KEY_MODEM_POWER_OFF,
                false);
        logD(RFX_LOG_TAG, "%s(): mdOff:%d, rid:%d.", __FUNCTION__, mdOff, m_slot_id);

        if (mdOff) {
            logD(RFX_LOG_TAG, "Flight mode power off modem but already modem powered off");
            sp<RfxMclMessage> responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS,
                    RfxVoidData(), msg);
            responseToTelCore(responseMsg);
            return;
        }

        atSendCommand("AT+EFUN=0");
        // Normal AT may be blocked if s_md_off is 1
        getNonSlotMclStatusManager()->setBoolValue(RFX_STATUS_KEY_MODEM_POWER_OFF, true);

        logD(RFX_LOG_TAG, "ENTER requestRadio PowerOff, set vendor.ril.ipo.radiooff to -1");
        rfx_property_set("vendor.ril.ipo.radiooff","-1");

        atSendCommand("AT+ECUSD=2,2");
        atSendCommand("AT+EMDT=0");
        atSendCommand("AT+EPOF");
        RfxRilUtils::triggerCCCIIoctl(CCCI_IOC_ENTER_DEEP_FLIGHT_ENHANCED);

        logD(RFX_LOG_TAG, "requestRadioPowerOff SET vendor.ril.ipo.radiooff to 1");
        rfx_property_set("vendor.ril.ipo.radiooff","1");

        for(int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++){
            getMclStatusManager(i)->setIntValue(RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_OFF);
        }
        logD(RFX_LOG_TAG, "Flight mode power off modem, trigger CCCI level 2 power off");
        int param = -1;

        sp<RfxMclMessage> responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS,
                RfxVoidData(), msg);
        responseToTelCore(responseMsg);

        /*if (queryBtSapStatus(rid) != BT_SAP_INIT) {
            LOGD("Flight mode power off modem, during SAP connection => disconnect SAP connection");
            disconnectSapConnection(rid);
        }*/
}

void RmcModemRequestHandler::requestResetRadio(const sp<RfxMclMessage> &msg) {
    logD(RFX_LOG_TAG, "start to reset radio - requestResetRadio");
    int mainSlotId = getNonSlotMclStatusManager()->getIntValue(
            RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT, 0);
    int mdOff = getNonSlotMclStatusManager()->getBoolValue(RFX_STATUS_KEY_MODEM_POWER_OFF, false);

    // only do power off when it is on
    if (!mdOff) {
        getNonSlotMclStatusManager()->setBoolValue(RFX_STATUS_KEY_MODEM_POWER_OFF, true);
        atSendCommand("AT+EMDT=0");
        atSendCommand("AT+EFUN=0");
        logD(RFX_LOG_TAG, "%s(): set radio off before power off MD.", __FUNCTION__);
        atSendCommand("AT+EPOF");

        // power off modem
        logD(RFX_LOG_TAG, "requestRadioPowerOff SET vendor.ril.ipo.radiooff to 1");
        rfx_property_set("vendor.ril.ipo.radiooff","1");
        for(int i = 0; i < RfxRilUtils::rfxGetSimCount(); i++){
            getMclStatusManager(i)->setIntValue(RFX_STATUS_KEY_RADIO_STATE, RADIO_STATE_OFF);
        }
        logD(RFX_LOG_TAG, "Flight mode power off modem, trigger CCCI level 2 power off");
        int param = -1;
        RfxRilUtils::triggerCCCIIoctl(CCCI_IOC_ENTER_DEEP_FLIGHT_ENHANCED);
    }

    // power on modem
    logD(RFX_LOG_TAG, "SET vendor.ril.ipo.radiooff to 0");
    rfx_property_set("vendor.ril.ipo.radiooff","0");
    logD(RFX_LOG_TAG, "Flight mode power on modem, trigger CCCI power on modem (new versio)");
    RfxRilUtils::triggerCCCIIoctl(CCCI_IOC_LEAVE_DEEP_FLIGHT_ENHANCED);

    sp<RfxMclMessage> responseMsg = RfxMclMessage::obtainResponse(RIL_E_SUCCESS,
            RfxVoidData(), msg);
    responseToTelCore(responseMsg);
}
