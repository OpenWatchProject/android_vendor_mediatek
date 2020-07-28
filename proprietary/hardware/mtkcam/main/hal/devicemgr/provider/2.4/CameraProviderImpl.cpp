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

#include "CameraProviderImpl.h"
//
#include <chrono>
#include <future>
//
#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <libladder.h>
#include <utils/Thread.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/std/LogTool.h>
#include <mtkcam/utils/metadata/IVendorTagDescriptor.h>

/******************************************************************************
 *
 ******************************************************************************/
using namespace android::hardware::camera::provider::V2_4;

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
extern "C"
ICameraProvider*
createICameraProvider_V2_4(const char* providerName, NSCam::ICameraDeviceManager* manager)
{
    MY_LOGI("+ %s", providerName);
    //
    if ( ! manager ) {
        MY_LOGE("bad camera device manager");
        return nullptr;
    }
    //
    auto provider = new CameraProviderImpl(providerName, manager);
    if ( ! provider ) {
        MY_LOGE("cannot allocate camera provider %s", providerName);
        return nullptr;
    }
    //
    if ( ! provider->initialize() ) {
        MY_LOGE("camera provider %s initialize failed", providerName);
        delete provider;
        provider = nullptr;
    }
    //
    MY_LOGI("- %s provider:%p manager:%p", providerName, provider, manager);
    return provider;
}


/******************************************************************************
 *
 ******************************************************************************/
