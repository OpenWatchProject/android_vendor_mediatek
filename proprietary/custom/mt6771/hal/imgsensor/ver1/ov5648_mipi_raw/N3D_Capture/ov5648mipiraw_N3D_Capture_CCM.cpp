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
#include "ov5648mipiraw_N3D_Capture.h"

const ISP_NVRAM_MULTI_CCM_STRUCT ov5648mipiraw_CCM_0024 = {

    .CCM_CT_valid_NUM = 5,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x1E9D0206, 0x0000015D, 0x02011F85, 0x0000007A, 0x1C080033, 0x000005C5
        }},
        {.set={//CT_01
            0x1EF802C1, 0x00000047, 0x02261F82, 0x00000058, 0x1D1F0042, 0x0000049F
        }},
        {.set={//CT_02
            0x1EC70356, 0x00001FE3, 0x022C1F7A, 0x0000005A, 0x1E12002B, 0x000003C3
        }},
        {.set={//CT_03
            0x1DB60467, 0x00001FE3, 0x02011F88, 0x00000077, 0x1DBE005D, 0x000003E5
        }},
        {.set={//CT_04
            0x1E41042A, 0x00001F95, 0x024A1FC0, 0x00001FF6, 0x1E230072, 0x0000036B
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
            390,  // i4R
            512,  // i4G
            1173  // i4B
        },
        {//CT_01
            453,  // i4R
            512,  // i4G
            1071  // i4B
        },
        {//CT_02
            586,  // i4R
            512,  // i4G
            911  // i4B
        },
        {//CT_03
            688,  // i4R
            512,  // i4G
            964  // i4B
        },
        {//CT_04
            731,  // i4R
            512,  // i4G
            569  // i4B
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
const ISP_NVRAM_MULTI_CCM_STRUCT ov5648mipiraw_CCM_0025 = {

    .CCM_CT_valid_NUM = 5,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x1E9D0206, 0x0000015D, 0x02011F85, 0x0000007A, 0x1C080033, 0x000005C5
        }},
        {.set={//CT_01
            0x1EF802C1, 0x00000047, 0x02261F82, 0x00000058, 0x1D1F0042, 0x0000049F
        }},
        {.set={//CT_02
            0x1EC70356, 0x00001FE3, 0x022C1F7A, 0x0000005A, 0x1E12002B, 0x000003C3
        }},
        {.set={//CT_03
            0x1DB60467, 0x00001FE3, 0x02011F88, 0x00000077, 0x1DBE005D, 0x000003E5
        }},
        {.set={//CT_04
            0x1E41042A, 0x00001F95, 0x024A1FC0, 0x00001FF6, 0x1E230072, 0x0000036B
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
            390,  // i4R
            512,  // i4G
            1173  // i4B
        },
        {//CT_01
            453,  // i4R
            512,  // i4G
            1071  // i4B
        },
        {//CT_02
            586,  // i4R
            512,  // i4G
            911  // i4B
        },
        {//CT_03
            688,  // i4R
            512,  // i4G
            964  // i4B
        },
        {//CT_04
            731,  // i4R
            512,  // i4G
            569  // i4B
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
const ISP_NVRAM_MULTI_CCM_STRUCT ov5648mipiraw_CCM_0026 = {

    .CCM_CT_valid_NUM = 5,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x1E9D0206, 0x0000015D, 0x02011F85, 0x0000007A, 0x1C080033, 0x000005C5
        }},
        {.set={//CT_01
            0x1EF802C1, 0x00000047, 0x02261F82, 0x00000058, 0x1D1F0042, 0x0000049F
        }},
        {.set={//CT_02
            0x1EC70356, 0x00001FE3, 0x022C1F7A, 0x0000005A, 0x1E12002B, 0x000003C3
        }},
        {.set={//CT_03
            0x1DB60467, 0x00001FE3, 0x02011F88, 0x00000077, 0x1DBE005D, 0x000003E5
        }},
        {.set={//CT_04
            0x1E41042A, 0x00001F95, 0x024A1FC0, 0x00001FF6, 0x1E230072, 0x0000036B
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
            390,  // i4R
            512,  // i4G
            1173  // i4B
        },
        {//CT_01
            453,  // i4R
            512,  // i4G
            1071  // i4B
        },
        {//CT_02
            586,  // i4R
            512,  // i4G
            911  // i4B
        },
        {//CT_03
            688,  // i4R
            512,  // i4G
            964  // i4B
        },
        {//CT_04
            731,  // i4R
            512,  // i4G
            569  // i4B
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
const ISP_NVRAM_MULTI_CCM_STRUCT ov5648mipiraw_CCM_0027 = {

    .CCM_CT_valid_NUM = 5,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x1E9D0206, 0x0000015D, 0x02011F85, 0x0000007A, 0x1C080033, 0x000005C5
        }},
        {.set={//CT_01
            0x1EF802C1, 0x00000047, 0x02261F82, 0x00000058, 0x1D1F0042, 0x0000049F
        }},
        {.set={//CT_02
            0x1EC70356, 0x00001FE3, 0x022C1F7A, 0x0000005A, 0x1E12002B, 0x000003C3
        }},
        {.set={//CT_03
            0x1DB60467, 0x00001FE3, 0x02011F88, 0x00000077, 0x1DBE005D, 0x000003E5
        }},
        {.set={//CT_04
            0x1E41042A, 0x00001F95, 0x024A1FC0, 0x00001FF6, 0x1E230072, 0x0000036B
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
            390,  // i4R
            512,  // i4G
            1173  // i4B
        },
        {//CT_01
            453,  // i4R
            512,  // i4G
            1071  // i4B
        },
        {//CT_02
            586,  // i4R
            512,  // i4G
            911  // i4B
        },
        {//CT_03
            688,  // i4R
            512,  // i4G
            964  // i4B
        },
        {//CT_04
            731,  // i4R
            512,  // i4G
            569  // i4B
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
const ISP_NVRAM_MULTI_CCM_STRUCT ov5648mipiraw_CCM_0028 = {

    .CCM_CT_valid_NUM = 5,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x1E9D0206, 0x0000015D, 0x02011F85, 0x0000007A, 0x1C080033, 0x000005C5
        }},
        {.set={//CT_01
            0x1EF802C1, 0x00000047, 0x02261F82, 0x00000058, 0x1D1F0042, 0x0000049F
        }},
        {.set={//CT_02
            0x1EC70356, 0x00001FE3, 0x022C1F7A, 0x0000005A, 0x1E12002B, 0x000003C3
        }},
        {.set={//CT_03
            0x1DB60467, 0x00001FE3, 0x02011F88, 0x00000077, 0x1DBE005D, 0x000003E5
        }},
        {.set={//CT_04
            0x1E41042A, 0x00001F95, 0x024A1FC0, 0x00001FF6, 0x1E230072, 0x0000036B
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
            390,  // i4R
            512,  // i4G
            1173  // i4B
        },
        {//CT_01
            453,  // i4R
            512,  // i4G
            1071  // i4B
        },
        {//CT_02
            586,  // i4R
            512,  // i4G
            911  // i4B
        },
        {//CT_03
            688,  // i4R
            512,  // i4G
            964  // i4B
        },
        {//CT_04
            731,  // i4R
            512,  // i4G
            569  // i4B
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
const ISP_NVRAM_MULTI_CCM_STRUCT ov5648mipiraw_CCM_0029 = {

    .CCM_CT_valid_NUM = 5,

    .CCM_Coef = {1, 2, 2},

    .CCM_Reg =
    {
        {.set={//CT_00
            0x1E9D0206, 0x0000015D, 0x02011F85, 0x0000007A, 0x1C080033, 0x000005C5
        }},
        {.set={//CT_01
            0x1EF802C1, 0x00000047, 0x02261F82, 0x00000058, 0x1D1F0042, 0x0000049F
        }},
        {.set={//CT_02
            0x1EC70356, 0x00001FE3, 0x022C1F7A, 0x0000005A, 0x1E12002B, 0x000003C3
        }},
        {.set={//CT_03
            0x1DB60467, 0x00001FE3, 0x02011F88, 0x00000077, 0x1DBE005D, 0x000003E5
        }},
        {.set={//CT_04
            0x1E41042A, 0x00001F95, 0x024A1FC0, 0x00001FF6, 0x1E230072, 0x0000036B
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
            390,  // i4R
            512,  // i4G
            1173  // i4B
        },
        {//CT_01
            453,  // i4R
            512,  // i4G
            1071  // i4B
        },
        {//CT_02
            586,  // i4R
            512,  // i4G
            911  // i4B
        },
        {//CT_03
            688,  // i4R
            512,  // i4G
            964  // i4B
        },
        {//CT_04
            731,  // i4R
            512,  // i4G
            569  // i4B
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
