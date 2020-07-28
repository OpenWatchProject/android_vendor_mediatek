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
#define LOG_TAG "awb_mgr"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <stdlib.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
//#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_feature.h>
#include <flash_awb_param.h>
#include <flash_awb_tuning_custom.h>
#include <awb_param.h>
#include <awb_tuning_custom.h>
#include <ae_param.h>
//#include <isp_mgr.h>
//#include <isp_tuning_mgr.h>
//#include <kd_camera_feature.h>
//#include <isp_tuning.h>
#include <drv/tuning_mgr.h>
#include <camera_custom_cam_cal.h>
#include <mtkcam/drv/mem/cam_cal_drv.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <flash_feature.h>
#include <awb_state.h>
#include <mtkcam/drv/IHalSensor.h>
#include <private/IopipeUtils.h>
#include "awb_mgr.h"
#include <nvbuf_util.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include <faces.h>
#include <aaa_common_custom.h>
#include <time.h>

#include <mtkcam/utils/sys/SensorProvider.h>

#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <StatisticBuf.h>
#include <mtkcam/drv/mem/cam_cal_drv.h>

#include <property_utils.h>

#define SENSOR_ACCE_POLLING_MS  20
#define SENSOR_GYRO_POLLING_MS  20
#define SENSOR_ACCE_SCALE       100
#define SENSOR_GYRO_SCALE       100
#define MWBMAX 9000
#define MWBMIN 2000
///#ifdef MY_LOG
///    #undef MY_LOG
///#endif

///#define CAM_LOGD(fmt, arg...) { if(gIsLogEn) {ALOGD(fmt, ##arg);} }
using namespace NSCam;
using namespace NSCam::Utils;

using namespace NS3A;
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;


#define AWB_LOG(fmt, arg...) \
    do { \
        if (m_i4DgbLog) { \
            CAM_LOGD(fmt, ##arg); \
        } else { \
            CAM_LOGD(fmt, ##arg); \
        } \
    }while(0)

#define AWB_LOG_IF(cond, ...) \
    do { \
        if (m_i4DgbLog) { \
            CAM_LOGD_IF(cond, __VA_ARGS__); \
        } else { \
            CAM_LOGD_IF(cond, __VA_ARGS__); \
        } \
    }while(0)


AwbMgr* AwbMgr::s_pAwbMgr = MNULL;
///static int gIsLogEn=0;
MINT32 LastCCT = 0;



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <ESensorDev_T const eSensorDev>
class AwbMgrDev : public AwbMgr
{
public:
    static
        AwbMgr&
        getInstance()
    {
        static AwbMgrDev<eSensorDev> singleton;
        AwbMgr::s_pAwbMgr = &singleton;
        return singleton;
    }

    AwbMgrDev()
        : AwbMgr(eSensorDev)
    {}

    virtual ~AwbMgrDev() {}
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AwbMgr&
    AwbMgr::
    getInstance(MINT32 const i4SensorDev)
{
    switch (i4SensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  AwbMgrDev<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  AwbMgrDev<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  AwbMgrDev<ESensorDev_Sub>::getInstance();
    case ESensorDev_SubSecond: //  Sub Second Sensor
        return  AwbMgrDev<ESensorDev_SubSecond>::getInstance();
    default:
        CAM_LOGD("i4SensorDev = %d", i4SensorDev);
        if (AwbMgr::s_pAwbMgr)
            return  *AwbMgr::s_pAwbMgr;
        else
            return  AwbMgrDev<ESensorDev_Main>::getInstance();
    }
}



static void aaoSeparation(void* aao, int w, int h, void* awb, int awbPitch, void* hist)
{
    AAA_TRACE_D("AwbSep");

    int i;
    int j;
    MUINT8* pawb=(MUINT8*)awb;
    MUINT8* p=(MUINT8*)aao;

    int lineByte;
    lineByte = (int)(w*8.5+0.5);

    // Fill awb
    if(awb!=0)
    {
        int awbW=w*4;
        if(awbPitch>awbW)
            awbW=awbPitch;
        for(j=0;j<90;j++)
        {
            memcpy(pawb, p, w*4);
            p+=lineByte;
            pawb+=awbW;
        }
    }

    // Fill hist
    if(hist!=0)
    {
        int* pHist=(int* )hist;
        p=(MUINT8*)aao;
        int aaoNoHistSz;
        aaoNoHistSz = lineByte*h;
        p+=aaoNoHistSz;
        for(j=0;j<4;j++)
        {
            for(i=0;i<128;i++)
            {
                int v;
                int b1;
                int b2;
                int b3;
                b1 = *p;
                b2 = *(p+1);
                b3 = *(p+2);
                v = b1+(b2<<8)+(b3<<16);
                p+=3;
                *pHist=v;
                pHist++;
            }
        }
    }

    AAA_TRACE_END_D;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AwbMgr::
    AwbMgr(ESensorDev_T eSensorDev)
#if USE_OPEN_SOURCE_AWB
    : m_pIAwbAlgo(IAwbAlgo::createInstance<EAAAOpt_OpenSource>(eSensorDev))
#else
    : m_pIAwbAlgo(IAwbAlgo::createInstance<EAAAOpt_MTK>(eSensorDev))
#endif
    , m_pAwbStateMgr(AwbStateMgr::getInstance(eSensorDev))
    , m_eAWBMode(LIB3A_AWB_MODE_AUTO)
    , m_i4StrobeMode(AWB_STROBE_MODE_OFF)
    , m_i4AWBNvramIdx(CAM_SCENARIO_PREVIEW)
    , m_i4StrobeNvramIdx(0)
    , m_i4FlashCaliNvramIdx(0)
    , m_eCamScenarioMode(CAM_SCENARIO_PREVIEW)
    , m_bEnableAWB(MFALSE)
    , m_bAWBLock(MFALSE)
    , m_bAdbAWBLock(MFALSE)
    , m_bOneShotAWB(MFALSE)
    , m_bAWBModeChanged(MFALSE)
    , m_bStrobeModeChanged(MFALSE)
    , m_bColdBoot(MTRUE)
    , m_bAWBNvramIdxChanged(MFALSE)
    , m_eSensorDev(eSensorDev)
    , m_eSensorTG(ESensorTG_1)
    , m_eSensorMode(ESensorMode_Preview)
    , m_i4SensorIdx(0)
    , m_bDebugEnable(MFALSE)
    , m_bInitState(MFALSE)
    , m_i4AFLV(70)
    , m_bSkipOneFrame(0)
    , m_bHBIN2Enable(MFALSE)
    , m_bAWBCalibrationBypassed(MFALSE)
    , m_pNVRAM_3A(MNULL)
    , m_pNVRAM_STROBE(MNULL)
    , m_pNVRAM_FLASH_CALIBRATION(MNULL)
    , m_flashAwbWeight(0)
    , m_flashDuty(0)
    , m_flashStep(0)
    , m_rAWBStatParam()
    , m_rAWBWindowConfig()
    , m_eColorCorrectionMode(MTK_COLOR_CORRECTION_MODE_FAST)
    , m_i4BinW(0)
    , m_i4BinH(0)
    , mAaoW(0)
    , mAaoH(0)
    , m_i4QbinRatio(0)
    , mIsMono(0)
    , mSensorType(0)
    , mGainSetNum(0)
    , m_i4DgbLog(0)
    //, m_i4AAOmode(0)
    , m_bFDenable(MFALSE)
    , m_bFaceAWBAreaChage(MFALSE)
    , m_bFlashAwb(MFALSE)
    , m_bAlgoInit(MFALSE)
    , mIsMwbCctChanged(0)
    , mMWBColorTemperature(0)
    , m_bIsTorch(MFALSE)
    , m_bIsMainFlashOn(MFALSE)
    , m_bIsUseLastGain(1)
    , m_AAOStride(0)
    , m_bGryoVd(MFALSE)
    , m_bAcceVd(MFALSE)
    , m_u8AcceTS(0)
    , m_u8PreAcceTS(0)
    , m_u8GyroTS(0)
    , m_u8PreGyroTS(0)
    , SkipFlashFrameCnt(0)
    , m_pCallBackCb(nullptr)
    , m_pNormalPipe(nullptr)
    , FDFrameCnt(0)
    , m_bIsFD(MFALSE)
    , bIsStartFDCnt(MFALSE)
    , semCb()
    , m_bIsNotPostSem(MFALSE)
    , m_FlareGain(512)
    , m_FlareOffset(20)
    , m_IsSpeedAwb(1)
    , m_eAETargetMode(AE_MODE_NORMAL)
    , m_i4CurrSensorId(0)
    , m_i4CurrModuleId(0)
    , m_i4CurrLensId(0)
    , m_i4MaxFPS(0)
    , m_bPostgainLock(MFALSE)
    , m_i4DgbRtvEnable(0)
    , m_i4RtvGroupId(0)
    , m_i4MagicNum(0)
{
    mBackupGain.i4R=512;
    mBackupGain.i4G=512;
    mBackupGain.i4B=512;
    m_rColorCorrectionGain.i4R = m_rColorCorrectionGain.i4G = m_rColorCorrectionGain.i4B = 512;
    //Initailize
    mGoldenGain2 = {0,0,0};
    mGoldenGain3 = {0,0,0};
    mUnitGain2 = {0,0,0};
    mUnitGain3 = {0,0,0};
    m_Postgain = {512,512,512};
/*
    mGoldenValueM = {0,0,0,0};
    mGoldenValueL = {0,0,0,0};
    mUnitValueM = {0,0,0,0};
    mUnitValueL = {0,0,0,0};
*/
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        m_bEnableAWB = isAWBEnabled<ESensorDev_Main>();
        break;
    case ESensorDev_MainSecond: //  Main Second Sensor
        m_bEnableAWB = isAWBEnabled<ESensorDev_MainSecond>();
        break;
    case ESensorDev_Sub: //  Sub Sensor
        m_bEnableAWB = isAWBEnabled<ESensorDev_Sub>();
        break;
    case ESensorDev_SubSecond: //  Sub Second Sensor
        m_bEnableAWB = isAWBEnabled<ESensorDev_SubSecond>();
        break;
    default:
        CAM_LOGE("eSensorDev = %d", eSensorDev);
        break;
    }
    mpAao = new MUINT8[54000];
    mpHist = new MUINT8[2048];

    memset(&m_rAWBStatParam, 0, sizeof(m_rAWBStatParam));
    memset(&m_rAWBRAWPreGain1, 0, sizeof(m_rAWBRAWPreGain1));
    memset(&m_rAWBRAWPreGain1M, 0, sizeof(m_rAWBRAWPreGain1M));
    memset(&m_rAWBRAWPreGain1L, 0, sizeof(m_rAWBRAWPreGain1L));

    memset(&m_rAWBInitInput, 0, sizeof(m_rAWBInitInput));
    memset(&m_rAWBOutput, 0, sizeof(m_rAWBOutput));

    memset(&m_eAWBFDArea, 0, sizeof(AWB_FD_INFO_T));
    memset(&m_eZoomWinInfo, 0, sizeof(AWBZOOM_WINDOW_T));
    memset(&m_rAWBCustPARAM, 0, sizeof(AWB_CUST_PARAM_T));

    memset(&m_AWBStatConfig, 0, sizeof(m_AWBStatConfig));
    memset(&m_AWBStatInfo, 0, sizeof(AWB_STAT_INFO_T));
    memset(&m_ISP_AWBInfo, 0, sizeof(AWB_ISP_INFO_T));

    m_i4AcceInfo[0] = m_i4AcceInfo [1] = m_i4AcceInfo [2] = 0;
    m_i4GyroInfo[0] = m_i4GyroInfo [1] = m_i4GyroInfo [2] = 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AwbMgr::
    ~AwbMgr()
{
    delete []mpAao;
    delete []mpHist;
}
/*
#include <cutils/properties.h>
static int getPropInt(const char* sId, int DefVal)
{
    int ret;
    char ss[PROPERTY_VALUE_MAX] = {'\0'};
    char sDef[20];
    sprintf(sDef,"%d",DefVal);
    property_get(sId, ss, sDef);
    ret = atoi(ss);
    return ret;
}
*/


//#define setRgbGainX(a, b)  { setRgbGain(a,b);}
#define setRgbGainX(a, b) {}
// #define setRgbGainX(a, b)  { AWB_LOG("setRgbGainX ln=%d",__LINE__); setRgbGain(a,b);}
#if 0
MBOOL AwbMgr::setRgbGain(MINT32 const i4SensorDev, AWB_GAIN_T& rIspAWBGain)
{
    int propR;
    int propG;
    int propB;
    propR = getPropInt("vendor.z.awb_gain_r", -1);
    propG = getPropInt("vendor.z.awb_gain_g", -1);
    propB = getPropInt("vendor.z.awb_gain_b", -1);
    if((propR!=-1) && (propG!=-1) && (propB!=-1))
    {
      rIspAWBGain.i4R = propR;
      rIspAWBGain.i4G = propG;
      rIspAWBGain.i4B = propG;
    }

    if(mIsMono)
    {
        rIspAWBGain.i4R=512;
        rIspAWBGain.i4G=512;
        rIspAWBGain.i4B=512;
    }

    strSensorAWBGain rSensorAWBGain;
    rSensorAWBGain.u4R = rIspAWBGain.i4R;
    rSensorAWBGain.u4GR = rIspAWBGain.i4G;
    rSensorAWBGain.u4GB = rIspAWBGain.i4G;
    rSensorAWBGain.u4B = rIspAWBGain.i4B;

    //AWB_LOG("awb gain %d %d %d %d",i4SensorDev, rIspAWBGain.i4R,rIspAWBGain.i4G, rIspAWBGain.i4B);
    AWB_LOG_IF(m_i4DgbLog ,"[%s()][%d] awb gain %d %d %d\n", __FUNCTION__, i4SensorDev, rIspAWBGain.i4R,rIspAWBGain.i4G, rIspAWBGain.i4B);

    //AAASensorMgr::getInstance().setSensorAWBGain((ESensorDev_T)i4SensorDev, &rSensorAWBGain);

    //return IspTuningMgr::getInstance().setAWBGain(i4SensorDev, rIspAWBGain);
    return TRUE;

}
#endif

MBOOL AwbMgr::setAEPreGain2(MINT32 const i4SensorDev, MINT32  sensorIdx, AWB_GAIN_T& rPreGain2)
{
  if(mIsMono)
  {
        rPreGain2.i4R=512;
        rPreGain2.i4G=512;
        rPreGain2.i4B=512;
  }

  m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4PreGain2R = rPreGain2.i4R;
  m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4PreGain2G = rPreGain2.i4G;
  m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4PreGain2B = rPreGain2.i4B;

  AWB_LOG_IF(m_i4DgbLog, "[%s()] i4PreGain2: %d/%d/%d \n", __FUNCTION__,
            m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4PreGain2R,m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4PreGain2G,
            m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4PreGain2B);

  //IspTuningMgr::getInstance().setIspAEPreGain2(i4SensorDev, sensorIdx, rPreGain2);
  return 1;
}


MBOOL AwbMgr::setIspAwbInfo(MINT32 const i4SensorDev, AWB_INFO_T &rAWBInfo)
{
    CAM_TRACE_CALL();
    if(mIsMono)
    {
        m_ISP_AWBInfo.rRPG.i4R = m_ISP_AWBInfo.rPGN.i4R = rAWBInfo.rCurrentAWBGain.i4R = 512;
        m_ISP_AWBInfo.rRPG.i4G = m_ISP_AWBInfo.rPGN.i4G = rAWBInfo.rCurrentAWBGain.i4G = 512;
        m_ISP_AWBInfo.rRPG.i4B = m_ISP_AWBInfo.rPGN.i4B = rAWBInfo.rCurrentAWBGain.i4B = 512;
        AWB_LOG( "[%s()] MONO Sensor GAIN512",__FUNCTION__);
    }

    MUINT32 rCscCCM[9] = {0x200, 0, 0, 0, 0x200, 0, 0, 0, 0x200};// Bayer
    //::memcpy(&m_rAWBOutput.rAWBInfo.rCscCCM, &rCscCCM, sizeof(rCscCCM));
    ::memcpy(&m_ISP_AWBInfo.rCscCCM, &rCscCCM, sizeof(rCscCCM));

    if ((0 == m_ISP_AWBInfo.rRPG.i4R) || (0 == m_ISP_AWBInfo.rPGN.i4R))
    {
        m_ISP_AWBInfo.rRPG.i4R = m_ISP_AWBInfo.rPGN.i4R = rAWBInfo.rCurrentAWBGain.i4R;
        m_ISP_AWBInfo.rRPG.i4G = m_ISP_AWBInfo.rPGN.i4G = rAWBInfo.rCurrentAWBGain.i4G;
        m_ISP_AWBInfo.rRPG.i4B = m_ISP_AWBInfo.rPGN.i4B = rAWBInfo.rCurrentAWBGain.i4B;
        //CAM_LOGE("[%s] RPG/PGN NULL\n", __FUNCTION__);
    }

    AWB_LOG( "[%s()] m_eSensorDev: %d, awb_gain: %d/%d/%d, PreGain1: %d/%d/%d, RPG: %d/%d/%d, PGN: %d/%d/%d, bAWBLock: %d (m_bAWBLock:%d, m_eAWBMode:%d)\n",
    __FUNCTION__, m_eSensorDev,
    rAWBInfo.rCurrentAWBGain.i4R, rAWBInfo.rCurrentAWBGain.i4G, rAWBInfo.rCurrentAWBGain.i4B,
    rAWBInfo.rPregain1.i4R, rAWBInfo.rPregain1.i4G, rAWBInfo.rPregain1.i4B,
    m_ISP_AWBInfo.rRPG.i4R, m_ISP_AWBInfo.rRPG.i4G, m_ISP_AWBInfo.rRPG.i4B,
    m_ISP_AWBInfo.rPGN.i4R, m_ISP_AWBInfo.rPGN.i4G, m_ISP_AWBInfo.rPGN.i4B,
    rAWBInfo.bAWBLock, m_bAWBLock, m_eAWBMode);

    //IspTuningMgr::getInstance().setAWBInfo(i4SensorDev, rAWBInfo);

    setAWBInfo(rAWBInfo);

    //Calculate postgain and update config
    m_Postgain.i4R = rAWBInfo.rPregain1.i4R;
    m_Postgain.i4G = rAWBInfo.rPregain1.i4G;
    m_Postgain.i4B = rAWBInfo.rPregain1.i4B;
    setPostgain(m_Postgain);

    //Set AWB gain to sensor for MVHDR sensor
    strSensorAWBGain rSensorAWBGain = {512,512,512,512};
    if((m_eAETargetMode == AE_MODE_MVHDR_TARGET) || (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET))
    {
        rSensorAWBGain.u4R = rAWBInfo.rCurrentAWBGain.i4R;
        rSensorAWBGain.u4GR = rAWBInfo.rCurrentAWBGain.i4G;
        rSensorAWBGain.u4GB = rAWBInfo.rCurrentAWBGain.i4G;
        rSensorAWBGain.u4B = rAWBInfo.rCurrentAWBGain.i4B;

        AAASensorMgr::getInstance().setSensorAWBGain((ESensorDev_T)i4SensorDev, &rSensorAWBGain);
    }

    AWB_LOG_IF(m_i4DgbLog ,"[%s()] m_Postgain: %d/%d/%d, m_eAETargetMode: %d, rSensorAWBGain: %d/%d/%d/%d \n", __FUNCTION__, m_Postgain.i4R,m_Postgain.i4G, m_Postgain.i4B,
		      m_eAETargetMode,rSensorAWBGain.u4R,rSensorAWBGain.u4GR,rSensorAWBGain.u4GB,rSensorAWBGain.u4B);

  return 1;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void printGain(AWB_GAIN_T& gain)
{
    int r;
    int g;
    int b;
    r = gain.i4R;
    g = gain.i4G;
    b = gain.i4B;
    static int vv=0;
    vv++;
    CAM_LOGD("printGain 0 t=%d rgb=%d %d %d",vv,r,g,b);
}

MBOOL AwbMgr::init(MINT32 i4SensorIdx, IAWBInitPara* initPara)
{
    (void)initPara;

    AWB_LOG("[%s()] m_eSensorDev: %d, i4SensorIdx = %d, m_eSensorMode = %d\n", __FUNCTION__, m_eSensorDev, i4SensorIdx, m_eSensorMode);

    m_i4SensorIdx = i4SensorIdx;

    getPropInt("vendor.debug.awb_log.enable", &m_i4DgbLog, 0);

    getPropInt("vendor.debug.awb_last_gain.enable", &m_bIsUseLastGain, 1);

    getPropInt("vendor.debug.awb.rtv.enable", &m_i4DgbRtvEnable, 0);
    if(m_i4DgbRtvEnable)
    {
        getPropInt("vendor.debug.awb.rtv.group_id", &m_i4RtvGroupId, 0);
    }

    //getPropInt("vendor.debug.awb_speedup.enable", &m_IsSpeedAwb, 1);
    m_IsSpeedAwb = 0;

    return MTRUE;
}

MBOOL AwbMgr::start()
{//capture init/ preview reinit
    AWB_LOG("[%s()] ln=%d senDev=%d",__FUNCTION__ ,__LINE__ ,m_eSensorDev);

    //m_AAOStride = (120*8+120*12*AE_PARAM_AE_LINEAR_STAT_EN+120*2+120*1);
    //BlockNumH*(BlockNumW*16+BlockNumW*2*(1+AA_AE_SE_STAT_EN)+BlockNumW*4*AA_AE_OVERCNT_EN)+u4EnCnt*256*3;
    m_AAOStride = 120*16+120*2*(1+AE_PARAM_AE_SE_STAT_EN)+120*4*AE_PARAM_AE_OVERCNT_EN;

    // set strobe mode to OFF
    if (setStrobeMode(AWB_STROBE_MODE_OFF) != MTRUE)
    {
        CAM_LOGE("setStrobeMode() fail\n");
    }

    // Get sensor resolution
    getSensorResolution();

    // Get NVRAM data
    if (!getNvramData()) {
        CAM_LOGE("getNvramData() fail\n");
        return MFALSE;
    }

    // Init AWB
    if (!AWBInit()) {
        CAM_LOGE("AWBInit() fail\n");
        return MFALSE;
    }

    // Face detection
    m_bFaceAWBAreaChage = MFALSE;
    m_eAWBFDArea.u4Count = 0;

    // AWB statistics config
    //ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).config(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable);

    // update AE RAW pre-gain2
    //IspTuningMgr::getInstance().setIspAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);

    // update AWB gain
    //IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rPreviewAWBGain);

    // force ISP validate
    //IspTuningMgr::getInstance().forceValidate(m_eSensorDev);

    queryHBinInfo();
    m_bStrobeModeChanged = MFALSE;
    m_bAWBModeChanged = MFALSE;
    m_bOneShotAWB = MTRUE; // do one-shot AWB
    m_bInitState = MTRUE; // init state
    bIsStartFDCnt = MTRUE;

    if(m_eAWBMode==LIB3A_AWB_MODE_MWB)
    {
         m_pIAwbAlgo->setAWBMode(m_eAWBMode);
         LIGHT_AREA_T rLightArea;
         m_pIAwbAlgo->getMWBLightArea(mMWBColorTemperature, rLightArea);

         m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4RightBound;
         m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4LeftBound;
         m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4LowerBound;
         m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4UpperBound;

         m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4RightBound;
         m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4LeftBound;
         m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4LowerBound;
         m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4UpperBound;
         m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode],
                                          m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
                                          m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);
    }
#if 0
    if (m_i4StrobeMode == AWB_STROBE_MODE_ON) {
        m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode],
            m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
            m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);

        // update AWB statistics config
        ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).config(m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
            , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY);
    }
    else {
        m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode],
            m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
            m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);

        // update AWB statistics config
        ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).config(m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
            , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY);
    }

    // update AE RAW pre-gain2
    IspTuningMgr::getInstance().setIspAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);
#else
        m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode],
            m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
            m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);

        // update AWB statistics config
        //AWBResultConfig_T rAWBResultConfig;

        setAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);

        ISP_AWB_CONFIG_T::getInstance(m_eSensorDev).AWBConfig(&m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], &m_sAWBResultConfig);

        //ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).configReg(&(m_sAWBResultConfig.rAWBRegInfo));
