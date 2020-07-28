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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_EIS_Q_CONTROL_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_EIS_Q_CONTROL_H_

#include "StreamingFeaturePipeUsage.h"
#include <deque>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

enum EISQ_ACTION
{
    EISQ_ACTION_NO,
    EISQ_ACTION_READY,
    EISQ_ACTION_INIT,
    EISQ_ACTION_PUSH,
    EISQ_ACTION_RUN,
    EISQ_ACTION_STOP,
};

class EISQState
{
public:
    MUINT32 mCounter = 0;
    EISQ_ACTION mAction = EISQ_ACTION_NO;
};

class EISQActionInfo
{
public:
    MINT32 mAppMode = IStreamingFeaturePipe::APP_PHOTO_PREVIEW;
    MUINT32 mRecordCount = 0;
    MBOOL mIsAppEIS = MFALSE;
    MBOOL mIsReady = MFALSE;
};

class EISQControl
{
public:
    MBOOL init(const StreamingFeaturePipeUsage &usage);
    MVOID update(const EISQActionInfo& status);
    MVOID reset();
    EISQState getCurrentState() const;

private:
    StreamingFeaturePipeUsage mPipeUsage;
    MBOOL mUseQ = MFALSE;
    MBOOL mUseTSQ = MFALSE;
    MUINT32 mPSize = 0;   // start frame
    MUINT32 mQSize = 0;   // queue size
    MUINT32 mPushCount = 0;
    EISQState mCurrent;
};

template <typename T>
T processEISQAction(EISQ_ACTION action, T data, std::deque<T> &queue, T def = T())
{
    T out = def;
    switch( action )
    {
    case EISQ_ACTION_NO:
        queue.clear();
        out = data;
        break;
    case EISQ_ACTION_READY:
        out = data;
        break;
    case EISQ_ACTION_INIT:
        out = data;
        break;
    case EISQ_ACTION_PUSH:
        queue.push_back(data);
        break;
    case EISQ_ACTION_RUN:
    case EISQ_ACTION_STOP:
        queue.push_back(data);
        out = queue.front();
        queue.pop_front();
        break;
    default:
        break;
    }
    return out;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_EIS_Q_CONTROL_H_
