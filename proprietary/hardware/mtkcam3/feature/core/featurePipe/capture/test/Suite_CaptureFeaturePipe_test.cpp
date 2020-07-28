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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#include <gtest/gtest.h>
#include <memory>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <functional>
#include <atomic>
#include <chrono>
#include <thread>
#include <mtkcam/drv/IHalSensor.h>
#include <cstring>

#include "TestUtil.h"
#include <mtkcam/def/common.h>
#include <iostream>
#include <mtkcam3/feature/featurePipe/ICaptureFeaturePipe.h>
#include <featurePipe/core/include/ImageBufferPool.h>
#include <featurePipe/core/include/GraphicBufferPool.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
#include <mtkcam/utils/std/Format.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include "../CaptureFeature_Common.h"

#define LOG_TAG "CapturePipeUnitTest"
#include <mtkcam/utils/std/Log.h>


#define MY_LOGD(fmt, arg...) CAM_LOGD(LOG_TAG "[%s] " fmt "\n", __FUNCTION__, ##arg)
/*******************************************************************************
* MACRO Utilities Define.
********************************************************************************/
namespace { // anonymous namespace for MACRO function
using AutoObject = std::unique_ptr<const char, std::function<void(const char*)>>;

std::unique_ptr<char>
getPreFix(const char* text)
{
    static const MINT32 LENGTH = 256;
    auto ret = std::unique_ptr<char>(new char[LENGTH]);
    ::strcpy(ret.get(), "[");
    ::strcat(ret.get(), text);
    ::strcat(ret.get(), "] ");
    return ret;
}
//
void printLog(const char* text, const char* fmt, ...)
{
    static const MINT32 LENGTH = 1024;
    auto buf = std::unique_ptr<char>(new char[LENGTH]);

    ::strcpy(buf.get(), getPreFix(text).get());
    ::strcat(buf.get(), fmt);

    const MINT32 newLength = LENGTH - ::strlen(buf.get());
    va_list ap;
    ::va_start(ap, fmt);
    ::vsnprintf(buf.get(), newLength, fmt, ap);
    ::va_end(ap);

    std::cout << buf.get() << std::endl;
}
#define UT_LOGD(FMT, arg...) printLog(__FUNCTION__, FMT, ##arg)
//
auto
createAutoScoper(const char* funcName) -> AutoObject
{
    CAM_LOGD("[%s] +", funcName);
    return AutoObject(funcName, [](const char* p)
    {
        CAM_LOGD("[%s] -", p);
    });
}
#define SCOPED_TRACER_NAME(TEXT) auto scoped_tracer = ::createAutoScoper(TEXT)
#define SCOPED_TRACER() auto scoped_tracer = ::createAutoScoper(__FUNCTION__)
//
} // end anonymous namespace for MACRO function


/*******************************************************************************
* Alias.
********************************************************************************/
using namespace NSCam;
using namespace NSCam::NSCamFeature::NSFeaturePipe::NSCapture;


/******************************************************************************
 *
 ******************************************************************************/
static char * sMain1FSFilename = {"/sdcard/test/BID_INPUT_FSRAW_1_4208x3120_5260_3.raw"};
static char * sMain2FSFilename = {"/sdcard/test/BID_INPUT_FSRAW_2_4208x3120_5260_3.raw"};
static char * sMain1RSFilename = {"/sdcard/test/BID_INPUT_RSRAW_1_1920x1248_3600_0.raw"};
static char * sMain2RSFilename = {"/sdcard/test/BID_INPUT_RSRAW_2_1920x1248_3600_0.raw"};
static char * sLCSO1Filename   = {"/sdcard/test/BID_LCSO_1_BUF_384x384_768.yuv"};
static char * sLCSO2Filename   = {"/sdcard/test/BID_LCSO_2_BUF_384x384_768.yuv"};

static const MSize fsRawSize(4208, 3120);
static const MSize rsRawSize(1920, 1248);

static const MSize yuvSize(4208, 3120);
static const MSize yuvPostviewSize(1920, 1080);
static const MSize thumbSize(160, 128);

