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
#define PIPE_CLASS_TAG "TuningHelper"
#define PIPE_MODULE_TAG "TuningHelper"
//#define PIPE_TRACE TRACE_TUNING_HELPER
#include <featurePipe/core/include/PipeLog.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam3/feature/3dnr/util_3dnr.h>
#include <mtkcam3/feature/utils/p2/P2Util.h>
#include <mtkcam3/feature/utils/p2/P2Trace.h>

#include "P2CamContext.h"
#include "TuningHelper.h"
#include <isp_tuning/isp_tuning.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

using NS3Av3::MetaSet_T;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

using namespace Feature;
using namespace Feature::P2Util;


MBOOL checkInput(const TuningHelper::Input &in, TuningHelper::MetaParam &metaParam)
{
    if( (in.mpISP == NULL && SUPPORT_ISP_HAL) || in.mTuningBuf == NULL)
    {
        MY_LOGE("Process Tuning fail! pISP(%p), tuningBuf(%p)",
                in.mpISP, in.mTuningBuf.get());
        return MFALSE;
    }

    if( in.mIOPack.mIMGI.mBuffer == NULL )
    {
        MY_LOGE("Process Tuning fail! Not support imgi is NULL");
        return MFALSE;
    }

    if(metaParam.mHalIn == NULL || metaParam.mAppIn == NULL )
    {
        MY_LOGE("Can not get Input Metadata from metaParam! halI/O(%p/%p), appI/O/ExtraO(%p/%p/%p)",
                metaParam.mHalIn, metaParam.mHalOut, metaParam.mAppIn, metaParam.mAppOut, metaParam.mExtraAppOut);
        return MFALSE;
    }

    return MTRUE;
}

MVOID
updateInputMeta_Raw2Yuv(
                      TuningHelper::MetaParam&  metaParam,
                      NS3Av3::MetaSet_T&        inMetaSet)
{
    if(metaParam.mScene == TuningHelper::Tuning_Normal)
    {
        sp<P2CamContext> p2CamContext = getP2CamContext(metaParam.mSensorID);

        if (p2CamContext != NULL && p2CamContext->get3dnr() != NULL &&
                metaParam.mpNr3dTuningInfo != NULL) // Run only if 3DNR is enabled
        {
            p2CamContext->get3dnr()->updateISPMetadata(&inMetaSet.halMeta,
                *(metaParam.mpNr3dTuningInfo));
        }
    }

}

MVOID
updateInputMeta_Yuv2Yuv(
                      TuningHelper::MetaParam   &metaParam,
                      NS3Av3::MetaSet_T         &inMetaSet,
                      Feature::P2Util::P2ObjPtr &p2ObjPtr,
                const MSize                     &imgiSize)
{
    if(metaParam.mScene == TuningHelper::Tuning_P2NR)
    {
        // 3DNR: ref: updateInputMeta_Raw2Yuv(..)
        sp<P2CamContext> p2CamContext = getP2CamContext(metaParam.mSensorID);
        if (p2CamContext != NULL && p2CamContext->get3dnr() != NULL && metaParam.mpNr3dTuningInfo != NULL) // Run only if 3DNR is enabled
        {
            p2CamContext->get3dnr()->updateISPMetadata(&inMetaSet.halMeta, *(metaParam.mpNr3dTuningInfo));
        }
        else
        {
        }

        MUINT8 prof = (MUINT8)NSIspTuning::EIspProfile_Auto_mHDR_Video;
        IMetadata::setEntry<MUINT8>(&inMetaSet.halMeta, MTK_3A_ISP_PROFILE, prof);
        IMetadata::setEntry<MINT32>(&inMetaSet.halMeta, MTK_ISP_P2_IN_IMG_FMT, 1);
        MINT32 resolution = imgiSize.w | imgiSize.h << 16;
        IMetadata::setEntry<MINT32>(&inMetaSet.halMeta, MTK_ISP_P2_IN_IMG_RES_REVISED, resolution);
        IMetadata::setEntry<MUINT8>(&inMetaSet.halMeta, MTK_ISP_P2_TUNING_UPDATE_MODE, 9); // TODO wait isp tuning define

        p2ObjPtr.profile = (MINT32)prof;
    }

}

