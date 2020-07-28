/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
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

#include "RpDataAllowController.h"
#include "RpDataUtils.h"
#include "RfxStatusDefs.h"
#include "modecontroller/RpCdmaLteModeController.h"
#include <libmtkrilutils.h>
#include "ims/RpImsController.h"
#include <cutils/properties.h>

#define RP_DAC_LOG_TAG "RP_DAC"

/*****************************************************************************
 * Class RfxDataAllowController
 * this is a none slot controller to manage DATA_ALLOW_REQUEST.
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpDataAllowController", RpDataAllowController, RfxController);

RpDataAllowController::RpDataAllowController() :
    mDoingDataAllow(false),
    mReqDataAllow(false),
    mDetachingPeer(0),
    mModeStatus(MODE_SWITCH_UNKNOWN),
    mLastAllowTrueRequest(NULL),
    mAttachAvailabe(NULL),
    mAttachDefaultPhoneWhenNoSim(false),
    mSimCount(0) {
}

RpDataAllowController::~RpDataAllowController() {
}

void RpDataAllowController::onInit() {
    RfxController::onInit();  // Required: invoke super class implementation

    RFX_LOG_I(RP_DAC_LOG_TAG, "onInit");
    mDoingDataAllow = false;
    mReqDataAllow = false;
    mDetachingPeer = 0;
    mModeStatus = MODE_SWITCH_UNKNOWN;
    mLastAllowTrueRequest = NULL;
    mSimCount = RpFeatureOptionUtils::getSimCount();
    mAttachAvailabe = new AttachAvailable[mSimCount];

    resetDataAllowStatus();

    const int request_id_list[] = {
        RIL_REQUEST_ALLOW_DATA,  // 123
        RIL_REQUEST_SET_PS_REGISTRATION,
    };

    // register request
    // NOTE. one id can only be registered by one controller
    for (int i = 0; i < mSimCount; i++) {
        registerToHandleRequest(i, request_id_list, sizeof(request_id_list) / sizeof(int));
        if (mSimCount == 1) {
            // AOSP single card will not send ALLOW_DATA request, set it to true by default
            mAttachAvailabe[i].fromFw = true;
        } else {
            mAttachAvailabe[i].fromFw = false;
        }
        mAttachAvailabe[i].fromOpCtrl = true;
    }

    // Register callbacks to listen mode change event.
    getNonSlotScopeStatusManager()->registerStatusChanged(RFX_STATUS_KEY_MODE_SWITCH,
            RfxStatusChangeCallback(this, &RpDataAllowController::onModeChange));
    getNonSlotScopeStatusManager()->registerStatusChanged(RFX_STATUS_KEY_PREFERRED_DATA_SIM,
            RfxStatusChangeCallback(this, &RpDataAllowController::onPreferredDataSimChanged));
    for (int j = 0; j < mSimCount; j++) {
        getStatusManager(j)->registerStatusChanged(RFX_STATUS_CONNECTION_STATE,
                RfxStatusChangeCallback(this, &RpDataAllowController::onHidlStateChanged));
        getStatusManager(j)->registerStatusChangedEx(
                RFX_STATUS_KEY_SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY,
                RfxStatusChangeCallbackEx(this, &RpDataAllowController::onSimMeLockChanged));
        getStatusManager(j)->registerStatusChanged(RFX_STATUS_KEY_CARD_TYPE,
                RfxStatusChangeCallback(this, &RpDataAllowController::onSimChanged));
    }
}

void RpDataAllowController::onDeinit() {
    RFX_LOG_I(RP_DAC_LOG_TAG, "onDeinit");
    mDoingDataAllow = false;
    mReqDataAllow = false;
    mDetachingPeer = 0;
    mModeStatus = MODE_SWITCH_UNKNOWN;
    mLastAllowTrueRequest = NULL;
    mAttachDefaultPhoneWhenNoSim = false;
    delete[] mAttachAvailabe;
    RfxController::onDeinit();
    getNonSlotScopeStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_MODE_SWITCH,
            RfxStatusChangeCallback(this, &RpDataAllowController::onModeChange));
    getNonSlotScopeStatusManager()->unRegisterStatusChanged(RFX_STATUS_KEY_PREFERRED_DATA_SIM,
            RfxStatusChangeCallback(this, &RpDataAllowController::onPreferredDataSimChanged));
    for (int j = 0; j < mSimCount; j++) {
        getStatusManager(j)->unRegisterStatusChanged(RFX_STATUS_CONNECTION_STATE,
                RfxStatusChangeCallback(this, &RpDataAllowController::onHidlStateChanged));
        getStatusManager(j)->unRegisterStatusChanged(RFX_STATUS_KEY_CARD_TYPE,
                RfxStatusChangeCallback(this, &RpDataAllowController::onSimChanged));
    }
}

bool RpDataAllowController::onHandleRequest(const sp<RfxMessage>& message) {
    RFX_LOG_I(RP_DAC_LOG_TAG, "[%d]Handle request %s",
            message->getPToken(), RpDataUtils::requestToString(message->getId()));

    switch (message->getId()) {
        case RIL_REQUEST_ALLOW_DATA:
            preprocessRequest(message);
            break;
        case RIL_REQUEST_SET_PS_REGISTRATION:
            preprocessRequest(message);
            break;
        default:
            RFX_LOG_D(RP_DAC_LOG_TAG, "unknown request, ignore!");
            break;
    }
    return true;
}

bool RpDataAllowController::onHandleResponse(const sp<RfxMessage>& message) {
    switch (message->getId()) {
        case RIL_REQUEST_ALLOW_DATA:
            handleSetDataAllowResponse(message);
            break;
        case RIL_REQUEST_SET_PS_REGISTRATION:
            handleSetPsRegistrationResponse(message);
            break;
        default:
            RFX_LOG_D(RP_DAC_LOG_TAG, "unknown response, ignore!");
            break;
    }
    return true;
}

bool RpDataAllowController::onPreviewMessage(const sp<RfxMessage>& message) {
    // This function will be called in the case of the registered request/response and urc.
    // For instance, register RIL_REQUEST_ALLOW_DATA will receive its request and response.
    // Therefore, it will be called twice in the way and we only care REQUEST in preview message,
    // but still need to return true in the case of type = RESPONSE.
    int requestToken = message->getPToken();
    int requestId = message->getId();

    if (message->getType() == REQUEST
            && isNeedSuspendRequest(message)) {
        return false;
    } else {
        return true;
    }
}

bool RpDataAllowController::isNeedSuspendRequest(const sp<RfxMessage>& message) {
    /*
     * white list for suspend request.
     */
    int requestToken = message->getPToken();
    int requestId = message->getId();
    if (requestId == RIL_REQUEST_ALLOW_DATA || requestId == RIL_REQUEST_SET_PS_REGISTRATION) {
        /* for denali, after sim switch TRM will happened, and rilproxy will be re-created,
         * AOSP will trigger to resendDataAllowed which is a concurrent process with mode
         * change and rat switch and ps type initialization(they follow sim switch too).
         * we must ensure they are inited over then can process data allow request,
         * or we will get wrong ps type.
         */
        if (RpFeatureOptionUtils::isSvlteSupport() && mModeStatus != MODE_SWITCH_FINISH) {
            return true;
        }
        if (!mDoingDataAllow) {
            RFX_LOG_I(RP_DAC_LOG_TAG, "[%d] isNeedSuspendRequest: First RIL_REQUEST_ALLOW_DATA"
                    ", set flag on", requestToken);
            mDoingDataAllow = true;
            return false;
        } else {
            return true;
        }
    }
    return false;
}

