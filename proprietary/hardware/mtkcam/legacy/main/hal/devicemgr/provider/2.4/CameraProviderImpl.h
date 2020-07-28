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

#ifndef _MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICEMGR_2_4_CAMERAPROVIDERIMPL_H_
#define _MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICEMGR_2_4_CAMERAPROVIDERIMPL_H_
//
#include <utils/StrongPointer.h>
#include <utils/Mutex.h>
//
#include <android/hardware/camera/provider/2.4/ICameraProvider.h>
//
#include <mtkcam/device/ICameraDeviceManager.h>
//
/******************************************************************************
 *
 ******************************************************************************/
namespace android {
namespace hardware {
namespace camera {
namespace provider {
namespace V2_4 {
//
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using namespace ::android::hardware::camera::common::V1_0;

/******************************************************************************
 *
 ******************************************************************************/
class CameraProviderImpl
    : public ICameraProvider
    , public android::hardware::hidl_death_recipient
    , public NSCam::ICameraDeviceManager::Callback
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                Data Members.

    NSCam::ICameraDeviceManager*mManager;
    hidl_vec<VendorTagSection>  mVendorTagSections;

    ::android::sp<ICameraProviderCallback>
                                mProviderCallback;
    ::android::Mutex            mProviderCallbackLock;
    ::android::hidl::base::V1_0::DebugInfo
                                mProviderCallbackDebugInfo;

protected:  ////                Operations.

    static  Status              mapToHidlCameraStatus(::android::status_t status);
    bool                        setupVendorTags();

    // Helper for getCameraDeviceInterface_VN_x to use.
    template <class InterfaceT, class InterfaceCallbackT>
    void                        getCameraDeviceInterface(
                                    const hidl_string& cameraDeviceName,
                                    InterfaceCallbackT _hidl_cb
                                );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
                                CameraProviderImpl(
                                    const char* providerName,
                                    NSCam::ICameraDeviceManager* manager
                                );

    virtual bool                initialize();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ICameraDeviceManager::Callback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual void                onCameraDeviceStatusChange(
                                    char const* deviceName, uint32_t new_status
                                ) override;

    virtual void                onTorchModeStatusChange(
                                    char const* deviceName, uint32_t new_status
                                ) override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ::android::hardware::hidl_death_recipient
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual void                serviceDied(uint64_t cookie, const wp<hidl::base::V1_0::IBase>& who) override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ::android::hardware::camera::provider::Vx_x::ICameraProvider Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual Return<Status>      setCallback(const ::android::sp<ICameraProviderCallback>& callback) override;

    virtual Return<void>        getVendorTags(getVendorTags_cb _hidl_cb) override;

    virtual Return<void>        getCameraIdList(getCameraIdList_cb _hidl_cb) override;

    virtual Return<void>        isSetTorchModeSupported(isSetTorchModeSupported_cb _hidl_cb) override;

    virtual Return<void>        getCameraDeviceInterface_V1_x(
                                    const hidl_string& cameraDeviceName,
                                    getCameraDeviceInterface_V1_x_cb _hidl_cb) override;

    virtual Return<void>        getCameraDeviceInterface_V3_x(
                                    const hidl_string& cameraDeviceName,
                                    getCameraDeviceInterface_V3_x_cb _hidl_cb) override;

    virtual Return<void>        debug(
                                    const ::android::hardware::hidl_handle& fd,
                                    const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options) override;

};


/******************************************************************************
 *
 ******************************************************************************/
}  // namespace V2_4
}  // namespace provider
}  // namespace camera
}  // namespace hardware
}  // namespace android

#endif  //_MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICEMGR_2_4_CAMERAPROVIDERIMPL_H_
