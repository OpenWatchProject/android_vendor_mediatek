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

/**
* @file DualFeatureEffectHal.h
* @brief EffectHal of Dualcam feature
*/
#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_DUAL_FEATURE_EFFECT_HAL_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_DUAL_FEATURE_EFFECT_HAL_H_
// Standard C header file
// Android system/core header file
#include <utils/Mutex.h>
#include <utils/KeyedVector.h>
// mtkcam custom header file
// mtkcam global header file
#include <mtkcam/feature/effectHalBase/EffectHalBase.h>
#include <mtkcam/feature/effectHalBase/EffectRequest.h>
#include <mtkcam/feature/stereo/pipe/IBokehPipe.h>
#include <mtkcam/feature/stereo/pipe/IDepthMapPipe.h>
#include <mtkcam/feature/stereo/pipe/IDepthMapEffectRequest.h>
#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe.h>
#include <mtkcam/feature/common/vsdof/common/DualFeatureRequest.h>
//
#include <mtkcam/feature/stereo/effecthal/FeatureParamToDualFeatureRequestHelper.h>
//
// Module header file

// Local header file
/*******************************************************************************
* Macro Define
********************************************************************************/

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature{
namespace NSDualFeature {
using namespace v1::Stereo;
using NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam;
using android::sp;
using android::KeyedVector;
using NSFeaturePipe_DepthMap::IDepthMapPipe;
using NSFeaturePipe::IBokehPipe;

class ILinkedPipeItem;
/*******************************************************************************
* EnumDefine
********************************************************************************/
enum DualFeatureReqBufferID
{
    RBID_IN_FULLRAW_MAIN1,
    RBID_IN_FULLRAW_MAIN2,
    RBID_IN_RESIZERAW_MAIN1,
    RBID_IN_RESIZERAW_MAIN2,
    RBID_IN_LCSO_MAIN1,
    RBID_IN_LCSO_MAIN2,

    RBID_IN_APP_META_MAIN1,
    RBID_IN_APP_META_MAIN2,
    RBID_IN_HAL_META_MAIN1,
    RBID_IN_HAL_META_MAIN2,
    RBID_IN_P1_RETURN_META,

    RBID_OUT_MAINIMAGE,
    RBID_OUT_BLUR_MAP,
    RBID_OUT_PREVIEW,
    RBID_OUT_PREVIEW_CALLBACK,
    RBID_OUT_FD,
    RBID_OUT_DEPTHMAP,

    RBID_OUT_APP_META,
    RBID_OUT_HAL_META
};

/*******************************************************************************
* Struct Define
********************************************************************************/
struct DualFeatureSettings
{
    MUINT32 miSensorIdx_Main1 = -1;
    MUINT32 miSensorIdx_Main2 = -1;
    StereoFeatureMode mMode = E_STEREO_FEATURE_VSDOF;
    SeneorModuleType mSensorType = v1::Stereo::BAYER_AND_BAYER;
    MSize mszRRZO_Main1;
};

/*******************************************************************************
* Function Define
********************************************************************************/
const char* RBID2Name(DualFeatureReqBufferID bufferID);
/*******************************************************************************
* Class Define
********************************************************************************/

/**
 * @class DualFeatureEffectHal
 * @brief EffectHal class for Dualcam feature
 */
class DualFeatureEffectHal : public EffectHalBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    DualFeatureEffectHal() = delete;
    DualFeatureEffectHal(const DualFeatureSettings& settings);
    virtual ~DualFeatureEffectHal();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  EffectHalBase Protected Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    virtual android::status_t   initImpl();
    virtual android::status_t   uninitImpl();
    virtual android::status_t   prepareImpl();
    virtual android::status_t   releaseImpl();
    virtual android::status_t   getNameVersionImpl(EffectHalVersion &nameVersion) const;
    virtual android::status_t   setParameterImpl(android::String8 &key, android::String8 &object);
    virtual android::status_t   setParametersImpl(sp<EffectParameter> parameter);
    virtual android::status_t   startImpl(uint64_t *uid=NULL);
    virtual android::status_t   abortImpl(EffectResult &result, EffectParameter const *parameter=NULL);
    virtual android::status_t   updateEffectRequestImpl(sp<EffectRequest> request);
    // no-use
    virtual bool allParameterConfigured();
    virtual android::status_t   getCaptureRequirementImpl(EffectParameter *inputParam, Vector<EffectCaptureRequirement> &requirements) const;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DualFeatureEffectHal Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MBOOL flush();
    MBOOL updateEffectRequest(sp<DualFeatureRequest> pRequest);
    MBOOL updateEffectRequest(const FeaturePipeParam& pipeParam);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DualFeatureEffectHal Private Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MBOOL buildLinkedPipe();
    MBOOL checkRequiredData(sp<DualFeatureRequest> pRequest);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static MVOID EffectHalCb(MVOID* tag, ResultState status, sp<IDualFeatureRequest>& request);
private:
    MVOID processRequestCallback(ResultState status, sp<IDualFeatureRequest>& request);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DualFeatureEffectHal Private Memeber
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    Mutex mOpLock;
    sp<ILinkedPipeItem> mpLinkedPipe;
    IDepthMapPipe* mpDepthMapPipe = nullptr;
    sp<IBokehPipe> mpBokehPipe = nullptr;
    MBOOL mbReady = MFALSE;
    DualFeatureSettings mSetting;
    MINT32 miLogLevel;
private:
    NSFeaturePipe::FeatureParamToDualFeatureRequestHelper converter;
};




};
};
};
 #endif

