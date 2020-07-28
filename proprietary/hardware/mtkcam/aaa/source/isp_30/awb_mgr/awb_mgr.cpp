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

#include <cutils/properties.h>
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
#include <isp_mgr.h>
#include <isp_tuning_mgr.h>
//#include <kd_camera_feature.h>
#include <isp_tuning.h>
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
#include "ae_param_flow.h"
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <StatisticBuf.h>

#define MWBMAX 9000
#define MWBMIN 2000
///#ifdef MY_LOG
///    #undef MY_LOG
///#endif

///#define CAM_LOGD(fmt, arg...) { if(gIsLogEn) {ALOGD(fmt, ##arg);} }



#if 1//iopipe2.0
#define IOPIPE_SET_MODUL_REG(handle,RegName,Value)
#define IOPIPE_SET_MODUL_REGS(handle, StartRegName, size, ValueArray)
namespace NSImageio {
    namespace NSIspio {
        enum EModule
        {
            //raw
            EModule_OB          = 00,
            EModule_BNR         = 05,
            EModule_LSC         = 10,
            EModule_RPG         = 15,
            EModule_AE          = 20,
            EModule_AWB         = 25,
            EModule_SGG1        = 30,
            EModule_FLK         = 35,
            EModule_AF          = 40,
            EModule_SGG2        = 45,
            EModule_SGG3        = 46,
            EModule_EIS         = 50,
            EModule_LCS         = 55,
            EModule_BPCI        = 60,
            EModule_LSCI        = 65,
            EModule_AAO         = 70,
            EModule_ESFKO       = 75,
            EModule_AFO         = 80,
            EModule_EISO        = 85,
            EModule_LCSO        = 90,
            EModule_iHDR        = 95,
            EModule_CAMSV_IMGO  = 100,
            //raw_d
            EModule_OB_D        = 1000,
            EModule_BNR_D         = 1005,
            EModule_LSC_D       = 1010,
            EModule_RPG_D       = 1015,
            EModule_BPCI_D      = 1020,
            EModule_LSCI_D      = 1025,
            EModule_AE_D        = 1030,
            EModule_AWB_D       = 1035,
            EModule_SGG1_D      = 1040,
            EModule_AF_D        = 1045,
            EModule_LCS_D       = 1050,
            EModule_AAO_D       = 1055,
            EModule_AFO_D       = 1060,
            EModule_LCSO_D      = 1065,
            EModule_iHDR_D      = 1070
        };
        enum EPIPECmd {

            //  IPECmd_SET_SENSOR_DEV             = 0x1001,
            //  EPIPECmd_SET_SENSOR_GAIN            = 0x1002,
            //  EPIPECmd_SET_SENSOR_EXP             = 0x1003,
            //  EPIPECmd_SET_CAM_MODE               = 0x1004,
            EPIPECmd_SET_SCENE_MODE             = 0x1005,
            EPIPECmd_SET_ISO                    = 0x1006,
            EPIPECmd_SET_FLUORESCENT_CCT        = 0x1007,
            EPIPECmd_SET_SCENE_LIGHT_VALUE      = 0x1008,
            EPIPECmd_VALIDATE_FRAME             = 0x1009,
            EPIPECmd_SET_OPERATION_MODE         = 0x100A,
            EPIPECmd_SET_EFFECT                 = 0x100B,
            EPIPECmd_SET_ZOOM_RATIO             = 0x100C,
            EPIPECmd_SET_BRIGHTNESS             = 0x100D,
            EPIPECmd_SET_CONTRAST               = 0x100E,
            EPIPECmd_SET_EDGE                   = 0x100F,
            EPIPECmd_SET_HUE                    = 0x1010,
            EPIPECmd_SET_SATURATION             = 0x1011,
            EPIPECmd_SEND_TUNING_CMD            = 0x1012,
            EPIPECmd_DECIDE_OFFLINE_CAPTURE     = 0x1013,
            EPIPECmd_LOCK_REG                   = 0x1014,
            EPIPECmd_SET_SHADING_IDX            = 0x1018,

            EPIPECmd_SET_RRZ                    = 0x101A,
            EPIPECmd_SET_P1_UPDATE              = 0x101B,
            EPIPECmd_SET_IMGO                   = 0x101C,

            EPIPECmd_SET_BASE_ADDR              = 0x1102,
            EPIPECmd_SET_CQ_CHANNEL             = 0x1103,
            EPIPECmd_SET_CQ_TRIGGER_MODE        = 0x1104,
            EPIPECmd_AE_SMOOTH                  = 0x1105,
            EPIPECmd_SET_FMT_EN                 = 0x1107,
            EPIPECmd_SET_GDMA_LINK_EN           = 0x1108,
            EPIPECmd_SET_FMT_START              = 0x1109,
            EPIPECmd_SET_CAM_CTL_DBG            = 0x110A,
            EPIPECmd_SET_IMG_PLANE_BY_IMGI      = 0x110B,
            EPIPECmd_SET_P2_QUEUE_CONTROL_STATE = 0x110C,

            EPIPECmd_GET_TG_OUT_SIZE            = 0x110F,
            EPIPECmd_GET_RMX_OUT_SIZE           = 0x1110,
            EPIPECmd_GET_HBIN_INFO              = 0x1111,
            EPIPECmd_GET_EIS_INFO               = 0x1112,
            EPIPECmd_GET_SUGG_BURST_QUEUE_NUM   = 0x1114,

            EPIPECmd_GET_TWIN_INFO              = 0x1116,

            EPIPECmd_SET_EIS_CBFP               = 0X1117,
            EPIPECmd_SET_LCS_CBFP               = 0X1118,
            EPIPECmd_SET_SGG2_CBFP              = 0X1119,

            EPIPECmd_GET_CUR_FRM_STATUS         = 0x111D,
            EPIPECmd_GET_CUR_SOF_IDX            = 0x111E,

            EPIPECmd_ALLOC_UNI                  = 0x1200,
            EPIPECmd_DEALLOC_UNI                = 0x1201,
            //EPIPECmd_ALLOC_FLK_PATH             = 0x1200,
            //EPIPECmd_DEALLOC_FLK_PATH           = 0x1201,
            //EPIPECmd_ALLOC_HDS_PATH             = 0x1202,
            //EPIPECmd_DEALLOC_HDS_PATH           = 0x1203,

            EPIPECmd_SET_NR3D_EN                = 0x1300,
            EPIPECmd_SET_NR3D_DMA_SEL           = 0x1301,
            EPIPECmd_SET_CRZ_EN                 = 0x1302,
            EPIPECmd_SET_JPEG_CFG               = 0x1303,
            EPIPECmd_SET_JPEG_WORKBUF_SIZE      = 0x1304,

            EPIPECmd_SET_MODULE_EN              = 0x1401, //phase out
            EPIPECmd_SET_MODULE_SEL             = 0x1402, //phase out
            EPIPECmd_SET_MODULE_CFG             = 0x1403, //phase out
            EPIPECmd_GET_MODULE_HANDLE          = 0x1404, //phase out
            EPIPECmd_SET_MODULE_CFG_DONE        = 0x1405, //phase out
            EPIPECmd_RELEASE_MODULE_HANDLE      = 0x1406, //phase out
            EPIPECmd_SET_MODULE_DBG_DUMP        = 0x1407, //phase out


            EPIPECmd_GET_SENSOR_PRV_RANGE       = 0x2001,
            EPIPECmd_GET_SENSOR_FULL_RANGE      = 0x2002,
            EPIPECmd_GET_RAW_DUMMY_RANGE        = 0x2003,
            EPIPECmd_GET_SENSOR_NUM             = 0x2004,
            EPIPECmd_GET_SENSOR_TYPE            = 0x2005,
            EPIPECmd_GET_RAW_INFO               = 0x2006,
            EPIPECmd_GET_EXIF_DEBUG_INFO        = 0x2007,
            EPIPECmd_GET_SHADING_IDX            = 0x2008,
            EPIPECmd_GET_ATV_DISP_DELAY         = 0x2009,
            EPIPECmd_GET_SENSOR_DELAY_FRAME_CNT = 0x200A,
            EPIPECmd_GET_CAM_CTL_DBG            = 0x200B,
            EPIPECmd_GET_FMT                    = 0x200C,
            EPIPECmd_GET_GDMA                   = 0x200D,
            EPIPECmd_GET_NR3D_GAIN              = 0x200E,
            EPIPECmd_ISP_RESET                  = 0x4001,
            EPIPECmd_MAX                        = 0xFFFF

        };
    };
};
#endif

using namespace NS3A;
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSIspTuningv3;
//using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;


