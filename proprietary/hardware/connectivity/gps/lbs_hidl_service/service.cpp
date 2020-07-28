#define LOG_TAG "lbs_hidl_service"

#include <lbs_hidl_service.h>
#include <hidl/LegacySupport.h>

int main() {
    ::vendor::mediatek::hardware::lbs::V1_0::implementation::cpp_main();
    ::android::hardware::joinRpcThreadpool();

    return 0;
}

