#define LOG_TAG "cctsvr_if"

extern "C" {
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/mtkfb.h>
}

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <mtkcam/main/acdk/AcdkTypes.h>
//#include "AcdkLog.h"
//#include "AcdkCctBase.h"
//#include "cct_main.h"
//#include "cct_calibration.h"
//#include <mtkcam/drv/IHalSensor.h>
//#include "AcdkErrCode.h"

//#include "cct_if.h"
//#include "cct_imp.h"
//#include <mtkcam/main/acdk/cct/cct_ErrCode.h>

#include <cutils/log.h>
#include <pthread.h>
#include "cct_server.h"
/*
*    @CCT_IF.CPP
*    CCT_IF provides user to do camera calibration or tuning by means of
*    mixing following commands. Which involoved with
*    3A, ISP, sensor, NVRAM, Calibration, ACDK releated.
*
*    CCT FW working model is based on ACDK framework, so it is required make
*    sure AcdkCctBase object is available before CCTIF Open.
*
*    CCT FW presents the following APIs:
*
*    CCTIF_Open
*    CCTIF_Close
*    CCTIF_Init
*    CCTIF_DeInit
*    CCTIF_IOControl
*/

/********************************************************************************/
#define MY_LOGD(fmt, arg...)        ALOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        ALOGE("[%s] " fmt, __FUNCTION__, ##arg)
/********************************************************************************/

CctServer *gpCctSvrObj = NULL;
MBOOL gbCaptureCallback = MFALSE;

#ifdef __cplusplus
extern "C" {
#endif


MBOOL CctSvrIF_Init(MINT32 sensorID)
{
    MY_LOGD("+");

    if (gpCctSvrObj == NULL)
        gpCctSvrObj = new CctServer( (CCT_SENSOR_TYPE_T)sensorID );
    else
        MY_LOGD("CctServer already exist, CaptureCallback flag=%s", gbCaptureCallback? "true":"false");

    MY_LOGD("-");

    return /*err*/MTRUE;
}


MBOOL CctSvrIF_DeInit()
{
    MY_LOGD("+");

    if(gpCctSvrObj == NULL) {
        return MFALSE;
    } else if(gbCaptureCallback) {
        MY_LOGD("Capture callback, don't delete CctServer");
    } else {
        delete gpCctSvrObj;
        gpCctSvrObj = NULL;
    }

    MY_LOGD("-");
    return MTRUE;
}


MBOOL CctSvrIF_Ctrl(MUINT32 cmd)
{
    MY_LOGD("+: cmd = 0x%x", cmd);

    if(gpCctSvrObj == NULL) {
        MY_LOGD("no CctServer object");
        return MFALSE;
    } else if(gbCaptureCallback) {
        MY_LOGD("Capture callback, don't call CctServer thread");
    } else {
        gpCctSvrObj->CCT_ServerCtrl((CCT_SVR_CTL_T)cmd);
    }

    MY_LOGD("-");

    return MTRUE;
}

#ifdef __cplusplus
} // extern "C"
#endif
