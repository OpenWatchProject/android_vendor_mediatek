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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RtcDataController.h"
#include "RtcDataUtils.h"
#include "ims/RtcImsController.h"
#include "RfxRilUtils.h"
#include <string>
#include <stdlib.h>
#include <compiler/compiler_utils.h>

#include "RtcDataAllowController.h"


#define RFX_LOG_TAG "RtcDC"

/// FastDormancy status that sync from EM. @{
#define RTC_EM_FASTDORMANCY_SYNC  "EM_FASTDORMANCY_SYNC"
#define RTC_EM_FASTDORMANCY_TIMER_LENGTH 3
#define RTC_EM_FASTDORMANCY_TIMER_ARGUMENT_LENGTH 4
/// @}

/*****************************************************************************
 * Class RtcDataController
 *****************************************************************************/
RFX_IMPLEMENT_CLASS("RtcDataController", RtcDataController, RfxController);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxDataCallResponseData, RFX_MSG_REQUEST_SETUP_DATA_CALL);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringsData, RfxVoidData, RFX_MSG_REQUEST_DEACTIVATE_DATA_CALL);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxIntsData, RFX_MSG_REQUEST_LAST_DATA_CALL_FAIL_CAUSE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxDataCallResponseData, RFX_MSG_REQUEST_DATA_CALL_LIST);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxSetDataProfileData, RfxVoidData, RFX_MSG_REQUEST_SET_DATA_PROFILE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SYNC_DATA_SETTINGS_TO_MD);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringData, RfxVoidData, RFX_MSG_REQUEST_RESET_MD_DATA_RETRY_COUNT);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIaApnData, RfxVoidData, RFX_MSG_REQUEST_SET_INITIAL_ATTACH_APN);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxLceStatusResponseData, RFX_MSG_REQUEST_START_LCE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxLceStatusResponseData, RFX_MSG_REQUEST_STOP_LCE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxLceDataResponseData, RFX_MSG_REQUEST_PULL_LCEDATA);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_LTE_ACCESS_STRATUM_REPORT);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_LTE_UPLINK_DATA_TRANSFER);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxFdModeData, RfxVoidData, RFX_MSG_REQUEST_SET_FD_MODE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxLinkCapacityReportingCriteriaData, RfxVoidData, RFX_MSG_REQUEST_SET_LINK_CAPACITY_REPORTING_CRITERIA);
RFX_REGISTER_DATA_TO_URC_ID(RfxDataCallResponseData, RFX_MSG_URC_DATA_CALL_LIST_CHANGED);
RFX_REGISTER_DATA_TO_URC_ID(RfxLceDataResponseData, RFX_MSG_URC_LCEDATA_RECV);
RFX_REGISTER_DATA_TO_URC_ID(RfxIntsData, RFX_MSG_URC_LTE_ACCESS_STRATUM_STATE_CHANGE);
RFX_REGISTER_DATA_TO_URC_ID(RfxVoidData, RFX_MSG_URC_MD_DATA_RETRY_COUNT_RESET);
RFX_REGISTER_DATA_TO_URC_ID(RfxLinkCapacityEstimateData, RFX_MSG_URC_LINK_CAPACITY_ESTIMATE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxVoidData, RfxVoidData, RFX_MSG_REQUEST_RESET_ALL_CONNECTIONS);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SET_PREFERRED_DATA_MODEM);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_SEND_DEVICE_STATE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxKeepaliveRequestData, RfxKeepaliveStatusData, RFX_MSG_REQUEST_START_KEEPALIVE);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxIntsData, RfxVoidData, RFX_MSG_REQUEST_STOP_KEEPALIVE);

RtcDataController::RtcDataController() :
    isUnderCapabilitySwitch(false),
    requestTokenIdForDisableIms(INVALID_VALUE),
    transIdForDisableIms(INVALID_VALUE),
    mIsPreferredDataMode(-1) {
}

RtcDataController::~RtcDataController() {
}

