/*
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

#define LOG_TAG "RILS-OP"

#include <android/hardware/radio/1.0/IRadio.h>
//#include <android/hardware/radio/deprecated/1.0/IOemHook.h>
#include <vendor/mediatek/hardware/radio_op/2.0/IRadioOp.h>
#include <vendor/mediatek/hardware/radio_op/2.0/IRadioResponseOp.h>
#include <vendor/mediatek/hardware/radio_op/2.0/IRadioIndicationOp.h>
#include <vendor/mediatek/hardware/radio_op/2.0/IPresenceRadioResponse.h>
#include <vendor/mediatek/hardware/radio_op/2.0/IPresenceRadioIndication.h>

#include <hwbinder/IPCThreadState.h>
#include <hwbinder/ProcessState.h>
#include <rilop_service.h>
#include <ril_service.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/SystemClock.h>
#include <inttypes.h>
#include "libmtkrilutils.h"
#include <telephony/mtk_rilop.h>
#include <telephony/mtk_ril.h>
#include "mtk_log.h"

#define INVALID_HEX_CHAR 16

using ::android::hardware::configureRpcThreadpool;
using ::android::hardware::joinRpcThreadpool;
using ::android::hardware::Return;
using ::android::hardware::Status;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_array;
using ::android::hardware::Void;
using android::CommandInfo;
using android::RequestInfo;
using android::requestToString;
using android::sp;

using ::android::hardware::radio::V1_0::RadioResponseType;
using ::android::hardware::radio::V1_0::RadioIndicationType;
using ::android::hardware::radio::V1_0::RadioResponseInfo;
using ::android::hardware::radio::V1_0::CardStatus;
using ::android::hardware::radio::V1_0::RadioError;
using ::android::hardware::radio::V1_0::RadioState;

using namespace vendor::mediatek::hardware::radio_op::V2_0;

namespace VENDOR_OP_V2_0 = vendor::mediatek::hardware::radio_op::V2_0;

#define BOOL_TO_INT(x) (x ? 1 : 0)
#define ATOI_NULL_HANDLED(x) (x ? atoi(x) : -1)
#define ATOI_NULL_HANDLED_DEF(x, defaultVal) (x ? atoi(x) : defaultVal)

RIL_RadioFunctions *s_vendorFunctions = NULL;
static CommandInfo *s_commands;

struct RadioImpl;
struct OemHookImpl;

#define MAX_SIM_COUNT 4
//#if (SIM_COUNT >= 2)
sp<RadioImpl> radioService[MAX_SIM_COUNT * android::CLIENT_COUNT];
// counter used for synchronization. It is incremented every time response callbacks are updated.
volatile int32_t mCounterRadio[MAX_SIM_COUNT * android::CLIENT_COUNT];

extern bool dispatchVoid(int serial, int slotId, int request);
extern bool dispatchInts(int serial, int slotId, int request, int countInts, ...);
extern RadioIndicationType convertIntToRadioIndicationType(int indicationType);
extern bool dispatchStrings(int serial, int slotId, int request, bool allowEmpty,
        int countStrings, ...);
extern bool dispatchString(int serial, int slotId, int request, const char * str);
extern void populateResponseInfo(RadioResponseInfo& responseInfo, int serial,
                                 int responseType, RIL_Errno e);
extern hidl_string convertCharPtrToHidlString(const char *ptr);
extern void memsetAndFreeStrings(int numPointers, ...);
extern bool copyHidlStringToRil(char **dest, const hidl_string &src, RequestInfo *pRI);
extern bool copyHidlStringToRil(char **dest, const hidl_string &src, RequestInfo *pRI, bool allowEmpty);


// To Compute IMS Slot Id
extern "C" int toRealSlot(int slotId);
extern "C" int toClientSlot(int slotId, android::ClientId clientId);

struct RadioImpl : public VENDOR_OP_V2_0::IRadioOp {
    int32_t mSlotId;
    sp<VENDOR_OP_V2_0::IRadioResponseOp> mRadioResponseMtk;
    sp<VENDOR_OP_V2_0::IRadioIndicationOp> mRadioIndicationMtk;
    sp<IImsRadioResponseOp> mRadioResponseIms;
    sp<IImsRadioIndicationOp> mRadioIndicationIms;
    sp<IDigitsRadioResponse> mRadioResponseDigits;
    sp<IDigitsRadioIndication> mRadioIndicationDigits;
    sp<IRcsRadioResponse> mRadioResponseRcs;
    sp<IRcsRadioIndication> mRadioIndicationRcs;
    sp<VENDOR_OP_V2_0::IPresenceRadioResponse> mRadioResponsePresence;
    sp<VENDOR_OP_V2_0::IPresenceRadioIndication> mRadioIndicationPresence;

    Return<void> responseAcknowledgement();

    Return<void> setResponseFunctions(
            const ::android::sp<VENDOR_OP_V2_0::IRadioResponseOp>& radioResponse,
            const ::android::sp<VENDOR_OP_V2_0::IRadioIndicationOp>& radioIndication);

    Return<void> setResponseFunctionsIms(
           const ::android::sp<IImsRadioResponseOp>& radioResponse,
           const ::android::sp<IImsRadioIndicationOp>& radioIndication);

    Return<void> setResponseFunctionsDigits(
           const ::android::sp<IDigitsRadioResponse>& radioResponse,
           const ::android::sp<IDigitsRadioIndication>& radioIndication);

    Return<void> setDigitsLine(int serial, int accountId,
                               int digitsSerial, bool isLogout,
                               bool hasNext, bool isNative,
                               const ::android::hardware::hidl_string& msisdn,
                               const ::android::hardware::hidl_string& sit);

    Return<void> setTrn(int serial,
                        const ::android::hardware::hidl_string& fromMsisdn,
                        const ::android::hardware::hidl_string& toMsisdn,
                        const ::android::hardware::hidl_string& trn);

    Return<void> setIncomingVirtualLine(int serial,
                        const ::android::hardware::hidl_string& fromMsisdn,
                        const ::android::hardware::hidl_string& toMsisdn);

    Return<void> dialFrom(int serial, const DialFrom& dialInfo);

    Return<void> sendUssiFrom(int serial, const hidl_string& from, int action,
                             const hidl_string& ussi);

    Return<void> cancelUssiFrom(int serial, const hidl_string& from);

    Return<void> setEmergencyCallConfig(int serial, int category, bool isForceEcc);

    Return<void> deviceSwitch(int serial,
                              const ::android::hardware::hidl_string& number,
                              const ::android::hardware::hidl_string& deviceId);

    Return<void> cancelDeviceSwitch(int serial);
    Return<void> exitSCBM(int serial);

    void checkReturnStatus(Return<void>& ret);

    Return<void> setRxTestConfig(int32_t serial, int32_t antType);
    Return<void> getRxTestResult(int32_t serial, int32_t mode);

    Return<void> setDisable2G(int32_t serial, bool mode);
    Return<void> getDisable2G(int32_t serial);

    Return<void> setResponseFunctionsRcs(
        const ::android::sp<IRcsRadioResponse>& radioResponse,
        const ::android::sp<IRcsRadioIndication>& radioIndication);

    Return<void> switchRcsRoiStatus(int32_t serial, bool status);
    Return<void> updateRcsCapabilities(int32_t serial, int32_t mode, const ::android::hardware::hidl_string& featureTags);
    Return<void> updateRcsSessionInfo(int32_t serial, int32_t count);
    Return<void> setDigitsRegStatus(int serial,
                                    const ::android::hardware::hidl_string& digitsinfo);

    Return<void> setResponseFunctionsPresence(
        const ::android::sp<IPresenceRadioResponse>& radioResponse,
        const ::android::sp<IPresenceRadioIndication>& radioIndication);

    Return<void> sendRsuRequest(int serial, const RsuRequestInfo& reuRequest);
};

void checkReturnStatus(int32_t slotId, Return<void>& ret, bool isRadioService) {
    if (ret.isOk() == false) {
        mtkLogE(LOG_TAG, "checkReturnStatus: unable to call response/indication callback");
        // Remote process hosting the callbacks must be dead. Reset the callback objects;
        // there's no other recovery to be done here. When the client process is back up, it will
        // call setResponseFunctions()

        // Caller should already hold rdlock, release that first
        // note the current counter to avoid overwriting updates made by another thread before
        // write lock is acquired.
        int counter = mCounterRadio[slotId];
        pthread_rwlock_t *radioServiceRwlockPtr = radio::getRadioServiceRwlock(slotId);
        int gotRLock = 0;
        int ret;
        if (radio::isTidInRLock((int) slotId, pthread_self()) != -1) {
            // had got the lock
            gotRLock = 1;
            radio::unlockRadioServiceRlock(radioServiceRwlockPtr, (int) slotId);
            mtkLogV(LOG_TAG, "checkReturnStatus: release r lock %d", slotId);
        } else {
            mtkLogE(LOG_TAG, "checkReturnStatus: didn't get r lock %d", slotId);
        }

        // acquire wrlock
        radio::lockRadioServiceWlock(radioServiceRwlockPtr, (int) slotId);

        // make sure the counter value has not changed
        if (counter == mCounterRadio[slotId]) {
            radioService[slotId]->mRadioResponseMtk = NULL;
            radioService[slotId]->mRadioIndicationMtk = NULL;
            mCounterRadio[slotId]++;
        } else {
            mtkLogE(LOG_TAG, "checkReturnStatus: not resetting responseFunctions as they likely "
                    "got updated on another thread");
        }

        // release wrlock
        radio::unlockRadioServiceWlock(radioServiceRwlockPtr, (int)slotId);

        if(gotRLock == 1) {
            // Reacquire rdlock
            radioServiceRwlockPtr = radio::getRadioServiceRwlock(slotId);
            radio::lockRadioServiceRlock(radioServiceRwlockPtr, (int)slotId);
            mtkLogV(LOG_TAG, "checkReturnStatus: got r lock %d", slotId);
        } else {
            mtkLogE(LOG_TAG, "checkReturnStatus: don't need to re-get r lock %d", slotId);
        }
    }
}

void RadioImpl::checkReturnStatus(Return<void>& ret) {
    ::checkReturnStatus(mSlotId, ret, true);
}

Return<void> RadioImpl::responseAcknowledgement() {
    android::releaseWakeLock();
    return Void();
}

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

Return<void> RadioImpl::setResponseFunctions(
        const ::android::sp<VENDOR_OP_V2_0::IRadioResponseOp>& radioResponseParam,
        const ::android::sp<VENDOR_OP_V2_0::IRadioIndicationOp>& radioIndicationParam) {
    mtkLogD(LOG_TAG, "setResponseFunctions");

    pthread_rwlock_t *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radio::lockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    mRadioResponseMtk = radioResponseParam;
    mRadioIndicationMtk = radioIndicationParam;

    radio::unlockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    return Void();
}

Return<void> RadioImpl::setResponseFunctionsIms(
        const ::android::sp<IImsRadioResponseOp>& radioResponseParam,
        const ::android::sp<IImsRadioIndicationOp>& radioIndicationParam) {
    mtkLogD(LOG_TAG, "setResponseFunctionsIms");

    pthread_rwlock_t *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radio::lockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    mRadioResponseIms = radioResponseParam;
    mRadioIndicationIms = radioIndicationParam;

    radio::unlockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    return Void();
}

Return<void> RadioImpl::setResponseFunctionsDigits(
           const ::android::sp<IDigitsRadioResponse>& radioResponseParam,
           const ::android::sp<IDigitsRadioIndication>& radioIndicationParam) {

    mtkLogD(LOG_TAG, "setResponseFunctionsDigits");

    pthread_rwlock_t *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radio::lockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    mRadioResponseDigits = radioResponseParam;
    mRadioIndicationDigits = radioIndicationParam;

    radio::unlockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    return Void();
}

int responseInt(RadioResponseInfo& responseInfo, int serial, int responseType, RIL_Errno e,
               void *response, size_t responseLen) {
    populateResponseInfo(responseInfo, serial, responseType, e);
    int ret = -1;

    if (response == NULL || responseLen != sizeof(int)) {
        mtkLogE(LOG_TAG, "responseInt: Invalid response");
        if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
    } else {
        int *p_int = (int *) response;
        ret = p_int[0];
    }
    return ret;
}

Return<void> RadioImpl::setRxTestConfig(int32_t serial, int32_t antType) {
    mtkLogD(LOG_TAG, "setRxTestConfig: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_VSS_ANTENNA_CONF, 1, antType);
    return Void();
}

Return<void> RadioImpl::getRxTestResult(int32_t serial, int32_t mode) {
    mtkLogD(LOG_TAG, "getRxTestResult: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_VSS_ANTENNA_INFO, 1, mode);
    return Void();
}

/* RCS over Internet PDN & DIGITS */
Return<void> RadioImpl::setResponseFunctionsRcs(
        const ::android::sp<IRcsRadioResponse>& radioResponseParam,
        const ::android::sp<IRcsRadioIndication>& radioIndicationParam) {
    mtkLogD(LOG_TAG, "setResponseFunctionsRcs");

    pthread_rwlock_t *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radio::lockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    mRadioResponseRcs = radioResponseParam;
    mRadioIndicationRcs = radioIndicationParam;

    radio::unlockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    return Void();
}
/* RCS over Internet PDN & DIGITS */

