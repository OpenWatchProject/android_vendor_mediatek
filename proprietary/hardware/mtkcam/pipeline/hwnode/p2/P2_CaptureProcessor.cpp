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

#include "P2_CaptureProcessor.h"

#define P2_CAPTURE_THREAD_NAME "p2_capture"

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    CaptureProcessor
#define P2_TRACE        TRACE_CAPTURE_PROCESSOR
#include "P2_LogHeader.h"

namespace P2
{

CaptureProcessor::CaptureProcessor()
    : Processor(P2_CAPTURE_THREAD_NAME)
{
    MY_LOG_FUNC_ENTER();
    MY_LOG_FUNC_EXIT();
}

CaptureProcessor::~CaptureProcessor()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    this->uninit();
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MBOOL CaptureProcessor::onInit(const P2InitParam &param)
{
    MY_LOG_S_FUNC_ENTER(param.mInfo);
    CAM_TRACE_NAME("P2_Capture:init()");
    MBOOL ret = MTRUE;
    mP2Info = param.mInfo;
    mConfigParam = param.mConfig;
    MY_LOG_S_FUNC_EXIT(param.mInfo);
    return ret;
}

MVOID CaptureProcessor::onUninit()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Capture:uninit()");
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MVOID CaptureProcessor::onThreadStart()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Capture:threadStart()");
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MVOID CaptureProcessor::onThreadStop()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Capture:threadStop()");
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MBOOL CaptureProcessor::onConfig(const P2ConfigParam &param)
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Capture:config()");
    mConfigParam = param;
    MY_LOG_S_FUNC_EXIT(mP2Info);
    return MTRUE;
}

MBOOL CaptureProcessor::onEnque(const sp<P2Request> &request)
{
    (void)request;
    CAM_TRACE_NAME("P2_Capture:enque()");
    MBOOL ret = MTRUE;
    return ret;
}

MVOID CaptureProcessor::onNotifyFlush()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Capture:notifyFlush()");
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

MVOID CaptureProcessor::onWaitFlush()
{
    MY_LOG_S_FUNC_ENTER(mP2Info);
    CAM_TRACE_NAME("P2_Capture:waitFlush()");
    MY_LOG_S_FUNC_EXIT(mP2Info);
}

} // namespace P2
