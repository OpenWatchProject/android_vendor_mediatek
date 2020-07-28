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
#define DEBUG_LOG_TAG "mmshal_test"

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/TypeTraits.h>

#include "utils/Singleton.h"

#include <vendor/mediatek/hardware/camera/postproc/1.0/IPostDevice.h>
#include <vendor/mediatek/hardware/camera/postproc/1.0/types.h>

#include <android/hardware/camera/device/3.2/types.h>

#include <android/hardware/graphics/common/1.0/types.h>
#include <android/hardware/graphics/allocator/2.0/IAllocator.h>
#include <android/hardware/graphics/mapper/2.0/IMapper.h>

#include <grallocusage/GrallocUsageConversion.h>

#include <hardware/gralloc1.h>

#include <cutils/native_handle.h>


#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <cassert>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <regex>

#include <sync/sync.h>

#include <gtest/gtest.h>


#include <sys/stat.h>
#include <cutils/properties.h>


using ::vendor::mediatek::hardware::camera::postproc::V1_0::IPostDevice;
using ::vendor::mediatek::hardware::camera::postproc::V1_0::ChromaNRParam;


using ::android::hardware::graphics::common::V1_0::Dataspace;
using ::android::hardware::graphics::common::V1_0::BufferUsage;
using ::android::hardware::graphics::common::V1_0::PixelFormat;
using ::android::hardware::graphics::mapper::V2_0::IMapper;
using ::android::hardware::graphics::mapper::V2_0::YCbCrLayout;
using ::android::hardware::graphics::mapper::V2_0::BufferDescriptor;
using ::android::hardware::graphics::allocator::V2_0::IAllocator;
using ::android::hardware::hidl_death_recipient;
using ::android::hardware::hidl_handle;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hidl::base::V1_0::IBase;
using ::android::RefBase;
using ::android::sp;
using ::android::wp;

// ------------------------------------------------------------------------

constexpr uint64_t kProducerUsage(GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN |
        GRALLOC1_PRODUCER_USAGE_CPU_WRITE_OFTEN | GRALLOC1_PRODUCER_USAGE_CAMERA);

constexpr uint64_t kConsumerUsage(GRALLOC1_CONSUMER_USAGE_CPU_READ_OFTEN);

const int32_t kGralloc0Usage = android_convertGralloc1To0Usage(
        kProducerUsage, kConsumerUsage);

// ------------------------------------------------------------------------

class MapperHelper : public Singleton<MapperHelper>
{
public:
    MapperHelper();

    // Creates a buffer descriptor, which is used with IAllocator to allocate
    // buffers
    void createDescriptor(const IMapper::BufferDescriptorInfo& descriptorInfo,
            BufferDescriptor& descriptor);

    // NOTE: This function replace the imported buffer handle in-place.
    //       The returned outBufferHandle must be freed with freeBuffer().
    void importBuffer(buffer_handle_t& outBufferHandle);
    void freeBuffer(buffer_handle_t bufferHandle) const;
    // The ownership of acquireFence is always transferred to the callee, even
    // on error
    void lock(buffer_handle_t bufferHandle, uint64_t cpuUsage,
            const IMapper::Rect& accessRegion, int acquireFence, void*& buffer) const;
    void lockYCbCr(buffer_handle_t bufferHandle, uint64_t cpuUsage,
            const IMapper::Rect& accessRegion, int acquireFence, YCbCrLayout& layout) const;
    // unlock returns a fence sync object (or -1) and the fence sync object is
    // owned by the caller
    void unlock(buffer_handle_t bufferHandle, int& releaseFence) const;

private:
    sp<IMapper> mMapper;

    void init();
};

MapperHelper::MapperHelper()
{
    init();
}

void MapperHelper::init()
{
    // get IMapper service
    mMapper = IMapper::getService();
    ASSERT_TRUE(mMapper) << "failed to get IMapper interface";
}

void MapperHelper::createDescriptor(
        const IMapper::BufferDescriptorInfo& descriptorInfo,
        BufferDescriptor& descriptor)
{
    auto ret = mMapper->createDescriptor(
        descriptorInfo, [&descriptor](const auto& err, const auto& desc) {
            ASSERT_EQ(err, ::android::hardware::graphics::mapper::V2_0::Error::NONE)
            << "IMapper::createDescriptor() failed(" << toString(err) << ")";
            descriptor = desc;
        });
    ASSERT_TRUE(ret.isOk());
}

