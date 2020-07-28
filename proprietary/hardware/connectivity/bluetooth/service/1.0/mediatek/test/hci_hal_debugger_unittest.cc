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

#if defined(MTK_BT_HAL_DEBUG) && (TRUE == MTK_BT_HAL_DEBUG)

#define LOG_TAG "mtk.bt.logtool-unittest"

#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <log/log.h>

#include "bt_vendor_lib.h"
#include "hci_hal_debugger.h"

using ::testing::Invoke;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::_;

namespace {

int FakeInit(const bt_vendor_callbacks_t* p_cb,
      unsigned char* local_bdaddr) {
  ALOGI("%s: p_cb: %p, local_bdaddr: %p", __func__, p_cb, local_bdaddr);
  return 0;
}

int FakeOp(bt_vendor_opcode_t opcode, void* param) {
  int ret(0);
  switch (opcode) {
    case BT_VND_OP_USERIAL_OPEN: {
      ret = 1;
      break;
    }
    case BT_VND_OP_USERIAL_CLOSE:
      break;
    case BT_VND_OP_SET_FW_ASSERT: {
      LOG_ALWAYS_FATAL_IF(!param, "%s: invalid input!",__func__);
      if (31 == *((int*)param) || 32 == *((int*)param)) {
        ret = 0;
      } else {
        ret = -1;
      }
      break;
    }
    default: {
      // fake OP code not implement yet
      ret = -1;
      break;
    }
  }
  return ret;
}

void FakeCleanUp() {}

class MockBtVendorLib {
 public:
  MockBtVendorLib() {
    ON_CALL(*this, Init(_,_))
        .WillByDefault(Invoke(&FakeInit));
    ON_CALL(*this, Operate(_,_))
        .WillByDefault(Invoke(&FakeOp));
    ON_CALL(*this, CleanUp())
        .WillByDefault(Invoke(&FakeCleanUp));
  }

  MOCK_METHOD2(Init, int(const bt_vendor_callbacks_t* p_cb,
      unsigned char* local_bdaddr));
  MOCK_METHOD2(Operate, int(bt_vendor_opcode_t opcode, void* param));
  MOCK_METHOD0(CleanUp, void());
};

using vendor::mediatek::bluetooth::hal::BtHciDebugger;

MockBtVendorLib* mock_bt_vendor_lib_;
}  // anonymous namespace

namespace vendor {
namespace mediatek {
namespace bluetooth {
namespace hal {

class HalH4DebuggerTest : public ::testing::Test {
 public:
  HalH4DebuggerTest()
      : fake_bt_vendor_if_{
          sizeof(fake_bt_vendor_if_),
          nullptr,
          nullptr,
          nullptr,
        } {}

 protected:
  void SetUp() override {
    mock_bt_vendor_lib_  = new MockBtVendorLib();

    fake_bt_vendor_if_.init = DelegateFakeInit;
    fake_bt_vendor_if_.op = DelegateFakeOp;
    fake_bt_vendor_if_.cleanup = DelegateCleanUp;

    EXPECT_CALL(*(mock_bt_vendor_lib_), Init(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*(mock_bt_vendor_lib_), Operate(_, _)).WillOnce(Return(1));

    EXPECT_TRUE((0 == fake_bt_vendor_if_.init(nullptr, nullptr)));
    int fd(-1);
    EXPECT_TRUE((1 == fake_bt_vendor_if_.op(BT_VND_OP_USERIAL_OPEN, &fd)));
    BtHciDebugger::GetInstance()->RefreshVendorInterface(&fake_bt_vendor_if_);
  }

  void TearDown() override {
    EXPECT_CALL(*(mock_bt_vendor_lib_), Operate(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*(mock_bt_vendor_lib_), CleanUp()).WillOnce(Return());

    EXPECT_TRUE((0 == fake_bt_vendor_if_.op(BT_VND_OP_USERIAL_CLOSE, nullptr)));
    fake_bt_vendor_if_.cleanup();

    delete mock_bt_vendor_lib_;
    mock_bt_vendor_lib_ = nullptr;
  }

  static int DelegateFakeInit(const bt_vendor_callbacks_t* p_cb,
      unsigned char* local_bdaddr) {
    return mock_bt_vendor_lib_->Init(p_cb, local_bdaddr);
  }

  static int DelegateFakeOp(bt_vendor_opcode_t opcode, void* param) {
    return mock_bt_vendor_lib_->Operate(opcode, param);
  }

  static void DelegateCleanUp() {
    mock_bt_vendor_lib_->CleanUp();
  }

  bt_vendor_interface_t fake_bt_vendor_if_;
};

TEST_F(HalH4DebuggerTest, TriggerCoreDump) {
  const int kRepeatTimes(3);
  EXPECT_CALL(*(mock_bt_vendor_lib_), Operate(_, _))
          .Times(kRepeatTimes)
          .WillRepeatedly(Invoke(&FakeOp));
  EXPECT_TRUE(BtHciDebugger::GetInstance()->TriggerFirmwareAssert(kInvalidEventData));
  // ONLY trigger once, skip once mock call
  EXPECT_FALSE(BtHciDebugger::GetInstance()->TriggerFirmwareAssert(kCommandTimedout));

  // Simulate reload vendor library again
  BtHciDebugger::GetInstance()->RefreshVendorInterface(&fake_bt_vendor_if_);
  // Wrong reason
  EXPECT_FALSE(BtHciDebugger::GetInstance()->TriggerFirmwareAssert(kUnknownError));
  EXPECT_TRUE(BtHciDebugger::GetInstance()->TriggerFirmwareAssert(kCommandTimedout));
}

}  // namespace hal
}  // namespace bluetooth
}  // namespace mediatek
}  // namespace vendor

#endif
