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
#include "hm1062_2mipimono_Scene_Capture.h"

const ISP_NVRAM_LCE_TUNING_PARAM_T hm1062_2mipimono_LCE_0000 = {
    .i4AutoHDREnable =
        0,

    .rAutoLCEParam = {
        .rLCEStats = {
            30,   //i4HistCCLB
            1600, //i4HistCCUB
            0,   //i4LCESeg
            0, //i4LCEContrastRatio
            0,  //i4LCEHistYThr
            51,  //SlopeLow --> 0.2*256
            614, //SlopeHigh --> 2.4*256
            384, //SlopeTop --> 1.5*256
            20,  //i4DetailRangeRatio --> base 1000
            64,  //i4CenSlopeMin

            0,    //i4vBlacKeep
            1600, //i4vFixedPt1
            2100, //i4vFixedPt2
            2800, //i4vFixedPt3
            3450, //i4vFixedPt4
            1000, //i4TCPLB
            20000,//I4TCPUB

            10,    //i4Stats0
            2000,  //i4Stats1
            2400,  //i4Stats2
            153,   //i4Stats3
            1,     //i4Stats4
        },

        .rLCELUTs = {//i4LCETbl
            {//  /*                            Dark    Strangth                                   */  |  /*   Bright    Strength   */
             //   LV0   LV1   LV2   LV3   LV4   LV5   LV6   LV7   LV8   LV9   LV10  LV11  LV12  LV13  |  LV2   LV4   LV6   LV8   LV10
                 {602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,     682,  682,  682,  682,  682},   //  0 DR index
                 {602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,     682,  682,  682,  682,  682},   //  1
                 {602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,     682,  682,  682,  682,  682},   //  2
                 {602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,     682,  682,  682,  682,  682},   //  3
                 {602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,     682,  682,  682,  682,  682},   //  4
                 {602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,     682,  682,  682,  682,  682},   //  5
                 {602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,     682,  682,  682,  682,  682},   //  6
                 {602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,     682,  682,  682,  682,  682},   //  7
                 {602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,     682,  682,  682,  682,  682},   //  8
                 {602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,     682,  682,  682,  682,  682},   //  9
                 {602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,  602,     682,  682,  682,  682,  682}    // 10
            },

            {    //i4CenSlopeEnhance
                //  0   1   2   3   4   5   6   7   8   9   10   11
                    0,  5, 10, 15, 20, 25, 20, 15, 10,  5,   0,   0
            }

        },

        .rLCEPara = {
        //      LV0    LV1    LV2    LV3    LV4    LV5    LV6    LV7    LV8    LV9   LV10   LV11   LV12   LV13   LV14   LV15   LV16   LV17   LV18
            {  1024,  1024,  1024,  1194,  1364,  1534,  1704,  1874,  2048,  2048,  2048,  2048,  2048,  2048,  2048,  2048,  2048,  2048,  2048}, //0 LVTarget
            {   950,   950,   940,   930,   920,   910,   900,   890,   880,   880,   880,   880,   880,   880,   880,   880,   880,   880,   880}, //1 BriRatio
            {  3400,  3400,  3400,   400,   390,  3380,  3370,  3360,  3350,  3350,  3350,  3350,  3350,  3350,  3350,  3350,  3350,  3350,  3350}, //2 BriLimit
            {  2600,  2600,  2600,  2600,  2642,  2684,  2726,  2770,  2812,  2854,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900,  2900}, //3 FlatBriTH
            { 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000}, //4 FlatHiBound
            {  7000,  7000,  7000,  7000,  7000,  7166,  7332,  7498,  7664,  7830,  8000,  8000,  8000,  8000,  8000,  8000,  8000,  8000,  8000}, //5 FlatLoBound
            {    85,    85,    85,    85,    85,    85,    85,    85,    85,    85,    85,    85,    85,    85,    85,    85,    85,    85,    85}, //6 LumaHiBoundRatio
            {    70,    70,    70,    70,    70,    70,    70,    70,    70,    70,    70,    70,    70,    70,    70,    70,    70,    70,    70}, //7 LumaLoBoundRatio
            {     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0}, //i4LCEPara8
            {     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0}, //i4LCEPara9
            {     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0}  //i4LCEPara10

        },

        .rFaceLCE = {    //i4FaceLCEPara
            1,   //m_bLCE_FD_Enable
            1,   //m_bKeepBrightEnable
            154, //m_i4keepBrightSlope
            89,  //m_u4LoBoundGainRatio
            128, //m_u4HiBoundGainRatio
            83,  //m_u4BrightTH
            308, //MaxLCEGain_L
            308, //MaxLCEGain_H
            70,  //m_u4FaceLV_L
            110, //m_u4FaceLV_H
            182, //m_u4FDProb_retio
            0,   //face para0
            0,   //face para1
            0,   //face para2
            0,   //face para3
            0,   //face para4
            0,   //face para5
            0,   //face para6
            0,   //face para7
            0,   //face para8
            0,   //face para9
            0,   //face para10


             //   LV0   LV1   LV2   LV3   LV4   LV5   LV6   LV7   LV8   LV9   LV10  LV11  LV12  LV13  LV14  LV15  LV16  LV17  LV18
            {512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512},   //  i4LCEfaceTbl0
            {512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512,  512},   //  i4LCEfaceTbl1


            {    //i4CenSlopeEnhance
                //  0   1   2   3   4   5   6   7   8   9   10   11
                    0,  5, 10, 15, 20, 25, 20, 15, 10,  5,   0,   0
            }
        },

        .rLCESmooth = {  // rLCESmooth
            70,      // i4LCEPosSpeed
            3,      // i4WaitAEStable
            3       // i4LCESpeed4AEStable
        },
        .rLCEFlare = {  // rLCEFlare
            0   // i4Enable
        },
    },
};
const ISP_NVRAM_GMA_STRUCT_T hm1062_2mipimono_GMA_0000 = {
  .GGM_Reg = {
        {.set={
            0x00000000, 0x00200802, 0x00401004, 0x00601806, 0x00802008, 0x00A0280A, 0x00C0300C, 0x00E0380E, 0x01004010, 0x01204812,
            0x01405014, 0x01605816, 0x01806018, 0x01A0681A, 0x01C0701C, 0x01E0781E, 0x02008020, 0x02208822, 0x02409024, 0x02609826,
            0x0280A028, 0x02A0A82A, 0x02C0B02C, 0x02E0B82E, 0x0300C030, 0x0320C832, 0x0340D034, 0x0360D836, 0x0380E038, 0x03A0E83A,
            0x03C0F03C, 0x03E0F83E, 0x04010040, 0x04210842, 0x04411044, 0x04611846, 0x04812048, 0x04A1284A, 0x04C1304C, 0x04E1384E,
            0x05014050, 0x05214852, 0x05415054, 0x05615856, 0x05816058, 0x05A1685A, 0x05C1705C, 0x05E1785E, 0x06018060, 0x06218862,
            0x06419064, 0x06619866, 0x0681A068, 0x06A1A86A, 0x06C1B06C, 0x06E1B86E, 0x0701C070, 0x0721C872, 0x0741D074, 0x0761D876,
            0x0781E078, 0x07A1E87A, 0x07C1F07C, 0x07E1F87E, 0x08020080, 0x08421084, 0x08822088, 0x08C2308C, 0x09024090, 0x09425094,
            0x09826098, 0x09C2709C, 0x0A0280A0, 0x0A4290A4, 0x0A82A0A8, 0x0AC2B0AC, 0x0B02C0B0, 0x0B42D0B4, 0x0B82E0B8, 0x0BC2F0BC,
            0x0C0300C0, 0x0C4310C4, 0x0C8320C8, 0x0CC330CC, 0x0D0340D0, 0x0D4350D4, 0x0D8360D8, 0x0DC370DC, 0x0E0380E0, 0x0E4390E4,
            0x0E83A0E8, 0x0EC3B0EC, 0x0F03C0F0, 0x0F43D0F4, 0x0F83E0F8, 0x0FC3F0FC, 0x10040100, 0x10842108, 0x11044110, 0x11846118,
            0x12048120, 0x1284A128, 0x1304C130, 0x1384E138, 0x14050140, 0x14852148, 0x15054150, 0x15856158, 0x16058160, 0x1685A168,
            0x1705C170, 0x1785E178, 0x18060180, 0x18862188, 0x19064190, 0x19866198, 0x1A0681A0, 0x1A86A1A8, 0x1B06C1B0, 0x1B86E1B8,
            0x1C0701C0, 0x1C8721C8, 0x1D0741D0, 0x1D8761D8, 0x1E0781E0, 0x1E87A1E8, 0x1F07C1F0, 0x1F87E1F8, 0x20080200, 0x20882208,
            0x21084210, 0x21886218, 0x22088220, 0x2288A228, 0x2308C230, 0x2388E238, 0x24090240, 0x24892248, 0x25094250, 0x25896258,
            0x26098260, 0x2689A268, 0x2709C270, 0x2789E278, 0x280A0280, 0x288A2288, 0x290A4290, 0x298A6298, 0x2A0A82A0, 0x2A8AA2A8,
            0x2B0AC2B0, 0x2B8AE2B8, 0x2C0B02C0, 0x2C8B22C8, 0x2D0B42D0, 0x2D8B62D8, 0x2E0B82E0, 0x2E8BA2E8, 0x2F0BC2F0, 0x2F8BE2F8,
            0x300C0300, 0x308C2308, 0x310C4310, 0x318C6318, 0x320C8320, 0x328CA328, 0x330CC330, 0x338CE338, 0x340D0340, 0x348D2348,
            0x350D4350, 0x358D6358, 0x360D8360, 0x368DA368, 0x370DC370, 0x378DE378, 0x380E0380, 0x388E2388, 0x390E4390, 0x398E6398,
            0x3A0E83A0, 0x3A8EA3A8, 0x3B0EC3B0, 0x3B8EE3B8, 0x3C0F03C0, 0x3C8F23C8, 0x3D0F43D0, 0x3D8F63D8, 0x3E0F83E0, 0x3E8FA3E8,
            0x3F0FC3F0, 0x3F8FE3F8
    }},
        {.set={
            0x00000000, 0x00200802, 0x00401004, 0x00601806, 0x00802008, 0x00A0280A, 0x00C0300C, 0x00E0380E, 0x01004010, 0x01204812,
            0x01405014, 0x01605816, 0x01806018, 0x01A0681A, 0x01C0701C, 0x01E0781E, 0x02008020, 0x02208822, 0x02409024, 0x02609826,
            0x0280A028, 0x02A0A82A, 0x02C0B02C, 0x02E0B82E, 0x0300C030, 0x0320C832, 0x0340D034, 0x0360D836, 0x0380E038, 0x03A0E83A,
            0x03C0F03C, 0x03E0F83E, 0x04010040, 0x04210842, 0x04411044, 0x04611846, 0x04812048, 0x04A1284A, 0x04C1304C, 0x04E1384E,
            0x05014050, 0x05214852, 0x05415054, 0x05615856, 0x05816058, 0x05A1685A, 0x05C1705C, 0x05E1785E, 0x06018060, 0x06218862,
            0x06419064, 0x06619866, 0x0681A068, 0x06A1A86A, 0x06C1B06C, 0x06E1B86E, 0x0701C070, 0x0721C872, 0x0741D074, 0x0761D876,
            0x0781E078, 0x07A1E87A, 0x07C1F07C, 0x07E1F87E, 0x08020080, 0x08421084, 0x08822088, 0x08C2308C, 0x09024090, 0x09425094,
            0x09826098, 0x09C2709C, 0x0A0280A0, 0x0A4290A4, 0x0A82A0A8, 0x0AC2B0AC, 0x0B02C0B0, 0x0B42D0B4, 0x0B82E0B8, 0x0BC2F0BC,
            0x0C0300C0, 0x0C4310C4, 0x0C8320C8, 0x0CC330CC, 0x0D0340D0, 0x0D4350D4, 0x0D8360D8, 0x0DC370DC, 0x0E0380E0, 0x0E4390E4,
            0x0E83A0E8, 0x0EC3B0EC, 0x0F03C0F0, 0x0F43D0F4, 0x0F83E0F8, 0x0FC3F0FC, 0x10040100, 0x10842108, 0x11044110, 0x11846118,
            0x12048120, 0x1284A128, 0x1304C130, 0x1384E138, 0x14050140, 0x14852148, 0x15054150, 0x15856158, 0x16058160, 0x1685A168,
            0x1705C170, 0x1785E178, 0x18060180, 0x18862188, 0x19064190, 0x19866198, 0x1A0681A0, 0x1A86A1A8, 0x1B06C1B0, 0x1B86E1B8,
            0x1C0701C0, 0x1C8721C8, 0x1D0741D0, 0x1D8761D8, 0x1E0781E0, 0x1E87A1E8, 0x1F07C1F0, 0x1F87E1F8, 0x20080200, 0x20882208,
            0x21084210, 0x21886218, 0x22088220, 0x2288A228, 0x2308C230, 0x2388E238, 0x24090240, 0x24892248, 0x25094250, 0x25896258,
            0x26098260, 0x2689A268, 0x2709C270, 0x2789E278, 0x280A0280, 0x288A2288, 0x290A4290, 0x298A6298, 0x2A0A82A0, 0x2A8AA2A8,
            0x2B0AC2B0, 0x2B8AE2B8, 0x2C0B02C0, 0x2C8B22C8, 0x2D0B42D0, 0x2D8B62D8, 0x2E0B82E0, 0x2E8BA2E8, 0x2F0BC2F0, 0x2F8BE2F8,
            0x300C0300, 0x308C2308, 0x310C4310, 0x318C6318, 0x320C8320, 0x328CA328, 0x330CC330, 0x338CE338, 0x340D0340, 0x348D2348,
            0x350D4350, 0x358D6358, 0x360D8360, 0x368DA368, 0x370DC370, 0x378DE378, 0x380E0380, 0x388E2388, 0x390E4390, 0x398E6398,
            0x3A0E83A0, 0x3A8EA3A8, 0x3B0EC3B0, 0x3B8EE3B8, 0x3C0F03C0, 0x3C8F23C8, 0x3D0F43D0, 0x3D8F63D8, 0x3E0F83E0, 0x3E8FA3E8,
            0x3F0FC3F0, 0x3F8FE3F8
    }},
        {.set={
			0x00000000, 0x00200802, 0x00401004, 0x00601806, 0x00802008, 0x00A0280A, 0x00C0300C, 0x00E0380E, 0x01004010, 0x01204812,
            0x01405014, 0x01605816, 0x01806018, 0x01A0681A, 0x01C0701C, 0x01E0781E, 0x02008020, 0x02208822, 0x02409024, 0x02609826,
            0x0280A028, 0x02A0A82A, 0x02C0B02C, 0x02E0B82E, 0x0300C030, 0x0320C832, 0x0340D034, 0x0360D836, 0x0380E038, 0x03A0E83A,
            0x03C0F03C, 0x03E0F83E, 0x04010040, 0x04210842, 0x04411044, 0x04611846, 0x04812048, 0x04A1284A, 0x04C1304C, 0x04E1384E,
            0x05014050, 0x05214852, 0x05415054, 0x05615856, 0x05816058, 0x05A1685A, 0x05C1705C, 0x05E1785E, 0x06018060, 0x06218862,
            0x06419064, 0x06619866, 0x0681A068, 0x06A1A86A, 0x06C1B06C, 0x06E1B86E, 0x0701C070, 0x0721C872, 0x0741D074, 0x0761D876,
            0x0781E078, 0x07A1E87A, 0x07C1F07C, 0x07E1F87E, 0x08020080, 0x08421084, 0x08822088, 0x08C2308C, 0x09024090, 0x09425094,
            0x09826098, 0x09C2709C, 0x0A0280A0, 0x0A4290A4, 0x0A82A0A8, 0x0AC2B0AC, 0x0B02C0B0, 0x0B42D0B4, 0x0B82E0B8, 0x0BC2F0BC,
            0x0C0300C0, 0x0C4310C4, 0x0C8320C8, 0x0CC330CC, 0x0D0340D0, 0x0D4350D4, 0x0D8360D8, 0x0DC370DC, 0x0E0380E0, 0x0E4390E4,
            0x0E83A0E8, 0x0EC3B0EC, 0x0F03C0F0, 0x0F43D0F4, 0x0F83E0F8, 0x0FC3F0FC, 0x10040100, 0x10842108, 0x11044110, 0x11846118,
            0x12048120, 0x1284A128, 0x1304C130, 0x1384E138, 0x14050140, 0x14852148, 0x15054150, 0x15856158, 0x16058160, 0x1685A168,
            0x1705C170, 0x1785E178, 0x18060180, 0x18862188, 0x19064190, 0x19866198, 0x1A0681A0, 0x1A86A1A8, 0x1B06C1B0, 0x1B86E1B8,
            0x1C0701C0, 0x1C8721C8, 0x1D0741D0, 0x1D8761D8, 0x1E0781E0, 0x1E87A1E8, 0x1F07C1F0, 0x1F87E1F8, 0x20080200, 0x20882208,
            0x21084210, 0x21886218, 0x22088220, 0x2288A228, 0x2308C230, 0x2388E238, 0x24090240, 0x24892248, 0x25094250, 0x25896258,
            0x26098260, 0x2689A268, 0x2709C270, 0x2789E278, 0x280A0280, 0x288A2288, 0x290A4290, 0x298A6298, 0x2A0A82A0, 0x2A8AA2A8,
            0x2B0AC2B0, 0x2B8AE2B8, 0x2C0B02C0, 0x2C8B22C8, 0x2D0B42D0, 0x2D8B62D8, 0x2E0B82E0, 0x2E8BA2E8, 0x2F0BC2F0, 0x2F8BE2F8,
            0x300C0300, 0x308C2308, 0x310C4310, 0x318C6318, 0x320C8320, 0x328CA328, 0x330CC330, 0x338CE338, 0x340D0340, 0x348D2348,
            0x350D4350, 0x358D6358, 0x360D8360, 0x368DA368, 0x370DC370, 0x378DE378, 0x380E0380, 0x388E2388, 0x390E4390, 0x398E6398,
            0x3A0E83A0, 0x3A8EA3A8, 0x3B0EC3B0, 0x3B8EE3B8, 0x3C0F03C0, 0x3C8F23C8, 0x3D0F43D0, 0x3D8F63D8, 0x3E0F83E0, 0x3E8FA3E8,
            0x3F0FC3F0, 0x3F8FE3F8
    }},
        {.set={
			0x00000000, 0x00200802, 0x00401004, 0x00601806, 0x00802008, 0x00A0280A, 0x00C0300C, 0x00E0380E, 0x01004010, 0x01204812,
            0x01405014, 0x01605816, 0x01806018, 0x01A0681A, 0x01C0701C, 0x01E0781E, 0x02008020, 0x02208822, 0x02409024, 0x02609826,
            0x0280A028, 0x02A0A82A, 0x02C0B02C, 0x02E0B82E, 0x0300C030, 0x0320C832, 0x0340D034, 0x0360D836, 0x0380E038, 0x03A0E83A,
            0x03C0F03C, 0x03E0F83E, 0x04010040, 0x04210842, 0x04411044, 0x04611846, 0x04812048, 0x04A1284A, 0x04C1304C, 0x04E1384E,
            0x05014050, 0x05214852, 0x05415054, 0x05615856, 0x05816058, 0x05A1685A, 0x05C1705C, 0x05E1785E, 0x06018060, 0x06218862,
            0x06419064, 0x06619866, 0x0681A068, 0x06A1A86A, 0x06C1B06C, 0x06E1B86E, 0x0701C070, 0x0721C872, 0x0741D074, 0x0761D876,
            0x0781E078, 0x07A1E87A, 0x07C1F07C, 0x07E1F87E, 0x08020080, 0x08421084, 0x08822088, 0x08C2308C, 0x09024090, 0x09425094,
            0x09826098, 0x09C2709C, 0x0A0280A0, 0x0A4290A4, 0x0A82A0A8, 0x0AC2B0AC, 0x0B02C0B0, 0x0B42D0B4, 0x0B82E0B8, 0x0BC2F0BC,
            0x0C0300C0, 0x0C4310C4, 0x0C8320C8, 0x0CC330CC, 0x0D0340D0, 0x0D4350D4, 0x0D8360D8, 0x0DC370DC, 0x0E0380E0, 0x0E4390E4,
            0x0E83A0E8, 0x0EC3B0EC, 0x0F03C0F0, 0x0F43D0F4, 0x0F83E0F8, 0x0FC3F0FC, 0x10040100, 0x10842108, 0x11044110, 0x11846118,
            0x12048120, 0x1284A128, 0x1304C130, 0x1384E138, 0x14050140, 0x14852148, 0x15054150, 0x15856158, 0x16058160, 0x1685A168,
            0x1705C170, 0x1785E178, 0x18060180, 0x18862188, 0x19064190, 0x19866198, 0x1A0681A0, 0x1A86A1A8, 0x1B06C1B0, 0x1B86E1B8,
            0x1C0701C0, 0x1C8721C8, 0x1D0741D0, 0x1D8761D8, 0x1E0781E0, 0x1E87A1E8, 0x1F07C1F0, 0x1F87E1F8, 0x20080200, 0x20882208,
            0x21084210, 0x21886218, 0x22088220, 0x2288A228, 0x2308C230, 0x2388E238, 0x24090240, 0x24892248, 0x25094250, 0x25896258,
            0x26098260, 0x2689A268, 0x2709C270, 0x2789E278, 0x280A0280, 0x288A2288, 0x290A4290, 0x298A6298, 0x2A0A82A0, 0x2A8AA2A8,
            0x2B0AC2B0, 0x2B8AE2B8, 0x2C0B02C0, 0x2C8B22C8, 0x2D0B42D0, 0x2D8B62D8, 0x2E0B82E0, 0x2E8BA2E8, 0x2F0BC2F0, 0x2F8BE2F8,
            0x300C0300, 0x308C2308, 0x310C4310, 0x318C6318, 0x320C8320, 0x328CA328, 0x330CC330, 0x338CE338, 0x340D0340, 0x348D2348,
            0x350D4350, 0x358D6358, 0x360D8360, 0x368DA368, 0x370DC370, 0x378DE378, 0x380E0380, 0x388E2388, 0x390E4390, 0x398E6398,
            0x3A0E83A0, 0x3A8EA3A8, 0x3B0EC3B0, 0x3B8EE3B8, 0x3C0F03C0, 0x3C8F23C8, 0x3D0F43D0, 0x3D8F63D8, 0x3E0F83E0, 0x3E8FA3E8,
            0x3F0FC3F0, 0x3F8FE3F8
    }}
  },
  .rGmaParam =  {
     // Normal Preview
       eISP_DYNAMIC_GMA_MODE,  // eGMAMode
       8,                  // i4LowContrastThr
       80,                  // i4LowContrastRatio
       3,                  // i4LowContrastSeg
       {
           {   // i4ContrastWeightingTbl
               //  0   1   2    3    4    5    6    7    8    9    10
                  50, 80, 100, 100, 100, 100, 100, 100, 100, 100, 100
           },
           {   // i4LVWeightingTbl
               //LV0   1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
                   0,  0,  0,  0,  0,  0,  0,  0, 33, 66, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100
               //  0,  0,  0,  0,  0, 33, 33, 33, 33, 33,  80,  100, 100, 100, 100, 100, 100, 100, 100, 100
           },
           {   // i4NightContrastWtTbl
               //  0   1   2    3    4    5    6    7    8    9    10
                //  50, 50, 50, 50,  50, 30, 20,  15,   0,   0,   0
             85, 85, 75, 50,  50, 30, 20,  15,   0,   0,   0
           },
           {   // i4NightLVWtTbl
               //LV0   1    2    3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19
               //  50,  50,  50,  40, 40, 40, 30, 30,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0
                  100, 100, 100, 100, 80, 50, 20, 0,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0
           }
       },
       {
           1,      // i4Enable
           0,      // i4WaitAEStable
           4       // i4Speed
       },
       {
           0,      // i4Enable
           9202,   // i4CenterPt
           50,     // i4LowPercent
           100000, // i4LowCurve100
           100000, // i4HighCurve100
           50,     // i4HighPercent
           100,    // i4SlopeH100
           100     // i4SlopeL100
       },
       {
           0       // rGMAFlare.i4Enable
       }
    }
};
const ISP_NVRAM_DCE_TUNING_PARAM_T hm1062_2mipimono_DCE_0000 = {
    .rDceParam=
    {
        30, // SmoothDCESpeed

        { //rDCELUTs
            { //i4DCETbl1
              // DarkStrength                                 BrightStrength
              // LV0  LV2  LV4  LV6  LV8  LV10 LV12 LV14 LV16 LV0  LV2  LV4  LV6  LV8  LV10 LV12 LV14 LV16  TBD
                { 30,  30,  30,  30,  30,  30,  30,  30,  30,  80,  80,  80,  80,  80,  80,  80,  80,  80,  100}, //  0 DR index : , BrightStrength
                { 30,  30,  30,  30,  30,  30,  30,  30,  30,  80,  80,  80,  80,  80,  80,  80,  80,  80,  100}, //  1
                { 30,  30,  30,  30,  30,  30,  30,  30,  30,  80,  80,  80,  80,  80,  80,  80,  80,  80,  100}, //  2
                { 30,  30,  30,  30,  30,  30,  30,  30,  30,  80,  80,  80,  80,  80,  80,  80,  80,  80,  100}, //  3
                { 30,  30,  30,  30,  30,  30,  30,  30,  30,  80,  80,  80,  80,  80,  80,  80,  80,  80,  100}, //  4
                { 30,  30,  30,  30,  30,  30,  30,  30,  30,  80,  80,  80,  80,  80,  80,  80,  80,  80,  100}, //  5
                { 30,  30,  30,  30,  30,  30,  30,  30,  30,  80,  80,  80,  80,  80,  80,  80,  80,  80,  100}, //  6
                { 30,  30,  30,  30,  30,  30,  30,  30,  30,  80,  80,  80,  80,  80,  80,  80,  80,  80,  100}, //  7
                { 30,  30,  30,  30,  30,  30,  30,  30,  30,  80,  80,  80,  80,  80,  80,  80,  80,  80,  100}, //  8
                { 30,  30,  30,  30,  30,  30,  30,  30,  30,  80,  80,  80,  80,  80,  80,  80,  80,  80,  100}, //  9
                { 30,  30,  30,  30,  30,  30,  30,  30,  30,  80,  80,  80,  80,  80,  80,  80,  80,  80,  100}, // 10
            },
            { //i4DCETbl2
              // LV0    LV2   LV4   LV6   LV8  LV10  LV12  LV14  LV16   LV0   LV2   LV4   LV6   LV8  LV10  LV12  LV14  LV16  TBD
				{   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 100}, // Face_DarkStrength : Face_BrightStrength
				{3300, 3300, 3300, 3300, 3300, 3300, 3300, 3300, 3300, 3500, 3500, 3500, 3500, 3500, 3500, 3500, 3500, 3500, 100}, // SkyDetectThr : SkyLimitThr
                {  20,   20,   20,   20,   20,   20,   20,   20,   20,   10,   10,   10,   10,   10,   10,   10,   10,   10, 100}, // SkyProtectOnThr : SkyProtectOffThr
                { 150,  150,  150,  150,  150,  150,  150,  150,  150,  80,  80,  80,  80,  80,  80,  80,  80,  80,  100}, // ContourLimitThr
                { 30,  30,  30,  30,  30,  30,  30,  30,  30,  80,  80,  80,  80,  80,  80,  80,  80,  80,  100}, // TBD
                { 30,  30,  30,  30,  30,  30,  30,  30,  30,  80,  80,  80,  80,  80,  80,  80,  80,  80,  100}, // TBD
                { 30,  30,  30,  30,  30,  30,  30,  30,  30,  80,  80,  80,  80,  80,  80,  80,  80,  80,  100}, // TBD
                { 30,  30,  30,  30,  30,  30,  30,  30,  30,  80,  80,  80,  80,  80,  80,  80,  80,  80,  100}, // TBD
                { 30,  30,  30,  30,  30,  30,  30,  30,  30,  80,  80,  80,  80,  80,  80,  80,  80,  80,  100}, // TBD
                { 30,  30,  30,  30,  30,  30,  30,  30,  30,  80,  80,  80,  80,  80,  80,  80,  80,  80,  100}, // TBD
                { 30,  30,  30,  30,  30,  30,  30,  30,  30,  80,  80,  80,  80,  80,  80,  80,  80,  80,  100}, // TBD              
            }
        }
    }
};
