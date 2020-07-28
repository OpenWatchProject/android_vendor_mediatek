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

#ifndef __RFX_SENDER_H__
#define __RFX_SENDER_H__

#include <utils/Looper.h>
#include <utils/threads.h>
#include "utils/RefBase.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "RfxMclMessage.h"
//#include "RfxReader.h"
#include "RfxAtResponse.h"
#include "RfxLog.h"
#include "RfxRilUtils.h"
#include "rfx_properties.h"
#include <sys/time.h>
#include "RfxChannelContext.h"
#include <semaphore.h>
#include "utils/Mutex.h"

using ::android::Looper;
using ::android::Thread;
using ::android::MessageHandler;
using ::android::Message;
using ::android::Mutex;
using ::android::RefBase;
using ::android::sp;

class RfxSender : public Thread {
    public:
        RfxSender(int fd, int channel_id, RfxChannelContext *context);

    private:
        class MclMessageHandler : public MessageHandler {
            public:
                MclMessageHandler(RfxSender* _sender, const sp<RfxMclMessage>& _msg) :
                        msg(_msg), sender(_sender) {}
                virtual ~MclMessageHandler() {}

            public:
                virtual void handleMessage(const Message& message);

            private:
                sp<RfxMclMessage> msg;
                RfxSender* sender;
        };

    public:
        void enqueueMessage(const sp<RfxMclMessage>& msg);

        void enqueueMessageFront(const sp<RfxMclMessage>& msg);

        sp<Looper> waitLooper(); // Must invoke after new RfxSender

    private:
        virtual bool threadLoop();

        void processMessage(const sp<RfxMclMessage>& msg);

    public:
        sp<RfxAtResponse> atSendCommandSinglelineAck (const char *command,
                const char *responsePrefix, RIL_Token ackToken);
        sp<RfxAtResponse> atSendCommandSingleline (const char *command, const char *responsePrefix);
        sp<RfxAtResponse> atSendCommandNumericAck (const char *command, RIL_Token ackToken);
        sp<RfxAtResponse> atSendCommandNumeric (const char *command);
        sp<RfxAtResponse> atSendCommandMultilineAck (const char *command,
                const char *responsePrefix, RIL_Token ackToken);
        sp<RfxAtResponse> atSendCommandMultiline (const char *command, const char *responsePrefix);
        sp<RfxAtResponse> atSendCommandAck (const char *command, RIL_Token ackToken);
        sp<RfxAtResponse> atSendCommand (const char *command);
        sp<RfxAtResponse> atSendCommandRawAck (const char *command, RIL_Token ackToken);
        sp<RfxAtResponse> atSendCommandRaw (const char *command);

        int sendUserData(int clientId, unsigned char* data, size_t length);
        int sendUserData(int clientId, int config, unsigned char* data, size_t length);

        int getFd() const {
            return m_fd;
        }
        void setFd(int fd) {
            m_fd = fd;
        }

    private:
        sp<RfxAtResponse> atSendCommandFullAck(const char *command, AtCommandType type,
                const char *responsePrefix, long long timeoutMsec, RIL_Token ackToken);
        sp<RfxAtResponse> atSendCommandFull(const char *command, AtCommandType type,
                const char *responsePrefix, long long timeoutMsec);
        sp<RfxAtResponse> atSendCommandFullNolockAck(const char *command, AtCommandType type,
                const char *responsePrefix, long long timeoutMsec, RIL_Token ackToken);
        sp<RfxAtResponse> atSendCommandFullNolock(const char *command, AtCommandType type,
                const char *responsePrefix, long long timeoutMsec);

        int getATCommandTimeout(const char *command);

        int writeline(const char *s);
        int writelineUserData(unsigned char* frame, size_t length);
        void printLog(int level, String8 log);

    private:
        sp<Looper> m_looper;
        int m_fd;
        int m_channel_id;
        Message m_dummy_msg;
        friend MclMessageHandler;
        int m_atTimeoutMsec;
        RfxChannelContext *m_context;
        pthread_t m_threadId;
        const char* mName;

        sem_t mWaitLooperSem;
        bool mNeedWaitLooper;
        Mutex mWaitLooperMutex;
        int mIsFuzzyTesting;
        int mFuzzyTestingTimeout;
};

#endif
