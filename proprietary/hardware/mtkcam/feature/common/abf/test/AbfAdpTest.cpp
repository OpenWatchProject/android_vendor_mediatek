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

#define LOG_TAG "ABF/Test"

#include <memory>
#include <future>
#include <utility>
#include <mutex>
#include <gtest/gtest.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Format.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
#include "../AbfAdapterUT.h"


using namespace android;
using namespace NSCam;


static sp<IImageBufferHeap> s_bufferCache(NULL);
static std::mutex s_bufferMutex; // To protect buffer singleton


// This function was copied from NRvendor::allocBuffer() then modified
static status_t allocBuffer(
        MSize const& size,
        EImageFormat const& format,
        sp<IImageBufferHeap>& rpHeap,
        bool useSingleton = true)
{
    std::lock_guard<std::mutex> _lock(s_bufferMutex);

    if (useSingleton && s_bufferCache.get() != NULL) {
        if (s_bufferCache->getImgSize() == size &&
            s_bufferCache->getImgFormat() == format)
        {
            rpHeap = s_bufferCache;
            return OK;
        }
    }

    // query format
    MUINT32 plane = NSCam::Utils::Format::queryPlaneCount(format);
    size_t bufBoundaryInBytes[3] = {0, 0, 0};
    size_t bufStridesInBytes[3] = {0};

    for (MUINT32 i = 0; i < plane; i++) {
        bufStridesInBytes[i] = NSCam::Utils::Format::queryPlaneWidthInPixels(format, i, size.w) *
                               NSCam::Utils::Format::queryPlaneBitsPerPixel(format, i) / 8;
    }

    IImageBufferAllocator::ImgParam imgParam =
            IImageBufferAllocator::ImgParam(
                    (EImageFormat) format,
                    size, bufStridesInBytes,
                    bufBoundaryInBytes, plane);

    rpHeap = IIonImageBufferHeap::create("ABF/UT", imgParam);

    if (useSingleton && rpHeap.get() != NULL) {
        s_bufferCache = rpHeap;
    }

    return rpHeap.get()!=NULL ? OK : NO_MEMORY;
}


// RAII : to lockBuf() & unlockBuf() automatically
class IImageBufferLock
{
private:
    sp<IImageBufferHeap> mBufferHeap;
    IImageBuffer *mBuffer;
    int lockedCount;

public:
    IImageBufferLock(sp<IImageBufferHeap> &bufferHeap, bool t_lock = true) : mBufferHeap(bufferHeap) {
        mBuffer = mBufferHeap->createImageBuffer();
        lockedCount = 0;
        if (t_lock)
            lock();
    }

    ~IImageBufferLock() {
        unlock();
    }

    void lock() {
        if (lockedCount == 0) {
            MINT usage = eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN;
            mBuffer->lockBuf("ABF/UT", usage);
        }
        lockedCount++;
    }

    void unlock() {
        if (lockedCount == 1) {
            mBuffer->unlockBuf("ABF/UT");
        }
        lockedCount--;
    }

    IImageBuffer *get() {
        return mBuffer;
    }
};


static AbfAdapter::ProcessParam getValidParam()
{
    AbfAdapter::ProcessParam param;
    param.iso = 100;
    return param;
}


// Our mock works?
TEST(TestAbfAdapter, BasicMock)
{
    sp<AbfAdapterUT> abf(new AbfAdapterUT(0));
    abf->setMock(MTKABFMock::createInstance());

    sp<IImageBufferHeap> imageHeap;
    status_t allocStatus = allocBuffer(MSize(1024, 1024), eImgFmt_I420, imageHeap);
    ASSERT_EQ(OK, allocStatus);
    IImageBufferLock buffer(imageHeap);

    AbfAdapter::ProcessParam param = getValidParam();
    if (abf->needAbf(param)) {
        EXPECT_TRUE(abf->process(param, buffer.get()));
    } else {
        EXPECT_FALSE(abf->process(param, buffer.get()));
    }
}