Return<void> RadioImpl::switchRcsRoiStatus(int32_t serial, bool status) {
    int32_t slotId = toRealSlot(mSlotId);
    mtkLogD(LOG_TAG, "switchRcsRoiStatus serial:%d, slotId:%d, status:%d", serial, slotId, status);
    dispatchInts(serial, slotId, RIL_REQUEST_SWITCH_RCS_ROI_STATUS, 1, BOOL_TO_INT(status));
    return Void();
}

Return<void> RadioImpl::updateRcsCapabilities(int32_t serial, int32_t mode, const ::android::hardware::hidl_string& featureTags) {
    int32_t slotId = toRealSlot(mSlotId);
    mtkLogD(LOG_TAG, "updateRcsCapabilities serial:%d, slotId:%d, mode:%d, featureTags:%s",
            serial, slotId, mode, featureTags.c_str());
    dispatchStrings(serial, slotId, RIL_REQUEST_UPDATE_RCS_CAPABILITIES, false, 2,
                    std::to_string(mode).c_str(), featureTags.c_str());
    return Void();
}

Return<void> RadioImpl::updateRcsSessionInfo(int32_t serial, int32_t count) {
    int32_t slotId = toRealSlot(mSlotId);
    mtkLogD(LOG_TAG, "updateRcsSessionInfo serial:%d, slotId:%d, count:%d", serial, slotId, count);
    dispatchInts(serial, slotId, RIL_REQUEST_UPDATE_RCS_SESSION_INFO, 1, count);
    return Void();
}

