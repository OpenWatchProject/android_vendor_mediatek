//
// Copyright 2016 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#if defined(MTK_BT_HAL_STATE_MACHINE) && (TRUE == MTK_BT_HAL_STATE_MACHINE)

#include "vendor_interface.h"

#define LOG_TAG "mtk.hal.bt.fake@1.0-impl"

#include <log/log.h>

namespace {

using android::hardware::hidl_vec;

}  // namespace

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

bool VendorInterface::Initialize(
    InitializeCompleteCallback initialize_complete_cb,
    PacketReadCallback event_cb, PacketReadCallback acl_cb,
    PacketReadCallback sco_cb) {
  ALOGE("%s: fake impl", __func__);
  hidl_vec<uint8_t> dummy;
  initialize_complete_cb(false);
  event_cb(dummy);
  acl_cb(dummy);
  sco_cb(dummy);
  return false;
}

void VendorInterface::Shutdown() {
  ALOGE("%s: fake impl", __func__);
}

VendorInterface* VendorInterface::get() {
  ALOGE("%s: fake impl", __func__);
  return nullptr;
}

bool VendorInterface::Open(InitializeCompleteCallback initialize_complete_cb,
                           PacketReadCallback event_cb,
                           PacketReadCallback acl_cb,
                           PacketReadCallback sco_cb) {
  ALOGE("%s: fake impl", __func__);
  lib_handle_ = nullptr;
  lib_interface_ = nullptr;
  hci_ = nullptr;
  firmware_startup_timer_ = nullptr;

  hidl_vec<uint8_t> dummy;
  initialize_complete_cb(false);
  event_cb(dummy);
  acl_cb(dummy);
  sco_cb(dummy);
  return false;
}

void VendorInterface::Close() {
  ALOGE("%s: fake impl", __func__);
}

size_t VendorInterface::Send(uint8_t type, const uint8_t* data, size_t length) {
  ALOGE("%s: fake impl type: %u, data: %p, length: %zu",
      __func__, type, data, length);
  return 0;
}

void VendorInterface::OnFirmwareConfigured(uint8_t result) {
  ALOGE("%s fake impl result: %d", __func__, result);
}

void VendorInterface::OnTimeout() {
  ALOGE("%s", __func__);
}

void VendorInterface::HandleIncomingEvent(const hidl_vec<uint8_t>& hci_packet) {
  ALOGE("%s: fake impl size: %zu", __func__, hci_packet.size());
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace bluetooth
}  // namespace hardware
}  // namespace android

#endif
