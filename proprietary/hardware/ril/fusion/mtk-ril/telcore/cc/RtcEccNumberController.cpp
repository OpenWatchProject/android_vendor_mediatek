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
#include "RtcEccNumberController.h"
#include "RfxAtLine.h"
#include "tinyxml.h"
#include <libmtkrilutils.h>
#include "RfxEmergencyNumberListData.h"
#include "RfxRilUtils.h"

#define RFX_LOG_TAG "RtcEccNumberController"

#define MAX_PROP_CHARS       50
#define MCC_CHAR_LEN         3
#define PLMN_CHAR_LEN        6
#define ESIMS_CAUSE_RECOVERY 14
#define MAX_ECC_NUM          16
#define MAX_ECC_BUF_SIZE     (MAX_ECC_NUM * 8 + 1)
#define MAX_PRINT_CHAR       2
#define CTA_MCC              "460"

#define DELAY_SET_FWK_READY_TIMER 5000 //5s

/*****************************************************************************
 * Class RtcEccNumberController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RtcEccNumberController", RtcEccNumberController, RfxController);

RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_SET_ECC_NUM);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData, RFX_MSG_REQUEST_GET_ECC_NUM);
RFX_REGISTER_DATA_TO_URC_ID(RfxStringData, RFX_MSG_UNSOL_ECC_NUM);
RFX_REGISTER_DATA_TO_URC_ID(RfxEmergencyNumberListData, RFX_MSG_URC_EMERGENCY_NUMBER_LIST);

RtcEccNumberController::RtcEccNumberController() :
        mGsmEcc(""), mC2kEcc(""),
        mCachedGsmUrc(NULL),
        mCachedC2kUrc(NULL),
        mIsSimInsert(false),
        mSimEccSource(NULL),
        mNetworkEccSource(NULL),
        mDefaultEccSource(NULL),
        mXmlEccSource(NULL),
        mFrameworkEccSource(NULL),
        mPropertyEccSource(NULL),
        mTestEccSource(NULL),
        mCtaEccSource(NULL),
        mIsFwkReady(false),
        mIsPendingUpdate(false),
        mTimerHandle(NULL) {
    char testMode[MTK_PROPERTY_VALUE_MAX] = {0};
    mtk_property_get("persist.vendor.ril.test_mode", testMode, "0");
    if (atoi(testMode) != 0) {
        mIsFwkReady = true;
        logD(RFX_LOG_TAG, "[%s] Set framework ready for test mode", __FUNCTION__);
    }
}

RtcEccNumberController::~RtcEccNumberController() {
    if (mCachedC2kUrc != NULL) {
        delete(mCachedC2kUrc);
    }
    if (mCachedGsmUrc != NULL) {
        delete(mCachedGsmUrc);
    }

    for (int i = 0; i < (int)mEccNumberSourceList.size(); i++) {
        if (mEccNumberSourceList[i] != NULL) {
            delete(mEccNumberSourceList[i]);
        }
    }
    mEccNumberSourceList.clear();
}

EmergencyCallRouting RtcEccNumberController::getEmergencyCallRouting(String8 number) {
    if (number.isEmpty()) {
        logE(RFX_LOG_TAG, "[%s] Empty number return unknown", __FUNCTION__);
        return ECC_ROUTING_UNKNOWN;
    }

    // Only print first two characters
    char maskNumber[MAX_PRINT_CHAR + 1] = {0};
    strncpy(maskNumber, number.string(), MAX_PRINT_CHAR);

    for (int i = 0; i < (int)mEccList.size(); i++) {
        if (mEccList[i].number == string(number.string())) {
            // For special/fake emergency number, it should dial using normal call routing when
            // SIM ready, for other cases, it should still dial using emergency routing
            int simState = getStatusManager()->getIntValue(RFX_STATUS_KEY_SIM_STATE);
            if ((mEccList[i].condition == CONDITION_MMI) &&
                    (simState == RFX_SIM_STATE_READY)) {
                RFX_LOG_I(RFX_LOG_TAG, "[%s] %s* is a special ECC", __FUNCTION__, maskNumber);
                return ECC_ROUTING_NORMAL;
            } else {
                RFX_LOG_I(RFX_LOG_TAG, "[%s] %s* is a ECC, condition: %d, simState: %d",
                        __FUNCTION__, maskNumber, mEccList[i].condition, simState);
                return ECC_ROUTING_EMERGENCY;
            }
        }
    }
    RFX_LOG_I(RFX_LOG_TAG, "[%s] %s* is not a ECC", __FUNCTION__, maskNumber);
    return ECC_ROUTING_UNKNOWN;
}

bool RtcEccNumberController::isEmergencyNumber(String8 number) {
    if (number.isEmpty()) {
        logE(RFX_LOG_TAG, "[%s] Empty number return false", __FUNCTION__);
        return false;
    }

    // Only print first two characters
    char maskNumber[MAX_PRINT_CHAR + 1] = {0};
    strncpy(maskNumber, number.string(), MAX_PRINT_CHAR);

    for (int i = 0; i < (int)mEccList.size(); i++) {
        if (mEccList[i].number == string(number.string())) {
            RFX_LOG_I(RFX_LOG_TAG, "[%s] %s* is a ECC with condition: %d", __FUNCTION__,
                    maskNumber, mEccList[i].condition);
            return true;
        }
    }
    RFX_LOG_I(RFX_LOG_TAG, "[%s] %s* is not a ECC", __FUNCTION__, maskNumber);
    return false;
}

int RtcEccNumberController::getServiceCategory(String8 number) {
    // Only print first two characters
    char maskNumber[MAX_PRINT_CHAR + 1] = {0};
    strncpy(maskNumber, number.string(), MAX_PRINT_CHAR);

    for (int i = 0; i < (int)mEccList.size(); i++) {
        if ((mEccList[i].number == string(number.string())) &&
                (mEccList[i].categories != ECC_CATEGORY_NOT_DEFINED)) {
            RFX_LOG_I(RFX_LOG_TAG, "[%s] %s* category is %d", __FUNCTION__, maskNumber,
                    mEccList[i].categories);
            return mEccList[i].categories;
        }
    }

    RFX_LOG_I(RFX_LOG_TAG, "[%s] %s* category is 0", __FUNCTION__, maskNumber);
    return 0;
}

void RtcEccNumberController::onInit() {
    // Required: invoke super class implementation
    RfxController::onInit();

    logD(RFX_LOG_TAG, "[%s]", __FUNCTION__);

    const int urc_id_list[] = {
        RFX_MSG_URC_CC_GSM_SIM_ECC,
        RFX_MSG_URC_CC_C2K_SIM_ECC,
        RFX_MSG_UNSOL_ECC_NUM
    };

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    registerToHandleUrc(urc_id_list, sizeof(urc_id_list)/sizeof(const int));

    const int requests[] = {
        RFX_MSG_REQUEST_SET_ECC_NUM,
        RFX_MSG_REQUEST_GET_ECC_NUM,
    };
    registerToHandleRequest(requests, sizeof(requests)/sizeof(int));

    // register callbacks to get card type change event
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_CARD_TYPE,
        RfxStatusChangeCallback(this, &RtcEccNumberController::onCardTypeChanged));

    // register callbacks to get PLMN(MCC,MNC) change event
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_OPERATOR_INCLUDE_LIMITED,
        RfxStatusChangeCallback(this, &RtcEccNumberController::onPlmnChanged));

    // register callbacks to get sim recovery event
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_SIM_ESIMS_CAUSE,
        RfxStatusChangeCallback(this, &RtcEccNumberController::onSimRecovery));

    // register callbacks to get sim state event
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_SIM_STATE,
        RfxStatusChangeCallback(this, &RtcEccNumberController::onSimStateChanged));

    // register callbacks to get sim state event
    getStatusManager()->registerStatusChanged(RFX_STATUS_CONNECTION_STATE,
        RfxStatusChangeCallback(this, &RtcEccNumberController::onConnectionStateChanged));

    // register callbacks to get GSM UICC MCC/MNC
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_UICC_GSM_NUMERIC,
        RfxStatusChangeCallback(this, &RtcEccNumberController::onGsmUiccMccMncChanged));

    // register callbacks to get CDMA UICC MCC/MNC
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_UICC_CDMA_NUMERIC,
        RfxStatusChangeCallback(this, &RtcEccNumberController::onCdmaUiccMccMncChanged));

    initEmergencyNumberSource();
}

void RtcEccNumberController::initEmergencyNumberSource() {
    mNetworkEccSource = new NetworkEccNumberSource(m_slot_id);
    mSimEccSource = new SimEccNumberSource(m_slot_id);
    mDefaultEccSource = new DefaultEccNumberSource(m_slot_id);

    mEccNumberSourceList.clear();
    // Add ECC source by priority
    mEccNumberSourceList.push_back(mNetworkEccSource);
    mEccNumberSourceList.push_back(mSimEccSource);
    mEccNumberSourceList.push_back(mDefaultEccSource);

    // Init property ECC if property is set
    char eccCount[MAX_PROP_CHARS] = {0};
    rfx_property_get(PROPERTY_ECC_COUNT, eccCount, "0");
    if (atoi(eccCount) > 0) {
        logD(RFX_LOG_TAG, "[%s] OEM property exist, init property source", __FUNCTION__);
        mPropertyEccSource = new OemPropertyEccNumberSource(m_slot_id);
        mEccNumberSourceList.push_back(mPropertyEccSource);
    }

    // Init XML ECC if XML file is exist
    FILE* file = fopen(ECC_PATH, "r");
    // Try to find a light way to check file exist
    if (file != NULL) {
        logD(RFX_LOG_TAG, "[%s] XML file exist, init XML source", __FUNCTION__);
        mXmlEccSource = new XmlEccNumberSource(m_slot_id);
        mEccNumberSourceList.push_back(mXmlEccSource);
        fclose(file);
    }

    mTestEccSource = new TestEccNumberSource(m_slot_id);
    mEccNumberSourceList.push_back(mTestEccSource);

    mCtaEccSource = new CtaEccNumberSource(m_slot_id);
    mEccNumberSourceList.push_back(mCtaEccSource);

    mEccList.clear();
}

void RtcEccNumberController::onCardTypeChanged(RfxStatusKeyEnum key,
    RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(key);
    if (oldValue.asInt() != newValue.asInt()) {
        logV(RFX_LOG_TAG, "[%s] oldValue %d, newValue %d", __FUNCTION__,
            oldValue.asInt(), newValue.asInt());
        if (newValue.asInt() == 0) {
            logD(RFX_LOG_TAG,"[%s], reset SIM/NW ECC due to No SIM", __FUNCTION__);
            mGsmEcc = String8("");
            mC2kEcc = String8("");
            // Clear network ECC when SIM removed according to spec.
            rfx_property_set(PROPERTY_NW_ECC_LIST[m_slot_id], "");
            mIsSimInsert = false;
            mSimEccSource->update("", "");
            updateEmergencySourcesForAllSimStateChanges(hasSimInAnySlot());
            //mDefaultEccSource->update(false);
            //if (mXmlEccSource != NULL) {
            //    mXmlEccSource->update(getPlmn(m_slot_id), false);
            //}
            mNetworkEccSource->clear();
            mTestEccSource->update(false);
            updateEmergencyNumber();
        } else if (!isCdmaCard(newValue.asInt())) {
            // no CSIM or RUIM application, clear CDMA ecc property
            logV(RFX_LOG_TAG,"[%s], Remove C2K ECC due to No C2K SIM", __FUNCTION__);
            mC2kEcc = String8("");
            mSimEccSource->update(string(mGsmEcc.string()), "");
            updateEmergencyNumber();
        }
    }
}

void RtcEccNumberController::onPlmnChanged(RfxStatusKeyEnum key,
    RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(key);
    bool isChange = false;
    logV(RFX_LOG_TAG, "[%s] oldValue %s, newValue %s", __FUNCTION__,
        (const char *)(oldValue.asString8()), (const char *)(newValue.asString8()));

    if (newValue.asString8().length() < MCC_CHAR_LEN) {
        logE(RFX_LOG_TAG, "[%s] MCC length error !", __FUNCTION__);
        return;
    }

    // The sequence of modem report network ecc list and camp on cell is not fixed.
    // So there will be cases when modem report network ecc list (+CEN1/+CEN2) and
    // AP will save the network ECC then before camp on cell, EOPS will be 000000.
    // Thus if AP query PLMN at this time, onMccMncIndication() will be called and
    // previous network ECC saved will be cleared. To fix this timing issue, we'll
    // not reset network ECC when PLMN change to "000000".
    if (strcmp((const char *)newValue.asString8(), "000000") == 0) {
        logV(RFX_LOG_TAG, "[%s] don't reset before camp on cell", __FUNCTION__);
        return;
    }

    char currentMccmnc[RFX_PROPERTY_VALUE_MAX] = {0};
    /* Check if the latest MCC/MNC is different from the value stored in system property,
       and if they are different then clear emergency number and service category */
    rfx_property_get(PROPERTY_NW_ECC_MCC[m_slot_id], currentMccmnc, "0");
    char mcc[MCC_CHAR_LEN + 1] = {0};
    strncpy(mcc, (const char *)newValue.asString8(), MCC_CHAR_LEN);
    if (strcmp(currentMccmnc, mcc)) {
        rfx_property_set(PROPERTY_NW_ECC_LIST[m_slot_id], "");
        mNetworkEccSource->clear();
        isChange = true;
    }

    isChange = updateEmergencySourcesForPlmnChange(mIsSimInsert);

    if (isChange) {
        updateEmergencyNumber();
    }
}

