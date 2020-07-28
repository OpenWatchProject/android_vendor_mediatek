/* MediaTek Inc. (C) 2018. All rights reserved.
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

#include "SecureElement.h"

#define LOG_TAG "SE-RadioResponse"
#include <log/log.h>

#include "selog.h"

#define DEBUG_RESPONSE 0
#define UNUSED(x) ( (void)(x) )

namespace android {
namespace hardware {
namespace secure_element {
namespace V1_0 {
namespace implementation {

RadioResponse::RadioResponse(sp<SERadioResponseHandler> handler, const hidl_string& name) {
    mHandler = handler;
    mSlotName = name;

    SELOGD("Created");
}

RadioResponse::~RadioResponse() {
    mHandler = nullptr;
}

// Methods from ::android::hardware::radio::V1_0::IRadioResponse follow.
Return<void> RadioResponse::getIccCardStatusResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::CardStatus& cardStatus) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    if (mHandler == nullptr)
        SELOGE("mHandler == nullptr");

    return mHandler->getIccCardStatusResponse(info, cardStatus);
}

Return<void> RadioResponse::getIccCardStatusResponse_1_2(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_2::CardStatus& cardStatus) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    if (mHandler == nullptr)
        SELOGE("mHandler == nullptr");

    return mHandler->getIccCardStatusResponse(info, cardStatus.base);
}

Return<void> RadioResponse::getIccCardStatusResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_4::CardStatus& cardStatus) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    if (mHandler == nullptr)
        SELOGE("mHandler == nullptr");

    return mHandler->getIccCardStatusResponse(info, cardStatus.base.base);
}

Return<void> RadioResponse::supplyIccPinForAppResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t remainingRetries) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(remainingRetries);

    return Void();
}

Return<void> RadioResponse::supplyIccPukForAppResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t remainingRetries) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(remainingRetries);

    return Void();
}

Return<void> RadioResponse::supplyIccPin2ForAppResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t remainingRetries) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(remainingRetries);

    return Void();
}

Return<void> RadioResponse::supplyIccPuk2ForAppResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t remainingRetries) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(remainingRetries);

    return Void();
}

Return<void> RadioResponse::changeIccPinForAppResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t remainingRetries) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(remainingRetries);

    return Void();
}

Return<void> RadioResponse::changeIccPin2ForAppResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t remainingRetries) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(remainingRetries);

    return Void();
}

Return<void> RadioResponse::supplyNetworkDepersonalizationResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t remainingRetries) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(remainingRetries);

    return Void();
}

Return<void> RadioResponse::getCurrentCallsResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_0::Call>& calls) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(calls);

    return Void();
}

Return<void> RadioResponse::dialResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);

    return Void();
}

Return<void> RadioResponse::getIMSIForAppResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_string& imsi) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(imsi);

    return Void();
}

Return<void> RadioResponse::hangupConnectionResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);

    return Void();
}

Return<void> RadioResponse::hangupWaitingOrBackgroundResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);

    return Void();
}

Return<void> RadioResponse::hangupForegroundResumeBackgroundResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);

    return Void();
}

Return<void> RadioResponse::switchWaitingOrHoldingAndActiveResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);

    return Void();
}

Return<void> RadioResponse::conferenceResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);

    return Void();
}

Return<void> RadioResponse::rejectCallResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);

    return Void();
}

Return<void> RadioResponse::getLastCallFailCauseResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::LastCallFailCauseInfo& failCauseinfo) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(failCauseinfo);

    return Void();
}

Return<void> RadioResponse::getSignalStrengthResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::SignalStrength& sigStrength) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(sigStrength);

    return Void();
}

Return<void> RadioResponse::getVoiceRegistrationStateResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::VoiceRegStateResult& voiceRegResponse) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(voiceRegResponse);

    return Void();
}

Return<void> RadioResponse::getDataRegistrationStateResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::DataRegStateResult& dataRegResponse) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(dataRegResponse);

    return Void();
}

Return<void> RadioResponse::getOperatorResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_string& longName, const hidl_string& shortName, const hidl_string& numeric) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(longName);
    UNUSED(shortName);
    UNUSED(numeric);

    return Void();
}

Return<void> RadioResponse::setRadioPowerResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);

    return Void();
}

Return<void> RadioResponse::sendDtmfResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);

    return Void();
}

Return<void> RadioResponse::sendSmsResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::SendSmsResult& sms) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(sms);

    return Void();
}

Return<void> RadioResponse::sendSMSExpectMoreResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::SendSmsResult& sms) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(sms);

    return Void();
}

Return<void> RadioResponse::setupDataCallResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::SetupDataCallResult& dcResponse) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(dcResponse);

    return Void();
}

Return<void> RadioResponse::iccIOForAppResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::IccIoResult& iccIo) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(iccIo);

    return Void();
}

Return<void> RadioResponse::sendUssdResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);

    return Void();
}

Return<void> RadioResponse::cancelPendingUssdResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);

    return Void();
}

Return<void> RadioResponse::getClirResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t n, int32_t m) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(n);
    UNUSED(m);

    return Void();
}

Return<void> RadioResponse::setClirResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);

    return Void();
}

Return<void> RadioResponse::getCallForwardStatusResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_0::CallForwardInfo>& callForwardInfos) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(callForwardInfos);

    return Void();
}

Return<void> RadioResponse::setCallForwardResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);

    return Void();
}

Return<void> RadioResponse::getCallWaitingResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, bool enable, int32_t serviceClass) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(enable);
    UNUSED(serviceClass);

    return Void();
}

Return<void> RadioResponse::setCallWaitingResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);

    return Void();
}

Return<void> RadioResponse::acknowledgeLastIncomingGsmSmsResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);

    return Void();
}

Return<void> RadioResponse::acceptCallResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);

    return Void();
}

Return<void> RadioResponse::deactivateDataCallResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);

    return Void();
}

Return<void> RadioResponse::getFacilityLockForAppResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t response) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(response);

    return Void();
}

Return<void> RadioResponse::setFacilityLockForAppResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t retry) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(retry);

    return Void();
}

Return<void> RadioResponse::setBarringPasswordResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);

    return Void();
}

Return<void> RadioResponse::getNetworkSelectionModeResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, bool manual) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(manual);

    return Void();
}

Return<void> RadioResponse::setNetworkSelectionModeAutomaticResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);

    return Void();
}

Return<void> RadioResponse::setNetworkSelectionModeManualResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);

    return Void();
}

Return<void> RadioResponse::getAvailableNetworksResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_0::OperatorInfo>& networkInfos) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    UNUSED(info);
    UNUSED(networkInfos);

    return Void();
}

Return<void> RadioResponse::startDtmfResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::stopDtmfResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::getBasebandVersionResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_string& version) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)version;

    return Void();
}

Return<void> RadioResponse::separateConnectionResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::setMuteResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::getMuteResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, bool enable) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)enable;

    return Void();
}

Return<void> RadioResponse::getClipResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, ::android::hardware::radio::V1_0::ClipStatus status) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)status;

    return Void();
}

Return<void> RadioResponse::getDataCallListResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_0::SetupDataCallResult>& dcResponse) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)dcResponse;

    return Void();
}

Return<void> RadioResponse::setSuppServiceNotificationsResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::writeSmsToSimResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t index) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)index;

    return Void();
}

Return<void> RadioResponse::deleteSmsOnSimResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::setBandModeResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::getAvailableBandModesResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_0::RadioBandMode>& bandModes) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)bandModes;

    return Void();
}

Return<void> RadioResponse::sendEnvelopeResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_string& commandResponse) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)commandResponse;

    return Void();
}

Return<void> RadioResponse::sendTerminalResponseToSimResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::handleStkCallSetupRequestFromSimResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::explicitCallTransferResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::setPreferredNetworkTypeResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::getPreferredNetworkTypeResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, ::android::hardware::radio::V1_0::PreferredNetworkType nwType) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)nwType;

    return Void();
}

Return<void> RadioResponse::getNeighboringCidsResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_0::NeighboringCell>& cells) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)cells;

    return Void();
}

Return<void> RadioResponse::setLocationUpdatesResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::setCdmaSubscriptionSourceResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::setCdmaRoamingPreferenceResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::getCdmaRoamingPreferenceResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, ::android::hardware::radio::V1_0::CdmaRoamingType type) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)type;

    return Void();
}

Return<void> RadioResponse::setTTYModeResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::getTTYModeResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, ::android::hardware::radio::V1_0::TtyMode mode) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)mode;

    return Void();
}

Return<void> RadioResponse::setPreferredVoicePrivacyResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::getPreferredVoicePrivacyResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, bool enable) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)enable;

    return Void();
}

Return<void> RadioResponse::sendCDMAFeatureCodeResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::sendBurstDtmfResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::sendCdmaSmsResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::SendSmsResult& sms) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)sms;

    return Void();
}

Return<void> RadioResponse::acknowledgeLastIncomingCdmaSmsResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::getGsmBroadcastConfigResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_0::GsmBroadcastSmsConfigInfo>& configs) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)configs;

    return Void();
}

Return<void> RadioResponse::setGsmBroadcastConfigResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::setGsmBroadcastActivationResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::getCdmaBroadcastConfigResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_0::CdmaBroadcastSmsConfigInfo>& configs) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)configs;

    return Void();
}

Return<void> RadioResponse::setCdmaBroadcastConfigResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::setCdmaBroadcastActivationResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::getCDMASubscriptionResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_string& mdn, const hidl_string& hSid, const hidl_string& hNid, const hidl_string& min, const hidl_string& prl) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)mdn;
    (void)hSid;
    (void)hNid;
    (void)min;
    (void)prl;

    return Void();
}

Return<void> RadioResponse::writeSmsToRuimResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, uint32_t index) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)index;

    return Void();
}

Return<void> RadioResponse::deleteSmsOnRuimResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::getDeviceIdentityResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_string& imei, const hidl_string& imeisv, const hidl_string& esn, const hidl_string& meid) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)imei;
    (void)imeisv;
    (void)esn;
    (void)meid;

    return Void();
}

Return<void> RadioResponse::exitEmergencyCallbackModeResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::getSmscAddressResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_string& smsc) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)smsc;

    return Void();
}

Return<void> RadioResponse::setSmscAddressResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::reportSmsMemoryStatusResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::reportStkServiceIsRunningResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::getCdmaSubscriptionSourceResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, ::android::hardware::radio::V1_0::CdmaSubscriptionSource source) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)source;

    return Void();
}

Return<void> RadioResponse::requestIsimAuthenticationResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_string& response) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)response;

    return Void();
}

Return<void> RadioResponse::acknowledgeIncomingGsmSmsWithPduResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::sendEnvelopeWithStatusResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::IccIoResult& iccIo) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)iccIo;

    return Void();
}

Return<void> RadioResponse::getVoiceRadioTechnologyResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, ::android::hardware::radio::V1_0::RadioTechnology rat) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)rat;

    return Void();
}

Return<void> RadioResponse::getCellInfoListResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_0::CellInfo>& cellInfo) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)cellInfo;

    return Void();
}

Return<void> RadioResponse::setCellInfoListRateResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::setInitialAttachApnResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::getImsRegistrationStateResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, bool isRegistered, ::android::hardware::radio::V1_0::RadioTechnologyFamily ratFamily) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)isRegistered;
    (void)ratFamily;

    return Void();
}

Return<void> RadioResponse::sendImsSmsResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::SendSmsResult& sms) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)sms;

    return Void();
}

Return<void> RadioResponse::iccTransmitApduBasicChannelResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::IccIoResult& result) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    if (mHandler == nullptr)
        SELOGE("mHandler == nullptr");

    return mHandler->iccTransmitApduBasicChannelResponse(info, result);
}

Return<void> RadioResponse::iccOpenLogicalChannelResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t channelId, const hidl_vec<int8_t>& selectResponse) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    if (mHandler == nullptr)
        SELOGE("mHandler == nullptr");

    return mHandler->iccOpenLogicalChannelResponse(info, channelId, selectResponse);
}

Return<void> RadioResponse::iccCloseLogicalChannelResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    if (mHandler == nullptr)
        SELOGE("mHandler == nullptr");

    return mHandler->iccCloseLogicalChannelResponse(info);
}

Return<void> RadioResponse::iccTransmitApduLogicalChannelResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::IccIoResult& result) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    if (mHandler == nullptr)
        SELOGE("mHandler == nullptr");

    return mHandler->iccTransmitApduLogicalChannelResponse(info, result);
}

Return<void> RadioResponse::nvReadItemResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_string& result) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)result;

    return Void();
}

Return<void> RadioResponse::nvWriteItemResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::nvWriteCdmaPrlResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::nvResetConfigResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::setUiccSubscriptionResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::setDataAllowedResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::getHardwareConfigResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_0::HardwareConfig>& config) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)config;

    return Void();
}

Return<void> RadioResponse::requestIccSimAuthenticationResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::IccIoResult& result) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)result;

    return Void();
}

Return<void> RadioResponse::setDataProfileResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::requestShutdownResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::getRadioCapabilityResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::RadioCapability& rc) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)rc;

    return Void();
}

Return<void> RadioResponse::setRadioCapabilityResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::RadioCapability& rc) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)rc;

    return Void();
}

Return<void> RadioResponse::startLceServiceResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::LceStatusInfo& statusInfo) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)statusInfo;

    return Void();
}

Return<void> RadioResponse::stopLceServiceResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::LceStatusInfo& statusInfo) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)statusInfo;

    return Void();
}

Return<void> RadioResponse::pullLceDataResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::LceDataInfo& lceInfo) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)lceInfo;

    return Void();
}

Return<void> RadioResponse::getModemActivityInfoResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::ActivityStatsInfo& activityInfo) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)activityInfo;

    return Void();
}

Return<void> RadioResponse::setAllowedCarriersResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t numAllowed) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)numAllowed;

    return Void();
}

Return<void> RadioResponse::getAllowedCarriersResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, bool allAllowed, const ::android::hardware::radio::V1_0::CarrierRestrictions& carriers) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)allAllowed;
    (void)carriers;

    return Void();
}

Return<void> RadioResponse::sendDeviceStateResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::setIndicationFilterResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::setSimCardPowerResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::acknowledgeRequest(int32_t serial) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)serial;

    return Void();
}


// Methods from ::android::hardware::radio::V1_1::IRadioResponse follow.
Return<void> RadioResponse::setCarrierInfoForImsiEncryptionResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::setSimCardPowerResponse_1_1(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::startNetworkScanResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::stopNetworkScanResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::startKeepaliveResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_1::KeepaliveStatus& status) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)status;

    return Void();
}

Return<void> RadioResponse::stopKeepaliveResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}


// Methods from ::android::hardware::radio::V1_2::IRadioResponse follow.
Return<void> RadioResponse::getCellInfoListResponse_1_2(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_2::CellInfo>& cellInfo) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)cellInfo;

    return Void();
}

Return<void> RadioResponse::setSignalStrengthReportingCriteriaResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::setLinkCapacityReportingCriteriaResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;

    return Void();
}

Return<void> RadioResponse::getCurrentCallsResponse_1_2(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_2::Call>& calls) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)calls;

    return Void();
}

Return<void> RadioResponse::getSignalStrengthResponse_1_2(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_2::SignalStrength& signalStrength) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)signalStrength;

    return Void();
}

Return<void> RadioResponse::getVoiceRegistrationStateResponse_1_2(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_2::VoiceRegStateResult& voiceRegResponse) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)voiceRegResponse;

    return Void();
}

Return<void> RadioResponse::getDataRegistrationStateResponse_1_2(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_2::DataRegStateResult& dataRegResponse) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)dataRegResponse;

    return Void();
}

Return<void> RadioResponse::setSystemSelectionChannelsResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    return Void();
}

Return<void> RadioResponse::enableModemResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    return Void();
}

Return<void> RadioResponse::getModemStackStatusResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, bool isEnabled) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)isEnabled;
    return Void();
}

Return<void> RadioResponse::emergencyDialResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    return Void();
}

Return<void> RadioResponse::startNetworkScanResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    return Void();
}

Return<void> RadioResponse::getCellInfoListResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_4::CellInfo>& cellInfo) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)cellInfo;
    return Void();
}

Return<void> RadioResponse::getDataRegistrationStateResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_4::DataRegStateResult& dataRegResponse) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)dataRegResponse;
    return Void();
}

Return<void> RadioResponse::getPreferredNetworkTypeBitmapResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_4::RadioAccessFamily> networkTypeBitmap) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)networkTypeBitmap;
    return Void();
}

Return<void> RadioResponse::setPreferredNetworkTypeBitmapResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    return Void();
}

Return<void> RadioResponse::getDataCallListResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_4::SetupDataCallResult>& dcResponse) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)dcResponse;
    return Void();
}

Return<void> RadioResponse::setupDataCallResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_4::SetupDataCallResult& dcResponse) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)dcResponse;
    return Void();
}

Return<void> RadioResponse::setAllowedCarriersResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    return Void();
}

Return<void> RadioResponse::getAllowedCarriersResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_4::CarrierRestrictionsWithPriority& carriers, ::android::hardware::radio::V1_4::SimLockMultiSimPolicy multiSimPolicy) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)carriers;
    (void)multiSimPolicy;
    return Void();
}

Return<void> RadioResponse::getSignalStrengthResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_4::SignalStrength& signalStrength) {
#if DEBUG_RESPONSE
    SELOGD("enter");
#endif
    (void)info;
    (void)signalStrength;
    return Void();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace secure_element
}  // namespace hardware
}  // namespace android