// MTKABF only supports I420
TEST(TestAbfAdapter, UnsupportedFormat)
{
    sp<AbfAdapterUT> abf(new AbfAdapterUT(0));
    abf->setMock(MTKABFMock::createInstance());

    sp<IImageBufferHeap> imageHeap;
    status_t allocStatus = allocBuffer(MSize(1024, 1024), eImgFmt_YUY2, imageHeap);
    ASSERT_EQ(OK, allocStatus);
    IImageBufferLock buffer(imageHeap);

    AbfAdapter::ProcessParam param = getValidParam();
    ASSERT_EQ(true, abf->needAbf(param));
    EXPECT_FALSE(abf->process(param, buffer.get()));
}


#if 0
// MTKABF doesn't support stride
TEST(TestAbfAdapter, UnsupportedStride)
{
    sp<AbfAdapterUT> abf(new AbfAdapterUT(0));
    abf->setMock(MTKABFMock::createInstance());

    sp<IImageBufferHeap> imageHeap;
    status_t allocStatus = allocBuffer(MSize(1022, 1024), eImgFmt_I420, imageHeap); // 1022
    ASSERT_EQ(OK, allocStatus);
    IImageBufferLock buffer(imageHeap);

    AbfAdapter::ProcessParam param = getValidParam();
    ASSERT_EQ(true, abf->needAbf(param));
    EXPECT_FALSE(abf->process(param, buffer.get()));
}
#endif


// ABF_FEATURE_CHECK_ENABLE return false
TEST(TestAbfAdapter, NoNeedAbf)
{
    sp<AbfAdapterUT> abf(new AbfAdapterUT(0));

    class MTKABFNoNeed : public MTKABFMock {
    public:
        virtual MRESULT doABFFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut) {
            if (FeatureID == ABF_FEATURE_CHECK_ENABLE) {
                MINT32 *abfSwitch = static_cast<MINT32*>(pParaOut);
                *abfSwitch = 0;
                return S_ABF_OK;
            }
            return MTKABFMock::doABFFeatureCtrl(FeatureID, pParaIn, pParaOut);
        }
    };
    abf->setMock(new MTKABFNoNeed());

    AbfAdapter::ProcessParam param = getValidParam();
    EXPECT_FALSE(abf->needAbf(param));
}


// needABF() return true but process() failed
TEST(TestAbfAdapter, MainFail)
{
    sp<AbfAdapterUT> abf(new AbfAdapterUT(0));

    class MTKABFMainFail : public MTKABFMock {
    public:
        virtual MRESULT ABFMain() {
            MTKABFMock::ABFMain();
            return E_ABF_ERR;
        }
    };
    abf->setMock(new MTKABFMainFail());

    sp<IImageBufferHeap> imageHeap;
    status_t allocStatus = allocBuffer(MSize(1024, 1024), eImgFmt_I420, imageHeap);
    ASSERT_EQ(OK, allocStatus);
    IImageBufferLock buffer(imageHeap);

    AbfAdapter::ProcessParam param = getValidParam();
    EXPECT_TRUE(abf->needAbf(param));
    EXPECT_FALSE(abf->process(param, buffer.get()));
}


// Test normal flow
TEST(TestAbfAdapter, MainSuccess)
{
    sp<AbfAdapterUT> abf(new AbfAdapterUT(0));

    class MTKABFMainDraw : public MTKABFMock {
    public:
        virtual MRESULT ABFMain() {
            MUINT32 stride =
                NSCam::Utils::Format::queryPlaneWidthInPixels(eImgFmt_I420, 0, mResult.Width) *
                NSCam::Utils::Format::queryPlaneBitsPerPixel(eImgFmt_I420, 0) / 8;

            // Draw a black line on Y plane
            char *pixelData = reinterpret_cast<char*>(mResult.pImg[0]);
            char *linePos = pixelData + stride * mResult.Height;
            memset(linePos, 0, stride);

            return MTKABFMock::ABFMain();
        }
    };
    abf->setMock(new MTKABFMainDraw());

    sp<IImageBufferHeap> imageHeap;
    status_t allocStatus = allocBuffer(MSize(1024, 1024), eImgFmt_I420, imageHeap);
    ASSERT_EQ(OK, allocStatus);
    IImageBufferLock buffer(imageHeap);

    AbfAdapter::ProcessParam param = getValidParam();
    EXPECT_TRUE(abf->needAbf(param));
    EXPECT_TRUE(abf->process(param, buffer.get()));
}


