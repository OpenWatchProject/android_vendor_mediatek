#define LOG_TAG "flash_custom_3.cpp"
#define MTK_LOG_ENABLE 1

#include <cutils/log.h>
#include "mtkcam/def/BuiltinTypes.h"
#include "camera_custom_awb_nvram.h"
#include "flash_tuning_custom.h"
#include "flash_feature.h"

int cust_getPrecapAfMode()
{
    ALOGD("%s().", __FUNCTION__);

    /*
     * <flash_custom_3.cpp>
     * enum {
     *     e_PrecapAf_None,
     *     e_PrecapAf_BeforePreflash,
     *     e_PrecapAf_AfterPreflash,
     * };
     */
    return e_PrecapAf_None;
}

int cust_isNeedDoPrecapAF_v2(
        int isLastFocusModeTAF, int isFocused, int flashMode,
        int afLampMode, int isBvLowerTriger)
{
    (void)isLastFocusModeTAF;
    (void)isFocused;
    (void)afLampMode;

    if (isBvLowerTriger == 1 && flashMode != LIB3A_FLASH_MODE_FORCE_OFF)
        return 1;
    else
        return 0;
}

