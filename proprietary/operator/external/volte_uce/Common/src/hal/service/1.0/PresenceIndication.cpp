#include "PresenceIndication.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace presence {
namespace V1_0 {
namespace implementation {

// Methods from ::vendor::mediatek::hardware::presence::V1_0::IPresenceIndication follow.
Return<void> PresenceIndication::readEvent(const hidl_vec<int8_t>& data, int32_t offset, int32_t length) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

IPresenceIndication* HIDL_FETCH_IPresenceIndication(const char* /* name */) {
    return new PresenceIndication();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace presence
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
