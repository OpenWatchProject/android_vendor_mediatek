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

#ifndef WPFA_CONTROL_MSG_HANDLER_H
#define WPFA_CONTROL_MSG_HANDLER_H

#include <sys/ioctl.h>
#include <semaphore.h>
#include <termios.h>
// #include <utils/Looper.h>
// #include <utils/threads.h>
// #include "utils/Mutex.h"
#include <Looper.h>
#include <threads.h>
#include "Mutex.h"

#include "WpfaDriverAdapter.h"
#include "WpfaDriverMessage.h"
#include "WpfaDriverBaseData.h"

#include "WpfaCcciReader.h"
#include "WpfaCcciSender.h"

#include "WpfaDriverUtilis.h"

#include <mtk_log.h>

using ::android::Looper;
using ::android::Thread;
using ::android::MessageHandler;
using ::android::Message;
using ::android::sp;

class WpfaControlMsgHandler : public Thread {

private:
    class CcciMsgHandler : public MessageHandler {
        public:
            CcciMsgHandler(WpfaControlMsgHandler* _sender, const sp<WpfaDriverMessage>& _msg) :
                msg(_msg) , sender(_sender){}

            virtual ~CcciMsgHandler() {}

            virtual void handleMessage(const Message& message);

        private:
            sp<WpfaDriverMessage> msg;
            WpfaControlMsgHandler* sender;
    };


private:
    WpfaControlMsgHandler();
    virtual ~WpfaControlMsgHandler() {}

public:
    static void init(int ccciHandler);
    static void enqueueDriverMessage(const sp<WpfaDriverMessage>& message);
    static void enqueueDriverlMessageFront(const sp<WpfaDriverMessage>& message);

    static sp<Looper> waitLooper();
    virtual bool threadLoop();

    int openCcciDriver();
    void sendMessageToCcci(const sp<WpfaDriverMessage>& msg);
    void sendMessageToRuleHandler(const sp<WpfaDriverMessage>& msg);
    void sendMessageToShmReadMsgHandler(const sp<WpfaDriverMessage>& msg);
    void sendMessageToShmWriteMsgHandler(const sp<WpfaDriverMessage>& msg);

private:
    int checkDriverAdapterState();

    static int mCcciFd;
    static WpfaControlMsgHandler *s_self;
    sp<Looper> mLooper;
    Message mDummyMsg;

    WpfaDriverAdapter *mDriverAdapter;
    WpfaCcciReader *mReader;
    WpfaCcciSender *mSender;

    WpfaDriverUtilis *mWpfaDriverUtilis;
};

#endif
