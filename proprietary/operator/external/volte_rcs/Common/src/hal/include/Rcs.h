#ifndef VENDOR_MEDIATEK_HARDWARE_RCS_V1_0_RCS_H
#define VENDOR_MEDIATEK_HARDWARE_RCS_V1_0_RCS_H

#include <vendor/mediatek/hardware/rcs/1.0/IRcs.h>
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
using ::vendor::mediatek::hardware::rcs::V1_0::IRcs;
using ::vendor::mediatek::hardware::rcs::V1_0::IRcsIndication;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct Rcs : public IRcs {
    // Methods from ::vendor::mediatek::hardware::rcs::V1_0::IRcs follow.
    Return<void> setResponseFunctions(const sp<IRcsIndication>& rcsIndication) override;
    Return<void> writeEvent(int32_t request_id, int32_t length, const hidl_vec<int8_t>& value) override;
    Return<void> writeBytes(const hidl_vec<int8_t>& value) override;
    Return<void> writeInt(int32_t value) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

extern "C" IRcs* HIDL_FETCH_IRcs(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace rcs
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_RCS_V1_0_RCS_H
