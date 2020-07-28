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
#ifndef _I_AE_FLOW_H_
#define _I_AE_FLOW_H_


#include <aaa_types.h>
#include <isp_tuning.h>
#include <ae_feature.h>
#include <ae_param.h>
#include <custom/aaa/AEPlinetable.h>

using namespace NSIspTuning;

typedef struct
{
    MUINT32 m_u4EffectiveIndex;
    MUINT32 u4Eposuretime;
    MUINT32 u4AfeGain;
    MUINT32 u4IspGain;
}AE_EXP_INFO;

typedef enum
{
    E_AE_FLOW_CPU_DOPVAE = 0,
    E_AE_FLOW_CPU_DOAFAE,
    E_AE_FLOW_CPU_DOPRECAPAE,
    E_AE_FLOW_CPU_DOCAPAE,
    E_AE_FLOW_CPU_DOAFASSIST
} E_AE_FLOW_CPU_T;

typedef enum
{
    E_AE_FLOW_CPU_SCHEDULER_RESET = 0,
    E_AE_FLOW_CPU_SCHEDULE_COUNT,
    E_AE_FLOW_CPU_SCHEDULE_STABLE,
    E_AE_FLOW_CPU_CYCLEINFO_ADDCOUNT,
    E_AE_FLOW_CPU_CYCLEINFO_RESET,
    E_AE_FLOW_CPU_CYCLEINFO_AERESET,
    E_AE_FLOW_CPU_CYCLEINFO_COUNTWAIT
} E_AE_FLOW_CPU_TRIGER_T;

typedef enum
{
    E_AE_FLOW_CPU_RESTORE_WAIT = 0,
    E_AE_FLOW_CPU_TG_RETURN,
    E_AE_FLOW_CPU_TG_VALID,
    E_AE_FLOW_CPU_ADB_UPDATE,
    E_AE_FLOW_CPU_ADB_LOCK,
    E_AE_FLOW_CPU_ADB_EXP,
    E_AE_FLOW_CPU_ADB_AFE,
    E_AE_FLOW_CPU_ADB_ISP,
    E_AE_FLOW_CPU_CYCLECOUNT_VALID,
    E_AE_FLOW_CPU_PREVQUEUE_EXP,
    E_AE_FLOW_CPU_PREVQUEUE_AFE,
    E_AE_FLOW_CPU_MANUAL_OVERSPEC
} E_AE_FLOW_CPU_STATUS_T;

typedef struct
{
    AE_INITIAL_INPUT_T* pAEInitInput; //AeMgr init input
    strFinerEvPline* pPreviewTableF;
    strAETable* pPreviewTableCurrent;
    MUINT32 u4IndexFMax;
    MUINT32 u4IndexFMin;
    MUINT32 u4FinerEVIdxBase;
    MUINT32 u4AAOmode;
    MINT32  i4SensorIdx;
    ESensorMode_T eSensorMode;

}AE_FLOW_INPUT_INFO_T;

typedef struct
{
    strFinerEvPline* pPreviewTableF;
    strAETable* pPreviewTableCurrent;
    LIB3A_AE_EVCOMP_T eAEEVcomp;
    MBOOL bZoomChange;
    MUINT32 u4IndexFMax;
    MUINT32 u4IndexFMin;
    MINT32  i4AEMaxFps;
    MUINT32 u4HwMagicNum;
    MINT32 i4ZoomRatio;
}AE_FLOW_CONTROL_INFO_T;

typedef struct
{
    E_AE_FLOW_CPU_T eAEFlowType;
    MBOOL bAAASchedule;
    eAETargetMODE eAETargetMode;
    MBOOL bPerframeAEFlag;
    MBOOL bTouchAEAreaChange;
    MBOOL bFaceAEAreaChange;
    MBOOL bAElimitor;
    MBOOL bAEReadyCapture;
    MBOOL bAEOneShotControl;
    MINT32 i4ActiveItem;
    MINT32 i4CwvY;
    MUINT32 u4CwvYStable;
    MUINT32 u4WOFDcnt;
    MINT32 i4WaitVDNum;
    MUINT32 u4HwMagicNum;
    MBOOL AFTouchROISet;

}AE_FLOW_SCHEDULE_INFO_T;

