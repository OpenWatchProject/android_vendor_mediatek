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

#include <gtest/gtest.h>
#include "camera3test_fixtures.h"
#include "TestExtensions.h"
#include <dlfcn.h>

namespace android {
namespace camera3 {
namespace tests {

//Empty callback functions
void emptyNotify(const camera3_callback_ops_t *callback_ops __attribute__((unused)),
        const camera3_notify_msg_t *msg __attribute__((unused)))
{
}

void emptyCaptureResult(const camera3_callback_ops_t *callback_ops __attribute__((unused)),
        const camera3_capture_result_t *result __attribute__((unused)))
{
}


void Camera3Module::SetUp() {
//    TEST_EXTENSION_FORKING_SET_UP;
    hw_module_t *hw_module = NULL;

    status_t res;

    //Get module
    ASSERT_EQ(0, hw_get_module(CAMERA_HARDWARE_MODULE_ID, (const hw_module_t **)&hw_module))
                << "Can't get camera module";
    ASSERT_TRUE(NULL != hw_module)
                << "hw_get_module didn't return a valid camera module";

    IF_ALOGV() {
        std::cout << "  Camera module name: "
                << hw_module->name << std::endl;
        std::cout << "  Camera module author: "
                << hw_module->author << std::endl;
        std::cout << "  Camera module API version: 0x" << std::hex
                << hw_module->module_api_version << std::endl;
        std::cout << "  Camera module HAL API version: 0x" << std::hex
                << hw_module->hal_api_version << std::endl;
    }

    int16_t version2_0 = CAMERA_MODULE_API_VERSION_2_0;
    ASSERT_LE(version2_0, hw_module->module_api_version)
            << "Camera module version is 0x"
            << std::hex << hw_module->module_api_version
            << ", should be at least 2.0. (0x"
            << std::hex << CAMERA_MODULE_API_VERSION_2_0 << ")";

    mCamModule = reinterpret_cast<camera_module_t*>(hw_module);

    //Get number of cameras
    ASSERT_TRUE(NULL != mCamModule->get_number_of_cameras)
                << "get_number_of_cameras is not implemented";
    mNumCams = mCamModule->get_number_of_cameras();
    ASSERT_LT(0, mNumCams) << "No camera devices available!";

    IF_ALOGV() {
        std::cout << "  Camera device count: " << mNumCams << std::endl;
    }

    //Check which cameras supports hal3
    mCamSupportsHal3 = new bool[mNumCams];

    for (int i = 0; i < mNumCams; i++) {
        camera_info info;
        res = mCamModule->get_camera_info(i, &info);
        ASSERT_EQ(0, res)
                << "Failure getting camera info for camera " << i;
        IF_ALOGV() {
            std::cout << "  Camera device: " << std::dec
                        << i << std::endl;;
            std::cout << "    Facing: " << std::dec
                        << info.facing  << std::endl;
            std::cout << "    Orientation: " << std::dec
                        << info.orientation  << std::endl;
            std::cout << "    Version: 0x" << std::hex <<
                    info.device_version  << std::endl;
        }
        if (info.device_version == CAMERA_DEVICE_API_VERSION_3_2) {
            mCamSupportsHal3[i] = true;
            ASSERT_TRUE(NULL != info.static_camera_characteristics);
            IF_ALOGV() {
                std::cout << "    Static camera metadata:"  << std::endl;
                dump_indented_camera_metadata(info.static_camera_characteristics,
                        0, 1, 6);
            }
        } else {
            mCamSupportsHal3[i] = false;
        }
    }
}

void Camera3Module::TearDown() {
//    TEST_EXTENSION_FORKING_TEAR_DOWN;
    delete [] mCamSupportsHal3;
    hw_module_t *module = reinterpret_cast<hw_module_t*>(mCamModule);
//    ASSERT_EQ(0, HWModuleHelpers::closeModule(module));

    ASSERT_EQ(0, dlclose(module->dso));
}

void Camera3Device::SetUp() {
//    TEST_EXTENSION_FORKING_SET_UP;
    Camera3Module::SetUp();
    ASSERT_TRUE(NULL != mCamModule->common.methods->open)
                << "Camera open() is unimplemented";

    mDevice = new camera3_device;
    mCallback_ops = new camera3_callback_ops_t;
    mCallback_ops->notify = emptyNotify;
    mCallback_ops->process_capture_result = emptyCaptureResult;

        //Open camera device for each available camera
    for (int id = 0; id < mNumCams; id++) {
        if (!isHal3Supported(id)) continue;
    }
}

void Camera3Device::openCamera(int id) {
    //Open camera
    char camId[10];
    snprintf(camId, 10, "%d", id);
    hw_device_t *device = NULL;
    ASSERT_EQ(0, mCamModule->common.methods->open(
        (const hw_module_t*)mCamModule, camId, &device))
            << "Can't open camera device of id:" << id;
    ASSERT_TRUE(NULL != device)
            << "Camera open() returned a NULL device of id:" << id;
    mDevice = reinterpret_cast<camera3_device_t*>(device);

    //Save static info
    camera_info info;
    ASSERT_EQ(OK, mCamModule->get_camera_info(id, &info));
    mStaticInfo = info.static_camera_characteristics;
}

void Camera3Device::closeCamera() {
    ASSERT_EQ(0, mDevice->common.close(reinterpret_cast<hw_device_t*>(mDevice)));
}

void Camera3Device::setCallback(Notify pNotify, CaptureResult pCaptureResult) {
    mCallback_ops->notify = pNotify;
    mCallback_ops->process_capture_result = pCaptureResult;
}

void Camera3Device::init() {
    mDevice->ops->initialize(mDevice, mCallback_ops);
}

void Camera3Device::getStaticEntry(int tag, camera_metadata_ro_entry *result) {
    ASSERT_EQ(OK, find_camera_metadata_ro_entry(mStaticInfo, tag, result));
}

void Camera3Device::getResolutionList(int32_t format,
        const int32_t **list,
        size_t *count) {
    ALOGV("Getting resolutions for format %x", format);
    status_t res;
    if (format != HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED) {
        camera_metadata_ro_entry_t availableConfig;
        res = find_camera_metadata_ro_entry(mStaticInfo,
                ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS,
                &availableConfig);
        ASSERT_EQ(OK, res);

        uint32_t formatIdx;
        for (formatIdx=0; formatIdx < availableConfig.count; formatIdx+=4) {
            if (availableConfig.data.i32[formatIdx] == format) {
                *list = availableConfig.data.i32;
                *count = availableConfig.count;
                break;
            }
        }
        ASSERT_NE(availableConfig.count, formatIdx)
            << "No support found for format 0x" << std::hex << format;
    }
}

void Camera3Device::TearDown() {
//    TEST_EXTENSION_FORKING_TEAR_DOWN;
    delete mCallback_ops;
    delete mDevice;
    Camera3Module::TearDown();
}

}  // namespace tests
}  // namespace camera3
}  // namespace android
