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
static auto torchModeStatusToText(uint32_t status) -> android::String8
{
    using T = NSCam::CameraDeviceManagerBase::ETorchModeStatus;
    switch ((T)status) {
    case T::NOT_AVAILABLE:          return String8("NOT_AVAILABLE");
    case T::AVAILABLE_OFF:          return String8("AVAILABLE_OFF");
    case T::AVAILABLE_ON:           return String8("AVAILABLE_ON ");
    }
    return String8("Unknown Torch Mode Status");
};


/******************************************************************************
 *
 ******************************************************************************/
static auto facingToText(int32_t facing) -> android::String8
{
    switch (facing) {
    case MTK_LENS_FACING_FRONT:     return String8("FRONT");
    case MTK_LENS_FACING_BACK:      return String8("BACK ");
    case MTK_LENS_FACING_EXTERNAL:  return String8("EXTERNAL");
    }
    return String8("Unknown Facing");
}


/******************************************************************************
 *
 ******************************************************************************/
CameraDeviceManagerBase::
PhysEnumDevice::
PhysEnumDevice()
    : mMetadataProvider(nullptr)
    , mInstanceId(-1)
    , mFacing(0)
    , mWantedOrientation(0)
    , mSetupOrientation(0)
    , mHasFlashUnit(0)
{
}


/******************************************************************************
 *
 ******************************************************************************/
CameraDeviceManagerBase::
VirtEnumDevice::
VirtEnumDevice()
    : RefBase()
    , mVirtDevice(nullptr)
    , mInstanceId(-1)
    , mTorchModeStatus((uint32_t)ETorchModeStatus::NOT_AVAILABLE)
    , mTorchModeStatusChanging(false)
{
}


/******************************************************************************
 *
 ******************************************************************************/
CameraDeviceManagerBase::
OpenDevice::
OpenDevice()
    : RefBase()
    , mPhysDevice(nullptr)
    , mVirtDevice(nullptr)
    , mMajorDeviceVersion(0)
    , mMinorDeviceVersion(0)
{
}


/******************************************************************************
 *
 ******************************************************************************/
