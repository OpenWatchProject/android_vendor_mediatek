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
#include "TuningHelper.h"
#include <mtkcam3/feature/eis/eis_ext.h>
#include <mtkcam3/feature/stereo/StereoCamEnum.h>
#include <mtkcam3/feature/3dnr/3dnr_defs.h>
#include <mtkcam3/feature/fsc/fsc_defs.h>
#include <camera_custom_eis.h>
#include <camera_custom_dualzoom.h>

// draft, just for offline fov debug
#include <cutils/properties.h>

#define PIPE_CLASS_TAG "PipeUsage"
#define PIPE_TRACE TRACE_STREAMING_FEATURE_USAGE
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

typedef IStreamingFeaturePipe::UsageHint UsageHint;
#define ADD_3DNR_RSC_SUPPORT 1
#define P2A_TOTAL_PATH_NUM 3 // general, physical, large

StreamingFeaturePipeUsage::StreamingFeaturePipeUsage()
{

}


StreamingFeaturePipeUsage::StreamingFeaturePipeUsage(UsageHint hint, MUINT32 sensorIndex)
    : mUsageHint(hint)
    , mStreamingSize(mUsageHint.mStreamingSize)
    , m3DNRMode(mUsageHint.m3DNRMode)
    , mFSCMode(mUsageHint.mFSCMode)
    , mDualMode(mUsageHint.mDualMode)
    , mSensorIndex(sensorIndex)
    , mOutCfg(mUsageHint.mOutCfg)
    , mNumSensor(mUsageHint.mAllSensorIDs.size())
    , mSecType(hint.mSecType)
    , mResizedRawSizeMap(mUsageHint.mResizedRawMap)
{
    if (mUsageHint.mMode == IStreamingFeaturePipe::USAGE_DEFAULT)
    {
        mUsageHint.mMode = IStreamingFeaturePipe::USAGE_FULL;
    }

    //remove future
    mVendorDebug = getPropertyValue(KEY_DEBUG_TPI, 0);
    mVendorLog = getPropertyValue(KEY_DEBUG_TPI_LOG, 0);
    mVendorErase = getPropertyValue(KEY_DEBUG_TPI_ERASE, 0);
    mSupportPure = (getPropertyValue(KEY_ENABLE_PURE_YUV, SUPPORT_PURE_YUV) == 1 ); // TODO test, remove later
    mForceIMG3O = getPropertyValue(KEY_FORCE_IMG3O, VAL_FORCE_IMG3O);

    switch (mUsageHint.mMode)
    {
    case IStreamingFeaturePipe::USAGE_P2A_FEATURE:
        mPipeFunc = PIPE_USAGE_EIS | PIPE_USAGE_3DNR | PIPE_USAGE_VENDOR | PIPE_USAGE_DSDN;
        mP2AMode = P2A_MODE_FEATURE;
        break;
    case IStreamingFeaturePipe::USAGE_FULL:
        mPipeFunc = PIPE_USAGE_EIS | PIPE_USAGE_3DNR | PIPE_USAGE_EARLY_DISPLAY | PIPE_USAGE_VENDOR | PIPE_USAGE_DSDN;
        mP2AMode = P2A_MODE_FEATURE;
        break;
    case IStreamingFeaturePipe::USAGE_STEREO_EIS:
        mPipeFunc = PIPE_USAGE_EIS;
        mP2AMode = P2A_MODE_BYPASS;
        break;
    case IStreamingFeaturePipe::USAGE_BATCH_SMVR:
        mPipeFunc = PIPE_USAGE_SMVR;
        mP2AMode = P2A_MODE_DISABLE;
        break;
    case IStreamingFeaturePipe::USAGE_P2A_PASS_THROUGH:
    default:
        mPipeFunc = PIPE_USAGE_VENDOR;
        mP2AMode = P2A_MODE_NORMAL;
        break;
    }

    if(m3DNRMode && !support3DNR())
    {
        MY_LOGE("3DNR not supportted! But UsageHint 3DNR Mode(%d) enabled!!", m3DNRMode);
    }

    updateOutSize(hint.mOutSizeVector);

    mDSDNCfg.config(mStreamingSize, mOutCfg.mFDSize, mUsageHint.mDSDNParam.mMaxRatioMultiple, mUsageHint.mDSDNParam.mMaxRatioDivider);
}

