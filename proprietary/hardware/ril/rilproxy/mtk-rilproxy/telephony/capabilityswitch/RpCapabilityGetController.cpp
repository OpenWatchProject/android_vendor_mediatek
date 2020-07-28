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

#include <cutils/properties.h>
#include <telephony/mtk_ril.h>
#include <Errors.h>
#include "RpCapabilityGetController.h"
#include "RfxLog.h"
#include "RfxStatusManager.h"
#include "util/RpFeatureOptionUtils.h"
#include <cutils/jstring.h>
#include "RpIrController.h"

#define RFX_LOG_TAG "RpCapabilityGetController"
#define RAF_CDMA_GROUP (RAF_IS95A | RAF_IS95B | RAF_1xRTT)
#define RAF_EVDO_GROUP (RAF_EVDO_0 | RAF_EVDO_A | RAF_EVDO_B | RAF_EHRPD)


/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

RFX_IMPLEMENT_CLASS("RpCapabilityGetController", RpCapabilityGetController, RfxController);

RpCapabilityGetController::RpCapabilityGetController() : mCSlot(0) {
}

RpCapabilityGetController::~RpCapabilityGetController() {
}

void RpCapabilityGetController::onInit() {
    RfxController::onInit(); // Required: invoke super class implementation
    RFX_LOG_D(RFX_LOG_TAG, "RpCapabilityGetController, onInit");
    const int urc_id_list[] = {
        RIL_UNSOL_RADIO_CAPABILITY
    };

    const int request_id_list[] = {
        RIL_REQUEST_GET_RADIO_CAPABILITY
    };

    // register request & URC id list
    // NOTE. one id can only be registered by one controller
    if (RpFeatureOptionUtils::isC2kSupport()) {
        registerToHandleUrc(0, urc_id_list, (sizeof(urc_id_list)/sizeof(int)));
        registerToHandleUrc(1, urc_id_list, (sizeof(urc_id_list)/sizeof(int)));
        registerToHandleRequest(0, request_id_list, sizeof(request_id_list)/sizeof(const int));
        registerToHandleRequest(1, request_id_list, sizeof(request_id_list)/sizeof(const int));
        //registerToHandleUrc(urc_id_list, (sizeof(urc_id_list)/sizeof(int)));
    }
    mCSlot = 0;
}

bool RpCapabilityGetController::onHandleUrc(const sp<RfxMessage>& message) {
    RFX_LOG_D(RFX_LOG_TAG, "onHandleUrc, handle: %s", urcToString(message->getId()));
    int msg_id = message->getId();
    char property_value[PROPERTY_VALUE_MAX] = {0};
    int session_id = -1;
    int rat;

    property_get("vendor.ril.rc.session.id1", property_value, "-1");
    session_id = atoi(property_value);

    switch (msg_id) {
    case RIL_UNSOL_RADIO_CAPABILITY: {
            RFX_LOG_D(RFX_LOG_TAG, "onHandleUrc,slotId= %d", message->getSlotId());
            int modemOffStateUrc = getNonSlotScopeStatusManager()->getIntValue(
                       RFX_STATUS_KEY_MODEM_OFF_STATE, MODEM_OFF_IN_IDLE);
            if (modemOffStateUrc == MODEM_OFF_BY_SIM_SWITCH && getSession(message) != -1 &&
                getSession(message) == session_id) {
                // trigger IR to suspend network
                RpIrController *irController =
                        (RpIrController *)findController(message->getSlotId(),
                        RFX_OBJ_CLASS_INFO(RpIrController));
                irController->resetSuspendNetwork();

                RFX_LOG_D(RFX_LOG_TAG, "Reset modemOffState on URC stag");
                getNonSlotScopeStatusManager()->setIntValue(RFX_STATUS_KEY_MODEM_OFF_STATE,
                                      MODEM_OFF_IN_IDLE);
                getNonSlotScopeStatusManager()->setIntValue(
                        RFX_STATUS_KEY_CAPABILITY_SWITCH_STATE, CAPABILITY_SWITCH_STATE_IDLE);
            }
            rat = getRadioAcessFamily(message);
            if (message->getSlotId() == mCSlot) {
                rat |= (RAF_CDMA_GROUP | RAF_EVDO_GROUP);
            } else {
                rat &= ~(RAF_CDMA_GROUP | RAF_EVDO_GROUP);
            }

            getStatusManager(message->getSlotId())->setIntValue(RFX_STATUS_KEY_SLOT_CAPABILITY, rat);
            if (session_id == -1) {
                responseToRilj(message);
            }
        }
        break;
    default: {
            RFX_LOG_E(RFX_LOG_TAG, "onHandleResponse, unknown urc:%d", msg_id);
        }
        break;
    }
    return true;
}

