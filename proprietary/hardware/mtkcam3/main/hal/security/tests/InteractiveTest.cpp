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

#define LOG_TAG "securecamera_test"

#include "InteractiveTest.h"

#include "utils/TestBase.h"
#include "camera/ACameraHelper.h"
#include "camera/ACameraMetadataCache.h"

#include <mtkcam/utils/std/ULog.h>

#include <camera/NdkCameraManager.h>

#include <VendorTagDescriptor.h>

// ------------------------------------------------------------------------

namespace NSCam {
namespace tests {

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAMERA_HAL_SERVER);

using android::hardware::camera::common::V1_0::helper::VendorTagDescriptorCache;
using TestDefs::ReaderUsageAndAFormat;

// ------------------------------------------------------------------------

InteractiveTest::InteractiveTest()
{
    // create ACameraManager instance
    //
    // The ACameraManager is responsible for detecting, characterizing,
    // and connecting to ACameraDevice.
    //
    // The caller must call ACameraManager_delete to free the resources once
    // it is done using the ACameraManager instance.
    mCameraManager = ACameraManager_create();
    if (mCameraManager == nullptr)
    {
        CAM_ULOGME("create ACameraManager failed");
        return;
    }

    // create a list of currently connected camera devices, including
    // cameras that may be in use by other camera API clients.
    //
    // Non-removable cameras use integers starting at 0 for their
    // identifiers, while removable cameras have a unique identifier for each
    // individual device, even if they are the same model.
    //
    // ACameraManager_getCameraIdList will allocate and return an  ACameraIdList.
    // The caller must call ACameraManager_deleteCameraIdList to free the memory.
    camera_status_t ret = ACameraManager_getCameraIdList(
            mCameraManager, &mCameraIdList);
    if (ret != ACAMERA_OK)
    {
        CAM_ULOGME("get cameraIdList failed: ret(%d)", ret);
        return;
    }

    CAM_ULOGM_ASSERT(VendorTagDescriptorCache::getGlobalVendorTagCache(),
            "get global vendor tag cache failed");
    if (mCameraIdList->numCameras < 1)
    {
        CAM_ULOGMW("Device has no camera on board");
        return;
    }
}

InteractiveTest::~InteractiveTest()
{
    // delete a list of camera devices allocated via ACameraManager_getCameraIdList.
    if (mCameraIdList)
    {
        ACameraManager_deleteCameraIdList(mCameraIdList);
        mCameraIdList = nullptr;
    }

    // delete the ACameraManager instance and free its resources
    if (mCameraManager)
    {
        ACameraManager_delete(mCameraManager);
        mCameraManager = nullptr;
    }
}

int InteractiveTest::run()
{
    for (int i = 0; i < mCameraIdList->numCameras; i++)
    {
        const char *cameraId(mCameraIdList->cameraIds[i]);
        CAM_ULOGM_ASSERT(cameraId != nullptr, "invalid camera ID");

        if (!isCameraDeviceSupported(cameraId))
            continue;

        for (const auto& readerUsageAndFormat : TestDefs::kReaderUsageAndFormat)
        {
            if (skipAHardwareBufferUsage(readerUsageAndFormat.first))
                continue;

            processCaptureRequests({cameraId, readerUsageAndFormat,
                                    TestDefs::kMaxAImagesStreaming, false,
                                    AImageReaderHelper::imageAvailable_cb()});
        }
    }

    return EXIT_SUCCESS;
}

bool InteractiveTest::isCapabilitySupported(
        const ACameraMetadata& staticInfo,
        acamera_metadata_enum_android_request_available_capabilities_t cap)
{
    ACameraMetadata_const_entry entry;
    ACameraMetadata_getConstEntry(
            &staticInfo, ACAMERA_REQUEST_AVAILABLE_CAPABILITIES, &entry);
    if (entry.count == 0)
    {
        CAM_ULOGME("cannot find ACAMERA_REQUEST_AVAILABLE_CAPABILITIES in static metadata");
        return false;
    }

    for (uint32_t i = 0; i < entry.count; i++)
    {
        if (entry.data.u8[i] == cap)
            return true;
    }

    return false;
}

bool InteractiveTest::getLensFacing(
    const ACameraMetadata& staticInfo, uint8_t& facing)
{
    ACameraMetadata_const_entry entry;
    ACameraMetadata_getConstEntry(&staticInfo, ACAMERA_LENS_FACING, &entry);
    if (entry.count == 0)
    {
        CAM_ULOGME("cannot find ACAMERA_LENS_FACING in static metadata");
        return false;
    }

    facing = entry.data.u8[0];
    return true;
}

bool InteractiveTest::processCaptureRequests(
    const ProcessCaptureRequestParam& param,
    const std::chrono::seconds durationInSecond)
{
    (void)durationInSecond;

    auto navigator = [](std::string message)
    {
        std::cout << "Press ENTER to " << message << "..." << std::endl;
        (void)getchar();
    };

    navigator("start capture");

    // start capture
    startProcessCaptureRequest(param);

    navigator("stop capture");

    getAcquiredImageCount();

    // stop capture
    stopProcessCaptureRequest();

    return true;
}

bool InteractiveTest::startProcessCaptureRequest(
    const ProcessCaptureRequestParam& param)
{
    mReaderHelper.reset(new AImageReaderHelper(
            TestDefs::kAImageWidth, TestDefs::kAImageHeight,
            param.readerUsageAndFormat.second, param.readerUsageAndFormat.first,
            param.readerMaxImages, param.readerAsync));
    int ret = mReaderHelper->initAImageReader();
    if (ret != AMEDIA_OK)
    {
        CAM_ULOGME("init image reader helper failed: ret(%d)", ret);
        return false;
    }

    // register user-defined callback if available
    if (param.callback)
        mReaderHelper->setImageCallback(param.callback);

    mCameraHelper.reset(new ACameraHelper(param.cameraId, mCameraManager));
    camera_status_t status =
        mCameraHelper->openCamera(mReaderHelper->getNativeWindow(),
                                  {} /*physicalImgReaders*/,
                                  false /*usePhysicalSettings*/,
                                  TEMPLATE_RECORD);
    if (status != ACAMERA_OK)
    {
        CAM_ULOGME("open camera failed: ret(%d)", status);
        return false;
    }

    if (!mCameraHelper->isCameraReady())
    {
        CAM_ULOGME("open camera failed");
        return false;
    }

    return true;
}

bool InteractiveTest::stopProcessCaptureRequest()
{
    mCameraHelper.reset(nullptr);
    mReaderHelper.reset(nullptr);

    return true;
}

ssize_t InteractiveTest::getAcquiredImageCount() const
{
    ssize_t acquiredImageCount = mReaderHelper->getAcquiredImageCount();
    // do nothing if the image callback is available
    if (acquiredImageCount == -1)
        return acquiredImageCount;

    if (acquiredImageCount > 0)
    {
        CAM_ULOGMD("acquired image count(%zd)", acquiredImageCount);
        std::cout << "[----------] acquired image count(" << acquiredImageCount
                  << ")" << std::endl;
    }
    else
    {
        CAM_ULOGMD("no available image...");
        std::cout << "[----------] no available image..." << std::endl;
    }

    return acquiredImageCount;
}

bool InteractiveTest::isCameraDeviceSupported(const char* cameraId) const
{
    if (!cameraId)
    {
        CAM_ULOGME("invalid camera ID");
        return false;
    }

    const ACameraMetadata *staticMetadata =
        ACameraMetadataCache::getInstance().getACameraMetadata(
            mCameraManager, cameraId);

    // get the camera lens facing
    uint8_t facing;
    CAM_ULOGM_ASSERT(getLensFacing(*staticMetadata, facing),
                     "get lens facing failed");

    // NOTE: we always use the front-facing camera
    if (facing != ACAMERA_LENS_FACING_FRONT)
        return false;

    // camera device selection
    if (TestDefs::kEnableSecureImageDataTest) {
        bool isSecureImage = isCapabilitySupported(*staticMetadata,
                ACAMERA_REQUEST_AVAILABLE_CAPABILITIES_SECURE_IMAGE_DATA);

        CAM_ULOGMD("camera id(%s) isSupportSecureImage(%d) facing(%u)",
                cameraId, isSecureImage, facing);

        if (!isSecureImage)
        {
            CAM_ULOGMW("Camera does not support SECURE_IMAGE_DATA");
            return false;
        }
    } else {
        bool isBackwardCompatible = isCapabilitySupported(*staticMetadata,
                ACAMERA_REQUEST_AVAILABLE_CAPABILITIES_BACKWARD_COMPATIBLE);

        CAM_ULOGMD("camera id(%s) isSupportBackwardCompatible(%d) facing(%u)",
                cameraId, isBackwardCompatible, facing);

        if (!isBackwardCompatible)
        {
            CAM_ULOGMW("Camera does not support BACKWARD_COMPATIBLE");
            return false;
        }
    }

    return true;
}

bool InteractiveTest::skipAHardwareBufferUsage(
    enum AHardwareBuffer_UsageFlags flags) const
{
    if (TestDefs::kEnableSecureImageDataTest &&
       (flags != AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT))
    {
        CAM_ULOGMW("SECURE_IMAGE_DATA_TEST is enabled: "
                "skip usage other than PROTECTED_CONTENT");
        return true;
    }

    if (!TestDefs::kEnableSecureImageDataTest &&
       (flags == AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT))
    {
        CAM_ULOGMW("SECURE_IMAGE_DATA_TEST is disabled: "
                "skip usage PROTECTED_CONTENT");
        return true;
    }

    return false;
}

} // namespace tests
} // namespace NSCam
