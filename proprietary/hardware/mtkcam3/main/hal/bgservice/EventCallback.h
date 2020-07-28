#ifndef VENDOR_MEDIATEK_HARDWARE_CAMERA_BGSERVICE_V1_0_EVENTCALLBACK_H
#define VENDOR_MEDIATEK_HARDWARE_CAMERA_BGSERVICE_V1_0_EVENTCALLBACK_H

#include <vendor/mediatek/hardware/camera/bgservice/1.0/IEventCallback.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace bgservice {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct EventCallback : public IEventCallback {
    // Methods from ::vendor::mediatek::hardware::camera::bgservice::V1_0::IEventCallback follow.
    Return<bool> onCompleted(int32_t ImgReaderId, int32_t frameNumber, uint32_t status) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" IEventCallback* HIDL_FETCH_IEventCallback(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace bgservice
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_CAMERA_BGSERVICE_V1_0_EVENTCALLBACK_H
