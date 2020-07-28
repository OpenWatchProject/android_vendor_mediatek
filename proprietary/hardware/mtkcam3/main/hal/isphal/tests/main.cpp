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
#define DEBUG_LOG_TAG "isphal_test"

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/TypeTraits.h>

#include "utils/Singleton.h"
#include "types.h"

#include <vendor/mediatek/hardware/camera/isphal/1.0/IISPModule.h>
#include <vendor/mediatek/hardware/camera/isphal/1.0/IDevice.h>
#include <vendor/mediatek/hardware/camera/isphal/1.0/ICallback.h>

#include <android/hardware/camera/provider/2.4/ICameraProvider.h>
#include <system/camera_metadata.h>
#include <mtkisp_metadata.h>

#include <android/hardware/camera/device/3.2/types.h>

#include <android/hardware/graphics/common/1.0/types.h>
#include <android/hardware/graphics/allocator/2.0/IAllocator.h>
#include <android/hardware/graphics/mapper/2.0/IMapper.h>

#include <grallocusage/GrallocUsageConversion.h>

#include <hardware/gralloc1.h>

#include <cutils/native_handle.h>
#include <cutils/properties.h>
// To avoid tag not sync with MTK HAL we included
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
//#include <system/camera_metadata_tags.h>

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

#include <json/json.h>

#include <sys/stat.h>
#include <cutils/properties.h>
#include <random>

using namespace isphal_test;

using namespace ::vendor::mediatek::hardware::camera;
using namespace ::vendor::mediatek::hardware::camera::isphal::V1_0;

using ::android::hardware::graphics::common::V1_0::Dataspace;
using ::android::hardware::graphics::common::V1_0::BufferUsage;
using ::android::hardware::graphics::common::V1_0::PixelFormat;
using ::android::hardware::graphics::mapper::V2_0::IMapper;
using ::android::hardware::graphics::mapper::V2_0::YCbCrLayout;
using ::android::hardware::graphics::mapper::V2_0::BufferDescriptor;
using ::android::hardware::graphics::allocator::V2_0::IAllocator;
using ::android::hardware::camera::device::V3_2::CameraMetadata;
using ::android::hardware::camera::device::V3_2::CameraBlob;
using ::android::hardware::camera::device::V3_2::CameraBlobId;
using ::android::hardware::camera::provider::V2_4::ICameraProvider;
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

static std::unordered_map<std::string, uint32_t> gIspMetaMap {
    { "com.mediatek.control.capture.hintForIspTuning", MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING },
    { "com.mediatek.control.capture.hintForIspFrameCount", MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_COUNT },
    { "com.mediatek.control.capture.hintForIspFrameIndex", MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_INDEX },
    { "com.android.control.aeMode.sensitivity", ANDROID_SENSOR_SENSITIVITY}
};

// ------------------------------------------------------------------------

template <typename T>
const std::string stringify(const std::vector<T>& v)
{
    std::stringstream ss;

    ss << "{ ";
    // enum type
    for (size_t i = 0; i < v.size(); i++)
    {
        if (i != 0) ss << ", ";
        ss << NSCam::toLiteral(v[i]);
    }
    ss << " }";

    return ss.str();
}

template <>
const std::string stringify(const std::vector<ISPFormat>& v)
{
    std::stringstream ss;

    for (const auto& ispFormat : v)
        ss << toString(ispFormat);

    return ss.str();
}

