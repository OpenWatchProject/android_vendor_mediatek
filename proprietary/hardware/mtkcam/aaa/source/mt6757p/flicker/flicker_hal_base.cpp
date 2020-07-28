#define LOG_TAG "flicker_hal_base"

#include <stdlib.h>
#include <stdio.h>
#include <mtkcam/def/common.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include <camera_custom_nvram.h>
#include <isp_tuning_mgr.h>
#include <tuning_mgr.h>
#include <camera_custom_cam_cal.h>
#include <mtkcam/drv/mem/cam_cal_drv.h>
#include <mtkcam/drv/IHalSensor.h>
#include <ctype.h>
#include <sys/stat.h>
#include <aaa_sensor_mgr.h>
#include <time.h>
#include <kd_camera_feature.h>
#include <isp_mgr.h>
#include <isp_tuning_mgr.h>
#include <vector>
#include "Flicker_type.h"
#include <utils/threads.h>
#include <cutils/log.h>
#include "camera_custom_nvram.h"
#include "awb_param.h"
#include "ae_param.h"
#include "af_param.h"
#include "sequential_testing.h"
#include "FlickerDetection.h"
#include "camera_custom_flicker.h"
#include "flicker_hal_base.h"
#include "flicker_hal.h"
#include "camera_custom_nvram.h"
#include "flicker_util.h"
#include "aaa_sensor_mgr.h"
#include "camera_custom_flicker_para.h"
#include "ae_feature.h"
#include "aaa_error_code.h"
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>

using namespace NS3Av3;
using namespace NSIspTuning;

inline int errOp(int e1, int e2, int e3, int e4)
{
    if(e1!=0)
        return 0;
    if(e2!=0)
        return 0;
    if(e3!=0)
        return 0;
    if(e4!=0)
        return 0;
    return 1;
}

#define def_Imp(function) \
int FlickerHalBase::function(int senorDev)  \
{   \
    int errMain=0; \
    int errSub=0; \
    int errMain2=0;   \
    int errSub2=0; \
    if (senorDev & ESensorDev_Main)    \
        errMain = FlickerHal::getInstance(ESensorDev_Main)->function();    \
    if (senorDev & ESensorDev_Sub)  \
        errSub = FlickerHal::getInstance(ESensorDev_Sub)->function();  \
    if (senorDev & ESensorDev_MainSecond)    \
        errMain2 = FlickerHal::getInstance(ESensorDev_MainSecond)->function();  \
    if (senorDev & ESensorDev_SubSecond)  \
        errSub2 = FlickerHal::getInstance(ESensorDev_SubSecond)->function();  \
    return errOp(errMain, errSub, errMain2, errSub2);    \
}

#define def_Imp1(function) \
int FlickerHalBase::function(int senorDev, int v1)  \
{   \
    int errMain=0; \
    int errSub=0; \
    int errMain2=0;   \
    int errSub2=0; \
    if (senorDev & ESensorDev_Main)    \
        errMain = FlickerHal::getInstance(ESensorDev_Main)->function(v1);    \
    if (senorDev & ESensorDev_Sub)  \
        errSub = FlickerHal::getInstance(ESensorDev_Sub)->function(v1);  \
    if (senorDev & ESensorDev_MainSecond)    \
        errMain2 = FlickerHal::getInstance(ESensorDev_MainSecond)->function(v1);  \
    if (senorDev & ESensorDev_SubSecond)  \
        errSub2 = FlickerHal::getInstance(ESensorDev_SubSecond)->function(v1);  \
    return errOp(errMain, errSub, errMain2, errSub2);    \
}

#define def_Imp2(function) \
int FlickerHalBase::function(int senorDev, int v1, int v2)  \
{   \
    int errMain=0; \
    int errSub=0; \
    int errMain2=0;   \
    int errSub2=0; \
    if (senorDev & ESensorDev_Main)    \
        errMain = FlickerHal::getInstance(ESensorDev_Main)->function(v1, v2);    \
    if (senorDev & ESensorDev_Sub)  \
        errSub = FlickerHal::getInstance(ESensorDev_Sub)->function(v1, v2);  \
    if (senorDev & ESensorDev_MainSecond)    \
        errMain2 = FlickerHal::getInstance(ESensorDev_MainSecond)->function(v1, v2);  \
    if (senorDev & ESensorDev_SubSecond)  \
        errSub2 = FlickerHal::getInstance(ESensorDev_SubSecond)->function(v1, v2);  \
    return errOp(errMain, errSub, errMain2, errSub2);    \
}

