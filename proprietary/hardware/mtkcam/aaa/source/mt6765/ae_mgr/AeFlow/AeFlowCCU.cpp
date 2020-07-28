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
#define LOG_TAG "AeFlowCCU"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#endif

#include <stdlib.h>
#include "AeFlowCCU.h"
#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>
#include <ae_param.h>
#include <aaa_error_code.h>
#include <isp_tuning_mgr.h>
#include <aaa_scheduler.h>


#define AE_FLOW_LOG(fmt, arg...) \
    do { \
        if (0) { \
            CAM_LOGE(fmt, ##arg); \
        } else { \
            CAM_LOGD(fmt, ##arg); \
        } \
    }while(0)

#define AE_FLOW_LOG_IF(cond, fmt, arg...) \
    do { \
        if (0) { \
            CAM_LOGE_IF(cond, fmt, ##arg); \
        } else { \
            CAM_LOGD_IF(cond, fmt, ##arg); \
        } \
    }while(0)

using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSCcuIf;

MUINT32 Log2x1000[LumLog2x1000_TABLE_SIZE] = {
    0,    0,   1000,    1585,    2000,    2322,    2585,    2807,
    3000,   3170,   3322,   3459,   3585,   3700,   3807,   3907,
    4000,   4087,   4170,   4248,   4322,   4392,   4459,   4524,
    4585,   4644,   4700,   4755,   4807,   4858,   4907,   4954,
    5000,   5044,   5087,   5129,   5170,   5209,   5248,   5285,
    5322,   5358,   5392,   5426,   5459,   5492,   5524,   5555,
    5585,   5615,   5644,   5672,   5700,   5728,   5755,   5781,
    5807,   5833,   5858,   5883,   5907,   5931,   5954,   5977,
    6000,   6022,   6044,   6066,   6087,   6109,   6129,   6150,
    6170,   6190,   6209,   6229,   6248,   6267,   6285,   6304,
    6322,   6340,   6358,   6375,   6392,   6409,   6426,   6443,
    6459,   6476,   6492,   6508,   6524,   6539,   6555,   6570,
    6585,   6600,   6615,   6629,   6644,   6658,   6672,   6687,
    6700,   6714,   6728,   6741,   6755,   6768,   6781,   6794,
    6807,   6820,   6833,   6845,   6858,   6870,   6883,   6895,
    6907,   6919,   6931,   6943,   6954,   6966,   6977,   6989,
    7000,   7011,   7022,   7033,   7044,   7055,   7066,   7077,
    7087,   7098,   7109,   7119,   7129,   7140,   7150,   7160,
    7170,   7180,   7190,   7200,   7209,   7219,   7229,   7238,
    7248,   7257,   7267,   7276,   7285,   7295,   7304,   7313,
    7322,   7331,   7340,   7349,   7358,   7366,   7375,   7384,
    7392,   7401,   7409,   7418,   7426,   7435,   7443,   7451,
    7459,   7468,   7476,   7484,   7492,   7500,   7508,   7516,
    7524,   7531,   7539,   7547,   7555,   7562,   7570,   7577,
    7585,   7592,   7600,   7607,   7615,   7622,   7629,   7637,
    7644,   7651,   7658,   7665,   7672,   7679,   7687,   7693,
    7700,   7707,   7714,   7721,   7728,   7735,   7741,   7748,
    7755,   7762,   7768,   7775,   7781,   7788,   7794,   7801,
    7807,   7814,   7820,   7827,   7833,   7839,   7845,   7852,
    7858,   7864,   7870,   7877,   7883,   7889,   7895,   7901,
    7907,   7913,   7919,   7925,   7931,   7937,   7943,   7948,
    7954,   7960,   7966,   7972,   7977,   7983,   7989,   7994,
    8000,   8006,   8011,   8017,   8022,   8028,   8033,   8039,
    8044,   8050,   8055,   8061,   8066,   8071,   8077,   8082,
    8087,   8093,   8098,   8103,   8109,   8114,   8119,   8124,
    8129,   8134,   8140,   8145,   8150,   8155,   8160,   8165,
    8170,   8175,   8180,   8185,   8190,   8195,   8200,   8205,
    8209,   8214,   8219,   8224,   8229,   8234,   8238,   8243,
    8248,   8253,   8257,   8262,   8267,   8271,   8276,   8281,
    8285,   8290,   8295,   8299,   8304,   8308,   8313,   8317,
    8322,   8326,   8331,   8335,   8340,   8344,   8349,   8353,
    8358,   8362,   8366,   8371,   8375,   8379,   8384,   8388,
    8392,   8397,   8401,   8405,   8409,   8414,   8418,   8422,
    8426,   8430,   8435,   8439,   8443,   8447,   8451,   8455,
    8459,   8464,   8468,   8472,   8476,   8480,   8484,   8488,
    8492,   8496,   8500,   8504,   8508,   8512,   8516,   8520,
    8524,   8527,   8531,   8535,   8539,   8543,   8547,   8551,
    8555,   8558,   8562,   8566,   8570,   8574,   8577,   8581,
    8585,   8589,   8592,   8596,   8600,   8604,   8607,   8611,
    8615,   8618,   8622,   8626,   8629,   8633,   8637,   8640,
    8644,   8647,   8651,   8655,   8658,   8662,   8665,   8669,
    8672,   8676,   8679,   8683,   8687,   8690,   8693,   8697,
    8700,   8704,   8707,   8711,   8714,   8718,   8721,   8725,
    8728,   8731,   8735,   8738,   8741,   8745,   8748,   8752,
    8755,   8758,   8762,   8765,   8768,   8771,   8775,   8778,
    8781,   8785,   8788,   8791,   8794,   8798,   8801,   8804,
    8807,   8811,   8814,   8817,   8820,   8823,   8827,   8830,
    8833,   8836,   8839,   8842,   8845,   8849,   8852,   8855,
    8858,   8861,   8864,   8867,   8870,   8873,   8877,   8880,
    8883,   8886,   8889,   8892,   8895,   8898,   8901,   8904,
    8907,   8910,   8913,   8916,   8919,   8922,   8925,   8928,
    8931,   8934,   8937,   8940,   8943,   8945,   8948,   8951,
    8954,   8957,   8960,   8963,   8966,   8969,   8972,   8974,
    8977,   8980,   8983,   8986,   8989,   8992,   8994,   8997,
    9000
};

static std::array<MY_INST_AEFLOW, SENSOR_IDX_MAX> gMultiton;

IAeFlow*
AeFlowCCU::
getInstance(ESensorDev_T sensor)
{
    AE_FLOW_LOG("eSensorDev(0x%02x)", (MUINT32)sensor);

    int i4SensorIdx = NS3Av3::mapSensorDevToIdx(sensor);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
    }

    MY_INST_AEFLOW& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<AeFlowCCU>(sensor);
    } );

    return rSingleton.instance.get();
}


AeFlowCCU::
AeFlowCCU(ESensorDev_T sensor)
    : m_eSensorDev(sensor)
    , m_i4SensorIdx(0)
    , m_eSensorMode(ESensorMode_Preview)
    , m_pICcuAe(NULL)
    , m_bIsCCUStart(MFALSE)
    , m_bIsCCUWorking(MFALSE)
    , m_bIsCCUAEWorking(MFALSE)
    , m_bIsCCUPaused(MFALSE)
    , m_bCCUAEFlag(MTRUE)
    , m_bCCUIsSensorSupported(MFALSE)
    , m_bEnableFlickerFPSstate(MFALSE)
    , m_bControlCCUInfochange(MFALSE)
    , m_i4OBCTableidx(0)
    , m_bSetCCUFrameRate(MFALSE)
    , m_3ALogEnable(MFALSE)
{

    AE_FLOW_LOG("Enter AeFlowCCU DeviceId:%d",(MUINT32)m_eSensorDev);
}

MVOID
AeFlowCCU::
init()
{
    AE_FLOW_LOG( "[%s():%d]\n", __FUNCTION__, (MUINT32)m_eSensorDev);
    //m_pICcuAe = ICcu::createInstance((const MINT8 *)"AeFlow", ICcu::AEAWB, ICcu::AEAWBCTRL);
    m_pICcuAe = ICcuCtrlAe::createInstance();
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.aaa.pvlog.enable", value, "0");
    m_3ALogEnable = atoi(value);
}

MVOID
AeFlowCCU::
uninit()
{
    AE_FLOW_LOG( "[%s():%d]\n", __FUNCTION__, (MUINT32)m_eSensorDev);

    if (m_pICcuAe != NULL)
    {
        m_pICcuAe->destroyInstance();
        m_pICcuAe = NULL;
    }
}

MVOID
AeFlowCCU::
start(MVOID* pAEInitInput)
{
    static MBOOL boot_failed = MFALSE;
    if(boot_failed)
    {
        AE_FLOW_LOG("ICCU boot failed before, skip starting ccu\n");
        return;
    }


    if(!m_bIsCCUStart){
        AE_FLOW_INPUT_INFO_T* pinitInfo = ((AE_FLOW_INPUT_INFO_T*) pAEInitInput);
        m_i4SensorIdx = pinitInfo->i4SensorIdx;
        m_eSensorMode = pinitInfo->eSensorMode;
        AE_FLOW_LOG( "[%s()]:%d/%d/%d\n", __FUNCTION__, (MUINT32)m_eSensorDev, m_i4SensorIdx, m_eSensorMode);

        SENSOR_INFO_IN_T rSensorinfoInput;
        SENSOR_INFO_OUT_T rSensorinfoOutput;
        CCU_AE_INITI_PARAM_T rAEInitparam;
        CCU_AE_CONTROL_CFG_T rAEControlcfg;
        AE_CORE_INIT rAEcoreInit;
        memset(&rSensorinfoInput, 0, sizeof(SENSOR_INFO_IN_T));
        memset(&rSensorinfoOutput, 0, sizeof(SENSOR_INFO_OUT_T));
        memset(&rAEInitparam, 0, sizeof(CCU_AE_INITI_PARAM_T));
        memset(&rAEControlcfg, 0, sizeof(CCU_AE_CONTROL_CFG_T));
        memset(&rAEcoreInit, 0, sizeof(AE_CORE_INIT));


        // init CCU
        AE_FLOW_LOG("CCU initial start, sensor id: %x\n", m_i4SensorIdx);
        if(m_pICcuAe->init(m_i4SensorIdx, m_eSensorDev) != CCU_CTRL_SUCCEED)
        {
            AE_FLOW_LOG("m_pICcuAe->init() fail");
            m_bCCUIsSensorSupported = MFALSE;
            return;
        }

        AE_FLOW_LOG("CCU initial & boot done\n");


        // start Sensor and I2C
        //rSensorinfoInput.u32SensorId = (MUINT32)m_eSensorDev;
        //rSensorinfoInput.u32SensorId = getSensorId(m_i4SensorIdx, m_eSensorDev);

        switch(m_eSensorMode) {
            case ESensorMode_Preview:
                rSensorinfoInput.eScenario = SENSOR_SCENARIO_PREVIEW;
                break;
            case ESensorMode_Capture:
                rSensorinfoInput.eScenario = SENSOR_SCENARIO_CAPTURE;
                break;
            case ESensorMode_Video:
                rSensorinfoInput.eScenario = SENSOR_SCENARIO_NORMAL_VIDEO;
                break;
            case ESensorMode_SlimVideo1:
                rSensorinfoInput.eScenario = SENSOR_SCENARIO_HIGH_SPEED_VIDEO;
                break;
            case ESensorMode_SlimVideo2:
                rSensorinfoInput.eScenario = SENSOR_SCENARIO_SLIM_VIDEO;
                break;
            default: // Shouldn't happen.
                rSensorinfoInput.eScenario = SENSOR_SCENARIO_PREVIEW;
                AE_FLOW_LOG("CCU - Invalid sensor mode: %d", m_eSensorMode);
        }

        //AE_FLOW_LOG("rSensorinfoInput.eScenario : %d",rSensorinfoInput.eScenario);
        //AE_FLOW_LOG("rSensorinfoInput.eWhichSensor : %d",rSensorinfoInput.eWhichSensor);
        //AE_FLOW_LOG("rSensorinfoInput.pu8BufferVAddr : %d",rSensorinfoInput.pu8BufferVAddr);
        //AE_FLOW_LOG("rSensorinfoInput.sensorI2cSlaveAddr : %d",rSensorinfoInput.sensorI2cSlaveAddr);
        //AE_FLOW_LOG("rSensorinfoInput.u16BufferLen : %d",rSensorinfoInput.u16BufferLen);
        //AE_FLOW_LOG("rSensorinfoInput.u16FPS : %d",rSensorinfoInput.u16FPS);


        if ( CCU_CTRL_SUCCEED != m_pICcuAe->ccuControl(MSG_TO_CCU_SENSOR_INIT, &rSensorinfoInput, &rSensorinfoOutput))
        {
            AE_FLOW_LOG("ccu_sensor_initialize fail\n");
            m_bCCUIsSensorSupported = MFALSE;
            return;
        }
        else
        {
            AE_FLOW_LOG("ccu_sensor_initialize success\n");
            if(!rSensorinfoOutput.u8SupportedByCCU)
            {
                AE_FLOW_LOG("this sensor is not supported by CCU\n");
                m_bCCUIsSensorSupported = MFALSE;
                return;
            }
        }
        // init ccu ae
        rAEControlcfg.hdr_stat_en = (MBOOL)(pinitInfo->pAEInitInput->rAEPARAM.strAEParasetting.bEnableHDRLSB>0);
        rAEControlcfg.ae_overcnt_en = pinitInfo->pAEInitInput->rAEPARAM.strAEParasetting.bEnableAEOVERCNTconfig;
        rAEControlcfg.tsf_stat_en = pinitInfo->pAEInitInput->rAEPARAM.strAEParasetting.bEnableTSFSTATconfig;
        rAEControlcfg.bits_num = (pinitInfo->u4AAOmode == 1)? 14:12;
        rAEControlcfg.ae_footprint_hum = pinitInfo->pAEInitInput->rAEPARAM.strAEParasetting.u4AEWinodwNumX;
        rAEControlcfg.ae_footprint_vum = pinitInfo->pAEInitInput->rAEPARAM.strAEParasetting.u4AEWinodwNumY;

        rAEcoreInit.EndBankIdx = 0;
        rAEcoreInit.m_i4AEMaxBlockWidth = pinitInfo->pAEInitInput->rAEPARAM.strAEParasetting.u4AEWinodwNumX;
        rAEcoreInit.m_i4AEMaxBlockHeight = pinitInfo->pAEInitInput->rAEPARAM.strAEParasetting.u4AEWinodwNumY;
        rAEcoreInit.uTgBlockNumX = AE_TG_BLOCK_NO_X;
        rAEcoreInit.uTgBlockNumY = AE_TG_BLOCK_NO_Y;
        rAEcoreInit.m_u4IndexMax = pinitInfo->u4IndexFMax;
        rAEcoreInit.m_u4IndexMin = pinitInfo->u4IndexFMin;
        rAEcoreInit.bSpeedupEscOverExp  = pinitInfo->pAEInitInput->rAEPARAM.bSpeedupEscOverExp;
        rAEcoreInit.bSpeedupEscUnderExp = pinitInfo->pAEInitInput->rAEPARAM.bSpeedupEscUnderExp;
        rAEcoreInit.pAETgTuningParam = *(pinitInfo->pAEInitInput->rAEPARAM.pAETgTuningParam);
        //rAEcoreInit.pTgWeightTbl = pinitInfo->pAEInitInput->rAEPARAM.pTgWeightTbl;
        memcpy(rAEcoreInit.pTgWeightTbl, pinitInfo->pAEInitInput->rAEPARAM.pTgWeightTbl, 9*12*sizeof(MUINT32));
        rAEcoreInit.pTgEVCompJumpRatioOverExp = pinitInfo->pAEInitInput->rAEPARAM.pTgEVCompJumpRatioOverExp;
        rAEcoreInit.pTgEVCompJumpRatioUnderExp= pinitInfo->pAEInitInput->rAEPARAM.pTgEVCompJumpRatioUnderExp;
        rAEcoreInit.LumLog2x1000 = Log2x1000;
        rAEcoreInit.SzLumLog2x1000 = sizeof(Log2x1000);
        rAEcoreInit.pCurrentTable = (strEvPline* )pinitInfo->pPreviewTableF;
        rAEcoreInit.i4MaxBV = pinitInfo->pPreviewTableCurrent->i4MaxBV;
        rAEcoreInit.i4BVOffset = pinitInfo->pAEInitInput->rAENVRAM.rCCTConfig.i4BVOffset;
        rAEcoreInit.u4MiniISOGain = pinitInfo->pAEInitInput->rAENVRAM.rDevicesInfo.u4MiniISOGain;
        rAEcoreInit.iMiniBVValue = pinitInfo->pAEInitInput->rAEPARAM.strAEParasetting.iMiniBVValue;
        rAEcoreInit.EVBase = pinitInfo->u4FinerEVIdxBase;
        rAEcoreInit.u2IspNvIsoIntvl_Length = pinitInfo->pISPNvramISOInterval->u2Length;
        memcpy(&rAEcoreInit.OBC_Table[0], &(pinitInfo->pISPNvramOBC_Table[0]), 4*sizeof(CCU_ISP_NVRAM_OBC_T));
        rAEInitparam.control_cfg = rAEControlcfg;
        rAEInitparam.algo_init_param = rAEcoreInit;
        // Debug
        int cnt = (rAEcoreInit.SzLumLog2x1000 / 4);
        AE_FLOW_LOG("[startCCU] IndexMax/BlockNumX/BlockNumY/footprint hum/vum/OverExp[70]/UnderExp[3]/Size/[0]/[cnt-1] : %x/%x/%x/%x/%x/%x/%x/%x/%x/%x \n",
            rAEcoreInit.m_u4IndexMax,rAEcoreInit.uTgBlockNumX,rAEcoreInit.uTgBlockNumY,
            rAEControlcfg.ae_footprint_hum,rAEControlcfg.ae_footprint_vum,
            rAEcoreInit.pTgEVCompJumpRatioOverExp[70],rAEcoreInit.pTgEVCompJumpRatioUnderExp[3],
            rAEcoreInit.SzLumLog2x1000, rAEcoreInit.LumLog2x1000[0], rAEcoreInit.LumLog2x1000[cnt-1]);
        AE_FLOW_LOG("[%s()] m_i4SensorDev:%d i4OBCtableidx: %d [0][1][2][3](offset/gain) = [%d/%d][%d/%d][%d/%d][%d/%d] \n",
            __FUNCTION__, m_eSensorDev, m_i4OBCTableidx,
            rAEcoreInit.OBC_Table[0].offst0.val, rAEcoreInit.OBC_Table[0].gain0.val,
            rAEcoreInit.OBC_Table[1].offst0.val, rAEcoreInit.OBC_Table[1].gain0.val,
            rAEcoreInit.OBC_Table[2].offst0.val, rAEcoreInit.OBC_Table[2].gain0.val,
            rAEcoreInit.OBC_Table[3].offst0.val, rAEcoreInit.OBC_Table[3].gain0.val);


        //AE_LOG("rAEcoreInit.pCurrentTable addr: %p\n", &rAEInitparam.algo_init_param.pCurrentTable);
        if (CCU_CTRL_SUCCEED != m_pICcuAe->ccuControl(MSG_TO_CCU_AE_INIT, &rAEInitparam, NULL))
        {
            AE_FLOW_LOG("ccu_ae_initialize fail\n");
            m_bCCUIsSensorSupported = MFALSE;
            return;
        }
        AE_FLOW_LOG("ccu_ae_initialize success\n");

        /*int debugInfo;
        while(1)
        {
            debugInfo = m_pICcu->readInfoReg(20);
            AE_FLOW_LOG( "[%s()] bankdone debug: %x\n", __FUNCTION__, debugInfo);
            usleep(1000);
        }*/
        m_bSetCCUFrameRate = MTRUE;
        m_bControlCCUInfochange = MTRUE;
        m_bIsCCUStart = MTRUE;
        m_bIsCCUPaused = MFALSE;
        m_bCCUIsSensorSupported = MTRUE;
    }
}

MVOID
AeFlowCCU::
stop()
{
    //if CCU is under pause state, shutdown & uninit flow should also take place
    if (m_bIsCCUStart /*|| m_bIsCCUPaused*/){
        AE_FLOW_LOG( "[%s()]\n", __FUNCTION__);

        if (CCU_CTRL_SUCCEED != m_pICcuAe->ccuControl(MSG_TO_CCU_AE_STOP, NULL, NULL))
        {
            AE_FLOW_LOG("MSG_TO_CCU_AE_STOP fail\n");
            return;
        }
        m_bIsCCUStart = MFALSE;
        m_bIsCCUAEWorking = MFALSE;
        //m_bIsCCUPaused = MFALSE;
    }
}

MVOID
AeFlowCCU::
pause()
{
    if (m_bIsCCUStart){
        AE_FLOW_LOG( "[%s()]\n", __FUNCTION__);
        this->stop();
        m_bIsCCUStart = MFALSE;
        m_bIsCCUAEWorking = MFALSE;
        //m_bIsCCUPaused = MTRUE;
    }

}

MBOOL
AeFlowCCU::
queryStatus(MUINT32 index)
{
    E_AE_FLOW_CCU_STATUS_T eStatus = (E_AE_FLOW_CCU_STATUS_T) index;
    switch (eStatus)
    {
        case E_AE_FLOW_CCU_WORKING:
            return IsCCUWorking();
        case E_AE_FLOW_CCU_AE_WORKING:
            return IsCCUAEWorking();
        case E_AE_FLOW_CCU_SENSOR_SUPPORTED:
            return m_bCCUIsSensorSupported;
        default:
            AE_FLOW_LOG("[%s():%d] Warning: index = %d \n", __FUNCTION__, m_eSensorDev, index);
            return MFALSE;
    }
}


MVOID
AeFlowCCU::
controltrigger(MUINT32 index, MBOOL btrigger)
{
    E_AE_FLOW_CCU_TRIGER_T etrigger = (E_AE_FLOW_CCU_TRIGER_T) index;
    switch (etrigger)
    {
        case E_AE_FLOW_CCU_TRIGGER_APINFO:
            m_bControlCCUInfochange = MTRUE;
            break;
        case E_AE_FLOW_CCU_AE_WORKING:
            m_bSetCCUFrameRate = MTRUE;
            break;
        case E_AE_FLOW_CCU_TRIGGER_FLICKER_FPS:
            m_bEnableFlickerFPSstate = btrigger;
            break;
        default:
            AE_FLOW_LOG("[%s():%d] Warning: index = %d \n", __FUNCTION__, m_eSensorDev, index);
    }
}
MVOID
AeFlowCCU::
startCCUAE()
{
    if(!m_bIsCCUAEWorking && IsCCUWorking()){

        AE_FLOW_LOG( "[%s()]\n", __FUNCTION__);
        m_pICcuAe->ccuControl(MSG_TO_CCU_AE_START, NULL, NULL);

        m_bIsCCUAEWorking = MTRUE;
    }
}

MVOID
AeFlowCCU::
controlCCU(AE_FLOW_CONTROL_INFO_T* pcontrolInfo)
{
    ccu_skip_algo_data rSkipAlgoData;
    if(IsCCUWorking() && IsCCUAEWorking()){
        // Fill Control Info data - add control member to avoid always memory copy
        if (m_bControlCCUInfochange)
        {
            ccu_ae_onchange_data rOnchangeData;
            memset(&rOnchangeData, 0, sizeof(ccu_ae_onchange_data));
            rOnchangeData.m_u4IndexMax = pcontrolInfo->u4IndexFMax;
            rOnchangeData.m_u4IndexMin = pcontrolInfo->u4IndexFMin;
            rOnchangeData.auto_flicker_en = m_bEnableFlickerFPSstate;
            rOnchangeData.pCurrentTable = (strEvPline*)pcontrolInfo->pPreviewTableF;
            /* Eric Here
            memcpy(&rOnchangeData.OBC_Table[0], &m_rISPNvramOBC_Table[0], 4*sizeof(CCU_ISP_NVRAM_OBC_T));
            */
            m_pICcuAe->ccuControl(MSG_TO_CCU_SET_AP_AE_ONCHANGE_DATA, &rOnchangeData, NULL);
            m_bControlCCUInfochange = MFALSE;
            AE_FLOW_LOG( "[%s()] MaxIdx/MinIdx/FlickerState/pPlineTableAddress/pPlinetable->eID %d/%d/%d/%d \n",
                __FUNCTION__,
                rOnchangeData.m_u4IndexMax,
                rOnchangeData.m_u4IndexMin,
                rOnchangeData.auto_flicker_en,
                //rOnchangeData.pCurrentTable,
                pcontrolInfo->pPreviewTableCurrent->eID);
        }
        // Fill frame sync data
        ccu_ae_frame_sync_data rCcuFrameSyncData;
        memset(&rCcuFrameSyncData, 0, sizeof(ccu_ae_frame_sync_data));
        rCcuFrameSyncData.is_zooming = pcontrolInfo->bZoomChange;
        rCcuFrameSyncData.magic_num = pcontrolInfo->u4HwMagicNum;
        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()]is_zooming:0x%x\n", __FUNCTION__, rCcuFrameSyncData.is_zooming);
        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()]magic_num:0x%x\n", __FUNCTION__, rCcuFrameSyncData.magic_num);

        m_pICcuAe->ccuControl(MSG_TO_CCU_SET_AP_AE_FRAME_SYNC_DATA, &rCcuFrameSyncData, NULL);

        // Fill APAE data
        AE_FLOW_LOG_IF(m_3ALogEnable,"[%s()]CCU_ZoomRatio: %d\n",__FUNCTION__,pcontrolInfo->i4ZoomRatio);
        ccu_ae_roi rAPAERoi;
        memset(&rAPAERoi, 0, sizeof(ccu_ae_roi));
        rAPAERoi.AEWinBlock_u4XHi  = 119;
        rAPAERoi.AEWinBlock_u4XLow = pcontrolInfo->i4ZoomRatio;
        rAPAERoi.AEWinBlock_u4YHi  =  89;
        rAPAERoi.AEWinBlock_u4YLow =   0;

        m_pICcuAe->ccuControl(MSG_TO_CCU_SET_AE_EV, &(pcontrolInfo->eAEEVcomp), NULL);

        if (pcontrolInfo->bskipCCUAlgo){
            rSkipAlgoData.do_skip = MTRUE;
        }else{
            rSkipAlgoData.do_skip = MFALSE;
        }
        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()]rSkipAlgoData.do_skip:0x%x\n", __FUNCTION__, rSkipAlgoData.do_skip);
		    m_pICcuAe->ccuControl(MSG_TO_CCU_SET_SKIP_ALGO, &rSkipAlgoData, NULL);

        if(m_bSetCCUFrameRate){
            ccu_max_framerate_data max_framerate_data;
	    max_framerate_data.framerate = (MUINT16)pcontrolInfo->i4AEMaxFps;
    	    max_framerate_data.min_framelength_en = MTRUE;
            //AE_LOG( "[%s()]m_i4AEMaxFps:0x%x\n", __FUNCTION__, max_framerate_data.framerate);
            m_pICcuAe->ccuControl(MSG_TO_CCU_SET_MAX_FRAMERATE, &max_framerate_data, NULL);
	    m_bSetCCUFrameRate = MFALSE;
        }
        //AE_FLOW_LOG_IF(m_3ALogEnable,"[%s()]rAPAERoi.AEWinBlock_u4XHi:0x%x\n", __FUNCTION__, rAPAERoi.AEWinBlock_u4XHi);
        //AE_FLOW_LOG_IF(m_3ALogEnable,"[%s()]rAPAERoi.AEWinBlock_u4XLow:0x%x\n", __FUNCTION__, rAPAERoi.AEWinBlock_u4XLow);
        //AE_FLOW_LOG_IF(m_3ALogEnable,"[%s()]rAPAERoi.AEWinBlock_u4YHi:0x%x\n", __FUNCTION__, rAPAERoi.AEWinBlock_u4YHi);
        //AE_FLOW_LOG_IF(m_3ALogEnable,"[%s()]rAPAERoi.AEWinBlock_u4YLow:0x%x\n", __FUNCTION__, rAPAERoi.AEWinBlock_u4YLow);
        m_pICcuAe->ccuControl(MSG_TO_CCU_SET_AE_ROI, &rAPAERoi, NULL);

    }

}

