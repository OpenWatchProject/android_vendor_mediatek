#ifndef VENDOR_MEDIATEK_HARDWARE_PRESENCE_V1_0_PRESENCE_H
#define VENDOR_MEDIATEK_HARDWARE_PRESENCE_V1_0_PRESENCE_H

#include <vendor/mediatek/hardware/presence/1.0/IPresence.h>
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
using ::vendor::mediatek::hardware::presence::V1_0::IPresence;
using ::vendor::mediatek::hardware::presence::V1_0::IPresenceIndication;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct Presence : public IPresence {
    // Methods from ::vendor::mediatek::hardware::presence::V1_0::IPresence follow.
    Return<void> setResponseFunctions(const sp<IPresenceIndication>& presenceIndication) override;
    Return<void> writeEvent(int32_t request_id, int32_t length, const hidl_vec<int8_t>& value) override;
    Return<void> writeBytes(const hidl_vec<int8_t>& value) override;
    Return<void> writeInt(int32_t value) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

extern "C" IPresence* HIDL_FETCH_IPresence(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace presence
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_PRESENCE_V1_0_PRESENCE_H