Return<void> RadioImpl::setDigitsRegStatus(int serial,
                            const ::android::hardware::hidl_string& digitsinfo) {

    mtkLogD(LOG_TAG, "setDigitsRegStatus - %s", digitsinfo.c_str());

    dispatchString(serial, mSlotId, RIL_REQUEST_SET_DIGITS_REG_STATUS, digitsinfo.c_str());
    return Void();
}

Return<void> RadioImpl::setResponseFunctionsPresence(
        const ::android::sp<VENDOR_OP_V2_0::IPresenceRadioResponse>& radioResponseParam,
        const ::android::sp<VENDOR_OP_V2_0::IPresenceRadioIndication>& radioIndicationParam) {
    mtkLogD(LOG_TAG, "setResponseFunctionsPresence");
    pthread_rwlock_t *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radio::lockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    mRadioResponsePresence = radioResponseParam;
    mRadioIndicationPresence = radioIndicationParam;
    radio::unlockRadioServiceWlock(radioServiceRwlockPtr, (int)mSlotId);

    return Void();
}

/*********************************************************************************/
/*  Vendor Command                                                               */
/*********************************************************************************/
Return<void> RadioImpl::setDigitsLine(int serial, int accountId,
                                      int digitsSerial, bool isLogout,
                                      bool hasNext, bool isNative,
                                      const ::android::hardware::hidl_string& msisdn,
                                      const ::android::hardware::hidl_string& sit) {
    mtkLogD(LOG_TAG, "setDigitsLine: serial %d", serial);

    hidl_string strAccountId = std::to_string(accountId);
    hidl_string strDigitsSerial = std::to_string(digitsSerial);
    hidl_string strIsLogout = std::to_string(isLogout);
    hidl_string strHasNext = std::to_string(hasNext);
    hidl_string strIsNative = std::to_string(isNative);

    dispatchStrings(serial, mSlotId, RIL_REQUEST_SET_DIGITS_LINE, true, 7,
                    strAccountId.c_str(), strDigitsSerial.c_str(), strIsLogout.c_str(),
                    strHasNext.c_str(), strIsNative.c_str(), msisdn.c_str(), sit.c_str());

    return Void();
}

