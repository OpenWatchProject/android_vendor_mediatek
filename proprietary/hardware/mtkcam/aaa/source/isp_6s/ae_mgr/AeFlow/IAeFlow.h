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
#include "ae_mgr_if.h"

//CCU
#define AE_MGR_INCLUDING_CCU
#include "ccu_ext_interface/ccu_ext_interface.h"
#include "iccu_ctrl_ae.h"
#include "iccu_mgr.h"

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

using namespace NSIspTuning;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                          CPU Flow Control                                  //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

typedef enum
{
    E_AE_ALGO_CPU_DEFAULT = 0, // [CPU DFT AE algo]
    E_AE_ALGO_CCU_DEFAULT,     // [CCU DFT AE algo]
    E_AE_ALGO_CPU_CUSTOM       // [CCU CUS AE algo]
} E_AE_ALGO_TYPE_T;

typedef enum
{
    E_AE_FLOW_CPU_TRIGGER_FRAME_INFO_UPDATE = 0,
    E_AE_FLOW_CPU_TRIGGER_ONCH_INFO_UPDATE
} E_AE_FLOW_CPU_TRIGER_T;

typedef enum
{
//    E_AE_FLOW_CPU_RESTORE_WAIT = 0,
//    E_AE_FLOW_CPU_TG_RETURN,
//    E_AE_FLOW_CPU_TG_VALID,
//    E_AE_FLOW_CPU_ADB_UPDATE,
//    E_AE_FLOW_CPU_ADB_LOCK,
//    E_AE_FLOW_CPU_ADB_EXP,
//    E_AE_FLOW_CPU_ADB_AFE,
//    E_AE_FLOW_CPU_ADB_ISP,
//    E_AE_FLOW_CPU_CYCLECOUNT_VALID

} E_AE_FLOW_CPU_STATUS_T;

typedef struct
{
    E_AE_ALGO_TYPE_T mCPUAlgoType;
    AE_CORE_INIT mInitData;

    // custom usage
    MINT32 i4SensorIdx;
    MBOOL bIsCCUAEInit;
}AE_FLOW_CPU_INIT_INFO_T;

typedef struct
{
    AE_CORE_CTRL_CCU_VSYNC_INFO mVsyncInfo;
    AE_CORE_CTRL_RUN_TIME_INFO  mRunTimeInfo;
    AE_CORE_MAIN_IN             mAEMainIn;
}AE_FLOW_CPU_CALCULATE_INFO_T;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                          CCU Flow Control                                  //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

typedef enum
{
    E_AE_FLOW_CCU_TRIGGER_FRAME_INFO_UPDATE = 0,
    E_AE_FLOW_CCU_TRIGGER_ONCH_INFO_UPDATE,
    E_AE_FLOW_CCU_TRIGGER_MAX_FPS_UPDATE,
    E_AE_FLOW_CCU_TRIGGER_FLICKER_ACTIVE_UPDATE
} E_AE_FLOW_CCU_TRIGER_T;

typedef enum
{
    E_AE_FLOW_CCU_WORKING = 0,
    E_AE_FLOW_CCU_AE_WORKING,
    E_AE_FLOW_CCU_SENSOR_SUPPORTED,
    E_AE_FLOW_CCU_AE_RESULT_GET
} E_AE_FLOW_CCU_STATUS_T;

