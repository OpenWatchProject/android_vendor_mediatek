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

#define LOG_TAG "MtkUiccSEHal"
#include <log/log.h>

#include "selog.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define SUPPORT_BASIC_CHANNEL 0 /* MTK not support */

#define MAX_INSTANCE (4)
#define SLOT_NAME_LEN (12)

#define SE_RIL_SERVICE_NAME_BASE "se"
#define MTK_SE_RIL_SERVICE_NAME_BASE "mtkSe"

namespace android {
namespace hardware {
namespace secure_element {
namespace V1_0 {
namespace implementation {

SecureElement::SecureElement(const int& id) {
    mCallback_V1_0 = nullptr;
    mSERadioRspHandler = nullptr;
    mMtkRadioDeathRecipient = nullptr;
    mRadioDeathRecipient = nullptr;

    mMtkRadio = nullptr;
    mMtkRadioRsp = nullptr;
    mMtkRadioInd = nullptr;
    mRadio = nullptr;
    mRadioRsp = nullptr;
    mRadioInd = nullptr;

    /* Add sim state change event */
    mSimPreviousState = false;

    mMainMutex = PTHREAD_MUTEX_INITIALIZER; /* lock for request/response */
    mRadioMutex = PTHREAD_MUTEX_INITIALIZER; /* lock for radio service */
    mCallbackMutex = PTHREAD_MUTEX_INITIALIZER; /* lock for SE callback */
    mRespMutex = PTHREAD_MUTEX_INITIALIZER; /* lock for waiting response */
    mRespCond = PTHREAD_COND_INITIALIZER;

    if (id <= 0 || id > MAX_INSTANCE) {
        mUiccId = -1;
        mSlotName.clear();

        SELOGE("not support UICC%d/SIM%d", id, id);
    } else {
        mUiccId = id;

        char name[SLOT_NAME_LEN];
        snprintf(name, SLOT_NAME_LEN, SE_RIL_SERVICE_NAME_BASE"%d", mUiccId);
        mSlotName = name;

        SELOGD("created for UICC%d/SIM%d", mUiccId, mUiccId);

        if (IS_DEBUG && WAIT_TIMEOUT_MS <= 0)
            SELOGV("no timeout");
    }

    mSerial = 0;
    mCardState = AOSP_V1_0::CardState::ABSENT;

    mTimerId = 0;
    mTimerValid = false;
}

SecureElement::~SecureElement() {

    SELOGD("destroy for UICC%d/SIM%d", mUiccId, mUiccId);
    if (mUiccId != -1)
        reset(true, true, "destroy");
}

void SecureElement::notifyResponse() {

    pthread_mutex_lock(&mRespMutex);
    pthread_cond_signal(&mRespCond); /* notify */
    pthread_mutex_unlock(&mRespMutex);
}

void SecureElement::radioServiceDied() {
    resetRadio("ERROR: radio service died");

    pthread_t thread;
    int created = pthread_create(&thread, NULL, runMainThread, this);
    if (created != 0)
        SELOGE("Create reconnet thread failed");
}

void SecureElement::respTimeout(union sigval sv) {
    sp<SecureElement> se = (SecureElement *)sv.sival_ptr;
    se->resetRadio("ERROR: response timeout");
}

// Methods from ::android::hardware::secure_element::V1_0::ISecureElement follow.
Return<void> SecureElement::init(const sp<::android::hardware::secure_element::V1_0::ISecureElementHalCallback>& clientCallback) {
    SELOGD("MtkUicc: 2.1.13");

    if (clientCallback == nullptr) {
        SELOGE("clientCallback == nullptr");
        return Void();
    } else {
        resetAll("INIT");

        pthread_mutex_lock(&mCallbackMutex);
        mCallback_V1_0 = clientCallback;
        if (mCallback_V1_0 != nullptr) {
            auto ret = mCallback_V1_0->linkToDeath(this, 0);
            // be checked for transport errors by calling its isOk() method
            if (!ret.isOk() || ret == false) {
                SELOGE("failed to register death notification");
                mCallback_V1_0 = nullptr;
                pthread_mutex_unlock(&mCallbackMutex);

                return Void();
            }
        }
        pthread_mutex_unlock(&mCallbackMutex);
    }

    pthread_t thread;
    int created = pthread_create(&thread, NULL, runCallbackThread, this);
    if (created != 0)
        SELOGE("Create callback thread failed");

    SELOGD("init done");
    return Void();
}

void SecureElement::handleCallbackThread(bool force) {
    bool present = isCardPresentInternal();
    /* Add sim state change event */
    if (force || mSimPreviousState != present) {
        pthread_mutex_lock(&mCallbackMutex);
        SELOGD("onStateChange %d", present);
        if(mCallback_V1_0 != nullptr) {
            /* SecureElementService may update access rules in this callback */
            auto ret = mCallback_V1_0->onStateChange(present);
            if (!ret.isOk()) // be checked for transport errors by calling its isOk() method
                SELOGE("SE service died");
            SELOGD("onStateChange %d done", present);
        }
        else {
            SELOGE("SE service was dead");
        }
        pthread_mutex_unlock(&mCallbackMutex);
    }
    mSimPreviousState = present;
    /* Add sim state change event */
}

Return<void> SecureElement::getAtr(getAtr_cb _hidl_cb) {
    hidl_vec<uint8_t> respVec;
    struct SERadioResponseData respData;
    bool result = false;
    if(mIsReset) {
        SELOGD("System is reset");
        _hidl_cb(respVec);
        return Void();
    }

    if (!initRadioService()) {
        SELOGD("radio is not ready");

        _hidl_cb(respVec);
        return Void();
    }

    pthread_mutex_lock(&mMainMutex);

    int32_t serial = getSerialAndIncrementLocked();

    pthread_mutex_lock(&mRadioMutex);
    SELOGV("[%d] > getAtr", serial);
    if(mMtkRadio == nullptr) {
        pthread_mutex_unlock(&mRadioMutex);
        SELOGE("mMtkRadio is nullptr");

        _hidl_cb(respVec);
        pthread_mutex_unlock(&mMainMutex);
        return Void();
    }

    auto retM = mMtkRadio->getATR(serial);
    if (!retM.isOk()) { // be checked for transport errors by calling its isOk() method
        pthread_mutex_unlock(&mRadioMutex);
        SELOGE("radio service died");
        goto getAtr_done;
    }
    pthread_mutex_unlock(&mRadioMutex);

    result = waitResponseLocked(respData, SE_ICC_GET_ATR, serial);
    if (result == true) {
        hexStringToByteVector(respVec, respData.response);
        if (IS_DEBUG)
            SELOGV("[%d] < getAtr %s err=%d", serial, respData.response.c_str(),
                                                      respData.info.error);
        else
            SELOGV("[%d] < getAtr * err=%d", serial, respData.info.error);
    }

getAtr_done:
    _hidl_cb(respVec);

    pthread_mutex_unlock(&mMainMutex);

    return Void();
}

Return<bool> SecureElement::isCardPresent() {
    bool present = isCardPresentInternal();

    SELOGD("present = %d", present);

    return present;
}

Return<void> SecureElement::transmit(const hidl_vec<uint8_t>& data, transmit_cb _hidl_cb) {
    hidl_vec<uint8_t> respVec;

    if(mIsReset) {
        SELOGD("System is reset");
        _hidl_cb(respVec);
        return Void();
    }
    if (!initRadioService()) {
        SELOGD("radio is not ready");

        _hidl_cb(respVec);
        return Void();
    }

    hidl_string dataString;
    byteVectorToHexString(dataString, data);

    if (IS_DEBUG)
        SELOGV("transmit %s", dataString.c_str());

    pthread_mutex_lock(&mMainMutex);

    if (transmitInternalLocked(respVec, data) == false) {
        respVec.resize(0);
    } else {
        if (respVec.size() >= 2) {
            uint8_t sw1 = respVec[respVec.size() - 2];
            uint8_t sw2 = respVec[respVec.size() - 1];
            if (sw1 == 0x6C) {
                if (reissueCommandLocked(respVec, data, sw2) == false)
                    respVec.resize(0);
            }
        }
    }

    _hidl_cb(respVec);

    pthread_mutex_unlock(&mMainMutex);

    return Void();
}

Return<void> SecureElement::openLogicalChannel(const hidl_vec<uint8_t>& aid, uint8_t p2,
                                               openLogicalChannel_cb _hidl_cb) {

    SecureElementStatus status = SecureElementStatus::IOERROR;
    struct SERadioResponseData respData;
    bool result = false;
    LogicalChannelResponse response;
    response.channelNumber = 0xFF;
    if(mIsReset) {
        SELOGD("System is reset");
        _hidl_cb(response, status);
        return Void();
    }

    if (!initRadioService()) {
        SELOGD("radio is not ready");

        _hidl_cb(response, status);
        return Void();
    }

    hidl_string aidString;
    byteVectorToHexString(aidString, aid);

    pthread_mutex_lock(&mMainMutex);

    int32_t serial = getSerialAndIncrementLocked();

    pthread_mutex_lock(&mRadioMutex);
    if (IS_DEBUG)
        SELOGV("[%d] > open logical %s p2=%u", serial, aidString.c_str(), p2);
    else
        SELOGV("[%d] > open logical *", serial);

    if(mRadio == nullptr) {
        pthread_mutex_unlock(&mRadioMutex);
        SELOGE("mRadio is nullptr");

        _hidl_cb(response, status);
        pthread_mutex_unlock(&mMainMutex);

        return Void();
    }

    auto ret = mRadio->iccOpenLogicalChannel(serial, aidString, (int32_t) p2);
    if (!ret.isOk()) { // be checked for transport errors by calling its isOk() method
        pthread_mutex_unlock(&mRadioMutex);
        SELOGE("radio service died");
        goto openLogicalChannel_done;
    }
    pthread_mutex_unlock(&mRadioMutex);

    result = waitResponseLocked(respData, SE_ICC_OPEN_CHANNEL, serial);
    if (result == true) {
        switch (respData.info.error) {
        case AOSP_V1_0::RadioError::MISSING_RESOURCE:
            status = SecureElementStatus::CHANNEL_NOT_AVAILABLE;
            break;
        case AOSP_V1_0::RadioError::NO_SUCH_ELEMENT:
            status = SecureElementStatus::NO_SUCH_ELEMENT_ERROR;
            break;
        case AOSP_V1_0::RadioError::NONE:
            status = SecureElementStatus::SUCCESS;
            break;
        default:
            break;
        }

        response.channelNumber = respData.channelId;
        response.selectResponse.resize(respData.selectResponse.size());
        for (size_t i = 0; i < respData.selectResponse.size(); ++i)
            response.selectResponse[i] = (uint8_t) respData.selectResponse[i];

        hidl_string respString;
        byteVectorToHexString(respString, response.selectResponse);

        uint8_t sw1 = 0;
        uint8_t sw2 = 0;
        /* If the SE returns a warning SW ({0x62, 0xXX} or {0x63, 0xXX})
         * without data, the API SHALL issue a GET RESPONSE with Le = 0x00
         */
        if (respData.selectResponse.size() == 2) {
            sw1 = response.selectResponse[response.selectResponse.size() - 2];
            sw2 = response.selectResponse[response.selectResponse.size() - 1];

            if (sw1 == 0x62 || sw1 == 0x63) {
                // GET RESPONSE
                hidl_vec<uint8_t> data;
                data.resize(5);
                data[0] = setChannelToClassByte(0x00, response.channelNumber);  /* CLA */
                data[1] = 0xC0;         /* INS */
                data[2] = 0x00;         /* P1 */
                data[3] = 0x00;         /* P2 */
                data[4] = 0x00;         /* Lc (P3) */

                hidl_vec<uint8_t> newRespVec;
                if (transmitInternalLocked(newRespVec, data) == true) {
                    if (newRespVec.size() >= 2) {
                        uint8_t resSW1 = newRespVec[newRespVec.size() - 2];
                        if (resSW1 == 0x90 || resSW1 == 0x61 || resSW1 == 0x62 || resSW1 == 0x63) {
                            response.selectResponse.resize(newRespVec.size());
                            size_t i = 0;
                            for (i = 0; i < newRespVec.size() - 2; ++i)
                                response.selectResponse[i] = newRespVec[i];
                            response.selectResponse[i] = sw1;
                            response.selectResponse[i + 1] = sw2;

                            respString.clear();
                            byteVectorToHexString(respString, response.selectResponse);
                        } else {
                            SELOGE("GET RESPONSE: unexpected SW1");
                        }
                    } else {
                        SELOGE("GET RESPONSE: length < 2");
                    }
                } else {
                    SELOGE("GET RESPONSE: transmit failed");
                }
            }
        }

        if (IS_DEBUG)
            SELOGV("[%d] < open logical [%d]%s err=%d status=%hhu", serial, response.channelNumber, 
                                                                    respString.c_str(),
                                                                    respData.info.error, status);
        else
            SELOGV("[%d] < open logical [%d] *%02X%02X (%zu) err=%d status=%hhu", serial, response.channelNumber, sw1, sw2, response.selectResponse.size(), respData.info.error, status);
    }

    if (status != SecureElementStatus::SUCCESS)
        response.selectResponse.resize(0);

openLogicalChannel_done:
    _hidl_cb(response, status);

    pthread_mutex_unlock(&mMainMutex);

    return Void();
}

Return<void> SecureElement::openBasicChannel(const hidl_vec<uint8_t>& aid, uint8_t p2, openBasicChannel_cb _hidl_cb) {
#if SUPPORT_BASIC_CHANNEL
    hidl_vec<uint8_t> respVec;
    SecureElementStatus status = SecureElementStatus::IOERROR;

    if(mIsReset) {
        SELOGD("System is reset");
        _hidl_cb(respVec, status);
        return Void();
    }
    if (!initRadioService()) {
        SELOGD("radio is not ready");

        _hidl_cb(respVec, status);
        return Void();
    }

    hidl_vec<uint8_t> data;
    data.resize(aid.size() + 4);
    data[0] = 0x00;                 /* basic channel */
    data[1] = 0xA4;                 /* INS */
    data[2] = 0x04;                 /* P1 */
    data[3] = p2;                   /* P2 */
    data[4] = (uint8_t) aid.size(); /* Lc (P3) */
    for (size_t i = 0; i < aid.size(); ++i)
        data[5 + i] = aid[i];

    hidl_string aidString;
    byteVectorToHexString(aidString, aid);

    if (IS_DEBUG)
        SELOGV("open basic %s p2=%u", aidString.c_str(), p2);
    else
        SELOGV("open basic *");

    pthread_mutex_lock(&mMainMutex);

    if (transmitInternalLocked(respVec, data) == true) {
        if (respVec.size() >= 2) {
            uint8_t sw1 = respVec[respVec.size() - 2];
            uint8_t sw2 = respVec[respVec.size() - 1];
            if (sw1 == 0x90 && sw2 == 0x00)
                status = SecureElementStatus::SUCCESS;
            else if ((sw1 == 0x6A && sw2 == 0x81) ||
                    (sw1 == 0x6A && sw2 == 0x84))
                status = SecureElementStatus::CHANNEL_NOT_AVAILABLE;
            else if ((sw1 == 0x6A && sw2 == 0x82) ||
                    (sw1 == 0x69 && sw2 == 0x85) ||
                    (sw1 == 0x69 && sw2 == 0x99))
                status = SecureElementStatus::NO_SUCH_ELEMENT_ERROR;
            else if (sw1 == 0x6A && sw2 == 0x86)
                status = SecureElementStatus::UNSUPPORTED_OPERATION;
        }
    }

    _hidl_cb(respVec, status);

    pthread_mutex_unlock(&mMainMutex);

#else /* SUPPORT_BASIC_CHANNEL */

    hidl_string aidString;
    byteVectorToHexString(aidString, aid);
    if (IS_DEBUG)
        SELOGV("not support open basic %s p2=%u", aidString.c_str(), p2);
    else
        SELOGV("not support open basic *");

    hidl_vec<uint8_t> respVec;
    _hidl_cb(respVec, SecureElementStatus::CHANNEL_NOT_AVAILABLE);

#endif /* SUPPORT_BASIC_CHANNEL */

    return Void();
}

Return<::android::hardware::secure_element::V1_0::SecureElementStatus> SecureElement::closeChannel(uint8_t channelNumber) {
    SecureElementStatus status = SecureElementStatus::FAILED;
    struct SERadioResponseData respData;
    bool result = false;
    if(mIsReset) {
        SELOGD("System is reset");
        return status;
    }

    if (!initRadioService()) {
        SELOGD("radio is not ready");
        return status;
    }

    pthread_mutex_lock(&mMainMutex);

    int32_t serial = getSerialAndIncrementLocked();

    pthread_mutex_lock(&mRadioMutex);
    SELOGV("[%d] > close [%u]", serial, channelNumber);

    if(mRadio == nullptr) {
        pthread_mutex_unlock(&mRadioMutex);
        SELOGE("mRadio is nullptr");

        pthread_mutex_unlock(&mMainMutex);
        return status;
    }

    auto ret = mRadio->iccCloseLogicalChannel(serial, (int32_t) channelNumber);
    if (!ret.isOk()) { // be checked for transport errors by calling its isOk() method
        pthread_mutex_unlock(&mRadioMutex);
        SELOGE("radio service died");
        goto closeChannel_done;
    }
    pthread_mutex_unlock(&mRadioMutex);

    result = waitResponseLocked(respData, SE_ICC_CLOSE_CHANNEL, serial);
    if (result == true) {
        if (respData.info.error == AOSP_V1_0::RadioError::NONE)
            status = SecureElementStatus::SUCCESS;

        SELOGV("[%d] < close [%u] err=%d", serial, channelNumber, respData.info.error);
    }

closeChannel_done:
    pthread_mutex_unlock(&mMainMutex);

    return status;
}


// Methods from ::android::hidl::hardware::hidl_death_recipient follow.
void SecureElement::serviceDied(uint64_t /* cookie */, const ::android::wp<::android::hidl::base::V1_0::IBase>& /* who */) {
    resetAll("ERROR: SE service died");
}


// Private methods follow.
bool SecureElement::transmitInternalLocked(hidl_vec<uint8_t>& respVec, const hidl_vec<uint8_t>& data) {
    /* Must get mMainMutex before use this function. */
    bool result = false;
    int32_t serial = -1;
    enum SERadioResponseType type;
    struct SERadioResponseData respData;

    hidl_string dataString;
    byteVectorToHexString(dataString, data);

    AOSP_V1_0::SimApdu apdu;
    if (createSimApdu(apdu, data) == false) {
        goto transmitInternalLocked_done;
    }

    serial = getSerialAndIncrementLocked();

    pthread_mutex_lock(&mRadioMutex);
    if (IS_DEBUG)
        SELOGV("[%d] > transmit [%d] %s", serial, apdu.sessionId, dataString.c_str());
    else
        SELOGV("[%d] > transmit [%d] *", serial, apdu.sessionId);
    if (apdu.sessionId != 0) {
        if(mRadio == nullptr) {
            pthread_mutex_unlock(&mRadioMutex);
            SELOGE("mRadio is nullptr");
            return result;
        }

        auto ret = mRadio->iccTransmitApduLogicalChannel(serial, apdu);
        if (!ret.isOk()) { // be checked for transport errors by calling its isOk() method
            pthread_mutex_unlock(&mRadioMutex);
            SELOGE("radio service died");
            goto transmitInternalLocked_done;
        }
    } else {
        if(mRadio == nullptr) {
            pthread_mutex_unlock(&mRadioMutex);
            SELOGE("mRadio is nullptr");
            return result;
        }

        auto ret = mRadio->iccTransmitApduBasicChannel(serial, apdu);
        if (!ret.isOk()) { // be checked for transport errors by calling its isOk() method
            pthread_mutex_unlock(&mRadioMutex);
            SELOGE("radio service died");
            goto transmitInternalLocked_done;
        }
    }
    pthread_mutex_unlock(&mRadioMutex);

    if (apdu.sessionId != 0)
        type = SE_ICC_TRANSMIT_APDU_LOGICAL;
    else
        type = SE_ICC_TRANSMIT_APDU_BASIC;

    result = waitResponseLocked(respData, type, serial);
    if (result == true) {
        iccIoResultToByteVector(respVec, respData.result);

        hidl_string respString;
        byteVectorToHexString(respString, respVec);
        if (IS_DEBUG) {
            SELOGV("[%d] < transmit [%d] %s err=%d", serial, apdu.sessionId, respString.c_str(), respData.info.error);
        } else {
            uint8_t sw1 = 0;
            uint8_t sw2 = 0;
            if (respVec.size() >= 2) {
                sw1 = respVec[respVec.size() - 2];
                sw2 = respVec[respVec.size() - 1];
            }
            SELOGV("[%d] < transmit [%d] *%02X%02X (%zu) err=%d", serial, apdu.sessionId, sw1, sw2, respString.size(), respData.info.error);
        }

        if (respData.info.error != AOSP_V1_0::RadioError::NONE)
            result = false;
    }

transmitInternalLocked_done:
    return result;
}

bool SecureElement::getIccCardStatusLocked() {
    /* Must get mMainMutex before use this function. */
    bool result = false;
    struct SERadioResponseData respData;
    int32_t serial = getSerialAndIncrementLocked();

    pthread_mutex_lock(&mRadioMutex);
    SELOGV("[%d] > getIccCardStatus", serial);
    if(mRadio == nullptr) {
        pthread_mutex_unlock(&mRadioMutex);
        SELOGE("mRadio is nullptr");
        return result;
    }

    auto ret = mRadio->getIccCardStatus(serial);
    if (!ret.isOk()) { // be checked for transport errors by calling its isOk() method
        pthread_mutex_unlock(&mRadioMutex);
        SELOGE("radio service died");
        goto getIccCardStatusLocked_done;
    }
    pthread_mutex_unlock(&mRadioMutex);

    result = waitResponseLocked(respData, SE_ICC_CARD_STATUS, serial);
    if (result == true) {
        mCardState = respData.cardStatus.cardState;

        SELOGV("[%d] < getIccCardStatus %d err=%d", serial, mCardState, respData.info.error);
    }

getIccCardStatusLocked_done:
    SELOGV("[%d] < getIccCardStatus", serial);
    return result;
}

bool SecureElement::isCardPresentInternal() {
    if (!initRadioService()) {
        SELOGD("radio is not ready");
        return false;
    }

    pthread_mutex_lock(&mMainMutex);

    getIccCardStatusLocked();
    bool present = (mCardState == AOSP_V1_0::CardState::PRESENT);

    pthread_mutex_unlock(&mMainMutex);

    return present;
}

bool SecureElement::initRadioService() {
    pthread_mutex_lock(&mRadioMutex);
    if (isRadioAliveLocked() == false) {
        /* MTK IRadio */
        char mtkname[SLOT_NAME_LEN] = {0};
        snprintf(mtkname, SLOT_NAME_LEN, MTK_SE_RIL_SERVICE_NAME_BASE"%d", mUiccId);
        mMtkRadio = VENDOR_V1_0::IMtkRadioEx::tryGetService(mtkname);
        if (mMtkRadio == nullptr) {
            SELOGE("get MTK IRadio failed");

            pthread_mutex_unlock(&mRadioMutex);
            return false;
        }

        /* MTK DeathRecipient */
        mMtkRadioDeathRecipient = new RadioDeathRecipient(this);
        if (mMtkRadioDeathRecipient == nullptr) {
            SELOGE("create MTK RadioDeathRecipient failed");
            mMtkRadio = nullptr;

            pthread_mutex_unlock(&mRadioMutex);
            return false;
        }
        auto retM = mMtkRadio->linkToDeath(mMtkRadioDeathRecipient, 0);
        if (!retM.isOk()) { // be checked for transport errors by calling its isOk() method
            SELOGE("MTK radio service died");
            mMtkRadioDeathRecipient = nullptr;
            mMtkRadio = nullptr;

            pthread_mutex_unlock(&mRadioMutex);
            return false;
        }

        /* IRadio */
        char name[SLOT_NAME_LEN] = {0};
        snprintf(name, SLOT_NAME_LEN, SE_RIL_SERVICE_NAME_BASE"%d", mUiccId);
        SELOGE("tryGetService %s", name);
        mRadio = AOSP_V1_4::IRadio::tryGetService(name);
        if (mRadio == nullptr) {
            SELOGE("get IRadio failed");
            mMtkRadioDeathRecipient = nullptr;
            mMtkRadio = nullptr;

            pthread_mutex_unlock(&mRadioMutex);
            return false;
        }

        /* DeathRecipient */
        mRadioDeathRecipient = new RadioDeathRecipient(this);
        if (mRadioDeathRecipient == nullptr) {
            SELOGE("create RadioDeathRecipient failed");
            mRadio = nullptr;
            mMtkRadioDeathRecipient = nullptr;
            mMtkRadio = nullptr;

            pthread_mutex_unlock(&mRadioMutex);
            return false;
        }
        auto ret = mRadio->linkToDeath(mRadioDeathRecipient, 0);
        if (!ret.isOk()) { // be checked for transport errors by calling its isOk() method
            SELOGE("MTK radio service died");
            mRadioDeathRecipient = nullptr;
            mRadio = nullptr;
            mMtkRadioDeathRecipient = nullptr;
            mMtkRadio = nullptr;

            pthread_mutex_unlock(&mRadioMutex);
            return false;
        }

        /* ResponseFunctions */
        if (initResponseFunctionsLocked() == false) {
            SELOGE("initResponseFunctions failed");

            auto ret = mRadio->unlinkToDeath(mRadioDeathRecipient);
            if (!ret.isOk()) // be checked for transport errors by calling its isOk() method
                SELOGE("radio service died");
            mRadioDeathRecipient = nullptr;
            mRadio = nullptr;

            auto retM = mMtkRadio->unlinkToDeath(mMtkRadioDeathRecipient);
            if (!retM.isOk()) // be checked for transport errors by calling its isOk() method
                SELOGE("MTK radio service died");
            mMtkRadioDeathRecipient = nullptr;
            mMtkRadio = nullptr;

            pthread_mutex_unlock(&mRadioMutex);
            return false;
        }
    }

    pthread_mutex_unlock(&mRadioMutex);

    return true;
}

bool SecureElement::initResponseFunctionsLocked() {
    /* Must get mRadioMutex before use this function. */
    mSERadioRspHandler = new SERadioResponseHandler(this, mSlotName);

    mMtkRadioRsp = new SERadioResponse(mSERadioRspHandler, mSlotName);
    mMtkRadioInd = new SERadioIndication(this, mSlotName);
    mRadioRsp = new RadioResponse(mSERadioRspHandler, mSlotName);
    mRadioInd = new RadioIndication(this, mSlotName);

    SELOGD("setResponseFunctionsSE (%p, %p)", mMtkRadioRsp.get(), mMtkRadioInd.get());
    auto retM = mMtkRadio->setResponseFunctionsSE(mMtkRadioRsp, mMtkRadioInd);
    if (!retM.isOk()) { // be checked for transport errors by calling its isOk() method
        SELOGE("MTK radio service died");
        mMtkRadioInd = nullptr;
        mMtkRadioRsp = nullptr;
        return false;
    }

    SELOGD("setResponseFunctions (%p, %p)", mRadioRsp.get(), mRadioInd.get());
    auto ret = mRadio->setResponseFunctions(mRadioRsp, mRadioInd);
    if (!ret.isOk()) { // be checked for transport errors by calling its isOk() method
        SELOGE("radio service died");
        mRadioInd = nullptr;
        mRadioRsp = nullptr;
        mMtkRadioInd = nullptr;
        mMtkRadioRsp = nullptr;
        return false;
    }

    return true;
}

void SecureElement::byteVectorToHexString(hidl_string& str, const hidl_vec<uint8_t>& vec, size_t offset, size_t length) {
    if (length == 0) {
        str.clear();
        return;
    }

    char buff[length * 2 + 1];
    memset(buff, 0, sizeof(buff));
    for (size_t i = 0; i < length; ++i) {
        snprintf(buff + i * 2, 3, "%02X", vec[i + offset]);
    }
    str = buff;
}

void SecureElement::byteVectorToHexString(hidl_string& str, const hidl_vec<uint8_t>& vec) {
    byteVectorToHexString(str, vec, 0, vec.size());
}

uint8_t SecureElement::hexCharToByte(const char& hex) {
    if (hex >= 'a')
        return hex - 'a' + 10;
    else if (hex >= 'A')
        return hex - 'A' + 10;

    return hex - '0';
}

void SecureElement::hexStringToByteVector(hidl_vec<uint8_t>& vec, const hidl_string& str, size_t offset, size_t length) {
    size_t vecSize = (length - offset) / 2;
    vec.resize(vecSize);
    for (size_t i = 0; i < vecSize; ++i) {
        char hex1 = str.c_str()[i * 2 + offset];
        char hex2 = str.c_str()[i * 2 + 1 + offset];
        vec[i] = hexCharToByte(hex1) * 16 + hexCharToByte(hex2);
    }
}

void SecureElement::hexStringToByteVector(hidl_vec<uint8_t>& vec, const hidl_string& str) {
    hexStringToByteVector(vec, str, 0, str.size());
}

void SecureElement::iccIoResultToByteVector(hidl_vec<uint8_t>& vec, const AOSP_V1_0::IccIoResult& result) {
    char buff[result.simResponse.size() + 4 + 1];
    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "%s%02X%02X", result.simResponse.c_str(), result.sw1, result.sw2);
    hidl_string str = buff;
    hexStringToByteVector(vec, str);
}

uint8_t SecureElement::setChannelToClassByte(const uint8_t& cla, int channelNumber) {
    uint8_t newCla = cla;
    if (channelNumber < 4) {
        // b7 = 0 indicates the first interindustry class byte coding
        newCla = (uint8_t) ((cla & 0xBC) | channelNumber);
    } else if (channelNumber < 20) {
        // b7 = 1 indicates the further interindustry class byte coding
        bool isSM = (cla & 0x0C) != 0;
        newCla = (uint8_t) ((cla & 0xB0) | 0x40 | (channelNumber - 4));
        if (isSM)
            newCla |= 0x20;
    } else {
        SELOGE("Channel number must be within [0..19]");
    }
    return newCla;
}

uint8_t SecureElement::clearChannelNumber(const uint8_t& cla) {
    bool isFirstInterindustry = (cla & 0x40) == 0x00;
    if (isFirstInterindustry)
        return cla & 0xFC;
    else
        return cla & 0xF0;
}

uint8_t SecureElement::parseChannelNumber(const uint8_t& cla) {
    bool isFirstInterindustry = (cla & 0x40) == 0x00;
    if (isFirstInterindustry)
        return cla & 0x03;
    else
        return (cla & 0x0F) + 4;
}

bool SecureElement::createSimApdu(AOSP_V1_0::SimApdu& apdu, const hidl_vec<uint8_t>& data) {
    if (data.size() < 4) {
        SELOGE("APDU data length is too short (%zu)", data.size());
        return false;
    }
    apdu.data.clear();

    apdu.sessionId = parseChannelNumber(data[0]); /* channel */
    apdu.cla = clearChannelNumber(data[0]);
    apdu.instruction = data[1];
    apdu.p1 = data[2];
    apdu.p2 = data[3];

    /*
        P3 will be negative for case 1.
        P3 will be short Le for case 2.
        P3 will be short Lc for case 3/4.
        P3 will be extended Le first byte for case 2E.
        P3 will be extended Lc first byte for case 3E/4E.

        if DATA size != 0, AT command will be CLA INS P1 P2 P3 DATA
        if DATA size == 0 and P3 is positive, AT command will be CLA INS P1 P2 P3
        if DATA size == 0 and P3 is negative, AT command will be CLA INS P1 P2
    */
    if (data.size() == 4) {
        /* Case 1. */
        apdu.p3 = -1;
    } else if (data.size() >= 5) {
        /* if data.size() == 5, Case 2. */
        /* if data[4] != 0, Case 3/4. */
        /* if data[4] == 0, Case 2E/3E/4E. */
        apdu.p3 = data[4];

        if (data.size() > 5)
            byteVectorToHexString(apdu.data, data, 5, data.size() - 5);
    }

    if (IS_DEBUG) {
        SELOGV("[%d] cla=0x%02X ins=0x%02X p1=0x%02X p2=0x%02X p3=0x%02X(%d) data=%s",
            apdu.sessionId, apdu.cla, apdu.instruction, apdu.p1, apdu.p2, apdu.p3, apdu.p3, apdu.data.c_str());
    }

    return true;
}

bool SecureElement::reissueCommandLocked(hidl_vec<uint8_t>& respVec, const hidl_vec<uint8_t>& data, uint8_t Le) {
    /* Must get mMainMutex before use this function. */
    SELOGD("reissue command Le=%u", Le);

    hidl_vec<uint8_t> reissueData;
    if (data.size() == 4
        || (data.size() > 5 && (data.size() - 5) == data[4])) {
        /* Case 1 or 3 */
        reissueData.resize(data.size() + 1);
        for (size_t i = 0; i < data.size(); ++i)
            reissueData[i] = data[i];
        reissueData[data.size()] = Le;
    } else {
        /* Case 2 or 4 */
        reissueData.resize(data.size());
        for (size_t i = 0; i < data.size() - 1; ++i)
            reissueData[i] = data[i];
        reissueData[data.size() - 1] = Le;
    }

    return transmitInternalLocked(respVec, reissueData);
}

void SecureElement::resetAll(const char* reason) {
    reset(false, true, reason);
}

void SecureElement::resetRadio(const char* reason) {
    reset(false, false, reason);
}

void SecureElement::reset(bool destroy, bool resetAll, const char* reason) {
    SELOGE("RESET reason: %s (%x, %x)", reason, destroy, resetAll);

    mIsReset = true;
    // Release response lock
    pthread_cond_signal(&mRespCond);
    clearWaitTimerLocked();

    /* Reset SE callback. */
    if (resetAll) {
        pthread_mutex_lock(&mCallbackMutex);
        if (mCallback_V1_0 != nullptr) {
            auto ret = mCallback_V1_0->unlinkToDeath(this);
            if (!ret.isOk()) // be checked for transport errors by calling its isOk() method
                SELOGE("SE service died");
        }
        mCallback_V1_0 = nullptr;
        pthread_mutex_unlock(&mCallbackMutex);
    }

    /* Reset RIL info. */
    pthread_mutex_lock(&mRadioMutex);
    if (mRadio != nullptr) {
        auto ret = mRadio->setResponseFunctions(nullptr, nullptr);
        if (!ret.isOk()) // be checked for transport errors by calling its isOk() method
            SELOGE("radio service died");
        if (mRadioDeathRecipient != nullptr) {
            auto ret = mRadio->unlinkToDeath(mRadioDeathRecipient);
            if (!ret.isOk()) // be checked for transport errors by calling its isOk() method
                SELOGE("radio service died");
        }
    }
    if (mMtkRadio != nullptr) {
        auto retM = mMtkRadio->setResponseFunctionsSE(nullptr, nullptr);
        if (!retM.isOk()) // be checked for transport errors by calling its isOk() method
            SELOGE("MTK radio service died");
        if (mMtkRadioDeathRecipient != nullptr) {
            auto retM = mMtkRadio->unlinkToDeath(mMtkRadioDeathRecipient);
            if (!retM.isOk()) // be checked for transport errors by calling its isOk() method
                SELOGE("MTK radio service died");
        }
    }
    mRadioInd = nullptr;
    mRadioRsp = nullptr;
    mRadio = nullptr;
    mMtkRadioInd = nullptr;
    mMtkRadioRsp = nullptr;
    mMtkRadio = nullptr;

    mRadioDeathRecipient = nullptr;
    mMtkRadioDeathRecipient = nullptr;
    mSERadioRspHandler = nullptr;

    pthread_mutex_unlock(&mRadioMutex);


    /* Reset request/response info. */
    pthread_mutex_lock(&mMainMutex);
    mSerial = 0;
    mCardState = AOSP_V1_0::CardState::ABSENT;

    /* Add sim state change event */
    mSimPreviousState = false;


    if (destroy == false) {


        pthread_mutex_unlock(&mMainMutex);
        SELOGD("RESET done");
    } else {
        /* Destroy mutex */
        pthread_mutex_destroy(&mRespMutex);
        pthread_mutex_destroy(&mCallbackMutex);
        pthread_mutex_destroy(&mRadioMutex);

        pthread_mutex_unlock(&mMainMutex);
        pthread_mutex_destroy(&mMainMutex);
        pthread_cond_destroy(&mRespCond);
        SELOGD("RESET Destroy done");
    }

    mIsReset = false;
}

int32_t SecureElement::getSerialAndIncrementLocked() {
    /* Must get mMainMutex before use this function. */
    mSerial ++;
    if(mSerial == MAX_SERIAL_NUMBER) {
        mSerial = 1;
    }

    return mSerial;
}

void SecureElement::startWaitTimerLocked() {
    /* Must get mMainMutex before use this function. */
    struct sigevent se;
    struct itimerspec ts;
    int ms = WAIT_TIMEOUT_MS;

    if (ms <= 0)
        return; /* no timeout */

    /* Create the timer */
    se.sigev_notify = SIGEV_THREAD;
    se.sigev_value.sival_ptr = this;
    se.sigev_notify_function = respTimeout;
    se.sigev_notify_attributes = NULL;
    if (timer_create(CLOCK_BOOTTIME, &se, &mTimerId) == -1) {
        SELOGE("timer_create == -1");
        return;
    }

    /* Start the timer */
    /* Initial expiration */
    ts.it_value.tv_sec = ms / 1000;
    ts.it_value.tv_nsec = (ms % 1000) * 1000000;
    /* Timer interval */
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;
    if (timer_settime(mTimerId, 0, &ts, NULL) == -1) {
        SELOGE("timer_settime == -1");
        return;
    }

    mTimerValid = true;
}

void SecureElement::clearWaitTimerLocked() {
    /* Must get mMainMutex before use this function. */
    if (mTimerValid == true) {
        timer_delete(mTimerId);

        mTimerValid = false;
    }
}

bool SecureElement::waitResponseLocked(SERadioResponseData& data, enum SERadioResponseType type, int32_t serial) {
    /* Must get mMainMutex before use this function. */
    bool result = false;
    startWaitTimerLocked();
    pthread_mutex_lock(&mRespMutex);
    pthread_cond_wait(&mRespCond, &mRespMutex); /* wait */
    pthread_mutex_unlock(&mRespMutex);
    clearWaitTimerLocked();

    pthread_mutex_lock(&mRadioMutex);
    if (isRadioAliveLocked()) {
        if(mSERadioRspHandler == nullptr) {
            pthread_mutex_unlock(&mRadioMutex);
            SELOGE("Handler is null");
        }
        else {
            result = mSERadioRspHandler->removeResponse(data, type, serial);
            pthread_mutex_unlock(&mRadioMutex);
            if (result == false) {
                // Assert
                // SELOG_ALWAYS_FATAL("cannot get response [%d: #%d]", type, serial);

                SELOGE("Access IOException: [%d: #%d]",type, serial);
            }
        }
    } else {
        pthread_mutex_unlock(&mRadioMutex);
        SELOGE("cannot get response [%d: #%d]. radio service died.", type, serial);
    }

    return result;
}

bool SecureElement::isRadioAliveLocked() {
    /* Must get mRadioMutex before use this function. */
    if (mMtkRadio != nullptr) {
        auto retM = mMtkRadio->ping();
        bool isDead = retM.isDeadObject();
        bool isOk = retM.isOk(); // be checked for transport errors by calling its isOk() method
        if (!isOk) {
            SELOGE("MTK isDead = %x, isOk = %x", isDead, isOk);
            return false;
        }
    } else {
        return false;
    }

    if (mRadio != nullptr) {
        auto ret = mRadio->ping();
        bool isDead = ret.isDeadObject();
        bool isOk = ret.isOk(); // be checked for transport errors by calling its isOk() method
        if (!isOk) {
            SELOGE("isDead = %x, isOk = %x", isDead, isOk);
            return false;
        }
    } else {
        return false;
    }

    return true;
}

void SecureElement::reconnect() {

    for(int i = 0; i < MAX_TRY_RADIO_TIMES; i++) {
        usleep(500000);
        SELOGD("EVENT_RADIO_PROXY_DEAD, coolies = %d", i);
        if (initRadioService()) {
            handleCallbackThread(true);
            break;
        }
    }
}

void *SecureElement::runCallbackThread(void *data) {

    SecureElement *instance = static_cast<SecureElement *>(data);
    instance->handleCallbackThread(true);
    return NULL;
}

void *SecureElement::runMainThread(void *data) {

    SecureElement *instance = static_cast<SecureElement *>(data);
    instance->reconnect();
    return NULL;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace secure_element
}  // namespace hardware
}  // namespace android
