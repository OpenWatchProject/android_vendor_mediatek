#ifndef VENDOR_MEDIATEK_HARDWARE_KEYMANAGE_V1_0_KEYMANAGE_H
#define VENDOR_MEDIATEK_HARDWARE_KEYMANAGE_V1_0_KEYMANAGE_H

#include <vendor/mediatek/hardware/keymanage/1.0/IKeymanage.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <cutils/log.h>
#include "meta_drmkey_install_para.h"
#include "keyblock.h"
#include "Keymanage_util.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace keymanage {
namespace V1_0 {
namespace implementation {

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::mediatek::hardware::keymanage::V1_0::HIDL_FT_DRMKEY_INSTALL_REQ;
using ::vendor::mediatek::hardware::keymanage::V1_0::IKeymanage;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct Keymanage : public IKeymanage {
    // Methods from IKeymanage follow.
    Return<void> meta_drmkey_install_op(const HIDL_FT_DRMKEY_INSTALL_REQ& fromClient_req, const HIDL_FT_DRMKEY_INSTALL_CNF& fromClient_cnf, const hidl_vec<uint8_t>& data, uint16_t len, meta_drmkey_install_op_cb _hidl_cb) override;
    Return<void> get_encrypt_drmkey_hidl(uint32_t keyID, get_encrypt_drmkey_hidl_cb _hidl_cb) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
extern "C" IKeymanage* HIDL_FETCH_IKeymanage(const char* name);

extern "C" int get_encrypt_drmkey_hidl_c(uint32_t keyID, unsigned int *inlength, unsigned char **enckbdrm);

}  // namespace implementation
}  // namespace V1_0
}  // namespace keymanage
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_KEYMANAGE_V1_0_KEYMANAGE_H
