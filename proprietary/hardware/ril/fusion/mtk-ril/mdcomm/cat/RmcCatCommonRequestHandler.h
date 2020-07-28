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

#ifndef __RMC_CAT_COMMON_REQUEST_HANDLER_H__
#define __RMC_CAT_COMMON_REQUEST_HANDLER_H__

#include "RfxBaseHandler.h"
#include "RfxIntsData.h"
#include "RfxStringData.h"
#include "RfxStringsData.h"
#include "RfxVoidData.h"
#include "RfxSimIoRspData.h"
#include "RfxMessageId.h"
#include "telephony/ril.h"
#include "RfxRawData.h"

// BTSAP @{
#ifdef __cplusplus
extern "C"
{
#endif
#include <vendor/mediatek/proprietary/hardware/ril/platformlib/common/libmtkrilutils/proto/sap-api.pb.h>
#include "pb_decode.h"
#include "pb_encode.h"
#ifdef __cplusplus
}
#endif

#define BT_SAP_CARDREADER_RESPONSE_DEFAULT        0x10
#define BT_SAP_CARDREADER_RESPONSE_READER_POWER   0x80
#define BT_SAP_CARDREADER_RESPONSE_SIM_INSERT     0x40
// BTSAP @}

#define RFX_LOG_TAG "RmcCatComReqHandler"

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

// BIP @{
typedef enum {
    /*
     * Results '0X' and '1X' indicate that the command has been performed.
     */
    RESULT_OK = 0x00,
    RESULT_PRFRMD_WITH_PARTIAL_COMPREHENSION = 0x01,
    RESULT_PRFRMD_WITH_MISSING_INFO = 0x02,
    RESULT_PRFRMD_WITH_ADDITIONAL_EFS_READ = 0x03,
    RESULT_PRFRMD_ICON_NOT_DISPLAYED = 0x04,
    RESULT_PRFRMD_MODIFIED_BY_NAA = 0x05,
    RESULT_PRFRMD_LIMITED_SERVICE = 0x06,
    RESULT_PRFRMD_WITH_MODIFICATION = 0x07,
    RESULT_UICC_SESSION_TERM_BY_USER = 0x10,
    RESULT_BACKWARD_MOVE_BY_USER = 0x11,
    RESULT_NO_RESPONSE_FROM_USER = 0x12,
    RESULT_HELP_INFO_REQUIRED = 0x13,
    /*
     * Results '2X' indicate to the UICC that it may be worth re-trying the
     * command at a later opportunity.
     */
    RESULT_TERMINAL_CRNTLY_UNABLE_TO_PROCESS = 0x20,
    RESULT_NETWORK_CRNTLY_UNABLE_TO_PROCESS = 0x21,
    RESULT_USER_NOT_ACCEPT = 0x22,
    /*
     * Results '3X' indicate that it is not worth the UICC re-trying with an
     * identical command, as it will only get the same response. However, the
     * decision to retry lies with the application.
     */
    RESULT_BEYOND_TERMINAL_CAPABILITY = 0x30,
    RESULT_CMD_TYPE_NOT_UNDERSTOOD = 0x31,
    RESULT_CMD_DATA_NOT_UNDERSTOOD = 0x32,
    RESULT_CMD_NUM_NOT_KNOWN = 0x33,
    RESULT_BIP_ERROR = 0x3a,
} sat_result_code_enum;
// BIP @}
class RmcCatCommonRequestHandler : public RfxBaseHandler {
    RFX_DECLARE_HANDLER_CLASS(RmcCatCommonRequestHandler);

    public:
        RmcCatCommonRequestHandler(int slot_id, int channel_id);
        virtual ~RmcCatCommonRequestHandler();

    protected:
        virtual void onHandleRequest(const sp<RfxMclMessage>& msg);

        virtual void onHandleTimer();
        virtual void onHandleEvent(const sp<RfxMclMessage>& msg);

    private:
        bool isCpinReady();
        void setStkFlag(bool* source, bool flag);
        void requestReportStkServiceIsRunning(const sp<RfxMclMessage>& msg);
        void requestStkSendEnvelopeCommand (const sp<RfxMclMessage>& msg);
        void requestStkSendTerminalResponse (const sp<RfxMclMessage>& msg);
        void requestStkHandleCallSetupRequestedFromSim (const sp<RfxMclMessage>& msg);
        void requestStkSendEnvelopeCommandWithStatus (const sp<RfxMclMessage>& msg);
        void handleStkEventNotify(const sp<RfxMclMessage>& msg);
        int getInCallNumber();
        void requestStkEventNotify(const sp<RfxMclMessage>& msg);
        void requestStkQeryCpinState(const sp<RfxMclMessage>& msg);
        void requestStkSendResponseByCmdType (const sp<RfxMclMessage>& msg);
        // BIP @{
        void requestBipSendConfirmInfo (const sp<RfxMclMessage>& msg);
        // BIP @}
        // BTSAP @{
        void requestBtSapTransferCardReaderStatus(const sp<RfxMclMessage>& msg);
        void sendStkBtSapResponseComplete(const sp<RfxMclMessage>& msg,
                RIL_Errno ret, int msgId, void *data);
        // BTSAP @}
    private:
        bool isProaCmdQueued;
        bool isEventNotifyQueued;
        char* pProactiveCmd;
        char* pEventNotifyCmd;
};

#endif
