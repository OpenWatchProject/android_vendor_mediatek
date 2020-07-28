#include "hwc2_defs.h"

#include "utils/tools.h"

const char* HWC2_PRESENT_VALI_STATE_PRESENT_DONE_STR = "PD";
const char* HWC2_PRESENT_VALI_STATE_CHECK_SKIP_VALI_STR = "CSV";
const char* HWC2_PRESENT_VALI_STATE_VALIDATE_STR = "V";
const char* HWC2_PRESENT_VALI_STATE_VALIDATE_DONE_STR = "VD";
const char* HWC2_PRESENT_VALI_STATE_PRESENT_STR = "P";
const char* HWC2_PRESENT_VALI_STATE_UNKNOWN_STR = "UNK";

const char* getPresentValiStateString(const HWC_VALI_PRESENT_STATE& state)
{
    switch(state)
    {
        case HWC_VALI_PRESENT_STATE_PRESENT_DONE:
            return HWC2_PRESENT_VALI_STATE_PRESENT_DONE_STR;

        case HWC_VALI_PRESENT_STATE_CHECK_SKIP_VALI:
            return HWC2_PRESENT_VALI_STATE_CHECK_SKIP_VALI_STR;

        case HWC_VALI_PRESENT_STATE_VALIDATE:
            return HWC2_PRESENT_VALI_STATE_VALIDATE_STR;

        case HWC_VALI_PRESENT_STATE_VALIDATE_DONE:
            return HWC2_PRESENT_VALI_STATE_VALIDATE_DONE_STR;

        case HWC_VALI_PRESENT_STATE_PRESENT:
            return HWC2_PRESENT_VALI_STATE_PRESENT_STR;

        default:
            HWC_LOGE("%s unknown state:%d", __func__, state);
            return HWC2_PRESENT_VALI_STATE_UNKNOWN_STR;
    }
}

const char* HWC_COMPOSITION_FILE_UNKNOWN_STR = "UNK";
const char* HWC_COMPOSITION_FILE_NOT_SET_STR = "NSET";
const char* HWC_COMPOSITION_FILE_HWC_STR = "HWC";
const char* HWC_COMPOSITION_FILE_HWC_DISPLAY_STR = "HWCD";
const char* HWC_COMPOSITION_FILE_HWC_LAYER_STR = "HWCL";
const char* HWC_COMPOSITION_FILE_PLATFORM_STR = "PF";
const char* HWC_COMPOSITION_FILE_PLATFORM_COMMON_STR = "PFC";

const char* getCompFileString(const HWC_COMP_FILE& comp_file)
{
    switch (comp_file)
    {
        case HWC_COMP_FILE_NSET:
            return HWC_COMPOSITION_FILE_NOT_SET_STR;

        case HWC_COMP_FILE_HWC:
            return HWC_COMPOSITION_FILE_HWC_STR;

        case HWC_COMP_FILE_HWCD:
            return HWC_COMPOSITION_FILE_HWC_DISPLAY_STR;

        case HWC_COMP_FILE_HWCL:
            return HWC_COMPOSITION_FILE_HWC_LAYER_STR;

        case HWC_COMP_FILE_PF:
            return HWC_COMPOSITION_FILE_PLATFORM_STR;

        case HWC_COMP_FILE_PFC:
            return HWC_COMPOSITION_FILE_PLATFORM_COMMON_STR;

        default:
            HWC_LOGW("%s: unknown composition file: %d", __func__, comp_file);
            return HWC_COMPOSITION_FILE_UNKNOWN_STR;
    }
}
