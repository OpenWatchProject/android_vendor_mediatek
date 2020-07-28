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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#include "Dummy_RSCStream.h"

#include "DebugControl.h"
#define PIPE_CLASS_TAG "DummyRSCStream"
#define PIPE_TRACE TRACE_RSC_STREAM_BASE
#include <featurePipe/core/include/PipeLog.h>

#include <time.h>
#include <unistd.h>
#include <cstdlib>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

namespace NSCam {

RSCStream* RSCStream::createInstance(const char*)
{
    return new RSCStream();
}

RSCStream::RSCStream()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

RSCStream::~RSCStream()
{
    TRACE_FUNC_ENTER();
    this->uninit();
    TRACE_FUNC_EXIT();
}

MBOOL RSCStream::init()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mMutex);
    MBOOL ret = MFALSE;
    if( mProcessThread == NULL )
    {
        mProcessThread = new ProcessThread();
        if( mProcessThread != NULL )
        {
            mProcessThread->run("rsc_thread");
            ret = MTRUE;
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL RSCStream::uninit()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mMutex);
    if( mProcessThread != NULL )
    {
        mProcessThread->signalStop();
        mProcessThread->join();
        mProcessThread = NULL;
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL RSCStream::RSCenque(const RSCParam &param)
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mMutex);
    MBOOL ret = MFALSE;
    if( mProcessThread != NULL )
    {
        mProcessThread->enque(param);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL RSCStream::RSCdeque(RSCParam &, MINT64)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MFALSE;
}

MVOID RSCStream::destroyInstance(char const*)
{
    TRACE_FUNC_ENTER();
    delete this;
    TRACE_FUNC_EXIT();
}

const char* RSCStream::getPipeName() const
{
    return "Dummy_RSCStream";
}

RSCStream::ProcessThread::ProcessThread()
    : mStop(false)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

RSCStream::ProcessThread::~ProcessThread()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

void RSCStream::ProcessThread::enque(const RSCParam &param)
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mThreadMutex);
    mQueue.push(param);
    mThreadCondition.broadcast();
    TRACE_FUNC_EXIT();
}

void RSCStream::ProcessThread::signalStop()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mThreadMutex);
    mStop = true;
    mThreadCondition.broadcast();
    TRACE_FUNC_EXIT();
}

android::status_t RSCStream::ProcessThread::readyToRun()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return android::NO_ERROR;
}

bool RSCStream::ProcessThread::threadLoop()
{
    TRACE_FUNC_ENTER();
    RSCParam param;

    while( waitParam(param) )
    {
        processParam(param);
    }

    TRACE_FUNC_EXIT();
    return false;
}

bool RSCStream::ProcessThread::waitParam(RSCParam &param)
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mThreadMutex);
    bool ret = false, done = false;

    do
    {
        if( mQueue.size() )
        {
            param = mQueue.front();
            mQueue.pop();
            ret = true;
            done = true;
        }
        else if( mStop )
        {
            ret = false;
            done = true;
        }
        else
        {
            mThreadCondition.wait(mThreadMutex);
        }
    }while( !done );

    TRACE_FUNC_EXIT();
    return ret;
}

void RSCStream::ProcessThread::processParam(RSCParam param)
{
    TRACE_FUNC_ENTER();
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 10000000 + (std::rand()%10000000);
    TRACE_FUNC("pseudo delay=%d ms", t.tv_nsec%1000000);
    nanosleep(&t, NULL);

    if( param.mpfnCallback)
    {
        param.mpfnCallback(param);
    }
    else
    {
        MY_LOGE("Missing param.mCallback");
    }
    TRACE_FUNC_EXIT();
}

} // namespace NSCam