static const MUINT32 lcsStride = 768;
static const MSize lcsSize(384, 384);

static const MSize depthSize(1600, 1200);

/*******************************************************************************
* Class Define
*******************************************************************************/
class WorkingBufferHandle final: public BufferHandle
{
public:
    WorkingBufferHandle(const char* name, MINT32 format, const MSize& size, const char* path = nullptr)
        : mName(name)
        , mPath(path)
        , mFormat(format)
        , mSize(size)
    {
        const MUINT32 plane = NSCam::Utils::Format::queryPlaneCount(mFormat);
        size_t bufBoundaryInBytes[3] = {0, 0, 0};
        size_t bufStridesInBytes[3] = {0};

        for (MUINT32 i = 0; i < plane; i++)
        {
            bufStridesInBytes[i] = NSCam::Utils::Format::queryPlaneWidthInPixels(mFormat, i, mSize.w) *
                                   NSCam::Utils::Format::queryPlaneBitsPerPixel(mFormat, i) / 8;
        }

        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam(
            (EImageFormat)mFormat, mSize, bufStridesInBytes, bufBoundaryInBytes, plane);

        mpImageBufferHeap = IIonImageBufferHeap::create(mName, imgParam);
    }

    MERROR acquire(MINT usage) override
    {
        mpImageBuffer = mpImageBufferHeap->createImageBuffer();
        if (mpImageBuffer == nullptr)
        {
            std::cerr << "working buffer: create image buffer failed" << std::endl;
            return UNKNOWN_ERROR;
        }
#if 0
        if (mPath != NULL) {
           mpImageBuffer->loadFromFile(mPath);
        }
#endif
        mpImageBuffer->setColorArrangement(SENSOR_FORMAT_ORDER_RAW_R);

        if (!(mpImageBuffer->lockBuf(mName, usage)))
        {
            std::cerr << "working buffer: lock image buffer failed" << std::endl;
            return UNKNOWN_ERROR;
        }
        return OK;
    }

    IImageBuffer* native() override
    {
        return mpImageBuffer.get();
    }

    void release() override
    {
        mpImageBuffer->unlockBuf(mName);
        mpImageBuffer.clear();
        mpImageBufferHeap.clear();
    }

    MVOID dump(std::ostream& os) const
    {
        os << "{name: " << mName << ", format: " << mFormat
           << ", size: (" << mSize.w << "x" << mSize.h << ")}";

    }

    ~WorkingBufferHandle()
    {
        if (mpImageBuffer != NULL)
        {
            mpImageBuffer->unlockBuf(mName);
            mpImageBuffer.clear();
            mpImageBufferHeap.clear();
        }
    }

private:
    const char*     mName;
    const char*     mPath;
    const MUINT32   mFormat;
    const MSize     mSize;

    sp<IImageBufferHeap>    mpImageBufferHeap;
    sp<IImageBuffer>        mpImageBuffer;
};

/******************************************************************************
 *
 ******************************************************************************/