#endif

 if (m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R == 0)
    {

       if ((1 == m_bIsUseLastGain) && (0 != LastCCT))
       {
          CalSencondSensorDefaultGain(m_rAWBOutput.rAWBInfo.rCurrentAWBGain, LastCCT);
          AWB_LOG("[%s] Use last gain = %d/%d/%d cct :%d",__FUNCTION__,m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R,
                   m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G, m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B, LastCCT);

       }
       else
       {
         m_rAWBOutput.rAWBInfo.rCurrentAWBGain = m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_DNP];
         AWB_LOG("[%s] DNP default gain = %d/%d/%d",__FUNCTION__,m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R,
                   m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G, m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B);
       }
    }


    // update AWB gain
    //IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rPreviewAWBGain);
    //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
    //Assign RPG/PGN
    //m_ISP_AWBInfo.rRPG.i4R = m_ISP_AWBInfo.rPGN.i4R = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
    //m_ISP_AWBInfo.rRPG.i4G = m_ISP_AWBInfo.rPGN.i4G = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
    //m_ISP_AWBInfo.rRPG.i4B = m_ISP_AWBInfo.rPGN.i4B = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
    //setIspAwbInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);

    // force ISP validate
    //IspTuningMgr::getInstance().forceValidate(m_eSensorDev);

    return MTRUE;
}
MBOOL AwbMgr::stop()
{
    if (m_i4StrobeMode == AWB_STROBE_MODE_ON)
       m_rAWBOutput.rAWBInfo.rCurrentAWBGain = m_rAWBOutput.rPreviewStrobeAWBGain;
    else
       m_rAWBOutput.rAWBInfo.rCurrentAWBGain = m_rAWBOutput.rPreviewAWBGain;
    //mBackupGain = m_rAWBOutput.rPreviewAWBGain;
    //IspTuningMgr::getInstance().setAWBGain(m_eSensorDev,mBackupGain);
    //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev,m_rAWBOutput.rAWBInfo);
    //Assign RPG/PGN
    m_ISP_AWBInfo.rRPG.i4R = m_ISP_AWBInfo.rPGN.i4R = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
    m_ISP_AWBInfo.rRPG.i4G = m_ISP_AWBInfo.rPGN.i4G = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
    m_ISP_AWBInfo.rRPG.i4B = m_ISP_AWBInfo.rPGN.i4B = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
    setIspAwbInfo(m_eSensorDev,m_rAWBOutput.rAWBInfo);
    LastCCT = m_rAWBOutput.rAWBInfo.i4CCT;
    FDFrameCnt = 0;
    m_bIsFD = MFALSE;

    m_bIsNotPostSem = MFALSE;

    if (m_pCallBackCb){
      delete m_pCallBackCb;
      m_pCallBackCb = NULL;
        }

    if (m_pNormalPipe){
      m_pNormalPipe->destroyInstance(LOG_TAG);
      m_pNormalPipe = NULL;
        }
        AWB_LOG("[%s()]m_pNormalPipe \n", __FUNCTION__);

    AWB_LOG("stop ln=%d senDev=%d",__LINE__,m_eSensorDev);
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::uninit()
{
    AWB_LOG("[%s()] m_eSensorDev: %d\n", __FUNCTION__, m_eSensorDev);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::setAWBMode(MINT32 i4NewAWBMode)
{
    LIB3A_AWB_MODE_T eNewAWBMode;
    AWB_LOG_IF(m_i4DgbLog,"[%s()] m_eSensorDev: %d mode=%d\n", __FUNCTION__, m_eSensorDev, i4NewAWBMode);
    //AWB_LOG("i4NewAWBMode: %d\n", i4NewAWBMode);

    switch (i4NewAWBMode) {
    case MTK_CONTROL_AWB_MODE_OFF: // Off
        eNewAWBMode = LIB3A_AWB_MODE_OFF;
        break;
    case MTK_CONTROL_AWB_MODE_AUTO: // Auto
        eNewAWBMode = LIB3A_AWB_MODE_AUTO;
        break;
    case MTK_CONTROL_AWB_MODE_DAYLIGHT: // Daylight
        eNewAWBMode = LIB3A_AWB_MODE_DAYLIGHT;
        break;
    case MTK_CONTROL_AWB_MODE_CLOUDY_DAYLIGHT: // Cloudy daylight
        eNewAWBMode = LIB3A_AWB_MODE_CLOUDY_DAYLIGHT;
        break;
    case MTK_CONTROL_AWB_MODE_SHADE: // Shade
        eNewAWBMode = LIB3A_AWB_MODE_SHADE;
        break;
    case MTK_CONTROL_AWB_MODE_TWILIGHT: // Twilight
        eNewAWBMode = LIB3A_AWB_MODE_TWILIGHT;
        break;
    case MTK_CONTROL_AWB_MODE_FLUORESCENT: // Fluorescent
        eNewAWBMode = LIB3A_AWB_MODE_FLUORESCENT;
        break;
    case MTK_CONTROL_AWB_MODE_WARM_FLUORESCENT: // Warm fluorescent
        eNewAWBMode = LIB3A_AWB_MODE_WARM_FLUORESCENT;
        break;
    case MTK_CONTROL_AWB_MODE_INCANDESCENT: // Incandescent
        eNewAWBMode = LIB3A_AWB_MODE_INCANDESCENT;
        break;
    case MTK_CONTROL_AWB_MODE_GRAYWORLD: // Grayword
        eNewAWBMode = LIB3A_AWB_MODE_GRAYWORLD;
        break;
    case MTK_CONTROL_AWB_MODE_MWB:
        eNewAWBMode = LIB3A_AWB_MODE_MWB;
        break;
    default:
        CAM_LOGE("E_AWB_UNSUPPORT_MODE: %d\n", i4NewAWBMode);
        return (E_AWB_UNSUPPORT_MODE);
    }

    if (m_eAWBMode != eNewAWBMode)
    {
        m_eAWBMode = eNewAWBMode;
        m_bAWBModeChanged = MTRUE;

        // reset AWB state
        if (m_eAWBMode == LIB3A_AWB_MODE_AUTO)
            m_pAwbStateMgr->reset(eState_InactiveAuto);
        else
            m_pAwbStateMgr->reset(eState_InactiveNotAuto);

        AWB_LOG("m_eAWBMode: %d\n", m_eAWBMode);
    }

    m_rAWBOutput.rAWBInfo.bAWBLock = (m_bAWBLock || (m_eAWBMode == LIB3A_AWB_MODE_OFF));
    if(m_rAWBOutput.rAWBInfo.bAWBLock)
    {
       //Assign RPG/PGN
       m_ISP_AWBInfo.rRPG.i4R = m_ISP_AWBInfo.rPGN.i4R = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
       m_ISP_AWBInfo.rRPG.i4G = m_ISP_AWBInfo.rPGN.i4G = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
       m_ISP_AWBInfo.rRPG.i4B = m_ISP_AWBInfo.rPGN.i4B = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
       setIspAwbInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AwbMgr::getAWBMode() const
{
    return static_cast<MINT32>(m_eAWBMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::setSensorMode(MINT32 i4NewSensorMode, MINT32 i4BinWidth, MINT32 i4BinHeight, MINT32 i4QbinWidth, MINT32 i4QbinHeight)
{
    ESensorMode_T eNewSensorMode;
    AWB_LOG("[%s()] m_eSensorDev: %d\n", __FUNCTION__, m_eSensorDev);
    AWB_LOG("i4NewSensorMode: %d\n", i4NewSensorMode);

    switch (i4NewSensorMode) {
    case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
        eNewSensorMode = ESensorMode_Preview;
        break;
    case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
        eNewSensorMode = ESensorMode_Capture;
        break;
    case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
        eNewSensorMode = ESensorMode_Video;
        break;
    case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
        eNewSensorMode = ESensorMode_SlimVideo1;
        break;
    case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
        eNewSensorMode = ESensorMode_SlimVideo2;
        break;
    case SENSOR_SCENARIO_ID_CUSTOM1:
        eNewSensorMode = ESensorMode_Custom1;
        break;
    case SENSOR_SCENARIO_ID_CUSTOM2:
        eNewSensorMode = ESensorMode_Custom2;
        break;
    case SENSOR_SCENARIO_ID_CUSTOM3:
        eNewSensorMode = ESensorMode_Custom3;
        break;
    case SENSOR_SCENARIO_ID_CUSTOM4:
        eNewSensorMode = ESensorMode_Custom4;
        break;
    case SENSOR_SCENARIO_ID_CUSTOM5:
        eNewSensorMode = ESensorMode_Custom5;
        break;
    default:
        CAM_LOGE("E_AWB_UNSUPPORT_SENSOR_MODE: %d\n", i4NewSensorMode);
        return MFALSE;
    }

    if ((m_eSensorMode != eNewSensorMode) || (m_i4BinW != i4BinWidth) || (m_i4BinH != i4BinHeight) || (mAaoW != i4QbinWidth) || (mAaoH != i4QbinHeight))
    {
        m_eSensorMode = eNewSensorMode;
        m_i4BinW = i4BinWidth;
        m_i4BinH = i4BinHeight;
        mAaoW = i4QbinWidth;
        mAaoH = i4QbinHeight;
        m_i4QbinRatio = i4BinWidth / i4QbinWidth;
        AWB_LOG("[%s()] m_eSensorMode: %d m_i4BinW = %d m_i4BinH = %d mAaoW = %d mAaoH = %d m_i4QbinRatio = %d\n", __FUNCTION__, m_eSensorMode, m_i4BinW, m_i4BinH, mAaoW, mAaoH, m_i4QbinRatio);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AwbMgr::getSensorMode() const
{
    return static_cast<MINT32>(m_eSensorMode);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::setStrobeMode(MINT32 i4NewStrobeMode)
{
    if ((i4NewStrobeMode < AWB_STROBE_MODE_ON) || (i4NewStrobeMode > AWB_STROBE_MODE_OFF))
    {
        CAM_LOGE("Unsupport strobe mode: %d\n", i4NewStrobeMode);
        return E_AWB_UNSUPPORT_MODE;
    }

    if (m_i4StrobeMode != i4NewStrobeMode)
    {
        m_i4StrobeMode = i4NewStrobeMode;
        m_bStrobeModeChanged = MTRUE;
        AWB_LOG("[%s()][%d] m_i4StrobeMode: %d\n", __FUNCTION__, m_eSensorDev, m_i4StrobeMode);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AwbMgr::getStrobeMode() const
{
    return m_i4StrobeMode;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::setFlashAWBData(FLASH_AWB_PASS_FLASH_INFO_T &rFlashAwbData)
{
    m_flashAwbWeight = rFlashAwbData.flashAwbWeight;
    m_flashDuty = rFlashAwbData.flashDuty;
    m_flashStep = rFlashAwbData.flashStep;

    AWB_LOG(" [%d] m_flashAwbWeight = %d\n", m_eSensorDev, m_flashAwbWeight);
    AWB_LOG(" [%d] m_flashDuty = %d\n", m_eSensorDev, m_flashDuty);
    AWB_LOG(" [%d] m_flashStep = %d\n", m_eSensorDev, m_flashStep);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::setAWBLock(MBOOL bAWBLock)
{
    AWB_LOG_IF(m_i4DgbLog,"[%d] bAWBLock: %d\n", m_eSensorDev, bAWBLock);

    if (m_bAWBLock != bAWBLock)
    {
        if (bAWBLock) { // AWB lock
            m_bAWBLock = MTRUE;
            m_bOneShotAWB = MTRUE;
            m_pAwbStateMgr->sendIntent(eIntent_AWBLockOn);
        }
        else { // AWB unlock
            m_bAWBLock = MFALSE;
            m_pAwbStateMgr->sendIntent(eIntent_AWBLockOff);
        }

        AWB_LOG("[%d] m_bAWBLock: %d\n", m_eSensorDev, m_bAWBLock);
    }

    m_rAWBOutput.rAWBInfo.bAWBLock = m_bAWBLock;
    if(m_rAWBOutput.rAWBInfo.bAWBLock)
    {
    //Assign RPG/PGN
    m_ISP_AWBInfo.rRPG.i4R = m_ISP_AWBInfo.rPGN.i4R = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
    m_ISP_AWBInfo.rRPG.i4G = m_ISP_AWBInfo.rPGN.i4G = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
    m_ISP_AWBInfo.rRPG.i4B = m_ISP_AWBInfo.rPGN.i4B = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
    setIspAwbInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::enableAWB()
{
    m_bEnableAWB = MTRUE;

    AWB_LOG("[%d] enableAWB()\n", m_eSensorDev);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::disableAWB()
{
    m_bEnableAWB = MFALSE;

    AWB_LOG("[%d] disableAWB()\n", m_eSensorDev);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::backup()
{
    mBackupGain = m_rAWBOutput.rAWBInfo.rCurrentAWBGain; //m_rAWBOutput.rPreviewAWBGain;
    //  printGain(mBackupGain);
    AWB_LOG("[%s] AWB(%d,%d,%d)", __FUNCTION__, mBackupGain.i4R, mBackupGain.i4G, mBackupGain.i4B);
    return MTRUE;
}
MRESULT AwbMgr::restore()
{
    m_rAWBOutput.rPreviewAWBGain = mBackupGain;
    m_rAWBOutput.rAWBInfo.rCurrentAWBGain = mBackupGain;
    //mBackupGain = m_rAWBOutput.rPreviewAWBGain;
    //IspTuningMgr::getInstance().setAWBGain(m_eSensorDev,mBackupGain);
    //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev,m_rAWBOutput.rAWBInfo);
    //Assign RPG
    m_ISP_AWBInfo.rRPG.i4R = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
    m_ISP_AWBInfo.rRPG.i4G = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
    m_ISP_AWBInfo.rRPG.i4B = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
    setIspAwbInfo(m_eSensorDev,m_rAWBOutput.rAWBInfo);

    //  printGain(mBackupGain);
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::setAWBStatCropRegion(MINT32 i4SensorMode, MINT32 i4CropOffsetX, MINT32 i4CropOffsetY, MINT32 i4CropRegionWidth, MINT32 i4CropRegionHeight)
{
    m_rAWBWindowConfig.bNeedCrop[i4SensorMode] = MTRUE;
    m_rAWBWindowConfig.i4CropOffsetX[i4SensorMode] = i4CropOffsetX;
    m_rAWBWindowConfig.i4CropOffsetY[i4SensorMode] = i4CropOffsetY;
    m_rAWBWindowConfig.i4CropRegionWidth[i4SensorMode] = i4CropRegionWidth;
    m_rAWBWindowConfig.i4CropRegionHeight[i4SensorMode] = i4CropRegionHeight;

    AWB_LOG("[%s()][%d] i4SensorMode = %d, bNeedCrop = %d, i4CropOffsetX = %d, i4CropOffsetY = %d, i4CropRegionWidth = %d, i4CropRegionHeight = %d",
            __FUNCTION__,
            m_eSensorDev, i4SensorMode,
            m_rAWBWindowConfig.bNeedCrop[i4SensorMode],
            m_rAWBWindowConfig.i4CropOffsetX[i4SensorMode],
            m_rAWBWindowConfig.i4CropOffsetY[i4SensorMode],
            m_rAWBWindowConfig.i4CropRegionWidth[i4SensorMode],
            m_rAWBWindowConfig.i4CropRegionHeight[i4SensorMode]);

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::doPvAWB(MINT32 i4FrameCount, MBOOL bAEStable, MINT32 i4SceneLV, MVOID *aaoBuf, MUINT32 u4ExposureTime, MBOOL bApplyToHW)
{
    (void)i4FrameCount;
    //use setprop to decide AWB is controlled by Sync AWB algo or AWB mgr
    //char value[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32 value = 0;

    getPropInt("vendor.debug.awb_mgr.lock", &m_bAdbAWBLock, 0);
    //m_bAdbAWBLock = atoi(value);
    m_i4MagicNum = i4FrameCount;

    AWB_LOG("[%s] MagicNumber %d bAEStable %d i4SceneLV %d bApplyToHW %d m_eAWBMode %d", __FUNCTION__, i4FrameCount,
            bAEStable, i4SceneLV, bApplyToHW, m_eAWBMode);

    AWB_INPUT_T rAWBInput;
    memset(&rAWBInput, 0, sizeof(rAWBInput));

    // Test MWB flow for precheck
    MINT32 m_setMWBCCT = 0;
    getPropInt("vendor.debug.awb_mgr.setMWB", &m_setMWBCCT, 0);

    if((m_setMWBCCT > 0) && (m_setMWBCCT != mMWBColorTemperature))
    {
        m_eAWBMode = LIB3A_AWB_MODE_MWB;
        mMWBColorTemperature = m_setMWBCCT;
        m_bAWBModeChanged = MTRUE;
        AWB_LOG_IF(m_i4DgbLog ,"[%s()] setMWB: m_eAWBMode = %d , mMWBColorTemperature = %d\n", __FUNCTION__, m_eAWBMode, mMWBColorTemperature);
    }
    else if(m_setMWBCCT < 0)
    {
        m_eAWBMode = LIB3A_AWB_MODE_AUTO;
        m_setMWBCCT = 0;
        m_bAWBModeChanged = MTRUE;
        property_set("vendor.debug.awb_mgr.setMWB", 0);
        AWB_LOG_IF(m_i4DgbLog ,"[%s()] setMWB to auto: m_eAWBMode = %d ,\n", __FUNCTION__, m_eAWBMode);
    }

    // FD count to determone if FD data need to be cleared
    if (bIsStartFDCnt)  // true when start
        {
          if (FDFrameCnt >= CUST_FACE_AWB_CLEAR_COUNT())
            {
             memset(&m_eAWBFDArea, 0, sizeof(AWB_FD_INFO_T));
             m_bFaceAWBAreaChage = MTRUE;
             FDFrameCnt = 0;
             bIsStartFDCnt = MFALSE;
             AWB_LOG_IF(m_i4DgbLog, "[%s] FDinfo is cleared", __FUNCTION__);
            }
          else if (m_bIsFD) // reset counter if setfd
            {
             FDFrameCnt = 0;
             m_bIsFD = MFALSE;
             AWB_LOG_IF(m_i4DgbLog, "[%s] reset FD count", __FUNCTION__);
            }
          else if (!m_bIsFD)
            {
             FDFrameCnt ++;
             AWB_LOG_IF(m_i4DgbLog, "[%s] Increase FD count", __FUNCTION__);
            }
        }

    ///AWB_LOG("doPvAWB m_eColorCorrectionMode=%d ln=%d i4SceneLV= %d", m_eColorCorrectionMode, __LINE__,i4SceneLV);
    if (m_eColorCorrectionMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX)
    {
        AWB_LOG("m_eColorCorrectionMode rgain=%d ggain=%d bgain=%d",
            m_rColorCorrectionGain.i4R,
            m_rColorCorrectionGain.i4G,
            m_rColorCorrectionGain.i4B);
        m_rAWBOutput.rAWBInfo.rCurrentAWBGain = m_rColorCorrectionGain;
        //IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rAWBInfo.rCurrentAWBGain);
        //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
        //Assign RPG/PGN
        m_ISP_AWBInfo.rRPG.i4R = m_ISP_AWBInfo.rPGN.i4R = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
        m_ISP_AWBInfo.rRPG.i4G = m_ISP_AWBInfo.rPGN.i4G = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
        m_ISP_AWBInfo.rRPG.i4B = m_ISP_AWBInfo.rPGN.i4B = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
        setIspAwbInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
    }
    else if ((m_bEnableAWB) &&
        (!m_bAWBLock) &&
        (!m_bAdbAWBLock) &&
        (m_eAWBMode != LIB3A_AWB_MODE_OFF))
    {
        AWB_LOG_IF(m_i4DgbLog ,"[%s()] m_bAWBModeChanged = %d mIsMwbCctChanged = %d\n", __FUNCTION__, m_bAWBModeChanged, mIsMwbCctChanged);
        //if ((m_bAWBModeChanged) && (!m_bInitState))
        if ((m_bAWBModeChanged) || mIsMwbCctChanged )//&& (!m_bInitState))
        {
            if(m_eAWBMode==LIB3A_AWB_MODE_MWB)
            {
                AWB_LOG_IF(m_i4DgbLog ,"[%s()] m_eAWBMode == LIB3A_AWB_MODE_MWB \n", __FUNCTION__);

                m_pIAwbAlgo->setAWBMode(m_eAWBMode);
                LIGHT_AREA_T rLightArea;
                m_pIAwbAlgo->getMWBLightArea(mMWBColorTemperature, rLightArea);

                m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4RightBound;
                m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4LeftBound;
                m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4LowerBound;
                m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4UpperBound;

                m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4RightBound;
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4LeftBound;
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4LowerBound;
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4UpperBound;

                m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode],
                                          m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
                                          m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);
                m_bAWBModeChanged = MFALSE;
                m_bOneShotAWB = MTRUE;
                //m_bSkipOneFrame = 3;
                mIsMwbCctChanged =  MFALSE;

                //update AWB statistics config
              #if 0
                ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).config(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
                , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY);
              #else
               //AWBResultConfig_T rAWBResultConfig;

               ISP_AWB_CONFIG_T::getInstance(m_eSensorDev).AWBConfig(&m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], &m_sAWBResultConfig);
               //ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).configReg(&(m_sAWBResultConfig.rAWBRegInfo));
              #endif

                // force ISP validate
                //IspTuningMgr::getInstance().forceValidate(m_eSensorDev);
            }
            else
            {
                m_pIAwbAlgo->setAWBMode(m_eAWBMode);
                m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode],
                                              m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
                                              m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);
                m_bAWBModeChanged = MFALSE;
                m_bOneShotAWB = MTRUE;
                m_bSkipOneFrame = 3;
                mIsMwbCctChanged =  MFALSE;


                //update AWB statistics config
               #if 0
                ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).config(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
                , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY);
               #else
                //AWBResultConfig_T rAWBResultConfig;

                ISP_AWB_CONFIG_T::getInstance(m_eSensorDev).AWBConfig(&m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], &m_sAWBResultConfig);
                //ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).configReg(&(m_sAWBResultConfig.rAWBRegInfo));
               #endif

                // force ISP validate
                //IspTuningMgr::getInstance().forceValidate(m_eSensorDev);
            }
        }
        else if ((m_bStrobeModeChanged) && (!m_bInitState))
        {
            AWB_LOG_IF(m_i4DgbLog, "[%s()][%d] m_bStrobeModeChanged = %d, m_i4StrobeMode = %d\n", __FUNCTION__, m_eSensorDev, m_bStrobeModeChanged, m_i4StrobeMode);
            //AWB_LOG_IF(m_i4DgbLog, "[%s()][%d] update AWB statistics config", __FUNCTION__, m_eSensorDev);
#if 0
            if (m_i4StrobeMode == AWB_STROBE_MODE_ON) {
                m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode],
                    m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
                    m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);
                // update AWB statistics config
                ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).config(m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
                    , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY);
            }
            else {
                m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode],
                    m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
                    m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);
                // update AWB statistics config
                ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).config(m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
                    , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY);
            }
#else
            m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode],
                    m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
                    m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);

             // update AWB statistics config
             //AWBResultConfig_T rAWBResultConfig;

             ISP_AWB_CONFIG_T::getInstance(m_eSensorDev).AWBConfig(&m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], &m_sAWBResultConfig);
             //ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).configReg(&(m_sAWBResultConfig.rAWBRegInfo));
#endif

            // restore for isp
            if (m_bFlashAwb){
                restore();
                m_bFlashAwb = MFALSE;
            }
            // force ISP validate
            //IspTuningMgr::getInstance().forceValidate(m_eSensorDev);

            m_bStrobeModeChanged = MFALSE;
            m_bOneShotAWB = MTRUE;
            ///m_bSkipOneFrame = 3;
            PostSem();
            return S_AWB_OK;
        }

        else if (m_bSkipOneFrame>0) { // skip one frame for AWB statistics ready
            m_bSkipOneFrame--;
        }
        else
        {
            rAWBInput.bIsStrobeFired = (m_i4StrobeMode == AWB_STROBE_MODE_ON) ? MTRUE : MFALSE;
            rAWBInput.i4SceneLV = i4SceneLV;
            rAWBInput.i4AWBState = AWB_STATE_PREVIEW;
            //rAWBInput.pAWBStatBuf = pAWBStatBuf;
            if (m_bOneShotAWB)
            {
                rAWBInput.eAWBSpeedMode = AWB_SPEED_MODE_ONESHOT;

                if (!m_bInitState) {
                    m_bOneShotAWB = MFALSE;
                }
            }
            else
            {
                rAWBInput.eAWBSpeedMode = AWB_SPEED_MODE_SMOOTH_TRANSITION;
            }

            rAWBInput.pAWBStatBuf = aaoBuf;//mpAao
            rAWBInput.i4StatLineSize = m_AAOStride;//120*4;//120*8.5;
            //rAWBInput.u4AWBAAOMode = m_i4AAOmode;
            rAWBInput.i4SensorMode = m_eSensorMode;
            rAWBInput.bAWBLock = m_bAWBLock;
            rAWBInput.u4ExposureTime = u4ExposureTime;
            rAWBInput.i4MaxFPS = m_i4MaxFPS;
            // Set FDarea to algorithm
            if (m_bFaceAWBAreaChage == MTRUE) {  // Face AWB
                m_pIAwbAlgo->setAWBFDArea(&m_eAWBFDArea);
                m_bFaceAWBAreaChage = MFALSE;
            }

            AAA_TRACE_D("CustAWB");
            CustAWB(rAWBInput, m_rAWBRAWPreGain1, m_rAWBRAWPreGain1M, m_rAWBRAWPreGain1L, rAWBInput.rCustAWBGain);
            AAA_TRACE_END_D;

            AWB_LOG_IF(m_i4DgbLog, "[%s()][%d] rAWBInput.i4StatLineSize= %d m_i4StrobeMode= %d\n", __FUNCTION__, m_eSensorDev, rAWBInput.i4StatLineSize, m_i4StrobeMode);

            AAA_TRACE_D("handleAWB");
            m_pIAwbAlgo->handleAWB(rAWBInput, m_rAWBOutput);
            AAA_TRACE_END_D;

            PostSem();

            if (m_i4StrobeMode == AWB_STROBE_MODE_ON) {
                // update AE RAW pre-gain2
                setAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);
                // IspTuningMgr::getInstance().setIspAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);

                // update AWB gain
                if (bApplyToHW)
                {
                    //  IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rPreviewStrobeAWBGain);
                    setRgbGainX(m_eSensorDev, m_rAWBOutput.rPreviewStrobeAWBGain);
            }
            }
            else {
                // update AE RAW pre-gain2
                setAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);
                // IspTuningMgr::getInstance().setIspAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);

                // update AWB gain
                if (bApplyToHW)
                {
                    //  IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rPreviewAWBGain);
                    setRgbGainX(m_eSensorDev, m_rAWBOutput.rPreviewAWBGain);
                }

            }

            // set AWB info
            if (bApplyToHW){
            //Assign RPG/PGN
            m_ISP_AWBInfo.rRPG.i4R = m_ISP_AWBInfo.rPGN.i4R = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
            m_ISP_AWBInfo.rRPG.i4G = m_ISP_AWBInfo.rPGN.i4G = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
            m_ISP_AWBInfo.rRPG.i4B = m_ISP_AWBInfo.rPGN.i4B = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
            //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
            setIspAwbInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
            }
            if (m_bInitState && bAEStable) {
                m_bInitState = MFALSE;
            }

            // update AWB state
            if (m_rAWBOutput.rAWBInfo.bAWBStable)
                m_pAwbStateMgr->sendIntent(eIntent_AWBScanFinished);
            else
                m_pAwbStateMgr->sendIntent(eIntent_AWBScan);

            // update real time AWB debug information

            AWB_LOG_IF(m_i4DgbLog,"AWB RealTimeCheck : %d %d", m_i4DgbRtvEnable, m_i4RtvGroupId);

            if (m_i4DgbRtvEnable)
            {
                char value[512] = {'\0'};
                MINT32 count;

                count = updateRTVString(value);
                if (count > 0)
                {
                    AWB_LOG_IF(m_i4DgbLog,"AWB_RTV : %s", value);
                    property_set("vendor.debug.awb_rtv.data", value );
                }
            }
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::doAFAWB(MVOID *aaoBuf, MINT32 i4SceneLV, MBOOL bApplyToHW)
{
    AWB_INPUT_T rAWBInput;
    memset(&rAWBInput, 0, sizeof(rAWBInput));

    AWB_LOG("%s(): i4SceneLV= %d", __FUNCTION__, i4SceneLV);

    if (m_eColorCorrectionMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX)
    {
        AWB_LOG("doAFAWB ln=%d", __LINE__);
        m_rAWBOutput.rAWBInfo.rCurrentAWBGain = m_rColorCorrectionGain;
        //IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rAWBInfo.rCurrentAWBGain);
        //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
        //Assign RPG/PGN
        m_ISP_AWBInfo.rRPG.i4R = m_ISP_AWBInfo.rPGN.i4R = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
        m_ISP_AWBInfo.rRPG.i4G = m_ISP_AWBInfo.rPGN.i4G = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
        m_ISP_AWBInfo.rRPG.i4B = m_ISP_AWBInfo.rPGN.i4B = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
        setIspAwbInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
    }
    else if (m_bEnableAWB && (!m_bAWBLock) && (!m_bAdbAWBLock) && (m_eAWBMode != LIB3A_AWB_MODE_OFF))
    {
        if (/*(m_i4StrobeMode == AWB_STROBE_MODE_ON) &&*/ (m_bStrobeModeChanged))
        {
            m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode],
                m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
                m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);
            m_bStrobeModeChanged = FALSE;

#if 0
            // update AWB statistics config
            ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).config(m_rAWBStatCfg[m_i4StrobeMode/*AWB_STROBE_MODE_ON*/][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
                , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY);

                ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).config(m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
                     , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY);

            // force ISP validate
            //IspTuningMgr::getInstance().forceValidate(m_eSensorDev);

#else
            // update AWB statistics config
            //AWBResultConfig_T rAWBResultConfig;

            ISP_AWB_CONFIG_T::getInstance(m_eSensorDev).AWBConfig(&m_rAWBStatCfg[m_i4StrobeMode/*AWB_STROBE_MODE_ON*/][m_eSensorMode][m_eAWBMode], &m_sAWBResultConfig);
            //ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).configReg(&(m_sAWBResultConfig.rAWBRegInfo));
#endif

            AWB_LOG("[%s()][%d] update AWB statistics config", __FUNCTION__, m_eSensorDev);
            //return S_AWB_OK;
        }

        //if (m_i4StrobeMode == AWB_STROBE_MODE_ON) {
        {
            ///rAWBInput.bIsStrobeFired = MTRUE;
            rAWBInput.bIsStrobeFired = (m_i4StrobeMode == AWB_STROBE_MODE_ON);
            rAWBInput.i4SceneLV = i4SceneLV;
            rAWBInput.i4AWBState = AWB_STATE_PREVIEW; //AWB_STATE_AF;
            rAWBInput.eAWBSpeedMode = AWB_SPEED_MODE_SMOOTH_TRANSITION; //AWB_SPEED_MODE_ONESHOT;
            rAWBInput.pAWBStatBuf = aaoBuf;//mpAao
            rAWBInput.i4StatLineSize = m_AAOStride;
            //rAWBInput.u4AWBAAOMode = m_i4AAOmode;
            rAWBInput.i4SensorMode = m_eSensorMode;
            rAWBInput.bAWBLock = m_bAWBLock;

            // Set FDarea to algorithm
            if (m_bFaceAWBAreaChage == MTRUE) {  // Face AWB
                m_pIAwbAlgo->setAWBFDArea(&m_eAWBFDArea);
                m_bFaceAWBAreaChage = MFALSE;
            }
            AAA_TRACE_D("CustAWB");
            CustAWB(rAWBInput, m_rAWBRAWPreGain1, m_rAWBRAWPreGain1M, m_rAWBRAWPreGain1L, rAWBInput.rCustAWBGain);
            AAA_TRACE_END_D;
            AAA_TRACE_D("handleAWB");
            m_pIAwbAlgo->handleAWB(rAWBInput, m_rAWBOutput);
            AAA_TRACE_END_D;
            PostSem();
        }

        // update AE RAW pre-gain2
        setAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);
        // IspTuningMgr::getInstance().setIspAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);

        // update AWB gain
        if (bApplyToHW)
        {
            //  IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rPreviewStrobeAWBGain);
            setRgbGainX(m_eSensorDev, m_rAWBOutput.rPreviewStrobeAWBGain);
        }

        // set AWB info
        if (bApplyToHW){
            //Assign RPG/PGN
            m_ISP_AWBInfo.rRPG.i4R = m_ISP_AWBInfo.rPGN.i4R = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
            m_ISP_AWBInfo.rRPG.i4G = m_ISP_AWBInfo.rPGN.i4G = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
            m_ISP_AWBInfo.rRPG.i4B = m_ISP_AWBInfo.rPGN.i4B = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
            //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
            setIspAwbInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
            }

        // update AWB state
        m_pAwbStateMgr->sendIntent(eIntent_AWBScanFinished);

    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::doPreCapAWB(MINT32 i4SceneLV, MVOID *aaoBuf, MBOOL bApplyToHW)
{
    AWB_INPUT_T rAWBInput;
    memset(&rAWBInput, 0, sizeof(rAWBInput));

    AWB_LOG(" doPreCapAWB : i4SceneLV= %d",i4SceneLV);

    if (m_eColorCorrectionMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX)
    {
        AWB_LOG("doPreCapAWB ln=%d", __LINE__);
        m_rAWBOutput.rAWBInfo.rCurrentAWBGain = m_rColorCorrectionGain;
        //IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rAWBInfo.rCurrentAWBGain);
        //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
        //Assign RPG/PGN
        m_ISP_AWBInfo.rRPG.i4R = m_ISP_AWBInfo.rPGN.i4R = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
        m_ISP_AWBInfo.rRPG.i4G = m_ISP_AWBInfo.rPGN.i4G = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
        m_ISP_AWBInfo.rRPG.i4B = m_ISP_AWBInfo.rPGN.i4B = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
        setIspAwbInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
    }
    else if (m_bEnableAWB && (((!m_bAWBLock) && (!m_bAdbAWBLock)) || (m_i4StrobeMode == AWB_STROBE_MODE_ON)) && (m_eAWBMode != LIB3A_AWB_MODE_OFF))
    {
        if(m_eAWBMode==LIB3A_AWB_MODE_MWB)
        {
                m_pIAwbAlgo->setAWBMode(m_eAWBMode);
                LIGHT_AREA_T rLightArea;
                m_pIAwbAlgo->getMWBLightArea(mMWBColorTemperature, rLightArea);


                m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4RightBound;
                m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4LeftBound;
                m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4LowerBound;
                m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4UpperBound;

                m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4RightBound;
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4LeftBound;
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4LowerBound;
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4UpperBound;
                AWB_LOG("CHKPOIT %d",__LINE__);
                m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode],
                                          m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
                                          m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);
               // update AWB statistics config
               #if 0
                ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).config(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable, mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY);
               #else
               //AWBResultConfig_T rAWBResultConfig;

               ISP_AWB_CONFIG_T::getInstance(m_eSensorDev).AWBConfig(&m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], &m_sAWBResultConfig);
               //ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).configReg(&(m_sAWBResultConfig.rAWBRegInfo));
               #endif

        }
        else if ((m_i4StrobeMode == AWB_STROBE_MODE_ON) && (m_bStrobeModeChanged) && (m_eAWBMode != LIB3A_AWB_MODE_MWB))
        {
            m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode],
                m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
                m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);
                SkipFlashFrameCnt = 2;
                AWB_LOG("[%s()]Flash On skipframe m_bSkipOneFrame 1st =%d m_i4StrobeMode = %d m_bStrobeModeChanged = %d",
                        __FUNCTION__, SkipFlashFrameCnt, m_i4StrobeMode, m_bStrobeModeChanged);
                m_bStrobeModeChanged = FALSE;

