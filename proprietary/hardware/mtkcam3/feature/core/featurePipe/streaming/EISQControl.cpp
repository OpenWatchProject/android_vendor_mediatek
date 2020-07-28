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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#include "EISQControl.h"

#include "DebugControl.h"
#define PIPE_CLASS_TAG "EISQControl"
#define PIPE_TRACE TRACE_EIS_Q_CONTROL
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

MBOOL EISQControl::init(const StreamingFeaturePipeUsage &usage)
{
    TRACE_FUNC_ENTER();
    mPipeUsage = usage;
    mUseQ = mPipeUsage.supportEIS_Q();
    mPSize = mPipeUsage.getEISStartFrame();
    mQSize = mPipeUsage.getEISQueueSize();
    mUseQ = mUseQ && mQSize;
    mUseTSQ = mUseQ && mPipeUsage.supportEIS_TSQ();
    mPushCount = 0;
    mCurrent = EISQState();
    MY_LOGI("useQ=%d tsQ=%d pSize=%d qSize=%d", mUseQ, mUseTSQ, mPSize, mQSize);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MVOID EISQControl::update(const EISQActionInfo& status)
{
    TRACE_FUNC_ENTER();
    EISQ_ACTION action = mCurrent.mAction;
    MUINT32 counter = mCurrent.mCounter;
    MBOOL isQR = mUseQ && (status.mAppMode == IStreamingFeaturePipe::APP_VIDEO_RECORD);
    MBOOL isQS = mUseQ && (status.mAppMode == IStreamingFeaturePipe::APP_VIDEO_STOP);

    switch(action)
    {
    case EISQ_ACTION_NO:
        action = isQR ? EISQ_ACTION_READY : EISQ_ACTION_NO;
        break;
    case EISQ_ACTION_READY:
        action = !isQR ? EISQ_ACTION_NO
                       : (status.mIsAppEIS && status.mRecordCount > mPSize && status.mIsReady) ?
                         EISQ_ACTION_INIT : EISQ_ACTION_READY;
        break;
    case EISQ_ACTION_INIT:
        action = isQR ? EISQ_ACTION_PUSH : EISQ_ACTION_NO;
        counter = isQR ? 1 : 0;
        break;
    case EISQ_ACTION_PUSH:
        action = isQR ? ((counter < mQSize) ? EISQ_ACTION_PUSH
                                            : EISQ_ACTION_RUN)
                      : (isQS ? EISQ_ACTION_STOP : EISQ_ACTION_NO);
        counter = isQR ? ((counter < mQSize) ? counter+1
                                             : counter)
                       : (isQS ? counter-1 : 0);
        mPushCount = (isQR || isQS) ? std::max(counter, mPushCount) : 0;
        break;
    case EISQ_ACTION_RUN:
        action = isQR ? action
                      : isQS ? EISQ_ACTION_STOP : EISQ_ACTION_NO;
        counter = isQR ? counter
                      : isQS ? mPushCount - 1 : 0;
        break;
    case EISQ_ACTION_STOP:
        action = isQS ? action : EISQ_ACTION_NO;
        counter = isQS && (counter > 0) ? counter-1 : 0;
        break;
    default:
        break;
    }

    if( action == EISQ_ACTION_NO )
    {
        mPushCount = 0;
    }

    mCurrent.mAction = action;
    mCurrent.mCounter = counter;

    TRACE_FUNC("AppMode=%d recNo=%d QAction=%d QCounter=%d", status.mAppMode, status.mRecordCount, mCurrent.mAction, mCurrent.mCounter);
    TRACE_FUNC_EXIT();
}

MVOID EISQControl::reset()
{
    TRACE_FUNC_ENTER();
    mPushCount = 0;
    mCurrent = EISQState();
    TRACE_FUNC_EXIT();
}

EISQState EISQControl::getCurrentState() const
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return mCurrent;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
