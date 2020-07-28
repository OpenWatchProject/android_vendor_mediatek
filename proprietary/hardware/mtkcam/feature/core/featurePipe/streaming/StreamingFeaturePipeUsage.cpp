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

#include "StreamingFeaturePipeUsage.h"
#include "StreamingFeaturePipe.h"
#include <mtkcam/feature/eis/eis_ext.h>
#include <mtkcam/feature/stereo/StereoCamEnum.h>
#include <mtkcam/feature/3dnr/3dnr_defs.h>
#include <mtkcam/feature/fsc/fsc_defs.h>
#include <camera_custom_eis.h>
#include <camera_custom_dualzoom.h>

// draft, just for offline fov debug
#include <cutils/properties.h>

#define PIPE_CLASS_TAG "PipeUsage"
#define PIPE_TRACE TRACE_STREAMING_FEATURE_USAGE
#include <featurePipe/core/include/PipeLog.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

typedef IStreamingFeaturePipe::UsageHint UsageHint;
#define ADD_3DNR_RSC_SUPPORT 1

StreamingFeaturePipeUsage::StreamingFeaturePipeUsage()
    : mPipeFunc(0)
    , mP2AMode(0)
    , mVendorMode(0)
    , mEnableVendorCusSize(MFALSE)
    , mEnableVendorInplace(MFALSE)
    , mEnableVendorCusFormat(MFALSE)
    , m3DNRMode(0)
    , mDualMode(0)
    , mSensorIndex(0)
    , mVendorFOVMode(0)
    , mDualWideSensorIndex(0)
    , mDualTeleSensorIndex(0)
    , mSecType(NSCam::SecType::mem_normal)
{

}


StreamingFeaturePipeUsage::StreamingFeaturePipeUsage(UsageHint hint, MUINT32 sensorIndex)
    : mUsageHint(hint)
    , mPipeFunc(0)
    , mP2AMode(0)
    , mStreamingSize(mUsageHint.mStreamingSize)
    , mVendorMode(mUsageHint.mVendorMode)
    , mVendorCusSize(mUsageHint.mVendorCusSize)
    , mEnableVendorCusSize(MFALSE)
    , mEnableVendorInplace(MFALSE)
    , mEnableVendorCusFormat(MFALSE)
    , m3DNRMode(mUsageHint.m3DNRMode)
    , mDualMode(mUsageHint.mDualMode)
    , mSensorIndex(sensorIndex)
    , mVendorFOVMode(mUsageHint.mVendorFOVMode)
    , mDualWideSensorIndex(0)
    , mDualTeleSensorIndex(0)
    , mSecType(hint.mSecType)
{
    if (mUsageHint.mMode == IStreamingFeaturePipe::USAGE_DEFAULT)
    {
        mUsageHint.mMode = IStreamingFeaturePipe::USAGE_FULL;
    }

    //remove future
    if (mSecType == SecType::mem_normal)
    {
        mUsageHint.mVendorMode = (getPropertyValue(KEY_ENABLE_VENDOR, SUPPORT_VENDOR_NODE) == 1 );
        mVendorMode = mUsageHint.mVendorMode;
        mEnableVendorCusSize = (getPropertyValue(KEY_ENABLE_VENDOR_CUS_SIZE, SUPPORT_VENDOR_CUS_SIZE) == 1 );
        mEnableVendorInplace = (getPropertyValue(KEY_ENABLE_VENDOR_INPLACE, SUPPORT_VENDOR_INPLACE) == 1 );
        mEnableVendorCusFormat = (getPropertyValue(KEY_ENABLE_VENDOR_CUS_FORMAT, SUPPORT_VENDOR_CUS_FORMAT) == 1 );

        mVendorFOVMode = (getPropertyValue(KEY_ENABLE_VENDOR_FOV, SUPPORT_VENDOR_FOV_NODE) == 1 );
    }

    switch (mUsageHint.mMode)
    {
    case IStreamingFeaturePipe::USAGE_P2A_PASS_THROUGH_TIME_SHARING:
        mPipeFunc = 0;
        mP2AMode = P2A_MODE_TIME_SHARING;
        break;
    case IStreamingFeaturePipe::USAGE_P2A_FEATURE:
        mPipeFunc = IStreamingFeaturePipe::PIPE_USAGE_3DNR;
        mP2AMode = P2A_MODE_FEATURE;
        break;
    case IStreamingFeaturePipe::USAGE_FULL:
        mPipeFunc = IStreamingFeaturePipe::PIPE_USAGE_EIS | IStreamingFeaturePipe::PIPE_USAGE_3DNR | IStreamingFeaturePipe::PIPE_USAGE_EARLY_DISPLAY;
        mP2AMode = P2A_MODE_FEATURE;
        break;
    case IStreamingFeaturePipe::USAGE_STEREO_EIS:
        mPipeFunc = IStreamingFeaturePipe::PIPE_USAGE_EIS;
        mP2AMode = P2A_MODE_BYPASS;
        break;
    case IStreamingFeaturePipe::USAGE_P2A_PASS_THROUGH:
    default:
        mPipeFunc = 0;
        mP2AMode = P2A_MODE_NORMAL;
        break;
    }

    if( mEnableVendorCusSize && supportEISNode() )
    {
        MY_LOGW("Force disable VendorCusSize because of EIS constrain");
        mEnableVendorCusSize = MFALSE;
    }

    mDualWideSensorIndex = supportDual() ? DUALZOOM_WIDE_CAM_ID : 0;
    mDualTeleSensorIndex = supportDual() ? DUALZOOM_TELE_CAM_ID : 0;
}