MVOID
updateOutputMeta_Raw2Yuv( TuningHelper::MetaParam& metaParam)
{
    if(metaParam.mHalOut != NULL)
    {
        if(metaParam.mIsFDCropValid)
        {
            IMetadata::setEntry<MRect>(metaParam.mHalOut, MTK_P2NODE_FD_CROP_REGION, metaParam.mFdCrop);
        }
    }

}

MVOID copyInMeta(const TuningHelper::Config &cfg, MetaSet_T &inMetaSet)
{
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TH(p2a.r2y)::copyInMeta");
    inMetaSet.appMeta = *cfg.mAppIn;
    inMetaSet.halMeta = *cfg.mHalIn;
    P2_CAM_TRACE_END(TRACE_ADVANCED);
}

MVOID updateInputMeta(const TuningHelper::Config &cfg, MetaSet_T &inMetaSet)
{
    (void)inMetaSet;
    if( cfg.mUsage == TuningHelper::Tuning_3DNR )
    {
        // sp<P2CamContext> p2CamContext = getP2CamContext(in.mSensorID);
        // p2CamContext->get3dnr()->updateISPMetadata(&inMetaSet.halMeta, *in.mpNr3dTuningInfo);
    }
    else if( cfg.mUsage == TuningHelper::Tuning_SMVR )
    {
    }
}

NS3Av3::TuningParam callSetISP(const P2Pack &p2Pack, const TuningHelper::Config &cfg, MetaSet_T &inMetaSet, MetaSet_T &outMetaSet)
{
    NS3Av3::TuningParam tuningParam;
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TH(p2a.r2y)::makeTuningParam");
    tuningParam = P2Util::makeTuningParam(p2Pack.mLog, p2Pack, cfg.mHalISP, inMetaSet, &outMetaSet, cfg.mResized, cfg.mTuningBuf, cfg.mLCSO, cfg.mDCEMagic, cfg.mDCESO, MFALSE, NULL, cfg.mLCSHO);
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    return tuningParam;
}

