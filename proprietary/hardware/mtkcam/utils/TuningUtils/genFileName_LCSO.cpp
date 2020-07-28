#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>
#include <mtkcam/utils/TuningUtils/CommonRule.h>

namespace NSCam {
namespace TuningUtils {

void genFileName_LCSO(
    char *pFilename,
    int nFilename,
    const FILE_DUMP_NAMING_HINT *pHint,
    const char *pUserString)
{
    if (pUserString == NULL) pUserString = "";
    int t;
    char *ptr = pFilename;
    if (property_get_int32("vendor.debug.enable.normalAEB", 0))
        t = MakePrefix(ptr, nFilename, pHint->UniqueKey, pHint->FrameNo, pHint->RequestNo, pHint->EvValue);
    else
    t = MakePrefix(ptr, nFilename, pHint->UniqueKey, pHint->FrameNo, pHint->RequestNo);
    ptr += t;
    nFilename -= t;

    if (pHint->SensorDev >= 0) {
        t = snprintf(ptr, nFilename, "-%s", SENSOR_DEV_TO_STRING(pHint->SensorDev));
        ptr += t;
        nFilename -= t;
    }

    if (*(pHint->additStr) != '\0') {
        t = snprintf(ptr, nFilename, "-%s", pHint->additStr);
        ptr += t;
        nFilename -= t;
    }

    if (*pUserString != '\0') {
        t = snprintf(ptr, nFilename, "-%s", pUserString);
        ptr += t;
        nFilename -= t;
    }

    t = snprintf(ptr, nFilename, "-PW%d-PH%d-BW%d", pHint->BufWidth, pHint->BufHeight, pHint->BufStride);
    ptr += t;
    nFilename -= t;

    // default LCSO valid bits is 12 for NDD requirement
    int lcso_valid_bit = 12;
    t = snprintf(ptr, nFilename, "__%dx%d_%d_s0",
        pHint->ImgWidth, pHint->ImgHeight,
        lcso_valid_bit);
    ptr += t;
    nFilename -= t;

    t = snprintf(ptr, nFilename, ".lcso");
    ptr += t;
    nFilename -= t;
}

} //namespace FileDump
} //namespace NSCam

