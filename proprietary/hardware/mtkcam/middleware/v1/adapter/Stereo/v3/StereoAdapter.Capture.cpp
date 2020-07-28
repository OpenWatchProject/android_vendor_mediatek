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
#include <mtkcam/utils/fwk/MtkCamera.h>
//
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <inc/ImgBufProvidersManager.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/middleware/v1/IParamsManagerV3.h>
#include <mtkcam/middleware/v1/ICamAdapter.h>
#include <inc/BaseCamAdapter.h>
//
#include "inc/v3/StereoAdapter.h"
using namespace NSStereoAdapter;
//
#include <hardware/camera3.h>
//
#include <mtkcam/drv/IHalSensor.h>
//
#include <StereoFlowControl.h>
using namespace NSCam::v1::NSLegacyPipeline;

#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>

/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")


/******************************************************************************
*   Function Prototype.
*******************************************************************************/
//
bool
createShotInstance(
    sp<IShot>&          rpShot,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId,
    sp<IParamsManager>  pParamsMgr
);
//

/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
updateShotInstance()
{
    String8 s8ShotMode = getParamsManager()->getShotModeStr();
    uint32_t u4ShotMode = getParamsManager()->getShotMode();
    // hard code use zsd flow to do capture.
    s8ShotMode = "ZSD";
    u4ShotMode = NSCam::eShotMode_ZsdShot;
    MY_LOGI("msAppMode(%s) <shot mode> %#x(%s)",
            msAppMode.string(),
            u4ShotMode,
            s8ShotMode.string());
    //return  createShotInstance(mpShot, u4ShotMode, getOpenId(), getParamsManager());
    return true;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
prepareToCapture()
{
    status_t status = NO_ERROR;
    //
lbExit:
    return status;
}


/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
isTakingPicture() const
{
    bool ret =  mpStateManager->isState(IState::eState_PreCapture) ||
                mpStateManager->isState(IState::eState_NormalCapture) ||
                mpStateManager->isState(IState::eState_VideoSnapshot);
    if  ( ret )
    {
        MY_LOGD("isTakingPicture(1):%s", mpStateManager->getCurrentState()->getName());
    }
    //
    return  ret;
}

/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
isSupportVideoSnapshot()
{
    FUNC_START;
    bool ret = false;
    if( ::strcmp(mpParamsManagerV3->getParamsMgr()->getStr(MtkCameraParameters::KEY_VIDEO_FRAME_FORMAT), MtkCameraParameters::PIXEL_FORMAT_BITSTREAM) == 0 ||
            mpParamsManagerV3->getParamsMgr()->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) > 30)
    {
        ret = false;
    }
    else
    {
        ret = true;
    }
    FUNC_END;
    return ret;
}



