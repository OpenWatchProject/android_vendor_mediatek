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

#define LOG_TAG "mtk.hal.bt@1.0-impl"
#include "bluetooth_hci.h"

#include <log/log.h>

#include "vendor_interface.h"

#if defined(MTK_BT_HAL_STATE_MACHINE) && (TRUE == MTK_BT_HAL_STATE_MACHINE)
#include "hci_hal_state_machine.h"
using vendor::mediatek::bluetooth::hal::BtHalStateMessage;
using vendor::mediatek::bluetooth::hal::StateMachine;
#endif

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

static const uint8_t HCI_DATA_TYPE_COMMAND = 1;
static const uint8_t HCI_DATA_TYPE_ACL = 2;
static const uint8_t HCI_DATA_TYPE_SCO = 3;

class BluetoothDeathRecipient : public hidl_death_recipient {
 public:
  BluetoothDeathRecipient(const sp<IBluetoothHci> hci)
    : mHci(hci), has_died_(false) {}

  virtual void serviceDied(
      uint64_t /*cookie*/,
      const wp<::android::hidl::base::V1_0::IBase>& /*who*/) {
    ALOGE("BluetoothDeathRecipient::serviceDied - Bluetooth service died");
    has_died_ = true;
#if defined(MTK_BT_HAL_STATE_MACHINE) && (TRUE == MTK_BT_HAL_STATE_MACHINE)
    BluetoothHci *hci = (BluetoothHci*)mHci.get();
    hci->UnlinkDeathReception();
    StateMachine::GetInstance()->Transit(BtHalStateMessage::kOffMsg);
#else
    mHci->close();
#endif
  }
  sp<IBluetoothHci> mHci;
  bool getHasDied() const { return has_died_; }
  void setHasDied(bool has_died) { has_died_ = has_died; }

 private:
  bool has_died_;
};

BluetoothHci::BluetoothHci()
    : death_recipient_(new BluetoothDeathRecipient(this)) {}

Return<void> BluetoothHci::initialize(
    const ::android::sp<IBluetoothHciCallbacks>& cb) {
  ALOGI("BluetoothHci::initialize()");
  if (cb == nullptr) {
    ALOGE("cb == nullptr! -> Unable to call initializationComplete(ERR)");
    return Void();
  }

  death_recipient_->setHasDied(false);
  cb->linkToDeath(death_recipient_, 0);
  unlink_cb_ = [cb](sp<BluetoothDeathRecipient>& death_recipient) {
    if (death_recipient->getHasDied())
      ALOGI("Skipping unlink call, service died.");
    else
      cb->unlinkToDeath(death_recipient);
  };

#if defined(MTK_BT_HAL_STATE_MACHINE) && (TRUE == MTK_BT_HAL_STATE_MACHINE)
  StateMachine::GetInstance()->Start();
  StateMachine::GetInstance()->InitHciCallbacks(cb);
  StateMachine::GetInstance()->Transit(BtHalStateMessage::kOnMsg);
#else
  bool rc = VendorInterface::Initialize(
      [cb](bool status) {
        auto hidl_status = cb->initializationComplete(
            status ? Status::SUCCESS : Status::INITIALIZATION_ERROR);
        if (!hidl_status.isOk()) {
          ALOGE("VendorInterface -> Unable to call initializationComplete()");
        }
      },
      [cb](const hidl_vec<uint8_t>& packet) {
        auto hidl_status = cb->hciEventReceived(packet);
        if (!hidl_status.isOk()) {
          ALOGE("VendorInterface -> Unable to call hciEventReceived()");
        }
      },
      [cb](const hidl_vec<uint8_t>& packet) {
        auto hidl_status = cb->aclDataReceived(packet);
        if (!hidl_status.isOk()) {
          ALOGE("VendorInterface -> Unable to call aclDataReceived()");
        }
      },
      [cb](const hidl_vec<uint8_t>& packet) {
        auto hidl_status = cb->scoDataReceived(packet);
        if (!hidl_status.isOk()) {
          ALOGE("VendorInterface -> Unable to call scoDataReceived()");
        }
      });
  if (!rc) {
    auto hidl_status = cb->initializationComplete(Status::INITIALIZATION_ERROR);
    if (!hidl_status.isOk()) {
      ALOGE("VendorInterface -> Unable to call initializationComplete(ERR)");
    }
  }
#endif

  return Void();
}

Return<void> BluetoothHci::close() {
  ALOGI("BluetoothHci::close()");
  unlink_cb_(death_recipient_);
#if defined(MTK_BT_HAL_STATE_MACHINE) && (TRUE == MTK_BT_HAL_STATE_MACHINE)
  StateMachine::GetInstance()->Transit(BtHalStateMessage::kOffMsg);
#else
  VendorInterface::Shutdown();
#endif
  return Void();
}

Return<void> BluetoothHci::sendHciCommand(const hidl_vec<uint8_t>& command) {
  sendDataToController(HCI_DATA_TYPE_COMMAND, command);
  return Void();
}

Return<void> BluetoothHci::sendAclData(const hidl_vec<uint8_t>& data) {
  sendDataToController(HCI_DATA_TYPE_ACL, data);
  return Void();
}

Return<void> BluetoothHci::sendScoData(const hidl_vec<uint8_t>& data) {
  sendDataToController(HCI_DATA_TYPE_SCO, data);
  return Void();
}

#if defined(MTK_BT_HAL_STATE_MACHINE) && (TRUE == MTK_BT_HAL_STATE_MACHINE)
void BluetoothHci::UnlinkDeathReception() {
  unlink_cb_(death_recipient_);
}
#endif

void BluetoothHci::sendDataToController(const uint8_t type,
                                        const hidl_vec<uint8_t>& data) {
  VendorInterface::get()->Send(type, data.data(), data.size());
}

IBluetoothHci* HIDL_FETCH_IBluetoothHci(const char* /* name */) {
  ALOGI("Init IBluetoothHCI");
#if defined(MTK_BT_HAL_STATE_MACHINE) && (TRUE == MTK_BT_HAL_STATE_MACHINE)
  StateMachine::GetInstance()->Start();
#endif
  return new BluetoothHci();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace bluetooth
}  // namespace hardware
}  // namespace android