void RtcEccNumberController::onSimRecovery(RfxStatusKeyEnum key,
    RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(key);
    RFX_UNUSED(oldValue);

    if (newValue.asInt() == ESIMS_CAUSE_RECOVERY) {
        logD(RFX_LOG_TAG, "[%s] parse from cached URC", __FUNCTION__);

        // Need parse from cached ECC URC when SIM recovery because when
        // sim lost it will clear ECC in card type change event
        parseSimEcc(mCachedGsmUrc, true);
        parseSimEcc(mCachedC2kUrc, false);
        mSimEccSource->update(string(mGsmEcc.string()), string(mC2kEcc.string()));
        updateEmergencyNumber();
    }
}

// Update special emergency number when sim state changed
// For special emergency number:
//     SIM ready: Normal call routing
//     Others: Emergency call routing
void RtcEccNumberController::onSimStateChanged(RfxStatusKeyEnum key,
    RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(key);

    if (oldValue.asInt() != newValue.asInt()) {
        logV(RFX_LOG_TAG,"[%s] update special emergency number", __FUNCTION__);
        updateSpecialEmergencyNumber();
    }
}

void RtcEccNumberController::onConnectionStateChanged(RfxStatusKeyEnum key,
    RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(oldValue);
    RFX_UNUSED(key);

    logD(RFX_LOG_TAG,"[%s] status: %d, mIsFwkReady: %d, mIsPendingUpdate: %d",
            __FUNCTION__, newValue.asBool(), mIsFwkReady, mIsPendingUpdate);

    if (mTimerHandle != NULL) {
        RfxTimer::stop(mTimerHandle);
        mTimerHandle = NULL;
    }

    if (!newValue.asBool()) {
        mIsFwkReady = false;
    } else if (!mIsFwkReady) {
        // Delay a timer to notify framework
        logD(RFX_LOG_TAG,"[%s] Delay to sent emergency list", __FUNCTION__);
        mTimerHandle = RfxTimer::start(RfxCallback0(this,
                &RtcEccNumberController::delaySetFwkReady), ms2ns(DELAY_SET_FWK_READY_TIMER));
    }
}

