/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define LOG_TAG "lsc_mgr_dft"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#define GLOBAL_ENABLE_MY_LOG    (1)
#endif

#include "LscMgrDefault.h"
#include <mtkcam/drv/IHalSensor.h>
#include <LscUtil.h>

#include "shading_tuning_custom.h"
#include "tsf_tuning_custom.h"
#include <ae_param.h>

#include <aaa_sensor_mgr.h>

#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/std/ULog.h>
#include <aaa_trace.h>

using namespace NSCam;

static LscMgrDefault* _pLscMain;
static LscMgrDefault* _pLscMain2;
static LscMgrDefault* _pLscSub;
static LscMgrDefault* _pLscSub2;


ILscMgr*
LscMgrDefault::
createInstance(ESensorDev_T const eSensorDev, MINT32 i4SensorIdx)
{
    LSC_LOG_BEGIN("eSensorDev(0x%02x)", (MUINT32)eSensorDev);

    switch (eSensorDev)
    {
    default:
    case ESensorDev_Main:       //  Main Sensor
        static LscMgrDefault singleton_main(ESensorDev_Main, i4SensorIdx);
        _pLscMain = &singleton_main;
        LSC_LOG_END("ESensorDev_Main(%p), i4SensorIdx(%d)", _pLscMain, i4SensorIdx);
        return _pLscMain;
    case ESensorDev_MainSecond: //  Main Second Sensor
        static LscMgrDefault singleton_mainsecond(ESensorDev_MainSecond, i4SensorIdx);
        _pLscMain2 = &singleton_mainsecond;
        LSC_LOG_END("ESensorDev_MainSecond(%p), i4SensorIdx(%d)", _pLscMain2, i4SensorIdx);
        return _pLscMain2;
    case ESensorDev_Sub:        //  Sub Sensor
        static LscMgrDefault singleton_sub(ESensorDev_Sub, i4SensorIdx);
        _pLscSub = &singleton_sub;
        LSC_LOG_END("ESensorDev_Sub(%p), i4SensorIdx(%d)", _pLscSub, i4SensorIdx);
        return _pLscSub;
    case ESensorDev_SubSecond: //  Sub Second Sensor
        static LscMgrDefault singleton_subsecond(ESensorDev_SubSecond, i4SensorIdx);
        _pLscSub2 = &singleton_subsecond;
        LSC_LOG_END("ESensorDev_SubSecond(%p), i4SensorIdx(%d)", _pLscSub2, i4SensorIdx);
        return _pLscSub2;
    }
}

ILscMgr*
LscMgrDefault::
getInstance(ESensorDev_T eSensorDev)
{
    switch (eSensorDev)
    {
    default:
    case ESensorDev_Main:       //  Main Sensor
        return _pLscMain;
    case ESensorDev_MainSecond: //  Main Second Sensor
        return _pLscMain2;
    case ESensorDev_Sub:        //  Sub Sensor
        return _pLscSub;
    case ESensorDev_SubSecond:  //  Sub Second Sensor
        return _pLscSub2;
    }
}

MUINT32
LscMgrDefault::getSensorId()
{
    IHalSensorList*const pIHalSensorList = MAKE_HalSensorList();
    CAM_ULOG_ASSERT(Utils::ULog::MOD_ISP_MGR, pIHalSensorList != nullptr, "[%s]: make HAL sensor list fail!", __FUNCTION__);

    SensorStaticInfo rSensorStaticInfo;
    switch  ( m_eSensorDev )
    {
    case ESensorDev_Main:
        LSC_LOG("dev(%d)", m_eSensorDev);
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
        break;

    case ESensorDev_Sub:
        LSC_LOG("dev(%d)", m_eSensorDev);
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
        break;
    case ESensorDev_MainSecond:
        LSC_LOG("dev(%d)", m_eSensorDev);
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
        break;
    case ESensorDev_SubSecond:
        LSC_LOG("ln=%d %d", __LINE__, m_eSensorDev);
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB_2, &rSensorStaticInfo);
        break;
    default:    //  Shouldn't happen.
        LSC_LOG("dev(%d)", m_eSensorDev);
        LSC_ERR("Invalid sensor dev(%d)", m_eSensorDev);
        break;
        // return MFALSE;
    }

    LSC_LOG("sensorId(%d)", rSensorStaticInfo.sensorDevID);

    return rSensorStaticInfo.sensorDevID;
}

