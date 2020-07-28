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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxMessageId.h"
#include "RmcOpSCBMUrcHandler.h"
#include <telephony/mtk_ril.h>
#include "RfxStatusDefs.h"
#include "RfxVoidData.h"
#include "RfxIntsData.h"

using ::android::String8;

static const char* urcList[] = {
    "+CIEV: 103"
};


RFX_IMPLEMENT_HANDLER_CLASS(RmcOpSCBMUrcHandler, RIL_CMD_PROXY_URC);
RFX_REGISTER_DATA_TO_URC_ID(RfxVoidData, RFX_MSG_UNSOL_ENTER_SCBM);
RFX_REGISTER_DATA_TO_URC_ID(RfxVoidData, RFX_MSG_UNSOL_EXIT_SCBM);


/*****************************************************************************
 * Class RfxController
 *****************************************************************************/
RmcOpSCBMUrcHandler::RmcOpSCBMUrcHandler(int slot_id, int channel_id) :
    RfxBaseHandler(slot_id, channel_id) {
    setTag(String8("RmcOpSCBMUrc"));
    const char **p = urcList;
    registerToHandleURC(p, sizeof(urcList)/sizeof(char*));
}

RmcOpSCBMUrcHandler::~RmcOpSCBMUrcHandler() {
}

void RmcOpSCBMUrcHandler::onHandleUrc(const sp<RfxMclMessage>& msg) {
    char* urc = msg->getRawUrc()->getLine();

    if (strstr(urc, "+CIEV: 103") != NULL ) {
        handleSCBMStatusMessage(msg);
    }
}

void RmcOpSCBMUrcHandler::handleSCBMStatusMessage(const sp<RfxMclMessage>& msg) {
    int type = 0, value = 0, ret = 0, urcType = 0;
    RfxAtLine *line = msg->getRawUrc();

    line->atTokStart(&ret);
    if (ret < 0) { return; }

    type = line->atTokNextint(&ret);
    if (ret < 0) { return; }

    value = line->atTokNextint(&ret);
    if (ret < 0) { return; }

    /**
     * 103: SCBM Mode Indicator
     * value = 0: Mode off
     * value = 1: Mode on success
     * value = 2: Mode on fail
     * value = 3: Mode on restart
     */
    if (type == 103) {
        if (value == 1 || value == 3) {
            urcType = RFX_MSG_UNSOL_ENTER_SCBM;
        } else if (value == 0) {
            urcType = RFX_MSG_UNSOL_EXIT_SCBM;
        }
    }
    sp<RfxMclMessage> urc = RfxMclMessage::obtainUrc(urcType, m_slot_id, RfxVoidData());
    responseToTelCore(urc);
}