CameraDeviceManagerBase::
ActiveOperation::
ActiveOperation()
    : mOpsLock()
    , mDataRWLock()
    , mOperation(ActiveOperation::IDLE)
    , mVirtDevice(nullptr)
{
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerBase::
ActiveOperation::
lock(
    const ::android::sp<IVirtualDevice>& pVirtualDevice,
    uint32_t operation
) -> ::android::status_t
{
    logIfOwnerExist();

    //  lock & set the active owner
    mOpsLock.lock();
    {
        RWLock::AutoWLock _l(mDataRWLock);

        if  (mVirtDevice != nullptr) {
            MY_LOGE("Not empty owner: %s %p operation:%u timestamp:%s",
                mVirtDevice->getInstanceName(), mVirtDevice.get(), mOperation, mTimestamp.c_str()
            );
            mOpsLock.unlock();
            return -ENODEV;
        }

        mOperation = operation;
        mTimestamp = NSCam::Utils::LogTool::get()->getFormattedLogTime();
        mVirtDevice = pVirtualDevice;
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerBase::
ActiveOperation::
unlock(
    const ::android::sp<IVirtualDevice>& pVirtualDevice,
    uint32_t operation
) -> void
{
    //  reset the active owner & unlock
    {
        RWLock::AutoWLock _l(mDataRWLock);

        if  (  mVirtDevice != pVirtualDevice
            || mOperation != operation )
        {
            MY_LOGW(
                "Active:(%s %p operation:%u timestamp:%s) Try:(%s %p operation:%u)",
                (mVirtDevice.get() ? mVirtDevice->getInstanceName() : "no device"),
                mVirtDevice.get(), mOperation, mTimestamp.c_str(),
                (pVirtualDevice.get() ? pVirtualDevice->getInstanceName() : "no device"), pVirtualDevice.get(), operation
            );
            return;
        }

        mOperation = ActiveOperation::IDLE;
        mTimestamp.clear();
        mVirtDevice = nullptr;
    }
    mOpsLock.unlock();
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerBase::
ActiveOperation::
validate(
    const ::android::sp<IVirtualDevice>& pVirtualDevice,
    uint32_t operation
) const -> ::android::status_t
{
    RWLock::AutoRLock _l(mDataRWLock);
    if  (  mVirtDevice != pVirtualDevice
        || mOperation != operation )
    {
        MY_LOGE(
            "%s must be called in pairs - "
            "Active:(%s %p operation:%u timestamp:%s) Try:(%s %p operation:%u)",
            (ActiveOperation::OPEN==operation ? "startOpenDevice/finishOpenDevice" : "startCloseDevice/finishCloseDevice"),
            (mVirtDevice.get() ? mVirtDevice->getInstanceName() : "no device"),
            mVirtDevice.get(), mOperation, mTimestamp.c_str(),
            (pVirtualDevice.get() ? pVirtualDevice->getInstanceName() : "no device"), pVirtualDevice.get(), operation
        );

        return -ENODEV;
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerBase::
ActiveOperation::
logIfOwnerExist(Vector<String8>* pLogs) const -> void
{
    Vector<String8> logs;
    {
        RWLock::AutoRLock _l(mDataRWLock);
        auto pDevice = mVirtDevice;
        if  ( pDevice != nullptr ) {
            logs.push_back(
                String8::format("Active owner: %s %p operation:%u timestamp:%s",
                (pDevice.get() ? pDevice->getInstanceName() : "unknown"),
                pDevice.get(), mOperation, mTimestamp.c_str()
            ));
        }
    }

    //output logs
    if  ( pLogs ) {
        pLogs->appendVector(logs);
    }
    else {
        for (auto const& log : logs) { MY_LOGI("%s", log.string()); }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerBase::
convertFromLegacyDeviceVersion(uint32_t legacyVersion, uint32_t* major, uint32_t* minor) -> void
{
    /*
     *  #define HARDWARE_MAKE_API_VERSION(maj,min) \
     *              ((((maj) & 0xff) << 8) | ((min) & 0xff))
    */
    if  ( minor ) {
        *minor = (0xff & (legacyVersion));
    }

    if  ( major ) {
        *major = (0xff & (legacyVersion >> 8));
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerBase::
queryMajorDeviceVersion(uint32_t deviceVersion) -> uint32_t
{
    uint32_t majorDeviceVersion = 0xff;

    convertFromLegacyDeviceVersion(deviceVersion, &majorDeviceVersion);

    auto const forcedMajorDeviceVersion = ::property_get_int32("debug.camera.force_device", -1);
    if  ( forcedMajorDeviceVersion != -1 ) {
        majorDeviceVersion = forcedMajorDeviceVersion;
        MY_LOGI(
            "Force major device version from 0x%x to %d (via debug.camera.force_device)",
            deviceVersion, forcedMajorDeviceVersion);
    }

    return majorDeviceVersion;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerBase::
getVirtualDeviceLocked(
    const std::string& deviceName,
    ::android::sp<VirtEnumDevice>* ppInfo,
    ::android::sp<IVirtualDevice>* ppVirtualDevice
) const -> ::android::status_t
{
    auto const& pInfo = mVirtEnumDeviceMap.valueFor(deviceName);
    if  ( pInfo == nullptr ) {
        MY_LOGE("[%s] this device name is unknown", deviceName.c_str());
        logLocked();
        return -EINVAL;//ILLEGAL_ARGUMENT
    }
    //
    auto const& pDevice = pInfo->mVirtDevice;
    if  ( pDevice == nullptr ) {
        MY_LOGE("[%s] bad virtual device", deviceName.c_str());
        logLocked();
        return -EINVAL;//ILLEGAL_ARGUMENT
    }
    //
    if  ( ppInfo ) {
        *ppInfo = pInfo;
    }
    //
    if  ( ppVirtualDevice ) {
        *ppVirtualDevice = pDevice;
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerBase::
logLocked(::android::Vector<::android::String8>* pLogs) const -> void
{
    Vector<String8> logs;

    logs.push_back(String8::format("Physical Devices: # %zu", mPhysEnumDeviceMap.size()));
    for (size_t i = 0; i < mPhysEnumDeviceMap.size(); i++) {
        auto instanceId = mPhysEnumDeviceMap.keyAt(i);
        String8 str = String8::format("  [%02x] ->", instanceId);
        //
        auto const& pInfo = mPhysEnumDeviceMap.valueAt(i);
        if  ( pInfo == nullptr ) {
            str += "Bad PhysEnumDevice";
            logs.push_back(str);
            continue;
        }
        str += String8::format(" orientation(wanted/setup)=(%3d/%-3d) ", pInfo->mWantedOrientation, pInfo->mSetupOrientation);
        str += facingToText(pInfo->mFacing);
        str += String8::format(" hasFlashUnit:%d %s [PhysEnumDevice:%p]", pInfo->mHasFlashUnit, pInfo->mSensorName.c_str(), pInfo.get());
        //
        logs.push_back(str);
    }

    logs.push_back(String8::format("Virtual Devices: # %zu", mVirtEnumDeviceMap.size()));
    for (size_t i = 0; i < mVirtEnumDeviceMap.size(); i++) {
        auto const& name = mVirtEnumDeviceMap.keyAt(i);
        String8 str = String8::format("  [%s] -> ", name.c_str());
        //
        auto const& pInfo = mVirtEnumDeviceMap.valueAt(i);
        if  ( pInfo == nullptr ) {
            str += "Bad VirtEnumDevice";
            logs.push_back(str);
            continue;
        }
        str += String8::format("%02x", pInfo->mInstanceId) + String8(" torchModeStatus:") + torchModeStatusToText(pInfo->mTorchModeStatus);
        if ( pInfo->mTorchModeStatusChanging ) {
            str += ",CHANGING";
        }
        //
        auto const& pVirt = pInfo->mVirtDevice;
        if  ( pVirt == nullptr ) {
            str += "Bad IVirtualDevice";
            logs.push_back(str);
            continue;
        }
        str += String8::format(" hasFlashUnit:%d [VirtEnumDevice:%p IVirtualDevice:%p]", pVirt->hasFlashUnit(), pInfo.get(), pVirt.get());
        //
        logs.push_back(str);
    }

    logs.push_back(String8::format("Open Devices: # %zu (multi-opened maximum: # %u)", mOpenDeviceMap.size(), onGetMaxNumOfMultiOpenCameras()));
    for (size_t i = 0; i < mOpenDeviceMap.size(); i++) {
        auto const& pInfo = mOpenDeviceMap.valueAt(i);
        auto const& pVirt = pInfo->mVirtDevice;
        if  ( pVirt == nullptr ) {
            continue;
        }
        logs.push_back(String8::format(
            "  [%s] -> %u.%u (since %s)",
            pVirt->getInstanceName(),
            pInfo->mMajorDeviceVersion, pInfo->mMinorDeviceVersion,
            pInfo->mOpenTimestamp.c_str()
        ));
    }

    mActiveOperation.logIfOwnerExist(&logs);

    //output logs
    if  ( pLogs ) {
        pLogs->appendVector(logs);
    }
    else {
        for (auto const& log : logs) {
            MY_LOGI("%s", log.string());
            MY_LOGI("--");
        }
    }
}