void RtcEccNumberController::onGsmUiccMccMncChanged(RfxStatusKeyEnum key,
    RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(oldValue);
    RFX_UNUSED(key);

    logV(RFX_LOG_TAG, "[%s] oldValue %s, newValue %s", __FUNCTION__,
        (const char *)(oldValue.asString8()), (const char *)(newValue.asString8()));

    if (updateEmergencySourcesForPlmnChange(true)) {
        updateEmergencyNumber();
    }
}

void RtcEccNumberController::onCdmaUiccMccMncChanged(RfxStatusKeyEnum key,
    RfxVariant oldValue, RfxVariant newValue) {
    RFX_UNUSED(oldValue);
    RFX_UNUSED(key);

    logV(RFX_LOG_TAG, "[%s] oldValue %s, newValue %s", __FUNCTION__,
        (const char *)(oldValue.asString8()), (const char *)(newValue.asString8()));

    if (updateEmergencySourcesForPlmnChange(true)) {
        updateEmergencyNumber();
    }
}

void RtcEccNumberController::delaySetFwkReady() {
    logD(RFX_LOG_TAG, "[%s] mIsFwkReady: %d, mIsPendingUpdate: %d", __FUNCTION__,
            mIsFwkReady, mIsPendingUpdate);
    mIsFwkReady = true;
    if (mIsPendingUpdate) {
        updateEmergencyNumber();
        mIsPendingUpdate = false;
    }
}

