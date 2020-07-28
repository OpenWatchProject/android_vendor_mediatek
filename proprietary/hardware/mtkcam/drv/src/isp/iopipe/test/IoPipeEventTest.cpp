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

#define LOG_TAG "IoPipeEventTest"

#include <atomic>
#include <future>
#include <gtest/gtest.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/drv/iopipe/Event/IoPipeEvent.h>


using namespace android;
using namespace NSCam::NSIoPipe;

namespace {

class FakeP1Node : public IoPipeEventSubscriber<IpRawP1ReleasedEvent>
{
public:
    std::atomic_int mP1AcquiringInvoked;
    std::atomic_int mP1Acquiring2Invoked;
    std::atomic_int mP1ReleasedInvoked;

    FakeP1Node() :
        mP1AcquiringInvoked(0),
        mP1Acquiring2Invoked(0),
        mP1ReleasedInvoked(0)
    {
    }

    void reset() {
        mP1AcquiringInvoked = 0;
        mP1Acquiring2Invoked = 0;
        mP1ReleasedInvoked = 0;
    }

    static constexpr unsigned int ACQ_RESULT = 123;
    static constexpr MINTPTR COOKIE = 456;
    static constexpr MINTPTR COOKIE2 = 789;

    static IoPipeEventCtrl onP1Acquiring(FakeP1Node *myself, IpRawP1AcquiringEvent &evt) {
        myself->mP1AcquiringInvoked++;
        evt.setResult(ACQ_RESULT); // not needed
        EXPECT_TRUE(evt.getCookie() == COOKIE);
        return IoPipeEventCtrl::OK;
    }

    static IoPipeEventCtrl onP1Acquiring2(FakeP1Node *myself, IpRawP1AcquiringEvent &evt) {
        myself->mP1Acquiring2Invoked++;
        EXPECT_TRUE(evt.getCookie() == COOKIE2);
        return IoPipeEventCtrl::OK;
    }

    IoPipeEventCtrl onIoPipeEvent(IpRawP1ReleasedEvent &) override {
        mP1ReleasedInvoked++;
        return IoPipeEventCtrl::OK;
    }
};

}


TEST(IoPipeEventTest, Basic)
{
    // NOTE:
    // use IoPipeEventSystem &eventSystem = IoPipeEventSystem::getGlobal();
    // to get global event system. The local system here is only for UT
    IoPipeEventSystem eventSystem("test");

    FakeP1Node p1;

    // Nobody subscribes yet, send shall not crash
    IpRawP1AcquiringEvent acqEvent;
    IoPipeEventCtrl ctrl = eventSystem.sendSyncEvent(acqEvent);
    EXPECT_EQ(IoPipeEventCtrl::OK, ctrl);

    // The original & most flexible API
    // Allow to accept procedure-styled function
    sp<IoPipeEventHandle> acqHandle = eventSystem.subscribe(
        EVT_IPRAW_P1_ACQUIRING, FakeP1Node::onP1Acquiring, &p1, FakeP1Node::COOKIE);
    ASSERT_NE(nullptr, acqHandle.get());

    // IpRawP1AcquiringEvent is strongly associated to the event ID
    sp<IoPipeEventHandle> acq2Handle = eventSystem.subscribe(
        FakeP1Node::onP1Acquiring2, &p1, FakeP1Node::COOKIE2);
    ASSERT_NE(nullptr, acq2Handle.get());

    // If FakeP1Node inherits multiple IoPipeEventSubscriber<>
    // We can explicitly the event class
    // EVT_IPRAW_P1_RELEASED can be omitted (as below)
    sp<IoPipeEventHandle> relHandle = eventSystem.subscribe<IpRawP1ReleasedEvent>(
        EVT_IPRAW_P1_RELEASED, p1, FakeP1Node::COOKIE);
    ASSERT_NE(nullptr, relHandle.get());

    p1.reset();

    eventSystem.sendSyncEvent(acqEvent);
    EXPECT_TRUE(acqEvent.getResult() == FakeP1Node::ACQ_RESULT);
    EXPECT_EQ(1, p1.mP1AcquiringInvoked.load());
    EXPECT_EQ(1, p1.mP1Acquiring2Invoked.load());

    p1.reset();

    acqHandle->unsubscribe();
    eventSystem.sendSyncEvent(acqEvent);
    EXPECT_EQ(0, p1.mP1AcquiringInvoked.load());
    EXPECT_EQ(1, p1.mP1Acquiring2Invoked.load());

    IpRawP1ReleasedEvent relEvent;
    eventSystem.sendSyncEvent(relEvent);
    EXPECT_EQ(1, p1.mP1Acquiring2Invoked.load()); // not affect
    EXPECT_EQ(1, p1.mP1ReleasedInvoked.load());

    relHandle->unsubscribe();
    EXPECT_FALSE(relHandle->isValid());

    eventSystem.sendSyncEvent(relEvent);
    EXPECT_EQ(1, p1.mP1ReleasedInvoked.load());

    eventSystem.unsubscribe(acq2Handle);
    eventSystem.sendSyncEvent(acqEvent);
    // the same with previous state
    EXPECT_EQ(0, p1.mP1AcquiringInvoked.load());
    EXPECT_EQ(1, p1.mP1Acquiring2Invoked.load());

    // acq2Handle will print warning since not unsibscribed
}


