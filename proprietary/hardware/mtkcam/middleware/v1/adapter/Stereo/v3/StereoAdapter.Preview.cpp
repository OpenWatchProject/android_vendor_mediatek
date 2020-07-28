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

#define LOG_TAG "MtkCam/StereoAdapter"
//
#include "MyUtils.h"
//
#include <cutils/properties.h>      // [debug] should be remove
//
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <camera_custom_zsd.h>
#include <mtkcam/utils/fwk/MtkCamera.h>
//
#include <inc/ImgBufProvidersManager.h>
//
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/middleware/v1/ICamAdapter.h>
#include <inc/BaseCamAdapter.h>
//
#include "inc/v3/StereoAdapter.h"
using namespace NSStereoAdapter;
//
#include <mtkcam/drv/IHalSensor.h>
using namespace NSCam;
//
#include <mtkcam/utils/hw/CamManager.h>
using namespace NSCam::Utils;
//
#include <camera_custom_vt.h>
//
#include <cutils/properties.h>
//
//#include <hwutils/CameraProfile.h>
//using namespace CPTool;
//
/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

#define MY_LOGD1(...)               MY_LOGD_IF((mLogLevel>=1),__VA_ARGS__)
#define MY_LOGD2(...)               MY_LOGD_IF((mLogLevel>=2),__VA_ARGS__)
#define MY_LOGD3(...)               MY_LOGD_IF((mLogLevel>=3),__VA_ARGS__)

//
#define FUNC_START                  MY_LOGD2("+")
#define FUNC_END                    MY_LOGD2("-")


/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
previewEnabled() const
{
    bool ret = false;
    ret =  (mpStateManager->isState(IState::eState_Preview)||
            mpStateManager->isState(IState::eState_ZSLCapture)||
                recordingEnabled());
    MY_LOGD("ret (%d)", ret);
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        //MY_LOGE("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
startPreview()
{
    CAM_TRACE_NAME("StereoAdapter:startPreview");
    return  mpStateManager->getCurrentState()->onStartPreview(this);
}


/******************************************************************************
*
*******************************************************************************/
void
CamAdapter::
stopPreview()
{
    CAM_TRACE_NAME("StereoAdapter:stopPreview");
    mpStateManager->getCurrentState()->onStopPreview(this);
}



/******************************************************************************
*
*******************************************************************************/
int
CamAdapter::
getSensorScenario
(
)
{
    CAM_TRACE_NAME("StereoAdapter:getSensorScenario");
    int scenarioMap[]= {
                        SENSOR_SCENARIO_ID_NORMAL_PREVIEW,    //0
                        SENSOR_SCENARIO_ID_NORMAL_PREVIEW,    //1:Preview Mode
                        SENSOR_SCENARIO_ID_NORMAL_PREVIEW,    //2:Capture Mode
                        SENSOR_SCENARIO_ID_NORMAL_PREVIEW,    //3:JPEG only
                        SENSOR_SCENARIO_ID_NORMAL_VIDEO,      //4:Video Preview
                        SENSOR_SCENARIO_ID_SLIM_VIDEO1,       //5:Slim Video 1
                        SENSOR_SCENARIO_ID_SLIM_VIDEO2,       //6:Slim Video 2
                       };

    // Engineer sensor Scenario
    String8 ms8SaveMode = mpParamsManagerV3->getParamsMgr()->getStr(MtkCameraParameters::KEY_RAW_SAVE_MODE);

    int camera_mode = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_CAMERA_MODE);
    if (camera_mode != 0)
    {
        ms8SaveMode = String8("4"); // force sensor scenario to "normal video" // should remove when em camera app modifed
    }
    const char *strSaveMode = ms8SaveMode.string();
    int mode = atoi(strSaveMode);


    if (mode > (int)(sizeof(scenarioMap)/sizeof(int) - 1) || mode < 0 )
    {
        MY_LOGW("Wrong mode:%d", mode);
        return SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    }

    // for debug
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get( "vendor.cameng.force_sensormode", value, "-1");
    MINT32 val = atoi(value);
    if( val > 0 )
    {
        MY_LOGD1("force use sensor scenario %d", val);
        return val;
    }

    //
    return scenarioMap[mode];
}


