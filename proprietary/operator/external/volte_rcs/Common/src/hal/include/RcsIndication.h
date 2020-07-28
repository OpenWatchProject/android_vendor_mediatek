#ifndef VENDOR_MEDIATEK_HARDWARE_RCS_V1_0_RCSINDICATION_H
#define VENDOR_MEDIATEK_HARDWARE_RCS_V1_0_RCSINDICATION_H

#include <vendor/mediatek/hardware/rcs/1.0/IRcsIndication.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace rcs {
namespace V1_0 {
namespace implementation {

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::mediatek::hardware::rcs::V1_0::IRcsIndication;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct RcsIndication : public IRcsIndication {
    // Methods from ::vendor::mediatek::hardware::rcs::V1_0::IRcsIndication follow.
    Return<void> readEvent(const hidl_vec<int8_t>& data, int32_t offset, int32_t length) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

extern "C" IRcsIndication* HIDL_FETCH_IRcsIndication(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace rcs
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_RCS_V1_0_RCSINDICATION_H
