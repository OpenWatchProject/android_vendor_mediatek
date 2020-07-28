#define LOG_TAG "AAL-Test"

#define MTK_LOG_ENABLE 1
#include <log/log.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <vendor/mediatek/hardware/pq/2.3/IPictureQuality.h>

using android::sp;
using vendor::mediatek::hardware::pq::V2_3::IPictureQuality;
using vendor::mediatek::hardware::pq::V2_0::Result;

// using namespace android;

static const int PREDEFINED_CURVE1[] = {
    0, 1, 10, 100, 1000, 10000, 20000,
    8, 8, 12,  40, 200,   255,   255
};
static const int PREDEFINED_CURVE2[] = {
    0, 16, 32, 50, 100, 140, 180, 240, 300, 600, 1000, 2000, 3000, 4000, 8000, 10000,
    30, 40, 50, 60, 70, 80, 102, 102, 102, 102, 102, 180, 200, 210, 230, 255
};



static void print_usages()
{
    printf(
        "aal-test [parameter] (value)\n"
        "\n"
        "Parameters:\n"
        "    func    2:ESS + 4:DRE\n"
        "    SBS     0 ~ 255 (Smart backlight strength)\n"
        "    SBR     0 ~ 255 (Smart backlight range)\n"
        "    RD      0 ~ 255 (Readability level)\n"
        "    curve   Show curve\n"
        "    setcurve (1/2)   Set curve 1/2\n"
        "    BS      Brighten ramp rate\n"
        "    DS      Darken ramp rate\n"
        "\n"
        "Examples:"
        "    # aal-test func 6    -> Enable DRE + ESS\n"
        "    # aal-test BR 190    -> Change brightness level to 190\n"
        );
}


int main(int argc, char *argv[])
{
    const char *param = "";
    int32_t value = 0;
    int32_t ret = 0;

    if (argc == 1) {
        print_usages();
        return 0;
    } else if (argc == 2) {
    } else if (argc == 3) {
        value = (int32_t)strtol(argv[2], NULL, 0);
        printf("param = %s, value = %d\n", argv[1], value);
    } else {
        printf("Invalid arguments\n");
    }

    ALOGI("AAL client start");
    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("failed to get HW service");
        return -1;
    }

    param = argv[1];
    if (strcmp(param, "func") == 0) {
        android::hardware::Return<Result> ret = service->setFunction(value, false);
        if (!ret.isOk() || ret != Result::OK) {
            ALOGE("IPictureQuality::setFunction failed!");
        }
    } else if (strcmp(param, "SBS") == 0) {
        android::hardware::Return<Result> ret = service->setSmartBacklightStrength(value);
        if (!ret.isOk() || ret != Result::OK) {
            ALOGE("IPictureQuality::setSmartBacklightStrength failed!");
        }
    } else if (strcmp(param, "SBR") == 0) {
        android::hardware::Return<Result> ret = service->setSmartBacklightRange(value);
        if (!ret.isOk() || ret != Result::OK) {
            ALOGE("IPictureQuality::setSmartBacklightRange failed!");
        }
    } else if (strcmp(param, "RD") == 0) {
        android::hardware::Return<Result> ret = service->setReadabilityLevel(value);
        if (!ret.isOk() || ret != Result::OK) {
            ALOGE("IPictureQuality::setReadabilityLevel failed!");
        }
    } else {
        printf("Invalid test command.\n");
    }

    printf("ret = %d\n", ret);

    ALOGI("AAL client exit");

    return 0;
}
