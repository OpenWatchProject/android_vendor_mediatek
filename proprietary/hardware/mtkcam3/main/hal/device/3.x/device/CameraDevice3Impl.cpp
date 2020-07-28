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

#include "CameraDevice3Impl.h"
#include "MyUtils.h"
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
#include <mtkcam/utils/std/ULog.h>

#include <unordered_set>

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAMERA_DEVICE);

//
using namespace android;
using namespace NSCam;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("%d[CameraDevice3Impl::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("%d[CameraDevice3Impl::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("%d[CameraDevice3Impl::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("%d[CameraDevice3Impl::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("%d[CameraDevice3Impl::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("%d[CameraDevice3Impl::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("%d[CameraDevice3Impl::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
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
const std::string CameraDevice3Impl::MyDebuggee::mName{ICameraDevice::descriptor};
auto CameraDevice3Impl::MyDebuggee::debug(android::Printer& printer, const std::vector<std::string>& options) -> void
{
    auto p = mContext.promote();
    if ( p != nullptr ) {
        p->debug(printer, options);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
CameraDevice3Impl::
~CameraDevice3Impl()
{
    if ( mDebuggee != nullptr ) {
        if ( auto pDbgMgr = IDebuggeeManager::get() ) {
            pDbgMgr->detach(mDebuggee->mCookie);
        }
        mDebuggee = nullptr;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
CameraDevice3Impl::
CameraDevice3Impl(
    ICameraDeviceManager* deviceManager,
    IMetadataProvider* metadataProvider,
    std::map<uint32_t, ::android::sp<IMetadataProvider>>const& physicalMetadataProviders,
    char const* deviceType,
    int32_t instanceId,
    int32_t virtualInstanceId
)
    : ICameraDevice()
    , mLogLevel(0)
    , mDeviceManager(deviceManager)
    , mStaticDeviceInfo(nullptr)
    , mMetadataProvider(metadataProvider)
    , mPhysicalMetadataProviders(physicalMetadataProviders)
    , mMetadataConverter(IMetadataConverter::createInstance(IDefaultMetadataTagSet::singleton()->getTagSet()))
{
    mStaticDeviceInfo = std::make_shared<Info>();
    if (mStaticDeviceInfo != nullptr) {
        mStaticDeviceInfo->mInstanceName    = String8::format("device@%u.%u/%s/%d", kMajorDeviceVersion, kMinorDeviceVersion, deviceType, virtualInstanceId).string();
        mStaticDeviceInfo->mInstanceId      = instanceId;
        mStaticDeviceInfo->mVirtualInstanceId = virtualInstanceId;
        mStaticDeviceInfo->mMajorVersion    = kMajorDeviceVersion;
        mStaticDeviceInfo->mMinorVersion    = kMinorDeviceVersion;
        mStaticDeviceInfo->mHasFlashUnit    = metadataProvider->getDeviceHasFlashLight();
        mStaticDeviceInfo->mFacing          = metadataProvider->getDeviceFacing();
    }

    mLogLevel = getCameraDevice3DebugLogLevel();
    //
    MY_LOGD("%p mStaticDeviceInfo:%p MetadataProvider:%p MetadataConverter:%p debug.camera.log.<CameraDevice3>:%d",
        this, mStaticDeviceInfo.get(), mMetadataProvider.get(), mMetadataConverter.get(), mLogLevel);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDevice3Impl::
initialize(
    const ::android::sp<ICameraDevice3Session>& session
) -> bool
{
    if (mStaticDeviceInfo == nullptr) {
        MY_LOGE("bad mStaticDeviceInfo");
        return false;
    }

    if (session == nullptr) {
        MY_LOGE("bad session");
        return false;
    }

    mSession = session;

    mDebuggee = std::make_shared<MyDebuggee>(this);
    if ( auto pDbgMgr = IDebuggeeManager::get() ) {
        mDebuggee->mCookie = pDbgMgr->attach(mDebuggee, 1);
    }

    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDevice3Impl::
getDeviceInterfaceBase(
    ::android::sp<IBase>& rpDevice
) const -> ::android::status_t
{
    rpDevice = const_cast<IBase*>(
        static_cast<const IBase*>(this));
    return android::OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDevice3Impl::
getDeviceInfo() const -> Info const&
{
    return *mStaticDeviceInfo;
}


/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice3Impl::
getResourceCost(getResourceCost_cb _hidl_cb)
{
    ::android::Mutex::Autolock _getRscLock(mGetResourceLock);
    CameraResourceCost resCost;
    resCost.resourceCost = 100;

    // NOTE: DEVICE_ID is the camera instance id defined by the camera provider HAL.
    //       It is either a small incrementing integer for "internal" device types,
    //       with 0 being the main back-facing camera and 1 being the main
    //       front-facing camera, if they exist.
    //       Or, for external devices, a unique serial number (if possible) that can be
    //       used to identify the device reliably when it is disconnected and reconnected.
    const MUINT DEVICE_ID = mStaticDeviceInfo->mInstanceId;
    IHalLogicalDeviceList* pHalDeviceList = MAKE_HalLogicalDeviceList();

    std::vector<String8> conflictingList;
    if(pHalDeviceList)
    {
        std::vector<MINT32> sensorList = pHalDeviceList->getSensorId(DEVICE_ID);
#if 0
        if(sensorList.size() > 1)
        {
            resCost.resourceCost = 100;
        }
        else
        {
            MBOOL canPowerOn = MTRUE;
            NSCamHW::HwInfoHelper helper(DEVICE_ID);
            helper.getSensorPowerOnPredictionResult(canPowerOn);
            resCost.resourceCost = (canPowerOn) ? 50 : 100;
        }
#endif
        // SENSOR_COUNT is the amount of imager(s)
        const MUINT SENSOR_COUNT = pHalDeviceList->queryNumberOfSensors();

        // DEVICE_COUNT is the amount of camera device(s)
        const MUINT DEVICE_COUNT = pHalDeviceList->queryNumberOfDevices();

        // conflictingDeviceIDList records camera device(s) in conflictingList
        std::unordered_set<MUINT> conflictingDeviceIDList;

        // helpers
        auto isFeatureSupported = [&pHalDeviceList](MUINT deviceID, DEVICE_FEATURE_TYPE type)
        {
            return (pHalDeviceList->getSupportedFeature(deviceID) & type) == type;
        };
        auto inConflictingDeviceIDList = [&conflictingDeviceIDList](MUINT deviceID) {
            return conflictingDeviceIDList.find(deviceID) != conflictingDeviceIDList.end(); };
        auto isPhysicalCameraDevice = [&SENSOR_COUNT](MUINT deviceID) {
            return deviceID < SENSOR_COUNT; };
        auto containLogicalCameraDevice = [&DEVICE_COUNT, &SENSOR_COUNT]() {
            return DEVICE_COUNT > SENSOR_COUNT; };
        auto addToList = [&conflictingList, &conflictingDeviceIDList](
                std::string type, MUINT deviceID)
        {
            conflictingList.push_back(String8::format("device@%u.%u/%s/%u",
                        kMajorDeviceVersion, kMinorDeviceVersion,
                        type.c_str(), deviceID));
            conflictingDeviceIDList.insert(deviceID);
        };

        if(isPhysicalCameraDevice(DEVICE_ID))
        {
            // [physical camera device(s)]

            // use case: all physical camera device(s) if existing logical camera device(s)
            if(containLogicalCameraDevice())
            {
                // add the corresponding logical camera device(s) into conflictingList
                // of a physical camera device
                for(MUINT s = SENSOR_COUNT; s < DEVICE_COUNT; ++s)
                {
                    std::vector<MINT32> sensors = pHalDeviceList->getSensorId(s);
                    bool hasSameSensor = false;
                    for(auto &sensor : sensors)
                    {
                        if(sensor == DEVICE_ID)
                        {
                            hasSameSensor = true;
                            break;
                        }
                    }

                    if(hasSameSensor)
                    {
                        // the logical camera device exists in the conflictingDeviceIDList, do nothing.
                        if(inConflictingDeviceIDList(s))
                            continue;

                        addToList("internal", s);
                    }

                    // because the secure camera device is realized by the logical camera device,
                    // add the logical camera device into conflictingList if it is a secure one.
                    if(isFeatureSupported(s, DEVICE_FEATURE_SECURE_CAMERA))
                    {
                        // the logical camera device exists in the conflictingDeviceIDList, do nothing.
                        if(inConflictingDeviceIDList(s))
                            continue;

                        addToList("internal", s);
                    }
                }
            }
        }
        else
        {
            // [logical camera device(s)]

            // use case: secure camera device
            if(isFeatureSupported(DEVICE_ID, DEVICE_FEATURE_SECURE_CAMERA))
            {
                // add all physical camera device into conflictingList
                for(auto s = 0; s < DEVICE_COUNT; ++s)
                {
                    // if the camera device either exists in the conflictingDeviceIDList,
                    // or is the same as itself, do nothing.
                    if(inConflictingDeviceIDList(s) || (s == DEVICE_ID))
                        continue;

                    addToList("internal", s);
                }
            }

            // use case: multi-camera device
            for(auto &s : sensorList)
            {
                // the physical camera device exists in the conflictingDeviceIDList,
                // do nothing.
                if(inConflictingDeviceIDList(s))
                    continue;

                addToList("internal", s);
            }
        }
    }
    resCost.conflictingDevices.resize(conflictingList.size());
    for(int i = 0; i < conflictingList.size(); ++i) {
        resCost.conflictingDevices[i] = conflictingList[i].string();
    }

    std::string conflictString;
    if(resCost.conflictingDevices.size() > 0)
    {
        for (size_t i = 0; i < resCost.conflictingDevices.size(); i++) {
            conflictString += resCost.conflictingDevices[i];
            if(i < resCost.conflictingDevices.size() - 1) {
                conflictString += ", ";
            }
        }
    }

    MY_LOGD("Camera Device %d: resourceCost %d, conflict to [%s]",
            DEVICE_ID, resCost.resourceCost, conflictString.c_str());

    _hidl_cb(Status::OK, resCost);
    return Void();
}


/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice3Impl::
getCameraCharacteristics(getCameraCharacteristics_cb _hidl_cb)
{
    CameraMetadata cameraCharacteristics;
    {
        camera_metadata const* p_camera_metadata = mMetadataProvider->getStaticCharacteristics();
        size_t size = mMetadataConverter->getCameraMetadataSize(p_camera_metadata);
        cameraCharacteristics.setToExternal((uint8_t *)p_camera_metadata, size, false/*shouldOwn*/);
    }

    _hidl_cb(Status::OK, cameraCharacteristics);
    return Void();
}


/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraDevice3Impl::
setTorchMode(TorchMode mode)
{
    return mapToHidlCameraStatus(mDeviceManager->setTorchMode(mStaticDeviceInfo->mInstanceName, static_cast<uint32_t>(mode)));
}


/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice3Impl::
open(const ::android::sp<V3_2::ICameraDeviceCallback>& callback, open_cb _hidl_cb)
{
    int systraceLevel = ::property_get_int32("vendor.debug.mtkcam.systrace.level", MTKCAM_SYSTRACE_LEVEL_DEFAULT);
    MY_LOGI("open camera3 device (%s) systraceLevel(%d) instanceId(%d) vid(%d)",
                        mStaticDeviceInfo->mInstanceName.c_str(),
                        systraceLevel,
                        mStaticDeviceInfo->mInstanceId,
                        mStaticDeviceInfo->mVirtualInstanceId);
    ::android::status_t status = mSession->open(V3_5::ICameraDeviceCallback::castFrom(callback));
    if  ( ::android::OK != status ) {
        _hidl_cb(mapToHidlCameraStatus(status), nullptr);
    }
    else {
        _hidl_cb(mapToHidlCameraStatus(status), mSession);
    }

    return Void();
}


/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice3Impl::
dumpState(const ::android::hardware::hidl_handle& handle)
{
    ::android::hardware::hidl_vec<::android::hardware::hidl_string> options;
    debug(handle, options);
    return Void();
}


/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice3Impl::
debug(const ::android::hardware::hidl_handle& handle __unused, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options __unused)
{
    //  validate handle
    if (handle == nullptr) {
        MY_LOGE("bad handle:%p", handle.getNativeHandle());
        return Void();
    }
    else if (handle->numFds != 1) {
        MY_LOGE("bad handle:%p numFds:%d", handle.getNativeHandle(), handle->numFds);
        return Void();
    }
    else if (handle->data[0] < 0) {
        MY_LOGE("bad handle:%p numFds:%d fd:%d < 0", handle.getNativeHandle(), handle->numFds, handle->data[0]);
        return Void();
    }

    FdPrinter printer(handle->data[0]);
    debug(printer, std::vector<std::string>{options.begin(), options.end()});

    return Void();
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDevice3Impl::
debug(android::Printer& printer, const std::vector<std::string>& options) -> void
{
    printer.printFormatLine("## Camera device [%s]", mStaticDeviceInfo->mInstanceName.c_str());
    mSession->dumpState(printer, options);
}

/******************************************************************************
 *
 ******************************************************************************/

Return<void>
CameraDevice3Impl::
getPhysicalCameraCharacteristics( const ::android::hardware::hidl_string& physicalCameraId, getPhysicalCameraCharacteristics_cb _hidl_cb)
{
    MY_LOGD("ID: %s", physicalCameraId.c_str());

    CameraMetadata cameraCharacteristics;
    auto physicalId = strtol(physicalCameraId.c_str(), NULL, 10);
    bool isHidden = mDeviceManager->isHiddenCamera(physicalId);
    if(isHidden) {
        auto pMetadataProvider = NSMetadataProviderManager::valueForByDeviceId(physicalId);
        camera_metadata const* pCameraMetadata = pMetadataProvider->getStaticCharacteristics();
        size_t size = mMetadataConverter->getCameraMetadataSize(pCameraMetadata);
        cameraCharacteristics.setToExternal((uint8_t *)pCameraMetadata, size, false/*shouldOwn*/);

        _hidl_cb(Status::OK, cameraCharacteristics);
    }
    else
    {
        MY_LOGD("Physical camera ID %ld is visible for logical ID %d", physicalId, mStaticDeviceInfo->mInstanceId);
        _hidl_cb(Status::ILLEGAL_ARGUMENT, cameraCharacteristics);
    }

    return Void();
}

/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraDevice3Impl::
isStreamCombinationSupported(const ::android::hardware::camera::device::V3_4::StreamConfiguration& streams,
                            isStreamCombinationSupported_cb _hidl_cb)
{
    if(streams.streams.size() == 0) {
        MY_LOGD("No stream found in stream configuration");
        _hidl_cb(Status::OK, false);
        return Void();
    }

    bool isSupported = true;
    for(auto &stream : streams.streams)
    {
        if(stream.v3_2.width  == 0 ||
           stream.v3_2.height == 0)
        {
            MY_LOGD("Invalid stream size: %dx%d", stream.v3_2.width, stream.v3_2.height);
            isSupported = false;
            break;
        }

        //TODO: Qury by ISP capability
    }

    _hidl_cb(Status::OK, isSupported);

    return Void();
}

