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

#ifndef ANDROID_HARDWARE_SECURE_ELEMENT_V1_0_SECUREELEMENT_H
#define ANDROID_HARDWARE_SECURE_ELEMENT_V1_0_SECUREELEMENT_H

#include <android/hardware/secure_element/1.0/ISecureElement.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/RefBase.h>

#include <android/hardware/radio/1.4/IRadio.h>
#include <android/hardware/radio/1.4/IRadioResponse.h>
#include <android/hardware/radio/1.4/IRadioIndication.h>

#include <vendor/mediatek/hardware/mtkradioex/1.0/IMtkRadioEx.h>
#include <vendor/mediatek/hardware/mtkradioex/1.0/ISERadioIndication.h>
#include <vendor/mediatek/hardware/mtkradioex/1.0/ISERadioResponse.h>

#include <pthread.h>
#include <time.h>

#define MAX_SIM_COUNT         (4)
#define MAX_SE_BINDER_THREADS (15)
#define RESPONSE_QUEUE_SIZE   (2)
#define WAIT_TIMEOUT_MS       (0) /* <= 0 for no timeout */

#define MAX_SERIAL_NUMBER                 (0xFFFF)
#define MAX_TRY_RADIO_TIMES               (120)
extern bool IS_DEBUG;

namespace AOSP_V1_0 = android::hardware::radio::V1_0;
namespace AOSP_V1_2 = android::hardware::radio::V1_2;
namespace AOSP_V1_4 = android::hardware::radio::V1_4;
namespace VENDOR_V1_0 = vendor::mediatek::hardware::mtkradioex::V1_0;

using namespace std;

namespace android {
namespace hardware {
namespace secure_element {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

enum SERadioResponseType {
    SE_ICC_CARD_STATUS = 0,
    SE_ICC_GET_ATR,
    SE_ICC_TRANSMIT_APDU_BASIC,
    SE_ICC_OPEN_CHANNEL,
    SE_ICC_CLOSE_CHANNEL,
    SE_ICC_TRANSMIT_APDU_LOGICAL,
};

struct SERadioResponseData {
    AOSP_V1_0::RadioResponseInfo info;
    AOSP_V1_0::CardStatus cardStatus;
    AOSP_V1_0::IccIoResult result;
    int32_t channelId;
    hidl_vec<int8_t> selectResponse;
    hidl_string response;
};

struct SERadioResponseQueueItem {
    enum SERadioResponseType type;
    bool handled;
    struct SERadioResponseData data;
};

struct SERadioResponseHandler;
struct RadioDeathRecipient;

struct SERadioResponse;
struct SERadioIndication;
struct RadioResponse;
struct RadioIndication;

struct SecureElement : public ISecureElement, public hidl_death_recipient {
    SecureElement(const int& id);
    ~SecureElement();
    void notifyResponse();
    void radioServiceDied();
    static void respTimeout(union sigval);
    void handleCallbackThread(bool force);

    // Methods from ::android::hardware::secure_element::V1_0::ISecureElement follow.
    Return<void> init(const sp<::android::hardware::secure_element::V1_0::ISecureElementHalCallback>& clientCallback) override;
    Return<void> getAtr(getAtr_cb _hidl_cb) override;
    Return<bool> isCardPresent() override;
    Return<void> transmit(const hidl_vec<uint8_t>& data, transmit_cb _hidl_cb) override;
    Return<void> openLogicalChannel(const hidl_vec<uint8_t>& aid, uint8_t p2, openLogicalChannel_cb _hidl_cb) override;
    Return<void> openBasicChannel(const hidl_vec<uint8_t>& aid, uint8_t p2, openBasicChannel_cb _hidl_cb) override;
    Return<::android::hardware::secure_element::V1_0::SecureElementStatus> closeChannel(uint8_t channelNumber) override;

    // Methods from ::android::hidl::hardware::hidl_death_recipient follow.
    void serviceDied(uint64_t /* cookie */, const ::android::wp<::android::hidl::base::V1_0::IBase>& /* who */) override;

private:
    sp<V1_0::ISecureElementHalCallback> mCallback_V1_0;
    sp<SERadioResponseHandler> mSERadioRspHandler;
    sp<RadioDeathRecipient> mMtkRadioDeathRecipient;
    sp<RadioDeathRecipient> mRadioDeathRecipient;