bool RpCapabilityGetController::onHandleResponse(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    int slot_id = message->getSlotId();
    // RFX_LOG_D(RFX_LOG_TAG, "onHandleResponse, handle: %s, slotId: %d", requestToString(msg_id), slot_id);

    switch (msg_id) {
    case RIL_REQUEST_GET_RADIO_CAPABILITY: {
            int status;
            RIL_RadioCapability rc;
            int t;
            Parcel* p_msg = message->getParcel();
            sp<RfxMessage> response = RfxMessage::obtainResponse(RIL_E_SUCCESS, message);
            Parcel* p_rsp = response->getParcel();

            memset (&rc, 0, sizeof(RIL_RadioCapability));
            status = p_msg->readInt32(&rc.version);
            status = p_msg->readInt32(&rc.session);
            status = p_msg->readInt32(&rc.phase);
            status = p_msg->readInt32(&rc.rat);
            //status = readStringFromParcelInplace(p_msg, rc.logicalModemUuid, sizeof(rc.logicalModemUuid));
            strncpy(rc.logicalModemUuid, String8(p_msg->readString16()).string(),
                    MAX_UUID_LENGTH - 1);
            rc.logicalModemUuid[MAX_UUID_LENGTH - 1] = '\0';
            status = p_msg->readInt32(&rc.status);

            // update C capability
            if (slot_id == mCSlot) {
                rc.rat |= (RAF_CDMA_GROUP | RAF_EVDO_GROUP);
            } else {
                rc.rat &= ~(RAF_CDMA_GROUP | RAF_EVDO_GROUP);
            }

            p_rsp->writeInt32(rc.version);
            p_rsp->writeInt32(rc.session);
            p_rsp->writeInt32(rc.phase);
            p_rsp->writeInt32(rc.rat);
            writeStringToParcel(p_rsp, rc.logicalModemUuid);
            p_rsp->writeInt32(rc.status);

            RFX_LOG_D(RFX_LOG_TAG, "handle:%s, slotId:%d, responseToRilj: %d, %d, %d, %d, %s, %d",
                    requestToString(msg_id), slot_id, rc.version, rc.session, rc.phase, rc.rat,
                    rc.logicalModemUuid, rc.status);
            getStatusManager(slot_id)->setIntValue(RFX_STATUS_KEY_SLOT_CAPABILITY, rc.rat);
            responseToRilj(response);
        }
        break;

    default:
        RFX_LOG_E(RFX_LOG_TAG, "onHandleResponse, unknown message:%d", msg_id);
        break;
    }
    return true;

}

int RpCapabilityGetController::getRadioAcessFamily(const sp<RfxMessage>& message) {
    int status;
    int request;
    int token;
    RIL_RadioCapability rc;
    int t = 0;
    Parcel* p = message->getParcel();

    memset (&rc, 0, sizeof(RIL_RadioCapability));

    status = p->readInt32(&t);
    rc.version = (int)t;
    //if (status != NO_ERROR) {
    //    goto invalid;
    //}

    status = p->readInt32(&t);
    rc.session= (int)t;
    //if (status != NO_ERROR) {
    //    goto invalid;
    //}

    status = p->readInt32(&t);
    rc.phase= (int)t;
    //if (status != NO_ERROR) {
    //    goto invalid;
    //}

    status = p->readInt32(&t);
    rc.rat = (int)t;
    //if (status != NO_ERROR) {
    //    goto invalid;
    //}

    //status = readStringFromParcelInplace(p, rc.logicalModemUuid, sizeof(rc.logicalModemUuid));
    //if (status != NO_ERROR) {
    //    goto invalid;
    //}

    //status = p->readInt32(&t);
    //rc.status = (int)t;

    //if (status != NO_ERROR) {
    //    goto invalid;
    //}
    RFX_LOG_D(RFX_LOG_TAG, "getRadioAcessFamily,rc.rat = %d",rc.rat);
    return rc.rat;

}

