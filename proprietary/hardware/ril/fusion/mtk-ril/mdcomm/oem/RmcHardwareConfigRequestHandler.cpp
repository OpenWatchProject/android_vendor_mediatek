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

#include "RmcHardwareConfigRequestHandler.h"
#include "RfxRilUtils.h"
#include "RfxVoidData.h"
#include <string.h>

#define RFX_LOG_TAG "RmcHdcHdr"

#define DEV_MODEM_RIL_MODEL_SINGLE 0;

// register data
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxHardwareConfigData,
        RFX_MSG_REQUEST_GET_HARDWARE_CONFIG);
RFX_REGISTER_DATA_TO_URC_ID(RfxHardwareConfigData,
        RFX_MSG_UNSOL_HARDWARE_CONFIG_CHANGED);
RFX_REGISTER_DATA_TO_EVENT_ID(RfxVoidData, RFX_MSG_EVENT_CAPABILITY_INIT_DONE);
// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcHardwareConfigRequestHandler, RIL_CMD_PROXY_1);

RmcHardwareConfigRequestHandler::RmcHardwareConfigRequestHandler(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id) {
    const int request[] = {
        RFX_MSG_REQUEST_GET_HARDWARE_CONFIG
    };

    const int eventList[] = {
        RFX_MSG_EVENT_CAPABILITY_INIT_DONE
    };
    registerToHandleRequest(request, sizeof(request)/sizeof(int));
    registerToHandleEvent(eventList, sizeof(eventList) / sizeof(int));
}

RmcHardwareConfigRequestHandler::~RmcHardwareConfigRequestHandler() {
}

void RmcHardwareConfigRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    int id = msg->getId();
    logD(RFX_LOG_TAG, "onHandleRequest: %d", id);
    switch (id) {
        case RFX_MSG_REQUEST_GET_HARDWARE_CONFIG:
            handleHardwareConfigRequest(msg);
            break;
        default:
            logE(RFX_LOG_TAG, "Should not be here");
            break;
    }
}

void RmcHardwareConfigRequestHandler::onHandleEvent(const sp<RfxMclMessage>& msg) {
    int id = msg->getId();
    logD(RFX_LOG_TAG, "onHandleEvent: %d", id);
    switch (id) {
        case RFX_MSG_EVENT_CAPABILITY_INIT_DONE:
            sendHardwareConfigUrc();
            break;
        default:
            logE(RFX_LOG_TAG, "should not be here");
            break;
    }
}

void RmcHardwareConfigRequestHandler::handleHardwareConfigRequest(const sp<RfxMclMessage>& msg) {
    RIL_HardwareConfig hardwareConfigs[2];
    logD(RFX_LOG_TAG, "handleHardwareConfigRequest: m_slot_id:%d", m_slot_id);
    getHardwareConfig(hardwareConfigs);

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(msg->getId(), RIL_E_SUCCESS,
            RfxHardwareConfigData((void *)hardwareConfigs, sizeof(hardwareConfigs)), msg, false);
    responseToTelCore(response);
    sendHardwareConfigUrc();
}

void RmcHardwareConfigRequestHandler::sendHardwareConfigUrc() {
    RIL_HardwareConfig hardwareConfigs[2];
    logD(RFX_LOG_TAG, "sendHardwareConfigUrc: m_slot_id:%d", m_slot_id);
    getHardwareConfig(hardwareConfigs);

    sp<RfxMclMessage> urc = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_HARDWARE_CONFIG_CHANGED,
                m_slot_id, RfxHardwareConfigData((void *)hardwareConfigs, sizeof(hardwareConfigs)));
    responseToTelCore(urc);
}

void RmcHardwareConfigRequestHandler::getHardwareConfig(RIL_HardwareConfig *hardwareConfigs) {
    char logicalModemId[MAX_UUID_LENGTH];
    RfxRilUtils::getLogicalModemId(logicalModemId, sizeof(logicalModemId), m_slot_id);

    // type modem
    hardwareConfigs[0].type = RIL_HARDWARE_CONFIG_MODEM;
    strncpy(hardwareConfigs[0].uuid, logicalModemId, MAX_UUID_LENGTH);
    hardwareConfigs[0].state = RIL_HARDWARE_CONFIG_STATE_ENABLED;
    hardwareConfigs[0].cfg.modem.rilModel = DEV_MODEM_RIL_MODEL_SINGLE;

    int slotCapability =
            getMclStatusManager(m_slot_id)->getIntValue(RFX_STATUS_KEY_SLOT_CAPABILITY, 0);
    hardwareConfigs[0].cfg.modem.rat = slotCapability;
    hardwareConfigs[0].cfg.modem.maxVoice = 1;
    hardwareConfigs[0].cfg.modem.maxData = 1;
    hardwareConfigs[0].cfg.modem.maxStandby = RfxRilUtils::rfxGetSimCount();

    // type sim
    hardwareConfigs[1].type = RIL_HARDWARE_CONFIG_SIM;
    strncpy(hardwareConfigs[1].uuid, logicalModemId, MAX_UUID_LENGTH);
    hardwareConfigs[1].state = RIL_HARDWARE_CONFIG_STATE_ENABLED;
    strncpy(hardwareConfigs[1].cfg.sim.modemUuid, logicalModemId, MAX_UUID_LENGTH);
}
