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
#include "RtcAtciController.h"
#include "rfx_properties.h"

/*****************************************************************************
 * Class RtcAtciController
 *****************************************************************************/
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringData, RfxStringData,
        RFX_MSG_REQUEST_OEM_HOOK_ATCI_INTERNAL);
RFX_REGISTER_DATA_TO_REQUEST_ID(RfxStringData, RfxStringData,
        RFX_MSG_REQUEST_ATCI_SPECIAL_COMMAND);
RFX_REGISTER_DATA_TO_URC_ID(RfxStringData, RFX_MSG_UNSOL_ATCI_RESPONSE);
RFX_IMPLEMENT_CLASS("RtcAtciController", RtcAtciController, RfxController);

const char *RtcAtciController::LOG_TAG_ATCI = "RtcAtciController";
const char *RtcAtciController::ENABLE_URC_PROP = "persist.vendor.service.atci_urc.enable";

void RtcAtciController::onInit() {
    logD(LOG_TAG_ATCI, "[%s] enter", __FUNCTION__);
    RfxController::onInit();
    const int request_id_list[] = {
        RFX_MSG_REQUEST_OEM_HOOK_ATCI_INTERNAL,
    };
    registerToHandleRequest(request_id_list,
            sizeof(request_id_list)/sizeof(const int), NORMAL);
}

bool RtcAtciController::onHandleAtciRequest(const sp<RfxMessage>& message) {
    int messageId = message->getId();
    logD(LOG_TAG_ATCI, "[%s] message: %d %s", __FUNCTION__, messageId, idToString(messageId));
    switch (messageId) {
        case RFX_MSG_REQUEST_OEM_HOOK_ATCI_INTERNAL: {
            String8 command = String8((char *)message->getData()->getData());
            if (command == String8("AT+CAPL")) {
                sp<RfxMessage> newMessage = RfxMessage::obtainRequest(
                        RFX_MSG_REQUEST_ATCI_SPECIAL_COMMAND,
                        RfxStringData((char *)command.string()), message);
                requestToMcl(newMessage);
            } else if (command.find("AT%EURC") >= 0) {
                logD(LOG_TAG_ATCI, "[%s] message: %s", __FUNCTION__, command.string());
                RfxStringData data((char *)"\r\nOK\r\n");
                command.removeAll("\r");
                command.removeAll("\n");
                if (command == String8("AT%EURC=1")) {
                    rfx_property_set(ENABLE_URC_PROP, "1");
                } else if (command == String8("AT%EURC=0")) {
                    rfx_property_set(ENABLE_URC_PROP, "0");
                } else {
                    data = RfxStringData((char *)"\r\nERROR\r\n");
                }
                responseToRilj(RfxMessage::obtainResponse(getSlotId(),
                        RFX_MSG_REQUEST_OEM_HOOK_ATCI_INTERNAL, RIL_E_SUCCESS, data, message));
            } else {
                requestToMcl(message);
            }
            break;
        }

        default:
            logE(LOG_TAG_ATCI, "[%s] should not be here", __FUNCTION__);
            return false;
    }
    return true;
}

bool RtcAtciController::onHandleAtciResponse(const sp<RfxMessage>& message) {
    int messageId = message->getId();
    logD(LOG_TAG_ATCI, "[%s] message: %d %s", __FUNCTION__, messageId, idToString(messageId));
    switch (messageId) {
        case RFX_MSG_REQUEST_OEM_HOOK_ATCI_INTERNAL:
            responseToRilj(message);
            break;

        case RFX_MSG_REQUEST_ATCI_SPECIAL_COMMAND: {
            sp<RfxMessage> response = RfxMessage::obtainResponse(
                    RFX_MSG_REQUEST_OEM_HOOK_ATCI_INTERNAL, message);
            responseToRilj(response);
            break;
        }

        default:
            logE(LOG_TAG_ATCI, "[%s] should not be here", __FUNCTION__);
            return false;
    }
    return true;
}
