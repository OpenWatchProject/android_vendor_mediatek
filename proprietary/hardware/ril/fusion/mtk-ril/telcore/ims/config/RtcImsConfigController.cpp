/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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
#include <stdlib.h>
#include <string>
#include <memory>
#include <sstream>

#include "RtcImsConfigController.h"
#include "RtcImsConfigDef.h"
#include "ImsConfigUtils.h"
#include "rfx_properties.h"
#include "RfxBaseHandler.h"
#include "RfxIntsData.h"
#include "RfxVoidData.h"
#include "RfxStringData.h"
#include "RfxStringsData.h"
#include "ImsConfigDataHelper.h"
#include "RfxMainThread.h"

#include <cutils/properties.h>
#include <mtkconfigutils.h>

using ::android::String8;

#define RFX_LOG_TAG "RtcImsConfigController"

#define AP_WIFI_ONLY           0
#define AP_CELLULAR_PREFERRED  1
#define AP_WIFI_PREFERRED      2
#define AP_CELLULAR_ONLY       3
#define AP_4G_PREFER           4

#define RIL_4G_PREFER          4
#define RIL_WIFI_ONLY          3
#define RIL_CELLULAR_PREFERRED 2
#define RIL_WIFI_PREFERRED     1

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RtcImsConfigController", RtcImsConfigController, RfxController);

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_IMS_CONFIG_SET_FEATURE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxIntsData, RFX_MSG_REQUEST_IMS_CONFIG_GET_FEATURE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_IMS_CONFIG_SET_PROVISION);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxStringData, RFX_MSG_REQUEST_IMS_CONFIG_GET_PROVISION);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxIntsData, RFX_MSG_REQUEST_IMS_CONFIG_GET_RESOURCE_CAP);
RFX_REGISTER_DATA_TO_URC_ID(RfxVoidData, RFX_MSG_UNSOL_IMS_CONFIG_DYNAMIC_IMS_SWITCH_COMPLETE);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_UNSOL_IMS_CONFIG_FEATURE_CHANGED);
RFX_REGISTER_DATA_TO_URC_ID(RfxStringsData, RFX_MSG_UNSOL_IMS_CONFIG_CONFIG_CHANGED);
RFX_REGISTER_DATA_TO_URC_ID(RfxVoidData, RFX_MSG_UNSOL_IMS_CONFIG_CONFIG_LOADED);

void RtcImsConfigController::onInit() {
    RfxController::onInit();  // Required: invoke super class implementation

    if (DEBUG) logD(RFX_LOG_TAG, "onInit()");

    DEBUG = isLogEnable();

    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_SIM_STATE,
            RfxStatusChangeCallback(this, &RtcImsConfigController::onSimStateChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_RADIO_STATE,
            RfxStatusChangeCallback(this, &RtcImsConfigController::onRadioStateChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_CARRIER_CONFIG_CHANGED,
            RfxStatusChangeCallback(this, &RtcImsConfigController::onCarrierConfigChanged));

    resetFeatureSendCmd();
    resetFeatureResource();
    resetFeatureValue();
    resetFeatureSendValue();

    const int request_id_list[] = {
            RFX_MSG_REQUEST_IMS_CONFIG_SET_FEATURE,
            RFX_MSG_REQUEST_IMS_CONFIG_GET_FEATURE,
            RFX_MSG_REQUEST_IMS_CONFIG_SET_PROVISION,
            RFX_MSG_REQUEST_IMS_CONFIG_GET_PROVISION,
            RFX_MSG_REQUEST_IMS_CONFIG_GET_MD_PROVISION,
            RFX_MSG_REQUEST_IMS_CONFIG_GET_RESOURCE_CAP
    };
    const int urc_id_list[] = {
            RFX_MSG_UNSOL_GET_PROVISION_DONE
    };

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(const int));
    registerToHandleUrc(urc_id_list, sizeof(urc_id_list)/sizeof(const int));
}


void RtcImsConfigController::onDeinit() {
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_SIM_STATE,
            RfxStatusChangeCallback(this, &RtcImsConfigController::onSimStateChanged));
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_RADIO_STATE,
            RfxStatusChangeCallback(this, &RtcImsConfigController::onRadioStateChanged));
    getStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_CARRIER_CONFIG_CHANGED,
            RfxStatusChangeCallback(this, &RtcImsConfigController::onCarrierConfigChanged));
    RfxController::onDeinit();
}

void RtcImsConfigController::initProvisionValue() {
    int mccMnc = getCurrentMccMnc();
    int operatorId = ImsConfigUtils::getOperatorId(mccMnc);

    if (DEBUG)
        logD(RFX_LOG_TAG, "initProvisionValue, mccMnc = %d, operatorId = %d", mccMnc, operatorId);

    mProvisionValue.clear();

    vector<shared_ptr<ConfigValue>> data =
            ImsConfigDataHelper::getConfigData(operatorId);

    for (int i = 0; i < (int)data.size(); i++) {
        mProvisionValue[data[i]->configId] = data[i]->provsionValue;
    }

    mInitDone = true;

    if (DEBUG) logD(RFX_LOG_TAG, "send config loaded urc");
    sp<RfxMessage> urc = RfxMessage::obtainUrc(getSlotId(),
                                               RFX_MSG_UNSOL_IMS_CONFIG_CONFIG_LOADED,
                                               RfxVoidData());
    responseToRilj(urc);
}

bool RtcImsConfigController::onHandleRequest(const sp<RfxMessage>& message){
    int msg_id = message->getId();
    if (DEBUG) logD(RFX_LOG_TAG, "onHandleRequest - %s", RFX_ID_TO_STR(msg_id));
    switch (msg_id) {
        case RFX_MSG_REQUEST_IMS_CONFIG_SET_FEATURE:
            setFeatureValue(message);
            break;
        case RFX_MSG_REQUEST_IMS_CONFIG_GET_FEATURE:
            getFeatureValue(message);
            break;
        case RFX_MSG_REQUEST_IMS_CONFIG_SET_PROVISION:
            setProvisionValue(message);
            break;
        case RFX_MSG_REQUEST_IMS_CONFIG_GET_PROVISION:
            getProvisionValue(message);
            break;
        case RFX_MSG_REQUEST_IMS_CONFIG_GET_RESOURCE_CAP:
            getImsResourceCapability(message);
            break;
        default:
            break;
    }

    return true;
}

bool RtcImsConfigController::onHandleUrc(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    logD(RFX_LOG_TAG, "onHandleUrc - %s", RFX_ID_TO_STR(msg_id));
    switch (msg_id) {
        case RFX_MSG_UNSOL_GET_PROVISION_DONE:
            handleGetProvisionUrc(message);
            break;
        default:
            responseToRilj(message);
    }
    return true;
}

bool RtcImsConfigController::onHandleResponse(const sp<RfxMessage>& message){
    int msg_id = message->getId();
    logD(RFX_LOG_TAG, "onHandleResponse - %s", RFX_ID_TO_STR(msg_id));
    switch (msg_id) {
        case RFX_MSG_REQUEST_IMS_CONFIG_GET_MD_PROVISION:
            handleGetProvisionResponse(message);
            break;
        case RFX_MSG_REQUEST_IMS_CONFIG_SET_PROVISION:
            handleSetProvisionResponse(message);
            break;
        default:
            responseToRilj(message);
    }
    return true;
}