#define AWB_LOG(fmt, arg...) \
    do { \
        if (m_i4DgbLog) { \
            MY_LOGE(fmt, ##arg); \
        } else { \
            CAM_LOGD(fmt, ##arg); \
        } \
    }while(0)

#define AWB_LOG_IF(cond, ...) \
    do { \
        if (m_i4DgbLog) { \
            MY_LOGE_IF(cond, __VA_ARGS__); \
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
    case ESensorDev_MainThird: //  Main Third Sensor
        return	AwbMgrDev<ESensorDev_MainThird>::getInstance();

    default:
        CAM_LOGD("i4SensorDev = %d", i4SensorDev);
        if (AwbMgr::s_pAwbMgr)
            return  *AwbMgr::s_pAwbMgr;
        else
            return  AwbMgrDev<ESensorDev_Main>::getInstance();
    }
}


#if 0
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
#endif
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
    , m_i4AWBNvramIdx(AWBNVRAMMapping[CAM_SCENARIO_PREVIEW])
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
    , m_i4AAOmode(0)
    , m_bFDenable(MFALSE)
    , m_bFaceAWBAreaChage(MFALSE)
    , m_bFlashAwb(MFALSE)
    , m_bAlgoInit(MFALSE)
    , mIsMwbCctChanged(0)
    , mMWBColorTemperature(0)
    , m_bIsTorch(MFALSE)
    , m_bIsMainFlashOn(MFALSE)
    , m_bIsUseLastGain(1)
    , AAOStride(0)
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
    case ESensorDev_MainThird: //  Main Third Sensor
        m_bEnableAWB = isAWBEnabled<ESensorDev_MainThird>();
        break;
    default:
        CAM_LOGE("eSensorDev = %d", eSensorDev);
        break;
    }
    mpAao = new MUINT8[54000];
    mpHist = new MUINT8[2048];

    memset(&m_rAWBStatParam, 0, sizeof(m_rAWBStatParam));
    memset(&m_rAWBRAWPreGain1, 0, sizeof(m_rAWBRAWPreGain1));
    memset(&m_rAWBInitInput, 0, sizeof(m_rAWBInitInput));
    memset(&m_rAWBOutput, 0, sizeof(m_rAWBOutput));

    memset(&m_eAWBFDArea, 0, sizeof(AWBMeteringArea_T));
    memset(&m_eZoomWinInfo, 0, sizeof(AWBZOOM_WINDOW_T));

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



//#define setRgbGainX(a, b)  { setRgbGain(a,b);}
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

    AWB_LOG("awb gain %d %d %d %d",i4SensorDev, rIspAWBGain.i4R,rIspAWBGain.i4G, rIspAWBGain.i4B);

    //AAASensorMgr::getInstance().setSensorAWBGain((ESensorDev_T)i4SensorDev, &rSensorAWBGain);


    return IspTuningMgr::getInstance().setAWBGain(i4SensorDev, rIspAWBGain);

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
  IspTuningMgr::getInstance().setIspAEPreGain2(i4SensorDev, sensorIdx, rPreGain2);
  return 1;
}


MBOOL AwbMgr::setIspAwbInfo(MINT32 const i4SensorDev, AWB_INFO_T &rAWBInfo)
{
  if(mIsMono)
  {
    rAWBInfo.rCurrentAWBGain.i4R=512;
    rAWBInfo.rCurrentAWBGain.i4G=512;
    rAWBInfo.rCurrentAWBGain.i4B=512;
  }


//  MUINT32 rCscCCM[9] = {0x200, 0, 0, 0, 0x200, 0, 0, 0, 0x200};// Bayer
 // ::memcpy(&m_rAWBOutput.rAWBInfo.rCscCCM, &rCscCCM, sizeof(rCscCCM));

  AWB_LOG( "[%s()] m_eSensorDev: %d, awb_gain: %d/%d/%d, (m_bAWBLock:%d, m_eAWBMode:%d)\n",
    __FUNCTION__, m_eSensorDev,
    rAWBInfo.rCurrentAWBGain.i4R, rAWBInfo.rCurrentAWBGain.i4G, rAWBInfo.rCurrentAWBGain.i4B,
    m_bAWBLock, m_eAWBMode);

  IspTuningMgr::getInstance().setAWBInfo(i4SensorDev, rAWBInfo);
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

MBOOL AwbMgr::cameraPreviewInit(MINT32 i4SensorIdx, Param_T &rParam, IAWBInitPara* initPara)
{
    (void)rParam;
    (void)initPara;

    AWB_LOG("[%s()] m_eSensorDev: %d, i4SensorIdx = %d, m_eSensorMode = %d\n", __FUNCTION__, m_eSensorDev, i4SensorIdx, m_eSensorMode);

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.awb_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    // sensor index
    m_i4SensorIdx = i4SensorIdx;

    // set strobe mode to OFF
    setStrobeMode(AWB_STROBE_MODE_OFF);

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

    //non-bayer CCM (RWB)
#if 0
    MUINT32 rCscCCM[9] = {0x200, 0, 0, 0, 0x200, 0, 0, 0, 0x200};// Bayer
    if ( SENSOR_RAW_RWB  == mSensorType) // rwb
        ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).SetNonBayer((MUINT32*)m_rAWBInitInput.rAWBParam.rNoneBayer.i4RWBCof);
    else
        ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).SetNonBayer(rCscCCM);
#endif
    // AWB statistics config
    ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).config(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
        , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY, m_i4SensorIdx);

    // update AE RAW pre-gain2
    setAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);
    // IspTuningMgr::getInstance().setIspAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);

    // update AWB gain
    //  IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rPreviewAWBGain);
    //setRgbGainX(m_eSensorDev, m_rAWBOutput.rPreviewAWBGain);

    // force ISP validate
    IspTuningMgr::getInstance().forceValidate(m_eSensorDev);

    return MTRUE;
}

