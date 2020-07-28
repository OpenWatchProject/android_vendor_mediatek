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
#include <mtkcam/hwutils/CameraProfile.h>
//
#if '1'== MTKCAM_HAVE_CAM_MANAGER
#include <mtkcam/hwutils/CamManager.h>
#endif
//
#if '1'== MTKCAM_HR_MONITOR_SUPPORT
#include <mtkcam/utils/fwk/MtkCamera.h>
#endif
//
#include "MyUtils.h"
#include "DefaultCameraDevice1.h"
//
#include <mtkcam/v1/camutils/FrameworkCBThread.h>
using namespace android::MtkCamUtils;
//
#ifdef HAVE_AEE_FEATURE
#include <aee.h>
#define AEE_ASSERT(String)            \
             do {                      \
                 aee_system_exception( \
                     LOG_TAG,          \
                     NULL,             \
                     DB_OPT_DEFAULT,   \
                     String);          \
             } while(0)
#else
#define AEE_ASSERT(String)
#endif
//
using namespace android;
using namespace NSCam;
using namespace NSCam::Utils;

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
#if '1'==MTKCAM_HAVE_3A_HAL
    , mpHal3a(NULL)
#endif
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    , mpHalSensor(NULL)
#endif
    //
    , mThreadHandle(0)
    , mbThreadRunning(false)
    , mRet(false)
    , mDisableWaitSensorThread(false)
    //
{
    //[TODO] Remove the debug messages
    MY_LOGV("MTKCAM_HAVE_SENSOR_HAL: %c\nMTKCAM_HAVE_3A_HAL: %c\nMTKCAM_HAVE_CAMDRV: %c\nMTKCAM_HAVE_CPUCTRL: %c\nMTKCAM_HAVE_DEVMETAINFO: %c\nMTKCAM_HAVE_CAM_MANAGER: %c\n",
             MTKCAM_HAVE_SENSOR_HAL,
             MTKCAM_HAVE_3A_HAL,
             MTKCAM_HAVE_CAMDRV,
             MTKCAM_HAVE_CPUCTRL,
             MTKCAM_HAVE_DEVMETAINFO,
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

    //[1] Create and initialize CPU controller
    //--------------------------------------------------------------------------
    int policy, priority;
    getThreadPriority(policy,priority);
    setThreadPriority(SCHED_OTHER,0);

    //[2] Init platform pofile
    //--------------------------------------------------------------------------
    CAM_TRACE_NAME("init(profile)");
    initPlatformProfile();
    AutoCPTLog cptlog(Event_Hal_DefaultCamDevice_init);
    CamProfile  profile(__FUNCTION__, "DefaultCam1Device");

    //[3] Init resource
    //--------------------------------------------------------------------------
#if '1'==MTKCAM_HAVE_CAMDRV
    {
        CAM_TRACE_NAME("init(resource)");
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Resource +");
        IResManager* pResManager = IResManager::getInstance();
        //
        if  ( pResManager != NULL )
        {
            if(!(pResManager->open(USER_NAME)))
            {
                MY_LOGE("pResManager->open fail");
                setThreadPriority(policy,priority);
                profile.print("");
                return NO_INIT;
            }
        }
        //
        profile.print("Resource -");
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Resource -");
    }
#endif  //MTKCAM_HAVE_CAMDRV

    //[4] Init and power on sensor
    //--------------------------------------------------------------------------
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    {
        CAM_TRACE_NAME("init(sensor power on)");
        if( pthread_create(&mThreadHandle, NULL, doThreadSensorOn, this) != 0 )
        {
            MY_LOGE("sensor power on thread create failed");
            setThreadPriority(policy,priority);
            profile.print("");
            return NO_INIT;
        }
        mbThreadRunning = true;

        // Workaround to yuv sensor, 3A initiation depends on sensor power-on finished
        if( NSCam::IHalSensorList::get()->queryType( this->getInstanceId() ) == NSCam::NSSensorType::eYUV )
        {
            if( !waitThreadSensorOnDone() )
            {
                MY_LOGE("sensor power on thread create failed");
                setThreadPriority(policy,priority);
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
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "3A Hal +");

#if '1'==MTKCAM_HAVE_DEVMETAINFO
        int const iHalSensorDevId = android::MtkCamUtils::DevMetaInfo::queryHalSensorDev(this->getInstanceId());
        mpHal3a = NS3A::Hal3ABase::createInstance(iHalSensorDevId);
#else
        mpHal3a = NS3A::IHal3A::createInstance(
                NS3A::IHal3A::E_Camera_1,
                this->getInstanceId(),
                LOG_TAG);
#endif
        if  ( ! mpHal3a ) {
            MY_LOGE("IHal3A::createInstance() fail");
            setThreadPriority(policy,priority);
            profile.print("");
            return NO_INIT;
        }
        profile.print("3A Hal -");
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "3A Hal -");
    }
#endif  //MTKCAM_HAVE_3A_HAL

    //[5] Devcie Base onOpenLocked scenario
    //--------------------------------------------------------------------------
    {
        CAM_TRACE_NAME("init(DevcieBase on Open)");
        if  ( OK != CameraDevice1Base::onOpenLocked())
        {
            MY_LOGE("CameraDevice1Base onOpenLocked failed");
            setThreadPriority(policy,priority);
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
    AutoCPTLog cptlog(Event_Hal_DefaultCamDevice_uninit);
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
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "3A Hal +");
        if  ( mpHal3a )
        {
#if '1'==MTKCAM_HAVE_DEVMETAINFO
            mpHal3a->destroyInstance();
#else
            mpHal3a->destroyInstance(LOG_TAG);

#endif
            mpHal3a = NULL;
        }
        profile.print("3A Hal -");
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "3A Hal -");
    }
#endif  //MTKCAM_HAVE_3A_HAL

    //  (3) Close Sensor
    //--------------------------------------------------------------------------
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    {
        CAM_TRACE_NAME("uninit(sensor)");
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Sensor Hal +");
        //
        if(mpHalSensor)
        {
            uint32_t sensorIdx = this->getInstanceId();
            mpHalSensor->powerOff(USER_NAME, 1, &sensorIdx);
            mpHalSensor->destroyInstance(USER_NAME);
            mpHalSensor = NULL;
        }
        MY_LOGD("SensorHal::destroyInstance()");
        profile.print("Sensor Hal -");
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Sensor Hal -");
    }
#endif  //MTKCAM_HAVE_SENSOR_HAL

    //  (4) Close Resource
    //--------------------------------------------------------------------------
#if '1'==MTKCAM_HAVE_CAMDRV
    {
        CAM_TRACE_NAME("uninit(resource)");
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Resource +");
        IResManager* pResManager = IResManager::getInstance();
        //
        if  ( pResManager != NULL )
        {
            if(!(pResManager->close(USER_NAME)))
            {
                MY_LOGE("pResManager->close fail");
            }
        }
        profile.print("Resource -");
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Resource -");
    }
#endif  //MTKCAM_HAVE_CAMDRV

    //  (5) Close CPUCtrl
    //--------------------------------------------------------------------------
    //
    profile.print("");
    MY_LOGD("-");
    return;
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
        //Use FrameworkCBThread to do error notify callback
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
        MY_LOGW("Cannot start preview ... Permission denied");
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
    CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Sensor Hal +");
    //
    uint32_t sensorIdx = this->getInstanceId();
    NSCam::IHalSensorList* pHalSensorList = NSCam::IHalSensorList::get();
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
        AEE_ASSERT("\nCRDISPATCH_KEY:MtkCam/Cam1Device:Sensor power on failed");
        return false;
    }
    //
    profile.print("Sensor Hal -");
    CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Sensor Hal -");