MBOOL StreamingFeaturePipeUsage::supportEISNode() const
{
    return (mPipeFunc & IStreamingFeaturePipe::PIPE_USAGE_EIS) &&
           ( EIS_MODE_IS_EIS_30_ENABLED(mUsageHint.mEISInfo.mode) ||
             EIS_MODE_IS_EIS_25_ENABLED(mUsageHint.mEISInfo.mode) ||
             EIS_MODE_IS_EIS_22_ENABLED(mUsageHint.mEISInfo.mode) );
}

MBOOL StreamingFeaturePipeUsage::supportWarpNode() const
{
    return supportEISNode();
}

MBOOL StreamingFeaturePipeUsage::supportMDPNode() const
{
    return supportEISNode();
}

MBOOL StreamingFeaturePipeUsage::supportRSCNode() const
{
    return ((mPipeFunc & IStreamingFeaturePipe::PIPE_USAGE_EIS) &&
           supportEIS_30() && EIS_MODE_IS_EIS_IMAGE_ENABLED(mUsageHint.mEISInfo.mode)) ||
           support3DNRRSC();
}

MBOOL StreamingFeaturePipeUsage::support4K2K() const
{
    return is4K2K(mStreamingSize);
}

MBOOL StreamingFeaturePipeUsage::supportEISFullImg() const
{
    return ( (supportDual() && !supportFOVCombineEIS()) || supportVendor()) && supportEISNode();
}

MBOOL StreamingFeaturePipeUsage::supportFOVCombineEIS() const
{
    MUINT32 videoType = mUsageHint.mEISInfo.videoConfig;
    return supportDual() && EISCustom::isEnabledFOVWarpCombine(videoType) && !supportVendor();
}

MBOOL StreamingFeaturePipeUsage::supportTimeSharing() const
{
    return mP2AMode == P2A_MODE_TIME_SHARING;
}

MBOOL StreamingFeaturePipeUsage::supportP2AFeature() const
{
    return mP2AMode == P2A_MODE_FEATURE;
}

MBOOL StreamingFeaturePipeUsage::supportBypassP2A() const
{
    return mP2AMode == P2A_MODE_BYPASS;
}