long getFileSize(const std::string& fileName)
{
    struct stat stat_buf;
    int rc = stat(fileName.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

// ------------------------------------------------------------------------

class JSONContainer : public Singleton<JSONContainer>
{
public:
    static const std::string kTestCaseSettingFile;

    Json::Value& getTestCaseSetting();

private:
    mutable std::mutex mTestCaseSettingLock;

    // Mandatory names in hierarchy:
    // [TestSuiteName][TestName][InputFiles|OutputFiles|RootDirectory]
    // Instantiated on the first use.
    Json::Value mTestCaseSetting;

    inline bool isTestCaseSettingLoadedLocked() const;
    ::android::status_t loadTestCaseSetting(const std::string& file);
};

const std::string JSONContainer::kTestCaseSettingFile =
    "/sdcard/test_case_setting.json";

bool JSONContainer::isTestCaseSettingLoadedLocked() const
{
    return !mTestCaseSetting.isNull();
}

::android::status_t JSONContainer::loadTestCaseSetting(const std::string& file)
{
    std::lock_guard<std::mutex> _l(mTestCaseSettingLock);

    if (isTestCaseSettingLoadedLocked())
    {
        CAM_LOGV("test case setting loaded, do nothing");
        return ::android::OK;
    }

    std::ifstream ifs(file, std::ifstream::in);
    if (!ifs.is_open())
    {
        std::cerr << strerror(errno) << ": " << file << "\n";
        return ::android::NAME_NOT_FOUND;
    }

    // create JSON object/array from the test case setting file
    Json::Reader reader;
    if (!reader.parse(ifs, mTestCaseSetting))
    {
        ifs.close();

        std::cerr << "parse JSON file " << file
            << " failed(" << reader.getFormattedErrorMessages() << ")\n";

        mTestCaseSetting = Json::Value();
        return ::android::BAD_VALUE;
    }

    ifs.close();

    return ::android::OK;
}

Json::Value& JSONContainer::getTestCaseSetting()
{
    loadTestCaseSetting(kTestCaseSettingFile);

    std::lock_guard<std::mutex> _l(mTestCaseSettingLock);
    assert(isTestCaseSettingLoadedLocked());

    return mTestCaseSetting;
};

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

/**
 * Adapter for HIDL HAL interface calls; calls into the HIDL HAL interfaces.
 */
class HalInterface : virtual public RefBase
{
public:
    HalInterface();
    virtual ~HalInterface();

    const sp<hidl_death_recipient> getHidlDeathRecipient() const
    { return mClientCallback.get(); }

    const sp<ICallback> getICallback() const
    { return mClientCallback; }

    // NOTE: for performance consideration, we *move* inputBuffers and outputBuffers
    //       so that they are invalid after recordISPRequest() call.

    using FileBuffer = std::pair<std::vector<File>, std::vector<ISPBuffer>>;
    void recordISPRequest(uint32_t requestNumber,
            FileBuffer&& inputBuffers, FileBuffer&& outputBuffers);

    // timeout in milliseconds
    // TIMEOUT_NEVER may be passed to the waitResult method to indicate that it
    // should wait indefinitely for the result.
    // return ::android::OK if success, otherwise return ::android::TIMED_OUT if
    // not receiving the result in time.
    enum { TIMEOUT_NEVER = -1 };
    ::android::status_t waitResult(int timeout);

private:
    struct ClientCallback :
        virtual public ICallback,
        virtual public hidl_death_recipient
    {
        ClientCallback(const wp<HalInterface>& interface) : mInterface(interface) {}

        ::android::status_t waitResult(int timeout);

        // Implementation of android::hardware::hidl_death_recipient interface
        void serviceDied(uint64_t cookie, const wp<IBase>& who) override;

        // Implementation of vendor::mediatek::hardware::camera::isphal::V1_0::ICallback
        Return<void> processResult(const ISPResult& result) override;

        // Implementation of vendor::mediatek::hardware::camera::isphal::V1_0::ICallback
        Return<void> processEarlyResult(const ISPResult& result) override;

        struct CaptureRequest
        {
            uint32_t requestNumber;
            FileBuffer inputBuffers;
            FileBuffer outputBuffers;

            explicit CaptureRequest(uint32_t _requestNumber,
                    FileBuffer&& _inputBuffers,
                    FileBuffer&& _outputBuffers)
            {
                requestNumber = _requestNumber;
                inputBuffers = std::move(_inputBuffers);
                outputBuffers = std::move(_outputBuffers);
            }
        };
        std::unordered_map<uint32_t, CaptureRequest> mCaptureRequest;

    private:
        wp<HalInterface> mInterface;

        // mResultCV is used for the processResult() in a synchronous manner
        mutable std::mutex mResultMutex;
        std::condition_variable mResultCV;

        void writeFile(const CaptureRequest& request);
    };

    sp<ClientCallback> mClientCallback;
};

HalInterface::HalInterface()
{
    // register the recipient for a notification if the hwbinder object goes away
    mClientCallback = new HalInterface::ClientCallback(this);
}

HalInterface::~HalInterface()
{
}

void HalInterface::recordISPRequest(uint32_t requestNumber,
        FileBuffer&& inputBuffers, FileBuffer&& outputBuffers)
{
    ASSERT_TRUE(mClientCallback);

    bool isOK = mClientCallback->mCaptureRequest.emplace(requestNumber,
            ClientCallback::CaptureRequest(requestNumber,
                std::move(inputBuffers), std::move(outputBuffers))).second;
    ASSERT_TRUE(isOK) << __FUNCTION__ << ": emplace capture request " <<
        requestNumber << " failed";
}

::android::status_t HalInterface::waitResult(int timeout)
{
    return mClientCallback->waitResult(timeout);
}

::android::status_t HalInterface::ClientCallback::waitResult(int timeout)
{
    std::unique_lock<std::mutex> l(mResultMutex);

    CAM_LOGD("Wait ISP result");
    if (timeout == TIMEOUT_NEVER)
    {
        const auto kTimeout = std::chrono::milliseconds(3000);
        do {
            std::cv_status ret = mResultCV.wait_for(l, kTimeout);
            if (ret == std::cv_status::no_timeout)
                return ::android::OK;

            // show timed-out message verbosely
            CAM_LOGE("waitResult timeout in %d milliseconds, wait again...",
                    kTimeout.count());
        } while (true);
    }

    const auto kTimeout = std::chrono::milliseconds(timeout);
    const std::cv_status ret = mResultCV.wait_for(l, kTimeout);
    if (ret == std::cv_status::timeout)
    {
        CAM_LOGE("waitResult timeout in %d milliseconds", timeout);
        return ::android::TIMED_OUT;
    }

    return ::android::OK;
}

void HalInterface::ClientCallback::serviceDied(
        uint64_t cookie, const wp<IBase>& who)
{
    (void) who;
    CAM_LOGI("ISPModule(%" PRIu64 ") has died", cookie);

    // TODO: do something here if necessary when the HAL server has gone

    // release HIDL interface
    auto parent = mInterface.promote();
    if (parent)
        parent->mClientCallback.clear();
}

Return<void> HalInterface::ClientCallback::processResult(
        const ISPResult& result)
{
    if (result.status != isphal::V1_0::Status::OK)
    {
        CAM_LOGE("processResult %u failed", result.reqNumber);
        return Void();
    }

    auto search = mCaptureRequest.find(result.reqNumber);
    if (search == mCaptureRequest.end())
    {
        CAM_LOGE("cannot find capture request %u", result.reqNumber);
        return Void();
    }

    // notify the outstanding test case to finish its remaining task
    CAM_LOGD("Isp callback done");
    mResultCV.notify_one();

    // writeFile may cost performance therefore we set it after condition notify
    writeFile(search->second);

    // remove result from the list of the original rquests
    mCaptureRequest.erase(search);
    return Void();
}

Return<void> HalInterface::ClientCallback::processEarlyResult(
        const ISPResult& result)
{
    // TBD

    return Void();
}

void HalInterface::ClientCallback::writeFile(const CaptureRequest& request)
{
    const uint32_t requestNumber(request.requestNumber);
    const std::vector<File>& outputFiles(request.outputBuffers.first);
    const std::vector<ISPBuffer>& outputBuffers(request.outputBuffers.second);
    ASSERT_EQ(outputFiles.size(), outputBuffers.size());

    const size_t size = outputFiles.size();
    for (size_t i = 0; i < size; i++)
    {
        const File& image(outputFiles[i]);
        const std::string& imageFQN(image.getFullyQualifiedName());
        CAM_LOGD("writing file (%s)", imageFQN.c_str());

        // create working directories
        std::regex rx("[a-z_0-9]+\\.{1}(jpeg|jpg|yuv|raw)$", std::regex::icase);
        std::smatch sm;
        if (std::regex_search(imageFQN, sm, rx))
        {
            CAM_LOGV("matched prefix: %s entire matched expression: %s",
                    sm.prefix().str().c_str(), sm.str(0).c_str());
            int ret = mkdir(sm.prefix().str().c_str(),
                    S_IRWXU | S_IRGRP | S_IXGRP  | S_IROTH | S_IXOTH);
            ASSERT_TRUE((ret == 0) || (errno == EEXIST))
                << "fail to create directory " << sm.prefix()
                << ": " << strerror(errno);
        }
        else
        {
            CAM_LOGD("Cannot open file");
            ASSERT_TRUE(false) << "cannot open file:" << imageFQN;
        }

        // open file
        std::ofstream ofs(imageFQN, std::ofstream::out);
        ASSERT_TRUE(ofs.is_open()) << strerror(errno) << ": " << imageFQN;

        CAM_LOGD("processResult: requestNumber(%u)", requestNumber);

        buffer_handle_t importedBuffer = outputBuffers[i].buffer.getNativeHandle();
        MapperHelper::getInstance().importBuffer(importedBuffer);

        if (image.isJPEG())
        {
            // NOTE: jpeg is a blob format
            // lock buffer for writing
            const IMapper::Rect region { 0, 0, static_cast<int32_t>(image.size), 1u };
            int fence = -1;
            void* data = nullptr;

            MapperHelper::getInstance().lock(importedBuffer,
                    (BufferUsage::CPU_WRITE_OFTEN | BufferUsage::CPU_READ_OFTEN),
                    region, fence, data);
            ASSERT_NE(data, nullptr) << "lock metadata failed";

            // show CameraBlob
            const char *jpegBuffer = reinterpret_cast<const char*>(data);
            const CameraBlob *transportHeader = reinterpret_cast<const CameraBlob*>(
                    jpegBuffer + image.size - sizeof(CameraBlob));
            CAM_LOGD("%s", toString(*transportHeader).c_str());

            // extract jpeg image from jpeg stream buffer and save it to file
            ofs.write(jpegBuffer, transportHeader->blobSize);
            ofs.close();

            MapperHelper::getInstance().unlock(importedBuffer, fence);
            if (fence >= 0)
                close(fence);
        }
        else if (image.format == static_cast<int>(PixelFormat::RAW16))
        {
            // NOTE: jpeg is a blob format
            // lock buffer for writing
            const IMapper::Rect region { 0, 0, static_cast<int32_t>(image.size), 1u };
            int fence = -1;
            void* data = nullptr;

            MapperHelper::getInstance().lock(importedBuffer,
                    (BufferUsage::CPU_WRITE_OFTEN | BufferUsage::CPU_READ_OFTEN),
                    region, fence, data);
            ASSERT_NE(data, nullptr) << "lock metadata failed";

            // show CameraBlob
            const char *buffer = reinterpret_cast<const char*>(data);
            ofs.write(buffer, image.size);
            ofs.close();

            MapperHelper::getInstance().unlock(importedBuffer, fence);
            if (fence >= 0)
                close(fence);
        }
        else
        {
            // TODO: handle non-JPEG cases (e.g.YUV) and save result into file system
            //ASSERT_TRUE(false) << "Please implement this part";
            // lock buffer for writing
            const IMapper::Rect region { 0, 0, static_cast<int32_t>(image.width),
                static_cast<int32_t>(image.height) };
            int fence = -1;
            uint8_t padding = 0;

            YCbCrLayout layout;
            MapperHelper::getInstance().lockYCbCr(importedBuffer,
                    static_cast<uint64_t>(BufferUsage::CPU_WRITE_OFTEN | BufferUsage::CPU_READ_OFTEN),
                    region, fence, layout);
            CAM_LOGD("lock layout: %s", toString(layout).c_str());
            CAM_LOGD("Write YUV stride(%d, %d)", layout.yStride, layout.cStride);

            auto yData = static_cast<uint8_t*>(layout.y);
            auto cbData = static_cast<uint8_t*>(layout.cb);
            auto crData = static_cast<uint8_t*>(layout.cr);

            char value;
            for (uint32_t y = 0; y < image.height; y++)
            {
                for (uint32_t x = 0; x < layout.yStride; x++)
                {
                    if(x < image.width) {
                        ofs << yData[layout.yStride * y + x];
                    } else {
                        ofs << padding;
                    }
                }
            }

            uint32_t uvHeight = image.height >> 1;
            uint32_t uvWidth = image.width >> 1;
            uint32_t uvStride = layout.cStride >> 1;

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
        }

        MapperHelper::getInstance().freeBuffer(importedBuffer);

        // close file
        ofs.close();
    }
}

// ------------------------------------------------------------------------

class Camera3DeviceHelper : public Singleton<Camera3DeviceHelper>
{
public:
    Camera3DeviceHelper();

    void getCameraDeviceName(const std::string& cameraId,
            std::string& cameraDeviceName);

    void getCameraCharacteristics(const std::string& cameraDeviceName,
            CameraMetadata& cameraMetadata);

private:
    // NOTE: the service name is defined in
    //       vendor/mediatek/proprietary/hardware/mtkcam3/main/hal/service/service.cpp
    static const std::string kCameraProviderName;

    sp<ICameraProvider> mProvider;

    void init();

    ::android::status_t parseDeviceName(const std::string& name,
            uint16_t *major, uint16_t *minor, std::string *type, std::string *id);

    std::unordered_map<std::string, CameraMetadata> mCameraCharacteristics;
};
const std::string Camera3DeviceHelper::kCameraProviderName("internal/0");

Camera3DeviceHelper::Camera3DeviceHelper()
{
    init();
}

void Camera3DeviceHelper::init()
{
    mProvider = ICameraProvider::getService(kCameraProviderName);
    ASSERT_TRUE(mProvider) << "failed to get camera provider interface";

    CAM_LOGI("camera provider(%s) name(%s) discovered",
            ICameraProvider::descriptor, kCameraProviderName.c_str());
}

void Camera3DeviceHelper::getCameraDeviceName(
        const std::string& cameraId, std::string& cameraDeviceName)
{
    // find the desired camera device name
    Return<void> ret;
    ret = mProvider->getCameraIdList(
        [&](auto status, const auto& idList) {
            ASSERT_EQ(::android::hardware::camera::common::V1_0::Status::OK, status)
                << "getCameraIdList failed: " << toString(status);

            for (size_t i = 0; i < idList.size(); i++)
            {
                const std::string& _cameraDeviceName(idList[i]);

                CAM_LOGV("camera device name[%zu] is %s", i, _cameraDeviceName.c_str());
                // TODO: check if need to keep the parsed information in mind
                // Extract major/minor versions, type and id
                uint16_t major, minor;
                std::string type, id;
                ::android::status_t res = parseDeviceName(_cameraDeviceName,
                    &major, &minor, &type, &id);
                ASSERT_EQ(::android::OK, res) << "parseDeviceName failed: " <<
                    strerror(-res) << " (" << res << ")";

                if (id.compare(cameraId) != 0)
                    continue;

                cameraDeviceName = _cameraDeviceName;
                CAM_LOGD("camera device name(%s) found", cameraDeviceName.c_str());
                break;
            }
        });
    ASSERT_TRUE(ret.isOk());
}

::android::status_t Camera3DeviceHelper::parseDeviceName(const std::string& name,
        uint16_t *major, uint16_t *minor, std::string *type, std::string *id)
{
    // Format must be "device@<major>.<minor>/<type>/<id>"

#define ERROR_MSG_PREFIX "%s: Invalid device name '%s'. " \
    "Should match 'device@<major>.<minor>/<type>/<id>' - "

    if (!major || !minor || !type || !id)
        return ::android::INVALID_OPERATION;

    // Verify starting prefix
    const char expectedPrefix[] = "device@";

    if (name.find(expectedPrefix) != 0) {
        CAM_LOGE(ERROR_MSG_PREFIX
                "does not start with '%s'",
                __FUNCTION__, name.c_str(), expectedPrefix);
        return ::android::BAD_VALUE;
    }

    // Extract major/minor versions
    constexpr std::string::size_type atIdx = sizeof(expectedPrefix) - 2;
    std::string::size_type dotIdx = name.find('.', atIdx);
    if (dotIdx == std::string::npos) {
        CAM_LOGE(ERROR_MSG_PREFIX
                "does not have @<major>. version section",
                __FUNCTION__, name.c_str());
        return ::android::BAD_VALUE;
    }
    std::string::size_type typeSlashIdx = name.find('/', dotIdx);
    if (typeSlashIdx == std::string::npos) {
        CAM_LOGE(ERROR_MSG_PREFIX
                "does not have .<minor>/ version section",
                __FUNCTION__, name.c_str());
        return ::android::BAD_VALUE;
    }

    char *endPtr;
    errno = 0;
    long majorVal = strtol(name.c_str() + atIdx + 1, &endPtr, 10);
    if (errno != 0) {
        CAM_LOGE(ERROR_MSG_PREFIX
                "cannot parse major version: %s (%d)",
                __FUNCTION__, name.c_str(), strerror(errno), errno);
        return ::android::BAD_VALUE;
    }
    if (endPtr != name.c_str() + dotIdx) {
        CAM_LOGE(ERROR_MSG_PREFIX
                "major version has unexpected length",
                __FUNCTION__, name.c_str());
        return ::android::BAD_VALUE;
    }
    long minorVal = strtol(name.c_str() + dotIdx + 1, &endPtr, 10);
    if (errno != 0) {
        CAM_LOGE(ERROR_MSG_PREFIX
                "cannot parse minor version: %s (%d)",
                __FUNCTION__, name.c_str(), strerror(errno), errno);
        return ::android::BAD_VALUE;
    }
    if (endPtr != name.c_str() + typeSlashIdx) {
        CAM_LOGE(ERROR_MSG_PREFIX
                "minor version has unexpected length",
                __FUNCTION__, name.c_str());
        return ::android::BAD_VALUE;
    }
    if (majorVal < 0 || majorVal > UINT16_MAX || minorVal < 0 || minorVal > UINT16_MAX) {
        CAM_LOGE(ERROR_MSG_PREFIX
                "major/minor version is out of range of uint16_t: %ld.%ld",
                __FUNCTION__, name.c_str(), majorVal, minorVal);
        return ::android::BAD_VALUE;
    }

    // Extract type and id

    std::string::size_type instanceSlashIdx = name.find('/', typeSlashIdx + 1);
    if (instanceSlashIdx == std::string::npos) {
        CAM_LOGE(ERROR_MSG_PREFIX
                "does not have /<type>/ component",
                __FUNCTION__, name.c_str());
        return ::android::BAD_VALUE;
    }
    std::string typeVal = name.substr(typeSlashIdx + 1, instanceSlashIdx - typeSlashIdx - 1);

    if (instanceSlashIdx == name.size() - 1) {
        CAM_LOGE(ERROR_MSG_PREFIX
                "does not have an /<id> component",
                __FUNCTION__, name.c_str());
        return ::android::BAD_VALUE;
    }
    std::string idVal = name.substr(instanceSlashIdx + 1);

#undef ERROR_MSG_PREFIX

    *major = static_cast<uint16_t>(majorVal);
    *minor = static_cast<uint16_t>(minorVal);
    *type = typeVal;
    *id = idVal;

    return ::android::OK;
}

void Camera3DeviceHelper::getCameraCharacteristics(
        const std::string& cameraDeviceName, CameraMetadata& cameraMetadata)
{
    Return<::android::hardware::camera::common::V1_0::Status> status(
            ::android::hardware::camera::common::V1_0::Status::OK);

    // get camera characteristics
    CAM_LOGD("getCameraCharacteristics: camera device(%s)", cameraDeviceName.c_str());
    sp<::android::hardware::camera::device::V3_2::ICameraDevice> device3_x;
    Return<void> ret = mProvider->getCameraDeviceInterface_V3_x(
            cameraDeviceName, [&](auto status, const auto& device3) {
            ASSERT_EQ(::android::hardware::camera::common::V1_0::Status::OK, status)
                << "getCameraDeviceInterface_V3_x failed(" << toString(status) << ")";
            ASSERT_NE(device3, nullptr);
            device3_x = device3;
            });
    ASSERT_TRUE(ret.isOk());

    // get cahced cameraCharacteristics if exist
    auto search = mCameraCharacteristics.find(cameraDeviceName);
    if (search != mCameraCharacteristics.end())
    {
        cameraMetadata = search->second;
        return;
    }

    ret = device3_x->getCameraCharacteristics([&status, &cameraMetadata](
                ::android::hardware::camera::common::V1_0::Status s,
                ::android::hardware::camera::device::V3_2::CameraMetadata metadata) {
            status = s;
            if (s == ::android::hardware::camera::common::V1_0::Status::OK)
            {
                camera_metadata_t *buffer =
                reinterpret_cast<camera_metadata_t*>(metadata.data());
                size_t expectedSize = metadata.size();
                int res = validate_camera_metadata_structure(buffer, &expectedSize);
                if (res == 0)
                {
                    cameraMetadata = metadata;
                    return;
                }

                CAM_LOGE("validate camera metadata structure failed(%d)", res);
                status = ::android::hardware::camera::common::V1_0::Status::INTERNAL_ERROR;
            }
    });
    ASSERT_TRUE(ret.isOk())
        << "Transaction error getting camera characteristics for device: "
        << ret.description().c_str();
    ASSERT_EQ(::android::hardware::camera::common::V1_0::Status::OK, status)
        << "Unable to get camera characteristics for device: " << toString(status);

    // store cameraCharacteristics for later use
    bool isOK = mCameraCharacteristics.emplace(cameraDeviceName, cameraMetadata).second;
    ASSERT_TRUE(isOK) << cameraDeviceName << ": emplace camera characteristics failed";
};

// ------------------------------------------------------------------------

/**
 * A testing base class that provids testing boilerplate.
 */
class TestBase : public ::testing::Test
{
protected:
    // You can do set-up work for each test here.
    TestBase() = default;

    // You can do clean-up work that doesn't throw exceptions here.
    ~TestBase() = default;

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods:

    // Code here will be called immediately after the constructor (right
    // before each test).
    virtual void SetUp();

    // Code here will be called immediately after each test (right
    // before the destructor).
    virtual void TearDown();

    inline const std::string getTestSuiteName() const;
    inline const std::string getTestName() const;
    inline const char* getTestSuiteNameCString() const;
    inline const char* getTestNameCString() const;
};

/**
 * This test fixture is for testing class IISPModule.
 */
class ISPModuleTest : public TestBase
{
protected:
    ISPModuleTest();
    ~ISPModuleTest();

    // Objects declared here can be used by all test cases of ISPModuleTest.
    sp<IISPModule> mISPModule;

    // A helper API to register a death recipient to get a notification
    // when a service dies.
    void linkToDeath(sp<HalInterface>& halInterface);

    // A helper API to undo a registered death recipient.
    void unlinkToDeath(const sp<HalInterface>& halInterface);

    // A helper API to open ISP device and
    // reduce boilerplate (e.g. EXPECT_* after each API call)
    // The instance of IDevice is valid after this helper API call
    void openISPDevice(sp<IDevice>& device);

    // A helper API to close ISP device and
    // reduce boilerplate (e.g. EXPECT_* after each API call)
    // The instance of IDevice is invalid after this helper API call
    void closeISPDevice(sp<IDevice>& device);

private:
    void init();
};

class ISPDeviceTest : public ISPModuleTest
{
protected:
    ISPDeviceTest();
    ~ISPDeviceTest();

    // instance of IDevice; valid after ctor
    sp<IDevice> mDevice;

    // instance of IDevice valid after ctor
    sp<HalInterface> mHalInterface;

    // camera device name
    //NOTE: The test app sets camera id, which is written to the JSON setting file
    //      and is a small incrementing integer for "internal" device types,
    //      with 0 being the main back-facing camera and 1 being
    //      the main front-facing camera, if they exist.
    //
    //      Then we transfers the camera id into a fully qualified
    //      camera device name that is used for manipulating
    //      the V3_x::ICameraDevice for static metadata retrieval.
    std::string mCameraDeviceName;

    // mandatory test case setting properties
    struct ValueNamePair
    {
        std::string name;
        Json::Value value;

        ValueNamePair() = default;
        explicit ValueNamePair(const std::string& _name, const Json::Value& _value)
            : name(_name) , value(_value) {}

        ValueNamePair& operator=(const ValueNamePair& other) {
            if (this != &other)
            {
                name = other.name;
                value = value;
            }
            return *this;
        }

        ValueNamePair& operator=(ValueNamePair&& other) {
            if (this != &other)
            {
                name = std::move(other.name);
                value = other.value;
                other.value = Json::Value::null;
            }
            return *this;
        }
    };

    ValueNamePair mTestSuite;
    ValueNamePair mTest;

    ValueNamePair mRootDirectory;
    ValueNamePair mTotalRequests;
    size_t getTotalRequests() const { return mTotalRequests.value.asUInt(); }

    std::vector<File> mInputFiles;
    std::vector<File> mOutputFiles;

    // This definition must be aligned with isphal's types.hal
    struct Metadata
    {
        std::string name;
        uint32_t tag;
        //
        union {
        uint8_t u8[32];
        int32_t i32[8];
        float   f32[8];
        int64_t i64[4];
        } data;
        uint32_t padding; // align 8 bytes

        enum class Type : int32_t {
            BYTE,
            INT32,
            INT64,
            FLOAT
        };
        Type type;
    };

    struct StreamConfiguration
    {
        // TODO: define struct Stream to substitute uint64_t
        std::vector<uint64_t> inputStreams;
        std::vector<uint64_t> outputStreams;
        // key: Metadata::tag, value: Metadata
        std::unordered_map<int32_t, Metadata> metadata;
    } mStreamConfiguration;

    struct RequestConfiguration
    {
        uint32_t requestNumber;
        // key: Metadata::tag, value: Metadata
        std::unordered_map<uint32_t, Metadata> metadata;
        // TODO: define struct Request to substitute uint64_t
        std::vector<uint64_t> inputBuffers;
        std::vector<uint64_t> outputBuffers;
    };
    // NOTE: the number of requestion configurations MUST be the same as that
    //       of total requests
    std::vector<RequestConfiguration> mRequestConfigurations;

    // All loaded settings from a JSON file are verified and reconstruct here
    // NOTE: If any of the property name are removed/added/changed,
    //       the implementation of this API MUST BE also updated.
    void jsonVerifier(bool printerEnabled = true);

    enum class Direction : int {
        IN,
        OUT
    };
    void formatPrinter(Direction direction, isphal::V1_0::Status status,
            const std::vector<ISPFormat>& supportedFormats) const;

    void featureTagInfoPrinter(const std::vector<ISPFeatureTagInfo>& tagInfos) const;

    // Load files from directory
    void loadFromFile(hidl_handle &rawHandle, const File& file, const PixelFormat format);
    void configIsp();
    void buildIspRequests(std::vector<ISPRequest> &requests);
    void RAW2YUV(std::vector<ISPBuffer> &inIspBuffers, std::vector<ISPBuffer> &outIspBuffers, std::vector<File> &inputFiles);
    void YUV2JPEG(std::vector<ISPBuffer> &inIspBuffers, std::vector<File> &inputFiles);
private:
    void init();
};

#ifdef ENABLE_JPEG_BUFFER_TEST
/**
 * This test fixture is for testing class JPEGBufferTest.
 */
class JPEGBufferTest : public TestBase
{
};
#endif // ENABLE_JPEG_BUFFER_TEST

void TestBase::SetUp()
{
    // Gets information about the currently running test.
    // Do NOT delete the returned object - it's managed by the UnitTest class.
    CAM_LOGD("Begin test: %s.%s", getTestSuiteNameCString(), getTestNameCString());
}

void TestBase::TearDown()
{
    // Gets information about the currently running test.
    // Do NOT delete the returned object - it's managed by the UnitTest class.
    CAM_LOGD("End test: %s.%s", getTestSuiteNameCString(), getTestNameCString());
}

const std::string TestBase::getTestSuiteName() const
{
    return ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name();
}

const std::string TestBase::getTestName() const
{
    return ::testing::UnitTest::GetInstance()->current_test_info()->name();
}

const char* TestBase::getTestSuiteNameCString() const
{
    return getTestSuiteName().c_str();
}

const char* TestBase::getTestNameCString() const
{
    return getTestName().c_str();
}

ISPModuleTest::ISPModuleTest()
{
    init();
}

ISPModuleTest::~ISPModuleTest()
{
    // disconnect from ISPModule
    mISPModule.clear();
}

void ISPModuleTest::init()
{
    // connect to ISPModule
    mISPModule = IISPModule::getService("internal/0");
    ASSERT_TRUE(mISPModule) << "failed to get ISPModule interface";
}

void ISPModuleTest::linkToDeath(sp<HalInterface>& halInterface)
{
    halInterface = new HalInterface();
    Return<bool> linked = mISPModule->linkToDeath(
            halInterface->getHidlDeathRecipient(),
            reinterpret_cast<uint64_t>(halInterface.get()) /*cookie*/);

    ASSERT_TRUE(linked.isOk())
        << "Transaction error in linking to ISPModule death: "
        << linked.description().c_str();
    ASSERT_TRUE(linked) << "Unable to link to ISPModule death notifications";
}

void ISPModuleTest::unlinkToDeath(const sp<HalInterface>& halInterface)
{
    Return<bool> unlinked =
        mISPModule->unlinkToDeath(halInterface->getHidlDeathRecipient());

    ASSERT_TRUE(unlinked.isOk())
        << "Transaction error in unlinking from ISPModule death: "
        << unlinked.description().c_str();
    ASSERT_TRUE(unlinked) << "Unable to unlink from ISPModule death notifications";
}

void ISPModuleTest::openISPDevice(sp<IDevice>& device)
{
    auto status(isphal::V1_0::Status::OK);
    Return<void> ret = mISPModule->openISPDevice([&status, &device](
                isphal::V1_0::Status _status, const sp<IDevice>& _device) {
                status = _status;
                device = _device;
                return Void();
            });

    ASSERT_TRUE(ret.isOk())
        << "Transaction error in openISPDevice from ISPModule: "
        << ret.description().c_str();
    ASSERT_EQ(status, isphal::V1_0::Status::OK)
        << "openISPDevice failed(" << toString(status) << ")";
    ASSERT_TRUE(device) << "invalid ISP device";
}

void ISPModuleTest::closeISPDevice(sp<IDevice>& device)
{
    Return<isphal::V1_0::Status> ret = device->closeISPDevice();
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in closeISPDevice from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "closeISPDevice failed";

    device.clear();
}

ISPDeviceTest::ISPDeviceTest()
{
    init();
}

ISPDeviceTest::~ISPDeviceTest()
{
    // close ISP device
    closeISPDevice(mDevice);

    // unlink from death
    unlinkToDeath(mHalInterface);
}

void ISPDeviceTest::init()
{
    // link to death
    linkToDeath(mHalInterface);

    // open ISP device
    openISPDevice(mDevice);
}

void ISPDeviceTest::loadFromFile(hidl_handle &rawHandle, const File& file, const PixelFormat format)
{
    buffer_handle_t importedBuffer = rawHandle.getNativeHandle();
    MapperHelper::getInstance().importBuffer(importedBuffer);
    int fence = -1;
    std::ifstream ifs(file.getFullyQualifiedName(),
                (std::ios::in | std::ios::binary));

    // lock buffer for writing
    if(format == PixelFormat::YCBCR_420_888) {
        const IMapper::Rect region { 0, 0, static_cast<int32_t>(file.width),
            static_cast<int32_t>(file.height) };

        YCbCrLayout layout;
        MapperHelper::getInstance().lockYCbCr(importedBuffer,
                static_cast<uint64_t>(BufferUsage::CPU_WRITE_OFTEN),
                region, fence, layout);
        CAM_LOGD("lock layout: %s", toString(layout).c_str());

        ASSERT_TRUE(ifs.is_open()) << strerror(errno) << ": " <<
            file.getFullyQualifiedName();

        auto yData = static_cast<uint8_t*>(layout.y);
        auto cbData = static_cast<uint8_t*>(layout.cb);
        auto crData = static_cast<uint8_t*>(layout.cr);

        char value;
        for (uint32_t y = 0; y < file.height; y++)
        {
            for (uint32_t x = 0; x < file.width; x++)
            {
                if (ifs.get(value))
                    yData[layout.yStride * y + x] = value;
                else
                    ASSERT_TRUE(false) << "cannot read more data from file "
                        << file.getFullyQualifiedName();
            }
        }

        uint32_t uvHeight = file.height >> 1;
        uint32_t uvWidth = file.width >> 1;

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
                        ASSERT_TRUE(false) << "cannot read more data from file "
                            << file.getFullyQualifiedName();

                    if (ifs.get(value))
                        cbData[layout.cStride * y + step] = value;
                    else
                        ASSERT_TRUE(false) << "cannot read more data from file "
                            << file.getFullyQualifiedName();
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
                        ASSERT_TRUE(false) << "cannot read more data from file "
                            << file.getFullyQualifiedName();
                }
            }

            for (uint32_t y = 0; y < uvHeight; y++) {
                for (uint32_t x = 0, step = 0; x < uvWidth; x++, step+=layout.chromaStep) {
                    if (ifs.get(value))
                        crData[layout.cStride * y + step] = value;
                    else
                        ASSERT_TRUE(false) << "cannot read more data from file "
                            << file.getFullyQualifiedName();
                }
            }
        }
    }
    else if(format == PixelFormat::BLOB || format == PixelFormat::RAW16 || format == PixelFormat::IMPLEMENTATION_DEFINED) {
        // lock buffer for writing
        //const IMapper::Rect region { 0, 0, static_cast<int32_t>(image.width),
        //    static_cast<int32_t>(image.height) };
        const IMapper::Rect region { 0, 0, static_cast<int32_t>(file.size),
            1 };
        int fence = -1;
        void* data = nullptr;

        MapperHelper::getInstance().lock(importedBuffer,
                (BufferUsage::CPU_WRITE_OFTEN | BufferUsage::CPU_READ_OFTEN),
                region, fence, data);
        ASSERT_NE(data, nullptr) << "lock image failed";

        // Open file
        ASSERT_TRUE(ifs.is_open()) << strerror(errno) << ": " <<
            file.getFullyQualifiedName();

        // Load raw image from file
        char value = 0;
        uint8_t *addr = static_cast<uint8_t *>(data);
        while (!ifs.eof()) {
            ifs.get(value);
            *addr = value;
            addr++;
        }
    }
    else {
        ifs.close();
        ASSERT_TRUE(false) << "format not support loading";
        return;
    }

    ifs.close();

    MapperHelper::getInstance().unlock(importedBuffer, fence);
    if (fence >= 0)
        close(fence);

    MapperHelper::getInstance().freeBuffer(importedBuffer);

}

