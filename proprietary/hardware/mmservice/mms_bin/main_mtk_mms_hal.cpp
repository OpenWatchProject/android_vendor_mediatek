#define LOG_TAG "vendor.mediatek.hardware.mms@1.5-service"

#define MTK_LOG_ENABLE 1

#include <iostream>
#include <cutils/log.h>
#include <vendor/mediatek/hardware/mms/1.5/IMms.h>
#include <hidl/LegacySupport.h>

using vendor::mediatek::hardware::mms::V1_5::IMms;
using android::hardware::defaultPassthroughServiceImplementation;

int main()
{
    int ret = 0;
    try {
        ret = defaultPassthroughServiceImplementation<IMms>(4);
    } catch (const std::__1::system_error & e) {

    }
    return ret;
}
