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
#ifndef _AE_FLOW_DFT_H_
#define _AE_FLOW_DFT_H_


#include <aaa_types.h>
#include <isp_tuning.h>
#include <AeFlow/IAeFlow.h>
#include <AeFlow/AeFlowCPU.h>
#include <dbg_aaa_param.h>
#include <dbg_ae_param.h>
#include <ae_algo_if.h>


using namespace NSIspTuning;
namespace NS3Av3
{
class AeFlowDefault : public IAeFlow
{
public:

    static IAeFlow*    getInstance(ESensorDev_T sensor);
    virtual MVOID      init(){};
    virtual MVOID      uninit() {}
    virtual MVOID      start(MVOID* pAEInitInput);
    virtual MVOID      pause(MBOOL IsNeedCCUSensorInit) {}
    virtual MVOID      stop() {}
    virtual MBOOL      queryStatus(MUINT32 index);
    virtual MVOID      getParam(MUINT32 index, MUINT32& i4value);
    virtual MVOID      controltrigger(MUINT32 index, MBOOL btrigger);
    virtual MVOID      schedulePvAE(MVOID* input, MVOID* output);
    virtual MVOID      calculateAE(MVOID* input, MVOID* output);
    virtual MVOID      setupParamPvAE(MVOID* input, MVOID* output) {(void)input;(void)output;}
    virtual MVOID      update(AE_CORE_CTRL_CPU_EXP_INFO* input);
    virtual MVOID      getdebuginfo(AE_DEBUG_INFO_T &exif_info) {(void)exif_info;}
    virtual MVOID      getAEInfoForISP(AE_INFO_T &ae_info){(void)ae_info;}
    virtual MVOID      PresetControlCCU(AE_FLOW_CONTROL_INFO_T* pcontrolInfo){(void)pcontrolInfo;}
    virtual MVOID      setMVHDR3ExpoProcInfo(MVOID* pMVHDR3ExpoStatBuf, MUINT32 u4MVHDRRatio_x100);
    virtual MVOID      getCCUResult(AE_CORE_MAIN_OUT* poutput,MBOOL isCCUResultGet){(void)poutput;(void)isCCUResultGet;}

                         AeFlowDefault(ESensorDev_T sensor);
    virtual            ~AeFlowDefault();
protected:
    MVOID   monitorAndReschedule(MBOOL bFaceAEAreaChage, MBOOL bTouchAEAreaChage,  MUINT32 u4AvgYcur, MUINT32 u4AvgYStable, MUINT32 u4WOFDCnt, MBOOL *bAECalc, MBOOL *bAEApply);
    MBOOL   AEStatisticChange( MUINT32 u4AvgYcur, MUINT32 u4AvgYpre, MUINT32 u4thres);
    MVOID   updateAECycleInfo(MINT32 i4cyclenum, MINT32 i4Shutter, MINT32 i4AfeGain, MINT32 i4ISPgain);
    MVOID   updatePreEvSettingQueue(MUINT32 newExpsuretime, MUINT32 newAfegain, MUINT32 newIspGain, MINT32 i4AEidxNext);

    NS3A::IAeAlgo* m_pIAeAlgo;
    ESensorDev_T m_eSensorDev;
    MUINT32 m_3ALogEnable;
    AE_PARAM_T* m_pAEParam;
    AE_NVRAM_T* m_pAENVRAM;
    //AE scheduler
    MUINT32 m_u4AEExitStableCnt;
    MUINT32 m_u4AEScheduleCnt;
    MINT32 m_i4WaitVDNum;
    MBOOL m_bAEMonitorStable;
	MBOOL m_bManualOverSpec;
    MUINT32 m_u4HwMagicNum;
    //Prepare HandleAE param
    MBOOL m_bAdbAEEnable;
    MBOOL m_bAdbAELock;
    MBOOL m_bAdbAELog;
    MBOOL m_bPerframeAEFlag;
    MBOOL m_bTouchAEAreaChange;
    eAETargetMODE m_eAETargetMode;
    MBOOL m_bAAASchedule;
    MBOOL m_bAElimitor;
    MBOOL m_bAFTouchROISet;
    // Calculate
    strAECycleInfo m_strAECycleInfo;
    strEvSetting   m_PreEvSettingQueue[MAX_AE_PRE_EVSETTING];
    MUINT32  m_u4PreAEidxQueue[MAX_AE_PRE_EVSETTING];
    MUINT32  m_u4PreTgIntAEidxQueue[MAX_AE_PRE_EVSETTING];
    strEvSetting   m_BackupEvSetting;
    // Adb test
    MBOOL m_bAdbAEPreviewUpdate;
    MINT32 m_i4AdbAEISPDisable;
    MUINT32 m_u4AdbAEShutterTime;
    MUINT32 m_u4AdbAESensorGain;
    MUINT32 m_u4AdbAEISPGain;
    MBOOL m_bTGValid;
    MBOOL m_bTGReturn;
    MUINT16 m_u2VCModeSelect;
    //Perframe-AE working
    MBOOL m_bPerFrameAEWorking;
};
};
#endif //_AE_FLOW_DFT_H_
