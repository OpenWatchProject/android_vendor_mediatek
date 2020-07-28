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

#include <cutils/properties.h>
#include "P2_StreamingProcessor.h"

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    Streaming_DSDN
#define P2_TRACE        TRACE_STREAMING_DSDN
#include "P2_LogHeader.h"

#include <camera_custom_dsdn.h>
#include <mtkcam3/feature/stereo/StereoCamEnum.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_STR_PROC);

    // -1: auto, 0: off 1: on
#define KEY_P2S_DSDN_MODE           "vendor.debug.p2s.dsdn.mode"
#define KEY_P2S_DSDN_THRESHOLD      "vendor.debug.p2s.dsdn.threshold"
#define ISO_OFFSET 800

using NSCam::NSCamFeature::NSFeaturePipe::MASK_DSDN20;

namespace P2
{

DSDNCustom::DUAL_MODE parseDSDNDual(MUINT32 dualMode)
{
    DSDNCustom::DUAL_MODE mode = DSDNCustom::DUAL_MODE_SINGLE;
    if( (dualMode & NSCam::v1::Stereo::E_STEREO_FEATURE_VSDOF)
        || (dualMode & NSCam::v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP) )
    {
        mode = DSDNCustom::DUAL_MODE_HW_DEPTH;
    }
    else if( dualMode != 0 )
    {
        mode = DSDNCustom::DUAL_MODE_OTHER;
    }
    return mode;
}

IStreamingFeaturePipe::DSDNParam StreamingProcessor::queryDSDNParam(const P2ConfigInfo &config)
{
    TRACE_FUNC_ENTER();
    DSDNCustom::ScenarioParam sceneParam;
    sceneParam.videoW = config.mUsageHint.mOutCfg.mVideoSize.w;
    sceneParam.videoH = config.mUsageHint.mOutCfg.mVideoSize.h;
    sceneParam.smvrFps = (config.mP2Type == P2_BATCH_SMVR) ? 30 * config.mUsageHint.mSMVRSpeed : 0;
    sceneParam.fps = sceneParam.smvrFps ? sceneParam.smvrFps :30; // TODO bring real fps from pipeline
    sceneParam.dualMode = parseDSDNDual(config.mUsageHint.mDualMode);
    DSDNCustom::Config dsdnConfig = DSDNCustom::getInstance()->getConfig(sceneParam);

    if(config.mUsageHint.mDsdnHint == 0) // Pipeline force off
    {
        dsdnConfig.mode = DSDNCustom::DSDN_MODE_OFF;
    }


    MINT32 debugMode = property_get_int32(KEY_P2S_DSDN_MODE, -1);
    mDSDNDebugISO_H = property_get_int32(KEY_P2S_DSDN_THRESHOLD, 0);

    IStreamingFeaturePipe::DSDNParam out;
    out.mMode = (debugMode >= 0) ? debugMode : (MUINT32)dsdnConfig.mode;
    out.mMaxRatioMultiple = std::max<MUINT32>(dsdnConfig.maxRatioMultiple, 1);
    out.mMaxRatioDivider  = std::max<MUINT32>(dsdnConfig.maxRatioDivider, 1);

    TRACE_FUNC_EXIT();
    return out;
}

MBOOL StreamingProcessor::prepareDSDN(P2Util::SimpleIn &input, const ILog &log)
{
    TRACE_S_FUNC_ENTER(log);
    MBOOL run = MFALSE;
    if( mPipeUsageHint.mDSDNParam.hasDSDN20() )
    {
        const P2PlatInfo::NVRamDSDN &nvramData = input.mRequest->mP2Pack.getSensorData().mNvramDsdn;
        MINT32 isoH = (mDSDNDebugISO_H > 0) ? mDSDNDebugISO_H : nvramData.mIsoThreshold;
        MINT32 isoL = (isoH > ISO_OFFSET) ? isoH - ISO_OFFSET : isoH;
        MINT iso = input.mRequest->mP2Pack.getSensorData().mISO;
        run = (isoH != 0) && ( (iso >= isoH) || (mDSDNState && iso >= isoL) );
        TRACE_S_FUNC(log, "DSDN run=%d iso=%d(%d:%d), nvram threshold(%d), debugThreshold(%d), ratio(m/d)=(%d,%d), initRatio(m/d)=(%d,%d)",
                            run, iso, isoL, isoH, nvramData.mIsoThreshold, mDSDNDebugISO_H, nvramData.mRatioMultiple, nvramData.mRatioDivider,
                            mPipeUsageHint.mDSDNParam.mMaxRatioMultiple, mPipeUsageHint.mDSDNParam.mMaxRatioDivider);
        input.mFeatureParam.setFeatureMask(MASK_DSDN20, run);
        mDSDNState = run;
    }
    TRACE_S_FUNC_EXIT(log);
    return run;
}

} // namespace P2
