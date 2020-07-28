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

#define LOG_TAG "MtkCam/camtest/IonHeap"
//
#include <array>
#include <chrono>
#include <future>
#include <set>
#include <thread>
#include <vector>
//
#include <gtest/gtest.h>
//
#include "CamLog.h"
#include "TestUtils.h"
//
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
using namespace NSCam;

#define ALIGN(x, alignment) (((x) + ((alignment)-1)) & ~((alignment)-1))


/******************************************************************************
 *
 ******************************************************************************/
void
Test_IonImageBufferHeap()
{
    CAM_LOGD("++++++++++++++++ Test_IonImageBufferHeap ++++++++++++++++");
    //
    //  Heap Blob
    {
        MUINT32 size = 2048;
        IImageBufferAllocator::ImgParam imgParam(size, 0);
        IIonImageBufferHeap::AllocExtraParam extraParam;
        sp<IImageBufferHeap> pHeap = IIonImageBufferHeap::create("Test_IonImageBufferHeap", imgParam, extraParam);
        OBJ_CHECK(pHeap);
        //
        test_heap_blob(IIonImageBufferHeap::magicName(), pHeap, size);
    }
    //
    //  Heap YUV420 3P
    {
        size_t bufStridesInBytes[3] = {2000, 1920, 960};
        size_t bufBoundaryInBytes[3] = {0, 0, 0};
        //
        IImageBufferAllocator::ImgParam imgParam_fail(
            eImgFmt_JPEG, MSize(1920, 1080),
            bufStridesInBytes, bufBoundaryInBytes, 3
        );
        IImageBufferAllocator::ImgParam imgParam_pass(
            eImgFmt_YV12, MSize(1920, 1080),
            bufStridesInBytes, bufBoundaryInBytes, 3
        );
        //
        IIonImageBufferHeap::AllocExtraParam extraParam;
        sp<IImageBufferHeap> pHeap_fail = IIonImageBufferHeap::create("Test_IonImageBufferHeap", imgParam_fail, extraParam);
        MY_CHECK(pHeap_fail == 0, "can NOT create JPEG format heap..");
        //
        sp<IImageBufferHeap> pHeap_pass = IIonImageBufferHeap::create("Test_IonImageBufferHeap", imgParam_pass, extraParam);
        OBJ_CHECK(pHeap_pass);
        //
        test_heap_yuv420_3p(IIonImageBufferHeap::magicName(), pHeap_pass, imgParam_pass);
        //
    }
    //
    CAM_LOGD("---------------- Test_IonImageBufferHeap ----------------");
}



/******************************************************************************
 *
 ******************************************************************************/
class IonImageBufferHeapTest : public ::testing::Test
{
protected:

    char const                  mPatterns[3] = {0xAA, 0x55, 0xA5};

    std::vector<MINT32> const   mFormats{
                                    eImgFmt_JPEG,
                                    eImgFmt_BLOB,
                                    eImgFmt_Y8, eImgFmt_Y16,
                                    eImgFmt_YUY2,
                                    eImgFmt_NV21, eImgFmt_YV12,
                                    eImgFmt_BAYER8, eImgFmt_BAYER10, eImgFmt_BAYER12, eImgFmt_BAYER14,
                                };

    std::vector<MSize> const    mSizes{
                                    MSize( 176,  144),
                                    MSize( 640,  480),
                                    MSize(1280,  720),
                                    MSize(1920, 1080),
                                    MSize(1920, 1088),
                                    MSize(4032, 3024),
                                };

    std::vector<std::array<size_t, 3>> const
                                mBoundaries{
                                    {{ 0, 0, 0}},
                                    {{ 1, 1, 1}},
                                    {{(1<< 0), (1<< 6), (1<<12)}},
                                    {{(1<< 4), (1<< 8), (1<<12)}},
                                    {{(1<<12), (1<<12), (1<<12)}},
                                };

    std::vector<std::array<size_t, 3>> const
                                mBoundaries_Start0{
                                    {{ 0, 0, 0}},
                                    {{ 0, 1, 1}},
                                    {{ 0, (1<< 6), (1<<12)}},
                                    {{ 0, (1<< 8), (1<<12)}},
                                    {{ 0, (1<<12), (1<<12)}},
                                };

    std::vector<MBOOL> const    mContiguousPlanes{MFALSE, MTRUE};

protected:
                    IonImageBufferHeapTest()
                    {
                        // You can do set-up work for each test here.
                        //CAM_LOGD("[IonImageBufferHeapTest::%s]", __FUNCTION__);
                    }