void ISPDeviceTest::jsonVerifier(bool printerEnabled)
{
    const std::string& jsonFileName(JSONContainer::kTestCaseSettingFile);

    // test suite and test names
    const std::string testSuiteName(getTestSuiteName());
    const std::string testName(getTestName());

    const Json::Value& testSuite =
        JSONContainer::getInstance().getTestCaseSetting()[testSuiteName.c_str()];
    ASSERT_FALSE(testSuite.isNull())
        << "Test suite " << testSuiteName << " does not exist in " << jsonFileName;
    const Json::Value& test = testSuite[testName.c_str()];
    ASSERT_FALSE(test.isNull())
        << "Test " << testName << " does not exist in " << jsonFileName;

    mTestSuite = ValueNamePair(testSuiteName, testSuite);
    mTest = ValueNamePair(testName, test);

    // camera device name
    const std::string cameraIdName("cameraId");

    const Json::Value& cameraId = mTest.value[cameraIdName.c_str()];
    ASSERT_FALSE(cameraId.isNull())
        << cameraIdName << " does not exist in " << jsonFileName;

    Camera3DeviceHelper::getInstance().getCameraDeviceName(
            cameraId.asString(), mCameraDeviceName);

    CAM_LOGD("%s(%s) cameraDeviceName(%s)",
            cameraIdName.c_str(), cameraId.asString().c_str(),
            mCameraDeviceName.c_str());

    // root directory and total requests
    const std::string rootDirectoryName("rootDirectory");
    const std::string totalRequestsName("totalRequests");

    const Json::Value& rootDirectory = mTest.value[rootDirectoryName.c_str()];
    ASSERT_FALSE(rootDirectory.isNull())
        << rootDirectoryName << " does not exist in " << jsonFileName;
    const Json::Value& totalRequests = mTest.value[totalRequestsName.c_str()];
    ASSERT_FALSE(totalRequests.isNull())
        << totalRequestsName << " does not exist in " << jsonFileName;

    mRootDirectory = ValueNamePair(rootDirectoryName, rootDirectory);
    mTotalRequests = ValueNamePair(totalRequestsName, totalRequests);

    CAM_LOGD("%s(%s) %s(%u)",
            rootDirectoryName.c_str(), mRootDirectory.value.asString().c_str(),
            totalRequestsName.c_str(), mTotalRequests.value.asUInt());

    // input/output files
    const std::string inputFilesName("inputFiles");
    const std::string outputFilesName("outputFiles");

    const Json::Value& inputFiles = mTest.value[inputFilesName.c_str()];
    ASSERT_FALSE(inputFiles.isNull())
        << inputFilesName << " does not exist in " << jsonFileName;
    Json::Value& outputFiles = mTest.value[outputFilesName.c_str()];
    ASSERT_FALSE(outputFiles.isNull())
        << outputFilesName << " does not exist in " << jsonFileName;

    // construct file structure
    auto constructFileList = [this](const Json::Value& files , std::vector<File>& fileList)
    {
        for (Json::ArrayIndex i = 0; i < files.size(); i++)
        {
            const auto& inputFile(files[i]);
            std::vector<Json::UInt> rowStrides {
                inputFile["rowStrides"][0].asUInt(),
                    inputFile["rowStrides"][1].asUInt(),
                    inputFile["rowStrides"][2].asUInt()};
            File file(mRootDirectory.value.asString(), inputFile["name"].asString(),
                    inputFile["width"].asUInt(), inputFile["height"].asUInt(),
                    inputFile["format"].asInt(), std::move(rowStrides),
                    inputFile["size"].asUInt());

            // if the file size in JSON file is not set,
            // we set it queried from the file system
            if (file.size == 0)
            {
                const std::string fqn(file.getFullyQualifiedName());
                const auto fileSize = getFileSize(fqn);
                if (fileSize != -1)
                {
                    CAM_LOGW("[%s] fize size in JSON is 0, we set it queried " \
                            "from the file system(%ld)",
                            fqn.c_str(), fileSize);
                    file.size = fileSize;
                }
            }


            fileList.push_back(std::move(file));
        }
    };
    constructFileList(inputFiles, mInputFiles);
    constructFileList(outputFiles, mOutputFiles);
    ASSERT_TRUE(mInputFiles.size() > 0);
    ASSERT_TRUE(mOutputFiles.size() > 0);

    // NOTE: JPEG's width and height is the same as the input ones
    for (auto&& file : mOutputFiles)
    {
        if (file.isJPEG()) {

            // get the first input image's width and height
            file.width = mInputFiles[0].width;
            file.height = mInputFiles[0].height;
            file.format = static_cast<int32_t>(PixelFormat::BLOB);

            if (mCameraDeviceName.empty())
            {
                CAM_LOGW("camera device name is empty");
                continue;
            }

            // get the maximum size in bytes for the compressed JPEG buffer
            // TODO: check if need to align Camera3Device::getJpegBufferSiz() in
            // frameworks/av/services/camera/libcameraservice/device3/Camera3Device.cpp
            CameraMetadata cameraMetadata;
            Camera3DeviceHelper::getInstance().getCameraCharacteristics(
                    mCameraDeviceName, cameraMetadata);

            camera_metadata_t *staticMeta = clone_camera_metadata(
                    reinterpret_cast<const camera_metadata_t*>(cameraMetadata.data()));
            ASSERT_NE(nullptr, staticMeta) << "clone camera metadata failed";
            camera_metadata_ro_entry entry;
            auto status = find_camera_metadata_ro_entry(staticMeta,
                    ANDROID_JPEG_MAX_SIZE, &entry);
            ASSERT_TRUE((status == 0) && (entry.count > 0)) <<
                mCameraDeviceName << ": can't find maximum JPEG size in static metadata";
            // update jpeg's size
            file.size = entry.data.i32[0];
            free_camera_metadata(staticMeta);
        }
    }

    auto printFileList = [this](const std::vector<File>& files, Direction direction)
    {
        for (const auto& file : files)
            CAM_LOGD("testName(%s) %sFileName(%s) width(%u) height(%u) format(%s) "
                    "rowStride[ %d %d %d ] size(%u)",
                    mTest.name.c_str(),
                    direction == Direction::IN ? "input" : "output",
                    file.getFullyQualifiedName().c_str(),
                    file.width, file.height, toString(file.getPixelFormat()).c_str(),
                    file.rowStride[0], file.rowStride[1] ,file.rowStride[2],
                    file.size);
    };

    if (printerEnabled)
    {
        printFileList(mInputFiles, Direction::IN);
        printFileList(mOutputFiles, Direction::OUT);
    }

    // stream configuration
    const std::string streamConfigurationName("ispStreamConfiguration");
    const std::string inputStreamsName("inputStreams");
    const std::string outputStreamsName("outputStreams");
    const std::string metadataName("metadata");

    const Json::Value& streamConfiguration =
        mTest.value[streamConfigurationName.c_str()];
    ASSERT_FALSE(streamConfiguration.isNull())
        << streamConfigurationName << " does not exist in " << jsonFileName;
    const Json::Value& inputStreams =
        streamConfiguration[inputStreamsName.c_str()];
    ASSERT_FALSE(inputStreams.isNull())
        << inputStreamsName << " does not exist in " << jsonFileName;
    const Json::Value& outputStreams =
        streamConfiguration[outputStreamsName.c_str()];
    ASSERT_FALSE(outputStreams.isNull())
        << outputStreamsName << " does not exist in " << jsonFileName;
    const Json::Value& metadata =
        streamConfiguration[metadataName.c_str()];
    ASSERT_FALSE(metadata.isNull())
        << metadataName << " does not exist in " << jsonFileName;

    auto constructUINT64 = [](const Json::Value& values, std::vector<uint64_t>& list)
    {
        for (Json::ArrayIndex i = 0; i < values.size(); i++)
            list.push_back(values[i].asUInt64());
    };
    constructUINT64(inputStreams, mStreamConfiguration.inputStreams);
    constructUINT64(outputStreams, mStreamConfiguration.outputStreams);

    auto constructMeta = [](const Json::Value& values, std::unordered_map<uint32_t, Metadata> &metaMap)
    {
        CAM_LOGD("constructMeta size(%d)", values.size());
        for(auto && iter : gIspMetaMap) {
            const auto& tagString(values[iter.first.c_str()]);
            if(tagString.isNull()) {
                continue;
            }

            Metadata ispMeta;
            ispMeta.name = iter.first;
            ispMeta.tag = static_cast<uint32_t>(iter.second);
            // TODO: remove hard code here
            ispMeta.data.i32[0] = tagString["value"].asInt();
#define TYPE_MBOOL 11
#define TYPE_MUINT8 0
            int type = tagString["type"].asInt();
            if(type == TYPE_MBOOL)  //because Test APK can't distinguish byte & bool, so we need to use additional type for bool
            {
                type = TYPE_MUINT8;
            }
            ispMeta.type = static_cast<Metadata::Type>(type);

            CAM_LOGD("Tag name(%s, %p), value(%d), type(%d)", ispMeta.name.c_str(), ispMeta.tag
                                                            , ispMeta.data.i32[0], ispMeta.type);
            metaMap.emplace(static_cast<uint32_t>(iter.second), std::move(ispMeta));
        }
    };

    auto printStreamList = [](const std::vector<uint64_t>& streams, Direction direction)
    {
        std::stringstream ss;
        ss << (direction == Direction::IN ? "input" : "output") << " stream = {";
        for (const auto& stream : streams)
            ss << " " << stream;
        ss << " }";
        CAM_LOGD("%s", ss.str().c_str());
    };

    if (printerEnabled)
    {
        printStreamList(mStreamConfiguration.inputStreams, Direction::IN);
        printStreamList(mStreamConfiguration.outputStreams, Direction::OUT);
    }

    // TODO: construct mStreamConfiguration.metadata

    // request configuration
    const std::string requestConfigurationName("ispRequestConfiguration");
    const std::string requestNumberName("requestNumber");
    const std::string inputBuffersName("inputBuffers");
    const std::string outputBuffersName("outputBuffers");
    const std::string requestMetadataName("metadata");

    const Json::Value& requestConfiguration =
        mTest.value[requestConfigurationName.c_str()];
    ASSERT_FALSE(requestConfiguration.isNull())
        << requestConfigurationName << " does not exist in " << jsonFileName;

    for (Json::ArrayIndex i = 0; i < requestConfiguration.size(); i++)
    {
        const Json::Value& _config = requestConfiguration[i];
        ASSERT_FALSE(_config.isNull())
            << "[" << i << "]" << " requestConfiguration does not exist in " << jsonFileName;
        RequestConfiguration config;

        const Json::Value& requestNumber = _config[requestNumberName.c_str()];
        ASSERT_FALSE(requestNumber.isNull())
            << requestNumberName << " does not exist in " << jsonFileName;
        config.requestNumber = requestNumber.asUInt();

        // TODO: construct mRequestConfigurations.metadata

        const Json::Value& inputBuffers = _config[inputBuffersName.c_str()];
        ASSERT_FALSE(inputBuffers.isNull())
            << inputBuffersName << " does not exist in " << jsonFileName;
        constructUINT64(inputBuffers, config.inputBuffers);

        const Json::Value& outputBuffers = _config[outputBuffersName.c_str()];
        ASSERT_FALSE(outputBuffers.isNull())
            << outputBuffersName << " does not exist in " << jsonFileName;
        constructUINT64(outputBuffers, config.outputBuffers);

        const Json::Value& requestMetadata = _config[requestMetadataName.c_str()];
        ASSERT_FALSE(requestMetadata.isNull())
            << requestMetadataName << " does not exist in " << jsonFileName;

        constructMeta(requestMetadata, config.metadata);

        mRequestConfigurations.push_back(std::move(config));
    }

    // NOTE: the number of requestion configurations MUST be the same as that
    //       of total requests
    ASSERT_EQ(getTotalRequests(), mRequestConfigurations.size())
        << "the number of total requests is not the same " \
           "as that of request configurations";

    auto printRequestConfiguration = [](const RequestConfiguration& config)
    {
        CAM_LOGD("requestNumber(%d)", config.requestNumber);

        // TODO: print metadata

        std::stringstream ss;
        ss << "input buffer = {";
        for (const auto& buffer : config.inputBuffers)
            ss << " " << buffer;
        ss << " }";
        CAM_LOGD("%s", ss.str().c_str());

        ss.str(std::string());

        ss << "output buffer = {";
        for (const auto& buffer : config.outputBuffers)
            ss << " " << buffer;
        ss << " }";
        CAM_LOGD("%s", ss.str().c_str());
    };

    if (printerEnabled)
    {
        for (std::size_t i = 0; i < mRequestConfigurations.size(); i++)
        {
            CAM_LOGD("[requestConfiguration %zu]", i);
            printRequestConfiguration(mRequestConfigurations[i]);
        }
    }
}

void ISPDeviceTest::formatPrinter(Direction direction,
        isphal::V1_0::Status status,
        const std::vector<ISPFormat>& supportedFormats) const
{
    const std::string directionString(
            direction == Direction::IN ? "input" : "output");

    EXPECT_EQ(status, isphal::V1_0::Status::OK) << "get supported " << directionString
        << " format failed(" << toString(status) << ")";
    EXPECT_TRUE(supportedFormats.size() > 0)
        << "no supported " << directionString << " format";
    CAM_LOGD_IF(status == isphal::V1_0::Status::OK, "supported %s format = %s",
            directionString.c_str(), stringify<ISPFormat>(supportedFormats).c_str());
}

void ISPDeviceTest::featureTagInfoPrinter(
        const std::vector<ISPFeatureTagInfo>& tagInfos) const
{
    for (const auto& tagInfo : tagInfos)
        CAM_LOGD("%s", toString(tagInfo).c_str());
}

void ISPDeviceTest::configIsp()
{
    // reconstruct buffers and settings from the JSON file
    //jsonVerifier();

    // configure ISP device
    ISPStreamConfiguration hidlConfiguration;
    auto initEmptyMetadata = [](ISPMetadata& metadata)
    {
        std::vector<uint8_t> emptyMetadata;
        ASSERT_EQ(initial_isp_metadata_vector(emptyMetadata, 1), 0);
        metadata = emptyMetadata;
    };

    // TODO: fill metadata if necessary
    ISPMetadata configureParams;
    initEmptyMetadata(configureParams);

    auto constructISPStream = [](Direction direction,
            const std::vector<uint64_t>& streams, const std::vector<File>& files)
    {
        std::vector<ISPStream> ispStreams;

        for (size_t i = 0; i < streams.size(); i++)
        {
            const File& file(files[i]);
            constexpr uint64_t privRawUsage(GRALLOC1_CONSUMER_USAGE_CAMERA | GRALLOC1_PRODUCER_USAGE_CAMERA);
            // NOTE: the usage is referenced from Camera3Device::createStream() in
            // frameworks/av/services/camera/libcameraservice/device3/Camera3Device.cpp
            // and Stream::usage in
            // hardware/interfaces/camera/device/3.2/types.hal
            //
            // TODO: correct dataspace if necessary
            ISPStream stream {
                .id = streams[i],
                .size   = file.size,
                .stride = file.isJPEG() ?
                    hidl_vec<uint32_t>({ file.size, 0, 0 }) :
                    hidl_vec<uint32_t>(file.rowStride),
                .width  = file.width,
                .height = file.height,
                .format = { file.getPixelFormat(),
                    static_cast<DataspaceFlags>(
                            file.isJPEG() ? Dataspace::V0_JFIF : Dataspace::UNKNOWN) },
                .usage = direction == Direction::IN ?
                    GRALLOC1_CONSUMER_USAGE_NONE : kConsumerUsage,
                .transform = 0
            };

            if (file.getPixelFormat() == PixelFormat::IMPLEMENTATION_DEFINED) {
                CAM_LOGD("Need to set up priv usage(%p)", privRawUsage);
                stream.usage = privRawUsage;
            }

            ispStreams.push_back(std::move(stream));
        }

        return ispStreams;
    };

    hidlConfiguration.InputStreams = constructISPStream(Direction::IN,
            mStreamConfiguration.inputStreams, mInputFiles);
    hidlConfiguration.OutputStreams = constructISPStream(Direction::OUT,
            mStreamConfiguration.outputStreams, mOutputFiles);
    hidlConfiguration.ConfigureParams = configureParams;

    // NOTE: image and metadata are paired per input buffer
    ASSERT_TRUE((mInputFiles.size() > 0) && (mInputFiles.size() % 2 == 0))
        << "the number of input files must be greater than 0 and be a multiple of 2";
    ASSERT_TRUE(mOutputFiles.size() > 0)
        << "the number of output files must be greater than 0";

    CAM_LOGD("[Input Streams]");
    for (const auto& inputStream : hidlConfiguration.InputStreams)
        CAM_LOGD("%s", toString(inputStream).c_str());
    CAM_LOGD("[Output Streams]");
    for (const auto& outputStream : hidlConfiguration.OutputStreams)
        CAM_LOGD("%s", toString(outputStream).c_str());
    // TODO: print metadata

    // configure ISP device
    Return<isphal::V1_0::Status> ret = mDevice->configureISPDevice(
            hidlConfiguration, mHalInterface->getICallback());
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in configureISPDevice from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "configureISPDevice failed";

}

