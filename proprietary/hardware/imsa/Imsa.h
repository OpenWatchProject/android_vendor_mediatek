#ifndef VENDOR_MEDIATEK_HARDWARE_IMSA_V1_0_IMSA_H
#define VENDOR_MEDIATEK_HARDWARE_IMSA_V1_0_IMSA_H

#include <vendor/mediatek/hardware/imsa/1.0/IImsa.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace imsa {
namespace V1_0 {
namespace implementation {

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::mediatek::hardware::imsa::V1_0::IImsa;
using ::vendor::mediatek::hardware::imsa::V1_0::IImsaIndication;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct Imsa : public IImsa {
    // Methods from ::vendor::mediatek::hardware::imsa::V1_0::IImsa follow.
    Return<void> setResponseFunctions(const sp<IImsaIndication>& imsaIndication) override;
    Return<void> writeEvent(int32_t request_id, int32_t length, const hidl_vec<int8_t>& value) override;
    Return<void> writeBytes(const hidl_vec<int8_t>& value) override;
    Return<void> writeInt(int32_t value) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

extern "C" IImsa* HIDL_FETCH_IImsa(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace imsa
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_IMSA_V1_0_IMSA_H
