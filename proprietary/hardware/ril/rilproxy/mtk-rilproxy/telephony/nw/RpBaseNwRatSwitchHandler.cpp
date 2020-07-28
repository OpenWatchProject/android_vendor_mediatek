/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#include "RpBaseNwRatSwitchHandler.h"

/*****************************************************************************
 * Class RpBaseNwRatSwitchHandler
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpBaseNwRatSwitchHandler", RpBaseNwRatSwitchHandler, RfxObject);

RpBaseNwRatSwitchHandler::RpBaseNwRatSwitchHandler() :
        mNwRatController(NULL),
        mSlotId(-1) {
}

RpBaseNwRatSwitchHandler::RpBaseNwRatSwitchHandler(RpNwRatController* nwRatController) :
    mNwRatController(nwRatController) {
    mSlotId = mNwRatController->getSlotId();
}

RpBaseNwRatSwitchHandler::~RpBaseNwRatSwitchHandler() {
}

void RpBaseNwRatSwitchHandler::requestGetPreferredNetworkType(const RILD_RadioTechnology_Group radioTechType, const sp<RfxMessage>& msg) {
    int msg_id = RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE;
    sp<RfxMessage> message = RfxMessage::obtainRequest(radioTechType, msg_id, msg);
    mNwRatController->requestToRild(message);
}

void RpBaseNwRatSwitchHandler::requestGetVoiceRadioTech(const sp<RfxMessage>& msg) {
    int msg_id = RIL_REQUEST_VOICE_RADIO_TECH;
    sp<RfxMessage> message = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM, msg_id, msg);
    mNwRatController->requestToRild(message);
}

void RpBaseNwRatSwitchHandler::responseGetVoiceRadioTech(const sp<RfxMessage>& response) {
    int error = response->getError() ;
    if (error != RIL_E_SUCCESS) {
        // error
        mNwRatController->responseToRilj(response);
    } else {
        // ok
        int source = response->getSource();
        int32_t stgCount = 0;
        int32_t radioTech = 0;
        response->getParcel()->readInt32(&stgCount);
        response->getParcel()->readInt32(&radioTech);
        if (source == RADIO_TECH_GROUP_GSM) {
            sp<RfxMessage> resToRilj = RfxMessage::obtainResponse(RIL_E_SUCCESS, response);
            resToRilj->getParcel()->writeInt32(1);
            resToRilj->getParcel()->writeInt32(radioTech);
            mNwRatController->responseToRilj(resToRilj);
        }
    }
}

void RpBaseNwRatSwitchHandler::updatePhone(const ModemSettings mdSettings) {
    int tech = RADIO_TECH_UNKNOWN;
    switch (mdSettings.prefNwType) {
        case PREF_NET_TYPE_GSM_ONLY:
        case PREF_NET_TYPE_GSM_WCDMA:
        case PREF_NET_TYPE_GSM_WCDMA_AUTO:
        case PREF_NET_TYPE_LTE_GSM_WCDMA:
        case PREF_NET_TYPE_LTE_WCDMA:
        case PREF_NET_TYPE_LTE_GSM:
            tech = RADIO_TECH_GPRS;
            break;

        case PREF_NET_TYPE_WCDMA:
            tech =  RADIO_TECH_HSPA;
            break;

        case PREF_NET_TYPE_LTE_ONLY:
        case PREF_NET_TYPE_LTE_TDD_ONLY:
            tech = RADIO_TECH_LTE;
            break;

        case PREF_NET_TYPE_CDMA_ONLY:
        case PREF_NET_TYPE_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_EVDO_ONLY:
        case PREF_NET_TYPE_LTE_CDMA_EVDO:
            tech = RADIO_TECH_1xRTT;
            break;

        case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
        case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
            if (mdSettings.nwsMode == NWS_MODE_CDMALTE) {
                tech = RADIO_TECH_1xRTT;
            } else if (mdSettings.nwsMode == NWS_MODE_CSFB) {
                tech = RADIO_TECH_GPRS;
            }
            break;
    }
    if (tech > RADIO_TECH_UNKNOWN) {
        sp<RfxMessage> urcToRilj = RfxMessage::obtainUrc(mSlotId,
                RIL_UNSOL_VOICE_RADIO_TECH_CHANGED);
        urcToRilj->getParcel()->writeInt32(1);
        urcToRilj->getParcel()->writeInt32(tech);
        mNwRatController->responseToRilj(urcToRilj);
    }
}

void RpBaseNwRatSwitchHandler::updatePhone(const int prefNwType, const NwsMode nwsMode) {
    int tech = techFromPrefNwType(prefNwType);
    if ((prefNwType == PREF_NET_TYPE_LTE_ONLY || prefNwType == PREF_NET_TYPE_LTE_TDD_ONLY)
            && (nwsMode == NWS_MODE_CDMALTE)) {
        tech = RADIO_TECH_1xRTT;
    }
    if ((prefNwType == PREF_NET_TYPE_LTE_ONLY || prefNwType == PREF_NET_TYPE_LTE_TDD_ONLY)
            && (nwsMode == NWS_MODE_CSFB)) {
        tech = RADIO_TECH_LTE;
    }

    if (tech > RADIO_TECH_UNKNOWN) {
        sp<RfxMessage> urcToRilj = RfxMessage::obtainUrc(mSlotId,
                RIL_UNSOL_VOICE_RADIO_TECH_CHANGED);
        urcToRilj->getParcel()->writeInt32(1);
        urcToRilj->getParcel()->writeInt32(tech);
        mNwRatController->responseToRilj(urcToRilj);
    }
}

int RpBaseNwRatSwitchHandler::techFromPrefNwType(const int prefNwType) {
    switch (prefNwType) {
    case PREF_NET_TYPE_GSM_ONLY:
    case PREF_NET_TYPE_LTE_GSM:
        return RADIO_TECH_GPRS;

    case PREF_NET_TYPE_GSM_WCDMA:
    case PREF_NET_TYPE_GSM_WCDMA_AUTO:
    case PREF_NET_TYPE_WCDMA:
    case PREF_NET_TYPE_LTE_GSM_WCDMA:
    case PREF_NET_TYPE_LTE_WCDMA:
        return RADIO_TECH_HSPA;

    case PREF_NET_TYPE_LTE_ONLY:
    case PREF_NET_TYPE_LTE_TDD_ONLY:
        return RADIO_TECH_LTE;

    case PREF_NET_TYPE_CDMA_ONLY:
        return RADIO_TECH_1xRTT;

    case PREF_NET_TYPE_CDMA_EVDO_AUTO:
    case PREF_NET_TYPE_EVDO_ONLY:
    case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
    case PREF_NET_TYPE_LTE_CDMA_EVDO:
    case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
        return RADIO_TECH_EVDO_A;
    }
    return RADIO_TECH_UNKNOWN;
}