MBOOL AwbMgr::init(MINT32 i4SensorIdx, IAWBInitPara* initPara)
{
    (void)initPara;

    AWB_LOG("[%s()] m_eSensorDev: %d, i4SensorIdx = %d, m_eSensorMode = %d\n", __FUNCTION__, m_eSensorDev, i4SensorIdx, m_eSensorMode);

    m_i4SensorIdx = i4SensorIdx;

    m_i4DgbLog  = property_get_int32("vendor.debug.awb_log.enable", 0);

    m_bIsUseLastGain  = property_get_int32("vendor.debug.awb_last_gain.enable", 1);
    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::camcorderPreviewInit(MINT32 i4SensorIdx, Param_T &rParam, IAWBInitPara* initPara)
{
    (void)initPara;

    AWB_LOG("[%s()] m_eSensorDev: %d, i4SensorIdx = %d, m_eSensorMode = %d\n", __FUNCTION__, m_eSensorDev, i4SensorIdx, m_eSensorMode);

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.awb_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    // sensor index
    m_i4SensorIdx = i4SensorIdx;

    AWB_LOG("rParam.u4StrobeMode = %d\n", rParam.u4StrobeMode);
    if (rParam.u4StrobeMode == LIB3A_FLASH_MODE_FORCE_TORCH) // Torch mode
        setStrobeMode(AWB_STROBE_MODE_ON);
    else
        setStrobeMode(AWB_STROBE_MODE_OFF);

    // Get sensor resolution
    getSensorResolution();

    // Get NVRAM data
    if (!getNvramData()) {
        CAM_LOGE("getNvramData() fail\n");
        return MFALSE;
    }

    // AWB init
    if (!AWBInit()) {
        CAM_LOGE("AWBInit() fail\n");
        return MFALSE;
    }

    // AWB statistics config
    ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).config(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
        , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY, m_i4SensorIdx);

    // update AE RAW pre-gain2
    setAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);
    // IspTuningMgr::getInstance().setIspAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);

    // update AWB gain
    //  IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rPreviewAWBGain);
    //setRgbGainX(m_eSensorDev, m_rAWBOutput.rPreviewAWBGain);

    // force ISP validate
    IspTuningMgr::getInstance().forceValidate(m_eSensorDev);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::cameraCaptureInit(IAWBInitPara* initPara)
{
    (void)initPara;

    AWB_LOG("[%s()] m_eSensorDev: %d\n", __FUNCTION__, m_eSensorDev);

    queryHBinInfo();

    if (m_bEnableAWB && (m_eAWBMode != LIB3A_AWB_MODE_OFF))
    {
        if (m_i4StrobeMode == AWB_STROBE_MODE_ON) {
            m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode],
                m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
                m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);

            // update AWB statistics config
            ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).config(m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
                , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY, m_i4SensorIdx);
        }
        else {
            m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode],
                m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
                m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);

            // update AWB statistics config
            ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).config(m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
                , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY, m_i4SensorIdx);
        }

        // force ISP validate
        IspTuningMgr::getInstance().forceValidate(m_eSensorDev);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::cameraPreviewReinit(IAWBInitPara* initPara)
{
    (void)initPara;

    AWB_LOG("[%s()] m_eSensorDev: %d\n", __FUNCTION__, m_eSensorDev);

    // set strobe mode to OFF: temp added
    setStrobeMode(AWB_STROBE_MODE_OFF);

    m_bStrobeModeChanged = MFALSE;
    m_bAWBModeChanged = MFALSE;
    m_bOneShotAWB = MTRUE; // do one-shot AWB
    m_bInitState = MTRUE; // init state

    // AWB statistics config
    ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).config(m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
        , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY, m_i4SensorIdx);

    // update AE RAW pre-gain2
    setAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);
    // IspTuningMgr::getInstance().setIspAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);

    // update AWB gain
    //  IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rPreviewAWBGain);
    //setRgbGainX(m_eSensorDev, m_rAWBOutput.rPreviewAWBGain);

    // force ISP validate
    IspTuningMgr::getInstance().forceValidate(m_eSensorDev);

    return MTRUE;
}
MBOOL AwbMgr::start()
{//capture init/ preview reinit
    AWB_LOG_IF(m_i4DgbLog,"start ln=%d senDev=%d",__LINE__,m_eSensorDev);

    // set strobe mode to OFF
    setStrobeMode(AWB_STROBE_MODE_OFF);

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
    if (m_i4StrobeMode == AWB_STROBE_MODE_ON) {
        m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode],
            m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
            m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);

        // update AWB statistics config
        ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).config(m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
            , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY, m_i4SensorIdx);
    }
    else {
        m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode],
            m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
            m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);

        // update AWB statistics config
        ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).config(m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
            , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY, m_i4SensorIdx);
    }

    // update AE RAW pre-gain2
    IspTuningMgr::getInstance().setIspAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);

    if (m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R == 0)
    {

       if ((1 == m_bIsUseLastGain) && (0 != LastCCT))
       {
          CalSencondSensorDefaultGain(m_rAWBOutput.rAWBInfo.rCurrentAWBGain, LastCCT);
          AWB_LOG_IF(m_i4DgbLog,"[%s] Use last gain = %d/%d/%d cct :%d",__FUNCTION__,m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R,
                   m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G, m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B, LastCCT);

       }
       else
       {
         m_rAWBOutput.rAWBInfo.rCurrentAWBGain = m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rDNP;
         AWB_LOG_IF(m_i4DgbLog,"[%s] DNP default gain = %d/%d/%d",__FUNCTION__,m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R,
                   m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G, m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B);
       }
    }
    // update AWB gain
    IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rPreviewAWBGain);
    //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
    setIspAwbInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);

    // force ISP validate
    IspTuningMgr::getInstance().forceValidate(m_eSensorDev);
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
    setIspAwbInfo(m_eSensorDev,m_rAWBOutput.rAWBInfo);
    LastCCT = m_rAWBOutput.rAWBInfo.i4CCT;

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

  ///  m_rAWBOutput.rAWBInfo.bAWBLock = (m_bAWBLock || (m_eAWBMode == LIB3A_AWB_MODE_OFF));
  ///  if(m_rAWBOutput.rAWBInfo.bAWBLock)
  if (m_bAWBLock) // tmp
    {
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
        AWB_LOG("[%d] m_i4StrobeMode: %d\n", m_eSensorDev, m_i4StrobeMode);
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

  //  m_rAWBOutput.rAWBInfo.bAWBLock = m_bAWBLock;
   // if(m_rAWBOutput.rAWBInfo.bAWBLock)
   if (m_bAWBLock) //tmp
    {
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
    IspTuningMgr::getInstance().setAWBGain(m_eSensorDev,mBackupGain);
    //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev,m_rAWBOutput.rAWBInfo);
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
MRESULT AwbMgr::doPvAWB(MINT32 i4FrameCount, MBOOL bAEStable, MINT32 i4SceneLV, MVOID *aaoBuf, MBOOL bApplyToHW)
{
    (void)i4FrameCount;

    //use setprop to decide AWB is controlled by Sync AWB algo or AWB mgr
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.awb_mgr.lock", value, "0");
    m_bAdbAWBLock = atoi(value);

    AWB_LOG_IF(m_i4DgbLog ,"[%s] i4FrameCount %d bAEStable %d i4SceneLV %d bApplyToHW %d", __FUNCTION__, i4FrameCount,
            bAEStable, i4SceneLV, bApplyToHW);
    AWB_INPUT_T rAWBInput;
    memset(&rAWBInput, 0, sizeof(rAWBInput));

    ///AWB_LOG("doPvAWB m_eColorCorrectionMode=%d ln=%d i4SceneLV= %d", m_eColorCorrectionMode, __LINE__,i4SceneLV);
    if (m_eColorCorrectionMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX)
    {
        AWB_LOG("doPvAWB ln=%d", __LINE__);
        AWB_LOG("rgain=%d ggain=%d bgain=%d",
            m_rColorCorrectionGain.i4R,
            m_rColorCorrectionGain.i4G,
            m_rColorCorrectionGain.i4B);
        m_rAWBOutput.rAWBInfo.rCurrentAWBGain = m_rColorCorrectionGain;
        IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rAWBInfo.rCurrentAWBGain);
        //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
        setIspAwbInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
    }
    else if ((m_bEnableAWB) &&
        (!m_bAWBLock) &&
        (!m_bAdbAWBLock) &&
        (m_eAWBMode != LIB3A_AWB_MODE_OFF))
    {
        AWB_LOG_IF(m_i4DgbLog ,"[%s] m_bAWBModeChanged mIsMwbCctChanged%d/%d \n", __FUNCTION__, m_bAWBModeChanged, mIsMwbCctChanged);
        //if ((m_bAWBModeChanged) && (!m_bInitState))
        if ((m_bAWBModeChanged) || mIsMwbCctChanged )//&& (!m_bInitState))
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


                m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode],
                                          m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
                                          m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);
                m_bAWBModeChanged = MFALSE;
                m_bOneShotAWB = MTRUE;
                //m_bSkipOneFrame = 3;
                mIsMwbCctChanged =  MFALSE;

                //update AWB statistics config
                ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).config(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
                , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY, m_i4SensorIdx);

            // force ISP validate
            IspTuningMgr::getInstance().forceValidate(m_eSensorDev);
        }
            else
            {
                m_pIAwbAlgo->setAWBMode(m_eAWBMode);
                m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode],
                                              m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
                                              m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);
                m_bAWBModeChanged = MFALSE;
                m_bOneShotAWB = MTRUE;
                //m_bSkipOneFrame = 3;
                mIsMwbCctChanged =  MFALSE;


                //update AWB statistics config
                ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).config(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
                , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY, m_i4SensorIdx);

                // force ISP validate
                IspTuningMgr::getInstance().forceValidate(m_eSensorDev);
            }
        }
        else if ((m_bStrobeModeChanged) && (!m_bInitState))
        {
            AWB_LOG("[%d] m_bStrobeModeChanged = %d, m_i4StrobeMode = %d\n", m_eSensorDev, m_bStrobeModeChanged, m_i4StrobeMode);
            AWB_LOG("[%s()][%d] update AWB statistics config", __FUNCTION__, m_eSensorDev);

            if (m_i4StrobeMode == AWB_STROBE_MODE_ON) {
                m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode],
                    m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
                    m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);
                // update AWB statistics config
                ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).config(m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
                    , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY, m_i4SensorIdx);
            }
            else {
                m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode],
                    m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
                    m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);
                // update AWB statistics config
                ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).config(m_rAWBStatCfg[AWB_STROBE_MODE_OFF][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
                    , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY, m_i4SensorIdx);
            }

            // restore for isp
            if (m_bFlashAwb){
                restore();
                m_bFlashAwb = MFALSE;
            }
            // force ISP validate
            IspTuningMgr::getInstance().forceValidate(m_eSensorDev);

            m_bStrobeModeChanged = MFALSE;
            m_bOneShotAWB = MTRUE;
            m_bSkipOneFrame = 3;
        }
        else if (m_bSkipOneFrame>0) { // skip one frame for AWB statistics ready
            m_bSkipOneFrame--;
            //m_bSkipOneFrame = MFALSE;
        }
        else {
            rAWBInput.bIsStrobeFired = (m_i4StrobeMode == AWB_STROBE_MODE_ON) ? MTRUE : MFALSE;
            rAWBInput.i4SceneLV = i4SceneLV;
            rAWBInput.i4AWBState = AWB_STATE_PREVIEW;
            //rAWBInput.pAWBStatBuf = pAWBStatBuf;
            rAWBInput.i4StatLineSize = AAOStride;
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
            //rAWBInput.i4StatLineSize = m_rAWBStatParam.i4WindowNumX*5;
            rAWBInput.pAWBStatBuf = aaoBuf;//mpAao
            rAWBInput.i4StatLineSize = 120*5;//120*8.5;
            ///rAWBInput.u4AWBAAOMode = m_i4AAOmode;
            rAWBInput.i4SensorMode = m_eSensorMode;
           /// rAWBInput.bAWBLock = m_bAWBLock;

            // Set FDarea to algorithm
            if (m_bFaceAWBAreaChage == MTRUE) {  // Face AWB
                m_pIAwbAlgo->setAWBFDArea(&m_eAWBFDArea);
                m_bFaceAWBAreaChage = MFALSE;
            }
            AAA_TRACE_G(handleAWB);
            AAA_TRACE_NL(handleAWB);
            m_pIAwbAlgo->handleAWB(rAWBInput, m_rAWBOutput);
            AAA_TRACE_END_NL;
            AAA_TRACE_END_G;

            // update AE RAW pre-gain2
            setAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);

            // set AWB info
            if (bApplyToHW)
                //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
                setIspAwbInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);

            if (m_bInitState && bAEStable) {
                m_bInitState = MFALSE;
            }

            // update AWB state
            if (m_rAWBOutput.rAWBInfo.bAWBStable)
                m_pAwbStateMgr->sendIntent(eIntent_AWBScanFinished);
            else
                m_pAwbStateMgr->sendIntent(eIntent_AWBScan);
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
        IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rAWBInfo.rCurrentAWBGain);
        //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
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

            // update AWB statistics config
            ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).config(m_rAWBStatCfg[m_i4StrobeMode/*AWB_STROBE_MODE_ON*/][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
                , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY, m_i4SensorIdx);

                ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).config(m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable
                     , mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY, m_i4SensorIdx);

            // force ISP validate
            IspTuningMgr::getInstance().forceValidate(m_eSensorDev);

            AWB_LOG("[%s()][%d] update AWB statistics config", __FUNCTION__, m_eSensorDev);
            //return S_AWB_OK;
        }

        //if (m_i4StrobeMode == AWB_STROBE_MODE_ON) {
        {
            ///rAWBInput.bIsStrobeFired = MTRUE;
            rAWBInput.bIsStrobeFired = (m_i4StrobeMode == AWB_STROBE_MODE_ON);
            rAWBInput.i4SceneLV = i4SceneLV;
            rAWBInput.i4AWBState = AWB_STATE_AF;
            //rAWBInput.pAWBStatBuf = pAWBStatBuf;
            rAWBInput.eAWBSpeedMode = AWB_SPEED_MODE_ONESHOT;
            //rAWBInput.i4StatLineSize = m_rAWBStatParam.i4WindowNumX*5;
            rAWBInput.pAWBStatBuf = aaoBuf;//mpAao
            rAWBInput.i4StatLineSize = 120*5;//120*8.5;
            ///rAWBInput.u4AWBAAOMode = m_i4AAOmode;
            rAWBInput.i4SensorMode = m_eSensorMode;
           /// rAWBInput.bAWBLock = m_bAWBLock;

            // Set FDarea to algorithm
            if (m_bFaceAWBAreaChage == MTRUE) {  // Face AWB
                m_pIAwbAlgo->setAWBFDArea(&m_eAWBFDArea);
                m_bFaceAWBAreaChage = MFALSE;
            }
            AAA_TRACE_G(handleAWB);
            AAA_TRACE_NL(handleAWB);
            m_pIAwbAlgo->handleAWB(rAWBInput, m_rAWBOutput);
            AAA_TRACE_END_NL;
            AAA_TRACE_END_G;
        }

        // update AE RAW pre-gain2
        setAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);

        // set AWB info
        if (bApplyToHW)
            //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
            setIspAwbInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);


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
        IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rAWBInfo.rCurrentAWBGain);
        //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
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
                ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).config(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable, mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY, m_i4SensorIdx);

        }
        else if ((m_i4StrobeMode == AWB_STROBE_MODE_ON) && (m_bStrobeModeChanged) && (m_eAWBMode != LIB3A_AWB_MODE_MWB))
        {
            m_pIAwbAlgo->setAWBStatConfig(m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode],
                m_rAWBWindowConfig.i4SensorWidth[m_eSensorMode],
                m_rAWBWindowConfig.i4SensorHeight[m_eSensorMode]);
            m_bStrobeModeChanged = FALSE;

            // update AWB statistics config
            ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).config(m_rAWBStatCfg[AWB_STROBE_MODE_ON][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable, mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY, m_i4SensorIdx);


            // force ISP validate
            IspTuningMgr::getInstance().forceValidate(m_eSensorDev);

            AWB_LOG("[%s()][%d] update AWB statistics config", __FUNCTION__, m_eSensorDev);
            return S_AWB_OK;
        }

        rAWBInput.bIsStrobeFired = (m_i4StrobeMode == AWB_STROBE_MODE_ON) ? MTRUE : MFALSE;
        rAWBInput.i4SceneLV = i4SceneLV;
        rAWBInput.i4AWBState = AWB_STATE_PRECAPTURE;
        //rAWBInput.pAWBStatBuf = pAWBStatBuf;
        rAWBInput.eAWBSpeedMode = AWB_SPEED_MODE_ONESHOT;
        //rAWBInput.i4StatLineSize = m_rAWBStatParam.i4WindowNumX*5;
        rAWBInput.pAWBStatBuf = aaoBuf;//mpAao
        rAWBInput.i4StatLineSize = 120*5;//120*8.5;
        ///rAWBInput.u4AWBAAOMode = m_i4AAOmode;
        rAWBInput.i4SensorMode = m_eSensorMode;
        ///rAWBInput.bAWBLock = m_bAWBLock;
        rAWBInput.i4StatLineSize = AAOStride;

        // Set FDarea to algorithm
        if (m_bFaceAWBAreaChage == MTRUE) {  // Face AWB
            m_pIAwbAlgo->setAWBFDArea(&m_eAWBFDArea);
            m_bFaceAWBAreaChage = MFALSE;
        }

        m_pIAwbAlgo->handleAWB(rAWBInput, m_rAWBOutput);

        // update AE RAW pre-gain2
        setAEPreGain2(m_eSensorDev, m_i4SensorIdx, m_rAWBOutput.rPreviewRAWPreGain2);
    
        // set AWB info
        if (bApplyToHW)
            //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
            setIspAwbInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);

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

    AWB_LOG(" doCapAWB : i4SceneLV= %d",i4SceneLV);

    StatisticBufInfo *paaobuf = reinterpret_cast<StatisticBufInfo *>(aaoBuf);
    MBOOL m_bAAOMainFlashOn = (paaobuf->mFlashState == MTK_FLASH_STATE_FIRED)  ? MTRUE : MFALSE;
    MBOOL bIsFlashFired = ((m_i4StrobeMode==AWB_STROBE_MODE_ON) || m_bIsMainFlashOn);
    AWB_LOG("[%s] i4SceneLV= %d m_bAAOMainFlashOn=%d bIsFlashFired=%d m_bIsMainFlashOn=%d\n", __FUNCTION__,
        i4SceneLV, m_bAAOMainFlashOn, bIsFlashFired, m_bIsMainFlashOn);

    if (m_eColorCorrectionMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX)
    {
        AWB_LOG("doCapAWB ln=%d", __LINE__);
        m_rAWBOutput.rAWBInfo.rCurrentAWBGain = m_rColorCorrectionGain;
        IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rAWBInfo.rCurrentAWBGain);
        //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
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
                ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).config(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable, mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY, m_i4SensorIdx);
       }
        rAWBInput.bIsStrobeFired = m_bAAOMainFlashOn;
        rAWBInput.i4SceneLV = i4SceneLV;
        rAWBInput.i4AWBState = AWB_STATE_CAPTURE;
        //rAWBInput.pAWBStatBuf = pAWBStatBuf;
        rAWBInput.eAWBSpeedMode = AWB_SPEED_MODE_ONESHOT;
        //rAWBInput.i4StatLineSize = m_rAWBStatParam.i4WindowNumX*5;

        rAWBInput.pAWBStatBuf = reinterpret_cast<MVOID *>(paaobuf->mVa);//mpAao
        rAWBInput.i4StatLineSize = 120*5;//120*8.5;
        ///rAWBInput.u4AWBAAOMode = m_i4AAOmode;
        rAWBInput.i4SensorMode = m_eSensorMode;
        ///rAWBInput.bAWBLock = m_bAWBLock;
        rAWBInput.i4StatLineSize = AAOStride;

        // Set FDarea to algorithm
        if (m_bFaceAWBAreaChage == MTRUE) {  // Face AWB
            m_pIAwbAlgo->setAWBFDArea(&m_eAWBFDArea);
            m_bFaceAWBAreaChage = MFALSE;
        }

        m_pIAwbAlgo->handleAWB(rAWBInput, m_rAWBOutput);