/******************************************************************************
* Normal Capture: Preview -> PreCapture->Idle
* ZSD Capture: Preview -> Capture
* Recording capture: Record -> Capture
*******************************************************************************/
status_t
CamAdapter::
takePicture()
{
    CAM_TRACE_NAME("StereoAdapter:takePicture");

    FUNC_START;
    status_t status = OK;

    if (isTakingPicture())
    {
        MY_LOGW("already taking picture...");
        return status;
    }

    if(mpStateManager->isState(IState::eState_Recording))
    {
        if (isSupportVideoSnapshot())
        {
            status = mpStateManager->getCurrentState()->onCapture(this);
            if (OK != status)
            {
                goto lbExit;
            }
        }
        else
        {
            MY_LOGE("Not support VSS");
            status = INVALID_OPERATION;
            goto lbExit;
        }
    }

    // Use ZSL mode as default
    {
        status = mpStateManager->getCurrentState()->onCapture(this);
        if(OK != status){
            goto lbExit;
        }
    }
lbExit:
    FUNC_END;
    return status;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
cancelPicture()
{
    CAM_TRACE_NAME("Adapter:cancelPicture");
    FUNC_START;
    if( mpStateManager->isState(IState::eState_NormalCapture) ||
        mpStateManager->isState(IState::eState_ZSLCapture) ||
        mpStateManager->isState(IState::eState_VideoSnapshot))
    {
        mpStateManager->getCurrentState()->onCancelCapture(this);
    }
    FUNC_END;
    return OK;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
setCShotSpeed(int32_t i4CShotSpeed)
{
    FUNC_START;
    MY_LOGE("no implement.");
    /*if(i4CShotSpeed <= 0 )
    {
        MY_LOGE("cannot set continuous shot speed as %d fps)", i4CShotSpeed);
        return BAD_VALUE;
    }

    sp<IShot> pShot = mpShot;
    if( pShot != 0 )
    {
        pShot->sendCommand(eCmd_setCShotSpeed, i4CShotSpeed, 0);
    }*/

    FUNC_END;
    return OK;
}


/******************************************************************************
*   CamAdapter::takePicture() -> IState::onCapture() ->
*   (Idle)IStateHandler::onHandleNormalCapture() -> CamAdapter::onHandleNormalCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandleNormalCapture()
{
    CAM_TRACE_NAME("StereoAdapter:onHandleNormalCapture");
    FUNC_START;

    status_t status = DEAD_OBJECT;
    sp<ZipImageCallbackThread> pCallbackThread = mpZipCallbackThread.promote();
    sp<ICaptureCmdQueThread> pCaptureCmdQueThread = mpCaptureCmdQueThread;
    //
    if( !pCallbackThread.get() )
    {
        MY_LOGE("no callback thread");
        goto lbExit;
    }
    //
    pCallbackThread->setShotMode(mShotMode, IState::eState_Idle);
    pCallbackThread = 0;
    //
    if  ( pCaptureCmdQueThread != 0 ) {
        status = pCaptureCmdQueThread->onCapture();
    }
    //
lbExit:
    FUNC_END;
    return  status;
}

/******************************************************************************
*
*******************************************************************************/
int
CamAdapter::
getFlashQuickCalibrationResult()
{
    FUNC_START;
    //
    #if '1'==MTKCAM_HAVE_3A_HAL
    //
    IHal3A* pHal3a = MAKE_Hal3A(getOpenId(), getName());

    if ( ! pHal3a )
    {
        MY_LOGE("pHal3a == NULL");
        return 1;
    }

    int result = 0;
    pHal3a->send3ACtrl(NS3Av3::E3ACtrl_GetQuickCalibration, (MINTPTR)&result, 0);

    pHal3a->destroyInstance(getName());
    #endif
    //
    FUNC_END;
    return result;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
onHandleCaptureDone()
{
    CAM_TRACE_NAME("StereoAdapter:onHandleCaptureDone");
    //
    FUNC_START;
    if (mpStateManager->isState(IState::eState_NormalCapture))
    {
        return onHandleNormalCaptureDone();
    }
    else if (mpStateManager->isState(IState::eState_ZSLCapture))
    {
        return onHandleZSLCaptureDone();
    }
    else if (mpStateManager->isState(IState::eState_VideoSnapshot))
    {
        return onHandleVideoSnapshotDone();
    }

    FUNC_END;
    return  OK;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
onHandleNormalCaptureDone()
{
    CAM_TRACE_NAME("Adapter:onHandleNormalCaptureDone");
    //
    FUNC_START;
    mpStateManager->transitState(IState::eState_Idle);
    FUNC_END;
    return OK ;
}


/******************************************************************************
*   CamAdapter::cancelPicture() -> IState::onCancelCapture() ->
*   IStateHandler::onHandleCancelCapture() -> CamAdapter::onHandleCancelCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandleCancelCapture()
{
    CAM_TRACE_NAME("StereoAdapter:onHandleCancelCapture");
    FUNC_START;
    /*sp<IShot> pShot = mpShot;
    if  ( pShot != 0 )
    {
        pShot->sendCommand(eCmd_cancel);
    }*/
    //
    if (mpStateManager->isState(IState::eState_ZSLCapture))
    {
        onHandleCancelZSLCapture();
    }
    else if (mpStateManager->isState(IState::eState_NormalCapture))
    {
        onHandleCancelNormalCapture();
    }
    else if (mpStateManager->isState(IState::eState_VideoSnapshot))
    {
        onHandleCancelVideoSnapshot();
    }
    FUNC_END;
    return  OK;
}


/******************************************************************************
*   CamAdapter::takePicture() -> IState::onCapture() ->
*   (Preview)IStateHandler::onHandleZSLCapture() -> CamAdapter::onHandleZSLCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandleZSLCapture()
{
    CAM_TRACE_NAME("StereoAdapter:onHandleZSLCapture");
    FUNC_START;

    status_t status = DEAD_OBJECT;
    sp<ZipImageCallbackThread> pCallbackThread = mpZipCallbackThread.promote();
    sp<ICaptureCmdQueThread> pCaptureCmdQueThread = mpCaptureCmdQueThread;
    MUINT32 shotMode = getParamsManager()->getShotMode();
    //
    if( !pCallbackThread.get() )
    {
        MY_LOGE("no callback thread");
        goto lbExit;
    }
    //
    MY_LOGD("ShotMode (%d)", shotMode);
    pCallbackThread->setShotMode(shotMode, IState::eState_Preview);
    pCallbackThread = 0;
    //
    if  ( pCaptureCmdQueThread != 0 ) {
        status = pCaptureCmdQueThread->onCapture();
    }
    //
lbExit:
    FUNC_END;
    return  status;

}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
onHandleZSLCaptureDone()
{
    //
    FUNC_START;
    CAM_TRACE_NAME("StereoAdapter:onHandleZSLCaptureDone");
    if (mpStateManager->isState(IState::eState_ZSLCapture))
    {
        /*sp<IFeatureFlowControl> spFlowControl = IResourceContainer::getInstance(getOpenId())->queryFeatureFlowControl();
        if( spFlowControl == NULL ) {
            MY_LOGW("failed to queryFeatureFlowControl");
        } else {
            spFlowControl->resumePreview();
        }*/

        mpStateManager->transitState(IState::eState_Preview);
    }
    else
    {
        MY_LOGW("Not in IState::ZSLCapture !!");
    }
    FUNC_END;
    return  OK;
}


/******************************************************************************
*   CamAdapter::cancelPicture() -> IState::onCancelCapture() ->
*   IStateHandler::onHandleCancelCapture() -> CamAdapter::onHandleCancelCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandleCancelPreviewCapture()
{
    return  onHandleCancelCapture();
}


/******************************************************************************
*
*
*******************************************************************************/
status_t
CamAdapter::
onHandleVideoSnapshot()
{
    CAM_TRACE_NAME("StereoAdapter:onHandleVideoSnapshot");
    FUNC_START;

    status_t status = DEAD_OBJECT;
    sp<ZipImageCallbackThread> pCallbackThread = mpZipCallbackThread.promote();
    sp<ICaptureCmdQueThread> pCaptureCmdQueThread = mpCaptureCmdQueThread;
    //
    if( !pCallbackThread.get() )
    {
        MY_LOGE("no callback thread");
        goto lbExit;
    }
    //
    pCallbackThread->setShotMode(getParamsManager()->getShotMode(), IState::eState_Recording);
    pCallbackThread = 0;
    //
    mpFlowControl->takePicture();

    if  ( pCaptureCmdQueThread != 0 ) {
        status = pCaptureCmdQueThread->onCapture();
    }
    //
lbExit:
    FUNC_END;
    return  status;

}


/******************************************************************************
*
*
*******************************************************************************/
status_t
CamAdapter::
onHandleVideoSnapshotDone()
{
    FUNC_START;
    if(mpStateManager->isState(IState::eState_VideoSnapshot))
    {
        mpStateManager->transitState(IState::eState_Recording);
    }
    else
    {
        MY_LOGW("Not in IState::eState_VideoSnapshot !!");
    }
    FUNC_END;
    return OK ;
}


/******************************************************************************
*
*
*******************************************************************************/
status_t
CamAdapter::
onHandleCancelVideoSnapshot()
{
    mpStateManager->transitState(IState::eState_Recording);
    return  OK;
}

/******************************************************************************
*
*
*******************************************************************************/
status_t
CamAdapter::
onHandleCancelNormalCapture()
{
    mpStateManager->transitState(IState::eState_Idle);
    return  OK;
}
/******************************************************************************
*
*
*******************************************************************************/
status_t
CamAdapter::
onHandleCancelZSLCapture()
{
    FUNC_START;
    if(mpFlowControl != nullptr)
    {
        ((StereoFlowControl*)mpFlowControl.get())->cancelTakePicture();
    }
    FUNC_END;
    return  OK;
}

/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
onCaptureThreadLoop()
{
    CAM_TRACE_NAME("StereoAdapter:onCaptureThreadLoop");
    FUNC_START;

    StereoShotParam stereoShotParam;
    MUINT32 transform = 0;
    if(mpFlowControl==NULL)
    {
        MY_LOGE("mpFlowControl is null");
        return false;
    }
    //
    //  [1] transit to "Capture" state.
    camera3_request_template_t templateID = CAMERA3_TEMPLATE_STILL_CAPTURE;
    if(mpStateManager->isState(IState::eState_Idle))
    {
        mpStateManager->transitState(IState::eState_NormalCapture);
        templateID = CAMERA3_TEMPLATE_STILL_CAPTURE;
    }
    else
    if(mpStateManager->isState(IState::eState_Preview))
    {
        mpStateManager->transitState(IState::eState_ZSLCapture);
        templateID = CAMERA3_TEMPLATE_STILL_CAPTURE;
    }
    else
    if(mpStateManager->isState(IState::eState_Recording))
    {
        mpStateManager->transitState(IState::eState_VideoSnapshot);
        templateID = CAMERA3_TEMPLATE_VIDEO_SNAPSHOT;
    }
    //
    if( mpStateManager->isState(IState::eState_VideoSnapshot))
    {
        MY_LOGD("In VSS");
    }
    else
    {
        {
            //  [3.1] prepare parameters
            sp<IParamsManager> pParamsMgr = getParamsManager();
            //
            int iPictureWidth = 0, iPictureHeight = 0;
            if(mpStateManager->isState(IState::eState_VideoSnapshot))
            {
            }
            else
            {
                pParamsMgr->getPictureSize(&iPictureWidth, &iPictureHeight);
            }
            //
            int iPreviewWidth = 0, iPreviewHeight = 0;
            pParamsMgr->getPreviewSize(&iPreviewWidth, &iPreviewHeight);
            String8 s8DisplayFormat = mpImgBufProvidersMgr->queryFormat(IImgBufProvider::eID_DISPLAY);
            if  ( String8::empty() == s8DisplayFormat ) {
                MY_LOGW("Display Format is empty");
            }
            //
            if(mName == MtkCameraParameters::APP_MODE_NAME_MTK_ZSD ||       //ZSD preview
                    mpStateManager->isState(IState::eState_VideoSnapshot) ) //VSS shot
            {
            }
            // convert rotation to transform
            MINT32 rotation = pParamsMgr->getInt(CameraParameters::KEY_ROTATION);
            //
            switch(rotation)
            {
                case 0:
                    transform = 0;
                    break;
                case 90:
                    transform = eTransform_ROT_90;
                    break;
                case 180:
                    transform = eTransform_ROT_180;
                    break;
                case 270:
                    transform = eTransform_ROT_270;
                    break;
                default:
                    break;
            }

            //  [3.2] prepare parameters: ShotParam
            stereoShotParam.mShotParam.miPictureFormat =
                    MtkCameraParameters::queryImageFormat(pParamsMgr->getStr(CameraParameters::KEY_PICTURE_FORMAT));
            if  ( 0 != iPictureWidth && 0 != iPictureHeight )
            {
                stereoShotParam.mShotParam.mi4PictureWidth       = iPictureWidth;
                stereoShotParam.mShotParam.mi4PictureHeight      = iPictureHeight;
            }
            else
            {   //  When 3rd-party apk sets "picture-size=0x0", replace it with "preview-size".
                stereoShotParam.mShotParam.mi4PictureWidth       = iPreviewWidth;
                stereoShotParam.mShotParam.mi4PictureHeight      = iPreviewHeight;
            }
            StereoSizeProvider::getInstance()->setCaptureImageSize(iPictureWidth, iPictureHeight);
            stereoShotParam.mShotParam.miPostviewDisplayFormat   = MtkCameraParameters::queryImageFormat(s8DisplayFormat.string());
            stereoShotParam.mShotParam.miPostviewClientFormat    = MtkCameraParameters::queryImageFormat(pParamsMgr->getStr(CameraParameters::KEY_PREVIEW_FORMAT));
            int iPostviewThumbFmt = MtkCameraParameters::queryImageFormat(pParamsMgr->getStr(MtkCameraParameters::KEY_POST_VIEW_FMT));
            stereoShotParam.mShotParam.miPostviewThumbFmt = (iPostviewThumbFmt != NSCam::eImgFmt_UNKNOWN) ? iPostviewThumbFmt : NSCam::eImgFmt_NV21;
            MY_LOGD("postview fmt(0x%x)", stereoShotParam.mShotParam.miPostviewThumbFmt);
            // if postview YUV format is unknown, set NV21 as default
            if (stereoShotParam.mShotParam.miPostviewClientFormat == eImgFmt_UNKNOWN)
                stereoShotParam.mShotParam.miPostviewClientFormat = eImgFmt_NV21;

            // post-view
            MSize postViewSize = StereoSizeProvider::getInstance()->postViewSize();
            stereoShotParam.mShotParam.mi4PostviewWidth          = postViewSize.w;
            stereoShotParam.mShotParam.mi4PostviewHeight         = postViewSize.h;
            stereoShotParam.mShotParam.ms8ShotFileName           = pParamsMgr->getStr(MtkCameraParameters::KEY_CAPTURE_PATH);
            stereoShotParam.mShotParam.mu4ZoomRatio              = pParamsMgr->getZoomRatio();
            stereoShotParam.mShotParam.muSensorGain              = pParamsMgr->getInt(MtkCameraParameters::KEY_ENG_MANUAL_SENSOR_GAIN);
            stereoShotParam.mShotParam.muSensorSpeed             = pParamsMgr->getInt(MtkCameraParameters::KEY_ENG_MANUAL_SHUTTER_SPEED);
            {
                MINT32 sensorGain, sensorSpeed;
                sensorGain = property_get_int32("vendor.camera.sensor.gain", -1);
                if( sensorGain > 0 )
                {
                    stereoShotParam.mShotParam.muSensorGain = sensorGain;
                }
                sensorSpeed = property_get_int32("vendor.camera.sensor.speed", -1);
                if( sensorSpeed > 0 )
                {
                    stereoShotParam.mShotParam.muSensorSpeed = sensorSpeed;
                }
                MY_LOGD("prop (gain,speed)=(%d,%d), param (gain,speed)=(%d,%d)",
                sensorGain, sensorSpeed, stereoShotParam.mShotParam.muSensorGain, stereoShotParam.mShotParam.muSensorSpeed);
            }
            stereoShotParam.mShotParam.mu4ShotCount              = pParamsMgr->getInt(MtkCameraParameters::KEY_BURST_SHOT_NUM);
            stereoShotParam.mShotParam.mu4Transform              = transform;
            stereoShotParam.mShotParam.mu4MultiFrameBlending     = pParamsMgr->getMultFrameBlending();

            {
                if( mpParamsManagerV3 != 0 ) {
                    ITemplateRequest* obj = NSTemplateRequestManager::valueFor(getOpenId());
                    if(obj == NULL) {
                        MY_LOGE("cannot get template request!");
                    }
                    else
                    {
                        //template id is defined in camera3.h
                        stereoShotParam.mShotParam.mAppSetting = obj->getMtkData(templateID);
                        mpParamsManagerV3->updateRequestJpeg(&stereoShotParam.mShotParam.mAppSetting);
                        mpParamsManagerV3->updateRequest(&stereoShotParam.mShotParam.mAppSetting, SENSOR_SCENARIO_ID_NORMAL_CAPTURE);
                        //
                        MRect reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion;
                        MSize sensorSize;
                        mpParamsManagerV3->getCropRegion(sensorSize, reqCropRegion, reqSensorCropRegion, reqPreviewCropRegion, reqSensorPreviewCropRegion);
                        //
                        {
                            IMetadata::IEntry entry1(MTK_SCALER_CROP_REGION);
                            entry1.push_back(reqCropRegion, Type2Type<MRect>());
                            stereoShotParam.mShotParam.mAppSetting.update(MTK_SCALER_CROP_REGION, entry1);
                        }
                        {
                            IMetadata::IEntry entry1(MTK_P1NODE_SENSOR_CROP_REGION);
                            if( mpStateManager->isState(IState::eState_VideoSnapshot) )
                            {
                                entry1.push_back(reqSensorPreviewCropRegion, Type2Type<MRect>());
                            }
                            else
                            {
                                entry1.push_back(reqSensorCropRegion, Type2Type<MRect>());
                            }
                            stereoShotParam.mShotParam.mHalSetting.update(MTK_P1NODE_SENSOR_CROP_REGION, entry1);
                        }

                        // update default HAL settings
                        mpParamsManagerV3->updateRequestHal(&stereoShotParam.mShotParam.mHalSetting,SENSOR_SCENARIO_ID_NORMAL_CAPTURE,MTRUE);

                        {
                            IMetadata::IEntry entry(MTK_HAL_REQUEST_REQUIRE_EXIF);
                            entry.push_back(true, Type2Type< MUINT8 >());
                            stereoShotParam.mShotParam.mHalSetting.update(entry.tag(), entry);
                            MY_LOGD("set HalSetting REQUIRE_EXIF (true)");
                        }
                        if( mIsRaw16CBEnable )
                        {
                            IMetadata::IEntry entry(MTK_STATISTICS_LENS_SHADING_MAP_MODE);
                            entry.push_back(MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON, Type2Type< MUINT8 >());
                            stereoShotParam.mShotParam.mAppSetting.update(entry.tag(), entry);
                            MY_LOGD("DNG set MTK_STATISTICS_LENS_SHADING_MAP_MODE (ON)");
                        }
                    }
                }
                else {
                    MY_LOGW("cannot create paramsmgr v3");
                }
                // T.B.D.
                // stereoShotParam.mShotParam.mHalSetting
                //
            }
            //
            //  [3.3] prepare parameters: JpegParam
            stereoShotParam.mJpegParam.mu4JpegQuality            = pParamsMgr->getInt(CameraParameters::KEY_JPEG_QUALITY);
            stereoShotParam.mJpegParam.mu4JpegThumbQuality       = pParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY);
            stereoShotParam.mJpegParam.mi4JpegThumbWidth         = pParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH);
            stereoShotParam.mJpegParam.mi4JpegThumbHeight        = pParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT);
            stereoShotParam.mJpegParam.ms8GpsLatitude            = pParamsMgr->getStr(CameraParameters::KEY_GPS_LATITUDE);
            stereoShotParam.mJpegParam.ms8GpsLongitude           = pParamsMgr->getStr(CameraParameters::KEY_GPS_LONGITUDE);
            stereoShotParam.mJpegParam.ms8GpsAltitude            = pParamsMgr->getStr(CameraParameters::KEY_GPS_ALTITUDE);
            stereoShotParam.mJpegParam.ms8GpsTimestamp           = pParamsMgr->getStr(CameraParameters::KEY_GPS_TIMESTAMP);
            stereoShotParam.mJpegParam.ms8GpsMethod              = pParamsMgr->getStr(CameraParameters::KEY_GPS_PROCESSING_METHOD);
            //
            stereoShotParam.miDOFLevel = pParamsMgr->getInt(MtkCameraParameters::KEY_STEREO_DOF_LEVEL);
            //
            if(mIsRaw16CBEnable)
            {
                stereoShotParam.mbDngMode = MTRUE;
            }
            //  [4.1] perform Shot operations.
			// todo: add dof level here.
            // [4.2] Notify AP for shutter sound play.
            this->onCB_Shutter(true, 0, getParamsManager()->getShotMode());
            ((StereoFlowControl*)mpFlowControl.get())->takePicture(stereoShotParam);
        }
    }
    //
    mpStateManager->getCurrentState()->onCaptureDone(this);
    //
    //
    FUNC_END;
    return  true;
}

