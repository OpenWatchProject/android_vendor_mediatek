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
    s5k4e6mipiraw_CCM_0000, s5k4e6mipiraw_CCM_0001, s5k4e6mipiraw_CCM_0002, s5k4e6mipiraw_CCM_0003, s5k4e6mipiraw_CCM_0004, s5k4e6mipiraw_CCM_0005, s5k4e6mipiraw_CCM_0006, s5k4e6mipiraw_CCM_0007, s5k4e6mipiraw_CCM_0008, s5k4e6mipiraw_CCM_0009,
    s5k4e6mipiraw_CCM_0010, s5k4e6mipiraw_CCM_0011, s5k4e6mipiraw_CCM_0012, s5k4e6mipiraw_CCM_0013, s5k4e6mipiraw_CCM_0014, s5k4e6mipiraw_CCM_0015, s5k4e6mipiraw_CCM_0016, s5k4e6mipiraw_CCM_0017, s5k4e6mipiraw_CCM_0018, s5k4e6mipiraw_CCM_0019,
    s5k4e6mipiraw_CCM_0020, s5k4e6mipiraw_CCM_0021, s5k4e6mipiraw_CCM_0022, s5k4e6mipiraw_CCM_0023, s5k4e6mipiraw_CCM_0024, s5k4e6mipiraw_CCM_0025, s5k4e6mipiraw_CCM_0026, s5k4e6mipiraw_CCM_0027, s5k4e6mipiraw_CCM_0028, s5k4e6mipiraw_CCM_0029,
    s5k4e6mipiraw_CCM_0030, s5k4e6mipiraw_CCM_0031, s5k4e6mipiraw_CCM_0032, s5k4e6mipiraw_CCM_0033, s5k4e6mipiraw_CCM_0034, s5k4e6mipiraw_CCM_0035, s5k4e6mipiraw_CCM_0036, s5k4e6mipiraw_CCM_0037, s5k4e6mipiraw_CCM_0038, s5k4e6mipiraw_CCM_0039,
    s5k4e6mipiraw_CCM_0040, s5k4e6mipiraw_CCM_0041, s5k4e6mipiraw_CCM_0042, s5k4e6mipiraw_CCM_0043, s5k4e6mipiraw_CCM_0044, s5k4e6mipiraw_CCM_0045, s5k4e6mipiraw_CCM_0046, s5k4e6mipiraw_CCM_0047, s5k4e6mipiraw_CCM_0048, s5k4e6mipiraw_CCM_0049,
    s5k4e6mipiraw_CCM_0050, s5k4e6mipiraw_CCM_0051, s5k4e6mipiraw_CCM_0052, s5k4e6mipiraw_CCM_0053, s5k4e6mipiraw_CCM_0054, s5k4e6mipiraw_CCM_0055, s5k4e6mipiraw_CCM_0056, s5k4e6mipiraw_CCM_0057, s5k4e6mipiraw_CCM_0058, s5k4e6mipiraw_CCM_0059,
    s5k4e6mipiraw_CCM_0060, s5k4e6mipiraw_CCM_0061, s5k4e6mipiraw_CCM_0062, s5k4e6mipiraw_CCM_0063, s5k4e6mipiraw_CCM_0064, s5k4e6mipiraw_CCM_0065, s5k4e6mipiraw_CCM_0066, s5k4e6mipiraw_CCM_0067, s5k4e6mipiraw_CCM_0068, s5k4e6mipiraw_CCM_0069,
    s5k4e6mipiraw_CCM_0070, s5k4e6mipiraw_CCM_0071, s5k4e6mipiraw_CCM_0072, s5k4e6mipiraw_CCM_0073, s5k4e6mipiraw_CCM_0074, s5k4e6mipiraw_CCM_0075, s5k4e6mipiraw_CCM_0076, s5k4e6mipiraw_CCM_0077, s5k4e6mipiraw_CCM_0078, s5k4e6mipiraw_CCM_0079,
    s5k4e6mipiraw_CCM_0080, s5k4e6mipiraw_CCM_0081, s5k4e6mipiraw_CCM_0082, s5k4e6mipiraw_CCM_0083, s5k4e6mipiraw_CCM_0084, s5k4e6mipiraw_CCM_0085, s5k4e6mipiraw_CCM_0086, s5k4e6mipiraw_CCM_0087, s5k4e6mipiraw_CCM_0088, s5k4e6mipiraw_CCM_0089,
    s5k4e6mipiraw_CCM_0090, s5k4e6mipiraw_CCM_0091, s5k4e6mipiraw_CCM_0092, s5k4e6mipiraw_CCM_0093, s5k4e6mipiraw_CCM_0094, s5k4e6mipiraw_CCM_0095, s5k4e6mipiraw_CCM_0096, s5k4e6mipiraw_CCM_0097, s5k4e6mipiraw_CCM_0098, s5k4e6mipiraw_CCM_0099,
    s5k4e6mipiraw_CCM_0100, s5k4e6mipiraw_CCM_0101, s5k4e6mipiraw_CCM_0102, s5k4e6mipiraw_CCM_0103, s5k4e6mipiraw_CCM_0104, s5k4e6mipiraw_CCM_0105, s5k4e6mipiraw_CCM_0106, s5k4e6mipiraw_CCM_0107, s5k4e6mipiraw_CCM_0108, s5k4e6mipiraw_CCM_0109,
    s5k4e6mipiraw_CCM_0110, s5k4e6mipiraw_CCM_0111, s5k4e6mipiraw_CCM_0112, s5k4e6mipiraw_CCM_0113, s5k4e6mipiraw_CCM_0114, s5k4e6mipiraw_CCM_0115, s5k4e6mipiraw_CCM_0116, s5k4e6mipiraw_CCM_0117, s5k4e6mipiraw_CCM_0118, s5k4e6mipiraw_CCM_0119,
    s5k4e6mipiraw_CCM_0120, s5k4e6mipiraw_CCM_0121, s5k4e6mipiraw_CCM_0122, s5k4e6mipiraw_CCM_0123, s5k4e6mipiraw_CCM_0124, s5k4e6mipiraw_CCM_0125, s5k4e6mipiraw_CCM_0126, s5k4e6mipiraw_CCM_0127, s5k4e6mipiraw_CCM_0128, s5k4e6mipiraw_CCM_0129,
    s5k4e6mipiraw_CCM_0130, s5k4e6mipiraw_CCM_0131, s5k4e6mipiraw_CCM_0132, s5k4e6mipiraw_CCM_0133, s5k4e6mipiraw_CCM_0134, s5k4e6mipiraw_CCM_0135, s5k4e6mipiraw_CCM_0136, s5k4e6mipiraw_CCM_0137, s5k4e6mipiraw_CCM_0138, s5k4e6mipiraw_CCM_0139,
    s5k4e6mipiraw_CCM_0140, s5k4e6mipiraw_CCM_0141, s5k4e6mipiraw_CCM_0142, s5k4e6mipiraw_CCM_0143, s5k4e6mipiraw_CCM_0144, s5k4e6mipiraw_CCM_0145, s5k4e6mipiraw_CCM_0146, s5k4e6mipiraw_CCM_0147, s5k4e6mipiraw_CCM_0148, s5k4e6mipiraw_CCM_0149,
    s5k4e6mipiraw_CCM_0150, s5k4e6mipiraw_CCM_0151, s5k4e6mipiraw_CCM_0152, s5k4e6mipiraw_CCM_0153, s5k4e6mipiraw_CCM_0154, s5k4e6mipiraw_CCM_0155, s5k4e6mipiraw_CCM_0156, s5k4e6mipiraw_CCM_0157, s5k4e6mipiraw_CCM_0158, s5k4e6mipiraw_CCM_0159,
    s5k4e6mipiraw_CCM_0160, s5k4e6mipiraw_CCM_0161, s5k4e6mipiraw_CCM_0162, s5k4e6mipiraw_CCM_0163, s5k4e6mipiraw_CCM_0164, s5k4e6mipiraw_CCM_0165, s5k4e6mipiraw_CCM_0166, s5k4e6mipiraw_CCM_0167, s5k4e6mipiraw_CCM_0168, s5k4e6mipiraw_CCM_0169,
    s5k4e6mipiraw_CCM_0170, s5k4e6mipiraw_CCM_0171, s5k4e6mipiraw_CCM_0172, s5k4e6mipiraw_CCM_0173, s5k4e6mipiraw_CCM_0174, s5k4e6mipiraw_CCM_0175, s5k4e6mipiraw_CCM_0176, s5k4e6mipiraw_CCM_0177, s5k4e6mipiraw_CCM_0178, s5k4e6mipiraw_CCM_0179,
    s5k4e6mipiraw_CCM_0180, s5k4e6mipiraw_CCM_0181, s5k4e6mipiraw_CCM_0182, s5k4e6mipiraw_CCM_0183, s5k4e6mipiraw_CCM_0184, s5k4e6mipiraw_CCM_0185, s5k4e6mipiraw_CCM_0186, s5k4e6mipiraw_CCM_0187, s5k4e6mipiraw_CCM_0188, s5k4e6mipiraw_CCM_0189,
    s5k4e6mipiraw_CCM_0190, s5k4e6mipiraw_CCM_0191, s5k4e6mipiraw_CCM_0192, s5k4e6mipiraw_CCM_0193, s5k4e6mipiraw_CCM_0194, s5k4e6mipiraw_CCM_0195, s5k4e6mipiraw_CCM_0196, s5k4e6mipiraw_CCM_0197, s5k4e6mipiraw_CCM_0198, s5k4e6mipiraw_CCM_0199,
    s5k4e6mipiraw_CCM_0200, s5k4e6mipiraw_CCM_0201, s5k4e6mipiraw_CCM_0202, s5k4e6mipiraw_CCM_0203, s5k4e6mipiraw_CCM_0204, s5k4e6mipiraw_CCM_0205, s5k4e6mipiraw_CCM_0206, s5k4e6mipiraw_CCM_0207, s5k4e6mipiraw_CCM_0208, s5k4e6mipiraw_CCM_0209,
    s5k4e6mipiraw_CCM_0210, s5k4e6mipiraw_CCM_0211, s5k4e6mipiraw_CCM_0212, s5k4e6mipiraw_CCM_0213, s5k4e6mipiraw_CCM_0214, s5k4e6mipiraw_CCM_0215, s5k4e6mipiraw_CCM_0216, s5k4e6mipiraw_CCM_0217, s5k4e6mipiraw_CCM_0218, s5k4e6mipiraw_CCM_0219,
    s5k4e6mipiraw_CCM_0220, s5k4e6mipiraw_CCM_0221, s5k4e6mipiraw_CCM_0222, s5k4e6mipiraw_CCM_0223, s5k4e6mipiraw_CCM_0224, s5k4e6mipiraw_CCM_0225, s5k4e6mipiraw_CCM_0226, s5k4e6mipiraw_CCM_0227, s5k4e6mipiraw_CCM_0228, s5k4e6mipiraw_CCM_0229,
    s5k4e6mipiraw_CCM_0230, s5k4e6mipiraw_CCM_0231, s5k4e6mipiraw_CCM_0232, s5k4e6mipiraw_CCM_0233, s5k4e6mipiraw_CCM_0234, s5k4e6mipiraw_CCM_0235, s5k4e6mipiraw_CCM_0236, s5k4e6mipiraw_CCM_0237, s5k4e6mipiraw_CCM_0238, s5k4e6mipiraw_CCM_0239,
},
.COLOR = {
    s5k4e6mipiraw_COLOR_0000, s5k4e6mipiraw_COLOR_0001, s5k4e6mipiraw_COLOR_0002, s5k4e6mipiraw_COLOR_0003, s5k4e6mipiraw_COLOR_0004, s5k4e6mipiraw_COLOR_0005, s5k4e6mipiraw_COLOR_0006, s5k4e6mipiraw_COLOR_0007, s5k4e6mipiraw_COLOR_0008, s5k4e6mipiraw_COLOR_0009,
    s5k4e6mipiraw_COLOR_0010, s5k4e6mipiraw_COLOR_0011, s5k4e6mipiraw_COLOR_0012, s5k4e6mipiraw_COLOR_0013, s5k4e6mipiraw_COLOR_0014, s5k4e6mipiraw_COLOR_0015, s5k4e6mipiraw_COLOR_0016, s5k4e6mipiraw_COLOR_0017, s5k4e6mipiraw_COLOR_0018, s5k4e6mipiraw_COLOR_0019,
    s5k4e6mipiraw_COLOR_0020, s5k4e6mipiraw_COLOR_0021, s5k4e6mipiraw_COLOR_0022, s5k4e6mipiraw_COLOR_0023, s5k4e6mipiraw_COLOR_0024, s5k4e6mipiraw_COLOR_0025, s5k4e6mipiraw_COLOR_0026, s5k4e6mipiraw_COLOR_0027, s5k4e6mipiraw_COLOR_0028, s5k4e6mipiraw_COLOR_0029,
    s5k4e6mipiraw_COLOR_0030, s5k4e6mipiraw_COLOR_0031, s5k4e6mipiraw_COLOR_0032, s5k4e6mipiraw_COLOR_0033, s5k4e6mipiraw_COLOR_0034, s5k4e6mipiraw_COLOR_0035, s5k4e6mipiraw_COLOR_0036, s5k4e6mipiraw_COLOR_0037, s5k4e6mipiraw_COLOR_0038, s5k4e6mipiraw_COLOR_0039,
    s5k4e6mipiraw_COLOR_0040, s5k4e6mipiraw_COLOR_0041, s5k4e6mipiraw_COLOR_0042, s5k4e6mipiraw_COLOR_0043, s5k4e6mipiraw_COLOR_0044, s5k4e6mipiraw_COLOR_0045, s5k4e6mipiraw_COLOR_0046, s5k4e6mipiraw_COLOR_0047, s5k4e6mipiraw_COLOR_0048, s5k4e6mipiraw_COLOR_0049,
    s5k4e6mipiraw_COLOR_0050, s5k4e6mipiraw_COLOR_0051, s5k4e6mipiraw_COLOR_0052, s5k4e6mipiraw_COLOR_0053, s5k4e6mipiraw_COLOR_0054, s5k4e6mipiraw_COLOR_0055, s5k4e6mipiraw_COLOR_0056, s5k4e6mipiraw_COLOR_0057, s5k4e6mipiraw_COLOR_0058, s5k4e6mipiraw_COLOR_0059,
    s5k4e6mipiraw_COLOR_0060, s5k4e6mipiraw_COLOR_0061, s5k4e6mipiraw_COLOR_0062, s5k4e6mipiraw_COLOR_0063, s5k4e6mipiraw_COLOR_0064, s5k4e6mipiraw_COLOR_0065, s5k4e6mipiraw_COLOR_0066, s5k4e6mipiraw_COLOR_0067, s5k4e6mipiraw_COLOR_0068, s5k4e6mipiraw_COLOR_0069,
    s5k4e6mipiraw_COLOR_0070, s5k4e6mipiraw_COLOR_0071, s5k4e6mipiraw_COLOR_0072, s5k4e6mipiraw_COLOR_0073, s5k4e6mipiraw_COLOR_0074, s5k4e6mipiraw_COLOR_0075, s5k4e6mipiraw_COLOR_0076, s5k4e6mipiraw_COLOR_0077, s5k4e6mipiraw_COLOR_0078, s5k4e6mipiraw_COLOR_0079,
    s5k4e6mipiraw_COLOR_0080, s5k4e6mipiraw_COLOR_0081, s5k4e6mipiraw_COLOR_0082, s5k4e6mipiraw_COLOR_0083, s5k4e6mipiraw_COLOR_0084, s5k4e6mipiraw_COLOR_0085, s5k4e6mipiraw_COLOR_0086, s5k4e6mipiraw_COLOR_0087, s5k4e6mipiraw_COLOR_0088, s5k4e6mipiraw_COLOR_0089,
    s5k4e6mipiraw_COLOR_0090, s5k4e6mipiraw_COLOR_0091, s5k4e6mipiraw_COLOR_0092, s5k4e6mipiraw_COLOR_0093, s5k4e6mipiraw_COLOR_0094, s5k4e6mipiraw_COLOR_0095, s5k4e6mipiraw_COLOR_0096, s5k4e6mipiraw_COLOR_0097, s5k4e6mipiraw_COLOR_0098, s5k4e6mipiraw_COLOR_0099,
    s5k4e6mipiraw_COLOR_0100, s5k4e6mipiraw_COLOR_0101, s5k4e6mipiraw_COLOR_0102, s5k4e6mipiraw_COLOR_0103, s5k4e6mipiraw_COLOR_0104, s5k4e6mipiraw_COLOR_0105, s5k4e6mipiraw_COLOR_0106, s5k4e6mipiraw_COLOR_0107, s5k4e6mipiraw_COLOR_0108, s5k4e6mipiraw_COLOR_0109,
    s5k4e6mipiraw_COLOR_0110, s5k4e6mipiraw_COLOR_0111, s5k4e6mipiraw_COLOR_0112, s5k4e6mipiraw_COLOR_0113, s5k4e6mipiraw_COLOR_0114, s5k4e6mipiraw_COLOR_0115, s5k4e6mipiraw_COLOR_0116, s5k4e6mipiraw_COLOR_0117, s5k4e6mipiraw_COLOR_0118, s5k4e6mipiraw_COLOR_0119,
    s5k4e6mipiraw_COLOR_0120, s5k4e6mipiraw_COLOR_0121, s5k4e6mipiraw_COLOR_0122, s5k4e6mipiraw_COLOR_0123, s5k4e6mipiraw_COLOR_0124, s5k4e6mipiraw_COLOR_0125, s5k4e6mipiraw_COLOR_0126, s5k4e6mipiraw_COLOR_0127, s5k4e6mipiraw_COLOR_0128, s5k4e6mipiraw_COLOR_0129,
    s5k4e6mipiraw_COLOR_0130, s5k4e6mipiraw_COLOR_0131, s5k4e6mipiraw_COLOR_0132, s5k4e6mipiraw_COLOR_0133, s5k4e6mipiraw_COLOR_0134, s5k4e6mipiraw_COLOR_0135, s5k4e6mipiraw_COLOR_0136, s5k4e6mipiraw_COLOR_0137, s5k4e6mipiraw_COLOR_0138, s5k4e6mipiraw_COLOR_0139,
    s5k4e6mipiraw_COLOR_0140, s5k4e6mipiraw_COLOR_0141, s5k4e6mipiraw_COLOR_0142, s5k4e6mipiraw_COLOR_0143, s5k4e6mipiraw_COLOR_0144, s5k4e6mipiraw_COLOR_0145, s5k4e6mipiraw_COLOR_0146, s5k4e6mipiraw_COLOR_0147, s5k4e6mipiraw_COLOR_0148, s5k4e6mipiraw_COLOR_0149,
    s5k4e6mipiraw_COLOR_0150, s5k4e6mipiraw_COLOR_0151, s5k4e6mipiraw_COLOR_0152, s5k4e6mipiraw_COLOR_0153, s5k4e6mipiraw_COLOR_0154, s5k4e6mipiraw_COLOR_0155, s5k4e6mipiraw_COLOR_0156, s5k4e6mipiraw_COLOR_0157, s5k4e6mipiraw_COLOR_0158, s5k4e6mipiraw_COLOR_0159,
    s5k4e6mipiraw_COLOR_0160, s5k4e6mipiraw_COLOR_0161, s5k4e6mipiraw_COLOR_0162, s5k4e6mipiraw_COLOR_0163, s5k4e6mipiraw_COLOR_0164, s5k4e6mipiraw_COLOR_0165, s5k4e6mipiraw_COLOR_0166, s5k4e6mipiraw_COLOR_0167, s5k4e6mipiraw_COLOR_0168, s5k4e6mipiraw_COLOR_0169,
    s5k4e6mipiraw_COLOR_0170, s5k4e6mipiraw_COLOR_0171, s5k4e6mipiraw_COLOR_0172, s5k4e6mipiraw_COLOR_0173, s5k4e6mipiraw_COLOR_0174, s5k4e6mipiraw_COLOR_0175, s5k4e6mipiraw_COLOR_0176, s5k4e6mipiraw_COLOR_0177, s5k4e6mipiraw_COLOR_0178, s5k4e6mipiraw_COLOR_0179,
    s5k4e6mipiraw_COLOR_0180, s5k4e6mipiraw_COLOR_0181, s5k4e6mipiraw_COLOR_0182, s5k4e6mipiraw_COLOR_0183, s5k4e6mipiraw_COLOR_0184, s5k4e6mipiraw_COLOR_0185, s5k4e6mipiraw_COLOR_0186, s5k4e6mipiraw_COLOR_0187, s5k4e6mipiraw_COLOR_0188, s5k4e6mipiraw_COLOR_0189,
    s5k4e6mipiraw_COLOR_0190, s5k4e6mipiraw_COLOR_0191, s5k4e6mipiraw_COLOR_0192, s5k4e6mipiraw_COLOR_0193, s5k4e6mipiraw_COLOR_0194, s5k4e6mipiraw_COLOR_0195, s5k4e6mipiraw_COLOR_0196, s5k4e6mipiraw_COLOR_0197, s5k4e6mipiraw_COLOR_0198, s5k4e6mipiraw_COLOR_0199,
    s5k4e6mipiraw_COLOR_0200, s5k4e6mipiraw_COLOR_0201, s5k4e6mipiraw_COLOR_0202, s5k4e6mipiraw_COLOR_0203, s5k4e6mipiraw_COLOR_0204, s5k4e6mipiraw_COLOR_0205, s5k4e6mipiraw_COLOR_0206, s5k4e6mipiraw_COLOR_0207, s5k4e6mipiraw_COLOR_0208, s5k4e6mipiraw_COLOR_0209,
    s5k4e6mipiraw_COLOR_0210, s5k4e6mipiraw_COLOR_0211, s5k4e6mipiraw_COLOR_0212, s5k4e6mipiraw_COLOR_0213, s5k4e6mipiraw_COLOR_0214, s5k4e6mipiraw_COLOR_0215, s5k4e6mipiraw_COLOR_0216, s5k4e6mipiraw_COLOR_0217, s5k4e6mipiraw_COLOR_0218, s5k4e6mipiraw_COLOR_0219,
    s5k4e6mipiraw_COLOR_0220, s5k4e6mipiraw_COLOR_0221, s5k4e6mipiraw_COLOR_0222, s5k4e6mipiraw_COLOR_0223, s5k4e6mipiraw_COLOR_0224, s5k4e6mipiraw_COLOR_0225, s5k4e6mipiraw_COLOR_0226, s5k4e6mipiraw_COLOR_0227, s5k4e6mipiraw_COLOR_0228, s5k4e6mipiraw_COLOR_0229,
    s5k4e6mipiraw_COLOR_0230, s5k4e6mipiraw_COLOR_0231, s5k4e6mipiraw_COLOR_0232, s5k4e6mipiraw_COLOR_0233, s5k4e6mipiraw_COLOR_0234, s5k4e6mipiraw_COLOR_0235, s5k4e6mipiraw_COLOR_0236, s5k4e6mipiraw_COLOR_0237, s5k4e6mipiraw_COLOR_0238, s5k4e6mipiraw_COLOR_0239,
    s5k4e6mipiraw_COLOR_0240, s5k4e6mipiraw_COLOR_0241, s5k4e6mipiraw_COLOR_0242, s5k4e6mipiraw_COLOR_0243, s5k4e6mipiraw_COLOR_0244, s5k4e6mipiraw_COLOR_0245, s5k4e6mipiraw_COLOR_0246, s5k4e6mipiraw_COLOR_0247, s5k4e6mipiraw_COLOR_0248, s5k4e6mipiraw_COLOR_0249,
    s5k4e6mipiraw_COLOR_0250, s5k4e6mipiraw_COLOR_0251, s5k4e6mipiraw_COLOR_0252, s5k4e6mipiraw_COLOR_0253, s5k4e6mipiraw_COLOR_0254, s5k4e6mipiraw_COLOR_0255, s5k4e6mipiraw_COLOR_0256, s5k4e6mipiraw_COLOR_0257, s5k4e6mipiraw_COLOR_0258, s5k4e6mipiraw_COLOR_0259,
    s5k4e6mipiraw_COLOR_0260, s5k4e6mipiraw_COLOR_0261, s5k4e6mipiraw_COLOR_0262, s5k4e6mipiraw_COLOR_0263, s5k4e6mipiraw_COLOR_0264, s5k4e6mipiraw_COLOR_0265, s5k4e6mipiraw_COLOR_0266, s5k4e6mipiraw_COLOR_0267, s5k4e6mipiraw_COLOR_0268, s5k4e6mipiraw_COLOR_0269,
    s5k4e6mipiraw_COLOR_0270, s5k4e6mipiraw_COLOR_0271, s5k4e6mipiraw_COLOR_0272, s5k4e6mipiraw_COLOR_0273, s5k4e6mipiraw_COLOR_0274, s5k4e6mipiraw_COLOR_0275, s5k4e6mipiraw_COLOR_0276, s5k4e6mipiraw_COLOR_0277, s5k4e6mipiraw_COLOR_0278, s5k4e6mipiraw_COLOR_0279,
    s5k4e6mipiraw_COLOR_0280, s5k4e6mipiraw_COLOR_0281, s5k4e6mipiraw_COLOR_0282, s5k4e6mipiraw_COLOR_0283, s5k4e6mipiraw_COLOR_0284, s5k4e6mipiraw_COLOR_0285, s5k4e6mipiraw_COLOR_0286, s5k4e6mipiraw_COLOR_0287, s5k4e6mipiraw_COLOR_0288, s5k4e6mipiraw_COLOR_0289,
    s5k4e6mipiraw_COLOR_0290, s5k4e6mipiraw_COLOR_0291, s5k4e6mipiraw_COLOR_0292, s5k4e6mipiraw_COLOR_0293, s5k4e6mipiraw_COLOR_0294, s5k4e6mipiraw_COLOR_0295, s5k4e6mipiraw_COLOR_0296, s5k4e6mipiraw_COLOR_0297, s5k4e6mipiraw_COLOR_0298, s5k4e6mipiraw_COLOR_0299,
    s5k4e6mipiraw_COLOR_0300, s5k4e6mipiraw_COLOR_0301, s5k4e6mipiraw_COLOR_0302, s5k4e6mipiraw_COLOR_0303, s5k4e6mipiraw_COLOR_0304, s5k4e6mipiraw_COLOR_0305, s5k4e6mipiraw_COLOR_0306, s5k4e6mipiraw_COLOR_0307, s5k4e6mipiraw_COLOR_0308, s5k4e6mipiraw_COLOR_0309,
    s5k4e6mipiraw_COLOR_0310, s5k4e6mipiraw_COLOR_0311, s5k4e6mipiraw_COLOR_0312, s5k4e6mipiraw_COLOR_0313, s5k4e6mipiraw_COLOR_0314, s5k4e6mipiraw_COLOR_0315, s5k4e6mipiraw_COLOR_0316, s5k4e6mipiraw_COLOR_0317, s5k4e6mipiraw_COLOR_0318, s5k4e6mipiraw_COLOR_0319,
    s5k4e6mipiraw_COLOR_0320, s5k4e6mipiraw_COLOR_0321, s5k4e6mipiraw_COLOR_0322, s5k4e6mipiraw_COLOR_0323, s5k4e6mipiraw_COLOR_0324, s5k4e6mipiraw_COLOR_0325, s5k4e6mipiraw_COLOR_0326, s5k4e6mipiraw_COLOR_0327, s5k4e6mipiraw_COLOR_0328, s5k4e6mipiraw_COLOR_0329,
    s5k4e6mipiraw_COLOR_0330, s5k4e6mipiraw_COLOR_0331, s5k4e6mipiraw_COLOR_0332, s5k4e6mipiraw_COLOR_0333, s5k4e6mipiraw_COLOR_0334, s5k4e6mipiraw_COLOR_0335, s5k4e6mipiraw_COLOR_0336, s5k4e6mipiraw_COLOR_0337, s5k4e6mipiraw_COLOR_0338, s5k4e6mipiraw_COLOR_0339,
    s5k4e6mipiraw_COLOR_0340, s5k4e6mipiraw_COLOR_0341, s5k4e6mipiraw_COLOR_0342, s5k4e6mipiraw_COLOR_0343, s5k4e6mipiraw_COLOR_0344, s5k4e6mipiraw_COLOR_0345, s5k4e6mipiraw_COLOR_0346, s5k4e6mipiraw_COLOR_0347, s5k4e6mipiraw_COLOR_0348, s5k4e6mipiraw_COLOR_0349,
    s5k4e6mipiraw_COLOR_0350, s5k4e6mipiraw_COLOR_0351, s5k4e6mipiraw_COLOR_0352, s5k4e6mipiraw_COLOR_0353, s5k4e6mipiraw_COLOR_0354, s5k4e6mipiraw_COLOR_0355, s5k4e6mipiraw_COLOR_0356, s5k4e6mipiraw_COLOR_0357, s5k4e6mipiraw_COLOR_0358, s5k4e6mipiraw_COLOR_0359,
    s5k4e6mipiraw_COLOR_0360, s5k4e6mipiraw_COLOR_0361, s5k4e6mipiraw_COLOR_0362, s5k4e6mipiraw_COLOR_0363, s5k4e6mipiraw_COLOR_0364, s5k4e6mipiraw_COLOR_0365, s5k4e6mipiraw_COLOR_0366, s5k4e6mipiraw_COLOR_0367, s5k4e6mipiraw_COLOR_0368, s5k4e6mipiraw_COLOR_0369,
    s5k4e6mipiraw_COLOR_0370, s5k4e6mipiraw_COLOR_0371, s5k4e6mipiraw_COLOR_0372, s5k4e6mipiraw_COLOR_0373, s5k4e6mipiraw_COLOR_0374, s5k4e6mipiraw_COLOR_0375, s5k4e6mipiraw_COLOR_0376, s5k4e6mipiraw_COLOR_0377, s5k4e6mipiraw_COLOR_0378, s5k4e6mipiraw_COLOR_0379,
    s5k4e6mipiraw_COLOR_0380, s5k4e6mipiraw_COLOR_0381, s5k4e6mipiraw_COLOR_0382, s5k4e6mipiraw_COLOR_0383, s5k4e6mipiraw_COLOR_0384, s5k4e6mipiraw_COLOR_0385, s5k4e6mipiraw_COLOR_0386, s5k4e6mipiraw_COLOR_0387, s5k4e6mipiraw_COLOR_0388, s5k4e6mipiraw_COLOR_0389,
    s5k4e6mipiraw_COLOR_0390, s5k4e6mipiraw_COLOR_0391, s5k4e6mipiraw_COLOR_0392, s5k4e6mipiraw_COLOR_0393, s5k4e6mipiraw_COLOR_0394, s5k4e6mipiraw_COLOR_0395, s5k4e6mipiraw_COLOR_0396, s5k4e6mipiraw_COLOR_0397, s5k4e6mipiraw_COLOR_0398, s5k4e6mipiraw_COLOR_0399,
    s5k4e6mipiraw_COLOR_0400, s5k4e6mipiraw_COLOR_0401, s5k4e6mipiraw_COLOR_0402, s5k4e6mipiraw_COLOR_0403, s5k4e6mipiraw_COLOR_0404, s5k4e6mipiraw_COLOR_0405, s5k4e6mipiraw_COLOR_0406, s5k4e6mipiraw_COLOR_0407, s5k4e6mipiraw_COLOR_0408, s5k4e6mipiraw_COLOR_0409,
    s5k4e6mipiraw_COLOR_0410, s5k4e6mipiraw_COLOR_0411, s5k4e6mipiraw_COLOR_0412, s5k4e6mipiraw_COLOR_0413, s5k4e6mipiraw_COLOR_0414, s5k4e6mipiraw_COLOR_0415, s5k4e6mipiraw_COLOR_0416, s5k4e6mipiraw_COLOR_0417, s5k4e6mipiraw_COLOR_0418, s5k4e6mipiraw_COLOR_0419,
    s5k4e6mipiraw_COLOR_0420, s5k4e6mipiraw_COLOR_0421, s5k4e6mipiraw_COLOR_0422, s5k4e6mipiraw_COLOR_0423, s5k4e6mipiraw_COLOR_0424, s5k4e6mipiraw_COLOR_0425, s5k4e6mipiraw_COLOR_0426, s5k4e6mipiraw_COLOR_0427, s5k4e6mipiraw_COLOR_0428, s5k4e6mipiraw_COLOR_0429,
    s5k4e6mipiraw_COLOR_0430, s5k4e6mipiraw_COLOR_0431, s5k4e6mipiraw_COLOR_0432, s5k4e6mipiraw_COLOR_0433, s5k4e6mipiraw_COLOR_0434, s5k4e6mipiraw_COLOR_0435, s5k4e6mipiraw_COLOR_0436, s5k4e6mipiraw_COLOR_0437, s5k4e6mipiraw_COLOR_0438, s5k4e6mipiraw_COLOR_0439,
    s5k4e6mipiraw_COLOR_0440, s5k4e6mipiraw_COLOR_0441, s5k4e6mipiraw_COLOR_0442, s5k4e6mipiraw_COLOR_0443, s5k4e6mipiraw_COLOR_0444, s5k4e6mipiraw_COLOR_0445, s5k4e6mipiraw_COLOR_0446, s5k4e6mipiraw_COLOR_0447, s5k4e6mipiraw_COLOR_0448, s5k4e6mipiraw_COLOR_0449,
    s5k4e6mipiraw_COLOR_0450, s5k4e6mipiraw_COLOR_0451, s5k4e6mipiraw_COLOR_0452, s5k4e6mipiraw_COLOR_0453, s5k4e6mipiraw_COLOR_0454, s5k4e6mipiraw_COLOR_0455, s5k4e6mipiraw_COLOR_0456, s5k4e6mipiraw_COLOR_0457, s5k4e6mipiraw_COLOR_0458, s5k4e6mipiraw_COLOR_0459,
    s5k4e6mipiraw_COLOR_0460, s5k4e6mipiraw_COLOR_0461, s5k4e6mipiraw_COLOR_0462, s5k4e6mipiraw_COLOR_0463, s5k4e6mipiraw_COLOR_0464, s5k4e6mipiraw_COLOR_0465, s5k4e6mipiraw_COLOR_0466, s5k4e6mipiraw_COLOR_0467, s5k4e6mipiraw_COLOR_0468, s5k4e6mipiraw_COLOR_0469,
    s5k4e6mipiraw_COLOR_0470, s5k4e6mipiraw_COLOR_0471, s5k4e6mipiraw_COLOR_0472, s5k4e6mipiraw_COLOR_0473, s5k4e6mipiraw_COLOR_0474, s5k4e6mipiraw_COLOR_0475, s5k4e6mipiraw_COLOR_0476, s5k4e6mipiraw_COLOR_0477, s5k4e6mipiraw_COLOR_0478, s5k4e6mipiraw_COLOR_0479,
    s5k4e6mipiraw_COLOR_0480, s5k4e6mipiraw_COLOR_0481, s5k4e6mipiraw_COLOR_0482, s5k4e6mipiraw_COLOR_0483, s5k4e6mipiraw_COLOR_0484, s5k4e6mipiraw_COLOR_0485, s5k4e6mipiraw_COLOR_0486, s5k4e6mipiraw_COLOR_0487, s5k4e6mipiraw_COLOR_0488, s5k4e6mipiraw_COLOR_0489,
    s5k4e6mipiraw_COLOR_0490, s5k4e6mipiraw_COLOR_0491, s5k4e6mipiraw_COLOR_0492, s5k4e6mipiraw_COLOR_0493, s5k4e6mipiraw_COLOR_0494, s5k4e6mipiraw_COLOR_0495, s5k4e6mipiraw_COLOR_0496, s5k4e6mipiraw_COLOR_0497, s5k4e6mipiraw_COLOR_0498, s5k4e6mipiraw_COLOR_0499,
    s5k4e6mipiraw_COLOR_0500, s5k4e6mipiraw_COLOR_0501, s5k4e6mipiraw_COLOR_0502, s5k4e6mipiraw_COLOR_0503, s5k4e6mipiraw_COLOR_0504, s5k4e6mipiraw_COLOR_0505, s5k4e6mipiraw_COLOR_0506, s5k4e6mipiraw_COLOR_0507, s5k4e6mipiraw_COLOR_0508, s5k4e6mipiraw_COLOR_0509,
    s5k4e6mipiraw_COLOR_0510, s5k4e6mipiraw_COLOR_0511, s5k4e6mipiraw_COLOR_0512, s5k4e6mipiraw_COLOR_0513, s5k4e6mipiraw_COLOR_0514, s5k4e6mipiraw_COLOR_0515, s5k4e6mipiraw_COLOR_0516, s5k4e6mipiraw_COLOR_0517, s5k4e6mipiraw_COLOR_0518, s5k4e6mipiraw_COLOR_0519,
    s5k4e6mipiraw_COLOR_0520, s5k4e6mipiraw_COLOR_0521, s5k4e6mipiraw_COLOR_0522, s5k4e6mipiraw_COLOR_0523, s5k4e6mipiraw_COLOR_0524, s5k4e6mipiraw_COLOR_0525, s5k4e6mipiraw_COLOR_0526, s5k4e6mipiraw_COLOR_0527, s5k4e6mipiraw_COLOR_0528, s5k4e6mipiraw_COLOR_0529,
    s5k4e6mipiraw_COLOR_0530, s5k4e6mipiraw_COLOR_0531, s5k4e6mipiraw_COLOR_0532, s5k4e6mipiraw_COLOR_0533, s5k4e6mipiraw_COLOR_0534, s5k4e6mipiraw_COLOR_0535, s5k4e6mipiraw_COLOR_0536, s5k4e6mipiraw_COLOR_0537, s5k4e6mipiraw_COLOR_0538, s5k4e6mipiraw_COLOR_0539,
    s5k4e6mipiraw_COLOR_0540, s5k4e6mipiraw_COLOR_0541, s5k4e6mipiraw_COLOR_0542, s5k4e6mipiraw_COLOR_0543, s5k4e6mipiraw_COLOR_0544, s5k4e6mipiraw_COLOR_0545, s5k4e6mipiraw_COLOR_0546, s5k4e6mipiraw_COLOR_0547, s5k4e6mipiraw_COLOR_0548, s5k4e6mipiraw_COLOR_0549,
    s5k4e6mipiraw_COLOR_0550, s5k4e6mipiraw_COLOR_0551, s5k4e6mipiraw_COLOR_0552, s5k4e6mipiraw_COLOR_0553, s5k4e6mipiraw_COLOR_0554, s5k4e6mipiraw_COLOR_0555, s5k4e6mipiraw_COLOR_0556, s5k4e6mipiraw_COLOR_0557, s5k4e6mipiraw_COLOR_0558, s5k4e6mipiraw_COLOR_0559,
    s5k4e6mipiraw_COLOR_0560, s5k4e6mipiraw_COLOR_0561, s5k4e6mipiraw_COLOR_0562, s5k4e6mipiraw_COLOR_0563, s5k4e6mipiraw_COLOR_0564, s5k4e6mipiraw_COLOR_0565, s5k4e6mipiraw_COLOR_0566, s5k4e6mipiraw_COLOR_0567, s5k4e6mipiraw_COLOR_0568, s5k4e6mipiraw_COLOR_0569,
    s5k4e6mipiraw_COLOR_0570, s5k4e6mipiraw_COLOR_0571, s5k4e6mipiraw_COLOR_0572, s5k4e6mipiraw_COLOR_0573, s5k4e6mipiraw_COLOR_0574, s5k4e6mipiraw_COLOR_0575, s5k4e6mipiraw_COLOR_0576, s5k4e6mipiraw_COLOR_0577, s5k4e6mipiraw_COLOR_0578, s5k4e6mipiraw_COLOR_0579,
    s5k4e6mipiraw_COLOR_0580, s5k4e6mipiraw_COLOR_0581, s5k4e6mipiraw_COLOR_0582, s5k4e6mipiraw_COLOR_0583, s5k4e6mipiraw_COLOR_0584, s5k4e6mipiraw_COLOR_0585, s5k4e6mipiraw_COLOR_0586, s5k4e6mipiraw_COLOR_0587, s5k4e6mipiraw_COLOR_0588, s5k4e6mipiraw_COLOR_0589,
    s5k4e6mipiraw_COLOR_0590, s5k4e6mipiraw_COLOR_0591, s5k4e6mipiraw_COLOR_0592, s5k4e6mipiraw_COLOR_0593, s5k4e6mipiraw_COLOR_0594, s5k4e6mipiraw_COLOR_0595, s5k4e6mipiraw_COLOR_0596, s5k4e6mipiraw_COLOR_0597, s5k4e6mipiraw_COLOR_0598, s5k4e6mipiraw_COLOR_0599,
    s5k4e6mipiraw_COLOR_0600, s5k4e6mipiraw_COLOR_0601, s5k4e6mipiraw_COLOR_0602, s5k4e6mipiraw_COLOR_0603, s5k4e6mipiraw_COLOR_0604, s5k4e6mipiraw_COLOR_0605, s5k4e6mipiraw_COLOR_0606, s5k4e6mipiraw_COLOR_0607, s5k4e6mipiraw_COLOR_0608, s5k4e6mipiraw_COLOR_0609,
    s5k4e6mipiraw_COLOR_0610, s5k4e6mipiraw_COLOR_0611, s5k4e6mipiraw_COLOR_0612, s5k4e6mipiraw_COLOR_0613, s5k4e6mipiraw_COLOR_0614, s5k4e6mipiraw_COLOR_0615, s5k4e6mipiraw_COLOR_0616, s5k4e6mipiraw_COLOR_0617, s5k4e6mipiraw_COLOR_0618, s5k4e6mipiraw_COLOR_0619,
    s5k4e6mipiraw_COLOR_0620, s5k4e6mipiraw_COLOR_0621, s5k4e6mipiraw_COLOR_0622, s5k4e6mipiraw_COLOR_0623, s5k4e6mipiraw_COLOR_0624, s5k4e6mipiraw_COLOR_0625, s5k4e6mipiraw_COLOR_0626, s5k4e6mipiraw_COLOR_0627, s5k4e6mipiraw_COLOR_0628, s5k4e6mipiraw_COLOR_0629,
    s5k4e6mipiraw_COLOR_0630, s5k4e6mipiraw_COLOR_0631, s5k4e6mipiraw_COLOR_0632, s5k4e6mipiraw_COLOR_0633, s5k4e6mipiraw_COLOR_0634, s5k4e6mipiraw_COLOR_0635, s5k4e6mipiraw_COLOR_0636, s5k4e6mipiraw_COLOR_0637, s5k4e6mipiraw_COLOR_0638, s5k4e6mipiraw_COLOR_0639,
    s5k4e6mipiraw_COLOR_0640, s5k4e6mipiraw_COLOR_0641, s5k4e6mipiraw_COLOR_0642, s5k4e6mipiraw_COLOR_0643, s5k4e6mipiraw_COLOR_0644, s5k4e6mipiraw_COLOR_0645, s5k4e6mipiraw_COLOR_0646, s5k4e6mipiraw_COLOR_0647, s5k4e6mipiraw_COLOR_0648, s5k4e6mipiraw_COLOR_0649,
    s5k4e6mipiraw_COLOR_0650, s5k4e6mipiraw_COLOR_0651, s5k4e6mipiraw_COLOR_0652, s5k4e6mipiraw_COLOR_0653, s5k4e6mipiraw_COLOR_0654, s5k4e6mipiraw_COLOR_0655, s5k4e6mipiraw_COLOR_0656, s5k4e6mipiraw_COLOR_0657, s5k4e6mipiraw_COLOR_0658, s5k4e6mipiraw_COLOR_0659,
    s5k4e6mipiraw_COLOR_0660, s5k4e6mipiraw_COLOR_0661, s5k4e6mipiraw_COLOR_0662, s5k4e6mipiraw_COLOR_0663, s5k4e6mipiraw_COLOR_0664, s5k4e6mipiraw_COLOR_0665, s5k4e6mipiraw_COLOR_0666, s5k4e6mipiraw_COLOR_0667, s5k4e6mipiraw_COLOR_0668, s5k4e6mipiraw_COLOR_0669,
    s5k4e6mipiraw_COLOR_0670, s5k4e6mipiraw_COLOR_0671, s5k4e6mipiraw_COLOR_0672, s5k4e6mipiraw_COLOR_0673, s5k4e6mipiraw_COLOR_0674, s5k4e6mipiraw_COLOR_0675, s5k4e6mipiraw_COLOR_0676, s5k4e6mipiraw_COLOR_0677, s5k4e6mipiraw_COLOR_0678, s5k4e6mipiraw_COLOR_0679,
    s5k4e6mipiraw_COLOR_0680, s5k4e6mipiraw_COLOR_0681, s5k4e6mipiraw_COLOR_0682, s5k4e6mipiraw_COLOR_0683, s5k4e6mipiraw_COLOR_0684, s5k4e6mipiraw_COLOR_0685, s5k4e6mipiraw_COLOR_0686, s5k4e6mipiraw_COLOR_0687, s5k4e6mipiraw_COLOR_0688, s5k4e6mipiraw_COLOR_0689,
    s5k4e6mipiraw_COLOR_0690, s5k4e6mipiraw_COLOR_0691, s5k4e6mipiraw_COLOR_0692, s5k4e6mipiraw_COLOR_0693, s5k4e6mipiraw_COLOR_0694, s5k4e6mipiraw_COLOR_0695, s5k4e6mipiraw_COLOR_0696, s5k4e6mipiraw_COLOR_0697, s5k4e6mipiraw_COLOR_0698, s5k4e6mipiraw_COLOR_0699,
    s5k4e6mipiraw_COLOR_0700, s5k4e6mipiraw_COLOR_0701, s5k4e6mipiraw_COLOR_0702, s5k4e6mipiraw_COLOR_0703, s5k4e6mipiraw_COLOR_0704, s5k4e6mipiraw_COLOR_0705, s5k4e6mipiraw_COLOR_0706, s5k4e6mipiraw_COLOR_0707, s5k4e6mipiraw_COLOR_0708, s5k4e6mipiraw_COLOR_0709,
    s5k4e6mipiraw_COLOR_0710, s5k4e6mipiraw_COLOR_0711, s5k4e6mipiraw_COLOR_0712, s5k4e6mipiraw_COLOR_0713, s5k4e6mipiraw_COLOR_0714, s5k4e6mipiraw_COLOR_0715, s5k4e6mipiraw_COLOR_0716, s5k4e6mipiraw_COLOR_0717, s5k4e6mipiraw_COLOR_0718, s5k4e6mipiraw_COLOR_0719,
    s5k4e6mipiraw_COLOR_0720, s5k4e6mipiraw_COLOR_0721, s5k4e6mipiraw_COLOR_0722, s5k4e6mipiraw_COLOR_0723, s5k4e6mipiraw_COLOR_0724, s5k4e6mipiraw_COLOR_0725, s5k4e6mipiraw_COLOR_0726, s5k4e6mipiraw_COLOR_0727, s5k4e6mipiraw_COLOR_0728, s5k4e6mipiraw_COLOR_0729,
    s5k4e6mipiraw_COLOR_0730, s5k4e6mipiraw_COLOR_0731, s5k4e6mipiraw_COLOR_0732, s5k4e6mipiraw_COLOR_0733, s5k4e6mipiraw_COLOR_0734, s5k4e6mipiraw_COLOR_0735, s5k4e6mipiraw_COLOR_0736, s5k4e6mipiraw_COLOR_0737, s5k4e6mipiraw_COLOR_0738, s5k4e6mipiraw_COLOR_0739,
    s5k4e6mipiraw_COLOR_0740, s5k4e6mipiraw_COLOR_0741, s5k4e6mipiraw_COLOR_0742, s5k4e6mipiraw_COLOR_0743, s5k4e6mipiraw_COLOR_0744, s5k4e6mipiraw_COLOR_0745, s5k4e6mipiraw_COLOR_0746, s5k4e6mipiraw_COLOR_0747, s5k4e6mipiraw_COLOR_0748, s5k4e6mipiraw_COLOR_0749,
    s5k4e6mipiraw_COLOR_0750, s5k4e6mipiraw_COLOR_0751, s5k4e6mipiraw_COLOR_0752, s5k4e6mipiraw_COLOR_0753, s5k4e6mipiraw_COLOR_0754, s5k4e6mipiraw_COLOR_0755, s5k4e6mipiraw_COLOR_0756, s5k4e6mipiraw_COLOR_0757, s5k4e6mipiraw_COLOR_0758, s5k4e6mipiraw_COLOR_0759,
    s5k4e6mipiraw_COLOR_0760, s5k4e6mipiraw_COLOR_0761, s5k4e6mipiraw_COLOR_0762, s5k4e6mipiraw_COLOR_0763, s5k4e6mipiraw_COLOR_0764, s5k4e6mipiraw_COLOR_0765, s5k4e6mipiraw_COLOR_0766, s5k4e6mipiraw_COLOR_0767, s5k4e6mipiraw_COLOR_0768, s5k4e6mipiraw_COLOR_0769,
    s5k4e6mipiraw_COLOR_0770, s5k4e6mipiraw_COLOR_0771, s5k4e6mipiraw_COLOR_0772, s5k4e6mipiraw_COLOR_0773, s5k4e6mipiraw_COLOR_0774, s5k4e6mipiraw_COLOR_0775, s5k4e6mipiraw_COLOR_0776, s5k4e6mipiraw_COLOR_0777, s5k4e6mipiraw_COLOR_0778, s5k4e6mipiraw_COLOR_0779,
    s5k4e6mipiraw_COLOR_0780, s5k4e6mipiraw_COLOR_0781, s5k4e6mipiraw_COLOR_0782, s5k4e6mipiraw_COLOR_0783, s5k4e6mipiraw_COLOR_0784, s5k4e6mipiraw_COLOR_0785, s5k4e6mipiraw_COLOR_0786, s5k4e6mipiraw_COLOR_0787, s5k4e6mipiraw_COLOR_0788, s5k4e6mipiraw_COLOR_0789,
    s5k4e6mipiraw_COLOR_0790, s5k4e6mipiraw_COLOR_0791, s5k4e6mipiraw_COLOR_0792, s5k4e6mipiraw_COLOR_0793, s5k4e6mipiraw_COLOR_0794, s5k4e6mipiraw_COLOR_0795, s5k4e6mipiraw_COLOR_0796, s5k4e6mipiraw_COLOR_0797, s5k4e6mipiraw_COLOR_0798, s5k4e6mipiraw_COLOR_0799,
    s5k4e6mipiraw_COLOR_0800, s5k4e6mipiraw_COLOR_0801, s5k4e6mipiraw_COLOR_0802, s5k4e6mipiraw_COLOR_0803, s5k4e6mipiraw_COLOR_0804, s5k4e6mipiraw_COLOR_0805, s5k4e6mipiraw_COLOR_0806, s5k4e6mipiraw_COLOR_0807, s5k4e6mipiraw_COLOR_0808, s5k4e6mipiraw_COLOR_0809,
    s5k4e6mipiraw_COLOR_0810, s5k4e6mipiraw_COLOR_0811, s5k4e6mipiraw_COLOR_0812, s5k4e6mipiraw_COLOR_0813, s5k4e6mipiraw_COLOR_0814, s5k4e6mipiraw_COLOR_0815, s5k4e6mipiraw_COLOR_0816, s5k4e6mipiraw_COLOR_0817, s5k4e6mipiraw_COLOR_0818, s5k4e6mipiraw_COLOR_0819,
    s5k4e6mipiraw_COLOR_0820, s5k4e6mipiraw_COLOR_0821, s5k4e6mipiraw_COLOR_0822, s5k4e6mipiraw_COLOR_0823, s5k4e6mipiraw_COLOR_0824, s5k4e6mipiraw_COLOR_0825, s5k4e6mipiraw_COLOR_0826, s5k4e6mipiraw_COLOR_0827, s5k4e6mipiraw_COLOR_0828, s5k4e6mipiraw_COLOR_0829,
    s5k4e6mipiraw_COLOR_0830, s5k4e6mipiraw_COLOR_0831, s5k4e6mipiraw_COLOR_0832, s5k4e6mipiraw_COLOR_0833, s5k4e6mipiraw_COLOR_0834, s5k4e6mipiraw_COLOR_0835, s5k4e6mipiraw_COLOR_0836, s5k4e6mipiraw_COLOR_0837, s5k4e6mipiraw_COLOR_0838, s5k4e6mipiraw_COLOR_0839,
    s5k4e6mipiraw_COLOR_0840, s5k4e6mipiraw_COLOR_0841, s5k4e6mipiraw_COLOR_0842, s5k4e6mipiraw_COLOR_0843, s5k4e6mipiraw_COLOR_0844, s5k4e6mipiraw_COLOR_0845, s5k4e6mipiraw_COLOR_0846, s5k4e6mipiraw_COLOR_0847, s5k4e6mipiraw_COLOR_0848, s5k4e6mipiraw_COLOR_0849,
    s5k4e6mipiraw_COLOR_0850, s5k4e6mipiraw_COLOR_0851, s5k4e6mipiraw_COLOR_0852, s5k4e6mipiraw_COLOR_0853, s5k4e6mipiraw_COLOR_0854, s5k4e6mipiraw_COLOR_0855, s5k4e6mipiraw_COLOR_0856, s5k4e6mipiraw_COLOR_0857, s5k4e6mipiraw_COLOR_0858, s5k4e6mipiraw_COLOR_0859,
    s5k4e6mipiraw_COLOR_0860, s5k4e6mipiraw_COLOR_0861, s5k4e6mipiraw_COLOR_0862, s5k4e6mipiraw_COLOR_0863, s5k4e6mipiraw_COLOR_0864, s5k4e6mipiraw_COLOR_0865, s5k4e6mipiraw_COLOR_0866, s5k4e6mipiraw_COLOR_0867, s5k4e6mipiraw_COLOR_0868, s5k4e6mipiraw_COLOR_0869,
    s5k4e6mipiraw_COLOR_0870, s5k4e6mipiraw_COLOR_0871, s5k4e6mipiraw_COLOR_0872, s5k4e6mipiraw_COLOR_0873, s5k4e6mipiraw_COLOR_0874, s5k4e6mipiraw_COLOR_0875, s5k4e6mipiraw_COLOR_0876, s5k4e6mipiraw_COLOR_0877, s5k4e6mipiraw_COLOR_0878, s5k4e6mipiraw_COLOR_0879,
    s5k4e6mipiraw_COLOR_0880, s5k4e6mipiraw_COLOR_0881, s5k4e6mipiraw_COLOR_0882, s5k4e6mipiraw_COLOR_0883, s5k4e6mipiraw_COLOR_0884, s5k4e6mipiraw_COLOR_0885, s5k4e6mipiraw_COLOR_0886, s5k4e6mipiraw_COLOR_0887, s5k4e6mipiraw_COLOR_0888, s5k4e6mipiraw_COLOR_0889,
    s5k4e6mipiraw_COLOR_0890, s5k4e6mipiraw_COLOR_0891, s5k4e6mipiraw_COLOR_0892, s5k4e6mipiraw_COLOR_0893, s5k4e6mipiraw_COLOR_0894, s5k4e6mipiraw_COLOR_0895, s5k4e6mipiraw_COLOR_0896, s5k4e6mipiraw_COLOR_0897, s5k4e6mipiraw_COLOR_0898, s5k4e6mipiraw_COLOR_0899,
    s5k4e6mipiraw_COLOR_0900, s5k4e6mipiraw_COLOR_0901, s5k4e6mipiraw_COLOR_0902, s5k4e6mipiraw_COLOR_0903, s5k4e6mipiraw_COLOR_0904, s5k4e6mipiraw_COLOR_0905, s5k4e6mipiraw_COLOR_0906, s5k4e6mipiraw_COLOR_0907, s5k4e6mipiraw_COLOR_0908, s5k4e6mipiraw_COLOR_0909,
    s5k4e6mipiraw_COLOR_0910, s5k4e6mipiraw_COLOR_0911, s5k4e6mipiraw_COLOR_0912, s5k4e6mipiraw_COLOR_0913, s5k4e6mipiraw_COLOR_0914, s5k4e6mipiraw_COLOR_0915, s5k4e6mipiraw_COLOR_0916, s5k4e6mipiraw_COLOR_0917, s5k4e6mipiraw_COLOR_0918, s5k4e6mipiraw_COLOR_0919,
    s5k4e6mipiraw_COLOR_0920, s5k4e6mipiraw_COLOR_0921, s5k4e6mipiraw_COLOR_0922, s5k4e6mipiraw_COLOR_0923, s5k4e6mipiraw_COLOR_0924, s5k4e6mipiraw_COLOR_0925, s5k4e6mipiraw_COLOR_0926, s5k4e6mipiraw_COLOR_0927, s5k4e6mipiraw_COLOR_0928, s5k4e6mipiraw_COLOR_0929,
    s5k4e6mipiraw_COLOR_0930, s5k4e6mipiraw_COLOR_0931, s5k4e6mipiraw_COLOR_0932, s5k4e6mipiraw_COLOR_0933, s5k4e6mipiraw_COLOR_0934, s5k4e6mipiraw_COLOR_0935, s5k4e6mipiraw_COLOR_0936, s5k4e6mipiraw_COLOR_0937, s5k4e6mipiraw_COLOR_0938, s5k4e6mipiraw_COLOR_0939,
    s5k4e6mipiraw_COLOR_0940, s5k4e6mipiraw_COLOR_0941, s5k4e6mipiraw_COLOR_0942, s5k4e6mipiraw_COLOR_0943, s5k4e6mipiraw_COLOR_0944, s5k4e6mipiraw_COLOR_0945, s5k4e6mipiraw_COLOR_0946, s5k4e6mipiraw_COLOR_0947, s5k4e6mipiraw_COLOR_0948, s5k4e6mipiraw_COLOR_0949,
    s5k4e6mipiraw_COLOR_0950, s5k4e6mipiraw_COLOR_0951, s5k4e6mipiraw_COLOR_0952, s5k4e6mipiraw_COLOR_0953, s5k4e6mipiraw_COLOR_0954, s5k4e6mipiraw_COLOR_0955, s5k4e6mipiraw_COLOR_0956, s5k4e6mipiraw_COLOR_0957, s5k4e6mipiraw_COLOR_0958, s5k4e6mipiraw_COLOR_0959,
},
.PCA = {
    s5k4e6mipiraw_PCA_0000,
},
.COLOR_PARAM = {
    s5k4e6mipiraw_COLOR_PARAM_0000, s5k4e6mipiraw_COLOR_PARAM_0001, s5k4e6mipiraw_COLOR_PARAM_0002, s5k4e6mipiraw_COLOR_PARAM_0003, s5k4e6mipiraw_COLOR_PARAM_0004, s5k4e6mipiraw_COLOR_PARAM_0005, s5k4e6mipiraw_COLOR_PARAM_0006, s5k4e6mipiraw_COLOR_PARAM_0007, s5k4e6mipiraw_COLOR_PARAM_0008, s5k4e6mipiraw_COLOR_PARAM_0009,
    s5k4e6mipiraw_COLOR_PARAM_0010, s5k4e6mipiraw_COLOR_PARAM_0011, s5k4e6mipiraw_COLOR_PARAM_0012, s5k4e6mipiraw_COLOR_PARAM_0013, s5k4e6mipiraw_COLOR_PARAM_0014, s5k4e6mipiraw_COLOR_PARAM_0015,
},
