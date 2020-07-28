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

#define LOG_TAG "AImageReaderHelper"

#include "AImageReaderHelper.h"

#include <cmath>
#include <sstream>
#include <fstream>

#include <mtkcam/utils/std/ULog.h>

#include <sync/sync.h>

#include <vndk/hardware_buffer.h>

#include <vendor/mediatek/hardware/gpu/1.0/IGraphicExt.h>
#include <graphics_mtk_defs.h>

#include <uree/system.h>
#include <uree/mem.h>

#include <dlfcn.h>

//#define DUMP_BUFFER

// ------------------------------------------------------------------------

namespace NSCam {

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAMERA_HAL_SERVER);

using ::android::sp;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_handle;
using ::android::hardware::graphics::common::V1_0::PixelFormat;
using ::vendor::mediatek::hardware::gpu::V1_0::IGraphicExt;
using ::vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_ATTRIBUTE_QUERY;
using ::vendor::mediatek::hardware::gpu::V1_0::GRALLOC_EXTRA_RESULT;

// ------------------------------------------------------------------------

static inline std::string toString(uint32_t format)
{
    switch (format)
    {
        // TODO: support other format if necessary
        // vendor extension format in graphics_mtk_defs.h
        case HAL_PIXEL_FORMAT_NV12:
            return "nv12";
        case AHARDWAREBUFFER_FORMAT_YCrCb_420_SP: // NV21
            return "nv21";
        default:
            return "unknown";
    }
}

static std::unique_ptr<char[], std::function<void(char[])>> toWorldSharedMemory(
        uint32_t secureHandle, size_t size)
{
    if (!NSCam::kMTEESupported)
    {
        CAM_ULOGMW("M-TEE is not supported, do nothing");
        std::unique_ptr<char[], std::function<void(char[])>> wsm;
        return wsm;
    }

#ifdef __LP64__
    const std::string UREE_LIBRARY_PATH("/vendor/lib64/libgz_uree.so");
#else
    const std::string UREE_LIBRARY_PATH("/vendor/lib/libgz_uree.so");
#endif
    // load library
    // NOTE: We esteem this library to be a plug-in.
    //       EL2 is not supported with the absence of this library.
    void *handle = dlopen(UREE_LIBRARY_PATH.c_str(), RTLD_LAZY);
    if (!handle)
    {
        CAM_ULOGMW("cannot load %s(%s)", UREE_LIBRARY_PATH.c_str(), dlerror());
        std::unique_ptr<char[], std::function<void(char[])>> wsm;
        return wsm;
    }

    auto closeHandle = [](void *handle) {
        // close library
        if (handle)
        {
            dlclose(handle);
        }
        return nullptr;
    };

    // reset error
    dlerror();

    // load symbols
    bool isSymbolsLoaded = true;
    auto loadSymbol = [&](const char *symbolName = "")
    {
        void *symbolAddr = dlsym(handle, symbolName);
        if (!symbolAddr)
        {
            isSymbolsLoaded = false;
            CAM_ULOGME("%s" ,dlerror());
        }
        return symbolAddr;
    };

    const std::string MEM_SRV_NAME("com.mediatek.geniezone.srv.mem");
    typedef TZ_RESULT CreateSession_t(const char*, UREE_SESSION_HANDLE*);
    const auto createSession =
        reinterpret_cast<CreateSession_t*>(loadSymbol("UREE_CreateSession"));

    typedef TZ_RESULT RegisterSharedmem_t(
            UREE_SESSION_HANDLE, UREE_SHAREDMEM_HANDLE*, UREE_SHAREDMEM_PARAM*);
    const auto registerSharedMem =
        reinterpret_cast<RegisterSharedmem_t*>(loadSymbol("UREE_RegisterSharedmem"));

    typedef TZ_RESULT ION_CP_Chm2Shm_t(
            UREE_SESSION_HANDLE, UREE_SHAREDMEM_HANDLE, UREE_ION_HANDLE, uint32_t);
    const auto secureHandleToWSM =
        reinterpret_cast<ION_CP_Chm2Shm_t*>(loadSymbol("UREE_ION_CP_Chm2Shm"));

    typedef TZ_RESULT UnregisterSharedmem_t(
            UREE_SESSION_HANDLE, UREE_SHAREDMEM_HANDLE);
    const auto unregisterSharedMem =
        reinterpret_cast<UnregisterSharedmem_t*>(loadSymbol("UREE_UnregisterSharedmem"));

    typedef TZ_RESULT CloseSession_t(UREE_SESSION_HANDLE);
    auto closeSession =
        reinterpret_cast<CloseSession_t*>(loadSymbol("UREE_CloseSession"));

    if (!isSymbolsLoaded)
    {
        CAM_ULOGME("%s", dlerror());
        handle = closeHandle(handle);
        std::unique_ptr<char[], std::function<void(char[])>> wsm;
        return wsm;
    }

    // allocate a World Shared Memory (WSM) accessible among EL0, EL1 and EL2
    // NOTE: WSM for EL2 must be aligned to memory page
    auto memptr = (char *)memalign(sysconf(_SC_PAGESIZE), size);
    CAM_ULOGMD("secureHandle(%#x): WSM(%#" PRIxPTR ") size(%zu) allocated",
            secureHandle, reinterpret_cast<uintptr_t>(memptr), size);
    std::unique_ptr<char[], std::function<void(char[])>> wsm(
            memptr,
            [](char memory[])
            {
                free(memory);
                CAM_ULOGMD("WSM(%#" PRIxPTR ") released",
                    reinterpret_cast<uintptr_t>(memory));
            });

    // create session for shared memory
    UREE_SESSION_HANDLE memSrvSession;
    TZ_RESULT ret = (*createSession)(MEM_SRV_NAME.c_str(), &memSrvSession);
    if (ret != TZ_RESULT_SUCCESS)
        CAM_ULOGME("create mem sesion failed(%#x)", ret);

    // register WSM to EL2
#if defined(__LP64__)
    constexpr auto MAX_VALUE = std::numeric_limits<uint32_t>::max();
#endif

    UREE_SHAREDMEM_HANDLE wsmHandle;
    UREE_SHAREDMEM_PARAM wsmParam {
        .buffer = memptr,
#if defined(__LP64__)
        .size = (size <= MAX_VALUE) ? static_cast<uint32_t>(size) : MAX_VALUE };
#else
        .size = size };
