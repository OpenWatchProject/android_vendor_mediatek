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

#define LOG_TAG "ACameraHelper"

#include "ACameraHelper.h"

#include <mtkcam/utils/std/ULog.h>

// ------------------------------------------------------------------------

namespace NSCam {

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAMERA_HAL_SERVER);

// ------------------------------------------------------------------------

ACameraHelper::ACameraHelper(const char *cameraId, ACameraManager *manager)
    : mCameraId(cameraId), mCameraManager(manager)
{
}

camera_status_t ACameraHelper::openCamera(native_handle_t* imageReaderAnw,
        const std::vector<PhysicalImgReaderInfo>& physicalImgReaders,
        bool usePhysicalSettings, ACameraDevice_request_template requestTemplate)
{
    (void)physicalImgReaders;
    (void)usePhysicalSettings;

    if (imageReaderAnw == nullptr)
    {
        CAM_ULOGME("cannot initialize camera before image reader get initialized");
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }

    if (mIsCameraReady)
    {
        CAM_ULOGME("camera has been initialized");
        return ACAMERA_ERROR_CAMERA_IN_USE;
    }

    // Open a connection to a camera with the given ID
    //
    // The opened camera device will be returned in the device parameter
    // TODO: add necessary implementations of the device state callback
    camera_status_t ret = ACameraManager_openCamera(
            mCameraManager, mCameraId, &mDeviceStateCallbacks, &mCameraDevice);
    if (ret != ACAMERA_OK || mCameraDevice == nullptr)
    {
        CAM_ULOGME("open camera failed: ret(%d) mCameraDevice(0x%" PRIxPTR ")",
                ret, reinterpret_cast<intptr_t>(mCameraDevice));
        return ret;
    }

    mImageReaderAnw = imageReaderAnw;

    // create a capture session output container
    //
    // The container is used in ACameraDevice_createCaptureSession method to
    // create a capture session.
    //
    // Use ACaptureSessionOutputContainer_free to
    // free the container and its memory after application no longer needs
    // the ACaptureSessionOutputContainer.
    ret = ACaptureSessionOutputContainer_create(&mSessionOutputContainer);
    if (ret != ACAMERA_OK)
    {
        CAM_ULOGME("ACaptureSessionOutputContainer_create failed: ret(%d)", ret);
        return ret;
    }

    // create a ACaptureSessionOutput object
    //
    // The ACaptureSessionOutput is used in ACaptureSessionOutputContainer_add
    // method to add an output ANativeWindow to ACaptureSessionOutputContainer.
    //
    // Use ACaptureSessionOutput_free to free the object and its memory after
    // application no longer needs the ACaptureSessionOutput.
    ret = ACaptureSessionOutput_create(mImageReaderAnw, &mSessionOutput);
    if (ret != ACAMERA_OK)
    {
        CAM_ULOGME("ACaptureSessionOutput_create failed: ret(%d)", ret);
        return ret;
    }

    // add an ACaptureSessionOutput object to ACaptureSessionOutputContainer
    ret = ACaptureSessionOutputContainer_add(
            mSessionOutputContainer, mSessionOutput);
    if (ret != ACAMERA_OK)
    {
        CAM_ULOGME("ACaptureSessionOutputContainer_add failed: ret(%d)", ret);
        return ret;
    }

    // check whether a particular ACaptureSessionOutputContainer is supported by
    // the camera device
    //
    // This method performs a runtime check of a given ACaptureSessionOutputContainer.
    // The result confirms whether or not the passed CaptureSession outputs can
    // be successfully used to create a camera capture session using
    // ACameraDevice_createCaptureSession
    ret = ACameraDevice_isSessionConfigurationSupported(
            mCameraDevice, mSessionOutputContainer);
    if (ret != ACAMERA_OK && ret != ACAMERA_ERROR_UNSUPPORTED_OPERATION)
    {
        CAM_ULOGME("ACameraDevice_isSessionConfigurationSupported failed: ret(%d)", ret);
        return ret;
    }

    // create a new camera capture session by providing the target output set of
    // ANativeWindow to the camera device
    ret = ACameraDevice_createCaptureSession(
            mCameraDevice, mSessionOutputContainer, &mSessionStateCallbacks, &mSession);
    if (ret != ACAMERA_OK)
    {
        CAM_ULOGME("ACameraDevice_createCaptureSession failed: ret(%d)", ret);
        return ret;
    }

    // create a ACaptureRequest for capturing images, initialized with template
    // for a target use case.
    //
    // Create a request suitable for still image capture while recording
    // video. Specifically, this means maximizing image quality without
    // disrupting the ongoing recording.
    // These requests would commonly be used with the
    // ACameraCaptureSession_capture method while a request based on
    // TEMPLATE_RECORD is is in use with ACameraCaptureSession_setRepeatingRequest.
    // This template is guaranteed to be supported on all camera devices.
    ret = ACameraDevice_createCaptureRequest(mCameraDevice,
           requestTemplate, &mCaptureRequest);
    if (ret != ACAMERA_OK)
    {
        CAM_ULOGME("ACameraDevice_createCaptureRequest failed: ret(%d)", ret);
        return ret;
    }

    // create a ACameraOutputTarget object
    //
    // The ACameraOutputTarget is used in ACaptureRequest_addTarget method to
    // add an output ANativeWindow to ACaptureRequest.
    //
    // Use ACameraOutputTarget_free to free the object and its memory after
    // application no longer needs the ACameraOutputTarget.
    ret = ACameraOutputTarget_create(mImageReaderAnw, &mReqImageReaderOutput);
    if (ret != ACAMERA_OK)
    {
        CAM_ULOGME("ACameraOutputTarget_create failed: ret(%d)", ret);
        return ret;
    }

    // add an ACameraOutputTarget object to ACaptureRequest
    ret = ACaptureRequest_addTarget(mCaptureRequest, mReqImageReaderOutput);
    if (ret != ACAMERA_OK)
    {
        CAM_ULOGME("ACaptureRequest_addTarget failed: ret(%d)", ret);
        return ret;
    }

    // Request endlessly repeating capture of a sequence of images by this capture session.
    //
    // With this method, the camera device will continually capture images,
    // cycling through the settings in the provided list of ACaptureRequest,
    // at the maximum rate possible.
    //
    // If a request is submitted through ACameraCaptureSession_capture,
    // the current repetition of the request list will be
    // completed before the higher-priority request is handled. This guarantees
    // that the application always receives a complete repeat burst captured in
    // minimal time, instead of bursts interleaved with higher-priority
    // captures, or incomplete captures.
    //
    // Repeating burst requests are a simple way for an application to
    // maintain a preview or other continuous stream of frames where each
    // request is different in a predicatable way, without having to continually
    // submit requests through ACameraCaptureSession_capture.
    //
    // To stop the repeating capture, call ACameraCaptureSession_stopRepeating.
    // Any ongoing burst will still be completed, however. Calling
    // ACameraCaptureSession_abortCaptures will also clear the request.
    //
    // Calling this method will replace a previously-set repeating requests
    // set up by this method, although any in-progress burst will be completed
    // before the new repeat burst will be used.
    if (requestTemplate == TEMPLATE_RECORD || requestTemplate == TEMPLATE_PREVIEW)
    {
        ACameraCaptureSession_setRepeatingRequest(
                mSession, nullptr, 1, &mCaptureRequest, nullptr);
        if (ret != ACAMERA_OK)
        {
            CAM_ULOGME("ACameraCaptureSession_setRepeatingRequest failed: ret(%d)", ret);
            return ret;
        }
    }

    mIsCameraReady = true;

    return ret;
}

