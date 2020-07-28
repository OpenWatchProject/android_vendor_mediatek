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
    imx376mipiraw_CCM_0000, imx376mipiraw_CCM_0001, imx376mipiraw_CCM_0002, imx376mipiraw_CCM_0003, imx376mipiraw_CCM_0004, imx376mipiraw_CCM_0005, imx376mipiraw_CCM_0006, imx376mipiraw_CCM_0007, imx376mipiraw_CCM_0008, imx376mipiraw_CCM_0009,
    imx376mipiraw_CCM_0010, imx376mipiraw_CCM_0011, imx376mipiraw_CCM_0012, imx376mipiraw_CCM_0013, imx376mipiraw_CCM_0014, imx376mipiraw_CCM_0015, imx376mipiraw_CCM_0016, imx376mipiraw_CCM_0017, imx376mipiraw_CCM_0018, imx376mipiraw_CCM_0019,
    imx376mipiraw_CCM_0020, imx376mipiraw_CCM_0021, imx376mipiraw_CCM_0022, imx376mipiraw_CCM_0023, imx376mipiraw_CCM_0024, imx376mipiraw_CCM_0025, imx376mipiraw_CCM_0026, imx376mipiraw_CCM_0027, imx376mipiraw_CCM_0028, imx376mipiraw_CCM_0029,
    imx376mipiraw_CCM_0030, imx376mipiraw_CCM_0031, imx376mipiraw_CCM_0032, imx376mipiraw_CCM_0033, imx376mipiraw_CCM_0034, imx376mipiraw_CCM_0035, imx376mipiraw_CCM_0036, imx376mipiraw_CCM_0037, imx376mipiraw_CCM_0038, imx376mipiraw_CCM_0039,
    imx376mipiraw_CCM_0040, imx376mipiraw_CCM_0041, imx376mipiraw_CCM_0042, imx376mipiraw_CCM_0043, imx376mipiraw_CCM_0044, imx376mipiraw_CCM_0045, imx376mipiraw_CCM_0046, imx376mipiraw_CCM_0047, imx376mipiraw_CCM_0048, imx376mipiraw_CCM_0049,
    imx376mipiraw_CCM_0050, imx376mipiraw_CCM_0051, imx376mipiraw_CCM_0052, imx376mipiraw_CCM_0053, imx376mipiraw_CCM_0054, imx376mipiraw_CCM_0055, imx376mipiraw_CCM_0056, imx376mipiraw_CCM_0057, imx376mipiraw_CCM_0058, imx376mipiraw_CCM_0059,
    imx376mipiraw_CCM_0060, imx376mipiraw_CCM_0061, imx376mipiraw_CCM_0062, imx376mipiraw_CCM_0063, imx376mipiraw_CCM_0064, imx376mipiraw_CCM_0065, imx376mipiraw_CCM_0066, imx376mipiraw_CCM_0067, imx376mipiraw_CCM_0068, imx376mipiraw_CCM_0069,
    imx376mipiraw_CCM_0070, imx376mipiraw_CCM_0071, imx376mipiraw_CCM_0072, imx376mipiraw_CCM_0073, imx376mipiraw_CCM_0074, imx376mipiraw_CCM_0075, imx376mipiraw_CCM_0076, imx376mipiraw_CCM_0077, imx376mipiraw_CCM_0078, imx376mipiraw_CCM_0079,
    imx376mipiraw_CCM_0080, imx376mipiraw_CCM_0081, imx376mipiraw_CCM_0082, imx376mipiraw_CCM_0083, imx376mipiraw_CCM_0084, imx376mipiraw_CCM_0085, imx376mipiraw_CCM_0086, imx376mipiraw_CCM_0087, imx376mipiraw_CCM_0088, imx376mipiraw_CCM_0089,
    imx376mipiraw_CCM_0090, imx376mipiraw_CCM_0091, imx376mipiraw_CCM_0092, imx376mipiraw_CCM_0093, imx376mipiraw_CCM_0094, imx376mipiraw_CCM_0095, imx376mipiraw_CCM_0096, imx376mipiraw_CCM_0097, imx376mipiraw_CCM_0098, imx376mipiraw_CCM_0099,
    imx376mipiraw_CCM_0100, imx376mipiraw_CCM_0101, imx376mipiraw_CCM_0102, imx376mipiraw_CCM_0103, imx376mipiraw_CCM_0104, imx376mipiraw_CCM_0105, imx376mipiraw_CCM_0106, imx376mipiraw_CCM_0107, imx376mipiraw_CCM_0108, imx376mipiraw_CCM_0109,
    imx376mipiraw_CCM_0110, imx376mipiraw_CCM_0111, imx376mipiraw_CCM_0112, imx376mipiraw_CCM_0113, imx376mipiraw_CCM_0114, imx376mipiraw_CCM_0115, imx376mipiraw_CCM_0116, imx376mipiraw_CCM_0117, imx376mipiraw_CCM_0118, imx376mipiraw_CCM_0119,
    imx376mipiraw_CCM_0120, imx376mipiraw_CCM_0121, imx376mipiraw_CCM_0122, imx376mipiraw_CCM_0123, imx376mipiraw_CCM_0124, imx376mipiraw_CCM_0125, imx376mipiraw_CCM_0126, imx376mipiraw_CCM_0127, imx376mipiraw_CCM_0128, imx376mipiraw_CCM_0129,
    imx376mipiraw_CCM_0130, imx376mipiraw_CCM_0131, imx376mipiraw_CCM_0132, imx376mipiraw_CCM_0133, imx376mipiraw_CCM_0134, imx376mipiraw_CCM_0135, imx376mipiraw_CCM_0136, imx376mipiraw_CCM_0137, imx376mipiraw_CCM_0138, imx376mipiraw_CCM_0139,
    imx376mipiraw_CCM_0140, imx376mipiraw_CCM_0141, imx376mipiraw_CCM_0142, imx376mipiraw_CCM_0143, imx376mipiraw_CCM_0144, imx376mipiraw_CCM_0145, imx376mipiraw_CCM_0146, imx376mipiraw_CCM_0147, imx376mipiraw_CCM_0148, imx376mipiraw_CCM_0149,
    imx376mipiraw_CCM_0150, imx376mipiraw_CCM_0151, imx376mipiraw_CCM_0152, imx376mipiraw_CCM_0153, imx376mipiraw_CCM_0154, imx376mipiraw_CCM_0155, imx376mipiraw_CCM_0156, imx376mipiraw_CCM_0157, imx376mipiraw_CCM_0158, imx376mipiraw_CCM_0159,
    imx376mipiraw_CCM_0160, imx376mipiraw_CCM_0161, imx376mipiraw_CCM_0162, imx376mipiraw_CCM_0163, imx376mipiraw_CCM_0164, imx376mipiraw_CCM_0165, imx376mipiraw_CCM_0166, imx376mipiraw_CCM_0167, imx376mipiraw_CCM_0168, imx376mipiraw_CCM_0169,
    imx376mipiraw_CCM_0170, imx376mipiraw_CCM_0171, imx376mipiraw_CCM_0172, imx376mipiraw_CCM_0173, imx376mipiraw_CCM_0174, imx376mipiraw_CCM_0175, imx376mipiraw_CCM_0176, imx376mipiraw_CCM_0177, imx376mipiraw_CCM_0178, imx376mipiraw_CCM_0179,
    imx376mipiraw_CCM_0180, imx376mipiraw_CCM_0181, imx376mipiraw_CCM_0182, imx376mipiraw_CCM_0183, imx376mipiraw_CCM_0184, imx376mipiraw_CCM_0185, imx376mipiraw_CCM_0186, imx376mipiraw_CCM_0187, imx376mipiraw_CCM_0188, imx376mipiraw_CCM_0189,
    imx376mipiraw_CCM_0190, imx376mipiraw_CCM_0191, imx376mipiraw_CCM_0192, imx376mipiraw_CCM_0193, imx376mipiraw_CCM_0194, imx376mipiraw_CCM_0195, imx376mipiraw_CCM_0196, imx376mipiraw_CCM_0197, imx376mipiraw_CCM_0198, imx376mipiraw_CCM_0199,
    imx376mipiraw_CCM_0200, imx376mipiraw_CCM_0201, imx376mipiraw_CCM_0202, imx376mipiraw_CCM_0203, imx376mipiraw_CCM_0204, imx376mipiraw_CCM_0205, imx376mipiraw_CCM_0206, imx376mipiraw_CCM_0207, imx376mipiraw_CCM_0208, imx376mipiraw_CCM_0209,
    imx376mipiraw_CCM_0210, imx376mipiraw_CCM_0211, imx376mipiraw_CCM_0212, imx376mipiraw_CCM_0213, imx376mipiraw_CCM_0214, imx376mipiraw_CCM_0215, imx376mipiraw_CCM_0216, imx376mipiraw_CCM_0217, imx376mipiraw_CCM_0218, imx376mipiraw_CCM_0219,
    imx376mipiraw_CCM_0220, imx376mipiraw_CCM_0221, imx376mipiraw_CCM_0222, imx376mipiraw_CCM_0223, imx376mipiraw_CCM_0224, imx376mipiraw_CCM_0225, imx376mipiraw_CCM_0226, imx376mipiraw_CCM_0227, imx376mipiraw_CCM_0228, imx376mipiraw_CCM_0229,
    imx376mipiraw_CCM_0230, imx376mipiraw_CCM_0231, imx376mipiraw_CCM_0232, imx376mipiraw_CCM_0233, imx376mipiraw_CCM_0234, imx376mipiraw_CCM_0235, imx376mipiraw_CCM_0236, imx376mipiraw_CCM_0237, imx376mipiraw_CCM_0238, imx376mipiraw_CCM_0239,
},
.COLOR = {
    imx376mipiraw_COLOR_0000, imx376mipiraw_COLOR_0001, imx376mipiraw_COLOR_0002, imx376mipiraw_COLOR_0003, imx376mipiraw_COLOR_0004, imx376mipiraw_COLOR_0005, imx376mipiraw_COLOR_0006, imx376mipiraw_COLOR_0007, imx376mipiraw_COLOR_0008, imx376mipiraw_COLOR_0009,
    imx376mipiraw_COLOR_0010, imx376mipiraw_COLOR_0011, imx376mipiraw_COLOR_0012, imx376mipiraw_COLOR_0013, imx376mipiraw_COLOR_0014, imx376mipiraw_COLOR_0015, imx376mipiraw_COLOR_0016, imx376mipiraw_COLOR_0017, imx376mipiraw_COLOR_0018, imx376mipiraw_COLOR_0019,
    imx376mipiraw_COLOR_0020, imx376mipiraw_COLOR_0021, imx376mipiraw_COLOR_0022, imx376mipiraw_COLOR_0023, imx376mipiraw_COLOR_0024, imx376mipiraw_COLOR_0025, imx376mipiraw_COLOR_0026, imx376mipiraw_COLOR_0027, imx376mipiraw_COLOR_0028, imx376mipiraw_COLOR_0029,
    imx376mipiraw_COLOR_0030, imx376mipiraw_COLOR_0031, imx376mipiraw_COLOR_0032, imx376mipiraw_COLOR_0033, imx376mipiraw_COLOR_0034, imx376mipiraw_COLOR_0035, imx376mipiraw_COLOR_0036, imx376mipiraw_COLOR_0037, imx376mipiraw_COLOR_0038, imx376mipiraw_COLOR_0039,
    imx376mipiraw_COLOR_0040, imx376mipiraw_COLOR_0041, imx376mipiraw_COLOR_0042, imx376mipiraw_COLOR_0043, imx376mipiraw_COLOR_0044, imx376mipiraw_COLOR_0045, imx376mipiraw_COLOR_0046, imx376mipiraw_COLOR_0047, imx376mipiraw_COLOR_0048, imx376mipiraw_COLOR_0049,
    imx376mipiraw_COLOR_0050, imx376mipiraw_COLOR_0051, imx376mipiraw_COLOR_0052, imx376mipiraw_COLOR_0053, imx376mipiraw_COLOR_0054, imx376mipiraw_COLOR_0055, imx376mipiraw_COLOR_0056, imx376mipiraw_COLOR_0057, imx376mipiraw_COLOR_0058, imx376mipiraw_COLOR_0059,
    imx376mipiraw_COLOR_0060, imx376mipiraw_COLOR_0061, imx376mipiraw_COLOR_0062, imx376mipiraw_COLOR_0063, imx376mipiraw_COLOR_0064, imx376mipiraw_COLOR_0065, imx376mipiraw_COLOR_0066, imx376mipiraw_COLOR_0067, imx376mipiraw_COLOR_0068, imx376mipiraw_COLOR_0069,
    imx376mipiraw_COLOR_0070, imx376mipiraw_COLOR_0071, imx376mipiraw_COLOR_0072, imx376mipiraw_COLOR_0073, imx376mipiraw_COLOR_0074, imx376mipiraw_COLOR_0075, imx376mipiraw_COLOR_0076, imx376mipiraw_COLOR_0077, imx376mipiraw_COLOR_0078, imx376mipiraw_COLOR_0079,
    imx376mipiraw_COLOR_0080, imx376mipiraw_COLOR_0081, imx376mipiraw_COLOR_0082, imx376mipiraw_COLOR_0083, imx376mipiraw_COLOR_0084, imx376mipiraw_COLOR_0085, imx376mipiraw_COLOR_0086, imx376mipiraw_COLOR_0087, imx376mipiraw_COLOR_0088, imx376mipiraw_COLOR_0089,
    imx376mipiraw_COLOR_0090, imx376mipiraw_COLOR_0091, imx376mipiraw_COLOR_0092, imx376mipiraw_COLOR_0093, imx376mipiraw_COLOR_0094, imx376mipiraw_COLOR_0095, imx376mipiraw_COLOR_0096, imx376mipiraw_COLOR_0097, imx376mipiraw_COLOR_0098, imx376mipiraw_COLOR_0099,
    imx376mipiraw_COLOR_0100, imx376mipiraw_COLOR_0101, imx376mipiraw_COLOR_0102, imx376mipiraw_COLOR_0103, imx376mipiraw_COLOR_0104, imx376mipiraw_COLOR_0105, imx376mipiraw_COLOR_0106, imx376mipiraw_COLOR_0107, imx376mipiraw_COLOR_0108, imx376mipiraw_COLOR_0109,
    imx376mipiraw_COLOR_0110, imx376mipiraw_COLOR_0111, imx376mipiraw_COLOR_0112, imx376mipiraw_COLOR_0113, imx376mipiraw_COLOR_0114, imx376mipiraw_COLOR_0115, imx376mipiraw_COLOR_0116, imx376mipiraw_COLOR_0117, imx376mipiraw_COLOR_0118, imx376mipiraw_COLOR_0119,
    imx376mipiraw_COLOR_0120, imx376mipiraw_COLOR_0121, imx376mipiraw_COLOR_0122, imx376mipiraw_COLOR_0123, imx376mipiraw_COLOR_0124, imx376mipiraw_COLOR_0125, imx376mipiraw_COLOR_0126, imx376mipiraw_COLOR_0127, imx376mipiraw_COLOR_0128, imx376mipiraw_COLOR_0129,
    imx376mipiraw_COLOR_0130, imx376mipiraw_COLOR_0131, imx376mipiraw_COLOR_0132, imx376mipiraw_COLOR_0133, imx376mipiraw_COLOR_0134, imx376mipiraw_COLOR_0135, imx376mipiraw_COLOR_0136, imx376mipiraw_COLOR_0137, imx376mipiraw_COLOR_0138, imx376mipiraw_COLOR_0139,
    imx376mipiraw_COLOR_0140, imx376mipiraw_COLOR_0141, imx376mipiraw_COLOR_0142, imx376mipiraw_COLOR_0143, imx376mipiraw_COLOR_0144, imx376mipiraw_COLOR_0145, imx376mipiraw_COLOR_0146, imx376mipiraw_COLOR_0147, imx376mipiraw_COLOR_0148, imx376mipiraw_COLOR_0149,
    imx376mipiraw_COLOR_0150, imx376mipiraw_COLOR_0151, imx376mipiraw_COLOR_0152, imx376mipiraw_COLOR_0153, imx376mipiraw_COLOR_0154, imx376mipiraw_COLOR_0155, imx376mipiraw_COLOR_0156, imx376mipiraw_COLOR_0157, imx376mipiraw_COLOR_0158, imx376mipiraw_COLOR_0159,
    imx376mipiraw_COLOR_0160, imx376mipiraw_COLOR_0161, imx376mipiraw_COLOR_0162, imx376mipiraw_COLOR_0163, imx376mipiraw_COLOR_0164, imx376mipiraw_COLOR_0165, imx376mipiraw_COLOR_0166, imx376mipiraw_COLOR_0167, imx376mipiraw_COLOR_0168, imx376mipiraw_COLOR_0169,
    imx376mipiraw_COLOR_0170, imx376mipiraw_COLOR_0171, imx376mipiraw_COLOR_0172, imx376mipiraw_COLOR_0173, imx376mipiraw_COLOR_0174, imx376mipiraw_COLOR_0175, imx376mipiraw_COLOR_0176, imx376mipiraw_COLOR_0177, imx376mipiraw_COLOR_0178, imx376mipiraw_COLOR_0179,
    imx376mipiraw_COLOR_0180, imx376mipiraw_COLOR_0181, imx376mipiraw_COLOR_0182, imx376mipiraw_COLOR_0183, imx376mipiraw_COLOR_0184, imx376mipiraw_COLOR_0185, imx376mipiraw_COLOR_0186, imx376mipiraw_COLOR_0187, imx376mipiraw_COLOR_0188, imx376mipiraw_COLOR_0189,
    imx376mipiraw_COLOR_0190, imx376mipiraw_COLOR_0191, imx376mipiraw_COLOR_0192, imx376mipiraw_COLOR_0193, imx376mipiraw_COLOR_0194, imx376mipiraw_COLOR_0195, imx376mipiraw_COLOR_0196, imx376mipiraw_COLOR_0197, imx376mipiraw_COLOR_0198, imx376mipiraw_COLOR_0199,
    imx376mipiraw_COLOR_0200, imx376mipiraw_COLOR_0201, imx376mipiraw_COLOR_0202, imx376mipiraw_COLOR_0203, imx376mipiraw_COLOR_0204, imx376mipiraw_COLOR_0205, imx376mipiraw_COLOR_0206, imx376mipiraw_COLOR_0207, imx376mipiraw_COLOR_0208, imx376mipiraw_COLOR_0209,
    imx376mipiraw_COLOR_0210, imx376mipiraw_COLOR_0211, imx376mipiraw_COLOR_0212, imx376mipiraw_COLOR_0213, imx376mipiraw_COLOR_0214, imx376mipiraw_COLOR_0215, imx376mipiraw_COLOR_0216, imx376mipiraw_COLOR_0217, imx376mipiraw_COLOR_0218, imx376mipiraw_COLOR_0219,
    imx376mipiraw_COLOR_0220, imx376mipiraw_COLOR_0221, imx376mipiraw_COLOR_0222, imx376mipiraw_COLOR_0223, imx376mipiraw_COLOR_0224, imx376mipiraw_COLOR_0225, imx376mipiraw_COLOR_0226, imx376mipiraw_COLOR_0227, imx376mipiraw_COLOR_0228, imx376mipiraw_COLOR_0229,
    imx376mipiraw_COLOR_0230, imx376mipiraw_COLOR_0231, imx376mipiraw_COLOR_0232, imx376mipiraw_COLOR_0233, imx376mipiraw_COLOR_0234, imx376mipiraw_COLOR_0235, imx376mipiraw_COLOR_0236, imx376mipiraw_COLOR_0237, imx376mipiraw_COLOR_0238, imx376mipiraw_COLOR_0239,
    imx376mipiraw_COLOR_0240, imx376mipiraw_COLOR_0241, imx376mipiraw_COLOR_0242, imx376mipiraw_COLOR_0243, imx376mipiraw_COLOR_0244, imx376mipiraw_COLOR_0245, imx376mipiraw_COLOR_0246, imx376mipiraw_COLOR_0247, imx376mipiraw_COLOR_0248, imx376mipiraw_COLOR_0249,
    imx376mipiraw_COLOR_0250, imx376mipiraw_COLOR_0251, imx376mipiraw_COLOR_0252, imx376mipiraw_COLOR_0253, imx376mipiraw_COLOR_0254, imx376mipiraw_COLOR_0255, imx376mipiraw_COLOR_0256, imx376mipiraw_COLOR_0257, imx376mipiraw_COLOR_0258, imx376mipiraw_COLOR_0259,
    imx376mipiraw_COLOR_0260, imx376mipiraw_COLOR_0261, imx376mipiraw_COLOR_0262, imx376mipiraw_COLOR_0263, imx376mipiraw_COLOR_0264, imx376mipiraw_COLOR_0265, imx376mipiraw_COLOR_0266, imx376mipiraw_COLOR_0267, imx376mipiraw_COLOR_0268, imx376mipiraw_COLOR_0269,
    imx376mipiraw_COLOR_0270, imx376mipiraw_COLOR_0271, imx376mipiraw_COLOR_0272, imx376mipiraw_COLOR_0273, imx376mipiraw_COLOR_0274, imx376mipiraw_COLOR_0275, imx376mipiraw_COLOR_0276, imx376mipiraw_COLOR_0277, imx376mipiraw_COLOR_0278, imx376mipiraw_COLOR_0279,
    imx376mipiraw_COLOR_0280, imx376mipiraw_COLOR_0281, imx376mipiraw_COLOR_0282, imx376mipiraw_COLOR_0283, imx376mipiraw_COLOR_0284, imx376mipiraw_COLOR_0285, imx376mipiraw_COLOR_0286, imx376mipiraw_COLOR_0287, imx376mipiraw_COLOR_0288, imx376mipiraw_COLOR_0289,
    imx376mipiraw_COLOR_0290, imx376mipiraw_COLOR_0291, imx376mipiraw_COLOR_0292, imx376mipiraw_COLOR_0293, imx376mipiraw_COLOR_0294, imx376mipiraw_COLOR_0295, imx376mipiraw_COLOR_0296, imx376mipiraw_COLOR_0297, imx376mipiraw_COLOR_0298, imx376mipiraw_COLOR_0299,
    imx376mipiraw_COLOR_0300, imx376mipiraw_COLOR_0301, imx376mipiraw_COLOR_0302, imx376mipiraw_COLOR_0303, imx376mipiraw_COLOR_0304, imx376mipiraw_COLOR_0305, imx376mipiraw_COLOR_0306, imx376mipiraw_COLOR_0307, imx376mipiraw_COLOR_0308, imx376mipiraw_COLOR_0309,
    imx376mipiraw_COLOR_0310, imx376mipiraw_COLOR_0311, imx376mipiraw_COLOR_0312, imx376mipiraw_COLOR_0313, imx376mipiraw_COLOR_0314, imx376mipiraw_COLOR_0315, imx376mipiraw_COLOR_0316, imx376mipiraw_COLOR_0317, imx376mipiraw_COLOR_0318, imx376mipiraw_COLOR_0319,
    imx376mipiraw_COLOR_0320, imx376mipiraw_COLOR_0321, imx376mipiraw_COLOR_0322, imx376mipiraw_COLOR_0323, imx376mipiraw_COLOR_0324, imx376mipiraw_COLOR_0325, imx376mipiraw_COLOR_0326, imx376mipiraw_COLOR_0327, imx376mipiraw_COLOR_0328, imx376mipiraw_COLOR_0329,
    imx376mipiraw_COLOR_0330, imx376mipiraw_COLOR_0331, imx376mipiraw_COLOR_0332, imx376mipiraw_COLOR_0333, imx376mipiraw_COLOR_0334, imx376mipiraw_COLOR_0335, imx376mipiraw_COLOR_0336, imx376mipiraw_COLOR_0337, imx376mipiraw_COLOR_0338, imx376mipiraw_COLOR_0339,
    imx376mipiraw_COLOR_0340, imx376mipiraw_COLOR_0341, imx376mipiraw_COLOR_0342, imx376mipiraw_COLOR_0343, imx376mipiraw_COLOR_0344, imx376mipiraw_COLOR_0345, imx376mipiraw_COLOR_0346, imx376mipiraw_COLOR_0347, imx376mipiraw_COLOR_0348, imx376mipiraw_COLOR_0349,
    imx376mipiraw_COLOR_0350, imx376mipiraw_COLOR_0351, imx376mipiraw_COLOR_0352, imx376mipiraw_COLOR_0353, imx376mipiraw_COLOR_0354, imx376mipiraw_COLOR_0355, imx376mipiraw_COLOR_0356, imx376mipiraw_COLOR_0357, imx376mipiraw_COLOR_0358, imx376mipiraw_COLOR_0359,
    imx376mipiraw_COLOR_0360, imx376mipiraw_COLOR_0361, imx376mipiraw_COLOR_0362, imx376mipiraw_COLOR_0363, imx376mipiraw_COLOR_0364, imx376mipiraw_COLOR_0365, imx376mipiraw_COLOR_0366, imx376mipiraw_COLOR_0367, imx376mipiraw_COLOR_0368, imx376mipiraw_COLOR_0369,
    imx376mipiraw_COLOR_0370, imx376mipiraw_COLOR_0371, imx376mipiraw_COLOR_0372, imx376mipiraw_COLOR_0373, imx376mipiraw_COLOR_0374, imx376mipiraw_COLOR_0375, imx376mipiraw_COLOR_0376, imx376mipiraw_COLOR_0377, imx376mipiraw_COLOR_0378, imx376mipiraw_COLOR_0379,
    imx376mipiraw_COLOR_0380, imx376mipiraw_COLOR_0381, imx376mipiraw_COLOR_0382, imx376mipiraw_COLOR_0383, imx376mipiraw_COLOR_0384, imx376mipiraw_COLOR_0385, imx376mipiraw_COLOR_0386, imx376mipiraw_COLOR_0387, imx376mipiraw_COLOR_0388, imx376mipiraw_COLOR_0389,
    imx376mipiraw_COLOR_0390, imx376mipiraw_COLOR_0391, imx376mipiraw_COLOR_0392, imx376mipiraw_COLOR_0393, imx376mipiraw_COLOR_0394, imx376mipiraw_COLOR_0395, imx376mipiraw_COLOR_0396, imx376mipiraw_COLOR_0397, imx376mipiraw_COLOR_0398, imx376mipiraw_COLOR_0399,
    imx376mipiraw_COLOR_0400, imx376mipiraw_COLOR_0401, imx376mipiraw_COLOR_0402, imx376mipiraw_COLOR_0403, imx376mipiraw_COLOR_0404, imx376mipiraw_COLOR_0405, imx376mipiraw_COLOR_0406, imx376mipiraw_COLOR_0407, imx376mipiraw_COLOR_0408, imx376mipiraw_COLOR_0409,
    imx376mipiraw_COLOR_0410, imx376mipiraw_COLOR_0411, imx376mipiraw_COLOR_0412, imx376mipiraw_COLOR_0413, imx376mipiraw_COLOR_0414, imx376mipiraw_COLOR_0415, imx376mipiraw_COLOR_0416, imx376mipiraw_COLOR_0417, imx376mipiraw_COLOR_0418, imx376mipiraw_COLOR_0419,
    imx376mipiraw_COLOR_0420, imx376mipiraw_COLOR_0421, imx376mipiraw_COLOR_0422, imx376mipiraw_COLOR_0423, imx376mipiraw_COLOR_0424, imx376mipiraw_COLOR_0425, imx376mipiraw_COLOR_0426, imx376mipiraw_COLOR_0427, imx376mipiraw_COLOR_0428, imx376mipiraw_COLOR_0429,
    imx376mipiraw_COLOR_0430, imx376mipiraw_COLOR_0431, imx376mipiraw_COLOR_0432, imx376mipiraw_COLOR_0433, imx376mipiraw_COLOR_0434, imx376mipiraw_COLOR_0435, imx376mipiraw_COLOR_0436, imx376mipiraw_COLOR_0437, imx376mipiraw_COLOR_0438, imx376mipiraw_COLOR_0439,
    imx376mipiraw_COLOR_0440, imx376mipiraw_COLOR_0441, imx376mipiraw_COLOR_0442, imx376mipiraw_COLOR_0443, imx376mipiraw_COLOR_0444, imx376mipiraw_COLOR_0445, imx376mipiraw_COLOR_0446, imx376mipiraw_COLOR_0447, imx376mipiraw_COLOR_0448, imx376mipiraw_COLOR_0449,
    imx376mipiraw_COLOR_0450, imx376mipiraw_COLOR_0451, imx376mipiraw_COLOR_0452, imx376mipiraw_COLOR_0453, imx376mipiraw_COLOR_0454, imx376mipiraw_COLOR_0455, imx376mipiraw_COLOR_0456, imx376mipiraw_COLOR_0457, imx376mipiraw_COLOR_0458, imx376mipiraw_COLOR_0459,
    imx376mipiraw_COLOR_0460, imx376mipiraw_COLOR_0461, imx376mipiraw_COLOR_0462, imx376mipiraw_COLOR_0463, imx376mipiraw_COLOR_0464, imx376mipiraw_COLOR_0465, imx376mipiraw_COLOR_0466, imx376mipiraw_COLOR_0467, imx376mipiraw_COLOR_0468, imx376mipiraw_COLOR_0469,
    imx376mipiraw_COLOR_0470, imx376mipiraw_COLOR_0471, imx376mipiraw_COLOR_0472, imx376mipiraw_COLOR_0473, imx376mipiraw_COLOR_0474, imx376mipiraw_COLOR_0475, imx376mipiraw_COLOR_0476, imx376mipiraw_COLOR_0477, imx376mipiraw_COLOR_0478, imx376mipiraw_COLOR_0479,
    imx376mipiraw_COLOR_0480, imx376mipiraw_COLOR_0481, imx376mipiraw_COLOR_0482, imx376mipiraw_COLOR_0483, imx376mipiraw_COLOR_0484, imx376mipiraw_COLOR_0485, imx376mipiraw_COLOR_0486, imx376mipiraw_COLOR_0487, imx376mipiraw_COLOR_0488, imx376mipiraw_COLOR_0489,
    imx376mipiraw_COLOR_0490, imx376mipiraw_COLOR_0491, imx376mipiraw_COLOR_0492, imx376mipiraw_COLOR_0493, imx376mipiraw_COLOR_0494, imx376mipiraw_COLOR_0495, imx376mipiraw_COLOR_0496, imx376mipiraw_COLOR_0497, imx376mipiraw_COLOR_0498, imx376mipiraw_COLOR_0499,
    imx376mipiraw_COLOR_0500, imx376mipiraw_COLOR_0501, imx376mipiraw_COLOR_0502, imx376mipiraw_COLOR_0503, imx376mipiraw_COLOR_0504, imx376mipiraw_COLOR_0505, imx376mipiraw_COLOR_0506, imx376mipiraw_COLOR_0507, imx376mipiraw_COLOR_0508, imx376mipiraw_COLOR_0509,
    imx376mipiraw_COLOR_0510, imx376mipiraw_COLOR_0511, imx376mipiraw_COLOR_0512, imx376mipiraw_COLOR_0513, imx376mipiraw_COLOR_0514, imx376mipiraw_COLOR_0515, imx376mipiraw_COLOR_0516, imx376mipiraw_COLOR_0517, imx376mipiraw_COLOR_0518, imx376mipiraw_COLOR_0519,
    imx376mipiraw_COLOR_0520, imx376mipiraw_COLOR_0521, imx376mipiraw_COLOR_0522, imx376mipiraw_COLOR_0523, imx376mipiraw_COLOR_0524, imx376mipiraw_COLOR_0525, imx376mipiraw_COLOR_0526, imx376mipiraw_COLOR_0527, imx376mipiraw_COLOR_0528, imx376mipiraw_COLOR_0529,
    imx376mipiraw_COLOR_0530, imx376mipiraw_COLOR_0531, imx376mipiraw_COLOR_0532, imx376mipiraw_COLOR_0533, imx376mipiraw_COLOR_0534, imx376mipiraw_COLOR_0535, imx376mipiraw_COLOR_0536, imx376mipiraw_COLOR_0537, imx376mipiraw_COLOR_0538, imx376mipiraw_COLOR_0539,
    imx376mipiraw_COLOR_0540, imx376mipiraw_COLOR_0541, imx376mipiraw_COLOR_0542, imx376mipiraw_COLOR_0543, imx376mipiraw_COLOR_0544, imx376mipiraw_COLOR_0545, imx376mipiraw_COLOR_0546, imx376mipiraw_COLOR_0547, imx376mipiraw_COLOR_0548, imx376mipiraw_COLOR_0549,
    imx376mipiraw_COLOR_0550, imx376mipiraw_COLOR_0551, imx376mipiraw_COLOR_0552, imx376mipiraw_COLOR_0553, imx376mipiraw_COLOR_0554, imx376mipiraw_COLOR_0555, imx376mipiraw_COLOR_0556, imx376mipiraw_COLOR_0557, imx376mipiraw_COLOR_0558, imx376mipiraw_COLOR_0559,
    imx376mipiraw_COLOR_0560, imx376mipiraw_COLOR_0561, imx376mipiraw_COLOR_0562, imx376mipiraw_COLOR_0563, imx376mipiraw_COLOR_0564, imx376mipiraw_COLOR_0565, imx376mipiraw_COLOR_0566, imx376mipiraw_COLOR_0567, imx376mipiraw_COLOR_0568, imx376mipiraw_COLOR_0569,
    imx376mipiraw_COLOR_0570, imx376mipiraw_COLOR_0571, imx376mipiraw_COLOR_0572, imx376mipiraw_COLOR_0573, imx376mipiraw_COLOR_0574, imx376mipiraw_COLOR_0575, imx376mipiraw_COLOR_0576, imx376mipiraw_COLOR_0577, imx376mipiraw_COLOR_0578, imx376mipiraw_COLOR_0579,
    imx376mipiraw_COLOR_0580, imx376mipiraw_COLOR_0581, imx376mipiraw_COLOR_0582, imx376mipiraw_COLOR_0583, imx376mipiraw_COLOR_0584, imx376mipiraw_COLOR_0585, imx376mipiraw_COLOR_0586, imx376mipiraw_COLOR_0587, imx376mipiraw_COLOR_0588, imx376mipiraw_COLOR_0589,
    imx376mipiraw_COLOR_0590, imx376mipiraw_COLOR_0591, imx376mipiraw_COLOR_0592, imx376mipiraw_COLOR_0593, imx376mipiraw_COLOR_0594, imx376mipiraw_COLOR_0595, imx376mipiraw_COLOR_0596, imx376mipiraw_COLOR_0597, imx376mipiraw_COLOR_0598, imx376mipiraw_COLOR_0599,
    imx376mipiraw_COLOR_0600, imx376mipiraw_COLOR_0601, imx376mipiraw_COLOR_0602, imx376mipiraw_COLOR_0603, imx376mipiraw_COLOR_0604, imx376mipiraw_COLOR_0605, imx376mipiraw_COLOR_0606, imx376mipiraw_COLOR_0607, imx376mipiraw_COLOR_0608, imx376mipiraw_COLOR_0609,
    imx376mipiraw_COLOR_0610, imx376mipiraw_COLOR_0611, imx376mipiraw_COLOR_0612, imx376mipiraw_COLOR_0613, imx376mipiraw_COLOR_0614, imx376mipiraw_COLOR_0615, imx376mipiraw_COLOR_0616, imx376mipiraw_COLOR_0617, imx376mipiraw_COLOR_0618, imx376mipiraw_COLOR_0619,
    imx376mipiraw_COLOR_0620, imx376mipiraw_COLOR_0621, imx376mipiraw_COLOR_0622, imx376mipiraw_COLOR_0623, imx376mipiraw_COLOR_0624, imx376mipiraw_COLOR_0625, imx376mipiraw_COLOR_0626, imx376mipiraw_COLOR_0627, imx376mipiraw_COLOR_0628, imx376mipiraw_COLOR_0629,
    imx376mipiraw_COLOR_0630, imx376mipiraw_COLOR_0631, imx376mipiraw_COLOR_0632, imx376mipiraw_COLOR_0633, imx376mipiraw_COLOR_0634, imx376mipiraw_COLOR_0635, imx376mipiraw_COLOR_0636, imx376mipiraw_COLOR_0637, imx376mipiraw_COLOR_0638, imx376mipiraw_COLOR_0639,
    imx376mipiraw_COLOR_0640, imx376mipiraw_COLOR_0641, imx376mipiraw_COLOR_0642, imx376mipiraw_COLOR_0643, imx376mipiraw_COLOR_0644, imx376mipiraw_COLOR_0645, imx376mipiraw_COLOR_0646, imx376mipiraw_COLOR_0647, imx376mipiraw_COLOR_0648, imx376mipiraw_COLOR_0649,
    imx376mipiraw_COLOR_0650, imx376mipiraw_COLOR_0651, imx376mipiraw_COLOR_0652, imx376mipiraw_COLOR_0653, imx376mipiraw_COLOR_0654, imx376mipiraw_COLOR_0655, imx376mipiraw_COLOR_0656, imx376mipiraw_COLOR_0657, imx376mipiraw_COLOR_0658, imx376mipiraw_COLOR_0659,
    imx376mipiraw_COLOR_0660, imx376mipiraw_COLOR_0661, imx376mipiraw_COLOR_0662, imx376mipiraw_COLOR_0663, imx376mipiraw_COLOR_0664, imx376mipiraw_COLOR_0665, imx376mipiraw_COLOR_0666, imx376mipiraw_COLOR_0667, imx376mipiraw_COLOR_0668, imx376mipiraw_COLOR_0669,
    imx376mipiraw_COLOR_0670, imx376mipiraw_COLOR_0671, imx376mipiraw_COLOR_0672, imx376mipiraw_COLOR_0673, imx376mipiraw_COLOR_0674, imx376mipiraw_COLOR_0675, imx376mipiraw_COLOR_0676, imx376mipiraw_COLOR_0677, imx376mipiraw_COLOR_0678, imx376mipiraw_COLOR_0679,
    imx376mipiraw_COLOR_0680, imx376mipiraw_COLOR_0681, imx376mipiraw_COLOR_0682, imx376mipiraw_COLOR_0683, imx376mipiraw_COLOR_0684, imx376mipiraw_COLOR_0685, imx376mipiraw_COLOR_0686, imx376mipiraw_COLOR_0687, imx376mipiraw_COLOR_0688, imx376mipiraw_COLOR_0689,
    imx376mipiraw_COLOR_0690, imx376mipiraw_COLOR_0691, imx376mipiraw_COLOR_0692, imx376mipiraw_COLOR_0693, imx376mipiraw_COLOR_0694, imx376mipiraw_COLOR_0695, imx376mipiraw_COLOR_0696, imx376mipiraw_COLOR_0697, imx376mipiraw_COLOR_0698, imx376mipiraw_COLOR_0699,
    imx376mipiraw_COLOR_0700, imx376mipiraw_COLOR_0701, imx376mipiraw_COLOR_0702, imx376mipiraw_COLOR_0703, imx376mipiraw_COLOR_0704, imx376mipiraw_COLOR_0705, imx376mipiraw_COLOR_0706, imx376mipiraw_COLOR_0707, imx376mipiraw_COLOR_0708, imx376mipiraw_COLOR_0709,
    imx376mipiraw_COLOR_0710, imx376mipiraw_COLOR_0711, imx376mipiraw_COLOR_0712, imx376mipiraw_COLOR_0713, imx376mipiraw_COLOR_0714, imx376mipiraw_COLOR_0715, imx376mipiraw_COLOR_0716, imx376mipiraw_COLOR_0717, imx376mipiraw_COLOR_0718, imx376mipiraw_COLOR_0719,
    imx376mipiraw_COLOR_0720, imx376mipiraw_COLOR_0721, imx376mipiraw_COLOR_0722, imx376mipiraw_COLOR_0723, imx376mipiraw_COLOR_0724, imx376mipiraw_COLOR_0725, imx376mipiraw_COLOR_0726, imx376mipiraw_COLOR_0727, imx376mipiraw_COLOR_0728, imx376mipiraw_COLOR_0729,
    imx376mipiraw_COLOR_0730, imx376mipiraw_COLOR_0731, imx376mipiraw_COLOR_0732, imx376mipiraw_COLOR_0733, imx376mipiraw_COLOR_0734, imx376mipiraw_COLOR_0735, imx376mipiraw_COLOR_0736, imx376mipiraw_COLOR_0737, imx376mipiraw_COLOR_0738, imx376mipiraw_COLOR_0739,
    imx376mipiraw_COLOR_0740, imx376mipiraw_COLOR_0741, imx376mipiraw_COLOR_0742, imx376mipiraw_COLOR_0743, imx376mipiraw_COLOR_0744, imx376mipiraw_COLOR_0745, imx376mipiraw_COLOR_0746, imx376mipiraw_COLOR_0747, imx376mipiraw_COLOR_0748, imx376mipiraw_COLOR_0749,
    imx376mipiraw_COLOR_0750, imx376mipiraw_COLOR_0751, imx376mipiraw_COLOR_0752, imx376mipiraw_COLOR_0753, imx376mipiraw_COLOR_0754, imx376mipiraw_COLOR_0755, imx376mipiraw_COLOR_0756, imx376mipiraw_COLOR_0757, imx376mipiraw_COLOR_0758, imx376mipiraw_COLOR_0759,
    imx376mipiraw_COLOR_0760, imx376mipiraw_COLOR_0761, imx376mipiraw_COLOR_0762, imx376mipiraw_COLOR_0763, imx376mipiraw_COLOR_0764, imx376mipiraw_COLOR_0765, imx376mipiraw_COLOR_0766, imx376mipiraw_COLOR_0767, imx376mipiraw_COLOR_0768, imx376mipiraw_COLOR_0769,
    imx376mipiraw_COLOR_0770, imx376mipiraw_COLOR_0771, imx376mipiraw_COLOR_0772, imx376mipiraw_COLOR_0773, imx376mipiraw_COLOR_0774, imx376mipiraw_COLOR_0775, imx376mipiraw_COLOR_0776, imx376mipiraw_COLOR_0777, imx376mipiraw_COLOR_0778, imx376mipiraw_COLOR_0779,
    imx376mipiraw_COLOR_0780, imx376mipiraw_COLOR_0781, imx376mipiraw_COLOR_0782, imx376mipiraw_COLOR_0783, imx376mipiraw_COLOR_0784, imx376mipiraw_COLOR_0785, imx376mipiraw_COLOR_0786, imx376mipiraw_COLOR_0787, imx376mipiraw_COLOR_0788, imx376mipiraw_COLOR_0789,
    imx376mipiraw_COLOR_0790, imx376mipiraw_COLOR_0791, imx376mipiraw_COLOR_0792, imx376mipiraw_COLOR_0793, imx376mipiraw_COLOR_0794, imx376mipiraw_COLOR_0795, imx376mipiraw_COLOR_0796, imx376mipiraw_COLOR_0797, imx376mipiraw_COLOR_0798, imx376mipiraw_COLOR_0799,
    imx376mipiraw_COLOR_0800, imx376mipiraw_COLOR_0801, imx376mipiraw_COLOR_0802, imx376mipiraw_COLOR_0803, imx376mipiraw_COLOR_0804, imx376mipiraw_COLOR_0805, imx376mipiraw_COLOR_0806, imx376mipiraw_COLOR_0807, imx376mipiraw_COLOR_0808, imx376mipiraw_COLOR_0809,
    imx376mipiraw_COLOR_0810, imx376mipiraw_COLOR_0811, imx376mipiraw_COLOR_0812, imx376mipiraw_COLOR_0813, imx376mipiraw_COLOR_0814, imx376mipiraw_COLOR_0815, imx376mipiraw_COLOR_0816, imx376mipiraw_COLOR_0817, imx376mipiraw_COLOR_0818, imx376mipiraw_COLOR_0819,
    imx376mipiraw_COLOR_0820, imx376mipiraw_COLOR_0821, imx376mipiraw_COLOR_0822, imx376mipiraw_COLOR_0823, imx376mipiraw_COLOR_0824, imx376mipiraw_COLOR_0825, imx376mipiraw_COLOR_0826, imx376mipiraw_COLOR_0827, imx376mipiraw_COLOR_0828, imx376mipiraw_COLOR_0829,
    imx376mipiraw_COLOR_0830, imx376mipiraw_COLOR_0831, imx376mipiraw_COLOR_0832, imx376mipiraw_COLOR_0833, imx376mipiraw_COLOR_0834, imx376mipiraw_COLOR_0835, imx376mipiraw_COLOR_0836, imx376mipiraw_COLOR_0837, imx376mipiraw_COLOR_0838, imx376mipiraw_COLOR_0839,
    imx376mipiraw_COLOR_0840, imx376mipiraw_COLOR_0841, imx376mipiraw_COLOR_0842, imx376mipiraw_COLOR_0843, imx376mipiraw_COLOR_0844, imx376mipiraw_COLOR_0845, imx376mipiraw_COLOR_0846, imx376mipiraw_COLOR_0847, imx376mipiraw_COLOR_0848, imx376mipiraw_COLOR_0849,
    imx376mipiraw_COLOR_0850, imx376mipiraw_COLOR_0851, imx376mipiraw_COLOR_0852, imx376mipiraw_COLOR_0853, imx376mipiraw_COLOR_0854, imx376mipiraw_COLOR_0855, imx376mipiraw_COLOR_0856, imx376mipiraw_COLOR_0857, imx376mipiraw_COLOR_0858, imx376mipiraw_COLOR_0859,
    imx376mipiraw_COLOR_0860, imx376mipiraw_COLOR_0861, imx376mipiraw_COLOR_0862, imx376mipiraw_COLOR_0863, imx376mipiraw_COLOR_0864, imx376mipiraw_COLOR_0865, imx376mipiraw_COLOR_0866, imx376mipiraw_COLOR_0867, imx376mipiraw_COLOR_0868, imx376mipiraw_COLOR_0869,
    imx376mipiraw_COLOR_0870, imx376mipiraw_COLOR_0871, imx376mipiraw_COLOR_0872, imx376mipiraw_COLOR_0873, imx376mipiraw_COLOR_0874, imx376mipiraw_COLOR_0875, imx376mipiraw_COLOR_0876, imx376mipiraw_COLOR_0877, imx376mipiraw_COLOR_0878, imx376mipiraw_COLOR_0879,
    imx376mipiraw_COLOR_0880, imx376mipiraw_COLOR_0881, imx376mipiraw_COLOR_0882, imx376mipiraw_COLOR_0883, imx376mipiraw_COLOR_0884, imx376mipiraw_COLOR_0885, imx376mipiraw_COLOR_0886, imx376mipiraw_COLOR_0887, imx376mipiraw_COLOR_0888, imx376mipiraw_COLOR_0889,
    imx376mipiraw_COLOR_0890, imx376mipiraw_COLOR_0891, imx376mipiraw_COLOR_0892, imx376mipiraw_COLOR_0893, imx376mipiraw_COLOR_0894, imx376mipiraw_COLOR_0895, imx376mipiraw_COLOR_0896, imx376mipiraw_COLOR_0897, imx376mipiraw_COLOR_0898, imx376mipiraw_COLOR_0899,
    imx376mipiraw_COLOR_0900, imx376mipiraw_COLOR_0901, imx376mipiraw_COLOR_0902, imx376mipiraw_COLOR_0903, imx376mipiraw_COLOR_0904, imx376mipiraw_COLOR_0905, imx376mipiraw_COLOR_0906, imx376mipiraw_COLOR_0907, imx376mipiraw_COLOR_0908, imx376mipiraw_COLOR_0909,
    imx376mipiraw_COLOR_0910, imx376mipiraw_COLOR_0911, imx376mipiraw_COLOR_0912, imx376mipiraw_COLOR_0913, imx376mipiraw_COLOR_0914, imx376mipiraw_COLOR_0915, imx376mipiraw_COLOR_0916, imx376mipiraw_COLOR_0917, imx376mipiraw_COLOR_0918, imx376mipiraw_COLOR_0919,
    imx376mipiraw_COLOR_0920, imx376mipiraw_COLOR_0921, imx376mipiraw_COLOR_0922, imx376mipiraw_COLOR_0923, imx376mipiraw_COLOR_0924, imx376mipiraw_COLOR_0925, imx376mipiraw_COLOR_0926, imx376mipiraw_COLOR_0927, imx376mipiraw_COLOR_0928, imx376mipiraw_COLOR_0929,
    imx376mipiraw_COLOR_0930, imx376mipiraw_COLOR_0931, imx376mipiraw_COLOR_0932, imx376mipiraw_COLOR_0933, imx376mipiraw_COLOR_0934, imx376mipiraw_COLOR_0935, imx376mipiraw_COLOR_0936, imx376mipiraw_COLOR_0937, imx376mipiraw_COLOR_0938, imx376mipiraw_COLOR_0939,
    imx376mipiraw_COLOR_0940, imx376mipiraw_COLOR_0941, imx376mipiraw_COLOR_0942, imx376mipiraw_COLOR_0943, imx376mipiraw_COLOR_0944, imx376mipiraw_COLOR_0945, imx376mipiraw_COLOR_0946, imx376mipiraw_COLOR_0947, imx376mipiraw_COLOR_0948, imx376mipiraw_COLOR_0949,
    imx376mipiraw_COLOR_0950, imx376mipiraw_COLOR_0951, imx376mipiraw_COLOR_0952, imx376mipiraw_COLOR_0953, imx376mipiraw_COLOR_0954, imx376mipiraw_COLOR_0955, imx376mipiraw_COLOR_0956, imx376mipiraw_COLOR_0957, imx376mipiraw_COLOR_0958, imx376mipiraw_COLOR_0959,
},
.PCA = {
    imx376mipiraw_PCA_0000,
},
.COLOR_PARAM = {
    imx376mipiraw_COLOR_PARAM_0000, imx376mipiraw_COLOR_PARAM_0001, imx376mipiraw_COLOR_PARAM_0002, imx376mipiraw_COLOR_PARAM_0003, imx376mipiraw_COLOR_PARAM_0004, imx376mipiraw_COLOR_PARAM_0005, imx376mipiraw_COLOR_PARAM_0006, imx376mipiraw_COLOR_PARAM_0007, imx376mipiraw_COLOR_PARAM_0008, imx376mipiraw_COLOR_PARAM_0009,
    imx376mipiraw_COLOR_PARAM_0010, imx376mipiraw_COLOR_PARAM_0011, imx376mipiraw_COLOR_PARAM_0012, imx376mipiraw_COLOR_PARAM_0013, imx376mipiraw_COLOR_PARAM_0014, imx376mipiraw_COLOR_PARAM_0015,
},
