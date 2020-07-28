#ifndef VENDOR_MEDIATEK_HARDWARE_OMADM_V1_0_OMADM_H
#define VENDOR_MEDIATEK_HARDWARE_OMADM_V1_0_OMADM_H

#include <vendor/mediatek/hardware/omadm/1.0/IOmadm.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace omadm {
namespace V1_0 {
namespace implementation {

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::mediatek::hardware::omadm::V1_0::IOmadm;
using ::vendor::mediatek::hardware::omadm::V1_0::IOmadmIndication;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct omadm : public IOmadm {
    // Methods from ::vendor::mediatek::hardware::imsa::V1_0::IImsa follow.
    Return<void> setResponseFunctions(const sp<IOmadmIndication>& iomadmIndication) override;
    Return<void> writeEvent(int32_t request_id, int32_t length, const hidl_vec<int8_t>& value) override;
    Return<void> writeBytes(const hidl_vec<int8_t>& value) override;
    Return<void> writeInt(int32_t value) override;
    Return<void> WapPushSysUpdate(int32_t transId, const hidl_string& pdu) override;
    Return<void> SmsCancelSysUpdate(const hidl_string& sms_user_data) override;
    Return<void> omadmControllerDispachAdminNetStatus(int32_t status, int32_t netId) override;
    Return<void> omadmNetManagerReply(int32_t net_feature, int32_t enabled) override;
    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

extern "C" IOmadm* HIDL_FETCH_IOmadm(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace imsa
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor


#endif  // VENDOR_MEDIATEK_HARDWARE_IMSA_V1_0_IMSA_H
