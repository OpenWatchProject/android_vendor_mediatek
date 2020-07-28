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

#define LOG_TAG "DualFeatureEffectHal"

 // Standard C header file

// Android system/core header file
#include <cutils/properties.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/feature/stereo/effecthal/DualFeatureEffectHal.h>
#include <mtkcam/feature/common/vsdof/common/DualFeatureRequest.h>
// Module header file
// Local header file
#include "LinkedPipeItem.h"
/*******************************************************************************
* Global Define
********************************************************************************/

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)

#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")

#define LOG_TAG "DualFeatureEffectHal"
#define FEATURE_PARAMS_TAG "dualcam.feature.params.bokeh"

using namespace android;

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature{
namespace NSDualFeature {

const char* RBID2Name(DualFeatureReqBufferID bufferID)
{
#define MAKE_NAME_CASE(name) \
  case name: return #name;

  switch(bufferID)
  {
    MAKE_NAME_CASE(RBID_IN_FULLRAW_MAIN1);
    MAKE_NAME_CASE(RBID_IN_FULLRAW_MAIN2);
    MAKE_NAME_CASE(RBID_IN_RESIZERAW_MAIN1);
    MAKE_NAME_CASE(RBID_IN_RESIZERAW_MAIN2);
    MAKE_NAME_CASE(RBID_IN_LCSO_MAIN1);
    MAKE_NAME_CASE(RBID_IN_LCSO_MAIN2);

    MAKE_NAME_CASE(RBID_IN_APP_META_MAIN1);
    MAKE_NAME_CASE(RBID_IN_APP_META_MAIN2);
    MAKE_NAME_CASE(RBID_IN_HAL_META_MAIN1);
    MAKE_NAME_CASE(RBID_IN_HAL_META_MAIN2);

    MAKE_NAME_CASE(RBID_OUT_MAINIMAGE);
    MAKE_NAME_CASE(RBID_OUT_BLUR_MAP);
    MAKE_NAME_CASE(RBID_OUT_PREVIEW);
    MAKE_NAME_CASE(RBID_OUT_PREVIEW_CALLBACK);
    MAKE_NAME_CASE(RBID_OUT_FD);
    MAKE_NAME_CASE(RBID_OUT_DEPTHMAP);

    MAKE_NAME_CASE(RBID_OUT_APP_META);
    MAKE_NAME_CASE(RBID_OUT_HAL_META);
  };
  return "UNKNOWN";
#undef MAKE_NAME_CASE
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DualFeatureEffectHal Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
DualFeatureEffectHal::
DualFeatureEffectHal(const DualFeatureSettings& settings)
: mSetting(settings)
{
    miLogLevel = ::property_get_int32("vendor.debug.camera.log.dualHal", 0);
    MY_LOGD("miSensorIdx_Main1=%d miSensorIdx_Main2=%d mode=%d mSensorType=%d RRZO_Size=(%d, %d)",
            mSetting.miSensorIdx_Main1, mSetting.miSensorIdx_Main2, mSetting.mMode, mSetting.mSensorType,
            mSetting.mszRRZO_Main1);

}

DualFeatureEffectHal::
~DualFeatureEffectHal()
{
    MY_LOGD("DualFeatureEffectHal destr");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  EffectHalBase Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

status_t
DualFeatureEffectHal::
initImpl()
{
    Mutex::Autolock autoLock(mOpLock);
    if(mbReady)
        return OK;
    FUNC_START;
    // depth map pipe section
    sp<DepthMapPipeSetting> pPipeSetting = new DepthMapPipeSetting();
    pPipeSetting->miSensorIdx_Main1 = mSetting.miSensorIdx_Main1;
    pPipeSetting->miSensorIdx_Main2 = mSetting.miSensorIdx_Main2;
    pPipeSetting->mszRRZO_Main1 = mSetting.mszRRZO_Main1;
    // pipe option
    sp<DepthMapPipeOption> pPipeOption = new DepthMapPipeOption();
    pPipeOption->mSensorType = mSetting.mSensorType;
    // default VSDOF mode/queued-depth
    pPipeOption->mFeatureMode = (mSetting.mMode == (E_STEREO_FEATURE_VSDOF|E_STEREO_FEATURE_CAPTURE))
                            ? eDEPTHNODE_MODE_VSDOF : eDEPTHNODE_MODE_MTK_UNPROCESS_DEPTH;
    pPipeOption->mFlowType = eDEPTH_FLOW_TYPE_QUEUED_DEPTH;
    // no depth control
    pPipeOption->setEnableDepthGenControl(MFALSE, 0);
    // create depth pipe
    mpDepthMapPipe = IDepthMapPipe::createInstance(pPipeSetting, pPipeOption);
    MBOOL bRet = mpDepthMapPipe->init();
    // bokeh pipe creation
    BokehPipeMode mode = (mSetting.mMode == (E_STEREO_FEATURE_VSDOF|E_STEREO_FEATURE_CAPTURE))
                            ? HW_BOKEH : VENDOR_BOKEH;
    mpBokehPipe = IBokehPipe::createInstance(mSetting.miSensorIdx_Main1, mode);
    bRet &= mpBokehPipe->init();
    //
    bRet &= this->buildLinkedPipe();
    if(!bRet)
    {
        MY_LOGE("Failed to init DualFeatureEffectHal!");
        mbReady = MFALSE;
        return BAD_VALUE;
    }
    mbReady = MTRUE;
    FUNC_END;
    return OK;
}

status_t
DualFeatureEffectHal::
uninitImpl()
{
    FUNC_START;
    Mutex::Autolock autoLock(mOpLock);
    if (mpDepthMapPipe != nullptr)
    {
        mpDepthMapPipe->uninit();
        delete mpDepthMapPipe;
        mpDepthMapPipe = nullptr;
    }
    // bokeh is sp
    if(mpBokehPipe != nullptr)
    {
        mpBokehPipe->uninit();
        mpBokehPipe = nullptr;
    }
    mbReady = MFALSE;
    // manually release all pipe item
    while(mpLinkedPipe.get() != nullptr)
    {
        auto nextItem = mpLinkedPipe->mpNext;
        mpLinkedPipe->mpNext = nullptr;
        mpLinkedPipe->mpHead = nullptr;
        mpLinkedPipe = nextItem;
    }
    FUNC_END;
    return OK;
}

status_t
DualFeatureEffectHal::
prepareImpl()
{
    return OK;
}

status_t
DualFeatureEffectHal::
releaseImpl()
{
    return OK;
}

status_t
DualFeatureEffectHal::
getNameVersionImpl(EffectHalVersion &nameVersion) const
{
    return OK;
}

status_t
DualFeatureEffectHal::
setParameterImpl(String8 &key, String8 &object)
{
    return OK;
}

status_t
DualFeatureEffectHal::
setParametersImpl(sp<EffectParameter> parameter)
{
    return OK;
}

status_t
DualFeatureEffectHal::
startImpl(uint64_t *uid)
{
    mpDepthMapPipe->sync();
    mpBokehPipe->sync();
    return OK;
}

status_t
DualFeatureEffectHal::
abortImpl(EffectResult &result, EffectParameter const *parameter)
{
    return OK;
}

status_t
DualFeatureEffectHal::
updateEffectRequestImpl(const sp<EffectRequest> request)
{
    MY_LOGE("Not implemented.");
    return BAD_VALUE;
}

bool
DualFeatureEffectHal::
allParameterConfigured()
{
    if(!mbReady)
        MY_LOGE("Has not finished the init operation!");
    return mbReady;
}

status_t
DualFeatureEffectHal::
getCaptureRequirementImpl(EffectParameter *inputParam, Vector<EffectCaptureRequirement> &requirements) const
{
    return OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DualFeatureEffectHal Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
status_t
DualFeatureEffectHal::
updateEffectRequest(sp<DualFeatureRequest> pRequest)
{
    Mutex::Autolock autoLock(mOpLock);
    if(!allParameterConfigured())
        return BAD_VALUE;

    if(!mpLinkedPipe->enque(pRequest))
    {
        MY_LOGE("[DualFeatureHal] Critical error, failed to enque request, reqID=%d",
                pRequest->getRequestNo());
        return BAD_VALUE;
    }
    return OK;
}

status_t
DualFeatureEffectHal::
updateEffectRequest(const FeaturePipeParam& featureParam)
{
    status_t ret = UNKNOWN_ERROR;
    // get main1 process_frame
    std::shared_ptr<NSCam::v3::DualFeature::process_frame> pFrame_main1 = nullptr;
    typedef NSCam::v3::DualFeature::process_frame DUAL_PROCESS_FRAME;
    if(!featureParam.mVarMap.tryGet<std::shared_ptr<DUAL_PROCESS_FRAME> >(
                                    VAR_DUAL_FEATURE_PROCESS_DATA, pFrame_main1))
    {
        MY_LOGE("cannot get main1 process_frame.");
    }
    // get main2 process_frame
    FeaturePipeParam param_main2;
    std::shared_ptr<NSCam::v3::DualFeature::process_frame> pFrame_main2 = nullptr;
    if(!featureParam.mVarMap.tryGet<FeaturePipeParam>(VAR_DUALCAM_FOV_SLAVE_PARAM, param_main2) ||
        !param_main2.mVarMap.tryGet<std::shared_ptr<DUAL_PROCESS_FRAME> >(VAR_DUAL_FEATURE_PROCESS_DATA, pFrame_main2))
    {
        MY_LOGD("Cannot get main2 FeaturePipeParam/");
    }
    if(pFrame_main1 != nullptr)
    {
        MY_LOGD("frameNo:%d", pFrame_main1->getFrame()->getRequestNo());
    }
    if(pFrame_main2 != nullptr)
    {
        MY_LOGD("frameNo:%d", pFrame_main2->getFrame()->getRequestNo());
    }
    sp<DualFeatureRequest> pRequest = DualFeatureRequest::createInstance(
                                                                    pFrame_main1->mRequestId,
                                                                    EffectHalCb,
                                                                    nullptr
                                                                    );
    pRequest->startTimer();
    pRequest->mFeatureParam = featureParam;
    if(converter.convertToEffectRequest(pFrame_main1, pFrame_main2, pRequest) != OK)
    {
        MY_LOGE("Convert fail");
        goto lbExit;
    }
    if(updateEffectRequest(pRequest) != OK)
    {
        goto lbExit;
    }
    ret = OK;
lbExit:
    if(ret != OK)
    {
        FeaturePipeParam param = featureParam;
        param.mQParams.mDequeSuccess = MFALSE;
        if(param.mCallback != nullptr)
        {
            param.mCallback(FeaturePipeParam::MSG_FRAME_DONE, param);
        }
        else if(param.mCallback == nullptr)
        {
            MY_LOGE("cannot get combin FeaturePipeParam, should not happened.");
        }
    }
    return OK;
}

MBOOL
DualFeatureEffectHal::
flush()
{
    Mutex::Autolock autoLock(mOpLock);
    if(mpDepthMapPipe != nullptr)
    {
        mpDepthMapPipe->flush();
    }
    if(mpBokehPipe != nullptr)
    {
        mpBokehPipe->flush();
    }
    return MTRUE;
}

MBOOL
DualFeatureEffectHal::
checkRequiredData(
    sp<DualFeatureRequest> pRequest
)
{
    DualFeatureReqBufferID requiredIDs[] = {RBID_IN_RESIZERAW_MAIN1, RBID_IN_RESIZERAW_MAIN2,
                                        RBID_IN_LCSO_MAIN1, RBID_IN_LCSO_MAIN2,
                                        RBID_IN_APP_META_MAIN1, RBID_IN_HAL_META_MAIN1,
                                        RBID_IN_HAL_META_MAIN2, RBID_OUT_MAINIMAGE,
                                        RBID_OUT_PREVIEW,
                                        RBID_OUT_APP_META, RBID_OUT_HAL_META};

    for(DualFeatureReqBufferID bufferID : requiredIDs)
    {
        if(!pRequest->isRequestBuffer(bufferID))
        {
            MY_LOGE("reqID=%d BufferID:%d  name=%s not exist! Please make sure data exist!",
                        pRequest->getRequestNo(), bufferID, RBID2Name(bufferID));
            return MFALSE;
        }
    }
    // thridparty mode need depthmap buffer extraly
    if((mSetting.mMode & E_STEREO_FEATURE_MTK_DEPTHMAP)
        && !pRequest->isRequestBuffer(RBID_OUT_DEPTHMAP))
    {
        MY_LOGE("reqID=%d DepthMap buffer not exist! mSetting.mMode=%d bufferID=%d",
                    pRequest->getRequestNo(), mSetting.mMode, RBID_OUT_DEPTHMAP);
        return MFALSE;
    }
    // for tk flow, it needs to check blur map
    if((mSetting.mMode & (E_STEREO_FEATURE_VSDOF|E_STEREO_FEATURE_CAPTURE))
        && !pRequest->isRequestBuffer(RBID_OUT_BLUR_MAP))
    {
        MY_LOGE("reqID=%d BlurMap buffer not exist! mSetting.mMode=%d bufferID=%d",
                    pRequest->getRequestNo(), mSetting.mMode, RBID_OUT_BLUR_MAP);
        return MFALSE;
    }
    return MTRUE;
}

MBOOL
DualFeatureEffectHal::
buildLinkedPipe()
{
    // DepthPipeItem -> BokehPipeItem
    sp<ILinkedPipeItem> pPipeItem_Depth = new DepthPipeItem(mpDepthMapPipe);
    sp<ILinkedPipeItem> pPipeItem_Bokeh = new BokehPipeItem(mpBokehPipe);
    //
    pPipeItem_Depth->mpNext = pPipeItem_Bokeh;
    pPipeItem_Depth->mpHead = pPipeItem_Depth;
    //
    pPipeItem_Bokeh->mpNext = nullptr;
    pPipeItem_Bokeh->mpHead = pPipeItem_Depth;
    // head is depth pipe item
    mpLinkedPipe = pPipeItem_Depth;

    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualFeatureEffectHal::
EffectHalCb(
    MVOID* tag,
    ResultState status,
    sp<IDualFeatureRequest>& request
)
{
    DualFeatureEffectHal *pDualHal = reinterpret_cast<DualFeatureEffectHal*>(tag);
    pDualHal->processRequestCallback(status, request);
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualFeatureEffectHal::
processRequestCallback(
    ResultState status,
    sp<IDualFeatureRequest>& request
)
{
    sp<DualFeatureRequest> pDualFeatureRequest = (DualFeatureRequest*)request.get();
    //
    request->stopTimer();
    MY_LOGD("[DualFeatureHal] reqID=%d status=%s total time=%dms",
                request->getRequestNo(), ResultState2Name(status), request->getElapsedTime());
    // get feature param
    FeaturePipeParam param = pDualFeatureRequest->mFeatureParam;
    MBOOL bNeedCallback = MFALSE;
    switch(status)
    {
        case eRESULT_DEPTH_NOT_READY:
        case eRESULT_COMPLETE:
            param.mQParams.mDequeSuccess = MTRUE;
            bNeedCallback = MTRUE;
        break;
        case eRESULT_ERROR:
        case eRESULT_FLUSH:
            param.mQParams.mDequeSuccess = MFALSE;
            bNeedCallback = MTRUE;
        break;
        case eRESULT_YUV_DONE:
        break;
        default:
            MY_LOGE("Should not happened.");
    }
    if(bNeedCallback && param.mCallback != nullptr)
    {
        param.mCallback(FeaturePipeParam::MSG_FRAME_DONE, param);
    }
    else if(param.mCallback == nullptr)
    {
        MY_LOGE("cannot get combin FeaturePipeParam, should not happened.");
    }
}
};
};
};