bool RpDataAllowController::onCheckIfResumeMessage(const sp<RfxMessage>& message) {
    int requestToken = message->getPToken();
    int requestId = message->getId();
    /* For gen90(D-1/2/3), after sim switch TRM will happened, and rilproxy will be re-created,
     * AOSP will trigger to resendDataAllowed which is a concurrent process with mode
     * change and rat switch and ps type initialization(they follow sim switch too).
     * we must ensure they are inited over then can process data allow request,
     * or we will get wrong ps type.
     */
    if (RpFeatureOptionUtils::isSvlteSupport() && mModeStatus != MODE_SWITCH_FINISH) {
        return false;
    }
    if (!mDoingDataAllow) {
        return true;
    } else {
        return false;
    }
}

bool RpDataAllowController::setActivePsSlotIfNeeded(bool allow, int slot) {
    // Only SVLTE project need to send AT+EACTS to specify data SIM by modem's request.
    // TODO: Discuss with MD to align SVLTE design to be the same as SRLTE.
    if (RpFeatureOptionUtils::isSvlteSupport()) {
        // send AT+EACTS before AT+CGATT=1
        if (allow) {
            sp<RfxMessage> msg = RfxMessage::obtainRequest(slot,
                    RADIO_TECH_GROUP_GSM,
                    RIL_LOCAL_GSM_REQUEST_SET_ACTIVE_PS_SLOT);
            Parcel* parcel2 = msg->getParcel();
            parcel2->writeInt32(1);  // content length.
            parcel2->writeInt32(slot + 1);
            requestToRild(msg);
            return true;
        }
    }
    return false;
}