MBOOL StreamingFeaturePipeUsage::supportYUVIn() const
{
    return mP2AMode == P2A_MODE_BYPASS;
}

MBOOL StreamingFeaturePipeUsage::supportEIS_22() const
{
    return EIS_MODE_IS_EIS_22_ENABLED(mUsageHint.mEISInfo.mode) && supportEISNode();
}

MBOOL StreamingFeaturePipeUsage::supportEIS_25() const
{
    return EIS_MODE_IS_EIS_25_ENABLED(mUsageHint.mEISInfo.mode) && supportEISNode();
}

MBOOL StreamingFeaturePipeUsage::supportEIS_30() const
{
    return EIS_MODE_IS_EIS_30_ENABLED(mUsageHint.mEISInfo.mode) && supportEISNode();
}

MBOOL StreamingFeaturePipeUsage::supportEIS_Q() const
{
    return ( supportEIS_25() || supportEIS_30() ) && EIS_MODE_IS_EIS_QUEUE_ENABLED(mUsageHint.mEISInfo.mode);
}

MBOOL StreamingFeaturePipeUsage::supportEIS_TSQ() const
{
    return mUsageHint.mUseTSQ && supportEIS_Q();
}

MBOOL StreamingFeaturePipeUsage::supportFEFM() const
{
    return supportEIS_25() && EIS_MODE_IS_EIS_IMAGE_ENABLED(mUsageHint.mEISInfo.mode);
}

MBOOL StreamingFeaturePipeUsage::supportRSC() const
{
    return supportEIS_30() && EIS_MODE_IS_EIS_IMAGE_ENABLED(mUsageHint.mEISInfo.mode);
}

MBOOL StreamingFeaturePipeUsage::supportWPE() const
{
    return ( supportEIS_30() || supportDual() ) && NSCam::NSIoPipe::WPEQuerySupport();
}

MBOOL StreamingFeaturePipeUsage::supportWarpCrop() const
{
    if( supportWPE() )
    {
        return MFALSE;
    }
    return SUPPORT_GPU_CROP;
}

MBOOL StreamingFeaturePipeUsage::supportDual() const
{
    return ( mDualMode == v1::Stereo::E_DUALCAM_FEATURE_ZOOM );
}
MBOOL StreamingFeaturePipeUsage::supportN3D() const
{
#if (MTKCAM_HAVE_DUALCAM_DENOISE_SUPPORT == 1)
    return ( mDualMode == v1::Stereo::E_STEREO_FEATURE_DENOISE );
#else
    return MFALSE;
#endif
}

MBOOL StreamingFeaturePipeUsage::supportVendorFOV() const
{
    return supportDual() && mVendorFOVMode && (mP2AMode == P2A_MODE_FEATURE);
}

MBOOL StreamingFeaturePipeUsage::supportFOV() const
{
    return supportDual() && !supportVendorFOV();
}

MBOOL StreamingFeaturePipeUsage::support3DNR() const
{
    MBOOL ret = MFALSE;
    // could use property to enalbe 3DNR
    // assign value at getPipeUsageHint
    if (this->is3DNRModeMaskEnable(NR3D::E3DNR_MODE_MASK_HAL_FORCE_SUPPORT) ||
        this->is3DNRModeMaskEnable(NR3D::E3DNR_MODE_MASK_UI_SUPPORT))
    {
        ret = this->supportP2AFeature();
    }

#if 0
    // TODO: remove

    #ifdef SUPPORT_3DNR
    ret = this->supportP2AFeature();
    #endif
#endif
    return ret;
}

MBOOL StreamingFeaturePipeUsage::support3DNRRSC() const
{
    MBOOL ret = MFALSE;
#if ADD_3DNR_RSC_SUPPORT
    if (support3DNR() && is3DNRModeMaskEnable(NR3D::E3DNR_MODE_MASK_RSC_EN))
    {
        ret = MTRUE;
    }
#endif // ADD_3DNR_RSC_SUPPORT
    return ret;
}