bool RtcImsConfigController::onCheckIfRejectMessage(const sp<RfxMessage>& message,
            bool isModemPowerOff, int radioState) {
    RFX_UNUSED(isModemPowerOff);

    int msg_id = message->getId();
    int slot_id = message->getSlotId();

    if (msg_id == RFX_MSG_REQUEST_IMS_CONFIG_SET_FEATURE ||
        msg_id == RFX_MSG_REQUEST_IMS_CONFIG_GET_FEATURE ||
        msg_id == RFX_MSG_REQUEST_IMS_CONFIG_SET_PROVISION ||
        msg_id == RFX_MSG_REQUEST_IMS_CONFIG_GET_PROVISION ||
        msg_id == RFX_MSG_REQUEST_IMS_CONFIG_GET_RESOURCE_CAP) {
        return false;
    }
    return RfxController::onCheckIfRejectMessage(message, isModemPowerOff, radioState);
}

void RtcImsConfigController::onSimStateChanged(RfxStatusKeyEnum key, RfxVariant old_value,
        RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);

    int simState = value.asInt();
    logD(RFX_LOG_TAG, "onSimStateChanged, simState = %d", simState);

    if (simState == RFX_SIM_STATE_ABSENT || simState == RFX_SIM_STATE_PIN_REQUIRED || simState ==
            RFX_SIM_STATE_PUK_REQUIRED || simState == RFX_SIM_STATE_NETWORK_LOCKED) {
        mECCAllowSendCmd = true;

        int slot_id = getSlotId();
        int volte = ImsConfigUtils::getFeaturePropValue(ImsConfigUtils::PROPERTY_VOLTE_ENALBE,
                                                        slot_id);
        int vilte = ImsConfigUtils::getFeaturePropValue(ImsConfigUtils::PROPERTY_VILTE_ENALBE,
                                                        slot_id);
        int wfc = ImsConfigUtils::getFeaturePropValue(ImsConfigUtils::PROPERTY_WFC_ENALBE, slot_id);
        if (volte == 1 && vilte == 1 && wfc == 1) {
            mECCAllowNotify = true;
        }
    }

    if(simState == RFX_SIM_STATE_ABSENT || simState == RFX_SIM_STATE_NOT_READY || simState ==
            RFX_SIM_STATE_PIN_REQUIRED || simState == RFX_SIM_STATE_PUK_REQUIRED || simState ==
            RFX_SIM_STATE_NETWORK_LOCKED) {
        resetFeatureSendCmd();
    }

    if(simState != RFX_SIM_STATE_READY) {
        // For SIM state ready, will wait for onCarrierConfigChanged()
        processDynamicImsSwitch();
    }
}

void RtcImsConfigController::processDynamicImsSwitch() {
    if (ImsConfigUtils::getSystemPropValue(ImsConfigUtils::PROPERTY_DYNAMIC_IMS_SWITCH_SUPPORT) !=
        1) {
        logD(RFX_LOG_TAG, "Do not support DYNAMIC IMS SWITCH, return!");
        return;
    }

    int currentMccMnc  = getCurrentMccMnc();

    if (DEBUG)
        logD(RFX_LOG_TAG, "processDynamicImsSwitch, currentMccMnc = %d", currentMccMnc);

    // Dynamic SIM Switch start
    int volte = FEATURE_VALUE_OFF;
    int vilte = FEATURE_VALUE_OFF;
    int vowifi = FEATURE_VALUE_OFF;
    int viwifi = FEATURE_VALUE_OFF;
    int vonr = FEATURE_VALUE_OFF;
    int vinr = FEATURE_VALUE_OFF;

    if (mMccmnc != currentMccMnc) {
        mMccmnc = currentMccMnc;

        if (mMccmnc == 0) {
            volte = FEATURE_VALUE_ON;
            vilte = FEATURE_VALUE_ON;
        } else {
            if (!ImsConfigUtils::isTestSim(getSlotId())) {
                volte = getStatusManager()->getString8Value(
                        RFX_STATUS_KEY_CONFIG_DEVICE_VOLTE_AVAILABLE) == "1";
                vilte = getStatusManager()->getString8Value(
                        RFX_STATUS_KEY_CONFIG_DEVICE_VT_AVAILABLE) == "1";
                vowifi = getStatusManager()->getString8Value(
                        RFX_STATUS_KEY_CONFIG_DEVICE_WFC_AVAILABLE) == "1";
                viwifi = vilte; // Currently, only one DIMS config for ViLTE & ViWifi.
                vonr = getStatusManager()->getString8Value(
                        RFX_STATUS_KEY_CONFIG_DEVICE_VONR_AVAILABLE) == "1";
                vinr = getStatusManager()->getString8Value(
                        RFX_STATUS_KEY_CONFIG_DEVICE_VINR_AVAILABLE) == "1";

                if (DEBUG)
                    logD(RFX_LOG_TAG, "DIMS volte:%d, vilte:%d, wfc:%d, vonr:%d, vinr:%d",
                            volte, vilte, vowifi, vonr, vinr);

                string iccid = ImsConfigUtils::getSystemPropStringValue(
                        ImsConfigUtils::PROPERTY_ICCID_SIM[getSlotId()]);
                if (DEBUG)
                    logD(RFX_LOG_TAG, "processDynamicImsSwitch, iccid = %s",
                            RfxRilUtils::pii(RFX_LOG_TAG, iccid.c_str()));

                int ctMccmnc = mMccmnc;
                if (!iccid.empty()) {
                    int mcc, mnc;
                    const string APTG_ICCID = "8988605";
                    const string CT_ICCID_1 = "898603";
                    const string CT_ICCID_2 = "898611";
                    if (iccid.compare(0, APTG_ICCID.size(), APTG_ICCID) == 0) {
                        mcc = 466;
                        mnc = 5;
                        // special case for 46605
                        logD(RFX_LOG_TAG, "special case for APTG");
                        volte = FEATURE_VALUE_ON;
                        vilte = FEATURE_VALUE_ON;
                        vowifi = FEATURE_VALUE_ON;
                        viwifi = FEATURE_VALUE_ON;
                    } else if (mMccmnc == 20404
                            && (iccid.compare(0, CT_ICCID_1.size(), CT_ICCID_1) == 0 ||
                                iccid.compare(0, CT_ICCID_2.size(), CT_ICCID_2) == 0)) {
                        ctMccmnc = 46003;
                        // special case for 46003 in case some CT sim use 20404 to get wrong result
                        logD(RFX_LOG_TAG, "special case for CT");
                        volte = FEATURE_VALUE_ON;
                        vilte = FEATURE_VALUE_ON;
                    }
                }

                if (ImsConfigUtils::isCtVolteDisabled(ctMccmnc)) {
                    logD(RFX_LOG_TAG, "CT VOLTE disabled case...");
                    volte = FEATURE_VALUE_OFF;
                    vilte = FEATURE_VALUE_OFF;
                    vowifi = FEATURE_VALUE_OFF;
                    viwifi = FEATURE_VALUE_OFF;
                    vonr = FEATURE_VALUE_OFF;
                    vinr = FEATURE_VALUE_OFF;
                }
            } else {
                volte = FEATURE_VALUE_ON;
                vilte = FEATURE_VALUE_ON;
                vowifi = FEATURE_VALUE_ON;
                viwifi = FEATURE_VALUE_ON;
                vonr = FEATURE_VALUE_ON;
                vinr = FEATURE_VALUE_ON;
            }
        }

        logD(RFX_LOG_TAG,
                "setImsResourceCapability, volte:%d, vilte:%d, wfc:%d, vonr:%d, vinr:%d",
                volte, vilte, vowifi, vonr, vinr);

        setImsResourceCapability(FEATURE_TYPE_VOICE_OVER_LTE, volte, false);
        setImsResourceCapability(FEATURE_TYPE_VIDEO_OVER_LTE, vilte, false);
        setImsResourceCapability(FEATURE_TYPE_VOICE_OVER_WIFI, vowifi, false);
        setImsResourceCapability(FEATURE_TYPE_VIDEO_OVER_WIFI, viwifi, false);
        setImsResourceCapability(FEATURE_TYPE_VOICE_OVER_NR, vonr, false);
        setImsResourceCapability(FEATURE_TYPE_VIDEO_OVER_NR, vinr, true);
    }
}

