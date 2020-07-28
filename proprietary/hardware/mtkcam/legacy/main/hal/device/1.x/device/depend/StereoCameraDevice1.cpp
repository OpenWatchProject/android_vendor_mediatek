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
#include <mtkcam/hwutils/CamManager.h>
#endif
//
#include <mtkcam/v1/camutils/FrameworkCBThread.h>
using namespace android::MtkCamUtils;
// for query static stereo data
#include <mtkcam/featureio/stereo_setting_provider.h>
//
#include "MyUtils.h"
#include "StereoCameraDevice1.h"
//
#if (MTKCAM_HAVE_AEE_FEATURE == 1)
#include <aee.h>
#endif
#include <cutils/properties.h>

using namespace android;
using namespace NSCam;
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
    CAM_LOGD("[StereoCameraDevice1::%s] +" , __FUNCTION__);
    return new StereoCameraDevice1(deviceManager,
                                   instanceName,
                                   instanceId,
                                   rDevName);
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
#if '1'==MTKCAM_HAVE_3A_HAL
    , mpHal3a_Main(NULL)
    , mpHal3a_Main2(NULL)
    , mpSync3AMgr(NULL)
#endif
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    , mpHalSensor(NULL)
#endif
    //
    , mThreadHandle(0)
    , mbThreadRunning(false)
    , mRet(false)
    , mDisableWaitSensorThread(false)

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

#ifdef MTKCAM_HAVE_SENSOR_HAL_STEREO_MODE
#if '1'== MTKCAM_HAVE_SENSOR_HAL
bool
StereoCameraDevice1::
setSensorMode(MBOOL enable)
{
    MUINT32 main1_syncMode,main2_syncMode,dualmode;
    MINT HalSensorRet;
    if(enable)
    {
        HalSensorRet = mpHalSensor->sendCommand(SENSOR_DEV_MAIN,SENSOR_CMD_GET_SENSOR_SYNC_MODE_CAPACITY,
                                                (MUINTPTR)&main1_syncMode, 0 , 0 );
        if(HalSensorRet)
        {
            MY_LOGE("main1 seneor get master/slave mode failed: %d", HalSensorRet);
            return false;
        }
        HalSensorRet = mpHalSensor->sendCommand(SENSOR_DEV_MAIN_2,SENSOR_CMD_GET_SENSOR_SYNC_MODE_CAPACITY,
                                                (MUINTPTR)&main2_syncMode, 0 , 0 );

        if(HalSensorRet)
        {
            MY_LOGE("main2 seneor get master/slave mode failed: %d", HalSensorRet);
            return false;
        }

        MY_LOGD("get from sensor: main1_syncMode=%d, main2_syncMode=%d",main1_syncMode,main2_syncMode);
        int32_t SkipSyncCheck = ::property_get_int32("debug.camera.skip.synccheck", 0);
        MY_LOGD("SkipSyncCheck= %d", SkipSyncCheck);
        if(!SkipSyncCheck)
        {
            if((main1_syncMode & SENSOR_MASTER_SYNC_MODE)&&(main2_syncMode & SENSOR_SLAVE_SYNC_MODE))
            {
                main1_syncMode = SENSOR_MASTER_SYNC_MODE;
                main2_syncMode = SENSOR_SLAVE_SYNC_MODE;
            }
            else if((main1_syncMode & SENSOR_SLAVE_SYNC_MODE)&&(main2_syncMode & SENSOR_MASTER_SYNC_MODE))
            {
                main1_syncMode = SENSOR_SLAVE_SYNC_MODE;
                main2_syncMode = SENSOR_MASTER_SYNC_MODE;
            }
            else
            {
                MY_LOGE("select sensor master/slave mode failed: main1_syncMode=%d, main2_syncMode=%d",
                        main1_syncMode,main2_syncMode);
                return false;
            }
        }
        else
        {
            main1_syncMode = SENSOR_MASTER_SYNC_MODE;
            main2_syncMode = SENSOR_MASTER_SYNC_MODE;
            MY_LOGI("SkipSyncCheck = %d: set main1_syncMode & main2_syncMode to master mode",SkipSyncCheck);
        }
        MY_LOGI("set to sensor: main1_syncMode=%d, main2_syncMode=%d",main1_syncMode,main2_syncMode);
        HalSensorRet = mpHalSensor->sendCommand(SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_SYNC_MODE,
                                        (MUINTPTR)&main1_syncMode, 0 , 0 );
        if(HalSensorRet)
        {
            MY_LOGE("sensor get master/slave mode failed: %d", HalSensorRet);
            return false;
        }

        HalSensorRet = mpHalSensor->sendCommand(SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_SYNC_MODE,
                                        (MUINTPTR)&main2_syncMode, 0 , 0 );
        if(HalSensorRet)
        {
            MY_LOGE("sensor get master/slave mode failed: %d", HalSensorRet);
            return false;
        }

        dualmode =1;
        HalSensorRet = mpHalSensor->sendCommand(SENSOR_DEV_MAIN, SENSOR_CMD_SET_DUAL_CAM_MODE,
                                        (MUINTPTR)&dualmode, 0 , 0 );
        if(HalSensorRet)
        {
            MY_LOGE("sensor set to dualcam mode failed: %d", HalSensorRet);
            return false;
        }
        MY_LOGD("sensor set to dualcam mode =%d done", dualmode);

    }
    else
    {
        // disable sensor dualcam mode
        main1_syncMode = SENSOR_MASTER_SYNC_MODE;
        main2_syncMode = SENSOR_MASTER_SYNC_MODE;
        HalSensorRet = mpHalSensor->sendCommand(SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_SYNC_MODE,
                                        (MUINTPTR)&main1_syncMode, 0 , 0 );
        if(HalSensorRet)
        {
            MY_LOGE("sensor get master/slave mode failed: %d", HalSensorRet);
            return false;
        }

        HalSensorRet = mpHalSensor->sendCommand(SENSOR_DEV_MAIN_2, SENSOR_CMD_SET_SENSOR_SYNC_MODE,
                                        (MUINTPTR)&main2_syncMode, 0 , 0 );
        if(HalSensorRet)
        {
            MY_LOGE("sensor get master/slave mode failed: %d", HalSensorRet);
            return false;
        }

        dualmode =0;
        HalSensorRet = mpHalSensor->sendCommand(SENSOR_DEV_MAIN, SENSOR_CMD_SET_DUAL_CAM_MODE,
                                        (MUINTPTR)&dualmode, 0 , 0 );
        if(HalSensorRet)
        {
            MY_LOGE("sensor set to dualcam mode failed: %d", HalSensorRet);
            return false;
        }
        MY_LOGD("sensor set to dualcam mode =%d done", dualmode);
    }

    return true;
}
#endif
#endif


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