#if ((!USE_OPEN_SOURCE_AWB) && (!USE_OPEN_SOURCE_FLASH_AE) )
        //if((rAWBInput.bIsStrobeFired) && (!m_bIsTorch))
        if((m_bAAOMainFlashOn) && (m_eAWBMode != LIB3A_AWB_MODE_MWB))
        {
            if(isFlashAWBv2Enabled(m_eSensorDev) &&
                (m_pNVRAM_FLASH_CALIBRATION->flashWBGain[m_flashDuty].i4R !=0) &&
                (m_pNVRAM_FLASH_CALIBRATION->flashWBGain[m_flashDuty].i4G !=0) &&
                (m_pNVRAM_FLASH_CALIBRATION->flashWBGain[m_flashDuty].i4B !=0))
            {
                FLASH_AWB_CAL_GAIN_INPUT_T FlashAwbCalGainInput;
                FLASH_AWB_OUTPUT_T FlashAwbOutput;
                FLASH_AWB_INIT_T FlashAwbInitParam;
                FlashAwbInitParam.flash_awb_tuning_param = m_pNVRAM_3A->rFlashAWBNVRAM.rTuningParam;

                FlashAwbCalGainInput.NoFlashWBGain = m_rAWBOutput.rPreviewAWBGain;
                FlashAwbCalGainInput.PureFlashWBGain = m_pNVRAM_FLASH_CALIBRATION->flashWBGain[m_flashDuty];
                FlashAwbCalGainInput.PureFlashWeight = m_flashAwbWeight;
                FlashAwbCalGainInput.FlashPreferenceGain = m_pNVRAM_3A->rFlashAWBNVRAM.rTuningParam.FlashPreferenceGain;

                AWB_LOG("[%d][flashAWB 2.1] m_flashDuty=%d, m_flashAwbWeight=%d ", m_eSensorDev, m_flashDuty, m_flashAwbWeight);
                AWB_LOG("[%d][flashAWB 2.1] pureFlash Rgain=%d, Ggain=%d, Bgain=%d", m_eSensorDev, FlashAwbCalGainInput.PureFlashWBGain.i4R, FlashAwbCalGainInput.PureFlashWBGain.i4G, FlashAwbCalGainInput.PureFlashWBGain.i4B);
                AWB_LOG("[%d][flashAWB 2.1] preview Rgain=%d, Ggain=%d, Bgain=%d", m_eSensorDev, FlashAwbCalGainInput.NoFlashWBGain.i4R, FlashAwbCalGainInput.NoFlashWBGain.i4G, FlashAwbCalGainInput.NoFlashWBGain.i4B);
                AWB_LOG("[%d][flashAWB 2.1] preview FlashPreferenceGain= (%d,  %d, %d)\n", m_eSensorDev,
                                        m_pNVRAM_3A->rFlashAWBNVRAM.rTuningParam.FlashPreferenceGain.i4R, m_pNVRAM_3A->rFlashAWBNVRAM.rTuningParam.FlashPreferenceGain.i4G, m_pNVRAM_3A->rFlashAWBNVRAM.rTuningParam.FlashPreferenceGain.i4B);

                m_pIAwbAlgo->Flash_Awb_calResultGain(FlashAwbCalGainInput, FlashAwbOutput);

                AWB_LOG("[%d][flashAWB 2.1] In Awb_mgr.cpp - FlashAwbOutput Rgain=%d, Ggain=%d, Bgain=%d",  m_eSensorDev, FlashAwbOutput.rAWBGain.i4R,  FlashAwbOutput.rAWBGain.i4G,  FlashAwbOutput.rAWBGain.i4B);

                m_rAWBOutput.rCaptureAWBGain.i4R = FlashAwbOutput.rAWBGain.i4R;
                m_rAWBOutput.rCaptureAWBGain.i4G = FlashAwbOutput.rAWBGain.i4G;
                m_rAWBOutput.rCaptureAWBGain.i4B = FlashAwbOutput.rAWBGain.i4B;

                m_rAWBOutput.rAWBInfo.rCurrentAWBGain = m_rAWBOutput.rCaptureAWBGain;
                if (m_eAWBMode != LIB3A_AWB_MODE_MWB)
                m_bFlashAwb = MTRUE;
            }
        }
