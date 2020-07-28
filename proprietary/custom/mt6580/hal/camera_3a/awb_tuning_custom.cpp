


#include "camera_custom_types.h"
#include "camera_custom_nvram.h"
#include "awb_feature.h"
#include "awb_param.h"
#include "awb_tuning_custom.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
//adb for log
//#include <cutils/xlog.h>
char * mod_cam = "/sys/devices/platform/image_sensor/camera_info";
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
isAWBEnabled()
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
isAWBCalibrationBypassed()
{
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AWB_STAT_PARAM_T const&
getAWBStatParam()
{
    // AWB Statistics Parameter
    static AWB_STAT_PARAM_T rAWBStatParam =
    {
        // Thresholds
          1, // Low threshold of R
          1, // Low threshold of G
          1, // Low threshold of B
        254, // High threshold of R
        254, // High threshold of G
        254, // High threshold of B

        // Pre-gain maximum limit clipping
           0xFFF, // Maximum limit clipping for R color
           0xFFF, // Maximum limit clipping for G color
           0xFFF, // Maximum limit clipping for B color

        // AWB error threshold
           0 // Programmable threshold for the allowed total over-exposed and under-exposed pixels in one main stat window
    };

    return (rAWBStatParam);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define AWB_CYCLE_NUM_PREVIEW   (4)
#define AWB_CYCLE_NUM_VIDEO (4)

const MINT32*
getAWBActiveCycle_Preview(MINT32 i4SceneLV)
{
    // Default AWB cycle
    static MINT32 i4AWBActiveCycle_Preview[AWB_CYCLE_NUM_PREVIEW] =
    {
        MTRUE,
        MTRUE,//MFALSE,
        MTRUE,
        MTRUE//MFALSE
    };

    return (&i4AWBActiveCycle_Preview[0]);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
const MINT32*
getAWBActiveCycle_Video(MINT32 i4SceneLV)
{
    // Default AWB cycle
    static MINT32 i4AWBActiveCycle_Video[AWB_CYCLE_NUM_VIDEO] =
    {
        MTRUE,
        MTRUE,//MFALSE,
        MTRUE,
        MTRUE//MFALSE
    };

    return (&i4AWBActiveCycle_Video[0]);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
getAWBCycleNum_Preview()
{
    return AWB_CYCLE_NUM_PREVIEW;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
getAWBCycleNum_Video()
{
    return AWB_CYCLE_NUM_VIDEO;
}


