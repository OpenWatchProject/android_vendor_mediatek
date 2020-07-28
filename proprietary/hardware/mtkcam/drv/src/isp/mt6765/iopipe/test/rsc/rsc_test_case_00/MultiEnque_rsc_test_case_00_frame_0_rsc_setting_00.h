extern "C" {
#include "frame_0/rsc_test_case_00_frame_0.h"
#include "frame_1/rsc_test_case_00_frame_1.h"
}
#include <mtkcam/def/common.h>
#include <mtkcam/drv/iopipe/PostProc/IRscStream.h>
#include <drv/imem_drv.h>
#include "rsc/rsc_testcommon.h"
using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSRsc;
bool multi_enque_rsc_tc00_frames_RSC_Config();

extern int g_bMultipleBuffer;
extern int g_RscCount;
