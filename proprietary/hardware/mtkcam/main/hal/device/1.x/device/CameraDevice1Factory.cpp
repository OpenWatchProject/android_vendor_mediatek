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

#include "CameraDevice1Base.h"
#include "MyUtils.h"
//
#include <cutils/properties.h>
#include <dlfcn.h>
//
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
#if (MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1)
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#endif // (MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1)
//
using namespace android;
using namespace NSCam;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
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
static
String8 const
queryClientAppMode()
{
    /*
    Before opening camera, client must call
    Camera::setProperty(
        String8(MtkCameraParameters::PROPERTY_KEY_CLIENT_APPMODE),
        String8(MtkCameraParameters::APP_MODE_NAME_MTK_xxx)
    ),
    where MtkCameraParameters::APP_MODE_NAME_MTK_xxx = one of the following:
        MtkCameraParameters::APP_MODE_NAME_DEFAULT
        MtkCameraParameters::APP_MODE_NAME_MTK_ENG
        MtkCameraParameters::APP_MODE_NAME_MTK_ATV
        MtkCameraParameters::APP_MODE_NAME_MTK_S3D
        MtkCameraParameters::APP_MODE_NAME_MTK_VT
    */
    String8 const s8ClientAppModeKey(MtkCameraParameters::PROPERTY_KEY_CLIENT_APPMODE);
    String8    s8ClientAppModeVal(MtkCameraParameters::APP_MODE_NAME_DEFAULT);

    //(1) get Client's property.
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    //
    ::property_get("vendor.client.em.appmode", value, "");
    s8ClientAppModeVal = value;
    MY_LOGD("device: %s %s", s8ClientAppModeKey.string(), s8ClientAppModeVal.string());
    //
    if( s8ClientAppModeVal.isEmpty() ) {

        //(1) get Client's property.
        Utils::Property::tryGet(s8ClientAppModeKey, s8ClientAppModeVal);
        //
        MY_LOGD("UtilsGet - device: %s %s", s8ClientAppModeKey.string(), s8ClientAppModeVal.string());
        //
        if( s8ClientAppModeVal.isEmpty() ) {
            s8ClientAppModeVal = MtkCameraParameters::APP_MODE_NAME_DEFAULT;
        }
        //
        //(2) reset Client's property.
         Utils::Property::set(s8ClientAppModeKey, String8::empty());
        MY_LOGD("UtilsSet - device: %s %s", s8ClientAppModeKey.string(), s8ClientAppModeVal.string());
    }

    {
        int forceSingleCam = property_get_int32("vendor.camera.forceSingleCam", 0);
        if (forceSingleCam) {
            s8ClientAppModeVal = MtkCameraParameters::APP_MODE_NAME_DEFAULT;
            Utils::Property::set(s8ClientAppModeKey, s8ClientAppModeVal);
            MY_LOGD("UtilsSet - device: %s %s (force)", s8ClientAppModeKey.string(), s8ClientAppModeVal.string());
        }
    }
    //
    return s8ClientAppModeVal;
}

/******************************************************************************
 *
 ******************************************************************************/