Return<void> RadioImpl::setTrn(int serial,
                               const ::android::hardware::hidl_string& fromMsisdn,
                               const ::android::hardware::hidl_string& toMsisdn,
                               const ::android::hardware::hidl_string& trn) {

    mtkLogD(LOG_TAG, "setTrn: serial %d", serial);

    dispatchStrings(serial, mSlotId, RIL_REQUEST_SET_TRN, true, 3,
                    fromMsisdn.c_str(), toMsisdn.c_str(), trn.c_str());

    return Void();
}

Return<void> RadioImpl::setIncomingVirtualLine(int serial,
                               const ::android::hardware::hidl_string& fromMsisdn,
                               const ::android::hardware::hidl_string& toMsisdn) {
    mtkLogD(LOG_TAG, "setIncomingVirtualLine: serial %d", serial);

    dispatchStrings(serial, mSlotId, RIL_REQUEST_SET_INCOMING_VIRTUAL_LINE, true, 2,
            fromMsisdn.c_str(), toMsisdn.c_str());

    return Void();
}

Return<void> RadioImpl::dialFrom(int32_t serial, const DialFrom& dialInfo) {
    mtkLogD(LOG_TAG, "dialFrom: serial %d", serial);
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_DIAL_FROM);
    if (pRI == NULL) {
        return Void();
    }
    RIL_DialFrom dial = {};
    int32_t sizeOfDial = sizeof(dial);
    if (!copyHidlStringToRil(&dial.address, dialInfo.address, pRI)) {
        return Void();
    }
    if (!copyHidlStringToRil(&dial.fromAddress, dialInfo.fromAddress, pRI)) {
        return Void();
    }
    dial.clir = (int) dialInfo.clir;
    dial.isVideoCall= (bool) dialInfo.isVideoCall;
    s_vendorFunctions->onRequest(RIL_REQUEST_DIAL_FROM, &dial, sizeOfDial, pRI, pRI->socket_id);
    memsetAndFreeStrings(2, dial.address, dial.fromAddress);
    return Void();
}

Return<void> RadioImpl::sendUssiFrom(int serial, const hidl_string& from, int action,
                         const hidl_string& ussi) {
    mtkLogD(LOG_TAG, "sendUssiFrom: serial %d", serial);
    hidl_string strAction = std::to_string(action);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_SEND_USSI_FROM, false, 3,
                    from.c_str(), strAction.c_str(), ussi.c_str());

    return Void();
}

Return<void> RadioImpl::cancelUssiFrom(int serial, const hidl_string& from) {
    mtkLogD(LOG_TAG, "cancelUssiFrom: serial %d", serial);
    dispatchString(serial, mSlotId, RIL_REQUEST_CANCEL_USSI_FROM, from.c_str());

    return Void();
}

Return<void> RadioImpl::setEmergencyCallConfig(int serial, int category, bool isForceEcc) {
    mtkLogD(LOG_TAG, "setEmergencyCallConfig: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_EMERGENCY_CALL_CONFIG, 2,
                 category, (int)isForceEcc);
    return Void();
}

