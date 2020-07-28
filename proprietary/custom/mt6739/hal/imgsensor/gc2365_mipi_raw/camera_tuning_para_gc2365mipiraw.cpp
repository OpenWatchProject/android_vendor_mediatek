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
#include "camera_AE_PLineTable_gc2365mipiraw.h"
#include "camera_info_gc2365mipiraw.h"
#include "camera_custom_AEPlinetable.h"
#include "camera_custom_tsf_tbl.h"
#include "AE_Tuning_Para/camera_ae_tuning_para_gc2365mipiraw.h"

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
        getAENVRAM_gc2365mipiraw<CAM_SCENARIO_PREVIEW>(),
        getAENVRAM_gc2365mipiraw<CAM_SCENARIO_VIDEO>(),
        getAENVRAM_gc2365mipiraw<CAM_SCENARIO_CAPTURE>(),
        getAENVRAM_gc2365mipiraw<CAM_SCENARIO_CUSTOM1>(),
        getAENVRAM_gc2365mipiraw<CAM_SCENARIO_CUSTOM2>(),
        getAENVRAM_gc2365mipiraw<CAM_SCENARIO_CUSTOM3>(),
        getAENVRAM_gc2365mipiraw<CAM_SCENARIO_CUSTOM4>()
    },

    // AWB NVRAM
    AWB_NVRAM_START
    {
        {
            {
                // AWB calibration data
                {
                    // rUnitGain (unit gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0    // i4B
                    },
                    // rGoldenGain (golden sample gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0    // i4B
                    },
                    // rUnitGain TL84 (unit gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0    // i4B
                    },
                    // rGoldenGain TL84 (golden sample gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0    // i4B
                    },
                     // rUnitGain Alight (unit gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0    // i4B
                    },
                    // rGoldenGain Alight (golden sample gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0    // i4B
                    },
                    // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0    // i4B
                    },
                    // rD65Gain (D65 WB gain: 1.0 = 512)
                    {
                            1088, // D65Gain_R
                            512, // D65Gain_G
                            689 // D65Gain_B
                    }
                },
                // Original XY coordinate of AWB light source
                {
                    // Strobe
                    {
                        0,    // i4X
                        0    // i4Y
                    },
                    // Horizon
                    {
                        -382, // OriX_Hor
                        -365 // OriY_Hor
                    },
                    // A
                    {
                        -258, // OriX_A
                        -386 // OriY_A
                    },
                    // TL84
                    {
                        -69, // OriX_TL84
                        -429 // OriY_TL84
                    },
                    // CWF
                    {
                        -69, // OriX_CWF
                        -485 // OriY_CWF
                    },
                    // DNP
                    {
                        60, // OriX_DNP
                        -449 // OriY_DNP
                    },
                    // D65
                    {
                        169, // OriX_D65
                        -388 // OriY_D65
                    },
                    // DF
                    {
                        0, // OriX_DF
                        0 // OriY_DF
                    }
                },
                // Rotated XY coordinate of AWB light source
                {
                    // Strobe
                    {
                        0,    // i4X
                        0    // i4Y
                    },
                    // Horizon
                    {
                        -382, // RotX_Hor
                        -365 // RotY_Hor
                    },
                    // A
                    {
                        -258, // RotX_A
                        -386 // RotY_A
                    },
                    // TL84
                    {
                        -69, // RotX_TL84
                        -429 // RotY_TL84
                    },
                    // CWF
                    {
                        -69, // RotX_CWF
                        -485 // RotY_CWF
                    },
                    // DNP
                    {
                        60, // RotX_DNP
                        -449 // RotY_DNP
                    },
                    // D65
                    {
                        169, // RotX_D65
                        -388 // RotY_D65
                    },
                    // DF
                    {
                        161, // RotX_DF
                        -473 // RotY_DF
                    }
                },
                // AWB gain of AWB light source
                {
                    // Strobe
                    {
                        810,    // i4R
                        512,    // i4G
                        677    // i4B
                    },
                    // Horizon
                    {
                        512, // AWBGAIN_HOR_R
                        524, // AWBGAIN_HOR_G
                        1441 // AWBGAIN_HOR_B
                    },
                    // A
                    {
                        609, // AWBGAIN_A_R
                        512, // AWBGAIN_A_G
                        1225 // AWBGAIN_A_B
                    },
                    // TL84
                    {
                        833, // AWBGAIN_TL84_R
                        512, // AWBGAIN_TL84_G
                        1004 // AWBGAIN_TL84_B
                    },
                    // CWF
                    {
                        900, // AWBGAIN_CWF_R
                        512, // AWBGAIN_CWF_G
                        1084 // AWBGAIN_CWF_B
                    },
                    // DNP
                    {
                        1020, // AWBGAIN_DNP_R
                        512, // AWBGAIN_DNP_G
                        866 // AWBGAIN_DNP_B
                    },
                    // D65
                    {
                        1088, // AWBGAIN_D65_R
                        512, // AWBGAIN_D65_G
                        689 // AWBGAIN_D65_B
                    },
                    // DF
                    {
                        512, // AWBGAIN_DF_R
                        512, // AWBGAIN_DF_G
                        512 // AWBGAIN_DF_B
                    }
                },
                // Rotation matrix parameter
                {
                    0, // RotationAngle
                    256, // Cos
                    0 // Sin
                },
                // Daylight locus parameter
                {
                    -125, //126,    // i4SlopeNumerator
                    128    // i4SlopeDenominator
                },
	            // Predictor gain
                {
                    141, // i4PrefRatio100
                    // DaylightLocus_L
                    {
                        1058, //1058, //1088, // i4R
                        530, //512, //512, // i4G
                        709 //689 //689 // i4B
                    },
                    // DaylightLocus_H
                    {
                        854, //839, // i4R
                        512, // i4G
                        878 //893, // i4B
                    },
                    // Temporal General
                    {
                        1088, // i4R
                        512, // i4G
                        689 // i4B
                    },
                    // AWB LSC Gain
                    {
                        854, //839, // i4R
                        512, // i4G
                        878 //893, // i4B
                    }
                },
                // AWB light area
                {
                    // Strobe:FIXME
                    {
                        -100, // i4RightBound
                        -250, // i4LeftBound
                        -361, // i4UpperBound
                        -600  // i4LowerBound
                    },
                    // Tungsten
                    {
                        -145, // TungRightBound
                        -782, // TungLeftBound
                        -310, //-330, // TungUpperBound
                        -420 //-408 // TungLowerBound
                    },
                    // Warm fluorescent
                    {
                        -145, // WFluoRightBound
                        -782, // WFluoLeftBound
                        -420, //-408, // WFluoUpperBound
                        -515 //-485 // WFluoLowerBound
                    },
                    // Fluorescent
                    {
                        12, // FluoRightBound
                        -145, //-164, // FluoLeftBound
                        -328, //-348, // FluoUpperBound
                            -430 //-454 //-444 // FluoLowerBound
                    },
                    // CWF
                    {
                        12, //23, //-10, // CWFRightBound
                        -145, //-164, // CWFLeftBound
                            -430, //-454, //-444, // CWFUpperBound
                        -540 //-535// CWFLowerBound
                    },
                    // Daylight
                    {
                        199, // DayRightBound
                        12, // DayLeftBound
                        -328, //-348, // DayUpperBound
                        -450 //-430 //-454 //-444 //-464 // DayLowerBound
                    },
                    // Shade
                    {
                        529, // ShadeRightBound
                        199, // ShadeLeftBound
                        -328, //-348, // ShadeUpperBound
                        -427 // ShadeLowerBound
                    },
                    // Daylight Fluorescent
                    {
                        199, // DFRightBound
                        12, //23, //-10, // DFLeftBound
                        -450, //-430, //-454, //-444, // DFUpperBound
                        -500 //-535 //-530 // DFLowerBound
                    }
                },
                // PWB light area
                {
                    // Reference area
                    {
                        529, // PRefRightBound
                        -782, // PRefLeftBound
                        -305, // PRefUpperBound
                        -530 // PRefLowerBound
                    },
                    // Daylight
                    {
                        224, // PDayRightBound
                        28, // PDayLeftBound
                        -348, // PDayUpperBound
                        -464 // PDayLowerBound
                    },
                    // Cloudy daylight
                    {
                        324, // PCloudyRightBound
                        149, // PCloudyLeftBound
                        -348, // PCloudyUpperBound
                        -464 // PCloudyLowerBound
                    },
                    // Shade
                    {
                        424, // PShadeRightBound
                        149, // PShadeLeftBound
                        -348, // PShadeUpperBound
                        -464 // PShadeLowerBound
                    },
                    // Twilight
                    {
                        28, // PTwiRightBound
                        -132, // PTwiLeftBound
                        -348, // PTwiUpperBound
                        -464 // PTwiLowerBound
                    },
                    // Fluorescent
                    {
                        219, // PFluoRightBound
                        -169, // PFluoLeftBound
                        -338, // PFluoUpperBound
                        -535 // PFluoLowerBound
                    },
                    // Warm fluorescent
                    {
                        -158, // PWFluoRightBound
                        -358, // PWFluoLeftBound
                        -338, // PWFluoUpperBound
                        -535 // PWFluoLowerBound
                    },
                    // Incandescent
                    {
                        -158, // PIncaRightBound
                        -358, // PIncaLeftBound
                        -348, // PIncaUpperBound
                        -464 // PIncaLowerBound
                    },
                    // Gray World
                    {
                        5000, // PGWRightBound
                        -5000, // PGWLeftBound
                        5000, // PGWUpperBound
                        -5000 // PGWLowerBound
                    }
                },
                // PWB default gain
                {
                    // Daylight
                    {
                        1052, // PWB_Day_R
                        512, // PWB_Day_G
                        748 // PWB_Day_B
                    },
                    // Cloudy daylight
                    {
                        1222, // PWB_Cloudy_R
                        512, // PWB_Cloudy_G
                        644 // PWB_Cloudy_B
                    },
                    // Shade
                    {
                        1307, // PWB_Shade_R
                        512, // PWB_Shade_G
                        602 // PWB_Shade_B
                    },
                    // Twilight
                    {
                        827, // PWB_Twi_R
                        512, // PWB_Twi_G
                        952 // PWB_Twi_B
                    },
                    // Fluorescent
                    {
                        956, // PWB_Fluo_R
                        512, // PWB_Fluo_G
                        894 // PWB_Fluo_B
                    },
                    // Warm fluorescent
                    {
                        652, // PWB_WFluo_R
                        512, // PWB_WFluo_G
                        1311 // PWB_WFluo_B
                    },
                    // Incandescent
                    {
                        626, // PWB_Inca_R
                        512, // PWB_Inca_G
                        1258 // PWB_Inca_B
                    },
                    // Gray World
                    {
                        512, // PWB_GW_R
                        512, // PWB_GW_G
                        512 // PWB_GW_B
                    }
                },
                // AWB preference color
                {
                    // Tungsten
                    {
                        40, //50, // TUNG_SLIDER
                        4478 //3772 //4030//3837 //3537 // TUNG_OFFS
                    },
                    // Warm fluorescent
                    {
                        40, //50, //65, // WFluo_SLIDER
                        4478 //4272 //4272 //3972 //4030//3837 //3537 // WFluo_OFFS
                    },
                    // Shade
                    {
                        50, // Shade_SLIDER
                        909 // Shade_OFFS
                    },
                    // Sunset Area
                    {
                        92, //64,   // i4Sunset_BoundXr_Thr
                        -449 //-463     // i4Sunset_BoundYr_Thr
                    },
                    // Shade F Area
                    {

                        -145, //-135, //-82,   // i4BoundXrThr
                        -420 //-412 //-415    // i4BoundYrThr
                    },
                    // Shade F Vertex
                    {

                        -145, //-135, //-82,   // i4BoundXrThr
                        -420 //-412 //-415    // i4BoundYrThr
                    },
                    // Shade CWF Area
                    {
                        -145,//-109, //-89,   // i4BoundXrThr
                        -465//-475 //-509    // i4BoundYrThr
                    },
                    // Shade CWF Vertex
                    {
                        -145,//-109, //-89,   // i4BoundXrThr
                        -465//-475 //-509    // i4BoundYrThr
                    }
                },
                // CCT estimation
                {
	                // CCT
	                {
		                2300,	// i4CCT[0]
		                2850,	// i4CCT[1]
		                3750,	// i4CCT[2]
		                5100,	// i4CCT[3]
		                6500 	// i4CCT[4]
	                },
	                // Rotated X coordinate
	                {
		                -551,	// i4RotatedXCoordinate[0]
		                -427,	// i4RotatedXCoordinate[1]
		                -238,	// i4RotatedXCoordinate[2]
		                -109,	// i4RotatedXCoordinate[3]
		                0 	    // i4RotatedXCoordinate[4]
	                }
                }
            },

            // Algorithm Tuning Paramter
            {
                // AWB Backup Enable
                0,

                // Daylight locus offset LUTs for tungsten
                {
                    21, // i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 250,  500, 1333, 1667, 2313, 2625, 2938, 3250, 3563, 3875, 4188, 4500, 4813, 5125, 5438, 5750, 6063, 6375, 6688,  7000} // i4LUTOut
                },

                // Daylight locus offset LUTs for WF
                {
                    21, // i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 350,  850, 1250, 1500, 1767, 1950, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778,  5000} // i4LUTOut
                },

                // Daylight locus offset LUTs for shade
                {
                    21, // i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 500, 1000, 1500, 2500, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8000, 8000, 8500, 9000, 9500, 10000}  // i4LUTOut
                },
                // Preference gain for each light source
                {
                    //        LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                    //        LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
          	            {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
        	        }, // STROBE
        	        {
          	            {530, 530, 480}, {530, 530, 480}, {530, 530, 480}, {530, 530, 480}, {530, 530, 480}, {520, 520, 496}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
           	            {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
        	        }, // TUNGSTEN
        	        {
                        {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508},
           	            {512, 512, 508}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
        	        }, // WARM F
        	        {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                    }, // F
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // CWF
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                    }, // DAYLIGHT
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // SHADE
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    } // DAYLIGHT F
                },
                // Parent block weight parameter
                {
                    1,      // bEnable
                    6           // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
                },
                // AWB LV threshold for predictor
                {
                    115, //100,    // i4InitLVThr_L
                    155, //140,    // i4InitLVThr_H
                    100 //80      // i4EnqueueLVThr
                },
                // AWB number threshold for temporal predictor
                {
                        65,     // i4Neutral_ParentBlk_Thr
                    //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  50,  25,   2,   2,   2,   2,   2,   2,   2}  // (%) i4CWFDF_LUTThr
                },
                // AWB light neutral noise reduction for outdoor
                {
                    //LV0  1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                    // Non neutral
	                { 3,   3,   3,   3,   3,   3,   3,   3,    3,   3,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
	                // Flurescent
	                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
	                // CWF
	                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
	                // Daylight
	                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   2,   2,   2,   2,   2,   2,   2,   2},  // (%)
	                // DF
	                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                },
                // AWB feature detection
                {
                    // Sunset Prop
                    {
                        1,          // i4Enable
                        120,        // i4LVThr_L
                        130,        // i4LVThr_H
                        10,         // i4SunsetCountThr
                        0,          // i4SunsetCountRatio_L
                        171         // i4SunsetCountRatio_H
                    },

                    // Shade F Detection
                    {
                        1,          // i4Enable
                        0,//105,        // i4LVThr
                        1,//105,        // i4LVThr
                        96 //10         // i4DaylightProb
                    },

                    // Shade CWF Detection
                    {
                        1,          // i4Enable
                        0,//95,         // i4LVThr
                        1,//95,         // i4LVThr
                        208 //224 //128 //35         // i4DaylightProb
                    },
                },

                // AWB non-neutral probability for spatial and temporal weighting look-up table (Max: 100; Min: 0)
                {
                    //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                    {  90, 90,  90,  90,  90,  90,  90,  90,  90,  90,   90,  50,  30,  20,  10,   0,   0,   0,   0}
                },

                // AWB daylight locus probability look-up table (Max: 100; Min: 0)
                {
                    //LV0  1    2    3    4    5    6    7    8    9     10    11   12   13   14   15  16   17   18
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100,  50,  25,   0,  0,   0,   0}, // Strobe
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,   75,  50,  25,  25,  25,  0,   0,   0}, // Tungsten
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,   75,  50,  25,  25,  25,  0,   0,   0}, // Warm fluorescent
                    {100, 100, 100, 100, 100, 100, 100, 100, 100,  95,   95,   70,  50,  25,  25,  25,  0,   0,   0}, // Fluorescent
                    { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,   80,   55,  30,  30,  30,  30,  0,   0,   0}, // CWF
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100, 100,  75,  50, 40,  30,  20}, // Daylight
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100,  75,  50,  25,   0,  0,   0,   0}, // Shade
                    { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,   90,   90,  90,  90,  75,  50, 30,   0,   0}  // Daylight fluorescent
                },

                // AWB tuning information
                {
                    6739,       // project code
                    5588,       // model
                    20150624,   // date
                    0,          // reserved 0
                    1,          // reserved 1
                    2,          // reserved 2
                    3,          // reserved 3
                    4,          // reserved 4
                }
            }
        },
        {
            {
                // AWB calibration data
                {
                    // rUnitGain (unit gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0    // i4B
                    },
                    // rGoldenGain (golden sample gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0    // i4B
                    },
                    // rUnitGain TL84 (unit gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0    // i4B
                    },
                    // rGoldenGain TL84 (golden sample gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0    // i4B
                    },
                     // rUnitGain Alight (unit gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0    // i4B
                    },
                    // rGoldenGain Alight (golden sample gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0    // i4B
                    },
                    // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
                    {
                            0,    // i4R
                            0,    // i4G
                            0    // i4B
                    },
                    // rD65Gain (D65 WB gain: 1.0 = 512)
                    {
                            1088, // D65Gain_R
                            512, // D65Gain_G
                            689 // D65Gain_B
                    }
                },
                // Original XY coordinate of AWB light source
                {
                    // Strobe
                    {
                        0,    // i4X
                        0    // i4Y
                    },
                    // Horizon
                    {
                        -382, // OriX_Hor
                        -365 // OriY_Hor
                    },
                    // A
                    {
                        -258, // OriX_A
                        -386 // OriY_A
                    },
                    // TL84
                    {
                        -69, // OriX_TL84
                        -429 // OriY_TL84
                    },
                    // CWF
                    {
                        -69, // OriX_CWF
                        -485 // OriY_CWF
                    },
                    // DNP
                    {
                        60, // OriX_DNP
                        -449 // OriY_DNP
                    },
                    // D65
                    {
                        169, // OriX_D65
                        -388 // OriY_D65
                    },
                    // DF
                    {
                        0, // OriX_DF
                        0 // OriY_DF
                    }
                },
                // Rotated XY coordinate of AWB light source
                {
                    // Strobe
                    {
                        0,    // i4X
                        0    // i4Y
                    },
                    // Horizon
                    {
                        -382, // RotX_Hor
                        -365 // RotY_Hor
                    },
                    // A
                    {
                        -258, // RotX_A
                        -386 // RotY_A
                    },
                    // TL84
                    {
                        -69, // RotX_TL84
                        -429 // RotY_TL84
                    },
                    // CWF
                    {
                        -69, // RotX_CWF
                        -485 // RotY_CWF
                    },
                    // DNP
                    {
                        60, // RotX_DNP
                        -449 // RotY_DNP
                    },
                    // D65
                    {
                        169, // RotX_D65
                        -388 // RotY_D65
                    },
                    // DF
                    {
                        161, // RotX_DF
                        -473 // RotY_DF
                    }
                },
                // AWB gain of AWB light source
                {
                    // Strobe
                    {
                        810,    // i4R
                        512,    // i4G
                        677    // i4B
                    },
                    // Horizon
                    {
                        512, // AWBGAIN_HOR_R
                        524, // AWBGAIN_HOR_G
                        1441 // AWBGAIN_HOR_B
                    },
                    // A
                    {
                        609, // AWBGAIN_A_R
                        512, // AWBGAIN_A_G
                        1225 // AWBGAIN_A_B
                    },
                    // TL84
                    {
                        833, // AWBGAIN_TL84_R
                        512, // AWBGAIN_TL84_G
                        1004 // AWBGAIN_TL84_B
                    },
                    // CWF
                    {
                        900, // AWBGAIN_CWF_R
                        512, // AWBGAIN_CWF_G
                        1084 // AWBGAIN_CWF_B
                    },
                    // DNP
                    {
                        1020, // AWBGAIN_DNP_R
                        512, // AWBGAIN_DNP_G
                        866 // AWBGAIN_DNP_B
                    },
                    // D65
                    {
                        1088, // AWBGAIN_D65_R
                        512, // AWBGAIN_D65_G
                        689 // AWBGAIN_D65_B
                    },
                    // DF
                    {
                        512, // AWBGAIN_DF_R
                        512, // AWBGAIN_DF_G
                        512 // AWBGAIN_DF_B
                    }
                },
                // Rotation matrix parameter
                {
                    0, // RotationAngle
                    256, // Cos
                    0 // Sin
                },
                // Daylight locus parameter
                {
                    -125, //126,    // i4SlopeNumerator
                    128    // i4SlopeDenominator
                },
	            // Predictor gain
                {
                    141, // i4PrefRatio100
                    // DaylightLocus_L
                    {
                        1088, //1058, //1088, // i4R
                        512, //512, //512, // i4G
                        689 //689 //689 // i4B
                    },
                    // DaylightLocus_H
                    {
                        854, //839, // i4R
                        512, // i4G
                        878 //893, // i4B
                    },
                    // Temporal General
                    {
                        1088, // i4R
                        512, // i4G
                        689 // i4B
                    },
                    // AWB LSC Gain
                    {
                        854, //839, // i4R
                        512, // i4G
                        878 //893, // i4B
                    }
                },
                // AWB light area
                {
                    // Strobe:FIXME
                    {
                        -100, // i4RightBound
                        -250, // i4LeftBound
                        -361, // i4UpperBound
                        -600  // i4LowerBound
                    },
                    // Tungsten
                    {
                        -145, // TungRightBound
                        -782, // TungLeftBound
                        -310, //-330, // TungUpperBound
                        -408 // TungLowerBound
                    },
                    // Warm fluorescent
                    {
                        -145, // WFluoRightBound
                        -782, // WFluoLeftBound
                        -408, // WFluoUpperBound
                        -515 //-485 // WFluoLowerBound
                    },
                    // Fluorescent
                    {
                            18, // FluoRightBound
                            -145, //-164, // FluoLeftBound
                            -328, //-348, // FluoUpperBound
                            -430 //-454 //-444 // FluoLowerBound
                    },
                    // CWF
                    {
                        23, //-10, // CWFRightBound
                        -145, //-164, // CWFLeftBound
                            -430, // CWFUpperBound
                            -540 // CWFLowerBound
                    },
                    // Daylight
                    {
                        199, // DayRightBound
                            28, // DayLeftBound
                            -348, // DayUpperBound
                            -430 // DayLowerBound
                    },
                    // Shade
                    {
                        529, // ShadeRightBound
                        199, // ShadeLeftBound
                            -328, // ShadeUpperBound
                        -427 // ShadeLowerBound
                    },
                    // Daylight Fluorescent
                    {
                        199, // DFRightBound
                        23, //-10, // DFLeftBound
                            -430, // DFUpperBound
                            -540 // DFLowerBound
                    }
                },
                // PWB light area
                {
                    // Reference area
                    {
                        529, // PRefRightBound
                        -782, // PRefLeftBound
                        -305, // PRefUpperBound
                        -530 // PRefLowerBound
                    },
                    // Daylight
                    {
                        224, // PDayRightBound
                        28, // PDayLeftBound
                        -348, // PDayUpperBound
                        -464 // PDayLowerBound
                    },
                    // Cloudy daylight
                    {
                        324, // PCloudyRightBound
                        149, // PCloudyLeftBound
                        -348, // PCloudyUpperBound
                        -464 // PCloudyLowerBound
                    },
                    // Shade
                    {
                        424, // PShadeRightBound
                        149, // PShadeLeftBound
                        -348, // PShadeUpperBound
                        -464 // PShadeLowerBound
                    },
                    // Twilight
                    {
                        28, // PTwiRightBound
                        -132, // PTwiLeftBound
                        -348, // PTwiUpperBound
                        -464 // PTwiLowerBound
                    },
                    // Fluorescent
                    {
                        219, // PFluoRightBound
                        -169, // PFluoLeftBound
                        -338, // PFluoUpperBound
                        -535 // PFluoLowerBound
                    },
                    // Warm fluorescent
                    {
                        -158, // PWFluoRightBound
                        -358, // PWFluoLeftBound
                        -338, // PWFluoUpperBound
                        -535 // PWFluoLowerBound
                    },
                    // Incandescent
                    {
                        -158, // PIncaRightBound
                        -358, // PIncaLeftBound
                        -348, // PIncaUpperBound
                        -464 // PIncaLowerBound
                    },
                    // Gray World
                    {
                        5000, // PGWRightBound
                        -5000, // PGWLeftBound
                        5000, // PGWUpperBound
                        -5000 // PGWLowerBound
                    }
                },
                // PWB default gain
                {
                    // Daylight
                    {
                        1052, // PWB_Day_R
                        512, // PWB_Day_G
                        748 // PWB_Day_B
                    },
                    // Cloudy daylight
                    {
                        1222, // PWB_Cloudy_R
                        512, // PWB_Cloudy_G
                        644 // PWB_Cloudy_B
                    },
                    // Shade
                    {
                        1307, // PWB_Shade_R
                        512, // PWB_Shade_G
                        602 // PWB_Shade_B
                    },
                    // Twilight
                    {
                        827, // PWB_Twi_R
                        512, // PWB_Twi_G
                        952 // PWB_Twi_B
                    },
                    // Fluorescent
                    {
                        956, // PWB_Fluo_R
                        512, // PWB_Fluo_G
                        894 // PWB_Fluo_B
                    },
                    // Warm fluorescent
                    {
                        652, // PWB_WFluo_R
                        512, // PWB_WFluo_G
                        1311 // PWB_WFluo_B
                    },
                    // Incandescent
                    {
                        626, // PWB_Inca_R
                        512, // PWB_Inca_G
                        1258 // PWB_Inca_B
                    },
                    // Gray World
                    {
                        512, // PWB_GW_R
                        512, // PWB_GW_G
                        512 // PWB_GW_B
                    }
                },
                // AWB preference color
                {
                    // Tungsten
                    {
                        40, //50, // TUNG_SLIDER
                        4478 //3772 //4030//3837 //3537 // TUNG_OFFS
                    },
                    // Warm fluorescent
                    {
                        40, //50, //65, // WFluo_SLIDER
                        4478 //4272 //4272 //3972 //4030//3837 //3537 // WFluo_OFFS
                    },
                    // Shade
                    {
                        50, // Shade_SLIDER
                        909 // Shade_OFFS
                    },
                    // Sunset Area
                    {
                        92, //64,   // i4Sunset_BoundXr_Thr
                        -449 //-463     // i4Sunset_BoundYr_Thr
                    },
                    // Shade F Area
                    {

                        -145, //-135, //-82,   // i4BoundXrThr
                        -420 //-412 //-415    // i4BoundYrThr
                    },
                    // Shade F Vertex
                    {

                        -64, //-135, //-82,   // i4BoundXrThr
                        -425 //-412 //-415    // i4BoundYrThr
                    },
                    // Shade CWF Area
                    {
                        -145,//-109, //-89,   // i4BoundXrThr
                        -465//-475 //-509    // i4BoundYrThr
                    },
                    // Shade CWF Vertex
                    {
                        -61,//-109, //-89,   // i4BoundXrThr
                        -503//-475 //-509    // i4BoundYrThr
                    }
                },
                // CCT estimation
                {
	                // CCT
	                {
		                2300,	// i4CCT[0]
		                2850,	// i4CCT[1]
		                3750,	// i4CCT[2]
		                5100,	// i4CCT[3]
		                6500 	// i4CCT[4]
	                },
	                // Rotated X coordinate
	                {
		                -551,	// i4RotatedXCoordinate[0]
		                -427,	// i4RotatedXCoordinate[1]
		                -238,	// i4RotatedXCoordinate[2]
		                -109,	// i4RotatedXCoordinate[3]
		                0 	    // i4RotatedXCoordinate[4]
	                }
                }
            },

            // Algorithm Tuning Paramter
            {
                // AWB Backup Enable
                0,

                // Daylight locus offset LUTs for tungsten
                {
                    21, // i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 300,  600, 1022, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778,  5000} // i4LUTOut
                },

                // Daylight locus offset LUTs for WF
                {
                    21, // i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 350,  700,  850, 1200, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778,  5000} // i4LUTOut
                },

                // Daylight locus offset LUTs for shade
                {
                    21, // i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 500, 1000, 1500, 2500, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8000, 8000, 8500, 9000, 9500, 10000}  // i4LUTOut
                },
                // Preference gain for each light source
                {
                    //        LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                    //        LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
          	            {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
        	        }, // STROBE
        	        {
          	            {530, 530, 480}, {530, 530, 480}, {530, 530, 480}, {530, 530, 480}, {530, 530, 480}, {520, 520, 496}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
           	            {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
        	        }, // TUNGSTEN
        	        {
                        {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508}, {512, 512, 508},
           	            {512, 512, 508}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
        	        }, // WARM F
        	        {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                    }, // F
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // CWF
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                    }, // DAYLIGHT
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // SHADE
                    {
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    } // DAYLIGHT F
                },
                // Parent block weight parameter
                {
                    1,      // bEnable
                    6           // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
                },
                // AWB LV threshold for predictor
                {
                    115, //100,    // i4InitLVThr_L
                    155, //140,    // i4InitLVThr_H
                    100 //80      // i4EnqueueLVThr
                },
                // AWB number threshold for temporal predictor
                {
                        65,     // i4Neutral_ParentBlk_Thr
                    //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                    { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  50,  25,   2,   2,   2,   2,   2,   2,   2}  // (%) i4CWFDF_LUTThr
                },
                // AWB light neutral noise reduction for outdoor
                {
                    //LV0  1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                    // Non neutral
	                { 3,   3,   3,   3,   3,   3,   3,   3,    3,   3,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
	                // Flurescent
	                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
	                // CWF
	                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
	                // Daylight
	                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   2,   2,   2,   2,   2,   2,   2,   2},  // (%)
	                // DF
	                { 0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   5,  10,  10,  10,  10,  10,  10,  10,  10},  // (%)
                },
                // AWB feature detection
                {
                    // Sunset Prop
                    {
                        1,          // i4Enable
                        120,        // i4LVThr_L
                        130,        // i4LVThr_H
                        10,         // i4SunsetCountThr
                        0,          // i4SunsetCountRatio_L
                        171         // i4SunsetCountRatio_H
                    },

                    // Shade F Detection
                    {
                        1,          // i4Enable
                        0,//105,        // i4LVThr
                        1,//105,        // i4LVThr
                        96 //10         // i4DaylightProb
                    },

                    // Shade CWF Detection
                    {
                        1,          // i4Enable
                        0,//95,         // i4LVThr
                        1,//95,         // i4LVThr
                        208 //224 //128 //35         // i4DaylightProb
                    },
                },

                // AWB non-neutral probability for spatial and temporal weighting look-up table (Max: 100; Min: 0)
                {
                    //LV0   1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18
                    {  90, 90,  90,  90,  90,  90,  90,  90,  90,  90,   90,  50,  30,  20,  10,   0,   0,   0,   0}
                },

                // AWB daylight locus probability look-up table (Max: 100; Min: 0)
                {
                    //LV0  1    2    3    4    5    6    7    8    9     10    11   12   13   14   15  16   17   18
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100,  50,  25,   0,  0,   0,   0}, // Strobe
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,   75,  50,  25,  25,  25,  0,   0,   0}, // Tungsten
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,   75,  50,  25,  25,  25,  0,   0,   0}, // Warm fluorescent
                    {100, 100, 100, 100, 100, 100, 100, 100, 100,  95,   95,   70,  50,  25,  25,  25,  0,   0,   0}, // Fluorescent
                    { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,   80,   55,  30,  30,  30,  30,  0,   0,   0}, // CWF
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100, 100, 100,  75,  50, 40,  30,  20}, // Daylight
                    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,  100,  100,  75,  50,  25,   0,  0,   0,   0}, // Shade
                    { 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,   80,   55,  30,  30,  30,  30,  0,   0,   0}  // Daylight fluorescent
                },

                // AWB tuning information
                {
                    6739,       // project code
                    5588,       // model
                    20150624,   // date
                    0,          // reserved 0
                    1,          // reserved 1
                    2,          // reserved 2
                    3,          // reserved 3
                    4,          // reserved 4
                }
            }
        }
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

