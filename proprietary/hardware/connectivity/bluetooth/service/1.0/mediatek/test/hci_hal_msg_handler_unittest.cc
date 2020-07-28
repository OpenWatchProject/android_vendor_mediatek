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

#define LOG_TAG "mtk.hal.bt-msg-handler-unittest"

#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <log/log.h>
#include <utils/Looper.h>

#include "hci_hal_msg_handler.h"
#include "mtk_util.h"
#include "TestHelpers.h"

using ::android::DelayedTask;
using ::android::Message;
using ::android::MessageHandler;
using ::android::sp;

using ::testing::InSequence;

namespace {

enum BtTestHanlderMsg {
  kOnMsg = 0,
  kOnAgainMsg = 1,
  kOffMsg = 2,
  kOffAgainMsg = 3,
};

using vendor::mediatek::bluetooth::hal::BtHalMessageLooper;

class DelayedSender : public DelayedTask {
  public:
   DelayedSender(int delay_millis,
       BtHalMessageLooper* looper,
       const android::sp<android::MessageHandler>& handler,
       BtTestHanlderMsg msg)
     : DelayedTask(delay_millis),
       looper_(looper),
       handler_(handler),
       msg_(msg) {}

   void doTask() override {
     looper_->EnqueueMessage(handler_, Message(msg_));
     ALOGI("%s: sent %d", __func__, msg_);
   }

   const BtHalMessageLooper* looper_;
   const android::sp<android::MessageHandler>& handler_;
   BtTestHanlderMsg msg_;

  private:
   DelayedSender(const DelayedSender&);
   DelayedSender& operator = (const DelayedSender&);
};

class MockBtMessageHandler : public MessageHandler {
 public:
  MOCK_METHOD1(handleMessage, void(const Message& message));
};

MATCHER_P(MsgHandlerMatcher, msg, "") {
  return (msg == static_cast<BtTestHanlderMsg>(arg.what));
}

}  // anonymous namespace

namespace vendor {
namespace mediatek {
namespace bluetooth {
namespace hal {

class HciHalMsgHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ALOGI("%s: HciHalMsgHandlerTest", __func__);
    mock_handler_ = new MockBtMessageHandler();
    bt_hal_msg_looper_  = std::unique_ptr<BtHalMessageLooper>(
        new BtHalMessageLooper());
    bt_hal_msg_looper_->StartMessageLooper(std::string("TestLooperThread"));
    // Need to wait for while for looper thread being started
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  void TearDown() override {
    ALOGI("%s: HciHalMsgHandlerTest", __func__);
    bt_hal_msg_looper_->StopMessageLooper();
    mock_handler_.clear();
  }

  std::unique_ptr<BtHalMessageLooper> bt_hal_msg_looper_;
  sp<MockBtMessageHandler> mock_handler_;
};

TEST_F(HciHalMsgHandlerTest, BasicOnOff) {
  EXPECT_CALL(*(mock_handler_), handleMessage(MsgHandlerMatcher(kOffMsg)))
          .Times(1);
  EXPECT_CALL(*(mock_handler_), handleMessage(MsgHandlerMatcher(kOnMsg)))
          .Times(1);
  bt_hal_msg_looper_->EnqueueMessage(mock_handler_, Message(kOnMsg));
  bt_hal_msg_looper_->EnqueueMessage(mock_handler_, Message(kOffMsg));
}

TEST_F(HciHalMsgHandlerTest, ThreadingTaskOnOff) {
  const BtTestHanlderMsg kMsgs[] = {kOnMsg, kOnAgainMsg, kOffMsg, kOffAgainMsg,};
  const size_t kMsgsSize = array_length(kMsgs);

  {
    InSequence s;
    for (size_t i(0); i < kMsgsSize; i++) {
      EXPECT_CALL(*(mock_handler_), handleMessage(MsgHandlerMatcher(kMsgs[i])))
              .Times(1);
    }
  }

  std::vector<sp<DelayedSender>> senders;
  for (size_t i(0); i < kMsgsSize; i++) {
    senders.emplace_back(
        new DelayedSender((i + 1) *100,
            bt_hal_msg_looper_.get(), mock_handler_, kMsgs[i])
    );
    // Thread name must be constant string
    senders[i]->run("sender");
  }
  ALOGI("%s: start threads to run tasks", __func__);

  for (auto& sender : senders) {
    sender->join();
  }
  ALOGI("%s: ThreadingTaskOnOff exited", __func__);
}

}  // namespace hal
}  // namespace bluetooth
}  // namespace mediatek
}  // namespace vendor

#endif
