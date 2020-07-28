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
#include "imx338mipiraw_HDR_Capture.h"

const ISP_NVRAM_MULTI_CCM_STRUCT imx338mipiraw_CCM_0018 = {

    .CCM_CT_valid_NUM = 5,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x00420404, 0x00001DBA, 0x02711F4E, 0x00000041, 0x1EEB1FAF, 0x00000366
        }},
        {.set={//CT_01
            0x1FDD03CE, 0x00001E55, 0x02881F68, 0x00000010, 0x1EDE1FF7, 0x0000032B
        }},
        {.set={//CT_02
            0x1F7D03A9, 0x00001EDA, 0x02401F69, 0x00000057, 0x1F4E1FFC, 0x000002B6
        }},
        {.set={//CT_03
            0x1F6903D4, 0x00001EC3, 0x02D61F03, 0x00000027, 0x1F520001, 0x000002AD
        }},
        {.set={//CT_04
            0x1F75033D, 0x00001F4E, 0x027F1F7D, 0x00000004, 0x1F171FF5, 0x000002F4
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
            488,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_01
            624,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_02
            910,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_03
           1029,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_04
           1184,    // i4R
            512,    // i4G
            512     // i4B
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
const ISP_NVRAM_MULTI_CCM_STRUCT imx338mipiraw_CCM_0019 = {

    .CCM_CT_valid_NUM = 5,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x00420404, 0x00001DBA, 0x02711F4E, 0x00000041, 0x1EEB1FAF, 0x00000366
        }},
        {.set={//CT_01
            0x1FDD03CE, 0x00001E55, 0x02881F68, 0x00000010, 0x1EDE1FF7, 0x0000032B
        }},
        {.set={//CT_02
            0x1F7D03A9, 0x00001EDA, 0x02401F69, 0x00000057, 0x1F4E1FFC, 0x000002B6
        }},
        {.set={//CT_03
            0x1F6903D4, 0x00001EC3, 0x02D61F03, 0x00000027, 0x1F520001, 0x000002AD
        }},
        {.set={//CT_04
            0x1F75033D, 0x00001F4E, 0x027F1F7D, 0x00000004, 0x1F171FF5, 0x000002F4
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
            488,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_01
            624,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_02
            910,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_03
           1029,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_04
           1184,    // i4R
            512,    // i4G
            512     // i4B
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
const ISP_NVRAM_MULTI_CCM_STRUCT imx338mipiraw_CCM_0020 = {

    .CCM_CT_valid_NUM = 5,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x00420404, 0x00001DBA, 0x02711F4E, 0x00000041, 0x1EEB1FAF, 0x00000366
        }},
        {.set={//CT_01
            0x1FDD03CE, 0x00001E55, 0x02881F68, 0x00000010, 0x1EDE1FF7, 0x0000032B
        }},
        {.set={//CT_02
            0x1F7D03A9, 0x00001EDA, 0x02401F69, 0x00000057, 0x1F4E1FFC, 0x000002B6
        }},
        {.set={//CT_03
            0x1F6903D4, 0x00001EC3, 0x02D61F03, 0x00000027, 0x1F520001, 0x000002AD
        }},
        {.set={//CT_04
            0x1F75033D, 0x00001F4E, 0x027F1F7D, 0x00000004, 0x1F171FF5, 0x000002F4
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
            488,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_01
            624,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_02
            910,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_03
           1029,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_04
           1184,    // i4R
            512,    // i4G
            512     // i4B
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
const ISP_NVRAM_MULTI_CCM_STRUCT imx338mipiraw_CCM_0021 = {

    .CCM_CT_valid_NUM = 5,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x00420404, 0x00001DBA, 0x02711F4E, 0x00000041, 0x1EEB1FAF, 0x00000366
        }},
        {.set={//CT_01
            0x1FDD03CE, 0x00001E55, 0x02881F68, 0x00000010, 0x1EDE1FF7, 0x0000032B
        }},
        {.set={//CT_02
            0x1F7D03A9, 0x00001EDA, 0x02401F69, 0x00000057, 0x1F4E1FFC, 0x000002B6
        }},
        {.set={//CT_03
            0x1F6903D4, 0x00001EC3, 0x02D61F03, 0x00000027, 0x1F520001, 0x000002AD
        }},
        {.set={//CT_04
            0x1F75033D, 0x00001F4E, 0x027F1F7D, 0x00000004, 0x1F171FF5, 0x000002F4
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
            488,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_01
            624,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_02
            910,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_03
           1029,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_04
           1184,    // i4R
            512,    // i4G
            512     // i4B
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
const ISP_NVRAM_MULTI_CCM_STRUCT imx338mipiraw_CCM_0022 = {

    .CCM_CT_valid_NUM = 5,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x00420404, 0x00001DBA, 0x02711F4E, 0x00000041, 0x1EEB1FAF, 0x00000366
        }},
        {.set={//CT_01
            0x1FDD03CE, 0x00001E55, 0x02881F68, 0x00000010, 0x1EDE1FF7, 0x0000032B
        }},
        {.set={//CT_02
            0x1F7D03A9, 0x00001EDA, 0x02401F69, 0x00000057, 0x1F4E1FFC, 0x000002B6
        }},
        {.set={//CT_03
            0x1F6903D4, 0x00001EC3, 0x02D61F03, 0x00000027, 0x1F520001, 0x000002AD
        }},
        {.set={//CT_04
            0x1F75033D, 0x00001F4E, 0x027F1F7D, 0x00000004, 0x1F171FF5, 0x000002F4
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
            488,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_01
            624,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_02
            910,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_03
           1029,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_04
           1184,    // i4R
            512,    // i4G
            512     // i4B
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
const ISP_NVRAM_MULTI_CCM_STRUCT imx338mipiraw_CCM_0023 = {

    .CCM_CT_valid_NUM = 5,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x00420404, 0x00001DBA, 0x02711F4E, 0x00000041, 0x1EEB1FAF, 0x00000366
        }},
        {.set={//CT_01
            0x1FDD03CE, 0x00001E55, 0x02881F68, 0x00000010, 0x1EDE1FF7, 0x0000032B
        }},
        {.set={//CT_02
            0x1F7D03A9, 0x00001EDA, 0x02401F69, 0x00000057, 0x1F4E1FFC, 0x000002B6
        }},
        {.set={//CT_03
            0x1F6903D4, 0x00001EC3, 0x02D61F03, 0x00000027, 0x1F520001, 0x000002AD
        }},
        {.set={//CT_04
            0x1F75033D, 0x00001F4E, 0x027F1F7D, 0x00000004, 0x1F171FF5, 0x000002F4
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
            488,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_01
            624,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_02
            910,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_03
           1029,    // i4R
            512,    // i4G
            512     // i4B
        },
        {//CT_04
           1184,    // i4R
            512,    // i4G
            512     // i4B
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