typedef struct
{
    MBOOL*  pModifiedPerframeFlag;
    MBOOL*  pModifiedCalc;
    MBOOL*  pModifiedApply;
    MINT32* pWaitVDNum;

}AE_FLOW_SCHEDULE_OUTPUT_T;

typedef struct
{
    E_AE_FLOW_CPU_T eAEFlowType;
    MVOID *pAEStatisticBuf;
    MINT64 i8TimeStamp;
    MBOOL bAEReadyCapture;
    MBOOL bAElock;
    MBOOL bEnSWBuffMode;
    MBOOL bAEOneShotControl;
    MINT32* i4AcceInfo;
    MINT32* i4GyroInfo;
    MBOOL bIs60Hz;
    MBOOL bRealISOSpeed;
    MUINT32 u4AEISOSpeed;
    MUINT32 u4VHDRratio;
    //AF2AEinfo
    MINT32 i4AF2AE_MagicNum;
    MINT32 i4AF2AE_IsAFDone;
    MINT32 i4AF2AE_AfDac;
    MINT32 i4AF2AE_IsSceneStable;
    //manual control
    MUINT32 u4AEMode;
    MUINT32 u4TargetExp;
    MUINT32 u4TargetISO;
    MUINT32 u41xGainISO;
    MUINT32 u4MinGain;
    MUINT32 u4MaxShutter;
    MUINT32 u4MaxSensorGain;
    MUINT32 u4MaxISPGain;
    MUINT32 u4FinerEVIdxBase;
}AE_FLOW_CALCULATE_INFO_T;


typedef struct
{
    MVOID* pIAeAlgo;
    AE_PARAM_T* pAEInitInput; //AeMgr init input
    AE_NVRAM_T* pAEInitInputNVRAM;
    eAETargetMODE eAETargetMode;
    MUINT32 i4CycleVDNum;
    MUINT32 i4ShutterDelayFrames;
    MUINT32 i4GainDelayFrames;
    MUINT32 i4IspGainDelayFrames;
    MUINT32 u4Index;
    MUINT32 u4Exp;
    MUINT32 u4Afe;
    MUINT32 u4Isp;
    MUINT16 u2VCModeSelect;

}AE_FLOW_CPUSTART_INFO_T;

namespace NS3Av3
{
class IAeFlow
{
public:

    typedef enum
    {
        E_AE_FLOW_TYPE_DFT = 0
    } E_AE_FLOW_TYPE_T;

    static IAeFlow*    getInstance(E_AE_FLOW_TYPE_T etype, ESensorDev_T sensor);
    virtual MVOID      init() = 0;
    virtual MVOID      uninit() = 0;
    virtual MVOID      start(MVOID* pAEInitInput) = 0;
    virtual MVOID      pause() = 0;
    virtual MVOID      stop() = 0;

    virtual MBOOL      queryStatus(MUINT32 index) = 0;
    virtual MVOID      getParam(MUINT32 index, MUINT32& i4value) = 0;
    virtual MVOID      controltrigger(MUINT32 index, MBOOL btrigger = MFALSE) = 0;
    virtual MVOID      schedulePvAE(MVOID* input, MVOID* output) = 0;
    virtual MVOID      calculateAE(MVOID* input, MVOID* output) = 0;
    virtual MVOID      setupParamPvAE(MVOID* input, MVOID* output) = 0;
    virtual MVOID      update(AE_EXP_INFO* input) = 0;

protected:
    virtual            ~IAeFlow(){}
};
};
#endif //_I_AE_FLOW_H_