void MapperHelper::importBuffer(buffer_handle_t& outBufferHandle)
{
    buffer_handle_t importedHandle;
    auto ret = mMapper->importBuffer(hidl_handle(outBufferHandle),
            [&importedHandle](const auto& err, const auto& buffer)
            {
                ASSERT_EQ(
                    err, ::android::hardware::graphics::mapper::V2_0::Error::NONE);

                importedHandle = static_cast<buffer_handle_t>(buffer);
            });
    ASSERT_TRUE(ret.isOk()) << "importBuffer failed";

    outBufferHandle = importedHandle;
}

void MapperHelper::freeBuffer(buffer_handle_t bufferHandle) const
{
    if (!bufferHandle)
        return;

    auto ret = mMapper->freeBuffer(const_cast<native_handle_t*>(bufferHandle));
    ASSERT_TRUE(ret.isOk()) << "freeBuffer failed(" << toString(ret) << ")";
}

void MapperHelper::lock(buffer_handle_t bufferHandle, uint64_t cpuUsage,
            const IMapper::Rect& accessRegion, int acquireFence, void*& buffer) const
{
    auto _bufferHandle = const_cast<native_handle_t*>(bufferHandle);

    // put acquireFence in a hidl_handle
    hidl_handle acquireFenceHandle;
    NATIVE_HANDLE_DECLARE_STORAGE(acquireFenceStorage, 1, 0);
    if (acquireFence >= 0)
    {
        auto h = native_handle_init(acquireFenceStorage, 1, 0);
        h->data[0] = acquireFence;
        acquireFenceHandle = h;
    }

    auto ret = mMapper->lock(_bufferHandle, cpuUsage, accessRegion, acquireFenceHandle,
            [&](const auto& err, const auto& tmpBuffer)
            {
                ASSERT_EQ(
                    err, ::android::hardware::graphics::mapper::V2_0::Error::NONE)
                << "lock buffer" << _bufferHandle << " failed";

                buffer = tmpBuffer;
                CAM_LOGD("lock addr(0x%X)", reinterpret_cast<intptr_t>(tmpBuffer));
            });
    // we own acquireFence even on errors
    if (acquireFence >= 0)
        close(acquireFence);

    ASSERT_TRUE(ret.isOk());
}

void MapperHelper::lockYCbCr(buffer_handle_t bufferHandle, uint64_t cpuUsage,
        const IMapper::Rect& accessRegion, int acquireFence,
        YCbCrLayout& layout) const
{
    auto _bufferHandle = const_cast<native_handle_t*>(bufferHandle);

    // put acquireFence in a hidl_handle
    hidl_handle acquireFenceHandle;
    NATIVE_HANDLE_DECLARE_STORAGE(acquireFenceStorage, 1, 0);
    if (acquireFence >= 0)
    {
        auto h = native_handle_init(acquireFenceStorage, 1, 0);
        h->data[0] = acquireFence;
        acquireFenceHandle = h;
    }

    auto ret = mMapper->lockYCbCr(_bufferHandle, cpuUsage, accessRegion, acquireFenceHandle,
            [&](const auto& err, const auto& tmpLayout)
            {
                ASSERT_EQ(
                    err, ::android::hardware::graphics::mapper::V2_0::Error::NONE)
                << "lockYCbCr buffer " << _bufferHandle << "failed";
                layout = tmpLayout;
                CAM_LOGD("YCbCrLayout: %s", toString(tmpLayout).c_str());
            });
    // we own acquireFence even on errors
    if (acquireFence >= 0)
        close(acquireFence);

    ASSERT_TRUE(ret.isOk());
}

void MapperHelper::unlock(
        buffer_handle_t bufferHandle, int& releaseFence) const
{
    auto _bufferHandle = const_cast<native_handle_t*>(bufferHandle);

    releaseFence = -1;

    auto ret = mMapper->unlock(_bufferHandle,
            [&](const auto& err, const auto& tmpReleaseFence)
            {
                ASSERT_EQ(
                    err, ::android::hardware::graphics::mapper::V2_0::Error::NONE)
                << "unlock failed";

                auto fenceHandle = tmpReleaseFence.getNativeHandle();
                if (fenceHandle && fenceHandle->numFds == 1)
                {
                    int fd = dup(fenceHandle->data[0]);
                    if (fd >= 0) {
                        releaseFence = fd;
                    } else {
                        CAM_LOGD("failed to dup unlock release fence");
                        sync_wait(fenceHandle->data[0], -1);
                    }
                }
            });

    ASSERT_TRUE(ret.isOk());
}

// ------------------------------------------------------------------------

