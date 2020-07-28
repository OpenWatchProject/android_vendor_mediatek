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
#include "RpMalController.h"
#include "Parcel.h"
#include "RfxMainThread.h"
#include "RfxSocketState.h"
#include "RfxLog.h"
#include "utils/RefBase.h"
#include "utils/Errors.h"
#include <cutils/properties.h>
#include <stdint.h>
#include <string.h>
#include "util/RpFeatureOptionUtils.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))

extern "C" void setRadioState(RIL_RadioState newState, RIL_SOCKET_ID rid);

static const int INVALID_SERIAL = -1;

/// M: add for op09 default volte setting @{
#define PROPERTY_3G_SIM         "persist.vendor.radio.simswitch"
#define PROPERTY_VOLTE_STATE    "persist.vendor.radio.volte_state"
#define PROPERTY_VOLTE_ENABLE   "persist.vendor.mtk.volte.enable"
/// @}

RFX_IMPLEMENT_CLASS("RpMalController", RpMalController, RfxController);

RpMalController::RpMalController() {
}

RpMalController::~RpMalController() {
}

static const int INVALID_VALUE = -1;

void RpMalController::onInit() {
    RFX_LOG_D(RP_MAL_CONTROLLER_TAG, "RpMalController init()");
    RfxController::onInit();

    static const int gsmRequest[] = {
        RIL_LOCAL_REQUEST_AT_COMMAND_WITH_PROXY,
    };

    static const int cdmaRequest[] = {
        RIL_LOCAL_C2K_REQUEST_AT_COMMAND_WITH_PROXY_CDMA,
    };

    for (int i = 0; i < SIM_COUNT; i++) {
        registerToHandleRequest(i, gsmRequest, ARRAY_LENGTH(gsmRequest));
        if (RpFeatureOptionUtils::isC2kSupport()) {
            registerToHandleRequest(i, cdmaRequest, ARRAY_LENGTH(cdmaRequest));
        }
    }

    RFX_LOG_D(RP_MAL_CONTROLLER_TAG, "MAL socket is set to ril-proxy-mal");
    property_set("vendor.ril.mal.socket", "rilproxy-mal");

    /// M: add for op09 volte setting @{
    initVolteSettingStatus();
    /// @}

    RFX_LOG_D(RP_MAL_CONTROLLER_TAG, "RpMalController init() done:");
}


bool RpMalController::onPreviewMessage(const sp<RfxMessage>& message) {
    if (message -> getType() == REQUEST) {
        if (canHandleRequest(message)) {
            return true;
        } else {
            return false;
        }
    } else {
        RFX_LOG_D(RP_MAL_CONTROLLER_TAG, "Not Request, don't do check");
        return true;
    }
}

bool RpMalController::onCheckIfResumeMessage(const sp<RfxMessage>& message) {
    if (message -> getType() == REQUEST) {
        if (canHandleRequest(message)) {
            return true;
        } else {
            return false;
        }
    } else {
        return true;
    }
}

bool RpMalController::canHandleRequest(const sp<RfxMessage>& message) {
    RfxSocketState socketState = getStatusManager(message -> getSlotId())
        -> getSocketStateValue(RFX_STATUS_KEY_SOCKET_STATE);
    if (message -> getDest() == RADIO_TECH_GROUP_GSM) {
        bool socketIsConnected = socketState.getSocketState(socketState.SOCKET_GSM);
        RFX_LOG_D(RP_MAL_CONTROLLER_TAG, "GSM socketIsConnected %s",
                socketIsConnected ? "true" : "false");
        return socketIsConnected ? true : false;
    } else {
        bool socketIsConnected = socketState.getSocketState(socketState.SOCKET_C2K);
        RFX_LOG_D(RP_MAL_CONTROLLER_TAG, "CDMA socketIsConnected %s",
                socketIsConnected ? "true" : "false");
        return socketIsConnected ? true : false;
    }
    return true;
}

bool RpMalController::onHandleRequest(const sp<RfxMessage>& message) {
    int msg_id = message -> getId();
    int pToken = message -> getPToken();
    RFX_LOG_D(RP_MAL_CONTROLLER_TAG, "RpMalController: handle request id %d", msg_id);

    switch (msg_id) {
    case RIL_LOCAL_REQUEST_AT_COMMAND_WITH_PROXY:
        requestToRild(message);
        /// M: add for op09 volte setting @{
        setVolteSettingStatus(message);
        /// @}
        break;
    case RIL_LOCAL_C2K_REQUEST_AT_COMMAND_WITH_PROXY_CDMA:
        requestToRild(message);
        break;
    default:
        break;
    }

    return true;
}

bool RpMalController::onHandleResponse(const sp<RfxMessage>& message) {
    int msg_id = message -> getId();
    RFX_LOG_D(RP_MAL_CONTROLLER_TAG, "RpMalController: handle response id %d", msg_id);

    switch (msg_id) {
    case RIL_LOCAL_REQUEST_AT_COMMAND_WITH_PROXY:
        responseToRilj(message);
        break;
    case RIL_LOCAL_C2K_REQUEST_AT_COMMAND_WITH_PROXY_CDMA:
        responseToRilj(message);
        break;
    default:
        break;
    }

    return true;
}