#endif

#if defined(__LP64__)
    if (__builtin_expect(size > MAX_VALUE, false))
        CAM_ULOGME("buffer size(%zu) to be copied exceeds %u", size, MAX_VALUE);
#endif

    ret = (*registerSharedMem)(memSrvSession, &wsmHandle, &wsmParam);
    if (ret != TZ_RESULT_SUCCESS)
        CAM_ULOGME("register WSM to EL2 failed(%#x)", ret);

    // a deep copy of secure-to-shared memory
    // NOTE: This API is disabled by default on production load
    //       (i.e. Android user build) for security reason.
    ret = (*secureHandleToWSM)(memSrvSession, wsmHandle, secureHandle, size);
    if (ret != TZ_RESULT_SUCCESS)
    {
        if (ret == static_cast<int>(TZ_RESULT_ERROR_NOT_SUPPORTED))
        {
            CAM_ULOGMW("UREE_ION_CP_Chm2Shm is not supported(%#x)", ret);
        }
        if (ret != static_cast<int>(TZ_RESULT_ERROR_NOT_SUPPORTED))
        {
            CAM_ULOGME("copy from secure(%u) to shared memory(%#" PRIxPTR ")" \
                    " failed(%#x)",
                    secureHandle, reinterpret_cast<uintptr_t>(memptr), ret);
        }
    }

    // unregister WMS from EL2
    ret = (*unregisterSharedMem)(memSrvSession, wsmHandle);
    if (ret != TZ_RESULT_SUCCESS)
        CAM_ULOGME("unregister shared memory from EL2 failed(%#x)", ret);

    // close session for shared memory
    ret = (*closeSession)(memSrvSession);
    if (ret != TZ_RESULT_SUCCESS)
        CAM_ULOGME("close mem sesion failed(%#x)", ret);

    // close library
    handle = closeHandle(handle);

    return wsm;
}

// ------------------------------------------------------------------------

std::mutex AImageReaderHelper::sAImageReaderHelperObserverLock;
std::set<void *> AImageReaderHelper::sAImageReaderHelperObserver;

