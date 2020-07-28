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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#include "TestDualCamMF_Common.h"

#define PIPE_MODULE_TAG "DualCamMFUnitTest"
#define PIPE_CLASS_TAG "Common"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG
#define USER_NAME PIPE_MODULE_TAG

#include "DualCamMF_UTLog.h"

// ***************************************************************
// DCMFEnvironment
// ***************************************************************
IHalSensor* DCMFEnvironment::HalSensorInit()
{
    MY_UTSCOPEDTRACE();

    NSCam::IHalSensorList* pHalSensorList = NSCam::IHalSensorList::get();
    EXPECT_TRUE(pHalSensorList != nullptr) << "failed to get the HAL sensor list.";

    pHalSensorList->searchSensors();
    int sensorCount = pHalSensorList->queryNumberOfSensors();

    int main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);
    MUINT pIndex[2] = { (MUINT)main1Idx, (MUINT)main2Idx };
    MUINT const main1Index = pIndex[0];
    MUINT const main2Index = pIndex[1];
    IHalSensor *mpHalSensor = pHalSensorList->createSensor( USER_NAME, 2, pIndex);
    EXPECT_TRUE(mpHalSensor != nullptr) << "failed to create HAL sensor.";

    // In stereo mode, Main1 needs power on first.
    // Power on main1 and main2 successively one after another.
    MY_UTLOG("power on main1 sensor.");
    EXPECT_TRUE(mpHalSensor->powerOn(USER_NAME, 1, &main1Index)) << "failed to open sensor power: " << main1Index;
    MY_UTLOG("power on main2 sensor.");
    EXPECT_TRUE(mpHalSensor->powerOn(USER_NAME, 1, &main2Index)) << "failed to open sensor power: " << main2Index;
    return mpHalSensor;
}

void DCMFEnvironment::SetUp()
{
    mpHalSensor = HalSensorInit();
    cam1DeviceInit();
    flowControlInit();
}

void DCMFEnvironment::TearDown()
{
    HalSensorUninit();
}

void DCMFEnvironment::HalSensorUninit()
{
    MY_UTSCOPEDTRACE();

    int main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);
    MUINT pIndex[2] = { (MUINT)main1Idx, (MUINT)main2Idx };
    MUINT const main1Index = pIndex[0];
    MUINT const main2Index = pIndex[1];

    MY_UTLOG("power off main1 sensor.");
    MUINT pIndex_main1[1] = { (MUINT)main1Index};
    mpHalSensor->powerOff(USER_NAME, 1, pIndex_main1);

    MY_UTLOG("power off main2 sensor.");
    MUINT pIndex_main2[1] = { (MUINT)main2Index};
    mpHalSensor->powerOff(USER_NAME, 1, pIndex_main2);

    mpHalSensor->destroyInstance(USER_NAME);
}

void DCMFEnvironment::cam1DeviceInit()
{
    StereoSettingProvider::setStereoProfile(STEREO_SENSOR_PROFILE_REAR_REAR);
}

void DCMFEnvironment::flowControlInit()
{
    StereoSettingProvider::setStereoFeatureMode(v1::Stereo::E_STEREO_FEATURE_DENOISE);
    StereoSettingProvider::setStereoModuleType(v1::Stereo::BAYER_AND_MONO);
    StereoSettingProvider::setImageRatio(eRatio_16_9);
}
// ***************************************************************
// Utility
// ***************************************************************
void Utility::periodicCall(std::chrono::seconds totalTime, std::function<void(std::chrono::seconds waitedTime)> func)
{
    const int waitCountForSeconds = totalTime.count();
    const std::chrono::seconds oneSecond(1);
    for(int i = 1; i <= totalTime.count(); i++)
    {
        std::this_thread::sleep_for(oneSecond);
        func(std::chrono::seconds(i));
    }
}