#if '1'==MTKCAM_HAVE_SENSOR_HAL
    // search sensor and update to property setting
    NSCam::IHalSensorList* pHalSensorList = NSCam::IHalSensorList::get();
    if(!pHalSensorList)
    {
        MY_LOGE("pHalSensorList == NULL");
        return ret;
    }

    // search sensor & update to parameter manager
    size_t const sensorNum = pHalSensorList->searchSensors();
    MY_LOGD("pHalSensorList:%p searchSensors:%zu queryNumberOfSensors:%d",
            pHalSensorList, sensorNum, pHalSensorList->queryNumberOfSensors());
    //Stereo Feature: need Main, Main2 sensors
    int32_t iSensorsList = 0;
    for (size_t i = 0; i < sensorNum; i++)
    {
        char szDeviceId[10];
        ::sprintf(szDeviceId, "%d", i);

        uint32_t const curDevIdx = pHalSensorList->querySensorDevIdx(i);
        iSensorsList |= curDevIdx;

        switch  (curDevIdx)
        {
        case SENSOR_DEV_MAIN_2:{
            String8 const s8Main2IdKey = String8("MTK_SENSOR_DEV_MAIN_2");
            String8 const s8Main2IdVal = String8(szDeviceId);
            NSCam::Utils::Property::set(s8Main2IdKey, s8Main2IdVal);
            MY_LOGI("main2 Camera found %zu", i);
            }break;
            //
        case SENSOR_DEV_MAIN:{
            String8 const s8MainIdKey = String8("MTK_SENSOR_DEV_MAIN");
            String8 const s8MainIdVal = String8(szDeviceId);
            Utils::Property::set(s8MainIdKey, s8MainIdVal);
            MY_LOGI("main1 Camera found %zu", i);
            }break;
            //
        case SENSOR_DEV_SUB:{
            String8 const s8SubIdKey = String8("MTK_SENSOR_DEV_SUB");
            String8 const s8SubIdVal = String8(szDeviceId);
            Utils::Property::set(s8SubIdKey, s8SubIdVal);
            MY_LOGI("sub Camera found %zu", i);
            }break;
            //
        default:
            break;
        }
    }
    MY_LOGD("iSensorsList=0x%08X", iSensorsList);
#endif

    if(!StereoSettingProvider::getStereoSensorIndex(mSensorId_Main, mSensorId_Main2))
    {
        MY_LOGE("Cannot get sensor ids from StereoSettingProvider!");
    }
    MY_LOGD("mSensorId_Main = %d, mSensorId_Main2 = %d",getOpenId_Main(),getOpenId_Main2());
    if  ( (-1 == getOpenId_Main()) || (-1 == getOpenId_Main2()) )
    {
        MY_LOGE("no dual main sensor");
        goto lbExit;
    }
    //
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
                    goto lbExit;
                }
            }
            //
            profile.print("Resource -");
            CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Resource -");
        }