    virtual         ~IonImageBufferHeapTest()
                    {
                        // You can do clean-up work that doesn't throw exception here.
                        //CAM_LOGD("[IonImageBufferHeapTest::%s]", __FUNCTION__);
                    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods.

    // Code here will be called immediately after the constructor (right before each test)
    virtual void    SetUp()
                    {
                        CAM_LOGD("[IonImageBufferHeapTest::%s]", __FUNCTION__);
                    }

    // Code here will be called immediately after each test (right before the destructor)
    virtual void    TearDown()
                    {
                        CAM_LOGD("[IonImageBufferHeapTest::%s]", __FUNCTION__);
                    }

    static MSize    convertToTestImgSize(int format, MSize const& size)
                    {
                        switch (format)
                        {
                        case eImgFmt_JPEG:
                        case eImgFmt_BLOB:
                            return MSize(size.w*size.h, 1);
                        default:
                            break;
                        }
                        //
                        return size;
                    }

    static std::array<size_t,3>
                    calculateTestStrides(int format, int imgWidth)
                    {
                        std::array<size_t,3> strides{{0}};
                        size_t planeCount = NSCam::Utils::Format::queryPlaneCount(format);
                        for (size_t i = 0; i < planeCount; i++) {
                            strides[i] = ALIGN((imgWidth * NSCam::Utils::Format::queryPlaneBitsPerPixel(format, i)), 8) >> 3;
                            EXPECT_TRUE(0!=strides[i]) << "format:" << format << " w:%d" << imgWidth <<" - stride=0 @ " << i << "-th plane";
                        }
                        return strides;
                    }

    static android::String8
                    toString(
                        int format,
                        MSize const& size,
                        std::array<size_t,3> const& strides,
                        std::array<size_t,3> const& boundary,
                        MBOOL const contiguousPlanes
                    )
                    {
                        size_t planeCount = NSCam::Utils::Format::queryPlaneCount(format);
                        return android::String8::format("%s %-4dx%d #plane:%zu contiguousPlanes:%d boundaries:[%zu %zu %zu] strides:[%zu %zu %zu]",
                            NSCam::Utils::Format::queryImageFormatName(format).c_str(),
                            size.w, size.h, planeCount, contiguousPlanes,
                            boundary[0], boundary[1], boundary[2],
                            strides[0], strides[1], strides[2]
                        );
                    }

    void            test(sp<IIonImageBufferHeap> pIonHeap, std::array<size_t,3> const& boundary, MBOOL const contiguousPlanes);

};


void
IonImageBufferHeapTest::
test(sp<IIonImageBufferHeap> pIonHeap, std::array<size_t,3> const& boundary, MBOOL const contiguousPlanes)
{
    int usage = 0;
    MBOOL ret = MFALSE;

    size_t const planeCount = pIonHeap->getPlaneCount();

    EXPECT_TRUE(pIonHeap->syncCache(eCACHECTRL_FLUSH));
    usage = eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE;
    ret = pIonHeap->lockBuf("IonImageBufferHeapTest", usage);
    EXPECT_TRUE(ret);
    if  (ret) {
        if  ( planeCount > 1 ) {
            android::String8 l = android::String8::format("Check contiguous planes: ");
            std::set<int> heapID;
            for (size_t i = 0; i < pIonHeap->getHeapIDCount(); i++) {
                heapID.insert( pIonHeap->getHeapID(i) );
            }
            if  ( contiguousPlanes ) {
                l += android::String8::format("contiguousPlanes:Y %zu==1", heapID.size());
                EXPECT_TRUE(heapID.size()==1) << l.c_str();
            }
            else {
                l += android::String8::format("contiguousPlanes:N %zu==%zu", heapID.size(), planeCount);
                EXPECT_TRUE(heapID.size()==planeCount) << l.c_str();
            }
            CAM_LOGD("%s", l.c_str());
        }

        for (size_t plane = 0; plane < planeCount; plane++) {
            auto const pa = pIonHeap->getBufPA(plane);
            auto const va = pIonHeap->getBufVA(plane);
            auto const sizeInBytes = pIonHeap->getBufSizeInBytes(plane);
            EXPECT_TRUE(0!=pa) << "pa:" << pa;
            EXPECT_TRUE(0!=va) << "va:" << va;
            if  (0!=va) {
                ::memset((void*)va, mPatterns[plane], sizeInBytes);
            }

            CAM_LOGD("  [%zu] PA:%" PRIxPTR " VA:%" PRIxPTR " boundary:%zu size:%zu", plane, pa, va, boundary[plane], sizeInBytes);
            if  (0 != boundary[plane]) {
                EXPECT_TRUE(0 == (pa % boundary[plane])) << "pa:" << pa << " is not aligned to " << boundary[plane];
                EXPECT_TRUE(0 == (va % boundary[plane])) << "va:" << va << " is not aligned to " << boundary[plane];
            }
        }
        pIonHeap->syncCache(eCACHECTRL_FLUSH);
        pIonHeap->syncCache(eCACHECTRL_INVALID);
        EXPECT_TRUE( pIonHeap->unlockBuf("IonImageBufferHeapTest") );
    }


    usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE;
    ret = pIonHeap->lockBuf("IonImageBufferHeapTest", usage);
    EXPECT_TRUE(ret);
    if  (ret) {
        for (size_t plane = 0; plane < planeCount; plane++) {
            auto const pa = pIonHeap->getBufPA(plane);
            auto const va = pIonHeap->getBufVA(plane);
            auto const sizeInBytes = pIonHeap->getBufSizeInBytes(plane);
            EXPECT_TRUE(0!=pa) << "pa:" << pa;
            EXPECT_TRUE(0!=va) << "va:" << va;

            MUINT8* pb = (MUINT8*)va;
            for (size_t i = 0; i < sizeInBytes; i++) {
                if  (mPatterns[plane] != pb[i]) {
                    android::String8 l = android::String8::format("[%zu] %#x(@%zu) != %#x", plane, pb[i], i, mPatterns[plane]);
                    EXPECT_TRUE(mPatterns[plane] == pb[i]) << l.c_str();
                    CAM_LOGE("%s", l.c_str());
                    break;
                }
            }
        }
        EXPECT_TRUE( pIonHeap->unlockBuf("IonImageBufferHeapTest") );
    }
}


TEST_F(IonImageBufferHeapTest, contiguousPlanes) {
    CAM_LOGI("@@IonImageBufferHeapTest::contiguousPlanes");

    for (auto const& format : mFormats) {

        auto const planeCount = NSCam::Utils::Format::queryPlaneCount(format);

        for (auto const& size : mSizes) {
            //
            MSize const imgSize = convertToTestImgSize(format, size);
            auto const strides = calculateTestStrides(format, imgSize.w);
            //
            for (auto const& boundary : mBoundaries) {

                for (auto const& contiguousPlanes : mContiguousPlanes) {
                    android::String8 log = toString(format, size, strides, boundary, contiguousPlanes);
                    CAM_LOGD("%s", log.c_str());

                    IIonImageBufferHeap::AllocImgParam_t imgParam{format, imgSize, strides.data(), boundary.data(), planeCount};
                    IIonImageBufferHeap::AllocExtraParam extParam;
                    extParam.contiguousPlanes = contiguousPlanes;

                    sp<IIonImageBufferHeap> pIonHeap = IIonImageBufferHeap::create("IonImageBufferHeapTest", imgParam, extParam, MTRUE);
                    ASSERT_TRUE(pIonHeap!=nullptr) << log.c_str();

                    test(pIonHeap, boundary, contiguousPlanes);

                    pIonHeap.clear();
                }
            }
        }
    }
}


TEST_F(IonImageBufferHeapTest, createImageBufferHeap_FromBlobHeap) {
    CAM_LOGI("@@IonImageBufferHeapTest::createImageBufferHeap_FromBlobHeap");

    for (auto const& blobsize : mSizes) {
        //
        MSize const blobImgSize = convertToTestImgSize(eImgFmt_BLOB, blobsize);
        auto const blobStrides = calculateTestStrides(eImgFmt_BLOB, blobImgSize.w);
        std::array<size_t, 3> const blobBoundary{{0, 0, 0}};
        IIonImageBufferHeap::AllocImgParam_t blobImgParam{eImgFmt_BLOB, blobImgSize, blobStrides.data(), blobBoundary.data(), 1};
        IIonImageBufferHeap::AllocExtraParam blobExtParam;

        android::String8 log = toString(eImgFmt_BLOB, blobImgSize, blobStrides, blobBoundary, MTRUE);
        CAM_LOGD("%s", log.c_str());

        sp<IIonImageBufferHeap> pIonBlobHeap = IIonImageBufferHeap::create("IonImageBufferHeapTest", blobImgParam, blobExtParam, MTRUE);
        ASSERT_TRUE(pIonBlobHeap!=nullptr) << log.c_str();


        for (auto const& format : mFormats) {
            auto const planeCount = NSCam::Utils::Format::queryPlaneCount(format);
            for (auto const& size : mSizes) {
                //
                MSize const imgSize = convertToTestImgSize(format, size);
                auto const strides = calculateTestStrides(format, imgSize.w);
                if  ( (ALIGN(strides[0]*imgSize.h*NSCam::Utils::Format::queryImageBitsPerPixel(format), 8)>>3)
                    > pIonBlobHeap->getBufSizeInBytes(0) )
                {
                    continue;
                }
                //
                for (auto boundary : mBoundaries) {
                    boundary[0] = 0;
                    android::String8 log = toString(format, size, strides, boundary, MTRUE/*contiguousPlanes*/);
                    CAM_LOGD("%s", log.c_str());

                    IIonImageBufferHeap::AllocImgParam_t imgParam{format, imgSize, strides.data(), boundary.data(), planeCount};

                    sp<IIonImageBufferHeap> pIonHeap = pIonBlobHeap->createImageBufferHeap_FromBlobHeap("IonImageBufferHeapTest", imgParam, MTRUE);
                    ASSERT_TRUE(pIonHeap!=nullptr) << log.c_str();

                    test(pIonHeap, boundary, MTRUE/*contiguousPlanes*/);

                    pIonHeap.clear();
                }
            }
        }


        pIonBlobHeap.clear();
    }
}


class IonImageBufferHeapAllocatorTest : public IonImageBufferHeapTest
{
protected:
    std::string const   mModuleDebugName{"NSCam::IIonImageBufferHeapAllocator"};