void RpDataAllowController::handleSetDataAllowRequest(const sp<RfxMessage>& request) {
    Parcel* parcel = request->getParcel();
    request->resetParcelDataStartPos();
    parcel->readInt32();  // pass length
    bool allowData = parcel->readInt32();  // get allow flag.

    RFX_LOG_D(RP_DAC_LOG_TAG, "[%d]handleSetDataAllowRequest: requestId:%d, phone:%d, allow:%d",
            request->getPToken(), request->getId(), request->getSlotId(), allowData);
    mReqDataAllow = allowData;
    setActivePsSlotIfNeeded(allowData, request->getSlotId());

    // Possibly send attach to GSM but send detach to C2K
    // Ex, When SIM Switch is disabled and Switch phone is very slowly
    // Switch data to CDMA when plugged in but psType is GSM and switch
    // to CDMA later to detach.
    if (needSetDataAllowGsmCdma(allowData, request->getSlotId())) {
        sendToGsm(request);
        sendToC2k(request);

        // Set flag which we send request to both GSM and CDMA,
        // we need to get response from both GSM and CDMA.
        setRequestDataAllowType(request->getPToken(), DATA_ALLOW_REQUEST_TYPE_GSM_AND_CDMA);
    } else {
        // If send request to C2K RILD when C capability is removed, RADIO_NOT_AVAILABLE will be
        // returned as response. This will cause the DcSwitchStateMachine directly from Detaching to
        // Idle State and PS can not detach normally.
        // Ex: CT 3G + CT 4G, data from CT 3G to CT 4G, CT 4G SIM can not register LTE.
        if (RpDataUtils::getPsType(this, request->getSlotId()) == RADIO_TECH_GROUP_C2K) {
            if (request->getSlotId() == RpCdmaLteModeController::getCdmaSocketSlotId()) {
                RFX_LOG_I(RP_DAC_LOG_TAG, "psType as CDMA");
                sendToC2k(request);
            } else {
                logW(RP_DAC_LOG_TAG, "PS type is CDMA, but C capability removed");
                sendToGsm(request);
            }
        } else {
            sendToGsm(request);
        }
        setRequestDataAllowType(request->getPToken(), DATA_ALLOW_REQUEST_TYPE_GSM_OR_CDMA);
    }
    // update property record which sim is in attach, record attach flag early.
    if (mReqDataAllow) {
        updateAttachSim(request->getSlotId(), mReqDataAllow);
    }
}

void RpDataAllowController::handleSetDataAllowResponse(const sp<RfxMessage>& response) {
    DataAllow_Request_Type requestType = getRequestDataAllowType(response->getPToken());

    if (requestType == DATA_ALLOW_REQUEST_TYPE_GSM_AND_CDMA) {
        sp<RfxMessage> outResponse;
        ResponseStatus status = RfxController::preprocessResponse(response, outResponse);
        RFX_LOG_I(RP_DAC_LOG_TAG, "[%d]handleSetDataAllowResponse allowData = %d, status = %d, getError() = %d",
                response->getPToken(), mReqDataAllow, status, response->getError());
        if (status == RESPONSE_STATUS_HAVE_MATCHED
                || status == RESPONSE_STATUS_HAVE_MATCHED_WITH_C_CHANGED) {
            if (mReqDataAllow && (response->getError() == RIL_E_OEM_MULTI_ALLOW_ERR
                    || outResponse->getError() == RIL_E_OEM_MULTI_ALLOW_ERR)) {
                // check if multiple PS allow error
                RfxController::resetPreprocessedResponse(response);
                handleMultiAttachError(response->getSlotId());
                return;
            }

            if (checkDetachingPeer()) {
                // Detact Peer Result couldn't pass to RILJ, it will re-attach directly
                return;
            }
            updateDataAllowStatus(response->getSlotId(), mReqDataAllow);
            if (response->getError() != RIL_E_SUCCESS) {
                responseToRilj(response);
            } else {
                responseToRilj(outResponse);
            }

            removeRequestDataAllowType(response->getPToken());
            mDoingDataAllow = false;
        }
        /* Other values: RESPONSE_STATUS_NO_NEED_WAIT_OTHERS, RESPONSE_STATUS_NO_MATCH_AND_SAVE,
           RESPONSE_STATUS_ALREADY_SAVED, RESPONSE_STATUS_HAVE_BEEN_DELETED, RESPONSE_STATUS_INVALID.
           RESPONSE_STATUS_NO_NEED_WAIT_OTHERS: Do nothing as data have checked whether request was
           sent to C2K/GSM.
           The others: No need to handle currently.
        */
    } else {
        RFX_LOG_I(RP_DAC_LOG_TAG, "[%d]handleSetDataAllowResponse allowData = %d, response->getError()=%d, getSlot()=%d",
                response->getPToken(), mReqDataAllow, response->getError(), response->getSlotId());
        if (mReqDataAllow && response->getError() == RIL_E_OEM_MULTI_ALLOW_ERR) {
            // check if multiple PS allow error
            handleMultiAttachError(response->getSlotId());
            return;
        }

        if (checkDetachingPeer()) {
            // Detact Peer Result couldn't pass to RILJ, it will re-attach directly
            return;
        }
        updateDataAllowStatus(response->getSlotId(), mReqDataAllow);
        responseToRilj(response);
        mDoingDataAllow = false;
        removeRequestDataAllowType(response->getPToken());
    }
    // update property record which sim is in attach, clear attach flag after detach.
    // detaching process should be considered the same as attach.
    if (!mReqDataAllow) {
        updateAttachSim(response->getSlotId(), mReqDataAllow);
    }
}