void RtcImsConfigController::onCarrierConfigChanged(RfxStatusKeyEnum key,
                                               RfxVariant old_value, RfxVariant value) {
    logD(RFX_LOG_TAG, "onCarrierConfigChanged()");
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    RFX_UNUSED(value);

    resetFeatureResource();
    resetFeatureValue();
    initProvisionValue();

    loadProvisionedValue();

    processDynamicImsSwitch();
}

void RtcImsConfigController::onRadioStateChanged(RfxStatusKeyEnum key, RfxVariant old_value,
        RfxVariant value) {
    int oldState = -1, newState = -1;

    RFX_UNUSED(key);
    oldState = old_value.asInt();
    newState = value.asInt();

    logD(RFX_LOG_TAG, "onRadioStateChanged, state = %d", newState);

    if (newState != RADIO_STATE_UNAVAILABLE) {
        int slot_id = getSlotId();
        int currentMccmnc = getCurrentMccMnc();
        String8 currentIccid = getStatusManager()->getString8Value(RFX_STATUS_KEY_SIM_ICCID);

        if (DEBUG)
            logD(RFX_LOG_TAG, "onRadioStateChanged, currentIccid: %s, mSendCfgIccid: %s",
                    RfxRilUtils::pii(RFX_LOG_TAG, currentIccid.string()),
                    RfxRilUtils::pii(RFX_LOG_TAG, mSendCfgIccid.string()));

        if (DEBUG)
            logD(RFX_LOG_TAG, "onRadioStateChanged, currentMccmnc: %d, mSendCfgMccmnc: %d",
                    currentMccmnc, mSendCfgMccmnc);

        if ((mSendCfgMccmnc != -1) && !mSendCfgIccid.isEmpty() &&
                (currentIccid.compare(mSendCfgIccid) == 0) && (currentMccmnc == mSendCfgMccmnc)) {
            triggerImsCfgCommand(slot_id);
        } else {
            resetFeatureSendCmd();
        }
    }
    /// M: ALPS04270086 airplane mode ECC issue. @{
    int simState = getStatusManager()->getIntValue(RFX_STATUS_KEY_SIM_STATE);
    if (newState == RADIO_STATE_ON && (simState == RFX_SIM_STATE_ABSENT || simState ==
            RFX_SIM_STATE_PIN_REQUIRED || simState == RFX_SIM_STATE_PUK_REQUIRED || simState ==
            RFX_SIM_STATE_NETWORK_LOCKED)) {
        mECCAllowSendCmd = true;
        logD(RFX_LOG_TAG, "onRadioStateChanged, mECCAllowSendCmd = true");
    }
    /// @}
}

string RtcImsConfigController::convertToString(int value) {
    std::ostringstream os;
    os << value;
    return os.str();
}

void RtcImsConfigController::resetFeatureSendCmd(){
    mFeatureSendCmd[FEATURE_TYPE_VOICE_OVER_LTE] = false;
    mFeatureSendCmd[FEATURE_TYPE_VIDEO_OVER_LTE] = false;
    mFeatureSendCmd[FEATURE_TYPE_VOICE_OVER_WIFI] = false;
    mFeatureSendCmd[FEATURE_TYPE_VIDEO_OVER_WIFI] = false;
    mFeatureSendCmd[FEATURE_TYPE_VOICE_OVER_NR] = false;
    mFeatureSendCmd[FEATURE_TYPE_VIDEO_OVER_NR] = false;
}

void RtcImsConfigController::resetFeatureResource(){
    mMccmnc = -1;
    mFeatureResource[FEATURE_TYPE_VOICE_OVER_LTE] = FEATURE_RESOURCE_OFF;
    mFeatureResource[FEATURE_TYPE_VIDEO_OVER_LTE] = FEATURE_RESOURCE_OFF;
    mFeatureResource[FEATURE_TYPE_VOICE_OVER_WIFI] = FEATURE_RESOURCE_OFF;
    mFeatureResource[FEATURE_TYPE_VIDEO_OVER_WIFI] = FEATURE_RESOURCE_OFF;
    mFeatureResource[FEATURE_TYPE_VOICE_OVER_NR] = FEATURE_RESOURCE_OFF;
    mFeatureResource[FEATURE_TYPE_VIDEO_OVER_NR] = FEATURE_RESOURCE_OFF;
}

void RtcImsConfigController::resetFeatureValue(){
    int slot_id = getSlotId();

    mVoLteFeatureValue.clear();
    mViLteFeatureValue.clear();
    mVoWifiFeatureValue.clear();
    mViWifiFeatureValue.clear();
    mVoNrFeatureValue.clear();
    mViNrFeatureValue.clear();

    mVoLteFeatureValue[NETWORK_TYPE_LTE] = ImsConfigUtils::getFeaturePropValue(
            ImsConfigUtils::PROPERTY_VOLTE_ENALBE, slot_id);
    mViLteFeatureValue[NETWORK_TYPE_LTE] = ImsConfigUtils::getFeaturePropValue(
            ImsConfigUtils::PROPERTY_VILTE_ENALBE, slot_id);
    mVoWifiFeatureValue[NETWORK_TYPE_LTE] = ImsConfigUtils::getFeaturePropValue(
            ImsConfigUtils::PROPERTY_WFC_ENALBE, slot_id);
    mViWifiFeatureValue[NETWORK_TYPE_LTE] = ImsConfigUtils::getFeaturePropValue(
            ImsConfigUtils::PROPERTY_VIWIFI_ENALBE, slot_id);
    mVoNrFeatureValue[NETWORK_TYPE_LTE] = ImsConfigUtils::getFeaturePropValue(
            ImsConfigUtils::PROPERTY_VONR_ENALBE, slot_id);
    mViNrFeatureValue[NETWORK_TYPE_LTE] = ImsConfigUtils::getFeaturePropValue(
            ImsConfigUtils::PROPERTY_VINR_ENALBE, slot_id);
}

void RtcImsConfigController::resetFeatureSendValue(){
    mFeatureSendValue[FEATURE_TYPE_VOICE_OVER_LTE] = FEATURE_VALUE_OFF;
    mFeatureSendValue[FEATURE_TYPE_VIDEO_OVER_LTE] = FEATURE_VALUE_OFF;
    mFeatureSendValue[FEATURE_TYPE_VOICE_OVER_WIFI] = FEATURE_VALUE_OFF;
    mFeatureSendValue[FEATURE_TYPE_VIDEO_OVER_WIFI] = FEATURE_VALUE_OFF;
    mFeatureSendValue[FEATURE_TYPE_VOICE_OVER_NR] = FEATURE_VALUE_OFF;
    mFeatureSendValue[FEATURE_TYPE_VIDEO_OVER_NR] = FEATURE_VALUE_OFF;
}

void RtcImsConfigController::setFeatureValue(const sp<RfxMessage>& message){
    int slot_id = getSlotId();
    int *params = (int *)message->getData()->getData();
    int feature_id = params[0];
    int network = params[1];
    int value = params[2];

    if (DEBUG)
        logD(RFX_LOG_TAG, "setFeatureValue, feature_id:%d,network:%d,value:%d,isLast:%d",
                feature_id, network, value, params[3]);

    if (feature_id == FEATURE_TYPE_VOICE_OVER_LTE) {
        mVoLteFeatureValue[network] = value;
    } else if (feature_id == FEATURE_TYPE_VIDEO_OVER_LTE) {
        mViLteFeatureValue[network] = value;
    } else if (feature_id == FEATURE_TYPE_VOICE_OVER_WIFI) {
        mVoWifiFeatureValue[network] = value;
    } else if (feature_id == FEATURE_TYPE_VIDEO_OVER_WIFI) {
        mViWifiFeatureValue[network] = value;
    } else if (feature_id == FEATURE_TYPE_VOICE_OVER_NR) {
        mVoNrFeatureValue[network] = value;
    } else if (feature_id == FEATURE_TYPE_VIDEO_OVER_NR) {
        mViNrFeatureValue[network] = value;
    } else {
        logD(RFX_LOG_TAG, "setFeatureValue, feature_id:%d, value:%d not support",
             feature_id, value);
    }

    handleSetFeatureValue(true, message);
}

