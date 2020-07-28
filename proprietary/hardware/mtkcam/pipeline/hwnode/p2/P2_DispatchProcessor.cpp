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

#include "P2_DispatchProcessor.h"

#define P2_DISPATCH_THREAD_NAME "p2_dispatch"

namespace P2
{

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    PartialP2Request
#define P2_TRACE        TRACE_PARTIAL_P2_REQUEST
#include "P2_LogHeader.h"

PartialP2Request::PartialP2Request(const sp<P2Request> &request)
    : P2FrameHolder(request)
    , LoggerHolder(request)
    , P2Request(request)
    , mFullRequest(request)
    , mIsResized(request->isResized())
    , mIsReprocess(request->isReprocess())
{
    TRACE_S_FUNC_ENTER(mLogger);
    TRACE_S_FUNC_EXIT(mLogger);
}

PartialP2Request::~PartialP2Request()
{
    TRACE_S_FUNC_ENTER(mLogger);
    TRACE_S_FUNC_EXIT(mLogger);
}

MBOOL PartialP2Request::isResized() const
{
    return mIsResized;
}

MBOOL PartialP2Request::isReprocess() const
{
    return mIsReprocess;
}

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    DispatchProcessor
#define P2_TRACE        TRACE_DISPATCH_PROCESSOR
#include "P2_LogHeader.h"

DispatchProcessor::DispatchProcessor()
    : Processor(P2_DISPATCH_THREAD_NAME)
    , mForceProcessor(VAL_P2_PROCESSOR)
    , mDebugDump(0)
    , mDebugDumpCount(0)
    , mEnableScanline(MFALSE)
{
    MY_LOG_FUNC_ENTER();
    mForceProcessor = property_get_int32(KEY_P2_PROCESSOR, VAL_P2_PROCESSOR);
    mBasicProcessor.setEnable(USE_BASIC_PROCESSOR);
    mCaptureProcessor.setEnable(USE_CAPTURE_PROCESSOR);
    MY_LOG_FUNC_EXIT();
}

DispatchProcessor::~DispatchProcessor()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    this->uninit();
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MBOOL DispatchProcessor::onInit(const P2InitParam &param)
{
    MY_LOG_S_FUNC_ENTER(param.mInfo);
    CAM_TRACE_NAME("P2_Dispatch:init()");

    MBOOL ret = MFALSE;

    mP2Info = param.mInfo;
    mConfigParam = param.mConfig;
    ret = mBasicProcessor.init(param) &&
          mStreamingProcessor.init(param) &&
          mCaptureProcessor.init(param);

    mDumpPlugin = new P2DumpPlugin();
    mScanlinePlugin = new P2ScanlinePlugin();
    MY_LOG_S_FUNC_EXIT(param.mInfo);
    return ret;
}

MVOID DispatchProcessor::onUninit()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Dispatch:uninit()");

    mBasicProcessor.uninit();
    mStreamingProcessor.uninit();
    mCaptureProcessor.uninit();

    mDumpPlugin = NULL;
    mScanlinePlugin = NULL;
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MVOID DispatchProcessor::onThreadStart()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Dispatch:threadStart()");
    mDebugDump = property_get_int32(KEY_P2_DUMP, VAL_P2_DUMP);
    mDebugDumpCount = property_get_int32(KEY_P2_DUMP_COUNT, VAL_P2_DUMP_COUNT);
    mEnableScanline = property_get_int32(KEY_P2_SCANLINE, VAL_P2_SCANLINE);
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MVOID DispatchProcessor::onThreadStop()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Dispatch:threadStop()");
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MBOOL DispatchProcessor::onConfig(const P2ConfigParam &param)
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    MBOOL ret = MFALSE;
    CAM_TRACE_NAME("P2_Dispatch:config()");
    mConfigParam = param;
    ret = mBasicProcessor.config(mConfigParam) &&
          mStreamingProcessor.config(mConfigParam) &&
          mCaptureProcessor.config(mConfigParam);
    MY_LOG_S_FUNC_EXIT(mP2Info);
    return ret;
}