IImageBuffer* Utility::createImageBufferFromFile(const IImageBufferAllocator::ImgParam imgParam, const char* path, const char* name, MINT usage)
{
    IImageBufferAllocator *allocator = IImageBufferAllocator::getInstance();
    IImageBuffer* imgBuf = allocator->alloc(name, imgParam);
    EXPECT_TRUE(imgBuf->loadFromFile(path)) << "name: " << name << std::endl << "path: " << path;
    EXPECT_TRUE(imgBuf->lockBuf(name, usage)) << "name: " << name << std::endl << "usage: " << usage;
    return imgBuf;
}

IImageBuffer* Utility::createEmptyImageBuffer(const IImageBufferAllocator::ImgParam imgParam, const char* name, MINT usage)
{
    IImageBufferAllocator *allocator = IImageBufferAllocator::getInstance();
    IImageBuffer* imgBuf = allocator->alloc(name, imgParam);
    EXPECT_TRUE(imgBuf->lockBuf(name, usage)) << "name: " << name << std::endl << "usage: " << usage; ;
    return imgBuf;
}
// ***************************************************************
// DCMFPipeDataMock
// ***************************************************************
void DCMFPipeDataMock::initImgBuf()
{
    static const MUINT32 fsRawStride = 5260;
    static const MSize fsRawSize(4208, 3120);
    static const MUINT32 rsRawStride = 3600;
    static const MSize rsRawSize(1920, 1248);

    static const MSize yuvSize(4208, 3120);
    static const MSize yuvPostviewSize(1920, 1080);
    static const MSize thumb(160, 128);

    static const MUINT32 lcsStride = 768;
    static const MSize lcsSize(384, 384);
    // in: FS Main 1
    {
        MUINT32 bufStridesInBytes[3] = {fsRawStride, 0, 0};
        MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10), fsRawSize, bufStridesInBytes, bufBoundaryInBytes, 1);

        mpInFSMain1ImgBuf = Utility::createImageBufferFromFile(imgParam, sMain1FSFilename, "BID_INPUT_FSRAW_1",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        EXPECT_TRUE(mpInFSMain1ImgBuf != nullptr) << "failed to load BID_INPUT_FSRAW_1 image.";
    }
    // in: FS Main 2
    {
        MUINT32 bufStridesInBytes[3] = {fsRawStride, 0, 0};
        MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10), fsRawSize, bufStridesInBytes, bufBoundaryInBytes, 1);

        mpInFSMain2ImgBuf = Utility::createImageBufferFromFile(imgParam, sMain2FSFilename, "BID_INPUT_FSRAW_2",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        EXPECT_TRUE(mpInFSMain2ImgBuf != nullptr) << "failed to load BID_INPUT_FSRAW_2 image.";
    }
    // in: RS Main 1
    {
        MUINT32 bufStridesInBytes[3] = {rsRawStride, 0, 0};
        MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10), rsRawSize, bufStridesInBytes, bufBoundaryInBytes, 1);

        mpInRSMain1ImgBuf = Utility::createImageBufferFromFile(imgParam, sMain1RSFilename, "BID_INPUT_RSRAW_1",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        EXPECT_TRUE(mpInRSMain1ImgBuf != nullptr) << "failed to load BID_INPUT_RSRAW_1 image.";
    }
    // in: RS Main 2
    {
        MUINT32 bufStridesInBytes[3] = {rsRawStride, 0, 0};
        MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10), rsRawSize, bufStridesInBytes, bufBoundaryInBytes, 1);

        mpInRSMain2ImgBuf = Utility::createImageBufferFromFile(imgParam, sMain2RSFilename, "BID_INPUT_RSRAW_2",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        EXPECT_TRUE(mpInRSMain2ImgBuf != nullptr) << "failed to load BID_INPUT_RSRAW_2 image.";
    }
    // out: BID_FS_YUV_1
    {
        int STRIDES[2] = {yuvSize.w, yuvSize.w};
        int BOUNDARY[2] = {0, 0};
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam(eImgFmt_NV21, yuvSize, STRIDES, BOUNDARY, 2);

        mpOutFSYUVMain1ImgBuf = Utility::createEmptyImageBuffer(imgParam, "BID_FS_YUV_1",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        EXPECT_TRUE(mpOutFSYUVMain1ImgBuf != nullptr) << "failed to load BID_FS_YUV_1 image.";
    }
    // out: BID_FS_YUV_2
    {
        int STRIDES[2] = {yuvSize.w, yuvSize.w};
        int BOUNDARY[2] = {0, 0};
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam(eImgFmt_NV21, yuvSize, STRIDES, BOUNDARY, 2);

        mpOutFSYUVMain2ImgBuf = Utility::createEmptyImageBuffer(imgParam, "BID_FS_YUV_2",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        EXPECT_TRUE(mpOutFSYUVMain2ImgBuf != nullptr) << "failed to load BID_FS_YUV_2 image.";
    }
    // out: BID_RS_YUV_1
    {
        int STRIDES[2] = {yuvSize.w, yuvSize.w};
        int BOUNDARY[2] = {0, 0};
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam(eImgFmt_NV21, yuvSize, STRIDES, BOUNDARY, 2);

        mpOutRSYUVMain1ImgBuf = Utility::createEmptyImageBuffer(imgParam, "BID_RS_YUV_1",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        EXPECT_TRUE(mpOutRSYUVMain1ImgBuf != nullptr) << "failed to load BID_RS_YUV_1 image.";
    }
    // out: BID_RS_YUV_2
    {
        int STRIDES[2] = {yuvSize.w, yuvSize.w};
        int BOUNDARY[2] = {0, 0};
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam(eImgFmt_NV21, yuvSize, STRIDES, BOUNDARY, 2);

        mpOutRSYUVMain2ImgBuf = Utility::createEmptyImageBuffer(imgParam, "BID_RS_YUV_2",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        EXPECT_TRUE(mpOutRSYUVMain2ImgBuf != nullptr) << "failed to load BID_RS_YUV_2 image.";
    }
    // out: BID_POSTVIEW
    {
        int STRIDES[2] = {yuvPostviewSize.w, yuvPostviewSize.w};
        int BOUNDARY[2] = {0, 0};
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam(eImgFmt_NV21, yuvPostviewSize, STRIDES, BOUNDARY, 2);

        mpOutYUVPostviewImgBuf = Utility::createEmptyImageBuffer(imgParam, "BID_POSTVIEW",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        EXPECT_TRUE(mpOutYUVPostviewImgBuf != nullptr) << "failed to load BID_POSTVIEW image.";
    }
    // in: LCS_1mpOutYUVPostviewImgBuf
    {
        MUINT32 bufStridesInBytes[3] = {lcsStride, 0, 0};
        MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10), lcsSize, bufStridesInBytes, bufBoundaryInBytes, 1);

        mpInLCSO1ImgBuf = Utility::createImageBufferFromFile(imgParam, sLCSO1Filename, "BID_LCS_1",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        EXPECT_TRUE(mpInLCSO1ImgBuf != nullptr) << "failed to load BID_LCS_1 image.";
    }
    // in: LCS_2
    {
        MUINT32 bufStridesInBytes[3] = {lcsStride, 0, 0};
        MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10), lcsSize, bufStridesInBytes, bufBoundaryInBytes, 1);

        mpInLCSO2ImgBuf = Utility::createImageBufferFromFile(imgParam, sLCSO2Filename, "BID_LCS_2",  eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        EXPECT_TRUE(mpInLCSO2ImgBuf != nullptr) << "failed to load BID_LCS_1 image.";
    }
}
//********************************************
// DualCamMFPipeFactory
//********************************************
sp<IDualCamMFPipe> DualCamMFPipeFactory::innerCreateDualCamMFPipe(DualCamMFType type)
{
    MY_UTSCOPEDTRACE();

    int main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);

    sp<IDualCamMFPipe> DualCamMFPipe = IDualCamMFPipe::createInstance(main1Idx, main2Idx, type);
    DualCamMFPipe->setFlushOnStop(MTRUE);
    DualCamMFPipe->init();
    DualCamMFPipe->sync();

    const std::chrono::seconds totalTime(2);
    Utility::periodicCall(totalTime, [totalTime](std::chrono::seconds waitedTime)
    {
        MY_UTLOG("wait for sync after create pipe: %lld/ %lld sec..", waitedTime.count(), totalTime.count());
    });
    return DualCamMFPipe;
}
// **************************************************************
// DualCamMFPipeCBHandlerBase
// **************************************************************
MUINT32 DualCamMFPipeCBHandlerBase::waitForDesiredCount(MUINT32 desiredCount, std::chrono::milliseconds timeout)
{
    MY_UTSCOPEDTRACE();
    const auto startWaitTime = std::chrono::system_clock::now();
    std::chrono::milliseconds waitedTime(0);
    MUINT32 currentCount(0);
    while(true)
    {
        waitedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startWaitTime);
        currentCount = getReceivedEventCount();
        if(desiredCount == currentCount)
        {
            MY_UTLOG("wait for desired count completed, count: %d (%d), waitedTime: %lld (%lld) ms.", currentCount, desiredCount, waitedTime.count(), timeout.count());
            break;
        }
        else if(waitedTime.count() < timeout.count())
        {
            MY_UTLOG("wait for desired count, count: %d (%d), waitedTime: %lld (%lld) ms.", currentCount, desiredCount, waitedTime.count(), timeout.count());
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        else // wait timeout
        {
            MY_UTLOG("wait for desired count timeout, count: %d (%d), waitedTime: %lld (%lld) ms.", currentCount, desiredCount, waitedTime.count(), timeout.count());
            break;
        }
    }
    return currentCount;
}

string DualCamMFPipeCBHandlerBase::toString(IDualCamMFPipeCB::EventId eventType)
{
    switch(eventType)
    {
        case IDualCamMFPipeCB::eCompleted:
            return "eCompleted";
        case IDualCamMFPipeCB::eFailed:
            return "eFailed";
        case IDualCamMFPipeCB::eShutter:
            return "eShutter";
        case IDualCamMFPipeCB::eP2Done:
            return "eP2Done";
        case IDualCamMFPipeCB::eFlush:
            return "eFlush";
        case IDualCamMFPipeCB::ePostview:
            return "ePostview";
    }
    return "UNKNOWN";
}
//********************************************
// StressTestEventCBHandler
//********************************************
android::status_t StressTestEventCBHandler::onEvent(EventId eventType, sp<PipeRequest>& request, void* param1, void* param2)
{
    std::string eventName = toString(eventType);
    MY_UTLOG("receive req., #: %d, type: %s",  request->getRequestNo(), eventName.c_str());
    if(eventType == IDualCamMFPipeCB::eCompleted)
    {
        MUINT32 newCount = mCompletedCount++;
        MY_UTLOG("update complete count: %d (%s)", newCount, eventName.c_str());
    }
    return OK;
}
//********************************************
// FlushTestEventCBHandler
//********************************************
android::status_t FlushTestEventCBHandler::onEvent(EventId eventType, sp<PipeRequest>& request, void* param1, void* param2)
{
    std::string eventName = toString(eventType);
    MY_UTLOG("receive req., #: %d, type: %s",  request->getRequestNo(), eventName.c_str());
    switch(eventType)
    {
        case IDualCamMFPipeCB::eFlush:
        case IDualCamMFPipeCB::eCompleted:
            MUINT32 newCount = mCompletedCount++;
            MY_UTLOG("update complete count: %d (%s)", newCount, eventName.c_str());
        break;
    }
    return OK;
}
// **************************************************************