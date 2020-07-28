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

#ifndef VENDOR_MEDIATEK_HARDWARE_CAMERA3_MAIN_HAL_SECURITY_TESTS_CAMERA2VENDORTEST_H
#define VENDOR_MEDIATEK_HARDWARE_CAMERA3_MAIN_HAL_SECURITY_TESTS_CAMERA2VENDORTEST_H

#include "utils/TestBase.h"
#include "InteractiveTest.h"

#include <chrono>
#include <memory>
#include <condition_variable>

// ------------------------------------------------------------------------

namespace NSCam {
namespace tests {

/**
 * This test fixture is for testing Camera2 VNDK API.
 */
class Camera2VendorTest : public TestBase, protected InteractiveTest
{
protected:
    // methods from NSCam::tests::InteractiveTest
    bool processCaptureRequests(
        const ProcessCaptureRequestParam &param,
        const std::chrono::seconds durationInSecond = 0s) override;

    static constexpr std::chrono::seconds kFirstCaptureDoneTimeout = 5s;
    // used by test case Camera2VendorTest.FirstCaptureLatency
    std::atomic_bool mFirstCaptureDone = false;
    mutable std::mutex mFirstCaptureDoneLock;
    std::condition_variable mFirstCaptureDoneCondition;

private:
    /**
     * @brief Due to the imager and the camera pipeline takes milliseconds to
     *      output the camera frames, waitForAvailableImage is a handy API that
     *      provides two behaviors:
     *      1. wait for the first camera frame if the image callback is available.
     *      2. wait for camera frames back from the Camera HAL3 impl.
     *         Otherwise soldier on until the system timeout.
     */
    void waitForAvailableImage();

    // NOTE: We enforce a timeout condition kAcquiredImageCountTimeout here
    //       to wait forever so that the system timeout occurs and
    //       generates the exception thereof.
    static constexpr std::chrono::milliseconds kAcquiredImageCountTimeout =
        std::chrono::milliseconds(5 * 1000);

    mutable std::mutex mAcquiredImageCountTimeoutLock;
    std::condition_variable mCondAcquiredImageCountTimeout;
}; // class Camera2VendorTest

} // namespace tests
} // namespace NSCam

#endif // VENDOR_MEDIATEK_HARDWARE_CAMERA3_MAIN_HAL_SECURITY_TESTS_CAMERA2VENDORTEST_H
