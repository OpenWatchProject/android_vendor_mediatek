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
#include "s5k3p8sxmipiraw_Face_Capture.h"

const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8sxmipiraw_CCM_0006 = {

    .CCM_CT_valid_NUM = 6,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x1FAE027F, 0x00001FD3, 0x02531FBB, 0x00001FF2, 0x1EBD1FF8, 0x0000034B
        }},
        {.set={//CT_01
            0x1F7C02B1, 0x00001FD3, 0x02531FBB, 0x00001FF2, 0x1EBD1FF8, 0x0000034B
        }},
        {.set={//CT_02
            0x1E8D03C2, 0x00001FB1, 0x02C41F87, 0x00001FB5, 0x1EC9000B, 0x0000032C
        }},
        {.set={//CT_03
            0x1E2503DB, 0x00000000, 0x02901F78, 0x00001FF8, 0x1EE90001, 0x00000316
        }},
        {.set={//CT_04
            0x1ECE034E, 0x00001FE4, 0x02BD1FD2, 0x00001F71, 0x1F000008, 0x000002F8
        }},
        {.set={//CT_05
            0x1E92038A, 0x00001FE4, 0x02BD1FD2, 0x00001F71, 0x1ED80008, 0x00000320
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
            480,  // i4R
            512,    // i4G
            1700  // i4B
        },
        {//CT_01
            681,  // i4R
            512,    // i4G
            1207  // i4B
        },
        {//CT_02
            978,  // i4R
            512,    // i4G
            915   // i4B
        },
        {//CT_03
           1200,  // i4R
            512,    // i4G
           1400   // i4B
        },
        {//CT_04
           1342,  // i4R
            512,    // i4G
            647  // i4B
        },
        {//CT_05
            1422,  // i4R
            512,    // i4G
             600  // i4B
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
const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8sxmipiraw_CCM_0007 = {

    .CCM_CT_valid_NUM = 6,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x1FAE027F, 0x00001FD3, 0x02531FBB, 0x00001FF2, 0x1EBD1FF8, 0x0000034B
        }},
        {.set={//CT_01
            0x1F7C02B1, 0x00001FD3, 0x02531FBB, 0x00001FF2, 0x1EBD1FF8, 0x0000034B
        }},
        {.set={//CT_02
            0x1E8D03C2, 0x00001FB1, 0x02C41F87, 0x00001FB5, 0x1EC9000B, 0x0000032C
        }},
        {.set={//CT_03
            0x1E2503DB, 0x00000000, 0x02901F78, 0x00001FF8, 0x1EE90001, 0x00000316
        }},
        {.set={//CT_04
            0x1ECE034E, 0x00001FE4, 0x02BD1FD2, 0x00001F71, 0x1F000008, 0x000002F8
        }},
        {.set={//CT_05
            0x1E92038A, 0x00001FE4, 0x02BD1FD2, 0x00001F71, 0x1ED80008, 0x00000320
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
            480,  // i4R
            512,    // i4G
            1700  // i4B
        },
        {//CT_01
            681,  // i4R
            512,    // i4G
            1207  // i4B
        },
        {//CT_02
            978,  // i4R
            512,    // i4G
            915   // i4B
        },
        {//CT_03
           1200,  // i4R
            512,    // i4G
           1400   // i4B
        },
        {//CT_04
           1342,  // i4R
            512,    // i4G
            647  // i4B
        },
        {//CT_05
            1422,  // i4R
            512,    // i4G
             600  // i4B
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
const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8sxmipiraw_CCM_0008 = {

    .CCM_CT_valid_NUM = 6,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x1FAE027F, 0x00001FD3, 0x02531FBB, 0x00001FF2, 0x1EBD1FF8, 0x0000034B
        }},
        {.set={//CT_01
            0x1F7C02B1, 0x00001FD3, 0x02531FBB, 0x00001FF2, 0x1EBD1FF8, 0x0000034B
        }},
        {.set={//CT_02
            0x1E8D03C2, 0x00001FB1, 0x02C41F87, 0x00001FB5, 0x1EC9000B, 0x0000032C
        }},
        {.set={//CT_03
            0x1E2503DB, 0x00000000, 0x02901F78, 0x00001FF8, 0x1EE90001, 0x00000316
        }},
        {.set={//CT_04
            0x1ECE034E, 0x00001FE4, 0x02BD1FD2, 0x00001F71, 0x1F000008, 0x000002F8
        }},
        {.set={//CT_05
            0x1E92038A, 0x00001FE4, 0x02BD1FD2, 0x00001F71, 0x1ED80008, 0x00000320
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
            480,  // i4R
            512,    // i4G
            1700  // i4B
        },
        {//CT_01
            681,  // i4R
            512,    // i4G
            1207  // i4B
        },
        {//CT_02
            978,  // i4R
            512,    // i4G
            915   // i4B
        },
        {//CT_03
           1200,  // i4R
            512,    // i4G
           1400   // i4B
        },
        {//CT_04
           1342,  // i4R
            512,    // i4G
            647  // i4B
        },
        {//CT_05
            1422,  // i4R
            512,    // i4G
             600  // i4B
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
const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8sxmipiraw_CCM_0009 = {

    .CCM_CT_valid_NUM = 6,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x1FAE027F, 0x00001FD3, 0x02531FBB, 0x00001FF2, 0x1EBD1FF8, 0x0000034B
        }},
        {.set={//CT_01
            0x1F7C02B1, 0x00001FD3, 0x02531FBB, 0x00001FF2, 0x1EBD1FF8, 0x0000034B
        }},
        {.set={//CT_02
            0x1E8D03C2, 0x00001FB1, 0x02C41F87, 0x00001FB5, 0x1EC9000B, 0x0000032C
        }},
        {.set={//CT_03
            0x1E2503DB, 0x00000000, 0x02901F78, 0x00001FF8, 0x1EE90001, 0x00000316
        }},
        {.set={//CT_04
            0x1ECE034E, 0x00001FE4, 0x02BD1FD2, 0x00001F71, 0x1F000008, 0x000002F8
        }},
        {.set={//CT_05
            0x1E92038A, 0x00001FE4, 0x02BD1FD2, 0x00001F71, 0x1ED80008, 0x00000320
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
            480,  // i4R
            512,    // i4G
            1700  // i4B
        },
        {//CT_01
            681,  // i4R
            512,    // i4G
            1207  // i4B
        },
        {//CT_02
            978,  // i4R
            512,    // i4G
            915   // i4B
        },
        {//CT_03
           1200,  // i4R
            512,    // i4G
           1400   // i4B
        },
        {//CT_04
           1342,  // i4R
            512,    // i4G
            647  // i4B
        },
        {//CT_05
            1422,  // i4R
            512,    // i4G
             600  // i4B
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
const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8sxmipiraw_CCM_0010 = {

    .CCM_CT_valid_NUM = 6,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x1FAE027F, 0x00001FD3, 0x02531FBB, 0x00001FF2, 0x1EBD1FF8, 0x0000034B
        }},
        {.set={//CT_01
            0x1F7C02B1, 0x00001FD3, 0x02531FBB, 0x00001FF2, 0x1EBD1FF8, 0x0000034B
        }},
        {.set={//CT_02
            0x1E8D03C2, 0x00001FB1, 0x02C41F87, 0x00001FB5, 0x1EC9000B, 0x0000032C
        }},
        {.set={//CT_03
            0x1E2503DB, 0x00000000, 0x02901F78, 0x00001FF8, 0x1EE90001, 0x00000316
        }},
        {.set={//CT_04
            0x1ECE034E, 0x00001FE4, 0x02BD1FD2, 0x00001F71, 0x1F000008, 0x000002F8
        }},
        {.set={//CT_05
            0x1E92038A, 0x00001FE4, 0x02BD1FD2, 0x00001F71, 0x1ED80008, 0x00000320
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
            480,  // i4R
            512,    // i4G
            1700  // i4B
        },
        {//CT_01
            681,  // i4R
            512,    // i4G
            1207  // i4B
        },
        {//CT_02
            978,  // i4R
            512,    // i4G
            915   // i4B
        },
        {//CT_03
           1200,  // i4R
            512,    // i4G
           1400   // i4B
        },
        {//CT_04
           1342,  // i4R
            512,    // i4G
            647  // i4B
        },
        {//CT_05
            1422,  // i4R
            512,    // i4G
             600  // i4B
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
const ISP_NVRAM_MULTI_CCM_STRUCT s5k3p8sxmipiraw_CCM_0011 = {

    .CCM_CT_valid_NUM = 6,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x1FAE027F, 0x00001FD3, 0x02531FBB, 0x00001FF2, 0x1EBD1FF8, 0x0000034B
        }},
        {.set={//CT_01
            0x1F7C02B1, 0x00001FD3, 0x02531FBB, 0x00001FF2, 0x1EBD1FF8, 0x0000034B
        }},
        {.set={//CT_02
            0x1E8D03C2, 0x00001FB1, 0x02C41F87, 0x00001FB5, 0x1EC9000B, 0x0000032C
        }},
        {.set={//CT_03
            0x1E2503DB, 0x00000000, 0x02901F78, 0x00001FF8, 0x1EE90001, 0x00000316
        }},
        {.set={//CT_04
            0x1ECE034E, 0x00001FE4, 0x02BD1FD2, 0x00001F71, 0x1F000008, 0x000002F8
        }},
        {.set={//CT_05
            0x1E92038A, 0x00001FE4, 0x02BD1FD2, 0x00001F71, 0x1ED80008, 0x00000320
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
            480,  // i4R
            512,    // i4G
            1700  // i4B
        },
        {//CT_01
            681,  // i4R
            512,    // i4G
            1207  // i4B
        },
        {//CT_02
            978,  // i4R
            512,    // i4G
            915   // i4B
        },
        {//CT_03
           1200,  // i4R
            512,    // i4G
           1400   // i4B
        },
        {//CT_04
           1342,  // i4R
            512,    // i4G
            647  // i4B
        },
        {//CT_05
            1422,  // i4R
            512,    // i4G
             600  // i4B
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
