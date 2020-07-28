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
    imx258mipiraw_CCM_0000, imx258mipiraw_CCM_0001, imx258mipiraw_CCM_0002, imx258mipiraw_CCM_0003, imx258mipiraw_CCM_0004, imx258mipiraw_CCM_0005, imx258mipiraw_CCM_0006, imx258mipiraw_CCM_0007, imx258mipiraw_CCM_0008, imx258mipiraw_CCM_0009,
    imx258mipiraw_CCM_0010, imx258mipiraw_CCM_0011, imx258mipiraw_CCM_0012, imx258mipiraw_CCM_0013, imx258mipiraw_CCM_0014, imx258mipiraw_CCM_0015, imx258mipiraw_CCM_0016, imx258mipiraw_CCM_0017, imx258mipiraw_CCM_0018, imx258mipiraw_CCM_0019,
    imx258mipiraw_CCM_0020, imx258mipiraw_CCM_0021, imx258mipiraw_CCM_0022, imx258mipiraw_CCM_0023, imx258mipiraw_CCM_0024, imx258mipiraw_CCM_0025, imx258mipiraw_CCM_0026, imx258mipiraw_CCM_0027, imx258mipiraw_CCM_0028, imx258mipiraw_CCM_0029,
    imx258mipiraw_CCM_0030, imx258mipiraw_CCM_0031, imx258mipiraw_CCM_0032, imx258mipiraw_CCM_0033, imx258mipiraw_CCM_0034, imx258mipiraw_CCM_0035, imx258mipiraw_CCM_0036, imx258mipiraw_CCM_0037, imx258mipiraw_CCM_0038, imx258mipiraw_CCM_0039,
    imx258mipiraw_CCM_0040, imx258mipiraw_CCM_0041, imx258mipiraw_CCM_0042, imx258mipiraw_CCM_0043, imx258mipiraw_CCM_0044, imx258mipiraw_CCM_0045, imx258mipiraw_CCM_0046, imx258mipiraw_CCM_0047, imx258mipiraw_CCM_0048, imx258mipiraw_CCM_0049,
    imx258mipiraw_CCM_0050, imx258mipiraw_CCM_0051, imx258mipiraw_CCM_0052, imx258mipiraw_CCM_0053, imx258mipiraw_CCM_0054, imx258mipiraw_CCM_0055, imx258mipiraw_CCM_0056, imx258mipiraw_CCM_0057, imx258mipiraw_CCM_0058, imx258mipiraw_CCM_0059,
    imx258mipiraw_CCM_0060, imx258mipiraw_CCM_0061, imx258mipiraw_CCM_0062, imx258mipiraw_CCM_0063, imx258mipiraw_CCM_0064, imx258mipiraw_CCM_0065, imx258mipiraw_CCM_0066, imx258mipiraw_CCM_0067, imx258mipiraw_CCM_0068, imx258mipiraw_CCM_0069,
    imx258mipiraw_CCM_0070, imx258mipiraw_CCM_0071, imx258mipiraw_CCM_0072, imx258mipiraw_CCM_0073, imx258mipiraw_CCM_0074, imx258mipiraw_CCM_0075, imx258mipiraw_CCM_0076, imx258mipiraw_CCM_0077, imx258mipiraw_CCM_0078, imx258mipiraw_CCM_0079,
    imx258mipiraw_CCM_0080, imx258mipiraw_CCM_0081, imx258mipiraw_CCM_0082, imx258mipiraw_CCM_0083, imx258mipiraw_CCM_0084, imx258mipiraw_CCM_0085, imx258mipiraw_CCM_0086, imx258mipiraw_CCM_0087, imx258mipiraw_CCM_0088, imx258mipiraw_CCM_0089,
    imx258mipiraw_CCM_0090, imx258mipiraw_CCM_0091, imx258mipiraw_CCM_0092, imx258mipiraw_CCM_0093, imx258mipiraw_CCM_0094, imx258mipiraw_CCM_0095, imx258mipiraw_CCM_0096, imx258mipiraw_CCM_0097, imx258mipiraw_CCM_0098, imx258mipiraw_CCM_0099,
    imx258mipiraw_CCM_0100, imx258mipiraw_CCM_0101, imx258mipiraw_CCM_0102, imx258mipiraw_CCM_0103, imx258mipiraw_CCM_0104, imx258mipiraw_CCM_0105, imx258mipiraw_CCM_0106, imx258mipiraw_CCM_0107, imx258mipiraw_CCM_0108, imx258mipiraw_CCM_0109,
    imx258mipiraw_CCM_0110, imx258mipiraw_CCM_0111, imx258mipiraw_CCM_0112, imx258mipiraw_CCM_0113, imx258mipiraw_CCM_0114, imx258mipiraw_CCM_0115, imx258mipiraw_CCM_0116, imx258mipiraw_CCM_0117, imx258mipiraw_CCM_0118, imx258mipiraw_CCM_0119,
    imx258mipiraw_CCM_0120, imx258mipiraw_CCM_0121, imx258mipiraw_CCM_0122, imx258mipiraw_CCM_0123, imx258mipiraw_CCM_0124, imx258mipiraw_CCM_0125, imx258mipiraw_CCM_0126, imx258mipiraw_CCM_0127, imx258mipiraw_CCM_0128, imx258mipiraw_CCM_0129,
    imx258mipiraw_CCM_0130, imx258mipiraw_CCM_0131, imx258mipiraw_CCM_0132, imx258mipiraw_CCM_0133, imx258mipiraw_CCM_0134, imx258mipiraw_CCM_0135, imx258mipiraw_CCM_0136, imx258mipiraw_CCM_0137, imx258mipiraw_CCM_0138, imx258mipiraw_CCM_0139,
    imx258mipiraw_CCM_0140, imx258mipiraw_CCM_0141, imx258mipiraw_CCM_0142, imx258mipiraw_CCM_0143, imx258mipiraw_CCM_0144, imx258mipiraw_CCM_0145, imx258mipiraw_CCM_0146, imx258mipiraw_CCM_0147, imx258mipiraw_CCM_0148, imx258mipiraw_CCM_0149,
    imx258mipiraw_CCM_0150, imx258mipiraw_CCM_0151, imx258mipiraw_CCM_0152, imx258mipiraw_CCM_0153, imx258mipiraw_CCM_0154, imx258mipiraw_CCM_0155, imx258mipiraw_CCM_0156, imx258mipiraw_CCM_0157, imx258mipiraw_CCM_0158, imx258mipiraw_CCM_0159,
    imx258mipiraw_CCM_0160, imx258mipiraw_CCM_0161, imx258mipiraw_CCM_0162, imx258mipiraw_CCM_0163, imx258mipiraw_CCM_0164, imx258mipiraw_CCM_0165, imx258mipiraw_CCM_0166, imx258mipiraw_CCM_0167, imx258mipiraw_CCM_0168, imx258mipiraw_CCM_0169,
    imx258mipiraw_CCM_0170, imx258mipiraw_CCM_0171, imx258mipiraw_CCM_0172, imx258mipiraw_CCM_0173, imx258mipiraw_CCM_0174, imx258mipiraw_CCM_0175, imx258mipiraw_CCM_0176, imx258mipiraw_CCM_0177, imx258mipiraw_CCM_0178, imx258mipiraw_CCM_0179,
    imx258mipiraw_CCM_0180, imx258mipiraw_CCM_0181, imx258mipiraw_CCM_0182, imx258mipiraw_CCM_0183, imx258mipiraw_CCM_0184, imx258mipiraw_CCM_0185, imx258mipiraw_CCM_0186, imx258mipiraw_CCM_0187, imx258mipiraw_CCM_0188, imx258mipiraw_CCM_0189,
    imx258mipiraw_CCM_0190, imx258mipiraw_CCM_0191, imx258mipiraw_CCM_0192, imx258mipiraw_CCM_0193, imx258mipiraw_CCM_0194, imx258mipiraw_CCM_0195, imx258mipiraw_CCM_0196, imx258mipiraw_CCM_0197, imx258mipiraw_CCM_0198, imx258mipiraw_CCM_0199,
    imx258mipiraw_CCM_0200, imx258mipiraw_CCM_0201, imx258mipiraw_CCM_0202, imx258mipiraw_CCM_0203, imx258mipiraw_CCM_0204, imx258mipiraw_CCM_0205, imx258mipiraw_CCM_0206, imx258mipiraw_CCM_0207, imx258mipiraw_CCM_0208, imx258mipiraw_CCM_0209,
    imx258mipiraw_CCM_0210, imx258mipiraw_CCM_0211, imx258mipiraw_CCM_0212, imx258mipiraw_CCM_0213, imx258mipiraw_CCM_0214, imx258mipiraw_CCM_0215, imx258mipiraw_CCM_0216, imx258mipiraw_CCM_0217, imx258mipiraw_CCM_0218, imx258mipiraw_CCM_0219,
    imx258mipiraw_CCM_0220, imx258mipiraw_CCM_0221, imx258mipiraw_CCM_0222, imx258mipiraw_CCM_0223, imx258mipiraw_CCM_0224, imx258mipiraw_CCM_0225, imx258mipiraw_CCM_0226, imx258mipiraw_CCM_0227, imx258mipiraw_CCM_0228, imx258mipiraw_CCM_0229,
    imx258mipiraw_CCM_0230, imx258mipiraw_CCM_0231, imx258mipiraw_CCM_0232, imx258mipiraw_CCM_0233, imx258mipiraw_CCM_0234, imx258mipiraw_CCM_0235, imx258mipiraw_CCM_0236, imx258mipiraw_CCM_0237, imx258mipiraw_CCM_0238, imx258mipiraw_CCM_0239,
},
.COLOR = {
    imx258mipiraw_COLOR_0000, imx258mipiraw_COLOR_0001, imx258mipiraw_COLOR_0002, imx258mipiraw_COLOR_0003, imx258mipiraw_COLOR_0004, imx258mipiraw_COLOR_0005, imx258mipiraw_COLOR_0006, imx258mipiraw_COLOR_0007, imx258mipiraw_COLOR_0008, imx258mipiraw_COLOR_0009,
    imx258mipiraw_COLOR_0010, imx258mipiraw_COLOR_0011, imx258mipiraw_COLOR_0012, imx258mipiraw_COLOR_0013, imx258mipiraw_COLOR_0014, imx258mipiraw_COLOR_0015, imx258mipiraw_COLOR_0016, imx258mipiraw_COLOR_0017, imx258mipiraw_COLOR_0018, imx258mipiraw_COLOR_0019,
    imx258mipiraw_COLOR_0020, imx258mipiraw_COLOR_0021, imx258mipiraw_COLOR_0022, imx258mipiraw_COLOR_0023, imx258mipiraw_COLOR_0024, imx258mipiraw_COLOR_0025, imx258mipiraw_COLOR_0026, imx258mipiraw_COLOR_0027, imx258mipiraw_COLOR_0028, imx258mipiraw_COLOR_0029,
    imx258mipiraw_COLOR_0030, imx258mipiraw_COLOR_0031, imx258mipiraw_COLOR_0032, imx258mipiraw_COLOR_0033, imx258mipiraw_COLOR_0034, imx258mipiraw_COLOR_0035, imx258mipiraw_COLOR_0036, imx258mipiraw_COLOR_0037, imx258mipiraw_COLOR_0038, imx258mipiraw_COLOR_0039,
    imx258mipiraw_COLOR_0040, imx258mipiraw_COLOR_0041, imx258mipiraw_COLOR_0042, imx258mipiraw_COLOR_0043, imx258mipiraw_COLOR_0044, imx258mipiraw_COLOR_0045, imx258mipiraw_COLOR_0046, imx258mipiraw_COLOR_0047, imx258mipiraw_COLOR_0048, imx258mipiraw_COLOR_0049,
    imx258mipiraw_COLOR_0050, imx258mipiraw_COLOR_0051, imx258mipiraw_COLOR_0052, imx258mipiraw_COLOR_0053, imx258mipiraw_COLOR_0054, imx258mipiraw_COLOR_0055, imx258mipiraw_COLOR_0056, imx258mipiraw_COLOR_0057, imx258mipiraw_COLOR_0058, imx258mipiraw_COLOR_0059,
    imx258mipiraw_COLOR_0060, imx258mipiraw_COLOR_0061, imx258mipiraw_COLOR_0062, imx258mipiraw_COLOR_0063, imx258mipiraw_COLOR_0064, imx258mipiraw_COLOR_0065, imx258mipiraw_COLOR_0066, imx258mipiraw_COLOR_0067, imx258mipiraw_COLOR_0068, imx258mipiraw_COLOR_0069,
    imx258mipiraw_COLOR_0070, imx258mipiraw_COLOR_0071, imx258mipiraw_COLOR_0072, imx258mipiraw_COLOR_0073, imx258mipiraw_COLOR_0074, imx258mipiraw_COLOR_0075, imx258mipiraw_COLOR_0076, imx258mipiraw_COLOR_0077, imx258mipiraw_COLOR_0078, imx258mipiraw_COLOR_0079,
    imx258mipiraw_COLOR_0080, imx258mipiraw_COLOR_0081, imx258mipiraw_COLOR_0082, imx258mipiraw_COLOR_0083, imx258mipiraw_COLOR_0084, imx258mipiraw_COLOR_0085, imx258mipiraw_COLOR_0086, imx258mipiraw_COLOR_0087, imx258mipiraw_COLOR_0088, imx258mipiraw_COLOR_0089,
    imx258mipiraw_COLOR_0090, imx258mipiraw_COLOR_0091, imx258mipiraw_COLOR_0092, imx258mipiraw_COLOR_0093, imx258mipiraw_COLOR_0094, imx258mipiraw_COLOR_0095, imx258mipiraw_COLOR_0096, imx258mipiraw_COLOR_0097, imx258mipiraw_COLOR_0098, imx258mipiraw_COLOR_0099,
    imx258mipiraw_COLOR_0100, imx258mipiraw_COLOR_0101, imx258mipiraw_COLOR_0102, imx258mipiraw_COLOR_0103, imx258mipiraw_COLOR_0104, imx258mipiraw_COLOR_0105, imx258mipiraw_COLOR_0106, imx258mipiraw_COLOR_0107, imx258mipiraw_COLOR_0108, imx258mipiraw_COLOR_0109,
    imx258mipiraw_COLOR_0110, imx258mipiraw_COLOR_0111, imx258mipiraw_COLOR_0112, imx258mipiraw_COLOR_0113, imx258mipiraw_COLOR_0114, imx258mipiraw_COLOR_0115, imx258mipiraw_COLOR_0116, imx258mipiraw_COLOR_0117, imx258mipiraw_COLOR_0118, imx258mipiraw_COLOR_0119,
    imx258mipiraw_COLOR_0120, imx258mipiraw_COLOR_0121, imx258mipiraw_COLOR_0122, imx258mipiraw_COLOR_0123, imx258mipiraw_COLOR_0124, imx258mipiraw_COLOR_0125, imx258mipiraw_COLOR_0126, imx258mipiraw_COLOR_0127, imx258mipiraw_COLOR_0128, imx258mipiraw_COLOR_0129,
    imx258mipiraw_COLOR_0130, imx258mipiraw_COLOR_0131, imx258mipiraw_COLOR_0132, imx258mipiraw_COLOR_0133, imx258mipiraw_COLOR_0134, imx258mipiraw_COLOR_0135, imx258mipiraw_COLOR_0136, imx258mipiraw_COLOR_0137, imx258mipiraw_COLOR_0138, imx258mipiraw_COLOR_0139,
    imx258mipiraw_COLOR_0140, imx258mipiraw_COLOR_0141, imx258mipiraw_COLOR_0142, imx258mipiraw_COLOR_0143, imx258mipiraw_COLOR_0144, imx258mipiraw_COLOR_0145, imx258mipiraw_COLOR_0146, imx258mipiraw_COLOR_0147, imx258mipiraw_COLOR_0148, imx258mipiraw_COLOR_0149,
    imx258mipiraw_COLOR_0150, imx258mipiraw_COLOR_0151, imx258mipiraw_COLOR_0152, imx258mipiraw_COLOR_0153, imx258mipiraw_COLOR_0154, imx258mipiraw_COLOR_0155, imx258mipiraw_COLOR_0156, imx258mipiraw_COLOR_0157, imx258mipiraw_COLOR_0158, imx258mipiraw_COLOR_0159,
    imx258mipiraw_COLOR_0160, imx258mipiraw_COLOR_0161, imx258mipiraw_COLOR_0162, imx258mipiraw_COLOR_0163, imx258mipiraw_COLOR_0164, imx258mipiraw_COLOR_0165, imx258mipiraw_COLOR_0166, imx258mipiraw_COLOR_0167, imx258mipiraw_COLOR_0168, imx258mipiraw_COLOR_0169,
    imx258mipiraw_COLOR_0170, imx258mipiraw_COLOR_0171, imx258mipiraw_COLOR_0172, imx258mipiraw_COLOR_0173, imx258mipiraw_COLOR_0174, imx258mipiraw_COLOR_0175, imx258mipiraw_COLOR_0176, imx258mipiraw_COLOR_0177, imx258mipiraw_COLOR_0178, imx258mipiraw_COLOR_0179,
    imx258mipiraw_COLOR_0180, imx258mipiraw_COLOR_0181, imx258mipiraw_COLOR_0182, imx258mipiraw_COLOR_0183, imx258mipiraw_COLOR_0184, imx258mipiraw_COLOR_0185, imx258mipiraw_COLOR_0186, imx258mipiraw_COLOR_0187, imx258mipiraw_COLOR_0188, imx258mipiraw_COLOR_0189,
    imx258mipiraw_COLOR_0190, imx258mipiraw_COLOR_0191, imx258mipiraw_COLOR_0192, imx258mipiraw_COLOR_0193, imx258mipiraw_COLOR_0194, imx258mipiraw_COLOR_0195, imx258mipiraw_COLOR_0196, imx258mipiraw_COLOR_0197, imx258mipiraw_COLOR_0198, imx258mipiraw_COLOR_0199,
    imx258mipiraw_COLOR_0200, imx258mipiraw_COLOR_0201, imx258mipiraw_COLOR_0202, imx258mipiraw_COLOR_0203, imx258mipiraw_COLOR_0204, imx258mipiraw_COLOR_0205, imx258mipiraw_COLOR_0206, imx258mipiraw_COLOR_0207, imx258mipiraw_COLOR_0208, imx258mipiraw_COLOR_0209,
    imx258mipiraw_COLOR_0210, imx258mipiraw_COLOR_0211, imx258mipiraw_COLOR_0212, imx258mipiraw_COLOR_0213, imx258mipiraw_COLOR_0214, imx258mipiraw_COLOR_0215, imx258mipiraw_COLOR_0216, imx258mipiraw_COLOR_0217, imx258mipiraw_COLOR_0218, imx258mipiraw_COLOR_0219,
    imx258mipiraw_COLOR_0220, imx258mipiraw_COLOR_0221, imx258mipiraw_COLOR_0222, imx258mipiraw_COLOR_0223, imx258mipiraw_COLOR_0224, imx258mipiraw_COLOR_0225, imx258mipiraw_COLOR_0226, imx258mipiraw_COLOR_0227, imx258mipiraw_COLOR_0228, imx258mipiraw_COLOR_0229,
    imx258mipiraw_COLOR_0230, imx258mipiraw_COLOR_0231, imx258mipiraw_COLOR_0232, imx258mipiraw_COLOR_0233, imx258mipiraw_COLOR_0234, imx258mipiraw_COLOR_0235, imx258mipiraw_COLOR_0236, imx258mipiraw_COLOR_0237, imx258mipiraw_COLOR_0238, imx258mipiraw_COLOR_0239,
    imx258mipiraw_COLOR_0240, imx258mipiraw_COLOR_0241, imx258mipiraw_COLOR_0242, imx258mipiraw_COLOR_0243, imx258mipiraw_COLOR_0244, imx258mipiraw_COLOR_0245, imx258mipiraw_COLOR_0246, imx258mipiraw_COLOR_0247, imx258mipiraw_COLOR_0248, imx258mipiraw_COLOR_0249,
    imx258mipiraw_COLOR_0250, imx258mipiraw_COLOR_0251, imx258mipiraw_COLOR_0252, imx258mipiraw_COLOR_0253, imx258mipiraw_COLOR_0254, imx258mipiraw_COLOR_0255, imx258mipiraw_COLOR_0256, imx258mipiraw_COLOR_0257, imx258mipiraw_COLOR_0258, imx258mipiraw_COLOR_0259,
    imx258mipiraw_COLOR_0260, imx258mipiraw_COLOR_0261, imx258mipiraw_COLOR_0262, imx258mipiraw_COLOR_0263, imx258mipiraw_COLOR_0264, imx258mipiraw_COLOR_0265, imx258mipiraw_COLOR_0266, imx258mipiraw_COLOR_0267, imx258mipiraw_COLOR_0268, imx258mipiraw_COLOR_0269,
    imx258mipiraw_COLOR_0270, imx258mipiraw_COLOR_0271, imx258mipiraw_COLOR_0272, imx258mipiraw_COLOR_0273, imx258mipiraw_COLOR_0274, imx258mipiraw_COLOR_0275, imx258mipiraw_COLOR_0276, imx258mipiraw_COLOR_0277, imx258mipiraw_COLOR_0278, imx258mipiraw_COLOR_0279,
    imx258mipiraw_COLOR_0280, imx258mipiraw_COLOR_0281, imx258mipiraw_COLOR_0282, imx258mipiraw_COLOR_0283, imx258mipiraw_COLOR_0284, imx258mipiraw_COLOR_0285, imx258mipiraw_COLOR_0286, imx258mipiraw_COLOR_0287, imx258mipiraw_COLOR_0288, imx258mipiraw_COLOR_0289,
    imx258mipiraw_COLOR_0290, imx258mipiraw_COLOR_0291, imx258mipiraw_COLOR_0292, imx258mipiraw_COLOR_0293, imx258mipiraw_COLOR_0294, imx258mipiraw_COLOR_0295, imx258mipiraw_COLOR_0296, imx258mipiraw_COLOR_0297, imx258mipiraw_COLOR_0298, imx258mipiraw_COLOR_0299,
    imx258mipiraw_COLOR_0300, imx258mipiraw_COLOR_0301, imx258mipiraw_COLOR_0302, imx258mipiraw_COLOR_0303, imx258mipiraw_COLOR_0304, imx258mipiraw_COLOR_0305, imx258mipiraw_COLOR_0306, imx258mipiraw_COLOR_0307, imx258mipiraw_COLOR_0308, imx258mipiraw_COLOR_0309,
    imx258mipiraw_COLOR_0310, imx258mipiraw_COLOR_0311, imx258mipiraw_COLOR_0312, imx258mipiraw_COLOR_0313, imx258mipiraw_COLOR_0314, imx258mipiraw_COLOR_0315, imx258mipiraw_COLOR_0316, imx258mipiraw_COLOR_0317, imx258mipiraw_COLOR_0318, imx258mipiraw_COLOR_0319,
    imx258mipiraw_COLOR_0320, imx258mipiraw_COLOR_0321, imx258mipiraw_COLOR_0322, imx258mipiraw_COLOR_0323, imx258mipiraw_COLOR_0324, imx258mipiraw_COLOR_0325, imx258mipiraw_COLOR_0326, imx258mipiraw_COLOR_0327, imx258mipiraw_COLOR_0328, imx258mipiraw_COLOR_0329,
    imx258mipiraw_COLOR_0330, imx258mipiraw_COLOR_0331, imx258mipiraw_COLOR_0332, imx258mipiraw_COLOR_0333, imx258mipiraw_COLOR_0334, imx258mipiraw_COLOR_0335, imx258mipiraw_COLOR_0336, imx258mipiraw_COLOR_0337, imx258mipiraw_COLOR_0338, imx258mipiraw_COLOR_0339,
    imx258mipiraw_COLOR_0340, imx258mipiraw_COLOR_0341, imx258mipiraw_COLOR_0342, imx258mipiraw_COLOR_0343, imx258mipiraw_COLOR_0344, imx258mipiraw_COLOR_0345, imx258mipiraw_COLOR_0346, imx258mipiraw_COLOR_0347, imx258mipiraw_COLOR_0348, imx258mipiraw_COLOR_0349,
    imx258mipiraw_COLOR_0350, imx258mipiraw_COLOR_0351, imx258mipiraw_COLOR_0352, imx258mipiraw_COLOR_0353, imx258mipiraw_COLOR_0354, imx258mipiraw_COLOR_0355, imx258mipiraw_COLOR_0356, imx258mipiraw_COLOR_0357, imx258mipiraw_COLOR_0358, imx258mipiraw_COLOR_0359,
    imx258mipiraw_COLOR_0360, imx258mipiraw_COLOR_0361, imx258mipiraw_COLOR_0362, imx258mipiraw_COLOR_0363, imx258mipiraw_COLOR_0364, imx258mipiraw_COLOR_0365, imx258mipiraw_COLOR_0366, imx258mipiraw_COLOR_0367, imx258mipiraw_COLOR_0368, imx258mipiraw_COLOR_0369,
    imx258mipiraw_COLOR_0370, imx258mipiraw_COLOR_0371, imx258mipiraw_COLOR_0372, imx258mipiraw_COLOR_0373, imx258mipiraw_COLOR_0374, imx258mipiraw_COLOR_0375, imx258mipiraw_COLOR_0376, imx258mipiraw_COLOR_0377, imx258mipiraw_COLOR_0378, imx258mipiraw_COLOR_0379,
    imx258mipiraw_COLOR_0380, imx258mipiraw_COLOR_0381, imx258mipiraw_COLOR_0382, imx258mipiraw_COLOR_0383, imx258mipiraw_COLOR_0384, imx258mipiraw_COLOR_0385, imx258mipiraw_COLOR_0386, imx258mipiraw_COLOR_0387, imx258mipiraw_COLOR_0388, imx258mipiraw_COLOR_0389,
    imx258mipiraw_COLOR_0390, imx258mipiraw_COLOR_0391, imx258mipiraw_COLOR_0392, imx258mipiraw_COLOR_0393, imx258mipiraw_COLOR_0394, imx258mipiraw_COLOR_0395, imx258mipiraw_COLOR_0396, imx258mipiraw_COLOR_0397, imx258mipiraw_COLOR_0398, imx258mipiraw_COLOR_0399,
    imx258mipiraw_COLOR_0400, imx258mipiraw_COLOR_0401, imx258mipiraw_COLOR_0402, imx258mipiraw_COLOR_0403, imx258mipiraw_COLOR_0404, imx258mipiraw_COLOR_0405, imx258mipiraw_COLOR_0406, imx258mipiraw_COLOR_0407, imx258mipiraw_COLOR_0408, imx258mipiraw_COLOR_0409,
    imx258mipiraw_COLOR_0410, imx258mipiraw_COLOR_0411, imx258mipiraw_COLOR_0412, imx258mipiraw_COLOR_0413, imx258mipiraw_COLOR_0414, imx258mipiraw_COLOR_0415, imx258mipiraw_COLOR_0416, imx258mipiraw_COLOR_0417, imx258mipiraw_COLOR_0418, imx258mipiraw_COLOR_0419,
    imx258mipiraw_COLOR_0420, imx258mipiraw_COLOR_0421, imx258mipiraw_COLOR_0422, imx258mipiraw_COLOR_0423, imx258mipiraw_COLOR_0424, imx258mipiraw_COLOR_0425, imx258mipiraw_COLOR_0426, imx258mipiraw_COLOR_0427, imx258mipiraw_COLOR_0428, imx258mipiraw_COLOR_0429,
    imx258mipiraw_COLOR_0430, imx258mipiraw_COLOR_0431, imx258mipiraw_COLOR_0432, imx258mipiraw_COLOR_0433, imx258mipiraw_COLOR_0434, imx258mipiraw_COLOR_0435, imx258mipiraw_COLOR_0436, imx258mipiraw_COLOR_0437, imx258mipiraw_COLOR_0438, imx258mipiraw_COLOR_0439,
    imx258mipiraw_COLOR_0440, imx258mipiraw_COLOR_0441, imx258mipiraw_COLOR_0442, imx258mipiraw_COLOR_0443, imx258mipiraw_COLOR_0444, imx258mipiraw_COLOR_0445, imx258mipiraw_COLOR_0446, imx258mipiraw_COLOR_0447, imx258mipiraw_COLOR_0448, imx258mipiraw_COLOR_0449,
    imx258mipiraw_COLOR_0450, imx258mipiraw_COLOR_0451, imx258mipiraw_COLOR_0452, imx258mipiraw_COLOR_0453, imx258mipiraw_COLOR_0454, imx258mipiraw_COLOR_0455, imx258mipiraw_COLOR_0456, imx258mipiraw_COLOR_0457, imx258mipiraw_COLOR_0458, imx258mipiraw_COLOR_0459,
    imx258mipiraw_COLOR_0460, imx258mipiraw_COLOR_0461, imx258mipiraw_COLOR_0462, imx258mipiraw_COLOR_0463, imx258mipiraw_COLOR_0464, imx258mipiraw_COLOR_0465, imx258mipiraw_COLOR_0466, imx258mipiraw_COLOR_0467, imx258mipiraw_COLOR_0468, imx258mipiraw_COLOR_0469,
    imx258mipiraw_COLOR_0470, imx258mipiraw_COLOR_0471, imx258mipiraw_COLOR_0472, imx258mipiraw_COLOR_0473, imx258mipiraw_COLOR_0474, imx258mipiraw_COLOR_0475, imx258mipiraw_COLOR_0476, imx258mipiraw_COLOR_0477, imx258mipiraw_COLOR_0478, imx258mipiraw_COLOR_0479,
    imx258mipiraw_COLOR_0480, imx258mipiraw_COLOR_0481, imx258mipiraw_COLOR_0482, imx258mipiraw_COLOR_0483, imx258mipiraw_COLOR_0484, imx258mipiraw_COLOR_0485, imx258mipiraw_COLOR_0486, imx258mipiraw_COLOR_0487, imx258mipiraw_COLOR_0488, imx258mipiraw_COLOR_0489,
    imx258mipiraw_COLOR_0490, imx258mipiraw_COLOR_0491, imx258mipiraw_COLOR_0492, imx258mipiraw_COLOR_0493, imx258mipiraw_COLOR_0494, imx258mipiraw_COLOR_0495, imx258mipiraw_COLOR_0496, imx258mipiraw_COLOR_0497, imx258mipiraw_COLOR_0498, imx258mipiraw_COLOR_0499,
    imx258mipiraw_COLOR_0500, imx258mipiraw_COLOR_0501, imx258mipiraw_COLOR_0502, imx258mipiraw_COLOR_0503, imx258mipiraw_COLOR_0504, imx258mipiraw_COLOR_0505, imx258mipiraw_COLOR_0506, imx258mipiraw_COLOR_0507, imx258mipiraw_COLOR_0508, imx258mipiraw_COLOR_0509,
    imx258mipiraw_COLOR_0510, imx258mipiraw_COLOR_0511, imx258mipiraw_COLOR_0512, imx258mipiraw_COLOR_0513, imx258mipiraw_COLOR_0514, imx258mipiraw_COLOR_0515, imx258mipiraw_COLOR_0516, imx258mipiraw_COLOR_0517, imx258mipiraw_COLOR_0518, imx258mipiraw_COLOR_0519,
    imx258mipiraw_COLOR_0520, imx258mipiraw_COLOR_0521, imx258mipiraw_COLOR_0522, imx258mipiraw_COLOR_0523, imx258mipiraw_COLOR_0524, imx258mipiraw_COLOR_0525, imx258mipiraw_COLOR_0526, imx258mipiraw_COLOR_0527, imx258mipiraw_COLOR_0528, imx258mipiraw_COLOR_0529,
    imx258mipiraw_COLOR_0530, imx258mipiraw_COLOR_0531, imx258mipiraw_COLOR_0532, imx258mipiraw_COLOR_0533, imx258mipiraw_COLOR_0534, imx258mipiraw_COLOR_0535, imx258mipiraw_COLOR_0536, imx258mipiraw_COLOR_0537, imx258mipiraw_COLOR_0538, imx258mipiraw_COLOR_0539,
    imx258mipiraw_COLOR_0540, imx258mipiraw_COLOR_0541, imx258mipiraw_COLOR_0542, imx258mipiraw_COLOR_0543, imx258mipiraw_COLOR_0544, imx258mipiraw_COLOR_0545, imx258mipiraw_COLOR_0546, imx258mipiraw_COLOR_0547, imx258mipiraw_COLOR_0548, imx258mipiraw_COLOR_0549,
    imx258mipiraw_COLOR_0550, imx258mipiraw_COLOR_0551, imx258mipiraw_COLOR_0552, imx258mipiraw_COLOR_0553, imx258mipiraw_COLOR_0554, imx258mipiraw_COLOR_0555, imx258mipiraw_COLOR_0556, imx258mipiraw_COLOR_0557, imx258mipiraw_COLOR_0558, imx258mipiraw_COLOR_0559,
    imx258mipiraw_COLOR_0560, imx258mipiraw_COLOR_0561, imx258mipiraw_COLOR_0562, imx258mipiraw_COLOR_0563, imx258mipiraw_COLOR_0564, imx258mipiraw_COLOR_0565, imx258mipiraw_COLOR_0566, imx258mipiraw_COLOR_0567, imx258mipiraw_COLOR_0568, imx258mipiraw_COLOR_0569,
    imx258mipiraw_COLOR_0570, imx258mipiraw_COLOR_0571, imx258mipiraw_COLOR_0572, imx258mipiraw_COLOR_0573, imx258mipiraw_COLOR_0574, imx258mipiraw_COLOR_0575, imx258mipiraw_COLOR_0576, imx258mipiraw_COLOR_0577, imx258mipiraw_COLOR_0578, imx258mipiraw_COLOR_0579,
    imx258mipiraw_COLOR_0580, imx258mipiraw_COLOR_0581, imx258mipiraw_COLOR_0582, imx258mipiraw_COLOR_0583, imx258mipiraw_COLOR_0584, imx258mipiraw_COLOR_0585, imx258mipiraw_COLOR_0586, imx258mipiraw_COLOR_0587, imx258mipiraw_COLOR_0588, imx258mipiraw_COLOR_0589,
    imx258mipiraw_COLOR_0590, imx258mipiraw_COLOR_0591, imx258mipiraw_COLOR_0592, imx258mipiraw_COLOR_0593, imx258mipiraw_COLOR_0594, imx258mipiraw_COLOR_0595, imx258mipiraw_COLOR_0596, imx258mipiraw_COLOR_0597, imx258mipiraw_COLOR_0598, imx258mipiraw_COLOR_0599,
    imx258mipiraw_COLOR_0600, imx258mipiraw_COLOR_0601, imx258mipiraw_COLOR_0602, imx258mipiraw_COLOR_0603, imx258mipiraw_COLOR_0604, imx258mipiraw_COLOR_0605, imx258mipiraw_COLOR_0606, imx258mipiraw_COLOR_0607, imx258mipiraw_COLOR_0608, imx258mipiraw_COLOR_0609,
    imx258mipiraw_COLOR_0610, imx258mipiraw_COLOR_0611, imx258mipiraw_COLOR_0612, imx258mipiraw_COLOR_0613, imx258mipiraw_COLOR_0614, imx258mipiraw_COLOR_0615, imx258mipiraw_COLOR_0616, imx258mipiraw_COLOR_0617, imx258mipiraw_COLOR_0618, imx258mipiraw_COLOR_0619,
    imx258mipiraw_COLOR_0620, imx258mipiraw_COLOR_0621, imx258mipiraw_COLOR_0622, imx258mipiraw_COLOR_0623, imx258mipiraw_COLOR_0624, imx258mipiraw_COLOR_0625, imx258mipiraw_COLOR_0626, imx258mipiraw_COLOR_0627, imx258mipiraw_COLOR_0628, imx258mipiraw_COLOR_0629,
    imx258mipiraw_COLOR_0630, imx258mipiraw_COLOR_0631, imx258mipiraw_COLOR_0632, imx258mipiraw_COLOR_0633, imx258mipiraw_COLOR_0634, imx258mipiraw_COLOR_0635, imx258mipiraw_COLOR_0636, imx258mipiraw_COLOR_0637, imx258mipiraw_COLOR_0638, imx258mipiraw_COLOR_0639,
    imx258mipiraw_COLOR_0640, imx258mipiraw_COLOR_0641, imx258mipiraw_COLOR_0642, imx258mipiraw_COLOR_0643, imx258mipiraw_COLOR_0644, imx258mipiraw_COLOR_0645, imx258mipiraw_COLOR_0646, imx258mipiraw_COLOR_0647, imx258mipiraw_COLOR_0648, imx258mipiraw_COLOR_0649,
    imx258mipiraw_COLOR_0650, imx258mipiraw_COLOR_0651, imx258mipiraw_COLOR_0652, imx258mipiraw_COLOR_0653, imx258mipiraw_COLOR_0654, imx258mipiraw_COLOR_0655, imx258mipiraw_COLOR_0656, imx258mipiraw_COLOR_0657, imx258mipiraw_COLOR_0658, imx258mipiraw_COLOR_0659,
    imx258mipiraw_COLOR_0660, imx258mipiraw_COLOR_0661, imx258mipiraw_COLOR_0662, imx258mipiraw_COLOR_0663, imx258mipiraw_COLOR_0664, imx258mipiraw_COLOR_0665, imx258mipiraw_COLOR_0666, imx258mipiraw_COLOR_0667, imx258mipiraw_COLOR_0668, imx258mipiraw_COLOR_0669,
    imx258mipiraw_COLOR_0670, imx258mipiraw_COLOR_0671, imx258mipiraw_COLOR_0672, imx258mipiraw_COLOR_0673, imx258mipiraw_COLOR_0674, imx258mipiraw_COLOR_0675, imx258mipiraw_COLOR_0676, imx258mipiraw_COLOR_0677, imx258mipiraw_COLOR_0678, imx258mipiraw_COLOR_0679,
    imx258mipiraw_COLOR_0680, imx258mipiraw_COLOR_0681, imx258mipiraw_COLOR_0682, imx258mipiraw_COLOR_0683, imx258mipiraw_COLOR_0684, imx258mipiraw_COLOR_0685, imx258mipiraw_COLOR_0686, imx258mipiraw_COLOR_0687, imx258mipiraw_COLOR_0688, imx258mipiraw_COLOR_0689,
    imx258mipiraw_COLOR_0690, imx258mipiraw_COLOR_0691, imx258mipiraw_COLOR_0692, imx258mipiraw_COLOR_0693, imx258mipiraw_COLOR_0694, imx258mipiraw_COLOR_0695, imx258mipiraw_COLOR_0696, imx258mipiraw_COLOR_0697, imx258mipiraw_COLOR_0698, imx258mipiraw_COLOR_0699,
    imx258mipiraw_COLOR_0700, imx258mipiraw_COLOR_0701, imx258mipiraw_COLOR_0702, imx258mipiraw_COLOR_0703, imx258mipiraw_COLOR_0704, imx258mipiraw_COLOR_0705, imx258mipiraw_COLOR_0706, imx258mipiraw_COLOR_0707, imx258mipiraw_COLOR_0708, imx258mipiraw_COLOR_0709,
    imx258mipiraw_COLOR_0710, imx258mipiraw_COLOR_0711, imx258mipiraw_COLOR_0712, imx258mipiraw_COLOR_0713, imx258mipiraw_COLOR_0714, imx258mipiraw_COLOR_0715, imx258mipiraw_COLOR_0716, imx258mipiraw_COLOR_0717, imx258mipiraw_COLOR_0718, imx258mipiraw_COLOR_0719,
    imx258mipiraw_COLOR_0720, imx258mipiraw_COLOR_0721, imx258mipiraw_COLOR_0722, imx258mipiraw_COLOR_0723, imx258mipiraw_COLOR_0724, imx258mipiraw_COLOR_0725, imx258mipiraw_COLOR_0726, imx258mipiraw_COLOR_0727, imx258mipiraw_COLOR_0728, imx258mipiraw_COLOR_0729,
    imx258mipiraw_COLOR_0730, imx258mipiraw_COLOR_0731, imx258mipiraw_COLOR_0732, imx258mipiraw_COLOR_0733, imx258mipiraw_COLOR_0734, imx258mipiraw_COLOR_0735, imx258mipiraw_COLOR_0736, imx258mipiraw_COLOR_0737, imx258mipiraw_COLOR_0738, imx258mipiraw_COLOR_0739,
    imx258mipiraw_COLOR_0740, imx258mipiraw_COLOR_0741, imx258mipiraw_COLOR_0742, imx258mipiraw_COLOR_0743, imx258mipiraw_COLOR_0744, imx258mipiraw_COLOR_0745, imx258mipiraw_COLOR_0746, imx258mipiraw_COLOR_0747, imx258mipiraw_COLOR_0748, imx258mipiraw_COLOR_0749,
    imx258mipiraw_COLOR_0750, imx258mipiraw_COLOR_0751, imx258mipiraw_COLOR_0752, imx258mipiraw_COLOR_0753, imx258mipiraw_COLOR_0754, imx258mipiraw_COLOR_0755, imx258mipiraw_COLOR_0756, imx258mipiraw_COLOR_0757, imx258mipiraw_COLOR_0758, imx258mipiraw_COLOR_0759,
    imx258mipiraw_COLOR_0760, imx258mipiraw_COLOR_0761, imx258mipiraw_COLOR_0762, imx258mipiraw_COLOR_0763, imx258mipiraw_COLOR_0764, imx258mipiraw_COLOR_0765, imx258mipiraw_COLOR_0766, imx258mipiraw_COLOR_0767, imx258mipiraw_COLOR_0768, imx258mipiraw_COLOR_0769,
    imx258mipiraw_COLOR_0770, imx258mipiraw_COLOR_0771, imx258mipiraw_COLOR_0772, imx258mipiraw_COLOR_0773, imx258mipiraw_COLOR_0774, imx258mipiraw_COLOR_0775, imx258mipiraw_COLOR_0776, imx258mipiraw_COLOR_0777, imx258mipiraw_COLOR_0778, imx258mipiraw_COLOR_0779,
    imx258mipiraw_COLOR_0780, imx258mipiraw_COLOR_0781, imx258mipiraw_COLOR_0782, imx258mipiraw_COLOR_0783, imx258mipiraw_COLOR_0784, imx258mipiraw_COLOR_0785, imx258mipiraw_COLOR_0786, imx258mipiraw_COLOR_0787, imx258mipiraw_COLOR_0788, imx258mipiraw_COLOR_0789,
    imx258mipiraw_COLOR_0790, imx258mipiraw_COLOR_0791, imx258mipiraw_COLOR_0792, imx258mipiraw_COLOR_0793, imx258mipiraw_COLOR_0794, imx258mipiraw_COLOR_0795, imx258mipiraw_COLOR_0796, imx258mipiraw_COLOR_0797, imx258mipiraw_COLOR_0798, imx258mipiraw_COLOR_0799,
    imx258mipiraw_COLOR_0800, imx258mipiraw_COLOR_0801, imx258mipiraw_COLOR_0802, imx258mipiraw_COLOR_0803, imx258mipiraw_COLOR_0804, imx258mipiraw_COLOR_0805, imx258mipiraw_COLOR_0806, imx258mipiraw_COLOR_0807, imx258mipiraw_COLOR_0808, imx258mipiraw_COLOR_0809,
    imx258mipiraw_COLOR_0810, imx258mipiraw_COLOR_0811, imx258mipiraw_COLOR_0812, imx258mipiraw_COLOR_0813, imx258mipiraw_COLOR_0814, imx258mipiraw_COLOR_0815, imx258mipiraw_COLOR_0816, imx258mipiraw_COLOR_0817, imx258mipiraw_COLOR_0818, imx258mipiraw_COLOR_0819,
    imx258mipiraw_COLOR_0820, imx258mipiraw_COLOR_0821, imx258mipiraw_COLOR_0822, imx258mipiraw_COLOR_0823, imx258mipiraw_COLOR_0824, imx258mipiraw_COLOR_0825, imx258mipiraw_COLOR_0826, imx258mipiraw_COLOR_0827, imx258mipiraw_COLOR_0828, imx258mipiraw_COLOR_0829,
    imx258mipiraw_COLOR_0830, imx258mipiraw_COLOR_0831, imx258mipiraw_COLOR_0832, imx258mipiraw_COLOR_0833, imx258mipiraw_COLOR_0834, imx258mipiraw_COLOR_0835, imx258mipiraw_COLOR_0836, imx258mipiraw_COLOR_0837, imx258mipiraw_COLOR_0838, imx258mipiraw_COLOR_0839,
    imx258mipiraw_COLOR_0840, imx258mipiraw_COLOR_0841, imx258mipiraw_COLOR_0842, imx258mipiraw_COLOR_0843, imx258mipiraw_COLOR_0844, imx258mipiraw_COLOR_0845, imx258mipiraw_COLOR_0846, imx258mipiraw_COLOR_0847, imx258mipiraw_COLOR_0848, imx258mipiraw_COLOR_0849,
    imx258mipiraw_COLOR_0850, imx258mipiraw_COLOR_0851, imx258mipiraw_COLOR_0852, imx258mipiraw_COLOR_0853, imx258mipiraw_COLOR_0854, imx258mipiraw_COLOR_0855, imx258mipiraw_COLOR_0856, imx258mipiraw_COLOR_0857, imx258mipiraw_COLOR_0858, imx258mipiraw_COLOR_0859,
    imx258mipiraw_COLOR_0860, imx258mipiraw_COLOR_0861, imx258mipiraw_COLOR_0862, imx258mipiraw_COLOR_0863, imx258mipiraw_COLOR_0864, imx258mipiraw_COLOR_0865, imx258mipiraw_COLOR_0866, imx258mipiraw_COLOR_0867, imx258mipiraw_COLOR_0868, imx258mipiraw_COLOR_0869,
    imx258mipiraw_COLOR_0870, imx258mipiraw_COLOR_0871, imx258mipiraw_COLOR_0872, imx258mipiraw_COLOR_0873, imx258mipiraw_COLOR_0874, imx258mipiraw_COLOR_0875, imx258mipiraw_COLOR_0876, imx258mipiraw_COLOR_0877, imx258mipiraw_COLOR_0878, imx258mipiraw_COLOR_0879,
    imx258mipiraw_COLOR_0880, imx258mipiraw_COLOR_0881, imx258mipiraw_COLOR_0882, imx258mipiraw_COLOR_0883, imx258mipiraw_COLOR_0884, imx258mipiraw_COLOR_0885, imx258mipiraw_COLOR_0886, imx258mipiraw_COLOR_0887, imx258mipiraw_COLOR_0888, imx258mipiraw_COLOR_0889,
    imx258mipiraw_COLOR_0890, imx258mipiraw_COLOR_0891, imx258mipiraw_COLOR_0892, imx258mipiraw_COLOR_0893, imx258mipiraw_COLOR_0894, imx258mipiraw_COLOR_0895, imx258mipiraw_COLOR_0896, imx258mipiraw_COLOR_0897, imx258mipiraw_COLOR_0898, imx258mipiraw_COLOR_0899,
    imx258mipiraw_COLOR_0900, imx258mipiraw_COLOR_0901, imx258mipiraw_COLOR_0902, imx258mipiraw_COLOR_0903, imx258mipiraw_COLOR_0904, imx258mipiraw_COLOR_0905, imx258mipiraw_COLOR_0906, imx258mipiraw_COLOR_0907, imx258mipiraw_COLOR_0908, imx258mipiraw_COLOR_0909,
    imx258mipiraw_COLOR_0910, imx258mipiraw_COLOR_0911, imx258mipiraw_COLOR_0912, imx258mipiraw_COLOR_0913, imx258mipiraw_COLOR_0914, imx258mipiraw_COLOR_0915, imx258mipiraw_COLOR_0916, imx258mipiraw_COLOR_0917, imx258mipiraw_COLOR_0918, imx258mipiraw_COLOR_0919,
    imx258mipiraw_COLOR_0920, imx258mipiraw_COLOR_0921, imx258mipiraw_COLOR_0922, imx258mipiraw_COLOR_0923, imx258mipiraw_COLOR_0924, imx258mipiraw_COLOR_0925, imx258mipiraw_COLOR_0926, imx258mipiraw_COLOR_0927, imx258mipiraw_COLOR_0928, imx258mipiraw_COLOR_0929,
    imx258mipiraw_COLOR_0930, imx258mipiraw_COLOR_0931, imx258mipiraw_COLOR_0932, imx258mipiraw_COLOR_0933, imx258mipiraw_COLOR_0934, imx258mipiraw_COLOR_0935, imx258mipiraw_COLOR_0936, imx258mipiraw_COLOR_0937, imx258mipiraw_COLOR_0938, imx258mipiraw_COLOR_0939,
    imx258mipiraw_COLOR_0940, imx258mipiraw_COLOR_0941, imx258mipiraw_COLOR_0942, imx258mipiraw_COLOR_0943, imx258mipiraw_COLOR_0944, imx258mipiraw_COLOR_0945, imx258mipiraw_COLOR_0946, imx258mipiraw_COLOR_0947, imx258mipiraw_COLOR_0948, imx258mipiraw_COLOR_0949,
    imx258mipiraw_COLOR_0950, imx258mipiraw_COLOR_0951, imx258mipiraw_COLOR_0952, imx258mipiraw_COLOR_0953, imx258mipiraw_COLOR_0954, imx258mipiraw_COLOR_0955, imx258mipiraw_COLOR_0956, imx258mipiraw_COLOR_0957, imx258mipiraw_COLOR_0958, imx258mipiraw_COLOR_0959,
},
.PCA = {
    imx258mipiraw_PCA_0000,
},
.COLOR_PARAM = {
    imx258mipiraw_COLOR_PARAM_0000, imx258mipiraw_COLOR_PARAM_0001, imx258mipiraw_COLOR_PARAM_0002, imx258mipiraw_COLOR_PARAM_0003, imx258mipiraw_COLOR_PARAM_0004, imx258mipiraw_COLOR_PARAM_0005, imx258mipiraw_COLOR_PARAM_0006, imx258mipiraw_COLOR_PARAM_0007, imx258mipiraw_COLOR_PARAM_0008, imx258mipiraw_COLOR_PARAM_0009,
    imx258mipiraw_COLOR_PARAM_0010, imx258mipiraw_COLOR_PARAM_0011, imx258mipiraw_COLOR_PARAM_0012, imx258mipiraw_COLOR_PARAM_0013, imx258mipiraw_COLOR_PARAM_0014, imx258mipiraw_COLOR_PARAM_0015,
},
