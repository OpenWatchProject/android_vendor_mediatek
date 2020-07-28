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

#define LOG_TAG "MtkCam/CamAdapter"
//
#include <mtkcam/utils/fwk/MtkCamera.h>
//
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <inc/ImgBufProvidersManager.h>
//
#include <mtkcam/v1/IParamsManager.h>
#include <mtkcam/v1/ICamAdapter.h>
#include <inc/BaseCamAdapter.h>
#include "inc/MtkStereoCamAdapter.h"
using namespace NSMtkStereoCamAdapter;

#include <mtkcam/featureio/stereo_hal_base.h>
// for query stereo static data
#include <mtkcam/featureio/stereo_setting_provider.h>
//
#include <sys/prctl.h>
//

#include <hwsync_drv.h>

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
    uint32_t const u4ShotMode = mpDefaultCtrlNode->getShotMode();
    MY_LOGD("ShotMode(%d)",u4ShotMode);
    return  createShotInstance(mpShot, u4ShotMode, getOpenId(), getParamsManager());
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
prepareToCapture()
{
    status_t status = NO_ERROR;
    sp<ZipImageCallbackThread> pCallbackThread;
    sp<ICaptureCmdQueThread> pCaptureCmdQueThread = mpCaptureCmdQueThread;
    //
    pCallbackThread = new ZipImageCallbackThread(
                            mpStateManager,
                            mpCamMsgCbInfo);
    //
    if  ( !pCallbackThread.get() || OK != (status = pCallbackThread->run("SteteroCamAdapter::prepareToCapture") ) )
    {
        MY_LOGE(
            "Fail to run CallbackThread - CallbackThread.get(%p), status[%s(%d)]",
            pCallbackThread.get(), ::strerror(-status), -status
        );
        goto lbExit;
    }
    mpZipCallbackThread = pCallbackThread;
    pCallbackThread = 0;
    //
lbExit:
    return status;
}


/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
waitForShotDone()
{
    sp<ZipImageCallbackThread> pCallbackThread = mpZipCallbackThread.promote();
    //  wait for capture callback is done
    if  ( pCallbackThread.get() )
    {
        //to make sure thread can exit
        pCallbackThread->doCallback(ZipImageCallbackThread::callback_type_exit);
        MY_LOGD("wait for capture callback done +");
        pCallbackThread->join();
        MY_LOGD("wait for capture callback done -");
        pCallbackThread = 0;
        mpZipCallbackThread = 0;
    }
    return true;
}


