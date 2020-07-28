/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#include <cutils/log.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_gc8c34mipiraw.h"
#include "camera_info_gc8c34mipiraw.h"
#include "camera_custom_AEPlinetable.h"
#include "camera_custom_tsf_tbl.h"
#include "AE_Tuning_Para/camera_ae_tuning_para_gc8c34mipiraw.h"
#include "AWB_Tuning_Para/camera_awb_tuning_para_gc8c34mipiraw.h"

#define NVRAM_TUNING_PARAM_NUM  5341001
extern "C" fptrDefault getDefaultDataFunc(MUINT32 sensorType)
{
    if(sensorType==0)
        return NSFeature::RAWSensorInfo<SENSOR_ID>::getDefaultData;
    else
        return NSFeature::YUVSensorInfo<SENSOR_ID>::getDefaultData;
}

const NVRAM_CAMERA_ISP_PARAM_STRUCT CAMERA_ISP_DEFAULT_VALUE =
{{
    //Version
    Version: NVRAM_CAMERA_PARA_FILE_VERSION,

    //SensorId
    SensorId: SENSOR_ID,
    ISPComm:{
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      }
    },
    ISPPca: {
#include INCLUDE_FILENAME_ISP_PCA_PARAM
    },
    ISPRegs:{
#include INCLUDE_FILENAME_ISP_REGS_PARAM
    },
    ISPMfbMixer:{{
      0x01FF0001, // MIX3_CTRL_0
      0x00FF0000, // MIX3_CTRL_1
      0xFFFF0000  // MIX3_SPARE
    }},
    ISPMulitCCM:{
      Poly22:{
				91100,  // i4R_AVG
				20870,  // i4R_STD
				108725,  // i4B_AVG
				25983,  // i4B_STD
				4095,  // i4R_MAX
				512,  // i4R_MIN
				4095,  // i4G_MAX
				512,  // i4G_MIN
				4095,  // i4B_MAX
				512,  // i4B_MIN
				// i4P00
				{
					11290000, -6657500, 480000, -832500, 6032500, -80000, 710000, -4947500, 9350000
				},
				// i4P10
				{
					3015533, -3829186, 806966, -518739, -212671, 736365, -177371, 1853662, -1679492
				},
				// i4P01
				{
					917651, -1244703, 324335, -987835, -54996, 1036004, -515998, 1865289, -1356771
				},
				// i4P20
				{
					0, 0, 0, 0, 0, 0, 0, 0, 0
				},
				// i4P11
				{
					0, 0, 0, 0, 0, 0, 0, 0, 0
				},
				// i4P02
				{
					0, 0, 0, 0, 0, 0, 0, 0, 0
				}

      },
      AWBGain:{
        // Strobe
        {
          1016,    // i4R
          512,    // i4G
          1101    // i4B
        },
        // A
        {
          654,    // i4R
          512,    // i4G
          1423    // i4B
        },
        // TL84
        {
          878,    // i4R
          512,    // i4G
          1095    // i4B
        },
        // CWF
        {
          974,    // i4R
          512,    // i4G
          1192    // i4B
        },
        // D65
        {
          1179,    // i4R
          512,    // i4G
          760    // i4B
        },
        // Reserved 1
        {
          512,    // i4R
          512,    // i4G
          512    // i4B
        },
        // Reserved 2
        {
          512,    // i4R
          512,    // i4G
          512    // i4B
        },
        // Reserved 3
        {
          512,    // i4R
          512,    // i4G
          512    // i4B
        }
      },
      Weight:{
        1, // Strobe
        1, // A
        1, // TL84
        1, // CWF
        1, // D65
        1, // Reserved 1
        1, // Reserved 2
        1  // Reserved 3
      }
    },

          MDPMulitCCM:{
                Poly22:{
						92125,      // i4R_AVG
						21795,      // i4R_STD
						111750,      // i4B_AVG
						27519,      // i4B_STD
						4095,      // i4R_MAX
						512,      // i4R_MIN
						4095,      // i4G_MAX
						512,      // i4G_MIN
						4095,      // i4B_MAX
						512,      // i4B_MIN
                  {  // i4P00[9]
						11060000, -5900000, -40000, -835000, 6547500, -582500, 805000, -5075000, 9382500
                  },
                  {  // i4P10[9]
						4149415, -5006971, 853426, 290124, -1178921, 888797, 528354, 465601, -1016579
                  },
                  {  // i4P01[9]
						3203607, -3835507, 636054, -94841, -1125427, 1220267, 371341, -129455, -268626
                  },
                  {  // i4P20[9]
						0, 0, 0, 0, 0, 0, 0, 0, 0
                  },
                  { // i4P11[9]
						0, 0, 0, 0, 0, 0, 0, 0, 0
                  },
                  { // i4P02[9]
						0, 0, 0, 0, 0, 0, 0, 0, 0
                  }

                },
                AWBGain:{
                  // Strobe
                  {
						1016,    // i4R
						512,    // i4G
						1101    // i4B
                  },
                  // A
                  {
						654,    // i4R
						512,    // i4G
						1423    // i4B
                  },
                  // TL84
                  {
						878,    // i4R
						512,    // i4G
						1095    // i4B
                  },
                  // CWF
                  {
						974,    // i4R
						512,    // i4G
						1192    // i4B
                  },
                  // D65
                  {
						1179,    // i4R
						512,    // i4G
						760    // i4B
                  },
                  // Reserved 1
                  {
						512,    // i4R
						512,    // i4G
						512    // i4B
                  },
                  // Reserved 2
                  {
						512,    // i4R
						512,    // i4G
						512    // i4B
                  },
                  // Reserved 3
                  {
						512,    // i4R
						512,    // i4G
						512    // i4B
                  }
                },
                Weight:{
                  1, // Strobe
                  1, // A
                  1, // TL84
                  1, // CWF
                  1, // D65
                  1, // Reserved 1
                  1, // Reserved 2
                  1  // Reserved 3
                }
              },
    isp_ccm_ratio: 1.0,
    //bInvokeSmoothCCM
    bInvokeSmoothCCM: MTRUE,
    DngMetadata:
    {
        0,  //i4RefereceIlluminant1
        3,  //i4RefereceIlluminant2
        {
            // rNoiseProfile[4]
            {
                {0.000004, 0.000336},
                {0.000000, -0.000234},
            },
            {
                {0.000004, 0.000336},
                {0.000000, -0.000234},
            },
            {
                {0.000004, 0.000336},
                {0.000000, -0.000234},
            },
            {
                {0.000004, 0.000336},
                {0.000000, -0.000234},
            },
        },
    },
    ANR_TBL:
    {0}
}};