class WorkingMetadataHandle final: public MetadataHandle
{
public:
    WorkingMetadataHandle()
    {
        {
            IMetadata exifMeta;
            IMetadata::IEntry entry_iso(MTK_3A_EXIF_AE_ISO_SPEED);
            entry_iso.push_back(1600, Type2Type< MINT32 >());
            exifMeta.update(entry_iso.tag(), entry_iso);
            trySetMetadata<IMetadata>(&mMetadata, MTK_3A_EXIF_METADATA, exifMeta);
        }
        //trySetMetadata<MINT32>(&mMetadata, MTK_JPEG_ORIENTATION, 270);
        trySetMetadata<MINT32>(&mMetadata, MTK_P1NODE_PROCESSOR_MAGICNUM, 0);

        IMetadata::IEntry entry(MTK_EIS_REGION);
        {
            entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_XINT
            entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_XFLOAT
            entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_YINT
            entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_YFLOAT
            entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_WIDTH
            entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_HEIGHT
            entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_MV2CENTERX
            entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_MV2CENTERY
            entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_ISFROMRZ
            entry.push_back(8, Type2Type< MINT32 >());  // EIS_REGION_INDEX_GMVX
            entry.push_back(8, Type2Type< MINT32 >());  // EIS_REGION_INDEX_GMVY
            entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_CONFX
            entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_CONFY
            entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_EXPTIME
            entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_HWTS
            entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_LWTS
            entry.push_back(0, Type2Type< MINT32 >());  // EIS_REGION_INDEX_MAX_GMV
            entry.push_back(0, Type2Type< MBOOL >());   // EIS_REGION_INDEX_ISFRONTBIN
        }
        mMetadata.update(MTK_EIS_REGION, entry);


        trySetMetadata<MSize>(&mMetadata, MTK_HAL_REQUEST_SENSOR_SIZE, fsRawSize);

        trySetMetadata<MRect>(&mMetadata, MTK_P1NODE_SCALAR_CROP_REGION, MRect(MPoint(), fsRawSize));
        trySetMetadata<MRect>(&mMetadata, MTK_P1NODE_DMA_CROP_REGION, MRect(MPoint(), fsRawSize));
        trySetMetadata<MSize>(&mMetadata, MTK_P1NODE_RESIZER_SIZE, MSize(1920, 1080));

        trySetMetadata<MINT32>(&mMetadata, MTK_P1NODE_SENSOR_MODE, SENSOR_SCENARIO_ID_NORMAL_CAPTURE);

        trySetMetadata<MRect>(&mMetadata, MTK_SCALER_CROP_REGION, MRect(MPoint(), fsRawSize));
        trySetMetadata<MUINT8>(&mMetadata, MTK_CONTROL_VIDEO_STABILIZATION_MODE, MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF);
    }

    MERROR acquire() override
    {
        return OK;
    }

    IMetadata* native() override
    {
        return &mMetadata;
    }

    void release() override
    {
    }

    MVOID dump(std::ostream& os) const
    {
        os << "{count: " << mMetadata.count() << "}";
    }

private:
    IMetadata                   mMetadata;
};

/******************************************************************************
 *
 ******************************************************************************/
class TestCallback final: public virtual RequestCallback
{
public:
    MUINT32 getReceivedEventCount() { return mCompletedCount; }

public:
    MUINT32 waitForDesiredCount(MUINT32 desiredCount, const std::chrono::milliseconds timeout)
    {
        const auto startWaitTime = std::chrono::system_clock::now();
        std::chrono::milliseconds waitedTime(0);
        MUINT32 currentCount(0);
        while(MTRUE)
        {
            waitedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startWaitTime);
            currentCount = getReceivedEventCount();
            if(desiredCount == currentCount)
            {
                UT_LOGD("wait for desired count completed, count: %d (%d), waitedTime: %lld (%lld) ms.", currentCount, desiredCount, waitedTime.count(), timeout.count());
                break;
            }
            else if(waitedTime.count() < timeout.count())
            {
                UT_LOGD("wait for desired count, count: %d (%d), waitedTime: %lld (%lld) ms.", currentCount, desiredCount, waitedTime.count(), timeout.count());
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            else // wait timeout
            {
                UT_LOGD("wait for desired count timeout, count: %d (%d), waitedTime: %lld (%lld) ms.", currentCount, desiredCount, waitedTime.count(), timeout.count());
                break;
            }
        }
        return currentCount;
    }


public:
    void onMetaResultAvailable(android::sp<ICaptureFeatureRequest> requestPtr, IMetadata* partialMeta)
    {
        std::cout << "request metaResultAvailable - no: "  << requestPtr->getRequestNo() << std::endl;
    }
    void onContinue(android::sp<ICaptureFeatureRequest> requestPtr) override
    {
        std::cout << "request continue - no: "  << requestPtr->getRequestNo() << std::endl;
    }

    void onAborted(android::sp<ICaptureFeatureRequest> requestPtr) override
    {
        std::cout << "request aborted - no: "  << requestPtr->getRequestNo() << std::endl;
    }