/******************************************************************************
*   CamAdapter::startPreview() -> IState::onStartPreview() ->
*   IStateHandler::onHandleStartPreview() -> CamAdapter::onHandleStartPreview()
*******************************************************************************/
status_t
CamAdapter::
onHandleStartPreview()
{
    CAM_TRACE_NAME("StereoAdapter:onHandleStartPreview");
    FUNC_START
    status_t ret = INVALID_OPERATION;
    int templateID = 0;
    int camera_mode = -1;
    //
    int sensorScenario = getSensorScenario();
    //
    if  ( ! BaseCamAdapter::init() )
    {
        goto lbExit;
    }
    camera_mode = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_CAMERA_MODE);
    // If current camera mode is record, it needs to ignore startPreview.
    if(camera_mode == 1)
    {
        ret = mpFlowControl->startPreview();
    }
    else if(camera_mode == 2)
    {
      ret = mpFlowControl->startRecording();
    }
    else
    {
        MY_LOGE("Should not happened");
    }
    if ( ret != OK )
    {
        MY_LOGE("Error to mpFlowControl->startPreview()");
        goto lbExit;
    }
    //
    mpStateManager->transitState(IState::eState_Preview);
   //
lbExit:
    //
    if(ret != OK)
    {
        forceStopAndCleanPreview();
    }
    //
    FUNC_END;
    return ret;
}


/******************************************************************************
*   CamAdapter::stopPreview() -> IState::onStopPreview() ->
*   IStateHandler::onHandleStopPreview() -> CamAdapter::onHandleStopPreview()
*******************************************************************************/
status_t
CamAdapter::
onHandleStopPreview()
{
    CAM_TRACE_NAME("StereoAdapter:onHandleStopPreview");
    return forceStopAndCleanPreview();
}


/******************************************************************************
*   CamAdapter::takePicture() -> IState::onPreCapture() ->
*   IStateHandler::onHandlePreCapture() -> CamAdapter::onHandlePreCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandlePreCapture()
{
    CAM_TRACE_NAME("StereoAdapter:onHandlePreCapture");
    FUNC_START;
    status_t ret = OK;
    int flashRequired = 0;
    //
    ret = mpFlowControl->precapture(flashRequired);
    if (ret != OK)
    {
        MY_LOGE("mpFlowControl->precapture Fail!");
        goto lbExit;
    }
    //
    mpStateManager->transitState(IState::eState_PreCapture);
    return OK;

lbExit:

    FUNC_END;
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
forceStopAndCleanPreview()
{
    FUNC_START;
    status_t ret = INVALID_OPERATION;
    //
    int camera_mode = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_CAMERA_MODE);

    if (mpFlowControl->stopPreview() != OK)
    {
        MY_LOGD("mpFlowControl->stopPreview() fail");
    }

    MY_LOGD1("transitState->eState_Idle");
    mpStateManager->transitState(IState::eState_Idle);
    //
    FUNC_END;
    //
    return ret;
}



/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
enableFlashCalibration(int enable)
{
    CAM_TRACE_NAME("StereoAdapter:enableFlashCalibration");
    FUNC_START;
    //
    status_t ret = OK;
    //
    #if '1'==MTKCAM_HAVE_3A_HAL
    IHal3A* pHal3a = MAKE_Hal3A(getOpenId(), getName());

    if ( ! pHal3a )
    {
        MY_LOGE("pHal3a == NULL");
        return INVALID_OPERATION;
    }

    if ( ! pHal3a->send3ACtrl(NS3Av3::E3ACtrl_EnableFlashQuickCalibration,enable, 0) )
    {
        MY_LOGE("enableFlashQuickCalibration fail");
        ret = INVALID_OPERATION;
        goto lbExit;
    }
    #endif

lbExit:
    #if '1'==MTKCAM_HAVE_3A_HAL
    pHal3a->destroyInstance(getName());
    #endif
    //
    FUNC_END;
    return ret;
}