class AllocatorHelper : public Singleton<AllocatorHelper>
{
public:
    AllocatorHelper();

    void allocateGraphicBuffer(
            uint32_t width, uint32_t height, uint64_t usage, PixelFormat format,
            hidl_handle *handle, uint32_t *rowStride = nullptr) const;

private:
    sp<IAllocator> mAllocator;
    void init();
};

AllocatorHelper::AllocatorHelper()
{
    init();
}

void AllocatorHelper::init()
{
    // get IAllocator service
    mAllocator = IAllocator::getService();
    ASSERT_TRUE(mAllocator) << "failed to get IAllocator interface";
}

void AllocatorHelper::allocateGraphicBuffer(
        uint32_t width, uint32_t height, uint64_t usage, PixelFormat format,
        hidl_handle *handle, uint32_t *rowStride) const
{
    ASSERT_NE(handle, nullptr);

    IMapper::BufferDescriptorInfo descriptorInfo {
            .width = width,
            .height = height,
            /*
             * NOTE: A buffer with multiple layers may be used as
             * the backing store of an array texture.
             * All layers of a buffer share the same characteristics (e.g.,
             * dimensions, format, usage). Devices that do not support
             * GRALLOC1_CAPABILITY_LAYERED_BUFFERS must allocate only buffers
             * with a single layer.
             */
            .layerCount = 1,
            .format = format,
            .usage = usage
        };

    BufferDescriptor descriptor;
    MapperHelper::getInstance().createDescriptor(descriptorInfo, descriptor);

    auto ret = mAllocator->allocate(descriptor, 1u,
        [&](auto err, uint32_t stride, const auto& buffers) {
            CAM_LOGD("stride(%u)", stride);
            if (rowStride)
                *rowStride = stride;
            ASSERT_EQ(err, ::android::hardware::graphics::mapper::V2_0::Error::NONE)
                << "IAllocator::allocate() failed(" << toString(err) << ")";
            ASSERT_EQ(buffers.size(), 1u);
            *handle = buffers[0];
        });
    ASSERT_TRUE(ret.isOk());
}

// ------------------------------------------------------------------------

class MMsTest : public ::testing::Test
{
protected:
    MMsTest();
    ~MMsTest();
    // Load files from directory
    //void loadFromFile(hidl_handle &rawHandle, const File& file, const PixelFormat format);
protected:
    sp<IPostDevice> mService;
private:
    void init();
};

MMsTest::MMsTest()
{
    init();
}

MMsTest::~MMsTest()
{
}

void MMsTest::init()
{
    // connect to ISPModule
    mService = IPostDevice::getService("internal/0");
    ASSERT_TRUE(mService) << "failed to get IPostDevice interface";
}