#endif  //MTKCAM_HAVE_CAMDRV

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
        if( NSCam::IHalSensorList::get()->queryType( getOpenId_Main() ) == NSCam::NSSensorType::eYUV ||
            NSCam::IHalSensorList::get()->queryType( getOpenId_Main2() ) == NSCam::NSSensorType::eYUV )
        {
            if( !waitThreadSensorOnDone() )
            {
                MY_LOGE("init in thread failed");
                goto lbExit;
            }
        }
    }
#endif  //MTKCAM_HAVE_SENSOR_HAL

    //[3] Create and open 3A HAL
    //--------------------------------------------------------------------------
#if '1'==MTKCAM_HAVE_3A_HAL
    {
        CAM_TRACE_NAME("init(3A)");
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "3A Hal +");

    #ifdef MTKCAM_HAVE_3A_HAL_CAMSV_MODE
        // set 3A hal main2 camsv mode on
        String8 const s8DualCamWithCamsv("IS_DUALCAM_CAMSV");
        String8 const s8DualCamWithCamsvValue = String8("1");
        NSCam::Utils::Property::set(s8DualCamWithCamsv, s8DualCamWithCamsvValue);
        MY_LOGD("notice 3A HAL main2 to use CAMSV path");
    #endif   //MTKCAM_HAVE_3A_HAL_CAMSV_MODE

        mpSync3AMgr = NS3A::ISync3AMgr::getInstance();
        if  ( ! mpSync3AMgr ) {
            MY_LOGE("ISync3AMgr::getInstance() fail");
            goto lbExit;
        }
        if  ( ! mpSync3AMgr->init(0, getOpenId_Main(), getOpenId_Main2()) ) {
            MY_LOGE("mpSync3AMgr->init fail");
            goto lbExit;
        }
        //
        mpHal3a_Main = NS3A::IHal3A::createInstance(
                NS3A::IHal3A::E_Camera_1,
                getOpenId_Main(),
                LOG_TAG);
        if  ( ! mpHal3a_Main ) {
            MY_LOGE("IHal3A::createInstance() fail");
            goto lbExit;
        }
        mpHal3a_Main2 = NS3A::IHal3A::createInstance(
                NS3A::IHal3A::E_Camera_1,
                getOpenId_Main2(),
                LOG_TAG);
        if  ( ! mpHal3a_Main2 ) {
            MY_LOGE("IHal3A::createInstance() fail: main2");
            goto lbExit;
        }
        profile.print("3A Hal -");
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "3A Hal -");
    }
#endif  //MTKCAM_HAVE_3A_HAL

    //[4] Devcie Base onOpenLocked scenario
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
    //  (1) Uninit Base
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
    if  ( mpSync3AMgr )
    {
        mpSync3AMgr->uninit();
        mpSync3AMgr = NULL;
    }
    UNINIT_PIPELINE(job_uninit3a_main, uninit3A, getOpenId_Main(), mpHal3a_Main);
    UNINIT_PIPELINE(job_uninit3a_main2, uninit3A, getOpenId_Main2(), mpHal3a_Main2);
    #ifdef MTKCAM_HAVE_3A_HAL_CAMSV_MODE
    // set 3A hal main2 camsv mode off
    String8 const s8DualCamWithCamsv("IS_DUALCAM_CAMSV");
    String8 const s8DualCamWithCamsvValue = String8("0");
    NSCam::Utils::Property::set(s8DualCamWithCamsv, s8DualCamWithCamsvValue);
    MY_LOGD("notice 3A HAL main2 to close CAMSV path");
    #endif
#endif  //MTKCAM_HAVE_3A_HAL
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    #ifdef MTKCAM_HAVE_SENSOR_HAL_STEREO_MODE
    if( !setSensorMode(MFALSE))
    {
        MY_LOGE("disable sensor dualcam mode failed");
    }
    #endif
    UNINIT_PIPELINE(job_powerOff_main, powerOffSensor, getOpenId_Main(), mpHalSensor);
    UNINIT_PIPELINE(job_powerOff_main2, powerOffSensor, getOpenId_Main2(), mpHalSensor);
    //
    for(MUINT32 i = 0 ;i<vThread.size();++i)
    {
        vThread[i].wait();
    }
    // distory sensor hal
    mpHalSensor->destroyInstance(USER_NAME);
    mpHalSensor = NULL;