void RtcImsConfigController::handleSetFeatureValue(bool success, const sp<RfxMessage>& message) {
    int slot_id = getSlotId();
    int *params = (int *)message->getData()->getData();
    int featureId = params[0];
    int network = params[1];
    int value = params[2];
    int isLast = params[3];

    int simState = getStatusManager()->getIntValue(RFX_STATUS_KEY_SIM_STATE);
    bool isAllowSendAT = false;
    int oldFeatureValue = 0;

    if (DEBUG)
        logD(RFX_LOG_TAG, "handleSetFeatureValue(), featureId:%d, network:%d, value:%d, isLast:%d",
                featureId, network, value, isLast);

    if (DEBUG)
        logD(RFX_LOG_TAG, "handleSetFeatureValue(), success:%d, simState:%d", success, simState);

    if (simState == RFX_SIM_STATE_READY) {
        if (mFeatureSendCmd[featureId]) {
            switch(featureId) {
                case FEATURE_TYPE_VOICE_OVER_LTE:
                    oldFeatureValue = ImsConfigUtils::getFeaturePropValue(
                            ImsConfigUtils::PROPERTY_VOLTE_ENALBE,
                            slot_id);
                    break;
                case FEATURE_TYPE_VIDEO_OVER_LTE:
                    oldFeatureValue = ImsConfigUtils::getFeaturePropValue(
                            ImsConfigUtils::PROPERTY_VILTE_ENALBE,
                            slot_id);
                    break;
                case FEATURE_TYPE_VOICE_OVER_WIFI:
                    oldFeatureValue = ImsConfigUtils::getFeaturePropValue(
                            ImsConfigUtils::PROPERTY_WFC_ENALBE,
                            slot_id);
                    break;
                case FEATURE_TYPE_VIDEO_OVER_WIFI:
                    oldFeatureValue = ImsConfigUtils::getFeaturePropValue(
                            ImsConfigUtils::PROPERTY_VIWIFI_ENALBE,
                            slot_id);
                    break;
                case FEATURE_TYPE_VOICE_OVER_NR:
                    oldFeatureValue = ImsConfigUtils::getFeaturePropValue(
                            ImsConfigUtils::PROPERTY_VONR_ENALBE,
                            slot_id);
                    break;
                case FEATURE_TYPE_VIDEO_OVER_NR:
                    oldFeatureValue = ImsConfigUtils::getFeaturePropValue(
                            ImsConfigUtils::PROPERTY_VINR_ENALBE,
                            slot_id);
                    break;
                default:
                    break;
            }

            if (DEBUG)
                logD(RFX_LOG_TAG,
                     "Already send feature %d once, compare feature value old: %d, new: %d",
                     featureId,
                     oldFeatureValue, value);

            if (value != oldFeatureValue || value != mFeatureSendValue[featureId]) {
                isAllowSendAT = true;
            }
        } else {
            isAllowSendAT = true;
            mFeatureSendCmd[featureId] = true;
        }
    } else if (simState == RFX_SIM_STATE_ABSENT || simState == RFX_SIM_STATE_PIN_REQUIRED ||
            simState == RFX_SIM_STATE_PUK_REQUIRED || simState == RFX_SIM_STATE_NETWORK_LOCKED) {
        if (DEBUG) logD(RFX_LOG_TAG, "ECCAllow : %d", mECCAllowSendCmd);
        if (mECCAllowSendCmd) {
            isAllowSendAT = ((value == FEATURE_VALUE_ON) &&
                    ((featureId == FEATURE_TYPE_VOICE_OVER_LTE) ||
                     (featureId == FEATURE_TYPE_VOICE_OVER_NR)));
            if (isAllowSendAT) {
                logD(RFX_LOG_TAG, "Allow to send enable VoLTE AT cmd once for ECC");
                mECCAllowSendCmd = false;
            }
        }
    }

    bool forceNotify = ((featureId == FEATURE_TYPE_VOICE_OVER_LTE ||
                         featureId == FEATURE_TYPE_VIDEO_OVER_LTE ||
                         featureId == FEATURE_TYPE_VOICE_OVER_NR ||
                         featureId == FEATURE_TYPE_VIDEO_OVER_NR) &&
                     ImsConfigUtils::isAllowForceNotify(getSlotId(), value));
    if (!isAllowSendAT) {
        isAllowSendAT = forceNotify;
    }

    if (DEBUG)
        logD(RFX_LOG_TAG,
             "Check if send AT cmd, feature = %d, value = %d, simState = %d, isAllowSendAT = %d",
             featureId, value, simState, isAllowSendAT);

    if (isAllowSendAT || (isLast == ISLAST_TRUE)) {
        if (isAllowSendAT) {
            char isEnable[10];
            sprintf(isEnable,"%d",value);

            switch(featureId) {
                case FEATURE_TYPE_VOICE_OVER_LTE:
                    ImsConfigUtils::setFeaturePropValue(
                            ImsConfigUtils::PROPERTY_VOLTE_ENALBE,
                            isEnable,
                            slot_id);
                    break;
                case FEATURE_TYPE_VIDEO_OVER_LTE:
                    ImsConfigUtils::setFeaturePropValue(
                            ImsConfigUtils::PROPERTY_VILTE_ENALBE,
                            isEnable,
                            slot_id);
                    break;
                case FEATURE_TYPE_VOICE_OVER_WIFI:
                    ImsConfigUtils::setFeaturePropValue(
                            ImsConfigUtils::PROPERTY_WFC_ENALBE,
                            isEnable,
                            slot_id);
                    break;
                case FEATURE_TYPE_VIDEO_OVER_WIFI:
                    ImsConfigUtils::setFeaturePropValue(
                            ImsConfigUtils::PROPERTY_VIWIFI_ENALBE,
                            isEnable,
                            slot_id);
                    break;
                case FEATURE_TYPE_VOICE_OVER_NR:
                    ImsConfigUtils::setFeaturePropValue(
                            ImsConfigUtils::PROPERTY_VONR_ENALBE,
                            isEnable,
                            slot_id);
                    break;
                case FEATURE_TYPE_VIDEO_OVER_NR:
                    ImsConfigUtils::setFeaturePropValue(
                            ImsConfigUtils::PROPERTY_VINR_ENALBE,
                            isEnable,
                            slot_id);
                    break;
                default:
                    break;
            }
        }

        mFeatureSendValue[featureId] = value;

        if (isLast != ISLAST_FALSE) {
            triggerImsCfgCommand(slot_id);
        }
    }

    RIL_Errno error = success ? RIL_E_SUCCESS:RIL_E_GENERIC_FAILURE;
    sp<RfxMessage> response = RfxMessage::obtainResponse(error, message, true);
    responseToRilj(response);

    if (success) {
        bool allowNotify = false;
        int simState = getStatusManager()->getIntValue(RFX_STATUS_KEY_SIM_STATE);
        bool featureOn = false;
        if(mECCAllowNotify) {
            allowNotify = ((simState == RFX_SIM_STATE_ABSENT || simState ==
                    RFX_SIM_STATE_PIN_REQUIRED || simState == RFX_SIM_STATE_PUK_REQUIRED ||
                    simState == RFX_SIM_STATE_NETWORK_LOCKED) &&
                    ((featureId == FEATURE_TYPE_VOICE_OVER_LTE) ||
                     (featureId == FEATURE_TYPE_VOICE_OVER_NR)));
            featureOn = value == FEATURE_VALUE_ON;
            if(allowNotify && !featureOn) {
                mECCAllowNotify = false;
            }
        }

        bool simStateAllow = simState == RFX_SIM_STATE_READY;
        if(simStateAllow || (allowNotify && featureOn) || forceNotify) {
            if (DEBUG)
                logD(RFX_LOG_TAG,
                     "Notify feature changed, simStateAllow = %d allowNotify = %d featureOn = %d forceNotify = %d",
                     simStateAllow, allowNotify, featureOn, forceNotify);
            int data[2] = {featureId, value};
            sp <RfxMessage> urc = RfxMessage::obtainUrc(getSlotId(),
                                                        RFX_MSG_UNSOL_IMS_CONFIG_FEATURE_CHANGED,
                                                        RfxIntsData(data, 2));
            responseToRilj(urc);
            mECCAllowNotify = false;
        }
    }

    if (((featureId == FEATURE_TYPE_VOICE_OVER_LTE) ||
         (featureId == FEATURE_TYPE_VOICE_OVER_NR)) &&
        ImsConfigUtils::isAllowForceNotify(getSlotId(), value)) {
        ImsConfigUtils::setAllowForceNotify(getSlotId(), false, 0);
    }
}

