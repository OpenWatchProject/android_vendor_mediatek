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
#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_s5k5e8yxmipiraw.h"
#include "camera_info_s5k5e8yxmipiraw.h"
#include "camera_custom_AEPlinetable.h"
#include "camera_custom_tsf_tbl.h"
#include "AE_Tuning_Para/camera_ae_tuning_para_s5k5e8yxmipiraw.h"
#include "AWB_Tuning_Para/camera_awb_tuning_para_s5k5e8yxmipiraw.h"


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
    .Version= NVRAM_CAMERA_PARA_FILE_VERSION,

    //SensorId
    .SensorId= SENSOR_ID,
    .ISPComm={
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      }
    },
    .ISPPca= {
#include INCLUDE_FILENAME_ISP_PCA_PARAM
    },
    .ISPRegs={
#include INCLUDE_FILENAME_ISP_REGS_PARAM
    },
    .ISPMfbMixer={{
      0x01FF0001, // MIX3_CTRL_0
      0x00FF0000, // MIX3_CTRL_1
      0xFFFF0000  // MIX3_SPARE
    }},
    .ISPMulitCCM={
        .CCM_Method= 2,  //0:dynamic CCM, 1:SCCM(v1.0), 2:SCCM(v2.0)

        .dynamic_CCM=
        {
            {.set={//CT_D65
                0x1F0C02F4, 0x00000000, 0x02491FE0, 0x00001FD7, 0x1F26001B, 0x000002BF
            }},
            {.set={//CT_TL84
                0x1F040306, 0x00001FF6, 0x02421FB3, 0x0000000B, 0x1F430006, 0x000002B7
            }},
            {.set={//CT_CWF
                0x1E2603B6, 0x00000024, 0x021E1FAD, 0x00000035, 0x1F07001C, 0x000002DD
            }},
            {.set={//CT_A
                0x1F7B02B3, 0x00001FD2, 0x022B1FB2, 0x00000023, 0x1E891FFB, 0x0000037C
            }}
        },

        .dynamic_CCM_AWBGain=
        {
            {//CT_D65
              512,    // i4R
              512,    // i4G
              512     // i4B
            },
            {//CT_TL84
              638,    // i4R
              512,    // i4G
              1457    // i4B
            },
            {//CT_CWF
              930,    // i4R
              512,    // i4G
              1138    // i4B
            },
            {//CT_A
              1052,   // i4R
              512,    // i4G
              1319    // i4B
            }
        },

        .PQ_Type= 0, //0:sRGB, 1:AdobeRGB, 2:P3

        .CCM_LV_valid_NUM = 1,

        .CCM_FLASH_valid_NUM = 1,

        .CCM_CT_valid_NUM = 5,

        .CCM=
        {
            {//LV_00
                {.set={//CT_00
                    0x1F6602D7, 0x00001FC3, 0x02071F98, 0x00000061, 0x1DE41FB8, 0x00000464
                }},
                {.set={//CT_01
                    0x1F7B02B3, 0x00001FD2, 0x022B1FB2, 0x00000023, 0x1E891FFB, 0x0000037C
                }},
                {.set={//CT_02
                    0x1F040306, 0x00001FF6, 0x02421FB3, 0x0000000B, 0x1F430006, 0x000002B7
                }},
                {.set={//CT_03
                    0x1E2603B6, 0x00000024, 0x021E1FAD, 0x00000035, 0x1F07001C, 0x000002DD
                }},
                {.set={//CT_04
                    0x1F0C02F4, 0x00000000, 0x02491FE0, 0x00001FD7, 0x1F26001B, 0x000002BF
                }},
                {.set={//CT_05
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_06
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_07
    				0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }}
            },
            {//LV_01
                {.set={//CT_00
    				0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_01
    				0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_02
    				0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_03
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_04
             		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_05
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_06
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_07
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }}
            },
            {//LV_02
                {.set={//CT_00
    				0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_01
    				0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_02
    				0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_03
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_04
             		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_05
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_06
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_07
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }}
            },
            {//LV_03
                {.set={//CT_00
    				0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_01
    				0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_02
    				0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_03
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_04
             		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_05
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_06
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_07
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }}
            },
            {//LV_04
                {.set={//CT_00
    				0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_01
    				0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_02
    				0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_03
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_04
             		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_05
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_06
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_07
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }}
            },
            {//LV_05
                {.set={//CT_00
    				0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_01
    				0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_02
    				0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_03
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_04
             		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_05
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_06
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_07
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }}
            },
            {//FLASH_00
                {.set={//CT_00
                    0x1F6602D7, 0x00001FC3, 0x02071F98, 0x00000061, 0x1DE41FB8, 0x00000464
                }},
                {.set={//CT_01
                    0x1F7B02B3, 0x00001FD2, 0x022B1FB2, 0x00000023, 0x1E891FFB, 0x0000037C
                }},
                {.set={//CT_02
                    0x1F040306, 0x00001FF6, 0x02421FB3, 0x0000000B, 0x1F430006, 0x000002B7
                }},
                {.set={//CT_03
                    0x1E2603B6, 0x00000024, 0x021E1FAD, 0x00000035, 0x1F07001C, 0x000002DD
                }},
                {.set={//CT_04
                    0x1F0C02F4, 0x00000000, 0x02491FE0, 0x00001FD7, 0x1F26001B, 0x000002BF
                }},
                {.set={//CT_05
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_06
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_07
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }}
            },
            {//FLASH_01
                {.set={//CT_00
    				0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_01
    				0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_02
    				0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_03
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_04
             		0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_05
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_06
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }},
                {.set={//CT_07
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
                }}
            }
        },

        .AWBGain=
        {
            {//LV_00
                {//CT_00
                  539,    // i4R
                  512,    // i4G
                 1741     // i4B
                },
                {//CT_01
                  671,    // i4R
                  512,    // i4G
                  1410    // i4B
                },
                {//CT_02
                  925,    // i4R
                  512,    // i4G
                 1082     // i4B
                },
                {//CT_03
                 1077,    // i4R
                  512,    // i4G
                 1242     // i4B
                },
                {//CT_04
                 1257,   // i4R
                  512,    // i4G
                  707     // i4B
                },
                {//CT_05
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_06
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_07
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                }
            },
            {//LV_01
                {//CT_00
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_01
                  638,    // i4R
                  512,    // i4G
                  1457    // i4B
                },
                {//CT_02
                  930,    // i4R
                  512,    // i4G
                  1138    // i4B
                },
                {//CT_03
                  1052,   // i4R
                  512,    // i4G
                  1319    // i4B
                },
                {//CT_04
                  1233,   // i4R
                  512,    // i4G
                  787     // i4B
                },
                {//CT_05
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_06
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_07
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                }
            },
            {//LV_02
                {//CT_00
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_01
                  638,    // i4R
                  512,    // i4G
                  1457    // i4B
                },
                {//CT_02
                  930,    // i4R
                  512,    // i4G
                  1138    // i4B
                },
                {//CT_03
                  1052,   // i4R
                  512,    // i4G
                  1319    // i4B
                },
                {//CT_04
                  1233,   // i4R
                  512,    // i4G
                  787     // i4B
                },
                {//CT_05
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_06
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_07
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                }
            },
            {//LV_03
                {//CT_00
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_01
                  638,    // i4R
                  512,    // i4G
                  1457    // i4B
                },
                {//CT_02
                  930,    // i4R
                  512,    // i4G
                  1138    // i4B
                },
                {//CT_03
                  1052,   // i4R
                  512,    // i4G
                  1319    // i4B
                },
                {//CT_04
                  1233,   // i4R
                  512,    // i4G
                  787     // i4B
                },
                {//CT_05
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_06
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_07
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                }
            },
            {//LV_04
                {//CT_00
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_01
                  638,    // i4R
                  512,    // i4G
                  1457    // i4B
                },
                {//CT_02
                  930,    // i4R
                  512,    // i4G
                  1138    // i4B
                },
                {//CT_03
                  1052,   // i4R
                  512,    // i4G
                  1319    // i4B
                },
                {//CT_04
                  1233,   // i4R
                  512,    // i4G
                  787     // i4B
                },
                {//CT_05
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_06
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_07
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                }
            },
            {//LV_05
                {//CT_00
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_01
                  638,    // i4R
                  512,    // i4G
                  1457    // i4B
                },
                {//CT_02
                  930,    // i4R
                  512,    // i4G
                  1138    // i4B
                },
                {//CT_03
                  1052,   // i4R
                  512,    // i4G
                  1319    // i4B
                },
                {//CT_04
                  1233,   // i4R
                  512,    // i4G
                  787     // i4B
                },
                {//CT_05
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_06
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_07
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                }
            },
            {//FLASH_00
                {//CT_00
                  539,    // i4R
                  512,    // i4G
                 1741     // i4B
                },
                {//CT_01
                  671,    // i4R
                  512,    // i4G
                  1410    // i4B
                },
                {//CT_02
                  925,    // i4R
                  512,    // i4G
                 1082     // i4B
                },
                {//CT_03
                 1077,    // i4R
                  512,    // i4G
                 1242     // i4B
                },
                {//CT_04
                 1257,   // i4R
                  512,    // i4G
                  707     // i4B
                },
                {//CT_05
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_06
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_07
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                }
            },
            {//FLASH_01
                {//CT_00
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_01
                  638,    // i4R
                  512,    // i4G
                  1457    // i4B
                },
                {//CT_02
                  930,    // i4R
                  512,    // i4G
                  1138    // i4B
                },
                {//CT_03
                  1052,   // i4R
                  512,    // i4G
                  1319    // i4B
                },
                {//CT_04
                  1233,   // i4R
                  512,    // i4G
                  787     // i4B
                },
                {//CT_05
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_06
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                },
                {//CT_07
                  512,    // i4R
                  512,    // i4G
                  512     // i4B
                }
            }
        },

       .Weight=
       {
            {//LV_00
                1, 1, 1, 1, 1, 1, 1, 1
            },
            {//LV_01
                1, 1, 1, 1, 1, 1, 1, 1
            },
            {//LV_02
                1, 1, 1, 1, 1, 1, 1, 1
            },
            {//LV_03
                1, 1, 1, 1, 1, 1, 1, 1
            },
            {//LV_04
                1, 1, 1, 1, 1, 1, 1, 1
            },
            {//LV_05
                1, 1, 1, 1, 1, 1, 1, 1
            },
            {//FLASH_00
                1, 1, 1, 1, 1, 1, 1, 1
            },
            {//FLASH_01
                1, 1, 1, 1, 1, 1, 1, 1
            }
        },
        .Poly22=
        {
            {//LV_00
                  75733, // i4R_AVG
     			  19321, // i4R_STD
    			 118017, // i4B_AVG
     			  31014, // i4B_STD
      			   1145, // i4R_MAX
       				441, // i4R_MIN
       				771, // i4G_MAX
       				573, // i4G_MIN
      			   1026, // i4B_MAX
       				798, // i4B_MIN
                { // i4P00[9]
                    7478333,  -2335000,    -25000,  -1496667,   6813333,   -200000,    206667,  -4016667,   8931667
                },
                { // i4P10[9]
                    1202861,  -2905079,   1674856,   -767934,   1171102,   -388928,    506160,   -950527,    451067
                },
                { // i4P01[9]
                    -687844,   -720764,   1386570,   -957686,    573290,    396309,    493071,  -1607068,   1117747
                },
                { // i4P20[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P11[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P02[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                }

            },
            {//LV_01
                 96325, // i4R_AVG
                 25002, // i4R_STD
                117525, // i4B_AVG
                 28993, // i4B_STD
    			  1257, // i4R_MAX
    			   851, // i4R_MIN
    			   703, // i4G_MAX
    			   652, // i4G_MIN
    			  1145, // i4B_MAX
    			   783, // i4B_MIN
                { // i4P00[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P10[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P01[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P20[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P11[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P02[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                }

             },
             {//LV_02
                 96325, // i4R_AVG
                 25002, // i4R_STD
                117525, // i4B_AVG
                 28993, // i4B_STD
    			  1257, // i4R_MAX
    			   851, // i4R_MIN
    			   703, // i4G_MAX
    			   652, // i4G_MIN
    			  1145, // i4B_MAX
    			   783, // i4B_MIN
                { // i4P00[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P10[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P01[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P20[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P11[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P02[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                }

              },
              {//LV_03
                 96325, // i4R_AVG
                 25002, // i4R_STD
                117525, // i4B_AVG
                 28993, // i4B_STD
    			  1257, // i4R_MAX
    			   851, // i4R_MIN
    			   703, // i4G_MAX
    			   652, // i4G_MIN
    			  1145, // i4B_MAX
    			   783, // i4B_MIN
                { // i4P00[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P10[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P01[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P20[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P11[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P02[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                }

              },
              {//LV_04
                 96325, // i4R_AVG
                 25002, // i4R_STD
                117525, // i4B_AVG
                 28993, // i4B_STD
    			  1257, // i4R_MAX
    			   851, // i4R_MIN
    			   703, // i4G_MAX
    			   652, // i4G_MIN
    			  1145, // i4B_MAX
    			   783, // i4B_MIN
                { // i4P00[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P10[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P01[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P20[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P11[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P02[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                }

              },
              {//LV05
                 96325, // i4R_AVG
                 25002, // i4R_STD
                117525, // i4B_AVG
                 28993, // i4B_STD
    			  1257, // i4R_MAX
    			   851, // i4R_MIN
    			   703, // i4G_MAX
    			   652, // i4G_MIN
    			  1145, // i4B_MAX
    			   783, // i4B_MIN
                { // i4P00[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P10[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P01[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P20[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P11[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P02[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                }

              },
              {//FLASH_00
                 96325, // i4R_AVG
                 25002, // i4R_STD
                117525, // i4B_AVG
                 28993, // i4B_STD
    			  1257, // i4R_MAX
    			   851, // i4R_MIN
    			   703, // i4G_MAX
    			   652, // i4G_MIN
    			  1145, // i4B_MAX
    			   783, // i4B_MIN
                { // i4P00[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P10[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P01[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P20[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P11[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P02[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                }
              },
              {//FLASH_01
                 96325, // i4R_AVG
                 25002, // i4R_STD
                117525, // i4B_AVG
                 28993, // i4B_STD
    			  1257, // i4R_MAX
    			   851, // i4R_MIN
    			   703, // i4G_MAX
    			   652, // i4G_MIN
    			  1145, // i4B_MAX
    			   783, // i4B_MIN
                { // i4P00[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P10[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P01[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P20[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P11[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                },
                { // i4P02[9]
                    0, 0, 0, 0, 0, 0, 0, 0, 0
                }

              }
          }
    },
    .DngMetadata=
    {
        0,  //i4RefereceIlluminant1
        3,  //i4RefereceIlluminant2
        {
            // rNoiseProfile[4]
            {
                {3.530980e-6, 3.357493e-4},
                {8.098436e-8, -2.336529e-4},
            },
            {
                {3.530980e-6, 3.357493e-4},
                {8.098436e-8, -2.336529e-4},
            },
            {
                {3.530980e-6, 3.357493e-4},
                {8.098436e-8, -2.336529e-4},
            },
            {
                {3.530980e-6, 3.357493e-4},
                {8.098436e-8, -2.336529e-4},
            },
        },
    },
    .ANR_TBL=
    {
      //CT_00
      {
        //ISO_00
        {.set={
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
          0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
          0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
          0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
          0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
          0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
          0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
          0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }},
            //ISO_01
        {.set={
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
          0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
          0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
          0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
          0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
          0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
          0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
          0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }},
            //ISO_02
        {.set={
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
          0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
          0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
          0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
          0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
          0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
          0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
          0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }},
            //ISO_03
        {.set={
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
          0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
          0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
          0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
          0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
          0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
          0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
          0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }},
            //ISO_04
        {.set={
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
          0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
          0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
          0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
          0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
          0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
          0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
          0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }},
            //ISO_05
        {.set={
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
          0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
          0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
          0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
          0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
          0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
          0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
          0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }}
        },
        //CT_01
      {
        //ISO_00
        {.set={
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
          0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
          0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
          0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
          0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
          0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
          0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
          0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }},
            //ISO_01
        {.set={
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
          0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
          0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
          0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
          0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
          0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
          0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
          0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }},
            //ISO_02
        {.set={
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
          0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
          0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
          0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
          0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
          0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
          0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
          0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }},
            //ISO_03
        {.set={
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
          0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
          0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
          0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
          0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
          0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
          0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
          0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }},
            //ISO_04
        {.set={
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
          0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
          0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
          0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
          0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
          0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
          0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
          0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }},
            //ISO_05
        {.set={
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04490447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445,
          0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444,
          0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444, 0x04440444, 0x04440444,
          0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04450445, 0x04440444,
          0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446, 0x04440444, 0x04440444,
          0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04450445, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444, 0x044e0449, 0x04440446,
          0x04440444, 0x04440444, 0x04490446, 0x04440445, 0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04450444, 0x04440444, 0x04440444, 0x04440444, 0x044a0447, 0x04440445, 0x04440444, 0x04440444,
          0x044d0448, 0x04440446, 0x04440444, 0x04440444, 0x04480446, 0x04440445, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04490446, 0x04440445,
          0x04440444, 0x04440444, 0x044c0447, 0x04440445, 0x04440444, 0x04440444, 0x04470445, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04440444,
          0x04440444, 0x04440444, 0x04440444, 0x04440444, 0x04430443, 0x04430443, 0x04440443, 0x04430443, 0x04430443, 0x04430443,
          0x04470445, 0x04430444, 0x04430443, 0x04430443, 0x04490446, 0x04430444, 0x04430443, 0x04430443, 0x04460444, 0x04430444,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443,
          0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443, 0x04430443
        }}
        }
    }
}};

const NVRAM_CAMERA_3A_STRUCT CAMERA_3A_NVRAM_DEFAULT_VALUE =
{
    NVRAM_CAMERA_3A_FILE_VERSION, // u4Version
    SENSOR_ID, // SensorId

    // AE NVRAM
    {
        getAENVRAM_s5k5e8yxmipiraw<CAM_SCENARIO_PREVIEW>(),
        getAENVRAM_s5k5e8yxmipiraw<CAM_SCENARIO_VIDEO>(),
        getAENVRAM_s5k5e8yxmipiraw<CAM_SCENARIO_CAPTURE>(),
        getAENVRAM_s5k5e8yxmipiraw<CAM_SCENARIO_CUSTOM1>(),
        getAENVRAM_s5k5e8yxmipiraw<CAM_SCENARIO_CUSTOM2>(),
        getAENVRAM_s5k5e8yxmipiraw<CAM_SCENARIO_CUSTOM3>(),
        getAENVRAM_s5k5e8yxmipiraw<CAM_SCENARIO_CUSTOM4>()
    },

    // AWB NVRAM
    AWB_NVRAM_START
    {
        getAWBNVRAM_s5k5e8yxmipiraw<CAM_SCENARIO_PREVIEW>(),
        getAWBNVRAM_s5k5e8yxmipiraw<CAM_SCENARIO_VIDEO>(),
        getAWBNVRAM_s5k5e8yxmipiraw<CAM_SCENARIO_CAPTURE>(),
        getAWBNVRAM_s5k5e8yxmipiraw<CAM_SCENARIO_CUSTOM1>(),
        getAWBNVRAM_s5k5e8yxmipiraw<CAM_SCENARIO_CUSTOM2>(),
        getAWBNVRAM_s5k5e8yxmipiraw<CAM_SCENARIO_CUSTOM3>(),
        getAWBNVRAM_s5k5e8yxmipiraw<CAM_SCENARIO_CUSTOM4>()
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

#include INCLUDE_FILENAME_BPCI_PARA

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
        sizeof(CAMERA_BPCI_STRUCT),
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
      case CAMERA_DATA_PDC_TABLE:
        memcpy(pDataBuf,&CAMERA_BPCI_DEFAULT_VALUE, sizeof(CAMERA_BPCI_STRUCT));
        break;
      case CAMERA_NVRAM_DATA_FEATURE:
        memcpy(pDataBuf,&CAMERA_FEATURE_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_FEATURE_STRUCT));
        break;
      default:
        break;
    }
    return 0;
  }};  //  NSFeature