#endif
        // Debug
        //m_rAWBOutput.rCaptureAWBGain.i4R = 512;
        //m_rAWBOutput.rCaptureAWBGain.i4G = 1;
        //m_rAWBOutput.rCaptureAWBGain.i4B = 1;

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

    if (m_bEnableAWB && (m_eAWBMode != LIB3A_AWB_MODE_OFF) && (m_i4StrobeMode == AWB_STROBE_MODE_ON))
    {
#if ((!USE_OPEN_SOURCE_AWB) && (!USE_OPEN_SOURCE_FLASH_AE) )
    if(isFlashAWBv2Enabled(m_eSensorDev) &&
        (m_pNVRAM_FLASH_CALIBRATION->flashWBGain[m_flashDuty].i4R !=0) &&
        (m_pNVRAM_FLASH_CALIBRATION->flashWBGain[m_flashDuty].i4G !=0) &&
        (m_pNVRAM_FLASH_CALIBRATION->flashWBGain[m_flashDuty].i4B !=0))
    {
        FLASH_AWB_CAL_GAIN_INPUT_T FlashAwbCalGainInput;
        FLASH_AWB_OUTPUT_T FlashAwbOutput;
        FLASH_AWB_INIT_T FlashAwbInitParam;
        FlashAwbInitParam.flash_awb_tuning_param = m_pNVRAM_3A->rFlashAWBNVRAM.rTuningParam;

        FlashAwbCalGainInput.NoFlashWBGain = m_rAWBOutput.rPreviewAWBGain;
        FlashAwbCalGainInput.PureFlashWBGain = m_pNVRAM_FLASH_CALIBRATION->flashWBGain[m_flashDuty];
        FlashAwbCalGainInput.PureFlashWeight = m_flashAwbWeight;
        FlashAwbCalGainInput.FlashPreferenceGain = m_pNVRAM_3A->rFlashAWBNVRAM.rTuningParam.FlashPreferenceGain;

        AWB_LOG("[%d][flashAWB 2.1] m_flashDuty=%d, m_flashAwbWeight=%d ", m_eSensorDev, m_flashDuty, m_flashAwbWeight);
        AWB_LOG("[%d][flashAWB 2.1] pureFlash Rgain=%d, Ggain=%d, Bgain=%d", m_eSensorDev, FlashAwbCalGainInput.PureFlashWBGain.i4R, FlashAwbCalGainInput.PureFlashWBGain.i4G, FlashAwbCalGainInput.PureFlashWBGain.i4B);
        AWB_LOG("[%d][flashAWB 2.1] preview Rgain=%d, Ggain=%d, Bgain=%d", m_eSensorDev, FlashAwbCalGainInput.NoFlashWBGain.i4R, FlashAwbCalGainInput.NoFlashWBGain.i4G, FlashAwbCalGainInput.NoFlashWBGain.i4B);
        AWB_LOG("[%d][flashAWB 2.1] preview FlashPreferenceGain= (%d,  %d, %d)\n", m_eSensorDev,
                                m_pNVRAM_3A->rFlashAWBNVRAM.rTuningParam.FlashPreferenceGain.i4R, m_pNVRAM_3A->rFlashAWBNVRAM.rTuningParam.FlashPreferenceGain.i4G, m_pNVRAM_3A->rFlashAWBNVRAM.rTuningParam.FlashPreferenceGain.i4B);

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
MBOOL AwbMgr::applyAWB(AWB_SYNC_OUTPUT_N3D_T &rAWBSyncOutput)
{
    m_rAWBOutput.rAWBInfo.rCurrentAWBGain = rAWBSyncOutput.rAWBGain;
    m_rAWBOutput.rAWBInfo.i4CCT = rAWBSyncOutput.i4CCT;

    // update AWB gain
    //  IspTuningMgr::getInstance().setAWBGain(m_eSensorDev, m_rAWBOutput.rAWBInfo.rCurrentAWBGain);
    //setRgbGainX(m_eSensorDev, m_rAWBOutput.rAWBInfo.rCurrentAWBGain);

    // set AWB info: FIXME: debug info update?
    //IspTuningMgr::getInstance().setAWBInfo(m_eSensorDev, m_rAWBOutput.rAWBInfo);
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
    i4ScaleUnit = m_rAWBInitInput.rAWBParam.rChipParam.i4AWBGainOutputScaleUnit;

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MRESULT AwbMgr::getAWBParentStat(AWB_PARENT_BLK_STAT_T &a_rAWBParentState,  MINT32 *a_ParentBlkNumX, MINT32 *a_ParentBlkNumY)
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
    AWB_LOG("[%s()][%d]\n", __FUNCTION__, m_eSensorDev);

    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    SensorStaticInfo rSensorStaticInfo;

    if(pIHalSensorList != NULL)
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

    AWB_LOG("i4SensorWidth[ESensorMode_Preview] = %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Preview]);
    AWB_LOG("i4SensorHeight[ESensorMode_Preview] = %d\n", m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Preview]);
    AWB_LOG("i4SensorWidth[ESensorMode_Capture] = %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Capture]);
    AWB_LOG("i4SensorHeight[ESensorMode_Capture] = %d\n", m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Capture]);
    AWB_LOG("i4SensorWidth[ESensorMode_Video] = %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Video]);
    AWB_LOG("i4SensorHeight[ESensorMode_Video] = %d\n", m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Video]);
    AWB_LOG("i4SensorWidth[ESensorMode_SlimVideo1] = %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_SlimVideo1]);
    AWB_LOG("i4SensorHeight[ESensorMode_SlimVideo1] = %d\n", m_rAWBWindowConfig.i4SensorHeight[ESensorMode_SlimVideo1]);
    AWB_LOG("i4SensorWidth[ESensorMode_SlimVideo2] = %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_SlimVideo2]);
    AWB_LOG("i4SensorHeight[ESensorMode_SlimVideo2] = %d\n", m_rAWBWindowConfig.i4SensorHeight[ESensorMode_SlimVideo2]);
    AWB_LOG("i4SensorWidth[ESensorMode_Custom1] = %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom1]);
    AWB_LOG("i4SensorHeight[ESensorMode_Custom1] = %d\n", m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom1]);
    AWB_LOG("i4SensorWidth[ESensorMode_Custom2] = %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom2]);
    AWB_LOG("i4SensorHeight[ESensorMode_Custom2] = %d\n", m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom2]);
    AWB_LOG("i4SensorWidth[ESensorMode_Custom3] = %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom3]);
    AWB_LOG("i4SensorHeight[ESensorMode_Custom3] = %d\n", m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom3]);
    AWB_LOG("i4SensorWidth[ESensorMode_Custom4] = %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom4]);
    AWB_LOG("i4SensorHeight[ESensorMode_Custom4] = %d\n", m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom4]);
    AWB_LOG("i4SensorWidth[ESensorMode_Custom5] = %d\n", m_rAWBWindowConfig.i4SensorWidth[ESensorMode_Custom5]);
    AWB_LOG("i4SensorHeight[ESensorMode_Custom5] = %d\n", m_rAWBWindowConfig.i4SensorHeight[ESensorMode_Custom5]);


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
    if(err!=0)
    {
        CAM_LOGE("getNvramData getBufAndRead fail\n");
        return E_AWB_NVRAM_ERROR;
    }

#if 1
    AWB_LOG_IF(m_i4DgbLog,"sizeof(m_pNVRAM_3A->SensorId) = %d\n", (int)sizeof(m_pNVRAM_3A->SensorId));
    AWB_LOG_IF(m_i4DgbLog,"sizeof(m_pNVRAM_3A->rAENVRAM) = %d\n", (int)sizeof(m_pNVRAM_3A->rAENVRAM));
    AWB_LOG_IF(m_i4DgbLog,"sizeof(m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx]) x 2 = %d\n", (int)sizeof(m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx]));
    AWB_LOG_IF(m_i4DgbLog,"sizeof(m_pNVRAM_3A->rFlashAWBNVRAM) = %d\n", (int)sizeof(m_pNVRAM_3A->rFlashAWBNVRAM));
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
    case ESensorDev_MainThird:
        i4SensorDevID = SENSOR_DEV_MAIN_3;
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
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4R);
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4G);
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4B);
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4R);
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4G);
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4B);
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
        mGoldenGain3.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4R_low);
        mGoldenGain3.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4G_low);
        mGoldenGain3.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rGoldGainu4B_low);
        mUnitGain3.i4R = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4R_low);
        mUnitGain3.i4G = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4G_low);
        mUnitGain3.i4B = static_cast<MINT32>(GetCamCalData.Single2A.S2aAwb.rUnitGainu4B_low);
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

    AWB_LOG("m_pNVRAM_3A->rAWBNVRAM[%d].rCalData.rGoldenGain.i4R = %d\n", m_i4AWBNvramIdx, m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4R);
    AWB_LOG("m_pNVRAM_3A->rAWBNVRAM[%d].rCalData.rGoldenGain.i4G = %d\n", m_i4AWBNvramIdx, m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4G);
    AWB_LOG("m_pNVRAM_3A->rAWBNVRAM[%d].rCalData.rGoldenGain.i4B = %d\n", m_i4AWBNvramIdx, m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4B);
    AWB_LOG("m_pNVRAM_3A->rAWBNVRAM[%d].rCalData.rUnitGain.i4R = %d\n", m_i4AWBNvramIdx, m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4R);
    AWB_LOG("m_pNVRAM_3A->rAWBNVRAM[%d].rCalData.rUnitGain.i4G = %d\n", m_i4AWBNvramIdx, m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4G);
    AWB_LOG("m_pNVRAM_3A->rAWBNVRAM[%d].rCalData.rUnitGain.i4B = %d\n", m_i4AWBNvramIdx, m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4B);
