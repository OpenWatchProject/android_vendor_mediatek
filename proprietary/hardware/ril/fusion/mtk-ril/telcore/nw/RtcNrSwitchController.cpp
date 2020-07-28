/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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
#include "RtcNrSwitchController.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

#define NRSW_CTRL_TAG "RtcNrSwCtrl"

RFX_IMPLEMENT_CLASS("RtcNrSwitchController", RtcNrSwitchController, RfxController);

RtcNrSwitchController::RtcNrSwitchController() :
    mNrSlot(0),
    mPreferredDataSlot(0) {
}

RtcNrSwitchController::~RtcNrSwitchController() {
}

void RtcNrSwitchController::onInit() {
    // Required: invoke super class implementation
    RfxController::onInit();
    logV(NRSW_CTRL_TAG, "[onInit]");
    char feature[] = "GEMINI_NR_PLUS_SUPPORTED";
    char NR_VER[] = "5G_SUPPORTED";

    const int request_id_list[] = {
        RFX_MSG_REQUEST_NR_SWITCH
    };

    if (getFeatureVersion(NR_VER, 0) == 1 && getFeatureVersion(feature, 0) == 0) {
        registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(const int), DEFAULT);
        char stgBuf[RFX_PROPERTY_VALUE_MAX] = { 0 };
        int val = 0;
        rfx_property_get("persist.vendor.radio.nrslot", stgBuf, "0");
        val = strtol(stgBuf, NULL, 10);
        if (val == 0) {
            rfx_property_set("persist.vendor.radio.nrslot", String8::format("%d", 1).string());
        }
        getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_PREFERRED_DATA_SIM,
                RfxStatusChangeCallback(this, &RtcNrSwitchController::onPreferredDataChanged));

        for (int slotId = RFX_SLOT_ID_0; slotId < RFX_SLOT_COUNT; slotId++) {
            getStatusManager(slotId)->registerStatusChangedEx(RFX_STATUS_KEY_AP_VOICE_CALL_COUNT,
                    RfxStatusChangeCallbackEx(this, &RtcNrSwitchController::onApVoiceCallCountChanged));
        }
    }
}

bool RtcNrSwitchController::onHandleResponse(const sp<RfxMessage>& response) {
    int msg_id = response->getId();

    switch (msg_id) {
        case RFX_MSG_REQUEST_NR_SWITCH:
            responseNrSwitch(response);
            return true;
        default:
            break;
    }
    return true;
}

void RtcNrSwitchController::responseNrSwitch(const sp<RfxMessage>& response) {
    logD(NRSW_CTRL_TAG, "[responseNrSwitch] error: %d ", response->getError());
    if (response->getError() == RIL_E_SUCCESS) {
        mNrSlot = getNonSlotScopeStatusManager()->getIntValue(
                RFX_STATUS_KEY_PREFERRED_DATA_SIM);
    }
}

int RtcNrSwitchController::getNrSlot() {
    return mNrSlot;
}

void RtcNrSwitchController::onPreferredDataChanged(RfxStatusKeyEnum key, RfxVariant old_value,
        RfxVariant value) {
    RFX_UNUSED(key);
    int old_mode = old_value.asInt();
    int new_mode = value.asInt();

    if (new_mode != -1 && old_mode != new_mode) {
        int mainSlot = getNonSlotScopeStatusManager()->getIntValue(
                RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT, 0);
        sp<RfxMessage> reqToRild = RfxMessage::obtainRequest(mainSlot,
                RFX_MSG_REQUEST_NR_SWITCH, RfxIntsData(&new_mode, 1));
        requestToMcl(reqToRild);
    }
    mPreferredDataSlot = new_mode;
}

void RtcNrSwitchController::onApVoiceCallCountChanged(int slotId, RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    int oldMode = old_value.asInt();
    int mode = value.asInt();

    logV(NRSW_CTRL_TAG, "%s, slotId:%d, key:%d oldMode:%d, mode:%d",
            __FUNCTION__, slotId, key, oldMode, mode);
    if (mode == 0 && oldMode > 0) {
        logV(NRSW_CTRL_TAG, "mNrSlot:%d, mPreferredDataSlot:%d", mNrSlot, mPreferredDataSlot);
        if (mNrSlot != mPreferredDataSlot) {
            int mainSlot = getNonSlotScopeStatusManager()->getIntValue(
                    RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT, 0);
            sp<RfxMessage> reqToRild = RfxMessage::obtainRequest(mainSlot,
                    RFX_MSG_REQUEST_NR_SWITCH, RfxIntsData(&mPreferredDataSlot, 1));
            requestToMcl(reqToRild);
        }
    }
}