MVOID StreamingFeaturePipeUsage::config(const TPIMgr *mgr)
{
    mTPIUsage.config(mgr);
    mSupportSharedInput = supportP2AP2() && (!supportTPI(TPIOEntry::YUV) ||
        mTPIUsage.use(TPIOEntry::YUV, TPIOUse::SHARED_INPUT));
    MY_LOGI("config usage : mSuppoRtSharedInput(%d)", mSupportSharedInput);
}

MBOOL StreamingFeaturePipeUsage::supportP2AP2() const
{
    return !supportDepthP2() && !supportSMP2();
}

MBOOL StreamingFeaturePipeUsage::supportDepthP2() const
{
    return supportDPE();
}

MBOOL StreamingFeaturePipeUsage::supportSMP2() const
{
    return mPipeFunc & PIPE_USAGE_SMVR;
}

MBOOL StreamingFeaturePipeUsage::supportLargeOut() const
{
    return mOutCfg.mHasLarge;
}
MBOOL StreamingFeaturePipeUsage::supportPhysicalOut() const
{
    return mOutCfg.mHasPhysical;
}

MBOOL StreamingFeaturePipeUsage::supportIMG3O() const
{
#ifdef SUPPORT_IMG3O
    return MTRUE;
#else
    return MFALSE;
#endif
}

MBOOL StreamingFeaturePipeUsage::supportP2NRNode() const
{
    return (mPipeFunc & PIPE_USAGE_DSDN) &&
           supportDSDN20();
}

MBOOL StreamingFeaturePipeUsage::supportVNRNode() const
{
    return (mPipeFunc & PIPE_USAGE_DSDN) &&
           supportDSDN20();
}

MBOOL StreamingFeaturePipeUsage::supportEISNode() const
{
    return (mPipeFunc & PIPE_USAGE_EIS) &&
           ( EIS_MODE_IS_EIS_30_ENABLED(mUsageHint.mEISInfo.mode) ||
             EIS_MODE_IS_EIS_22_ENABLED(mUsageHint.mEISInfo.mode) );
}

MBOOL StreamingFeaturePipeUsage::supportWarpNode() const
{
    return supportEISNode();
}

MBOOL StreamingFeaturePipeUsage::supportRSCNode() const
{
    return ((mPipeFunc & PIPE_USAGE_EIS) &&
            supportEISRSC()) ||
           ((mPipeFunc & PIPE_USAGE_3DNR) &&
            support3DNRRSC());
}

MBOOL StreamingFeaturePipeUsage::supportTOFNode() const
{
    return supportTOF();
}

MBOOL StreamingFeaturePipeUsage::supportP2ALarge() const
{
    return mOutCfg.mHasLarge;
}

