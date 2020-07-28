/*
 * Copyright (C) 2015 MediaTek Inc.
 * Modification based on code covered by the mentioned copyright
 * and/or permission notice(s).
 */
/*
 * Copyright (C) 2012 The Android Open Source Project
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

#ifndef __ANDROID_HAL_camera3_TESTS_UTILS__
#define __ANDROID_HAL_camera3_TESTS_UTILS__

// Utility classes for camera3 HAL testing

#include <system/camera_metadata.h>
#include <hardware/camera3.h>
#include <hardware/gralloc.h>

#include <gui/Surface.h>
#include <gui/CpuConsumer.h>

#include <utils/List.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>

namespace android {
namespace camera3 {
namespace tests {


/**
 * Simple class to wait on the CpuConsumer to have a frame available
 */
class FrameWaiter : public CpuConsumer::FrameAvailableListener {
  public:
    FrameWaiter();

    /**
     * Wait for max timeout nanoseconds for a new frame. Returns
     * OK if a frame is available, TIMED_OUT if the timeout was reached.
     */
    status_t waitForFrame(nsecs_t timeout);

    virtual void onFrameAvailable();

    int mPendingFrames;
    Mutex mMutex;
    Condition mCondition;
};

struct HWModuleHelpers {
    /* attempt to unload the library with dlclose */
    static int closeModule(hw_module_t* module);
};

}
}
}

#endif
