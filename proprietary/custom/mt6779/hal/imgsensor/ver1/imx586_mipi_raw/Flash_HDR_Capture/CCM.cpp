/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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
#include "camera_custom_nvram.h"
#include "camera_custom_isp_nvram.h"

#include "inc.h"

#define CCM_BASE                                                     CCM_CCM_BASE

const ISP_NVRAM_MULTI_CCM_STRUCT CCM_BASE[6] = {
    // IDX_0
    {
    
        .CCM_CT_valid_NUM = 5,
    
        .CCM_Coef = {1, 2, 2},
    
        .CCM_Reg =
        {
            {.set={//CT_00
                0x1F4202A1, 0x0000001D, 0x025F1F68, 0x00000039, 0x1E151FE9, 0x00000402
            }},
            {.set={//CT_01
                0x1F4202A1, 0x0000001D, 0x025F1F68, 0x00000039, 0x1E151FE9, 0x00000402
            }},
            {.set={//CT_02
                0x1E690375, 0x00000022, 0x025B1F8A, 0x0000001B, 0x1E750008, 0x00000383
            }},
            {.set={//CT_03
                0x1DE003F0, 0x00000030, 0x02881F58, 0x00000020, 0x1E960025, 0x00000345
            }},
            {.set={//CT_04
                0x1E7C034A, 0x0000003A, 0x029F1FAE, 0x00001FB3, 0x1EF90006, 0x00000301
            }},
            {.set={//CT_05
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_06
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_07
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_08
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_09
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }}
        },
    
        .AWBGain =
        {
            {//CT_00
                512,    // i4R
                512,    // i4G
               1886     // i4B
            },
            {//CT_01
                595,    // i4R
                512,    // i4G
               1565     // i4B
            },
            {//CT_02
                793,    // i4R
                512,    // i4G
               1336     // i4B
            },
            {//CT_03
                892,    // i4R
                512,    // i4G
               1338     // i4B
            },
            {//CT_04
               1096,    // i4R
                512,    // i4G
                850     // i4B
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
            },
            {//CT_08
                512,    // i4R
                512,    // i4G
                512     // i4B
            },
            {//CT_09
                512,    // i4R
                512,    // i4G
                512     // i4B
            }
        }
    },
    // IDX_1
    {
    
        .CCM_CT_valid_NUM = 5,
    
        .CCM_Coef = {1, 2, 2},
    
        .CCM_Reg =
        {
            {.set={//CT_00
                0x1F4202A1, 0x0000001D, 0x025F1F68, 0x00000039, 0x1E151FE9, 0x00000402
            }},
            {.set={//CT_01
                0x1F4202A1, 0x0000001D, 0x025F1F68, 0x00000039, 0x1E151FE9, 0x00000402
            }},
            {.set={//CT_02
                0x1E690375, 0x00000022, 0x025B1F8A, 0x0000001B, 0x1E750008, 0x00000383
            }},
            {.set={//CT_03
                0x1DE003F0, 0x00000030, 0x02881F58, 0x00000020, 0x1E960025, 0x00000345
            }},
            {.set={//CT_04
                0x1E7C034A, 0x0000003A, 0x029F1FAE, 0x00001FB3, 0x1EF90006, 0x00000301
            }},
            {.set={//CT_05
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_06
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_07
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_08
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_09
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }}
        },
    
        .AWBGain =
        {
            {//CT_00
                512,    // i4R
                512,    // i4G
               1886     // i4B
            },
            {//CT_01
                595,    // i4R
                512,    // i4G
               1565     // i4B
            },
            {//CT_02
                793,    // i4R
                512,    // i4G
               1336     // i4B
            },
            {//CT_03
                892,    // i4R
                512,    // i4G
               1338     // i4B
            },
            {//CT_04
               1096,    // i4R
                512,    // i4G
                850     // i4B
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
            },
            {//CT_08
                512,    // i4R
                512,    // i4G
                512     // i4B
            },
            {//CT_09
                512,    // i4R
                512,    // i4G
                512     // i4B
            }
        }
    },
    // IDX_2
    {
    
        .CCM_CT_valid_NUM = 5,
    
        .CCM_Coef = {1, 2, 2},
    
        .CCM_Reg =
        {
            {.set={//CT_00
                0x1F4202A1, 0x0000001D, 0x025F1F68, 0x00000039, 0x1E151FE9, 0x00000402
            }},
            {.set={//CT_01
                0x1F4202A1, 0x0000001D, 0x025F1F68, 0x00000039, 0x1E151FE9, 0x00000402
            }},
            {.set={//CT_02
                0x1E690375, 0x00000022, 0x025B1F8A, 0x0000001B, 0x1E750008, 0x00000383
            }},
            {.set={//CT_03
                0x1DE003F0, 0x00000030, 0x02881F58, 0x00000020, 0x1E960025, 0x00000345
            }},
            {.set={//CT_04
                0x1E7C034A, 0x0000003A, 0x029F1FAE, 0x00001FB3, 0x1EF90006, 0x00000301
            }},
            {.set={//CT_05
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_06
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_07
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_08
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_09
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }}
        },
    
        .AWBGain =
        {
            {//CT_00
                512,    // i4R
                512,    // i4G
               1886     // i4B
            },
            {//CT_01
                595,    // i4R
                512,    // i4G
               1565     // i4B
            },
            {//CT_02
                793,    // i4R
                512,    // i4G
               1336     // i4B
            },
            {//CT_03
                892,    // i4R
                512,    // i4G
               1338     // i4B
            },
            {//CT_04
               1096,    // i4R
                512,    // i4G
                850     // i4B
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
            },
            {//CT_08
                512,    // i4R
                512,    // i4G
                512     // i4B
            },
            {//CT_09
                512,    // i4R
                512,    // i4G
                512     // i4B
            }
        }
    },
    // IDX_3
    {
    
        .CCM_CT_valid_NUM = 5,
    
        .CCM_Coef = {1, 2, 2},
    
        .CCM_Reg =
        {
            {.set={//CT_00
                0x1F4202A1, 0x0000001D, 0x025F1F68, 0x00000039, 0x1E151FE9, 0x00000402
            }},
            {.set={//CT_01
                0x1F4202A1, 0x0000001D, 0x025F1F68, 0x00000039, 0x1E151FE9, 0x00000402
            }},
            {.set={//CT_02
                0x1E690375, 0x00000022, 0x025B1F8A, 0x0000001B, 0x1E750008, 0x00000383
            }},
            {.set={//CT_03
                0x1DE003F0, 0x00000030, 0x02881F58, 0x00000020, 0x1E960025, 0x00000345
            }},
            {.set={//CT_04
                0x1E7C034A, 0x0000003A, 0x029F1FAE, 0x00001FB3, 0x1EF90006, 0x00000301
            }},
            {.set={//CT_05
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_06
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_07
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_08
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_09
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }}
        },
    
        .AWBGain =
        {
            {//CT_00
                512,    // i4R
                512,    // i4G
               1886     // i4B
            },
            {//CT_01
                595,    // i4R
                512,    // i4G
               1565     // i4B
            },
            {//CT_02
                793,    // i4R
                512,    // i4G
               1336     // i4B
            },
            {//CT_03
                892,    // i4R
                512,    // i4G
               1338     // i4B
            },
            {//CT_04
               1096,    // i4R
                512,    // i4G
                850     // i4B
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
            },
            {//CT_08
                512,    // i4R
                512,    // i4G
                512     // i4B
            },
            {//CT_09
                512,    // i4R
                512,    // i4G
                512     // i4B
            }
        }
    },
    // IDX_4
    {
    
        .CCM_CT_valid_NUM = 5,
    
        .CCM_Coef = {1, 2, 2},
    
        .CCM_Reg =
        {
            {.set={//CT_00
                0x1F4202A1, 0x0000001D, 0x025F1F68, 0x00000039, 0x1E151FE9, 0x00000402
            }},
            {.set={//CT_01
                0x1F4202A1, 0x0000001D, 0x025F1F68, 0x00000039, 0x1E151FE9, 0x00000402
            }},
            {.set={//CT_02
                0x1E690375, 0x00000022, 0x025B1F8A, 0x0000001B, 0x1E750008, 0x00000383
            }},
            {.set={//CT_03
                0x1DE003F0, 0x00000030, 0x02881F58, 0x00000020, 0x1E960025, 0x00000345
            }},
            {.set={//CT_04
                0x1E7C034A, 0x0000003A, 0x029F1FAE, 0x00001FB3, 0x1EF90006, 0x00000301
            }},
            {.set={//CT_05
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_06
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_07
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_08
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_09
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }}
        },
    
        .AWBGain =
        {
            {//CT_00
                512,    // i4R
                512,    // i4G
               1886     // i4B
            },
            {//CT_01
                595,    // i4R
                512,    // i4G
               1565     // i4B
            },
            {//CT_02
                793,    // i4R
                512,    // i4G
               1336     // i4B
            },
            {//CT_03
                892,    // i4R
                512,    // i4G
               1338     // i4B
            },
            {//CT_04
               1096,    // i4R
                512,    // i4G
                850     // i4B
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
            },
            {//CT_08
                512,    // i4R
                512,    // i4G
                512     // i4B
            },
            {//CT_09
                512,    // i4R
                512,    // i4G
                512     // i4B
            }
        }
    },
    // IDX_5
    {
    
        .CCM_CT_valid_NUM = 5,
    
        .CCM_Coef = {1, 2, 2},
    
        .CCM_Reg =
        {
            {.set={//CT_00
                0x1F4202A1, 0x0000001D, 0x025F1F68, 0x00000039, 0x1E151FE9, 0x00000402
            }},
            {.set={//CT_01
                0x1F4202A1, 0x0000001D, 0x025F1F68, 0x00000039, 0x1E151FE9, 0x00000402
            }},
            {.set={//CT_02
                0x1E690375, 0x00000022, 0x025B1F8A, 0x0000001B, 0x1E750008, 0x00000383
            }},
            {.set={//CT_03
                0x1DE003F0, 0x00000030, 0x02881F58, 0x00000020, 0x1E960025, 0x00000345
            }},
            {.set={//CT_04
                0x1E7C034A, 0x0000003A, 0x029F1FAE, 0x00001FB3, 0x1EF90006, 0x00000301
            }},
            {.set={//CT_05
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_06
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_07
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_08
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }},
            {.set={//CT_09
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }}
        },
    
        .AWBGain =
        {
            {//CT_00
                512,    // i4R
                512,    // i4G
               1886     // i4B
            },
            {//CT_01
                595,    // i4R
                512,    // i4G
               1565     // i4B
            },
            {//CT_02
                793,    // i4R
                512,    // i4G
               1336     // i4B
            },
            {//CT_03
                892,    // i4R
                512,    // i4G
               1338     // i4B
            },
            {//CT_04
               1096,    // i4R
                512,    // i4G
                850     // i4B
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
            },
            {//CT_08
                512,    // i4R
                512,    // i4G
                512     // i4B
            },
            {//CT_09
                512,    // i4R
                512,    // i4G
                512     // i4B
            }
        }
    },
};