Return<void> RadioImpl::getDisable2G(int serial) {
    mtkLogD(LOG_TAG, "getDisable2G: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_GET_DISABLE_2G);
    return Void();
}

Return<void> RadioImpl::setDisable2G(int serial, bool mode) {
    mtkLogD(LOG_TAG, "setDisable2G: serial %d", serial);
    dispatchInts(serial, mSlotId, RIL_REQUEST_SET_DISABLE_2G, 1, BOOL_TO_INT(mode));
    return Void();
}

Return<void> RadioImpl::deviceSwitch(int serial,
                                     const ::android::hardware::hidl_string& number,
                                     const ::android::hardware::hidl_string& deviceId) {
    mtkLogD(LOG_TAG, "deviceSwitch: serial %d", serial);
    dispatchStrings(serial, mSlotId, RIL_REQUEST_DEVICE_SWITCH, false, 2,
                    number.c_str(), deviceId.c_str());
    return Void();
}

Return<void> RadioImpl::cancelDeviceSwitch(int serial) {
    mtkLogD(LOG_TAG, "cancelDeviceSwitch: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_CANCEL_DEVICE_SWITCH);
    return Void();
}

Return<void> RadioImpl::exitSCBM(int32_t serial) {
    mtkLogD(LOG_TAG, "exitSCBM: serial %d", serial);
    dispatchVoid(serial, mSlotId, RIL_REQUEST_EXIT_SCBM);
    return Void();
}

Return<void> RadioImpl::sendRsuRequest(int serial, const RsuRequestInfo& rri) {
    mtkLogD(LOG_TAG, "sendRsuRequest");
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId, RIL_REQUEST_SEND_RSU_REQUEST);
    if (pRI == NULL) {
        return Void();
    }

    RIL_RsuRequestInfo r_rri = {};
    r_rri.opId = (int)rri.opId;
    r_rri.requestId = (int)rri.requestId;
    r_rri.requestType = (int)rri.requestType;
    r_rri.reserveInt1 = (int)rri.reserveInt1;
    r_rri.reserveInt2 = (int)rri.reserveInt2;

    if (!copyHidlStringToRil(&r_rri.data, rri.data, pRI, true)) {
        return Void();
    }
    if (!copyHidlStringToRil(&r_rri.reserveString1, rri.reserveString1, pRI, true)) {
        memsetAndFreeStrings(1, r_rri.data);
        return Void();
    }

    s_vendorFunctions->onRequest(RIL_REQUEST_SEND_RSU_REQUEST, &r_rri, sizeof(r_rri), pRI, pRI->socket_id);

    memsetAndFreeStrings(2, r_rri.data, r_rri.reserveString1);

    return Void();
}

/*********************************************************************************/
/*  Response of Vendor Solicated Command                                         */
/*********************************************************************************/
int radio::setDigitsLineResponse(int slotId, android::ClientId clientId,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responselen) {

    mtkLogD(LOG_TAG, "setDigitsLineResponse: serial %d", serial);
    if (radioService[slotId]->mRadioResponseDigits != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->
                                 mRadioResponseDigits->
                                 setDigitsLineResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "setDigitsLineResponse: radioService[%d]->mRadioResponseDigits == NULL",
                                                                              slotId);
    }

    return 0;
}

int radio::setRxTestConfigResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    mtkLogD(LOG_TAG, "setRxTestConfigResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<int32_t> respAntConf;
        int numInts = responseLen / sizeof(int);
        if (response == NULL || responseLen % sizeof(int) != 0) {
            mtkLogE(LOG_TAG, "setRxTestConfigResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            respAntConf.resize(numInts);
            for (int i = 0; i < numInts; i++) {
                respAntConf[i] = (int32_t) pInt[i];
            }
        }
        Return<void> retStatus
                = radioService[slotId]->mRadioResponseMtk->setRxTestConfigResponse(responseInfo,
                respAntConf);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "setRxTestConfigResponse: radioService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }

    return 0;
}

int radio::getRxTestResultResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    mtkLogD(LOG_TAG, "getRxTestResultResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        hidl_vec<int32_t> respAntInfo;
        int numInts = responseLen / sizeof(int);
        if (response == NULL || responseLen % sizeof(int) != 0) {
            mtkLogE(LOG_TAG, "getRxTestResultResponse Invalid response: NULL");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            int *pInt = (int *) response;
            respAntInfo.resize(numInts);
            for (int i = 0; i < numInts; i++) {
                respAntInfo[i] = (int32_t) pInt[i];
            }
        }
        Return<void> retStatus
                = radioService[slotId]->mRadioResponseMtk->getRxTestResultResponse(responseInfo,
                respAntInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "getRxTestResultResponse: radioService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }

    return 0;
}

int radio::setTrnResponse(int slotId, android::ClientId clientId __unused,
                          int responseType, int serial, RIL_Errno e,
                          void *response, size_t responselen) {

    mtkLogD(LOG_TAG, "setTrnResponse: serial %d", serial);
    if (radioService[slotId]->mRadioResponseDigits != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->
                                 mRadioResponseDigits->
                                 setTrnResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "setTrnResponse: radioService[%d]->mRadioResponseDigits == NULL",
                                                                       slotId);
    }

    return 0;
}

int radio::setIncomingVirtualLineResponse(int slotId, android::ClientId clientId __unused,
                          int responseType, int serial, RIL_Errno e,
                          void *response, size_t responselen) {
    mtkLogD(LOG_TAG, "setIncomingVirtualLineResponse: serial %d", serial);
    if (radioService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->
                                 mRadioResponseMtk->
                                 setIncomingVirtualLineResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "setIncomingVirtualLineResponse: radioService[%d]->mRadioResponseMtk == NULL",
                                                                       slotId);
    }

    return 0;
}

int radio::dialFromResponse(int slotId, android::ClientId clientId __unused,
                int responseType, int serial, RIL_Errno e, void *response, size_t responselen) {
    mtkLogD(LOG_TAG, "dialFromResponse: serial %d", serial);
    return 0;
}

int radio::sendUssiFromResponse(int slotId, android::ClientId clientId __unused,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responselen) {

    mtkLogD(LOG_TAG, "sendUssiFromResponse: serial %d", serial);
    if (radioService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->
                                 mRadioResponseIms->
                                 sendUssiFromResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "sendUssiFromResponse: radioService[%d]->mRadioResponseIms == NULL",
                                                                        slotId);
    }

    return 0;
}

