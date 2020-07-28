
#define LOG_TAG "Test3ARunner"

#include <mtkcam/utils/std/Log.h>
#include "test_3a_runner.h"

#define MY_LOG(fmt, arg...)                  CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_ERR(fmt, arg...)                  CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)

IHal3A*      Test3ARunner::m_pIHal3A;
MUINT        Test3ARunner::m_sensorIdx;
MUINT32      Test3ARunner::m_Magic;
MBOOL        Test3ARunner::m_PreStopVsync;
MBOOL        Test3ARunner::m_WaitVsync;
MetaSet_T    Test3ARunner::m_Request;
TuningParam  Test3ARunner::m_TuningParam;
List<MetaSet_T>  Test3ARunner::m_Controls;
sem_t        Test3ARunner::m_FakeP1Done;
SET_FUNC     Test3ARunner::m_UpdataMetaFunc[MODULE_3A_NUM];
SET_FUNC_ISP Test3ARunner::m_UpdataMetaFunc_Isp[MODULE_3A_NUM];
VER_FUNC     Test3ARunner::m_VerifyFunc[MODULE_3A_NUM];
VER_FUNC_ISP Test3ARunner::m_VerifyFunc_Isp[MODULE_3A_NUM];
char         Test3ARunner::m_FailMsg[MAX_TOTAL_FAIL_MSG_LEN];
MBOOL        Test3ARunner::m_Pass;

//Test3ARunner*         Test3ARunner::pTest3ARunner = NULL;

/*******************************************************************************
*   test IHal3A
********************************************************************************/
// Test3ARunner* Test3ARunner::createInstance()
// {
//     Test3ARunner* p3ARun = NULL;

//     MY_LOG("+");
//     p3ARun = Test3ARunner::pTest3ARunner;
//     if (p3ARun == NULL) {
//         Test3ARunner::pTest3ARunner = new Test3ARunner();
//         p3ARun = Test3ARunner::pTest3ARunner;
//     }
//     MY_LOG("-");

//     return p3ARun;
// }

Test3ARunner::
Test3ARunner()
    : m_fgPrecap(0)
    , m_pHalSensor(NULL)
    , m_pTemplate(NULL)
{
    MY_LOG("this(%p)", this);
    for(int i=0; i<MODULE_3A_NUM; i++)
    {
        m_VerifyFunc[i] = NULL;
        m_VerifyFunc_Isp[i] = NULL;
        m_UpdataMetaFunc[i] = NULL;
        m_UpdataMetaFunc_Isp[i] = NULL;
    }
    memset(m_FailMsg, '\0', sizeof(char)*MAX_TOTAL_FAIL_MSG_LEN);
    m_Pass = MTRUE;
    m_pIHal3A = NULL;
}

MBOOL
Test3ARunner::
initialize()
{
    m_PreStopVsync = MFALSE;
    m_WaitVsync = MFALSE;
    m_Magic = 2;
    if (onInit())
    {
        UPDATE_ENTRY_SINGLE(m_Request.halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, (MINT32)0);
        m_Request.appMeta = m_pTemplate->getMtkData(CAMERA3_TEMPLATE_PREVIEW);
        m_Controls.push_back(m_Request);
        UPDATE_ENTRY_SINGLE(m_Request.halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, (MINT32)1);
        m_Controls.push_back(m_Request);
        UPDATE_ENTRY_SINGLE(m_Request.halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, (MINT32)m_Magic);

        m_Controls.push_back(m_Request);

        MY_LOG("dump 3A meta +");
        // dump3AMetaData(m_Request);
        MY_LOG("dump 3A meta -");

        // // set
        m_pIHal3A->set(m_Controls);

        return MTRUE;

    }
    return MFALSE;
}

MBOOL
Test3ARunner::
destroy()
{
    return onUninit();
}

void
Test3ARunner::
run(MINT32 u4Step)
{
    MY_LOG("+");
    MUINT32 u4FrmCount = 1;
    m_PreStopVsync = MFALSE;
    m_WaitVsync = MTRUE;

    ::sem_wait(&m_FakeP1Done);
    while (u4FrmCount <= u4Step)
    {
        MY_LOG("wait +");
        ::sem_wait(&m_FakeP1Done);

        if(u4FrmCount == (u4Step - 1))
        {
            ::usleep(50);
            m_PreStopVsync = MTRUE;
        }

        deque(m_Magic-1);
        MY_LOG("wait %d -, m_Pass(%d)", u4FrmCount, m_Pass);
        if(!m_Pass) break;
        u4FrmCount ++;
    }
    m_WaitVsync = MFALSE;
    MY_LOG("-");
}

