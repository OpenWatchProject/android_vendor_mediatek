/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
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

#include "RpEmbmsAtController.h"
#include "RpEmbmsControllerProxy.h"
#include "RpEmbmsUtils.h"
#include "RfxLog.h"
#include <cutils/properties.h>
#include <telephony/mtk_ril.h>
#define RFX_LOG_TAG "RP_EMBMS_CON_PROXY"

#define RP_EMBMS_PROPERTY_ENABLE "persist.vendor.sys.embms.enable"

/*****************************************************************************
 * Class RpEmbmsControllerProxy
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpEmbmsControllerProxy", RpEmbmsControllerProxy, RfxController);

RpEmbmsControllerProxy::RpEmbmsControllerProxy() :
        mActiveEmbmsController(NULL) {
}

RpEmbmsControllerProxy::~RpEmbmsControllerProxy() {
}

void RpEmbmsControllerProxy::onInit() {
    RfxController::onInit();  // Required: invoke super class implementation

    RpEmbmsAtController *p;
    RFX_OBJ_CREATE(p, RpEmbmsAtController, this);
    mActiveEmbmsController = (RfxController *) p;
    logD(RFX_LOG_TAG, "onInit to TK-AT: ctrl = %p.", mActiveEmbmsController);

    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_RADIO_STATE,
        RfxStatusChangeCallback(this, &RpEmbmsControllerProxy::onRadioStateChanged));
}

void RpEmbmsControllerProxy::onDeinit() {
    logD(RFX_LOG_TAG, "onDeinit");
    RfxController::onDeinit();
}

void RpEmbmsControllerProxy::onRadioStateChanged(RfxStatusKeyEnum key,
    RfxVariant old_value, RfxVariant value) {
    char defaultData[PROPERTY_VALUE_MAX] = {0};
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);

    RIL_RadioState radioState = (RIL_RadioState) value.asInt();
    logD(RFX_LOG_TAG, "radioState %d", radioState);

    if (radioState == RADIO_STATE_ON) {
        property_get(RP_EMBMS_PROPERTY_ENABLE, defaultData, "");
        if (strlen(defaultData) == 0 || strcmp("0", defaultData) == 0) {
            return;
        }
        sp<RfxMessage> msg = RfxMessage::obtainRequest(getSlotId(),
                    RADIO_TECH_GROUP_GSM, RIL_LOCAL_REQUEST_EMBMS_ENABLE);
        Parcel* newParcel = msg->getParcel();
        newParcel->writeInt32(2);  // 2 parameters
        newParcel->writeInt32(0);  // trans_id
        newParcel->writeInt32(EMBMS_COMMAND_RIL);  // EMBMS_COMMAND_RIL
        requestToRild(msg);
    }
}