void RtcDataController::onInit() {
    RfxController::onInit();  // Required: invoke super class implementation
    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] enter", m_slot_id, __FUNCTION__);

    int modemOffState = getNonSlotScopeStatusManager()->getIntValue(
        RFX_STATUS_KEY_MODEM_OFF_STATE, MODEM_OFF_IN_IDLE);
    isUnderCapabilitySwitch = (modemOffState == MODEM_OFF_BY_SIM_SWITCH) ? true : false;

    const int request_id_list[] = {
        RFX_MSG_REQUEST_SYNC_DATA_SETTINGS_TO_MD,
        RFX_MSG_REQUEST_RESET_MD_DATA_RETRY_COUNT,
        RFX_MSG_REQUEST_START_LCE,
        RFX_MSG_REQUEST_STOP_LCE,
        RFX_MSG_REQUEST_PULL_LCEDATA,
        RFX_MSG_REQUEST_SETUP_DATA_CALL,
        RFX_MSG_REQUEST_DEACTIVATE_DATA_CALL,
        RFX_MSG_REQUEST_DATA_CALL_LIST,
        RFX_MSG_REQUEST_LAST_DATA_CALL_FAIL_CAUSE,
        RFX_MSG_REQUEST_SET_DATA_PROFILE,
        RFX_MSG_REQUEST_SET_INITIAL_ATTACH_APN,
        RFX_MSG_REQUEST_SET_LTE_ACCESS_STRATUM_REPORT,
        RFX_MSG_REQUEST_SET_LTE_UPLINK_DATA_TRANSFER,
        RFX_MSG_REQUEST_SET_FD_MODE,
        RFX_MSG_REQUEST_RESET_ALL_CONNECTIONS,
        RFX_MSG_REQUEST_SET_LINK_CAPACITY_REPORTING_CRITERIA,
        RFX_MSG_REQUEST_SET_PREFERRED_DATA_MODEM,
        RFX_MSG_REQUEST_SEND_DEVICE_STATE,
        RFX_MSG_REQUEST_START_KEEPALIVE,
        RFX_MSG_REQUEST_STOP_KEEPALIVE,
    };

    registerToHandleRequest(request_id_list,
            sizeof(request_id_list) / sizeof(const int));

    registerForStatusChange();
}

void RtcDataController::registerForStatusChange() {
    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] enter", m_slot_id, __FUNCTION__);
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_WORLD_MODE_STATE,
        RfxStatusChangeCallback(this, &RtcDataController::onWorldModeStateChanged));

    getNonSlotScopeStatusManager()->registerStatusChanged(RFX_STATUS_KEY_MODEM_OFF_STATE,
        RfxStatusChangeCallback(this, &RtcDataController::onModemOffStateChanged));

    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_UICC_GSM_NUMERIC,
            RfxStatusChangeCallback(this, &RtcDataController::onUiccMccMncChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_UICC_CDMA_NUMERIC,
            RfxStatusChangeCallback(this, &RtcDataController::onUiccMccMncChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_GSM_SPN,
            RfxStatusChangeCallback(this, &RtcDataController::onSpnChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_CDMA_SPN,
            RfxStatusChangeCallback(this, &RtcDataController::onSpnChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_GSM_IMSI,
            RfxStatusChangeCallback(this, &RtcDataController::onImsiChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_C2K_IMSI,
            RfxStatusChangeCallback(this, &RtcDataController::onImsiChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_GSM_GID1,
            RfxStatusChangeCallback(this, &RtcDataController::onGid1Changed));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_GSM_PNN,
            RfxStatusChangeCallback(this, &RtcDataController::onPnnChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_GSM_IMPI,
            RfxStatusChangeCallback(this, &RtcDataController::onImpiChanged));
    getStatusManager()->registerStatusChanged(RFX_STATUS_KEY_SLOT_ALLOW,
            RfxStatusChangeCallback(this, &RtcDataController::onAllowedChanged));


    /// Sync data setting from OEM @{
    // Register data setting status change from OEM hook string.
    getStatusManager()->registerStatusChanged(
                RFX_STATUS_KEY_TELEPHONY_ASSISTANT_STATUS,
                RfxStatusChangeCallback(this, &RtcDataController::onDataSettingStatusChanged));
    /// @}
}

void RtcDataController::onWorldModeStateChanged(RfxStatusKeyEnum key,
    RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    int newValue = value.asInt();
    int oldValue = old_value.asInt();
    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] old = %d, new = %d",
            m_slot_id, __FUNCTION__, oldValue, newValue);
    if (newValue == WORLD_MODE_SWITCHING) {
        sp<RfxMessage> reqToRild = RfxMessage::obtainRequest(m_slot_id,
                RFX_MSG_REQUEST_CLEAR_ALL_PDN_INFO, RfxVoidData());
        requestToMcl(reqToRild);
    }
}

void RtcDataController::onModemOffStateChanged(RfxStatusKeyEnum key,
    RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    int newValue = value.asInt();
    int oldValue = old_value.asInt();
    if (newValue == MODEM_OFF_BY_SIM_SWITCH) {
        RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] Enter Sim switch state", m_slot_id, __FUNCTION__);
        isUnderCapabilitySwitch = true;
    } else {
        if (isUnderCapabilitySwitch) {
            RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] Leave Sim switch state", m_slot_id, __FUNCTION__);
            char no_reset_support[RFX_PROPERTY_VALUE_MAX] = { 0 };
            rfx_property_get("vendor.ril.simswitch.no_reset_support", no_reset_support, "0");
            if (strcmp(no_reset_support, "1")==0) {
                sp<RfxMessage> reqToRild = RfxMessage::obtainRequest(m_slot_id,
                    RFX_MSG_REQUEST_RESEND_SYNC_DATA_SETTINGS_TO_MD, RfxVoidData());
                reqToRild->setAddAtFront(true);
                requestToMcl(reqToRild);
            }
        }
        isUnderCapabilitySwitch = false;
    }
}