#if 0
            // update AWB statistics config
            ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).config(m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable, mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY);


            // force ISP validate
            //IspTuningMgr::getInstance().forceValidate(m_eSensorDev);
#else
            // update AWB statistics config
            //AWBResultConfig_T rAWBResultConfig;

            ISP_AWB_CONFIG_T::getInstance(m_eSensorDev).AWBConfig(&m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode], &m_sAWBResultConfig);
            //ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).configReg(&(m_sAWBResultConfig.rAWBRegInfo));

#endif
            PostSem();
            return S_AWB_OK;
        }
       #if 0
        if (SkipFlashFrameCnt > 0){
            SkipFlashFrameCnt --;
            AWB_LOG("[%s()]skipframe m_bSkipOneFrame=%d", __FUNCTION__, SkipFlashFrameCnt);
            return S_AWB_OK;
            }
        #endif
        rAWBInput.bIsStrobeFired = (m_i4StrobeMode == AWB_STROBE_MODE_ON) ? MTRUE : MFALSE;
        rAWBInput.i4SceneLV = i4SceneLV;
        rAWBInput.i4AWBState = AWB_STATE_PRECAPTURE;
        //rAWBInput.pAWBStatBuf = pAWBStatBuf;
        rAWBInput.eAWBSpeedMode = AWB_SPEED_MODE_ONESHOT;
        //rAWBInput.i4StatLineSize = m_rAWBStatParam.i4WindowNumX*5;
        rAWBInput.pAWBStatBuf = aaoBuf;//mpAao
        rAWBInput.i4StatLineSize = m_AAOStride;//120*4;//120*8.5;
        //rAWBInput.u4AWBAAOMode = m_i4AAOmode;
        rAWBInput.i4SensorMode = m_eSensorMode;
        rAWBInput.bAWBLock = m_bAWBLock;

        // Set FDarea to algorithm
        if (m_bFaceAWBAreaChage == MTRUE) {  // Face AWB
            m_pIAwbAlgo->setAWBFDArea(&m_eAWBFDArea);
            m_bFaceAWBAreaChage = MFALSE;
        }
        AAA_TRACE_D("CustAWB");
        CustAWB(rAWBInput, m_rAWBRAWPreGain1, m_rAWBRAWPreGain1M, m_rAWBRAWPreGain1L, rAWBInput.rCustAWBGain);
        AAA_TRACE_END_D;
        AAA_TRACE_D("handleAWB");
        m_pIAwbAlgo->handleAWB(rAWBInput, m_rAWBOutput);
        AAA_TRACE_END_D;
        PostSem();

        // update AE RAW pre-gain2
        setAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);
        // IspTuningMgr::getInstance().setIspAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);

        // update AWB gain
        if (bApplyToHW)
        {
            //IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rPreviewStrobeAWBGain);
            setRgbGainX(m_eSensorDev, m_rAWBOutput.rPreviewStrobeAWBGain);
        }

        // set AWB info
        if (bApplyToHW){
            //Assign RPG/PGN
            m_ISP_AWBInfo.rRPG.i4R = m_ISP_AWBInfo.rPGN.i4R = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
            m_ISP_AWBInfo.rRPG.i4G = m_ISP_AWBInfo.rPGN.i4G = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
            m_ISP_AWBInfo.rRPG.i4B = m_ISP_AWBInfo.rPGN.i4B = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
            //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
            setIspAwbInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
          }

        // update AWB state
        m_pAwbStateMgr->sendIntent(eIntent_AWBScanFinished);

    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::doCapAWB(MINT32 i4SceneLV, MVOID *aaoBuf, MBOOL bApplyToHW)
{
    AWB_INPUT_T rAWBInput;
    memset(&rAWBInput, 0, sizeof(rAWBInput));

    StatisticBufInfo *paaobuf = reinterpret_cast<StatisticBufInfo *>(aaoBuf);

    MBOOL m_bAAOMainFlashOn = (paaobuf->mFlashState == MTK_FLASH_STATE_FIRED)  ? MTRUE : MFALSE;
    MBOOL bIsFlashFired = ((m_i4StrobeMode==AWB_STROBE_MODE_ON) || m_bIsMainFlashOn);
    AWB_LOG("[%s] i4SceneLV= %d m_bAAOMainFlashOn=%d bIsFlashFired=%d m_bIsMainFlashOn=%d\n", __FUNCTION__,
        i4SceneLV, m_bAAOMainFlashOn, bIsFlashFired, m_bIsMainFlashOn);

    //char value[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32 m_setFlashwin = 0;
    getPropInt("vendor.debug.awb_mgr.setFlashwin", &m_setFlashwin, 0);


    if (m_eColorCorrectionMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX)
    {
        AWB_LOG("doCapAWB ln=%d", __LINE__);
        m_rAWBOutput.rAWBInfo.rCurrentAWBGain = m_rColorCorrectionGain;
        //IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rAWBInfo.rCurrentAWBGain);
        //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
        //Assign RPG/PGN
        m_ISP_AWBInfo.rRPG.i4R = m_ISP_AWBInfo.rPGN.i4R = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
        m_ISP_AWBInfo.rRPG.i4G = m_ISP_AWBInfo.rPGN.i4G = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
        m_ISP_AWBInfo.rRPG.i4B = m_ISP_AWBInfo.rPGN.i4B = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
        setIspAwbInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
    }
    else if (m_bEnableAWB && (((!m_bAWBLock) && (!m_bAdbAWBLock)) || (bIsFlashFired == MTRUE)) && (m_eAWBMode != LIB3A_AWB_MODE_OFF))
    {
       if(m_eAWBMode==LIB3A_AWB_MODE_MWB)
       {
                 m_pIAwbAlgo->setAWBMode(m_eAWBMode);
                 LIGHT_AREA_T rLightArea;
                 m_pIAwbAlgo->getMWBLightArea(mMWBColorTemperature, rLightArea);

                 MINT32 i4StrobeMode = (bIsFlashFired == MTRUE)? AWB_STROBE_MODE_ON: AWB_STROBE_MODE_OFF;
                m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4RightBound;
                m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4LeftBound;
                m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4LowerBound;
                m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4UpperBound;

                m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4RightBound;
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4LeftBound;
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4LowerBound;
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4UpperBound;
                 AWB_LOG(" doCapAWB mMWBColorTemperature= %d",mMWBColorTemperature);
                 m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[i4StrobeMode][m_eSensorMode][m_eAWBMode],
                              m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
                              m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);
                               // update AWB statistics config
                #if 0
                ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).config(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable, mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY);                #else
                #else
                // update AWB statistics config
                //AWBResultConfig_T rAWBResultConfig;

                ISP_AWB_CONFIG_T::getInstance(m_eSensorDev).AWBConfig(&m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], &m_sAWBResultConfig);
                //ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).configReg(&(m_sAWBResultConfig.rAWBRegInfo));
                #endif

       }
        rAWBInput.bIsStrobeFired = m_bAAOMainFlashOn;
        rAWBInput.i4SceneLV = i4SceneLV;
        rAWBInput.i4AWBState = AWB_STATE_CAPTURE;
        //rAWBInput.pAWBStatBuf = pAWBStatBuf;
        rAWBInput.eAWBSpeedMode = AWB_SPEED_MODE_ONESHOT;
        //rAWBInput.i4StatLineSize = m_rAWBStatParam.i4WindowNumX*5;

        rAWBInput.pAWBStatBuf = reinterpret_cast<MVOID *>(paaobuf->mVa);//mpAao
        rAWBInput.i4StatLineSize = m_AAOStride;//120*4;//120*8.5;
        //rAWBInput.u4AWBAAOMode = m_i4AAOmode;
        rAWBInput.i4SensorMode = m_eSensorMode;
        rAWBInput.bAWBLock = m_bAWBLock;

        // Set FDarea to algorithm
        if (m_bFaceAWBAreaChage == MTRUE) {  // Face AWB
            m_pIAwbAlgo->setAWBFDArea(&m_eAWBFDArea);
            m_bFaceAWBAreaChage = MFALSE;
        }
        AAA_TRACE_D("CustAWB");
        CustAWB(rAWBInput, m_rAWBRAWPreGain1, m_rAWBRAWPreGain1M, m_rAWBRAWPreGain1L, rAWBInput.rCustAWBGain);
        AAA_TRACE_END_D;
        AAA_TRACE_D("handleAWB");
        m_pIAwbAlgo->handleAWB(rAWBInput, m_rAWBOutput);
        AAA_TRACE_END_D;
        PostSem();

        //Assign RPG/PGN for flash 1.0
        m_ISP_AWBInfo.rRPG.i4R = m_ISP_AWBInfo.rPGN.i4R = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
        m_ISP_AWBInfo.rRPG.i4G = m_ISP_AWBInfo.rPGN.i4G = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
        m_ISP_AWBInfo.rRPG.i4B = m_ISP_AWBInfo.rPGN.i4B = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;

#if ((!USE_OPEN_SOURCE_AWB) && (!USE_OPEN_SOURCE_FLASH_AE) )
        //if((rAWBInput.bIsStrobeFired) && (!m_bIsTorch))
        if((m_bAAOMainFlashOn) && (m_eAWBMode != LIB3A_AWB_MODE_MWB))
        {
            if ((NULL == m_pNVRAM_FLASH_CALIBRATION )||(NULL == m_pNVRAM_STROBE)){
                CAM_LOGE("Flash NVRAM POINTER NULL\n");
                return MFALSE;
               }

            if(isFlashAWBv2Enabled(m_eSensorDev) &&
                (m_pNVRAM_FLASH_CALIBRATION->Flash_Calibration[m_i4FlashCaliNvramIdx].flashWBGain[m_flashDuty].i2R !=0) &&
                (m_pNVRAM_FLASH_CALIBRATION->Flash_Calibration[m_i4FlashCaliNvramIdx].flashWBGain[m_flashDuty].i2B !=0) &&
                (m_setFlashwin == 0))
            {
                FLASH_AWB_CAL_GAIN_INPUT_T FlashAwbCalGainInput;
                FLASH_AWB_OUTPUT_T FlashAwbOutput;
                //FLASH_AWB_INIT_T FlashAwbInitParam;
                //FlashAwbInitParam.flash_awb_tuning_param = m_pNVRAM_STROBE->Flash_AWB[m_i4StrobeNvramIdx].rTuningParam;

                FlashAwbCalGainInput.NoFlashWBGain = m_rAWBOutput.rPreviewAWBGain;
                FlashAwbCalGainInput.PureFlashWBGain.i4R = m_pNVRAM_FLASH_CALIBRATION->Flash_Calibration[m_i4FlashCaliNvramIdx].flashWBGain[m_flashDuty].i2R;
                FlashAwbCalGainInput.PureFlashWBGain.i4G = 512;
                FlashAwbCalGainInput.PureFlashWBGain.i4B = m_pNVRAM_FLASH_CALIBRATION->Flash_Calibration[m_i4FlashCaliNvramIdx].flashWBGain[m_flashDuty].i2B;
                FlashAwbCalGainInput.PureFlashWeight = m_flashAwbWeight;
                for (int i = 0; i < FLASH_LV_INDEX_NUM; i++)
                {
                    FlashAwbCalGainInput.FlashPreferenceGain[i].i4R = m_pNVRAM_STROBE->Flash_AWB[m_i4StrobeNvramIdx].rTuningParam.FlashPreferenceGain[i].i4R;
                    FlashAwbCalGainInput.FlashPreferenceGain[i].i4G = m_pNVRAM_STROBE->Flash_AWB[m_i4StrobeNvramIdx].rTuningParam.FlashPreferenceGain[i].i4G;
                    FlashAwbCalGainInput.FlashPreferenceGain[i].i4B = m_pNVRAM_STROBE->Flash_AWB[m_i4StrobeNvramIdx].rTuningParam.FlashPreferenceGain[i].i4B;
                }

                AWB_LOG("[%d][flashAWB 2.1] m_flashDuty=%d, m_flashAwbWeight=%d ", m_eSensorDev, m_flashDuty, m_flashAwbWeight);
                AWB_LOG("[%d][flashAWB 2.1] pureFlash Rgain=%d, Ggain=%d, Bgain=%d", m_eSensorDev, FlashAwbCalGainInput.PureFlashWBGain.i4R, FlashAwbCalGainInput.PureFlashWBGain.i4G, FlashAwbCalGainInput.PureFlashWBGain.i4B);
                AWB_LOG("[%d][flashAWB 2.1] preview Rgain=%d, Ggain=%d, Bgain=%d", m_eSensorDev, FlashAwbCalGainInput.NoFlashWBGain.i4R, FlashAwbCalGainInput.NoFlashWBGain.i4G, FlashAwbCalGainInput.NoFlashWBGain.i4B);
                //AWB_LOG("[%d][flashAWB 2.1] preview FlashPreferenceGain= (%d,  %d, %d)\n", m_eSensorDev,
                //                        m_pNVRAM_STROBE->Flash_AWB[m_i4StrobeNvramIdx].rTuningParam.FlashPreferenceGain.i4R, m_pNVRAM_STROBE->Flash_AWB[m_i4StrobeNvramIdx].rTuningParam.FlashPreferenceGain.i4G, m_pNVRAM_STROBE->Flash_AWB[m_i4StrobeNvramIdx].rTuningParam.FlashPreferenceGain.i4B);

                m_pIAwbAlgo->Flash_Awb_calResultGain(FlashAwbCalGainInput, FlashAwbOutput);

                AWB_LOG("[%d][flashAWB 2.1] In Awb_mgr.cpp - FlashAwbOutput Rgain=%d, Ggain=%d, Bgain=%d",  m_eSensorDev, FlashAwbOutput.rAWBGain.i4R,  FlashAwbOutput.rAWBGain.i4G,  FlashAwbOutput.rAWBGain.i4B);

                m_ISP_AWBInfo.rPGN.i4R = m_rAWBOutput.rCaptureAWBGain.i4R = FlashAwbOutput.rAWBGain.i4R;
                m_ISP_AWBInfo.rPGN.i4G = m_rAWBOutput.rCaptureAWBGain.i4G = FlashAwbOutput.rAWBGain.i4G;
                m_ISP_AWBInfo.rPGN.i4B = m_rAWBOutput.rCaptureAWBGain.i4B = FlashAwbOutput.rAWBGain.i4B;

                m_rAWBOutput.rAWBInfo.rCurrentAWBGain = m_rAWBOutput.rCaptureAWBGain;
                //Assign RPG
                m_ISP_AWBInfo.rRPG.i4R = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
                m_ISP_AWBInfo.rRPG.i4G = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
                m_ISP_AWBInfo.rRPG.i4B = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
                if (m_eAWBMode != LIB3A_AWB_MODE_MWB)
                m_bFlashAwb = MTRUE;
            }
        }