MBOOL DispatchProcessor::onEnque(const sp<P2FrameRequest> &request)
{
    TRACE_S_FUNC_ENTER(request);
    CAM_TRACE_NAME("P2_Dispatch:enque()");

    if( request != NULL )
    {
        if( needDebugDump(request) )
        {
            request->registerImgPlugin(mDumpPlugin);
        }
        if( mEnableScanline )
        {
            request->registerImgPlugin(mScanlinePlugin);
        }

        std::vector<sp<P2Request>> p2Requests = request->extractP2Requests();
        for( const auto &it : p2Requests )
        {
            sp<P2Request> capture, streaming;

            if( it == NULL )
            {
                MY_S_LOGW(request, "Invalid P2Request = NULL");
                continue;
            }
            else if( request->getLogLevel() >= 1 )
            {
                it->dump();
            }

            if( !it->isValidMeta(IN_APP) || !it->isValidMeta(IN_P1_HAL) )
            {
                MY_S_LOGW(request, "Meta check failed: inApp(%d) inHal(%d)", it->isValidMeta(IN_APP), it->isValidMeta(IN_P1_HAL));
                continue;
            }

            capture = parseCaptureRequest(it);
            if( capture != NULL )
            {
                mCaptureProcessor.enque(capture);
            }

            streaming = parseStreamingRequest(it);
            if( streaming != NULL )
            {
                mStreamingProcessor.enque(streaming);
            }

            if( needBasicProcess(it) )
            {
                mBasicProcessor.enque(it);
            }
            else
            {
                it->releaseResource(P2Request::RES_ALL);
            }
        }
    }

    TRACE_S_FUNC_EXIT(request);
    return MTRUE;
}

MVOID DispatchProcessor::onNotifyFlush()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Dispatch:notifyFlush()");
    mBasicProcessor.notifyFlush();
    mStreamingProcessor.notifyFlush();
    mCaptureProcessor.notifyFlush();
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MVOID DispatchProcessor::onWaitFlush()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Dispatch:waitFlush()");
    mBasicProcessor.waitFlush();
    mStreamingProcessor.waitFlush();
    mCaptureProcessor.waitFlush();
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

sp<P2Request> DispatchProcessor::parseCaptureRequest(const sp<P2Request> &request)
{
    TRACE_S_FUNC_ENTER(request);
    (void)request;
    sp<P2Request> capture;
    TRACE_S_FUNC_EXIT(request);
    return capture;
}

sp<P2Request> DispatchProcessor::parseStreamingRequest(const sp<P2Request> &request)
{
    TRACE_S_FUNC_ENTER(request);
    sp<P2Request> streaming;
    if( request != NULL &&
        (!mForceProcessor || mForceProcessor == VAL_P2_STREAMING) )
    {
        streaming = new PartialP2Request(request);
        if( streaming == NULL )
        {
            MY_S_LOGW(request, "OOM: cannot allocate PartialP2Request");
        }
        else
        {
            streaming->mMeta[IN_APP]    = request->mMeta[IN_APP];
            streaming->mMeta[IN_P1_APP] = request->mMeta[IN_P1_APP];
            streaming->mMeta[IN_P1_HAL] = request->mMeta[IN_P1_HAL];
            streaming->mImg[IN_FULL]    = request->mImg[IN_FULL];
            streaming->mImg[IN_RESIZED] = request->mImg[IN_RESIZED];
            streaming->mImg[IN_LCSO]    = request->mImg[IN_LCSO];
            streaming->mImg[IN_RSSO]    = request->mImg[IN_RSSO];

            streaming->mMeta[OUT_APP]   = std::move(request->mMeta[OUT_APP]);
            streaming->mMeta[OUT_HAL]   = std::move(request->mMeta[OUT_HAL]);
            streaming->mImg[OUT_FD]     = std::move(request->mImg[OUT_FD]);
            streaming->mImgOutArray     = std::move(request->mImgOutArray);
        }
    }
    TRACE_S_FUNC_EXIT(request);
    return streaming;
}

MBOOL DispatchProcessor::needBasicProcess(const sp<P2Request> &request)
{
    MBOOL ret = MFALSE;
    TRACE_S_FUNC_ENTER(request);
    ret = (request != NULL && request->hasInput() && request->hasOutput() );
    TRACE_S_FUNC_EXIT(request);
    return ret;
}

MBOOL DispatchProcessor::needDebugDump(const sp<P2FrameRequest> &request)
{
    MBOOL ret = MFALSE;
    TRACE_S_FUNC_ENTER(request);
    MUINT32 id = request->getFrameID();
    ret = mDebugDump &&
          (id >= mDebugDump) &&
          (id < (mDebugDump + mDebugDumpCount));
    TRACE_S_FUNC_EXIT(request);
    return ret;
}

} // namespace P2
