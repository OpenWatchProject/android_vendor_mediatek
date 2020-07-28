#include "dfps/string_def.h"

#include <linux/dfrc_drv.h>

namespace android {

const char* getApiString(int32_t api)
{
    char const *apiString[DFRC_DRV_API_MAXIMUM] = {
        "GIFT",
        "VIDEO",
        "RRC_INPUT",
        "RRC_VIDEO",
        "THERMAL",
        "LOADING",
        "WHITELIST",
    };

    if (api >= DFRC_DRV_API_MAXIMUM || api < 0) {
        return NULL;
    }

    return apiString[api];
}

const char* getModeString(int32_t mode)
{
    char const *modeString[DFRC_DRV_MODE_MAXIMUM] = {
        "DEFAULT",
        "FRR",
        "ARR",
        "INTERNAL_SW"
    };

    if (mode >= DFRC_DRV_MODE_MAXIMUM || mode < 0) {
        return NULL;
    }

    return modeString[mode];
}

};
