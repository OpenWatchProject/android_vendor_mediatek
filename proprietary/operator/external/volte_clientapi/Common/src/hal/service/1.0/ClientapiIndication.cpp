#include "ClientapiIndication.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace clientapi {
namespace V1_0 {
namespace implementation {

// Methods from ::vendor::mediatek::hardware::clientapi::V1_0::IClientapiIndication follow.
Return<void> ClientapiIndication::readEvent(const hidl_vec<int8_t>& data, int32_t offset, int32_t length) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

IClientapiIndication* HIDL_FETCH_IClientapiIndication(const char* /* name */) {
    return new ClientapiIndication();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace clientapi
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