/*
    AWB_LOG("m_pNVRAM_3A->rAWBNVRAM[%d].rAlgoCalParam.rCalData.rGoldenValue.i4R = %d\n", m_i4AWBNvramIdx, m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenValue.i4R);
    AWB_LOG("m_pNVRAM_3A->rAWBNVRAM[%d].rAlgoCalParam.rCalData.rGoldenValue.i4Gr = %d\n", m_i4AWBNvramIdx, m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenValue.i4Gr);
    AWB_LOG("m_pNVRAM_3A->rAWBNVRAM[%d].rAlgoCalParam.rCalData.rGoldenValue.i4Gb = %d\n", m_i4AWBNvramIdx, m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenValue.i4Gb);
    AWB_LOG("m_pNVRAM_3A->rAWBNVRAM[%d].rAlgoCalParam.rCalData.rGoldenValue.i4B = %d\n", m_i4AWBNvramIdx, m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenValue.i4B);
    AWB_LOG("m_pNVRAM_3A->rAWBNVRAM[%d].rAlgoCalParam.rCalData.rUnitValue.i4R = %d\n", m_i4AWBNvramIdx, m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitValue.i4R);
    AWB_LOG("m_pNVRAM_3A->rAWBNVRAM[%d].rAlgoCalParam.rCalData.rUnitValue.i4Gr = %d\n", m_i4AWBNvramIdx, m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitValue.i4Gr);
    AWB_LOG("m_pNVRAM_3A->rAWBNVRAM[%d].rAlgoCalParam.rCalData.rUnitValue.i4Gb = %d\n", m_i4AWBNvramIdx, m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitValue.i4Gb);
    AWB_LOG("m_pNVRAM_3A->rAWBNVRAM[%d].rAlgoCalParam.rCalData.rUnitValue.i4B = %d\n", m_i4AWBNvramIdx, m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitValue.i4B);
*/
    MY_LOG("mGoldenGain2.i4R[%d] mUnitGain2.i4R = %d\n", mGoldenGain2.i4R, mUnitGain2.i4R);
    MY_LOG("mGoldenGain2.i4G[%d] mUnitGain2.i4G = %d\n", mGoldenGain2.i4G, mUnitGain2.i4G);
    MY_LOG("mGoldenGain2.i4B[%d] mUnitGain2.i4B = %d\n", mGoldenGain2.i4B, mUnitGain2.i4B);

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
    for (MINT32 sensorMode = ESensorMode_Preview; sensorMode < ESensorMode_NUM; sensorMode++)
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

        AWB_LOG("[%s][%d] Sensor mode: %d i4SensorWidth: %d, i4SensorHeight: %d i4NumX: %d, i4NumY: %d i4SizeX: %d, i4SizeY: %d i4PitchX: %d, i4PitchY: %d i4OriginX: %d, i4OriginY: %d\n",
                 __FUNCTION__, m_eSensorDev, sensorMode,
                 m_rAWBWindowConfig.i4SensorWidth[sensorMode], m_rAWBWindowConfig.i4SensorHeight[sensorMode],
                 m_rAWBWindowConfig.i4NumX[sensorMode], m_rAWBWindowConfig.i4NumY[sensorMode],
                 m_rAWBWindowConfig.i4SizeX[sensorMode], m_rAWBWindowConfig.i4SizeY[sensorMode],
                 m_rAWBWindowConfig.i4PitchX[sensorMode], m_rAWBWindowConfig.i4PitchY[sensorMode],
                 m_rAWBWindowConfig.i4OriginX[sensorMode], m_rAWBWindowConfig.i4OriginY[sensorMode]);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::AWBStatConfig()
{
    MINT32 sensorMode, awbMode;

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

            if (m_i4AAOmode == 0)
            {
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGainLimitR = m_rAWBStatParam.i4PreGainLimitR;
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGainLimitG = m_rAWBStatParam.i4PreGainLimitG;
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGainLimitB = m_rAWBStatParam.i4PreGainLimitB;

            }
            else
            {
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGainLimitR = (m_rAWBStatParam.i4PreGainLimitR << 2) | 3;
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGainLimitG = (m_rAWBStatParam.i4PreGainLimitG << 2) | 3;
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGainLimitB = (m_rAWBStatParam.i4PreGainLimitB << 2) | 3;

            }

            //m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGainR = m_rAWBRAWPreGain1.i4R;
            //m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGainG = m_rAWBRAWPreGain1.i4G;
            //m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4PreGainB = m_rAWBRAWPreGain1.i4B;

            if (m_rAWBStatParam.i4ErrorThreshold != 0) // default setting
            {
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorThreshold = m_rAWBStatParam.i4ErrorThreshold;
            }
            else // error pixel count threshold = window size * error pixel count threshold ratio / 256, the error pixel count threshold ratio's base is 256
            {
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorThreshold = m_rAWBWindowConfig.i4SizeX[sensorMode] * m_rAWBWindowConfig.i4SizeY[sensorMode] * m_rAWBStatParam.i4ErrorThreshold / 256;
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
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorThreshold = m_rAWBWindowConfig.i4SizeX[sensorMode] * m_rAWBWindowConfig.i4SizeY[sensorMode] * m_i4QbinRatio * m_rAWBStatParam.i4ErrorThreshold / 256;
            }
            else // non-HDR
            {
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorThreshold = 0x3FFF;
            }

            AWB_LOG_IF(m_i4DgbLog, "[%s()] i4ErrorThreshold: %d => %d i4ErrorShiftBits: %d i4MoErrorThreshold: %d\n", __FUNCTION__,
                m_rAWBStatParam.i4ErrorThreshold, m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorThreshold,
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorShiftBits,
                m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4ErrorThreshold);

            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4Cos = m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rRotationMatrix.i4Cos;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][awbMode].i4Sin = m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rRotationMatrix.i4Sin;
        }

