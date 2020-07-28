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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#if defined(MTK_BT_HAL_STATE_MACHINE) && (TRUE == MTK_BT_HAL_STATE_MACHINE)

#define LOG_TAG "mtk.hal.bt-state-machine-unittest"

#include <chrono>
#include <memory>
#include <thread>

#include <android/hardware/bluetooth/1.0/IBluetoothHciCallbacks.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <log/log.h>

#include "hci_hal_state_machine.h"
#include "TestHelpers.h"

using ::testing::_;
using ::testing::Invoke;

namespace {

using ::android::DelayedTask;
using ::android::hardware::bluetooth::V1_0::IBluetoothHciCallbacks;
using ::android::hardware::bluetooth::V1_0::Status;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

using vendor::mediatek::bluetooth::hal::BtHalStateMessage;
using vendor::mediatek::bluetooth::hal::StateMachine;
using vendor::mediatek::bluetooth::hal::VendorStateController;
using vendor::mediatek::bluetooth::hal::VendorStateObserver;

constexpr int kTestWaitingTimeInMs = 500;

class StateTask : public DelayedTask {
  public:
    StateTask(int delay_millis,
       int delay_millis_after_task,
       StateMachine* sm,
       BtHalStateMessage msg)
     : DelayedTask(delay_millis),
       delay_after_work_(delay_millis_after_task),
       state_machine_(sm),
       msg_(msg) {}

   void doTask() override {
     if (BtHalStateMessage::kOnMsg == msg_) {
       state_machine_->InitHciCallbacks(nullptr);
     }
     state_machine_->Transit(msg_);
     ALOGI("%s: executed %d and to wait %d ms to be done",
         __func__, msg_, delay_after_work_);
     std::this_thread::sleep_for(std::chrono::milliseconds(delay_after_work_));
   }

   int delay_after_work_;
   StateMachine* state_machine_;
   BtHalStateMessage msg_;

  private:
   StateTask(const StateTask&);
   StateTask& operator = (const StateTask&);
};

class MockBtVendorStateObserver : public VendorStateObserver {
 public:
  MOCK_METHOD1(OnStateChanged, void(BtHalStateMessage state_msg));
};

class MockBtVendorStateController : public VendorStateController {
 public:
  MOCK_METHOD1(InitHciCallbacks,
      void(const ::android::sp<IBluetoothHciCallbacks>& cb));
  MOCK_CONST_METHOD0(TurnOn, void(void));
  MOCK_CONST_METHOD0(TurnOff, void(void));
};

class FakeBtVendorStateController : public VendorStateController {
  public:
   FakeBtVendorStateController(int on_delay_millis, int off_delay_millis)
     : on_delay_millis_(on_delay_millis),
       off_delay_millis_(off_delay_millis) {}

   MOCK_METHOD1(InitHciCallbacks,
       void(const ::android::sp<IBluetoothHciCallbacks>& cb));
   MOCK_CONST_METHOD0(TurnOn, void(void));
   MOCK_CONST_METHOD0(TurnOff, void(void));

   void DelegateToFake() {
     ON_CALL(*this, TurnOn())
         .WillByDefault(Invoke(this, &FakeBtVendorStateController::FakeTurnOn));
     ON_CALL(*this, TurnOff())
         .WillByDefault(Invoke(this, &FakeBtVendorStateController::FakeTurnOff));
   }

   void FakeTurnOn() const {
     ALOGI("%s: executed and to wait %d ms to be done",
         __func__, on_delay_millis_);
     std::this_thread::sleep_for(std::chrono::milliseconds(on_delay_millis_));
   }

   void FakeTurnOff() const {
     ALOGI("%s: executed and to wait %d ms to be done",
         __func__, off_delay_millis_);
     std::this_thread::sleep_for(std::chrono::milliseconds(off_delay_millis_));
   }

   int on_delay_millis_;
   int off_delay_millis_;
};

MATCHER_P(BtHalStateChangedMatcher, expected, "") {
  return (expected == arg);
}

}  // anonymous namespace

namespace vendor {
namespace mediatek {
namespace bluetooth {
namespace hal {

class HciHalStateMachineTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ALOGI("%s: HciHalStateMachineTest", __func__);
    mock_bt_state_ob_.reset(new MockBtVendorStateObserver());
  }

