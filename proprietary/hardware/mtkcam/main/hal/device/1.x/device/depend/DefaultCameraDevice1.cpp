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

#include <sys/prctl.h>
//
#if '1'== MTKCAM_HAVE_CAM_MANAGER
#include <mtkcam/utils/hw/CamManager.h>
#endif
#include <mtkcam/utils/hw/HwInfoHelper.h>
//
#if '1'== MTKCAM_HR_MONITOR_SUPPORT
#include <mtkcam/utils/fwk/MtkCamera.h>
#endif
//
#include <mtkcam/middleware/v1/camutils/FrameworkCBThread.h>
using namespace android::MtkCamUtils;
//
#include "MyUtils.h"
#include "DefaultCameraDevice1.h"
//
#if (MTKCAM_HAVE_AEE_FEATURE == 1)
#include <aee.h>
#endif
#if (MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1)
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#endif

using namespace android;
using namespace NSCam;
using namespace NSCam::Utils;

#define MAX_POWERON_COUNT 2

 /******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("%d[DefaultCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("%d[DefaultCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("%d[DefaultCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("%d[DefaultCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("%d[DefaultCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("%d[DefaultCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("%d[DefaultCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)

     //
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

/******************************************************************************
 *
 ******************************************************************************/
extern "C"
NSCam::CameraDevice1Base*
createCameraDevice1_Default(
    ICameraDeviceManager* deviceManager,
    std::string           instanceName,
    int32_t               instanceId,
    android::String8 const& rDevName
)
{
    return new DefaultCameraDevice1(deviceManager
                                  , instanceName
                                  , instanceId
                                  , rDevName);

}

/******************************************************************************
 *
 ******************************************************************************/
DefaultCameraDevice1::
DefaultCameraDevice1(
    ICameraDeviceManager* deviceManager,
    std::string instanceName,
    int32_t instanceId,
    android::String8 const& rDevName

)
    : CameraDevice1Base(deviceManager, instanceName, instanceId, rDevName)
    //
#if '1'==MTKCAM_HAVE_3A_HAL
    , mpHal3a(NULL)
#endif
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    , mpHalSensor(NULL)
#endif
    //
    , mThreadHandle(0)
    , mbThreadRunning(MFALSE)
    , mRet(false)
    , mDisableWaitSensorThread(false)
    //
{
    //[TODO] Remove the debug messages
    MY_LOGV("MTKCAM_HAVE_SENSOR_HAL: %c\nMTKCAM_HAVE_3A_HAL: %c\nMTKCAM_HAVE_CAM_MANAGER: %c\n",
             MTKCAM_HAVE_SENSOR_HAL,
             MTKCAM_HAVE_3A_HAL,
             MTKCAM_HAVE_CAM_MANAGER);
}


/******************************************************************************
 *
 ******************************************************************************/
