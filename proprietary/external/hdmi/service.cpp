#define LOG_TAG "vendor.mediatek.hardware.hdmi@1.0-service"

#include <vendor/mediatek/hardware/hdmi/1.0/IMtkHdmiService.h>

#include <hidl/HidlTransportSupport.h>
#include <hidl/LegacySupport.h>

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::defaultPassthroughServiceImplementation;

using vendor::mediatek::hardware::hdmi::V1_0::IMtkHdmiService;

int main() {
    return defaultPassthroughServiceImplementation<IMtkHdmiService>();
}
