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
#include "RpSimControllerBase.h"
#include "RpGsmSimController.h"
#include "RfxStatusDefs.h"
#include <log/log.h>
#include <cutils/properties.h>
#include <cutils/jstring.h>


/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpGsmSimController", RpGsmSimController, RpSimControllerBase);

RpGsmSimController::RpGsmSimController() {
}

RpGsmSimController::~RpGsmSimController() {
}

void RpGsmSimController::onInit() {
    RpSimControllerBase::onInit(); // Required: invoke super class implementation

    RLOGD("[RpGsmSimController] onInit (slot %d)", getSlotId());
}

#if 0
void RpSimController::onAvailable(RfxStatusKeyEnum key,
    RfxVariant old_value, RfxVariant value) {
}

void RpSimController::onPowerOn(RfxStatusKeyEnum key,
    RfxVariant old_value, RfxVariant value) {
}
#endif

bool RpGsmSimController::onHandleRequest(const sp<RfxMessage>& message) {
    RLOGD("[RpSimController] handle req %s (slot %d)", requestToString(message->getId()),
            getSlotId());
    return true;
}

bool RpGsmSimController::onHandleUrc(const sp<RfxMessage>& message) {
    RLOGD("[RpSimController] handle urc %s (slot %d)", urcToString(message->getId()),
            getSlotId());
    return true;
}

bool RpGsmSimController::onHandleResponse(const sp<RfxMessage>& message) {
    RLOGD("[RpSimController] handle %s response (slot %d)", requestToString(message->getId()),
            getSlotId());

    return true;
}


void RpGsmSimController::handleEnterNetworkDepersonalizationReq(
        const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleEnterNetworkDepersonalizationReq (slot %d)", getSlotId());

    sendReq(message);
}

void RpGsmSimController::handleEnterNetworkDepersonalizationRsp(
        const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleEnterNetworkDepersonalizationRsp (slot %d)", getSlotId());

    sendRsp(message);
}

void RpGsmSimController::handleSetSimNetworkLockReq(const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleSetSimNetworkLockReq (slot %d)", getSlotId());

    sendReq(message);
}

void RpGsmSimController::handleSetSimNetworkLockRsp(const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleSetSimNetworkLockRsp (slot %d)", getSlotId());

    sendRsp(message);
}

void RpGsmSimController::handlePinPukReq(const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handlePinPukReq (slot %d)", getSlotId());

    sendReq(message);
}

void RpGsmSimController::handlePinPukRsp(const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handlePinPukRsp (slot %d)", getSlotId());

    sendRsp(message);
}

void RpGsmSimController::handleQuerySimFacilityReq(const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleQuerySimFacilityReq (slot %d)", getSlotId());

    sendReq(message);
}

void RpGsmSimController::handleQuerySimFacilityRsp(const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleQuerySimFacilityRsp (slot %d)", getSlotId());

    sendRsp(message);
}

void RpGsmSimController::handleSetSimFacilityReq(const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleSetSimFacilityReq (slot %d)", getSlotId());

    sendReq(message);
}

void RpGsmSimController::handleSetSimFacilityRsp(const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleSetSimFacilityRsp (slot %d)", getSlotId());

    sendRsp(message);
}

void RpGsmSimController::handleIccOpenChannelReq(const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleIccOpenChannelReq (slot %d)", getSlotId());

    sendReq(message);
}

void RpGsmSimController::handleIccOpenChannelRsp(const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleIccOpenChannelRsp (slot %d)", getSlotId());

    sendRsp(message);
}

void RpGsmSimController::handleIccCloseChannelReq(const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleIccCloseChannelReq (slot %d)", getSlotId());

    sendReq(message);
}

void RpGsmSimController::handleIccCloseChannelRsp(const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleIccCloseChannelRsp (slot %d)", getSlotId());

    sendRsp(message);
}

void RpGsmSimController::handleTransmitApduReq(const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleTransmitApduReq (slot %d)", getSlotId());

    sendReq(message);
}

