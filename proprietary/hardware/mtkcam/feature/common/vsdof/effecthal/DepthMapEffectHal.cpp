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

#define LOG_TAG "DepthMapEffectHal"

 // Standard C header file

// Android system/core header file
#include <cutils/properties.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/utils/std/Log.h>
// Module header file
#include <mtkcam/feature/stereo/effecthal/DepthMapEffectHal.h>
// Local header file

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

#define LOG_TAG "DepthMapEffectHal"

using namespace android;

namespace NSCam{
namespace NSCamFeature{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
DepthMapEffectHal::
DepthMapEffectHal()
: mpDepthMapPipe(NULL),
  mbReadyToPush(MFALSE)
{
    miLogLevel = ::property_get_int32("vendor.debug.camera.log", -1);
    if ( miLogLevel == -1 ) {
        miLogLevel = ::property_get_int32("vendor.debug.camera.log.depthMap", 0);
    }
}

DepthMapEffectHal::
~DepthMapEffectHal()
{}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  EffectHalBase Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
status_t
DepthMapEffectHal::
initImpl()
{
    return OK;
}

status_t
DepthMapEffectHal::
uninitImpl()
{
    MY_LOGD("+");
    Mutex::Autolock autoLock(mOpLock);
    if (mpDepthMapPipe)
    {
        mpDepthMapPipe->uninit();
        delete mpDepthMapPipe;
        mpDepthMapPipe = NULL;
    }
    MY_LOGD("-");

    return OK;
}

status_t
DepthMapEffectHal::
prepareImpl()
{
    return OK;
}

status_t
DepthMapEffectHal::
releaseImpl()
{
    return OK;
}

status_t
DepthMapEffectHal::
getNameVersionImpl(EffectHalVersion &nameVersion) const
{
    return OK;
}

status_t
DepthMapEffectHal::
setParametersImpl(sp<EffectParameter> parameter)
{
    Mutex::Autolock autoLock(mOpLock);
    // setting
    mpSetting = new DepthMapPipeSetting();
    mpSetting->miSensorIdx_Main1 = parameter->getInt(EFFECTKEY_SENSOR_IDX_MAIN1);
    mpSetting->miSensorIdx_Main2 = parameter->getInt(EFFECTKEY_SENSOR_IDX_MAIN2);
    MY_LOGD("miSensorIdx_Main1=%d miSensorIdx_Main2=%d",
            mpSetting->miSensorIdx_Main1, mpSetting->miSensorIdx_Main2);
    // pipe option
    mpPipeOption = new DepthMapPipeOption();
    mpPipeOption->mSensorType = (SeneorModuleType) parameter->getInt(EFFECTKEY_SENSOR_TYPE);
    mpPipeOption->mFeatureMode = (DepthNodeFeatureMode) parameter->getInt(EFFECTKEY_FEATURE_MODE);
    if(parameter->getInt(EFFECTKEY_FLOW_TYPE) == -1)
        mpPipeOption->mFlowType = eDEPTH_FLOW_TYPE_QUEUED_DEPTH; // default flow type: QUEUED DEPTH
    else
        mpPipeOption->mFlowType = (DepthMapFlowType) parameter->getInt(EFFECTKEY_FLOW_TYPE);
    mpPipeOption->setEnableDepthGenControl(MTRUE, 1);
    //
    MY_LOGD("sensorType=%d featureMode=%d flowType=%d",
                mpPipeOption->mSensorType, mpPipeOption->mFeatureMode, mpPipeOption->mFlowType);
    return OK;
}

status_t
DepthMapEffectHal::
setParameterImpl(String8 &key, String8 &object)
{
    return OK;
}

status_t
DepthMapEffectHal::
startImpl(uint64_t *uid)
{
    Mutex::Autolock autoLock(mOpLock);
    mpDepthMapPipe = IDepthMapPipe::createInstance(mpSetting, mpPipeOption);
    mpDepthMapPipe->init();
    mpDepthMapPipe->sync();
    return OK;
}

status_t
DepthMapEffectHal::
abortImpl(EffectResult &result, EffectParameter const *parameter)
{
    return OK;
}

status_t
DepthMapEffectHal::
updateEffectRequestImpl(const sp<EffectRequest> request)
{
    MY_LOGE("Please use sp<IDepthMapEffectRequest> instead of sp<EffectRequest>.");
    return BAD_VALUE;
}

bool
DepthMapEffectHal::
allParameterConfigured()
{
    Mutex::Autolock autoLock(mOpLock);
    if( mpSetting != nullptr &&
        mpSetting->miSensorIdx_Main1 >=0 &&
        mpSetting->miSensorIdx_Main2 >=0 )
    {
        mbReadyToPush = MTRUE;
        return true;
    }
    else
    {
        MY_LOGE("Sensor index not ready!");
        mbReadyToPush = MFALSE;
        return false;
    }
}

status_t
DepthMapEffectHal::
getCaptureRequirementImpl(EffectParameter *inputParam, Vector<EffectCaptureRequirement> &requirements) const
{
    return OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DepthMapEffectHal Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

status_t
DepthMapEffectHal::
updateEffectRequest(android::sp<IDepthMapEffectRequest> request)
{
    Mutex::Autolock autoLock(mOpLock);

    MY_LOGD_IF(miLogLevel>=1, "+ reqID=%d", request->getRequestNo());

    if(mbReadyToPush)
    {
        mpDepthMapPipe->enque(request);
    }
    else
    {
        MY_LOGE("Not ready for pushing EffectRequest into DepthMapPipe!");
        return -1;
    }

    MY_LOGD_IF(miLogLevel>=1, "-", request->getRequestNo());
    return OK;
}


MBOOL
DepthMapEffectHal::
flush()
{
    if(mpDepthMapPipe != NULL)
    {
        mpDepthMapPipe->flush();
        return MTRUE;
    }
    else
    {
        return MFALSE;
    }
}

}; //NSFeatureBM
}; //NSCam