static
CameraDevice1Base*
createSpecificCameraDevice1(
    String8 const                   s8ClientAppMode,
    ICameraDeviceManager*           deviceManager,
    std::string                     instanceName,
    int32_t                         instanceId
)
{

CameraDevice1Base* pdev = NULL;
    String8 s8CamDeviceInstFactory;

    //[TODO] check the library name
    String8 const s8LibPath = String8::format("libmtkcam_device1.so");
    void *handle = ::dlopen(s8LibPath.string(), RTLD_GLOBAL);

    IHalLogicalDeviceList* pHalDeviceList;
    std::vector<MINT32> Ids;
    pHalDeviceList = MAKE_HalLogicalDeviceList();

    Ids = pHalDeviceList->getSensorId(instanceId);
    instanceId = Ids[0];
    if ( ! handle )
    {
        char const *err_str = ::dlerror();
        MY_LOGW("ERROR: dlopen library=%s %s", s8LibPath.string(), err_str?err_str:"unknown");
        return pdev;
    }

    if  ( s8ClientAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_ENG )
    {
        s8CamDeviceInstFactory = String8::format("createCameraDevice1_Default");
        MY_LOGI("APP_MODE_NAME_DEFAULT");
    }
    else
    if  ( s8ClientAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_ATV )
    {
        s8CamDeviceInstFactory = String8::format("createCam1Device_MtkAtv");
        MY_LOGI("APP_MODE_NAME_ATV");
    }
    //[TODO] sync the function name with Stereo team
    else
    if  ( s8ClientAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_STEREO )
    {
#if (MTKCAM_HAVE_VSDOF_MODE == 2)
        s8CamDeviceInstFactory = String8::format("createCameraDevice1_Dual");
        MY_LOGI("APP_MODE_NAME_MTK_DUALCAM (vsdof v2)");
#else
        s8CamDeviceInstFactory = String8::format("createCameraDevice1_MtkStereo");
        MY_LOGI("APP_MODE_NAME_STEREO");
#endif
#if (MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1)
            // force set stereo feature mode to VSDOF.
            // Because use this device just for VSDOF.
            MY_LOGD("get sensor id from setting provider");
            //
            int32_t sensorId_Main1 = -1, sensor_Id_Main2 = -1;
            NSCam::IHalSensorList *pSensorHalList = MAKE_HalSensorList();
            int sensorCount = pSensorHalList->queryNumberOfSensors();
            MINT32 stereoMode = NSCam::v1::Stereo::E_STEREO_FEATURE_CAPTURE | NSCam::v1::Stereo::E_STEREO_FEATURE_VSDOF;
            MUINT32 sensorDev = (MUINT32)pSensorHalList->querySensorDevIdx(instanceId);
            MINT32 forcemode = property_get_int32( "vendor.camera.stereo.mode", -1);
            if(forcemode == -1)
            {
                forcemode = getStereoModeType();
            }
            if(forcemode == 0)
            {
                MY_LOGI("vsdof");
                stereoMode = NSCam::v1::Stereo::E_STEREO_FEATURE_CAPTURE | NSCam::v1::Stereo::E_STEREO_FEATURE_VSDOF;
            }
            else if(forcemode == 1)
            {
                MY_LOGI("force 3rd flow");
                stereoMode = NSCam::v1::Stereo::E_STEREO_FEATURE_THIRD_PARTY;
            }
            else if(forcemode == 2)
            {
                MY_LOGI("force tk depth");
                stereoMode = NSCam::v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP;
            }
            else
            {
                MY_LOGA("not support");
            }
            StereoSettingProvider::setStereoProfile(sensorDev, sensorCount);
            StereoSettingProvider::setStereoFeatureMode(stereoMode);
            if(!StereoSettingProvider::getStereoSensorIndex(sensorId_Main1, sensor_Id_Main2)){
                MY_LOGE("Cannot get sensor ids from StereoSettingProvider!");
            }
            MY_LOGD("Sensor count: %d main1: %d main: %d", sensorCount, sensorId_Main1, sensor_Id_Main2);
            instanceId = sensorId_Main1;
#endif // (MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1)
    }
#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1) || (MTK_CAM_STEREO_DENOISE_SUPPORT == 1)
    else
    if  ( s8ClientAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_DUALCAM && instanceId == 0)
    {
        // denoise is de-feature! so, if use b+m module, it has to switch to use single cam device.
        // main2 always mono in b+m module. so, it can simple check main2 is mono sensor or not.
        // 1. get sensor index (no need to set feature mode.)
        int32_t sensorId_Main1 = -1, sensor_Id_Main2 = -1;
        if(!StereoSettingProvider::getStereoSensorIndex(sensorId_Main1, sensor_Id_Main2)){
            MY_LOGE("Cannot get sensor ids from StereoSettingProvider!");
        }
        MUINT sensorRawFmtType_Main2 = StereoSettingProvider::getSensorRawFormat(sensor_Id_Main2);
        MY_LOGD("sensor type(%d) b(%d)", sensorRawFmtType_Main2, sensorRawFmtType_Main2 == SENSOR_RAW_MONO);
        if(sensorRawFmtType_Main2 == SENSOR_RAW_MONO)
        {
            s8CamDeviceInstFactory = String8::format("createCameraDevice1_Default");
            MY_LOGI("APP_MODE_NAME_DEFAULT");
        }
        else
        {
            s8CamDeviceInstFactory = String8::format("createCameraDevice1_Dual");
            MY_LOGI("APP_MODE_NAME_MTK_DUALCAM");
        }
    }
#endif
    else
    {
        s8CamDeviceInstFactory = String8::format("createCameraDevice1_Default");
        MY_LOGI("APP_MODE_NAME_DEFAULT");
    }

    //
    void* pCreateInstance = ::dlsym(handle, s8CamDeviceInstFactory.string());
    if (0 == pCreateInstance){
        MY_LOGE("Not exist: %s for %s", s8CamDeviceInstFactory.string(), s8ClientAppMode.string());
        ::dlclose(handle);
        handle = NULL;
        return nullptr;
    }

    if(s8ClientAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_ATV){
        pdev = reinterpret_cast<NSCam::CameraDevice1Base* (*)(ICameraDeviceManager*
                                                            , std::string
                                                            , int32_t
                                                            , android::String8)>
                    (pCreateInstance)(deviceManager, instanceName, 0xFF, s8ClientAppMode);
    }
    else{
        pdev = reinterpret_cast<NSCam::CameraDevice1Base* (*)(ICameraDeviceManager*
                                                            , std::string
                                                            , int32_t
                                                            , android::String8)>
                    (pCreateInstance)(deviceManager, instanceName, instanceId, s8ClientAppMode);
    }

    //
    if (pdev == nullptr){
        MY_LOGE("Fail to create CameraDevice1: %s", s8ClientAppMode.string());
    }

    ::dlclose(handle);
    handle = NULL;
    //
    MY_LOGI("- %p", pdev);
    return  pdev;
}

/******************************************************************************
 *
 ******************************************************************************/
CameraDevice1Base*
CameraDevice1Base::
createCameraDevice(
    ICameraDeviceManager* deviceManager,
    std::string           instanceName,
    int32_t               instanceId
)
{

    // [1] query client app mode (default/eng/atv/s3d/vt)
    CameraDevice1Base* pDevice = NULL;
    String8 const mClientAppMode = queryClientAppMode();

    // [2] check if create the device for debug
    // try get property from system property
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    //[TODO] chage the property label (not open)
    int openId = property_get_int32( "vendor.debug.camera.open", -1);

    if(openId != -1)
    {
        instanceId = openId;
    }

    // [3] create cameradevice according to client app mode
    pDevice = createSpecificCameraDevice1(mClientAppMode,
                                          deviceManager,
                                          instanceName,
                                          instanceId);
    if ( ! pDevice ) {
        MY_LOGE("Fail to create CameraDevice1Impl");
        return nullptr;
    }

    return pDevice;
}

