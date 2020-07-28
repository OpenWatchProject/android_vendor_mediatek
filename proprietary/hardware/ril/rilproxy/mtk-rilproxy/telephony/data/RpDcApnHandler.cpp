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

#include "RpDcApnHandler.h"
#include "RpDataUtils.h"
#include "util/RpFeatureOptionUtils.h"

#ifdef RFX_LOG_TAG
#undef RFX_LOG_TAG
#endif
#define RFX_LOG_TAG "RP_DC_APN"

/*****************************************************************************
 * Class RpDcApnHandler
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpDcApnHandler", RpDcApnHandler, RfxController);

RpDcApnHandler::RpDcApnHandler() {
}

RpDcApnHandler::~RpDcApnHandler() {
}

void RpDcApnHandler::onInit() {
    RfxController::onInit();  // Required: invoke super class implementation
    logD(RFX_LOG_TAG, "onInit");
}

void RpDcApnHandler::onDeinit() {
    logD(RFX_LOG_TAG, "onDeinit");
    RfxController::onDeinit();
}

sp<RfxMessage> RpDcApnHandler::onSetInitialAttachApn(const sp<RfxMessage>& request) {
    Parcel* parcel = request->getParcel();
    request->resetParcelDataStartPos();
    String16 apn = parcel->readString16();  // apn
    String16 protocol = parcel->readString16();  // protocol
    String16 roamingProtocol = parcel->readString16();  // roamingProtocol
    int authType = parcel->readInt32();  // authType
    String16 username = parcel->readString16();  // username
    String16 password = parcel->readString16();  // password
    logD(RFX_LOG_TAG, "onSetInitialAttachApn apn:%s,protocol:%s,roamingProtocol:%s,authType:%d,username:%s,",
            String8(apn).string(), String8(protocol).string(), String8(roamingProtocol).string(),
            authType, String8(username).string());
    // For ctnet ia apn, remove usename and password
    // For the purpose of sharing apn between LTE and CDMA for CT 4G card,
    // we added username and password for CTNET APN(CDMA required),
    // but for the APN with username and password willn't be saved in ia cache.
    // This will casue that once set initial attach apn, ps will be detach and attach.
    if (apn == CTNET && username == CTNET_USER && password == CTNET_PASSWORD) {
        int supportedTypesBitmask = parcel->readInt32();
        int bearerBitmask = parcel->readInt32();
        int modemCognitive = parcel->readInt32();
        int mtu = parcel->readInt32();
        String16 mvnoType = parcel->readString16();
        String16 mvnoMatchData = parcel->readString16();
        int canHandleIms = parcel->readInt32();

        sp<RfxMessage> newMsg = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM, request->getId(),
                request, false);
        Parcel* newParcel = newMsg->getParcel();
        String16 emptyStr("");
        newParcel->writeString16(apn);
        newParcel->writeString16(protocol);
        newParcel->writeString16(roamingProtocol);
        newParcel->writeInt32(authType);
        newParcel->writeString16(emptyStr);
        newParcel->writeString16(emptyStr);
        newParcel->writeInt32(supportedTypesBitmask);
        newParcel->writeInt32(bearerBitmask);
        newParcel->writeInt32(modemCognitive);
        newParcel->writeInt32(mtu);
        newParcel->writeString16(mvnoType);
        newParcel->writeString16(mvnoMatchData);
        newParcel->writeInt32(canHandleIms);

        return newMsg;
    }
    return request;
}