    sp<VENDOR_V1_0::IMtkRadioEx> mMtkRadio;
    sp<SERadioResponse> mMtkRadioRsp;
    sp<SERadioIndication> mMtkRadioInd;
    sp<AOSP_V1_4::IRadio> mRadio;
    sp<RadioResponse> mRadioRsp;
    sp<RadioIndication> mRadioInd;

    pthread_mutex_t mMainMutex;
    pthread_mutex_t mRadioMutex;
    pthread_mutex_t mCallbackMutex;
    pthread_mutex_t mRespMutex;
    pthread_cond_t mRespCond;

    bool mIsReset;

    int mUiccId;
    hidl_string mSlotName;
    int32_t mSerial;
    AOSP_V1_0::CardState mCardState;

    timer_t mTimerId;
    bool mTimerValid;
    /* Add sim state change event */
    bool mSimPreviousState;

    // Callback Handler to fire first SIM state event
    static void *runCallbackThread(void *data);

    // Handler
    void reconnect();
    static void *runMainThread(void *data);

    // Private methods follow.
    bool transmitInternalLocked(hidl_vec<uint8_t>& respVec, const hidl_vec<uint8_t>& data);
    bool getIccCardStatusLocked();
    bool isCardPresentInternal();
    bool initRadioService();
    bool initResponseFunctionsLocked();
    void byteVectorToHexString(hidl_string& str, const hidl_vec<uint8_t>& vec, size_t offset, size_t length);
    void byteVectorToHexString(hidl_string& str, const hidl_vec<uint8_t>& vec);
    uint8_t hexCharToByte(const char& hex);
    void hexStringToByteVector(hidl_vec<uint8_t>& vec, const hidl_string& str, size_t offset, size_t length);
    void hexStringToByteVector(hidl_vec<uint8_t>& vec, const hidl_string& str);
    void iccIoResultToByteVector(hidl_vec<uint8_t>& vec, const AOSP_V1_0::IccIoResult& result);
    uint8_t setChannelToClassByte(const uint8_t& cla, int channelNumber);
    uint8_t clearChannelNumber(const uint8_t& cla);
    uint8_t parseChannelNumber(const uint8_t& cla);
    bool createSimApdu(AOSP_V1_0::SimApdu& apdu, const hidl_vec<uint8_t>& data);
    bool reissueCommandLocked(hidl_vec<uint8_t>& respVec, const hidl_vec<uint8_t>& data, uint8_t Le);
    void resetAll(const char* reason);
    void resetRadio(const char* reason);
    void reset(bool destroy, bool resetAll, const char* reason);
    int32_t getSerialAndIncrementLocked();
    void startWaitTimerLocked();
    void clearWaitTimerLocked();
    bool waitResponseLocked(SERadioResponseData& data, enum SERadioResponseType type, int32_t serial);
    bool isRadioAliveLocked();
};

struct SERadioResponseHandler : public RefBase {
    SERadioResponseHandler(sp<SecureElement> se, const hidl_string& name);
    ~SERadioResponseHandler();
    bool removeResponse(struct SERadioResponseData& data, enum SERadioResponseType type, int32_t serial);

    // Methods for response
    Return<void> getIccCardStatusResponse(const AOSP_V1_0::RadioResponseInfo& info, const AOSP_V1_0::CardStatus& cardStatus);
    Return<void> getATRResponse(const AOSP_V1_0::RadioResponseInfo& info, const hidl_string& response);
    Return<void> iccTransmitApduBasicChannelResponse(const AOSP_V1_0::RadioResponseInfo& info, const AOSP_V1_0::IccIoResult& result);
    Return<void> iccOpenLogicalChannelResponse(const AOSP_V1_0::RadioResponseInfo& info, int32_t channelId, const hidl_vec<int8_t>& selectResponse);
    Return<void> iccCloseLogicalChannelResponse(const AOSP_V1_0::RadioResponseInfo& info);
    Return<void> iccTransmitApduLogicalChannelResponse(const AOSP_V1_0::RadioResponseInfo& info, const AOSP_V1_0::IccIoResult& result);

private:
    sp<SecureElement> mSE;
    hidl_string mSlotName;
    hidl_array<SERadioResponseQueueItem, RESPONSE_QUEUE_SIZE> mQueue;
    uint8_t mQueueNextIndex;
    pthread_mutex_t mQueueMutex;

