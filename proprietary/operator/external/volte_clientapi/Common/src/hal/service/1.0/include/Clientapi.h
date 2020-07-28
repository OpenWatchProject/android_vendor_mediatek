#ifndef VENDOR_MEDIATEK_HARDWARE_CLIENTAPI_V1_0_CLIENTAPI_H
#define VENDOR_MEDIATEK_HARDWARE_CLIENTAPI_V1_0_CLIENTAPI_H

#include <vendor/mediatek/hardware/clientapi/1.0/IClientapi.h>
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
using ::vendor::mediatek::hardware::clientapi::V1_0::IClientapi;
using ::vendor::mediatek::hardware::clientapi::V1_0::IClientapiIndication;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct Clientapi : public IClientapi {
    // Methods from ::vendor::mediatek::hardware::clientapi::V1_0::IClientapi follow.
    Return<void> setResponseFunctions(const sp<IClientapiIndication>& clientapiIndication) override;
    Return<void> writeEvent(int32_t request_id, int32_t length, const hidl_vec<int8_t>& value) override;
    Return<void> writeBytes(const hidl_vec<int8_t>& value) override;
    Return<void> writeInt(int32_t value) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

extern "C" IClientapi* HIDL_FETCH_IClientapi(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace clientapi
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_CLIENTAPI_V1_0_CLIENTAPI_H