void ISPDeviceTest::buildIspRequests(std::vector<ISPRequest> &requests)
{
    auto initEmptyMetadata = [](ISPMetadata& metadata)
    {
        std::vector<uint8_t> emptyMetadata;
        ASSERT_EQ(initial_isp_metadata_vector(emptyMetadata, 1), 0);
        metadata = emptyMetadata;
    };

    const size_t totalRequests(getTotalRequests());
    for (size_t i = 0; i < totalRequests; i++)
    {
        const RequestConfiguration& requestConfiguration(mRequestConfigurations[i]);
        const uint32_t requestNumber(requestConfiguration.requestNumber);

        CAM_LOGD("construct ISP request %u", requestNumber);

        // construct metadata
        // TODO: fill metadata if necessary
        ISPMetadata settings;
        //initEmptyMetadata(settings);
        std::vector<uint8_t> emptyMetadata;
        ASSERT_EQ(initial_isp_metadata_vector(emptyMetadata, 1), 0);

        for(auto && itr : requestConfiguration.metadata) {
            // TODO: remove hard code
            const auto &meta = itr.second;
            auto err = add_isp_metadata_entry(emptyMetadata, meta.tag, (uint32_t)(meta.type), (void *)(&meta.data),
                           1 * sizeof(int), nullptr);
            if (err)
            {
                CAM_LOGW("tag[0x%X] is not support with err[%d], maybe data size is too large", meta.tag, err);
                continue;
            }

            mtkisp_metadata_entry_t *pentry = nullptr;

            // Dump metadata to check
            err = get_isp_metadata_entry(emptyMetadata, meta.tag, &pentry);

            if (err)
            {
                CAM_LOGW("Get entry fail", meta.tag, err);
                continue;
            }

            CAM_LOGD("Get entry data(%d)", pentry->data.i32[0]);
        }

        settings = emptyMetadata;

        // construct input buffers
        // NOTE: ISPDevice allows one or more input buffers per request
        HalInterface::FileBuffer inputBuffers;
        size_t inputImageIndex = 0;
        size_t inputImageSize = mInputFiles.size() >> 1;
        size_t inputMetadataIndex = mInputFiles.size() >> 1;
        for (const auto streamId : requestConfiguration.inputBuffers)
        {
            while ((inputImageIndex < inputImageSize) && (inputMetadataIndex < mInputFiles.size())) {
                // load image into hidl handle
                hidl_handle image_handle;
                const File& image(mInputFiles[inputImageIndex]);
                {
                    AllocatorHelper::getInstance().allocateGraphicBuffer(
                            image.width, image.height,
                            GRALLOC1_CONSUMER_USAGE_CAMERA | GRALLOC1_PRODUCER_USAGE_CAMERA, //kGralloc0Usage,
                            image.getPixelFormat(), &image_handle);
                    CAM_LOGD("input image [%zu](%s)", inputImageIndex,
                            toString(image_handle).c_str());

                    loadFromFile(image_handle, image, image.getPixelFormat());
                }

                // load metadata into hidl handle
                hidl_handle metadata_handle;
                const File& metadata(mInputFiles[inputMetadataIndex]);
                {
                    // TODO: set metadata format to blob when parsing from JSON settings
                    uint32_t rowStride = 0;
                    AllocatorHelper::getInstance().allocateGraphicBuffer(
                            metadata.size, 1u,
                            kGralloc0Usage,
                            PixelFormat::BLOB, &metadata_handle, &rowStride);
                    CAM_LOGD("input metadata [%zu](%s)", inputMetadataIndex,
                            toString(metadata_handle).c_str());

                    loadFromFile(metadata_handle, metadata, PixelFormat::BLOB);

                    // lock again for metadata header validation
                    {
                        buffer_handle_t importedBuffer = metadata_handle.getNativeHandle();
                        MapperHelper::getInstance().importBuffer(importedBuffer);
                        // lock buffer for writing
                        const IMapper::Rect region { 0, 0, static_cast<int32_t>(metadata.size), 1u };
                        int fence = -1;
                        void* data = nullptr;

                        MapperHelper::getInstance().lock(importedBuffer,
                                static_cast<uint64_t>(BufferUsage::CPU_READ_OFTEN),
                                region, fence, data);

                        ASSERT_NE(data, nullptr);

                        // show metadata header
                        uint8_t *addr = static_cast<uint8_t *>(data);
                        CAM_LOGD("metadata size(%d)", metadata.size);
                        CAM_LOGD("metadata header begin");
                        for (int i = 0; i < 4; i++)
                        {
                            CAM_LOGD("0x%X 0x%X 0x%X 0x%X",
                                    *addr, *(addr+1), *(addr+2), *(addr+3));
                            addr = addr + 4;
                        }
                        CAM_LOGD("metadata header end");

                        MapperHelper::getInstance().unlock(importedBuffer, fence);
                        if (fence >= 0)
                            close(fence);

                        MapperHelper::getInstance().freeBuffer(importedBuffer);
                    }
                }

                // construct ISP buffer
                ISPBuffer buffer
                {
                    .id = streamId,
                    .buffer = image_handle,
                    .tuningData = metadata_handle
                };
                inputBuffers.first.push_back(image);
                inputBuffers.second.push_back(std::move(buffer));

                inputImageIndex++;
                inputMetadataIndex++;
            }
        }

        // construct output buffers
        HalInterface::FileBuffer outputBuffers;
        size_t outputImageIndex = 0;
        size_t outputMetadataIndex = mOutputFiles.size() >> 1;
        for (const auto streamId : requestConfiguration.outputBuffers)
        {
            const File& image(mOutputFiles[outputImageIndex]);
            hidl_handle image_handle;
            // NOTE: jpeg is a blob format
            AllocatorHelper::getInstance().allocateGraphicBuffer(
                    image.isJPEG() ? image.size : image.width,
                    image.isJPEG() ? 1 : image.height,
                    kGralloc0Usage,
                    image.getPixelFormat(), &image_handle);
            CAM_LOGD("output image [%zu](%s)", outputImageIndex,
                    toString(image_handle).c_str());

            // load metadata into hidl handle
            hidl_handle metadata_handle;
            const File& metadata(mOutputFiles[outputMetadataIndex]);
            {
                // TODO: set metadata format to blob when parsing from JSON settings
                uint32_t rowStride = 0;
                AllocatorHelper::getInstance().allocateGraphicBuffer(
                        metadata.size, 1u,
                        kGralloc0Usage,
                        PixelFormat::BLOB, &metadata_handle, &rowStride);
                CAM_LOGD("Output metadata [%zu](%s), size(%d)", outputMetadataIndex,
                        toString(metadata_handle).c_str(), metadata.size);
            }

            // construct ISP buffer
            ISPBuffer buffer
            {
                .id = streamId,
                .buffer = image_handle,
                .tuningData = metadata_handle // tuning data is no use for output buffer
            };
            outputBuffers.first.push_back(image);
            outputBuffers.second.push_back(std::move(buffer));

            outputImageIndex++;
        }

        // construct ISP request
        ISPRequest request
        {
            .reqNumber = requestNumber,
            .settings = settings,
            .InputBuffers = inputBuffers.second,
            .OutputBuffers = outputBuffers.second
        };
        requests.push_back(std::move(request));

        // record capture requests for callback lookup
        mHalInterface->recordISPRequest(requestNumber,
                std::move(inputBuffers), std::move(outputBuffers));
    }
}

void ISPDeviceTest::YUV2JPEG(std::vector<ISPBuffer> &inIspBuffers, std::vector<File> &inputFiles)
{
    CAM_LOGD("Start to YUV2JPEG");
    // Prepare file information
    std::vector<File> outputFiles;

    // NOTE: JPEG's width and height is the same as the input ones
    for (auto && file : inputFiles)
    {
        // get the first input image's width and height
        uint32_t width = file.width;
        uint32_t height = file.height;
        int32_t format = static_cast<int32_t>(PixelFormat::BLOB);
        int32_t size = 0;
        std::string fileName = "result/result_01.jpg";

        std::vector<Json::UInt> rowStrides {0, 0, 0};

        if (mCameraDeviceName.empty())
        {
            CAM_LOGW("camera device name is empty");
            continue;
        }

        // get the maximum size in bytes for the compressed JPEG buffer
        // TODO: check if need to align Camera3Device::getJpegBufferSiz() in
        // frameworks/av/services/camera/libcameraservice/device3/Camera3Device.cpp
        CameraMetadata cameraMetadata;
        Camera3DeviceHelper::getInstance().getCameraCharacteristics(
                mCameraDeviceName, cameraMetadata);

        camera_metadata_t *staticMeta = clone_camera_metadata(
                reinterpret_cast<const camera_metadata_t*>(cameraMetadata.data()));
        ASSERT_NE(nullptr, staticMeta) << "clone camera metadata failed";
        camera_metadata_ro_entry entry;
        auto status = find_camera_metadata_ro_entry(staticMeta,
                ANDROID_JPEG_MAX_SIZE, &entry);
        ASSERT_TRUE((status == 0) && (entry.count > 0)) <<
            mCameraDeviceName << ": can't find maximum JPEG size in static metadata";
        // update jpeg's size
        size = entry.data.i32[0];
        free_camera_metadata(staticMeta);

        File jpgFile(mRootDirectory.value.asString(), fileName,
                        width, height, format, std::move(rowStrides), size);
        outputFiles.push_back(jpgFile);
    }

    // instance of IDevice; valid after ctor
    sp<IDevice> device = nullptr;
    openISPDevice(device);

    // configure ISP device
    ISPStreamConfiguration hidlConfiguration;
    auto initEmptyMetadata = [](ISPMetadata& metadata)
    {
        std::vector<uint8_t> emptyMetadata;
        ASSERT_EQ(initial_isp_metadata_vector(emptyMetadata, 1), 0);
#if 0
        ////////////////////////////////////////////////////
        //test: set crop
        uint32_t metaType = 1;
        int rectVec[4] = {1280,1440,640,480};

        auto err = add_isp_metadata_entry(emptyMetadata, MTK_SCALER_CROP_REGION, metaType, (void *)(rectVec),4 * sizeof(int), nullptr);
        if (err)
        {
            CAM_LOGW("MTK_SCALER_CROP_REGION set fail! ret(%d)", err);
        }
        CAM_LOGD("set crop (x(%d) y(%d) w(%d) h(%d)",rectVec[0],rectVec[1],rectVec[2],rectVec[3]);
#endif
        ////////////////////////////////////////////////////

        metadata = emptyMetadata;
    };

    // TODO: fill metadata if necessary
    ISPMetadata configureParams;
    initEmptyMetadata(configureParams);

    auto constructISPStream = [](Direction direction,
            const std::vector<uint64_t>& streams, const std::vector<File>& files)
    {
        std::vector<ISPStream> ispStreams;

        for (size_t i = 0; i < streams.size(); i++)
        {
            const File& file(files[i]);
            // NOTE: the usage is referenced from Camera3Device::createStream() in
            // frameworks/av/services/camera/libcameraservice/device3/Camera3Device.cpp
            // and Stream::usage in
            // hardware/interfaces/camera/device/3.2/types.hal
            //
            // TODO: correct dataspace if necessary
            ISPStream stream {
                .id = streams[i],
                .size   = file.size,
                .stride = file.isJPEG() ?
                    hidl_vec<uint32_t>({ file.size, 0, 0 }) :
                    hidl_vec<uint32_t>(file.rowStride),
                .width  = file.width,
                .height = file.height,
                .format = { file.getPixelFormat(),
                    static_cast<DataspaceFlags>(
                            file.isJPEG() ? Dataspace::V0_JFIF : Dataspace::UNKNOWN) },
                .usage = direction == Direction::IN ?
                    GRALLOC1_CONSUMER_USAGE_NONE : kConsumerUsage,
                .transform = 0
            };
            ispStreams.push_back(std::move(stream));
        }

        return ispStreams;
    };

    hidlConfiguration.InputStreams = constructISPStream(Direction::IN,
            mStreamConfiguration.inputStreams, inputFiles);
    hidlConfiguration.OutputStreams = constructISPStream(Direction::OUT,
            mStreamConfiguration.outputStreams, outputFiles);
    hidlConfiguration.ConfigureParams = configureParams;

    // NOTE: image and metadata are paired per input buffer
    ASSERT_TRUE((inputFiles.size() > 0) && (inputFiles.size() % 2 == 0))
        << "the number of input files must be greater than 0 and be a multiple of 2";
    ASSERT_TRUE(outputFiles.size() > 0)
        << "the number of output files must be greater than 0";

    CAM_LOGD("[Input Streams]");
    for (const auto& inputStream : hidlConfiguration.InputStreams)
        CAM_LOGD("%s", toString(inputStream).c_str());
    CAM_LOGD("[Output Streams]");
    for (const auto& outputStream : hidlConfiguration.OutputStreams)
        CAM_LOGD("%s", toString(outputStream).c_str());
    // TODO: print metadata

    // configure ISP device
    Return<isphal::V1_0::Status> ret = device->configureISPDevice(
            hidlConfiguration, mHalInterface->getICallback());
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in configureISPDevice from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "configureISPDevice failed";

    // process image requests
    std::vector<ISPRequest> requests;
    const size_t totalRequests(getTotalRequests());
    for (size_t i = 0; i < totalRequests; i++)
    {
        const RequestConfiguration& requestConfiguration(mRequestConfigurations[i]);
        uint32_t requestNumber = requestConfiguration.requestNumber + 1;

        CAM_LOGD("construct ISP request %u", requestNumber);

        // construct metadata
        // TODO: fill metadata if necessary
        ISPMetadata settings;
        initEmptyMetadata(settings);

        // construct input buffers
        // NOTE: ISPDevice allows one or more input buffers per request
        HalInterface::FileBuffer inputBuffers;
        for (const auto streamId : requestConfiguration.inputBuffers)
        {
            inIspBuffers[0].id = streamId;
            inputBuffers.first.push_back(inputFiles[0]);
            inputBuffers.second.push_back(std::move(inIspBuffers[0]));
        }

        // construct output buffers
        HalInterface::FileBuffer outputBuffers;
        size_t outputImageIndex = 0;
        for (const auto streamId : requestConfiguration.outputBuffers)
        {
            const File& image(outputFiles[outputImageIndex]);
            hidl_handle image_handle;
            // NOTE: jpeg is a blob format
            AllocatorHelper::getInstance().allocateGraphicBuffer(
                    image.isJPEG() ? image.size : image.width,
                    image.isJPEG() ? 1 : image.height,
                    kGralloc0Usage,
                    image.getPixelFormat(), &image_handle);
            CAM_LOGD("output image [%zu](%s)", outputImageIndex,
                    toString(image_handle).c_str());

            // construct ISP buffer
            ISPBuffer buffer
            {
                .id = streamId,
                .buffer = image_handle,
                .tuningData = hidl_handle() // tuning data is no use for output buffer
            };
            outputBuffers.first.push_back(image);
            outputBuffers.second.push_back(std::move(buffer));

            outputImageIndex++;
        }

        // construct ISP request
        ISPRequest request
        {
            .reqNumber = requestNumber,
            .settings = settings,
            .InputBuffers = inputBuffers.second,
            .OutputBuffers = outputBuffers.second
        };
        requests.push_back(std::move(request));

        // record capture requests for callback lookup
        mHalInterface->recordISPRequest(requestNumber,
                std::move(inputBuffers), std::move(outputBuffers));
    }

    ret = device->processImgRequest(requests);
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in processImgRequest from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "processImgRequest failed";

    // wait result (tiemout is 1000 milliseconds by default)
    const int timeout = 5000;
    ASSERT_EQ(::android::OK, mHalInterface->waitResult(timeout))
        << "did not receive the result for " << timeout << " milliseconds";

    // close ISP device
    closeISPDevice(device);

    CAM_LOGD("End YUV2JPEG");
}

