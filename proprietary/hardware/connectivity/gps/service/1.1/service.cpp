#define LOG_TAG "vendor.mediatek.hardware.gnss@1.1-service"

#include <android/hardware/gnss/1.1/IGnss.h>

#include <hidl/LegacySupport.h>

#include <binder/ProcessState.h>

using android::hardware::gnss::V1_1::IGnss;
using android::hardware::defaultPassthroughServiceImplementation;

int main() {
    // The GNSS HAL may communicate to other vendor components via
    // /dev/vndbinder
    android::ProcessState::initWithDriver("/dev/vndbinder");
    return defaultPassthroughServiceImplementation<IGnss>();
}