TEST(IoPipeEventTest, MultiSubscr)
{
    // NOTE:
    // use IoPipeEventSystem &eventSystem = IoPipeEventSystem::getGlobal();
    // to get global event system. The local system here is only for UT
    IoPipeEventSystem eventSystem("test");

    FakeP1Node p1a, p1b;

    sp<IoPipeEventHandle> acqHandle = eventSystem.subscribe(
        EVT_IPRAW_P1_ACQUIRING, FakeP1Node::onP1Acquiring, &p1a, FakeP1Node::COOKIE);

    // doubly subscribe
    // A warning message will be printed
    sp<IoPipeEventHandle> acqHandle2 = eventSystem.subscribe(
        EVT_IPRAW_P1_ACQUIRING, FakeP1Node::onP1Acquiring, &p1a, FakeP1Node::COOKIE);

    // Different user
    sp<IoPipeEventHandle> acqHandle3 = eventSystem.subscribe(
        EVT_IPRAW_P1_ACQUIRING, FakeP1Node::onP1Acquiring, &p1b, FakeP1Node::COOKIE);

    // Will get the same handle
    EXPECT_TRUE(acqHandle == acqHandle2);
    EXPECT_TRUE(acqHandle != acqHandle3);

    p1a.reset();

    IpRawP1AcquiringEvent acqEvent;
    eventSystem.sendSyncEvent(acqEvent);
    // Only receive once
    EXPECT_EQ(1, p1a.mP1AcquiringInvoked.load());
    // Both users received
    EXPECT_EQ(1, p1b.mP1AcquiringInvoked.load());

    acqHandle->unsubscribe();
    // will fail and print warning
    acqHandle2->unsubscribe();
    acqHandle3->unsubscribe();
}


TEST(IoPipeEventTest, Racing)
{
    IoPipeEventSystem eventSystem("test");
    FakeP1Node p1a, p1b;
    static const int N_ITER = 10000;

    // t1 sends RELEASED to t2, receives ACQUIRING from t2
    sp<IoPipeEventHandle> acqHandle = eventSystem.subscribe(
        EVT_IPRAW_P1_ACQUIRING, FakeP1Node::onP1Acquiring, &p1a, FakeP1Node::COOKIE);

    auto t1 = [&eventSystem, &p1a] {
        for (int i = 0; i < N_ITER; i++) {
            IpRawP1ReleasedEvent relEvent;
            eventSystem.sendSyncEvent(relEvent);
        }
    };

    auto t2 = [&eventSystem, &p1b] {
        for (int i = 0; i < N_ITER; i++) {
             sp<IoPipeEventHandle> acqHandle2 = eventSystem.subscribe(
                EVT_IPRAW_P1_ACQUIRING, FakeP1Node::onP1Acquiring, &p1b, FakeP1Node::COOKIE);
            sp<IoPipeEventHandle> relHandle = eventSystem.subscribe(
                p1b, FakeP1Node::COOKIE);

            IpRawP1AcquiringEvent acqEvent;
            eventSystem.sendSyncEvent(acqEvent);

            relHandle->unsubscribe();
            acqHandle2->unsubscribe();
        }
    };

    std::future<void> future_t1 = std::async(std::launch::async, t1);
    std::future<void> future_t2 = std::async(std::launch::async, t2);

    future_t1.get();
    future_t2.get();

    EXPECT_TRUE(p1a.mP1AcquiringInvoked == N_ITER);

    acqHandle->unsubscribe();
}


TEST(IoPipeEventTest, AutoUnsubscribe)
{
    sp<IoPipeEventHandle> handle;
    FakeP1Node p1;

    {
        IoPipeEventSystem eventSystem("test");
        handle = eventSystem.subscribe(
            EVT_IPRAW_P1_ACQUIRING, FakeP1Node::onP1Acquiring, &p1, FakeP1Node::COOKIE);
        EXPECT_TRUE(handle->isValid());
    }

    EXPECT_FALSE(handle->isValid());
    // Will not NE
    handle->unsubscribe();
}


int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