void AImageReaderHelper::onImageAvailable(void* context, AImageReader* reader)
{
    AImageReaderHelper *thisContext =
        reinterpret_cast<AImageReaderHelper *>(context);

    // Check if AImageReaderHelper instance is valid or not
    bool isAImageReaderHelperValid = [&]() {
        std::lock_guard<std::mutex> _l(sAImageReaderHelperObserverLock);
        auto search = sAImageReaderHelperObserver.find(thisContext);
        if (__builtin_expect(search == sAImageReaderHelperObserver.end(), false))
            return false;
        return true;
    }();

    if (__builtin_expect(isAImageReaderHelperValid, true))
    {
        thisContext->handleImageAvailable(reader);
    }
    else
    {
        CAM_ULOGMW("AImageReader(%" PRIxPTR ") instance does not exist, do nothing",
            reinterpret_cast<intptr_t>(thisContext));
    }
}

AImageReaderHelper::AImageReaderHelper(int32_t width, int32_t height,
        int32_t format, uint64_t usage, int32_t maxImages, bool async)
    : mWidth(width),
    mHeight(height),
    mFormat(format),
    mUsage(usage),
    mMaxImages(maxImages),
    mAsync(async),
    mAcquiredImageCount(0)
{
    // Insert AImageReaderHelper instance to the observer list
    {
        std::lock_guard<std::mutex> _l(sAImageReaderHelperObserverLock);
        auto ret = sAImageReaderHelperObserver.insert(this);
        assert(ret.second == true);
        CAM_ULOGMV("insert AImageReader(0x%" PRIxPTR ")",
            reinterpret_cast<intptr_t>(*ret.first));
    }
}

AImageReaderHelper::~AImageReaderHelper()
{
    std::lock_guard<std::mutex> _l(mExitingLock);

    // Delete an AImageReader instance shall return all images generated by
    // this reader back to the system.
    // NOTE: imageReaderAnw will released by AImageReader's dtor
    if (mImageReaderAnw)
    {
        // the dtor of AImageReader will stop the dedicated callback thread
        AImageReader_delete(mImageReader);
    }

    // Remove AImageReaderHelper instance from the observer list
    {
        std::lock_guard<std::mutex> _l(sAImageReaderHelperObserverLock);
        auto search = sAImageReaderHelperObserver.find(this);
        assert(search != sAImageReaderHelperObserver.end());
        CAM_ULOGMV("remove AImageReader(0x%" PRIxPTR ")",
            reinterpret_cast<intptr_t>(*search));
        sAImageReaderHelperObserver.erase(search);
    }
}

int AImageReaderHelper::initAImageReader()
{
    if (mImageReader != nullptr || mImageReaderAnw != nullptr)
    {
        CAM_ULOGME("image reader has been initialized: mImageReader(0x%" PRIxPTR \
                ") mImageReaderAnw(0x%" PRIxPTR ")",
                reinterpret_cast<intptr_t>(mImageReader),
                reinterpret_cast<intptr_t>(mImageReaderAnw));
        return AMEDIA_ERROR_INVALID_OPERATION;
    }

    // [Consumer]
    // AImageReader constructor with an additional parameter for the consumer usage.
    auto ret = AImageReader_newWithUsage(mWidth, mHeight, mFormat, mUsage,
            mMaxImages, &mImageReader);
    if (ret != AMEDIA_OK || mImageReader == nullptr)
    {
        CAM_ULOGME("create image reader failed: ret(%d) mImageReader(0x%" PRIxPTR \
                ")", ret, reinterpret_cast<intptr_t>(mImageReader));
        return ret;
    }

    // [Consumer]
    // Get the native_handle_t corresponding to the ANativeWindow owned by the
    // AImageReader provided.
    ret = AImageReader_getWindowNativeHandle(mImageReader, &mImageReaderAnw);
    if (ret != AMEDIA_OK || mImageReaderAnw == nullptr)
    {
        CAM_ULOGME("get native handle owned by image reader failed: ret(%d) " \
                "mImageReader(0x%" PRIxPTR ") mImageReaderAnw(0x%" PRIxPTR ")",
                ret, reinterpret_cast<intptr_t>(mImageReader),
                reinterpret_cast<intptr_t>(mImageReaderAnw));
        return ret;
    }

    // Set the AImageReader_ImageListener::onImageAvailable() listener of
    // this image reader.
    // Calling this method will replace previously registered listeners.
    ret = AImageReader_setImageListener(
            mImageReader, &mImageReaderImageAvailableListener);
    if (ret != AMEDIA_OK)
    {
        CAM_ULOGME("set ImageListener failed: ret(%d) " \
                "mImageReader(0x%" PRIxPTR ")", ret,
                reinterpret_cast<intptr_t>(mImageReader));
        return ret;
    }

    // Set the AImageReader_BufferRemovedListener::onBufferRemoved() of
    // this image reader.
    //
    // This callback is called when an old AHardwareBuffer is about
    // to be removed from the image reader.
    //
    // Note that registering this callback is optional unless the user
    // holds on extra reference to AHardwareBuffer returned from
    // AImage_getHardwareBuffer by calling AHardwareBuffer_acquire or
    // creating external graphic objects, such as EglImage, from it.
    ret = AImageReader_setBufferRemovedListener(
            mImageReader, &mImageReaderBufferRemovedListener);
    if (ret != AMEDIA_OK)
    {
        CAM_ULOGME("set BufferRemovedListener failed: ret(%d) " \
                "mImageReader(0x%" PRIxPTR ")", ret,
                reinterpret_cast<intptr_t>(mImageReader));
        return ret;
    }

    return ret;
}

