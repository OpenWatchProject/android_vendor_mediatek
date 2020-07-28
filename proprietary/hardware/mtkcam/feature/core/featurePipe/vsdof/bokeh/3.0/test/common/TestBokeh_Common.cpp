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

#define LOG_TAG "test/TestBokeh_Common"

#include <time.h>
#include <gtest/gtest.h>

#include <vector>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/feature/stereo/StereoCamEnum.h>
#include <mtkcam/drv/IHalSensor.h>
#include <featurePipe/core/include/SyncUtil.h>
#include <featurePipe/core/include/CamGraph.h>
//
#include "TestBokeh_Common.h"

using namespace VSDOF::Bokeh::UT;

UTEnvironmenSetup::
UTEnvironmenSetup(char* username, Profile profile)
: mvSensorIndex{-1, -1}
, mProfile(profile)
{
    mUsername = username;
    NSCam::IHalSensorList* pHalSensorList = MAKE_HalSensorList();
    pHalSensorList->searchSensors();
    mSensorCount = pHalSensorList->queryNumberOfSensors();
    MY_LOGD("mSensorCount  :%d", mSensorCount);
    mIsReadyToUT = powerOnSensor();

    //DepthPipeLoggingSetup::mbProfileLog = MTRUE;
    //DepthPipeLoggingSetup::mbDebugLog = MTRUE;

    MY_LOGD("mIsReadyToUT: %d", mIsReadyToUT);
    if(mIsReadyToUT)
    {
        StereoSettingProvider::setImageRatio(profile.imageRatio);
        StereoSettingProvider::setStereoProfile(profile.sensorProfile);
        StereoSettingProvider::setStereoFeatureMode(profile.featureMode);
        StereoSettingProvider::setStereoModuleType(profile.moduleType);
    }
    else
        MY_LOGE("Failed to init the environment setup!");


}

UTEnvironmenSetup::
~UTEnvironmenSetup()
{
    MY_LOGD("+");
    // close sensor
    for(int i = 0; i < 2; i++)
    {
        if(mvSensorIndex[i] >= 0)
        {
            powerOffSensor(mvSensorIndex[i]);
        }
    }

    if(mpHalSensor)
    {
        mpHalSensor->destroyInstance(mUsername);
        mpHalSensor = NULL;
    }
    MY_LOGD("-");
}

bool
UTEnvironmenSetup::
powerOnSensor()
{
    MY_LOGD("+");
    if(mSensorCount < 2)
    {
        MY_LOGE("mSensorCount:%d  < 2", mSensorCount);
        return false;
    }

    NSCam::IHalSensorList* pHalSensorList = MAKE_HalSensorList();
    StereoSettingProvider::getStereoSensorIndex(mvSensorIndex[0], mvSensorIndex[1]);
    MY_LOGD("mvSensorIndex =%d %d", mvSensorIndex[0], mvSensorIndex[1]);

    MUINT pIndex[] = { (MUINT)mvSensorIndex[0], (MUINT)mvSensorIndex[1]};
    if(!pHalSensorList)
    {
        MY_LOGE("pHalSensorList == NULL");
        return false;
    }
    //
    mpHalSensor = pHalSensorList->createSensor( mUsername,
                                               2,
                                               pIndex);
    if(mpHalSensor == NULL)
    {
       MY_LOGE("mpHalSensor is NULL");
       return false;
    }
    // In stereo mode, Main1 needs power on first.
    // Power on main1 and main2 successively one after another.
    if( !mpHalSensor->powerOn(mUsername, 1, &pIndex[0]) )
    {
        MY_LOGE("sensor power on failed: %d", pIndex[0]);
        return false;
    }
    if( !mpHalSensor->powerOn(mUsername, 1, &pIndex[1]) )
    {
        MY_LOGE("sensor power on failed: %d", pIndex[1]);
        powerOffSensor(pIndex[0]);
        return false;
    }

    MY_LOGD("-");
    return true;
}

