#ifndef VENDOR_MEDIATEK_HARDWARE_KEYINSTALL_V1_0_KEYINSTALL_H
#define VENDOR_MEDIATEK_HARDWARE_KEYINSTALL_V1_0_KEYINSTALL_H

#include <vendor/mediatek/hardware/keyinstall/1.0/IKeyinstall.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <cutils/log.h>
#include "meta_drmkey_install_para.h"
#include "Keyinstall_util.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace keyinstall {
namespace V1_0 {
namespace implementation {

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::mediatek::hardware::keyinstall::V1_0::HIDL_FT_DRMKEY_INSTALL_CNF;
using ::vendor::mediatek::hardware::keyinstall::V1_0::HIDL_FT_DRMKEY_INSTALL_REQ;
using ::vendor::mediatek::hardware::keyinstall::V1_0::IKeyinstall;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct Keyinstall : public IKeyinstall {
    // Methods from ::vendor::mediatek::hardware::keyinstall::V1_0::IKeyinstall follow.
    Return<void> meta_drmkey_install_op(const HIDL_FT_DRMKEY_INSTALL_REQ& fromClient_req, const HIDL_FT_DRMKEY_INSTALL_CNF& fromClient_cnf, const hidl_vec<uint8_t>& data, uint16_t len, meta_drmkey_install_op_cb _hidl_cb) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

extern "C" IKeyinstall* HIDL_FETCH_IKeyinstall(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace keyinstall
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_KEYINSTALL_V1_0_KEYINSTALL_H