void RtcDataController::onUiccMccMncChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    String8 strMccMnc = value.asString8();
    String8 mccMncKey("");

    if (!strMccMnc.isEmpty() && 0 == atoi(value.asString8().string())) {
        strMccMnc = String8::format("%d", atoi(value.asString8().string()));
    }

    RFX_LOG_D(RFX_LOG_TAG, "[%d]onUiccMccMncChanged: strMccMnc = %s",
            m_slot_id, strMccMnc.string());

    if (RFX_STATUS_KEY_UICC_GSM_NUMERIC == key) {
        mccMncKey.append("vendor.ril.data.gsm_mcc_mnc");
    } else {
        mccMncKey.append("vendor.ril.data.cdma_mcc_mnc");
    }
    mccMncKey.append(String8::format("%d", m_slot_id));
    rfx_property_set(mccMncKey, strMccMnc.string());

    // using 'RFX_MSG_URC_MD_DATA_RETRY_COUNT_RESET' to notify AP the mcc/mnc is ready
    // AP should do the corresponding handling
    if (!strMccMnc.isEmpty()) {
        sp<RfxMessage> message = RfxMessage::obtainUrc(getSlotId(),
                RFX_MSG_URC_MD_DATA_RETRY_COUNT_RESET, RfxVoidData());
        responseToRilj(message);
    }
}

void RtcDataController::onSpnChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    String8 strSpn = value.asString8();
    String8 keySpn("");

    RFX_LOG_D(RFX_LOG_TAG, "[%d]onSpnChanged: strSpn = %s", m_slot_id, strSpn.string());

    if (RFX_STATUS_KEY_GSM_SPN == key) {
        keySpn.append("vendor.ril.data.gsm_spn");
    } else {
        keySpn.append("vendor.ril.data.cdma_spn");
    }
    keySpn.append(String8::format("%d", m_slot_id));
    rfx_property_set(keySpn, strSpn.string());
}

void RtcDataController::onImsiChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    String8 keyImsi("");
    std::string strImsi = std::string(value.asString8().string());
    std::string strImsiMask(strImsi.substr(0, 9));
    strImsiMask.append("xxxxxx");

    RFX_LOG_D(RFX_LOG_TAG, "[%d]onImsiChanged: strImsiMask = %s", m_slot_id, strImsiMask.c_str());

    if (RFX_STATUS_KEY_GSM_IMSI == key) {
        keyImsi.append("vendor.ril.data.gsm_imsi");
    } else {
        keyImsi.append("vendor.ril.data.cdma_imsi");
    }
    keyImsi.append(String8::format("%d", m_slot_id));
    rfx_property_set(keyImsi, strImsiMask.c_str());
}

void RtcDataController::onGid1Changed(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    String8 strGid1 = value.asString8();

    RFX_LOG_D(RFX_LOG_TAG, "[%d]onGid1Changed: strGid1 = %s", m_slot_id, strGid1.string());

    String8 keyGid1("vendor.ril.data.gid1-");
    keyGid1.append(String8::format("%d", m_slot_id));
    rfx_property_set(keyGid1, strGid1.string());
}

void RtcDataController::onPnnChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    String8 strPnn = value.asString8();

    RFX_LOG_D(RFX_LOG_TAG, "[%d]onPnnChanged: strPnn = %s", m_slot_id, strPnn.string());

    String8 keyPnn("vendor.ril.data.pnn");
    keyPnn.append(String8::format("%d", m_slot_id));
    rfx_property_set(keyPnn, strPnn.string());
}

void RtcDataController::onImpiChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    String8 strImpi = value.asString8();

    RFX_LOG_D(RFX_LOG_TAG, "[%d]onImpiChanged: strImpi = %s", m_slot_id, strImpi.string());

    String8 keyImpi("vendor.ril.data.impi");
    keyImpi.append(String8::format("%d", m_slot_id));
    rfx_property_set(keyImpi, strImpi.string());
}

/// Sync FastDormancy state from EM. @{
void RtcDataController::onDataSettingStatusChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    String8 strStatus = value.asString8();
    RFX_LOG_D(RFX_LOG_TAG, "[%d] onDataSettingStatusChanged: value = %s",
            m_slot_id, strStatus.string());
    if (strStatus.find(String8(RTC_EM_FASTDORMANCY_SYNC)) != -1) {
        int timer[RTC_EM_FASTDORMANCY_TIMER_LENGTH] = {0};
        int status[RTC_EM_FASTDORMANCY_TIMER_ARGUMENT_LENGTH] = {0};

        char *tempFdSetting = strtok((char *)strStatus.string(), ":,");
        int i = 0;
        while (tempFdSetting != NULL && i < RTC_EM_FASTDORMANCY_TIMER_LENGTH) {
            tempFdSetting = strtok(NULL, ":,");
            if (tempFdSetting != NULL) {
                timer[i++] = atoi(tempFdSetting);
            }
        }

        RFX_LOG_D(RFX_LOG_TAG, "[%d] onDataSettingStatusChanged: fastdormancy = %d,%d,%d",
                m_slot_id, timer[0], timer[1], timer[2]);

        // mode 2 is for fastdormancy timer
        if (timer[0] == 2) {
            status[0] = 3; // args num
            status[1] = timer[0]; // mode
            status[2] = timer[1]; // timer type
            status[3] = timer[2]; // timer value

            sp<RfxMessage> request = RfxMessage::obtainRequest(m_slot_id,
                RFX_MSG_REQUEST_SET_FD_MODE,
                RfxIntsData(status, RTC_EM_FASTDORMANCY_TIMER_ARGUMENT_LENGTH));
            requestToMcl(request);
        }
    }
}
/// @}

