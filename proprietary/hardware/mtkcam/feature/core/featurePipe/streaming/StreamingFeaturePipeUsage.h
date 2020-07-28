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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_PIPE_USAGE_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_PIPE_USAGE_H_

//#include "MtkHeader.h"
#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe.h>
#include <mtkcam/feature/3dnr/3dnr_defs.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class StreamingFeaturePipeUsage
{
public:
    StreamingFeaturePipeUsage();
    StreamingFeaturePipeUsage(IStreamingFeaturePipe::UsageHint hint, MUINT32 sensorIndex);

    MBOOL supportEISNode() const;
    MBOOL supportWarpNode() const;
    MBOOL supportMDPNode() const;
    MBOOL supportRSCNode() const;

    MBOOL support4K2K() const;
    MBOOL supportEISFullImg() const;
    MBOOL supportFOVCombineEIS() const;

    MBOOL supportTimeSharing() const;
    MBOOL supportP2AFeature() const;
    MBOOL supportBypassP2A() const;
    MBOOL supportYUVIn() const;

    MBOOL supportEIS_22() const;
    MBOOL supportEIS_25() const;
    MBOOL supportEIS_30() const;
    MBOOL supportEIS_Q() const;
    MBOOL supportEIS_TSQ() const;
    MBOOL supportFEFM() const;
    MBOOL supportRSC() const;
    MBOOL supportWPE() const;
    MBOOL supportWarpCrop() const;

    MBOOL supportDual() const;
    MBOOL supportN3D() const;
    MBOOL supportVendorFOV() const;
    MBOOL supportFOV() const;


    MBOOL support3DNR() const;
    MBOOL support3DNRRSC() const;
    MBOOL is3DNRModeMaskEnable(NR3D::E3DNR_MODE_MASK mask) const;
    MBOOL supportFSC() const;
    MBOOL supportVendorFSCFullImg() const;

    MBOOL supportVendor() const;
    MBOOL supportVendorCusSize() const;
    MBOOL supportVendorInplace() const;
    MBOOL supportVendorCusFormat() const;
    MBOOL supportVendorFullImg() const;
    MBOOL supportMultiSensor() const;
    MBOOL supportFull_YUY2() const;
    MBOOL supportGraphicBuffer() const;

    MBOOL supportVFB() const;


    IStreamingFeaturePipe::UsageHint getUsageHint() const;
    MUINT32 getMode() const;
    MUINT32 getEISMode() const;
    MUINT32 getVendorMode() const;
    MUINT32 get3DNRMode() const;
    MUINT32 getFSCMode() const;
    MUINT32 getVendorFOVMode() const;
    MSize   getStreamingSize() const;
    MSize   getVendorCusSize() const;

    MUINT32 getNumP2ABuffer() const;
    MUINT32 getNumWarpInBuffer() const;
    MUINT32 getNumExtraWarpInBuffer() const;
    MUINT32 getNumWarpOutBuffer() const;
    MUINT32 getNumFOVWarpOutBuffer() const;

    MUINT32 getNumVendorInBuffer() const;
    MUINT32 getNumVendorOutBuffer() const;
    MUINT32 getNumVendorFOVBuffer() const;
    MUINT32 getSensorIndex() const;
    MUINT32 getDualSensorIndex_Wide() const;
    MUINT32 getDualSensorIndex_Tele() const;
    MUINT32 getEISFactor() const;
    MUINT32 getEISQueueSize() const;
    MUINT32 getEISStartFrame() const;
    MUINT32 getEISVideoConfig() const;
    MUINT32 getWarpPrecision() const;
    MBOOL getSecureFlag() const;

private:
    enum P2A_MODE_ENUM
    {
        P2A_MODE_NORMAL,
        P2A_MODE_TIME_SHARING,
        P2A_MODE_FEATURE,
        P2A_MODE_BYPASS
    };

    class BufferNumInfo
    {
    public:
        BufferNumInfo()
            : mBasic(0)
            , mExtra(0)
        {
        }

        BufferNumInfo(MUINT32 basic, MUINT32 extra=0)
            : mBasic(basic)
            , mExtra(extra)
        {
        }

    public:
        MUINT32 mBasic;
        MUINT32 mExtra;
    };

    BufferNumInfo get3DNRBufferNum() const;
    BufferNumInfo getEISBufferNum() const;
    BufferNumInfo getVendorBufferNum() const;
    BufferNumInfo getDualFOVBufferNum() const;

    IStreamingFeaturePipe::UsageHint    mUsageHint;
    MUINT32                             mPipeFunc;
    MUINT32                             mP2AMode;

    MSize                               mStreamingSize;
    MUINT32                             mVendorMode;
    MSize                               mVendorCusSize;
    MBOOL                               mEnableVendorCusSize;
    MBOOL                               mEnableVendorInplace;
    MBOOL                               mEnableVendorCusFormat;
    MUINT32                             m3DNRMode;
    MUINT32                             mDualMode;
    MUINT32                             mSensorIndex;
    MUINT32                             mVendorFOVMode;
    MUINT32                             mDualWideSensorIndex;
    MUINT32                             mDualTeleSensorIndex;
    NSCam::SecType                      mSecType;
};

} // NSFeaturePipe
} // NSCamFeature
} // NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_PIPE_USAGE_H_

