#ifndef VENDOR_MEDIATEK_HARDWARE_CAMERA_CCAP_V1_0_CCAPCONTROL_H
#define VENDOR_MEDIATEK_HARDWARE_CAMERA_CCAP_V1_0_CCAPCONTROL_H

#include <vendor/mediatek/hardware/camera/ccap/1.0/ICCAPControl.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace ccap {
namespace V1_0 {
namespace implementation {

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::mediatek::hardware::camera::ccap::V1_0::ICCAPControl;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct CCAPControl : public ICCAPControl {
    // Methods from ::vendor::mediatek::hardware::camera::ccap::V1_0::ICCAPControl follow.
    Return<int32_t> intf_ccap_atci_op(const hidl_memory& req, const hidl_memory& cnf, int32_t cct_op_legacy) override;
    Return<int32_t> intf_ccap_init() override;
    Return<int32_t> intf_ccap_adb_op() override;
    Return<int32_t> intf_ccap_const(const hidl_memory& constVal) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

extern "C" ICCAPControl* HIDL_FETCH_ICCAPControl(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace ccap
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_CAMERA_CCAP_V1_0_CCAPCONTROL_H
