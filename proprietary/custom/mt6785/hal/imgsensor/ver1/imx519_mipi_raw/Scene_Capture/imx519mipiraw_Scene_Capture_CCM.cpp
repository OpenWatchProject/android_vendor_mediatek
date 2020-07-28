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
#include "imx519mipiraw_Scene_Capture.h"

const ISP_NVRAM_MULTI_CCM_STRUCT imx519mipiraw_CCM_0000 = {
    .CCM_CT_valid_NUM = 5,
    .CCM_Coef = {1, 2, 2},
    .CCM_Reg=
    {
        {.set={//CT_00
            0x1FE8024C, 0x00001FCC, 0x02001FAE, 0x00000052, 0x1E6E1FF4, 0x0000039E
        }},
        {.set={//CT_01
            0x1FA00271, 0x00001FEF, 0x02021FB7, 0x00000047, 0x1E970019, 0x00000350
        }},
        {.set={//CT_02
            0x1F07032D, 0x00001FCC, 0x02531FB4, 0x00001FF9, 0x1EFE0023, 0x000002DF
        }},
        {.set={//CT_03
            0x1E3A03BE, 0x00000008, 0x02311FA0, 0x0000002F, 0x1EE9002D, 0x000002EA
        }},
        {.set={//CT_04
            0x1E950380, 0x00001FEB, 0x02531FD0, 0x00001FDD, 0x1ECB0028, 0x0000030D
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
    .AWBGain=
    {
        {//CT_00
            499,  // i4R
            512,  // i4G
            2031  // i4B
        },
        {//CT_01
            624,  // i4R
            512,  // i4G
            1629  // i4B
        },
        {//CT_02
            829,  // i4R
            512,  // i4G
            1253  // i4B
        },
        {//CT_03
            992,  // i4R
            512,  // i4G
            1417  // i4B
        },
        {//CT_04
            1173,  // i4R
            512,  // i4G
            834  // i4B
        },
        {//CT_05
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_06
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_07
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_08
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_09
            512,  // i4R
            512,  // i4G
            512  // i4B
        }
    },

};
const ISP_NVRAM_MULTI_CCM_STRUCT imx519mipiraw_CCM_0001 = {
    .CCM_CT_valid_NUM = 5,
    .CCM_Coef = {1, 2, 2},
    .CCM_Reg=
    {
        {.set={//CT_00
            0x1FE8024C, 0x00001FCC, 0x02001FAE, 0x00000052, 0x1E6E1FF4, 0x0000039E
        }},
        {.set={//CT_01
            0x1FA00271, 0x00001FEF, 0x02021FB7, 0x00000047, 0x1E970019, 0x00000350
        }},
        {.set={//CT_02
            0x1F07032D, 0x00001FCC, 0x02531FB4, 0x00001FF9, 0x1EFE0023, 0x000002DF
        }},
        {.set={//CT_03
            0x1E3A03BE, 0x00000008, 0x02311FA0, 0x0000002F, 0x1EE9002D, 0x000002EA
        }},
        {.set={//CT_04
            0x1E950380, 0x00001FEB, 0x02531FD0, 0x00001FDD, 0x1ECB0028, 0x0000030D
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
    .AWBGain=
    {
        {//CT_00
            499,  // i4R
            512,  // i4G
            2031  // i4B
        },
        {//CT_01
            624,  // i4R
            512,  // i4G
            1629  // i4B
        },
        {//CT_02
            829,  // i4R
            512,  // i4G
            1253  // i4B
        },
        {//CT_03
            992,  // i4R
            512,  // i4G
            1417  // i4B
        },
        {//CT_04
            1173,  // i4R
            512,  // i4G
            834  // i4B
        },
        {//CT_05
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_06
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_07
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_08
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_09
            512,  // i4R
            512,  // i4G
            512  // i4B
        }
    },

};
const ISP_NVRAM_MULTI_CCM_STRUCT imx519mipiraw_CCM_0002 = {
    .CCM_CT_valid_NUM = 5,
    .CCM_Coef = {1, 2, 2},
    .CCM_Reg=
    {
        {.set={//CT_00
            0x1FE8024C, 0x00001FCC, 0x02001FAE, 0x00000052, 0x1E6E1FF4, 0x0000039E
        }},
        {.set={//CT_01
            0x1FA00271, 0x00001FEF, 0x02021FB7, 0x00000047, 0x1E970019, 0x00000350
        }},
        {.set={//CT_02
            0x1F07032D, 0x00001FCC, 0x02531FB4, 0x00001FF9, 0x1EFE0023, 0x000002DF
        }},
        {.set={//CT_03
            0x1E3A03BE, 0x00000008, 0x02311FA0, 0x0000002F, 0x1EE9002D, 0x000002EA
        }},
        {.set={//CT_04
            0x1E950380, 0x00001FEB, 0x02531FD0, 0x00001FDD, 0x1ECB0028, 0x0000030D
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
    .AWBGain=
    {
        {//CT_00
            499,  // i4R
            512,  // i4G
            2031  // i4B
        },
        {//CT_01
            624,  // i4R
            512,  // i4G
            1629  // i4B
        },
        {//CT_02
            829,  // i4R
            512,  // i4G
            1253  // i4B
        },
        {//CT_03
            992,  // i4R
            512,  // i4G
            1417  // i4B
        },
        {//CT_04
            1173,  // i4R
            512,  // i4G
            834  // i4B
        },
        {//CT_05
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_06
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_07
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_08
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_09
            512,  // i4R
            512,  // i4G
            512  // i4B
        }
    },
};
const ISP_NVRAM_MULTI_CCM_STRUCT imx519mipiraw_CCM_0003 = {
    .CCM_CT_valid_NUM = 5,
    .CCM_Coef = {1, 2, 2},
    .CCM_Reg=
    {
        {.set={//CT_00
            0x1FE8024C, 0x00001FCC, 0x02001FAE, 0x00000052, 0x1E6E1FF4, 0x0000039E
        }},
        {.set={//CT_01
            0x1FA00271, 0x00001FEF, 0x02021FB7, 0x00000047, 0x1E970019, 0x00000350
        }},
        {.set={//CT_02
            0x1F07032D, 0x00001FCC, 0x02531FB4, 0x00001FF9, 0x1EFE0023, 0x000002DF
        }},
        {.set={//CT_03
            0x1E3A03BE, 0x00000008, 0x02311FA0, 0x0000002F, 0x1EE9002D, 0x000002EA
        }},
        {.set={//CT_04
            0x1E950380, 0x00001FEB, 0x02531FD0, 0x00001FDD, 0x1ECB0028, 0x0000030D
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
    .AWBGain=
    {
        {//CT_00
            499,  // i4R
            512,  // i4G
            2031  // i4B
        },
        {//CT_01
            624,  // i4R
            512,  // i4G
            1629  // i4B
        },
        {//CT_02
            829,  // i4R
            512,  // i4G
            1253  // i4B
        },
        {//CT_03
            992,  // i4R
            512,  // i4G
            1417  // i4B
        },
        {//CT_04
            1173,  // i4R
            512,  // i4G
            834  // i4B
        },
        {//CT_05
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_06
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_07
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_08
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_09
            512,  // i4R
            512,  // i4G
            512  // i4B
        }
    },
};
const ISP_NVRAM_MULTI_CCM_STRUCT imx519mipiraw_CCM_0004 = {
    .CCM_CT_valid_NUM = 5,
    .CCM_Coef = {1, 2, 2},
    .CCM_Reg=
    {
        {.set={//CT_00
            0x1F550328, 0x00001F83, 0x026D1F8B, 0x00000008, 0x1ECF1FFD, 0x00000334
        }},
        {.set={//CT_01
            0x1F5C02E3, 0x00001FC1, 0x02621F79, 0x00000025, 0x1ED61FEE, 0x0000033C
        }},
        {.set={//CT_02
            0x1F3F02F0, 0x00001FD1, 0x02731F9E, 0x00001FEF, 0x1E760010, 0x0000037A
        }},
        {.set={//CT_03
            0x1F3A02F7, 0x00001FCF, 0x02971F91, 0x00001FD8, 0x1E9D0007, 0x0000035C
        }},
        {.set={//CT_04
            0x1E950380, 0x00001FEB, 0x02531FD0, 0x00001FDD, 0x1ECB0028, 0x0000030D
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
    .AWBGain=
    {
        {//CT_00
            499,  // i4R
            512,  // i4G
            2031  // i4B
        },
        {//CT_01
            624,  // i4R
            512,  // i4G
            1629  // i4B
        },
        {//CT_02
            829,  // i4R
            512,  // i4G
            1253  // i4B
        },
        {//CT_03
            992,  // i4R
            512,  // i4G
            1417  // i4B
        },
        {//CT_04
            1173,  // i4R
            512,  // i4G
            834  // i4B
        },
        {//CT_05
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_06
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_07
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_08
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_09
            512,  // i4R
            512,  // i4G
            512  // i4B
        }
    },
};
const ISP_NVRAM_MULTI_CCM_STRUCT imx519mipiraw_CCM_0005 = {
    .CCM_CT_valid_NUM = 5,
    .CCM_Coef = {1, 2, 2},
    .CCM_Reg=
    {
        {.set={//CT_00
            0x1F550328, 0x00001F83, 0x026D1F8B, 0x00000008, 0x1ECF1FFD, 0x00000334
        }},
        {.set={//CT_01
            0x1F5C02E3, 0x00001FC1, 0x02621F79, 0x00000025, 0x1ED61FEE, 0x0000033C
        }},
        {.set={//CT_02
            0x1F3F02F0, 0x00001FD1, 0x02731F9E, 0x00001FEF, 0x1E760010, 0x0000037A
        }},
        {.set={//CT_03
            0x1F3A02F7, 0x00001FCF, 0x02971F91, 0x00001FD8, 0x1E9D0007, 0x0000035C
        }},
        {.set={//CT_04
            0x1E950380, 0x00001FEB, 0x02531FD0, 0x00001FDD, 0x1ECB0028, 0x0000030D
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
    .AWBGain=
    {
        {//CT_00
            499,  // i4R
            512,  // i4G
            2031  // i4B
        },
        {//CT_01
            624,  // i4R
            512,  // i4G
            1629  // i4B
        },
        {//CT_02
            829,  // i4R
            512,  // i4G
            1253  // i4B
        },
        {//CT_03
            992,  // i4R
            512,  // i4G
            1417  // i4B
        },
        {//CT_04
            1173,  // i4R
            512,  // i4G
            834  // i4B
        },
        {//CT_05
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_06
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_07
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_08
            512,  // i4R
            512,  // i4G
            512  // i4B
        },
        {//CT_09
            512,  // i4R
            512,  // i4G
            512  // i4B
        }
    }
};
