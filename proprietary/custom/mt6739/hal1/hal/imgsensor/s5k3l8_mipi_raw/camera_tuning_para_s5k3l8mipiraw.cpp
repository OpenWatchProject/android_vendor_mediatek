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
#include <log/log.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_s5k3l8mipiraw.h"
#include "camera_info_s5k3l8mipiraw.h"
#include "camera_custom_AEPlinetable.h"
#include "camera_custom_tsf_tbl.h"

#define NVRAM_TUNING_PARAM_NUM  5341001

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
        // rDevicesInfo
        {
            1195,   // u4MinGain, 1024 base =  1x
            8192,  // u4MaxGain, 16x
            85,     // u4MiniISOGain, ISOxx
            128,    // u4GainStepUnit, 1x/8
            19770,     // u4PreExpUnit
            30,     // u4PreMaxFrameRate
            19770,     // u4VideoExpUnit
            30,     // u4VideoMaxFrameRate
            1024,   // u4Video2PreRatio, 1024 base = 1x
            13139,     // u4CapExpUnit
            24,     // u4CapMaxFrameRate
            1024,   // u4Cap2PreRatio, 1024 base = 1x
            19770,     // u4Video1ExpUnit
            30,     // u4Video1MaxFrameRate
            1024,   // u4Video12PreRatio, 1024 base = 1x
            19770,     // u4Video2ExpUnit
            30,     // u4Video2MaxFrameRate
            1024,   // u4Video22PreRatio, 1024 base = 1x
            19770,     // u4Custom1ExpUnit
            30,     // u4Custom1MaxFrameRate
            1024,   // u4Custom12PreRatio, 1024 base = 1x
            19770,     // u4Custom2ExpUnit
            30,     // u4Custom2MaxFrameRate
            1024,   // u4Custom22PreRatio, 1024 base = 1x
            19770,     // u4Custom3ExpUnit
            30,     // u4Custom3MaxFrameRate
            1024,   // u4Custom32PreRatio, 1024 base = 1x
            19770,     // u4Custom4ExpUnit
            30,     // u4Custom4MaxFrameRate
            1024,   // u4Custom42PreRatio, 1024 base = 1x
            19770,     // u4Custom5ExpUnit
            30,     // u4Custom5MaxFrameRate
            1024,   // u4Custom52PreRatio, 1024 base = 1x
            28,      // u4LensFno, Fno = 2.8
            350     // u4FocusLength_100x
        },
        // rHistConfig
        {
            4, // 2,   // u4HistHighThres
            40,  // u4HistLowThres
            2,   // u4MostBrightRatio
            1,   // u4MostDarkRatio
            160, // u4CentralHighBound
            20,  // u4CentralLowBound
            {240, 230, 220, 210, 200}, // u4OverExpThres[AE_CCT_STRENGTH_NUM]
            {62, 70, 82, 108, 141},  // u4HistStretchThres[AE_CCT_STRENGTH_NUM]
            {18, 22, 26, 30, 34}       // u4BlackLightThres[AE_CCT_STRENGTH_NUM]
        },
        // rCCTConfig
        {
            TRUE,            // bEnableBlackLight
            TRUE,            // bEnableHistStretch
            TRUE,           // bEnableAntiOverExposure
            TRUE,            // bEnableTimeLPF
            TRUE,            // bEnableCaptureThres
            TRUE,            // bEnableVideoThres
            TRUE,            // bEnableVideo1Thres
            TRUE,            // bEnableVideo2Thres
            TRUE,            // bEnableCustom1Thres
            TRUE,            // bEnableCustom2Thres
            TRUE,            // bEnableCustom3Thres
            TRUE,            // bEnableCustom4Thres
            TRUE,            // bEnableCustom5Thres
            TRUE,            // bEnableStrobeThres
            47,                // u4AETarget
            47,                // u4StrobeAETarget

            50,                // u4InitIndex
            4,                 // u4BackLightWeight
            32,                // u4HistStretchWeight
            4,                 // u4AntiOverExpWeight
            2,                 // u4BlackLightStrengthIndex
            2,                 // u4HistStretchStrengthIndex
            2,                 // u4AntiOverExpStrengthIndex
            2,                 // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
            90,                // u4InDoorEV = 9.0, 10 base
            -8,               // i4BVOffset delta BV = -2.3
            64,                 // u4PreviewFlareOffset
            64,                 // u4CaptureFlareOffset
            3,                 // u4CaptureFlareThres
            64,                 // u4VideoFlareOffset
            3,                 // u4VideoFlareThres
            64,               // u4CustomFlareOffset
            3,                 //  u4CustomFlareThres
            64,                 // u4StrobeFlareOffset //12 bits
            3,                 // u4StrobeFlareThres // 0.5%
            8,                 // u4PrvMaxFlareThres //12 bit
            0,                 // u4PrvMinFlareThres
            8,                 // u4VideoMaxFlareThres // 12 bit
            0,                 // u4VideoMinFlareThres
            18,                // u4FlatnessThres              // 10 base for flatness condition.
            75,    // u4FlatnessStrength
            //rMeteringSpec
            {
                //rHS_Spec
                {
                    TRUE,//bEnableHistStretch           // enable histogram stretch
                    1024,//u4HistStretchWeight          // Histogram weighting value
                    40, //50, //20,//u4Pcent                      // 1%=10, 0~1000
                    160,//u4Thd                        // 0~255
                    61,//75, //54, //74,//u4FlatThd                    // 0~255

                    120,//u4FlatBrightPcent
                    120,//u4FlatDarkPcent
                    //sFlatRatio
                    {
                        1000,  //i4X1
                        1024,  //i4Y1
                        2400, //i4X2
                        0     //i4Y2
                    },
                    TRUE, //bEnableGreyTextEnhance
                    1800, //u4GreyTextFlatStart, > sFlatRatio.i4X1, < sFlatRatio.i4X2
                    {
                        10,     //i4X1
                        1024,   //i4Y1
                        80,     //i4X2
                        0       //i4Y2
                    }
                },
                //rAOE_Spec
                {
                    TRUE, //bEnableAntiOverExposure
                    1024, //u4AntiOverExpWeight
                    10,    //u4Pcent
                    200,  //u4Thd

                    TRUE, //bEnableCOEP
                    1,    //u4COEPcent
                    106,  //u4COEThd
                    0,  // u4BVCompRatio
                    //sCOEYRatio;     // the outer y ratio
                    {
                        23,   //i4X1
                        1024,  //i4Y1
                        47,   //i4X2
                        0     //i4Y2
                    },
                    //sCOEDiffRatio;  // inner/outer y difference ratio
                    {
                        1500, //i4X1
                        0,    //i4Y1
                        2100, //i4X2
                        1024   //i4Y2
                    }
                },
                //rABL_Spec
                {
                    TRUE,//bEnableBlackLigh
                    1024,//u4BackLightWeigh
                    400,//u4Pcent
                    28,//u4Thd,
                    255, // center luminance
                    256, // final target limitation, 256/128 = 2x
                    //sFgBgEVRatio
                    {
                        2200, //i4X1
                        0,    //i4Y1
                        4000, //i4X2
                        1024   //i4Y2
                    },
                    //sBVRatio
                    {
                        3800,//i4X1
                        0,   //i4Y1
                        5000,//i4X2
                        1024  //i4Y2
                    }
                },
                //rNS_Spec
                {
                    TRUE, // bEnableNightScene
                    5,    //u4Pcent
                    160,  //u4Thd
                    60,//72, //52,   //u4FlatThd

                    200,  //u4BrightTonePcent
                    86, //u4BrightToneThd

                    500,  //u4LowBndPcent
                    5,    //u4LowBndThd
                    23,    //u4LowBndThdLimit

                    50,  //u4FlatBrightPcent;
                    300,   //u4FlatDarkPcent;
                    //sFlatRatio
                    {
                        1200, //i4X1
                        1024, //i4Y1
                        2400, //i4X2
                        0    //i4Y2
                    },
                    //sBVRatio
                    {
                        -500, //i4X1
                        1024,  //i4Y1
                        3000, //i4X2
                        0     //i4Y2
                    },
                    TRUE, // bEnableNightSkySuppresion
                    //sSkyBVRatio
                    {
                        -4000, //i4X1
                        1024, //i4X2
                        -2000,  //i4Y1
                        0     //i4Y2
                    }
                },
                // rTOUCHFD_Spec
                {
                    40, //uMeteringYLowBound;
                    50, //uMeteringYHighBound;
                    40, //uFaceYLowBound;
                    50, //uFaceYHighBound;
                    3,  //uFaceCentralWeight;
                    120,//u4MeteringStableMax;
                    80, //u4MeteringStableMin;
                }
            }, //End rMeteringSpec
            // rFlareSpec
            {
                {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //uPrvFlareWeightArr[16];
                {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, //uVideoFlareWeightArr[16];
                96,                                               //u4FlareStdThrHigh;
                48,                                               //u4FlareStdThrLow;
                0,                                                //u4PrvCapFlareDiff;
                4,                                                //u4FlareMaxStepGap_Fast;
                0,                                                //u4FlareMaxStepGap_Slow;
                1800,                                             //u4FlarMaxStepGapLimitBV;
                0,                                                //u4FlareAEStableCount;
            },
            //rAEMoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                190, //u4Bright2TargetEnd
                20,   //u4Dark2TargetStart
                90, //u4B2TEnd
                80,  //u4B2TStart
                80,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAEVideoMoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                80,  //u4B2TStart
                                80,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAEVideo1MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                80,  //u4B2TStart
                                80,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAEVideo2MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                80,  //u4B2TStart
                                80,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAECustom1MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                80,  //u4B2TStart
                                80,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAECustom2MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                80,  //u4B2TStart
                                80,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAECustom3MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                80,  //u4B2TStart
                                80,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAECustom4MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                80,  //u4B2TStart
                                80,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAECustom5MoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                150,  //u4Bright2TargetEnd
                20,    //u4Dark2TargetStart
                90, //u4B2TEnd
                80,  //u4B2TStart
                                80,  //u4D2TEnd
                90,  //u4D2TStart
            },

            //rAEFaceMoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                190,  //u4Bright2TargetEnd
                10,    //u4Dark2TargetStart
                80, //u4B2TEnd
                30,  //u4B2TStart
                20,  //u4D2TEnd
                60,  //u4D2TStart
            },

            //rAETrackingMoveRatio =
            {
                100, //u4SpeedUpRatio
                100, //u4GlobalRatio
                190,  //u4Bright2TargetEnd
                10,    //u4Dark2TargetStart
                80, //u4B2TEnd
                30,  //u4B2TStart
                20,  //u4D2TEnd
                60,  //u4D2TStart
            },
            //rAEAOENVRAMParam =
            {
                1,      // i4AOEStrengthIdx: 0 / 1 / 2
                130,    // u4BVCompRatio
                {
                    {
                        47,  //u4Y_Target
                        25,  //u4AOE_OE_percent
                        210,  //u4AOE_OEBound
                        10,    //u4AOE_DarkBound
                        950,    //u4AOE_LowlightPrecent
                        1,    //u4AOE_LowlightBound
                        145,    //u4AOESceneLV_L
                        180,    //u4AOESceneLV_H
                        40,    //u4AOE_SWHdrLE_Bound
                    },
                    {
                        47,  //u4Y_Target
                        25,  //u4AOE_OE_percent
                        210,  //u4AOE_OEBound
                        15, //20,    //u4AOE_DarkBound
                        950,    //u4AOE_LowlightPrecent
                        3, //10,    //u4AOE_LowlightBound
                        145,    //u4AOESceneLV_L
                        180,    //u4AOESceneLV_H
                        40,    //u4AOE_SWHdrLE_Bound
                    },
                    {
                        47,  //u4Y_Target
                        25,  //u4AOE_OE_percent
                        210,  //u4AOE_OEBound
                        25,    //u4AOE_DarkBound
                        950,    //u4AOE_LowlightPrecent
                        8,    //u4AOE_LowlightBound
                        145,    //u4AOESceneLV_L
                        180,    //u4AOESceneLV_H
                        40,    //u4AOE_SWHdrLE_Bound
                    }
                }
            }
        }
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
                        0, // UnitGain_R
                        0, // UnitGain_G
                        0  // UnitGain_B
                    },
                    // rGoldenGain (golden sample gain: 1.0 = 512)
                    {
                        0, // GoldenGain_R
                        0, // GoldenGain_G
                        0  // GoldenGain_B
                    },
                    // rUnitGain for TL84(unit gain: 1.0 = 512)
                    {
                        0, // UnitGain_R
                        0, // UnitGain_G
                        0  // UnitGain_B
                    },
                    // rGoldenGain for TL84(golden sample gain: 1.0 = 512)
                    {
                        0, // GoldenGain_R
                        0, // GoldenGain_G
                        0  // GoldenGain_B
                    },
                    // rUnitGain for ALight(unit gain: 1.0 = 512)
                    {
                        0, // UnitGain_R
                        0, // UnitGain_G
                        0  // UnitGain_B
                    },
                    // rGoldenGain for ALight(golden sample gain: 1.0 = 512)
                    {
                        0, // GoldenGain_R
                        0, // GoldenGain_G
                        0  // GoldenGain_B
                    },
                    // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
                    {
                        0, // TuningUnitGain_R
                        0, // TuningUnitGain_G
                        0 // TuningUnitGain_B
                    },
                    // rD65Gain (D65 WB gain: 1.0 = 512)
                    {
                        1223, // D65Gain_R
                        512, // D65Gain_G
                        660 // D65Gain_B
                    }
                },
                // Original XY coordinate of AWB light source
                {
                    // Strobe
                    {
                        0, // OriX_Strobe
                        0 // OriY_Strobe
                    },
                    // Horizon
                    {
                        -366, // OriX_Hor
                        -359 // OriY_Hor
                    },
                    // A
                    {
                        -223, // OriX_A
                        -387 // OriY_A
                    },
                    // TL84
                    {
                        -11, // OriX_TL84
                        -416 // OriY_TL84
                    },
                    // CWF
                    {
                        -5, // OriX_CWF
                        -514 // OriY_CWF
                    },
                    // DNP
                    {
                        107, // OriX_DNP
                        -433 // OriY_DNP
                    },
                    // D65
                    {
                        228, // OriX_D65
                        -416 // OriY_D65
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
                        0, // RotX_Strobe
                        0 // RotY_Strobe
                    },
                    // Horizon
                    {
                        -339, // RotX_Hor
                        -383 // RotY_Hor
                    },
                    // A
                    {
                        -195, // RotX_A
                        -401 // RotY_A
                    },
                    // TL84
                    {
                        18, // RotX_TL84
                        -415 // RotY_TL84
                    },
                    // CWF
                    {
                        31, // RotX_CWF
                        -512 // RotY_CWF
                    },
                    // DNP
                    {
                        137, // RotX_DNP
                        -424 // RotY_DNP
                    },
                    // D65
                    {
                        256, // RotX_D65
                        -398 // RotY_D65
                    },
                    // DF
                    {
                        254, // RotX_DF
                        -498 // RotY_DF
                    }
                },
                // AWB gain of AWB light source
                {
                    // Strobe
                    {
                        512, // AWBGAIN_STROBE_R
                        512, // AWBGAIN_STROBE_G
                        512 // AWBGAIN_STROBE_B
                    },
                    // Horizon
                    {
                        512, // AWBGAIN_HOR_R
                        517, // AWBGAIN_HOR_G
                        1378 // AWBGAIN_HOR_B
                    },
                    // A
                    {
                        639, // AWBGAIN_A_R
                        512, // AWBGAIN_A_G
                        1170 // AWBGAIN_A_B
                    },
                    // TL84
                    {
                        886, // AWBGAIN_TL84_R
                        512, // AWBGAIN_TL84_G
                        912 // AWBGAIN_TL84_B
                    },
                    // CWF
                    {
                        1020, // AWBGAIN_CWF_R
                        512, // AWBGAIN_CWF_G
                        1034 // AWBGAIN_CWF_B
                    },
                    // DNP
                    {
                        1065, // AWBGAIN_DNP_R
                        512, // AWBGAIN_DNP_G
                        796 // AWBGAIN_DNP_B
                    },
                    // D65
                    {
                        1223, // AWBGAIN_D65_R
                        512, // AWBGAIN_D65_G
                        660 // AWBGAIN_D65_B
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
                    -4, // RotationAngle
                    255, // Cos
                    -18 // Sin
                },
                // Daylight locus parameter
                {
                    -110, // SlopeNumerator
                    128 // SlopeDenominator
                },
                // Predictor gain
                {
                    0, // i4PrefRatio100
                    // DaylightLocus_L
                    {
                        1152, // i4R
                        512, // i4G
                        661 // i4B
                    },
                    // DaylightLocus_H
                    {
                        962, // i4R
                        512, // i4G
                        787 // i4B
                    },
                    // Temporal General
                    {
                        953, // i4R
                        512, // i4G
                        835 // i4B
                    },
                    // AWB LSC
                    {
                        962, // i4R
                        512, // i4G
                        787 // i4B
                    }
                },
                // AWB light area
                {
                    // Strobe
                    {
                        0, // StrobeRightBound
                        0, // StrobeLeftBound
                        0, // StrobeUpperBound
                        0 // StrobeLowerBound
                    },
                    // Tungsten
                    {
                        -67, // TungRightBound
                        -739, // TungLeftBound
                        -308, // TungUpperBound
                        -373 // TungLowerBound
                    },
                    // Warm fluorescent
                    {
                        -67, // WFluoRightBound
                        -739, // WFluoLeftBound
                        -373, // WFluoUpperBound
                        -602 // WFluoLowerBound
                    },
                    // Fluorescent
                    {
                        98, // FluoRightBound
                        -67, // FluoLeftBound
                        -338, // FluoUpperBound
                        -464 // FluoLowerBound
                    },
                    // CWF
                    {
                        129, // CWFRightBound
                        -67, // CWFLeftBound
                        -464, // CWFUpperBound
                        -567 // CWFLowerBound
                    },
                    // Daylight
                    {
                        286, // DayRightBound
                        98, // DayLeftBound
                        -338, // DayUpperBound
                        -464 // DayLowerBound
                    },
                    // Shade
                    {
                        616, // ShadeRightBound
                        286, // ShadeLeftBound
                        -338, // ShadeUpperBound
                        -444 // ShadeLowerBound
                    },
                    // Daylight Fluorescent
                    {
                        286, // DFRightBound
                        129, // DFLeftBound
                        -464, // DFUpperBound
                        -567 // DFLowerBound
                    }
                },
                // PWB light area
                {
                    // Reference area
                    {
                        616, // PRefRightBound
                        -739, // PRefLeftBound
                        -283, // PRefUpperBound
                        -602 // PRefLowerBound
                    },
                    // Daylight
                    {
                        311, // PDayRightBound
                        98, // PDayLeftBound
                        -338, // PDayUpperBound
                        -464 // PDayLowerBound
                    },
                    // Cloudy daylight
                    {
                        411, // PCloudyRightBound
                        236, // PCloudyLeftBound
                        -338, // PCloudyUpperBound
                        -464 // PCloudyLowerBound
                    },
                    // Shade
                    {
                        511, // PShadeRightBound
                        236, // PShadeLeftBound
                        -338, // PShadeUpperBound
                        -464 // PShadeLowerBound
                    },
                    // Twilight
                    {
                        98, // PTwiRightBound
                        -62, // PTwiLeftBound
                        -338, // PTwiUpperBound
                        -464 // PTwiLowerBound
                    },
                    // Fluorescent
                    {
                        306, // PFluoRightBound
                        -82, // PFluoLeftBound
                        -348, // PFluoUpperBound
                        -562 // PFluoLowerBound
                    },
                    // Warm fluorescent
                    {
                        -95, // PWFluoRightBound
                        -295, // PWFluoLeftBound
                        -348, // PWFluoUpperBound
                        -562 // PWFluoLowerBound
                    },
                    // Incandescent
                    {
                        -95, // PIncaRightBound
                        -295, // PIncaLeftBound
                        -338, // PIncaUpperBound
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
                        1140, // PWB_Day_R
                        512, // PWB_Day_G
                        708 // PWB_Day_B
                    },
                    // Cloudy daylight
                    {
                        1354, // PWB_Cloudy_R
                        512, // PWB_Cloudy_G
                        609 // PWB_Cloudy_B
                    },
                    // Shade
                    {
                        1456, // PWB_Shade_R
                        512, // PWB_Shade_G
                        572 // PWB_Shade_B
                    },
                    // Twilight
                    {
                        870, // PWB_Twi_R
                        512, // PWB_Twi_G
                        895 // PWB_Twi_B
                    },
                    // Fluorescent
                    {
                        1067, // PWB_Fluo_R
                        512, // PWB_Fluo_G
                        859 // PWB_Fluo_B
                    },
                    // Warm fluorescent
                    {
                        684, // PWB_WFluo_R
                        512, // PWB_WFluo_G
                        1264 // PWB_WFluo_B
                    },
                    // Incandescent
                    {
                        639, // PWB_Inca_R
                        512, // PWB_Inca_G
                        1169 // PWB_Inca_B
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
                        40, // TUNG_SLIDER
                        4613 // TUNG_OFFS
                    },
                    // Warm fluorescent
                    {
                        40, // WFluo_SLIDER
                        4613 // WFluo_OFFS
                    },
                    // Shade
                    {
                        50, // Shade_SLIDER
                        910 // Shade_OFFS
                    },
                    // Sunset Area
                    {
                        167, // i4Sunset_BoundXr_Thr
                        -424 // i4Sunset_BoundYr_Thr
                    },
                    // Shade F Area
                    {
                        -67, // i4ShadeF_BoundXr_Thr
                        -419 // i4ShadeF_BoundYr_Thr
                    },
                    // Shade F Vertex
                    {
                        16, // i4ShadeF_VertexXr_Thr
                        -442 // i4ShadeF_VertexYr_Thr
                    },
                    // Shade CWF Area
                    {
                        -67, // i4ShadeCWF_BoundXr_Thr
                        -516 // i4ShadeCWF_BoundYr_Thr
                    },
                    // Shade CWF Vertex
                    {
                        31, // i4ShadeCWF_VertexXr_Thr
                        -542 // i4ShadeCWF_VertexYr_Thr
                    }
                },
                // CCT estimation
                {
                    // CCT
                    {
                        2300,  // CCT0
                        2850,  // CCT1
                        3750,  // CCT2
                        5100,  // CCT3
                        6500  // CCT4
                    },
                    // Rotated X coordinate
                    {
                        -595,  // RotatedXCoordinate0
                        -451,  // RotatedXCoordinate1
                        -238,  // RotatedXCoordinate2
                        -119,  // RotatedXCoordinate3
                        0  // RotatedXCoordinate4
                    }
                }
            },
            // Algorithm Tuning Paramter
            {
                // AWB Backup Enable
                0,
                // Daylight locus offset LUTs for tungsten
                {
                    21,// i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 350, 800, 1222, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778, 5000}   // i4LUTOut
                },
                // Daylight locus offset LUTs for WF
                {
                    21,// i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 350, 700, 1000, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778, 5000}   // i4LUTOut
                },
                // Daylight locus offset LUTs for Shade
                {
                    21,// i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}   // i4LUTOut
                },
                // Preference gain for each light source
                {
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // STROBE
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // TUNGSTEN
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // WARM F
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                    }, // F
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // CWF
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                    }, // DAYLIGHT
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // SHADE
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    } // DAYLIGHT F
                },
                // Parent block weight parameter
                {
                    1,  // bEnable
                    6  // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
                },
                // AWB LV threshold for predictor
                {
                    90,  // i4InitLVThr_L
                    130,  // i4InitLVThr_H
                    100  // i4EnqueueLVThr
                },
                // AWB number threshold for temporal predictor
                {
                    65,  // i4Neutral_ParentBlk_Thr
                    //LV0    1     2     3     4     5     6     7     8     9     10    11    12    13    14    15    16    17    18
                    { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,   50,   25,    2,    2,    2,    2,    2,    2,    2}  // i4CWFDF_LUTThr
                },
                // AWB light neutral noise reduction for outdoor
                {
                    //LV0  1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18
                    // Non neutral
                    {  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  10,  10,  10,  10,  10,  10,  10,  10},  //
                    // Flurescent
                    {  0,  0,  0,  0,  0,  3,  5,  5,  5,  5,  5,  10,  10,  10,  10,  10,  10,  10,  10},  //
                    // CWF
                    {  0,  0,  0,  0,  0,  3,  5,  5,  5,  5,  5,  10,  10,  10,  10,  10,  10,  10,  10},  //
                    // Daylight
                    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   2,   2,   2,   2,   2,   2,   2,   2},  //
                    // DF
                    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  10,  10,  10,  10,  10,  10,  10,  10}  //
                },
                // AWB feature detection
                {
                     // Sunset Prop
                     {
                         1, // i4Enable
                         120, // i4LVThr_L
                         130, // i4LVThr_H
                         10, // i4SunsetCountThr
                         0, // i4SunsetCountRatio_L
                         171 // i4SunsetCountRatio_H
                     },
                     // Shade F Detection
                     {
                         1, // i4Enable
                         50, // i4LVThr_L
                         90, // i4LVThr_H
                         128 // i4DaylightProb
                     },
                     // Shade CWF Detection
                     {
                         1, // i4Enable
                         50, // i4LVThr_L
                         90, // i4LVThr_H
                         192 // i4DaylightProb
                     }
                 },
                 // AWB non-neutral probability for spatial and temporal weighting look-up table (Max: 100; Min: 0)
                 {
                     //LV0   1    2    3     4     5     6     7     8     9     10    11    12    13    14    15    16    17    18
                     {  0,  33,  66,  100,  100,  100,  100,  100,  100,  100,  100,   70,   30,   20,   10,    0,    0,    0,    0}
                },
                // AWB daylight locus probability look-up table (Max: 100; Min: 0)
                {
                    //LV0    1     2     3     4     5     6     7     8     9     10    11    12    13    14   15   16   17   18
                    { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,   50,   25,   0,   0,   0,   0}, // Strobe
                    { 95,  95,  95,  95,  95,  95,  100,  100,  100,  100,  100,  100,  100,   75,   50,  25,   0,   0,   0}, // Tungsten
                    { 95,  95,  95,  95,  95,  95,  100,  100,  100,  100,  100,   75,   50,   25,   25,  25,   0,   0,   0}, // Warm fluorescent
                    { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,   95,   75,   50,   25,   25,  25,   0,   0,   0}, // Fluorescent
                    {  90,   90,   90,   90,   90,   90,   90,   90,   90,   90,   80,   55,   30,   30,   30,  30,   0,   0,   0}, // CWF
                    { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,   75,  50,  40,  30,  20}, // Daylight
                    { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,   75,   50,   25,   0,   0,   0,   0}, // Shade
                    {  90,   90,   90,   90,   90,   90,   90,   90,   90,   90,   80,   55,   30,   30,   30,  30,   0,   0,   0}  // Daylight fluorescent
                },
                // AWB tuning information
                {
                     0, // i4ProjCode
                     0, // i4Model
                     0, // i4Date
                     0, // i4Res0
                     0, // i4Res1
                     0, // i4Res2
                     0, // i4Res3
                     0, // i4Res4
                }
            }
        },
        {
            {
                // AWB calibration data
                {
                    // rUnitGain (unit gain: 1.0 = 512)
                    {
                        0, // UnitGain_R
                        0, // UnitGain_G
                        0  // UnitGain_B
                    },
                    // rGoldenGain (golden sample gain: 1.0 = 512)
                    {
                        0, // GoldenGain_R
                        0, // GoldenGain_G
                        0  // GoldenGain_B
                    },
                    // rUnitGain for TL84(unit gain: 1.0 = 512)
                    {
                        0, // UnitGain_R
                        0, // UnitGain_G
                        0  // UnitGain_B
                    },
                    // rGoldenGain for TL84(golden sample gain: 1.0 = 512)
                    {
                        0, // GoldenGain_R
                        0, // GoldenGain_G
                        0  // GoldenGain_B
                    },
                    // rUnitGain for ALight(unit gain: 1.0 = 512)
                    {
                        0, // UnitGain_R
                        0, // UnitGain_G
                        0  // UnitGain_B
                    },
                    // rGoldenGain for ALight(golden sample gain: 1.0 = 512)
                    {
                        0, // GoldenGain_R
                        0, // GoldenGain_G
                        0  // GoldenGain_B
                    },
                    // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
                    {
                        0, // TuningUnitGain_R
                        0, // TuningUnitGain_G
                        0 // TuningUnitGain_B
                    },
                    // rD65Gain (D65 WB gain: 1.0 = 512)
                    {
                        1223, // D65Gain_R
                        512, // D65Gain_G
                        660 // D65Gain_B
                    }
                },
                // Original XY coordinate of AWB light source
                {
                    // Strobe
                    {
                        0, // OriX_Strobe
                        0 // OriY_Strobe
                    },
                    // Horizon
                    {
                        -366, // OriX_Hor
                        -359 // OriY_Hor
                    },
                    // A
                    {
                        -223, // OriX_A
                        -387 // OriY_A
                    },
                    // TL84
                    {
                        -11, // OriX_TL84
                        -416 // OriY_TL84
                    },
                    // CWF
                    {
                        -5, // OriX_CWF
                        -514 // OriY_CWF
                    },
                    // DNP
                    {
                        107, // OriX_DNP
                        -433 // OriY_DNP
                    },
                    // D65
                    {
                        228, // OriX_D65
                        -416 // OriY_D65
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
                        0, // RotX_Strobe
                        0 // RotY_Strobe
                    },
                    // Horizon
                    {
                        -339, // RotX_Hor
                        -383 // RotY_Hor
                    },
                    // A
                    {
                        -195, // RotX_A
                        -401 // RotY_A
                    },
                    // TL84
                    {
                        18, // RotX_TL84
                        -415 // RotY_TL84
                    },
                    // CWF
                    {
                        31, // RotX_CWF
                        -512 // RotY_CWF
                    },
                    // DNP
                    {
                        137, // RotX_DNP
                        -424 // RotY_DNP
                    },
                    // D65
                    {
                        256, // RotX_D65
                        -398 // RotY_D65
                    },
                    // DF
                    {
                        254, // RotX_DF
                        -498 // RotY_DF
                    }
                },
                // AWB gain of AWB light source
                {
                    // Strobe
                    {
                        512, // AWBGAIN_STROBE_R
                        512, // AWBGAIN_STROBE_G
                        512 // AWBGAIN_STROBE_B
                    },
                    // Horizon
                    {
                        512, // AWBGAIN_HOR_R
                        517, // AWBGAIN_HOR_G
                        1378 // AWBGAIN_HOR_B
                    },
                    // A
                    {
                        639, // AWBGAIN_A_R
                        512, // AWBGAIN_A_G
                        1170 // AWBGAIN_A_B
                    },
                    // TL84
                    {
                        886, // AWBGAIN_TL84_R
                        512, // AWBGAIN_TL84_G
                        912 // AWBGAIN_TL84_B
                    },
                    // CWF
                    {
                        1020, // AWBGAIN_CWF_R
                        512, // AWBGAIN_CWF_G
                        1034 // AWBGAIN_CWF_B
                    },
                    // DNP
                    {
                        1065, // AWBGAIN_DNP_R
                        512, // AWBGAIN_DNP_G
                        796 // AWBGAIN_DNP_B
                    },
                    // D65
                    {
                        1223, // AWBGAIN_D65_R
                        512, // AWBGAIN_D65_G
                        660 // AWBGAIN_D65_B
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
                    -4, // RotationAngle
                    255, // Cos
                    -18 // Sin
                },
                // Daylight locus parameter
                {
                    -110, // SlopeNumerator
                    128 // SlopeDenominator
                },
                // Predictor gain
                {
                    0, // i4PrefRatio100
                    // DaylightLocus_L
                    {
                        1152, // i4R
                        512, // i4G
                        661 // i4B
                    },
                    // DaylightLocus_H
                    {
                        962, // i4R
                        512, // i4G
                        787 // i4B
                    },
                    // Temporal General
                    {
                        953, // i4R
                        512, // i4G
                        835 // i4B
                    },
                    // AWB LSC
                    {
                        962, // i4R
                        512, // i4G
                        787 // i4B
                    }
                },
                // AWB light area
                {
                    // Strobe
                    {
                        0, // StrobeRightBound
                        0, // StrobeLeftBound
                        0, // StrobeUpperBound
                        0 // StrobeLowerBound
                    },
                    // Tungsten
                    {
                        -67, // TungRightBound
                        -739, // TungLeftBound
                        -308, // TungUpperBound
                        -373 // TungLowerBound
                    },
                    // Warm fluorescent
                    {
                        -67, // WFluoRightBound
                        -739, // WFluoLeftBound
                        -373, // WFluoUpperBound
                        -602 // WFluoLowerBound
                    },
                    // Fluorescent
                    {
                        98, // FluoRightBound
                        -67, // FluoLeftBound
                        -338, // FluoUpperBound
                        -464 // FluoLowerBound
                    },
                    // CWF
                    {
                        129, // CWFRightBound
                        -67, // CWFLeftBound
                        -464, // CWFUpperBound
                        -567 // CWFLowerBound
                    },
                    // Daylight
                    {
                        286, // DayRightBound
                        98, // DayLeftBound
                        -338, // DayUpperBound
                        -464 // DayLowerBound
                    },
                    // Shade
                    {
                        616, // ShadeRightBound
                        286, // ShadeLeftBound
                        -338, // ShadeUpperBound
                        -444 // ShadeLowerBound
                    },
                    // Daylight Fluorescent
                    {
                        286, // DFRightBound
                        129, // DFLeftBound
                        -464, // DFUpperBound
                        -567 // DFLowerBound
                    }
                },
                // PWB light area
                {
                    // Reference area
                    {
                        616, // PRefRightBound
                        -739, // PRefLeftBound
                        -283, // PRefUpperBound
                        -602 // PRefLowerBound
                    },
                    // Daylight
                    {
                        311, // PDayRightBound
                        98, // PDayLeftBound
                        -338, // PDayUpperBound
                        -464 // PDayLowerBound
                    },
                    // Cloudy daylight
                    {
                        411, // PCloudyRightBound
                        236, // PCloudyLeftBound
                        -338, // PCloudyUpperBound
                        -464 // PCloudyLowerBound
                    },
                    // Shade
                    {
                        511, // PShadeRightBound
                        236, // PShadeLeftBound
                        -338, // PShadeUpperBound
                        -464 // PShadeLowerBound
                    },
                    // Twilight
                    {
                        98, // PTwiRightBound
                        -62, // PTwiLeftBound
                        -338, // PTwiUpperBound
                        -464 // PTwiLowerBound
                    },
                    // Fluorescent
                    {
                        306, // PFluoRightBound
                        -82, // PFluoLeftBound
                        -348, // PFluoUpperBound
                        -562 // PFluoLowerBound
                    },
                    // Warm fluorescent
                    {
                        -95, // PWFluoRightBound
                        -295, // PWFluoLeftBound
                        -348, // PWFluoUpperBound
                        -562 // PWFluoLowerBound
                    },
                    // Incandescent
                    {
                        -95, // PIncaRightBound
                        -295, // PIncaLeftBound
                        -338, // PIncaUpperBound
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
                        1140, // PWB_Day_R
                        512, // PWB_Day_G
                        708 // PWB_Day_B
                    },
                    // Cloudy daylight
                    {
                        1354, // PWB_Cloudy_R
                        512, // PWB_Cloudy_G
                        609 // PWB_Cloudy_B
                    },
                    // Shade
                    {
                        1456, // PWB_Shade_R
                        512, // PWB_Shade_G
                        572 // PWB_Shade_B
                    },
                    // Twilight
                    {
                        870, // PWB_Twi_R
                        512, // PWB_Twi_G
                        895 // PWB_Twi_B
                    },
                    // Fluorescent
                    {
                        1067, // PWB_Fluo_R
                        512, // PWB_Fluo_G
                        859 // PWB_Fluo_B
                    },
                    // Warm fluorescent
                    {
                        684, // PWB_WFluo_R
                        512, // PWB_WFluo_G
                        1264 // PWB_WFluo_B
                    },
                    // Incandescent
                    {
                        639, // PWB_Inca_R
                        512, // PWB_Inca_G
                        1169 // PWB_Inca_B
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
                        40, // TUNG_SLIDER
                        4613 // TUNG_OFFS
                    },
                    // Warm fluorescent
                    {
                        40, // WFluo_SLIDER
                        4613 // WFluo_OFFS
                    },
                    // Shade
                    {
                        50, // Shade_SLIDER
                        910 // Shade_OFFS
                    },
                    // Sunset Area
                    {
                        167, // i4Sunset_BoundXr_Thr
                        -424 // i4Sunset_BoundYr_Thr
                    },
                    // Shade F Area
                    {
                        -67, // i4ShadeF_BoundXr_Thr
                        -419 // i4ShadeF_BoundYr_Thr
                    },
                    // Shade F Vertex
                    {
                        16, // i4ShadeF_VertexXr_Thr
                        -442 // i4ShadeF_VertexYr_Thr
                    },
                    // Shade CWF Area
                    {
                        -67, // i4ShadeCWF_BoundXr_Thr
                        -516 // i4ShadeCWF_BoundYr_Thr
                    },
                    // Shade CWF Vertex
                    {
                        31, // i4ShadeCWF_VertexXr_Thr
                        -542 // i4ShadeCWF_VertexYr_Thr
                    }
                },
                // CCT estimation
                {
                    // CCT
                    {
                        2300,  // CCT0
                        2850,  // CCT1
                        3750,  // CCT2
                        5100,  // CCT3
                        6500  // CCT4
                    },
                    // Rotated X coordinate
                    {
                        -595,  // RotatedXCoordinate0
                        -451,  // RotatedXCoordinate1
                        -238,  // RotatedXCoordinate2
                        -119,  // RotatedXCoordinate3
                        0  // RotatedXCoordinate4
                    }
                }
            },
            // Algorithm Tuning Paramter
            {
                // AWB Backup Enable
                0,
                // Daylight locus offset LUTs for tungsten
                {
                    21,// i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 350, 800, 1222, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778, 5000}   // i4LUTOut
                },
                // Daylight locus offset LUTs for WF
                {
                    21,// i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 350, 700, 1000, 1444, 1667, 1889, 2111, 2333, 2556, 2778, 3000, 3222, 3444, 3667, 3889, 4111, 4333, 4556, 4778, 5000}   // i4LUTOut
                },
                // Daylight locus offset LUTs for Shade
                {
                    21,// i4Size: LUT dimension
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}, // i4LUTIn
                    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000}   // i4LUTOut
                },
                // Preference gain for each light source
                {
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // STROBE
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // TUNGSTEN
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // WARM F
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                    }, // F
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // CWF
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}, {502, 512, 512}
                    }, // DAYLIGHT
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    }, // SHADE
                    {
                        //    LV0              LV1              LV2              LV3              LV4              LV5              LV6              LV7              LV8              LV9
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512},
                        //    LV10             LV11             LV12             LV13             LV14             LV15             LV16             LV17             LV18
                        {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}, {512, 512, 512}
                    } // DAYLIGHT F
                },
                // Parent block weight parameter
                {
                    1,  // bEnable
                    6  // i4ScalingFactor: [6] 1~12, [7] 1~6, [8] 1~3, [9] 1~2, [>=10]: 1
                },
                // AWB LV threshold for predictor
                {
                    90,  // i4InitLVThr_L
                    130,  // i4InitLVThr_H
                    100  // i4EnqueueLVThr
                },
                // AWB number threshold for temporal predictor
                {
                    65,  // i4Neutral_ParentBlk_Thr
                    //LV0    1     2     3     4     5     6     7     8     9     10    11    12    13    14    15    16    17    18
                    { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,   50,   25,    2,    2,    2,    2,    2,    2,    2}  // i4CWFDF_LUTThr
                },
                // AWB light neutral noise reduction for outdoor
                {
                    //LV0  1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18
                    // Non neutral
                    {  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  10,  10,  10,  10,  10,  10,  10,  10},  //
                    // Flurescent
                    {  0,  0,  0,  0,  0,  3,  5,  5,  5,  5,  5,  10,  10,  10,  10,  10,  10,  10,  10},  //
                    // CWF
                    {  0,  0,  0,  0,  0,  3,  5,  5,  5,  5,  5,  10,  10,  10,  10,  10,  10,  10,  10},  //
                    // Daylight
                    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   2,   2,   2,   2,   2,   2,   2,   2},  //
                    // DF
                    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  10,  10,  10,  10,  10,  10,  10,  10}  //
                },
                // AWB feature detection
                {
                     // Sunset Prop
                     {
                         1, // i4Enable
                         120, // i4LVThr_L
                         130, // i4LVThr_H
                         10, // i4SunsetCountThr
                         0, // i4SunsetCountRatio_L
                         171 // i4SunsetCountRatio_H
                     },
                     // Shade F Detection
                     {
                         1, // i4Enable
                         50, // i4LVThr_L
                         90, // i4LVThr_H
                         128 // i4DaylightProb
                     },
                     // Shade CWF Detection
                     {
                         1, // i4Enable
                         50, // i4LVThr_L
                         90, // i4LVThr_H
                         192 // i4DaylightProb
                     }
                 },
                 // AWB non-neutral probability for spatial and temporal weighting look-up table (Max: 100; Min: 0)
                 {
                     //LV0   1    2    3     4     5     6     7     8     9     10    11    12    13    14    15    16    17    18
                     {  0,  33,  66,  100,  100,  100,  100,  100,  100,  100,  100,   70,   30,   20,   10,    0,    0,    0,    0}
                },
                // AWB daylight locus probability look-up table (Max: 100; Min: 0)
                {
                    //LV0    1     2     3     4     5     6     7     8     9     10    11    12    13    14   15   16   17   18
                    { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,   50,   25,   0,   0,   0,   0}, // Strobe
                    { 95,  95,  95,  95,  95,  95,  100,  100,  100,  100,  100,  100,  100,   75,   50,  25,   0,   0,   0}, // Tungsten
                    { 95,  95,  95,  95,  95,  95,  100,  100,  100,  100,  100,   75,   50,   25,   25,  25,   0,   0,   0}, // Warm fluorescent
                    { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,   95,   75,   50,   25,   25,  25,   0,   0,   0}, // Fluorescent
                    {  90,   90,   90,   90,   90,   90,   90,   90,   90,   90,   80,   55,   30,   30,   30,  30,   0,   0,   0}, // CWF
                    { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,   75,  50,  40,  30,  20}, // Daylight
                    { 100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,   75,   50,   25,   0,   0,   0,   0}, // Shade
                    {  90,   90,   90,   90,   90,   90,   90,   90,   90,   90,   80,   55,   30,   30,   30,  30,   0,   0,   0}  // Daylight fluorescent
                },
                // AWB tuning information
                {
                     0, // i4ProjCode
                     0, // i4Model
                     0, // i4Date
                     0, // i4Res0
                     0, // i4Res1
                     0, // i4Res2
                     0, // i4Res3
                     0, // i4Res4
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


