#include "EventCallback.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace bgservice {
namespace V1_0 {
namespace implementation {

// Methods from ::vendor::mediatek::hardware::camera::bgservice::V1_0::IEventCallback follow.
Return<bool> EventCallback::onCompleted(int32_t ImgReaderId, int32_t frameNumber, uint32_t status) {
    // TODO implement
    return bool {};
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//IEventCallback* HIDL_FETCH_IEventCallback(const char* /* name */) {
    //return new EventCallback();
//}
//
}  // namespace implementation
}  // namespace V1_0
}  // namespace bgservice
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
