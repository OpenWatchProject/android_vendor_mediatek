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
    MY_LOGI("dlopen libmtkcam_device1.so +");  //using for debug only
    void *handle = ::dlopen(s8LibPath.string(), RTLD_GLOBAL);
    MY_LOGI("dlopen libmtkcam_device1.so -");  //using for debug only
    if ( ! handle )
    {
        char const *err_str = ::dlerror();
        MY_LOGW("ERROR: dlopen library=%s %s", s8LibPath.string(), err_str?err_str:"unknown");
        return pdev;
    }

    if  ( s8ClientAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_ENG )
    {
        s8CamDeviceInstFactory = String8::format("createCameraDevice1_Default");
    }
    else
    if  ( s8ClientAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_ATV )
    {
        s8CamDeviceInstFactory = String8::format("createCam1Device_MtkAtv");
    }
    //[TODO] sync the function name with Stereo team
    else
    if  ( s8ClientAppMode == MtkCameraParameters::APP_MODE_NAME_MTK_STEREO )
    {
        s8CamDeviceInstFactory = String8::format("createCameraDevice1_MtkStereo");
    }
    else
    {
        s8CamDeviceInstFactory = String8::format("createCameraDevice1_Default");
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

    /* [2] check if create the device for debug
    int mDevID = params->instanceId;
    int mDebugDevID = -1;

    // try get property from system property
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    //[TODO] chage the property label (not open)
    property_get( "vendor.debug.camera.open", value, "-1");
    mDebugDevID = atoi(value);

    if(mDebugDevID != -1){
        mDevID = mDebugDevID;
        MY_LOGD("Create Debug camera from system property : %d", mDevID);
    }

    else{    // try get property from AP
        int APDebugDevID = -1;
        Utils::Property::tryGet(String8("vendor.debug.camera.open"), APDebugDevID);
        if (APDebugDevID != -1){
            mDevID = APDebugDevID;
            MY_LOGD("Create Debug camera from AP : %d", mDevID);
        }
    }*/

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