MVOID copyOutMeta(const TuningHelper::Config &cfg, const MetaSet_T &inMetaSet, const MetaSet_T &outMetaSet)
{
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TH(p2a.r2y)::copyOutMeta");
    if( cfg.mAppOut != NULL )
    {
        *cfg.mAppOut = outMetaSet.appMeta;
    }
    if( cfg.mHalOut != NULL )
    {
        *cfg.mHalOut = inMetaSet.halMeta;
        *cfg.mHalOut += outMetaSet.halMeta;
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
}

MVOID updateOutMeta(const TuningHelper::Config &cfg)
{
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TH(p2a.r2y)::updateOutputMeta");
    if( cfg.mHalOut != NULL )
    {
        if( cfg.mUseFDCrop )
        {
            IMetadata::setEntry<MRect>(cfg.mHalOut, MTK_P2NODE_FD_CROP_REGION, cfg.mFDCrop);
        }
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
}

MBOOL TuningHelper::makeTuningParam_SMVR(const P2Pack &p2Pack, const Config &cfg, NS3Av3::TuningParam &tuningParam)
{
    MBOOL ret = MFALSE;

    if( (cfg.mHalISP == NULL && SUPPORT_ISP_HAL) ||
        cfg.mTuningBuf == NULL )
    {
        MY_LOGE("Invalid tuning: HalISP(%p), tuningBuf(%p)",
                cfg.mHalISP, cfg.mTuningBuf);
    }
    else if( cfg.mHalIn == NULL || cfg.mAppIn == NULL )
    {
        MY_LOGE("Invalid tuning: appI/O(%p/%p), halI/O(%p/%p)",
                cfg.mAppIn, cfg.mAppOut, cfg.mHalIn, cfg.mHalOut);
    }
    else
    {
        if( cfg.mAppOut == NULL || cfg.mHalOut == NULL)
        {
            MY_LOGW("Missing output metdata: appO(%p), halO(%p)",
                    cfg.mAppOut, cfg.mHalOut);
        }

        {
            NS3Av3::MetaSet_T inMetaSet;
            NS3Av3::MetaSet_T outMetaSet;

            copyInMeta(cfg, inMetaSet);
            updateInputMeta(cfg, inMetaSet);
            tuningParam = callSetISP(p2Pack, cfg, inMetaSet, outMetaSet);
            copyOutMeta(cfg, inMetaSet, outMetaSet);
            updateOutMeta(cfg);

            P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TH(p2a.r2y)::freeMetaSet");
        }
        P2_CAM_TRACE_END(TRACE_ADVANCED);
        ret = MTRUE;
    }
    return ret;
}

MBOOL
TuningHelper::
processIsp_P2A_Raw2Yuv(
                const Input&                            in,
                      Feature::P2Util::DIPFrameParams&  frameParam,
                      MetaParam&                        metaParam)
{
    if( !checkInput(in, metaParam) )
    {
        return MFALSE;
    }

    {
        P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TH(p2a.r2y)::copyInMeta");
        NS3Av3::MetaSet_T inMetaSet;
        NS3Av3::MetaSet_T outMetaSet;

        inMetaSet.appMeta = *(metaParam.mAppIn); // copy
        inMetaSet.halMeta = *(metaParam.mHalIn); // copy
        P2_CAM_TRACE_END(TRACE_ADVANCED);

        updateInputMeta_Raw2Yuv(metaParam, inMetaSet);

        // ---  set Isp ---
        P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TH(p2a.r2y)::makeFrameParam");
        void *syncTuning = (in.mSyncTuningBuf != NULL) ? in.mSyncTuningBuf->mpVA : NULL;
        NS3Av3::TuningParam tuningParam =
            P2Util::makeTuningParam(in.mP2Pack.mLog, in.mP2Pack, in.mpISP, inMetaSet, &outMetaSet,
                    in.mIOPack.isResized(), in.mTuningBuf->mpVA, in.mIOPack.mLCSO.mBuffer, in.mDCESOMagicNum, in.mIOPack.mDCESO.mBuffer
                    , metaParam.isSlave(), syncTuning, in.mIOPack.mLCSHO.mBuffer);
        frameParam = P2Util::makeDIPFrameParams(in.mP2Pack, in.mTag, in.mIOPack, in.mP2ObjPtr, tuningParam, in.mNeedRegDump);
        metaParam.mDCESOEnqued = tuningParam.bDCES_Enalbe;
        P2_CAM_TRACE_END(TRACE_ADVANCED);

        P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TH(p2a.r2y)::copyOutMeta");
        if(metaParam.mAppOut != NULL)
            *(metaParam.mAppOut) = outMetaSet.appMeta; //copy
        if(metaParam.mExtraAppOut != NULL)
            *(metaParam.mExtraAppOut) = outMetaSet.appMeta; //copy
        if(metaParam.mHalOut != NULL)
        {
            *(metaParam.mHalOut) = inMetaSet.halMeta; //copy
            *(metaParam.mHalOut) += outMetaSet.halMeta; //copy
        }
        P2_CAM_TRACE_END(TRACE_ADVANCED);
        P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TH(p2a.r2y)::updateOutputMeta");
        updateOutputMeta_Raw2Yuv(metaParam);
        P2_CAM_TRACE_END(TRACE_ADVANCED);
        P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TH(p2a.r2y)::freeMetaSet");
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    return MTRUE;
}

MBOOL
TuningHelper::
processIsp_P2NR_Yuv2Yuv(
                const Input&                                    in,
                      NSCam::Feature::P2Util::DIPFrameParams&   frameParam,
                      MetaParam&                                metaParam)
{
    if( !checkInput(in, metaParam) )
    {
        return MFALSE;
    }

    {
        P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TH(p2a.r2y)::copyInMeta");
        NS3Av3::MetaSet_T inMetaSet;

        inMetaSet.appMeta = *(metaParam.mAppIn); // copy
        inMetaSet.halMeta = *(metaParam.mHalIn); // copy
        P2_CAM_TRACE_END(TRACE_ADVANCED);
        Feature::P2Util::P2ObjPtr p2ObjPtr = in.mP2ObjPtr;
        updateInputMeta_Yuv2Yuv(metaParam, inMetaSet, p2ObjPtr, in.mIOPack.mIMGI.getImgSize());

        // ---  set Isp ---
        P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TH(p2a.r2y)::makeFrameParam");
        NS3Av3::TuningParam tuningParam =
            P2Util::makeTuningParam(in.mP2Pack.mLog, in.mP2Pack, in.mpISP, inMetaSet, NULL,
                    in.mIOPack.isResized(), in.mTuningBuf->mpVA, in.mIOPack.mLCSO.mBuffer, in.mDCESOMagicNum, in.mIOPack.mDCESO.mBuffer
                    , metaParam.isSlave(), NULL, in.mIOPack.mLCSHO.mBuffer);
        frameParam = P2Util::makeDIPFrameParams(in.mP2Pack, in.mTag, in.mIOPack, p2ObjPtr, tuningParam, in.mNeedRegDump);
        metaParam.mDCESOEnqued = tuningParam.bDCES_Enalbe;
        P2_CAM_TRACE_END(TRACE_ADVANCED);

        P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TH(p2a.r2y)::freeMetaSet");
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    return MTRUE;
}

MBOOL
TuningHelper::
processSecure(
            Feature::P2Util::DIPFrameParams&   frameParam,
               Feature::SecureBufferControl&   secBufCtrl)
{
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "TH(p2a.ps)::processSecure");
    frameParam.mSecureFra = MFALSE;
    NSIoPipe::EDIPSecureEnum secureEnum = secBufCtrl.getSecureEnum();
    if (secureEnum != NSIoPipe::EDIPSecureEnum::EDIPSecure_NONE)
    {
        frameParam.mSecureFra = MTRUE;
        for( auto &in : frameParam.mvIn )
        {
            NSIoPipe::EDIPSecDMAType secDMAType = NSIoPipe::NSPostProc::INormalStream::needSecureBuffer(in.mPortID.index);
            MY_LOGD_IF(secBufCtrl.mLogLevel, "FrameParam In PortID(%u) secDMAType(%u)", in.mPortID.index, secDMAType);
            if(secDMAType == NSIoPipe::EDIPSecDMAType::EDIPSecDMAType_TUNE)
            {
                in.mSecureTag = NSIoPipe::EDIPSecureEnum::EDIPSecure_NONE;
            }
            else if(secDMAType == NSIoPipe::EDIPSecDMAType::EDIPSecDMAType_TUNE_SHARED && in.mBuffer != NULL)
            {
                MVOID *secHandle = secBufCtrl.registerAndGetSecHandle(in.mBuffer);
                if ( secHandle != NULL )
                {
                    MY_LOGD_IF(secBufCtrl.mLogLevel, "Alloc secure buffer success, PortID(%u) secHandle(%p)", in.mPortID.index, secHandle);
                    in.mSecureTag = secureEnum;
                    in.mSecHandle = (MUINTPTR)secHandle;
                }
                else
                {
                    MY_LOGE("Alloc secure buffer fail, PortID(%u)", in.mPortID.index);
                    in.mSecureTag = NSIoPipe::EDIPSecureEnum::EDIPSecure_NONE;
                    return MFALSE;
                }
            }
            else if(secDMAType == NSIoPipe::EDIPSecDMAType::EDIPSecDMAType_IMAGE)
            {
                in.mSecureTag = secureEnum;
            }
            else
            {
                in.mSecureTag = NSIoPipe::EDIPSecureEnum::EDIPSecure_NONE;
            }
        }
        for( auto &out : frameParam.mvOut )
        {
            NSIoPipe::EDIPSecDMAType secDMAType = NSIoPipe::NSPostProc::INormalStream::needSecureBuffer(out.mPortID.index);
            MY_LOGD_IF(secBufCtrl.mLogLevel, "FrameParam Out PortID(%u) secDMAType(%u)", out.mPortID.index, secDMAType);
            if(secDMAType == NSIoPipe::EDIPSecDMAType::EDIPSecDMAType_TUNE)
            {
                out.mSecureTag = NSIoPipe::EDIPSecureEnum::EDIPSecure_NONE;
            }
            else if(secDMAType == NSIoPipe::EDIPSecDMAType::EDIPSecDMAType_IMAGE)
            {
                out.mSecureTag = secureEnum;
            }
            else
            {
                out.mSecureTag = NSIoPipe::EDIPSecureEnum::EDIPSecure_NONE;
            }
        }
    }

    P2_CAM_TRACE_END(TRACE_ADVANCED);
    return MTRUE;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