#endif  //MTKCAM_HAVE_SENSOR_HAL

    int32_t diffTimeUs = profile.getDiffTime()/1000;
    MY_LOGD("diffTimeUs %d",diffTimeUs);
    if(diffTimeUs > 3000)
    {
#if (MTKCAM_HAVE_AEE_FEATURE == 1)
        AEE_ASSERT("\nCRDISPATCH_KEY:MtkCam/Cam1Device:Sensor power on over 3s");
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
        mbThreadRunning = false;
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
int32_t
DefaultCameraDevice1::
queryDisplayBufCount() const
{
    char value[PROPERTY_VALUE_MAX];
    property_get("ro.vendor.gmo.ram_optimize", value, "0");
    if (atoi(value)) {
        return 3;
    }
    return 6;
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
DefaultCameraDevice1::
setParameters(const hidl_string& params)
{
    status_t status = OK;

    //  (1) Update params to mpParamsMgr.
    //--------------------------------------------------------------------------
    status = mpParamsMgr->setParameters(String8(params.c_str()));
    if  ( OK != status ) {
        return  mapToHidlCameraStatus(status);
    }

    //  Here (1) succeeded.
    //  (2) If CamAdapter exists, apply mpParamsMgr to CamAdapter;
    //      otherwise it will be applied when CamAdapter is created.
    //--------------------------------------------------------------------------
    {
        sp<ICamAdapter> pCamAdapter = mpCamAdapter;
        if  ( pCamAdapter != 0 ) {
            status = pCamAdapter->setParameters();
        }
#if '1'==MTKCAM_HAVE_3A_HAL
        else if ( mpHal3a )
        {
            //  Flashlight may turn on/off in case that CamAdapter doesn't exist (i.e. never call startPreview)

            using namespace NS3A;
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

    return  mapToHidlCameraStatus(status);
}
