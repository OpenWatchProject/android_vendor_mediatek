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

#ifndef _MTKCAM_HWNODE_P2_PROCESSOR_H_
#define _MTKCAM_HWNODE_P2_PROCESSOR_H_

#include "P2_Processor_t.h"

#include "P2_LogHeaderBegin.h"
#include "P2_DebugControl.h"
#define P2_CLASS_TAG    Processor
#define P2_TRACE        TRACE_PROCESSOR
#include "P2_LogHeader.h"

#define DEFAULT_THREAD_POLICY     SCHED_OTHER
#define DEFAULT_THREAD_PRIORITY   -2
using namespace android;
namespace P2
{

template <typename Init_T, typename Config_T, typename Enque_T>
Processor<Init_T, Config_T, Enque_T>::Processor(const std::string &name)
    : mName(name)
    , mThreadPolicy(DEFAULT_THREAD_POLICY)
    , mThreadPriority(DEFAULT_THREAD_PRIORITY)
    , mEnable(MTRUE)
    , mIdleWaitMS(0)
{
    TRACE_S_FUNC_ENTER(mName);
    TRACE_S_FUNC_EXIT(mName);
}

template <typename Init_T, typename Config_T, typename Enque_T>
Processor<Init_T, Config_T, Enque_T>::Processor(const std::string &name, MINT32 policy, MINT32 priority)
    : mName(name)
    , mThreadPolicy(policy)
    , mThreadPriority(priority)
    , mEnable(MTRUE)
    , mIdleWaitMS(0)
{
    TRACE_S_FUNC_ENTER(mName);
    TRACE_S_FUNC_EXIT(mName);
}

template <typename Init_T, typename Config_T, typename Enque_T>
Processor<Init_T, Config_T, Enque_T>::~Processor()
{
    TRACE_S_FUNC_ENTER(mName);
    if( mThread.get() != NULL )
    {
        MY_S_LOGE(mName, "Processor::uninit() not called: Child class MUST ensure uninit() in own destructor");
        mThread = NULL;
    }
    TRACE_S_FUNC_EXIT(mName);
}

template <typename Init_T, typename Config_T, typename Enque_T>
const char* Processor<Init_T, Config_T, Enque_T>::getName() const
{
    return mName.c_str();
}

template <typename Init_T, typename Config_T, typename Enque_T>
MBOOL Processor<Init_T, Config_T, Enque_T>::setEnable(MBOOL enable)
{
    TRACE_S_FUNC_ENTER(mName);
    android::Mutex::Autolock _lock(mThreadMutex);
    if( mThread == NULL )
    {
        mEnable = enable;
    }
    TRACE_S_FUNC_EXIT(mName);
    return mEnable;
}

template <typename Init_T, typename Config_T, typename Enque_T>
MVOID Processor<Init_T, Config_T, Enque_T>::setIdleWaitMS(MUINT32 ms)
{
    TRACE_S_FUNC_ENTER(mName);
    android::Mutex::Autolock _lock(mThreadMutex);
    if( mThread == NULL )
    {
        mIdleWaitMS = ms;
    }
    TRACE_S_FUNC_EXIT(mName);
}

template <typename Init_T, typename Config_T, typename Enque_T>
MBOOL Processor<Init_T, Config_T, Enque_T>::init(const Init_T &param)
{
    TRACE_S_FUNC_ENTER(mName);
    android::Mutex::Autolock _lock(mThreadMutex);
    MBOOL ret = !mEnable;
    if( mEnable && mThread == NULL )
    {
        if( this->onInit(param) )
        {
            mThread = new ProcessThread(this);
            if( mThread == NULL )
            {
                MY_S_LOGE(mName, "OOM: cannot create ProcessThread");
                this->onUninit();
            }
            else
            {
                mThread->run(mName.c_str());
                ret = MTRUE;
            }
        }
    }
    TRACE_S_FUNC_EXIT(mName);
    return ret;
}

template <typename Init_T, typename Config_T, typename Enque_T>
MVOID Processor<Init_T, Config_T, Enque_T>::uninit()
{
    TRACE_S_FUNC_ENTER(mName);
    android::Mutex::Autolock _lock(mThreadMutex);
    if( mThread != NULL )
    {
        mThread->stop();
        if( 0 != mThread->join() )
        {
            MY_S_LOGW(mName, "ProcessThread join failed");
        }
        mThread = NULL;
        this->onUninit();
    }
    TRACE_S_FUNC_EXIT(mName);
}

template <typename Init_T, typename Config_T, typename Enque_T>
MBOOL Processor<Init_T, Config_T, Enque_T>::config(const Config_T &param)
{
    TRACE_S_FUNC_ENTER(mName);
    android::Mutex::Autolock _lock(mThreadMutex);
    MBOOL ret = !mEnable;
    if( mThread != NULL )
    {
        ret = this->onConfig(param);
    }
    TRACE_S_FUNC_EXIT(mName);
    return ret;
}

template <typename Init_T, typename Config_T, typename Enque_T>
MBOOL Processor<Init_T, Config_T, Enque_T>::enque(const Enque_T &param)
{
    TRACE_S_FUNC_ENTER(mName);
    android::Mutex::Autolock _lock(mThreadMutex);
    MBOOL ret = MFALSE;
    if( mThread != NULL )
    {
        mThread->enque(param);
        ret = MTRUE;
    }
    TRACE_S_FUNC_EXIT(mName);
    return ret;
}

template <typename Init_T, typename Config_T, typename Enque_T>
MVOID Processor<Init_T, Config_T, Enque_T>::flush()
{
    TRACE_S_FUNC_ENTER(mName);
    android::Mutex::Autolock _lock(mThreadMutex);
    if( mThread != NULL )
    {
        this->onNotifyFlush();
        mThread->flush();
        this->onWaitFlush();
    }
    TRACE_S_FUNC_EXIT(mName);
}

template <typename Init_T, typename Config_T, typename Enque_T>
MVOID Processor<Init_T, Config_T, Enque_T>::notifyFlush()
{
    TRACE_S_FUNC_ENTER(mName);
    android::Mutex::Autolock _lock(mThreadMutex);
    if( mThread != NULL )
    {
        this->onNotifyFlush();
    }
    TRACE_S_FUNC_EXIT(mName);
}

template <typename Init_T, typename Config_T, typename Enque_T>
MVOID Processor<Init_T, Config_T, Enque_T>::waitFlush()
{
    TRACE_S_FUNC_ENTER(mName);
    android::Mutex::Autolock _lock(mThreadMutex);
    if( mThread != NULL )
    {
        mThread->flush();
        this->onWaitFlush();
    }
    TRACE_S_FUNC_EXIT(mName);
}

template <typename Init_T, typename Config_T, typename Enque_T>
Processor<Init_T, Config_T, Enque_T>::ProcessThread::ProcessThread(Processor *parent)
    : mParent(parent)
    , mName(parent->mName)
    , mIdleWaitTime(milliseconds_to_nanoseconds(parent->mIdleWaitMS))
    , mStop(MFALSE)
    , mIdle(MTRUE)
{
    TRACE_S_FUNC_ENTER(mName);
    TRACE_S_FUNC_EXIT(mName);
}

template <typename Init_T, typename Config_T, typename Enque_T>
Processor<Init_T, Config_T, Enque_T>::ProcessThread::~ProcessThread()
{
    TRACE_S_FUNC_ENTER(mName);
    TRACE_S_FUNC_EXIT(mName);
}

template <typename Init_T, typename Config_T, typename Enque_T>
android::status_t Processor<Init_T, Config_T, Enque_T>::ProcessThread::readyToRun()
{
    TRACE_S_FUNC_ENTER(mName);
    struct sched_param sched;
    ::sched_getparam(0, &sched);
    if( mParent->mThreadPolicy == SCHED_OTHER )
    {
        sched.sched_priority = 0;
        ::sched_setscheduler(0, mParent->mThreadPolicy, &sched);
        ::setpriority(PRIO_PROCESS, 0, mParent->mThreadPriority);
    }
    else
    {
        sched.sched_priority = mParent->mThreadPriority;
        ::sched_setscheduler(0, mParent->mThreadPolicy, &sched);
    }
    TRACE_S_FUNC_EXIT(mName);
    return 0;
}

template <typename Init_T, typename Config_T, typename Enque_T>
bool Processor<Init_T, Config_T, Enque_T>::ProcessThread::threadLoop()
{
    TRACE_S_FUNC_ENTER(mName);
    Enque_T param;
    WaitResult waitResult;
    mParent->onThreadStart();
    do
    {
        waitResult = waitEnqueParam(param);
        if( waitResult == WAIT_OK )
        {
            mParent->onEnque(param);
        }
        else if( waitResult == WAIT_IDLE )
        {
            mParent->onIdle();
        }
        param = Enque_T();
    }while( waitResult != WAIT_EXIT );
    mParent->onThreadStop();
    TRACE_S_FUNC_EXIT(mName);
    return false;
}

template <typename Init_T, typename Config_T, typename Enque_T>
MVOID Processor<Init_T, Config_T, Enque_T>::ProcessThread::enque(const Enque_T &param)
{
    TRACE_S_FUNC_ENTER(mName);
    android::Mutex::Autolock _lock(mMutex);
    mQueue.push(param);
    mCondition.broadcast();
    TRACE_S_FUNC_EXIT(mName);
}

template <typename Init_T, typename Config_T, typename Enque_T>
MVOID Processor<Init_T, Config_T, Enque_T>::ProcessThread::flush()
{
    TRACE_S_FUNC_ENTER(mName);
    android::Mutex::Autolock _lock(mMutex);
    while( !mIdle )
    {
        mCondition.wait(mMutex);
    }
    TRACE_S_FUNC_EXIT(mName);
}

template <typename Init_T, typename Config_T, typename Enque_T>
MVOID Processor<Init_T, Config_T, Enque_T>::ProcessThread::stop()
{
    TRACE_S_FUNC_ENTER(mName);
    android::Mutex::Autolock _lock(mMutex);
    mStop = MTRUE;
    mCondition.broadcast();
    TRACE_S_FUNC_EXIT(mName);
}

template <typename Init_T, typename Config_T, typename Enque_T>
typename Processor<Init_T, Config_T, Enque_T>::ProcessThread::WaitResult Processor<Init_T, Config_T, Enque_T>::ProcessThread::waitEnqueParam(Enque_T &param)
{
    TRACE_S_FUNC_ENTER(mName);
    android::Mutex::Autolock _lock(mMutex);
    WaitResult result = WAIT_ERROR;
    MBOOL needWaitIdle = MFALSE;
    if( mQueue.size() == 0 )
    {
        if( !mIdle )
        {
            mIdle = MTRUE;
            needWaitIdle = MTRUE;
            mCondition.broadcast();
        }

        if( mStop )
        {
            result = WAIT_EXIT;
        }
        else if( needWaitIdle && mIdleWaitTime )
        {
            android::status_t status;
            status = mCondition.waitRelative(mMutex, mIdleWaitTime);
            if( status == TIMED_OUT )
            {
                result = WAIT_IDLE;
            }
        }
        else
        {
            mCondition.wait(mMutex);
        }
    }
    if( mQueue.size() )
    {
        param = mQueue.front();
        mQueue.pop();
        mIdle = MFALSE;
        result = WAIT_OK;
    }
    TRACE_S_FUNC_EXIT(mName);
    return result;
}

} // namespace P2

#include "P2_LogHeaderEnd.h"
#endif // _MTKCAM_HWNODE_P2_PROCESSOR_H_
