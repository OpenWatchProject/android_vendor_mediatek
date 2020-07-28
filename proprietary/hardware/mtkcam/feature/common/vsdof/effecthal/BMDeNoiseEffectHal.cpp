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
#define LOG_TAG "BMDeNoiseEffectHal"

#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>
#include <mtkcam/feature/stereo/effecthal/BMDeNoiseEffectHal.h>
#include <mtkcam/feature/stereo/pipe/IBMDeNoisePipe.h>


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

using namespace android;

namespace NSCam{

BMDeNoiseEffectHal::
BMDeNoiseEffectHal()
: mpBMDeNoisePipe(nullptr),
  miSensorIdx_Main1(-1),
  miSensorIdx_Main2(-1),
  mbReadyToPush(MFALSE)
{
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("vendor.debug.camera.log", cLogLevel, "0");
    miLogLevel = atoi(cLogLevel);
    if ( miLogLevel == 0 ) {
        ::property_get("vendor.debug.camera.log.bmdenoise", cLogLevel, "0");
        miLogLevel = atoi(cLogLevel);
    }
}

BMDeNoiseEffectHal::
~BMDeNoiseEffectHal()
{}

status_t
BMDeNoiseEffectHal::
initImpl()
{
    return OK;
}

status_t
BMDeNoiseEffectHal::
uninitImpl()
{
    Mutex::Autolock autoLock(mOpLock);
    if (mpBMDeNoisePipe != nullptr)
    {
        mpBMDeNoisePipe->uninit();
        mpBMDeNoisePipe = nullptr;
    }


    return OK;
}

status_t
BMDeNoiseEffectHal::
prepareImpl()
{
    return OK;
}

status_t
BMDeNoiseEffectHal::
releaseImpl()
{
    return OK;
}

status_t
BMDeNoiseEffectHal::
getNameVersionImpl(EffectHalVersion &nameVersion) const
{
    return OK;
}

status_t
BMDeNoiseEffectHal::
setParametersImpl(sp<EffectParameter> parameter)
{
    Mutex::Autolock autoLock(mOpLock);
    miSensorIdx_Main1 = parameter->getInt(SENSOR_IDX_MAIN1);
    miSensorIdx_Main2 = parameter->getInt(SENSOR_IDX_MAIN2);

    MY_LOGD("SENSOR_IDX: %d, %d", miSensorIdx_Main1, miSensorIdx_Main2);

    return OK;
}

status_t
BMDeNoiseEffectHal::
setParameterImpl(String8 &key, String8 &object)
{
    return OK;
}

status_t
BMDeNoiseEffectHal::
startImpl(uint64_t *uid)
{
    Mutex::Autolock autoLock(mOpLock);
    mpBMDeNoisePipe = IBMDeNoisePipe::createInstance(miSensorIdx_Main1, miSensorIdx_Main2);
    mpBMDeNoisePipe->setFlushOnStop(MTRUE);
    mpBMDeNoisePipe->init();
    return OK;
}

status_t
BMDeNoiseEffectHal::
abortImpl(EffectResult &result, EffectParameter const *parameter)
{
    return OK;
}

status_t
BMDeNoiseEffectHal::
updateEffectRequestImpl(const sp<EffectRequest> request)
{
    Mutex::Autolock autoLock(mOpLock);

    MY_LOGD_IF(miLogLevel>=1, "+ reqID=%d", request->getRequestNo());

    if(mbReadyToPush)
    {
        sp<EffectRequest> enque_req = (sp<EffectRequest>)request;
        mpBMDeNoisePipe->enque(enque_req);
    }
    else
    {
        MY_LOGE("Not ready for pushing EffectRequest into BMDeNoisePipe!");
        return -1;
    }

    MY_LOGD_IF(miLogLevel>=1, "-", request->getRequestNo());
    return OK;
}

bool
BMDeNoiseEffectHal::
allParameterConfigured()
{
    Mutex::Autolock autoLock(mOpLock);
    if( miSensorIdx_Main1 >=0 && miSensorIdx_Main2 >=0 )
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
BMDeNoiseEffectHal::
getCaptureRequirementImpl(EffectParameter *inputParam, Vector<EffectCaptureRequirement> &requirements) const
{
    return OK;
}


MBOOL
BMDeNoiseEffectHal::
flush()
{
    if(mpBMDeNoisePipe != nullptr)
    {
        mpBMDeNoisePipe->flush();
        return MTRUE;
    }
    else
    {
        return MFALSE;
    }
}

};
