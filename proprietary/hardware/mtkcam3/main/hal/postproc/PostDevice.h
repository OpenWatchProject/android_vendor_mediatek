#ifndef HIDL_GENERATED_VENDOR_MEDIATEK_HARDWARE_CAMERA_POSTPROC_V1_0_ISPDEVICE_H
#define HIDL_GENERATED_VENDOR_MEDIATEK_HARDWARE_CAMERA_POSTPROC_V1_0_ISPDEVICE_H

#include <vendor/mediatek/hardware/camera/postproc/1.0/types.h>
#include <vendor/mediatek/hardware/camera/postproc/1.0/IPostDevice.h>

// #include <hidl/MQDescriptor.h>
// #include <hidl/Status.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace postproc {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

using ::vendor::mediatek::hardware::camera::postproc::V1_0::ChromaNRParam;

class PostDevice : public IPostDevice {
public:
    PostDevice();
    ~PostDevice();
    // Methods from ::vendor::mediatek::hardware::camera::postproc::V1_0::IPostDevice follow.
    Return<int32_t> doChromaNR(const ::vendor::mediatek::hardware::camera::postproc::V1_0::ChromaNRParam& param) override;
};

extern "C" IPostDevice* HIDL_FETCH_IPostDevice(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace isphal
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // HIDL_GENERATED_VENDOR_MEDIATEK_HARDWARE_CAMERA_POSTPROC_V1_0_ISPDEVICE_H