#endif
        // Debug
        //m_rAWBOutput.rCaptureAWBGain.i4R = 512;
        //m_rAWBOutput.rCaptureAWBGain.i4G = 1;
        //m_rAWBOutput.rCaptureAWBGain.i4B = 1;

        // update AWB gain
        if (bApplyToHW)
        {
            //  IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rCaptureAWBGain);
            setRgbGainX(m_eSensorDev, m_rAWBOutput.rCaptureAWBGain);
        }


        // set AWB info
        if (bApplyToHW)
            //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
            setIspAwbInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);

    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::doCapFlashAWBDisplay(MBOOL bApplyToHW)
{
    AWB_LOG("[%s()][%d]\n", __FUNCTION__, m_eSensorDev);

    //char value[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32 m_setFlashwin = 0;
    getPropInt("vendor.debug.awb_mgr.setFlashwin", &m_setFlashwin, 0);


    if (m_bEnableAWB && (m_eAWBMode != LIB3A_AWB_MODE_OFF) && (m_i4StrobeMode == AWB_STROBE_MODE_ON))
    {
#if ((!USE_OPEN_SOURCE_AWB) && (!USE_OPEN_SOURCE_FLASH_AE) )
    if(isFlashAWBv2Enabled(m_eSensorDev) &&
        (m_pNVRAM_FLASH_CALIBRATION->Flash_Calibration[m_i4FlashCaliNvramIdx].flashWBGain[m_flashDuty].i2R !=0) &&
        (m_pNVRAM_FLASH_CALIBRATION->Flash_Calibration[m_i4FlashCaliNvramIdx].flashWBGain[m_flashDuty].i2B !=0) &&
        (m_setFlashwin ==0))
    {
        FLASH_AWB_CAL_GAIN_INPUT_T FlashAwbCalGainInput;
        FLASH_AWB_OUTPUT_T FlashAwbOutput;
        //FLASH_AWB_INIT_T FlashAwbInitParam;
        //FlashAwbInitParam.flash_awb_tuning_param = m_pNVRAM_STROBE->Flash_AWB[m_i4StrobeNvramIdx].rTuningParam;

        FlashAwbCalGainInput.NoFlashWBGain = m_rAWBOutput.rPreviewAWBGain;
        FlashAwbCalGainInput.PureFlashWBGain.i4R = m_pNVRAM_FLASH_CALIBRATION->Flash_Calibration[m_i4FlashCaliNvramIdx].flashWBGain[m_flashDuty].i2R;
        FlashAwbCalGainInput.PureFlashWBGain.i4G = 512;
        FlashAwbCalGainInput.PureFlashWBGain.i4B = m_pNVRAM_FLASH_CALIBRATION->Flash_Calibration[m_i4FlashCaliNvramIdx].flashWBGain[m_flashDuty].i2B;
        FlashAwbCalGainInput.PureFlashWeight = m_flashAwbWeight;
        for (int i = 0; i < FLASH_LV_INDEX_NUM; i++)
        {
            FlashAwbCalGainInput.FlashPreferenceGain[i].i4R = m_pNVRAM_STROBE->Flash_AWB[m_i4StrobeNvramIdx].rTuningParam.FlashPreferenceGain[i].i4R;
            FlashAwbCalGainInput.FlashPreferenceGain[i].i4G = m_pNVRAM_STROBE->Flash_AWB[m_i4StrobeNvramIdx].rTuningParam.FlashPreferenceGain[i].i4G;
            FlashAwbCalGainInput.FlashPreferenceGain[i].i4B = m_pNVRAM_STROBE->Flash_AWB[m_i4StrobeNvramIdx].rTuningParam.FlashPreferenceGain[i].i4B;
        }
        AWB_LOG("[%d][flashAWB 2.1] m_flashDuty=%d, m_flashAwbWeight=%d ", m_eSensorDev, m_flashDuty, m_flashAwbWeight);
        AWB_LOG("[%d][flashAWB 2.1] pureFlash Rgain=%d, Ggain=%d, Bgain=%d", m_eSensorDev, FlashAwbCalGainInput.PureFlashWBGain.i4R, FlashAwbCalGainInput.PureFlashWBGain.i4G, FlashAwbCalGainInput.PureFlashWBGain.i4B);
        AWB_LOG("[%d][flashAWB 2.1] preview Rgain=%d, Ggain=%d, Bgain=%d", m_eSensorDev, FlashAwbCalGainInput.NoFlashWBGain.i4R, FlashAwbCalGainInput.NoFlashWBGain.i4G, FlashAwbCalGainInput.NoFlashWBGain.i4B);
        //AWB_LOG("[%d][flashAWB 2.1] preview FlashPreferenceGain= (%d,  %d, %d)\n", m_eSensorDev,
        //                        m_pNVRAM_STROBE->Flash_AWB[m_i4StrobeNvramIdx].rTuningParam.FlashPreferenceGain.i4R, m_pNVRAM_STROBE->Flash_AWB[m_i4StrobeNvramIdx].rTuningParam.FlashPreferenceGain.i4G, m_pNVRAM_STROBE->Flash_AWB[m_i4StrobeNvramIdx].rTuningParam.FlashPreferenceGain.i4B);

        m_pIAwbAlgo->Flash_Awb_calResultGain(FlashAwbCalGainInput, FlashAwbOutput);

        AWB_LOG("[%d][flashAWB 2.1] In Awb_mgr.cpp - FlashAwbOutput Rgain=%d, Ggain=%d, Bgain=%d",  m_eSensorDev, FlashAwbOutput.rAWBGain.i4R,  FlashAwbOutput.rAWBGain.i4G,  FlashAwbOutput.rAWBGain.i4B);

        m_rAWBOutput.rCaptureAWBGain.i4R = FlashAwbOutput.rAWBGain.i4R;
        m_rAWBOutput.rCaptureAWBGain.i4G = FlashAwbOutput.rAWBGain.i4G;
        m_rAWBOutput.rCaptureAWBGain.i4B = FlashAwbOutput.rAWBGain.i4B;

        m_rAWBOutput.rAWBInfo.rCurrentAWBGain = m_rAWBOutput.rCaptureAWBGain;
    }
#endif
    // Debug
    //m_rAWBOutput.rCaptureAWBGain.i4R = 512;
    //m_rAWBOutput.rCaptureAWBGain.i4G = 1;
    //m_rAWBOutput.rCaptureAWBGain.i4B = 1;
    // update AWB gain
    if (bApplyToHW)
    {
        //  IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rCaptureAWBGain);
        setRgbGainX(m_eSensorDev, m_rAWBOutput.rCaptureAWBGain);
    }

    // set AWB info
    if (bApplyToHW){
        m_ISP_AWBInfo.rRPG.i4R = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
        m_ISP_AWBInfo.rRPG.i4G = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
        m_ISP_AWBInfo.rRPG.i4B = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
        //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
        setIspAwbInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::applyAWB(AWB_SYNC_OUTPUT_N3D_T &rAWBSyncOutput)
{
    m_rAWBOutput.rAWBInfo.rCurrentAWBGain = rAWBSyncOutput.rAWBGain;
    m_rAWBOutput.rAWBInfo.i4CCT = rAWBSyncOutput.i4CCT;

    // update AWB gain
    //  IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rAWBInfo.rCurrentAWBGain);
    setRgbGainX(m_eSensorDev, m_rAWBOutput.rAWBInfo.rCurrentAWBGain);

    // set AWB info: FIXME: debug info update?
    //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
    m_ISP_AWBInfo.rRPG.i4R = m_ISP_AWBInfo.rPGN.i4R = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
    m_ISP_AWBInfo.rRPG.i4G = m_ISP_AWBInfo.rPGN.i4G = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
    m_ISP_AWBInfo.rRPG.i4B = m_ISP_AWBInfo.rPGN.i4B = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
    setIspAwbInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::getDebugInfo(AWB_DEBUG_INFO_T &rAWBDebugInfo, AWB_DEBUG_DATA_T &rAWBDebugData)
{
    m_pIAwbAlgo->getDebugInfo(rAWBDebugInfo, rAWBDebugData);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AwbMgr::getAWBCCT()
{
    return m_rAWBOutput.rAWBInfo.i4CCT;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::getASDInfo(AWB_ASD_INFO_T &a_rAWBASDInfo)
{
    m_pIAwbAlgo->getASDInfo(a_rAWBASDInfo);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::getAWBOutput(AWB_OUTPUT_T &a_rAWBOutput)
{
    a_rAWBOutput = m_rAWBOutput;

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::getAWBGain(AWB_GAIN_T& rAwbGain, MINT32& i4ScaleUnit) const
{
    //AWB_LOG("%s()\n", __FUNCTION__);

    rAwbGain = m_rAWBOutput.rAWBInfo.rCurrentAWBGain;
    i4ScaleUnit = m_rAWBInitInput.rAWBNVRAM.rAlgoTuningParam.rChipParam.i4AWBGainOutputScaleUnit;

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MRESULT AwbMgr::getAWBParentStat(AWB_PARENT_BLK_STAT_T *a_rAWBParentState,  MINT32 *a_ParentBlkNumX, MINT32 *a_ParentBlkNumY)
{
        m_pIAwbAlgo->getAWBParentStat(a_rAWBParentState, a_ParentBlkNumX, a_ParentBlkNumY);


        return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*MBOOL AwbMgr::getInitInputParam(SYNC_CTC_INIT_PARAM &CCTInitParam, NORMAL_GAIN_INIT_INPUT_PARAM &NoramlGainInitParam)
{
    if  (!m_pNVRAM_3A) {
        CAM_LOGE("m_pNVRAM_3A = NULL");
        return MFALSE;
    }

    CCTInitParam.i4CCT[eHorizon_Light] = m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCCTEstimation.i4CCT[eHorizon_Light];
    CCTInitParam.i4CCT[eA_Light] = m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCCTEstimation.i4CCT[eA_Light];
    CCTInitParam.i4CCT[eTL84_Light] = m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCCTEstimation.i4CCT[eTL84_Light];
    CCTInitParam.i4CCT[eDNP_Light] = m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCCTEstimation.i4CCT[eDNP_Light];
    CCTInitParam.i4CCT[eD65_Light] = m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCCTEstimation.i4CCT[eD65_Light];

    NoramlGainInitParam.NormalWB_Gain[eHorizon_Light] = m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rHorizon;
    NoramlGainInitParam.NormalWB_Gain[eA_Light] = m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rA;
    NoramlGainInitParam.NormalWB_Gain[eTL84_Light] = m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rTL84;
    NoramlGainInitParam.NormalWB_Gain[eDNP_Light] = m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rDNP;
    NoramlGainInitParam.NormalWB_Gain[eD65_Light] = m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rD65;

    return MTRUE;
}*/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::getSensorResolution()
{
    ///AWB_LOG("[%s()][%d]\n", __FUNCTION__, m_eSensorDev);

    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    SensorStaticInfo rSensorStaticInfo;
    pIHalSensorList->querySensorStaticInfo(m_eSensorDev, &rSensorStaticInfo);

    /*
    m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Preview] = rSensorStaticInfo.previewWidth;
    m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Preview] = rSensorStaticInfo.previewHeight;
    m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Capture] = rSensorStaticInfo.captureWidth;
    m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Capture] = rSensorStaticInfo.captureHeight;
    m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Video] = rSensorStaticInfo.videoWidth;
    m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Video] = rSensorStaticInfo.videoHeight;
    m_rAWBWindowConfig.i4SensorWidth[ESensorMode_SlimVideo1] = rSensorStaticInfo.video1Width;
    m_rAWBWindowConfig.i4SensorHeight[ESensorMode_SlimVideo1] = rSensorStaticInfo.video1Height;
    m_rAWBWindowConfig.i4SensorWidth[ESensorMode_SlimVideo2] = rSensorStaticInfo.video2Width;
    m_rAWBWindowConfig.i4SensorHeight[ESensorMode_SlimVideo2] = rSensorStaticInfo.video2Height;
    m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom1] = rSensorStaticInfo.SensorCustom1Width;
    m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom1] = rSensorStaticInfo.SensorCustom1Height;
    m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom2] = rSensorStaticInfo.SensorCustom2Width;
    m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom2] = rSensorStaticInfo.SensorCustom2Height;
    m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom3] = rSensorStaticInfo.SensorCustom3Width;
    m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom3] = rSensorStaticInfo.SensorCustom3Height;
    m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom4] = rSensorStaticInfo.SensorCustom4Width;
    m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom4] = rSensorStaticInfo.SensorCustom4Height;
    m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom5] = rSensorStaticInfo.SensorCustom5Width;
    m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom5] = rSensorStaticInfo.SensorCustom5Height;
    */

    m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Preview]    = mAaoW;
    m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Preview]   = mAaoH;
    m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Capture]    = mAaoW;
    m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Capture]   = mAaoH;
    m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Video]      = mAaoW;
    m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Video]     = mAaoH;
    m_rAWBWindowConfig.i4SensorWidth[ESensorMode_SlimVideo1] = mAaoW;
    m_rAWBWindowConfig.i4SensorHeight[ESensorMode_SlimVideo1]= mAaoH;
    m_rAWBWindowConfig.i4SensorWidth[ESensorMode_SlimVideo2] = mAaoW;
    m_rAWBWindowConfig.i4SensorHeight[ESensorMode_SlimVideo2]= mAaoH;
    m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom1]    = mAaoW;
    m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom1]   = mAaoH;
    m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom2]    = mAaoW;
    m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom2]   = mAaoH;
    m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom3]    = mAaoW;
    m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom3]   = mAaoH;
    m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom4]    = mAaoW;
    m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom4]   = mAaoH;
    m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom5]    = mAaoW;
    m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom5]   = mAaoH;

    AWB_LOG("[%s]i4SensorWidth/i4SensorHeight [ESensorMode_Preview] = %d/%d\n", __FUNCTION__,
        m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Preview], m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Preview]);
#if 0
    AWB_LOG_IF(m_i4DgbLog, "i4SensorHeight[ESensorMode_Preview] = %d\n", m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Preview]);
    AWB_LOG_IF(m_i4DgbLog, "i4SensorWidth[ESensorMode_Capture] = %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Capture]);
    AWB_LOG_IF(m_i4DgbLog, "i4SensorHeight[ESensorMode_Capture] = %d\n", m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Capture]);
    AWB_LOG_IF(m_i4DgbLog, "i4SensorWidth[ESensorMode_Video] = %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Video]);
    AWB_LOG_IF(m_i4DgbLog, "i4SensorHeight[ESensorMode_Video] = %d\n", m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Video]);
    AWB_LOG_IF(m_i4DgbLog, "i4SensorWidth[ESensorMode_SlimVideo1] = %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_SlimVideo1]);
    AWB_LOG_IF(m_i4DgbLog, "i4SensorHeight[ESensorMode_SlimVideo1] = %d\n", m_rAWBWindowConfig.i4SensorHeight[ESensorMode_SlimVideo1]);
    AWB_LOG_IF(m_i4DgbLog, "i4SensorWidth[ESensorMode_SlimVideo2] = %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_SlimVideo2]);
    AWB_LOG_IF(m_i4DgbLog, "i4SensorHeight[ESensorMode_SlimVideo2] = %d\n", m_rAWBWindowConfig.i4SensorHeight[ESensorMode_SlimVideo2]);
    AWB_LOG_IF(m_i4DgbLog, "i4SensorWidth[ESensorMode_Custom1] = %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom1]);
    AWB_LOG_IF(m_i4DgbLog, "i4SensorHeight[ESensorMode_Custom1] = %d\n", m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom1]);
    AWB_LOG_IF(m_i4DgbLog, "i4SensorWidth[ESensorMode_Custom2] = %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom2]);
    AWB_LOG_IF(m_i4DgbLog, "i4SensorHeight[ESensorMode_Custom2] = %d\n", m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom2]);
    AWB_LOG_IF(m_i4DgbLog, "i4SensorWidth[ESensorMode_Custom3] = %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom3]);
    AWB_LOG_IF(m_i4DgbLog, "i4SensorHeight[ESensorMode_Custom3] = %d\n", m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom3]);
    AWB_LOG_IF(m_i4DgbLog, "i4SensorWidth[ESensorMode_Custom4] = %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom4]);
    AWB_LOG_IF(m_i4DgbLog, "i4SensorHeight[ESensorMode_Custom4] = %d\n", m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom4]);
    AWB_LOG_IF(m_i4DgbLog, "i4SensorWidth[ESensorMode_Custom5] = %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom5]);
    AWB_LOG_IF(m_i4DgbLog, "i4SensorHeight[ESensorMode_Custom5] = %d\n", m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom5]);
#else
    AWB_LOG_IF(m_i4DgbLog, "[ESensorMode_Capture] = %d / %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Capture],m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Capture]);
    AWB_LOG_IF(m_i4DgbLog, "[ESensorMode_Video] = %d / %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Video], m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Video]);
    AWB_LOG_IF(m_i4DgbLog, "[ESensorMode_SlimVideo1] = %d / %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_SlimVideo1], m_rAWBWindowConfig.i4SensorHeight[ESensorMode_SlimVideo1]);
    AWB_LOG_IF(m_i4DgbLog, "[ESensorMode_SlimVideo2] = %d / %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_SlimVideo2], m_rAWBWindowConfig.i4SensorHeight[ESensorMode_SlimVideo2]);
    AWB_LOG_IF(m_i4DgbLog, "[ESensorMode_Custom1] = %d / %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom1], m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom1]);
    AWB_LOG_IF(m_i4DgbLog, "[ESensorMode_Custom2] = %d / %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom2], m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom2]);
    AWB_LOG_IF(m_i4DgbLog, "[ESensorMode_Custom3] = %d / %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom3], m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom3]);
    AWB_LOG_IF(m_i4DgbLog, "[ESensorMode_Custom4] = %d / %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom4], m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom4]);
    AWB_LOG_IF(m_i4DgbLog, "[ESensorMode_Custom5] = %d / %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom5], m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom5]);

#endif

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::getNvramData()
{
    AWB_LOG_IF(m_i4DgbLog,"[%s()][%d]\n", __FUNCTION__, m_eSensorDev);

    // Get default AWB calibration data
    int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_eSensorDev, (void*&)m_pNVRAM_3A);
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_FLASH_CALIBRATION, m_eSensorDev, (void*&)m_pNVRAM_FLASH_CALIBRATION);
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_STROBE, m_eSensorDev, (void*&)m_pNVRAM_STROBE);
    if(err!=0)
    {
        CAM_LOGE("getNvramData getBufAndRead fail\n");
        return E_AWB_NVRAM_ERROR;
    }

#if 1
    AWB_LOG_IF(m_i4DgbLog,"sizeof(m_pNVRAM_3A->SensorId) = %d\n", (int)sizeof(m_pNVRAM_3A->SensorId));
    AWB_LOG_IF(m_i4DgbLog,"sizeof(m_pNVRAM_3A->rAENVRAM) = %d\n", (int)sizeof(m_pNVRAM_3A->AE));
    AWB_LOG_IF(m_i4DgbLog,"sizeof(m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx]) x 2 = %d\n", (int)sizeof(m_pNVRAM_3A->AWB[m_i4AWBNvramIdx]));
    AWB_LOG_IF(m_i4DgbLog,"sizeof(m_pNVRAM_3A) = %d\n", (int)sizeof(*m_pNVRAM_3A));
#endif


    /*
    // Get default AWB calibration data
    if (FAILED(NvramDrvMgr::getInstance().init(m_eSensorDev))) {
    CAM_LOGE("NvramDrvMgr init fail\n");
    return E_AWB_SENSOR_ERROR;
    }

    NvramDrvMgr::getInstance().getRefBuf(m_pNVRAM_3A);
    NvramDrvMgr::getInstance().uninit();

    #if 1
    AWB_LOG("sizeof(m_pNVRAM_3A->u4Version) = %d\n", sizeof(m_pNVRAM_3A->u4Version));
    AWB_LOG("sizeof(m_pNVRAM_3A->SensorId) = %d\n", sizeof(m_pNVRAM_3A->SensorId));
    AWB_LOG("sizeof(m_pNVRAM_3A->rAENVRAM) = %d\n", sizeof(m_pNVRAM_3A->rAENVRAM));
    AWB_LOG("sizeof(m_pNVRAM_3A->rAWBNVRAM) = %d\n", sizeof(m_pNVRAM_3A->rAWBNVRAM));
    AWB_LOG("sizeof(m_pNVRAM_3A->rFlashAWBNVRAM) = %d\n", sizeof(m_pNVRAM_3A->rFlashAWBNVRAM));
    #endif
    */
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

AWB_GAIN_T rgb2Gain512(double r, double g, double b)
{
    AWB_GAIN_T ret;
    double maxV;
    maxV = r;
    if(maxV<g)
        maxV=g;
    if(maxV<b)
        maxV=b;
    ret.i4R=512*maxV/r;
    ret.i4G=512*maxV/g;
    ret.i4B=512*maxV/b;
    return ret;
}
MBOOL AwbMgr::getEEPROMData()
{
    AWB_LOG("[%s()][%d]\n", __FUNCTION__, m_eSensorDev);

    MUINT32 result=0;
    CAM_CAL_DATA_STRUCT GetCamCalData;
    CamCalDrvBase *pCamCalDrvObj = CamCalDrvBase::createInstance();
    MINT32 i4SensorDevID;

    switch (m_eSensorDev)
    {
    case ESensorDev_Main:
        i4SensorDevID = SENSOR_DEV_MAIN;
        break;
    case ESensorDev_Sub:
        i4SensorDevID = SENSOR_DEV_SUB;
        break;
    case ESensorDev_MainSecond:
        i4SensorDevID = SENSOR_DEV_MAIN_2;
        break;
    case ESensorDev_Main3D:
        i4SensorDevID = SENSOR_DEV_MAIN_3D;
        return S_AWB_OK;
    case ESensorDev_SubSecond:
        i4SensorDevID = SENSOR_DEV_SUB_2;
        break;
    default:
        i4SensorDevID = SENSOR_DEV_NONE;
        return S_AWB_OK;
    }

#if 0
    CAMERA_CAM_CAL_TYPE_ENUM enCamCalEnum=CAMERA_CAM_CAL_DATA_MODULE_VERSION;
    result= m_pCamCalDrvObj->GetCamCalCalData(i4SensorDevID, enCamCalEnum, (void *)&GetCamCalData);
    AWB_LOG("(0x%8x)=m_pCamCalDrvObj->GetCamCalCalData", result);
    if(result&CamCalReturnErr[enCamCalEnum])
    {
        AWB_LOG("err (%s)", CamCalErrString[enCamCalEnum]);
    }
    else
    {
        AWB_LOG("NO err (%s)", CamCalErrString[enCamCalEnum]);
    }
    enCamCalEnum = CAMERA_CAM_CAL_DATA_PART_NUMBER;
    result= m_pCamCalDrvObj->GetCamCalCalData(i4SensorDevID, enCamCalEnum, (void *)&GetCamCalData);
    AWB_LOG("(0x%8x)=m_pCamCalDrvObj->GetCamCalCalData", result);
    if(result&CamCalReturnErr[enCamCalEnum])
    {
        AWB_LOG("err (%s)", CamCalErrString[enCamCalEnum]);
    }
    else
    {
        AWB_LOG("NO err (%s)", CamCalErrString[enCamCalEnum]);
    }
#endif

    CAMERA_CAM_CAL_TYPE_ENUM enCamCalEnum = CAMERA_CAM_CAL_DATA_3A_GAIN;
    result= pCamCalDrvObj->GetCamCalCalData(i4SensorDevID, enCamCalEnum, (void *)&GetCamCalData);
    AWB_LOG("(0x%8x)=pCamCalDrvObj->GetCamCalCalData", result);


    mGainSetNum = 0;

    if (result&CamCalReturnErr[enCamCalEnum])
    {
        AWB_LOG("err (%s)", CamCalErrString[enCamCalEnum]);
    }
    else
    {
        AWB_LOG("NO err (%s)", CamCalErrString[enCamCalEnum]);
        m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4R);
        m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4G);
        m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4B);
        m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4R);
        m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4G);
        m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4B);
/*
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenValue.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldenR);
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenValue.i4Gr = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldenGr);
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenValue.i4Gb = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldenGb);
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenValue.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldenB);
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitValue.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rValueR);
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitValue.i4Gr = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rValueGr);
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitValue.i4Gb = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rValueGb);
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitValue.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rValueB);
*/
        mGoldenGain2.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4R);
        mGoldenGain2.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4G);
        mGoldenGain2.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4B);
        mUnitGain2.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4R);
        mUnitGain2.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4G);
        mUnitGain2.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4B);
        mGoldenGain3.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4R);
        mGoldenGain3.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4G);
        mGoldenGain3.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4B);
        mUnitGain3.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4R);
        mUnitGain3.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4G);
        mUnitGain3.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4B);
        mGainSetNum = 1;

    if(GetCamCalData.Single2A.S2aAwb.rGainSetNum == 2)
     {
        mGoldenGain2.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4R_low);
        mGoldenGain2.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4G_low);
        mGoldenGain2.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4B_low);
        mUnitGain2.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4R_low);
        mUnitGain2.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4G_low);
        mUnitGain2.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4B_low);
        mGainSetNum = 2;
    }

   if(GetCamCalData.Single2A.S2aAwb.rGainSetNum == 3)
    {
       mGoldenGain2.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4R_mid);
       mGoldenGain2.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4G_mid);
       mGoldenGain2.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4B_mid);
       mUnitGain2.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4R_mid);
       mUnitGain2.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4G_mid);
       mUnitGain2.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4B_mid);
       mGoldenGain3.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4R_low);
       mGoldenGain3.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4G_low);
       mGoldenGain3.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4B_low);
       mUnitGain3.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4R_low);
       mUnitGain3.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4G_low);
       mUnitGain3.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4B_low);
       mGainSetNum = 3;
   }
 }

    // Test OTP flow for precheck
    //char value[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32 m_setOTP = 0;
    getPropInt("vendor.debug.awb_mgr.setOTP", &m_setOTP, 0);

    if(m_setOTP == 1)
    {
        getPropInt("vendor.debug.awb_mgr.setOTPgainR", &m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4R, 0);
        //m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4R = atoi(value);
        getPropInt("vendor.debug.awb_mgr.setOTPgainG", &m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4G, 0);
        //m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4G = atoi(value);
        getPropInt("vendor.debug.awb_mgr.setOTPgainB", &m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4B, 0);
        //m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4B = atoi(value);

        getPropInt("vendor.debug.awb_mgr.setOTPgainR_mid", &mUnitGain2.i4R, 0);
        //mUnitGain2.i4R = atoi(value);
        getPropInt("vendor.debug.awb_mgr.setOTPgainG_mid", &mUnitGain2.i4G, 0);
        //mUnitGain2.i4G = atoi(value);
        getPropInt("vendor.debug.awb_mgr.setOTPgainB_mid", &mUnitGain2.i4B, 0);
        //mUnitGain2.i4B = atoi(value);

        getPropInt("vendor.debug.awb_mgr.setOTPgainR_low", &mUnitGain3.i4R, 0);
        //mUnitGain3.i4R = atoi(value);
        getPropInt("vendor.debug.awb_mgr.setOTPgainG_low", &mUnitGain3.i4G, 0);
        //mUnitGain3.i4G = atoi(value);
        getPropInt("vendor.debug.awb_mgr.setOTPgainB_low", &mUnitGain3.i4B, 0);
        //mUnitGain3.i4B = atoi(value);

        AWB_LOG_IF(m_i4DgbLog ,"[%s()][%d] setOTP: mUnitGain(high) %d/%d/%d  mUnitGain2(mid) %d/%d/%d  mUnitGain3(low) %d/%d/%d \n", __FUNCTION__, m_eSensorDev,
                   m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4R,m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4G,
                   m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4B,
                   mUnitGain2.i4R,mUnitGain2.i4G,mUnitGain2.i4B,mUnitGain3.i4R,mUnitGain3.i4G,mUnitGain3.i4B);
    }

    AWB_LOG("[%s()][%d]m_pNVRAM_3A->AWB[%d].rCalData.rGoldenGain.i4R/G/B = %d/%d/%d, rUnitGain.i4R/G/B = %d/%d/%d\n", __FUNCTION__, m_eSensorDev, m_i4AWBNvramIdx,
       m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4R,
       m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4G,
       m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4B,
       m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4R,
       m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4G,
       m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4B);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::queryHBinInfo()
{
#if 0 //Fix ME
    INormalPipe* pPipe = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe(m_i4SensorIdx,"awb_mgr");//iopipe2.0
    NormalPipe_HBIN_Info rNormalPipe_HBIN_Info;

    // query HBIN info
    if (MFALSE == pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_GET_HBIN_INFO, m_i4SensorIdx, (MINTPTR)&rNormalPipe_HBIN_Info, MNULL))
    {
        // Error Handling
        CAM_LOGE("EPIPECmd_GET_HBIN_INFO fail");
        pPipe->destroyInstance("awb_mgr");
        return MFALSE;
    }

    m_bHBIN2Enable = rNormalPipe_HBIN_Info.mEnabled;

    AWB_LOG("m_bHBIN2Enable = %d\n", m_bHBIN2Enable);

    pPipe->destroyInstance("awb_mgr");