void RtcDataController::onDeinit() {
    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] enter", m_slot_id, __FUNCTION__);
    RfxController::onDeinit();
}

bool RtcDataController::onHandleRequest(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] requestId: %s",
            m_slot_id, __FUNCTION__, idToString(msg_id));

    switch (msg_id) {
        case RFX_MSG_REQUEST_SYNC_DATA_SETTINGS_TO_MD:
            handleSyncDataSettingsToMD(message);
            break;
        case RFX_MSG_REQUEST_SETUP_DATA_CALL:
            handleSetupDataRequest(message);
            break;
        case RFX_MSG_REQUEST_DEACTIVATE_DATA_CALL:
            handleDeactivateDataRequest(message);
            break;
        case RFX_MSG_REQUEST_SET_PREFERRED_DATA_MODEM:
            handleSetPreferredDataModem(message);
            break;
        case RFX_MSG_REQUEST_SET_FD_MODE:
        case RFX_MSG_REQUEST_RESET_MD_DATA_RETRY_COUNT:
        case RFX_MSG_REQUEST_START_LCE:
        case RFX_MSG_REQUEST_STOP_LCE:
        case RFX_MSG_REQUEST_PULL_LCEDATA:
        case RFX_MSG_REQUEST_DATA_CALL_LIST:
        case RFX_MSG_REQUEST_LAST_DATA_CALL_FAIL_CAUSE:
        case RFX_MSG_REQUEST_SET_DATA_PROFILE:
        case RFX_MSG_REQUEST_SET_INITIAL_ATTACH_APN:
        case RFX_MSG_REQUEST_SET_LTE_ACCESS_STRATUM_REPORT:
        case RFX_MSG_REQUEST_SET_LTE_UPLINK_DATA_TRANSFER:
        case RFX_MSG_REQUEST_RESET_ALL_CONNECTIONS:
        case RFX_MSG_REQUEST_SET_LINK_CAPACITY_REPORTING_CRITERIA:
        case RFX_MSG_REQUEST_SEND_DEVICE_STATE:
        case RFX_MSG_REQUEST_START_KEEPALIVE:
        case RFX_MSG_REQUEST_STOP_KEEPALIVE:
            requestToMcl(message);
            break;
        default:
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] unknown request, ignore!", m_slot_id, __FUNCTION__);
            break;
    }
    return true;
}

bool RtcDataController::onHandleResponse(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] responseId: %s",
            m_slot_id, __FUNCTION__ , idToString(msg_id));

    switch (msg_id) {
        case RFX_MSG_REQUEST_SETUP_DATA_CALL:
            handleSetupDataResponse(message);
            break;
        case RFX_MSG_REQUEST_SYNC_DATA_SETTINGS_TO_MD:
        case RFX_MSG_REQUEST_SET_FD_MODE:
        case RFX_MSG_REQUEST_DEACTIVATE_DATA_CALL:
        case RFX_MSG_REQUEST_SET_PREFERRED_DATA_MODEM:
        case RFX_MSG_REQUEST_RESET_MD_DATA_RETRY_COUNT:
        case RFX_MSG_REQUEST_START_LCE:
        case RFX_MSG_REQUEST_STOP_LCE:
        case RFX_MSG_REQUEST_PULL_LCEDATA:
        case RFX_MSG_REQUEST_DATA_CALL_LIST:
        case RFX_MSG_REQUEST_LAST_DATA_CALL_FAIL_CAUSE:
        case RFX_MSG_REQUEST_SET_DATA_PROFILE:
        case RFX_MSG_REQUEST_SET_INITIAL_ATTACH_APN:
        case RFX_MSG_REQUEST_SET_LTE_ACCESS_STRATUM_REPORT:
        case RFX_MSG_REQUEST_SET_LTE_UPLINK_DATA_TRANSFER:
        case RFX_MSG_REQUEST_RESET_ALL_CONNECTIONS:
        case RFX_MSG_REQUEST_SET_LINK_CAPACITY_REPORTING_CRITERIA:
        case RFX_MSG_REQUEST_SEND_DEVICE_STATE:
        case RFX_MSG_REQUEST_START_KEEPALIVE:
        case RFX_MSG_REQUEST_STOP_KEEPALIVE:
            responseToRilj(message);
            break;
        default:
            RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] unknown response, ignore!", m_slot_id, __FUNCTION__);
            break;
    }
    return true;
}

bool RtcDataController::onHandleUrc(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    RFX_LOG_D(RFX_LOG_TAG, "[%d][%s] urcId: %s", m_slot_id, __FUNCTION__, idToString(msg_id));
    return true;
}