void RtcImsConfigController::getFeatureValue(const sp<RfxMessage>& message) {
    int slot_id = getSlotId();
    int *params = (int *)message->getData()->getData();
    int feature_id = params[0];
    int network = params[1];
    std::map<int, int>::iterator iter;
    RIL_Errno error = RIL_E_SUCCESS;

    if (DEBUG) logD(RFX_LOG_TAG, "getFeatureValue, feature_id:%d, network:%d", feature_id, network);

    params[0] = FEATURE_VALUE_OFF;

    if (feature_id == FEATURE_TYPE_VOICE_OVER_LTE) {
        iter = mVoLteFeatureValue.find(network);

        if (iter != mVoLteFeatureValue.end()) {
            params[0] = mVoLteFeatureValue[network];
        }
    } else if (feature_id == FEATURE_TYPE_VIDEO_OVER_LTE) {
        iter = mViLteFeatureValue.find(network);

        if (iter != mViLteFeatureValue.end()) {
            params[0] = mViLteFeatureValue[network];
        }
    } else if (feature_id == FEATURE_TYPE_VOICE_OVER_WIFI) {
        iter = mVoWifiFeatureValue.find(network);

        if (iter != mVoWifiFeatureValue.end()) {
            params[0] = mVoWifiFeatureValue[network];
        }
    } else if (feature_id == FEATURE_TYPE_VIDEO_OVER_WIFI) {
        iter = mViWifiFeatureValue.find(network);

        if (iter != mViWifiFeatureValue.end()) {
            params[0] = mViWifiFeatureValue[network];
        }
    } else if (feature_id == FEATURE_TYPE_VOICE_OVER_NR) {
        iter = mVoNrFeatureValue.find(network);

        if (iter != mVoNrFeatureValue.end()) {
            params[0] = mVoNrFeatureValue[network];
        }
    } else if (feature_id == FEATURE_TYPE_VIDEO_OVER_NR) {
        iter = mViNrFeatureValue.find(network);

        if (iter != mViNrFeatureValue.end()) {
            params[0] = mViNrFeatureValue[network];
        }
    } else {
        logD(RFX_LOG_TAG, "getFeatureValue, feature_id:%d, network:%d not support",
             feature_id, network);
        error = RIL_E_GENERIC_FAILURE;
    }

    sp<RfxMessage> response = RfxMessage::obtainResponse(error, message, true);
    responseToRilj(response);
}

void RtcImsConfigController::setImsResourceCapability(int feature_id, int value,
                                                      bool isDynamicImsSwitchLast) {
    if (DEBUG)
        logD(RFX_LOG_TAG, "setImsResourceCapability, feature_id:%d,value:%d", feature_id, value);

    if (feature_id == FEATURE_TYPE_VOICE_OVER_LTE ||
        feature_id == FEATURE_TYPE_VIDEO_OVER_LTE ||
        feature_id == FEATURE_TYPE_VOICE_OVER_WIFI ||
        feature_id == FEATURE_TYPE_VIDEO_OVER_WIFI ||
        feature_id == FEATURE_TYPE_VOICE_OVER_NR ||
        feature_id == FEATURE_TYPE_VIDEO_OVER_NR) {
        mFeatureResource[feature_id] = value;
    } else {
        logD(RFX_LOG_TAG, "setImsResourceCapability, feature_id:%d, value:%d not support",
             feature_id, value);
    }

    if (isDynamicImsSwitchLast) {
        sp<RfxMessage> urc = RfxMessage::obtainUrc(getSlotId(),
                                                   RFX_MSG_UNSOL_IMS_CONFIG_DYNAMIC_IMS_SWITCH_COMPLETE,
                                                   RfxVoidData());
        responseToRilj(urc);
    }
}

void RtcImsConfigController::getImsResourceCapability(const sp<RfxMessage>& message) {
    int slot_id = getSlotId();
    int *params = (int *)message->getData()->getData();
    int feature_id = params[0];
    RIL_Errno error = RIL_E_SUCCESS;

    if (DEBUG) logD(RFX_LOG_TAG, "getImsResourceCapability, feature_id:%d",feature_id);

    if (feature_id == FEATURE_TYPE_VOICE_OVER_LTE ||
        feature_id == FEATURE_TYPE_VIDEO_OVER_LTE ||
        feature_id == FEATURE_TYPE_VOICE_OVER_WIFI ||
        feature_id == FEATURE_TYPE_VIDEO_OVER_WIFI ||
        feature_id == FEATURE_TYPE_VOICE_OVER_NR ||
        feature_id == FEATURE_TYPE_VIDEO_OVER_NR) {
        params[0] = mFeatureResource[feature_id];
    } else {
        logD(RFX_LOG_TAG, "getImsResourceCapability, feature_id:%d not support", feature_id);
        error = RIL_E_GENERIC_FAILURE;
        params[0] = FEATURE_RESOURCE_OFF;
    }

    sp<RfxMessage> response = RfxMessage::obtainResponse(error, message, true);
    responseToRilj(response);
}

bool RtcImsConfigController::ensureStorageInitStatus(const sp<RfxMessage> &message) {
    int msg_id = message->getId();
    if (!mInitDone) {
        logD(RFX_LOG_TAG, "storage not ready while - %s", RFX_ID_TO_STR(msg_id));
        switch (msg_id) {
            case RFX_MSG_REQUEST_IMS_CONFIG_SET_PROVISION: {
                sp<RfxMessage> response = RfxMessage::obtainResponse(message->getSlotId(),
                                                                     message->getId(),
                                                                     RIL_E_GENERIC_FAILURE,
                                                                     RfxVoidData(),
                                                                     message);
                responseToRilj(response);
            }
                break;
            case RFX_MSG_REQUEST_IMS_CONFIG_GET_PROVISION: {
                char data[] = "n/a";
                sp<RfxMessage> response = RfxMessage::obtainResponse(message->getSlotId(),
                                                                     message->getId(),
                                                                     RIL_E_GENERIC_FAILURE,
                                                                     RfxStringData(data),
                                                                     message);
                responseToRilj(response);
            }
                break;
            default:
                break;
        }

        return false;
    } else {
        return true;
    }
}