#endif

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::AWBWindowConfig()
{
    MINT32 sensorMode = ESensorMode_Preview;
    for (sensorMode = ESensorMode_Preview; sensorMode < ESensorMode_NUM; sensorMode++)
    {
        // Window number: 1x1 up to 128x128
        m_rAWBWindowConfig.i4NumX[sensorMode] = m_rAWBStatParam.i4WindowNumX;
        m_rAWBWindowConfig.i4NumY[sensorMode] = m_rAWBStatParam.i4WindowNumY;

        // Window pitch
        m_rAWBWindowConfig.i4PitchX[sensorMode] = m_rAWBWindowConfig.i4SensorWidth[sensorMode] / m_rAWBWindowConfig.i4NumX[sensorMode];
        m_rAWBWindowConfig.i4PitchY[sensorMode] = m_rAWBWindowConfig.i4SensorHeight[sensorMode] / m_rAWBWindowConfig.i4NumY[sensorMode];

        // Window size: minimum horizontal size = 4, minimum vertical size = 2
        m_rAWBWindowConfig.i4SizeX[sensorMode] = (m_rAWBWindowConfig.i4PitchX[sensorMode] / 2) * 2; // even number
        m_rAWBWindowConfig.i4SizeY[sensorMode] = (m_rAWBWindowConfig.i4PitchY[sensorMode] / 2) * 2; // even number

        if (m_rAWBWindowConfig.i4SizeX[sensorMode] < 4) {
            AWB_LOG("[%d][AWBWindowConfig] i4SizeX < 4: %d\n", m_eSensorDev, m_rAWBWindowConfig.i4SizeX[sensorMode]);
            m_rAWBWindowConfig.i4SizeX[sensorMode] = 4;
        }

        if (m_rAWBWindowConfig.i4SizeY[sensorMode] < 2) {
            AWB_LOG("[%d][AWBWindowConfig] i4SizeY < 2: %d\n", m_eSensorDev, m_rAWBWindowConfig.i4SizeY[sensorMode]);
            m_rAWBWindowConfig.i4SizeY[sensorMode] = 2;
        }

        // Window origin
        m_rAWBWindowConfig.i4OriginX[sensorMode] = (m_rAWBWindowConfig.i4SensorWidth[sensorMode] - (m_rAWBWindowConfig.i4PitchX[sensorMode] * m_rAWBWindowConfig.i4NumX[sensorMode])) / 2;
        m_rAWBWindowConfig.i4OriginY[sensorMode] = (m_rAWBWindowConfig.i4SensorHeight[sensorMode] - (m_rAWBWindowConfig.i4PitchY[sensorMode] * m_rAWBWindowConfig.i4NumY[sensorMode])) / 2;

        //HORG = even for over exposure cnt > 0
        m_rAWBWindowConfig.i4OriginY[sensorMode] = (m_rAWBWindowConfig.i4OriginY[sensorMode] / 2) *2;
        AWB_LOG_IF(m_i4DgbLog, "[%s][%d] Sensor mode: %d, i4SensorWidth: %d i4SensorHeight: %d, i4NumX: %d i4NumY: %d, i4SizeX: %d i4SizeY: %d, i4PitchX: %d i4PitchY: %d, i4OriginX: %d i4OriginY: %d\n",
                 __FUNCTION__, m_eSensorDev, sensorMode,
                 m_rAWBWindowConfig.i4SensorWidth[sensorMode], m_rAWBWindowConfig.i4SensorHeight[sensorMode],
                 m_rAWBWindowConfig.i4NumX[sensorMode], m_rAWBWindowConfig.i4NumY[sensorMode],
                 m_rAWBWindowConfig.i4SizeX[sensorMode], m_rAWBWindowConfig.i4SizeY[sensorMode],
                 m_rAWBWindowConfig.i4PitchX[sensorMode], m_rAWBWindowConfig.i4PitchY[sensorMode],
                 m_rAWBWindowConfig.i4OriginX[sensorMode], m_rAWBWindowConfig.i4OriginY[sensorMode]);
    }

       AWB_LOG("[%s][%d] Sensor mode: %d, i4SensorWidth: %d i4SensorHeight: %d, i4NumX: %d i4NumY: %d, i4SizeX: %d i4SizeY: %d, i4PitchX: %d i4PitchY: %d, i4OriginX: %d i4OriginY: %d\n",
                 __FUNCTION__, m_eSensorDev, ESensorMode_Preview,
                 m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Preview], m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Preview],
                 m_rAWBWindowConfig.i4NumX[ESensorMode_Preview], m_rAWBWindowConfig.i4NumY[ESensorMode_Preview],
                 m_rAWBWindowConfig.i4SizeX[ESensorMode_Preview], m_rAWBWindowConfig.i4SizeY[ESensorMode_Preview],
                 m_rAWBWindowConfig.i4PitchX[ESensorMode_Preview], m_rAWBWindowConfig.i4PitchY[ESensorMode_Preview],
                 m_rAWBWindowConfig.i4OriginX[ESensorMode_Preview], m_rAWBWindowConfig.i4OriginY[ESensorMode_Preview]);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::AWBStatConfig()
{
    MINT32 sensorMode, awbMode;

    // Check strobe win config for precheck
    //char value[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32 m_setFlashwin = 0;
    getPropInt("vendor.debug.awb_mgr.setFlashwin", &m_setFlashwin, 0);

    // STROBE ON
    for (sensorMode = ESensorMode_Preview; sensorMode < ESensorMode_NUM; sensorMode++)
    {
        MINT32 i4WindowPixelNumR = (m_rAWBWindowConfig.i4SizeX[sensorMode] * m_rAWBWindowConfig.i4SizeY[sensorMode]) / 4;
        MINT32 i4WindowPixelNumG = i4WindowPixelNumR * 2;
        MINT32 i4WindowPixelNumB = i4WindowPixelNumR;
        MINT32 i4PixelCountR = ((1 << 24) + (i4WindowPixelNumR >> 1)) / i4WindowPixelNumR;
        MINT32 i4PixelCountG = ((1 << 24) + (i4WindowPixelNumG >> 1)) / i4WindowPixelNumG;
        MINT32 i4PixelCountB = ((1 << 24) + (i4WindowPixelNumB >> 1)) / i4WindowPixelNumB;

        // 1. Update AWB window config, RAW pregain1, and rotation matrix
        for (awbMode = LIB3A_AWB_MODE_OFF; awbMode < LIB3A_AWB_MODE_NUM; awbMode++)
        {
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4StatEnable = 1;

            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4WindowSizeX = m_rAWBWindowConfig.i4SizeX[sensorMode];
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4WindowSizeY = m_rAWBWindowConfig.i4SizeY[sensorMode];

            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4WindowPitchX = m_rAWBWindowConfig.i4PitchX[sensorMode];
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4WindowPitchY = m_rAWBWindowConfig.i4PitchY[sensorMode];

            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4WindowOriginX = m_rAWBWindowConfig.i4OriginX[sensorMode];
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4WindowOriginY = m_rAWBWindowConfig.i4OriginY[sensorMode];

            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4WindowNumX = m_rAWBWindowConfig.i4NumX[sensorMode];
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4WindowNumY = m_rAWBWindowConfig.i4NumY[sensorMode];

            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4LowThresholdR = m_rAWBStatParam.i4LowThresholdR;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4LowThresholdG = m_rAWBStatParam.i4LowThresholdG;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4LowThresholdB = m_rAWBStatParam.i4LowThresholdB;

            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4HighThresholdR = m_rAWBStatParam.i4HighThresholdR;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4HighThresholdG = m_rAWBStatParam.i4HighThresholdG;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4HighThresholdB = m_rAWBStatParam.i4HighThresholdB;

            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PixelCountR = i4PixelCountR;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PixelCountG = i4PixelCountG;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PixelCountB = i4PixelCountB;

            //if (m_i4AAOmode == 0)
            if((m_eAETargetMode == AE_MODE_IVHDR_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET) || (m_eAETargetMode == AE_MODE_ZVHDR_TARGET) ||
            (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)) // High bit mode (Support HDR) : LMT1 = (2^19)-1
            {
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGainLimitR = 0x80000 - (0x1000 -m_rAWBStatParam.i4PreGainLimitR);
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGainLimitG = 0x80000 - (0x1000 -m_rAWBStatParam.i4PreGainLimitG);
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGainLimitB = 0x80000 - (0x1000 -m_rAWBStatParam.i4PreGainLimitB);
            }
            else  // Low bit mode (Not support HDR) : LMT1 = (2^16)-1
            {
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGainLimitR = 0x10000 - (0x1000 -m_rAWBStatParam.i4PreGainLimitR);
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGainLimitG = 0x10000 - (0x1000 -m_rAWBStatParam.i4PreGainLimitG);
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGainLimitB = 0x10000 - (0x1000 -m_rAWBStatParam.i4PreGainLimitB);
            }

            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4LightSrcHighThresholdR = m_rAWBStatParam.i4LightSrcHighThresholdR;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4LightSrcHighThresholdG = m_rAWBStatParam.i4LightSrcHighThresholdG;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4LightSrcHighThresholdB = m_rAWBStatParam.i4LightSrcHighThresholdB;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4LightSrcLowThresholdR = m_rAWBStatParam.i4LightSrcLowThresholdR;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4LightSrcLowThresholdG = m_rAWBStatParam.i4LightSrcLowThresholdG;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4LightSrcLowThresholdB = m_rAWBStatParam.i4LightSrcLowThresholdB;

            // Valid data width
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ValidDataWidth = 0xFFF; //fix to (2^12)-1

            // AWB format shift
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4FormatShift = 0; //fix to 0

            // AWB output mode select flag
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4StatMode = m_rAWBStatParam.i4LinearOutputEn;

            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4HdrSupportEn = 1;

            //Pregain1
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGainR= 0x200;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGainG= 0x200;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGainB= 0x200;

            //Postgain
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PostGainR = m_rAWBRAWPreGain1.i4R;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PostGainG = m_rAWBRAWPreGain1.i4G;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PostGainB = m_rAWBRAWPreGain1.i4B;

            //AE pregain2
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGain2R= m_rAWBOutput.rPreviewRAWPreGain2.i4R;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGain2G= m_rAWBOutput.rPreviewRAWPreGain2.i4G;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGain2B= m_rAWBOutput.rPreviewRAWPreGain2.i4B;

            if (m_rAWBStatParam.i4ErrorThreshold != 0) // default setting
            {
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorThreshold = m_rAWBStatParam.i4ErrorThreshold;
            }
            else // error pixel count threshold = window size * error pixel count threshold ratio / 256, the error pixel count threshold ratio's base is 256
            {
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorThreshold = m_rAWBWindowConfig.i4SizeX[sensorMode] * m_rAWBWindowConfig.i4SizeY[sensorMode] * m_rAWBStatParam.i4ErrorRatio / 256;
            }

            // (0xF << (error shift bit + 1)) > error pixel count threshold, error shift bit's range is 0~7
            if (m_rAWBStatParam.i4ErrorShiftBits != 0)
            {
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorShiftBits = m_rAWBStatParam.i4ErrorShiftBits;
            }
            if (m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorThreshold < 30)
            {
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorShiftBits = 0;
            }
            else if (m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorThreshold < 60)
            {
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorShiftBits = 1;
            }
            else if (m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorThreshold < 120)
            {
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorShiftBits = 2;
            }
            else if (m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorThreshold < 240)
            {
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorShiftBits = 3;
            }
            else if (m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorThreshold < 480)
            {
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorShiftBits = 4;
            }
            else if (m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorThreshold < 960)
            {
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorShiftBits = 5;
            }
            else if (m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorThreshold < 1920)
            {
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorShiftBits = 6;
            }
            else if (m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorThreshold < 3840)
            {
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorShiftBits = 7;
            }
            else
            {
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorShiftBits = 0;
            }

            if ((m_eCamScenarioMode == CAM_SCENARIO_CUSTOM1) || (m_eCamScenarioMode == CAM_SCENARIO_CUSTOM2)) // HDR
            { // motion error pixel count threshold = window size * QBIN ratio *  motion error pixel count threshold ratio / 256, the motion error pixel count threshold ratio's base is 256
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4MoErrorThreshold = m_rAWBWindowConfig.i4SizeX[sensorMode] * m_rAWBWindowConfig.i4SizeY[sensorMode] * m_i4QbinRatio * m_rAWBStatParam.i4MoErrorRatio / 256;
            }
            else // non-HDR
            {
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4MoErrorThreshold = 0x3FFF;
            }

            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4Cos = m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rRotationMatrix.i4Cos;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4Sin = m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rRotationMatrix.i4Sin;
        }

#if 0
		AWB_LOG_IF(m_i4DgbLog, "[%s()] i4ErrorThreshold: %d => %d, i4ErrorShiftBits: %d, i4MoErrorThreshold: %d, i4PreGainLimitR :%d, \n", __FUNCTION__,
                m_rAWBStatParam.i4ErrorThreshold, m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4ErrorThreshold,
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4ErrorShiftBits,
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4MoErrorThreshold,
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4PreGainLimitR);

        AWB_LOG("[AWBStatConfig] sensorMode = %d\n", sensorMode);
        AWB_LOG("[AWBStatConfig] i4WindowNumX = %d\n", m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4WindowNumX);
        AWB_LOG("[AWBStatConfig] i4WindowNumY = %d\n", m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4WindowNumY);
        AWB_LOG("[AWBStatConfig] i4WindowPitchX = %d\n", m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4WindowPitchX);
        AWB_LOG("[AWBStatConfig] i4WindowPitchY = %d\n", m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4WindowPitchY);
        AWB_LOG("[AWBStatConfig] i4WindowSizeX = %d\n", m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4WindowSizeX);
        AWB_LOG("[AWBStatConfig] i4WindowSizeY = %d\n", m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4WindowSizeY);
        AWB_LOG("[AWBStatConfig] i4WindowOriginX = %d\n", m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4WindowOriginX);
        AWB_LOG("[AWBStatConfig] i4WindowOriginY = %d\n", m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4WindowOriginY);
        AWB_LOG("[AWBStatConfig] i4WindowPixelNumR = %d\n", i4WindowPixelNumR);
        AWB_LOG("[AWBStatConfig] i4WindowPixelNumG = %d\n", i4WindowPixelNumG);
        AWB_LOG("[AWBStatConfig] i4WindowPixelNumB = %d\n", i4WindowPixelNumB);
        AWB_LOG("[AWBStatConfig] i4PixelCountR = %d\n", m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4PixelCountR);
        AWB_LOG("[AWBStatConfig] i4PixelCountG = %d\n", m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4PixelCountG);
        AWB_LOG("[AWBStatConfig] i4PixelCountB = %d\n", m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4PixelCountB);
        AWB_LOG("[AWBStatConfig] i4Cos = %d\n", m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4Cos);
        AWB_LOG("[AWBStatConfig] i4Sin = %d\n", m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4Sin);
#else
        AWB_LOG_IF(m_i4DgbLog,"[AWBStatConfig] sensorMode = %d i4WindowNumX/Y=%d/%d i4WindowPitchX/Y=%d/%d i4WindowSizeX/Y=%d/%d i4WindowOriginX/Y=%d/%d i4WindowPixelNumR/G/B=%d/%d/%d i4PixelCountR/G/B=%d/%d/%d i4Cos/i4Sin=%d/%d \n", sensorMode,
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4WindowNumX, m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4WindowNumY,
		m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4WindowPitchX, m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4WindowPitchY,
		m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4WindowSizeX, m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4WindowSizeY,
		m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4WindowOriginX, m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4WindowOriginY,
		i4WindowPixelNumR, i4WindowPixelNumG, i4WindowPixelNumB,
		m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4PixelCountR, m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4PixelCountG, m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4PixelCountB,
		m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4Cos, m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4Sin);

		AWB_LOG_IF(m_i4DgbLog,"[AWBStatConfig] sensorMode = %d i4PreGainLimitR/G/B=%d/%d/%d i4LightSrcHighThresholdR/G/B=%d/%d/%d i4LightSrcLowThresholdR/G/B=%d/%d/%d i4ValidDataWidth=%d i4FormatShift=%d i4StatMode=%d i4ErrorThreshold=%d i4ErrorShiftBits=%d i4MoErrorThreshold=%d\n", sensorMode,
		m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4PreGainLimitR,m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4PreGainLimitG,m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4PreGainLimitB,
		m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4LightSrcHighThresholdR,m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4LightSrcHighThresholdG,m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4LightSrcHighThresholdB,
		m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4LightSrcLowThresholdR,m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4LightSrcLowThresholdG,m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4LightSrcLowThresholdB,
		m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4ValidDataWidth, m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4FormatShift,
		m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4StatMode, m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4ErrorThreshold,
		m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4ErrorShiftBits, m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4MoErrorThreshold);

#endif
        // 2. Update AWB light area
        LIGHT_AREA_T *pAWBArea = &m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rAWBLightArea[AWB_LIGHT_T];

        // Tungsten
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_T] = pAWBArea->i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_T] = pAWBArea->i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_T] = pAWBArea->i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_T] = pAWBArea->i4UpperBound;

        pAWBArea = &m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rAWBLightArea[AWB_LIGHT_WF];
        // Warm fluorescent
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_WF] = pAWBArea->i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_WF] = pAWBArea->i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_WF] = pAWBArea->i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_WF] = pAWBArea->i4UpperBound;

        pAWBArea = &m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rAWBLightArea[AWB_LIGHT_F];
        // Fluorescent
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_F] = pAWBArea->i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_F] = pAWBArea->i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_F] = pAWBArea->i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_F] = pAWBArea->i4UpperBound;

        pAWBArea = &m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rAWBLightArea[AWB_LIGHT_CWF];
        // CWF
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_CWF] = pAWBArea->i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_CWF] = pAWBArea->i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_CWF] = pAWBArea->i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_CWF] = pAWBArea->i4UpperBound;

        pAWBArea = &m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rAWBLightArea[AWB_LIGHT_DAYLIGHT];
        // Daylight
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_DAYLIGHT] = pAWBArea->i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_DAYLIGHT] = pAWBArea->i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_DAYLIGHT] = pAWBArea->i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_DAYLIGHT] = pAWBArea->i4UpperBound;

        pAWBArea = &m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rAWBLightArea[AWB_LIGHT_SHADE];
        // Shade
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_SHADE] = pAWBArea->i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_SHADE] = pAWBArea->i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_SHADE] = pAWBArea->i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_SHADE] = pAWBArea->i4UpperBound;

        pAWBArea = &m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rAWBLightArea[AWB_LIGHT_DF];
        // Daylight fluorescent
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_DF] = pAWBArea->i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_DF] = pAWBArea->i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_DF] = pAWBArea->i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_DF] = pAWBArea->i4UpperBound;

        pAWBArea = &m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rAWBLightArea[AWB_LIGHT_STROBE];
        // Strobe
        if(m_setFlashwin == 1) //set strobe window for precheck
        {
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_STROBE] = m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rAWBLightArea[AWB_LIGHT_SHADE].i4RightBound;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_STROBE] = m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rAWBLightArea[AWB_LIGHT_T].i4LeftBound;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_STROBE] = m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rAWBLightArea[AWB_LIGHT_DF].i4LowerBound;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_STROBE] = m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rAWBLightArea[AWB_LIGHT_F].i4UpperBound;
        }
        else
        {
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_STROBE] = pAWBArea->i4RightBound;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_STROBE] = pAWBArea->i4LeftBound;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_STROBE] = pAWBArea->i4LowerBound;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_STROBE] = pAWBArea->i4UpperBound;
        }
#if 0
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rTungsten.i4RightBound = %d\n", m_eSensorDev, pAWBArea->rTungsten.i4RightBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rTungsten.i4LeftBound = %d\n", m_eSensorDev, pAWBArea->rTungsten.i4LeftBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rTungsten.i4LowerBound = %d\n", m_eSensorDev, pAWBArea->rTungsten.i4LowerBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rTungsten.i4UpperBound = %d\n", m_eSensorDev, pAWBArea->rTungsten.i4UpperBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rWarmFluorescent.i4RightBound = %d\n", m_eSensorDev, pAWBArea->rWarmFluorescent.i4RightBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rWarmFluorescent.i4LeftBound = %d\n", m_eSensorDev, pAWBArea->rWarmFluorescent.i4LeftBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rWarmFluorescent.i4LowerBound = %d\n", m_eSensorDev, pAWBArea->rWarmFluorescent.i4LowerBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rWarmFluorescent.i4UpperBound = %d\n", m_eSensorDev, pAWBArea->rWarmFluorescent.i4UpperBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rFluorescent.i4RightBound = %d\n", m_eSensorDev, pAWBArea->rFluorescent.i4RightBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rFluorescent.i4LeftBound = %d\n", m_eSensorDev, pAWBArea->rFluorescent.i4LeftBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rFluorescent.i4LowerBound = %d\n", m_eSensorDev, pAWBArea->rFluorescent.i4LowerBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rFluorescent.i4UpperBound = %d\n", m_eSensorDev, pAWBArea->rFluorescent.i4UpperBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rCWF.i4RightBound = %d\n", m_eSensorDev, pAWBArea->rCWF.i4RightBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rCWF.i4LeftBound = %d\n", m_eSensorDev, pAWBArea->rCWF.i4LeftBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rCWF.i4LowerBound = %d\n", m_eSensorDev, pAWBArea->rCWF.i4LowerBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rCWF.i4UpperBound = %d\n", m_eSensorDev, pAWBArea->rCWF.i4UpperBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rDaylight.i4RightBound = %d\n", m_eSensorDev, pAWBArea->rDaylight.i4RightBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rDaylight.i4LeftBound = %d\n", m_eSensorDev, pAWBArea->rDaylight.i4LeftBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rDaylight.i4LowerBound = %d\n", m_eSensorDev, pAWBArea->rDaylight.i4LowerBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rDaylight.i4UpperBound = %d\n", m_eSensorDev, pAWBArea->rDaylight.i4UpperBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rDaylightFluorescent.i4RightBound = %d\n", m_eSensorDev, pAWBArea->rDaylightFluorescent.i4RightBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rDaylightFluorescent.i4LeftBound = %d\n", m_eSensorDev, pAWBArea->rDaylightFluorescent.i4LeftBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rDaylightFluorescent.i4LowerBound = %d\n", m_eSensorDev, pAWBArea->rDaylightFluorescent.i4LowerBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rDaylightFluorescent.i4UpperBound = %d\n", m_eSensorDev, pAWBArea->rDaylightFluorescent.i4UpperBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rStrobe.i4RightBound = %d\n", m_eSensorDev, pAWBArea->rStrobe.i4RightBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rStrobe.i4LeftBound = %d\n", m_eSensorDev, pAWBArea->rStrobe.i4LeftBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rStrobe.i4LowerBound = %d\n", m_eSensorDev, pAWBArea->rStrobe.i4LowerBound);
        AWB_LOG("[%d][AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea.rStrobe.i4UpperBound = %d\n", m_eSensorDev, pAWBArea->rStrobe.i4UpperBound);
#else
        AWB_LOG_IF(m_i4DgbLog,"[AWBStatConfig] m_rAWBInitInput.rAWBNVRAM.rAWBLightArea(R,L,D,U) : [T]=(%d,%d,%d,%d) [WF]=(%d,%d,%d,%d) [F]=(%d,%d,%d,%d) [CWF]=(%d,%d,%d,%d) [D]=(%d,%d,%d,%d) [Shade]=(%d,%d,%d,%d) [DF]=(%d,%d,%d,%d) [Strobe]=(%d,%d,%d,%d)\n",
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_T], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_T], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_T], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_T],
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_WF], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_WF], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_WF],m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_WF],
		m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_F], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_F], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_F], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_F],
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_CWF], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_CWF], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_CWF], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_CWF],
		m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_DAYLIGHT], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_DAYLIGHT], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_DAYLIGHT], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_DAYLIGHT],
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_SHADE], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_SHADE], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_SHADE], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_SHADE],
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_DF], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_DF], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_DF], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_DF],
     	m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_STROBE], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_STROBE], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_STROBE], m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_STROBE]);