void RpGsmSimController::handleTransmitApduRsp(const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleTransmitApduRsp (slot %d)", getSlotId());

    sendRsp(message);
}

void RpGsmSimController::handleGetAtrReq(const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleGetAtrReq (slot %d)", getSlotId());

    sendReq(message);
}

void RpGsmSimController::handleGetAtrRsp(const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleGetAtrRsp (slot %d)", getSlotId());

    sendRsp(message);
}

void RpGsmSimController::handleSetUiccSubscriptionReq(const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleSetUiccSubscriptionReq (slot %d)", getSlotId());

    sendReq(message);
}

void RpGsmSimController::handleSetUiccSubscriptionRsp(const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleSetUiccSubscriptionRsp (slot %d)", getSlotId());

    sendRsp(message);
}

void RpGsmSimController::handleSimIoReq(const sp<RfxMessage>& message) {
    sendReq(message);
}

void RpGsmSimController::handleSimIoRsp(const sp<RfxMessage>& message) {
    sendRsp(message);
}

void RpGsmSimController::handleSimAuthenticationReq(const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleSimAuthenticationReq (slot %d)", getSlotId());

    sendReq(message);
}

void RpGsmSimController::handleSimAuthenticationRsp(const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleSimAuthenticationRsp (slot %d)", getSlotId());

    sendRsp(message);
}

void RpGsmSimController::handleSimTransmitApduBasicReq(
        const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleSimTransmitApduBasicReq (slot %d)", getSlotId());

    sendReq(message);
}

void RpGsmSimController::handleSimTransmitApduBasicRsp(
        const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleSimTransmitApduBasicRsp (slot %d)", getSlotId());

    sendRsp(message);
}

void RpGsmSimController::handleSubsidylockRequest(
        const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleSubsidylockRequest (slot %d)", getSlotId());

    sendReq(message);
}

void RpGsmSimController::handleSubsidylockResponse(
        const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleSubsidylockResponse (slot %d)", getSlotId());

    sendRsp(message);
}

void RpGsmSimController::handleSetAllowedCarriersReq(
        const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleSetAllowedCarriersReq (slot %d)", getSlotId());

    sendReq(message);
}

void RpGsmSimController::handleSetAllowedCarriersRsp(
        const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleSetAllowedCarriersRsp (slot %d)", getSlotId());

    sendRsp(message);
}

void RpGsmSimController::handleGetAllowedCarriersReq(
        const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleGetAllowedCarriersReq (slot %d)", getSlotId());

    sendReq(message);
}

void RpGsmSimController::handleGetAllowedCarriersRsp(
        const sp<RfxMessage>& message) {
    RLOGD("[RpGsmSimController] handleGetAllowedCarriersRsp (slot %d)", getSlotId());

    sendRsp(message);
}

void RpGsmSimController::sendReq(const sp<RfxMessage>& message) {
    // Pass the request to RILD directly
    //size_t dataPos = 0;
    //Parcel* req = message->getParcel();
    sp<RfxMessage> request = RfxMessage::obtainRequest(RADIO_TECH_GROUP_GSM,
        message->getId(), message, true);

    // Clone Parcel
    //dataPos = req->dataPosition();
    //request->getParcel()->appendFrom(req, dataPos, (req->dataSize() - dataPos));
    //request->getParcel()->setDataPosition(0);

    // Send to GSM Rild
    requestToRild(request);
}

void RpGsmSimController::sendRsp(const sp<RfxMessage>& message) {
    Parcel *rsq = message->getParcel();
    size_t dataPos = 0;
    Parcel *p = NULL;

    sp<RfxMessage> response = RfxMessage::obtainResponse(message->getError(),
                message,true);

    // Clone parcel
    //dataPos = rsq->dataPosition();
    //p = response->getParcel();

    //p->appendFrom(rsq, dataPos, (rsq->dataSize() - dataPos));
    //p->setDataPosition(0);

    // Send to RILJ directly
    responseToRilj(response);
}