bool
UTEnvironmenSetup::
powerOffSensor(MUINT index)
{
    if( !mpHalSensor->powerOff(mUsername, 1, &index) )
    {
        MY_LOGE("sensor power off failed: %d", index);
        return false;
    }
    return true;
}


MBOOL VSDOF::Bokeh::UT::setupReqMetadata(EffectRequestPtr pRequest)
{

    MY_LOGD("Eric: setupReqMetadata start");
    IMetadata* pMetadata;
    // InAppMeta
    pMetadata = new IMetadata();
    pushRequestMetadata(pRequest, {BOKEH_ER_IN_APP_META, FRAME_INPUT}, pMetadata);

    trySetMetadata<MINT32>(pMetadata, MTK_JPEG_ORIENTATION, 0);
    trySetMetadata<MUINT8>(pMetadata, MTK_CONTROL_AF_TRIGGER, 0);
    trySetMetadata<MINT32>(pMetadata, MTK_STEREO_FEATURE_DOF_LEVEL, 0);
    trySetMetadata<MINT32>(pMetadata, MTK_CONTROL_AF_STATE, 1);

    IMetadata::IEntry entry(MTK_3A_FEATURE_AF_ROI);
    entry.push_back(940, Type2Type< MINT32 >());
    entry.push_back(520, Type2Type< MINT32 >());
    entry.push_back(980, Type2Type< MINT32 >());
    entry.push_back(560, Type2Type< MINT32 >());
    pMetadata->update(MTK_3A_FEATURE_AF_ROI, entry);

    // InHalMeta Main1
    pMetadata = new IMetadata();
    pushRequestMetadata(pRequest, {BOKEH_ER_IN_HAL_META_MAIN1, FRAME_INPUT}, pMetadata);

    // magic num
    trySetMetadata<MINT32>(pMetadata, MTK_P1NODE_PROCESSOR_MAGICNUM, 0);
    // sensor mode
    trySetMetadata<MINT32>(pMetadata, MTK_P1NODE_SENSOR_MODE, SENSOR_SCENARIO_ID_NORMAL_PREVIEW);

    // InHalMain2
    pMetadata = new IMetadata();
    trySetMetadata<MINT32>(pMetadata, MTK_P1NODE_PROCESSOR_MAGICNUM, 0);
    pushRequestMetadata(pRequest, {BOKEH_ER_IN_HAL_META_MAIN2, FRAME_INPUT}, pMetadata);
    // OutAppMeta BID_META_OUT_APP
    pMetadata = new IMetadata();
    pushRequestMetadata(pRequest, {BOKEH_ER_OUT_APP_META, FRAME_OUTPUT}, pMetadata);
    // OutHalMeta
    pMetadata = new IMetadata();
    pushRequestMetadata(pRequest, {BOKEH_ER_OUT_HAL_META, FRAME_OUTPUT}, pMetadata);

    MY_LOGD("Eric: setupReqMetadata end");

    return MTRUE;
}



MBOOL VSDOF::Bokeh::UT::pushRequestMetadata(
    EffectRequestPtr pRequest,
    const BokehNodeBufferSetting& setting,
    IMetadata* pMetaBuf
)
{
    sp<EffectFrameInfo> pEffectFrame = new EffectFrameInfo(pRequest->getRequestNo(), setting.bufferID);
    sp<EffectParameter> pEffParam = new EffectParameter();

    pEffParam->setPtr(BOKEH_META_KEY_STRING, (void*)pMetaBuf);
    pEffectFrame->setFrameParameter(pEffParam);
    //
    if(setting.ioType == FRAME_INPUT)
        pRequest->vInputFrameInfo.add(setting.bufferID, pEffectFrame);
    else
        pRequest->vOutputFrameInfo.add(setting.bufferID, pEffectFrame);

    return MTRUE;
}
