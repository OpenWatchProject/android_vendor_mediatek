/*
 * Copyright 2016 The Android Open Source Project
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

#define LOG_TAG "vendor.mediatek.hardware.wfo@1.0-service"

#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/Looper.h>
#include <utils/StrongPointer.h>
#include "WifiOffload.h"

using vendor::mediatek::hardware::wfo::V1_0::IWifiOffload;
using vendor::mediatek::hardware::wfo::V1_0::implementation::WifiOffload;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

int main(int /*argc*/, char** argv) {
    UNUSED(argv);
    LOG(INFO) << "WifiOffload Hal is starting...";
    configureRpcThreadpool(1, true /* callerWillJoin */);
    // Setup hwbinder service
    android::sp<vendor::mediatek::hardware::wfo::V1_0::IWifiOffload> service =
        new vendor::mediatek::hardware::wfo::V1_0::implementation::WifiOffload();
    CHECK_EQ(service->registerAsService("wfo_hidl_service"), android::NO_ERROR)
        << "Failed to register wfo HAL";
    joinRpcThreadpool();
    LOG(INFO) << "WifiOffload Hal is terminating...";
    return 0;
}
