#define LOG_TAG "vendor.mediatek.hardware.dfps@1.0-service"

#include <sched.h>

#include <vendor/mediatek/hardware/dfps/1.0/IFpsPolicyService.h>

#include <hidl/LegacySupport.h>

using vendor::mediatek::hardware::dfps::V1_0::IFpsPolicyService;
using android::hardware::defaultPassthroughServiceImplementation;

int main()
{
    return defaultPassthroughServiceImplementation<IFpsPolicyService>(2);
}
