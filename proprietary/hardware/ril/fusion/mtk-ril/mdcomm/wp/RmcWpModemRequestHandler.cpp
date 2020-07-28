/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include <telephony/mtk_ril.h>
#include "RfxLog.h"
#include "RfxRilUtils.h"
#include "RmcWpModemRequestHandler.h"

#define WP_LOG_TAG "RmcWp"
#define WP_RELOAD_MODEM_TYPE (1)
#define WP_STORE_MODEM_TYPE (2)

// register handler to channel
RFX_IMPLEMENT_HANDLER_CLASS(RmcWpModemRequestHandler, RIL_CMD_PROXY_9);

RmcWpModemRequestHandler::RmcWpModemRequestHandler(int slot_id, int channel_id):RfxBaseHandler(slot_id, channel_id) {
    const int request[] = {
        RFX_MSG_REQUEST_WORLD_MODE_RELOAD_MODEM_TYPE,
        RFX_MSG_REQUEST_WORLD_MODE_STORE_MODEM_TYPE,
        RFX_MSG_REQUEST_WORLD_MODE_STORE_ID,
        RFX_MSG_REQUEST_WORLD_MODE_RELOAD_ID,
        RFX_MSG_REQUEST_WORLD_MODE_MODIFY_MODEM_TYPE,
    };

    registerToHandleRequest(request, sizeof(request)/sizeof(int));
}

RmcWpModemRequestHandler::~RmcWpModemRequestHandler() {
}

void RmcWpModemRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    logD(WP_LOG_TAG, "onHandleRequest: %d", msg->getId());
    int request = msg->getId();
    switch (request) {
        case RFX_MSG_REQUEST_WORLD_MODE_RELOAD_ID:
        case RFX_MSG_REQUEST_WORLD_MODE_RELOAD_MODEM_TYPE:
            requestReloadModem(msg);
            break;
        case RFX_MSG_REQUEST_WORLD_MODE_STORE_ID:
        case RFX_MSG_REQUEST_WORLD_MODE_STORE_MODEM_TYPE:
            requestStoreModem(msg);
            break;
        case RFX_MSG_REQUEST_WORLD_MODE_MODIFY_MODEM_TYPE:
            requestModifyModem(msg);
            break;
        default:
            assert(0);
            break;
    }
}

void RmcWpModemRequestHandler::requestModifyModem(const sp<RfxMclMessage>& msg) {
    int *pInt = (int *)msg->getData()->getData();
    int responseInfo[1] = {0};
    int applyType = pInt[0];
    int modemType = pInt[1];
    responseInfo[0] = applyType;
    if (applyType == WP_RELOAD_MODEM_TYPE) {
        RfxRilUtils::triggerCCCIIoctlEx(CCCI_IOC_RELOAD_MD_TYPE, &modemType);
    } else if (applyType == WP_STORE_MODEM_TYPE) {
        RfxRilUtils::triggerCCCIIoctlEx(CCCI_IOC_STORE_MD_TYPE, &modemType);
    }
    logD(WP_LOG_TAG, "requestStoreModem complete:%d, %d", applyType, modemType);
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(
            msg->getId(), RIL_E_SUCCESS, RfxIntsData(responseInfo, 1), msg, false);
    // response to TeleCore
    responseToTelCore(response);
    return;
}

void RmcWpModemRequestHandler::requestStoreModem(const sp<RfxMclMessage>& msg) {
    int *pInt = (int *)msg->getData()->getData();
    int modemType = pInt[0] & 0x00FF;

    RfxRilUtils::triggerCCCIIoctlEx(CCCI_IOC_STORE_MD_TYPE, &modemType);
    logD(WP_LOG_TAG, "requestStoreModem complete MDtype = %d", modemType);
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(
            msg->getId(), RIL_E_SUCCESS, RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);

    return;
}

void RmcWpModemRequestHandler::requestReloadModem(const sp<RfxMclMessage>& msg) {
    int *pInt = (int *)msg->getData()->getData();
    int modemType = pInt[0];

    RfxRilUtils::triggerCCCIIoctlEx(CCCI_IOC_RELOAD_MD_TYPE, &modemType);
    logD(WP_LOG_TAG, "requestReloadModem complete MDtype = %d", modemType);
    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(
            msg->getId(), RIL_E_SUCCESS, RfxVoidData(), msg, false);
    // response to TeleCore
    responseToTelCore(response);

    return;
}