    // Private methods follow.
    void addToQueue(enum SERadioResponseType type, const SERadioResponseData& data);
};

struct SERadioResponse : public VENDOR_V1_0::ISERadioResponse {
    SERadioResponse(sp<SERadioResponseHandler> handler, const hidl_string& name);
    ~SERadioResponse();

    // Methods from ::vendor::mediatek::hardware::mtkradioex::V1_0::ISERadioResponse follow.
    Return<void> getATRResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_string& response) override;

private:
    sp<SERadioResponseHandler> mHandler;
    hidl_string mSlotName;
};

struct RadioResponse : public AOSP_V1_4::IRadioResponse {
    RadioResponse(sp<SERadioResponseHandler> handler, const hidl_string& name);
    ~RadioResponse();

    // Methods from ::android::hardware::radio::V1_0::IRadioResponse follow.
    Return<void> getIccCardStatusResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::CardStatus& cardStatus) override;
    Return<void> supplyIccPinForAppResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t remainingRetries) override;
    Return<void> supplyIccPukForAppResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t remainingRetries) override;
    Return<void> supplyIccPin2ForAppResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t remainingRetries) override;
    Return<void> supplyIccPuk2ForAppResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t remainingRetries) override;
    Return<void> changeIccPinForAppResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t remainingRetries) override;
    Return<void> changeIccPin2ForAppResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t remainingRetries) override;
    Return<void> supplyNetworkDepersonalizationResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t remainingRetries) override;
    Return<void> getCurrentCallsResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_0::Call>& calls) override;
    Return<void> dialResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getIMSIForAppResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_string& imsi) override;
    Return<void> hangupConnectionResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> hangupWaitingOrBackgroundResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> hangupForegroundResumeBackgroundResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> switchWaitingOrHoldingAndActiveResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> conferenceResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> rejectCallResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getLastCallFailCauseResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::LastCallFailCauseInfo& failCauseinfo) override;
    Return<void> getSignalStrengthResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::SignalStrength& sigStrength) override;
    Return<void> getVoiceRegistrationStateResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::VoiceRegStateResult& voiceRegResponse) override;
    Return<void> getDataRegistrationStateResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::DataRegStateResult& dataRegResponse) override;
    Return<void> getOperatorResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_string& longName, const hidl_string& shortName, const hidl_string& numeric) override;
    Return<void> setRadioPowerResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> sendDtmfResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> sendSmsResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::SendSmsResult& sms) override;
    Return<void> sendSMSExpectMoreResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::SendSmsResult& sms) override;
    Return<void> setupDataCallResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::SetupDataCallResult& dcResponse) override;
    Return<void> iccIOForAppResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::IccIoResult& iccIo) override;
    Return<void> sendUssdResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> cancelPendingUssdResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getClirResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t n, int32_t m) override;
    Return<void> setClirResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getCallForwardStatusResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_0::CallForwardInfo>& callForwardInfos) override;
    Return<void> setCallForwardResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getCallWaitingResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, bool enable, int32_t serviceClass) override;
    Return<void> setCallWaitingResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> acknowledgeLastIncomingGsmSmsResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> acceptCallResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> deactivateDataCallResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getFacilityLockForAppResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t response) override;
    Return<void> setFacilityLockForAppResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t retry) override;
    Return<void> setBarringPasswordResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getNetworkSelectionModeResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, bool manual) override;
    Return<void> setNetworkSelectionModeAutomaticResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> setNetworkSelectionModeManualResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getAvailableNetworksResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_0::OperatorInfo>& networkInfos) override;
    Return<void> startDtmfResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> stopDtmfResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getBasebandVersionResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_string& version) override;
    Return<void> separateConnectionResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> setMuteResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getMuteResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, bool enable) override;
    Return<void> getClipResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, ::android::hardware::radio::V1_0::ClipStatus status) override;
    Return<void> getDataCallListResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_0::SetupDataCallResult>& dcResponse) override;
    Return<void> setSuppServiceNotificationsResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> writeSmsToSimResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t index) override;
    Return<void> deleteSmsOnSimResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> setBandModeResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getAvailableBandModesResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_0::RadioBandMode>& bandModes) override;
    Return<void> sendEnvelopeResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_string& commandResponse) override;
    Return<void> sendTerminalResponseToSimResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> handleStkCallSetupRequestFromSimResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> explicitCallTransferResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> setPreferredNetworkTypeResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getPreferredNetworkTypeResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, ::android::hardware::radio::V1_0::PreferredNetworkType nwType) override;
    Return<void> getNeighboringCidsResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_0::NeighboringCell>& cells) override;
    Return<void> setLocationUpdatesResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> setCdmaSubscriptionSourceResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> setCdmaRoamingPreferenceResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getCdmaRoamingPreferenceResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, ::android::hardware::radio::V1_0::CdmaRoamingType type) override;
    Return<void> setTTYModeResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getTTYModeResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, ::android::hardware::radio::V1_0::TtyMode mode) override;
    Return<void> setPreferredVoicePrivacyResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getPreferredVoicePrivacyResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, bool enable) override;
    Return<void> sendCDMAFeatureCodeResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> sendBurstDtmfResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> sendCdmaSmsResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::SendSmsResult& sms) override;
    Return<void> acknowledgeLastIncomingCdmaSmsResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getGsmBroadcastConfigResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_0::GsmBroadcastSmsConfigInfo>& configs) override;
    Return<void> setGsmBroadcastConfigResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> setGsmBroadcastActivationResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getCdmaBroadcastConfigResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_0::CdmaBroadcastSmsConfigInfo>& configs) override;
    Return<void> setCdmaBroadcastConfigResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> setCdmaBroadcastActivationResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getCDMASubscriptionResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_string& mdn, const hidl_string& hSid, const hidl_string& hNid, const hidl_string& min, const hidl_string& prl) override;
    Return<void> writeSmsToRuimResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, uint32_t index) override;
    Return<void> deleteSmsOnRuimResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getDeviceIdentityResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_string& imei, const hidl_string& imeisv, const hidl_string& esn, const hidl_string& meid) override;
    Return<void> exitEmergencyCallbackModeResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getSmscAddressResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_string& smsc) override;
    Return<void> setSmscAddressResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> reportSmsMemoryStatusResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> reportStkServiceIsRunningResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getCdmaSubscriptionSourceResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, ::android::hardware::radio::V1_0::CdmaSubscriptionSource source) override;
    Return<void> requestIsimAuthenticationResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_string& response) override;
    Return<void> acknowledgeIncomingGsmSmsWithPduResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> sendEnvelopeWithStatusResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::IccIoResult& iccIo) override;
    Return<void> getVoiceRadioTechnologyResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, ::android::hardware::radio::V1_0::RadioTechnology rat) override;
    Return<void> getCellInfoListResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_0::CellInfo>& cellInfo) override;
    Return<void> setCellInfoListRateResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> setInitialAttachApnResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getImsRegistrationStateResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, bool isRegistered, ::android::hardware::radio::V1_0::RadioTechnologyFamily ratFamily) override;
    Return<void> sendImsSmsResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::SendSmsResult& sms) override;
    Return<void> iccTransmitApduBasicChannelResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::IccIoResult& result) override;
    Return<void> iccOpenLogicalChannelResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t channelId, const hidl_vec<int8_t>& selectResponse) override;
    Return<void> iccCloseLogicalChannelResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> iccTransmitApduLogicalChannelResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::IccIoResult& result) override;
    Return<void> nvReadItemResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_string& result) override;
    Return<void> nvWriteItemResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> nvWriteCdmaPrlResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> nvResetConfigResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> setUiccSubscriptionResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> setDataAllowedResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getHardwareConfigResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_0::HardwareConfig>& config) override;
    Return<void> requestIccSimAuthenticationResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::IccIoResult& result) override;
    Return<void> setDataProfileResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> requestShutdownResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getRadioCapabilityResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::RadioCapability& rc) override;
    Return<void> setRadioCapabilityResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::RadioCapability& rc) override;
    Return<void> startLceServiceResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::LceStatusInfo& statusInfo) override;
    Return<void> stopLceServiceResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::LceStatusInfo& statusInfo) override;
    Return<void> pullLceDataResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::LceDataInfo& lceInfo) override;
    Return<void> getModemActivityInfoResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_0::ActivityStatsInfo& activityInfo) override;
    Return<void> setAllowedCarriersResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, int32_t numAllowed) override;
    Return<void> getAllowedCarriersResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, bool allAllowed, const ::android::hardware::radio::V1_0::CarrierRestrictions& carriers) override;
    Return<void> sendDeviceStateResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> setIndicationFilterResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> setSimCardPowerResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> acknowledgeRequest(int32_t serial) override;

    // Methods from ::android::hardware::radio::V1_1::IRadioResponse follow.
    Return<void> setCarrierInfoForImsiEncryptionResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> setSimCardPowerResponse_1_1(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> startNetworkScanResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> stopNetworkScanResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> startKeepaliveResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_1::KeepaliveStatus& status) override;
    Return<void> stopKeepaliveResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;

    // Methods from ::android::hardware::radio::V1_2::IRadioResponse follow.
    Return<void> getCellInfoListResponse_1_2(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_2::CellInfo>& cellInfo) override;
    Return<void> getIccCardStatusResponse_1_2(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_2::CardStatus& cardStatus) override;
    Return<void> getIccCardStatusResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_4::CardStatus& cardStatus) override;
    Return<void> setSignalStrengthReportingCriteriaResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> setLinkCapacityReportingCriteriaResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getCurrentCallsResponse_1_2(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const hidl_vec<::android::hardware::radio::V1_2::Call>& calls) override;
    Return<void> getSignalStrengthResponse_1_2(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_2::SignalStrength& signalStrength) override;
    Return<void> getVoiceRegistrationStateResponse_1_2(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_2::VoiceRegStateResult& voiceRegResponse) override;
    Return<void> getDataRegistrationStateResponse_1_2(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_2::DataRegStateResult& dataRegResponse) override;

    Return<void> setSystemSelectionChannelsResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> enableModemResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getModemStackStatusResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, bool isEnabled) override;
    Return<void> emergencyDialResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> startNetworkScanResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info)  override;
    Return<void> getCellInfoListResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_4::CellInfo>& cellInfo) override;
    Return<void> getDataRegistrationStateResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_4::DataRegStateResult& dataRegResponse) override;
    Return<void> getPreferredNetworkTypeBitmapResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_4::RadioAccessFamily> networkTypeBitmap) override;
    Return<void> setPreferredNetworkTypeBitmapResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info)  override;
    Return<void> getDataCallListResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_4::SetupDataCallResult>& dcResponse) override;
    Return<void> setupDataCallResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_4::SetupDataCallResult& dcResponse) override;
    Return<void> setAllowedCarriersResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info) override;
    Return<void> getAllowedCarriersResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_4::CarrierRestrictionsWithPriority& carriers, ::android::hardware::radio::V1_4::SimLockMultiSimPolicy multiSimPolicy) override;
    Return<void> getSignalStrengthResponse_1_4(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::radio::V1_4::SignalStrength& signalStrength) override;

