#define LOG_TAG "vendor.mediatek.hardware.gpu@1.0-service"

#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>

#include "GraphicExt.h"

#include <iostream>
#include <log/log.h>
#include <hidl/LegacySupport.h>
#include <vendor/mediatek/hardware/gpu/1.0/IGraphicExt.h>

using ::android::hardware::configureRpcThreadpool;
using ::vendor::mediatek::hardware::gpu::V1_0::implementation::GraphicExtService;
using ::android::hardware::joinRpcThreadpool;
using ::android::OK;
using ::android::sp;

int main(int /* argc */, char* /* argv */ []) {
    sp<GraphicExtService> graphic_ext_service = new GraphicExtService();
    configureRpcThreadpool(1, true /* will join */);
    if (graphic_ext_service->registerAsService() != OK) {
        ALOGE("register gpu hidl service fail");
        return 1;
    }
    joinRpcThreadpool();

    ALOGD("gpu service exited!");
    return 1;
}
