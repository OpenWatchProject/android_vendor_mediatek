/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_1_X_INCLUDE_CAMERADEVICE_H_
#define _MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_1_X_INCLUDE_CAMERADEVICE_H_
//
#include <mtkcam/main/hal/ICameraDeviceManager.h>
//
#include "CameraDevice1Base.h"

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


/******************************************************************************
 * Data Members.
 ******************************************************************************/
class CameraDevice
    : public ICameraDeviceManager::IVirtualDevice
    , public IMtkCameraDevice
{
public:
    // setup during constructor
    ICameraDeviceManager*                    mDeviceManager = nullptr;       //  device manager.
    std::shared_ptr<Info>                    mStaticDeviceInfo = nullptr;    //  device info
    ::android::sp<IMetadataProvider>         mMetadataProvider = nullptr;
    char const*                              mDeviceType = nullptr;
    int32_t                                  mInstanceId;

    // setup during opening camera
    ::android::sp<CameraDevice1Base>         mpDevice = nullptr;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

public:
    virtual            ~CameraDevice();
                       CameraDevice(
                           ICameraDeviceManager* deviceManager,
                           IMetadataProvider* metadataProvider,
                           char const* deviceType,
                           int32_t instanceId
                       );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    ICameraDeviceManager::IVirtualDevice Interfaces.

    virtual auto       getDeviceInterfaceBase(
                           sp<IBase>& rpDevice
                       ) const -> status_t override;

    virtual auto       getDeviceInfo() const -> Info const& override;

public:     ////    ICameraDevice Interfaces.
    Return<void>       getResourceCost(getResourceCost_cb _hidl_cb) override;
    Return<void>       getCameraInfo(getCameraInfo_cb _hidl_cb) override;
    Return<Status>     setTorchMode(TorchMode mode) override;
    Return<Status>     dumpState(const hidl_handle& fd) override;
    Return<Status>     open(const sp<ICameraDeviceCallback>& callback) override;
    Return<Status>     setPreviewWindow(const sp<ICameraDevicePreviewCallback>& window) override;
    Return<void>       enableMsgType(uint32_t msgType) override;
    Return<void>       disableMsgType(uint32_t msgType) override;
    Return<bool>       msgTypeEnabled(uint32_t msgType) override;
    Return<Status>     startPreview() override;
    Return<void>       stopPreview() override;
    Return<bool>       previewEnabled() override;
    Return<Status>     storeMetaDataInBuffers(bool enable) override;
    Return<Status>     startRecording() override;
    Return<void>       stopRecording() override;
    Return<bool>       recordingEnabled() override;
    Return<void>       releaseRecordingFrame(uint32_t memId, uint32_t bufferIndex) override;
    Return<void>       releaseRecordingFrameHandle(
                           uint32_t memId, uint32_t bufferIndex, const hidl_handle& frame) override;
    Return<void>       releaseRecordingFrameHandleBatch(
                           const hidl_vec<VideoFrameMessage>&) override;
    Return<Status>     autoFocus() override;
    Return<Status>     cancelAutoFocus() override;
    Return<Status>     takePicture() override;
    Return<Status>     cancelPicture() override;
    Return<Status>     setParameters(const hidl_string& params) override;
    Return<void>       getParameters(getParameters_cb _hidl_cb) override;
    Return<Status>     sendCommand(CommandType cmd, int32_t arg1, int32_t arg2) override;
    Return<void>       close() override;

public:     ////    IMtkCameraDevice Interfaces.
    Return<void>       setProperty(const hidl_string& key, const hidl_string& value) override;
    Return<void>       getProperty(const hidl_string& key, getProperty_cb _hidl_cb) override;
};
};
#endif  //_MTK_HARDWARE_MTKCAM_MAIN_HAL_DEVICE_1_X_DEVICE_CAMERADEVICE_H_
