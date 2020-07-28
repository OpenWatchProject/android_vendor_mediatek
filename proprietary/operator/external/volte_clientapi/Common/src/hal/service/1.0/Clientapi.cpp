#include "Clientapi.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace clientapi {
namespace V1_0 {
namespace implementation {

// Methods from ::vendor::mediatek::hardware::clientapi::V1_0::IClientapi follow.
Return<void> Clientapi::setResponseFunctions(const sp<IClientapiIndication>& clientapiIndication) {
    // TODO implement
    return Void();
}

Return<void> Clientapi::writeEvent(int32_t request_id, int32_t length, const hidl_vec<int8_t>& value) {
    // TODO implement
    return Void();
}

Return<void> Clientapi::writeBytes(const hidl_vec<int8_t>& value) {
    // TODO implement
    return Void();
}

Return<void> Clientapi::writeInt(int32_t value) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

IClientapi* HIDL_FETCH_IClientapi(const char* /* name */) {
    return new Clientapi();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace clientapi
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