void
Test3ARunner::
doNotifyCb (MINT32  _msgType, MINTPTR _ext1, MINTPTR _ext2, MINTPTR _ext3)
{
    if (_msgType == IHal3ACb::eID_NOTIFY_VSYNC_DONE)
    {
        MY_LOG("eID_NOTIFY_VSYNC_DONE magic(%d) + ", m_Magic);
        // MINT32 magic  = static_cast<MINT32>(_ext1);
        m_Magic++;
        if(!m_WaitVsync){
            return;
        }

        ::sem_post(&m_FakeP1Done);

        if(m_PreStopVsync){
            return;
        }

        MINT32 status = static_cast<MINT32>(_ext2);
        // enque
        enque(m_Magic-1);
        // remove old
        m_Controls.erase(m_Controls.begin());

        // aquire new request

        UPDATE_ENTRY_SINGLE(m_Request.halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM, (MINT32)m_Magic);
        // if (m_fgPrecap == 2)
        // {
        //     m_fgPrecap = 0;
        //     m_Request.appMeta = m_pTemplate->getMtkData(CAMERA3_TEMPLATE_STILL_CAPTURE);
        // }
        // else
        // {
        //     m_Request.appMeta = m_pTemplate->getMtkData(CAMERA3_TEMPLATE_PREVIEW);
        // }
        // if (magic == 15)
        // {   // emulate precapture trigger
        //     UPDATE_ENTRY_SINGLE(m_Request.appMeta, MTK_CONTROL_AE_PRECAPTURE_TRIGGER, (MUINT8)MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START);
        // }
        // else
        // {
        //     UPDATE_ENTRY_SINGLE(m_Request.appMeta, MTK_CONTROL_AE_PRECAPTURE_TRIGGER, (MUINT8)MTK_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE);
        // }
        // UPDATE_ENTRY_SINGLE(m_Request.appMeta, MTK_STATISTICS_LENS_SHADING_MAP_MODE, (MUINT8)MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON);

        MetaSet_T& rMetaSet = m_Request;
        for(int i=0; i<MODULE_3A_NUM; i++)
        {
            if(!m_UpdataMetaFunc[i]) continue;
            m_UpdataMetaFunc[i](m_Magic, rMetaSet);
        }

        m_Controls.push_back(m_Request);

        // set
        m_pIHal3A->set(m_Controls);
        MY_LOG("eID_NOTIFY_VSYNC_DONE -");
    }
    else if(_msgType == IHal3ACb::eID_NOTIFY_3APROC_FINISH)
    {
        MY_LOG("eID_NOTIFY_3APROC_FINISH");
    }

}

MBOOL
Test3ARunner::
prepareMeta_Set(MODULE_3A_T module, SET_FUNC inFunc)
{
    MBOOL ret = MTRUE;
    MetaSet_T& rMetaSet = m_Request;
    if(!inFunc(0, rMetaSet))
    {
        ret = MFALSE;
    }
    return ret;
}

MBOOL
Test3ARunner::
prepareMeta_SetIsp(MODULE_3A_T module, SET_FUNC_ISP inFunc)
{
    MBOOL ret = MTRUE;
    MetaSet_T rMetaSet;
    TuningParam& rTuningParam = m_TuningParam;
    if(!inFunc(0, rMetaSet, rTuningParam))
    {
        ret = MFALSE;
    }
    return ret;
}

MBOOL
Test3ARunner::
setUpdateMetaFunc_Set(MODULE_3A_T module, SET_FUNC inFunc)
{
    MBOOL ret = MTRUE;
    switch(module)
    {
        case AAA:
        case AE:
        case AF:
        case AWB:
            m_UpdataMetaFunc[module] = inFunc;
        default:
            break;
    }

    return ret;
}

