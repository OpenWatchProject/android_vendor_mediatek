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

#ifndef _MTK_CAMERA_WPE_WARP_STREAM_H_
#define _MTK_CAMERA_WPE_WARP_STREAM_H_

#include "WarpStream.h"
#include "Dummy_WPEWarpStream.h"
#include "NormalStreamBase.h"
#include "WarpBase.h"
#include "WPEWarp.h"

#include <mtkcam/drv/iopipe/PostProc/IHalWpePipe.h>

typedef  NSCam::NSIoPipe::NSWpe::WPEQParams WPEQParams;
typedef  NSCam::NSIoPipe::PQParam PQParam;


namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class WPEEnqueData
{
public:
    WarpParam mWarpParam;
    WPEQParams mWPEQParam;
    PQParam mPQParam;
};

class WPEWarpStream : public virtual WarpStream , public virtual NormalStreamBase<WPEEnqueData>
{
public:
    typedef NSCam::NSIoPipe::QParams QParams;
    typedef NSCam::NSIoPipe::FrameParams FrameParams;

public:
    static WPEWarpStream* createInstance();

public:
    WPEWarpStream();
    virtual ~WPEWarpStream();
    MBOOL init(const MUINT32 &sensorIdx, const MSize &maxImageSize, const MSize &maxWarpSize);
    MVOID uninit();
    MBOOL enque(WarpParam param);

private:
    MVOID prepareQParams(QParams &param, WPEEnqueData &data, const WarpParam &warpParam);
    MVOID enqueNormalStream(const QParams &param, const WPEEnqueData &data);
    virtual MVOID onNormalStreamBaseCB(const QParams &param, const WPEEnqueData &data);

private:
    MVOID checkMDPOut(WPEEnqueData &data);
    MVOID setBasic(FrameParams &fparam, WPEEnqueData &data);
    MVOID setInBuffer(FrameParams &fparam, WPEEnqueData &data);
    MVOID setOutBuffer(FrameParams &fparam, WPEEnqueData &data);
    MVOID setMDPCrop(FrameParams &fparam, WPEEnqueData &data);
    MVOID setWPEQParams(FrameParams &fparam, WPEEnqueData &data);
    MVOID setWPEMode(WPEEnqueData &data);
    MVOID setWPECrop(WPEEnqueData &data);
    MVOID setWPEBuffer(WPEEnqueData &data);
    MVOID setPqParam(FrameParams &fparam, WPEEnqueData &data);

    static MSize toWPEOutSize(const WarpParam &param);
    static MCropRect toCropRect(const MRectF &corpF);
    static MCropRect toWPEScaleCrop(const WarpParam &param, const MRectF &srcCrop);

private:
    NSCam::NSIoPipe::NSPostProc::INormalStream *mNormalStream;

private:
    class ProcessThread : public android::Thread
    {
    public:
        ProcessThread();
        virtual ~ProcessThread();
        MVOID threadEnque(const WarpParam &param);
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

    android::sp<ProcessThread> mProcessThread;
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_WPE_WARP_STREAM_H_