bool RtcEccNumberController::onHandleUrc(const sp<RfxMessage>& message) {
    int msgId = message->getId();

    switch (msgId) {
        case RFX_MSG_URC_CC_GSM_SIM_ECC:
            handleGsmSimEcc(message);
            break;
        case RFX_MSG_URC_CC_C2K_SIM_ECC:
            handleC2kSimEcc(message);
            break;
        case RFX_MSG_UNSOL_ECC_NUM:
            handleUpdateNetworkEmergencyNumber(message);
            break;
        default:
            break;
    }

    return true;
}

/*
 * [MD1 EF ECC URC format]
 * + ESMECC: <m>[,<number>,<service category>[,<number>,<service category>]]
 * <m>: number of ecc entry
 * <number>: ecc number
 * <service category>: service category
 * Ex.
 * URC string:+ESIMECC:3,115,4,334,5,110,1
 *
 * Note:If it has no EF ECC, RtcEccNumberController will receive "0"
*/
void RtcEccNumberController::handleGsmSimEcc(const sp<RfxMessage>& message){
    if (mCachedGsmUrc != NULL) {
        delete(mCachedGsmUrc);
    }
    // SIM ECC URC come earlier then other SIM status key
    mIsSimInsert = true;

    mCachedGsmUrc = new RfxAtLine((const char* )(message->getData()->getData()), NULL);
    //RFX_UNUSED(message);
    //mCachedGsmUrc = new RfxAtLine("AT< +ESIMECC: 16,112,0,911,0,123,1,125,2,119,4,146,8,144,16,126,32,111,64,141,34,147,84,146,65,013,1,014,1,015,1,016,1", NULL);

    parseSimEcc(mCachedGsmUrc, true);
    mSimEccSource->update(string(mGsmEcc.string()), string(mC2kEcc.string()));
    //mDefaultEccSource->update(true);
    //updateEmergencySourcesForPlmnChange(true);
    updateEmergencySourcesForAllSimStateChanges(true);
    mTestEccSource->update(true);
    updateEmergencyNumber();
}

/*
 * [MD3 EF ECC URC format]
 * +CECC:<m>[,<number [,<number >]]
 * <m>: number of ecc entry
 * <number>: ecc number
 * Ex.
 * URC string:2,115,334
 *
 * Note:If it has no EF ECC, RtcEccNumberController will receive "0"
 *
*/
void RtcEccNumberController::handleC2kSimEcc(const sp<RfxMessage>& message){
    if (mCachedC2kUrc != NULL) {
        delete(mCachedC2kUrc);
    }

    // SIM ECC URC come earlier then other SIM status key
    mIsSimInsert = true;

    mCachedC2kUrc = new RfxAtLine((const char* )(message->getData()->getData()), NULL);

    parseSimEcc(mCachedC2kUrc, false);
    mSimEccSource->update(string(mGsmEcc.string()), string(mC2kEcc.string()));
    //mDefaultEccSource->update(true);
    //updateEmergencySourcesForPlmnChange(true);
    updateEmergencySourcesForAllSimStateChanges(true);
    mTestEccSource->update(true);
    updateEmergencyNumber();
}