Status
CameraProviderImpl::
mapToHidlCameraStatus(::android::status_t status)
{
    switch (status)
    {
        case 0/*OK*/:                       return Status::OK;
        case -EINVAL/*BAD_VALUE*/:          return Status::ILLEGAL_ARGUMENT;
        case -EBUSY:                        return Status::CAMERA_IN_USE;
        case -EUSERS:                       return Status::MAX_CAMERAS_IN_USE;
        case -ENOSYS/*INVALID_OPERATION*/:  return Status::METHOD_NOT_SUPPORTED;
        case -EOPNOTSUPP:                   return Status::OPERATION_NOT_SUPPORTED;
        case -EPIPE/*DEAD_OBJECT*/:         return Status::CAMERA_DISCONNECTED;
        case -ENODEV/*NO_INIT*/:            return Status::INTERNAL_ERROR;
        default: break;
    }

    MY_LOGW("unknown android::status_t [%d %s]", -status, ::strerror(-status));
    return Status::INTERNAL_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
CameraProviderImpl::
CameraProviderImpl(const char* /*providerName*/, NSCam::ICameraDeviceManager* manager)
    : ICameraProvider()
    //
    , mManager(manager)
    , mVendorTagSections()
    //
    , mProviderCallback(nullptr)
    , mProviderCallbackLock()
{
    ::memset(&mProviderCallbackDebugInfo, 0, sizeof(mProviderCallbackDebugInfo));
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CameraProviderImpl::
initialize()
{
    MY_LOGI("+");
    //
    //  (1) setup vendor tags (and global vendor tags)
    if  ( ! setupVendorTags() ) {
        MY_LOGE("setupVendorTags() fail");
        return false;
    }
    //  (2) check device manager
    if  ( ! mManager ) {
        MY_LOGE("bad ICameraDeviceManager");
        return false;
    }
    //
    MY_LOGI("-");
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CameraProviderImpl::
setupVendorTags()
{
    auto pVendorTagDesc = NSCam::getVendorTagDescriptor();
    if  ( ! pVendorTagDesc ) {
        MY_LOGE("bad pVendorTagDesc");
        return false;
    }
    //
    //  setup mVendorTagSections
    auto vSection = pVendorTagDesc->getSections();
    size_t const numSections = vSection.size();
    mVendorTagSections.resize(numSections);
    for (size_t i = 0; i < numSections; i++) {
        auto const& s = vSection[i];
        //
        //s.tags; -> tags;
        std::vector<VendorTag> tags;
        tags.reserve(s.tags.size());
        for (auto const& t : s.tags) {
            VendorTag vt;
            vt.tagId    = t.second.tagId;
            vt.tagName  = t.second.tagName;
            vt.tagType  = (CameraMetadataType) t.second.tagType;
            tags.push_back(vt);
        }
        //
        mVendorTagSections[i].tags = tags;
        mVendorTagSections[i].sectionName = s.sectionName;
    }
    //
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
void
CameraProviderImpl::
onCameraDeviceStatusChange(char const* deviceName, uint32_t new_status)
{
    ::android::sp<ICameraProviderCallback> callback;
    {
        Mutex::Autolock _l(mProviderCallbackLock);
        callback = mProviderCallback;
    }

    if  ( callback == 0 ) {
        MY_LOGW("bad mProviderCallback - %s new_status:%u", deviceName, new_status);
        return;
    }

    CameraDeviceStatus const status = (CameraDeviceStatus) new_status;
    auto ret = callback->cameraDeviceStatusChange(deviceName, status);
    if (!ret.isOk()) {
        MY_LOGE("Transaction error in ICameraProviderCallback::cameraDeviceStatusChange: %s", ret.description().c_str());
    }
    MY_LOGI("%s CameraDeviceStatus:%u", deviceName, status);
}


/******************************************************************************
 *
 ******************************************************************************/
void
CameraProviderImpl::
onTorchModeStatusChange(char const* deviceName, uint32_t new_status)
{
    ::android::sp<ICameraProviderCallback> callback;
    {
        Mutex::Autolock _l(mProviderCallbackLock);
        callback = mProviderCallback;
    }

    if  ( callback == 0 ) {
        MY_LOGW("bad mProviderCallback - %s new_status:%u", deviceName, new_status);
        return;
    }

    TorchModeStatus const status = (TorchModeStatus) new_status;
    auto ret = callback->torchModeStatusChange(deviceName, status);
    if (!ret.isOk()) {
        MY_LOGE("Transaction error in ICameraProviderCallback::torchModeStatusChange: %s", ret.description().c_str());
    }
    MY_LOGI("%s TorchModeStatus:%u", deviceName, status);
}


/******************************************************************************
 *
 ******************************************************************************/
void
CameraProviderImpl::
serviceDied(uint64_t cookie __unused, const wp<hidl::base::V1_0::IBase>& who __unused)
{
    if (cookie != (uint64_t)this) {
        MY_LOGW("Unexpected ICameraProviderCallback serviceDied cookie 0x%" PRIx64 ", expected %p", cookie, this);
    }
    MY_LOGW("ICameraProviderCallback %s has died; removing it - cookie:0x%" PRIx64 " this:%p", toString(mProviderCallbackDebugInfo).c_str(), cookie, this);
    {
        Mutex::Autolock _l(mProviderCallbackLock);
        mProviderCallback = nullptr;
        ::memset(&mProviderCallbackDebugInfo, 0, sizeof(mProviderCallbackDebugInfo));
    }
}


/******************************************************************************
 *
 ******************************************************************************/
Return<Status>
CameraProviderImpl::
setCallback(const ::android::sp<ICameraProviderCallback>& callback)
{
    {
        Mutex::Autolock _l(mProviderCallbackLock);

        //  cleanup the previous callback if existed.
        if ( mProviderCallback != nullptr ) {
            {//unlinkToDeath
                hardware::Return<bool> ret = mProviderCallback->unlinkToDeath(this);
                if (!ret.isOk()) {
                    MY_LOGE("Transaction error in unlinking to ICameraProviderCallback death: %s", ret.description().c_str());
                } else if (!ret) {
                    MY_LOGW("Unable to unlink to ICameraProviderCallback death notifications");
                }
            }

            mProviderCallback = nullptr;
            ::memset(&mProviderCallbackDebugInfo, 0, sizeof(mProviderCallbackDebugInfo));
        }

        //  setup the new callback.
        mProviderCallback = callback;
        if ( mProviderCallback != nullptr ) {
            {//linkToDeath
                hardware::Return<bool> ret = mProviderCallback->linkToDeath(this, (uint64_t)this);
                if (!ret.isOk()) {
                    MY_LOGE("Transaction error in linking to ICameraProviderCallback death: %s", ret.description().c_str());
                } else if (!ret) {
                    MY_LOGW("Unable to link to ICameraProviderCallback death notifications");
                }
            }

            {//getDebugInfo
                hardware::Return<void> ret = mProviderCallback->getDebugInfo([this](const auto& info){
                    mProviderCallbackDebugInfo = info;
                });
                if (!ret.isOk()) {
                    MY_LOGE("Transaction error in ICameraProviderCallback::getDebugInfo: %s", ret.description().c_str());
                }
            }

            MY_LOGD("ICameraProviderCallback %s", toString(mProviderCallbackDebugInfo).c_str());
        }
    }
    return mapToHidlCameraStatus(mManager->setCallbacks(this));
}


/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraProviderImpl::
getVendorTags(getVendorTags_cb _hidl_cb)
{
    _hidl_cb(Status::OK, mVendorTagSections);
    return Void();
}


/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraProviderImpl::
getCameraIdList(getCameraIdList_cb _hidl_cb)
{
    //  deviceNameList from device manager
    std::vector<std::string> deviceNameList;
    auto status = mManager->getDeviceNameList(deviceNameList);

    //  deviceNameList -> hidlDeviceNameList
    hidl_vec<hidl_string> hidlDeviceNameList;
    hidlDeviceNameList.resize(deviceNameList.size());
    for (size_t i = 0; i < deviceNameList.size(); i++) {
        hidlDeviceNameList[i] = deviceNameList[i];
    }
    _hidl_cb(mapToHidlCameraStatus(status), hidlDeviceNameList);
    return Void();
}


/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraProviderImpl::
isSetTorchModeSupported(isSetTorchModeSupported_cb _hidl_cb)
{
    _hidl_cb(Status::OK, mManager->isSetTorchModeSupported());
    return Void();
}


/******************************************************************************
 *
 ******************************************************************************/
template <class InterfaceT, class InterfaceCallbackT>
void
CameraProviderImpl::
getCameraDeviceInterface(
    const hidl_string& cameraDeviceName,
    InterfaceCallbackT _hidl_cb
)
{
    ::android::sp<NSCam::ICameraDeviceManager::IBase_t> pBaseDevice(nullptr);
    auto status = mManager->getDeviceInterface(cameraDeviceName, pBaseDevice);
    //
    auto pICameraDevice = ::android::sp<InterfaceT>(static_cast<InterfaceT*>(pBaseDevice.get()));
    if  ( pBaseDevice == nullptr || pICameraDevice == nullptr || 0 != status ) {
        MY_LOGE(
            "DeviceName:%s pBaseDevice:%p pICameraDevice:%p status:%s(%d)",
            cameraDeviceName.c_str(), pBaseDevice.get(), pICameraDevice.get(), ::strerror(-status), -status
        );
    }
    _hidl_cb(mapToHidlCameraStatus(status), pICameraDevice);
}


/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraProviderImpl::
getCameraDeviceInterface_V1_x(
    const hidl_string& cameraDeviceName,
    getCameraDeviceInterface_V1_x_cb _hidl_cb)
{
    getCameraDeviceInterface<::android::hardware::camera::device::V1_0::ICameraDevice>(cameraDeviceName, _hidl_cb);
    return Void();
}


/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraProviderImpl::
getCameraDeviceInterface_V3_x(
    const hidl_string& cameraDeviceName,
    getCameraDeviceInterface_V3_x_cb _hidl_cb)
{
    getCameraDeviceInterface<::android::hardware::camera::device::V3_2::ICameraDevice>(cameraDeviceName, _hidl_cb);
    return Void();
}


/******************************************************************************
 *
 ******************************************************************************/
Return<void>
CameraProviderImpl::
debug(const ::android::hardware::hidl_handle& handle, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options)
{
    CAM_TRACE_CALL();


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


    const int fd = handle->data[0];


#if   MTKCAM_TARGET_BUILD_VARIANT=='u'
    const char targetBuild[] = "user-build";
#elif MTKCAM_TARGET_BUILD_VARIANT=='d'
    const char targetBuild[] = "userdebug-build";
#else
    const char targetBuild[] = "eng-build";
#endif

    struct MyPrinter : public Printer {
        int         mDupFd = -1;
        FdPrinter   mFdPrinter;
        LogPrinter  mLogPrinter;

                    MyPrinter(int fd)
                        : mDupFd(::dup(fd))
                        , mFdPrinter(mDupFd)
                        , mLogPrinter(LOG_TAG, ANDROID_LOG_INFO)
                    {}
                    ~MyPrinter()
                    {
                        if ( mDupFd >= 0 ) {
                            close(mDupFd);
                            mDupFd = -1;
                        }
                    }
        virtual void printLine(const char* string)
                    {
                        mFdPrinter.printLine(string);
                    #if MTKCAM_TARGET_BUILD_VARIANT=='u'
                        //in user-build: dump it to the log, too.
                        mLogPrinter.printLine(string);
                    #endif
                    }
    };
    auto printer = std::make_shared<MyPrinter>(fd);

    {
        std::string log{" ="};
        for (auto const& option : options) {
            log += " \"";
            log += option;
            log += "\"";
        }
        MY_LOGI("fd:%d options(#%zu)%s", fd, options.size(), log.c_str());
        printer->printFormatLine("## Reporting + (%s)", NSCam::Utils::LogTool::get()->getFormattedLogTime().c_str());
        printer->printFormatLine("## Camera Hal Service (camerahalserver pid:%d %zuBIT) %s ## fd:%d options(#%zu)%s", ::getpid(), (sizeof(intptr_t)<<3), targetBuild, fd, options.size(), log.c_str());
    }
    {
        Mutex::Autolock _l(mProviderCallbackLock);
        printer->printFormatLine("  ICameraProviderCallback %p %s", mProviderCallback.get(), toString(mProviderCallbackDebugInfo).c_str());
    }

    //// Device Manager
    {
        mManager->debug(printer, std::vector<std::string>{options.begin(), options.end()});
        printer->printFormatLine("## Reporting done - (%s)", NSCam::Utils::LogTool::get()->getFormattedLogTime().c_str());
    }

    MY_LOGI("-");

    return ICameraProvider::debug(handle, options);
}