#endif
        // 3. Update PWB light area
        LIGHT_AREA_T *pPWBArea = &m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rPWBReferenceArea;

        // Reference area
        for (MINT32 i = LIB3A_AWB_MODE_DAYLIGHT; i < LIB3A_AWB_MODE_NUM; i++)
        {
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][i].i4AWBXY_WINR[PWB_REFERENCE_AREA_INDEX] = pPWBArea->i4RightBound;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][i].i4AWBXY_WINL[PWB_REFERENCE_AREA_INDEX] = pPWBArea->i4LeftBound;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][i].i4AWBXY_WIND[PWB_REFERENCE_AREA_INDEX] = pPWBArea->i4LowerBound;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][i].i4AWBXY_WINU[PWB_REFERENCE_AREA_INDEX] = pPWBArea->i4UpperBound;
        }

        pPWBArea = &m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rPWBLightArea[PWB_LIGHT_DAYLIGHT];
        // Daylight neutral area
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_DAYLIGHT].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_DAYLIGHT].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_DAYLIGHT].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_DAYLIGHT].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4UpperBound;

        pPWBArea = &m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rPWBLightArea[PWB_LIGHT_CLOUDY_D];
        // Cloudy daylight neutral area
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_CLOUDY_DAYLIGHT].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_CLOUDY_DAYLIGHT].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_CLOUDY_DAYLIGHT].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_CLOUDY_DAYLIGHT].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4UpperBound;

        pPWBArea = &m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rPWBLightArea[PWB_LIGHT_SHADE];
        // Shade neutral area
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_SHADE].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_SHADE].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_SHADE].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_SHADE].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4UpperBound;

        pPWBArea = &m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rPWBLightArea[PWB_LIGHT_TWILIGHT];
        // Twilight neutral area
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_TWILIGHT].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_TWILIGHT].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_TWILIGHT].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_TWILIGHT].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4UpperBound;

        pPWBArea = &m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rPWBLightArea[PWB_LIGHT_F];
        // Twilight neutral area
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_FLUORESCENT].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_FLUORESCENT].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_FLUORESCENT].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_FLUORESCENT].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4UpperBound;

        pPWBArea = &m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rPWBLightArea[PWB_LIGHT_WF];
        // Warm fluorescent neutral area
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_WARM_FLUORESCENT].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_WARM_FLUORESCENT].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_WARM_FLUORESCENT].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_WARM_FLUORESCENT].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4UpperBound;

        pPWBArea = &m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rPWBLightArea[PWB_LIGHT_INCANDESCENT];
        // Incandescent neutral area
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_INCANDESCENT].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_INCANDESCENT].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_INCANDESCENT].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_INCANDESCENT].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4UpperBound;

        pPWBArea = &m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rPWBLightArea[PWB_LIGHT_GRAY_WORLD];
        // Gray world neutral area
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_GRAYWORLD].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_GRAYWORLD].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_GRAYWORLD].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_GRAYWORLD].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->i4UpperBound;
    }

    // STROBE OFF
    for (sensorMode = ESensorMode_Preview; sensorMode < ESensorMode_NUM; sensorMode++)
    {
        for (awbMode = LIB3A_AWB_MODE_OFF; awbMode < LIB3A_AWB_MODE_NUM; awbMode++)
        {
            m_rAWBStatCfg[AWB_STROBE_MODE_OFF][sensorMode][awbMode] = m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode];
        }
        m_rAWBStatCfg[AWB_STROBE_MODE_OFF][sensorMode][LIB3A_AWB_MODE_OFF] = m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO];
    }

    // Update strobe AWB light area
    for (sensorMode = ESensorMode_Preview; sensorMode < ESensorMode_NUM; sensorMode++)
    {
        if(m_setFlashwin == 1) //set strobe window for precheck
        {
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_STROBE] = m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rAWBLightArea[AWB_LIGHT_SHADE].i4RightBound;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_STROBE] = m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rAWBLightArea[AWB_LIGHT_T].i4LeftBound;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_STROBE] = m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rAWBLightArea[AWB_LIGHT_DF].i4LowerBound;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_STROBE] = m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rAWBLightArea[AWB_LIGHT_F].i4UpperBound;
        }
        else
        {
        m_rAWBStatCfg[AWB_STROBE_MODE_OFF][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_STROBE] = 0;
        m_rAWBStatCfg[AWB_STROBE_MODE_OFF][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_STROBE] = 0;
        m_rAWBStatCfg[AWB_STROBE_MODE_OFF][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_STROBE] = 0;
        m_rAWBStatCfg[AWB_STROBE_MODE_OFF][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_STROBE] = 0;
    }
    }

// nvram data print
#if 0
if (m_i4DgbLog)
{
   MINT32 strobeMode = 0;
   for (strobeMode = AWB_STROBE_MODE_ON; strobeMode < AWB_STROBE_MODE_NUM; strobeMode++)
       {
    for (sensorMode = ESensorMode_Preview; sensorMode < ESensorMode_NUM; sensorMode++)
        {
        for (awbMode = LIB3A_AWB_MODE_OFF; awbMode < LIB3A_AWB_MODE_NUM; awbMode++)
            {
                   AWB_LOG_IF(m_i4DgbLog, " [strobeMode][sensorMode][awbMode] [%d/%d/%d] = %d/%d/%d/%d \n", strobeMode, sensorMode, awbMode,
                    m_rAWBStatCfg[strobeMode][sensorMode][awbMode].i4AWBXY_WINL[0],
                    m_rAWBStatCfg[strobeMode][sensorMode][awbMode].i4AWBXY_WINR[0],
                    m_rAWBStatCfg[strobeMode][sensorMode][awbMode].i4AWBXY_WIND[0],
                    m_rAWBStatCfg[strobeMode][sensorMode][awbMode].i4AWBXY_WINU[0]);
            }
        }
    }
}
#endif
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::AWBRAWPreGain1Config()
{
    for (MINT32 strobeMode = AWB_STROBE_MODE_ON; strobeMode < AWB_STROBE_MODE_NUM; strobeMode++)
        for (MINT32 sensorMode = ESensorMode_Preview; sensorMode < ESensorMode_NUM; sensorMode++)
            for (MINT32 awbMode = LIB3A_AWB_MODE_OFF; awbMode < LIB3A_AWB_MODE_NUM; awbMode++)
            {
                m_rAWBStatCfg[strobeMode][sensorMode][awbMode].i4PostGainR = m_rAWBRAWPreGain1.i4R;
                m_rAWBStatCfg[strobeMode][sensorMode][awbMode].i4PostGainG = m_rAWBRAWPreGain1.i4G;
                m_rAWBStatCfg[strobeMode][sensorMode][awbMode].i4PostGainB = m_rAWBRAWPreGain1.i4B;
            }

            AWB_LOG("[AWBRAWPreGain1Config()] i4PostGainR/G/B = %d/%d/%d\n", m_rAWBStatCfg[AWB_STROBE_MODE_ON][ESensorMode_Preview][LIB3A_AWB_MODE_AUTO].i4PostGainR,
                  m_rAWBStatCfg[AWB_STROBE_MODE_ON][ESensorMode_Preview][LIB3A_AWB_MODE_AUTO].i4PostGainG,
                  m_rAWBStatCfg[AWB_STROBE_MODE_ON][ESensorMode_Preview][LIB3A_AWB_MODE_AUTO].i4PostGainB);

            return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::setTGInfo(MINT32 const i4TGInfo)
{
    AWB_LOG("[%s()] i4TGInfo: %d \n", __FUNCTION__, i4TGInfo);

    switch (i4TGInfo)
    {
    case CAM_TG_1:
        m_eSensorTG = ESensorTG_1;
        break;
    case CAM_TG_2:
        m_eSensorTG = ESensorTG_2;
        break;
    default:
        CAM_LOGE("i4TGInfo = %d", i4TGInfo);
        return MFALSE;
    }

    return MTRUE;
}

MBOOL AwbMgr::setIsMono(int isMono, MUINT32 i4SensorType)
{
      mIsMono=isMono;
      mSensorType = i4SensorType;
      return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::setAWBNvramIdx(MINT32 const i4AWBNvramIdx)
{
    if (m_i4AWBNvramIdx != i4AWBNvramIdx) {
        AWB_LOG("[%s()] m_i4AWBNvramIdx: %d ==> %d \n", __FUNCTION__, m_i4AWBNvramIdx, i4AWBNvramIdx);
        m_i4AWBNvramIdx = i4AWBNvramIdx;
        m_bAWBNvramIdxChanged = MTRUE;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::setStrobeNvramIdx(MINT32 const i4StrobeNvramIdx)
{
    if (m_i4StrobeNvramIdx != i4StrobeNvramIdx) {
        AWB_LOG("[%s()] m_i4StrobeNvramIdx: %d ==> %d \n", __FUNCTION__, m_i4StrobeNvramIdx, i4StrobeNvramIdx);
        m_i4StrobeNvramIdx = i4StrobeNvramIdx;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::setFlashCaliNvramIdx(MINT32 const i4FlashCaliNvramIdx)
{
    if (m_i4FlashCaliNvramIdx != i4FlashCaliNvramIdx) {
        AWB_LOG("[%s()] m_i4FlashCaliNvramIdx: %d ==> %d \n", __FUNCTION__, m_i4FlashCaliNvramIdx, i4FlashCaliNvramIdx);
        m_i4FlashCaliNvramIdx = i4FlashCaliNvramIdx;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::setCamScenarioMode(MUINT32 u4NewCamScenarioMode)
{
    CAM_SCENARIO_T eNewCamScenarioMode = static_cast<CAM_SCENARIO_T>(u4NewCamScenarioMode);

    //get custom param by sensor ID,MODULE ID, LENS ID
    SensorStaticInfo sInfo;
    CAM_CAL_DATA_STRUCT GetCamCalData;
    CamCalDrvBase *pCamCalDrvObj = CamCalDrvBase::createInstance();
    MUINT32 result=0;
    MINT32 i4SensorDevID;

    if( m_eCamScenarioMode != eNewCamScenarioMode) {
        AWB_LOG( "[%s()] m_eSensorDev: %d, m_eCamScenarioMode: %d -> %d, m_i4AWBNvramIdx: %d -> %d\n", __FUNCTION__, m_eSensorDev, m_eCamScenarioMode, eNewCamScenarioMode, m_i4AWBNvramIdx, AWBNVRAMMapping[eNewCamScenarioMode]);

        m_eCamScenarioMode = eNewCamScenarioMode;
        m_i4AWBNvramIdx = AWBNVRAMMapping[m_eCamScenarioMode];
        m_bAWBNvramIdxChanged = MTRUE;

	switch (m_eSensorDev)
        {
        case ESensorDev_Main:
            i4SensorDevID = NSCam::SENSOR_DEV_MAIN;
            break;
        case ESensorDev_Sub:
            i4SensorDevID = NSCam::SENSOR_DEV_SUB;
            break;
        case ESensorDev_MainSecond:
            i4SensorDevID = NSCam::SENSOR_DEV_MAIN_2;
            break;
        case ESensorDev_Main3D:
            i4SensorDevID = NSCam::SENSOR_DEV_MAIN_3D;
            break;
        case ESensorDev_SubSecond:
            i4SensorDevID = NSCam::SENSOR_DEV_SUB_2;
            break;
        default:
            i4SensorDevID = NSCam::SENSOR_DEV_NONE;
            break;
        }

	// Get Sensor ID
	IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
	pIHalSensorList->querySensorStaticInfo(i4SensorDevID, &sInfo);
	// Get Module info
	CAMERA_CAM_CAL_TYPE_ENUM enCamCalEnum=CAMERA_CAM_CAL_DATA_MODULE_VERSION;
	result= pCamCalDrvObj->GetCamCalCalData(i4SensorDevID, enCamCalEnum, (void *)&GetCamCalData);

	m_i4CurrSensorId = sInfo.sensorDevID;
	m_i4CurrModuleId = (result == CAM_CAL_ERR_NO_ERR)? (GetCamCalData.PartNumber[1] << 8) + GetCamCalData.PartNumber[0] : 0;
	m_i4CurrLensId = (result == CAM_CAL_ERR_NO_ERR)? (GetCamCalData.PartNumber[9] << 8) + GetCamCalData.PartNumber[8] : 0;
    // Get AWB Module param Data
	m_rAWBCustPARAM = getAWBModuleParamData(m_eCamScenarioMode, m_i4CurrSensorId, m_i4CurrModuleId, m_i4CurrLensId);
	AWB_LOG("[%s()]m_eCamScenarioMode=%d (0x%8x)=m_pCamCalDrvObj->GetCamCalCalData SensorID:0x%0X ModuleID:0x%0X LensID:0x%0X CustVer:%d", 			__FUNCTION__, m_eCamScenarioMode,result,m_i4CurrSensorId, m_i4CurrModuleId, m_i4CurrLensId,
		m_rAWBCustPARAM.pCustomParam->CustomAWBInfo.u4AWBCustomVersion);

        if(m_pNVRAM_3A != NULL)
        {
            getSensorResolution();

            if (!AWBInit())
            {
                CAM_LOGE("AWBInit() fail\n");
                return MFALSE;
            }
        }
    }

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::setNVRAMIndex(MUINT32 a_eNVRAMIndex)
{
    if( m_i4AWBNvramIdx != a_eNVRAMIndex) {
        AWB_LOG( "[%s()] m_eSensorDev: %d, m_i4AWBNvramIdx: %d -> %d\n", __FUNCTION__, m_eSensorDev, m_i4AWBNvramIdx, a_eNVRAMIndex);

        m_i4AWBNvramIdx = a_eNVRAMIndex;
        m_bAWBNvramIdxChanged = MTRUE;

        if(m_pNVRAM_3A != NULL)
        {
            getSensorResolution();

            if (!AWBInit())
            {
                CAM_LOGE("AWBInit() fail\n");
                return MFALSE;
            }
        }
    }

    return S_AWB_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::AWBInit()
{
    AWB_LOG_IF(m_i4DgbLog,"[%s()][%d]\n", __FUNCTION__, m_eSensorDev);

    m_rAWBInitInput.eAWBMode = m_eAWBMode;
    m_rAWBInitInput.i4NVRAMIndex = m_i4AWBNvramIdx;//.eAWBScenario = m_eCamScenarioMode;
    m_bStrobeModeChanged = MFALSE;
    m_bAWBModeChanged = MFALSE;
    m_bOneShotAWB = MTRUE; // do one-shot AWB
    m_bInitState = MTRUE; // init state

    switch (m_eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        m_rAWBInitInput.rAWBParam = getAWBParam<ESensorDev_Main>();
         //// ISP 5.0 move to nvram
        //m_rAWBInitInput.rAWBStatParam = getAWBStatParam<ESensorDev_Main>();
        //m_rAWBStatParam = getAWBStatParam<ESensorDev_Main>();
        break;
    case ESensorDev_MainSecond: //  Main Second Sensor
        m_rAWBInitInput.rAWBParam = getAWBParam<ESensorDev_MainSecond>();
        //m_rAWBInitInput.rAWBStatParam = getAWBStatParam<ESensorDev_MainSecond>();
        //m_rAWBStatParam = getAWBStatParam<ESensorDev_MainSecond>();
        break;
    case ESensorDev_Sub: //  Sub Sensor
        m_rAWBInitInput.rAWBParam = getAWBParam<ESensorDev_Sub>();
        //m_rAWBInitInput.rAWBStatParam = getAWBStatParam<ESensorDev_Sub>();
        //m_rAWBStatParam = getAWBStatParam<ESensorDev_Sub>();
        break;
    case ESensorDev_SubSecond: //  Sub Sensor
        m_rAWBInitInput.rAWBParam = getAWBParam<ESensorDev_SubSecond>();
        //m_rAWBInitInput.rAWBStatParam = getAWBStatParam<ESensorDev_SubSecond>();
        //m_rAWBStatParam = getAWBStatParam<ESensorDev_SubSecond>();
        break;
    default:
        CAM_LOGE("m_eSensorDev = %d", m_eSensorDev);
        return MFALSE;
    }
#if 0
    // Query TG info
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    IHalSensor* pIHalSensor = pIHalSensorList->createSensor("awb_mgr", m_i4SensorIdx);
    SensorDynamicInfo rSensorDynamicInfo;

    switch  ( m_eSensorDev )
    {
    case ESensorDev_Main:
        pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_MAIN, &rSensorDynamicInfo);
        break;
    case ESensorDev_Sub:
        pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_SUB, &rSensorDynamicInfo);
        break;
    case ESensorDev_MainSecond:
        pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorDynamicInfo);
        break;
    default:    //  Shouldn't happen.
        CAM_LOGE("Invalid sensor device: %d", m_eSensorDev);
    }

    if  ( pIHalSensor )
        pIHalSensor->destroyInstance("awb_mgr");

    AWB_LOG("[%d]TG = %d, pixel mode = %d\n", m_eSensorDev, rSensorDynamicInfo.TgInfo, rSensorDynamicInfo.pixelMode);

    switch (rSensorDynamicInfo.TgInfo)
    {
    case CAM_TG_1:
        m_eSensorTG = ESensorTG_1;
        break;
    case CAM_TG_2:
        m_eSensorTG = ESensorTG_2;
        break;
    default:
        CAM_LOGE("rSensorDynamicInfo.TgInfo = %d", rSensorDynamicInfo.TgInfo);
        return MFALSE;
    }
#endif

    // HBIN2 enable check
    queryHBinInfo();

    //char value[PROPERTY_VALUE_MAX] = {'\0'};
    MBOOL bDisable = 0;
    getPropInt("vendor.awb.otp.disable", &bDisable, 0);

    MBOOL bForceRead = 0;
    getPropInt("vendor.awb.otp.force.read", &bForceRead, 0);

    if (m_bAWBCalibrationBypassed) {
        m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4R = 0;
        m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4G = 0;
        m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4B = 0;
        m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4R = 0;
        m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4G = 0;
        m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4B = 0;
/*
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenValue.i4R = 0;
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenValue.i4Gr = 0;
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenValue.i4Gb = 0;
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenValue.i4B = 0;
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitValue.i4R = 0;
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitValue.i4Gr = 0;
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitValue.i4Gb = 0;
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitValue.i4B = 0;
*/
    }

    // EEPROM
    if (((!bDisable) &&
        (!m_bAWBCalibrationBypassed) &&
        ((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4R == 0) ||
        (m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4G == 0) ||
        (m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4B == 0) ||
        (m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4R == 0) ||
        (m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4G == 0) ||
        (m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4B == 0) ||
        m_bAWBNvramIdxChanged)) ||
        (bForceRead))
    {
        getEEPROMData(); // TBC
    }

//    if (m_bColdBoot || bForceRead || m_bAWBNvramIdxChanged) {
        m_rAWBInitInput.rAWBNVRAM = m_pNVRAM_3A->AWB[m_i4AWBNvramIdx];

      //new nvram
        m_rAWBInitInput.rAWBStatParam = m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rStatParam;
        m_rAWBStatParam = m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rStatParam;


        AWB_LOG_IF(m_i4DgbLog, "m_rAWBInitInput.eAWBMode = %d\n", m_rAWBInitInput.eAWBMode);
        //AWB_LOG_IF(m_i4DgbLog, "m_eSensorDev = %d\n", m_eSensorDev);

        AWBWindowConfig();
        AWBStatConfig();

       if(m_eAWBMode==LIB3A_AWB_MODE_MWB)
      {
         m_pIAwbAlgo->setAWBMode(m_eAWBMode);
         LIGHT_AREA_T rLightArea;
         m_pIAwbAlgo->getMWBLightArea(mMWBColorTemperature, rLightArea);

         m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4RightBound;
         m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4LeftBound;
         m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4LowerBound;
         m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4UpperBound;

         m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4RightBound;
         m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4LeftBound;
         m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4LowerBound;
         m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = rLightArea.i4UpperBound;
      }
     else
        m_pIAwbAlgo->setAWBMode(m_eAWBMode);

        m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode],
            m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
            m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);
        // update AWB statistics config
        #if 0
         ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).config(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable, mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY);
        #else
        // update AWB statistics config
        //AWBResultConfig_T rAWBResultConfig;
        ISP_AWB_CONFIG_T::getInstance(m_eSensorDev).AWBConfig(&m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], &m_sAWBResultConfig);
        //ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).configReg(&(m_sAWBResultConfig.rAWBRegInfo));
        #endif

        m_rAWBInitInput.rMultiIlluminance.i4GainSetNum=mGainSetNum;
#if 0
        m_rAWBInitInput.rMultiIlluminance.rGoldenGain2 = mGoldenGain2;
        m_rAWBInitInput.rMultiIlluminance.rGoldenGain3 = mGoldenGain3;
        m_rAWBInitInput.rMultiIlluminance.rUnitGain2 = mUnitGain2;
        m_rAWBInitInput.rMultiIlluminance.rUnitGain3 = mUnitGain3;
#endif
#if 1

        m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rCalData.rUnitGainM = mUnitGain2;
        m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rCalData.rUnitGainL = mUnitGain3;
        m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rCalData.rGoldenGainM = mGoldenGain2;
        m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rCalData.rGoldenGainL = mGoldenGain3;
/*
        m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rCalData.rGoldenValueM = mGoldenValueM;
        m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rCalData.rGoldenValueL = mGoldenValueL;
        m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rCalData.rUnitValueM = mUnitValueM;
        m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rCalData.rUnitValueL = mUnitValueL;
*/
#endif

        if (m_bAlgoInit == MFALSE)
        {
            if (FAILED(m_pIAwbAlgo->initAWB(m_rAWBInitInput, m_rAWBOutput, m_rAWBRAWPreGain1, m_rAWBRAWPreGain1M, m_rAWBRAWPreGain1L))) {
                CAM_LOGE("m_pIAwbAlgo->initAWB fail\n");
                return MFALSE;
            }

            m_bAlgoInit = MTRUE;
        }
        else
        {
            m_pIAwbAlgo->updateAWBParam(m_rAWBInitInput);
        }

        m_rAWBOutput.rAWBInfo.rPregain1 = m_rAWBRAWPreGain1;

        AWBRAWPreGain1Config();

        m_bColdBoot = MFALSE;
        m_bAWBNvramIdxChanged = MFALSE;
//    }
//    else {
//        m_pIAwbAlgo->setAWBMode(m_eAWBMode);
//        m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode],
//            m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
//            m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);
//    }
   // CB initial
    sem_init(&semCb, 0, 0);
    // NormalIOPipe create instance
    if (m_pNormalPipe == NULL)
    {
        AWB_LOG("[%s()] m_pNormalPipe is NULL, m_i4SensorIdx =%d\n", __FUNCTION__, m_i4SensorIdx);
        m_pNormalPipe = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe(m_i4SensorIdx, LOG_TAG);
        if (m_pNormalPipe == NULL)
        {
            CAM_LOGE("Fail to create NormalPipe");
            return MFALSE;
        }
    }

   if (!m_pCallBackCb){
       m_pCallBackCb = new AwbCb(m_eSensorDev);
       if (!m_pCallBackCb)
          CAM_LOGE("[%s] Callback Object created failed for SensorDev(%d)", __FUNCTION__, m_eSensorDev);
       else{
           m_pNormalPipe->sendCommand(ENPipeCmd_SET_AWB_CBFP, (MINTPTR)m_pCallBackCb, 0, 0);
           AWB_LOG("[%s()] Callback is created! m_pCallBackCb = %x \n", __FUNCTION__, m_pCallBackCb);
        }
       }

#if 0 // move to start() avoid nvram change effect
    // Face detection
   m_bFaceAWBAreaChage = MFALSE;
   m_eAWBFDArea.u4Count = 0;
#endif

    // reset AWB state
    if (m_eAWBMode == LIB3A_AWB_MODE_AUTO)
        m_pAwbStateMgr->reset(eState_InactiveAuto);
    else
        m_pAwbStateMgr->reset(eState_InactiveNotAuto);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::getColorCorrectionGain(MFLOAT& fGain_R, MFLOAT& fGain_G, MFLOAT& fGain_B)
{
    //AWB_LOG("%s()\n", __FUNCTION__);

    fGain_R = static_cast<MFLOAT>(m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R) / m_rAWBInitInput.rAWBNVRAM.rAlgoTuningParam.rChipParam.i4AWBGainOutputScaleUnit;
    fGain_G = static_cast<MFLOAT>(m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G) / m_rAWBInitInput.rAWBNVRAM.rAlgoTuningParam.rChipParam.i4AWBGainOutputScaleUnit;
    fGain_B = static_cast<MFLOAT>(m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B) / m_rAWBInitInput.rAWBNVRAM.rAlgoTuningParam.rChipParam.i4AWBGainOutputScaleUnit;

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::setColorCorrectionGain(MFLOAT fGain_R, MFLOAT fGain_G, MFLOAT fGain_B)
{
    AWB_LOG("%s() rgain=%5.2f ggain=%5.2f bgain=%5.2f\n", __FUNCTION__, (float)fGain_R, (float)fGain_G, (float)fGain_B);

    m_rColorCorrectionGain.i4R = static_cast<MINT32>((fGain_R * AWB_SCALE_UNIT) + 0.5);
    m_rColorCorrectionGain.i4G = static_cast<MINT32>((fGain_G * AWB_SCALE_UNIT) + 0.5);
    m_rColorCorrectionGain.i4B = static_cast<MINT32>((fGain_B * AWB_SCALE_UNIT) + 0.5);

    if (m_rColorCorrectionGain.i4R > m_rAWBInitInput.rAWBNVRAM.rAlgoTuningParam.rChipParam.i4AWBGainOutputUpperLimit)
        m_rColorCorrectionGain.i4R = m_rAWBInitInput.rAWBNVRAM.rAlgoTuningParam.rChipParam.i4AWBGainOutputUpperLimit;

    if (m_rColorCorrectionGain.i4G > m_rAWBInitInput.rAWBNVRAM.rAlgoTuningParam.rChipParam.i4AWBGainOutputUpperLimit)
        m_rColorCorrectionGain.i4G = m_rAWBInitInput.rAWBNVRAM.rAlgoTuningParam.rChipParam.i4AWBGainOutputUpperLimit;

    if (m_rColorCorrectionGain.i4B > m_rAWBInitInput.rAWBNVRAM.rAlgoTuningParam.rChipParam.i4AWBGainOutputUpperLimit)
        m_rColorCorrectionGain.i4B = m_rAWBInitInput.rAWBNVRAM.rAlgoTuningParam.rChipParam.i4AWBGainOutputUpperLimit;

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::getAWBState(mtk_camera_metadata_enum_android_control_awb_state_t& eAWBState)
{
    eAWBState = m_pAwbStateMgr->getCurrentState();

    AWB_LOG_IF(m_i4DgbLog,"[%s()]eAWBState= %d", __FUNCTION__, eAWBState);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::setColorCorrectionMode(MINT32 i4ColorCorrectionMode)
{
    AWB_LOG_IF(m_i4DgbLog,"%s() %d\n (TRANSFORM_MATRIX=%d MODE_FAST=%d HIGH_QUALITY=%d)", __FUNCTION__, i4ColorCorrectionMode,
        MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX,
        MTK_COLOR_CORRECTION_MODE_FAST,
        MTK_COLOR_CORRECTION_MODE_HIGH_QUALITY
        );

    switch (i4ColorCorrectionMode)
    {
    case MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX:
        m_eColorCorrectionMode = MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX;
        break;
    case MTK_COLOR_CORRECTION_MODE_FAST:
        m_eColorCorrectionMode = MTK_COLOR_CORRECTION_MODE_FAST;
        break;
    case MTK_COLOR_CORRECTION_MODE_HIGH_QUALITY:
        m_eColorCorrectionMode = MTK_COLOR_CORRECTION_MODE_HIGH_QUALITY;
        break;
    default:
        CAM_LOGE("Incorrect color correction mode = %d", i4ColorCorrectionMode);
    }

    return MTRUE;
}

MBOOL AwbMgr::setAETargetMode(eAETargetMODE mode) //for hdr
{
        m_eAETargetMode = mode;

        if((m_eAETargetMode == AE_MODE_IVHDR_TARGET) || (m_eAETargetMode == AE_MODE_MVHDR_TARGET) || (m_eAETargetMode == AE_MODE_ZVHDR_TARGET) ||
        (m_eAETargetMode == AE_MODE_4CELL_MVHDR_TARGET)) // High bit mode (Support HDR) : LMT1 = (2^19)-1
        {
                m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4PreGainLimitR = (m_rAWBStatParam.i4PreGainLimitR << 3) | 7;
                m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4PreGainLimitG = (m_rAWBStatParam.i4PreGainLimitG << 3) | 7;
                m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4PreGainLimitB = (m_rAWBStatParam.i4PreGainLimitB << 3) | 7;
        }
        else  // Low bit mode (Not support HDR) : LMT1 = (2^16)-1
        {
                m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4PreGainLimitR = m_rAWBStatParam.i4PreGainLimitR;
                m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4PreGainLimitG = m_rAWBStatParam.i4PreGainLimitG;
                m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4PreGainLimitB = m_rAWBStatParam.i4PreGainLimitB;
        }

        ISP_AWB_CONFIG_T::getInstance(m_eSensorDev).AWBConfig_update(&m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], &m_sAWBResultConfig);

        if( m_pIAwbAlgo != NULL)
    {
        m_pIAwbAlgo->setAWBHdrMode(static_cast<AWB_HDR_MODE_T> (mode));
        AWB_LOG("[%s()] mode: %d", __FUNCTION__, mode );
    }
    else
        AWB_LOG("[%s()] m_pIAwbAlgo=NULL!! mode: %d", __FUNCTION__, mode);

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if 0
MBOOL AwbMgr::setAAOMode(MUINT32 u4NewAWBAAOMode)
{
    AWB_LOG("[%s()] m_i4AAOmode: %d -> %d (0:8/12bits, 1:14bits)\n", __FUNCTION__, m_i4AAOmode, u4NewAWBAAOMode);
    m_i4AAOmode = u4NewAWBAAOMode;

    for (MINT32 strobeMode = AWB_STROBE_MODE_ON; strobeMode < AWB_STROBE_MODE_NUM; strobeMode++)
    {
        for (MINT32 sensorMode = ESensorMode_Preview; sensorMode < ESensorMode_NUM; sensorMode++)
        {
            for (MINT32 awbMode = LIB3A_AWB_MODE_OFF; awbMode < LIB3A_AWB_MODE_NUM; awbMode++)
            {
                if (m_i4AAOmode == 0)
                {
                    m_rAWBStatCfg[strobeMode][sensorMode][awbMode].i4PreGainLimitR = m_rAWBStatParam.i4PreGainLimitR;
                    m_rAWBStatCfg[strobeMode][sensorMode][awbMode].i4PreGainLimitG = m_rAWBStatParam.i4PreGainLimitG;
                    m_rAWBStatCfg[strobeMode][sensorMode][awbMode].i4PreGainLimitB = m_rAWBStatParam.i4PreGainLimitB;
                }
                else
                {
                    m_rAWBStatCfg[strobeMode][sensorMode][awbMode].i4PreGainLimitR = (m_rAWBStatParam.i4PreGainLimitR << 2) | 3;
                    m_rAWBStatCfg[strobeMode][sensorMode][awbMode].i4PreGainLimitG = (m_rAWBStatParam.i4PreGainLimitG << 2) | 3;
                    m_rAWBStatCfg[strobeMode][sensorMode][awbMode].i4PreGainLimitB = (m_rAWBStatParam.i4PreGainLimitB << 2) | 3;
                }
            }
        }
    }

    //ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).SetAAOMode(m_i4AAOmode);

    return MTRUE;
}
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::getAAOConfig(AWB_AAO_CONFIG_Param_T &rAWBConfig)
{
    int BlockNumW = m_rAWBStatParam.i4WindowNumX;
    int BlockNumH = m_rAWBStatParam.i4WindowNumY;

    int PitchW = mAaoW/BlockNumW;
    int PitchH = mAaoH/BlockNumH;
    int SizeW  = (PitchW / 2) * 2;
    int SizeH  = (PitchH / 2) * 2;
    int OriginX = (mAaoW - PitchW*BlockNumW)/2;
    int OriginY = (mAaoH - PitchH*BlockNumH)/2;

    rAWBConfig.width = mAaoW;
    rAWBConfig.height= mAaoH;
    rAWBConfig.offset_x = OriginX;
    rAWBConfig.offset_y = OriginY;
    rAWBConfig.size_x = SizeW;
    rAWBConfig.size_y = SizeH;
    rAWBConfig.num_x = BlockNumW;
    rAWBConfig.num_y = BlockNumH;

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Face detection
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AwbMgr::setFDenable(MBOOL bFDenable)
{
    if (bFDenable != m_bFDenable){
        //AWB_LOG( "[%s()] FDenable is from %d -> %d\n", __FUNCTION__, m_bFDenable, bFDenable);
        m_bFDenable = bFDenable;
        m_bFaceAWBAreaChage = MTRUE;
    }
    return S_AWB_OK;
}

MRESULT AwbMgr::setFDInfo(MVOID* a_sFaces, MINT32 i4tgwidth, MINT32 i4tgheight)
{
    MtkCameraFaceMetadata *pFaces = (MtkCameraFaceMetadata *)a_sFaces;
    MUINT32 ifaceIdx = 0;
    m_bIsFD = MTRUE;

    if(CUST_ENABLE_FACE_AWB() == MTRUE) {
        if(pFaces == NULL) {
            AWB_LOG( "[%s()] FD window is NULL pointer 1\n", __FUNCTION__);
            return S_AWB_OK;
        }

        if((pFaces->faces) == NULL) {
            AWB_LOG( "[%s()] FD window is NULL pointer 2\n", __FUNCTION__);
            return S_AWB_OK;
        }

        //First time with face to without face
        if (m_eAWBFDArea.u4Count && (pFaces->number_of_faces == 0)){
            m_bFaceAWBAreaChage = MTRUE;
            //AWB_LOG( "[%s()] First frame without face Count %d ->%d \n", __FUNCTION__, m_eAWBFDArea.u4Count, pFaces->number_of_faces);
        }

        // Clear m_eAWBFDArea content
        memset(&m_eAWBFDArea, 0, sizeof(AWB_FD_INFO_T));

        if(m_bFDenable == MTRUE){

            // Define FD area number
            if(pFaces->number_of_faces > MAX_AWB_METER_AREAS) {
                m_eAWBFDArea.u4Count = MAX_AWB_METER_AREAS;
            } else {
                m_eAWBFDArea.u4Count = pFaces->number_of_faces;
            }
            //portrait
            m_eAWBFDArea.rPortInfo.i4Enable = pFaces->CNNFaces.PortEnable;
            m_eAWBFDArea.rPortInfo.fIsPortrait = pFaces->CNNFaces.CnnResult1;
            m_eAWBFDArea.rPortInfo.fNoPortrait = pFaces->CNNFaces.CnnResult0;
            m_eAWBFDArea.rPortInfo.i4IsPortrait = pFaces->CNNFaces.IsTrueFace;
            m_eAWBFDArea.i4MLGenderDbgInfo = pFaces->fld_GenderInfo[0];

            for(ifaceIdx=0; ifaceIdx < m_eAWBFDArea.u4Count; ifaceIdx++)
            {
                m_eAWBFDArea.rAreas[ifaceIdx].i4Left = pFaces->faces[ifaceIdx].rect[0];
                m_eAWBFDArea.rAreas[ifaceIdx].i4Right = pFaces->faces[ifaceIdx].rect[2];
                m_eAWBFDArea.rAreas[ifaceIdx].i4Top = pFaces->faces[ifaceIdx].rect[1];
                m_eAWBFDArea.rAreas[ifaceIdx].i4Bottom = pFaces->faces[ifaceIdx].rect[3];
                m_eAWBFDArea.rAreas[ifaceIdx].i4Weight = 1;

                // new FD info
                m_eAWBFDArea.rAreas[ifaceIdx].i4Id = pFaces->faces[ifaceIdx].id;
                m_eAWBFDArea.rAreas[ifaceIdx].i4Type = pFaces->faces_type[ifaceIdx];
                m_eAWBFDArea.rAreas[ifaceIdx].i4Motion[0] = pFaces->motion[ifaceIdx][0];
                m_eAWBFDArea.rAreas[ifaceIdx].i4Motion[1] = pFaces->motion[ifaceIdx][1];

                // landmark cv,rip,rop
                m_eAWBFDArea.rAreas[ifaceIdx].i4LandmarkCV = pFaces->fa_cv[ifaceIdx];
                m_eAWBFDArea.rAreas[ifaceIdx].i4LandmarkRIP = pFaces->fld_rip[ifaceIdx];
                m_eAWBFDArea.rAreas[ifaceIdx].i4LandmarkROP = pFaces->fld_rop[ifaceIdx];

                if (pFaces->fa_cv[ifaceIdx] > 0)
                {
                    // left eye, left/right
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_X0] = pFaces->leyex0[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_Y0] = pFaces->leyey0[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_X1] = pFaces->leyex1[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_Y1] = pFaces->leyey1[ifaceIdx];

                    // left eye, up/down
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_UX] = pFaces->leyeux[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_UY] = pFaces->leyeuy[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_DX] = pFaces->leyedx[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_DY] = pFaces->leyedy[ifaceIdx];

                    // right eye
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_X0] = pFaces->reyex0[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_Y0] = pFaces->reyey0[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_X1] = pFaces->reyex1[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_Y1] = pFaces->reyey1[ifaceIdx];

                    // right eye, up/down
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_UX] = pFaces->reyeux[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_UY] = pFaces->reyeuy[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_DX] = pFaces->reyedx[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_DY] = pFaces->reyedy[ifaceIdx];

                    // mouth
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_MOUTH][AWB_LANDMARK_X0] = pFaces->mouthx0[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_MOUTH][AWB_LANDMARK_Y0] = pFaces->mouthy0[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_MOUTH][AWB_LANDMARK_X1] = pFaces->mouthx1[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_MOUTH][AWB_LANDMARK_Y1] = pFaces->mouthy1[ifaceIdx];

                    // nose
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_NOSE][AWB_LANDMARK_X0] = pFaces->nosex[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_NOSE][AWB_LANDMARK_Y0] = pFaces->nosey[ifaceIdx];
                }

                // FD area info
                m_eAWBFDArea.rAreas[ifaceIdx].i4Left   = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Left*2000/i4tgwidth;
                m_eAWBFDArea.rAreas[ifaceIdx].i4Right  = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Right*2000/i4tgwidth;
                m_eAWBFDArea.rAreas[ifaceIdx].i4Top    = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Top*2000/i4tgheight;
                m_eAWBFDArea.rAreas[ifaceIdx].i4Bottom = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Bottom*2000/i4tgheight;

                // Gender
                m_eAWBFDArea.rAreas[ifaceIdx].i4GenderInfo = pFaces->GenderLabel[ifaceIdx];

                if(pFaces->fa_cv[ifaceIdx] > 0)
                {
                    // left eye
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_X0] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_X0]*2000/i4tgwidth;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_Y0] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_Y0]*2000/i4tgheight;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_X1] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_X1]*2000/i4tgwidth;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_Y1] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_Y1]*2000/i4tgheight;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_UX] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_UX]*2000/i4tgwidth;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_UY] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_UY]*2000/i4tgheight;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_DX] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_DX]*2000/i4tgwidth;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_DY] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_DY]*2000/i4tgheight;

                    // right eye
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_X0] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_X0]*2000/i4tgwidth;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_Y0] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_Y0]*2000/i4tgheight;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_X1] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_X1]*2000/i4tgwidth;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_Y1] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_Y1]*2000/i4tgheight;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_UX] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_UX]*2000/i4tgwidth;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_UY] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_UY]*2000/i4tgheight;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_DX] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_DX]*2000/i4tgwidth;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_DY] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_DY]*2000/i4tgheight;

                    // mouth
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_MOUTH][AWB_LANDMARK_X0] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_MOUTH][AWB_LANDMARK_X0]*2000/i4tgwidth;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_MOUTH][AWB_LANDMARK_Y0] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_MOUTH][AWB_LANDMARK_Y0]*2000/i4tgheight;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_MOUTH][AWB_LANDMARK_X1] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_MOUTH][AWB_LANDMARK_X1]*2000/i4tgwidth;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_MOUTH][AWB_LANDMARK_Y1] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_MOUTH][AWB_LANDMARK_Y1]*2000/i4tgheight;

                    // nose
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_NOSE][AWB_LANDMARK_X0] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_NOSE][AWB_LANDMARK_X0]*2000/i4tgwidth;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_NOSE][AWB_LANDMARK_Y0] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_NOSE][AWB_LANDMARK_Y0]*2000/i4tgheight;
                }

                AWB_LOG( "[%s():Landmark][LEFT_EYE] Left: (%d,%d), Right:(%d,%d), Up:(%d,%d), Down:(%d,%d)\n", __FUNCTION__,
                m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_X0],m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_Y0],
                m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_X1],m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_Y1],
                m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_UX],m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_UY],
                m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_DX],m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_LEFT_EYE][AWB_LANDMARK_DY]);

                AWB_LOG( "[%s():Landmark][RIGHT_EYE] Left: (%d,%d), Right:(%d,%d), Up:(%d,%d), Down:(%d,%d)\n", __FUNCTION__,
                m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_X0],m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_Y0],
                m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_X1],m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_Y1],
                m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_UX],m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_UY],
                m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_DX],m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_RIGHT_EYE][AWB_LANDMARK_DY]);

                AWB_LOG( "[%s():Landmark][MOUTH] Left: (%d,%d), Right:(%d,%d)\n", __FUNCTION__,
                m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_MOUTH][AWB_LANDMARK_X0],m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_MOUTH][AWB_LANDMARK_Y0],
                m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_MOUTH][AWB_LANDMARK_X1],m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_MOUTH][AWB_LANDMARK_Y1]);

                AWB_LOG( "[%s():Landmark][NOSE] Center: (%d,%d)\n", __FUNCTION__,
                m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_NOSE][AWB_LANDMARK_X0],m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[AWB_LANDMARK_NOSE][AWB_LANDMARK_Y0]);

                AWB_LOG( "[%s():Landmark] CV: %d, RIP: %d, ROP: %d \n", __FUNCTION__, m_eAWBFDArea.rAreas[ifaceIdx].i4LandmarkCV,
                        m_eAWBFDArea.rAreas[ifaceIdx].i4LandmarkRIP,  m_eAWBFDArea.rAreas[ifaceIdx].i4LandmarkROP);

                AWB_LOG( "[%s():Modified] AWB FD meter area Idx:%d Left:%d Right:%d Top:%d Bottom:%d InitWidth:%d InitHeight:%d\n", __FUNCTION__, ifaceIdx,
                        m_eAWBFDArea.rAreas[ifaceIdx].i4Left, m_eAWBFDArea.rAreas[ifaceIdx].i4Right,
                        m_eAWBFDArea.rAreas[ifaceIdx].i4Top, m_eAWBFDArea.rAreas[ifaceIdx].i4Bottom,
                        m_eZoomWinInfo.u4XWidth,m_eZoomWinInfo.u4YHeight );
                AWB_LOG( "[%s():Previous] AWB FD meter area Idx:%d Left:%d Right:%d Top:%d Bottom:%d \n", __FUNCTION__, ifaceIdx,
                        pFaces->faces[ifaceIdx].rect[0], pFaces->faces[ifaceIdx].rect[1],
                        pFaces->faces[ifaceIdx].rect[2] , pFaces->faces[ifaceIdx].rect[3] );

                m_bFaceAWBAreaChage = MTRUE;

            }
            AWB_LOG_IF(m_i4DgbLog, "[%s()] i4SensorDev:%d line:%d Face Number:%d FaceAWBChange:%d Portrait %d/%d/%d/%d", __FUNCTION__, m_eSensorDev, __LINE__, m_eAWBFDArea.u4Count, m_bFaceAWBAreaChage,
                       m_eAWBFDArea.rPortInfo.i4Enable, m_eAWBFDArea.rPortInfo.fIsPortrait, m_eAWBFDArea.rPortInfo.fNoPortrait, m_eAWBFDArea.rPortInfo.i4IsPortrait);
        }
    }
    else {
        AWB_LOG( "[%s()] i4SensorDev:%d line:%d The set face AWB is disable:%d ", __FUNCTION__, m_eSensorDev, __LINE__, CUST_ENABLE_FACE_AWB());
    }

    return S_AWB_OK;
}