void RtcEccNumberController::parseSimEcc(RfxAtLine *line, bool isGsm) {
    String8 writeEcc = String8("");
    int err = 0;
    int count = 0;

    if (line == NULL) {
        logE(RFX_LOG_TAG, "[%s] error: line is NULL", __FUNCTION__);
        return;
    }

    logV(RFX_LOG_TAG, "[%s] line: %s", __FUNCTION__, line->getLine());

    line->atTokStart(&err);
    if (err < 0) goto error;

    // get ECC number count
    count = line->atTokNextint(&err);
    if (err < 0) goto error;

    if (count > 0) {
        for (int i = 0; i < count; i++) {
            if (isGsm) {
                char* ecc = line->atTokNextstr(&err);
                if (err < 0) goto error;
                char* eccCategory = line->atTokNextstr(&err);
                if (err < 0) goto error;
                writeEcc.appendFormat("%s,%s;", ecc, eccCategory);
            } else {
                char* ecc = line->atTokNextstr(&err);
                if (err < 0) goto error;
                writeEcc.appendFormat("%s,", ecc);
            }
        }
    } else {
        logV(RFX_LOG_TAG, "[%s] There is no ECC number stored in SIM", __FUNCTION__);
    }

    if (isGsm) {
        mGsmEcc = writeEcc;
        logD(RFX_LOG_TAG,"[%s] mGsmEcc: %s", __FUNCTION__, mGsmEcc.string());
    } else {
        mC2kEcc = writeEcc;
        logD(RFX_LOG_TAG,"[%s] mC2kEcc: %s", __FUNCTION__, mC2kEcc.string());
    }

    return;
error:
    logE(RFX_LOG_TAG, "[%s] parsing error!", __FUNCTION__);
}

bool RtcEccNumberController::isCdmaCard(int cardType) {
     if ((cardType & RFX_CARD_TYPE_RUIM) > 0 ||
         (cardType & RFX_CARD_TYPE_CSIM) > 0) {
         return true;
     }
     return false;
}

void RtcEccNumberController::handleUpdateNetworkEmergencyNumber(const sp<RfxMessage>& message){
    RFX_UNUSED(message);
    if (mNetworkEccSource->update()) {
        updateEmergencyNumber();
    }
}

void RtcEccNumberController::createEmergencyNumberListResponse(RIL_EmergencyNumber *data) {
    if (data == NULL) {
        return;
    }

    for (int i = 0; i < mEccList.size(); i++) {
        asprintf(&(data[i].number), "%s", mEccList[i].number.c_str());
        asprintf(&(data[i].mcc), "%s", mEccList[i].mcc.c_str());
        asprintf(&(data[i].mnc), "%s", mEccList[i].mnc.c_str());
        if (mEccList[i].categories == ECC_CATEGORY_NOT_DEFINED) {
            data[i].categories = 0;
        } else {
            data[i].categories = mEccList[i].categories;
        }
        data[i].urns = NULL; // not used yet
        // Map EmergencyNumberSource to RIL_EmergencyNumberSource (AOSP value)
        // Convert all extended source type to MODEM_CONFIG
        data[i].sources = (mEccList[i].sources & 0x0F) |
                (((mEccList[i].sources & 0xF0) > 0) ? MODEM_CONFIG : 0);
    }
}

void RtcEccNumberController::freeEmergencyNumberListResponse(RIL_EmergencyNumber *data) {
    for (int i = 0; i < mEccList.size(); i++) {
        FREEIF(data[i].number);
        FREEIF(data[i].mcc);
        FREEIF(data[i].mnc);
    }
    FREEIF(data);
}

// For CTA requirement, if phone type G (PIN lock) + C (no SIM)
// the CTA number (110,119,120,122) should dial using C phone
// which can be called to mannual emergency center. but for this case,
// this no PLMN information available for NO SIM phone, thus these numbers
// will not be added to ECC list from XML configuration.
// To meet this requirement, we check the phone's PLMN to add CTA ECC.
// It will only work when no SIM insert and there is CTA MCC ("460")
// can be detected in other phone
bool RtcEccNumberController::isNeedCtaEcc() {
    if (mIsSimInsert) {
        // if SIM insert, using XML configure for CTA ECC
        return false;
    }
    int simCount = RfxRilUtils::rfxGetSimCount();
    for (int i = 0; i < simCount; i++) {
        string plmn = getPlmn(i);
        // Check if there is CTA MCC in other slot
        if (plmn.length() > MCC_CHAR_LEN && plmn.substr(0, MCC_CHAR_LEN) == CTA_MCC) {
            return true;
        }
    }
    return false;
}