void ISPDeviceTest::RAW2YUV(std::vector<ISPBuffer> &inIspBuffers, std::vector<ISPBuffer> &outIspBuffers, std::vector<File> &inputFiles)
{
    CAM_LOGD("Start to RAW2YUV");
    // Prepare file information
    std::vector<File> outputFiles;

    // NOTE: JPEG's width and height is the same as the input ones
    for (auto && file : inputFiles)
    {
        // get the first input image's width and height
        uint32_t width = file.width;
        uint32_t height = file.height;
        int32_t format = static_cast<int32_t>(PixelFormat::YCBCR_420_888);
        int32_t size = width * height * 1.5;
        std::string fileName = "result/result_01.yuv";
        std::vector<Json::UInt> rowStrides {0, 0, 0};

        File yuvFile(mRootDirectory.value.asString(), fileName,
                        width, height, format, std::move(rowStrides), size);
        outputFiles.push_back(yuvFile);
    }

    // instance of IDevice; valid after ctor
    sp<IDevice> device = nullptr;
    openISPDevice(device);

    // configure ISP device
    ISPStreamConfiguration hidlConfiguration;
    auto initEmptyMetadata = [](ISPMetadata& metadata)
    {
        std::vector<uint8_t> emptyMetadata;
        ASSERT_EQ(initial_isp_metadata_vector(emptyMetadata, 1), 0);
        metadata = emptyMetadata;
    };

    // TODO: fill metadata if necessary
    ISPMetadata configureParams;
    initEmptyMetadata(configureParams);

    auto constructISPStream = [](Direction direction,
            const std::vector<uint64_t>& streams, const std::vector<File>& files)
    {
        std::vector<ISPStream> ispStreams;

        for (size_t i = 0; i < streams.size(); i++)
        {
            const File& file(files[i]);
            ISPStream stream {
                .id = streams[i],
                .size   = file.size,
                .stride = file.isJPEG() ?
                    hidl_vec<uint32_t>({ file.size, 0, 0 }) :
                    hidl_vec<uint32_t>(file.rowStride),
                .width  = file.width,
                .height = file.height,
                .format = { file.getPixelFormat(),
                    static_cast<DataspaceFlags>(
                            file.isJPEG() ? Dataspace::V0_JFIF : Dataspace::UNKNOWN) },
                .usage = direction == Direction::IN ?
                    GRALLOC1_CONSUMER_USAGE_NONE : kConsumerUsage,
                .transform = 0
            };
            ispStreams.push_back(std::move(stream));
        }

        return ispStreams;
    };

    hidlConfiguration.InputStreams = constructISPStream(Direction::IN,
            mStreamConfiguration.inputStreams, inputFiles);
    hidlConfiguration.OutputStreams = constructISPStream(Direction::OUT,
            mStreamConfiguration.outputStreams, outputFiles);
    hidlConfiguration.ConfigureParams = configureParams;

    // NOTE: image and metadata are paired per input buffer
    ASSERT_TRUE((inputFiles.size() > 0) && (inputFiles.size() % 2 == 0))
        << "the number of input files must be greater than 0 and be a multiple of 2";
    ASSERT_TRUE(outputFiles.size() > 0)
        << "the number of output files must be greater than 0";

    CAM_LOGD("[Input Streams]");
    for (const auto& inputStream : hidlConfiguration.InputStreams)
        CAM_LOGD("%s", toString(inputStream).c_str());
    CAM_LOGD("[Output Streams]");
    for (const auto& outputStream : hidlConfiguration.OutputStreams)
        CAM_LOGD("%s", toString(outputStream).c_str());
    // TODO: print metadata

    // configure ISP device
    Return<isphal::V1_0::Status> ret = device->configureISPDevice(
            hidlConfiguration, mHalInterface->getICallback());
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in configureISPDevice from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "configureISPDevice failed";

    // process image requests
    std::vector<ISPRequest> requests;
    const size_t totalRequests(getTotalRequests());
    for (size_t i = 0; i < totalRequests; i++)
    {
        const RequestConfiguration& requestConfiguration(mRequestConfigurations[i]);
        uint32_t requestNumber = requestConfiguration.requestNumber + 1;

        CAM_LOGD("construct ISP request %u", requestNumber);

        // construct metadata
        // TODO: fill metadata if necessary
        ISPMetadata settings;
        initEmptyMetadata(settings);

        // construct input buffers
        // NOTE: ISPDevice allows one or more input buffers per request
        HalInterface::FileBuffer inputBuffers;
        for (const auto streamId : requestConfiguration.inputBuffers)
        {
            inIspBuffers[0].id = streamId;
            inputBuffers.first.push_back(inputFiles[0]);
            inputBuffers.second.push_back(inIspBuffers[0]);
        }

        // construct output buffers
        HalInterface::FileBuffer outputBuffers;
        size_t outputImageIndex = 0;
        size_t outputMetadataIndex = outputFiles.size() >> 1;
        for (const auto streamId : requestConfiguration.outputBuffers)
        {
            const File& image(outputFiles[outputImageIndex]);
            hidl_handle image_handle;
            // NOTE: jpeg is a blob format
            AllocatorHelper::getInstance().allocateGraphicBuffer(
                    image.isJPEG() ? image.size : image.width,
                    image.isJPEG() ? 1 : image.height,
                    kGralloc0Usage,
                    image.getPixelFormat(), &image_handle);
            CAM_LOGD("output image [%zu](%s)", outputImageIndex,
                    toString(image_handle).c_str());

            // load metadata into hidl handle
            hidl_handle metadata_handle;
            const File& metadata(mOutputFiles[outputMetadataIndex]);
            {
                // TODO: set metadata format to blob when parsing from JSON settings
                uint32_t rowStride = 0;
                AllocatorHelper::getInstance().allocateGraphicBuffer(
                        metadata.size, 1u,
                        kGralloc0Usage,
                        PixelFormat::BLOB, &metadata_handle, &rowStride);
                CAM_LOGD("Output metadata [%zu](%s), size(%d)", outputMetadataIndex,
                        toString(metadata_handle).c_str(), metadata.size);
            }

            // construct ISP buffer
            ISPBuffer buffer
            {
                .id = streamId,
                .buffer = image_handle,
                .tuningData = metadata_handle // tuning data is no use for output buffer
            };
            outIspBuffers.push_back(buffer); // Need one copy for outputbuffer
            outputBuffers.first.push_back(image);
            outputBuffers.second.push_back(std::move(buffer));

            outputImageIndex++;
            outputMetadataIndex++;
        }

        // construct ISP request
        ISPRequest request
        {
            .reqNumber = requestNumber,
            .settings = settings,
            .InputBuffers = inputBuffers.second,
            .OutputBuffers = outputBuffers.second
        };
        requests.push_back(std::move(request));

        // record capture requests for callback lookup
        mHalInterface->recordISPRequest(requestNumber,
                std::move(inputBuffers), std::move(outputBuffers));
    }

    ret = device->processImgRequest(requests);
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in processImgRequest from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "processImgRequest failed";

    // wait result (tiemout is 1000 milliseconds by default)
    const int timeout = 1000;
    ASSERT_EQ(::android::OK, mHalInterface->waitResult(timeout))
        << "did not receive the result for " << timeout << " milliseconds";

    // close ISP device
    closeISPDevice(device);

    CAM_LOGD("End RAW2YUV");
}

// ------------------------------------------------------------------------

TEST_F(ISPModuleTest, linkAndUnlinkHWBinderDeath)
{
    // link to death
    sp<HalInterface> halInterface;
    linkToDeath(halInterface);

    // unlink the registered death recipient
    unlinkToDeath(halInterface);
}

TEST_F(ISPModuleTest, openAndCloseISPDevice)
{
    // open ISP device
    sp<IDevice> device;
    openISPDevice(device);

    // close ISP device
    closeISPDevice(device);
}

TEST_F(ISPDeviceTest, getISPSupportedFormats)
{
    isphal::V1_0::Status status(isphal::V1_0::Status::OK);
    std::vector<ISPFormat> supportedFormats;
    auto getSupportedFormat = [&](Direction direction) {
        if (direction == Direction::IN)
        {
            Return<void> ret =
                mDevice->getISPSupportInputFormat([&status, &supportedFormats](
                            isphal::V1_0::Status _status,
                            const hidl_vec<ISPFormat>& _supportedFormats) {
                        status = _status;
                        supportedFormats = _supportedFormats;
                        });
            ASSERT_TRUE(ret.isOk())
                << "Transaction error in getISPSupportInputFormat from ISPDevice: "
                << ret.description().c_str();
            ASSERT_EQ(status, isphal::V1_0::Status::OK) << "getISPSupportInputFormat failed("
                << toString(status) << ")";

            return;
        }

        Return<void> ret =
            mDevice->getISPSupportOutputFormat([&status, &supportedFormats](
                        isphal::V1_0::Status _status,
                        const hidl_vec<ISPFormat>& _supportedFormats) {
                    status = _status;
                    supportedFormats = _supportedFormats;
                    });
        ASSERT_TRUE(ret.isOk())
            << "Transaction error in getISPSupportOutputFormat from ISPDevice: "
            << ret.description().c_str();
        ASSERT_EQ(status, isphal::V1_0::Status::OK) << "getISPSupportOutputFormat failed("
            << toString(status) << ")";
    };

    // get supported input format
    getSupportedFormat(Direction::IN);
    ISPDeviceTest::formatPrinter(Direction::IN, status, supportedFormats);

    // get supported output format
    getSupportedFormat(Direction::OUT);
    ISPDeviceTest::formatPrinter(Direction::OUT, status, supportedFormats);
}

TEST_F(ISPDeviceTest, getISPSupportFeatures)
{
    isphal::V1_0::Status status(isphal::V1_0::Status::OK);

    // get supported features
    std::vector<ISPFeatureTagInfo> tagInfos;
    Return<void> ret = mDevice->getISPSupportFeatures([&status, &tagInfos](
                isphal::V1_0::Status _status,
                const hidl_vec<ISPFeatureTagInfo>& _tagInfos){
            status = _status;
            tagInfos = _tagInfos;
            });
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in getISPSupportFeatures from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(status, isphal::V1_0::Status::OK) << "getISPSupportFeatures failed("
        << toString(status) << ")";

    ISPDeviceTest::featureTagInfoPrinter(tagInfos);
}

TEST_F(ISPDeviceTest, singleRequestYUV2JPEG)
{
    // reconstruct buffers and settings from the JSON file
    jsonVerifier();

    // configure ISP device
    ISPStreamConfiguration hidlConfiguration;
    auto initEmptyMetadata = [](ISPMetadata& metadata)
    {
        std::vector<uint8_t> emptyMetadata;
        ASSERT_EQ(initial_isp_metadata_vector(emptyMetadata, 1), 0);
        metadata = emptyMetadata;
    };

    // TODO: fill metadata if necessary
    ISPMetadata configureParams;
    initEmptyMetadata(configureParams);

    auto constructISPStream = [](Direction direction,
            const std::vector<uint64_t>& streams, const std::vector<File>& files)
    {
        std::vector<ISPStream> ispStreams;

        for (size_t i = 0; i < streams.size(); i++)
        {
            const File& file(files[i]);
            // NOTE: the usage is referenced from Camera3Device::createStream() in
            // frameworks/av/services/camera/libcameraservice/device3/Camera3Device.cpp
            // and Stream::usage in
            // hardware/interfaces/camera/device/3.2/types.hal
            //
            // TODO: correct dataspace if necessary
            ISPStream stream {
                .id = streams[i],
                .size   = file.size,
                .stride = file.isJPEG() ?
                    hidl_vec<uint32_t>({ file.size, 0, 0 }) :
                    hidl_vec<uint32_t>(file.rowStride),
                .width  = file.width,
                .height = file.height,
                .format = { file.getPixelFormat(),
                    static_cast<DataspaceFlags>(
                            file.isJPEG() ? Dataspace::V0_JFIF : Dataspace::UNKNOWN) },
                .usage = direction == Direction::IN ?
                    GRALLOC1_CONSUMER_USAGE_NONE : kConsumerUsage,
                .transform = 0
            };
            ispStreams.push_back(std::move(stream));
        }

        return ispStreams;
    };

    hidlConfiguration.InputStreams = constructISPStream(Direction::IN,
            mStreamConfiguration.inputStreams, mInputFiles);
    hidlConfiguration.OutputStreams = constructISPStream(Direction::OUT,
            mStreamConfiguration.outputStreams, mOutputFiles);
    hidlConfiguration.ConfigureParams = configureParams;

    // NOTE: image and metadata are paired per input buffer
    ASSERT_TRUE((mInputFiles.size() > 0) && (mInputFiles.size() % 2 == 0))
        << "the number of input files must be greater than 0 and be a multiple of 2";
    ASSERT_TRUE(mOutputFiles.size() > 0)
        << "the number of output files must be greater than 0";

    CAM_LOGD("[Input Streams]");
    for (const auto& inputStream : hidlConfiguration.InputStreams)
        CAM_LOGD("%s", toString(inputStream).c_str());
    CAM_LOGD("[Output Streams]");
    for (const auto& outputStream : hidlConfiguration.OutputStreams)
        CAM_LOGD("%s", toString(outputStream).c_str());
    // TODO: print metadata

    // configure ISP device
    Return<isphal::V1_0::Status> ret = mDevice->configureISPDevice(
            hidlConfiguration, mHalInterface->getICallback());
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in configureISPDevice from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "configureISPDevice failed";

    // process image requests
    std::vector<ISPRequest> requests;
    const size_t totalRequests(getTotalRequests());
    for (size_t i = 0; i < totalRequests; i++)
    {
        const RequestConfiguration& requestConfiguration(mRequestConfigurations[i]);
        const uint32_t requestNumber(requestConfiguration.requestNumber);

        CAM_LOGD("construct ISP request %u", requestNumber);

        // construct metadata
        // TODO: fill metadata if necessary
        ISPMetadata settings;
        initEmptyMetadata(settings);

        // construct input buffers
        // NOTE: ISPDevice allows one or more input buffers per request
        HalInterface::FileBuffer inputBuffers;
        size_t inputImageIndex = 0;
        size_t inputMetadataIndex = mInputFiles.size() >> 1;
        for (const auto streamId : requestConfiguration.inputBuffers)
        {
            // load image into hidl handle
            hidl_handle image_handle;
            const File& image(mInputFiles[inputImageIndex]);
            {
                AllocatorHelper::getInstance().allocateGraphicBuffer(
                        image.width, image.height,
                        kGralloc0Usage,
                        image.getPixelFormat(), &image_handle);
                CAM_LOGD("input image [%zu](%s)", inputImageIndex,
                        toString(image_handle).c_str());

                buffer_handle_t importedBuffer = image_handle.getNativeHandle();
                MapperHelper::getInstance().importBuffer(importedBuffer);

                // lock buffer for writing
                const IMapper::Rect region { 0, 0, static_cast<int32_t>(image.width),
                    static_cast<int32_t>(image.height) };
                int fence = -1;

                YCbCrLayout layout;
                MapperHelper::getInstance().lockYCbCr(importedBuffer,
                        static_cast<uint64_t>(BufferUsage::CPU_WRITE_OFTEN),
                        region, fence, layout);
                CAM_LOGD("lock layout: %s", toString(layout).c_str());

                std::ifstream ifs(image.getFullyQualifiedName(),
                        (std::ios::in | std::ios::binary));
                ASSERT_TRUE(ifs.is_open()) << strerror(errno) << ": " <<
                    image.getFullyQualifiedName();

                auto yData = static_cast<uint8_t*>(layout.y);
                auto cbData = static_cast<uint8_t*>(layout.cb);
                auto crData = static_cast<uint8_t*>(layout.cr);

                char value;
                for (uint32_t y = 0; y < image.height; y++)
                {
                    for (uint32_t x = 0; x < image.width; x++)
                    {
                        if (ifs.get(value))
                            yData[layout.yStride * y + x] = value;
                        else
                            ASSERT_TRUE(false) << "cannot read more data from file "
                                << image.getFullyQualifiedName();
                    }
                }

                uint32_t uvHeight = image.height >> 1;
                uint32_t uvWidth = image.width >> 1;

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
                                ASSERT_TRUE(false) << "cannot read more data from file "
                                    << image.getFullyQualifiedName();

                            if (ifs.get(value))
                                cbData[layout.cStride * y + step] = value;
                            else
                                ASSERT_TRUE(false) << "cannot read more data from file "
                                    << image.getFullyQualifiedName();
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
                                ASSERT_TRUE(false) << "cannot read more data from file "
                                    << image.getFullyQualifiedName();
                        }
                    }

                    for (uint32_t y = 0; y < uvHeight; y++) {
                        for (uint32_t x = 0, step = 0; x < uvWidth; x++, step+=layout.chromaStep) {
                            if (ifs.get(value))
                                crData[layout.cStride * y + step] = value;
                            else
                                ASSERT_TRUE(false) << "cannot read more data from file "
                                    << image.getFullyQualifiedName();
                        }
                    }
                }

                ifs.close();

                MapperHelper::getInstance().unlock(importedBuffer, fence);
                if (fence >= 0)
                    close(fence);

                MapperHelper::getInstance().freeBuffer(importedBuffer);

                // TODO: lock again for bit-true validation
            }

            // load metadata into hidl handle
            hidl_handle metadata_handle;
            const File& metadata(mInputFiles[inputMetadataIndex]);
            {
                // TODO: set metadata format to blob when parsing from JSON settings
                uint32_t rowStride = 0;
                AllocatorHelper::getInstance().allocateGraphicBuffer(
                        metadata.size, 1u,
                        kGralloc0Usage,
                        PixelFormat::BLOB, &metadata_handle, &rowStride);
                CAM_LOGD("input metadata [%zu](%s)", inputMetadataIndex,
                        toString(metadata_handle).c_str());

                buffer_handle_t importedBuffer = metadata_handle.getNativeHandle();
                MapperHelper::getInstance().importBuffer(importedBuffer);

                // lock buffer for writing
                const IMapper::Rect region { 0, 0, static_cast<int32_t>(metadata.size), 1u };
                int fence = -1;
                void* addr = nullptr;

                MapperHelper::getInstance().lock(importedBuffer,
                        static_cast<uint64_t>(BufferUsage::CPU_WRITE_OFTEN),
                        region, fence, addr);
                ASSERT_NE(addr, nullptr) << "lock metadata failed";
                CAM_LOGD("lock addr(0x%X)", reinterpret_cast<intptr_t>(addr));

                std::ifstream ifs(metadata.getFullyQualifiedName(),
                        (std::ios::in | std::ios::binary));
                ASSERT_TRUE(ifs.is_open()) << strerror(errno) << ": " <<
                    metadata.getFullyQualifiedName();

                auto data = static_cast<uint8_t*>(addr);
                char value;
                int count = 0;
                while (ifs.get(value))
                {
                    CAM_LOGD_IF(count < 16, "%d: 0x%X", count, value);
                    data[count] = value;
                    count++;
                }
                CAM_LOGD("metadata: write %d bytes", count);
                ifs.close();

                MapperHelper::getInstance().unlock(importedBuffer, fence);
                if (fence >= 0)
                    close(fence);

                MapperHelper::getInstance().freeBuffer(importedBuffer);

                // lock again for metadata header validation
                {
                    buffer_handle_t importedBuffer = metadata_handle.getNativeHandle();
                    MapperHelper::getInstance().importBuffer(importedBuffer);
                    // lock buffer for writing
                    const IMapper::Rect region { 0, 0, static_cast<int32_t>(metadata.size), 1u };
                    int fence = -1;
                    void* data = nullptr;

                    MapperHelper::getInstance().lock(importedBuffer,
                            static_cast<uint64_t>(BufferUsage::CPU_READ_OFTEN),
                            region, fence, data);

                    ASSERT_NE(data, nullptr);

                    // show metadata header
                    uint8_t *addr = static_cast<uint8_t *>(data);
                    CAM_LOGD("metadata size(%d)", metadata.size);
                    CAM_LOGD("metadata header begin");
                    for (int i = 0; i < 4; i++)
                    {
                        CAM_LOGD("0x%X 0x%X 0x%X 0x%X",
                                *addr, *(addr+1), *(addr+2), *(addr+3));
                        addr = addr + 4;
                    }
                    CAM_LOGD("metadata header end");

                    MapperHelper::getInstance().unlock(importedBuffer, fence);
                    if (fence >= 0)
                        close(fence);

                    MapperHelper::getInstance().freeBuffer(importedBuffer);
                }
            }

            // construct ISP buffer
            ISPBuffer buffer
            {
                .id = streamId,
                .buffer = image_handle,
                .tuningData = metadata_handle
            };
            inputBuffers.first.push_back(image);
            inputBuffers.second.push_back(std::move(buffer));

            inputImageIndex++;
            inputMetadataIndex++;
        }

        // construct output buffers
        HalInterface::FileBuffer outputBuffers;
        size_t outputImageIndex = 0;
        for (const auto streamId : requestConfiguration.outputBuffers)
        {
            const File& image(mOutputFiles[outputImageIndex]);
            hidl_handle image_handle;
            // NOTE: jpeg is a blob format
            AllocatorHelper::getInstance().allocateGraphicBuffer(
                    image.isJPEG() ? image.size : image.width,
                    image.isJPEG() ? 1 : image.height,
                    kGralloc0Usage,
                    image.getPixelFormat(), &image_handle);
            CAM_LOGD("output image [%zu](%s)", outputImageIndex,
                    toString(image_handle).c_str());

            // construct ISP buffer
            ISPBuffer buffer
            {
                .id = streamId,
                .buffer = image_handle,
                .tuningData = hidl_handle() // tuning data is no use for output buffer
            };
            outputBuffers.first.push_back(image);
            outputBuffers.second.push_back(std::move(buffer));

            outputImageIndex++;
        }

        // construct ISP request
        ISPRequest request
        {
            .reqNumber = requestNumber,
            .settings = settings,
            .InputBuffers = inputBuffers.second,
            .OutputBuffers = outputBuffers.second
        };
        requests.push_back(std::move(request));

        // record capture requests for callback lookup
        mHalInterface->recordISPRequest(requestNumber,
                std::move(inputBuffers), std::move(outputBuffers));
    }

    ret = mDevice->processImgRequest(requests);
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in processImgRequest from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "processImgRequest failed";

    // wait result (tiemout is 1000 milliseconds by default)
    const int timeout = 1000;
    ASSERT_EQ(::android::OK, mHalInterface->waitResult(timeout))
        << "did not receive the result for " << timeout << " milliseconds";
}

