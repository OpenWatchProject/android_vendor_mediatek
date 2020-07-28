#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_AAA_TEST_TEST_3A_RUNNER_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_AAA_TEST_TEST_3A_RUNNER_H_

//
#include <stdio.h>
#include <stdlib.h>
//
#include <vector>
#include <utils/String8.h>

#include <IEventIrq.h>

#include <mtkcam/drv/IHalSensor.h>
#include <private/aaa_hal_private.h>
#include <mtkcam/aaa/IHal3A.h>
#include "aaa_result.h"

// #include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>

#if CAM3_FAKE_SENSOR_DRV
#include "fake_sensor_drv/INormalPipe.h"
#else
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#endif

#include <semaphore.h>
#include <utils/threads.h>
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <hardware/camera3.h>
//#include <system/camera_metadata.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include "aaa_hal_if.h"
#include "aaa_utils.h"

#include <IEventIrq.h>

#include "ICaseControl.h"

using namespace std;
using namespace NSCam;
using namespace NS3Av3;
using namespace android;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;

#define MAX_TOTAL_FAIL_MSG_LEN 500
#define MAX_PER_FAIL_MSG_LEN 50

enum MODULE_3A_T
{
    AAA = 0,
    AF,
    AE,
    AWB,
    MODULE_3A_NUM
};

/*******************************************************************************
*   test IHal3A
********************************************************************************/
class Test3ARunner : public IHal3ACb
{
public:
    //static  Test3ARunner   *createInstance();

    Test3ARunner();
    ~Test3ARunner(){};

    MBOOL initialize();
    MBOOL destroy();

    void run(MINT32 u4Step);
    virtual void doNotifyCb (MINT32  _msgType, MINTPTR _ext1, MINTPTR _ext2, MINTPTR _ext3);

public:

    MBOOL prepareMeta_Set(MODULE_3A_T module, SET_FUNC inFunc);
    MBOOL prepareMeta_SetIsp(MODULE_3A_T module, SET_FUNC_ISP inFunc);
    MBOOL setUpdateMetaFunc_Set(MODULE_3A_T module, SET_FUNC inFunc);
    MBOOL setUpdateMetaFunc_SetIsp(MODULE_3A_T module, SET_FUNC_ISP inFunc);
    MBOOL setVerifyFunc_Get(MODULE_3A_T module, VER_FUNC inFunc);
    MBOOL setVerifyFunc_SetIsp(MODULE_3A_T module, VER_FUNC_ISP inFunc);

    const char* failMsg();

    MBOOL pass();

private:
    // const char[]

private:
    MBOOL powerOnSensor();
    MBOOL onInit();
    MBOOL onUninit();
    MBOOL enque(MUINT32 u4Magic);
    MBOOL deque(MUINT32 u4Magic);

    MVOID dump3AMetaData(MetaSet_T InMeta);


    static IHal3A*      m_pIHal3A;
    NSCam::IHalSensor*  m_pHalSensor;
    MINT32              m_fgPrecap;
    ITemplateRequest*   m_pTemplate;

    static MUINT        m_sensorIdx;
    static MUINT32      m_Magic;
    static MBOOL        m_PreStopVsync;
    static MBOOL        m_WaitVsync;
    static MetaSet_T    m_Request;
    static TuningParam  m_TuningParam;
    static List<MetaSet_T>  m_Controls;
    static sem_t        m_FakeP1Done;
    static SET_FUNC     m_UpdataMetaFunc[MODULE_3A_NUM];
    static SET_FUNC_ISP m_UpdataMetaFunc_Isp[MODULE_3A_NUM];
    static VER_FUNC     m_VerifyFunc[MODULE_3A_NUM];
    static VER_FUNC_ISP m_VerifyFunc_Isp[MODULE_3A_NUM];
    static char         m_FailMsg[MAX_TOTAL_FAIL_MSG_LEN];
    static MBOOL        m_Pass;
    //static Test3ARunner* pTest3ARunner;
};

static Test3ARunner test3ARunner;

#endif