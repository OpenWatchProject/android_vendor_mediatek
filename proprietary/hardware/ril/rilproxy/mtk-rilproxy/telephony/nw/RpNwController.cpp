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
#include <log/log.h>
#include "RfxLog.h"
#include <cutils/properties.h>
#include <cutils/jstring.h>
#include "data/RpDataController.h"
#include "RfxStatusDefs.h"
#include "RpNwController.h"
#include "RpNwRatController.h"
#include "RpIrController.h"
#include "RpWpController.h"
#include <stdio.h>
#include "util/RpFeatureOptionUtils.h"

#undef LOG_TAG
#define LOG_TAG "RpNwController"
extern "C" int RatConfig_isC2kSupported();
/*****************************************************************************
 * Class RpNwController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpNwController", RpNwController, RfxController);

RpNwController::RpNwController() {
}

RpNwController::~RpNwController() {
}

void RpNwController::onInit() {
    // Required: invoke super class implementation
    RfxController::onInit();
    const int request_id_list[] =
            { RIL_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT,
            RIL_REQUEST_QUERY_AVAILABLE_NETWORKS,
            RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE};
    const int urc_id_list[] =
            { RIL_LOCAL_C2K_UNSOL_ENG_MODE_NETWORK_INFO,
            RIL_UNSOL_CDMA_PRL_CHANGED};
    registerToHandleRequest(request_id_list, (sizeof(request_id_list)/sizeof(int)));
    logD(LOG_TAG, " register REQUEST");
    if (RpFeatureOptionUtils::isC2kSupport()) {
        registerToHandleUrc(urc_id_list, (sizeof(urc_id_list)/sizeof(int)));
    }

    // create the object of RpNwRatController as the child controller
    // of this object (an instance of RpNwController)
    RpNwRatController* nw_rat_controller = NULL;
    RFX_OBJ_CREATE(nw_rat_controller, RpNwRatController, this);

    // create the object of RpWpController as the child controller
    // of this object (an instance of RpNwController)
    RpWpController *wp_controller;
    RFX_OBJ_CREATE(wp_controller, RpWpController, this);

    RpIrController *ir_controller;
    RFX_OBJ_CREATE(ir_controller, RpIrController, this);
    RpRadioController* radioController = (RpRadioController *) findController(
            getSlotId(), RFX_OBJ_CLASS_INFO(RpRadioController));
    // connect the signal defined by another module
    /// M: [Network][C2K]add for band8 desense. @{
    radioController->m_c2k_radio_changed_singal.connect(this,
            &RpNwController::onC2kRadioChanged);
    /// @}
}

void RpNwController::onDeinit() {
    // Required: invoke super class implementation
    RfxController::onDeinit();
}

bool RpNwController::onHandleRequest(const sp<RfxMessage>& message) {
    logD(LOG_TAG, " handle req %s (slot %d)",
            requestToString(message->getId()), getSlotId());

    switch (message->getId()) {
    case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT:
    case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS:
        notifyPlmnSearchState(PLMN_SEARCH_START);
        requestToRild(message);
        break;
    case RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE:
        requestToRild(message);
        break;
    default:
        break;
    }
    return true;
}

bool RpNwController::onHandleUrc(const sp<RfxMessage>& message) {
    logD(LOG_TAG, " handle urc %s (slot %d)",
            urcToString(message->getId()), getSlotId());

    switch (message->getId()) {
    case RIL_LOCAL_C2K_UNSOL_ENG_MODE_NETWORK_INFO:
        convertFromEiToNi(message);
        break;
    case RIL_UNSOL_CDMA_PRL_CHANGED:
        getPrlVersion(message);
        break;

    default:
        break;
    }
    return true;
}

bool RpNwController::onHandleResponse(const sp<RfxMessage>& message) {
    logD(LOG_TAG, " handle %s response (slot %d)",
            requestToString(message->getId()), getSlotId());

    switch (message->getId()) {
    case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT:
    case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS:
        notifyPlmnSearchState(PLMN_SEARCH_COMPLETE);
        responseToRilj(message);
        break;
    case RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE:
        checkCallState();
        if (!mIsListeningCall && !mIsListeningImsCall) {
            logD(LOG_TAG, "[BD] have no call, config band mode ");
            if (mLocalBandToken == message->getToken()) {
                mLocalBandToken = 0;
                configBandMode();
            } else {
                responseToRilj(message);
            }
        }
        break;
    default:
        break;
    }
    return true;
}

const char* RpNwController::requestToString(int reqId) {
    switch (reqId) {
    case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT:
        return "RIL_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT";
    case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS:
        return "RIL_REQUEST_QUERY_AVAILABLE_NETWORKS";
    case RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE:
        return "RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE";
    default:
        logD(LOG_TAG, " <UNKNOWN_REQUEST>");
        break;
    }
    return "";
}

const char* RpNwController::urcToString(int reqId) {
    switch (reqId) {
    case RIL_UNSOL_CDMA_PRL_CHANGED:
        return "RIL_UNSOL_CDMA_PRL_CHANGED";
        break;
    default:
        logD(LOG_TAG, " <UNKNOWN_URC>");
        break;
    }
    return "";
}

void RpNwController::notifyPlmnSearchState(int plmnSearchAction) {
    logD(LOG_TAG, "notifyPlmnSearchState: %d", plmnSearchAction);
    sp<RfxAction> action = new RfxAction0(this, &RpNwController::onConfigDcStateDone);
    RpDataController* dataController = (RpDataController *) findController(
            getSlotId(), RFX_OBJ_CLASS_INFO(RpDataController));
    // configure data connection state
    dataController->configDcStateForPlmnSearch(plmnSearchAction,
            action);
}

void RpNwController::convertFromEiToNi(const sp<RfxMessage>& message) {
    int source = message->getSource();
    Parcel *p = message->getParcel();

    logD(LOG_TAG, " convertFromEiToNi: source = %d", source);
    if (source == RADIO_TECH_GROUP_C2K) {
        int32_t size = 0;
        p->readInt32(&size);
        if (size <= 0) {
            logD(LOG_TAG, "[RpNwController] convertFromEiToNi: invalid message");
            return;
        }

        logD(LOG_TAG, " convertFromEiToNi: get urc message size = %d",
                size);
        char *pStrings[size];
        for(int i = 0; i< size; i++){
            pStrings[i] = strdupReadString(p);
            logD(LOG_TAG, " convertFromEiToNi: get data from urc, data = %s ",
                    pStrings[i]);
        }

        sp<RfxMessage> urcToRilj = RfxMessage::obtainUrc(getSlotId(),
                RIL_UNSOL_NETWORK_INFO);
        urcToRilj->getParcel()->writeInt32(2);
        writeStringToParcel(urcToRilj->getParcel(), Eng_Nw_Info_Type);
        writeStringToParcel(urcToRilj->getParcel(), pStrings[size- 1]);
        responseToRilj(urcToRilj);
    }
}

char *RpNwController::strdupReadString(Parcel *p) {
    size_t stringlen;
    const char16_t *s16;

    s16 = p->readString16Inplace(&stringlen);

    return strndup16to8(s16, stringlen);
}

void RpNwController::writeStringToParcel(Parcel *p, const char *s) {
    char16_t *s16;
    size_t s16_len = 0;
    s16 = strdup8to16(s, &s16_len);
    p->writeString16(s16, s16_len);
    free(s16);
}

void RpNwController::getPrlVersion(const sp<RfxMessage>& message) {
    Parcel *p = message->getParcel();
    int pos = p->dataPosition();
    int32_t num = p->readInt32();
    int32_t prlversion = p->readInt32();

    // get prlvalue
    char sPrlversionValue[32];
    sprintf(sPrlversionValue, "%d", prlversion);
    p->setDataPosition(pos);

    // get prlkey
    int slotId = getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_SOCKET_SLOT,
            RFX_SLOT_ID_UNKNOWN);
    char strSlotId[32];
    sprintf(strSlotId, "%d", slotId);
    char sPrlversionKey[32] = "vendor.cdma.prl.version";
    strncat(sPrlversionKey, strSlotId, 32 - strlen(sPrlversionKey) - 1);

    // set key and value
    property_set(sPrlversionKey, sPrlversionValue);

    logD(LOG_TAG, "getPrlVersion num=%d, prlversion=%d, sPrlversionvalue=%s, sPrlversionKey=%s",
            num, prlversion, sPrlversionValue, sPrlversionKey);
    responseToRilj(message);
}

/// M: [Network][C2K]add for band8 desense. @{
bool RpNwController::isGsmCard() {
    int cardType = getStatusManager(getSlotId())->getIntValue(
            RFX_STATUS_KEY_CARD_TYPE, -1);
    logD(LOG_TAG, "cardType is %d. ", cardType);
    bool is3gCdmaSim = getStatusManager()->getBoolValue(
            RFX_STATUS_KEY_CT3G_DUALMODE_CARD, false);
    if ((cardType & RFX_CARD_TYPE_SIM) > 0
            || ((cardType & RFX_CARD_TYPE_USIM) > 0
            && !is3gCdmaSim)) {
        logD(LOG_TAG, "slot %d is gsm card. ", getSlotId());
        return true;
    }
    return false;
}

int RpNwController::getCapabilitySlotId() {
    char tempstr[PROPERTY_VALUE_MAX];
    memset(tempstr, 0, sizeof(tempstr));
    property_get("persist.vendor.radio.simswitch", tempstr, "1");
    int capabilitySlotId = atoi(tempstr) - 1;
    logD(LOG_TAG, "getCapabilitySlotId, capability slot is %d .",capabilitySlotId);
    return capabilitySlotId;
}

void RpNwController::onC2kRadioChanged(int radioPower) {
    int isC2KSupport = RatConfig_isC2kSupported();

    char tempstr[PROPERTY_VALUE_MAX] = { 0 };
    memset(tempstr, 0, sizeof(tempstr));
    property_get("ro.vendor.mtk_c2k_lte_mode", tempstr, "0");
    int isSvlteSupport = atoi(tempstr) == 1 ? 1 : 0;

    logD(LOG_TAG, "[BD] isC2KSupport = %d, isSvlteSupport = %d, "
            "mC2kRadioPower = %d. ", isC2KSupport, isSvlteSupport,
            mC2kRadioPower);

    if (isC2KSupport && isSvlteSupport && !isOP09ASupport()
            && RFX_SLOT_COUNT == 2) {
        int cdma_lte_slot_id = getNonSlotScopeStatusManager()->getIntValue(
                RFX_STATUS_KEY_ACTIVE_CDMALTE_MODE_SLOT, -1);
        int currentSlotId = getSlotId();
        logD(LOG_TAG, "[BD] onC2kRadioChanged radioPower = %d, "
                "cdma_lte_slot_id = %d, mFirstRadioChange is %s. ", radioPower,
                cdma_lte_slot_id, mFirstRadioChange ? "true" : "false");

        RpNwController *another = (RpNwController *) findController(
                getSlotId() == 0 ? 1 : 0,
                RFX_OBJ_CLASS_INFO(RpNwController));
        if (cdma_lte_slot_id == currentSlotId
                && (radioPower != mC2kRadioPower || mFirstRadioChange)) {
            if (another->isGsmCard()) {
                mC2kRadioPower = radioPower;

                // register the listener.
                getStatusManager()->registerStatusChanged(
                        RFX_STATUS_KEY_NWS_MODE,
                        RfxStatusChangeCallback(this,
                                &RpNwController::onNwsModeChanged));
                getNonSlotScopeStatusManager()->registerStatusChanged(
                        RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT,
                        RfxStatusChangeCallback(this,
                                &RpNwController::onCapabilityChange));
                if (getCapabilitySlotId() == (getSlotId() == 0 ? 1 : 0)) {
                    mForceSwitch = 1;
                    logD(LOG_TAG, "[BD] another gsm card is the capability card,"
                                    "force switch band mode ");
                }
                queryAvaliableBandMode();
            } else {
                // unRegister the listener.
                getStatusManager()->unRegisterStatusChanged(
                        RFX_STATUS_KEY_NWS_MODE,
                        RfxStatusChangeCallback(this,
                                &RpNwController::onNwsModeChanged));
                getNonSlotScopeStatusManager()->unRegisterStatusChanged(
                        RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT,
                        RfxStatusChangeCallback(this,
                                &RpNwController::onCapabilityChange));
            }
        }

        if (mFirstRadioChange) {
            mFirstRadioChange = false;
        }
    }
}

void RpNwController::onNwsModeChanged(RfxStatusKeyEnum key,
        RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(key);
    logD(LOG_TAG, "[BD] onNwsModeChanged oldValue = %d, "
            "newValue = %d. ", oldValue.asInt(), newValue.asInt());
    int oldType = oldValue.asInt();
    int newType = newValue.asInt();
    if (oldType != newType) {
        logD(LOG_TAG, "[BD] onNwsModeChanged nws mode changed,"
                "config the band mode. ");
        configBandMode();
    }
}

void RpNwController::onCapabilityChange(RfxStatusKeyEnum key,
        RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(key);
    logD(LOG_TAG, "[BD] onCapabilityChange oldValue = %d, "
            "newValue = %d. ", oldValue.asInt(), newValue.asInt());
    int oldType = oldValue.asInt();
    int newType = newValue.asInt();
    if (oldType != newType) {
        logD(LOG_TAG, "[BD] onCapabilityChange capability slot"
                " changed, config the band mode. ");
        mForceSwitch = 1;
        queryAvaliableBandMode();
    }
}

void RpNwController::onConfigDcStateDone() {
    logD(LOG_TAG, "onConfigDcStateDone");
}

void RpNwController::queryAvaliableBandMode(){
    int msg_id = RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE;
    sp<RfxMessage> message = RfxMessage::obtainRequest(getSlotId(),
            RADIO_TECH_GROUP_GSM, msg_id);
    mLocalBandToken = message->getToken();
    requestToRild(message);
    logD(LOG_TAG, "[BD] Query Available band mode ");
}

void RpNwController::configBandMode(){
    int nwsMode = getStatusManager()->getIntValue(RFX_STATUS_KEY_NWS_MODE,
            NWS_MODE_CSFB);
    if (getCapabilitySlotId() == (getSlotId() == 0 ? 1 : 0)) {
        if (mC2kRadioPower) {
            // In roaming state, C2K is on, but need open band8.
            if (nwsMode == NWS_MODE_CSFB) {
                mBandMode = BM_FOR_DESENSE_RADIO_ON_ROAMING;
            } else {
                // In home, C2K is on, close band8.
                mBandMode = BM_FOR_DESENSE_RADIO_ON;
            }
        } else {
            // C2K off,need open band8
            mBandMode = BM_FOR_DESENSE_RADIO_OFF;
        }
    } else {
        //capability slot is not another gsm card.
        mBandMode = BM_FOR_DESENSE_B8_OPEN;
    }

    // Need check preferred network type when open band8
    if (mBandMode != BM_FOR_DESENSE_RADIO_ON) {
        RpNwRatController* capabilityNwRatController = (RpNwRatController *) findController(
                getCapabilitySlotId(), RFX_OBJ_CLASS_INFO(RpNwRatController));
        if (capabilityNwRatController->getLastPreferredNetworkType()
                == PREF_NET_TYPE_LTE_TDD_ONLY) {
            logD(LOG_TAG, "[BD] configBandMode return!!! Due to in LTE_TDD_ONLY mode");
            return;
        }
    }

    RpNwController *capabilityController = (RpNwController *) findController(
            getCapabilitySlotId(), RFX_OBJ_CLASS_INFO(RpNwController));
    capabilityController->setBandMode(mBandMode, mForceSwitch);
    mForceSwitch = 0;
}

void RpNwController::setBandMode(int bandMode, int forceSwitch) {
    int msg_id = RIL_REQUEST_SET_BAND_MODE;
    sp<RfxMessage> message = RfxMessage::obtainRequest(getSlotId(),
            RADIO_TECH_GROUP_GSM, msg_id);
    message->getParcel()->writeInt32(3);
    message->getParcel()->writeInt32(bandMode);
    message->getParcel()->writeInt32(forceSwitch);
    message->getParcel()->writeInt32(0);
    logD(LOG_TAG, "[BD] setBandMode: bandMode = %d, forceSwitch = %d. ",
            bandMode, forceSwitch);
    requestToRild(message);
}

/**
* Check if there is a ongoing call. If call is ongoing ,should listen it's status. Otherwise config band mode.
*/
void RpNwController::checkCallState() {
    for (int slotId = 0; slotId < RFX_SLOT_COUNT; slotId++) {
        bool isCalling = (getStatusManager(slotId)->getIntValue(
                RFX_STATUS_KEY_VOICE_CALL_COUNT, 0) > 0);
        bool isImsCalling = getStatusManager(slotId)->getBoolValue(
                RFX_STATUS_KEY_IMS_IN_CALL, false);
        if (isCalling > 0) {
            mIsListeningCall = true;
            getStatusManager(slotId)->registerStatusChangedEx(
                    RFX_STATUS_KEY_VOICE_CALL_COUNT,
                    RfxStatusChangeCallbackEx(this, &RpNwController::onCallCountChanged));
            logD(LOG_TAG, "[BD] calling , need wait call ended ");
            break;
        }
        if (isImsCalling > 0) {
            mIsListeningImsCall = true;
            getStatusManager(slotId)->registerStatusChangedEx(
                    RFX_STATUS_KEY_IMS_IN_CALL,
                    RfxStatusChangeCallbackEx(this, &RpNwController::onImsCallCountChanged));
            logD(LOG_TAG, "[BD] IMS calling , need wait call ended ");
            break;
        }
    }
}

