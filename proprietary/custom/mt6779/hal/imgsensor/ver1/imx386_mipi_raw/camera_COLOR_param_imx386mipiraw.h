/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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
.COMM =
{
    .COLOR =
    {
        .COLOR_Method =
        {
            1,
        }
    },
    .CCM =
    {
        .dynamic_CCM=
        {
            {.set={
                0x1ECE034E, 0x00001FE4, 0x02BD1FBE, 0x00001F85, 0x1F000008, 0x000002F8
            }},
            {.set={
                0x1E8D03C2, 0x00001FB1, 0x02C41F87, 0x00001FB5, 0x1EC9000B, 0x0000032C
            }},
            {.set={
                0x1DF00417, 0x00001FDE, 0x02B01F7A, 0x00001FD6, 0x1ECE0012, 0x00000320
            }},
            {.set={
                0x1F4302FD, 0x00001FC0, 0x02761F9E, 0x00001FEC, 0x1E331FF4, 0x000003D9
            }}
        },

        .dynamic_CCM_AWBGain=
        {
            {
              1342,
              512,
              647
            },
            {
              978,
              512,
              915
            },
            {
              1114,
              512,
              1021
            },
            {
              681,
              512,
              1207
            }
        }
    }
},
.CCM = {
    imx386mipiraw_CCM_0000[0],imx386mipiraw_CCM_0000[1],imx386mipiraw_CCM_0000[2],imx386mipiraw_CCM_0000[3],imx386mipiraw_CCM_0000[4],imx386mipiraw_CCM_0000[5],imx386mipiraw_CCM_0006[0],imx386mipiraw_CCM_0006[1],imx386mipiraw_CCM_0006[2],imx386mipiraw_CCM_0006[3],
    imx386mipiraw_CCM_0006[4],imx386mipiraw_CCM_0006[5],imx386mipiraw_CCM_0012[0],imx386mipiraw_CCM_0012[1],imx386mipiraw_CCM_0012[2],imx386mipiraw_CCM_0012[3],imx386mipiraw_CCM_0012[4],imx386mipiraw_CCM_0012[5],imx386mipiraw_CCM_0018[0],imx386mipiraw_CCM_0018[1],
    imx386mipiraw_CCM_0018[2],imx386mipiraw_CCM_0018[3],imx386mipiraw_CCM_0018[4],imx386mipiraw_CCM_0018[5],imx386mipiraw_CCM_0024[0],imx386mipiraw_CCM_0024[1],imx386mipiraw_CCM_0024[2],imx386mipiraw_CCM_0024[3],imx386mipiraw_CCM_0024[4],imx386mipiraw_CCM_0024[5],
    imx386mipiraw_CCM_0030[0],imx386mipiraw_CCM_0030[1],imx386mipiraw_CCM_0030[2],imx386mipiraw_CCM_0030[3],imx386mipiraw_CCM_0030[4],imx386mipiraw_CCM_0030[5],imx386mipiraw_CCM_0036[0],imx386mipiraw_CCM_0036[1],imx386mipiraw_CCM_0036[2],imx386mipiraw_CCM_0036[3],
    imx386mipiraw_CCM_0036[4],imx386mipiraw_CCM_0036[5],
},
.COLOR_PARAM = {
    imx386mipiraw_COLOR_PARAM_0000[0],imx386mipiraw_COLOR_PARAM_0001[0],imx386mipiraw_COLOR_PARAM_0002[0],imx386mipiraw_COLOR_PARAM_0003[0],imx386mipiraw_COLOR_PARAM_0004[0],imx386mipiraw_COLOR_PARAM_0005[0],imx386mipiraw_COLOR_PARAM_0006[0],
},
.COLOR = {
    imx386mipiraw_COLOR_0000[0],imx386mipiraw_COLOR_0000[1],imx386mipiraw_COLOR_0000[2],imx386mipiraw_COLOR_0000[3],imx386mipiraw_COLOR_0000[4],imx386mipiraw_COLOR_0000[5],imx386mipiraw_COLOR_0000[6],imx386mipiraw_COLOR_0000[7],imx386mipiraw_COLOR_0000[8],imx386mipiraw_COLOR_0000[9],
    imx386mipiraw_COLOR_0000[10],imx386mipiraw_COLOR_0000[11],imx386mipiraw_COLOR_0000[12],imx386mipiraw_COLOR_0000[13],imx386mipiraw_COLOR_0000[14],imx386mipiraw_COLOR_0000[15],imx386mipiraw_COLOR_0000[16],imx386mipiraw_COLOR_0000[17],imx386mipiraw_COLOR_0000[18],imx386mipiraw_COLOR_0000[19],
    imx386mipiraw_COLOR_0000[20],imx386mipiraw_COLOR_0000[21],imx386mipiraw_COLOR_0000[22],imx386mipiraw_COLOR_0000[23],imx386mipiraw_COLOR_0000[24],imx386mipiraw_COLOR_0000[25],imx386mipiraw_COLOR_0000[26],imx386mipiraw_COLOR_0000[27],imx386mipiraw_COLOR_0000[28],imx386mipiraw_COLOR_0000[29],
    imx386mipiraw_COLOR_0000[30],imx386mipiraw_COLOR_0000[31],imx386mipiraw_COLOR_0000[32],imx386mipiraw_COLOR_0000[33],imx386mipiraw_COLOR_0000[34],imx386mipiraw_COLOR_0000[35],imx386mipiraw_COLOR_0000[36],imx386mipiraw_COLOR_0000[37],imx386mipiraw_COLOR_0000[38],imx386mipiraw_COLOR_0000[39],
    imx386mipiraw_COLOR_0000[40],imx386mipiraw_COLOR_0000[41],imx386mipiraw_COLOR_0000[42],imx386mipiraw_COLOR_0000[43],imx386mipiraw_COLOR_0000[44],imx386mipiraw_COLOR_0000[45],imx386mipiraw_COLOR_0000[46],imx386mipiraw_COLOR_0000[47],imx386mipiraw_COLOR_0000[48],imx386mipiraw_COLOR_0000[49],
    imx386mipiraw_COLOR_0000[50],imx386mipiraw_COLOR_0000[51],imx386mipiraw_COLOR_0000[52],imx386mipiraw_COLOR_0000[53],imx386mipiraw_COLOR_0000[54],imx386mipiraw_COLOR_0000[55],imx386mipiraw_COLOR_0000[56],imx386mipiraw_COLOR_0000[57],imx386mipiraw_COLOR_0000[58],imx386mipiraw_COLOR_0000[59],
    imx386mipiraw_COLOR_0060[0],imx386mipiraw_COLOR_0060[1],imx386mipiraw_COLOR_0060[2],imx386mipiraw_COLOR_0060[3],imx386mipiraw_COLOR_0060[4],imx386mipiraw_COLOR_0060[5],imx386mipiraw_COLOR_0060[6],imx386mipiraw_COLOR_0060[7],imx386mipiraw_COLOR_0060[8],imx386mipiraw_COLOR_0060[9],
    imx386mipiraw_COLOR_0060[10],imx386mipiraw_COLOR_0060[11],imx386mipiraw_COLOR_0060[12],imx386mipiraw_COLOR_0060[13],imx386mipiraw_COLOR_0060[14],imx386mipiraw_COLOR_0060[15],imx386mipiraw_COLOR_0060[16],imx386mipiraw_COLOR_0060[17],imx386mipiraw_COLOR_0060[18],imx386mipiraw_COLOR_0060[19],
    imx386mipiraw_COLOR_0060[20],imx386mipiraw_COLOR_0060[21],imx386mipiraw_COLOR_0060[22],imx386mipiraw_COLOR_0060[23],imx386mipiraw_COLOR_0060[24],imx386mipiraw_COLOR_0060[25],imx386mipiraw_COLOR_0060[26],imx386mipiraw_COLOR_0060[27],imx386mipiraw_COLOR_0060[28],imx386mipiraw_COLOR_0060[29],
    imx386mipiraw_COLOR_0060[30],imx386mipiraw_COLOR_0060[31],imx386mipiraw_COLOR_0060[32],imx386mipiraw_COLOR_0060[33],imx386mipiraw_COLOR_0060[34],imx386mipiraw_COLOR_0060[35],imx386mipiraw_COLOR_0060[36],imx386mipiraw_COLOR_0060[37],imx386mipiraw_COLOR_0060[38],imx386mipiraw_COLOR_0060[39],
    imx386mipiraw_COLOR_0060[40],imx386mipiraw_COLOR_0060[41],imx386mipiraw_COLOR_0060[42],imx386mipiraw_COLOR_0060[43],imx386mipiraw_COLOR_0060[44],imx386mipiraw_COLOR_0060[45],imx386mipiraw_COLOR_0060[46],imx386mipiraw_COLOR_0060[47],imx386mipiraw_COLOR_0060[48],imx386mipiraw_COLOR_0060[49],
    imx386mipiraw_COLOR_0060[50],imx386mipiraw_COLOR_0060[51],imx386mipiraw_COLOR_0060[52],imx386mipiraw_COLOR_0060[53],imx386mipiraw_COLOR_0060[54],imx386mipiraw_COLOR_0060[55],imx386mipiraw_COLOR_0060[56],imx386mipiraw_COLOR_0060[57],imx386mipiraw_COLOR_0060[58],imx386mipiraw_COLOR_0060[59],
    imx386mipiraw_COLOR_0120[0],imx386mipiraw_COLOR_0120[1],imx386mipiraw_COLOR_0120[2],imx386mipiraw_COLOR_0120[3],imx386mipiraw_COLOR_0120[4],imx386mipiraw_COLOR_0120[5],imx386mipiraw_COLOR_0120[6],imx386mipiraw_COLOR_0120[7],imx386mipiraw_COLOR_0120[8],imx386mipiraw_COLOR_0120[9],
    imx386mipiraw_COLOR_0120[10],imx386mipiraw_COLOR_0120[11],imx386mipiraw_COLOR_0120[12],imx386mipiraw_COLOR_0120[13],imx386mipiraw_COLOR_0120[14],imx386mipiraw_COLOR_0120[15],imx386mipiraw_COLOR_0120[16],imx386mipiraw_COLOR_0120[17],imx386mipiraw_COLOR_0120[18],imx386mipiraw_COLOR_0120[19],
    imx386mipiraw_COLOR_0120[20],imx386mipiraw_COLOR_0120[21],imx386mipiraw_COLOR_0120[22],imx386mipiraw_COLOR_0120[23],imx386mipiraw_COLOR_0120[24],imx386mipiraw_COLOR_0120[25],imx386mipiraw_COLOR_0120[26],imx386mipiraw_COLOR_0120[27],imx386mipiraw_COLOR_0120[28],imx386mipiraw_COLOR_0120[29],
    imx386mipiraw_COLOR_0120[30],imx386mipiraw_COLOR_0120[31],imx386mipiraw_COLOR_0120[32],imx386mipiraw_COLOR_0120[33],imx386mipiraw_COLOR_0120[34],imx386mipiraw_COLOR_0120[35],imx386mipiraw_COLOR_0120[36],imx386mipiraw_COLOR_0120[37],imx386mipiraw_COLOR_0120[38],imx386mipiraw_COLOR_0120[39],
    imx386mipiraw_COLOR_0120[40],imx386mipiraw_COLOR_0120[41],imx386mipiraw_COLOR_0120[42],imx386mipiraw_COLOR_0120[43],imx386mipiraw_COLOR_0120[44],imx386mipiraw_COLOR_0120[45],imx386mipiraw_COLOR_0120[46],imx386mipiraw_COLOR_0120[47],imx386mipiraw_COLOR_0120[48],imx386mipiraw_COLOR_0120[49],
    imx386mipiraw_COLOR_0120[50],imx386mipiraw_COLOR_0120[51],imx386mipiraw_COLOR_0120[52],imx386mipiraw_COLOR_0120[53],imx386mipiraw_COLOR_0120[54],imx386mipiraw_COLOR_0120[55],imx386mipiraw_COLOR_0120[56],imx386mipiraw_COLOR_0120[57],imx386mipiraw_COLOR_0120[58],imx386mipiraw_COLOR_0120[59],
    imx386mipiraw_COLOR_0180[0],imx386mipiraw_COLOR_0180[1],imx386mipiraw_COLOR_0180[2],imx386mipiraw_COLOR_0180[3],imx386mipiraw_COLOR_0180[4],imx386mipiraw_COLOR_0180[5],imx386mipiraw_COLOR_0180[6],imx386mipiraw_COLOR_0180[7],imx386mipiraw_COLOR_0180[8],imx386mipiraw_COLOR_0180[9],
    imx386mipiraw_COLOR_0180[10],imx386mipiraw_COLOR_0180[11],imx386mipiraw_COLOR_0180[12],imx386mipiraw_COLOR_0180[13],imx386mipiraw_COLOR_0180[14],imx386mipiraw_COLOR_0180[15],imx386mipiraw_COLOR_0180[16],imx386mipiraw_COLOR_0180[17],imx386mipiraw_COLOR_0180[18],imx386mipiraw_COLOR_0180[19],
    imx386mipiraw_COLOR_0180[20],imx386mipiraw_COLOR_0180[21],imx386mipiraw_COLOR_0180[22],imx386mipiraw_COLOR_0180[23],imx386mipiraw_COLOR_0180[24],imx386mipiraw_COLOR_0180[25],imx386mipiraw_COLOR_0180[26],imx386mipiraw_COLOR_0180[27],imx386mipiraw_COLOR_0180[28],imx386mipiraw_COLOR_0180[29],
    imx386mipiraw_COLOR_0180[30],imx386mipiraw_COLOR_0180[31],imx386mipiraw_COLOR_0180[32],imx386mipiraw_COLOR_0180[33],imx386mipiraw_COLOR_0180[34],imx386mipiraw_COLOR_0180[35],imx386mipiraw_COLOR_0180[36],imx386mipiraw_COLOR_0180[37],imx386mipiraw_COLOR_0180[38],imx386mipiraw_COLOR_0180[39],
    imx386mipiraw_COLOR_0180[40],imx386mipiraw_COLOR_0180[41],imx386mipiraw_COLOR_0180[42],imx386mipiraw_COLOR_0180[43],imx386mipiraw_COLOR_0180[44],imx386mipiraw_COLOR_0180[45],imx386mipiraw_COLOR_0180[46],imx386mipiraw_COLOR_0180[47],imx386mipiraw_COLOR_0180[48],imx386mipiraw_COLOR_0180[49],
    imx386mipiraw_COLOR_0180[50],imx386mipiraw_COLOR_0180[51],imx386mipiraw_COLOR_0180[52],imx386mipiraw_COLOR_0180[53],imx386mipiraw_COLOR_0180[54],imx386mipiraw_COLOR_0180[55],imx386mipiraw_COLOR_0180[56],imx386mipiraw_COLOR_0180[57],imx386mipiraw_COLOR_0180[58],imx386mipiraw_COLOR_0180[59],
    imx386mipiraw_COLOR_0240[0],imx386mipiraw_COLOR_0240[1],imx386mipiraw_COLOR_0240[2],imx386mipiraw_COLOR_0240[3],imx386mipiraw_COLOR_0240[4],imx386mipiraw_COLOR_0240[5],imx386mipiraw_COLOR_0240[6],imx386mipiraw_COLOR_0240[7],imx386mipiraw_COLOR_0240[8],imx386mipiraw_COLOR_0240[9],
    imx386mipiraw_COLOR_0240[10],imx386mipiraw_COLOR_0240[11],imx386mipiraw_COLOR_0240[12],imx386mipiraw_COLOR_0240[13],imx386mipiraw_COLOR_0240[14],imx386mipiraw_COLOR_0240[15],imx386mipiraw_COLOR_0240[16],imx386mipiraw_COLOR_0240[17],imx386mipiraw_COLOR_0240[18],imx386mipiraw_COLOR_0240[19],
    imx386mipiraw_COLOR_0240[20],imx386mipiraw_COLOR_0240[21],imx386mipiraw_COLOR_0240[22],imx386mipiraw_COLOR_0240[23],imx386mipiraw_COLOR_0240[24],imx386mipiraw_COLOR_0240[25],imx386mipiraw_COLOR_0240[26],imx386mipiraw_COLOR_0240[27],imx386mipiraw_COLOR_0240[28],imx386mipiraw_COLOR_0240[29],
    imx386mipiraw_COLOR_0240[30],imx386mipiraw_COLOR_0240[31],imx386mipiraw_COLOR_0240[32],imx386mipiraw_COLOR_0240[33],imx386mipiraw_COLOR_0240[34],imx386mipiraw_COLOR_0240[35],imx386mipiraw_COLOR_0240[36],imx386mipiraw_COLOR_0240[37],imx386mipiraw_COLOR_0240[38],imx386mipiraw_COLOR_0240[39],
    imx386mipiraw_COLOR_0240[40],imx386mipiraw_COLOR_0240[41],imx386mipiraw_COLOR_0240[42],imx386mipiraw_COLOR_0240[43],imx386mipiraw_COLOR_0240[44],imx386mipiraw_COLOR_0240[45],imx386mipiraw_COLOR_0240[46],imx386mipiraw_COLOR_0240[47],imx386mipiraw_COLOR_0240[48],imx386mipiraw_COLOR_0240[49],
    imx386mipiraw_COLOR_0240[50],imx386mipiraw_COLOR_0240[51],imx386mipiraw_COLOR_0240[52],imx386mipiraw_COLOR_0240[53],imx386mipiraw_COLOR_0240[54],imx386mipiraw_COLOR_0240[55],imx386mipiraw_COLOR_0240[56],imx386mipiraw_COLOR_0240[57],imx386mipiraw_COLOR_0240[58],imx386mipiraw_COLOR_0240[59],
    imx386mipiraw_COLOR_0300[0],imx386mipiraw_COLOR_0300[1],imx386mipiraw_COLOR_0300[2],imx386mipiraw_COLOR_0300[3],imx386mipiraw_COLOR_0300[4],imx386mipiraw_COLOR_0300[5],imx386mipiraw_COLOR_0300[6],imx386mipiraw_COLOR_0300[7],imx386mipiraw_COLOR_0300[8],imx386mipiraw_COLOR_0300[9],
    imx386mipiraw_COLOR_0300[10],imx386mipiraw_COLOR_0300[11],imx386mipiraw_COLOR_0300[12],imx386mipiraw_COLOR_0300[13],imx386mipiraw_COLOR_0300[14],imx386mipiraw_COLOR_0300[15],imx386mipiraw_COLOR_0300[16],imx386mipiraw_COLOR_0300[17],imx386mipiraw_COLOR_0300[18],imx386mipiraw_COLOR_0300[19],
    imx386mipiraw_COLOR_0300[20],imx386mipiraw_COLOR_0300[21],imx386mipiraw_COLOR_0300[22],imx386mipiraw_COLOR_0300[23],imx386mipiraw_COLOR_0300[24],imx386mipiraw_COLOR_0300[25],imx386mipiraw_COLOR_0300[26],imx386mipiraw_COLOR_0300[27],imx386mipiraw_COLOR_0300[28],imx386mipiraw_COLOR_0300[29],
    imx386mipiraw_COLOR_0300[30],imx386mipiraw_COLOR_0300[31],imx386mipiraw_COLOR_0300[32],imx386mipiraw_COLOR_0300[33],imx386mipiraw_COLOR_0300[34],imx386mipiraw_COLOR_0300[35],imx386mipiraw_COLOR_0300[36],imx386mipiraw_COLOR_0300[37],imx386mipiraw_COLOR_0300[38],imx386mipiraw_COLOR_0300[39],
    imx386mipiraw_COLOR_0300[40],imx386mipiraw_COLOR_0300[41],imx386mipiraw_COLOR_0300[42],imx386mipiraw_COLOR_0300[43],imx386mipiraw_COLOR_0300[44],imx386mipiraw_COLOR_0300[45],imx386mipiraw_COLOR_0300[46],imx386mipiraw_COLOR_0300[47],imx386mipiraw_COLOR_0300[48],imx386mipiraw_COLOR_0300[49],
    imx386mipiraw_COLOR_0300[50],imx386mipiraw_COLOR_0300[51],imx386mipiraw_COLOR_0300[52],imx386mipiraw_COLOR_0300[53],imx386mipiraw_COLOR_0300[54],imx386mipiraw_COLOR_0300[55],imx386mipiraw_COLOR_0300[56],imx386mipiraw_COLOR_0300[57],imx386mipiraw_COLOR_0300[58],imx386mipiraw_COLOR_0300[59],
    imx386mipiraw_COLOR_0360[0],imx386mipiraw_COLOR_0360[1],imx386mipiraw_COLOR_0360[2],imx386mipiraw_COLOR_0360[3],imx386mipiraw_COLOR_0360[4],imx386mipiraw_COLOR_0360[5],imx386mipiraw_COLOR_0360[6],imx386mipiraw_COLOR_0360[7],imx386mipiraw_COLOR_0360[8],imx386mipiraw_COLOR_0360[9],
    imx386mipiraw_COLOR_0360[10],imx386mipiraw_COLOR_0360[11],imx386mipiraw_COLOR_0360[12],imx386mipiraw_COLOR_0360[13],imx386mipiraw_COLOR_0360[14],imx386mipiraw_COLOR_0360[15],imx386mipiraw_COLOR_0360[16],imx386mipiraw_COLOR_0360[17],imx386mipiraw_COLOR_0360[18],imx386mipiraw_COLOR_0360[19],
    imx386mipiraw_COLOR_0360[20],imx386mipiraw_COLOR_0360[21],imx386mipiraw_COLOR_0360[22],imx386mipiraw_COLOR_0360[23],imx386mipiraw_COLOR_0360[24],imx386mipiraw_COLOR_0360[25],imx386mipiraw_COLOR_0360[26],imx386mipiraw_COLOR_0360[27],imx386mipiraw_COLOR_0360[28],imx386mipiraw_COLOR_0360[29],
    imx386mipiraw_COLOR_0360[30],imx386mipiraw_COLOR_0360[31],imx386mipiraw_COLOR_0360[32],imx386mipiraw_COLOR_0360[33],imx386mipiraw_COLOR_0360[34],imx386mipiraw_COLOR_0360[35],imx386mipiraw_COLOR_0360[36],imx386mipiraw_COLOR_0360[37],imx386mipiraw_COLOR_0360[38],imx386mipiraw_COLOR_0360[39],
    imx386mipiraw_COLOR_0360[40],imx386mipiraw_COLOR_0360[41],imx386mipiraw_COLOR_0360[42],imx386mipiraw_COLOR_0360[43],imx386mipiraw_COLOR_0360[44],imx386mipiraw_COLOR_0360[45],imx386mipiraw_COLOR_0360[46],imx386mipiraw_COLOR_0360[47],imx386mipiraw_COLOR_0360[48],imx386mipiraw_COLOR_0360[49],
    imx386mipiraw_COLOR_0360[50],imx386mipiraw_COLOR_0360[51],imx386mipiraw_COLOR_0360[52],imx386mipiraw_COLOR_0360[53],imx386mipiraw_COLOR_0360[54],imx386mipiraw_COLOR_0360[55],imx386mipiraw_COLOR_0360[56],imx386mipiraw_COLOR_0360[57],imx386mipiraw_COLOR_0360[58],imx386mipiraw_COLOR_0360[59],
},
