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
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFdTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_TUNING_HELPER_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_TUNING_HELPER_H_

#include <utils/RefBase.h>
#include <mtkcam/def/BuiltinTypes.h>
#include <mtkcam/aaa/IHalISP.h>
#include <mtkcam/drv/def/IPostProcDef.h>
#include <mtkcam3/feature/featurePipe/util/VarMap.h>
#include <mtkcam3/feature/3dnr/util_3dnr.h>
#include <mtkcam3/feature/3dnr/3dnr_defs.h>

#include <featurePipe/core/include/TuningBufferPool.h>
#include <mtkcam3/feature/utils/p2/P2Pack.h>
#include <mtkcam3/feature/utils/p2/P2IO.h>
#include <mtkcam3/feature/utils/p2/P2Util.h>
#include <mtkcam3/feature/utils/SecureBufferControlUtils.h>
#include <mtkcam3/feature/utils/p2/DIPStream.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

#define MIN_P2A_TUNING_BUF_NUM (MUINT32)(4) // 3 for P2A driver depth, add 1 buffer

class TuningHelper : public RefBase
{
public:
    enum Scene
    {
        Tuning_Normal,
        Tuning_3DNR,
        Tuning_SMVR,
        Tuning_P2NR,
    };

    class Input
    {
    public:
        /*Cannot be NULL*/
        const Feature::P2Util::P2Pack    &mP2Pack;
        SmartTuningBuffer           &mTuningBuf;
        NS3Av3::IHalISP             *mpISP            = NULL;
        NSIoPipe::NSPostProc::ENormalStreamTag mTag  = NSIoPipe::NSPostProc::ENormalStreamTag_Normal;
        Feature::P2Util::P2ObjPtr   mP2ObjPtr;
        Feature::P2Util::P2IOPack   mIOPack;
        MINT32                      mDCESOMagicNum = -1;
        SmartTuningBuffer           mSyncTuningBuf;
        MBOOL                       mNeedRegDump = MFALSE;

        Input(const Feature::P2Util::P2Pack &p2Pack, SmartTuningBuffer &tuningBuf)
        : mP2Pack(p2Pack)
        , mTuningBuf(tuningBuf)
        {}

    };

    class MetaParam
    {
    public:
        NSCam::IMetadata*    mHalIn         = NULL;
        NSCam::IMetadata*    mAppIn         = NULL;
        NSCam::IMetadata*    mHalOut        = NULL;
        NSCam::IMetadata*    mAppOut        = NULL;
        NSCam::IMetadata*    mExtraAppOut   = NULL;
        Scene                mScene         = Tuning_Normal;
        MBOOL                mIsFDCropValid = MFALSE;
        MUINT32              mSensorID      = INVALID_SENSOR_ID;
        MUINT32              mMasterID      = INVALID_SENSOR_ID;
        MRect                mFdCrop;
        MBOOL                mDCESOEnqued   = MFALSE;
        // ---- 3DNR ----
        NR3D::NR3DTuningInfo *mpNr3dTuningInfo   = NULL; // defined in 3dnr_defs.h

        MBOOL isSlave() { return (mSensorID != mMasterID); }
    };

    class Config
    {
    public:
        Scene                           mUsage = Tuning_Normal;
        NS3Av3::IHalISP                 *mHalISP = NULL;
        NSIoPipe::NSPostProc::ENormalStreamTag mTag = NSIoPipe::NSPostProc::ENormalStreamTag_Normal;
        void                            *mTuningBuf = NULL;

        IMetadata                       *mAppIn = NULL;
        IMetadata                       *mAppOut = NULL;
        IMetadata                       *mHalIn = NULL;
        IMetadata                       *mHalOut = NULL;

        IImageBuffer                    *mLCSO = NULL;
        IImageBuffer                    *mLCSHO = NULL;
        IImageBuffer                    *mDCESO = NULL;

        MBOOL                           mResized = MFALSE;
        MBOOL                           mUseFDCrop = MFALSE;
        MRect                           mFDCrop;
        MINT32                          mDCEMagic = -1;
    };

    /* In Raw2Yuv, need update App/Hal out Meta */
    static MBOOL processIsp_P2A_Raw2Yuv(
                                 const Input&                   in,
                 Feature::P2Util::DIPFrameParams&               frameParam,
                                       MetaParam&               metaParam);

    static MBOOL processIsp_P2NR_Yuv2Yuv(
                                 const Input&                   in,
                 Feature::P2Util::DIPFrameParams&               frameParam,
                                       MetaParam&               metaParam);

    static MBOOL makeTuningParam_SMVR(const Feature::P2Util::P2Pack &p2Pack, const Config &cfg, TuningParam &tuningParam);

    static MBOOL processSecure(
                             Feature::P2Util::DIPFrameParams&   frameParam,
                                Feature::SecureBufferControl&   secBufCtrl);
protected:

private:
    TuningHelper();
    ~TuningHelper();

};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_TUNING_HELPER_H_