typedef struct
{
    AE_INITIAL_INPUT_T* pAEInitInput; //AeMgr init input
    strFinerEvPline* pPreviewTableF;
    strAETable* pPreviewTableCurrent;
    //CCU_ISP_NVRAM_ISO_INTERVAL_STRUCT* pISPNvramISOInterval;
    CCU_ISP_NVRAM_OBC_T* pISPNvramOBC_Table;
    CCU_ISP_NVRAM_OBC_T* pISP5NvramOBC_Table;
    MUINT32 u4IndexFMax;
    MUINT32 u4IndexFMin;
    MUINT32 u4FinerEVIdxBase;
    MUINT32 u4AAOmode;
    MINT32  i4SensorIdx;
    ESensorMode_T eSensorMode;
    MUINT32 u4Index;
    MUINT32 u4IndexF;
    MUINT32 u4Exp;
    MUINT32 u4Afe;
    MUINT32 u4Isp;
    MUINT32 u4RealISO;
    MBOOL bIsCCUAEInit;
    MUINT16 u2Length;
    MINT16* u2IDX_Partition;
    AE_CORE_INIT mInitData;
    CCU_AE_TargetMODE eAETargetMode;

}AE_FLOW_CCU_INIT_INFO_T;

typedef struct
{
    strFinerEvPline* pPreviewTableF;
    strAETable* pPreviewTableCurrent;
    //LIB3A_AE_EVCOMP_T eAEEVcomp;
    //MBOOL bZoomChange;
    MBOOL bskipCCUAlgo;
    MBOOL bManualAE;
    MUINT32 u4IndexFMax;
    MUINT32 u4IndexFMin;
    MINT32  i4AEMaxFps;
    MUINT32 u4HwMagicNum;
    MUINT32 u4RequestNum;
    MUINT32 u4ManualExp;
    MUINT32 u4ManualISO;
    MUINT32 u4ManualAfeGain;
    MUINT32 u4ManualISPGain;
    AE_CORE_CTRL_CCU_VSYNC_INFO mFrameData;
    AE_CORE_CTRL_RUN_TIME_INFO  mOnchData;
    AE_STAT_PARAM_T rAEStatCfg;
    MINT32 i4ZoomRatio;
    //CCU_ISP_NVRAM_OBC_T* pISPNvramOBC_Table;
    CCU_ISP_NVRAM_OBC_T* pISP5NvramOBC_Table;
    MINT32 OBCTableidx;
    MUINT32 u4CurrentIndex;
    MUINT32 u4CurrentIndexF;
    MUINT32 u4AENVRAMIdx;
    MUINT32 u4FinerEVBase;
    CCU_AE_NVRAM_T* pAEOnchNVRAM;
    LIB3A_AE_METERING_MODE_T u4AEMeteringMode;
    MUINT32 u4Prvflare;
    MUINT32 u4FDProb;
    MUINT32 u4FaceFailCnt;
    MUINT32 u4FaceFoundCnt;
    MBOOL bUpdateAELockIdx;
    MBOOL bForceResetCCUStable;
}AE_FLOW_CCU_CONTROL_INFO_T;


namespace NS3Av3
{
class IAeFlow
{
public:

    typedef enum
    {
        E_AE_FLOW_TYPE_DFT = 0,
        E_AE_FLOW_TYPE_CCU
    } E_AE_FLOW_TYPE_T;

    static IAeFlow*    getInstance(E_AE_FLOW_TYPE_T etype, ESensorDev_T sensor);
    virtual MVOID      init() = 0;
    virtual MVOID      uninit() = 0;
    virtual MVOID      start(MVOID* pAEInitInput) = 0;
    virtual MVOID      pause() = 0;
    virtual MVOID      stop() = 0;

    virtual MBOOL      queryStatus(MUINT32 index) = 0;
    virtual MVOID      controltrigger(MUINT32 index, MBOOL btrigger = MFALSE) = 0;
    virtual MBOOL      calculateAE(MVOID* input, MVOID* output) = 0;
    virtual MVOID      getCCUResult(MVOID* poutput) = 0;
    virtual MVOID      getAEInfoForISP(AE_INFO_T &ae_info) = 0;
    virtual MVOID      getDebugInfo(MVOID* exif_info, MVOID* dbg_data_info) = 0;
    virtual MVOID      controlCCU(AE_FLOW_CCU_CONTROL_INFO_T* pcontrolInfo) = 0;

protected:
    virtual            ~IAeFlow(){}
};
};
#endif //_I_AE_FLOW_H_
