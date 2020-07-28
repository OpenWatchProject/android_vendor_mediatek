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
#ifndef _CCUIF_COMPAT_H_
#define _CCUIF_COMPAT_H_

//#include "ccu_sensor_if.h"
#include "ccu_ext_interface/ccu_ext_interface.h"
#include "algorithm/ae_algo_ctrl.h"
#include "ccu_ext_interface/ccu_af_reg.h"


/*******************************************************************************
* ARM linux kernel 64 -> CCU 32 compatible
* Make all pointers as 32bit MVA,
* avoiding pointer type compatible problem in kernel driver
********************************************************************************/
#define CCU_COMPAT_PTR_T MUINT32

namespace NSCcuIf {

typedef struct COMPAT_SENSOR_INFO_IN
{
    //U32 u32SensorId;
    U16 u16FPS;                     // Current FPS used by set_mode()
    U32 eScenario;            // Preview, Video, HS Video, and etc.
    U32 sensorI2cSlaveAddr;
    U32 dma_buf_va_h;
    U32 dma_buf_va_l;
    U32 u32I2CId;
    CCU_COMPAT_PTR_T dma_buf_mva;  // virtual address of I2C DMA buffer
    enum ccu_tg_info tg_info;
    bool isSpSensor;
} COMPAT_SENSOR_INFO_IN_T;

typedef struct {
    MUINT16 u2Length;
} COMPAT_CCU_ISP_NVRAM_ISO_INTERVAL_STRUCT;


typedef struct
{
    MBOOL Call_AE_Core_init;
    MUINT32 u4LastIdx;
    MUINT32 EndBankIdx;
    MUINT32 u4InitIndex;


    MINT32  m_i4AEMaxBlockWidth;
    MINT32  m_i4AEMaxBlockHeight;
    // MUINT32 m_pWtTbl_W[5][5];
    // MBOOL   m_AECmdSet_bRotateWeighting;
    MINT32 OBC_ISO_IDX_Range[5];
    MUINT32 m_u4AETarget;

    /* OBC version2 */
    MUINT16 u2LengthV2;
    MINT16 IDX_PartitionV2[NVRAM_ISP_REGS_ISO_SUPPORT_NUM];
    CCU_ISP_NVRAM_OBC_T OBC_TableV2[NVRAM_ISP_REGS_ISO_SUPPORT_NUM];
    CCU_ISP_NVRAM_OBC_T OBC_Table[4];

    MUINT32 m_LineWidth;
    MUINT32 m_u4IndexFMax;
    MUINT32 m_u4IndexFMin;
    MUINT32 m_u4IndexMax;
    MUINT32 m_u4IndexMin;
    MUINT32 m_u4FinerEVIdxBase; // Real index base
    MUINT32 m_u4AAO_AWBValueWinSize;            // bytes of AWB Value window
    MUINT32 m_u4AAO_AWBSumWinSize;              // bytes of AWB Sum window 
    MUINT32 m_u4AAO_AEYWinSize;                 // bytes of AEY window
    MUINT32 m_u4AAO_AEOverWinSize;              // bytes of AEOverCnt window
    MUINT32 m_u4AAO_HistSize;                   // bytes of each Hist
    MUINT32 m_u4PSO_SE_AWBWinSize;              // bytes of AWB window
    MUINT32 m_u4PSO_LE_AWBWinSize;              // bytes of AWB window
    MUINT32 m_u4PSO_SE_AEYWinSize;              // bytes of AEY window  
    MUINT32 m_u4PSO_LE_AEYWinSize;              // bytes of AEY window
    MUINT32 m_u4PSO_SE_HistSize;                // bytes of each Hist
    MUINT32 m_u4PSO_LE_HistSize;                // bytes of each Hist
    MINT32 i4MaxBV;
    MINT32 i4MinBV;
    MINT32 i4BVOffset;
    MUINT16 u2Length;

    //CCU_AE_DEVICES_INFO_T rDevicesInfo;

    //AE_NVRAM_T rAENVRAM;         // AE NVRAM param
    //AE_PARAM_T rAEPARAM;
    //AE_PLINETABLE_T *rAEPlineTable;
    //AE_PLINEMAPPINGTABLE_T rAEPlineMapTable[30];
    //EZOOM_WINDOW_T rEZoomWin;
    //MINT32 i4AEMaxBlockWidth;  // AE max block width
    //MINT32 i4AEMaxBlockHeight; // AE max block height
    //MINT32 i4AAOLineByte;
    CCU_LIB3A_AE_METERING_MODE_T eAEMeteringMode;
    CCU_LIB3A_AE_SCENE_T eAEScene;
    CCU_LIB3A_AECAM_MODE_T eAECamMode;
    CCU_LIB3A_AE_FLICKER_MODE_T eAEFlickerMode;
    CCU_LIB3A_AE_FLICKER_AUTO_MODE_T eAEAutoFlickerMode;
    CCU_LIB3A_AE_EVCOMP_T eAEEVcomp;
    MUINT32 u4AEISOSpeed;
    MINT32    i4AEMaxFps;
    MINT32    i4AEMinFps;
    MINT32    i4SensorMode;
    CCU_AE_SENSOR_DEV_T eSensorDev;
    CCU_CAM_SCENARIO_T eAEScenario;

    CCU_COMPAT_PTR_T m_LumLog2x1000;
    CCU_COMPAT_PTR_T pCurrentTableF;
    CCU_COMPAT_PTR_T pCurrentTable;
    CCU_COMPAT_PTR_T pEVValueArray;
    CCU_COMPAT_PTR_T IDX_Partition;
    CCU_COMPAT_PTR_T IDX_Part_Middle;
    //Smooth
    CCU_COMPAT_PTR_T rAEMovingRatio;
    CCU_COMPAT_PTR_T pAETouchMovingRatio;
    CCU_COMPAT_PTR_T rAEVideoMovingRatio;    // Video
    CCU_COMPAT_PTR_T rAEFaceMovingRatio;     // Face AE
    CCU_COMPAT_PTR_T rAETrackingMovingRatio; // Object Tracking
    CCU_COMPAT_PTR_T pAeNVRAM;
}COMPAT_AE_CORE_INIT;

typedef struct
{
    CCU_AE_CONTROL_CFG_T control_cfg;
    COMPAT_AE_CORE_INIT algo_init_param;
    CCU_COMPAT_PTR_T sync_algo_init_param;
} COMPAT_CCU_AE_INITI_PARAM_T;

typedef struct
{
    MBOOL auto_flicker_en;
    MUINT32 m_u4IndexMax;
    MUINT32 m_u4IndexMin;
    MUINT32 m_u4IndexFMax;
    MUINT32 m_u4IndexFMin;
    MUINT32 m_u4IndexF;
    MUINT32 m_u4Index;
    MINT32 i4MaxBV;
    MINT32 i4MinBV;
    /* OBC version2 */
    // MUINT16 u2LengthV2;
    // MINT16 IDX_PartitionV2[NVRAM_ISP_REGS_ISO_SUPPORT_NUM];
    CCU_ISP_NVRAM_OBC_T OBC_TableV2[NVRAM_ISP_REGS_ISO_SUPPORT_NUM];
    CCU_ISP_NVRAM_OBC_T OBC_Table[4];

    MUINT32 m_u4FinerEVIdxBase;
    MUINT32 m_u4Prvflare;
    MUINT32 u4UpdateLockIndex;
    CCU_COMPAT_PTR_T pCurrentTable;
    CCU_COMPAT_PTR_T pCurrentTableF;
    CCU_COMPAT_PTR_T pAeNVRAM;
} COMPAT_ccu_ae_onchange_data;

};  //namespace NSCcuIf
#endif  //  _CCUIF_H_

