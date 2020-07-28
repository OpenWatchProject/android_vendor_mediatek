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
//
using namespace android;
using namespace NSCam;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("%d[CameraDevice3Impl::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("%d[CameraDevice3Impl::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("%d[CameraDevice3Impl::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("%d[CameraDevice3Impl::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("%d[CameraDevice3Impl::%s] " fmt, getInstanceId(), __FUNCTION__, ##arg)
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
    char const* deviceType,
    int32_t instanceId
)
    : ICameraDevice()
    , mLogLevel(0)
    , mDeviceManager(deviceManager)
    , mStaticDeviceInfo(nullptr)
    , mMetadataProvider(metadataProvider)
    , mMetadataConverter(IMetadataConverter::createInstance(IDefaultMetadataTagSet::singleton()->getTagSet()))
{
    mStaticDeviceInfo = std::make_shared<Info>();
    if (mStaticDeviceInfo != nullptr) {
        mStaticDeviceInfo->mInstanceName    = String8::format("device@%u.%u/%s/%d", kMajorDeviceVersion, kMinorDeviceVersion, deviceType, instanceId).string();
        mStaticDeviceInfo->mInstanceId      = instanceId;
        mStaticDeviceInfo->mMajorVersion    = kMajorDeviceVersion;
        mStaticDeviceInfo->mMinorVersion    = kMinorDeviceVersion;
        mStaticDeviceInfo->mHasFlashUnit    = metadataProvider->getDeviceHasFlashLight();
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
    CameraResourceCost resCost;
    resCost.resourceCost = 0;
    resCost.conflictingDevices.resize(0);
#if 0 //Coverity 2357922 Useless call
    for (size_t i = 0; i < resCost.conflictingDevices.size(); i++) {
        resCost.conflictingDevices[i];
        MY_LOGV("conflicting with camDevice %s", resCost.conflictingDevices[i].c_str());
    }
#endif
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
    mMetadataConverter->convertToHidl(mMetadataProvider->getMtkStaticCharacteristics(), &cameraCharacteristics);
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
open(const ::android::sp<ICameraDeviceCallback>& callback, open_cb _hidl_cb)
{
    ::android::status_t status = mSession->open(callback);
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