private:
    sp<SERadioResponseHandler> mHandler;
    hidl_string mSlotName;
};

struct SERadioIndication : public VENDOR_V1_0::ISERadioIndication {
    SERadioIndication(sp<SecureElement> se, hidl_string name);
    ~SERadioIndication();

private:
    sp<SecureElement> mSE;
    hidl_string mSlotName;
};

struct RadioIndication : public AOSP_V1_4::IRadioIndication {
    RadioIndication(sp<SecureElement> se, hidl_string name);
    ~RadioIndication();

    // Methods from ::android::hardware::radio::V1_0::IRadioIndication follow.
    Return<void> radioStateChanged(::android::hardware::radio::V1_0::RadioIndicationType type, ::android::hardware::radio::V1_0::RadioState radioState) override;
    Return<void> callStateChanged(::android::hardware::radio::V1_0::RadioIndicationType type) override;
    Return<void> networkStateChanged(::android::hardware::radio::V1_0::RadioIndicationType type) override;
    Return<void> newSms(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_vec<uint8_t>& pdu) override;
    Return<void> newSmsStatusReport(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_vec<uint8_t>& pdu) override;
    Return<void> newSmsOnSim(::android::hardware::radio::V1_0::RadioIndicationType type, int32_t recordNumber) override;
    Return<void> onUssd(::android::hardware::radio::V1_0::RadioIndicationType type, ::android::hardware::radio::V1_0::UssdModeType modeType, const hidl_string& msg) override;
    Return<void> nitzTimeReceived(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_string& nitzTime, uint64_t receivedTime) override;
    Return<void> currentSignalStrength(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_0::SignalStrength& signalStrength) override;
    Return<void> dataCallListChanged(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_vec<::android::hardware::radio::V1_0::SetupDataCallResult>& dcList) override;
    Return<void> suppSvcNotify(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_0::SuppSvcNotification& suppSvc) override;
    Return<void> stkSessionEnd(::android::hardware::radio::V1_0::RadioIndicationType type) override;
    Return<void> stkProactiveCommand(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_string& cmd) override;
    Return<void> stkEventNotify(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_string& cmd) override;
    Return<void> stkCallSetup(::android::hardware::radio::V1_0::RadioIndicationType type, int64_t timeout) override;
    Return<void> simSmsStorageFull(::android::hardware::radio::V1_0::RadioIndicationType type) override;
    Return<void> simRefresh(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_0::SimRefreshResult& refreshResult) override;
    Return<void> callRing(::android::hardware::radio::V1_0::RadioIndicationType type, bool isGsm, const ::android::hardware::radio::V1_0::CdmaSignalInfoRecord& record) override;
    Return<void> simStatusChanged(::android::hardware::radio::V1_0::RadioIndicationType type) override;
    Return<void> cdmaNewSms(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_0::CdmaSmsMessage& msg) override;
    Return<void> newBroadcastSms(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_vec<uint8_t>& data) override;
    Return<void> cdmaRuimSmsStorageFull(::android::hardware::radio::V1_0::RadioIndicationType type) override;
    Return<void> restrictedStateChanged(::android::hardware::radio::V1_0::RadioIndicationType type, ::android::hardware::radio::V1_0::PhoneRestrictedState state) override;
    Return<void> enterEmergencyCallbackMode(::android::hardware::radio::V1_0::RadioIndicationType type) override;
    Return<void> cdmaCallWaiting(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_0::CdmaCallWaiting& callWaitingRecord) override;
    Return<void> cdmaOtaProvisionStatus(::android::hardware::radio::V1_0::RadioIndicationType type, ::android::hardware::radio::V1_0::CdmaOtaProvisionStatus status) override;
    Return<void> cdmaInfoRec(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_0::CdmaInformationRecords& records) override;
    Return<void> indicateRingbackTone(::android::hardware::radio::V1_0::RadioIndicationType type, bool start) override;
    Return<void> resendIncallMute(::android::hardware::radio::V1_0::RadioIndicationType type) override;
    Return<void> cdmaSubscriptionSourceChanged(::android::hardware::radio::V1_0::RadioIndicationType type, ::android::hardware::radio::V1_0::CdmaSubscriptionSource cdmaSource) override;
    Return<void> cdmaPrlChanged(::android::hardware::radio::V1_0::RadioIndicationType type, int32_t version) override;
    Return<void> exitEmergencyCallbackMode(::android::hardware::radio::V1_0::RadioIndicationType type) override;
    Return<void> rilConnected(::android::hardware::radio::V1_0::RadioIndicationType type) override;
    Return<void> voiceRadioTechChanged(::android::hardware::radio::V1_0::RadioIndicationType type, ::android::hardware::radio::V1_0::RadioTechnology rat) override;
    Return<void> cellInfoList(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_vec<::android::hardware::radio::V1_0::CellInfo>& records) override;
    Return<void> imsNetworkStateChanged(::android::hardware::radio::V1_0::RadioIndicationType type) override;
    Return<void> subscriptionStatusChanged(::android::hardware::radio::V1_0::RadioIndicationType type, bool activate) override;
    Return<void> srvccStateNotify(::android::hardware::radio::V1_0::RadioIndicationType type, ::android::hardware::radio::V1_0::SrvccState state) override;
    Return<void> hardwareConfigChanged(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_vec<::android::hardware::radio::V1_0::HardwareConfig>& configs) override;
    Return<void> radioCapabilityIndication(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_0::RadioCapability& rc) override;
    Return<void> onSupplementaryServiceIndication(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_0::StkCcUnsolSsResult& ss) override;
    Return<void> stkCallControlAlphaNotify(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_string& alpha) override;
    Return<void> lceData(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_0::LceDataInfo& lce) override;
    Return<void> pcoData(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_0::PcoDataInfo& pco) override;
    Return<void> modemReset(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_string& reason) override;