    void onCompleted(android::sp<ICaptureFeatureRequest> requestPtr, MERROR error) override
    {
        MUINT32 newCount = mCompletedCount++;
        UT_LOGD("request completed, req#:%d, comppletedCount:%d", requestPtr->getRequestNo(), newCount);
    }

    ~TestCallback()
    {

    }

private:
    std::atomic<MUINT32> mCompletedCount;
};


class MySensor
{
public:
    MySensor();
    ~MySensor();

public:
    MUINT32 getMain1Index() { return mSensorIndex[0]; }
    MUINT32 getMain2Index() { return mSensorIndex[1]; }

    MBOOL init();

private:
    const std::string   mName;
    MUINT32             mSensorIndex[2];
    IHalSensor*         mHalSensor;
};


MySensor::
MySensor()
: mName {"MySansor"}
, mSensorIndex {0, 1}
, mHalSensor {nullptr}
{

}

MySensor::
~MySensor()
{
    if(mHalSensor)
    {
        const MUINT32 size = sizeof(mSensorIndex)/sizeof(mSensorIndex[0]);
        mHalSensor->powerOff(mName.c_str(), size, mSensorIndex);
        mHalSensor->destroyInstance(mName.c_str());
        mHalSensor = nullptr;
        UT_LOGD("powerOffSensor, index0=%u, index1=%u", mSensorIndex[0], mSensorIndex[1]);
    }
}

MBOOL
MySensor::
init()
{
    const MUINT32 size = sizeof(mSensorIndex)/sizeof(mSensorIndex[0]);

    IHalSensor* pHalSensor = nullptr;
    NSCam::IHalSensorList* sensorList = MAKE_HalSensorList();

    if(!sensorList)
    {
        UT_LOGD("failed to get sensor list, index0=%u, index1=%u", mSensorIndex[0], mSensorIndex[1]);
        return MFALSE;
    }

    const MUINT32 count = sensorList->searchSensors();
    mHalSensor = sensorList->createSensor(mName.c_str(), size, mSensorIndex);

    if(!mHalSensor)
    {
        UT_LOGD("failed to createSensor, index0=%u, index1=%u", mSensorIndex[0], mSensorIndex[1]);
        return MFALSE;
    }
    UT_LOGD("createSensor, count=%u index0=%u, index1=%u", count, mSensorIndex[0], mSensorIndex[1]);

    UT_LOGD("powerOnSensor, index0=%u, index1=%u", mSensorIndex[0], mSensorIndex[1]);
    mHalSensor->powerOn(mName.c_str(), size, mSensorIndex);

    return MTRUE;
}

class MYEnvironment final: public testing::Environment
{

public:
    MYEnvironment()
    : mMySensor{new MySensor()} { }

public:
    static MUINT32 getMain1Index() {return gMain1Index; }
    static MUINT32 getMain2Index() {return gMain2Index; }

public:
    void SetUp()    override;
    void TearDown() override;

private:
    void initHalSensor();

private:
    static MUINT32 gMain1Index;
    static MUINT32 gMain2Index;

    std::unique_ptr<MySensor> mMySensor;
};
MUINT32 MYEnvironment::gMain1Index = -1;
MUINT32 MYEnvironment::gMain2Index = -1;

void
MYEnvironment::
SetUp()
{
    mMySensor->init();
    gMain1Index = mMySensor->getMain1Index();
    gMain2Index = mMySensor->getMain2Index();
}

void
MYEnvironment::
TearDown()
{
    // I don't know what, we need in this function to destroy the MYSensor object
    mMySensor = nullptr;
}