void RtcImsConfigController::setProvisionValue(const sp<RfxMessage> &message) {
    if (!ensureStorageInitStatus(message)) {
        return;
    }

    int slot_id = getSlotId();
    char** params = (char**) message->getData()->getData();
    int config_id = std::atoi(params[0]);
    const char* value = (params[1] == NULL) ? "" : params[1];

    if (DEBUG)
        logD(RFX_LOG_TAG, "setProvisionValue(), config_id: %d, params: %s, %s, value: %s",
                config_id, params[0], params[1], value);

    mProvisionValue[config_id] = value;

    handleImsConfigExt(message);

    if (mProvisionedValue[config_id] != value) {
        mProvisionedValue[config_id] = value;
        saveProvisionedValue();
    }

    handleSetProvisionValue(true, message);
}

bool RtcImsConfigController::handleImsConfigExt(const sp<RfxMessage> &message) {
    char** params = (char**) message->getData()->getData();
    int config_id = std::atoi(params[0]);
    const char* value = (params[1] == NULL) ? "" : params[1];
    int iValue;
    int msg_id;
    sp<RfxMessage> newMsg;

    logD(RFX_LOG_TAG, "handleImsConfigExt(), config_id: [%d], value: [%s]", config_id, value);

    switch (config_id) {
        case IMS_CONFIG_VOICE_OVER_WIFI_MODE: {
            iValue = (strcmp(value, "") == 0) ? -1 : std::atoi(value);
            if (iValue == -1) {
                return true;
            }
            msg_id = RFX_MSG_REQUEST_SET_WFC_PROFILE;

            logD(RFX_LOG_TAG, "RFX_MSG_REQUEST_SET_WFC_PROFILE, value: [%d]", iValue);

            int rilValue = RIL_WIFI_PREFERRED;

            switch(iValue) {
                case AP_WIFI_ONLY:
                    rilValue = RIL_WIFI_ONLY;
                    break;
                case AP_CELLULAR_PREFERRED:
                    rilValue = RIL_CELLULAR_PREFERRED;
                    break;
                case AP_WIFI_PREFERRED:
                    rilValue = RIL_WIFI_PREFERRED;
                    break;
                case AP_4G_PREFER:
                    rilValue = RIL_4G_PREFER;
                    break;
                default:
                    logD(RFX_LOG_TAG, "setWfcMode mapping error, value is invalid!");
                    break;
            }

            newMsg = RfxMessage::obtainRequest(getSlotId(), RFX_MSG_REQUEST_SET_WFC_PROFILE,
                    RfxIntsData((void *) &rilValue, sizeof(int)));
            RfxMainThread::enqueueMessage(newMsg);
            return true;
        }
        default: {
            // Do nothing.
            return false;
        }
    }
}

void RtcImsConfigController::handleSetProvisionValue(bool success, const sp<RfxMessage> &message) {
    char **params = (char **) message->getData()->getData();
    int config_id = std::atoi(params[0]);
    const char* value = (params[1] == NULL) ? "" : params[1];

    bool isProvisionToModem = ImsConfigDataHelper::isProvisionToModem(config_id);
    if (isProvisionToModem) {
        // Store message for response to RILJ.
        mSetProvisionMessage = RfxMessage::obtainRequest(message->getSlotId(), message->getId(),
                message);

        //request to MCL
        string provisionStr = ImsConfigDataHelper::getConfigProvisionStr(config_id);

        logD(RFX_LOG_TAG, "set provision to MD, provisionStr: %s, value: %s", provisionStr.c_str(),
             value);

        char **data = (char **) calloc(2, sizeof(char *));

        if (data != NULL) {
            data[0] = strdup(provisionStr.c_str());
            data[1] = strdup(value);
        }

        if ((data != NULL) && (data[0] != NULL) && (data[1] != NULL)) {
            sp<RfxMessage> message = RfxMessage::obtainRequest(
                    getSlotId(),
                    RFX_MSG_REQUEST_IMS_CONFIG_SET_PROVISION,
                    RfxStringsData(data, 2));
            requestToMcl(message);
            free(data[0]);
            free(data[1]);
            free(data);
        } else {
            logD(RFX_LOG_TAG, "handleSetProvisionValue(), calloc failed!");

            if (data != NULL) {
                if (data[0] != NULL) {
                    free(data[0]);
                }

                if (data[1] != NULL) {
                    free(data[1]);
                }

                free(data);
            }

            sp<RfxMessage> response = RfxMessage::obtainResponse(RIL_E_GENERIC_FAILURE, message,
                                                                 true);
            responseToRilj(response);
        }
    } else {
        RIL_Errno error = success ? RIL_E_SUCCESS : RIL_E_GENERIC_FAILURE;
        sp<RfxMessage> response = RfxMessage::obtainResponse(error, message,
                                                             true);
        responseToRilj(response);
    }

    if (success) {
        if (DEBUG) logD(RFX_LOG_TAG, "send config changed urc, data: %s,%s", params[0], params[1]);
        sp<RfxMessage> urc = RfxMessage::obtainUrc(getSlotId(),
                                                   RFX_MSG_UNSOL_IMS_CONFIG_CONFIG_CHANGED,
                                                   RfxStringsData(params, 2));
        responseToRilj(urc);
    }
    if(!isProvisionToModem) {
    }
}

void RtcImsConfigController::handleSetProvisionResponse(const sp <RfxMessage> &message) {
    sp<RfxMessage> response = RfxMessage::obtainResponse(mSetProvisionMessage->getSlotId(),
            mSetProvisionMessage->getId(), message->getError(), RfxVoidData(),
            mSetProvisionMessage);
    responseToRilj(response);
}

void RtcImsConfigController::getProvisionValue(const sp <RfxMessage> &message) {
    if (!ensureStorageInitStatus(message)) {
        return;
    }

    int slot_id = getSlotId();
    int *params = (int *) message->getData()->getData();
    int configId = params[0];

    if (DEBUG)
        logD(RFX_LOG_TAG, "getProvisionValue(), configId: %d", configId);

    if (ImsConfigDataHelper::isProvisionToModem(configId)) {
        // Store message for response when receive URC.
        mGetProvisionId = configId;
        mGetProvisionMessage = RfxMessage::obtainRequest(message->getSlotId(), message->getId(),
                message);

        //request to MCL
        string provisionStr = ImsConfigDataHelper::getConfigProvisionStr(configId);

        char* data = strdup(provisionStr.c_str());

        if (data != NULL) {
            logD(RFX_LOG_TAG, "get provision from MD, provisionStr: %s", data);
            sp<RfxMessage> message = RfxMessage::obtainRequest(
                    slot_id,
                    RFX_MSG_REQUEST_IMS_CONFIG_GET_MD_PROVISION,
                    RfxStringData(data));
            requestToMcl(message);
            free(data);
        } else {
            logE(RFX_LOG_TAG, "getProvisionValue(), data is NULL!");

            sp<RfxMessage> response = RfxMessage::obtainResponse(message->getSlotId(),
                    message->getId(), RIL_E_INVALID_ARGUMENTS, RfxVoidData(), message);

            responseToRilj(response);
        }
    } else {
        sp<RfxMessage> response;
        char* data = strdup(mProvisionValue[configId].c_str());

        if (data != NULL) {
            response = RfxMessage::obtainResponse(message->getSlotId(),
                    message->getId(), RIL_E_SUCCESS, RfxStringData(data), message);
        } else {
            logE(RFX_LOG_TAG, "getProvisionValue(), data is NULL!");

            response = RfxMessage::obtainResponse(message->getSlotId(),
                    message->getId(), RIL_E_INVALID_ARGUMENTS, RfxVoidData(), message);
        }

        responseToRilj(response);

        if (data != NULL) {
            free(data);
        }
    }
}

