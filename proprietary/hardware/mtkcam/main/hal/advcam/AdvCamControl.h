#ifndef VENDOR_MEDIATEK_HARDWARE_CAMERA_ADVCAM_V1_0_ADVCAMCONTROL_H
#define VENDOR_MEDIATEK_HARDWARE_CAMERA_ADVCAM_V1_0_ADVCAMCONTROL_H

#include <vendor/mediatek/hardware/camera/advcam/1.0/IAdvCamControl.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace advcam {
namespace V1_0 {
namespace implementation {

using ::android::hardware::camera::common::V1_0::Status;
using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::mediatek::hardware::camera::advcam::V1_0::IAdvCamControl;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct AdvCamControl : public IAdvCamControl {
    // Methods from ::vendor::mediatek::hardware::camera::advcam::V1_0::IAdvCamControl follow.
    Return<Status> setConfigureParam(uint32_t id, const hidl_vec<uint8_t>& configParam) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.
};

extern "C" IAdvCamControl* HIDL_FETCH_IAdvCamControl(const char* name);

/*
* Wrapper for calling AdvCamSettingMgr
*/
class AdvCamControlWrap {

public:
    static AdvCamControlWrap* getInstance();

    void setConfigureParam(uint32_t id, const camera_metadata_t* metadata);
private:
    AdvCamControlWrap() {};
    ~AdvCamControlWrap() {};
};// class AdvCamControlWrap

}  // namespace implementation
}  // namespace V1_0
}  // namespace advcam
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor


#endif  // VENDOR_MEDIATEK_HARDWARE_CAMERA_ADVCAM_V1_0_ADVCAMCONTROL_H