int main(int argc, char **argv)
{
    testing::AddGlobalTestEnvironment(new MYEnvironment());
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(DISABLE_PiprTest, InitAndUnint)
{
    const MUINT32 main1Index = MYEnvironment::getMain1Index();

    ICaptureFeaturePipe::UsageHint usage = ICaptureFeaturePipe::UsageHint();
    sp<ICaptureFeaturePipe> pipePtr = ICaptureFeaturePipe::createInstance(main1Index, usage);
    UT_LOGD("create pipe, main1Index:%u", main1Index);

    pipePtr->init();
    sp<TestCallback> callback = new TestCallback();
    pipePtr->setCallback(callback);

    pipePtr->uninit();
    pipePtr = nullptr;

    SUCCEED();
}

TEST(FeatureTest, BokehBasicTest)
{
    SCOPED_TRACER();

    const char* name = "BokehBasicTest";
    const MUINT32 main1Index = MYEnvironment::getMain1Index();
    ICaptureFeaturePipe::UsageHint usage = ICaptureFeaturePipe::UsageHint();
    sp<ICaptureFeaturePipe> pipePtr = ICaptureFeaturePipe::createInstance(main1Index, usage);
    UT_LOGD("create pipe, main1Index:%u", main1Index);

    pipePtr->init();
    sp<TestCallback> callback = new TestCallback();
    pipePtr->setCallback(callback);

    sp<ICaptureFeatureRequest> requestPtr = pipePtr->acquireRequest();
    // add buffer
    requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle(name, eImgFmt_BAYER10, fsRawSize));
    requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle(name, eImgFmt_BAYER10, fsRawSize));
    requestPtr->addBuffer(BID_MAN_IN_LCS, new WorkingBufferHandle(name, eImgFmt_NV21, lcsSize));
    requestPtr->addBuffer(BID_SUB_IN_LCS, new WorkingBufferHandle(name, eImgFmt_NV21, lcsSize));
    requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle(name, eImgFmt_NV21, yuvSize));
    requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle(name, eImgFmt_YUY2, thumbSize));
    // add metadata
    requestPtr->addMetadata(MID_MAN_IN_APP, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_MAN_IN_HAL, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_MAN_IN_P1_DYNAMIC, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_SUB_IN_APP, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_SUB_IN_HAL, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_SUB_IN_P1_DYNAMIC, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_MAN_OUT_APP, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_MAN_OUT_HAL, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_SUB_OUT_APP, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_SUB_OUT_HAL, new WorkingMetadataHandle());
    // add feature
    requestPtr->addFeature(FID_BOKEH);
    requestPtr->addFeature(FID_DEPTH);

    pipePtr->enque(requestPtr);
    pipePtr->releaseRequest(requestPtr);

    const std::chrono::milliseconds waitTime(10000);
    const MINT32 desiredCompletedCount = 1;
    MUINT32 actualCompletedCount = callback->waitForDesiredCount(desiredCompletedCount, waitTime);

    ASSERT_EQ(desiredCompletedCount, actualCompletedCount)
    << "actual/desired completed count: "
    << actualCompletedCount << "/"
    << desiredCompletedCount;

    {
        SCOPED_TRACER_NAME("unit_pipe");
        pipePtr->uninit();
        pipePtr = nullptr;
    }

    SUCCEED();
}