void RtcImsConfigController::handleGetProvisionResponse(const sp <RfxMessage> &message) {
    // RIL_E_OEM_ERROR_24 means no MD default value, need to get from AP
    if (message->getError() == RIL_E_OEM_ERROR_24) {
        sp<RfxMessage> response;

        char* data = strdup(mProvisionValue[mGetProvisionId].c_str());

        if (data != NULL) {
            response = RfxMessage::obtainResponse(mGetProvisionMessage->getSlotId(),
                    mGetProvisionMessage->getId(), RIL_E_SUCCESS, RfxStringData(data),
                    mGetProvisionMessage);
        } else {
            logE(RFX_LOG_TAG, "handleGetProvisionResponse(), data is NULL!");

            response = RfxMessage::obtainResponse(mGetProvisionMessage->getSlotId(),
                    mGetProvisionMessage->getId(), RIL_E_INVALID_ARGUMENTS, RfxVoidData(),
                    mGetProvisionMessage);
        }


        responseToRilj(response);

        if (data != NULL) {
            free(data);
        }
    } else if (message->getError() != RIL_E_SUCCESS) {
        sp<RfxMessage> response = RfxMessage::obtainResponse(mGetProvisionMessage->getSlotId(),
                mGetProvisionMessage->getId(), message->getError(), RfxVoidData(),
                mGetProvisionMessage);
        responseToRilj(response);
    } else {
        if (DEBUG)
            logD(RFX_LOG_TAG, "handleGetProvisionResponse, wait for URC to response.");
    }
}

void RtcImsConfigController::handleGetProvisionUrc(const sp <RfxMessage> &message) {
    char **params = (char **) message->getData()->getData();
    char *config_item = params[0];
    char *value = params[1];

    logD(RFX_LOG_TAG, "handleGetProvisionUrc(), config_item:%s value: %s", config_item, value);

    // RIL_E_OEM_ERROR_24 means no MD default value, need to get from AP
    if (message->getError() == RIL_E_OEM_ERROR_24) {
        int config_id = ImsConfigDataHelper::getConfigId(string(config_item));
        sp<RfxMessage> response;
        char* data = strdup(mProvisionValue[config_id].c_str());

        if (data != NULL) {
            response = RfxMessage::obtainResponse(mGetProvisionMessage->getSlotId(),
                    mGetProvisionMessage->getId(), RIL_E_SUCCESS, RfxStringData(data),
                    mGetProvisionMessage);
        } else {
            logE(RFX_LOG_TAG, "handleGetProvisionUrc(), data is NULL!");

            response = RfxMessage::obtainResponse(mGetProvisionMessage->getSlotId(),
                    mGetProvisionMessage->getId(), RIL_E_INVALID_ARGUMENTS, RfxVoidData(),
                    mGetProvisionMessage);
        }

        responseToRilj(response);

        if (data != NULL) {
            free(data);
        }
    } else {
        sp<RfxMessage> response = RfxMessage::obtainResponse(mGetProvisionMessage->getSlotId(),
                mGetProvisionMessage->getId(), message->getError(), RfxStringData(value),
                mGetProvisionMessage);
        responseToRilj(response);
    }
}

void RtcImsConfigController::triggerImsCfgCommand(int slot_id) {
    if (!ImsConfigUtils::isPhoneIdSupportIms(slot_id)) {
        if (DEBUG) {
            logI(RFX_LOG_TAG, "Do not send EIMS feature values for phone = %d", slot_id);
        }
        return;
     }

    int params[6];
    int vonr, vinr;
    int isAllowTurnOff = 1;
    char value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    char feature[] = "IMS over 3gpp";

    isAllowTurnOff =
            getStatusManager()->getString8Value(RFX_STATUS_KEY_CARRIER_ALLOW_TURN_OFF_IMS) == "1";

    if (DEBUG) logI(RFX_LOG_TAG, "isAllowTurnOff = %d",isAllowTurnOff);
        // Get latest feature value from sys props
    params[0] = RfxRilUtils::isVolteSupport() ? ImsConfigUtils::getFeaturePropValue(
            ImsConfigUtils::PROPERTY_VOLTE_ENALBE, slot_id) : 0;  // volte
    params[1] = RfxRilUtils::isVilteSupport() ? ImsConfigUtils::getFeaturePropValue(
            ImsConfigUtils::PROPERTY_VILTE_ENALBE, slot_id) : 0;  // vilte
    params[2] = RfxRilUtils::isWfcSupport() ? ImsConfigUtils::getFeaturePropValue(
            ImsConfigUtils::PROPERTY_WFC_ENALBE, slot_id) : 0;    // vowifi
    params[3] = RfxRilUtils::isViwifiSupport() ? ImsConfigUtils::getFeaturePropValue(
            ImsConfigUtils::PROPERTY_VIWIFI_ENALBE, slot_id) : 0; // viwifi
    params[4] = RfxRilUtils::isSmsSupport() ? ImsConfigUtils::getSystemPropValue(
            ImsConfigUtils::PROPERTY_IMS_SUPPORT) : 0;   // sms

    if (DEBUG)
        logI(RFX_LOG_TAG,
             "triggerImsCfg feature version:%d, phoneId:%d", getFeatureVersion(feature), slot_id);

    if (getFeatureVersion(feature) > 1) {
        // IMS NR
        vonr = ImsConfigUtils::getFeaturePropValue(ImsConfigUtils::PROPERTY_VONR_ENALBE, slot_id);
        vinr = ImsConfigUtils::getFeaturePropValue(ImsConfigUtils::PROPERTY_VINR_ENALBE, slot_id);

        if (DEBUG)
            logI(RFX_LOG_TAG,
                 "triggerImsCfg volte:%d, vilte:%d, vonr:%d, vinr:%d, phoneId:%d",
                 params[0], params[1], vonr, vinr, slot_id);

        params[0] = (params[0] | (vonr << 1));
        params[1] = (params[1] | (vinr << 1));
    }

    // logic to decide eims with (volte|vilte|vowifi|viwifi|(isAllowTurnOff^1))
    params[5] = (params[0] | params[1] | params[2] | params[3] | params[4] | (isAllowTurnOff^1));

    if (DEBUG)
        logI(RFX_LOG_TAG,
             "triggerImsCfg volte:%d, vilte:%d, vowifi:%d, viwifi:%d, sms:%d, eims:%d, phoneId:%d",
             params[0], params[1], params[2], params[3], params[4], params[5], slot_id);

    mSendCfgMccmnc = getCurrentMccMnc();
    mSendCfgIccid = getStatusManager()->getString8Value(RFX_STATUS_KEY_SIM_ICCID);

    if (DEBUG)
        logD(RFX_LOG_TAG, "triggerImsCfg, mSendCfgMccmnc: %d, mSendCfgIccid: %s", mSendCfgMccmnc,
                RfxRilUtils::pii(RFX_LOG_TAG, mSendCfgIccid.string()));

    // Allow to send with AT+EIMSCFG
    sp<RfxMessage> message =  RfxMessage::obtainRequest(
                                            getSlotId(),
                                            RFX_MSG_REQUEST_SET_IMSCFG,
                                            RfxIntsData(params, 6));
    // send to RtcImsController
    // TODO: for long term should handle RFX_MSG_REQUEST_SET_IMSCFG here
    RfxMainThread::enqueueMessage(message);
}

int RtcImsConfigController::getCurrentMccMnc() {
    int mccmnc = 0;
    String8 defaultValue = String8("0");
    String8 mccmncStr = getStatusManager()->getString8Value(RFX_STATUS_KEY_UICC_GSM_NUMERIC,
                                                 defaultValue);

    if ((mccmncStr.string() == NULL) || (mccmncStr == "0") || (mccmncStr == "")) {
        mccmncStr = getStatusManager()->getString8Value(
                RFX_STATUS_KEY_UICC_CDMA_NUMERIC, defaultValue);
    }

    if (mccmncStr.string() == NULL) {
        mccmncStr = String8("0");
    }

    if (mccmncStr.string() != NULL) {
        mccmnc = atoi(mccmncStr.string());
    }
    return mccmnc;
}