void RpDataAllowController::onPreferredDataSimChanged(RfxStatusKeyEnum key,
            RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    int dataSim = value.asInt();
    int currentSimCount = 0;

    // When there is no sims, Google AOSP will always set sim 0 as prefer data sim.
    // Set a flag in this scenario for attach capability sim later if needed.
    if (dataSim == 0) {
        for (int i = 0; i < mSimCount; i++) {
            if (getStatusManager(i)->getIntValue(RFX_STATUS_KEY_CARD_TYPE, -1) > 0 ) {
                currentSimCount++;
            }
        }
        RFX_LOG_D(RP_DAC_LOG_TAG, "[%s] currentSimCount %d", __FUNCTION__, currentSimCount);
        if (currentSimCount == 0) {
            mAttachDefaultPhoneWhenNoSim = true;
        } else {
            mAttachDefaultPhoneWhenNoSim = false;
        }
    } else if (dataSim > 0) {
        mAttachDefaultPhoneWhenNoSim = false;
    } else {
        RFX_LOG_E(RP_DAC_LOG_TAG, "[%s] error, should not be here", __FUNCTION__);
    }

    if(dataSim >= 0 && mOnDemandQueue.size() == 0){
        RFX_LOG_D(RP_DAC_LOG_TAG, "[%s] set allow to preferred data sim: %d",
                __FUNCTION__, dataSim);
        onSetDataAllow(dataSim);
    } else if (mOnDemandQueue.size() > 0) {
        RFX_LOG_D(RP_DAC_LOG_TAG, "[%s] do not set allow to preferred data sim, due to "
            "type=%d, slot=%d", __FUNCTION__, mOnDemandQueue[0].type, mOnDemandQueue[0].slotId);
    }
}

void RpDataAllowController::onSetDataAllow(int slotId) {
    if(slotId < 0 || slotId >= mSimCount){
        RFX_LOG_E(RP_DAC_LOG_TAG, "onSetDataAllow: inValid slotId", slotId);
        return;
    }
    for (int i = 0; i < mSimCount; i++) {
        if(i != slotId) {
            obtainAndEnqueueDataAllowRequst(i, false);
        }
    }
    obtainAndEnqueueDataAllowRequst(slotId, true);
    RpDataUtils::setAllowDataSlot(true, slotId);
}

void RpDataAllowController::obtainAndEnqueueDataAllowRequst(int slotId, bool allow) {
    sp<RfxMessage> request = RfxMessage::obtainRequest(slotId,
            RADIO_TECH_GROUP_GSM, RIL_REQUEST_ALLOW_DATA);
    request->getParcel()->writeInt32(1); // Parcel Length
    request->getParcel()->writeInt32(allow ? 1 : 0); // Allow or not
    request->setSlotId(slotId);

    // Put into the queue for pre-view message (or no needed)
    RfxMainThread::enqueueMessage(request);
}

void RpDataAllowController::sendToGsm(const sp<RfxMessage>& request) {
    sp<RfxMessage> message = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
            request->getId(),
            request,
            true);
    requestToRild(message);
}

void RpDataAllowController::sendToC2k(const sp<RfxMessage>& request) {
    sp<RfxMessage> message = RfxMessage::obtainRequest(RADIO_TECH_GROUP_C2K,
            request->getId(),
            request,
            true);
    requestToRild(message);
}

void RpDataAllowController::setRequestDataAllowType(int requestToken,
            DataAllow_Request_Type requestType) {
    RFX_LOG_D(RP_DAC_LOG_TAG, "setRequestDataAllowType: token = %d, type = %d", requestToken, requestType);
    mDataAllowTypeMap[requestToken] = requestType;
}

DataAllow_Request_Type RpDataAllowController::getRequestDataAllowType(int requestToken) {
    std::map<int, DataAllow_Request_Type>::iterator iterator = mDataAllowTypeMap.find(requestToken);
    if (iterator != mDataAllowTypeMap.end()) {
        RFX_LOG_D(RP_DAC_LOG_TAG, "getRequestDataAllowType: token = %d, type = %d, size = %d",
                requestToken, iterator->second, mDataAllowTypeMap.size());
        return iterator->second;
    }

    return DATA_ALLOW_REQUEST_TYPE_GSM_OR_CDMA;
}

void RpDataAllowController::removeRequestDataAllowType(int requestToken) {
    mDataAllowTypeMap.erase(requestToken);
}

bool RpDataAllowController::needSetDataAllowGsmCdma(bool isAllowData, int slotId) {
    if (!RpFeatureOptionUtils::isC2kSupport()) {
        return false;
    }

    bool isCdmaLteMode = RpDataUtils::isCdmaLteMode(this, slotId);
    int cdmaSlot = RpCdmaLteModeController::getCdmaSocketSlotId();
    RFX_LOG_I(RP_DAC_LOG_TAG, "needSetDataAllowGsmCdma: cdmaSlot=%d, isCdmaLteMode=%d, allow=%d, slot=%d",
            cdmaSlot, isCdmaLteMode, isAllowData, slotId);
    if (isCdmaLteMode) {
        return (slotId == cdmaSlot);
    } else {
        if (isAllowData) {
            return false;
        }
        // Always return true if current slot is CDMA slot except the case
        // that when CDMA card in roaming mode.
        if (slotId == cdmaSlot) {
            int cardType = getStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_CARD_TYPE);
            int mode = getStatusManager()->getIntValue(RFX_STATUS_KEY_NWS_MODE, NWS_MODE_CSFB);
            RFX_LOG_I(RP_DAC_LOG_TAG, "needSetDataAllowGsmCdma: cardType = %d, mode = %d", cardType, mode);

            if (RpDataUtils::isCdmaCard(cardType) && mode == NWS_MODE_CSFB) {
                return false;
            } else {
                return true;
            }
        }
        return false;
    }
}

