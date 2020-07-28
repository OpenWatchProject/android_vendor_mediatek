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
#if '1'== MTKCAM_HAVE_CAM_MANAGER
#include <mtkcam/utils/hw/CamManager.h>
#endif
//
#include <mtkcam/utils/fwk/MtkCamera.h>
//
#include <mtkcam/middleware/v1/camutils/FrameworkCBThread.h>
using namespace android::MtkCamUtils;
//
#include "MyUtils.h"
#include "DualCameraDevice1.h"
//
#include <cutils/properties.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
using namespace android;
//
using namespace NSCam::Utils;
//
#include <mtkcam/middleware/v1/camutils/FrameworkCBThread.h>
//
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
//
#include <mtkcam/utils/hw/HwInfoHelper.h>
//
#if (MTKCAM_HAVE_AEE_FEATURE == 1)
#include <aee.h>
#endif // MTKCAM_HAVE_AEE_FEATURE

#if (MTKCAM_HAVE_MTKSTEREO_SUPPORT == 1) || (MTK_CAM_STEREO_DENOISE_SUPPORT == 1)
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/feature/DualCam/IDualCamPolicy.h>
#include <mtkcam/feature/DualCam/DualCam.Common.h>
//#include <mtkcam/aaa/ISync3A.h>
#include <mtkcam/middleware/v1/PostProc/IPostProc.h>
#define DUALCAM_SHOT (1)
#else
#define DUALCAM_SHOT (0)
#endif
//
#include <future>
#include <functional>
#include <chrono>
#include <ostream>
//
using namespace NSCam;
using namespace NSCam::v1::Stereo;
//
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("%d[DualCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("%d[DualCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("%d[DualCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("%d[DualCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("%d[DualCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("%d[DualCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("%d[DualCameraDevice1::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
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
#define IMG_1080P_W         (1920)
#define IMG_1080P_H         (1080)
#define IMG_1080P_H_ALIGN   (1088)
#define IMG_1080P_SIZE      (IMG_1080P_W*IMG_1080P_H_ALIGN)
/******************************************************************************
 *
 ******************************************************************************/
namespace
{
    static MBOOL getBGStartTelePreviewSupported()
    {
        static const MBOOL ret = property_get_int32("debug.camera.bgStartTelePreview", 1);
        return ret;
    }
}
/******************************************************************************
 *
 ******************************************************************************/
extern "C"
NSCam::CameraDevice1Base*
createCameraDevice1_Dual(
    ICameraDeviceManager* deviceManager,
    std::string           instanceName,
    int32_t               instanceId,
    android::String8 const& rDevName
)
{
    return new DualCameraDevice1(deviceManager
                                  , instanceName
                                  , instanceId
                                  , rDevName);

}
/******************************************************************************
 *
 ******************************************************************************/
bool
ImgBufProviderClientBridge::
onImgBufProviderCreated(sp<IImgBufProvider>const& rpProvider)
{
    Mutex::Autolock _l(mLock);
    for (size_t i = 0; i < mvClient.size(); i++)
    {
        CAM_LOGD("WillDBG3 onImgBufProviderCreated");
        mvClient.editItemAt(i)->onImgBufProviderCreated(rpProvider);
    }
    return  true;
}
/******************************************************************************
 *
 ******************************************************************************/
void
ImgBufProviderClientBridge::
onImgBufProviderDestroyed(int32_t const i4ProviderId)
{
    Mutex::Autolock _l(mLock);
    for (size_t i = 0; i < mvClient.size(); i++)
    {
        mvClient.editItemAt(i)->onImgBufProviderDestroyed(i4ProviderId);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
void
ImgBufProviderClientBridge::
addImgBufProviderClient(sp<IImgBufProviderClient> rpClient)
{
    Mutex::Autolock _l(mLock);
    mvClient.push_back(rpClient);
}
/******************************************************************************
 *
 ******************************************************************************/
void
ImgBufProviderClientBridge::
clearImgBufProviderClient()
{
    Mutex::Autolock _l(mLock);
    mvClient.clear();
}
/******************************************************************************
 *
 ******************************************************************************/
DualCameraDevice1::
DualCameraDevice1(
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
DualCameraDevice1::
~DualCameraDevice1()
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
DualCameraDevice1::
onOpenLocked()
{
    FUNCTION_SCOPE;
    //[1] Init platform pofile
    //--------------------------------------------------------------------------
    CAM_TRACE_NAME("init(profile)");
    CamProfile  profile(__FUNCTION__, "DaulCam1Device");
    //
    status_t    ret = NO_INIT;
    int     err = 0, i4DeviceNum = 0;
    int     Supported = 0;
    //[2] update stereo profile
    //--------------------------------------------------------------------------
    if(updateSensorId() != OK)
    {
        MY_LOGE("Cannot get valid sensor id, init camera fail.");
        goto lbExit;
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

#if 0 // dual cam zoom not support YUV sensor yet.
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
#endif // 0
#endif  //MTKCAM_HAVE_SENSOR_HAL
    }
    //[4] Create and open 3A HAL
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
        INIT_3A(mpHal3a, this->getInstanceId());
        INIT_3A(mpHal3a_2, getOpenId_Main2());
#undef RESTORE_STATIC_DATA
        profile.print("3A Hal -");
    }
#endif  //MTKCAM_HAVE_3A_HAL
    //--------------------------------------------------------------------------
    {
        CAM_TRACE_NAME("init(DevcieBase on Open)");
        //  (2) Init Base.
        if ( OK != CameraDevice1Base::onOpenLocked())
        {
            MY_LOGE("CameraDevice1Base onOpenLocked failed");
            goto lbExit;
        }
    }
    mpParamsMgrV3_Sup = IParamsManagerV3::createInstance(mDevName, mSensorId_Main2, mpParamsMgr);
    mpParamsMgrV3_Sup->setStaticData();

    mpBridge = new ImgBufProviderClientBridge();

    //
    //--------------------------------------------------------------------------
    //
    ret = OK;
lbExit:
    profile.print("");
    mb4K2KVideoRecord = false;
    MY_LOGD("- ret(%d)", ret);
    updateDeviceState(MtkCameraParameters::KEY_DUALCAM_DEVICE_STATE_IDLE);
    mbIsUninit = false;
    return  ret;
}
/******************************************************************************
 *
 ******************************************************************************/
void
DualCameraDevice1::
onCloseLocked()
{
    FUNCTION_SCOPE;
    if(mbIsUninit)
    {
        MY_LOGE("already uninit. skip");
        MY_LOGD("-");
        return;
    }
    CamProfile  profile(__FUNCTION__, "DualCameraDevice1");
    //
    if( !waitThreadSensorOnDone() )
    {
        MY_LOGE("sensor power on thread create failed");
    }
    //--------------------------------------------------------------------------
    // reset stereo feature mode
    StereoSettingProvider::setStereoFeatureMode(0);
    StereoSettingProvider::setStereoShotMode(0);
    StereoSettingProvider::setStereoModuleType(0);
    StereoSettingProvider::setStereoProfile((ENUM_STEREO_SENSOR_PROFILE)0);
    std::future<void> tUninitClient;
    //  (1) Uninit Base
    {
        CAM_TRACE_NAME("uninit(base)");
#define RESTORE_STATIC_DATA(PARAM_MGR)\
        do\
        {\
            if(PARAM_MGR != nullptr)\
            {\
                PARAM_MGR->restoreStaticData();\
            }\
        }while(0);
        RESTORE_STATIC_DATA(mpParamsMgrV3);
        RESTORE_STATIC_DATA(mpParamsMgrV3_Sup);
#undef RESTORE_STATIC_DATA
        //
        if (mpDisplayClient != 0)
        {
            mpDisplayClient->uninit();
            mpDisplayClient.clear();
        }
        //
        // use other thread to uninit camera client
        tUninitClient = std::async(
                        std::launch::async,
                        [this]()
                        {
                            if (mpCamClient != 0)
                            {
                                mpCamClient->uninit();
                                mpCamClient.clear();
                            }
                        });
        // forward to registered clients
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->uninit();
        }
        vmpCamClient.clear();
#define RELEASE_ADPATER(ADAPTER)\
        do\
        {\
            if(ADAPTER != nullptr)\
            {\
                ADAPTER->uninit();\
                ADAPTER = nullptr;\
            }\
        }while(0);
        MY_LOGD("WillDBG uninit Wide");
        RELEASE_ADPATER(mpCamAdapter_W);
        MY_LOGD("WillDBG uninit Tele");
        RELEASE_ADPATER(mpCamAdapter_T);
#undef RESTORE_STATIC_DATA
        MY_LOGD("WillDBG uninit adapter done");
        mpCamAdapter = NULL;
        if (mpBridge != NULL)
        {
            mpBridge->clearImgBufProviderClient();
            mpBridge = NULL;
        }
        //
        mpParamsMgr->uninit();
        //
        //  Uninitialize Device Callback.
        mpCamMsgCbInfo->mCam1DeviceCb = NULL;
        //
        if (mpCpuCtrl != NULL)
        {
            mpCpuCtrl->disable();
        }
        profile.print("Cam1DeviceBase::onUninit() -");
    }
    {
        mSyncMgr = nullptr;
    }
    //--------------------------------------------------------------------------
    //  (2) Uninit3A and power off sensor.
    std::vector<std::future<MBOOL> > vThread;
#define UNINIT_PIPELINE(JOB_NAME, EXECTUE_FUNC_NAME, PARAM1, PARAM2)\
    do{\
        struct JOB_NAME\
        {\
            static\
                MBOOL      execute(DualCameraDevice1* pSelf) {\
                                return pSelf->EXECTUE_FUNC_NAME(pSelf->PARAM1, pSelf->PARAM2);\
                            }\
        };\
        vThread.push_back(std::async(std::launch::async, &JOB_NAME::execute, this));\
    }while(0);
#if '1'==MTKCAM_HAVE_3A_HAL
    UNINIT_PIPELINE(job_uninit3a_main, uninit3A, getInstanceId(), mpHal3a);
    UNINIT_PIPELINE(job_uninit3a_main2, uninit3A, getOpenId_Main2(), mpHal3a_2);
#endif  //MTKCAM_HAVE_3A_HAL
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    UNINIT_PIPELINE(job_powerOff_main, powerOffSensor, getInstanceId(), mpHalSensor);
    UNINIT_PIPELINE(job_powerOff_main2, powerOffSensor, getOpenId_Main2(), mpHalSensor_2);
#endif  //MTKCAM_HAVE_SENSOR_HAL
#undef UNINIT_PIPELINE
    //--------------------------------------------------------------------------
    //
    // check if tUninitClient is finished.
    if(tUninitClient.valid())
    {
        tUninitClient.get();
    }
    for(auto& thread:vThread)
    {
        if(thread.valid())
        {
            thread.get();
        }
    }
    mpScenarioCtrl_main1 = NULL;
    mpScenarioCtrl_main2 = NULL;
    mbIsUninit = true;
    profile.print("");
    return;
}


/******************************************************************************
 * [Template method] Called by startPreview().
 ******************************************************************************/
bool
DualCameraDevice1::
onStartPreview()
{
    bool ret = false;
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
    Utils::CamManager* pCamMgr = Utils::CamManager::getInstance();
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

    //  (3) Set dual cam feature mode
    //--------------------------------------------------------------------------
    if(setFeatureMode() != OK)
    {
        MY_LOGE("unsupport dual cam mode, please check it");
        goto lbExit;
    }
    if(setSensorModuleType() != OK)
    {
        MY_LOGE("Set sensor module type fail");
        goto lbExit;
    }

    //  (4) Initialize Camera Adapter.
    //--------------------------------------------------------------------------
    if (! initDualCameraAdapter())
    {
        MY_LOGE("NULL Camera Adapter");
        goto lbExit;
    }
    //
    ret = true;
lbExit:
    return ret;
}


/******************************************************************************
 *  [Template method] Called by stopPreview().
 ******************************************************************************/
void
DualCameraDevice1::
onStopPreview()
{
    FUNCTION_SCOPE;
    // if current device name is used for VSDOF, set enable 3async manually.
    if(mDevName == MtkCameraParameters::APP_MODE_NAME_MTK_STEREO)
    {
        NS3Av3::Stereo_Param_T rStereoParam;
        if(mpHal3a == nullptr || mpHal3a_2 == nullptr)
        {
            MY_LOGE("m1(%p) m2(%p)", mpHal3a, mpHal3a_2);
        }
        MY_LOGD("Set hwsync disable");
        rStereoParam.i4Sync2AMode = NS3Av3::E_SYNC2A_MODE::E_SYNC2A_MODE_NONE;
        rStereoParam.i4SyncAFMode = NS3Av3::E_SYNCAF_MODE::E_SYNCAF_MODE_OFF;
        rStereoParam.i4HwSyncMode = NS3Av3::E_HW_FRM_SYNC_MODE::E_HW_FRM_SYNC_MODE_OFF;
        rStereoParam.i4MasterIdx = getInstanceId();
        rStereoParam.i4SlaveIdx = getOpenId_Main2();
        mpHal3a->send3ACtrl(NS3Av3::E3ACtrl_SetStereoParams, (MINTPTR)&rStereoParam, 0);
        mpHal3a_2->send3ACtrl(NS3Av3::E3ACtrl_SetStereoParams, (MINTPTR)&rStereoParam, 0);
    }
    std::future<void> tUninitTask_W = std::async(
                    std::launch::async,
                    [this]()
                    {
                        if (mpCamAdapter_W != 0)
                        {
                            mpCamAdapter_W->cancelPicture();
                            mpCamAdapter_W->uninit();
                            mpCamAdapter_W = NULL;
                        }
                    });
    if (mpCamAdapter_T != 0)
    {
        mpCamAdapter_T->cancelPicture();
        mpCamAdapter_T->uninit();
        mpCamAdapter_T = NULL;
    }
    if(tUninitTask_W.valid())
    {
        tUninitTask_W.get();
    }
    mpCamAdapter = NULL;
    mpBridge->clearImgBufProviderClient();
}


/******************************************************************************
 * [Template method] Called by startRecording().
 ******************************************************************************/
bool
DualCameraDevice1::
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
 * Create a thread to hide some initial steps to speed up launch time
 ******************************************************************************/
bool
DualCameraDevice1::
powerOnSensor()
{
    FUNCTION_SCOPE;
    bool    ret = false;
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    CAM_TRACE_CALL();
    Utils::CamProfile  profile(__FUNCTION__, "StereoCam1Device");
    std::future<bool> main1_poweron_job;
    std::future<bool> main2_poweron_job;
    mpScenarioCtrl_main1 = NSCam::IScenarioControl::create(getInstanceId());
    mpScenarioCtrl_main2 = NSCam::IScenarioControl::create(getOpenId_Main2());
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
            mpHalSensor_2 = pHalSensorList->createSensor(
                                                USER_NAME,
                                                sensorId);
            if(!mpHalSensor_2)
            {
                MY_LOGE("mpHalSensor_2 is nullptr");
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
            if( !mpHalSensor_2->powerOn(USER_NAME, 1, &sensorId) )
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
    main1_poweron_job = std::async(std::launch::async, power_on_sensor, this->getInstanceId(), true);
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
 * the init function to be called in the thread
 ******************************************************************************/
void*
DualCameraDevice1::
doThreadSensorOn(void* arg)
{
    ::prctl(PR_SET_NAME,"initCamdevice", 0, 0, 0);
    DualCameraDevice1* pSelf = reinterpret_cast<DualCameraDevice1*>(arg);
    pSelf->mRet = pSelf->powerOnSensor();
    pSelf->mDeviceManager->updatePowerOnDone();
    pthread_exit(NULL);
    return NULL;
}


/******************************************************************************
 * Wait for initializations by thread are done.
 ******************************************************************************/
bool
DualCameraDevice1::
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
DualCameraDevice1::
disableWaitSensorThread(bool disable)
{
    MY_LOGD("disable(%d)",disable);
    mDisableWaitSensorThread = disable;
    return true;
}


/******************************************************************************
 *  Set the camera parameters. This returns BAD_VALUE if any parameter is
 *  invalid or not supported.
 ******************************************************************************/
status_t
DualCameraDevice1::
onSetParameters(const char* params    __attribute__((unused)))
{
    FUNCTION_SCOPE;
    status_t status = OK;
    //  Here (1) succeeded.
    //  (2) If CamAdapter exists, apply mpParamsMgr to CamAdapter;
    //      otherwise it will be applied when CamAdapter is created.
    {
#if 0 //TBD
#if '1'==MTKCAM_HAVE_3A_HAL
        else if (mpHal3a)
        {
            //  Flashlight may turn on/off in case that CamAdapter doesn't exist (i.e. never call startPreview)

            using namespace NS3Av3;
            Param_T param;
            //
            if (! mpHal3a->getParams(param))
            {
                MY_LOGW("3A Hal::getParams() fail - err(%x)", mpHal3a->getErrorCode());
            }
            //
            String8 const s8FlashMode = mpParamsMgr->getStr(CameraParameters::KEY_FLASH_MODE);
            if (! s8FlashMode.isEmpty())
            {
                param.u4StrobeMode = PARAMSMANAGER_MAP_INST(eMapFlashMode)->valueFor(s8FlashMode);
            }
            //
            if (! mpHal3a->setParams(param))
            {
                MY_LOGW("3A Hal::setParams() fail - err(%x)", mpHal3a->getErrorCode());
            }
        }
        else
        {
            MY_LOGW("mpHal3a==NULL");
        }
#endif
#endif
    }
#if '1' == MTKCAM_HR_MONITOR_SUPPORT
    {
        sp<ICamClient> pCamClient = mpCamClient;
        //if  ( pCamClient != 0 && INVALID_OPERATION != (status = pCamClient->sendCommand(CAMERA_CMD_CHECKPARA_HR_PREVIEW, 0, 0)) )
        if (pCamClient != 0)
        {
            //  we just return since this cammand has been handled.
            //return  status;
            pCamClient->sendCommand(CAMERA_CMD_CHECKPARA_HR_PREVIEW, 0, 0);
        }
    }
#endif

lbExit:
    return  status;
}

// Overwrite Cam1DeviceBase
/******************************************************************************
 *  Start preview mode.
 ******************************************************************************/
Return<Status>
DualCameraDevice1::
startPreview()
{
    CAM_TRACE_CALL();
    MY_LOGI("+");

    status_t status = OK;
    bool usePreviewThread = false;
    NSCam::Utils::CamManager* pCamMgr = NSCam::Utils::CamManager::getInstance();
    //
    if( mpParamsMgr->getInt(MtkCameraParameters::KEY_CAMERA_MODE) != MtkCameraParameters::CAMERA_MODE_NORMAL &&
        mbWindowReady == false)
    {
        MY_LOGD("use thread to init preview");
        usePreviewThread = true;
        disableWaitSensorThread(true);
    }
    //
    {
        CAM_TRACE_NAME("deviceStartPreview");
        if (mpCamAdapter != 0 && mpCamAdapter->isTakingPicture())
        {
            MY_LOGE("Capture is not done");
            status = INVALID_OPERATION;
            return  mapToHidlCameraStatus(status);
        }
        //
        if (previewEnabled())
        {
            MY_LOGD("Preview already running");
            status = ALREADY_EXISTS;
            return  mapToHidlCameraStatus(status);
        }
        //  Check Permission.
        if (! pCamMgr->getPermission())
        {
            MY_LOGE("Cannot start preview ... Permission denied");
            sp<IFrameworkCBThread> spFrameworkCBThread = IFrameworkCBThread::createInstance(getInstanceId(),
                    mpCamMsgCbInfo);
            spFrameworkCBThread->init();
            IFrameworkCBThread::callback_data cbData;
            cbData.callbackType = IFrameworkCBThread::CALLBACK_TYPE_NOTIFY;
            cbData.type         = CAMERA_MSG_ERROR;
            cbData.ext1         = CAMERA_ERROR_SERVER_DIED;
            cbData.ext2         = 0;
            spFrameworkCBThread->postCB(cbData);
            spFrameworkCBThread->uninit();
            spFrameworkCBThread = NULL;
            status = OK;
            goto lbExit;
        }
        //
        if (! onStartPreview())
        {
            MY_LOGE("onStartPreviewLocked() fail");
            status = INVALID_OPERATION;
            goto lbExit;
        }
    }
    //
    {
        CAM_TRACE_NAME("clientStartPreview");
        if (mpDisplayClient == 0)
        {
            MY_LOGD("DisplayClient is not ready.");
        }
        else if (OK != (status = enableDisplayClient()))
        {
            goto lbExit;
        }
        //
        if (mpCamClient != 0)
        {
            if (! mpCamClient->startPreview())
            {
                status = INVALID_OPERATION;
                goto lbExit;
            }
        }
        // forward to registered clients
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->startPreview();
        }
    }

    //
    //  startPreview in Camera Adapter.
    {
        if (usePreviewThread)
        {
            if (pthread_create(&mStartPreviewTThreadHandle, NULL, startPreviewThread, this) != 0)
            {
                ALOGE("startPreview pthread create failed");
            }
        }
        else
        {
            status = dualAdapterStartPreview();
        }
    }
    //
    //
    enableMsgType(CAMERA_MSG_PREVIEW_METADATA);
    //
    mIsPreviewEnabled = true;

    if (mpParamsMgr->getRecordingHint())
    {
        MSize paramSize;
        mpParamsMgr->getVideoSize(&paramSize.w, &paramSize.h);
        mb4K2KVideoRecord = (paramSize.w*paramSize.h > IMG_1080P_SIZE) ? MTRUE : MFALSE;
    }
    //
    status = OK;
    isStoppreview = MFALSE;
lbExit:
    if (OK != status)
    {
        MY_LOGD("Cleanup after error");
        //
        if (mpCamClient != 0)
        {
            mpCamClient->stopPreview();
        }
        // forward to registered clients
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->stopPreview();
        }
        //
        disableDisplayClient();
    }
    else
    {
        updateDeviceState(MtkCameraParameters::KEY_DUALCAM_DEVICE_STATE_PREVIEWING);
    }
    //
    MY_LOGI("- status(%d)", status);
    return  mapToHidlCameraStatus(status);
}

/******************************************************************************
 *  Stop a previously started preview.
 ******************************************************************************/
Return<void>
DualCameraDevice1::
stopPreview()
{
    CAM_TRACE_CALL();
    MY_LOGI("+");
    //
    waitStartPreviewDone();
    //
    disableMsgType(CAMERA_MSG_PREVIEW_METADATA);
    //
    if(::getBGStartTelePreviewSupported())
    {
        if(mBgStartTelePreviewJob.valid())
        {
            status_t tmp = mBgStartTelePreviewJob.get();
            MY_LOGD("wait done for previous background start tele preview, result: %d", tmp);
        }
    }
    {
        Mutex::Autolock _l(mPreviewMetaLock);
        MY_LOGD("stoppreview");
        isStoppreview = MTRUE;
    }
    //
    if (! previewEnabled())
    {
        MY_LOGD("Preview already stopped, perhaps!");
        MY_LOGD("We still force to clean up again.");
    }
    // flush hwsync
    mSyncMgr = NSCam::ISyncManager::getInstance(getInstanceId());
    if (mSyncMgr != NULL)
    {
        mSyncMgr->setEvent(0, NULL);
        mSyncMgr = NULL;
    }
    else
    {
        MY_LOGW("Cannot get syncmanager in stop preview. Can't flush hwsync");
    }
    // stop wide adapter simultaneously
    std::function<void(void)> stopAction_W =
                    [this]()mutable{
                        CAM_TRACE_NAME("adapterStopPreview");
                        if (mpCamAdapter_W != 0)
                        {
                            if (mpCamAdapter_W->recordingEnabled())
                            {
                                stopRecording();
                            }
                            mpCamAdapter_W->stopPreview();
                        }
                    };
    std::future<void> stopPreviewCmd_W =
                        std::async(
                                    std::launch::async,
                                    stopAction_W);
    //
    {
        CAM_TRACE_NAME("adapterStopPreview");
        if (mpCamAdapter_T != 0)
        {
            if (mpCamAdapter_T->recordingEnabled())
            {
                stopRecording();
            }
            mpCamAdapter_T->stopPreview();
        }
    }
    // join stop thread
    if(stopPreviewCmd_W.valid())
    {
        MY_LOGD("have share state");
        stopPreviewCmd_W.get();
    }
    //
    {
        CAM_TRACE_NAME("clientStopPreview");
        if (mpCamClient != 0)
        {
            mpCamClient->stopPreview();
        }
        //
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->stopPreview();
        }
        //
        disableDisplayClient();
    }
    //
    //
    {
        CAM_TRACE_NAME("deviceStopPreview");
        onStopPreview();
    }
    //
    //
#if 1
    if (mpDisplayClient != 0)
    {
        mpDisplayClient->waitUntilDrained();
    }
#endif
lbExit:
    //  Always set it to false.
    mIsPreviewEnabled = false;
    mb4K2KVideoRecord = false;
    updateDeviceState(MtkCameraParameters::KEY_DUALCAM_DEVICE_STATE_IDLE);
    MY_LOGI("-");
    return Void();
}


/******************************************************************************
 * Set the camera parameters. This returns BAD_VALUE if any parameter is
 * invalid or not supported.
 ******************************************************************************/
Return<Status>
DualCameraDevice1::
setParameters(const hidl_string& params)
{
    CAM_TRACE_CALL();
    MY_LOGD("");
    waitStartPreviewDone();
    status_t status = OK;
    //char* zoom;
    //char* zoom_ratio;
    //
    //  (1) Update params to mpParamsMgr.
    status = mpParamsMgr->setParameters(String8(params.c_str()));
    if (OK != status)
    {
        goto lbExit;
    }

    //  Here (1) succeeded.
    //  (2) If CamAdapter exists, apply mpParamsMgr to CamAdapter;
    //      otherwise it will be applied when CamAdapter is created.
    if (mpCamAdapter_W != 0)
    {
        status = mpCamAdapter_W->setParameters();

    }
    if (mpCamAdapter_T != 0)
    {
        status = mpCamAdapter_T->setParameters();
    }

    status = onSetParameters(params.c_str());
lbExit:
    MY_LOGD("- status(%d)", status);
    return  mapToHidlCameraStatus(status);
}

/******************************************************************************
 * Start record mode. When a record image is available a CAMERA_MSG_VIDEO_FRAME
 * message is sent with the corresponding frame. Every record frame must be released
 * by a cameral hal client via releaseRecordingFrame() before the client calls
 * disableMsgType(CAMERA_MSG_VIDEO_FRAME). After the client calls
 * disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's responsibility
 * to manage the life-cycle of the video recording frames, and the client must
 * not modify/access any video recording frames.
 ******************************************************************************/
Return<Status>
DualCameraDevice1::
startRecording()
{
    CAM_TRACE_CALL();
    MY_LOGI("+");
    //
    status_t status = OK;
    //
    if (mpCamAdapter == 0)
    {
        MY_LOGE("NULL Camera Adapter");
        status = DEAD_OBJECT;
        goto lbExit;
    }

    if  ( ! onStartRecording() )
    {
        MY_LOGE("onStartRecording() fail");
        status = INVALID_OPERATION;
        goto lbExit;
    }
    //  startRecording in Camera Adapter.
    {
        // *****************************************************************************************
        // stopPreview() to make sure both wide/tele DualCamStreamFeaturePipe are destroyed
        // (destroy Cam0, Cam2 (SyncManager ref count:2->1->0(end)) and then create Cam0 and Cam2)
        //
        // without stop tele, FeatureFlowControl::needReconstructRecordingPipe() would re-create.
        // the SyncManager ref count:(2->1->2)->1->2 would not be zero
        // *****************************************************************************************
        MBOOL needReconstruct = needReconstructRecordingPipe(mpParamsMgrV3, true);
        needReconstruct = needReconstruct || needReconstructRecordingPipe(mpParamsMgrV3_Sup, true);

        if (mpCamAdapter_T != 0 && needReconstruct)
        {
            MY_LOGD("Tele stopPreview()");
            // flush hwsync
            mSyncMgr = NSCam::ISyncManager::getInstance(getInstanceId());
            if (mSyncMgr != NULL)
            {
                mSyncMgr->setEvent(0, NULL);
                mSyncMgr = NULL;
            }
            else
            {
                MY_LOGW("Cannot get syncmanager in stop preview. Can't flush hwsync");
            }
            //
            mpCamAdapter_T->stopPreview();
        }

        CAM_TRACE_NAME("adapterStartRecording");
        MY_LOGD("Wide startRecording()");
        status = mpCamAdapter_W->startRecording();
        if (OK != status)
        {
            MY_LOGE("Wide startRecording() in CameraAdapter returns: [%s(%d)]", ::strerror(-status), -status);
            goto lbExit;
        }

        if (mpCamAdapter_T != 0)
        {
            if (needReconstruct)
            {
                MY_LOGD("Tele startPreview()");
                mpParamsMgr->setRecordLikely(true);
                mpCamAdapter_T->startPreview();
            }
            MY_LOGD("Tele startRecording()");
            status = mpCamAdapter_T->startRecording();
            if (needReconstruct)
            {
                mpParamsMgr->setRecordLikely(false);
                // register callback to syncmanager
                mSyncMgr = NSCam::ISyncManager::getInstance(getInstanceId());
                if (mSyncMgr != NULL)
                {
                    mSyncMgr->registerMgrCb(this);
                    mSyncMgr = NULL;
                }
                else
                {
                    MY_LOGW("Cannot get syncmanager in start record");
                }
            }
        }

        if (OK != status)
        {
            MY_LOGE("Tele startRecording() in CameraAdapter returns: [%s(%d)]", ::strerror(-status), -status);
            goto lbExit;
        }
    }
    //
    {
        CAM_TRACE_NAME("clientStartRecording");
        if (mpCamClient != 0)
        {
            //  Get recording format & size.
            //  Set.
            if (! mpCamClient->startRecording())
            {
                status = INVALID_OPERATION;
                goto lbExit;
            }
        }
        // forward to registered clients
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->startRecording();
        }
    }
    isStoppreview = MFALSE;
lbExit:
    if (OK == status)
    {
        updateDeviceState(MtkCameraParameters::KEY_DUALCAM_DEVICE_STATE_RECORDING);
    }
    return  mapToHidlCameraStatus(status);
}

/******************************************************************************
 * Stop a previously started recording.
 ******************************************************************************/
Return<void>
DualCameraDevice1::
stopRecording()
{
    CAM_TRACE_CALL();
    MY_LOGI("+");
    //
    {
        CAM_TRACE_NAME("adapterStopRecording");
        if (mpCamAdapter_W != 0)
        {
            mpCamAdapter_W->stopRecording();
        }
        if (mpCamAdapter_T != 0)
        {
            mpCamAdapter_T->stopRecording();
        }
    }
    //
    {
        CAM_TRACE_NAME("clientStopRecording");
        if (mpCamClient != 0)
        {
            mpCamClient->stopRecording();
        }
        // forward to registered clients
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->stopRecording();
        }
    }
    // ************************************************************************
    //
    // needReconstruct shall has the same logic as startRecording()
    //
    // ************************************************************************
    MBOOL needReconstruct = needReconstructRecordingPipe(mpParamsMgrV3, true);
    needReconstruct = needReconstruct || needReconstructRecordingPipe(mpParamsMgrV3_Sup, true);

    if (needReconstruct)
    {
        // flush hwsync
        mSyncMgr = NSCam::ISyncManager::getInstance(getInstanceId());
        if (mSyncMgr != NULL)
        {
            mSyncMgr->setEvent(0, NULL);
            mSyncMgr = NULL;
        }
        else
        {
            MY_LOGW("Cannot get syncmanager in stop preview. Can't flush hwsync");
        }
        // reset low layer flag and reduce the resource (e.g. 4K to FHD)
        if (mpCamAdapter_W != 0)
        {
            MY_LOGD("Wide stopPreview()");
            mpCamAdapter_W->stopPreview();
        }
        if (mpCamAdapter_T != 0)
        {
            MY_LOGD("Tele stopPreview()");
            mpCamAdapter_T->stopPreview();
        }

        // re-start
        if (mpCamAdapter_W != 0)
        {
            MY_LOGD("Wide startPreview()");
            mpCamAdapter_W->startPreview();
        }
        if (mpCamAdapter_T != 0)
        {
            MY_LOGD("Tele startPreview()");
            mpCamAdapter_T->startPreview();
        }
        isStoppreview = MFALSE;
    }

    updateDeviceState(MtkCameraParameters::KEY_DUALCAM_DEVICE_STATE_PREVIEWING);
    return Void();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
DualCameraDevice1::
takePicture()
{
    CAM_TRACE_CALL();
    MY_LOGI("+");
    //
    status_t status = OK;
    //
    if  ( mpCamAdapter == 0 )
    {
        MY_LOGE("NULL Camera Adapter");
        status = DEAD_OBJECT;
        goto lbExit;
    }
    //
    if(!(mLastEnableMsg & CAMERA_MSG_SHUTTER))
    {
        MY_LOGD("Disable shutter sound");
        disableMsgType(CAMERA_MSG_SHUTTER);
    }
    //
    {
        CAM_TRACE_NAME("clientTakePicture");
        //
        if  ( mpCamClient != 0 )
        {
            mpCamClient->takePicture();
        }
        // forward to registered clients
        Vector<sp<ICamClient> >::const_iterator it;
        for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
        {
            (*it)->takePicture();
        }
    }
    //
    {
        int featureMode = StereoSettingProvider::getStereoFeatureMode();
        if(featureMode == E_DUALCAM_FEATURE_ZOOM)
        {
            sp<ISyncManager> pSyncMgr = ISyncManager::getInstance(getInstanceId());
            if(pSyncMgr!=nullptr){
                pSyncMgr->lock(nullptr);
            }
        }
        Mutex::Autolock _l(mSyncLock);
        CAM_TRACE_NAME("adapterTakePicture");
        //  takePicture in Camera Adapter.
        if(mIsRaw16CBEnabled)
        {
            MY_LOGD("CAMERA_CMD_ENABLE_RAW16_CALLBACK:1");
            mpCamAdapter->sendCommand(CAMERA_CMD_ENABLE_RAW16_CALLBACK, 1, 0);
        }
        //
        if ( (strcmp(mpCamAdapter->getName(), MtkCameraParameters::APP_MODE_NAME_MTK_ZSD)
            && (strcmp(mpCamAdapter->getName(), MtkCameraParameters::APP_MODE_NAME_MTK_STEREO))) &&
             !recordingEnabled() )
        {
            // flush hwsync
            mSyncMgr = NSCam::ISyncManager::getInstance(getInstanceId());
            if (mSyncMgr != NULL)
            {
                mSyncMgr->setEvent(0, NULL);
                mSyncMgr = NULL;
            }
            else
            {
                MY_LOGW("Cannot get syncmanager in stop preview. Can't flush hwsync");
            }
            if (mpCamAdapter != mpCamAdapter_W) {
                mpCamAdapter_W->stopPreview();
            } else if (mpCamAdapter != mpCamAdapter_T) {
                mpCamAdapter_T->stopPreview();
            }
        }

        if(featureMode == E_DUALCAM_FEATURE_ZOOM)
        {

#if (MTKCAM_HAVE_DUAL_ZOOM_FUSION_SUPPORT == 1)
            // Fusion shot entry point
            StereoSettingProvider::setStereoShotMode(eShotMode_FusionShot);
            if (mpCamAdapter_W.get() == NULL || mpCamAdapter_T.get() == NULL)
            {
                MY_LOGE("Error: Wide CamAdatper(%p), Tele CamAdapter(%p)",
                                (void*)mpCamAdapter_W.get(),
                                (void*)mpCamAdapter_T.get());
                goto lbExit;
            }

            // make sure it is ZSD mode
            String8 pszZsdMode = mpParamsMgr->getStr(MtkCameraParameters::KEY_ZSD_MODE);
            if  (pszZsdMode != MtkCameraParameters::ON)
            {
                MY_LOGE("%s=%s, fusion shall work in ZSD mode !",
                    MtkCameraParameters::KEY_ZSD_MODE, pszZsdMode.string());
                status = UNKNOWN_ERROR;
                goto lbExit;
            }

            MY_LOGD("wait wide capture cmd done+");
            status = mpCamAdapter_W->takePicture();
            MY_LOGD("wait wide capture cmd done- (%d)", status);

            status_t tele_status = OK;

            // do takepicture simultaneously
            std::function<status_t(void)> main2TakeAction =
                    [this, &tele_status] () mutable {
                        MY_LOGD("wait tele capture cmd done+");
                        tele_status = mpCamAdapter_T->takePicture();
                        return tele_status;
                    };

            std::future<status_t> main2CaptureCmd =
                    std::async(std::launch::async, main2TakeAction);

            main2CaptureCmd.wait();
            MY_LOGD("wait tele capture cmd done- (%d)", tele_status);

            if (status != OK || tele_status != OK)
            {
                MY_LOGE("capture fail");
                status = UNKNOWN_ERROR;
            }
#else
            // for zoom, just takepicture to specific adapter
            status = mpCamAdapter->takePicture();
#endif // MTKCAM_HAVE_DUAL_ZOOM_FUSION_SUPPORT
        }
        else if(featureMode == E_STEREO_FEATURE_DENOISE ||
                featureMode == (E_STEREO_FEATURE_CAPTURE|E_STEREO_FEATURE_VSDOF) ||
                featureMode == (E_STEREO_FEATURE_MTK_DEPTHMAP))
        {
            {
                Mutex::Autolock _l(mPostProcCheckLock);
                android::NSPostProc::IImagePostProcessManager* pIIPPManager =
                            android::NSPostProc::IImagePostProcessManager::getInstance();
                MBOOL isPostProcAvailable = pIIPPManager->isAvailable();
                while(!isPostProcAvailable)
                {
                    MY_LOGD("wait post processor available +");
                    mPostProcCheckCond.waitRelative(mPostProcCheckLock, 100000000LL);// wait 100ms
                    isPostProcAvailable = pIIPPManager->isAvailable();
                    MY_LOGD("wait post processor available (%d)-", isPostProcAvailable);
                    if(isStoppreview)
                    {
                        MY_LOGD("camera is exit, return value directly");
                        return mapToHidlCameraStatus(OK);
                    }
                }
            }
            String8 s8ShotMode;
            uint32_t u4ShotMode;
            bool isDualCamShot = getPolicyDecision(s8ShotMode, u4ShotMode);
            if(isStoppreview)
            {
                goto lbExit;
            }

            MY_LOGD("decision shot: (%s:%d)", s8ShotMode.string(), u4ShotMode);
            StereoSettingProvider::setStereoShotMode(u4ShotMode);
            if(isDualCamShot)
            {
                if(mpCamAdapter_W.get() == nullptr ||
                   mpCamAdapter_T.get() == nullptr)
                {
                    MY_LOGE("Error: Cam1Adatper(%p) Cam2Adapter(%p)",
                                    (void*)mpCamAdapter_W.get(),
                                    (void*)mpCamAdapter_T.get());
                    goto lbExit;
                }
                 // do takepicture simultaneously
                std::future<status_t> main2TakeAction = std::async(
                    std::launch::async,
                    [this](){
                        status_t status = mpCamAdapter_T->takePicture();
                        return status;
                    }
                );
                status = mpCamAdapter_W->takePicture();
                MY_LOGD("wait capture cmd done+");
                status_t status_2 = main2TakeAction.get();
                MY_LOGD("wait capture cmd done-");
                if(status != OK || status_2 != OK)
                {
                    MY_LOGE("capture fail");
                    status = UNKNOWN_ERROR;
                }
            }
            else
            {
                // single cam capture
                status = mpCamAdapter->takePicture();
                sp<ISyncManager> pSyncMgr = ISyncManager::getInstance(getInstanceId());
                if(pSyncMgr!=nullptr){
                    pSyncMgr->unlock(nullptr);
                }
            }
        }
        else
        {
            MY_LOGE("Not support");
            goto lbExit;
        }
        if  ( OK != status )
        {
            MY_LOGE("CamAdapter->takePicture() returns: [%s(%d)]", ::strerror(-status), -status);
            goto lbExit;
        }
    }

lbExit:

    MY_LOGI("-");
    return   mapToHidlCameraStatus(status);
}

/******************************************************************************
 *  Start auto focus, the notification callback routine is called
 *  with CAMERA_MSG_FOCUS once when focusing is complete. autoFocus()
 *  will be called again if another auto focus is needed.
 ******************************************************************************/
Return<Status>
DualCameraDevice1::
autoFocus()
{
    FUNCTION_SCOPE;
    status_t status = OK;
    //
    waitStartPreviewDone();
    //
    if  ( ! previewEnabled() )
    {
        MY_LOGW("preview is not enabled");
        return mapToHidlCameraStatus(status);
    }
    //
    //disableMsgType(CAMERA_MSG_PREVIEW_METADATA);
    //
    if ( mpCamAdapter_W == 0 && mpCamAdapter_T == 0)
    {
        return mapToHidlCameraStatus(DEAD_OBJECT);
    }
    if (mpCamAdapter_W != 0)
    {
        status = mpCamAdapter_W->autoFocus();
    }
    if (status != OK)
    {
        return mapToHidlCameraStatus(status);
    }
    if (mpCamAdapter_T != 0)
    {
        status = mpCamAdapter_T->autoFocus();
    }
    return mapToHidlCameraStatus(status);
}


/******************************************************************************
 * Cancels auto-focus function. If the auto-focus is still in progress,
 * this function will cancel it. Whether the auto-focus is in progress
 * or not, this function will return the focus position to the default.
 * If the camera does not support auto-focus, this is a no-op.
 ******************************************************************************/
Return<Status>
DualCameraDevice1::
cancelAutoFocus()
{
    status_t status = OK;
    waitStartPreviewDone();
    //
    if ( mpCamAdapter_W == 0 && mpCamAdapter_T == 0)
    {
        return mapToHidlCameraStatus(OK);
    }
    if (mpCamAdapter_W != 0)
    {
        status = mpCamAdapter_W->cancelAutoFocus();
    }
    if (status != OK)
    {
        return mapToHidlCameraStatus(status);
    }
    if (mpCamAdapter_T != 0)
    {
        status = mpCamAdapter_T->cancelAutoFocus();
    }
    return mapToHidlCameraStatus(status);
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
DualCameraDevice1::
initDisplayClient(preview_stream_ops* window)
{
    CAM_TRACE_CALL();
#if 0//'1'!=MTKCAM_HAVE_DISPLAY_CLIENT
#warning "Not Build Display Client"
    MY_LOGD("Not Build Display Client");
    return  OK;
#else
    status_t status = OK;
    Size previewSize;
    //
    MY_LOGD("+ window(%p)", window);
    //
    //
    //  [1] Check to see whether the passed window is NULL or not.
    if (! window)
    {
        MY_LOGW("NULL window is passed into...");
        mbWindowReady = false;
        //
        if (mpDisplayClient != 0)
        {
            MY_LOGW("destroy the current display client(%p)...", mpDisplayClient.get());
            mpDisplayClient->uninit();
            mpDisplayClient.clear();
        }
        status = OK;
        goto lbExit;
    }
    mbWindowReady = true;
    //
    //
    //  [2] Get preview size.
    if (! queryPreviewSize(previewSize.width, previewSize.height))
    {
        MY_LOGE("queryPreviewSize");
        status = DEAD_OBJECT;
        goto lbExit;
    }
    //
    //
    //  [3] Initialize Display Client.
    if (mpDisplayClient != 0)
    {
        if (previewEnabled())
        {
            MY_LOGW("Do nothing since Display Client(%p) is already created after startPreview()",
                    mpDisplayClient.get());
            //          This method must be called before startPreview(). The one exception is that
            //          if the preview surface texture is not set (or set to null) before startPreview() is called,
            //          then this method may be called once with a non-null parameter to set the preview surface.
            status = OK;
            goto lbExit;
        }
        else
        {
            MY_LOGW("New window is set after stopPreview or takePicture. Destroy the current display client(%p)...",
                    mpDisplayClient.get());
            mpDisplayClient->uninit();
            mpDisplayClient.clear();
        }
    }
    //  [3.1] create a Display Client.
    mpDisplayClient = IDisplayClient::createInstance();
    if (mpDisplayClient == 0)
    {
        MY_LOGE("Cannot create mpDisplayClient");
        status = NO_MEMORY;
        goto lbExit;
    }
    //  Display Rotation
    if (mpParamsMgr->getDisplayRotationSupported())
    {
        MY_LOGD("orientation = %d", mOrientation);
        mpDisplayClient->SetOrientationForDisplay(mOrientation);
    }
    //  [3.2] initialize the newly-created Display Client.
    if (! mpDisplayClient->init())
    {
        MY_LOGE("mpDisplayClient init() failed");
        mpDisplayClient->uninit();
        mpDisplayClient.clear();
        status = NO_MEMORY;
        goto lbExit;
    }
    //  [3.3] set preview_stream_ops & related window info.
    if (! mpDisplayClient->setWindow(window, previewSize.width, previewSize.height,
                                     queryDisplayBufCount()))
    {
        status = INVALID_OPERATION;
        goto lbExit;
    }

    //  [3.4] set Image Buffer Provider Client if it exist.
    if (mpBridge != 0 && ! mpDisplayClient->setImgBufProviderClient(mpBridge))
    {
        status = INVALID_OPERATION;
        goto lbExit;
    }
    //
    //
    status = OK;
    //
lbExit:
    if (OK != status)
    {
        MY_LOGD("Cleanup...");
        mpDisplayClient->uninit();
        mpDisplayClient.clear();
    }
    //
    MY_LOGD("- status(%d)", status);
    return  status;
#endif//MTKCAM_HAVE_DISPLAY_CLIENT
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
DualCameraDevice1::
enableDisplayClient()
{
    status_t status = OK;
    Size previewSize;
    //
    MY_LOGD("+");
    //
    //  [1] Get preview size.
    if (! queryPreviewSize(previewSize.width, previewSize.height))
    {
        MY_LOGE("queryPreviewSize");
        status = DEAD_OBJECT;
        goto lbExit;
    }
    //  [2] Enable
    if (! mpDisplayClient->enableDisplay(previewSize.width, previewSize.height, queryDisplayBufCount(),
                                         mpBridge))
    {
        MY_LOGE("mpDisplayClient(%p)->enableDisplay()", mpDisplayClient.get());
        status = INVALID_OPERATION;
        goto lbExit;
    }
    //
    status = OK;
lbExit:
    MY_LOGD("- status(%d)", status);
    return  status;
}

/******************************************************************************
 *
 ******************************************************************************/
void
DualCameraDevice1::
startPreviewImp()
{
    CAM_TRACE_CALL();
    FUNCTION_SCOPE;
    //
    status_t status = OK;
    //
    if( !waitThreadSensorOnDone() )
    {
        MY_LOGE("init in thread failed");
        return;
    }

    status = dualAdapterStartPreview();

    if (mpParamsMgr->getRecordingHint())
    {
        MSize paramSize;
        mpParamsMgr->getVideoSize(&paramSize.w, &paramSize.h);
        mb4K2KVideoRecord = (paramSize.w*paramSize.h > IMG_1080P_SIZE) ? MTRUE : MFALSE;
    }
    isStoppreview = MFALSE;
lbExit:
    return;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
DualCameraDevice1::
initDualCameraAdapter()
{
    bool ret = false;
    //
    //  (1) Check to see if CamAdapter has existed or not.
    //mpCamAdapter = NULL;
    mpBridge->clearImgBufProviderClient();
    //

    // update StereoSetting for image ratio
    // get preview size
    int preview_width = 0;
    int preview_height = 0;
    mpParamsMgrV3->getParamsMgr()->getPreviewSize(&preview_width, &preview_height);
    MY_LOGD("width(%d) height(%d)", preview_width, preview_height);
    double ratio_4_3 = 4.0/3.0;
    double preview_ratio = ((double)preview_width) / ((double)preview_height);
    if(preview_ratio == ratio_4_3)
    {
        MY_LOGD("set to 4:3");
        StereoSettingProvider::setImageRatio(eRatio_4_3);
    }
    else
    {
        MY_LOGD("set to 16:9");
        StereoSettingProvider::setImageRatio(eRatio_16_9);
    }

    std::future<status_t> w_init_adapter_job;
    std::future<status_t> t_init_adapter_job;
    auto init_adapter = [this](bool isMain1) -> status_t
    {
        bool isValid = false;
        sp<ICamAdapter> adapter = (isMain1) ? mpCamAdapter_W : mpCamAdapter_T;
        if(adapter != nullptr)
        {
            if (ICamAdapter::isValidInstance(adapter))
            {
                // do nothing & just return true if the same app.
                MY_LOGD("valid camera adapter: %s", adapter->getName());
                isValid = true;
            }
            else
            {
                // cleanup the original one if different app.
                MY_LOGW("invalid camera adapter: %s", adapter->getName());
                adapter->uninit();
                adapter.clear();
            }
        }
        if(!isValid)
        {
            int32_t openId = (isMain1) ? this->getInstanceId() : this->getOpenId_Main2();
            sp<IParamsManagerV3> pParamMgr = (isMain1) ? mpParamsMgrV3 : mpParamsMgrV3_Sup;
            if(isMain1)
            {
                adapter = mpCamAdapter_W = ICamAdapter::createInstance(mDevName, openId, pParamMgr);
            }
            else
            {
                adapter = mpCamAdapter_T = ICamAdapter::createInstance(mDevName, openId, pParamMgr);
            }
            if (adapter != 0 && adapter->init())
            {
                //  (.1) init.
                adapter->setCallbacks(mpCamMsgCbInfo);
                adapter->enableMsgType(mpCamMsgCbInfo->mMsgEnabled);

                //  (.2) Invoke its setParameters
                if (OK != adapter->setParameters())
                {
                    //  If fail, it should destroy instance before return.
                    MY_LOGE("[%d] [%s]->setParameters() fail", isMain1, adapter->getName());
                    return UNKNOWN_ERROR;
                }

                //  (.3) Send to-do commands.
                {
                    Mutex::Autolock _lock(mTodoCmdMapLock);
                    for (size_t i = 0; i < mTodoCmdMap.size(); i++)
                    {
                        CommandInfo const& rCmdInfo = mTodoCmdMap.valueAt(i);
                        MY_LOGD("send queued cmd(%#x),args(%d,%d)", rCmdInfo.cmd, rCmdInfo.arg1, rCmdInfo.arg2);
                        adapter->sendCommand(rCmdInfo.cmd, rCmdInfo.arg1, rCmdInfo.arg2);
                    }
                    mTodoCmdMap.clear();
                }
            }
            else
            {
                MY_LOGE("[%d] [%s](%p)->init() fail", isMain1, adapter->getName(), adapter.get());
                return UNKNOWN_ERROR;
            }
        }
        return OK;
    };
    w_init_adapter_job = std::async(std::launch::async, init_adapter, true);
    t_init_adapter_job = std::async(std::launch::async, init_adapter, false);
    status_t w_result = w_init_adapter_job.get();
    status_t t_result = t_init_adapter_job.get();
    mpCamAdapter = mpCamAdapter_W;
    //
    mpBridge->addImgBufProviderClient(mpCamAdapter_W);
    if (mpCamAdapter_T != 0)
        mpBridge->addImgBufProviderClient(mpCamAdapter_T);

    if (mpDisplayClient != 0 && ! mpDisplayClient->setImgBufProviderClient(mpBridge))
    {
        MY_LOGE("mpDisplayClient->setImgBufProviderClient() fail");
        goto lbExit;
    }
    //  (.5) [CamClient] set Image Buffer Provider Client if needed.
    if (mpCamClient != 0 && ! mpCamClient->setImgBufProviderClient(mpBridge))
    {
        MY_LOGE("mpCamClient->setImgBufProviderClient() fail");
        goto lbExit;
    }
    //
    Vector<sp<ICamClient> >::const_iterator it;
    for (it = vmpCamClient.begin(); it != vmpCamClient.end(); ++it)
    {
        (*it)->setImgBufProviderClient(mpBridge) ;
    }
    ret = true;
lbExit:

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT32
DualCameraDevice1::
onEvent(
    MINT32 const i4OpenId           __attribute__((unused)),
    MINT32 arg1                     __attribute__((unused)),
    MINT32 arg2                     __attribute__((unused)),
    void* arg3                      __attribute__((unused)))
{
    Mutex::Autolock _l(mSyncLock);
    MY_LOGD("DualCameraDevice1::onEvent id(%d), 0x%08X, 0x%08X",i4OpenId, arg1, arg2);
    switch(arg1)
    {
        case MTK_SYNCMGR_MSG_NOTIFY_MASTER_ID:
        {
            if (i4OpenId == getInstanceId()) {
                mpCamAdapter = mpCamAdapter_W;
            } else {
                mpCamAdapter = mpCamAdapter_T;
            }
            mpCamClient->sendCommand(CAMERA_CMD_SET_FD_MAINCAM_ID, i4OpenId, 0);
            break;
        }
        default:
        {
            break;
        }
    }
    return 0;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
DualCameraDevice1::
updateSensorId(
)
{
    int i4DeviceNum = 0;
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    if(!pHalSensorList)
    {
        MY_LOGE("pHalSensorList == NULL");
        return 0;
    }
    //
    if(NSCam::Utils::CamManager::getInstance()->getSensorCount() > 0)
    {
        MY_LOGE("CamManager.getSensorCount() > 0, close them before open stereo mode.");
        return UNKNOWN_ERROR;
    }
    //
    i4DeviceNum = pHalSensorList->queryNumberOfSensors();
    if(i4DeviceNum < 3)
    {
        MY_LOGE("i4DeviceNum(%d) < 3, please check whether sensor modules are installed properly", i4DeviceNum);
        return UNKNOWN_ERROR;
    }
    else if(i4DeviceNum > 4)
    {
        MY_LOGW("i4DeviceNum(%d) > 4, it is weired to turn on stereo cam in such situation!", i4DeviceNum);
        return UNKNOWN_ERROR;
    }
    else
    {
        MY_LOGD("i4DeviceNum(%d)", i4DeviceNum);
    }

    uint32_t curDevIdx = pHalSensorList->querySensorDevIdx(getInstanceId());
    switch(curDevIdx)
    {
        // for main1 & main2, vsdof may use these sensor id.
        case SENSOR_DEV_MAIN:
        case SENSOR_DEV_MAIN_2:
            MY_LOGD("setup rear+rear sensor pair to StereoSettingProvider");
            StereoSettingProvider::setStereoProfile(STEREO_SENSOR_PROFILE_REAR_REAR);
            break;
        case SENSOR_DEV_SUB:
            MY_LOGD("setup front+front sensor pair to StereoSettingProvider");
            StereoSettingProvider::setStereoProfile(STEREO_SENSOR_PROFILE_FRONT_FRONT);
            break;
        default:
            MY_LOGE("Unrecognized opendId(%d)", getInstanceId());
            return UNKNOWN_ERROR;
    }

    int dummy_main1Id = -1;
    if(!StereoSettingProvider::getStereoSensorIndex(dummy_main1Id, mSensorId_Main2)){
        MY_LOGE("Cannot get sensor ids from StereoSettingProvider!");
        return UNKNOWN_ERROR;
    }
    else
    {
        MY_LOGD("main1 openId:%d, main2 openId:%d", getInstanceId(), mSensorId_Main2);
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
DualCameraDevice1::
setFeatureMode(
)
{
    status_t ret = UNKNOWN_ERROR;
    if(mDevName == MtkCameraParameters::APP_MODE_NAME_MTK_STEREO)
    {
        // no need to set feature mode again.
        // because vsdof mode already set before create DualCameraDevice1.
        ret = OK;
    }
    else
    {
        // denoise and dual cam zoom use same device.
        // using main2 id to decide current feature mode.
        MY_LOGD("Dual cam main2 id: %d", mSensorId_Main2);
        MY_LOGD("Dual cam main2 fmt: %d", StereoSettingProvider::getSensorRawFormat(mSensorId_Main2));
        if(SENSOR_RAW_MONO == StereoSettingProvider::getSensorRawFormat(mSensorId_Main2))
        {
            MY_LOGD("denosie mode");
            StereoSettingProvider::setStereoFeatureMode(E_STEREO_FEATURE_DENOISE);
            ret = OK;
        }
        else
        {
            // *****************************************************************
            // MTKCAM_HAVE_DUAL_ZOOM_SUPPORT
            // MTKCAM_HAVE_DUAL_ZOOM_FUSION_SUPPORT
            // Both use the same Stereo Feature Mode - E_DUALCAM_FEATURE_ZOOM
            // *****************************************************************
            MY_LOGD("dualcam zoom mode");
            StereoSettingProvider::setStereoFeatureMode(E_DUALCAM_FEATURE_ZOOM);
            ret = OK;
        }
    }
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
DualCameraDevice1::
setSensorModuleType()
{
    MINT32 value = 0;
    // get main1 sensor format
    MUINT sensorRawFmtType_Main1 = StereoSettingProvider::getSensorRawFormat(getInstanceId());
    // get main2 sensor format
    MUINT sensorRawFmtType_Main2 = StereoSettingProvider::getSensorRawFormat(getOpenId_Main2());

    if(sensorRawFmtType_Main2 == SENSOR_RAW_MONO)
    {
        MY_LOGD("Sensor module is B+M");
        value = v1::Stereo::BAYER_AND_MONO;
    }
    else
    {
        MY_LOGD("Sensor module is B+B");
        value = v1::Stereo::BAYER_AND_BAYER;
    }
    StereoSettingProvider::setStereoModuleType(value);
    return OK;
}
/******************************************************************************
 * logic to this function should be the same as
 * FeatureFlowControl::needReconstructRecordingPipe()
 ******************************************************************************/
MBOOL
DualCameraDevice1::
needReconstructRecordingPipe(sp<IParamsManagerV3> param, MBOOL mIsRecordMode)
{
    MSize paramSize;
    param->getParamsMgr()->getVideoSize(&paramSize.w, &paramSize.h);
    MBOOL b4K2KVideoRecord = (paramSize.w*paramSize.h > IMG_1080P_SIZE) ? MTRUE : MFALSE;

    MBOOL ret = MFALSE;

    // mb4K2KVideoRecord in FeatureFlowControl is assigned in constructRecordingPipeline()
    // which is called after FeatureFlowControl::needReconstructRecordingPipe()
    // the value is previous setting (ex: video mode and only preview), not current operation
    // as the timing here in DualCam1Device, it means current operation.
    //
    // DualCam1Device::mb4K2KVideoRecord     (it means current operation)
    // FeatureFlowControl::mb4K2KVideoRecord (it means previous setting)
    //
    // so 4k2k force to reconstruct when recording
    if( mb4K2KVideoRecord != b4K2KVideoRecord)
    {
        ret = MTRUE;
    }
    MY_LOGD("(0x%p) param(%dx%d), b4K2K(%d), ret(%d)", param.get(),
             paramSize.w, paramSize.h, mb4K2KVideoRecord, ret);
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void
DualCameraDevice1::
updateDeviceState(MINT32 state)
{
    if(mpParamsMgr != nullptr)
    {
        mpParamsMgr->set(MtkCameraParameters::KEY_DUALCAM_DEVICE_STATE, state);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
DualCameraDevice1::
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
DualCameraDevice1::
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
/******************************************************************************
 *
 ******************************************************************************/
status_t
DualCameraDevice1::
dualAdapterStartPreview()
{
    CAM_TRACE_CALL();
    MY_LOGI("+");
    status_t status = OK;

    auto startPreviewFun = [](sp<ICamAdapter>&& camAdapter) -> status_t
    {
        CAM_LOGD("[start_preview] +: OpenId: %d", camAdapter->getOpenId());

        status_t ret = OK;
        auto start = std::chrono::system_clock::now();
        {
            ret = camAdapter->startPreview();
        }
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);

        CAM_LOGD("[start_preview] -: name: %d, elapsed time: %lld ms", camAdapter->getOpenId(), elapsedTime.count());
        return ret;
    };

    status_t wideStartPreviewResult = OK;
    status_t teleStartPreviewResult = OK;
    {
        auto start = std::chrono::system_clock::now();
        {
            std::future<status_t> wideStartPreviewJob = std::async(std::launch::deferred, startPreviewFun, mpCamAdapter_W);
            wideStartPreviewResult = wideStartPreviewJob.get();
            //
            if(!::getBGStartTelePreviewSupported())
            {
                mBgStartTelePreviewJob = std::async(std::launch::deferred, startPreviewFun, mpCamAdapter_T);
                teleStartPreviewResult = mBgStartTelePreviewJob.get();
            }
            else
            {
                MY_LOGD("background start tele preview is supported");
                if(mBgStartTelePreviewJob.valid())
                {
                    status_t tmp = mBgStartTelePreviewJob.get();
                    MY_LOGD("wait done for previous background start tele preview, result: %d", tmp);
                }
                mBgStartTelePreviewJob = std::async(std::launch::async, startPreviewFun, mpCamAdapter_T);
            }
        }
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);

        MY_LOGI("start dualcam preview elapsed time: %lld ms", elapsedTime.count());
    }
    if((OK != wideStartPreviewResult) || (OK != teleStartPreviewResult))
    {
        MY_LOGE("startPreview() in CameraAdapter returns: [%s(%d)]", ::strerror(-status), -status);
        sp<IFrameworkCBThread> spFrameworkCBThread = IFrameworkCBThread::createInstance(this->getInstanceId(),mpCamMsgCbInfo);
        spFrameworkCBThread->init();
        IFrameworkCBThread::callback_data cbData;
        cbData.callbackType = IFrameworkCBThread::CALLBACK_TYPE_NOTIFY;
        cbData.type         = CAMERA_MSG_ERROR;
        cbData.ext1         = CAMERA_ERROR_SERVER_DIED;
        cbData.ext2         = 0;
        spFrameworkCBThread->postCB(cbData);
        spFrameworkCBThread->uninit();
        spFrameworkCBThread = NULL;
        status = OK;
        MY_LOGE("startPreview fail, return OK, callback CAMERA_ERROR_SERVER_DIED");
        return status;
    }
    // register callback to syncmanager
    {
        mSyncMgr = NSCam::ISyncManager::getInstance(getInstanceId());
        if (mSyncMgr != NULL)
        {
            mSyncMgr->registerMgrCb(this);
            mSyncMgr = NULL;
        }
        else
        {
            MY_LOGW("Cannot get syncmanager in start preview");
        }
    }

    MY_LOGI("- status(%d)", status);
    return  status;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
DualCameraDevice1::
getPolicyDecision(
    String8& rShotModeStr,
    uint32_t& rShotMode
) const
{
    bool usingDualCamShot = false;
#if DUALCAM_SHOT
    MBOOL isForceMode = false;
    MUINT8 sceneResult = (MUINT8)NSCam::DENOISE_SCENE_RESULT::DENOISE_SCENE_NOT_SUPPORT;

    // CShot not supported, will use single cam shot to hanle it
    rShotMode = mpParamsMgr->getShotMode();
    if(rShotMode == eShotMode_ContinuousShot){
        MY_LOGD("dualcam does not support CShot!");
        return usingDualCamShot;
    }

    // 0. get sync mgr and lock it for feature decision
    sp<ISyncManager> pSyncMgr = ISyncManager::getInstance(getInstanceId());
    if(pSyncMgr!=nullptr)
    {
        PolicyLockParams policyLockParams;
        pSyncMgr->lock(&policyLockParams);

        MY_LOGD("bUseDualCamShot:%d, bIsMain2On:%d",
            policyLockParams.bUseDualCamShot, // bUseDualCamShot: main2 is ON and 3A is stable
            policyLockParams.bIsMain2On       // bIsMain2On: main2 is ON or just about to turn ON, 3A may not be stable yet
        );

        // 1. check force mode
        MINT32 forceDeNoiseMode = property_get_int32("debug.camera.forceDenoiseShot", -1);
        MINT32 forceDualCamMode = property_get_int32("debug.camera.forceDualCamShot", -1);

        if(forceDeNoiseMode == 0 || forceDualCamMode == 0){
            MY_LOGD("force using ZSDShot");
            sceneResult = (MUINT8)NSCam::DENOISE_SCENE_RESULT::DENOISE_SCENE_NOT_SUPPORT;
            isForceMode = true;
        }

        if(policyLockParams.bIsMain2On){
            switch(forceDeNoiseMode){
                case 1:
                    MY_LOGD("force using BMDNShot");
                    sceneResult = (MUINT8)NSCam::DENOISE_SCENE_RESULT::DENOISE_SCENE_DENOISE;
                    isForceMode = true;
                    break;
                case 2:
                    MY_LOGD("force using MFHRShot");
                    sceneResult = (MUINT8)NSCam::DENOISE_SCENE_RESULT::DENOISE_SCENE_HIGH_RES;
                    isForceMode = true;
                    break;
            }
            switch(forceDualCamMode){
                case 3:
                    MY_LOGD("force using DCMFShot");
                    sceneResult = (MUINT8)NSCam::DENOISE_SCENE_RESULT::DUALCAM_SCENE_DCMF;
                    isForceMode = true;
                    break;
            }
        }

        // 2. if not force mode, check with policy
        if(!isForceMode){
            if(policyLockParams.bUseDualCamShot){
                int featureMode = StereoSettingProvider::getStereoFeatureMode();
                if(featureMode == (E_STEREO_FEATURE_CAPTURE|E_STEREO_FEATURE_VSDOF)||
                   featureMode == (E_STEREO_FEATURE_MTK_DEPTHMAP))
                {
                    sceneResult = (MUINT8)NSCam::DENOISE_SCENE_RESULT::DUALCAM_SCENE_DCMF;
                }
                else
                {
                sp<IFrameInfo> pFrameInfo = IResourceContainer::getInstance(getInstanceId())->queryLatestFrameInfo();
                MY_LOGW_IF((pFrameInfo == nullptr), "Can't query Latest FrameInfo!");
                IMetadata metadata;
                if (pFrameInfo.get()){
                    pFrameInfo->getFrameMetadata(eSTREAMID_META_APP_DYNAMIC_P2, metadata);
                }
                // get shot mode from p2 app matadata
                {
                    IMetadata::IEntry entry = metadata.entryFor(MTK_STEREO_FEATURE_SHOT_MODE);
                    if(!entry.isEmpty()) {
                        sceneResult = entry.itemAt(0, Type2Type<MUINT8>());
                    }else{
                        MY_LOGW("Cannot get denoise scene result");
                    }
                    }
                }
            }else{
                MY_LOGD("dual cam is not ready, using default shotmode");
                sceneResult = (MUINT8)NSCam::DENOISE_SCENE_RESULT::DENOISE_SCENE_NOT_SUPPORT;
            }
        }
    }

    // 3. update shot mode string
    switch(sceneResult){
        case (MUINT8)NSCam::DENOISE_SCENE_RESULT::DENOISE_SCENE_DENOISE:
            MY_LOGD("using BMDNShot");
            rShotModeStr = "BMDN";
            rShotMode = NSCam::eShotMode_BMDNShot;
            usingDualCamShot = true;
            break;
        case (MUINT8)NSCam::DENOISE_SCENE_RESULT::DENOISE_SCENE_HIGH_RES:
            MY_LOGD("using MFHRShot");
            rShotModeStr = "MFHR";
            rShotMode = NSCam::eShotMode_MFHRShot;
            usingDualCamShot = true;
            break;
        case (MUINT8)NSCam::DENOISE_SCENE_RESULT::DUALCAM_SCENE_DCMF:
            MY_LOGD("using DCMFShot");
            rShotModeStr = "DCMF";
            rShotMode = NSCam::eShotMode_DCMFShot;
            usingDualCamShot = true;
            // for DCMF shot, it has to wait until main2 is ready
            // How to check main2 is ready? (get main2 dynamic metadata)
            {
                Mutex::Autolock _l(mPreviewMetaLock);
                sp<IFrameInfo> pFrameInfo = IResourceContainer::getInstance(getOpenId_Main2())->queryLatestFrameInfo();
                while(pFrameInfo == nullptr)
                {
                    MY_LOGW("Can't query Latest FrameInfo! wait +");
                    mPreviewMetaCond.waitRelative(mPreviewMetaLock, 33000000LL);//33ms
                    if(isStoppreview)
                    {
                        MY_LOGD("stoppreview break loop (Latest FrameInfo)");
                        return usingDualCamShot;
                    }
                    pFrameInfo = IResourceContainer::getInstance(getOpenId_Main2())->queryLatestFrameInfo();
                    MY_LOGW("Can't query Latest FrameInfo! wait -");
                }
                IMetadata metadata;
                if (pFrameInfo.get()){
                    pFrameInfo->getFrameMetadata(eSTREAMID_META_HAL_DYNAMIC_P1, metadata);
                }
                while(metadata.isEmpty())
                {
                    MY_LOGW("Can't get p2 metadata! wait +");
                    mPreviewMetaCond.waitRelative(mPreviewMetaLock, 33000000LL);//33ms
                    if(isStoppreview)
                    {
                        MY_LOGD("stoppreview break loop (p2 metadata)");
                        return usingDualCamShot;
                    }
                    pFrameInfo->getFrameMetadata(eSTREAMID_META_HAL_DYNAMIC_P1, metadata);
                    MY_LOGW("Can't get p2 metadata! wait -");
                }
            }
            break;
        default:
            MY_LOGD("using ZSDShot");
            rShotModeStr = "ZSD";
            rShotMode = NSCam::eShotMode_ZsdShot;
    }

    MY_LOGD("shot(%s) force(%d) usingDualCamShot(%d)", rShotModeStr.string(), isForceMode, usingDualCamShot);
#endif
    return usingDualCamShot;
}