TEST_F(MMsTest, swNRTest)
{
    CAM_LOGD("SWNR HIDL test");

    int32_t width = 4656;
    int32_t height = 3496;
    int32_t stride = 4656;
    int32_t halformat = HAL_PIXEL_FORMAT_YCrCb_420_SP;
    PixelFormat pixelFormat = PixelFormat::YCBCR_420_888;
    char *inputFileName = "/data/vendor/camera_dump/input.yuv";

    // load image into hidl handle
    hidl_handle image_handle;

    AllocatorHelper::getInstance().allocateGraphicBuffer(
            width, height,
            kGralloc0Usage,
            pixelFormat, &image_handle);

    buffer_handle_t importedBuffer = image_handle.getNativeHandle();
    MapperHelper::getInstance().importBuffer(importedBuffer);

    // lock buffer for writing
    const IMapper::Rect region { 0, 0, width, height };
    int fence = -1;

    YCbCrLayout layout;
    MapperHelper::getInstance().lockYCbCr(importedBuffer,
            static_cast<uint64_t>(BufferUsage::CPU_WRITE_OFTEN),
            region, fence, layout);
    //CAM_LOGD("lock layout: %s", toString(layout).c_str());

    std::ifstream ifs(inputFileName,
            (std::ios::in | std::ios::binary));
    ASSERT_TRUE(ifs.is_open()) << strerror(errno) << ": " <<
        inputFileName;

    auto yData = static_cast<uint8_t*>(layout.y);
    auto cbData = static_cast<uint8_t*>(layout.cb);
    auto crData = static_cast<uint8_t*>(layout.cr);

    char value;
    for (uint32_t y = 0; y < height; y++)
    {
        for (uint32_t x = 0; x < width; x++)
        {
            if (ifs.get(value))
                yData[layout.yStride * y + x] = value;
            else
                CAM_LOGW("cannot read more data from file ");
        }
    }

    uint32_t uvHeight = height >> 1;
    uint32_t uvWidth = width >> 1;

    CAM_LOGD("YUV Ystride(%d), Cstride(%d) chromaStep(%d)", layout.yStride, layout.cStride, layout.chromaStep);

    // NOTE: if chromaStep is 2, adopt semiplanar arrangement
    //       to access the cb/cr point
    //       (chroma values are interleaved and each chroma
    //       value is one byte).
    //       Otherwise, we adopt planar arrangement
    //       to access the cb/cr point respectively.
    // FIXME: get the actual pixel format to known the UV ordering
    if (layout.chromaStep == 2)
    {
        for (uint32_t y = 0; y < uvHeight; y++) {
            for (uint32_t x = 0, step = 0; x < uvWidth; x++, step += layout.chromaStep) {
                if (ifs.get(value))
                    crData[layout.cStride * y + step] = value;
                else
                    CAM_LOGW("cannot read more data from file ");

                if (ifs.get(value))
                    cbData[layout.cStride * y + step] = value;
                else
                    CAM_LOGW("cannot read more data from file ");
            }
        }
    }
    else
    {
        for (uint32_t y = 0; y < uvHeight; y++) {
            for (uint32_t x = 0, step = 0; x < uvWidth; x++, step+=layout.chromaStep) {
                if (ifs.get(value))
                    cbData[layout.cStride * y + step] = value;
                else
                    CAM_LOGW("cannot read more data from file ");
            }
        }

        for (uint32_t y = 0; y < uvHeight; y++) {
            for (uint32_t x = 0, step = 0; x < uvWidth; x++, step+=layout.chromaStep) {
                if (ifs.get(value))
                    crData[layout.cStride * y + step] = value;
                else
                    CAM_LOGW("cannot read more data from file ");
            }
        }
    }

    ifs.close();

    ChromaNRParam param;
    param.open_id = 0;
    param.current_iso = 1200;
    param.src_width = width;
    param.src_height = height;
    param.src_yPitch = layout.yStride;
    param.src_uvPitch = layout.cStride;
    param.src_format = halformat;
    param.inputHandle = image_handle;
    mService->doChromaNR(param);

    // Write to file for checking
    // open file
    char *outputFileName = "/data/vendor/camera_dump/output.yuv";
    std::ofstream ofs(outputFileName, std::ofstream::out);
    ASSERT_TRUE(ofs.is_open()) << strerror(errno) << ": " << outputFileName;

    uint8_t padding = 0;

    for (uint32_t y = 0; y < height; y++)
    {
        for (uint32_t x = 0; x < layout.yStride; x++)
        {
            if(x < width) {
                ofs << yData[layout.yStride * y + x];
            } else {
                ofs << padding;
            }
        }
    }

    uint32_t uvStride = layout.cStride >> 1;
    if (layout.chromaStep == 2)
    {
        for (uint32_t y = 0; y < uvHeight; y++) {
            for (uint32_t x = 0, step = 0; x < uvStride; x++, step += layout.chromaStep) {
                if(x < uvWidth) {
                    ofs << crData[layout.cStride * y + step];
                    ofs << cbData[layout.cStride * y + step];
                } else {
                    ofs << padding;
                    ofs << padding;
                }
            }
        }
    }

    MapperHelper::getInstance().unlock(importedBuffer, fence);
    if (fence >= 0)
        close(fence);

    ofs.close();
}

// ------------------------------------------------------------------------

int runGoogleTest()
{
    return RUN_ALL_TESTS();
}

// ------------------------------------------------------------------------

void showUsage(const char *programName)
{
    std::printf("Usage: %s [OPTION]\n\n", programName);
    std::printf("DESCRIPTION\n");
    std::printf("\t-g [--help|-h|-?] \texecute Google tests.\n");
    std::printf("\t-j\t\t\texecute JSON tests.\n");
    std::printf("\t\t\t\t--help, -h or -? lists the supported flags and their usage\n");
}

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        showUsage(argv[0]);
        return ::android::OK;
    }

    // parse the command line and remove all recognized Google Test flags
    // must call this function before calling RUN_ALL_TESTS()
    ::testing::InitGoogleTest(&argc, argv);

    int opt;
    while ((opt = getopt(argc, argv, "gj")) != -1)
    {
        switch (opt)
        {
            case 'g':
            case 'j':
                runGoogleTest();
                break;
            default:
                break;
        }
    }

    return ::android::OK;
}