//------------------------------
// imp
#define def_ImpBody(function,senorDev) \
    int errMain=0; \
    int errSub=0; \
    int errMain2=0;   \
    int errSub2=0; \
    if (senorDev & ESensorDev_Main)    \
        errMain = FlickerHal::getInstance(ESensorDev_Main)->function();    \
    if (senorDev & ESensorDev_Sub)  \
        errSub = FlickerHal::getInstance(ESensorDev_Sub)->function();  \
    if (senorDev & ESensorDev_MainSecond)    \
        errMain2 = FlickerHal::getInstance(ESensorDev_MainSecond)->function();  \
    if (senorDev & ESensorDev_SubSecond)  \
        errSub2 = FlickerHal::getInstance(ESensorDev_SubSecond)->function();  \
    return errOp(errMain, errSub, errMain2, errSub2);

#define def_ImpBody1(function,senorDev,p1) \
    int errMain=0; \
    int errSub=0; \
    int errMain2=0;   \
    int errSub2=0; \
    if (senorDev & ESensorDev_Main)    \
        errMain = FlickerHal::getInstance(ESensorDev_Main)->function(p1);    \
    if (senorDev & ESensorDev_Sub)  \
        errSub = FlickerHal::getInstance(ESensorDev_Sub)->function(p1);  \
    if (senorDev & ESensorDev_MainSecond)    \
        errMain2 = FlickerHal::getInstance(ESensorDev_MainSecond)->function(p1);  \
    if (senorDev & ESensorDev_SubSecond)  \
        errSub2 = FlickerHal::getInstance(ESensorDev_SubSecond)->function(p1);  \
    return errOp(errMain, errSub, errMain2, errSub2);


#define def_ImpBody2(function,senorDev,p1, p2) \
    int errMain=0; \
    int errSub=0; \
    int errMain2=0;   \
    int errSub2=0; \
    if (senorDev & ESensorDev_Main)    \
        errMain = FlickerHal::getInstance(ESensorDev_Main)->function(p1, p2);    \
    if (senorDev & ESensorDev_Sub)  \
        errSub = FlickerHal::getInstance(ESensorDev_Sub)->function(p1, p2);  \
    if (senorDev & ESensorDev_MainSecond)    \
        errMain2 = FlickerHal::getInstance(ESensorDev_MainSecond)->function(p1, p2);  \
    if (senorDev & ESensorDev_SubSecond)  \
        errSub2 = FlickerHal::getInstance(ESensorDev_SubSecond)->function(p1, p2);  \
    return errOp(errMain, errSub, errMain2, errSub2);


#define def_ImpBody2_noRet(function,senorDev,p1, p2, ret) \
    int errMain=0; \
    int errSub=0; \
    int errMain2=0;   \
    int errSub2=0; \
    if (senorDev & ESensorDev_Main)    \
        errMain = FlickerHal::getInstance(ESensorDev_Main)->function(p1, p2);    \
    if (senorDev & ESensorDev_Sub)  \
        errSub = FlickerHal::getInstance(ESensorDev_Sub)->function(p1, p2);  \
    if (senorDev & ESensorDev_MainSecond)    \
        errMain2 = FlickerHal::getInstance(ESensorDev_MainSecond)->function(p1, p2);  \
    if (senorDev & ESensorDev_SubSecond)  \
        errSub2 = FlickerHal::getInstance(ESensorDev_SubSecond)->function(p1, p2);  \
    ret = errOp(errMain, errSub, errMain2, errSub2);


//=======================================================
//  implementation
//=======================================================
static int g_flickerEn=1;
FlickerHalBase& FlickerHalBase::getInstance()
{
    static FlickerHalBase obj;
    return obj;

}
/*
int FlickerHalBase::createBuf(int sensorDev)
{
    g_flickerEn = FlickerUtil::getPropInt("vendor.z.flicker_en", 1);
    if(g_flickerEn==0)
        return 1;
    def_ImpBody(createBuf, sensorDev);
}

int FlickerHalBase::releaseBuf(int sensorDev)
{
    if(g_flickerEn==0)
        return 1;
    def_ImpBody(releaseBuf, sensorDev);
}

int FlickerHalBase::close(int sensorDev)
{
    if(g_flickerEn==0)
        return 1;
    def_ImpBody(close, sensorDev);
}
*/
int FlickerHalBase::setTGSize(int sensorDev, int width, int height)
{
    if(g_flickerEn==0)
        return 1;
    //def_ImpBody2(update, sensorDev, in, out);
    def_ImpBody2(setTGSize, sensorDev, width, height);
}

