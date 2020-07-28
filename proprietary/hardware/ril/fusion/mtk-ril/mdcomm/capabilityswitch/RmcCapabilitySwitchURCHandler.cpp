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

#include "RmcCapabilitySwitchURCHandler.h"
#include "RmcCapabilitySwitchUtil.h"
#include "RmcCapabilitySwitchRequestHandler.h"
#include <telephony/mtk_ril.h>
#include "RfxMessageId.h"
#include "RfxRadioCapabilityData.h"
#include "rfx_properties.h"

#define RFX_LOG_TAG "RmcCapa"

RFX_IMPLEMENT_HANDLER_CLASS(RmcCapabilitySwitchURCHandler, RIL_CMD_PROXY_URC);

Mutex RmcCapabilitySwitchURCHandler::s_switch_urc_channel_mutex;
int RmcCapabilitySwitchURCHandler::s_urc_count = 0;

RmcCapabilitySwitchURCHandler::RmcCapabilitySwitchURCHandler(int slot_id, int channel_id):
    RfxBaseHandler(slot_id, channel_id) {
    const char* urc[] = {
        "+ESIMMAP",
        "+EPSMAP"
    };
    logI(RFX_LOG_TAG, "constructor slot_id = %d, channel_id = %d", slot_id, channel_id);
    registerToHandleURC(urc, sizeof(urc)/sizeof(char *));
}

RmcCapabilitySwitchURCHandler::~RmcCapabilitySwitchURCHandler() {
}

void RmcCapabilitySwitchURCHandler::onHandleUrc(const sp<RfxMclMessage> &msg) {
    if (RmcCapabilitySwitchUtil::isDssNoResetSupport() == false) {
        return;
    }
    if(strStartsWith(msg->getRawUrc()->getLine(), "+ESIMMAP:")) {
        handleSimSwitchUrc(msg, 0);
    } else if(strStartsWith(msg->getRawUrc()->getLine(), "+EPSMAP:")) {
        handleSimSwitchUrc(msg, 1);
    }
}

void RmcCapabilitySwitchURCHandler::handleSimSwitchUrc(const sp<RfxMclMessage>& msg, int version) {
    int err;
    int new_major_slot;
    int old_major_slot = getNonSlotMclStatusManager()->getIntValue(
            RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT, 0);
    //parse urc info
    logI(RFX_LOG_TAG, "handleSimSwitchUrc begin, old_major_slot=%d, urc_count=%d",
         old_major_slot, s_urc_count);
    RfxAtLine* urc_line = msg->getRawUrc();
    urc_line->atTokStart(&err);
    if (err < 0) {
        logE(RFX_LOG_TAG, "handleSimSwitchUrc err %d", err);
        return;
    }
    new_major_slot = urc_line->atTokNextint(&err);
    if (err < 0) {
        logE(RFX_LOG_TAG, "handleSimSwitchUrc first int err %d %d", err, new_major_slot);
        return;
    }
    if (version == 0) {
        int i;
        for (i = 0; new_major_slot > 1; i++) {
            new_major_slot = new_major_slot >> 1;
        }
        new_major_slot = i;
    } else {
        new_major_slot = urc_line->atTokNextint(&err) - 1;
        if (err < 0) {
            logE(RFX_LOG_TAG, "handleSimSwitchUrc second int err %d %d", err, new_major_slot);
            return;
        }
    }
    s_switch_urc_channel_mutex.lock();
    if (checkIfSwitch()) {
        logI(RFX_LOG_TAG, "switch urc channels");
        RmcCapabilitySwitchRequestHandler::setSIMInitState(0);
        for(int i = RIL_URC; i < RfxChannelManager::getSupportChannels(); i+=RIL_CHANNEL_OFFSET) {
            lockReaderMutex(i);
        }
        RmcCapabilitySwitchRequestHandler::switchChannel(RIL_URC, old_major_slot, new_major_slot);
        getNonSlotMclStatusManager()->setBoolValue(RFX_STATUS_KEY_CAPABILITY_SWITCH_URC_CHANNEL,
                                                   false);
        for(int i = RIL_CMD_11; i < RfxChannelManager::getSupportChannels();
                i += RIL_CHANNEL_OFFSET) {
            RmcCapabilitySwitchRequestHandler::lockRestartMutex(i);
        }
        RmcCapabilitySwitchRequestHandler::switchChannel(RIL_CMD_11, old_major_slot,
                new_major_slot);
        for(int i = RIL_CMD_11; i < RfxChannelManager::getSupportChannels();
                i += RIL_CHANNEL_OFFSET) {
            RmcCapabilitySwitchRequestHandler::unlockRestartMutex(i);
        }
        for(int i = RIL_URC; i < RfxChannelManager::getSupportChannels(); i+=RIL_CHANNEL_OFFSET) {
            unlockReaderMutex(i);
        }
    }
    s_switch_urc_channel_mutex.unlock();
}

void RmcCapabilitySwitchURCHandler::lockReaderMutex(int channel_id) {
    RfxChannel *p_channel;
    RfxChannelContext *p_channel_context;

    p_channel = RfxChannelManager::getChannel(channel_id);
    p_channel_context = p_channel->getContext();
    p_channel_context->m_readerMutex.lock();
}

void RmcCapabilitySwitchURCHandler::unlockReaderMutex(int channel_id) {
    RfxChannel *p_channel;
    RfxChannelContext *p_channel_context;

    p_channel = RfxChannelManager::getChannel(channel_id);
    p_channel_context = p_channel->getContext();
    p_channel_context->m_readerMutex.unlock();
}

bool RmcCapabilitySwitchURCHandler::checkIfSwitch() {
    bool ret = getNonSlotMclStatusManager()->
            getBoolValue(RFX_STATUS_KEY_CAPABILITY_SWITCH_URC_CHANNEL, false);
    if (ret) {
        s_urc_count++;
        if (RfxRilUtils::rfxGetSimCount() == s_urc_count) {
            s_urc_count = 0;
            return true;
        }
    }
    return false;
}
