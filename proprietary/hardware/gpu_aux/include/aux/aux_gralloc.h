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

#ifndef AUX_INCLUDE_AUX_AUX_GRALLOC_H_
#define AUX_INCLUDE_AUX_AUX_GRALLOC_H_

#include <string>
#include <hidl/HidlSupport.h>
#include <utils/StrongPointer.h>
#include <utils/Singleton.h>
#include <system/graphics.h>
#include <nativebase/nativebase.h>
#include <vndk/hardware_buffer.h>
#include <android/rect.h>

using android::hardware::hidl_handle;
using android::Singleton;
using android::status_t;

typedef int32_t PixelFormat;

typedef struct AuxBufferInfo {
    int err;
    int ion_fd;
    int width;
    int height;
    int format;
    int usage;
    int stride;
    int vertical_stride;
    int alloc_size;
    int status;
    int pool_id;
    int timestamp;
    int status2;
    int videobuffer_status;
    AuxBufferInfo() :
            err(0), ion_fd(-1), width(0), height(0),
            format(0), usage(0), stride(0), vertical_stride(0),
            alloc_size(0), status(0), pool_id(0), timestamp(0),
            status2(0), videobuffer_status(0) {}
} AuxBufferInfo;

class Rect : public ARect {
 public:
    inline Rect(int32_t l, int32_t t, int32_t r, int32_t b) {
        left = l;
        top = t;
        right = r;
        bottom = b;
    }

    // a valid rectangle has a non negative width and height
    inline bool isValid() const {
        return (getWidth() >= 0) && (getHeight() >= 0);
    }

    // an empty rect has a zero width or height, or is invalid
    inline bool isEmpty() const {
        return (getWidth() <= 0) || (getHeight() <= 0);
    }

    // rectangle's width
    __attribute__((no_sanitize("signed-integer-overflow")))
    inline int32_t getWidth() const {
        return right - left;
    }

    // rectangle's height
    __attribute__((no_sanitize("signed-integer-overflow")))
    inline int32_t getHeight() const {
        return bottom - top;
    }

    inline int32_t width() const { return getWidth(); }
    inline int32_t height() const { return getHeight(); }
};

class AuxGralloc : public Singleton<AuxGralloc> {
 public:
    status_t allocateBuffer(uint32_t width, uint32_t height, PixelFormat format, ANativeWindowBuffer **anb, AHardwareBuffer **ahb);
    status_t freeBuffer(ANativeWindowBuffer *anb, AHardwareBuffer *ahb);
    AuxBufferInfo getBufferInfo(const ANativeWindowBuffer *buffer);
    int32_t setBufferSFInfo(const ANativeWindowBuffer *buffer, uint32_t mask, uint32_t value);
 private:
    friend class Singleton<AuxGralloc>;
    AuxGralloc();
    ~AuxGralloc();
};

// A wrapper to IMapper
class AuxGrallocMapper {
 public:
    virtual ~AuxGrallocMapper();

    virtual bool isLoaded() const = 0;

    virtual status_t createDescriptor(void* bufferDescriptorInfo, void* outBufferDescriptor) const = 0;

    // Import a buffer that is from another HAL, another process, or is cloned.
    // The returned handle must be freed with freeBuffer.
    virtual status_t importBuffer(const hidl_handle& rawHandle, buffer_handle_t* outBufferHandle) const = 0;

    virtual void freeBuffer(buffer_handle_t bufferHandle) const = 0;

    virtual status_t validateBufferSize(buffer_handle_t bufferHandle, uint32_t width,
                                        uint32_t height, PixelFormat format,
                                        uint32_t layerCount, uint64_t usage,
                                        uint32_t stride) const = 0;

    virtual void getTransportSize(buffer_handle_t bufferHandle, uint32_t* outNumFds,
                                  uint32_t* outNumInts) const = 0;

    // The ownership of acquireFence is always transferred to the callee, even on errors.
    virtual status_t lock(buffer_handle_t bufferHandle, uint64_t usage, const Rect& bounds,
                          int acquireFence, void** outData, int32_t* outBytesPerPixel,
                          int32_t* outBytesPerStride) const = 0;

    // The ownership of acquireFence is always transferred to the callee, even on errors.
    virtual status_t lock(buffer_handle_t bufferHandle, uint64_t usage, const Rect& bounds,
                          int acquireFence, android_ycbcr* ycbcr) const = 0;

    // unlock returns a fence sync object (or -1) and the fence sync object is
    // owned by the caller
    virtual int unlock(buffer_handle_t bufferHandle) const = 0;

    // isSupported queries whether or not a buffer with the given width, height,
    // format, layer count, and usage can be allocated on the device.  If
    // *outSupported is set to true, a buffer with the given specifications may be successfully
    // allocated if resources are available.  If false, a buffer with the given specifications will
    // never successfully allocate on this device. Note that this function is not guaranteed to be
    // supported on all devices, in which case a status_t of INVALID_OPERATION will be returned.
    virtual status_t isSupported(uint32_t width, uint32_t height, PixelFormat format,
                                 uint32_t layerCount, uint64_t usage, bool* outSupported) const = 0;
};

#endif  // AUX_INCLUDE_AUX_AUX_GRALLOC_H_