void RtcDataController::handleSyncDataSettingsToMD(const sp<RfxMessage>& message) {
    // For sync the data settings.
    int *pReqData = (int *) message->getData()->getData();
    int reqDataNum = message->getData()->getDataLength() / sizeof(int);

    int defaultDataSelected = SKIP_DATA_SETTINGS; // default data Sim

    if (reqDataNum >= DEFAULT_DATA_SIM + 1) {  // For telephony framework backward comparable.
        defaultDataSelected = pReqData[DEFAULT_DATA_SIM];
    }

    if (defaultDataSelected != SKIP_DATA_SETTINGS) {
        getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_DEFAULT_DATA_SIM,
            defaultDataSelected);
    }

    requestToMcl(message);
}

void RtcDataController::preCheckIfNeedDisableIms(const sp<RfxMessage>& message) {
    const char **pReqData = (const char **) message->getData()->getData();

    int slot_id = m_slot_id;
    RfxNwServiceState defaultServiceState (0, 0, 0 ,0);
    RfxNwServiceState serviceState = getStatusManager()
            ->getServiceStateValue(RFX_STATUS_KEY_SERVICE_STATE, defaultServiceState);
    int dataRadioTech = serviceState.getRilDataRadioTech();

    RFX_LOG_D(RFX_LOG_TAG, "preCheckIfNeedDisableIms: apntype=%s, slot id=%d, datastate=%d ",
            pReqData[1], slot_id, dataRadioTech);

    //Check if apn type is MMS
    if (atoi(pReqData[1]) != RIL_DATA_PROFILE_VENDOR_MMS) {
        return;
    }
    //RFX_LOG_D(RFX_LOG_TAG, "preCheckIfNeedDisableIms: apn type is mms");

    //Check if MMS is sent by secondary SIM, get slot id
    if (slot_id == INVALID_VALUE) {
        return;
    }
    slot_id = ((m_slot_id == 0) ? 1 : 0);
    //RFX_LOG_D(RFX_LOG_TAG, "preCheckIfNeedDisableIms: slot is secondary");

    //Check if he RAT is under 2G/3G/C2K
    switch (dataRadioTech) {
        case RADIO_TECH_LTE:
            return;
        case RADIO_TECH_LTE_CA:
            return;
        default:
            break;
    }
    requestTokenIdForDisableIms = message->getToken();

    //ImsPreCheck
    RtcImsController *imsController;
    sp<RfxAction> action;
    logD(RFX_LOG_TAG, "Disable IMS , slotId=%d", slot_id);
    imsController = (RtcImsController *) findController(slot_id,
            RFX_OBJ_CLASS_INFO(RtcImsController));
    action = new RfxAction1<const sp<RfxMessage>>(this,
            &RtcDataController::onImsConfirmed, message);
    imsController->requestImsDisable(slot_id, action);
    //RFX_LOG_D(RFX_LOG_TAG, "requestImsDisable finished");
}

void RtcDataController::handleSetupDataRequest(const sp<RfxMessage>& message) {
    if(RtcDataUtils::isSupportTemporaryDisableIms() && (RfxRilUtils::rfxGetSimCount() == 2)) {
        preCheckIfNeedDisableIms(message);
    }
    if (isPreferredDataMode()) {
        enqueueForPreferredDataMode(message);
    } else {
        requestToMcl(message);
    }
}

void RtcDataController::handleSetupDataResponse(const sp<RfxMessage>& response) {
    // For preferred data mode
    if (isPreferredDataMode()) {
        if (response->getError() == RIL_E_SUCCESS) {
            RfxDataCallResponseData *pRfxDataCallResponseData =
                    (RfxDataCallResponseData*)response->getData();
            MTK_RIL_Data_Call_Response_v11 *pRILResponse =
                    (MTK_RIL_Data_Call_Response_v11*)pRfxDataCallResponseData->getData();
            std::map<int,int>::iterator it = m_mapProfileIdToken.begin();
            for (; it != m_mapProfileIdToken.end(); ++it) {
                if (it->second == response->getToken()) {
                    break;
                }
            }
            if (it != m_mapProfileIdToken.end()) {
                RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] cid = %d, profileId = %d", m_slot_id,
                        __FUNCTION__, pRILResponse->cid, it->first);
                m_mapCidProfileId[pRILResponse->cid] = it->first;
            }
        }
    }

    //Remember the transferID of MMS pdn
    if (RtcDataUtils::isSupportTemporaryDisableIms() && RfxRilUtils::rfxGetSimCount() == 2
            && response->getToken() == requestTokenIdForDisableIms) {
        if (response->getError() != RIL_E_SUCCESS) {
            logD(RFX_LOG_TAG, "setupdata response fail!");
            requestResumeIms(response);
            responseToRilj(response);
            return;
        }
        const int *pRspData = (int *) response->getData()->getData();
        RFX_LOG_D(RFX_LOG_TAG, "handleSetupDataResponse: cid=%d", pRspData[2]);
        transIdForDisableIms = pRspData[2];
        requestTokenIdForDisableIms = INVALID_VALUE;
    }
    responseToRilj(response);
}

