#define LOG_TAG "vendor.mediatek.hardware.mtkcodecservice@1.0-service"

#include <vendor/mediatek/hardware/mtkcodecservice/1.1/IMtkCodecService.h>
#include <hidl/LegacySupport.h>

using vendor::mediatek::hardware::mtkcodecservice::V1_1::IMtkCodecService;
using android::hardware::defaultLazyPassthroughServiceImplementation;

int main() {
    return defaultLazyPassthroughServiceImplementation<IMtkCodecService>();
}