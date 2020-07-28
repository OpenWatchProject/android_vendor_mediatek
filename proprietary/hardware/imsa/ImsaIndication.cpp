#include "ImsaIndication.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace imsa {
namespace V1_0 {
namespace implementation {

// Methods from ::vendor::mediatek::hardware::imsa::V1_0::IImsaIndication follow.
Return<void> ImsaIndication::readEvent(const hidl_vec<int8_t>& data, int32_t offset, int32_t length) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

IImsaIndication* HIDL_FETCH_IImsaIndication(const char* /* name */) {
    return new ImsaIndication();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace imsa
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
