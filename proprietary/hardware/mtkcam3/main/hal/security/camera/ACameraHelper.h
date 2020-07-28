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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef VENDOR_MEDIATEK_HARDWARE_CAMERA3_MAIN_HAL_SECURITY_CAMERA_ACAMERAHELPER_H
#define VENDOR_MEDIATEK_HARDWARE_CAMERA3_MAIN_HAL_SECURITY_CAMERA_ACAMERAHELPER_H

#include <camera/NdkCameraManager.h>
#include <media/NdkImageReader.h>

#include <mutex>

// ------------------------------------------------------------------------

namespace NSCam {

class ACameraHelper final
{
public:
    ACameraHelper(const char* cameraId, ACameraManager* manager);
    ~ACameraHelper() { closeCamera(); }

    struct PhysicalImgReaderInfo
    {
        const char* physicalCameraId;
        native_handle_t* anw;
    };
    camera_status_t openCamera(native_handle_t* imageReaderAnw,
                               const std::vector<PhysicalImgReaderInfo>& physicalImgReaders,
                               bool usePhysicalSettings,
                               ACameraDevice_request_template requestTemplate);

    void closeCamera();

    bool isCameraReady() const { return mIsCameraReady; }

    static void onDeviceDisconnected(
            void* /*obj*/, ACameraDevice* /*device*/) {}

    static void onDeviceError(
            void* /*obj*/, ACameraDevice* /*device*/, int /*errorCode*/) {}

    static void onSessionClosed(
            void* /*obj*/, ACameraCaptureSession* /*session*/) {}

    static void onSessionReady(
            void* /*obj*/, ACameraCaptureSession* /*session*/) {}

    static void onSessionActive(
            void* /*obj*/, ACameraCaptureSession* /*session*/) {}

private:
    ACameraDevice_StateCallbacks mDeviceStateCallbacks {
        this, onDeviceDisconnected, onDeviceError };

    ACameraCaptureSession_stateCallbacks mSessionStateCallbacks {
        this, onSessionClosed, onSessionReady, onSessionActive };

    native_handle_t* mImageReaderAnw = nullptr;  // not owned by us.
    const char* mCameraId;
    ACameraManager* mCameraManager = nullptr;

    // camera device
    ACameraDevice* mCameraDevice = nullptr;

    // capture session
    ACaptureSessionOutputContainer* mSessionOutputContainer = nullptr;
    ACaptureSessionOutput* mSessionOutput = nullptr;

    ACameraCaptureSession* mSession = nullptr;

    // capture request
    ACaptureRequest* mCaptureRequest = nullptr;
    ACameraOutputTarget* mReqImageReaderOutput = nullptr;

    bool mIsCameraReady = false;

    mutable std::mutex mMutex;
    int mCompletedCaptureCallbackCount = 0;

    ACameraCaptureSession_captureCallbacks mCaptureCallbacks = {
        // TODO: Add tests for other callbacks
        this, // context
        nullptr, // onCaptureStarted
        nullptr, // onCaptureProgressed
        [](void* context, ACameraCaptureSession* session,
                ACaptureRequest* request, const ACameraMetadata* result)
        {
            (void)session;
            (void)request;
            (void)result;

            ACameraHelper *thisContext = static_cast<ACameraHelper *>(context);
            std::lock_guard<std::mutex> l(thisContext->mMutex);
            thisContext->mCompletedCaptureCallbackCount++;
        },
        nullptr, // onCaptureFailed
        nullptr, // onCaptureSequenceCompleted
        nullptr, // onCaptureSequenceAborted
        nullptr, // onCaptureBufferLost
    };
}; // class ACameraHelper

} // namespace NSCam

#endif // VENDOR_MEDIATEK_HARDWARE_CAMERA3_MAIN_HAL_SECURITY_CAMERA_ACAMERAHELPER_H