void RtcDataController::handleDeactivateDataRequest(const sp<RfxMessage>& message) {
    const char **pReqData = (const char **) message->getData()->getData();

    RFX_LOG_D(RFX_LOG_TAG, "handleDeactivateDataRequest: cid=%s", pReqData[0]);
    if (isPreferredDataMode()) {
        dequeueForPreferredDataMode(message);
    }
    //If the cid is same, resume ims
    if (RtcDataUtils::isSupportTemporaryDisableIms() && RfxRilUtils::rfxGetSimCount() == 2
            && transIdForDisableIms == atoi(pReqData[0]) && atoi(pReqData[0]) >= 0) {
        requestResumeIms(message);
    }
    requestToMcl(message);
}

void RtcDataController::handleSetPreferredDataModem(const sp<RfxMessage>& message) {
    const int *pReqData = (const int *)message->getData()->getData();
    int preferredMdId = pReqData[0]; // 0 for slot0, 1 for slot1.
    char feature[] = "EDATASIM Supported";
    int isEDataSimSupported = getFeatureVersion(feature);
    RFX_LOG_D(RFX_LOG_TAG,
            "handleSetPreferredDataModem: preferred Modem=%d, EDATASIM supported=%d",
            preferredMdId, isEDataSimSupported);

    if (preferredMdId < 0 || preferredMdId >= RFX_SLOT_COUNT) {
        sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(message->getSlotId(),
                RFX_MSG_REQUEST_SET_PREFERRED_DATA_MODEM, RIL_E_INVALID_ARGUMENTS,
                RfxVoidData(), message);
        responseToRilj(responseMsg);
        return;
    }

    getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_PREFERRED_DATA_SIM,
            preferredMdId, true);

    if (isEDataSimSupported == 1) {
        message->setSlotId(0);
        requestToMcl(message);
    } else {
        sp<RfxMessage> responseMsg = RfxMessage::obtainResponse(message->getSlotId(),
                RFX_MSG_REQUEST_SET_PREFERRED_DATA_MODEM, RIL_E_SUCCESS,
                RfxVoidData(), message);
        responseToRilj(responseMsg);
    }
}

bool RtcDataController::onCheckIfRejectMessage(const sp<RfxMessage>& message,
        bool isModemPowerOff, int radioState) {
    int msgId = message->getId();
    if((radioState == (int)RADIO_STATE_OFF) &&
            (msgId == RFX_MSG_REQUEST_START_LCE ||
             msgId == RFX_MSG_REQUEST_STOP_LCE ||
             msgId == RFX_MSG_REQUEST_PULL_LCEDATA ||
             msgId == RFX_MSG_REQUEST_SYNC_DATA_SETTINGS_TO_MD ||
             msgId == RFX_MSG_REQUEST_SET_DATA_PROFILE ||
             msgId == RFX_MSG_REQUEST_SET_PREFERRED_DATA_MODEM ||
             msgId == RFX_MSG_REQUEST_SET_INITIAL_ATTACH_APN ||
            (RfxRilUtils::isWfcSupport() &&
             msgId == RFX_MSG_REQUEST_SETUP_DATA_CALL) ||
            (RfxRilUtils::isWfcSupport() &&
             msgId == RFX_MSG_REQUEST_DEACTIVATE_DATA_CALL) ||
             msgId == RFX_MSG_REQUEST_SET_LINK_CAPACITY_REPORTING_CRITERIA ||
             msgId == RFX_MSG_REQUEST_SEND_DEVICE_STATE ||
             msgId == RFX_MSG_REQUEST_SET_FD_MODE)) {
        return false;
    } else if ((radioState == (int)RADIO_STATE_UNAVAILABLE) &&
            (msgId == RFX_MSG_REQUEST_SYNC_DATA_SETTINGS_TO_MD ||
             msgId == RFX_MSG_REQUEST_SET_DATA_PROFILE ||
             msgId == RFX_MSG_REQUEST_SET_PREFERRED_DATA_MODEM ||
            (RfxRilUtils::isWfcSupport() &&
             msgId == RFX_MSG_REQUEST_SETUP_DATA_CALL) ||
            (RfxRilUtils::isWfcSupport() &&
             msgId == RFX_MSG_REQUEST_DEACTIVATE_DATA_CALL))) {
        return false;
    }
    return RfxController::onCheckIfRejectMessage(message, isModemPowerOff, radioState);
}

bool RtcDataController::onPreviewMessage(const sp<RfxMessage>& message) {
    if (canHandleRequest(message)) {
        // RFX_LOG_D(RFX_LOG_TAG, "onPreviewMessage: true");
        return true;
    }
    // RFX_LOG_D(RFX_LOG_TAG, "onPreviewMessage: false");
    return false;
}

bool RtcDataController::onCheckIfResumeMessage(const sp<RfxMessage>& message) {
    if (canHandleRequest(message)) {
        // RFX_LOG_D(RFX_LOG_TAG, "onCheckIfResumeMessage: true");
        return true;
    }
    // RFX_LOG_D(RFX_LOG_TAG, "onCheckIfResumeMessage: false");
    return false;
}

