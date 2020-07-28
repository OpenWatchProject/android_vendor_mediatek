#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>
#include <mtkcam/utils/TuningUtils/CommonRule.h>
#include <cutils/properties.h>

namespace NSCam {
namespace TuningUtils {

// P2-like node dump
void genFileName_RAW(
    char *pFilename,
    int nFilename,
    const FILE_DUMP_NAMING_HINT *pHint,
    RAW_PORT type,
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

    if (type != RAW_PORT_NULL) {
        if (type == RAW_PORT_IMGO) {
            t = snprintf(ptr, nFilename, "-imgo");
        } else if (type == RAW_PORT_RRZO) {
            t = snprintf(ptr, nFilename, "-rrzo");
        } else {
            t = snprintf(ptr, nFilename, "-undef");
        }
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

    t = snprintf(ptr, nFilename, "__%dx%d_%s_%s",
        pHint->ImgWidth, pHint->ImgHeight,
        IMAGE_FORMAT_TO_BITS(pHint->ImgFormat),
        SENSOR_FORMAT_TO_STRING(pHint->SensorFormatOrder));
    ptr += t;
    nFilename -= t;

    t = snprintf(ptr, nFilename, ".%s", IMAGE_FORMAT_TO_FILE_EXT(pHint->ImgFormat));
    ptr += t;
    nFilename -= t;
}

} //namespace FileDump
} //namespace NSCam

