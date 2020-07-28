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


/*******************************************************************************
* ARM linux kernel 64 -> CCU 32 compatible
* Make all pointers as 32bit MVA,
* avoiding pointer type compatible problem in kernel driver
********************************************************************************/
#define CCU_COMPAT_PTR_T MUINT32

namespace NSCcuIf {

typedef struct COMPAT_SENSOR_INFO_IN
{
    U32 u32SensorId;
    U16 u16FPS;                     // Current FPS used by set_mode()
    SENSOR_SCENARIO_T eScenario;            // Preview, Video, HS Video, and etc.
    U16 u16BufferLen;               // length of I2C DMA buffer
    U32 sensorI2cSlaveAddr;
    U32 u32BufferPAddrH;
    U32 u32BufferPAddrL;
    U32 u32I2CId;
    CCU_COMPAT_PTR_T pu8BufferVAddr;  // virtual address of I2C DMA buffer
} COMPAT_SENSOR_INFO_IN_T;

typedef struct {
    MUINT16 u2Length;
} COMPAT_CCU_ISP_NVRAM_ISO_INTERVAL_STRUCT;


typedef struct
{
    MUINT32 EndBankIdx;
    MUINT32 m_i4AEMaxBlockWidth;
    MUINT32 m_i4AEMaxBlockHeight;
    MUINT32 uTgBlockNumX;          //m_AeTuningParam->strAEParasetting.uTgBlockNumX
    MUINT32 uTgBlockNumY;          //m_AeTuningParam->strAEParasetting.uTgBlockNumX
    MUINT32 m_u4IndexMax;
    MUINT32 m_u4IndexMin;
    MBOOL   bSpeedupEscOverExp;   //m_AeTuningParam->bSpeedupEscOverExp
    MBOOL   bSpeedupEscUnderExp;  //m_AeTuningParam->bSpeedupEscUnderExp
    strAETgTuningPara pAETgTuningParam; //m_AeTuningParam->m_AeTuningParam
    MUINT32 pTgWeightTbl[9][12];
    MUINT32 SzLumLog2x1000;
    MINT32  i4MaxBV;       //m_pPreviewTableCurrent->i4MaxBV or instace in g_AE_PreviewAutoTable,custom\mt6757\hal\imgsensor\s5k2p8_mipi_raw\camera_AE_PLineTable_s5k2p8raw.h
    MINT32  i4BVOffset;    //m_AeCCTConfig.i4BVOffset
    MUINT32 u4MiniISOGain; //m_pAENVRAM->rDevicesInfo.u4MiniISOGain
    MINT32  iMiniBVValue;  //m_AeTuningParam->strAEParasetting.iMiniBVValue
    MUINT32 StatisticStable;
    MUINT32 EVBase;

    CCU_ISP_NVRAM_OBC_T OBC_Table[5];
    MINT32 OBC_ISO_IDX_Range[5];

    MUINT16 u2IspNvIsoIntvl_Length;
    CCU_COMPAT_PTR_T pCurrentTable;         //m_pPreviewTableCurrent->pCurrentTable
    CCU_COMPAT_PTR_T pTgEVCompJumpRatioOverExp;  //m_AeTuningParam->pTgEVCompJumpRatioOverExp
    CCU_COMPAT_PTR_T pTgEVCompJumpRatioUnderExp; //m_AeTuningParam->pTgEVCompJumpRatioUnderExp
    CCU_COMPAT_PTR_T LumLog2x1000;
    CCU_COMPAT_PTR_T IspNvIsoIntvl_IDX_Partition;
    CCU_COMPAT_PTR_T IspNvIsoIntvl_IDX_Part_Middle;
}COMPAT_AE_CORE_INIT;

typedef struct
{
    CCU_AE_CONTROL_CFG_T control_cfg;
    COMPAT_AE_CORE_INIT algo_init_param;
} COMPAT_CCU_AE_INITI_PARAM_T;

typedef struct
{
    MBOOL auto_flicker_en;
    MUINT32 m_u4IndexMax;
    MUINT32 m_u4IndexMin;
    CCU_COMPAT_PTR_T pCurrentTable;
} COMPAT_ccu_ae_onchange_data;

};  //namespace NSCcuDrv
#endif  //  _CCUIF_H_

