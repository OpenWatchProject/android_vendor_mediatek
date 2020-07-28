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

#ifndef WPFA_FILTER_RULE_REQ_HANDLER_H
#define WPFA_FILTER_RULE_REQ_HANDLER_H

#include <sys/ioctl.h>
#include <semaphore.h>
#include <termios.h>
// #include <utils/Looper.h>
// #include <utils/threads.h>
#include <Looper.h>
#include <threads.h>
// #include "utils/Mutex.h"
#include "Mutex.h"
#include "hardware/ccci_intf.h"

#include "WpfaDriver.h"
#include "WpfaDriverAdapter.h"
#include "WpfaDriverMessage.h"
#include "WpfaDriverBaseData.h"

#include <mtk_log.h>

using ::android::Looper;
using ::android::Thread;
using ::android::MessageHandler;
using ::android::Message;
using ::android::sp;

class WpfaFilterRuleReqHandler : public Thread {

private:
    class RuleReqMsgHandler : public MessageHandler {
        public:
            RuleReqMsgHandler(WpfaFilterRuleReqHandler* _dispatcher,
                    const sp<WpfaDriverMessage>& _msg) :
                msg(_msg) , sender(_dispatcher){}

            virtual ~RuleReqMsgHandler() {}

            virtual void handleMessage(const Message& message);

        private:
            sp<WpfaDriverMessage> msg;
            WpfaFilterRuleReqHandler* sender;
    };


private:
    WpfaFilterRuleReqHandler();
    virtual ~WpfaFilterRuleReqHandler() {}

public:
    static void init();
    static void enqueueFilterRuleReqMessage(const sp<WpfaDriverMessage>& message);
    static void enqueueFilterRuleReqMessageFront(const sp<WpfaDriverMessage>& message);

    static sp<Looper> waitLooper();
    virtual bool threadLoop();

private:
    void processMessage(const sp<WpfaDriverMessage>& msg);
    int sendRegAcceptResponse(uint16_t tId, uint32_t fId, int32_t errCause);
    int sendDeregAcceptResponse(uint16_t tId, uint32_t fId, int32_t errCause);
    int checkDriverState();
    int checkDriverAdapterState();
    int DriverMsgIdToEventId(uint16_t msgId);

    static WpfaFilterRuleReqHandler *s_self;
    sp<Looper> mLooper;
    Message mDummyMsg;

    WpfaDriver *mWpfaDriver;
    WpfaDriverAdapter *mDriverAdapter;
};

#endif