MBOOL
Test3ARunner::
setUpdateMetaFunc_SetIsp(MODULE_3A_T module, SET_FUNC_ISP inFunc)
{
    MBOOL ret = MTRUE;
    switch(module)
    {
        case AAA:
        case AE:
        case AF:
        case AWB:
            m_UpdataMetaFunc_Isp[module] = inFunc;
        default:
            break;
    }

    return ret;
}

MBOOL
Test3ARunner::
setVerifyFunc_Get(MODULE_3A_T module, VER_FUNC inFunc)
{
    MBOOL ret = MTRUE;
    switch(module)
    {
        case AAA:
        case AE:
        case AF:
        case AWB:
            m_VerifyFunc[(MODULE_3A_T)module] = inFunc;
        default:
            break;
    }

    return ret;
}

MBOOL
Test3ARunner::
setVerifyFunc_SetIsp(MODULE_3A_T module, VER_FUNC_ISP inFunc)
{
    MBOOL ret = MTRUE;
    switch(module)
    {
        case AAA:
        case AE:
        case AF:
        case AWB:
            m_VerifyFunc_Isp[(MODULE_3A_T)module] = inFunc;
        default:
            break;
    }

    return ret;
}

const char*
Test3ARunner::
failMsg()
{
    return m_FailMsg;
}

MBOOL
Test3ARunner::
pass()
{
    return m_Pass;
}

MBOOL
Test3ARunner::
powerOnSensor()
{
    MY_LOG("+");
    MBOOL    ret = MFALSE;
    //  (1) Open Sensor
    NSCam::IHalSensorList* const pHalSensorList = MAKE_HalSensorList();

    MUINT32 sensorDev = 0;
    MUINT32 sensorCount = 0;
    SensorStaticInfo sensorStaticInfo;
    if (!pHalSensorList)
    {
        MY_ERR("pHalSensorList == NULL");
        goto lbExit;
    }
    //
    pHalSensorList->searchSensors();
    m_pHalSensor = pHalSensorList->createSensor(
                                        LOG_TAG,
                                        0);
    if (m_pHalSensor == NULL)
    {
        MY_ERR("m_pHalSensor is NULL");
        goto lbExit;
    }

    m_sensorIdx = 0;
    if( !m_pHalSensor->powerOn(LOG_TAG, 1, &m_sensorIdx) )
    {
        MY_ERR("sensor power on failed: %d", m_sensorIdx);
        goto lbExit;
    }

    sensorDev = pHalSensorList->querySensorDevIdx(m_sensorIdx);
    memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));
    pHalSensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);
    sensorCount = pHalSensorList->queryNumberOfSensors();
    MY_LOG("sensorCount:%d ", sensorCount);

    ret = MTRUE;
lbExit:
    MY_LOG(" -");
    return ret;
}

MBOOL
Test3ARunner::
onInit()
{
    MY_LOG("+");
    if(::sem_init(&m_FakeP1Done, 0, 0)==-1)
    {
        MY_LOG("sem init fail");
    }
    MY_LOG("m_FakeP1Done = %d", m_FakeP1Done);
    if (!powerOnSensor())
    {
        MY_ERR("Fail to power on sensor");
        return MFALSE;
    }


    MINT32 const dev = 0;

    m_pIHal3A = MAKE_Hal3A(dev, LOG_TAG);
    if (!m_pIHal3A)
    {
        MY_ERR("Fail to create IHal3A");
        return MFALSE;
    }

    m_pIHal3A->notifyPwrOn();
    m_pIHal3A->setSensorMode(SENSOR_SCENARIO_ID_NORMAL_PREVIEW);
    m_pIHal3A->attachCb(IHal3ACb::eID_NOTIFY_VSYNC_DONE, this);
    m_pIHal3A->attachCb(IHal3ACb::eID_NOTIFY_3APROC_FINISH, this);
    m_pIHal3A->config(1);
    m_pIHal3A->start();

    m_pTemplate = ITemplateRequest::getInstance(0);
    MY_LOG("-");
    return MTRUE;
}

