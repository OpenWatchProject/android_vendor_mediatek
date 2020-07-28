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
#include "camera_custom_nvram.h"
#include "s5k2t7spmipiraw_Flash_HDR_Capture.h"

const ISP_NVRAM_MULTI_CCM_STRUCT s5k2t7spmipiraw_CCM_0036 = {

    .CCM_CT_valid_NUM = 5,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x1FBE0231, 0x00000011, 0x02241F9C, 0x00000040, 0x1DFB0011, 0x000003F4
        }},
        {.set={//CT_01
            0x1FBE0231, 0x00000011, 0x02241F9C, 0x00000040, 0x1DFB0011, 0x000003F4
        }},
        {.set={//CT_02
            0x1F3B02E2, 0x00001FE3, 0x02471FB1, 0x00000008, 0x1E540058, 0x00000354
        }},
        {.set={//CT_03
            0x1E8D036D, 0x00000006, 0x024B1F8F, 0x00000026, 0x1E4D004A, 0x00000369
        }},
        {.set={//CT_04
            0x1E8A036E, 0x00000008, 0x02821FDD, 0x00001FA1, 0x1DFB0062, 0x000003A3
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
            522,  // i4R
            512,  // i4G
            1477  // i4B
        },
        {//CT_01
            654,  // i4R
            512,  // i4G
            1241  // i4B
        },
        {//CT_02
            960,  // i4R
            512,  // i4G
            985  // i4B
        },
        {//CT_03
            1096,  // i4R
            512,  // i4G
            1074  // i4B
        },
        {//CT_04
            1327,  // i4R
            512,  // i4G
            653  // i4B
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
};
const ISP_NVRAM_MULTI_CCM_STRUCT s5k2t7spmipiraw_CCM_0037 = {

    .CCM_CT_valid_NUM = 5,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x1FBE0231, 0x00000011, 0x02241F9C, 0x00000040, 0x1DFB0011, 0x000003F4
        }},
        {.set={//CT_01
            0x1FBE0231, 0x00000011, 0x02241F9C, 0x00000040, 0x1DFB0011, 0x000003F4
        }},
        {.set={//CT_02
            0x1F3B02E2, 0x00001FE3, 0x02471FB1, 0x00000008, 0x1E540058, 0x00000354
        }},
        {.set={//CT_03
            0x1E8D036D, 0x00000006, 0x024B1F8F, 0x00000026, 0x1E4D004A, 0x00000369
        }},
        {.set={//CT_04
            0x1E8A036E, 0x00000008, 0x02821FDD, 0x00001FA1, 0x1DFB0062, 0x000003A3
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
            522,  // i4R
            512,  // i4G
            1477  // i4B
        },
        {//CT_01
            654,  // i4R
            512,  // i4G
            1241  // i4B
        },
        {//CT_02
            960,  // i4R
            512,  // i4G
            985  // i4B
        },
        {//CT_03
            1096,  // i4R
            512,  // i4G
            1074  // i4B
        },
        {//CT_04
            1327,  // i4R
            512,  // i4G
            653  // i4B
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
};
const ISP_NVRAM_MULTI_CCM_STRUCT s5k2t7spmipiraw_CCM_0038 = {

    .CCM_CT_valid_NUM = 5,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x1FBE0231, 0x00000011, 0x02241F9C, 0x00000040, 0x1DFB0011, 0x000003F4
        }},
        {.set={//CT_01
            0x1FBE0231, 0x00000011, 0x02241F9C, 0x00000040, 0x1DFB0011, 0x000003F4
        }},
        {.set={//CT_02
            0x1F3B02E2, 0x00001FE3, 0x02471FB1, 0x00000008, 0x1E540058, 0x00000354
        }},
        {.set={//CT_03
            0x1E8D036D, 0x00000006, 0x024B1F8F, 0x00000026, 0x1E4D004A, 0x00000369
        }},
        {.set={//CT_04
            0x1E8A036E, 0x00000008, 0x02821FDD, 0x00001FA1, 0x1DFB0062, 0x000003A3
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
            522,  // i4R
            512,  // i4G
            1477  // i4B
        },
        {//CT_01
            654,  // i4R
            512,  // i4G
            1241  // i4B
        },
        {//CT_02
            960,  // i4R
            512,  // i4G
            985  // i4B
        },
        {//CT_03
            1096,  // i4R
            512,  // i4G
            1074  // i4B
        },
        {//CT_04
            1327,  // i4R
            512,  // i4G
            653  // i4B
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
};
const ISP_NVRAM_MULTI_CCM_STRUCT s5k2t7spmipiraw_CCM_0039 = {

    .CCM_CT_valid_NUM = 5,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x1FBE0231, 0x00000011, 0x02241F9C, 0x00000040, 0x1DFB0011, 0x000003F4
        }},
        {.set={//CT_01
            0x1FBE0231, 0x00000011, 0x02241F9C, 0x00000040, 0x1DFB0011, 0x000003F4
        }},
        {.set={//CT_02
            0x1F3B02E2, 0x00001FE3, 0x02471FB1, 0x00000008, 0x1E540058, 0x00000354
        }},
        {.set={//CT_03
            0x1E8D036D, 0x00000006, 0x024B1F8F, 0x00000026, 0x1E4D004A, 0x00000369
        }},
        {.set={//CT_04
            0x1E8A036E, 0x00000008, 0x02821FDD, 0x00001FA1, 0x1DFB0062, 0x000003A3
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
            522,  // i4R
            512,  // i4G
            1477  // i4B
        },
        {//CT_01
            654,  // i4R
            512,  // i4G
            1241  // i4B
        },
        {//CT_02
            960,  // i4R
            512,  // i4G
            985  // i4B
        },
        {//CT_03
            1096,  // i4R
            512,  // i4G
            1074  // i4B
        },
        {//CT_04
            1327,  // i4R
            512,  // i4G
            653  // i4B
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
};
const ISP_NVRAM_MULTI_CCM_STRUCT s5k2t7spmipiraw_CCM_0040 = {

    .CCM_CT_valid_NUM = 5,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x1FBE0231, 0x00000011, 0x02241F9C, 0x00000040, 0x1DFB0011, 0x000003F4
        }},
        {.set={//CT_01
            0x1FBE0231, 0x00000011, 0x02241F9C, 0x00000040, 0x1DFB0011, 0x000003F4
        }},
        {.set={//CT_02
            0x1F3B02E2, 0x00001FE3, 0x02471FB1, 0x00000008, 0x1E540058, 0x00000354
        }},
        {.set={//CT_03
            0x1E8D036D, 0x00000006, 0x024B1F8F, 0x00000026, 0x1E4D004A, 0x00000369
        }},
        {.set={//CT_04
            0x1E8A036E, 0x00000008, 0x02821FDD, 0x00001FA1, 0x1DFB0062, 0x000003A3
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
            522,  // i4R
            512,  // i4G
            1477  // i4B
        },
        {//CT_01
            654,  // i4R
            512,  // i4G
            1241  // i4B
        },
        {//CT_02
            960,  // i4R
            512,  // i4G
            985  // i4B
        },
        {//CT_03
            1096,  // i4R
            512,  // i4G
            1074  // i4B
        },
        {//CT_04
            1327,  // i4R
            512,  // i4G
            653  // i4B
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
};
const ISP_NVRAM_MULTI_CCM_STRUCT s5k2t7spmipiraw_CCM_0041 = {

    .CCM_CT_valid_NUM = 5,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x1FBE0231, 0x00000011, 0x02241F9C, 0x00000040, 0x1DFB0011, 0x000003F4
        }},
        {.set={//CT_01
            0x1FBE0231, 0x00000011, 0x02241F9C, 0x00000040, 0x1DFB0011, 0x000003F4
        }},
        {.set={//CT_02
            0x1F3B02E2, 0x00001FE3, 0x02471FB1, 0x00000008, 0x1E540058, 0x00000354
        }},
        {.set={//CT_03
            0x1E8D036D, 0x00000006, 0x024B1F8F, 0x00000026, 0x1E4D004A, 0x00000369
        }},
        {.set={//CT_04
            0x1E8A036E, 0x00000008, 0x02821FDD, 0x00001FA1, 0x1DFB0062, 0x000003A3
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
            522,  // i4R
            512,  // i4G
            1477  // i4B
        },
        {//CT_01
            654,  // i4R
            512,  // i4G
            1241  // i4B
        },
        {//CT_02
            960,  // i4R
            512,  // i4G
            985  // i4B
        },
        {//CT_03
            1096,  // i4R
            512,  // i4G
            1074  // i4B
        },
        {//CT_04
            1327,  // i4R
            512,  // i4G
            653  // i4B
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
};
