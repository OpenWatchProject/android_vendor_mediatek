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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_DUAL_CAM_STREAMING_FEATURE_PIPE_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_DUAL_CAM_STREAMING_FEATURE_PIPE_H_

#include <mtkcam/feature/DualCam/IDualCamStreamingFeaturePipe.h>
#include "SyncManager.h"

using namespace android;

namespace NSCam
{
namespace NSCamFeature
{
namespace NSFeaturePipe
{

class DualCamStreamingFeaturePipe : public IDualCamStreamingFeaturePipe
{
    public:
        static MINT32                    totalRefCount;
        MINT32                           mRefCount;

        DualCamStreamingFeaturePipe() = default;
        DualCamStreamingFeaturePipe(MUINT32 sensorIndex, const UsageHint& usageHint);
        ~DualCamStreamingFeaturePipe();

        static wp<IStreamingFeaturePipe> singletonStreamingFeaturePipe;

    protected:
        MINT32                           mSensorId = 0;
        sp<IStreamingFeaturePipe>        mStreamingFeaturePipe;
        sp<SyncManager>                  mSyncManager;
        Mutex                            mRequsetLock;

    public:
        virtual void onSynced(MUINT32 sensorId, MUINT32 frameNo, FeaturePipeParam& param);
        virtual void onCallback(MUINT32 sensorId, MUINT32 frameNo, FeaturePipeParam& param,
                        FeaturePipeParam::MSG_TYPE type, MBOOL isDrop);

    public:
        // IStreamingFeaturePipe Interface
        MBOOL init(const char* name = NULL) override;
        MBOOL uninit(const char* name = NULL) override;
        MBOOL enque(const FeaturePipeParam& param) override;
        MBOOL flush() override;
        MBOOL setJpegParam(NSCam::NSIoPipe::NSPostProc::EJpgCmd cmd, int arg1, int arg2) override;
        MBOOL setFps(MINT32 fps) override;
        MBOOL sendCommand(NSCam::NSIoPipe::NSPostProc::ESDCmd cmd, MINTPTR arg1 = 0,
                          MINTPTR arg2 = 0, MINTPTR arg3 = 0) override;
        MBOOL addMultiSensorId(MUINT32 sensorId) override;

        MUINT32 getRegTableSize() override;
        MVOID sync() override;
        IImageBuffer* requestBuffer() override;
        MBOOL returnBuffer(IImageBuffer* buffer) override;

    protected:
        MINT32 getTimestamp(IMetadata* a);
};

} // NSFeaturePipe
} // NSCamFeature
} // NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_DUAL_CAM_STREAMING_FEATURE_PIPE_H_
