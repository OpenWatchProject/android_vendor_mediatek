#ifndef VENDOR_MEDIATEK_HARDWARE_CAMERA_ISPHAL_V1_0_ISPMANAGER_H
#define VENDOR_MEDIATEK_HARDWARE_CAMERA_ISPHAL_V1_0_ISPMANAGER_H

#include <vendor/mediatek/hardware/camera/isphal/1.0/IDevice.h>
#include <utils/RefBase.h>

namespace NSCam {
namespace ISPHal {

using ::vendor::mediatek::hardware::camera::isphal::V1_0::IDevice;

class IISPManager
{
public:
    virtual                     ~IISPManager() {}
    virtual auto removeISPDevice(int32_t id) -> void = 0;
    virtual auto createISPDevice() -> android::sp<IDevice> = 0;
    static IISPManager*        getInstance();
};

}  // namespace ISPHal
}  // namespace NSCam

#endif  // VENDOR_MEDIATEK_HARDWARE_CAMERA_ISPHAL_V1_0_ISPMANAGER_H