#endif  //MTKCAM_HAVE_SENSOR_HAL
#undef UNINIT_PIPELINE

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
    MY_LOGD("+");
    bool    ret = false;
    //  (1) Open Sensor
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    CAM_TRACE_CALL();
    Utils::CamProfile  profile(__FUNCTION__, "StereoCam1Device");
    CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Sensor Hal +");
    // power on sensor
    MUINT pIndex[2] = { (MUINT)getOpenId_Main(), (MUINT)getOpenId_Main2() };
    MUINT const main1Index = getOpenId_Main();
    MUINT const main2Index = getOpenId_Main2();
    SensorStaticInfo sensorStaticInfo;

    NSCam::IHalSensorList* pHalSensorList = NSCam::IHalSensorList::get();
    if(!pHalSensorList)
    {
        MY_LOGE("pHalSensorList == NULL");
        goto lbExit;
    }
    mpHalSensor = pHalSensorList->createSensor( USER_NAME, 2, pIndex);
    if(mpHalSensor == NULL)
    {
       MY_LOGE("mpHalSensor is NULL");
       goto lbExit;
    }
    //
    // work around
    // In stereo mode, Main1 needs power on first.
    // Power on main1 manual.
    if( !mpHalSensor->powerOn(USER_NAME, 1, &main1Index) )
    {
        MY_LOGE("sensor power on failed: %d", pIndex[0]);
        goto lbExit;
    }
    if( !mpHalSensor->powerOn(USER_NAME, 1, &main2Index) )
    {
        MY_LOGE("sensor power on failed: %d", pIndex[1]);
        goto lbExit;
    }

#ifdef MTKCAM_HAVE_SENSOR_HAL_STEREO_MODE
    if( !setSensorMode(MTRUE))
    {
        MY_LOGE("enable sensor dualcam mode failed");
        goto lbExit;
    }
    // query main2 info
    pHalSensorList->querySensorStaticInfo(SENSOR_DEV_MAIN_2, &sensorStaticInfo);
    if(sensorStaticInfo.sensorType == SENSOR_TYPE_YUV)
    {
        // Main2 cam is yuv sensor.
        String8 const s8IsMain2YUV("IS_MAIN2_YUV");
        String8 const s8IsMain2YUVValue = String8("1");
        NSCam::Utils::Property::set(s8IsMain2YUV, s8IsMain2YUVValue);
        MY_LOGD("main2 is yuv sensor, sensorStaticInfo.sensorType=%d",sensorStaticInfo.sensorType);

        String8 const s8IsMain2Mono("IS_MAIN2_MONO");
        String8 const s8IsMain2MonoValue = String8("0");
        NSCam::Utils::Property::set(s8IsMain2Mono, s8IsMain2MonoValue);
        MY_LOGD("main2 is not mono sensor, sensorStaticInfo.rawFmtType=%d",sensorStaticInfo.rawFmtType);
    }
    else
    {
        // Main2 cam is raw sensor.
        String8 const s8IsMain2YUV("IS_MAIN2_YUV");
        String8 const s8IsMain2YUVValue = String8("0");
        NSCam::Utils::Property::set(s8IsMain2YUV, s8IsMain2YUVValue);
        MY_LOGD("main2 is raw sensor, sensorStaticInfo.sensorType=%d",sensorStaticInfo.sensorType);

        if (sensorStaticInfo.rawFmtType == SENSOR_RAW_MONO)
        {
            // Main2 cam is mono sensor.
            String8 const s8IsMain2Mono("IS_MAIN2_MONO");
            String8 const s8IsMain2MonoValue = String8("1");
            NSCam::Utils::Property::set(s8IsMain2Mono, s8IsMain2MonoValue);
            MY_LOGD("main2 is mono sensor, sensorStaticInfo.rawFmtType=%d",sensorStaticInfo.rawFmtType);
        }
        else
        {
            String8 const s8IsMain2Mono("IS_MAIN2_MONO");
            String8 const s8IsMain2MonoValue = String8("0");
            NSCam::Utils::Property::set(s8IsMain2Mono, s8IsMain2MonoValue);
            MY_LOGD("main2 is not mono sensor, sensorStaticInfo.rawFmtType=%d",sensorStaticInfo.rawFmtType);
        }
    }
#endif
    //
    profile.print("Sensor Hal -");
    CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Sensor Hal -");
#endif  //MTKCAM_HAVE_SENSOR_HAL

    ret = true;
lbExit:
    MY_LOGD("-");
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
    NS3A::IHal3A*& hal3A
)
{
    MY_LOGD("(%d) +", openId);
    if(hal3A == nullptr)
    {
        MY_LOGW("(%d) hal3a is nullptr", openId);
        return OK;
    }
    hal3A->destroyInstance(LOG_TAG);
    hal3A = NULL;
    MY_LOGD("(%d) -", openId);
    return OK;
}
