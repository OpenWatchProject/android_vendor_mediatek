#ifndef VENDOR_MEDIATEK_HARDWARE_CAMERA_ATMS_V1_0_ATMS_H
#define VENDOR_MEDIATEK_HARDWARE_CAMERA_ATMS_V1_0_ATMS_H

#include <vendor/mediatek/hardware/camera/atms/1.0/IATMs.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/Mutex.h> // android::Mutex

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace atms {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct ATMs : public IATMs {
    // Methods from ::vendor::mediatek::hardware::camera::atms::V1_0::IATMs follow.
    Return<int32_t> set_atms_factor(const hidl_string& factor_type, const hidl_string& factor_value) override;
    Return<void> get_atms_factor(const hidl_string& factor_type, get_atms_factor_cb _hidl_cb) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
 extern "C" IATMs* HIDL_FETCH_IATMs(const char* name);
/*
* Wrapper for calling ATMsEventMgr
*/
class ATMsWrap{

public:
    static ATMsWrap* getInstance();
    Return<int32_t> setFactor(std::string factor_type, std::string factor_Value);

     ATMsWrap() {};
    ~ATMsWrap() {};

private:
    mutable android::Mutex                   mLock;
};// class ATMsWrap

}  // namespace implementation
}  // namespace V1_0
}  // namespace atms
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_CAMERA_ATMS_V1_0_ATMS_H
