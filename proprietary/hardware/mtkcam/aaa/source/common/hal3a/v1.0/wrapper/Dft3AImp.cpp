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
#define LOG_TAG "Dft3A"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <Dft3A.h>
#include <mtkcam/utils/std/Log.h>
#include <aaa_error_code.h>
#include <cutils/properties.h>
#include <private/IopipeUtils.h>
#include <aaa_hal_sttCtrl.h>
#include <mtkcam/drv/IHalSensor.h>
#include <assert.h>
#include <mtkcam/utils/std/ULog.h>

#include <isp_tuning_mgr.h>
#include <isp_mgr.h>
#include "isp_mgr_af_stat.h"

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_3A_HAL(String) \
          do { \
              aee_system_exception( \
                  "Hal3A", \
                  NULL, \
                  DB_OPT_DEFAULT, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_3A_HAL(String)
#endif

#define MY_ASSERT(cond, ...) \
    if(!(cond)) { \
        CAM_ULOG_ASSERT(Utils::ULog::MOD_3A_FRAMEWORK_IP_BASE, cond, "[%s] Assertion Failed ! ( %s )", __FUNCTION__, #cond); \
    }

using namespace android;
using namespace NS3Av3;
using namespace NSIspTuningv3;

class Dft3AImp : public Dft3A
{
public:
    static I3AWrapper*  getInstance(MINT32 const i4SensorOpenIndex);
    virtual MVOID       destroyInstance();
    virtual MINT32      config(const ConfigInfo_T& rConfigInfo);
    virtual MBOOL       start();
    virtual MBOOL       stop();
    virtual MVOID       pause(){}
    virtual MVOID       resume(MINT32){}
    virtual MVOID       setSensorMode(MINT32 i4SensorMode);
    virtual MBOOL       generateP2(MINT32 flowType, const NSIspTuning::ISP_INFO_T& rIspInfo, void* pRegBuf, ResultP2_T* pResultP2);
    virtual MBOOL       validateP1(const ParamIspProfile_T& rParamIspProfile, MBOOL fgPerframe);
    virtual MBOOL       setParams(Param_T const &rNewParam, MBOOL bUpdateScenario);
    virtual MBOOL       setAfParams(AF_Param_T const &rNewParam);
    virtual MBOOL       autoFocus();
    virtual MBOOL       cancelAutoFocus();
    virtual MVOID       setFDEnable(MBOOL fgEnable);
    virtual MBOOL       setFDInfo(MVOID* prFaces, MVOID* prAFFaces);
    virtual MBOOL       setOTInfo(MVOID* prOT, MVOID* prAFOT);
    virtual MBOOL       setFlashLightOnOff(MBOOL bOnOff/*1=on; 0=off*/, MBOOL bMainPre/*1=main; 0=pre*/, MINT32 /*i4P1DoneSttNum*/);
    virtual MBOOL       setPreFlashOnOff(MBOOL bOnOff/*1=on; 0=off*/);
    virtual MBOOL       isNeedTurnOnPreFlash() const;
    virtual MBOOL       chkMainFlashOnCond() const;
    virtual MBOOL       chkPreFlashOnCond() const;
    virtual MBOOL       isStrobeBVTrigger() const;
    virtual MBOOL       chkCapFlash() const {return MFALSE;}
    virtual MINT32      getCurrResult(MUINT32 i4FrmId, MINT32 i4SubsampleCount = 0) const;
    virtual MINT32      getCurrentHwId() const;
    virtual MBOOL       postCommand(ECmd_T const r3ACmd, const ParamIspProfile_T* pParam = 0);
    virtual MINT32      send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR iArg1, MINTPTR iArg2);
    virtual MINT32      queryMagicNumber() const {return 0;}
    virtual VOID        queryTgSize(MINT32 &i4TgWidth, MINT32 &i4TgHeight);
    virtual MINT32      attachCb(I3ACallBack* cb);
    virtual MINT32      detachCb(I3ACallBack* cb);
    virtual VOID        queryHbinSize(MINT32 &i4HbinWidth, MINT32 &i4HbinHeight);
    virtual MBOOL       notifyPwrOn();
    virtual MBOOL       notifyPwrOff();
    virtual MBOOL       notifyP1PwrOn();
    virtual MBOOL       notifyP1PwrOff();
    virtual MVOID       setIspSensorInfo2AF(MINT32 MagicNum);
    virtual MBOOL       dumpP1Params(MINT32 /*i4MagicNum*/){return MTRUE;}
    virtual MBOOL       setP2Params(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2);
    virtual MBOOL       getP2Result(P2Param_T const &rNewP2Param, ResultP2_T* pResultP2);
    virtual MBOOL       setISPInfo(P2Param_T const &rNewP2Param, NSIspTuning::ISP_INFO_T &rIspInfo, MINT32 type);
    virtual MBOOL       preset(Param_T const &rNewParam);
    virtual MBOOL       notifyResult4TG(MINT32 i4PreFrmId);
    virtual MBOOL       notify4CCU(MUINT32 u4PreFrmId, ISP_NVRAM_OBC_T const &rOBCResult);
    virtual MVOID       notifyPreStop();

protected:  //    Ctor/Dtor.
                        Dft3AImp(MINT32 const i4SensorOpenIndex);
    virtual             ~Dft3AImp(){}

                        Dft3AImp(const Dft3AImp&);
                        Dft3AImp& operator=(const Dft3AImp&);

    //virtual MUINT32     Scenario4AEAWB(Param_T const &rParam);
    //virtual MUINT32     Scenario4AF(Param_T const &rParam);

    MBOOL               init();
    MBOOL               uninit();

    NSCam::NSIoPipe::NSCamIOPipe::INormalPipe* m_pCamIO;
    IspTuningMgr*       m_pTuning;
    MINT32              m_i4SensorIdx;
    MUINT32             m_u4SensorDev;
    MUINT32             m_u4SensorMode;
    MUINT32             m_u4Count;
};


I3AWrapper*
Dft3A::
getInstance(MINT32 const i4SensorOpenIndex)
{
    return Dft3AImp::getInstance(i4SensorOpenIndex);
}

I3AWrapper*
Dft3AImp::
getInstance(MINT32 const i4SensorOpenIndex)
{
    static Dft3AImp singleton(i4SensorOpenIndex);
    singleton.init();
    return &singleton;
}

MVOID
Dft3AImp::
destroyInstance()
{
    uninit();
}

Dft3AImp::
Dft3AImp(MINT32 const i4SensorOpenIndex)
    : m_pCamIO(NULL)
    , m_pTuning(NULL)
    , m_i4SensorIdx(i4SensorOpenIndex)
    , m_u4SensorDev(0)
    , m_u4SensorMode(0)
    , m_u4Count(0)
{
    CAM_LOGD("[%s] sensoridx(%d)", __FUNCTION__, i4SensorOpenIndex);

    IHalSensorList*const pHalSensorList = MAKE_HalSensorList();
    MY_ASSERT(pHalSensorList != nullptr);
    m_u4SensorDev = pHalSensorList->querySensorDevIdx(m_i4SensorIdx);
}

MBOOL
Dft3AImp::
init()
{
    if (m_pTuning == NULL)
    {
        m_pTuning = &IspTuningMgr::getInstance();
        if (!m_pTuning->init(m_u4SensorDev, m_i4SensorIdx))
        {
            CAM_LOGE("Fail to init IspTuningMgr (%d,%d)", m_u4SensorDev, m_i4SensorIdx);
            AEE_ASSERT_3A_HAL("Fail to init IspTuningMgr");
            return MFALSE;
        }
    }
    m_u4Count = 0;
    return MTRUE;
}

MBOOL
Dft3AImp::
uninit()
{
    if (m_pTuning)
    {
        CAM_LOGD("[%s] (%d)", __FUNCTION__, m_u4SensorDev);
        m_pTuning->uninit(m_u4SensorDev);
        m_pTuning = NULL;
        return MTRUE;
    }
    return MFALSE;
}

MINT32
Dft3AImp::
config(const ConfigInfo_T& /*rConfigInfo*/)
{
    if (m_pCamIO == NULL)
    {
        CAM_LOGD("[%s] +", __FUNCTION__);

        m_pCamIO = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe(m_i4SensorIdx, LOG_TAG);
        if (m_pCamIO == NULL)
        {
            CAM_LOGE("Fail to create NormalPipe");
            return E_3A_ERR;
        }

        AE_STAT_PARAM_T rTmpAe;
        AWB_STAT_CONFIG_T rTmpAwb;

        MINT32 m_i4BINSzW=0;
        MINT32 m_i4BINSzH=0;
        MINT32 m_i4QBINSzW=0;
        MINT32 m_i4QBINSzH=0;
        MINT32 m_i4TGSzW=0;
        MINT32 m_i4TGSzH=0;
        m_pCamIO->sendCommand( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_BIN_INFO, (MINTPTR)&m_i4BINSzW, (MINTPTR)&m_i4BINSzH, 0);
        m_pCamIO->sendCommand( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_HBIN_INFO, (MINTPTR)&m_i4QBINSzW, (MINTPTR)&m_i4QBINSzH, 0);
        m_pCamIO->sendCommand( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_TG_OUT_SIZE, (MINTPTR)&m_i4TGSzW, (MINTPTR)&m_i4TGSzH, 0);

        CAM_LOGD("[%s] AFO : BIN(%dx, %d), QBIN(%d, %d), TG(%d, %d)", __FUNCTION__, m_i4BINSzW, m_i4BINSzH, m_i4QBINSzW, m_i4QBINSzH, m_i4TGSzW, m_i4TGSzH);

        // HW setting.
        MINT32 i4HWBlkNumX;
        MINT32 i4HWBlkNumY;
        MINT32 i4HWBlkSizeX;
        MINT32 i4HWBlkSizeY;
        MINT32 i4FirstTimeConfig;
        AREA_T sArea_HW;
        AF_CONFIG_T testcfg;
        ::memset(&testcfg, 0, sizeof(AF_CONFIG_T));
#if 0
                /*For AF HAL control flow*/
                { m_i4TGSzW, m_i4TGSzH},
                { m_i4BINSzW, m_i4BINSzH},

                /*AF ROI*/
                {
                    /* X, Y, W, H, Info */
                    m_i4TGSzW*10/100,
                    m_i4TGSzH*10/100,
                    m_i4TGSzW*80/100,
                    m_i4TGSzW*80/100,
                    0
                },
                /*AF_BLK_1*/
                32,
                32,
                /*SGG PGN*/
                16,
                /*SGG GMRC 1*/
                20,
                29,
                43,
                62,
                /*SGG GMRC 2*/
                88,
                126,
                180,
                /*AF_CON*/
                0,
                0,
                3,
                { 1, 3, 7, 1}, /*[0]AF_BLF_EN ; [1]AF_BLF_D_LVL ; [2]AF_BLF_R_LVL ; [3] AF_BLF_VFIR_MUX*/
                /*AF_TH_0*/
                { 1, 1}, /*[0]AF_H_TH_0 ; [1]AF_H_TH_1 */
                /*AF_TH_1*/
                1,
                240,
                /*AF_FLT_1 ~ AF_FLT_3*/
                { 15, 36, 43, 36, 22, 10, 3, 0, 0, 0, 0, 0},
                /*AF_FLT_4 ~ AF_FLT_6*/
                { 15, 36, 43, 36, 22, 10, 3, 0, 0, 0, 0, 0},
                /*AF_FLT_7 ~ AF_FLT_8*/
                { 256, 256, 1792, 1792} /*[0]AF_VFLT_X0 ; [1]AF_VFLT_X1 ; [2]AF_VFLT_X2 ; [3] AF_VFLT_X3*/
            };
#endif
        // AE
        int blockwnum=120;
        int blockhnum=90;
        int pixelwnum=m_i4QBINSzW/4;
        int pixelhnum=m_i4QBINSzH/2;
        rTmpAe.rAEHistWinCFG[0].bAEHistEn = 1;
        rTmpAe.rAEHistWinCFG[1].bAEHistEn = 1;
        rTmpAe.rAEHistWinCFG[2].bAEHistEn = 1;
        rTmpAe.rAEHistWinCFG[3].bAEHistEn = 1;
        rTmpAe.rAEHistWinCFG[0].uAEHistBin= 0;
        rTmpAe.rAEHistWinCFG[1].uAEHistBin= 0;
        rTmpAe.rAEHistWinCFG[2].uAEHistBin= 0;
        rTmpAe.rAEHistWinCFG[3].uAEHistBin= 0;
        rTmpAe.rAEHistWinCFG[0].uAEHistOpt= 3;
        rTmpAe.rAEHistWinCFG[1].uAEHistOpt= 3;
        rTmpAe.rAEHistWinCFG[2].uAEHistOpt= 4;
        rTmpAe.rAEHistWinCFG[3].uAEHistOpt= 4;

        rTmpAe.rAEHistWinCFG[0].uAEHistXLow=0;
        rTmpAe.rAEHistWinCFG[0].uAEHistXHi=blockwnum-1;
        rTmpAe.rAEHistWinCFG[0].uAEHistYLow=0;
        rTmpAe.rAEHistWinCFG[0].uAEHistYHi=blockhnum-1;
        rTmpAe.rAEHistWinCFG[1].uAEHistXLow=0;
        rTmpAe.rAEHistWinCFG[1].uAEHistXHi=blockwnum-1;
        rTmpAe.rAEHistWinCFG[1].uAEHistYLow=0;
        rTmpAe.rAEHistWinCFG[1].uAEHistYHi=blockhnum-1;
        rTmpAe.rAEHistWinCFG[2].uAEHistXLow=0;
        rTmpAe.rAEHistWinCFG[2].uAEHistXHi=blockwnum-1;
        rTmpAe.rAEHistWinCFG[2].uAEHistYLow=0;
        rTmpAe.rAEHistWinCFG[2].uAEHistYHi=blockhnum-1;
        rTmpAe.rAEHistWinCFG[3].uAEHistXLow=0;
        rTmpAe.rAEHistWinCFG[3].uAEHistXHi=blockwnum-1;
        rTmpAe.rAEHistWinCFG[3].uAEHistYLow=0;
        rTmpAe.rAEHistWinCFG[3].uAEHistYHi=blockhnum-1;

        rTmpAe.rAEPixelHistWinCFG[0].bAEHistEn = 1;
        rTmpAe.rAEPixelHistWinCFG[1].bAEHistEn = 1;
        rTmpAe.rAEPixelHistWinCFG[2].bAEHistEn = 1;
        rTmpAe.rAEPixelHistWinCFG[3].bAEHistEn = 1;

        rTmpAe.rAEPixelHistWinCFG[0].bAEHistEn = 1;
        rTmpAe.rAEPixelHistWinCFG[1].bAEHistEn = 1;
        rTmpAe.rAEPixelHistWinCFG[2].bAEHistEn = 1;
        rTmpAe.rAEPixelHistWinCFG[3].bAEHistEn = 1;
        rTmpAe.rAEPixelHistWinCFG[0].uAEHistBin= 0;
        rTmpAe.rAEPixelHistWinCFG[1].uAEHistBin= 0;
        rTmpAe.rAEPixelHistWinCFG[2].uAEHistBin= 0;
        rTmpAe.rAEPixelHistWinCFG[3].uAEHistBin= 0;
        rTmpAe.rAEPixelHistWinCFG[0].uAEHistOpt= 3;
        rTmpAe.rAEPixelHistWinCFG[1].uAEHistOpt= 3;
        rTmpAe.rAEPixelHistWinCFG[2].uAEHistOpt= 4;
        rTmpAe.rAEPixelHistWinCFG[3].uAEHistOpt= 4;

        rTmpAe.rAEPixelHistWinCFG[0].uAEHistXLow=0;
        rTmpAe.rAEPixelHistWinCFG[0].uAEHistXHi=pixelwnum-1;
        rTmpAe.rAEPixelHistWinCFG[0].uAEHistYLow=0;
        rTmpAe.rAEPixelHistWinCFG[0].uAEHistYHi=pixelhnum-1;
        rTmpAe.rAEPixelHistWinCFG[1].uAEHistXLow=0;
        rTmpAe.rAEPixelHistWinCFG[1].uAEHistXHi=pixelwnum-1;
        rTmpAe.rAEPixelHistWinCFG[1].uAEHistYLow=0;
        rTmpAe.rAEPixelHistWinCFG[1].uAEHistYHi=pixelhnum-1;
        rTmpAe.rAEPixelHistWinCFG[2].uAEHistXLow=0;
        rTmpAe.rAEPixelHistWinCFG[2].uAEHistXHi=pixelwnum-1;
        rTmpAe.rAEPixelHistWinCFG[2].uAEHistYLow=0;
        rTmpAe.rAEPixelHistWinCFG[2].uAEHistYHi=pixelhnum-1;
        rTmpAe.rAEPixelHistWinCFG[3].uAEHistXLow=0;
        rTmpAe.rAEPixelHistWinCFG[3].uAEHistXHi=pixelwnum-1;
        rTmpAe.rAEPixelHistWinCFG[3].uAEHistYLow=0;
        rTmpAe.rAEPixelHistWinCFG[3].uAEHistYHi=pixelhnum-1;

        //AWB
        rTmpAwb.i4WindowNumX = blockwnum;
        rTmpAwb.i4WindowNumY = blockhnum;
        rTmpAwb.i4WindowOriginX = m_i4QBINSzW;
        rTmpAwb.i4WindowOriginY = m_i4QBINSzH;

        rTmpAwb.i4WindowPitchX=(m_i4QBINSzW/blockwnum)/2*2;
        rTmpAwb.i4WindowPitchY=(m_i4QBINSzH/blockhnum)/2*2;
        rTmpAwb.i4WindowSizeX = (rTmpAwb.i4WindowPitchX / 2) * 2;
        rTmpAwb.i4WindowSizeY = (rTmpAwb.i4WindowPitchY / 2) * 2;

        rTmpAwb.i4PreGainR = 512;
        rTmpAwb.i4PreGainG = 512;
        rTmpAwb.i4PreGainB = 512;

        rTmpAwb.i4PreGainLimitR = 0xFFF;
        rTmpAwb.i4PreGainLimitG = 0xFFF;
        rTmpAwb.i4PreGainLimitB = 0xFFF;

        rTmpAwb.i4LowThresholdR = 1;
        rTmpAwb.i4LowThresholdG = 1;
        rTmpAwb.i4LowThresholdB = 1;

        rTmpAwb.i4HighThresholdR = 254;
        rTmpAwb.i4HighThresholdG = 254;
        rTmpAwb.i4HighThresholdB = 254;

        int i4WindowPixelNumR = (rTmpAwb.i4WindowSizeX * rTmpAwb.i4WindowSizeY) / 4;
        int i4WindowPixelNumG = i4WindowPixelNumR * 2;
        int i4WindowPixelNumB = i4WindowPixelNumR;
        int i4PixelCountR = ((1 << 24) + (i4WindowPixelNumR >> 1)) / i4WindowPixelNumR;
        int i4PixelCountG = ((1 << 24) + (i4WindowPixelNumG >> 1)) / i4WindowPixelNumG;
        int i4PixelCountB = ((1 << 24) + (i4WindowPixelNumB >> 1)) / i4WindowPixelNumB;

        rTmpAwb.i4PixelCountR = i4PixelCountR;
        rTmpAwb.i4PixelCountG = i4PixelCountG;
        rTmpAwb.i4PixelCountB = i4PixelCountB;

        rTmpAwb.i4ErrorThreshold = 20;
        rTmpAwb.i4ErrorShiftBits = 0;

        rTmpAwb.i4Cos = 256;
        rTmpAwb.i4Sin = 0;


        rTmpAwb.i4AWBXY_WINL[0] = -250;
        rTmpAwb.i4AWBXY_WINR[0] = -100;
        rTmpAwb.i4AWBXY_WIND[0] = -600;
        rTmpAwb.i4AWBXY_WINU[0] = -361;

        rTmpAwb.i4AWBXY_WINL[1] = -782;
        rTmpAwb.i4AWBXY_WINR[1] = -145;
        rTmpAwb.i4AWBXY_WIND[1] = -408;
        rTmpAwb.i4AWBXY_WINU[1] = -310;

        rTmpAwb.i4AWBXY_WINL[2] = -782;
        rTmpAwb.i4AWBXY_WINR[2] = -145;
        rTmpAwb.i4AWBXY_WIND[2] = -515;
        rTmpAwb.i4AWBXY_WINU[2] = -408;

        rTmpAwb.i4AWBXY_WINL[3] = -145;
        rTmpAwb.i4AWBXY_WINR[3] = 18;
        rTmpAwb.i4AWBXY_WIND[3] = -454;
        rTmpAwb.i4AWBXY_WINU[3] = -328;

        rTmpAwb.i4AWBXY_WINL[4] = -145;
        rTmpAwb.i4AWBXY_WINR[4] = 23;
        rTmpAwb.i4AWBXY_WIND[4] = -540;
        rTmpAwb.i4AWBXY_WINU[4] = -454;

        rTmpAwb.i4AWBXY_WINL[5] = 18;
        rTmpAwb.i4AWBXY_WINR[5] = 199;
        rTmpAwb.i4AWBXY_WIND[5] = -454;
        rTmpAwb.i4AWBXY_WINU[5] = -328;

        rTmpAwb.i4AWBXY_WINL[6] = 199;
        rTmpAwb.i4AWBXY_WINR[6] = 529;
        rTmpAwb.i4AWBXY_WIND[6] = -427;
        rTmpAwb.i4AWBXY_WINU[6] = -328;

        rTmpAwb.i4AWBXY_WINL[7] = 23;
        rTmpAwb.i4AWBXY_WINR[7] = 199;
        rTmpAwb.i4AWBXY_WIND[7] = -540;
        rTmpAwb.i4AWBXY_WINU[7] = -454;

        rTmpAwb.i4AWBXY_WINL[8] = 0;
        rTmpAwb.i4AWBXY_WINR[8] = 0;
        rTmpAwb.i4AWBXY_WIND[8] = 0;
        rTmpAwb.i4AWBXY_WINU[8] = 0;

        rTmpAwb.i4AWBXY_WINL[9] = 0;
        rTmpAwb.i4AWBXY_WINR[9] = 0;
        rTmpAwb.i4AWBXY_WIND[9] = 0;
        rTmpAwb.i4AWBXY_WINU[9] = 0;

        m_pTuning->setSensorMode(m_u4SensorDev, m_u4SensorMode, 0, m_i4QBINSzW, m_i4QBINSzH);
        m_pTuning->setIspProfile(m_u4SensorDev, NSIspTuning::EIspProfile_Preview);
        RequestSet_T rRequestSet;
        rRequestSet.vNumberSet.clear();
        rRequestSet.vNumberSet.push_back(1);

        m_pTuning->validate(m_u4SensorDev, rRequestSet, MTRUE);

        CAM_LOGD("[%s] -", __FUNCTION__);
    }
    return S_3A_OK;
}

MBOOL
Dft3AImp::
start()
{
    CAM_LOGD("[%s]", __FUNCTION__);
    return MTRUE;
}


MBOOL
Dft3AImp::
stop()
{
    if (m_pCamIO != NULL)
    {
        CAM_LOGD("[%s] +", __FUNCTION__);
        m_pCamIO->destroyInstance(LOG_TAG);
        m_pCamIO = NULL;
        CAM_LOGD("[%s] -", __FUNCTION__);
    }
    return MTRUE;
}

MBOOL
Dft3AImp::
generateP2(MINT32 /*flowType*/, const NSIspTuning::ISP_INFO_T& /*rIspInfo*/, void* /*pRegBuf*/, ResultP2_T* /*pResultP2*/)
{
    CAM_LOGD("[%s]", __FUNCTION__);
    return MTRUE;
}

MBOOL
Dft3AImp::
validateP1(const ParamIspProfile_T& rParamIspProfile, MBOOL fgPerframe)
{
    CAM_LOGD("[%s]", __FUNCTION__);
    m_pTuning->setIspProfile(m_u4SensorDev, rParamIspProfile.eIspProfile);
    m_pTuning->notifyRPGEnable(m_u4SensorDev, rParamIspProfile.iEnableRPG);
    m_pTuning->validatePerFrame(m_u4SensorDev, rParamIspProfile.rRequestSet, fgPerframe);
    return MTRUE;
}

MBOOL
Dft3AImp::
setParams(Param_T const &/*rNewParam*/, MBOOL /*bUpdateScenario*/)
{
    CAM_LOGD("[%s]", __FUNCTION__);
    return MTRUE;
}

MBOOL
Dft3AImp::
setAfParams(AF_Param_T const &/*rNewParam*/)
{
    CAM_LOGD("[%s]", __FUNCTION__);
    return MTRUE;
}


MBOOL
Dft3AImp::
autoFocus()
{
    CAM_LOGD("[%s]", __FUNCTION__);
    return MTRUE;
}

MBOOL
Dft3AImp::
cancelAutoFocus()
{
    CAM_LOGD("[%s]", __FUNCTION__);
    return MTRUE;
}

MVOID
Dft3AImp::
setFDEnable(MBOOL /*fgEnable*/)
{
    CAM_LOGD("[%s]", __FUNCTION__);
}

MBOOL
Dft3AImp::
setFDInfo(MVOID* /*prFaces*/, MVOID* /*prAFFaces*/)
{
    CAM_LOGD("[%s]", __FUNCTION__);
    return MTRUE;
}

MBOOL
Dft3AImp::
setOTInfo(MVOID* /*prOT*/, MVOID* /*prAFOT*/)
{
    CAM_LOGD("[%s]", __FUNCTION__);
    return MTRUE;
}


MBOOL
Dft3AImp::
setFlashLightOnOff(MBOOL /*bOnOff*/, MBOOL /*bMainPre*/, MINT32 /*i4P1DoneSttNum*/)
{
    CAM_LOGD("[%s]", __FUNCTION__);
    return MTRUE;
}

MBOOL
Dft3AImp::
setPreFlashOnOff(MBOOL /*bOnOff*/)
{
    MY_LOG("[%s]", __FUNCTION__);
    return MTRUE;
}

MBOOL
Dft3AImp::
isNeedTurnOnPreFlash() const
{
    MY_LOG("[%s]", __FUNCTION__);
    return MFALSE;
}

MBOOL
Dft3AImp::
chkMainFlashOnCond() const
{
    CAM_LOGD("[%s]", __FUNCTION__);
    return MFALSE;
}

MBOOL
Dft3AImp::
chkPreFlashOnCond() const
{
    CAM_LOGD("[%s]", __FUNCTION__);
    return MFALSE;
}

MBOOL
Dft3AImp::
isStrobeBVTrigger() const
{
    CAM_LOGD("[%s]", __FUNCTION__);
    return MFALSE;
}

MINT32
Dft3AImp::
getCurrResult(MUINT32 /*i4FrmId*/, MINT32 /*i4SubsampleCount*/) const
{
    CAM_LOGD("[%s]", __FUNCTION__);
    return 0;
}

MINT32
Dft3AImp::
getCurrentHwId() const
{
    MINT32 idx = 0;
    m_pCamIO->sendCommand(
                        NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_CUR_SOF_IDX,
                        (MINTPTR)&idx, 0, 0);
    CAM_LOGD("[%s] idx(%d)", __FUNCTION__, idx);
    return idx;
}

MVOID
Dft3AImp::
setSensorMode(MINT32 i4SensorMode)
{
    CAM_LOGD("[%s]+ sensorDev(%d), Mode(%d)", __FUNCTION__, m_u4SensorDev, i4SensorMode);

    m_u4SensorMode = i4SensorMode;

    CAM_LOGD("[%s]-", __FUNCTION__);
}

MBOOL
Dft3AImp::
postCommand(ECmd_T const r3ACmd, const ParamIspProfile_T* pParam)
{
    if (ECmd_Update == r3ACmd)
    {
        CAM_LOGD("[%s]+ #(%d)", __FUNCTION__, pParam->i4MagicNum);
        m_u4Count ++;
#if (CAM3_3ATESTLVL >= CAM3_3ASTTUT)
        MUINT enable;
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.debug.stt_flow.enable", value, "1");
        enable = atoi(value);
        char strFile[512] = {'\0'};

        if (enable & ENABLE_STT_FLOW_AAO)
        {
            IBufMgr* pAAOBufMgr = Hal3ASttCtrl::getInstance(m_u4SensorDev)->getBufMgr(BUF_AAO);
            if(pAAOBufMgr == NULL) return MTRUE;

            const StatisticBufInfo* pAABuf = pAAOBufMgr->dequeueSwBuf();
            sprintf(strFile, "/sdcard/dft3a_aao/aao_%d", m_u4Count);
            pAABuf->dump(strFile);
        }
        if (enable & ENABLE_STT_FLOW_AFO)
        {
            IBufMgr* pAFOBufMgr = Hal3ASttCtrl::getInstance(m_u4SensorDev)->getBufMgr(BUF_AFO);
            if(pAFOBufMgr == NULL) return MTRUE;

            const StatisticBufInfo* pAFBuf = pAFOBufMgr->dequeueSwBuf();
            sprintf(strFile, "/sdcard/dft3a_afo/afo_%d", m_u4Count);
            pAFBuf->dump(strFile);
        }
#endif
        validateP1(*pParam, MTRUE);
        CAM_LOGD("[%s]- #(%d)", __FUNCTION__, pParam->i4MagicNum);
    }

    return MTRUE;
}

MINT32
Dft3AImp::
send3ACtrl(E3ACtrl_T /*e3ACtrl*/, MINTPTR /*iArg1*/, MINTPTR /*iArg2*/)
{
    return 0;
}

VOID
Dft3AImp::
queryTgSize(MINT32 &/*i4TgWidth*/, MINT32 &/*i4TgHeight*/)
{
    return ;
};

MINT32
Dft3AImp::
attachCb(I3ACallBack* /*cb*/)
{
    return 0;
}

MINT32
Dft3AImp::
detachCb(I3ACallBack* /*cb*/)
{
    return 0;
}

VOID
Dft3AImp::
queryHbinSize(MINT32 &/*i4HbinWidth*/, MINT32 &/*i4HbinHeight*/)
{
    return ;
}

MBOOL
Dft3AImp::
notifyPwrOn()
{
    return MTRUE;
}

MBOOL
Dft3AImp::
notifyPwrOff()
{
    return MTRUE;
}

MBOOL
Dft3AImp::
notifyP1PwrOn()
{
    return MTRUE;
}

MBOOL
Dft3AImp::
notifyP1PwrOff()
{
    return MTRUE;
}

MVOID
Dft3AImp::
setIspSensorInfo2AF(MINT32)
{
    MY_LOG("[%s]-", __FUNCTION__);
}

MBOOL
Dft3AImp::
notifyResult4TG(MINT32 /*i4PreFrmId*/)
{
    return MTRUE;
}

MBOOL
Dft3AImp::
notify4CCU(MUINT32 /*u4PreFrmId*/, ISP_NVRAM_OBC_T const &/*rOBCResult*/)
{
    return MTRUE;
}


MBOOL
Dft3AImp::
setP2Params(P2Param_T const &/*rNewP2Param*/, ResultP2_T* /*pResultP2*/)
{
    return MTRUE;
}

MBOOL
Dft3AImp::
setISPInfo(P2Param_T const &/*rNewP2Param*/, NSIspTuning::ISP_INFO_T &/*rIspInfo*/, MINT32 /*type*/)
{
    return MTRUE;
}

MBOOL
Dft3AImp::
preset(Param_T const &/*rNewParam*/)
{
    return MTRUE;
}

MBOOL
Dft3AImp::
getP2Result(P2Param_T const &/*rNewP2Param*/, ResultP2_T* /*pResultP2*/)
{
    return MTRUE;
}

MVOID
Dft3AImp::
notifyPreStop()
{
    MY_LOG("[%s]", __FUNCTION__);
}
