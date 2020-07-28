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

#include "Camera2VendorTest.h"

#include "camera/ACameraMetadataCache.h"

#include <mtkcam/utils/std/ULog.h>

#include <camera/NdkCameraManager.h>

#include <thread>

// ------------------------------------------------------------------------

namespace NSCam {
namespace tests {

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAMERA_HAL_SERVER);

using namespace std::chrono_literals;

// ------------------------------------------------------------------------

bool Camera2VendorTest::processCaptureRequests(
    const ProcessCaptureRequestParam& param,
    const std::chrono::seconds durationInSecond)
{
    // start capture
    if (!startProcessCaptureRequest(param))
        return false;

    // wait for a specific duration if necessary
    if (durationInSecond.count())
    {
        CAM_ULOGMD("wait for %d second(s)", durationInSecond.count());
        std::cout << "[----------] wait for " << durationInSecond.count()
                  << " second(s)" << std::endl;
        std::this_thread::sleep_for(durationInSecond);
    }

    // wait for the camera frame(s)
    waitForAvailableImage();

    // stop capture
    if (!stopProcessCaptureRequest())
        return false;

    return true;
}

void Camera2VendorTest::waitForAvailableImage()
{
    // wait for the first camera frame if the image callback is available
    ssize_t acquiredImageCount = getAcquiredImageCount();
    if (acquiredImageCount == -1)
    {
        std::unique_lock<std::mutex> _l(mFirstCaptureDoneLock);
        bool noTimeout = mFirstCaptureDoneCondition.wait_for(
                _l, kFirstCaptureDoneTimeout,
                [&]
                {
                    // exit when receiving the first camera frame
                    if (mFirstCaptureDone)
                        return true;

                    return false;
                });
        EXPECT_TRUE(noTimeout) << "Timeout " <<
            kFirstCaptureDoneTimeout.count() << " second(s) expires";

        return;
    }

    // we need camera frames back from the Camera HAL3 impl.
    // Otherwise soldier on until the system timeout.
    size_t timeoutCount = 0;
    while (acquiredImageCount == 0)
    {
        CAM_ULOGMW("we need camera frame back from the camera HAL3 impl:"
                " timeout(%zu)", timeoutCount++);
        // wait for kAcquiredImageCountTimeout
        std::unique_lock<std::mutex> _l(mAcquiredImageCountTimeoutLock);
        mCondAcquiredImageCountTimeout.wait_for(_l, kAcquiredImageCountTimeout);

        acquiredImageCount = getAcquiredImageCount();
    }
}

// ------------------------------------------------------------------------

TEST_F(Camera2VendorTest, CreateWindowNativeHandle)
{
    for (int i = 0; i < mCameraIdList->numCameras; i++)
    {
        const char* cameraId(mCameraIdList->cameraIds[i]);
        ASSERT_TRUE(cameraId != nullptr) << "invalid camera ID";

        if (!isCameraDeviceSupported(cameraId))
            continue;

        for (const auto& readerUsageAndFormat : TestDefs::kReaderUsageAndFormat)
        {
            if (skipAHardwareBufferUsage(readerUsageAndFormat.first))
                continue;

            for (const auto& durationInSecond : { 1s, 5s, 10s })
            {
                const bool ret = processCaptureRequests(
                    {cameraId, readerUsageAndFormat,
                     TestDefs::kMaxAImagesStreaming, false,
                     AImageReaderHelper::imageAvailable_cb()},
                     durationInSecond);

                EXPECT_TRUE(ret) << "process capture requests failed";
                if (!ret)
                {
                    CAM_ULOGME("process capture requests failed");
                    return;
                }
            }
        }
    }
}

TEST_F(Camera2VendorTest, FirstCaptureLatency)
{
    const char *cameraId =
        [this](const ACameraIdList *cameraIdList) -> const char* {
        for (int i = 0; i < cameraIdList->numCameras; i++)
        {
            const char *cameraId = cameraIdList->cameraIds[i];
            if (!isCameraDeviceSupported(cameraId))
                continue;

            return cameraId;
        }

        return nullptr;
    }(mCameraIdList);

    EXPECT_NE(cameraId, nullptr) << "invalid camera ID";
    if (cameraId == nullptr)
    {
        CAM_ULOGME("invalid camera ID");
        return;
    }

    // TODO: leverage what defined in TestDefs::kReaderUsageAndFormat
    const TestDefs::ReaderUsageAndAFormat readerUsageAndFormat =
        TestDefs::kEnableSecureImageDataTest ?
        std::make_pair(AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT, AIMAGE_FORMAT_PRIVATE) :
        std::make_pair(AHARDWAREBUFFER_USAGE_CPU_READ_NEVER, AIMAGE_FORMAT_PRIVATE);

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::time_point();
    auto onBufferAvailable =
        [this, &end](AImageReader* reader) {
            (void)reader;

            // the second camera frame coming, do nothing.
            if (mFirstCaptureDone)
                return;

            end = std::chrono::steady_clock::now();

            // raise flag when receiving the first camera frame
            mFirstCaptureDone = true;
            mFirstCaptureDoneCondition.notify_one();
            CAM_ULOGMV("the first captured camera frame received");
        };

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    // open camera and get the first camera frame
    ASSERT_TRUE(processCaptureRequests(
        {cameraId, readerUsageAndFormat,
         TestDefs::kMaxAImagesStreaming, false,
         onBufferAvailable})) << "process capture requests failed";

    // latency measurement from opening the camera device to
    // the first camera frame arival
    auto firstCaptureLatencyInMillisecond =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // to logging system
    CAM_ULOGMD("The first camera frame latency is %s millisecond(s)",
               ::testing::PrintToString(firstCaptureLatencyInMillisecond).c_str());
    // to standard outout
    std::cout << "[----------] The first capture latency is " <<
        firstCaptureLatencyInMillisecond << " millisecond(s)" << std::endl;
}

} // namespace tests
} // namespace NSCam