ssize_t AImageReaderHelper::getAcquiredImageCount()
{
    // return -1 if the image callback is available
    {
        std::lock_guard<std::mutex> _l(mImageAvailableLock);
        if (mImageAvailable_cb)
            return -1;
    }

    // the default behavior is to return the amount of acquired images
    return mAcquiredImageCount;
}

int32_t AImageReaderHelper::getWidth() const
{
    int32_t width;
    media_status_t status = AImageReader_getWidth(mImageReader, &width);
    if (status != AMEDIA_OK)
        CAM_ULOGME("get width failed");
    return width;
}

int32_t AImageReaderHelper::getHeight() const
{
    int32_t height;
    media_status_t status = AImageReader_getHeight(mImageReader, &height);
    if (status != AMEDIA_OK)
        CAM_ULOGME("get height failed");
    return height;
}

int32_t AImageReaderHelper::getFormat() const
{
    int32_t format;
    media_status_t status = AImageReader_getFormat(mImageReader, &format);
    if (status != AMEDIA_OK)
        CAM_ULOGME("get format failed");
    return format;
}

sp<AImageReaderHelper::HGraphicBufferProducer> AImageReaderHelper::getHGBPFromHandle()
{
    const native_handle_t *handle(mImageReaderAnw);
    if (handle == nullptr)
        return nullptr;

    if (handle->numFds != 0  ||
            handle->numInts < std::ceil(sizeof(size_t) / sizeof(int)))
        return nullptr;

    return convertNativeHandleToHGBP(handle);
}

bool AImageReaderHelper::getSecureHandle(
        const hidl_handle& bufferHandle, SECHAND& secureHandle)
{
    // get IGraphicExt service
    sp<IGraphicExt> graphicExt = IGraphicExt::getService();
    if (!graphicExt)
    {
        CAM_ULOGME("failed to get IGraphicExt interface");
        return false;
    }

    // Reference: vendor/mediatek/proprietary/hardware/gpu_hidl/service/GraphicExt.cpp
    graphicExt->gralloc_extra_query(
            hidl_handle(bufferHandle),
            GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_SECURE_HANDLE,
            [&secureHandle](auto retval, const auto& dataHandle)
            {
                auto ret = GRALLOC_EXTRA_RESULT(retval);
                if (ret != GRALLOC_EXTRA_RESULT::GRALLOC_EXTRA_OK)
                {
                    CAM_ULOGME("gralloc_extra_query failed: %s",
                    toString(ret).c_str());
                    return;
                }

                // NOTE: the gralloc_extra implementation place the secure handle into
                // the address of &dataHandle->data[0]
                secureHandle = *reinterpret_cast<const SECHAND *>(dataHandle->data);
            });

    return true;
}