bool RtcDataController::canHandleRequest(const sp<RfxMessage>& message) {
    int msgId = message->getId();

    if (msgId == RFX_MSG_REQUEST_SYNC_DATA_SETTINGS_TO_MD) {
        //check sim switch
        if (isUnderCapabilitySwitch == true) {
            // RFX_LOG_D(RFX_LOG_TAG, "[%s] Is under sim switch, don't process DDS sync to MD.",
                // idToString(msgId));
            return false;
        }
    }
    // RFX_LOG_D(RFX_LOG_TAG, "canHandleRequest [%s] true.", idToString(msgId));
    return true;
}

void RtcDataController::onImsConfirmed(const sp<RfxMessage> message) {
    int slotId = message->getSlotId(); // get sim slot id.
    int defaultDataSim = getNonSlotScopeStatusManager()
            ->getIntValue(RFX_STATUS_KEY_DEFAULT_DATA_SIM); // get default data sim slot id.
    logD(RFX_LOG_TAG, "onImsConfirmed Slot: %d, ims preCheck Done", defaultDataSim);
}

void RtcDataController::requestResumeIms(const sp<RfxMessage> message) {
    RtcImsController *imsController;
    sp<RfxAction> action;
    int slot_id = m_slot_id;
    slot_id = ((m_slot_id == 0) ? 1 : 0);
    logD(RFX_LOG_TAG, "Resume IMS precheck, slotId=%d", slot_id);
    imsController = (RtcImsController *) findController(slot_id,
            RFX_OBJ_CLASS_INFO(RtcImsController));
    action = new RfxAction1<const sp<RfxMessage>>(this,
            &RtcDataController::onImsConfirmed, message);
    imsController->requestImsResume(slot_id, action);
    //RFX_LOG_D(RFX_LOG_TAG, "requestImsResume finished");
    requestTokenIdForDisableIms = INVALID_VALUE;
}

void RtcDataController::onAllowedChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    int allowed = value.asInt();

    RFX_LOG_I(RFX_LOG_TAG, "[%d]onAllowedChanged: allowed = %d", m_slot_id, allowed);
    if ((getStatusManager(m_slot_id)->getIntValue(RFX_STATUS_KEY_SLOT_ALLOW, 0)) == 1) {
        sp<RfxMessage> message = RfxMessage::obtainUrc(m_slot_id,
                RFX_MSG_URC_MD_DATA_RETRY_COUNT_RESET, RfxVoidData());
        responseToRilj(message);
    }
}

bool RtcDataController::isPreferredDataMode() {
    if (mIsPreferredDataMode != -1) {
        RFX_LOG_D(RFX_LOG_TAG, "[%d]isPreferredDataMode: %d", m_slot_id, mIsPreferredDataMode);
        return (mIsPreferredDataMode == 1) ? true : false;
    }
    char preferredDataMode[MTK_PROPERTY_VALUE_MAX] = {0};
    mtk_property_get("vendor.ril.data.preferred_data_mode", preferredDataMode, "0");
    RFX_LOG_D(RFX_LOG_TAG, "[%d]isPreferredDataMode: preferred_data_mode = %d", m_slot_id,
            atoi(preferredDataMode));
    if (atoi(preferredDataMode) != 1) {
        mIsPreferredDataMode = 0;
        return false;
    }
    mIsPreferredDataMode = 1;
    return true;
}

void RtcDataController::enqueueForPreferredDataMode(const sp<RfxMessage>& message) {
    char **pReqData = (char **) message->getData()->getData();
    int nProfileId = atoi(pReqData[1]);
    if (nProfileId == -1) {
        int supportedTypesBitmask = atoi(pReqData[8]);
        for (int j = 0; j < RIL_APN_TYPE_COUNT; j++) {
            if (supportedTypesBitmask & (1 << j)) {
                if ((1 << j) == RIL_APN_TYPE_DEFAULT) {
                    int preferSim = getNonSlotScopeStatusManager()->getIntValue(
                            RFX_STATUS_KEY_PREFERRED_DATA_SIM);
                    if (preferSim != m_slot_id) {
                        RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] non preferred data sim, skip default apn",
                                m_slot_id, __FUNCTION__);
                        continue;
                    }
                }
                nProfileId = getProfileID(1 << j);
                free(pReqData[1]);
                asprintf(&pReqData[1], "%d", nProfileId);
                break;
            }
        }
    }

    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] profileId: %d", m_slot_id, __FUNCTION__, nProfileId);

    if (nProfileId != RIL_DATA_PROFILE_IMS && nProfileId != RIL_DATA_PROFILE_VENDOR_VSIM
            && nProfileId != RIL_DATA_PROFILE_VENDOR_EMERGENCY) {
        if (nProfileId != RIL_DATA_PROFILE_DEFAULT && m_mapProfileIdToken.count(nProfileId) == 0) {
            RtcDataAllowController *pRtcDataAllowController;
            pRtcDataAllowController =
                    (RtcDataAllowController *) findController(message->getSlotId(),
                    RFX_OBJ_CLASS_INFO(RtcDataAllowController));
            pRtcDataAllowController->enqueueNetworkRequest(nProfileId, m_slot_id);
        }
        if ((getStatusManager(m_slot_id)->getIntValue(RFX_STATUS_KEY_SLOT_ALLOW, 0)) == 1) {
            m_mapProfileIdToken[nProfileId] = message->getToken();
            requestToMcl(message);
        } else {
            MTK_RIL_Data_Call_Response_v11* response =
                    (MTK_RIL_Data_Call_Response_v11*)calloc(1, sizeof(MTK_RIL_Data_Call_Response_v11));
            if (response == NULL) {
                RFX_LOG_E(RFX_LOG_TAG, "[%d][%s] allocate response failed", m_slot_id, __FUNCTION__);
                requestToMcl(message);
                return;
            }
            response->status = PDP_FAIL_DATA_NOT_ALLOW;
            response->suggestedRetryTime = -1;
            response->cid = -1;
            response->rat = 1;
            responseToRilj(RfxMessage::obtainResponse(m_slot_id, message->getId(),
                    RIL_E_SUCCESS, RfxDataCallResponseData(response, 1), message));
            free(response);
        }
    } else {
        requestToMcl(message);
    }
}

