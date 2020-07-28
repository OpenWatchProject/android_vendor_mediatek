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
    imx398mipiraw_CCM_0000, imx398mipiraw_CCM_0001, imx398mipiraw_CCM_0002, imx398mipiraw_CCM_0003, imx398mipiraw_CCM_0004, imx398mipiraw_CCM_0005, imx398mipiraw_CCM_0006, imx398mipiraw_CCM_0007, imx398mipiraw_CCM_0008, imx398mipiraw_CCM_0009,
    imx398mipiraw_CCM_0010, imx398mipiraw_CCM_0011, imx398mipiraw_CCM_0012, imx398mipiraw_CCM_0013, imx398mipiraw_CCM_0014, imx398mipiraw_CCM_0015, imx398mipiraw_CCM_0016, imx398mipiraw_CCM_0017, imx398mipiraw_CCM_0018, imx398mipiraw_CCM_0019,
    imx398mipiraw_CCM_0020, imx398mipiraw_CCM_0021, imx398mipiraw_CCM_0022, imx398mipiraw_CCM_0023, imx398mipiraw_CCM_0024, imx398mipiraw_CCM_0025, imx398mipiraw_CCM_0026, imx398mipiraw_CCM_0027, imx398mipiraw_CCM_0028, imx398mipiraw_CCM_0029,
    imx398mipiraw_CCM_0030, imx398mipiraw_CCM_0031, imx398mipiraw_CCM_0032, imx398mipiraw_CCM_0033, imx398mipiraw_CCM_0034, imx398mipiraw_CCM_0035, imx398mipiraw_CCM_0036, imx398mipiraw_CCM_0037, imx398mipiraw_CCM_0038, imx398mipiraw_CCM_0039,
    imx398mipiraw_CCM_0040, imx398mipiraw_CCM_0041, imx398mipiraw_CCM_0042, imx398mipiraw_CCM_0043, imx398mipiraw_CCM_0044, imx398mipiraw_CCM_0045, imx398mipiraw_CCM_0046, imx398mipiraw_CCM_0047, imx398mipiraw_CCM_0048, imx398mipiraw_CCM_0049,
    imx398mipiraw_CCM_0050, imx398mipiraw_CCM_0051, imx398mipiraw_CCM_0052, imx398mipiraw_CCM_0053, imx398mipiraw_CCM_0054, imx398mipiraw_CCM_0055, imx398mipiraw_CCM_0056, imx398mipiraw_CCM_0057, imx398mipiraw_CCM_0058, imx398mipiraw_CCM_0059,
    imx398mipiraw_CCM_0060, imx398mipiraw_CCM_0061, imx398mipiraw_CCM_0062, imx398mipiraw_CCM_0063, imx398mipiraw_CCM_0064, imx398mipiraw_CCM_0065, imx398mipiraw_CCM_0066, imx398mipiraw_CCM_0067, imx398mipiraw_CCM_0068, imx398mipiraw_CCM_0069,
    imx398mipiraw_CCM_0070, imx398mipiraw_CCM_0071, imx398mipiraw_CCM_0072, imx398mipiraw_CCM_0073, imx398mipiraw_CCM_0074, imx398mipiraw_CCM_0075, imx398mipiraw_CCM_0076, imx398mipiraw_CCM_0077, imx398mipiraw_CCM_0078, imx398mipiraw_CCM_0079,
    imx398mipiraw_CCM_0080, imx398mipiraw_CCM_0081, imx398mipiraw_CCM_0082, imx398mipiraw_CCM_0083, imx398mipiraw_CCM_0084, imx398mipiraw_CCM_0085, imx398mipiraw_CCM_0086, imx398mipiraw_CCM_0087, imx398mipiraw_CCM_0088, imx398mipiraw_CCM_0089,
    imx398mipiraw_CCM_0090, imx398mipiraw_CCM_0091, imx398mipiraw_CCM_0092, imx398mipiraw_CCM_0093, imx398mipiraw_CCM_0094, imx398mipiraw_CCM_0095, imx398mipiraw_CCM_0096, imx398mipiraw_CCM_0097, imx398mipiraw_CCM_0098, imx398mipiraw_CCM_0099,
    imx398mipiraw_CCM_0100, imx398mipiraw_CCM_0101, imx398mipiraw_CCM_0102, imx398mipiraw_CCM_0103, imx398mipiraw_CCM_0104, imx398mipiraw_CCM_0105, imx398mipiraw_CCM_0106, imx398mipiraw_CCM_0107, imx398mipiraw_CCM_0108, imx398mipiraw_CCM_0109,
    imx398mipiraw_CCM_0110, imx398mipiraw_CCM_0111, imx398mipiraw_CCM_0112, imx398mipiraw_CCM_0113, imx398mipiraw_CCM_0114, imx398mipiraw_CCM_0115, imx398mipiraw_CCM_0116, imx398mipiraw_CCM_0117, imx398mipiraw_CCM_0118, imx398mipiraw_CCM_0119,
    imx398mipiraw_CCM_0120, imx398mipiraw_CCM_0121, imx398mipiraw_CCM_0122, imx398mipiraw_CCM_0123, imx398mipiraw_CCM_0124, imx398mipiraw_CCM_0125, imx398mipiraw_CCM_0126, imx398mipiraw_CCM_0127, imx398mipiraw_CCM_0128, imx398mipiraw_CCM_0129,
    imx398mipiraw_CCM_0130, imx398mipiraw_CCM_0131, imx398mipiraw_CCM_0132, imx398mipiraw_CCM_0133, imx398mipiraw_CCM_0134, imx398mipiraw_CCM_0135, imx398mipiraw_CCM_0136, imx398mipiraw_CCM_0137, imx398mipiraw_CCM_0138, imx398mipiraw_CCM_0139,
    imx398mipiraw_CCM_0140, imx398mipiraw_CCM_0141, imx398mipiraw_CCM_0142, imx398mipiraw_CCM_0143, imx398mipiraw_CCM_0144, imx398mipiraw_CCM_0145, imx398mipiraw_CCM_0146, imx398mipiraw_CCM_0147, imx398mipiraw_CCM_0148, imx398mipiraw_CCM_0149,
    imx398mipiraw_CCM_0150, imx398mipiraw_CCM_0151, imx398mipiraw_CCM_0152, imx398mipiraw_CCM_0153, imx398mipiraw_CCM_0154, imx398mipiraw_CCM_0155, imx398mipiraw_CCM_0156, imx398mipiraw_CCM_0157, imx398mipiraw_CCM_0158, imx398mipiraw_CCM_0159,
    imx398mipiraw_CCM_0160, imx398mipiraw_CCM_0161, imx398mipiraw_CCM_0162, imx398mipiraw_CCM_0163, imx398mipiraw_CCM_0164, imx398mipiraw_CCM_0165, imx398mipiraw_CCM_0166, imx398mipiraw_CCM_0167, imx398mipiraw_CCM_0168, imx398mipiraw_CCM_0169,
    imx398mipiraw_CCM_0170, imx398mipiraw_CCM_0171, imx398mipiraw_CCM_0172, imx398mipiraw_CCM_0173, imx398mipiraw_CCM_0174, imx398mipiraw_CCM_0175, imx398mipiraw_CCM_0176, imx398mipiraw_CCM_0177, imx398mipiraw_CCM_0178, imx398mipiraw_CCM_0179,
    imx398mipiraw_CCM_0180, imx398mipiraw_CCM_0181, imx398mipiraw_CCM_0182, imx398mipiraw_CCM_0183, imx398mipiraw_CCM_0184, imx398mipiraw_CCM_0185, imx398mipiraw_CCM_0186, imx398mipiraw_CCM_0187, imx398mipiraw_CCM_0188, imx398mipiraw_CCM_0189,
    imx398mipiraw_CCM_0190, imx398mipiraw_CCM_0191, imx398mipiraw_CCM_0192, imx398mipiraw_CCM_0193, imx398mipiraw_CCM_0194, imx398mipiraw_CCM_0195, imx398mipiraw_CCM_0196, imx398mipiraw_CCM_0197, imx398mipiraw_CCM_0198, imx398mipiraw_CCM_0199,
    imx398mipiraw_CCM_0200, imx398mipiraw_CCM_0201, imx398mipiraw_CCM_0202, imx398mipiraw_CCM_0203, imx398mipiraw_CCM_0204, imx398mipiraw_CCM_0205, imx398mipiraw_CCM_0206, imx398mipiraw_CCM_0207, imx398mipiraw_CCM_0208, imx398mipiraw_CCM_0209,
    imx398mipiraw_CCM_0210, imx398mipiraw_CCM_0211, imx398mipiraw_CCM_0212, imx398mipiraw_CCM_0213, imx398mipiraw_CCM_0214, imx398mipiraw_CCM_0215, imx398mipiraw_CCM_0216, imx398mipiraw_CCM_0217, imx398mipiraw_CCM_0218, imx398mipiraw_CCM_0219,
    imx398mipiraw_CCM_0220, imx398mipiraw_CCM_0221, imx398mipiraw_CCM_0222, imx398mipiraw_CCM_0223, imx398mipiraw_CCM_0224, imx398mipiraw_CCM_0225, imx398mipiraw_CCM_0226, imx398mipiraw_CCM_0227, imx398mipiraw_CCM_0228, imx398mipiraw_CCM_0229,
    imx398mipiraw_CCM_0230, imx398mipiraw_CCM_0231, imx398mipiraw_CCM_0232, imx398mipiraw_CCM_0233, imx398mipiraw_CCM_0234, imx398mipiraw_CCM_0235, imx398mipiraw_CCM_0236, imx398mipiraw_CCM_0237, imx398mipiraw_CCM_0238, imx398mipiraw_CCM_0239,
},
.COLOR = {
    imx398mipiraw_COLOR_0000, imx398mipiraw_COLOR_0001, imx398mipiraw_COLOR_0002, imx398mipiraw_COLOR_0003, imx398mipiraw_COLOR_0004, imx398mipiraw_COLOR_0005, imx398mipiraw_COLOR_0006, imx398mipiraw_COLOR_0007, imx398mipiraw_COLOR_0008, imx398mipiraw_COLOR_0009,
    imx398mipiraw_COLOR_0010, imx398mipiraw_COLOR_0011, imx398mipiraw_COLOR_0012, imx398mipiraw_COLOR_0013, imx398mipiraw_COLOR_0014, imx398mipiraw_COLOR_0015, imx398mipiraw_COLOR_0016, imx398mipiraw_COLOR_0017, imx398mipiraw_COLOR_0018, imx398mipiraw_COLOR_0019,
    imx398mipiraw_COLOR_0020, imx398mipiraw_COLOR_0021, imx398mipiraw_COLOR_0022, imx398mipiraw_COLOR_0023, imx398mipiraw_COLOR_0024, imx398mipiraw_COLOR_0025, imx398mipiraw_COLOR_0026, imx398mipiraw_COLOR_0027, imx398mipiraw_COLOR_0028, imx398mipiraw_COLOR_0029,
    imx398mipiraw_COLOR_0030, imx398mipiraw_COLOR_0031, imx398mipiraw_COLOR_0032, imx398mipiraw_COLOR_0033, imx398mipiraw_COLOR_0034, imx398mipiraw_COLOR_0035, imx398mipiraw_COLOR_0036, imx398mipiraw_COLOR_0037, imx398mipiraw_COLOR_0038, imx398mipiraw_COLOR_0039,
    imx398mipiraw_COLOR_0040, imx398mipiraw_COLOR_0041, imx398mipiraw_COLOR_0042, imx398mipiraw_COLOR_0043, imx398mipiraw_COLOR_0044, imx398mipiraw_COLOR_0045, imx398mipiraw_COLOR_0046, imx398mipiraw_COLOR_0047, imx398mipiraw_COLOR_0048, imx398mipiraw_COLOR_0049,
    imx398mipiraw_COLOR_0050, imx398mipiraw_COLOR_0051, imx398mipiraw_COLOR_0052, imx398mipiraw_COLOR_0053, imx398mipiraw_COLOR_0054, imx398mipiraw_COLOR_0055, imx398mipiraw_COLOR_0056, imx398mipiraw_COLOR_0057, imx398mipiraw_COLOR_0058, imx398mipiraw_COLOR_0059,
    imx398mipiraw_COLOR_0060, imx398mipiraw_COLOR_0061, imx398mipiraw_COLOR_0062, imx398mipiraw_COLOR_0063, imx398mipiraw_COLOR_0064, imx398mipiraw_COLOR_0065, imx398mipiraw_COLOR_0066, imx398mipiraw_COLOR_0067, imx398mipiraw_COLOR_0068, imx398mipiraw_COLOR_0069,
    imx398mipiraw_COLOR_0070, imx398mipiraw_COLOR_0071, imx398mipiraw_COLOR_0072, imx398mipiraw_COLOR_0073, imx398mipiraw_COLOR_0074, imx398mipiraw_COLOR_0075, imx398mipiraw_COLOR_0076, imx398mipiraw_COLOR_0077, imx398mipiraw_COLOR_0078, imx398mipiraw_COLOR_0079,
    imx398mipiraw_COLOR_0080, imx398mipiraw_COLOR_0081, imx398mipiraw_COLOR_0082, imx398mipiraw_COLOR_0083, imx398mipiraw_COLOR_0084, imx398mipiraw_COLOR_0085, imx398mipiraw_COLOR_0086, imx398mipiraw_COLOR_0087, imx398mipiraw_COLOR_0088, imx398mipiraw_COLOR_0089,
    imx398mipiraw_COLOR_0090, imx398mipiraw_COLOR_0091, imx398mipiraw_COLOR_0092, imx398mipiraw_COLOR_0093, imx398mipiraw_COLOR_0094, imx398mipiraw_COLOR_0095, imx398mipiraw_COLOR_0096, imx398mipiraw_COLOR_0097, imx398mipiraw_COLOR_0098, imx398mipiraw_COLOR_0099,
    imx398mipiraw_COLOR_0100, imx398mipiraw_COLOR_0101, imx398mipiraw_COLOR_0102, imx398mipiraw_COLOR_0103, imx398mipiraw_COLOR_0104, imx398mipiraw_COLOR_0105, imx398mipiraw_COLOR_0106, imx398mipiraw_COLOR_0107, imx398mipiraw_COLOR_0108, imx398mipiraw_COLOR_0109,
    imx398mipiraw_COLOR_0110, imx398mipiraw_COLOR_0111, imx398mipiraw_COLOR_0112, imx398mipiraw_COLOR_0113, imx398mipiraw_COLOR_0114, imx398mipiraw_COLOR_0115, imx398mipiraw_COLOR_0116, imx398mipiraw_COLOR_0117, imx398mipiraw_COLOR_0118, imx398mipiraw_COLOR_0119,
    imx398mipiraw_COLOR_0120, imx398mipiraw_COLOR_0121, imx398mipiraw_COLOR_0122, imx398mipiraw_COLOR_0123, imx398mipiraw_COLOR_0124, imx398mipiraw_COLOR_0125, imx398mipiraw_COLOR_0126, imx398mipiraw_COLOR_0127, imx398mipiraw_COLOR_0128, imx398mipiraw_COLOR_0129,
    imx398mipiraw_COLOR_0130, imx398mipiraw_COLOR_0131, imx398mipiraw_COLOR_0132, imx398mipiraw_COLOR_0133, imx398mipiraw_COLOR_0134, imx398mipiraw_COLOR_0135, imx398mipiraw_COLOR_0136, imx398mipiraw_COLOR_0137, imx398mipiraw_COLOR_0138, imx398mipiraw_COLOR_0139,
    imx398mipiraw_COLOR_0140, imx398mipiraw_COLOR_0141, imx398mipiraw_COLOR_0142, imx398mipiraw_COLOR_0143, imx398mipiraw_COLOR_0144, imx398mipiraw_COLOR_0145, imx398mipiraw_COLOR_0146, imx398mipiraw_COLOR_0147, imx398mipiraw_COLOR_0148, imx398mipiraw_COLOR_0149,
    imx398mipiraw_COLOR_0150, imx398mipiraw_COLOR_0151, imx398mipiraw_COLOR_0152, imx398mipiraw_COLOR_0153, imx398mipiraw_COLOR_0154, imx398mipiraw_COLOR_0155, imx398mipiraw_COLOR_0156, imx398mipiraw_COLOR_0157, imx398mipiraw_COLOR_0158, imx398mipiraw_COLOR_0159,
    imx398mipiraw_COLOR_0160, imx398mipiraw_COLOR_0161, imx398mipiraw_COLOR_0162, imx398mipiraw_COLOR_0163, imx398mipiraw_COLOR_0164, imx398mipiraw_COLOR_0165, imx398mipiraw_COLOR_0166, imx398mipiraw_COLOR_0167, imx398mipiraw_COLOR_0168, imx398mipiraw_COLOR_0169,
    imx398mipiraw_COLOR_0170, imx398mipiraw_COLOR_0171, imx398mipiraw_COLOR_0172, imx398mipiraw_COLOR_0173, imx398mipiraw_COLOR_0174, imx398mipiraw_COLOR_0175, imx398mipiraw_COLOR_0176, imx398mipiraw_COLOR_0177, imx398mipiraw_COLOR_0178, imx398mipiraw_COLOR_0179,
    imx398mipiraw_COLOR_0180, imx398mipiraw_COLOR_0181, imx398mipiraw_COLOR_0182, imx398mipiraw_COLOR_0183, imx398mipiraw_COLOR_0184, imx398mipiraw_COLOR_0185, imx398mipiraw_COLOR_0186, imx398mipiraw_COLOR_0187, imx398mipiraw_COLOR_0188, imx398mipiraw_COLOR_0189,
    imx398mipiraw_COLOR_0190, imx398mipiraw_COLOR_0191, imx398mipiraw_COLOR_0192, imx398mipiraw_COLOR_0193, imx398mipiraw_COLOR_0194, imx398mipiraw_COLOR_0195, imx398mipiraw_COLOR_0196, imx398mipiraw_COLOR_0197, imx398mipiraw_COLOR_0198, imx398mipiraw_COLOR_0199,
    imx398mipiraw_COLOR_0200, imx398mipiraw_COLOR_0201, imx398mipiraw_COLOR_0202, imx398mipiraw_COLOR_0203, imx398mipiraw_COLOR_0204, imx398mipiraw_COLOR_0205, imx398mipiraw_COLOR_0206, imx398mipiraw_COLOR_0207, imx398mipiraw_COLOR_0208, imx398mipiraw_COLOR_0209,
    imx398mipiraw_COLOR_0210, imx398mipiraw_COLOR_0211, imx398mipiraw_COLOR_0212, imx398mipiraw_COLOR_0213, imx398mipiraw_COLOR_0214, imx398mipiraw_COLOR_0215, imx398mipiraw_COLOR_0216, imx398mipiraw_COLOR_0217, imx398mipiraw_COLOR_0218, imx398mipiraw_COLOR_0219,
    imx398mipiraw_COLOR_0220, imx398mipiraw_COLOR_0221, imx398mipiraw_COLOR_0222, imx398mipiraw_COLOR_0223, imx398mipiraw_COLOR_0224, imx398mipiraw_COLOR_0225, imx398mipiraw_COLOR_0226, imx398mipiraw_COLOR_0227, imx398mipiraw_COLOR_0228, imx398mipiraw_COLOR_0229,
    imx398mipiraw_COLOR_0230, imx398mipiraw_COLOR_0231, imx398mipiraw_COLOR_0232, imx398mipiraw_COLOR_0233, imx398mipiraw_COLOR_0234, imx398mipiraw_COLOR_0235, imx398mipiraw_COLOR_0236, imx398mipiraw_COLOR_0237, imx398mipiraw_COLOR_0238, imx398mipiraw_COLOR_0239,
    imx398mipiraw_COLOR_0240, imx398mipiraw_COLOR_0241, imx398mipiraw_COLOR_0242, imx398mipiraw_COLOR_0243, imx398mipiraw_COLOR_0244, imx398mipiraw_COLOR_0245, imx398mipiraw_COLOR_0246, imx398mipiraw_COLOR_0247, imx398mipiraw_COLOR_0248, imx398mipiraw_COLOR_0249,
    imx398mipiraw_COLOR_0250, imx398mipiraw_COLOR_0251, imx398mipiraw_COLOR_0252, imx398mipiraw_COLOR_0253, imx398mipiraw_COLOR_0254, imx398mipiraw_COLOR_0255, imx398mipiraw_COLOR_0256, imx398mipiraw_COLOR_0257, imx398mipiraw_COLOR_0258, imx398mipiraw_COLOR_0259,
    imx398mipiraw_COLOR_0260, imx398mipiraw_COLOR_0261, imx398mipiraw_COLOR_0262, imx398mipiraw_COLOR_0263, imx398mipiraw_COLOR_0264, imx398mipiraw_COLOR_0265, imx398mipiraw_COLOR_0266, imx398mipiraw_COLOR_0267, imx398mipiraw_COLOR_0268, imx398mipiraw_COLOR_0269,
    imx398mipiraw_COLOR_0270, imx398mipiraw_COLOR_0271, imx398mipiraw_COLOR_0272, imx398mipiraw_COLOR_0273, imx398mipiraw_COLOR_0274, imx398mipiraw_COLOR_0275, imx398mipiraw_COLOR_0276, imx398mipiraw_COLOR_0277, imx398mipiraw_COLOR_0278, imx398mipiraw_COLOR_0279,
    imx398mipiraw_COLOR_0280, imx398mipiraw_COLOR_0281, imx398mipiraw_COLOR_0282, imx398mipiraw_COLOR_0283, imx398mipiraw_COLOR_0284, imx398mipiraw_COLOR_0285, imx398mipiraw_COLOR_0286, imx398mipiraw_COLOR_0287, imx398mipiraw_COLOR_0288, imx398mipiraw_COLOR_0289,
    imx398mipiraw_COLOR_0290, imx398mipiraw_COLOR_0291, imx398mipiraw_COLOR_0292, imx398mipiraw_COLOR_0293, imx398mipiraw_COLOR_0294, imx398mipiraw_COLOR_0295, imx398mipiraw_COLOR_0296, imx398mipiraw_COLOR_0297, imx398mipiraw_COLOR_0298, imx398mipiraw_COLOR_0299,
    imx398mipiraw_COLOR_0300, imx398mipiraw_COLOR_0301, imx398mipiraw_COLOR_0302, imx398mipiraw_COLOR_0303, imx398mipiraw_COLOR_0304, imx398mipiraw_COLOR_0305, imx398mipiraw_COLOR_0306, imx398mipiraw_COLOR_0307, imx398mipiraw_COLOR_0308, imx398mipiraw_COLOR_0309,
    imx398mipiraw_COLOR_0310, imx398mipiraw_COLOR_0311, imx398mipiraw_COLOR_0312, imx398mipiraw_COLOR_0313, imx398mipiraw_COLOR_0314, imx398mipiraw_COLOR_0315, imx398mipiraw_COLOR_0316, imx398mipiraw_COLOR_0317, imx398mipiraw_COLOR_0318, imx398mipiraw_COLOR_0319,
    imx398mipiraw_COLOR_0320, imx398mipiraw_COLOR_0321, imx398mipiraw_COLOR_0322, imx398mipiraw_COLOR_0323, imx398mipiraw_COLOR_0324, imx398mipiraw_COLOR_0325, imx398mipiraw_COLOR_0326, imx398mipiraw_COLOR_0327, imx398mipiraw_COLOR_0328, imx398mipiraw_COLOR_0329,
    imx398mipiraw_COLOR_0330, imx398mipiraw_COLOR_0331, imx398mipiraw_COLOR_0332, imx398mipiraw_COLOR_0333, imx398mipiraw_COLOR_0334, imx398mipiraw_COLOR_0335, imx398mipiraw_COLOR_0336, imx398mipiraw_COLOR_0337, imx398mipiraw_COLOR_0338, imx398mipiraw_COLOR_0339,
    imx398mipiraw_COLOR_0340, imx398mipiraw_COLOR_0341, imx398mipiraw_COLOR_0342, imx398mipiraw_COLOR_0343, imx398mipiraw_COLOR_0344, imx398mipiraw_COLOR_0345, imx398mipiraw_COLOR_0346, imx398mipiraw_COLOR_0347, imx398mipiraw_COLOR_0348, imx398mipiraw_COLOR_0349,
    imx398mipiraw_COLOR_0350, imx398mipiraw_COLOR_0351, imx398mipiraw_COLOR_0352, imx398mipiraw_COLOR_0353, imx398mipiraw_COLOR_0354, imx398mipiraw_COLOR_0355, imx398mipiraw_COLOR_0356, imx398mipiraw_COLOR_0357, imx398mipiraw_COLOR_0358, imx398mipiraw_COLOR_0359,
    imx398mipiraw_COLOR_0360, imx398mipiraw_COLOR_0361, imx398mipiraw_COLOR_0362, imx398mipiraw_COLOR_0363, imx398mipiraw_COLOR_0364, imx398mipiraw_COLOR_0365, imx398mipiraw_COLOR_0366, imx398mipiraw_COLOR_0367, imx398mipiraw_COLOR_0368, imx398mipiraw_COLOR_0369,
    imx398mipiraw_COLOR_0370, imx398mipiraw_COLOR_0371, imx398mipiraw_COLOR_0372, imx398mipiraw_COLOR_0373, imx398mipiraw_COLOR_0374, imx398mipiraw_COLOR_0375, imx398mipiraw_COLOR_0376, imx398mipiraw_COLOR_0377, imx398mipiraw_COLOR_0378, imx398mipiraw_COLOR_0379,
    imx398mipiraw_COLOR_0380, imx398mipiraw_COLOR_0381, imx398mipiraw_COLOR_0382, imx398mipiraw_COLOR_0383, imx398mipiraw_COLOR_0384, imx398mipiraw_COLOR_0385, imx398mipiraw_COLOR_0386, imx398mipiraw_COLOR_0387, imx398mipiraw_COLOR_0388, imx398mipiraw_COLOR_0389,
    imx398mipiraw_COLOR_0390, imx398mipiraw_COLOR_0391, imx398mipiraw_COLOR_0392, imx398mipiraw_COLOR_0393, imx398mipiraw_COLOR_0394, imx398mipiraw_COLOR_0395, imx398mipiraw_COLOR_0396, imx398mipiraw_COLOR_0397, imx398mipiraw_COLOR_0398, imx398mipiraw_COLOR_0399,
    imx398mipiraw_COLOR_0400, imx398mipiraw_COLOR_0401, imx398mipiraw_COLOR_0402, imx398mipiraw_COLOR_0403, imx398mipiraw_COLOR_0404, imx398mipiraw_COLOR_0405, imx398mipiraw_COLOR_0406, imx398mipiraw_COLOR_0407, imx398mipiraw_COLOR_0408, imx398mipiraw_COLOR_0409,
    imx398mipiraw_COLOR_0410, imx398mipiraw_COLOR_0411, imx398mipiraw_COLOR_0412, imx398mipiraw_COLOR_0413, imx398mipiraw_COLOR_0414, imx398mipiraw_COLOR_0415, imx398mipiraw_COLOR_0416, imx398mipiraw_COLOR_0417, imx398mipiraw_COLOR_0418, imx398mipiraw_COLOR_0419,
    imx398mipiraw_COLOR_0420, imx398mipiraw_COLOR_0421, imx398mipiraw_COLOR_0422, imx398mipiraw_COLOR_0423, imx398mipiraw_COLOR_0424, imx398mipiraw_COLOR_0425, imx398mipiraw_COLOR_0426, imx398mipiraw_COLOR_0427, imx398mipiraw_COLOR_0428, imx398mipiraw_COLOR_0429,
    imx398mipiraw_COLOR_0430, imx398mipiraw_COLOR_0431, imx398mipiraw_COLOR_0432, imx398mipiraw_COLOR_0433, imx398mipiraw_COLOR_0434, imx398mipiraw_COLOR_0435, imx398mipiraw_COLOR_0436, imx398mipiraw_COLOR_0437, imx398mipiraw_COLOR_0438, imx398mipiraw_COLOR_0439,
    imx398mipiraw_COLOR_0440, imx398mipiraw_COLOR_0441, imx398mipiraw_COLOR_0442, imx398mipiraw_COLOR_0443, imx398mipiraw_COLOR_0444, imx398mipiraw_COLOR_0445, imx398mipiraw_COLOR_0446, imx398mipiraw_COLOR_0447, imx398mipiraw_COLOR_0448, imx398mipiraw_COLOR_0449,
    imx398mipiraw_COLOR_0450, imx398mipiraw_COLOR_0451, imx398mipiraw_COLOR_0452, imx398mipiraw_COLOR_0453, imx398mipiraw_COLOR_0454, imx398mipiraw_COLOR_0455, imx398mipiraw_COLOR_0456, imx398mipiraw_COLOR_0457, imx398mipiraw_COLOR_0458, imx398mipiraw_COLOR_0459,
    imx398mipiraw_COLOR_0460, imx398mipiraw_COLOR_0461, imx398mipiraw_COLOR_0462, imx398mipiraw_COLOR_0463, imx398mipiraw_COLOR_0464, imx398mipiraw_COLOR_0465, imx398mipiraw_COLOR_0466, imx398mipiraw_COLOR_0467, imx398mipiraw_COLOR_0468, imx398mipiraw_COLOR_0469,
    imx398mipiraw_COLOR_0470, imx398mipiraw_COLOR_0471, imx398mipiraw_COLOR_0472, imx398mipiraw_COLOR_0473, imx398mipiraw_COLOR_0474, imx398mipiraw_COLOR_0475, imx398mipiraw_COLOR_0476, imx398mipiraw_COLOR_0477, imx398mipiraw_COLOR_0478, imx398mipiraw_COLOR_0479,
    imx398mipiraw_COLOR_0480, imx398mipiraw_COLOR_0481, imx398mipiraw_COLOR_0482, imx398mipiraw_COLOR_0483, imx398mipiraw_COLOR_0484, imx398mipiraw_COLOR_0485, imx398mipiraw_COLOR_0486, imx398mipiraw_COLOR_0487, imx398mipiraw_COLOR_0488, imx398mipiraw_COLOR_0489,
    imx398mipiraw_COLOR_0490, imx398mipiraw_COLOR_0491, imx398mipiraw_COLOR_0492, imx398mipiraw_COLOR_0493, imx398mipiraw_COLOR_0494, imx398mipiraw_COLOR_0495, imx398mipiraw_COLOR_0496, imx398mipiraw_COLOR_0497, imx398mipiraw_COLOR_0498, imx398mipiraw_COLOR_0499,
    imx398mipiraw_COLOR_0500, imx398mipiraw_COLOR_0501, imx398mipiraw_COLOR_0502, imx398mipiraw_COLOR_0503, imx398mipiraw_COLOR_0504, imx398mipiraw_COLOR_0505, imx398mipiraw_COLOR_0506, imx398mipiraw_COLOR_0507, imx398mipiraw_COLOR_0508, imx398mipiraw_COLOR_0509,
    imx398mipiraw_COLOR_0510, imx398mipiraw_COLOR_0511, imx398mipiraw_COLOR_0512, imx398mipiraw_COLOR_0513, imx398mipiraw_COLOR_0514, imx398mipiraw_COLOR_0515, imx398mipiraw_COLOR_0516, imx398mipiraw_COLOR_0517, imx398mipiraw_COLOR_0518, imx398mipiraw_COLOR_0519,
    imx398mipiraw_COLOR_0520, imx398mipiraw_COLOR_0521, imx398mipiraw_COLOR_0522, imx398mipiraw_COLOR_0523, imx398mipiraw_COLOR_0524, imx398mipiraw_COLOR_0525, imx398mipiraw_COLOR_0526, imx398mipiraw_COLOR_0527, imx398mipiraw_COLOR_0528, imx398mipiraw_COLOR_0529,
    imx398mipiraw_COLOR_0530, imx398mipiraw_COLOR_0531, imx398mipiraw_COLOR_0532, imx398mipiraw_COLOR_0533, imx398mipiraw_COLOR_0534, imx398mipiraw_COLOR_0535, imx398mipiraw_COLOR_0536, imx398mipiraw_COLOR_0537, imx398mipiraw_COLOR_0538, imx398mipiraw_COLOR_0539,
    imx398mipiraw_COLOR_0540, imx398mipiraw_COLOR_0541, imx398mipiraw_COLOR_0542, imx398mipiraw_COLOR_0543, imx398mipiraw_COLOR_0544, imx398mipiraw_COLOR_0545, imx398mipiraw_COLOR_0546, imx398mipiraw_COLOR_0547, imx398mipiraw_COLOR_0548, imx398mipiraw_COLOR_0549,
    imx398mipiraw_COLOR_0550, imx398mipiraw_COLOR_0551, imx398mipiraw_COLOR_0552, imx398mipiraw_COLOR_0553, imx398mipiraw_COLOR_0554, imx398mipiraw_COLOR_0555, imx398mipiraw_COLOR_0556, imx398mipiraw_COLOR_0557, imx398mipiraw_COLOR_0558, imx398mipiraw_COLOR_0559,
    imx398mipiraw_COLOR_0560, imx398mipiraw_COLOR_0561, imx398mipiraw_COLOR_0562, imx398mipiraw_COLOR_0563, imx398mipiraw_COLOR_0564, imx398mipiraw_COLOR_0565, imx398mipiraw_COLOR_0566, imx398mipiraw_COLOR_0567, imx398mipiraw_COLOR_0568, imx398mipiraw_COLOR_0569,
    imx398mipiraw_COLOR_0570, imx398mipiraw_COLOR_0571, imx398mipiraw_COLOR_0572, imx398mipiraw_COLOR_0573, imx398mipiraw_COLOR_0574, imx398mipiraw_COLOR_0575, imx398mipiraw_COLOR_0576, imx398mipiraw_COLOR_0577, imx398mipiraw_COLOR_0578, imx398mipiraw_COLOR_0579,
    imx398mipiraw_COLOR_0580, imx398mipiraw_COLOR_0581, imx398mipiraw_COLOR_0582, imx398mipiraw_COLOR_0583, imx398mipiraw_COLOR_0584, imx398mipiraw_COLOR_0585, imx398mipiraw_COLOR_0586, imx398mipiraw_COLOR_0587, imx398mipiraw_COLOR_0588, imx398mipiraw_COLOR_0589,
    imx398mipiraw_COLOR_0590, imx398mipiraw_COLOR_0591, imx398mipiraw_COLOR_0592, imx398mipiraw_COLOR_0593, imx398mipiraw_COLOR_0594, imx398mipiraw_COLOR_0595, imx398mipiraw_COLOR_0596, imx398mipiraw_COLOR_0597, imx398mipiraw_COLOR_0598, imx398mipiraw_COLOR_0599,
    imx398mipiraw_COLOR_0600, imx398mipiraw_COLOR_0601, imx398mipiraw_COLOR_0602, imx398mipiraw_COLOR_0603, imx398mipiraw_COLOR_0604, imx398mipiraw_COLOR_0605, imx398mipiraw_COLOR_0606, imx398mipiraw_COLOR_0607, imx398mipiraw_COLOR_0608, imx398mipiraw_COLOR_0609,
    imx398mipiraw_COLOR_0610, imx398mipiraw_COLOR_0611, imx398mipiraw_COLOR_0612, imx398mipiraw_COLOR_0613, imx398mipiraw_COLOR_0614, imx398mipiraw_COLOR_0615, imx398mipiraw_COLOR_0616, imx398mipiraw_COLOR_0617, imx398mipiraw_COLOR_0618, imx398mipiraw_COLOR_0619,
    imx398mipiraw_COLOR_0620, imx398mipiraw_COLOR_0621, imx398mipiraw_COLOR_0622, imx398mipiraw_COLOR_0623, imx398mipiraw_COLOR_0624, imx398mipiraw_COLOR_0625, imx398mipiraw_COLOR_0626, imx398mipiraw_COLOR_0627, imx398mipiraw_COLOR_0628, imx398mipiraw_COLOR_0629,
    imx398mipiraw_COLOR_0630, imx398mipiraw_COLOR_0631, imx398mipiraw_COLOR_0632, imx398mipiraw_COLOR_0633, imx398mipiraw_COLOR_0634, imx398mipiraw_COLOR_0635, imx398mipiraw_COLOR_0636, imx398mipiraw_COLOR_0637, imx398mipiraw_COLOR_0638, imx398mipiraw_COLOR_0639,
    imx398mipiraw_COLOR_0640, imx398mipiraw_COLOR_0641, imx398mipiraw_COLOR_0642, imx398mipiraw_COLOR_0643, imx398mipiraw_COLOR_0644, imx398mipiraw_COLOR_0645, imx398mipiraw_COLOR_0646, imx398mipiraw_COLOR_0647, imx398mipiraw_COLOR_0648, imx398mipiraw_COLOR_0649,
    imx398mipiraw_COLOR_0650, imx398mipiraw_COLOR_0651, imx398mipiraw_COLOR_0652, imx398mipiraw_COLOR_0653, imx398mipiraw_COLOR_0654, imx398mipiraw_COLOR_0655, imx398mipiraw_COLOR_0656, imx398mipiraw_COLOR_0657, imx398mipiraw_COLOR_0658, imx398mipiraw_COLOR_0659,
    imx398mipiraw_COLOR_0660, imx398mipiraw_COLOR_0661, imx398mipiraw_COLOR_0662, imx398mipiraw_COLOR_0663, imx398mipiraw_COLOR_0664, imx398mipiraw_COLOR_0665, imx398mipiraw_COLOR_0666, imx398mipiraw_COLOR_0667, imx398mipiraw_COLOR_0668, imx398mipiraw_COLOR_0669,
    imx398mipiraw_COLOR_0670, imx398mipiraw_COLOR_0671, imx398mipiraw_COLOR_0672, imx398mipiraw_COLOR_0673, imx398mipiraw_COLOR_0674, imx398mipiraw_COLOR_0675, imx398mipiraw_COLOR_0676, imx398mipiraw_COLOR_0677, imx398mipiraw_COLOR_0678, imx398mipiraw_COLOR_0679,
    imx398mipiraw_COLOR_0680, imx398mipiraw_COLOR_0681, imx398mipiraw_COLOR_0682, imx398mipiraw_COLOR_0683, imx398mipiraw_COLOR_0684, imx398mipiraw_COLOR_0685, imx398mipiraw_COLOR_0686, imx398mipiraw_COLOR_0687, imx398mipiraw_COLOR_0688, imx398mipiraw_COLOR_0689,
    imx398mipiraw_COLOR_0690, imx398mipiraw_COLOR_0691, imx398mipiraw_COLOR_0692, imx398mipiraw_COLOR_0693, imx398mipiraw_COLOR_0694, imx398mipiraw_COLOR_0695, imx398mipiraw_COLOR_0696, imx398mipiraw_COLOR_0697, imx398mipiraw_COLOR_0698, imx398mipiraw_COLOR_0699,
    imx398mipiraw_COLOR_0700, imx398mipiraw_COLOR_0701, imx398mipiraw_COLOR_0702, imx398mipiraw_COLOR_0703, imx398mipiraw_COLOR_0704, imx398mipiraw_COLOR_0705, imx398mipiraw_COLOR_0706, imx398mipiraw_COLOR_0707, imx398mipiraw_COLOR_0708, imx398mipiraw_COLOR_0709,
    imx398mipiraw_COLOR_0710, imx398mipiraw_COLOR_0711, imx398mipiraw_COLOR_0712, imx398mipiraw_COLOR_0713, imx398mipiraw_COLOR_0714, imx398mipiraw_COLOR_0715, imx398mipiraw_COLOR_0716, imx398mipiraw_COLOR_0717, imx398mipiraw_COLOR_0718, imx398mipiraw_COLOR_0719,
    imx398mipiraw_COLOR_0720, imx398mipiraw_COLOR_0721, imx398mipiraw_COLOR_0722, imx398mipiraw_COLOR_0723, imx398mipiraw_COLOR_0724, imx398mipiraw_COLOR_0725, imx398mipiraw_COLOR_0726, imx398mipiraw_COLOR_0727, imx398mipiraw_COLOR_0728, imx398mipiraw_COLOR_0729,
    imx398mipiraw_COLOR_0730, imx398mipiraw_COLOR_0731, imx398mipiraw_COLOR_0732, imx398mipiraw_COLOR_0733, imx398mipiraw_COLOR_0734, imx398mipiraw_COLOR_0735, imx398mipiraw_COLOR_0736, imx398mipiraw_COLOR_0737, imx398mipiraw_COLOR_0738, imx398mipiraw_COLOR_0739,
    imx398mipiraw_COLOR_0740, imx398mipiraw_COLOR_0741, imx398mipiraw_COLOR_0742, imx398mipiraw_COLOR_0743, imx398mipiraw_COLOR_0744, imx398mipiraw_COLOR_0745, imx398mipiraw_COLOR_0746, imx398mipiraw_COLOR_0747, imx398mipiraw_COLOR_0748, imx398mipiraw_COLOR_0749,
    imx398mipiraw_COLOR_0750, imx398mipiraw_COLOR_0751, imx398mipiraw_COLOR_0752, imx398mipiraw_COLOR_0753, imx398mipiraw_COLOR_0754, imx398mipiraw_COLOR_0755, imx398mipiraw_COLOR_0756, imx398mipiraw_COLOR_0757, imx398mipiraw_COLOR_0758, imx398mipiraw_COLOR_0759,
    imx398mipiraw_COLOR_0760, imx398mipiraw_COLOR_0761, imx398mipiraw_COLOR_0762, imx398mipiraw_COLOR_0763, imx398mipiraw_COLOR_0764, imx398mipiraw_COLOR_0765, imx398mipiraw_COLOR_0766, imx398mipiraw_COLOR_0767, imx398mipiraw_COLOR_0768, imx398mipiraw_COLOR_0769,
    imx398mipiraw_COLOR_0770, imx398mipiraw_COLOR_0771, imx398mipiraw_COLOR_0772, imx398mipiraw_COLOR_0773, imx398mipiraw_COLOR_0774, imx398mipiraw_COLOR_0775, imx398mipiraw_COLOR_0776, imx398mipiraw_COLOR_0777, imx398mipiraw_COLOR_0778, imx398mipiraw_COLOR_0779,
    imx398mipiraw_COLOR_0780, imx398mipiraw_COLOR_0781, imx398mipiraw_COLOR_0782, imx398mipiraw_COLOR_0783, imx398mipiraw_COLOR_0784, imx398mipiraw_COLOR_0785, imx398mipiraw_COLOR_0786, imx398mipiraw_COLOR_0787, imx398mipiraw_COLOR_0788, imx398mipiraw_COLOR_0789,
    imx398mipiraw_COLOR_0790, imx398mipiraw_COLOR_0791, imx398mipiraw_COLOR_0792, imx398mipiraw_COLOR_0793, imx398mipiraw_COLOR_0794, imx398mipiraw_COLOR_0795, imx398mipiraw_COLOR_0796, imx398mipiraw_COLOR_0797, imx398mipiraw_COLOR_0798, imx398mipiraw_COLOR_0799,
    imx398mipiraw_COLOR_0800, imx398mipiraw_COLOR_0801, imx398mipiraw_COLOR_0802, imx398mipiraw_COLOR_0803, imx398mipiraw_COLOR_0804, imx398mipiraw_COLOR_0805, imx398mipiraw_COLOR_0806, imx398mipiraw_COLOR_0807, imx398mipiraw_COLOR_0808, imx398mipiraw_COLOR_0809,
    imx398mipiraw_COLOR_0810, imx398mipiraw_COLOR_0811, imx398mipiraw_COLOR_0812, imx398mipiraw_COLOR_0813, imx398mipiraw_COLOR_0814, imx398mipiraw_COLOR_0815, imx398mipiraw_COLOR_0816, imx398mipiraw_COLOR_0817, imx398mipiraw_COLOR_0818, imx398mipiraw_COLOR_0819,
    imx398mipiraw_COLOR_0820, imx398mipiraw_COLOR_0821, imx398mipiraw_COLOR_0822, imx398mipiraw_COLOR_0823, imx398mipiraw_COLOR_0824, imx398mipiraw_COLOR_0825, imx398mipiraw_COLOR_0826, imx398mipiraw_COLOR_0827, imx398mipiraw_COLOR_0828, imx398mipiraw_COLOR_0829,
    imx398mipiraw_COLOR_0830, imx398mipiraw_COLOR_0831, imx398mipiraw_COLOR_0832, imx398mipiraw_COLOR_0833, imx398mipiraw_COLOR_0834, imx398mipiraw_COLOR_0835, imx398mipiraw_COLOR_0836, imx398mipiraw_COLOR_0837, imx398mipiraw_COLOR_0838, imx398mipiraw_COLOR_0839,
    imx398mipiraw_COLOR_0840, imx398mipiraw_COLOR_0841, imx398mipiraw_COLOR_0842, imx398mipiraw_COLOR_0843, imx398mipiraw_COLOR_0844, imx398mipiraw_COLOR_0845, imx398mipiraw_COLOR_0846, imx398mipiraw_COLOR_0847, imx398mipiraw_COLOR_0848, imx398mipiraw_COLOR_0849,
    imx398mipiraw_COLOR_0850, imx398mipiraw_COLOR_0851, imx398mipiraw_COLOR_0852, imx398mipiraw_COLOR_0853, imx398mipiraw_COLOR_0854, imx398mipiraw_COLOR_0855, imx398mipiraw_COLOR_0856, imx398mipiraw_COLOR_0857, imx398mipiraw_COLOR_0858, imx398mipiraw_COLOR_0859,
    imx398mipiraw_COLOR_0860, imx398mipiraw_COLOR_0861, imx398mipiraw_COLOR_0862, imx398mipiraw_COLOR_0863, imx398mipiraw_COLOR_0864, imx398mipiraw_COLOR_0865, imx398mipiraw_COLOR_0866, imx398mipiraw_COLOR_0867, imx398mipiraw_COLOR_0868, imx398mipiraw_COLOR_0869,
    imx398mipiraw_COLOR_0870, imx398mipiraw_COLOR_0871, imx398mipiraw_COLOR_0872, imx398mipiraw_COLOR_0873, imx398mipiraw_COLOR_0874, imx398mipiraw_COLOR_0875, imx398mipiraw_COLOR_0876, imx398mipiraw_COLOR_0877, imx398mipiraw_COLOR_0878, imx398mipiraw_COLOR_0879,
    imx398mipiraw_COLOR_0880, imx398mipiraw_COLOR_0881, imx398mipiraw_COLOR_0882, imx398mipiraw_COLOR_0883, imx398mipiraw_COLOR_0884, imx398mipiraw_COLOR_0885, imx398mipiraw_COLOR_0886, imx398mipiraw_COLOR_0887, imx398mipiraw_COLOR_0888, imx398mipiraw_COLOR_0889,
    imx398mipiraw_COLOR_0890, imx398mipiraw_COLOR_0891, imx398mipiraw_COLOR_0892, imx398mipiraw_COLOR_0893, imx398mipiraw_COLOR_0894, imx398mipiraw_COLOR_0895, imx398mipiraw_COLOR_0896, imx398mipiraw_COLOR_0897, imx398mipiraw_COLOR_0898, imx398mipiraw_COLOR_0899,
    imx398mipiraw_COLOR_0900, imx398mipiraw_COLOR_0901, imx398mipiraw_COLOR_0902, imx398mipiraw_COLOR_0903, imx398mipiraw_COLOR_0904, imx398mipiraw_COLOR_0905, imx398mipiraw_COLOR_0906, imx398mipiraw_COLOR_0907, imx398mipiraw_COLOR_0908, imx398mipiraw_COLOR_0909,
    imx398mipiraw_COLOR_0910, imx398mipiraw_COLOR_0911, imx398mipiraw_COLOR_0912, imx398mipiraw_COLOR_0913, imx398mipiraw_COLOR_0914, imx398mipiraw_COLOR_0915, imx398mipiraw_COLOR_0916, imx398mipiraw_COLOR_0917, imx398mipiraw_COLOR_0918, imx398mipiraw_COLOR_0919,
    imx398mipiraw_COLOR_0920, imx398mipiraw_COLOR_0921, imx398mipiraw_COLOR_0922, imx398mipiraw_COLOR_0923, imx398mipiraw_COLOR_0924, imx398mipiraw_COLOR_0925, imx398mipiraw_COLOR_0926, imx398mipiraw_COLOR_0927, imx398mipiraw_COLOR_0928, imx398mipiraw_COLOR_0929,
    imx398mipiraw_COLOR_0930, imx398mipiraw_COLOR_0931, imx398mipiraw_COLOR_0932, imx398mipiraw_COLOR_0933, imx398mipiraw_COLOR_0934, imx398mipiraw_COLOR_0935, imx398mipiraw_COLOR_0936, imx398mipiraw_COLOR_0937, imx398mipiraw_COLOR_0938, imx398mipiraw_COLOR_0939,
    imx398mipiraw_COLOR_0940, imx398mipiraw_COLOR_0941, imx398mipiraw_COLOR_0942, imx398mipiraw_COLOR_0943, imx398mipiraw_COLOR_0944, imx398mipiraw_COLOR_0945, imx398mipiraw_COLOR_0946, imx398mipiraw_COLOR_0947, imx398mipiraw_COLOR_0948, imx398mipiraw_COLOR_0949,
    imx398mipiraw_COLOR_0950, imx398mipiraw_COLOR_0951, imx398mipiraw_COLOR_0952, imx398mipiraw_COLOR_0953, imx398mipiraw_COLOR_0954, imx398mipiraw_COLOR_0955, imx398mipiraw_COLOR_0956, imx398mipiraw_COLOR_0957, imx398mipiraw_COLOR_0958, imx398mipiraw_COLOR_0959,
},
.PCA = {
    imx398mipiraw_PCA_0000,
},
.COLOR_PARAM = {
    imx398mipiraw_COLOR_PARAM_0000, imx398mipiraw_COLOR_PARAM_0001, imx398mipiraw_COLOR_PARAM_0002, imx398mipiraw_COLOR_PARAM_0003, imx398mipiraw_COLOR_PARAM_0004, imx398mipiraw_COLOR_PARAM_0005, imx398mipiraw_COLOR_PARAM_0006, imx398mipiraw_COLOR_PARAM_0007, imx398mipiraw_COLOR_PARAM_0008, imx398mipiraw_COLOR_PARAM_0009,
    imx398mipiraw_COLOR_PARAM_0010, imx398mipiraw_COLOR_PARAM_0011, imx398mipiraw_COLOR_PARAM_0012, imx398mipiraw_COLOR_PARAM_0013, imx398mipiraw_COLOR_PARAM_0014, imx398mipiraw_COLOR_PARAM_0015,
},
