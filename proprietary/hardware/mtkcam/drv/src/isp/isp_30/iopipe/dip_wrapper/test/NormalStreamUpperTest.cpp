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

#define LOG_TAG "NormalStreamUpperTest"

#include <chrono>
#include <thread>
#include <random>
#include <future>
#include <vector>
#include <atomic>
#include <utils/Mutex.h>
#include <utils/List.h>
#include <utils/Condition.h>
#include <utils/Thread.h>
#include <gtest/gtest.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/module/module.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
#include <mtkcam/def/common.h>
#include <mtkcam/drv/iopipe/PortMap.h>
#include <mtkcam/drv/iopipe/Event/IoPipeEvent.h>
#include "../NormalStreamUpper.h"


#define TEST_LOGD(fmt, arg...) do { CAM_LOGD("[%s]" fmt, __func__, ##arg); } while(0)


using namespace android;


namespace NSCam {
namespace NSIoPipe {
namespace NSPostProc {
namespace Wrapper {


class SleepRandom
{
private:
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<int> dis;

public:

    SleepRandom(int fromMs, int upToMs) :
        rd(), gen(rd()), dis(fromMs, upToMs)
    {
    }

    void operator() () {
        int ms = dis(gen);
        if (ms > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
};


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"


class NormalStreamMock : public INormalStream
{
public:
    NormalStreamMock(MUINT32 openedSensorIndex) : mNumOfInstances(0), mNumOfIpRawRequests(0) {
        (void)openedSensorIndex;
    }

    virtual ~NormalStreamMock();

    virtual MBOOL enque(NSCam::NSIoPipe::QParams const& rParams);
    virtual MBOOL deque(NSCam::NSIoPipe::QParams& rParams, MINT64 i8TimeoutNs);

    virtual MBOOL setJpegParam(EJpgCmd jpgCmd,int arg1,int arg2) {
        return MTRUE;
    }

    virtual MBOOL setFps(MINT32 fps) {
        return MTRUE;
    }

    virtual MBOOL sendCommand(ESDCmd cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3) {
        return MTRUE;
    }

    virtual MBOOL init(char const* szCallerName) {
        return init(szCallerName, EStreamPipeID_Normal);
    }

    virtual MBOOL init(char const* szCallerName, NSCam::NSIoPipe::EStreamPipeID mPipeID);

    virtual MBOOL uninit(char const* szCallerName) {
        return uninit(szCallerName, EStreamPipeID_Normal);
    }

    virtual MBOOL uninit(char const* szCallerName, NSCam::NSIoPipe::EStreamPipeID mPipeID);

    virtual MERROR getLastErrorCode() const {
        return 0;
    }

    virtual MBOOL startVideoRecord(MINT32 wd,MINT32 ht, MINT32 fps) {
        return MTRUE;
    }

    virtual MBOOL stopVideoRecord() {
        return MTRUE;
    }

    virtual MVOID destroyInstance();

    std::atomic_int mNumOfInstances;
    std::atomic_int mNumOfIpRawRequests;

private:
    class SimuThread : public android::Thread {
    public:
        SimuThread(NormalStreamMock &normalStream) :
            mNormalStream(normalStream)
        {
        }

        static bool isIpRawRequest(const QParams &qParams);
        virtual bool threadLoop() override;

    private:
        NormalStreamMock &mNormalStream;
    };
    friend class SimuThread;

    android::sp<SimuThread> mSimuThread;
    android::Mutex mMutex;
    android::Condition mPendingCond;
    android::List<QParams> mProcessQueue;
    android::List<QParams> mResultQueue;
    android::Condition mResultCond;
};


NormalStreamMock::~NormalStreamMock()
{
}


MBOOL NormalStreamMock::init(char const*, NSCam::NSIoPipe::EStreamPipeID)
{
    mSimuThread = new SimuThread(*this);
    mSimuThread->run("NormalStreamMock");
    return MTRUE;
}


MBOOL NormalStreamMock::uninit(char const*, NSCam::NSIoPipe::EStreamPipeID)
{
    {
        Mutex::Autolock lock(mMutex);
        mSimuThread->requestExit();
        mPendingCond.broadcast();
    }

    mSimuThread->join();

    return MTRUE;
}


MBOOL NormalStreamMock::enque(NSCam::NSIoPipe::QParams const& rParams)
{
    Mutex::Autolock lock(mMutex);
    TEST_LOGD("cookie = %p", rParams.mpCookie);
    mProcessQueue.push_back(rParams);
    mPendingCond.broadcast();
    return MTRUE;
}


MBOOL NormalStreamMock::deque(NSCam::NSIoPipe::QParams& rParams, MINT64 i8TimeoutNs)
{
    Mutex::Autolock lock(mMutex);
    TEST_LOGD("Deque request, timeout = %" PRId64, i8TimeoutNs);

    if (mResultQueue.size() > 0) {
        rParams = *mResultQueue.begin();
        TEST_LOGD("cookie = %p", rParams.mpCookie);
        mResultQueue.erase(mResultQueue.begin());
        return MTRUE;
    }

    if (i8TimeoutNs > 0)
        mResultCond.waitRelative(mMutex, i8TimeoutNs);
    else if (i8TimeoutNs < 0) {
        while (mResultQueue.size() == 0)
            mResultCond.wait(mMutex);
    } else
        return MFALSE;

    if (mResultQueue.size() > 0) {
        rParams = *mResultQueue.begin();
        TEST_LOGD("cookie = %p", rParams.mpCookie);
        mResultQueue.erase(mResultQueue.begin());
        return MTRUE;
    }

    return MFALSE;
}


MVOID NormalStreamMock::destroyInstance()
{
    mNumOfInstances--;
}


bool NormalStreamMock::SimuThread::isIpRawRequest(const QParams &qParams)
{
    for (auto &frameParam : qParams.mvFrameParams)
        if (frameParam.mStreamTag == ENormalStreamTag_IP_Tpipe)
            return true;

    return false;
}


bool NormalStreamMock::SimuThread::threadLoop()
{
    SleepRandom sleepRandom(0, 5);
    QParams pendingItem;

    while (!exitPending()) {
        {
            Mutex::Autolock lock(mNormalStream.mMutex);

            while (mNormalStream.mProcessQueue.empty() && !exitPending()) {
                mNormalStream.mPendingCond.wait(mNormalStream.mMutex);
            }

            if (exitPending())
                return false;

            // I hope move can be supported, but it did not
            pendingItem = std::move(*mNormalStream.mProcessQueue.begin());
            mNormalStream.mProcessQueue.erase(mNormalStream.mProcessQueue.begin());
        }

        sleepRandom();

        if (isIpRawRequest(pendingItem))
            mNormalStream.mNumOfIpRawRequests++;

        pendingItem.mDequeSuccess = MTRUE;

        if (pendingItem.mpfnCallback != NULL) {
            TEST_LOGD("cookie = %p, callback", pendingItem.mpCookie);
            (*pendingItem.mpfnCallback)(pendingItem);
        } else {
            Mutex::Autolock lock(mNormalStream.mMutex);
            TEST_LOGD("cookie = %p, push result", pendingItem.mpCookie);
            mNormalStream.mResultQueue.push_back(std::move(pendingItem));
            mNormalStream.mResultCond.signal();
        }
    }

    return true;
}

#pragma clang diagnostic pop


class UTDriverFactory
{
public:
    UTDriverFactory() : mNormalStream(0) { }

    INormalStream* operator() (MUINT32) {
        mNormalStream.mNumOfInstances++;
        return &mNormalStream;
    }

    NormalStreamMock mNormalStream;
};


class ImageHeapRAII
{
    static constexpr size_t BUFFER_SIZE = 1920 * 1080;

public:
    ~ImageHeapRAII() {
        release();
    }

    sp<IImageBuffer> createBuffer() {
        IImageBufferAllocator::ImgParam imgParam(BUFFER_SIZE, 0);
        sp<IImageBufferHeap> heap = IIonImageBufferHeap::create(LOG_TAG, imgParam);
        mHeapCreated.push_back(heap);

        sp<IImageBuffer> buffer(heap->createImageBuffer());
        mBufferCreated.push_back(buffer);

        return buffer;
    }

    void release() {
        mHeapCreated.clear();
    }

private:

    std::vector<sp<IImageBufferHeap>> mHeapCreated;
    std::vector<sp<IImageBuffer>> mBufferCreated;
};


class FakeP1Node
{
public:
    FakeP1Node() : mP1Acquiring(0) { }

    static IoPipeEventCtrl onP1Acquiring(FakeP1Node *inst, IpRawP1AcquiringEvent &evt) {
        Mutex::Autolock lock(inst->mMutex);

        std::this_thread::sleep_for(std::chrono::milliseconds(3));
        inst->mP1Acquiring++;
        evt.setResult(IoPipeEvent::RESULT_OK);

        TEST_LOGD("mP1Acquiring = %d", inst->mP1Acquiring);

        return IoPipeEventCtrl::OK;
    }

    static IoPipeEventCtrl onP1Released(FakeP1Node *inst, IpRawP1ReleasedEvent &) {
        Mutex::Autolock lock(inst->mMutex);

        std::this_thread::sleep_for(std::chrono::milliseconds(3));
        inst->mP1Acquiring--;
        inst->mEventCond.broadcast();

        TEST_LOGD("mP1Acquiring = %d", inst->mP1Acquiring);

        return IoPipeEventCtrl::OK;
    }

    bool waitForReleased() {
        Mutex::Autolock lock(mMutex);

        if (mP1Acquiring > 0) {
            mEventCond.waitRelative(mMutex, 1000ULL * 1000000ULL); // 1 sec
        }

        return (mP1Acquiring == 0);
    }

    Mutex mMutex;
    Condition mEventCond;
    int mP1Acquiring;
    sp<IoPipeEventHandle> mAcqEventHandle;
    sp<IoPipeEventHandle> mRelEventHandle;
};


class Callback
{
private:
    int mCalled;
    Mutex mMutex;
    Condition mCond;

public:
    Callback() : mCalled(0) { }

    static MVOID callback(QParams& rParams);

    void fill(QParams& rParams) {
        rParams.mpCookie = static_cast<MVOID*>(this);
        rParams.mpfnCallback = &callback;
        rParams.mpfnEnQFailCallback = &callback;
        rParams.mpfnEnQBlockCallback = &callback;
    }

    bool waitForCallback(int n) {
        Mutex::Autolock lock(mMutex);
        for (int i = mCalled; i < n && mCalled < n; i++) {
            mCond.waitRelative(mMutex, 1000ULL * 1000000ULL);
        }
        return (mCalled >= n);
    }
};


MVOID Callback::callback(QParams& rParams)
{
    Callback *inst = static_cast<Callback*>(rParams.mpCookie);

    Mutex::Autolock lock(inst->mMutex);
    inst->mCalled++;
    inst->mCond.broadcast();
    TEST_LOGD("mCalled = %d", inst->mCalled);
}


}
}
}
}


// =================================================================================

using namespace NSCam;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NSCam::NSIoPipe::NSPostProc::Wrapper;


TEST(TestNormalStreamUpper, Basic)
{
    FakeP1Node fakeP1;
    IoPipeEventSystem &globalEvent = IoPipeEventSystem::getGlobal();
    fakeP1.mAcqEventHandle = globalEvent.subscribe(FakeP1Node::onP1Acquiring, &fakeP1);
    fakeP1.mRelEventHandle = globalEvent.subscribe(FakeP1Node::onP1Released, &fakeP1);

    UTDriverFactory driverFactory;
    INormalStream *normalStream = NormalStreamUpper::createInstance(0, driverFactory);
    ASSERT_NE(nullptr, normalStream);

    MBOOL isInited = normalStream->init(LOG_TAG);
    ASSERT_TRUE(isInited);

    ImageHeapRAII heapRAII;

    {
        sp<IImageBuffer> inputBuffer = heapRAII.createBuffer();
        sp<IImageBuffer> outputBuffer = heapRAII.createBuffer();

        QParams qParams;
        FrameParams frameParam;
        frameParam.mvIn.push_back(NSCam::NSIoPipe::Input(PORT_IMGI, inputBuffer.get()));
        frameParam.mvOut.push_back(NSCam::NSIoPipe::Output(PORT_WROTO, outputBuffer.get()));
        qParams.mvFrameParams.push_back(frameParam);

        normalStream->enque(qParams);
    }

    Callback callback;
    {
        sp<IImageBuffer> inputBuffer = heapRAII.createBuffer();
        sp<IImageBuffer> outputBuffer = heapRAII.createBuffer();

        inputBuffer->setImgDesc(eIMAGE_DESC_ID_RAW_TYPE, eIMAGE_DESC_RAW_TYPE_PURE);

        QParams qParams;
        FrameParams frameParam;
        frameParam.mvIn.push_back(NSCam::NSIoPipe::Input(PORT_IMGI, inputBuffer.get()));
        frameParam.mvOut.push_back(NSCam::NSIoPipe::Output(PORT_WROTO, outputBuffer.get()));
        qParams.mvFrameParams.push_back(frameParam);

        callback.fill(qParams);

        normalStream->enque(qParams);
    }

    QParams qParams;
    normalStream->deque(qParams);
    EXPECT_TRUE(callback.waitForCallback(1));

    MBOOL isUninited = normalStream->uninit(LOG_TAG);
    EXPECT_TRUE(isUninited);

    EXPECT_TRUE(fakeP1.waitForReleased());

    normalStream->destroyInstance();

    fakeP1.mAcqEventHandle->unsubscribe();
    fakeP1.mRelEventHandle->unsubscribe();
}


TEST(TestNormalStreamUpper, InterOper)
{
    static constexpr int N_ITERS = 1000;

    FakeP1Node fakeP1;
    IoPipeEventSystem &globalEvent = IoPipeEventSystem::getGlobal();
    fakeP1.mAcqEventHandle = globalEvent.subscribe(FakeP1Node::onP1Acquiring, &fakeP1);
    fakeP1.mRelEventHandle = globalEvent.subscribe(FakeP1Node::onP1Released, &fakeP1);

    UTDriverFactory driverFactory;
    ImageHeapRAII heapRAII;

    INormalStream *normalStream = NormalStreamUpper::createInstance(0, driverFactory);
    ASSERT_NE(nullptr, normalStream);

    MBOOL isInited = normalStream->init(LOG_TAG);
    ASSERT_TRUE(isInited);

    auto threadDeque = [normalStream, &heapRAII] () {
        SleepRandom sleepRandom(0, 5);
        sp<IImageBuffer> inputBuffer = heapRAII.createBuffer();
        sp<IImageBuffer> outputBuffer = heapRAII.createBuffer();
        sp<IImageBuffer> inputBufferPureRaw = heapRAII.createBuffer();
        inputBufferPureRaw->setImgDesc(eIMAGE_DESC_ID_RAW_TYPE, eIMAGE_DESC_RAW_TYPE_PURE);

        for (int i = 0; i < N_ITERS / 2; i++) {
            {
                QParams qParams;
                FrameParams frameParam;
                frameParam.mvIn.push_back(NSCam::NSIoPipe::Input(PORT_IMGI, inputBuffer.get()));
                frameParam.mvOut.push_back(NSCam::NSIoPipe::Output(PORT_WROTO, outputBuffer.get()));
                qParams.mvFrameParams.push_back(frameParam);

                qParams.mpCookie = &qParams;
                normalStream->enque(qParams);

                qParams.mpCookie = NULL;
                normalStream->deque(qParams);

                EXPECT_TRUE(qParams.mpCookie == &qParams);
            }

            sleepRandom();

            {
                QParams qParams;
                FrameParams frameParam;
                frameParam.mvIn.push_back(NSCam::NSIoPipe::Input(PORT_IMGI, inputBufferPureRaw.get()));
                frameParam.mvOut.push_back(NSCam::NSIoPipe::Output(PORT_WROTO, outputBuffer.get()));
                qParams.mvFrameParams.push_back(frameParam);

                qParams.mpCookie = &qParams;
                normalStream->enque(qParams);

                qParams.mpCookie = NULL;
                normalStream->deque(qParams);

                EXPECT_TRUE(qParams.mpCookie == &qParams);
            }

            sleepRandom();
        }
    };

    auto threadNormalCallback = [normalStream, &heapRAII] () {
        SleepRandom sleepRandom(0, 5);
        sp<IImageBuffer> inputBuffer = heapRAII.createBuffer();
        sp<IImageBuffer> outputBuffer = heapRAII.createBuffer();

        for (int i = 0; i < N_ITERS; i++) {
            QParams qParams;
            FrameParams frameParam;
            frameParam.mvIn.push_back(NSCam::NSIoPipe::Input(PORT_IMGI, inputBuffer.get()));
            frameParam.mvOut.push_back(NSCam::NSIoPipe::Output(PORT_WROTO, outputBuffer.get()));
            qParams.mvFrameParams.push_back(frameParam);

            Callback callback;
            callback.fill(qParams);

            normalStream->enque(qParams);

            EXPECT_TRUE(callback.waitForCallback(1));

            sleepRandom();
        }
    };

    auto threadIpRawCallback = [normalStream, &heapRAII] () {
        SleepRandom sleepRandom(0, 5);
        sp<IImageBuffer> inputBufferPureRaw = heapRAII.createBuffer();
        inputBufferPureRaw->setImgDesc(eIMAGE_DESC_ID_RAW_TYPE, eIMAGE_DESC_RAW_TYPE_PURE);
        sp<IImageBuffer> outputBuffer = heapRAII.createBuffer();

        for (int i = 0; i < N_ITERS; i++) {
            QParams qParams;
            FrameParams frameParam;
            frameParam.mvIn.push_back(NSCam::NSIoPipe::Input(PORT_IMGI, inputBufferPureRaw.get()));
            frameParam.mvOut.push_back(NSCam::NSIoPipe::Output(PORT_WROTO, outputBuffer.get()));
            qParams.mvFrameParams.push_back(frameParam);

            Callback callback;
            callback.fill(qParams);

            normalStream->enque(qParams);

            EXPECT_TRUE(callback.waitForCallback(1));

            sleepRandom();
        }
    };


    auto threadIpRawAnother = [&heapRAII, &driverFactory] () {
        SleepRandom sleepRandom(0, 5);
        sp<IImageBuffer> inputBufferPureRaw = heapRAII.createBuffer();
        inputBufferPureRaw->setImgDesc(eIMAGE_DESC_ID_RAW_TYPE, eIMAGE_DESC_RAW_TYPE_PURE);
        sp<IImageBuffer> outputBuffer = heapRAII.createBuffer();

        for (int i = 0; i < N_ITERS; i++) {
            INormalStream *normalStream = NormalStreamUpper::createInstance(0, driverFactory);
            normalStream->init(LOG_TAG);

            QParams qParams;
            FrameParams frameParam;
            frameParam.mvIn.push_back(NSCam::NSIoPipe::Input(PORT_IMGI, inputBufferPureRaw.get()));
            frameParam.mvOut.push_back(NSCam::NSIoPipe::Output(PORT_WROTO, outputBuffer.get()));
            qParams.mvFrameParams.push_back(frameParam);

            Callback callback;
            callback.fill(qParams);

            normalStream->enque(qParams);

            EXPECT_TRUE(callback.waitForCallback(1));

            normalStream->uninit(LOG_TAG);
            normalStream->destroyInstance();

            sleepRandom();
        }
    };

    std::vector<std::future<void>> futures;
    futures.emplace_back(std::async(std::launch::async, threadDeque));
    futures.emplace_back(std::async(std::launch::async, threadNormalCallback));
    futures.emplace_back(std::async(std::launch::async, threadIpRawCallback));
    futures.emplace_back(std::async(std::launch::async, threadIpRawAnother));

    for (auto &f : futures)
        f.get();

    MBOOL isUninited = normalStream->uninit(LOG_TAG);
    EXPECT_TRUE(isUninited);

    EXPECT_TRUE(fakeP1.waitForReleased());
    EXPECT_TRUE(driverFactory.mNormalStream.mNumOfIpRawRequests == N_ITERS + N_ITERS / 2 + N_ITERS);

    normalStream->destroyInstance();
    EXPECT_EQ(0, driverFactory.mNormalStream.mNumOfInstances.load());

    fakeP1.mAcqEventHandle->unsubscribe();
    fakeP1.mRelEventHandle->unsubscribe();
}


int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


