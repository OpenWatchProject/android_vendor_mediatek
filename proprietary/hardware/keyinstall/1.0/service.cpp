#define LOG_TAG "vendor.mediatek.hardware.keyinstall@1.0-service"

#include <vendor/mediatek/hardware/keyinstall/1.0/IKeyinstall.h>

#include <hidl/HidlTransportSupport.h>
#include <hidl/LegacySupport.h>

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::defaultPassthroughServiceImplementation;

using vendor::mediatek::hardware::keyinstall::V1_0::IKeyinstall;

int main() {
    return defaultPassthroughServiceImplementation<IKeyinstall>();
}