const NVRAM_CAMERA_3A_STRUCT CAMERA_3A_NVRAM_DEFAULT_VALUE =
{
    NVRAM_CAMERA_3A_FILE_VERSION, // u4Version
    SENSOR_ID, // SensorId

    // AE NVRAM
    {
        getAENVRAM_gc8c34mipiraw<CAM_SCENARIO_CAPTURE>(),
        getAENVRAM_gc8c34mipiraw<CAM_SCENARIO_CAPTURE>(),
        getAENVRAM_gc8c34mipiraw<CAM_SCENARIO_CAPTURE>(),
        getAENVRAM_gc8c34mipiraw<CAM_SCENARIO_CAPTURE>(),
        getAENVRAM_gc8c34mipiraw<CAM_SCENARIO_CAPTURE>(),
        getAENVRAM_gc8c34mipiraw<CAM_SCENARIO_CAPTURE>(),
        getAENVRAM_gc8c34mipiraw<CAM_SCENARIO_CAPTURE>()
    },

        // AWB NVRAM
    AWB_NVRAM_START
    {
        getAWBNVRAM_gc8c34mipiraw<CAM_SCENARIO_CAPTURE>(),
        getAWBNVRAM_gc8c34mipiraw<CAM_SCENARIO_CAPTURE>(),
        getAWBNVRAM_gc8c34mipiraw<CAM_SCENARIO_CAPTURE>(),
        getAWBNVRAM_gc8c34mipiraw<CAM_SCENARIO_CAPTURE>(),
        getAWBNVRAM_gc8c34mipiraw<CAM_SCENARIO_CAPTURE>(),
        getAWBNVRAM_gc8c34mipiraw<CAM_SCENARIO_CAPTURE>(),
        getAWBNVRAM_gc8c34mipiraw<CAM_SCENARIO_CAPTURE>()
    },

    // Flash AWB NVRAM
    {
#include INCLUDE_FILENAME_FLASH_AWB_PARA
    },

    {0}
};

#include INCLUDE_FILENAME_ISP_LSC_PARAM
//};  //  namespace

const CAMERA_TSF_TBL_STRUCT CAMERA_TSF_DEFAULT_VALUE =
{
    {
        1,  // isTsfEn
        2,  // tsfCtIdx
        {20, 2000, -110, -110, 512, 512, 512, 0}    // rAWBInput[8]
    },

#include INCLUDE_FILENAME_TSF_PARA
#include INCLUDE_FILENAME_TSF_DATA
};

const NVRAM_CAMERA_FEATURE_STRUCT CAMERA_FEATURE_DEFAULT_VALUE =
{
#include INCLUDE_FILENAME_FEATURE_PARA
};

typedef NSFeature::RAWSensorInfo<SENSOR_ID> SensorInfoSingleton_T;


namespace NSFeature {
  template <>
  UINT32
  SensorInfoSingleton_T::
  impGetDefaultData(CAMERA_DATA_TYPE_ENUM const CameraDataType, VOID*const pDataBuf, UINT32 const size) const
  {
    UINT32 dataSize[CAMERA_DATA_TYPE_NUM] = {sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
        sizeof(NVRAM_CAMERA_3A_STRUCT),
        sizeof(NVRAM_CAMERA_SHADING_STRUCT),
        sizeof(NVRAM_LENS_PARA_STRUCT),
        sizeof(AE_PLINETABLE_T),
        0,
        sizeof(CAMERA_TSF_TBL_STRUCT),
        0,
        0,
        sizeof(NVRAM_CAMERA_FEATURE_STRUCT)
    };

    if (CameraDataType > CAMERA_NVRAM_DATA_FEATURE || NULL == pDataBuf || (size != dataSize[CameraDataType]))
    {
      return 1;
    }

    switch(CameraDataType)
    {
      case CAMERA_NVRAM_DATA_ISP:
        memcpy(pDataBuf,&CAMERA_ISP_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
        break;
      case CAMERA_NVRAM_DATA_3A:
        memcpy(pDataBuf,&CAMERA_3A_NVRAM_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_3A_STRUCT));
        break;
      case CAMERA_NVRAM_DATA_SHADING:
        memcpy(pDataBuf,&CAMERA_SHADING_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_SHADING_STRUCT));
        break;
      case CAMERA_DATA_AE_PLINETABLE:
        memcpy(pDataBuf,&g_PlineTableMapping,sizeof(AE_PLINETABLE_T));
        break;
      case CAMERA_DATA_TSF_TABLE:
        memcpy(pDataBuf,&CAMERA_TSF_DEFAULT_VALUE,sizeof(CAMERA_TSF_TBL_STRUCT));
        break;
      case CAMERA_NVRAM_DATA_FEATURE:
        memcpy(pDataBuf,&CAMERA_FEATURE_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_FEATURE_STRUCT));
        break;
      default:
        break;
    }
    return 0;
  }};  //  NSFeature