bool RpDataAllowController::preprocessRequest(const sp<RfxMessage>& request) {
    Parcel* parcel = request->getParcel();
    request->resetParcelDataStartPos();
    parcel->readInt32();  // pass length
    bool allowData = parcel->readInt32();  // get allow flag.
    int requestId = request->getId();
    int slotId = request->getSlotId();

    if (requestId == RIL_REQUEST_ALLOW_DATA) {
        mReqDataAllow = allowData;
        mAttachAvailabe[slotId].fromFw = allowData ? true : false;
    } else if (requestId == RIL_REQUEST_SET_PS_REGISTRATION) {
        mAttachAvailabe[slotId].fromOpCtrl = allowData ? true : false;
    }

    if (allowData) {
        // Copy the request
        // 1. if allow true,  apply for retry.
        if (requestId == RIL_REQUEST_ALLOW_DATA) {
            mLastAllowTrueRequest = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
                    request->getId(), request, true);
        }
        if (!isAttachAvailable(slotId)) {
            // isAttachAvailable == false that means it not allow to do really attach,
            // And we give a fake response pretend it's success for releasing wakelock.
            // btw, when the OpCtrl allow as to re-attach, it will call setPsRegistration again.
            RFX_LOG_D(RP_DAC_LOG_TAG, "Ignore this allow/attach because of isAttachAvailable false");
            if (requestId == RIL_REQUEST_ALLOW_DATA) {
                responseToRilj(RfxMessage::obtainResponse(RIL_E_SUCCESS, request));
            }
            mDoingDataAllow = false;
        } else {
            handleSetDataAllowRequest(request);
        }
    } else {
        // 2. if allow false, wait for the callback using. (precheckDetach)
        precheckDetach(request);
    }

    return true;
}

/*
 * Create request to detach peer phone.
 * The follow will be:
 *   handleMultiAttachError -> precheckDetach (ims precheck or preCheckIfNeedDeactAllData)->
 *   -> handleSetDataAllowResponse -> handleSetDataAllowRequest(mLastAllowTrueRequest)
 * The last step means that re-attach for original attach request.
 */
void RpDataAllowController::handleMultiAttachError(int activePhoneId) {
    // Check with ims module if we could detach
    RFX_LOG_I(RP_DAC_LOG_TAG, "detachPeerPhone: activePhoneId = %d", activePhoneId);

    // Detach peer phone
    for (int i = 0; i < mSimCount; i++) {
        if (i != activePhoneId) {
            // Create detach request for common
            sp<RfxMessage> msg =
                    RfxMessage::obtainRequest(i, RADIO_TECH_GROUP_GSM, RIL_REQUEST_ALLOW_DATA);
            msg->getParcel()->writeInt32(1);
            msg->getParcel()->writeInt32(0);
            msg->setSlotId(i);

            RFX_LOG_D(RP_DAC_LOG_TAG, "detachPeerPhone: precheck PhoneId = %d", i);
            // Notify Detach precheck
            mDetachingPeer++;
            precheckDetach(msg);
        }
    }
}

/*
 * Check if the process is detaching peer.
 * Return true for ignoring response to RILJ because the requests are created by RILProxy.
 */
bool RpDataAllowController::checkDetachingPeer() {
    if (mDetachingPeer > 0) {
        mDetachingPeer--;
        RFX_LOG_D(RP_DAC_LOG_TAG, "handleSetDataAllowResponse consume detachPeer, mDetachingPeer %d",
                 mDetachingPeer);
        if (mDetachingPeer == 0) {
            // resume the attach request
            handleSetDataAllowRequest(mLastAllowTrueRequest);
        }
        return true;
    }
    return false;
}

/*
 * Check the detach is allowed or not.
 * Abstract from proprocess function for common using like detachPeer.
 */
void RpDataAllowController::precheckDetach(const sp<RfxMessage>& request) {
    if (isMultiPsAttachSupport() && request->getId() == RIL_REQUEST_ALLOW_DATA) {
        // To skip pre-check IMS status and return the request in MPS project while detach.
        // If there are requirement need to detach IMS in MPS project, could modify flow :
        // precheckImsStatus->onImsConfirmed->preCheckIfNeedDeactAllData->onDeactAllDataConfirmed
        preCheckIfNeedDeactAllData( request);
    } else {
        // Detach need notify to ims module
        precheckImsStatus(request);
    }
    return;
}

//M: start for ImsPreCheck
bool RpDataAllowController::precheckImsStatus(const sp<RfxMessage>& message) {
    // Skip pre-check if support MPS
    if (!isMultiPsAttachSupport()) {
        RpImsController *imsController;
        sp<RfxAction> action;
        Parcel* parcel = message->getParcel();
        message->resetParcelDataStartPos();
        parcel->readInt32();  // pass length
        bool allow = parcel->readInt32();  // get allow flag.
        int slotId = message->getSlotId(); // get sim slot id.

        logD(RP_DAC_LOG_TAG, "Detach IMS precheck, slotId: %d", slotId);

        imsController = (RpImsController *) findController(message->getSlotId(),
                                                       RFX_OBJ_CLASS_INFO(RpImsController));
        action = new RfxAction1<const sp<RfxMessage>>(this,
                                                   &RpDataAllowController::onImsConfirmed, message);
        imsController->requestImsDeregister(slotId, action);
     } else {
         RFX_LOG_D(RP_DAC_LOG_TAG, "skip IMS precheck for MPS");
         handleSetDataAllowRequest(message);
     }

    return true;
}