MBOOL
Test3ARunner::
onUninit()
{

    if (m_pIHal3A)
    {
        m_pIHal3A->stopStt();
        m_pIHal3A->detachCb(IHal3ACb::eID_NOTIFY_VSYNC_DONE, this);
        m_pIHal3A->stop();
        m_pIHal3A->notifyPwrOff();
        m_pIHal3A->destroyInstance(LOG_TAG);
        m_pIHal3A = NULL;
    }

    if (m_pHalSensor)
    {
        if( !m_pHalSensor->powerOff(LOG_TAG, 1, &m_sensorIdx) )
        {
            MY_ERR("sensor power off failed: %d", m_sensorIdx);
            return MFALSE;
        }
        m_pHalSensor = NULL;
    }

    ::sem_destroy(&m_FakeP1Done);
    return MTRUE;
}

MBOOL
Test3ARunner::
enque(MUINT32 u4Magic)
{
    MY_LOG("u4Magic(%d)", u4Magic);
    return MTRUE;
}

MBOOL
Test3ARunner::
deque(MUINT32 u4Magic)
{
    MY_LOG("u4Magic(%d), m_Pass(%d)", u4Magic, m_Pass);
    MBOOL ret = MTRUE;
    MetaSet_T result, result2;
    m_pIHal3A->get(u4Magic, result);
    for(int i=0; i<MODULE_3A_NUM; i++)
    {
        char perFailMsg[MAX_PER_FAIL_MSG_LEN] = {'\0'};
        if(!m_VerifyFunc[i]) continue;
        if(!m_VerifyFunc[i](u4Magic, result, perFailMsg))
        {
            MY_LOG("verifyFunc_Get fail");
            m_Pass = MFALSE;
            strcat(m_FailMsg, perFailMsg);
            strcat(m_FailMsg, "\n");
        }
    }

/*
    MUINT8 u1AeState, u1AwbState, u1AfState, u1AePrecap;
    NSIspTuning::RAWIspCamInfo_U rCamInfo;
    QUERY_ENTRY_SINGLE(result.appMeta, MTK_CONTROL_AE_PRECAPTURE_TRIGGER, u1AePrecap);
    QUERY_ENTRY_SINGLE(result.appMeta, MTK_CONTROL_AE_STATE, u1AeState);
    QUERY_ENTRY_SINGLE(result.appMeta, MTK_CONTROL_AF_STATE, u1AfState);
    QUERY_ENTRY_SINGLE(result.appMeta, MTK_CONTROL_AWB_STATE, u1AwbState);
    if (u1AePrecap)
    {
        m_fgPrecap = 1;
    }
    if (m_fgPrecap == 1 && ((u1AeState == MTK_CONTROL_AE_STATE_CONVERGED) || (u1AeState == MTK_CONTROL_AE_STATE_FLASH_REQUIRED)))
    {
        // cap
        m_fgPrecap = 2;
    }

    MY_LOG("AE(%d), AF(%d), AWB(%d)", u1AeState, u1AfState, u1AwbState);
    MUINT8 u1ShadingMapMode = 0;
    if (QUERY_ENTRY_SINGLE(result.appMeta, MTK_STATISTICS_LENS_SHADING_MAP_MODE, u1ShadingMapMode))
    {
        if (u1ShadingMapMode == MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON)
        {
            MFLOAT rMap[4] = {0.0f};
            if (GET_ENTRY_ARRAY(result.appMeta, MTK_STATISTICS_LENS_SHADING_MAP, rMap, 4))
            {
                MY_LOG("{%3.3f, %3.3f, %3.3f, %3.3f}", rMap[0], rMap[1], rMap[2], rMap[3]);
            }
            else
            {
                MY_ERR("Fail to get shading map");
            }
        }
    }
    if (GET_ENTRY_ARRAY(result.halMeta, MTK_PROCESSOR_CAMINFO, rCamInfo.data, sizeof(NSIspTuning::RAWIspCamInfo_U)))
    {
        MY_LOG("rCamInfo.u4Id(%d)", rCamInfo.u4Id);
        MY_LOG("rCamInfo.eIspProfile(%d)", rCamInfo.eIspProfile);
        MY_LOG("rCamInfo.eSensorMode(%d)", rCamInfo.eSensorMode);
        MY_LOG("rCamInfo.eIdx_Scene(%d)", rCamInfo.eIdx_Scene);
        MY_LOG("rCamInfo.u4ISOValue(%d)", rCamInfo.u4ISOValue);
        MY_LOG("rCamInfo.eIdx_ISO(%d)", rCamInfo.eIdx_ISO);
        MY_LOG("rCamInfo.eIdx_PCA_LUT(%d)", rCamInfo.eIdx_PCA_LUT);
        MY_LOG("rCamInfo.eIdx_CCM(%d)", rCamInfo.eIdx_CCM);
        MY_LOG("rCamInfo.eIdx_Shading_CCT(%d)", rCamInfo.eIdx_Shading_CCT);
        MY_LOG("rCamInfo.i4ZoomRatio_x100(%d)", rCamInfo.i4ZoomRatio_x100);
        MY_LOG("rCamInfo.i4LightValue_x10(%d)", rCamInfo.i4LightValue_x10);
    }*/

    for(int i=0; i<MODULE_3A_NUM; i++)
    {
        if(!m_UpdataMetaFunc_Isp[i]) continue;
        m_UpdataMetaFunc_Isp[i](m_Magic, result, m_TuningParam);
    }

    m_pIHal3A->setIsp(MFALSE, result, &m_TuningParam, &result2);

    for(int i=0; i<MODULE_3A_NUM; i++)
    {
        char perFailMsg[MAX_PER_FAIL_MSG_LEN] = {'\0'};
        if(!m_VerifyFunc_Isp[i]) continue;
        if(!m_VerifyFunc_Isp[i](u4Magic, result2, m_TuningParam, perFailMsg))
        {
            MY_LOG("verifyFunc_SetIsp fail");
            m_Pass = MFALSE;
            strcat(m_FailMsg, perFailMsg);
            strcat(m_FailMsg, "\n");
        }
    }

    return ret;
}

