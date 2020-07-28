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
#define P2_CLASS_TAG    DispatchProcessor
#define P2_TRACE        TRACE_DISPATCH_PROCESSOR
#include "P2_LogHeader.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_PROC_COMMON);

DispatchProcessor::DispatchProcessor()
    : Processor(P2_DISPATCH_THREAD_NAME)
{
    MY_LOG_FUNC_ENTER();
    mForceProcessor = property_get_int32(KEY_P2_PROC_POLICY, VAL_P2_PROC_DFT_POLICY);
    MY_LOG_FUNC_EXIT();
}

DispatchProcessor::~DispatchProcessor()
{
    MY_LOG_S_FUNC_ENTER(mLog);
    this->uninit();
    MY_LOG_S_FUNC_EXIT(mLog);
}

MBOOL DispatchProcessor::onInit(const P2InitParam &param)
{
    ILog log = param.mP2Info.mLog;
    MY_LOG_S_FUNC_ENTER(log);
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_Dispatch:init()");

    MBOOL ret = MTRUE;

    mP2Info = param.mP2Info;
    mLog = mP2Info.mLog;

    mEnableBasic = (mForceProcessor == P2_POLICY_FORCE_BASIC)
                   || USE_BASIC_PROCESSOR
                   || (mP2Info.getConfigInfo().mP2Type == P2_HS_VIDEO);

    mEnableStreaming = (mP2Info.getConfigInfo().mP2Type != P2_HS_VIDEO) &&
                       (mForceProcessor == P2_POLICY_DYNAMIC
                       || mForceProcessor == P2_POLICY_FORCE_STREAMING);


    MY_LOGI("Enable Basic/Streaming (%d/%d)",
            mEnableBasic, mEnableStreaming);

    mBasicProcessor.setEnable(mEnableBasic);
    mStreamingProcessor.setEnable(mEnableStreaming);

    ret = ret
          && mBasicProcessor.init(param)
          && mStreamingProcessor.init(param);

    mDumpPlugin = new P2DumpPlugin();
    mScanlinePlugin = new P2ScanlinePlugin();
    mDrawIDPlugin = new P2DrawIDPlugin();

    MY_LOG_S_FUNC_EXIT(log);
    return ret;
}

MVOID DispatchProcessor::onUninit()
{
    MY_LOG_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_Dispatch:uninit()");

    mBasicProcessor.uninit();
    mStreamingProcessor.uninit();

    mDumpPlugin = NULL;
    mScanlinePlugin = NULL;
    MY_LOG_S_FUNC_EXIT(mLog);
}

MVOID DispatchProcessor::onThreadStart()
{
    MY_LOG_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_Dispatch:threadStart()");
    MY_LOG_S_FUNC_EXIT(mLog);
}

MVOID DispatchProcessor::onThreadStop()
{
    MY_LOG_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_Dispatch:threadStop()");
    MY_LOG_S_FUNC_EXIT(mLog);
}

MBOOL DispatchProcessor::onConfig(const P2ConfigParam &param)
{
    MY_LOG_S_FUNC_ENTER(mLog);
    MBOOL ret = MFALSE;
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_Dispatch:config()");
    mP2Info = param.mP2Info;
    ret = mBasicProcessor.config(param) &&
          mStreamingProcessor.config(param);
    MY_LOG_S_FUNC_EXIT(mLog);
    return ret;
}

MBOOL DispatchProcessor::onEnque(const sp<P2FrameRequest> &request)
{
    ILog reqLog = spToILog(request);
    TRACE_S_FUNC_ENTER(reqLog);
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_Dispatch:enque()");

    if( request != NULL )
    {
        P2DumpType dumpType = P2_DUMP_NONE;
        dumpType = mDumpPlugin->needDumpFrame(request->getFrameID());

        if( mScanlinePlugin->isEnabled() )
        {
            request->registerImgPlugin(mScanlinePlugin, MTRUE);
        }
        if( mDrawIDPlugin->isEnabled() )
        {
            request->registerImgPlugin(mDrawIDPlugin, MTRUE);
        }
        if( dumpType != P2_DUMP_NONE )
        {
            request->registerImgPlugin(mDumpPlugin);
        }

        std::vector<sp<P2Request>> p2Requests = request->extractP2Requests();
        if( mEnableStreaming )
        {
            for( const auto &it : p2Requests )
            {
                it->mDumpType = dumpType;
            }
            mStreamingProcessor.enque(p2Requests);
        }
        else
        {
            for( const auto &it : p2Requests )
            {
                if( it == NULL )
                {
                    MY_S_LOGW(reqLog, "Invalid P2Request = NULL");
                    continue;
                }

                if( reqLog.getLogLevel() >= 2 )
                {
                    it->dump();
                }
                it->mDumpType = dumpType;

                if( !it->isValidMeta(IN_APP) ||
                    !it->hasValidMirrorMeta(IN_P1_HAL) )
                {
                    MY_S_LOGW(reqLog, "Meta check failed: inApp(%d) inMirrorHal(%d)", it->isValidMeta(IN_APP), it->hasValidMirrorMeta(IN_P1_HAL));
                    continue;
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
    }

    TRACE_S_FUNC_EXIT(reqLog);
    return MTRUE;
}

MVOID DispatchProcessor::onNotifyFlush()
{
    TRACE_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_Dispatch:notifyFlush()");
    mBasicProcessor.notifyFlush();
    mStreamingProcessor.notifyFlush();
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID DispatchProcessor::onWaitFlush()
{
    TRACE_S_FUNC_ENTER(mLog);
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_Dispatch:waitFlush()");
    mBasicProcessor.waitFlush();
    mStreamingProcessor.waitFlush();
    TRACE_S_FUNC_EXIT(mLog);
}

MBOOL DispatchProcessor::needBasicProcess(const sp<P2Request> &request)
{
    ILog log = spToILog(request);
    MBOOL ret = MFALSE;
    TRACE_S_FUNC_ENTER(log);
    ret = (request != NULL && mEnableBasic &&
          request->hasInput() && request->hasOutput() );
    TRACE_S_FUNC(log, "in=%d out=%d ret=%d", request->hasInput(), request->hasOutput(), ret);
    TRACE_S_FUNC_EXIT(log, "ret=%d", ret);
    return ret;
}

} // namespace P2
