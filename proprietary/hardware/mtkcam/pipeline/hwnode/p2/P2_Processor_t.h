/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTKCAM_HWNODE_P2_PROCESSOR_T_H_
#define _MTKCAM_HWNODE_P2_PROCESSOR_T_H_

#include <queue>
#include <string>
#include <utils/RefBase.h>
#include <utils/Thread.h>
#include "P2_Header.h"

namespace P2
{

template <typename Init_T, typename Config_T, typename Enque_T>
class Processor
{
public:
    Processor(const std::string &name);
    Processor(const std::string &name, MINT32 policy, MINT32 priority);
    virtual ~Processor();
    const char* getName() const;
    MBOOL setEnable(MBOOL enable);
    MVOID setIdleWaitMS(MUINT32 msec);
    MBOOL init(const Init_T &param);
    MVOID uninit();
    MBOOL config(const Config_T &param);
    MBOOL enque(const Enque_T &param);
    MVOID flush();
    MVOID notifyFlush();
    MVOID waitFlush();

protected:
    virtual MBOOL onInit(const Init_T &param) = 0;
    virtual MVOID onUninit() = 0;
    virtual MVOID onThreadStart() = 0;
    virtual MVOID onThreadStop() = 0;
    virtual MBOOL onConfig(const Config_T &param) = 0;
    virtual MBOOL onEnque(const Enque_T &param) = 0;
    virtual MVOID onNotifyFlush() = 0;
    virtual MVOID onWaitFlush() = 0;
    virtual MVOID onIdle() {}

private:
    class ProcessThread : virtual public android::Thread
    {
    public:
        ProcessThread(Processor *parent);
        virtual ~ProcessThread();
        android::status_t readyToRun();
        bool threadLoop();

        MVOID enque(const Enque_T &param);
        MVOID flush();
        MVOID stop();

    private:
        enum WaitResult { WAIT_OK, WAIT_ERROR, WAIT_IDLE, WAIT_EXIT };
        WaitResult waitEnqueParam(Enque_T &param);

    private:
        Processor *mParent;
        const std::string mName;
        nsecs_t mIdleWaitTime;
        android::Mutex mMutex;
        android::Condition mCondition;
        MBOOL mStop;
        MBOOL mIdle;
        std::queue<Enque_T> mQueue;
    };

private:
    android::Mutex mThreadMutex;
    android::sp<ProcessThread> mThread;
    const std::string mName;
    MINT32 mThreadPolicy;
    MINT32 mThreadPriority;
    MBOOL mEnable;
    MUINT32 mIdleWaitMS;
};

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_PROCESSOR_T_H_
