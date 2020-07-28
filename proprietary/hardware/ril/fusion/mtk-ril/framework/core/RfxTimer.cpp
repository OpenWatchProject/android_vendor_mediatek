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
 * File name:  rfx_timer.cpp
 * Author: Jun Liu (MTK80064)
 * Description:
 * Implementation of timer for Ril proxy framework
 */

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "RfxTimer.h"
#include "RfxMainThread.h"
#include "RfxAsyncSignal.h"
#include "RfxRootController.h"

#define RFX_LOG_TAG "RfxTimer"

/*****************************************************************************
 * Class TimerHandler
 *****************************************************************************/
class TimerHandler : public RfxMainHandler {
public:

    TimerHandler(const RfxCallback0 &_callback) : callback(_callback) {}
    virtual ~TimerHandler() {}

protected:

    virtual void onHandleMessage(const Message& message) {
        RFX_UNUSED(message);
        RFX_LOG_D(RFX_LOG_TAG, "onHandleMessage() begin, this = %p", this);

        // callback to slot
        callback.invoke();

        RFX_LOG_D(RFX_LOG_TAG, "onHandleMessage() end, this = %p", this);
    }

private:

    RfxCallback0 callback;
};

/*****************************************************************************
 * Class RfxTimer
 *****************************************************************************/

TimerHandle RfxTimer::start(const RfxCallback0 &callback, nsecs_t time) {

    Looper *looper = RfxMainThread::getLooper().get();

    if (looper != NULL) {
        Message dummy_msg;
        sp<MessageHandler> handler = new TimerHandler(callback);
        RFX_LOG_D(RFX_LOG_TAG, "start(), timer = %p", handler.get());
        looper->sendMessageDelayed(time, handler, dummy_msg);
        return handler;
    } else {
        return TimerHandle(NULL);
    }
}


void RfxTimer::stop(const TimerHandle &timer_handle) {
    Looper *looper = RfxMainThread::getLooper().get();

    if (looper != NULL) {
        RFX_LOG_D(RFX_LOG_TAG, "stop(), timer = %p", timer_handle.get());
        looper->removeMessages(timer_handle);
    }
}