void RpDataAllowController::onImsConfirmed(const sp<RfxMessage> message) {
    int slotId = message->getSlotId(); // get sim slot id.
    logD(RP_DAC_LOG_TAG, "onImsConfirmed Slot: %d, ims preCheck Done", slotId);
    handleSetDataAllowRequest(message);
}

/*
 * This method is used to record/update which sim is doing attach.
 * -1 - no sim attach
 *  0 - sim1 attach
 *  1 - sim2 attach and so on.
 * Currently this info only used by md3 rild function combineDataAttach.
 *
 * Attention: this record is only ensured in single ps attach project.
 * TODO: for multiple ps attach we need extend if needed.
 */
void RpDataAllowController::updateAttachSim(int slotId, bool allow) {
    char newRecord[PROPERTY_VALUE_MAX] = { 0 };
    char oldRecord[PROPERTY_VALUE_MAX] = { 0 };
    int old = 0;
    property_get(RP_ATTACH_SIM, oldRecord, "-1");
    old = atoi(oldRecord);
    if (allow) {
        // always record latest attach sim.
        sprintf(newRecord, "%d", slotId);
        property_set(RP_ATTACH_SIM, newRecord);
    } else {
        // clear record only if it is matched.
        if (slotId == old) {
            sprintf(newRecord, "%d", -1);
            property_set(RP_ATTACH_SIM, newRecord);
        }
    }
    mAttachAvailabe[slotId].fromFw = allow;
    RFX_LOG_D(RP_DAC_LOG_TAG, "updateAttachSim attachSim:%s", newRecord);
}

void RpDataAllowController::onModeChange(RfxStatusKeyEnum key,
    RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    mModeStatus = value.asInt();
}

// Multi-PS Attach start
// The parameter of allow in message always is false.
void RpDataAllowController::preCheckIfNeedDeactAllData(const sp<RfxMessage>& message) {
    // It needs to deact all PDN before set EDALLOW=0 for the modem resource limitation.
    // The reason to do this in RilProxy is for the consideration to support C2K in the
    // multi-PS project. In that case, it should also deact C2K PDN as well.
    // FIXME: Need to check/UT/IT the case once support C2K in 92MD
    // Allow data =false and will send EDALLOW = 0  so do deact.
    RpDataController *rpDataController;
    sp<RfxAction> action;
    int slotId = message->getSlotId();

    RFX_LOG_D(RP_DAC_LOG_TAG, "preCheckIfNeedDeactAllData: MPS-isMpsSupport= %d, allowdData= %d"
            ", MPS-goin deact all data, slotId: %d",
            isMultiPsAttachSupport(), mReqDataAllow, slotId);

    rpDataController = (RpDataController *) findController(message->getSlotId(),
            RFX_OBJ_CLASS_INFO(RpDataController));

    action = new RfxAction1<const sp<RfxMessage>>(this,
            &RpDataAllowController::onDeactAllDataConfirmed, message);

    // Call the API to deact all data PDN and wait its callback to confirm once done.
    rpDataController->handleDeactAllDataCall(slotId, action);
}

void RpDataAllowController::onDeactAllDataConfirmed(const sp<RfxMessage> message) {
    // The callback for the deact data done.
    RFX_LOG_D(RP_DAC_LOG_TAG, "onDeactAllDataConfirmed: MPS-deact done before EDALLOW=0");
    handleSetDataAllowRequest(message);
    return;
}
// Multi-PS Attach end

/*
 * slotId: request to which slot id
 * register: True/False (means Allow/Disallow or Attach/Detach) equal to allowData
 */
bool RpDataAllowController::setPsRegistration(int slotId, bool regAllow) {
    // Create equest
    sp<RfxMessage> request = RfxMessage::obtainRequest(slotId, RADIO_TECH_GROUP_GSM,
            RIL_REQUEST_SET_PS_REGISTRATION);
    request->getParcel()->writeInt32(1); // Parcel Length
    request->getParcel()->writeInt32(regAllow?1:0); // Allow or not
    request->setSlotId(slotId);

    // Put into the queue for pre-view message (or no needed)
    RfxMainThread::enqueueMessage(request);
    return true;
}

void RpDataAllowController::handleSetPsRegistrationResponse(const sp<RfxMessage>& response) {
    int slotId = response->getSlotId();
    RFX_LOG_D(RP_DAC_LOG_TAG, "Finish SET_PS_REGISTRATION and no need response(%d)",
            mAttachAvailabe[slotId].fromOpCtrl);
    updateDataAllowStatus(slotId, mAttachAvailabe[slotId].fromOpCtrl);
    mDoingDataAllow = false;
}

