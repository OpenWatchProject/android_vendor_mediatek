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

#if defined(MTK_BT_HAL_DEBUG) && (TRUE == MTK_BT_HAL_DEBUG)

#define LOG_TAG "mtk.hal.bt-debugger"

#include "hci_hal_debugger.h"

#include <cutils/properties.h>
#include <inttypes.h>
#include <sys/time.h>
#include <time.h>
#include <utils/Log.h>

#include <climits>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

// To disable it before check in!
// #define MTK_BT_HAL_SELF_DEBUG FALSE

namespace vendor {
namespace mediatek {
namespace bluetooth {
namespace hal {

static constexpr char kBtHalDebugProperty[] = "persist.vendor.service.bthal.debug";

std::string PacketDirTypeToString(PacketDirectionType type) {
  switch (type) {
    case kTx: return std::string("TX");
    case kRx: return std::string("RX");
    default: return std::string("Unknown");
  }
}

constexpr int kMaxDumpPacketDataLength(16);
struct BtHciDebugPacket {
  PacketDirectionType dir_type;
  uint8_t packet_type;
  std::vector<uint8_t> data;
  uint16_t data_length;
  std::string time_tag;
};

constexpr int kCircularSize(30);
class BtHciDebuggerImpl {
 public:
  BtHciDebuggerImpl() :
      head_index_(0),
      count_(0),
      is_bt_hal_debug_on_(false),
      is_coredump_set_(false),
      lib_interface_(nullptr) {
    packets_info_.resize(kCircularSize);
  }
  ~BtHciDebuggerImpl() = default;

  void Archive(PacketDirectionType dir_type,
      uint8_t type,
      const uint8_t* data,
      uint16_t data_length) {
    // Adding mutex synchronization here may drag down TX/RX a bit
    // If it is with concern, it is could be removed and replaced
    // two separate TX/RX packets_info_ circular array to archive
    // packets data in the respective TX/RX thread
    std::lock_guard<std::mutex> lock(packets_mutex_);
    head_index_ = count_ % kCircularSize;
    uint16_t dump_len = (data_length < kMaxDumpPacketDataLength)
        ? data_length : kMaxDumpPacketDataLength;
    BtHciDebugPacket packet = {
        dir_type,
        type,
        std::vector<uint8_t>(data, data + dump_len),
        data_length,
        GetLogTimeTag()
    };
    packets_info_[head_index_] = packet;
#if defined(MTK_BT_HAL_SELF_DEBUG) && (TRUE == MTK_BT_HAL_SELF_DEBUG)
    ALOGW("%s: head_index_: %d, count_: %d", __func__, head_index_, count_);
    ALOGW("%s: %s, time:%s, packet type: %u, data len: %u, data: %s",
        __func__,
        PacketDirTypeToString(packet.dir_type).c_str(),
        packet.time_tag.c_str(),
        packet.packet_type,
        packet.data_length,
        DataArrayToString<uint8_t>(
            packet.data.data(), packet.data.size()).c_str());
#endif
    if (INT_MAX == (count_ + 1)) {
      count_ = 0;
    }
    count_++;
  }

  void OnNotify() const {
    for (int i = head_index_; i < kCircularSize; i++) {
      LogArchivedData(packets_info_[i]);
    }
    for (int i = 0; i < head_index_; i++) {
      LogArchivedData(packets_info_[i]);
    }
  }

  void RefreshHalDebugState() {
    char property[PROPERTY_VALUE_MAX] = {0};
    if (property_get(kBtHalDebugProperty, property, NULL)) {
      if (!strncmp("true", property, 4)) {
        is_bt_hal_debug_on_ = true;
        ALOGW("%s: bt hal debug enabled.", __func__);
      }
    }
  }

  void Dump(PacketDirectionType pkt_dir,
      uint8_t type, const uint8_t* data, size_t length) const {
    if (is_bt_hal_debug_on_) {
      size_t dump_len = length < kMaxDumpPacketDataLength
          ? length : kMaxDumpPacketDataLength;
      ALOGW("%s: %s, type %u, data len: %zu, data: %s", __func__,
          PacketDirTypeToString(pkt_dir).c_str(),
          type, length, DataArrayToString<uint8_t>(data, dump_len).c_str());
    }
  }

  void RefreshVendorInterface(const bt_vendor_interface_t *interface) {
    lib_interface_ = interface;
    is_coredump_set_ = false;
  }

  bool TriggerFirmwareAssert(int reason) {
    bool is_done(false);
    do {
      if (is_coredump_set_) {
        break;
      }
      ALOGW("%s triggering firmware core dump!", __func__);
      if (lib_interface_ == NULL) {
        ALOGE("%s Act FW Core dump Fails! Vender not set.", __func__);
        break;
      }
      if (!lib_interface_->op(BT_VND_OP_SET_FW_ASSERT, &reason)) {
        ALOGW("%s Act FW Core dump Success!", __func__);
        is_coredump_set_ = true;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        is_done = true;
      } else {
        ALOGW("%s Act FW Coredump Fails!", __func__);
      }
    } while (0);
    return is_done;
  }

 private:
  void LogArchivedData(const BtHciDebugPacket& packet) const {
    ALOGW("%s: %s, time:%s, packet type: %u, data len: %u, data: %s",
        __func__,
        PacketDirTypeToString(packet.dir_type).c_str(),
        packet.time_tag.c_str(),
        packet.packet_type,
        packet.data_length,
        DataArrayToString<uint8_t>(
            packet.data.data(), packet.data.size()).c_str());
  }