// Repeatly run to try to detect leaks
TEST(TestAbfAdapter, Repeat)
{
    sp<AbfAdapterUT> abf(new AbfAdapterUT(0));
    abf->setMock(MTKABFMock::createInstance());

    sp<IImageBufferHeap> imageHeap;
    status_t allocStatus = allocBuffer(MSize(1024, 1024), eImgFmt_I420, imageHeap);
    ASSERT_EQ(OK, allocStatus);
    IImageBufferLock buffer(imageHeap);

    AbfAdapter::ProcessParam param = getValidParam();
    for (int i = 0; i < 100; i++) {
        if (!abf->needAbf(param) ||
            !abf->process(param, buffer.get()))
        {
            GTEST_FAIL();
            break;
        }
    }
}


// AbfAdapter + real MTKABF
TEST(TestAbfIT, Basic)
{
    sp<AbfAdapterUT> abf(new AbfAdapterUT(0));

    sp<IImageBufferHeap> imageHeap;
    status_t allocStatus = allocBuffer(MSize(1024, 1024), eImgFmt_I420, imageHeap);
    ASSERT_EQ(OK, allocStatus);
    IImageBufferLock buffer(imageHeap);

    AbfAdapter::ProcessParam param = getValidParam();

    EXPECT_TRUE(abf->needAbf(param));
    EXPECT_TRUE(abf->process(param, buffer.get()));

    EXPECT_TRUE(abf->needAbf(param));
    EXPECT_TRUE(abf->process(param, buffer.get()));
}


// We create multiple threads to operate AbfAdapter
// to simulate multiple instance (e.g. dual cam) case
TEST(TestAbfIT, MultiInstance)
{
    std::promise<void> goPromise;
    auto goSignal = goPromise.get_future().share();

    auto repeat = [goSignal] {
        sp<AbfAdapterUT> abf(new AbfAdapterUT(0));

        sp<IImageBufferHeap> imageHeap;
        status_t allocStatus = allocBuffer(MSize(1024, 1024), eImgFmt_I420, imageHeap, false);
        ASSERT_EQ(OK, allocStatus);
        IImageBufferLock buffer(imageHeap);

        AbfAdapter::ProcessParam param = getValidParam();

        goSignal.wait();

        for (int i = 0; i < 10; i++) {
            if (!abf->needAbf(param) ||
                !abf->process(param, buffer.get()))
            {
                GTEST_FAIL();
                break;
            }
        }
    };

    std::future<void> fn[5];
    for (int i = 0; i < 5; i++) {
        fn[i] = std::async(std::launch::async, repeat);
    }

    goPromise.set_value();

    for (int i = 0; i < 5; i++) {
        fn[i].wait();
    }
}


// AbfAdapter + real MTKABF + given input
TEST(TestAbfIT, YuvInput)
{
    sp<AbfAdapterUT> abf(new AbfAdapterUT(0));

    sp<IImageBufferHeap> imageHeap;
    status_t allocStatus = allocBuffer(MSize(5344, 4016), eImgFmt_I420, imageHeap);
    ASSERT_EQ(OK, allocStatus);
    IImageBufferLock buffer(imageHeap, false);

    AbfAdapter::ProcessParam param = getValidParam();

    if (buffer.get()->loadFromFile("test-abf-in-5344x4016.yuv")) {
        buffer.lock();
        EXPECT_TRUE(abf->needAbf(param));
        EXPECT_TRUE(abf->process(param, buffer.get()));
        buffer.get()->saveToFile("test-abf-out-5344x4016.yuv");
    }
}


int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

