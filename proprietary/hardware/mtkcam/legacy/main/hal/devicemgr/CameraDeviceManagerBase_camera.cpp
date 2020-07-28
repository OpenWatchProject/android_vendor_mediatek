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
using namespace android;
using namespace NSCam;
//
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
auto
CameraDeviceManagerBase::
enumerateDevicesLocked() -> ::android::status_t
{
    status_t status = OK;
    {
        Utils::CamProfile _profile(__FUNCTION__, "CameraDeviceManagerBase");
        status = onEnumerateDevicesLocked();
        _profile.print("");
    }
    logLocked();
    return status;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerBase::
addVirtualDevicesLocked(PhysEnumDevice* pPhyDevice) -> void
{
    if  (pPhyDevice == nullptr) {
        MY_LOGW("Bad physical device");
        return;
    }

    uint32_t const instanceId = pPhyDevice->mInstanceId;
    auto pMetadataProvider = pPhyDevice->mMetadataProvider;
    if  (pMetadataProvider == nullptr) {
        MY_LOGW("Bad metadata provider");
        return;
    }


    auto add_virtual_device = [&](IVirtualDevice* pVirtualDevice){
            if ( pVirtualDevice != nullptr ) {
                if  ( auto pInfo = new VirtEnumDevice ) {
                    if ( pVirtualDevice->hasFlashUnit() ) {
                        mIsSetTorchModeSupported = true;
                        pInfo->mTorchModeStatus = (uint32_t)ETorchModeStatus::AVAILABLE_OFF;
                    }
                    pInfo->mVirtDevice = pVirtualDevice;
                    pInfo->mInstanceId = pVirtualDevice->getInstanceId();
                    mVirtEnumDeviceMap.add(pVirtualDevice->getInstanceName(), pInfo);
                }
            }
        };

    auto create_and_add_virtual_device = [&](uint32_t majorVersion){
            auto create_device = mVirtEnumDeviceFactoryMap[majorVersion].mCreateVirtualCameraDevice;
            if ( create_device == nullptr ) {
                CAM_LOGE("The symbole createVirtualCameraDevice for device version %d does not exist!!!", majorVersion);
                return;
            }
            CreateVirtualCameraDeviceParams param = {
                .instanceId = static_cast<int32_t>(instanceId),
                .deviceType = mType.c_str(),
                .pMetadataProvider = pMetadataProvider.get(),
                .pDeviceManager = this,
            };
            auto new_device = create_device(&param);
            add_virtual_device(new_device);
        };


    //  enumerating virtual devices...
    if ( 0 != mOverriddenDeviceHalVersions.size() )
    {
        MY_LOGI("force to override device major versions...");
        for (auto v : mOverriddenDeviceHalVersions) {
            create_and_add_virtual_device(v);
        }
    }
    else
    {
    #if (3 == MTKCAM_HAL_VERSION)
        create_and_add_virtual_device(3);
        MY_LOGD("Create camera device 3");
    #else
        create_and_add_virtual_device(1);
        MY_LOGD("Create camera device 1");
    #endif
    }
}

/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerBase::
waitForPreviousPowerOnDone() -> void
{
    sem_wait(&mSemPowerOn);
}

/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerBase::
updatePowerOnDone() -> void
{
    sem_post(&mSemPowerOn);
}

/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerBase::
validateOpenLocked(
    const ::android::sp<IVirtualDevice>& pVirtualDevice
) const -> ::android::status_t
{
    const char* deviceName = pVirtualDevice->getInstanceName();
    int32_t const i4OpenId =  pVirtualDevice->getInstanceId();
    uint32_t const majorVersion = pVirtualDevice->getMajorVersion();
    //
    auto pEnumInfo = mVirtEnumDeviceMap.valueFor(deviceName);
    if  ( pEnumInfo == 0 ) {
        MY_LOGE("Bad %s %d", deviceName, i4OpenId);
        logLocked();
        /*
         * -EINVAL:     The input arguments are invalid, i.e. the id is invalid,
         *              and/or the module is invalid.
         */
        return -EINVAL;
    }
    //
    auto pOpenDevice = mOpenDeviceMap.valueFor(i4OpenId);
    if  ( pOpenDevice != 0 ) {
        auto pOpenedVirtualDevice = pOpenDevice->mVirtDevice;
        MY_LOGE(
            "Cannot open %s %d, since %s has already been opened",
            deviceName, i4OpenId, pOpenedVirtualDevice->getInstanceName());
        logLocked();
        /*
         * -EBUSY:      The camera device was already opened for this camera id
         *              (by using this method or common.methods->open method),
         *              regardless of the device HAL version it was opened as.
         */
        return -EBUSY;
    }
    //
    return onValidateOpenLocked(pVirtualDevice);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerBase::
attachOpenDeviceLocked(
    const ::android::sp<IVirtualDevice>& pVirtualDevice
) -> ::android::status_t
{
    onAttachOpenDeviceLocked(pVirtualDevice);

    const char* deviceName = pVirtualDevice->getInstanceName();
    int32_t const openId =  pVirtualDevice->getInstanceId();
    //
    sp<OpenDevice> pOpenDevice;
    pOpenDevice = new OpenDevice;
    pOpenDevice->mPhysDevice = mPhysEnumDeviceMap.valueFor(openId);
    pOpenDevice->mVirtDevice = pVirtualDevice;
    pOpenDevice->mMajorDeviceVersion = pVirtualDevice->getMajorVersion();
    pOpenDevice->mMinorDeviceVersion = pVirtualDevice->getMinorVersion();
    pOpenDevice->mOpenTimestamp = NSCam::Utils::LogTool::get()->getFormattedLogTime();
    //
    mOpenDeviceMap.add(openId, pOpenDevice);
    MY_LOGI("device: %s openTimestamp:%s", deviceName, pOpenDevice->mOpenTimestamp.c_str());
    //
    // set Android log detect to higher level
    setLogLevelToEngLoad(((1 == mOpenDeviceMap.size())?1:0),1);

    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerBase::
detachOpenDeviceLocked(
    const ::android::sp<IVirtualDevice>& pVirtualDevice
) -> ::android::status_t
{
    onDetachOpenDeviceLocked(pVirtualDevice);

    const char* deviceName = pVirtualDevice->getInstanceName();
    int32_t const openId =  pVirtualDevice->getInstanceId();
    //
    ssize_t const index = mOpenDeviceMap.indexOfKey(openId);
    if ( index < 0 ) {
        MY_LOGE("Cannot find %s %d - index:%zd", deviceName, openId, index);
        logLocked();
        return  NAME_NOT_FOUND;
    }
    //
    auto pOpenDevice = mOpenDeviceMap.valueAt(index);
    if ( pOpenDevice == 0 || pOpenDevice->mVirtDevice != pVirtualDevice )
    {
        MY_LOGE(
            "Cannot find %s %d - index:%zd pOpenDevice:%p pVirtualDevice:%p",
            deviceName, openId, index, pOpenDevice.get(), pVirtualDevice.get());
        logLocked();
        return  NAME_NOT_FOUND;
    }
    //
    MY_LOGI("device: %s openTimestamp:%s", deviceName, pOpenDevice->mOpenTimestamp.c_str());
    mOpenDeviceMap.removeItemsAt(index);
    pOpenDevice = nullptr;
    //
    // set Android log detect to default level
    setLogLevelToEngLoad(((0 == mOpenDeviceMap.size())?1:0),0);

    return  OK;
}