MRESULT AwbMgr::setZoomWinInfo(MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    if((u4XOffset <= 8192)&&(u4YOffset <= 8192)
        &&(u4Width <= 16384)&&(u4Height <= 16384)){
        if((m_eZoomWinInfo.u4XOffset != u4XOffset) || (m_eZoomWinInfo.u4XWidth != u4Width) ||
           (m_eZoomWinInfo.u4YOffset != u4YOffset) || (m_eZoomWinInfo.u4YHeight != u4Height)) {
/*
            AWB_LOG( "[%s()] New WinX:%d %d New WinY:%d %d Old WinX:%d %d Old WinY:%d %d\n", __FUNCTION__, u4XOffset, u4Width, u4YOffset, u4Height,
                   m_eZoomWinInfo.u4XOffset, m_eZoomWinInfo.u4XWidth,
                   m_eZoomWinInfo.u4YOffset, m_eZoomWinInfo.u4YHeight);
*/
            m_eZoomWinInfo.bZoomChange = MTRUE;
            m_eZoomWinInfo.u4XOffset = u4XOffset;
            m_eZoomWinInfo.u4XWidth = u4Width;
            m_eZoomWinInfo.u4YOffset = u4YOffset;
            m_eZoomWinInfo.u4YHeight = u4Height;
        }
    } else {
        AWB_LOG( "[%s()] Wrong zoom window size %d/%d/%d/%d \n", __FUNCTION__, u4XOffset, u4Width, u4YOffset, u4Height);
    }
    return S_AWB_OK;
}

MRESULT AwbMgr::getFocusArea(std::vector<MINT32> &vecOut, MINT32 i4tgwidth, MINT32 i4tgheight)
{
    vecOut.clear();

    /*
     * The order of vector vecOut is :
     * type -> number of ROI -> left of ROI1 -> top of ROI1 -> right of ROI1 -> bottom of ROI1 -> result of ROI1 -> left of ROI2 -> top of ROI2 -> right of ROI2 -> bottom of ROI2 -> result of ROI2 -> etc.
     */

    if (m_eAWBFDArea.u4Count != 0)
    {
        MINT32 type = 0; // Face AWB
        vecOut.push_back(type);
        MINT32 number = static_cast<MINT32>(m_eAWBFDArea.u4Count);
        vecOut.push_back(number);

        for (MINT32 ifaceIdx = 0; ifaceIdx < number; ifaceIdx ++)
        {
            MINT32 i4Left   = (m_eAWBFDArea.rAreas[ifaceIdx].i4Left   + 1000) * i4tgwidth  / 2000;
            MINT32 i4Top    = (m_eAWBFDArea.rAreas[ifaceIdx].i4Top    + 1000) * i4tgheight / 2000;
            MINT32 i4Right  = (m_eAWBFDArea.rAreas[ifaceIdx].i4Right  + 1000) * i4tgwidth  / 2000;
            MINT32 i4Bottom = (m_eAWBFDArea.rAreas[ifaceIdx].i4Bottom + 1000) * i4tgheight / 2000;
            MINT32 i4result = 0;

            vecOut.push_back(i4Left);
            vecOut.push_back(i4Top);
            vecOut.push_back(i4Right);
            vecOut.push_back(i4Bottom);
            vecOut.push_back(i4result);

            AWB_LOG_IF(m_i4DgbLog, "[%s()] AWB FD meter area Idx:%d Left:%d => %d Top:%d => %d Right:%d => %d Bottom:%d => %d TGWidth:%d TGHeight:%d\n", __FUNCTION__, ifaceIdx,
                    i4Left, m_eAWBFDArea.rAreas[ifaceIdx].i4Left,
                    i4Top, m_eAWBFDArea.rAreas[ifaceIdx].i4Top,
                    i4Right, m_eAWBFDArea.rAreas[ifaceIdx].i4Right,
                    i4Bottom, m_eAWBFDArea.rAreas[ifaceIdx].i4Bottom,
                    i4tgwidth, i4tgheight);
        }
    }
    else
    {
        MINT32 type = 0; // Face AWB
        vecOut.push_back(type);

        MINT32 number = 0;
        vecOut.push_back(number);
    }

    return S_AWB_OK;
}

MBOOL AwbMgr::setMWBColorTemperature(int colorTemperature)
{
    if(colorTemperature!=mMWBColorTemperature)
    {
        mMWBColorTemperature = colorTemperature;
        mIsMwbCctChanged = MTRUE;
    }
    if ( colorTemperature < MWBMIN)
      mIsMwbCctChanged = MFALSE;
    AWB_LOG_IF(m_i4DgbLog ,"[%s] colorTemperature mMWBColorTemperature mIsMwbCctChanged%d/%d/%d/% \n", __FUNCTION__, colorTemperature, mMWBColorTemperature, mIsMwbCctChanged);
    return MTRUE;
}
MBOOL AwbMgr::getSuppotMWBColorTemperature(MUINT32 &max, MUINT32 &min)
{
    max = MWBMAX;
    min = MWBMIN;

    return MTRUE;
}
MBOOL AwbMgr::getAWBColorTemperature(MUINT32 &colorTemperature)
{
    colorTemperature = (MUINT32)m_pIAwbAlgo->getCCT();
    return MTRUE;
}
MBOOL AwbMgr::SetTorchMode(MBOOL is_torch_now)
{
    m_bIsTorch = is_torch_now;
    AWB_LOG_IF(m_i4DgbLog ,"[%s] m_bIsTorch %d \n", __FUNCTION__, m_bIsTorch);
    return MTRUE;
}

MBOOL AwbMgr::SetMainFlashInfo(MBOOL is_main_flash_on)
{
    m_bIsMainFlashOn = is_main_flash_on;
    AWB_LOG_IF(m_i4DgbLog ,"[%s] m_bIsMainFlashOn %d \n", __FUNCTION__, m_bIsMainFlashOn);
    return MTRUE;
}