/// M: add for op09 volte setting @{
void RpMalController::initVolteSettingStatus() {
    if (!RpFeatureOptionUtils::isCtVolteSupport()) {
        return;
    }

    int setValue;
    // int mainSlotId = getStatusManager(RFX_SLOT_ID_UNKNOWN)->getIntValue(
    //                RFX_STATUS_KEY_MAIN_CAPABILITY_SLOT, 0);
    char tempstr[PROPERTY_VALUE_MAX] = { 0 };
    property_get(PROPERTY_3G_SIM, tempstr, "1");
    int mainSlotId = atoi(tempstr) - 1;

    char volteEnable[PROPERTY_VALUE_MAX] = { 0 };
    property_get(PROPERTY_VOLTE_ENABLE, volteEnable, "0");
    int volteValue = atoi(volteEnable);
    RFX_LOG_D(RP_MAL_CONTROLLER_TAG, "initVolteSettingStatus: volteEnable = %d, mainSlotId = %d",
            volteValue, mainSlotId);

    for (int slotCheckingBit = 0; slotCheckingBit < SIM_COUNT; slotCheckingBit++) {
        if (RpFeatureOptionUtils::isMultipleImsSupport() == 0) {
            if (slotCheckingBit == mainSlotId) {
                setValue = volteValue;
            } else {
                setValue = 0;
            }
        } else {
            setValue = (volteValue >> slotCheckingBit) & 0x01;
        }

        getStatusManager(slotCheckingBit)
            -> setIntValue(RFX_STATUS_KEY_VOLTE_STATE, setValue);
    }
}

void RpMalController::setVolteSettingStatus(const sp<RfxMessage>& message) {
    if (!RpFeatureOptionUtils::isCtVolteSupport()) {
        return;
    }

    int32_t status;
    int32_t request;
    int32_t token;
    int32_t channel;
    int32_t len;
    const void *data;
    Parcel *p = message->getParcel();
    p->setDataPosition(0);
    status = p->readInt32(&request);
    status = p->readInt32(&token);
    status = p->readInt32(&channel);
    status = p->readInt32(&len);
    int slot_id = message->getSlotId();
    if (status == android::NO_ERROR && len > 0) {
        data = p->readInplace(len);
        //RLOGD("setVolteSettingStatus: data = %s, request:%d, token:%d, ch:%d, len:%d, slot:%d ",
        //      (char*)data, request, token, channel, len, slot_id);
        if (data != NULL && strncmp((char*)data, "AT+EIMSVOLTE", 12) == 0) {
            int setValue;
            char value[2] = { 0 };
            value[0] = ((char*) data)[13];
            // if (RpFeatureOptionUtils::isOp09())
            {
                int sim_state = getStatusManager(slot_id)->getIntValue(
                                RFX_STATUS_KEY_SIM_STATE);
                if (sim_state == RFX_SIM_STATE_NOT_READY) {
                    RFX_LOG_D(RP_MAL_CONTROLLER_TAG, "setVolteSettingStatus: skip set for invalid sim type");
                    return;
                } else {
                    char volteEnable[PROPERTY_VALUE_MAX] = { 0 };
                    property_get(PROPERTY_VOLTE_ENABLE, volteEnable, "0");
                    int enalbeValue = atoi(volteEnable);
                    char volteState[PROPERTY_VALUE_MAX] = { 0 };
                    property_get(PROPERTY_VOLTE_STATE, volteState, "0");
                    int stateValue = atoi(volteState);
                    if (RpFeatureOptionUtils::isMultipleImsSupport() == 0) {
                        if (enalbeValue == 1) {
                            stateValue = stateValue | (1 << slot_id);
                        } else {
                            stateValue = stateValue & (~(1 << slot_id));
                        }
                    } else {
                        if (((enalbeValue >> slot_id) & 0x01) == 1) {
                            stateValue = stateValue | (1 << slot_id);
                        } else {
                            stateValue = stateValue & (~(1 << slot_id));
                        }
                    }
                    char temp[3] = { 0 };
                    if (stateValue > 10) {
                        temp[0] = '1';
                        temp[1] = '0' + (stateValue - 10);
                    } else {
                        temp[0] = '0' + stateValue;
                    }
                    property_set(PROPERTY_VOLTE_STATE, temp);
                    RFX_LOG_D(RP_MAL_CONTROLLER_TAG, "setVolteSettingStatus: state = %d, %s",
                            stateValue, temp);
                }
            }

            for (int i = 0; i < SIM_COUNT; i++) {
                if (i == slot_id) {
                    setValue = atoi(value);
                } else {
                    setValue = 0;  // false
                }
                getStatusManager(i)
                    -> setIntValue(RFX_STATUS_KEY_VOLTE_STATE, setValue);
            }
        }
    }
}
/// @}
