/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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
#include "RfxMessageId.h"
#include "RfxStringData.h"
#include "RmcAtciCommonRequestHandler.h"

/*****************************************************************************
 * Class RmcAtciCommonRequestHandler
 *****************************************************************************/
RFX_IMPLEMENT_HANDLER_CLASS(RmcAtciCommonRequestHandler, RIL_CMD_PROXY_6);

RmcAtciCommonRequestHandler::RmcAtciCommonRequestHandler(int slot_id, int channel_id) :
        RmcAtciRequestHandler(slot_id, channel_id) {
    const int request[] = {
        RFX_MSG_REQUEST_OEM_HOOK_ATCI_INTERNAL
    };
    const int event[] = {
        RFX_MSG_EVENT_RAW_URC,
    };
    const char* urc[] = {
        (char *) "+ERXRSSI",
        (char *) "+ERSSI",
    };
    registerToHandleRequest(request, sizeof(request)/sizeof(int));
    registerToHandleEvent(event, sizeof(event)/sizeof(int));
    registerToHandleURC(urc, sizeof(urc)/sizeof(char*));
}

void RmcAtciCommonRequestHandler::onHandleRequest(const sp<RfxMclMessage>& msg) {
    int messageId = msg->getId();
    logD(LOG_TAG_ATCI, "[%s] message: %d %s", __FUNCTION__, messageId, idToString(messageId));
    switch (messageId) {
        case RFX_MSG_REQUEST_OEM_HOOK_ATCI_INTERNAL:
            handleOemHookAtciInternalRequest(msg);
            break;

        default:
            logE(LOG_TAG_ATCI, "[%s] should not be here", __FUNCTION__);
            break;
    }
}

void RmcAtciCommonRequestHandler::onHandleEvent(const sp<RfxMclMessage>& msg) {
    int messageId = msg->getId();
    switch (messageId) {
        case RFX_MSG_EVENT_RAW_URC:
            handleMsgEventRawUrc(msg);
            break;

        default:
            logE(LOG_TAG_ATCI, "[%s] should not be here", __FUNCTION__);
            break;
    }
}