int radio::cancelUssiFromResponse(int slotId, android::ClientId clientId __unused,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responselen) {

    mtkLogD(LOG_TAG, "cancelUssiFromResponse: serial %d", serial);
    if (radioService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->
                                 mRadioResponseIms->
                                 cancelUssiFromResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "cancelUssiFromResponse: radioService[%d]->mRadioResponseIms == NULL",
                                                                        slotId);
    }

    return 0;
}

int radio::setEmergencyCallConfigResponse(int slotId, android::ClientId clientId __unused,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responselen) {

    mtkLogD(LOG_TAG, "setEmergencyCallConfigResponse: serial %d", serial);
    if (radioService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->
                                 mRadioResponseIms->
                                 setEmergencyCallConfigResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "setEmergencyCallConfigResponse: radioService[%d]->mRadioResponseIms == NULL",
                                                                        slotId);
    }

    return 0;
}

int radio::setDisable2GResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    mtkLogD(LOG_TAG, "setDisable2GResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponseMtk->setDisable2GResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "setDisable2GResponse: radioService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }
    return 0;
}

int radio::getDisable2GResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen) {
    mtkLogD(LOG_TAG, "getDisable2GResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        int ret = responseInt(responseInfo, serial, responseType, e, response, responseLen);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponseMtk->getDisable2GResponse(responseInfo, ret);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "getDisable2GResponse: radioService[%d]->mRadioResponseMtk == NULL",
                slotId);
    }
    return 0;
}

int radio::deviceSwitchResponse(int slotId, android::ClientId clientId __unused,
                        int responseType, int serial, RIL_Errno e,
                        void *response, size_t responselen) {

    mtkLogD(LOG_TAG, "deviceSwtichResponse: serial %d", serial);
    if (radioService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->
                                 mRadioResponseIms->
                                 deviceSwitchResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "deviceSwtichResponse: radioService[%d]->mRadioResponseIms == NULL",
                                                                        slotId);
    }

    return 0;
}

int radio::cancelDeviceSwitchResponse(int slotId, android::ClientId clientId __unused,
                        int responseType, int serial, RIL_Errno e,
                        void *response, size_t responselen) {

    mtkLogD(LOG_TAG, "cancelDeviceSwtichResponse: serial %d", serial);
    if (radioService[slotId]->mRadioResponseIms != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus = radioService[slotId]->
                                 mRadioResponseIms->
                                 cancelDeviceSwitchResponse(responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "cancelDeviceSwtichResponse: radioService[%d]->mRadioResponseIms == NULL",
                                                                        slotId);
    }

    return 0;
}

int radio::setDigitsRegStatuseResponse(int slotId, android::ClientId clientId __unused,
                          int responseType, int serial, RIL_Errno e,
                          void *response, size_t responselen) {

    mtkLogD(LOG_TAG, "setDigitsRegStatuseResponse: serial %d", serial);
    return 0;
}

int radio::exitSCBMResponse(int slotId, android::ClientId clientId __unused,
                        int responseType, int serial, RIL_Errno e,
                        void *response, size_t responseLen) {
    mtkLogD(LOG_TAG, "exitSCBMResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
                = radioService[slotId]->mRadioResponseMtk->exitSCBMResponse(
                responseInfo);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "exitSCBMResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::sendRsuRequestResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    mtkLogD(LOG_TAG, "sendRsuRequestResponse: serial %d", serial);

    if (radioService[slotId]->mRadioResponseMtk != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        RsuResponseInfo result = {};
        if (response == NULL || responseLen != sizeof(RIL_RsuResponseInfo)) {
            mtkLogE(LOG_TAG, "sendRsuRequestResponse: response == NULL");
        } else {
            result.opId = ((RIL_RsuResponseInfo*)(response))->opId;
            result.requestId = ((RIL_RsuResponseInfo*)(response))->requestId;
            result.errCode = ((RIL_RsuResponseInfo*)(response))->errCode;
            result.data = convertCharPtrToHidlString(((RIL_RsuResponseInfo*)(response))->data);
            result.time = ((RIL_RsuResponseInfo*)(response))->time;
            result.version = ((RIL_RsuResponseInfo*)(response))->version;
            result.status = ((RIL_RsuResponseInfo*)(response))->status;
            result.reserveInt1 = ((RIL_RsuResponseInfo*)(response))->reserveInt1;
            result.reserveInt2 = ((RIL_RsuResponseInfo*)(response))->reserveInt2;
            result.reserveString1 =
                    convertCharPtrToHidlString(((RIL_RsuResponseInfo*)(response))->reserveString1);
        }

        Return<void> retStatus = radioService[slotId]->mRadioResponseMtk->
                        sendRsuRequestResponse(responseInfo, result);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "handleRsuUpdateLockDataResponse: radioService[%d]->mRadioResponse == NULL",
                slotId);
    }

    return 0;
}

int radio::switchRcsRoiStatusResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    mtkLogD(LOG_TAG, "switchRcsRoiStatusResponse: serial:%d, slotId:%d, imsSlotId:%d",
            serial, slotId, imsSlotId);
    if (radioService[imsSlotId]->mRadioResponseRcs != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
            = radioService[imsSlotId]->mRadioResponseRcs->switchRcsRoiStatusResponse(responseInfo);
        radioService[imsSlotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "switchRcsRoiStatusResponse:radioService[%d]->mRadioResponseRcs NULL",
                imsSlotId);
    }
    return 0;
}

int radio::updateRcsCapabilitiesResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    mtkLogD(LOG_TAG, "updateRcsCapabilitiesResponse: serial:%d, slotId:%d, imsSlotId:%d",
            serial, slotId, imsSlotId);
    if (radioService[imsSlotId]->mRadioResponseRcs != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
            = radioService[imsSlotId]->mRadioResponseRcs
                ->updateRcsCapabilitiesResponse(responseInfo);
        radioService[imsSlotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "updateRcsCapabilitiesResponse:radioService[%d]->mRadioResponseRcs NULL",
                imsSlotId);
    }
    return 0;
}

int radio::updateRcsSessionInfoResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen) {
    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    mtkLogD(LOG_TAG, "updateRcsSessionInfoResponse: serial:%d, slotId:%d, imsSlotId:%d",
            serial, slotId, imsSlotId);
    if (radioService[imsSlotId]->mRadioResponseRcs != NULL) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);
        Return<void> retStatus
            = radioService[imsSlotId]->mRadioResponseRcs->updateRcsSessionInfoResponse(responseInfo);
        radioService[imsSlotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "updateRcsSessionInfoResponse:radioService[%d]->mRadioResponseRcs NULL",
                imsSlotId);
    }
    return 0;
}

/*********************************************************************************/
/*  Vendor Unsolicated Command                                                   */
/*********************************************************************************/
int radio::digitsLineIndicationInd(int slotId,
                                   int indicationType, int token, RIL_Errno e,
                                   void *response, size_t responselen) {
    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (radioService[imsSlotId] != NULL
            && radioService[imsSlotId]->mRadioIndicationDigits != NULL) {

        char **resp = (char **) response;
        int numStrings = responselen / sizeof(char *);
        if(numStrings < 23) {
            mtkLogE(LOG_TAG, "digitsLineIndicationInd: items length invalid, slotId = %d",
                                                              imsSlotId);
            return 0;
        }

        int accountId = atoi(resp[0]);
        int serial = atoi(resp[1]);
        int msisdnNum = atoi(resp[2]);

        hidl_vec<hidl_string> msisdns;
        msisdns.resize(10);

        hidl_vec<bool> isActive;
        isActive.resize(10);

        int index, respIdx = 3;
        for (index = 0; index < 10; index ++) {
            msisdns[index] = convertCharPtrToHidlString(resp[respIdx ++]);
            isActive[index] = atoi(resp[respIdx ++]);
        }

        Return<void> retStatus = radioService[imsSlotId]->
                                 mRadioIndicationDigits->digitsLineIndication(
                                 convertIntToRadioIndicationType(indicationType),
                                 accountId, serial, msisdnNum, msisdns, isActive);

        radioService[imsSlotId]->checkReturnStatus(retStatus);
    }
    else {
        mtkLogE(LOG_TAG, "digitsLineIndicationInd: radioService[%d]->mRadioIndicationDigits == NULL",
                                                                               imsSlotId);
    }

    return 0;
}

int radio::getTrnIndicationInd(int slotId,
                               int indicationType, int token, RIL_Errno e,
                               void *response, size_t responselen) {
    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (radioService[imsSlotId] != NULL
            && radioService[imsSlotId]->mRadioIndicationDigits != NULL) {

        char **resp = (char **) response;
        int numStrings = responselen / sizeof(char *);
        if(numStrings < 2) {
            mtkLogE(LOG_TAG, "getTrnIndicationInd: items length invalid, slotId = %d",
                                                              imsSlotId);
            return 0;
        }

        hidl_string fromMsisdn = convertCharPtrToHidlString(resp[0]);
        hidl_string toMsisdn = convertCharPtrToHidlString(resp[1]);


        Return<void> retStatus = radioService[imsSlotId]->
                                 mRadioIndicationDigits->getTrnIndication(
                                 convertIntToRadioIndicationType(indicationType),
                                 fromMsisdn, toMsisdn);

        radioService[imsSlotId]->checkReturnStatus(retStatus);
    }
    else {
        mtkLogE(LOG_TAG, "getTrnIndicationInd: radioService[%d]->mRadioIndicationDigits == NULL",
                                                                          imsSlotId);
    }

    return 0;
}

int radio::responseModulationInfoInd(int slotId,
                                           int indicationType, int token, RIL_Errno e,
                                           void *response, size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndicationMtk != NULL) {
        if (response == NULL || responseLen % sizeof(char *) != 0) {
            mtkLogE(LOG_TAG, "responseModulationInfoInd Invalid response: NULL");
            return 0;
        }
        mtkLogD(LOG_TAG, "responseModulationInfoInd");
        hidl_vec<int32_t> data;
        int *pInt = (int *) response;
        int numInts = responseLen / sizeof(int);
        data.resize(numInts);
        for (int i = 0; i < numInts; i++) {
            data[i] = (int32_t) pInt[i];
        }
        Return<void> retStatus = radioService[slotId]->mRadioIndicationMtk->responseModulationInfoInd(
                convertIntToRadioIndicationType(indicationType), data);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "responseModulationInfoInd: radioService[%d]->responseModulationInfoInd == NULL", slotId);
    }
    return 0;
}