MBOOL AwbMgr::CalSencondSensorDefaultGain(AWB_GAIN_T &gain, MINT32 cct)
{

    MUINT32 ratio_1 = 0;
    MUINT32 ratio_2 = 0;
    MUINT32 casexx = 0;

    if (cct >= 6500)
    {
      gain = m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_D65];
      casexx = 0;
    }
    else if ( (cct < 6500) && (cct >= 5100))
    {
      casexx = 1;
      ratio_1 = (((float)cct- 5100)/(6500-5100))*100;
      ratio_2 = ((6500- (float)cct)/(6500-5100))*100;
      gain.i4R = (((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_D65].i4R * ratio_1)/100)+ ((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_DNP].i4R * ratio_2)/100));
      gain.i4G = (((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_D65].i4G * ratio_1)/100)+ ((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_DNP].i4G * ratio_2)/100));
      gain.i4B = (((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_D65].i4B * ratio_1)/100)+ ((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_DNP].i4B * ratio_2)/100));
    }
    else if ( (cct < 5100) &&  (cct >= 3750))
    {
      casexx  = 2;
      ratio_1 = (((float)cct- 3750)/(5100-3750))*100;
      ratio_2 = ((5100- (float)cct)/(5100-3750))*100;
      gain.i4R = (((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_DNP].i4R * ratio_1)/100)+ ((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_TL84].i4R * ratio_2)/100));
      gain.i4G = (((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_DNP].i4G * ratio_1)/100)+ ((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_TL84].i4G * ratio_2)/100));
      gain.i4B = (((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_DNP].i4B * ratio_1)/100)+ ((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_TL84].i4B * ratio_2)/100));
    }
    else if ( (cct < 3750) &&  (cct >= 2800))
    {
      casexx =3;
      ratio_1 = (((float)cct- 2800)/(3750-2800))*100;
      ratio_2 = ((3750- (float)cct)/(3750-2800))*100;
      gain.i4R = (((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_TL84].i4R * ratio_1)/100)+ ((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_A].i4R * ratio_2)/100));
      gain.i4G = (((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_TL84].i4G * ratio_1)/100)+ ((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_A].i4G * ratio_2)/100));
      gain.i4B = (((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_TL84].i4B * ratio_1)/100)+ ((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_A].i4B * ratio_2)/100));
    }

    else if ( (cct < 2800) &&  (cct >= 2300))
    {
     casexx = 4;
      ratio_1 = (((float)cct- 2300)/(2800-2300))*100;
      ratio_2 = ((2800- (float)cct)/(2800-2300))*100;
      gain.i4R = (((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_A].i4R * ratio_1)/100)+((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_H].i4R * ratio_2)/100));
      gain.i4G = (((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_A].i4G * ratio_1)/100)+((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_H].i4G * ratio_2)/100));
      gain.i4B = (((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_A].i4B * ratio_1)/100)+((m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_H].i4B * ratio_2)/100));
    }
    else // 2300 below
    {
      gain = m_pNVRAM_3A->AWB[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain[AWB_CAL_LIGHT_SRC_H];
      casexx = 5;
    }


    AWB_LOG_IF(m_i4DgbLog,"[%s] r1:%d r2:%d cct:%d  case: %d gain: %d/%d/%d",__FUNCTION__, ratio_1, ratio_2, cct, casexx, gain.i4R,
                   gain.i4G, gain.i4B);


    return MTRUE;
}
/******************************************************************************
*
******************************************************************************/
MVOID
AwbMgr::CustAWB(AWB_INPUT_T &a_rAWBInput, AWB_GAIN_T &a_rPregain1, AWB_GAIN_T &a_rPregain1M, AWB_GAIN_T &a_rPregain1L, AWB_GAIN_T &a_rOutputGain)
{
    //-------Get AWB Statistic Data Sample Code-------//
    //AWB_LINEAR_STAT_T rLinearStatBuf[AWB_WINDOW_NUM];
    //memset(&rLinearStatBuf, 0, sizeof(AWB_LINEAR_STAT_T) * AWB_WINDOW_NUM);
    //m_pIAwbAlgo->getAWBStat(a_rAWBInput, 1, reinterpret_cast<void *>(rLinearStatBuf)); //Mode 0 = Non-linear data, Mode 1 = Linear data

    //AWB_MAIN_STAT_2B_T rNonLinearStatBuf[AWB_WINDOW_NUM];
    //memset(&rNonLinearStatBuf, 0, sizeof(AWB_MAIN_STAT_2B_T) * AWB_WINDOW_NUM);
    //m_pIAwbAlgo->getAWBStat(a_rAWBInput, 0, reinterpret_cast<void *>(rNonLinearStatBuf)); //Mode 0 = Non-linear data, Mode 1 = Linear data
    //------------------------------------------------//

    memset(&a_rOutputGain, 0, sizeof(AWB_GAIN_T));
}

MBOOL AwbMgr::CallBackAwb(MVOID* pIn, MVOID* pOut)
{

if ((m_eColorCorrectionMode != MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX) && (m_IsSpeedAwb))
{
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        CAM_LOGE("error in clock_gettime! Please check\n");

    ts.tv_nsec += 20000000;
    int s = sem_timedwait(&semCb, &ts);
    if (s == -1)
        {
          AWB_LOG("[%s()] Waitsem late m_bIsNotPostSem = %d\n", __FUNCTION__, m_bIsNotPostSem);
          m_bIsNotPostSem = MTRUE;
        }
   AAA_TRACE_D ("AWB_CB_Setting");
   //update AWB statistics config
   //ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).Cbreconfig(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode],
   //                           m_rAWBOutput, m_FlareGain, m_FlareOffset, pIn , pOut);
   AWB_LOG_IF(m_i4DgbLog,"[%s()] Callback is executed \n", __FUNCTION__);
   AAA_TRACE_END_D;
}
return MTRUE;
}

MBOOL AwbMgr::PostSem(void)
{
   AWB_LOG_IF(m_i4DgbLog,"[%s()] CALLBACK m_bIsNotPostSem = %d\n", __FUNCTION__, m_bIsNotPostSem);

  if (!m_bIsNotPostSem)
    ::sem_post(&semCb);
  else
    m_bIsNotPostSem = MFALSE;

   return MTRUE;
}

MBOOL AwbMgr::SetAWBFlare(MUINT32 Flare , MUINT32 FlareOffset)
{
   if (0 != Flare){
     m_FlareGain = Flare;
     m_FlareOffset = FlareOffset;
    }
   //else
     //AWB_LOG_IF(m_i4DgbLog,"[%s] AE Flare zero\n", __FUNCTION__);

   //AWB_LOG_IF(m_i4DgbLog,"[%s()]AEFlare =%d m_FlareGain = %d AEFlareOffset =%d m_FlareOffset =%d\n", __FUNCTION__,
   //           Flare, m_FlareGain, FlareOffset, m_FlareOffset);
   return MTRUE;
}

// ISP6.0
MBOOL AwbMgr::setAWBInfo(AWB_INFO_T& rAWBInfo)
{
    m_ISP_AWBInfo.i4CCT = rAWBInfo.i4CCT;
    m_ISP_AWBInfo.i4SatRatio = 1; // set default temporarily
    m_ISP_AWBInfo.rAwbGainNoPref = rAWBInfo.rAwbGainNoPref;
    m_ISP_AWBInfo.rAwbGainPref = rAWBInfo.rAwbGainNoPref; //set default temporarily

    m_ISP_AWBInfo.rCurrentAWBGain = rAWBInfo.rCurrentAWBGain;
    m_ISP_AWBInfo.i4FluorescentIndex = rAWBInfo.i4FluorescentIndex;

    m_ISP_AWBInfo.rPregain1 = rAWBInfo.rPregain1;
    memcpy(&m_ISP_AWBInfo.rCscCCM, &rAWBInfo.rCscCCM, sizeof(rAWBInfo.rCscCCM));

    //m_ISP_AWBInfo.rRPG = rAWBInfo.rRPG;
    //m_ISP_AWBInfo.rPGN = rAWBInfo.rPGN;

    m_ISP_AWBInfo.bAWBLock = rAWBInfo.bAWBLock;

    return MTRUE;
}

MBOOL AwbMgr::getAWBInfo(AWB_ISP_INFO_T& rAWBInfo)
{
    memcpy(&rAWBInfo, &m_ISP_AWBInfo, sizeof(m_ISP_AWBInfo));
    return MTRUE;
}

MRESULT AwbMgr::configReg(AWBResultConfig_T *pResultConfig)
{
    memcpy(pResultConfig, &m_sAWBResultConfig, sizeof(AWBResultConfig_T));
    return S_AWB_OK;
}

MBOOL AwbMgr::getPostgain(AWB_GAIN_T& rPostgain)
{
    rPostgain.i4R = m_Postgain.i4R;
    rPostgain.i4G = m_Postgain.i4G;
    rPostgain.i4B = m_Postgain.i4B;

    return MTRUE;
}

MBOOL AwbMgr::setPostgain(AWB_GAIN_T rPostgain)
{
    if(!m_bPostgainLock)
    {
        m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4PostGainR = rPostgain.i4R;
        m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4PostGainG = rPostgain.i4G;
        m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4PostGainB = rPostgain.i4B;

        m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4PostGainR = rPostgain.i4R;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4PostGainG = rPostgain.i4G;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4PostGainB = rPostgain.i4B;

        AWB_LOG_IF(m_i4DgbLog, "[%s()] i4PostGain: %d/%d/%d \n", __FUNCTION__,
                   m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4PostGainR,m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4PostGainG,
                   m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4PostGainB);

        ISP_AWB_CONFIG_T::getInstance(m_eSensorDev).AWBConfig_update(&m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], &m_sAWBResultConfig);
    }

    return MTRUE;
}

MBOOL AwbMgr::setPostgainLock(MBOOL bPostgainLock)
{
    if(m_bPostgainLock != bPostgainLock)
    {
        AWB_LOG("[%s()] m_bPostgainLock: %d=>%d \n", __FUNCTION__, m_bPostgainLock, bPostgainLock);
        m_bPostgainLock = bPostgainLock;
    }

    return MTRUE;
}

MBOOL AwbMgr::getAWBStatInfo(AWB_STAT_INFO_T& rAWBStatInfo)
{
    rAWBStatInfo.i4WindowSizeX = m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4WindowSizeX;
    rAWBStatInfo.i4WindowSizeY = m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4WindowSizeY;
    rAWBStatInfo.i4LowThresholdR = m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4LowThresholdR;
    rAWBStatInfo.i4LowThresholdG = m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4LowThresholdG;
    rAWBStatInfo.i4LowThresholdB = m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4LowThresholdB;
    rAWBStatInfo.i4HighThresholdR = m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4HighThresholdR;
    rAWBStatInfo.i4HighThresholdG = m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4HighThresholdG;
    rAWBStatInfo.i4HighThresholdB = m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4HighThresholdB;
    rAWBStatInfo.i4LinearOutputEn = m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4StatMode;

    AWB_LOG_IF(m_i4DgbLog, "[%s()] winSizeX/winSizeY: %d/%d LowThreshold: %d/%d/%d HighThreshold: %d/%d/%d i4LinearOutputEn: %d \n", __FUNCTION__,
              rAWBStatInfo.i4WindowSizeX, rAWBStatInfo.i4WindowSizeY, rAWBStatInfo.i4LowThresholdR, rAWBStatInfo.i4LowThresholdG, rAWBStatInfo.i4LowThresholdB,
              rAWBStatInfo.i4HighThresholdR, rAWBStatInfo.i4HighThresholdG, rAWBStatInfo.i4HighThresholdB, rAWBStatInfo.i4LinearOutputEn);

    return MTRUE;
}

MBOOL AwbMgr::setAWBStatInfo(AWB_STAT_INFO_T rAWBStatInfo)
{
    m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode].i4StatMode =  rAWBStatInfo.i4LinearOutputEn;
    m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode].i4StatMode =  rAWBStatInfo.i4LinearOutputEn;

    AWB_LOG_IF(m_i4DgbLog, "[%s()] i4LinearOutputEn: %d \n", __FUNCTION__,
               m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode].i4StatMode);

    ISP_AWB_CONFIG_T::getInstance(m_eSensorDev).AWBConfig_update(&m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], &m_sAWBResultConfig);

    return MTRUE;
}

MINT32 AwbMgr::updateRTVString(char* stringBuffer)
{
    #define MAX_EXTRACOLOR_NUM  (8)
    MINT32 totalCount=0;
    MINT32 tmpEnum;
    MINT32 tmpValue;
    AWB_RT_DEBUG_INFO_T rAWBRTDebugInfo;

    m_pIAwbAlgo->getRTDebugInfo(rAWBRTDebugInfo);

    if(!stringBuffer)
    {
        return totalCount;
    }
    else
    {
        char tmpString[32] = {"\0"};
        MINT32 n = sprintf(tmpString, "AWB[%d]ReqNum[%d]:", m_eSensorDev, m_i4MagicNum);
        MINT32 totalLength = 0;
        strncpy(stringBuffer, tmpString, n);
        MINT32 startId, endId;

        switch(m_i4RtvGroupId) {
            case 1: // Probability info
                startId = (int)E_RTV_AWB_P_STB;
                endId = (int)E_RTV_AWB_P_DF;
                break;
            case 2: // High,low CCT offset
                startId = (int)E_RTV_AWB_DAY_LOCUS_OFFSET_T;
                endId = (int)E_RTV_AWB_NEW_OFFSET_S;
                break;
            case 3: // Extra color
                startId = (int)E_RTV_AWB_EXTRACOLOR0_WT;
                endId = (int)E_RTV_AWB_SKYDETECT_LUTIDX;
                break;
            case 4: // FD general
                startId = (int)E_RTV_AWB_FACE_DETECT;
                endId = (int)E_RTV_AWB_FACEAST_GAINPROB_SUM;
                break;
            case 5: // FD detail
                startId = (int)E_RTV_AWB_FACEPREF_PROB;
                endId = (int)E_RTV_AWB_FACEAST_SHIFT;
                break;
            case 6: // All
                startId = (int)E_RTV_AWB_SCENE_LV;
                endId = (int)E_RTV_AWB_FACEAST_SHIFT;
                break;
            case 0: // Basic
            default:
                startId = (int)E_RTV_AWB_SCENE_LV;
                endId = (int)E_RTV_AWB_NOPREF_GAIN_B;
                break;

        }

        AWB_LOG_IF(m_i4DgbLog, "%s GroupId(%d): [%d] - [%d]", __FUNCTION__, m_i4RtvGroupId, startId, endId);

        MINT32 i4ExtraColorCount = 0;

        for(int i = startId; i <= endId; i++) {
            tmpEnum = i;
            tmpValue = -1;

            if(((i >= E_RTV_AWB_EXTRACOLOR0_WT) && (i <= E_RTV_AWB_EXTRACOLOR15_WT)) &&
                ((rAWBRTDebugInfo.i4ExtraColorWt[i - E_RTV_AWB_EXTRACOLOR0_WT] == 0) ||
                (i4ExtraColorCount >= MAX_EXTRACOLOR_NUM)))
            {
                /*
                * Only record 8 extra color windows
                */
                continue;
            }

            memset(tmpString, 0, sizeof(tmpString));
            switch(i)
            {
                case E_RTV_AWB_SCENE_LV:
                    tmpValue = m_rAWBOutput.rAWBInfo.i4SceneLV;
                    break;
                case E_RTV_AWB_RELIABLE_MODE:
                    tmpValue = rAWBRTDebugInfo.i4ReliableMode;
                    break;
                case E_RTV_AWB_GAIN_R:
                    tmpValue = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R;
                    break;
                case E_RTV_AWB_GAIN_G:
                    tmpValue = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G;
                    break;
                case E_RTV_AWB_GAIN_B:
                    tmpValue = m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B;
                    break;
                case E_RTV_AWB_CCT:
                    tmpValue = m_rAWBOutput.rAWBInfo.i4CCT;
                    break;
                case E_RTV_AWB_NOPREF_GAIN_R:
                    tmpValue = rAWBRTDebugInfo.i4AWBNoPrefGainR;
                    break;
                case E_RTV_AWB_NOPREF_GAIN_G:
                    tmpValue = rAWBRTDebugInfo.i4AWBNoPrefGainG;
                    break;
                case E_RTV_AWB_NOPREF_GAIN_B:
                    tmpValue = rAWBRTDebugInfo.i4AWBNoPrefGainB;
                    break;
                case E_RTV_AWB_DAY_LOCUS_OFFSET_T:
                    tmpValue = rAWBRTDebugInfo.i4DaylightLocusOffsetT;
                    break;
                case E_RTV_AWB_NEW_OFFSET_T:
                    tmpValue = rAWBRTDebugInfo.i4DaylightLocusNewOffsetT;
                    break;
                case E_RTV_AWB_DAY_LOCUS_OFFSET_WF:
                    tmpValue = rAWBRTDebugInfo.i4DaylightLocusOffsetWF;
                    break;
                case E_RTV_AWB_NEW_OFFSET_WF:
                    tmpValue = rAWBRTDebugInfo.i4DaylightLocusNewOffsetWF;
                    break;
                case E_RTV_AWB_DAY_LOCUS_OFFSET_S:
                    tmpValue = rAWBRTDebugInfo.i4DaylightLocusOffsetS;
                    break;
                case E_RTV_AWB_NEW_OFFSET_S:
                    tmpValue = rAWBRTDebugInfo.i4DaylightLocusNewOffsetS;
                    break;
                case E_RTV_AWB_P_STB:
                    tmpValue = m_rAWBOutput.rAWBInfo.rProb.i4P[AWB_LIGHT_STROBE];
                    break;
                case E_RTV_AWB_P_T:
                    tmpValue = m_rAWBOutput.rAWBInfo.rProb.i4P[AWB_LIGHT_T];
                    break;
                case E_RTV_AWB_P_WF:
                    tmpValue = m_rAWBOutput.rAWBInfo.rProb.i4P[AWB_LIGHT_WF];
                    break;
                case E_RTV_AWB_P_F:
                    tmpValue = m_rAWBOutput.rAWBInfo.rProb.i4P[AWB_LIGHT_F];
                    break;
                case E_RTV_AWB_P_CWF:
                    tmpValue = m_rAWBOutput.rAWBInfo.rProb.i4P[AWB_LIGHT_CWF];
                    break;
                case E_RTV_AWB_P_D:
                    tmpValue = m_rAWBOutput.rAWBInfo.rProb.i4P[AWB_LIGHT_DAYLIGHT];
                    break;
                case E_RTV_AWB_P_S:
                    tmpValue = m_rAWBOutput.rAWBInfo.rProb.i4P[AWB_LIGHT_SHADE];
                    break;
                case E_RTV_AWB_P_DF:
                    tmpValue = m_rAWBOutput.rAWBInfo.rProb.i4P[AWB_LIGHT_DF];
                    break;
                case E_RTV_AWB_EXTRACOLOR0_WT:
                    i4ExtraColorCount++;
                    tmpValue = rAWBRTDebugInfo.i4ExtraColorWt[0];
                    break;
                case E_RTV_AWB_EXTRACOLOR1_WT:
                    i4ExtraColorCount++;
                    tmpValue = rAWBRTDebugInfo.i4ExtraColorWt[1];
                    break;
                case E_RTV_AWB_EXTRACOLOR2_WT:
                    i4ExtraColorCount++;
                    tmpValue = rAWBRTDebugInfo.i4ExtraColorWt[2];
                    break;
                case E_RTV_AWB_EXTRACOLOR3_WT:
                    i4ExtraColorCount++;
                    tmpValue = rAWBRTDebugInfo.i4ExtraColorWt[3];
                    break;
                case E_RTV_AWB_EXTRACOLOR4_WT:
                    i4ExtraColorCount++;
                    tmpValue = rAWBRTDebugInfo.i4ExtraColorWt[4];
                    break;
                case E_RTV_AWB_EXTRACOLOR5_WT:
                    i4ExtraColorCount++;
                    tmpValue = rAWBRTDebugInfo.i4ExtraColorWt[5];
                    break;
                case E_RTV_AWB_EXTRACOLOR6_WT:
                    i4ExtraColorCount++;
                    tmpValue = rAWBRTDebugInfo.i4ExtraColorWt[6];
                    break;
                case E_RTV_AWB_EXTRACOLOR7_WT:
                    i4ExtraColorCount++;
                    tmpValue = rAWBRTDebugInfo.i4ExtraColorWt[7];
                    break;
                case E_RTV_AWB_EXTRACOLOR8_WT:
                    i4ExtraColorCount++;
                    tmpValue = rAWBRTDebugInfo.i4ExtraColorWt[8];
                    break;
                case E_RTV_AWB_EXTRACOLOR9_WT:
                    i4ExtraColorCount++;
                    tmpValue = rAWBRTDebugInfo.i4ExtraColorWt[9];
                    break;
                case E_RTV_AWB_EXTRACOLOR10_WT:
                    i4ExtraColorCount++;
                    tmpValue = rAWBRTDebugInfo.i4ExtraColorWt[10];
                    break;
                case E_RTV_AWB_EXTRACOLOR11_WT:
                    i4ExtraColorCount++;
                    tmpValue = rAWBRTDebugInfo.i4ExtraColorWt[11];
                    break;
                case E_RTV_AWB_EXTRACOLOR12_WT:
                    i4ExtraColorCount++;
                    tmpValue = rAWBRTDebugInfo.i4ExtraColorWt[12];
                    break;
                case E_RTV_AWB_EXTRACOLOR13_WT:
                    i4ExtraColorCount++;
                    tmpValue = rAWBRTDebugInfo.i4ExtraColorWt[13];
                    break;
                case E_RTV_AWB_EXTRACOLOR14_WT:
                    i4ExtraColorCount++;
                    tmpValue = rAWBRTDebugInfo.i4ExtraColorWt[14];
                    break;
                case E_RTV_AWB_EXTRACOLOR15_WT:
                    i4ExtraColorCount++;
                    tmpValue = rAWBRTDebugInfo.i4ExtraColorWt[15];
                    break;
                case E_RTV_AWB_SKYDETECT_WT:
                    tmpValue = rAWBRTDebugInfo.i4SkyDetectWt;
                    break;
                case E_RTV_AWB_SKYDETECT_LUTIDX:
                    tmpValue = rAWBRTDebugInfo.i4SkyDetectLutIdx;
                    break;
                case E_RTV_AWB_FACE_DETECT:
                    tmpValue = rAWBRTDebugInfo.i4FaceDetect;
                    break;
                case E_RTV_AWB_FACEPREF_EXECUTE:
                    tmpValue = rAWBRTDebugInfo.i4FacePrefExecute;
                    break;
                case E_RTV_AWB_FACEAST_ENABLE:
                    tmpValue = rAWBRTDebugInfo.i4FaceAstEnable;
                    break;
                case E_RTV_AWB_FACE_WT_AVG_R:
                    tmpValue = rAWBRTDebugInfo.i4FaceWTAvgR;
                    break;
                case E_RTV_AWB_FACE_WT_AVG_G:
                    tmpValue = rAWBRTDebugInfo.i4FaceWTAvgG;
                    break;
                case E_RTV_AWB_FACE_WT_AVG_B:
                    tmpValue = rAWBRTDebugInfo.i4FaceWTAvgB;
                    break;
                case E_RTV_AWB_FACEAST_SIZE_RATIO:
                    tmpValue = rAWBRTDebugInfo.i4FaceAstSizeRatio;
                    break;
                case E_RTV_AWB_FACEAST_FC_RATIO:
                    tmpValue = rAWBRTDebugInfo.i4FaceAstFCRatio;
                    break;
                case E_RTV_AWB_FACEPREF_GAIN_RATIO_REST:
                    tmpValue = rAWBRTDebugInfo.i4FaceAstGainRatioRest;
                    break;
                case E_RTV_AWB_FACEAST_GAINPROB_SUM: {
                    MINT32 i4ProbSum = 0;
                    i4ProbSum += rAWBRTDebugInfo.i4FinalGainProbT;
                    i4ProbSum += rAWBRTDebugInfo.i4FinalGainProbWF;
                    i4ProbSum += rAWBRTDebugInfo.i4FinalGainProbF;
                    i4ProbSum += rAWBRTDebugInfo.i4FinalGainProbCWF;
                    i4ProbSum += rAWBRTDebugInfo.i4FinalGainProbDaylight;
                    i4ProbSum += rAWBRTDebugInfo.i4FinalGainProbS;
                    i4ProbSum += rAWBRTDebugInfo.i4FinalGainProbDF;
                    tmpValue = i4ProbSum;}
                    break;
                case E_RTV_AWB_FACEPREF_PROB:
                    tmpValue = rAWBRTDebugInfo.i4FacePrefProb;
                    break;
                case E_RTV_AWB_FACEPREF_GRATIO_RG:
                    tmpValue = rAWBRTDebugInfo.i4FacePrefGRatioRG;
                    break;
                case E_RTV_AWB_FACEPREF_GRATIO_BG:
                    tmpValue = rAWBRTDebugInfo.i4FacePrefGRatioBG;
                    break;
                case E_RTV_AWB_FACEPREF_DGAIN_R: {
                    tmpValue = (rAWBRTDebugInfo.i4FacePrefTargetGainR * 512 + (rAWBRTDebugInfo.i4FacePrefOriginGainR >> 1))
                                / rAWBRTDebugInfo.i4FacePrefOriginGainR;}
                    break;
                case E_RTV_AWB_FACEPREF_DGAIN_G: {
                    tmpValue = (rAWBRTDebugInfo.i4FacePrefTargetGainG * 512 + (rAWBRTDebugInfo.i4FacePrefOriginGainG >> 1))
                                / rAWBRTDebugInfo.i4FacePrefOriginGainG;}
                    break;
                case E_RTV_AWB_FACEPREF_DGAIN_B: {
                    tmpValue = (rAWBRTDebugInfo.i4FacePrefTargetGainB * 512 + (rAWBRTDebugInfo.i4FacePrefOriginGainB >> 1))
                                / rAWBRTDebugInfo.i4FacePrefOriginGainB;}
                    break;
                case E_RTV_AWB_FACEAST_SHIFT:
                    tmpValue = rAWBRTDebugInfo.i4FaceAstShift;
                    break;
                default:
                    tmpEnum = E_RTV_AWB_SCENE_LV;
                    tmpValue = -2;
                    break;
            }

            MINT32 n = sprintf(tmpString, "%d,%d;", tmpEnum, tmpValue);
            strncat(stringBuffer, tmpString, n);
        }
        totalCount = strlen(stringBuffer);
    }

    AWB_LOG_IF(m_i4DgbLog, "%s total(%d): %s", __FUNCTION__, totalCount, stringBuffer);

    #undef MAX_EXTRACOLOR_NUM

    return totalCount;

}