bool AImageReaderHelper::getPixelFormat(
        const hidl_handle& bufferHandle, PixelFormat& pixelFormat)
{
    // get IGraphicExt service
    sp<IGraphicExt> graphicExt = IGraphicExt::getService();
    if (!graphicExt)
    {
        CAM_ULOGME("failed to get IGraphicExt interface");
        return false;
    }

    // Reference: vendor/mediatek/proprietary/hardware/gpu_hidl/service/GraphicExt.cpp
    graphicExt->gralloc_extra_query(
            hidl_handle(bufferHandle),
            GRALLOC_EXTRA_ATTRIBUTE_QUERY::GRALLOC_EXTRA_GET_FORMAT,
            [&pixelFormat](auto retval, const auto& dataHandle)
            {
                auto ret = GRALLOC_EXTRA_RESULT(retval);
                if (ret != GRALLOC_EXTRA_RESULT::GRALLOC_EXTRA_OK)
                {
                    CAM_ULOGME("gralloc_extra_query failed: %s",
                    toString(ret).c_str());
                    return;
                }

                // NOTE: the gralloc_extra implementation place the format into
                // the address of &dataHandle->data[0]
                pixelFormat = *reinterpret_cast<const PixelFormat*>(
                    dataHandle->data);
            });

    return true;
}

// Reference: frameworks/av/media/utils/AImageReaderUtils.cpp
// AOSP design: Retrieves HGraphicBufferProducer corresponding to
// the native_handle_t provided (this native handle MUST have been obtained
// by AImageReader_getWindowNativeHandle()).
sp<AImageReaderHelper::HGraphicBufferProducer> AImageReaderHelper::convertNativeHandleToHGBP(
        const native_handle_t* handle)
{
    // Read the size of the halToken vec<uint8_t>
    hidl_vec<uint8_t> halToken;
    halToken.setToExternal(
        reinterpret_cast<uint8_t *>(const_cast<int *>(&(handle->data[1]))),
        handle->data[0]);
    sp<HGraphicBufferProducer> hgbp =
        HGraphicBufferProducer::castFrom(
                ::android::retrieveHalInterface(halToken));
    return hgbp;
}

bool AImageReaderHelper::isFormatSupported(uint32_t format)
{
    switch (format)
    {
        // TODO: support other format if necessary
        // vendor extension format in graphics_mtk_defs.h
        case HAL_PIXEL_FORMAT_NV12:
        case AHARDWAREBUFFER_FORMAT_YCrCb_420_SP: // NV21
            return true;
        default:
            return false;
    }
}

std::vector<AImageReaderHelper::Plane> AImageReaderHelper::getPlanes(
        const AHardwareBuffer_Desc& desc)
{
    std::vector<Plane> planes;

    switch (desc.format)
    {
        // NV12 and NV21 are half width, half height
        case HAL_PIXEL_FORMAT_NV12:
        case AHARDWAREBUFFER_FORMAT_YCrCb_420_SP: // NV21
            planes.push_back({ desc.width, desc.height, 1, desc.stride });
            planes.push_back({ desc.width >> 1, desc.height >> 1, 2, desc.stride >> 1 });
            planes.push_back({ desc.width >> 1, desc.height >> 1, 2, desc.stride >> 1 });
            break;
        default:
            CAM_ULOGMW("buffer format(%#x) is not supported", desc.format);
    }

    return planes;
}

void AImageReaderHelper::dumpBuffer(const AHardwareBuffer_Desc& desc,
        const native_handle_t* bufferHandle, size_t imageCount)
{
    if (!bufferHandle)
    {
        CAM_ULOGME("invalid buffer handle");
        return;
    }

    if (!isFormatSupported(desc.format))
    {
        CAM_ULOGME("buffer format(%#x) is not supported", desc.format);
        return;
    }

    // calculate buffer size
    size_t bufferSize = 0;
    for (const auto& plane : getPlanes(desc))
        bufferSize += plane.rowStride * plane.height;

    // get the secure handle of a buffer handle
    SECHAND secureHandle;
    if (!getSecureHandle(hidl_handle(bufferHandle), secureHandle))
    {
        CAM_ULOGME("failed to get secure handle");
        return;
    }

    // copy from secure buffer to normal buffer
    auto memPtr = toWorldSharedMemory(secureHandle, bufferSize);

    // dump normal buffer to file
    const std::string kDumpPath("/data/vendor/camera_dump/");

    std::ostringstream fileName;
    const std::string kDelimiter("_");
    fileName << kDumpPath << "buf" << kDelimiter <<
        std::setw(4) << std::setfill('0') << imageCount
        << kDelimiter << desc.width << kDelimiter << desc.height <<
        kDelimiter << desc.stride << "." << toString(desc.format);

    std::ofstream ofs(fileName.str(), std::ios::binary);
    ofs.write(memPtr.get(), bufferSize);
    ofs.close();
}