/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
isTakingPicture() const
{
    bool ret =  mpStateManager->isState(IState::eState_PreCapture) ||
                mpStateManager->isState(IState::eState_Capture) ||
                mpStateManager->isState(IState::eState_PreviewCapture) ||
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
 ******************************************************************************/
bool
CamAdapter::
main2zsd()
{
    MY_LOGD("+");
    if(mpDefaultCtrlNode_Main2)
    {
        mpDefaultCtrlNode_Main2->stopPreview();
    }
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
void*
CamAdapter::
doThreadZsd(void* arg)
{
    ::prctl(PR_SET_NAME,"zsdThread", 0, 0, 0);
    CamAdapter* pSelf = reinterpret_cast<CamAdapter*>(arg);
    return (void*)pSelf->main2zsd();
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
takePicture()
{
    status_t status = OK;
    //
    // wait for previous callback is done
    waitForShotDone();
    //
    if( OK != (status = prepareToCapture()) )
    {
        MY_LOGE("failed to prepareToCapture");
        goto lbExit;
    }
    //
    if( !mpDefaultCtrlNode->updateShotMode() )
    {
        MY_LOGE("update shot mode failed");
        status = INVALID_OPERATION;
        goto lbExit;
    }
    //
    mShotMode = mpDefaultCtrlNode->getShotMode();
    mbTakePicPrvNotStop = false;
    //
    {
        if(mShotMode == eShotMode_ZsdShot)
        {
            // Do Not uncomment the followings since we use syncNode to manage capBuf
            // DCNode will never satisfy mPostBufCount != mRetBufCount which causes dead lock

            // void* threadRet = NULL;
            // if( pthread_create(&mThreadHandle, NULL, doThreadZsd, this) != 0 )
            // {
            //     MY_LOGE("pthread create failed");
            //     goto lbExit;
            // }
            // if(mpDefaultCtrlNode)
            // {
            //     mpDefaultCtrlNode->stopPreview();
            // }
            // if( pthread_join(mThreadHandle, &threadRet) != 0 )
            // {
            //     MY_LOGE("pthread join fail");
            //     goto lbExit;
            // }
            if(mpSyncNode){
                mpSyncNode->stopCapBuf();
            }
            // (1) uninit algorithm
            if(mpStereoCtrlNode)
            {
                mpStereoCtrlNode->uninit();
            }
        }
        else
        {
            MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: Pre-cpture +");
            status = mpStateManager->getCurrentState()->onPreCapture(this);
            MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: Pre-cpture -");
            if(OK != status)
            {
                goto lbExit;
            }
            //
            MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: Stop preview +");
            status = mpStateManager->getCurrentState()->onStopPreview(this);
            MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: Stop preview -");
            if(OK != status)
            {
                goto lbExit;
            }

            if(mpResMgrDrv!=NULL)
            {
                MY_LOGD("Switch MMDVFS to High");
                mpResMgrDrv->setItem(ResMgrDrv::ITEM_STEREO, 1);
                mpResMgrDrv->triggerCtrl();
                mpHwSyncDrv-> sendCommand (HW_SYNC_CMD_UNDATE_CLOCK, 0x0, 0x0, 0x0, HW_SYNC_CLOCK_STAGE_1);
            }
        }
    }
    //
    MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: start capture +");
    status = mpStateManager->getCurrentState()->onCapture(this);
    MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: start capture -");
    //
    if(OK != status)
    {
        goto lbExit;
    }
    //
lbExit:
    return status;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
cancelPicture()
{
    if( mpStateManager->isState(IState::eState_Capture) ||
        mpStateManager->isState(IState::eState_PreviewCapture) ||
        mpStateManager->isState(IState::eState_VideoSnapshot))
    {
        mpStateManager->getCurrentState()->onCancelCapture(this);
    }
    return OK;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
setCShotSpeed(int32_t i4CShotSpeed)
{
    if(i4CShotSpeed <= 0 )
    {
        MY_LOGE("cannot set continuous shot speed as %d fps)", i4CShotSpeed);
        return BAD_VALUE;
    }

    sp<IShot> pShot = mpShot;
    if( pShot != 0 )
    {
        pShot->sendCommand(eCmd_setCShotSpeed, i4CShotSpeed, 0);
    }

    return OK;
}


/******************************************************************************
*   CamAdapter::takePicture() -> IState::onCapture() ->
*   IStateHandler::onHandleCapture() -> CamAdapter::onHandleCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandleCapture()
{
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
    return  status;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
onHandleCaptureDone()
{
    if(mpResMgrDrv)
    {
        ResMgrDrv::MODE_STRUCT mode;
        mpResMgrDrv->getMode(&mode);
        mode.scenSw = ResMgrDrv::SCEN_SW_CAM_IDLE;
        mpResMgrDrv->setMode(&mode);

        if(!mbZsdMode)
        {
            if(mpResMgrDrv!=NULL)
            {
                MY_LOGD("Switch MMDVFS to Low");
                mpResMgrDrv->setItem(ResMgrDrv::ITEM_STEREO, 0);
                mpResMgrDrv->triggerCtrl();
                mpHwSyncDrv-> sendCommand (HW_SYNC_CMD_UNDATE_CLOCK, 0x0, 0x0, 0x0, HW_SYNC_CLOCK_STAGE_0);
            }
        }
    }
    //
    mpStateManager->transitState(IState::eState_Idle);
    return  OK;
}


/******************************************************************************
*   CamAdapter::cancelPicture() -> IState::onCancelCapture() ->
*   IStateHandler::onHandleCancelCapture() -> CamAdapter::onHandleCancelCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandleCancelCapture()
{
    MY_LOGD("");
    sp<IShot> pShot = mpShot;
    if  ( pShot != 0 )
    {
        pShot->sendCommand(eCmd_cancel);
    }
    //
    return  OK;
}


/******************************************************************************
*   CamAdapter::takePicture() -> IState::onCapture() ->
*   IStateHandler::onHandleCapture() -> CamAdapter::onHandleCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandlePreviewCapture()
{
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
    pCallbackThread->setShotMode(mShotMode, IState::eState_Preview);
    pCallbackThread = 0;
    //
    if  ( pCaptureCmdQueThread != 0 ) {
        status = pCaptureCmdQueThread->onCapture();
    }
    //
lbExit:
    return  status;

}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
onHandlePreviewCaptureDone()
{
    if(mpResMgrDrv)
    {
        ResMgrDrv::MODE_STRUCT mode;
        mpResMgrDrv->getMode(&mode);
        mode.scenSw = ResMgrDrv::SCEN_SW_ZSD;
        mpResMgrDrv->setMode(&mode);
    }
    // (1) init algorithm
    if(mpStereoCtrlNode)
    {
        mpStereoCtrlNode->init();
    }
    // (2) enable preview
    if(mpDefaultCtrlNode)
    {
        mpDefaultCtrlNode->startPreview();
    }
    if(mpDefaultCtrlNode_Main2)
    {
        mpDefaultCtrlNode_Main2->startPreview();
    }
    if(mpSyncNode){
        mpSyncNode->startCapBuf();
    }
    //
    mpStateManager->transitState(IState::eState_Preview);
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
    return  OK;
}


/******************************************************************************
*
*
*******************************************************************************/
status_t
CamAdapter::
onHandleVideoSnapshotDone()
{
    if(mpResMgrDrv)
    {
        ResMgrDrv::MODE_STRUCT mode;
        mpResMgrDrv->getMode(&mode);
        if(mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) <= 30)
        {
            mode.scenSw = ResMgrDrv::SCEN_SW_VIDEO_REC;
        }
        else
        {
            mode.scenSw = ResMgrDrv::SCEN_SW_VIDEO_REC_HS;
        }
        mpResMgrDrv->setMode(&mode);
    }
    //
    mpStateManager->transitState(IState::eState_Recording);
    return  OK;
}


/******************************************************************************
*
*
*******************************************************************************/
status_t
CamAdapter::
onHandleCancelVideoSnapshot()
{
    //TBD
    return  OK;
}


/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
onCaptureThreadLoop()
{
    MY_LOGD("+");
    bool ret = false;
    MUINT32 transform = 0;
    ResMgrDrv::MODE_STRUCT mode;
    //
    if(mpResMgrDrv)
    {
        mpResMgrDrv->getMode(&mode);
    }
    //
    //  [1] transit to "Capture" state.
    if(mpStateManager->isState(IState::eState_Idle))
    {
        mpStateManager->transitState(IState::eState_Capture);
        mode.scenSw = ResMgrDrv::SCEN_SW_CAM_CAP;
    }
    else
    if(mpStateManager->isState(IState::eState_Preview))
    {
        mpStateManager->transitState(IState::eState_PreviewCapture);
        mode.scenSw = ResMgrDrv::SCEN_SW_ZSD;
    }
    else
    if(mpStateManager->isState(IState::eState_Recording))
    {
        mpStateManager->transitState(IState::eState_VideoSnapshot);
        mode.scenSw = ResMgrDrv::SCEN_SW_VIDEO_VSS;
    }
    //
    if(mpResMgrDrv)
    {
        mpResMgrDrv->setMode(&mode);
    }
    //
    {
        //
        //  [2.1] update mpShot instance.
        ret = updateShotInstance();
        sp<IShot> pShot = mpShot;
        //
        //  [2.2] return if no shot instance.
        if  ( ! ret || pShot == 0 )
        {
            #warning "[TODO] perform a dummy compressed-image callback or CAMERA_MSG_ERROR to inform app of end of capture?"
            MY_LOGE("updateShotInstance(%d), pShot.get(%p)", ret, pShot.get());
            goto lbExit;
        }
        else
        {
#define parse_pair(string, first, second, delimeter)                        \
        do {                                                                \
            char *str = (char *)string;                                     \
            char delim = (char)delimeter;                                   \
            char *end;                                                      \
            int w = (int)strtol(string, &end, 10);                          \
            if (*end != delim) {                                            \
                CAM_LOGE("Cannot find (%c) in str=%s", (char)delim, str);   \
                return -1;                                                  \
            }                                                               \
            int h = (int)strtol(end+1, &end, 10);                           \
            *first = w;                                                     \
            *second = h;                                                    \
        }while(0)

            //  [3.1] prepare parameters
            sp<IParamsManager> pParamsMgr = getParamsManager();

            // String8 s8RefocusSize   = pParamsMgr->getStr(MtkCameraParameters::KEY_REFOCUS_PICTURE_SIZE); // For APP, Definition of 5M 4:3
            // String8 s8StereoSize    = pParamsMgr->getStr(MtkCameraParameters::KEY_STEREO_PICTURE_SIZE);  //(1920+256)x2, 1440+256(16-align) // JPS
            // int testWidth = 0, testHeight = 0;
            // parse_pair(s8RefocusSize.string(), &testWidth, &testHeight, 'x');
            // MY_LOGD("Shane 201412281657 onCaptureThreadLoop 5");
            // MY_LOGD("testWidth=%d, testHeight=%d", testWidth, testHeight);

            int iPictureWidth = 0, iPictureHeight = 0;
            int iPreviewWidth = 0, iPreviewHeight = 0;
            int iJpsWidth = 0, iJpsHeight = 0;
            pParamsMgr->getPreviewSize(&iPreviewWidth, &iPreviewHeight);
            String8 s8DisplayFormat = mpImgBufProvidersMgr->queryFormat(IImgBufProvider::eID_DISPLAY);
            if  ( String8::empty() == s8DisplayFormat ) {
                MY_LOGW("Display Format is empty");
            }
            MY_LOGD("onCaptureThreadLoop iPreviewWidth=%d, iPreviewHeight=%d",iPreviewWidth,iPreviewHeight);

            // Shane 20150121
            // Query Proper ratio settings from stereo_hal_base
            MY_LOGD("prvSize %dx%d", iPreviewWidth, iPreviewHeight);
            if(iPreviewWidth*3 == iPreviewHeight*4){
                MY_LOGD("prvSize ratio is 4:3");
                StereoSettingProvider::setImageRatio(eRatio_4_3);
            }else if(iPreviewWidth*9 == iPreviewHeight*16){
                MY_LOGD("prvSize ratio is 16:9");
                StereoSettingProvider::setImageRatio(eRatio_16_9);
            }else{
                MY_LOGE("prvSize ratio is not supported. use 16:9 instead");
                StereoSettingProvider::setImageRatio(eRatio_16_9);
            }

            STEREO_PARAMS_T stereoParams;
            MINT32 sensorId_main   = 0;
            MINT32 sensorId_main2  = 0;
            //
            StereoSettingProvider::getStereoSensorIndex(sensorId_main, sensorId_main2);
            MY_LOGD("onCaptureThreadLoop : SensorId_Main =  %d ",sensorId_main);
            MY_LOGD("onCaptureThreadLoop : SensorId_Main2 =  %d ",sensorId_main2);

            StereoSettingProvider::getStereoParams(stereoParams);
            parse_pair(stereoParams.refocusSize, &iPictureWidth, &iPictureHeight, 'x');
            parse_pair(stereoParams.jpsSize, &iJpsWidth, &iJpsHeight, 'x');

            MY_LOGD("stereo_hal_jps(%dx%d), stereo_hal_picture(%dx%d)", iJpsWidth, iJpsHeight, iPictureWidth, iPictureHeight);

            //

            bool setCapBufMgr = MFALSE;

            if( mbZsdMode )
            {
                setCapBufMgr = MTRUE;
            }
            MY_LOGD("mbZsdMode=%d, setCapBufMgr=%d", mbZsdMode, setCapBufMgr);


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

            // Shane modify for sub camera mirror
            // Use following code to adopt mirror settings set by APP
            // MINT32 mirror =1;
            // switch(rotation)
            // {
            //     case 0:
            //         if(mirror == 1)
            //             transform = eTransform_FLIP_H; // 0x01
            //         else
            //             transform = 0;                 // 0x00
            //         break;
            //     case 90:
            //         if(mirror == 1)
            //             transform = eTransform_FLIP_V |eTransform_ROT_90; // 0x06
            //         else
            //             transform = eTransform_ROT_90;                    // 0x04
            //         break;
            //     case 180:
            //         if(mirror == 1)
            //             transform = eTransform_FLIP_V;                    // 0x02
            //         else
            //             transform = eTransform_ROT_180;                   // 0x03
            //         break;
            //     case 270:
            //         if(mirror ==1 )
            //             transform = eTransform_FLIP_H|eTransform_ROT_90;  // 0x05
            //         else
            //             transform = eTransform_ROT_270;                   // 0x07
            //         break;
            //     default:
            //         break;
            // }
            // MY_LOGD("Stereo debug capture rotation is  %d  mirror is %d", rotation, mirror);
            MY_LOGD("Stereo debug capture transform is %d ", transform);

            //  [3.2] prepare parameters: ShotParam
            ShotParam shotParam;
            shotParam.miPictureFormat           = NSCam::Utils::Format::queryImageFormat(pParamsMgr->getStr(CameraParameters::KEY_PICTURE_FORMAT));
            // if  ( 0 != iPictureWidth && 0 != iPictureHeight )
            // {
            //     shotParam.mi4PictureWidth       = iPictureWidth;
            //     shotParam.mi4PictureHeight      = iPictureHeight;
            // }
            // else
            // {   //  When 3rd-party apk sets "picture-size=0x0", replace it with "preview-size".
            //     shotParam.mi4PictureWidth       = iPreviewWidth;
            //     shotParam.mi4PictureHeight      = iPreviewHeight;
            // }
            shotParam.mi4PictureWidth       = iPictureWidth;
            shotParam.mi4PictureHeight      = iPictureHeight;

            shotParam.miPostviewDisplayFormat   = NSCam::Utils::Format::queryImageFormat(s8DisplayFormat.string());
            shotParam.miPostviewClientFormat    = NSCam::Utils::Format::queryImageFormat(pParamsMgr->getStr(CameraParameters::KEY_PREVIEW_FORMAT));
            shotParam.mi4PostviewWidth          = iPreviewWidth;
            shotParam.mi4PostviewHeight         = iPreviewHeight;
            shotParam.ms8ShotFileName           = pParamsMgr->getStr(MtkCameraParameters::KEY_CAPTURE_PATH);
            shotParam.mu4ZoomRatio              = pParamsMgr->getZoomRatio();
            shotParam.mu4ShotCount              = pParamsMgr->getInt(MtkCameraParameters::KEY_BURST_SHOT_NUM);
            shotParam.mu4Transform              = transform;
            shotParam.mu4MultiFrameBlending     = pParamsMgr->getMultFrameBlending();
            shotParam.ms8StereoFileName         = pParamsMgr->getStr(MtkCameraParameters::KEY_REFOCUS_JPS_FILE_NAME);
            //
            //  [3.3] prepare parameters: JpegParam
            JpegParam jpegParam;
            jpegParam.mu4JpegQuality            = pParamsMgr->getInt(CameraParameters::KEY_JPEG_QUALITY);
            jpegParam.mu4JpegThumbQuality       = pParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY);
            jpegParam.mi4JpegThumbWidth         = pParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH);
            jpegParam.mi4JpegThumbHeight        = pParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT);
            jpegParam.ms8GpsLatitude            = pParamsMgr->getStr(CameraParameters::KEY_GPS_LATITUDE);
            jpegParam.ms8GpsLongitude           = pParamsMgr->getStr(CameraParameters::KEY_GPS_LONGITUDE);
            jpegParam.ms8GpsAltitude            = pParamsMgr->getStr(CameraParameters::KEY_GPS_ALTITUDE);
            jpegParam.ms8GpsTimestamp           = pParamsMgr->getStr(CameraParameters::KEY_GPS_TIMESTAMP);
            jpegParam.ms8GpsMethod              = pParamsMgr->getStr(CameraParameters::KEY_GPS_PROCESSING_METHOD);
            // parse_pair(s8StereoSize.string(), &jpegParam.mi4JpsWidth, &jpegParam.mi4JpsHeight, 'x');
            jpegParam.mi4JpsWidth = iJpsWidth;
            jpegParam.mi4JpsHeight = iJpsHeight;
            MY_LOGD("jpegParam.mu4JpegQuality= %d",      jpegParam.mu4JpegQuality);
            MY_LOGD("jpegParam.mu4JpegThumbQuality= %d", jpegParam.mu4JpegThumbQuality);
            //  [4.1] perform Shot operations.
            if(!mbTakePicPrvNotStop)
            {
                mpDefaultBufHdl->mapPort(DefaultBufHandler::eBuf_Disp, PASS2_PRV_DST_0, 60*100, MTRUE);
                mpDefaultBufHdl->mapPort(DefaultBufHandler::eBuf_Disp, PASS2_CAP_DST_0, 60*100);
            }
            //
            //check if raw16 CB is enable
            if( mEnRaw16CB == 1 )//CAMERA_CMD_ENABLE_RAW16_CALLBACK
            {
                MY_LOGD("adapter.capture: mEnRaw16CB == 1 to enableDataMsg(NSShot::EIShot_DATA_MSG_RAW)");
                pShot->enableDataMsg(NSShot::EIShot_DATA_MSG_RAW);
            }

            ret =
                    pShot->sendCommand(eCmd_reset)
                &&  pShot->setCallback(this)
                &&  pShot->sendCommand(eCmd_setShotParam, (MUINTPTR)&shotParam, sizeof(ShotParam))
                &&  pShot->sendCommand(eCmd_setJpegParam, (MUINTPTR)&jpegParam, sizeof(JpegParam))
                &&  pShot->sendCommand(eCmd_setPrvBufHdl, (MUINTPTR)mpDefaultBufHdl, sizeof(DefaultBufHandler))
                &&  ( setCapBufMgr ?
                        pShot->sendCommand(eCmd_setCapBufMgr, (MUINTPTR)mpCapBufMgr, sizeof(CapBufMgr), (MUINTPTR)mpCapBufMgr_Main2) :
                        MTRUE )
                &&  pShot->sendCommand(eCmd_capture)
                    ;
            if  ( ! ret )
            {
                MY_LOGE("fail to perform shot operations");
            }
            //
            if(!mbTakePicPrvNotStop)
            {
                mpDefaultBufHdl->unmapPort(DefaultBufHandler::eBuf_Disp);
            }
#undef parse_pair
        }
        //
        //
        lbExit:
        //
        //  [5.1] uninit shot instance.
        MY_LOGD("free shot instance: (mpShot/pShot)=(%p/%p)", mpShot.get(), pShot.get());
        mpShot = NULL;
        pShot  = NULL;
        //
        //  [5.2] notify capture done.
    }
    //
    MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: capture done +");
    mpStateManager->getCurrentState()->onCaptureDone(this);
    MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: capture done -");
    //
    //
    MY_LOGD("-");
    return  true;
}

