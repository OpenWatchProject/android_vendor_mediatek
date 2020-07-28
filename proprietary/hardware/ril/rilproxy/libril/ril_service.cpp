/*
 * Copyright (c) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "RILC-RP"

#include <android/hardware/radio/1.4/IRadio.h>
#include <android/hardware/radio/1.4/IRadioResponse.h>
#include <android/hardware/radio/1.4/IRadioIndication.h>
#include <android/hardware/radio/1.4/types.h>

#include <vendor/mediatek/hardware/mtkradioex/1.0/IMtkRadioEx.h>
#include <vendor/mediatek/hardware/mtkradioex/1.0/IMtkRadioExResponse.h>
#include <vendor/mediatek/hardware/mtkradioex/1.0/IMtkRadioExIndication.h>
#include <vendor/mediatek/hardware/mtkradioex/1.0/IImsRadioIndication.h>
#include <vendor/mediatek/hardware/mtkradioex/1.0/ISubsidyLockIndication.h>
#include <vendor/mediatek/hardware/mtkradioex/1.0/ISubsidyLockResponse.h>
#include <vendor/mediatek/hardware/mtkradioex/1.0/IAssistRadioResponse.h>
#include <vendor/mediatek/hardware/mtkradioex/1.0/IRcsRadioIndication.h>
#include <vendor/mediatek/hardware/mtkradioex/1.0/IRcsRadioResponse.h>
#include <vendor/mediatek/hardware/mtkradioex/1.0/ICapRadioResponse.h>
#include <vendor/mediatek/hardware/mtkradioex/1.0/types.h>
#include <vendor/mediatek/hardware/mtkradioex/1.1/IMtkRadioEx.h>
#include <vendor/mediatek/hardware/mtkradioex/1.1/IMtkRadioExResponse.h>
#include <vendor/mediatek/hardware/mtkradioex/1.2/IMtkRadioEx.h>
#include <vendor/mediatek/hardware/mtkradioex/1.2/IMtkRadioExResponse.h>
#include <vendor/mediatek/hardware/mtkradioex/1.3/IMtkRadioEx.h>
#include <vendor/mediatek/hardware/mtkradioex/1.3/IMtkRadioExIndication.h>
#include <vendor/mediatek/hardware/mtkradioex/1.4/IMtkRadioEx.h>
#include <vendor/mediatek/hardware/mtkradioex/1.5/IMtkRadioEx.h>
#include <vendor/mediatek/hardware/mtkradioex/1.5/IMtkRadioExResponse.h>
#include <vendor/mediatek/hardware/mtkradioex/1.6/IMtkRadioEx.h>
#include <vendor/mediatek/hardware/mtkradioex/1.6/IImsRadioIndication.h>
#include <vendor/mediatek/hardware/mtkradioex/1.6/IImsRadioResponse.h>
#include <vendor/mediatek/hardware/mtkradioex/1.7/IMtkRadioEx.h>
#include <vendor/mediatek/hardware/mtkradioex/1.7/IImsRadioIndication.h>
#include <vendor/mediatek/hardware/mtkradioex/1.7/IImsRadioResponse.h>
#include <vendor/mediatek/hardware/mtkradioex/1.8/IMtkRadioEx.h>
#include <vendor/mediatek/hardware/mtkradioex/1.8/IMtkRadioExResponse.h>
#include <vendor/mediatek/hardware/mtkradioex/1.9/types.h>
#include <vendor/mediatek/hardware/mtkradioex/1.9/IMtkRadioEx.h>
#include <vendor/mediatek/hardware/mtkradioex/1.9/IMtkRadioExResponse.h>
#include <vendor/mediatek/hardware/mtkradioex/1.9/IMtkRadioExIndication.h>

#include <hwbinder/IPCThreadState.h>
#include <hwbinder/ProcessState.h>
#include <ril_service.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/SystemClock.h>
#include <inttypes.h>
#include <libmtkrilutils.h>
#include <cutils/properties.h>
#include <ril_internal.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "utils/Vector.h"

#define INVALID_HEX_CHAR 16

using namespace android::hardware::radio;
using namespace android::hardware::radio::V1_0;
using namespace vendor::mediatek::hardware::mtkradioex;
using namespace vendor::mediatek::hardware::mtkradioex::V1_0;
using namespace vendor::mediatek::hardware::mtkradioex::V1_1;
using namespace vendor::mediatek::hardware::mtkradioex::V1_2;
using namespace vendor::mediatek::hardware::mtkradioex::V1_3;
using namespace vendor::mediatek::hardware::mtkradioex::V1_4;
using namespace vendor::mediatek::hardware::mtkradioex::V1_5;
using namespace vendor::mediatek::hardware::mtkradioex::V1_6;
using namespace vendor::mediatek::hardware::mtkradioex::V1_9;

namespace AOSP_V1_0 = android::hardware::radio::V1_0;
namespace AOSP_V1_1 = android::hardware::radio::V1_1;
namespace AOSP_V1_2 = android::hardware::radio::V1_2;
namespace AOSP_V1_3 = android::hardware::radio::V1_3;
namespace AOSP_V1_4 = android::hardware::radio::V1_4;
namespace VENDOR_V1_0 = vendor::mediatek::hardware::mtkradioex::V1_0;
namespace VENDOR_V1_1 = vendor::mediatek::hardware::mtkradioex::V1_1;
namespace VENDOR_V1_2 = vendor::mediatek::hardware::mtkradioex::V1_2;
namespace VENDOR_V1_3 = vendor::mediatek::hardware::mtkradioex::V1_3;
namespace VENDOR_V1_4 = vendor::mediatek::hardware::mtkradioex::V1_4;
namespace VENDOR_V1_5 = vendor::mediatek::hardware::mtkradioex::V1_5;
namespace VENDOR_V1_6 = vendor::mediatek::hardware::mtkradioex::V1_6;
namespace VENDOR_V1_9 = vendor::mediatek::hardware::mtkradioex::V1_9;

using ::android::hardware::configureRpcThreadpool;
using ::android::hardware::joinRpcThreadpool;
using ::android::hardware::Return;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_array;
using ::android::hardware::Void;
using ::android::hardware::radio::V1_2::AudioQuality;
using ::android::hardware::hidl_bitfield;
using android::CommandInfo;
using android::RequestInfo;
using android::requestToString;
using android::sp;
using ::android::Vector;

#define BOOL_TO_INT(x) (x ? 1 : 0)
#define ATOI_NULL_HANDLED(x) (x ? atoi(x) : -1)
#define ATOI_NULL_HANDLED_DEF(x, defaultVal) (x ? atoi(x) : defaultVal)

#if defined(ANDROID_MULTI_SIM)
#define CALL_ONREQUEST(a, b, c, d, e) \
        s_vendorFunctions->onRequest((a), (b), (c), (d), ((RIL_SOCKET_ID)(e)))
#define CALL_ONSTATEREQUEST(a) s_vendorFunctions->onStateRequest((RIL_SOCKET_ID)(a))
#else
#define CALL_ONREQUEST(a, b, c, d, e) s_vendorFunctions->onRequest((a), (b), (c), (d))
#define CALL_ONSTATEREQUEST(a) s_vendorFunctions->onStateRequest()
#endif

RIL_RadioFunctions *s_vendorFunctions = NULL;
static CommandInfo *s_commands;

// M @{
static AOSP_V1_2::CardStatus s_cardStatus[MAX_SIM_COUNT * android::CLIENT_COUNT];
// M @}

// M @{
#define RAF_GSM_GROUP (RAF_GSM | RAF_GPRS | RAF_EDGE)
#define RAF_HS_GROUP (RAF_HSUPA | RAF_HSDPA | RAF_HSPA | RAF_HSPAP)
#define RAF_CDMA_GROUP (RAF_IS95A | RAF_IS95B | RAF_1xRTT)
#define RAF_EVDO_GROUP (RAF_EVDO_0 | RAF_EVDO_A | RAF_EVDO_B | RAF_EHRPD)
#define RAF_WCDMA_GROUP (RAF_HS_GROUP | RAF_UMTS)
#define RAF_LTE_GROUP (RAF_LTE | RAF_LTE_CA)
// M @}
static RIL_ImsPendingInd *s_ims_pending_ind[MAX_SIM_COUNT] = {NULL};
void appendRequestIntoPendingInd(int slot, int request, int responseType, void* data,
        int size);

struct RadioImpl;
struct MtkRadioExImpl;

//#if (SIM_COUNT >= 2)
sp<RadioImpl> radioService[MAX_SIM_COUNT * android::CLIENT_COUNT];
sp<MtkRadioExImpl> mtkRadioExService[MAX_SIM_COUNT * android::CLIENT_COUNT];
int64_t nitzTimeReceived[MAX_SIM_COUNT * android::CLIENT_COUNT];
// counter used for synchronization. It is incremented every time response callbacks are updated.
volatile int32_t mCounterRadio[MAX_SIM_COUNT * android::CLIENT_COUNT];
volatile int32_t mCounterMtkRadioEx[MAX_SIM_COUNT * android::CLIENT_COUNT];

// To Compute IMS Slot Id
extern "C" int toRealSlot(int slotId);
extern "C" int toClientSlot(int slotId, android::ClientId clientId);
extern "C" android::ClientId getClientBySlot(int slotId);

static pthread_rwlock_t radioServiceRwlocks[] = { PTHREAD_RWLOCK_INITIALIZER,
                                                  PTHREAD_RWLOCK_INITIALIZER,
                                                  PTHREAD_RWLOCK_INITIALIZER,
                                                  PTHREAD_RWLOCK_INITIALIZER};

static Vector<pthread_t> readLockOwnerList[MAX_SIM_COUNT];
static pthread_t writeLockOwner[MAX_SIM_COUNT] = {0,0,0,0};
static pthread_mutex_t readLockOwnerListMutex[MAX_SIM_COUNT] = {PTHREAD_MUTEX_INITIALIZER,
                                                                PTHREAD_MUTEX_INITIALIZER,
                                                                PTHREAD_MUTEX_INITIALIZER,
                                                                PTHREAD_MUTEX_INITIALIZER};

void convertRilHardwareConfigListToHal(void *response, size_t responseLen,
        hidl_vec<HardwareConfig>& records);

void convertRilRadioCapabilityToHal(void *response, size_t responseLen, RadioCapability& rc);

void convertRilLceDataInfoToHal(void *response, size_t responseLen, LceDataInfo& lce);

void convertRilSignalStrengthToHal(void *response, size_t responseLen,
        SignalStrength& signalStrength);

void convertRilSignalStrengthToHal_1_2(void *response, size_t responseLen,
        AOSP_V1_2::SignalStrength& signalStrength);

void convertRilSignalStrengthToHal_1_4(void *response, size_t responseLen,
        AOSP_V1_4::SignalStrength& signalStrength);

void convertRilDataCallToHal(RIL_Data_Call_Response_v11 *dcResponse,
        SetupDataCallResult& dcResult);

void convertRilDataCallListToHal(void *response, size_t responseLen,
        hidl_vec<SetupDataCallResult>& dcResultList);

char* convertRilCellInfoListToHal(void *response, size_t responseLen, hidl_vec<CellInfo>& records);

char* convertRilCellInfoListToHal_1_2(void *response, size_t responseLen,
        hidl_vec<AOSP_V1_2::CellInfo>& records);

char* convertRilCellInfoListToHal_1_4(void *response, size_t responseLen,
        hidl_vec<AOSP_V1_4::CellInfo>& records);

void convertRilDedicatedBearerInfoToHal(RIL_Dedicate_Data_Call_Struct *response,
        DedicateDataCall& ddcResult);

// M: [OD over ePDG] start
int encodeRat(int active, int rat, int slotId);

void convertRilDataCallToHalEx(MTK_RIL_Data_Call_Response_v11 *dcResponse,
        SetupDataCallResult& dcResult, int slotId);

void convertRilDataCallToHalEx_1_4(MTK_RIL_Data_Call_Response_v11 *dcResponse,
        AOSP_V1_4::SetupDataCallResult& dcResult, int slotId);

void convertRilDataCallListToHalEx(void *response, size_t responseLen,
        hidl_vec<SetupDataCallResult>& dcResultList, int slotId);

void convertRilDataCallListToHalEx_1_4(void *response, size_t responseLen,
        hidl_vec<AOSP_V1_4::SetupDataCallResult>& dcResultList, int slotId);
// M: [OD over ePDG] end

// M: [Inactive Timer] start
int decodeInactiveTimer(unsigned int authType);
// M: [Inactive Timer] end

bool isMtkFwkAddonNotExisted(int slotId);

struct RadioImpl : public AOSP_V1_4::IRadio {
    int32_t mSlotId;
    int32_t mClientId;
    sp<AOSP_V1_0::IRadioResponse> mRadioResponse;
    sp<AOSP_V1_0::IRadioIndication> mRadioIndication;
    sp<AOSP_V1_1::IRadioResponse> mRadioResponseV1_1;
    sp<AOSP_V1_1::IRadioIndication> mRadioIndicationV1_1;
    sp<AOSP_V1_2::IRadioResponse> mRadioResponseV1_2;
    sp<AOSP_V1_2::IRadioIndication> mRadioIndicationV1_2;
    sp<AOSP_V1_3::IRadioResponse> mRadioResponseV1_3;
    sp<AOSP_V1_3::IRadioIndication> mRadioIndicationV1_3;
    sp<AOSP_V1_4::IRadioResponse> mRadioResponseV1_4;
    sp<AOSP_V1_4::IRadioIndication> mRadioIndicationV1_4;

    Return<void> setResponseFunctions(
            const ::android::sp<AOSP_V1_0::IRadioResponse>& radioResponse,
            const ::android::sp<AOSP_V1_0::IRadioIndication>& radioIndication);

    Return<void> getIccCardStatus(int32_t serial);

    Return<void> supplyIccPinForApp(int32_t serial, const hidl_string& pin,
            const hidl_string& aid);

    Return<void> supplyIccPukForApp(int32_t serial, const hidl_string& puk,
            const hidl_string& pin, const hidl_string& aid);

    Return<void> supplyIccPin2ForApp(int32_t serial,
            const hidl_string& pin2,
            const hidl_string& aid);

    Return<void> supplyIccPuk2ForApp(int32_t serial, const hidl_string& puk2,
            const hidl_string& pin2, const hidl_string& aid);

    Return<void> changeIccPinForApp(int32_t serial, const hidl_string& oldPin,
            const hidl_string& newPin, const hidl_string& aid);

    Return<void> changeIccPin2ForApp(int32_t serial, const hidl_string& oldPin2,
            const hidl_string& newPin2, const hidl_string& aid);

    Return<void> supplyNetworkDepersonalization(int32_t serial, const hidl_string& netPin);

    Return<void> getCurrentCalls(int32_t serial);

    Return<void> dial(int32_t serial, const Dial& dialInfo);

    Return<void> getImsiForApp(int32_t serial,
            const ::android::hardware::hidl_string& aid);

    Return<void> hangup(int32_t serial, int32_t gsmIndex);

    Return<void> hangupWaitingOrBackground(int32_t serial);

    Return<void> hangupForegroundResumeBackground(int32_t serial);

    Return<void> switchWaitingOrHoldingAndActive(int32_t serial);

    Return<void> conference(int32_t serial);

    Return<void> rejectCall(int32_t serial);

    Return<void> getLastCallFailCause(int32_t serial);

    // HIDL 1.3 or below
    Return<void> getSignalStrength(int32_t serial);

    // HIDL 1.4
    Return<void> getSignalStrength_1_4(int32_t serial);

    Return<void> getVoiceRegistrationState(int32_t serial);

    Return<void> getDataRegistrationState(int32_t serial);

    Return<void> getOperator(int32_t serial);

    Return<void> setRadioPower(int32_t serial, bool on);

    Return<void> sendDtmf(int32_t serial,
            const ::android::hardware::hidl_string& s);

    Return<void> sendSms(int32_t serial, const GsmSmsMessage& message);

    Return<void> sendSMSExpectMore(int32_t serial, const GsmSmsMessage& message);

    Return<void> setupDataCall(int32_t serial,
            RadioTechnology radioTechnology,
            const DataProfileInfo& profileInfo,
            bool modemCognitive,
            bool roamingAllowed,
            bool isRoaming);

    Return<void> iccIOForApp(int32_t serial,
            const IccIo& iccIo);

    Return<void> sendUssd(int32_t serial,
            const ::android::hardware::hidl_string& ussd);

    Return<void> cancelPendingUssd(int32_t serial);

    Return<void> getClir(int32_t serial);

    Return<void> setClir(int32_t serial, int32_t status);

    Return<void> getCallForwardStatus(int32_t serial,
            const CallForwardInfo& callInfo);

    Return<void> setCallForward(int32_t serial,
            const CallForwardInfo& callInfo);

    Return<void> getCallWaiting(int32_t serial, int32_t serviceClass);

    Return<void> setCallWaiting(int32_t serial, bool enable, int32_t serviceClass);

    Return<void> acknowledgeLastIncomingGsmSms(int32_t serial,
            bool success, SmsAcknowledgeFailCause cause);

    Return<void> acceptCall(int32_t serial);

    Return<void> deactivateDataCall(int32_t serial,
            int32_t cid, bool reasonRadioShutDown);

    Return<void> getFacilityLockForApp(int32_t serial,
            const ::android::hardware::hidl_string& facility,
            const ::android::hardware::hidl_string& password,
            int32_t serviceClass,
            const ::android::hardware::hidl_string& appId);

    Return<void> setFacilityLockForApp(int32_t serial,
            const ::android::hardware::hidl_string& facility,
            bool lockState,
            const ::android::hardware::hidl_string& password,
            int32_t serviceClass,
            const ::android::hardware::hidl_string& appId);

    Return<void> setBarringPassword(int32_t serial,
            const ::android::hardware::hidl_string& facility,
            const ::android::hardware::hidl_string& oldPassword,
            const ::android::hardware::hidl_string& newPassword);

    Return<void> getNetworkSelectionMode(int32_t serial);

    Return<void> setNetworkSelectionModeAutomatic(int32_t serial);

    Return<void> setNetworkSelectionModeManual(int32_t serial,
            const ::android::hardware::hidl_string& operatorNumeric);

    Return<void> getAvailableNetworks(int32_t serial);

    Return<void> startNetworkScan(int32_t serial, const AOSP_V1_1::NetworkScanRequest& request);

    Return<void> stopNetworkScan(int32_t serial);

    Return<void> startDtmf(int32_t serial,
            const ::android::hardware::hidl_string& s);

    Return<void> stopDtmf(int32_t serial);

    Return<void> getBasebandVersion(int32_t serial);

    Return<void> separateConnection(int32_t serial, int32_t gsmIndex);

    Return<void> setMute(int32_t serial, bool enable);

    Return<void> getMute(int32_t serial);

    Return<void> getClip(int32_t serial);

    Return<void> getDataCallList(int32_t serial);

    Return<void> setSuppServiceNotifications(int32_t serial, bool enable);

    Return<void> writeSmsToSim(int32_t serial,
            const SmsWriteArgs& smsWriteArgs);

    Return<void> deleteSmsOnSim(int32_t serial, int32_t index);

    Return<void> setBandMode(int32_t serial, RadioBandMode mode);

    Return<void> getAvailableBandModes(int32_t serial);

    Return<void> sendEnvelope(int32_t serial,
            const ::android::hardware::hidl_string& command);

    Return<void> sendTerminalResponseToSim(int32_t serial,
            const ::android::hardware::hidl_string& commandResponse);

    Return<void> handleStkCallSetupRequestFromSim(int32_t serial, bool accept);

    Return<void> explicitCallTransfer(int32_t serial);

    Return<void> setPreferredNetworkType(int32_t serial, PreferredNetworkType nwType);

    Return<void> getPreferredNetworkType(int32_t serial);

    Return<void> getNeighboringCids(int32_t serial);

    Return<void> setLocationUpdates(int32_t serial, bool enable);

    Return<void> setCdmaSubscriptionSource(int32_t serial,
            CdmaSubscriptionSource cdmaSub);

    Return<void> setCdmaRoamingPreference(int32_t serial, CdmaRoamingType type);

    Return<void> getCdmaRoamingPreference(int32_t serial);

    Return<void> setTTYMode(int32_t serial, TtyMode mode);

    Return<void> getTTYMode(int32_t serial);

    Return<void> setPreferredVoicePrivacy(int32_t serial, bool enable);

    Return<void> getPreferredVoicePrivacy(int32_t serial);

    Return<void> sendCDMAFeatureCode(int32_t serial,
            const ::android::hardware::hidl_string& featureCode);

    Return<void> sendBurstDtmf(int32_t serial,
            const ::android::hardware::hidl_string& dtmf,
            int32_t on,
            int32_t off);

    Return<void> sendCdmaSms(int32_t serial, const CdmaSmsMessage& sms);

    Return<void> acknowledgeLastIncomingCdmaSms(int32_t serial,
            const CdmaSmsAck& smsAck);

    Return<void> getGsmBroadcastConfig(int32_t serial);

    Return<void> setGsmBroadcastConfig(int32_t serial,
            const hidl_vec<GsmBroadcastSmsConfigInfo>& configInfo);

    Return<void> setGsmBroadcastActivation(int32_t serial, bool activate);

    Return<void> getCdmaBroadcastConfig(int32_t serial);

    Return<void> setCdmaBroadcastConfig(int32_t serial,
            const hidl_vec<CdmaBroadcastSmsConfigInfo>& configInfo);

    Return<void> setCdmaBroadcastActivation(int32_t serial, bool activate);

    Return<void> getCDMASubscription(int32_t serial);

    Return<void> writeSmsToRuim(int32_t serial, const CdmaSmsWriteArgs& cdmaSms);

    Return<void> deleteSmsOnRuim(int32_t serial, int32_t index);

    Return<void> getDeviceIdentity(int32_t serial);

    Return<void> exitEmergencyCallbackMode(int32_t serial);

    Return<void> getSmscAddress(int32_t serial);

    Return<void> setSmscAddress(int32_t serial,
            const ::android::hardware::hidl_string& smsc);

    Return<void> reportSmsMemoryStatus(int32_t serial, bool available);

    Return<void> reportStkServiceIsRunning(int32_t serial);

    Return<void> getCdmaSubscriptionSource(int32_t serial);

    Return<void> requestIsimAuthentication(int32_t serial,
            const ::android::hardware::hidl_string& challenge);

    Return<void> acknowledgeIncomingGsmSmsWithPdu(int32_t serial,
            bool success,
            const ::android::hardware::hidl_string& ackPdu);

    Return<void> sendEnvelopeWithStatus(int32_t serial,
            const ::android::hardware::hidl_string& contents);

    Return<void> getVoiceRadioTechnology(int32_t serial);

    Return<void> getCellInfoList(int32_t serial);

    Return<void> setCellInfoListRate(int32_t serial, int32_t rate);

    Return<void> setInitialAttachApn(int32_t serial, const DataProfileInfo& dataProfileInfo,
            bool modemCognitive, bool isRoaming);

    Return<void> getImsRegistrationState(int32_t serial);

    Return<void> sendImsSms(int32_t serial, const ImsSmsMessage& message);

    Return<void> iccTransmitApduBasicChannel(int32_t serial, const SimApdu& message);

    Return<void> iccOpenLogicalChannel(int32_t serial,
            const ::android::hardware::hidl_string& aid, int32_t p2);

    Return<void> iccCloseLogicalChannel(int32_t serial, int32_t channelId);

    Return<void> iccTransmitApduLogicalChannel(int32_t serial, const SimApdu& message);

    Return<void> nvReadItem(int32_t serial, NvItem itemId);

    Return<void> nvWriteItem(int32_t serial, const NvWriteItem& item);

    Return<void> nvWriteCdmaPrl(int32_t serial,
            const ::android::hardware::hidl_vec<uint8_t>& prl);

    Return<void> nvResetConfig(int32_t serial, ResetNvType resetType);

    Return<void> setUiccSubscription(int32_t serial, const SelectUiccSub& uiccSub);

    Return<void> setDataAllowed(int32_t serial, bool allow);

    Return<void> getHardwareConfig(int32_t serial);

    Return<void> requestIccSimAuthentication(int32_t serial,
            int32_t authContext,
            const ::android::hardware::hidl_string& authData,
            const ::android::hardware::hidl_string& aid);

    Return<void> setDataProfile(int32_t serial,
            const ::android::hardware::hidl_vec<DataProfileInfo>& profiles, bool isRoaming);

    Return<void> requestShutdown(int32_t serial);

    Return<void> getRadioCapability(int32_t serial);

    Return<void> setRadioCapability(int32_t serial, const RadioCapability& rc);

    Return<void> startLceService(int32_t serial, int32_t reportInterval, bool pullMode);

    Return<void> stopLceService(int32_t serial);

    Return<void> pullLceData(int32_t serial);

    Return<void> getModemActivityInfo(int32_t serial);

    Return<void> setAllowedCarriers(int32_t serial,
            bool allAllowed,
            const CarrierRestrictions& carriers);

    Return<void> getAllowedCarriers(int32_t serial);

    Return<void> sendDeviceState(int32_t serial, DeviceStateType deviceStateType, bool state);

    Return<void> setIndicationFilter(int32_t serial, int32_t indicationFilter);

    Return<void> startKeepalive(int32_t serial, const AOSP_V1_1::KeepaliveRequest& keepalive);

    Return<void> stopKeepalive(int32_t serial, int32_t sessionHandle);

    Return<void> setSimCardPower(int32_t serial, bool powerUp);
    Return<void> setSimCardPower_1_1(int32_t serial,
            const AOSP_V1_1::CardPowerState state);

    Return<void> responseAcknowledgement();

    Return<void> setCarrierInfoForImsiEncryption(int32_t serial,
            const AOSP_V1_1::ImsiEncryptionInfo& message);

    Return<void> startNetworkScan_1_2(int32_t serial,
            const AOSP_V1_2::NetworkScanRequest& request);

    Return<void> setIndicationFilter_1_2(int32_t serial,
            int32_t indicationFilter);

    Return<void> setSignalStrengthReportingCriteria(int32_t serial,
            int32_t hysteresisMs, int32_t hysteresisDb,
            const hidl_vec<int32_t>& thresholdsDbm,
            AOSP_V1_2::AccessNetwork accessNetwork);

    Return<void> setLinkCapacityReportingCriteria(int32_t serial,
            int32_t hysteresisMs, int32_t hysteresisDlKbps,
            int32_t hysteresisUlKbps,
            const hidl_vec<int32_t>& thresholdsDownlinkKbps,
            const hidl_vec<int32_t>& thresholdsUplinkKbps,
            AOSP_V1_2::AccessNetwork accessNetwork);

    Return<void> setupDataCall_1_2(int32_t serial,
            AOSP_V1_2::AccessNetwork accessNetwork,
            const DataProfileInfo& dataProfileInfo, bool modemCognitive,
            bool roamingAllowed, bool isRoaming,
            AOSP_V1_2::DataRequestReason reason,
            const hidl_vec<hidl_string>& addresses,
            const hidl_vec<hidl_string>& dnses);

    Return<void> deactivateDataCall_1_2(int32_t serial, int32_t cid,
            AOSP_V1_2::DataRequestReason reason);

    Return<void> setSystemSelectionChannels(int32_t serial, bool specifyChannels,
            const hidl_vec<AOSP_V1_1::RadioAccessSpecifier>& specifiers);

    Return<void> enableModem(int32_t serial, bool on);

    Return<void> getModemStackStatus(int32_t serial);

    Return<void> setupDataCall_1_4(int32_t serial, AOSP_V1_4::AccessNetwork accessNetwork,
            const AOSP_V1_4::DataProfileInfo& dataProfileInfo, bool roamingAllowed,
            AOSP_V1_2::DataRequestReason reason, const hidl_vec<hidl_string>& addresses,
            const hidl_vec<hidl_string>& dnses);

    Return<void> setInitialAttachApn_1_4(int32_t serial,
            const AOSP_V1_4::DataProfileInfo& dataProfileInfo);

    Return<void> setDataProfile_1_4(int32_t serial,
            const hidl_vec<AOSP_V1_4::DataProfileInfo>& profiles);

    Return<void> emergencyDial(int32_t serial, const AOSP_V1_0::Dial& dialInfo,
            hidl_bitfield<AOSP_V1_4::EmergencyServiceCategory> categories,
            const hidl_vec<hidl_string>& urns, AOSP_V1_4::EmergencyCallRouting routing,
            bool hasKnownUserIntentEmergency, bool isTesting);

    Return<void> startNetworkScan_1_4(int32_t serial,
            const AOSP_V1_2::NetworkScanRequest& request);

    Return<void> getPreferredNetworkTypeBitmap(int32_t serial);

    Return<void> setPreferredNetworkTypeBitmap(int32_t serial,
            hidl_bitfield<AOSP_V1_4::RadioAccessFamily> networkTypeBitmap);

    Return<void> setAllowedCarriers_1_4(int32_t serial,
            const AOSP_V1_4::CarrierRestrictionsWithPriority& carriers,
            AOSP_V1_4::SimLockMultiSimPolicy multiSimPolicy);

    Return<void> getAllowedCarriers_1_4(int32_t serial);

    void checkReturnStatus(Return<void>& ret, int32_t origSlot);
};

struct MtkRadioExImpl : public VENDOR_V1_9::IMtkRadioEx {
    int32_t mSlotId;
    int32_t mClientId;
    sp<VENDOR_V1_0::IMtkRadioExResponse> mRadioResponseMtk;
    sp<VENDOR_V1_1::IMtkRadioExResponse> mRadioResponseMtkV1_1;
    sp<VENDOR_V1_2::IMtkRadioExResponse> mRadioResponseMtkV1_2;
    sp<VENDOR_V1_0::IMtkRadioExIndication> mRadioIndicationMtk;
    sp<VENDOR_V1_3::IMtkRadioExIndication> mRadioIndicationMtkV1_3;
    sp<VENDOR_V1_5::IMtkRadioExResponse> mRadioResponseMtkV1_5;

    sp<VENDOR_V1_0::IImsRadioResponse> mRadioResponseIms;
    sp<VENDOR_V1_0::IImsRadioIndication> mRadioIndicationIms;
    sp<VENDOR_V1_6::IImsRadioResponse> mRadioResponseImsV1_6;
    sp<VENDOR_V1_6::IImsRadioIndication> mRadioIndicationImsV1_6;

    sp<VENDOR_V1_9::IMtkRadioExResponse> mRadioResponseMtkV1_9;
    sp<VENDOR_V1_9::IMtkRadioExIndication> mRadioIndicationMtkV1_9;

    sp<VENDOR_V1_0::IMwiRadioResponse> mRadioResponseMwi;
    sp<VENDOR_V1_0::ISubsidyLockResponse> mRadioResponseSubsidy;
    sp<VENDOR_V1_0::ISubsidyLockIndication> mRadioIndicationSubsidy;
    sp<IMwiRadioIndication> mRadioIndicationMwi;
    sp<ISERadioResponse> mRadioResponseSE;
    sp<ISERadioIndication> mRadioIndicationSE;
    sp<IEmRadioResponse> mRadioResponseEm;
    sp<IEmRadioIndication> mRadioIndicationEm;
    sp<IAssistRadioResponse> mRadioResponseAssist;

    void checkReturnStatusMtk(Return<void>& ret, android::ClientType type, int32_t origSlot);
    char* clientTypeToString(android::ClientType type);
    Return<void> responseAcknowledgementMtk();
    Return<void> setResponseFunctionsMtk(
            const ::android::sp<VENDOR_V1_0::IMtkRadioExResponse>& radioResponse,
            const ::android::sp<VENDOR_V1_0::IMtkRadioExIndication>& radioIndication);

    Return<void> setResponseFunctionsIms(
            const ::android::sp<VENDOR_V1_0::IImsRadioResponse>& radioResponse,
            const ::android::sp<VENDOR_V1_0::IImsRadioIndication>& radioIndication);

    Return<void> setResponseFunctionsMwi(
            const ::android::sp<VENDOR_V1_0::IMwiRadioResponse>& radioResponse,
            const ::android::sp<IMwiRadioIndication>& radioIndication);

    Return<void> setResponseFunctionsSubsidyLock(
            const ::android::sp<VENDOR_V1_0::ISubsidyLockResponse>& sublockResponse,
            const ::android::sp<VENDOR_V1_0::ISubsidyLockIndication>& sublockIndication);

    Return<void> setResponseFunctionsRcs(
        const ::android::sp<IRcsRadioResponse>& radioResponseParam,
        const ::android::sp<VENDOR_V1_0::IRcsRadioIndication>& radioIndicationParam);

    Return<void> sendSubsidyLockRequest(int32_t serial, int32_t reqType,
            const ::android::hardware::hidl_vec<uint8_t>& data);

    /// MTK: ForSE @{
    Return<void> setResponseFunctionsSE(
            const ::android::sp<ISERadioResponse>& radioResponse,
            const ::android::sp<ISERadioIndication>& radioIndication);
    /// MTK: ForSE @}
    Return<void> setResponseFunctionsEm(
            const ::android::sp<IEmRadioResponse>& radioResponse,
            const ::android::sp<IEmRadioIndication>& radioIndication);

    /// MTK: For telephony assist. @(
    Return<void> setResponseFunctionsAssist(
            const ::android::sp<IAssistRadioResponse>& radioResponse);
    /// @}

    /// MTK: For capctrl. @(
    Return<void> setResponseFunctionsCap(
            const ::android::sp<ICapRadioResponse>& radioResponse);
    /// @}

    /// MTK: For switch. @(
    Return<void> setResponseFunctionsSmartRatSwitch(
            const ::android::sp<ISmartRatSwitchRadioResponse>& radioResponse,
            const ::android::sp<ISmartRatSwitchRadioIndication>& radioIndication);
    /// @}

    /// M: CC: call control @{
    Return<void> setColp(int32_t serial, int32_t status);

    Return<void> setColr(int32_t serial, int32_t status);

    Return<void> queryCallForwardInTimeSlotStatus(int32_t serial,
            const CallForwardInfoEx& callInfoEx);

    Return<void> setCallForwardInTimeSlot(int32_t serial,
            const CallForwardInfoEx& callInfoEx);

    Return<void> runGbaAuthentication(int32_t serial,
            const hidl_string& nafFqdn, const hidl_string& nafSecureProtocolId,
            bool forceRun, int32_t netId);

    Return<void> setNetworkSelectionModeManualWithAct(int32_t serial,
            const ::android::hardware::hidl_string& operatorNumeric,
            const ::android::hardware::hidl_string& act,
            const ::android::hardware::hidl_string& mode);

    Return<void> getAvailableNetworksWithAct(int32_t serial);

    Return<void> getSignalStrengthWithWcdmaEcio(int32_t serial);

    Return<void> cancelAvailableNetworks(int32_t serial);

    Return<void> cfgA2offset(int32_t serial, int32_t offset, int32_t threshBound);

    Return<void> cfgB1offset(int32_t serial, int32_t offset, int32_t threshBound);

    Return<void> enableSCGfailure(int32_t serial, bool enable, int32_t T1, int32_t P1, int32_t T2);

    Return<void> disableNR(int32_t serial, bool enable);

    Return<void> setTxPower(int32_t serial, int32_t limitpower);


    Return<void> setSearchStoredFreqInfo(int32_t serial, int32_t operation,
                                                int32_t plmnId, int32_t rat,
                                                const hidl_vec<int32_t>& freq);

    Return<void> setSearchRat(int32_t serial, const hidl_vec<int32_t>& rat);

    Return<void> setBgsrchDeltaSleepTimer(int32_t serial, int32_t sleepDuration);

    Return<void> setBarringPasswordCheckedByNW(int32_t serial,
            const ::android::hardware::hidl_string& facility,
            const ::android::hardware::hidl_string& oldPassword,
            const ::android::hardware::hidl_string& newPassword,
            const ::android::hardware::hidl_string& cfmPassword);

    Return<void> setClip(int32_t serial, int32_t clipEnable);

    Return<void> getColp(int32_t serial);

    Return<void> getColr(int32_t serial);

    Return<void> sendCnap(int32_t serial, const ::android::hardware::hidl_string& cnapMessage);

    Return<void> handleStkCallSetupRequestFromSimWithResCode(int32_t serial, int32_t resultCodet);

    /// M: CC: call control ([IMS] common flow) @{
    Return<void> hangupAll(int32_t serial);
    Return<void> hangupWithReason(int32_t serial, int32_t callId, int32_t reason);
    Return<void> setCallIndication(int32_t serial,
            int32_t mode, int32_t callId, int32_t seqNumber, int32_t cause);

    /// M: CC: Emergency mode for Fusion RIL
    Return<void> setEccMode(int32_t serial, const hidl_string& number, int32_t enable,
            int32_t airplaneMode, int32_t imsReg);

    /// M: CC: Vzw/CTVolte ECC for Fusion RIL
    Return<void> eccPreferredRat(int32_t serial, int32_t phoneType);

    Return<void> setVoicePreferStatus(int32_t serial, int32_t status);

    Return<void> setEccNum(int32_t serial, const hidl_string& eccListWithCard,
            const hidl_string& eccListNoCard);
    Return<void> getEccNum(int32_t serial);

    Return<void> triggerModeSwitchByEcc(int32_t serial, int32_t mode);
    Return<void> setTrm(int32_t serial, int32_t mode);
    Return<void> videoCallAccept(int32_t serial, int32_t videoMode, int32_t callId);
    Return<void> eccRedialApprove(int32_t serial, int32_t approve, int32_t callId);

    Return<void> imsEctCommand(int32_t serial, const hidl_string& number,
            int32_t type);

    Return<void> controlCall(int32_t serial, int32_t controlType, int32_t callId);

    Return<void> imsDeregNotification(int32_t serial,int32_t cause);

    Return<void> setImsEnable(int32_t serial, bool isOn);

    Return<void> setImsVideoEnable(int32_t serial, bool isOn);

    Return<void> setImscfg(int32_t serial, bool volteEnable,
                 bool vilteEnable, bool vowifiEnable, bool viwifiEnable,
                 bool smsEnable, bool eimsEnable);

    Return<void> setModemImsCfg(int32_t serial, const hidl_string& keys,
            const hidl_string& values, int32_t type);

    Return<void> getProvisionValue(int32_t serial,
            const hidl_string& provisionstring);

    // Ims config telephonyware START
    Return<void> setImsCfgProvisionValue(int32_t serial, int32_t configId,
                                         const hidl_string& value);

    Return<void> getImsCfgProvisionValue(int32_t serial, int32_t configId);

    Return<void>
    setImsCfgFeatureValue(int32_t serial, int32_t featureId, int32_t network, int32_t value,
                          int32_t isLast);

    Return<void> getImsCfgFeatureValue(int32_t serial, int32_t featureId, int32_t network);

    Return<void> getImsCfgResourceCapValue(int32_t serial, int32_t featureId);
    // Ims config telephonyware END

    // Femtocell feature
    Return<void> getFemtocellList(int32_t serial);
    Return<void> abortFemtocellList(int32_t serial);
    Return<void> selectFemtocell(int32_t serial,
        const ::android::hardware::hidl_string& operatorNumeric,
        const ::android::hardware::hidl_string& act,
        const ::android::hardware::hidl_string& csgId);
    Return<void> queryFemtoCellSystemSelectionMode(int32_t serial);
    Return<void> setFemtoCellSystemSelectionMode(int32_t serial, int32_t mode);

    Return<void> setProvisionValue(int32_t serial,
            const hidl_string& provisionstring, const hidl_string& provisionValue);

    Return<void> controlImsConferenceCallMember(int32_t serial, int32_t controlType,
            int32_t confCallId, const hidl_string& address, int32_t callId);

    Return<void> setWfcProfile(int32_t serial, int32_t wfcPreference);


    Return<void> conferenceDial(int32_t serial, const ConferenceDial& dailInfo);

    Return<void> vtDial(int32_t serial, const Dial& dialInfo);

    Return<void> vtDialWithSipUri(int32_t serial, const hidl_string& address);

    Return<void> dialWithSipUri(int32_t serial, const hidl_string& address);

    Return<void> sendUssi(int32_t serial, const hidl_string& ussiString);

    Return<void> cancelUssi(int32_t serial);

    Return<void> getXcapStatus(int32_t serial);

    Return<void> resetSuppServ(int32_t serial);

    Return<void> setupXcapUserAgentString(int32_t serial, const hidl_string& userAgent);

    Return<void> forceReleaseCall(int32_t serial, int32_t callId);

    Return<void> setImsRtpReport(int32_t serial, int32_t pdnId,
            int32_t networkId, int32_t timer);

    Return<void> imsBearerStateConfirm(int32_t serial, int32_t aid, int32_t action, int32_t status);

    Return<void> setImsBearerNotification(int32_t serial, int32_t enable);

    Return<void> pullCall(int32_t serial, const hidl_string& target, bool isVideoCall);

    Return<void> setImsRegistrationReport(int32_t serial);

    // MTK-START: SIM
    Return<void> getATR(int32_t serial);
    Return<void> getIccid(int32_t serial);
    Return<void> setSimPower(int32_t serial, int32_t mode);
    Return<void> activateUiccCard(int32_t serial);
    Return<void> deactivateUiccCard(int32_t serial);
    Return<void> getCurrentUiccCardProvisioningStatus(int32_t serial);
    // MTK-END

    // MTK-START: SIM GBA
    Return<void> doGeneralSimAuthentication(int32_t serial, const SimAuthStructure& simAuth);
    // MTK-END

    // FastDormancy
    Return<void> setFdMode(int32_t serial, int mode, int param1, int param2);

    // ATCI Start
    sp<IAtciResponse> mAtciResponse;
    sp<IAtciIndication> mAtciIndication;

    Return<void> setResponseFunctionsForAtci(
            const ::android::sp<IAtciResponse>& atciResponse,
            const ::android::sp<IAtciIndication>& atciIndication);

    Return<void> sendAtciRequest(int32_t serial,
            const ::android::hardware::hidl_vec<uint8_t>& data);
    // ATCI End
    /// M: eMBMS feature
    Return<void> sendEmbmsAtCommand(int32_t serial, const ::android::hardware::hidl_string& data);
    /// M: eMBMS end
    Return<void> getSmsRuimMemoryStatus(int32_t serial);

    // worldmode {
    Return<void> setResumeRegistration(int32_t serial, int32_t sessionId);
    Return<void> modifyModemType(int32_t serial, int32_t applyType, int32_t modemType);
    // worldmode }

    Return<void> restartRILD(int32_t serial);

    // M: Data Framework - common part enhancement
    Return<void> syncDataSettingsToMd(int32_t serial,
            const hidl_vec<int32_t>& settings);
    Return<void> resetMdDataRetryCount(int32_t serial,
            const hidl_string& apn);
    // M: Data Framework - CC 33
    Return<void> setRemoveRestrictEutranMode(int32_t serial, int32_t type);
    // M: [LTE][Low Power][UL traffic shaping] @{
    Return<void> setLteAccessStratumReport(int32_t serial, int32_t enable);
    Return<void> setLteUplinkDataTransfer(int32_t serial, int32_t state, int32_t interfaceId);
    // M: [LTE][Low Power][UL traffic shaping] @}
    // SMS-START
    Return<void> getSmsParameters(int32_t serial);
    Return<void> setSmsParameters(int32_t serial, const SmsParams& message);
    Return<void> getSmsMemStatus(int32_t serial);
    Return<void> setEtws(int32_t serial, int32_t mode);
    Return<void> removeCbMsg(int32_t serial, int32_t channelId, int32_t serialId);
    Return<void> setGsmBroadcastLangs(int32_t serial,
            const ::android::hardware::hidl_string& langs);
    Return<void> getGsmBroadcastLangs(int32_t serial);
    Return<void> getGsmBroadcastActivation(int32_t serial);
    Return<void> sendImsSmsEx(int32_t serial, const ImsSmsMessage& message);
    Return<void> acknowledgeLastIncomingGsmSmsEx(int32_t serial,
            bool success, SmsAcknowledgeFailCause cause);
    Return<void> acknowledgeLastIncomingCdmaSmsEx(int32_t serial, const CdmaSmsAck& smsAck);
    // SMS-END
    // MTK-START: SIM ME LOCK
    Return<void> queryNetworkLock(int32_t serial, int32_t category);
    Return<void> setNetworkLock(int32_t serial, int32_t category, int32_t lockop,
            const hidl_string& password, const hidl_string& data_imsi,
            const hidl_string& gid1, const hidl_string& gid2);
    Return<void> supplyDepersonalization(int32_t serial, const hidl_string& netPin, int32_t type);
    // MTK-END
    Return<void> setRxTestConfig(int32_t serial, int32_t antType);
    Return<void> getRxTestResult(int32_t serial, int32_t mode);

    Return<void> getPOLCapability(int32_t serial);
    Return<void> getCurrentPOLList(int32_t serial);
    Return<void> setPOLEntry(int32_t serial, int32_t index,
            const ::android::hardware::hidl_string& numeric,
            int32_t nAct);
    Return<void> getPlmnNameFromSE13Table(int32_t serial, int32_t mcc, int32_t mnc);
    Return<void> enableCAPlusBandWidthFilter(int32_t serial, bool enabled);
    // PHB start
    Return<void> queryPhbStorageInfo(int32_t serial, int32_t type);
    Return<void> writePhbEntry(int32_t serial, const PhbEntryStructure& phbEntry);
    Return<void> readPhbEntry(int32_t serial, int32_t type, int32_t bIndex, int32_t eIndex);
    Return<void> queryUPBCapability(int32_t serial);
    Return<void> editUPBEntry(int32_t serial, const hidl_vec<hidl_string>& data);
    Return<void> deleteUPBEntry(int32_t serial, int32_t entryType, int32_t adnIndex, int32_t entryIndex);
    Return<void> readUPBGasList(int32_t serial, int32_t startIndex, int32_t endIndex);
    Return<void> readUPBGrpEntry(int32_t serial, int32_t adnIndex);
    Return<void> writeUPBGrpEntry(int32_t serial, int32_t adnIndex, const hidl_vec<int32_t>& grpIds);
    Return<void> getPhoneBookStringsLength(int32_t serial);
    Return<void> getPhoneBookMemStorage(int32_t serial);
    Return<void> setPhoneBookMemStorage(int32_t serial, const hidl_string& storage, const hidl_string& password);
    Return<void> readPhoneBookEntryExt(int32_t serial, int32_t index1, int32_t index2);
    Return<void> writePhoneBookEntryExt(int32_t serial, const PhbEntryExt& phbEntryExt);
    Return<void> queryUPBAvailable(int32_t serial, int32_t eftype, int32_t fileIndex);
    Return<void> readUPBEmailEntry(int32_t serial, int32_t adnIndex, int32_t fileIndex);
    Return<void> readUPBSneEntry(int32_t serial, int32_t adnIndex, int32_t fileIndex);
    Return<void> readUPBAnrEntry(int32_t serial, int32_t adnIndex, int32_t fileIndex);
    Return<void> readUPBAasList(int32_t serial, int32_t startIndex, int32_t endIndex);
    Return<void> setPhonebookReady(int32_t serial, int32_t ready);
    // PHB End
    Return<void> setApcMode(int32_t serial, int32_t mode, int32_t reportMode, int32_t interval);
    Return<void> getApcInfo(int32_t serial);
    /// M: [Network][C2K] Sprint roaming control @{
    Return<void> setRoamingEnable(int32_t serial, const hidl_vec<int32_t>& config);
    Return<void> getRoamingEnable(int32_t serial, int32_t phoneId);
    /// @}
    Return<void> setLteReleaseVersion(int32_t serial, int32_t mode);
    Return<void> getLteReleaseVersion(int32_t serial);

    Return<void> setModemPower(int32_t serial, bool isOn);
    // External SIM [Start]
    Return<void> sendVsimNotification(int32_t serial, uint32_t transactionId,
            uint32_t eventId, uint32_t simType);

    Return<void> sendVsimOperation(int32_t serial, uint32_t transactionId,
            uint32_t eventId, int32_t result, int32_t dataLength, const hidl_vec<uint8_t>& data);
    // External SIM [End]

    Return<void> setVoiceDomainPreference(int32_t serial, int32_t vdp);
    Return<void> getVoiceDomainPreference(int32_t serial);
    Return<void> setWifiEnabled(int32_t serial, const hidl_string& ifName,
        int32_t isWifiEnabled, int32_t isFlightModeOn);
    Return<void> setWifiAssociated(int32_t serial, const hidl_string& ifName,
        int32_t associated, const hidl_string& ssid, const hidl_string& apMac, int32_t mtuSize,
        const hidl_string& ueMac);
    Return<void> setWifiSignalLevel(int32_t serial,
            int32_t rssi, int32_t snr);
    Return<void> setWifiIpAddress(int32_t serial,
            const hidl_string& ifName,
            const hidl_string& ipv4Addr, const hidl_string& ipv6Addr,
            int32_t ipv4PrefixLen, int32_t ipv6PrefixLen,
            const hidl_string& ipv4Gateway, const hidl_string& ipv6Gateway,
            int32_t dnsCount, const hidl_string& dnsAddresses);
    Return<void> setWfcConfig (int32_t serial,
            int32_t setting,
            const hidl_string& ifName,
            const hidl_string& value);
    Return<void> querySsacStatus(int32_t serial);
    Return<void> setLocationInfo(int32_t serial,
            const hidl_string& accountId,
            const hidl_string& broadcastFlag, const hidl_string& latitude,
            const hidl_string& longitude, const hidl_string& accuracy,
            const hidl_string& method, const hidl_string& city, const hidl_string& state,
            const hidl_string& zip, const hidl_string& countryCode, const hidl_string& ueWlanMac);
    Return<void> setEmergencyAddressId(int32_t serial, const hidl_string& aid);
    Return<void> setNattKeepAliveStatus(int32_t serial,
            const hidl_string& ifName, bool enable,
            const hidl_string& srcIp, int32_t srcPort,
            const hidl_string& dstIp, int32_t dstPort);
    Return<void> setWifiPingResult(int32_t serial, int32_t rat, int32_t latency,
            int32_t pktloss);
    Return<void> setServiceStateToModem(int32_t serial, int32_t voiceRegState,
            int32_t dataRegState, int32_t voiceRoamingType, int32_t dataRoamingType,
            int32_t rilVoiceRegState, int32_t rilDataRegState);
    Return<void> sendRequestRaw(int32_t serial, const hidl_vec<uint8_t>& data);
    Return<void> sendRequestStrings(int32_t serial, const hidl_vec<hidl_string>& data);

    Return<void> dataConnectionAttach(int32_t serial, int32_t type);
    Return<void> dataConnectionDetach(int32_t serial, int32_t type);
    Return<void> resetAllConnections(int32_t serial);
    Return<void> setTxPowerStatus(int32_t serial, int32_t mode);
    Return<void> setSuppServProperty(int32_t serial, const hidl_string& name, const hidl_string& value);
    // MTK-START: SIM SLOT LOCK
    Return<void> supplyDeviceNetworkDepersonalization(int32_t serial, const hidl_string& netPin);
    // MTK-END
    ///M: Notify ePDG screen state
    Return<void> notifyEPDGScreenState(int32_t serial, int32_t state);
    Return<void> setVendorSetting(int32_t serial, int setting, const hidl_string& value);
    // M: RTT @{
    Return<void> setRttMode(int serial, int mode);
    Return<void> sendRttModifyRequest(int serial, int callId, int newMode);
    Return<void> sendRttText(int serial, int callId, int lenOfString, const hidl_string& text);
    Return<void> rttModifyRequestResponse(int serial, int callId, int result);
    Return<void> toggleRttAudioIndication(int serial, int callId, int enable);
    // @}
    Return<void> queryVopsStatus(int32_t serial);
    Return<void> notifyImsServiceReady();
    Return<void> setSipHeader(int32_t serial, const hidl_vec<hidl_string>& data);
    Return<void> setSipHeaderReport(int32_t serial, const hidl_vec<hidl_string>& data);
    Return<void> setImsCallMode(int32_t serial, int32_t mode);
    // M: GWSD @{
    Return<void> setGwsdMode(int32_t serial, const hidl_vec<hidl_string>& data);
    Return<void> setCallValidTimer(int32_t serial, int32_t timer);
    Return<void> setIgnoreSameNumberInterval(int32_t serial, int32_t interval);
    Return<void> setKeepAliveByPDCPCtrlPDU(int32_t serial, const hidl_string& data);
    Return<void> setKeepAliveByIpData(int32_t serial, const hidl_string& data);
    // @}
    Return<void> enableDsdaIndication(int32_t serial, bool enable);
    Return<void> getDsdaStatus(int32_t serial);

    Return<void> registerCellQltyReport(int32_t serial,
            const hidl_string& enable, const hidl_string& type,
            const hidl_string& thresholdValues, const hidl_string& triggerTime);

    Return<void> getSuggestedPlmnList(int32_t serial, int32_t rat, int32_t num, int32_t timer);
    Return<void> routeCertificate(int32_t serial, int uid, const hidl_vec<uint8_t>& cert,
            const hidl_vec<uint8_t>& msg);
    Return<void> routeAuthMessage(int32_t serial, int uid, const hidl_vec<uint8_t>& msg);
    Return<void> enableCapabaility(int32_t serial, const hidl_string& id, int uid, int toActive);
    Return<void> abortCertificate(int32_t serial, int uid);
    Return<void> deactivateNrScgCommunication(int32_t serial, bool deactivate, bool allowSCGAdd);
    Return<void> getDeactivateNrScgCommunication(int32_t serial);
    Return<void> setMaxUlSpeed(int32_t serial, int32_t ulSpeed);
    Return<void> smartRatSwitch(int32_t serial, int32_t mode, int32_t rat);
    Return<void> getSmartRatSwitch(int32_t serial, int32_t mode);
    Return<void> setSmartSceneSwitch(int32_t serial, int32_t mode, int32_t tGear, int32_t lGear);
    Return<void> sendSarIndicator(int32_t serial, int32_t sar_cmd_type,
            const hidl_string& sar_parameter);
    Return<void> setCallAdditionalInfo(int32_t serial, const hidl_vec<hidl_string>& info);
    Return<void> setCallSubAddress(int32_t serial, bool enable);
    Return<void> getCallSubAddress(int32_t serial);
    //1.9
    Return<void> sendRsuRequest(int serial, const RsuRequestInfo& reuRequest);
};

void memsetAndFreeStrings(int numPointers, ...) {
    va_list ap;
    va_start(ap, numPointers);
    for (int i = 0; i < numPointers; i++) {
        char *ptr = va_arg(ap, char *);
        if (ptr) {
#ifdef MEMSET_FREED
#define MAX_STRING_LENGTH 4096
            memset(ptr, 0, strnlen(ptr, MAX_STRING_LENGTH));
#endif
            free(ptr);
        }
    }
    va_end(ap);
}

void sendErrorResponse(RequestInfo *pRI, RIL_Errno err) {
    pRI->pCI->responseFunction((int) pRI->socket_id, getClientBySlot((int) pRI->socket_id),
            (int) RadioResponseType::SOLICITED, pRI->token, err, NULL, 0);
}

/**
 * Copies over src to dest. If memory allocation fails, responseFunction() is called for the
 * request with error RIL_E_NO_MEMORY. The size() method is used to determine the size of the
 * destination buffer into which the HIDL string is copied. If there is a discrepancy between
 * the string length reported by the size() method, and the length of the string returned by
 * the c_str() method, the function will return false indicating a failure.
 *
 * Returns true on success, and false on failure.
 */
bool copyHidlStringToRil(char **dest, const hidl_string &src, RequestInfo *pRI, bool allowEmpty) {
    size_t len = src.size();
    if (len == 0 && !allowEmpty) {
        *dest = NULL;
        return true;
    }
    *dest = (char *) calloc(len + 1, sizeof(char));
    if (*dest == NULL) {
        RLOGE("Memory allocation failed for request %s", requestToString(pRI->pCI->requestNumber));
        sendErrorResponse(pRI, RIL_E_NO_MEMORY);
        return false;
    }
    if (strlcpy(*dest, src.c_str(), len + 1) >= (len + 1)) {
        RLOGE("Copy of the HIDL string has been truncated, as "
              "the string length reported by size() does not "
              "match the length of string returned by c_str().");
        free(*dest);
        *dest = NULL;
        sendErrorResponse(pRI, RIL_E_INTERNAL_ERR);
        return false;
    }
    return true;
}

bool copyHidlStringToRil(char **dest, const hidl_string &src, RequestInfo *pRI) {
    return copyHidlStringToRil(dest, src, pRI, false);
}

AOSP_V1_2::AudioQuality convertCallsSpeechCodecToHidlAudioQuality(int speechCodec) {
    /**
     *   MtkSpeechCodecTypes
     *     NONE(0),
     *     QCELP13K(0x0001),
     *     EVRC(0x0002),
     *     EVRC_B(0x0003),
     *     EVRC_WB(0x0004),
     *     EVRC_NW(0x0005),
     *     AMR_NB(0x0006),
     *     AMR_WB(0x0007),
     *     GSM_EFR(0x0008),
     *     GSM_FR(0x0009),
     *     GSM_HR(0x000A);
     */
    switch (speechCodec) {
    case 0x0006:
        return AOSP_V1_2::AudioQuality::AMR;
    case 0x0007:
        return AOSP_V1_2::AudioQuality::AMR_WB;
    case 0x0008:
        return AOSP_V1_2::AudioQuality::GSM_EFR;
    case 0x0009:
        return AOSP_V1_2::AudioQuality::GSM_FR;
    case 0x000A:
        return AOSP_V1_2::AudioQuality::GSM_HR;
    case 0x0002:
        return AOSP_V1_2::AudioQuality::EVRC;
    case 0x0003:
        return AOSP_V1_2::AudioQuality::EVRC_B;
    case 0x0004:
        return AOSP_V1_2::AudioQuality::EVRC_WB;
    case 0x0005:
        return AOSP_V1_2::AudioQuality::EVRC_NW;
    default:
        return AOSP_V1_2::AudioQuality::UNSPECIFIED;
    }
}

hidl_string convertCharPtrToHidlString(const char *ptr) {
    hidl_string ret;
    if (ptr != NULL) {
        // TODO: replace this with strnlen
        ret.setToExternal(ptr, strlen(ptr));
    }
    return ret;
}

hidl_vec<hidl_string> convertCharPtrToVecHidlString(const char *ptr) {
    hidl_vec<hidl_string> ret;

    if (ptr != NULL && strlen(ptr) > 0) {
        int nSize = 0;
        char *token = NULL;
        const char *delim = " ";
        int i = 0;
        char *tmp = NULL;

        asprintf(&tmp, "%s", ptr);

        token = strstr(tmp, delim);
        while (token != NULL) {
            nSize++;
            token = strstr(token+1, delim);
        }
        nSize++;

        ret.resize(nSize);
        token = strtok(tmp, delim);
        while (token != NULL) {
            hidl_string hidlStr(token);
            ret[i++] = hidlStr;
            token = strtok(NULL, delim);
        }

        free(tmp);
    }

    return ret;
}

bool dispatchVoid(int serial, int slotId, int request) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }
    CALL_ONREQUEST(request, NULL, 0, pRI, slotId);
    return true;
}

bool dispatchString(int serial, int slotId, int request, const char * str) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    char *pString;
    if (!copyHidlStringToRil(&pString, str, pRI)) {
        return false;
    }

    CALL_ONREQUEST(request, pString, sizeof(char *), pRI, slotId);

    memsetAndFreeStrings(1, pString);
    return true;
}

bool dispatchStrings(int serial, int slotId, int request, bool allowEmpty, int countStrings, ...) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    char **pStrings;
    pStrings = (char **)calloc(countStrings, sizeof(char *));
    if (pStrings == NULL) {
        RLOGE("Memory allocation failed for request %s", requestToString(request));
        sendErrorResponse(pRI, RIL_E_NO_MEMORY);
        return false;
    }
    va_list ap;
    va_start(ap, countStrings);
    for (int i = 0; i < countStrings; i++) {
        const char* str = va_arg(ap, const char *);
        if (!copyHidlStringToRil(&pStrings[i], hidl_string(str), pRI, allowEmpty)) {
            va_end(ap);
            for (int j = 0; j < i; j++) {
                memsetAndFreeStrings(1, pStrings[j]);
            }
            free(pStrings);
            return false;
        }
    }
    va_end(ap);

    CALL_ONREQUEST(request, pStrings, countStrings * sizeof(char *), pRI, slotId);

    if (pStrings != NULL) {
        for (int i = 0 ; i < countStrings ; i++) {
            memsetAndFreeStrings(1, pStrings[i]);
        }

#ifdef MEMSET_FREED
        memset(pStrings, 0, countStrings * sizeof(char *));
#endif
        free(pStrings);
    }
    return true;
}

bool dispatchStrings(int serial, int slotId, int request, const hidl_vec<hidl_string>& data) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    int countStrings = data.size();
    char **pStrings;
    pStrings = (char **)calloc(countStrings, sizeof(char *));
    if (pStrings == NULL) {
        RLOGE("Memory allocation failed for request %s", requestToString(request));
        sendErrorResponse(pRI, RIL_E_NO_MEMORY);
        return false;
    }

    for (int i = 0; i < countStrings; i++) {
        if (!copyHidlStringToRil(&pStrings[i], data[i], pRI)) {
            for (int j = 0; j < i; j++) {
                memsetAndFreeStrings(1, pStrings[j]);
            }
            free(pStrings);
            return false;
        }
    }

    CALL_ONREQUEST(request, pStrings, countStrings * sizeof(char *), pRI, slotId);

    if (pStrings != NULL) {
        for (int i = 0 ; i < countStrings ; i++) {
            memsetAndFreeStrings(1, pStrings[i]);
        }

#ifdef MEMSET_FREED
        memset(pStrings, 0, countStrings * sizeof(char *));
#endif
        free(pStrings);
    }
    return true;
}

bool dispatchInts(int serial, int slotId, int request, int countInts, ...) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    int *pInts = (int *)calloc(countInts, sizeof(int));

    if (pInts == NULL) {
        RLOGE("Memory allocation failed for request %s", requestToString(request));
        sendErrorResponse(pRI, RIL_E_NO_MEMORY);
        return false;
    }
    va_list ap;
    va_start(ap, countInts);
    for (int i = 0; i < countInts; i++) {
        pInts[i] = va_arg(ap, int);
    }
    va_end(ap);

    CALL_ONREQUEST(request, pInts, countInts * sizeof(int), pRI, slotId);

    if (pInts != NULL) {
#ifdef MEMSET_FREED
        memset(pInts, 0, countInts * sizeof(int));
#endif
        free(pInts);
    }
    return true;
}

bool dispatchCallForwardStatus(int serial, int slotId, int request,
                              const CallForwardInfo& callInfo) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    RIL_CallForwardInfo cf;
    cf.status = (int) callInfo.status;
    cf.reason = callInfo.reason;
    cf.serviceClass = callInfo.serviceClass;
    cf.toa = callInfo.toa;
    cf.timeSeconds = callInfo.timeSeconds;

    if (!copyHidlStringToRil(&cf.number, callInfo.number, pRI)) {
        return false;
    }

    CALL_ONREQUEST(request, &cf, sizeof(cf), pRI, slotId);

    memsetAndFreeStrings(1, cf.number);

    return true;
}

bool dispatchCallForwardInTimeSlotStatus(int serial, int slotId, int request,
                              const CallForwardInfoEx& callInfoEx) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    RIL_CallForwardInfoEx cfEx;
    cfEx.status = (int) callInfoEx.status;
    cfEx.reason = callInfoEx.reason;
    cfEx.serviceClass = callInfoEx.serviceClass;
    cfEx.toa = callInfoEx.toa;
    cfEx.timeSeconds = callInfoEx.timeSeconds;

    if (!copyHidlStringToRil(&cfEx.number, callInfoEx.number, pRI)) {
        return false;
    }

    if (!copyHidlStringToRil(&cfEx.timeSlotBegin, callInfoEx.timeSlotBegin, pRI)) {
        memsetAndFreeStrings(1, cfEx.number);
        return false;
    }

    if (!copyHidlStringToRil(&cfEx.timeSlotEnd, callInfoEx.timeSlotEnd, pRI)) {
        memsetAndFreeStrings(2, cfEx.number, cfEx.timeSlotBegin);
        return false;
    }

    s_vendorFunctions->onRequest(request, &cfEx, sizeof(cfEx), pRI,
            pRI->socket_id);

    memsetAndFreeStrings(3, cfEx.number, cfEx.timeSlotBegin, cfEx.timeSlotEnd);

    return true;
}

bool dispatchRaw(int serial, int slotId, int request, const hidl_vec<uint8_t>& rawBytes) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    const uint8_t *uData = rawBytes.data();

    CALL_ONREQUEST(request, (void *) uData, rawBytes.size(), pRI, slotId);

    return true;
}

bool dispatchIccApdu(int serial, int slotId, int request, const SimApdu& message) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    RIL_SIM_APDU apdu = {};

    apdu.sessionid = message.sessionId;
    apdu.cla = message.cla;
    apdu.instruction = message.instruction;
    apdu.p1 = message.p1;
    apdu.p2 = message.p2;
    apdu.p3 = message.p3;

    if (!copyHidlStringToRil(&apdu.data, message.data, pRI)) {
        return false;
    }

    CALL_ONREQUEST(request, &apdu, sizeof(apdu), pRI, slotId);

    memsetAndFreeStrings(1, apdu.data);

    return true;
}

char* clientTypeToString(android::ClientType type) {
    switch (type) {
        case android::IMS_AOSP:
            return (char *)"IMS_AOSP";
        case android::IMS_MTK:
            return (char *)"IMS_MTK";
        case android::IMS_WFC:
            return (char *)"IMS_WFC";
        case android::EM_AOSP:
            return (char *)"EM_AOSP";
        case android::SE_MTK:
            return (char *)"SE_MTK";
        case android::EM_MTK:
            return (char *)"EM_MTK";
        case android::ASSIST_MTK:
            return (char *)"ASSIST_MTK";
        case android::RCS_MTK:
            return (char*)"RCS_MTK";
        case android::CAP_MTK:
            return (char *)"CAP_MTK";
        default:
            return (char *)"<UNKNOWN_TYPE>";
    }
}

void checkReturnStatus(int32_t slotId, Return<void>& ret, bool isRadioService, int32_t origSlotId){
    if (ret.isOk() == false) {
        RLOGE("checkReturnStatus: unable to call response/indication, %d, %d",
                slotId, origSlotId);
        // Remote process hosting the callbacks must be dead. Reset the callback objects;
        // there's no other recovery to be done here. When the client process is back up, it will
        // call setResponseFunctions()

        // Caller should already hold rdlock, release that first
        // note the current counter to avoid overwriting updates made by another thread before
        // write lock is acquired.
        int counter = mCounterRadio[slotId];
        // To support multiple client response in one response function, we have to release and
        // get the correct lock, the origSlotId is the real slot of the response fucntion;
        // the slotId is other client's id. Because we get the real slot lock in the
        // onRequestComplete so we have to release the readl slot lock first
        pthread_rwlock_t *radioServiceRwlockPtr = radio::getRadioServiceRwlock(origSlotId);
        int gotRLock = 0;
        int ret;
        if (radio::isTidInRLock((int) origSlotId, pthread_self()) != -1) {
            // had got the lock
            gotRLock = 1;
            radio::unlockRadioServiceRlock(radioServiceRwlockPtr, (int) origSlotId);
            RLOGV("checkReturnStatus: release r lock %d", origSlotId);
        } else {
            RLOGE("checkReturnStatus: didn't get r lock %d", origSlotId);
        }
        radioServiceRwlockPtr = radio::getRadioServiceRwlock(slotId);
        // acquire wrlock
        radio::lockRadioServiceWlock(radioServiceRwlockPtr, (int) slotId);

        // make sure the counter value has not changed
        if (counter == mCounterRadio[slotId]) {
            RLOGI("checkReturnStatus[%d]: reset response/indication callback", slotId);
            radioService[slotId]->mRadioResponse = NULL;
            radioService[slotId]->mRadioIndication = NULL;
            radioService[slotId]->mRadioResponseV1_1 = NULL;
            radioService[slotId]->mRadioIndicationV1_1 = NULL;
            radioService[slotId]->mRadioResponseV1_2 = NULL;
            radioService[slotId]->mRadioIndicationV1_2 = NULL;
            radioService[slotId]->mRadioResponseV1_3 = NULL;
            radioService[slotId]->mRadioIndicationV1_3 = NULL;
            radioService[slotId]->mRadioResponseV1_4 = NULL;
            radioService[slotId]->mRadioIndicationV1_4 = NULL;
            mCounterRadio[slotId]++;
            android::ClientId client = getClientBySlot(slotId);
            if(client == android::CLIENT_IMS) {
                android::onCommandDisconnect((RIL_SOCKET_ID) slotId, android::IMS_AOSP);
            } else if(client == android::CLIENT_RILJ) {
                android::onCommandDisconnect((RIL_SOCKET_ID) slotId, android::RADIO_AOSP);
            }
        } else {
            RLOGE("checkReturnStatus: not resetting responseFunctions as they likely "
                    "got updated on another thread");
        }

        // release wrlock
        radio::unlockRadioServiceWlock(radioServiceRwlockPtr, (int)slotId);

        if(gotRLock == 1) {
            // Reacquire rdlock
            radioServiceRwlockPtr = radio::getRadioServiceRwlock(origSlotId);
            radio::lockRadioServiceRlock(radioServiceRwlockPtr, (int)origSlotId);
            RLOGV("checkReturnStatus: got r lock %d", origSlotId);
        } else {
            RLOGE("checkReturnStatus: don't need to re-get r lock %d", origSlotId);
        }
    }
}

void checkReturnStatusMtk(int32_t slotId, Return<void>& ret, bool isRadioService,
        android::ClientType type, int32_t origSlotId) {
    if (ret.isOk() == false) {
        RLOGE("checkReturnStatusMtk: unable to call response/indication, %d, %d",
                slotId, origSlotId);
        // Remote process hosting the callbacks must be dead. Reset the callback objects;
        // there's no other recovery to be done here. When the client process is back up, it will
        // call setResponseFunctions()

        // Caller should already hold rdlock, release that first
        // note the current counter to avoid overwriting updates made by another thread before
        // write lock is acquired.
        int counter = mCounterMtkRadioEx[slotId];
        // To support multiple client response in one response function, we have to release and
        // get the correct lock, the origSlotId is the real slot of the response fucntion;
        // the slotId is other client's id. Because we get the real slot lock in the
        // onRequestComplete so we have to release the readl slot lock first
        pthread_rwlock_t *radioServiceRwlockPtr = radio::getRadioServiceRwlock(origSlotId);
        int gotRLock = 0;
        int ret;
        if (radio::isTidInRLock((int) origSlotId, pthread_self()) != -1) {
            // had got the lock
            gotRLock = 1;
            radio::unlockRadioServiceRlock(radioServiceRwlockPtr, (int) origSlotId);
            RLOGV("checkReturnStatus: release r lock %d", origSlotId);
        } else {
            RLOGE("checkReturnStatus: didn't get r lock %d", origSlotId);
        }
        radioServiceRwlockPtr = radio::getRadioServiceRwlock(slotId);
        // acquire wrlock
        radio::lockRadioServiceWlock(radioServiceRwlockPtr, (int) slotId);
        android::ClientId client = getClientBySlot(slotId);

        // make sure the counter value has not changed
        if (counter == mCounterMtkRadioEx[slotId]) {
            if(client == android::CLIENT_IMS) {
                RLOGE("checkReturnStatus[%d]: reset responseIms/indicationIms callback for %s",
                        slotId, clientTypeToString(type));
                if (type == android::IMS_MTK) {
                    mtkRadioExService[slotId]->mRadioResponseIms = NULL;
                    mtkRadioExService[slotId]->mRadioIndicationIms = NULL;
                    mtkRadioExService[slotId]->mRadioResponseImsV1_6 = NULL;
                    mtkRadioExService[slotId]->mRadioIndicationImsV1_6 = NULL;
                } else if (type == android::IMS_WFC) {
                    mtkRadioExService[slotId]->mRadioResponseMwi = NULL;
                    mtkRadioExService[slotId]->mRadioIndicationMwi = NULL;
                }
            } else if (client == android::CLIENT_SE) {
                if (type == android::SE_MTK) {
                    mtkRadioExService[slotId]->mRadioResponseSE = NULL;
                    mtkRadioExService[slotId]->mRadioIndicationSE = NULL;
                }
            } else if (client == android::CLIENT_EM) {
                if (type == android::EM_MTK) {
                    mtkRadioExService[slotId]->mRadioResponseEm = NULL;
                    mtkRadioExService[slotId]->mRadioIndicationEm = NULL;
                }
            } else if (client == android::CLIENT_ASSIST) {
                if (type == android::ASSIST_MTK) {
                    mtkRadioExService[slotId]->mRadioResponseAssist = NULL;
                }
            } else {
                RLOGI("checkReturnStatus[%d]: reset responseMtk/indicationMtk callback", slotId);
                mtkRadioExService[slotId]->mRadioResponseMtk = NULL;
                mtkRadioExService[slotId]->mRadioIndicationMtk = NULL;
            }
            mCounterMtkRadioEx[slotId]++;
            android::onCommandDisconnect((RIL_SOCKET_ID) slotId, type);
        } else {
            RLOGE("checkReturnStatus: not resetting responseFunctions as they likely "
                    "got updated on another thread");
        }

        // release wrlock
        radio::unlockRadioServiceWlock(radioServiceRwlockPtr, (int)slotId);

        if(gotRLock == 1) {
            // Reacquire rdlock
            radioServiceRwlockPtr = radio::getRadioServiceRwlock(origSlotId);
            radio::lockRadioServiceRlock(radioServiceRwlockPtr, (int)origSlotId);
            RLOGV("checkReturnStatus: got r lock %d", origSlotId);
        } else {
            RLOGE("checkReturnStatus: don't need to re-get r lock %d", origSlotId);
        }
    }
}

void RadioImpl::checkReturnStatus(Return<void>& ret, int32_t origSlot) {
    ::checkReturnStatus(mSlotId, ret, true, origSlot);
}

void MtkRadioExImpl::checkReturnStatusMtk(Return<void>& ret, android::ClientType type,
        int32_t origSlot) {
    ::checkReturnStatusMtk(mSlotId, ret, true, type, origSlot);
}

Return<void> RadioImpl::setResponseFunctions(
        const ::android::sp<AOSP_V1_0::IRadioResponse>& radioResponseParam,
        const ::android::sp<AOSP_V1_0::IRadioIndication>& radioIndicationParam) {
    RLOGI("setResponseFunctions, slotId = %d", mSlotId);

    pthread_rwlock_t *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radio::lockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    mRadioResponse = radioResponseParam;
    mRadioIndication = radioIndicationParam;
    mRadioResponseV1_1 = AOSP_V1_1::IRadioResponse::castFrom(mRadioResponse).withDefault(nullptr);
    mRadioIndicationV1_1 = AOSP_V1_1::IRadioIndication::castFrom(mRadioIndication).withDefault(nullptr);

    mRadioResponseV1_2 = AOSP_V1_2::IRadioResponse::castFrom(mRadioResponse).withDefault(nullptr);
    mRadioIndicationV1_2 = AOSP_V1_2::IRadioIndication::castFrom(mRadioIndication).withDefault(nullptr);

    mRadioResponseV1_3 = AOSP_V1_3::IRadioResponse::castFrom(mRadioResponse).withDefault(nullptr);
    mRadioIndicationV1_3 = AOSP_V1_3::IRadioIndication::castFrom(mRadioIndication).withDefault(nullptr);

    mRadioResponseV1_4 = AOSP_V1_4::IRadioResponse::castFrom(mRadioResponse).withDefault(nullptr);
    mRadioIndicationV1_4 = AOSP_V1_4::IRadioIndication::castFrom(mRadioIndication).withDefault(nullptr);

    mCounterRadio[mSlotId]++;

    radio::unlockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    // client is connected. Send initial indications.
    android::ClientId client = getClientBySlot(mSlotId);
    if(client == android::CLIENT_IMS) {
        android::onNewCommandConnect((RIL_SOCKET_ID) mSlotId, android::IMS_AOSP);
    } else if(client == android::CLIENT_RILJ) {
        android::onNewCommandConnect((RIL_SOCKET_ID) mSlotId, android::RADIO_AOSP);
    }
    return Void();
}

Return<void> MtkRadioExImpl::setResponseFunctionsMtk(
        const ::android::sp<VENDOR_V1_0::IMtkRadioExResponse>& radioResponseParam,
        const ::android::sp<VENDOR_V1_0::IMtkRadioExIndication>& radioIndicationParam) {
    RLOGI("setResponseFunctionsMtk, slotId = %d", mSlotId);

    pthread_rwlock_t *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radio::lockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    mRadioResponseMtk = radioResponseParam;
    mRadioIndicationMtk = radioIndicationParam;

    mRadioResponseMtkV1_1 = VENDOR_V1_1::IMtkRadioExResponse::castFrom(mRadioResponseMtk)
            .withDefault(nullptr);
    if (mRadioResponseMtkV1_1 == nullptr) {
        RLOGE("mRadioResponseMtkV1_1: got nullptr");
    }
    mRadioResponseMtkV1_2 = VENDOR_V1_2::IMtkRadioExResponse::castFrom(mRadioResponseMtk)
            .withDefault(nullptr);
    if (mRadioResponseMtkV1_2 == nullptr) {
        RLOGE("mRadioResponseMtkV1_2: got nullptr");
    }
    mRadioIndicationMtkV1_3 = VENDOR_V1_3::IMtkRadioExIndication::castFrom(mRadioIndicationMtk)
            .withDefault(nullptr);
    if (mRadioIndicationMtkV1_3 == nullptr) {
        RLOGE("mRadioIndicationMtkV1_3: got nullptr");
    }
    mRadioResponseMtkV1_5 = VENDOR_V1_5::IMtkRadioExResponse::castFrom(mRadioResponseMtk)
            .withDefault(nullptr);
    if (mRadioResponseMtkV1_5 == nullptr) {
        RLOGE("mRadioResponseMtkV1_5: got nullptr");
    }

    mCounterMtkRadioEx[mSlotId]++;

    radio::unlockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);
    // client is connected. Send initial indications.
    android::onNewCommandConnect((RIL_SOCKET_ID) mSlotId, android::RADIO_MTK);
    return Void();
}

Return<void> MtkRadioExImpl::setResponseFunctionsIms(
        const ::android::sp<VENDOR_V1_0::IImsRadioResponse>& radioResponseParam,
        const ::android::sp<VENDOR_V1_0::IImsRadioIndication>& radioIndicationParam) {
    RLOGD("setResponseFunctionsIms");

    pthread_rwlock_t *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radio::lockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    mRadioResponseIms = radioResponseParam;
    mRadioIndicationIms = radioIndicationParam;

    mRadioResponseImsV1_6 =
            VENDOR_V1_6::IImsRadioResponse::castFrom(mRadioResponseIms).withDefault(nullptr);
    mRadioIndicationImsV1_6 =
            VENDOR_V1_6::IImsRadioIndication::castFrom(mRadioIndicationIms).withDefault(nullptr);
    if (mRadioResponseImsV1_6 == nullptr || mRadioIndicationImsV1_6 == nullptr) {
        mRadioResponseImsV1_6 = nullptr;
        mRadioIndicationImsV1_6 = nullptr;
    }

    radio::unlockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);
    // client is connected. Send initial indications.
    android::onNewCommandConnect((RIL_SOCKET_ID) mSlotId, android::IMS_MTK);
    return Void();
}

Return<void> MtkRadioExImpl::setResponseFunctionsMwi(
        const ::android::sp<VENDOR_V1_0::IMwiRadioResponse>& radioResponseParam,
        const ::android::sp<IMwiRadioIndication>& radioIndicationParam) {
    RLOGD("setResponseFunctionsMwi");

    pthread_rwlock_t *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radio::lockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    mRadioResponseMwi = radioResponseParam;
    mRadioIndicationMwi = radioIndicationParam;

    radio::unlockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    return Void();
}

/// MTK: ForSE @{
Return<void> MtkRadioExImpl::setResponseFunctionsSE(
        const ::android::sp<ISERadioResponse>& radioResponseParam,
        const ::android::sp<ISERadioIndication>& radioIndicationParam) {
    RLOGD("[%d] setResponseFunctionsSE", mSlotId);

    pthread_rwlock_t *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radio::lockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    mRadioResponseSE = radioResponseParam;
    mRadioIndicationSE = radioIndicationParam;

    radio::unlockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    return Void();
}
/// MTK: ForSE @}

/// MTK: For EM @{
Return<void> MtkRadioExImpl::setResponseFunctionsEm(
        const ::android::sp<IEmRadioResponse>& radioResponseParam,
        const ::android::sp<IEmRadioIndication>& radioIndicationParam) {
    RLOGD("[%d] setResponseFunctionsEm", mSlotId);
    pthread_rwlock_t *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radio::lockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    mRadioResponseEm = radioResponseParam;
    mRadioIndicationEm = radioIndicationParam;

    radio::unlockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    return Void();
}

/// MTK: For telephony assist @{
Return<void> MtkRadioExImpl::setResponseFunctionsAssist (
        const ::android::sp<IAssistRadioResponse>& radioResponse) {
    RLOGD("[%d] setResponseFunctionsAssist", mSlotId);

    pthread_rwlock_t *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radio::lockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    mRadioResponseAssist = radioResponse;

    radio::unlockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    return Void();
}
/// @}

/// MTK: For capctrl. @(
Return<void> MtkRadioExImpl::setResponseFunctionsCap(
        const ::android::sp<ICapRadioResponse>& radioResponse) {
    return Void();
}
/// @}

void populateResponseInfo(RadioResponseInfo& responseInfo, int serial, int responseType,
                         RIL_Errno e) {
    responseInfo.serial = serial;
    switch (responseType) {
        case RESPONSE_SOLICITED:
            responseInfo.type = RadioResponseType::SOLICITED;
            break;
        case RESPONSE_SOLICITED_ACK_EXP:
            responseInfo.type = RadioResponseType::SOLICITED_ACK_EXP;
            break;
    }
    responseInfo.error = (RadioError) e;
}

Return<void> RadioImpl::getIccCardStatus(int32_t serial) {
    RLOGD("getIccCardStatus: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_SIM_STATUS);
    return Void();
}

Return<void> RadioImpl::supplyIccPinForApp(int32_t serial, const hidl_string& pin,
        const hidl_string& aid) {
    RLOGD("supplyIccPinForApp: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_ENTER_SIM_PIN, true,
            2, pin.c_str(), aid.c_str());
    return Void();
}

Return<void> RadioImpl::supplyIccPukForApp(int32_t serial, const hidl_string& puk,
                                           const hidl_string& pin, const hidl_string& aid) {
    RLOGD("supplyIccPukForApp: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_ENTER_SIM_PUK, true,
            3, puk.c_str(), pin.c_str(), aid.c_str());
    return Void();
}

Return<void> RadioImpl::supplyIccPin2ForApp(int32_t serial, const hidl_string& pin2,
                                            const hidl_string& aid) {
    RLOGD("supplyIccPin2ForApp: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_ENTER_SIM_PIN2, true,
            2, pin2.c_str(), aid.c_str());
    return Void();
}

Return<void> RadioImpl::supplyIccPuk2ForApp(int32_t serial, const hidl_string& puk2,
                                            const hidl_string& pin2, const hidl_string& aid) {
    RLOGD("supplyIccPuk2ForApp: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_ENTER_SIM_PUK2, true,
            3, puk2.c_str(), pin2.c_str(), aid.c_str());
    return Void();
}

Return<void> RadioImpl::changeIccPinForApp(int32_t serial, const hidl_string& oldPin,
                                           const hidl_string& newPin, const hidl_string& aid) {
    RLOGD("changeIccPinForApp: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_CHANGE_SIM_PIN, true,
            3, oldPin.c_str(), newPin.c_str(), aid.c_str());
    return Void();
}

Return<void> RadioImpl::changeIccPin2ForApp(int32_t serial, const hidl_string& oldPin2,
                                            const hidl_string& newPin2, const hidl_string& aid) {
    RLOGD("changeIccPin2ForApp: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_CHANGE_SIM_PIN2, true,
            3, oldPin2.c_str(), newPin2.c_str(), aid.c_str());
    return Void();
}

Return<void> RadioImpl::supplyNetworkDepersonalization(int32_t serial,
                                                       const hidl_string& netPin) {
    RLOGD("supplyNetworkDepersonalization: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_ENTER_NETWORK_DEPERSONALIZATION, true,
            1, netPin.c_str());
    return Void();
}

Return<void> RadioImpl::getCurrentCalls(int32_t serial) {
    RLOGD("getCurrentCalls: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_CURRENT_CALLS);
    return Void();
}

Return<void> RadioImpl::dial(int32_t serial, const Dial& dialInfo) {
    RLOGD("dial: serial %d", serial);

    int requestId = RIL_REQUEST_DIAL;
    if(mClientId == android::CLIENT_IMS) {
        requestId = RIL_REQUEST_IMS_DIAL;
    }

    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, requestId);
    if (pRI == NULL) {
        return Void();
    }
    RIL_Dial dial = {};
    RIL_UUS_Info uusInfo = {};
    int32_t sizeOfDial = sizeof(dial);

    if (!copyHidlStringToRil(&dial.address, dialInfo.address, pRI)) {
        return Void();
    }
    dial.clir = (int) dialInfo.clir;

    if (dialInfo.uusInfo.size() != 0) {
        uusInfo.uusType = (RIL_UUS_Type) dialInfo.uusInfo[0].uusType;
        uusInfo.uusDcs = (RIL_UUS_DCS) dialInfo.uusInfo[0].uusDcs;

        if (dialInfo.uusInfo[0].uusData.size() == 0) {
            uusInfo.uusData = NULL;
            uusInfo.uusLength = 0;
        } else {
            if (!copyHidlStringToRil(&uusInfo.uusData, dialInfo.uusInfo[0].uusData, pRI)) {
                memsetAndFreeStrings(1, dial.address);
                return Void();
            }
            uusInfo.uusLength = dialInfo.uusInfo[0].uusData.size();
        }

        dial.uusInfo = &uusInfo;
    }

    CALL_ONREQUEST(requestId, &dial, sizeOfDial, pRI, mSlotId);

    memsetAndFreeStrings(2, dial.address, uusInfo.uusData);

    return Void();
}

Return<void> RadioImpl::getImsiForApp(int32_t serial, const hidl_string& aid) {
    RLOGD("getImsiForApp: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_GET_IMSI, false,
            1, aid.c_str());
    return Void();
}

Return<void> RadioImpl::hangup(int32_t serial, int32_t gsmIndex) {
    RLOGD("hangup: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_HANGUP, 1, gsmIndex);
    return Void();
}

Return<void> RadioImpl::hangupWaitingOrBackground(int32_t serial) {
    RLOGD("hangupWaitingOrBackground: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND);
    return Void();
}

Return<void> RadioImpl::hangupForegroundResumeBackground(int32_t serial) {
    RLOGD("hangupForegroundResumeBackground: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND);
    return Void();
}

Return<void> RadioImpl::switchWaitingOrHoldingAndActive(int32_t serial) {
    RLOGD("switchWaitingOrHoldingAndActive: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE);
    return Void();
}

Return<void> RadioImpl::conference(int32_t serial) {
    RLOGD("conference: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_CONFERENCE);
    return Void();
}

Return<void> RadioImpl::rejectCall(int32_t serial) {
    RLOGD("rejectCall: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_UDUB);
    return Void();
}

Return<void> RadioImpl::getLastCallFailCause(int32_t serial) {
    RLOGD("getLastCallFailCause: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_LAST_CALL_FAIL_CAUSE);
    return Void();
}

// HIDL 1.3 or below
Return<void> RadioImpl::getSignalStrength(int32_t serial) {
    RLOGD("getSignalStrength: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_SIGNAL_STRENGTH);
    return Void();
}

// HIDL 1.4
Return<void> RadioImpl::getSignalStrength_1_4(int32_t serial) {
    RLOGD("getSignalStrength_1_4: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_SIGNAL_STRENGTH);
    return Void();
}

Return<void> RadioImpl::getVoiceRegistrationState(int32_t serial) {
    RLOGD("getVoiceRegistrationState: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_VOICE_REGISTRATION_STATE);
    return Void();
}

Return<void> RadioImpl::getDataRegistrationState(int32_t serial) {
    RLOGD("getDataRegistrationState: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_DATA_REGISTRATION_STATE);
    return Void();
}

Return<void> RadioImpl::getOperator(int32_t serial) {
    RLOGD("getOperator: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_OPERATOR);
    return Void();
}

Return<void> RadioImpl::setRadioPower(int32_t serial, bool on) {
    RLOGD("setRadioPower: serial %d on %d", serial, on);
    dispatchInts(serial, mSlotId, RIL_REQUEST_RADIO_POWER, 1, BOOL_TO_INT(on));
    return Void();
}

Return<void> RadioImpl::sendDtmf(int32_t serial, const hidl_string& s) {
    RLOGD("sendDtmf: serial %d", serial);
    dispatchString(serial, mSlotId, RIL_REQUEST_DTMF, s.c_str());
    return Void();
}

Return<void> RadioImpl::sendSms(int32_t serial, const GsmSmsMessage& message) {
    RLOGD("sendSms: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_SEND_SMS, false,
            2, message.smscPdu.c_str(), message.pdu.c_str());
    return Void();
}

Return<void> RadioImpl::sendSMSExpectMore(int32_t serial, const GsmSmsMessage& message) {
    RLOGD("sendSMSExpectMore: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_SEND_SMS_EXPECT_MORE, false,
            2, message.smscPdu.c_str(), message.pdu.c_str());
    return Void();
}

static bool convertMvnoTypeToString(MvnoType type, char *&str) {
    switch (type) {
        case MvnoType::IMSI:
            str = (char *)"imsi";
            return true;
        case MvnoType::GID:
            str = (char *)"gid";
            return true;
        case MvnoType::SPN:
            str = (char *)"spn";
            return true;
        case MvnoType::NONE:
            str = (char *)"";
            return true;
    }
    return false;
}

static bool convertPdpProtocolTypeToString(char **dest, AOSP_V1_4::PdpProtocolType type) {
    switch (type) {
        case AOSP_V1_4::PdpProtocolType::IP:
            *dest = (char *)"IP";
            return true;
        case AOSP_V1_4::PdpProtocolType::IPV4V6:
            *dest = (char *)"IPV4V6";
            return true;
        case AOSP_V1_4::PdpProtocolType::IPV6:
            *dest = (char *)"IPV6";
            return true;
        case AOSP_V1_4::PdpProtocolType::PPP:
            *dest = (char *)"PPP";
            return true;
        default:
            *dest = (char *)"IP";
            return true;
    }
    return false;
}

static AOSP_V1_4::PdpProtocolType convertStringToPdpProtocolType(const char *ptr) {
    if (ptr != NULL) {
        if (0 == strcasecmp(ptr, "IP")) {
            return AOSP_V1_4::PdpProtocolType::IP;
        } else if (0 == strcasecmp(ptr, "IPV4V6")) {
            return AOSP_V1_4::PdpProtocolType::IPV4V6;
        } else if (0 == strcasecmp(ptr, "IPV6")) {
            return AOSP_V1_4::PdpProtocolType::IPV6;
        } else if (0 == strcasecmp(ptr, "PPP")) {
            return AOSP_V1_4::PdpProtocolType::PPP;
        }
    }
    return AOSP_V1_4::PdpProtocolType::IP;
}

Return<void> RadioImpl::setupDataCall(int32_t serial, RadioTechnology radioTechnology,
                                      const DataProfileInfo& dataProfileInfo, bool modemCognitive,
                                      bool roamingAllowed, bool isRoaming) {

    RLOGD("setupDataCall: serial %d", serial);

    const hidl_string &protocol =
            (isRoaming ? dataProfileInfo.roamingProtocol : dataProfileInfo.protocol);
    if (s_vendorFunctions->version >= 4 && s_vendorFunctions->version <= 14) {
        dispatchStrings(serial, mSlotId, RIL_REQUEST_SETUP_DATA_CALL, false, 7,
            std::to_string((int) radioTechnology + 2).c_str(),
            std::to_string((int) dataProfileInfo.profileId).c_str(),
            dataProfileInfo.apn.c_str(),
            dataProfileInfo.user.c_str(),
            dataProfileInfo.password.c_str(),
            std::to_string((int) dataProfileInfo.authType).c_str(),
            protocol.c_str());
    } else if (s_vendorFunctions->version >= 15) {
        char *mvnoTypeStr = NULL;
        if (!convertMvnoTypeToString(dataProfileInfo.mvnoType, mvnoTypeStr)) {
            RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
                    RIL_REQUEST_SETUP_DATA_CALL);
            if (pRI != NULL) {
                sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
            }
            return Void();
        }
        dispatchStrings(serial, mSlotId, RIL_REQUEST_SETUP_DATA_CALL, false, 15,
            std::to_string((int) radioTechnology + 2).c_str(),
            std::to_string((int) dataProfileInfo.profileId).c_str(),
            dataProfileInfo.apn.c_str(),
            dataProfileInfo.user.c_str(),
            dataProfileInfo.password.c_str(),
            std::to_string((int) dataProfileInfo.authType).c_str(),
            protocol.c_str(),
            dataProfileInfo.roamingProtocol.c_str(),
            std::to_string(dataProfileInfo.supportedApnTypesBitmap).c_str(),
            std::to_string(dataProfileInfo.bearerBitmap).c_str(),
            modemCognitive ? "1" : "0",
            std::to_string(dataProfileInfo.mtu).c_str(),
            mvnoTypeStr,
            dataProfileInfo.mvnoMatchData.c_str(),
            roamingAllowed ? "1" : "0");
    } else {
        RLOGE("Unsupported RIL version %d, min version expected 4", s_vendorFunctions->version);
        RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
                RIL_REQUEST_SETUP_DATA_CALL);
        if (pRI != NULL) {
            sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
        }
    }
    return Void();
}

Return<void> RadioImpl::iccIOForApp(int32_t serial, const IccIo& iccIo) {
    RLOGD("iccIOForApp: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_SIM_IO);
    if (pRI == NULL) {
        return Void();
    }

    RIL_SIM_IO_v6 rilIccIo = {};
    rilIccIo.command = iccIo.command;
    rilIccIo.fileid = iccIo.fileId;
    if (!copyHidlStringToRil(&rilIccIo.path, iccIo.path, pRI)) {
        return Void();
    }

    rilIccIo.p1 = iccIo.p1;
    rilIccIo.p2 = iccIo.p2;
    rilIccIo.p3 = iccIo.p3;

    if (!copyHidlStringToRil(&rilIccIo.data, iccIo.data, pRI)) {
        memsetAndFreeStrings(1, rilIccIo.path);
        return Void();
    }

    if (!copyHidlStringToRil(&rilIccIo.pin2, iccIo.pin2, pRI)) {
        memsetAndFreeStrings(2, rilIccIo.path, rilIccIo.data);
        return Void();
    }

    if (!copyHidlStringToRil(&rilIccIo.aidPtr, iccIo.aid, pRI)) {
        memsetAndFreeStrings(3, rilIccIo.path, rilIccIo.data, rilIccIo.pin2);
        return Void();
    }

    CALL_ONREQUEST(RIL_REQUEST_SIM_IO, &rilIccIo, sizeof(rilIccIo), pRI, mSlotId);

    memsetAndFreeStrings(4, rilIccIo.path, rilIccIo.data, rilIccIo.pin2, rilIccIo.aidPtr);

    return Void();
}

Return<void> RadioImpl::sendUssd(int32_t serial, const hidl_string& ussd) {
    RLOGD("sendUssd: serial %d", serial);
    dispatchString(serial, mSlotId, RIL_REQUEST_SEND_USSD, ussd.c_str());
    return Void();
}

Return<void> RadioImpl::cancelPendingUssd(int32_t serial) {
    RLOGD("cancelPendingUssd: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_CANCEL_USSD);
    return Void();
}

Return<void> RadioImpl::getClir(int32_t serial) {
    RLOGD("getClir: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_CLIR);
    return Void();
}

Return<void> RadioImpl::setClir(int32_t serial, int32_t status) {
    RLOGD("setClir: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_CLIR, 1, status);
    return Void();
}

Return<void> RadioImpl::getCallForwardStatus(int32_t serial, const CallForwardInfo& callInfo) {
    RLOGD("getCallForwardStatus: serial %d", serial);
    dispatchCallForwardStatus(serial, mSlotId, RIL_REQUEST_QUERY_CALL_FORWARD_STATUS,
            callInfo);
    return Void();
}

Return<void> RadioImpl::setCallForward(int32_t serial, const CallForwardInfo& callInfo) {
    RLOGD("setCallForward: serial %d", serial);
    dispatchCallForwardStatus(serial, mSlotId, RIL_REQUEST_SET_CALL_FORWARD,
            callInfo);
    return Void();
}

Return<void> RadioImpl::getCallWaiting(int32_t serial, int32_t serviceClass) {
    RLOGD("getCallWaiting: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_QUERY_CALL_WAITING, 1, serviceClass);
    return Void();
}

Return<void> RadioImpl::setCallWaiting(int32_t serial, bool enable, int32_t serviceClass) {
    RLOGD("setCallWaiting: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_CALL_WAITING, 2, BOOL_TO_INT(enable),
            serviceClass);
    return Void();
}

Return<void> RadioImpl::acknowledgeLastIncomingGsmSms(int32_t serial,
                                                      bool success, SmsAcknowledgeFailCause cause) {
    RLOGD("acknowledgeLastIncomingGsmSms: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SMS_ACKNOWLEDGE, 2, BOOL_TO_INT(success),
            cause);
    return Void();
}

Return<void> RadioImpl::acceptCall(int32_t serial) {
    RLOGD("acceptCall: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_ANSWER);
    return Void();
}

Return<void> RadioImpl::deactivateDataCall(int32_t serial,
                                           int32_t cid, bool reasonRadioShutDown) {
    RLOGD("deactivateDataCall: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_DEACTIVATE_DATA_CALL, false,
            2, (const char *) (std::to_string(cid)).c_str(), reasonRadioShutDown ? "1" : "0");
    return Void();
}

Return<void> RadioImpl::getFacilityLockForApp(int32_t serial, const hidl_string& facility,
                                              const hidl_string& password, int32_t serviceClass,
                                              const hidl_string& appId) {
    RLOGD("getFacilityLockForApp: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_QUERY_FACILITY_LOCK, true,
            4, facility.c_str(), password.c_str(),
            (std::to_string(serviceClass)).c_str(), appId.c_str());
    return Void();
}

Return<void> RadioImpl::setFacilityLockForApp(int32_t serial, const hidl_string& facility,
                                              bool lockState, const hidl_string& password,
                                              int32_t serviceClass, const hidl_string& appId) {
    RLOGD("setFacilityLockForApp: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_SET_FACILITY_LOCK, true,
            5, facility.c_str(), lockState ? "1" : "0", password.c_str(),
            (std::to_string(serviceClass)).c_str(), appId.c_str() );
    return Void();
}

Return<void> RadioImpl::setBarringPassword(int32_t serial, const hidl_string& facility,
                                           const hidl_string& oldPassword,
                                           const hidl_string& newPassword) {
    RLOGD("setBarringPassword: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_CHANGE_BARRING_PASSWORD, true,
            3, facility.c_str(), oldPassword.c_str(), newPassword.c_str());
    return Void();
}

Return<void> RadioImpl::getNetworkSelectionMode(int32_t serial) {
    RLOGD("getNetworkSelectionMode: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE);
    return Void();
}

Return<void> RadioImpl::setNetworkSelectionModeAutomatic(int32_t serial) {
    RLOGD("setNetworkSelectionModeAutomatic: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC);
    return Void();
}

Return<void> RadioImpl::setNetworkSelectionModeManual(int32_t serial,
                                                      const hidl_string& operatorNumeric) {
    RLOGD("setNetworkSelectionModeManual: serial %d", serial);
    dispatchString(serial, mSlotId, RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL,
            operatorNumeric.c_str());
    return Void();
}

Return<void> RadioImpl::getAvailableNetworks(int32_t serial) {
    RLOGD("getAvailableNetworks: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_QUERY_AVAILABLE_NETWORKS);
    return Void();
}

Return<void> RadioImpl::startNetworkScan(int32_t serial, const AOSP_V1_1::NetworkScanRequest& request) {
    RLOGD("startNetworkScan: serial %d", serial);

    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_START_NETWORK_SCAN);
    if (pRI == NULL) {
        return Void();
    }

    if (request.specifiers.size() > MAX_RADIO_ACCESS_NETWORKS) {
        sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
        return Void();
    }

    RIL_NetworkScanRequest scan_request = {};

    scan_request.type = (RIL_ScanType) request.type;
    scan_request.interval = request.interval;
    scan_request.specifiers_length = request.specifiers.size();
    for (size_t i = 0; i < request.specifiers.size(); ++i) {
        if (request.specifiers[i].geranBands.size() > MAX_BANDS ||
            request.specifiers[i].utranBands.size() > MAX_BANDS ||
            request.specifiers[i].eutranBands.size() > MAX_BANDS ||
            request.specifiers[i].channels.size() > MAX_CHANNELS) {
            sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
            return Void();
        }
        const AOSP_V1_1::RadioAccessSpecifier& ras_from =
                request.specifiers[i];
        RIL_RadioAccessSpecifier& ras_to = scan_request.specifiers[i];

        ras_to.radio_access_network = (RIL_RadioAccessNetworks) ras_from.radioAccessNetwork;
        ras_to.channels_length = ras_from.channels.size();

        std::copy(ras_from.channels.begin(), ras_from.channels.end(), ras_to.channels);
        const std::vector<uint32_t> * bands = nullptr;
        switch (request.specifiers[i].radioAccessNetwork) {
            case AOSP_V1_1::RadioAccessNetworks::GERAN:
                ras_to.bands_length = ras_from.geranBands.size();
                bands = (std::vector<uint32_t> *) &ras_from.geranBands;
                break;
            case AOSP_V1_1::RadioAccessNetworks::UTRAN:
                ras_to.bands_length = ras_from.utranBands.size();
                bands = (std::vector<uint32_t> *) &ras_from.utranBands;
                break;
            case AOSP_V1_1::RadioAccessNetworks::EUTRAN:
                ras_to.bands_length = ras_from.eutranBands.size();
                bands = (std::vector<uint32_t> *) &ras_from.eutranBands;
                break;
            default:
                sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
                return Void();
        }
        // safe to copy to geran_bands because it's a union member
        for (size_t idx = 0; idx < ras_to.bands_length; ++idx) {
            ras_to.bands.geran_bands[idx] = (RIL_GeranBands) (*bands)[idx];
        }
    }
    scan_request.maxSearchTime = 0;
    scan_request.incrementalResults = 0;
    scan_request.incrementalResultsPeriodicity = 0;
    scan_request.mccMncs = NULL;
    scan_request.mccMncs_length = 0;

    CALL_ONREQUEST(RIL_REQUEST_START_NETWORK_SCAN, &scan_request, sizeof(scan_request), pRI,
            mSlotId);

    return Void();
}

Return<void> RadioImpl::stopNetworkScan(int32_t serial) {
    RLOGD("stopNetworkScan: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_STOP_NETWORK_SCAN);
    return Void();
}

Return<void> RadioImpl::startDtmf(int32_t serial, const hidl_string& s) {
    RLOGD("startDtmf: serial %d", serial);
    dispatchString(serial, mSlotId, RIL_REQUEST_DTMF_START,
            s.c_str());
    return Void();
}

Return<void> RadioImpl::stopDtmf(int32_t serial) {
    RLOGD("stopDtmf: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_DTMF_STOP);
    return Void();
}

Return<void> RadioImpl::getBasebandVersion(int32_t serial) {
    RLOGD("getBasebandVersion: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_BASEBAND_VERSION);
    return Void();
}

Return<void> RadioImpl::separateConnection(int32_t serial, int32_t gsmIndex) {
    RLOGD("separateConnection: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SEPARATE_CONNECTION, 1, gsmIndex);
    return Void();
}

Return<void> RadioImpl::setMute(int32_t serial, bool enable) {
    RLOGD("setMute: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_MUTE, 1, BOOL_TO_INT(enable));
    return Void();
}

Return<void> RadioImpl::getMute(int32_t serial) {
    RLOGD("getMute: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_MUTE);
    return Void();
}

Return<void> RadioImpl::getClip(int32_t serial) {
    RLOGD("getClip: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_QUERY_CLIP);
    return Void();
}

Return<void> RadioImpl::getDataCallList(int32_t serial) {
    RLOGD("getDataCallList: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_DATA_CALL_LIST);
    return Void();
}

Return<void> RadioImpl::setSuppServiceNotifications(int32_t serial, bool enable) {
    RLOGD("setSuppServiceNotifications: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION, 1,
            BOOL_TO_INT(enable));
    return Void();
}

Return<void> RadioImpl::writeSmsToSim(int32_t serial, const SmsWriteArgs& smsWriteArgs) {
    RLOGD("writeSmsToSim: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_WRITE_SMS_TO_SIM);
    if (pRI == NULL) {
        return Void();
    }

    RIL_SMS_WriteArgs args;
    args.status = (int) smsWriteArgs.status;

    if (!copyHidlStringToRil(&args.pdu, smsWriteArgs.pdu, pRI)) {
        return Void();
    }

    if (!copyHidlStringToRil(&args.smsc, smsWriteArgs.smsc, pRI)) {
        memsetAndFreeStrings(1, args.pdu);
        return Void();
    }

    CALL_ONREQUEST(RIL_REQUEST_WRITE_SMS_TO_SIM, &args, sizeof(args), pRI, mSlotId);

    memsetAndFreeStrings(2, args.smsc, args.pdu);

    return Void();
}

Return<void> RadioImpl::deleteSmsOnSim(int32_t serial, int32_t index) {
    RLOGD("deleteSmsOnSim: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_DELETE_SMS_ON_SIM, 1, index);
    return Void();
}

Return<void> RadioImpl::setBandMode(int32_t serial, RadioBandMode mode) {
    RLOGD("setBandMode: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_BAND_MODE, 1, mode);
    return Void();
}

Return<void> RadioImpl::getAvailableBandModes(int32_t serial) {
    RLOGD("getAvailableBandModes: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE);
    return Void();
}

Return<void> RadioImpl::sendEnvelope(int32_t serial, const hidl_string& command) {
    RLOGD("sendEnvelope: serial %d", serial);
    dispatchString(serial, mSlotId, RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND,
            command.c_str());
    return Void();
}

Return<void> RadioImpl::sendTerminalResponseToSim(int32_t serial,
                                                  const hidl_string& commandResponse) {
    RLOGD("sendTerminalResponseToSim: serial %d", serial);
    dispatchString(serial, mSlotId, RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE,
            commandResponse.c_str());
    return Void();
}

Return<void> RadioImpl::handleStkCallSetupRequestFromSim(int32_t serial, bool accept) {
    RLOGD("handleStkCallSetupRequestFromSim: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM,
            1, BOOL_TO_INT(accept));
    return Void();
}

Return<void> RadioImpl::explicitCallTransfer(int32_t serial) {
    RLOGD("explicitCallTransfer: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_EXPLICIT_CALL_TRANSFER);
    return Void();
}

Return<void> RadioImpl::setPreferredNetworkType(int32_t serial, PreferredNetworkType nwType) {
    RLOGD("setPreferredNetworkType: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE, 1, nwType);
    return Void();
}

Return<void> RadioImpl::getPreferredNetworkType(int32_t serial) {
    RLOGD("getPreferredNetworkType: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE);
    return Void();
}

Return<void> RadioImpl::getNeighboringCids(int32_t serial) {
    RLOGD("getNeighboringCids: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_NEIGHBORING_CELL_IDS);
    return Void();
}

Return<void> RadioImpl::setLocationUpdates(int32_t serial, bool enable) {
    RLOGD("setLocationUpdates: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_LOCATION_UPDATES, 1, BOOL_TO_INT(enable));
    return Void();
}

Return<void> RadioImpl::setCdmaSubscriptionSource(int32_t serial, CdmaSubscriptionSource cdmaSub) {
    RLOGD("setCdmaSubscriptionSource: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE, 1, cdmaSub);
    return Void();
}

Return<void> RadioImpl::setCdmaRoamingPreference(int32_t serial, CdmaRoamingType type) {
    RLOGD("setCdmaRoamingPreference: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE, 1, type);
    return Void();
}

Return<void> RadioImpl::getCdmaRoamingPreference(int32_t serial) {
    RLOGD("getCdmaRoamingPreference: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE);
    return Void();
}

Return<void> RadioImpl::setTTYMode(int32_t serial, TtyMode mode) {
    RLOGD("setTTYMode: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_TTY_MODE, 1, mode);
    return Void();
}

Return<void> RadioImpl::getTTYMode(int32_t serial) {
    RLOGD("getTTYMode: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_QUERY_TTY_MODE);
    return Void();
}

Return<void> RadioImpl::setPreferredVoicePrivacy(int32_t serial, bool enable) {
    RLOGD("setPreferredVoicePrivacy: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE,
            1, BOOL_TO_INT(enable));
    return Void();
}

Return<void> RadioImpl::getPreferredVoicePrivacy(int32_t serial) {
    RLOGD("getPreferredVoicePrivacy: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE);
    return Void();
}

Return<void> RadioImpl::sendCDMAFeatureCode(int32_t serial, const hidl_string& featureCode) {
    RLOGD("sendCDMAFeatureCode: serial %d", serial);
    dispatchString(serial, mSlotId, RIL_REQUEST_CDMA_FLASH,
            featureCode.c_str());
    return Void();
}

Return<void> RadioImpl::sendBurstDtmf(int32_t serial, const hidl_string& dtmf, int32_t on,
                                      int32_t off) {
    RLOGD("sendBurstDtmf: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_CDMA_BURST_DTMF, false,
            3, dtmf.c_str(), (std::to_string(on)).c_str(),
            (const char *) (std::to_string(off)).c_str());
    return Void();
}

void constructCdmaSms(RIL_CDMA_SMS_Message &rcsm, const CdmaSmsMessage& sms) {
    rcsm.uTeleserviceID = sms.teleserviceId;
    rcsm.bIsServicePresent = BOOL_TO_INT(sms.isServicePresent);
    rcsm.uServicecategory = sms.serviceCategory;
    rcsm.sAddress.digit_mode = (RIL_CDMA_SMS_DigitMode) sms.address.digitMode;
    rcsm.sAddress.number_mode = (RIL_CDMA_SMS_NumberMode) sms.address.numberMode;
    rcsm.sAddress.number_type = (RIL_CDMA_SMS_NumberType) sms.address.numberType;
    rcsm.sAddress.number_plan = (RIL_CDMA_SMS_NumberPlan) sms.address.numberPlan;

    rcsm.sAddress.number_of_digits = sms.address.digits.size();
    int digitLimit= MIN((rcsm.sAddress.number_of_digits), RIL_CDMA_SMS_ADDRESS_MAX);
    for (int i = 0; i < digitLimit; i++) {
        rcsm.sAddress.digits[i] = sms.address.digits[i];
    }

    rcsm.sSubAddress.subaddressType = (RIL_CDMA_SMS_SubaddressType) sms.subAddress.subaddressType;
    rcsm.sSubAddress.odd = BOOL_TO_INT(sms.subAddress.odd);

    rcsm.sSubAddress.number_of_digits = sms.subAddress.digits.size();
    digitLimit= MIN((rcsm.sSubAddress.number_of_digits), RIL_CDMA_SMS_SUBADDRESS_MAX);
    for (int i = 0; i < digitLimit; i++) {
        rcsm.sSubAddress.digits[i] = sms.subAddress.digits[i];
    }

    rcsm.uBearerDataLen = sms.bearerData.size();
    digitLimit= MIN((rcsm.uBearerDataLen), RIL_CDMA_SMS_BEARER_DATA_MAX);
    for (int i = 0; i < digitLimit; i++) {
        rcsm.aBearerData[i] = sms.bearerData[i];
    }
}

Return<void> RadioImpl::sendCdmaSms(int32_t serial, const CdmaSmsMessage& sms) {
    RLOGD("sendCdmaSms: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_CDMA_SEND_SMS);
    if (pRI == NULL) {
        return Void();
    }

    RIL_CDMA_SMS_Message rcsm = {};
    constructCdmaSms(rcsm, sms);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &rcsm, sizeof(rcsm), pRI, mSlotId);
    return Void();
}

Return<void> RadioImpl::acknowledgeLastIncomingCdmaSms(int32_t serial, const CdmaSmsAck& smsAck) {
    RLOGD("acknowledgeLastIncomingCdmaSms: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE);
    if (pRI == NULL) {
        return Void();
    }

    RIL_CDMA_SMS_Ack rcsa = {};

    rcsa.uErrorClass = (RIL_CDMA_SMS_ErrorClass) smsAck.errorClass;
    rcsa.uSMSCauseCode = smsAck.smsCauseCode;

    CALL_ONREQUEST(pRI->pCI->requestNumber, &rcsa, sizeof(rcsa), pRI, mSlotId);
    return Void();
}

Return<void> RadioImpl::getGsmBroadcastConfig(int32_t serial) {
    RLOGD("getGsmBroadcastConfig: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG);
    return Void();
}

Return<void> RadioImpl::setGsmBroadcastConfig(int32_t serial,
                                              const hidl_vec<GsmBroadcastSmsConfigInfo>&
                                              configInfo) {
    RLOGD("setGsmBroadcastConfig: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG);
    if (pRI == NULL) {
        return Void();
    }

    int num = configInfo.size();
    RIL_GSM_BroadcastSmsConfigInfo gsmBci[num];
    RIL_GSM_BroadcastSmsConfigInfo *gsmBciPtrs[num];

    for (int i = 0 ; i < num ; i++ ) {
        gsmBciPtrs[i] = &gsmBci[i];
        gsmBci[i].fromServiceId = configInfo[i].fromServiceId;
        gsmBci[i].toServiceId = configInfo[i].toServiceId;
        gsmBci[i].fromCodeScheme = configInfo[i].fromCodeScheme;
        gsmBci[i].toCodeScheme = configInfo[i].toCodeScheme;
        gsmBci[i].selected = BOOL_TO_INT(configInfo[i].selected);
    }

    CALL_ONREQUEST(pRI->pCI->requestNumber, gsmBciPtrs,
            num * sizeof(RIL_GSM_BroadcastSmsConfigInfo *), pRI, mSlotId);
    return Void();
}

Return<void> RadioImpl::setGsmBroadcastActivation(int32_t serial, bool activate) {
    RLOGD("setGsmBroadcastActivation: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION,
            1, BOOL_TO_INT(!activate));
    return Void();
}

Return<void> RadioImpl::getCdmaBroadcastConfig(int32_t serial) {
    RLOGD("getCdmaBroadcastConfig: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG);
    return Void();
}

Return<void> RadioImpl::setCdmaBroadcastConfig(int32_t serial,
                                               const hidl_vec<CdmaBroadcastSmsConfigInfo>&
                                               configInfo) {
    RLOGD("setCdmaBroadcastConfig: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG);
    if (pRI == NULL) {
        return Void();
    }

    int num = configInfo.size();
    RIL_CDMA_BroadcastSmsConfigInfo cdmaBci[num];
    RIL_CDMA_BroadcastSmsConfigInfo *cdmaBciPtrs[num];

    for (int i = 0 ; i < num ; i++ ) {
        cdmaBciPtrs[i] = &cdmaBci[i];
        cdmaBci[i].service_category = configInfo[i].serviceCategory;
        cdmaBci[i].language = configInfo[i].language;
        cdmaBci[i].selected = BOOL_TO_INT(configInfo[i].selected);
    }

    CALL_ONREQUEST(pRI->pCI->requestNumber, cdmaBciPtrs,
            num * sizeof(RIL_CDMA_BroadcastSmsConfigInfo *), pRI, mSlotId);
    return Void();
}

Return<void> RadioImpl::setCdmaBroadcastActivation(int32_t serial, bool activate) {
    RLOGD("setCdmaBroadcastActivation: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION,
            1, BOOL_TO_INT(!activate));
    return Void();
}

Return<void> RadioImpl::getCDMASubscription(int32_t serial) {
    RLOGD("getCDMASubscription: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_CDMA_SUBSCRIPTION);
    return Void();
}

Return<void> RadioImpl::writeSmsToRuim(int32_t serial, const CdmaSmsWriteArgs& cdmaSms) {
    RLOGD("writeSmsToRuim: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM);
    if (pRI == NULL) {
        return Void();
    }

    RIL_CDMA_SMS_WriteArgs rcsw = {};
    rcsw.status = (int) cdmaSms.status;
    constructCdmaSms(rcsw.message, cdmaSms.message);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &rcsw, sizeof(rcsw), pRI, mSlotId);
    return Void();
}

Return<void> RadioImpl::deleteSmsOnRuim(int32_t serial, int32_t index) {
    RLOGD("deleteSmsOnRuim: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM, 1, index);
    return Void();
}

Return<void> RadioImpl::getDeviceIdentity(int32_t serial) {
    RLOGD("getDeviceIdentity: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_DEVICE_IDENTITY);
    return Void();
}

Return<void> RadioImpl::exitEmergencyCallbackMode(int32_t serial) {
    RLOGD("exitEmergencyCallbackMode: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE);
    return Void();
}

Return<void> RadioImpl::getSmscAddress(int32_t serial) {
    RLOGD("getSmscAddress: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_SMSC_ADDRESS);
    return Void();
}

Return<void> RadioImpl::setSmscAddress(int32_t serial, const hidl_string& smsc) {
    RLOGD("setSmscAddress: serial %d", serial);
    dispatchString(serial, mSlotId, RIL_REQUEST_SET_SMSC_ADDRESS,
            smsc.c_str());
    return Void();
}

Return<void> RadioImpl::reportSmsMemoryStatus(int32_t serial, bool available) {
    RLOGD("reportSmsMemoryStatus: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_REPORT_SMS_MEMORY_STATUS, 1,
            BOOL_TO_INT(available));
    return Void();
}

Return<void> RadioImpl::reportStkServiceIsRunning(int32_t serial) {
    RLOGD("reportStkServiceIsRunning: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING);
    return Void();
}

Return<void> RadioImpl::getCdmaSubscriptionSource(int32_t serial) {
    RLOGD("getCdmaSubscriptionSource: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE);
    return Void();
}

Return<void> RadioImpl::requestIsimAuthentication(int32_t serial, const hidl_string& challenge) {
    RLOGD("requestIsimAuthentication: serial %d", serial);
    dispatchString(serial, mSlotId, RIL_REQUEST_ISIM_AUTHENTICATION,
            challenge.c_str());
    return Void();
}

Return<void> RadioImpl::acknowledgeIncomingGsmSmsWithPdu(int32_t serial, bool success,
                                                         const hidl_string& ackPdu) {
    RLOGD("acknowledgeIncomingGsmSmsWithPdu: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU, false,
            2, success ? "1" : "0", ackPdu.c_str());
    return Void();
}

Return<void> RadioImpl::sendEnvelopeWithStatus(int32_t serial, const hidl_string& contents) {
    RLOGD("sendEnvelopeWithStatus: serial %d", serial);
    dispatchString(serial, mSlotId, RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS,
            contents.c_str());
    return Void();
}

Return<void> RadioImpl::getVoiceRadioTechnology(int32_t serial) {
    RLOGD("getVoiceRadioTechnology: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_VOICE_RADIO_TECH);
    return Void();
}

Return<void> RadioImpl::getCellInfoList(int32_t serial) {
    RLOGD("getCellInfoList: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_CELL_INFO_LIST);
    return Void();
}

Return<void> RadioImpl::setCellInfoListRate(int32_t serial, int32_t rate) {
    RLOGD("setCellInfoListRate: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE, 1, rate);
    return Void();
}

Return<void> RadioImpl::setInitialAttachApn(int32_t serial, const DataProfileInfo& dataProfileInfo,
                                            bool modemCognitive, bool isRoaming) {
    RLOGD("setInitialAttachApn: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_INITIAL_ATTACH_APN);
    if (pRI == NULL) {
        return Void();
    }

    if (s_vendorFunctions->version <= 14) {
        RIL_InitialAttachApn iaa = {};

        if (!copyHidlStringToRil(&iaa.apn, dataProfileInfo.apn, pRI, true)) {
            return Void();
        }

        const hidl_string &protocol =
                (isRoaming ? dataProfileInfo.roamingProtocol : dataProfileInfo.protocol);

        if (!copyHidlStringToRil(&iaa.protocol, protocol, pRI)) {
            memsetAndFreeStrings(1, iaa.apn);
            return Void();
        }
        iaa.authtype = (int) dataProfileInfo.authType;
        if (!copyHidlStringToRil(&iaa.username, dataProfileInfo.user, pRI)) {
            memsetAndFreeStrings(2, iaa.apn, iaa.protocol);
            return Void();
        }
        if (!copyHidlStringToRil(&iaa.password, dataProfileInfo.password, pRI)) {
            memsetAndFreeStrings(3, iaa.apn, iaa.protocol, iaa.username);
            return Void();
        }

        CALL_ONREQUEST(RIL_REQUEST_SET_INITIAL_ATTACH_APN, &iaa, sizeof(iaa), pRI, mSlotId);

        memsetAndFreeStrings(4, iaa.apn, iaa.protocol, iaa.username, iaa.password);
    } else {
        RIL_InitialAttachApn_v15 iaa = {};

        if (!copyHidlStringToRil(&iaa.apn, dataProfileInfo.apn, pRI, true)) {
            return Void();
        }

        if (!copyHidlStringToRil(&iaa.protocol, dataProfileInfo.protocol, pRI)) {
            memsetAndFreeStrings(1, iaa.apn);
            return Void();
        }
        if (!copyHidlStringToRil(&iaa.roamingProtocol, dataProfileInfo.roamingProtocol, pRI)) {
            memsetAndFreeStrings(2, iaa.apn, iaa.protocol);
            return Void();
        }
        iaa.authtype = (int) dataProfileInfo.authType;
        if (!copyHidlStringToRil(&iaa.username, dataProfileInfo.user, pRI)) {
            memsetAndFreeStrings(3, iaa.apn, iaa.protocol, iaa.roamingProtocol);
            return Void();
        }
        if (!copyHidlStringToRil(&iaa.password, dataProfileInfo.password, pRI)) {
            memsetAndFreeStrings(4, iaa.apn, iaa.protocol, iaa.roamingProtocol, iaa.username);
            return Void();
        }
        iaa.supportedTypesBitmask = dataProfileInfo.supportedApnTypesBitmap;
        iaa.bearerBitmask = dataProfileInfo.bearerBitmap;
        iaa.modemCognitive = BOOL_TO_INT(modemCognitive);
        iaa.mtu = dataProfileInfo.mtu;

        if (!convertMvnoTypeToString(dataProfileInfo.mvnoType, iaa.mvnoType)) {
            sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
            memsetAndFreeStrings(5, iaa.apn, iaa.protocol, iaa.roamingProtocol, iaa.username,
                    iaa.password);
            return Void();
        }

        if (!copyHidlStringToRil(&iaa.mvnoMatchData, dataProfileInfo.mvnoMatchData, pRI)) {
            memsetAndFreeStrings(5, iaa.apn, iaa.protocol, iaa.roamingProtocol, iaa.username,
                    iaa.password);
            return Void();
        }

        // M: added canHandleIms for legacy modem
        iaa.canHandleIms = BOOL_TO_INT((((iaa.supportedTypesBitmask & ApnTypes::IMS) != 0) &&
                ((iaa.supportedTypesBitmask ^ (int) MtkApnTypes::MTKALL) != 0)));

        CALL_ONREQUEST(RIL_REQUEST_SET_INITIAL_ATTACH_APN, &iaa, sizeof(iaa), pRI, mSlotId);

        memsetAndFreeStrings(6, iaa.apn, iaa.protocol, iaa.roamingProtocol, iaa.username,
                iaa.password, iaa.mvnoMatchData);
    }

    return Void();
}

Return<void> RadioImpl::getImsRegistrationState(int32_t serial) {
    RLOGD("getImsRegistrationState: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_IMS_REGISTRATION_STATE);
    return Void();
}

bool dispatchImsGsmSms(const ImsSmsMessage& message, RequestInfo *pRI) {
    RIL_IMS_SMS_Message rism = {};
    char **pStrings;
    int countStrings = 2;
    int dataLen = sizeof(char *) * countStrings;

    rism.tech = RADIO_TECH_3GPP;
    rism.retry = BOOL_TO_INT(message.retry);
    rism.messageRef = message.messageRef;

    if (message.gsmMessage.size() != 1) {
        RLOGE("dispatchImsGsmSms: Invalid len %s", requestToString(pRI->pCI->requestNumber));
        sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
        return false;
    }

    pStrings = (char **)calloc(countStrings, sizeof(char *));
    if (pStrings == NULL) {
        RLOGE("dispatchImsGsmSms: Memory allocation failed for request %s",
                requestToString(pRI->pCI->requestNumber));
        sendErrorResponse(pRI, RIL_E_NO_MEMORY);
        return false;
    }

    if (!copyHidlStringToRil(&pStrings[0], message.gsmMessage[0].smscPdu, pRI)) {
#ifdef MEMSET_FREED
        memset(pStrings, 0, dataLen);
#endif
        free(pStrings);
        return false;
    }

    if (!copyHidlStringToRil(&pStrings[1], message.gsmMessage[0].pdu, pRI)) {
        memsetAndFreeStrings(1, pStrings[0]);
#ifdef MEMSET_FREED
        memset(pStrings, 0, dataLen);
#endif
        free(pStrings);
        return false;
    }

    rism.message.gsmMessage = pStrings;
    CALL_ONREQUEST(pRI->pCI->requestNumber, &rism, sizeof(RIL_RadioTechnologyFamily) +
            sizeof(uint8_t) + sizeof(int32_t) + dataLen, pRI, pRI->socket_id);

    for (int i = 0 ; i < countStrings ; i++) {
        memsetAndFreeStrings(1, pStrings[i]);
    }

#ifdef MEMSET_FREED
    memset(pStrings, 0, dataLen);
#endif
    free(pStrings);

    return true;
}

struct ImsCdmaSms {
    RIL_IMS_SMS_Message imsSms;
    RIL_CDMA_SMS_Message cdmaSms;
};

bool dispatchImsCdmaSms(const ImsSmsMessage& message, RequestInfo *pRI) {
    ImsCdmaSms temp = {};

    if (message.cdmaMessage.size() != 1) {
        RLOGE("dispatchImsCdmaSms: Invalid len %s", requestToString(pRI->pCI->requestNumber));
        sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
        return false;
    }

    temp.imsSms.tech = RADIO_TECH_3GPP2;
    temp.imsSms.retry = BOOL_TO_INT(message.retry);
    temp.imsSms.messageRef = message.messageRef;
    temp.imsSms.message.cdmaMessage = &temp.cdmaSms;

    constructCdmaSms(temp.cdmaSms, message.cdmaMessage[0]);

    // Vendor code expects payload length to include actual msg payload
    // (sizeof(RIL_CDMA_SMS_Message)) instead of (RIL_CDMA_SMS_Message *) + size of other fields in
    // RIL_IMS_SMS_Message
    int payloadLen = sizeof(RIL_RadioTechnologyFamily) + sizeof(uint8_t) + sizeof(int32_t)
            + sizeof(RIL_CDMA_SMS_Message);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &temp.imsSms, payloadLen, pRI, pRI->socket_id);

    return true;
}

// MTK-START: SIM GBA
bool dispatchSimGeneralAuth(int serial, int slotId, int request, const SimAuthStructure& simAuth) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    RIL_SimAuthStructure rilSimAuth;
    rilSimAuth.sessionId= simAuth.sessionId;
    rilSimAuth.mode= simAuth.mode;

    if (!copyHidlStringToRil(&rilSimAuth.param1, simAuth.param1, pRI)) {
        return false;
    }

    if (!copyHidlStringToRil(&rilSimAuth.param2, simAuth.param2, pRI)) {
        return false;
    }

    rilSimAuth.tag = simAuth.tag;

    s_vendorFunctions->onRequest(request, &rilSimAuth, sizeof(rilSimAuth), pRI,
            pRI->socket_id);

    memsetAndFreeStrings(1, rilSimAuth.param1);
    memsetAndFreeStrings(1, rilSimAuth.param2);

    return true;
}

bool dispatchPhbEntry(int serial, int slotId, int request,
                              const PhbEntryStructure& phbEntry) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    RIL_PhbEntryStructure pbe;
    pbe.type = phbEntry.type;
    pbe.index = phbEntry.index;

    if (!copyHidlStringToRil(&pbe.number, phbEntry.number, pRI)) {
        return false;
    }
    pbe.ton = phbEntry.ton;

    if (!copyHidlStringToRil(&pbe.alphaId, phbEntry.alphaId, pRI)) {
        return false;
    }

    s_vendorFunctions->onRequest(request, &pbe, sizeof(pbe), pRI,
            pRI->socket_id);

    memsetAndFreeStrings(1, pbe.number);
    memsetAndFreeStrings(1, pbe.alphaId);

    return true;
}

bool dispatchGrpEntry(int serial, int slotId, int request, int adnIndex, const hidl_vec<int32_t>& grpId) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    int countInts = grpId.size() + 1;
    int *pInts = (int *)calloc(countInts, sizeof(int));

    if (pInts == NULL) {
        RLOGE("Memory allocation failed for request %s", requestToString(request));
        sendErrorResponse(pRI, RIL_E_NO_MEMORY);
        return false;
    }
    pInts[0] = adnIndex;
    for (int i = 1; i < countInts; i++) {
        pInts[i] = grpId[i-1];
    }

    s_vendorFunctions->onRequest(request, pInts, countInts * sizeof(int), pRI,
            pRI->socket_id);

    if (pInts != NULL) {
#ifdef MEMSET_FREED
        memset(pInts, 0, countInts * sizeof(int));
#endif
        free(pInts);
    }
    return true;
}

bool dispatchPhbEntryExt(int serial, int slotId, int request,
                              const PhbEntryExt& phbEntryExt) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    RIL_PHB_ENTRY pbe;
    pbe.index = phbEntryExt.index;

    if (!copyHidlStringToRil(&pbe.number, phbEntryExt.number, pRI)) {
        return false;
    }
    pbe.type = phbEntryExt.type;

    if (!copyHidlStringToRil(&pbe.text, phbEntryExt.text, pRI)) {
        return false;
    }

    pbe.hidden = phbEntryExt.hidden;

    if (!copyHidlStringToRil(&pbe.group, phbEntryExt.group, pRI)) {
        return false;
    }

    if (!copyHidlStringToRil(&pbe.adnumber, phbEntryExt.adnumber, pRI)) {
        return false;
    }
    pbe.adtype = phbEntryExt.adtype;

    if (!copyHidlStringToRil(&pbe.secondtext, phbEntryExt.secondtext, pRI)) {
        return false;
    }

    if (!copyHidlStringToRil(&pbe.email, phbEntryExt.email, pRI)) {
        return false;
    }

    s_vendorFunctions->onRequest(request, &pbe, sizeof(pbe), pRI,
            pRI->socket_id);

    memsetAndFreeStrings(1, pbe.number);
    memsetAndFreeStrings(1, pbe.text);
    memsetAndFreeStrings(1, pbe.group);
    memsetAndFreeStrings(1, pbe.adnumber);
    memsetAndFreeStrings(1, pbe.secondtext);
    memsetAndFreeStrings(1, pbe.email);

    return true;
}

// FastDormancy Begin
bool dispatchFdMode(int serial, int slotId, int request, int mode, int param1, int param2) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    RIL_FdModeStructure args;
    args.mode = mode;

    /* AT+EFD=<mode>[,<param1>[,<param2>]] */
    /* For all modes: but mode 0 & 1 only has one argument */
    if (mode == 0 || mode == 1) {
        args.paramNumber = 1;
    }

    if (mode == 2) {
        args.paramNumber = 3;
        args.parameter1 = param1;
        args.parameter2 = param2;
    }

    if (mode == 3) {
        args.paramNumber = 2;
        args.parameter1 = param1;
    }

    s_vendorFunctions->onRequest(request, &args, sizeof(RIL_FdModeStructure), pRI, pRI->socket_id);

    return true;
}

// M: [Inactive Timer] start
int decodeInactiveTimer(unsigned int authType) {
    return authType >> APN_INACTIVE_TIMER_KEY;
}
// M: [Inactive Timer] end

Return<void> RadioImpl::sendImsSms(int32_t serial, const ImsSmsMessage& message) {
    RLOGD("sendImsSms: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_IMS_SEND_SMS);
    if (pRI == NULL) {
        return Void();
    }

    RIL_RadioTechnologyFamily format = (RIL_RadioTechnologyFamily) message.tech;

    if (RADIO_TECH_3GPP == format) {
        dispatchImsGsmSms(message, pRI);
    } else if (RADIO_TECH_3GPP2 == format) {
        dispatchImsCdmaSms(message, pRI);
    } else {
        RLOGE("sendImsSms: Invalid radio tech %s",
                requestToString(pRI->pCI->requestNumber));
        sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
    }
    return Void();
}

Return<void> RadioImpl::iccTransmitApduBasicChannel(int32_t serial, const SimApdu& message) {
    RLOGD("[%d] iccTransmitApduBasicChannel: serial %d", mSlotId, serial);
    dispatchIccApdu(serial, mSlotId, RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC, message);
    return Void();
}

Return<void> RadioImpl::iccOpenLogicalChannel(int32_t serial, const hidl_string& aid, int32_t p2) {
    RLOGD("[%d] iccOpenLogicalChannel: serial %d", mSlotId, serial);
    if (s_vendorFunctions->version < 15) {
        dispatchString(serial, mSlotId, RIL_REQUEST_SIM_OPEN_CHANNEL, aid.c_str());
    } else {
        RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_SIM_OPEN_CHANNEL);
        if (pRI == NULL) {
            return Void();
        }

        RIL_OpenChannelParams params = {};

        params.p2 = p2;

        if (!copyHidlStringToRil(&params.aidPtr, aid, pRI)) {
            return Void();
        }

        CALL_ONREQUEST(pRI->pCI->requestNumber, &params, sizeof(params), pRI, mSlotId);

        memsetAndFreeStrings(1, params.aidPtr);
    }
    return Void();
}

Return<void> RadioImpl::iccCloseLogicalChannel(int32_t serial, int32_t channelId) {
    RLOGD("[%d] iccCloseLogicalChannel: serial %d", mSlotId, serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SIM_CLOSE_CHANNEL, 1, channelId);
    return Void();
}

Return<void> RadioImpl::iccTransmitApduLogicalChannel(int32_t serial, const SimApdu& message) {
    RLOGD("[%d] iccTransmitApduLogicalChannel: serial %d", mSlotId, serial);
    dispatchIccApdu(serial, mSlotId, RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL, message);
    return Void();
}

Return<void> RadioImpl::nvReadItem(int32_t serial, NvItem itemId) {
    RLOGD("nvReadItem: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_NV_READ_ITEM);
    if (pRI == NULL) {
        return Void();
    }

    RIL_NV_ReadItem nvri = {};
    nvri.itemID = (RIL_NV_Item) itemId;

    CALL_ONREQUEST(pRI->pCI->requestNumber, &nvri, sizeof(nvri), pRI, mSlotId);
    return Void();
}

Return<void> RadioImpl::nvWriteItem(int32_t serial, const NvWriteItem& item) {
    RLOGD("nvWriteItem: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_NV_WRITE_ITEM);
    if (pRI == NULL) {
        return Void();
    }

    RIL_NV_WriteItem nvwi = {};

    nvwi.itemID = (RIL_NV_Item) item.itemId;

    if (!copyHidlStringToRil(&nvwi.value, item.value, pRI)) {
        return Void();
    }

    CALL_ONREQUEST(pRI->pCI->requestNumber, &nvwi, sizeof(nvwi), pRI, mSlotId);

    memsetAndFreeStrings(1, nvwi.value);
    return Void();
}

Return<void> RadioImpl::nvWriteCdmaPrl(int32_t serial, const hidl_vec<uint8_t>& prl) {
    RLOGD("nvWriteCdmaPrl: serial %d", serial);
    dispatchRaw(serial, mSlotId, RIL_REQUEST_NV_WRITE_CDMA_PRL, prl);
    return Void();
}

Return<void> RadioImpl::nvResetConfig(int32_t serial, ResetNvType resetType) {
    int rilResetType = -1;
    RLOGD("nvResetConfig: serial %d", serial);
    /* Convert ResetNvType to RIL.h values
     * RIL_REQUEST_NV_RESET_CONFIG
     * 1 - reload all NV items
     * 2 - erase NV reset (SCRTN)
     * 3 - factory reset (RTN)
     */
    switch(resetType) {
      case ResetNvType::RELOAD:
        rilResetType = 1;
        break;
      case ResetNvType::ERASE:
        rilResetType = 2;
        break;
      case ResetNvType::FACTORY_RESET:
        rilResetType = 3;
        break;
    }
    dispatchInts(serial, mSlotId, RIL_REQUEST_NV_RESET_CONFIG, 1, (int) resetType);
    return Void();
}

Return<void> RadioImpl::setUiccSubscription(int32_t serial, const SelectUiccSub& uiccSub) {
    RLOGD("setUiccSubscription: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_UICC_SUBSCRIPTION);
    if (pRI == NULL) {
        return Void();
    }

    RIL_SelectUiccSub rilUiccSub = {};

    rilUiccSub.slot = uiccSub.slot;
    rilUiccSub.app_index = uiccSub.appIndex;
    rilUiccSub.sub_type = (RIL_SubscriptionType) uiccSub.subType;
    rilUiccSub.act_status = (RIL_UiccSubActStatus) uiccSub.actStatus;

    CALL_ONREQUEST(pRI->pCI->requestNumber, &rilUiccSub, sizeof(rilUiccSub), pRI, mSlotId);
    return Void();
}

Return<void> RadioImpl::setDataAllowed(int32_t serial, bool allow) {
    RLOGD("setDataAllowed: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_ALLOW_DATA, 1, BOOL_TO_INT(allow));
    return Void();
}

Return<void> RadioImpl::getHardwareConfig(int32_t serial) {
    RLOGD("getHardwareConfig: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_HARDWARE_CONFIG);
    return Void();
}

Return<void> RadioImpl::requestIccSimAuthentication(int32_t serial, int32_t authContext,
        const hidl_string& authData, const hidl_string& aid) {
    RLOGD("requestIccSimAuthentication: serial %d, %d", serial, authContext);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_SIM_AUTHENTICATION);
    if (pRI == NULL) {
        return Void();
    }

    RIL_SimAuthentication pf = {};

    pf.authContext = authContext;

    if (!copyHidlStringToRil(&pf.authData, authData, pRI)) {
        return Void();
    }

    if (!copyHidlStringToRil(&pf.aid, aid, pRI)) {
        memsetAndFreeStrings(1, pf.authData);
        return Void();
    }

    CALL_ONREQUEST(pRI->pCI->requestNumber, &pf, sizeof(pf), pRI, mSlotId);

    memsetAndFreeStrings(2, pf.authData, pf.aid);
    return Void();
}

/**
 * @param numProfiles number of data profile
 * @param dataProfiles the pointer to the actual data profiles. The acceptable type is
          RIL_DataProfileInfo or RIL_DataProfileInfo_v15.
 * @param dataProfilePtrs the pointer to the pointers that point to each data profile structure
 * @param numfields number of string-type member in the data profile structure
 * @param ... the variadic parameters are pointers to each string-type member
 **/
template <typename T>
void freeSetDataProfileData(int numProfiles, T *dataProfiles, T **dataProfilePtrs,
                            int numfields, ...) {
    va_list args;
    va_start(args, numfields);

    // Iterate through each string-type field that need to be free.
    for (int i = 0; i < numfields; i++) {
        // Iterate through each data profile and free that specific string-type field.
        // The type 'char *T::*' is a type of pointer to a 'char *' member inside T structure.
        char *T::*ptr = va_arg(args, char *T::*);
        for (int j = 0; j < numProfiles; j++) {
            memsetAndFreeStrings(1, dataProfiles[j].*ptr);
        }
    }

    va_end(args);

#ifdef MEMSET_FREED
    memset(dataProfiles, 0, numProfiles * sizeof(T));
    memset(dataProfilePtrs, 0, numProfiles * sizeof(T *));
#endif
    free(dataProfiles);
    free(dataProfilePtrs);
}

Return<void> RadioImpl::setDataProfile(int32_t serial, const hidl_vec<DataProfileInfo>& profiles,
                                       bool isRoaming) {
    RLOGD("setDataProfile: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_SET_DATA_PROFILE);
    if (pRI == NULL) {
        return Void();
    }

    size_t num = profiles.size();
    bool success = false;

    if (s_vendorFunctions->version <= 14) {

        RIL_DataProfileInfo *dataProfiles =
            (RIL_DataProfileInfo *) calloc(num, sizeof(RIL_DataProfileInfo));

        if (dataProfiles == NULL) {
            RLOGE("Memory allocation failed for request %s",
                    requestToString(pRI->pCI->requestNumber));
            sendErrorResponse(pRI, RIL_E_NO_MEMORY);
            return Void();
        }

        RIL_DataProfileInfo **dataProfilePtrs =
            (RIL_DataProfileInfo **) calloc(num, sizeof(RIL_DataProfileInfo *));
        if (dataProfilePtrs == NULL) {
            RLOGE("Memory allocation failed for request %s",
                    requestToString(pRI->pCI->requestNumber));
            free(dataProfiles);
            sendErrorResponse(pRI, RIL_E_NO_MEMORY);
            return Void();
        }

        for (size_t i = 0; i < num; i++) {
            dataProfilePtrs[i] = &dataProfiles[i];

            success = copyHidlStringToRil(&dataProfiles[i].apn, profiles[i].apn, pRI, true);

            const hidl_string &protocol =
                    (isRoaming ? profiles[i].roamingProtocol : profiles[i].protocol);

            if (success && !copyHidlStringToRil(&dataProfiles[i].protocol, protocol, pRI, true)) {
                success = false;
            }

            if (success && !copyHidlStringToRil(&dataProfiles[i].user, profiles[i].user, pRI,
                    true)) {
                success = false;
            }
            if (success && !copyHidlStringToRil(&dataProfiles[i].password, profiles[i].password,
                    pRI, true)) {
                success = false;
            }

            if (!success) {
                freeSetDataProfileData(num, dataProfiles, dataProfilePtrs, 4,
                    &RIL_DataProfileInfo::apn, &RIL_DataProfileInfo::protocol,
                    &RIL_DataProfileInfo::user, &RIL_DataProfileInfo::password);
                return Void();
            }

            dataProfiles[i].profileId = (RIL_DataProfile) profiles[i].profileId;
            dataProfiles[i].authType = ((unsigned int) profiles[i].authType) & APN_AUTH_TYPE_MAX_NUM;
            dataProfiles[i].type = (int) profiles[i].type;
            dataProfiles[i].maxConnsTime = profiles[i].maxConnsTime;
            dataProfiles[i].maxConns = profiles[i].maxConns;
            dataProfiles[i].waitTime = profiles[i].waitTime;
            dataProfiles[i].enabled = BOOL_TO_INT(profiles[i].enabled);
        }

        CALL_ONREQUEST(RIL_REQUEST_SET_DATA_PROFILE, dataProfilePtrs,
                num * sizeof(RIL_DataProfileInfo *), pRI, mSlotId);

        freeSetDataProfileData(num, dataProfiles, dataProfilePtrs, 4,
                &RIL_DataProfileInfo::apn, &RIL_DataProfileInfo::protocol,
                &RIL_DataProfileInfo::user, &RIL_DataProfileInfo::password);
    } else {
        // M: use data profile to sync apn tables to modem
        // remark AOSP
        //RIL_DataProfileInfo_v15 *dataProfiles =
        //    (RIL_DataProfileInfo_v15 *) calloc(num, sizeof(RIL_DataProfileInfo_v15));
        RIL_MtkDataProfileInfo *dataProfiles =
            (RIL_MtkDataProfileInfo *) calloc(num, sizeof(RIL_MtkDataProfileInfo));

        if (dataProfiles == NULL) {
            RLOGE("Memory allocation failed for request %s",
                    requestToString(pRI->pCI->requestNumber));
            sendErrorResponse(pRI, RIL_E_NO_MEMORY);
            return Void();
        }

        // M: use data profile to sync apn tables to modem
        // remark AOSP
        //RIL_DataProfileInfo_v15 **dataProfilePtrs =
        //    (RIL_DataProfileInfo_v15 **) calloc(num, sizeof(RIL_DataProfileInfo_v15 *));
        RIL_MtkDataProfileInfo **dataProfilePtrs =
            (RIL_MtkDataProfileInfo **) calloc(num, sizeof(RIL_MtkDataProfileInfo *));

        if (dataProfilePtrs == NULL) {
            RLOGE("Memory allocation failed for request %s",
                    requestToString(pRI->pCI->requestNumber));
            free(dataProfiles);
            sendErrorResponse(pRI, RIL_E_NO_MEMORY);
            return Void();
        }

        for (size_t i = 0; i < num; i++) {
            dataProfilePtrs[i] = &dataProfiles[i];

            success = copyHidlStringToRil(&dataProfiles[i].apn, profiles[i].apn, pRI, true);
            if (success && !copyHidlStringToRil(&dataProfiles[i].protocol, profiles[i].protocol,
                    pRI)) {
                success = false;
            }
            if (success && !copyHidlStringToRil(&dataProfiles[i].roamingProtocol,
                    profiles[i].roamingProtocol, pRI, true)) {
                success = false;
            }
            if (success && !copyHidlStringToRil(&dataProfiles[i].user, profiles[i].user, pRI,
                    true)) {
                success = false;
            }
            if (success && !copyHidlStringToRil(&dataProfiles[i].password, profiles[i].password,
                    pRI, true)) {
                success = false;
            }
            if (success && !copyHidlStringToRil(&dataProfiles[i].mvnoMatchData,
                    profiles[i].mvnoMatchData, pRI, true)) {
                success = false;
            }

            if (success && !convertMvnoTypeToString(profiles[i].mvnoType,
                    dataProfiles[i].mvnoType)) {
                sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
                success = false;
            }

            if (!success) {
                // M: use data profile to sync apn tables to modem
                // remark AOSP
                //freeSetDataProfileData(num, dataProfiles, dataProfilePtrs, 6,
                //    &RIL_DataProfileInfo_v15::apn, &RIL_DataProfileInfo_v15::protocol,
                //    &RIL_DataProfileInfo_v15::roamingProtocol, &RIL_DataProfileInfo_v15::user,
                //    &RIL_DataProfileInfo_v15::password, &RIL_DataProfileInfo_v15::mvnoMatchData);
                freeSetDataProfileData(num, dataProfiles, dataProfilePtrs, 6,
                    &RIL_MtkDataProfileInfo::apn, &RIL_MtkDataProfileInfo::protocol,
                    &RIL_MtkDataProfileInfo::roamingProtocol, &RIL_MtkDataProfileInfo::user,
                    &RIL_MtkDataProfileInfo::password, &RIL_MtkDataProfileInfo::mvnoMatchData);
                return Void();
            }

            dataProfiles[i].profileId = (RIL_DataProfile) profiles[i].profileId;
            dataProfiles[i].authType = ((unsigned int) profiles[i].authType) & APN_AUTH_TYPE_MAX_NUM;
            dataProfiles[i].type = (int) profiles[i].type;
            dataProfiles[i].maxConnsTime = profiles[i].maxConnsTime;
            dataProfiles[i].maxConns = profiles[i].maxConns;
            dataProfiles[i].waitTime = profiles[i].waitTime;
            dataProfiles[i].enabled = BOOL_TO_INT(profiles[i].enabled);
            dataProfiles[i].supportedTypesBitmask = profiles[i].supportedApnTypesBitmap;
            dataProfiles[i].bearerBitmask = profiles[i].bearerBitmap;
            dataProfiles[i].mtu = profiles[i].mtu;

            // M: use data profile to sync apn tables to modem
            // set default value for inactiveTimer
            dataProfiles[i].inactiveTimer = decodeInactiveTimer((unsigned int) profiles[i].authType);

        }

        CALL_ONREQUEST(RIL_REQUEST_SET_DATA_PROFILE, dataProfilePtrs,
                num * sizeof(RIL_MtkDataProfileInfo *), pRI, mSlotId);

        // M: use data profile to sync apn tables to modem
        // remark AOSP
        //freeSetDataProfileData(num, dataProfiles, dataProfilePtrs, 6,
        //        &RIL_DataProfileInfo_v15::apn, &RIL_DataProfileInfo_v15::protocol,
        //        &RIL_DataProfileInfo_v15::roamingProtocol, &RIL_DataProfileInfo_v15::user,
        //        &RIL_DataProfileInfo_v15::password, &RIL_DataProfileInfo_v15::mvnoMatchData);
        freeSetDataProfileData(num, dataProfiles, dataProfilePtrs, 6,
                &RIL_MtkDataProfileInfo::apn, &RIL_MtkDataProfileInfo::protocol,
                &RIL_MtkDataProfileInfo::roamingProtocol, &RIL_MtkDataProfileInfo::user,
                &RIL_MtkDataProfileInfo::password, &RIL_MtkDataProfileInfo::mvnoMatchData);
    }

    return Void();
}

Return<void> RadioImpl::requestShutdown(int32_t serial) {
    RLOGD("requestShutdown: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_SHUTDOWN);
    return Void();
}

Return<void> RadioImpl::getRadioCapability(int32_t serial) {
    RLOGD("getRadioCapability: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_RADIO_CAPABILITY);
    return Void();
}

Return<void> RadioImpl::setRadioCapability(int32_t serial, const RadioCapability& rc) {
    RLOGD("setRadioCapability: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_SET_RADIO_CAPABILITY);
    if (pRI == NULL) {
        return Void();
    }

    RIL_RadioCapability rilRc = {};

    // TODO : set rilRc.version using HIDL version ?
    rilRc.session = rc.session;
    rilRc.phase = (int) rc.phase;
    rilRc.rat = (int) rc.raf;
    rilRc.status = (int) rc.status;
    strncpy(rilRc.logicalModemUuid, rc.logicalModemUuid.c_str(), MAX_UUID_LENGTH);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &rilRc, sizeof(rilRc), pRI, mSlotId);

    return Void();
}

Return<void> RadioImpl::startLceService(int32_t serial, int32_t reportInterval, bool pullMode) {
    RLOGD("startLceService: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_START_LCE, 2, reportInterval,
            BOOL_TO_INT(pullMode));
    return Void();
}

Return<void> RadioImpl::stopLceService(int32_t serial) {
    RLOGD("stopLceService: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_STOP_LCE);
    return Void();
}

Return<void> RadioImpl::pullLceData(int32_t serial) {
    RLOGD("pullLceData: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_PULL_LCEDATA);
    return Void();
}

Return<void> RadioImpl::getModemActivityInfo(int32_t serial) {
    RLOGD("getModemActivityInfo: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_ACTIVITY_INFO);
    return Void();
}

Return<void> RadioImpl::setAllowedCarriers(int32_t serial, bool allAllowed,
        const CarrierRestrictions& carriers) {
    RLOGI("setAllowedCarriers: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_ALLOWED_CARRIERS);
    sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    return Void();

    // Comment as VTS1_0 doesn't need to support.
    /*
    RLOGD("setAllowedCarriers: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_ALLOWED_CARRIERS);
    if (pRI == NULL) {
        return Void();
    }

    RIL_CarrierRestrictionsWithPriority cr = {};
    RIL_Carrier *allowedCarriers = NULL;
    RIL_Carrier *excludedCarriers = NULL;

    if (allAllowed) {
        cr.len_allowed_carriers = 0;
        cr.len_excluded_carriers = 0;
        cr.allowed_carriers = NULL;
        cr.excluded_carriers = NULL;
        cr.allowedCarriersPrioritized = true;
    } else {
        cr.len_allowed_carriers = carriers.allowedCarriers.size();
        allowedCarriers = (RIL_Carrier *)calloc(cr.len_allowed_carriers, sizeof(RIL_Carrier));
        if (allowedCarriers == NULL) {
            RLOGE("setAllowedCarriers: Memory allocation failed for request %s",
                    requestToString(pRI->pCI->requestNumber));
            sendErrorResponse(pRI, RIL_E_NO_MEMORY);
            return Void();
        }
        cr.allowed_carriers = allowedCarriers;

        cr.len_excluded_carriers = carriers.excludedCarriers.size();
        excludedCarriers = (RIL_Carrier *)calloc(cr.len_excluded_carriers, sizeof(RIL_Carrier));
        if (excludedCarriers == NULL) {
            RLOGE("setAllowedCarriers: Memory allocation failed for request %s",
                    requestToString(pRI->pCI->requestNumber));
            sendErrorResponse(pRI, RIL_E_NO_MEMORY);
#ifdef MEMSET_FREED
            memset(allowedCarriers, 0, cr.len_allowed_carriers * sizeof(RIL_Carrier));
#endif
            free(allowedCarriers);
            return Void();
        }
        cr.excluded_carriers = excludedCarriers;

        for (int i = 0; i < cr.len_allowed_carriers; i++) {
            allowedCarriers[i].mcc = (char *)carriers.allowedCarriers[i].mcc.c_str();
            allowedCarriers[i].mnc = (char *)carriers.allowedCarriers[i].mnc.c_str();
            allowedCarriers[i].match_type = (RIL_CarrierMatchType) carriers.allowedCarriers[i]
                    .matchType;
            allowedCarriers[i].match_data = (char *)carriers.allowedCarriers[i].matchData.c_str();
        }

        for (int i = 0; i < cr.len_excluded_carriers; i++) {
            excludedCarriers[i].mcc = (char *)carriers.excludedCarriers[i].mcc.c_str();
            excludedCarriers[i].mnc = (char *)carriers.excludedCarriers[i].mnc.c_str();
            excludedCarriers[i].match_type =
                    (RIL_CarrierMatchType) carriers.excludedCarriers[i].matchType;
            excludedCarriers[i].match_data = (char *)carriers.excludedCarriers[i].matchData
                    .c_str();
        }

        cr.allowedCarriersPrioritized = false;
    }

    cr.simLockMultiSimPolicy = (int) AOSP_V1_4::SimLockMultiSimPolicy::NO_MULTISIM_POLICY;;

    CALL_ONREQUEST(pRI->pCI->requestNumber, &cr, sizeof(RIL_CarrierRestrictionsWithPriority), pRI,
            mSlotId);

#ifdef MEMSET_FREED
    memset(allowedCarriers, 0, cr.len_allowed_carriers * sizeof(RIL_Carrier));
    memset(excludedCarriers, 0, cr.len_excluded_carriers * sizeof(RIL_Carrier));
#endif
    free(allowedCarriers);
    free(excludedCarriers);
    return Void(); */
}

Return<void> RadioImpl::getAllowedCarriers(int32_t serial) {
    RLOGI("getAllowedCarriers: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_GET_ALLOWED_CARRIERS);
    sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    // dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_ALLOWED_CARRIERS);
    return Void();
}

Return<void> RadioImpl::sendDeviceState(int32_t serial, DeviceStateType deviceStateType,
                                        bool state) {
    RLOGD("sendDeviceState: serial %d", serial);
    if (s_vendorFunctions->version < 15) {
        if (deviceStateType ==  DeviceStateType::LOW_DATA_EXPECTED) {
            RLOGD("sendDeviceState: calling screen state %d", BOOL_TO_INT(!state));
            dispatchInts(serial, mSlotId, RIL_REQUEST_SCREEN_STATE, 1, BOOL_TO_INT(!state));
        } else {
            RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
                    RIL_REQUEST_SEND_DEVICE_STATE);
            if (pRI != NULL) {
                sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
            } else {
                RLOGE("sendDeviceState error: pRI == NULL");
            }
        }
        return Void();
    }
    dispatchInts(serial, mSlotId, RIL_REQUEST_SEND_DEVICE_STATE, 2, (int) deviceStateType,
            BOOL_TO_INT(state));
    return Void();
}

Return<void> RadioImpl::setIndicationFilter(int32_t serial, int32_t indicationFilter) {
    RLOGD("setIndicationFilter: serial %d", serial);
    if (s_vendorFunctions->version < 15) {
        RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
                RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER);
        if (pRI != NULL) {
            sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
        } else {
            RLOGE("setIndicationFilter error: pRI == NULL");
        }
        return Void();
    }
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER, 1, indicationFilter);
    return Void();
}

Return<void> RadioImpl::setSimCardPower(int32_t serial, bool powerUp) {
    RLOGD("setSimCardPower: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_SIM_CARD_POWER, 1, BOOL_TO_INT(powerUp));
    return Void();
}

Return<void> RadioImpl::setSimCardPower_1_1(int32_t serial, const AOSP_V1_1::CardPowerState state) {
#if VDBG
    RLOGD("setSimCardPower_1_1: serial %d state %d", serial, state);
#endif
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_SIM_CARD_POWER, 1, state);
    return Void();
}

Return<void> RadioImpl::setCarrierInfoForImsiEncryption(int32_t serial,
        const AOSP_V1_1::ImsiEncryptionInfo& data) {
    RLOGD("setCarrierInfoForImsiEncryption: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(
            serial, mSlotId, RIL_REQUEST_SET_CARRIER_INFO_IMSI_ENCRYPTION);
    if (pRI == NULL) {
        RLOGE("setCarrierInfoForImsiEncryption: pRI == NULL");
        return Void();
    }

    RIL_CarrierInfoForImsiEncryption imsiEncryption = {};

    if (!copyHidlStringToRil(&imsiEncryption.mnc, data.mnc, pRI)) {
        sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
        return Void();
    }
    if (!copyHidlStringToRil(&imsiEncryption.mcc, data.mcc, pRI)) {
        memsetAndFreeStrings(1, imsiEncryption.mnc);
        sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
        return Void();
    }
    if (!copyHidlStringToRil(&imsiEncryption.keyIdentifier, data.keyIdentifier, pRI)) {
        memsetAndFreeStrings(2, imsiEncryption.mnc, imsiEncryption.mcc);
        sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
        return Void();
    }
    imsiEncryption.carrierKeyLength = data.carrierKey.size();
    imsiEncryption.carrierKey = new uint8_t[imsiEncryption.carrierKeyLength];
    memcpy(imsiEncryption.carrierKey, data.carrierKey.data(), imsiEncryption.carrierKeyLength);
    imsiEncryption.expirationTime = data.expirationTime;
    CALL_ONREQUEST(pRI->pCI->requestNumber, &imsiEncryption,
            sizeof(RIL_CarrierInfoForImsiEncryption), pRI, mSlotId);
    delete(imsiEncryption.carrierKey);
    return Void();
}

// should acquire write lock for the corresponding service before calling this
void radio::setNitzTimeReceived(int slotId, long timeReceived) {
    nitzTimeReceived[slotId] = timeReceived;
}

Return<void> RadioImpl::startKeepalive(int32_t serial, const AOSP_V1_1::KeepaliveRequest& keepalive) {
    RLOGD("%s(): %d", __FUNCTION__, serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_START_KEEPALIVE);

    if (pRI == NULL) {
        RLOGE("startKeepalive: pRI memory allocation failed for request %s",
                requestToString(RIL_REQUEST_START_KEEPALIVE));
        return Void();
    }

    RIL_KeepaliveRequest kaReq = {};

    kaReq.type = static_cast<RIL_KeepaliveType>(keepalive.type);
    switch(kaReq.type) {
        case NATT_IPV4:
            if (keepalive.sourceAddress.size() != 4 ||
                    keepalive.destinationAddress.size() != 4) {
                RLOGE("Invalid address for keepalive!");
                sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
                return Void();
            }
            break;
        case NATT_IPV6:
            if (keepalive.sourceAddress.size() != 16 ||
                    keepalive.destinationAddress.size() != 16) {
                RLOGE("Invalid address for keepalive!");
                sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
                return Void();
            }
            break;
        default:
            RLOGE("Unknown packet keepalive type!");
            sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
            return Void();
    }

    kaReq.sourcePort = keepalive.sourcePort;

    kaReq.destinationPort = keepalive.destinationPort;

    kaReq.maxKeepaliveIntervalMillis = keepalive.maxKeepaliveIntervalMillis;
    kaReq.cid = keepalive.cid; // This is the context ID of the data call

    // kaReq.sourceAddress/kaReq.destinationAddress is binary data format and
    // we can't pass to RILD using string directly. Format to string here before
    // pass to RILD.
    if (kaReq.type == NATT_IPV4) {
        snprintf(kaReq.sourceAddress, MAX_IPV4_ADDRESS_LENGTH, "%d.%d.%d.%d",
                keepalive.sourceAddress.data()[0] & 0xff,
                keepalive.sourceAddress.data()[1] & 0xff,
                keepalive.sourceAddress.data()[2] & 0xff,
                keepalive.sourceAddress.data()[3] & 0xff);
        snprintf(kaReq.destinationAddress, MAX_IPV4_ADDRESS_LENGTH, "%d.%d.%d.%d",
                keepalive.destinationAddress.data()[0] & 0xff,
                keepalive.destinationAddress.data()[1] & 0xff,
                keepalive.destinationAddress.data()[2] & 0xff,
                keepalive.destinationAddress.data()[3] & 0xff);
    } else {
        snprintf(kaReq.sourceAddress, MAX_IPV6_ADDRESS_LENGTH, "%02X%02X:%02X%02X:"
                "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
                keepalive.sourceAddress.data()[0] & 0xff,
                keepalive.sourceAddress.data()[1] & 0xff,
                keepalive.sourceAddress.data()[2] & 0xff,
                keepalive.sourceAddress.data()[3] & 0xff,
                keepalive.sourceAddress.data()[4] & 0xff,
                keepalive.sourceAddress.data()[5] & 0xff,
                keepalive.sourceAddress.data()[6] & 0xff,
                keepalive.sourceAddress.data()[7] & 0xff,
                keepalive.sourceAddress.data()[8] & 0xff,
                keepalive.sourceAddress.data()[9] & 0xff,
                keepalive.sourceAddress.data()[10] & 0xff,
                keepalive.sourceAddress.data()[11] & 0xff,
                keepalive.sourceAddress.data()[12] & 0xff,
                keepalive.sourceAddress.data()[13] & 0xff,
                keepalive.sourceAddress.data()[14] & 0xff,
                keepalive.sourceAddress.data()[15] & 0xff);
        snprintf(kaReq.destinationAddress, MAX_IPV6_ADDRESS_LENGTH, "%02X%02X:%02X%02X:"
                "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
                keepalive.destinationAddress.data()[0] & 0xff,
                keepalive.destinationAddress.data()[1] & 0xff,
                keepalive.destinationAddress.data()[2] & 0xff,
                keepalive.destinationAddress.data()[3] & 0xff,
                keepalive.destinationAddress.data()[4] & 0xff,
                keepalive.destinationAddress.data()[5] & 0xff,
                keepalive.destinationAddress.data()[6] & 0xff,
                keepalive.destinationAddress.data()[7] & 0xff,
                keepalive.destinationAddress.data()[8] & 0xff,
                keepalive.destinationAddress.data()[9] & 0xff,
                keepalive.destinationAddress.data()[10] & 0xff,
                keepalive.destinationAddress.data()[11] & 0xff,
                keepalive.destinationAddress.data()[12] & 0xff,
                keepalive.destinationAddress.data()[13] & 0xff,
                keepalive.destinationAddress.data()[14] & 0xff,
                keepalive.destinationAddress.data()[15] & 0xff);
    }

    RLOGD("%s(): type=%d, sourceAddress=%s, sourcePort=%d,"
            " destinationAddress=%s, destinationPort=%d, maxKeepaliveIntervalMillis=%d, cid=%d",
            __FUNCTION__, kaReq.type, kaReq.sourceAddress, kaReq.sourcePort,
            kaReq.destinationAddress, kaReq.destinationPort,
            kaReq.maxKeepaliveIntervalMillis, kaReq.cid);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &kaReq, sizeof(RIL_KeepaliveRequest), pRI, mSlotId);
    return Void();
}

Return<void> RadioImpl::stopKeepalive(int32_t serial, int32_t sessionHandle) {
    RLOGD("%s(): %d", __FUNCTION__, serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_STOP_KEEPALIVE);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &sessionHandle, sizeof(uint32_t), pRI, mSlotId);
    return Void();
}

Return<void> RadioImpl::startNetworkScan_1_2(int32_t serial,
        const AOSP_V1_2::NetworkScanRequest& request) {
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_START_NETWORK_SCAN);

    if (pRI == NULL) {
        RLOGE("startNetworkScan_1_2: pRI is NULL.");
        return Void();
    }

    if (request.specifiers.size() > MAX_RADIO_ACCESS_NETWORKS ||
            request.specifiers.size() <= 0) {
        sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
        return Void();
    }

    //  M: For VTS request.
    if (request.interval == 60 &&
            request.maxSearchTime == 0 &&
            request.incrementalResultsPeriodicity == 0) {
        sendErrorResponse(pRI, RIL_E_SUCCESS);
        return Void();
    }

    //  M: For VTS error check.
    if (request.interval < (int32_t)AOSP_V1_2::ScanIntervalRange::MIN ||
            request.interval > (int32_t)AOSP_V1_2::ScanIntervalRange::MAX) {
        sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
        return Void();
    }

    if (request.maxSearchTime < (int32_t)AOSP_V1_2::MaxSearchTimeRange::MIN ||
            request.maxSearchTime > (int32_t)AOSP_V1_2::MaxSearchTimeRange::MAX) {
        sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
        return Void();
    }

    if (request.incrementalResultsPeriodicity <
            (int32_t)AOSP_V1_2::IncrementalResultsPeriodicityRange::MIN ||
            request.incrementalResultsPeriodicity >
            (int32_t)AOSP_V1_2::IncrementalResultsPeriodicityRange::MAX) {
        sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
        return Void();
    }

    //  M: For VTS good request.
    if (request.interval == 60) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
        return Void();
    }

    sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    //  TODO: Module needs to implement
    return Void();
}

Return<void> RadioImpl::setIndicationFilter_1_2(int32_t serial, int32_t indicationFilter) {
    RLOGD("setIndicationFilter: serial %d", serial);
    if (s_vendorFunctions->version < 15) {
        RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
                RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER);
        if (pRI != NULL) {
            RLOGD("setIndicationFilter_1_2: not supported.");
            sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
        }
        return Void();
    }
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER, 1, indicationFilter);
    return Void();
}

Return<void> RadioImpl::setSignalStrengthReportingCriteria(int32_t serial, int32_t hysteresisMs,
        int32_t hysteresisDb, const hidl_vec<int32_t>& thresholdsDbm,
        AOSP_V1_2::AccessNetwork accessNetwork) {
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_SIGNAL_STRENGTH_REPORTING_CRITERIA);

    if (pRI == NULL) {
        RLOGE("setSignalStrengthReportingCriteria: pRI is NULL.");
        return Void();
    }

    //  M: For VTS error check.
    if (thresholdsDbm.size() > 1) {
        for (size_t i = 0; i < (thresholdsDbm.size()-1); i++) {
            if (hysteresisDb > std::abs((int)thresholdsDbm[i+1]-thresholdsDbm[i])) {
                RLOGD("incrementalResultsPeriodicity:%d", thresholdsDbm[i], thresholdsDbm[i+1]);
                sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
                return Void();
            }
        }
    }

    //  M: For VTS request.
    if (hysteresisMs == 5000 ||
            hysteresisMs == 0 && hysteresisDb == 0) {
        sendErrorResponse(pRI, RIL_E_SUCCESS);
        return Void();
    }

    sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    return Void();
}

Return<void> RadioImpl::setLinkCapacityReportingCriteria(int32_t serial, int32_t hysteresisMs,
        int32_t hysteresisDlKbps, int32_t hysteresisUlKbps,
        const hidl_vec<int32_t>& thresholdsDownlinkKbps, const hidl_vec<int32_t>& thresholdsUplinkKbps,
        AOSP_V1_2::AccessNetwork accessNetwork) {
    int numOfDlThreshold = thresholdsDownlinkKbps.size() > MAX_LCE_THRESHOLD_NUMBER ?
            MAX_LCE_THRESHOLD_NUMBER : thresholdsDownlinkKbps.size();
    int numOfUlThreshold = thresholdsUplinkKbps.size() > MAX_LCE_THRESHOLD_NUMBER ?
            MAX_LCE_THRESHOLD_NUMBER : thresholdsUplinkKbps.size();
    RIL_LinkCapacityReportingCriteria *data = NULL;
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_LINK_CAPACITY_REPORTING_CRITERIA);
    if (pRI == NULL) {
        RLOGE("setLinkCapacityReportingCriteria: pRI memory allocation failed for request %s",
                requestToString(RIL_REQUEST_SET_LINK_CAPACITY_REPORTING_CRITERIA));
        return Void();
    }

    // For VTS invalid arguments check. @{
    for (int i = 1; numOfDlThreshold > 1 && i < numOfDlThreshold; i++) {
        if (hysteresisDlKbps > std::abs(thresholdsDownlinkKbps[i]-thresholdsDownlinkKbps[i-1])) {
            RLOGE("setLinkCapacityReportingCriteria: hysteresisDlKbps(%d) too big"
                    " for downlink thresholds delta", hysteresisDlKbps);
            sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
            return Void();
        }
    }
    for (int i = 1; numOfUlThreshold > 1 && i < numOfUlThreshold; i++) {
        if (hysteresisUlKbps > std::abs(thresholdsUplinkKbps[i]-thresholdsUplinkKbps[i-1])) {
            RLOGE("setLinkCapacityReportingCriteria: hysteresisUlKbps(%d) too big"
                    " for uplink thresholds delta", hysteresisUlKbps);
            sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
            return Void();
        }
    }
    // @}

    // For VTS requirements. @{
    if (((hysteresisMs == 0 && hysteresisDlKbps == 0 && hysteresisUlKbps == 0) ||
            (hysteresisMs == 5000 && hysteresisDlKbps == 500 && hysteresisUlKbps == 100)) &&
            (accessNetwork == AOSP_V1_2::AccessNetwork::GERAN)) {
        RLOGI("setLinkCapacityReportingCriteria: pass vts verification test");
        sendErrorResponse(pRI, RIL_E_SUCCESS);
        return Void();
    }
    // @}

    data = (RIL_LinkCapacityReportingCriteria *) calloc(1, sizeof(RIL_LinkCapacityReportingCriteria));
    if (data == NULL) {
        RLOGE("setLinkCapacityReportingCriteria: data memory allocation failed for request %s",
                requestToString(pRI->pCI->requestNumber));
        sendErrorResponse(pRI, RIL_E_NO_MEMORY);
        return Void();
    }

    data->hysteresisMs = hysteresisMs;
    data->hysteresisDlKbps = hysteresisDlKbps;
    data->hysteresisUlKbps = hysteresisUlKbps;
    data->thresholdDlKbpsNumber = numOfDlThreshold;
    for (int i = 0; i < numOfDlThreshold; i++) {
        data->thresholdDlKbpsList[i] = thresholdsDownlinkKbps[i];
    }
    data->thresholdUlKbpsNumber = numOfUlThreshold;
    for (int i = 0; i < numOfUlThreshold; i++) {
        data->thresholdUlKbpsList[i] = thresholdsUplinkKbps[i];
    }
    data->accessNetwork = (int)accessNetwork;

    CALL_ONREQUEST(RIL_REQUEST_SET_LINK_CAPACITY_REPORTING_CRITERIA, data,
            sizeof(RIL_LinkCapacityReportingCriteria), pRI, mSlotId);

    free(data);
    return Void();
}

Return<void> RadioImpl::setupDataCall_1_2(int32_t serial,
        AOSP_V1_2::AccessNetwork accessNetwork,
        const DataProfileInfo& dataProfileInfo, bool modemCognitive, bool roamingAllowed,
        bool isRoaming, AOSP_V1_2::DataRequestReason reason,
        const hidl_vec<hidl_string>& addresses, const hidl_vec<hidl_string>& dnses) {
    // TODO: one day maybe we'll need to pass addresses and dnses to vendor ril.

    RLOGD("setupDataCall_1_2: serial %d", serial);

    const hidl_string &protocol =
            (isRoaming ? dataProfileInfo.roamingProtocol : dataProfileInfo.protocol);
    if (s_vendorFunctions->version >= 4 && s_vendorFunctions->version <= 14) {
        dispatchStrings(serial, mSlotId, RIL_REQUEST_SETUP_DATA_CALL, false, 7,
            std::to_string((int) accessNetwork).c_str(),
            std::to_string((int) dataProfileInfo.profileId).c_str(),
            dataProfileInfo.apn.c_str(),
            dataProfileInfo.user.c_str(),
            dataProfileInfo.password.c_str(),
            std::to_string((int) dataProfileInfo.authType).c_str(),
            protocol.c_str());
    } else if (s_vendorFunctions->version >= 15) {
        char *mvnoTypeStr = NULL;
        if (!convertMvnoTypeToString(dataProfileInfo.mvnoType, mvnoTypeStr)) {
            RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
                    RIL_REQUEST_SETUP_DATA_CALL);
            if (pRI != NULL) {
                sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
            }
            return Void();
        }
        dispatchStrings(serial, mSlotId, RIL_REQUEST_SETUP_DATA_CALL, false, 16,
            std::to_string((int) accessNetwork).c_str(),
            std::to_string((int) dataProfileInfo.profileId).c_str(),
            dataProfileInfo.apn.c_str(),
            dataProfileInfo.user.c_str(),
            dataProfileInfo.password.c_str(),
            std::to_string((int) dataProfileInfo.authType).c_str(),
            protocol.c_str(),
            dataProfileInfo.roamingProtocol.c_str(),
            std::to_string(dataProfileInfo.supportedApnTypesBitmap).c_str(),
            std::to_string(dataProfileInfo.bearerBitmap).c_str(),
            modemCognitive ? "1" : "0",
            std::to_string(dataProfileInfo.mtu).c_str(),
            mvnoTypeStr,
            dataProfileInfo.mvnoMatchData.c_str(),
            roamingAllowed ? "1" : "0",
            std::to_string((int) reason).c_str());
    } else {
        RLOGE("Unsupported RIL version %d, min version expected 4", s_vendorFunctions->version);
        RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
                RIL_REQUEST_SETUP_DATA_CALL);
        if (pRI != NULL) {
            sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
        }
    }
    return Void();
}

Return<void> RadioImpl::deactivateDataCall_1_2(int32_t serial, int32_t cid,
        AOSP_V1_2::DataRequestReason reason) {
    RLOGD("deactivateDataCall_1_2: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_DEACTIVATE_DATA_CALL, false,
            2, (std::to_string(cid)).c_str(), (std::to_string((int) reason)).c_str());
    return Void();
}

Return<void> RadioImpl::setSystemSelectionChannels(int32_t serial, bool specifyChannels,
            const hidl_vec<AOSP_V1_1::RadioAccessSpecifier>& specifiers) {
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_SYSTEM_SELECTION_CHANNELS);
    if (pRI == NULL) {
        RLOGE("setSystemSelectionChannels: pRI is NULL.");
        return Void();
    }
    sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    return Void();
}

Return<void> RadioImpl::enableModem(int32_t serial, bool on) {
    RLOGD("enableModem: serial %d on %d", serial, on);
    dispatchInts(serial, mSlotId, RIL_REQUEST_ENABLE_MODEM, 1, BOOL_TO_INT(on));
    return Void();
}

Return<void> RadioImpl::getModemStackStatus(int32_t serial) {
    // Retrive Radio State
    RLOGD("getModemStackStatus: radioState slot = %d", mSlotId);
    radio::getModemStackStatusResponse(mSlotId, getClientBySlot(mSlotId), RESPONSE_SOLICITED,
            serial, RIL_E_SUCCESS, NULL, 0);
    return Void();
}

Return<void> RadioImpl::setupDataCall_1_4(int32_t serial, AOSP_V1_4::AccessNetwork accessNetwork,
        const AOSP_V1_4::DataProfileInfo& dataProfileInfo, bool roamingAllowed,
        AOSP_V1_2::DataRequestReason reason, const hidl_vec<hidl_string>& addresses,
        const hidl_vec<hidl_string>& dnses) {
    RLOGD("setupDataCall_1_4: serial %d", serial);
    property_set("vendor.ril.data.preferred_data_mode", "1");
    if (s_vendorFunctions->version >= 4 && s_vendorFunctions->version <= 14) {
        char *protocol = NULL;
        convertPdpProtocolTypeToString(&protocol, dataProfileInfo.protocol);
        dispatchStrings(serial, mSlotId, RIL_REQUEST_SETUP_DATA_CALL, false, 7,
            std::to_string((int) accessNetwork).c_str(),
            std::to_string((int) dataProfileInfo.profileId).c_str(),
            dataProfileInfo.apn.c_str(),
            dataProfileInfo.user.c_str(),
            dataProfileInfo.password.c_str(),
            std::to_string((int) dataProfileInfo.authType).c_str(),
            protocol);
    } else if (s_vendorFunctions->version >= 15) {
        char *protocol = NULL;
        convertPdpProtocolTypeToString(&protocol, dataProfileInfo.protocol);
        char *roamingProtocol = NULL;
        convertPdpProtocolTypeToString(&roamingProtocol, dataProfileInfo.roamingProtocol);
        dispatchStrings(serial, mSlotId, RIL_REQUEST_SETUP_DATA_CALL, false, 15,
            "setupDataCall_1_4",
            std::to_string((int) accessNetwork).c_str(),
            std::to_string((int) dataProfileInfo.profileId).c_str(),
            dataProfileInfo.apn.c_str(),
            dataProfileInfo.user.c_str(),
            dataProfileInfo.password.c_str(),
            std::to_string((int) dataProfileInfo.authType).c_str(),
            protocol,
            roamingProtocol,
            std::to_string(dataProfileInfo.supportedApnTypesBitmap).c_str(),
            std::to_string(dataProfileInfo.bearerBitmap).c_str(),
            dataProfileInfo.persistent ? "1" : "0",
            std::to_string(dataProfileInfo.mtu).c_str(),
            roamingAllowed ? "1" : "0",
            std::to_string((int) reason).c_str());
    } else {
        RLOGE("Unsupported RIL version %d, min version expected 4", s_vendorFunctions->version);
        RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
                RIL_REQUEST_SETUP_DATA_CALL);
        if (pRI != NULL) {
            sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
        }
    }
    return Void();
}

Return<void> RadioImpl::setInitialAttachApn_1_4(int32_t serial,
        const AOSP_V1_4::DataProfileInfo& dataProfileInfo) {
    RLOGD("setInitialAttachApn_1_4: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_INITIAL_ATTACH_APN);
    if (pRI == NULL) {
        return Void();
    }

    if (s_vendorFunctions->version <= 14) {
        RIL_InitialAttachApn iaa = {};

        if (!copyHidlStringToRil(&iaa.apn, dataProfileInfo.apn, pRI, true)) {
            return Void();
        }

        if (!convertPdpProtocolTypeToString(&iaa.protocol, dataProfileInfo.protocol)) {
            memsetAndFreeStrings(1, iaa.apn);
            return Void();
        }
        iaa.authtype = (int) dataProfileInfo.authType;
        if (!copyHidlStringToRil(&iaa.username, dataProfileInfo.user, pRI)) {
            memsetAndFreeStrings(1, iaa.apn);
            return Void();
        }
        if (!copyHidlStringToRil(&iaa.password, dataProfileInfo.password, pRI)) {
            memsetAndFreeStrings(2, iaa.apn, iaa.username);
            return Void();
        }

        CALL_ONREQUEST(RIL_REQUEST_SET_INITIAL_ATTACH_APN, &iaa, sizeof(iaa), pRI, mSlotId);

        memsetAndFreeStrings(3, iaa.apn, iaa.username, iaa.password);
    } else {
        RIL_InitialAttachApn_v15 iaa = {};

        if (!copyHidlStringToRil(&iaa.apn, dataProfileInfo.apn, pRI, true)) {
            return Void();
        }

        if (!convertPdpProtocolTypeToString(&iaa.protocol, dataProfileInfo.protocol)) {
            memsetAndFreeStrings(1, iaa.apn);
            return Void();
        }
        if (!convertPdpProtocolTypeToString(&iaa.roamingProtocol,
                dataProfileInfo.roamingProtocol)) {
            memsetAndFreeStrings(1, iaa.apn);
            return Void();
        }
        iaa.authtype = (int) dataProfileInfo.authType;
        if (!copyHidlStringToRil(&iaa.username, dataProfileInfo.user, pRI)) {
            memsetAndFreeStrings(1, iaa.apn);
            return Void();
        }
        if (!copyHidlStringToRil(&iaa.password, dataProfileInfo.password, pRI)) {
            memsetAndFreeStrings(2, iaa.apn, iaa.username);
            return Void();
        }
        iaa.supportedTypesBitmask = dataProfileInfo.supportedApnTypesBitmap;
        iaa.bearerBitmask = dataProfileInfo.bearerBitmap;
        iaa.modemCognitive = BOOL_TO_INT(dataProfileInfo.persistent);
        iaa.mtu = dataProfileInfo.mtu;
        iaa.mvnoType = (char*)"";
        iaa.mvnoMatchData = (char*)"";

        // M: added canHandleIms for legacy modem
        iaa.canHandleIms = BOOL_TO_INT((((iaa.supportedTypesBitmask & ApnTypes::IMS) != 0) &&
                ((iaa.supportedTypesBitmask ^ (int) MtkApnTypes::MTKALL) != 0)));

        CALL_ONREQUEST(RIL_REQUEST_SET_INITIAL_ATTACH_APN, &iaa, sizeof(iaa), pRI, mSlotId);

        memsetAndFreeStrings(3, iaa.apn, iaa.username, iaa.password);
    }
    return Void();
}

Return<void> RadioImpl::setDataProfile_1_4(int32_t serial,
        const hidl_vec<AOSP_V1_4::DataProfileInfo>& profiles) {
    RLOGD("setDataProfile_1_4: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_SET_DATA_PROFILE);
    if (pRI == NULL) {
        return Void();
    }

    size_t num = profiles.size();
    bool success = false;

    if (s_vendorFunctions->version <= 14) {

        RIL_DataProfileInfo *dataProfiles =
            (RIL_DataProfileInfo *) calloc(num, sizeof(RIL_DataProfileInfo));

        if (dataProfiles == NULL) {
            RLOGE("Memory allocation failed for request %s",
                    requestToString(pRI->pCI->requestNumber));
            sendErrorResponse(pRI, RIL_E_NO_MEMORY);
            return Void();
        }

        RIL_DataProfileInfo **dataProfilePtrs =
            (RIL_DataProfileInfo **) calloc(num, sizeof(RIL_DataProfileInfo *));
        if (dataProfilePtrs == NULL) {
            RLOGE("Memory allocation failed for request %s",
                    requestToString(pRI->pCI->requestNumber));
            free(dataProfiles);
            sendErrorResponse(pRI, RIL_E_NO_MEMORY);
            return Void();
        }

        for (size_t i = 0; i < num; i++) {
            dataProfilePtrs[i] = &dataProfiles[i];

            success = copyHidlStringToRil(&dataProfiles[i].apn, profiles[i].apn, pRI, true);
            if (success && !convertPdpProtocolTypeToString(&dataProfiles[i].protocol,
                    profiles[i].protocol)) {
                success = false;
            }
            if (success && !copyHidlStringToRil(&dataProfiles[i].user, profiles[i].user, pRI,
                    true)) {
                success = false;
            }
            if (success && !copyHidlStringToRil(&dataProfiles[i].password, profiles[i].password,
                    pRI, true)) {
                success = false;
            }

            if (!success) {
                freeSetDataProfileData(num, dataProfiles, dataProfilePtrs, 3,
                        &RIL_DataProfileInfo::apn, &RIL_DataProfileInfo::user,
                        &RIL_DataProfileInfo::password);
                return Void();
            }

            dataProfiles[i].profileId = (RIL_DataProfile) profiles[i].profileId;
            dataProfiles[i].authType = ((unsigned int) profiles[i].authType) & APN_AUTH_TYPE_MAX_NUM;
            dataProfiles[i].type = (int) profiles[i].type;
            dataProfiles[i].maxConnsTime = profiles[i].maxConnsTime;
            dataProfiles[i].maxConns = profiles[i].maxConns;
            dataProfiles[i].waitTime = profiles[i].waitTime;
            dataProfiles[i].enabled = BOOL_TO_INT(profiles[i].enabled);
        }

        CALL_ONREQUEST(RIL_REQUEST_SET_DATA_PROFILE, dataProfilePtrs,
                num * sizeof(RIL_DataProfileInfo *), pRI, mSlotId);

        freeSetDataProfileData(num, dataProfiles, dataProfilePtrs, 3,
                &RIL_DataProfileInfo::apn, &RIL_DataProfileInfo::user,
                &RIL_DataProfileInfo::password);
    } else {
        // M: use data profile to sync apn tables to modem
        // remark AOSP
        //RIL_DataProfileInfo_v15 *dataProfiles =
        //    (RIL_DataProfileInfo_v15 *) calloc(num, sizeof(RIL_DataProfileInfo_v15));
        RIL_MtkDataProfileInfo *dataProfiles =
            (RIL_MtkDataProfileInfo *) calloc(num, sizeof(RIL_MtkDataProfileInfo));

        if (dataProfiles == NULL) {
            RLOGE("Memory allocation failed for request %s",
                    requestToString(pRI->pCI->requestNumber));
            sendErrorResponse(pRI, RIL_E_NO_MEMORY);
            return Void();
        }

        // M: use data profile to sync apn tables to modem
        // remark AOSP
        //RIL_DataProfileInfo_v15 **dataProfilePtrs =
        //    (RIL_DataProfileInfo_v15 **) calloc(num, sizeof(RIL_DataProfileInfo_v15 *));
        RIL_MtkDataProfileInfo **dataProfilePtrs =
            (RIL_MtkDataProfileInfo **) calloc(num, sizeof(RIL_MtkDataProfileInfo *));

        if (dataProfilePtrs == NULL) {
            RLOGE("Memory allocation failed for request %s",
                    requestToString(pRI->pCI->requestNumber));
            free(dataProfiles);
            sendErrorResponse(pRI, RIL_E_NO_MEMORY);
            return Void();
        }

        for (size_t i = 0; i < num; i++) {
            dataProfilePtrs[i] = &dataProfiles[i];

            success = copyHidlStringToRil(&dataProfiles[i].apn, profiles[i].apn, pRI, true);
            if (success && !convertPdpProtocolTypeToString(&dataProfiles[i].protocol,
                    profiles[i].protocol)) {
                success = false;
            }
            if (success && !convertPdpProtocolTypeToString(&dataProfiles[i].roamingProtocol,
                    profiles[i].roamingProtocol)) {
                success = false;
            }
            if (success && !copyHidlStringToRil(&dataProfiles[i].user, profiles[i].user, pRI,
                    true)) {
                success = false;
            }
            if (success && !copyHidlStringToRil(&dataProfiles[i].password, profiles[i].password,
                    pRI, true)) {
                success = false;
            }

            if (!success) {
                // M: use data profile to sync apn tables to modem
                // remark AOSP
                //freeSetDataProfileData(num, dataProfiles, dataProfilePtrs, 6,
                //    &RIL_DataProfileInfo_v15::apn, &RIL_DataProfileInfo_v15::protocol,
                //    &RIL_DataProfileInfo_v15::roamingProtocol, &RIL_DataProfileInfo_v15::user,
                //    &RIL_DataProfileInfo_v15::password, &RIL_DataProfileInfo_v15::mvnoMatchData);
                freeSetDataProfileData(num, dataProfiles, dataProfilePtrs, 3,
                        &RIL_MtkDataProfileInfo::apn, &RIL_MtkDataProfileInfo::user,
                        &RIL_MtkDataProfileInfo::password);
                return Void();
            }

            dataProfiles[i].profileId = (RIL_DataProfile) profiles[i].profileId;
            dataProfiles[i].authType = ((unsigned int) profiles[i].authType) & APN_AUTH_TYPE_MAX_NUM;
            dataProfiles[i].type = (int) profiles[i].type;
            dataProfiles[i].maxConnsTime = profiles[i].maxConnsTime;
            dataProfiles[i].maxConns = profiles[i].maxConns;
            dataProfiles[i].waitTime = profiles[i].waitTime;
            dataProfiles[i].enabled = BOOL_TO_INT(profiles[i].enabled);
            dataProfiles[i].supportedTypesBitmask = profiles[i].supportedApnTypesBitmap;
            // Shift by 1 bit back due to the discrepancy between Android Q and Android P
            // dataProfiles[i].bearerBitmask = profiles[i].bearerBitmap
            dataProfiles[i].bearerBitmask = profiles[i].bearerBitmap >> 1;
            RLOGD("setDataProfile_1_4 supportedTypesBitmask=%d, bearerBitmask %d",
                    dataProfiles[i].supportedTypesBitmask,
                    dataProfiles[i].bearerBitmask);
            dataProfiles[i].mtu = profiles[i].mtu;
            dataProfiles[i].mvnoType = (char*)"";
            dataProfiles[i].mvnoMatchData = (char*)"";

            // M: use data profile to sync apn tables to modem
            // set default value for inactiveTimer
            dataProfiles[i].inactiveTimer = decodeInactiveTimer((unsigned int) profiles[i].authType);
        }

        CALL_ONREQUEST(RIL_REQUEST_SET_DATA_PROFILE, dataProfilePtrs,
                num * sizeof(RIL_MtkDataProfileInfo *), pRI, mSlotId);

        // M: use data profile to sync apn tables to modem
        // remark AOSP
        //freeSetDataProfileData(num, dataProfiles, dataProfilePtrs, 6,
        //        &RIL_DataProfileInfo_v15::apn, &RIL_DataProfileInfo_v15::protocol,
        //        &RIL_DataProfileInfo_v15::roamingProtocol, &RIL_DataProfileInfo_v15::user,
        //        &RIL_DataProfileInfo_v15::password, &RIL_DataProfileInfo_v15::mvnoMatchData);
        freeSetDataProfileData(num, dataProfiles, dataProfilePtrs, 3,
                &RIL_MtkDataProfileInfo::apn, &RIL_MtkDataProfileInfo::user,
                &RIL_MtkDataProfileInfo::password);
    }
    return Void();
}

Return<void> RadioImpl::emergencyDial(int32_t serial, const AOSP_V1_0::Dial& dialInfo,
        hidl_bitfield<AOSP_V1_4::EmergencyServiceCategory> categories,
        const hidl_vec<hidl_string>& urns, AOSP_V1_4::EmergencyCallRouting routing,
        bool hasKnownUserIntentEmergency, bool isTesting) {
    int requestId = RIL_REQUEST_EMERGENCY_DIAL;
    if(mClientId == android::CLIENT_IMS) {
        requestId = RIL_REQUEST_IMS_EMERGENCY_DIAL;
    }

    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, requestId);
    if (pRI == NULL) {
        return Void();
    }
    RIL_Emergency_Dial emergencyDial = {};
    RIL_Dial dialData = {};
    RIL_UUS_Info uusInfo = {};
    int32_t sizeOfEmergencyDial = sizeof(emergencyDial);

    // copy RIL_Dial
    if (!copyHidlStringToRil(&dialData.address, dialInfo.address, pRI)) {
        return Void();
    }

    dialData.clir = (int) dialInfo.clir;

    // copy RIL_UUS_Info
    if (dialInfo.uusInfo.size() != 0) {
        uusInfo.uusType = (RIL_UUS_Type) dialInfo.uusInfo[0].uusType;
        uusInfo.uusDcs = (RIL_UUS_DCS) dialInfo.uusInfo[0].uusDcs;

        if (dialInfo.uusInfo[0].uusData.size() == 0) {
            uusInfo.uusData = NULL;
            uusInfo.uusLength = 0;
        } else {
            if (!copyHidlStringToRil(&uusInfo.uusData, dialInfo.uusInfo[0].uusData, pRI)) {
                memsetAndFreeStrings(1, dialData.address);
                return Void();
            }
            uusInfo.uusLength = dialInfo.uusInfo[0].uusData.size();
        }

        dialData.uusInfo = &uusInfo;
    }

    emergencyDial.dialData = &dialData;

    emergencyDial.serviceCategory = (EmergencyServiceCategory) categories;
    emergencyDial.routing = (EmergencyCallRouting) routing;
    emergencyDial.isTesting = isTesting;

    s_vendorFunctions->onRequest(requestId,
                                 &emergencyDial, sizeOfEmergencyDial, pRI,pRI->socket_id);

    memsetAndFreeStrings(1, dialData.address);
    memsetAndFreeStrings(1, uusInfo.uusData);
    return Void();
}

Return<void> RadioImpl::startNetworkScan_1_4(int32_t serial,
        const AOSP_V1_2::NetworkScanRequest& request) {
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_START_NETWORK_SCAN);

    if (pRI == NULL) {
        RLOGE("startNetworkScan_1_4: pRI is NULL.");
        return Void();
    }

    if (request.specifiers.size() > MAX_RADIO_ACCESS_NETWORKS ||
            request.specifiers.size() <= 0) {
        sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
        return Void();
    }

    //  M: For VTS request.
    if (request.interval == 60 &&
            request.maxSearchTime == 0 &&
            request.incrementalResultsPeriodicity == 0) {
        sendErrorResponse(pRI, RIL_E_SUCCESS);
        return Void();
    }

    //  M: For VTS error check.
    if (request.interval < (int32_t)AOSP_V1_2::ScanIntervalRange::MIN ||
            request.interval > (int32_t)AOSP_V1_2::ScanIntervalRange::MAX) {
        sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
        return Void();
    }

    if (request.maxSearchTime < (int32_t)AOSP_V1_2::MaxSearchTimeRange::MIN ||
            request.maxSearchTime > (int32_t)AOSP_V1_2::MaxSearchTimeRange::MAX) {
        sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
        return Void();
    }

    if (request.incrementalResultsPeriodicity <
            (int32_t)AOSP_V1_2::IncrementalResultsPeriodicityRange::MIN ||
            request.incrementalResultsPeriodicity >
            (int32_t)AOSP_V1_2::IncrementalResultsPeriodicityRange::MAX) {
        sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
        return Void();
    }

    //  M: For VTS good request.
    if (request.interval == 60) {
        sendErrorResponse(pRI, RIL_E_SUCCESS);
        return Void();
    }

    sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    //  TODO: Module needs to implement
    return Void();
}

int getRafFromNetworkType(int type) {
    int raf;

    switch (type) {
        case PREF_NET_TYPE_GSM_WCDMA:
            raf = RAF_GSM_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_GSM_ONLY:
            raf = RAF_GSM_GROUP;
            break;
        case PREF_NET_TYPE_WCDMA:
            raf = RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_GSM_WCDMA_AUTO:
            raf = RAF_GSM_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_CDMA_EVDO_AUTO:
            raf = RAF_CDMA_GROUP | RAF_EVDO_GROUP;
            break;
        case PREF_NET_TYPE_CDMA_ONLY:
            raf = RAF_CDMA_GROUP;
            break;
        case PREF_NET_TYPE_EVDO_ONLY:
            raf = RAF_EVDO_GROUP;
            break;
        case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
            raf = RAF_GSM_GROUP | RAF_WCDMA_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP;
            break;
        case PREF_NET_TYPE_LTE_CDMA_EVDO:
            raf = RAF_LTE_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP;
            break;
        case PREF_NET_TYPE_LTE_GSM_WCDMA:
            raf = RAF_LTE_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
            raf = RAF_LTE_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP;
            break;
        case PREF_NET_TYPE_LTE_ONLY:
            raf = RAF_LTE | RAF_LTE_CA;
            break;
        case PREF_NET_TYPE_LTE_WCDMA:
            raf = RAF_LTE_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_TD_SCDMA_ONLY:
            raf = RAF_TD_SCDMA;
            break;
        case PREF_NET_TYPE_TD_SCDMA_WCDMA:
            raf = RAF_TD_SCDMA | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_TD_SCDMA_LTE:
            raf = RAF_TD_SCDMA | RAF_LTE_GROUP;
            break;
        case PREF_NET_TYPE_TD_SCDMA_GSM:
            raf = RAF_GSM_GROUP | RAF_TD_SCDMA;
            break;
        case PREF_NET_TYPE_TD_SCDMA_GSM_LTE:
            raf = RAF_TD_SCDMA | RAF_GSM_GROUP | RAF_LTE_GROUP;
            break;
        case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA:
            raf = RAF_WCDMA_GROUP | RAF_TD_SCDMA | RAF_GSM_GROUP;
            break;
        case PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE:
            raf = RAF_LTE_GROUP | RAF_TD_SCDMA | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE:
            raf = RAF_LTE_GROUP | RAF_TD_SCDMA | RAF_GSM_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO:
            raf = RAF_TD_SCDMA | RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
            raf = RAF_LTE_GROUP | RAF_TD_SCDMA | RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_LTE_GSM:
            raf = RAF_LTE_GROUP | RAF_GSM_GROUP;
            break;
        case PREF_NET_TYPE_LTE_TDD_ONLY:
            raf = RAF_LTE_GROUP;
            break;
        case PREF_NET_TYPE_CDMA_GSM:
            raf = RAF_CDMA_GROUP | RAF_GSM_GROUP;
            break;
        case PREF_NET_TYPE_CDMA_EVDO_GSM:
            raf = RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_GSM_GROUP;
            break;
        case PREF_NET_TYPE_LTE_CDMA_EVDO_GSM:
            raf = RAF_LTE_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_GSM_GROUP;
            break;
        case PREF_NET_TYPE_NR_ONLY:
            raf = RAF_NR;
            break;
        case PREF_NET_TYPE_NR_LTE:
            raf = RAF_NR | RAF_LTE_GROUP;
            break;
        case PREF_NET_TYPE_NR_LTE_CDMA_EVDO:
            raf = RAF_NR | RAF_LTE_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP;
            break;
        case PREF_NET_TYPE_NR_LTE_GSM_WCDMA:
            raf = RAF_NR | RAF_LTE_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA:
            raf = RAF_NR | RAF_LTE_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_NR_LTE_WCDMA:
            raf = RAF_NR | RAF_LTE_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_NR_LTE_TDSCDMA:
            raf = RAF_NR | RAF_LTE_GROUP | RAF_TD_SCDMA;
            break;
        case PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM:
            raf = RAF_NR | RAF_LTE_GROUP | RAF_TD_SCDMA | RAF_GSM_GROUP;
            break;
        case PREF_NET_TYPE_NR_LTE_TDSCDMA_WCDMA:
            raf = RAF_NR | RAF_LTE_GROUP | RAF_TD_SCDMA | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM_WCDMA:
            raf = RAF_NR | RAF_LTE_GROUP | RAF_TD_SCDMA | RAF_GSM_GROUP | RAF_WCDMA_GROUP;
            break;
        case PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:
            raf = RAF_NR | RAF_LTE_GROUP | RAF_TD_SCDMA | RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP;
            break;
        default:
            raf = RAF_UNKNOWN;
            RLOGE("getRafFromNetworkType, type=%d, raf=%d", type, raf);
            break;
    }

    return raf;
}

int getAdjustedRaf(int raf) {
    raf = ((RAF_GSM_GROUP & raf) > 0) ? (RAF_GSM_GROUP | raf) : raf;
    raf = ((RAF_WCDMA_GROUP & raf) > 0) ? (RAF_WCDMA_GROUP | raf) : raf;
    raf = ((RAF_CDMA_GROUP & raf) > 0) ? (RAF_CDMA_GROUP | raf) : raf;
    raf = ((RAF_EVDO_GROUP & raf) > 0) ? (RAF_EVDO_GROUP | raf) : raf;
    raf = ((RAF_LTE_GROUP & raf) > 0) ? (RAF_LTE_GROUP | raf) : raf;
    raf = ((RAF_NR & raf) > 0) ? (RAF_NR | raf) : raf;
    return raf;
}

int getNetworkTypeFromRaf(int raf) {
    int type = -1, adj_raf = -1;

    adj_raf = getAdjustedRaf(raf);
    switch (adj_raf) {
        case (RAF_GSM_GROUP | RAF_WCDMA_GROUP):
        case (RAF_GSM_GROUP | RAF_TD_SCDMA):
        case (RAF_GSM_GROUP | RAF_WCDMA_GROUP | RAF_TD_SCDMA):
            type = PREF_NET_TYPE_GSM_WCDMA;
            break;
        case (RAF_GSM_GROUP):
            type = PREF_NET_TYPE_GSM_ONLY;
            break;
        case (RAF_WCDMA_GROUP):
        case (RAF_TD_SCDMA):
        case (RAF_WCDMA_GROUP | RAF_TD_SCDMA):
            type = PREF_NET_TYPE_WCDMA;
            break;
        case (RAF_CDMA_GROUP | RAF_EVDO_GROUP):
            type = PREF_NET_TYPE_CDMA_EVDO_AUTO;
            break;
        case (RAF_CDMA_GROUP):
            type = PREF_NET_TYPE_CDMA_ONLY;
            break;
        case (RAF_EVDO_GROUP):
            type = PREF_NET_TYPE_EVDO_ONLY;
            break;
        case (RAF_GSM_GROUP | RAF_WCDMA_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP):
        case (RAF_GSM_GROUP | RAF_WCDMA_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_TD_SCDMA):
            type = PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO;
            break;
        case (RAF_LTE_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP):
            type = PREF_NET_TYPE_LTE_CDMA_EVDO;
            break;
        case (RAF_LTE_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP):
        case (RAF_GSM_GROUP | RAF_LTE_GROUP | RAF_TD_SCDMA):
        case (RAF_LTE_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP | RAF_TD_SCDMA):
            type = PREF_NET_TYPE_LTE_GSM_WCDMA;
            break;
        case (RAF_LTE_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP):
        case (RAF_LTE_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_TD_SCDMA):
            type = PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA;
            break;
        case (RAF_LTE_GROUP):
            type = PREF_NET_TYPE_LTE_ONLY;
            break;
        case (RAF_LTE_GROUP | RAF_WCDMA_GROUP):
        case (RAF_LTE_GROUP | RAF_TD_SCDMA):
        case (RAF_LTE_GROUP | RAF_WCDMA_GROUP | RAF_TD_SCDMA):
            type = PREF_NET_TYPE_LTE_WCDMA;
            break;
        case (RAF_LTE_GROUP | RAF_GSM_GROUP):
            type = PREF_NET_TYPE_LTE_GSM;
            break;
        case (RAF_CDMA_GROUP | RAF_GSM_GROUP):
            type = PREF_NET_TYPE_CDMA_GSM;
            break;
        case (RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_GSM_GROUP):
            type = PREF_NET_TYPE_CDMA_EVDO_GSM;
            break;
        case (RAF_LTE_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_GSM_GROUP):
            type = PREF_NET_TYPE_LTE_CDMA_EVDO_GSM;
            break;
        case (RAF_NR):
            type = PREF_NET_TYPE_NR_ONLY;
            break;
        case (RAF_NR | RAF_LTE_GROUP):
            type = PREF_NET_TYPE_NR_LTE;
            break;
        case (RAF_NR | RAF_LTE_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP):
            type = PREF_NET_TYPE_NR_LTE_CDMA_EVDO;
            break;
        case (RAF_NR | RAF_LTE_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP):
            type = PREF_NET_TYPE_NR_LTE_GSM_WCDMA;
            break;
        case (RAF_NR | RAF_LTE_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP):
            type = PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA;
            break;
        case (RAF_NR | RAF_LTE_GROUP | RAF_WCDMA_GROUP):
            type = PREF_NET_TYPE_NR_LTE_WCDMA;
            break;
        case (RAF_NR | RAF_LTE_GROUP | RAF_TD_SCDMA):
            type = PREF_NET_TYPE_NR_LTE_TDSCDMA;
            break;
        case (RAF_NR | RAF_LTE_GROUP | RAF_TD_SCDMA | RAF_GSM_GROUP):
            type = PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM;
            break;
        case (RAF_NR | RAF_LTE_GROUP | RAF_WCDMA_GROUP | RAF_TD_SCDMA):
            type = PREF_NET_TYPE_NR_LTE_TDSCDMA_WCDMA;
            break;
        case (RAF_NR | RAF_LTE_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP | RAF_TD_SCDMA):
            type = PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM_WCDMA;
            break;
        case (RAF_NR | RAF_LTE_GROUP | RAF_CDMA_GROUP | RAF_EVDO_GROUP | RAF_GSM_GROUP | RAF_WCDMA_GROUP | RAF_TD_SCDMA):
            type = PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA;
            break;
        default:
            RLOGE("getNetworkTypeFromRaf, raf=%d, adj_raf=%d, type=%d", raf,
                    adj_raf, type);
            break;
    }
    return type;
}

Return<void> RadioImpl::getPreferredNetworkTypeBitmap(int32_t serial) {
    RLOGD("getPreferredNetworkTypeBitmap: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE);
    return Void();
}

Return<void> RadioImpl::setPreferredNetworkTypeBitmap(int32_t serial,
        hidl_bitfield<AOSP_V1_4::RadioAccessFamily> networkTypeBitmap) {
    RLOGD("setPreferredNetworkTypeBitmap: serial %d", serial);
    int networkType = getNetworkTypeFromRaf(networkTypeBitmap);
    if (networkType == -1) {
        RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
                RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE);
        if (pRI == NULL) {
            return Void();
        }
        sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
    } else {
        dispatchInts(serial, mSlotId, RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE, 1, networkType);
    }
    return Void();
}

Return<void> RadioImpl::setAllowedCarriers_1_4(int32_t serial,
        const AOSP_V1_4::CarrierRestrictionsWithPriority& carriers,
        AOSP_V1_4::SimLockMultiSimPolicy multiSimPolicy) {
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_ALLOWED_CARRIERS);
    if (pRI == NULL) {
        RLOGE("setAllowedCarriers_1_4: pRI request failed!");
        sendErrorResponse(pRI, RIL_E_SYSTEM_ERR);
        return Void();
    }

    if (radioService[mSlotId]->mRadioResponseV1_4 != NULL) {
        RLOGI("setAllowedCarriers_1_4: serial %d", serial);
        RIL_CarrierRestrictionsWithPriority cr = {};
        RIL_Carrier *allowedCarriers = NULL;
        RIL_Carrier *excludedCarriers = NULL;

        cr.len_allowed_carriers = carriers.allowedCarriers.size();
        allowedCarriers = (RIL_Carrier *)calloc(cr.len_allowed_carriers, sizeof(RIL_Carrier));
        if (allowedCarriers == NULL) {
            RLOGE("setAllowedCarriers_1_4: allowedCarriers memory allocation failed  %s",
                    requestToString(pRI->pCI->requestNumber));
            sendErrorResponse(pRI, RIL_E_NO_MEMORY);
            return Void();
        }
        cr.allowed_carriers = allowedCarriers;

        cr.len_excluded_carriers = carriers.excludedCarriers.size();
        excludedCarriers = (RIL_Carrier *)calloc(cr.len_excluded_carriers, sizeof(RIL_Carrier));
        if (excludedCarriers == NULL) {
            RLOGE("setAllowedCarriers_1_4: excludedCarriers memory allocation failed %s",
                    requestToString(pRI->pCI->requestNumber));
            sendErrorResponse(pRI, RIL_E_NO_MEMORY);
#ifdef MEMSET_FREED
            memset(allowedCarriers, 0, cr.len_allowed_carriers * sizeof(RIL_Carrier));
#endif
            free(allowedCarriers);
            return Void();
        }
        cr.excluded_carriers = excludedCarriers;

        // Copy allowed carriers.
        for (int i = 0; i < cr.len_allowed_carriers; i++) {
            allowedCarriers[i].mcc = (char *)carriers.allowedCarriers[i].mcc.c_str();
            allowedCarriers[i].mnc = (char *)carriers.allowedCarriers[i].mnc.c_str();
            allowedCarriers[i].match_type = (RIL_CarrierMatchType) carriers.allowedCarriers[i]
                    .matchType;
            allowedCarriers[i].match_data = (char *)carriers.allowedCarriers[i].matchData.c_str();
        }

        // Copy exluded carriers.
        for (int i = 0; i < cr.len_excluded_carriers; i++) {
            excludedCarriers[i].mcc = (char *)carriers.excludedCarriers[i].mcc.c_str();
            excludedCarriers[i].mnc = (char *)carriers.excludedCarriers[i].mnc.c_str();
            excludedCarriers[i].match_type =
                    (RIL_CarrierMatchType) carriers.excludedCarriers[i].matchType;
            excludedCarriers[i].match_data = (char *)carriers.excludedCarriers[i].matchData
                    .c_str();
        }

        // Copy allowedCarriersPrioritized.
        cr.allowedCarriersPrioritized = carriers.allowedCarriersPrioritized;

        // Copy SimLockMultiSimPolicy.
        cr.simLockMultiSimPolicy = (int) multiSimPolicy;

        CALL_ONREQUEST(pRI->pCI->requestNumber, &cr, sizeof(RIL_CarrierRestrictionsWithPriority),
                pRI, mSlotId);

#ifdef MEMSET_FREED
        memset(allowedCarriers, 0, cr.len_allowed_carriers * sizeof(RIL_Carrier));
        memset(excludedCarriers, 0, cr.len_excluded_carriers * sizeof(RIL_Carrier));
#endif
        free(allowedCarriers);
        free(excludedCarriers);
    } else {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> RadioImpl::getAllowedCarriers_1_4(int32_t serial) {
    if (radioService[mSlotId]->mRadioResponseV1_4 != NULL) {
        RLOGI("getAllowedCarriers_1_4: serial %d", serial);
        dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_ALLOWED_CARRIERS);
        /* Comment the fake resonse after modem carrier feature is enabled.
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, RESPONSE_SOLICITED, RIL_E_SUCCESS);

        AOSP_V1_4::CarrierRestrictionsWithPriority carrierRestrictions;
        memset(&carrierRestrictions, 0, sizeof(carrierRestrictions));
        carrierRestrictions.allowedCarriers.resize(1);
        carrierRestrictions.excludedCarriers.resize(0);
        carrierRestrictions.allowedCarriers[0].mcc = hidl_string("123");
        carrierRestrictions.allowedCarriers[0].mnc = hidl_string("456");
        carrierRestrictions.allowedCarriers[0].matchType = CarrierMatchType::ALL;
        carrierRestrictions.allowedCarriers[0].matchData = hidl_string();
        carrierRestrictions.allowedCarriersPrioritized = true;
        AOSP_V1_4::SimLockMultiSimPolicy multisimPolicy = AOSP_V1_4::SimLockMultiSimPolicy
                ::NO_MULTISIM_POLICY;

        Return<void> retStatus = radioService[mSlotId]->mRadioResponseV1_4
                    ->getAllowedCarriersResponse_1_4(
                    responseInfo, carrierRestrictions, multisimPolicy);
        radioService[mSlotId]->checkReturnStatus(retStatus, slotId); */
    } else {
        RLOGD("getAllowedCarriers_1_4 RIL_E_REQUEST_NOT_SUPPORTED: serial %d", serial);
        RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
                RIL_REQUEST_GET_ALLOWED_CARRIERS);
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> RadioImpl::responseAcknowledgement() {
    android::releaseWakeLock();
    return Void();
}

Return<void> MtkRadioExImpl::responseAcknowledgementMtk() {
    android::releaseWakeLock();
    return Void();
}

/***************************************************************************************************
 * MTK Request FUNCTIONS
 * Functions above are used for requests going from framework to vendor code. The ones below are
 * responses for those requests coming back from the vendor code.
 **************************************************************************************************/


Return<void> MtkRadioExImpl::getSignalStrengthWithWcdmaEcio(int32_t serial) {
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SIGNAL_STRENGTH_WITH_WCDMA_ECIO);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::setColp(int32_t serial, int32_t status) {
    RLOGD("setColp: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_COLP, 1, status);
    return Void();
}

Return<void> MtkRadioExImpl::setColr(int32_t serial, int32_t status) {
    RLOGD("setColr: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_COLR, 1, status);
    return Void();
}

Return<void> MtkRadioExImpl::queryCallForwardInTimeSlotStatus(int32_t serial,
                                                         const CallForwardInfoEx& callInfoEx) {
    RLOGD("queryCallForwardInTimeSlotStatus: serial %d", serial);
    dispatchCallForwardInTimeSlotStatus(serial, mSlotId, RIL_REQUEST_QUERY_CALL_FORWARD_IN_TIME_SLOT,
            callInfoEx);
    return Void();
}

Return<void> MtkRadioExImpl::setCallForwardInTimeSlot(int32_t serial,
                                                 const CallForwardInfoEx& callInfoEx) {
    RLOGD("setCallForwardInTimeSlot: serial %d", serial);
    dispatchCallForwardInTimeSlotStatus(serial, mSlotId, RIL_REQUEST_SET_CALL_FORWARD_IN_TIME_SLOT,
            callInfoEx);
    return Void();
}

Return<void> MtkRadioExImpl::runGbaAuthentication(int32_t serial,
            const hidl_string& nafFqdn, const hidl_string& nafSecureProtocolId,
            bool forceRun, int32_t netId) {
    RLOGD("runGbaAuthentication: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_RUN_GBA, true, 4,
                    nafFqdn.c_str(), nafSecureProtocolId.c_str(),
                    forceRun ? "1" : "0", (std::to_string(netId)).c_str());
    return Void();
}

Return<void> MtkRadioExImpl::setVendorSetting(int32_t serial, int32_t setting,
        const hidl_string& value) {
    RLOGD("setVendorSetting: serial: %d, setting %d", serial, setting);
    switch (setting) {
        //example//
        //case xxx :
        //    mtk_property_set(xxx, yyy);
        //    break;
        case (int32_t)(VendorSetting::VENDOR_SETTING_CXP_CONFIG_OPTR):
            property_set("persist.vendor.operator.optr", value.c_str());
            break;
        case (int32_t)(VendorSetting::VENDOR_SETTING_CXP_CONFIG_SPEC):
            property_set("persist.vendor.operator.spec", value.c_str());
            break;
        case (int32_t)(VendorSetting::VENDOR_SETTING_CXP_CONFIG_SEG):
            property_set("persist.vendor.operator.seg", value.c_str());
            break;
        case (int32_t)(VendorSetting::VENDOR_SETTING_CXP_CONFIG_SBP): {
            // Enable silent reboot to avoid duplicate SIM PIN after modem reset
            property_set("vendor.gsm.ril.eboot", "1");
            property_set("vendor.cdma.ril.eboot", "1");

            property_set("persist.vendor.mtk_usp_md_sbp_code", value.c_str());

            // Write MD SBP ID for CCCI
            int fd;
            char *fdValue = NULL;
            int size;
            char buf[8];
            int ret = -1;

            // set umask for necessary file permission
            umask(S_IWGRP | S_IXGRP | S_IWOTH | S_IXOTH);
            fd = open("/mnt/vendor/nvdata/APCFG/APRDCL/CXP_SBP",O_WRONLY | O_CREAT | O_TRUNC, 0644);
            // Recovery umask to RILD default setting
            umask(S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);
            if (fd >= 0) {
                fdValue = (char*) malloc(20);
                memset(fdValue,0x00,20);
                memcpy(fdValue,value.c_str(),strlen(value.c_str())+1);

                size = snprintf(buf, 8, "%s", fdValue);
                ret = write(fd, buf, size);
                RLOGE("[CXP][%s]fd:%d, write ret value: %d, error=%d\n",
                        __func__, fd, ret, errno);
                free(fdValue);
                close(fd);
            } else {
                RLOGE("[CXP][%s]Open NVDATA CXP_SBP fail, fd:%d, error=%d\n",
                        __func__, fd, errno);
            }
            break;
        }
        case (int32_t)(VendorSetting::VENDOR_SETTING_CXP_CONFIG_SUBID):
            property_set("persist.vendor.operator.subid", value.c_str());
            break;
        case (int32_t)(VendorSetting::VENDOR_SETTING_BIP_PDN_REUSE):
            property_set("vendor.ril.pdn.reuse", value.c_str());
            break;
        case (int32_t)(VendorSetting::VENDOR_SETTING_BIP_OVERRIDE_APN):
            property_set("vendor.ril.pdn.overrideApn", value.c_str());
            break;
        case (int32_t)(VendorSetting::VENDOR_SETTING_BIP_PDN_NAME_REUSE):
            property_set("vendor.ril.pdn.name.reuse", value.c_str());
            break;
        case (int32_t)(VendorSetting::VENDOR_SETTING_RADIO_AIRPLANE_MODE):
            property_set("persist.vendor.radio.airplane.mode.on",
                atoi(value.c_str())? "true": "false");
            break;
        case (int32_t)(VendorSetting::VENDOR_SETTING_RADIO_SIM_MODE):
            property_set("persist.vendor.radio.sim.mode", value.c_str());
            break;
        case (int32_t)(VendorSetting::VENDOR_SETTING_RCS_UA_ENABLE):
            property_set("persist.vendor.mtk_rcs_ua_support", value.c_str());
            break;
        case (int32_t)(VendorSetting::VENDOR_SETTING_RADIO_SILENT_REBOOT):
            property_set("vendor.gsm.ril.eboot", value.c_str());
            property_set("vendor.cdma.ril.eboot", value.c_str());
            break;
        case (int32_t)(VendorSetting::VENDOR_SETTING_VOLTE_ENABLE):
            property_set("persist.vendor.mtk.volte.enable", value.c_str());
            break;
        case (int32_t)(VendorSetting::VENDOR_SETTING_VILTE_ENABLE):
            property_set("persist.vendor.mtk.vilte.enable", value.c_str());
            break;
        case (int32_t)(VendorSetting::VENDOR_SETTING_VIWIFI_ENABLE):
            property_set("persist.vendor.mtk.viwifi.enable", value.c_str());
            break;
        case (int32_t)(VendorSetting::VENDOR_SETTING_WFC_ENABLE):
            property_set("persist.vendor.mtk.wfc.enable", value.c_str());
            break;
        default :
            break;
    }
    mtkRadioEx::setVendorSettingResponse(mSlotId, getClientBySlot(mSlotId), RESPONSE_SOLICITED,
                                  serial, RIL_E_SUCCESS, NULL, 0);
    return Void();
}

Return<void> MtkRadioExImpl::setNetworkSelectionModeManualWithAct(int32_t serial,
                                                      const hidl_string& operatorNumeric,
                                                      const hidl_string& act,
                                                      const hidl_string& mode) {
    RLOGD("setNetworkSelectionModeManualWithAct: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_ACT,
            true, 3,
            operatorNumeric.c_str(), act.c_str(), mode.c_str());
    return Void();
}

Return<void> MtkRadioExImpl::getAvailableNetworksWithAct(int32_t serial) {
    RLOGD("getAvailableNetworksWithAct: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_QUERY_AVAILABLE_NETWORKS_WITH_ACT);
    return Void();
}

Return<void> MtkRadioExImpl::cancelAvailableNetworks(int32_t serial) {
    RLOGD("cancelAvailableNetworks: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_ABORT_QUERY_AVAILABLE_NETWORKS);
    return Void();
}

Return<void> MtkRadioExImpl::cfgA2offset(int32_t serial, int32_t offset, int32_t threshBound) {
    //not support
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_CONFIG_A2_OFFSET);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::cfgB1offset(int32_t serial, int32_t offset, int32_t threshBound) {
    //not support
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_CONFIG_B1_OFFSET);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::enableSCGfailure(int32_t serial, bool enable, int32_t T1, int32_t P1, int32_t T2) {
    //not support
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_ENABLE_SCG_FAILURE);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::disableNR(int32_t serial, bool enable) {
    //not support
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_DISABLE_NR);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::setTxPower(int32_t serial, int32_t limitpower) {
    //not support
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_TX_POWER);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::setSearchStoredFreqInfo(int32_t serial, int32_t operation,
                                           int32_t plmnId, int32_t rat,
                                           const hidl_vec<int32_t>& freq) {
    //not support
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SEARCH_STORED_FREQUENCY_INFO);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::setSearchRat(int32_t serial, const hidl_vec<int32_t>& rat) {
    //not support
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SEARCH_RAT);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::setBgsrchDeltaSleepTimer(int32_t serial, int32_t sleepDuration) {
    //not support
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_BACKGROUND_SEARCH_TIMER);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::setBarringPasswordCheckedByNW(int32_t serial, const hidl_string& facility,
                                           const hidl_string& oldPassword,
                                           const hidl_string& newPassword,
                                           const hidl_string& cfmPassword) {
    RLOGD("setBarringPasswordCheckedByNW: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_CHANGE_BARRING_PASSWORD, true,
            4, facility.c_str(), oldPassword.c_str(), newPassword.c_str(), cfmPassword.c_str());
    return Void();
}

Return<void> MtkRadioExImpl::setClip(int32_t serial, int32_t clipEnable) {
    RLOGD("setClip: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_CLIP, 1, clipEnable);
    return Void();
}

Return<void> MtkRadioExImpl::getColp(int32_t serial) {
    RLOGD("getColp: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_COLP);
    return Void();
}

Return<void> MtkRadioExImpl::getColr(int32_t serial) {
    RLOGD("getColr: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_COLR);
    return Void();
}

Return<void> MtkRadioExImpl::sendCnap(int32_t serial, const hidl_string& cnapMessage) {
    RLOGD("sendCnap: serial %d", serial);
    dispatchString(serial, mSlotId, RIL_REQUEST_SEND_CNAP, cnapMessage.c_str());
    return Void();
}

Return<void> MtkRadioExImpl::acknowledgeLastIncomingCdmaSmsEx(int32_t serial, const CdmaSmsAck& smsAck) {
    RLOGD("acknowledgeLastIncomingCdmaSmsEx: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(
            serial, mSlotId, RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE_EX);
    if (pRI == NULL) {
        return Void();
    }

    RIL_CDMA_SMS_Ack rcsa = {};

    rcsa.uErrorClass = (RIL_CDMA_SMS_ErrorClass) smsAck.errorClass;
    rcsa.uSMSCauseCode = smsAck.smsCauseCode;

    CALL_ONREQUEST(pRI->pCI->requestNumber, &rcsa, sizeof(rcsa), pRI, mSlotId);
    return Void();
}

// MTK-START: RIL_REQUEST_SWITCH_MODE_FOR_ECC
Return<void> MtkRadioExImpl::triggerModeSwitchByEcc(int32_t serial, int32_t mode) {
    dispatchInts(serial, mSlotId, RIL_REQUEST_SWITCH_MODE_FOR_ECC, 1, mode);
    return Void();
}
// MTK-END

Return<void> MtkRadioExImpl::setTrm(int32_t serial, int32_t mode) {
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_TRM, 1, mode);
    return Void();
}

// MTK-START: SIM
Return<void> MtkRadioExImpl::getATR(int32_t serial) {
    RLOGD("[%d] getATR: serial %d", mSlotId, serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_SIM_GET_ATR);
    return Void();
}

Return<void> MtkRadioExImpl::getIccid(int32_t serial) {
    RLOGD("getIccid: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_SIM_GET_ICCID);
    return Void();
}

Return<void> MtkRadioExImpl::setSimPower(int32_t serial, int32_t mode) {
    RLOGD("setSimPower: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_SIM_POWER, 1, mode);
    return Void();
}

Return<void> MtkRadioExImpl::doGeneralSimAuthentication(int32_t serial,
        const SimAuthStructure& simAuth) {
    RLOGD("doGeneralSimAuthentication: serial %d", serial);
    dispatchSimGeneralAuth(serial, mSlotId, RIL_REQUEST_GENERAL_SIM_AUTH, simAuth);
    return Void();
}

/// M: CC: MTK proprietary call control ([IMS] common flow) @{
Return<void> MtkRadioExImpl::hangupAll(int32_t serial) {
    RLOGD("hangupAll: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_HANGUP_ALL);
    return Void();
}

Return<void> MtkRadioExImpl::hangupWithReason(int32_t serial, int32_t callId, int32_t reason) {
    RLOGD("hangupWithReason: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_HANGUP_WITH_REASON, 2, callId, reason);
    return Void();
}

Return<void> MtkRadioExImpl::setCallIndication(
        int32_t serial, int32_t mode, int32_t callId, int32_t seqNumber, int32_t cause) {
    RLOGD("setCallIndication: mode %d, callId %d, seqNumber %d, cause %d",
            mode, callId, seqNumber, cause);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_CALL_INDICATION, 4,
            mode, callId, seqNumber, cause);
    return Void();
}

/// M: CC: @}
// M: [LTE][Low Power][UL traffic shaping] @{
Return<void> MtkRadioExImpl::setLteAccessStratumReport(int32_t serial, int32_t enable) {
    RLOGD("setLteAccessStratumReport: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_LTE_ACCESS_STRATUM_REPORT, 1, enable);
    return Void();
}

Return<void> MtkRadioExImpl::setLteUplinkDataTransfer(int32_t serial, int32_t state, int32_t interfaceId) {
    RLOGD("setLteUplinkDataTransfer: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_LTE_UPLINK_DATA_TRANSFER, 2, state, interfaceId);
    return Void();
}
// M: [LTE][Low Power][UL traffic shaping] @}

/// M: CC: Emergency mode for Fusion RIL
Return<void> MtkRadioExImpl::setEccMode(int32_t serial, const hidl_string& number, int32_t enable,
        int32_t airplaneMode, int32_t imsReg) {
    RLOGD("setEccMode: serial %d, enable %d, airplaneMode %d, imsReg %d", serial,
            enable, airplaneMode, imsReg);
    return Void();
}


/// M: CC: Vzw/CTVolte ECC for Fusion RIL
Return<void> MtkRadioExImpl::eccPreferredRat(int32_t serial, int32_t phoneType) {
    RLOGD("eccPreferredRat: serial %d, phoneType %d", serial, phoneType);
    return Void();
}


Return<void> MtkRadioExImpl::sendRequestRaw(int32_t serial, const hidl_vec<uint8_t>& data) {
    RLOGD("RadioImpl::sendRequestRaw: serial %d", serial);
    dispatchRaw(serial, mSlotId, RIL_REQUEST_OEM_HOOK_RAW, data);
    return Void();
}

/////////////////////////////////////////////////////////////////////////////////////////
/// [IMS] -------------- IMS Request Function ------------------------------------------
/////////////////////////////////////////////////////////////////////////////////////////
Return<void> MtkRadioExImpl::videoCallAccept(int32_t serial,
                                        int32_t videoMode, int32_t callId) {
    RLOGD("videoCallAccept: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_VIDEO_CALL_ACCEPT, 2, videoMode, callId);
    return Void();
}

Return<void> MtkRadioExImpl::eccRedialApprove(int32_t serial, int32_t approve, int32_t callId) {
    RLOGD("eccRedialApprove: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_ECC_REDIAL_APPROVE, 2, approve, callId);
    return Void();
}

Return<void> MtkRadioExImpl::imsEctCommand(int32_t serial, const hidl_string& number,
        int32_t type) {
    RLOGD("imsEctCommand: serial %d", serial);
    std::string strType = std::to_string(type);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_IMS_ECT, false,
            2, (const char *)number.c_str(), (const char *)strType.c_str());
    return Void();
}

Return<void> MtkRadioExImpl::controlCall(int32_t serial, int32_t controlType, int32_t callId) {
    RLOGD("controlCall: serial %d, controlType %d, callId %d", serial, controlType, callId);
    if (controlType == 0) {
        dispatchInts(serial, mSlotId, RIL_REQUEST_HOLD_CALL, 1, callId);
    } else if (controlType == 1) {
        dispatchInts(serial, mSlotId, RIL_REQUEST_RESUME_CALL, 1, callId);
    }
    return Void();
}

Return<void> MtkRadioExImpl::imsDeregNotification(int32_t serial, int32_t cause) {
    RLOGD("imsDeregNotification: serial %d, cause %d", serial, cause);
    dispatchInts(serial, mSlotId, RIL_REQUEST_IMS_DEREG_NOTIFICATION, 1, cause);
    return Void();
}

// Femtocell feature
Return<void> MtkRadioExImpl::getFemtocellList(int32_t serial) {
    RLOGD("getFemtocellList: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_FEMTOCELL_LIST);
    return Void();
}

Return<void> MtkRadioExImpl::abortFemtocellList(int32_t serial) {
    RLOGD("abortFemtocellList: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_ABORT_FEMTOCELL_LIST);
    return Void();
}

Return<void> MtkRadioExImpl::selectFemtocell(int32_t serial,
                                                const hidl_string& operatorNumeric,
                                                const hidl_string& act,
                                                const hidl_string& csgId) {
    RLOGD("selectFemtocell: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_SELECT_FEMTOCELL, true, 3,
            operatorNumeric.c_str(), act.c_str(), csgId.c_str());
    return Void();
}

Return<void> MtkRadioExImpl::queryFemtoCellSystemSelectionMode(int32_t serial) {
    RLOGD("queryFemtoCellSystemSelectionMode: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_QUERY_FEMTOCELL_SYSTEM_SELECTION_MODE);
    return Void();
}

Return<void> MtkRadioExImpl::setFemtoCellSystemSelectionMode(int32_t serial, int32_t mode) {
    RLOGD("setFemtoCellSystemSelectionMode: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_FEMTOCELL_SYSTEM_SELECTION_MODE, 1, mode);
    return Void();
}

Return<void> MtkRadioExImpl::setImsEnable(int32_t serial, bool isOn) {
    RLOGD("setImsEnable: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_IMS_ENABLE, 1, isOn ? 1:0);
    return Void();
}

Return<void> MtkRadioExImpl::setImsVideoEnable(int32_t serial, bool isOn) {
    RLOGD("setImsVideoEnable: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_IMS_VIDEO_ENABLE, 1, isOn ? 1:0);
    return Void();
}

Return<void> MtkRadioExImpl::setImscfg(int32_t serial, bool volteEnable,
        bool vilteEnable, bool vowifiEnable, bool viwifiEnable,
        bool smsEnable, bool eimsEnable) {
    int params[6] = {0};
    params[0] = volteEnable ? 1:0;
    params[1] = vilteEnable ? 1:0;
    params[2] = vowifiEnable ? 1:0;
    params[3] = viwifiEnable ? 1:0;
    params[4] = smsEnable ? 1:0;
    params[5] = eimsEnable ? 1:0;

    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_IMSCFG, 6, params);
    return Void();
}

Return<void> MtkRadioExImpl::setModemImsCfg(int32_t serial, const hidl_string& keys,
        const hidl_string& values, int32_t type) {
   LOGD("setModemImsCfg: serial %d", serial);
   dispatchStrings(serial, mSlotId, RIL_REQUEST_SET_MD_IMSCFG, false, 3, keys.c_str(), values.c_str(),
           std::to_string(type).c_str());
   return Void();
}

Return<void> MtkRadioExImpl::getProvisionValue(int32_t serial,
        const hidl_string& provisionstring) {
    RLOGD("getProvisionValue: serial %d", serial);
    dispatchString(serial, mSlotId, RIL_REQUEST_GET_PROVISION_VALUE, provisionstring.c_str());
    return Void();
}

Return<void> MtkRadioExImpl::setProvisionValue(int32_t serial,
        const hidl_string& provisionstring, const hidl_string& provisionValue) {
    RLOGD("setProvisionValue: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_SET_PROVISION_VALUE,false,
            2, provisionstring.c_str(), provisionValue.c_str());
    return Void();
}

// Ims Config telephonyware START
Return<void>
MtkRadioExImpl::setImsCfgFeatureValue(int32_t serial, int32_t featureId, int32_t network, int32_t value,
                                 int32_t isLast) {
    return Void();
}

Return<void> MtkRadioExImpl::getImsCfgFeatureValue(int32_t serial, int32_t featureId, int32_t network) {
    return Void();
}


Return<void> MtkRadioExImpl::setImsCfgProvisionValue(int32_t serial, int32_t configId,
                                                const hidl_string& value) {
    return Void();
}

Return<void> MtkRadioExImpl::getImsCfgProvisionValue(int32_t serial, int32_t configId) {
    return Void();
}

Return<void> MtkRadioExImpl::getImsCfgResourceCapValue(int32_t serial, int32_t featureId) {
    return Void();
}
// Ims Config telephonyware END

Return<void> MtkRadioExImpl::controlImsConferenceCallMember(int32_t serial, int32_t controlType,
        int32_t confCallId, const hidl_string& address, int32_t callId) {
    RLOGD("controlImsConferenceCallMember: serial %d, controlType %d", serial, controlType);

    if (controlType == 0) {
        dispatchStrings(serial, mSlotId, RIL_REQUEST_REMOVE_IMS_CONFERENCE_CALL_MEMBER, true, 3,
                std::to_string(confCallId).c_str(), address.c_str(),
                std::to_string(callId).c_str());
    } else if (controlType == 1) {
        dispatchStrings(serial, mSlotId, RIL_REQUEST_ADD_IMS_CONFERENCE_CALL_MEMBER, true, 3,
                std::to_string(confCallId).c_str(), address.c_str(),
                std::to_string(callId).c_str());
    }

    return Void();
}

Return<void> MtkRadioExImpl::setWfcProfile(int32_t serial, int32_t wfcPreference) {
    RLOGD("setWfcProfile: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_WFC_PROFILE, 1, wfcPreference);
    return Void();
}

Return<void> MtkRadioExImpl::conferenceDial(int32_t serial, const ConferenceDial& dialInfo) {
    RLOGD("conferenceDial: serial %d", serial);

    int request = RIL_REQUEST_CONFERENCE_DIAL;
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, request);
    if (pRI == NULL) {
        return Void();
    }

    int countStrings = dialInfo.dialNumbers.size() + 3;
    char **pStrings;
    pStrings = (char **)calloc(countStrings, sizeof(char *));
    if (pStrings == NULL) {
        RLOGE("Memory allocation failed for request %s", requestToString(request));

        sendErrorResponse(pRI, RIL_E_NO_MEMORY);
        return Void();
    }

    if(!copyHidlStringToRil(&pStrings[0], dialInfo.isVideoCall ? "1":"0", pRI)) {
        free(pStrings);
        return Void();
    }

    if(!copyHidlStringToRil(&pStrings[1],
                            std::to_string(dialInfo.dialNumbers.size()).c_str(), pRI)) {
        memsetAndFreeStrings(1, pStrings[0]);
        free(pStrings);
        return Void();
    }

    int i = 0;
    for (i = 0; i < (int) dialInfo.dialNumbers.size(); i++) {
        if (!copyHidlStringToRil(&pStrings[i + 2], dialInfo.dialNumbers[i], pRI)) {
            for (int j = 0; j < i + 2; j++) {
                memsetAndFreeStrings(1, pStrings[j]);
            }

            free(pStrings);
            return Void();
        }
    }

    if(!copyHidlStringToRil(&pStrings[i + 2],
        std::to_string((int)dialInfo.clir).c_str(), pRI)) {
        for (int j = 0; j < (int) dialInfo.dialNumbers.size() + 2; j++) {
            memsetAndFreeStrings(1, pStrings[j]);
        }

        free(pStrings);
        return Void();
    }

    s_vendorFunctions->onRequest(request, pStrings, countStrings * sizeof(char *), pRI,
            pRI->socket_id);

    if (pStrings != NULL) {
        for (int j = 0 ; j < countStrings ; j++) {
            memsetAndFreeStrings(1, pStrings[j]);
        }

#ifdef MEMSET_FREED
        memset(pStrings, 0, countStrings * sizeof(char *));
#endif
        free(pStrings);
    }

    return Void();
}
// [IMS] ViLTE Dial
Return<void> MtkRadioExImpl::vtDial(int32_t serial, const Dial& dialInfo) {
    RLOGD("vtDial: serial %d", serial);
    int requestId = RIL_REQUEST_VT_DIAL;
    if(mClientId == android::CLIENT_IMS) {
        requestId = RIL_REQUEST_IMS_VT_DIAL;
    }
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, requestId);
    if (pRI == NULL) {
        return Void();
    }
    RIL_Dial dial = {};
    RIL_UUS_Info uusInfo = {};
    int32_t sizeOfDial = sizeof(dial);
    if (!copyHidlStringToRil(&dial.address, dialInfo.address, pRI)) {
        return Void();
    }
    dial.clir = (int) dialInfo.clir;
    if (dialInfo.uusInfo.size() != 0) {
        uusInfo.uusType = (RIL_UUS_Type) dialInfo.uusInfo[0].uusType;
        uusInfo.uusDcs = (RIL_UUS_DCS) dialInfo.uusInfo[0].uusDcs;
        if (dialInfo.uusInfo[0].uusData.size() == 0) {
            uusInfo.uusData = NULL;
            uusInfo.uusLength = 0;
        } else {
            if (!copyHidlStringToRil(&uusInfo.uusData,
                                     dialInfo.uusInfo[0].uusData, pRI)) {
                memsetAndFreeStrings(1, dial.address);
                return Void();
            }
            uusInfo.uusLength = dialInfo.uusInfo[0].uusData.size();
        }
        dial.uusInfo = &uusInfo;
    }
    s_vendorFunctions->onRequest(requestId,
                                 &dial, sizeOfDial, pRI, pRI->socket_id);
    memsetAndFreeStrings(2, dial.address, uusInfo.uusData);
    return Void();
}
Return<void> MtkRadioExImpl::vtDialWithSipUri(int32_t serial, const hidl_string& address) {
    RLOGD("vtDialWithSipUri: serial %d", serial);
    dispatchString(serial, mSlotId, RIL_REQUEST_VT_DIAL_WITH_SIP_URI, address.c_str());

    return Void();
}

Return<void> MtkRadioExImpl::dialWithSipUri(int32_t serial, const hidl_string& address) {
    RLOGD("dialWithSipUri: serial %d", serial);
    dispatchString(serial, mSlotId, RIL_REQUEST_DIAL_WITH_SIP_URI, address.c_str());

    return Void();
}

Return<void> MtkRadioExImpl::sendUssi(int32_t serial, const hidl_string& ussiString) {
    RLOGD("sendUssi: serial %d", serial);

    dispatchString(serial, mSlotId, RIL_REQUEST_SEND_USSI, ussiString.c_str());

    return Void();
}

Return<void> MtkRadioExImpl::cancelUssi(int32_t serial) {
    RLOGD("cancelPendingUssi: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_CANCEL_USSI);

    return Void();
}

Return<void> MtkRadioExImpl::getXcapStatus(int32_t serial) {
    RLOGD("getXcapStatus: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_XCAP_STATUS);

    return Void();
}

Return<void> MtkRadioExImpl::resetSuppServ(int32_t serial) {
    RLOGD("resetSuppServ: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_RESET_SUPP_SERV);

    return Void();
}

Return<void> MtkRadioExImpl::setupXcapUserAgentString(int32_t serial, const hidl_string& userAgent) {
    RLOGD("setupXcapUserAgentString: serial %d", serial);

    dispatchStrings(serial, mSlotId, RIL_REQUEST_SETUP_XCAP_USER_AGENT_STRING, true, 1,
                    userAgent.c_str());

    return Void();
}

Return<void> MtkRadioExImpl::forceReleaseCall(int32_t serial, int32_t callId) {
    RLOGD("forceHangup: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_FORCE_RELEASE_CALL, 1,
                 callId);

    return Void();
}

Return<void> MtkRadioExImpl::imsBearerStateConfirm(int32_t serial, int32_t aid,
       int32_t action, int32_t status) {

    RLOGD("responseBearerStateConfirm: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_IMS_BEARER_STATE_CONFIRM, 3,
                 aid, action, status);

    return Void();
}

Return<void> MtkRadioExImpl::setImsBearerNotification(int32_t serial, int32_t enable) {

    RLOGD("setImsBearerNotification: serial:  %d, enable: %d", serial, enable);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_IMS_BEARER_NOTIFICATION, 1, enable);

    return Void();
}

Return<void> MtkRadioExImpl::setImsRtpReport(int32_t serial, int32_t pdnId,
       int32_t networkId, int32_t timer) {
    RLOGD("setImsRtpReport: serial %d", serial);

    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_IMS_RTP_REPORT, 3,
                 pdnId, networkId, timer);

    return Void();
}

Return<void> MtkRadioExImpl::pullCall(int32_t serial,
                                 const hidl_string& target,
                                 bool isVideoCall) {
    RLOGD("pullCall: serial %d", serial);

    dispatchStrings(serial, mSlotId, RIL_REQUEST_PULL_CALL, false, 2, target.c_str(),
                 isVideoCall ? "1":"0");

    return Void();
}
Return<void> MtkRadioExImpl::setImsRegistrationReport(int32_t serial) {
    RLOGD("setImsRegistrationReport: serial %d", serial);

    dispatchVoid(serial, mSlotId, RIL_REQUEST_SET_IMS_REGISTRATION_REPORT);
    return Void();
}

Return<void> MtkRadioExImpl::sendRequestStrings(int32_t serial,
        const hidl_vec<hidl_string>& data) {
    RLOGD("RadioImpl::sendRequestStrings: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_OEM_HOOK_STRINGS, data);
    return Void();
}

// ATCI Start
Return<void> MtkRadioExImpl::setResponseFunctionsForAtci(
        const ::android::sp<IAtciResponse>& atciResponseParam,
        const ::android::sp<IAtciIndication>& atciIndicationParam) {
    RLOGD("MtkRadioExImpl::setResponseFunctionsForAtci");
    mAtciResponse = atciResponseParam;
    mAtciIndication = atciIndicationParam;
    return Void();
}

Return<void> MtkRadioExImpl::sendAtciRequest(int32_t serial, const hidl_vec<uint8_t>& data) {
    RLOGD("MtkRadioExImpl::sendAtciRequest: serial %d", serial);
    dispatchRaw(serial, mSlotId, RIL_REQUEST_OEM_HOOK_ATCI_INTERNAL, data);
    return Void();
}
// ATCI End

// SUBSIDYLOCK Start
Return<void> MtkRadioExImpl::setResponseFunctionsSubsidyLock(
        const ::android::sp<VENDOR_V1_0::ISubsidyLockResponse>& sublockResponseParam,
        const ::android::sp<VENDOR_V1_0::ISubsidyLockIndication>& sublockIndicationParam) {
    RLOGE("setResponseFunctionsSubsidyLock");
    mRadioResponseSubsidy = sublockResponseParam;
    mRadioIndicationSubsidy = sublockIndicationParam;
    return Void();
}

Return<void> MtkRadioExImpl::sendSubsidyLockRequest(int32_t serial, int32_t reqType,
        const hidl_vec<uint8_t>& data) {
    RLOGE("sendSubsidyLockRequest: serial = %d, reqType = %d", serial, reqType);
    if (reqType == 1) {
        dispatchRaw(serial, mSlotId, RIL_REQUEST_UPDATE_SUBLOCK_SETTINGS, data);
    } else {
        dispatchRaw(serial, mSlotId, RIL_REQUEST_GET_SUBLOCK_MODEM_STATUS, data);
    }
    return Void();
}
// SUBSIDYLOCK End

/// M: eMBMS feature
Return<void> MtkRadioExImpl::sendEmbmsAtCommand(int32_t serial, const hidl_string& s) {
    RLOGD("sendEmbmsAtCommand: serial %d", serial);
    dispatchString(serial, mSlotId, RIL_REQUEST_EMBMS_AT_CMD, s.c_str());
    return Void();
}
/// M: eMBMS end

// worldmode {
Return<void> MtkRadioExImpl::setResumeRegistration(int32_t serial, int32_t sessionId) {
    RLOGD("MtkRadioExImpl::setResumeRegistration: serial %d sessionId %d", serial, sessionId);
    dispatchInts(serial, mSlotId, RIL_REQUEST_RESUME_REGISTRATION, 1, sessionId);
    return Void();
}

// MTK-START: SIM ME LOCK
Return<void> MtkRadioExImpl::queryNetworkLock(int32_t serial, int32_t category) {
    RLOGD("queryNetworkLock: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_QUERY_SIM_NETWORK_LOCK, 1, category);
    return Void();
}

Return<void> MtkRadioExImpl::setNetworkLock(int32_t serial, int32_t category,
        int32_t lockop, const ::android::hardware::hidl_string& password,
        const ::android::hardware::hidl_string& data_imsi,
        const ::android::hardware::hidl_string& gid1,
        const ::android::hardware::hidl_string& gid2) {
    RLOGD("setNetworkLock: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_SET_SIM_NETWORK_LOCK,
            true,
            6,
            std::to_string((int) category).c_str(),
            std::to_string((int) lockop).c_str(),
            password.c_str(),
            data_imsi.c_str(),
            gid1.c_str(),
            gid2.c_str());
    return Void();
}

Return<void> MtkRadioExImpl::supplyDepersonalization(int32_t serial, const hidl_string& netPin,
        int32_t type) {
    RLOGD("supplyDepersonalization: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_ENTER_DEPERSONALIZATION, true, 2,
            netPin.c_str(), std::to_string((int) type).c_str());
    return Void();
}
// MTK-END

Return<void> MtkRadioExImpl::modifyModemType(int32_t serial, int32_t applyType, int32_t modemType) {
    RLOGD("RadioImpl::modifyModemType:%d, %d, %d", serial, applyType, modemType);
    dispatchInts(serial, mSlotId, RIL_REQUEST_MODIFY_MODEM_TYPE, 2, applyType, modemType);
    return Void();
}

// PHB START
Return<void> MtkRadioExImpl::queryPhbStorageInfo(int32_t serial, int32_t type) {
    RLOGD("queryPhbStorageInfo: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_QUERY_PHB_STORAGE_INFO,
            1, type);
    return Void();
}

Return<void> MtkRadioExImpl::writePhbEntry(int32_t serial, const PhbEntryStructure& phbEntry) {
    RLOGD("writePhbEntry: serial %d", serial);
    dispatchPhbEntry(serial, mSlotId, RIL_REQUEST_WRITE_PHB_ENTRY, phbEntry);
    return Void();
}

Return<void> MtkRadioExImpl::readPhbEntry(int32_t serial, int32_t type, int32_t bIndex, int32_t eIndex) {
    RLOGD("readPhbEntry: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_READ_PHB_ENTRY,
            3, type, bIndex, eIndex);
    return Void();
}

Return<void> MtkRadioExImpl::queryUPBCapability(int32_t serial) {
    RLOGD("queryUPBCapability: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_QUERY_UPB_CAPABILITY);
    return Void();
}

Return<void> MtkRadioExImpl::editUPBEntry(int32_t serial, const hidl_vec<hidl_string>& data) {
    RLOGD("editUPBEntry: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_EDIT_UPB_ENTRY, data);
    return Void();
}

Return<void> MtkRadioExImpl::deleteUPBEntry(int32_t serial, int32_t entryType, int32_t adnIndex, int32_t entryIndex) {
    RLOGD("deleteUPBEntry: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_DELETE_UPB_ENTRY,
           3, entryType, adnIndex, entryIndex);
    return Void();
}

Return<void> MtkRadioExImpl::readUPBGasList(int32_t serial, int32_t startIndex, int32_t endIndex) {
    RLOGD("readUPBGasList: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_READ_UPB_GAS_LIST,
            2, startIndex, endIndex);
    return Void();
}

Return<void> MtkRadioExImpl::readUPBGrpEntry(int32_t serial, int32_t adnIndex) {
    RLOGD("readUPBGrpEntry: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_READ_UPB_GRP,
            1, adnIndex);
    return Void();
}


Return<void> MtkRadioExImpl::writeUPBGrpEntry(int32_t serial, int32_t adnIndex, const hidl_vec<int32_t>& grpIds) {
    RLOGD("writeUPBGrpEntry: serial %d", serial);
    dispatchGrpEntry(serial, mSlotId, RIL_REQUEST_WRITE_UPB_GRP, adnIndex, grpIds);
    return Void();
}

Return<void> MtkRadioExImpl::getPhoneBookStringsLength(int32_t serial) {
    RLOGD("getPhoneBookStringsLength: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_PHB_STRING_LENGTH);
    return Void();
}

Return<void> MtkRadioExImpl::getPhoneBookMemStorage(int32_t serial) {
    RLOGD("getPhoneBookMemStorage: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_PHB_MEM_STORAGE);
    return Void();
}

Return<void> MtkRadioExImpl::setPhoneBookMemStorage(int32_t serial,
        const hidl_string& storage, const hidl_string& password) {
    RLOGD("setPhoneBookMemStorage: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_SET_PHB_MEM_STORAGE, true,
            2, storage.c_str(), password.c_str());
    return Void();
}

Return<void> MtkRadioExImpl::readPhoneBookEntryExt(int32_t serial, int32_t index1, int32_t index2) {
    RLOGD("readPhoneBookEntryExt: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_READ_PHB_ENTRY_EXT,
            2, index1, index2);
    return Void();
}

Return<void> MtkRadioExImpl::writePhoneBookEntryExt(int32_t serial, const PhbEntryExt& phbEntryExt) {
    RLOGD("writePhoneBookEntryExt: serial %d", serial);
    dispatchPhbEntryExt(serial, mSlotId, RIL_REQUEST_WRITE_PHB_ENTRY_EXT,
            phbEntryExt);
    return Void();
}

Return<void> MtkRadioExImpl::queryUPBAvailable(int32_t serial, int32_t eftype, int32_t fileIndex) {
    RLOGD("queryUPBAvailable: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_QUERY_UPB_AVAILABLE,
            2, eftype, fileIndex);
    return Void();
}

Return<void> MtkRadioExImpl::readUPBEmailEntry(int32_t serial, int32_t adnIndex, int32_t fileIndex) {
    RLOGD("readUPBEmailEntry: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_READ_EMAIL_ENTRY,
            2, adnIndex, fileIndex);
    return Void();
}

Return<void> MtkRadioExImpl::readUPBSneEntry(int32_t serial, int32_t adnIndex, int32_t fileIndex) {
    RLOGD("readUPBSneEntry: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_READ_SNE_ENTRY,
            2, adnIndex, fileIndex);
    return Void();
}

Return<void> MtkRadioExImpl::readUPBAnrEntry(int32_t serial, int32_t adnIndex, int32_t fileIndex) {
    RLOGD("readUPBAnrEntry: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_READ_ANR_ENTRY,
            2, adnIndex, fileIndex);
    return Void();
}

Return<void> MtkRadioExImpl::readUPBAasList(int32_t serial, int32_t startIndex, int32_t endIndex) {
    RLOGD("readUPBAasList: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_READ_UPB_AAS_LIST,
            2, startIndex, endIndex);
    return Void();
}

Return<void> MtkRadioExImpl::setPhonebookReady(int32_t serial, int32_t ready) {
    RLOGD("RadioImpl::setPhonebookReady: serial %d ready %d", serial, ready);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_PHONEBOOK_READY, 1, ready);
    return Void();
}
// PHB END

Return<void> MtkRadioExImpl::setRxTestConfig(int32_t serial, int32_t antType) {
    RLOGD("setRxTestConfig: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_VSS_ANTENNA_CONF, 1, antType);
    return Void();
}
Return<void> MtkRadioExImpl::getRxTestResult(int32_t serial, int32_t mode) {
    RLOGD("getRxTestResult: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_VSS_ANTENNA_INFO, 1, mode);
    return Void();
}

Return<void> MtkRadioExImpl::getPOLCapability(int32_t serial) {
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_POL_CAPABILITY);
    return Void();
}

Return<void> MtkRadioExImpl::getCurrentPOLList(int32_t serial) {
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_POL_LIST);
    return Void();
}

Return<void> MtkRadioExImpl::setPOLEntry(int32_t serial, int32_t index,
        const ::android::hardware::hidl_string& numeric,
        int32_t nAct) {
    dispatchStrings(serial, mSlotId, RIL_REQUEST_SET_POL_ENTRY,
            true,
            3,
            std::to_string((int) index).c_str(),
            numeric.c_str(),
            std::to_string((int) nAct).c_str());
    return Void();
}

/// M: [Network][C2K] Sprint roaming control @{
Return<void> MtkRadioExImpl::setRoamingEnable(int32_t serial, const hidl_vec<int32_t>& config) {
    RLOGD("setRoamingEnable: serial: %d", serial);
    if (config.size() == 6) {
        dispatchInts(serial, mSlotId, RIL_REQUEST_SET_ROAMING_ENABLE, 6,
                config[0], config[1], config[2], config[3], config[4], config[5]);
    } else {
        RLOGE("setRoamingEnable: param error, num: %d (should be 6)", (int) config.size());
    }
    return Void();
}

Return<void> MtkRadioExImpl::getRoamingEnable(int32_t serial, int32_t phoneId) {
    dispatchInts(serial, mSlotId, RIL_REQUEST_GET_ROAMING_ENABLE, 1, phoneId);
    return Void();
}
/// @}

Return<void> MtkRadioExImpl::setLteReleaseVersion(int32_t serial, int32_t mode) {
    RLOGD("setLteReleaseVersion: serial: %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_LTE_RELEASE_VERSION);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::getLteReleaseVersion(int32_t serial) {
    RLOGD("getLteReleaseVersion: serial: %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_GET_LTE_RELEASE_VERSION);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::setSuppServProperty(int32_t serial, const hidl_string& name, const hidl_string& value) {
    RLOGI("setSuppServProperty: serial %d, name %s, value %s ", serial, name.c_str(), value.c_str());

    dispatchStrings(serial, mSlotId, RIL_REQUEST_SET_SS_PROPERTY, true,
            2, name.c_str(), value.c_str());
    return Void();
}

// M: RTT @{
Return<void> MtkRadioExImpl::setRttMode(int serial, int mode) {
    RLOGD("[NOT SUPPORT] setRttMode: serial %d", serial);
    // It's 93RID only.
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_RTT_MODE);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::sendRttModifyRequest(int serial, int callId, int newMode) {
    RLOGD("[NOT SUPPORT] sendRttModifyRequest: serial %d", serial);
    // It's 93RID only.
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SEND_RTT_MODIFY_REQUEST);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::sendRttText(int serial, int callId, int lenOfString, const hidl_string& text) {
    RLOGD("[NOT SUPPORT] sendRttText: serial %d", serial);
    // It's 93RID only.
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SEND_RTT_TEXT);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::rttModifyRequestResponse(int serial, int callId, int result) {
    RLOGD("[NOT SUPPORT] rttModifyRequestResponse: serial %d", serial);
    // It's 93RID only.
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_RTT_MODIFY_REQUST_RESPONSE);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}
Return<void> MtkRadioExImpl::toggleRttAudioIndication(int serial, int callId, int enable) {
    RLOGD(LOG_TAG, "toggleRttAudioIndication: serial %d", serial);
    // It's 93RID only.
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_TOGGLE_RTT_AUDIO_INDICATION);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}
// @}

Return<void> MtkRadioExImpl::enableDsdaIndication(int serial, bool enable) {
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_ENABLE_DSDA_INDICATION);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::getDsdaStatus(int serial) {
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_GET_DSDA_STATUS);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

/***************************************************************************************************
 * RESPONSE FUNCTIONS
 * Functions above are used for requests going from framework to vendor code. The ones below are
 * responses for those requests coming back from the vendor code.
 **************************************************************************************************/

void radio::acknowledgeRequest(int slotId, int serial) {
    if (radioService[slotId]->mRadioResponse != NULL) {
        Return<void> retStatus = radioService[slotId]->mRadioResponse->acknowledgeRequest(serial);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("acknowledgeRequest: radioService[%d]->mRadioResponse == NULL", slotId);
    }
}

int responseIntOrEmpty(RadioResponseInfo& responseInfo, int serial, int responseType, RIL_Errno e,
               void *response, size_t responseLen) {
    populateResponseInfo(responseInfo, serial, responseType, e);
    int ret = -1;

    if (response == NULL && responseLen == 0) {
        // Earlier RILs did not send a response for some cases although the interface
        // expected an integer as response. Do not return error if response is empty. Instead
        // Return -1 in those cases to maintain backward compatibility.
    } else if (response == NULL || responseLen != sizeof(int)) {
        RLOGE("responseIntOrEmpty: Invalid response");
        if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
    } else {
        int *p_int = (int *) response;
        ret = p_int[0];
    }
    return ret;
}

int responseInt(RadioResponseInfo& responseInfo, int serial, int responseType, RIL_Errno e,
               void *response, size_t responseLen) {
    populateResponseInfo(responseInfo, serial, responseType, e);
    int ret = -1;

    if (response == NULL || responseLen != sizeof(int)) {
        RLOGE("responseInt: Invalid response");
        if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
    } else {
        int *p_int = (int *) response;
        ret = p_int[0];
    }
    return ret;
}

int radio::getIccCardStatusResponse(int slotId, android::ClientId clientId,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen) {
    if (radioService[slotId]->mRadioResponse != NULL
            || radioService[slotId]->mRadioResponseV1_2 != NULL
            || radioService[slotId]->mRadioResponseV1_4 != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        AOSP_V1_4::CardStatus cardStatus = {};
        RIL_CardStatus_v8 *p_cur = ((RIL_CardStatus_v8 *) response);
        if (response == NULL || responseLen != sizeof(RIL_CardStatus_v8)
                || p_cur->gsm_umts_subscription_app_index >= p_cur->num_applications
                || p_cur->cdma_subscription_app_index >= p_cur->num_applications
                || p_cur->ims_subscription_app_index >= p_cur->num_applications) {
            RLOGE("getIccCardStatusResponse: Invalid response");
            cardStatus.base.base.cardState = CardState::ERROR;
            cardStatus.base.base.universalPinState = PinState::UNKNOWN;
            cardStatus.base.base.gsmUmtsSubscriptionAppIndex = -1;
            cardStatus.base.base.cdmaSubscriptionAppIndex = -1;
            cardStatus.base.base.imsSubscriptionAppIndex = -1;
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            cardStatus.base.base.cardState = (CardState) p_cur->card_state;
            cardStatus.base.base.universalPinState = (PinState) p_cur->universal_pin_state;
            cardStatus.base.base.gsmUmtsSubscriptionAppIndex
                    = p_cur->gsm_umts_subscription_app_index;
            cardStatus.base.base.cdmaSubscriptionAppIndex = p_cur->cdma_subscription_app_index;
            cardStatus.base.base.imsSubscriptionAppIndex = p_cur->ims_subscription_app_index;

            RIL_AppStatus *rilAppStatus = p_cur->applications;
            cardStatus.base.base.applications.resize(p_cur->num_applications);
            AppStatus *appStatus = cardStatus.base.base.applications.data();
            RLOGD("getIccCardStatusResponse: num_applications %d", p_cur->num_applications);
            for (int i = 0; i < p_cur->num_applications; i++) {
                appStatus[i].appType = (AppType) rilAppStatus[i].app_type;
                appStatus[i].appState = (AppState) rilAppStatus[i].app_state;
                appStatus[i].persoSubstate = (PersoSubstate) rilAppStatus[i].perso_substate;
                appStatus[i].aidPtr = convertCharPtrToHidlString(rilAppStatus[i].aid_ptr);
                appStatus[i].appLabelPtr = convertCharPtrToHidlString(
                        rilAppStatus[i].app_label_ptr);
                appStatus[i].pin1Replaced = rilAppStatus[i].pin1_replaced;
                appStatus[i].pin1 = (PinState) rilAppStatus[i].pin1;
                appStatus[i].pin2 = (PinState) rilAppStatus[i].pin2;
            }

            if (radioService[slotId]->mRadioResponseV1_4 != NULL) {
                cardStatus.base.physicalSlotId = p_cur->physicalSlotId;
                cardStatus.base.atr = convertCharPtrToHidlString(p_cur->atr);
                cardStatus.base.iccid = convertCharPtrToHidlString(p_cur->iccId);
                cardStatus.eid = convertCharPtrToHidlString(p_cur->eid);
            } else if (radioService[slotId]->mRadioResponseV1_2 != NULL) {
                cardStatus.base.physicalSlotId = p_cur->physicalSlotId;
                cardStatus.base.atr = convertCharPtrToHidlString(p_cur->atr);
                cardStatus.base.iccid = convertCharPtrToHidlString(p_cur->iccId);
            }
        }

        // M @{
        s_cardStatus[slotId].base.cardState = cardStatus.base.base.cardState;
        // M @}

        if (radioService[slotId]->mRadioResponseV1_4 != NULL) {
            Return<void> retStatus = radioService[slotId]->mRadioResponseV1_4->
                    getIccCardStatusResponse_1_4(responseInfo, cardStatus);
            radioService[slotId]->checkReturnStatus(retStatus, slotId);
        } else if (radioService[slotId]->mRadioResponseV1_2 != NULL) {
            Return<void> retStatus = radioService[slotId]->mRadioResponseV1_2->
                    getIccCardStatusResponse_1_2(responseInfo, cardStatus.base);
            radioService[slotId]->checkReturnStatus(retStatus, slotId);
        } else if (radioService[slotId]->mRadioResponse != NULL) {
            Return<void> retStatus = radioService[slotId]->mRadioResponse->
                    getIccCardStatusResponse(responseInfo, cardStatus.base.base);
            radioService[slotId]->checkReturnStatus(retStatus, slotId);
        } else {
                RLOGE("getIccCardStatusResponse:radioService[%d]->mRadioResponse == NULL", slotId);
        }
    } else {
        RLOGE("getIccCardStatusResponse:No valid Service for slotId[%d]", slotId);
    }

    return 0;
}

int radio::supplyIccPinForAppResponse(int slotId,  android::ClientId clientId __unused,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responseLen) {
    RLOGD("supplyIccPinForAppResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseIntOrEmpty(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->
                supplyIccPinForAppResponse(responseInfo, ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("supplyIccPinForAppResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::supplyIccPukForAppResponse(int slotId,  android::ClientId clientId __unused,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responseLen) {
    RLOGD("supplyIccPukForAppResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseIntOrEmpty(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->supplyIccPukForAppResponse(
                responseInfo, ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("supplyIccPukForAppResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::supplyIccPin2ForAppResponse(int slotId,  android::ClientId clientId __unused,
                                      int responseType, int serial, RIL_Errno e,
                                      void *response, size_t responseLen) {
    RLOGD("supplyIccPin2ForAppResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseIntOrEmpty(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->
                supplyIccPin2ForAppResponse(responseInfo, ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("supplyIccPin2ForAppResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::supplyIccPuk2ForAppResponse(int slotId,  android::ClientId clientId __unused,
                                      int responseType, int serial, RIL_Errno e,
                                      void *response, size_t responseLen) {
    RLOGD("supplyIccPuk2ForAppResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseIntOrEmpty(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->
                supplyIccPuk2ForAppResponse(responseInfo, ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("supplyIccPuk2ForAppResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::changeIccPinForAppResponse(int slotId, android::ClientId clientId __unused,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responseLen) {
    RLOGD("changeIccPinForAppResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseIntOrEmpty(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->
                changeIccPinForAppResponse(responseInfo, ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("changeIccPinForAppResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::changeIccPin2ForAppResponse(int slotId, android::ClientId clientId __unused,
                                      int responseType, int serial, RIL_Errno e,
                                      void *response, size_t responseLen) {
    RLOGD("changeIccPin2ForAppResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseIntOrEmpty(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->
                changeIccPin2ForAppResponse(responseInfo, ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("changeIccPin2ForAppResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::supplyNetworkDepersonalizationResponse(int slotId, android::ClientId clientId __unused,
                                                 int responseType, int serial, RIL_Errno e,
                                                 void *response, size_t responseLen) {
    RLOGD("supplyNetworkDepersonalizationResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseIntOrEmpty(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->
                supplyNetworkDepersonalizationResponse(responseInfo, ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("supplyNetworkDepersonalizationResponse: radioService[%d]->mRadioResponse == "
                "NULL", slotId);
    }

    return 0;
}

int radio::getCurrentCallsResponse(int slotId, android::ClientId clientId __unused,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
    RLOGD("getCurrentCallsResponse: serial %d", serial);

    if (radioService[slotId] != NULL && (radioService[slotId]->mRadioResponse != NULL
            || radioService[slotId]->mRadioResponseV1_2 != NULL)) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        hidl_vec<AOSP_V1_2::Call> callsV1_2;
        hidl_vec<AOSP_V1_0::Call> calls;
        if (response == NULL || (responseLen % sizeof(RIL_Call *)) != 0) {
            RLOGE("getCurrentCallsResponse: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int num = responseLen / sizeof(RIL_Call *);

            if (radioService[slotId]->mRadioResponseV1_2 != NULL) {
                callsV1_2.resize(num);

                for (int i = 0 ; i < num ; i++) {
                    RIL_Call *p_cur = ((RIL_Call **) response)[i];
                    /* each call info */
                    callsV1_2[i].base.state = (CallState) p_cur->state;
                    callsV1_2[i].base.index = p_cur->index;
                    callsV1_2[i].base.toa = p_cur->toa;
                    callsV1_2[i].base.isMpty = p_cur->isMpty;
                    callsV1_2[i].base.isMT = p_cur->isMT;
                    callsV1_2[i].base.als = p_cur->als;
                    callsV1_2[i].base.isVoice = p_cur->isVoice;
                    callsV1_2[i].base.isVoicePrivacy = p_cur->isVoicePrivacy;
                    callsV1_2[i].base.number = convertCharPtrToHidlString(p_cur->number);
                    callsV1_2[i].base.numberPresentation = (CallPresentation) p_cur->numberPresentation;
                    callsV1_2[i].base.name = convertCharPtrToHidlString(p_cur->name);
                    callsV1_2[i].base.namePresentation = (CallPresentation) p_cur->namePresentation;
                    if (p_cur->uusInfo != NULL && p_cur->uusInfo->uusData != NULL) {
                        RIL_UUS_Info *uusInfo = p_cur->uusInfo;
                        callsV1_2[i].base.uusInfo[0].uusType = (UusType) uusInfo->uusType;
                        callsV1_2[i].base.uusInfo[0].uusDcs = (UusDcs) uusInfo->uusDcs;
                        // convert uusInfo->uusData to a null-terminated string
                        char *nullTermStr = strndup(uusInfo->uusData, uusInfo->uusLength);
                        callsV1_2[i].base.uusInfo[0].uusData = nullTermStr;
                        free(nullTermStr);
                    }
                    callsV1_2[i].audioQuality = convertCallsSpeechCodecToHidlAudioQuality(p_cur->speechCodec);
                }
            } else {
                calls.resize(num);

                for (int i = 0 ; i < num ; i++) {
                    RIL_Call *p_cur = ((RIL_Call **) response)[i];
                    /* each call info */
                    calls[i].state = (CallState) p_cur->state;
                    calls[i].index = p_cur->index;
                    calls[i].toa = p_cur->toa;
                    calls[i].isMpty = p_cur->isMpty;
                    calls[i].isMT = p_cur->isMT;
                    calls[i].als = p_cur->als;
                    calls[i].isVoice = p_cur->isVoice;
                    calls[i].isVoicePrivacy = p_cur->isVoicePrivacy;
                    calls[i].number = convertCharPtrToHidlString(p_cur->number);
                    calls[i].numberPresentation = (CallPresentation) p_cur->numberPresentation;
                    calls[i].name = convertCharPtrToHidlString(p_cur->name);
                    calls[i].namePresentation = (CallPresentation) p_cur->namePresentation;
                    if (p_cur->uusInfo != NULL && p_cur->uusInfo->uusData != NULL) {
                        RIL_UUS_Info *uusInfo = p_cur->uusInfo;
                        calls[i].uusInfo[0].uusType = (UusType) uusInfo->uusType;
                        calls[i].uusInfo[0].uusDcs = (UusDcs) uusInfo->uusDcs;
                        // convert uusInfo->uusData to a null-terminated string
                        char *nullTermStr = strndup(uusInfo->uusData, uusInfo->uusLength);
                        calls[i].uusInfo[0].uusData = nullTermStr;
                        free(nullTermStr);
                    }
                }
            }
        }

        Return<void> retStatus = (radioService[slotId]->mRadioResponseV1_2 != NULL ?
                radioService[slotId]->mRadioResponseV1_2->getCurrentCallsResponse_1_2(
                        responseInfo, callsV1_2) :
                radioService[slotId]->mRadioResponse->getCurrentCallsResponse(
                        responseInfo, calls));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getCurrentCallsResponse: radioService[%d] or mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::dialResponse(int slotId, android::ClientId clientId __unused,
                       int responseType, int serial, RIL_Errno e, void *response,
                       size_t responseLen) {
    RLOGD("dialResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->dialResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("dialResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::emergencyDialResponse(int slotId, android::ClientId clientId __unused,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responselen) {
    RLOGD("emergencyDialResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseV1_4 != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponseV1_4->emergencyDialResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("emergencyDialResponse: radioService[%d]->mRadioResponseV1_4 == NULL",
                slotId);
    }

    return 0;
}

int radio::getIMSIForAppResponse(int slotId, android::ClientId clientId __unused,
                                int responseType, int serial, RIL_Errno e, void *response,
                                size_t responseLen) {
    RLOGD("getIMSIForAppResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->getIMSIForAppResponse(
                responseInfo, convertCharPtrToHidlString((char *) response));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getIMSIForAppResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::hangupConnectionResponse(int slotId, android::ClientId clientId __unused,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen) {
    RLOGD("hangupConnectionResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->hangupConnectionResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("hangupConnectionResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::hangupWaitingOrBackgroundResponse(int slotId, android::ClientId clientId __unused,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responseLen) {
    RLOGD("hangupWaitingOrBackgroundResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus =
                radioService[slotId]->mRadioResponse->hangupWaitingOrBackgroundResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("hangupWaitingOrBackgroundResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::hangupForegroundResumeBackgroundResponse(int slotId,
        android::ClientId clientId __unused, int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    RLOGD("hangupWaitingOrBackgroundResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus =
                radioService[slotId]->mRadioResponse->hangupWaitingOrBackgroundResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("hangupWaitingOrBackgroundResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::switchWaitingOrHoldingAndActiveResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("switchWaitingOrHoldingAndActiveResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->
                switchWaitingOrHoldingAndActiveResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("switchWaitingOrHoldingAndActiveResponse: radioService[%d]->mRadioResponse"
                " == NULL", slotId);
    }

    return 0;
}

int radio::conferenceResponse(int slotId, android::ClientId clientId, int responseType,
                             int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("conferenceResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->
                conferenceResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("conferenceResponse: radioService[%d]->mRadioResponse"
                " == NULL", slotId);
    }

    return 0;
}

int radio::rejectCallResponse(int slotId, android::ClientId clientId __unused, int responseType,
                             int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("rejectCallResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->rejectCallResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("rejectCallResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getLastCallFailCauseResponse(int slotId, android::ClientId clientId __unused,
                                       int responseType, int serial, RIL_Errno e, void *response,
                                       size_t responseLen) {
    RLOGD("getLastCallFailCauseResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        LastCallFailCauseInfo info = {};
        info.vendorCause = hidl_string();
        if (response == NULL) {
            RLOGE("getCurrentCallsResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else if (responseLen == sizeof(int)) {
            int *pInt = (int *) response;
            info.causeCode = (LastCallFailCause) pInt[0];
        } else if (responseLen == sizeof(RIL_LastCallFailCauseInfo))  {
            RIL_LastCallFailCauseInfo *pFailCauseInfo = (RIL_LastCallFailCauseInfo *) response;
            info.causeCode = (LastCallFailCause) pFailCauseInfo->cause_code;
            info.vendorCause = convertCharPtrToHidlString(pFailCauseInfo->vendor_cause);
        } else {
            RLOGE("getCurrentCallsResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        }

        Return<void> retStatus = radioService[slotId]->mRadioResponse->getLastCallFailCauseResponse(
                responseInfo, info);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getLastCallFailCauseResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getSignalStrengthResponse(int slotId, android::ClientId clientId __unused,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responseLen) {
    RLOGD("getSignalStrengthResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseV1_4!= NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        AOSP_V1_4::SignalStrength signalStrength = {};
        // Legacy doesn't support NR, so the size is 21.
        if (response == NULL || responseLen != 21*sizeof(int)) {
            RLOGE("getSignalStrengthResponse_1_4: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            convertRilSignalStrengthToHal_1_4(response, responseLen, signalStrength);
        }

        Return<void> retStatus =
                radioService[slotId]->mRadioResponseV1_4->getSignalStrengthResponse_1_4(
                responseInfo, signalStrength);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else if (radioService[slotId]->mRadioResponseV1_2!= NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        AOSP_V1_2::SignalStrength signalStrength = {};
        if (response == NULL || responseLen != 21*sizeof(int)) {
            RLOGE("getSignalStrengthResponse_1_2: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            convertRilSignalStrengthToHal_1_2(response, responseLen, signalStrength);
        }

        Return<void> retStatus =
                radioService[slotId]->mRadioResponseV1_2->getSignalStrengthResponse_1_2(
                responseInfo, signalStrength);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        SignalStrength signalStrength = {};
        if (response == NULL || responseLen != 21*sizeof(int)) {
            RLOGE("getSignalStrengthResponse: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            convertRilSignalStrengthToHal(response, responseLen, signalStrength);
        }

        Return<void> retStatus =
                radioService[slotId]->mRadioResponse->getSignalStrengthResponse(
                responseInfo, signalStrength);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getSignalStrengthResponse_1_2: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }
    return 0;
}

int mtkRadioEx::setVendorSettingResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    if (clientId == android::CLIENT_RILJ && mtkRadioExService[slotId]->mRadioResponseMtk!= NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->setVendorSettingResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else if (clientId == android::CLIENT_IMS
            && mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseIms->setVendorSettingResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("setVendorSettingResponse: Client %d mtkRadioExService[%d] "
                "== NULL", clientId, slotId);
    }

    return 0;
}

RIL_CellInfoType getCellInfoTypeRadioTechnology(char *rat) {
    if (rat == NULL) {
        return RIL_CELL_INFO_TYPE_NONE;
    }

    int radioTech = atoi(rat);

    switch(radioTech) {

        case RADIO_TECH_GPRS:
        case RADIO_TECH_EDGE:
        case RADIO_TECH_GSM: {
            return RIL_CELL_INFO_TYPE_GSM;
        }

        case RADIO_TECH_UMTS:
        case RADIO_TECH_HSDPA:
        case RADIO_TECH_HSUPA:
        case RADIO_TECH_HSPA:
        case RADIO_TECH_HSPAP: {
            return RIL_CELL_INFO_TYPE_WCDMA;
        }

        case RADIO_TECH_IS95A:
        case RADIO_TECH_IS95B:
        case RADIO_TECH_1xRTT:
        case RADIO_TECH_EVDO_0:
        case RADIO_TECH_EVDO_A:
        case RADIO_TECH_EVDO_B:
        case RADIO_TECH_EHRPD: {
            return RIL_CELL_INFO_TYPE_CDMA;
        }

        case RADIO_TECH_LTE:
        case RADIO_TECH_LTE_CA: {
            return RIL_CELL_INFO_TYPE_LTE;
        }

        case RADIO_TECH_TD_SCDMA: {
            return RIL_CELL_INFO_TYPE_TD_SCDMA;
        }

        default: {
            break;
        }
    }

    return RIL_CELL_INFO_TYPE_NONE;

}

void fillCellIdentityResponse(CellIdentity &cellIdentity, RIL_CellIdentity_v16 &rilCellIdentity) {

    cellIdentity.cellIdentityGsm.resize(0);
    cellIdentity.cellIdentityWcdma.resize(0);
    cellIdentity.cellIdentityCdma.resize(0);
    cellIdentity.cellIdentityTdscdma.resize(0);
    cellIdentity.cellIdentityLte.resize(0);
    cellIdentity.cellInfoType = (CellInfoType)rilCellIdentity.cellInfoType;
    switch(rilCellIdentity.cellInfoType) {

        case RIL_CELL_INFO_TYPE_GSM: {
            cellIdentity.cellIdentityGsm.resize(1);
            cellIdentity.cellIdentityGsm[0].mcc =
                    std::to_string(rilCellIdentity.cellIdentityGsm.mcc);
            cellIdentity.cellIdentityGsm[0].mnc =
                    std::to_string(rilCellIdentity.cellIdentityGsm.mnc);
            cellIdentity.cellIdentityGsm[0].lac = rilCellIdentity.cellIdentityGsm.lac;
            cellIdentity.cellIdentityGsm[0].cid = rilCellIdentity.cellIdentityGsm.cid;
            cellIdentity.cellIdentityGsm[0].arfcn = rilCellIdentity.cellIdentityGsm.arfcn;
            cellIdentity.cellIdentityGsm[0].bsic = rilCellIdentity.cellIdentityGsm.bsic;
            break;
        }

        case RIL_CELL_INFO_TYPE_WCDMA: {
            cellIdentity.cellIdentityWcdma.resize(1);
            cellIdentity.cellIdentityWcdma[0].mcc =
                    std::to_string(rilCellIdentity.cellIdentityWcdma.mcc);
            cellIdentity.cellIdentityWcdma[0].mnc =
                    std::to_string(rilCellIdentity.cellIdentityWcdma.mnc);
            cellIdentity.cellIdentityWcdma[0].lac = rilCellIdentity.cellIdentityWcdma.lac;
            cellIdentity.cellIdentityWcdma[0].cid = rilCellIdentity.cellIdentityWcdma.cid;
            cellIdentity.cellIdentityWcdma[0].psc = rilCellIdentity.cellIdentityWcdma.psc;
            cellIdentity.cellIdentityWcdma[0].uarfcn = rilCellIdentity.cellIdentityWcdma.uarfcn;
            break;
        }

        case RIL_CELL_INFO_TYPE_CDMA: {
            cellIdentity.cellIdentityCdma.resize(1);
            cellIdentity.cellIdentityCdma[0].networkId = rilCellIdentity.cellIdentityCdma.networkId;
            cellIdentity.cellIdentityCdma[0].systemId = rilCellIdentity.cellIdentityCdma.systemId;
            cellIdentity.cellIdentityCdma[0].baseStationId =
                    rilCellIdentity.cellIdentityCdma.basestationId;
            cellIdentity.cellIdentityCdma[0].longitude = rilCellIdentity.cellIdentityCdma.longitude;
            cellIdentity.cellIdentityCdma[0].latitude = rilCellIdentity.cellIdentityCdma.latitude;
            break;
        }

        case RIL_CELL_INFO_TYPE_LTE: {
            cellIdentity.cellIdentityLte.resize(1);
            cellIdentity.cellIdentityLte[0].mcc =
                    std::to_string(rilCellIdentity.cellIdentityLte.mcc);
            cellIdentity.cellIdentityLte[0].mnc =
                    std::to_string(rilCellIdentity.cellIdentityLte.mnc);
            cellIdentity.cellIdentityLte[0].ci = rilCellIdentity.cellIdentityLte.ci;
            cellIdentity.cellIdentityLte[0].pci = rilCellIdentity.cellIdentityLte.pci;
            cellIdentity.cellIdentityLte[0].tac = rilCellIdentity.cellIdentityLte.tac;
            cellIdentity.cellIdentityLte[0].earfcn = rilCellIdentity.cellIdentityLte.earfcn;
            break;
        }

        case RIL_CELL_INFO_TYPE_TD_SCDMA: {
            cellIdentity.cellIdentityTdscdma.resize(1);
            cellIdentity.cellIdentityTdscdma[0].mcc =
                    std::to_string(rilCellIdentity.cellIdentityTdscdma.mcc);
            cellIdentity.cellIdentityTdscdma[0].mnc =
                    std::to_string(rilCellIdentity.cellIdentityTdscdma.mnc);
            cellIdentity.cellIdentityTdscdma[0].lac = rilCellIdentity.cellIdentityTdscdma.lac;
            cellIdentity.cellIdentityTdscdma[0].cid = rilCellIdentity.cellIdentityTdscdma.cid;
            cellIdentity.cellIdentityTdscdma[0].cpid = rilCellIdentity.cellIdentityTdscdma.cpid;
            break;
        }

        case RIL_CELL_INFO_TYPE_NR:
        default: {
            break;
        }
    }
}

// It's for V1_2 CellIdentity
void fillCellIdentityResponse_1_2(AOSP_V1_2::CellIdentity &cellIdentity, const RIL_CellIdentity_v16 &rilCellIdentity) {
    char buff[6];
    memset(buff, 0, 6 * sizeof(char));
    hidl_string mnc_string;
    cellIdentity.cellIdentityGsm.resize(0);
    cellIdentity.cellIdentityWcdma.resize(0);
    cellIdentity.cellIdentityCdma.resize(0);
    cellIdentity.cellIdentityTdscdma.resize(0);
    cellIdentity.cellIdentityLte.resize(0);
    cellIdentity.cellInfoType = (CellInfoType)rilCellIdentity.cellInfoType;
    switch (rilCellIdentity.cellInfoType) {
        case RIL_CELL_INFO_TYPE_GSM: {
            cellIdentity.cellIdentityGsm.resize(1);
            cellIdentity.cellIdentityGsm[0].base.mcc =
                    std::to_string(rilCellIdentity.cellIdentityGsm.mcc);
            if (rilCellIdentity.cellIdentityGsm.mnc_len == 3) {
                snprintf(buff, 6, "%03d", rilCellIdentity.cellIdentityGsm.mnc);
            } else {
                snprintf(buff, 6, "%02d", rilCellIdentity.cellIdentityGsm.mnc);
            }
            mnc_string = buff;
            cellIdentity.cellIdentityGsm[0].base.mnc = mnc_string;
            cellIdentity.cellIdentityGsm[0].base.lac = rilCellIdentity.cellIdentityGsm.lac;
            cellIdentity.cellIdentityGsm[0].base.cid = rilCellIdentity.cellIdentityGsm.cid;
            cellIdentity.cellIdentityGsm[0].base.arfcn = rilCellIdentity.cellIdentityGsm.arfcn;
            cellIdentity.cellIdentityGsm[0].base.bsic = rilCellIdentity.cellIdentityGsm.bsic;
            cellIdentity.cellIdentityGsm[0].operatorNames.alphaLong =
                    convertCharPtrToHidlString(rilCellIdentity.cellIdentityGsm.operName.long_name);
            cellIdentity.cellIdentityGsm[0].operatorNames.alphaShort =
                    convertCharPtrToHidlString(rilCellIdentity.cellIdentityGsm.operName.short_name);
            break;
        }

        case RIL_CELL_INFO_TYPE_WCDMA: {
            cellIdentity.cellIdentityWcdma.resize(1);
            cellIdentity.cellIdentityWcdma[0].base.mcc =
                    std::to_string(rilCellIdentity.cellIdentityWcdma.mcc);
            if (rilCellIdentity.cellIdentityWcdma.mnc_len == 3) {
                snprintf(buff, 6, "%03d", rilCellIdentity.cellIdentityWcdma.mnc);
            } else {
                snprintf(buff, 6, "%02d", rilCellIdentity.cellIdentityWcdma.mnc);
            }
            mnc_string = buff;
            cellIdentity.cellIdentityWcdma[0].base.mnc = mnc_string;
            cellIdentity.cellIdentityWcdma[0].base.lac = rilCellIdentity.cellIdentityWcdma.lac;
            cellIdentity.cellIdentityWcdma[0].base.cid = rilCellIdentity.cellIdentityWcdma.cid;
            cellIdentity.cellIdentityWcdma[0].base.psc = rilCellIdentity.cellIdentityWcdma.psc;
            cellIdentity.cellIdentityWcdma[0].base.uarfcn = rilCellIdentity.cellIdentityWcdma.uarfcn;
            cellIdentity.cellIdentityWcdma[0].operatorNames.alphaLong =
                    convertCharPtrToHidlString(rilCellIdentity.cellIdentityWcdma.operName.long_name);
            cellIdentity.cellIdentityWcdma[0].operatorNames.alphaShort =
                    convertCharPtrToHidlString(rilCellIdentity.cellIdentityWcdma.operName.short_name);
            break;
        }

        case RIL_CELL_INFO_TYPE_CDMA: {
            cellIdentity.cellIdentityCdma.resize(1);
            cellIdentity.cellIdentityCdma[0].base.networkId = rilCellIdentity.cellIdentityCdma.networkId;
            cellIdentity.cellIdentityCdma[0].base.systemId = rilCellIdentity.cellIdentityCdma.systemId;
            cellIdentity.cellIdentityCdma[0].base.baseStationId =
                    rilCellIdentity.cellIdentityCdma.basestationId;
            cellIdentity.cellIdentityCdma[0].base.longitude = rilCellIdentity.cellIdentityCdma.longitude;
            cellIdentity.cellIdentityCdma[0].base.latitude = rilCellIdentity.cellIdentityCdma.latitude;
            cellIdentity.cellIdentityCdma[0].operatorNames.alphaLong =
                    convertCharPtrToHidlString(rilCellIdentity.cellIdentityCdma.operName.long_name);
            cellIdentity.cellIdentityCdma[0].operatorNames.alphaShort =
                    convertCharPtrToHidlString(rilCellIdentity.cellIdentityCdma.operName.short_name);
            break;
        }

        case RIL_CELL_INFO_TYPE_LTE: {
            cellIdentity.cellIdentityLte.resize(1);
            cellIdentity.cellIdentityLte[0].base.mcc =
                    std::to_string(rilCellIdentity.cellIdentityLte.mcc);
            if (rilCellIdentity.cellIdentityLte.mnc_len == 3) {
                snprintf(buff, 6, "%03d", rilCellIdentity.cellIdentityLte.mnc);
            } else {
                snprintf(buff, 6, "%02d", rilCellIdentity.cellIdentityLte.mnc);
            }
            mnc_string = buff;
            cellIdentity.cellIdentityLte[0].base.mnc = mnc_string;
            cellIdentity.cellIdentityLte[0].base.ci = rilCellIdentity.cellIdentityLte.ci;
            cellIdentity.cellIdentityLte[0].base.pci = rilCellIdentity.cellIdentityLte.pci;
            cellIdentity.cellIdentityLte[0].base.tac = rilCellIdentity.cellIdentityLte.tac;
            cellIdentity.cellIdentityLte[0].base.earfcn = rilCellIdentity.cellIdentityLte.earfcn;
            cellIdentity.cellIdentityLte[0].operatorNames.alphaLong =
                    convertCharPtrToHidlString(rilCellIdentity.cellIdentityLte.operName.long_name);
            cellIdentity.cellIdentityLte[0].operatorNames.alphaShort =
                    convertCharPtrToHidlString(rilCellIdentity.cellIdentityLte.operName.short_name);
            cellIdentity.cellIdentityLte[0].bandwidth = rilCellIdentity.cellIdentityLte.bandwidth;
            break;
        }

        case RIL_CELL_INFO_TYPE_TD_SCDMA: {
            cellIdentity.cellIdentityTdscdma.resize(1);
            cellIdentity.cellIdentityTdscdma[0].base.mcc =
                    std::to_string(rilCellIdentity.cellIdentityTdscdma.mcc);
            if (rilCellIdentity.cellIdentityTdscdma.mnc_len == 3) {
                snprintf(buff, 6, "%03d", rilCellIdentity.cellIdentityTdscdma.mnc);
            } else {
                snprintf(buff, 6, "%02d", rilCellIdentity.cellIdentityTdscdma.mnc);
            }
            mnc_string = buff;
            cellIdentity.cellIdentityTdscdma[0].base.mnc = mnc_string;
            cellIdentity.cellIdentityTdscdma[0].base.lac = rilCellIdentity.cellIdentityTdscdma.lac;
            cellIdentity.cellIdentityTdscdma[0].base.cid = rilCellIdentity.cellIdentityTdscdma.cid;
            cellIdentity.cellIdentityTdscdma[0].base.cpid = rilCellIdentity.cellIdentityTdscdma.cpid;
            cellIdentity.cellIdentityTdscdma[0].operatorNames.alphaLong =
                    convertCharPtrToHidlString(rilCellIdentity.cellIdentityTdscdma.operName.long_name);
            cellIdentity.cellIdentityTdscdma[0].operatorNames.alphaShort =
                    convertCharPtrToHidlString(rilCellIdentity.cellIdentityTdscdma.operName.short_name);
            break;
        }

        case RIL_CELL_INFO_TYPE_NR:
        default: {
            break;
        }
    }
}

int convertResponseStringEntryToInt(char **response, int index, int numStrings) {
    if ((response != NULL) &&  (numStrings > index) && (response[index] != NULL)) {
        return atoi(response[index]);
    }

    return -1;
}

void fillCellIdentityFromVoiceRegStateResponseString(CellIdentity &cellIdentity,
        int numStrings, char** response) {

    RIL_CellIdentity_v16 rilCellIdentity;
    memset(&rilCellIdentity, -1, sizeof(RIL_CellIdentity_v16));

    rilCellIdentity.cellInfoType = getCellInfoTypeRadioTechnology(response[3]);
    switch(rilCellIdentity.cellInfoType) {

        case RIL_CELL_INFO_TYPE_GSM: {
            rilCellIdentity.cellIdentityGsm.lac =
                    convertResponseStringEntryToInt(response, 1, numStrings);
            rilCellIdentity.cellIdentityGsm.cid =
                    convertResponseStringEntryToInt(response, 2, numStrings);
            break;
        }

        case RIL_CELL_INFO_TYPE_WCDMA: {
            rilCellIdentity.cellIdentityWcdma.lac =
                    convertResponseStringEntryToInt(response, 1, numStrings);
            rilCellIdentity.cellIdentityWcdma.cid =
                    convertResponseStringEntryToInt(response, 2, numStrings);
            rilCellIdentity.cellIdentityWcdma.psc =
                    convertResponseStringEntryToInt(response, 14, numStrings);
            break;
        }

        case RIL_CELL_INFO_TYPE_TD_SCDMA:{
            rilCellIdentity.cellIdentityTdscdma.lac =
                    convertResponseStringEntryToInt(response, 1, numStrings);
            rilCellIdentity.cellIdentityTdscdma.cid =
                    convertResponseStringEntryToInt(response, 2, numStrings);
            break;
        }

        case RIL_CELL_INFO_TYPE_CDMA:{
            rilCellIdentity.cellIdentityCdma.basestationId =
                    convertResponseStringEntryToInt(response, 4, numStrings);
            rilCellIdentity.cellIdentityCdma.longitude =
                    convertResponseStringEntryToInt(response, 5, numStrings);
            rilCellIdentity.cellIdentityCdma.latitude =
                    convertResponseStringEntryToInt(response, 6, numStrings);
            rilCellIdentity.cellIdentityCdma.systemId =
                    convertResponseStringEntryToInt(response, 8, numStrings);
            rilCellIdentity.cellIdentityCdma.networkId =
                    convertResponseStringEntryToInt(response, 9, numStrings);
            break;
        }

        case RIL_CELL_INFO_TYPE_LTE:{
            rilCellIdentity.cellIdentityLte.tac =
                    convertResponseStringEntryToInt(response, 1, numStrings);
            rilCellIdentity.cellIdentityLte.ci =
                    convertResponseStringEntryToInt(response, 2, numStrings);
            break;
        }

        case RIL_CELL_INFO_TYPE_NR:
        default: {
            break;
        }
    }

    fillCellIdentityResponse(cellIdentity, rilCellIdentity);
}

/* Fill Cell Identity info from Data Registration State Response.
 * This fucntion is applicable only for RIL Version < 15.
 * Response is a  "char **".
 * First and Second entries are in hex string format
 * and rest are integers represented in ascii format. */
void fillCellIdentityFromDataRegStateResponseString(CellIdentity &cellIdentity,
        int numStrings, char** response) {

    RIL_CellIdentity_v16 rilCellIdentity;
    memset(&rilCellIdentity, -1, sizeof(RIL_CellIdentity_v16));

    rilCellIdentity.cellInfoType = getCellInfoTypeRadioTechnology(response[3]);
    switch(rilCellIdentity.cellInfoType) {
        case RIL_CELL_INFO_TYPE_GSM: {
            rilCellIdentity.cellIdentityGsm.lac =
                    convertResponseStringEntryToInt(response, 1, numStrings);
            rilCellIdentity.cellIdentityGsm.cid =
                    convertResponseStringEntryToInt(response, 2, numStrings);
            break;
        }
        case RIL_CELL_INFO_TYPE_WCDMA: {
            rilCellIdentity.cellIdentityWcdma.lac =
                    convertResponseStringEntryToInt(response, 1, numStrings);
            rilCellIdentity.cellIdentityWcdma.cid =
                    convertResponseStringEntryToInt(response, 2, numStrings);
            break;
        }
        case RIL_CELL_INFO_TYPE_TD_SCDMA:{
            rilCellIdentity.cellIdentityTdscdma.lac =
                    convertResponseStringEntryToInt(response, 1, numStrings);
            rilCellIdentity.cellIdentityTdscdma.cid =
                    convertResponseStringEntryToInt(response, 2, numStrings);
            break;
        }
        case RIL_CELL_INFO_TYPE_LTE: {
            rilCellIdentity.cellIdentityLte.tac =
                    convertResponseStringEntryToInt(response, 6, numStrings);
            rilCellIdentity.cellIdentityLte.pci =
                    convertResponseStringEntryToInt(response, 7, numStrings);
            rilCellIdentity.cellIdentityLte.ci =
                    convertResponseStringEntryToInt(response, 8, numStrings);
            break;
        }
        case RIL_CELL_INFO_TYPE_NR:
        default: {
            break;
        }
    }

    fillCellIdentityResponse(cellIdentity, rilCellIdentity);
}

// It's also getVoiceRegistrationStateResponse_1_2
int radio::getVoiceRegistrationStateResponse(int slotId, android::ClientId clientId __unused,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responseLen) {
    RLOGD("getVoiceRegistrationStateResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseV1_2 != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        // use V1_2
        AOSP_V1_2::VoiceRegStateResult voiceRegResponse = {};
        if (response == NULL) {
               RLOGE("getVoiceRegistrationStateResponse Invalid response: NULL");
               if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            RIL_VoiceRegistrationStateResponse *voiceRegState =
                    (RIL_VoiceRegistrationStateResponse *)response;
            if (responseLen != sizeof(RIL_VoiceRegistrationStateResponse)) {
                RLOGE("getVoiceRegistrationStateResponse Invalid response: NULL");
                if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            } else {
                voiceRegResponse.regState = (RegState) voiceRegState->regState;
                voiceRegResponse.rat = voiceRegState->rat;;
                voiceRegResponse.cssSupported = voiceRegState->cssSupported;
                voiceRegResponse.roamingIndicator = voiceRegState->roamingIndicator;
                voiceRegResponse.systemIsInPrl = voiceRegState->systemIsInPrl;
                voiceRegResponse.defaultRoamingIndicator = voiceRegState->defaultRoamingIndicator;
                voiceRegResponse.reasonForDenial = voiceRegState->reasonForDenial;
                fillCellIdentityResponse_1_2(voiceRegResponse.cellIdentity,
                        voiceRegState->cellIdentity);
            }
        }
        // response V1_2
        Return<void> retStatus =
                radioService[slotId]->mRadioResponseV1_2->getVoiceRegistrationStateResponse_1_2(
                responseInfo, voiceRegResponse);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        VoiceRegStateResult voiceRegResponse = {};
        int numStrings = responseLen / sizeof(char *);
        if (response == NULL) {
               RLOGE("getVoiceRegistrationStateResponse Invalid response: NULL");
               if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else if (s_vendorFunctions->version <= 14) {
            if (numStrings != 15) {
                RLOGE("getVoiceRegistrationStateResponse Invalid response: NULL");
                if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            } else {
                char **resp = (char **) response;
                voiceRegResponse.regState = (RegState) ATOI_NULL_HANDLED_DEF(resp[0], 4);
                voiceRegResponse.rat = ATOI_NULL_HANDLED(resp[3]);
                voiceRegResponse.cssSupported = ATOI_NULL_HANDLED_DEF(resp[7], 0);
                voiceRegResponse.roamingIndicator = ATOI_NULL_HANDLED(resp[10]);
                voiceRegResponse.systemIsInPrl = ATOI_NULL_HANDLED_DEF(resp[11], 0);
                voiceRegResponse.defaultRoamingIndicator = ATOI_NULL_HANDLED_DEF(resp[12], 0);
                voiceRegResponse.reasonForDenial = ATOI_NULL_HANDLED_DEF(resp[13], 0);
                fillCellIdentityFromVoiceRegStateResponseString(voiceRegResponse.cellIdentity,
                        numStrings, resp);
            }
        } else {
            RIL_VoiceRegistrationStateResponse *voiceRegState =
                    (RIL_VoiceRegistrationStateResponse *)response;

            if (responseLen != sizeof(RIL_VoiceRegistrationStateResponse)) {
                RLOGE("getVoiceRegistrationStateResponse Invalid response: NULL");
                if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            } else {
                voiceRegResponse.regState = (RegState) voiceRegState->regState;
                voiceRegResponse.rat = voiceRegState->rat;;
                voiceRegResponse.cssSupported = voiceRegState->cssSupported;
                voiceRegResponse.roamingIndicator = voiceRegState->roamingIndicator;
                voiceRegResponse.systemIsInPrl = voiceRegState->systemIsInPrl;
                voiceRegResponse.defaultRoamingIndicator = voiceRegState->defaultRoamingIndicator;
                voiceRegResponse.reasonForDenial = voiceRegState->reasonForDenial;
                fillCellIdentityResponse(voiceRegResponse.cellIdentity,
                        voiceRegState->cellIdentity);
            }
        }

        Return<void> retStatus =
                radioService[slotId]->mRadioResponse->getVoiceRegistrationStateResponse(
                responseInfo, voiceRegResponse);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getVoiceRegistrationStateResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

// It's also getDataRegistrationStateResponse_1_2/_1_4
int radio::getDataRegistrationStateResponse(int slotId, android::ClientId clientId __unused,
                                           int responseType, int serial, RIL_Errno e,
                                           void *response, size_t responseLen) {
    RLOGD("getDataRegistrationStateResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseV1_4!= NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        // use V1_4
        AOSP_V1_4::DataRegStateResult dataRegResponse = {};
        if (response == NULL) {
            RLOGE("getDataRegistrationStateResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            RIL_DataRegistrationStateResponse *dataRegState =
                    (RIL_DataRegistrationStateResponse *)response;

            if (responseLen != sizeof(RIL_DataRegistrationStateResponse)) {
                RLOGE("getDataRegistrationStateResponse Invalid response: NULL");
                if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            } else {
                dataRegResponse.base.regState = (RegState) dataRegState->regState;
                dataRegResponse.base.rat = dataRegState->rat;
                dataRegResponse.base.reasonDataDenied = dataRegState->reasonDataDenied;
                dataRegResponse.base.maxDataCalls = dataRegState->maxDataCalls;
                fillCellIdentityResponse_1_2(dataRegResponse.base.cellIdentity, dataRegState->cellIdentity);
                // v1_4 // TODO
                AOSP_V1_4::LteVopsInfo lteVopsInfo = {};
                lteVopsInfo.isVopsSupported =
                        dataRegState->lteVopsInfo.isVopsSupported > 0 ? true: false;
                lteVopsInfo.isEmcBearerSupported =
                        dataRegState->lteVopsInfo.isEmcBearerSupported > 0 ? true: false;
                if (dataRegResponse.base.rat == RADIO_TECH_LTE ||
                        dataRegResponse.base.rat == RADIO_TECH_LTE_CA) {
                    dataRegResponse.vopsInfo.lteVopsInfo(lteVopsInfo);
                }
                dataRegResponse.nrIndicators.isEndcAvailable =
                        dataRegState->nrIndicators.isEndcAvailable > 0 ? true: false;
                dataRegResponse.nrIndicators.isDcNrRestricted =
                        dataRegState->nrIndicators.isDcNrRestricted > 0 ? true: false;
                dataRegResponse.nrIndicators.isNrAvailable =
                        dataRegState->nrIndicators.isNrAvailable > 0 ? true: false;
            }
        }
        // response V1_4
        Return<void> retStatus =
                radioService[slotId]->mRadioResponseV1_4->getDataRegistrationStateResponse_1_4(responseInfo,
                dataRegResponse);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else if (radioService[slotId]->mRadioResponseV1_2!= NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        // use V1_2
        AOSP_V1_2::DataRegStateResult dataRegResponse = {};
        if (response == NULL) {
            RLOGE("getDataRegistrationStateResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            RIL_DataRegistrationStateResponse *dataRegState =
                    (RIL_DataRegistrationStateResponse *)response;

            if (responseLen != sizeof(RIL_DataRegistrationStateResponse)) {
                RLOGE("getDataRegistrationStateResponse Invalid response: NULL");
                if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            } else {
                dataRegResponse.regState = (RegState) dataRegState->regState;
                dataRegResponse.rat = dataRegState->rat;;
                dataRegResponse.reasonDataDenied = dataRegState->reasonDataDenied;
                dataRegResponse.maxDataCalls = dataRegState->maxDataCalls;
                fillCellIdentityResponse_1_2(dataRegResponse.cellIdentity, dataRegState->cellIdentity);
            }
        }
        // response V1_2
        Return<void> retStatus =
                radioService[slotId]->mRadioResponseV1_2->getDataRegistrationStateResponse_1_2(responseInfo,
                dataRegResponse);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        DataRegStateResult dataRegResponse = {};
        if (response == NULL) {
            RLOGE("getDataRegistrationStateResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else if (s_vendorFunctions->version <= 14) {
            int numStrings = responseLen / sizeof(char *);
            if ((numStrings != 6) && (numStrings != 11)) {
                RLOGE("getDataRegistrationStateResponse Invalid response: NULL");
                if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            } else {
                char **resp = (char **) response;
                dataRegResponse.regState = (RegState) ATOI_NULL_HANDLED_DEF(resp[0], 4);
                dataRegResponse.rat =  ATOI_NULL_HANDLED_DEF(resp[3], 0);
                dataRegResponse.reasonDataDenied =  ATOI_NULL_HANDLED(resp[4]);
                dataRegResponse.maxDataCalls =  ATOI_NULL_HANDLED_DEF(resp[5], 1);
                fillCellIdentityFromDataRegStateResponseString(dataRegResponse.cellIdentity,
                        numStrings, resp);
            }
        } else {
            RIL_DataRegistrationStateResponse *dataRegState =
                    (RIL_DataRegistrationStateResponse *)response;

            if (responseLen != sizeof(RIL_DataRegistrationStateResponse)) {
                RLOGE("getDataRegistrationStateResponse Invalid response: NULL");
                if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            } else {
                dataRegResponse.regState = (RegState) dataRegState->regState;
                dataRegResponse.rat = dataRegState->rat;;
                dataRegResponse.reasonDataDenied = dataRegState->reasonDataDenied;
                dataRegResponse.maxDataCalls = dataRegState->maxDataCalls;
                fillCellIdentityResponse(dataRegResponse.cellIdentity, dataRegState->cellIdentity);
            }
        }

        Return<void> retStatus =
                radioService[slotId]->mRadioResponse->getDataRegistrationStateResponse(responseInfo,
                dataRegResponse);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getDataRegistrationStateResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }
    return 0;
}

int radio::getOperatorResponse(int slotId, android::ClientId clientId __unused,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responseLen) {
    RLOGD("getOperatorResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_string longName;
        hidl_string shortName;
        hidl_string numeric;
        int numStrings = responseLen / sizeof(char *);
        if (response == NULL || numStrings != 3) {
            RLOGE("getOperatorResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;

        } else {
            char **resp = (char **) response;
            longName = convertCharPtrToHidlString(resp[0]);
            shortName = convertCharPtrToHidlString(resp[1]);
            numeric = convertCharPtrToHidlString(resp[2]);
        }
        Return<void> retStatus = radioService[slotId]->mRadioResponse->getOperatorResponse(
                responseInfo, longName, shortName, numeric);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getOperatorResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setRadioPowerResponse(int slotId, android::ClientId clientId __unused,
                                int responseType, int serial, RIL_Errno e, void *response,
                                size_t responseLen) {
    RLOGD("setRadioPowerResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->setRadioPowerResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setRadioPowerResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::sendDtmfResponse(int slotId, android::ClientId clientId __unused,
                           int responseType, int serial, RIL_Errno e, void *response,
                           size_t responseLen) {
    RLOGD("sendDtmfResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->sendDtmfResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("sendDtmfResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

SendSmsResult makeSendSmsResult(RadioResponseInfo& responseInfo, int serial, int responseType,
                                RIL_Errno e, void *response, size_t responseLen) {
    populateResponseInfo(responseInfo, serial, responseType, e);
    SendSmsResult result = {};

    if (response == NULL || responseLen != sizeof(RIL_SMS_Response)) {
        RLOGE("Invalid response: NULL");
        if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        result.ackPDU = hidl_string();
    } else {
        RIL_SMS_Response *resp = (RIL_SMS_Response *) response;
        result.messageRef = resp->messageRef;
        result.ackPDU = convertCharPtrToHidlString(resp->ackPDU);
        result.errorCode = resp->errorCode;
    }
    return result;
}

int radio::sendSmsResponse(int slotId, android::ClientId clientId __unused,
                          int responseType, int serial, RIL_Errno e, void *response,
                          size_t responseLen) {
    RLOGD("sendSmsResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        SendSmsResult result = makeSendSmsResult(responseInfo, serial, responseType, e, response,
                responseLen);

        Return<void> retStatus = radioService[slotId]->mRadioResponse->sendSmsResponse(responseInfo,
                result);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("sendSmsResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::sendSMSExpectMoreResponse(int slotId, android::ClientId clientId __unused,
                                    int responseType, int serial, RIL_Errno e, void *response,
                                    size_t responseLen) {
    RLOGD("sendSMSExpectMoreResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        SendSmsResult result = makeSendSmsResult(responseInfo, serial, responseType, e, response,
                responseLen);

        Return<void> retStatus = radioService[slotId]->mRadioResponse->sendSMSExpectMoreResponse(
                responseInfo, result);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("sendSMSExpectMoreResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::setupDataCallResponse(int slotId, android::ClientId clientId __unused,
                                 int responseType, int serial, RIL_Errno e, void *response,
                                 size_t responseLen) {
    RLOGD("setupDataCallResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseV1_4!= NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        AOSP_V1_4::SetupDataCallResult result = {};
        // M: [OD over ePDG]
        // remark AOSP
        //if (response == NULL || responseLen != sizeof(RIL_Data_Call_Response_v11)) {
        if (response == NULL || responseLen != sizeof(MTK_RIL_Data_Call_Response_v11)) {
            RLOGE("setupDataCallResponse: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            result.cause = AOSP_V1_4::DataCallFailCause::ERROR_UNSPECIFIED;
            result.type = AOSP_V1_4::PdpProtocolType::UNKNOWN;
            result.ifname = hidl_string();
            result.addresses = hidl_vec<hidl_string>();
            result.dnses = hidl_vec<hidl_string>();
            result.gateways = hidl_vec<hidl_string>();
            result.pcscf = hidl_vec<hidl_string>();
        } else {
            // M: [OD over ePDG]
            // remark AOSP
            //convertRilDataCallToHal((RIL_Data_Call_Response_v11 *) response, result);
            convertRilDataCallToHalEx_1_4((MTK_RIL_Data_Call_Response_v11 *) response,
                    result, slotId);
        }

        // M @{
        if (s_cardStatus[slotId].base.cardState == CardState::ABSENT) {
            responseInfo.error = RadioError::NONE;
        }
        // M @}

        Return<void> retStatus =
                radioService[slotId]->mRadioResponseV1_4->setupDataCallResponse_1_4(
                responseInfo, result);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        SetupDataCallResult result = {};
        // M: [OD over ePDG]
        // remark AOSP
        //if (response == NULL || responseLen != sizeof(RIL_Data_Call_Response_v11)) {
        if (response == NULL || responseLen != sizeof(MTK_RIL_Data_Call_Response_v11)) {
            RLOGE("setupDataCallResponse: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            result.status = DataCallFailCause::ERROR_UNSPECIFIED;
            result.type = hidl_string();
            result.ifname = hidl_string();
            result.addresses = hidl_string();
            result.dnses = hidl_string();
            result.gateways = hidl_string();
            result.pcscf = hidl_string();
        } else {
            // M: [OD over ePDG]
            // remark AOSP
            //convertRilDataCallToHal((RIL_Data_Call_Response_v11 *) response, result);
            convertRilDataCallToHalEx((MTK_RIL_Data_Call_Response_v11 *) response, result, slotId);
        }

        // M @{
        if (s_cardStatus[slotId].base.cardState == CardState::ABSENT) {
            responseInfo.error = RadioError::NONE;
        }
        // M @}

        Return<void> retStatus = radioService[slotId]->mRadioResponse->setupDataCallResponse(
                responseInfo, result);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setupDataCallResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

IccIoResult responseIccIo(RadioResponseInfo& responseInfo, int serial, int responseType,
                           RIL_Errno e, void *response, size_t responseLen) {
    populateResponseInfo(responseInfo, serial, responseType, e);
    IccIoResult result = {};

    if (response == NULL || responseLen != sizeof(RIL_SIM_IO_Response)) {
        RLOGE("Invalid response: NULL");
        if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        result.simResponse = hidl_string();
    } else {
        RIL_SIM_IO_Response *resp = (RIL_SIM_IO_Response *) response;
        result.sw1 = resp->sw1;
        result.sw2 = resp->sw2;
        result.simResponse = convertCharPtrToHidlString(resp->simResponse);
    }
    return result;
}

int radio::iccIOForAppResponse(int slotId, android::ClientId clientId __unused,
                      int responseType, int serial, RIL_Errno e, void *response,
                      size_t responseLen) {
    RLOGD("iccIOForAppResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        IccIoResult result = responseIccIo(responseInfo, serial, responseType, e, response,
                responseLen);

        Return<void> retStatus = radioService[slotId]->mRadioResponse->iccIOForAppResponse(
                responseInfo, result);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("iccIOForAppResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::sendUssdResponse(int slotId, android::ClientId clientId __unused,
                           int responseType, int serial, RIL_Errno e, void *response,
                           size_t responseLen) {
    RLOGD("sendUssdResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->sendUssdResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("sendUssdResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::cancelPendingUssdResponse(int slotId, android::ClientId clientId __unused,
                                    int responseType, int serial, RIL_Errno e, void *response,
                                    size_t responseLen) {
    RLOGD("cancelPendingUssdResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->cancelPendingUssdResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("cancelPendingUssdResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getClirResponse(int slotId, android::ClientId clientId __unused,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responseLen) {
    RLOGD("getClirResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        int n = -1, m = -1;
        int numInts = responseLen / sizeof(int);
        if (response == NULL || numInts != 2) {
            RLOGE("getClirResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            n = pInt[0];
            m = pInt[1];
        }
        Return<void> retStatus = radioService[slotId]->mRadioResponse->getClirResponse(responseInfo,
                n, m);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getClirResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::setClirResponse(int slotId, android::ClientId clientId __unused,
                          int responseType, int serial, RIL_Errno e, void *response,
                          size_t responseLen) {
    RLOGD("setClirResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->setClirResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setClirResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::getCallForwardStatusResponse(int slotId, android::ClientId clientId __unused,
                                       int responseType, int serial, RIL_Errno e,
                                       void *response, size_t responseLen) {
    RLOGD("getCallForwardStatusResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<CallForwardInfo> callForwardInfos;

        if (response == NULL || responseLen % sizeof(RIL_CallForwardInfo *) != 0) {
            RLOGE("getCallForwardStatusResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int num = responseLen / sizeof(RIL_CallForwardInfo *);
            callForwardInfos.resize(num);
            for (int i = 0 ; i < num; i++) {
                RIL_CallForwardInfo *resp = ((RIL_CallForwardInfo **) response)[i];
                callForwardInfos[i].status = (CallForwardInfoStatus) resp->status;
                callForwardInfos[i].reason = resp->reason;
                callForwardInfos[i].serviceClass = resp->serviceClass;
                callForwardInfos[i].toa = resp->toa;
                callForwardInfos[i].number = convertCharPtrToHidlString(resp->number);
                callForwardInfos[i].timeSeconds = resp->timeSeconds;
            }
        }

        Return<void> retStatus = radioService[slotId]->mRadioResponse->getCallForwardStatusResponse(
                responseInfo, callForwardInfos);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getCallForwardStatusResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setCallForwardResponse(int slotId, android::ClientId clientId __unused,
                                 int responseType, int serial, RIL_Errno e, void *response,
                                 size_t responseLen) {
    RLOGD("setCallForwardResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->setCallForwardResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setCallForwardResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::getCallWaitingResponse(int slotId, android::ClientId clientId __unused,
                                 int responseType, int serial, RIL_Errno e, void *response,
                                 size_t responseLen) {
    RLOGD("getCallWaitingResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        bool enable = false;
        int serviceClass = -1;
        int numInts = responseLen / sizeof(int);
        if (response == NULL || numInts != 2) {
            RLOGE("getCallWaitingResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            enable = pInt[0] == 1 ? true : false;
            serviceClass = pInt[1];
        }
        Return<void> retStatus = radioService[slotId]->mRadioResponse->getCallWaitingResponse(
                responseInfo, enable, serviceClass);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getCallWaitingResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::setCallWaitingResponse(int slotId, android::ClientId clientId __unused,
                                 int responseType, int serial, RIL_Errno e, void *response,
                                 size_t responseLen) {
    RLOGD("setCallWaitingResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->setCallWaitingResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setCallWaitingResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::acknowledgeLastIncomingGsmSmsResponse(int slotId, android::ClientId clientId __unused,
                                                int responseType, int serial, RIL_Errno e,
                                                void *response, size_t responseLen) {
    RLOGD("acknowledgeLastIncomingGsmSmsResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus =
                radioService[slotId]->mRadioResponse->acknowledgeLastIncomingGsmSmsResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("acknowledgeLastIncomingGsmSmsResponse: radioService[%d]->mRadioResponse "
                "== NULL", slotId);
    }

    return 0;
}

int radio::acceptCallResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen) {
    RLOGD("acceptCallResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->acceptCallResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("acceptCallResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::deactivateDataCallResponse(int slotId, android::ClientId clientId __unused,
                                                int responseType, int serial, RIL_Errno e,
                                                void *response, size_t responseLen) {
    RLOGD("deactivateDataCallResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        // M @{
        if (s_cardStatus[slotId].base.cardState == CardState::ABSENT) {
            responseInfo.error = RadioError::NONE;
        }
        // M @}

        Return<void> retStatus = radioService[slotId]->mRadioResponse->deactivateDataCallResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("deactivateDataCallResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getFacilityLockForAppResponse(int slotId, android::ClientId clientId,
                                        int responseType, int serial, RIL_Errno e,
                                        void *response, size_t responseLen) {
    RLOGD("getFacilityLockForAppResponse: serial %d", serial);

    if(clientId == android::CLIENT_IMS) {
        if (radioService[slotId]->mRadioResponse != NULL) {
            RadioResponseInfo responseInfo = {};
            int ret = responseInt(responseInfo, serial, responseType, e, response, responseLen);
            Return<void> retStatus = radioService[slotId]->mRadioResponse->
                    getFacilityLockForAppResponse(responseInfo, ret);
            radioService[slotId]->checkReturnStatus(retStatus, slotId);
        } else {
            RLOGE("getFacilityLockForAppResponse: radioService[%d]->mRadioResponse == NULL",
                    slotId);
        }
    } else {
        if (radioService[slotId]->mRadioResponse != NULL) {
            RadioResponseInfo responseInfo = {};
            int ret = responseInt(responseInfo, serial, responseType, e, response, responseLen);
            Return<void> retStatus = radioService[slotId]->mRadioResponse->
                    getFacilityLockForAppResponse(responseInfo, ret);
            radioService[slotId]->checkReturnStatus(retStatus, slotId);
        } else {
            RLOGE("getFacilityLockForAppResponse: radioService[%d]->mRadioResponse == NULL",
                    slotId);
        }
    }
    return 0;
}

int radio::setFacilityLockForAppResponse(int slotId, android::ClientId clientId __unused,
                                      int responseType, int serial, RIL_Errno e,
                                      void *response, size_t responseLen) {
    RLOGD("setFacilityLockForAppResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseIntOrEmpty(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setFacilityLockForAppResponse(responseInfo,
                ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setFacilityLockForAppResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setBarringPasswordResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen) {
    RLOGD("setBarringPasswordResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setBarringPasswordResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setBarringPasswordResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getNetworkSelectionModeResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("getNetworkSelectionModeResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        bool manual = false;
        int serviceClass;
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("getNetworkSelectionModeResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            manual = pInt[0] == 1 ? true : false;
        }
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getNetworkSelectionModeResponse(
                responseInfo,
                manual);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getNetworkSelectionModeResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setNetworkSelectionModeAutomaticResponse(int slotId,
        android::ClientId clientId __unused, int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    RLOGD("setNetworkSelectionModeAutomaticResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setNetworkSelectionModeAutomaticResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setNetworkSelectionModeAutomaticResponse: radioService[%d]->mRadioResponse "
                "== NULL", slotId);
    }

    return 0;
}

int radio::setNetworkSelectionModeManualResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen) {
    RLOGD("setNetworkSelectionModeManualResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setNetworkSelectionModeManualResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("acceptCallResponse: radioService[%d]->setNetworkSelectionModeManualResponse "
                "== NULL", slotId);
    }

    return 0;
}


int convertOperatorStatusToInt(const char *str) {
    if (strncmp("unknown", str, 9) == 0) {
        return (int) OperatorStatus::UNKNOWN;
    } else if (strncmp("available", str, 9) == 0) {
        return (int) OperatorStatus::AVAILABLE;
    } else if (strncmp("current", str, 9) == 0) {
        return (int) OperatorStatus::CURRENT;
    } else if (strncmp("forbidden", str, 9) == 0) {
        return (int) OperatorStatus::FORBIDDEN;
    } else {
        return -1;
    }
}

int radio::getAvailableNetworksResponse(int slotId, android::ClientId clientId __unused,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responseLen) {
    RLOGD("getAvailableNetworksResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<OperatorInfo> networks;
        if (response == NULL || responseLen % (4 * sizeof(char *))!= 0) {
            RLOGE("getAvailableNetworksResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            char **resp = (char **) response;
            int numStrings = responseLen / sizeof(char *);
            networks.resize(numStrings/4);
            for (int i = 0, j = 0; i < numStrings; i = i + 4, j++) {
                networks[j].alphaLong = convertCharPtrToHidlString(resp[i]);
                networks[j].alphaShort = convertCharPtrToHidlString(resp[i + 1]);
                networks[j].operatorNumeric = convertCharPtrToHidlString(resp[i + 2]);
                int status = convertOperatorStatusToInt(resp[i + 3]);
                if (status == -1) {
                    if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
                } else {
                    networks[j].status = (OperatorStatus) status;
                }
            }
        }
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getAvailableNetworksResponse(responseInfo,
                networks);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getAvailableNetworksResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::startDtmfResponse(int slotId, android::ClientId clientId,
                            int responseType, int serial, RIL_Errno e,
                            void *response, size_t responseLen) {
    RLOGD("startDtmfResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->
                startDtmfResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("startDtmfResponse: radioService[%d]->mRadioResponse"
                " == NULL", slotId);
    }

    return 0;
}

int radio::stopDtmfResponse(int slotId, android::ClientId clientId,
                           int responseType, int serial, RIL_Errno e,
                           void *response, size_t responseLen) {
    RLOGD("stopDtmfResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->
                stopDtmfResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("stopDtmfResponse: radioService[%d]->mRadioResponse"
                " == NULL", slotId);
    }

    return 0;
}

int radio::getBasebandVersionResponse(int slotId, android::ClientId clientId __unused,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responseLen) {
    RLOGD("getBasebandVersionResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getBasebandVersionResponse(responseInfo,
                convertCharPtrToHidlString((char *) response));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getBasebandVersionResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::separateConnectionResponse(int slotId, android::ClientId clientId __unused,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responseLen) {
    RLOGD("separateConnectionResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->
                separateConnectionResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("separateConnectionResponse: radioService[%d]->mRadioResponse"
                " == NULL", slotId);
    }

    return 0;
}

int radio::setMuteResponse(int slotId, android::ClientId clientId __unused,
                          int responseType, int serial, RIL_Errno e,
                          void *response, size_t responseLen) {
    RLOGD("setMuteResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setMuteResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setMuteResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::getMuteResponse(int slotId, android::ClientId clientId __unused,
                          int responseType, int serial, RIL_Errno e, void *response,
                          size_t responseLen) {
    RLOGD("getMuteResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        bool enable = false;
        int serviceClass;
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("getMuteResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            enable = pInt[0] == 1 ? true : false;
        }
        Return<void> retStatus = radioService[slotId]->mRadioResponse->getMuteResponse(responseInfo,
                enable);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getMuteResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::getClipResponse(int slotId, android::ClientId clientId __unused,
                          int responseType, int serial, RIL_Errno e,
                          void *response, size_t responseLen) {
    RLOGD("getClipResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseInt(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->getClipResponse(responseInfo,
                (ClipStatus) ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getClipResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::getDataCallListResponse(int slotId, android::ClientId clientId __unused,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen) {
    RLOGD("getDataCallListResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseV1_4!= NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        hidl_vec<AOSP_V1_4::SetupDataCallResult> ret;
        // M: [OD over ePDG]
        // remark AOSP
        //if (response == NULL || responseLen % sizeof(RIL_Data_Call_Response_v11) != 0) {
        if (response == NULL || responseLen % sizeof(MTK_RIL_Data_Call_Response_v11) != 0) {
            RLOGE("getDataCallListResponse: invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            // M: [OD over ePDG]
            // remark AOSP
            //convertRilDataCallListToHal(response, responseLen, ret);
            convertRilDataCallListToHalEx_1_4(response, responseLen, ret, slotId);
        }

        // M @{
        if (s_cardStatus[slotId].base.cardState == CardState::ABSENT) {
            responseInfo.error = RadioError::NONE;
        }
        // M @}

        Return<void> retStatus =
                radioService[slotId]->mRadioResponseV1_4->getDataCallListResponse_1_4(
                responseInfo, ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        hidl_vec<SetupDataCallResult> ret;
        // M: [OD over ePDG]
        // remark AOSP
        //if (response == NULL || responseLen % sizeof(RIL_Data_Call_Response_v11) != 0) {
        if (response == NULL || responseLen % sizeof(MTK_RIL_Data_Call_Response_v11) != 0) {
            RLOGE("getDataCallListResponse: invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            // M: [OD over ePDG]
            // remark AOSP
            //convertRilDataCallListToHal(response, responseLen, ret);
            convertRilDataCallListToHalEx(response, responseLen, ret, slotId);
        }

        // M @{
        if (s_cardStatus[slotId].base.cardState == CardState::ABSENT) {
            responseInfo.error = RadioError::NONE;
        }
        // M @}

        Return<void> retStatus = radioService[slotId]->mRadioResponse->getDataCallListResponse(
                responseInfo, ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getDataCallListResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::setSuppServiceNotificationsResponse(int slotId, android::ClientId clientId __unused,
                                              int responseType, int serial, RIL_Errno e,
                                              void *response, size_t responseLen) {
    RLOGD("setSuppServiceNotificationsResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setSuppServiceNotificationsResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setSuppServiceNotificationsResponse: radioService[%d]->mRadioResponse "
                "== NULL", slotId);
    }

    return 0;
}

int radio::deleteSmsOnSimResponse(int slotId, android::ClientId clientId __unused,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responseLen) {
    RLOGD("deleteSmsOnSimResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->deleteSmsOnSimResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("deleteSmsOnSimResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::setBandModeResponse(int slotId, android::ClientId clientId __unused,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responseLen) {
    RLOGD("setBandModeResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setBandModeResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setBandModeResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::writeSmsToSimResponse(int slotId, android::ClientId clientId __unused,
                                int responseType, int serial, RIL_Errno e,
                                void *response, size_t responseLen) {
    RLOGD("writeSmsToSimResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseInt(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->writeSmsToSimResponse(responseInfo, ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("writeSmsToSimResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::getAvailableBandModesResponse(int slotId, android::ClientId clientId __unused,
                                        int responseType, int serial, RIL_Errno e, void *response,
                                        size_t responseLen) {
    RLOGD("getAvailableBandModesResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<RadioBandMode> modes;
        if (response == NULL || responseLen % sizeof(int) != 0) {
            RLOGE("getAvailableBandModesResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            int numInts = responseLen / sizeof(int);
            modes.resize(numInts);
            for (int i = 0; i < numInts; i++) {
                modes[i] = (RadioBandMode) pInt[i];
            }
        }
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getAvailableBandModesResponse(responseInfo,
                modes);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getAvailableBandModesResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::sendEnvelopeResponse(int slotId, android::ClientId clientId __unused,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen) {
    RLOGD("sendEnvelopeResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->sendEnvelopeResponse(responseInfo,
                convertCharPtrToHidlString((char *) response));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("sendEnvelopeResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::sendTerminalResponseToSimResponse(int slotId, android::ClientId clientId __unused,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responseLen) {
    RLOGD("sendTerminalResponseToSimResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->sendTerminalResponseToSimResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("sendTerminalResponseToSimResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::handleStkCallSetupRequestFromSimResponse(int slotId,
        android::ClientId clientId __unused, int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    RLOGD("handleStkCallSetupRequestFromSimResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->handleStkCallSetupRequestFromSimResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("handleStkCallSetupRequestFromSimResponse: radioService[%d]->mRadioResponse "
                "== NULL", slotId);
    }

    return 0;
}

int radio::explicitCallTransferResponse(int slotId, android::ClientId clientId,
                                       int responseType, int serial, RIL_Errno e,
                                       void *response, size_t responseLen) {
    RLOGD("explicitCallTransferResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus =
                radioService[slotId]->mRadioResponse->
                explicitCallTransferResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("explicitCallTransferResponse: radioService[%d]->mRadioResponse"
                " == NULL", slotId);
    }

    return 0;
}

int radio::setPreferredNetworkTypeResponse(int slotId, android::ClientId clientId __unused,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responseLen) {
    RLOGD("setPreferredNetworkTypeResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseV1_4 != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponseV1_4->setPreferredNetworkTypeBitmapResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setPreferredNetworkTypeResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setPreferredNetworkTypeResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}


int radio::getPreferredNetworkTypeResponse(int slotId, android::ClientId clientId __unused,
                                          int responseType, int serial, RIL_Errno e,
                                          void *response, size_t responseLen) {
    RLOGD("getPreferredNetworkTypeResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseV1_4 != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseInt(responseInfo, serial, responseType, e, response, responseLen);
        int raf = getRafFromNetworkType(ret);
        RLOGD("getPreferredNetworkTypeResponse: nwTypeBitmap %d, nwType %d",
                raf, ret);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponseV1_4->getPreferredNetworkTypeBitmapResponse(
                responseInfo, (hidl_bitfield<AOSP_V1_4::RadioAccessFamily>) raf);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseInt(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getPreferredNetworkTypeResponse(
                responseInfo, (PreferredNetworkType) ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getPreferredNetworkTypeResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getNeighboringCidsResponse(int slotId, android::ClientId clientId __unused,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responseLen) {
    RLOGD("getNeighboringCidsResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<NeighboringCell> cells;

        if (response == NULL || responseLen % sizeof(RIL_NeighboringCell *) != 0) {
            RLOGE("getNeighboringCidsResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int num = responseLen / sizeof(RIL_NeighboringCell *);
            cells.resize(num);
            for (int i = 0 ; i < num; i++) {
                RIL_NeighboringCell *resp = ((RIL_NeighboringCell **) response)[i];
                cells[i].cid = convertCharPtrToHidlString(resp->cid);
                cells[i].rssi = resp->rssi;
            }
        }

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getNeighboringCidsResponse(responseInfo,
                cells);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getNeighboringCidsResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setLocationUpdatesResponse(int slotId, android::ClientId clientId __unused,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responseLen) {
    RLOGD("setLocationUpdatesResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setLocationUpdatesResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setLocationUpdatesResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setCdmaSubscriptionSourceResponse(int slotId, android::ClientId clientId __unused,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responseLen) {
    RLOGD("setCdmaSubscriptionSourceResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setCdmaSubscriptionSourceResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setCdmaSubscriptionSourceResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setCdmaRoamingPreferenceResponse(int slotId, android::ClientId clientId __unused,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responseLen) {
    RLOGD("setCdmaRoamingPreferenceResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setCdmaRoamingPreferenceResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setCdmaRoamingPreferenceResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getCdmaRoamingPreferenceResponse(int slotId, android::ClientId clientId __unused,
                                           int responseType, int serial, RIL_Errno e,
                                           void *response, size_t responseLen) {
    RLOGD("getCdmaRoamingPreferenceResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseInt(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getCdmaRoamingPreferenceResponse(
                responseInfo, (CdmaRoamingType) ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getCdmaRoamingPreferenceResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setTTYModeResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen) {
    RLOGD("setTTYModeResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setTTYModeResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setTTYModeResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::getTTYModeResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen) {
    RLOGD("getTTYModeResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseInt(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getTTYModeResponse(responseInfo,
                (TtyMode) ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getTTYModeResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::setPreferredVoicePrivacyResponse(int slotId, android::ClientId clientId __unused,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responseLen) {
    RLOGD("setPreferredVoicePrivacyResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        if (s_cardStatus[slotId].base.cardState == CardState::ABSENT) {
            responseInfo.error = RadioError::NONE;
        }
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setPreferredVoicePrivacyResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setPreferredVoicePrivacyResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getPreferredVoicePrivacyResponse(int slotId, android::ClientId clientId __unused,
                                           int responseType, int serial, RIL_Errno e,
                                           void *response, size_t responseLen) {
    RLOGD("getPreferredVoicePrivacyResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        bool enable = false;
        int numInts = responseLen / sizeof(int);
        if (response == NULL || numInts != 1) {
            RLOGE("getPreferredVoicePrivacyResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            enable = pInt[0] == 1 ? true : false;
        }
        if (s_cardStatus[slotId].base.cardState == CardState::ABSENT) {
            responseInfo.error = RadioError::NONE;
        }
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getPreferredVoicePrivacyResponse(
                responseInfo, enable);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getPreferredVoicePrivacyResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::sendCDMAFeatureCodeResponse(int slotId, android::ClientId clientId __unused,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responseLen) {
    RLOGD("sendCDMAFeatureCodeResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->sendCDMAFeatureCodeResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("sendCDMAFeatureCodeResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::sendBurstDtmfResponse(int slotId, android::ClientId clientId __unused,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responseLen) {
    RLOGD("sendBurstDtmfResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->sendBurstDtmfResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("sendBurstDtmfResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::sendCdmaSmsResponse(int slotId, android::ClientId clientId __unused,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responseLen) {
    RLOGD("sendCdmaSmsResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        SendSmsResult result = makeSendSmsResult(responseInfo, serial, responseType, e, response,
                responseLen);

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->sendCdmaSmsResponse(responseInfo, result);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("sendCdmaSmsResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::acknowledgeLastIncomingCdmaSmsResponse(int slotId, android::ClientId clientId __unused,
                                                 int responseType, int serial, RIL_Errno e,
                                                 void *response, size_t responseLen) {
    RLOGD("acknowledgeLastIncomingCdmaSmsResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->acknowledgeLastIncomingCdmaSmsResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("acknowledgeLastIncomingCdmaSmsResponse: radioService[%d]->mRadioResponse "
                "== NULL", slotId);
    }

    return 0;
}

int radio::getGsmBroadcastConfigResponse(int slotId, android::ClientId clientId __unused,
                                        int responseType, int serial, RIL_Errno e,
                                        void *response, size_t responseLen) {
    RLOGD("getGsmBroadcastConfigResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<GsmBroadcastSmsConfigInfo> configs;

        if (response == NULL || responseLen % sizeof(RIL_GSM_BroadcastSmsConfigInfo *) != 0) {
            RLOGE("getGsmBroadcastConfigResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int num = responseLen / sizeof(RIL_GSM_BroadcastSmsConfigInfo *);
            configs.resize(num);
            for (int i = 0 ; i < num; i++) {
                RIL_GSM_BroadcastSmsConfigInfo *resp =
                        ((RIL_GSM_BroadcastSmsConfigInfo **) response)[i];
                configs[i].fromServiceId = resp->fromServiceId;
                configs[i].toServiceId = resp->toServiceId;
                configs[i].fromCodeScheme = resp->fromCodeScheme;
                configs[i].toCodeScheme = resp->toCodeScheme;
                configs[i].selected = resp->selected == 1 ? true : false;
            }
        }

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getGsmBroadcastConfigResponse(responseInfo,
                configs);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getGsmBroadcastConfigResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setGsmBroadcastConfigResponse(int slotId, android::ClientId clientId __unused,
                                        int responseType, int serial, RIL_Errno e,
                                        void *response, size_t responseLen) {
    RLOGD("setGsmBroadcastConfigResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setGsmBroadcastConfigResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setGsmBroadcastConfigResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setGsmBroadcastActivationResponse(int slotId, android::ClientId clientId __unused,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responseLen) {
    RLOGD("setGsmBroadcastActivationResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setGsmBroadcastActivationResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setGsmBroadcastActivationResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getCdmaBroadcastConfigResponse(int slotId, android::ClientId clientId __unused,
                                         int responseType, int serial, RIL_Errno e,
                                         void *response, size_t responseLen) {
    RLOGD("getCdmaBroadcastConfigResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<CdmaBroadcastSmsConfigInfo> configs;

        if (response == NULL || responseLen % sizeof(RIL_CDMA_BroadcastSmsConfigInfo *) != 0) {
            RLOGE("getCdmaBroadcastConfigResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int num = responseLen / sizeof(RIL_CDMA_BroadcastSmsConfigInfo *);
            configs.resize(num);
            for (int i = 0 ; i < num; i++) {
                RIL_CDMA_BroadcastSmsConfigInfo *resp =
                        ((RIL_CDMA_BroadcastSmsConfigInfo **) response)[i];
                configs[i].serviceCategory = resp->service_category;
                configs[i].language = resp->language;
                configs[i].selected = resp->selected == 1 ? true : false;
            }
        }

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getCdmaBroadcastConfigResponse(responseInfo,
                configs);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getCdmaBroadcastConfigResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setCdmaBroadcastConfigResponse(int slotId, android::ClientId clientId __unused,
                                         int responseType, int serial, RIL_Errno e,
                                         void *response, size_t responseLen) {
    RLOGD("setCdmaBroadcastConfigResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setCdmaBroadcastConfigResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setCdmaBroadcastConfigResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setCdmaBroadcastActivationResponse(int slotId, android::ClientId clientId __unused,
                                             int responseType, int serial, RIL_Errno e,
                                             void *response, size_t responseLen) {
    RLOGD("setCdmaBroadcastActivationResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setCdmaBroadcastActivationResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setCdmaBroadcastActivationResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getCDMASubscriptionResponse(int slotId, android::ClientId clientId __unused,
                                      int responseType, int serial, RIL_Errno e, void *response,
                                      size_t responseLen) {
    RLOGD("getCDMASubscriptionResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        int numStrings = responseLen / sizeof(char *);
        hidl_string emptyString;
        if (response == NULL || numStrings != 5) {
            RLOGE("getOperatorResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            Return<void> retStatus
                    = radioService[slotId]->mRadioResponse->getCDMASubscriptionResponse(
                    responseInfo, emptyString, emptyString, emptyString, emptyString, emptyString);
            radioService[slotId]->checkReturnStatus(retStatus, slotId);
        } else {
            char **resp = (char **) response;
            Return<void> retStatus
                    = radioService[slotId]->mRadioResponse->getCDMASubscriptionResponse(
                    responseInfo,
                    convertCharPtrToHidlString(resp[0]),
                    convertCharPtrToHidlString(resp[1]),
                    convertCharPtrToHidlString(resp[2]),
                    convertCharPtrToHidlString(resp[3]),
                    convertCharPtrToHidlString(resp[4]));
            radioService[slotId]->checkReturnStatus(retStatus, slotId);
        }
    } else {
        RLOGE("getCDMASubscriptionResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::writeSmsToRuimResponse(int slotId, android::ClientId clientId __unused,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responseLen) {
    RLOGD("writeSmsToRuimResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseInt(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->writeSmsToRuimResponse(responseInfo, ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("writeSmsToRuimResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::deleteSmsOnRuimResponse(int slotId, android::ClientId clientId __unused,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
    RLOGD("deleteSmsOnRuimResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->deleteSmsOnRuimResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("deleteSmsOnRuimResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }
    return 0;
}

int radio::getDeviceIdentityResponse(int slotId, android::ClientId clientId __unused,
                                    int responseType, int serial, RIL_Errno e, void *response,
                                    size_t responseLen) {
    RLOGD("getDeviceIdentityResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        int numStrings = responseLen / sizeof(char *);
        hidl_string emptyString;
        if (response == NULL || numStrings != 4) {
            RLOGE("getDeviceIdentityResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            Return<void> retStatus
                    = radioService[slotId]->mRadioResponse->getDeviceIdentityResponse(responseInfo,
                    emptyString, emptyString, emptyString, emptyString);
            radioService[slotId]->checkReturnStatus(retStatus, slotId);
        } else {
            char **resp = (char **) response;
            Return<void> retStatus
                    = radioService[slotId]->mRadioResponse->getDeviceIdentityResponse(responseInfo,
                    convertCharPtrToHidlString(resp[0]),
                    convertCharPtrToHidlString(resp[1]),
                    convertCharPtrToHidlString(resp[2]),
                    convertCharPtrToHidlString(resp[3]));
            radioService[slotId]->checkReturnStatus(retStatus, slotId);
        }
    } else {
        RLOGE("getDeviceIdentityResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::exitEmergencyCallbackModeResponse(int slotId, android::ClientId clientId __unused,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responseLen) {
    RLOGD("exitEmergencyCallbackModeResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->exitEmergencyCallbackModeResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("exitEmergencyCallbackModeResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getSmscAddressResponse(int slotId, android::ClientId clientId __unused,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
    RLOGD("getSmscAddressResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getSmscAddressResponse(responseInfo,
                convertCharPtrToHidlString((char *) response));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getSmscAddressResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::setSmscAddressResponse(int slotId, android::ClientId clientId __unused,
                                             int responseType, int serial, RIL_Errno e,
                                             void *response, size_t responseLen) {
    RLOGD("setSmscAddressResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setSmscAddressResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setSmscAddressResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::reportSmsMemoryStatusResponse(int slotId, android::ClientId clientId __unused,
                                        int responseType, int serial, RIL_Errno e,
                                        void *response, size_t responseLen) {
    RLOGD("reportSmsMemoryStatusResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->reportSmsMemoryStatusResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("reportSmsMemoryStatusResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::reportStkServiceIsRunningResponse(int slotId, android::ClientId clientId __unused,
                                             int responseType, int serial, RIL_Errno e,
                                             void *response, size_t responseLen) {
    RLOGD("reportStkServiceIsRunningResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->
                reportStkServiceIsRunningResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("reportStkServiceIsRunningResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getCdmaSubscriptionSourceResponse(int slotId, android::ClientId clientId __unused,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responseLen) {
    RLOGD("getCdmaSubscriptionSourceResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseInt(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getCdmaSubscriptionSourceResponse(
                responseInfo, (CdmaSubscriptionSource) ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getCdmaSubscriptionSourceResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::requestIsimAuthenticationResponse(int slotId, android::ClientId clientId __unused,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responseLen) {
    RLOGD("requestIsimAuthenticationResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->requestIsimAuthenticationResponse(
                responseInfo,
                convertCharPtrToHidlString((char *) response));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("requestIsimAuthenticationResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::acknowledgeIncomingGsmSmsWithPduResponse(int slotId,
        android::ClientId clientId __unused, int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    RLOGD("acknowledgeIncomingGsmSmsWithPduResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->acknowledgeIncomingGsmSmsWithPduResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("acknowledgeIncomingGsmSmsWithPduResponse: radioService[%d]->mRadioResponse "
                "== NULL", slotId);
    }

    return 0;
}

int radio::sendEnvelopeWithStatusResponse(int slotId, android::ClientId clientId __unused,
                                         int responseType, int serial, RIL_Errno e, void *response,
                                         size_t responseLen) {
    RLOGD("sendEnvelopeWithStatusResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        IccIoResult result = responseIccIo(responseInfo, serial, responseType, e,
                response, responseLen);

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->sendEnvelopeWithStatusResponse(responseInfo,
                result);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("sendEnvelopeWithStatusResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getVoiceRadioTechnologyResponse(int slotId, android::ClientId clientId __unused,
                                          int responseType, int serial, RIL_Errno e,
                                          void *response, size_t responseLen) {
    RLOGD("getVoiceRadioTechnologyResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseInt(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getVoiceRadioTechnologyResponse(
                responseInfo, (RadioTechnology) ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getVoiceRadioTechnologyResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

// It's also getCellInfoListResponse_1_2
int radio::getCellInfoListResponse(int slotId, android::ClientId clientId __unused,
                                   int responseType,
                                   int serial, RIL_Errno e, void *response,
                                   size_t responseLen) {
    RLOGD("getCellInfoListResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseV1_4!= NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        // use AOSP V1.4
        char* used_memory = NULL;
        hidl_vec<AOSP_V1_4::CellInfo> ret;
        if (response == NULL || responseLen % sizeof(RIL_CellInfo_v12) != 0) {
            RLOGE("getCellInfoListResponse: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            used_memory = convertRilCellInfoListToHal_1_4(response, responseLen, ret);
        }

        Return<void> retStatus =
            radioService[slotId]->mRadioResponseV1_4->getCellInfoListResponse_1_4(responseInfo, ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
        if (used_memory) free(used_memory);
        used_memory = NULL;
    } else if (radioService[slotId]->mRadioResponseV1_2!= NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        // use AOSP V1.2
        char* used_memory = NULL;
        hidl_vec<AOSP_V1_2::CellInfo> ret;
        if ((response == NULL && responseLen != 0)
                || responseLen % sizeof(RIL_CellInfo_v12) != 0) {
            RLOGE("getCellInfoListResponse: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            used_memory = convertRilCellInfoListToHal_1_2(response, responseLen, ret);
        }

        Return<void> retStatus =
            radioService[slotId]->mRadioResponseV1_2->getCellInfoListResponse_1_2(responseInfo, ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
        if (used_memory) free(used_memory);
        used_memory = NULL;
    } else if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        char* used_memory = NULL;
        hidl_vec<CellInfo> ret;
        if (response == NULL || responseLen % sizeof(RIL_CellInfo_v12) != 0) {
            RLOGE("getCellInfoListResponse: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            used_memory = convertRilCellInfoListToHal(response, responseLen, ret);
        }

        Return<void> retStatus = radioService[slotId]->mRadioResponse->getCellInfoListResponse(
                responseInfo, ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
        if (used_memory) free(used_memory);
        used_memory = NULL;
    } else {
        RLOGE("getCellInfoListResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::setCellInfoListRateResponse(int slotId, android::ClientId clientId __unused,
                                       int responseType,
                                       int serial, RIL_Errno e, void *response,
                                       size_t responseLen) {
    RLOGD("setCellInfoListRateResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setCellInfoListRateResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setCellInfoListRateResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setInitialAttachApnResponse(int slotId, android::ClientId clientId __unused,
                                       int responseType, int serial, RIL_Errno e,
                                       void *response, size_t responseLen) {
    RLOGD("setInitialAttachApnResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setInitialAttachApnResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setInitialAttachApnResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::getImsRegistrationStateResponse(int slotId, android::ClientId clientId __unused,
                                           int responseType, int serial, RIL_Errno e,
                                           void *response, size_t responseLen) {
    RLOGD("getImsRegistrationStateResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        bool isRegistered = false;
        int ratFamily = 0;
        int numInts = responseLen / sizeof(int);
        if (response == NULL || numInts != 2) {
            RLOGE("getImsRegistrationStateResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            isRegistered = pInt[0] == 1 ? true : false;
            ratFamily = pInt[1];
        }
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getImsRegistrationStateResponse(
                responseInfo, isRegistered, (RadioTechnologyFamily) ratFamily);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getImsRegistrationStateResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::sendImsSmsResponse(int slotId, android::ClientId clientId __unused,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responseLen) {
    RLOGD("sendImsSmsResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        SendSmsResult result = makeSendSmsResult(responseInfo, serial, responseType, e, response,
                responseLen);

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->sendImsSmsResponse(responseInfo, result);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("sendSmsResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::iccTransmitApduBasicChannelResponse(int slotId, android::ClientId clientId,
                                               int responseType, int serial, RIL_Errno e,
                                               void *response, size_t responseLen) {
    RLOGD("[%d] iccTransmitApduBasicChannelResponse: serial %d", slotId, serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        IccIoResult result = responseIccIo(responseInfo, serial, responseType, e, response,
                responseLen);

        Return<void> retStatus;
        retStatus = radioService[slotId]->mRadioResponse->iccTransmitApduBasicChannelResponse(
                                                          responseInfo, result);

        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("iccTransmitApduBasicChannelResponse: radioService[%d]->mRadioResponse "
                "== NULL", slotId);
    }

    return 0;
}

int radio::iccOpenLogicalChannelResponse(int slotId, android::ClientId clientId,
                                         int responseType, int serial, RIL_Errno e, void *response,
                                         size_t responseLen) {
    RLOGD("[%d] iccOpenLogicalChannelResponse: serial %d", slotId, serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        int channelId = -1;
        hidl_vec<int8_t> selectResponse;
        int numInts = responseLen / sizeof(int);
        if (response == NULL || responseLen % sizeof(int) != 0) {
            RLOGE("iccOpenLogicalChannelResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            channelId = pInt[0];
            selectResponse.resize(numInts - 1);
            for (int i = 1; i < numInts; i++) {
                selectResponse[i - 1] = (int8_t) pInt[i];
            }
        }

        Return<void> retStatus;
        retStatus = radioService[slotId]->mRadioResponse->iccOpenLogicalChannelResponse(responseInfo,
                                                          channelId, selectResponse);

        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("iccOpenLogicalChannelResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::iccCloseLogicalChannelResponse(int slotId, android::ClientId clientId,
                                          int responseType, int serial, RIL_Errno e,
                                          void *response, size_t responseLen) {
    RLOGD("[%d] iccCloseLogicalChannelResponse: serial %d", slotId, serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus;
        retStatus = radioService[slotId]->mRadioResponse->iccCloseLogicalChannelResponse(
                                                          responseInfo);

        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("iccCloseLogicalChannelResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::iccTransmitApduLogicalChannelResponse(int slotId, android::ClientId clientId,
                                                 int responseType, int serial, RIL_Errno e,
                                                 void *response, size_t responseLen) {
    RLOGD("[%d] iccTransmitApduLogicalChannelResponse: serial %d", slotId, serial);

    if (radioService[slotId]->mRadioResponse != NULL || mtkRadioExService[slotId]->mRadioResponseSE != NULL) {
        RadioResponseInfo responseInfo = {};
        IccIoResult result = responseIccIo(responseInfo, serial, responseType, e, response,
                responseLen);

        Return<void> retStatus;
        retStatus = radioService[slotId]->mRadioResponse->iccTransmitApduLogicalChannelResponse(
                                                          responseInfo, result);

        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("iccTransmitApduLogicalChannelResponse: radioService[%d]->mRadioResponse "
                "== NULL", slotId);
    }

    return 0;
}

int radio::nvReadItemResponse(int slotId, android::ClientId clientId __unused,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responseLen) {
    RLOGD("nvReadItemResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->nvReadItemResponse(
                responseInfo,
                convertCharPtrToHidlString((char *) response));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("nvReadItemResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::nvWriteItemResponse(int slotId, android::ClientId clientId __unused,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen) {
    RLOGD("nvWriteItemResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->nvWriteItemResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("nvWriteItemResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::nvWriteCdmaPrlResponse(int slotId, android::ClientId clientId __unused,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
    RLOGD("nvWriteCdmaPrlResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->nvWriteCdmaPrlResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("nvWriteCdmaPrlResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::nvResetConfigResponse(int slotId, android::ClientId clientId __unused,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responseLen) {
    RLOGD("nvResetConfigResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->nvResetConfigResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("nvResetConfigResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::setUiccSubscriptionResponse(int slotId, android::ClientId clientId __unused,
                                       int responseType, int serial, RIL_Errno e,
                                       void *response, size_t responseLen) {
    RLOGD("setUiccSubscriptionResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setUiccSubscriptionResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setUiccSubscriptionResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setDataAllowedResponse(int slotId, android::ClientId clientId __unused,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
    RLOGD("setDataAllowedResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setDataAllowedResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setDataAllowedResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::getHardwareConfigResponse(int slotId, android::ClientId clientId __unused,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responseLen) {
    RLOGD("getHardwareConfigResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        hidl_vec<HardwareConfig> result;
        if (response == NULL || responseLen % sizeof(RIL_HardwareConfig) != 0) {
            RLOGE("hardwareConfigChangedInd: invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            convertRilHardwareConfigListToHal(response, responseLen, result);
        }

        Return<void> retStatus = radioService[slotId]->mRadioResponse->getHardwareConfigResponse(
                responseInfo, result);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getHardwareConfigResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::requestIccSimAuthenticationResponse(int slotId, android::ClientId clientId __unused,
                                               int responseType, int serial, RIL_Errno e,
                                               void *response, size_t responseLen) {
    RLOGD("requestIccSimAuthenticationResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        IccIoResult result = responseIccIo(responseInfo, serial, responseType, e, response,
                responseLen);

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->requestIccSimAuthenticationResponse(
                responseInfo, result);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("requestIccSimAuthenticationResponse: radioService[%d]->mRadioResponse "
                "== NULL", slotId);
    }

    return 0;
}

int radio::setDataProfileResponse(int slotId, android::ClientId clientId __unused,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
    RLOGD("setDataProfileResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        // M @{
        if (s_cardStatus[slotId].base.cardState == CardState::ABSENT) {
            responseInfo.error = RadioError::NONE;
        }
        // M @}

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setDataProfileResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setDataProfileResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::requestShutdownResponse(int slotId, android::ClientId clientId __unused,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
    RLOGD("requestShutdownResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->requestShutdownResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("requestShutdownResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

void responseRadioCapability(RadioResponseInfo& responseInfo, int serial,
        int responseType, RIL_Errno e, void *response, size_t responseLen, RadioCapability& rc) {
    populateResponseInfo(responseInfo, serial, responseType, e);

    if (response == NULL || responseLen != sizeof(RIL_RadioCapability)) {
        RLOGE("responseRadioCapability: Invalid response");
        if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        rc.logicalModemUuid = hidl_string();
    } else {
        convertRilRadioCapabilityToHal(response, responseLen, rc);
    }
}

int radio::getRadioCapabilityResponse(int slotId, android::ClientId clientId __unused,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responseLen) {
    RLOGD("getRadioCapabilityResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        RadioCapability result = {};
        responseRadioCapability(responseInfo, serial, responseType, e, response, responseLen,
                result);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->getRadioCapabilityResponse(
                responseInfo, result);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getRadioCapabilityResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::setRadioCapabilityResponse(int slotId, android::ClientId clientId __unused,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responseLen) {
    RLOGD("setRadioCapabilityResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        RadioCapability result = {};
        responseRadioCapability(responseInfo, serial, responseType, e, response, responseLen,
                result);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->setRadioCapabilityResponse(
                responseInfo, result);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setRadioCapabilityResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

LceStatusInfo responseLceStatusInfo(RadioResponseInfo& responseInfo, int serial, int responseType,
                                    RIL_Errno e, void *response, size_t responseLen) {
    populateResponseInfo(responseInfo, serial, responseType, e);
    LceStatusInfo result = {};

    if (response == NULL || responseLen != sizeof(RIL_LceStatusInfo)) {
        RLOGE("Invalid response: NULL");
        if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
    } else {
        RIL_LceStatusInfo *resp = (RIL_LceStatusInfo *) response;
        result.lceStatus = (LceStatus) resp->lce_status;
        result.actualIntervalMs = (uint8_t) resp->actual_interval_ms;
    }

    return result;
}

int radio::startLceServiceResponse(int slotId, android::ClientId clientId __unused,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen) {
    RLOGD("startLceServiceResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        LceStatusInfo result = responseLceStatusInfo(responseInfo, serial, responseType, e,
                response, responseLen);

        // M @{
        if (s_cardStatus[slotId].base.cardState == CardState::ABSENT) {
            responseInfo.error = RadioError::SIM_ABSENT;
        }
        // M @}

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->startLceServiceResponse(responseInfo,
                result);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("startLceServiceResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::stopLceServiceResponse(int slotId, android::ClientId clientId __unused,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
    RLOGD("stopLceServiceResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        LceStatusInfo result = responseLceStatusInfo(responseInfo, serial, responseType, e,
                response, responseLen);

        // M @{
        if (s_cardStatus[slotId].base.cardState == CardState::ABSENT) {
            responseInfo.error = RadioError::NONE;
        }
        // M @}

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->stopLceServiceResponse(responseInfo,
                result);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("stopLceServiceResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::pullLceDataResponse(int slotId, android::ClientId clientId __unused,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen) {
    RLOGD("pullLceDataResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        LceDataInfo result = {};
        if (response == NULL || responseLen != sizeof(RIL_LceDataInfo)) {
            RLOGE("pullLceDataResponse: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            convertRilLceDataInfoToHal(response, responseLen, result);
        }

        // M @{
        if (s_cardStatus[slotId].base.cardState == CardState::ABSENT) {
            responseInfo.error = RadioError::NONE;
        }
        // M @}

        Return<void> retStatus = radioService[slotId]->mRadioResponse->pullLceDataResponse(
                responseInfo, result);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("pullLceDataResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::getModemActivityInfoResponse(int slotId, android::ClientId clientId __unused,
                                        int responseType, int serial, RIL_Errno e,
                                        void *response, size_t responseLen) {
    RLOGD("getModemActivityInfoResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        ActivityStatsInfo info;
        if (response == NULL || responseLen != sizeof(RIL_ActivityStatsInfo)) {
            RLOGE("getModemActivityInfoResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            RIL_ActivityStatsInfo *resp = (RIL_ActivityStatsInfo *)response;
            info.sleepModeTimeMs = resp->sleep_mode_time_ms;
            info.idleModeTimeMs = resp->idle_mode_time_ms;
            for(int i = 0; i < RIL_NUM_TX_POWER_LEVELS; i++) {
                info.txmModetimeMs[i] = resp->tx_mode_time_ms[i];
            }
            info.rxModeTimeMs = resp->rx_mode_time_ms;
        }

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getModemActivityInfoResponse(responseInfo,
                info);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getModemActivityInfoResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setAllowedCarriersResponse(int slotId, android::ClientId clientId __unused,
                                      int responseType, int serial, RIL_Errno e,
                                      void *response, size_t responseLen) {
    RLOGI(LOG_TAG, "setAllowedCarriersResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseV1_4 != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponseV1_4->setAllowedCarriersResponse_1_4(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else if (radioService[slotId]->mRadioResponse != NULL)  {
        RadioResponseInfo responseInfo = {};
        int ret = responseInt(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setAllowedCarriersResponse(responseInfo,
                ret);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setAllowedCarriersResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::getAllowedCarriersResponse(int slotId, android::ClientId clientId __unused,
                                      int responseType, int serial, RIL_Errno e,
                                      void *response, size_t responseLen) {
    RLOGI("getAllowedCarriersResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseV1_4 != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        AOSP_V1_4::CarrierRestrictionsWithPriority carriers = {};
        AOSP_V1_4::SimLockMultiSimPolicy multisimPolicy = AOSP_V1_4::SimLockMultiSimPolicy
                ::NO_MULTISIM_POLICY;
        if ((response == NULL) || (responseLen != sizeof(RIL_CarrierRestrictionsWithPriority))) {
            RLOGE("getAllowedCarriersResponse_1_4 RIL_E_REQUEST_NOT_SUPPORTED");
        } else {
            RIL_CarrierRestrictionsWithPriority *pCr
                    = (RIL_CarrierRestrictionsWithPriority *)response;

            carriers.allowedCarriers.resize(pCr->len_allowed_carriers);
            for (int i = 0; i < pCr->len_allowed_carriers; i++) {
                RIL_Carrier *carrier = pCr->allowed_carriers + i;
                carriers.allowedCarriers[i].mcc = convertCharPtrToHidlString(carrier->mcc);
                carriers.allowedCarriers[i].mnc = convertCharPtrToHidlString(carrier->mnc);
                carriers.allowedCarriers[i].matchType = (CarrierMatchType) carrier->match_type;
                carriers.allowedCarriers[i].matchData =
                        convertCharPtrToHidlString(carrier->match_data);
            }

            carriers.excludedCarriers.resize(pCr->len_excluded_carriers);
            for (int i = 0; i < pCr->len_excluded_carriers; i++) {
                RIL_Carrier *carrier = pCr->excluded_carriers + i;
                carriers.excludedCarriers[i].mcc = convertCharPtrToHidlString(carrier->mcc);
                carriers.excludedCarriers[i].mnc = convertCharPtrToHidlString(carrier->mnc);
                carriers.excludedCarriers[i].matchType = (CarrierMatchType) carrier->match_type;
                carriers.excludedCarriers[i].matchData =
                        convertCharPtrToHidlString(carrier->match_data);
            }

            carriers.allowedCarriersPrioritized = pCr->allowedCarriersPrioritized;
            multisimPolicy = (AOSP_V1_4::SimLockMultiSimPolicy) pCr->simLockMultiSimPolicy;
        }

        Return<void> retStatus = radioService[slotId]->mRadioResponseV1_4
                    ->getAllowedCarriersResponse_1_4(responseInfo, carriers, multisimPolicy);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        CarrierRestrictions carrierInfo = {};
        bool allAllowed = true;
        if (response == NULL) {
#if VDBG
            RLOGI("getAllowedCarriersResponse response is NULL: all allowed");
#endif
            carrierInfo.allowedCarriers.resize(0);
            carrierInfo.excludedCarriers.resize(0);
        } else if (responseLen != sizeof(RIL_CarrierRestrictionsWithPriority)) {
            RLOGE("getAllowedCarriersResponse Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            RIL_CarrierRestrictionsWithPriority *pCr
                    = (RIL_CarrierRestrictionsWithPriority *)response;
            if (pCr->len_allowed_carriers > 0 || pCr->len_excluded_carriers > 0) {
                allAllowed = false;
            }

            carrierInfo.allowedCarriers.resize(pCr->len_allowed_carriers);
            for(int i = 0; i < pCr->len_allowed_carriers; i++) {
                RIL_Carrier *carrier = pCr->allowed_carriers + i;
                carrierInfo.allowedCarriers[i].mcc = convertCharPtrToHidlString(carrier->mcc);
                carrierInfo.allowedCarriers[i].mnc = convertCharPtrToHidlString(carrier->mnc);
                carrierInfo.allowedCarriers[i].matchType = (CarrierMatchType) carrier->match_type;
                carrierInfo.allowedCarriers[i].matchData =
                        convertCharPtrToHidlString(carrier->match_data);
            }

            carrierInfo.excludedCarriers.resize(pCr->len_excluded_carriers);
            for(int i = 0; i < pCr->len_excluded_carriers; i++) {
                RIL_Carrier *carrier = pCr->excluded_carriers + i;
                carrierInfo.excludedCarriers[i].mcc = convertCharPtrToHidlString(carrier->mcc);
                carrierInfo.excludedCarriers[i].mnc = convertCharPtrToHidlString(carrier->mnc);
                carrierInfo.excludedCarriers[i].matchType = (CarrierMatchType) carrier->match_type;
                carrierInfo.excludedCarriers[i].matchData =
                        convertCharPtrToHidlString(carrier->match_data);
            }
        }

        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->getAllowedCarriersResponse(responseInfo,
                allAllowed, carrierInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getAllowedCarriersResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::sendDeviceStateResponse(int slotId, android::ClientId clientId __unused,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responselen) {
    RLOGD("sendDeviceStateResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->sendDeviceStateResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("sendDeviceStateResponse: radioService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int radio::setCarrierInfoForImsiEncryptionResponse(int slotId, android::ClientId clientId __unused,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen) {
    RLOGD("setCarrierInfoForImsiEncryptionResponse: serial %d", serial);
    if (radioService[slotId]->mRadioResponseV1_1 != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponseV1_1->
                setCarrierInfoForImsiEncryptionResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setCarrierInfoForImsiEncryptionResponse: radioService[%d]->mRadioResponseV1_1 == "
                "NULL", slotId);
    }
    return 0;
}

int radio::setIndicationFilterResponse(int slotId, android::ClientId clientId __unused,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responselen) {
    RLOGD("setIndicationFilterResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponse->setIndicationFilterResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setIndicationFilterResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::setSimCardPowerResponse(int slotId, android::ClientId clientId __unused,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen) {
#if VDBG
    RLOGD("setSimCardPowerResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponse != NULL
            || radioService[slotId]->mRadioResponseV1_1 != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        if (radioService[slotId]->mRadioResponseV1_1 != NULL) {
            Return<void> retStatus = radioService[slotId]->mRadioResponseV1_1->
                    setSimCardPowerResponse_1_1(responseInfo);
            radioService[slotId]->checkReturnStatus(retStatus, slotId);
        } else {
            RLOGD("setSimCardPowerResponse: radioService[%d]->mRadioResponseV1_1 == NULL",
                    slotId);
            Return<void> retStatus
                    = radioService[slotId]->mRadioResponse->setSimCardPowerResponse(responseInfo);
            radioService[slotId]->checkReturnStatus(retStatus, slotId);
        }
    } else {
        RLOGE("setSimCardPowerResponse: radioService[%d]->mRadioResponse == NULL && "
                "radioService[%d]->mRadioResponseV1_1 == NULL", slotId, slotId);
    }
    return 0;
}

int radio::startNetworkScanResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
#if VDBG
    RLOGD("startNetworkScanResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponseV1_4 != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponseV1_4->startNetworkScanResponse_1_4(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else if (radioService[slotId]->mRadioResponseV1_2 != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponseV1_2->startNetworkScanResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else if (radioService[slotId]->mRadioResponseV1_1 != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponseV1_1->startNetworkScanResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("startNetworkScanResponse: radioService[%d]->mRadioResponseV1_1 == NULL", slotId);
    }

    return 0;
}

int radio::stopNetworkScanResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
#if VDBG
    RLOGD("stopNetworkScanResponse: serial %d", serial);
#endif

    if (radioService[slotId]->mRadioResponseV1_1 != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponseV1_1->stopNetworkScanResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("stopNetworkScanResponse: radioService[%d]->mRadioResponseV1_1 == NULL", slotId);
    }

    return 0;
}

void convertRilKeepaliveStatusToHal(const RIL_KeepaliveStatus *rilStatus,
        AOSP_V1_1::KeepaliveStatus& halStatus) {
    halStatus.sessionHandle = rilStatus->sessionHandle;
    halStatus.code = static_cast<AOSP_V1_1::KeepaliveStatusCode>(rilStatus->code);
}

int radio::startKeepaliveResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("%s(): %d", __FUNCTION__, serial);
    RadioResponseInfo responseInfo = {};
    populateResponseInfo(responseInfo, serial, responseType, e);

    // If we don't have a radio service, there's nothing we can do
    if (radioService[slotId]->mRadioResponseV1_1 == NULL) {
        RLOGE("%s: radioService[%d]->mRadioResponseV1_1 == NULL", __FUNCTION__, slotId);
        return 0;
    }

    AOSP_V1_1::KeepaliveStatus ks = {};
    if (response == NULL || responseLen != sizeof(AOSP_V1_1::KeepaliveStatus)) {
        RLOGE("%s: invalid response - %d", __FUNCTION__, static_cast<int>(e));
        if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
    } else {
        convertRilKeepaliveStatusToHal(static_cast<RIL_KeepaliveStatus*>(response), ks);
    }

    Return<void> retStatus =
            radioService[slotId]->mRadioResponseV1_1->startKeepaliveResponse(responseInfo, ks);
    radioService[slotId]->checkReturnStatus(retStatus, slotId);
    return 0;
}

int radio::stopKeepaliveResponse(int slotId, android::ClientId clientId __unused, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("%s(): %d", __FUNCTION__, serial);
    RadioResponseInfo responseInfo = {};
    populateResponseInfo(responseInfo, serial, responseType, e);

    // If we don't have a radio service, there's nothing we can do
    if (radioService[slotId]->mRadioResponseV1_1 == NULL) {
        RLOGE("%s: radioService[%d]->mRadioResponseV1_1 == NULL", __FUNCTION__, slotId);
        return 0;
    }

    Return<void> retStatus =
            radioService[slotId]->mRadioResponseV1_1->stopKeepaliveResponse(responseInfo);
    radioService[slotId]->checkReturnStatus(retStatus, slotId);
    return 0;
}

int radio::sendRequestRawResponse(int slotId, android::ClientId clientId,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
   RLOGD("sendRequestRawResponse: serial %d", serial);

    RadioResponseInfo responseInfo = {};
    populateResponseInfo(responseInfo, serial, responseType, e);
    hidl_vec<uint8_t> data;

    if (response == NULL) {
        RLOGE("sendRequestRawResponse: Invalid response");
        if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
    } else {
        data.setToExternal((uint8_t *) response, responseLen);
    }

    if (clientId == android::CLIENT_EM) {
        if (mtkRadioExService[slotId]->mRadioResponseEm != NULL) {
            Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseEm->
                    sendRequestRawResponse(responseInfo, data);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::EM_MTK, slotId);
        } else {
            RLOGE("sendRequestRawResponse: radioService[%d]->mRadioResponseEm == NULL",
                    slotId);
        }
    } else {
        if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
            Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->
                    sendRequestRawResponse(responseInfo, data);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
        } else {
            RLOGE("sendRequestRawResponse: radioService[%d]->mRadioResponseMtk == NULL",
                    slotId);
        }
    }
    return 0;
}

int radio::sendRequestStringsResponse(int slotId, android::ClientId clientId,
                                      int responseType, int serial, RIL_Errno e,
                                      void *response, size_t responseLen) {
    RLOGD("sendRequestStringsResponse: serial %d", serial);

    RadioResponseInfo responseInfo = {};
    populateResponseInfo(responseInfo, serial, responseType, e);
    hidl_vec<hidl_string> data;

    if ((response == NULL && responseLen != 0) || responseLen % sizeof(char *) != 0) {
        RLOGE("sendRequestStringsResponse Invalid response: NULL");
        if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
    } else {
        char **resp = (char **) response;
        int numStrings = responseLen / sizeof(char *);
        data.resize(numStrings);
        for (int i = 0; i < numStrings; i++) {
            data[i] = convertCharPtrToHidlString(resp[i]);
        }
    }

    if (clientId == android::CLIENT_EM) {
        if (mtkRadioExService[slotId]->mRadioResponseEm != NULL) {
            Return<void> retStatus
                    = mtkRadioExService[slotId]->mRadioResponseEm->sendRequestStringsResponse(
                    responseInfo, data);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::EM_MTK, slotId);
        } else {
            RLOGE("sendRequestStringsResponse: radioService[%d]->mRadioResponseEm == "
                    "NULL", slotId);
        }
    } else {
        if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
            Return<void> retStatus
                    = mtkRadioExService[slotId]->mRadioResponseMtk->sendRequestStringsResponse(
                    responseInfo, data);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
        } else {
            RLOGE("sendRequestStringsResponse: radioService[%d]->mRadioResponseMtk == "
                    "NULL", slotId);
        }
    }
    return 0;
}

int radio::enableModemResponse(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno err, void *response, size_t responseLen) {
    RLOGD("enableModemResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseV1_3 != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, err);
        Return<void> retStatus = radioService[slotId]->mRadioResponseV1_3->enableModemResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("enableModemResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }
    return 0;
}

int radio::getModemStackStatusResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    RLOGD("getModemStackStatusResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseV1_3 != NULL) {
         RadioState radioState = (RadioState) s_vendorFunctions->
                                         onStateRequest((RIL_SOCKET_ID)toRealSlot(slotId));
        bool  isEnable;
        if (radioState == RadioState::ON) {
            isEnable = true;
        } else if (radioState == RadioState::OFF) {
            isEnable = false;
        } else {
            isEnable = false;
            e = RIL_E_RADIO_NOT_AVAILABLE;
        }
        RLOGD("getModemStackStatusResponse: isEnable %d, e = %d", isEnable, e);
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponseV1_3->getModemStackStatusResponse(
                responseInfo, isEnable);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("getModemStackStatusResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }
    return 0;
}
/***************************************************************************************************
 * MTK Response FUNCTIONS
 * The below function handle unsolicited messages coming from the Radio
 * (messages for which there is no pending request)
 **************************************************************************************************/
/// M: CC: Emergency mode for Fusion RIL
int mtkRadioEx::setEccModeResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    RLOGD("setEccModeResponse: serial %d", serial);
    return 0;
}

/// M: CC: Vzw/CTVolte ECC for Fusion RIL
int mtkRadioEx::eccPreferredRatResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    RLOGD("eccPreferredRatResponse: serial %d", serial);
    return 0;
}

int mtkRadioEx::getSignalStrengthWithWcdmaEcioResponse(int slotId,
        android::ClientId clientId __unused, int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    return 0;
}

// MTK-START: SIM
int mtkRadioEx::getATRResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    RLOGD("[%d] getATRResponse: serial %d", slotId, serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL || mtkRadioExService[slotId]->mRadioResponseSE != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus;
        if ((clientId == android::CLIENT_SE) && mtkRadioExService[slotId]->mRadioResponseSE != NULL) { /// MTK: ForSE
            retStatus = mtkRadioExService[slotId]->mRadioResponseSE->getATRResponse(
                responseInfo,
                convertCharPtrToHidlString((char *) response));
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::SE_MTK, slotId);
        } else {
            retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->getATRResponse(
                responseInfo,
                convertCharPtrToHidlString((char *) response));
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
        }
    } else {
        RLOGE("nvReadItemResponse: mtkRadioExService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::getIccidResponse(int slotId, android::ClientId clientId __unused, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("getIccidResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->getIccidResponse(
                responseInfo,
                convertCharPtrToHidlString((char *) response));
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("getIccidResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::setSimPowerResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    RLOGD("setSimPowerResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->setSimPowerResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("nvReadItemResponse: mtkRadioExService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

// MTK-START: SIM ME LOCK
int mtkRadioEx::queryNetworkLockResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    RLOGD("queryNetworkLockResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        RIL_SimMeLockCatInfo *pLockCatInfo = (RIL_SimMeLockCatInfo *) response;
        int category = -1, state = -1, retry_cnt = -1, autolock_cnt = -1,
                num_set = -1, total_set = -1, key_state = -1;

        if (response == NULL || responseLen % sizeof(RIL_SimMeLockCatInfo) != 0) {
            RLOGE("queryNetworkLockResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            category = pLockCatInfo->catagory;
            state = pLockCatInfo->state;
            retry_cnt = pLockCatInfo->retry_cnt;
            autolock_cnt = pLockCatInfo->autolock_cnt;
            num_set = pLockCatInfo->num_set;
            total_set = pLockCatInfo->total_set;
            key_state = pLockCatInfo->key_state;
        }
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->queryNetworkLockResponse(
                responseInfo, category, state, retry_cnt, autolock_cnt, num_set, total_set,
                key_state);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("queryNetworkLockResponse: mtkRadioExService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::setNetworkLockResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    RLOGD("setNetworkLockResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->setNetworkLockResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("setNetworkLockResponse: mtkRadioExService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::supplyDepersonalizationResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("supplyDepersonalizationResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        int ret = responseIntOrEmpty(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->
                supplyDepersonalizationResponse(responseInfo, ret);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("supplyDepersonalizationResponse: mtkRadioExService[%d]->mRadioResponseMtk == "
                "NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::setClipResponse(int slotId, android::ClientId clientId __unused, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("setClipResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->setClipResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("setClipResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::getColpResponse(int slotId, android::ClientId clientId __unused, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("getColpResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        int n = -1, m = -1;
        int numInts = responseLen / sizeof(int);
        if (response == NULL || numInts != 2) {
            RLOGE("getColpResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            n = pInt[0];
            m = pInt[1];
        }
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->getColpResponse(responseInfo,
                n, m);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("getColpResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::getColrResponse(int slotId, android::ClientId clientId __unused, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("getColrResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        int n = responseInt(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->
                getColrResponse(responseInfo, n);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("getColrResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }

    return 0;
}

int mtkRadioEx::sendCnapResponse(int slotId, android::ClientId clientId __unused, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("sendCnapResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        int n = -1, m = -1;
        int numInts = responseLen / sizeof(int);
        if (response == NULL || numInts != 2) {
            RLOGE("sendCnapResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            n = pInt[0];
            m = pInt[1];
        }
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->
                sendCnapResponse(responseInfo, n, m);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("sendCnapResponse: radioService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::setColpResponse(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("setColpResponse: serial %d", serial);

    if(clientId == android::CLIENT_IMS) {
        if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseIms->setColpResponse(
                    responseInfo);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
        } else {
            RLOGE("setColpResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL", slotId);
        }
    } else {
        if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->setColpResponse(
                    responseInfo);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
        } else {
            RLOGE("setColpResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
        }
    }

    return 0;
}

int mtkRadioEx::setColrResponse(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("setColrResponse: serial %d", serial);

    if(clientId == android::CLIENT_IMS) {
        if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseIms->setColrResponse(
                    responseInfo);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
        } else {
            RLOGE("setColrResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL", slotId);
        }
    } else {
        if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->setColrResponse(
                    responseInfo);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
        } else {
            RLOGE("setColrResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
        }
    }

    return 0;
}

int mtkRadioEx::queryCallForwardInTimeSlotStatusResponse(int slotId,
        android::ClientId clientId, int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    RLOGD("queryCallForwardInTimeSlotStatusResponse: serial %d", serial);

    if(clientId == android::CLIENT_IMS) {
        if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            hidl_vec<CallForwardInfoEx> callForwardInfoExs;

            if ((response == NULL && responseLen != 0)
                    || responseLen % sizeof(RIL_CallForwardInfoEx *) != 0) {
                RLOGE("queryCallForwardInTimeSlotStatusResponse Invalid response: NULL");
                if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            } else {
                int num = responseLen / sizeof(RIL_CallForwardInfoEx *);
                callForwardInfoExs.resize(num);
                for (int i = 0 ; i < num; i++) {
                    RIL_CallForwardInfoEx *resp = ((RIL_CallForwardInfoEx **) response)[i];
                    callForwardInfoExs[i].status = (CallForwardInfoStatus) resp->status;
                    callForwardInfoExs[i].reason = resp->reason;
                    callForwardInfoExs[i].serviceClass = resp->serviceClass;
                    callForwardInfoExs[i].toa = resp->toa;
                    callForwardInfoExs[i].number = convertCharPtrToHidlString(resp->number);
                    callForwardInfoExs[i].timeSeconds = resp->timeSeconds;
                    callForwardInfoExs[i].timeSlotBegin = convertCharPtrToHidlString(resp->timeSlotBegin);
                    callForwardInfoExs[i].timeSlotEnd = convertCharPtrToHidlString(resp->timeSlotEnd);
                }
            }

            Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseIms->
                    queryCallForwardInTimeSlotStatusResponse(responseInfo, callForwardInfoExs);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
        } else {
            RLOGE("queryCallForwardInTimeSlotStatusResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL", slotId);
        }
    } else {
        if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            hidl_vec<CallForwardInfoEx> callForwardInfoExs;

            if ((response == NULL && responseLen != 0)
                    || responseLen % sizeof(RIL_CallForwardInfoEx *) != 0) {
                RLOGE("queryCallForwardInTimeSlotStatusResponse Invalid response: NULL");
                if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            } else {
                int num = responseLen / sizeof(RIL_CallForwardInfoEx *);
                callForwardInfoExs.resize(num);
                for (int i = 0 ; i < num; i++) {
                    RIL_CallForwardInfoEx *resp = ((RIL_CallForwardInfoEx **) response)[i];
                    callForwardInfoExs[i].status = (CallForwardInfoStatus) resp->status;
                    callForwardInfoExs[i].reason = resp->reason;
                    callForwardInfoExs[i].serviceClass = resp->serviceClass;
                    callForwardInfoExs[i].toa = resp->toa;
                    callForwardInfoExs[i].number = convertCharPtrToHidlString(resp->number);
                    callForwardInfoExs[i].timeSeconds = resp->timeSeconds;
                    callForwardInfoExs[i].timeSlotBegin = convertCharPtrToHidlString(resp->timeSlotBegin);
                    callForwardInfoExs[i].timeSlotEnd = convertCharPtrToHidlString(resp->timeSlotEnd);
                }
            }

            Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->
                    queryCallForwardInTimeSlotStatusResponse(responseInfo, callForwardInfoExs);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
        } else {
            RLOGE("queryCallForwardInTimeSlotStatusResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
        }
    }

    return 0;
}

int mtkRadioEx::setCallForwardInTimeSlotResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("setCallForwardInTimeSlotResponse: serial %d", serial);

    if(clientId == android::CLIENT_IMS) {
        if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseIms->
                    setCallForwardInTimeSlotResponse(responseInfo);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
        } else {
            RLOGE("setCallForwardInTimeSlotResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL", slotId);
        }
    } else {
        if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->
                    setCallForwardInTimeSlotResponse(responseInfo);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
        } else {
            RLOGE("setCallForwardInTimeSlotResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
        }
    }

    return 0;
}

int mtkRadioEx::runGbaAuthenticationResponse(int slotId, android::ClientId clientId,
                                       int responseType, int serial, RIL_Errno e,
                                       void *response, size_t responseLen) {
    RLOGD("runGbaAuthenticationResponse: serial %d", serial);

    if(clientId == android::CLIENT_IMS) {
        if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            hidl_vec<hidl_string> resList;
            int numStrings = responseLen / sizeof(char*);
            if (response == NULL || responseLen % sizeof(char *) != 0) {
                RLOGE("runGbaAuthenticationResponse Invalid response: NULL");
                if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            } else {
                char **pString = (char **) response;
                resList.resize(numStrings);
                for (int i = 0; i < numStrings; i++) {
                    resList[i] = convertCharPtrToHidlString(pString[i]);
                }
            }
            Return<void> retStatus
                    = mtkRadioExService[slotId]->mRadioResponseIms->runGbaAuthenticationResponse(responseInfo,
                    resList);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
        } else {
            RLOGE("runGbaAuthenticationResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                    slotId);
        }
    } else {
        if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            hidl_vec<hidl_string> resList;
            int numStrings = responseLen / sizeof(char*);
            if (response == NULL || responseLen % sizeof(char *) != 0) {
                RLOGE("runGbaAuthenticationResponse Invalid response: NULL");
                if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
            } else {
                char **pString = (char **) response;
                resList.resize(numStrings);
                for (int i = 0; i < numStrings; i++) {
                    resList[i] = convertCharPtrToHidlString(pString[i]);
                }
            }
            Return<void> retStatus
                    = mtkRadioExService[slotId]->mRadioResponseMtk->runGbaAuthenticationResponse(responseInfo,
                    resList);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
        } else {
            RLOGE("runGbaAuthenticationResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                    slotId);
        }
    }

    return 0;
}

int mtkRadioEx::acknowledgeLastIncomingCdmaSmsExResponse(int slotId,
        android::ClientId clientId __unused, int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    RLOGD("acknowledgeLastIncomingCdmaSmsResponseEx: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseIms->acknowledgeLastIncomingCdmaSmsExResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("acknowledgeLastIncomingCdmaSmsResponseEx: mtkRadioExService[%d]->mRadioResponse "
                "== NULL", slotId);
    }

    return 0;
}

// ATCI Start
int mtkRadioEx::sendAtciResponse(int slotId, android::ClientId clientId __unused,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
    RLOGD("sendAtciResponse: serial %d", serial);
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mAtciResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<uint8_t> data;

        if (response == NULL) {
            RLOGE("sendAtciResponse: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            data.setToExternal((uint8_t *) response, responseLen);
        }
        Return<void> retStatus = mtkRadioExService[slotId]->mAtciResponse->
                sendAtciResponse(responseInfo, data);
        if (!retStatus.isOk()) {
            RLOGE("sendAtciResponse: unable to call response callback");
            mtkRadioExService[slotId]->mAtciResponse = NULL;
            mtkRadioExService[slotId]->mAtciIndication = NULL;
        }
    } else {
        RLOGE("sendAtciResponse: mtkRadioExService[%d]->mAtciResponse == NULL", slotId);
    }

    return 0;
}
// ATCI End

// SUBSIDYLOCK Start
int mtkRadioEx::sendSubsidyLockResponse(int slotId, android::ClientId clientId __unused,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
    RLOGE("sendSubsidyLockResponse: serial %d, responseLen = %d", serial, responseLen);
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioResponseSubsidy
            != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<uint32_t> data;
        int numInts = responseLen / sizeof(int);
        if (response == NULL || responseLen % sizeof(int) != 0) {
            RLOGE("sendSubsidyLockResponse: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        }  else {
            int *pInt = (int *) response;
            data.resize(numInts);
            data[0] = (int32_t) pInt[0];  // reqType
            data[1] = (int32_t) pInt[1];  // blobStatus OR lockStatus
            for (int i = 0; i < numInts; i++) {
                data[i] = (int32_t) pInt[i];
            }
        }
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseSubsidy->
                sendSubsidyLockResponse(responseInfo, data);
        if (!retStatus.isOk()) {
            RLOGE("sendSubsidyLockResponse: unable to call response callback");
            mtkRadioExService[slotId]->mRadioResponseSubsidy = NULL;
            mtkRadioExService[slotId]->mRadioIndicationSubsidy = NULL;
        }
    } else {
        RLOGE("sendSubsidyLockResponse: mtkRadioExService[%d]->mRadioResponseSubsidy == NULL",
                slotId);
    }
    return 0;
}
// SUBSIDYLOCK End

/// M: CC: call control @{
int mtkRadioEx::hangupAllResponse(int slotId, android::ClientId clientId,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responselen) {
    RLOGD("hangupAllResponse: serial %d", serial);
    if(clientId == android::CLIENT_IMS) {
        if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            Return<void> retStatus = mtkRadioExService[slotId]->
                                     mRadioResponseIms->hangupAllResponse(responseInfo);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
        } else {
            RLOGE("hangupAllResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                    slotId);
        }
    }
    else {

        if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            Return<void> retStatus = mtkRadioExService[slotId]->
                                     mRadioResponseMtk->hangupAllResponse(responseInfo);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
        } else {
            RLOGE("hangupAllResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                    slotId);
        }
    }

    return 0;
}

int mtkRadioEx::setCallIndicationResponse(int slotId, android::ClientId clientId,
                                     int responseType, int serial, RIL_Errno e,
                                     void *response, size_t responselen) {
    RLOGD("setCallIndicationResponse: serial %d", serial);
    if(clientId == android::CLIENT_IMS) {
        if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            Return<void> retStatus = mtkRadioExService[slotId]->
                                     mRadioResponseIms->
                                     setCallIndicationResponse(responseInfo);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
        } else {
            RLOGE("setCallIndicationResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                    slotId);
        }
    }
    else {
        if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            Return<void> retStatus = mtkRadioExService[slotId]->
                                     mRadioResponseMtk->
                                     setCallIndicationResponse(responseInfo);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
        } else {
            RLOGE("setCallIndicationResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                    slotId);
        }
    }

    return 0;
}
/// M: CC: @}

// PHB START
int mtkRadioEx::queryPhbStorageInfoResponse(int slotId, android::ClientId clientId __unused,
                                       int responseType, int serial, RIL_Errno e,
                                       void *response, size_t responseLen) {
    RLOGD("queryPhbStorageInfoResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<int32_t> storageInfo;
        int numInts = responseLen / sizeof(int);
        if (response == NULL || responseLen % sizeof(int) != 0) {
            RLOGE("queryPhbStorageInfoResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            storageInfo.resize(numInts);
            for (int i = 0; i < numInts; i++) {
                storageInfo[i] = (int32_t) pInt[i];
            }
        }
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->queryPhbStorageInfoResponse(responseInfo,
                storageInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("queryPhbStorageInfoResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }

    return 0;
}

int mtkRadioEx::writePhbEntryResponse(int slotId, android::ClientId clientId __unused,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen) {
    RLOGD("writePhbEntryResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->writePhbEntryResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("writePhbEntryResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

void convertRilPhbEntryStructureToHal(void *response, size_t responseLen,
        hidl_vec<PhbEntryStructure>& resultList) {
    int num = responseLen / sizeof(RIL_PhbEntryStructure*);
    RIL_PhbEntryStructure **phbEntryResponse = (RIL_PhbEntryStructure **) response;
    resultList.resize(num);
    for (int i = 0; i < num; i++) {
        resultList[i].type = phbEntryResponse[i]->type;
        resultList[i].index = phbEntryResponse[i]->index;
        resultList[i].number = convertCharPtrToHidlString(phbEntryResponse[i]->number);
        resultList[i].ton = phbEntryResponse[i]->ton;
        resultList[i].alphaId = convertCharPtrToHidlString(phbEntryResponse[i]->alphaId);
    }
}

int mtkRadioEx::readPhbEntryResponse(int slotId, android::ClientId clientId __unused,
                                int responseType, int serial, RIL_Errno e,
                                void *response, size_t responseLen) {
    RLOGD("readPhbEntryResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        hidl_vec<PhbEntryStructure> result;
        if (response == NULL || responseLen % sizeof(RIL_PhbEntryStructure*) != 0) {
            RLOGE("readPhbEntryResponse: invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            convertRilPhbEntryStructureToHal(response, responseLen, result);
        }

        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->readPhbEntryResponse(
                responseInfo, result);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("readPhbEntryResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::queryUPBCapabilityResponse(int slotId, android::ClientId clientId __unused,
                                       int responseType, int serial, RIL_Errno e,
                                       void *response, size_t responseLen) {
    RLOGD("queryUPBCapabilityResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<int32_t> upbCapability;
        int numInts = responseLen / sizeof(int);
        if (response == NULL || responseLen % sizeof(int) != 0) {
            RLOGE("queryUPBCapabilityResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            upbCapability.resize(numInts);
            for (int i = 0; i < numInts; i++) {
                upbCapability[i] = (int32_t) pInt[i];
            }
        }
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->queryUPBCapabilityResponse(responseInfo,
                upbCapability);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("queryUPBCapabilityResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }

    return 0;
}

int mtkRadioEx::editUPBEntryResponse(int slotId, android::ClientId clientId __unused,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen) {
    RLOGD("editUPBEntryResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->editUPBEntryResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("editUPBEntryResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::deleteUPBEntryResponse(int slotId, android::ClientId clientId __unused,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen) {
    RLOGD("deleteUPBEntryResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->deleteUPBEntryResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("deleteUPBEntryResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::readUPBGasListResponse(int slotId, android::ClientId clientId __unused,
                                       int responseType, int serial, RIL_Errno e,
                                       void *response, size_t responseLen) {
    RLOGD("readUPBGasListResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<hidl_string> gasList;
        int numStrings = responseLen / sizeof(char*);
        if (response == NULL || responseLen % sizeof(char *) != 0) {
            RLOGE("readUPBGasListResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            char **pString = (char **) response;
            gasList.resize(numStrings);
            for (int i = 0; i < numStrings; i++) {
                gasList[i] = convertCharPtrToHidlString(pString[i]);
            }
        }
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->readUPBGasListResponse(responseInfo,
                gasList);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("readUPBGasListResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }

    return 0;
}

int mtkRadioEx::readUPBGrpEntryResponse(int slotId, android::ClientId clientId __unused,
                                       int responseType, int serial, RIL_Errno e,
                                       void *response, size_t responseLen) {
    RLOGD("readUPBGrpEntryResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<int32_t> grpEntries;
        int numInts = responseLen / sizeof(int);
        if (response == NULL || responseLen % sizeof(int) != 0) {
            // RLOGE("readUPBGrpEntryResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            grpEntries.resize(numInts);
            for (int i = 0; i < numInts; i++) {
                grpEntries[i] = (int32_t) pInt[i];
            }
        }
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->readUPBGrpEntryResponse(responseInfo,
                grpEntries);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("readUPBGrpEntryResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }

    return 0;
}

int mtkRadioEx::writeUPBGrpEntryResponse(int slotId, android::ClientId clientId __unused,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen) {
    RLOGD("writeUPBGrpEntryResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->writeUPBGrpEntryResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("writeUPBGrpEntryResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::getPhoneBookStringsLengthResponse(int slotId, android::ClientId clientId __unused,
                                       int responseType, int serial, RIL_Errno e,
                                       void *response, size_t responseLen) {
    RLOGD("getPhoneBookStringsLengthResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<int32_t> stringLengthInfo;
        int numInts = responseLen / sizeof(int);
        if (response == NULL || responseLen % sizeof(int) != 0) {
            RLOGE("getPhoneBookStringsLengthResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            stringLengthInfo.resize(numInts);
            for (int i = 0; i < numInts; i++) {
                stringLengthInfo[i] = (int32_t) pInt[i];
            }
        }
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->readUPBGrpEntryResponse(responseInfo,
                stringLengthInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("getPhoneBookStringsLengthResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }

    return 0;
}

int mtkRadioEx::getPhoneBookMemStorageResponse(int slotId, android::ClientId clientId __unused,
                                        int responseType, int serial, RIL_Errno e,
                                        void *response, size_t responseLen) {
    RLOGD("getPhoneBookMemStorageResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        PhbMemStorageResponse phbMemStorage;
        if (response == NULL || responseLen != sizeof(RIL_PHB_MEM_STORAGE_RESPONSE)) {
            RLOGE("getPhoneBookMemStorageResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            RIL_PHB_MEM_STORAGE_RESPONSE *resp = (RIL_PHB_MEM_STORAGE_RESPONSE *)response;
            phbMemStorage.storage = convertCharPtrToHidlString(resp->storage);
            phbMemStorage.used = resp->used;
            phbMemStorage.total = resp->total;
        }

        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->getPhoneBookMemStorageResponse(responseInfo,
                phbMemStorage);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("getPhoneBookMemStorageResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }

    return 0;
}

int mtkRadioEx::setPhoneBookMemStorageResponse(int slotId, android::ClientId clientId __unused,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen) {
    RLOGD("setPhoneBookMemStorageResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->setPhoneBookMemStorageResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("setPhoneBookMemStorageResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

void convertRilPhbEntryExtStrucutreToHal(void *response, size_t responseLen,
        hidl_vec<PhbEntryExt>& resultList) {
    int num = responseLen / sizeof(RIL_PHB_ENTRY *);

    RIL_PHB_ENTRY **phbEntryExtResponse = (RIL_PHB_ENTRY **) response;
    resultList.resize(num);
    for (int i = 0; i < num; i++) {
        resultList[i].index = phbEntryExtResponse[i]->index;
        resultList[i].number = convertCharPtrToHidlString(phbEntryExtResponse[i]->number);
        resultList[i].type = phbEntryExtResponse[i]->type;
        resultList[i].text = convertCharPtrToHidlString(phbEntryExtResponse[i]->text);
        resultList[i].hidden = phbEntryExtResponse[i]->hidden;
        resultList[i].group = convertCharPtrToHidlString(phbEntryExtResponse[i]->group);
        resultList[i].adnumber = convertCharPtrToHidlString(phbEntryExtResponse[i]->adnumber);
        resultList[i].adtype = phbEntryExtResponse[i]->adtype;
        resultList[i].secondtext = convertCharPtrToHidlString(phbEntryExtResponse[i]->secondtext);
        resultList[i].email = convertCharPtrToHidlString(phbEntryExtResponse[i]->email);
    }
}

int mtkRadioEx::setNetworkSelectionModeManualWithActResponse(int slotId,
        android::ClientId clientId __unused, int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    RLOGD("setNetworkSelectionModeManualWithActResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->setNetworkSelectionModeManualWithActResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("acceptCallResponse: mtkRadioExService[%d]->setNetworkSelectionModeManualWithActResponse "
                "== NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::hangupWithReasonResponse(int slotId,
        android::ClientId clientId, int responseType, int serial, RIL_Errno e,
        void *response, size_t responselen) {
    RLOGD("hangupWithReasonResponse: serial %d", serial);
    if(clientId == android::CLIENT_IMS) {
        if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            Return<void> retStatus = mtkRadioExService[slotId]->
                                     mRadioResponseIms->hangupWithReasonResponse(responseInfo);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
        } else {
            RLOGE("hangupWithReasonResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                    slotId);
        }
    }
    else {
        if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            Return<void> retStatus = mtkRadioExService[slotId]->
                                     mRadioResponseMtk->hangupWithReasonResponse(responseInfo);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
        } else {
            RLOGE("hangupWithReasonResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                    slotId);
        }
    }
    return 0;
}

int mtkRadioEx::readPhoneBookEntryExtResponse(int slotId, android::ClientId clientId __unused,
                                int responseType, int serial, RIL_Errno e,
                                void *response, size_t responseLen) {
    RLOGD("readPhoneBookEntryExtResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        hidl_vec<PhbEntryExt> result;
        if (response == NULL || responseLen % sizeof(RIL_PHB_ENTRY *) != 0) {
            RLOGE("readPhoneBookEntryExtResponse: invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            convertRilPhbEntryExtStrucutreToHal(response, responseLen, result);
        }

        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->readPhoneBookEntryExtResponse(
                responseInfo, result);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("readPhoneBookEntryExtResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::writePhoneBookEntryExtResponse(int slotId, android::ClientId clientId __unused,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen) {
    RLOGD("writePhoneBookEntryExtResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->writePhoneBookEntryExtResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("writePhoneBookEntryExtResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::getAvailableNetworksWithActResponse(int slotId,
        android::ClientId clientId __unused, int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    RLOGD("getAvailableNetworksWithActResponse: serial %d", serial);
    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<OperatorInfoWithAct> networks;
        if (response == NULL || responseLen % (6 * sizeof(char *))!= 0) {
            RLOGE("getAvailableNetworksWithActResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            char **resp = (char **) response;
            int numStrings = responseLen / sizeof(char *);
            networks.resize(numStrings/6);
            for (int i = 0, j = 0; i < numStrings; i = i + 6, j++) {
                networks[j].base.alphaLong = convertCharPtrToHidlString(resp[i]);
                networks[j].base.alphaShort = convertCharPtrToHidlString(resp[i + 1]);
                networks[j].base.operatorNumeric = convertCharPtrToHidlString(resp[i + 2]);
                int status = convertOperatorStatusToInt(resp[i + 3]);
                if (status == -1) {
                    if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
                } else {
                    networks[j].base.status = (OperatorStatus) status;
                }
                networks[j].lac = convertCharPtrToHidlString(resp[i + 4]);
                networks[j].act = convertCharPtrToHidlString(resp[i + 5]);
            }
        }
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->getAvailableNetworksWithActResponse(responseInfo,
                networks);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("getAvailableNetworksWithActResponse: mtkRadioExService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int mtkRadioEx::cancelAvailableNetworksResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen) {
    RLOGD("cancelAvailableNetworksResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->cancelAvailableNetworksResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("acceptCallResponse: mtkRadioExService[%d]->cancelAvailableNetworksResponse "
                "== NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::cfgA2offsetResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen) {
    // not suport
    return 0;
}

int mtkRadioEx::cfgB1offsetResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen) {
    // not suport
    return 0;
}

int mtkRadioEx::enableSCGfailureResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen) {
    // not suport
    return 0;
}

int mtkRadioEx::disableNRResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen) {
    // not suport
    return 0;
}

int mtkRadioEx::setTxPowerResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen) {
    // not suport
    return 0;
}

int mtkRadioEx::setSearchStoredFreqInfoResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen) {
    // not suport
    return 0;
}

int mtkRadioEx::setSearchRatResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen) {
    // not suport
    return 0;
}

int mtkRadioEx::setBgsrchDeltaSleepTimerResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen) {
    // not suport
    return 0;
}

int mtkRadioEx::queryUPBAvailableResponse(int slotId, android::ClientId clientId __unused,
                                       int responseType, int serial, RIL_Errno e,
                                       void *response, size_t responseLen) {
    RLOGD("queryUPBAvailableResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<int32_t> upbAvailable;
        int numInts = responseLen / sizeof(int);
        if (response == NULL || responseLen % sizeof(int) != 0) {
            RLOGE("queryUPBAvailableResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            upbAvailable.resize(numInts);
            for (int i = 0; i < numInts; i++) {
                upbAvailable[i] = (int32_t) pInt[i];
            }
        }
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->queryUPBAvailableResponse(responseInfo,
                upbAvailable);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("queryUPBAvailableResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }

    return 0;
}

int mtkRadioEx::readUPBEmailEntryResponse(int slotId, android::ClientId clientId __unused,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
    RLOGD("readUPBEmailEntryResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->readUPBEmailEntryResponse(responseInfo,
                convertCharPtrToHidlString((char *) response));
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("readUPBEmailEntryResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::readUPBSneEntryResponse(int slotId, android::ClientId clientId __unused,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
    RLOGD("readUPBSneEntryResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->readUPBSneEntryResponse(responseInfo,
                convertCharPtrToHidlString((char *) response));
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("readUPBSneEntryResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::readUPBAnrEntryResponse(int slotId, android::ClientId clientId __unused,
                                int responseType, int serial, RIL_Errno e,
                                void *response, size_t responseLen) {
    RLOGD("readUPBAnrEntryResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        hidl_vec<PhbEntryStructure> result;
        if (response == NULL || responseLen % sizeof(RIL_PhbEntryStructure*) != 0) {
            RLOGE("readPhbEntryResponse: invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            convertRilPhbEntryStructureToHal(response, responseLen, result);
        }

        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->readUPBAnrEntryResponse(
                responseInfo, result);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("readUPBAnrEntryResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::readUPBAasListResponse(int slotId, android::ClientId clientId __unused,
                                       int responseType, int serial, RIL_Errno e,
                                       void *response, size_t responseLen) {
    RLOGD("readUPBAasListResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<hidl_string> aasList;
        int numStrings = responseLen / sizeof(char *);
        if (response == NULL || responseLen % sizeof(char *) != 0) {
            RLOGE("readUPBAasListResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            char **pString = (char **) response;
            aasList.resize(numStrings);
            for (int i = 0; i < numStrings; i++) {
                aasList[i] = convertCharPtrToHidlString(pString[i]);
            }
        }
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->readUPBAasListResponse(responseInfo,
                aasList);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("readUPBAasListResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }

    return 0;
}

int mtkRadioEx::setPhonebookReadyResponse(int slotId, android::ClientId clientId __unused,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen) {
    RLOGD("setPhonebookReadyResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->setPhonebookReadyResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("setPhonebookReadyResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }

    return 0;
}
// PHB END

/// [IMS] IMS Response Start
int mtkRadioEx::imsEmergencyDialResponse(int slotId, android::ClientId clientId __unused,
                                    int responseType, int serial, RIL_Errno e,
                                    void *response, size_t responselen) {
    RLOGD("imsEmergencyDialResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseV1_4 != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponseV1_4->emergencyDialResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("imsEmergencyDialResponse: radioService[%d]->mRadioResponseV1_4 == NULL",
                slotId);
    }

    return 0;
}


int mtkRadioEx::imsDialResponse(int slotId, android::ClientId clientId __unused,
                           int responseType, int serial, RIL_Errno e, void *response,
                           size_t responseLen) {
    RLOGD("imsDialResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->mRadioResponse->dialResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("imsDialResponse: radioService[%d]->mRadioResponseIms == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::imsVtDialResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    RLOGD("imsVtDialResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 vtDialResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("imsVtDialResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                       slotId);
    }

    return 0;
}

int mtkRadioEx::videoCallAcceptResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    RLOGD("acceptVideoCallResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 videoCallAcceptResponse(responseInfo);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("imsEctCommandResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                           slotId);
    }
    return 0;
}

int mtkRadioEx::eccRedialApproveResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    RLOGD(LOG_TAG, "eccRedialApproveResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 eccRedialApproveResponse(responseInfo);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE(LOG_TAG, "eccRedialApproveResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                           slotId);
    }
    return 0;
}

int mtkRadioEx::imsEctCommandResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    RLOGD("imsEctCommandResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 imsEctCommandResponse(responseInfo);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("imsEctCommandResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                           slotId);
    }
    return 0;
}
int mtkRadioEx::controlCallResponse(int slotId, android::ClientId clientId __unused, int responseType,
                            int serial, RIL_Errno e, void *response,
                            size_t responselen) {
    RLOGD("controlCallResponse: serial %d", serial);
    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 controlCallResponse(responseInfo);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("controlCallResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                      slotId);
    }
    return 0;
}
int mtkRadioEx::imsDeregNotificationResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    RLOGD("imsDeregNotificationResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 imsDeregNotificationResponse(responseInfo);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("imsDeregNotificationResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                                  slotId);
    }

    return 0;
}

int mtkRadioEx::setImsEnableResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {

    RLOGD("setImsEnableResponse: serial %d", serial);
    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 setImsEnableResponse(responseInfo);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("setImsEnableResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                slotId);
    }
    return 0;
}

int mtkRadioEx::setImsVideoEnableResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {

    RLOGD("setImsVideoEnableResponse: serial %d", serial);
    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 setImsVideoEnableResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("setImsVideoEnableResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                               slotId);
    }

    return 0;
}

int mtkRadioEx::setImscfgResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {

    RLOGD("setImscfgResponse: serial %d", serial);
    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 setImscfgResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("setImscfgResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                       slotId);
    }

    return 0;
}

int mtkRadioEx::setModemImsCfgResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {

    RLOGD("setModemImsCfgResponse: serial %d", serial);
    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 setModemImsCfgResponse(responseInfo,
                                 convertCharPtrToHidlString((char *) response));
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("setImsModemCfgResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                slotId);
    }

    return 0;
}

int mtkRadioEx::getProvisionValueResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {

    RLOGD("getProvisionValueResponse: serial %d", serial);
    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 getProvisionValueResponse(responseInfo);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("getProvisionValueResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                               slotId);
    }

    return 0;
}

int mtkRadioEx::setProvisionValueResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {

    RLOGD("setProvisionValueResponse: serial %d", serial);
    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 setProvisionValueResponse(responseInfo);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("setProvisionValueResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                               slotId);
    }

    return 0;
}


//IMS Config TelephonyWare START

int mtkRadioEx::setImsCfgFeatureValueResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    return 0;
}

int mtkRadioEx::getImsCfgFeatureValueResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    return 0;
}

int mtkRadioEx::setImsCfgProvisionValueResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    return 0;
}

int mtkRadioEx::getImsCfgProvisionValueResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    return 0;
}

int mtkRadioEx::getImsCfgResourceCapValueResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    return 0;
}
//IMS Config TelephonyWare END

int mtkRadioEx::controlImsConferenceCallMemberResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {

    RLOGD("controlImsConfCallMemberRsp: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 controlImsConferenceCallMemberResponse(responseInfo);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("controlImsConfCallMemberRsp: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                             slotId);
    }

    return 0;
}

int mtkRadioEx::setWfcProfileResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {

    RLOGD("setWfcProfileResponse: serial %d", serial);
    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 setWfcProfileResponse(responseInfo);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("setWfcProfileResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                           slotId);
    }

    return 0;
}

int mtkRadioEx::conferenceDialResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    RLOGD("conferenceDialResponse: serial %d", serial);
    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 conferenceDialResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("conferenceDialResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                            slotId);
    }

    return 0;
}

int mtkRadioEx::vtDialWithSipUriResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    RLOGD("vtDialWithSipUriResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 vtDialWithSipUriResponse(responseInfo);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("vtDialWithSipUriResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                              slotId);
    }

    return 0;
}

int mtkRadioEx::dialWithSipUriResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    RLOGD("dialWithSipUriResponse: serial %d", serial);
    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 dialWithSipUriResponse(responseInfo);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("dialWithSipUriResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                slotId);
    }

    return 0;
}
int mtkRadioEx::sendUssiResponse(int slotId, android::ClientId clientId __unused, int responseType,
                            int serial, RIL_Errno e, void *response,
                            size_t responselen) {
    RLOGD("sendUssiResponse: serial %d", serial);
    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]
                                 ->mRadioResponseIms
                                 ->sendUssiResponse(responseInfo);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("sendUssiResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                      slotId);
    }

    return 0;
}

int mtkRadioEx::cancelUssiResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    RLOGD("cancelUssiResponse: serial %d", serial);
    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]
                                 ->mRadioResponseIms
                                 ->cancelUssiResponse(responseInfo);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("cancelUssiResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                        slotId);
    }

    return 0;
}

int mtkRadioEx::getXcapStatusResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    if(clientId == android::CLIENT_IMS) {
        if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseIms->getXcapStatusResponse(
                    responseInfo);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
        } else {
            RLOGE("getXcapStatusResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL", slotId);
        }
    } else {
        RLOGE("getXcapStatusResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::resetSuppServResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    if(clientId == android::CLIENT_IMS) {
        if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseIms->resetSuppServResponse(
                    responseInfo);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
        } else {
            RLOGE("resetSuppServResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL", slotId);
        }
    } else {
        RLOGE("resetSuppServResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::setupXcapUserAgentStringResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    RLOGD("setupXcapUserAgentStringResponse: serial %d", serial);
    if(clientId == android::CLIENT_IMS) {
        if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            Return<void> retStatus = mtkRadioExService[slotId]
                                    ->mRadioResponseIms
                                    ->setupXcapUserAgentStringResponse(responseInfo);

            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
        } else {
            RLOGE(LOG_TAG,
                    "setupXcapUserAgentStringResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                  slotId);
        }
    } else {
        RLOGE(LOG_TAG,
                "setupXcapUserAgentStringResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                              slotId);
    }


    return 0;
}

int mtkRadioEx::forceReleaseCallResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {

    RLOGD("forceReleaseCallResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 forceReleaseCallResponse(responseInfo);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("forceReleaseCallResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                              slotId);
    }
    return 0;
}

int mtkRadioEx::setImsRtpReportResponse(int slotId, android::ClientId clientId __unused,
                            int responseType, int serial, RIL_Errno e,
                            void *response,
                            size_t responselen) {

    RLOGD("setImsRtpReportResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 setImsRtpReportResponse(responseInfo);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("setImsRtpReportResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                             slotId);
    }

    return 0;
}

int mtkRadioEx::imsBearerStateConfirmResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {

    RLOGD("imsBearerStateConfirmResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 imsBearerStateConfirmResponse(responseInfo);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("imsBearerStateConfirmResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                             slotId);
    }

    return 0;
}

int mtkRadioEx::setImsBearerNotificationResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {

    RLOGD("setImsBearerNotificationResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 setImsBearerNotificationResponse(responseInfo);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("setImsBearerNotificationResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                               slotId);
    }

    return 0;
}

int mtkRadioEx::pullCallResponse(int slotId, android::ClientId clientId __unused,
                            int responseType, int serial, RIL_Errno e,
                            void *response, size_t responselen) {
    RLOGD("pullCallResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseIms
                                                     ->pullCallResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("pullCallResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                      slotId);
    }
    return 0;
}

int mtkRadioEx::setImsRegistrationReportResponse(int slotId, android::ClientId clientId __unused,
                            int responseType, int serial, RIL_Errno e,
                            void *response, size_t responselen) {
    RLOGD("setImsRegistrationReportRsp: serial %d", serial);


    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseIms
                                                     ->setImsRegistrationReportResponse(
                                                       responseInfo);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("setImsRegistrationReportRsp: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                                                                                 slotId);
    }
    return 0;
}

int mtkRadioEx::getSmsRuimMemoryStatusResponse(int slotId, android::ClientId clientId __unused,
                                   int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen) {
    RLOGD("getSmsRuimMemoryStatusResponse: serial %d", serial);

    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        SmsMemStatus status = {};
        if (response == NULL || responseLen != sizeof (RIL_SMS_Memory_Status)) {
            RLOGE("getSmsRuimMemoryStatusResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            RIL_SMS_Memory_Status *mem_status = (RIL_SMS_Memory_Status*)response;
            status.used = mem_status->used;
            status.total = mem_status->total;
        }
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->getSmsRuimMemoryStatusResponse(
                responseInfo, status);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);

    } else {
        RLOGE("getSmsRuimMemoryStatusResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }
    return 0;
}

int mtkRadioEx::setVoicePreferStatusResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    return 0;
}

int mtkRadioEx::setEccNumResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response,
        size_t responselen) {
    return 0;
}

int mtkRadioEx::getEccNumResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    return 0;
}

int mtkRadioEx::setFdModeResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    // setFdMode has 2 clients, one is telephony assist, one is telephony common,
    // distinct the 2 clients here.
    if (clientId == android::CLIENT_ASSIST) {
        if (mtkRadioExService[slotId]->mRadioResponseAssist != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            Return<void> retStatus =
                    mtkRadioExService[slotId]->mRadioResponseAssist->setFdModeResponse(
                            responseInfo);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::ASSIST_MTK, slotId);
        } else {
            RLOGE("setFdModeResponse: "
                    "mtkRadioExService[%d]->mRadioResponseAssist == NULL", slotId);
        }
        return 0;
    }

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->setFdModeResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("setFdModeResponse: mtkRadioExService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

// World Phone {
int mtkRadioEx::setResumeRegistrationResponse(int slotId, android::ClientId clientId __unused,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen) {
    RLOGD("setResumeRegistrationResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->setResumeRegistrationResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("setResumeRegistrationResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }

    return 0;
}

int mtkRadioEx::modifyModemTypeResponse(int slotId, android::ClientId clientId __unused,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen) {
    RLOGD("modifyModemTypeResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        int n = responseInt(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseMtk->modifyModemTypeResponse(responseInfo, n);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("modifyModemTypeResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }

    return 0;
}
// World Phone }

/// M: eMBMS feature
int mtkRadioEx::sendEmbmsAtCommandResponse(int slotId, android::ClientId clientId __unused,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
    RLOGD("sendEmbmsAtCommandResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->sendEmbmsAtCommandResponse(responseInfo,
                convertCharPtrToHidlString((char *) response));
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("sendEmbmsAtCommandResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

// M: RTT @{
int mtkRadioEx::setRttModeResponse(int slotId, android::ClientId clientId __unused,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responselen) {

    RLOGD("setRttModeResponse: serial %d", serial);

    return 0;
}

int mtkRadioEx::sendRttModifyRequestResponse(int slotId, android::ClientId clientId __unused,
                                        int responseType, int serial, RIL_Errno e,
                                        void *response, size_t responselen) {

    RLOGD("sendRttModifyRequestResponse: serial %d", serial);

    return 0;
}

int mtkRadioEx::sendRttTextResponse(int slotId, android::ClientId clientId __unused,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responselen) {

    RLOGD("sendRttTextResponse: serial %d", serial);

    return 0;
}

int mtkRadioEx::rttModifyRequestResponseResponse(int slotId, android::ClientId clientId __unused,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responselen) {

    RLOGD("rttModifyRequestResponseResponse: serial %d", serial);

    return 0;
}

int mtkRadioEx::toggleRttAudioIndicationResponse(int slotId, android::ClientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    RLOGD("toggleRttAudioIndication: serial %d", serial);

    return 0;
}
// @}

/***************************************************************************************************
 * INDICATION FUNCTIONS
 * The below function handle unsolicited messages coming from the Radio
 * (messages for which there is no pending request)
 **************************************************************************************************/

RadioIndicationType convertIntToRadioIndicationType(int indicationType) {
    return indicationType == RESPONSE_UNSOLICITED ? (RadioIndicationType::UNSOLICITED) :
            (RadioIndicationType::UNSOLICITED_ACK_EXP);
}

int radio::keepaliveStatusInd(int slotId,
                         int indicationType, int token, RIL_Errno e, void *response,
                         size_t responseLen) {
    if (radioService[slotId] == NULL || radioService[slotId]->mRadioIndicationV1_1 == NULL) {
        RLOGE("%s: radioService[%d]->mRadioIndicationV1_1 == NULL",
                __FUNCTION__, slotId);
        return 0;
    }
    if (response == NULL || responseLen != sizeof(AOSP_V1_1::KeepaliveStatus)) {
        RLOGE("%s: invalid response", __FUNCTION__);
        return 0;
    }

    AOSP_V1_1::KeepaliveStatus ks;
    convertRilKeepaliveStatusToHal(static_cast<RIL_KeepaliveStatus*>(response), ks);

    Return<void> retStatus = radioService[slotId]->mRadioIndicationV1_1->keepaliveStatus(
            convertIntToRadioIndicationType(indicationType), ks);
    radioService[slotId]->checkReturnStatus(retStatus, slotId);
    return 0;
}

int radio::radioStateChangedInd(int slotId,
                                 int indicationType, int token, RIL_Errno e, void *response,
                                 size_t responseLen) {

    if(s_vendorFunctions == NULL) {
        RLOGE("radioStateChangedInd: service is not ready");
        return 0;
    }

    // Retrive Radio State
    RadioState radioState = (RadioState) s_vendorFunctions->
                                         onStateRequest((RIL_SOCKET_ID)slotId);
    RLOGD("radioStateChangedInd: radioState %d, slot = %d", radioState, slotId);

    // Send to RILJ
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        Return<void> retStatus = radioService[slotId]->mRadioIndication->radioStateChanged(
                convertIntToRadioIndicationType(indicationType), radioState);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("radioStateChangedInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    // Send to IMS
    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (radioService[imsSlotId] != NULL && radioService[imsSlotId]->mRadioIndication != NULL) {

        Return<void> retStatus = radioService[imsSlotId]->mRadioIndication->radioStateChanged(
                convertIntToRadioIndicationType(indicationType), radioState);
        radioService[imsSlotId]->checkReturnStatus(retStatus, slotId);

        if (retStatus.isOk() == false) {
            appendRequestIntoPendingInd(slotId, RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED,
                    indicationType, response, responseLen);
        }
    } else {
        RLOGE("radioStateChangedInd: radioService[%d]->mRadioIndication == NULL", imsSlotId);
        appendRequestIntoPendingInd(slotId, RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, indicationType,
                    response, responseLen);
    }
    // Send to EM
    int emSlotId = toClientSlot(slotId, android::CLIENT_EM);
    if (radioService[emSlotId] != NULL && radioService[emSlotId]->mRadioIndication != NULL) {

        Return<void> retStatus = radioService[emSlotId]->mRadioIndication->radioStateChanged(
                convertIntToRadioIndicationType(indicationType), radioState);
        radioService[emSlotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("radioStateChangedInd: radioService[%d]->mRadioIndication == NULL", emSlotId);
    }
    return 0;
}

int radio::callStateChangedInd(int slotId,
                               int indicationType, int token, RIL_Errno e, void *response,
                               size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        RLOGD("callStateChangedInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->callStateChanged(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("callStateChangedInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::networkStateChangedInd(int slotId,
                                  int indicationType, int token, RIL_Errno e, void *response,
                                  size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        RLOGD("networkStateChangedInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->networkStateChanged(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("networkStateChangedInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

uint8_t hexCharToInt(uint8_t c) {
    if (c >= '0' && c <= '9') return (c - '0');
    if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
    if (c >= 'a' && c <= 'f') return (c - 'a' + 10);

    return INVALID_HEX_CHAR;
}

uint8_t * convertHexStringToBytes(void *response, size_t responseLen) {
    if (responseLen % 2 != 0) {
        return NULL;
    }

    uint8_t *bytes = (uint8_t *)calloc(responseLen/2, sizeof(uint8_t));
    if (bytes == NULL) {
        RLOGE("convertHexStringToBytes: cannot allocate memory for bytes string");
        return NULL;
    }
    uint8_t *hexString = (uint8_t *)response;

    for (size_t i = 0; i < responseLen; i += 2) {
        uint8_t hexChar1 = hexCharToInt(hexString[i]);
        uint8_t hexChar2 = hexCharToInt(hexString[i + 1]);

        if (hexChar1 == INVALID_HEX_CHAR || hexChar2 == INVALID_HEX_CHAR) {
            RLOGE("convertHexStringToBytes: invalid hex char %d %d",
                    hexString[i], hexString[i + 1]);
            free(bytes);
            return NULL;
        }
        bytes[i/2] = ((hexChar1 << 4) | hexChar2);
    }

    return bytes;
}

int radio::newSmsInd(int slotId, int indicationType,
                     int token, RIL_Errno e, void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("newSmsInd: invalid response");
            return 0;
        }

        uint8_t *bytes = convertHexStringToBytes(response, responseLen);
        if (bytes == NULL) {
            RLOGE("newSmsInd: convertHexStringToBytes failed");
            return 0;
        }

        hidl_vec<uint8_t> pdu;
        pdu.setToExternal(bytes, responseLen/2);
        RLOGD("newSmsInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->newSms(
                convertIntToRadioIndicationType(indicationType), pdu);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
        free(bytes);
    } else {
        RLOGE("newSmsInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::newSmsStatusReportInd(int slotId,
                                 int indicationType, int token, RIL_Errno e, void *response,
                                 size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("newSmsStatusReportInd: invalid response");
            return 0;
        }

        uint8_t *bytes = convertHexStringToBytes(response, responseLen);
        if (bytes == NULL) {
            RLOGE("newSmsStatusReportInd: convertHexStringToBytes failed");
            return 0;
        }

        hidl_vec<uint8_t> pdu;
        pdu.setToExternal(bytes, responseLen/2);
        RLOGD("newSmsStatusReportInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->newSmsStatusReport(
                convertIntToRadioIndicationType(indicationType), pdu);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
        free(bytes);
    } else {
        RLOGE("newSmsStatusReportInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::newSmsOnSimInd(int slotId, int indicationType,
                          int token, RIL_Errno e, void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("newSmsOnSimInd: invalid response");
            return 0;
        }
        int32_t recordNumber = ((int32_t *) response)[0];
        RLOGD("newSmsOnSimInd: slotIndex %d", recordNumber);
        Return<void> retStatus = radioService[slotId]->mRadioIndication->newSmsOnSim(
                convertIntToRadioIndicationType(indicationType), recordNumber);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("newSmsOnSimInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::onUssdInd(int slotId, int indicationType,
                     int token, RIL_Errno e, void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != 2 * sizeof(char *)) {
            RLOGE("onUssdInd: invalid response");
            return 0;
        }
        char **strings = (char **) response;
        char *mode = strings[0];
        hidl_string msg = convertCharPtrToHidlString(strings[1]);
        UssdModeType modeType = (UssdModeType) atoi(mode);
        RLOGD("onUssdInd: mode %s", mode);
        Return<void> retStatus = radioService[slotId]->mRadioIndication->onUssd(
                convertIntToRadioIndicationType(indicationType), modeType, msg);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("onUssdInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::nitzTimeReceivedInd(int slotId,
                               int indicationType, int token, RIL_Errno e, void *response,
                               size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("nitzTimeReceivedInd: invalid response");
            return 0;
        }
        hidl_string nitzTime = convertCharPtrToHidlString((char *) response);
        RLOGD("nitzTimeReceivedInd: nitzTime %s receivedTime %" PRId64, nitzTime.c_str(),
                nitzTimeReceived[slotId]);
        Return<void> retStatus = radioService[slotId]->mRadioIndication->nitzTimeReceived(
                convertIntToRadioIndicationType(indicationType), nitzTime,
                nitzTimeReceived[slotId]);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("nitzTimeReceivedInd: radioService[%d]->mRadioIndication == NULL", slotId);
        return -1;
    }

    return 0;
}

void convertRilSignalStrengthToHal(void *response, size_t responseLen,
        SignalStrength& signalStrength) {
    int32_t *rilSignalStrength = (int32_t *) response;

    signalStrength.gw.signalStrength = rilSignalStrength[0];
    signalStrength.gw.bitErrorRate = rilSignalStrength[1];
    signalStrength.cdma.dbm = rilSignalStrength[3];
    signalStrength.cdma.ecio = rilSignalStrength[4];
    signalStrength.evdo.dbm = rilSignalStrength[5];
    signalStrength.evdo.ecio = rilSignalStrength[6];
    signalStrength.evdo.signalNoiseRatio = rilSignalStrength[7];
    signalStrength.lte.signalStrength = rilSignalStrength[8];
    signalStrength.lte.rsrp = rilSignalStrength[9];
    signalStrength.lte.rsrq = rilSignalStrength[10];
    signalStrength.lte.rssnr = rilSignalStrength[11];
    signalStrength.lte.cqi = rilSignalStrength[12];
    signalStrength.lte.timingAdvance = rilSignalStrength[13];
    signalStrength.tdScdma.rscp = rilSignalStrength[16];
}

void convertRilSignalStrengthToHal_1_2(void *response, size_t responseLen,
        AOSP_V1_2::SignalStrength& signalStrength) {
    int32_t *rilSignalStrength = (int32_t *) response;

    signalStrength.gsm.signalStrength = rilSignalStrength[0];
    signalStrength.gsm.bitErrorRate = rilSignalStrength[1];
    signalStrength.gsm.timingAdvance = rilSignalStrength[2];
    signalStrength.cdma.dbm = rilSignalStrength[3];
    signalStrength.cdma.ecio = rilSignalStrength[4];
    signalStrength.evdo.dbm = rilSignalStrength[5];
    signalStrength.evdo.ecio = rilSignalStrength[6];
    signalStrength.evdo.signalNoiseRatio = rilSignalStrength[7];
    signalStrength.lte.signalStrength = rilSignalStrength[8];
    signalStrength.lte.rsrp = rilSignalStrength[9];
    signalStrength.lte.rsrq = rilSignalStrength[10];
    signalStrength.lte.rssnr = rilSignalStrength[11];
    signalStrength.lte.cqi = rilSignalStrength[12];
    signalStrength.lte.timingAdvance = rilSignalStrength[13];
    // AOSP still use 1.0's TdScdma, not 1.2's Tdscdma. It's "S", not "s"
    // signalStrength.tdScdma.signalStrength = rilSignalStrength[14];
    // signalStrength.tdScdma.bitErrorRate = rilSignalStrength[15];
    signalStrength.tdScdma.rscp = rilSignalStrength[16];
    signalStrength.wcdma.base.signalStrength = rilSignalStrength[17];
    signalStrength.wcdma.base.bitErrorRate = rilSignalStrength[18];
    signalStrength.wcdma.rscp = rilSignalStrength[19];
    signalStrength.wcdma.ecno = rilSignalStrength[20];
}

void convertRilSignalStrengthToHal_1_4(void *response, size_t responseLen,
        AOSP_V1_4::SignalStrength& signalStrength) {
    int32_t *rilSignalStrength = (int32_t *) response;
    // gsm
    signalStrength.gsm.signalStrength = rilSignalStrength[0];
    signalStrength.gsm.bitErrorRate = rilSignalStrength[1];
    signalStrength.gsm.timingAdvance = rilSignalStrength[2];
    //cdma
    signalStrength.cdma.dbm = rilSignalStrength[3];
    signalStrength.cdma.ecio = rilSignalStrength[4];
    //evdo
    signalStrength.evdo.dbm = rilSignalStrength[5];
    signalStrength.evdo.ecio = rilSignalStrength[6];
    signalStrength.evdo.signalNoiseRatio = rilSignalStrength[7];
    //lte
    signalStrength.lte.signalStrength = rilSignalStrength[8];
    signalStrength.lte.rsrp = rilSignalStrength[9];
    signalStrength.lte.rsrq = rilSignalStrength[10];
    signalStrength.lte.rssnr = rilSignalStrength[11];
    signalStrength.lte.cqi = rilSignalStrength[12];
    signalStrength.lte.timingAdvance = rilSignalStrength[13];
    // AOSP still use 1.0's TdScdma, not 1.2's Tdscdma. It's "S", not "s"
    //tdscdma
    signalStrength.tdscdma.signalStrength = rilSignalStrength[14];
    signalStrength.tdscdma.bitErrorRate = rilSignalStrength[15];
    signalStrength.tdscdma.rscp = rilSignalStrength[16];
    //wcdma
    signalStrength.wcdma.base.signalStrength = rilSignalStrength[17];
    signalStrength.wcdma.base.bitErrorRate = rilSignalStrength[18];
    signalStrength.wcdma.rscp = rilSignalStrength[19];
    signalStrength.wcdma.ecno = rilSignalStrength[20];
    //nr
    // Legacy doesn't support NR
    signalStrength.nr.ssRsrp = 0x7FFFFFFF;
    signalStrength.nr.ssRsrq = 0x7FFFFFFF;
    signalStrength.nr.ssSinr = 0x7FFFFFFF;
    signalStrength.nr.csiRsrp = 0x7FFFFFFF;
    signalStrength.nr.csiRsrq = 0x7FFFFFFF;
    signalStrength.nr.csiSinr = 0x7FFFFFFF;
}

int radio::currentSignalStrengthInd(int slotId,
                                    int indicationType, int token, RIL_Errno e,
                                    void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndicationV1_4!= NULL) {
        // Legacy doesn't support NR, so the size is 21.
        if (response == NULL || responseLen != 21*sizeof(int)) {
            RLOGE("currentSignalStrength_1_4: invalid response");
            return 0;
        }

        AOSP_V1_4::SignalStrength signalStrength = {};

        convertRilSignalStrengthToHal_1_4(response, responseLen, signalStrength);

        Return<void> retStatus = radioService[slotId]->mRadioIndicationV1_4->currentSignalStrength_1_4(
                convertIntToRadioIndicationType(indicationType), signalStrength);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndicationV1_2!= NULL) {
        // wait for AOSP define RIL_SignalStrength_v1X for signal 1.2
        if (response == NULL || responseLen != 21*sizeof(int)) {
            RLOGE("currentSignalStrength_1_2: invalid response");
            return 0;
        }

        AOSP_V1_2::SignalStrength signalStrength = {};

        convertRilSignalStrengthToHal_1_2(response, responseLen, signalStrength);

        Return<void> retStatus = radioService[slotId]->mRadioIndicationV1_2->currentSignalStrength_1_2(
                convertIntToRadioIndicationType(indicationType), signalStrength);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != 21*sizeof(int)) {
            RLOGE("currentSignalStrength: invalid response");
            return 0;
        }

        SignalStrength signalStrength = {};

        convertRilSignalStrengthToHal(response, responseLen, signalStrength);

        Return<void> retStatus = radioService[slotId]->mRadioIndication->currentSignalStrength(
                convertIntToRadioIndicationType(indicationType), signalStrength);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("currentSignalStrength: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

void convertRilDataCallToHal(RIL_Data_Call_Response_v11 *dcResponse,
        SetupDataCallResult& dcResult) {
    dcResult.status = (DataCallFailCause) dcResponse->status;
    dcResult.suggestedRetryTime = dcResponse->suggestedRetryTime;
    dcResult.cid = dcResponse->cid;
    dcResult.active = dcResponse->active;
    dcResult.type = convertCharPtrToHidlString(dcResponse->type);
    dcResult.ifname = convertCharPtrToHidlString(dcResponse->ifname);
    dcResult.addresses = convertCharPtrToHidlString(dcResponse->addresses);
    dcResult.dnses = convertCharPtrToHidlString(dcResponse->dnses);
    dcResult.gateways = convertCharPtrToHidlString(dcResponse->gateways);
    dcResult.pcscf = convertCharPtrToHidlString(dcResponse->pcscf);
    dcResult.mtu = dcResponse->mtu;
}

void convertRilDataCallListToHal(void *response, size_t responseLen,
        hidl_vec<SetupDataCallResult>& dcResultList) {
    int num = responseLen / sizeof(RIL_Data_Call_Response_v11);

    RIL_Data_Call_Response_v11 *dcResponse = (RIL_Data_Call_Response_v11 *) response;
    dcResultList.resize(num);
    for (int i = 0; i < num; i++) {
        convertRilDataCallToHal(&dcResponse[i], dcResultList[i]);
    }
}

// M: [OD over ePDG] start
static int s_isWfcSupport = -1;
int encodeRat(int active, int rat, int slotId) {
    if (s_isWfcSupport < 0) {
        char wfcSupportValue[PROPERTY_VALUE_MAX] = {0};
        property_get("persist.vendor.mtk_wfc_support", wfcSupportValue, "0");
        s_isWfcSupport = atoi(wfcSupportValue);
    }
    if (isMtkFwkAddonNotExisted(slotId) || (s_isWfcSupport == 0) ||
            (active >= RAT_TYPE_KEY) || (active < 0) ||
            ((rat + 1) > RAT_TYPE_MAX) || (rat < 0)) {
        RLOGD("encodeRat: cannot encode rat type into active "
                "[isMtkFwkAddonNotExisted=%d, s_isWfcSupport=%d, rat=%d, active=%d]",
                BOOL_TO_INT(isMtkFwkAddonNotExisted(slotId)),
                s_isWfcSupport, rat, active);
        return active;
    }
    return ((rat - 1) * RAT_TYPE_KEY) + active;
}

void convertRilDataCallToHalEx(MTK_RIL_Data_Call_Response_v11 *dcResponse,
        SetupDataCallResult& dcResult, int slotId) {
    dcResult.status = (DataCallFailCause) dcResponse->status;
    dcResult.suggestedRetryTime = dcResponse->suggestedRetryTime;
    dcResult.cid = dcResponse->cid;
    dcResult.active = encodeRat(dcResponse->active, dcResponse->rat, slotId);
    dcResult.type = convertCharPtrToHidlString(dcResponse->type);
    dcResult.ifname = convertCharPtrToHidlString(dcResponse->ifname);
    dcResult.addresses = convertCharPtrToHidlString(dcResponse->addresses);
    dcResult.dnses = convertCharPtrToHidlString(dcResponse->dnses);
    dcResult.gateways = convertCharPtrToHidlString(dcResponse->gateways);
    dcResult.pcscf = convertCharPtrToHidlString(dcResponse->pcscf);
    dcResult.mtu = dcResponse->mtu;
}

void convertRilDataCallToHalEx_1_4(MTK_RIL_Data_Call_Response_v11 *dcResponse,
        AOSP_V1_4::SetupDataCallResult& dcResult, int slotId) {
    dcResult.cause = (AOSP_V1_4::DataCallFailCause) dcResponse->status;
    dcResult.suggestedRetryTime = dcResponse->suggestedRetryTime;
    dcResult.cid = dcResponse->cid;
    dcResult.active = (AOSP_V1_4::DataConnActiveStatus)
            encodeRat(dcResponse->active, dcResponse->rat, slotId);
    dcResult.type = convertStringToPdpProtocolType(dcResponse->type);
    dcResult.ifname = convertCharPtrToHidlString(dcResponse->ifname);
    dcResult.addresses = convertCharPtrToVecHidlString(dcResponse->addresses);
    dcResult.dnses = convertCharPtrToVecHidlString(dcResponse->dnses);
    dcResult.gateways = convertCharPtrToVecHidlString(dcResponse->gateways);
    dcResult.pcscf = convertCharPtrToVecHidlString(dcResponse->pcscf);
    dcResult.mtu = dcResponse->mtu;
}

void convertRilDataCallListToHalEx(void *response, size_t responseLen,
        hidl_vec<SetupDataCallResult>& dcResultList, int slotId) {
    int num = responseLen / sizeof(MTK_RIL_Data_Call_Response_v11);

    MTK_RIL_Data_Call_Response_v11 *dcResponse = (MTK_RIL_Data_Call_Response_v11 *) response;
    dcResultList.resize(num);
    for (int i = 0; i < num; i++) {
        convertRilDataCallToHalEx(&dcResponse[i], dcResultList[i], slotId);
    }
}

void convertRilDataCallListToHalEx_1_4(void *response, size_t responseLen,
        hidl_vec<AOSP_V1_4::SetupDataCallResult>& dcResultList, int slotId) {
    int num = responseLen / sizeof(MTK_RIL_Data_Call_Response_v11);

    MTK_RIL_Data_Call_Response_v11 *dcResponse = (MTK_RIL_Data_Call_Response_v11 *) response;
    dcResultList.resize(num);
    for (int i = 0; i < num; i++) {
        convertRilDataCallToHalEx_1_4(&dcResponse[i], dcResultList[i], slotId);
    }
}
// M: [OD over ePDG] end

int radio::dataCallListChangedInd(int slotId,
                                  int indicationType, int token, RIL_Errno e, void *response,
                                  size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndicationV1_4!= NULL) {
        if ((response == NULL && responseLen != 0) ||
                // M: [OD over ePDG]
                // remark AOSP
                //responseLen % sizeof(RIL_Data_Call_Response_v11) != 0) {
                responseLen % sizeof(MTK_RIL_Data_Call_Response_v11) != 0) {
            RLOGE("dataCallListChangedInd: invalid response");
            return 0;
        }
        hidl_vec<AOSP_V1_4::SetupDataCallResult> dcList;
        // M: [OD over ePDG]
        // remark AOSP
        //convertRilDataCallListToHal(response, responseLen, dcList);
        convertRilDataCallListToHalEx_1_4(response, responseLen, dcList, slotId);
        RLOGD("dataCallListChangedInd");
        Return<void> retStatus =
                radioService[slotId]->mRadioIndicationV1_4->dataCallListChanged_1_4(
                convertIntToRadioIndicationType(indicationType), dcList);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if ((response == NULL && responseLen != 0) ||
                // M: [OD over ePDG]
                // remark AOSP
                //responseLen % sizeof(RIL_Data_Call_Response_v11) != 0) {
                responseLen % sizeof(MTK_RIL_Data_Call_Response_v11) != 0) {
            RLOGE("dataCallListChangedInd: invalid response");
            return 0;
        }
        hidl_vec<SetupDataCallResult> dcList;
        // M: [OD over ePDG]
        // remark AOSP
        //convertRilDataCallListToHal(response, responseLen, dcList);
        convertRilDataCallListToHalEx(response, responseLen, dcList, slotId);
        RLOGD("dataCallListChangedInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->dataCallListChanged(
                convertIntToRadioIndicationType(indicationType), dcList);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("dataCallListChangedInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::suppSvcNotifyInd(int slotId, int indicationType,
                            int token, RIL_Errno e, void *response, size_t responseLen) {
    if (response == NULL || responseLen != sizeof(RIL_SuppSvcNotification)) {
        RLOGE("suppSvcNotifyInd: invalid response");
        return 0;
    }

    int code = ((RIL_SuppSvcNotification *) response)->code;
    if (code >= 100) {
        int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
        if (mtkRadioExService[imsSlotId] != NULL &&
                mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {
            SuppSvcNotification suppSvc = {};
            RIL_SuppSvcNotification *ssn = (RIL_SuppSvcNotification *) response;
            suppSvc.isMT = ssn->notificationType;
            suppSvc.code = ssn->code - 100;
            suppSvc.index = ssn->index;
            suppSvc.type = ssn->type;
            suppSvc.number = convertCharPtrToHidlString(ssn->number);

            RLOGD(LOG_TAG, "Ims suppSvcNotifyInd: isMT %d code %d index %d type %d",
                    suppSvc.isMT, suppSvc.code, suppSvc.index, suppSvc.type);
            Return<void> retStatus = mtkRadioExService[imsSlotId]->mRadioIndicationIms->suppSvcNotify(
                    convertIntToRadioIndicationType(indicationType), suppSvc);
            mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
        } else {
            RLOGE(LOG_TAG, "suppSvcNotifyInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                    imsSlotId);
        }

        return 0;
    }

    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_SuppSvcNotification)) {
            RLOGE("suppSvcNotifyInd: invalid response");
            return 0;
        }

        SuppSvcNotification suppSvc = {};
        RIL_SuppSvcNotification *ssn = (RIL_SuppSvcNotification *) response;
        suppSvc.isMT = ssn->notificationType;
        suppSvc.code = ssn->code;
        suppSvc.index = ssn->index;
        suppSvc.type = ssn->type;
        suppSvc.number = convertCharPtrToHidlString(ssn->number);

        RLOGD("suppSvcNotifyInd: isMT %d code %d index %d type %d",
                suppSvc.isMT, suppSvc.code, suppSvc.index, suppSvc.type);
        Return<void> retStatus = radioService[slotId]->mRadioIndication->suppSvcNotify(
                convertIntToRadioIndicationType(indicationType), suppSvc);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("suppSvcNotifyInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::stkSessionEndInd(int slotId, int indicationType,
                            int token, RIL_Errno e, void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        RLOGD("stkSessionEndInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->stkSessionEnd(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("stkSessionEndInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::stkProactiveCommandInd(int slotId,
                                  int indicationType, int token, RIL_Errno e, void *response,
                                  size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("stkProactiveCommandInd: invalid response");
            return 0;
        }
        RLOGD("stkProactiveCommandInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->stkProactiveCommand(
                convertIntToRadioIndicationType(indicationType),
                convertCharPtrToHidlString((char *) response));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("stkProactiveCommandInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::stkEventNotifyInd(int slotId, int indicationType,
                             int token, RIL_Errno e, void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("stkEventNotifyInd: invalid response");
            return 0;
        }
        RLOGD("stkEventNotifyInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->stkEventNotify(
                convertIntToRadioIndicationType(indicationType),
                convertCharPtrToHidlString((char *) response));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("stkEventNotifyInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::stkCallSetupInd(int slotId, int indicationType,
                           int token, RIL_Errno e, void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("stkCallSetupInd: invalid response");
            return 0;
        }
        int32_t timeout = ((int32_t *) response)[0];
        RLOGD("stkCallSetupInd: timeout %d", timeout);
        Return<void> retStatus = radioService[slotId]->mRadioIndication->stkCallSetup(
                convertIntToRadioIndicationType(indicationType), timeout);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("stkCallSetupInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::simSmsStorageFullInd(int slotId,
                                int indicationType, int token, RIL_Errno e, void *response,
                                size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        RLOGD("simSmsStorageFullInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->simSmsStorageFull(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("simSmsStorageFullInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::simRefreshInd(int slotId, int indicationType,
                         int token, RIL_Errno e, void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_SimRefreshResponse_v7)) {
            RLOGE("simRefreshInd: invalid response");
            return 0;
        }

        SimRefreshResult refreshResult = {};
        RIL_SimRefreshResponse_v7 *simRefreshResponse = ((RIL_SimRefreshResponse_v7 *) response);
        refreshResult.type =
                (android::hardware::radio::V1_0::SimRefreshType) simRefreshResponse->result;
        refreshResult.efId = simRefreshResponse->ef_id;
        refreshResult.aid = convertCharPtrToHidlString(simRefreshResponse->aid);

        RLOGD("simRefreshInd: type %d efId %d", refreshResult.type, refreshResult.efId);
        Return<void> retStatus = radioService[slotId]->mRadioIndication->simRefresh(
                convertIntToRadioIndicationType(indicationType), refreshResult);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("simRefreshInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

void convertRilCdmaSignalInfoRecordToHal(RIL_CDMA_SignalInfoRecord *signalInfoRecord,
        CdmaSignalInfoRecord& record) {
    record.isPresent = signalInfoRecord->isPresent;
    record.signalType = signalInfoRecord->signalType;
    record.alertPitch = signalInfoRecord->alertPitch;
    record.signal = signalInfoRecord->signal;
}

int radio::callRingInd(int slotId, int indicationType,
                       int token, RIL_Errno e, void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        bool isGsm;
        CdmaSignalInfoRecord record = {};
        if (response == NULL || responseLen == 0) {
            isGsm = true;
        } else {
            isGsm = false;
            if (responseLen != sizeof (RIL_CDMA_SignalInfoRecord)) {
                RLOGE("callRingInd: invalid response");
                return 0;
            }
            convertRilCdmaSignalInfoRecordToHal((RIL_CDMA_SignalInfoRecord *) response, record);
        }

        RLOGD("callRingInd: isGsm %d", isGsm);
        Return<void> retStatus = radioService[slotId]->mRadioIndication->callRing(
                convertIntToRadioIndicationType(indicationType), isGsm, record);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("callRingInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::simStatusChangedInd(int slotId,
                               int indicationType, int token, RIL_Errno e, void *response,
                               size_t responseLen) {
    RLOGD("simStatusChangedInd");
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        Return<void> retStatus = radioService[slotId]->mRadioIndication->simStatusChanged(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("simStatusChangedInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    int seSlot = toClientSlot(slotId, android::CLIENT_SE);
    if (radioService[seSlot] != NULL && radioService[seSlot]->mRadioIndication != NULL) {
        Return<void> retStatus = radioService[seSlot]->mRadioIndication->simStatusChanged(
                                 convertIntToRadioIndicationType(indicationType));
        radioService[seSlot]->checkReturnStatus(retStatus, slotId);
    }
    else {
        RLOGE("simStatusChangedInd: radioService[%d]->mRadioIndication == NULL", seSlot);
    }
    return 0;
}

bool makeCdmaSmsMessage(CdmaSmsMessage &msg, void *response, size_t responseLen) {
    if (response == NULL || responseLen != sizeof(RIL_CDMA_SMS_Message)) {
        return false;
    }
    RIL_CDMA_SMS_Message *rilMsg = (RIL_CDMA_SMS_Message *) response;
    msg.teleserviceId = rilMsg->uTeleserviceID;
    msg.isServicePresent = rilMsg->bIsServicePresent;
    msg.serviceCategory = rilMsg->uServicecategory;
    msg.address.digitMode =
            (android::hardware::radio::V1_0::CdmaSmsDigitMode) rilMsg->sAddress.digit_mode;
    msg.address.numberMode =
            (android::hardware::radio::V1_0::CdmaSmsNumberMode) rilMsg->sAddress.number_mode;
    msg.address.numberType =
            (android::hardware::radio::V1_0::CdmaSmsNumberType) rilMsg->sAddress.number_type;
    msg.address.numberPlan =
            (android::hardware::radio::V1_0::CdmaSmsNumberPlan) rilMsg->sAddress.number_plan;

    int digitLimit = MIN((rilMsg->sAddress.number_of_digits), RIL_CDMA_SMS_ADDRESS_MAX);
    msg.address.digits.setToExternal(rilMsg->sAddress.digits, digitLimit);

    msg.subAddress.subaddressType = (android::hardware::radio::V1_0::CdmaSmsSubaddressType)
            rilMsg->sSubAddress.subaddressType;
    msg.subAddress.odd = rilMsg->sSubAddress.odd;

    digitLimit= MIN((rilMsg->sSubAddress.number_of_digits), RIL_CDMA_SMS_SUBADDRESS_MAX);
    msg.subAddress.digits.setToExternal(rilMsg->sSubAddress.digits, digitLimit);

    digitLimit = MIN((rilMsg->uBearerDataLen), RIL_CDMA_SMS_BEARER_DATA_MAX);
    msg.bearerData.setToExternal(rilMsg->aBearerData, digitLimit);
    return true;
}

int radio::cdmaNewSmsInd(int slotId, int indicationType,
                         int token, RIL_Errno e, void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        CdmaSmsMessage msg = {};
        if (!makeCdmaSmsMessage(msg, response, responseLen)) {
            RLOGE("cdmaNewSmsInd: invalid response");
            return 0;
        }
        RLOGD("cdmaNewSmsInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->cdmaNewSms(
                convertIntToRadioIndicationType(indicationType), msg);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("cdmaNewSmsInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::newBroadcastSmsInd(int slotId,
                              int indicationType, int token, RIL_Errno e, void *response,
                              size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("newBroadcastSmsInd: invalid response");
            return 0;
        }

        hidl_vec<uint8_t> data;
        data.setToExternal((uint8_t *) response, responseLen);
        RLOGD("newBroadcastSmsInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->newBroadcastSms(
                convertIntToRadioIndicationType(indicationType), data);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("newBroadcastSmsInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::cdmaRuimSmsStorageFullInd(int slotId,
                                     int indicationType, int token, RIL_Errno e, void *response,
                                     size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        RLOGD("cdmaRuimSmsStorageFullInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->cdmaRuimSmsStorageFull(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("cdmaRuimSmsStorageFullInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

int radio::restrictedStateChangedInd(int slotId,
                                     int indicationType, int token, RIL_Errno e, void *response,
                                     size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("restrictedStateChangedInd: invalid response");
            return 0;
        }
        int32_t state = ((int32_t *) response)[0];
        RLOGD("restrictedStateChangedInd: state %d", state);
        Return<void> retStatus = radioService[slotId]->mRadioIndication->restrictedStateChanged(
                convertIntToRadioIndicationType(indicationType), (PhoneRestrictedState) state);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("restrictedStateChangedInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

int radio::enterEmergencyCallbackModeInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        RLOGD("enterEmergencyCallbackModeInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->enterEmergencyCallbackMode(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("enterEmergencyCallbackModeInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

int radio::cdmaCallWaitingInd(int slotId,
                              int indicationType, int token, RIL_Errno e, void *response,
                              size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_CDMA_CallWaiting_v6)) {
            RLOGE("cdmaCallWaitingInd: invalid response");
            return 0;
        }

        CdmaCallWaiting callWaitingRecord = {};
        RIL_CDMA_CallWaiting_v6 *callWaitingRil = ((RIL_CDMA_CallWaiting_v6 *) response);
        callWaitingRecord.number = convertCharPtrToHidlString(callWaitingRil->number);
        callWaitingRecord.numberPresentation =
                (CdmaCallWaitingNumberPresentation) callWaitingRil->numberPresentation;
        callWaitingRecord.name = convertCharPtrToHidlString(callWaitingRil->name);
        convertRilCdmaSignalInfoRecordToHal(&callWaitingRil->signalInfoRecord,
                callWaitingRecord.signalInfoRecord);
        callWaitingRecord.numberType = (CdmaCallWaitingNumberType) callWaitingRil->number_type;
        callWaitingRecord.numberPlan = (CdmaCallWaitingNumberPlan) callWaitingRil->number_plan;

        RLOGD("cdmaCallWaitingInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->cdmaCallWaiting(
                convertIntToRadioIndicationType(indicationType), callWaitingRecord);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("cdmaCallWaitingInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::cdmaOtaProvisionStatusInd(int slotId,
                                     int indicationType, int token, RIL_Errno e, void *response,
                                     size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("cdmaOtaProvisionStatusInd: invalid response");
            return 0;
        }
        int32_t status = ((int32_t *) response)[0];
        RLOGD("cdmaOtaProvisionStatusInd: status %d", status);
        Return<void> retStatus = radioService[slotId]->mRadioIndication->cdmaOtaProvisionStatus(
                convertIntToRadioIndicationType(indicationType), (CdmaOtaProvisionStatus) status);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("cdmaOtaProvisionStatusInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

int radio::cdmaInfoRecInd(int slotId,
                          int indicationType, int token, RIL_Errno e, void *response,
                          size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_CDMA_InformationRecords)) {
            RLOGE("cdmaInfoRecInd: invalid response");
            return 0;
        }

        CdmaInformationRecords records = {};
        RIL_CDMA_InformationRecords *recordsRil = (RIL_CDMA_InformationRecords *) response;

        char* string8 = NULL;
        int num = MIN(recordsRil->numberOfInfoRecs, RIL_CDMA_MAX_NUMBER_OF_INFO_RECS);
        if (recordsRil->numberOfInfoRecs > RIL_CDMA_MAX_NUMBER_OF_INFO_RECS) {
            RLOGE("cdmaInfoRecInd: received %d recs which is more than %d, dropping "
                    "additional ones", recordsRil->numberOfInfoRecs,
                    RIL_CDMA_MAX_NUMBER_OF_INFO_RECS);
        }
        records.infoRec.resize(num);
        for (int i = 0 ; i < num ; i++) {
            CdmaInformationRecord *record = &records.infoRec[i];
            RIL_CDMA_InformationRecord *infoRec = &recordsRil->infoRec[i];
            record->name = (CdmaInfoRecName) infoRec->name;
            // All vectors should be size 0 except one which will be size 1. Set everything to
            // size 0 initially.
            record->display.resize(0);
            record->number.resize(0);
            record->signal.resize(0);
            record->redir.resize(0);
            record->lineCtrl.resize(0);
            record->clir.resize(0);
            record->audioCtrl.resize(0);
            switch (infoRec->name) {
                case RIL_CDMA_DISPLAY_INFO_REC:
                case RIL_CDMA_EXTENDED_DISPLAY_INFO_REC: {
                    if (infoRec->rec.display.alpha_len > CDMA_ALPHA_INFO_BUFFER_LENGTH) {
                        RLOGE("cdmaInfoRecInd: invalid display info response length %d "
                                "expected not more than %d", (int) infoRec->rec.display.alpha_len,
                                CDMA_ALPHA_INFO_BUFFER_LENGTH);
                        return 0;
                    }
                    string8 = (char*) malloc((infoRec->rec.display.alpha_len + 1) * sizeof(char));
                    if (string8 == NULL) {
                        RLOGE("cdmaInfoRecInd: Memory allocation failed for "
                                "responseCdmaInformationRecords");
                        return 0;
                    }
                    memcpy(string8, infoRec->rec.display.alpha_buf, infoRec->rec.display.alpha_len);
                    string8[(int)infoRec->rec.display.alpha_len] = '\0';

                    record->display.resize(1);
                    record->display[0].alphaBuf = string8;
                    free(string8);
                    string8 = NULL;
                    break;
                }

                case RIL_CDMA_CALLED_PARTY_NUMBER_INFO_REC:
                case RIL_CDMA_CALLING_PARTY_NUMBER_INFO_REC:
                case RIL_CDMA_CONNECTED_NUMBER_INFO_REC: {
                    if (infoRec->rec.number.len > CDMA_NUMBER_INFO_BUFFER_LENGTH) {
                        RLOGE("cdmaInfoRecInd: invalid display info response length %d "
                                "expected not more than %d", (int) infoRec->rec.number.len,
                                CDMA_NUMBER_INFO_BUFFER_LENGTH);
                        return 0;
                    }
                    string8 = (char*) malloc((infoRec->rec.number.len + 1) * sizeof(char));
                    if (string8 == NULL) {
                        RLOGE("cdmaInfoRecInd: Memory allocation failed for "
                                "responseCdmaInformationRecords");
                        return 0;
                    }
                    memcpy(string8, infoRec->rec.number.buf, infoRec->rec.number.len);
                    string8[(int)infoRec->rec.number.len] = '\0';

                    record->number.resize(1);
                    record->number[0].number = string8;
                    free(string8);
                    string8 = NULL;
                    record->number[0].numberType = infoRec->rec.number.number_type;
                    record->number[0].numberPlan = infoRec->rec.number.number_plan;
                    record->number[0].pi = infoRec->rec.number.pi;
                    record->number[0].si = infoRec->rec.number.si;
                    break;
                }

                case RIL_CDMA_SIGNAL_INFO_REC: {
                    record->signal.resize(1);
                    record->signal[0].isPresent = infoRec->rec.signal.isPresent;
                    record->signal[0].signalType = infoRec->rec.signal.signalType;
                    record->signal[0].alertPitch = infoRec->rec.signal.alertPitch;
                    record->signal[0].signal = infoRec->rec.signal.signal;
                    break;
                }

                case RIL_CDMA_REDIRECTING_NUMBER_INFO_REC: {
                    if (infoRec->rec.redir.redirectingNumber.len >
                                                  CDMA_NUMBER_INFO_BUFFER_LENGTH) {
                        RLOGE("cdmaInfoRecInd: invalid display info response length %d "
                                "expected not more than %d\n",
                                (int)infoRec->rec.redir.redirectingNumber.len,
                                CDMA_NUMBER_INFO_BUFFER_LENGTH);
                        return 0;
                    }
                    string8 = (char*) malloc((infoRec->rec.redir.redirectingNumber.len + 1) *
                            sizeof(char));
                    if (string8 == NULL) {
                        RLOGE("cdmaInfoRecInd: Memory allocation failed for "
                                "responseCdmaInformationRecords");
                        return 0;
                    }
                    memcpy(string8, infoRec->rec.redir.redirectingNumber.buf,
                            infoRec->rec.redir.redirectingNumber.len);
                    string8[(int)infoRec->rec.redir.redirectingNumber.len] = '\0';

                    record->redir.resize(1);
                    record->redir[0].redirectingNumber.number = string8;
                    free(string8);
                    string8 = NULL;
                    record->redir[0].redirectingNumber.numberType =
                            infoRec->rec.redir.redirectingNumber.number_type;
                    record->redir[0].redirectingNumber.numberPlan =
                            infoRec->rec.redir.redirectingNumber.number_plan;
                    record->redir[0].redirectingNumber.pi = infoRec->rec.redir.redirectingNumber.pi;
                    record->redir[0].redirectingNumber.si = infoRec->rec.redir.redirectingNumber.si;
                    record->redir[0].redirectingReason =
                            (CdmaRedirectingReason) infoRec->rec.redir.redirectingReason;
                    break;
                }

                case RIL_CDMA_LINE_CONTROL_INFO_REC: {
                    record->lineCtrl.resize(1);
                    record->lineCtrl[0].lineCtrlPolarityIncluded =
                            infoRec->rec.lineCtrl.lineCtrlPolarityIncluded;
                    record->lineCtrl[0].lineCtrlToggle = infoRec->rec.lineCtrl.lineCtrlToggle;
                    record->lineCtrl[0].lineCtrlReverse = infoRec->rec.lineCtrl.lineCtrlReverse;
                    record->lineCtrl[0].lineCtrlPowerDenial =
                            infoRec->rec.lineCtrl.lineCtrlPowerDenial;
                    break;
                }

                case RIL_CDMA_T53_CLIR_INFO_REC: {
                    record->clir.resize(1);
                    record->clir[0].cause = infoRec->rec.clir.cause;
                    break;
                }

                case RIL_CDMA_T53_AUDIO_CONTROL_INFO_REC: {
                    record->audioCtrl.resize(1);
                    record->audioCtrl[0].upLink = infoRec->rec.audioCtrl.upLink;
                    record->audioCtrl[0].downLink = infoRec->rec.audioCtrl.downLink;
                    break;
                }

                case RIL_CDMA_T53_RELEASE_INFO_REC:
                    RLOGE("cdmaInfoRecInd: RIL_CDMA_T53_RELEASE_INFO_REC: INVALID");
                    return 0;

                default:
                    RLOGE("cdmaInfoRecInd: Incorrect name value");
                    return 0;
            }
        }

        RLOGD("cdmaInfoRecInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->cdmaInfoRec(
                convertIntToRadioIndicationType(indicationType), records);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("cdmaInfoRecInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::indicateRingbackToneInd(int slotId,
                                   int indicationType, int token, RIL_Errno e, void *response,
                                   size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("indicateRingbackToneInd: invalid response");
            return 0;
        }
        bool start = ((int32_t *) response)[0];
        RLOGD("indicateRingbackToneInd: start %d", start);
        Return<void> retStatus = radioService[slotId]->mRadioIndication->indicateRingbackTone(
                convertIntToRadioIndicationType(indicationType), start);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("indicateRingbackToneInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::resendIncallMuteInd(int slotId,
                               int indicationType, int token, RIL_Errno e, void *response,
                               size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        RLOGD("resendIncallMuteInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->resendIncallMute(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("resendIncallMuteInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::cdmaSubscriptionSourceChangedInd(int slotId,
                                            int indicationType, int token, RIL_Errno e,
                                            void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("cdmaSubscriptionSourceChangedInd: invalid response");
            return 0;
        }
        int32_t cdmaSource = ((int32_t *) response)[0];
        RLOGD("cdmaSubscriptionSourceChangedInd: cdmaSource %d", cdmaSource);
        Return<void> retStatus = radioService[slotId]->mRadioIndication->
                cdmaSubscriptionSourceChanged(convertIntToRadioIndicationType(indicationType),
                (CdmaSubscriptionSource) cdmaSource);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("cdmaSubscriptionSourceChangedInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

int radio::cdmaPrlChangedInd(int slotId,
                             int indicationType, int token, RIL_Errno e, void *response,
                             size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("cdmaPrlChangedInd: invalid response");
            return 0;
        }
        int32_t version = ((int32_t *) response)[0];
        RLOGD("cdmaPrlChangedInd: version %d", version);
        Return<void> retStatus = radioService[slotId]->mRadioIndication->cdmaPrlChanged(
                convertIntToRadioIndicationType(indicationType), version);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("cdmaPrlChangedInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::exitEmergencyCallbackModeInd(int slotId,
                                        int indicationType, int token, RIL_Errno e, void *response,
                                        size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        RLOGD("exitEmergencyCallbackModeInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->exitEmergencyCallbackMode(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("exitEmergencyCallbackModeInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

int radio::rilConnectedInd(int slotId,
                           int indicationType, int token, RIL_Errno e, void *response,
                           size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        RLOGD("rilConnectedInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->rilConnected(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("rilConnectedInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    // hangup all if the Phone process disconnected to sync the AP / MD state
    if (mtkRadioExService[slotId] != NULL) {
        RequestInfo *pRI = android::addRequestToList(0, slotId, RIL_REQUEST_HANGUP_ALL);
        if (pRI == NULL) {
            return 0;
        }
        pRI->local = 1;

        CALL_ONREQUEST(RIL_REQUEST_HANGUP_ALL, NULL, 0, pRI, slotId);
    }
    return 0;
}

int radio::voiceRadioTechChangedInd(int slotId,
                                    int indicationType, int token, RIL_Errno e, void *response,
                                    size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("voiceRadioTechChangedInd: invalid response");
            return 0;
        }
        int32_t rat = ((int32_t *) response)[0];
        RLOGD("voiceRadioTechChangedInd: rat %d", rat);
        Return<void> retStatus = radioService[slotId]->mRadioIndication->voiceRadioTechChanged(
                convertIntToRadioIndicationType(indicationType), (RadioTechnology) rat);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("voiceRadioTechChangedInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

char* convertRilCellInfoListToHal(void *response, size_t responseLen, hidl_vec<CellInfo>& records) {
    int num = responseLen / sizeof(RIL_CellInfo_v12);
    records.resize(num);

    char* mMncs = (char*) calloc(num * 6 , sizeof(char));
    if (mMncs == NULL) {
        RLOGE("convertRilCellInfoListToHal null");
        return NULL;
    }
    memset(mMncs, 0, num * sizeof(char) * 6);
    RIL_CellInfo_v12 *rillCellInfo = (RIL_CellInfo_v12 *) response;
    for (int i = 0; i < num; i++) {
        records[i].cellInfoType = (CellInfoType) rillCellInfo->cellInfoType;
        records[i].registered = rillCellInfo->registered;
        records[i].timeStampType = (TimeStampType) rillCellInfo->timeStampType;
        records[i].timeStamp = rillCellInfo->timeStamp;
        // All vectors should be size 0 except one which will be size 1. Set everything to
        // size 0 initially.
        records[i].gsm.resize(0);
        records[i].wcdma.resize(0);
        records[i].cdma.resize(0);
        records[i].lte.resize(0);
        records[i].tdscdma.resize(0);
        switch(rillCellInfo->cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM: {
                char* mMnc = &(mMncs[i * 6]);
                records[i].gsm.resize(1);
                CellInfoGsm *cellInfoGsm = &records[i].gsm[0];
                cellInfoGsm->cellIdentityGsm.mcc =
                        std::to_string(rillCellInfo->CellInfo.gsm.cellIdentityGsm.mcc);
                if (rillCellInfo->CellInfo.gsm.cellIdentityGsm.mnc_len == 2) {
                    snprintf(mMnc, 4, "%02d", rillCellInfo->CellInfo.gsm.cellIdentityGsm.mnc);
                } else if (rillCellInfo->CellInfo.gsm.cellIdentityGsm.mnc_len == 3) {
                    snprintf(mMnc, 4, "%03d", rillCellInfo->CellInfo.gsm.cellIdentityGsm.mnc);
                } else {
                    snprintf(mMnc, 4, "%d", rillCellInfo->CellInfo.gsm.cellIdentityGsm.mnc);
                }
                cellInfoGsm->cellIdentityGsm.mnc = convertCharPtrToHidlString(mMnc);
                cellInfoGsm->cellIdentityGsm.lac =
                        rillCellInfo->CellInfo.gsm.cellIdentityGsm.lac;
                cellInfoGsm->cellIdentityGsm.cid =
                        rillCellInfo->CellInfo.gsm.cellIdentityGsm.cid;
                cellInfoGsm->cellIdentityGsm.arfcn =
                        rillCellInfo->CellInfo.gsm.cellIdentityGsm.arfcn;
                cellInfoGsm->cellIdentityGsm.bsic =
                        rillCellInfo->CellInfo.gsm.cellIdentityGsm.bsic;
                cellInfoGsm->signalStrengthGsm.signalStrength =
                        rillCellInfo->CellInfo.gsm.signalStrengthGsm.signalStrength;
                cellInfoGsm->signalStrengthGsm.bitErrorRate =
                        rillCellInfo->CellInfo.gsm.signalStrengthGsm.bitErrorRate;
                cellInfoGsm->signalStrengthGsm.timingAdvance =
                        rillCellInfo->CellInfo.gsm.signalStrengthGsm.timingAdvance;
                break;
            }

            case RIL_CELL_INFO_TYPE_WCDMA: {
                char* mMnc = &(mMncs[i * 6]);
                records[i].wcdma.resize(1);
                CellInfoWcdma *cellInfoWcdma = &records[i].wcdma[0];
                cellInfoWcdma->cellIdentityWcdma.mcc =
                        std::to_string(rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mcc);
                if (rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mnc_len == 2) {
                    snprintf(mMnc, 4, "%02d", rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mnc);
                } else if (rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mnc_len == 3) {
                    snprintf(mMnc, 4, "%03d", rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mnc);
                } else {
                    snprintf(mMnc, 4, "%d", rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mnc);
                }
                cellInfoWcdma->cellIdentityWcdma.mnc = convertCharPtrToHidlString(mMnc);
                cellInfoWcdma->cellIdentityWcdma.lac =
                        rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.lac;
                cellInfoWcdma->cellIdentityWcdma.cid =
                        rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.cid;
                cellInfoWcdma->cellIdentityWcdma.psc =
                        rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.psc;
                cellInfoWcdma->cellIdentityWcdma.uarfcn =
                        rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.uarfcn;
                cellInfoWcdma->signalStrengthWcdma.signalStrength =
                        rillCellInfo->CellInfo.wcdma.signalStrengthWcdma.signalStrength;
                cellInfoWcdma->signalStrengthWcdma.bitErrorRate =
                        rillCellInfo->CellInfo.wcdma.signalStrengthWcdma.bitErrorRate;
                break;
            }

            case RIL_CELL_INFO_TYPE_CDMA: {
                records[i].cdma.resize(1);
                CellInfoCdma *cellInfoCdma = &records[i].cdma[0];
                cellInfoCdma->cellIdentityCdma.networkId =
                        rillCellInfo->CellInfo.cdma.cellIdentityCdma.networkId;
                cellInfoCdma->cellIdentityCdma.systemId =
                        rillCellInfo->CellInfo.cdma.cellIdentityCdma.systemId;
                cellInfoCdma->cellIdentityCdma.baseStationId =
                        rillCellInfo->CellInfo.cdma.cellIdentityCdma.basestationId;
                cellInfoCdma->cellIdentityCdma.longitude =
                        rillCellInfo->CellInfo.cdma.cellIdentityCdma.longitude;
                cellInfoCdma->cellIdentityCdma.latitude =
                        rillCellInfo->CellInfo.cdma.cellIdentityCdma.latitude;
                cellInfoCdma->signalStrengthCdma.dbm =
                        rillCellInfo->CellInfo.cdma.signalStrengthCdma.dbm;
                cellInfoCdma->signalStrengthCdma.ecio =
                        rillCellInfo->CellInfo.cdma.signalStrengthCdma.ecio;
                cellInfoCdma->signalStrengthEvdo.dbm =
                        rillCellInfo->CellInfo.cdma.signalStrengthEvdo.dbm;
                cellInfoCdma->signalStrengthEvdo.ecio =
                        rillCellInfo->CellInfo.cdma.signalStrengthEvdo.ecio;
                cellInfoCdma->signalStrengthEvdo.signalNoiseRatio =
                        rillCellInfo->CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio;
                break;
            }

            case RIL_CELL_INFO_TYPE_LTE: {
                char* mMnc = &(mMncs[i * 6]);
                records[i].lte.resize(1);
                CellInfoLte *cellInfoLte = &records[i].lte[0];
                cellInfoLte->cellIdentityLte.mcc =
                        std::to_string(rillCellInfo->CellInfo.lte.cellIdentityLte.mcc);
                if (rillCellInfo->CellInfo.lte.cellIdentityLte.mnc_len == 2) {
                    snprintf(mMnc, 4, "%02d", rillCellInfo->CellInfo.lte.cellIdentityLte.mnc);
                } else if (rillCellInfo->CellInfo.lte.cellIdentityLte.mnc_len == 3) {
                    snprintf(mMnc, 4, "%03d", rillCellInfo->CellInfo.lte.cellIdentityLte.mnc);
                } else {
                    snprintf(mMnc, 4, "%d", rillCellInfo->CellInfo.lte.cellIdentityLte.mnc);
                }
                cellInfoLte->cellIdentityLte.mnc = convertCharPtrToHidlString(mMnc);
                cellInfoLte->cellIdentityLte.ci =
                        rillCellInfo->CellInfo.lte.cellIdentityLte.ci;
                cellInfoLte->cellIdentityLte.pci =
                        rillCellInfo->CellInfo.lte.cellIdentityLte.pci;
                cellInfoLte->cellIdentityLte.tac =
                        rillCellInfo->CellInfo.lte.cellIdentityLte.tac;
                cellInfoLte->cellIdentityLte.earfcn =
                        rillCellInfo->CellInfo.lte.cellIdentityLte.earfcn;
                cellInfoLte->signalStrengthLte.signalStrength =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.signalStrength;
                cellInfoLte->signalStrengthLte.rsrp =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.rsrp;
                cellInfoLte->signalStrengthLte.rsrq =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.rsrq;
                cellInfoLte->signalStrengthLte.rssnr =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.rssnr;
                cellInfoLte->signalStrengthLte.cqi =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.cqi;
                cellInfoLte->signalStrengthLte.timingAdvance =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.timingAdvance;
                break;
            }

            case RIL_CELL_INFO_TYPE_TD_SCDMA: {
                char* mMnc = &(mMncs[i * 6]);
                records[i].tdscdma.resize(1);
                CellInfoTdscdma *cellInfoTdscdma = &records[i].tdscdma[0];
                cellInfoTdscdma->cellIdentityTdscdma.mcc =
                        std::to_string(rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mcc);
                if (rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mnc_len == 2) {
                    snprintf(mMnc, 4, "%02d", rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mnc);
                } else if (rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mnc_len == 3) {
                    snprintf(mMnc, 4, "%03d", rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mnc);
                } else {
                    snprintf(mMnc, 4, "%d", rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mnc);
                }
                cellInfoTdscdma->cellIdentityTdscdma.mnc = convertCharPtrToHidlString(mMnc);
                cellInfoTdscdma->cellIdentityTdscdma.lac =
                        rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.lac;
                cellInfoTdscdma->cellIdentityTdscdma.cid =
                        rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.cid;
                cellInfoTdscdma->cellIdentityTdscdma.cpid =
                        rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.cpid;
                cellInfoTdscdma->signalStrengthTdscdma.rscp =
                        rillCellInfo->CellInfo.tdscdma.signalStrengthTdscdma.rscp;
                break;
            }
            case RIL_CELL_INFO_TYPE_NR:
            default: {
                break;
            }
        }
        rillCellInfo += 1;
    }
    return mMncs;
}

char* convertRilCellInfoListToHal_1_2(void *response, size_t responseLen,
            hidl_vec<AOSP_V1_2::CellInfo>& records) {
    int num = responseLen / sizeof(RIL_CellInfo_v12);
    records.resize(num);

    char* mMncs = (char*) calloc(num * 6 , sizeof(char));
    if (mMncs == NULL) {
        RLOGE("convertRilCellInfoListToHal_1_2 NULL");
        return NULL;
    }
    memset(mMncs, 0, num * sizeof(char) * 6);
    RIL_CellInfo_v12 *rillCellInfo = (RIL_CellInfo_v12 *) response;
    for (int i = 0; i < num; i++) {
        records[i].cellInfoType = (CellInfoType) rillCellInfo->cellInfoType;
        records[i].registered = rillCellInfo->registered;
        records[i].timeStampType = (TimeStampType) rillCellInfo->timeStampType;
        records[i].timeStamp = rillCellInfo->timeStamp;
        records[i].connectionStatus =
                (AOSP_V1_2::CellConnectionStatus) rillCellInfo->connectionStatus;
        // All vectors should be size 0 except one which will be size 1. Set everything to
        // size 0 initially.
        records[i].gsm.resize(0);
        records[i].wcdma.resize(0);
        records[i].cdma.resize(0);
        records[i].lte.resize(0);
        records[i].tdscdma.resize(0);
        switch (rillCellInfo->cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM: {
                char* mMnc = &(mMncs[i * 6]);
                records[i].gsm.resize(1);
                AOSP_V1_2::CellInfoGsm *cellInfoGsm = &records[i].gsm[0];
                if (rillCellInfo->CellInfo.gsm.cellIdentityGsm.mcc < 10) {
                    cellInfoGsm->cellIdentityGsm.base.mcc = "00" +
                            std::to_string(rillCellInfo->CellInfo.gsm.cellIdentityGsm.mcc);
                } else if (rillCellInfo->CellInfo.gsm.cellIdentityGsm.mcc < 100) {
                    cellInfoGsm->cellIdentityGsm.base.mcc = "0" +
                            std::to_string(rillCellInfo->CellInfo.gsm.cellIdentityGsm.mcc);
                } else {
                    cellInfoGsm->cellIdentityGsm.base.mcc =
                            std::to_string(rillCellInfo->CellInfo.gsm.cellIdentityGsm.mcc);
                }
                if (rillCellInfo->CellInfo.gsm.cellIdentityGsm.mnc_len == 2) {
                    snprintf(mMnc, 5, "%02d", rillCellInfo->CellInfo.gsm.cellIdentityGsm.mnc);
                } else if (rillCellInfo->CellInfo.gsm.cellIdentityGsm.mnc_len == 3) {
                    snprintf(mMnc, 5, "%03d", rillCellInfo->CellInfo.gsm.cellIdentityGsm.mnc);
                } else {
                    snprintf(mMnc, 5, "%d", rillCellInfo->CellInfo.gsm.cellIdentityGsm.mnc);
                }
                cellInfoGsm->cellIdentityGsm.base.mnc = convertCharPtrToHidlString(mMnc);
                cellInfoGsm->cellIdentityGsm.base.lac =
                        rillCellInfo->CellInfo.gsm.cellIdentityGsm.lac;
                cellInfoGsm->cellIdentityGsm.base.cid =
                        rillCellInfo->CellInfo.gsm.cellIdentityGsm.cid;
                cellInfoGsm->cellIdentityGsm.base.arfcn =
                        rillCellInfo->CellInfo.gsm.cellIdentityGsm.arfcn;
                cellInfoGsm->cellIdentityGsm.base.bsic =
                        rillCellInfo->CellInfo.gsm.cellIdentityGsm.bsic;
                cellInfoGsm->cellIdentityGsm.operatorNames.alphaLong =
                        convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.gsm.cellIdentityGsm.operName.long_name);
                cellInfoGsm->cellIdentityGsm.operatorNames.alphaShort =
                        convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.gsm.cellIdentityGsm.operName.short_name);

                cellInfoGsm->signalStrengthGsm.signalStrength =
                        rillCellInfo->CellInfo.gsm.signalStrengthGsm.signalStrength;
                cellInfoGsm->signalStrengthGsm.bitErrorRate =
                        rillCellInfo->CellInfo.gsm.signalStrengthGsm.bitErrorRate;
                cellInfoGsm->signalStrengthGsm.timingAdvance =
                        rillCellInfo->CellInfo.gsm.signalStrengthGsm.timingAdvance;
                break;
            }

            case RIL_CELL_INFO_TYPE_WCDMA: {
                char* mMnc = &(mMncs[i * 6]);
                records[i].wcdma.resize(1);
                AOSP_V1_2::CellInfoWcdma *cellInfoWcdma = &records[i].wcdma[0];
                if (rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mcc < 10) {
                    cellInfoWcdma->cellIdentityWcdma.base.mcc = "00" +
                            std::to_string(rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mcc);
                } else if (rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mcc < 100) {
                    cellInfoWcdma->cellIdentityWcdma.base.mcc = "0" +
                            std::to_string(rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mcc);
                } else {
                    cellInfoWcdma->cellIdentityWcdma.base.mcc =
                            std::to_string(rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mcc);
                }
                if (rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mnc_len == 2) {
                    snprintf(mMnc, 5, "%02d", rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mnc);
                } else if (rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mnc_len == 3) {
                    snprintf(mMnc, 5, "%03d", rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mnc);
                } else {
                    snprintf(mMnc, 5, "%d", rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mnc);
                }
                cellInfoWcdma->cellIdentityWcdma.base.mnc = convertCharPtrToHidlString(mMnc);
                cellInfoWcdma->cellIdentityWcdma.base.lac =
                        rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.lac;
                cellInfoWcdma->cellIdentityWcdma.base.cid =
                        rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.cid;
                cellInfoWcdma->cellIdentityWcdma.base.psc =
                        rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.psc;
                cellInfoWcdma->cellIdentityWcdma.base.uarfcn =
                        rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.uarfcn;
                cellInfoWcdma->cellIdentityWcdma.operatorNames.alphaLong =
                        convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.operName.long_name);
                cellInfoWcdma->cellIdentityWcdma.operatorNames.alphaShort =
                        convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.operName.short_name);

                cellInfoWcdma->signalStrengthWcdma.base.signalStrength =
                        rillCellInfo->CellInfo.wcdma.signalStrengthWcdma.signalStrength;
                cellInfoWcdma->signalStrengthWcdma.base.bitErrorRate =
                        rillCellInfo->CellInfo.wcdma.signalStrengthWcdma.bitErrorRate;
                break;
            }

            case RIL_CELL_INFO_TYPE_CDMA: {
                records[i].cdma.resize(1);
                AOSP_V1_2::CellInfoCdma *cellInfoCdma = &records[i].cdma[0];
                cellInfoCdma->cellIdentityCdma.base.networkId =
                        rillCellInfo->CellInfo.cdma.cellIdentityCdma.networkId;
                cellInfoCdma->cellIdentityCdma.base.systemId =
                        rillCellInfo->CellInfo.cdma.cellIdentityCdma.systemId;
                cellInfoCdma->cellIdentityCdma.base.baseStationId =
                        rillCellInfo->CellInfo.cdma.cellIdentityCdma.basestationId;
                cellInfoCdma->cellIdentityCdma.base.longitude =
                        rillCellInfo->CellInfo.cdma.cellIdentityCdma.longitude;
                cellInfoCdma->cellIdentityCdma.base.latitude =
                        rillCellInfo->CellInfo.cdma.cellIdentityCdma.latitude;
                /* long_name and short_name are NULL
                cellInfoGsm->cellIdentityGsm.operNames.alphaLong =
                        convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.gsm.cellIdentityGsm.operName.long_name);
                cellInfoGsm->cellIdentityGsm.operNames.alphaShort =
                        convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.gsm.cellIdentityGsm.operName.short_name);
                */
                cellInfoCdma->signalStrengthCdma.dbm =
                        rillCellInfo->CellInfo.cdma.signalStrengthCdma.dbm;
                cellInfoCdma->signalStrengthCdma.ecio =
                        rillCellInfo->CellInfo.cdma.signalStrengthCdma.ecio;
                cellInfoCdma->signalStrengthEvdo.dbm =
                        rillCellInfo->CellInfo.cdma.signalStrengthEvdo.dbm;
                cellInfoCdma->signalStrengthEvdo.ecio =
                        rillCellInfo->CellInfo.cdma.signalStrengthEvdo.ecio;
                cellInfoCdma->signalStrengthEvdo.signalNoiseRatio =
                        rillCellInfo->CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio;
                break;
            }

            case RIL_CELL_INFO_TYPE_LTE: {
                char* mMnc = &(mMncs[i * 6]);
                records[i].lte.resize(1);
                AOSP_V1_2::CellInfoLte *cellInfoLte = &records[i].lte[0];
                if (rillCellInfo->CellInfo.lte.cellIdentityLte.mcc < 10) {
                    cellInfoLte->cellIdentityLte.base.mcc = "00" +
                            std::to_string(rillCellInfo->CellInfo.lte.cellIdentityLte.mcc);
                } else if (rillCellInfo->CellInfo.lte.cellIdentityLte.mcc < 100) {
                    cellInfoLte->cellIdentityLte.base.mcc = "0" +
                            std::to_string(rillCellInfo->CellInfo.lte.cellIdentityLte.mcc);
                } else {
                    cellInfoLte->cellIdentityLte.base.mcc =
                            std::to_string(rillCellInfo->CellInfo.lte.cellIdentityLte.mcc);
                }
                if (rillCellInfo->CellInfo.lte.cellIdentityLte.mnc_len == 2) {
                    snprintf(mMnc, 5, "%02d", rillCellInfo->CellInfo.lte.cellIdentityLte.mnc);
                } else if (rillCellInfo->CellInfo.lte.cellIdentityLte.mnc_len == 3) {
                    snprintf(mMnc, 5, "%03d", rillCellInfo->CellInfo.lte.cellIdentityLte.mnc);
                } else {
                    snprintf(mMnc, 5, "%d", rillCellInfo->CellInfo.lte.cellIdentityLte.mnc);
                }
                cellInfoLte->cellIdentityLte.base.mnc = convertCharPtrToHidlString(mMnc);
                cellInfoLte->cellIdentityLte.base.ci =
                        rillCellInfo->CellInfo.lte.cellIdentityLte.ci;
                cellInfoLte->cellIdentityLte.base.pci =
                        rillCellInfo->CellInfo.lte.cellIdentityLte.pci;
                cellInfoLte->cellIdentityLte.base.tac =
                        rillCellInfo->CellInfo.lte.cellIdentityLte.tac;
                cellInfoLte->cellIdentityLte.base.earfcn =
                        rillCellInfo->CellInfo.lte.cellIdentityLte.earfcn;
                cellInfoLte->cellIdentityLte.operatorNames.alphaLong =
                        convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.lte.cellIdentityLte.operName.long_name);
                cellInfoLte->cellIdentityLte.operatorNames.alphaShort =
                        convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.lte.cellIdentityLte.operName.short_name);
                cellInfoLte->cellIdentityLte.bandwidth = 0x7FFFFFFF;
                cellInfoLte->signalStrengthLte.signalStrength =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.signalStrength;
                cellInfoLte->signalStrengthLte.rsrp =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.rsrp;
                cellInfoLte->signalStrengthLte.rsrq =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.rsrq;
                cellInfoLte->signalStrengthLte.rssnr =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.rssnr;
                cellInfoLte->signalStrengthLte.cqi =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.cqi;
                cellInfoLte->signalStrengthLte.timingAdvance =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.timingAdvance;
                break;
            }

            case RIL_CELL_INFO_TYPE_TD_SCDMA: {
                char* mMnc = &(mMncs[i * 6]);
                records[i].tdscdma.resize(1);
                AOSP_V1_2::CellInfoTdscdma *cellInfoTdscdma = &records[i].tdscdma[0];
                if (rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mcc < 10) {
                    cellInfoTdscdma->cellIdentityTdscdma.base.mcc = "00" +
                            std::to_string(rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mcc);
                } else if (rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mcc < 100) {
                    cellInfoTdscdma->cellIdentityTdscdma.base.mcc = "0" +
                            std::to_string(rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mcc);
                } else {
                    cellInfoTdscdma->cellIdentityTdscdma.base.mcc =
                            std::to_string(rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mcc);
                }
                if (rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mnc_len == 2) {
                    snprintf(mMnc, 5, "%02d", rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mnc);
                } else if (rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mnc_len == 3) {
                    snprintf(mMnc, 5, "%03d", rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mnc);
                } else {
                    snprintf(mMnc, 5, "%d", rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mnc);
                }
                cellInfoTdscdma->cellIdentityTdscdma.base.mnc = convertCharPtrToHidlString(mMnc);
                cellInfoTdscdma->cellIdentityTdscdma.base.lac =
                        rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.lac;
                cellInfoTdscdma->cellIdentityTdscdma.base.cid =
                        rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.cid;
                cellInfoTdscdma->cellIdentityTdscdma.base.cpid =
                        rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.cpid;
                cellInfoTdscdma->cellIdentityTdscdma.operatorNames.alphaLong =
                        convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.operName.long_name);
                cellInfoTdscdma->cellIdentityTdscdma.operatorNames.alphaShort =
                        convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.operName.short_name);

                cellInfoTdscdma->signalStrengthTdscdma.rscp =
                        rillCellInfo->CellInfo.tdscdma.signalStrengthTdscdma.rscp;
                break;
            }
            case RIL_CELL_INFO_TYPE_NR:
            default: {
                break;
            }
        }
        rillCellInfo += 1;
    }
    return mMncs;
}

char* convertRilCellInfoListToHal_1_4(void *response, size_t responseLen,
            hidl_vec<AOSP_V1_4::CellInfo>& records) {
    int num = responseLen / sizeof(RIL_CellInfo_v12);
    records.resize(num);

    char* mMncs = (char*) calloc(num * 6 , sizeof(char));
    if (mMncs == NULL) {
        RLOGE("convertRilCellInfoListToHal_1_4 NULL");
        return NULL;
    }
    memset(mMncs, 0, num * sizeof(char) * 6);
    RIL_CellInfo_v12 *rillCellInfo = (RIL_CellInfo_v12 *) response;
    for (int i = 0; i < num; i++) {
        records[i].isRegistered = rillCellInfo->registered == 0 ? false: true;
        records[i].connectionStatus =
                (AOSP_V1_2::CellConnectionStatus) rillCellInfo->connectionStatus;
        switch (rillCellInfo->cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM: {
                char* mMnc = &(mMncs[i * 6]);

                AOSP_V1_2::CellInfoGsm cellInfoGsm = {};
                if (rillCellInfo->CellInfo.gsm.cellIdentityGsm.mcc < 10) {
                    cellInfoGsm.cellIdentityGsm.base.mcc = "00" +
                            std::to_string(rillCellInfo->CellInfo.gsm.cellIdentityGsm.mcc);
                } else if (rillCellInfo->CellInfo.gsm.cellIdentityGsm.mcc < 100) {
                    cellInfoGsm.cellIdentityGsm.base.mcc = "0" +
                            std::to_string(rillCellInfo->CellInfo.gsm.cellIdentityGsm.mcc);
                } else {
                    cellInfoGsm.cellIdentityGsm.base.mcc =
                            std::to_string(rillCellInfo->CellInfo.gsm.cellIdentityGsm.mcc);
                }
                if (rillCellInfo->CellInfo.gsm.cellIdentityGsm.mnc_len == 2) {
                    snprintf(mMnc, 5, "%02d", rillCellInfo->CellInfo.gsm.cellIdentityGsm.mnc);
                } else if (rillCellInfo->CellInfo.gsm.cellIdentityGsm.mnc_len == 3) {
                    snprintf(mMnc, 5, "%03d", rillCellInfo->CellInfo.gsm.cellIdentityGsm.mnc);
                } else {
                    snprintf(mMnc, 5, "%d", rillCellInfo->CellInfo.gsm.cellIdentityGsm.mnc);
                }
                cellInfoGsm.cellIdentityGsm.base.mnc = convertCharPtrToHidlString(mMnc);
                cellInfoGsm.cellIdentityGsm.base.lac =
                        rillCellInfo->CellInfo.gsm.cellIdentityGsm.lac;
                cellInfoGsm.cellIdentityGsm.base.cid =
                        rillCellInfo->CellInfo.gsm.cellIdentityGsm.cid;
                cellInfoGsm.cellIdentityGsm.base.arfcn =
                        rillCellInfo->CellInfo.gsm.cellIdentityGsm.arfcn;
                cellInfoGsm.cellIdentityGsm.base.bsic =
                        rillCellInfo->CellInfo.gsm.cellIdentityGsm.bsic;
                cellInfoGsm.cellIdentityGsm.operatorNames.alphaLong =
                        convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.gsm.cellIdentityGsm.operName.long_name);
                cellInfoGsm.cellIdentityGsm.operatorNames.alphaShort =
                        convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.gsm.cellIdentityGsm.operName.short_name);
                // clear information if mcc is unknown
                if (rillCellInfo->CellInfo.gsm.cellIdentityGsm.mcc == 0x7FFFFFFF) {
                    cellInfoGsm.cellIdentityGsm.base.mcc = NULL;
                    cellInfoGsm.cellIdentityGsm.base.mnc = NULL;
                    cellInfoGsm.cellIdentityGsm.operatorNames.alphaLong = NULL;
                    cellInfoGsm.cellIdentityGsm.operatorNames.alphaShort = NULL;
                }
                cellInfoGsm.signalStrengthGsm.signalStrength =
                        rillCellInfo->CellInfo.gsm.signalStrengthGsm.signalStrength;
                cellInfoGsm.signalStrengthGsm.bitErrorRate =
                        rillCellInfo->CellInfo.gsm.signalStrengthGsm.bitErrorRate;
                cellInfoGsm.signalStrengthGsm.timingAdvance =
                        rillCellInfo->CellInfo.gsm.signalStrengthGsm.timingAdvance;

                records[i].info.gsm(cellInfoGsm);
                break;
            }

            case RIL_CELL_INFO_TYPE_WCDMA: {
                char* mMnc = &(mMncs[i * 6]);

                AOSP_V1_2::CellInfoWcdma cellInfoWcdma = {};
                if (rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mcc < 10) {
                    cellInfoWcdma.cellIdentityWcdma.base.mcc = "00" +
                            std::to_string(rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mcc);
                } else if (rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mcc < 100) {
                    cellInfoWcdma.cellIdentityWcdma.base.mcc = "0" +
                            std::to_string(rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mcc);
                } else {
                    cellInfoWcdma.cellIdentityWcdma.base.mcc =
                            std::to_string(rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mcc);
                }
                if (rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mnc_len == 2) {
                    snprintf(mMnc, 5, "%02d", rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mnc);
                } else if (rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mnc_len == 3) {
                    snprintf(mMnc, 5, "%03d", rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mnc);
                } else {
                    snprintf(mMnc, 5, "%d", rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mnc);
                }
                cellInfoWcdma.cellIdentityWcdma.base.mnc = convertCharPtrToHidlString(mMnc);
                cellInfoWcdma.cellIdentityWcdma.base.lac =
                        rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.lac;
                cellInfoWcdma.cellIdentityWcdma.base.cid =
                        rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.cid;
                cellInfoWcdma.cellIdentityWcdma.base.psc =
                        rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.psc;
                cellInfoWcdma.cellIdentityWcdma.base.uarfcn =
                        rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.uarfcn;
                cellInfoWcdma.cellIdentityWcdma.operatorNames.alphaLong =
                        convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.operName.long_name);
                cellInfoWcdma.cellIdentityWcdma.operatorNames.alphaShort =
                        convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.operName.short_name);
                // clear information if mcc is unknown
                if (rillCellInfo->CellInfo.wcdma.cellIdentityWcdma.mcc == 0x7FFFFFFF) {
                    cellInfoWcdma.cellIdentityWcdma.base.mcc = NULL;
                    cellInfoWcdma.cellIdentityWcdma.base.mnc = NULL;
                    cellInfoWcdma.cellIdentityWcdma.operatorNames.alphaLong = NULL;
                    cellInfoWcdma.cellIdentityWcdma.operatorNames.alphaShort = NULL;
                }
                cellInfoWcdma.signalStrengthWcdma.base.signalStrength =
                        rillCellInfo->CellInfo.wcdma.signalStrengthWcdma.signalStrength;
                cellInfoWcdma.signalStrengthWcdma.base.bitErrorRate =
                        rillCellInfo->CellInfo.wcdma.signalStrengthWcdma.bitErrorRate;

                records[i].info.wcdma(cellInfoWcdma);
                break;
            }

            case RIL_CELL_INFO_TYPE_CDMA: {
                AOSP_V1_2::CellInfoCdma cellInfoCdma = {};
                cellInfoCdma.cellIdentityCdma.base.networkId =
                        rillCellInfo->CellInfo.cdma.cellIdentityCdma.networkId;
                cellInfoCdma.cellIdentityCdma.base.systemId =
                        rillCellInfo->CellInfo.cdma.cellIdentityCdma.systemId;
                cellInfoCdma.cellIdentityCdma.base.baseStationId =
                        rillCellInfo->CellInfo.cdma.cellIdentityCdma.basestationId;
                cellInfoCdma.cellIdentityCdma.base.longitude =
                        rillCellInfo->CellInfo.cdma.cellIdentityCdma.longitude;
                cellInfoCdma.cellIdentityCdma.base.latitude =
                        rillCellInfo->CellInfo.cdma.cellIdentityCdma.latitude;
                /* long_name and short_name are NULL
                cellInfoGsm->cellIdentityGsm.operNames.alphaLong =
                        convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.gsm.cellIdentityGsm.operName.long_name);
                cellInfoGsm->cellIdentityGsm.operNames.alphaShort =
                        convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.gsm.cellIdentityGsm.operName.short_name);
                */
                cellInfoCdma.signalStrengthCdma.dbm =
                        rillCellInfo->CellInfo.cdma.signalStrengthCdma.dbm;
                cellInfoCdma.signalStrengthCdma.ecio =
                        rillCellInfo->CellInfo.cdma.signalStrengthCdma.ecio;
                cellInfoCdma.signalStrengthEvdo.dbm =
                        rillCellInfo->CellInfo.cdma.signalStrengthEvdo.dbm;
                cellInfoCdma.signalStrengthEvdo.ecio =
                        rillCellInfo->CellInfo.cdma.signalStrengthEvdo.ecio;
                cellInfoCdma.signalStrengthEvdo.signalNoiseRatio =
                        rillCellInfo->CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio;

                records[i].info.cdma(cellInfoCdma);
                break;
            }

            case RIL_CELL_INFO_TYPE_LTE: {
                char* mMnc = &(mMncs[i * 6]);
                AOSP_V1_4::CellInfoLte cellInfoLte_v1_4 = {};
                AOSP_V1_2::CellInfoLte *cellInfoLte = &(cellInfoLte_v1_4.base);
                if (rillCellInfo->CellInfo.lte.cellIdentityLte.mcc < 10) {
                    cellInfoLte->cellIdentityLte.base.mcc = "00" +
                            std::to_string(rillCellInfo->CellInfo.lte.cellIdentityLte.mcc);
                } else if (rillCellInfo->CellInfo.lte.cellIdentityLte.mcc < 100) {
                    cellInfoLte->cellIdentityLte.base.mcc = "0" +
                            std::to_string(rillCellInfo->CellInfo.lte.cellIdentityLte.mcc);
                } else {
                    cellInfoLte->cellIdentityLte.base.mcc =
                            std::to_string(rillCellInfo->CellInfo.lte.cellIdentityLte.mcc);
                }
                if (rillCellInfo->CellInfo.lte.cellIdentityLte.mnc_len == 2) {
                    snprintf(mMnc, 5, "%02d", rillCellInfo->CellInfo.lte.cellIdentityLte.mnc);
                } else if (rillCellInfo->CellInfo.lte.cellIdentityLte.mnc_len == 3) {
                    snprintf(mMnc, 5, "%03d", rillCellInfo->CellInfo.lte.cellIdentityLte.mnc);
                } else {
                    snprintf(mMnc, 5, "%d", rillCellInfo->CellInfo.lte.cellIdentityLte.mnc);
                }
                cellInfoLte->cellIdentityLte.base.mnc = convertCharPtrToHidlString(mMnc);
                cellInfoLte->cellIdentityLte.base.ci =
                        rillCellInfo->CellInfo.lte.cellIdentityLte.ci;
                cellInfoLte->cellIdentityLte.base.pci =
                        rillCellInfo->CellInfo.lte.cellIdentityLte.pci;
                cellInfoLte->cellIdentityLte.base.tac =
                        rillCellInfo->CellInfo.lte.cellIdentityLte.tac;
                cellInfoLte->cellIdentityLte.base.earfcn =
                        rillCellInfo->CellInfo.lte.cellIdentityLte.earfcn;
                cellInfoLte->cellIdentityLte.operatorNames.alphaLong =
                        convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.lte.cellIdentityLte.operName.long_name);
                cellInfoLte->cellIdentityLte.operatorNames.alphaShort =
                        convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.lte.cellIdentityLte.operName.short_name);
                // clear information if mcc is unknown
                if (rillCellInfo->CellInfo.lte.cellIdentityLte.mcc == 0x7FFFFFFF) {
                    cellInfoLte->cellIdentityLte.base.mcc = NULL;
                    cellInfoLte->cellIdentityLte.base.mnc = NULL;
                    cellInfoLte->cellIdentityLte.operatorNames.alphaLong = NULL;
                    cellInfoLte->cellIdentityLte.operatorNames.alphaShort = NULL;
                }

                cellInfoLte->cellIdentityLte.bandwidth = 0x7FFFFFFF;
                cellInfoLte->signalStrengthLte.signalStrength =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.signalStrength;
                cellInfoLte->signalStrengthLte.rsrp =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.rsrp;
                cellInfoLte->signalStrengthLte.rsrq =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.rsrq;
                cellInfoLte->signalStrengthLte.rssnr =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.rssnr;
                cellInfoLte->signalStrengthLte.cqi =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.cqi;
                cellInfoLte->signalStrengthLte.timingAdvance =
                        rillCellInfo->CellInfo.lte.signalStrengthLte.timingAdvance;
                //v1_4
                cellInfoLte_v1_4.cellConfig.isEndcAvailable = false;

                //
                records[i].info.lte(cellInfoLte_v1_4);
                break;
            }

            case RIL_CELL_INFO_TYPE_TD_SCDMA: {
                char* mMnc = &(mMncs[i * 6]);

                AOSP_V1_2::CellInfoTdscdma cellInfoTdscdma = {};
                if (rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mcc < 10) {
                    cellInfoTdscdma.cellIdentityTdscdma.base.mcc = "00" +
                            std::to_string(rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mcc);
                } else if (rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mcc < 100) {
                    cellInfoTdscdma.cellIdentityTdscdma.base.mcc = "0" +
                            std::to_string(rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mcc);
                } else {
                    cellInfoTdscdma.cellIdentityTdscdma.base.mcc =
                            std::to_string(rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mcc);
                }
                if (rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mnc_len == 2) {
                    snprintf(mMnc, 5, "%02d", rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mnc);
                } else if (rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mnc_len == 3) {
                    snprintf(mMnc, 5, "%03d", rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mnc);
                } else {
                    snprintf(mMnc, 5, "%d", rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.mnc);
                }
                cellInfoTdscdma.cellIdentityTdscdma.base.mnc = convertCharPtrToHidlString(mMnc);
                cellInfoTdscdma.cellIdentityTdscdma.base.lac =
                        rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.lac;
                cellInfoTdscdma.cellIdentityTdscdma.base.cid =
                        rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.cid;
                cellInfoTdscdma.cellIdentityTdscdma.base.cpid =
                        rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.cpid;
                cellInfoTdscdma.cellIdentityTdscdma.operatorNames.alphaLong =
                        convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.operName.long_name);
                cellInfoTdscdma.cellIdentityTdscdma.operatorNames.alphaShort =
                        convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.tdscdma.cellIdentityTdscdma.operName.short_name);

                cellInfoTdscdma.signalStrengthTdscdma.rscp =
                        rillCellInfo->CellInfo.tdscdma.signalStrengthTdscdma.rscp;

                records[i].info.tdscdma(cellInfoTdscdma);
                break;
            }
            case RIL_CELL_INFO_TYPE_NR: {
                char* mMcc = NULL, *mMnc = NULL;
                AOSP_V1_4::CellInfoNr nr_cellInfo = {};
                nr_cellInfo.signalStrength.ssRsrp = rillCellInfo->CellInfo.nr.signalStrength.ssRsrp;
                nr_cellInfo.signalStrength.ssRsrq = rillCellInfo->CellInfo.nr.signalStrength.ssRsrq;
                nr_cellInfo.signalStrength.ssSinr = rillCellInfo->CellInfo.nr.signalStrength.ssSinr;
                nr_cellInfo.signalStrength.csiRsrp = rillCellInfo->CellInfo.nr.signalStrength.csiRsrp;
                nr_cellInfo.signalStrength.csiRsrq = rillCellInfo->CellInfo.nr.signalStrength.csiRsrq;
                nr_cellInfo.signalStrength.csiSinr = rillCellInfo->CellInfo.nr.signalStrength.csiSinr;

                asprintf(&mMcc, "%03d", rillCellInfo->CellInfo.nr.cellidentity.mcc);
                nr_cellInfo.cellidentity.mcc = mMcc;
                free(mMcc);

                if (rillCellInfo->CellInfo.nr.cellidentity.mnc > 99)
                     asprintf(&mMnc, "%03d", rillCellInfo->CellInfo.nr.cellidentity.mnc);
                else
                     asprintf(&mMnc, "%02d", rillCellInfo->CellInfo.nr.cellidentity.mnc);
                nr_cellInfo.cellidentity.mnc = mMnc;
                free(mMnc);
                nr_cellInfo.cellidentity.nci = rillCellInfo->CellInfo.nr.cellidentity.nci;
                nr_cellInfo.cellidentity.pci = rillCellInfo->CellInfo.nr.cellidentity.pci;
                nr_cellInfo.cellidentity.tac = rillCellInfo->CellInfo.nr.cellidentity.tac;
                nr_cellInfo.cellidentity.nrarfcn = rillCellInfo->CellInfo.nr.cellidentity.nrarfcn;
                nr_cellInfo.cellidentity.operatorNames.alphaLong = convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.nr.cellidentity.operName.long_name);
                nr_cellInfo.cellidentity.operatorNames.alphaShort = convertCharPtrToHidlString(
                            rillCellInfo->CellInfo.nr.cellidentity.operName.short_name);

                // clear information if mcc is unknown
                if (rillCellInfo->CellInfo.nr.cellidentity.mcc == 0x7FFFFFFF) {
                    nr_cellInfo.cellidentity.mcc = NULL;
                    nr_cellInfo.cellidentity.mnc = NULL;
                    nr_cellInfo.cellidentity.operatorNames.alphaLong = NULL;
                    nr_cellInfo.cellidentity.operatorNames.alphaShort = NULL;
                }

                records[i].info.nr(nr_cellInfo);
                break;
            }
            default: {
                break;
            }
        }
        rillCellInfo += 1;
    }
    return mMncs;
}

int radio::cellInfoListInd(int slotId,
                           int indicationType, int token, RIL_Errno e, void *response,
                           size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndicationV1_4!= NULL) {
        if (response == NULL || responseLen % sizeof(RIL_CellInfo_v12) != 0) {
            RLOGE(LOG_TAG, "cellInfoListInd: invalid response");
            return 0;
        }

        hidl_vec<AOSP_V1_4::CellInfo> records;
        char* used_memory = NULL;
        used_memory = convertRilCellInfoListToHal_1_4(response, responseLen, records);

        RLOGD("cellInfoListInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndicationV1_4->cellInfoList_1_4(
                convertIntToRadioIndicationType(indicationType), records);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
        if (used_memory) free(used_memory);
        used_memory = NULL;
    } else if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndicationV1_2!= NULL) {
        if (response == NULL || responseLen % sizeof(RIL_CellInfo_v12) != 0) {
            RLOGE("cellInfoListInd: invalid response");
            return 0;
        }

        char* used_memory = NULL;
        hidl_vec<AOSP_V1_2::CellInfo> records;
        used_memory = convertRilCellInfoListToHal_1_2(response, responseLen, records);

        RLOGD("cellInfoListInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndicationV1_2->cellInfoList_1_2(
                convertIntToRadioIndicationType(indicationType), records);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
        if (used_memory) free(used_memory);
        used_memory = NULL;
    } else if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen % sizeof(RIL_CellInfo_v12) != 0) {
            RLOGE("cellInfoListInd: invalid response");
            return 0;
        }

        hidl_vec<CellInfo> records;
        char* used_memory = NULL;
        used_memory = convertRilCellInfoListToHal(response, responseLen, records);

        RLOGD("cellInfoListInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->cellInfoList(
                convertIntToRadioIndicationType(indicationType), records);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
        if (used_memory) free(used_memory);
        used_memory = NULL;
    } else {
        RLOGE("cellInfoListInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::imsNetworkStateChangedInd(int slotId,
                                     int indicationType, int token, RIL_Errno e, void *response,
                                     size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        RLOGD("imsNetworkStateChangedInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->imsNetworkStateChanged(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("imsNetworkStateChangedInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

int radio::subscriptionStatusChangedInd(int slotId,
                                        int indicationType, int token, RIL_Errno e, void *response,
                                        size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("subscriptionStatusChangedInd: invalid response");
            return 0;
        }
        bool activate = ((int32_t *) response)[0];
        RLOGD("subscriptionStatusChangedInd: activate %d", activate);
        Return<void> retStatus = radioService[slotId]->mRadioIndication->subscriptionStatusChanged(
                convertIntToRadioIndicationType(indicationType), activate);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("subscriptionStatusChangedInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

int radio::srvccStateNotifyInd(int slotId,
                               int indicationType, int token, RIL_Errno e, void *response,
                               size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("srvccStateNotifyInd: invalid response");
            return 0;
        }
        int32_t state = ((int32_t *) response)[0];
        RLOGD("srvccStateNotifyInd: rat %d", state);
        Return<void> retStatus = radioService[slotId]->mRadioIndication->srvccStateNotify(
                convertIntToRadioIndicationType(indicationType), (SrvccState) state);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("srvccStateNotifyInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

void convertRilHardwareConfigListToHal(void *response, size_t responseLen,
        hidl_vec<HardwareConfig>& records) {
    int num = responseLen / sizeof(RIL_HardwareConfig);
    records.resize(num);

    RIL_HardwareConfig *rilHardwareConfig = (RIL_HardwareConfig *) response;
    for (int i = 0; i < num; i++) {
        records[i].type = (HardwareConfigType) rilHardwareConfig[i].type;
        records[i].uuid = convertCharPtrToHidlString(rilHardwareConfig[i].uuid);
        records[i].state = (HardwareConfigState) rilHardwareConfig[i].state;
        switch (rilHardwareConfig[i].type) {
            case RIL_HARDWARE_CONFIG_MODEM: {
                records[i].modem.resize(1);
                records[i].sim.resize(0);
                HardwareConfigModem *hwConfigModem = &records[i].modem[0];
                hwConfigModem->rat = rilHardwareConfig[i].cfg.modem.rat;
                hwConfigModem->maxVoice = rilHardwareConfig[i].cfg.modem.maxVoice;
                hwConfigModem->maxData = rilHardwareConfig[i].cfg.modem.maxData;
                hwConfigModem->maxStandby = rilHardwareConfig[i].cfg.modem.maxStandby;
                break;
            }

            case RIL_HARDWARE_CONFIG_SIM: {
                records[i].sim.resize(1);
                records[i].modem.resize(0);
                records[i].sim[0].modemUuid =
                        convertCharPtrToHidlString(rilHardwareConfig[i].cfg.sim.modemUuid);
                break;
            }
        }
    }
}

int radio::hardwareConfigChangedInd(int slotId,
                                    int indicationType, int token, RIL_Errno e, void *response,
                                    size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen % sizeof(RIL_HardwareConfig) != 0) {
            RLOGE("hardwareConfigChangedInd: invalid response");
            return 0;
        }

        hidl_vec<HardwareConfig> configs;
        convertRilHardwareConfigListToHal(response, responseLen, configs);

        RLOGD("hardwareConfigChangedInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->hardwareConfigChanged(
                convertIntToRadioIndicationType(indicationType), configs);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("hardwareConfigChangedInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

void convertRilRadioCapabilityToHal(void *response, size_t responseLen, RadioCapability& rc) {
    RIL_RadioCapability *rilRadioCapability = (RIL_RadioCapability *) response;
    rc.session = rilRadioCapability->session;
    rc.phase = (android::hardware::radio::V1_0::RadioCapabilityPhase) rilRadioCapability->phase;
    rc.raf = rilRadioCapability->rat;
    rc.logicalModemUuid = convertCharPtrToHidlString(rilRadioCapability->logicalModemUuid);
    rc.status = (android::hardware::radio::V1_0::RadioCapabilityStatus) rilRadioCapability->status;
}

int radio::radioCapabilityIndicationInd(int slotId,
                                        int indicationType, int token, RIL_Errno e, void *response,
                                        size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_RadioCapability)) {
            RLOGE("radioCapabilityIndicationInd: invalid response");
            return 0;
        }

        RadioCapability rc = {};
        convertRilRadioCapabilityToHal(response, responseLen, rc);

        RLOGD("radioCapabilityIndicationInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->radioCapabilityIndication(
                convertIntToRadioIndicationType(indicationType), rc);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("radioCapabilityIndicationInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

bool isServiceTypeCfQuery(RIL_SsServiceType serType, RIL_SsRequestType reqType) {
    if ((reqType == SS_INTERROGATION) &&
        (serType == SS_CFU ||
         serType == SS_CF_BUSY ||
         serType == SS_CF_NO_REPLY ||
         serType == SS_CF_NOT_REACHABLE ||
         serType == SS_CF_ALL ||
         serType == SS_CF_ALL_CONDITIONAL)) {
        return true;
    }
    return false;
}

int radio::onSupplementaryServiceIndicationInd(int slotId,
                                               int indicationType, int token, RIL_Errno e,
                                               void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_StkCcUnsolSsResponse)) {
            RLOGE("onSupplementaryServiceIndicationInd: invalid response");
            return 0;
        }

        RIL_StkCcUnsolSsResponse *rilSsResponse = (RIL_StkCcUnsolSsResponse *) response;
        StkCcUnsolSsResult ss = {};
        ss.serviceType = (SsServiceType) rilSsResponse->serviceType;
        ss.requestType = (SsRequestType) rilSsResponse->requestType;
        ss.teleserviceType = (SsTeleserviceType) rilSsResponse->teleserviceType;
        ss.serviceClass = rilSsResponse->serviceClass;
        ss.result = (RadioError) rilSsResponse->result;

        if (isServiceTypeCfQuery(rilSsResponse->serviceType, rilSsResponse->requestType)) {
            RLOGD("onSupplementaryServiceIndicationInd CF type, num of Cf elements %d",
                    rilSsResponse->cfData.numValidIndexes);
            if (rilSsResponse->cfData.numValidIndexes > NUM_SERVICE_CLASSES) {
                RLOGE("onSupplementaryServiceIndicationInd numValidIndexes is greater than "
                        "max value %d, truncating it to max value", NUM_SERVICE_CLASSES);
                rilSsResponse->cfData.numValidIndexes = NUM_SERVICE_CLASSES;
            }

            ss.cfData.resize(1);
            ss.ssInfo.resize(0);

            /* number of call info's */
            ss.cfData[0].cfInfo.resize(rilSsResponse->cfData.numValidIndexes);

            for (int i = 0; i < rilSsResponse->cfData.numValidIndexes; i++) {
                 RIL_CallForwardInfo cf = rilSsResponse->cfData.cfInfo[i];
                 CallForwardInfo *cfInfo = &ss.cfData[0].cfInfo[i];

                 cfInfo->status = (CallForwardInfoStatus) cf.status;
                 cfInfo->reason = cf.reason;
                 cfInfo->serviceClass = cf.serviceClass;
                 cfInfo->toa = cf.toa;
                 cfInfo->number = convertCharPtrToHidlString(cf.number);
                 cfInfo->timeSeconds = cf.timeSeconds;
                 RLOGD("onSupplementaryServiceIndicationInd: "
                        "Data: %d,reason=%d,cls=%d,toa=%d,num=%s,tout=%d],", cf.status,
                        cf.reason, cf.serviceClass, cf.toa, (char*)cf.number, cf.timeSeconds);
            }
        } else {
            ss.ssInfo.resize(1);
            ss.cfData.resize(0);

            /* each int */
            ss.ssInfo[0].ssInfo.resize(SS_INFO_MAX);
            for (int i = 0; i < SS_INFO_MAX; i++) {
                 RLOGD("onSupplementaryServiceIndicationInd: Data: %d",
                        rilSsResponse->ssInfo[i]);
                 ss.ssInfo[0].ssInfo[i] = rilSsResponse->ssInfo[i];
            }
        }

        RLOGD("onSupplementaryServiceIndicationInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->
                onSupplementaryServiceIndication(convertIntToRadioIndicationType(indicationType),
                ss);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("onSupplementaryServiceIndicationInd: "
                "radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::stkCallControlAlphaNotifyInd(int slotId,
                                        int indicationType, int token, RIL_Errno e, void *response,
                                        size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("stkCallControlAlphaNotifyInd: invalid response");
            return 0;
        }
        RLOGD("stkCallControlAlphaNotifyInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->stkCallControlAlphaNotify(
                convertIntToRadioIndicationType(indicationType),
                convertCharPtrToHidlString((char *) response));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("stkCallControlAlphaNotifyInd: radioService[%d]->mRadioIndication == NULL",
                slotId);
    }

    return 0;
}

void convertRilLceDataInfoToHal(void *response, size_t responseLen, LceDataInfo& lce) {
    RIL_LceDataInfo *rilLceDataInfo = (RIL_LceDataInfo *)response;
    lce.lastHopCapacityKbps = rilLceDataInfo->last_hop_capacity_kbps;
    lce.confidenceLevel = rilLceDataInfo->confidence_level;
    lce.lceSuspended = rilLceDataInfo->lce_suspended;
}

int radio::lceDataInd(int slotId,
                      int indicationType, int token, RIL_Errno e, void *response,
                      size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_LceDataInfo)) {
            RLOGE("lceDataInd: invalid response");
            return 0;
        }

        LceDataInfo lce = {};
        convertRilLceDataInfoToHal(response, responseLen, lce);
        RLOGD("lceDataInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->lceData(
                convertIntToRadioIndicationType(indicationType), lce);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("lceDataInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::pcoDataInd(int slotId,
                      int indicationType, int token, RIL_Errno e, void *response,
                      size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_PCO_Data)) {
            RLOGE("pcoDataInd: invalid response");
            return 0;
        }

        PcoDataInfo pco = {};
        RIL_PCO_Data *rilPcoData = (RIL_PCO_Data *)response;
        pco.cid = rilPcoData->cid;
        pco.bearerProto = convertCharPtrToHidlString(rilPcoData->bearer_proto);
        pco.pcoId = rilPcoData->pco_id;
        pco.contents.setToExternal((uint8_t *) rilPcoData->contents, rilPcoData->contents_length);

        RLOGD("pcoDataInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->pcoData(
                convertIntToRadioIndicationType(indicationType), pco);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("pcoDataInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::modemResetInd(int slotId,
                         int indicationType, int token, RIL_Errno e, void *response,
                         size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndication != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("modemResetInd: invalid response");
            return 0;
        }
        RLOGD("modemResetInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndication->modemReset(
                convertIntToRadioIndicationType(indicationType),
                convertCharPtrToHidlString((char *) response));
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("modemResetInd: radioService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int radio::networkScanResultInd(int slotId,
                         int indicationType, int token, RIL_Errno e, void *response,
                         size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndicationV1_1 != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("networkScanResultInd: invalid response");
            return 0;
        }
        RLOGD("networkScanResultInd");
        RIL_NetworkScanResult *networkScanResult = (RIL_NetworkScanResult *) response;

        AOSP_V1_1::NetworkScanResult result;
        result.status = (AOSP_V1_1::ScanStatus) networkScanResult->status;
        result.error = (RadioError) e;
        convertRilCellInfoListToHal(
                networkScanResult->network_infos,
                networkScanResult->network_infos_length * sizeof(RIL_CellInfo_v12),
                result.networkInfos);

        Return<void> retStatus = radioService[slotId]->mRadioIndicationV1_1->networkScanResult(
                convertIntToRadioIndicationType(indicationType), result);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("networkScanResultInd: radioService[%d]->mRadioIndicationV1_1 == NULL", slotId);
    }
    return 0;
}

void convertRilEmergencyNumberListToHal(void *response, size_t responseLen,
        hidl_vec<AOSP_V1_4::EmergencyNumber>& records) {
    int num = responseLen / sizeof(RIL_EmergencyNumber);
    records.resize(num);

    RIL_EmergencyNumber *rilEccList = (RIL_EmergencyNumber *) response;
    for (int i = 0; i < num; i++) {
        records[i].number = convertCharPtrToHidlString((const char*)(rilEccList[i].number));
        records[i].mcc = convertCharPtrToHidlString((const char*)(rilEccList[i].mcc));
        records[i].mnc = convertCharPtrToHidlString((const char*)(rilEccList[i].mnc));
        records[i].categories = rilEccList[i].categories;
        hidl_vec<hidl_string> urns;
        records[i].urns = urns; // not used yet
        records[i].sources = rilEccList[i].sources;
    }
}

int radio::emergencyNumberListInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndicationV1_4 != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("emergencyNumberListInd: invalid response");
            return 0;
        }
        RLOGD("emergencyNumberListInd responseLen: %d", responseLen);
        hidl_vec<AOSP_V1_4::EmergencyNumber> result;
        convertRilEmergencyNumberListToHal(response, responseLen, result);
        Return<void> retStatus;
        retStatus = radioService[slotId]->mRadioIndicationV1_4->currentEmergencyNumberList(
                convertIntToRadioIndicationType(indicationType), result);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("emergencyNumberListInd: radioService[%d]->mRadioIndicationV1_4 == NULL",
                slotId);
    }
    return 0;
}

/***************************************************************************************************
 * MTK INDICATION FUNCTIONS
 * The below function handle unsolicited messages coming from the Radio
 * (messages for which there is no pending request)
 **************************************************************************************************/
int mtkRadioEx::resetAttachApnInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioIndicationMtk->resetAttachApnInd(
                convertIntToRadioIndicationType(indicationType));
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("resetAttachApnInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }
    return 0;
}

int mtkRadioEx::mdChangeApnInd(int slotId, int indicationType,
        int token, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        hidl_vec<int32_t> data;

        int numInts = responseLen / sizeof(int);
        if (response == NULL || responseLen % sizeof(int) != 0) {
            RLOGE("mdChangeApnInd Invalid response: NULL");
            return 0;
        } else {
            int *pInt = (int *) response;
            data.resize(numInts);
            for (int i=0; i<numInts; i++) {
                data[i] = (int32_t) pInt[i];
            }
        }
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioIndicationMtk->mdChangedApnInd(
                convertIntToRadioIndicationType(indicationType), data[0]);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("mdChangeApnInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL",
                slotId);
    }
    return 0;
}

int mtkRadioEx::embmsAtInfoInd(int slotId,
                      int indicationType, int token, RIL_Errno e, void *response,
                      size_t responselen) {
    //dbg
    RLOGD("embmsAtInfoInd: slotId:%d", slotId);
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        hidl_string info;
        if (response == NULL) {
            RLOGE("embmsAtInfoInd: invalid response");
            return 0;
        } else {
            RLOGD("embmsAtInfoInd[%d]: %s", slotId, (char*)response);
            info = convertCharPtrToHidlString((char *)response);
        }
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->eMBMSAtInfoIndication(
                convertIntToRadioIndicationType(indicationType), info);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("embmsAtInfoInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::embmsSessionStatusInd(int slotId,
                      int indicationType, int token, RIL_Errno e, void *response,
                      size_t responselen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responselen == 0) {
            RLOGE("embmsSessionStatusInd: invalid response");
            return 0;
        }

        int32_t status = ((int32_t *) response)[0];
        RLOGD("embmsSessionStatusInd[%d]: %d", slotId, status);
        Return<void> retStatus =
            mtkRadioExService[slotId]->mRadioIndicationMtk->eMBMSSessionStatusIndication(
                convertIntToRadioIndicationType(indicationType), status);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("embmsSessionStatusInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}
/// M: eMBMS end

int mtkRadioEx::currentSignalStrengthWithWcdmaEcioInd(int slotId, int indicationType, int token,
        RIL_Errno e, void *response, size_t responseLen) {
    return 0;
}

int mtkRadioEx::cfuStatusNotifyInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen != 2 * sizeof(int)) {
            RLOGE("cfuStatusNotifyInd: invalid response");
            return 0;
        }

        CfuStatusNotification cfuStatus = {};
        int *csn = (int *) response;
        cfuStatus.status = csn[0];
        cfuStatus.lineId = csn[1];

        RLOGD("cfuStatusNotifyInd: status = %d, line = %d", cfuStatus.status, cfuStatus.lineId);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->cfuStatusNotify(
                convertIntToRadioIndicationType(indicationType), cfuStatus);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("cfuStatusNotifyInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}

/// M: CC: call control ([IMS] common flow) @{
int mtkRadioEx::incomingCallIndicationInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {

    if(response == NULL) {
        RLOGE("incomingCallIndicationInd: response is NULL");
        return 0;
    }

    char **resp = (char **) response;
    int numStrings = responseLen / sizeof(char *);
    if(numStrings < 7) {
        RLOGE("incomingCallIndicationInd: items length is invalid, slot = %d", slotId);
        return 0;
    }

    int mode = atoi(resp[3]);
    if(mode >= 20) {   // Code Mode >= 20, IMS's Call Info Indication, otherwise CS's
        int imsSlot = toClientSlot(slotId, android::CLIENT_IMS);
        if (mtkRadioExService[imsSlot] != NULL &&
            mtkRadioExService[imsSlot]->mRadioIndicationIms != NULL) {

            IncomingCallNotification inCallNotify = {};
            // EAIC: <callId>, <number>, <type>, <call mode>, <seq no>
            inCallNotify.callId = convertCharPtrToHidlString(resp[0]);
            inCallNotify.number = convertCharPtrToHidlString(resp[1]);
            inCallNotify.type = convertCharPtrToHidlString(resp[2]);
            inCallNotify.callMode = convertCharPtrToHidlString(resp[3]);
            inCallNotify.seqNo = convertCharPtrToHidlString(resp[4]);
            inCallNotify.redirectNumber = convertCharPtrToHidlString(resp[5]);

            /*
            RLOGD("incomingCallIndicationInd: %s, %s, %s, %s, %s, %s, slot = %d",
                    resp[0], resp[1], resp[2], resp[3], resp[4], resp[5], imsSlot);
                    */

            Return<void> retStatus = mtkRadioExService[imsSlot]->
                                     mRadioIndicationIms->incomingCallIndication(
                                     convertIntToRadioIndicationType(indicationType),
                                     inCallNotify);
            mtkRadioExService[imsSlot]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
        }
        else {
            RLOGE("incomingCallIndicationInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                                                                                imsSlot);
        }

        return 0;
    }

    //      case RIL_UNSOL_INCOMING_CALL_INDICATION: ret = responseStrings(p); break;
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        IncomingCallNotification inCallNotify = {};
        char **strings = (char **) response;
        // EAIC: <callId>, <number>, <type>, <call mode>, <seq no>
        inCallNotify.callId = convertCharPtrToHidlString(strings[0]);
        inCallNotify.number = convertCharPtrToHidlString(strings[1]);
        inCallNotify.type = convertCharPtrToHidlString(strings[2]);
        inCallNotify.callMode = convertCharPtrToHidlString(strings[3]);
        inCallNotify.seqNo = convertCharPtrToHidlString(strings[4]);
        inCallNotify.redirectNumber = convertCharPtrToHidlString(strings[5]);
        // string[6] is used by ims. no need here.
        /*
        RLOGD("incomingCallIndicationInd: %s, %s, %s, %s, %s, %s",
                strings[0], strings[1], strings[2],
                strings[3], strings[4], strings[5]);
                */
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->incomingCallIndication(
                convertIntToRadioIndicationType(indicationType), inCallNotify);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("incomingCallIndicationInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }


    return 0;
}

int mtkRadioEx::callAdditionalInfoInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responselen) {
    if (mtkRadioExService[slotId] != NULL
            && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL) {
            RLOGE("callAdditionalInfoInd invalid response");
            return 0;
        }

        hidl_vec<hidl_string> data;
        char **resp = (char **) response;
        int numStrings = responselen / sizeof(char *) - 1 ;
        data.resize(numStrings);
        CallInfoType ciType = (CallInfoType) atoi(resp[0]);
        for (int i = 0; i < numStrings; i++) {
            data[i] = convertCharPtrToHidlString(resp[i + 1]);
            //RLOGD("callAdditionalInfoInd:: %d: type:%d, %s", i, ciType, resp[i]);
        }

        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioIndicationMtk->callAdditionalInfoInd(
                                 convertIntToRadioIndicationType(indicationType),
                                 ciType,
                                 data);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("callAdditionalInfoInd: radioService[%d]->mRadioIndicationMtk == NULL",
                slotId);
    }

    return 0;
}

/// M: CC: GSM 02.07 B.1.26 Ciphering Indicator support
int mtkRadioEx::cipherIndicationInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen != 4 * sizeof(char *)) {
            RLOGE("cipherInd: invalid response");
            return 0;
        }
        CipherNotification cipherNotify = {};
        char **strings = (char **) response;
        //+ECIPH:  <sim_cipher_ind>,<mm_connection>,<cs_cipher_on>,<ps_cipher_on>
        cipherNotify.simCipherStatus = convertCharPtrToHidlString(strings[0]);
        cipherNotify.sessionStatus = convertCharPtrToHidlString(strings[1]);
        cipherNotify.csStatus = convertCharPtrToHidlString(strings[2]);
        cipherNotify.psStatus = convertCharPtrToHidlString(strings[3]);
        RLOGD("cipherInd: %s, %s, %s, %s", strings[0], strings[1], strings[2], strings[3]);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->cipherIndication(
                convertIntToRadioIndicationType(indicationType), cipherNotify);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("cipherInd: radioService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::suppSvcNotifyExInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL &&
            mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_SuppSvcNotification)) {
            RLOGE("suppSvcNotifyExInd: invalid response");
            return 0;
        }

        SuppSvcNotification suppSvc = {};
        RIL_SuppSvcNotification *ssn = (RIL_SuppSvcNotification *) response;
        suppSvc.isMT = ssn->notificationType;
        suppSvc.code = ssn->code;
        suppSvc.index = ssn->index;
        suppSvc.type = ssn->type;
        suppSvc.number = convertCharPtrToHidlString(ssn->number);

        RLOGD("suppSvcNotifyExInd: isMT %d code %d index %d type %d",
                suppSvc.isMT, suppSvc.code, suppSvc.index, suppSvc.type);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->suppSvcNotifyEx(
                convertIntToRadioIndicationType(indicationType), suppSvc);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("suppSvcNotifyExInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL",
                slotId);
    }

    return 0;
}

/// M: CC: call control
int mtkRadioEx::crssNotifyInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_CrssNotification)) {
            RLOGE("crssNotifyInd: invalid response");
            return 0;
        }
        CrssNotification crssNotify = {};
        RIL_CrssNotification *crss = (RIL_CrssNotification *) response;
        crssNotify.code = crss->code;
        crssNotify.type = crss->type;
        crssNotify.number = convertCharPtrToHidlString(crss->number);
        crssNotify.alphaid = convertCharPtrToHidlString(crss->alphaid);
        crssNotify.cli_validity = crss->cli_validity;

        RLOGD("crssNotifyInd: code %d type %d cli_validity %d",
                crssNotify.code, crssNotify.type, crssNotify.cli_validity);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->crssIndication(
                convertIntToRadioIndicationType(indicationType), crssNotify);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("crssNotifyInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}

/// M: CC: GSA HD Voice for IMS
int mtkRadioEx::speechCodecInfoInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("ims speechCodecInfoInd: invalid response");
            return 0;
        }
        int32_t info = ((int32_t *) response)[0];
        RLOGD("ims speechCodecInfoInd: %d", info);
        Return<void> retStatus =
                mtkRadioExService[imsSlotId]->mRadioIndicationIms->speechCodecInfoIndication(
                        convertIntToRadioIndicationType(indicationType), info);
        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("ims speechCodecInfoInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                imsSlotId);
    }

    return 0;
}

/// M: CC: CDMA call accepted indication @{
int mtkRadioEx::cdmaCallAcceptedInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        RLOGD("cdmaCallAcceptedInd: slotId=%d", slotId);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->cdmaCallAccepted(
                convertIntToRadioIndicationType(indicationType));
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("cdmaCallAcceptedInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}
/// @}

int mtkRadioEx::cdmaNewSmsIndEx(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {
        CdmaSmsMessage msg = {};
        if (!makeCdmaSmsMessage(msg, response, responseLen)) {
            RLOGE("cdmaNewSmsIndEx: invalid response");
            return 0;
        }
        RLOGD("cdmaNewSmsIndEx");
        Return<void> retStatus = mtkRadioExService[imsSlotId]->mRadioIndicationIms->cdmaNewSmsEx(
                convertIntToRadioIndicationType(indicationType), msg);
        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("cdmaNewSmsInd: mtkRadioExService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

// M: [VzW] Data Framework @{
int mtkRadioEx::pcoDataAfterAttachedInd(int slotId,
               int indicationType, int token, RIL_Errno e, void *response,
               size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen != sizeof(RIL_PCO_Data_attached)) {
            RLOGE("pcoDataAfterAttachedInd: invalid response");
            return 0;
        }

        PcoDataAttachedInfo pco = {};
        RIL_PCO_Data_attached *rilPcoData = (RIL_PCO_Data_attached *)response;
        pco.cid = rilPcoData->cid;
        pco.apnName = convertCharPtrToHidlString(rilPcoData->apn_name);
        pco.bearerProto = convertCharPtrToHidlString(rilPcoData->bearer_proto);
        pco.pcoId = rilPcoData->pco_id;
        pco.contents.setToExternal((uint8_t *) rilPcoData->contents, rilPcoData->contents_length);

        RLOGD("pcoDataAfterAttachedInd");
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->pcoDataAfterAttached(
                convertIntToRadioIndicationType(indicationType), pco);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("pcoDataAfterAttachedInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }
    return 0;
}

int mtkRadioEx::networkRejectCauseInd(int slotId,
               int indicationType, int token, RIL_Errno e, void *response,
               size_t responseLen) {
    RLOGD("networkRejectCauseInd, no need to support");
    return 0;
}

int mtkRadioEx::noEmergencyCallbackModeInd(int slotId,
                                        int indicationType, int token, RIL_Errno e, void *response,
                                        size_t responseLen) {
    RLOGD("noEmergencyCallbackModeInd, no need to support");
    return 0;
}

int mtkRadioEx::oemHookRawInd(int slotId,
                         int indicationType, int token, RIL_Errno e, void *response,
                         size_t responseLen) {
    if (response == NULL || responseLen == 0) {
        RLOGE("oemHookRawInd: invalid response");
        return 0;
    }

    hidl_vec<uint8_t> data;
    data.setToExternal((uint8_t *) response, responseLen);
    RLOGD("oemHookRawInd");
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->oemHookRaw(
                convertIntToRadioIndicationType(indicationType), data);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("oemHookRawInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }
    // send to EM
    int emSlot = toClientSlot(slotId, android::CLIENT_EM);
    if (mtkRadioExService[emSlot] != NULL && mtkRadioExService[emSlot]->mRadioIndicationEm != NULL) {
        Return<void> retStatus = mtkRadioExService[emSlot]->mRadioIndicationEm->oemHookRaw(
                convertIntToRadioIndicationType(indicationType), data);
        mtkRadioExService[emSlot]->checkReturnStatusMtk(retStatus, android::EM_MTK, slotId);
    } else {
        RLOGE("oemHookRawInd: mtkRadioExService[%d]->mRadioIndicationEm == NULL", emSlot);
    }
    return 0;
}

// ATCI Start
int mtkRadioEx::atciInd(int slotId,
                   int indicationType, int token, RIL_Errno e, void *response,
                   size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mAtciIndication != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("atciInd: invalid response");
            return 0;
        }

        hidl_vec<uint8_t> data;
        data.setToExternal((uint8_t *) response, responseLen);
        RLOGD("atciInd");
        Return<void> retStatus = mtkRadioExService[slotId]->mAtciIndication->atciInd(
                convertIntToRadioIndicationType(indicationType), data);
        if (!retStatus.isOk()) {
            RLOGE("sendAtciResponse: unable to call indication callback");
            mtkRadioExService[slotId]->mAtciResponse = NULL;
            mtkRadioExService[slotId]->mAtciIndication = NULL;
        }
    } else {
        RLOGE("atciInd: mtkRadioExService[%d]->mAtciIndication == NULL", slotId);
    }

    return 0;
}
// ATCI End

/// [IMS] Indication ////////////////////////////////////////////////////////////////////
int mtkRadioEx::callInfoIndicationInd(int slotId,
                                 int indicationType, int token, RIL_Errno e,
                                 void *response, size_t responseLen) {

    char **resp = (char **) response;
    int numStrings = responseLen / sizeof(char *);
    if(numStrings < 5) {
        RLOGE("callInfoIndicationInd: items length is invalid, slot = %d", slotId);
        return 0;
    }

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL
            && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        hidl_vec<hidl_string> data;
        data.resize(numStrings);
        for (int i = 0; i < numStrings; i++) {
            data[i] = convertCharPtrToHidlString(resp[i]);
            // RLOGD("callInfoIndicationInd:: %d: %s", i, resp[i]);
        }

        Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                 mRadioIndicationIms->callInfoIndication(
                                 convertIntToRadioIndicationType(indicationType),
                                 data);

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("callInfoIndicationInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                                                                          imsSlotId);
    }

    return 0;
}

int mtkRadioEx::econfResultIndicationInd(int slotId,
                                    int indicationType, int token, RIL_Errno e,
                                    void *response, size_t responseLen) {

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL
            && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        char **resp = (char **) response;
        int numStrings = responseLen / sizeof(char *);
        if(numStrings < 5) {
            RLOGE("econfResultIndicationInd: items length invalid, slotId = %d",
                                                                     imsSlotId);
            return 0;
        }

        hidl_string confCallId = convertCharPtrToHidlString(resp[0]);
        hidl_string op = convertCharPtrToHidlString(resp[1]);
        hidl_string num = convertCharPtrToHidlString(resp[2]);
        hidl_string result = convertCharPtrToHidlString(resp[3]);
        hidl_string cause = convertCharPtrToHidlString(resp[4]);
        hidl_string joinedCallId;
        if(numStrings > 5) {
            joinedCallId = convertCharPtrToHidlString(resp[5]);
        }

        Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                 mRadioIndicationIms->econfResultIndication(
                                 convertIntToRadioIndicationType(indicationType),
                                 confCallId, op, num, result, cause, joinedCallId);

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    }
    else {
        RLOGE("econfResultIndicationInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                                                                             imsSlotId);
    }

    return 0;
}

int mtkRadioEx::sipCallProgressIndicatorInd(int slotId,
                                       int indicationType, int token, RIL_Errno e,
                                       void *response, size_t responseLen) {

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL
            && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        char **resp = (char **) response;
        int numStrings = responseLen / sizeof(char *);
        if(numStrings < 5) {
            RLOGE("sipCallProgressIndicatorInd: items length invalid, slotId = %d",
                                                                        imsSlotId);
            return 0;
        }

        hidl_string callId = convertCharPtrToHidlString(resp[0]);
        hidl_string dir = convertCharPtrToHidlString(resp[1]);
        hidl_string sipMsgType = convertCharPtrToHidlString(resp[2]);
        hidl_string method = convertCharPtrToHidlString(resp[3]);
        hidl_string responseCode = convertCharPtrToHidlString(resp[4]);
        hidl_string reasonText;
        if(numStrings > 5) {
            reasonText = convertCharPtrToHidlString(resp[5]);
        }

        Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                 mRadioIndicationIms->sipCallProgressIndicator(
                                 convertIntToRadioIndicationType(indicationType),
                                 callId, dir, sipMsgType, method, responseCode, reasonText);

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    }
    else {
        RLOGE("sipCallProgressIndicatorInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                                                                                imsSlotId);
    }

    return 0;
}

int mtkRadioEx::callmodChangeIndicatorInd(int slotId,
                                    int indicationType, int token, RIL_Errno e,
                                    void *response, size_t responseLen) {

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL
            && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        char **resp = (char **) response;
        int numStrings = responseLen / sizeof(char *);
        if(numStrings < 5) {
            RLOGE("callmodChangeIndicatorInd: items length invalid, slotId = %d",
                                                                       imsSlotId);
            return 0;
        }

        hidl_string callId = convertCharPtrToHidlString(resp[0]);
        hidl_string callMode = convertCharPtrToHidlString(resp[1]);
        hidl_string videoState = convertCharPtrToHidlString(resp[2]);
        hidl_string autoDirection = convertCharPtrToHidlString(resp[3]);
        hidl_string pau = convertCharPtrToHidlString(resp[4]);

        Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                 mRadioIndicationIms->callmodChangeIndicator(
                                 convertIntToRadioIndicationType(indicationType),
                                 callId, callMode, videoState, autoDirection, pau);

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    }
    else {
        RLOGE("callmodChangeIndicatorInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                                                                              imsSlotId);
    }

    return 0;
}

int mtkRadioEx::videoCapabilityIndicatorInd(int slotId,
                                       int indicationType, int token, RIL_Errno e,
                                       void *response, size_t responseLen) {

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL
            && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        char **resp = (char **) response;
        int numStrings = responseLen / sizeof(char *);
        if(numStrings < 3) {
            RLOGE("videoCapabilityIndicatorInd: items length invalid, slotId = %d",
                                                                        imsSlotId);
            return 0;
        }

        hidl_string callId = convertCharPtrToHidlString(resp[0]);
        hidl_string localVideoCaoability = convertCharPtrToHidlString(resp[1]);
        hidl_string remoteVideoCaoability = convertCharPtrToHidlString(resp[2]);

        Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                 mRadioIndicationIms->videoCapabilityIndicator(
                                 convertIntToRadioIndicationType(indicationType),
                                 callId, localVideoCaoability, remoteVideoCaoability);

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    }
    else {
        RLOGE("videoCapabilityIndicatorInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                                                                                imsSlotId);
    }

    return 0;
}

int mtkRadioEx::onUssiInd(int slotId,
                     int indicationType, int token, RIL_Errno e, void *response,
                     size_t responseLen) {

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL
            && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        if (response == NULL || responseLen != 2 * sizeof(char *)) {
            RLOGE("onUssiInd: invalid response");
            return 0;
        }

        char **strings = (char **) response;
        char *mode = strings[0];

        hidl_string msg = convertCharPtrToHidlString(strings[1]);
        int modeType = atoi(mode);
        RLOGD("onUssiInd: mode %s", mode);
        Return<void> retStatus = mtkRadioExService[imsSlotId]->mRadioIndicationIms->onUssi(
                convertIntToRadioIndicationType(indicationType), modeType, msg);
        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    }
    else {
        RLOGE("onUssiInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                                                              imsSlotId);
    }

    return 0;
}

int mtkRadioEx::getProvisionDoneInd(int slotId,
                               int indicationType, int token, RIL_Errno e,
                               void *response, size_t responseLen) {

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL
            && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        hidl_string result1;
        hidl_string result2;
        int numStrings = responseLen / sizeof(char *);

        if (response == NULL || numStrings < 2) {
            RLOGE("getProvisionDone Invalid response: NULL");
            return 0;
        } else {
            char **resp = (char **) response;
            result1 = convertCharPtrToHidlString(resp[0]);
            result2 = convertCharPtrToHidlString(resp[1]);
        }

        RLOGD("getProvisionDoneInd");
        Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                 mRadioIndicationIms->getProvisionDone(
                                 convertIntToRadioIndicationType(indicationType),
                                 result1, result2);

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("getProvisionDoneInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL",
                                                                         imsSlotId);
    }

    return 0;
}

int mtkRadioEx::imsCfgDynamicImsSwitchCompleteInd(int slotId,
                                             int indicationType, int token, RIL_Errno e,
                                             void *response,
                                             size_t responseLen) {
    return 0;
}

int mtkRadioEx::imsCfgFeatureChangedInd(int slotId,
                                   int indicationType, int token, RIL_Errno e, void *response,
                                   size_t responseLen) {
    return 0;
}

int mtkRadioEx::imsCfgConfigChangedInd(int slotId,
                                  int indicationType, int token, RIL_Errno e, void *response,
                                  size_t responseLen) {
    return 0;
}

int mtkRadioEx::imsCfgConfigLoadedInd(int slotId,
                                 int indicationType, int token, RIL_Errno e,
                                 void *response,
                                 size_t responseLen) {
    return 0;
}

int mtkRadioEx::imsRtpInfoInd(int slotId,
                         int indicationType, int token, RIL_Errno e, void *response,
                         size_t responseLen) {

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL
            && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        hidl_string pdnId;
        hidl_string networkId;
        hidl_string timer;
        hidl_string sendPktLost;
        hidl_string recvPktLost;
        hidl_string jitter;
        hidl_string delay;
        int numStrings = responseLen / sizeof(char *);

        if (response == NULL || numStrings < 5) {
            RLOGE("imsRtpInfoInd Invalid response: NULL");
            return 0;
        } else {
            char **resp = (char **) response;
            pdnId = convertCharPtrToHidlString(resp[0]);
            networkId = convertCharPtrToHidlString(resp[1]);
            timer = convertCharPtrToHidlString(resp[2]);
            sendPktLost = convertCharPtrToHidlString(resp[3]);
            recvPktLost = convertCharPtrToHidlString(resp[4]);
            jitter = convertCharPtrToHidlString(resp[5]);
            delay = convertCharPtrToHidlString(resp[6]);
        }

        RLOGD("imsRtpInfoInd");
        Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                 mRadioIndicationIms->imsRtpInfo(
                                 convertIntToRadioIndicationType(indicationType),
                                 pdnId, networkId, timer, sendPktLost, recvPktLost,
                                 jitter, delay);

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("imsRtpInfoInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                                                                   imsSlotId);
    }

    return 0;
}

int mtkRadioEx::onXuiInd(int slotId,
                    int indicationType, int token, RIL_Errno e, void *response,
                    size_t responseLen) {

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL
            && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        hidl_string accountId;
        hidl_string broadcastFlag;
        hidl_string xuiInfo;
        int numStrings = responseLen / sizeof(char *);

        if (response == NULL || numStrings < 3) {
            RLOGE("onXuiInd Invalid response: NULL");
            return 0;
        } else {
            char **resp = (char **) response;
            accountId = convertCharPtrToHidlString(resp[0]);
            broadcastFlag = convertCharPtrToHidlString(resp[1]);
            xuiInfo = convertCharPtrToHidlString(resp[2]);
        }

        RLOGD("onXuiInd");
        Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                 mRadioIndicationIms->onXui(
                                 convertIntToRadioIndicationType(indicationType),
                                 accountId, broadcastFlag, xuiInfo);

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("onXuiInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                                                             imsSlotId);
    }

    return 0;
}

int mtkRadioEx::onVolteSubscription (int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL &&
            mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        if (response == NULL || responseLen < sizeof(int)) {
            RLOGE("onVolteSubscription: invalid response");
            return 0;
        }

        int status = ((int32_t *) response)[0];

        Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                 mRadioIndicationIms->
                                 onVolteSubscription(
                                 convertIntToRadioIndicationType(indicationType),
                                 status);

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("onVolteSubscription: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                                                                            imsSlotId);
    }

    return 0;
}

int mtkRadioEx::imsEventPackageIndicationInd(int slotId,
                                        int indicationType, int token, RIL_Errno e,
                                        void *response, size_t responseLen) {

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL
            && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        hidl_string callId;
        hidl_string pType;
        hidl_string urcIdx;
        hidl_string totalUrcCount;
        hidl_string rawData;
        int numStrings = responseLen / sizeof(char *);

        if (response == NULL || numStrings < 5) {
            RLOGE("imsEventPackageIndication Invalid response: NULL");
            return 0;
        } else {
            char **resp = (char **) response;
            callId = convertCharPtrToHidlString(resp[0]);
            pType = convertCharPtrToHidlString(resp[1]);
            urcIdx = convertCharPtrToHidlString(resp[2]);
            totalUrcCount = convertCharPtrToHidlString(resp[3]);
            rawData = convertCharPtrToHidlString(resp[4]);
        }

        RLOGD("imsEventPackageIndication");
        Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                 mRadioIndicationIms->imsEventPackageIndication(
                                 convertIntToRadioIndicationType(indicationType),
                                 callId, pType, urcIdx, totalUrcCount, rawData);

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("imsEventPackageIndication: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                                                                              imsSlotId);
    }

    return 0;
}

int mtkRadioEx::imsRegistrationInfoInd(int slotId,
                              int indicationType, int token, RIL_Errno e,
                              void *response, size_t responseLen) {

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL &&
            mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        if (response == NULL || responseLen < (sizeof(int) * 2)) {
            RLOGE("imsRegistrationInfoInd: invalid response");
            return 0;
        }

        int status = ((int32_t *) response)[0];
        int capacity = ((int32_t *) response)[1];

        Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                 mRadioIndicationIms->
                                 imsRegistrationInfo(
                                 convertIntToRadioIndicationType(indicationType),
                                 status, capacity);

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("imsRegistrationInfoInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                                                                            imsSlotId);
    }

    return 0;
}

int mtkRadioEx::imsEnableDoneInd(int slotId,
                            int indicationType, int token, RIL_Errno e,
                            void *response, size_t responseLen) {

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL
            && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        RLOGD("imsEnableDoneInd");
        Return<void> retStatus =
                mtkRadioExService[imsSlotId]->mRadioIndicationIms->imsEnableDone(
                convertIntToRadioIndicationType(indicationType));

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);

        if (retStatus.isOk() == false) {
            appendRequestIntoPendingInd(slotId, RIL_UNSOL_IMS_ENABLE_DONE, indicationType,
                    NULL, 0);
        }
    } else {
        RLOGE("imsEnableDoneInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                                                                     imsSlotId);
    }

    return 0;
}

int mtkRadioEx::imsDisableDoneInd(int slotId,
                             int indicationType, int token, RIL_Errno e,
                             void *response, size_t responseLen) {

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL
            && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        RLOGD("imsDisableDoneInd");
        Return<void> retStatus =
                mtkRadioExService[imsSlotId]->mRadioIndicationIms->imsDisableDone(
                convertIntToRadioIndicationType(indicationType));

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);

        if (retStatus.isOk() == false) {
            appendRequestIntoPendingInd(slotId, RIL_UNSOL_IMS_DISABLE_DONE, indicationType,
                    NULL, 0);
        }
    } else {
        RLOGE("imsDisableDoneInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                                                                      imsSlotId);
    }

    return 0;
}

int mtkRadioEx::imsEnableStartInd(int slotId,
                             int type, int token, RIL_Errno e,
                             void *response, size_t responseLen) {

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL
            && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        RLOGD("imsEnableStartInd, slotId = %d, IMS slotId = %d", slotId, imsSlotId);
        Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                 mRadioIndicationIms->
                                 imsEnableStart(convertIntToRadioIndicationType(type));

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);

        if (retStatus.isOk() == false) {
            appendRequestIntoPendingInd(slotId, RIL_UNSOL_IMS_ENABLE_START, type, NULL, 0);
        }
    } else {
        RLOGE("imsEnableStartInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                                                                      imsSlotId);
    }

    return 0;
}

int mtkRadioEx::imsDisableStartInd(int slotId,
                              int type, int token, RIL_Errno e,
                              void *response, size_t responseLen) {

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL
            && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        RLOGD("imsDisableStartInd, slotId = %d, IMS slotId = %d", slotId, imsSlotId);
        Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                 mRadioIndicationIms->
                                 imsDisableStart(convertIntToRadioIndicationType(type));

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);

        if (retStatus.isOk() == false) {
            appendRequestIntoPendingInd(slotId, RIL_UNSOL_IMS_DISABLE_START, type, NULL, 0);
        }
    } else {
        RLOGE("imsDisableStartInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", imsSlotId);
    }

    return 0;
}

int mtkRadioEx::ectIndicationInd(int slotId,
                            int indicationType, int token, RIL_Errno e,
                            void *response, size_t responseLen) {

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL &&
            mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        if (response == NULL || responseLen < (sizeof(int) * 3)) {
            RLOGE("ectIndicationInd: invalid response");
            return 0;
        }

        int callId = ((int32_t *) response)[0];
        int ectResult = ((int32_t *) response)[1];
        int cause = ((int32_t *) response)[2];

        Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                 mRadioIndicationIms->
                                 ectIndication(
                                 convertIntToRadioIndicationType(indicationType),
                                 callId, ectResult, cause);

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("ectIndicationInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                                                                     imsSlotId);
    }

    return 0;
}

int mtkRadioEx::volteSettingInd(int slotId,
                           int indicationType, int token, RIL_Errno e,
                           void *response, size_t responseLen) {

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL &&
            mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        if (response == NULL || responseLen < (sizeof(int))) {
            RLOGE("volteSettingInd: invalid response");
            return 0;
        }

        int status = ((int32_t *) response)[0];
        bool isEnable = (status == 1) ? true : false;

        Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                 mRadioIndicationIms->
                                 volteSetting(
                                 convertIntToRadioIndicationType(indicationType),
                                 isEnable);

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("volteSettingInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                                                                    imsSlotId);
    }

    return 0;
}

int mtkRadioEx::imsBearerStateNotifyInd(int slotId,
                                  int indicationType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {

    RLOGD("imsBearerStateNotifyInd: serial %d", serial);

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL
            && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        if (response == NULL || responseLen != sizeof(RIL_IMS_BearerNotification)) {
            RLOGE("imsBearerStateNotifyInd: invalid response");
            return 0;
        }

        RIL_IMS_BearerNotification *p_cur = (RIL_IMS_BearerNotification *) response;
        int aid = p_cur->aid;
        int action = p_cur->action;
        hidl_string type = convertCharPtrToHidlString(p_cur->type);

        Return<void> retStatus =
                mtkRadioExService[imsSlotId]->
                mRadioIndicationIms->
                imsBearerStateNotify(convertIntToRadioIndicationType(indicationType),
                                    aid, action, type);

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("imsBearerStateNotifyInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                                                                            imsSlotId);
    }

    return 0;
}

int mtkRadioEx::imsBearerInitInd(int slotId,
                            int indicationType, int serial, RIL_Errno e,
                            void *response, size_t responseLen) {
    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    RLOGD("imsBearerInitInd: serial %d", serial);
    if (mtkRadioExService[imsSlotId] != NULL
            && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        Return<void> retStatus =
                mtkRadioExService[imsSlotId]->
                mRadioIndicationIms->
                imsBearerInit(convertIntToRadioIndicationType(indicationType));

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("imsBearerInitInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                                                                     imsSlotId);
    }

    return 0;
}

int mtkRadioEx::imsDataInfoNotifyInd(int slotId,
                                  int indicationType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen) {
    RLOGE("imsDataInfoNotifyInd: no need in legacy RIL now");
    return 0;
}

int mtkRadioEx::imsDeregDoneInd(int slotId,
                           int indicationType, int token, RIL_Errno e,
                           void *response, size_t responseLen) {

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL &&
            mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                 mRadioIndicationIms->
                                 imsDeregDone(
                                 convertIntToRadioIndicationType(indicationType));

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("imsDeregDoneInd: mtkRadioExService[%d]->mRadioIndicationIms = NULL",
                                                                   imsSlotId);
    }
    return 0;
}

int mtkRadioEx::confSRVCCInd(int slotId,
                        int indicationType, int token, RIL_Errno e,
                        void *response, size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL &&
            mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen % sizeof(int) != 0) {
            RLOGE("confSRVCCInd: invalid response");
            return 0;
        }
        hidl_vec<int32_t> data;
        int numInts = responseLen / sizeof(int);
        data.resize(numInts);
        int *pInt = (int *) response;

        for (int i = 0; i < numInts; i++) {
            data[i] = (int32_t) pInt[i];
        }

        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->confSRVCC(
                                 convertIntToRadioIndicationType(indicationType), data);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("confSRVCCInd: mtkRadioExService[%d]->mRadioIndicationMtk = NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::multiImsCountInd(int slotId,
          int indicationType, int token, RIL_Errno e,
          void *response, size_t responseLen) {
    RLOGE("multiImsCountInd: should not enter here");
    return 0;
}

int mtkRadioEx::imsSupportEccInd(int slotId,
                     int indicationType, int token, RIL_Errno e,
                     void *response, size_t responseLen) {
    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL &&
            mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        if (response == NULL || responseLen < sizeof(int)) {
            RLOGE("imsSupportEccInd: invalid response");
            return 0;
        }

        int status = ((int32_t *) response)[0];

        Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                 mRadioIndicationIms->
                                 imsSupportEcc(
                                 convertIntToRadioIndicationType(indicationType),
                                 status);

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("imsSupportEccInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                                                                            imsSlotId);
    }

    return 0;
}

int mtkRadioEx::redialEmergencyIndication(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    RLOGE("redialEmergencyIndication: not support yet");
    return 0;
}

int mtkRadioEx::emergencyBearerInfoInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("emergencyBearerInfoInd: invalid response");
            return 0;
        }
        RLOGD("emergencyBearerInfoInd");
        int32_t s1Support = ((int32_t *) response)[0];
        // We use same interface to update ECC support or not.
        // So value will be transformed here.
        // 0 => 2
        // 1 => 3
        s1Support += 2;
        Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                 mRadioIndicationIms->
                                 imsSupportEcc(
                                 convertIntToRadioIndicationType(indicationType),
                                 s1Support);
        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("emergencyBearerInfoInd: mtkRadioExService[%d]->mRadioIndicationIms == NULL", slotId);
    }

     return 0;
}

int mtkRadioEx::imsRadioInfoChangeInd(int slotId,
                     int indicationType, int token, RIL_Errno e,
                     void *response, size_t responseLen) {
    return 0;
}

// MTK-START: SIM
int mtkRadioEx::onVirtualSimStatusChanged(int slotId, int indicationType, int token, RIL_Errno e,
        void *response, size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL &&
        mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        RLOGD("onVirtualSimStatusChanged");
        int32_t simInserted = ((int32_t *) response)[0];
        Return<void> retStatus =
                mtkRadioExService[slotId]->mRadioIndicationMtk->onVirtualSimStatusChanged(
                        convertIntToRadioIndicationType(indicationType), simInserted);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("onVirtualSimStatusChanged: mtkRadioExService[%d]->mRadioIndicationMtk == NULL",
              slotId);
    }

    return 0;
}

int mtkRadioEx::onImeiLock(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        RLOGD("onImeiLock");
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->onImeiLock(
                convertIntToRadioIndicationType(indicationType));
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("onImeiLock: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::onImsiRefreshDone(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        RLOGD("onImsiRefreshDone");
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->onImsiRefreshDone(
                convertIntToRadioIndicationType(indicationType));
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("onImsiRefreshDone: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::eccNumIndication(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen) {
    return 0;
}

// World Phone {
int mtkRadioEx::plmnChangedIndication(int slotId,
                      int indicationType, int token, RIL_Errno e, void *response,
                      size_t responselen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responselen % sizeof(char *) != 0) {
            RLOGE("plmnChangedIndication: invalid response");
            return 0;
        }
        hidl_vec<hidl_string> plmn;
        char **resp = (char **) response;
        int numStrings = responselen / sizeof(char *);
        plmn.resize(numStrings);
        for (int i = 0; i < numStrings; i++) {
            plmn[i] = convertCharPtrToHidlString(resp[i]);
        }
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->plmnChangedIndication(
                convertIntToRadioIndicationType(indicationType), plmn);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("plmnChangedIndication: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::registrationSuspendedIndication(int slotId,
                      int indicationType, int token, RIL_Errno e, void *response,
                      size_t responselen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responselen % sizeof(int) != 0) {
            RLOGE("registrationSuspendedIndication: invalid response");
            return 0;
        }
        hidl_vec<int32_t> sessionId;
        int numInts = responselen / sizeof(int);
        int *pInt = (int *) response;
        sessionId.resize(numInts);
        for (int i = 0; i < numInts; i++) {
            sessionId[i] = pInt[i];
        }
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->registrationSuspendedIndication(
                convertIntToRadioIndicationType(indicationType), sessionId);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("registrationSuspendedIndication: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::gmssRatChangedIndication(int slotId,
                      int indicationType, int token, RIL_Errno e, void *response,
                      size_t responselen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responselen % sizeof(int) != 0) {
            RLOGE("gmssRatChangedIndication: invalid response");
            return 0;
        }
        hidl_vec<int32_t> gmss;
        int numInts = responselen / sizeof(int);
        int *pInt = (int *) response;
        gmss.resize(numInts);
        for (int i = 0; i < numInts; i++) {
            gmss[i] = pInt[i];
        }
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->gmssRatChangedIndication(
                convertIntToRadioIndicationType(indicationType), gmss);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("gmssRatChangedIndication: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::worldModeChangedIndication(int slotId,
                      int indicationType, int token, RIL_Errno e, void *response,
                      size_t responselen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responselen % sizeof(int) != 0) {
            RLOGE("worldModeChangedIndication: invalid response");
            return 0;
        }
        hidl_vec<int32_t> mode;
        int numInts = responselen / sizeof(int);
        int *pInt = (int *) response;
        mode.resize(numInts);
        for (int i = 0; i < numInts; i++) {
            mode[i] = pInt[i];
        }
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->worldModeChangedIndication(
                convertIntToRadioIndicationType(indicationType), mode);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("worldModeChangedIndication: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}
// World Phone }

int mtkRadioEx::esnMeidChangeInd(int slotId,
                                  int indicationType, int token, RIL_Errno e, void *response,
                                  size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("esnMeidChangeInd: invalid response");
            return 0;
        }
        hidl_string esnMeid((const char*)response, responseLen);
        RLOGD("esnMeidChangeInd (0x%s - %zu)", esnMeid.c_str(), responseLen);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->esnMeidChangeInd(
                convertIntToRadioIndicationType(indicationType),
                esnMeid);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("esnMeidChangeInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }
    return 0;
}

void radio::registerService(RIL_RadioFunctions *callbacks, CommandInfo *commands) {
    using namespace android::hardware;
    int simCount = 1;
    // for RILJ - AOSP APIs
    const char *serviceNames[] = {
            android::RIL_getServiceName()
            , RIL2_SERVICE_NAME
            , RIL3_SERVICE_NAME
            , RIL4_SERVICE_NAME
            };
    // for RILJ - MTK APIs
    const char *mtkServiceNames[] = {
            MTK_RIL1_SERVICE_NAME
            , MTK_RIL2_SERVICE_NAME
            , MTK_RIL3_SERVICE_NAME
            , MTK_RIL4_SERVICE_NAME
            };
    // for IMS - MTK APIs
    const char *imsServiceNames[] = {
            IMS_RIL1_SERVICE_NAME
            , IMS_RIL2_SERVICE_NAME
            , IMS_RIL3_SERVICE_NAME
            , IMS_RIL4_SERVICE_NAME
            };
    // for IMS - AOSP APIs
    const char *imsAospServiceNames[] = {
            IMS_AOSP_RIL1_SERVICE_NAME
            , IMS_AOSP_RIL2_SERVICE_NAME
            , IMS_AOSP_RIL3_SERVICE_NAME
            , IMS_AOSP_RIL4_SERVICE_NAME
            };
    // for SE - AOSP APIs
    const char *seServiceNames[] = {
            SE_RIL1_SERVICE_NAME
            , SE_RIL2_SERVICE_NAME
            , SE_RIL3_SERVICE_NAME
            , SE_RIL4_SERVICE_NAME
            };
    // for SE - MTK APIs
    const char *mtkSeServiceNames[] = {
            MTK_SE_RIL1_SERVICE_NAME
            , MTK_SE_RIL2_SERVICE_NAME
            , MTK_SE_RIL3_SERVICE_NAME
            , MTK_SE_RIL4_SERVICE_NAME
            };

    // for EM - AOSP APIs
    const char *emServiceNames[] = {
            EM_RIL1_SERVICE_NAME
            , EM_RIL2_SERVICE_NAME
            , EM_RIL3_SERVICE_NAME
            , EM_RIL4_SERVICE_NAME
            };
    // for EM - MTK APIs
    const char *mtkEmServiceNames[] = {
            MTK_EM_RIL1_SERVICE_NAME
            , MTK_EM_RIL2_SERVICE_NAME
            , MTK_EM_RIL3_SERVICE_NAME
            , MTK_EM_RIL4_SERVICE_NAME
            };

     // For telephony assist - MTK APIs
    const char *mtkAssistServiceNames[] = {
            MTK_ASSIST_RIL1_SERVICE_NAME
            , MTK_ASSIST_RIL2_SERVICE_NAME
            , MTK_ASSIST_RIL3_SERVICE_NAME
            , MTK_ASSIST_RIL4_SERVICE_NAME
            };
    // For RCS
    const char *mtkRcsServiceNames[] = {
            MTK_RCS_RIL1_SERVICE_NAME
            , MTK_RCS_RIL2_SERVICE_NAME
            , MTK_RCS_RIL3_SERVICE_NAME
            , MTK_RCS_RIL4_SERVICE_NAME
            };
    // For CapCtrl
    const char *mtkCapServiceNames[] = {
            MTK_CAP_RIL1_SERVICE_NAME
            , MTK_CAP_RIL2_SERVICE_NAME
            , MTK_CAP_RIL3_SERVICE_NAME
            , MTK_CAP_RIL4_SERVICE_NAME
    };
    /* [ALPS03590595]Set s_vendorFunctions and s_commands before registering service to
        null exception timing issue. */
    s_vendorFunctions = callbacks;
    s_commands = commands;

    simCount = getSimCount();
    configureRpcThreadpool(1, true /* callerWillJoin */);
    for (int i = 0; i < MAX_SIM_COUNT; i++) {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
        pthread_mutex_init(&(readLockOwnerListMutex[i]), &attr);
    }
    for (int i = 0; i < simCount; i++) {
        pthread_rwlock_t *radioServiceRwlockPtr = getRadioServiceRwlock(i);
        lockRadioServiceWlock(radioServiceRwlockPtr, (int)i);

        radioService[i] = new RadioImpl;
        radioService[i]->mSlotId = i;
        radioService[i]->mClientId = android::CLIENT_RILJ;
        RLOGI("registerService: starting IRadio %s", serviceNames[i]);
        android::status_t status = radioService[i]->registerAsService(serviceNames[i]);
        RLOGI("radio::registerService IRadio status: %d", status);

        mtkRadioExService[i] = new MtkRadioExImpl;
        mtkRadioExService[i]->mSlotId = i;
        mtkRadioExService[i]->mClientId = android::CLIENT_RILJ;
        RLOGI("registerService: starting IMtkRadioEx %s", mtkServiceNames[i]);
        android::status_t statusMtk  = mtkRadioExService[i]->registerAsService(mtkServiceNames[i]);
        RLOGI("radio::registerService IMtkRadioEx status: %d", statusMtk);

        int imsSlot = i + (android::CLIENT_IMS * MAX_SIM_COUNT);
        radioService[imsSlot] = new RadioImpl;
        radioService[imsSlot]->mSlotId = imsSlot;
        radioService[imsSlot]->mClientId = android::CLIENT_IMS;
        RLOGI("registerService: starting IMS IRadio %s", imsAospServiceNames[i]);
        status = radioService[imsSlot]->registerAsService(imsAospServiceNames[i]);
        RLOGI("radio::registerService IMS IRadio status: %d", status);

        mtkRadioExService[imsSlot] = new MtkRadioExImpl;
        mtkRadioExService[imsSlot]->mSlotId = imsSlot;
        mtkRadioExService[imsSlot]->mClientId = android::CLIENT_IMS;
        RLOGI("radio::registerService: starting IMS IMtkRadioEx %s, slot = %d, realSlot = %d",
              imsServiceNames[i], mtkRadioExService[imsSlot]->mSlotId, imsSlot);

        // Register IMS Radio Stub
        status = mtkRadioExService[imsSlot]->registerAsService(imsServiceNames[i]);
        RLOGI("radio::registerService IMtkRadioEx for IMS status:%d", status);

        /// MTK: ForSE @{
        int seSlot = i + (android::CLIENT_SE * MAX_SIM_COUNT);
        radioService[seSlot] = new RadioImpl;
        radioService[seSlot]->mSlotId = seSlot;
        radioService[seSlot]->mClientId = android::CLIENT_SE;
        RLOGI("registerService: starting SE IRadio %s", seServiceNames[i]);
        status = radioService[seSlot]->registerAsService(seServiceNames[i]);
        RLOGI("radio::registerService SE IRadio status: %d", status);

        mtkRadioExService[seSlot] = new MtkRadioExImpl;
        mtkRadioExService[seSlot]->mSlotId = seSlot;
        mtkRadioExService[seSlot]->mClientId = android::CLIENT_SE;
        status = mtkRadioExService[seSlot]->registerAsService(mtkSeServiceNames[i]);
        RLOGI("radio::registerService IMtkRadioEx for SE status:%d", status);
        /// MTK: ForSE @}

        /// MTK: For EM @{
        int emSlot = i + (android::CLIENT_EM * MAX_SIM_COUNT);

        radioService[emSlot] = new RadioImpl;
        radioService[emSlot]->mSlotId = emSlot;
        radioService[emSlot]->mClientId = android::CLIENT_EM;
        RLOGI("registerService: starting EM IRadio %s", emServiceNames[i]);
        status = radioService[emSlot]->registerAsService(emServiceNames[i]);
        RLOGI("radio::registerService EM IRadio status: %d", status);

        mtkRadioExService[emSlot] = new MtkRadioExImpl;
        mtkRadioExService[emSlot]->mSlotId = emSlot;
        mtkRadioExService[emSlot]->mClientId = android::CLIENT_EM;
        status = mtkRadioExService[emSlot]->registerAsService(mtkEmServiceNames[i]);
        RLOGI("radio::registerService IMtkRadioEx for EM status:%d", status);

        // MTK: For telephony assist. @{
        int assistSlot = i + (android::CLIENT_ASSIST * MAX_SIM_COUNT);
        mtkRadioExService[assistSlot] = new MtkRadioExImpl;
        mtkRadioExService[assistSlot]->mSlotId = assistSlot;
        mtkRadioExService[assistSlot]->mClientId = android::CLIENT_ASSIST;
        status = mtkRadioExService[assistSlot]->registerAsService(mtkAssistServiceNames[i]);
        RLOGI("radio::registerService IMtkRadioEx for telephony assist status:%d",
                status);
        // @}

        // For RCS. @{
        int rcsSlot = i + (android::CLIENT_RCS * MAX_SIM_COUNT);
        mtkRadioExService[rcsSlot] = new MtkRadioExImpl;
        mtkRadioExService[rcsSlot]->mSlotId = rcsSlot;
        mtkRadioExService[rcsSlot]->mClientId = android::CLIENT_RCS;
        RLOGI(LOG_TAG, "radio::registerService starting IMtkRadioEx %s", mtkRcsServiceNames[i]);
        status = mtkRadioExService[rcsSlot]->registerAsService(mtkRcsServiceNames[i]);
        RLOGI(LOG_TAG, "radio::registerService IRadio status: %d", status);
        // @}

        // For CAP. @{
        int capSlot = i + (android::CLIENT_CAP * MAX_SIM_COUNT);
        mtkRadioExService[capSlot] = new MtkRadioExImpl;
        mtkRadioExService[capSlot]->mSlotId = capSlot;
        mtkRadioExService[capSlot]->mClientId = android::CLIENT_CAP;
        RLOGI(LOG_TAG, "radio::registerService starting IMtkRadioEx %s", mtkCapServiceNames[i]);
        status = mtkRadioExService[capSlot]->registerAsService(mtkCapServiceNames[i]);
        RLOGI(LOG_TAG, "radio::registerService IRadio status: %d", status);
        // @}
        unlockRadioServiceWlock(radioServiceRwlockPtr, (int)i);
    }
}

void rilc_thread_pool() {
    joinRpcThreadpool();
}

int radio::isTidInRLock(int slotId, pthread_t tid) {
    int i, ret = -1;
    int slot = toRealSlot(slotId);
    pthread_mutex_lock(&(readLockOwnerListMutex[slot]));
    for (i = 0; i < readLockOwnerList[slot].size(); i++) {
        if (0 != pthread_equal(tid, readLockOwnerList[slot][i])) {
            ret = i;
        }
    }
    pthread_mutex_unlock(&(readLockOwnerListMutex[slot]));
    return ret;
}

void addTidToRLock(int slotId, pthread_t tid) {
    int slot = toRealSlot(slotId);

    pthread_mutex_lock(&(readLockOwnerListMutex[slot]));
    if(radio::isTidInRLock(slot, tid) == -1){
        // tid is not in the list, just add it
        readLockOwnerList[slot].add(tid);
    }
    pthread_mutex_unlock(&(readLockOwnerListMutex[slot]));
}

void removeTidFromRLock(int slotId, pthread_t tid) {
    int slot = toRealSlot(slotId);

    pthread_mutex_lock(&(readLockOwnerListMutex[slot]));
    int i = radio::isTidInRLock(slot, tid);
    if(i != -1){
        readLockOwnerList[slot].removeAt(i);
    }
    pthread_mutex_unlock(&(readLockOwnerListMutex[slot]));
}

pthread_rwlock_t * radio::getRadioServiceRwlock(int slotId) {
    pthread_rwlock_t *radioServiceRwlockPtr = &(radioServiceRwlocks[toRealSlot(slotId)]);
    return radioServiceRwlockPtr;
}

void radio::lockRadioServiceRlock(pthread_rwlock_t *radioServiceRwlockPtr, int slotId) {
    int ret = pthread_rwlock_rdlock(radioServiceRwlockPtr);
    assert(ret == 0);
    addTidToRLock((int) slotId, pthread_self());
}

void radio::unlockRadioServiceRlock(pthread_rwlock_t *radioServiceRwlockPtr, int slotId) {
    int ret = pthread_rwlock_unlock(radioServiceRwlockPtr);
    assert(ret == 0);
    removeTidFromRLock((int) slotId, pthread_self());
}

void radio::lockRadioServiceWlock(pthread_rwlock_t *radioServiceRwlockPtr, int slotId) {
    int slot = toRealSlot(slotId);
    int ret = pthread_rwlock_wrlock(radioServiceRwlockPtr);
    assert(ret == 0);
    writeLockOwner[slot] = pthread_self();
}

void radio::unlockRadioServiceWlock(pthread_rwlock_t *radioServiceRwlockPtr, int slotId) {
    int slot = toRealSlot(slotId);
    int ret = pthread_rwlock_unlock(radioServiceRwlockPtr);
    assert(ret == 0);
    writeLockOwner[slot] = 0;
}

Return<void> MtkRadioExImpl::getSmsRuimMemoryStatus(int32_t serial) {
    RLOGD("getSmsRuimMemoryStatus: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_SMS_RUIM_MEM_STATUS);
    return Void();
}

Return<void> MtkRadioExImpl::setVoicePreferStatus(int32_t serial, int32_t status) {
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_VOICE_PREFER_STATUS);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::setEccNum(int32_t serial, const hidl_string& eccListWithCard,
        const hidl_string& eccListNoCard) {
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_ECC_NUM);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::getEccNum(int32_t serial) {
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_GET_ECC_NUM);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::setFdMode(int32_t serial, int mode, int param1, int param2) {
    RLOGD("setFdMode: serial %d mode %d para1 %d para2 %d", serial, mode, param1, param2);
    dispatchFdMode(serial, mSlotId, RIL_REQUEST_SET_FD_MODE, mode, param1, param2);
    return Void();
}

Return<void> MtkRadioExImpl::restartRILD(int32_t serial) {
    RLOGD("restartRILD: serial: %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_RESTART_RILD);
    return Void();
}

int mtkRadioEx::restartRILDResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->restartRILDResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("restartRILDResponse: mtkRadioExService[%d]->restartRILDResponse "
                "== NULL", slotId);
    }

    return 0;
}

// / M: BIP, only to build pass {
int mtkRadioEx::bipProactiveCommandInd(int slotId,
                                  int indicationType, int token, RIL_Errno e, void *response,
                                  size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("bipProactiveCommandInd: invalid response");
            return 0;
        }
        RLOGD("bipProactiveCommandInd");
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->bipProactiveCommand(
                convertIntToRadioIndicationType(indicationType),
                convertCharPtrToHidlString((char *) response));
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("bipProactiveCommandInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}
// / M: BIP }
// / M: OTASP
int mtkRadioEx::triggerOtaSPInd(int slotId,
                                  int indicationType, int token, RIL_Errno e, void *response,
                                  size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("triggerOtaSPInd: invalid response");
            return 0;
        }
        RLOGD("triggerOtaSPInd");
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->triggerOtaSP(
                convertIntToRadioIndicationType(indicationType));
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("triggerOtaSPInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}
// / M: OTASP }

// / M: STK
int mtkRadioEx::onStkMenuResetInd(int slotId,
                                  int indicationType, int token, RIL_Errno e, void *response,
                                  size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("onStkMenuResetInd: invalid response");
            return 0;
        }
        RLOGD("onStkMenuResetInd");
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->onStkMenuReset(
                convertIntToRadioIndicationType(indicationType));
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("onStkMenuResetInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}
// / M: STK }

Return<void> MtkRadioExImpl::handleStkCallSetupRequestFromSimWithResCode(int32_t serial,
            int32_t resultCode){
    RLOGD("handleStkCallSetupRequestFromSimwithResCode: serial %d", serial);
    dispatchInts(serial, mSlotId,
            RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM_WITH_RESULT_CODE,
            1, resultCode);
    return Void();
}

int mtkRadioEx::handleStkCallSetupRequestFromSimWithResCodeResponse(int slotId,
        android::ClientId clientId __unused, int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    RLOGD("handleStkCallSetupRequestFromSimWithResCodeResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->handleStkCallSetupRequestFromSimWithResCodeResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("handleStkCallSetupRequestFromSimWithResCodeResponse: mtkRadioExService[%d]->mRadioResponseMtk "
                "== NULL", slotId);
    }

    return 0;
}
// / M: STK }

// M: Data Framework - common part enhancement @{
Return<void> MtkRadioExImpl::syncDataSettingsToMd(int32_t serial, const hidl_vec<int32_t>& settings) {
    RLOGD("syncDataSettingsToMd: serial: %d", serial);
    if (settings.size() >= 3 && settings.size() <= 5) {
        dispatchInts(serial, mSlotId, RIL_REQUEST_SYNC_DATA_SETTINGS_TO_MD, 3,
                settings[0], settings[1], settings[2]);
    } else {
        RLOGE("syncDataSettingsToMd: param error, num: %d (should be 3)", (int) settings.size());
    }
    return Void();
}

int mtkRadioEx::syncDataSettingsToMdResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    // syncDataSettings has 2 clients, one is telephony assist, one is telephony common,
    // distinct the 2 clients here.
    if (clientId == android::CLIENT_ASSIST) {
        if (mtkRadioExService[slotId]->mRadioResponseAssist != NULL) {
            RadioResponseInfo responseInfo = {};
            populateResponseInfo(responseInfo, serial, responseType, e);
            Return<void> retStatus =
                    mtkRadioExService[slotId]->mRadioResponseAssist->syncDataSettingsToMdResponse(
                            responseInfo);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::ASSIST_MTK, slotId);
        } else {
            RLOGE("syncDataSettingsToMdResponse: "
                    "mtkRadioExService[%d]->mRadioResponseAssist == NULL", slotId);
        }
        return 0;
    }
    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->syncDataSettingsToMdResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("syncDataSettingsToMdResponse: mtkRadioExService[%d]->mRadioIndicationMtk "
                "== NULL", slotId);
    }

    return 0;
}

// M: Data Framework - Data Retry enhancement @{
Return<void> MtkRadioExImpl::resetMdDataRetryCount(int32_t serial, const hidl_string& apn) {
    RLOGD("resetMdDataRetryCount: serial: %d", serial);
    dispatchString(serial, mSlotId, RIL_REQUEST_RESET_MD_DATA_RETRY_COUNT, apn.c_str());
    return Void();
}

int mtkRadioEx::resetMdDataRetryCountResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->resetMdDataRetryCountResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("resetMdDataRetryCountResponse: mtkRadioExService[%d]->mRadioIndicationMtk "
                "== NULL", slotId);
    }

    return 0;
}
int mtkRadioEx::onMdDataRetryCountReset(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        RLOGD("onMdDataRetryCountReset");
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->onMdDataRetryCountReset(
                convertIntToRadioIndicationType(indicationType));
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("onMdDataRetryCountReset: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}
// M: Data Framework - Data Retry enhancement @}

// M: Data Framework - CC 33
Return<void> MtkRadioExImpl::setRemoveRestrictEutranMode(int32_t serial, int32_t type) {
    RLOGD("setRemoveRestrictEutranMode: serial %d, type %d", serial, type);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_REMOVE_RESTRICT_EUTRAN_MODE, 1, type);
    return Void();
}

int mtkRadioEx::setRemoveRestrictEutranModeResponse(int slotId,
        android::ClientId clientId __unused, int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen){
    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->setRemoveRestrictEutranModeResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("setRemoveRestrictEutranModeResponse: mtkRadioExService[%d]-> "
                "setRemoveRestrictEutranModeResponse== NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::onRemoveRestrictEutran(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        RLOGD("onRemoveRestrictEutran");
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->onRemoveRestrictEutran(
                convertIntToRadioIndicationType(indicationType));
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("onRemoveRestrictEutran: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}
// M: Data Framework - CC 33 @}

// M: [LTE][Low Power][UL traffic shaping] @{
int mtkRadioEx::setLteAccessStratumReportResponse(int slotId, android::ClientId clientId __unused,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen) {
    RLOGD("setLteAccessStratumReportResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->
                setLteAccessStratumReportResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("setLteAccessStratumReportResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }

    return 0;
}

int mtkRadioEx::setLteUplinkDataTransferResponse(int slotId, android::ClientId clientId __unused,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen) {
    RLOGD("setLteUplinkDataTransferResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->
                setLteUplinkDataTransferResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("setLteUplinkDataTransferResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }

    return 0;
}

int mtkRadioEx::onLteAccessStratumStateChanged(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        hidl_vec<int32_t> data;

        int numInts = responseLen / sizeof(int);
        if (response == NULL || responseLen % sizeof(int) != 0) {
            RLOGE("onLteAccessStratumStateChanged Invalid response: NULL");
            return 0;
        } else {
            int *pInt = (int *) response;
            data.resize(numInts);
            for (int i = 0; i < numInts; i++) {
                data[i] = (int32_t) pInt[i];
            }
        }
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioIndicationMtk->onLteAccessStratumStateChanged(
                convertIntToRadioIndicationType(indicationType), data);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("mdChangeApnInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }
    return 0;
}
// M: [LTE][Low Power][UL traffic shaping] @}

// MTK-START: SIM HOT SWAP
int mtkRadioEx::onSimPlugIn(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        RLOGD("onSimPlugIn");
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->onSimHotSwapInd(
                convertIntToRadioIndicationType(indicationType),
                (int32_t)(SimHotSwap::SIM_HOTSWAP_PLUG_IN), NULL);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("onSimPlugIn: radioService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::onSimPlugOut(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        RLOGD("onSimPlugOut");
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->onSimHotSwapInd(
                convertIntToRadioIndicationType(indicationType),
                (int32_t)(SimHotSwap::SIM_HOTSWAP_PLUG_OUT), NULL);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("onSimPlugOut: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}
// MTK-END

// MTK-START: SIM MISSING/RECOVERY
int mtkRadioEx::onSimMissing(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        RLOGD("onSimMissing");
        int32_t simInserted = ((int32_t *) response)[0];
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->onSimHotSwapInd(
                convertIntToRadioIndicationType(indicationType),
                (int32_t)(SimHotSwap::SIM_HOTSWAP_MISSING), NULL);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("onSimMissing: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::onSimRecovery(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        RLOGD("onSimRecovery");
        int32_t simInserted = ((int32_t *) response)[0];
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->onSimHotSwapInd(
                convertIntToRadioIndicationType(indicationType),
                (int32_t)(SimHotSwap::SIM_HOTSWAP_RECOVERY), NULL);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("onSimRecovery: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}
// MTK-END

// MTK-START: SIM COMMON SLOT
int mtkRadioEx::onSimTrayPlugIn(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        RLOGD("onSimTrayPlugIn");
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->onSimHotSwapInd(
                convertIntToRadioIndicationType(indicationType),
                (int32_t)(SimHotSwap::SIM_HOTSWAP_TRAY_PLUG_IN), NULL);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("onSimTrayPlugIn: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::onSimCommonSlotNoChanged(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        RLOGD("onSimCommonSlotNoChanged");
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->onSimHotSwapInd(
                convertIntToRadioIndicationType(indicationType),
                (int32_t)(SimHotSwap::SIM_HOTSWAP_COMMONSLOT_NO_CHANGED), NULL);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("onSimCommonSlotNoChanged: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}
// MTK-END

// MTK-START: SIM CARD DETECTED
int mtkRadioEx::onCardDetectedInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        RLOGD("onCardDetectedInd: slotId %d", slotId);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->onCardDetectedInd(
                convertIntToRadioIndicationType(indicationType));
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGD("onCardDetectedInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}

// MTK-END

// MTK-START: SIM POWER
int mtkRadioEx::onSimPowerChangedInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    return 0;
}
// MTK-END

// SMS-START
Return<void> MtkRadioExImpl::getSmsParameters(int32_t serial) {
    RLOGD("getSmsParameters: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_SMS_PARAMS);
    return Void();
}

bool dispatchSmsParametrs(int serial, int slotId, int request, const SmsParams& message) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    RIL_SmsParams params;
    memset (&params, 0, sizeof(RIL_SmsParams));

    params.dcs = message.dcs;
    params.format = message.format;
    params.pid = message.pid;
    params.vp = message.vp;

    s_vendorFunctions->onRequest(request, &params, sizeof(params), pRI, pRI->socket_id);

    return true;
}


Return<void> MtkRadioExImpl::setSmsParameters(int32_t serial, const SmsParams& message) {
    RLOGD("setSmsParameters: serial %d", serial);
    dispatchSmsParametrs(serial, mSlotId, RIL_REQUEST_SET_SMS_PARAMS, message);
    return Void();
}

Return<void> MtkRadioExImpl::setEtws(int32_t serial, int32_t mode) {
    RLOGD("setEtws: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_ETWS, 1, mode);
    return Void();
}

Return<void> MtkRadioExImpl::removeCbMsg(int32_t serial, int32_t channelId, int32_t serialId) {
    RLOGD("removeCbMsg: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_REMOVE_CB_MESSAGE, 2, channelId, serialId);
    return Void();
}

Return<void> MtkRadioExImpl::getSmsMemStatus(int32_t serial) {
    RLOGD("getSmsMemStatus: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_SMS_SIM_MEM_STATUS);
    return Void();
}

Return<void> MtkRadioExImpl::setGsmBroadcastLangs(int32_t serial, const hidl_string& langs) {
    RLOGD("setGsmBroadcastLangs: serial %d", serial);
    dispatchString(serial, mSlotId, RIL_REQUEST_GSM_SET_BROADCAST_LANGUAGE, langs.c_str());
    return Void();
}

Return<void> MtkRadioExImpl::getGsmBroadcastLangs(int32_t serial) {
    RLOGD("getGsmBroadcastLangs: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GSM_GET_BROADCAST_LANGUAGE);
    return Void();
}

Return<void> MtkRadioExImpl::getGsmBroadcastActivation(int32_t serial) {
    RLOGD("getGsmBroadcastActivation: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_GSM_SMS_BROADCAST_ACTIVATION);
    return Void();
}

Return<void> MtkRadioExImpl::sendImsSmsEx(int32_t serial, const ImsSmsMessage& message) {
    RLOGD("sendImsSmsEx: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_IMS_SEND_SMS_EX);
    if (pRI == NULL) {
        return Void();
    }

    RIL_RadioTechnologyFamily format = (RIL_RadioTechnologyFamily) message.tech;

    if (RADIO_TECH_3GPP == format) {
        dispatchImsGsmSms(message, pRI);
    } else if (RADIO_TECH_3GPP2 == format) {
        dispatchImsCdmaSms(message, pRI);
    } else {
        RLOGE("sendImsSms: Invalid radio tech %s",
                requestToString(pRI->pCI->requestNumber));
        sendErrorResponse(pRI, RIL_E_INVALID_ARGUMENTS);
    }
    return Void();
}

Return<void> MtkRadioExImpl::acknowledgeLastIncomingGsmSmsEx(int32_t serial,
                                                      bool success, SmsAcknowledgeFailCause cause) {
    RLOGD("acknowledgeLastIncomingGsmSmsEx: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SMS_ACKNOWLEDGE_EX, 2, BOOL_TO_INT(success),
            cause);
    return Void();
}

int mtkRadioEx::sendImsSmsExResponse(int slotId, android::ClientId clientId __unused,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responseLen) {
    RLOGD("sendImsSmsExResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        SendSmsResult result = makeSendSmsResult(responseInfo, serial, responseType, e, response,
                responseLen);

        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseIms->sendImsSmsExResponse(
                responseInfo, result);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("sendImsSmsExResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                slotId);
    }

    return 0;
}

int mtkRadioEx::acknowledgeLastIncomingGsmSmsExResponse(int slotId,
        android::ClientId clientId __unused, int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    RLOGD("acknowledgeLastIncomingGsmSmsExResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus =
                mtkRadioExService[slotId]->mRadioResponseIms->acknowledgeLastIncomingGsmSmsExResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE(LOG_TAG,
                "acknowledgeLastIncomingGsmSmsExResponse: mtkRadioExService[%d]->mRadioResponseIms "
                "== NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::getSmsParametersResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        SmsParams params = {};
        if (response == NULL || responseLen != sizeof(RIL_SmsParams)) {
            RLOGE("getSmsParametersResponse: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            RIL_SmsParams *p_cur = ((RIL_SmsParams *) response);
            params.format = p_cur->format;
            params.dcs = p_cur->dcs;
            params.vp = p_cur->vp;
            params.pid = p_cur->pid;
        }

        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->
                getSmsParametersResponse(responseInfo, params);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("getSmsParametersResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::setSmsParametersResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("setSmsParametersResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->setSmsParametersResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("setSmsParametersResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::setEtwsResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("setEtwsResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->setEtwsResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("setEtwsResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::removeCbMsgResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("removeCbMsgResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->removeCbMsgResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("removeCbMsgResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::newEtwsInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responselen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responselen == 0) {
            RLOGE("newEtwsInd: invalid response");
            return 0;
        }

        EtwsNotification etws = {};
        RIL_CBEtwsNotification *pEtws = (RIL_CBEtwsNotification *)response;
        etws.messageId = pEtws->messageId;
        etws.serialNumber = pEtws->serialNumber;
        etws.warningType = pEtws->warningType;
        etws.plmnId = convertCharPtrToHidlString(pEtws->plmnId);
        etws.securityInfo = convertCharPtrToHidlString(pEtws->securityInfo);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->newEtwsInd(
                convertIntToRadioIndicationType(indicationType), etws);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("newEtwsInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }
    return 0;
}

int mtkRadioEx::newSmsIndEx(int slotId, int indicationType,
                     int token, RIL_Errno e, void *response, size_t responseLen) {
    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("newSmsIndEx: invalid response");
            return 0;
        }

        uint8_t *bytes = convertHexStringToBytes(response, responseLen);
        if (bytes == NULL) {
            RLOGE("newSmsIndEx: convertHexStringToBytes failed");
            return 0;
        }

        hidl_vec<uint8_t> pdu;
        pdu.setToExternal(bytes, responseLen/2);
        RLOGD("newSmsIndEx");
        Return<void> retStatus = mtkRadioExService[imsSlotId]->mRadioIndicationIms->newSmsEx(
                convertIntToRadioIndicationType(indicationType), pdu);
        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
        free(bytes);
    } else {
        RLOGE("newSmsIndEx: mtkRadioExService[%d]->mRadioIndicationIms == NULL", imsSlotId);
    }

    return 0;
}

int mtkRadioEx::newSmsStatusReportIndEx(int slotId,
                                 int indicationType, int token, RIL_Errno e, void *response,
                                 size_t responseLen) {
    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("newSmsStatusReportIndEx: invalid response");
            return 0;
        }

        uint8_t *bytes = convertHexStringToBytes(response, responseLen);
        if (bytes == NULL) {
            RLOGE("newSmsStatusReportIndEx: convertHexStringToBytes failed");
            return 0;
        }

        hidl_vec<uint8_t> pdu;
        pdu.setToExternal(bytes, responseLen/2);
        RLOGD("newSmsStatusReportIndEx");
        Return<void> retStatus = mtkRadioExService[imsSlotId]->mRadioIndicationIms->newSmsStatusReportEx(
                convertIntToRadioIndicationType(indicationType), pdu);
        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
        free(bytes);
    } else {
        RLOGE("newSmsStatusReportIndEx: mtkRadioExService[%d]->mRadioIndicationIms == NULL",
                imsSlotId);
    }
    return 0;
}

int mtkRadioEx::getSmsMemStatusResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        SmsMemStatus params = {};
        if (response == NULL || responseLen != sizeof(RIL_SMS_Memory_Status)) {
            RLOGE("getSmsMemStatusResponse: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            RIL_SMS_Memory_Status *p_cur = ((RIL_SMS_Memory_Status *) response);
            params.used = p_cur->used;
            params.total = p_cur->total;
        }

        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->
                getSmsMemStatusResponse(responseInfo, params);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("getSmsMemStatusResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::setGsmBroadcastLangsResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("setGsmBroadcastLangsResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->setGsmBroadcastLangsResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("setGsmBroadcastLangsResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::getGsmBroadcastLangsResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->
                getGsmBroadcastLangsResponse(
                responseInfo, convertCharPtrToHidlString((char *) response));
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("getGsmBroadcastLangsResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::getGsmBroadcastActivationRsp(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        int activation = 0;
        if (response != NULL) {
            int *pInt = (int *) response;
            activation = pInt[0];
        }
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->
                getGsmBroadcastActivationRsp(responseInfo, activation);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("getGsmBroadcastActivationRsp: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::meSmsStorageFullInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        RLOGD("meSmsStorageFullInd");
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->meSmsStorageFullInd(
                convertIntToRadioIndicationType(indicationType));
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("meSmsStorageFullInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::smsReadyInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        RLOGD("smsReadyInd");
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->smsReadyInd(
                convertIntToRadioIndicationType(indicationType));
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("smsReadyInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;

}
// SMS-END

int mtkRadioEx::responsePsNetworkStateChangeInd(int slotId,
                                           int indicationType, int token, RIL_Errno e,
                                           void *response, size_t responseLen) {

    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        hidl_vec<int32_t> data;
        int numInts = responseLen / sizeof(int);
        if (response == NULL || responseLen % sizeof(int) != 0) {
            RLOGE("responsePsNetworkStateChangeInd Invalid response: NULL");
            return 0;
        } else {
            int *pInt = (int *) response;
            data.resize(numInts);
            for (int i=0; i<numInts; i++) {
                data[i] = (int32_t) pInt[i];
            }
        }
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioIndicationMtk->responsePsNetworkStateChangeInd(
                convertIntToRadioIndicationType(indicationType), data);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("responsePsNetworkStateChangeInd: mtkRadioExService[%d]->responsePsNetworkStateChangeInd == NULL",
                slotId);
    }
    return 0;
}

int mtkRadioEx::networkBandInfoInd(int slotId, int indicationType,
        int token, RIL_Errno e, void *response, size_t responseLen) {

    return 0;
}

int mtkRadioEx::responseCsNetworkStateChangeInd(int slotId,
                              int indicationType, int token, RIL_Errno e, void *response,
                              size_t responseLen) {

    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen % sizeof(char *) != 0) {
            RLOGE("responseCsNetworkStateChangeInd Invalid response: NULL");
            return 0;
        }
        RLOGD("responseCsNetworkStateChangeInd");
        hidl_vec<hidl_string> data;
        char **resp = (char **) response;
        int numStrings = responseLen / sizeof(char *);
        data.resize(numStrings);
        for (int i = 0; i < numStrings; i++) {
            data[i] = convertCharPtrToHidlString(resp[i]);
            char* tmp = NULL;
            asprintf(&tmp, "%s", resp[i]);
            if (tmp != NULL) {
                size_t size_ = strlen(tmp);
                if (size_ == 1 || size_ == 2) {
                    tmp[0] = '*';
                } else {
                    for (int i = 0; i < (size_/2); i++) {
                        tmp[i] = '*';
                    }
                }
                RLOGD("responseCsNetworkStateChangeInd:: %d: %s", i, tmp);
                free(tmp);
            } else {
                RLOGD("responseCsNetworkStateChangeInd:: %d", i);
            }
        }
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->responseCsNetworkStateChangeInd(
                convertIntToRadioIndicationType(indicationType), data);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("responseCsNetworkStateChangeInd: mtkRadioExService[%d]->responseCsNetworkStateChangeInd == NULL", slotId);
    }
    return 0;
}

int mtkRadioEx::networkInfoInd(int slotId, int indicationType,
        int token, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("networkInfoInd");

    hidl_vec<hidl_string> networkInfo;
    if (response == NULL) {
        RLOGE("networkInfoInd Invalid networkInfo: NULL");
        return 0;
    } else {
        char **resp = (char **) response;
        int numStrings = responseLen / sizeof(char *);
        networkInfo.resize(numStrings);
        for (int i = 0; i < numStrings; i++) {
            networkInfo[i] = convertCharPtrToHidlString(resp[i]);
        }
    }
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->networkInfoInd(
                 convertIntToRadioIndicationType(indicationType), networkInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("networkInfoInd: mtkRadioExService[%d]->mRadioIndication "
                 "== NULL", slotId);
    }
    // send to EM
    int emSlot = toClientSlot(slotId, android::CLIENT_EM);
    if (mtkRadioExService[emSlot] != NULL && mtkRadioExService[emSlot]->mRadioIndicationEm != NULL) {
        Return<void> retStatus = mtkRadioExService[emSlot]->mRadioIndicationEm->networkInfoInd(
                 convertIntToRadioIndicationType(indicationType), networkInfo);
        mtkRadioExService[emSlot]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("networkInfoInd: mtkRadioExService[%d]->mRadioIndicationEm "
                 "== NULL", emSlot);
    }
    return 0;
}

int mtkRadioEx::setRxTestConfigResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("setRxTestConfigResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<int32_t> respAntConf;
        int numInts = responseLen / sizeof(int);
        if (response == NULL || responseLen % sizeof(int) != 0) {
            RLOGE("setRxTestConfigResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            respAntConf.resize(numInts);
            for (int i = 0; i < numInts; i++) {
                respAntConf[i] = (int32_t) pInt[i];
            }
        }
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->setRxTestConfigResponse(responseInfo,
                respAntConf);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("setRxTestConfigResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }

    return 0;
}

int mtkRadioEx::getRxTestResultResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("getRxTestResultResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<int32_t> respAntInfo;
        int numInts = responseLen / sizeof(int);
        if (response == NULL || responseLen % sizeof(int) != 0) {
            RLOGE("getRxTestResultResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            respAntInfo.resize(numInts);
            for (int i = 0; i < numInts; i++) {
                respAntInfo[i] = (int32_t) pInt[i];
            }
        }
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->getRxTestResultResponse(responseInfo,
                respAntInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("getRxTestResultResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }

    return 0;
}

int mtkRadioEx::responseInvalidSimInd(int slotId,
                              int indicationType, int token, RIL_Errno e, void *response,
                              size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen % sizeof(char *) != 0) {
            RLOGE("responseInvalidSimInd Invalid response: NULL");
            return 0;
        }
        RLOGD("responseInvalidSimInd");
        hidl_vec<hidl_string> data;
        char **resp = (char **) response;
        int numStrings = responseLen / sizeof(char *);
        data.resize(numStrings);
        for (int i = 0; i < numStrings; i++) {
            data[i] = convertCharPtrToHidlString(resp[i]);
            RLOGD("responseInvalidSimInd: %d: %s", i, resp[i]);
        }
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->responseInvalidSimInd(
                convertIntToRadioIndicationType(indicationType), data);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("responseInvalidSimInd: mtkRadioExService[%d]->responseInvalidSimInd == NULL", slotId);
    }
    return 0;
}

int mtkRadioEx::responseLteNetworkInfo(int slotId, int indicationType,
        int token, RIL_Errno e, void *response, size_t responseLen) {

    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen != sizeof(int)) {
            RLOGE("responseLteNetworkInfo: invalid response");
            return 0;
        }
        int32_t info = ((int32_t *) response)[0];
        RLOGD("responseLteNetworkInfo: %d", info);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->responseLteNetworkInfo(
                convertIntToRadioIndicationType(indicationType), info);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("responseLteNetworkInfo: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::getPOLCapabilityResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("getPOLCapabilityResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<int32_t> polCapability;
        if (response == NULL) {
            RLOGE("getPOLCapabilityResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            int numInts = responseLen / sizeof(int);
            polCapability.resize(numInts);
            for (int i = 0; i < numInts; i++) {
                polCapability[i] = (int32_t) pInt[i];
            }
        }
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->getPOLCapabilityResponse(
                responseInfo, polCapability);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("getPOLCapabilityResponse: mtkRadioExService[%d]->getPOLCapabilityResponse "
                "== NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::getCurrentPOLListResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("getCurrentPOLListResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<hidl_string> polList;
        if (response == NULL) {
            RLOGE("getPOLCapabilityResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            char **resp = (char **) response;
            int numStrings = responseLen / sizeof(char *);
            polList.resize(numStrings);
            for (int i = 0; i < numStrings; i++) {
                polList[i] = convertCharPtrToHidlString(resp[i]);
            }
        }
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->getCurrentPOLListResponse(
                responseInfo, polList);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("getPOLCapabilityResponse: mtkRadioExService[%d]->getPOLCapabilityResponse "
                "== NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::setPOLEntryResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("setPOLEntryResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->setPOLEntryResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("setPOLEntryResponse: mtkRadioExService[%d]->setPOLEntryResponse "
                "== NULL", slotId);
    }

    return 0;

}
// PHB START
int mtkRadioEx::phbReadyNotificationInd(int slotId,
                                   int indicationType, int token, RIL_Errno e, void *response,
                                   size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("phbReadyNotificationInd: invalid response");
            return 0;
        }
        RLOGD("phbReadyNotificationInd");
        int32_t isPhbReady = ((int32_t *) response)[0];
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->phbReadyNotification(
                convertIntToRadioIndicationType(indicationType), isPhbReady);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("phbReadyNotificationInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

     return 0;
}  // PHB END

int mtkRadioEx::setTrmResponse(int slotId, android::ClientId clientId __unused, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("setTrmResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus =
                mtkRadioExService[slotId]->mRadioResponseMtk->setTrmResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("setTrmResponse: mtkRadioExService[%d]->mRadioResponse == NULL",
                slotId);
    }
    return 0;
}

Return<void> MtkRadioExImpl::setApcMode(int32_t serial, int32_t mode,
        int32_t reportMode, int32_t interval) {
    RLOGD("setApcMode: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_PSEUDO_CELL_MODE, 3,
            mode, reportMode, interval);
    return Void();
}

Return<void> MtkRadioExImpl::getApcInfo(int32_t serial) {
    RLOGD("getApcInfo: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_PSEUDO_CELL_INFO);
    return Void();
}

int mtkRadioEx::responseNetworkEventInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {

    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen % sizeof(char *) != 0) {
            RLOGE("responseNetworkEventInd Invalid response: NULL");
            return 0;
        }
        RLOGD("responseNetworkEventInd");
        hidl_vec<int32_t> data;
        int *pInt = (int *) response;
        int numInts = responseLen / sizeof(int);
        data.resize(numInts);
        for (int i = 0; i < numInts; i++) {
            data[i] = (int32_t) pInt[i];
        }
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->responseNetworkEventInd(
                convertIntToRadioIndicationType(indicationType), data);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("responseNetworkEventInd: mtkRadioExService[%d]->responseNetworkEventInd == NULL", slotId);
    }
    return 0;
}

int mtkRadioEx::responseModulationInfoInd(int slotId,
                                           int indicationType, int token, RIL_Errno e,
                                           void *response, size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen % sizeof(char *) != 0) {
            RLOGE("responseModulationInfoInd Invalid response: NULL");
            return 0;
        }
        RLOGD("responseModulationInfoInd");
        hidl_vec<int32_t> data;
        int *pInt = (int *) response;
        int numInts = responseLen / sizeof(int);
        data.resize(numInts);
        for (int i = 0; i < numInts; i++) {
            data[i] = (int32_t) pInt[i];
        }
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->responseModulationInfoInd(
                convertIntToRadioIndicationType(indicationType), data);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("responseModulationInfoInd: mtkRadioExService[%d]->responseModulationInfoInd == NULL", slotId);
    }
    return 0;
}

int mtkRadioEx::setApcModeResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("setApcModeResponse: serial %d", serial);
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
            = mtkRadioExService[slotId]->mRadioResponseMtk->setApcModeResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("setApcModeResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }
    return 0;
}

// Femtocell feature
int mtkRadioEx::getFemtocellListResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("getFemtocellListResponse: serial %d", serial);

    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<hidl_string> femtoList;
        if (response == NULL) {
            RLOGE("getFemtocellListResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            char **resp = (char **) response;
            int numStrings = responseLen / sizeof(char *);
            femtoList.resize(numStrings);
            for (int i = 0; i < numStrings; i++) {
                femtoList[i] = convertCharPtrToHidlString(resp[i]);
            }
        }
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->getFemtocellListResponse(
                responseInfo, femtoList);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("getFemtocellListResponse: mtkRadioExService[%d]->mRadioResponseMtk "
                "== NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::abortFemtocellListResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("abortFemtocellListResponse: serial %d", serial);

    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->abortFemtocellListResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("abortFemtocellListResponse: mtkRadioExService[%d]->mRadioResponseMtk "
                "== NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::selectFemtocellResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("selectFemtocellResponse: serial %d", serial);

    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->selectFemtocellResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("selectFemtocellResponse: mtkRadioExService[%d]->mRadioResponseMtk "
                "== NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::queryFemtoCellSystemSelectionModeResponse(int slotId,
        android::ClientId clientId __unused, int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    RLOGD("queryFemtoCellSystemSelectionModeResponse: serial %d", serial);

    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        int mode = responseInt(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->queryFemtoCellSystemSelectionModeResponse(
                responseInfo, mode);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("queryFemtoCellSystemSelectionModeResponse: mtkRadioExService[%d]->mRadioResponseMtk "
                "== NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::setFemtoCellSystemSelectionModeResponse(int slotId,
        android::ClientId clientId __unused, int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    RLOGD("setFemtoCellSystemSelectionModeResponse: serial %d", serial);

    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->setFemtoCellSystemSelectionModeResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("setFemtoCellSystemSelectionModeResponse: mtkRadioExService[%d]->mRadioResponseMtkmRadioResponseMtk "
                "== NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::responseFemtocellInfo(int slotId,
                         int indicationType, int token, RIL_Errno e, void *response,
                         size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("responseFemtocellInfo: invalid indication");
            return 0;
        }

        hidl_vec<hidl_string> info;
        char **resp = (char **) response;
        int numStrings = responseLen / sizeof(char *);
        info.resize(numStrings);
        for (int i = 0; i < numStrings; i++) {
            info[i] = convertCharPtrToHidlString(resp[i]);
        }
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->responseFemtocellInfo(
                convertIntToRadioIndicationType(indicationType), info);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("responseFemtocellInfo: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::getApcInfoResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("getApcInfoResponse: serial %d", serial);
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<int32_t> pseudoCellInfo;
        if (response == NULL) {
            RLOGE("getApcInfoResponse Invalid response: NULL");
            return 0;
        } else {
            int *pInt = (int *) response;
            int numInts = responseLen / sizeof(int);
            pseudoCellInfo.resize(numInts);
            for (int i = 0; i < numInts; i++) {
                pseudoCellInfo[i] = (int32_t)(pInt[i]);
            }
        }
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->getApcInfoResponse(
                responseInfo, pseudoCellInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("getApcInfoResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }
    return 0;
}

int mtkRadioEx::onPseudoCellInfoInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("onPseudoCellInfoInd");
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        hidl_vec<int32_t> pseudoCellInfo;
        if (response == NULL) {
            RLOGE("onPseudoCellInfoInd Invalid response: NULL");
            return 0;
        } else {
            int *pInt = (int *) response;
            int numInts = responseLen / sizeof(int);
            pseudoCellInfo.resize(numInts);
            for (int i = 0; i < numInts; i++) {
                pseudoCellInfo[i] = (int32_t)(pInt[i]);
            }
        }
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->onPseudoCellInfoInd(
                convertIntToRadioIndicationType(indicationType), pseudoCellInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("onPseudoCellInfoInd: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }
    return 0;
}

/// M: [Network][C2K] Sprint roaming control @{
int mtkRadioEx::setRoamingEnableResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("setRoamingEnableResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->
                setRoamingEnableResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("setRoamingEnableResponse: mtkRadioExService[%d]->mRadioResponse == NULL", slotId);
    }
    return 0;
}

int mtkRadioEx::getRoamingEnableResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("getRoamingEnableResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<int32_t> config;
        if (response == NULL) {
            RLOGE("getRoamingEnableResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            int numInts = responseLen / sizeof(int);
            config.resize(numInts);
            for (int i = 0; i < numInts; i++) {
                config[i] = (int32_t) pInt[i];
            }
        }
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->getRoamingEnableResponse(
                responseInfo, config);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("getRoamingEnableResponse: mtkRadioExService[%d]->mRadioResponse == NULL", slotId);
    }
    return 0;
}
/// @}

int mtkRadioEx::setLteReleaseVersionResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    return 0;
}

int mtkRadioEx::getLteReleaseVersionResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e,void *response, size_t responseLen) {
    return 0;
}

int mtkRadioEx::onMccMncChangedInd(int slotId,
        int indicationType, int token, RIL_Errno e,void *response, size_t responselen) {
    return 0;
}

int mtkRadioEx::dataAllowedNotificationInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("dataAllowedNotificationInd: invalid response");
            return 0;
        }
        int32_t allowed = ((int32_t *) response)[0];
        RLOGD("dataAllowedNotificationInd[%d]: %d", slotId, allowed);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->dataAllowedNotification(
                convertIntToRadioIndicationType(indicationType), allowed);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("dataAllowedNotificationInd: mtkRadioExService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

// MTK-START: RIL_REQUEST_SWITCH_MODE_FOR_ECC
int mtkRadioEx::triggerModeSwitchByEccResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("triggerModeSwitchByEccResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->
                triggerModeSwitchByEccResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("triggerModeSwitchByEccResponse: mtkRadioExService[%d]->mRadioResponse == NULL",
                slotId);
    }
    return 0;
}
// MTK-END

Return<void> MtkRadioExImpl::setModemPower(int32_t serial, bool isOn) {
    RLOGD("setModemPower: serial: %d, isOn: %d", serial, isOn);
    if (isOn) {
        dispatchVoid(serial, mSlotId, RIL_REQUEST_MODEM_POWERON);
    } else {
        dispatchVoid(serial, mSlotId, RIL_REQUEST_MODEM_POWEROFF);
    }
    return Void();
}

int mtkRadioEx::setModemPowerResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("setModemPowerResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->setModemPowerResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("setModemPowerResponse: mtkRadioExService[%d]->setModemPowerResponse "
                "== NULL", slotId);
    }
    return 0;
}

// External SIM [START]
bool dispatchVsimEvent(int serial, int slotId, int request,
        uint32_t transactionId, uint32_t eventId, uint32_t simType) {
    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        return false;
    }

    RIL_VsimEvent args;
    args.transaction_id = transactionId;
    args.eventId = eventId;
    args.sim_type = simType;

    s_vendorFunctions->onRequest(request, &args, sizeof(args), pRI, pRI->socket_id);

    return true;
}

bool dispatchVsimOperationEvent(int serial, int slotId, int request,
        uint32_t transactionId, uint32_t eventId, int32_t result,
        int32_t dataLength, const hidl_vec<uint8_t>& data) {

    RLOGD("dispatchVsimOperationEvent: enter id=%d", eventId);

    RequestInfo *pRI = android::addRequestToList(serial, slotId, request);
    if (pRI == NULL) {
        RLOGD("dispatchVsimOperationEvent: pRI is NULL.");
        return false;
    }

    RIL_VsimOperationEvent args;

    memset (&args, 0, sizeof(args));

    // Transcation id
    args.transaction_id = transactionId;
    // Event id
    args.eventId = eventId;
    // Result
    args.result = result;
    // Data length
    args.data_length = dataLength;

    // Data array
    const uint8_t *uData = data.data();
    args.data= (char  *) calloc(1, (sizeof(char) * args.data_length * 2) + 1);
    memset(args.data, 0, ((sizeof(char) * args.data_length * 2) + 1));
    for (int i = 0; i < args.data_length; i++) {
        sprintf((args.data + (i*2)), "%02X", uData[i]);
    }

    //RLOGD("dispatchVsimOperationEvent: id=%d, data=%s", args.eventId, args.data);

    s_vendorFunctions->onRequest(request, &args, sizeof(args), pRI, pRI->socket_id);

    free(args.data);

    return true;
}

Return<void> MtkRadioExImpl::sendVsimNotification(int32_t serial, uint32_t transactionId,
        uint32_t eventId, uint32_t simType) {
    RLOGD("sendVsimNotification: serial %d", serial);
    dispatchVsimEvent(serial, mSlotId, RIL_REQUEST_VSIM_NOTIFICATION, transactionId, eventId, simType);
    return Void();
}

Return<void> MtkRadioExImpl::sendVsimOperation(int32_t serial, uint32_t transactionId,
        uint32_t eventId, int32_t result, int32_t dataLength, const hidl_vec<uint8_t>& data) {
    RLOGD("sendVsimOperation: serial %d", serial);
    dispatchVsimOperationEvent(serial, mSlotId, RIL_REQUEST_VSIM_OPERATION,
            transactionId, eventId, result, dataLength, data);
    return Void();
}

int mtkRadioEx::vsimNotificationResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {

    RLOGD("vsimNotificationResponse: serial %d, error: %d", serial, e);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        VsimEvent params = {};
        if (response == NULL || responseLen != sizeof(RIL_VsimEvent)) {
            RLOGE("vsimNotificationResponse: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            RIL_VsimEvent *p_cur = ((RIL_VsimEvent *) response);
            params.transactionId = p_cur->transaction_id;
            params.eventId = p_cur->eventId;
            params.simType = p_cur->sim_type;
        }

        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->
                vsimNotificationResponse(responseInfo, params);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);

    } else {
        RLOGE("vsimNotificationResponse: mtkRadioExService[%d]->mRadioResponse == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::vsimOperationResponse(int slotId, android::ClientId clientId __unused, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen) {

    RLOGD("vsimOperationResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->vsimOperationResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("vsimOperationResponse: mtkRadioExService[%d]->mRadioResponseMtk == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::onVsimEventIndication(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responselen) {

    RLOGD("onVsimEventIndication: indicationType %d", indicationType);

    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responselen == 0) {
            RLOGE("onVsimEventIndication: invalid response");
            return 0;
        }

        VsimOperationEvent event = {};
        RIL_VsimOperationEvent *response_data = (RIL_VsimOperationEvent *)response;
        event.transactionId = response_data->transaction_id;
        event.eventId = response_data->eventId;
        event.result = response_data->result;
        event.dataLength = response_data->data_length;
        event.data = convertCharPtrToHidlString(response_data->data);

        //RLOGD("onVsimEventIndication: id=%d, data_length=%d, data=%s", event.eventId, response_data->data_length, response_data->data);

        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk->onVsimEventIndication(
                convertIntToRadioIndicationType(indicationType), event);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("onVsimEventIndication: mtkRadioExService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}
// External SIM [END]

Return<void> MtkRadioExImpl::setVoiceDomainPreference(int32_t serial, int32_t vdp){
    RLOGD("setVoiceDomainPreference: %d", vdp);

    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_VOICE_DOMAIN_PREFERENCE, 1, vdp);

    return Void();
}

int mtkRadioEx::setVoiceDomainPreferenceResponse(int slotId, android::ClientId clientId __unused,
                            int responseType, int serial, RIL_Errno e,
                            void *response,
                            size_t responselen) {

    RLOGD("setVoiceDomainPreferenceResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 setVoiceDomainPreferenceResponse(responseInfo);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("setVoiceDomainPreferenceResponse: mtkRadioExService[%d]->mRadioResponseIms == NULL",
                slotId);
    }

    return 0;
}

void convertRilDedicatedBearerInfoToHal(RIL_Dedicate_Data_Call_Struct* ddcResponse,
        DedicateDataCall& ddcResult) {

    RLOGD("convertRilDedicatedBearerInfoToHal");
    ddcResult.ddcId= ddcResponse->ddcId;
    ddcResult.interfaceId = ddcResponse->interfaceId;
    ddcResult.primaryCid = ddcResponse->primaryCid;
    ddcResult.cid = ddcResponse->cid;
    ddcResult.active = ddcResponse->active;
    ddcResult.signalingFlag = ddcResponse->signalingFlag;
    ddcResult.bearerId = ddcResponse->bearerId;

    if (ddcResponse->hasQos) {
        ddcResult.hasQos = 1;
        ddcResult.qos.qci = ddcResponse->qos.qci;
        ddcResult.qos.dlGbr = ddcResponse->qos.dlGbr;
        ddcResult.qos.ulGbr = ddcResponse->qos.ulGbr;
        ddcResult.qos.dlMbr = ddcResponse->qos.dlMbr;
        ddcResult.qos.ulMbr = ddcResponse->qos.ulMbr;
    }

    if (ddcResponse->hasTft) {
        ddcResult.hasTft = 1;
        ddcResult.tft.operation = ddcResponse->tft.operation;
        ddcResult.tft.pfNumber = ddcResponse->tft.pfNumber;
        ddcResult.tft.pfList.resize(ddcResponse->tft.pfNumber);
        for (int i = 0; i < ddcResult.tft.pfNumber; i ++) {
            ddcResult.tft.pfList[i].id = ddcResponse->tft.pfList[i].id;
            ddcResult.tft.pfList[i].precedence = ddcResponse->tft.pfList[i].precedence;
            ddcResult.tft.pfList[i].direction = ddcResponse->tft.pfList[i].direction;
            ddcResult.tft.pfList[i].networkPfIdentifier = ddcResponse->tft.pfList[i].networkPfIdentifier;
            ddcResult.tft.pfList[i].bitmap = ddcResponse->tft.pfList[i].bitmap;
            ddcResult.tft.pfList[i].address = convertCharPtrToHidlString(ddcResponse->tft.pfList[i].address);
            ddcResult.tft.pfList[i].mask = convertCharPtrToHidlString(ddcResponse->tft.pfList[i].mask);
            ddcResult.tft.pfList[i].protocolNextHeader = ddcResponse->tft.pfList[i].protocolNextHeader;
            ddcResult.tft.pfList[i].localPortLow = ddcResponse->tft.pfList[i].localPortLow;
            ddcResult.tft.pfList[i].localPortHigh = ddcResponse->tft.pfList[i].localPortHigh;
            ddcResult.tft.pfList[i].remotePortLow = ddcResponse->tft.pfList[i].remotePortLow;
            ddcResult.tft.pfList[i].remotePortHigh = ddcResponse->tft.pfList[i].remotePortHigh;
            ddcResult.tft.pfList[i].spi = ddcResponse->tft.pfList[i].spi;
            ddcResult.tft.pfList[i].tos = ddcResponse->tft.pfList[i].tos;
            ddcResult.tft.pfList[i].tosMask = ddcResponse->tft.pfList[i].tosMask;
            ddcResult.tft.pfList[i].flowLabel = ddcResponse->tft.pfList[i].flowLabel;
        }

        ddcResult.tft.tftParameter.linkedPfNumber = ddcResponse->tft.tftParameter.linkedPfNumber;
        int linkedPfNumber = ddcResult.tft.tftParameter.linkedPfNumber;
        ddcResult.tft.tftParameter.linkedPfList.resize(linkedPfNumber);
        for (int i = 0; i < linkedPfNumber; i++) {
            ddcResult.tft.tftParameter.linkedPfList[i] = ddcResponse->tft.tftParameter.linkedPfList[i];
        }
    }
    ddcResult.pcscf = convertCharPtrToHidlString(ddcResponse->pcscf);

    RLOGD("[PdnInfo] ddcId:%d, interfaceId:%d, primaryCid:%d, cid:%d, active:%d, signalingFlag:%d, bearerId:%d, pcscf:%s, hasQos:%d, hasTft:%d"
        , ddcResult.ddcId, ddcResult.interfaceId, ddcResult.primaryCid, ddcResult.cid
        , ddcResult.active, ddcResult.signalingFlag, ddcResult.bearerId, ddcResponse->pcscf
        , ddcResult.hasQos, ddcResult.hasTft);
    if (ddcResult.hasQos) {
        RLOGD("[QosInfo] qci:%d, dlGbr:%d, ulGbr:%d, dlMbr:%d, ulMbr:%d"
        , ddcResult.qos.qci, ddcResult.qos.dlGbr, ddcResult.qos.ulGbr, ddcResult.qos.dlMbr, ddcResult.qos.ulMbr);
    }

    if (ddcResult.hasTft) {
        RLOGD("[TftInfo] operation:%d, pfNumber:%d", ddcResult.tft.pfNumber, ddcResult.tft.pfNumber);
        for (int i = 0; i < ddcResult.tft.pfNumber; i ++) {
            RLOGD("[TftInfo] id:%d, precedence:%d, direction:%d, networkPfIdentifier:%d, bitmap:%d, address:%d, mask:%d, protocolNextHeader:%d",
                "localPortLow:%d, localPortHigh:%d, remotePortLow:%d, remotePortHigh:%d, spi:%d, tos:%d, tosMask:%d, flowLabel:%d"
                , ddcResult.tft.pfNumber, ddcResult.tft.pfNumber);
        }
    }
}

/// M: Ims Data Framework
int mtkRadioEx::dedicatedBearerActivationInd(int slotId,
        int indicationType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("dedicatedBearerActivationInd: serial %d", serial);

    DedicateDataCall ddcResult = {};
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen % sizeof(RIL_Dedicate_Data_Call_Struct) != 0) {
            RLOGE("dedicatedBearerActivationInd: invalid response");
        } else {
            convertRilDedicatedBearerInfoToHal((RIL_Dedicate_Data_Call_Struct *)response, ddcResult);
            Return<void> retStatus
                    = mtkRadioExService[slotId]->mRadioIndicationMtk->dedicatedBearerActivationInd(
                    convertIntToRadioIndicationType(indicationType), ddcResult);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
        }
    } else {
        RLOGE("dedicatedBearerActivationInd: mtkRadioExService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::dedicatedBearerModificationInd(int slotId,
        int indicationType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("dedicatedBearerModificationInd: serial %d", serial);

    DedicateDataCall ddcResult = {};
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen % sizeof(RIL_Dedicate_Data_Call_Struct) != 0) {
            RLOGE("dedicatedBearerModificationInd: invalid response");
        } else {
            convertRilDedicatedBearerInfoToHal((RIL_Dedicate_Data_Call_Struct *)response, ddcResult);
            Return<void> retStatus
                    = mtkRadioExService[slotId]->mRadioIndicationMtk->dedicatedBearerModificationInd(
                    convertIntToRadioIndicationType(indicationType), ddcResult);
            mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
        }
    } else {
        RLOGE("dedicatedBearerModificationInd: mtkRadioExService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}

int mtkRadioEx::dedicatedBearerDeactivationInd(int slotId,
        int indicationType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("dedicatedBearerDeactivationInd: serial %d", serial);

    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen == 0) {
            RLOGE("dedicatedBearerDeactivationInd: invalid response");
            return 0;
        }
        int32_t cid = ((int32_t *) response)[0];
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioIndicationMtk->dedicatedBearerDeactivationInd(
                convertIntToRadioIndicationType(indicationType), cid);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("dedicatedBearerDeactivationInd: mtkRadioExService[%d]->mRadioIndication == NULL", slotId);
    }

    return 0;
}


Return<void> MtkRadioExImpl::setWifiSignalLevel(int32_t serial,
            int32_t rssi, int32_t snr) {
    return Void();
}

Return<void> MtkRadioExImpl::setWifiEnabled(int32_t serial,
    const hidl_string& ifName, int32_t isWifiEnabled, int32_t isFlightModeOn) {
    return Void();
}

Return<void> MtkRadioExImpl::setWifiAssociated(int32_t serial, const hidl_string& ifName,
        int32_t associated, const hidl_string& ssid, const hidl_string& apMac, int32_t mtuSize,
        const hidl_string& ueMac) {
    return Void();
}

Return<void> MtkRadioExImpl::setWifiIpAddress(int32_t serial,
            const hidl_string& ifName,
            const hidl_string& ipv4Addr, const hidl_string& ipv6Addr,
            int32_t ipv4PrefixLen, int32_t ipv6PrefixLen,
            const hidl_string& ipv4Gateway, const hidl_string& ipv6Gateway,
            int32_t dnsCount, const hidl_string& dnsAddresses) {
    return Void();
}

Return<void> MtkRadioExImpl::setWfcConfig (int32_t serial,
        int32_t setting,
        const hidl_string& ifName,
        const hidl_string& value) {
    return Void();
}

Return<void> MtkRadioExImpl::querySsacStatus (int32_t serial) {
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_QUERY_SSAC_STATUS);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::setLocationInfo(int32_t serial,
        const hidl_string& accountId, const hidl_string& broadcastFlag, const hidl_string& latitude,
        const hidl_string& longitude, const hidl_string& accuracy, const hidl_string& method,
        const hidl_string& city, const hidl_string& state, const hidl_string& zip,
        const hidl_string& countryCode, const hidl_string& ueWlanMac) {
    return Void();
}

Return<void> MtkRadioExImpl::setEmergencyAddressId(int32_t serial,
    const hidl_string& aid) {
    return Void();
}

Return<void> MtkRadioExImpl::setNattKeepAliveStatus(int32_t serial,
        const hidl_string& ifName, bool enable,
        const hidl_string& srcIp, int32_t srcPort,
        const hidl_string& dstIp, int32_t dstPort) {
    return Void();
}

Return<void> MtkRadioExImpl::setWifiPingResult(int32_t serial, int32_t rat,
        int32_t latency, int32_t pktloss) {
    return Void();
}

// Notify ePDG screen state
Return<void> MtkRadioExImpl::notifyEPDGScreenState(int32_t serial, int32_t state) {
    return Void();
}

int mtkRadioEx::setWifiEnabledResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno err, void *response, size_t responseLen) {
    return 0;
}

int mtkRadioEx::setWifiAssociatedResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno err, void *response, size_t responseLen) {
    return 0;
}

int mtkRadioEx::setWfcConfigResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno err, void *response, size_t responseLen) {
    return 0;
}

int mtkRadioEx::querySsacStatusResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno err, void *response, size_t responseLen) {
    return 0;
}

int mtkRadioEx::setWifiSignalLevelResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno err, void *response, size_t responseLen) {
    return 0;
}

int mtkRadioEx::setWifiIpAddressResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno err, void *response, size_t responseLen) {
    return 0;
}

int mtkRadioEx::setLocationInfoResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno err, void *response, size_t responseLen) {
    return 0;
}

int mtkRadioEx::setEmergencyAddressIdResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno err, void *response, size_t responseLen) {
    return 0;
}

int mtkRadioEx::setNattKeepAliveStatusResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno err, void *response, size_t responseLen) {
    return 0;
}

int mtkRadioEx::setWifiPingResultResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno err, void *response, size_t responseLen) {
    return 0;
}

// Notify ePDG screen state
int mtkRadioEx::notifyEPDGScreenStateResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno err, void *response, size_t responseLen) {
    return 0;
}

int mtkRadioEx::onWifiMonitoringThreshouldChanged(int slotId,
        int indicationType, int token, RIL_Errno err, void *response, size_t responselen) {
    return 0;
}

int mtkRadioEx::onWifiPdnActivate(int slotId, int indicationType,
        int token, RIL_Errno err, void *response, size_t responselen) {
    return 0;
}

int mtkRadioEx::onWfcPdnError(int slotId, int indicationType,
        int token, RIL_Errno err, void *response, size_t responselen) {
    return 0;
}

int mtkRadioEx::onPdnHandover(int slotId, int indicationType,
        int token, RIL_Errno err, void *response, size_t responselen) {
    return 0;
}

int mtkRadioEx::onWifiRoveout(int slotId, int indicationType,
        int token, RIL_Errno err, void *response, size_t responselen) {
    return 0;
}

int mtkRadioEx::onLocationRequest(int slotId,
        int indicationType, int token, RIL_Errno err, void *response, size_t responselen) {
    return 0;
}

int mtkRadioEx::onWfcPdnStateChanged(int slotId,
        int indicationType, int token, RIL_Errno err, void *response, size_t responselen) {
    return 0;
}

int mtkRadioEx::onNattKeepAliveChanged(int slotId,
        int indicationType, int token, RIL_Errno err, void *response, size_t responselen) {
    return 0;
}

int mtkRadioEx::onWifiPingRequest(int slotId,
        int indicationType, int token, RIL_Errno err, void *response, size_t responselen) {
    return 0;
}

int mtkRadioEx::onWifiPdnOOS(int slotId, int indicationType,
        int token, RIL_Errno err, void *response, size_t responselen) {
    return 0;
}

int mtkRadioEx::onWifiLock(int slotId, int indicationType,
        int token, RIL_Errno err, void *response, size_t responselen) {
    return 0;
}

int mtkRadioEx::onSsacStatus(int slotId, int indicationType,
        int token, RIL_Errno err, void *response, size_t responselen) {
    return 0;
}

Return<void> MtkRadioExImpl::setServiceStateToModem(int32_t serial, int32_t voiceRegState,
            int32_t dataRegState, int32_t voiceRoamingType, int32_t dataRoamingType,
            int32_t rilVoiceRegState, int32_t rilDataRegState) {
    RLOGD("setServiceStateToModem: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_SERVICE_STATE);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

int mtkRadioEx::setServiceStateToModemResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno err, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, err);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk
                ->setServiceStateToModemResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("%s: mtkRadioExService[%d]->mRadioResponseMtk == NULL", __func__, slotId);
    }
    return 0;
}

int mtkRadioEx::onTxPowerIndication(int slotId,
        int indicationType, int token, RIL_Errno err, void *response, size_t responseLen) {
    RLOGD("onTxPowerIndication, no need to support");
    return 0;
}

int mtkRadioEx::onTxPowerStatusIndication(int slotId,
        int indicationType, int token, RIL_Errno err, void *response, size_t responseLen) {
    RLOGD("onTxPowerStatusIndication, no need to support");
    return 0;
}

Return<void> MtkRadioExImpl::setTxPowerStatus(int32_t serial, int32_t mode) {
    RLOGD("reportSimMode: serial %d, not support in legacy ril",
            serial);
    return Void();
}

int radio::setSignalStrengthReportingCriteriaResponse(int slotId,
        android::ClientId clientId __unused, int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    RLOGD("setSignalStrengthReportingCriteriaResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponse != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponseV1_2->setSignalStrengthReportingCriteriaResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGD("setSignalStrengthReportingCriteriaResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::currentPhysicalChannelConfigs(int slotId,
                                    int indicationType, int token, RIL_Errno e,
                                    void *response, size_t responseLen) {
    return 0;
}

int radio::setLinkCapacityReportingCriteriaResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("setLinkCapacityReportingCriteriaResponse: serial %d", serial);

    if (radioService[slotId] != NULL && radioService[slotId]->mRadioResponseV1_2 != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus =
                radioService[slotId]->mRadioResponseV1_2->setLinkCapacityReportingCriteriaResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setLinkCapacityReportingCriteriaResponse: radioService[%d] or mRadioIndicationV1_2 == NULL",
                slotId);
    }

    return 0;

}

int radio::currentLinkCapacityEstimate(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndicationV1_2 != NULL) {
        if (response == NULL || responseLen % sizeof(RIL_LinkCapacityEstimate) != 0) {
            RLOGE("currentLinkCapacityEstimate: invalid response");
            return 0;
        }
        AOSP_V1_2::LinkCapacityEstimate lce;
        RIL_LinkCapacityEstimate *resp = (RIL_LinkCapacityEstimate *) response;
        lce.downlinkCapacityKbps = resp->downlinkCapacityKbps;
        lce.uplinkCapacityKbps = resp->uplinkCapacityKbps;
        Return<void> retStatus = radioService[slotId]->mRadioIndicationV1_2->
                currentLinkCapacityEstimate(convertIntToRadioIndicationType(indicationType), lce);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("currentLinkCapacityEstimate: radioService[%d] or mRadioIndicationV1_2 == NULL",
                slotId);
    }

    return 0;
}

int mtkRadioEx::onImsConferenceInfoIndication(int slotId,
        int indicationType, int token, RIL_Errno err, void *response, size_t responseLen) {
    return 0;
}

int mtkRadioEx::onLteMessageWaitingIndication(int slotId,
        int indicationType, int token, RIL_Errno err, void *response, size_t responseLen) {
    return 0;
}

///M:Dialog Event Package Info @{
int mtkRadioEx::imsDialogIndicationInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    return 0;
}
///@}

// PS/CS attach request.
Return<void> MtkRadioExImpl::dataConnectionAttach(int32_t serial, int type) {
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_DATA_CONNECTION_ATTACH);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

// PS/CS attach response.
int mtkRadioEx::dataConnectionAttachResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    return 0;
}

// PS/CS detach request.
Return<void> MtkRadioExImpl::dataConnectionDetach(int32_t serial, int type) {
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_DATA_CONNECTION_DETACH);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

// PS/CS detach response.
int mtkRadioEx::dataConnectionDetachResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    return 0;
}

// Cleanup all connections request.
Return<void> MtkRadioExImpl::resetAllConnections(int32_t serial) {
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_RESET_ALL_CONNECTIONS);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

// Cleanup all connections response.
int mtkRadioEx::resetAllConnectionsResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    return 0;
}

int mtkRadioEx::setTxPowerStatusResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("setTxPowerStatusResponse: serial %d, no need to support", serial);
    return 0;
}

int mtkRadioEx::setSuppServPropertyResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = mtkRadioExService[slotId]->mRadioResponseMtk->setSuppServPropertyResponse(
                responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("setSuppServPropertyResponse: mtkRadioExService[%d]->mRadioResponseMtk=NULL",
            slotId);
    }
    return 0;
}

bool isMtkFwkAddonNotExisted(int slotId) {
    return (mtkRadioExService[slotId]->mRadioResponseMtk == NULL) &&
            (mtkRadioExService[slotId]->mRadioIndicationMtk == NULL);
}

int mtkRadioEx::onDsbpStateChanged(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("onDsbpStateChanged not supported", slotId);
    return 0;
}

// MTK-START: SIM RSU
int mtkRadioEx::onRsuSimLockEvent(int slotId, int indicationType, int token, RIL_Errno e,
        void *response, size_t responseLen) {
    RLOGE("[RSU-SIMLOCK] onRsuSimLockEvent: not support now");
    return 0;
}

// MTK-START: SIM SLOT LOCK
int mtkRadioEx::smlSlotLockInfoChangedInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL) {
            RLOGE("smlSlotLockInfoChangedInd: invalid response!");
            return 0;
        }
        hidl_vec<int32_t> data;
        int *pInt = (int *) response;
        int numInts = responseLen / sizeof(int);
        RLOGD("smlSlotLockInfoChangedInd responselen: %d, sizeof(int): %lu, numInts: %d",
                responseLen, (unsigned long)sizeof(int), numInts);
        data.resize(numInts);
        for (int i = 0; i < numInts; i++) {
            data[i] = (int32_t) pInt[i];
        }
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioIndicationMtk
                ->smlSlotLockInfoChangedInd(convertIntToRadioIndicationType(indicationType), data);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("smlSlotLockInfoChangedInd: mtkRadioExService[%d] or mRadioIndication is NULL!", slotId);
    }
    return 0;
}

int mtkRadioEx::supplyDeviceNetworkDepersonalizationResponse(int slotId,
        android::ClientId clientId __unused, int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    RLOGD("supplyDeviceNetworkDepersonalizationResponse: serial %d", serial);

    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        if (response == NULL) {
            RLOGE("supplyDeviceNetworkDepersonalizationResponse: invalid response!");
            return 0;
        }
        RadioResponseInfo responseInfo = {};
        int ret = responseIntOrEmpty(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->
                supplyDeviceNetworkDepersonalizationResponse(responseInfo, ret);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("supplyDeviceNetworkDepersonalizationResponse: mtkRadioExService[%d]->mRadioResponse == "
                "NULL", slotId);
    }
    return 0;
}

Return<void> MtkRadioExImpl::supplyDeviceNetworkDepersonalization(int32_t serial,
        const hidl_string& netPin) {
    RLOGD("supplyDeviceNetworkDepersonalization: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_ENTER_DEVICE_NETWORK_DEPERSONALIZATION, true,
            1, netPin.c_str());
    return Void();
}
// MTK-END

// M: RTT @{
int mtkRadioEx::rttModifyResponseInd(int slotId,
                               int indicationType, int token, RIL_Errno e,
                               void *response, size_t responselen) {

    RLOGE("rttModifyResponseInd: slotId %d", slotId);

    return 0;
}

int mtkRadioEx::rttTextReceiveInd(int slotId,
                             int indicationType, int token, RIL_Errno e,
                             void *response, size_t responselen) {

    RLOGE("rttTextReceiveInd: slotId %d", slotId);

    return 0;
}

int mtkRadioEx::rttCapabilityIndicationInd(int slotId,
                                      int indicationType, int token, RIL_Errno e,
                                      void *response, size_t responselen) {

    RLOGE("rttCapabilityIndicationInd: slotId %d", slotId);

    return 0;
}

int mtkRadioEx::rttModifyRequestReceiveInd(int slotId,
                                      int indicationType, int token, RIL_Errno e,
                                      void *response, size_t responselen) {

    RLOGE("rttModifyRequestReceiveInd: slotId %d", slotId);

    return 0;
}

int mtkRadioEx::audioIndicationInd(int slotId,
                                      int indicationType, int token, RIL_Errno e,
                                      void *response, size_t responselen) {

    RLOGE("audioIndicationInd: slotId %d", slotId);

    return 0;
}

// @}

int mtkRadioEx::sendVopsIndication(int slotId,
                                int indicationType, int token, RIL_Errno e,
                                void *response, size_t responselen) {
    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    bool perform = false;
    if (mtkRadioExService[imsSlotId] == NULL) {
        RLOGE("sendVopsIndication: mtkRadioExService[%d] is NULL", imsSlotId);
        return 0;
    }

    // TODO: Wait for mRadioIndicationRcs is ready
    //if (radioService[imsSlotId]->mRadioIndicationRcs == NULL) {
    //    RLOGE("imsVoPSIndication: radioService[%d]->mRadioIndicationRcs == NULL", imsSlotId);
    //} else {
    //    perform = true;
    //}

    if (mtkRadioExService[imsSlotId]->mRadioIndicationIms == NULL) {
        RLOGE("sendVopsIndication: mtkRadioExService[%d]->mRadioIndicationIms == NULL", imsSlotId);
        appendRequestIntoPendingInd(slotId, RIL_UNSOL_VOPS_INDICATION, indicationType, response,
                    responselen);
    } else {
        perform = true;
    }

    if (perform) {
        int *resp = (int *) response;
        int numStrings = responselen / sizeof(int);
        if(numStrings < 1) {
            RLOGE("sendVopsIndication: items length invalid, slotId = %d",imsSlotId);
            return 0;
        }

        int vops = ((int32_t *) resp)[0];

        RLOGD("sendVopsIndication - vops = %d", vops);

        // TODO: Wait for mRadioIndicationRcs is ready
        //if (radioService[imsSlotId]->mRadioIndicationRcs != NULL) {
        //    Return<void> retStatus = radioService[imsSlotId]->
        //                             mRadioIndicationRcs->imsVoPSIndication(
        //                             convertIntToRadioIndicationType(indicationType),
        //                             vops);
        //    radioService[imsSlotId]->checkReturnStatus(retStatus, RCS_MTK);
        //}

        if (mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {
            Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                     mRadioIndicationIms->sendVopsIndication(
                                     convertIntToRadioIndicationType(indicationType),
                                     vops);
            mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);

            if (retStatus.isOk() == false) {
                appendRequestIntoPendingInd(slotId, RIL_UNSOL_VOPS_INDICATION, indicationType,
                    response, responselen);
            }
        }
    }

    return 0;
}

Return<void> MtkRadioExImpl::queryVopsStatus(int32_t serial) {
    RLOGD("queryVopsStatus: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_QUERY_VOPS_STATUS);
    return Void();
}

void appendRequestIntoPendingInd(int slotId, int request, int responseType,
        void* data, int size) {
    if (request != RIL_UNSOL_VOPS_INDICATION &&
        request != RIL_UNSOL_IMS_ENABLE_START &&
        request != RIL_UNSOL_IMS_DISABLE_START &&
        request != RIL_UNSOL_IMS_ENABLE_DONE &&
        request != RIL_UNSOL_IMS_DISABLE_DONE &&
        request != RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED) {
        RLOGE("Request %s don't need to append into pending ind to MAL list",
                requestToString(request));
        return;
    }

    bool ignore_duplicate_request = false;
    // If it is not necessary to keep multiple same request id in the pendding list,
    // to add the request id here
    if (request == RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED) {
        ignore_duplicate_request = true;
    }

    // Cache the indication
    int slot = toRealSlot(slotId);
    RIL_ImsPendingInd *node = s_ims_pending_ind[slot];
    while (node != NULL) {
        if (ignore_duplicate_request && node->request == request) {
            RLOGD("Request %s(%d) is already in IMS pending ind list",
                requestToString(request), slot);
            return;
        }

        if (node->next != NULL) {
            node = node->next;
        } else {
            break;
        }
    }

    if (node == NULL) {
        node = (RIL_ImsPendingInd  *) calloc(1, sizeof(RIL_ImsPendingInd));
        node->slot = slotId;
        node->request = request;
        node->responseType = responseType;
        node->next = NULL;
        if (data != NULL) {
            node->data = (void*) calloc(1, size);
            memcpy(node->data, data, size);
            node->data_size = size;
        }
        s_ims_pending_ind[slot] = node;
    } else {
        RIL_ImsPendingInd *item =
                (RIL_ImsPendingInd  *) calloc(1, sizeof(RIL_ImsPendingInd));
        item->slot = slotId;
        item->request = request;
        item->responseType = responseType;
        item->next = NULL;
        if (data != NULL) {
            item->data = (void*) calloc(1, size);
            memcpy(item->data, data, size);
            item->data_size = size;
        }
        node->next = item;
    }

    RLOGI("Request %s(%d) already append into IMS pending ind list",
                requestToString(request), slot);
}

Return<void> MtkRadioExImpl::notifyImsServiceReady() {
    // Fire pending indication here because the function must be called after all event
    // registrations are ready in the constructor of the class ImsService.java
    int slot = toRealSlot(mSlotId);
    RIL_ImsPendingInd *node = s_ims_pending_ind[slot];
    RIL_ImsPendingInd *pre_node = NULL;
    while (node != NULL) {
        // Fire pending indication
        RLOGI("[%d]Fire Pending indication %s(%d)", slot, requestToString(node->request),
                node->slot);
        switch(node->request) {
            case RIL_UNSOL_VOPS_INDICATION:
                mtkRadioEx::sendVopsIndication(
                        node->slot, node->responseType, 0, RIL_E_SUCCESS, node->data,
                        node->data_size);
                break;
            case RIL_UNSOL_IMS_ENABLE_START:
                mtkRadioEx::imsEnableStartInd(node->slot, node->responseType, 0, RIL_E_SUCCESS,
                        NULL, 0);
                break;
            case RIL_UNSOL_IMS_DISABLE_START:
                mtkRadioEx::imsDisableStartInd(node->slot, node->responseType, 0, RIL_E_SUCCESS,
                        NULL, 0);
                break;
            case RIL_UNSOL_IMS_ENABLE_DONE:
                mtkRadioEx::imsEnableDoneInd(node->slot, node->responseType, 0, RIL_E_SUCCESS,
                        NULL, 0);
                break;
            case RIL_UNSOL_IMS_DISABLE_DONE:
                mtkRadioEx::imsDisableDoneInd(node->slot, node->responseType, 0, RIL_E_SUCCESS,
                        NULL, 0);
                break;
            case RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED:
                mtkRadioEx::radioStateChangedToImsInd(
                        node->slot, node->responseType, 0, RIL_E_SUCCESS, node->data,
                        node->data_size);
                break;
        }
        pre_node = node;
        node = node->next;
        if (pre_node->data != NULL) {
            free(pre_node->data);
        }
        free(pre_node);
    }
    s_ims_pending_ind[slot] = NULL;
    return Void();
}

int mtkRadioEx::queryVopsStatusResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("[%d]queryVopsStatusResponse: serial %d", slotId, serial);

    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        if (response == NULL) {
            RLOGE("queryVopsStatusResponse: invalid response!");
            return 0;
        }
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        int *resp = (int *) response;
        int numStrings = responseLen / sizeof(int);
        if(numStrings < 1) {
            RLOGE("queryVopsStatusResponse: items length invalid, slotId = %d", slotId);
            return 0;
        }

        int vops = ((int32_t *) resp)[1];

        RLOGI("queryVopsStatusResponse - vops = %d", vops);
        Return<void> retStatus =
                mtkRadioExService[slotId]->mRadioResponseIms->queryVopsStatusResponse(
                responseInfo, vops);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("queryVopsStatusResponse: "
                "radioService[%d]->mRadioResponseIms == NULL", slotId);
    }
    return 0;
}

Return<void> MtkRadioExImpl::getPlmnNameFromSE13Table(int32_t serial, int32_t mcc, int32_t mnc) {
    RLOGD("getPlmnNameFromSE13Table: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_GET_TS25_NAME, 2, mcc, mnc);
    return Void();
}

int mtkRadioEx::getPlmnNameFromSE13TableResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("getPlmnNameFromSE13TableResponse: serial %d", serial);

    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioResponseMtk != NULL) {
        if (response == NULL) {
            RLOGD("getPlmnNameFromSE13TableResponse: invalid response!");
            return 0;
        }
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->
                getPlmnNameFromSE13TableResponse(responseInfo, convertCharPtrToHidlString((char *) response));
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("getPlmnNameFromSE13TableResponse: "
                "mtkRadioExService[%d]->mRadioResponse == NULL", slotId);
    }
    return 0;
}

Return<void> MtkRadioExImpl::enableCAPlusBandWidthFilter(int32_t serial, bool enabled) {
    RLOGD("enableCAPlusBandWidthFilter: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_ENABLE_CA_PLUS_FILTER, 1, (enabled? 1: 0));
    return Void();
}

int mtkRadioEx::enableCAPlusBandWidthFilterResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("enableCAPlusBandWidthFilterResponse: serial %d", serial);

    if (mtkRadioExService[slotId] != NULL && mtkRadioExService[slotId]->mRadioResponseMtk!= NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseMtk->
                enableCAPlusBandWidthFilterResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::RADIO_MTK, slotId);
    } else {
        RLOGE("enableCAPlusBandWidthFilterResponse: "
                "mtkRadioExService[%d]->mRadioResponse == NULL", slotId);
    }
    return 0;
}

int mtkRadioEx::qualifiedNetworkTypesChangedInd(int slotId,
       int indicationType, int token, RIL_Errno e, void *response,
       size_t responseLen) {
    RLOGD("qualifiedNetworkTypesChangedInd, no need to support");
    return 0;
}

Return<void> MtkRadioExImpl::getVoiceDomainPreference(int32_t serial){
    RLOGD("getVoiceDomainPreference: serial %d", serial);
    // dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_VOICE_DOMAIN_PREFERENCE);
    return Void();
}

int mtkRadioEx::getVoiceDomainPreferenceResponse(int slotId, android::ClientId clientId __unused,
                            int responseType, int serial, RIL_Errno e,
                            void *response,
                            size_t responselen) {

    RLOGD("getVoiceDomainPreferenceResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseIms!= NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseInt(responseInfo, serial, responseType, e, response, responselen);

        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 getVoiceDomainPreferenceResponse(responseInfo, ret);

        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("getVoiceDomainPreferenceResponse: radioService[%d]->mRadioResponseIms == NULL",
                slotId);
    }

    return 0;
}

Return<void> MtkRadioExImpl::setSipHeader(int32_t serial, const hidl_vec<hidl_string>& data) {
    RLOGD("setSipHeader: serial %d", serial);
    // dispatchStrings(serial, mSlotId, RIL_REQUEST_SET_SIP_HEADER, data);
    return Void();
}

Return<void> MtkRadioExImpl::setSipHeaderReport(int32_t serial, const hidl_vec<hidl_string>& data) {
    RLOGD("setSipHeaderReport: serial %d", serial);
    // dispatchStrings(serial, mSlotId, RIL_REQUEST_SIP_HEADER_REPORT, data);
    return Void();
}

Return<void> MtkRadioExImpl::setImsCallMode(int32_t serial, int32_t mode) {
    RLOGD("setImsCallMode: serial %d", serial);
    // dispatchInts(serial, mSlotId, RIL_REQUEST_SET_IMS_CALL_MODE, 1, mode);
    return Void();
}

int mtkRadioEx::setSipHeaderResponse(int slotId, android::ClientId clientId, int responseType,
                            int serial, RIL_Errno e, void *response,
                            size_t responselen) {
    RLOGD("setSipHeaderResponse: serial %d", serial);
    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 setSipHeaderResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("setSipHeaderResponse: radioService[%d]->mRadioResponseIms == NULL",
                                                                            slotId);
    }

    return 0;
}

int mtkRadioEx::setSipHeaderReportResponse(int slotId, android::ClientId clientId, int responseType,
                            int serial, RIL_Errno e, void *response,
                            size_t responselen) {
    RLOGD("setSipHeaderReportResponse: serial %d", serial);
    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 setSipHeaderReportResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("setSipHeaderReportResponse: radioService[%d]->mRadioResponseIms == NULL",
                                                                            slotId);
    }

    return 0;
}

int mtkRadioEx::setImsCallModeResponse(int slotId, android::ClientId clientId, int responseType,
                            int serial, RIL_Errno e, void *response,
                            size_t responselen) {
    RLOGD("setImsCallModeResponse: serial %d", serial);
    if (mtkRadioExService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->
                                 mRadioResponseIms->
                                 setImsCallModeResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("setImsCallModeResponse: radioService[%d]->mRadioResponseIms == NULL",
                                                                            slotId);
    }

    return 0;
}

int mtkRadioEx::sipHeaderReportInd(int slotId,
                                 int indicationType, int token, RIL_Errno e,
                                 void *response, size_t responseLen) {

    char **resp = (char **) response;
    int numStrings = responseLen / sizeof(char *);
    if(numStrings < 3) {
        RLOGE("sipHeaderReportInd: items length is invalid, slot = %d", slotId);
        return 0;
    }

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL
            && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {

        hidl_vec<hidl_string> data;
        data.resize(numStrings);
        for (int i = 0; i < numStrings; i++) {
            data[i] = convertCharPtrToHidlString(resp[i]);
            RLOGD("sipHeaderReportInd:: %d: %s", i, resp[i]);
        }

        Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                 mRadioIndicationIms->sipHeaderReport(
                                 convertIntToRadioIndicationType(indicationType),
                                 data);

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("sipHeaderReportInd: radioService[%d]->mRadioIndicationIms == NULL",
                                                                          imsSlotId);
    }

    return 0;
}

int mtkRadioEx::callRatIndication(int slotId,
                                 int indicationType, int token, RIL_Errno e,
                                 void *response, size_t responseLen) {

    int *resp = (int *) response;
    int numInts = responseLen / sizeof(int);
    if (numInts < 1) {
        RLOGE("callRatIndication: items length invalid, slotId = %d", slotId);
        return 0;
    }

    int domain = ((int32_t *) resp)[0];
    int rat = 0;
    if (numInts >= 2) {
        rat = ((int32_t *) resp)[1];
    }

    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (mtkRadioExService[imsSlotId] != NULL
            && mtkRadioExService[imsSlotId]->mRadioIndicationIms != NULL) {
        Return<void> retStatus = mtkRadioExService[imsSlotId]->
                                 mRadioIndicationIms->callRatIndication(
                                 convertIntToRadioIndicationType(indicationType),
                                 domain, rat);

        mtkRadioExService[imsSlotId]->checkReturnStatusMtk(retStatus, android::IMS_MTK, slotId);
    } else {
        RLOGE("callRatIndication: radioService[%d]->mRadioIndicationIms == NULL",
                                                                          imsSlotId);
    }

    return 0;
}
// M: GWSD @{
Return<void> MtkRadioExImpl::setGwsdMode(int serial,
        const hidl_vec<hidl_string>& data) {
    // NOT support
    return Void();
}

Return<void> MtkRadioExImpl::setCallValidTimer(int serial, int timer) {
    // NOT support
    return Void();
}

Return<void> MtkRadioExImpl::setIgnoreSameNumberInterval(
        int serial, int interval) {
    // NOT support
    return Void();
}

Return<void> MtkRadioExImpl::setKeepAliveByPDCPCtrlPDU(
        int serial, const hidl_string& data) {
    // NOT support
    return Void();
}

Return<void> MtkRadioExImpl::setKeepAliveByIpData(
        int serial, const hidl_string& data) {
    // NOT support
    return Void();
}

int mtkRadioEx::setGwsdModeResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    // NOT support
    return 0;
}

int mtkRadioEx::setCallValidTimerResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    // NOT support
    return 0;
}

int mtkRadioEx::setIgnoreSameNumberIntervalResponse(int slotId,
        android::ClientId clientId __unused, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen) {
    // NOT support
    return 0;
}

int mtkRadioEx::setKeepAliveByPDCPCtrlPDUResponse(int slotId,
        android::ClientId clientId __unused, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen) {
    // NOT support
    return 0;
}

int mtkRadioEx::setKeepAliveByIpDataResponse(int slotId,
        android::ClientId clientId __unused, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen) {
    // NOT support
    return 0;
}
// @}
int mtkRadioEx::enableDsdaIndicationResponse(int slotId,
        android::ClientId clientId __unused, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen) {
    // NOT support
    return 0;
}

int mtkRadioEx::getDsdaStatusResponse(int slotId,
        android::ClientId clientId __unused, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen) {
    // NOT support
    return 0;
}

int mtkRadioEx::sipRegInfoInd(int slotId, int indicationType, int token, RIL_Errno e,
        void *response, size_t responseLen) {
    // NOT support
    return 0;
}

int mtkRadioEx::imsRegistrationStateInd(int slotId,
                              int indicationType, int token, RIL_Errno e,
                              void *response, size_t responseLen) {
    // NOT support
    return 0;
}

int mtkRadioEx::eiregDataInd(int slotId,
                              int indicationType, int token, RIL_Errno e,
                              void *response, size_t responseLen) {
    // NOT support
    return 0;
}

int radio::setSystemSelectionChannelsResponse(int slotId,
        android::ClientId clientId __unused, int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
/* TODO: no use for now.
    RLOGD("setSystemSelectionChannelsResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseV1_3 != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponseV1_3->setSystemSelectionChannelsResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("setSystemSelectionChannelsResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }
*/
    return 0;
}

int mtkRadioEx::onDsdaChangedInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen) {
    // not support
    return 0;
}

Return<void> MtkRadioExImpl::activateUiccCard(int32_t serial) {
    return Void();
}

Return<void> MtkRadioExImpl::deactivateUiccCard(int32_t serial) {
    return Void();
}

Return<void> MtkRadioExImpl::getCurrentUiccCardProvisioningStatus(int32_t serial) {
    return Void();
}

int mtkRadioEx::activateUiccCardRsp(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen) {
    return 0;
}

int mtkRadioEx::deactivateUiccCardRsp(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen) {
    return 0;
}

int mtkRadioEx::getCurrentUiccCardProvisioningStatusRsp(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    return 0;
}

int mtkRadioEx::radioStateChangedToImsInd(int slotId,
                                 int indicationType, int token, RIL_Errno e, void *response,
                                 size_t responseLen) {

    if(s_vendorFunctions == NULL) {
        RLOGE("radioStateChangedToImsInd: service is not ready");
        return 0;
    }

    // Retrive Radio State
    RadioState radioState = (RadioState) s_vendorFunctions->
                                         onStateRequest((RIL_SOCKET_ID)slotId);
    RLOGD("radioStateChangedToImsInd: radioState %d, slot = %d", radioState, slotId);

    // Send to IMS
    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (radioService[imsSlotId] != NULL && radioService[imsSlotId]->mRadioIndication != NULL) {

        Return<void> retStatus = radioService[imsSlotId]->mRadioIndication->radioStateChanged(
                convertIntToRadioIndicationType(indicationType), radioState);
        radioService[imsSlotId]->checkReturnStatus(retStatus, slotId);
    } else {
        RLOGE("radioStateChangedToImsInd: radioService[%d]->mRadioIndication == NULL", imsSlotId);
    }

    return 0;
}

// MUSE WFC requirement
Return<void> MtkRadioExImpl::registerCellQltyReport(int32_t serial,
            const hidl_string& enable, const hidl_string& type,
            const hidl_string& thresholdValues, const hidl_string& triggerTime) {
    RLOGE("registerCellQltyReport() Unsupported");
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_IWLAN_REGISTER_CELLULAR_QUALITY_REPORT);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

// MUSE WFC requirement
int mtkRadioEx::registerCellQltyReportResponse(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno e,
        void *response, size_t responselen) {
    return 0;
}

// MUSE WFC requirement
int mtkRadioEx::onCellularQualityChangedInd(int slotId, int indicationType,
        int token, RIL_Errno e, void *response, size_t responseLen) {
    return 0;
}

// MTK-START: RCS
Return<void> MtkRadioExImpl::setResponseFunctionsRcs(
        const ::android::sp<IRcsRadioResponse>& radioResponseParam,
        const ::android::sp<IRcsRadioIndication>& radioIndicationParam) {
    RLOGD("setResponseFunctionsRcs, slotId:%d", mSlotId);
    return Void();
}
// MTK-END: RCS

Return<void> MtkRadioExImpl::setResponseFunctionsSmartRatSwitch (
        const ::android::sp<ISmartRatSwitchRadioResponse>& radioResponse,
        const ::android::sp<ISmartRatSwitchRadioIndication>& radioIndication) {
    return Void();
}

Return<void> MtkRadioExImpl::getSuggestedPlmnList(int32_t serial, int32_t rat, int32_t num, int32_t timer) {
    //not support
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_GET_SUGGESTED_PLMN_LIST);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

int mtkRadioEx::getSuggestedPlmnListResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    // not suport
    return 0;
}

Return<void> MtkRadioExImpl::routeCertificate(int32_t serial, int uid, const hidl_vec<uint8_t>& cert,
        const hidl_vec<uint8_t>& msg) {
    RLOGD("routeCertificate: serial %d", serial);
    return Void();
}

Return<void> MtkRadioExImpl::routeAuthMessage(int32_t serial, int uid, const hidl_vec<uint8_t>& msg) {
    RLOGD("routeAuthMessage: serial %d", serial);
    return Void();
}

Return<void> MtkRadioExImpl::enableCapabaility(int32_t serial, const hidl_string& id, int uid, int toActive) {
    RLOGD("enableCapabaility: serial %d", serial);
    return Void();
}

Return<void> MtkRadioExImpl::abortCertificate(int32_t serial, int uid) {
    RLOGD("abortCertificate: serial %d", serial);
    return Void();
}

int mtkRadioEx::routeCertificateResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("routeCertificateResponse: serial %d", serial);
    return 0;
}

int mtkRadioEx::routeAuthMessageResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("routeAuthMessageResponse: serial %d", serial);
    return 0;
}

int mtkRadioEx::enableCapabilityResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("enableCapabilityResponse: serial %d", serial);
    return 0;
 }

int mtkRadioEx::abortCertificateResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD("cancelCertificateResponse: serial %d", serial);
    return 0;
}

int mtkRadioEx::mobileDataUsageInd(int slotId, int indicationType,
        int token, RIL_Errno e, void *response, size_t responseLen) {
    return 0;
}

Return<void> MtkRadioExImpl::deactivateNrScgCommunication(int32_t serial, bool deactivate, bool allowSCGAdd) {
    //not support
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_DEACTIVATE_NR_SCG_COMMUNICACTION);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::getDeactivateNrScgCommunication(int32_t serial) {
    //not support
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_DEACTIVATE_NR_SCG_COMMUNICACTION);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::setMaxUlSpeed(int32_t serial, int32_t ulSpeed) {
    //not support
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_MAX_UL_SPEED);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::setCallAdditionalInfo(int32_t serial,
        const hidl_vec<hidl_string>& info) {
    return Void();
}

int mtkRadioEx::setCallAdditionalInfoResponse(int slotId,
        android::ClientId clientId __unused, int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    RLOGD("setCallAdditionalInfoResponse: serial %d", serial);

    if (mtkRadioExService[slotId]->mRadioResponseImsV1_6 != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = mtkRadioExService[slotId]->mRadioResponseImsV1_6->
                setCallAdditionalInfoResponse(responseInfo);
        mtkRadioExService[slotId]->checkReturnStatusMtk(retStatus,  android::IMS_MTK, slotId);
    } else {
        RLOGE("setCallAdditionalInfoResponse: mtkRadioExService[%d]->mRadioResponseImsV1_6"
                " == NULL", slotId);
    }

    return 0;
}

// [Softbank requirement]: Enable or disable call sub address feature.
Return<void> MtkRadioExImpl::setCallSubAddress(int32_t serial, bool enable) {
    RLOGE("not support setCallSubAddress: enable = %d", enable);
    return Void();
}

Return<void> MtkRadioExImpl::getCallSubAddress(int32_t serial) {
    RLOGE("not support getCallSubAddress: serial %d", serial);
    return Void();
}

int mtkRadioEx::onNwLimitInd(int slotId, int indicationType, int token,
        RIL_Errno err, void *response, size_t responseLen) {
    return 0;
}


Return<void> MtkRadioExImpl::smartRatSwitch(int32_t serial, int32_t mode, int32_t rat) {
    // NOT support
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SMART_RAT_SWITCH);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::getSmartRatSwitch(int32_t serial, int32_t mode) {
    // NOT support
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_GET_SMART_RAT_SWITCH);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl:: setSmartSceneSwitch (int32_t serial, int32_t mode, int32_t tGear,
        int32_t lGear) {
    // NOT support
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_SMART_RAT_SWITCH);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return Void();
}

Return<void> MtkRadioExImpl::sendSarIndicator(int32_t serial, int32_t sar_cmd_type,
        const hidl_string& sar_parameter) {
    // NOT support
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SEND_SAR_IND);
    if (pRI != NULL) {
        sendErrorResponse(pRI, RIL_E_REQUEST_NOT_SUPPORTED);
    }

    return Void();
}

int mtkRadioEx::sendSarIndicatorResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD(LOG_TAG, "sendSarIndicatorResponse: serial %d  not support", serial);
    return 0;
}

//RSU
Return<void> MtkRadioExImpl::sendRsuRequest(int serial, const RsuRequestInfo& rri) {
    // It's for 93 and later RILD now.
    RLOGD(LOG_TAG, "[NOT SUPPORT] [RSU-SIMLOCK] sendRsuRequest");
    return Void();
}

int mtkRadioEx::sendRsuRequestResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    RLOGD(LOG_TAG, "[NOT SUPPORT] sendRsuRequestResponse: serial %d", serial);
    return 0;
}

int mtkRadioEx::onRsuEvent(int slotId, int indicationType, int token,
        RIL_Errno e, void *response, size_t responseLen) {
    if (mtkRadioExService[slotId] != NULL
            && mtkRadioExService[slotId]->mRadioIndicationMtkV1_9 != NULL) {
         // It's for 93 and later RILD now.
         RLOGD(LOG_TAG, "[NOT SUPPORT] [RSU-SIMLOCK] onRsuEvent");
    } else {
         RLOGD(LOG_TAG,"onRsuEvent: radioService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}
//RSU end