TEST_F(ISPDeviceTest, singleRequestYUV2YUV)
{
    // reconstruct buffers and settings from the JSON file
    jsonVerifier();

    const File& imageInput(mInputFiles[0]);
    for(auto && image : mOutputFiles) {
        int isFHDCropping = property_get_int32("vendor.debug.camera.ISPY2Y.FHDCrop", 0);
        unsigned int FHD_WIDTH  = 1920;
        unsigned int FHD_HEIGHT = 1080;
        switch(__builtin_expect(isFHDCropping,0)){
            case 1:
                CAM_LOGD("adopt FHD size to test yuv cropping");
                image.width = FHD_WIDTH;
                image.height = FHD_HEIGHT;
                image.size = FHD_WIDTH*FHD_HEIGHT*1.5;
                break;

            case 0:
                CAM_LOGD("adopt original yuv size");
                image.width = imageInput.width;
                image.height = imageInput.height;
                image.size = imageInput.size;
                break;
            default:
                break;
        }
        image.format = static_cast<int32_t>(PixelFormat::YCBCR_420_888);
    }

    // configure ISP device
    ISPStreamConfiguration hidlConfiguration;
    auto initEmptyMetadata = [](ISPMetadata& metadata)
    {
        std::vector<uint8_t> emptyMetadata;
        ASSERT_EQ(initial_isp_metadata_vector(emptyMetadata, 1), 0);
        metadata = emptyMetadata;
    };

    // TODO: fill metadata if necessary
    ISPMetadata configureParams;
    initEmptyMetadata(configureParams);

    auto constructISPStream = [](Direction direction,
            const std::vector<uint64_t>& streams, const std::vector<File>& files)
    {
        std::vector<ISPStream> ispStreams;

        for (size_t i = 0; i < streams.size(); i++)
        {
            const File& file(files[i]);
            // NOTE: the usage is referenced from Camera3Device::createStream() in
            // frameworks/av/services/camera/libcameraservice/device3/Camera3Device.cpp
            // and Stream::usage in
            // hardware/interfaces/camera/device/3.2/types.hal
            //
            // TODO: correct dataspace if necessary
            ISPStream stream {
                .id = streams[i],
                .size   = file.size,
                .stride = file.isJPEG() ?
                    hidl_vec<uint32_t>({ file.size, 0, 0 }) :
                    hidl_vec<uint32_t>(file.rowStride),
                .width  = file.width,
                .height = file.height,
                .format = { file.getPixelFormat(),
                    static_cast<DataspaceFlags>(
                            file.isJPEG() ? Dataspace::V0_JFIF : Dataspace::UNKNOWN) },
                .usage = direction == Direction::IN ?
                    GRALLOC1_CONSUMER_USAGE_NONE : kConsumerUsage,
                .transform = 0
            };
            ispStreams.push_back(std::move(stream));
        }

        return ispStreams;
    };

    hidlConfiguration.InputStreams = constructISPStream(Direction::IN,
            mStreamConfiguration.inputStreams, mInputFiles);
    hidlConfiguration.OutputStreams = constructISPStream(Direction::OUT,
            mStreamConfiguration.outputStreams, mOutputFiles);
    hidlConfiguration.ConfigureParams = configureParams;

    // NOTE: image and metadata are paired per input buffer
    ASSERT_TRUE((mInputFiles.size() > 0) && (mInputFiles.size() % 2 == 0))
        << "the number of input files must be greater than 0 and be a multiple of 2";
    ASSERT_TRUE(mOutputFiles.size() > 0)
        << "the number of output files must be greater than 0";

    CAM_LOGD("[Input Streams]");
    for (const auto& inputStream : hidlConfiguration.InputStreams)
        CAM_LOGD("%s", toString(inputStream).c_str());
    CAM_LOGD("[Output Streams]");
    for (const auto& outputStream : hidlConfiguration.OutputStreams)
        CAM_LOGD("%s", toString(outputStream).c_str());
    // TODO: print metadata

    // configure ISP device
    Return<isphal::V1_0::Status> ret = mDevice->configureISPDevice(
            hidlConfiguration, mHalInterface->getICallback());
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in configureISPDevice from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "configureISPDevice failed";

    // process image requests
    std::vector<ISPRequest> requests;
    const size_t totalRequests(getTotalRequests());
    for (size_t i = 0; i < totalRequests; i++)
    {
        const RequestConfiguration& requestConfiguration(mRequestConfigurations[i]);
        const uint32_t requestNumber(requestConfiguration.requestNumber);

        CAM_LOGD("construct ISP request %u", requestNumber);

        // construct metadata
        // TODO: fill metadata if necessary
        ISPMetadata settings;
        initEmptyMetadata(settings);

        // construct input buffers
        // NOTE: ISPDevice allows one or more input buffers per request
        HalInterface::FileBuffer inputBuffers;
        size_t inputImageIndex = 0;
        size_t inputMetadataIndex = mInputFiles.size() >> 1;
        for (const auto streamId : requestConfiguration.inputBuffers)
        {
            // load image into hidl handle
            hidl_handle image_handle;
            const File& image(mInputFiles[inputImageIndex]);
            {
                AllocatorHelper::getInstance().allocateGraphicBuffer(
                        image.width, image.height,
                        kGralloc0Usage,
                        image.getPixelFormat(), &image_handle);
                CAM_LOGD("input image [%zu](%s)", inputImageIndex,
                        toString(image_handle).c_str());

                buffer_handle_t importedBuffer = image_handle.getNativeHandle();
                MapperHelper::getInstance().importBuffer(importedBuffer);

                // lock buffer for writing
                const IMapper::Rect region { 0, 0, static_cast<int32_t>(image.width),
                    static_cast<int32_t>(image.height) };
                int fence = -1;

                YCbCrLayout layout;
                MapperHelper::getInstance().lockYCbCr(importedBuffer,
                        static_cast<uint64_t>(BufferUsage::CPU_WRITE_OFTEN),
                        region, fence, layout);
                CAM_LOGD("lock layout: %s", toString(layout).c_str());

                std::ifstream ifs(image.getFullyQualifiedName(),
                        (std::ios::in | std::ios::binary));
                ASSERT_TRUE(ifs.is_open()) << strerror(errno) << ": " <<
                    image.getFullyQualifiedName();

                auto yData = static_cast<uint8_t*>(layout.y);
                auto cbData = static_cast<uint8_t*>(layout.cb);
                auto crData = static_cast<uint8_t*>(layout.cr);

                char value;
                for (uint32_t y = 0; y < image.height; y++)
                {
                    for (uint32_t x = 0; x < image.width; x++)
                    {
                        if (ifs.get(value))
                            yData[layout.yStride * y + x] = value;
                        else
                            ASSERT_TRUE(false) << "cannot read more data from file "
                                << image.getFullyQualifiedName();
                    }
                }

                uint32_t uvHeight = image.height >> 1;
                uint32_t uvWidth = image.width >> 1;

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
                                ASSERT_TRUE(false) << "cannot read more data from file "
                                    << image.getFullyQualifiedName();

                            if (ifs.get(value))
                                cbData[layout.cStride * y + step] = value;
                            else
                                ASSERT_TRUE(false) << "cannot read more data from file "
                                    << image.getFullyQualifiedName();
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
                                ASSERT_TRUE(false) << "cannot read more data from file "
                                    << image.getFullyQualifiedName();
                        }
                    }

                    for (uint32_t y = 0; y < uvHeight; y++) {
                        for (uint32_t x = 0, step = 0; x < uvWidth; x++, step+=layout.chromaStep) {
                            if (ifs.get(value))
                                crData[layout.cStride * y + step] = value;
                            else
                                ASSERT_TRUE(false) << "cannot read more data from file "
                                    << image.getFullyQualifiedName();
                        }
                    }
                }

                ifs.close();

                MapperHelper::getInstance().unlock(importedBuffer, fence);
                if (fence >= 0)
                    close(fence);

                MapperHelper::getInstance().freeBuffer(importedBuffer);

                // TODO: lock again for bit-true validation
            }

            // load metadata into hidl handle
            hidl_handle metadata_handle;
            const File& metadata(mInputFiles[inputMetadataIndex]);
            {
                // TODO: set metadata format to blob when parsing from JSON settings
                uint32_t rowStride = 0;
                AllocatorHelper::getInstance().allocateGraphicBuffer(
                        metadata.size, 1u,
                        kGralloc0Usage,
                        PixelFormat::BLOB, &metadata_handle, &rowStride);
                CAM_LOGD("input metadata [%zu](%s)", inputMetadataIndex,
                        toString(metadata_handle).c_str());

                buffer_handle_t importedBuffer = metadata_handle.getNativeHandle();
                MapperHelper::getInstance().importBuffer(importedBuffer);

                // lock buffer for writing
                const IMapper::Rect region { 0, 0, static_cast<int32_t>(metadata.size), 1u };
                int fence = -1;
                void* addr = nullptr;

                MapperHelper::getInstance().lock(importedBuffer,
                        static_cast<uint64_t>(BufferUsage::CPU_WRITE_OFTEN),
                        region, fence, addr);
                ASSERT_NE(addr, nullptr) << "lock metadata failed";
                CAM_LOGD("lock addr(0x%X)", reinterpret_cast<intptr_t>(addr));

                std::ifstream ifs(metadata.getFullyQualifiedName(),
                        (std::ios::in | std::ios::binary));
                ASSERT_TRUE(ifs.is_open()) << strerror(errno) << ": " <<
                    metadata.getFullyQualifiedName();

                auto data = static_cast<uint8_t*>(addr);
                char value;
                int count = 0;
                while (ifs.get(value))
                {
                    CAM_LOGD_IF(count < 16, "%d: 0x%X", count, value);
                    data[count] = value;
                    count++;
                }
                CAM_LOGD("metadata: write %d bytes", count);
                ifs.close();

                MapperHelper::getInstance().unlock(importedBuffer, fence);
                if (fence >= 0)
                    close(fence);

                MapperHelper::getInstance().freeBuffer(importedBuffer);

                // lock again for metadata header validation
                {
                    buffer_handle_t importedBuffer = metadata_handle.getNativeHandle();
                    MapperHelper::getInstance().importBuffer(importedBuffer);
                    // lock buffer for writing
                    const IMapper::Rect region { 0, 0, static_cast<int32_t>(metadata.size), 1u };
                    int fence = -1;
                    void* data = nullptr;

                    MapperHelper::getInstance().lock(importedBuffer,
                            static_cast<uint64_t>(BufferUsage::CPU_READ_OFTEN),
                            region, fence, data);

                    ASSERT_NE(data, nullptr);

                    // show metadata header
                    uint8_t *addr = static_cast<uint8_t *>(data);
                    CAM_LOGD("metadata size(%d)", metadata.size);
                    CAM_LOGD("metadata header begin");
                    for (int i = 0; i < 4; i++)
                    {
                        CAM_LOGD("0x%X 0x%X 0x%X 0x%X",
                                *addr, *(addr+1), *(addr+2), *(addr+3));
                        addr = addr + 4;
                    }
                    CAM_LOGD("metadata header end");

                    MapperHelper::getInstance().unlock(importedBuffer, fence);
                    if (fence >= 0)
                        close(fence);

                    MapperHelper::getInstance().freeBuffer(importedBuffer);
                }
            }

            // construct ISP buffer
            ISPBuffer buffer
            {
                .id = streamId,
                .buffer = image_handle,
                .tuningData = metadata_handle
            };
            inputBuffers.first.push_back(image);
            inputBuffers.second.push_back(std::move(buffer));

            inputImageIndex++;
            inputMetadataIndex++;
        }

        // construct output buffers
        HalInterface::FileBuffer outputBuffers;
        size_t outputImageIndex = 0;
        size_t outputMetadataIndex = mOutputFiles.size() >> 1;
        for (const auto streamId : requestConfiguration.outputBuffers)
        {
            const File& image(mOutputFiles[outputImageIndex]);
            hidl_handle image_handle;
            // NOTE: jpeg is a blob format
            AllocatorHelper::getInstance().allocateGraphicBuffer(
                    image.isJPEG() ? image.size : image.width,
                    image.isJPEG() ? 1 : image.height,
                    kGralloc0Usage,
                    image.getPixelFormat(), &image_handle);
            CAM_LOGD("output image [%zu](%s)", outputImageIndex,
                    toString(image_handle).c_str());

            // load metadata into hidl handle
            hidl_handle metadata_handle;
            const File& metadata(mOutputFiles[outputMetadataIndex]);
            {
                // TODO: set metadata format to blob when parsing from JSON settings
                uint32_t rowStride = 0;
                AllocatorHelper::getInstance().allocateGraphicBuffer(
                        metadata.size, 1u,
                        kGralloc0Usage,
                        PixelFormat::BLOB, &metadata_handle, &rowStride);
                CAM_LOGD("Output metadata [%zu](%s), size(%d)", outputMetadataIndex,
                        toString(metadata_handle).c_str(), metadata.size);
            }

            // construct ISP buffer
            ISPBuffer buffer
            {
                .id = streamId,
                .buffer = image_handle,
                .tuningData = metadata_handle
            };
            outputBuffers.first.push_back(image);
            outputBuffers.second.push_back(std::move(buffer));

            outputImageIndex++;
        }

        // construct ISP request
        ISPRequest request
        {
            .reqNumber = requestNumber,
            .settings = settings,
            .InputBuffers = inputBuffers.second,
            .OutputBuffers = outputBuffers.second
        };
        requests.push_back(std::move(request));

        // record capture requests for callback lookup
        mHalInterface->recordISPRequest(requestNumber,
                std::move(inputBuffers), std::move(outputBuffers));
    }

    ret = mDevice->processImgRequest(requests);
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in processImgRequest from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "processImgRequest failed";

    // wait result 3000 ms to make sure files written successfully
    const int timeout = 3000;
    ASSERT_EQ(::android::OK, mHalInterface->waitResult(timeout))
        << "did not receive the result for " << timeout << " milliseconds";

    // Encode yuv to jpeg
    std::vector<ISPBuffer> yuvInputBuffers;
    for(auto && ispBuf : requests[0].OutputBuffers) {
        yuvInputBuffers.push_back(ispBuf);
    }

    YUV2JPEG(yuvInputBuffers, mOutputFiles);
}

TEST_F(ISPDeviceTest, singleRequestRAW2YUV)
{
    // reconstruct buffers and settings from the JSON file
    jsonVerifier();

    const File& imageInput(mInputFiles[0]);

    for(auto && image : mOutputFiles) {
        //File& image(mOutputFiles[outputImageIndex]);
        image.width = imageInput.width;
        image.height = imageInput.height;
        image.size = imageInput.width * imageInput.height * 1.5;
        image.format = static_cast<int32_t>(PixelFormat::YCBCR_420_888);
    }
    //
    configIsp();
    //
    std::vector<ISPRequest> requests;
    buildIspRequests(requests);
    //
    Return<isphal::V1_0::Status> ret = mDevice->processImgRequest(requests);
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in processImgRequest from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "processImgRequest failed";

    // wait result (tiemout is 1000 milliseconds by default)
    const int timeout = 1000;
    ASSERT_EQ(::android::OK, mHalInterface->waitResult(timeout))
        << "did not receive the result for " << timeout << " milliseconds";

    // Encode yuv to jpeg
    std::vector<ISPBuffer> yuvInputBuffers;
    for(auto && ispBuf : requests[0].OutputBuffers) {
        yuvInputBuffers.push_back(ispBuf);
    }

    YUV2JPEG(yuvInputBuffers, mOutputFiles);
}

