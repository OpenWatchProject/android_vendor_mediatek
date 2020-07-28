#include "Imsa.h"

#define UNUSED(x) (x)   //eliminate "warning: unused parameter"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace imsa {
namespace V1_0 {
namespace implementation {

// Methods from ::vendor::mediatek::hardware::imsa::V1_0::IImsa follow.
Return<void> Imsa::setResponseFunctions(const sp<IImsaIndication>& imsaIndication) {
    // TODO implement
    UNUSED(imsaIndication);
    return Void();
}

Return<void> Imsa::writeEvent(int32_t request_id, int32_t length, const hidl_vec<int8_t>& value) {
    // TODO implement
    UNUSED(request_id);
    UNUSED(length);
    UNUSED(value);
    return Void();
}

Return<void> Imsa::writeBytes(const hidl_vec<int8_t>& value) {
    // TODO implement
    UNUSED(value);
    return Void();
}

Return<void> Imsa::writeInt(int32_t value) {
    // TODO implement
    UNUSED(value);
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

IImsa* HIDL_FETCH_IImsa(const char* /* name */) {
    return new Imsa();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace imsa
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
