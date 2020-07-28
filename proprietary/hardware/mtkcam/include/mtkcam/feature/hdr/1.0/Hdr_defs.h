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
#ifndef _HDR_DEFS_H_
#define _HDR_DEFS_H_

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/
#define L1_CACHE_BYTES 32 


typedef enum {
    HDR_STATE_INIT                    = 0x0000,
    HDR_STATE_NORMALIZATION            = 0x0001,
    HDR_STATE_FEATURE_EXTRACITON    = 0x0002,
    HDR_STATE_ALIGNMENT                = 0x0003,
    HDR_STATE_BLEND                = 0x0004,
    HDR_STATE_UNINIT                = 0x0800,
} HdrState_e;

enum 
{
    HDRProcParam_Begin = 0,
    HDRProcParam_Set_sensor_size        ,
    HDRProcParam_Set_sensor_type        ,
    HDRProcParam_Set_transform          ,
    HDRProcParam_Get_src_main_format    ,
    HDRProcParam_Get_src_main_size      ,
    HDRProcParam_Get_src_small_format   ,
    HDRProcParam_Get_src_small_size     ,
    HDRProcParam_Set_AOEMode            ,
    HDRProcParam_Set_MaxSensorAnalogGain,
    HDRProcParam_Set_MaxAEExpTimeInUS   ,   
    HDRProcParam_Set_MinAEExpTimeInUS   ,   
    HDRProcParam_Set_ShutterLineTime    ,    
    HDRProcParam_Set_MaxAESensorGain    ,    
    HDRProcParam_Set_MinAESensorGain    ,    
    HDRProcParam_Set_ExpTimeInUS0EV     ,     
    HDRProcParam_Set_SensorGain0EV      ,      
    HDRProcParam_Set_FlareOffset0EV     ,     
    HDRProcParam_Set_GainBase0EV        ,        
    HDRProcParam_Set_LE_LowAvg          ,          
    HDRProcParam_Set_SEDeltaEVx100      ,      
    HDRProcParam_Set_Histogram          , 
    HDRProcParam_Num
};

#endif  //  _HDR_DEFS_H_

