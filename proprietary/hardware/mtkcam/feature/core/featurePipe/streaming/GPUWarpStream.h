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

#ifndef _MTK_CAMERA_GPU_WARP_STREAM_H_
#define _MTK_CAMERA_GPU_WARP_STREAM_H_

#include <queue>
#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <utils/Thread.h>

#include "WarpStream.h"
#include "GPUWarp.h"
#include "MDPWrapper.h"

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class GPUWarpStream : public virtual WarpStream
{
public:
    static GPUWarpStream* createInstance();

public:
    GPUWarpStream();
    virtual ~GPUWarpStream();
    MBOOL init(const MUINT32 &sensorIdx, const MSize &maxImageSize, const MSize &maxWarpSize);
    MVOID uninit();
    MBOOL enque(WarpParam param);

private:
    static MSize toWarpOutSize(const MSizeF &inSize);
    static WarpParam::MCropRect toCropRect(const MRectF &inCropF);

private:
    class ProcessThread : public android::Thread
    {
    public:
        ProcessThread();
        virtual ~ProcessThread();
        MVOID enque(const WarpParam &param);
        MVOID signalStop();

    public:
        android::status_t readyToRun();
        bool threadLoop();

    private:
        MBOOL waitParam(WarpParam &param);
        MVOID processParam(WarpParam param);

    private:
        std::queue<WarpParam> mQueue;
        android::Mutex mThreadMutex;
        android::Condition mThreadCondition;
        MBOOL mStop;
        MDPWrapper mMDP;
    };

private:
    android::Mutex mMutex;
    android::sp<ProcessThread> mProcessThread;
    GPUWarp mGPUWarp;
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_GPU_WARP_STREAM_H_