  void TearDown() override {
    StateMachine::GetInstance()->Stop();
    state_machine_->RemoveObserver(mock_bt_state_ob_.get());
    mock_bt_state_ob_.reset();
    ALOGI("%s: HciHalStateMachineTest", __func__);
  }

  void Init(VendorStateController* controller) {
    state_machine_ = StateMachine::GetInstance();
    state_machine_->UpdateVendorController(controller);
    state_machine_->AddObserver(mock_bt_state_ob_.get());
    state_machine_->Reset();
    EXPECT_EQ(0, state_machine_->Start());
    // Need to wait for while for looper thread being started
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  std::unique_ptr<MockBtVendorStateObserver> mock_bt_state_ob_;
  StateMachine* state_machine_;
};

TEST_F(HciHalStateMachineTest, BasicOnOff) {
  std::unique_ptr<MockBtVendorStateController> mock_bt_state_ctl(
      new MockBtVendorStateController());
  Init(mock_bt_state_ctl.get());
  const size_t kTestTimes(2);

  EXPECT_CALL(*(mock_bt_state_ob_), OnStateChanged(BtHalStateChangedMatcher(kOffMsg)))
          .Times(kTestTimes);
  EXPECT_CALL(*(mock_bt_state_ctl), TurnOff()).Times(kTestTimes);

  EXPECT_CALL(*(mock_bt_state_ob_), OnStateChanged(BtHalStateChangedMatcher(kOnMsg)))
          .Times(kTestTimes);
  EXPECT_CALL(*(mock_bt_state_ctl), TurnOn()).Times(kTestTimes);
  EXPECT_CALL(*(mock_bt_state_ctl), InitHciCallbacks(_)).Times(kTestTimes);

  for (size_t i(0); i < kTestTimes; i++) {
    state_machine_->InitHciCallbacks(nullptr);
    state_machine_->Transit(kOnMsg);
    state_machine_->Transit(kOffMsg);
  }

  // Wait for a while to make sure the tasks are handled - avoid controller nullptr crash
  std::this_thread::sleep_for(std::chrono::milliseconds(kTestWaitingTimeInMs));
}

TEST_F(HciHalStateMachineTest, OffInMiddleOfOnRequest) {
  std::unique_ptr<MockBtVendorStateController> mock_bt_state_ctl(
      new MockBtVendorStateController());
  Init(mock_bt_state_ctl.get());

  EXPECT_CALL(*(mock_bt_state_ob_), OnStateChanged(BtHalStateChangedMatcher(kOffMsg)))
          .Times(1);
  EXPECT_CALL(*(mock_bt_state_ctl), TurnOff()).Times(1);

  EXPECT_CALL(*(mock_bt_state_ob_), OnStateChanged(BtHalStateChangedMatcher(kOnMsg)))
          .Times(1);
  EXPECT_CALL(*(mock_bt_state_ctl), TurnOn()).Times(1);
  EXPECT_CALL(*(mock_bt_state_ctl), InitHciCallbacks(_)).Times(1);

  sp<StateTask> on_task(new StateTask(100, 100, state_machine_, kOnMsg));
  // Off request is requested to start in the middle of On request
  sp<StateTask> off_task(new StateTask(150, 0, state_machine_, kOffMsg));
  on_task->run("on");
  off_task->run("off");

  on_task->join();
  off_task->join();

  // Wait for a while to make sure the tasks are handled
  std::this_thread::sleep_for(std::chrono::milliseconds(kTestWaitingTimeInMs));
}

TEST_F(HciHalStateMachineTest, DoubleOnInARowRequest) {
  std::unique_ptr<MockBtVendorStateController> mock_bt_state_ctl(
      new MockBtVendorStateController());
  Init(mock_bt_state_ctl.get());

  EXPECT_CALL(*(mock_bt_state_ctl), TurnOff()).Times(1);

  EXPECT_CALL(*(mock_bt_state_ob_), OnStateChanged(BtHalStateChangedMatcher(kOnMsg)))
          .Times(2);
  EXPECT_CALL(*(mock_bt_state_ctl), TurnOn()).Times(2);
  EXPECT_CALL(*(mock_bt_state_ctl), InitHciCallbacks(_)).Times(2);

  sp<StateTask> on_task(new StateTask(0, 0, state_machine_, kOnMsg));
  // Let's check double On case
  sp<StateTask> on2_task(new StateTask(50, 0, state_machine_, kOnMsg));
  on_task->run("on");
  on2_task->run("on2");

  on_task->join();
  on2_task->join();

  // Wait for a while to make sure the tasks are handled
  std::this_thread::sleep_for(std::chrono::milliseconds(kTestWaitingTimeInMs));
}

TEST_F(HciHalStateMachineTest, OnInMiddlerOfAnotherOnRequest) {
  std::unique_ptr<MockBtVendorStateController> mock_bt_state_ctl(
      new MockBtVendorStateController());
  Init(mock_bt_state_ctl.get());

  EXPECT_CALL(*(mock_bt_state_ctl), TurnOff()).Times(1);

  EXPECT_CALL(*(mock_bt_state_ob_), OnStateChanged(BtHalStateChangedMatcher(kOnMsg)))
          .Times(2);
  EXPECT_CALL(*(mock_bt_state_ctl), TurnOn()).Times(2);
  EXPECT_CALL(*(mock_bt_state_ctl), InitHciCallbacks(_)).Times(2);

  sp<StateTask> on_task(new StateTask(0, 100, state_machine_, kOnMsg));
  // Let's check double On case
  sp<StateTask> on2_task(new StateTask(50, 100, state_machine_, kOnMsg));
  on_task->run("on");
  on2_task->run("on2");

  on_task->join();
  on2_task->join();

  // Wait for a while to make sure the tasks are handled
  std::this_thread::sleep_for(std::chrono::milliseconds(kTestWaitingTimeInMs));
}

TEST_F(HciHalStateMachineTest, OffInMiddleOfOnTask) {
  FakeBtVendorStateController fake_bt_state_ctl(100, 0);
  fake_bt_state_ctl.DelegateToFake();
  Init(&fake_bt_state_ctl);

  EXPECT_CALL(*(mock_bt_state_ob_), OnStateChanged(BtHalStateChangedMatcher(kOffMsg)))
          .Times(1);
  EXPECT_CALL(fake_bt_state_ctl, TurnOff()).Times(1);

  EXPECT_CALL(*(mock_bt_state_ob_), OnStateChanged(BtHalStateChangedMatcher(kOnMsg)))
          .Times(1);
  EXPECT_CALL(fake_bt_state_ctl, TurnOn()).Times(1);
  EXPECT_CALL(fake_bt_state_ctl, InitHciCallbacks(_)).Times(1);

  sp<StateTask> on_task(new StateTask(0, 0, state_machine_, kOnMsg));
  // Off task is requested to start in the middle of On task
  sp<StateTask> off_task(new StateTask(50, 0, state_machine_, kOffMsg));
  on_task->run("on");
  off_task->run("off");

  on_task->join();
  off_task->join();

  // Wait for a while to make sure the tasks are handled
  std::this_thread::sleep_for(std::chrono::milliseconds(kTestWaitingTimeInMs));
}

TEST_F(HciHalStateMachineTest, OnInMiddleOfTask) {
  FakeBtVendorStateController fake_bt_state_ctl(0, 200);
  fake_bt_state_ctl.DelegateToFake();
  Init(&fake_bt_state_ctl);

  EXPECT_CALL(*(mock_bt_state_ob_), OnStateChanged(BtHalStateChangedMatcher(kOffMsg)))
          .Times(1);
  EXPECT_CALL(fake_bt_state_ctl, TurnOff()).Times(1);

  EXPECT_CALL(*(mock_bt_state_ob_), OnStateChanged(BtHalStateChangedMatcher(kOnMsg)))
          .Times(2);
  EXPECT_CALL(fake_bt_state_ctl, TurnOn()).Times(2);
  EXPECT_CALL(fake_bt_state_ctl, InitHciCallbacks(_)).Times(2);

  sp<StateTask> on_task(new StateTask(0, 0, state_machine_, kOnMsg));
  sp<StateTask> off_task(new StateTask(50, 0, state_machine_, kOffMsg));
  on_task->run("on");
  // On task comes in the middle of Off task
  sp<StateTask> on2_task(new StateTask(100, 0, state_machine_, kOnMsg));
  on_task->run("on");
  off_task->run("off");
  on2_task->run("onagain");

  on_task->join();
  off_task->join();
  on2_task->join();

  // Wait for a while to make sure the tasks are handled
  std::this_thread::sleep_for(std::chrono::milliseconds(kTestWaitingTimeInMs));
}

}  // namespace hal
}  // namespace bluetooth
}  // namespace mediatek
}  // namespace vendor

#endif
