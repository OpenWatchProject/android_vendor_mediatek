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

#ifndef __RFX_BASE_HANDLER__H__
#define __RFX_BASE_HANDLER__H__

#include "utils/String8.h"
#include <utils/RefBase.h>
#include <telephony/mtk_ril.h>
#include "RfxHandlerManager.h"
#include "RfxMclMessage.h"
//#include "RfxReader.h"
#include "RfxSender.h"
#include "RfxMclStatusManager.h"
// #include "RfxChannel.h"
#include "RfxChannelManager.h"
#include "RfxDataCloneManager.h"
#include "RfxMessageId.h"
#include "RfxDefs.h"
#include <vendor/mediatek/proprietary/hardware/ril/platformlib/common/libmtkrilutils/proto/sap-api.pb.h>

using ::android::String8;
using ::android::sp;
using ::android::wp;
using ::android::RefBase;

#define LOG_BUF_SIZE 1024

// class RfxChannel;
class RfxBaseHandler;

#define RFX_DECLARE_HANDLER_CLASS(_class_name)                                  \
    public:                                                                     \
        static RfxBaseHandler *createInstance(int slot_id, int channel_id);     \
    public:                                                                     \
        class InitHelper {                                                      \
        public:                                                                 \
            InitHelper(int c_id, int slot_category);                            \
        }

#define RFX_IMPLEMENT_HANDLER_CLASS(_class_name, _channel_id)   \
    RfxBaseHandler *_class_name::createInstance(int slot_id, int channel_id)    \
    {                                                                           \
        _class_name *handler;                                                   \
        handler = new _class_name(slot_id, channel_id);                         \
        return handler;                                                         \
    }                                                                           \
                                                                                \
    _class_name::InitHelper::InitHelper(int c_id, int slot_category) {          \
         RfxHandlerManager::registerHandler(_class_name::createInstance, c_id, slot_category, false, false); \
    }                                                                                          \
                                                                                               \
    _class_name::InitHelper _class_name##s_init_val(_channel_id, SLOT);

#define RFX_IMPLEMENT_OP_PARENT_HANDLER_CLASS(_class_name, _channel_id)         \
    RfxBaseHandler *_class_name::createInstance(int slot_id, int channel_id)    \
    {                                                                           \
        _class_name *handler;                                                   \
        handler = new _class_name(slot_id, channel_id);                         \
        return handler;                                                         \
    }                                                                           \
                                                                                \
    _class_name::InitHelper::InitHelper(int c_id, int slot_category) {          \
         RfxHandlerManager::registerHandler(_class_name::createInstance, c_id, slot_category, true, false); \
    }                                                                                          \
                                                                                               \
    _class_name::InitHelper _class_name##s_init_val(_channel_id, SLOT);

#define RFX_IMPLEMENT_NON_SLOT_HANDLER_CLASS(_class_name, _channel_id)          \
    RfxBaseHandler *_class_name::createInstance(int slot_id, int channel_id)    \
    {                                                                           \
        _class_name *handler;                                                   \
        handler = new _class_name(slot_id, channel_id);                         \
        return handler;                                                         \
    }                                                                           \
                                                                                \
    _class_name::InitHelper::InitHelper(int c_id, int slot_category) {          \
         RfxHandlerManager::registerHandler(_class_name::createInstance, c_id, slot_category, false, false); \
    }                                                                                          \
                                                                                               \
    _class_name::InitHelper _class_name##s_init_val(_channel_id, NON_SLOT);

#define RFX_IMPLEMENT_OP_PARENT_NON_SLOT_HANDLER_CLASS(_class_name, _channel_id)\
    RfxBaseHandler *_class_name::createInstance(int slot_id, int channel_id)    \
    {                                                                           \
        _class_name *handler;                                                   \
        handler = new _class_name(slot_id, channel_id);                         \
        return handler;                                                         \
    }                                                                           \
                                                                                \
    _class_name::InitHelper::InitHelper(int c_id, int slot_category) {          \
         RfxHandlerManager::registerHandler(_class_name::createInstance, c_id, slot_category, true, false); \
    }                                                                                          \
                                                                                               \
    _class_name::InitHelper _class_name##s_init_val(_channel_id, NON_SLOT);

#define RFX_IMPLEMENT_OP_HANDLER_CLASS(_class_name, _channel_id)   \
    RfxBaseHandler *_class_name::createInstance(int slot_id, int channel_id)    \
    {                                                                           \
        _class_name *handler;                                                   \
        handler = new _class_name(slot_id, channel_id);                         \
        return handler;                                                         \
    }                                                                           \
                                                                                \
    _class_name::InitHelper::InitHelper(int c_id, int slot_category) {          \
         RfxHandlerManager::registerHandler(_class_name::createInstance, c_id, slot_category, false, true); \
    }                                                                                          \
                                                                                               \
    _class_name::InitHelper _class_name##s_init_val(_channel_id, SLOT);


#define RFX_REGISTER_HANDLER_CLASS(class_name, channel_id)                      \
    class_name::InitHelper class_name##channel_id##s_init_val(channel_id, SLOT);