MBOOL StreamingFeaturePipeUsage::support4K2K() const
{
    return is4K2K(mStreamingSize);
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

MBOOL StreamingFeaturePipeUsage::supportP1YUVIn() const
{
    return mUsageHint.mInCfg.mType == IStreamingFeaturePipe::InConfig::P1YUV_IN;
}

MBOOL StreamingFeaturePipeUsage::supportPure() const
{
    return mSupportPure;
}

MBOOL StreamingFeaturePipeUsage::supportPreviewEIS() const
{
    return mUsageHint.mEISInfo.previewEIS && supportEISNode();
}

MBOOL StreamingFeaturePipeUsage::supportEIS_Q() const
{
    return mUsageHint.mEISInfo.supportQ;
}

MBOOL StreamingFeaturePipeUsage::supportEIS_TSQ() const
{
    return mUsageHint.mUseTSQ && supportEIS_Q();
}

MBOOL StreamingFeaturePipeUsage::supportEISRSC() const
{
    return mUsageHint.mEISInfo.supportRSC;
}

MBOOL StreamingFeaturePipeUsage::supportWPE() const
{
    return EIS_MODE_IS_EIS_30_ENABLED(mUsageHint.mEISInfo.mode) && NSCam::NSIoPipe::WPEQuerySupport();
}

MBOOL StreamingFeaturePipeUsage::supportWarpCrop() const
{
    return supportWPE() ? MFALSE : SUPPORT_GPU_CROP;
}

MBOOL StreamingFeaturePipeUsage::supportDual() const
{
    return  getNumSensor() >= 2;
}

MBOOL StreamingFeaturePipeUsage::supportDepth() const
{
    return supportDPE() || supportTOF();
}

MBOOL StreamingFeaturePipeUsage::supportDPE() const
{
    return SUPPORT_VSDOF &&
            ( (mDualMode & v1::Stereo::E_STEREO_FEATURE_VSDOF) ||
              (mDualMode & v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP) );
}

MBOOL StreamingFeaturePipeUsage::supportBokeh() const
{
    return SUPPORT_VSDOF &&
            (mDualMode & v1::Stereo::E_STEREO_FEATURE_VSDOF);
}

MBOOL StreamingFeaturePipeUsage::supportTOF() const
{
    //return mDualMode == v1::Stereo::E_STEREO_FEATURE_TOF;
    return MFALSE;
}

MBOOL StreamingFeaturePipeUsage::support3DNR() const
{
    MBOOL ret = MFALSE;
    if(!supportIMG3O())
    {
        return MFALSE;
    }
    // could use property to enalbe 3DNR
    // assign value at getPipeUsageHint
    if (this->is3DNRModeMaskEnable(NR3D::E3DNR_MODE_MASK_HAL_FORCE_SUPPORT) ||
        this->is3DNRModeMaskEnable(NR3D::E3DNR_MODE_MASK_UI_SUPPORT))
    {
        ret = this->supportP2AFeature();
    }
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

MBOOL StreamingFeaturePipeUsage::support3DNRLMV() const
{
    MBOOL ret = MFALSE;
    if (support3DNR() && !is3DNRModeMaskEnable(NR3D::E3DNR_MODE_MASK_SMVR_EN) )
    {
        ret = MTRUE;
    }
    return ret;
}

MBOOL StreamingFeaturePipeUsage::is3DNRModeMaskEnable(NR3D::E3DNR_MODE_MASK mask) const
{
    return (m3DNRMode & mask);
}

MBOOL StreamingFeaturePipeUsage::supportDSDN20() const
{
    return SUPPORT_VNR_DSDN && mUsageHint.mDSDNParam.hasDSDN20();
}

MBOOL StreamingFeaturePipeUsage::supportFSC() const
{
    return (mUsageHint.mFSCMode & NSCam::FSC::EFSC_MODE_MASK_FSC_EN);
}

MBOOL StreamingFeaturePipeUsage::supportVendorFSCFullImg() const
{
    return supportTPI(TPIOEntry::YUV) && supportFSC() && !supportEISNode();
}

MBOOL StreamingFeaturePipeUsage::supportEnlargeRsso() const
{
    return mUsageHint.mEnlargeRsso;
}

MBOOL StreamingFeaturePipeUsage::supportFull_NV21() const
{
    return USE_NV21_FULL_IMG && supportWPE() && !USE_WPE_STAND_ALONE &&
           NSCam::NSIoPipe::WPEQuerySupportFormat(NSCam::NSIoPipe::WPEFORMAT_420_2P_1ROUND_DL);
}

MBOOL StreamingFeaturePipeUsage::supportFull_YUY2() const
{
    return USE_YUY2_FULL_IMG && supportWPE();
}

MBOOL StreamingFeaturePipeUsage::supportGraphicBuffer() const
{
    return SUPPORT_GRAPHIC_BUFFER;
}

EImageFormat StreamingFeaturePipeUsage::getFullImgFormat() const
{
    return supportFull_NV21() ? eImgFmt_NV21:
           supportFull_YUY2() ? eImgFmt_YUY2:
                                eImgFmt_YV12;
}

EImageFormat StreamingFeaturePipeUsage::getVNRImgFormat() const
{
    return mDSDNCfg.getFormat(DSDNCfg::FULL);
}

std::vector<MUINT32> StreamingFeaturePipeUsage::getAllSensorIDs() const
{
    return mUsageHint.mAllSensorIDs;
}

MUINT32 StreamingFeaturePipeUsage::getMode() const
{
    return mUsageHint.mMode;
}

MUINT32 StreamingFeaturePipeUsage::getEISMode() const
{
    return mUsageHint.mEISInfo.mode;
}

NSPipelinePlugin::TPICustomConfig StreamingFeaturePipeUsage::getTPICustomConfig() const
{
    return mUsageHint.mTPICustomConfig;
}

MUINT32 StreamingFeaturePipeUsage::getEISFactor() const
{
    return mUsageHint.mEISInfo.factor ? mUsageHint.mEISInfo.factor : 100;
}

DSDNCfg StreamingFeaturePipeUsage::getDSDNCfg() const
{
    return mDSDNCfg;
}

MUINT32 StreamingFeaturePipeUsage::getSensorModule() const
{
    return mUsageHint.mSensorModule;
}

MUINT32 StreamingFeaturePipeUsage::getDualMode() const
{
    return mDualMode;
}

MUINT32 StreamingFeaturePipeUsage::get3DNRMode() const
{
    return m3DNRMode;
}

MUINT32 StreamingFeaturePipeUsage::getFSCMode() const
{
    return mFSCMode;
}

MUINT32 StreamingFeaturePipeUsage::getSMVRSpeed() const
{
    return mUsageHint.mSMVRSpeed;
}

TP_MASK_T StreamingFeaturePipeUsage::getTPMask() const
{
    return mUsageHint.mTP;
}

MFLOAT StreamingFeaturePipeUsage::getTPMarginRatio() const
{
    return mUsageHint.mTPMarginRatio;
}

IMetadata StreamingFeaturePipeUsage::getAppSessionMeta() const
{
    return mUsageHint.mAppSessionMeta;
}

MSize StreamingFeaturePipeUsage::getStreamingSize() const
{
    return mStreamingSize;
}

MSize StreamingFeaturePipeUsage::getRrzoSizeByIndex(MUINT32 sID)
{
    MSize result= {0, 0};
    if (0 == mResizedRawSizeMap.count(sID)) {
        MY_LOGE("sensor ID(%d) MapCount(%zu)", sID, mResizedRawSizeMap.count(sID));
        return result;
    }
    result = mResizedRawSizeMap[sID];
    return result;
}

MSize StreamingFeaturePipeUsage::getMaxOutSize() const
{
    return mMaxOutArea;
}

MUINT32 StreamingFeaturePipeUsage::getNumSensor() const
{
    return mNumSensor;
}

MUINT32 StreamingFeaturePipeUsage::getNumP2ABuffer() const
{
    MUINT32 num = ((mOutCfg.mMaxOutNum > 2) || mForceIMG3O) ? 3 : 0; // need full for additional MDP run ( no need consider physical & large)
    num = max(num, get3DNRBufferNum().mBasic);
    num = max(num, getVendorBufferNum().mBasic);
    return num;
}

MUINT32 StreamingFeaturePipeUsage::getNumP2APureBuffer() const
{
    MUINT32 num = max((MUINT32)3, getVendorBufferNum().mBasic);
    return num;
}

MUINT32 StreamingFeaturePipeUsage::getNumDCESOBuffer() const
{
    return getNumSensor() * DCESO_DELAY_COUNT * P2A_TOTAL_PATH_NUM;
}

MUINT32 StreamingFeaturePipeUsage::getNumDepthImgBuffer() const
{
    // TODO consider DepthPipe buffer depth
    return 3;
}

MUINT32 StreamingFeaturePipeUsage::getNumBokehOutBuffer() const
{
    // TODO need confirm by VSDOF owner
    return 3;
}

MUINT32 StreamingFeaturePipeUsage::getNumP2ATuning() const
{
    MUINT32 num = MIN_P2A_TUNING_BUF_NUM;
    if(mOutCfg.mHasPhysical)
    {
        num *= 2;
    }
    if(supportP2ALarge())
    {
        num *= 2;
    }
    num = max(num, getNumP2ABuffer());
    if( supportDual() )
    {
        num *= 2;
    }

    return num;
}

MUINT32 StreamingFeaturePipeUsage::getNumP2ASyncTuning() const
{
    MUINT32 num = MIN_P2A_TUNING_BUF_NUM;
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

MUINT32 StreamingFeaturePipeUsage::getNumDSDNBuffer() const
{
    return supportDSDN20() ? 4 : 0;
}

MVOID StreamingFeaturePipeUsage::updateOutSize(const std::vector<MSize> &out)
{
    mOutSizeVector = out;
    std::sort(mOutSizeVector.begin(), mOutSizeVector.end(),
              [](const MSize &lhs, const MSize &rhs)
                { return lhs.w*lhs.h > rhs.w*rhs.h; });

    for( const MSize &size : mOutSizeVector )
    {
        if( size.w > mStreamingSize.w || size.h > mStreamingSize.h )
        {
            MY_LOGI("large out size detected(%dx%d)", size.w, size.h);
            continue;
        }
        if( size.w > mMaxOutArea.w )
        {
            mMaxOutArea.w = size.w;
        }
        if( size.h > mMaxOutArea.h )
        {
            mMaxOutArea.h = size.h;
        }
    }

    if( supportVendorDebug() )
    {
        for( const MSize &s : out )
        {
            MY_LOGI("Out size=(%dx%d)", s.w, s.h);
        }
        for( const MSize &s : mOutSizeVector )
        {
            MY_LOGI("Out size=(%dx%d)", s.w, s.h);
        }
        MY_LOGI("mMaxOutArea=(%dx%d)", mMaxOutArea.w, mMaxOutArea.h);
    }
}

StreamingFeaturePipeUsage::BufferNumInfo StreamingFeaturePipeUsage::get3DNRBufferNum() const
{
    MUINT32 num = 0;
    if( support3DNR() )
    {
        num = 3;
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
    return supportTPI(TPIOEntry::YUV) ? BufferNumInfo(3+1) : BufferNumInfo(0);
}

MUINT32 StreamingFeaturePipeUsage::getSensorIndex() const
{
    return mSensorIndex;
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

TPIUsage StreamingFeaturePipeUsage::getTPIUsage() const
{
    return mTPIUsage;
}

MUINT32 StreamingFeaturePipeUsage::getTPINodeCount(TPIOEntry entry) const
{
    return mTPIUsage.getNodeCount(entry);
}

MBOOL StreamingFeaturePipeUsage::supportTPI(TPIOEntry entry) const
{
    return mTPIUsage.useEntry(entry);
}

MBOOL StreamingFeaturePipeUsage::supportVendorDebug() const
{
    return mVendorDebug;
}

MBOOL StreamingFeaturePipeUsage::supportVendorLog() const
{
    return mVendorLog;
}

MBOOL StreamingFeaturePipeUsage::supportVendorErase() const
{
    return mVendorErase;
}

MBOOL StreamingFeaturePipeUsage::supportSharedInput() const
{
    return mSupportSharedInput;
}

MBOOL StreamingFeaturePipeUsage::supportVMDPNode() const
{
    return supportTPI(TPIOEntry::YUV) ||
           ( supportDSDN20() && ( !supportWarpNode() || !supportPreviewEIS()) );
}

MBOOL StreamingFeaturePipeUsage::isSecureP2() const
{
    return (mSecType != NSCam::SecType::mem_normal);
}

MBOOL StreamingFeaturePipeUsage::isForceP2Zoom() const
{
    return FORCE_ZOOM_BEFORE_TPI;
}


NSCam::SecType StreamingFeaturePipeUsage::getSecureType() const
{
    return mSecType;
}

} // NSFeaturePipe
} // NSCamFeature
} // NSCam