#if 0
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
#endif
        // 2. Update AWB light area
        AWB_LIGHT_AREA_T *pAWBArea = &m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rAWBLightArea;

        // Tungsten
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_TUNGSTEN] = pAWBArea->rTungsten.i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_TUNGSTEN] = pAWBArea->rTungsten.i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_TUNGSTEN] = pAWBArea->rTungsten.i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_TUNGSTEN] = pAWBArea->rTungsten.i4UpperBound;

        // Warm fluorescent
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_WARM_FLUORESCENT] = pAWBArea->rWarmFluorescent.i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_WARM_FLUORESCENT] = pAWBArea->rWarmFluorescent.i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_WARM_FLUORESCENT] = pAWBArea->rWarmFluorescent.i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_WARM_FLUORESCENT] = pAWBArea->rWarmFluorescent.i4UpperBound;

        // Fluorescent
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_FLUORESCENT] = pAWBArea->rFluorescent.i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_FLUORESCENT] = pAWBArea->rFluorescent.i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_FLUORESCENT] = pAWBArea->rFluorescent.i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_FLUORESCENT] = pAWBArea->rFluorescent.i4UpperBound;

        // CWF
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_CWF] = pAWBArea->rCWF.i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_CWF] = pAWBArea->rCWF.i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_CWF] = pAWBArea->rCWF.i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_CWF] = pAWBArea->rCWF.i4UpperBound;

        // Daylight
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_DAYLIGHT] = pAWBArea->rDaylight.i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_DAYLIGHT] = pAWBArea->rDaylight.i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_DAYLIGHT] = pAWBArea->rDaylight.i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_DAYLIGHT] = pAWBArea->rDaylight.i4UpperBound;

        // Shade
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_SHADE] = pAWBArea->rShade.i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_SHADE] = pAWBArea->rShade.i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_SHADE] = pAWBArea->rShade.i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_SHADE] = pAWBArea->rShade.i4UpperBound;

        // Daylight fluorescent
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_DAYLIGHT_FLUORESCENT] = pAWBArea->rDaylightFluorescent.i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_DAYLIGHT_FLUORESCENT] = pAWBArea->rDaylightFluorescent.i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_DAYLIGHT_FLUORESCENT] = pAWBArea->rDaylightFluorescent.i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_DAYLIGHT_FLUORESCENT] = pAWBArea->rDaylightFluorescent.i4UpperBound;

        // Strobe
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_STROBE] = pAWBArea->rStrobe.i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_STROBE] = pAWBArea->rStrobe.i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_STROBE] = pAWBArea->rStrobe.i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_STROBE] = pAWBArea->rStrobe.i4UpperBound;
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
#endif
        // 3. Update PWB light area
        PWB_LIGHT_AREA_T *pPWBArea = &m_rAWBInitInput.rAWBNVRAM.rAlgoCalParam.rPWBLightArea;

        // Reference area
        for (MINT32 i = LIB3A_AWB_MODE_DAYLIGHT; i < LIB3A_AWB_MODE_NUM; i++)
        {
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][i].i4AWBXY_WINR[PWB_REFERENCE_AREA_INDEX] = pPWBArea->rReferenceArea.i4RightBound;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][i].i4AWBXY_WINL[PWB_REFERENCE_AREA_INDEX] = pPWBArea->rReferenceArea.i4LeftBound;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][i].i4AWBXY_WIND[PWB_REFERENCE_AREA_INDEX] = pPWBArea->rReferenceArea.i4LowerBound;
            m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][i].i4AWBXY_WINU[PWB_REFERENCE_AREA_INDEX] = pPWBArea->rReferenceArea.i4UpperBound;
        }

        // Daylight neutral area
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_DAYLIGHT].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rDaylight.i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_DAYLIGHT].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rDaylight.i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_DAYLIGHT].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rDaylight.i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_DAYLIGHT].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rDaylight.i4UpperBound;

        // Cloudy daylight neutral area
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_CLOUDY_DAYLIGHT].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rCloudyDaylight.i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_CLOUDY_DAYLIGHT].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rCloudyDaylight.i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_CLOUDY_DAYLIGHT].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rCloudyDaylight.i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_CLOUDY_DAYLIGHT].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rCloudyDaylight.i4UpperBound;

        // Shade neutral area
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_SHADE].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rShade.i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_SHADE].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rShade.i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_SHADE].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rShade.i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_SHADE].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rShade.i4UpperBound;

        // Twilight neutral area
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_TWILIGHT].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rTwilight.i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_TWILIGHT].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rTwilight.i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_TWILIGHT].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rTwilight.i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_TWILIGHT].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rTwilight.i4UpperBound;

        // Twilight neutral area
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_FLUORESCENT].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rFluorescent.i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_FLUORESCENT].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rFluorescent.i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_FLUORESCENT].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rFluorescent.i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_FLUORESCENT].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rFluorescent.i4UpperBound;

        // Warm fluorescent neutral area
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_WARM_FLUORESCENT].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rWarmFluorescent.i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_WARM_FLUORESCENT].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rWarmFluorescent.i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_WARM_FLUORESCENT].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rWarmFluorescent.i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_WARM_FLUORESCENT].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rWarmFluorescent.i4UpperBound;

        // Incandescent neutral area
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_INCANDESCENT].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rIncandescent.i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_INCANDESCENT].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rIncandescent.i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_INCANDESCENT].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rIncandescent.i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_INCANDESCENT].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rIncandescent.i4UpperBound;

        // Gray world neutral area
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_GRAYWORLD].i4AWBXY_WINR[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rGrayWorld.i4RightBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_GRAYWORLD].i4AWBXY_WINL[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rGrayWorld.i4LeftBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_GRAYWORLD].i4AWBXY_WIND[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rGrayWorld.i4LowerBound;
        m_rAWBStatCfg[AWB_STROBE_MODE_ON][sensorMode][LIB3A_AWB_MODE_GRAYWORLD].i4AWBXY_WINU[PWB_NEUTRAL_AREA_INDEX] = pPWBArea->rGrayWorld.i4UpperBound;
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
        m_rAWBStatCfg[AWB_STROBE_MODE_OFF][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINR[AWB_LIGHT_STROBE] = 0;
        m_rAWBStatCfg[AWB_STROBE_MODE_OFF][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINL[AWB_LIGHT_STROBE] = 0;
        m_rAWBStatCfg[AWB_STROBE_MODE_OFF][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WIND[AWB_LIGHT_STROBE] = 0;
        m_rAWBStatCfg[AWB_STROBE_MODE_OFF][sensorMode][LIB3A_AWB_MODE_AUTO].i4AWBXY_WINU[AWB_LIGHT_STROBE] = 0;
    }

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
                m_rAWBStatCfg[strobeMode][sensorMode][awbMode].i4PreGainR = m_rAWBRAWPreGain1.i4R;
                m_rAWBStatCfg[strobeMode][sensorMode][awbMode].i4PreGainG = m_rAWBRAWPreGain1.i4G;
                m_rAWBStatCfg[strobeMode][sensorMode][awbMode].i4PreGainB = m_rAWBRAWPreGain1.i4B;
            }

            AWB_LOG("[AWBRAWPreGain1Config] i4PreGainR = %d\n", m_rAWBStatCfg[AWB_STROBE_MODE_ON][ESensorMode_Preview][LIB3A_AWB_MODE_AUTO].i4PreGainR);
            AWB_LOG("[AWBRAWPreGain1Config] i4PreGainG = %d\n", m_rAWBStatCfg[AWB_STROBE_MODE_ON][ESensorMode_Preview][LIB3A_AWB_MODE_AUTO].i4PreGainG);
            AWB_LOG("[AWBRAWPreGain1Config] i4PreGainB = %d\n", m_rAWBStatCfg[AWB_STROBE_MODE_ON][ESensorMode_Preview][LIB3A_AWB_MODE_AUTO].i4PreGainB);

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
	
	ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).SetTGInfo(i4TGInfo);

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
MRESULT AwbMgr::setCamScenarioMode(MUINT32 u4NewCamScenarioMode)
{
    CAM_SCENARIO_T eNewCamScenarioMode = static_cast<CAM_SCENARIO_T>(u4NewCamScenarioMode);

    if( m_eCamScenarioMode != eNewCamScenarioMode) {
        AWB_LOG( "[%s()] m_eSensorDev: %d, m_eCamScenarioMode: %d -> %d, m_i4AWBNvramIdx: %d -> %d\n", __FUNCTION__, m_eSensorDev, m_eCamScenarioMode, eNewCamScenarioMode, m_i4AWBNvramIdx, AWBNVRAMMapping[eNewCamScenarioMode]);

        m_eCamScenarioMode = eNewCamScenarioMode;
        m_i4AWBNvramIdx = AWBNVRAMMapping[m_eCamScenarioMode];
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
   /// m_rAWBInitInput.eAWBScenario = m_eCamScenarioMode;
    m_bStrobeModeChanged = MFALSE;
    m_bAWBModeChanged = MFALSE;
    m_bOneShotAWB = MTRUE; // do one-shot AWB
    m_bInitState = MTRUE; // init state

    switch (m_eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        m_rAWBInitInput.rAWBParam = getAWBParam<ESensorDev_Main>();
        m_rAWBInitInput.rAWBStatParam = getAWBStatParam<ESensorDev_Main>();
        m_rAWBStatParam = getAWBStatParam<ESensorDev_Main>();
        break;
    case ESensorDev_MainSecond: //  Main Second Sensor
        m_rAWBInitInput.rAWBParam = getAWBParam<ESensorDev_MainSecond>();
        m_rAWBInitInput.rAWBStatParam = getAWBStatParam<ESensorDev_MainSecond>();
        m_rAWBStatParam = getAWBStatParam<ESensorDev_MainSecond>();
        break;
    case ESensorDev_Sub: //  Sub Sensor
        m_rAWBInitInput.rAWBParam = getAWBParam<ESensorDev_Sub>();
        m_rAWBInitInput.rAWBStatParam = getAWBStatParam<ESensorDev_Sub>();
        m_rAWBStatParam = getAWBStatParam<ESensorDev_Sub>();
        break;
    case ESensorDev_SubSecond: //  Sub Second Sensor
        m_rAWBInitInput.rAWBParam = getAWBParam<ESensorDev_SubSecond>();
        m_rAWBInitInput.rAWBStatParam = getAWBStatParam<ESensorDev_SubSecond>();
        m_rAWBStatParam = getAWBStatParam<ESensorDev_SubSecond>();
        break;
    case ESensorDev_MainThird: //  Main Third Sensor
        m_rAWBInitInput.rAWBParam = getAWBParam<ESensorDev_MainThird>();
        m_rAWBInitInput.rAWBStatParam = getAWBStatParam<ESensorDev_MainThird>();
        m_rAWBStatParam = getAWBStatParam<ESensorDev_MainThird>();
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

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.awb.otp.disable", value, "0");
    MBOOL bDisable = atoi(value);

    property_get("vendor.awb.otp.force.read", value, "0");
    MBOOL bForceRead = atoi(value);

    if (m_bAWBCalibrationBypassed) {
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4R = 0;
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4G = 0;
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4B = 0;
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4R = 0;
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4G = 0;
        m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4B = 0;
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
        ((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4R == 0) ||
        (m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4G == 0) ||
        (m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rGoldenGain.i4B == 0) ||
        (m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4R == 0)   ||
        (m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4G == 0)   ||
        (m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rCalData.rUnitGain.i4B == 0))) ||
        (bForceRead) ||
        m_bAWBNvramIdxChanged)
    {
        getEEPROMData(); // TBC
    }

//    if (m_bColdBoot || bForceRead || m_bAWBNvramIdxChanged) {
        m_rAWBInitInput.rAWBNVRAM = m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx];

        AWB_LOG("m_rAWBInitInput.eAWBMode = %d\n", m_rAWBInitInput.eAWBMode);
        AWB_LOG("m_eSensorDev = %d\n", m_eSensorDev);

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
         ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).config(m_rAWBStatCfg[m_i4StrobeMode][m_eSensorMode][m_eAWBMode], m_bHBIN2Enable, mAaoW, mAaoH, m_rAWBStatParam.i4WindowNumX, m_rAWBStatParam.i4WindowNumY, m_i4SensorIdx);


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
            if (FAILED(m_pIAwbAlgo->initAWB(m_rAWBInitInput, m_rAWBOutput, m_rAWBRAWPreGain1))) {
                CAM_LOGE("m_pIAwbAlgo->initAWB fail\n");
                return MFALSE;
            }

            m_bAlgoInit = MTRUE;
        }
        else
        {
            m_pIAwbAlgo->updateAWBParam(m_rAWBInitInput);
        }

        ///m_rAWBOutput.rAWBInfo.rPregain1 = m_rAWBRAWPreGain1;

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

    // Face detection
    m_pIAwbAlgo->setAWBFDArea(&m_eAWBFDArea);

    // reset AWB state
    if (m_eAWBMode == LIB3A_AWB_MODE_AUTO)
        m_pAwbStateMgr->reset(eState_InactiveAuto);
    else
        m_pAwbStateMgr->reset(eState_InactiveNotAuto);

    //AAO stride assign
    AAOStride = (4+1+AE_TSF_ENABLE*2)*m_rAWBStatParam.i4WindowNumX;
    MY_LOG("[%s] Stride Size= %d", __FUNCTION__, AAOStride);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::getColorCorrectionGain(MFLOAT& fGain_R, MFLOAT& fGain_G, MFLOAT& fGain_B)
{
    //AWB_LOG("%s()\n", __FUNCTION__);

    fGain_R = static_cast<MFLOAT>(m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4R) / m_rAWBInitInput.rAWBParam.rChipParam.i4AWBGainOutputScaleUnit;
    fGain_G = static_cast<MFLOAT>(m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4G) / m_rAWBInitInput.rAWBParam.rChipParam.i4AWBGainOutputScaleUnit;
    fGain_B = static_cast<MFLOAT>(m_rAWBOutput.rAWBInfo.rCurrentAWBGain.i4B) / m_rAWBInitInput.rAWBParam.rChipParam.i4AWBGainOutputScaleUnit;

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

    if (m_rColorCorrectionGain.i4R > m_rAWBInitInput.rAWBParam.rChipParam.i4AWBGainOutputUpperLimit)
        m_rColorCorrectionGain.i4R = m_rAWBInitInput.rAWBParam.rChipParam.i4AWBGainOutputUpperLimit;

    if (m_rColorCorrectionGain.i4G > m_rAWBInitInput.rAWBParam.rChipParam.i4AWBGainOutputUpperLimit)
        m_rColorCorrectionGain.i4G = m_rAWBInitInput.rAWBParam.rChipParam.i4AWBGainOutputUpperLimit;

    if (m_rColorCorrectionGain.i4B > m_rAWBInitInput.rAWBParam.rChipParam.i4AWBGainOutputUpperLimit)
        m_rColorCorrectionGain.i4B = m_rAWBInitInput.rAWBParam.rChipParam.i4AWBGainOutputUpperLimit;

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::getAWBState(mtk_camera_metadata_enum_android_control_awb_state_t& eAWBState)
{
    AWB_LOG_IF(m_i4DgbLog,"%s()\n", __FUNCTION__);

    eAWBState = m_pAwbStateMgr->getCurrentState();

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

MBOOL AwbMgr::SetAETargetMode(int mode) //for hdr
{
    (void)mode;

// replaced by setCamScenarioMode
/*
    if(mode==AE_MODE_IVHDR_TARGET || mode==AE_MODE_MVHDR_TARGET || mode==AE_MODE_ZVHDR_TARGET)
        setAWBNvramIdx(AWBNVRAMMapping[CAM_SCENARIO_CUSTOM1]);
    else
        setAWBNvramIdx(AWBNVRAMMapping[CAM_SCENARIO_PREVIEW]);
*/
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AwbMgr::setAAOMode(MUINT32 u4NewAWBAAOMode)
{
    AWB_LOG("%s(): m_i4AAOmode: %d -> %d (0:8/12bits, 1:14bits)\n", __FUNCTION__, m_i4AAOmode, u4NewAWBAAOMode);
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

    ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev,m_eSensorTG).SetAAOMode(m_i4AAOmode);

    return MTRUE;
}

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

    ///if(CUST_ENABLE_FACE_AWB() == MTRUE) {
    if (1){ //tmp
        if(pFaces == NULL) {
            AWB_LOG( "[%s()] FD window is NULL pointer 1\n", __FUNCTION__);
            return S_AWB_OK;
        }

        //Fix IP Base Coverity
        //if((pFaces->faces) == NULL) {
        //    AWB_LOG( "[%s()] FD window is NULL pointer 2\n", __FUNCTION__);
        //    return S_AWB_OK;
        //}

        //First time with face to without face
        if (m_eAWBFDArea.u4Count && (pFaces->number_of_faces == 0)){
            m_bFaceAWBAreaChage = MTRUE;
            //AWB_LOG( "[%s()] First frame without face Count %d ->%d \n", __FUNCTION__, m_eAWBFDArea.u4Count, pFaces->number_of_faces);
        }

        // Clear m_eAWBFDArea content
        memset(&m_eAWBFDArea, 0, sizeof(AWBMeteringArea_T));

        if(m_bFDenable == MTRUE){

            // Define FD area number
            if(pFaces->number_of_faces > MAX_AWB_METER_AREAS) {
                m_eAWBFDArea.u4Count = MAX_AWB_METER_AREAS;
            } else {
                m_eAWBFDArea.u4Count = pFaces->number_of_faces;
            }

            for(ifaceIdx=0; ifaceIdx < m_eAWBFDArea.u4Count; ifaceIdx++) {
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

                // landmark cv
                m_eAWBFDArea.rAreas[ifaceIdx].i4LandmarkCV = pFaces->fa_cv[ifaceIdx];
                if (pFaces->fa_cv[ifaceIdx] > 0) {

                    // left eye
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[0][0] = pFaces->leyex0[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[0][1] = pFaces->leyey0[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[0][2] = pFaces->leyex1[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[0][3] = pFaces->leyey1[ifaceIdx];

                    // right eye
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[1][0] = pFaces->reyex0[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[1][1] = pFaces->reyey0[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[1][2] = pFaces->reyex1[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[1][3] = pFaces->reyey1[ifaceIdx];

                    // mouth
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[2][0] = pFaces->mouthx0[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[2][1] = pFaces->mouthy0[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[2][2] = pFaces->mouthx1[ifaceIdx];
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[2][3] = pFaces->mouthy1[ifaceIdx];
                }

                // FD area info
                m_eAWBFDArea.rAreas[ifaceIdx].i4Left   = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Left*2000/i4tgwidth;
                m_eAWBFDArea.rAreas[ifaceIdx].i4Right  = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Right*2000/i4tgwidth;
                m_eAWBFDArea.rAreas[ifaceIdx].i4Top    = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Top*2000/i4tgheight;
                m_eAWBFDArea.rAreas[ifaceIdx].i4Bottom = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Bottom*2000/i4tgheight;

                if(pFaces->fa_cv[ifaceIdx] > 0)
                {
                    // left eye
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[0][0] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[0][0]*2000/i4tgwidth;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[0][1] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[0][1]*2000/i4tgwidth;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[0][2] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[0][2]*2000/i4tgheight;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[0][3] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[0][3]*2000/i4tgheight;

                    // right eye
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[1][0] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[1][0]*2000/i4tgwidth;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[1][1] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[1][1]*2000/i4tgwidth;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[1][2] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[1][2]*2000/i4tgheight;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[1][3] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[1][3]*2000/i4tgheight;

                    // mouth
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[2][0] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[2][0]*2000/i4tgwidth;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[2][1] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[2][1]*2000/i4tgwidth;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[2][2] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[2][2]*2000/i4tgheight;
                    m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[2][3] = -1000 + m_eAWBFDArea.rAreas[ifaceIdx].i4Landmark[2][3]*2000/i4tgheight;
                }

                AWB_LOG( "[%s():Modified] AWB FD meter area Idx:%d Left:%d Right:%d Top:%d Bottom:%d InitWidth:%d InitHeight:%d\n", __FUNCTION__, ifaceIdx,
                        m_eAWBFDArea.rAreas[ifaceIdx].i4Left, m_eAWBFDArea.rAreas[ifaceIdx].i4Right,
                        m_eAWBFDArea.rAreas[ifaceIdx].i4Top, m_eAWBFDArea.rAreas[ifaceIdx].i4Bottom,
                        m_eZoomWinInfo.u4XWidth,m_eZoomWinInfo.u4YHeight );
                AWB_LOG( "[%s():Previous] AWB FD meter area Idx:%d Left:%d Right:%d Top:%d Bottom:%d \n", __FUNCTION__, ifaceIdx,
                        pFaces->faces[ifaceIdx].rect[0], pFaces->faces[ifaceIdx].rect[1],
                        pFaces->faces[ifaceIdx].rect[2] , pFaces->faces[ifaceIdx].rect[3] );

                m_bFaceAWBAreaChage = MTRUE;
            }

            AWB_LOG_IF(m_i4DgbLog, "[%s()] i4SensorDev:%d line:%d Face Number:%d FaceAWBChange:%d", __FUNCTION__, m_eSensorDev, __LINE__, m_eAWBFDArea.u4Count, m_bFaceAWBAreaChage);
        }
    }
    else {
        ///AWB_LOG( "[%s()] i4SensorDev:%d line:%d The set face AWB is disable:%d ", __FUNCTION__, m_eSensorDev, __LINE__, CUST_ENABLE_FACE_AWB());
        AWB_LOG( "[%s()] i4SensorDev:%d line:%d ", __FUNCTION__, m_eSensorDev, __LINE__);
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

MRESULT AwbMgr::getFocusArea(android::Vector<MINT32> &vecOut, MINT32 i4tgwidth, MINT32 i4tgheight)
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
    AWB_LOG_IF(m_i4DgbLog ,"[%s] colorTemperature mMWBColorTemperature mIsMwbCctChanged%d/%d/%d \n", __FUNCTION__, colorTemperature, mMWBColorTemperature, mIsMwbCctChanged);
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
      gain = m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rD65;
      casexx = 0;
    }
    else if ( (cct < 6500) && (cct >= 5100))
    {
      casexx = 1;
      ratio_1 = (((float)cct- 5100)/(6500-5100))*100;
      ratio_2 = ((6500- (float)cct)/(6500-5100))*100;
      gain.i4R = (((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rD65.i4R * ratio_1)/100)+ ((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rDNP.i4R * ratio_2)/100));
      gain.i4G = (((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rD65.i4G * ratio_1)/100)+ ((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rDNP.i4G * ratio_2)/100));
      gain.i4B = (((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rD65.i4B * ratio_1)/100)+ ((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rDNP.i4B * ratio_2)/100));
    }
    else if ( (cct < 5100) &&  (cct >= 3750))
    {
      casexx  = 2;
      ratio_1 = (((float)cct- 3750)/(5100-3750))*100;
      ratio_2 = ((5100- (float)cct)/(5100-3750))*100;
      gain.i4R = (((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rDNP.i4R * ratio_1)/100)+ ((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rTL84.i4R * ratio_2)/100));
      gain.i4G = (((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rDNP.i4G * ratio_1)/100)+ ((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rTL84.i4G * ratio_2)/100));
      gain.i4B = (((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rDNP.i4B * ratio_1)/100)+ ((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rTL84.i4B * ratio_2)/100));
    }
    else if ( (cct < 3750) &&  (cct >= 2800))
    {
      casexx =3;
      ratio_1 = (((float)cct- 2800)/(3750-2800))*100;
      ratio_2 = ((3750- (float)cct)/(3750-2800))*100;
      gain.i4R = (((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rTL84.i4R * ratio_1)/100)+ ((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rA.i4R * ratio_2)/100));
      gain.i4G = (((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rTL84.i4G * ratio_1)/100)+ ((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rA.i4G * ratio_2)/100));
      gain.i4B = (((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rTL84.i4B * ratio_1)/100)+ ((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rA.i4B * ratio_2)/100));
    }

    else if ( (cct < 2800) &&  (cct >= 2300))
    {
     casexx = 4;
      ratio_1 = (((float)cct- 2300)/(2800-2300))*100;
      ratio_2 = ((2800- (float)cct)/(2800-2300))*100;
      gain.i4R = (((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rA.i4R * ratio_1)/100)+((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rHorizon.i4R * ratio_2)/100));
      gain.i4G = (((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rA.i4G * ratio_1)/100)+((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rHorizon.i4G * ratio_2)/100));
      gain.i4B = (((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rA.i4B * ratio_1)/100)+((m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rHorizon.i4B * ratio_2)/100));
    }
    else // 2300 below
    {
      gain = m_pNVRAM_3A->rAWBNVRAM[m_i4AWBNvramIdx].rAlgoCalParam.rLightAWBGain.rHorizon;
      casexx = 5;
    }


    AWB_LOG_IF(m_i4DgbLog,"[%s] r1:%d r2:%d cct:%d  case: %d gain: %d/%d/%d",__FUNCTION__, ratio_1, ratio_2, cct, casexx, gain.i4R,
                   gain.i4G, gain.i4B);


    return MTRUE;
}