void RtcEccNumberController::updateEmergencyNumber() {
    // Update CTA emergency number if needed
    mCtaEccSource->update(isNeedCtaEcc());

    mEccList.clear();
    for (int i = 0; i < (int)mEccNumberSourceList.size(); i++) {
        mEccNumberSourceList[i]->addToEccList(mEccList);
    }

    // Remove non emergency numbers with condition NEVER after add all
    // emergency numbers with different source.
    removeNonEmergencyNumbers();

    String8 eccList = String8("");
    for (int i = 0; i < (int)mEccList.size(); i++) {
        // Skip duplicate emergency number
        if (!isEccMatchInList(String8(mEccList[i].number.c_str()), eccList)) {
            if (eccList == "") {
                eccList += String8(mEccList[i].number.c_str());
            } else {
                eccList += String8(",") + String8(mEccList[i].number.c_str());
            }
        }
    }

    logI(RFX_LOG_TAG, "[%s] mIsFwkReady: %d, mIsPendingUpdate: %d, ecc list: %s", __FUNCTION__,
            mIsFwkReady, mIsPendingUpdate, (char*)eccList.string());

    // For framework solution, it will set ril.ecclist in framework
    // So we should not override it in RILD
    if (mFrameworkEccSource == NULL) {
        // Set ril.ecclist property (For backward compatible)
        rfx_property_set(PROPERTY_ECC_LIST[m_slot_id], eccList.string());
    }

    // Save special ECC list to property
    updateSpecialEmergencyNumber();

    // Send URC to framework (Q AOSP)
    if (mIsFwkReady) {
        int length = mEccList.size() * sizeof(RIL_EmergencyNumber);
        RIL_EmergencyNumber *pResponse = (RIL_EmergencyNumber *)calloc(1, length);
        createEmergencyNumberListResponse(pResponse);
        sp<RfxMessage> urc = RfxMessage::obtainUrc(m_slot_id, RFX_MSG_URC_EMERGENCY_NUMBER_LIST,
                    RfxEmergencyNumberListData(pResponse, length));
        responseToRilj(urc);
        freeEmergencyNumberListResponse(pResponse);

        // Send URC to framework (Legacy)
        urc = RfxMessage::obtainUrc(getSlotId(), RFX_MSG_UNSOL_ECC_NUM,
                RfxStringData((char *)eccList.string()));
        responseToRilj(urc);
        mIsPendingUpdate = false;
    } else {
        mIsPendingUpdate = true;
    }

    dumpEccList();
}

void RtcEccNumberController::updateSpecialEmergencyNumber() {
    String8 specialEccList = String8("");
    int simState = getStatusManager()->getIntValue(RFX_STATUS_KEY_SIM_STATE);
    if (simState == RFX_SIM_STATE_READY) {
        for (int i = 0; i < (int)mEccList.size(); i++) {
            if (mEccList[i].condition == CONDITION_MMI) {
                if (specialEccList == "") {
                    specialEccList += String8(mEccList[i].number.c_str());
                } else {
                    specialEccList += String8(",") + String8(mEccList[i].number.c_str());
                }
            }
        }
    }
    rfx_property_set(PROPERTY_SPECIAL_ECC_LIST[m_slot_id], specialEccList.string());
    logV(RFX_LOG_TAG, "[%s] simState: %d, list: %s", __FUNCTION__, simState,
            (char*)specialEccList.string());
}

string RtcEccNumberController::convertPlmnForRoaming(string plmn) {
    // Convert for CT roaming PLMN
    if (plmn == "20404" && isCtCard()) {
        return string("46011");
    }
    return plmn;
}

string RtcEccNumberController::getPlmn(int slot) {
    // Get from network
    String8 networkPlmn = getStatusManager(slot)->getString8Value(
            RFX_STATUS_KEY_OPERATOR_INCLUDE_LIMITED, String8(""));
    logV(RFX_LOG_TAG, "[%s] networkPlmn: %s", __FUNCTION__, (char *)networkPlmn.string());
    if (networkPlmn.length() > MCC_CHAR_LEN &&
            // Don't return invalid network PLMN here, instead we
            // should try to get from SIM/UICC MCC/MNC
            networkPlmn != "000000" && networkPlmn != "FFFFFF") {
        return string(networkPlmn.string());
    }

    // Try to get from GSM SIM
    char mccmnc[MTK_PROPERTY_VALUE_MAX] = {0};
    String8 gsmProp(PROPERTY_MCC_MNC);
    gsmProp.append((slot == 0) ? "" : String8::format(".%d", slot));
    rfx_property_get((const char *)gsmProp.string(), mccmnc, "");
    logV(RFX_LOG_TAG, "[%s] %s is: %s", __FUNCTION__, gsmProp.string(), mccmnc);
    if (strlen(mccmnc) > MCC_CHAR_LEN) {
        return convertPlmnForRoaming(string(mccmnc));
    }

    // Try to get from CDMA SIM
    String8 cdmaProp(PROPERTY_MCC_MNC_CDMA);
    cdmaProp.append((slot == 0) ? "" : String8::format(".%d", slot));
    rfx_property_get((const char *)cdmaProp.string(), mccmnc, "");
    logV(RFX_LOG_TAG, "[%s] %s is: %s", __FUNCTION__, cdmaProp.string(), mccmnc);
    if (strlen(mccmnc) > MCC_CHAR_LEN) {
        return string(mccmnc);
    }

    // No PLMN detect, return ""
    logV(RFX_LOG_TAG, "[%s] No PLMN detected!", __FUNCTION__);
    return "";
}

