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

#include "RmcWpURCHandler.h"
#include "RmcWpRequestHandler.h"
#include <telephony/mtk_ril.h>
#include "RfxLog.h"
#include "rfx_properties.h"

#define WP_LOG_TAG "RmcWp"
RFX_IMPLEMENT_HANDLER_CLASS(RmcWpURCHandler, RIL_CMD_PROXY_URC);
int RmcWpURCHandler::mSlotId = 0;
int RmcWpURCHandler::ecsraUrcParams[5] = {0};

RmcWpURCHandler::RmcWpURCHandler(int slot_id, int channel_id):RfxBaseHandler(slot_id, channel_id) {
    const char* urc[] = {
        "+ECSRA",
    };
    logD(WP_LOG_TAG, "constructor slot_id = %d, channel_id = %d", slot_id, channel_id);
    registerToHandleURC(urc, sizeof(urc)/sizeof(char *));
}

RmcWpURCHandler::~RmcWpURCHandler() {
}

void RmcWpURCHandler::onHandleUrc(const sp<RfxMclMessage>& msg) {
    logD(WP_LOG_TAG, "+ECSRA info = %s", msg->getRawUrc()->getLine());
    if(strStartsWith(msg->getRawUrc()->getLine(), "+ECSRA:")) {
        mSlotId = msg->getSlotId();
        handleWorldModeUrc(msg);
    }
}

void RmcWpURCHandler::handleWorldModeUrc(const sp<RfxMclMessage>& msg) {
    int err;
    //parse urc info
    logD(WP_LOG_TAG, "handleWorldModeUrc begin");
    RfxAtLine* Urcline = msg->getRawUrc();
    Urcline->atTokStart(&err);

    if (err < 0) {
        logD(WP_LOG_TAG, "handleWorldModeUrc err %d", err);
        return;
    }

    for (int i = 0; i <= 4; i++) {
        ecsraUrcParams[i] = Urcline->atTokNextint(&err);
        if (err < 0) {
            logD(WP_LOG_TAG, "handleWorldModeUrc err %d %d %d", err, i, ecsraUrcParams[i]);
            return;
        }
    }

    //set world mode prepare state
    rfx_property_set("persist.vendor.radio.wm_state", "-1");
    //inform telcore to world mode change start
    int WorldModeState[2];
    WorldModeState[0] = 0;
    // send cause to RTC
    WorldModeState[1] = ecsraUrcParams[4];
    sp<RfxMclMessage> urc = RfxMclMessage::obtainUrc(
            RFX_MSG_URC_WORLD_MODE_CHANGED, mSlotId, RfxIntsData(WorldModeState, 2));
    responseToTelCore(urc);
    logD(WP_LOG_TAG, "handleWorldModeUrc state = %d", WorldModeState[0]);
}

int RmcWpURCHandler::getSlotId() {
    return mSlotId;
}

int RmcWpURCHandler::getEcsraUrcParam(int index) {
    assert(index >= 0 && index <= 4);
    return ecsraUrcParams[index];
}

bool RmcWpURCHandler::onCheckIfRejectMessage(const sp<RfxMclMessage>& msg, RIL_RadioState radioState) {
    bool reject = false;
    if (RADIO_STATE_UNAVAILABLE == radioState) {
        if (strStartsWith(msg->getRawUrc()->getLine(), "+ECSRA:")) {
            reject = false;
        } else {
            reject = true;
        }
    }
    logD(WP_LOG_TAG, "onCheckIfRejectMessage: %d %d", radioState, reject);
    return reject;
}
