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

#include "MyUtils.h"
//
#include <cutils/properties.h>
//
#if '1'==MTKCAM_HAVE_DEVMETAINFO
    #include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
    #include <mtkcam/v1/camutils/CamInfo.h>
    using namespace android::MtkCamUtils;
#endif
//
#if '1'==MTKCAM_HAVE_CAM_MANAGER
    #include <mtkcam/hwutils/CamManager.h>
#endif
//
#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/hal/IHalFlash.h>
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
CameraDeviceManagerImpl::
CameraDeviceManagerImpl(char const* type)
    : CameraDeviceManagerBase(type)
{
}


/******************************************************************************
 *
 *  Invoked by CamDeviceManagerBase::enumerateDevicesLocked()
 *
 ******************************************************************************/
auto
CameraDeviceManagerImpl::
onEnumerateDevicesLocked() -> ::android::status_t
{
#if '1'==MTKCAM_HAVE_DEVMETAINFO
    DevMetaInfo::clear();
#endif
#if '1'==MTKCAM_HAVE_METADATAPROVIDER
    NSMetadataProviderManager::clear();
#endif
    mPhysEnumDeviceMap.clear();
    //--------------------------------------------------------------------------
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    IHalSensorList*const pHalSensorList = IHalSensorList::get();
    size_t const sensorNum = pHalSensorList->searchSensors();
    CAM_LOGI("pHalSensorList:%p searchSensors:%zu queryNumberOfSensors:%d", pHalSensorList, sensorNum, pHalSensorList->queryNumberOfSensors());
    mPhysEnumDeviceMap.setCapacity(sensorNum);
    mVirtEnumDeviceMap.setCapacity(sensorNum*2);
    for (size_t instanceId = 0; instanceId < sensorNum; instanceId++)
    {
        sp<PhysEnumDevice> pPhysDevice = new PhysEnumDevice;
        mPhysEnumDeviceMap.add(instanceId, pPhysDevice);
        //
        sp<IMetadataProvider> pMetadataProvider;
#if '1'==MTKCAM_HAVE_METADATAPROVIDER
        pMetadataProvider = IMetadataProvider::create(instanceId);
        NSMetadataProviderManager::add(instanceId, pMetadataProvider.get());
        MY_LOGD("[0x%02zx] IMetadataProvider:%p sensor:%s", instanceId, pMetadataProvider.get(), pHalSensorList->queryDriverName(instanceId));
#endif
        //
        pPhysDevice->mMetadataProvider   = pMetadataProvider;
        pPhysDevice->mSensorName         = pHalSensorList->queryDriverName(instanceId);
        pPhysDevice->mInstanceId         = instanceId;
        pPhysDevice->mFacing             = pMetadataProvider->getDeviceFacing();
        pPhysDevice->mWantedOrientation  = pMetadataProvider->getDeviceWantedOrientation();
        pPhysDevice->mSetupOrientation   = pMetadataProvider->getDeviceSetupOrientation();
        pPhysDevice->mHasFlashUnit       = pMetadataProvider->getDeviceHasFlashLight();
        //
        addVirtualDevicesLocked(pPhysDevice.get());

        #if '1'==MTKCAM_HAVE_DEVMETAINFO
        {
            camera_info camInfo;
            ::memset(&camInfo, 0, sizeof(camInfo));
            camInfo.facing = (pPhysDevice->mFacing == MTK_LENS_FACING_FRONT) ? CAMERA_FACING_FRONT : CAMERA_FACING_BACK;
            camInfo.orientation = pPhysDevice->mWantedOrientation;
            DevMetaInfo::add(instanceId, camInfo, pPhysDevice->mSetupOrientation, eDevId_ImgSensor, pHalSensorList->querySensorDevIdx(instanceId));
        }
        #endif
    }
#endif  //#if '1'==MTKCAM_HAVE_SENSOR_HAL
    //--------------------------------------------------------------------------
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerImpl::
onGetMaxNumOfMultiOpenCameras() const -> uint32_t
{
    static_assert(MTKCAM_MAX_NUM_OF_MULTI_OPEN >= 1, "MTKCAM_MAX_NUM_OF_MULTI_OPEN < 1");

#if defined(MTKCAM_HAVE_NATIVE_PIP) && (MTKCAM_MAX_NUM_OF_MULTI_OPEN == 1)
    return 2;
#endif

    return MTKCAM_MAX_NUM_OF_MULTI_OPEN;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerImpl::
onValidateOpenLocked(
    const ::android::sp<IVirtualDevice>& pVirtualDevice
) const -> ::android::status_t
{
    const char* deviceName = pVirtualDevice->getInstanceName();
    int32_t const i4OpenId =  pVirtualDevice->getInstanceId();
    uint32_t const majorVersion = pVirtualDevice->getMajorVersion();
    //
    if  ( 0 != mOpenDeviceMap.size() )
    {
        if  ( onGetMaxNumOfMultiOpenCameras() <= mOpenDeviceMap.size() )
        {
#if defined(MTKCAM_HAVE_NATIVE_PIP)
            MY_LOGI("MTKCAM_HAVE_NATIVE_PIP defined");
#else
            MY_LOGI("MTKCAM_HAVE_NATIVE_PIP not defined => not support PIP");
#endif
            MY_LOGI("MTKCAM_MAX_NUM_OF_MULTI_OPEN=%d", MTKCAM_MAX_NUM_OF_MULTI_OPEN);
            MY_LOGE("The maximal number of camera devices that can be opened concurrently were opened already.");
            MY_LOGE("[Now] fail to open (%s deviceId%d major:0x%x) => failure", deviceName, i4OpenId, majorVersion);
            logLocked();
            return -EUSERS;
        }

        if  (majorVersion != mOpenDeviceMap.valueAt(0)->mMajorDeviceVersion)
        {
            MY_LOGE("Multi-open with different major versions");
            MY_LOGE("[Now] fail to open (%s deviceId%d major:0x%x) => failure", deviceName, i4OpenId, majorVersion);
            logLocked();
            return -EUSERS;
        }
    }
    //
#if '1'==MTKCAM_HAVE_CAM_MANAGER
    if ( ! NSCam::Utils::CamManager::getInstance()->getPermission() )
    {
        MY_LOGE("Cannot open device %s %d ... Permission denied", deviceName, i4OpenId);
        return -EUSERS;
    }
#endif
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerImpl::
onAttachOpenDeviceLocked(
    const ::android::sp<IVirtualDevice>& pVirtualDevice __unused
) -> void
{
#if '1'==MTKCAM_HAVE_CAM_MANAGER
    auto pCamMgr = NSCam::Utils::CamManager::getInstance();
    pCamMgr->incDevice();
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerImpl::
onDetachOpenDeviceLocked(
    const ::android::sp<IVirtualDevice>& pVirtualDevice __unused
) -> void
{
#if '1'==MTKCAM_HAVE_CAM_MANAGER
    auto pCamMgr = NSCam::Utils::CamManager::getInstance();
    pCamMgr->decDevice();
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerImpl::
onEnableTorchLocked(
    const std::string& deviceName,
    bool enable
) -> ::android::status_t
{
#if '1'==MTKCAM_HAVE_FLASH_HAL
    auto const& pInfo = mVirtEnumDeviceMap.valueFor(deviceName);
    auto const instanceId = pInfo->mInstanceId;
    //
    IHalFlash*const pHalFlash = IHalFlash::getInstance(instanceId);
    bool enable_old = pHalFlash->getTorchStatus(instanceId) == 1;
    if (enable_old == enable) {
        MY_LOGD(
            "do nothing due to torch status unchanged - %s:%u torch enable:%d",
            deviceName.c_str(), instanceId, enable);
        return OK;
    }
    //
    if ( pHalFlash->setTorchOnOff(instanceId, enable) != 0 ) {
        MY_LOGE(
            "setTorchOnOff fail - %s:%u torch enable(new/old)=(%d/%d)",
            deviceName.c_str(), instanceId, enable, enable_old);
        return -ENODEV;//INTERNAL_ERROR
    }
    //
    return OK;
#else
    MY_LOGE("[%s] MTKCAM_HAVE_FLASH_HAL=0", deviceName.c_str());
    (void)deviceName;
    (void)enable;
    return -ENOSYS;
#endif
}

