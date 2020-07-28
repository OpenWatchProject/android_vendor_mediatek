/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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
#include "RfxRootController.h"
#include "RtcSmsUtil.h"
#include "RtcSmsNSlotController.h"

/*****************************************************************************
 * Class RtcSmsParsingMessage
 *****************************************************************************/
RtcSmsParsingMessage::~RtcSmsParsingMessage() {
    RFX_LOG_V(SMS_TAG, "[RtcSmsParsingMessage]destroyMessage %d, 0x%zu",
            m_id, (size_t)m_parcel);
    if (m_parcel != NULL) {
        delete(m_parcel);
        m_parcel = NULL;
    }
}

sp<RtcSmsParsingMessage> RtcSmsParsingMessage::obtainMessage(int32_t id, Parcel* parcel) {
    sp<RtcSmsParsingMessage> new_msg = new RtcSmsParsingMessage();
    RFX_LOG_V(SMS_TAG, "[RtcSmsParsingMessage]obtainMessage %d, 0x%zu",
            id, (size_t)parcel);
    new_msg->m_id = id;
    new_msg->m_parcel = parcel;
    return new_msg;
};

/*****************************************************************************
 * Class RtcSmsHandler
 *****************************************************************************/
void RtcSmsHandler::sendMessage(sp<Looper> looper) {
    looper->sendMessage(this, m_dummyMsg);
}


/*****************************************************************************
 * Class RtcSmsParsingThreadHandler
 *****************************************************************************/
void RtcSmsParsingThreadHandler::handleMessage(const Message& message) {
    RFX_UNUSED(message);
    RfxRootController *root = RFX_OBJ_GET_INSTANCE(RfxRootController);
    RtcSmsNSlotController *ctrl = (RtcSmsNSlotController *)root->findController(
            RFX_OBJ_CLASS_INFO(RtcSmsNSlotController));
    if (ctrl != NULL) {
        ctrl->onHandleSmsMessage(m_msg);
    }
}

