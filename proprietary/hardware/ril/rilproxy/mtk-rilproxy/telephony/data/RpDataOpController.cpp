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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RpDataOpController.h"
#include "RpDataAllowController.h"
#include "RpDataUtils.h"
#include "atchannel.h"
#include <libmtkrilutils.h>
/*****************************************************************************
 * Class RpDataOpController
 * This class is meant to handle all operator requested data feature
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RpDataOpController", RpDataOpController, RfxController);
Vector<String8> *RpDataOpController::sOp12ImsiList = new Vector<String8>();
Vector<String8> *RpDataOpController::sOp17ImsiList = new Vector<String8>();
Vector<String8> *RpDataOpController::sOp129ImsiList = new Vector<String8>();
RpDataOpController::RpDataOpController() {
    logD(RP_DATA_OPCTRL_TAG, " Constructor RpDataOpController");
    mOpCode = -1;
    mAttachWhenNeededEnabled = false;
    mRoaming = false;
    mDataRoaming = false;
    mAttachSuggestion = true;
}

static const char* VZW_MCC_MNC[] =
{
    "310004", "310005", "310006", "310010", "310012", "310013",
    "310350", "310590", "310820", "310890", "310910", "311012",
    "311110", "311270", "311271", "311272", "311273", "311274",
    "311275", "311276", "311277", "311278", "311279", "311280",
    "311281", "311282", "311283", "311284", "311285", "311286",
    "311287", "311288", "311289", "311390", "311480", "311481",
    "311482", "311483", "311484", "311485", "311486", "311487",
    "311488", "311489", "311590", "312770"
};

RpDataOpController::~RpDataOpController() {
}

void RpDataOpController::onInit() {
    logD(RP_DATA_OPCTRL_TAG, " onInit()");
    RfxController::onInit(); // Required: invoke super class implementation
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_GSM_IMSI,
            RfxStatusChangeCallback(this, &RpDataOpController::onImsiChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_SERVICE_STATE,
            RfxStatusChangeCallback(this, &RpDataOpController::onServiceStateChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_DATA_SETTINGS,
            RfxStatusChangeCallback(this, &RpDataOpController::onDataSettingsChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_PDN_DEACT_ERR_CAUSE,
            RfxStatusChangeCallback(this, &RpDataOpController::onPdnDeactError));
    initOpImsiList();

    const int urc_id_list[] = {
        RIL_UNSOL_VOLTE_LTE_CONNECTION_STATUS,  // For VZW which would be used after SIM activation.
    };
    registerToHandleUrc(urc_id_list, sizeof(urc_id_list) / sizeof(int));
}

void RpDataOpController::initOpImsiList() {
    if (sOp12ImsiList->size() == 0) {
        int length = sizeof(VZW_MCC_MNC) / sizeof(VZW_MCC_MNC[0]);
        for (int i = 0; i < length; i++) {
            sOp12ImsiList->add(String8(VZW_MCC_MNC[i]));
        }
    }
    if (sOp17ImsiList->size() == 0) {
        // static member only init once
        sOp17ImsiList->add(String8("44001"));
        sOp17ImsiList->add(String8("44009"));
        sOp17ImsiList->add(String8("44010"));
    }
    if (sOp129ImsiList->size() == 0) {
        // static member only init once
        sOp129ImsiList->add(String8("44051"));
        sOp129ImsiList->add(String8("44054"));
    }
}

bool RpDataOpController::onHandleUrc(const sp<RfxMessage>& message) {
    logD(RP_DATA_OPCTRL_TAG, "Handle URC %s", RpDataUtils::urcToString(message->getId()));

    switch (message->getId()) {
    case RIL_UNSOL_VOLTE_LTE_CONNECTION_STATUS: {
        handleVolteLteConnectionStatus(message);
        break;
    }
    default:
        logD(RP_DATA_OPCTRL_TAG, "unknown urc, ignore!");
        break;
    }
    return true;
}

void RpDataOpController::onImsiChanged(RfxStatusKeyEnum key,
        RfxVariant oldValue, RfxVariant newValue) {
    const String8 imsi(newValue.asString8());

    if (!imsi.isEmpty()) {
        mOpCode = -1;
        for (int i = 0; i < (int) sOp12ImsiList->size(); i++) {
            String8 plmn(imsi.string(), sOp12ImsiList->itemAt(i).length());
            if (plmn == sOp12ImsiList->itemAt(i)) {
                mOpCode = 12;
                break;
            }
        }
        for (int i = 0; i < (int) sOp129ImsiList->size(); i++) {
            String8 plmn(imsi.string(), sOp129ImsiList->itemAt(i).length());
            if (plmn == sOp129ImsiList->itemAt(i)) {
                mOpCode = 129;
                break;
            }
        }
    } else {
        mOpCode = -1;
    }
    switch (mOpCode) {
        case 129:
            mAttachWhenNeededEnabled = true;
            break;
        default:
            mAttachWhenNeededEnabled = false;
            break;
    }
    logD(RP_DATA_OPCTRL_TAG, "[onImsiChanged] mAttachWhenNeededEnabled: %d",
            mAttachWhenNeededEnabled);
    suggestPsRegistration();
}


void RpDataOpController::onDataSettingsChanged(RfxStatusKeyEnum key,
        RfxVariant oldValue, RfxVariant newValue) {
    RfxDataSettings oldDataSettings = oldValue.asDataSettings();
    RfxDataSettings newDataSettings = newValue.asDataSettings();
    logD(RP_DATA_OPCTRL_TAG, "[onDataSettingsChanged] %s", newDataSettings.toString().string());
    if (newDataSettings.getDataRoaming() == 1) {
        mDataRoaming = true;
    } else {
        mDataRoaming = false;
    }
    switch (mOpCode) {
        case 129:
            if (mAttachWhenNeededEnabled) {
                suggestPsRegistration();
            }
            break;
        default:
            break;
    }
}

void RpDataOpController::onServiceStateChanged(RfxStatusKeyEnum key,
        RfxVariant oldValue, RfxVariant newValue) {
    RfxNwServiceState oldSS = oldValue.asServiceState();
    RfxNwServiceState newSS = newValue.asServiceState();
    int oldVoiceRegState = oldSS.getRilVoiceRegState();
    int oldDataRegState = oldSS.getRilDataRegState();
    int newVoiceRegState = newSS.getRilVoiceRegState();
    int newDataRegState = newSS.getRilDataRegState();
    int newDataRadioTech = newSS.getRilDataRadioTech();

    // In current design, this property will be used only in isKeepDefaultPdn().
    // And what we care about is whether it is LTE or not.
    // So we set property as empty ("") if it is not LTE.
    if (newDataRadioTech == RADIO_TECH_LTE) {
        setMSimProperty(m_slot_id, "vendor.gsm.network.type", "LTE");
    } else {
        setMSimProperty(m_slot_id, "vendor.gsm.network.type", "");
    }

    switch (mOpCode) {
        case 129:
            if (mAttachWhenNeededEnabled) {
                if ((oldVoiceRegState != newVoiceRegState)
                        || (oldDataRegState != newDataRegState)) {
                    logD(RP_DATA_OPCTRL_TAG, "[onServiceStateChanged] newSS: %s",
                           newSS.toString().string());
                    if (newDataRegState == RIL_REG_STATE_HOME) {
                        mRoaming = false;
                        suggestPsRegistration();
                    } else if (newDataRegState == RIL_REG_STATE_ROAMING) {
                        mRoaming = true;
                        suggestPsRegistration();
                    } else {
                        // PS is not in service, use CS state to guess home or roaming
                        if (newVoiceRegState == RIL_REG_STATE_HOME) {
                            mRoaming = false;
                            suggestPsRegistration();
                        } else if (newVoiceRegState == RIL_REG_STATE_ROAMING) {
                            mRoaming = true;
                            suggestPsRegistration();
                        } else {
                            // both PS & CS are out of service, do nothing
                        }
                    }
                }
            }
            break;
        default:
            break;
    }
}

void RpDataOpController::onPdnDeactError(RfxStatusKeyEnum key,
        RfxVariant oldValue, RfxVariant newValue) {
    int deactErr = newValue.asInt();
    logD(RP_DATA_OPCTRL_TAG, "[onPdnDeactError] %d", deactErr);
    switch (mOpCode) {
        case 129:
            if (deactErr == CME_LAST_PDN_NOT_ALLOW) {
                if (!mRoaming && mAttachSuggestion) {
                    RpDataAllowController *dataAllowCtrl = (RpDataAllowController *) findController(
                            m_slot_id, RFX_OBJ_CLASS_INFO(RpDataAllowController));
                    logD(RP_DATA_OPCTRL_TAG, "[onPdnDeactError] re-attach due to last PDN in home");
                    dataAllowCtrl->setPsRegistration(m_slot_id, false);
                    dataAllowCtrl->setPsRegistration(m_slot_id, true);
                    return;
                }
            }
            break;
        default:
            break;
    }
    logD(RP_DATA_OPCTRL_TAG, "[onPdnDeactError] ignore");
}

void RpDataOpController::suggestPsRegistration() {
    bool isRegAllow = true;
    RpDataAllowController *dataAllowCtrl = (RpDataAllowController *) findController(
            m_slot_id, RFX_OBJ_CLASS_INFO(RpDataAllowController));
    switch (mOpCode) {
        case 129:
            if (mRoaming && !mDataRoaming) {
                isRegAllow = false;
            }
            break;
        default:
            break;
    }
    // attach when needed feature is disabled, suggest attach to true
    if (!mAttachWhenNeededEnabled) {
        isRegAllow = true;
    }
    if (dataAllowCtrl != NULL) {
        logD(RP_DATA_OPCTRL_TAG,
                "mRoaming: %d, mDataRoaming: %d, mAttachSuggestion: %d, isRegAllow: %d",
                mRoaming, mDataRoaming, mAttachSuggestion, isRegAllow);
        if (isRegAllow != mAttachSuggestion) {
            mAttachSuggestion = isRegAllow;
            dataAllowCtrl->setPsRegistration(m_slot_id, isRegAllow);
        }
    } else {
        logD(RP_DATA_OPCTRL_TAG, "dataAllowCtrl = NULL");
    }
}

bool RpDataOpController::isAttachWhenNeededEnabled() {
    return mAttachWhenNeededEnabled;
}

bool RpDataOpController::isRoaming() {
    return mRoaming;
}

bool RpDataOpController::isDataRoamingEnabled() {
    return mDataRoaming;
}

void RpDataOpController::handleVolteLteConnectionStatus(const sp<RfxMessage>& message) {
    logD(RP_DATA_OPCTRL_TAG, "[handleVolteLteConnectionStatus] mOpCode = %d", mOpCode);

    switch (mOpCode) {
        case 12: {
            // Do re-attach directly without check the data.
            RpDataAllowController *dataAllowCtrl = (RpDataAllowController *) findController(
                    m_slot_id, RFX_OBJ_CLASS_INFO(RpDataAllowController));

            Parcel* p = message->getParcel();
            int len = 0;
            int value = 0;
            p->readInt32(&len);
            if (len > 0) {
                p->readInt32(&value);
                logD(RP_DATA_OPCTRL_TAG,
                        "[handleVolteLteConnectionStatus] value: [%d]", value);
                if (value == 32) {
                    dataAllowCtrl->setPsRegistration(m_slot_id, false);
                    dataAllowCtrl->setPsRegistration(m_slot_id, true);
                }
            }
            responseToRilj(message);
            break;
        }
        default:
            break;
    }
}
