/* Copyright Statement:
 * *
 * * This software/firmware and related documentation ("MediaTek Software") are
 * * protected under relevant copyright laws. The information contained herein
 * * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * * Without the prior written permission of MediaTek inc. and/or its licensors,
 * * any reproduction, modification, use or disclosure of MediaTek Software,
 * * and information contained herein, in whole or in part, shall be strictly prohibited.
 * *
 * * MediaTek Inc. (C) 2016. All rights reserved.
 * *
 * * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 * *
 * * The following software/firmware and/or related documentation ("MediaTek Software")
 * * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * * applicable license agreements with MediaTek Inc.
 * */

#pragma once

#if defined(MTK_BT_HAL_STATE_MACHINE) && (TRUE == MTK_BT_HAL_STATE_MACHINE)

#include <android/hardware/bluetooth/1.0/IBluetoothHciCallbacks.h>
#include <utils/StrongPointer.h>

#include <functional>
#include <memory>

#include "mtk_util.h"

namespace vendor {
namespace mediatek {
namespace bluetooth {
namespace hal {

enum BtHalStateMessage {
  kIdleMsg = 0,
  kOnMsg = 1,
  kOffMsg = 2,
};

class VendorStateObserver {
  public:
   virtual ~VendorStateObserver() = default;

   virtual void OnStateChanged(BtHalStateMessage state_msg) = 0;
};

using ::android::hardware::bluetooth::V1_0::IBluetoothHciCallbacks;
class VendorStateController {
 public:
  virtual ~VendorStateController() = default;

  virtual void InitHciCallbacks(
      const ::android::sp<IBluetoothHciCallbacks>& cb) = 0;
  virtual void TurnOn() const = 0;
  virtual void TurnOff() const = 0;
};

class StateMachineImpl;
class StateMachine : public Singleton<StateMachine> {
 public:
  ~StateMachine();

  int Start();
  void Stop();

  void InitHciCallbacks(
         const ::android::sp<IBluetoothHciCallbacks>& cb);

  // Off state is responsible to free controller
  void Transit(BtHalStateMessage state);

  /* Observe BT HAL state change, mainly for UT test right now */
  void AddObserver(VendorStateObserver* ob);
  void RemoveObserver(VendorStateObserver* ob);
  void Reset();
  /* Observe BT HAL state change, mainly for UT test right now */

  // To expose to receive controller from the external, mainly for UTTest
  void UpdateVendorController(VendorStateController* controller);

 private:
  StateMachine();

  friend class Singleton<StateMachine>;

  std::unique_ptr<StateMachineImpl> sm_impl_;
};

}  // namespace hal
}  // namespace bluetooth
}  // namespace mediatek
}  // namespace vendor

#endif