  size_t GetTimeofDayInMicroSec(void) const {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return static_cast<size_t>(tv.tv_usec);
  }

  std::string GetLogTimeTag() const {
    char curtime[64] = {0};
    struct tm* tmp = NULL;
    time_t lt;
    /* get current time */
    lt = time(NULL);
    tmp = localtime(&lt);
    if (tmp == NULL){
      ALOGE("%s: log time is NULL ", __func__);
      return curtime;
    }
    strftime(curtime, sizeof(curtime), "%m-%d %H:%M:%S", tmp);
#if defined(MTK_BT_HAL_SELF_DEBUG) && (TRUE == MTK_BT_HAL_SELF_DEBUG)
    ALOGW("%s: log time : %s.", __func__, curtime);
#endif
    size_t str_len = strlen(curtime);
    std::snprintf(curtime + str_len,
        sizeof(curtime) - str_len,
        ".%06zu", GetTimeofDayInMicroSec());
#if defined(MTK_BT_HAL_SELF_DEBUG) && (TRUE == MTK_BT_HAL_SELF_DEBUG)
    ALOGW("%s: log time with ms suffix: %s.", __func__, curtime);
#endif
    return std::string(curtime);
  }

  std::vector<BtHciDebugPacket> packets_info_;
  int head_index_;
  int count_;
  bool is_bt_hal_debug_on_;

  std::mutex packets_mutex_;

  // MTK controller core dump trigger mechanism
  bool is_coredump_set_;
  const bt_vendor_interface_t* lib_interface_;
};

BtHciDebugger::BtHciDebugger() : debugger_impl_(new BtHciDebuggerImpl()) {}

BtHciDebugger::~BtHciDebugger() {
  if (debugger_impl_) {
    debugger_impl_.reset();
    debugger_impl_ = nullptr;
  }
}

void BtHciDebugger::Archive(PacketDirectionType dir_type,
    uint8_t type,
    const uint8_t* data,
    uint16_t data_length) {
  debugger_impl_->Archive(dir_type, type, data, data_length);
}

void BtHciDebugger::OnNotify() const {
  debugger_impl_->OnNotify();
}

void BtHciDebugger::RefreshHalDebugState() {
  debugger_impl_->RefreshHalDebugState();
}

void BtHciDebugger::Dump(PacketDirectionType pkt_dir,
    uint8_t type, const uint8_t* data, size_t length) const {
  debugger_impl_->Dump(pkt_dir, type, data, length);
}

void BtHciDebugger::RefreshVendorInterface(
    const bt_vendor_interface_t *interface) {
  debugger_impl_->RefreshVendorInterface(interface);
}

bool BtHciDebugger::TriggerFirmwareAssert(int reason) {
  return debugger_impl_->TriggerFirmwareAssert(reason);
}

class BtHciDebugBulletinImpl {
 public:
  BtHciDebugBulletinImpl() = default;
  ~BtHciDebugBulletinImpl() = default;

  void Check(PacketDirectionType dir_type, const uint8_t* data) const {
    if (kTx == dir_type) {
      if (HandleTxData(data)) {
        for (auto ob : obs_) {
          ob->OnNotify();
        }
      }
/*
    } else if (kRx == dir_type) {
      if (HandleRxData(data)) {
        for (auto ob : obs_) {
          ob->OnNotify();
        }
      }
*/
    }
  }

  void AddObserver(const BtHciDebugger* ob) {
    obs_.push_back(ob);
  }

 private:
  bool HandleTxData(const uint8_t* data) const {
    // only need 4 byte to parse TX command
    const int kTxCmdHeadLen(4);
    std::vector<uint8_t> tx_cmd(data, data + kTxCmdHeadLen);
    if (kTxCmdHeadLen == tx_cmd.size()) {
      // case 1: 0xFD5B
      if ((0x5B == tx_cmd[0]) && (0xFD == tx_cmd[1])) {
        return true;
      }
    }
    return false;
  }

  bool HandleRxData(const uint8_t* data) const {
    // only need 4 byte to parse RX command
    const int kRxCmdHeadLen(4);
    std::vector<uint8_t> rx_evt(data, data + kRxCmdHeadLen);
    if (kRxCmdHeadLen == rx_evt.size()) {
      // case 1: HW Error event: 0x04, 0x10, 0x01, 0x00
      if ((0x10 == rx_evt[0])
          && (0x01 == rx_evt[1]) && (0x00 == rx_evt[2])) {
        return true;
      }
    }
    return false;
  }

  std::vector<const BtHciDebugger*> obs_;
};

BtHciDebugBulletin::BtHciDebugBulletin() : bulletin_impl_(
    new BtHciDebugBulletinImpl()) {}

BtHciDebugBulletin::~BtHciDebugBulletin() {
  if (bulletin_impl_) {
    bulletin_impl_.reset();
    bulletin_impl_ = nullptr;
  }
}

void BtHciDebugBulletin::Check(PacketDirectionType dir_type,
    const uint8_t* data) const {
  bulletin_impl_->Check(dir_type, data);
}

void BtHciDebugBulletin::AddObserver(const BtHciDebugger* ob) {
  bulletin_impl_->AddObserver(ob);
}

}  // namespace hal
}  // namespace bluetooth
}  // namespace mediatek
}  // namespace vendor

#endif