int FlickerHalBase::setSensorMode(int sensorDev, int mode, int width, int height)
{
    if(g_flickerEn==0)
        return 1;
    int errMain=0;
    int errSub=0;
    int errMain2=0;
    int errSub2=0;
    if (sensorDev & ESensorDev_Main)
        errMain = FlickerHal::getInstance(ESensorDev_Main)->setSensorMode(mode, width, height);
    if (sensorDev & ESensorDev_Sub)
        errSub = FlickerHal::getInstance(ESensorDev_Sub)->setSensorMode(mode, width, height);
    if (sensorDev & ESensorDev_MainSecond)
        errMain2 = FlickerHal::getInstance(ESensorDev_MainSecond)->setSensorMode(mode, width, height);
    if (sensorDev & ESensorDev_SubSecond)
        errSub2 = FlickerHal::getInstance(ESensorDev_SubSecond)->setSensorMode(mode, width, height);
    return errOp(errMain, errSub, errMain2, errSub2);
}

int FlickerHalBase::setFlickerMode(int sensorDev, int mode)
{
    if(g_flickerEn==0)
        return 1;
    def_ImpBody1(setFlickerMode, sensorDev, mode);
}
/*
int FlickerHalBase::open(int sensorDev, int a, int b)
{
    if(g_flickerEn==0)
        return 1;
    def_ImpBody2(open, sensorDev, a, b);
}
*/
/*
def_Imp1(setSensorMode)
def_Imp1(setFlickerMode)

def_Imp(createBuf);
def_Imp(releaseBuf);

def_Imp2(open)
def_Imp(close)
*/
static int gFlickerResult = HAL_FLICKER_AUTO_50HZ;
int FlickerHalBase::update(int sensorDev, FlickerInput* in, FlickerOutput* out)
{
    if(g_flickerEn==0)
        return 1;
    //def_ImpBody2(update, sensorDev, in, out);
    int ret;
    AAA_TRACE_G(detectFlicker_SW);
    AAA_TRACE_NL(detectFlicker_SW);
    def_ImpBody2_noRet(update, sensorDev, in, out, ret);
    AAA_TRACE_END_NL;
    AAA_TRACE_END_G;
    gFlickerResult = out->flickerResult;
    return ret;
}


int FlickerHalBase::getFlickerResult(int& flickerResult)
{
    flickerResult = gFlickerResult;
    return 1;
}

int FlickerHalBase::getFlickerResult(int sensorDev, int& flickerResult)
{
    if(g_flickerEn==0)
        return 1;
    def_ImpBody1(getFlickerResult, sensorDev, flickerResult);
}

MBOOL FlickerHalBase::Start(int sensorDev)
{
    g_flickerEn = FlickerUtil::getPropInt("vendor.z.flicker_en", 1);
    if(g_flickerEn==0)
        return MTRUE;
    def_ImpBody(Start, sensorDev);
}

MBOOL FlickerHalBase::Stop(int sensorDev)
{
    if(g_flickerEn==0)
        return MTRUE;
    def_ImpBody(Stop, sensorDev);
}

int FlickerHalBase::setUniInfo(int sensorDev, MBOOL const i4UniInfo)
{
    if(g_flickerEn==0)
        return 1;
    def_ImpBody1(setUniInfo, sensorDev, i4UniInfo);
}

MRESULT FlickerHalBase::Init(int sensorDev, MINT32 const i4SensorIdx)
{
    if(g_flickerEn==0)
        return S_3A_OK;
    def_ImpBody1(Init, sensorDev, i4SensorIdx);
}

MRESULT FlickerHalBase::Uninit(int sensorDev)
{
    if(g_flickerEn==0)
        return S_3A_OK;
    def_ImpBody(Uninit, sensorDev);
}

