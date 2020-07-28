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

#include <vector>
#include <functional>
#include <atomic>
#include <random>
#include <thread>

#include <gtest/gtest.h>

#include <utils/String8.h>
#include <utils/Vector.h>
#include <utils/String8.h>
#include <cutils/properties.h>

#include <mtkcam/def/ImageFormat.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>

#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/pipe/IDualCamMultiFramePipe.h>
#include <mtkcam/aaa/IHal3A.h>

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>

#include "../DualCamMFPipe_Common.h"

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NSCam::NSCamFeature::NSFeaturePipe::NSDCMF;

//********************************************
// DCMFEnvironment
//********************************************
class DCMFEnvironment : public testing::Environment
{
public:
    virtual void SetUp();
    virtual void TearDown();

private:
    IHalSensor* HalSensorInit();
    void        HalSensorUninit();
    void        cam1DeviceInit();
    void        flowControlInit();

 private:
    IHalSensor* mpHalSensor;
};
//********************************************
// Utility
//********************************************
class Utility
{
 public:
    static void periodicCall(std::chrono::seconds totalTime, std::function<void(std::chrono::seconds waitedTime)> func);

    template<typename T>
    static T getRandomNumber(T a, T b);

    static IImageBuffer* createImageBufferFromFile(const IImageBufferAllocator::ImgParam imgParam, const char* path, const char* name, MINT usage);
    static IImageBuffer* createEmptyImageBuffer(const IImageBufferAllocator::ImgParam imgParam, const char* name, MINT usage);
};

template<typename T>
T Utility::getRandomNumber(T a, T b)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_int_distribution<T> dis(a, b);
    return dis(gen);
}
//********************************************
// DCMFPipeDataMock
//********************************************
struct DCMFPipeDataMock
{
public:
    static const DCMFPipeDataMock& getInstance()
    {
        static DCMFPipeDataMock instance;
        return instance;
    };

public:
    DCMFPipeDataMock(DCMFPipeDataMock const&)   = delete;
    void operator=(DCMFPipeDataMock const&)     = delete;

public:
    // out
    sp<IImageBuffer> mpOutFSYUVMain1ImgBuf     = nullptr;
    sp<IImageBuffer> mpOutFSYUVMain2ImgBuf     = nullptr;
    sp<IImageBuffer> mpOutRSYUVMain1ImgBuf     = nullptr;
    sp<IImageBuffer> mpOutRSYUVMain2ImgBuf     = nullptr;
    sp<IImageBuffer> mpOutYUVPostviewImgBuf    = nullptr;

    // in
    sp<IImageBuffer> mpInFSMain1ImgBuf  = nullptr;
    sp<IImageBuffer> mpInFSMain2ImgBuf  = nullptr;
    sp<IImageBuffer> mpInRSMain1ImgBuf  = nullptr;
    sp<IImageBuffer> mpInRSMain2ImgBuf  = nullptr;
    sp<IImageBuffer> mpInLCSO1ImgBuf    = nullptr;
    sp<IImageBuffer> mpInLCSO2ImgBuf    = nullptr;

private:
    DCMFPipeDataMock()
    {
        initImgBuf();
    };

    void initImgBuf();

    constexpr static char * sMain1FSFilename = {"/sdcard/DCMF/test/BID_INPUT_FSRAW_1_4208x3120_5260_3.raw"};
    constexpr static char * sMain2FSFilename = {"/sdcard/DCMF/test/BID_INPUT_FSRAW_2_4208x3120_5260_3.raw"};
    constexpr static char * sMain1RSFilename = {"/sdcard/DCMF/test/BID_INPUT_RSRAW_1_1920x1248_3600_0.raw"};
    constexpr static char * sMain2RSFilename = {"/sdcard/DCMF/test/BID_INPUT_RSRAW_2_1920x1248_3600_0.raw"};
    constexpr static char * sLCSO1Filename   = {"/sdcard/DCMF/test/BID_LCSO_1_BUF_384x384_768.yuv"};
    constexpr static char * sLCSO2Filename   = {"/sdcard/DCMF/test/BID_LCSO_2_BUF_384x384_768.yuv"};
};
//********************************************
// DualCamMFPipeFactory
//********************************************
class DualCamMFPipeFactory
{
public:
    static sp<IDualCamMFPipe> createDualCamMFPipe()
    {
        return innerCreateDualCamMFPipe(DualCamMFType::TYPE_DCMF_3rdParty);
    }

private:
    static sp<IDualCamMFPipe> innerCreateDualCamMFPipe(DualCamMFType type);
};
//********************************************
// IDCMFRequestCreater
//********************************************
class IDCMFRequestCreater : public virtual android::RefBase
{
public:
    virtual std::vector<sp<PipeRequest>> createRequest(sp<IDualCamMFPipeCB> cb) = 0;
    virtual ~IDCMFRequestCreater() {};
};
//********************************************
// DualCamMFPipeCBHandlerBase
//********************************************
class DualCamMFPipeCBHandlerBase : public virtual IDualCamMFPipeCB
{
public:
    DualCamMFPipeCBHandlerBase() : mCompletedCount(0) { }

    MUINT32 getReceivedEventCount() { return mCompletedCount; }
    MUINT32 waitForDesiredCount(MUINT32 desiredCount, std::chrono::milliseconds timeout);

protected:
    string toString(IDualCamMFPipeCB::EventId eventType);

protected:
    std::atomic<MUINT32> mCompletedCount;
};
//********************************************
// StressTestEventCBHandler
//********************************************
class StressTestEventCBHandler : public DualCamMFPipeCBHandlerBase
{
    virtual android::status_t onEvent(EventId eventType, sp<PipeRequest>& request, void* param1 = NULL, void* param2 = NULL) override;
};
//********************************************
// FlushTestEventCBHandler
//********************************************
class FlushTestEventCBHandler : public DualCamMFPipeCBHandlerBase
{
    virtual android::status_t onEvent(EventId eventType, sp<PipeRequest>& request, void* param1 = NULL, void* param2 = NULL) override;
};
//********************************************