MVOID
Test3ARunner::
dump3AMetaData(MetaSet_T InMeta)
{
    // dump 3a appMeta
    const IMetadata& appMeta = InMeta.appMeta;
    for (int i = 0; i < appMeta.count(); i++)
    {
        IMetadata::IEntry entry = appMeta.entryAt(i);
        MUINT32 tag = entry.tag();

        switch (tag)
        {
        case MTK_CONTROL_MODE:  // dynamic
            {
                MUINT8 u1ControlMode = entry.itemAt(0, Type2Type< MUINT8 >());
                MY_LOG("MTK_CONTROL_MODE(%d)", u1ControlMode);
            }
            break;
        case MTK_CONTROL_CAPTURE_INTENT:
            {
                MUINT8 u1CapIntent = entry.itemAt(0, Type2Type< MUINT8 >());
                MY_LOG("MTK_CONTROL_CAPTURE_INTENT(%d)", u1CapIntent);
            }
            break;
        // AWB
        case MTK_CONTROL_AWB_LOCK:
            {
                MUINT8 bLock = entry.itemAt(0, Type2Type< MUINT8 >());
                MY_LOG("MTK_CONTROL_AWB_LOCK(%d)", bLock);
            }
            break;
        case MTK_CONTROL_AWB_MODE:  // dynamic
            {
                MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                MY_LOG("MTK_CONTROL_AWB_MODE(%d)", u1Mode);
            }
            break;
        case MTK_CONTROL_AWB_REGIONS:
            {
                MINT32 numRgns = entry.count() / 5;
                MY_LOG("MTK_CONTROL_AWB_REGIONS(%d)", numRgns);
                for (MINT32 k = 0; k < numRgns; k++)
                {
                    CameraArea_T rArea;
                    rArea.i4Left     = entry.itemAt(k*5+0, Type2Type< MINT32 >());
                    rArea.i4Top      = entry.itemAt(k*5+1, Type2Type< MINT32 >());
                    rArea.i4Right    = entry.itemAt(k*5+2, Type2Type< MINT32 >());
                    rArea.i4Bottom   = entry.itemAt(k*5+3, Type2Type< MINT32 >());
                    rArea.i4Weight   = entry.itemAt(k*5+4, Type2Type< MINT32 >());
                    MY_LOG("MTK_CONTROL_AWB_REGIONS L(%d) T(%d) R(%d) B(%d) W(%d)", rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                }
            }
            break;
        // AE
        case MTK_CONTROL_AE_ANTIBANDING_MODE:
            {
                MINT32 i4Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                MY_LOG("MTK_CONTROL_AE_ANTIBANDING_MODE(%d)", i4Mode);
            }
            break;
        case MTK_CONTROL_AE_EXPOSURE_COMPENSATION:
            {
                MINT32 i4ExpIdx = entry.itemAt(0, Type2Type< MINT32 >());
                MY_LOG("MTK_CONTROL_AE_EXPOSURE_COMPENSATION(%d)", i4ExpIdx);
            }
            break;
        case MTK_CONTROL_AE_LOCK:
            {
                MUINT8 bLock = entry.itemAt(0, Type2Type< MUINT8 >());
                MY_LOG("MTK_CONTROL_AE_LOCK(%d)", bLock);
            }
            break;
        case MTK_CONTROL_AE_MODE:
            {
                MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                MY_LOG("MTK_CONTROL_AE_MODE(%d)", u1Mode);
            }
            break;
        case MTK_CONTROL_AE_REGIONS:    // dynamic
            {
                MINT32 numRgns = entry.count() / 5;
                MY_LOG("MTK_CONTROL_AE_REGIONS(%d)", numRgns);
                for (MINT32 k = 0; k < numRgns; k++)
                {
                    CameraArea_T rArea;
                    rArea.i4Left     = entry.itemAt(k*5+0, Type2Type< MINT32 >());
                    rArea.i4Top      = entry.itemAt(k*5+1, Type2Type< MINT32 >());
                    rArea.i4Right    = entry.itemAt(k*5+2, Type2Type< MINT32 >());
                    rArea.i4Bottom   = entry.itemAt(k*5+3, Type2Type< MINT32 >());
                    rArea.i4Weight   = entry.itemAt(k*5+4, Type2Type< MINT32 >());
                    MY_LOG("MTK_CONTROL_AE_REGIONS L(%d) T(%d) R(%d) B(%d) W(%d)", rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                }
            }
            break;
        case MTK_CONTROL_AE_TARGET_FPS_RANGE:
            {
                MINT32 i4MinFps = entry.itemAt(0, Type2Type< MINT32 >());
                MINT32 i4MaxFps = entry.itemAt(1, Type2Type< MINT32 >());
                MY_LOG("MTK_CONTROL_AE_TARGET_FPS_RANGE(%d,%d)", i4MinFps, i4MaxFps);
            }
            break;
        case MTK_CONTROL_AE_PRECAPTURE_TRIGGER:
            {
                MUINT8 u1AePrecapTrig = entry.itemAt(0, Type2Type< MUINT8 >());
                MY_LOG("MTK_CONTROL_AE_PRECAPTURE_TRIGGER(%d)", u1AePrecapTrig);
            }
            break;
        case MTK_CONTROL_AE_PRECAPTURE_ID:
            {
                MINT32 i4AePreCapId = entry.itemAt(0, Type2Type< MINT32 >());
                MY_LOG("MTK_CONTROL_AE_PRECAPTURE_ID(%d)", i4AePreCapId);
            }
            break;

        case MTK_FLASH_MODE:
            {
                MUINT8 u1Mode = entry.itemAt(0, Type2Type< MUINT8 >());
                MY_LOG("MTK_FLASH_MODE(%d)", u1Mode);
            }
            break;

        // Sensor
        case MTK_SENSOR_EXPOSURE_TIME:
            {
                MINT64 i8AeExposureTime = entry.itemAt(0, Type2Type< MINT64 >());
                MY_LOG("MTK_SENSOR_EXPOSURE_TIME(%ld)", i8AeExposureTime);
            }
            break;
        case MTK_SENSOR_SENSITIVITY:
            {
                MINT32 i4AeSensitivity = entry.itemAt(0, Type2Type< MINT32 >());
                MY_LOG("MTK_SENSOR_SENSITIVITY(%d)", i4AeSensitivity);
            }
            break;
        case MTK_SENSOR_FRAME_DURATION:
            {
                MINT64 i8FrameDuration = entry.itemAt(0, Type2Type< MINT64 >());
                MY_LOG("MTK_SENSOR_FRAME_DURATION(%ld)", i8FrameDuration);
            }
            break;
        case MTK_BLACK_LEVEL_LOCK:
            {
                MUINT8 u1BlackLvlLock = entry.itemAt(0, Type2Type< MUINT8 >());
                MY_LOG("MTK_BLACK_LEVEL_LOCK(%d)", u1BlackLvlLock);
            }
            break;

        // AF
        case MTK_CONTROL_AF_MODE:
            {
                MUINT8 u1AfMode = entry.itemAt(0, Type2Type< MUINT8 >());
                MY_LOG("MTK_CONTROL_AF_MODE(%d)", u1AfMode);
            }
            break;
        case MTK_CONTROL_AF_REGIONS:    // dynamic
            {
                    MINT32 numRgns = entry.count() / 5;
                    MY_LOG("MTK_CONTROL_AF_REGIONS(%d)", numRgns);
                    for (MINT32 k = 0; k < numRgns; k++)
                    {
                        CameraArea_T rArea;
                        rArea.i4Left     = entry.itemAt(k*5+0, Type2Type< MINT32 >());
                        rArea.i4Top      = entry.itemAt(k*5+1, Type2Type< MINT32 >());
                        rArea.i4Right    = entry.itemAt(k*5+2, Type2Type< MINT32 >());
                        rArea.i4Bottom   = entry.itemAt(k*5+3, Type2Type< MINT32 >());
                        rArea.i4Weight   = entry.itemAt(k*5+4, Type2Type< MINT32 >());
                        MY_LOG("MTK_CONTROL_AF_REGIONS L(%d) T(%d) R(%d) B(%d) W(%d)", rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom, rArea.i4Weight);
                }
            }
            break;
        case MTK_CONTROL_AF_TRIGGER:
            {
                MUINT8 u1AfTrig = entry.itemAt(0, Type2Type< MUINT8 >());
                MY_LOG("MTK_CONTROL_AF_TRIGGER(%d)", u1AfTrig);
            }
            break;
        case MTK_CONTROL_AF_TRIGGER_ID:
            {
                MINT32 i4AfTrigId = entry.itemAt(0, Type2Type< MINT32 >());
                MY_LOG("MTK_CONTROL_AF_TRIGGER_ID(%d)", i4AfTrigId);
            }
            break;

        // Lens
        case MTK_LENS_FOCUS_DISTANCE:
            {
                MFLOAT fFocusDist = entry.itemAt(0, Type2Type< MFLOAT >());
                MY_LOG("MTK_LENS_FOCUS_DISTANCE(%3.6f)", fFocusDist);
            }
            break;

        // ISP
        case MTK_CONTROL_EFFECT_MODE:
            {
                MUINT8 u1EffectMode = entry.itemAt(0, Type2Type< MUINT8 >());
                MY_LOG("MTK_CONTROL_EFFECT_MODE(%d)", u1EffectMode);
            }
            break;
        case MTK_CONTROL_SCENE_MODE:
            {
                MUINT8 u1SceneMode = entry.itemAt(0, Type2Type< MUINT8 >());
                MY_LOG("MTK_CONTROL_SCENE_MODE(%d)", u1SceneMode);
            }
            break;
        case MTK_EDGE_MODE:
            {
                MUINT8 u1EdgeMode = entry.itemAt(0, Type2Type< MUINT8 >());
                MY_LOG("MTK_EDGE_MODE(%d)", u1EdgeMode);
            }
            break;
        case MTK_NOISE_REDUCTION_MODE:
            {
                MUINT8 u1NRMode = entry.itemAt(0, Type2Type< MUINT8 >());
                MY_LOG("MTK_NOISE_REDUCTION_MODE(%d)", u1NRMode);
            }
            break;

        // Color correction
        case MTK_COLOR_CORRECTION_MODE:
            {
                MUINT8 u1ColorCrctMode = entry.itemAt(0, Type2Type<MUINT8>());
                MY_LOG("MTK_COLOR_CORRECTION_MODE(%d)", u1ColorCrctMode);
            }
            break;
        case MTK_COLOR_CORRECTION_GAINS:
            {
                // [R G_even G_odd B]
                MFLOAT fGains[4];
                fGains[0] = entry.itemAt(0, Type2Type<MFLOAT>());
                fGains[1] = entry.itemAt(1, Type2Type<MFLOAT>());
                fGains[2] = entry.itemAt(2, Type2Type<MFLOAT>());
                fGains[3] = entry.itemAt(3, Type2Type<MFLOAT>());
                MY_LOG("MTK_COLOR_CORRECTION_GAINS(%f, %f, %f, %f)", fGains[0], fGains[1], fGains[2], fGains[3]);
            }
            break;
        case MTK_COLOR_CORRECTION_TRANSFORM:
            {
                for (MINT32 k = 0; k < 9; k++)
                {
                    MRational rMat;
                    MFLOAT fVal = 0.0f;
                    rMat = entry.itemAt(k, Type2Type<MRational>());
                    MY_LOG("MTK_COLOR_CORRECTION_TRANSFORM rMat[%d]=(%d/%d)=(%3.6f)", k, rMat.numerator, rMat.denominator, fVal);
                }
            }
            break;
        case MTK_COLOR_CORRECTION_ABERRATION_MODE:
            {
                MUINT8 u1ColorAberrationMode = entry.itemAt(0, Type2Type<MUINT8>());
                MY_LOG("MTK_COLOR_CORRECTION_ABERRATION_MODE(%d)", u1ColorAberrationMode);
            }
            break;
        // Shading
        case MTK_SHADING_MODE:
            {
                MUINT8 u1ShadingMode = entry.itemAt(0, Type2Type<MUINT8>());
                MY_LOG("MTK_SHADING_MODE(%d)", u1ShadingMode);
            }
            break;
        case MTK_STATISTICS_LENS_SHADING_MAP_MODE:
            {
                MUINT8 u1ShadingMapMode = entry.itemAt(0, Type2Type<MUINT8>());
                MY_LOG("MTK_STATISTICS_LENS_SHADING_MAP_MODE(%d)", u1ShadingMapMode);
            }
            break;
/* WARNING! Muse TO FIX
        case MTK_FACE_FEATURE_FORCE_FACE_3A:
            {
                i4ForceFace3A = entry.itemAt(0, Type2Type<MINT32>());
                MY_LOG(1, "[%s] MTK_FACE_FEATURE_FORCE_FACE_3A(%d)", i4ForceFace3A);
            }
            break;
*/
        case MTK_JPEG_THUMBNAIL_SIZE:
        case MTK_JPEG_ORIENTATION:
            break;
        default:
            break;
        }
    }

    // dump 3a halMeta
    const IMetadata& halMeta = InMeta.halMeta;
    for (int i = 0; i < halMeta.count(); i++)
    {
        IMetadata::IEntry entry = halMeta.entryAt(i);
        MUINT32 tag = entry.tag();

        switch (tag)
        {
        case MTK_P1NODE_PROCESSOR_MAGICNUM:
            {
                MINT32 i4Magic = entry.itemAt(0, Type2Type<MINT32>());
                MY_LOG("MTK_P1NODE_PROCESSOR_MAGICNUM(%d)", i4Magic);
            }
            break;
        case MTK_P1NODE_MIN_FRM_DURATION:
            {
                MINT64 i8FrameDuration = entry.itemAt(0, Type2Type<MINT64>());
                MY_LOG("MTK_P1NODE_MIN_FRM_DURATION(%d)", i8FrameDuration);
            }
            break;
        case MTK_HAL_REQUEST_ISO_SPEED:
            {
                MINT32 i4IsoSpeed = entry.itemAt(0, Type2Type<MINT32>());
                MY_LOG("MTK_HAL_REQUEST_ISO_SPEED(%d)", i4IsoSpeed);
            }
            break;
        case MTK_3A_ISP_PROFILE:
            {
                MUINT8 u1IspProfile = entry.itemAt(0, Type2Type<MUINT8>());
                MY_LOG("MTK_3A_ISP_PROFILE(%d)", u1IspProfile);
            }
            break;
        case MTK_P1NODE_RAW_TYPE:
            {
                MINT32 i4RawType = entry.itemAt(0, Type2Type<MINT32>());
                MY_LOG("MTK_P1NODE_RAW_TYPE(%d)", i4RawType);
            }
            break;
        /*
        case :
            {
                MINT64 i8FrameDuration = entry.itemAt(0, Type2Type<MINT64>());
                MY_LOG("(%d)", i8FrameDuration);
            }
            break;
        */
        default:
            break;
        }
    }
}