MBOOL StreamingFeaturePipeUsage::is3DNRModeMaskEnable(NR3D::E3DNR_MODE_MASK mask) const
{
    return (m3DNRMode & mask);
}

MBOOL StreamingFeaturePipeUsage::supportFSC() const
{
    MBOOL ret = MFALSE;

    ret = (mUsageHint.mFSCMode & NSCam::FSC::EFSC_MODE_MASK_FSC_EN);

    return ret;
}

MBOOL StreamingFeaturePipeUsage::supportVendorFSCFullImg() const
{
    return supportVendor() && supportFSC() && !supportEISNode();
}

MBOOL StreamingFeaturePipeUsage::supportVendor() const
{
    return mVendorMode && (mP2AMode == P2A_MODE_FEATURE);
}

MBOOL StreamingFeaturePipeUsage::supportVendorCusSize() const
{
    return supportVendor() && mEnableVendorCusSize;
}

MBOOL StreamingFeaturePipeUsage::supportVendorInplace() const
{
    return supportVendor() && mEnableVendorInplace;
}

MBOOL StreamingFeaturePipeUsage::supportVendorCusFormat() const
{
    return supportVendor() && mEnableVendorCusFormat;
}

MBOOL StreamingFeaturePipeUsage::supportVendorFullImg() const
{
    return supportVendorCusSize() || supportVendorInplace() || supportVendorCusFormat() || supportVendorFSCFullImg();
}

MBOOL StreamingFeaturePipeUsage::supportMultiSensor() const
{
    return supportDual();
}

MBOOL StreamingFeaturePipeUsage::supportFull_YUY2() const
{
    return USE_YUY2_FULL_IMG && supportWPE() ;
}

MBOOL StreamingFeaturePipeUsage::supportGraphicBuffer() const
{
    return !supportWPE();
}

MBOOL StreamingFeaturePipeUsage::supportVFB() const
{
    MBOOL ret = MFALSE;
    #if SUPPORT_VFB
    ret = (mUsageHint.mMode == USAGE_FULL);
    #endif // SUPPORT_VFB
    return ret;
}

UsageHint StreamingFeaturePipeUsage::getUsageHint() const
{
    return mUsageHint;
}

MUINT32 StreamingFeaturePipeUsage::getMode() const
{
    return mUsageHint.mMode;
}

MUINT32 StreamingFeaturePipeUsage::getEISMode() const
{
    return mUsageHint.mEISInfo.mode;
}

MUINT32 StreamingFeaturePipeUsage::getEISFactor() const
{
    return mUsageHint.mEISInfo.factor ? mUsageHint.mEISInfo.factor : 100;
}

MUINT32 StreamingFeaturePipeUsage::getVendorMode() const
{
    return mVendorMode;
}

MUINT32 StreamingFeaturePipeUsage::get3DNRMode() const
{
    return m3DNRMode;
}

MUINT32 StreamingFeaturePipeUsage::getFSCMode() const
{
    return mUsageHint.mFSCMode;
}

MUINT32 StreamingFeaturePipeUsage::getVendorFOVMode() const
{
    return mVendorFOVMode;
}

MSize StreamingFeaturePipeUsage::getStreamingSize() const
{
    return mStreamingSize;
}

MSize StreamingFeaturePipeUsage::getVendorCusSize() const
{
    return mVendorCusSize;
}

MUINT32 StreamingFeaturePipeUsage::getNumP2ABuffer() const
{
    MUINT32 num = 0;
    if( !supportWarpNode() || supportEISFullImg() )
    {
        num = max(num, get3DNRBufferNum().mBasic);
        num = max(num, getVendorBufferNum().mBasic);
    }

    if( supportDual() )
    {
        num = max(num, get3DNRBufferNum().mBasic);
        num = max(num, getDualFOVBufferNum().mBasic);
    }
    return num;
}

