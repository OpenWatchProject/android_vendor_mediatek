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
#include <sys/prctl.h>
//
#if '1'== MTKCAM_HAVE_CAM_MANAGER
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/hw/CamManager.h>
#endif
//
#include <mtkcam/middleware/v1/camutils/FrameworkCBThread.h>
using namespace android::MtkCamUtils;
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/middleware/v1/PostProc/IPostProc.h>
//
#include "MyUtils.h"
#include "StereoCameraDevice1.h"
//
#if (MTKCAM_HAVE_AEE_FEATURE == 1)
#include <aee.h>
#endif
//
using namespace android;
using namespace NSCam;
using namespace StereoHAL;
using namespace NSCam::v1::Stereo;

#define THERMAL_DENOISE_POLICY_NAME "thermal_policy_04"
#define THERMAL_VSDOF_POLICY_NAME "thermal_policy_03"
#define MAX_CAPTURE_RETRY_COUNT (60)
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("%d[StereoCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("%d[StereoCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("%d[StereoCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("%d[StereoCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("%d[StereoCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("%d[StereoCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("%d[StereoCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
// ----------------------------------------------------------------------------
// function utility
// ----------------------------------------------------------------------------
// function scope
#define __DEBUG
#define __SCOPE_TIMER
#ifdef __DEBUG
#define FUNCTION_SCOPE      auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#include <memory>
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] +",pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}
#else
#define FUNCTION_SCOPE
#endif // function scope
/******************************************************************************
 *
 ******************************************************************************/
extern "C"
NSCam::CameraDevice1Base*
createCameraDevice1_MtkStereo(
    ICameraDeviceManager* deviceManager,
    std::string           instanceName,
    int32_t               instanceId,
    android::String8 const& rDevName
)
{
    return new StereoCameraDevice1(deviceManager, instanceName, instanceId, rDevName);
}


/******************************************************************************
 *
 ******************************************************************************/
StereoCameraDevice1::
StereoCameraDevice1(
    ICameraDeviceManager* deviceManager,
    std::string instanceName,
    int32_t instanceId,
    android::String8 const& rDevName
)
    : CameraDevice1Base(deviceManager, instanceName, instanceId, rDevName)
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
StereoCameraDevice1::
~StereoCameraDevice1()
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
StereoCameraDevice1::
onOpenLocked()
{
    FUNCTION_SCOPE;

    //[1] Init platform pofile
    CAM_TRACE_NAME("init(profile)");
    Utils::CamProfile  profile(__FUNCTION__, "StereoCameraDevice1");
    //
    status_t    ret = NO_INIT;
    if(!StereoSettingProvider::getStereoSensorIndex(mSensorId_Main, mSensorId_Main2))
    {
                MY_LOGE("Cannot get sensor ids from StereoSettingProvider!");
    }
    //
    /*if(NSCam::Utils::CamManager::getInstance()->getDeviceCount() > 0)
    {
        MY_LOGE("CamManager.getDeviceCount() > 0, close them before open stereo mode.");
        goto lbExit;
    }*/
    //[2] Init and power on sensor
    //--------------------------------------------------------------------------
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    //--------------------------------------------------------------------------
    {
        CAM_TRACE_NAME("init(sensor power on)");
        //  (0) power on sensor
        if( pthread_create(&mThreadHandle, NULL, doThreadSensorOn, this) != 0 )
        {
            MY_LOGE("sensor power on thread create failed");
                goto lbExit;
        }
        mbThreadRunning = MTRUE;
        //
        // workaround: yuv sensor, 3A depends on sensor power-on
        if( MAKE_HalSensorList()->queryType( getOpenId_Main() ) == NSCam::NSSensorType::eYUV ||
            MAKE_HalSensorList()->queryType( getOpenId_Main2() ) == NSCam::NSSensorType::eYUV )
        {
            if( !waitThreadSensorOnDone() )
            {
                MY_LOGE("init in thread failed");
                goto lbExit;
            }
        }
    }
#endif  //MTKCAM_HAVE_SENSOR_HAL
    //[2] Create and open 3A HAL
    //--------------------------------------------------------------------------
#if '1'==MTKCAM_HAVE_3A_HAL
    {
        CAM_TRACE_NAME("init(3A)");
#define INIT_3A(pHAL3A, OPENID)\
        do\
        {\
            pHAL3A = MAKE_Hal3A(\
                    OPENID,\
                    LOG_TAG);\
            if (!pHAL3A)\
            {\
                MY_LOGE("IHal3A::createInstance() fail" #pHAL3A);\
                goto lbExit;\
            }\
            pHAL3A->notifyPwrOn();\
        }while(0);
        INIT_3A(mpHal3a_Main, this->getInstanceId());
        INIT_3A(mpHal3a_Main2, getOpenId_Main2());
#undef RESTORE_STATIC_DATA

        profile.print("3A Hal -");
    }
#endif  //MTKCAM_HAVE_3A_HAL
    //[3] Devcie Base onOpenLocked scenario
    //--------------------------------------------------------------------------
    {
        CAM_TRACE_NAME("init(DevcieBase on Open)");
        if  ( OK != CameraDevice1Base::onOpenLocked())
        {
            MY_LOGE("CameraDevice1Base onOpenLocked failed");
            goto lbExit;
        }
    }
    //
    //--------------------------------------------------------------------------
    ret = OK;
    mbIsUninit = false;
    mTakePictureRetryCount = 0;
lbExit:
    profile.print("");
    MY_LOGD("ret(%d) sensorId(%d,%d)", ret, getOpenId_Main(), getOpenId_Main2());
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void
StereoCameraDevice1::
onCloseLocked()
{
    FUNCTION_SCOPE;
    if(mbIsUninit)
    {
        MY_LOGE("already uninit. skip");
        return;
    }
    Utils::CamProfile  profile(__FUNCTION__, "StereoCameraDevice1");
    //
    if( !waitThreadSensorOnDone() )
    {
        MY_LOGE("sensor power on thread create failed");
    }
    //  (1) reset stereo hal setting.
    //--------------------------------------------------------------------------
    // reset stereo feature mode
    StereoSettingProvider::setStereoFeatureMode(0);
    StereoSettingProvider::setStereoShotMode(0);
    //  (2) Uninit Base
    //--------------------------------------------------------------------------
    {
        CAM_TRACE_NAME("uninit(base)");
        CameraDevice1Base::onCloseLocked();
        profile.print("CameraDevice1Base::onCloseLocked() -");
    }
    //  (2) Uninit3A and power off sensor.
    //--------------------------------------------------------------------------
    std::vector<std::future<MBOOL> > vThread;
#define UNINIT_PIPELINE(JOB_NAME, EXECTUE_FUNC_NAME, PARAM1, PARAM2)\
    do{\
        struct JOB_NAME\
        {\
            static\
                MBOOL      execute(StereoCameraDevice1* pSelf) {\
                                return pSelf->EXECTUE_FUNC_NAME(pSelf->PARAM1, pSelf->PARAM2);\
                            }\
        };\
        vThread.push_back(std::async(std::launch::async, &JOB_NAME::execute, this));\
    }while(0);
#if '1'==MTKCAM_HAVE_3A_HAL
    UNINIT_PIPELINE(job_uninit3a_main, uninit3A, getOpenId_Main(), mpHal3a_Main);
    UNINIT_PIPELINE(job_uninit3a_main2, uninit3A, getOpenId_Main2(), mpHal3a_Main2);
#endif  //MTKCAM_HAVE_3A_HAL
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    UNINIT_PIPELINE(job_powerOff_main, powerOffSensor, getOpenId_Main(), mpHalSensor);
    UNINIT_PIPELINE(job_powerOff_main2, powerOffSensor, getOpenId_Main2(), mpHalSensor_Main2);
#endif  //MTKCAM_HAVE_SENSOR_HAL
#undef UNINIT_PIPELINE
    //
    for(MUINT32 i = 0 ;i<vThread.size();++i)
    {
        vThread[i].wait();
    }
#if '1'==MTKCAM_HAVE_CAMDRV
    {
        CAM_TRACE_NAME("uninit(resource)");
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
    }
#endif  //MTKCAM_HAVE_CAMDRV
    //--------------------------------------------------------------------------
    profile.print("");
    mbIsUninit = true;
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
StereoCameraDevice1::
onStartPreview()
{
    FUNCTION_SCOPE;
    bool ret = false;
    //
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
    //
    //  (1) Check Permission.
    //--------------------------------------------------------------------------
#if '1'== MTKCAM_HAVE_CAM_MANAGER
    Utils::CamManager* pCamMgr = Utils::CamManager::getInstance();
    if ( ! pCamMgr->getPermission() )
    {
        MY_LOGE("Cannot start preview ... Permission denied");
        goto lbExit;
    }
#endif

    //  (2) Update Hal App Mode.
    //--------------------------------------------------------------------------
    if  ( ! mpParamsMgr->updateHalAppMode() )
    {
        MY_LOGE("mpParamsMgr->updateHalAppMode() fail");
        return false;
    }
    // if current device name is used for VSDOF, set enable 3async manually.
    {
        NS3Av3::Stereo_Param_T rStereoParam;
        if(mpHal3a_Main == nullptr || mpHal3a_Main2 == nullptr)
        {
            MY_LOGE("m1(%p) m2(%p)", mpHal3a_Main, mpHal3a_Main2);
            goto lbExit;
        }
        MY_LOGD("Set hwsync enable");
        rStereoParam.i4Sync2AMode = NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_VSDOF;
        rStereoParam.i4SyncAFMode = NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_ON;
        rStereoParam.i4HwSyncMode = NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_ON;
        rStereoParam.i4MasterIdx = getInstanceId();
        rStereoParam.i4SlaveIdx = getOpenId_Main2();
        mpHal3a_Main->send3ACtrl(NS3Av3::E3ACtrl_SetStereoParams, (MINTPTR)&rStereoParam, 0);
        mpHal3a_Main2->send3ACtrl(NS3Av3::E3ACtrl_SetStereoParams, (MINTPTR)&rStereoParam, 0);
    }

    //  (3) Initialize Camera Adapter.
    //--------------------------------------------------------------------------
    if  ( ! initCameraAdapter() )
    {
        MY_LOGE("NULL Camera Adapter");
        goto lbExit;
    }

    // (4) Enter thermal policy.
    //--------------------------------------------------------------------------
    struct job
    {
        static
            MBOOL      execute(StereoCameraDevice1* pSelf) {
                            pSelf->enterThermalPolicy();
                            return MTRUE;
                        }
    };

    mThread_ThermalPolicy = std::async(std::launch::async, &job::execute, this);

    ret = true;

lbExit:
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void
StereoCameraDevice1::
onStopPreview()
{
    FUNCTION_SCOPE;
    // if current device name is used for VSDOF, set enable 3async manually.
    {
        NS3Av3::Stereo_Param_T rStereoParam;
        if(mpHal3a_Main == nullptr || mpHal3a_Main2 == nullptr)
        {
            MY_LOGE("m1(%p) m2(%p)", mpHal3a_Main, mpHal3a_Main2);
        }
        MY_LOGD("Set hwsync disable");
        rStereoParam.i4Sync2AMode = NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_NONE;
        rStereoParam.i4SyncAFMode = NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_OFF;
        rStereoParam.i4HwSyncMode = NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_OFF;
        rStereoParam.i4MasterIdx = getInstanceId();
        rStereoParam.i4SlaveIdx = getOpenId_Main2();
        mpHal3a_Main->send3ACtrl(NS3Av3::E3ACtrl_SetStereoParams, (MINTPTR)&rStereoParam, 0);
        mpHal3a_Main2->send3ACtrl(NS3Av3::E3ACtrl_SetStereoParams, (MINTPTR)&rStereoParam, 0);
    }
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->cancelPicture();
        mpCamAdapter->uninit();
        mpCamAdapter.clear();
    }
    thermalExitThread.wait();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
StereoCameraDevice1::
onStartRecording()
{
#if '1'== MTKCAM_HAVE_CAM_MANAGER
    //  (1) Check Permission.
    if ( Utils::CamManager::getInstance()->isMultiDevice() )
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
 * Create a thread to hide some initial steps to speed up launch time
 ******************************************************************************/
bool
StereoCameraDevice1::
powerOnSensor()
{
    FUNCTION_SCOPE;
    bool    ret = false;
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    CAM_TRACE_CALL();
    Utils::CamProfile  profile(__FUNCTION__, "StereoCam1Device");
    std::future<bool> main1_poweron_job;
    std::future<bool> main2_poweron_job;
    auto power_on_sensor = [this](MUINT sensorId, MBOOL isMain1)
    {
        // (1) increase sensor count
        //--------------------------------------------------------------------------
        NSCam::Utils::CamManager::getInstance()->incSensorCount(LOG_TAG);
        // (2) create sensor interface
        //--------------------------------------------------------------------------
        NSCam::IHalSensorList* pHalSensorList = MAKE_HalSensorList();
        if(!pHalSensorList)
        {
            MY_LOGE("pHalSensorList == NULL");
            return false;
        }
        if(isMain1)
        {
            mpHalSensor = pHalSensorList->createSensor(
                                                USER_NAME,
                                                sensorId);
            if(!mpHalSensor)
            {
                MY_LOGE("mpHalSensor is nullptr");
                return false;
            }
        }
        else
        {
            mpHalSensor_Main2 = pHalSensorList->createSensor(
                                                USER_NAME,
                                                sensorId);
            if(!mpHalSensor_Main2)
            {
                MY_LOGE("mpHalSensor_Main2 is nullptr");
                return false;
            }
        }
        //--------------------------------------------------------------------------
        if(isMain1)
        {
            if( !mpHalSensor->powerOn(USER_NAME, 1, &sensorId) )
            {
                MY_LOGE("Sensor power on failed: %d", sensorId);
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
        }
        else
        {
            if( !mpHalSensor_Main2->powerOn(USER_NAME, 1, &sensorId) )
            {
                MY_LOGE("Sensor power on failed: %d", sensorId);
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
        }
        return true;
    };
    main1_poweron_job = std::async(std::launch::async, power_on_sensor, getOpenId_Main(), true);
    main2_poweron_job = std::async(std::launch::async, power_on_sensor, getOpenId_Main2(), false);
    main1_poweron_job.wait();
    main2_poweron_job.wait();
    //
    profile.print("Sensor Hal -");
#endif  //MTKCAM_HAVE_SENSOR_HAL
    ret = true;
lbExit:
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void*
StereoCameraDevice1::
doThreadSensorOn(void* arg)
{
    ::prctl(PR_SET_NAME,"initCamdevice", 0, 0, 0);
    StereoCameraDevice1* pSelf = reinterpret_cast<StereoCameraDevice1*>(arg);
    pSelf->mRet = pSelf->powerOnSensor();
    pSelf->mDeviceManager->updatePowerOnDone();
    pthread_exit(NULL);
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
StereoCameraDevice1::
waitThreadSensorOnDone()
{
    bool ret = false;
    if( mbThreadRunning )
    {
        MY_LOGD("wait sensor power on done +");
        int s = pthread_join(mThreadHandle, NULL);
        MY_LOGD("wait sensor power on done -");
        mbThreadRunning = MFALSE;
        if( s != 0 )
        {
            MY_LOGE("sensor_power_on thread join error: %d", s);
            goto lbExit;
        }

        if( !mRet )
        {
            MY_LOGE("init in thread failed");
            goto lbExit;
        }
    }

    ret = true;
lbExit:
    return ret;
}

/******************************************************************************
 *
 *
 ******************************************************************************/
bool
StereoCameraDevice1::
disableWaitSensorThread(bool disable)
{
    MY_LOGD("disable(%d)",disable);
    mDisableWaitSensorThread = disable;
    return true;
}

/******************************************************************************
 *
 *
 ******************************************************************************/
Return<Status>
StereoCameraDevice1::
takePicture()
{
    {
        Mutex::Autolock _l(mPostProcCheckLock);
        android::NSPostProc::IImagePostProcessManager* pIIPPManager =
                    android::NSPostProc::IImagePostProcessManager::getInstance();
        MBOOL isPostProcAvailable = pIIPPManager->isAvailable();
        while(!isPostProcAvailable)
        {
            MY_LOGD("wait post processor available + retry(%d)", mTakePictureRetryCount++);
            mPostProcCheckCond.waitRelative(mPostProcCheckLock, 100000000LL);// wait 100ms
            isPostProcAvailable = pIIPPManager->isAvailable();
            MY_LOGD("wait post processor available (%d)-", isPostProcAvailable);
            if(mTakePictureRetryCount > MAX_CAPTURE_RETRY_COUNT)
            {
                MY_LOGE("takepicture retry fail, kill!!");
                ::raise(SIGINT);
            }
        }
        mTakePictureRetryCount = 0;
    }
    return CameraDevice1Base::takePicture();
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoCameraDevice1::
onSetParameters(const char* params)
{
    (void)params;
    MY_LOGD("");
    status_t status = OK;
    return  status;
}


/******************************************************************************
 * stereo has it own stop flow.
 * because it needs to hide exit thermal policy to improve performance.
 ******************************************************************************/
Return<void>
StereoCameraDevice1::
stopPreview()
{
    struct job
    {
        static
            MBOOL      execute(StereoCameraDevice1* pSelf) {
                            pSelf->exitThermalPolicy();
                            return MTRUE;
                        }
    };

    thermalExitThread = std::async(std::launch::async, &job::execute, this);
    CameraDevice1Base::stopPreview();
    return Void();
}

/******************************************************************************
 *  enter thermal policy
 *
 ******************************************************************************/
status_t
StereoCameraDevice1::
enterThermalPolicy(
)
{
    switch(StereoSettingProvider::getStereoFeatureMode()){
        case E_STEREO_FEATURE_CAPTURE | E_STEREO_FEATURE_VSDOF:
        case E_STEREO_FEATURE_THIRD_PARTY:
        case E_STEREO_FEATURE_MTK_DEPTHMAP:
            MY_LOGD("enable thermal policy 03");
            Utils::CamManager::getInstance()->setThermalPolicy(THERMAL_VSDOF_POLICY_NAME, 1);
            break;
        case E_STEREO_FEATURE_DENOISE:
            MY_LOGD("enable thermal policy 04");
            Utils::CamManager::getInstance()->setThermalPolicy(THERMAL_DENOISE_POLICY_NAME, 1);
            break;
        default:
            MY_LOGW("feature combination not supported:%d, use default policy",
                                            StereoSettingProvider::getStereoFeatureMode());
    }
    return OK;
}

/******************************************************************************
 *  exit thermal policy
 *
 ******************************************************************************/
status_t
StereoCameraDevice1::
exitThermalPolicy(
)
{
    MY_LOGD("disable thermal policies");
    Utils::CamManager::getInstance()->setThermalPolicy(THERMAL_VSDOF_POLICY_NAME, 0);
    Utils::CamManager::getInstance()->setThermalPolicy(THERMAL_DENOISE_POLICY_NAME, 0);
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoCameraDevice1::
powerOffSensor(
    int32_t openId,
    NSCam::IHalSensor*& halSensor
)
{
    MY_LOGD("(%d) +", openId);
    if(halSensor == nullptr)
    {
        MY_LOGW("(%d) halSensor is nullptr", openId);
        return OK;
    }
    if(openId < 0)
    {
        MY_LOGW("openId(%d) < 0, skip.", openId);
        return OK;
    }
    MUINT pIndex_main[1] = { (MUINT)openId};
    halSensor->powerOff(USER_NAME, 1, pIndex_main);
    NSCam::Utils::CamManager::getInstance()->decSensorCount(LOG_TAG);

    halSensor->destroyInstance(USER_NAME);
    halSensor = NULL;
    MY_LOGD("(%d) -", openId);
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
StereoCameraDevice1::
uninit3A(
    int32_t openId,
    NS3Av3::IHal3A*& hal3A
)
{
    MY_LOGD("(%d) +", openId);
    if(hal3A == nullptr)
    {
        MY_LOGW("(%d) hal3a is nullptr", openId);
        return OK;
    }
    hal3A->notifyPwrOff();
    hal3A->destroyInstance(LOG_TAG);
    hal3A = NULL;
    MY_LOGD("(%d) -", openId);
    return OK;
}