void RpNwController::onCallCountChanged(int slotId, RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    int callCount = value.asInt();
    // When current call finished, execute pended mode switch.
    if (callCount == 0) {
        cancelListeningCallStatus();
        logD(LOG_TAG, "[BD][onCallCountChanged] Slot %d call finished", slotId);
        queryAvaliableBandMode();
    }
}

void RpNwController::onImsCallCountChanged(int slotId, RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    // When current call finished, execute pended mode switch.
    if (value.asBool() == false) {
        cancelListeningImsCallStatus();
        logD(LOG_TAG, "[BD][onImsCallCountChanged] Slot %d call finished", slotId);
        queryAvaliableBandMode();
    }
}

void RpNwController::cancelListeningCallStatus() {
    mIsListeningCall = false;
    for (int slotId = 0; slotId < RFX_SLOT_COUNT; slotId++) {
        getStatusManager(slotId)->unRegisterStatusChangedEx(RFX_STATUS_KEY_VOICE_CALL_COUNT,
            RfxStatusChangeCallbackEx(this, &RpNwController::onCallCountChanged));
    }
}

void RpNwController::cancelListeningImsCallStatus() {
    mIsListeningImsCall = false;
    for (int slotId = 0; slotId < RFX_SLOT_COUNT; slotId++) {
        getStatusManager(slotId)->unRegisterStatusChangedEx(RFX_STATUS_KEY_IMS_IN_CALL,
            RfxStatusChangeCallbackEx(this, &RpNwController::onImsCallCountChanged));
    }
}
/// @}