void RtcImsConfigController::saveProvisionedValue() {
    int mccMnc = getCurrentMccMnc();
    int slot_id = getSlotId();
    std::map<int, std::string>::iterator iter;
    std::stringstream ss;   // For mimeType is INTEGER only
    int propCount = 1;

    if (mccMnc == 0) {
        logD(RFX_LOG_TAG, "saveProvisionedValue, mccMnc is 0");
        return;
    }

    resetProvisionedValue();

    ss.clear();
    ss.str("");

    for (iter = mProvisionedValue.begin(); iter != mProvisionedValue.end(); iter++) {
        int mimeType = ImsConfigDataHelper::getConfigDataType(iter->first);
        if (mimeType == INTEGER) {
            // If the length property value is over maximum, save previous property value first.
            if ((ss.str().length() + std::to_string(iter->first).length() + iter->second.length() +
                    2) >= RFX_PROPERTY_VALUE_MAX) {
                std::string propName = "persist.vendor.mtk.provision.int.";
                propName.append(std::to_string(slot_id)).append(std::to_string(propCount));
                std::string propValue = "";

                ss >> propValue;
                rfx_property_set(propName.c_str(), propValue.c_str());

                propCount++;
                ss.clear();
                ss.str("");
            }

            ss << iter->first,
            ss << ",";
            ss << iter->second;
            ss << ";";
        } else if (mimeType == STRING) {
            if ((std::to_string(iter->first).length() + iter->second.length() + 1) <
                    RFX_PROPERTY_VALUE_MAX) {
                std::string propName = "persist.vendor.mtk.provision.str.";
                propName.append(std::to_string(slot_id)).append(std::to_string(iter->first));

                rfx_property_set(propName.c_str(), iter->second.c_str());
            } else {
                logE(RFX_LOG_TAG, "saveProvisionedValue, configId: %d, value: %s too long",
                        iter->first, iter->second.c_str());
            }
        } else {
            logE(RFX_LOG_TAG, "saveProvisionedValue, not support configId = %d, mimeType = %d",
                    iter->first, mimeType);
        }
    }

    // For mimeType is INTEGER only
    if (ss.str().length() > 0) {
        std::string propName = "persist.vendor.mtk.provision.int.";
        propName.append(std::to_string(slot_id)).append(std::to_string(propCount));
        std::string propValue = "";

        ss >> propValue;

        rfx_property_set(propName.c_str(), propValue.c_str());
    }
}

void RtcImsConfigController::loadProvisionedValue() {
    int mccMnc = getCurrentMccMnc();
    int slot_id = getSlotId();
    bool isEmpty = false;
    std::stringstream ss;
    int propCount = 1;

    mProvisionedValue.clear();

    if (mccMnc == 0) {
        if (DEBUG) logD(RFX_LOG_TAG, "loadProvisionedValue, mccMnc is 0");
        return;
    }

    // Get previous stored mccmnc to check if load or reset.
    std::string propMccMncName = "persist.vendor.mtk.provision.mccmnc.";
    propMccMncName.append(std::to_string(slot_id));
    char propMccMncValue[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get(propMccMncName.c_str(), propMccMncValue, "");

    if (DEBUG)
        logD(RFX_LOG_TAG, "loadProvisionedValue, curMccMnc: %d, preMccMnc: %d", mccMnc,
                atoi(propMccMncValue));

    if (strlen(propMccMncValue)) {
        if (mccMnc != atoi(propMccMncValue)) {
            // If current mccmnc is not equal to previous stored mccmnc,
            // No need to load the previous provisioned value.
            // Previous provisioned value will been reset when setProvisionValue.
            return;
        }
    } else {
        // If no stored mccmnc, it means no previous provisioned value.
        return;
    }

    ss.clear();
    ss.str("");

    // Load provisioned value and update mProvisionedValue (mimeType is integer)
    while (!isEmpty) {
        std::string propName = "persist.vendor.mtk.provision.int.";
        propName.append(std::to_string(slot_id)).append(std::to_string(propCount));
        char propValue[RFX_PROPERTY_VALUE_MAX] = { 0 };
        rfx_property_get(propName.c_str(), propValue, "");

        if (strlen(propValue)) {
            ss << propValue;
            propCount++;
        } else {
            isEmpty = true;
        }
    }

    if (ss.str().length() > 0) {
        std::string provisionedId = "";
        std::string provisionedValue = "";

        while (std::getline(ss, provisionedId, ',')) {
            std::getline(ss, provisionedValue, ';');

            mProvisionedValue[atoi(provisionedId.c_str())] = provisionedValue;
            mProvisionValue[atoi(provisionedId.c_str())] = provisionedValue;

            provisionedId = "";
            provisionedValue = "";
        }
    }

    // Load provisioned value and update mProvisionedValue (mimeType is string)
    for (int i = IMS_CONFIG_PROVISIONED_START; i <= IMS_CONFIG_MTK_PROVISIONED_END; i++) {
        int mimeType = ImsConfigDataHelper::getConfigDataType(i);
        if (mimeType == STRING) {
            std::string propName = "persist.vendor.mtk.provision.str.";
            propName.append(std::to_string(slot_id)).append(std::to_string(i));
            char propValue[RFX_PROPERTY_VALUE_MAX] = { 0 };

            rfx_property_get(propName.c_str(), propValue, "");

            if (DEBUG)
                logD(RFX_LOG_TAG, "loadProvisionedValue, propName = %s, propValue=%s",
                        propName.c_str(), propValue);

            if (strlen(propValue)) {
                mProvisionedValue[i] = propValue;
                mProvisionValue[i] = propValue;
            }
        }

        if (i == IMS_CONFIG_PROVISIONED_END) {
            i = (IMS_CONFIG_MTK_PROVISIONED_START - 1);
        }
    }
}

void RtcImsConfigController::resetProvisionedValue() {
    int mccMnc = getCurrentMccMnc();
    int slot_id = getSlotId();
    int propCount = 1;

    std::string propMccMncName = "persist.vendor.mtk.provision.mccmnc.";
    propMccMncName.append(std::to_string(slot_id));
    std::string propMccMncValue = std::to_string(mccMnc);
    rfx_property_set(propMccMncName.c_str(), propMccMncValue.c_str());

    std::string propIntName = "persist.vendor.mtk.provision.int.";
    propIntName.append(std::to_string(slot_id)).append(std::to_string(propCount));
    char propPreviousIntValue[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get(propIntName.c_str(), propPreviousIntValue, "");

    if (strlen(propPreviousIntValue) > 0) {
        std::string propIntValue = "";
        rfx_property_set(propIntName.c_str(), propIntValue.c_str());
    }

    for (int i = IMS_CONFIG_PROVISIONED_START; i <= IMS_CONFIG_MTK_PROVISIONED_END; i++) {
        int mimeType = ImsConfigDataHelper::getConfigDataType(i);
        if (mimeType == STRING) {
            std::string propStrName = "persist.vendor.mtk.provision.str.";
            propStrName.append(std::to_string(slot_id)).append(std::to_string(i));
            char propPreviousStrValue[RFX_PROPERTY_VALUE_MAX] = { 0 };
            rfx_property_get(propStrName.c_str(), propPreviousStrValue, "");

            if (strlen(propPreviousStrValue) > 0) {
                std::string propStrValue = "";
                rfx_property_set(propStrName.c_str(), propStrValue.c_str());
            }
        }

        if (i == IMS_CONFIG_PROVISIONED_END) {
            i = (IMS_CONFIG_MTK_PROVISIONED_START - 1);
        }
    }
}

bool RtcImsConfigController::isLogEnable() {
    std::string propName = "persist.vendor.logmuch";
    char propValue[RFX_PROPERTY_VALUE_MAX] = { 0 };

    rfx_property_get(propName.c_str(), propValue, "false");

    if (!strcmp(propValue, "false")) {
        return true;
    } else {
        return false;
    }
}


