#ifndef VENDOR_MEDIATEK_HARDWARE_PRESENCE_V1_0_PRESENCEINDICATION_H
#define VENDOR_MEDIATEK_HARDWARE_PRESENCE_V1_0_PRESENCEINDICATION_H

#include <vendor/mediatek/hardware/presence/1.0/IPresenceIndication.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace presence {
namespace V1_0 {
namespace implementation {

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::mediatek::hardware::presence::V1_0::IPresenceIndication;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct PresenceIndication : public IPresenceIndication {
    // Methods from ::vendor::mediatek::hardware::presence::V1_0::IPresenceIndication follow.
    Return<void> readEvent(const hidl_vec<int8_t>& data, int32_t offset, int32_t length) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

extern "C" IPresenceIndication* HIDL_FETCH_IPresenceIndication(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace presence
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_PRESENCE_V1_0_PRESENCEINDICATION_H