DefaultCameraDevice1::
~DefaultCameraDevice1()
{
    if( !waitThreadSensorOnDone() )
    {
        MY_LOGE("Sensor_power_on thread failed");
    }
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
DefaultCameraDevice1::
onOpenLocked()
{
    MY_LOGD("+");

    //[1] Init platform pofile
    //--------------------------------------------------------------------------
    CAM_TRACE_NAME("init(profile)");
    CamProfile  profile(__FUNCTION__, "DefaultCam1Device");
    if(NSCam::Utils::CamManager::getInstance()->getSensorCount() > 1)
    {
        MY_LOGE("CamManager.getSensorCount() > 1, close them before open default mode.");
        return UNKNOWN_ERROR;
    }
    //[2] Check ISP Resource
    //--------------------------------------------------------------------------
    MBOOL isPowerOnSuccess = MTRUE;
    NSCamHW::HwInfoHelper helper(mInstanceId);
    if(mDeviceManager->getOpenedCameraNum()>1){
        helper.getSensorPowerOnPredictionResult(isPowerOnSuccess);
        if(!isPowerOnSuccess){
            MY_LOGE("sensor power on predicted to be fail, ISP resource is not enough");
            return -EUSERS;
        }
    }
    //[3] Init and power on sensor
    //--------------------------------------------------------------------------
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    {
        CAM_TRACE_NAME("init(sensor power on)");
        if( pthread_create(&mThreadHandle, NULL, doThreadSensorOn, this) != 0 )
        {
            MY_LOGE("sensor power on thread create failed");
            profile.print("");
            return NO_INIT;
        }
        mbThreadRunning = MTRUE;

        // Workaround to yuv sensor, 3A initiation depends on sensor power-on finished
        if( MAKE_HalSensorList()->queryType( this->getInstanceId() ) == NSCam::NSSensorType::eYUV )
        {
            if( !waitThreadSensorOnDone() )
            {
                MY_LOGE("sensor power on thread create failed");
                profile.print("");
                return NO_INIT;
            }
        }
    }
#endif  //MTKCAM_HAVE_3A_HAL

    //[4] Create and open 3A HAL
    //--------------------------------------------------------------------------
#if '1'==MTKCAM_HAVE_3A_HAL
    {
        CAM_TRACE_NAME("init(3A)");

        mpHal3a = MAKE_Hal3A(
                this->getInstanceId(),
                LOG_TAG);
        if  ( ! mpHal3a ) {
            MY_LOGE("IHal3A::createInstance() fail");
            profile.print("");
            return NO_INIT;
        }
        mpHal3a->notifyPwrOn();
        profile.print("3A Hal -");
    }
#endif  //MTKCAM_HAVE_3A_HAL

    //[5] Devcie Base onOpenLocked scenario
    //--------------------------------------------------------------------------
    {
        CAM_TRACE_NAME("init(DevcieBase on Open)");
        if  ( OK != CameraDevice1Base::onOpenLocked())
        {
            MY_LOGE("CameraDevice1Base onOpenLocked failed");
            profile.print("");
            return NO_INIT;
        }
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
void
DefaultCameraDevice1::
onCloseLocked()
{
    MY_LOGD("+");
    CamProfile  profile(__FUNCTION__, "DefaultCameraDevice1");
    //
    if( !waitThreadSensorOnDone() )
    {
        MY_LOGE("sensor power on thread create failed");
    }

    //  (1) Uninit Base
    //--------------------------------------------------------------------------
    {
        CAM_TRACE_NAME("uninit(base)");
        CameraDevice1Base::onCloseLocked();
        profile.print("CameraDevice1Base::onCloseLocked() -");
    }

    //  (2) Close 3A
    //--------------------------------------------------------------------------
#if '1'==MTKCAM_HAVE_3A_HAL
    {
        CAM_TRACE_NAME("uninit(3A)");
        if  ( mpHal3a )
        {
            mpHal3a->notifyPwrOff();
            mpHal3a->destroyInstance(LOG_TAG);
            mpHal3a = NULL;
        }
        profile.print("3A Hal -");
    }
#endif  //MTKCAM_HAVE_3A_HAL

    //  (3) Close Sensor
    //--------------------------------------------------------------------------
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    {
        CAM_TRACE_NAME("uninit(sensor)");
        //
        if(mpHalSensor)
        {
            uint32_t sensorIdx = this->getInstanceId();
            mpHalSensor->powerOff(USER_NAME, 1, &sensorIdx);
            NSCam::Utils::CamManager::getInstance()->decSensorCount(LOG_TAG);
            mpHalSensor->destroyInstance(USER_NAME);
            mpHalSensor = NULL;
        }
        MY_LOGD("SensorHal::destroyInstance()");
        profile.print("Sensor Hal -");
    }
#endif  //MTKCAM_HAVE_SENSOR_HAL
    //

#if (1 == STEREO_CAMERA_SUPPORTED)
    // reset stereo feature mode
    MY_LOGD("reset stereo feature mode");
    StereoSettingProvider::setStereoFeatureMode(0);
    StereoSettingProvider::setStereoShotMode(0);
    StereoSettingProvider::setStereoModuleType(0);
    StereoSettingProvider::setStereoProfile((ENUM_STEREO_SENSOR_PROFILE)0);
#endif

    profile.print("");
    MY_LOGD("-");
    return;
}

/******************************************************************************
 *
 *
 ******************************************************************************/
int
DefaultCameraDevice1::
awbModeStringToEnum(const char *awbMode) {
    return
        !awbMode ?
            MTK_CONTROL_AWB_MODE_AUTO :
        !strcmp(awbMode, CameraParameters::WHITE_BALANCE_AUTO) ?
            MTK_CONTROL_AWB_MODE_AUTO :
        !strcmp(awbMode, CameraParameters::WHITE_BALANCE_INCANDESCENT) ?
            MTK_CONTROL_AWB_MODE_INCANDESCENT :
        !strcmp(awbMode, CameraParameters::WHITE_BALANCE_FLUORESCENT) ?
            MTK_CONTROL_AWB_MODE_FLUORESCENT :
        !strcmp(awbMode, CameraParameters::WHITE_BALANCE_WARM_FLUORESCENT) ?
            MTK_CONTROL_AWB_MODE_WARM_FLUORESCENT :
        !strcmp(awbMode, CameraParameters::WHITE_BALANCE_DAYLIGHT) ?
            MTK_CONTROL_AWB_MODE_DAYLIGHT :
        !strcmp(awbMode, CameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT) ?
            MTK_CONTROL_AWB_MODE_CLOUDY_DAYLIGHT :
        !strcmp(awbMode, CameraParameters::WHITE_BALANCE_TWILIGHT) ?
            MTK_CONTROL_AWB_MODE_TWILIGHT :
        !strcmp(awbMode, CameraParameters::WHITE_BALANCE_SHADE) ?
            MTK_CONTROL_AWB_MODE_SHADE :
        -1;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
DefaultCameraDevice1::
onStartPreview()
{
    //  (0) wait for thread
    //--------------------------------------------------------------------------
    if(!mDisableWaitSensorThread)
    {
        if( !waitThreadSensorOnDone() )
        {
            MY_LOGE("init in thread failed");
            return false;
        }
    }

    //  (1) Check Permission.
    //--------------------------------------------------------------------------
#if '1'== MTKCAM_HAVE_CAM_MANAGER
    //  Check Permission.
    CamManager* pCamMgr = CamManager::getInstance();
    if ( ! pCamMgr->getPermission() )
    {
        MY_LOGE("Cannot start preview ... Permission denied");
        return false;
    }
#endif

    //  (2) Update Hal App Mode.
    //--------------------------------------------------------------------------
    if  ( ! mpParamsMgr->updateHalAppMode() )
    {
        MY_LOGE("mpParamsMgr->updateHalAppMode() fail");
        return false;
    }

    //  (3) Initialize Camera Adapter.
    //--------------------------------------------------------------------------
    if  ( ! initCameraAdapter() )
    {
        MY_LOGE("NULL Camera Adapter");
        return false;
    }

    //  (4) Initialize Camera Awb Mode.
    //--------------------------------------------------------------------------
#if '1'==MTKCAM_HAVE_3A_HAL
   if (mpHal3a)
     {
         using namespace NS3Av3;
         MINT32 newu4AwbMode;

         newu4AwbMode = (MINT32)awbModeStringToEnum(
            (mpParamsMgr->getStr(MtkCameraParameters::KEY_WHITE_BALANCE)).c_str() );

         MY_LOGD("E3ACtrl_T(%#x) update newAwbMode(%d) to 3A.\n", E3ACtrl_SetAwbMode, newu4AwbMode);
         mpHal3a->send3ACtrl(E3ACtrl_SetAwbMode, (MINTPTR)newu4AwbMode, 0);
     }
#endif

    return true;
}

/******************************************************************************
 *  [Template method] Called by stopPreview().
 ******************************************************************************/
void
DefaultCameraDevice1::
onStopPreview()
{
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->cancelPicture();
        mpCamAdapter->uninit();
        mpCamAdapter.clear();
    }
}


/******************************************************************************
 * [Template method] Called by startRecording().
 ******************************************************************************/
bool
DefaultCameraDevice1::
onStartRecording()
{
#if '1'== MTKCAM_HAVE_CAM_MANAGER
    //  (1) Check Permission.
    if ( CamManager::getInstance()->isMultiDevice() )
    {
        MY_LOGE("Cannot start recording ... Permission denied");
        sp<IFrameworkCBThread> spFrameworkCBThread = IFrameworkCBThread::createInstance(getInstanceId(),mpCamMsgCbInfo);
        spFrameworkCBThread->init();
        IFrameworkCBThread::callback_data cbData;
        cbData.callbackType = IFrameworkCBThread::CALLBACK_TYPE_NOTIFY;
        cbData.type         = CAMERA_MSG_ERROR;
        cbData.ext1         = CAMERA_ERROR_SERVER_DIED;
        cbData.ext2         = 0;
        spFrameworkCBThread->postCB(cbData);
        spFrameworkCBThread->uninit();
        spFrameworkCBThread = NULL;
        return false;
    }
#endif
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
DefaultCameraDevice1::
powerOnSensor()
{
    MY_LOGD("+");

#if '1'==MTKCAM_HAVE_SENSOR_HAL
    CAM_TRACE_CALL();
    CamProfile  profile(__FUNCTION__, "DefaultCameraDevice1");
    //
    uint32_t sensorIdx = this->getInstanceId();
    NSCam::Utils::CamManager::getInstance()->incSensorCount(LOG_TAG);
    NSCam::IHalSensorList* pHalSensorList = MAKE_HalSensorList();
    if(!pHalSensorList)
    {
        MY_LOGE("pHalSensorList == NULL");
        return false;
    }
    //
    mpHalSensor = pHalSensorList->createSensor(
                                        USER_NAME,
                                        this->getInstanceId());
    if(mpHalSensor == NULL)
    {
       MY_LOGE("mpHalSensor is NULL");
       return false;
    }
    //
    sensorIdx = this->getInstanceId();
    if( !mpHalSensor->powerOn(USER_NAME, 1, &sensorIdx) )
    {
        MY_LOGE("Sensor power on failed: %d", sensorIdx);
        NSCam::Utils::CamManager::getInstance()->decSensorCount(LOG_TAG);
#if (MTKCAM_HAVE_AEE_FEATURE == 1)
        aee_system_exception(
            LOG_TAG,
            NULL,
            DB_OPT_DEFAULT,
            "\nCRDISPATCH_KEY:MtkCam/Cam1Device:Sensor power on failed");
#endif
        return false;
    }
    //
    profile.print("Sensor Hal -");
#endif  //MTKCAM_HAVE_SENSOR_HAL

    int32_t diffTimeUs = profile.getDiffTime()/1000;
    MY_LOGD("diffTimeUs %d",diffTimeUs);
    if(diffTimeUs > 3000)
    {
#if (MTKCAM_HAVE_AEE_FEATURE == 1)
                aee_system_exception(
                    LOG_TAG,
                    NULL,
                    DB_OPT_DEFAULT,
                    "\nCRDISPATCH_KEY:MtkCam/Cam1Device:Sensor power on over 3s");
#endif
    }

    MY_LOGD("-");
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
void*
DefaultCameraDevice1::
doThreadSensorOn(void* arg)
{
    ::prctl(PR_SET_NAME,"initCamdevice", 0, 0, 0);
    DefaultCameraDevice1* pSelf = reinterpret_cast<DefaultCameraDevice1*>(arg);
    pSelf->mRet = pSelf->powerOnSensor();
    pSelf->mDeviceManager->updatePowerOnDone();
    pthread_exit(NULL);
    return NULL;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
DefaultCameraDevice1::
waitThreadSensorOnDone()
{
    if( mbThreadRunning )
    {
        MY_LOGD("wait sensor power on done +");
        int s = pthread_join(mThreadHandle, NULL);
        MY_LOGD("wait sensor power on done -");
        mbThreadRunning = MFALSE;
        if( s != 0 )
        {
            MY_LOGE("sensor_power_on thread join error: %d", s);
            return false;
        }

        if( !mRet )
        {
            MY_LOGE("sensor power on failed");
            return false;
        }
    }
    return true;
}

/******************************************************************************
 *
 *
 ******************************************************************************/
bool
DefaultCameraDevice1::
disableWaitSensorThread(bool disable)
{
    MY_LOGD("disable(%d)",disable);
    mDisableWaitSensorThread = disable;
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
DefaultCameraDevice1::
onSetParameters(const char* params)
{
    (void)params;
    MY_LOGD("");
    status_t status = OK;

    //  Here (1) succeeded.
    //  (2) If CamAdapter exists, apply mpParamsMgr to CamAdapter;
    //      otherwise it will be applied when CamAdapter is created.
    //--------------------------------------------------------------------------
    {
#if 0 //TBD
#if '1'==MTKCAM_HAVE_3A_HAL
        else if ( mpHal3a )
        {
            //  Flashlight may turn on/off in case that CamAdapter doesn't exist (i.e. never call startPreview)

            using namespace NS3Av3;
            Param_T param;
            //
            if  ( ! mpHal3a->getParams(param) ) {
                MY_LOGW("3A Hal::getParams() fail - err(%x)", mpHal3a->getErrorCode());
            }
            //
            String8 const s8FlashMode = mpParamsMgr->getStr(CameraParameters::KEY_FLASH_MODE);
            if  ( ! s8FlashMode.isEmpty() ) {
                param.u4StrobeMode = PARAMSMANAGER_MAP_INST(eMapFlashMode)->valueFor(s8FlashMode);
            }
            //
            if  ( ! mpHal3a->setParams(param) ) {
                MY_LOGW("3A Hal::setParams() fail - err(%x)", mpHal3a->getErrorCode());
            }
        }
        else
        {
            MY_LOGW("mp3AHal==NULL");
        }
#endif
#endif
    }

#if '1' == MTKCAM_HR_MONITOR_SUPPORT
        {
            sp<ICamClient> pCamClient = mpCamClient;
            //if  ( pCamClient != 0 && INVALID_OPERATION != (status = pCamClient->sendCommand(CAMERA_CMD_CHECKPARA_HR_PREVIEW, 0, 0)) )
            if  ( pCamClient != 0 )
            {   //  we just return since this cammand has been handled.
                //return  status;
                pCamClient->sendCommand(CAMERA_CMD_CHECKPARA_HR_PREVIEW, 0, 0);
            }
        }
#endif

    return  status;
}