int RpCapabilityGetController::getSession(const sp<RfxMessage>& message) {
    int status;
    RIL_RadioCapability rc;
    int t = 0;
    Parcel* p = message->getParcel();

    memset (&rc, 0, sizeof(RIL_RadioCapability));
    message->resetParcelDataStartPos();
    status = p->readInt32(&t);
    rc.version = (int)t;
    status = p->readInt32(&t);
    rc.session= (int)t;
    message->resetParcelDataStartPos();
    RFX_LOG_D(RFX_LOG_TAG, "getSession,rc.session = %d",rc.session);
    return rc.session;
}

const char* RpCapabilityGetController::requestToString(int reqId) {
    switch (reqId) {
        case RIL_REQUEST_SET_RADIO_CAPABILITY:
            return "RIL_REQUEST_SET_RADIO_CAPABILITY";

        case RIL_REQUEST_GET_RADIO_CAPABILITY:
            return "RIL_REQUEST_GET_RADIO_CAPABILITY";

        default:
            RFX_LOG_E(RFX_LOG_TAG, "requestToString, reqId: %d", reqId);
            return "UNKNOWN_REQUEST";
    }
}

const char* RpCapabilityGetController::urcToString(int urcId) {
    switch (urcId) {
        case RIL_UNSOL_RADIO_CAPABILITY:
            return "RIL_UNSOL_RADIO_CAPABILITY";

        default:
            RFX_LOG_E(RFX_LOG_TAG, "requestToString, urcId: %d", urcId);
            return "UNKNOWN_URC";
    }
}

void RpCapabilityGetController::writeStringToParcel(Parcel *p, const char *s) {
    char16_t *s16;
    size_t s16_len = 0;
    s16 = strdup8to16(s, &s16_len);
    p->writeString16(s16, s16_len);
    free(s16);
}

void RpCapabilityGetController::updateRadioCapability(int cslot) {
    char property_value[PROPERTY_VALUE_MAX] = { 0 };
    int session_id = 0;
    int radio_capability;
    Parcel *p;
    sp<RfxMessage> urcToRilj;

    property_get("vendor.ril.rc.session.id1", property_value, "-1");
    session_id = atoi(property_value);
    // RFX_LOG_D(RFX_LOG_TAG, "updateRadioCapability cslot=%d, sessionId:%d", cslot, session_id);
    mCSlot = cslot;

    for (int i = 0; i < RFX_SLOT_COUNT; i++) {
        radio_capability = getStatusManager(i)->getIntValue(RFX_STATUS_KEY_SLOT_CAPABILITY, 0);
        if (i == cslot) {
            radio_capability |= (RAF_CDMA_GROUP | RAF_EVDO_GROUP);
        } else {
            radio_capability &= ~(RAF_CDMA_GROUP | RAF_EVDO_GROUP);
        }

        RFX_LOG_D(RFX_LOG_TAG, "updateRadioCapability, cslot=%d, sessionId=%d, capability[%d]=%d",
                cslot, session_id, i, radio_capability);
        getStatusManager(i)->setIntValue(RFX_STATUS_KEY_SLOT_CAPABILITY, radio_capability, false, false);

        urcToRilj = RfxMessage::obtainUrc(i, RIL_UNSOL_RADIO_CAPABILITY);
        p = urcToRilj->getParcel();
        p->writeInt32(RIL_RADIO_CAPABILITY_VERSION);
        p->writeInt32(session_id);
        p->writeInt32(RC_PHASE_UNSOL_RSP);
        p->writeInt32(radio_capability);
        writeStringToParcel(p, "modem_sys3");
        p->writeInt32(RC_STATUS_SUCCESS);

        responseToRilj(urcToRilj);
    }
}

