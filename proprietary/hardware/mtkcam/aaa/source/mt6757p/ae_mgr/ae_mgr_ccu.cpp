/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
#define LOG_TAG "ae_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
//#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <ae_param.h>
//#include <CamDefs.h>
#include <faces.h>
#include <isp_tuning.h>
#include <isp_mgr.h>
#include <isp_tuning_mgr.h>
#include <ae_algo_if.h>
//
#include <ae_tuning_custom.h>
#include <aaa_sensor_mgr.h>
#include "camera_custom_hdr.h"
#include "camera_custom_ivhdr.h"
#include "camera_custom_mvhdr.h"
#include <kd_camera_feature.h>
#include <mtkcam/drv/IHalSensor.h>
#include "ae_mgr_if.h"
#include "ae_mgr.h"
#include <aaa_scheduler.h>
#include <aaa_common_custom.h>
#include "aaa_sensor_buf_mgr.h"
#define AE_MGR_INCLUDING_CCU
#include "ccu_ext_interface/ccu_ext_interface.h"

using namespace NS3A;
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSIspTuningv3;
using namespace NSCam;
using namespace NSCcuIf::NSCcuDrv;

MUINT32 LumLog2x1000[] = {
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

#define AE_LOG(fmt, arg...) \
    do { \
        if (m_i4DgbLogE) { \
            CAM_LOGE(fmt, ##arg); \
        } else { \
            CAM_LOGD(fmt, ##arg); \
        } \
    }while(0)

#define AE_LOG_IF(cond, ...) \
    do { \
        if (m_i4DgbLogE) { \
            CAM_LOGE_IF(cond, __VA_ARGS__); \
        } else { \
            CAM_LOGD_IF(cond, __VA_ARGS__); \
        } \
    }while(0)


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::initCCU()
{
    AE_LOG( "[%s()]\n", __FUNCTION__);

    m_pICcu = ICcu::createInstance((const MINT8 *)"AeMgr", ICcu::AEAWB, ICcu::AEAWBCTRL);

    return S_AE_OK;

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::uninitCCU()
{
    AE_LOG( "[%s()]\n", __FUNCTION__);

    if (m_pICcu != NULL)
    {
        m_pICcu->destroyInstance();
        m_pICcu = NULL;
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include <mtkcam/drv/IHalSensor.h>
static MUINT getSensorId(MUINT m_i4SensorIdx, MUINT m_eSensorDev)
{
    MUINT sensorId;

    // Query TG info
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    //IHalSensor* pIHalSensor = pIHalSensorList->createSensor("ae_mgr", m_i4SensorIdx);
    SensorStaticInfo sInfo;

    switch(m_eSensorDev) {
        case ESensorDev_Main:
            pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &sInfo);
            break;
        case ESensorDev_Sub:
            pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &sInfo);
            break;
        case ESensorDev_MainSecond:
            pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &sInfo);
            break;
        case ESensorDev_SubSecond:
            pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB_2, &sInfo);
            break;
        default:    //  Shouldn't happen.
            CAM_LOGE("Invalid sensor device: %d", m_eSensorDev);
    }

    sensorId = sInfo.sensorDevID;

    /*if(pIHalSensor) {
        pIHalSensor->destroyInstance("ae_mgr");
    }*/

    return sensorId;
}

MRESULT AeMgr::startCCU()
{
    static MBOOL boot_failed = MFALSE;
    MBOOL res = S_AE_OK;
    if(boot_failed)
    {
        AE_LOG("ICCU boot failed before, skip starting ccu\n");
        return E_AE_SENSOR_INFO;
    }

    if(!m_bIsCCUStart){

        AE_LOG( "[%s()]\n", __FUNCTION__);

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
        AE_LOG("CCU initial start, sensor id: %x\n", getSensorId(m_i4SensorIdx, m_eSensorDev));
        if (MFALSE == m_pICcu->init()) {
            AE_LOG("ICCU init fail");
            return E_AE_SENSOR_INFO;
        }

        if (m_eSensorDev & ESensorDev_Main){
            rSensorinfoInput.eWhichSensor = WHICH_SENSOR_MAIN;
        }else if (m_eSensorDev & ESensorDev_Sub){
            rSensorinfoInput.eWhichSensor = WHICH_SENSOR_SUB;
        }else if (m_eSensorDev & ESensorDev_MainSecond){
            rSensorinfoInput.eWhichSensor = WHICH_SENSOR_MAIN_2;
        }else{
            AE_LOG("[CCU sensor start failed] m_eSensorDev = %d\n",m_eSensorDev);
            m_pICcu->uninit();
            return E_AE_SENSOR_INFO;
        }

        if(m_pICcu->ccu_check_sensor_support(rSensorinfoInput.eWhichSensor) != MTRUE) {
            AE_LOG("CCU not support this sensor\n");
            m_pICcu->uninit();
            return E_AE_SENSOR_INFO;
        }

        // boot CCU
        if (0 != m_pICcu->ccu_boot(rSensorinfoInput.eWhichSensor)) {
            AE_LOG("ICCU boot fail");
            boot_failed = MTRUE;
            m_pICcu->uninit();
            return E_AE_SENSOR_INFO;
        }
        AE_LOG("CCU initial & boot done\n");


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
                AE_LOG("CCU - Invalid sensor mode: %d", m_eSensorMode);
        }

        if ( CCUIF_SUCCEED != m_pICcu->ccu_sensor_initialize(&rSensorinfoInput, &rSensorinfoOutput))
        {
            AE_LOG("ccu_sensor_initialize fail\n");
            m_pICcu->ccu_shutdown();
            m_pICcu->uninit();
            return E_AE_SENSOR_INFO;
        }
        else
        {
            AE_LOG("ccu_sensor_initialize success\n");
            if(!rSensorinfoOutput.u8SupportedByCCU)
            {
                AE_LOG("this sensor is not supported by CCU\n");
                m_pICcu->ccu_shutdown();
                m_pICcu->uninit();
                return E_AE_SENSOR_INFO;
            }
        }


        // init ccu ae
        rAEControlcfg.hdr_stat_en = m_bEnableHDRYConfig;
        rAEControlcfg.ae_overcnt_en = m_bEnableOverCntConfig;
        rAEControlcfg.tsf_stat_en = m_bEnableTSFConfig;
        rAEControlcfg.bits_num = (m_i4AAOmode==1)?14:12;
        rAEControlcfg.ae_footprint_hum = m_u4BlockNumX;
        rAEControlcfg.ae_footprint_vum = m_u4BlockNumY;
        rAEControlcfg.ae_dl_source = 1;
        rAEControlcfg.ae_footprint_lines_per_bank = 10;

        rAEcoreInit.EndBankIdx = 0;
        rAEcoreInit.m_i4AEMaxBlockWidth = m_u4BlockNumX;
        rAEcoreInit.m_i4AEMaxBlockHeight = m_u4BlockNumY;
        rAEcoreInit.uTgBlockNumX = AE_TG_BLOCK_NO_X;
        rAEcoreInit.uTgBlockNumY = AE_TG_BLOCK_NO_Y;
        rAEcoreInit.m_u4IndexMax = m_u4IndexFMax;
        rAEcoreInit.m_u4IndexMin = m_u4IndexFMin;
        rAEcoreInit.bSpeedupEscOverExp  = m_rAEInitInput.rAEPARAM.bSpeedupEscOverExp;
        rAEcoreInit.bSpeedupEscUnderExp = m_rAEInitInput.rAEPARAM.bSpeedupEscUnderExp;
        rAEcoreInit.pAETgTuningParam = *m_rAEInitInput.rAEPARAM.pAETgTuningParam;
        //rAEcoreInit.pTgWeightTbl = m_rAEInitInput.rAEPARAM.pTgWeightTbl;
        memcpy(rAEcoreInit.pTgWeightTbl, m_rAEInitInput.rAEPARAM.pTgWeightTbl, 9*12*sizeof(MUINT32));
        rAEcoreInit.pTgEVCompJumpRatioOverExp = m_rAEInitInput.rAEPARAM.pTgEVCompJumpRatioOverExp;
        rAEcoreInit.pTgEVCompJumpRatioUnderExp= m_rAEInitInput.rAEPARAM.pTgEVCompJumpRatioUnderExp;
        rAEcoreInit.LumLog2x1000 = LumLog2x1000;
        rAEcoreInit.SzLumLog2x1000 = sizeof(LumLog2x1000);
        rAEcoreInit.pCurrentTable = (strEvPline*)&m_PreviewTableF;
        rAEcoreInit.i4MaxBV = m_pPreviewTableCurrent->i4MaxBV;
        rAEcoreInit.i4BVOffset = m_rAEInitInput.rAENVRAM.rCCTConfig.i4BVOffset;
        rAEcoreInit.u4MiniISOGain = m_p3ANVRAM->rAENVRAM[m_u4AENVRAMIdx].rDevicesInfo.u4MiniISOGain;
        rAEcoreInit.iMiniBVValue = m_rAEInitInput.rAEPARAM.strAEParasetting.iMiniBVValue;
        rAEcoreInit.EVBase = m_u4FinerEVIdxBase;
        rAEcoreInit.m_rIspIsoEnv.u2Length = m_rISPNvramISOInterval.u2Length;
        rAEcoreInit.m_rIspIsoEnv.IDX_Partition = &(m_rISPNvramISOInterval.IDX_Partition[0]);
        memcpy(&rAEcoreInit.OBC_Table[0], &m_rISPNvramOBC_Table[0], 4*sizeof(CCU_ISP_NVRAM_OBC_T));
        rAEInitparam.control_cfg = rAEControlcfg;
        rAEInitparam.algo_init_param = rAEcoreInit;
        int cnt = (rAEcoreInit.SzLumLog2x1000 / 4);
        AE_LOG("[startCCU] IndexMax/BlockNumX/BlockNumY/footprint hum/vum/OverExp[70]/UnderExp[3]/Size/[0]/[cnt-1] : %x/%x/%x/%x/%x/%x/%x/%x/%x/%x \n",
            rAEcoreInit.m_u4IndexMax,rAEcoreInit.uTgBlockNumX,rAEcoreInit.uTgBlockNumY,
            rAEControlcfg.ae_footprint_hum,rAEControlcfg.ae_footprint_vum,
            rAEcoreInit.pTgEVCompJumpRatioOverExp[70],rAEcoreInit.pTgEVCompJumpRatioUnderExp[3],
            rAEcoreInit.SzLumLog2x1000, rAEcoreInit.LumLog2x1000[0], rAEcoreInit.LumLog2x1000[cnt-1]);

        AE_LOG("[%s()] m_i4SensorDev:%d i4OBCtableidx: %d [0][1][2][3](offset/gain) = [%d/%d][%d/%d][%d/%d][%d/%d] \n",
            __FUNCTION__, m_eSensorDev, m_i4OBCTableidx,
            rAEcoreInit.OBC_Table[0].offst0, rAEcoreInit.OBC_Table[0].gain0,
            rAEcoreInit.OBC_Table[1].offst0, rAEcoreInit.OBC_Table[1].gain0,
            rAEcoreInit.OBC_Table[2].offst0, rAEcoreInit.OBC_Table[2].gain0,
            rAEcoreInit.OBC_Table[3].offst0, rAEcoreInit.OBC_Table[3].gain0);


        if ( CCUIF_SUCCEED != m_pICcu->ccu_ae_initialize(&rAEInitparam) )
        {
            AE_LOG("ccu_ae_initialize fail\n");
            m_pICcu->ccu_shutdown();
            m_pICcu->uninit();
            return E_AE_SENSOR_INFO;
        }
        AE_LOG("ccu_ae_initialize success\n");

        /*int debugInfo;
        while(1)
        {
            debugInfo = m_pICcu->readInfoReg(20);
            AE_LOG( "[%s()] bankdone debug: %x\n", __FUNCTION__, debugInfo);
            usleep(1000);
        }*/

        m_bIsCCUStart = MTRUE;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::startCCUAE()
{
    if(!IsCCUAEWorking() && IsCCUWorking() && IsCCUStart()){

        AE_LOG( "[%s()]\n", __FUNCTION__);
        m_pICcu->ccu_ae_start();
        m_bIsCCUAEWorking = MTRUE;
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::stopCCU()
{
    if (m_bIsCCUStart){
        AE_LOG( "[%s()]\n", __FUNCTION__);
        m_pICcu->ccu_shutdown();
        m_pICcu->uninit();
        m_bIsCCUStart = MFALSE;
        m_bIsCCUAEWorking = MFALSE;
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::controlCCU()
{
    if(IsCCUWorking() && IsCCUAEWorking())
    {

    ccu_ae_roi rAPAERoi;
    ccu_ae_frame_sync_data rCcuFrameSyncData;
    ccu_ae_onchange_data rOnchangeData;

    memset(&rAPAERoi, 0, sizeof(ccu_ae_roi));
    memset(&rCcuFrameSyncData, 0, sizeof(ccu_ae_frame_sync_data));
    memset(&rOnchangeData, 0, sizeof(ccu_ae_onchange_data));

    if (m_bControlCCUInfochange)
    {
        rOnchangeData.m_u4IndexMax = m_u4IndexFMax;
        rOnchangeData.m_u4IndexMin = m_u4IndexFMin;
        rOnchangeData.auto_flicker_en = m_bEnableFlickerFPSstate;
        rOnchangeData.pCurrentTable = (strEvPline*)&m_PreviewTableF;
        /* Eric Here
        memcpy(&rOnchangeData.OBC_Table[0], &m_rISPNvramOBC_Table[0], 4*sizeof(CCU_ISP_NVRAM_OBC_T));
        */
        m_pICcu->ccu_ae_set_onchange_data(&rOnchangeData);
        m_bControlCCUInfochange = MFALSE;
        AE_LOG( "[%s()] MaxIdx/MinIdx/FlickerState/pPlineTableAddress/pPlinetable->eID %d/%d/%d/0x%x/%d \n",
            __FUNCTION__,
            rOnchangeData.m_u4IndexMax,
            rOnchangeData.m_u4IndexMin,
            rOnchangeData.auto_flicker_en,
            rOnchangeData.pCurrentTable,
            m_pPreviewTableCurrent->eID);
    }



    rCcuFrameSyncData.is_zooming = m_eZoomWinInfo.bZoomChange;
    rCcuFrameSyncData.magic_num = m_u4HwMagicNum;
    //rCcuFrameSyncData.m_u4IndexMax = m_u4IndexMax;
    //rCcuFrameSyncData.m_u4IndexMin = m_u4IndexMin;
    //AE_LOG( "[%s()]rCcuFrameSyncData.is_zooming:0x%x\n", __FUNCTION__, rCcuFrameSyncData.is_zooming);
    AE_LOG_IF(m_3ALogEnable, "[%s()]rCcuFrameSyncData.magic_num:0x%x\n", __FUNCTION__, rCcuFrameSyncData.magic_num);
    //AE_LOG_IF(m_3ALogEnable, "[%s()]rCcuFrameSyncData.m_u4IndexMax:0x%x\n", __FUNCTION__, rCcuFrameSyncData.m_u4IndexMax);
    //AE_LOG_IF(m_3ALogEnable, "[%s()]rCcuFrameSyncData.m_u4IndexMin:0x%x\n", __FUNCTION__, rCcuFrameSyncData.m_u4IndexMin);

    m_pICcu->ccu_ae_set_frame_sync_data(&rCcuFrameSyncData);

    rAPAERoi.AEWinBlock_u4XHi =119;
    rAPAERoi.AEWinBlock_u4XLow=  0;
    rAPAERoi.AEWinBlock_u4YHi = 89;
    rAPAERoi.AEWinBlock_u4YLow=  0;

    //AE_LOG( "[%s()]m_eAEEVcomp:0x%x\n", __FUNCTION__, m_eAEEVcomp);
    m_pICcu->ccu_ae_set_ev_comp(&m_eAEEVcomp);

    if ((m_eAEMode == LIB3A_AE_MODE_OFF)||m_bAELock||m_bAdbAELock)
    {
        //AE_LOG( "[%s()]ccu_ae_set_skip_algo\n", __FUNCTION__);
        m_pICcu->ccu_ae_set_skip_algo(MTRUE);
    }
    else
    {
        m_pICcu->ccu_ae_set_skip_algo(MFALSE);
    }

    if(m_bSetCCUFrameRate){
        AE_LOG( "[%s()]m_i4AEMaxFps:0x%x\n", __FUNCTION__, m_i4AEMaxFps);
        m_pICcu->ccu_ae_set_max_framerate((MUINT16)m_i4AEMaxFps, MTRUE);
        m_bSetCCUFrameRate = MFALSE;
    }
    //AE_LOG( "[%s()]rAPAERoi.AEWinBlock_u4XHi:0x%x\n", __FUNCTION__, rAPAERoi.AEWinBlock_u4XHi);
    //AE_LOG( "[%s()]rAPAERoi.AEWinBlock_u4XLow:0x%x\n", __FUNCTION__, rAPAERoi.AEWinBlock_u4XLow);
    //AE_LOG( "[%s()]rAPAERoi.AEWinBlock_u4YHi:0x%x\n", __FUNCTION__, rAPAERoi.AEWinBlock_u4YHi);
    //AE_LOG( "[%s()]rAPAERoi.AEWinBlock_u4YLow:0x%x\n", __FUNCTION__, rAPAERoi.AEWinBlock_u4YLow);
    m_pICcu->ccu_ae_set_roi(&rAPAERoi);

    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getCCUResult()
{
    if(IsCCUWorking() && IsCCUAEWorking()){

    /*
    int debugInfo;
    debugInfo = m_pICcu->readInfoReg(20);

    AE_LOG( "[%s()] bankdone debug: %x\n", __FUNCTION__, debugInfo);
    */

    AE_LOG_IF(m_3ALogEnable, "[%s()] AE_CORE_MAIN_OUT size: %x\n", __FUNCTION__, sizeof(AE_CORE_MAIN_OUT));

    AE_CORE_MAIN_OUT rAEOut;
    memset(&rAEOut, 0, sizeof(AE_CORE_MAIN_OUT));
    m_pICcu->ccu_ae_get_output(&rAEOut);

    /*AE_LOG( "[%s()] rAEOut.m_u4EffectiveIndex: 0x%x\n", __FUNCTION__, rAEOut.m_u4EffectiveIndex);
    AE_LOG( "[%s()] rAEOut.m_u4Index: 0x%x\n", __FUNCTION__, rAEOut.m_u4Index);
    AE_LOG( "[%s()] rAEOut.m_i4RealBV: 0x%x\n", __FUNCTION__, rAEOut.m_i4RealBV);
    AE_LOG( "[%s()] rAEOut.m_i4RealBVx1000: 0x%x\n", __FUNCTION__, rAEOut.m_i4RealBVx1000);
    AE_LOG( "[%s()] rAEOut.m_i4BV: 0x%x\n", __FUNCTION__, rAEOut.m_i4BV);
    AE_LOG( "[%s()] rAEOut.m_i4AoeCompBV: 0x%x\n", __FUNCTION__, rAEOut.m_i4AoeCompBV);
    AE_LOG( "[%s()] rAEOut.m_i4EV: 0x%x\n", __FUNCTION__, rAEOut.m_i4EV);
    AE_LOG( "[%s()] rAEOut.enumCpuAction: 0x%x\n", __FUNCTION__, rAEOut.enumCpuAction);
    AE_LOG( "[%s()] rAEOut.enumCcuAction: 0x%x\n", __FUNCTION__, rAEOut.enumCcuAction);

    AE_LOG( "[%s()] rAEOut.ObcResult.offst0: 0x%x\n", __FUNCTION__, rAEOut.ObcResult.offst0);
    AE_LOG( "[%s()] rAEOut.ObcResult.offst1: 0x%x\n", __FUNCTION__, rAEOut.ObcResult.offst1);
    AE_LOG( "[%s()] rAEOut.ObcResult.offst2: 0x%x\n", __FUNCTION__, rAEOut.ObcResult.offst2);
    AE_LOG( "[%s()] rAEOut.ObcResult.offst3: 0x%x\n", __FUNCTION__, rAEOut.ObcResult.offst3);
    AE_LOG( "[%s()] rAEOut.ObcResult.gain0: 0x%x\n", __FUNCTION__, rAEOut.ObcResult.gain0);
    AE_LOG( "[%s()] rAEOut.ObcResult.gain1: 0x%x\n", __FUNCTION__, rAEOut.ObcResult.gain1);
    AE_LOG( "[%s()] rAEOut.ObcResult.gain2: 0x%x\n", __FUNCTION__, rAEOut.ObcResult.gain2);
    AE_LOG( "[%s()] rAEOut.ObcResult.gain3: 0x%x\n", __FUNCTION__, rAEOut.ObcResult.gain3);*/

    if (rAEOut.enumCpuAction == AE_CPU_ACTION_DO_FULL_AE)
    {
        AE_LOG( "[%s() CCU Nextidx %d->%d]\n", __FUNCTION__, m_i4AEidxNext, rAEOut.a_Output.i4AEidxNext);

        //Fixme: check strAEOutput is differ from strCcuAeOutput

        m_rAEOutput.rPreviewMode.u4Eposuretime = rAEOut.a_Output.EvSetting.u4Eposuretime;
        m_rAEOutput.rPreviewMode.u4AfeGain = rAEOut.a_Output.EvSetting.u4AfeGain;
        m_rAEOutput.rPreviewMode.u4IspGain = rAEOut.a_Output.EvSetting.u4IspGain;
        m_i4AEidxNext = rAEOut.a_Output.i4AEidxNext;
        m_i4AEidxCurrent = rAEOut.a_Output.i4AEidxCurrent;
        m_i4EVvalue = rAEOut.a_Output.i4EV;
        m_BVvalue = rAEOut.a_Output.Bv;
        m_rAEOutput.rPreviewMode.u4RealISO = rAEOut.a_Output.u4ISO;
        m_AOECompBVvalue = rAEOut.a_Output.AoeCompBv;
        m_strAECycleInfo.m_i4FrameCnt = 0;
        m_pIAeAlgo->setEVIdxInfo_v4p0(m_u4IndexMax ,m_u4IndexMin ,m_u4Index, m_u4IndexFMax, m_u4IndexFMin, m_i4AEidxNext);
        updateAEInfo2ISP(AE_AUTO_FRAMERATE_STATE, m_rAEOutput.rPreviewMode.u4IspGain);

        for (int i = m_uEvQueueSize - 2 ; i >= 0; i--) {
            m_u4PreTgIntAEidxQueue[i+1]= m_u4PreTgIntAEidxQueue[i];
        }
            m_u4PreTgIntAEidxQueue[0] = m_i4AEidxNext;



        if (m_strAECycleInfo.m_i4FrameCnt > 10)
            updatePreEvSettingQueue(m_rAEOutput.rPreviewMode.u4Eposuretime,m_rAEOutput.rPreviewMode.u4AfeGain,m_rAEOutput.rPreviewMode.u4IspGain);


    }
    else
    {   // FixMe : Post new recalculate RA thread HERE
        if (m_eAEMode != LIB3A_AE_MODE_OFF && (rAEOut.a_Output.EvSetting.u4Eposuretime!=0) &&
            (rAEOut.a_Output.EvSetting.u4AfeGain!=0)&&(rAEOut.a_Output.EvSetting.u4IspGain!=0)){
            m_rAEOutput.rPreviewMode.u4Eposuretime = rAEOut.a_Output.EvSetting.u4Eposuretime;
            m_rAEOutput.rPreviewMode.u4AfeGain = rAEOut.a_Output.EvSetting.u4AfeGain;
            m_rAEOutput.rPreviewMode.u4IspGain = rAEOut.a_Output.EvSetting.u4IspGain;
            updateAEInfo2ISP(AE_AUTO_FRAMERATE_STATE, m_rAEOutput.rPreviewMode.u4IspGain);
        }
    }
    AE_LOG( "[%s] i4SensorDev:%d , Output %d/%d/%d \n", __FUNCTION__, m_eSensorDev,
                m_rAEOutput.rPreviewMode.u4Eposuretime,
                m_rAEOutput.rPreviewMode.u4AfeGain,
                m_rAEOutput.rPreviewMode.u4IspGain);

        m_pCCUresultCB->doNotifyCb(0,NULL,NULL,NULL);
    }
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::setAEInfo2CCU(MUINT32 exp, MUINT32 afe, MUINT32 isp, MUINT32 iso, MUINT32 effective_idx, MUINT32 idx, MUINT32 delata_idx)
{
    if(IsCCUWorking() && IsCCUAEWorking()){

    //per-frame

    AE_CORE_CTRL_CPU_EXP_INFO rAECoreCtrlCPUInfo;
    memset(&rAECoreCtrlCPUInfo, 0, sizeof(AE_CORE_CTRL_CPU_EXP_INFO));

    rAECoreCtrlCPUInfo.u4CpuReady = 1;
    rAECoreCtrlCPUInfo.m_u4EffectiveIndex = effective_idx;
    rAECoreCtrlCPUInfo.m_i4deltaIndex = delata_idx;
    rAECoreCtrlCPUInfo.u4Eposuretime = exp;
    rAECoreCtrlCPUInfo.u4AfeGain = afe;
    rAECoreCtrlCPUInfo.u4IspGain = isp;
    rAECoreCtrlCPUInfo.u4ISO = iso;

    AE_LOG_IF(m_3ALogEnable, "[%s()] u4CpuReady: 0x%x m_u4EffectiveIndex: 0x%x m_i4deltaIndex: 0x%x u4Eposuretime: 0x%x u4AfeGain: 0x%x u4IspGain: 0x%x u4ISO: 0x%x\n", __FUNCTION__,
              rAECoreCtrlCPUInfo.u4CpuReady, rAECoreCtrlCPUInfo.m_u4EffectiveIndex, rAECoreCtrlCPUInfo.m_i4deltaIndex,
              rAECoreCtrlCPUInfo.u4Eposuretime, rAECoreCtrlCPUInfo.u4AfeGain, rAECoreCtrlCPUInfo.u4IspGain, rAECoreCtrlCPUInfo.u4ISO);

    m_pICcu->ccu_ae_set_ap_output(&rAECoreCtrlCPUInfo);

    }
    return S_AE_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::IsCCUStart()
{
    MBOOL res = MFALSE;
    if((m_eAETargetMode==AE_MODE_NORMAL)&& \
        (!m_bStereoManualPline)&& \
        (m_i4SMSubSamples == 1))
    {
        res = MTRUE;
        AE_LOG_IF(m_3ALogEnable, "[%s()]\n", __FUNCTION__);
    }
    else
    {
        AE_LOG_IF(m_3ALogEnable, "[CCU start condition not reached]\n");

    }
    return res;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::IsCCUWorking()
{
    MBOOL res = MFALSE;
    if (m_bIsCCUStart){
        if((!m_bStereoManualPline)&&(m_eAETargetMode == AE_MODE_NORMAL)&&(m_i4SMSubSamples == 1)){
            res = MTRUE;
        }
    }
    m_bIsCCUWorking = res;
    return res;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AeMgr::IsCCUAEWorking()
{
    return m_bIsCCUAEWorking;
}