bool RpDataAllowController::isAttachAvailable(int slotId){
    RFX_LOG_I(RP_DAC_LOG_TAG, "isAttachAvailable[%d]: fromFw:%d, fromOpCtrl:%d",
            slotId, mAttachAvailabe[slotId].fromFw, mAttachAvailabe[slotId].fromOpCtrl);
    if (mAttachAvailabe[slotId].fromFw && mAttachAvailabe[slotId].fromOpCtrl) {
        return true;
    }
    return false;
}

void RpDataAllowController::updateDataAllowStatus(int slotId, bool allow) {
    char propertyVal[PROPERTY_VALUE_MAX] = {0};
    if (allow) {
        for (int i = 0; i < mSimCount; i++) {
            getMSimProperty(i, PROP_DATA_ALLOW_STATUS, propertyVal);
            propertyVal[1] = '\0';
            if (i != slotId && atoi(propertyVal) != 0) {
                setMSimProperty(i, PROP_DATA_ALLOW_STATUS, "0");
                if (RpDataUtils::isPreferredDataMode()) {
                    getStatusManager(i)->setIntValue(RFX_STATUS_KEY_SLOT_ALLOW, 0);
                }
            }
        }
        setMSimProperty(slotId, PROP_DATA_ALLOW_STATUS, "1");
        if (RpDataUtils::isPreferredDataMode()) {
            getStatusManager(slotId)->setIntValue(RFX_STATUS_KEY_SLOT_ALLOW, 1);
        }
    } else {
        setMSimProperty(slotId, PROP_DATA_ALLOW_STATUS, "0");
        if (RpDataUtils::isPreferredDataMode()) {
            getStatusManager(slotId)->setIntValue(RFX_STATUS_KEY_SLOT_ALLOW, 0);
        }
    }
    RFX_LOG_D(RP_DAC_LOG_TAG, "[updateDataAllowStatus] SIM%d: %d", slotId, allow);
}

void RpDataAllowController::resetDataAllowStatus() {
    if (!RpDataUtils::isPreferredDataMode()) {
        logD(RP_DAC_LOG_TAG, "resendAllowData: isn't preferred data mode, return");
        return;
    }
    for (int i = 0; i < mSimCount; i++) {
        if (isDataAllow(i)) {
            getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_PREFERRED_DATA_SIM,
                    i, true);
            updateDataAllowStatus(i, true);
        }
    }
}

bool RpDataAllowController::isDataAllow(int slotId) {
    char propertyVal[PROPERTY_VALUE_MAX] = {0};
    getMSimProperty(slotId, PROP_DATA_ALLOW_STATUS, propertyVal);
    if (strlen(propertyVal) != 0 && atoi(propertyVal) == 1) {
        return true;
    }
    return false;
}

int RpDataAllowController::checkRequestExistInQueue(int type, int slotId) {
    for(int i = 0 ; i < (int)mOnDemandQueue.size(); i++){
        if(mOnDemandQueue[i].type == type &&
                mOnDemandQueue[i].slotId == slotId) {
            return i;
        }
    }
    return -1;
}

int RpDataAllowController::checkRequestExistInQueue(int type) {
    for(int i = 0 ; i < (int)mOnDemandQueue.size(); i++){
        if(mOnDemandQueue[i].type == type) {
            return i;
        }
    }
    return -1;
}

void RpDataAllowController::onHidlStateChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    bool oldState = old_value.asBool();
    bool newState = value.asBool();

    RFX_LOG_I(RP_DAC_LOG_TAG, "onHidlStateChanged: old state:%d, new state:%d, clear queue",
            oldState, newState);
    mOnDemandQueue.clear();
}

void RpDataAllowController::onSimMeLockChanged(int slotId, RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);

    int shouldService = value.asInt();
    int policy = getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_SML_SLOT_LOCK_POLICY, 0);
    RFX_LOG_D(RP_DAC_LOG_TAG, "SimMeLockChanged[%d]: policy = %d, shouldService = %d",
            slotId, policy, shouldService);
    if (policy == SML_SLOT_LOCK_POLICY_UNKNOWN
            || policy == SML_SLOT_LOCK_POLICY_NONE
            || policy == SML_SLOT_LOCK_POLICY_LEGACY) {
        return;
    }
    if (shouldService != SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_FULL &&
        shouldService != SML_SLOT_LOCK_POLICY_SERVICE_CAPABILITY_PS_ONLY) {
        mOnDemandQueue.erase(
                std::remove_if(mOnDemandQueue.begin(), mOnDemandQueue.end(),
                [slotId](const OnDemandRequest & request) { return request.slotId == slotId; }),
                mOnDemandQueue.end());
        if (mOnDemandQueue.size() == 0) {
            int preferSim = getNonSlotScopeStatusManager()->getIntValue(
                    RFX_STATUS_KEY_PREFERRED_DATA_SIM);
            RFX_LOG_D(RP_DAC_LOG_TAG, "SimMeLockChanged[%d]: set back to prefer sim: %d",
                    slotId, preferSim);
            onSetDataAllow(preferSim);
        } else if (mOnDemandQueue.size() > 0) {
            RFX_LOG_D(RP_DAC_LOG_TAG, "SimMeLockChanged[%d]: set to on-demand: type: %d, sim: %d",
                    slotId, mOnDemandQueue[0].type, mOnDemandQueue[0].slotId);
            onSetDataAllow(mOnDemandQueue[0].slotId);
        }
    }
}

