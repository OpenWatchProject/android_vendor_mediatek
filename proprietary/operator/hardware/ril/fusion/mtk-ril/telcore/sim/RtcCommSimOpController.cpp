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
#include "RfxController.h"
#include "RtcCommSimOpController.h"

#include "RfxMessageId.h"

/*****************************************************************************
 * Class RtcCommSimOpController
 *****************************************************************************/

#define RFX_LOG_TAG "RtcCommSimOp"

RFX_IMPLEMENT_CLASS("RtcCommSimOpController", RtcCommSimOpController, RtcCommSimController);

RtcCommSimOpController::RtcCommSimOpController() {
}

RtcCommSimOpController::~RtcCommSimOpController() {
}

void RtcCommSimOpController::onInit() {
    RtcCommSimController::onInit();
    const int request_id_list[] = {
        RFX_MSG_REQUEST_SEND_RSU_REQUEST,
        RFX_MSG_REQUEST_GET_SHARED_KEY,
        RFX_MSG_REQUEST_UPDATE_SIM_LOCK_SETTINGS,
        RFX_MSG_REQUEST_GET_SIM_LOCK_INFO,
        RFX_MSG_REQUEST_RESET_SIM_LOCK_SETTINGS,
        RFX_MSG_REQUEST_GET_MODEM_STATUS,
        RFX_MSG_REQUEST_ATT_GET_SHARED_KEY,
        RFX_MSG_REQUEST_ATT_UPDATE_SIM_LOCK_SETTINGS,
        RFX_MSG_REQUEST_ATT_GET_SIM_LOCK_VERSION,
        RFX_MSG_REQUEST_ATT_RESET_SIM_LOCK_SETTINGS,
        RFX_MSG_REQUEST_ATT_GET_MODEM_STATUS,
        RFX_MSG_REQUEST_ATT_GET_MODEM_CONFIG,
        RFX_MSG_REQUEST_ATT_GET_API_VERSION,
        RFX_MSG_REQUEST_ATT_GET_SIM_SLOTS,
    };

    registerToHandleRequest(request_id_list, sizeof(request_id_list)/sizeof(const int), DEFAULT);
}

void RtcCommSimOpController::onDeinit() {
    RfxController::onDeinit();
}

bool RtcCommSimOpController::onHandleRequest(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    switch(msg_id) {
        case RFX_MSG_REQUEST_SEND_RSU_REQUEST:
        case RFX_MSG_REQUEST_GET_SHARED_KEY:
        case RFX_MSG_REQUEST_UPDATE_SIM_LOCK_SETTINGS:
        case RFX_MSG_REQUEST_GET_SIM_LOCK_INFO:
        case RFX_MSG_REQUEST_RESET_SIM_LOCK_SETTINGS:
        case RFX_MSG_REQUEST_GET_MODEM_STATUS:
        case RFX_MSG_REQUEST_ATT_GET_SHARED_KEY:
        case RFX_MSG_REQUEST_ATT_UPDATE_SIM_LOCK_SETTINGS:
        case RFX_MSG_REQUEST_ATT_GET_SIM_LOCK_VERSION:
        case RFX_MSG_REQUEST_ATT_RESET_SIM_LOCK_SETTINGS:
        case RFX_MSG_REQUEST_ATT_GET_MODEM_STATUS:
        case RFX_MSG_REQUEST_ATT_GET_MODEM_CONFIG:
        case RFX_MSG_REQUEST_ATT_GET_API_VERSION:
        case RFX_MSG_REQUEST_ATT_GET_SIM_SLOTS:
            requestToMcl(message);
            return true;
        default:
            break;
    }
    return RtcCommSimController::onHandleRequest(message);
}

bool RtcCommSimOpController::onHandleResponse(const sp<RfxMessage>& message) {
    int msg_id = message->getId();
    switch(msg_id) {
        case RFX_MSG_REQUEST_SEND_RSU_REQUEST:
        case RFX_MSG_REQUEST_GET_SHARED_KEY:
        case RFX_MSG_REQUEST_UPDATE_SIM_LOCK_SETTINGS:
        case RFX_MSG_REQUEST_GET_SIM_LOCK_INFO:
        case RFX_MSG_REQUEST_RESET_SIM_LOCK_SETTINGS:
        case RFX_MSG_REQUEST_GET_MODEM_STATUS:
        case RFX_MSG_REQUEST_ATT_GET_SHARED_KEY:
        case RFX_MSG_REQUEST_ATT_UPDATE_SIM_LOCK_SETTINGS:
        case RFX_MSG_REQUEST_ATT_GET_SIM_LOCK_VERSION:
        case RFX_MSG_REQUEST_ATT_RESET_SIM_LOCK_SETTINGS:
        case RFX_MSG_REQUEST_ATT_GET_MODEM_STATUS:
        case RFX_MSG_REQUEST_ATT_GET_MODEM_CONFIG:
        case RFX_MSG_REQUEST_ATT_GET_API_VERSION:
        case RFX_MSG_REQUEST_ATT_GET_SIM_SLOTS:
            responseToRilj(message);
            return true;
        default:
            break;
    }
    return RtcCommSimController::onHandleResponse(message);
}

bool RtcCommSimOpController::onCheckIfRejectMessage(const sp<RfxMessage>& message,
        bool isModemPowerOff, int radioState) {
    int msgId = message->getId();
    if (!isModemPowerOff && (radioState == (int)RADIO_STATE_OFF) &&
        (msgId == RFX_MSG_REQUEST_SEND_RSU_REQUEST ||
         msgId == RFX_MSG_REQUEST_GET_SHARED_KEY ||
         msgId == RFX_MSG_REQUEST_UPDATE_SIM_LOCK_SETTINGS ||
         msgId == RFX_MSG_REQUEST_GET_SIM_LOCK_INFO ||
         msgId == RFX_MSG_REQUEST_RESET_SIM_LOCK_SETTINGS ||
         msgId == RFX_MSG_REQUEST_GET_MODEM_STATUS ||
         msgId == RFX_MSG_REQUEST_ATT_GET_SHARED_KEY ||
         msgId == RFX_MSG_REQUEST_ATT_UPDATE_SIM_LOCK_SETTINGS ||
         msgId == RFX_MSG_REQUEST_ATT_GET_SIM_LOCK_VERSION ||
         msgId == RFX_MSG_REQUEST_ATT_RESET_SIM_LOCK_SETTINGS ||
         msgId == RFX_MSG_REQUEST_ATT_GET_MODEM_STATUS ||
         msgId == RFX_MSG_REQUEST_ATT_GET_MODEM_CONFIG ||
         msgId == RFX_MSG_REQUEST_ATT_GET_API_VERSION ||
         msgId == RFX_MSG_REQUEST_ATT_GET_SIM_SLOTS)) {
        return false;
    }
    return RtcCommSimController::onCheckIfRejectMessage(message, isModemPowerOff, radioState);
}