// class RfxBaseHandler : public virtual RefBase {
class RfxBaseHandler {

    public:
        RfxBaseHandler(int slot_id, int channel_id) : m_slot_id(slot_id),
                m_channel_id(channel_id) {}
        virtual ~RfxBaseHandler() {}

    public:

        void processMessage(const sp<RfxMclMessage>& msg);

        void processTimer() {
            onHandleTimer();
        }

        void registerToHandleRequest(const int *request_id_list, size_t length);

        void registerToHandleURC(const char **urc_prefix_list, size_t length);

        void registerToHandleEvent(const int *event_prefix_list, size_t length);

        void registerToHandleEvent(int clientId, const int *event_prefix_list, size_t length);

        void responseToTelCore(const sp<RfxMclMessage>& msg);

        void sendBtSapStatusInd(RIL_SIM_SAP_STATUS_IND_Status message);

        void sendEvent(int id, const RfxBaseData &data, int channelId, int slotId,
                int clientId = -1, int token = -1, nsecs_t delayTime = 0,
                MTK_RIL_REQUEST_PRIORITY priority =
                MTK_RIL_REQUEST_PRIORITY::MTK_RIL_REQUEST_PRIORITY_MEDIUM);

        // cancelEvent

        void sendEvent(sp<RfxMclMessage> msg);

        void logD(const char *tag, const char *fmt, ...) const;
        void logI(const char *tag, const char *fmt, ...) const;
        void logV(const char *tag, const char *fmt, ...) const;
        void logE(const char *tag, const char *fmt, ...) const;
        void logW(const char *tag, const char *fmt, ...) const;

        RfxMclStatusManager* getMclStatusManager();
        RfxMclStatusManager* getMclStatusManager(int slotId);
        RfxMclStatusManager* getNonSlotMclStatusManager();

        // at send command related functions
        sp<RfxAtResponse> atSendCommandSingleline (const char *command,
                const char *responsePrefix, RIL_Token ackToken = NULL);
        sp<RfxAtResponse> atSendCommandSingleline (const String8 &command,
                const char *responsePrefix, RIL_Token ackToken = NULL);

        sp<RfxAtResponse> atSendCommandNumeric (const char *command,
                RIL_Token ackToken = NULL);
        sp<RfxAtResponse> atSendCommandNumeric (const String8 &command,
                RIL_Token ackToken = NULL);

        sp<RfxAtResponse> atSendCommandMultiline (const char *command,
                const char *responsePrefix, RIL_Token ackToken = NULL);
        sp<RfxAtResponse> atSendCommandMultiline (const String8 &command,
                const char *responsePrefix, RIL_Token ackToken = NULL);

        sp<RfxAtResponse> atSendCommand (const char *command,
                RIL_Token ackToken = NULL);
        sp<RfxAtResponse> atSendCommand (const String8 &command,
                RIL_Token ackToken = NULL);

        sp<RfxAtResponse> atSendCommandRaw (const char *command,
                RIL_Token ackToken = NULL);
        sp<RfxAtResponse> atSendCommandRaw (const String8 &command,
                RIL_Token ackToken = NULL);

        bool sendUserData(int clientId, unsigned char* data, size_t length);
        bool sendUserData(int clientId, int config, unsigned char* data,
                size_t length);

        int strStartsWith(const char *line, const char *prefix);

        const char* idToString(int id);

        int getFeatureVersion(char *feature, int defaultVaule);

        int getFeatureVersion(char *feature);
    protected:

        virtual void onHandleRequest(const sp<RfxMclMessage>& msg);

        virtual void onHandleUrc(const sp<RfxMclMessage>& msg);

        virtual void onHandleTimer() {}

        virtual void onHandleEvent(const sp<RfxMclMessage>& msg);

        virtual bool onCheckIfRejectMessage(const sp<RfxMclMessage>& msg,
                RIL_RadioState radioState);
        /*
         * Convert the URC to the strings data without any logic
         * msg: URC message
         * rfxMsg: ID of rfxMsg
         * maxLen: Expected max length of URC params.
                   Parse all params if it is 0.
         * appendPhoneId: add phone Id to the rfx message for Fwk.
         */
        void notifyStringsDataToTcl(
            const sp<RfxMclMessage>& msg, int rfxMsg, int maxLen, bool appendPhoneId = false);
        /*
         * Convert the URC to the ints data without any logic
         * msg: URC message
         * rfxMsg: ID of rfxMsg
         * maxLen: Expected max length of URC params.
                   Parse all params if it is 0.
         * appendPhoneId: add phone Id to the rfx message for Fwk.
         */
        void notifyIntsDataToTcl(
            const sp<RfxMclMessage>& msg, int rfxMsg, int maxLen, bool appendPhoneId = false);
        /*
         * Handle the AT command which will response the void to the TCL
         * Helpful to reduce the duplicate code
         */
        RIL_Errno handleCmdWithVoidResponse(const sp<RfxMclMessage>& msg, String8 cmd);

    private:
        RfxSender *getSender();

    protected:

        int m_slot_id;
        int m_channel_id;
};

#endif