void RpDataAllowController::enqueueNetworkRequest(int requestId, int slotId) {
    int dataSim = getNonSlotScopeStatusManager()->getIntValue(
            RFX_STATUS_KEY_DEFAULT_DATA_SIM);
    RFX_LOG_D(RP_DAC_LOG_TAG, "enqueueNetworkRequest: requestId = %d, slotId = %d, dataSim = %d",
            requestId , slotId, dataSim);
    if(slotId == dataSim || requestId == RIL_DATA_PROFILE_DEFAULT
            || requestId == RIL_DATA_PROFILE_IMS) {
        return;
    }

    if (mOnDemandQueue.size() == 0) {
        RFX_LOG_D(RP_DAC_LOG_TAG, "enqueueNetworkRequest: do set data allow to non-dds");

        OnDemandRequest demandRequest;
        demandRequest.type = requestId;
        demandRequest.slotId = slotId;
        mOnDemandQueue.push_back(demandRequest);

        onSetDataAllow(slotId);
        return;
    } else {
        int ret = checkRequestExistInQueue(requestId, slotId);
        if (ret >= 0) {
            RFX_LOG_D(RP_DAC_LOG_TAG,
                    "enqueueNetworkRequest: type=%d, slotId=%d, already enqueue, return",
                    requestId , slotId);
            return;
        }

        OnDemandRequest demandRequest;
        demandRequest.type = requestId;
        demandRequest.slotId = slotId;
        mOnDemandQueue.push_back(demandRequest);

        RFX_LOG_D(RP_DAC_LOG_TAG, "enqueueNetworkRequest: curr queue: %d",
                mOnDemandQueue.size());
        return;
    }
}

void RpDataAllowController::dequeueNetworkRequest(int requestId, int slotId) {
    RFX_LOG_D(RP_DAC_LOG_TAG, "dequeueNetworkRequest: requestId = %d , slotId = %d,",
            requestId , slotId);
    if (requestId == RIL_DATA_PROFILE_DEFAULT || requestId == RIL_DATA_PROFILE_IMS) {
        return;
    }

    int ret = checkRequestExistInQueue(requestId, slotId);
    if (ret >= 0) {
        mOnDemandQueue.erase(mOnDemandQueue.begin() + ret);
        RFX_LOG_D(RP_DAC_LOG_TAG, "dequeueNetworkRequest: curr queue: %d",
                mOnDemandQueue.size());
    }
    if (mOnDemandQueue.size() == 0) {
        int dataSim = getNonSlotScopeStatusManager()->getIntValue(
                RFX_STATUS_KEY_PREFERRED_DATA_SIM);
        onSetDataAllow(dataSim);
    } else if (mOnDemandQueue.size() > 0){
        onSetDataAllow(mOnDemandQueue[0].slotId);
    }
}

void RpDataAllowController::onSimChanged(RfxStatusKeyEnum key,
        RfxVariant old_value, RfxVariant new_value) {
    RFX_UNUSED(key);
    RFX_UNUSED(old_value);
    RFX_UNUSED(new_value);
    RFX_LOG_D(RP_DAC_LOG_TAG, "%s: mAttachDefaultPhoneWhenNoSim %d",
            __FUNCTION__, mAttachDefaultPhoneWhenNoSim);

    // Attach wrong sim with if below 3 conditions satisfied:
    // 1. Hot plugout all sims and then plugin more than 1 sim.
    // 2. Capability sim is not sim 0.
    // 3. Default data is unselected.
    // To fix this issue, reattach right sim based on sims inserted status
    // and current prefer sim settings.
    if (mAttachDefaultPhoneWhenNoSim) {
        int currentSimCount = 0;
        for (int i = 0; i < mSimCount; i++) {
            if (getStatusManager(i)->getIntValue(RFX_STATUS_KEY_CARD_TYPE, -1) > 0) {
                currentSimCount++;
                if (currentSimCount > 1) {
                    mAttachDefaultPhoneWhenNoSim = false;
                    attachCapabilitySim();
                    break;
                }
            }
        }
    }
}

void RpDataAllowController::attachCapabilitySim() {
    char capabilitySim[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.vendor.radio.simswitch", capabilitySim, "1");
    int preferredSim= atoi(capabilitySim) - 1;
    int curPreferredSim = getNonSlotScopeStatusManager()->getIntValue(
            RFX_STATUS_KEY_PREFERRED_DATA_SIM);

    RFX_LOG_I(RP_DAC_LOG_TAG, "%s: old prefer sim %d, new prefer sim %d",
            __FUNCTION__, curPreferredSim, preferredSim);

    if (preferredSim == curPreferredSim) {
        return;
    }

    // Set capability sim as prefer sim, capability sim will be attahced.
    // Currently probably Framework has no prefer sim.
    if (preferredSim >= 0 && preferredSim < mSimCount) {
        getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_PREFERRED_DATA_SIM,
                preferredSim, true);
    }
}