    IIonImageBufferHeapAllocator::ReserveParam const
                        mReserveParam{
                            .numberOfBuffers = 8,
                            .bufferSizeInBytes = 16000000*2, //16M
                        };

    IIonImageBufferHeapAllocator*   mIonImageBufferHeapAllocator = nullptr;

protected:
    virtual void    SetUp()
                    {
                        CAM_LOGD("[IonImageBufferHeapAllocatorTest::%s]", __FUNCTION__);
                        IonImageBufferHeapTest::SetUp();

                        mIonImageBufferHeapAllocator = IIonImageBufferHeapAllocator::getInstance();
                        ASSERT_TRUE(mIonImageBufferHeapAllocator!=nullptr) << "Bad instance";
                    }

    // Code here will be called immediately after each test (right before the destructor)
    virtual void    TearDown()
                    {
                        CAM_LOGD("[IonImageBufferHeapAllocatorTest::%s]", __FUNCTION__);
                        IonImageBufferHeapTest::TearDown();
                    }

    IIonImageBufferHeap*
                    call__IIonImageBufferHeapAllocator_create(
                        char const* szUserName,
                        IIonImageBufferHeapAllocator::AllocImgParam_t const& rImgParam,
                        IIonImageBufferHeapAllocator::AllocExtraParam_t const& rExtraParam,
                        MBOOL const enableLog
                    )
                    {
                        CAM_LOGD_IF(1, "API IIonImageBufferHeapAllocator::create");
                        IIonImageBufferHeap* pIonHeap = mIonImageBufferHeapAllocator->create(szUserName, rImgParam, rExtraParam, enableLog);
                        return pIonHeap;
                    }

    MBOOL           call__IIonImageBufferHeapAllocator_unreserve(char const* szUserName)
                    {
                        CAM_LOGD_IF(1, "API + IIonImageBufferHeapAllocator::unreserve %s", szUserName);
                        auto ret = mIonImageBufferHeapAllocator->unreserve(szUserName);
                        CAM_LOGD_IF(1, "API - IIonImageBufferHeapAllocator::unreserve %s", szUserName);
                        return ret;
                    }

    MBOOL           call__IIonImageBufferHeapAllocator_reserve(char const* szUserName, NSCam::IIonImageBufferHeapAllocator::ReserveParam const& param)
                    {
                        CAM_LOGD_IF(1, "API + IIonImageBufferHeapAllocator::reserve %s reserve:%zux%zu", szUserName, param.numberOfBuffers, param.bufferSizeInBytes);
                        auto ret = mIonImageBufferHeapAllocator->reserve(szUserName, param);
                        CAM_LOGD_IF(1, "API - IIonImageBufferHeapAllocator::reserve %s", szUserName);
                        return ret;
                    }

protected:

