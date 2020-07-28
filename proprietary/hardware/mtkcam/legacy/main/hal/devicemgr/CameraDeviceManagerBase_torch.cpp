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
isTorchModeSupportedLocked(
    const std::string& deviceName
) const -> ::android::status_t
{
    ::android::sp<VirtEnumDevice> pInfo = nullptr;
    ::android::sp<IVirtualDevice> pDevice = nullptr;
    auto status = getVirtualDeviceLocked(deviceName, &pInfo, &pDevice);
    if  ( OK != status ) {
        return status;
    }
    //
    if  ( ! mIsSetTorchModeSupported ) {
        MY_LOGW(
            "[%s] This does not support direct operation of flashlight torch mode. "
            "The framework must open the camera device and turn the torch on through the device interface.",
            deviceName.c_str());
        return -ENOSYS;//METHOD_NOT_SUPPORTED
    }
    //
    if  ( ! pDevice->hasFlashUnit() ) {
        MY_LOGW("[%s] this camera device does not have a flash unit", deviceName.c_str());
        logLocked();
        return -EOPNOTSUPP;//OPERATION_NOT_SUPPORTED
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerBase::
isTorchModeControllableLocked(
    const std::string& deviceName
) const -> ::android::status_t
{
    status_t status = isTorchModeSupportedLocked(deviceName);
    if  ( OK != status ) {
        return status;
    }

    if  ( mOpenDeviceMap.size() != 0 ) {
        MY_LOGE("[%s] Due to other camera devices (# %zu) being open, the torch cannot be controlled currently.",
            deviceName.c_str(), mOpenDeviceMap.size());
        logLocked();
        return -EUSERS;//MAX_CAMERAS_IN_USE
    }

    /**
     *  [TODO]
     *     CAMERA_IN_USE:
     *         This camera device has been opened, so the torch cannot be
     *         controlled until it is closed.
     */

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerBase::
flushTorchModeStatusChangeCallback() -> ::android::status_t
{
    CAM_TRACE_NAME(LOG_TAG":flushTorchModeStatusChangeCallback");

    //(1) move callback list to local
    std::list<std::pair<std::string, uint32_t>> callback_list;
    sp<Callback> callback;
    {
        RWLock::AutoWLock _l(mDataRWLock);
        //
        callback = mCallback.promote();
        for (size_t i = 0; i < mVirtEnumDeviceMap.size(); i++) {
            auto const& name = mVirtEnumDeviceMap.keyAt(i);
            auto const& pInfo = mVirtEnumDeviceMap.valueAt(i);
            if  ( pInfo->mTorchModeStatusChanging ) {
                pInfo->mTorchModeStatusChanging = false;
                callback_list.push_back({name, pInfo->mTorchModeStatus});
            }
        }
    }
    //
    if  (callback == nullptr) {
        MY_LOGW("cannot promote callback - torch mode status callback list:# %zu", callback_list.size());
        return -ENODEV;//INTERNAL_ERROR
    }
    //
    //(2) perform local callback outside locking
    for (auto const& item : callback_list) {
        callback->onTorchModeStatusChange(item.first.c_str(), item.second);
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerBase::
addTorchModeStatusChangeLocked(
    const std::string& deviceName,
    bool enable
) -> void
{
    ETorchModeStatus torchModeStatus = ( ! enable )
        ?  ETorchModeStatus::AVAILABLE_OFF
        :  ETorchModeStatus::AVAILABLE_ON
        ;

    auto const& pInfo = mVirtEnumDeviceMap.valueFor(deviceName);
    if  ( pInfo != nullptr ) {
        pInfo->mTorchModeStatus = (uint32_t)torchModeStatus;
        pInfo->mTorchModeStatusChanging = true;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
CameraDeviceManagerBase::
setTorchModeStatusWhenOpenCloseCameraLocked(
    const std::string& deviceName,
    uint32_t newTorchModeStatus,
    bool markAsChanging
) -> ::android::status_t
{
    status_t status = OK;
    ::android::sp<VirtEnumDevice> pInfo = nullptr;
    ::android::sp<IVirtualDevice> pDevice = nullptr;
    status = getVirtualDeviceLocked(deviceName, &pInfo, &pDevice);
    if  ( OK != status ) {
        return status;
    }
    //
    if  ( ! mIsSetTorchModeSupported ) {
        return OK;
    }
    //
    if  ( ! pDevice->hasFlashUnit() ) {
        return OK;
    }
    //
    if  ( pInfo->mTorchModeStatus == newTorchModeStatus ) {
        MY_LOGD("the same torch mode status transition: %u", newTorchModeStatus);
        return OK;
    }
    //
    status = onEnableTorchLocked(deviceName, false);
    if  ( OK != status ) {
        MY_LOGW("fail to turn off torch while newTorchModeStatus:%u", newTorchModeStatus);
        return status;
    }
    //
    //  change torch mode status
    pInfo->mTorchModeStatus = newTorchModeStatus;
    pInfo->mTorchModeStatusChanging = markAsChanging;
    MY_LOGD("%s newTorchModeStatus:%u", deviceName.c_str(), newTorchModeStatus);
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
CameraDeviceManagerBase::
SetTorchModeStatusCommand::
SetTorchModeStatusCommand(
    CameraDeviceManagerBase* pManager,
    const ::android::sp<IVirtualDevice>& pVirtualDevice,
    ETorchModeStatus newTorchModeStatus
)
    : ICommand()
    , mpManager(pManager)
    , mpVirtualDevice(pVirtualDevice)
    , mOldStatus(ETorchModeStatus::NOT_AVAILABLE)
    , mNewStatus(newTorchModeStatus)
{
}


//  torch mode status transition
static auto const& getTorchTransSet()
{
    typedef CameraDeviceManagerBase::ETorchModeStatus ETorchModeStatusT;

    //  torch mode status transition: old status -> new status
    static std::set<std::pair<ETorchModeStatusT, ETorchModeStatusT>> const
    inst= {
        //open camera
        {ETorchModeStatusT::AVAILABLE_OFF, ETorchModeStatusT::NOT_AVAILABLE},
        {ETorchModeStatusT::AVAILABLE_ON,  ETorchModeStatusT::NOT_AVAILABLE},
        //close camera
        {ETorchModeStatusT::NOT_AVAILABLE, ETorchModeStatusT::AVAILABLE_OFF},
        //unchanged
        {ETorchModeStatusT::AVAILABLE_ON,  ETorchModeStatusT::AVAILABLE_ON},
        {ETorchModeStatusT::AVAILABLE_OFF, ETorchModeStatusT::AVAILABLE_OFF},
        {ETorchModeStatusT::NOT_AVAILABLE, ETorchModeStatusT::NOT_AVAILABLE},
    };

    return inst;
}


/******************************************************************************
 *
 ******************************************************************************/
::android::status_t
CameraDeviceManagerBase::
SetTorchModeStatusCommand::
doExecute()
{
    status_t status = OK;
    const char* deviceName = mpVirtualDevice->getInstanceName();
    ::android::sp<VirtEnumDevice> pInfo = nullptr;

    // [1] get current (old) torch mode status and backup it.
    status = mpManager->getVirtualDeviceLocked(deviceName, &pInfo, nullptr);
    if  ( OK != status ) {
        return status;
    }
    //
    mOldStatus = static_cast<ETorchModeStatus>(pInfo->mTorchModeStatus);
    //
    // [2] find the mapping: old status -> new status
    auto find = getTorchTransSet().find({mOldStatus, mNewStatus});
    if  ( find == getTorchTransSet().end() ) {
        MY_LOGE("unsupported torch mode status transition: %u -> %u", mOldStatus, mNewStatus);
        mpManager->logLocked();
        return -ENODEV;
    }
    //
    // [3] change torch mode status
    return mpManager->setTorchModeStatusWhenOpenCloseCameraLocked(deviceName, static_cast<uint32_t>(mNewStatus), true);
}


/******************************************************************************
 *
 ******************************************************************************/
::android::status_t
CameraDeviceManagerBase::
SetTorchModeStatusCommand::
undoExecute()
{
    const char* deviceName = mpVirtualDevice->getInstanceName();
    //
    // [1] find the mapping: old status -> new status
    auto find = getTorchTransSet().find({mOldStatus, mNewStatus});
    if  ( find == getTorchTransSet().end() ) {
        MY_LOGE("unsupported torch mode status transition: %u <- %u", mOldStatus, mNewStatus);
        mpManager->logLocked();
        return -ENODEV;
    }
    //
    // [2] change torch mode status
    return mpManager->setTorchModeStatusWhenOpenCloseCameraLocked(deviceName, static_cast<uint32_t>(mOldStatus), false);
}

