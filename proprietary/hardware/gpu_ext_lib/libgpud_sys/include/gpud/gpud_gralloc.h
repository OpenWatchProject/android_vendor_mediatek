/*
 * Copyright (C) 2018-2019 MediaTek Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GPUD_INCLUDE_GPUD_GPUD_GRALLOC_H_
#define GPUD_INCLUDE_GPUD_GPUD_GRALLOC_H_

#include <hardware/gralloc.h>
#include <hardware/gralloc1.h>
#include <utils/Singleton.h>
#include <nativebase/nativebase.h>

using namespace android;

class GrallocHalWrapper : public Singleton<GrallocHalWrapper> {
 public:
    int registerBuffer(const ANativeWindowBuffer *buffer);
    int unregisterBuffer(const ANativeWindowBuffer *buffer);
    int lockBuffer(const ANativeWindowBuffer *buffer, int usage, void **vaddr);
    int unlockBuffer(const ANativeWindowBuffer *buffer);

 private:
    friend class Singleton<GrallocHalWrapper>;
    GrallocHalWrapper();
    ~GrallocHalWrapper();
    static int mapGralloc1Error(int grallocError);
    static buffer_handle_t getBufferHandle(const ANativeWindowBuffer *buffer);

    int mError;
    int mVersion;
    gralloc_module_t *mGrallocModule;
    // gralloc
    alloc_device_t *mAllocDevice;
    // gralloc1
    gralloc1_device_t *mGralloc1Device;
    GRALLOC1_PFN_RETAIN mPfnRetain;
    GRALLOC1_PFN_RELEASE mPfnRelease;
    GRALLOC1_PFN_GET_NUM_FLEX_PLANES mPfnGetNumFlexPlanes;
    GRALLOC1_PFN_LOCK mPfnLock;
    GRALLOC1_PFN_LOCK_FLEX mPfnLockFlex;
    GRALLOC1_PFN_UNLOCK mPfnUnlock;
};

#endif  // GPUD_INCLUDE_GPUD_GPUD_GRALLOC_H_