void RtcDataController::dequeueForPreferredDataMode(const sp<RfxMessage>& message) {
    const char **pReqData = (const char **) message->getData()->getData();
    int nCid = atoi(pReqData[0]);
    int nProfileId = 0;
    int nDataSim = 0;

    if (nCid <= 0) {
        nProfileId = nCid * -1;
    } else {
        nProfileId = m_mapCidProfileId[nCid];
    }

    RFX_LOG_I(RFX_LOG_TAG, "[%d][%s] cid: %d, profileId: %d", m_slot_id,
            __FUNCTION__, nCid, nProfileId);

    if (nProfileId == RIL_DATA_PROFILE_IMS || nProfileId == RIL_DATA_PROFILE_DEFAULT
            || nProfileId == RIL_DATA_PROFILE_VENDOR_EMERGENCY
            || nProfileId == RIL_DATA_PROFILE_VENDOR_VSIM) {
        m_mapCidProfileId.erase(nCid);
        m_mapProfileIdToken.erase(nProfileId);
        return;
    }

    RtcDataAllowController *pRtcDataAllowController =
            (RtcDataAllowController *) findController(message->getSlotId(),
            RFX_OBJ_CLASS_INFO(RtcDataAllowController));
    if (pRtcDataAllowController->dequeueNetworkRequest(nProfileId, m_slot_id)) {
        int nDataSim = getNonSlotScopeStatusManager()->getIntValue(
                    RFX_STATUS_KEY_DEFAULT_DATA_SIM);
        sp<RfxMessage> message = RfxMessage::obtainUrc(nDataSim,
                RFX_MSG_URC_MD_DATA_RETRY_COUNT_RESET, RfxVoidData());
        responseToRilj(message);
    }

    m_mapCidProfileId.erase(nCid);
    m_mapProfileIdToken.erase(nProfileId);
}

int RtcDataController::getProfileID(int apnTypeId) {
    switch (apnTypeId) {
        case RIL_APN_TYPE_DEFAULT: return RIL_DATA_PROFILE_DEFAULT;
        case RIL_APN_TYPE_DUN: return RIL_DATA_PROFILE_TETHERED;
        case RIL_APN_TYPE_FOTA: return RIL_DATA_PROFILE_FOTA;
        case RIL_APN_TYPE_IMS: return RIL_DATA_PROFILE_IMS;
        case RIL_APN_TYPE_CBS: return RIL_DATA_PROFILE_CBS;
        case RIL_APN_TYPE_MMS: return RIL_DATA_PROFILE_VENDOR_MMS;
        case RIL_APN_TYPE_SUPL: return RIL_DATA_PROFILE_VENDOR_SUPL;
        case RIL_APN_TYPE_XCAP: return RIL_DATA_PROFILE_VENDOR_XCAP;
        case RIL_APN_TYPE_BIP: return RIL_DATA_PROFILE_VENDOR_BIP;
        case RIL_APN_TYPE_HIPRI: return RIL_DATA_PROFILE_VENDOR_HIPRI;
        case RIL_APN_TYPE_EMERGENCY: return RIL_DATA_PROFILE_VENDOR_EMERGENCY;
        case RIL_APN_TYPE_WAP: return RIL_DATA_PROFILE_VENDOR_WAP;
        case RIL_APN_TYPE_RCS: return RIL_DATA_PROFILE_VENDOR_RCS;
        case RIL_APN_TYPE_VSIM: return RIL_DATA_PROFILE_VENDOR_VSIM;
        case RIL_APN_TYPE_MCX: return RIL_DATA_PROFILE_VENDOR_MCX;
        default: return RIL_DATA_PROFILE_DEFAULT;
    }
}
