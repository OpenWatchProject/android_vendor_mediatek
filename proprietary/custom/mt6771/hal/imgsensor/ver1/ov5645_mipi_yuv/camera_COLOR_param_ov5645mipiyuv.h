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
    ov5645mipiyuv_CCM_0000, ov5645mipiyuv_CCM_0001, ov5645mipiyuv_CCM_0002, ov5645mipiyuv_CCM_0003, ov5645mipiyuv_CCM_0004, ov5645mipiyuv_CCM_0005, ov5645mipiyuv_CCM_0006, ov5645mipiyuv_CCM_0007, ov5645mipiyuv_CCM_0008, ov5645mipiyuv_CCM_0009,
    ov5645mipiyuv_CCM_0010, ov5645mipiyuv_CCM_0011, ov5645mipiyuv_CCM_0012, ov5645mipiyuv_CCM_0013, ov5645mipiyuv_CCM_0014, ov5645mipiyuv_CCM_0015, ov5645mipiyuv_CCM_0016, ov5645mipiyuv_CCM_0017, ov5645mipiyuv_CCM_0018, ov5645mipiyuv_CCM_0019,
    ov5645mipiyuv_CCM_0020, ov5645mipiyuv_CCM_0021, ov5645mipiyuv_CCM_0022, ov5645mipiyuv_CCM_0023, ov5645mipiyuv_CCM_0024, ov5645mipiyuv_CCM_0025, ov5645mipiyuv_CCM_0026, ov5645mipiyuv_CCM_0027, ov5645mipiyuv_CCM_0028, ov5645mipiyuv_CCM_0029,
    ov5645mipiyuv_CCM_0030, ov5645mipiyuv_CCM_0031, ov5645mipiyuv_CCM_0032, ov5645mipiyuv_CCM_0033, ov5645mipiyuv_CCM_0034, ov5645mipiyuv_CCM_0035, ov5645mipiyuv_CCM_0036, ov5645mipiyuv_CCM_0037, ov5645mipiyuv_CCM_0038, ov5645mipiyuv_CCM_0039,
    ov5645mipiyuv_CCM_0040, ov5645mipiyuv_CCM_0041, ov5645mipiyuv_CCM_0042, ov5645mipiyuv_CCM_0043, ov5645mipiyuv_CCM_0044, ov5645mipiyuv_CCM_0045, ov5645mipiyuv_CCM_0046, ov5645mipiyuv_CCM_0047, ov5645mipiyuv_CCM_0048, ov5645mipiyuv_CCM_0049,
    ov5645mipiyuv_CCM_0050, ov5645mipiyuv_CCM_0051, ov5645mipiyuv_CCM_0052, ov5645mipiyuv_CCM_0053, ov5645mipiyuv_CCM_0054, ov5645mipiyuv_CCM_0055, ov5645mipiyuv_CCM_0056, ov5645mipiyuv_CCM_0057, ov5645mipiyuv_CCM_0058, ov5645mipiyuv_CCM_0059,
    ov5645mipiyuv_CCM_0060, ov5645mipiyuv_CCM_0061, ov5645mipiyuv_CCM_0062, ov5645mipiyuv_CCM_0063, ov5645mipiyuv_CCM_0064, ov5645mipiyuv_CCM_0065, ov5645mipiyuv_CCM_0066, ov5645mipiyuv_CCM_0067, ov5645mipiyuv_CCM_0068, ov5645mipiyuv_CCM_0069,
    ov5645mipiyuv_CCM_0070, ov5645mipiyuv_CCM_0071, ov5645mipiyuv_CCM_0072, ov5645mipiyuv_CCM_0073, ov5645mipiyuv_CCM_0074, ov5645mipiyuv_CCM_0075, ov5645mipiyuv_CCM_0076, ov5645mipiyuv_CCM_0077, ov5645mipiyuv_CCM_0078, ov5645mipiyuv_CCM_0079,
    ov5645mipiyuv_CCM_0080, ov5645mipiyuv_CCM_0081, ov5645mipiyuv_CCM_0082, ov5645mipiyuv_CCM_0083, ov5645mipiyuv_CCM_0084, ov5645mipiyuv_CCM_0085, ov5645mipiyuv_CCM_0086, ov5645mipiyuv_CCM_0087, ov5645mipiyuv_CCM_0088, ov5645mipiyuv_CCM_0089,
    ov5645mipiyuv_CCM_0090, ov5645mipiyuv_CCM_0091, ov5645mipiyuv_CCM_0092, ov5645mipiyuv_CCM_0093, ov5645mipiyuv_CCM_0094, ov5645mipiyuv_CCM_0095, ov5645mipiyuv_CCM_0096, ov5645mipiyuv_CCM_0097, ov5645mipiyuv_CCM_0098, ov5645mipiyuv_CCM_0099,
    ov5645mipiyuv_CCM_0100, ov5645mipiyuv_CCM_0101, ov5645mipiyuv_CCM_0102, ov5645mipiyuv_CCM_0103, ov5645mipiyuv_CCM_0104, ov5645mipiyuv_CCM_0105, ov5645mipiyuv_CCM_0106, ov5645mipiyuv_CCM_0107, ov5645mipiyuv_CCM_0108, ov5645mipiyuv_CCM_0109,
    ov5645mipiyuv_CCM_0110, ov5645mipiyuv_CCM_0111, ov5645mipiyuv_CCM_0112, ov5645mipiyuv_CCM_0113, ov5645mipiyuv_CCM_0114, ov5645mipiyuv_CCM_0115, ov5645mipiyuv_CCM_0116, ov5645mipiyuv_CCM_0117, ov5645mipiyuv_CCM_0118, ov5645mipiyuv_CCM_0119,
    ov5645mipiyuv_CCM_0120, ov5645mipiyuv_CCM_0121, ov5645mipiyuv_CCM_0122, ov5645mipiyuv_CCM_0123, ov5645mipiyuv_CCM_0124, ov5645mipiyuv_CCM_0125, ov5645mipiyuv_CCM_0126, ov5645mipiyuv_CCM_0127, ov5645mipiyuv_CCM_0128, ov5645mipiyuv_CCM_0129,
    ov5645mipiyuv_CCM_0130, ov5645mipiyuv_CCM_0131, ov5645mipiyuv_CCM_0132, ov5645mipiyuv_CCM_0133, ov5645mipiyuv_CCM_0134, ov5645mipiyuv_CCM_0135, ov5645mipiyuv_CCM_0136, ov5645mipiyuv_CCM_0137, ov5645mipiyuv_CCM_0138, ov5645mipiyuv_CCM_0139,
    ov5645mipiyuv_CCM_0140, ov5645mipiyuv_CCM_0141, ov5645mipiyuv_CCM_0142, ov5645mipiyuv_CCM_0143, ov5645mipiyuv_CCM_0144, ov5645mipiyuv_CCM_0145, ov5645mipiyuv_CCM_0146, ov5645mipiyuv_CCM_0147, ov5645mipiyuv_CCM_0148, ov5645mipiyuv_CCM_0149,
    ov5645mipiyuv_CCM_0150, ov5645mipiyuv_CCM_0151, ov5645mipiyuv_CCM_0152, ov5645mipiyuv_CCM_0153, ov5645mipiyuv_CCM_0154, ov5645mipiyuv_CCM_0155, ov5645mipiyuv_CCM_0156, ov5645mipiyuv_CCM_0157, ov5645mipiyuv_CCM_0158, ov5645mipiyuv_CCM_0159,
    ov5645mipiyuv_CCM_0160, ov5645mipiyuv_CCM_0161, ov5645mipiyuv_CCM_0162, ov5645mipiyuv_CCM_0163, ov5645mipiyuv_CCM_0164, ov5645mipiyuv_CCM_0165, ov5645mipiyuv_CCM_0166, ov5645mipiyuv_CCM_0167, ov5645mipiyuv_CCM_0168, ov5645mipiyuv_CCM_0169,
    ov5645mipiyuv_CCM_0170, ov5645mipiyuv_CCM_0171, ov5645mipiyuv_CCM_0172, ov5645mipiyuv_CCM_0173, ov5645mipiyuv_CCM_0174, ov5645mipiyuv_CCM_0175, ov5645mipiyuv_CCM_0176, ov5645mipiyuv_CCM_0177, ov5645mipiyuv_CCM_0178, ov5645mipiyuv_CCM_0179,
    ov5645mipiyuv_CCM_0180, ov5645mipiyuv_CCM_0181, ov5645mipiyuv_CCM_0182, ov5645mipiyuv_CCM_0183, ov5645mipiyuv_CCM_0184, ov5645mipiyuv_CCM_0185, ov5645mipiyuv_CCM_0186, ov5645mipiyuv_CCM_0187, ov5645mipiyuv_CCM_0188, ov5645mipiyuv_CCM_0189,
    ov5645mipiyuv_CCM_0190, ov5645mipiyuv_CCM_0191, ov5645mipiyuv_CCM_0192, ov5645mipiyuv_CCM_0193, ov5645mipiyuv_CCM_0194, ov5645mipiyuv_CCM_0195, ov5645mipiyuv_CCM_0196, ov5645mipiyuv_CCM_0197, ov5645mipiyuv_CCM_0198, ov5645mipiyuv_CCM_0199,
    ov5645mipiyuv_CCM_0200, ov5645mipiyuv_CCM_0201, ov5645mipiyuv_CCM_0202, ov5645mipiyuv_CCM_0203, ov5645mipiyuv_CCM_0204, ov5645mipiyuv_CCM_0205, ov5645mipiyuv_CCM_0206, ov5645mipiyuv_CCM_0207, ov5645mipiyuv_CCM_0208, ov5645mipiyuv_CCM_0209,
    ov5645mipiyuv_CCM_0210, ov5645mipiyuv_CCM_0211, ov5645mipiyuv_CCM_0212, ov5645mipiyuv_CCM_0213, ov5645mipiyuv_CCM_0214, ov5645mipiyuv_CCM_0215, ov5645mipiyuv_CCM_0216, ov5645mipiyuv_CCM_0217, ov5645mipiyuv_CCM_0218, ov5645mipiyuv_CCM_0219,
    ov5645mipiyuv_CCM_0220, ov5645mipiyuv_CCM_0221, ov5645mipiyuv_CCM_0222, ov5645mipiyuv_CCM_0223, ov5645mipiyuv_CCM_0224, ov5645mipiyuv_CCM_0225, ov5645mipiyuv_CCM_0226, ov5645mipiyuv_CCM_0227, ov5645mipiyuv_CCM_0228, ov5645mipiyuv_CCM_0229,
    ov5645mipiyuv_CCM_0230, ov5645mipiyuv_CCM_0231, ov5645mipiyuv_CCM_0232, ov5645mipiyuv_CCM_0233, ov5645mipiyuv_CCM_0234, ov5645mipiyuv_CCM_0235, ov5645mipiyuv_CCM_0236, ov5645mipiyuv_CCM_0237, ov5645mipiyuv_CCM_0238, ov5645mipiyuv_CCM_0239,
},
.COLOR = {
    ov5645mipiyuv_COLOR_0000, ov5645mipiyuv_COLOR_0001, ov5645mipiyuv_COLOR_0002, ov5645mipiyuv_COLOR_0003, ov5645mipiyuv_COLOR_0004, ov5645mipiyuv_COLOR_0005, ov5645mipiyuv_COLOR_0006, ov5645mipiyuv_COLOR_0007, ov5645mipiyuv_COLOR_0008, ov5645mipiyuv_COLOR_0009,
    ov5645mipiyuv_COLOR_0010, ov5645mipiyuv_COLOR_0011, ov5645mipiyuv_COLOR_0012, ov5645mipiyuv_COLOR_0013, ov5645mipiyuv_COLOR_0014, ov5645mipiyuv_COLOR_0015, ov5645mipiyuv_COLOR_0016, ov5645mipiyuv_COLOR_0017, ov5645mipiyuv_COLOR_0018, ov5645mipiyuv_COLOR_0019,
    ov5645mipiyuv_COLOR_0020, ov5645mipiyuv_COLOR_0021, ov5645mipiyuv_COLOR_0022, ov5645mipiyuv_COLOR_0023, ov5645mipiyuv_COLOR_0024, ov5645mipiyuv_COLOR_0025, ov5645mipiyuv_COLOR_0026, ov5645mipiyuv_COLOR_0027, ov5645mipiyuv_COLOR_0028, ov5645mipiyuv_COLOR_0029,
    ov5645mipiyuv_COLOR_0030, ov5645mipiyuv_COLOR_0031, ov5645mipiyuv_COLOR_0032, ov5645mipiyuv_COLOR_0033, ov5645mipiyuv_COLOR_0034, ov5645mipiyuv_COLOR_0035, ov5645mipiyuv_COLOR_0036, ov5645mipiyuv_COLOR_0037, ov5645mipiyuv_COLOR_0038, ov5645mipiyuv_COLOR_0039,
    ov5645mipiyuv_COLOR_0040, ov5645mipiyuv_COLOR_0041, ov5645mipiyuv_COLOR_0042, ov5645mipiyuv_COLOR_0043, ov5645mipiyuv_COLOR_0044, ov5645mipiyuv_COLOR_0045, ov5645mipiyuv_COLOR_0046, ov5645mipiyuv_COLOR_0047, ov5645mipiyuv_COLOR_0048, ov5645mipiyuv_COLOR_0049,
    ov5645mipiyuv_COLOR_0050, ov5645mipiyuv_COLOR_0051, ov5645mipiyuv_COLOR_0052, ov5645mipiyuv_COLOR_0053, ov5645mipiyuv_COLOR_0054, ov5645mipiyuv_COLOR_0055, ov5645mipiyuv_COLOR_0056, ov5645mipiyuv_COLOR_0057, ov5645mipiyuv_COLOR_0058, ov5645mipiyuv_COLOR_0059,
    ov5645mipiyuv_COLOR_0060, ov5645mipiyuv_COLOR_0061, ov5645mipiyuv_COLOR_0062, ov5645mipiyuv_COLOR_0063, ov5645mipiyuv_COLOR_0064, ov5645mipiyuv_COLOR_0065, ov5645mipiyuv_COLOR_0066, ov5645mipiyuv_COLOR_0067, ov5645mipiyuv_COLOR_0068, ov5645mipiyuv_COLOR_0069,
    ov5645mipiyuv_COLOR_0070, ov5645mipiyuv_COLOR_0071, ov5645mipiyuv_COLOR_0072, ov5645mipiyuv_COLOR_0073, ov5645mipiyuv_COLOR_0074, ov5645mipiyuv_COLOR_0075, ov5645mipiyuv_COLOR_0076, ov5645mipiyuv_COLOR_0077, ov5645mipiyuv_COLOR_0078, ov5645mipiyuv_COLOR_0079,
    ov5645mipiyuv_COLOR_0080, ov5645mipiyuv_COLOR_0081, ov5645mipiyuv_COLOR_0082, ov5645mipiyuv_COLOR_0083, ov5645mipiyuv_COLOR_0084, ov5645mipiyuv_COLOR_0085, ov5645mipiyuv_COLOR_0086, ov5645mipiyuv_COLOR_0087, ov5645mipiyuv_COLOR_0088, ov5645mipiyuv_COLOR_0089,
    ov5645mipiyuv_COLOR_0090, ov5645mipiyuv_COLOR_0091, ov5645mipiyuv_COLOR_0092, ov5645mipiyuv_COLOR_0093, ov5645mipiyuv_COLOR_0094, ov5645mipiyuv_COLOR_0095, ov5645mipiyuv_COLOR_0096, ov5645mipiyuv_COLOR_0097, ov5645mipiyuv_COLOR_0098, ov5645mipiyuv_COLOR_0099,
    ov5645mipiyuv_COLOR_0100, ov5645mipiyuv_COLOR_0101, ov5645mipiyuv_COLOR_0102, ov5645mipiyuv_COLOR_0103, ov5645mipiyuv_COLOR_0104, ov5645mipiyuv_COLOR_0105, ov5645mipiyuv_COLOR_0106, ov5645mipiyuv_COLOR_0107, ov5645mipiyuv_COLOR_0108, ov5645mipiyuv_COLOR_0109,
    ov5645mipiyuv_COLOR_0110, ov5645mipiyuv_COLOR_0111, ov5645mipiyuv_COLOR_0112, ov5645mipiyuv_COLOR_0113, ov5645mipiyuv_COLOR_0114, ov5645mipiyuv_COLOR_0115, ov5645mipiyuv_COLOR_0116, ov5645mipiyuv_COLOR_0117, ov5645mipiyuv_COLOR_0118, ov5645mipiyuv_COLOR_0119,
    ov5645mipiyuv_COLOR_0120, ov5645mipiyuv_COLOR_0121, ov5645mipiyuv_COLOR_0122, ov5645mipiyuv_COLOR_0123, ov5645mipiyuv_COLOR_0124, ov5645mipiyuv_COLOR_0125, ov5645mipiyuv_COLOR_0126, ov5645mipiyuv_COLOR_0127, ov5645mipiyuv_COLOR_0128, ov5645mipiyuv_COLOR_0129,
    ov5645mipiyuv_COLOR_0130, ov5645mipiyuv_COLOR_0131, ov5645mipiyuv_COLOR_0132, ov5645mipiyuv_COLOR_0133, ov5645mipiyuv_COLOR_0134, ov5645mipiyuv_COLOR_0135, ov5645mipiyuv_COLOR_0136, ov5645mipiyuv_COLOR_0137, ov5645mipiyuv_COLOR_0138, ov5645mipiyuv_COLOR_0139,
    ov5645mipiyuv_COLOR_0140, ov5645mipiyuv_COLOR_0141, ov5645mipiyuv_COLOR_0142, ov5645mipiyuv_COLOR_0143, ov5645mipiyuv_COLOR_0144, ov5645mipiyuv_COLOR_0145, ov5645mipiyuv_COLOR_0146, ov5645mipiyuv_COLOR_0147, ov5645mipiyuv_COLOR_0148, ov5645mipiyuv_COLOR_0149,
    ov5645mipiyuv_COLOR_0150, ov5645mipiyuv_COLOR_0151, ov5645mipiyuv_COLOR_0152, ov5645mipiyuv_COLOR_0153, ov5645mipiyuv_COLOR_0154, ov5645mipiyuv_COLOR_0155, ov5645mipiyuv_COLOR_0156, ov5645mipiyuv_COLOR_0157, ov5645mipiyuv_COLOR_0158, ov5645mipiyuv_COLOR_0159,
    ov5645mipiyuv_COLOR_0160, ov5645mipiyuv_COLOR_0161, ov5645mipiyuv_COLOR_0162, ov5645mipiyuv_COLOR_0163, ov5645mipiyuv_COLOR_0164, ov5645mipiyuv_COLOR_0165, ov5645mipiyuv_COLOR_0166, ov5645mipiyuv_COLOR_0167, ov5645mipiyuv_COLOR_0168, ov5645mipiyuv_COLOR_0169,
    ov5645mipiyuv_COLOR_0170, ov5645mipiyuv_COLOR_0171, ov5645mipiyuv_COLOR_0172, ov5645mipiyuv_COLOR_0173, ov5645mipiyuv_COLOR_0174, ov5645mipiyuv_COLOR_0175, ov5645mipiyuv_COLOR_0176, ov5645mipiyuv_COLOR_0177, ov5645mipiyuv_COLOR_0178, ov5645mipiyuv_COLOR_0179,
    ov5645mipiyuv_COLOR_0180, ov5645mipiyuv_COLOR_0181, ov5645mipiyuv_COLOR_0182, ov5645mipiyuv_COLOR_0183, ov5645mipiyuv_COLOR_0184, ov5645mipiyuv_COLOR_0185, ov5645mipiyuv_COLOR_0186, ov5645mipiyuv_COLOR_0187, ov5645mipiyuv_COLOR_0188, ov5645mipiyuv_COLOR_0189,
    ov5645mipiyuv_COLOR_0190, ov5645mipiyuv_COLOR_0191, ov5645mipiyuv_COLOR_0192, ov5645mipiyuv_COLOR_0193, ov5645mipiyuv_COLOR_0194, ov5645mipiyuv_COLOR_0195, ov5645mipiyuv_COLOR_0196, ov5645mipiyuv_COLOR_0197, ov5645mipiyuv_COLOR_0198, ov5645mipiyuv_COLOR_0199,
    ov5645mipiyuv_COLOR_0200, ov5645mipiyuv_COLOR_0201, ov5645mipiyuv_COLOR_0202, ov5645mipiyuv_COLOR_0203, ov5645mipiyuv_COLOR_0204, ov5645mipiyuv_COLOR_0205, ov5645mipiyuv_COLOR_0206, ov5645mipiyuv_COLOR_0207, ov5645mipiyuv_COLOR_0208, ov5645mipiyuv_COLOR_0209,
    ov5645mipiyuv_COLOR_0210, ov5645mipiyuv_COLOR_0211, ov5645mipiyuv_COLOR_0212, ov5645mipiyuv_COLOR_0213, ov5645mipiyuv_COLOR_0214, ov5645mipiyuv_COLOR_0215, ov5645mipiyuv_COLOR_0216, ov5645mipiyuv_COLOR_0217, ov5645mipiyuv_COLOR_0218, ov5645mipiyuv_COLOR_0219,
    ov5645mipiyuv_COLOR_0220, ov5645mipiyuv_COLOR_0221, ov5645mipiyuv_COLOR_0222, ov5645mipiyuv_COLOR_0223, ov5645mipiyuv_COLOR_0224, ov5645mipiyuv_COLOR_0225, ov5645mipiyuv_COLOR_0226, ov5645mipiyuv_COLOR_0227, ov5645mipiyuv_COLOR_0228, ov5645mipiyuv_COLOR_0229,
    ov5645mipiyuv_COLOR_0230, ov5645mipiyuv_COLOR_0231, ov5645mipiyuv_COLOR_0232, ov5645mipiyuv_COLOR_0233, ov5645mipiyuv_COLOR_0234, ov5645mipiyuv_COLOR_0235, ov5645mipiyuv_COLOR_0236, ov5645mipiyuv_COLOR_0237, ov5645mipiyuv_COLOR_0238, ov5645mipiyuv_COLOR_0239,
    ov5645mipiyuv_COLOR_0240, ov5645mipiyuv_COLOR_0241, ov5645mipiyuv_COLOR_0242, ov5645mipiyuv_COLOR_0243, ov5645mipiyuv_COLOR_0244, ov5645mipiyuv_COLOR_0245, ov5645mipiyuv_COLOR_0246, ov5645mipiyuv_COLOR_0247, ov5645mipiyuv_COLOR_0248, ov5645mipiyuv_COLOR_0249,
    ov5645mipiyuv_COLOR_0250, ov5645mipiyuv_COLOR_0251, ov5645mipiyuv_COLOR_0252, ov5645mipiyuv_COLOR_0253, ov5645mipiyuv_COLOR_0254, ov5645mipiyuv_COLOR_0255, ov5645mipiyuv_COLOR_0256, ov5645mipiyuv_COLOR_0257, ov5645mipiyuv_COLOR_0258, ov5645mipiyuv_COLOR_0259,
    ov5645mipiyuv_COLOR_0260, ov5645mipiyuv_COLOR_0261, ov5645mipiyuv_COLOR_0262, ov5645mipiyuv_COLOR_0263, ov5645mipiyuv_COLOR_0264, ov5645mipiyuv_COLOR_0265, ov5645mipiyuv_COLOR_0266, ov5645mipiyuv_COLOR_0267, ov5645mipiyuv_COLOR_0268, ov5645mipiyuv_COLOR_0269,
    ov5645mipiyuv_COLOR_0270, ov5645mipiyuv_COLOR_0271, ov5645mipiyuv_COLOR_0272, ov5645mipiyuv_COLOR_0273, ov5645mipiyuv_COLOR_0274, ov5645mipiyuv_COLOR_0275, ov5645mipiyuv_COLOR_0276, ov5645mipiyuv_COLOR_0277, ov5645mipiyuv_COLOR_0278, ov5645mipiyuv_COLOR_0279,
    ov5645mipiyuv_COLOR_0280, ov5645mipiyuv_COLOR_0281, ov5645mipiyuv_COLOR_0282, ov5645mipiyuv_COLOR_0283, ov5645mipiyuv_COLOR_0284, ov5645mipiyuv_COLOR_0285, ov5645mipiyuv_COLOR_0286, ov5645mipiyuv_COLOR_0287, ov5645mipiyuv_COLOR_0288, ov5645mipiyuv_COLOR_0289,
    ov5645mipiyuv_COLOR_0290, ov5645mipiyuv_COLOR_0291, ov5645mipiyuv_COLOR_0292, ov5645mipiyuv_COLOR_0293, ov5645mipiyuv_COLOR_0294, ov5645mipiyuv_COLOR_0295, ov5645mipiyuv_COLOR_0296, ov5645mipiyuv_COLOR_0297, ov5645mipiyuv_COLOR_0298, ov5645mipiyuv_COLOR_0299,
    ov5645mipiyuv_COLOR_0300, ov5645mipiyuv_COLOR_0301, ov5645mipiyuv_COLOR_0302, ov5645mipiyuv_COLOR_0303, ov5645mipiyuv_COLOR_0304, ov5645mipiyuv_COLOR_0305, ov5645mipiyuv_COLOR_0306, ov5645mipiyuv_COLOR_0307, ov5645mipiyuv_COLOR_0308, ov5645mipiyuv_COLOR_0309,
    ov5645mipiyuv_COLOR_0310, ov5645mipiyuv_COLOR_0311, ov5645mipiyuv_COLOR_0312, ov5645mipiyuv_COLOR_0313, ov5645mipiyuv_COLOR_0314, ov5645mipiyuv_COLOR_0315, ov5645mipiyuv_COLOR_0316, ov5645mipiyuv_COLOR_0317, ov5645mipiyuv_COLOR_0318, ov5645mipiyuv_COLOR_0319,
    ov5645mipiyuv_COLOR_0320, ov5645mipiyuv_COLOR_0321, ov5645mipiyuv_COLOR_0322, ov5645mipiyuv_COLOR_0323, ov5645mipiyuv_COLOR_0324, ov5645mipiyuv_COLOR_0325, ov5645mipiyuv_COLOR_0326, ov5645mipiyuv_COLOR_0327, ov5645mipiyuv_COLOR_0328, ov5645mipiyuv_COLOR_0329,
    ov5645mipiyuv_COLOR_0330, ov5645mipiyuv_COLOR_0331, ov5645mipiyuv_COLOR_0332, ov5645mipiyuv_COLOR_0333, ov5645mipiyuv_COLOR_0334, ov5645mipiyuv_COLOR_0335, ov5645mipiyuv_COLOR_0336, ov5645mipiyuv_COLOR_0337, ov5645mipiyuv_COLOR_0338, ov5645mipiyuv_COLOR_0339,
    ov5645mipiyuv_COLOR_0340, ov5645mipiyuv_COLOR_0341, ov5645mipiyuv_COLOR_0342, ov5645mipiyuv_COLOR_0343, ov5645mipiyuv_COLOR_0344, ov5645mipiyuv_COLOR_0345, ov5645mipiyuv_COLOR_0346, ov5645mipiyuv_COLOR_0347, ov5645mipiyuv_COLOR_0348, ov5645mipiyuv_COLOR_0349,
    ov5645mipiyuv_COLOR_0350, ov5645mipiyuv_COLOR_0351, ov5645mipiyuv_COLOR_0352, ov5645mipiyuv_COLOR_0353, ov5645mipiyuv_COLOR_0354, ov5645mipiyuv_COLOR_0355, ov5645mipiyuv_COLOR_0356, ov5645mipiyuv_COLOR_0357, ov5645mipiyuv_COLOR_0358, ov5645mipiyuv_COLOR_0359,
    ov5645mipiyuv_COLOR_0360, ov5645mipiyuv_COLOR_0361, ov5645mipiyuv_COLOR_0362, ov5645mipiyuv_COLOR_0363, ov5645mipiyuv_COLOR_0364, ov5645mipiyuv_COLOR_0365, ov5645mipiyuv_COLOR_0366, ov5645mipiyuv_COLOR_0367, ov5645mipiyuv_COLOR_0368, ov5645mipiyuv_COLOR_0369,
    ov5645mipiyuv_COLOR_0370, ov5645mipiyuv_COLOR_0371, ov5645mipiyuv_COLOR_0372, ov5645mipiyuv_COLOR_0373, ov5645mipiyuv_COLOR_0374, ov5645mipiyuv_COLOR_0375, ov5645mipiyuv_COLOR_0376, ov5645mipiyuv_COLOR_0377, ov5645mipiyuv_COLOR_0378, ov5645mipiyuv_COLOR_0379,
    ov5645mipiyuv_COLOR_0380, ov5645mipiyuv_COLOR_0381, ov5645mipiyuv_COLOR_0382, ov5645mipiyuv_COLOR_0383, ov5645mipiyuv_COLOR_0384, ov5645mipiyuv_COLOR_0385, ov5645mipiyuv_COLOR_0386, ov5645mipiyuv_COLOR_0387, ov5645mipiyuv_COLOR_0388, ov5645mipiyuv_COLOR_0389,
    ov5645mipiyuv_COLOR_0390, ov5645mipiyuv_COLOR_0391, ov5645mipiyuv_COLOR_0392, ov5645mipiyuv_COLOR_0393, ov5645mipiyuv_COLOR_0394, ov5645mipiyuv_COLOR_0395, ov5645mipiyuv_COLOR_0396, ov5645mipiyuv_COLOR_0397, ov5645mipiyuv_COLOR_0398, ov5645mipiyuv_COLOR_0399,
    ov5645mipiyuv_COLOR_0400, ov5645mipiyuv_COLOR_0401, ov5645mipiyuv_COLOR_0402, ov5645mipiyuv_COLOR_0403, ov5645mipiyuv_COLOR_0404, ov5645mipiyuv_COLOR_0405, ov5645mipiyuv_COLOR_0406, ov5645mipiyuv_COLOR_0407, ov5645mipiyuv_COLOR_0408, ov5645mipiyuv_COLOR_0409,
    ov5645mipiyuv_COLOR_0410, ov5645mipiyuv_COLOR_0411, ov5645mipiyuv_COLOR_0412, ov5645mipiyuv_COLOR_0413, ov5645mipiyuv_COLOR_0414, ov5645mipiyuv_COLOR_0415, ov5645mipiyuv_COLOR_0416, ov5645mipiyuv_COLOR_0417, ov5645mipiyuv_COLOR_0418, ov5645mipiyuv_COLOR_0419,
    ov5645mipiyuv_COLOR_0420, ov5645mipiyuv_COLOR_0421, ov5645mipiyuv_COLOR_0422, ov5645mipiyuv_COLOR_0423, ov5645mipiyuv_COLOR_0424, ov5645mipiyuv_COLOR_0425, ov5645mipiyuv_COLOR_0426, ov5645mipiyuv_COLOR_0427, ov5645mipiyuv_COLOR_0428, ov5645mipiyuv_COLOR_0429,
    ov5645mipiyuv_COLOR_0430, ov5645mipiyuv_COLOR_0431, ov5645mipiyuv_COLOR_0432, ov5645mipiyuv_COLOR_0433, ov5645mipiyuv_COLOR_0434, ov5645mipiyuv_COLOR_0435, ov5645mipiyuv_COLOR_0436, ov5645mipiyuv_COLOR_0437, ov5645mipiyuv_COLOR_0438, ov5645mipiyuv_COLOR_0439,
    ov5645mipiyuv_COLOR_0440, ov5645mipiyuv_COLOR_0441, ov5645mipiyuv_COLOR_0442, ov5645mipiyuv_COLOR_0443, ov5645mipiyuv_COLOR_0444, ov5645mipiyuv_COLOR_0445, ov5645mipiyuv_COLOR_0446, ov5645mipiyuv_COLOR_0447, ov5645mipiyuv_COLOR_0448, ov5645mipiyuv_COLOR_0449,
    ov5645mipiyuv_COLOR_0450, ov5645mipiyuv_COLOR_0451, ov5645mipiyuv_COLOR_0452, ov5645mipiyuv_COLOR_0453, ov5645mipiyuv_COLOR_0454, ov5645mipiyuv_COLOR_0455, ov5645mipiyuv_COLOR_0456, ov5645mipiyuv_COLOR_0457, ov5645mipiyuv_COLOR_0458, ov5645mipiyuv_COLOR_0459,
    ov5645mipiyuv_COLOR_0460, ov5645mipiyuv_COLOR_0461, ov5645mipiyuv_COLOR_0462, ov5645mipiyuv_COLOR_0463, ov5645mipiyuv_COLOR_0464, ov5645mipiyuv_COLOR_0465, ov5645mipiyuv_COLOR_0466, ov5645mipiyuv_COLOR_0467, ov5645mipiyuv_COLOR_0468, ov5645mipiyuv_COLOR_0469,
    ov5645mipiyuv_COLOR_0470, ov5645mipiyuv_COLOR_0471, ov5645mipiyuv_COLOR_0472, ov5645mipiyuv_COLOR_0473, ov5645mipiyuv_COLOR_0474, ov5645mipiyuv_COLOR_0475, ov5645mipiyuv_COLOR_0476, ov5645mipiyuv_COLOR_0477, ov5645mipiyuv_COLOR_0478, ov5645mipiyuv_COLOR_0479,
    ov5645mipiyuv_COLOR_0480, ov5645mipiyuv_COLOR_0481, ov5645mipiyuv_COLOR_0482, ov5645mipiyuv_COLOR_0483, ov5645mipiyuv_COLOR_0484, ov5645mipiyuv_COLOR_0485, ov5645mipiyuv_COLOR_0486, ov5645mipiyuv_COLOR_0487, ov5645mipiyuv_COLOR_0488, ov5645mipiyuv_COLOR_0489,
    ov5645mipiyuv_COLOR_0490, ov5645mipiyuv_COLOR_0491, ov5645mipiyuv_COLOR_0492, ov5645mipiyuv_COLOR_0493, ov5645mipiyuv_COLOR_0494, ov5645mipiyuv_COLOR_0495, ov5645mipiyuv_COLOR_0496, ov5645mipiyuv_COLOR_0497, ov5645mipiyuv_COLOR_0498, ov5645mipiyuv_COLOR_0499,
    ov5645mipiyuv_COLOR_0500, ov5645mipiyuv_COLOR_0501, ov5645mipiyuv_COLOR_0502, ov5645mipiyuv_COLOR_0503, ov5645mipiyuv_COLOR_0504, ov5645mipiyuv_COLOR_0505, ov5645mipiyuv_COLOR_0506, ov5645mipiyuv_COLOR_0507, ov5645mipiyuv_COLOR_0508, ov5645mipiyuv_COLOR_0509,
    ov5645mipiyuv_COLOR_0510, ov5645mipiyuv_COLOR_0511, ov5645mipiyuv_COLOR_0512, ov5645mipiyuv_COLOR_0513, ov5645mipiyuv_COLOR_0514, ov5645mipiyuv_COLOR_0515, ov5645mipiyuv_COLOR_0516, ov5645mipiyuv_COLOR_0517, ov5645mipiyuv_COLOR_0518, ov5645mipiyuv_COLOR_0519,
    ov5645mipiyuv_COLOR_0520, ov5645mipiyuv_COLOR_0521, ov5645mipiyuv_COLOR_0522, ov5645mipiyuv_COLOR_0523, ov5645mipiyuv_COLOR_0524, ov5645mipiyuv_COLOR_0525, ov5645mipiyuv_COLOR_0526, ov5645mipiyuv_COLOR_0527, ov5645mipiyuv_COLOR_0528, ov5645mipiyuv_COLOR_0529,
    ov5645mipiyuv_COLOR_0530, ov5645mipiyuv_COLOR_0531, ov5645mipiyuv_COLOR_0532, ov5645mipiyuv_COLOR_0533, ov5645mipiyuv_COLOR_0534, ov5645mipiyuv_COLOR_0535, ov5645mipiyuv_COLOR_0536, ov5645mipiyuv_COLOR_0537, ov5645mipiyuv_COLOR_0538, ov5645mipiyuv_COLOR_0539,
    ov5645mipiyuv_COLOR_0540, ov5645mipiyuv_COLOR_0541, ov5645mipiyuv_COLOR_0542, ov5645mipiyuv_COLOR_0543, ov5645mipiyuv_COLOR_0544, ov5645mipiyuv_COLOR_0545, ov5645mipiyuv_COLOR_0546, ov5645mipiyuv_COLOR_0547, ov5645mipiyuv_COLOR_0548, ov5645mipiyuv_COLOR_0549,
    ov5645mipiyuv_COLOR_0550, ov5645mipiyuv_COLOR_0551, ov5645mipiyuv_COLOR_0552, ov5645mipiyuv_COLOR_0553, ov5645mipiyuv_COLOR_0554, ov5645mipiyuv_COLOR_0555, ov5645mipiyuv_COLOR_0556, ov5645mipiyuv_COLOR_0557, ov5645mipiyuv_COLOR_0558, ov5645mipiyuv_COLOR_0559,
    ov5645mipiyuv_COLOR_0560, ov5645mipiyuv_COLOR_0561, ov5645mipiyuv_COLOR_0562, ov5645mipiyuv_COLOR_0563, ov5645mipiyuv_COLOR_0564, ov5645mipiyuv_COLOR_0565, ov5645mipiyuv_COLOR_0566, ov5645mipiyuv_COLOR_0567, ov5645mipiyuv_COLOR_0568, ov5645mipiyuv_COLOR_0569,
    ov5645mipiyuv_COLOR_0570, ov5645mipiyuv_COLOR_0571, ov5645mipiyuv_COLOR_0572, ov5645mipiyuv_COLOR_0573, ov5645mipiyuv_COLOR_0574, ov5645mipiyuv_COLOR_0575, ov5645mipiyuv_COLOR_0576, ov5645mipiyuv_COLOR_0577, ov5645mipiyuv_COLOR_0578, ov5645mipiyuv_COLOR_0579,
    ov5645mipiyuv_COLOR_0580, ov5645mipiyuv_COLOR_0581, ov5645mipiyuv_COLOR_0582, ov5645mipiyuv_COLOR_0583, ov5645mipiyuv_COLOR_0584, ov5645mipiyuv_COLOR_0585, ov5645mipiyuv_COLOR_0586, ov5645mipiyuv_COLOR_0587, ov5645mipiyuv_COLOR_0588, ov5645mipiyuv_COLOR_0589,
    ov5645mipiyuv_COLOR_0590, ov5645mipiyuv_COLOR_0591, ov5645mipiyuv_COLOR_0592, ov5645mipiyuv_COLOR_0593, ov5645mipiyuv_COLOR_0594, ov5645mipiyuv_COLOR_0595, ov5645mipiyuv_COLOR_0596, ov5645mipiyuv_COLOR_0597, ov5645mipiyuv_COLOR_0598, ov5645mipiyuv_COLOR_0599,
    ov5645mipiyuv_COLOR_0600, ov5645mipiyuv_COLOR_0601, ov5645mipiyuv_COLOR_0602, ov5645mipiyuv_COLOR_0603, ov5645mipiyuv_COLOR_0604, ov5645mipiyuv_COLOR_0605, ov5645mipiyuv_COLOR_0606, ov5645mipiyuv_COLOR_0607, ov5645mipiyuv_COLOR_0608, ov5645mipiyuv_COLOR_0609,
    ov5645mipiyuv_COLOR_0610, ov5645mipiyuv_COLOR_0611, ov5645mipiyuv_COLOR_0612, ov5645mipiyuv_COLOR_0613, ov5645mipiyuv_COLOR_0614, ov5645mipiyuv_COLOR_0615, ov5645mipiyuv_COLOR_0616, ov5645mipiyuv_COLOR_0617, ov5645mipiyuv_COLOR_0618, ov5645mipiyuv_COLOR_0619,
    ov5645mipiyuv_COLOR_0620, ov5645mipiyuv_COLOR_0621, ov5645mipiyuv_COLOR_0622, ov5645mipiyuv_COLOR_0623, ov5645mipiyuv_COLOR_0624, ov5645mipiyuv_COLOR_0625, ov5645mipiyuv_COLOR_0626, ov5645mipiyuv_COLOR_0627, ov5645mipiyuv_COLOR_0628, ov5645mipiyuv_COLOR_0629,
    ov5645mipiyuv_COLOR_0630, ov5645mipiyuv_COLOR_0631, ov5645mipiyuv_COLOR_0632, ov5645mipiyuv_COLOR_0633, ov5645mipiyuv_COLOR_0634, ov5645mipiyuv_COLOR_0635, ov5645mipiyuv_COLOR_0636, ov5645mipiyuv_COLOR_0637, ov5645mipiyuv_COLOR_0638, ov5645mipiyuv_COLOR_0639,
    ov5645mipiyuv_COLOR_0640, ov5645mipiyuv_COLOR_0641, ov5645mipiyuv_COLOR_0642, ov5645mipiyuv_COLOR_0643, ov5645mipiyuv_COLOR_0644, ov5645mipiyuv_COLOR_0645, ov5645mipiyuv_COLOR_0646, ov5645mipiyuv_COLOR_0647, ov5645mipiyuv_COLOR_0648, ov5645mipiyuv_COLOR_0649,
    ov5645mipiyuv_COLOR_0650, ov5645mipiyuv_COLOR_0651, ov5645mipiyuv_COLOR_0652, ov5645mipiyuv_COLOR_0653, ov5645mipiyuv_COLOR_0654, ov5645mipiyuv_COLOR_0655, ov5645mipiyuv_COLOR_0656, ov5645mipiyuv_COLOR_0657, ov5645mipiyuv_COLOR_0658, ov5645mipiyuv_COLOR_0659,
    ov5645mipiyuv_COLOR_0660, ov5645mipiyuv_COLOR_0661, ov5645mipiyuv_COLOR_0662, ov5645mipiyuv_COLOR_0663, ov5645mipiyuv_COLOR_0664, ov5645mipiyuv_COLOR_0665, ov5645mipiyuv_COLOR_0666, ov5645mipiyuv_COLOR_0667, ov5645mipiyuv_COLOR_0668, ov5645mipiyuv_COLOR_0669,
    ov5645mipiyuv_COLOR_0670, ov5645mipiyuv_COLOR_0671, ov5645mipiyuv_COLOR_0672, ov5645mipiyuv_COLOR_0673, ov5645mipiyuv_COLOR_0674, ov5645mipiyuv_COLOR_0675, ov5645mipiyuv_COLOR_0676, ov5645mipiyuv_COLOR_0677, ov5645mipiyuv_COLOR_0678, ov5645mipiyuv_COLOR_0679,
    ov5645mipiyuv_COLOR_0680, ov5645mipiyuv_COLOR_0681, ov5645mipiyuv_COLOR_0682, ov5645mipiyuv_COLOR_0683, ov5645mipiyuv_COLOR_0684, ov5645mipiyuv_COLOR_0685, ov5645mipiyuv_COLOR_0686, ov5645mipiyuv_COLOR_0687, ov5645mipiyuv_COLOR_0688, ov5645mipiyuv_COLOR_0689,
    ov5645mipiyuv_COLOR_0690, ov5645mipiyuv_COLOR_0691, ov5645mipiyuv_COLOR_0692, ov5645mipiyuv_COLOR_0693, ov5645mipiyuv_COLOR_0694, ov5645mipiyuv_COLOR_0695, ov5645mipiyuv_COLOR_0696, ov5645mipiyuv_COLOR_0697, ov5645mipiyuv_COLOR_0698, ov5645mipiyuv_COLOR_0699,
    ov5645mipiyuv_COLOR_0700, ov5645mipiyuv_COLOR_0701, ov5645mipiyuv_COLOR_0702, ov5645mipiyuv_COLOR_0703, ov5645mipiyuv_COLOR_0704, ov5645mipiyuv_COLOR_0705, ov5645mipiyuv_COLOR_0706, ov5645mipiyuv_COLOR_0707, ov5645mipiyuv_COLOR_0708, ov5645mipiyuv_COLOR_0709,
    ov5645mipiyuv_COLOR_0710, ov5645mipiyuv_COLOR_0711, ov5645mipiyuv_COLOR_0712, ov5645mipiyuv_COLOR_0713, ov5645mipiyuv_COLOR_0714, ov5645mipiyuv_COLOR_0715, ov5645mipiyuv_COLOR_0716, ov5645mipiyuv_COLOR_0717, ov5645mipiyuv_COLOR_0718, ov5645mipiyuv_COLOR_0719,
    ov5645mipiyuv_COLOR_0720, ov5645mipiyuv_COLOR_0721, ov5645mipiyuv_COLOR_0722, ov5645mipiyuv_COLOR_0723, ov5645mipiyuv_COLOR_0724, ov5645mipiyuv_COLOR_0725, ov5645mipiyuv_COLOR_0726, ov5645mipiyuv_COLOR_0727, ov5645mipiyuv_COLOR_0728, ov5645mipiyuv_COLOR_0729,
    ov5645mipiyuv_COLOR_0730, ov5645mipiyuv_COLOR_0731, ov5645mipiyuv_COLOR_0732, ov5645mipiyuv_COLOR_0733, ov5645mipiyuv_COLOR_0734, ov5645mipiyuv_COLOR_0735, ov5645mipiyuv_COLOR_0736, ov5645mipiyuv_COLOR_0737, ov5645mipiyuv_COLOR_0738, ov5645mipiyuv_COLOR_0739,
    ov5645mipiyuv_COLOR_0740, ov5645mipiyuv_COLOR_0741, ov5645mipiyuv_COLOR_0742, ov5645mipiyuv_COLOR_0743, ov5645mipiyuv_COLOR_0744, ov5645mipiyuv_COLOR_0745, ov5645mipiyuv_COLOR_0746, ov5645mipiyuv_COLOR_0747, ov5645mipiyuv_COLOR_0748, ov5645mipiyuv_COLOR_0749,
    ov5645mipiyuv_COLOR_0750, ov5645mipiyuv_COLOR_0751, ov5645mipiyuv_COLOR_0752, ov5645mipiyuv_COLOR_0753, ov5645mipiyuv_COLOR_0754, ov5645mipiyuv_COLOR_0755, ov5645mipiyuv_COLOR_0756, ov5645mipiyuv_COLOR_0757, ov5645mipiyuv_COLOR_0758, ov5645mipiyuv_COLOR_0759,
    ov5645mipiyuv_COLOR_0760, ov5645mipiyuv_COLOR_0761, ov5645mipiyuv_COLOR_0762, ov5645mipiyuv_COLOR_0763, ov5645mipiyuv_COLOR_0764, ov5645mipiyuv_COLOR_0765, ov5645mipiyuv_COLOR_0766, ov5645mipiyuv_COLOR_0767, ov5645mipiyuv_COLOR_0768, ov5645mipiyuv_COLOR_0769,
    ov5645mipiyuv_COLOR_0770, ov5645mipiyuv_COLOR_0771, ov5645mipiyuv_COLOR_0772, ov5645mipiyuv_COLOR_0773, ov5645mipiyuv_COLOR_0774, ov5645mipiyuv_COLOR_0775, ov5645mipiyuv_COLOR_0776, ov5645mipiyuv_COLOR_0777, ov5645mipiyuv_COLOR_0778, ov5645mipiyuv_COLOR_0779,
    ov5645mipiyuv_COLOR_0780, ov5645mipiyuv_COLOR_0781, ov5645mipiyuv_COLOR_0782, ov5645mipiyuv_COLOR_0783, ov5645mipiyuv_COLOR_0784, ov5645mipiyuv_COLOR_0785, ov5645mipiyuv_COLOR_0786, ov5645mipiyuv_COLOR_0787, ov5645mipiyuv_COLOR_0788, ov5645mipiyuv_COLOR_0789,
    ov5645mipiyuv_COLOR_0790, ov5645mipiyuv_COLOR_0791, ov5645mipiyuv_COLOR_0792, ov5645mipiyuv_COLOR_0793, ov5645mipiyuv_COLOR_0794, ov5645mipiyuv_COLOR_0795, ov5645mipiyuv_COLOR_0796, ov5645mipiyuv_COLOR_0797, ov5645mipiyuv_COLOR_0798, ov5645mipiyuv_COLOR_0799,
    ov5645mipiyuv_COLOR_0800, ov5645mipiyuv_COLOR_0801, ov5645mipiyuv_COLOR_0802, ov5645mipiyuv_COLOR_0803, ov5645mipiyuv_COLOR_0804, ov5645mipiyuv_COLOR_0805, ov5645mipiyuv_COLOR_0806, ov5645mipiyuv_COLOR_0807, ov5645mipiyuv_COLOR_0808, ov5645mipiyuv_COLOR_0809,
    ov5645mipiyuv_COLOR_0810, ov5645mipiyuv_COLOR_0811, ov5645mipiyuv_COLOR_0812, ov5645mipiyuv_COLOR_0813, ov5645mipiyuv_COLOR_0814, ov5645mipiyuv_COLOR_0815, ov5645mipiyuv_COLOR_0816, ov5645mipiyuv_COLOR_0817, ov5645mipiyuv_COLOR_0818, ov5645mipiyuv_COLOR_0819,
    ov5645mipiyuv_COLOR_0820, ov5645mipiyuv_COLOR_0821, ov5645mipiyuv_COLOR_0822, ov5645mipiyuv_COLOR_0823, ov5645mipiyuv_COLOR_0824, ov5645mipiyuv_COLOR_0825, ov5645mipiyuv_COLOR_0826, ov5645mipiyuv_COLOR_0827, ov5645mipiyuv_COLOR_0828, ov5645mipiyuv_COLOR_0829,
    ov5645mipiyuv_COLOR_0830, ov5645mipiyuv_COLOR_0831, ov5645mipiyuv_COLOR_0832, ov5645mipiyuv_COLOR_0833, ov5645mipiyuv_COLOR_0834, ov5645mipiyuv_COLOR_0835, ov5645mipiyuv_COLOR_0836, ov5645mipiyuv_COLOR_0837, ov5645mipiyuv_COLOR_0838, ov5645mipiyuv_COLOR_0839,
    ov5645mipiyuv_COLOR_0840, ov5645mipiyuv_COLOR_0841, ov5645mipiyuv_COLOR_0842, ov5645mipiyuv_COLOR_0843, ov5645mipiyuv_COLOR_0844, ov5645mipiyuv_COLOR_0845, ov5645mipiyuv_COLOR_0846, ov5645mipiyuv_COLOR_0847, ov5645mipiyuv_COLOR_0848, ov5645mipiyuv_COLOR_0849,
    ov5645mipiyuv_COLOR_0850, ov5645mipiyuv_COLOR_0851, ov5645mipiyuv_COLOR_0852, ov5645mipiyuv_COLOR_0853, ov5645mipiyuv_COLOR_0854, ov5645mipiyuv_COLOR_0855, ov5645mipiyuv_COLOR_0856, ov5645mipiyuv_COLOR_0857, ov5645mipiyuv_COLOR_0858, ov5645mipiyuv_COLOR_0859,
    ov5645mipiyuv_COLOR_0860, ov5645mipiyuv_COLOR_0861, ov5645mipiyuv_COLOR_0862, ov5645mipiyuv_COLOR_0863, ov5645mipiyuv_COLOR_0864, ov5645mipiyuv_COLOR_0865, ov5645mipiyuv_COLOR_0866, ov5645mipiyuv_COLOR_0867, ov5645mipiyuv_COLOR_0868, ov5645mipiyuv_COLOR_0869,
    ov5645mipiyuv_COLOR_0870, ov5645mipiyuv_COLOR_0871, ov5645mipiyuv_COLOR_0872, ov5645mipiyuv_COLOR_0873, ov5645mipiyuv_COLOR_0874, ov5645mipiyuv_COLOR_0875, ov5645mipiyuv_COLOR_0876, ov5645mipiyuv_COLOR_0877, ov5645mipiyuv_COLOR_0878, ov5645mipiyuv_COLOR_0879,
    ov5645mipiyuv_COLOR_0880, ov5645mipiyuv_COLOR_0881, ov5645mipiyuv_COLOR_0882, ov5645mipiyuv_COLOR_0883, ov5645mipiyuv_COLOR_0884, ov5645mipiyuv_COLOR_0885, ov5645mipiyuv_COLOR_0886, ov5645mipiyuv_COLOR_0887, ov5645mipiyuv_COLOR_0888, ov5645mipiyuv_COLOR_0889,
    ov5645mipiyuv_COLOR_0890, ov5645mipiyuv_COLOR_0891, ov5645mipiyuv_COLOR_0892, ov5645mipiyuv_COLOR_0893, ov5645mipiyuv_COLOR_0894, ov5645mipiyuv_COLOR_0895, ov5645mipiyuv_COLOR_0896, ov5645mipiyuv_COLOR_0897, ov5645mipiyuv_COLOR_0898, ov5645mipiyuv_COLOR_0899,
    ov5645mipiyuv_COLOR_0900, ov5645mipiyuv_COLOR_0901, ov5645mipiyuv_COLOR_0902, ov5645mipiyuv_COLOR_0903, ov5645mipiyuv_COLOR_0904, ov5645mipiyuv_COLOR_0905, ov5645mipiyuv_COLOR_0906, ov5645mipiyuv_COLOR_0907, ov5645mipiyuv_COLOR_0908, ov5645mipiyuv_COLOR_0909,
    ov5645mipiyuv_COLOR_0910, ov5645mipiyuv_COLOR_0911, ov5645mipiyuv_COLOR_0912, ov5645mipiyuv_COLOR_0913, ov5645mipiyuv_COLOR_0914, ov5645mipiyuv_COLOR_0915, ov5645mipiyuv_COLOR_0916, ov5645mipiyuv_COLOR_0917, ov5645mipiyuv_COLOR_0918, ov5645mipiyuv_COLOR_0919,
    ov5645mipiyuv_COLOR_0920, ov5645mipiyuv_COLOR_0921, ov5645mipiyuv_COLOR_0922, ov5645mipiyuv_COLOR_0923, ov5645mipiyuv_COLOR_0924, ov5645mipiyuv_COLOR_0925, ov5645mipiyuv_COLOR_0926, ov5645mipiyuv_COLOR_0927, ov5645mipiyuv_COLOR_0928, ov5645mipiyuv_COLOR_0929,
    ov5645mipiyuv_COLOR_0930, ov5645mipiyuv_COLOR_0931, ov5645mipiyuv_COLOR_0932, ov5645mipiyuv_COLOR_0933, ov5645mipiyuv_COLOR_0934, ov5645mipiyuv_COLOR_0935, ov5645mipiyuv_COLOR_0936, ov5645mipiyuv_COLOR_0937, ov5645mipiyuv_COLOR_0938, ov5645mipiyuv_COLOR_0939,
    ov5645mipiyuv_COLOR_0940, ov5645mipiyuv_COLOR_0941, ov5645mipiyuv_COLOR_0942, ov5645mipiyuv_COLOR_0943, ov5645mipiyuv_COLOR_0944, ov5645mipiyuv_COLOR_0945, ov5645mipiyuv_COLOR_0946, ov5645mipiyuv_COLOR_0947, ov5645mipiyuv_COLOR_0948, ov5645mipiyuv_COLOR_0949,
    ov5645mipiyuv_COLOR_0950, ov5645mipiyuv_COLOR_0951, ov5645mipiyuv_COLOR_0952, ov5645mipiyuv_COLOR_0953, ov5645mipiyuv_COLOR_0954, ov5645mipiyuv_COLOR_0955, ov5645mipiyuv_COLOR_0956, ov5645mipiyuv_COLOR_0957, ov5645mipiyuv_COLOR_0958, ov5645mipiyuv_COLOR_0959,
},
.PCA = {
    ov5645mipiyuv_PCA_0000,
},
.COLOR_PARAM = {
    ov5645mipiyuv_COLOR_PARAM_0000, ov5645mipiyuv_COLOR_PARAM_0001, ov5645mipiyuv_COLOR_PARAM_0002, ov5645mipiyuv_COLOR_PARAM_0003, ov5645mipiyuv_COLOR_PARAM_0004, ov5645mipiyuv_COLOR_PARAM_0005, ov5645mipiyuv_COLOR_PARAM_0006, ov5645mipiyuv_COLOR_PARAM_0007, ov5645mipiyuv_COLOR_PARAM_0008, ov5645mipiyuv_COLOR_PARAM_0009,
    ov5645mipiyuv_COLOR_PARAM_0010, ov5645mipiyuv_COLOR_PARAM_0011, ov5645mipiyuv_COLOR_PARAM_0012, ov5645mipiyuv_COLOR_PARAM_0013, ov5645mipiyuv_COLOR_PARAM_0014, ov5645mipiyuv_COLOR_PARAM_0015,
},
