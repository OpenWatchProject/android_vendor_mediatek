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

#include "GPUWarpStream.h"

#include "DebugControl.h"
#define PIPE_CLASS_TAG "GPUWarpStream"
#define PIPE_TRACE TRACE_GPU_WARP_STREAM_BASE
#include <featurePipe/core/include/PipeLog.h>
#include <utils/String8.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

GPUWarpStream* GPUWarpStream::createInstance()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return new GPUWarpStream();
}

GPUWarpStream::GPUWarpStream()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

GPUWarpStream::~GPUWarpStream()
{
    TRACE_FUNC_ENTER();
    this->uninit();
    TRACE_FUNC_EXIT();
}

MBOOL GPUWarpStream::init(const MUINT32 &sensorIdx, const MSize &maxImageSize, const MSize &maxWarpSize)
{
    TRACE_FUNC_ENTER();
    (void)(sensorIdx);
    android::Mutex::Autolock lock(mMutex);
    MBOOL ret = MFALSE;
    if( mProcessThread == NULL )
    {
        if( (mProcessThread = new ProcessThread()) != NULL )
        {
            if( (ret = mGPUWarp.init(maxImageSize, maxWarpSize)) )
            {
                mProcessThread->run("Cam@fpipe.mdp");
            }
            else
            {
                MY_LOGE("cannot init GPUWarp");
                mProcessThread = NULL;
            }
        }
        else
        {
            MY_LOGE("OOM: cannot create ProcessThread");
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MVOID GPUWarpStream::uninit()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mMutex);
    if( mProcessThread != NULL )
    {
        mProcessThread->signalStop();
        mGPUWarp.uninit();
        mProcessThread->join();
        mProcessThread = NULL;
    }
    TRACE_FUNC_EXIT();
}

MBOOL GPUWarpStream::enque(WarpParam param)
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mMutex);
    MBOOL ret = MFALSE;
    if( mProcessThread != NULL )
    {
        param.mResult = param.mByPass || mGPUWarp.processWarp(param.mIn, param.mWarpOut, param.mWarpMap.mBuffer, param.mInSize, toWarpOutSize(param.mWarpOutSize));
        param.mRequest->mTimer.stopEnqueWarp(Timer::STOP_RUNNING);
        mProcessThread->enque(param);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MSize GPUWarpStream::toWarpOutSize(const MSizeF &inSize)
{
    TRACE_FUNC_ENTER();
    MSize outSize = MSize(inSize.w + 0.5, inSize.h + 0.5);
    TRACE_FUNC("inSize(%f,%f),outSize(%dx%d)", inSize.w, inSize.h, outSize.w, outSize.h);
    TRACE_FUNC_EXIT();
    return outSize;
}

WarpParam::MCropRect GPUWarpStream::toCropRect(const MRectF &inCropF)
{
    TRACE_FUNC_ENTER();
    WarpParam::MCropRect outCrop = WarpParam::MCropRect(MPoint(inCropF.p.x + 0.5, inCropF.p.y + 0.5), MSize(inCropF.s.w + 0.5, inCropF.s.h + 0.5));

    TRACE_FUNC("inCrop(%f,%f)(%fx%f), outCrop(%d,%d)(%dx%d)",
               inCropF.p.x, inCropF.p.y, inCropF.s.w, inCropF.s.h,
               outCrop.p_integral.x, outCrop.p_integral.y, outCrop.s.w, outCrop.s.h);
    TRACE_FUNC_EXIT();
    return outCrop;
}

GPUWarpStream::ProcessThread::ProcessThread()
    : mStop(MFALSE)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

GPUWarpStream::ProcessThread::~ProcessThread()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MVOID GPUWarpStream::ProcessThread::enque(const WarpParam &param)
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mThreadMutex);
    mQueue.push(param);
    mThreadCondition.broadcast();
    TRACE_FUNC_EXIT();
}

MVOID GPUWarpStream::ProcessThread::signalStop()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mThreadMutex);
    mStop = MTRUE;
    mThreadCondition.broadcast();
    TRACE_FUNC_EXIT();
}

android::status_t GPUWarpStream::ProcessThread::readyToRun()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return android::NO_ERROR;
}

bool GPUWarpStream::ProcessThread::threadLoop()
{
    TRACE_FUNC_ENTER();
    WarpParam param;

    while( waitParam(param) )
    {
        processParam(param);
        param = WarpParam();
    }

    TRACE_FUNC_EXIT();
    return false;
}

MBOOL GPUWarpStream::ProcessThread::waitParam(WarpParam &param)
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mThreadMutex);
    MBOOL ret = MTRUE, done = MFALSE;

    do
    {
        if( mQueue.size() )
        {
            param = mQueue.front();
            mQueue.pop();
            done = MTRUE;
        }
        else if( mStop )
        {
            ret = MFALSE;
            done = MTRUE;
        }
        else
        {
            mThreadCondition.wait(mThreadMutex);
        }
    }while( !done );

    TRACE_FUNC_EXIT();
    return ret;
}

MVOID GPUWarpStream::ProcessThread::processParam(WarpParam param)
{
    TRACE_FUNC_ENTER();
    if( param.mResult && param.mMDPOut.size() )
    {
        if( param.mRequest == NULL )
        {
            MY_LOGE("Missing request");
            param.mResult = MFALSE;
        }
        else
        {
            param.mRequest->mTimer.startWarpMDP();
            if( param.mByPass )
            {
                param.mResult = mMDP.process(param.mIn->getImageBufferPtr(), param.mMDPOut);
            }
            else
            {
                String8 str;
                MSize warpOutSize = toWarpOutSize(param.mWarpOutSize);
                str += String8::format("GPU:In(%dx%d),Out(%fx%f)=>(%dx%d).MDP[size=%d]:",
                                       param.mInSize.w, param.mInSize.h,
                                       param.mWarpOutSize.w, param.mWarpOutSize.h,
                                       warpOutSize.w, warpOutSize.h,
                                       param.mMDPOut.size());
                for( auto &out : param.mMDPOut )
                {
                    // Round to integral
                    out.mCropRect = toCropRect(out.mCropF);
                    str += String8::format("Crop(%f,%f)(%fx%f)=>(%d,%d)(%dx%d).",
                        out.mCropF.p.x, out.mCropF.p.y, out.mCropF.s.w, out.mCropF.s.h,
                        out.mCropRect.p_integral.x, out.mCropRect.p_integral.y,
                        out.mCropRect.s.w, out.mCropRect.s.h);
                }
                param.mResult = mMDP.process(param.mWarpOut->getImageBufferPtr(), param.mMDPOut);

                MY_LOGD("%s", str.string());
            }
            param.mRequest->mTimer.stopWarpMDP();
        }
    }

    if( param.mCallback )
    {
        param.mCallback(param);
    }
    else
    {
        MY_LOGE("Missing callback");
    }
    TRACE_FUNC_EXIT();
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