TEST(FeatureTest, FusionBasicTest)
{
    SCOPED_TRACER();

    const char* name = "FusionBasicTest";
    const MUINT32 main1Index = MYEnvironment::getMain1Index();
    ICaptureFeaturePipe::UsageHint usage = ICaptureFeaturePipe::UsageHint();
    sp<ICaptureFeaturePipe> pipePtr = ICaptureFeaturePipe::createInstance(main1Index, usage);
    UT_LOGD("create pipe, main1Index:%u", main1Index);

    pipePtr->init();
    sp<TestCallback> callback = new TestCallback();
    pipePtr->setCallback(callback);

    sp<ICaptureFeatureRequest> requestPtr = pipePtr->acquireRequest();
    // add buffer
    requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle(name, eImgFmt_BAYER10, fsRawSize));
    requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle(name, eImgFmt_BAYER10, fsRawSize));
    //requestPtr->addBuffer(BID_MAN_OUT_DEPTH, new WorkingBufferHandle(name, eImgFmt_Y8, depthSize));
    requestPtr->addBuffer(BID_MAN_IN_LCS, new WorkingBufferHandle(name, eImgFmt_NV21, lcsSize));
    requestPtr->addBuffer(BID_SUB_IN_LCS, new WorkingBufferHandle(name, eImgFmt_NV21, lcsSize));
    requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle(name, eImgFmt_NV21, yuvSize));
    requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle(name, eImgFmt_YUY2, thumbSize));
    // add metadata
    requestPtr->addMetadata(MID_MAN_IN_APP, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_MAN_IN_HAL, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_MAN_IN_P1_DYNAMIC, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_SUB_IN_APP, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_SUB_IN_HAL, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_SUB_IN_P1_DYNAMIC, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_MAN_OUT_APP, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_MAN_OUT_HAL, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_SUB_OUT_APP, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_SUB_OUT_HAL, new WorkingMetadataHandle());
    // add feature
    requestPtr->addFeature(FID_FUSION);

    pipePtr->enque(requestPtr);
    pipePtr->releaseRequest(requestPtr);

    const std::chrono::milliseconds waitTime(10000);
    const MINT32 desiredCompletedCount = 1;

    MUINT32 actualCompletedCount = callback->waitForDesiredCount(desiredCompletedCount, waitTime);

    ASSERT_EQ(desiredCompletedCount, actualCompletedCount)
    << "actual/desired completed count: "
    << actualCompletedCount << "/"
    << desiredCompletedCount;

    {
        SCOPED_TRACER_NAME("unit_pipe");
        pipePtr->uninit();
        pipePtr = nullptr;
    }

    SUCCEED();
}


TEST(FeatureTest, SWNRBasicTest)
{
    const char* name = "SWNRBasicTest";
    const MUINT32 main1Index = MYEnvironment::getMain1Index();
    ICaptureFeaturePipe::UsageHint usage = ICaptureFeaturePipe::UsageHint();
    sp<ICaptureFeaturePipe> pipePtr = ICaptureFeaturePipe::createInstance(main1Index, usage);
    UT_LOGD("create pipe, main1Index:%u", main1Index);

    pipePtr->init();
    sp<TestCallback> callback = new TestCallback();
    pipePtr->setCallback(callback);

    sp<ICaptureFeatureRequest> requestPtr = pipePtr->acquireRequest();
    // add buffer
    requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle(name, eImgFmt_BAYER10, fsRawSize, sMain1FSFilename));
    requestPtr->addBuffer(BID_MAN_IN_LCS, new WorkingBufferHandle(name, eImgFmt_NV21, lcsSize, sLCSO1Filename));
    requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle(name, eImgFmt_NV21, yuvSize));
    requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle(name, eImgFmt_YUY2, thumbSize));
    // add metadata
    requestPtr->addMetadata(MID_MAN_IN_APP, new WorkingMetadataHandle());
    requestPtr->addMetadata(MID_MAN_IN_HAL, new WorkingMetadataHandle());
    // add feature
    requestPtr->addFeature(FID_SWNR);

    pipePtr->enque(requestPtr);
    pipePtr->releaseRequest(requestPtr);

    const std::chrono::milliseconds waitTime(10000);
    const MINT32 desiredCompletedCount = 1;

    MUINT32 actualCompletedCount = callback->waitForDesiredCount(desiredCompletedCount, waitTime);

    ASSERT_EQ(desiredCompletedCount, actualCompletedCount)
    << "actual/desired completed count: "
    << actualCompletedCount << "/"
    << desiredCompletedCount;

    {
        SCOPED_TRACER_NAME("unit_pipe");
        pipePtr->uninit();
        pipePtr = nullptr;
    }

    SUCCEED();
}



// int main2(int argc, char **argv)
// {
//     // 1. power on sensor
//     NSCam::IHalSensor *sensor = NULL;
//     MUINT32 index[] = { 0, 1 };
//     const MUINT32 size = sizeof(index)/sizeof(index[0]);
//     sensor = powerOnCamSensor(index, size);



//     // 2. create feature pipe
//     ICaptureFeaturePipe::UsageHint usage = ICaptureFeaturePipe::UsageHint();
//     sp<ICaptureFeaturePipe> pPipe = ICaptureFeaturePipe::createInstance(0, usage);

