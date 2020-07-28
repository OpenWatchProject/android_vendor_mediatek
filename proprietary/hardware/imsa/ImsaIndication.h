#ifndef VENDOR_MEDIATEK_HARDWARE_IMSA_V1_0_IMSAINDICATION_H
#define VENDOR_MEDIATEK_HARDWARE_IMSA_V1_0_IMSAINDICATION_H

#include <vendor/mediatek/hardware/imsa/1.0/IImsaIndication.h>
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
using ::vendor::mediatek::hardware::imsa::V1_0::IImsaIndication;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct ImsaIndication : public IImsaIndication {
    // Methods from ::vendor::mediatek::hardware::imsa::V1_0::IImsaIndication follow.
    Return<void> readEvent(const hidl_vec<int8_t>& data, int32_t offset, int32_t length) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

extern "C" IImsaIndication* HIDL_FETCH_IImsaIndication(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace imsa
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_IMSA_V1_0_IMSAINDICATION_H