MBOOL
LscMgrDefault::setSensorTblFlow()
{
    if(m_i4SensorID==0x0576)
    {
        MUINT32 GridX=12;
        MUINT32 GridY=9;

        MUINT32 lenRChannel=GridX*GridY*sizeof(MUINT16);
        MUINT32 lenGbChannel=lenRChannel;
        MUINT32 lenBChannel=lenRChannel;
        MUINT32 lenGrChannel=lenRChannel;

        std::vector<MUINT16*> myvec;

        //malloc working buffer
        MUINT16* rChannel=(MUINT16*)malloc(lenRChannel);
        MUINT16* gbChannel=(MUINT16*)malloc(lenGbChannel);
        MUINT16* bChannel=(MUINT16*)malloc(lenBChannel);
        MUINT16* grChannel=(MUINT16*)malloc(lenGrChannel);

        myvec.push_back(rChannel);
        myvec.push_back(gbChannel);
        myvec.push_back(bChannel);
        myvec.push_back(grChannel);

        if (doShadingToSensorGain(GridX, GridY, myvec))
{

            LSC_LOG("R  Channel %d %d",  rChannel[0],  rChannel[1]);
            LSC_LOG("Gb Channel %d %d", gbChannel[0], gbChannel[1]);
            LSC_LOG("B  Channel %d %d",  bChannel[0],  bChannel[1]);
            LSC_LOG("Gr Channel %d %d", grChannel[0], grChannel[1]);

            //copyRawLscToNvram(CAL_DATA_LOAD);
            //m_e1to3Flag = E_LSC_123_WITH_MTK_OTP_OK;
            LSC_LOG("MTK OTP: Do doShadingToSensorGain ok");

            //update to sensor via aaa_sensor_mgr
            NS3Av3::AAASensorMgr::getInstance().setSensorShadingTbl(m_eSensorDev, myvec, lenGbChannel);
        }
    else
        {
            //m_e1to3Flag = E_LSC_123_WITH_MTK_OTP_ERR2;
            LSC_LOG("MTK OTP: Do doShadingToSensorGain fail");
        }

        free(grChannel);
        free(gbChannel);
        free(rChannel);
        free(bChannel);
    }

    return MTRUE;
}

MBOOL
LscMgrDefault::
sendGainTblToSensor(int GridX, int GridY)
{
    MUINT32 lengthEachChannel=GridX*GridY*sizeof(MUINT16);

    std::vector<MUINT16*> myvec;

    //malloc working buffer
    LSC_LOG("[%s] alloc gain table", __func__);
    MUINT16* m_u2rChannel =(MUINT16*)malloc(lengthEachChannel);
    MUINT16* m_u2gbChannel=(MUINT16*)malloc(lengthEachChannel);
    MUINT16* m_u2bChannel =(MUINT16*)malloc(lengthEachChannel);
    MUINT16* m_u2grChannel=(MUINT16*)malloc(lengthEachChannel);

    myvec.push_back(m_u2rChannel);
    myvec.push_back(m_u2gbChannel);
    myvec.push_back(m_u2bChannel);
    myvec.push_back(m_u2grChannel);


    if(doShadingToSensorGain(GridX, GridY, myvec))
    {
        LSC_LOG("R  Channel %d %d",  m_u2rChannel[0],  m_u2rChannel[1]);
        LSC_LOG("Gb Channel %d %d", m_u2gbChannel[0], m_u2gbChannel[1]);
        LSC_LOG("B  Channel %d %d",  m_u2bChannel[0],  m_u2bChannel[1]);
        LSC_LOG("Gr Channel %d %d", m_u2grChannel[0], m_u2grChannel[1]);

        LSC_LOG("[%s]: Do doShadingToSensorGain ok", __func__);

    } else
        LSC_LOG("[%s]: Do doShadingToSensorGain fail", __func__);

    //wait for sensor power on
    #define MAX_POLLING_COUNT 100

    AAA_TRACE_D("LSC EEPROM");

    MINT32 i = MAX_POLLING_COUNT;
    MUINT32 u4PowerOn = 0;
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    CAM_ULOG_ASSERT(Utils::ULog::MOD_ISP_MGR, pIHalSensorList != nullptr, "[%s]: make HAL sensor list fail!", __FUNCTION__);
    IHalSensor* pIHalSensor = pIHalSensorList->createSensor(LOG_TAG, m_i4SensorIdx);
    //LSC_LOG("waiting for sensor(%d) power on", m_eSensorDev);
    pIHalSensor->sendCommand(m_eSensorDev, NSCam::SENSOR_CMD_GET_SENSOR_POWER_ON_STETE, (MUINTPTR)&u4PowerOn, 0, 0);
    while (0 == (m_eSensorDev & u4PowerOn) && i != 0)
    {
        usleep(1000 * 10);
        i --;
        pIHalSensor->sendCommand(m_eSensorDev, NSCam::SENSOR_CMD_GET_SENSOR_POWER_ON_STETE, (MUINTPTR)&u4PowerOn, 0, 0);
    }
    LSC_LOG("[%s] Sensor(%d) power on done, count(%d)", __func__, m_eSensorDev, (MAX_POLLING_COUNT - i));

    //update to sensor via aaa_sensor_mgr
    if(m_eSensorDev & u4PowerOn)
        NS3Av3::AAASensorMgr::getInstance().setSensorShadingTbl(m_eSensorDev, myvec, lengthEachChannel);

    pIHalSensor->destroyInstance(LOG_TAG);

    AAA_TRACE_END_D;

    free(m_u2rChannel);
    free(m_u2gbChannel);
    free(m_u2bChannel);
    free(m_u2grChannel);

    return MTRUE;
}

MVOID*
LscMgrDefault::
threadSendSensor(void* arg)
{
    LscMgrDefault* _this = reinterpret_cast<LscMgrDefault*>(arg);

    _this->doThreadSendSensor();

    pthread_exit(NULL);

    return NULL;
}

MVOID
LscMgrDefault::
doThreadSendSensor()
{
    //do something
    LSC_LOG("[LSC send data thread] %d +", pthread_self());

    setSensorTblFlow();

    LSC_LOG("[LSC send data thread] %d -", pthread_self());
}