MUINT32 StreamingFeaturePipeUsage::getNumWarpInBuffer() const
{
    MUINT32 num = 0;
    num = max(num, getEISBufferNum().mBasic);
    num += getVendorBufferNum().mBasic;
    return num;
}

MUINT32 StreamingFeaturePipeUsage::getNumExtraWarpInBuffer() const
{
    MUINT32 num = 0;
    num = max(num, getEISBufferNum().mExtra);
    num += getVendorBufferNum().mExtra;
    return num;
}

MUINT32 StreamingFeaturePipeUsage::getNumWarpOutBuffer() const
{
    return supportWarpNode() ? 3 : 0;
}

MUINT32 StreamingFeaturePipeUsage::getNumFOVWarpOutBuffer() const
{
    MUINT32 num = 0;
    if (supportDual() && supportVendor())
    {
        num = max(getDualFOVBufferNum().mBasic, getVendorBufferNum().mBasic);
    }
    return num;
}

MUINT32 StreamingFeaturePipeUsage::getNumVendorInBuffer() const
{
    MUINT32 num = 0;
    num = max(num,  getVendorBufferNum().mBasic);
    return num;
}

MUINT32 StreamingFeaturePipeUsage::getNumVendorOutBuffer() const
{
    MUINT32 num = 0;
    num = max(num,  getVendorBufferNum().mBasic);
    return num;
}

MUINT32 StreamingFeaturePipeUsage::getNumVendorFOVBuffer() const
{
    MUINT32 num = 0;
    num = max(num, getDualFOVBufferNum().mBasic * 2); // for master and slave
    return num;
}

StreamingFeaturePipeUsage::BufferNumInfo StreamingFeaturePipeUsage::get3DNRBufferNum() const
{
    MUINT32 num = 0;
    if( support3DNR() )
    {
        num = 3;
        if( supportDual() )
        {
            num += 1;
        }
    }

    return BufferNumInfo(num);
}

StreamingFeaturePipeUsage::BufferNumInfo StreamingFeaturePipeUsage::getEISBufferNum() const
{
    MUINT32 basic = 0, extra = 0;
    if( supportEISNode() )
    {
        basic = 5;
        if( supportEIS_Q() )
        {
            extra = getEISQueueSize();
        }
    }
    return BufferNumInfo(basic, extra);
}

StreamingFeaturePipeUsage::BufferNumInfo StreamingFeaturePipeUsage::getVendorBufferNum() const
{
    return supportVendor() ? BufferNumInfo(3) : BufferNumInfo(0);
}

StreamingFeaturePipeUsage::BufferNumInfo StreamingFeaturePipeUsage::getDualFOVBufferNum() const
{
    return supportDual() ? BufferNumInfo(3) : BufferNumInfo(0);
}

MUINT32 StreamingFeaturePipeUsage::getSensorIndex() const
{
    return mSensorIndex;
}

MUINT32 StreamingFeaturePipeUsage::getDualSensorIndex_Wide() const
{
    return mDualWideSensorIndex;
}

MUINT32 StreamingFeaturePipeUsage::getDualSensorIndex_Tele() const
{
    return mDualTeleSensorIndex;
}

MUINT32 StreamingFeaturePipeUsage::getEISQueueSize() const
{
    return mUsageHint.mEISInfo.queueSize;
}

MUINT32 StreamingFeaturePipeUsage::getEISStartFrame() const
{
    return mUsageHint.mEISInfo.startFrame;
}

MUINT32 StreamingFeaturePipeUsage::getEISVideoConfig() const
{
    return mUsageHint.mEISInfo.videoConfig;
}

MUINT32 StreamingFeaturePipeUsage::getWarpPrecision() const
{
    return supportWPE() ? 5 : WARP_MAP_PRECISION_BIT;
}

MBOOL StreamingFeaturePipeUsage::getSecureFlag() const
{
    return ((MINT)mSecType > 0) ? MTRUE : MFALSE;
}

} // NSFeaturePipe
} // NSCamFeature
} // NSCam
