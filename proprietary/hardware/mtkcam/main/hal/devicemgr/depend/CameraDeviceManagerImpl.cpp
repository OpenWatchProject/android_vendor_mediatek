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
#include <mtkcam/utils/hw/CamManager.h>
//
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/IHalFlash.h>

#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
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
    NSMetadataProviderManager::clear();
    mPhysEnumDeviceMap.clear();
    //--------------------------------------------------------------------------
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    //IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    //size_t const sensorNum = pHalSensorList->searchSensors();
    ///
    IHalLogicalDeviceList* pHalDeviceList;
    pHalDeviceList = MAKE_HalLogicalDeviceList();//IHalLogicalDeviceList::get();
    size_t const deviceNum = pHalDeviceList->searchDevices();
    ///
    CAM_LOGI("pHalDeviceList:%p logicDeviceCount:%zu physicSensorCount:%d",
                                pHalDeviceList,
                                deviceNum,
                                pHalDeviceList->queryNumberOfSensors());

    mVirtEnumDeviceMap.setCapacity(deviceNum*2);
    for (size_t instanceId = 0; instanceId < deviceNum; instanceId++)
    {
        //
        sp<IMetadataProvider> pMetadataProvider;
        pMetadataProvider = IMetadataProvider::create(instanceId);
        NSMetadataProviderManager::add(instanceId, pMetadataProvider.get());
        MY_LOGD("[0x%02zx] IMetadataProvider:%p sensor:%s", instanceId, pMetadataProvider.get(), pHalDeviceList->queryDriverName(instanceId));
        addVirtualDevicesLocked(instanceId, pMetadataProvider);
    }

    size_t const sensorNum = pHalDeviceList->queryNumberOfSensors();
    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    mPhysEnumDeviceMap.setCapacity(sensorNum);
    for (size_t sensorId = 0; sensorId < sensorNum; sensorId++)
    {
        sp<PhysEnumDevice> pPhysDevice = new PhysEnumDevice;
        sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(sensorId);
        pPhysDevice->mMetadataProvider   = pMetadataProvider;
        pPhysDevice->mSensorName         = pHalSensorList->queryDriverName(sensorId);
        pPhysDevice->mInstanceId         = sensorId;
        pPhysDevice->mFacing             = pMetadataProvider->getDeviceFacing();
        pPhysDevice->mWantedOrientation  = pMetadataProvider->getDeviceWantedOrientation();
        pPhysDevice->mSetupOrientation   = pMetadataProvider->getDeviceSetupOrientation();
        pPhysDevice->mHasFlashUnit       = pMetadataProvider->getDeviceHasFlashLight();

        mPhysEnumDeviceMap.add(sensorId, pPhysDevice);
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

        // ip-based platform should remove this constraint
        // if  (majorVersion != mOpenDeviceMap.valueAt(0)->mMajorDeviceVersion)
        // {
        //     MY_LOGE("Multi-open with different major versions");
        //     MY_LOGE("[Now] fail to open (%s deviceId%d major:0x%x) => failure", deviceName, i4OpenId, majorVersion);
        //     logLocked();
        //     return -EUSERS;
        // }
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
    const ::android::sp<IVirtualDevice>& pVirtualDevice
) -> void
{
    int32_t const i4OpenId =  pVirtualDevice->getInstanceId();
#if '1'==MTKCAM_HAVE_CAM_MANAGER
    auto pCamMgr = NSCam::Utils::CamManager::getInstance();
    pCamMgr->incDevice(i4OpenId);
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerImpl::
onDetachOpenDeviceLocked(
    const ::android::sp<IVirtualDevice>& pVirtualDevice
) -> void
{
    int32_t const i4OpenId =  pVirtualDevice->getInstanceId();
#if '1'==MTKCAM_HAVE_CAM_MANAGER
    auto pCamMgr = NSCam::Utils::CamManager::getInstance();
    pCamMgr->decDevice(i4OpenId);
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
    auto const& pInfo = mVirtEnumDeviceMap.valueFor(deviceName);
    // needs to get correct index id from logic device.
    IHalLogicalDeviceList* pHalDeviceList;
    pHalDeviceList = MAKE_HalLogicalDeviceList();
    auto indexIdsList = pHalDeviceList->getSensorId(pInfo->mInstanceId);
    MY_LOGE_IF(indexIdsList.size() == 0, "size == 0 should not happened");
    auto const instanceId = indexIdsList[0];
    //
    IHalFlash*const pHalFlash = MAKE_HalFlash(instanceId);
    bool enable_old = pHalFlash->getTorchStatus() == 1;
    if (enable_old == enable) {
        MY_LOGD(
            "do nothing due to torch status unchanged - %s:%u torch enable:%d",
            deviceName.c_str(), instanceId, enable);
        pHalFlash->destroyInstance();
        return OK;
    }
    //
    if ( pHalFlash->setTorchOnOff(enable) != 0 ) {
        MY_LOGE(
            "setTorchOnOff fail - %s:%u torch enable(new/old)=(%d/%d)",
            deviceName.c_str(), instanceId, enable, enable_old);
        pHalFlash->destroyInstance();
        return -ENODEV;//INTERNAL_ERROR
    }
    pHalFlash->destroyInstance();
    //
    return OK;
}

