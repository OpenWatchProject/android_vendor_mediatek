#define LOG_TAG "vendor.mediatek.hardware.keymanage@1.0-service"

#include <vendor/mediatek/hardware/keymanage/1.0/IKeymanage.h>

#include <hidl/HidlTransportSupport.h>
#include <hidl/LegacySupport.h>

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::defaultPassthroughServiceImplementation;

using vendor::mediatek::hardware::keymanage::V1_0::IKeymanage;

int main() {
    return defaultPassthroughServiceImplementation<IKeymanage>();
}
