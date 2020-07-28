/*
 * Copyright (C) 2015 MediaTek Inc.
 * Modification based on code covered by the mentioned copyright
 * and/or permission notice(s).
 */
/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __ANDROID_HAL_CAMERA3_TEST_COMMON__
#define __ANDROID_HAL_CAMERA3_TEST_COMMON__

#include <gtest/gtest.h>
#include <hardware/hardware.h>
#include <hardware/camera3.h>
#include "camera3_utils.h"
#include "TestExtensions.h"

namespace android {
namespace camera3 {
namespace tests {

static const int kMmaxCams = 2;
static const uint16_t kVersion3_2 = HARDWARE_MODULE_API_VERSION(3, 2);

typedef void (*Notify)(const camera3_callback_ops_t*, const camera3_notify_msg_t *);
typedef void (*CaptureResult)(const camera3_callback_ops_t*, const camera3_capture_result_t *);

void emptyNotify(const camera3_callback_ops_t *callback_ops,
        const camera3_notify_msg_t *msg);

void emptyCaptureResult(const camera3_callback_ops_t *callback_ops,
        const camera3_capture_result_t *result);

class Camera3Module /*: public testing::Test*/ {
 public:

     //Member functions
 public:
    /******************************************************
     * Flow within SetUp
     * 1. Get camera hardware module
     * 2. Make sure camera module api is above 2.0
     * 3. Get number of available cameras
     * 4. Determine which camera supports HAL 3.2
     ******************************************************/
    void                SetUp();

    /******************************************************
     * Close module using camera3_utils::HWModuleHelper.
     * Delete member variable.
     ******************************************************/
    void                TearDown();

    bool                        isHal3Supported(int id)
                                    { return mCamSupportsHal3[id]; }

    int                         getNumOfCams()
                                    { return mNumCams; }

    const camera_module_t*      getCamModule()
                                    { return mCamModule; }

    //Member variables
 protected:
    bool                        *mCamSupportsHal3;
    int                         mNumCams;
    camera_module_t             *mCamModule;
};

class Camera3Device : public Camera3Module {
 public:
    Camera3Device() :
        mDevice(NULL) {}

    ~Camera3Device() {}

    //Member functions
 public:
    /******************************************************
     * Flow with SetUp
     * 1. Using camera module to open each available camera
     * device
     * 2. Initialize each device with EMPTY callback_ops
     ******************************************************/
    void                SetUp();

    /******************************************************
     * Get resolutionlist of certain format using
     * tag ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS.
     * If format is available within device, result will be
     * store in list. Otherwise, assertion happens.
     *
     * Input:
     *      format - the desire format
     * Output:
     *      list   - n * 4 (format, width, height, input/output stream)
     *      count  - size of list
     ******************************************************/
    void                        getResolutionList(int32_t format,
                                                const int32_t **list,
                                                size_t *count);

    void                        openCamera(int id);

    void                        closeCamera();

    void                        setCallback(Notify pNotify, CaptureResult pCaptureResult);

    void                        init();

    void                        getStaticEntry(int tag, camera_metadata_ro_entry *result);

    /******************************************************
     * Close all camera devices and delete member variables
     ******************************************************/
    void                TearDown();

    camera3_device_t*           getCam3Device()
                                    { return mDevice; }

    camera3_device              *mDevice;

    //Member variables
 protected:
    camera3_callback_ops_t      *mCallback_ops;
    const camera_metadata_t     *mStaticInfo;
};

}  // namespace tests
}  // namespace camera3
}  // namespace android

#endif  // __ANDROID_HAL_CAMERA3_TEST_COMMON__