void AImageReaderHelper::setImageCallback(imageAvailable_cb cb)
{
    std::lock_guard<std::mutex> _l(mImageAvailableLock);
    mImageAvailable_cb = cb;
}

void AImageReaderHelper::handleImageAvailable(AImageReader* reader)
{
    struct AutoLock {
        AutoLock(std::mutex& _lock) : lock(_lock) {
            isLocked = lock.try_lock();
        }
        ~AutoLock() { lock.unlock(); }

        std::mutex& lock;
        bool isLocked = false;
    } _l(mExitingLock);

    if (!_l.isLocked)
    {
        CAM_ULOGMW("AImageReader is exiting, do nothing.");
        return;
    }

    // delegate to the user-defined callback
    {
        std::lock_guard<std::mutex> _l(mImageAvailableLock);
        if (mImageAvailable_cb)
        {
            mImageAvailable_cb(reader);
            return;
        }
    }

    // NOTE: The default behavior is to count the acquired images

    // make sure AImage will be deleted automatically when it goes out of
    // scope.
    auto imageDeleter = [this](AImage* img)
    {
        if (mAsync)
            // AImage_deleteAsync() with kNoFenceFd is equivalent to AImage_delete()
            AImage_deleteAsync(img, kNoFenceFd);
        else
            AImage_delete(img);
    };
    std::unique_ptr<AImage, decltype(imageDeleter)> outImage(nullptr, imageDeleter);

    // default operation if no registered callback
    AImage *image = nullptr;
    int acquireFenceFd = kNoFenceFd;
    media_status_t status = [&]
    {
        return mAsync ?
            AImageReader_acquireNextImageAsync(
                    reader, &image, &acquireFenceFd) :
            AImageReader_acquireNextImage(reader, &image);
    }();

    if (status != AMEDIA_OK || image == nullptr)
    {
        CAM_ULOGME("acquire the next image failed: status(%d) image(0x%" PRIxPTR ")",
                status, reinterpret_cast<intptr_t>(image));
        return;
    }

    if (acquireFenceFd != kNoFenceFd)
    {
        // wait fence (if exist) for 1000 ms
        int err = sync_wait(acquireFenceFd, 1000);

        // the image must be sent back the the system and
        // delete the AImage object from memory even if the failure of sync_wait()
        if (err < 0)
            CAM_ULOGMW("sync_wait() failed: %s", strerror(errno));
    }

    outImage.reset(image);

    // get the hardware buffer handle of the input image
    // intended for GPU and/or hardware access
    AHardwareBuffer *outBuffer = nullptr;
    status = AImage_getHardwareBuffer(outImage.get(), &outBuffer);
    if (status != AMEDIA_OK || outBuffer == nullptr)
    {
        CAM_ULOGME("get hardware buffer failed: status(%d) outBuffer(0x%" PRIxPTR ")",
                status, reinterpret_cast<intptr_t>(outBuffer));
        return;
    }

    // get a description of the AHardwareBuffer
    AHardwareBuffer_Desc outDesc;
    AHardwareBuffer_describe(outBuffer, &outDesc);

    const native_handle_t *outBufferHandle = AHardwareBuffer_getNativeHandle(outBuffer);
    if (outBufferHandle == nullptr)
    {
        CAM_ULOGME("get native handle failed");
        return;
    }

    CAM_ULOGMV("buffer w(%u) h(%u) layers(%u) f(%u) u(%" PRIu64 ") s(%u)",
            outDesc.width, outDesc.height, outDesc.layers,
            outDesc.format, outDesc.usage, outDesc.stride);

    if ((outDesc.usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT) ==
            AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT)
    {
        // get the secure handle of a buffer handle
        SECHAND secureHandle;
        if (getSecureHandle(hidl_handle(outBufferHandle), secureHandle))
            CAM_ULOGMD("secure handle(0x%x)", secureHandle);
        else
            CAM_ULOGME("get secure handle failed");

#ifdef DUMP_BUFFER
        dumpBuffer(outDesc, outBufferHandle, mAcquiredImageCount);
#endif
    }

    mAcquiredImageCount++;

    // force release the managed object before exiting the callback
    outImage.reset(nullptr);

    mExitingLock.unlock();
}

} // namespace NSCam
