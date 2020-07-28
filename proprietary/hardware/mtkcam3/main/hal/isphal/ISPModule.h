#ifndef VENDOR_MEDIATEK_HARDWARE_CAMERA_IISPHAL_V1_0_ISPMODULE_H
#define VENDOR_MEDIATEK_HARDWARE_CAMERA_IISPHAL_V1_0_ISPMODULE_H

#include <vendor/mediatek/hardware/camera/isphal/1.0/IISPModule.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/Mutex.h> // android::Mutex
#include <utils/KeyedVector.h>
#include "IISPManager.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace isphal {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
//using ::android::hardware::camera::common::V1_0;

using ::vendor::mediatek::hardware::camera::isphal::V1_0::IISPModule;
using ::vendor::mediatek::hardware::camera::isphal::V1_0::Status;
using namespace NSCam;
using namespace NSCam::ISPHal;

class ISPModule
    : public IISPModule
    , public android::hardware::hidl_death_recipient
{
    // Methods from ::vendor::mediatek::hardware::camera::isphal::V1_0::IISPModule follow.
    Return<void> openISPDevice(openISPDevice_cb _hidl_cb) override;
public:
    // Methods from ::android::hardware::hidl_death_recipient follow.
    virtual void serviceDied(
            uint64_t cookie, const ::android::wp<::android::hidl::base::V1_0::IBase>& who) override;

private:
    mutable ::android::Mutex            mLock;
};

extern "C" IISPModule* HIDL_FETCH_IISPModule(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace bgservice
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

namespace NSCam {
namespace ISPHal {

class ISPManager
    : public IISPManager
{
    virtual auto removeISPDevice(int32_t id) -> void;
    virtual auto createISPDevice() -> android::sp<IDevice>;
private:
    mutable ::android::Mutex            mLock;
    android::KeyedVector<uint32_t, android::sp<IDevice>> mDeviceMap;
    uint32_t mDeviceCount = 0;
};

}
}

#endif  // VENDOR_MEDIATEK_HARDWARE_CAMERA_IISPHAL_V1_0_ISPMODULE_H
