#ifndef VENDOR_MEDIATEK_HARDWARE_CLIENTAPI_V1_0_CLIENTAPIINDICATION_H
#define VENDOR_MEDIATEK_HARDWARE_CLIENTAPI_V1_0_CLIENTAPIINDICATION_H

#include <vendor/mediatek/hardware/clientapi/1.0/IClientapiIndication.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace clientapi {
namespace V1_0 {
namespace implementation {

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::mediatek::hardware::clientapi::V1_0::IClientapiIndication;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct ClientapiIndication : public IClientapiIndication {
    // Methods from ::vendor::mediatek::hardware::clientapi::V1_0::IClientapiIndication follow.
    Return<void> readEvent(const hidl_vec<int8_t>& data, int32_t offset, int32_t length) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};
extern "C" IClientapiIndication* HIDL_FETCH_IClientapiIndication(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace clientapi
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_CLIENTAPI_V1_0_CLIENTAPIINDICATION_H
