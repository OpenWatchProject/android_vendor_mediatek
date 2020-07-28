/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
/*
 * File name:  RfxMainThread.h
 * Author: Jun Liu (MTK80064)
 * Description:
 * Header file for RfxMainThread
 */

#ifndef __RFX_MAIN_THREAD_H__
#define __RFX_MAIN_THREAD_H__

/*****************************************************************************
 * Include
 *****************************************************************************/

#include <utils/Looper.h>
#include <utils/threads.h>
#include "utils/Timers.h"
#include "RfxMessage.h"

using ::android::Looper;
using ::android::Thread;
using ::android::MessageHandler;
using ::android::Message;
using ::android::sp;

/*****************************************************************************
 * Class RfxMainHandler
 *****************************************************************************/

class RfxMainHandler : public MessageHandler {
public:

    virtual ~RfxMainHandler() {}

public:

    virtual void handleMessage(const Message& message);

protected:

    // Sub-class should override onHandleMessage(), not handleMessage()
    virtual void onHandleMessage(const Message& message) = 0;
};

/*****************************************************************************
 * Class RfxMainThread
 *****************************************************************************/

class RfxMainThread : public Thread {

private:

    RfxMainThread();

    virtual ~RfxMainThread();

public:

    static void init();

    static sp<Looper> waitLooper(); // Must invoke after calling rfx_init()

    static void enqueueMessage(const sp<RfxMessage>& message);

    static void enqueueMessageFront(const sp<RfxMessage>& message);

    static sp<Looper> getLooper();

    static void clearMessages();

private:

    virtual bool threadLoop();
    void initControllers();

private:

    static RfxMainThread *s_self;

    sp<Looper> m_looper;

    Message m_dummy_msg;
};

#endif // __RFX_MAIN_THREAD_H__

