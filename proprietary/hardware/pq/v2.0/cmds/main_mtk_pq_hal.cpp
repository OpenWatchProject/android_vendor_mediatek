#define LOG_TAG "vendor.mediatek.hardware.pq@2.2-service"

#define MTK_LOG_ENABLE 1

#include <iostream>
#include <log/log.h>
#include <hidl/LegacySupport.h>
#include <vendor/mediatek/hardware/pq/2.3/IPictureQuality.h>

using vendor::mediatek::hardware::pq::V2_3::IPictureQuality;
using android::hardware::defaultPassthroughServiceImplementation;

int main()
{
    int ret = 0;

    try {
        ret = defaultPassthroughServiceImplementation<IPictureQuality>(4);
    } catch (const std::__1::system_error & e) {

    }

    return ret;
}