bool RtcEccNumberController::isEccMatchInList(String8 number, String8 eccList) {
    // Add match boundary char ',' for easy match
    String8 searchEccList = String8(",") + eccList + String8(",");
    if (searchEccList.find(String8(",") + number + String8(",")) != -1) {
        return true;
    }

    return false;
}

string RtcEccNumberController::getSourcesString(int sources) {
    string sourcesString = "";
    if (sources & SOURCE_NETWORK) {
        sourcesString += string("|") + string("Network");
    }
    if (sources & SOURCE_SIM) {
        sourcesString += string("|") + string("SIM");
    }
    if (sources & SOURCE_CONFIG) {
        sourcesString += string("|") + string("Config");
    }
    if (sources & SOURCE_DEFAULT) {
        sourcesString += string("|") + string("Default");
    }
    if (sources & SOURCE_OEM_PROPERTY) {
        sourcesString += string("|") + string("Property");
    }
    if (sources & SOURCE_FRAMEWORK) {
        sourcesString += string("|") + string("Framework");
    }
    if (sources & SOURCE_TEST) {
        sourcesString += string("|") + string("TEST");
    }
    if (sources & SOURCE_CTA) {
        sourcesString += string("|") + string("CTA");
    }
    return sourcesString;
}

void RtcEccNumberController::dumpEccList() {
    for (int i = 0; i < (int)mEccList.size(); i++) {
        logV(RFX_LOG_TAG, "[%s] ECC [%d][%s,%s,%s,%d,%d,%s]",
                __FUNCTION__, i,
                mEccList[i].number.c_str(),
                mEccList[i].mcc.c_str(),
                mEccList[i].mnc.c_str(),
                mEccList[i].categories,
                mEccList[i].condition,
                getSourcesString(mEccList[i].sources).c_str());
    }
    if (ECC_DEBUG == 1) {
        testEcc();
    }
}

void RtcEccNumberController::testEcc() {
    logD(RFX_LOG_TAG, "[testEcc] start==================");
    String8 testEccList[] = {
            String8("111"),
            String8("222"),
            String8("123"),
            String8("112"),
            String8("911"),
            String8("000"),
            String8("08"),
            String8("110"),
            String8("119"),
            String8("120"),
            String8("122"),
            String8("10086")};
    for (int i = 0; i < (int)(sizeof(testEccList) / sizeof(testEccList[0])); i ++) {
        logD(RFX_LOG_TAG, "[testEcc] %s EccRouting: %d, category: %d", testEccList[i].string(),
                getEmergencyCallRouting(testEccList[i]), getServiceCategory(testEccList[i]));
    }
    logD(RFX_LOG_TAG, "[testEcc] end==================");
}

bool RtcEccNumberController::onHandleRequest(const sp<RfxMessage>& message) {
    logV(RFX_LOG_TAG, "[%d]Handle request %s",
            message->getPToken(), RFX_ID_TO_STR(message->getId()));

    switch (message->getId()) {
    case RFX_MSG_REQUEST_SET_ECC_NUM:
        handleSetEccNum(message);
        break;
    case RFX_MSG_REQUEST_GET_ECC_NUM:
        handleGetEccNum(message);
        break;
    default:
        logD(RFX_LOG_TAG, "unknown request, ignore!");
        break;
    }
    return true;
}

void RtcEccNumberController::handleSetEccNum(const sp<RfxMessage>& message) {
    const char **strings = (const char **)message->getData()->getData();
    if (strings == NULL || (strings[0] == NULL && strings[1] == NULL)) {
        logE(RFX_LOG_TAG, "handleSetEccNum invalid arguments.");
        sp<RfxMessage> responseMsg =
                RfxMessage::obtainResponse(RIL_E_INVALID_ARGUMENTS, message, true);
        responseToRilj(responseMsg);
        return;
    }

    logD(RFX_LOG_TAG, "handleSetEccNum EccListWithCard: %s, EccListNoCard: %s",
            strings[0], strings[1]);

    // Create mFrameworkEccSource when needed
    if (mFrameworkEccSource == NULL) {
        mFrameworkEccSource = new FrameworkEccNumberSource(m_slot_id);
        mEccNumberSourceList.push_back(mFrameworkEccSource);
    }

    mFrameworkEccSource->set(strings[0], strings[1]);
    mFrameworkEccSource->update(mIsSimInsert);

    sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(RIL_E_SUCCESS, message,
            true);
    responseToRilj(responseMsg);

    // sent update ecc list to upper layer
    updateEmergencyNumber();
}

void RtcEccNumberController::handleGetEccNum(const sp<RfxMessage>& message) {
    logD(RFX_LOG_TAG, "handleGetEccNum");
    sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(RIL_E_SUCCESS, message,
            true);
    responseToRilj(responseMsg);

    // Response ecc number
    updateEmergencyNumber();
}