TEST_F(ISPDeviceTest, singleZSL_RAW2YUV)
{
    // reconstruct buffers and settings from the JSON file
    jsonVerifier();

    const File& imageInput(mInputFiles[0]);

    for(auto && image : mOutputFiles) {
        //File& image(mOutputFiles[outputImageIndex]);
        image.width = imageInput.width;
        image.height = imageInput.height;
        image.size = imageInput.width * imageInput.height * 1.5;
        image.format = static_cast<int32_t>(PixelFormat::YCBCR_420_888);
    }
    //
    configIsp();
    //
    std::vector<ISPRequest> requests;
    buildIspRequests(requests);
    //
    Return<isphal::V1_0::Status> ret = mDevice->processImgRequest(requests);
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in processImgRequest from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "processImgRequest failed";

    // wait result (tiemout is 1000 milliseconds by default)
    const int timeout = 1000;
    ASSERT_EQ(::android::OK, mHalInterface->waitResult(timeout))
        << "did not receive the result for " << timeout << " milliseconds";

    // Encode yuv to jpeg
    std::vector<ISPBuffer> yuvInputBuffers;
    for(auto && ispBuf : requests[0].OutputBuffers) {
        yuvInputBuffers.push_back(ispBuf);
    }

    YUV2JPEG(yuvInputBuffers, mOutputFiles);
}

TEST_F(ISPDeviceTest, singleRequestMFNR)
{
    jsonVerifier();

    // Setup output yuv information, we query it from input
    const File& imageInput(mInputFiles[0]);
    for(auto && image : mOutputFiles) {
        //File& image(mOutputFiles[outputImageIndex]);
        image.width = imageInput.width;
        image.height = imageInput.height;
        image.size = imageInput.width * imageInput.height * 1.5;
        image.format = static_cast<int32_t>(PixelFormat::YCBCR_420_888);
    }

    configIsp();

    StatisticsInfo info;
    std::vector<uint8_t> metadata;
    ASSERT_EQ(initial_isp_metadata_vector(metadata, 3), 0);

    auto writeMeta = [](uint32_t tag, uint32_t type, int value, std::vector<uint8_t> &metadata) ->int {
        int32_t dataChunk[8];
        dataChunk[0] = value;
        CAM_LOGD("Tag(%d), value(%d), type(%d)", tag, value, type);
        auto err = add_isp_metadata_entry(metadata, tag, type, (void *)(&dataChunk),
                               1 * sizeof(int), nullptr);
        if (err)
        {
            CAM_LOGW("tag[0x%x] is not support with err[%d], maybe data size is too large", tag, err);
        }

        // Dump metadata to check
        mtkisp_metadata_entry_t *pentry = nullptr;
        err = get_isp_metadata_entry(metadata, tag, &pentry);
        if (err)
        {
            CAM_LOGW("Get entry fail", tag, err);
        }
        CAM_LOGD("Get entry data(%d)", pentry->data.i32[0]);
        return 0;
    };
    std::random_device rd;
    std::default_random_engine gen = std::default_random_engine(rd());
    std::uniform_int_distribution<int> dis(1,2000);
    std::uniform_int_distribution<int> dis2(1000,20000);
    int iso = dis(gen), exp = dis2(gen);
    CAM_LOGD("random iso: %d, exp: %dns", iso, exp);

    writeMeta(ANDROID_SENSOR_SENSITIVITY, 1, iso, metadata);
    writeMeta(ANDROID_SENSOR_EXPOSURE_TIME, 1, exp, metadata);
    writeMeta(MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING, 1, 1, metadata);

    info.meta = metadata;
    isphal::V1_0::Status status(isphal::V1_0::Status::OK);
    uint32_t framecount = 0;
    std::vector<FeatureSetting> settings;
    Return<void> ret_t = mDevice->queryFeatureSetting(info, [&status, &framecount, &settings](
                isphal::V1_0::Status _status,
                uint32_t _framecount,
                const hidl_vec<FeatureSetting>& _settings){
            status = _status;
            framecount = _framecount;
            settings = _settings;
            });
    CAM_LOGD("framecount: %d, size of settings: %d", framecount, settings.size());
    //end
    std::vector<ISPRequest> requests;
    buildIspRequests(requests);

    Return<isphal::V1_0::Status> ret = mDevice->processImgRequest(requests);
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in processImgRequest from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "processImgRequest failed";

    // wait result (tiemout is 1000 milliseconds by default)
    const int timeout = 2000;
    ASSERT_EQ(::android::OK, mHalInterface->waitResult(timeout))
        << "did not receive the result for " << timeout << " milliseconds";

    // Encode yuv to jpeg
    std::vector<ISPBuffer> yuvInputBuffers;
    for(auto && ispBuf : requests[0].OutputBuffers) {
        yuvInputBuffers.push_back(ispBuf);
    }

    YUV2JPEG(yuvInputBuffers, mOutputFiles);
}

TEST_F(ISPDeviceTest, singleRequestMFNRnoDrop)
{
    property_set("vendor.mfll.force", "1");
    property_set("vendor.mfll.drop_num", "0");
    property_set("vendor.mfll.capture_num", "6");
    property_set("vendor.mfll.blend_num", "6");
    jsonVerifier();

    // Setup output yuv information, we query it from input
    const File& imageInput(mInputFiles[0]);
    for(auto && image : mOutputFiles) {
        //File& image(mOutputFiles[outputImageIndex]);
        image.width = imageInput.width;
        image.height = imageInput.height;
        image.size = imageInput.width * imageInput.height * 1.5;
        image.format = static_cast<int32_t>(PixelFormat::YCBCR_420_888);
    }

    configIsp();

    std::vector<ISPRequest> requests;
    buildIspRequests(requests);

    Return<isphal::V1_0::Status> ret = mDevice->processImgRequest(requests);
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in processImgRequest from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "processImgRequest failed";

    // wait result (tiemout is 1000 milliseconds by default)
    const int timeout = 2000;
    ASSERT_EQ(::android::OK, mHalInterface->waitResult(timeout))
        << "did not receive the result for " << timeout << " milliseconds";

    // Encode yuv to jpeg
    std::vector<ISPBuffer> yuvInputBuffers;
    for(auto && ispBuf : requests[0].OutputBuffers) {
        yuvInputBuffers.push_back(ispBuf);
    }

    YUV2JPEG(yuvInputBuffers, mOutputFiles);
}

TEST_F(ISPDeviceTest, singleRequestMFNRDropN)
{
    property_set("vendor.mfll.force", "1");
    property_set("vendor.mfll.drop_num", "3");
    property_set("vendor.mfll.capture_num", "6");
    property_set("vendor.mfll.blend_num", "6");
    jsonVerifier();

    // Setup output yuv information, we query it from input
    const File& imageInput(mInputFiles[0]);
    for(auto && image : mOutputFiles) {
        //File& image(mOutputFiles[outputImageIndex]);
        image.width = imageInput.width;
        image.height = imageInput.height;
        image.size = imageInput.width * imageInput.height * 1.5;
        image.format = static_cast<int32_t>(PixelFormat::YCBCR_420_888);
    }

    configIsp();

    std::vector<ISPRequest> requests;
    buildIspRequests(requests);

    Return<isphal::V1_0::Status> ret = mDevice->processImgRequest(requests);
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in processImgRequest from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "processImgRequest failed";

    // wait result (tiemout is 1000 milliseconds by default)
    const int timeout = 2000;
    ASSERT_EQ(::android::OK, mHalInterface->waitResult(timeout))
        << "did not receive the result for " << timeout << " milliseconds";

    // Encode yuv to jpeg
    std::vector<ISPBuffer> yuvInputBuffers;
    for(auto && ispBuf : requests[0].OutputBuffers) {
        yuvInputBuffers.push_back(ispBuf);
    }

    YUV2JPEG(yuvInputBuffers, mOutputFiles);
}

TEST_F(ISPDeviceTest, singleRequestMFNRDropTo1)
{
    property_set("vendor.mfll.force", "1");
    property_set("vendor.mfll.drop_num", "5");
    property_set("vendor.mfll.capture_num", "6");
    property_set("vendor.mfll.blend_num", "6");
    jsonVerifier();

    // Setup output yuv information, we query it from input
    const File& imageInput(mInputFiles[0]);
    for(auto && image : mOutputFiles) {
        //File& image(mOutputFiles[outputImageIndex]);
        image.width = imageInput.width;
        image.height = imageInput.height;
        image.size = imageInput.width * imageInput.height * 1.5;
        image.format = static_cast<int32_t>(PixelFormat::YCBCR_420_888);
    }

    configIsp();

    std::vector<ISPRequest> requests;
    buildIspRequests(requests);

    Return<isphal::V1_0::Status> ret = mDevice->processImgRequest(requests);
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in processImgRequest from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "processImgRequest failed";

    // wait result (tiemout is 1000 milliseconds by default)
    const int timeout = 2000;
    ASSERT_EQ(::android::OK, mHalInterface->waitResult(timeout))
        << "did not receive the result for " << timeout << " milliseconds";

    // Encode yuv to jpeg
    std::vector<ISPBuffer> yuvInputBuffers;
    for(auto && ispBuf : requests[0].OutputBuffers) {
        yuvInputBuffers.push_back(ispBuf);
    }

    YUV2JPEG(yuvInputBuffers, mOutputFiles);
}

TEST_F(ISPDeviceTest, singleRequestAINR2YUV)
{
    jsonVerifier();

    // Setup output yuv information, we query it from input
    // Image output and tuning are allocated as YUV to store data
    const File& imageInput(mInputFiles[0]);
    for(auto && image : mOutputFiles) {
        image.width = imageInput.width;
        image.height = imageInput.height;
        image.size = imageInput.width * imageInput.height * 1.5;
        image.format = static_cast<int32_t>(PixelFormat::YCBCR_420_888);
    }

    configIsp();

    // Query StatisticsInfo
    StatisticsInfo info;
    std::vector<uint8_t> metadata;
    // TODO: We suggest "add_isp_metadata_entry" should support metadata re-allocate
    ASSERT_EQ(initial_isp_metadata_vector(metadata, 3), 0); // 3 means that there are 3 entry of metadata
    auto writeMeta = [](uint32_t tag, uint32_t type, int value, std::vector<uint8_t> &metadata) ->int {
        int32_t dataChunk[8];
        dataChunk[0] = value;
        CAM_LOGD("Tag(0x%x), value(%d), type(%d)", tag, value, type);
        auto err = add_isp_metadata_entry(metadata, tag, type, (void *)(&dataChunk),
                               1 * sizeof(int), nullptr);
        if (err)
        {
            CAM_LOGW("tag[0x%x] is not support with err[%d], maybe data size is too large", tag, err);
        }

        // Dump metadata to check
        mtkisp_metadata_entry_t *pentry = nullptr;
        err = get_isp_metadata_entry(metadata, tag, &pentry);
        if (err)
        {
            CAM_LOGW("Get entry fail", tag, err);
        }
        CAM_LOGD("Get entry data(%d)", pentry->data.i32[0]);
        return 0;
    };

    int iso = 6000;
    int64_t shutter = 59999;
    int tuningHint = 2; // MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_AINR
    writeMeta(ANDROID_SENSOR_SENSITIVITY, 1, iso, metadata);
    writeMeta(ANDROID_SENSOR_EXPOSURE_TIME, 2, shutter, metadata);
    writeMeta(MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING, 1, tuningHint, metadata);
    info.meta = metadata;

    isphal::V1_0::Status status(isphal::V1_0::Status::OK);
    std::vector<FeatureSetting> featureSettings;
    mDevice->queryFeatureSetting(info, [&status, &featureSettings](
                            isphal::V1_0::Status _status,
                            uint32_t framecount, const hidl_vec<FeatureSetting>& _settings) {
                                status = _status;
                                featureSettings = _settings;
                                CAM_LOGD("Capture frameCount(%d) size(%lu)", framecount, featureSettings.size());
                        });

    std::vector<ISPRequest> requests;
    buildIspRequests(requests);

    Return<isphal::V1_0::Status> ret = mDevice->processImgRequest(requests);
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in processImgRequest from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "processImgRequest failed";

    // wait result (tiemout is 1000 milliseconds by default)
    const int timeout = 5000;
    ASSERT_EQ(::android::OK, mHalInterface->waitResult(timeout))
        << "did not receive the result for " << timeout << " milliseconds";

    // Encode yuv to jpeg
    std::vector<ISPBuffer> yuvInputBuffers;
    for(auto && ispBuf : requests[0].OutputBuffers) {
        yuvInputBuffers.push_back(ispBuf);
    }

    YUV2JPEG(yuvInputBuffers, mOutputFiles);
}

TEST_F(ISPDeviceTest, singleRequestAINR2RAW)
{
    jsonVerifier();

    // Setup output yuv information, we query it from input
    // Image output and tuning are allocated as YUV to store data
    const File& imageInput(mInputFiles[0]);
    for(size_t i = 0; i < mOutputFiles.size(); i++) {
        mOutputFiles[i].width = imageInput.width;
        mOutputFiles[i].height = imageInput.height;
        if(i % 2 == 0) {
            mOutputFiles[i].size = imageInput.width * imageInput.height * 2;
            mOutputFiles[i].format = static_cast<int32_t>(PixelFormat::RAW16);
        } else {
            mOutputFiles[i].size = imageInput.width * imageInput.height * 1.5;
            mOutputFiles[i].format = static_cast<int32_t>(PixelFormat::YCBCR_420_888);
        }
    }

    configIsp();

    std::vector<ISPRequest> requests;
    buildIspRequests(requests);

    Return<isphal::V1_0::Status> ret = mDevice->processImgRequest(requests);
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in processImgRequest from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "processImgRequest failed";

    // wait result (tiemout is 1000 milliseconds by default)
    const int timeout = 5000;
    ASSERT_EQ(::android::OK, mHalInterface->waitResult(timeout))
        << "did not receive the result for " << timeout << " milliseconds";

#if 0
    // Encode raw to yuv
    std::vector<ISPBuffer> rawInputBuffers;
    for(auto && ispBuf : requests[0].OutputBuffers) {
        rawInputBuffers.push_back(ispBuf);
    }

    std::vector<ISPBuffer> outIspBuffers;
    RAW2YUV(rawInputBuffers, outIspBuffers, mOutputFiles);
#endif

    // Parse resolution from OutputFiles and determine format and size by requirement
    std::vector<File> inputYuvFiles;
    for (auto && file : mOutputFiles)
    {
        // get the first input image's width and height
        uint32_t width = file.width;
        uint32_t height = file.height;
        int32_t format = static_cast<int32_t>(PixelFormat::YCBCR_420_888);
        int32_t size = width * height * 1.5;
        std::string fileName = "result/result_01.yuv";
        std::vector<Json::UInt> rowStrides {0, 0, 0};

        File yuvFile(mRootDirectory.value.asString(), fileName,
                        width, height, format, std::move(rowStrides), size);
        inputYuvFiles.push_back(yuvFile);
    }
    std::vector<ISPBuffer> outIspBuffers;
    YUV2JPEG(outIspBuffers, inputYuvFiles);
}

TEST_F(ISPDeviceTest, singleRequestAINR2JPEG)
{
    jsonVerifier();

    configIsp();

    std::vector<ISPRequest> requests;
    buildIspRequests(requests);

    Return<isphal::V1_0::Status> ret = mDevice->processImgRequest(requests);
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in processImgRequest from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "processImgRequest failed";

    // wait result (tiemout is 1000 milliseconds by default)
    const int timeout = 5000;
    ASSERT_EQ(::android::OK, mHalInterface->waitResult(timeout))
        << "did not receive the result for " << timeout << " milliseconds";
}

TEST_F(ISPDeviceTest, multipleRequestMFNR)
{
    // reconstruct buffers and settings from the JSON file
    jsonVerifier();
}

