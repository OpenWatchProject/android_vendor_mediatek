#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>
#include <mtkcam/utils/TuningUtils/CommonRule.h>

namespace NSCam {
namespace TuningUtils {

void genFileName_YUV(
    char *pFilename,
    int nFilename,
    const FILE_DUMP_NAMING_HINT *pHint,
    YUV_PORT type,
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

    if (type != YUV_PORT_NULL) {
        if (type == YUV_PORT_IMG2O) {
            t = snprintf(ptr, nFilename, "-img2o");
        } else if (type == YUV_PORT_IMG3O) {
            t = snprintf(ptr, nFilename, "-img3o");
        } else if (type == YUV_PORT_WROTO) {
            t = snprintf(ptr, nFilename, "-wroto");
        } else if (type == YUV_PORT_WDMAO) {
            t = snprintf(ptr, nFilename, "-wdmao");
        } else if (type == YUV_PORT_TIMGO) {
            t = snprintf(ptr, nFilename, "-timgo");
        } else if (type == YUV_PORT_DP0) {
            t = snprintf(ptr, nFilename, "-dp0");
        } else if (type == YUV_PORT_DP1) {
            t = snprintf(ptr, nFilename, "-dp1");
        } else if (type == YUV_PORT_CRZO1) {
            t = snprintf(ptr, nFilename, "-crzor1");
        } else if (type == YUV_PORT_VNRO) {
            t = snprintf(ptr, nFilename, "-vnro");
        } else if (type == YUV_PORT_IMGI) {
            t = snprintf(ptr, nFilename, "-imgi");
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
    ;
    t = snprintf(ptr, nFilename, "__%dx%d_%s_s0",
            pHint->ImgWidth,
            pHint->ImgHeight,
            IMAGE_FORMAT_TO_BITS(pHint->ImgFormat));
    ptr += t;
    nFilename -= t;

    t = snprintf(ptr, nFilename, ".%s", IMAGE_FORMAT_TO_FILE_EXT(pHint->ImgFormat));
    ptr += t;
    nFilename -= t;

}

} //namespace FileDump
} //namespace NSCam

