#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>
#include <mtkcam/utils/TuningUtils/CommonRule.h>
#include <cutils/properties.h>
//#include <mtkcam/utils/std/Log.h>

namespace NSCam {
namespace TuningUtils {

// Jpeg dump
void genFileName_JPG(
    char *pFilename,
    int nFilename,
    const FILE_DUMP_NAMING_HINT *pHint,
    const char *pUserString)
{
    if (pUserString == NULL) pUserString = "";
    int t;
    char *ptr = pFilename;
    if (property_get_int32("vendor.debug.enable.normalAEB", 0))
    {
        //CAM_LOGE("vendor.debug.enable.normalAEB : EV(%d)", pHint->EvValue);
        t = MakePrefix(ptr, nFilename, pHint->UniqueKey, pHint->FrameNo, pHint->RequestNo, pHint->EvValue);
    }
    else
        t = MakePrefix(ptr, nFilename, pHint->UniqueKey, pHint->FrameNo, pHint->RequestNo);
    ptr += t;
    nFilename -= t;

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

    t = snprintf(ptr, nFilename, ".jpg");
    ptr += t;
    nFilename -= t;
}

} //namespace FileDump
} //namespace NSCam