MVOID
AeFlowCCU::
getCCUResult(AE_CORE_MAIN_OUT* poutput)
{
    if(IsCCUWorking() && IsCCUAEWorking()){

    m_pICcuAe->ccuControl(MSG_TO_CCU_GET_CCU_OUTPUT, NULL, poutput);

    AE_FLOW_LOG_IF(m_3ALogEnable,"[%s()] EffectiveIndex/Index/RealBV/BVx1000/BV/AoeBV/EV/Cpu/Ccu: %d/%d/%d/%d/%d/%d/%d/%d/%d\n", __FUNCTION__,
        poutput->m_u4EffectiveIndex, poutput->m_u4Index, poutput->m_i4RealBV, poutput->m_i4RealBVx1000,
        poutput->m_i4BV, poutput->m_i4AoeCompBV, poutput->m_i4EV, poutput->enumCpuAction, poutput->enumCcuAction);
    }

}


MVOID
AeFlowCCU::
setupParamPvAE(MVOID* input, MVOID* output)
{

    startCCUAE();
    controlCCU((AE_FLOW_CONTROL_INFO_T*) input);
    getCCUResult((AE_CORE_MAIN_OUT*) output);

    AE_FLOW_LOG_IF(m_3ALogEnable,"[%s()]\n", __FUNCTION__);
}

MVOID
AeFlowCCU::
update(AE_CORE_CTRL_CPU_EXP_INFO* input)
{
    if(IsCCUWorking()&& IsCCUAEWorking()){
        m_pICcuAe->ccuControl(MSG_TO_CCU_SET_AP_AE_OUTPUT, input, NULL);
    }
}

MUINT
AeFlowCCU::
getSensorId(MUINT m_i4SensorIdx, MUINT m_eSensorDev)
{
    return m_i4SensorIdx;
}

MBOOL
AeFlowCCU::
IsCCUWorking()
{
    MBOOL res = MFALSE;
    if (m_bIsCCUStart){
        res = MTRUE;
    }
    m_bIsCCUWorking = res;
    return res;
}

MBOOL
AeFlowCCU::
IsCCUAEWorking()
{
    return m_bIsCCUAEWorking;
}


AeFlowCCU::
~AeFlowCCU()
{

    AE_FLOW_LOG("Delete ~AeFlowCCU - DeviceId:%d",(MUINT32)m_eSensorDev);

}


