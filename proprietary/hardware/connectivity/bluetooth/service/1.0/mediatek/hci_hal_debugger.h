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

#if defined(MTK_BT_HAL_DEBUG) && (TRUE == MTK_BT_HAL_DEBUG)

#define MTK_BT_HAL_H4_DEBUG TRUE

#include <stdint.h>

#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

#include "bt_vendor_lib.h"
#include "mtk_util.h"

enum FirmwareAssertReason {
  kUnknownError = 30,
  kCommandTimedout = 31,
  kInvalidEventData = 32,
};

namespace vendor {
namespace mediatek {
namespace bluetooth {
namespace hal {

template <typename T>
std::string DataArrayToString(const T* arr, size_t array_len) {
  std::stringstream ss;
  int temp(0);
  for (size_t i(0); i < array_len; i++) {
    temp = static_cast<int>(arr[i]);
    ss << " " << std::uppercase << std::setfill('0') << std::hex << std::setw(2)
       << temp;
  }
  return ss.str();
}

enum PacketDirectionType {
  kTx,
  kRx,
};

std::string PacketDirTypeToString(PacketDirectionType type);

class BtHciDebuggerImpl;
class BtHciDebugger : public Singleton<BtHciDebugger> {
 public:
  ~BtHciDebugger();

  void Archive(PacketDirectionType dir_type,
      uint8_t type,
      const uint8_t* data,
      uint16_t data_length);
  void OnNotify() const;
  void RefreshHalDebugState();
  void Dump(PacketDirectionType pkt_dir,
      uint8_t type, const uint8_t* data, size_t length) const;

  void RefreshVendorInterface(const bt_vendor_interface_t *interface);
  bool TriggerFirmwareAssert(int reason);

 private:
  BtHciDebugger();

  friend class Singleton<BtHciDebugger>;

  std::unique_ptr<BtHciDebuggerImpl> debugger_impl_;
};

class BtHciDebugBulletinImpl;
class BtHciDebugBulletin : public Singleton<BtHciDebugBulletin> {
  public:
   ~BtHciDebugBulletin();

   void Check(PacketDirectionType dir_type, const uint8_t* data) const;
   void AddObserver(const BtHciDebugger* ob);

  private:
   BtHciDebugBulletin();

   friend class Singleton<BtHciDebugBulletin>;

   std::unique_ptr<BtHciDebugBulletinImpl> bulletin_impl_;
};

}  // namespace hal
}  // namespace bluetooth
}  // namespace mediatek
}  // namespace vendor

#endif