    // Methods from ::android::hardware::radio::V1_1::IRadioIndication follow.
    Return<void> carrierInfoForImsiEncryption(::android::hardware::radio::V1_0::RadioIndicationType info) override;
    Return<void> networkScanResult(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_1::NetworkScanResult& result) override;
    Return<void> keepaliveStatus(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_1::KeepaliveStatus& status) override;

    // Methods from ::android::hardware::radio::V1_2::IRadioIndication follow.
    Return<void> networkScanResult_1_2(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_2::NetworkScanResult& result) override;
    Return<void> cellInfoList_1_2(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_vec<::android::hardware::radio::V1_2::CellInfo>& records) override;
    Return<void> currentLinkCapacityEstimate(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_2::LinkCapacityEstimate& lce) override;
    Return<void> currentPhysicalChannelConfigs(::android::hardware::radio::V1_0::RadioIndicationType type, const hidl_vec<::android::hardware::radio::V1_2::PhysicalChannelConfig>& configs) override;
    Return<void> currentSignalStrength_1_2(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_2::SignalStrength& signalStrength) override;

    Return<void> currentEmergencyNumberList(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_4::EmergencyNumber>& emergencyNumberList) override;
    Return<void> cellInfoList_1_4(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_4::CellInfo>& records) override;
    Return<void> networkScanResult_1_4(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_4::NetworkScanResult& result) override;
    Return<void> currentPhysicalChannelConfigs_1_4(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_4::PhysicalChannelConfig>& configs) override;
    Return<void> dataCallListChanged_1_4(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_4::SetupDataCallResult>& dcList) override;
    Return<void> currentSignalStrength_1_4(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::radio::V1_4::SignalStrength& signalStrength) override;

private:
    sp<SecureElement> mSE;
    hidl_string mSlotName;
};

struct RadioDeathRecipient : public hidl_death_recipient {
    RadioDeathRecipient(sp<SecureElement> se);
    ~RadioDeathRecipient();

    // Methods from ::android::hidl::hardware::hidl_death_recipient follow.
    void serviceDied(uint64_t /* cookie */, const ::android::wp<::android::hidl::base::V1_0::IBase>& /* who */) override;

private:
    sp<SecureElement> mSE;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace secure_element
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_SECURE_ELEMENT_V1_0_SECUREELEMENT_H