    template <class T>
    void            waitFutureDone(T& f, bool dumpDebug, int waitDurationInMs = 300)
                    {
                        CAM_LOGD("  $$ + wait future done ...");
                        android::LogPrinter printer(LOG_TAG, ANDROID_LOG_INFO);
                        std::future_status status;
                        do {
                            if  (dumpDebug) {
                                if ( auto pDbgMgr = IDebuggeeManager::get() ) {
                                    pDbgMgr->debug(printer, std::vector<std::string>{"--module", mModuleDebugName.c_str()});
                                }
                            }
                            status = f.wait_for(std::chrono::milliseconds(waitDurationInMs));
                        } while (status != std::future_status::ready);
                        CAM_LOGD("  $$ - wait future done ...");
                    }
};


TEST_F(IonImageBufferHeapAllocatorTest, create_without_reservation) {
    CAM_LOGI("@@IonImageBufferHeapAllocatorTest::create_without_reservation");

    for (auto const& format : mFormats) {

        auto const planeCount = NSCam::Utils::Format::queryPlaneCount(format);

        for (auto const& size : mSizes) {
            //
            MSize const imgSize = convertToTestImgSize(format, size);
            auto const strides = calculateTestStrides(format, imgSize.w);
            //
            for (auto const& boundary : mBoundaries_Start0) {

                MBOOL contiguousPlanes = MTRUE;
                android::String8 log = toString(format, size, strides, boundary, contiguousPlanes);
                CAM_LOGD("%s", log.c_str());

                IIonImageBufferHeap::AllocImgParam_t imgParam{format, imgSize, strides.data(), boundary.data(), planeCount};
                IIonImageBufferHeap::AllocExtraParam extParam;
                extParam.contiguousPlanes = MTRUE;

                sp<IIonImageBufferHeap> pIonHeap = call__IIonImageBufferHeapAllocator_create("IonImageBufferHeapTest", imgParam, extParam, MTRUE);
                ASSERT_TRUE(pIonHeap!=nullptr) << log.c_str();

                test(pIonHeap, boundary, contiguousPlanes);

                pIonHeap.clear();
            }
        }
    }
}


//adb shell data/nativetest/camtest_imgbuf/camtest_imgbuf --gtest_filter=IonImageBufferHeapAllocatorTest.unreserve_during_reserving
TEST_F(IonImageBufferHeapAllocatorTest, unreserve_during_reserving) {
    CAM_LOGI("@@IonImageBufferHeapAllocatorTest::unreserve_during_reserving");

    android::String8 str;

    str = android::String8::format("[reserving, delay 5ms, unreserve]");
    CAM_LOGI(" $ %s", str.c_str());
    ASSERT_TRUE(call__IIonImageBufferHeapAllocator_reserve(str.c_str(), mReserveParam)) << str.c_str();
    std::this_thread::sleep_for (std::chrono::milliseconds(5));
    ASSERT_TRUE(call__IIonImageBufferHeapAllocator_unreserve(str.c_str())) << str.c_str();
    std::this_thread::sleep_for (std::chrono::milliseconds(200));

    str = android::String8::format("[reserving, delay 10ms, unreserve]");
    CAM_LOGI(" $ %s", str.c_str());
    ASSERT_TRUE(call__IIonImageBufferHeapAllocator_reserve(str.c_str(), mReserveParam)) << str.c_str();
    std::this_thread::sleep_for (std::chrono::milliseconds(10));
    ASSERT_TRUE(call__IIonImageBufferHeapAllocator_unreserve(str.c_str())) << str.c_str();
    std::this_thread::sleep_for (std::chrono::milliseconds(200));

    str = android::String8::format("[reserving, delay 50ms, unreserve]");
    CAM_LOGI(" $ %s", str.c_str());
    ASSERT_TRUE(call__IIonImageBufferHeapAllocator_reserve(str.c_str(), mReserveParam)) << str.c_str();
    std::this_thread::sleep_for (std::chrono::milliseconds(50));
    ASSERT_TRUE(call__IIonImageBufferHeapAllocator_unreserve(str.c_str())) << str.c_str();
    std::this_thread::sleep_for (std::chrono::milliseconds(200));

    str = android::String8::format("[reserve, delay 1s, unreserve]");
    CAM_LOGI(" $ %s", str.c_str());
    ASSERT_TRUE(call__IIonImageBufferHeapAllocator_reserve(str.c_str(), mReserveParam)) << str.c_str();
    std::this_thread::sleep_for (std::chrono::milliseconds(1000));
    ASSERT_TRUE(call__IIonImageBufferHeapAllocator_unreserve(str.c_str())) << str.c_str();
    std::this_thread::sleep_for (std::chrono::milliseconds(200));
}


TEST_F(IonImageBufferHeapAllocatorTest, multiple_reserve) {
    CAM_LOGI("@@IonImageBufferHeapAllocatorTest::multiple_reserve");

    constexpr int count = 20;
    std::string userName{"multi-reserve"};
    for (int i = 0; i < count; i++) {
        ASSERT_TRUE(call__IIonImageBufferHeapAllocator_reserve((userName+":"+std::to_string(i)).c_str(), mReserveParam)) << i;
        std::this_thread::sleep_for (std::chrono::milliseconds(10));
    }

    for (int i = count-1; i >= 0; i--) {
        ASSERT_TRUE(call__IIonImageBufferHeapAllocator_unreserve((userName+":"+std::to_string(i)).c_str())) << i;
        std::this_thread::sleep_for (std::chrono::milliseconds(10));
    }
}


TEST_F(IonImageBufferHeapAllocatorTest, create_with_reservation) {
    CAM_LOGI("@@IonImageBufferHeapAllocatorTest::create_with_reservation");

    ASSERT_TRUE(call__IIonImageBufferHeapAllocator_reserve(LOG_TAG, mReserveParam));
    std::this_thread::sleep_for (std::chrono::milliseconds(500));

    for (auto const& format : mFormats) {

        auto const planeCount = NSCam::Utils::Format::queryPlaneCount(format);

        for (auto const& size : mSizes) {

            MSize const imgSize = convertToTestImgSize(format, size);
            auto const strides = calculateTestStrides(format, imgSize.w);

            for (auto const& boundary : mBoundaries_Start0) {

                MBOOL contiguousPlanes = MTRUE;
                android::String8 log = toString(format, size, strides, boundary, contiguousPlanes);
                CAM_LOGD("%s", log.c_str());

                IIonImageBufferHeap::AllocImgParam_t imgParam{format, imgSize, strides.data(), boundary.data(), planeCount};
                IIonImageBufferHeap::AllocExtraParam extParam;
                extParam.contiguousPlanes = MTRUE;

                sp<IIonImageBufferHeap> pIonHeap = call__IIonImageBufferHeapAllocator_create("IonImageBufferHeapAllocatorTest", imgParam, extParam, MTRUE);
                ASSERT_TRUE(pIonHeap!=nullptr) << log.c_str();

                test(pIonHeap, boundary, contiguousPlanes);

                CAM_LOGD("pIonHeap:%p getStrongCount:%d", pIonHeap.get(), pIonHeap->getStrongCount());
                pIonHeap.clear();
            }
        }
    }

    ASSERT_TRUE(call__IIonImageBufferHeapAllocator_unreserve(LOG_TAG));
    std::this_thread::sleep_for (std::chrono::milliseconds(200));
}


TEST_F(IonImageBufferHeapAllocatorTest, reuse) {
    CAM_LOGI("@@IonImageBufferHeapAllocatorTest::reuse");

    ASSERT_TRUE(call__IIonImageBufferHeapAllocator_reserve(LOG_TAG, mReserveParam));
    std::this_thread::sleep_for (std::chrono::milliseconds(500));

    for (auto const& format : mFormats) {

        auto const planeCount = NSCam::Utils::Format::queryPlaneCount(format);

        for (auto const& size : mSizes) {

            MSize const imgSize = convertToTestImgSize(format, size);
            auto const strides = calculateTestStrides(format, imgSize.w);

            for (auto const& boundary : mBoundaries_Start0) {

                MBOOL contiguousPlanes = MTRUE;

                IIonImageBufferHeap::AllocImgParam_t imgParam{format, imgSize, strides.data(), boundary.data(), planeCount};
                IIonImageBufferHeap::AllocExtraParam extParam;
                extParam.contiguousPlanes = MTRUE;

                std::vector<sp<IIonImageBufferHeap>> vAllocated;

                //(1) allocate all reserved heaps.
                CAM_LOGD("  $ (1) allocate all reserved heaps");
                for (size_t i = 0; i < mReserveParam.numberOfBuffers; i++) {
                    android::String8 log = toString(format, size, strides, boundary, contiguousPlanes) + android::String8::format("-%zu", i);
                    vAllocated.push_back(call__IIonImageBufferHeapAllocator_create(log.c_str(), imgParam, extParam, MFALSE));
                    ASSERT_TRUE(vAllocated[i]!=nullptr) << log.c_str();
                }
                EXPECT_TRUE(vAllocated.size()==mReserveParam.numberOfBuffers) << vAllocated.size() << " != " << mReserveParam.numberOfBuffers;

                //(2) allocate one more heap, which should not be from the reserved pool.
                {
                    CAM_LOGD("  $ (2) allocate one more heap, which should not be from the reserved pool.");
                    android::String8 log = toString(format, size, strides, boundary, contiguousPlanes) + "-one_more_alloc";
                    sp<IIonImageBufferHeap> pIonHeap = call__IIonImageBufferHeapAllocator_create("IonImageBufferHeapAllocatorTest", imgParam, extParam, MTRUE);
                    ASSERT_TRUE(pIonHeap!=nullptr) << log.c_str();
                }

                //(3) release all allocated heaps.
                CAM_LOGD("  $ (3) release all allocated heaps - vAllocated.size:%zu", vAllocated.size());
                for (auto& v : vAllocated) {//dtor is invoked in order
                    v.clear();
                }
                vAllocated.clear();         //dtor is invoked reversely.

                //(4) allocate all heaps again, to make sure all reserved heaps can be re-used...
                CAM_LOGD("  $ (4) allocate all heaps again, to make sure all reserved heaps can be re-used...");
                for (size_t i = 0; i < mReserveParam.numberOfBuffers; i++) {
                    android::String8 log = toString(format, size, strides, boundary, contiguousPlanes) + android::String8::format("-%zu", i);
                    vAllocated.push_back(call__IIonImageBufferHeapAllocator_create(log.c_str(), imgParam, extParam, MFALSE));
                    ASSERT_TRUE(vAllocated[i]!=nullptr) << log.c_str();

                    test(vAllocated[i], boundary, contiguousPlanes);
                    CAM_LOGD("%zu pIonHeap:%p getStrongCount:%d", i, vAllocated[i].get(), vAllocated[i]->getStrongCount());
                }
            }
        }
    }

    ASSERT_TRUE(call__IIonImageBufferHeapAllocator_unreserve(LOG_TAG));
    std::this_thread::sleep_for (std::chrono::milliseconds(200));
}


TEST_F(IonImageBufferHeapAllocatorTest, unreserve_before_release) {
    CAM_LOGI("@@IonImageBufferHeapAllocatorTest::unreserve_before_release");

    auto test_for_each_setting = [this](size_t const count){

        std::vector<sp<IIonImageBufferHeap>> vAllocated(count, nullptr);

        for (auto const& format : mFormats) {

            auto const planeCount = NSCam::Utils::Format::queryPlaneCount(format);

            for (auto const& size : mSizes) {

                MSize const imgSize = convertToTestImgSize(format, size);
                auto const strides = calculateTestStrides(format, imgSize.w);

                for (auto const& boundary : mBoundaries_Start0) {

                    MBOOL contiguousPlanes = MTRUE;

                    IIonImageBufferHeap::AllocImgParam_t imgParam{format, imgSize, strides.data(), boundary.data(), planeCount};
                    IIonImageBufferHeap::AllocExtraParam extParam;
                    extParam.contiguousPlanes = MTRUE;

                    //(1) reserve
                    CAM_LOGD("  $ (1) reserve");
                    ASSERT_TRUE(call__IIonImageBufferHeapAllocator_reserve(LOG_TAG, mReserveParam));
                    std::this_thread::sleep_for (std::chrono::milliseconds(200));

                    //(2) alloc
                    CAM_LOGD("  $ (2) allocate reserved heaps: #size:%zu", vAllocated.size());
                    for (size_t i = 0; i < vAllocated.size(); i++) {
                        android::String8 log = toString(format, size, strides, boundary, contiguousPlanes) + android::String8::format("-%zu", i);
                        vAllocated[i] = (call__IIonImageBufferHeapAllocator_create(log.c_str(), imgParam, extParam, MFALSE));
                        ASSERT_TRUE(vAllocated[i]!=nullptr) << log.c_str();
                    }

                    //(3) unreserve
                    CAM_LOGD("  $ (3) unreserve");
                    ASSERT_TRUE(call__IIonImageBufferHeapAllocator_unreserve(LOG_TAG));
                    std::this_thread::sleep_for (std::chrono::milliseconds(200));

                    //(4) access test
                    CAM_LOGD("  $ (4) access test after unreserve done");
                    for (size_t i = 0; i < vAllocated.size(); i++) {
                        test(vAllocated[i], boundary, contiguousPlanes);
                        CAM_LOGD("%zu pIonHeap:%p getStrongCount:%d", i, vAllocated[i].get(), vAllocated[i]->getStrongCount());
                    }

                    //(5) release
                    CAM_LOGD("  $ (5) release all allocated heaps - vAllocated.size:%zu", vAllocated.size());
                    vAllocated.clear();         //dtor is invoked reversely.
                }
            }
        }
    };


    auto f = std::async(std::launch::async, [&](){
        CAM_LOGD("  $$ allocate all");
        test_for_each_setting(mReserveParam.numberOfBuffers);

        CAM_LOGD("  $$ allocate one");
        test_for_each_setting(1);
    });


    waitFutureDone(f, /*dumpDebug*/false);
}


TEST_F(IonImageBufferHeapAllocatorTest, create_without_reservation_vs_create_with_reservation) {
    CAM_LOGI("@@IonImageBufferHeapAllocatorTest::create_without_reservation_vs_create_with_reservation");

    using namespace std::chrono_literals;
    typedef std::chrono::duration<double, std::milli> duration_t;
    struct Report
    {
        std::string str;
        double      duration; //ms
    };

    auto test_for_each_setting = [this](size_t const count, std::vector<Report>& vReport){
        auto test = [&](auto format, auto const& imgSize, size_t planeCount){
            auto const strides = calculateTestStrides(format, imgSize.w);
            auto const& boundary = mBoundaries_Start0[0];
            IIonImageBufferHeap::AllocImgParam_t imgParam{format, imgSize, strides.data(), boundary.data(), planeCount};
            IIonImageBufferHeap::AllocExtraParam extParam;
            extParam.contiguousPlanes = MTRUE;

            std::vector<sp<IIonImageBufferHeap>> vAllocated(count, nullptr);
            //(1) alloc
            CAM_LOGD("  $ (1) allocate reserved heaps: #size:%zu", vAllocated.size());
            for (size_t i = 0; i < vAllocated.size(); i++) {
                android::String8 log = toString(format, imgSize, strides, boundary, extParam.contiguousPlanes) + android::String8::format("-%zu", i);

//                auto start = std::chrono::high_resolution_clock::now();
                nsecs_t const start = ::systemTime();
                vAllocated[i] = (call__IIonImageBufferHeapAllocator_create(log.c_str(), imgParam, extParam, MFALSE));
//                auto end = std::chrono::high_resolution_clock::now();
                nsecs_t const end = ::systemTime();
                vReport.push_back(Report{.str=log.c_str(), .duration=(end-start)/1000000.0});

                ASSERT_TRUE(vAllocated[i]!=nullptr) << log.c_str();
            }

            //(2) release
            CAM_LOGD("  $ (2) release all allocated heaps - vAllocated.size:%zu", vAllocated.size());
            vAllocated.clear();         //dtor is invoked reversely.
        };

        for (auto const& format : mFormats) {
            auto const planeCount = NSCam::Utils::Format::queryPlaneCount(format);
            for (auto const& size : mSizes) {
                MSize const imgSize = convertToTestImgSize(format, size);
                test(format, imgSize, planeCount);
            }
        }
    };


    auto f = std::async(std::launch::async, [&](){
        std::vector<Report> reportWithoutReserve, reportWithReserve;
        {
            CAM_LOGD("  $$ allocate without reservation");
            test_for_each_setting(mReserveParam.numberOfBuffers, reportWithoutReserve);
        }
        {
            CAM_LOGD("  $$ allocate with reservation");

            //(1) reserve
            CAM_LOGD("  $ allocate with reservation - (1) reserve");
            ASSERT_TRUE(call__IIonImageBufferHeapAllocator_reserve(LOG_TAG, mReserveParam));
            std::this_thread::sleep_for (std::chrono::milliseconds(200));

            //(2) test
            CAM_LOGD("  $ allocate with reservation - (2) test");
            test_for_each_setting(mReserveParam.numberOfBuffers, reportWithReserve);

            //(3) unreserve
            CAM_LOGD("  $ allocate with reservation - (3) unreserve");
            ASSERT_TRUE(call__IIonImageBufferHeapAllocator_unreserve(LOG_TAG));
            std::this_thread::sleep_for (std::chrono::milliseconds(200));
        }

        ASSERT_TRUE(reportWithoutReserve.size() == reportWithReserve.size()) << "reportWithoutReserve.size:" << reportWithoutReserve.size() << " != reportWithReserve.size:" << reportWithReserve.size();

        CAM_LOGD("$$ Summary");

        //  no reserve
        double avgDuration1 = 0, maxDuration1 = 0, minDuration1 = 1000000;
        //  reserve
        double avgDuration2 = 0, maxDuration2 = 0, minDuration2 = 1000000;

        for (size_t i = 0; i < reportWithoutReserve.size(); i++) {
            auto const duration1 = reportWithoutReserve[i].duration;
            auto const duration2 = reportWithReserve[i].duration;

            avgDuration1 += duration1;
            avgDuration2 += duration2;
            maxDuration1 = std::max(maxDuration1, duration1);
            maxDuration2 = std::max(maxDuration2, duration2);
            minDuration1 = std::min(minDuration1, duration1);
            minDuration2 = std::min(minDuration2, duration2);

            CAM_LOGD("%s: duration(ms)= delta(%.2f) no-reserve(%.2f) reserve(%.2f)",
                reportWithoutReserve[i].str.c_str(), (duration1-duration2), duration1, duration2
            );
        }
        avgDuration1 /= reportWithoutReserve.size();
        avgDuration2 /= reportWithReserve.size();
        CAM_LOGD("duration(ms) no-reserve: max(%.2f) min(%.2f) avg(%.2f)", maxDuration1, minDuration1, avgDuration1);
        CAM_LOGD("duration(ms)    reserve: max(%.2f) min(%.2f) avg(%.2f)", maxDuration2, minDuration2, avgDuration2);
    });


    waitFutureDone(f, /*dumpDebug*/false);
}


//adb shell data/nativetest/camtest_imgbuf/camtest_imgbuf --gtest_filter=IonImageBufferHeapAllocatorTest.create_fixed_size_without_reservation_vs_create_with_reservation
TEST_F(IonImageBufferHeapAllocatorTest, create_fixed_size_without_reservation_vs_create_with_reservation) {
    CAM_LOGI("@@IonImageBufferHeapAllocatorTest::create_fixed_size_without_reservation_vs_create_with_reservation");

    using namespace std::chrono_literals;
    typedef std::chrono::duration<double, std::milli> duration_t;
    struct Report
    {
        std::string str;
        double      duration; //ms
    };

    auto test_for_each_setting = [this](size_t const count, std::vector<Report>& vReport){
        auto test = [&](auto format, auto const& imgSize, size_t planeCount){
            auto const strides = calculateTestStrides(format, imgSize.w);
            auto const& boundary = mBoundaries_Start0[0];
            IIonImageBufferHeap::AllocImgParam_t imgParam{format, imgSize, strides.data(), boundary.data(), planeCount};
            IIonImageBufferHeap::AllocExtraParam extParam;
            extParam.contiguousPlanes = MTRUE;

            std::vector<sp<IIonImageBufferHeap>> vAllocated(count, nullptr);
            //(1) alloc
            CAM_LOGD("  $ (1) allocate reserved heaps: #size:%zu", vAllocated.size());
            for (size_t i = 0; i < vAllocated.size(); i++) {
                android::String8 log = toString(format, imgSize, strides, boundary, extParam.contiguousPlanes) + android::String8::format("-%zu", i);

//                auto start = std::chrono::high_resolution_clock::now();
                nsecs_t const start = ::systemTime();
                vAllocated[i] = (call__IIonImageBufferHeapAllocator_create(log.c_str(), imgParam, extParam, MFALSE));
//                auto end = std::chrono::high_resolution_clock::now();
                nsecs_t const end = ::systemTime();
                vReport.push_back(Report{.str=log.c_str(), .duration=(end-start)/1000000.0});

                ASSERT_TRUE(vAllocated[i]!=nullptr) << log.c_str();
            }

            //(2) release
            CAM_LOGD("  $ (2) release all allocated heaps - vAllocated.size:%zu", vAllocated.size());
            vAllocated.clear();         //dtor is invoked reversely.
        };

        test(eImgFmt_BLOB, MSize(mReserveParam.bufferSizeInBytes/*16M*/, 1), 1/*planeCount*/);
    };


    auto f = std::async(std::launch::async, [&](){
        std::vector<Report> reportWithoutReserve, reportWithReserve;
        {
            CAM_LOGD("  $$ allocate without reservation");
            test_for_each_setting(mReserveParam.numberOfBuffers, reportWithoutReserve);
        }
        {
            CAM_LOGD("  $$ allocate with reservation");

            //(1) reserve
            CAM_LOGD("  $ allocate with reservation - (1) reserve");
            ASSERT_TRUE(call__IIonImageBufferHeapAllocator_reserve(LOG_TAG, mReserveParam));
            std::this_thread::sleep_for (std::chrono::milliseconds(200));

            //(2) test
            CAM_LOGD("  $ allocate with reservation - (2) test");
            test_for_each_setting(mReserveParam.numberOfBuffers, reportWithReserve);

            //(3) unreserve
            CAM_LOGD("  $ allocate with reservation - (3) unreserve");
            ASSERT_TRUE(call__IIonImageBufferHeapAllocator_unreserve(LOG_TAG));
            std::this_thread::sleep_for (std::chrono::milliseconds(200));
        }

        ASSERT_TRUE(reportWithoutReserve.size() == reportWithReserve.size()) << "reportWithoutReserve.size:" << reportWithoutReserve.size() << " != reportWithReserve.size:" << reportWithReserve.size();

        CAM_LOGD("$$ Summary");

        //  no reserve
        double avgDuration1 = 0, maxDuration1 = 0, minDuration1 = 1000000;
        //  reserve
        double avgDuration2 = 0, maxDuration2 = 0, minDuration2 = 1000000;

        for (size_t i = 0; i < reportWithoutReserve.size(); i++) {
            auto const duration1 = reportWithoutReserve[i].duration;
            auto const duration2 = reportWithReserve[i].duration;

            avgDuration1 += duration1;
            avgDuration2 += duration2;
            maxDuration1 = std::max(maxDuration1, duration1);
            maxDuration2 = std::max(maxDuration2, duration2);
            minDuration1 = std::min(minDuration1, duration1);
            minDuration2 = std::min(minDuration2, duration2);

            CAM_LOGD("%s: duration(ms)= delta(%.2f) no-reserve(%.2f) reserve(%.2f)",
                reportWithoutReserve[i].str.c_str(), (duration1-duration2), duration1, duration2
            );
        }
        avgDuration1 /= reportWithoutReserve.size();
        avgDuration2 /= reportWithReserve.size();
        CAM_LOGD("duration(ms) no-reserve: max(%.2f) min(%.2f) avg(%.2f)", maxDuration1, minDuration1, avgDuration1);
        CAM_LOGD("duration(ms)    reserve: max(%.2f) min(%.2f) avg(%.2f)", maxDuration2, minDuration2, avgDuration2);
    });


    waitFutureDone(f, /*dumpDebug*/false);
}


//adb shell data/nativetest/camtest_imgbuf/camtest_imgbuf --gtest_filter=IonImageBufferHeapAllocatorTest.debug_create_fixed_size_with_reservation
TEST_F(IonImageBufferHeapAllocatorTest, debug_create_fixed_size_with_reservation) {
    CAM_LOGI("@@IonImageBufferHeapAllocatorTest::debug_create_fixed_size_with_reservation");

    using namespace std::chrono_literals;
    typedef std::chrono::duration<double, std::milli> duration_t;
    struct Report
    {
        std::string str;
        double      duration; //ms
    };

    auto test_for_each_setting = [this](size_t const count, std::vector<Report>& vReport){
        auto test = [&](auto format, auto const& imgSize, size_t planeCount){
            auto const strides = calculateTestStrides(format, imgSize.w);
            auto const& boundary = mBoundaries_Start0[0];
            IIonImageBufferHeap::AllocImgParam_t imgParam{format, imgSize, strides.data(), boundary.data(), planeCount};
            IIonImageBufferHeap::AllocExtraParam extParam;
            extParam.contiguousPlanes = MTRUE;

            std::vector<sp<IIonImageBufferHeap>> vAllocated(count, nullptr);
            //(1) alloc
            CAM_LOGD("  $ (1) allocate reserved heaps: #size:%zu", vAllocated.size());
            for (size_t i = 0; i < vAllocated.size(); i++) {
                android::String8 log = toString(format, imgSize, strides, boundary, extParam.contiguousPlanes) + android::String8::format("-%zu", i);

//                auto start = std::chrono::high_resolution_clock::now();
                nsecs_t const start = ::systemTime();
                vAllocated[i] = (call__IIonImageBufferHeapAllocator_create(log.c_str(), imgParam, extParam, MFALSE));
//                auto end = std::chrono::high_resolution_clock::now();
                nsecs_t const end = ::systemTime();
                vReport.push_back(Report{.str=log.c_str(), .duration=(end-start)/1000000.0});

                ASSERT_TRUE(vAllocated[i]!=nullptr) << log.c_str();
                std::this_thread::sleep_for (std::chrono::milliseconds(100));
            }

            //(2) release
            CAM_LOGD("  $ (2) release all allocated heaps - vAllocated.size:%zu", vAllocated.size());
            vAllocated.clear();         //dtor is invoked reversely.
        };

        test(eImgFmt_BLOB, MSize(mReserveParam.bufferSizeInBytes/*16M*/, 1), 1/*planeCount*/);
    };


    auto f = std::async(std::launch::async, [&](){
        std::vector<Report> reportWithReserve;
        {
            CAM_LOGD("  $$ allocate with reservation");

            //(1) reserve
            CAM_LOGD("  $ allocate with reservation - (1) reserve");
            ASSERT_TRUE(call__IIonImageBufferHeapAllocator_reserve(LOG_TAG, mReserveParam));
            std::this_thread::sleep_for (std::chrono::milliseconds(500));

            //(2) test
            CAM_LOGD("  $ allocate with reservation - (2) test");
            test_for_each_setting(mReserveParam.numberOfBuffers, reportWithReserve);

            //(3) unreserve
            CAM_LOGD("  $ allocate with reservation - (3) unreserve");
            ASSERT_TRUE(call__IIonImageBufferHeapAllocator_unreserve(LOG_TAG));
            std::this_thread::sleep_for (std::chrono::milliseconds(500));
        }

        CAM_LOGD("$$ Summary");

        //  reserve
        double avgDuration2 = 0, maxDuration2 = 0, minDuration2 = 1000000;
        for (size_t i = 0; i < reportWithReserve.size(); i++) {
            auto const duration2 = reportWithReserve[i].duration;
            avgDuration2 += duration2;
            maxDuration2 = std::max(maxDuration2, duration2);
            minDuration2 = std::min(minDuration2, duration2);

            CAM_LOGD("%s: duration(ms)= reserve(%.2f)",
                reportWithReserve[i].str.c_str(), duration2
            );
        }
        avgDuration2 /= reportWithReserve.size();
        CAM_LOGD("duration(ms)    reserve: max(%.2f) min(%.2f) avg(%.2f)", maxDuration2, minDuration2, avgDuration2);

    });


    waitFutureDone(f, /*dumpDebug*/true, /*waitDurationInMs*/70);
}