//     pPipe->init();

//     sp<RequestCallback> callback = new TestCallback();

//     pPipe->setCallback(callback);
//     sp<ICaptureFeatureRequest> requestPtr;

// #if 0
//     // MFNR Frame 1
//     requestPtr = pPipe->acquireRequest();
//     requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, fsRawSize, sMain1FSFilename));
//     requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, fsRawSize, sMain2FSFilename));
//     requestPtr->addBuffer(BID_MAN_IN_LCS, new WorkingBufferHandle("Tester", eImgFmt_NV21, lcsSize, sLCSO1Filename));
//     requestPtr->addBuffer(BID_SUB_IN_LCS, new WorkingBufferHandle("Tester", eImgFmt_NV21, lcsSize, sLCSO2Filename));
//     requestPtr->addBuffer(BID_MAN_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, rsRawSize, sMain1RSFilename));
//     requestPtr->addBuffer(BID_SUB_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, rsRawSize, sMain2RSFilename));
//     requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle("Tester", eImgFmt_NV21, yuvSize));
//     requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle("Tester", eImgFmt_NV21, thumbSize));
//     requestPtr->addMetadata(MID_MAN_IN_APP, new WorkingMetadataHandle());
//     requestPtr->addMetadata(MID_MAN_IN_HAL, new WorkingMetadataHandle());
//     requestPtr->addParameter(PID_ENABLE_MFB, 1);
//     requestPtr->addParameter(PID_FRAME_INDEX, 0);
//     requestPtr->addParameter(PID_FRAME_COUNT, 3);
//     requestPtr->addFeature(FID_SWNR);
//     requestPtr->addFeature(FID_MFNR);
//     pPipe->enque(requestPtr);
//     pPipe->releaseRequest(requestPtr);

//     // MFNR Frame 2
//     requestPtr = pPipe->acquireRequest();
//     requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, fsRawSize, sMain1FSFilename));
//     requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, fsRawSize, sMain2FSFilename));
//     requestPtr->addBuffer(BID_MAN_IN_LCS, new WorkingBufferHandle("Tester", eImgFmt_NV21, lcsSize, sLCSO1Filename));
//     requestPtr->addBuffer(BID_SUB_IN_LCS, new WorkingBufferHandle("Tester", eImgFmt_NV21, lcsSize, sLCSO2Filename));
//     requestPtr->addBuffer(BID_MAN_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, rsRawSize, sMain1RSFilename));
//     requestPtr->addBuffer(BID_SUB_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, rsRawSize, sMain2RSFilename));
//     requestPtr->addMetadata(MID_MAN_IN_APP, new WorkingMetadataHandle());
//     requestPtr->addMetadata(MID_MAN_IN_HAL, new WorkingMetadataHandle());

//     requestPtr->addParameter(PID_ENABLE_MFB, 1);
//     requestPtr->addParameter(PID_FRAME_INDEX, 1);
//     requestPtr->addParameter(PID_FRAME_COUNT, 3);
//     requestPtr->addFeature(FID_MFNR);
//     pPipe->enque(requestPtr);
//     pPipe->releaseRequest(requestPtr);

//     // MFNR Frame 3
//     requestPtr = pPipe->acquireRequest();
//     requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, fsRawSize, sMain1FSFilename));
//     requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, fsRawSize, sMain2FSFilename));
//     requestPtr->addBuffer(BID_MAN_IN_LCS, new WorkingBufferHandle("Tester", eImgFmt_NV21, lcsSize, sLCSO1Filename));
//     requestPtr->addBuffer(BID_SUB_IN_LCS, new WorkingBufferHandle("Tester", eImgFmt_NV21, lcsSize, sLCSO2Filename));
//     requestPtr->addBuffer(BID_MAN_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, rsRawSize, sMain1RSFilename));
//     requestPtr->addBuffer(BID_SUB_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, rsRawSize, sMain2RSFilename));
//     requestPtr->addMetadata(MID_MAN_IN_APP, new WorkingMetadataHandle());
//     requestPtr->addMetadata(MID_MAN_IN_HAL, new WorkingMetadataHandle());