bool RtcEccNumberController::onCheckIfRejectMessage(const sp<RfxMessage>& message,
        bool isModemPowerOff, int radioState) {
    int msgId = message->getId();
    if((radioState == (int)RADIO_STATE_OFF) &&
            (msgId == RFX_MSG_REQUEST_SET_ECC_NUM ||
             msgId == RFX_MSG_REQUEST_GET_ECC_NUM)) {
        return false;
    } else if ((radioState == (int)RADIO_STATE_UNAVAILABLE) &&
            (msgId == RFX_MSG_REQUEST_SET_ECC_NUM ||
             msgId == RFX_MSG_REQUEST_GET_ECC_NUM)) {
        return false;
    }
    return RfxController::onCheckIfRejectMessage(message, isModemPowerOff, radioState);
}

bool RtcEccNumberController::isCtCard() {
    bool ret = false;
    int type = getStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_CARD_TYPE);
    if (type == CT_4G_UICC_CARD ||
            type == CT_UIM_SIM_CARD ||
            type == CT_3G_UIM_CARD) {
        ret = true;
    }
    return ret;
}

bool RtcEccNumberController::updateEmergencySourcesForPlmnChange(bool isSimInsert) {
    bool isChange = false;
    // reload XML ECC
    if (mXmlEccSource != NULL &&
            mXmlEccSource->update(getPlmn(m_slot_id), isSimInsert)) {
        isChange = true;
    }

    // reload property ECC
    if (mPropertyEccSource != NULL &&
            mPropertyEccSource->update(getPlmn(m_slot_id), isSimInsert)) {
        isChange = true;
    }

    // Update peer slot emergency numbers if possible
    int simCount = RfxRilUtils::rfxGetSimCount();
    for (int i = 0; i < simCount; i++) {
        if (i != m_slot_id) {
            RtcEccNumberController* eccController = (RtcEccNumberController *)findController(
                    i, RFX_OBJ_CLASS_INFO(RtcEccNumberController));
            if (eccController != NULL) {
                logD(RFX_LOG_TAG, "[%s] update ECC for peer slot %d", __FUNCTION__, i);
                if (eccController->updateEmergencySourcesFromPeer(getPlmn(m_slot_id))) {
                    eccController->updateEmergencyNumber();
                }
            }
        }
    }

    return isChange;
}

// Update emergency number when PLMN changes from peer slot
// For example: SIM1 have SIM and SIM2 no SIM, SIM2 emergency number will not update
// for PLMN because it can't get valid PLMN. In this case, we can update SIM2 emergency
// numbers if SIM1 got PLMN.
bool RtcEccNumberController::updateEmergencySourcesFromPeer(string plmn) {
    if (getPlmn(m_slot_id).length() > MCC_CHAR_LEN || plmn.length() < MCC_CHAR_LEN
            || mIsSimInsert) {
        logD(RFX_LOG_TAG, "[%s] No need mIsSimInsert: %d", __FUNCTION__, mIsSimInsert);
        return false;
    }

    logD(RFX_LOG_TAG, "[%s] Update for PLMN: %s", __FUNCTION__, plmn.c_str());

    bool isChange = false;
    // reload XML ECC
    if (mXmlEccSource != NULL &&
            mXmlEccSource->update(plmn, mIsSimInsert)) {
        isChange = true;
    }

    // reload property ECC
    if (mPropertyEccSource != NULL &&
            mPropertyEccSource->update(plmn, mIsSimInsert)) {
        isChange = true;
    }

    return isChange;
}

void RtcEccNumberController::removeNonEmergencyNumbers() {
    for (vector<EmergencyNumber>::iterator iter = mEccList.begin(); iter != mEccList.end();) {
        if ((*iter).condition == (int)CONDITION_NEVER) {
            logD(RFX_LOG_TAG, "[%s] Remove %s", __FUNCTION__, (*iter).number.c_str());
            iter = mEccList.erase(iter);
        } else {
            ++iter;
        }
    }
}

bool RtcEccNumberController::isSimInserted() {
    return mIsSimInsert;
}

bool RtcEccNumberController::hasSimInAnySlot() {
    if (mIsSimInsert) {
        return true;
    }
    int simCount = RfxRilUtils::rfxGetSimCount();
    for (int i = 0; i < simCount; i++) {
        RtcEccNumberController* eccNumberController = (RtcEccNumberController *)findController(
                i, RFX_OBJ_CLASS_INFO(RtcEccNumberController));
        if (eccNumberController != NULL && eccNumberController->isSimInserted()) {
            return true;
        }
    }
    return false;
}

void RtcEccNumberController::updateForSimStateChanges(bool isSimInserted) {
    logD(RFX_LOG_TAG, "[%s] isSimInserted: %d", __FUNCTION__, isSimInserted);
    mDefaultEccSource->update(isSimInserted);
    updateEmergencySourcesForPlmnChange(isSimInserted);
}

void RtcEccNumberController::updateEmergencySourcesForAllSimStateChanges(bool isSimInserted) {
    int simCount = RfxRilUtils::rfxGetSimCount();
    for (int i = 0; i < simCount; i++) {
        RtcEccNumberController* eccNumberController = (RtcEccNumberController *)findController(
                i, RFX_OBJ_CLASS_INFO(RtcEccNumberController));
        if (eccNumberController != NULL) {
            eccNumberController->updateForSimStateChanges(isSimInserted);
            eccNumberController->updateEmergencyNumber();
        }
    }
}