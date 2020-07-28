#include "Presence.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace presence {
namespace V1_0 {
namespace implementation {

// Methods from ::vendor::mediatek::hardware::presence::V1_0::IPresence follow.
Return<void> Presence::setResponseFunctions(const sp<IPresenceIndication>& presenceIndication) {
    // TODO implement
    return Void();
}

Return<void> Presence::writeEvent(int32_t request_id, int32_t length, const hidl_vec<int8_t>& value) {
    // TODO implement
    return Void();
}

Return<void> Presence::writeBytes(const hidl_vec<int8_t>& value) {
    // TODO implement
    return Void();
}

Return<void> Presence::writeInt(int32_t value) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

IPresence* HIDL_FETCH_IPresence(const char* /* name */) {
    return new Presence();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace presence
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