void ACameraHelper::closeCamera()
{
    if (!mIsCameraReady)
    {
        CAM_ULOGME("closeCamera failed: camera is not ready");
        return;
    }
    // Cancel any ongoing repeating capture set by ACameraCaptureSession_setRepeatingRequest
    camera_status_t status = ACameraCaptureSession_stopRepeating(mSession);
    if (status != ACAMERA_OK)
    {
        CAM_ULOGME("ACameraCaptureSession_stopRepeating failed: ret(%d)", status);
    }

    // free a ACameraOutputTarget object
    if (mReqImageReaderOutput)
    {
        ACameraOutputTarget_free(mReqImageReaderOutput);
        mReqImageReaderOutput = nullptr;
    }

    // free a ACaptureRequest object
    if (mCaptureRequest)
    {
        ACaptureRequest_free(mCaptureRequest);
        mCaptureRequest = nullptr;
    }

    // close this capture session
    //
    // closing a session frees up the target output Surfaces of the session for
    // reuse with either a new session, or to other APIs that can draw to Surfaces
    if (mSession != nullptr)
    {
        ACameraCaptureSession_close(mSession);
        mSession = nullptr;
    }

    // free a ACaptureSessionOutput object
    if (mSessionOutput)
    {
        ACaptureSessionOutput_free(mSessionOutput);
        mSessionOutput = nullptr;
    }

    // free a capture session output container
    if (mSessionOutputContainer)
    {
        ACaptureSessionOutputContainer_free(mSessionOutputContainer);
        mSessionOutputContainer = nullptr;
    }

    // close the connection and free this ACameraDevice synchronously
    if (mCameraDevice)
    {
        // access to the ACameraDevice after calling this method will cause a crash
        auto status = ACameraDevice_close(mCameraDevice);
        if (status == ACAMERA_OK)
            mCameraDevice = nullptr;
        else
            CAM_ULOGME("close camera failed: ret(%d) device(0x%" PRIxPTR ")", status,
                reinterpret_cast<intptr_t>(mCameraDevice));
    }

    mIsCameraReady = false;
}

} // namespace NSCam