TEST_F(ISPDeviceTest, singleRequestYUV2JPEG_multicam)
{
    // reconstruct buffers and settings from the JSON file
    jsonVerifier();

    // configure ISP device
    ISPStreamConfiguration hidlConfiguration;
    auto initEmptyMetadata = [](ISPMetadata& metadata)
    {
        std::vector<uint8_t> emptyMetadata;
        ASSERT_EQ(initial_isp_metadata_vector(emptyMetadata, 1), 0);
        metadata = emptyMetadata;
    };

    // TODO: fill metadata if necessary
    ISPMetadata configureParams;
    initEmptyMetadata(configureParams);

    auto constructISPStream = [](Direction direction,
            const std::vector<uint64_t>& streams, const std::vector<File>& files)
    {
        std::vector<ISPStream> ispStreams;

        for (size_t i = 0; i < streams.size(); i++)
        {
            const File& file(files[i]);
            // NOTE: the usage is referenced from Camera3Device::createStream() in
            // frameworks/av/services/camera/libcameraservice/device3/Camera3Device.cpp
            // and Stream::usage in
            // hardware/interfaces/camera/device/3.2/types.hal
            //
            // TODO: correct dataspace if necessary
            ISPStream stream {
                .id = streams[i],
                .size   = file.size,
                .stride = file.isJPEG() ?
                    hidl_vec<uint32_t>({ file.size, 0, 0 }) :
                    hidl_vec<uint32_t>(file.rowStride),
                .width  = file.width,
                .height = file.height,
                .format = { file.getPixelFormat(),
                    static_cast<DataspaceFlags>(
                            file.isJPEG() ? Dataspace::V0_JFIF : Dataspace::UNKNOWN) },
                .usage = direction == Direction::IN ?
                    GRALLOC1_CONSUMER_USAGE_NONE : kConsumerUsage,
                .transform = 0
            };
            ispStreams.push_back(std::move(stream));
        }

        return ispStreams;
    };

    hidlConfiguration.InputStreams = constructISPStream(Direction::IN,
            mStreamConfiguration.inputStreams, mInputFiles);
    hidlConfiguration.OutputStreams = constructISPStream(Direction::OUT,
            mStreamConfiguration.outputStreams, mOutputFiles);
    hidlConfiguration.ConfigureParams = configureParams;

    // NOTE: image and metadata are paired per input buffer
    ASSERT_TRUE((mInputFiles.size() > 0) && (mInputFiles.size() % 2 == 0))
        << "the number of input files must be greater than 0 and be a multiple of 2";
    ASSERT_TRUE(mOutputFiles.size() > 0)
        << "the number of output files must be greater than 0";

    CAM_LOGD("[Input Streams]");
    for (const auto& inputStream : hidlConfiguration.InputStreams)
        CAM_LOGD("%s", toString(inputStream).c_str());
    CAM_LOGD("[Output Streams]");
    for (const auto& outputStream : hidlConfiguration.OutputStreams)
        CAM_LOGD("%s", toString(outputStream).c_str());
    // TODO: print metadata

    // configure ISP device
    Return<isphal::V1_0::Status> ret = mDevice->configureISPDevice(
            hidlConfiguration, mHalInterface->getICallback());
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in configureISPDevice from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "configureISPDevice failed";

    // process image requests
    std::vector<ISPRequest> requests;
    const size_t totalRequests(getTotalRequests());

    size_t inputImageIndex = 0;
    size_t inputMetadataIndex = 1;

    for (size_t outFileIdx = 0; outFileIdx < mOutputFiles.size(); outFileIdx++)
    {
        requests.clear();
        const RequestConfiguration& requestConfiguration(mRequestConfigurations[0]);
        const uint32_t requestNumber = outFileIdx;

        CAM_LOGD("construct ISP request %u", requestNumber);

        // construct metadata
        // TODO: fill metadata if necessary
        ISPMetadata settings;
        initEmptyMetadata(settings);

        // construct input buffers
        // NOTE: ISPDevice allows one or more input buffers per request
        HalInterface::FileBuffer inputBuffers;
        for (const auto streamId : requestConfiguration.inputBuffers)
        {
            // load image into hidl handle
            hidl_handle image_handle;

            const File& image(mInputFiles[inputImageIndex]);
            {
                AllocatorHelper::getInstance().allocateGraphicBuffer(
                        image.width, image.height,
                        kGralloc0Usage,
                        image.getPixelFormat(), &image_handle);
                CAM_LOGD("input image [%zu](%s)", inputImageIndex,
                        toString(image_handle).c_str());

                buffer_handle_t importedBuffer = image_handle.getNativeHandle();
                MapperHelper::getInstance().importBuffer(importedBuffer);

                // lock buffer for writing
                const IMapper::Rect region { 0, 0, static_cast<int32_t>(image.width),
                    static_cast<int32_t>(image.height) };
                int fence = -1;

                YCbCrLayout layout;
                MapperHelper::getInstance().lockYCbCr(importedBuffer,
                        static_cast<uint64_t>(BufferUsage::CPU_WRITE_OFTEN),
                        region, fence, layout);
                CAM_LOGD("lock layout: %s", toString(layout).c_str());

                std::ifstream ifs(image.getFullyQualifiedName(),
                        (std::ios::in | std::ios::binary));
                ASSERT_TRUE(ifs.is_open()) << strerror(errno) << ": " <<
                    image.getFullyQualifiedName();

                auto yData = static_cast<uint8_t*>(layout.y);
                auto cbData = static_cast<uint8_t*>(layout.cb);
                auto crData = static_cast<uint8_t*>(layout.cr);

                char value;
                for (uint32_t y = 0; y < image.height; y++)
                {
                    for (uint32_t x = 0; x < image.width; x++)
                    {
                        if (ifs.get(value))
                            yData[layout.yStride * y + x] = value;
                        else
                            ASSERT_TRUE(false) << "cannot read more data from file "
                                << image.getFullyQualifiedName();
                    }
                }

                uint32_t uvHeight = image.height >> 1;
                uint32_t uvWidth = image.width >> 1;

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
                                ASSERT_TRUE(false) << "cannot read more data from file "
                                    << image.getFullyQualifiedName();

                            if (ifs.get(value))
                                cbData[layout.cStride * y + step] = value;
                            else
                                ASSERT_TRUE(false) << "cannot read more data from file "
                                    << image.getFullyQualifiedName();
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
                                ASSERT_TRUE(false) << "cannot read more data from file "
                                    << image.getFullyQualifiedName();
                        }
                    }

                    for (uint32_t y = 0; y < uvHeight; y++) {
                        for (uint32_t x = 0, step = 0; x < uvWidth; x++, step+=layout.chromaStep) {
                            if (ifs.get(value))
                                crData[layout.cStride * y + step] = value;
                            else
                                ASSERT_TRUE(false) << "cannot read more data from file "
                                    << image.getFullyQualifiedName();
                        }
                    }
                }

                ifs.close();

                MapperHelper::getInstance().unlock(importedBuffer, fence);
                if (fence >= 0)
                    close(fence);

                MapperHelper::getInstance().freeBuffer(importedBuffer);

                // TODO: lock again for bit-true validation
            }

            // load metadata into hidl handle
            hidl_handle metadata_handle;
            const File& metadata(mInputFiles[inputMetadataIndex]);
            {
                // TODO: set metadata format to blob when parsing from JSON settings
                uint32_t rowStride = 0;
                AllocatorHelper::getInstance().allocateGraphicBuffer(
                        metadata.size, 1u,
                        kGralloc0Usage,
                        PixelFormat::BLOB, &metadata_handle, &rowStride);
                CAM_LOGD("input metadata [%zu](%s)", inputMetadataIndex,
                        toString(metadata_handle).c_str());

                buffer_handle_t importedBuffer = metadata_handle.getNativeHandle();
                MapperHelper::getInstance().importBuffer(importedBuffer);

                // lock buffer for writing
                const IMapper::Rect region { 0, 0, static_cast<int32_t>(metadata.size), 1u };
                int fence = -1;
                void* addr = nullptr;

                MapperHelper::getInstance().lock(importedBuffer,
                        static_cast<uint64_t>(BufferUsage::CPU_WRITE_OFTEN),
                        region, fence, addr);
                ASSERT_NE(addr, nullptr) << "lock metadata failed";
                CAM_LOGD("lock addr(0x%X)", reinterpret_cast<intptr_t>(addr));

                std::ifstream ifs(metadata.getFullyQualifiedName(),
                        (std::ios::in | std::ios::binary));
                ASSERT_TRUE(ifs.is_open()) << strerror(errno) << ": " <<
                    metadata.getFullyQualifiedName();

                auto data = static_cast<uint8_t*>(addr);
                char value;
                int count = 0;
                while (ifs.get(value))
                {
                    CAM_LOGD_IF(count < 16, "%d: 0x%X", count, value);
                    data[count] = value;
                    count++;
                }
                CAM_LOGD("metadata: write %d bytes", count);
                ifs.close();

                MapperHelper::getInstance().unlock(importedBuffer, fence);
                if (fence >= 0)
                    close(fence);

                MapperHelper::getInstance().freeBuffer(importedBuffer);

                // lock again for metadata header validation
                {
                    buffer_handle_t importedBuffer = metadata_handle.getNativeHandle();
                    MapperHelper::getInstance().importBuffer(importedBuffer);
                    // lock buffer for writing
                    const IMapper::Rect region { 0, 0, static_cast<int32_t>(metadata.size), 1u };
                    int fence = -1;
                    void* data = nullptr;

                    MapperHelper::getInstance().lock(importedBuffer,
                            static_cast<uint64_t>(BufferUsage::CPU_READ_OFTEN),
                            region, fence, data);

                    ASSERT_NE(data, nullptr);

                    // show metadata header
                    uint8_t *addr = static_cast<uint8_t *>(data);
                    CAM_LOGD("metadata size(%d)", metadata.size);
                    CAM_LOGD("metadata header begin");
                    for (int i = 0; i < 4; i++)
                    {
                        CAM_LOGD("0x%X 0x%X 0x%X 0x%X",
                                *addr, *(addr+1), *(addr+2), *(addr+3));
                        addr = addr + 4;
                    }
                    CAM_LOGD("metadata header end");

                    MapperHelper::getInstance().unlock(importedBuffer, fence);
                    if (fence >= 0)
                        close(fence);

                    MapperHelper::getInstance().freeBuffer(importedBuffer);
                }
            }

            // construct ISP buffer
            ISPBuffer buffer
            {
                .id = streamId,
                .buffer = image_handle,
                .tuningData = metadata_handle
            };
            inputBuffers.first.push_back(image);
            inputBuffers.second.push_back(std::move(buffer));

        }

        // construct output buffers
        HalInterface::FileBuffer outputBuffers;
        for (const auto streamId : requestConfiguration.outputBuffers)
        {
            const File& image(mOutputFiles[outFileIdx]);
            hidl_handle image_handle;
            // NOTE: jpeg is a blob format
            AllocatorHelper::getInstance().allocateGraphicBuffer(
                    image.isJPEG() ? image.size : image.width,
                    image.isJPEG() ? 1 : image.height,
                    kGralloc0Usage,
                    image.getPixelFormat(), &image_handle);
            CAM_LOGD("output image [%zu](%s)", outFileIdx,
                    toString(image_handle).c_str());

            // construct ISP buffer
            ISPBuffer buffer
            {
                .id = streamId,
                .buffer = image_handle,
                .tuningData = hidl_handle() // tuning data is no use for output buffer
            };
            outputBuffers.first.push_back(image);
            outputBuffers.second.push_back(std::move(buffer));

        }

        // construct ISP request
        ISPRequest request
        {
            .reqNumber = requestNumber,
            .settings = settings,
            .InputBuffers = inputBuffers.second,
            .OutputBuffers = outputBuffers.second
        };
        requests.push_back(std::move(request));

        // record capture requests for callback lookup
        mHalInterface->recordISPRequest(requestNumber,
                std::move(inputBuffers), std::move(outputBuffers));

        CAM_LOGD("Y2J-mul processImgRequest ... %d", outFileIdx);

        ret = mDevice->processImgRequest(requests);
        ASSERT_TRUE(ret.isOk())
            << "Transaction error in processImgRequest from ISPDevice: "
            << ret.description().c_str();
        ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "processImgRequest failed";

        // wait result (tiemout is 1000 milliseconds by default)
        const int timeout = 1000;
        ASSERT_EQ(::android::OK, mHalInterface->waitResult(timeout))
            << "did not receive the result for " << timeout << " milliseconds";

        CAM_LOGD("Y2J-mul done ... %d", outFileIdx);

        inputImageIndex += 2;
        inputMetadataIndex += 2;
    }
}

#ifdef ENABLE_JPEG_BUFFER_TEST
TEST_F(JPEGBufferTest, JPEGStream)
{
    const std::string jpegBufferName("/sdcard/jpeg.blob");

    const auto jpegBufferSize = getFileSize(jpegBufferName);
    ASSERT_NE(jpegBufferSize, -1) << "get file size failed";

    std::unique_ptr<char[]> jpegBuffer(new char[jpegBufferSize]);

    // read jpeg stream buffer from file
    std::ifstream ifs(jpegBufferName, (std::ios::in | std::ios::binary));
    ASSERT_TRUE(ifs.is_open()) << strerror(errno) << ": " << jpegBufferName << "\n";
    ifs.read(jpegBuffer.get(), jpegBufferSize);
    ifs.close();

    // show CameraBlob
    CAM_LOGD("parse jpegBuffer(%s) size(%ld)",
            jpegBufferName.c_str(), jpegBufferSize);
    const CameraBlob *transportHeader = reinterpret_cast<const CameraBlob*>(
            jpegBuffer.get() + jpegBufferSize - sizeof(CameraBlob));
    CAM_LOGD("%s", toString(*transportHeader).c_str());

    // extract jpeg image from jpeg stream buffer and save it to file
    std::ofstream ofs(jpegBufferName + ".jpeg", (std::ios::out | std::ios::binary));
    ASSERT_TRUE(ofs.is_open()) << strerror(errno) << ": " << jpegBufferName << "\n";
    ofs.write(jpegBuffer.get(), transportHeader->blobSize);
    ofs.close();
}
#endif // ENABLE_JPEG_BUFFER_TEST

TEST_F(ISPDeviceTest, singleRequestRAW2YUV_Logical_multicam)
{
    // reconstruct buffers and settings from the JSON file
    jsonVerifier();

    const File& imageInput(mInputFiles[0]);

    for(auto && image : mOutputFiles) {
        //File& image(mOutputFiles[outputImageIndex]);
        image.width = imageInput.width;
        image.height = imageInput.height;
        image.size = imageInput.width * imageInput.height * 1.5;
        image.format = static_cast<int32_t>(PixelFormat::YCBCR_420_888);
    }
    //
    configIsp();
    //
    std::vector<ISPRequest> requests;
    buildIspRequests(requests);
    //
    Return<isphal::V1_0::Status> ret = mDevice->processImgRequest(requests);
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in processImgRequest from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "processImgRequest failed";

    // wait result (tiemout is 1000 milliseconds by default)
    const int timeout = 1000;
    ASSERT_EQ(::android::OK, mHalInterface->waitResult(timeout))
        << "did not receive the result for " << timeout << " milliseconds";

    // Encode yuv to jpeg
    std::vector<ISPBuffer> yuvInputBuffers;
    for(auto && ispBuf : requests[0].OutputBuffers) {
        yuvInputBuffers.push_back(ispBuf);
    }

    YUV2JPEG(yuvInputBuffers, mOutputFiles);
}

TEST_F(ISPDeviceTest, singleZSL_RAW2YUV_Logical_multicam)
{
    // reconstruct buffers and settings from the JSON file
    jsonVerifier();

    const File& imageInput(mInputFiles[0]);

    for(auto && image : mOutputFiles) {
        //File& image(mOutputFiles[outputImageIndex]);
        image.width = imageInput.width;
        image.height = imageInput.height;
        image.size = imageInput.width * imageInput.height * 1.5;
        image.format = static_cast<int32_t>(PixelFormat::YCBCR_420_888);
    }
    //
    configIsp();
    //
    std::vector<ISPRequest> requests;
    buildIspRequests(requests);
    //
    Return<isphal::V1_0::Status> ret = mDevice->processImgRequest(requests);
    ASSERT_TRUE(ret.isOk())
        << "Transaction error in processImgRequest from ISPDevice: "
        << ret.description().c_str();
    ASSERT_EQ(ret, isphal::V1_0::Status::OK) << "processImgRequest failed";

    // wait result (tiemout is 1000 milliseconds by default)
    const int timeout = 1000;
    ASSERT_EQ(::android::OK, mHalInterface->waitResult(timeout))
        << "did not receive the result for " << timeout << " milliseconds";

    // Encode yuv to jpeg
    std::vector<ISPBuffer> yuvInputBuffers;
    for(auto && ispBuf : requests[0].OutputBuffers) {
        yuvInputBuffers.push_back(ispBuf);
    }

    YUV2JPEG(yuvInputBuffers, mOutputFiles);
}


// ------------------------------------------------------------------------

int runGoogleTest()
{
    return RUN_ALL_TESTS();
}

// ------------------------------------------------------------------------

std::ostream& printOneValue(const Json::Value& value, std::stringstream& ss)
{
    switch (value.type())
    {
        case Json::nullValue:
            ss << "(null)";
            break;
        case Json::intValue:
            ss << value.asInt();
            break;
        case Json::uintValue:
            ss << value.asUInt();
            break;
        case Json::realValue:
            ss << value.asDouble();
            break;
        case Json::booleanValue:
            ss << value.asBool();
            break;
        case Json::stringValue:
            ss << value.asCString();
            break;
        default:
            ss << "unknown type=[" << value.type() << "]";
    }

    return ss;
}

void traverseJSON(const Json::Value& root, size_t depth)
{
    static std::stringstream ss;

    CAM_LOGV("(%zu) root type(%d) size(%u)", depth, root.type(), root.size());

    // traverse non-leaf node (object or array) recursively
    if (root.size() > 0)
    {
        if (root.type() == Json::objectValue)
            ss << std::setfill(' ') << std::setw(depth+1) << "{" << std::endl;
        else if (root.type() == Json::arrayValue)
            ss << std::setfill(' ') << std::setw(depth+1) << "[" << std::endl;

        Json::Value::iterator end = root.end();
        for (Json::Value::iterator it = root.begin(); it != end; ++it)
        {
            // do not break line if the next level reaches the leaf node
            if ((it->size() >= 0) && (root.type() == Json::arrayValue))
                ss << std::setfill(' ') << std::setw(depth+1) << ' ';
            else if ((it->size() >= 0) && (root.type() == Json::objectValue))
                ss << std::setfill(' ') << std::setw(depth+1) << ' ';

            // print either the index of the member name of the referenced value
            printOneValue(it.key(), ss) << ": ";

            // do not break line if the next level reaches the leaf node
            if (it->size() > 0)
            {
                ss << std::endl;
                CAM_LOGD("%s", ss.str().c_str());
                ss.str(std::string());
            }

            traverseJSON(*it, depth+1);
        }

        if (root.type() == Json::objectValue)
            ss << std::endl << std::setfill(' ') << std::setw(depth+1) << "}";
        else if (root.type() == Json::arrayValue)
            ss << std::endl << std::setfill(' ') << std::setw(depth+1) << "]";

        ss << std::endl;
        CAM_LOGD("%s", ss.str().c_str());
        ss.str(std::string());

        return;
    }

    // leaf node (value except object or arrary type)
    printOneValue(root, ss) << std::endl;
    CAM_LOGD("%s", ss.str().c_str());
    ss.str(std::string());
}

int jsonTest()
{
    const Json::Value& testCaseSetting(
            JSONContainer::getInstance().getTestCaseSetting());

    // 0. traverse JSON object/array
    traverseJSON(testCaseSetting, 1);

    // 1. Show test suite names
    std::vector<std::string> testSuiteNameList;
    {
        Json::Value::iterator end = testCaseSetting.end();
        for (Json::Value::iterator it = testCaseSetting.begin(); it != end; ++it)
        {
            std::stringstream ss;
            printOneValue(it.key(), ss);
            testSuiteNameList.push_back(ss.str());
            CAM_LOGD("Test suite: %s", ss.str().c_str());
        }
    }

    // 2. Show test names
    std::vector<std::string> testNameList;
    for (const auto& testSuiteName : testSuiteNameList)
    {
        const Json::Value testSuite = testCaseSetting[testSuiteName.c_str()];
        Json::Value::iterator end = testSuite.end();
        for (Json::Value::iterator it = testSuite.begin(); it != end; ++it)
        {
            std::stringstream ss;
            printOneValue(it.key(), ss);
            testNameList.push_back(ss.str());
            CAM_LOGD("Test name: %s", ss.str().c_str());
        }
    }

    return ::android::OK;
}

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
                if (!!JSONContainer::getInstance().getTestCaseSetting())
                    runGoogleTest();
                break;
            case 'j':
                if (!!JSONContainer::getInstance().getTestCaseSetting())
                    jsonTest();
                break;
            default:
                break;
        }
    }

    return ::android::OK;
}