//     requestPtr->addParameter(PID_ENABLE_MFB, 1);
//     requestPtr->addParameter(PID_FRAME_INDEX, 2);
//     requestPtr->addParameter(PID_FRAME_COUNT, 3);
//     requestPtr->addFeature(FID_MFNR);
//     pPipe->enque(requestPtr);
//     pPipe->releaseRequest(requestPtr);

// #endif

// #if 1
//     requestPtr = pPipe->acquireRequest();
//     requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_BAYER10, fsRawSize, sMain1FSFilename));
//     requestPtr->addBuffer(BID_MAN_IN_LCS, new WorkingBufferHandle("Tester", eImgFmt_NV21, lcsSize, sLCSO1Filename));
//     requestPtr->addMetadata(MID_MAN_IN_APP, new WorkingMetadataHandle());
//     requestPtr->addMetadata(MID_MAN_IN_HAL, new WorkingMetadataHandle());

//     requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle("Tester", eImgFmt_NV21, yuvSize));
//     requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle("Tester", eImgFmt_YUY2, thumbSize));
//     requestPtr->addFeature(FID_SWNR);
//     pPipe->enque(requestPtr);
//     pPipe->releaseRequest(requestPtr);
// #endif
// #if 0
//     // 3. test MFNR + SWNR + BOKEH
//     requestPtr = pPipe->acquireRequest();
//     requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(320,180)));

//     requestPtr->addFeature(FID_DEPTH);
//     requestPtr->addFeature(FID_BOKEH);
//     requestPtr->addFeature(FID_SWNR);
//     requestPtr->addFeature(FID_MFNR);
//     pPipe->enque(requestPtr);
//     pPipe->releaseRequest(requestPtr);


//     // 3. test RAW-HDR + SWNR + BOKEH
//     requestPtr = pPipe->acquireRequest();
//     requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(320,180)));

//     requestPtr->addFeature(FID_DEPTH);
//     requestPtr->addFeature(FID_BOKEH);
//     requestPtr->addFeature(FID_SWNR);
//     requestPtr->addFeature(FID_HDR);
//     pPipe->enque(requestPtr);
//     pPipe->releaseRequest(requestPtr);

//     // 3. test SWNR + BOKEH
//     requestPtr = pPipe->acquireRequest();
//     requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(320,180)));

//     requestPtr->addFeature(FID_DEPTH);
//     requestPtr->addFeature(FID_BOKEH);
//     requestPtr->addFeature(FID_SWNR);
//     pPipe->enque(requestPtr);
//     pPipe->releaseRequest(requestPtr);

//     // 4. test BOKEH
//     requestPtr = pPipe->acquireRequest();
//     requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(320,180)));

//     requestPtr->addFeature(FID_DEPTH);
//     requestPtr->addFeature(FID_BOKEH);
//     pPipe->enque(requestPtr);
//     pPipe->releaseRequest(requestPtr);


//     // 5. test MFNR + SWNR

//     requestPtr = pPipe->acquireRequest();
//     requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(320,180)));

//     requestPtr->addFeature(FID_MFNR);
//     requestPtr->addFeature(FID_MFNR);
//     pPipe->enque(requestPtr);
//     pPipe->releaseRequest(requestPtr);


//     // 6. test MFNR
//     requestPtr = pPipe->acquireRequest();
//     requestPtr->addBuffer(BID_MAN_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_FULL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_SUB_IN_RSZ, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_JPEG, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(1920,1080)));
//     requestPtr->addBuffer(BID_MAN_OUT_THUMBNAIL, new WorkingBufferHandle("Tester", eImgFmt_NV21, MSize(320,180)));

//     requestPtr->addFeature(FID_SWNR);
//     pPipe->enque(requestPtr);
//     pPipe->releaseRequest(requestPtr);

// #endif
//     sleep(1);

//     // x. power off
//     powerOffCamSensor(sensor, index, size);

//     return 0;
// }
