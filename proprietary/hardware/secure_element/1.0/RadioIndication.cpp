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

#define LOG_TAG "SE-RadioInd"
#include <log/log.h>
#include "selog.h"

#define UNUSED(x) ( (void)(x) )

namespace android {
namespace hardware {
namespace secure_element {
namespace V1_0 {
namespace implementation {

RadioIndication::RadioIndication(sp<SecureElement> se, hidl_string name) {
    mSE = se;
    mSlotName = name;

    SELOGD("Created");
}

RadioIndication::~RadioIndication() {
    mSE = nullptr;
}


// Methods from ::android::hardware::radio::V1_0::IRadioIndication follow.
Return<void> RadioIndication::radioStateChanged(::android::hardware::radio::V1_0::RadioIndicationType type, ::android::hardware::radio::V1_0::RadioState radioState) {
    UNUSED(type);
    UNUSED(radioState);

    return Void();
}

Return<void> RadioIndication::callStateChanged(::android::hardware::radio::V1_0::RadioIndicationType type) {
    UNUSED(type);

    return Void();
}

Return<void> RadioIndication::networkStateChanged(::android::hardware::radio::V1_0::RadioIndicationType type) {
    UNUSED(type);

    return Void();
}

Return<void> RadioIndication::newSms(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_vec<uint8_t>& pdu) {
    UNUSED(type);
    UNUSED(pdu);

    return Void();
}

Return<void> RadioIndication::newSmsStatusReport(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_vec<uint8_t>& pdu) {
    UNUSED(type);
    UNUSED(pdu);

    return Void();
}

Return<void> RadioIndication::newSmsOnSim(::android::hardware::radio::V1_0::RadioIndicationType type, int32_t recordNumber) {
    UNUSED(type);
    UNUSED(recordNumber);

    return Void();
}

Return<void> RadioIndication::onUssd(::android::hardware::radio::V1_0::RadioIndicationType type, ::android::hardware::radio::V1_0::UssdModeType modeType, const hidl_string& msg) {
    UNUSED(type);
    UNUSED(modeType);
    UNUSED(msg);

    return Void();
}

Return<void> RadioIndication::nitzTimeReceived(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_string& nitzTime, uint64_t receivedTime) {
    UNUSED(type);
    UNUSED(nitzTime);
    UNUSED(receivedTime);

    return Void();
}

Return<void> RadioIndication::currentSignalStrength(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_0::SignalStrength& signalStrength) {
    UNUSED(type);
    UNUSED(signalStrength);

    return Void();
}

Return<void> RadioIndication::dataCallListChanged(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_vec<::android::hardware::radio::V1_0::SetupDataCallResult>& dcList) {
    UNUSED(type);
    UNUSED(dcList);

    return Void();
}

Return<void> RadioIndication::suppSvcNotify(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_0::SuppSvcNotification& suppSvc) {
    UNUSED(type);
    UNUSED(suppSvc);

    return Void();
}

Return<void> RadioIndication::stkSessionEnd(::android::hardware::radio::V1_0::RadioIndicationType type) {
    UNUSED(type);

    return Void();
}

Return<void> RadioIndication::stkProactiveCommand(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_string& cmd) {
    UNUSED(type);
    UNUSED(cmd);

    return Void();
}

Return<void> RadioIndication::stkEventNotify(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_string& cmd) {
    UNUSED(type);
    UNUSED(cmd);

    return Void();
}

Return<void> RadioIndication::stkCallSetup(::android::hardware::radio::V1_0::RadioIndicationType type, int64_t timeout) {
    UNUSED(type);
    UNUSED(timeout);

    return Void();
}

Return<void> RadioIndication::simSmsStorageFull(::android::hardware::radio::V1_0::RadioIndicationType type) {
    UNUSED(type);

    return Void();
}

Return<void> RadioIndication::simRefresh(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_0::SimRefreshResult& refreshResult) {
    UNUSED(type);
    UNUSED(refreshResult);

    return Void();
}

Return<void> RadioIndication::callRing(::android::hardware::radio::V1_0::RadioIndicationType type, bool isGsm, const ::android::hardware::radio::V1_0::CdmaSignalInfoRecord& record) {
    UNUSED(type);
    UNUSED(isGsm);
    UNUSED(record);

    return Void();
}

Return<void> RadioIndication::simStatusChanged(::android::hardware::radio::V1_0::RadioIndicationType type) {
    SELOGD("simStatusChanged stats = %d", type);
    mSE->handleCallbackThread(false);

    return Void();
}

Return<void> RadioIndication::cdmaNewSms(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_0::CdmaSmsMessage& msg) {
    UNUSED(type);
    UNUSED(msg);

    return Void();
}

Return<void> RadioIndication::newBroadcastSms(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_vec<uint8_t>& data) {
    UNUSED(type);
    UNUSED(data);

    return Void();
}

Return<void> RadioIndication::cdmaRuimSmsStorageFull(::android::hardware::radio::V1_0::RadioIndicationType type) {
    UNUSED(type);

    return Void();
}

Return<void> RadioIndication::restrictedStateChanged(::android::hardware::radio::V1_0::RadioIndicationType type, ::android::hardware::radio::V1_0::PhoneRestrictedState state) {
    UNUSED(type);
    UNUSED(state);

    return Void();
}

Return<void> RadioIndication::enterEmergencyCallbackMode(::android::hardware::radio::V1_0::RadioIndicationType type) {
    UNUSED(type);

    return Void();
}

Return<void> RadioIndication::cdmaCallWaiting(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_0::CdmaCallWaiting& callWaitingRecord) {
    UNUSED(type);
    UNUSED(callWaitingRecord);

    return Void();
}

Return<void> RadioIndication::cdmaOtaProvisionStatus(::android::hardware::radio::V1_0::RadioIndicationType type, ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus status) {
    UNUSED(type);
    UNUSED(status);

    return Void();
}

Return<void> RadioIndication::cdmaInfoRec(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_0::CdmaInformationRecords& records) {
    UNUSED(type);
    UNUSED(records);

    return Void();
}

Return<void> RadioIndication::indicateRingbackTone(::android::hardware::radio::V1_0::RadioIndicationType type, bool start) {
    UNUSED(type);
    UNUSED(start);

    return Void();
}

Return<void> RadioIndication::resendIncallMute(::android::hardware::radio::V1_0::RadioIndicationType type) {
    UNUSED(type);

    return Void();
}

Return<void> RadioIndication::cdmaSubscriptionSourceChanged(::android::hardware::radio::V1_0::RadioIndicationType type, ::android::hardware::radio::V1_0::CdmaSubscriptionSource cdmaSource) {
    UNUSED(type);
    UNUSED(cdmaSource);

    return Void();
}

Return<void> RadioIndication::cdmaPrlChanged(::android::hardware::radio::V1_0::RadioIndicationType type, int32_t version) {
    UNUSED(type);
    UNUSED(version);

    return Void();
}

Return<void> RadioIndication::exitEmergencyCallbackMode(::android::hardware::radio::V1_0::RadioIndicationType type) {
    UNUSED(type);

    return Void();
}

Return<void> RadioIndication::rilConnected(::android::hardware::radio::V1_0::RadioIndicationType type) {
    UNUSED(type);

    return Void();
}

Return<void> RadioIndication::voiceRadioTechChanged(::android::hardware::radio::V1_0::RadioIndicationType type, ::android::hardware::radio::V1_0::RadioTechnology rat) {
    UNUSED(type);
    UNUSED(rat);

    return Void();
}

Return<void> RadioIndication::cellInfoList(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_vec<::android::hardware::radio::V1_0::CellInfo>& records) {
    UNUSED(type);
    UNUSED(records);

    return Void();
}

Return<void> RadioIndication::imsNetworkStateChanged(::android::hardware::radio::V1_0::RadioIndicationType type) {
    UNUSED(type);

    return Void();
}

Return<void> RadioIndication::subscriptionStatusChanged(::android::hardware::radio::V1_0::RadioIndicationType type, bool activate) {
    UNUSED(type);
    UNUSED(activate);

    return Void();
}

Return<void> RadioIndication::srvccStateNotify(::android::hardware::radio::V1_0::RadioIndicationType type, ::android::hardware::radio::V1_0::SrvccState state) {
    UNUSED(type);
    UNUSED(state);

    return Void();
}

Return<void> RadioIndication::hardwareConfigChanged(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_vec<::android::hardware::radio::V1_0::HardwareConfig>& configs) {
    UNUSED(type);
    UNUSED(configs);

    return Void();
}

Return<void> RadioIndication::radioCapabilityIndication(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_0::RadioCapability& rc) {
    UNUSED(type);
    UNUSED(rc);

    return Void();
}

Return<void> RadioIndication::onSupplementaryServiceIndication(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_0::StkCcUnsolSsResult& ss) {
    UNUSED(type);
    UNUSED(ss);

    return Void();
}

Return<void> RadioIndication::stkCallControlAlphaNotify(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_string& alpha) {
    UNUSED(type);
    UNUSED(alpha);

    return Void();
}

Return<void> RadioIndication::lceData(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_0::LceDataInfo& lce) {
    UNUSED(type);
    UNUSED(lce);

    return Void();
}

Return<void> RadioIndication::pcoData(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_0::PcoDataInfo& pco) {
    UNUSED(type);
    UNUSED(pco);

    return Void();
}

Return<void> RadioIndication::modemReset(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_string& reason) {
    UNUSED(type);
    UNUSED(reason);

    return Void();
}


// Methods from ::android::hardware::radio::V1_1::IRadioIndication follow.
Return<void> RadioIndication::carrierInfoForImsiEncryption(::android::hardware::radio::V1_0::RadioIndicationType info) {
    UNUSED(info);

    return Void();
}

Return<void> RadioIndication::networkScanResult(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_1::NetworkScanResult& result) {
    UNUSED(type);
    UNUSED(result);

    return Void();
}

Return<void> RadioIndication::keepaliveStatus(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_1::KeepaliveStatus& status) {
    UNUSED(type);
    UNUSED(status);

    return Void();
}


// Methods from ::android::hardware::radio::V1_2::IRadioIndication follow.
Return<void> RadioIndication::networkScanResult_1_2(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_2::NetworkScanResult& result) {
    UNUSED(type);
    UNUSED(result);

    return Void();
}

Return<void> RadioIndication::cellInfoList_1_2(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_vec<::android::hardware::radio::V1_2::CellInfo>& records) {
    UNUSED(type);
    UNUSED(records);

    return Void();
}

Return<void> RadioIndication::currentLinkCapacityEstimate(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_2::LinkCapacityEstimate& lce) {
    UNUSED(type);
    UNUSED(lce);

    return Void();
}

Return<void> RadioIndication::currentPhysicalChannelConfigs(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_vec<::android::hardware::radio::V1_2::PhysicalChannelConfig>& configs) {
    UNUSED(type);
    UNUSED(configs);

    return Void();
}

Return<void> RadioIndication::currentSignalStrength_1_2(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_2::SignalStrength& signalStrength) {
    UNUSED(type);
    UNUSED(signalStrength);

    return Void();
}

Return<void> RadioIndication::currentEmergencyNumberList(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_4::EmergencyNumber>& emergencyNumberList) {
    UNUSED(type);
    UNUSED(emergencyNumberList);
    return Void();
}

Return<void> RadioIndication::cellInfoList_1_4(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_4::CellInfo>& records) {
    UNUSED(type);
    UNUSED(records);
    return Void();
}

Return<void> RadioIndication::networkScanResult_1_4(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_4::NetworkScanResult& result) {
    UNUSED(type);
    UNUSED(result);
    return Void();
}

Return<void> RadioIndication::currentPhysicalChannelConfigs_1_4(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_4::PhysicalChannelConfig>& configs) {
    UNUSED(type);
    UNUSED(configs);
    return Void();
}

Return<void> RadioIndication::dataCallListChanged_1_4(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_4::SetupDataCallResult>& dcList) {
    UNUSED(type);
    UNUSED(dcList);
    return Void();
}

Return<void> RadioIndication::currentSignalStrength_1_4(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_4::SignalStrength& signalStrength) {
    UNUSED(type);
    UNUSED(signalStrength);
    return Void();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace secure_element
}  // namespace hardware
}  // namespace android
