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

#ifndef __RMC_CAT_URC_HANDLER_H__
#define __RMC_CAT_URC_HANDLER_H__

#define LOG_TAG "RmcCatUrcHandler"
#define NUM_STK_CALL_CTRL 3
#define EVENT_NOTIFY_BUFFER_LEN    10

#include "RfxBaseHandler.h"
#include "RfxCdmaInfoRecData.h"
#include "RfxStringData.h"
#include "RfxStringsData.h"
#include "RfxIntsData.h"
#include "RfxVoidData.h"
#include "RfxSimRefreshData.h"
#include "RfxMessageId.h"
#include "telephony/ril.h"


typedef enum {
    CMD_REFRESH = 0x01,
    CMD_MORE_TIME = 0x02,
    CMD_POLL_INTERVAL = 0x03,
    CMD_POLLING_OFF = 0x04,
    CMD_SETUP_EVENT_LIST = 0x05,
    CMD_SETUP_CALL = 0x10,
    CMD_SEND_SS = 0x11,
    CMD_SEND_USSD = 0x12,
    CMD_SEND_SMS = 0x13,
    CMD_DTMF = 0x14,
    CMD_LAUNCH_BROWSER = 0x15,
    CMD_PLAY_TONE = 0x20,
    CMD_DSPL_TXT = 0x21,
    CMD_GET_INKEY = 0x22,
    CMD_GET_INPUT = 0x23,
    CMD_SELECT_ITEM = 0x24,
    CMD_SETUP_MENU = 0x25,
    CMD_PROVIDE_LOCAL_INFO = 0x26,
    CMD_TIMER_MANAGER = 0x27,
    CMD_IDLE_MODEL_TXT = 0x28,
    CMD_PERFORM_CARD_APDU = 0x30,
    CMD_POWER_ON_CARD = 0x31,
    CMD_POWER_OFF_CARD = 0x32,
    CMD_GET_READER_STATUS = 0x33,
    CMD_RUN_AT = 0x34,
    CMD_LANGUAGE_NOTIFY = 0x35,
    CMD_OPEN_CHAN = 0x40,
    CMD_CLOSE_CHAN = 0x41,
    CMD_RECEIVE_DATA = 0x42,
    CMD_SEND_DATA = 0x43,
    CMD_GET_CHAN_STATUS = 0x44,
    CMD_RFU = 0x60,
    CMD_END_PROACTIVE_SESSION = 0x81,
    CMD_DETAIL = 0xFF
} sat_proactive_cmd_enum;

typedef enum {
    CMD_TYPE_PROACTIVE = 0x00,
    CMD_TYPE_NOTIFY = 0x01,
    CMD_YPE_SESSIONEND = 0x02
} sat_cmd_type_num;

typedef struct {
    RIL_SimRefreshResponse_v7* simRefreshRsp;
    char* efId;
} SimRefreshResponse;

class RmcCatUrcHandler : public RfxBaseHandler {
    RFX_DECLARE_HANDLER_CLASS(RmcCatUrcHandler);

    public:
        RmcCatUrcHandler(int slot_id, int channel_id);
        virtual ~RmcCatUrcHandler();

    protected:
        virtual void onHandleRequest(const sp<RfxMclMessage>& msg) {RFX_UNUSED(msg);}

        virtual void onHandleUrc(const sp<RfxMclMessage>& msg);

        virtual void onHandleTimer();
        virtual void onHandleEvent(const sp<RfxMclMessage>& msg);

    private:
        void handleBipEventNotify(const sp<RfxMclMessage>& msg);
        void handleReportCpinState(const sp<RfxMclMessage>& msg);
        void handleStkProactiveCommand(const sp<RfxMclMessage>& msg);
        void handleStkEventNotify(const sp<RfxMclMessage>& msg);
        void handleStkSessionEnd(const sp<RfxMclMessage>& msg);
        void handleStkCallControl(const sp<RfxMclMessage>& msg);
        bool onCheckIfRejectMessage(const sp<RfxMclMessage>& msg, RIL_RadioState radioState);
};

#endif
