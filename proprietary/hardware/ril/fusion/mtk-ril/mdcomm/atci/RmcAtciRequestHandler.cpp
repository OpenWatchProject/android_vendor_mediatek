/* Copyright Statement:
 *
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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxStringData.h"
#include "RfxMessageId.h"
#include "RmcAtciRequestHandler.h"

/*****************************************************************************
 * Class RmcAtciRequestHandler
 *****************************************************************************/
RFX_IMPLEMENT_HANDLER_CLASS(RmcAtciRequestHandler, RIL_CMD_PROXY_6);
RFX_REGISTER_DATA_TO_EVENT_ID(RfxStringData, RFX_MSG_EVENT_RAW_URC);

const char *RmcAtciRequestHandler::ERROR_RESPONSE = "\r\nERROR\r\n";
const char *RmcAtciRequestHandler::ENABLE_URC_PROP = "persist.vendor.service.atci_urc.enable";
const char *RmcAtciRequestHandler::LOG_TAG_ATCI = "RmcAtciRequestHandler";

RmcAtciRequestHandler::RmcAtciRequestHandler(int slot_id, int channel_id) :
        RfxBaseHandler(slot_id, channel_id) {
}

void RmcAtciRequestHandler::handleOemHookAtciInternalRequest(const sp<RfxMclMessage>& msg) {
    const char *data = (const char *)msg->getData()->getData();
    logD(LOG_TAG_ATCI, "[%s] enter", __FUNCTION__);

    if (strncasecmp(data, "ATV0", 4) == 0) {
        logD(LOG_TAG_ATCI, "[%s] set isNumericSet on", __FUNCTION__);
        getMclStatusManager()->setBoolValue(RFX_STATUS_KEY_ATCI_IS_NUMERIC, true);
    } else if (strncasecmp(data, "ATV1", 4) == 0) {
        logD(LOG_TAG_ATCI, "[%s] set isNumericSet off", __FUNCTION__);
        getMclStatusManager()->setBoolValue(RFX_STATUS_KEY_ATCI_IS_NUMERIC, false);
    }

    sp<RfxAtResponse> atResponse = atSendCommandRaw(data);
    if (atResponse->getError() < 0) {
        logE(LOG_TAG_ATCI, "[%s] atSendCommandRaw fail: %d", __FUNCTION__, atResponse->getError());
        sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(RIL_E_GENERIC_FAILURE,
                RfxStringData((char *)ERROR_RESPONSE), msg);
        responseToTelCore(response);
        return;
    }

    String8 ret = String8("\r\n");
    for (RfxAtLine* line = atResponse->getIntermediates(); line != NULL; line = line->getNext()) {
        ret.appendFormat("%s\r\n", line->getLine());
    }
    ret.appendFormat("%s\r\n", atResponse->getFinalResponse()->getLine());

    sp<RfxMclMessage> response = RfxMclMessage::obtainResponse(RIL_E_SUCCESS,
            RfxStringData((char *)ret.string()), msg);
    responseToTelCore(response);
}

void RmcAtciRequestHandler::handleMsgEventRawUrc(const sp<RfxMclMessage>& msg) {
    char enabled[RFX_PROPERTY_VALUE_MAX] = {0};
    rfx_property_get(ENABLE_URC_PROP, enabled, "0");
    const char *data = (const char *)msg->getData()->getData();
    if (NULL != data && atoi(enabled) == 1) {
        sp<RfxMclMessage> urc = RfxMclMessage::obtainUrc(RFX_MSG_UNSOL_ATCI_RESPONSE, m_slot_id,
                RfxStringData((char *)String8::format("\r\n%s\r\n", data).string()));
        responseToTelCore(urc);
    }
}