extern "C" void radio::registerOpService(RIL_RadioFunctions *callbacks,
                                         android::CommandInfo *commands) {

    using namespace android::hardware;
    int simCount = 1;
    const char *serviceNames[] = {
            android::RIL_getServiceName()
            , RIL2_SERVICE_NAME
            , RIL3_SERVICE_NAME
            , RIL4_SERVICE_NAME
            };
    const char *imsServiceNames[] = {
            IMS_WWOP_RIL1_SERVICE_NAME
            , IMS_WWOP_RIL2_SERVICE_NAME
            , IMS_WWOP_RIL3_SERVICE_NAME
            , IMS_WWOP_RIL4_SERVICE_NAME
            };

    simCount = getSimCount();
    configureRpcThreadpool(1, true /* callerWillJoin */);
    for (int i = 0; i < simCount; i++) {
        pthread_rwlock_t *radioServiceRwlockPtr = getRadioServiceRwlock(i);
        lockRadioServiceWlock(radioServiceRwlockPtr, i);

        radioService[i] = new RadioImpl;
        radioService[i]->mSlotId = i;
        mtkLogI(LOG_TAG, "registerOpService: starting IRadioOp %s", serviceNames[i]);
        android::status_t status = radioService[i]->registerAsService(serviceNames[i]);

        int imsSlot = i + (android::CLIENT_IMS * MAX_SIM_COUNT);
        radioService[imsSlot] = new RadioImpl;
        radioService[imsSlot]->mSlotId = imsSlot;
        mtkLogD(LOG_TAG, "radio::registerService: starting IMS IRadioOp %s, slot = %d, realSlot = %d",
              imsServiceNames[i], radioService[imsSlot]->mSlotId, imsSlot);

        // Register IMS Radio Stub
        status = radioService[imsSlot]->registerAsService(imsServiceNames[i]);
        mtkLogD(LOG_TAG, "radio::registerService IRadio for IMS status:%d", status);
        unlockRadioServiceWlock(radioServiceRwlockPtr, i);
    }

    s_vendorFunctions = callbacks;
    s_commands = commands;
}

// MTK-START: SIM TMO RSU
int radio::onSimMeLockEvent(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndicationMtk != NULL) {
        int32_t eventId = ((int32_t *) response)[0];

        mtkLogD(LOG_TAG, "onSimMeLockEvent: eventId %d", eventId);
        Return<void> retStatus = radioService[slotId]->mRadioIndicationMtk->
                onSimMeLockEvent(convertIntToRadioIndicationType(indicationType), eventId);
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "onSimMeLockEvent: radioService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}
// MTK-END

int radio::digitsLineIndication(int slotId,
                               int indicationType, int token, RIL_Errno e,
                               void *response, size_t responselen) {
    int imsSlotId = toClientSlot(slotId, android::CLIENT_IMS);
    if (radioService[imsSlotId] != NULL
            && radioService[imsSlotId]->mRadioIndicationRcs != NULL) {

        char *resp = (char *) response;
        int numStrings = responselen / sizeof(int);
        if(numStrings < 1) {
            mtkLogE(LOG_TAG, "digitsLineIndication: items length invalid, slotId = %d",imsSlotId);
            return 0;
        }

        mtkLogD(LOG_TAG, "digitsLineIndication: %s", resp);
        hidl_string cmdline = convertCharPtrToHidlString(resp);

        Return<void> retStatus = radioService[imsSlotId]->
                                 mRadioIndicationRcs->digitsLineIndication(
                                 convertIntToRadioIndicationType(indicationType),
                                 cmdline);

        radioService[imsSlotId]->checkReturnStatus(retStatus);

    } else {
        mtkLogE(LOG_TAG, "digitsLineIndication: radioService[%d]->mRadioIndicationIms == NULL", imsSlotId);
    }

    return 0;
}

int radio::enterSCBMInd(int slotId,
                     int indicationType, int token, RIL_Errno e, void *response,
                     size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndicationMtk != NULL) {
        mtkLogD(LOG_TAG, "enterSCBMInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndicationMtk->enterSCBMInd(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "enterSCBMInd: radioService[%d]->mRadioIndicationMtk == NULL",
                slotId);
    }

    return 0;
}

int radio::exitSCBMInd(int slotId,
                     int indicationType, int token, RIL_Errno e, void *response,
                     size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndicationMtk != NULL) {
        mtkLogD(LOG_TAG, "exitSCBMInd");
        Return<void> retStatus = radioService[slotId]->mRadioIndicationMtk->exitSCBMInd(
                convertIntToRadioIndicationType(indicationType));
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "exitSCBMInd: radioService[%d]->mRadioIndicationMtk == NULL",
                slotId);
    }

    return 0;
}

int radio::onRsuEvent(int slotId, int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen) {
    if (radioService[slotId] != NULL && radioService[slotId]->mRadioIndicationMtk != NULL) {
        mtkLogD(LOG_TAG, "onRsuEvent");
        char** strings = (char**)response;
        Return<void> retStatus = radioService[slotId]->mRadioIndicationMtk->onRsuEvent(
                convertIntToRadioIndicationType(indicationType),
                atoi(strings[0]), convertCharPtrToHidlString(strings[1]));
        radioService[slotId]->checkReturnStatus(retStatus);
    } else {
        mtkLogE(LOG_TAG, "onRsuEvent: radioService[%d]->mRadioIndicationMtk == NULL", slotId);
    }

    return 0;
}
