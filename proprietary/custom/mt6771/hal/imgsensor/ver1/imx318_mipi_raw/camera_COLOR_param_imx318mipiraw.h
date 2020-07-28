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
    imx318mipiraw_CCM_0000, imx318mipiraw_CCM_0001, imx318mipiraw_CCM_0002, imx318mipiraw_CCM_0003, imx318mipiraw_CCM_0004, imx318mipiraw_CCM_0005, imx318mipiraw_CCM_0006, imx318mipiraw_CCM_0007, imx318mipiraw_CCM_0008, imx318mipiraw_CCM_0009,
    imx318mipiraw_CCM_0010, imx318mipiraw_CCM_0011, imx318mipiraw_CCM_0012, imx318mipiraw_CCM_0013, imx318mipiraw_CCM_0014, imx318mipiraw_CCM_0015, imx318mipiraw_CCM_0016, imx318mipiraw_CCM_0017, imx318mipiraw_CCM_0018, imx318mipiraw_CCM_0019,
    imx318mipiraw_CCM_0020, imx318mipiraw_CCM_0021, imx318mipiraw_CCM_0022, imx318mipiraw_CCM_0023, imx318mipiraw_CCM_0024, imx318mipiraw_CCM_0025, imx318mipiraw_CCM_0026, imx318mipiraw_CCM_0027, imx318mipiraw_CCM_0028, imx318mipiraw_CCM_0029,
    imx318mipiraw_CCM_0030, imx318mipiraw_CCM_0031, imx318mipiraw_CCM_0032, imx318mipiraw_CCM_0033, imx318mipiraw_CCM_0034, imx318mipiraw_CCM_0035, imx318mipiraw_CCM_0036, imx318mipiraw_CCM_0037, imx318mipiraw_CCM_0038, imx318mipiraw_CCM_0039,
    imx318mipiraw_CCM_0040, imx318mipiraw_CCM_0041, imx318mipiraw_CCM_0042, imx318mipiraw_CCM_0043, imx318mipiraw_CCM_0044, imx318mipiraw_CCM_0045, imx318mipiraw_CCM_0046, imx318mipiraw_CCM_0047, imx318mipiraw_CCM_0048, imx318mipiraw_CCM_0049,
    imx318mipiraw_CCM_0050, imx318mipiraw_CCM_0051, imx318mipiraw_CCM_0052, imx318mipiraw_CCM_0053, imx318mipiraw_CCM_0054, imx318mipiraw_CCM_0055, imx318mipiraw_CCM_0056, imx318mipiraw_CCM_0057, imx318mipiraw_CCM_0058, imx318mipiraw_CCM_0059,
    imx318mipiraw_CCM_0060, imx318mipiraw_CCM_0061, imx318mipiraw_CCM_0062, imx318mipiraw_CCM_0063, imx318mipiraw_CCM_0064, imx318mipiraw_CCM_0065, imx318mipiraw_CCM_0066, imx318mipiraw_CCM_0067, imx318mipiraw_CCM_0068, imx318mipiraw_CCM_0069,
    imx318mipiraw_CCM_0070, imx318mipiraw_CCM_0071, imx318mipiraw_CCM_0072, imx318mipiraw_CCM_0073, imx318mipiraw_CCM_0074, imx318mipiraw_CCM_0075, imx318mipiraw_CCM_0076, imx318mipiraw_CCM_0077, imx318mipiraw_CCM_0078, imx318mipiraw_CCM_0079,
    imx318mipiraw_CCM_0080, imx318mipiraw_CCM_0081, imx318mipiraw_CCM_0082, imx318mipiraw_CCM_0083, imx318mipiraw_CCM_0084, imx318mipiraw_CCM_0085, imx318mipiraw_CCM_0086, imx318mipiraw_CCM_0087, imx318mipiraw_CCM_0088, imx318mipiraw_CCM_0089,
    imx318mipiraw_CCM_0090, imx318mipiraw_CCM_0091, imx318mipiraw_CCM_0092, imx318mipiraw_CCM_0093, imx318mipiraw_CCM_0094, imx318mipiraw_CCM_0095, imx318mipiraw_CCM_0096, imx318mipiraw_CCM_0097, imx318mipiraw_CCM_0098, imx318mipiraw_CCM_0099,
    imx318mipiraw_CCM_0100, imx318mipiraw_CCM_0101, imx318mipiraw_CCM_0102, imx318mipiraw_CCM_0103, imx318mipiraw_CCM_0104, imx318mipiraw_CCM_0105, imx318mipiraw_CCM_0106, imx318mipiraw_CCM_0107, imx318mipiraw_CCM_0108, imx318mipiraw_CCM_0109,
    imx318mipiraw_CCM_0110, imx318mipiraw_CCM_0111, imx318mipiraw_CCM_0112, imx318mipiraw_CCM_0113, imx318mipiraw_CCM_0114, imx318mipiraw_CCM_0115, imx318mipiraw_CCM_0116, imx318mipiraw_CCM_0117, imx318mipiraw_CCM_0118, imx318mipiraw_CCM_0119,
    imx318mipiraw_CCM_0120, imx318mipiraw_CCM_0121, imx318mipiraw_CCM_0122, imx318mipiraw_CCM_0123, imx318mipiraw_CCM_0124, imx318mipiraw_CCM_0125, imx318mipiraw_CCM_0126, imx318mipiraw_CCM_0127, imx318mipiraw_CCM_0128, imx318mipiraw_CCM_0129,
    imx318mipiraw_CCM_0130, imx318mipiraw_CCM_0131, imx318mipiraw_CCM_0132, imx318mipiraw_CCM_0133, imx318mipiraw_CCM_0134, imx318mipiraw_CCM_0135, imx318mipiraw_CCM_0136, imx318mipiraw_CCM_0137, imx318mipiraw_CCM_0138, imx318mipiraw_CCM_0139,
    imx318mipiraw_CCM_0140, imx318mipiraw_CCM_0141, imx318mipiraw_CCM_0142, imx318mipiraw_CCM_0143, imx318mipiraw_CCM_0144, imx318mipiraw_CCM_0145, imx318mipiraw_CCM_0146, imx318mipiraw_CCM_0147, imx318mipiraw_CCM_0148, imx318mipiraw_CCM_0149,
    imx318mipiraw_CCM_0150, imx318mipiraw_CCM_0151, imx318mipiraw_CCM_0152, imx318mipiraw_CCM_0153, imx318mipiraw_CCM_0154, imx318mipiraw_CCM_0155, imx318mipiraw_CCM_0156, imx318mipiraw_CCM_0157, imx318mipiraw_CCM_0158, imx318mipiraw_CCM_0159,
    imx318mipiraw_CCM_0160, imx318mipiraw_CCM_0161, imx318mipiraw_CCM_0162, imx318mipiraw_CCM_0163, imx318mipiraw_CCM_0164, imx318mipiraw_CCM_0165, imx318mipiraw_CCM_0166, imx318mipiraw_CCM_0167, imx318mipiraw_CCM_0168, imx318mipiraw_CCM_0169,
    imx318mipiraw_CCM_0170, imx318mipiraw_CCM_0171, imx318mipiraw_CCM_0172, imx318mipiraw_CCM_0173, imx318mipiraw_CCM_0174, imx318mipiraw_CCM_0175, imx318mipiraw_CCM_0176, imx318mipiraw_CCM_0177, imx318mipiraw_CCM_0178, imx318mipiraw_CCM_0179,
    imx318mipiraw_CCM_0180, imx318mipiraw_CCM_0181, imx318mipiraw_CCM_0182, imx318mipiraw_CCM_0183, imx318mipiraw_CCM_0184, imx318mipiraw_CCM_0185, imx318mipiraw_CCM_0186, imx318mipiraw_CCM_0187, imx318mipiraw_CCM_0188, imx318mipiraw_CCM_0189,
    imx318mipiraw_CCM_0190, imx318mipiraw_CCM_0191, imx318mipiraw_CCM_0192, imx318mipiraw_CCM_0193, imx318mipiraw_CCM_0194, imx318mipiraw_CCM_0195, imx318mipiraw_CCM_0196, imx318mipiraw_CCM_0197, imx318mipiraw_CCM_0198, imx318mipiraw_CCM_0199,
    imx318mipiraw_CCM_0200, imx318mipiraw_CCM_0201, imx318mipiraw_CCM_0202, imx318mipiraw_CCM_0203, imx318mipiraw_CCM_0204, imx318mipiraw_CCM_0205, imx318mipiraw_CCM_0206, imx318mipiraw_CCM_0207, imx318mipiraw_CCM_0208, imx318mipiraw_CCM_0209,
    imx318mipiraw_CCM_0210, imx318mipiraw_CCM_0211, imx318mipiraw_CCM_0212, imx318mipiraw_CCM_0213, imx318mipiraw_CCM_0214, imx318mipiraw_CCM_0215, imx318mipiraw_CCM_0216, imx318mipiraw_CCM_0217, imx318mipiraw_CCM_0218, imx318mipiraw_CCM_0219,
    imx318mipiraw_CCM_0220, imx318mipiraw_CCM_0221, imx318mipiraw_CCM_0222, imx318mipiraw_CCM_0223, imx318mipiraw_CCM_0224, imx318mipiraw_CCM_0225, imx318mipiraw_CCM_0226, imx318mipiraw_CCM_0227, imx318mipiraw_CCM_0228, imx318mipiraw_CCM_0229,
    imx318mipiraw_CCM_0230, imx318mipiraw_CCM_0231, imx318mipiraw_CCM_0232, imx318mipiraw_CCM_0233, imx318mipiraw_CCM_0234, imx318mipiraw_CCM_0235, imx318mipiraw_CCM_0236, imx318mipiraw_CCM_0237, imx318mipiraw_CCM_0238, imx318mipiraw_CCM_0239,
},
.COLOR = {
    imx318mipiraw_COLOR_0000, imx318mipiraw_COLOR_0001, imx318mipiraw_COLOR_0002, imx318mipiraw_COLOR_0003, imx318mipiraw_COLOR_0004, imx318mipiraw_COLOR_0005, imx318mipiraw_COLOR_0006, imx318mipiraw_COLOR_0007, imx318mipiraw_COLOR_0008, imx318mipiraw_COLOR_0009,
    imx318mipiraw_COLOR_0010, imx318mipiraw_COLOR_0011, imx318mipiraw_COLOR_0012, imx318mipiraw_COLOR_0013, imx318mipiraw_COLOR_0014, imx318mipiraw_COLOR_0015, imx318mipiraw_COLOR_0016, imx318mipiraw_COLOR_0017, imx318mipiraw_COLOR_0018, imx318mipiraw_COLOR_0019,
    imx318mipiraw_COLOR_0020, imx318mipiraw_COLOR_0021, imx318mipiraw_COLOR_0022, imx318mipiraw_COLOR_0023, imx318mipiraw_COLOR_0024, imx318mipiraw_COLOR_0025, imx318mipiraw_COLOR_0026, imx318mipiraw_COLOR_0027, imx318mipiraw_COLOR_0028, imx318mipiraw_COLOR_0029,
    imx318mipiraw_COLOR_0030, imx318mipiraw_COLOR_0031, imx318mipiraw_COLOR_0032, imx318mipiraw_COLOR_0033, imx318mipiraw_COLOR_0034, imx318mipiraw_COLOR_0035, imx318mipiraw_COLOR_0036, imx318mipiraw_COLOR_0037, imx318mipiraw_COLOR_0038, imx318mipiraw_COLOR_0039,
    imx318mipiraw_COLOR_0040, imx318mipiraw_COLOR_0041, imx318mipiraw_COLOR_0042, imx318mipiraw_COLOR_0043, imx318mipiraw_COLOR_0044, imx318mipiraw_COLOR_0045, imx318mipiraw_COLOR_0046, imx318mipiraw_COLOR_0047, imx318mipiraw_COLOR_0048, imx318mipiraw_COLOR_0049,
    imx318mipiraw_COLOR_0050, imx318mipiraw_COLOR_0051, imx318mipiraw_COLOR_0052, imx318mipiraw_COLOR_0053, imx318mipiraw_COLOR_0054, imx318mipiraw_COLOR_0055, imx318mipiraw_COLOR_0056, imx318mipiraw_COLOR_0057, imx318mipiraw_COLOR_0058, imx318mipiraw_COLOR_0059,
    imx318mipiraw_COLOR_0060, imx318mipiraw_COLOR_0061, imx318mipiraw_COLOR_0062, imx318mipiraw_COLOR_0063, imx318mipiraw_COLOR_0064, imx318mipiraw_COLOR_0065, imx318mipiraw_COLOR_0066, imx318mipiraw_COLOR_0067, imx318mipiraw_COLOR_0068, imx318mipiraw_COLOR_0069,
    imx318mipiraw_COLOR_0070, imx318mipiraw_COLOR_0071, imx318mipiraw_COLOR_0072, imx318mipiraw_COLOR_0073, imx318mipiraw_COLOR_0074, imx318mipiraw_COLOR_0075, imx318mipiraw_COLOR_0076, imx318mipiraw_COLOR_0077, imx318mipiraw_COLOR_0078, imx318mipiraw_COLOR_0079,
    imx318mipiraw_COLOR_0080, imx318mipiraw_COLOR_0081, imx318mipiraw_COLOR_0082, imx318mipiraw_COLOR_0083, imx318mipiraw_COLOR_0084, imx318mipiraw_COLOR_0085, imx318mipiraw_COLOR_0086, imx318mipiraw_COLOR_0087, imx318mipiraw_COLOR_0088, imx318mipiraw_COLOR_0089,
    imx318mipiraw_COLOR_0090, imx318mipiraw_COLOR_0091, imx318mipiraw_COLOR_0092, imx318mipiraw_COLOR_0093, imx318mipiraw_COLOR_0094, imx318mipiraw_COLOR_0095, imx318mipiraw_COLOR_0096, imx318mipiraw_COLOR_0097, imx318mipiraw_COLOR_0098, imx318mipiraw_COLOR_0099,
    imx318mipiraw_COLOR_0100, imx318mipiraw_COLOR_0101, imx318mipiraw_COLOR_0102, imx318mipiraw_COLOR_0103, imx318mipiraw_COLOR_0104, imx318mipiraw_COLOR_0105, imx318mipiraw_COLOR_0106, imx318mipiraw_COLOR_0107, imx318mipiraw_COLOR_0108, imx318mipiraw_COLOR_0109,
    imx318mipiraw_COLOR_0110, imx318mipiraw_COLOR_0111, imx318mipiraw_COLOR_0112, imx318mipiraw_COLOR_0113, imx318mipiraw_COLOR_0114, imx318mipiraw_COLOR_0115, imx318mipiraw_COLOR_0116, imx318mipiraw_COLOR_0117, imx318mipiraw_COLOR_0118, imx318mipiraw_COLOR_0119,
    imx318mipiraw_COLOR_0120, imx318mipiraw_COLOR_0121, imx318mipiraw_COLOR_0122, imx318mipiraw_COLOR_0123, imx318mipiraw_COLOR_0124, imx318mipiraw_COLOR_0125, imx318mipiraw_COLOR_0126, imx318mipiraw_COLOR_0127, imx318mipiraw_COLOR_0128, imx318mipiraw_COLOR_0129,
    imx318mipiraw_COLOR_0130, imx318mipiraw_COLOR_0131, imx318mipiraw_COLOR_0132, imx318mipiraw_COLOR_0133, imx318mipiraw_COLOR_0134, imx318mipiraw_COLOR_0135, imx318mipiraw_COLOR_0136, imx318mipiraw_COLOR_0137, imx318mipiraw_COLOR_0138, imx318mipiraw_COLOR_0139,
    imx318mipiraw_COLOR_0140, imx318mipiraw_COLOR_0141, imx318mipiraw_COLOR_0142, imx318mipiraw_COLOR_0143, imx318mipiraw_COLOR_0144, imx318mipiraw_COLOR_0145, imx318mipiraw_COLOR_0146, imx318mipiraw_COLOR_0147, imx318mipiraw_COLOR_0148, imx318mipiraw_COLOR_0149,
    imx318mipiraw_COLOR_0150, imx318mipiraw_COLOR_0151, imx318mipiraw_COLOR_0152, imx318mipiraw_COLOR_0153, imx318mipiraw_COLOR_0154, imx318mipiraw_COLOR_0155, imx318mipiraw_COLOR_0156, imx318mipiraw_COLOR_0157, imx318mipiraw_COLOR_0158, imx318mipiraw_COLOR_0159,
    imx318mipiraw_COLOR_0160, imx318mipiraw_COLOR_0161, imx318mipiraw_COLOR_0162, imx318mipiraw_COLOR_0163, imx318mipiraw_COLOR_0164, imx318mipiraw_COLOR_0165, imx318mipiraw_COLOR_0166, imx318mipiraw_COLOR_0167, imx318mipiraw_COLOR_0168, imx318mipiraw_COLOR_0169,
    imx318mipiraw_COLOR_0170, imx318mipiraw_COLOR_0171, imx318mipiraw_COLOR_0172, imx318mipiraw_COLOR_0173, imx318mipiraw_COLOR_0174, imx318mipiraw_COLOR_0175, imx318mipiraw_COLOR_0176, imx318mipiraw_COLOR_0177, imx318mipiraw_COLOR_0178, imx318mipiraw_COLOR_0179,
    imx318mipiraw_COLOR_0180, imx318mipiraw_COLOR_0181, imx318mipiraw_COLOR_0182, imx318mipiraw_COLOR_0183, imx318mipiraw_COLOR_0184, imx318mipiraw_COLOR_0185, imx318mipiraw_COLOR_0186, imx318mipiraw_COLOR_0187, imx318mipiraw_COLOR_0188, imx318mipiraw_COLOR_0189,
    imx318mipiraw_COLOR_0190, imx318mipiraw_COLOR_0191, imx318mipiraw_COLOR_0192, imx318mipiraw_COLOR_0193, imx318mipiraw_COLOR_0194, imx318mipiraw_COLOR_0195, imx318mipiraw_COLOR_0196, imx318mipiraw_COLOR_0197, imx318mipiraw_COLOR_0198, imx318mipiraw_COLOR_0199,
    imx318mipiraw_COLOR_0200, imx318mipiraw_COLOR_0201, imx318mipiraw_COLOR_0202, imx318mipiraw_COLOR_0203, imx318mipiraw_COLOR_0204, imx318mipiraw_COLOR_0205, imx318mipiraw_COLOR_0206, imx318mipiraw_COLOR_0207, imx318mipiraw_COLOR_0208, imx318mipiraw_COLOR_0209,
    imx318mipiraw_COLOR_0210, imx318mipiraw_COLOR_0211, imx318mipiraw_COLOR_0212, imx318mipiraw_COLOR_0213, imx318mipiraw_COLOR_0214, imx318mipiraw_COLOR_0215, imx318mipiraw_COLOR_0216, imx318mipiraw_COLOR_0217, imx318mipiraw_COLOR_0218, imx318mipiraw_COLOR_0219,
    imx318mipiraw_COLOR_0220, imx318mipiraw_COLOR_0221, imx318mipiraw_COLOR_0222, imx318mipiraw_COLOR_0223, imx318mipiraw_COLOR_0224, imx318mipiraw_COLOR_0225, imx318mipiraw_COLOR_0226, imx318mipiraw_COLOR_0227, imx318mipiraw_COLOR_0228, imx318mipiraw_COLOR_0229,
    imx318mipiraw_COLOR_0230, imx318mipiraw_COLOR_0231, imx318mipiraw_COLOR_0232, imx318mipiraw_COLOR_0233, imx318mipiraw_COLOR_0234, imx318mipiraw_COLOR_0235, imx318mipiraw_COLOR_0236, imx318mipiraw_COLOR_0237, imx318mipiraw_COLOR_0238, imx318mipiraw_COLOR_0239,
    imx318mipiraw_COLOR_0240, imx318mipiraw_COLOR_0241, imx318mipiraw_COLOR_0242, imx318mipiraw_COLOR_0243, imx318mipiraw_COLOR_0244, imx318mipiraw_COLOR_0245, imx318mipiraw_COLOR_0246, imx318mipiraw_COLOR_0247, imx318mipiraw_COLOR_0248, imx318mipiraw_COLOR_0249,
    imx318mipiraw_COLOR_0250, imx318mipiraw_COLOR_0251, imx318mipiraw_COLOR_0252, imx318mipiraw_COLOR_0253, imx318mipiraw_COLOR_0254, imx318mipiraw_COLOR_0255, imx318mipiraw_COLOR_0256, imx318mipiraw_COLOR_0257, imx318mipiraw_COLOR_0258, imx318mipiraw_COLOR_0259,
    imx318mipiraw_COLOR_0260, imx318mipiraw_COLOR_0261, imx318mipiraw_COLOR_0262, imx318mipiraw_COLOR_0263, imx318mipiraw_COLOR_0264, imx318mipiraw_COLOR_0265, imx318mipiraw_COLOR_0266, imx318mipiraw_COLOR_0267, imx318mipiraw_COLOR_0268, imx318mipiraw_COLOR_0269,
    imx318mipiraw_COLOR_0270, imx318mipiraw_COLOR_0271, imx318mipiraw_COLOR_0272, imx318mipiraw_COLOR_0273, imx318mipiraw_COLOR_0274, imx318mipiraw_COLOR_0275, imx318mipiraw_COLOR_0276, imx318mipiraw_COLOR_0277, imx318mipiraw_COLOR_0278, imx318mipiraw_COLOR_0279,
    imx318mipiraw_COLOR_0280, imx318mipiraw_COLOR_0281, imx318mipiraw_COLOR_0282, imx318mipiraw_COLOR_0283, imx318mipiraw_COLOR_0284, imx318mipiraw_COLOR_0285, imx318mipiraw_COLOR_0286, imx318mipiraw_COLOR_0287, imx318mipiraw_COLOR_0288, imx318mipiraw_COLOR_0289,
    imx318mipiraw_COLOR_0290, imx318mipiraw_COLOR_0291, imx318mipiraw_COLOR_0292, imx318mipiraw_COLOR_0293, imx318mipiraw_COLOR_0294, imx318mipiraw_COLOR_0295, imx318mipiraw_COLOR_0296, imx318mipiraw_COLOR_0297, imx318mipiraw_COLOR_0298, imx318mipiraw_COLOR_0299,
    imx318mipiraw_COLOR_0300, imx318mipiraw_COLOR_0301, imx318mipiraw_COLOR_0302, imx318mipiraw_COLOR_0303, imx318mipiraw_COLOR_0304, imx318mipiraw_COLOR_0305, imx318mipiraw_COLOR_0306, imx318mipiraw_COLOR_0307, imx318mipiraw_COLOR_0308, imx318mipiraw_COLOR_0309,
    imx318mipiraw_COLOR_0310, imx318mipiraw_COLOR_0311, imx318mipiraw_COLOR_0312, imx318mipiraw_COLOR_0313, imx318mipiraw_COLOR_0314, imx318mipiraw_COLOR_0315, imx318mipiraw_COLOR_0316, imx318mipiraw_COLOR_0317, imx318mipiraw_COLOR_0318, imx318mipiraw_COLOR_0319,
    imx318mipiraw_COLOR_0320, imx318mipiraw_COLOR_0321, imx318mipiraw_COLOR_0322, imx318mipiraw_COLOR_0323, imx318mipiraw_COLOR_0324, imx318mipiraw_COLOR_0325, imx318mipiraw_COLOR_0326, imx318mipiraw_COLOR_0327, imx318mipiraw_COLOR_0328, imx318mipiraw_COLOR_0329,
    imx318mipiraw_COLOR_0330, imx318mipiraw_COLOR_0331, imx318mipiraw_COLOR_0332, imx318mipiraw_COLOR_0333, imx318mipiraw_COLOR_0334, imx318mipiraw_COLOR_0335, imx318mipiraw_COLOR_0336, imx318mipiraw_COLOR_0337, imx318mipiraw_COLOR_0338, imx318mipiraw_COLOR_0339,
    imx318mipiraw_COLOR_0340, imx318mipiraw_COLOR_0341, imx318mipiraw_COLOR_0342, imx318mipiraw_COLOR_0343, imx318mipiraw_COLOR_0344, imx318mipiraw_COLOR_0345, imx318mipiraw_COLOR_0346, imx318mipiraw_COLOR_0347, imx318mipiraw_COLOR_0348, imx318mipiraw_COLOR_0349,
    imx318mipiraw_COLOR_0350, imx318mipiraw_COLOR_0351, imx318mipiraw_COLOR_0352, imx318mipiraw_COLOR_0353, imx318mipiraw_COLOR_0354, imx318mipiraw_COLOR_0355, imx318mipiraw_COLOR_0356, imx318mipiraw_COLOR_0357, imx318mipiraw_COLOR_0358, imx318mipiraw_COLOR_0359,
    imx318mipiraw_COLOR_0360, imx318mipiraw_COLOR_0361, imx318mipiraw_COLOR_0362, imx318mipiraw_COLOR_0363, imx318mipiraw_COLOR_0364, imx318mipiraw_COLOR_0365, imx318mipiraw_COLOR_0366, imx318mipiraw_COLOR_0367, imx318mipiraw_COLOR_0368, imx318mipiraw_COLOR_0369,
    imx318mipiraw_COLOR_0370, imx318mipiraw_COLOR_0371, imx318mipiraw_COLOR_0372, imx318mipiraw_COLOR_0373, imx318mipiraw_COLOR_0374, imx318mipiraw_COLOR_0375, imx318mipiraw_COLOR_0376, imx318mipiraw_COLOR_0377, imx318mipiraw_COLOR_0378, imx318mipiraw_COLOR_0379,
    imx318mipiraw_COLOR_0380, imx318mipiraw_COLOR_0381, imx318mipiraw_COLOR_0382, imx318mipiraw_COLOR_0383, imx318mipiraw_COLOR_0384, imx318mipiraw_COLOR_0385, imx318mipiraw_COLOR_0386, imx318mipiraw_COLOR_0387, imx318mipiraw_COLOR_0388, imx318mipiraw_COLOR_0389,
    imx318mipiraw_COLOR_0390, imx318mipiraw_COLOR_0391, imx318mipiraw_COLOR_0392, imx318mipiraw_COLOR_0393, imx318mipiraw_COLOR_0394, imx318mipiraw_COLOR_0395, imx318mipiraw_COLOR_0396, imx318mipiraw_COLOR_0397, imx318mipiraw_COLOR_0398, imx318mipiraw_COLOR_0399,
    imx318mipiraw_COLOR_0400, imx318mipiraw_COLOR_0401, imx318mipiraw_COLOR_0402, imx318mipiraw_COLOR_0403, imx318mipiraw_COLOR_0404, imx318mipiraw_COLOR_0405, imx318mipiraw_COLOR_0406, imx318mipiraw_COLOR_0407, imx318mipiraw_COLOR_0408, imx318mipiraw_COLOR_0409,
    imx318mipiraw_COLOR_0410, imx318mipiraw_COLOR_0411, imx318mipiraw_COLOR_0412, imx318mipiraw_COLOR_0413, imx318mipiraw_COLOR_0414, imx318mipiraw_COLOR_0415, imx318mipiraw_COLOR_0416, imx318mipiraw_COLOR_0417, imx318mipiraw_COLOR_0418, imx318mipiraw_COLOR_0419,
    imx318mipiraw_COLOR_0420, imx318mipiraw_COLOR_0421, imx318mipiraw_COLOR_0422, imx318mipiraw_COLOR_0423, imx318mipiraw_COLOR_0424, imx318mipiraw_COLOR_0425, imx318mipiraw_COLOR_0426, imx318mipiraw_COLOR_0427, imx318mipiraw_COLOR_0428, imx318mipiraw_COLOR_0429,
    imx318mipiraw_COLOR_0430, imx318mipiraw_COLOR_0431, imx318mipiraw_COLOR_0432, imx318mipiraw_COLOR_0433, imx318mipiraw_COLOR_0434, imx318mipiraw_COLOR_0435, imx318mipiraw_COLOR_0436, imx318mipiraw_COLOR_0437, imx318mipiraw_COLOR_0438, imx318mipiraw_COLOR_0439,
    imx318mipiraw_COLOR_0440, imx318mipiraw_COLOR_0441, imx318mipiraw_COLOR_0442, imx318mipiraw_COLOR_0443, imx318mipiraw_COLOR_0444, imx318mipiraw_COLOR_0445, imx318mipiraw_COLOR_0446, imx318mipiraw_COLOR_0447, imx318mipiraw_COLOR_0448, imx318mipiraw_COLOR_0449,
    imx318mipiraw_COLOR_0450, imx318mipiraw_COLOR_0451, imx318mipiraw_COLOR_0452, imx318mipiraw_COLOR_0453, imx318mipiraw_COLOR_0454, imx318mipiraw_COLOR_0455, imx318mipiraw_COLOR_0456, imx318mipiraw_COLOR_0457, imx318mipiraw_COLOR_0458, imx318mipiraw_COLOR_0459,
    imx318mipiraw_COLOR_0460, imx318mipiraw_COLOR_0461, imx318mipiraw_COLOR_0462, imx318mipiraw_COLOR_0463, imx318mipiraw_COLOR_0464, imx318mipiraw_COLOR_0465, imx318mipiraw_COLOR_0466, imx318mipiraw_COLOR_0467, imx318mipiraw_COLOR_0468, imx318mipiraw_COLOR_0469,
    imx318mipiraw_COLOR_0470, imx318mipiraw_COLOR_0471, imx318mipiraw_COLOR_0472, imx318mipiraw_COLOR_0473, imx318mipiraw_COLOR_0474, imx318mipiraw_COLOR_0475, imx318mipiraw_COLOR_0476, imx318mipiraw_COLOR_0477, imx318mipiraw_COLOR_0478, imx318mipiraw_COLOR_0479,
    imx318mipiraw_COLOR_0480, imx318mipiraw_COLOR_0481, imx318mipiraw_COLOR_0482, imx318mipiraw_COLOR_0483, imx318mipiraw_COLOR_0484, imx318mipiraw_COLOR_0485, imx318mipiraw_COLOR_0486, imx318mipiraw_COLOR_0487, imx318mipiraw_COLOR_0488, imx318mipiraw_COLOR_0489,
    imx318mipiraw_COLOR_0490, imx318mipiraw_COLOR_0491, imx318mipiraw_COLOR_0492, imx318mipiraw_COLOR_0493, imx318mipiraw_COLOR_0494, imx318mipiraw_COLOR_0495, imx318mipiraw_COLOR_0496, imx318mipiraw_COLOR_0497, imx318mipiraw_COLOR_0498, imx318mipiraw_COLOR_0499,
    imx318mipiraw_COLOR_0500, imx318mipiraw_COLOR_0501, imx318mipiraw_COLOR_0502, imx318mipiraw_COLOR_0503, imx318mipiraw_COLOR_0504, imx318mipiraw_COLOR_0505, imx318mipiraw_COLOR_0506, imx318mipiraw_COLOR_0507, imx318mipiraw_COLOR_0508, imx318mipiraw_COLOR_0509,
    imx318mipiraw_COLOR_0510, imx318mipiraw_COLOR_0511, imx318mipiraw_COLOR_0512, imx318mipiraw_COLOR_0513, imx318mipiraw_COLOR_0514, imx318mipiraw_COLOR_0515, imx318mipiraw_COLOR_0516, imx318mipiraw_COLOR_0517, imx318mipiraw_COLOR_0518, imx318mipiraw_COLOR_0519,
    imx318mipiraw_COLOR_0520, imx318mipiraw_COLOR_0521, imx318mipiraw_COLOR_0522, imx318mipiraw_COLOR_0523, imx318mipiraw_COLOR_0524, imx318mipiraw_COLOR_0525, imx318mipiraw_COLOR_0526, imx318mipiraw_COLOR_0527, imx318mipiraw_COLOR_0528, imx318mipiraw_COLOR_0529,
    imx318mipiraw_COLOR_0530, imx318mipiraw_COLOR_0531, imx318mipiraw_COLOR_0532, imx318mipiraw_COLOR_0533, imx318mipiraw_COLOR_0534, imx318mipiraw_COLOR_0535, imx318mipiraw_COLOR_0536, imx318mipiraw_COLOR_0537, imx318mipiraw_COLOR_0538, imx318mipiraw_COLOR_0539,
    imx318mipiraw_COLOR_0540, imx318mipiraw_COLOR_0541, imx318mipiraw_COLOR_0542, imx318mipiraw_COLOR_0543, imx318mipiraw_COLOR_0544, imx318mipiraw_COLOR_0545, imx318mipiraw_COLOR_0546, imx318mipiraw_COLOR_0547, imx318mipiraw_COLOR_0548, imx318mipiraw_COLOR_0549,
    imx318mipiraw_COLOR_0550, imx318mipiraw_COLOR_0551, imx318mipiraw_COLOR_0552, imx318mipiraw_COLOR_0553, imx318mipiraw_COLOR_0554, imx318mipiraw_COLOR_0555, imx318mipiraw_COLOR_0556, imx318mipiraw_COLOR_0557, imx318mipiraw_COLOR_0558, imx318mipiraw_COLOR_0559,
    imx318mipiraw_COLOR_0560, imx318mipiraw_COLOR_0561, imx318mipiraw_COLOR_0562, imx318mipiraw_COLOR_0563, imx318mipiraw_COLOR_0564, imx318mipiraw_COLOR_0565, imx318mipiraw_COLOR_0566, imx318mipiraw_COLOR_0567, imx318mipiraw_COLOR_0568, imx318mipiraw_COLOR_0569,
    imx318mipiraw_COLOR_0570, imx318mipiraw_COLOR_0571, imx318mipiraw_COLOR_0572, imx318mipiraw_COLOR_0573, imx318mipiraw_COLOR_0574, imx318mipiraw_COLOR_0575, imx318mipiraw_COLOR_0576, imx318mipiraw_COLOR_0577, imx318mipiraw_COLOR_0578, imx318mipiraw_COLOR_0579,
    imx318mipiraw_COLOR_0580, imx318mipiraw_COLOR_0581, imx318mipiraw_COLOR_0582, imx318mipiraw_COLOR_0583, imx318mipiraw_COLOR_0584, imx318mipiraw_COLOR_0585, imx318mipiraw_COLOR_0586, imx318mipiraw_COLOR_0587, imx318mipiraw_COLOR_0588, imx318mipiraw_COLOR_0589,
    imx318mipiraw_COLOR_0590, imx318mipiraw_COLOR_0591, imx318mipiraw_COLOR_0592, imx318mipiraw_COLOR_0593, imx318mipiraw_COLOR_0594, imx318mipiraw_COLOR_0595, imx318mipiraw_COLOR_0596, imx318mipiraw_COLOR_0597, imx318mipiraw_COLOR_0598, imx318mipiraw_COLOR_0599,
    imx318mipiraw_COLOR_0600, imx318mipiraw_COLOR_0601, imx318mipiraw_COLOR_0602, imx318mipiraw_COLOR_0603, imx318mipiraw_COLOR_0604, imx318mipiraw_COLOR_0605, imx318mipiraw_COLOR_0606, imx318mipiraw_COLOR_0607, imx318mipiraw_COLOR_0608, imx318mipiraw_COLOR_0609,
    imx318mipiraw_COLOR_0610, imx318mipiraw_COLOR_0611, imx318mipiraw_COLOR_0612, imx318mipiraw_COLOR_0613, imx318mipiraw_COLOR_0614, imx318mipiraw_COLOR_0615, imx318mipiraw_COLOR_0616, imx318mipiraw_COLOR_0617, imx318mipiraw_COLOR_0618, imx318mipiraw_COLOR_0619,
    imx318mipiraw_COLOR_0620, imx318mipiraw_COLOR_0621, imx318mipiraw_COLOR_0622, imx318mipiraw_COLOR_0623, imx318mipiraw_COLOR_0624, imx318mipiraw_COLOR_0625, imx318mipiraw_COLOR_0626, imx318mipiraw_COLOR_0627, imx318mipiraw_COLOR_0628, imx318mipiraw_COLOR_0629,
    imx318mipiraw_COLOR_0630, imx318mipiraw_COLOR_0631, imx318mipiraw_COLOR_0632, imx318mipiraw_COLOR_0633, imx318mipiraw_COLOR_0634, imx318mipiraw_COLOR_0635, imx318mipiraw_COLOR_0636, imx318mipiraw_COLOR_0637, imx318mipiraw_COLOR_0638, imx318mipiraw_COLOR_0639,
    imx318mipiraw_COLOR_0640, imx318mipiraw_COLOR_0641, imx318mipiraw_COLOR_0642, imx318mipiraw_COLOR_0643, imx318mipiraw_COLOR_0644, imx318mipiraw_COLOR_0645, imx318mipiraw_COLOR_0646, imx318mipiraw_COLOR_0647, imx318mipiraw_COLOR_0648, imx318mipiraw_COLOR_0649,
    imx318mipiraw_COLOR_0650, imx318mipiraw_COLOR_0651, imx318mipiraw_COLOR_0652, imx318mipiraw_COLOR_0653, imx318mipiraw_COLOR_0654, imx318mipiraw_COLOR_0655, imx318mipiraw_COLOR_0656, imx318mipiraw_COLOR_0657, imx318mipiraw_COLOR_0658, imx318mipiraw_COLOR_0659,
    imx318mipiraw_COLOR_0660, imx318mipiraw_COLOR_0661, imx318mipiraw_COLOR_0662, imx318mipiraw_COLOR_0663, imx318mipiraw_COLOR_0664, imx318mipiraw_COLOR_0665, imx318mipiraw_COLOR_0666, imx318mipiraw_COLOR_0667, imx318mipiraw_COLOR_0668, imx318mipiraw_COLOR_0669,
    imx318mipiraw_COLOR_0670, imx318mipiraw_COLOR_0671, imx318mipiraw_COLOR_0672, imx318mipiraw_COLOR_0673, imx318mipiraw_COLOR_0674, imx318mipiraw_COLOR_0675, imx318mipiraw_COLOR_0676, imx318mipiraw_COLOR_0677, imx318mipiraw_COLOR_0678, imx318mipiraw_COLOR_0679,
    imx318mipiraw_COLOR_0680, imx318mipiraw_COLOR_0681, imx318mipiraw_COLOR_0682, imx318mipiraw_COLOR_0683, imx318mipiraw_COLOR_0684, imx318mipiraw_COLOR_0685, imx318mipiraw_COLOR_0686, imx318mipiraw_COLOR_0687, imx318mipiraw_COLOR_0688, imx318mipiraw_COLOR_0689,
    imx318mipiraw_COLOR_0690, imx318mipiraw_COLOR_0691, imx318mipiraw_COLOR_0692, imx318mipiraw_COLOR_0693, imx318mipiraw_COLOR_0694, imx318mipiraw_COLOR_0695, imx318mipiraw_COLOR_0696, imx318mipiraw_COLOR_0697, imx318mipiraw_COLOR_0698, imx318mipiraw_COLOR_0699,
    imx318mipiraw_COLOR_0700, imx318mipiraw_COLOR_0701, imx318mipiraw_COLOR_0702, imx318mipiraw_COLOR_0703, imx318mipiraw_COLOR_0704, imx318mipiraw_COLOR_0705, imx318mipiraw_COLOR_0706, imx318mipiraw_COLOR_0707, imx318mipiraw_COLOR_0708, imx318mipiraw_COLOR_0709,
    imx318mipiraw_COLOR_0710, imx318mipiraw_COLOR_0711, imx318mipiraw_COLOR_0712, imx318mipiraw_COLOR_0713, imx318mipiraw_COLOR_0714, imx318mipiraw_COLOR_0715, imx318mipiraw_COLOR_0716, imx318mipiraw_COLOR_0717, imx318mipiraw_COLOR_0718, imx318mipiraw_COLOR_0719,
    imx318mipiraw_COLOR_0720, imx318mipiraw_COLOR_0721, imx318mipiraw_COLOR_0722, imx318mipiraw_COLOR_0723, imx318mipiraw_COLOR_0724, imx318mipiraw_COLOR_0725, imx318mipiraw_COLOR_0726, imx318mipiraw_COLOR_0727, imx318mipiraw_COLOR_0728, imx318mipiraw_COLOR_0729,
    imx318mipiraw_COLOR_0730, imx318mipiraw_COLOR_0731, imx318mipiraw_COLOR_0732, imx318mipiraw_COLOR_0733, imx318mipiraw_COLOR_0734, imx318mipiraw_COLOR_0735, imx318mipiraw_COLOR_0736, imx318mipiraw_COLOR_0737, imx318mipiraw_COLOR_0738, imx318mipiraw_COLOR_0739,
    imx318mipiraw_COLOR_0740, imx318mipiraw_COLOR_0741, imx318mipiraw_COLOR_0742, imx318mipiraw_COLOR_0743, imx318mipiraw_COLOR_0744, imx318mipiraw_COLOR_0745, imx318mipiraw_COLOR_0746, imx318mipiraw_COLOR_0747, imx318mipiraw_COLOR_0748, imx318mipiraw_COLOR_0749,
    imx318mipiraw_COLOR_0750, imx318mipiraw_COLOR_0751, imx318mipiraw_COLOR_0752, imx318mipiraw_COLOR_0753, imx318mipiraw_COLOR_0754, imx318mipiraw_COLOR_0755, imx318mipiraw_COLOR_0756, imx318mipiraw_COLOR_0757, imx318mipiraw_COLOR_0758, imx318mipiraw_COLOR_0759,
    imx318mipiraw_COLOR_0760, imx318mipiraw_COLOR_0761, imx318mipiraw_COLOR_0762, imx318mipiraw_COLOR_0763, imx318mipiraw_COLOR_0764, imx318mipiraw_COLOR_0765, imx318mipiraw_COLOR_0766, imx318mipiraw_COLOR_0767, imx318mipiraw_COLOR_0768, imx318mipiraw_COLOR_0769,
    imx318mipiraw_COLOR_0770, imx318mipiraw_COLOR_0771, imx318mipiraw_COLOR_0772, imx318mipiraw_COLOR_0773, imx318mipiraw_COLOR_0774, imx318mipiraw_COLOR_0775, imx318mipiraw_COLOR_0776, imx318mipiraw_COLOR_0777, imx318mipiraw_COLOR_0778, imx318mipiraw_COLOR_0779,
    imx318mipiraw_COLOR_0780, imx318mipiraw_COLOR_0781, imx318mipiraw_COLOR_0782, imx318mipiraw_COLOR_0783, imx318mipiraw_COLOR_0784, imx318mipiraw_COLOR_0785, imx318mipiraw_COLOR_0786, imx318mipiraw_COLOR_0787, imx318mipiraw_COLOR_0788, imx318mipiraw_COLOR_0789,
    imx318mipiraw_COLOR_0790, imx318mipiraw_COLOR_0791, imx318mipiraw_COLOR_0792, imx318mipiraw_COLOR_0793, imx318mipiraw_COLOR_0794, imx318mipiraw_COLOR_0795, imx318mipiraw_COLOR_0796, imx318mipiraw_COLOR_0797, imx318mipiraw_COLOR_0798, imx318mipiraw_COLOR_0799,
    imx318mipiraw_COLOR_0800, imx318mipiraw_COLOR_0801, imx318mipiraw_COLOR_0802, imx318mipiraw_COLOR_0803, imx318mipiraw_COLOR_0804, imx318mipiraw_COLOR_0805, imx318mipiraw_COLOR_0806, imx318mipiraw_COLOR_0807, imx318mipiraw_COLOR_0808, imx318mipiraw_COLOR_0809,
    imx318mipiraw_COLOR_0810, imx318mipiraw_COLOR_0811, imx318mipiraw_COLOR_0812, imx318mipiraw_COLOR_0813, imx318mipiraw_COLOR_0814, imx318mipiraw_COLOR_0815, imx318mipiraw_COLOR_0816, imx318mipiraw_COLOR_0817, imx318mipiraw_COLOR_0818, imx318mipiraw_COLOR_0819,
    imx318mipiraw_COLOR_0820, imx318mipiraw_COLOR_0821, imx318mipiraw_COLOR_0822, imx318mipiraw_COLOR_0823, imx318mipiraw_COLOR_0824, imx318mipiraw_COLOR_0825, imx318mipiraw_COLOR_0826, imx318mipiraw_COLOR_0827, imx318mipiraw_COLOR_0828, imx318mipiraw_COLOR_0829,
    imx318mipiraw_COLOR_0830, imx318mipiraw_COLOR_0831, imx318mipiraw_COLOR_0832, imx318mipiraw_COLOR_0833, imx318mipiraw_COLOR_0834, imx318mipiraw_COLOR_0835, imx318mipiraw_COLOR_0836, imx318mipiraw_COLOR_0837, imx318mipiraw_COLOR_0838, imx318mipiraw_COLOR_0839,
    imx318mipiraw_COLOR_0840, imx318mipiraw_COLOR_0841, imx318mipiraw_COLOR_0842, imx318mipiraw_COLOR_0843, imx318mipiraw_COLOR_0844, imx318mipiraw_COLOR_0845, imx318mipiraw_COLOR_0846, imx318mipiraw_COLOR_0847, imx318mipiraw_COLOR_0848, imx318mipiraw_COLOR_0849,
    imx318mipiraw_COLOR_0850, imx318mipiraw_COLOR_0851, imx318mipiraw_COLOR_0852, imx318mipiraw_COLOR_0853, imx318mipiraw_COLOR_0854, imx318mipiraw_COLOR_0855, imx318mipiraw_COLOR_0856, imx318mipiraw_COLOR_0857, imx318mipiraw_COLOR_0858, imx318mipiraw_COLOR_0859,
    imx318mipiraw_COLOR_0860, imx318mipiraw_COLOR_0861, imx318mipiraw_COLOR_0862, imx318mipiraw_COLOR_0863, imx318mipiraw_COLOR_0864, imx318mipiraw_COLOR_0865, imx318mipiraw_COLOR_0866, imx318mipiraw_COLOR_0867, imx318mipiraw_COLOR_0868, imx318mipiraw_COLOR_0869,
    imx318mipiraw_COLOR_0870, imx318mipiraw_COLOR_0871, imx318mipiraw_COLOR_0872, imx318mipiraw_COLOR_0873, imx318mipiraw_COLOR_0874, imx318mipiraw_COLOR_0875, imx318mipiraw_COLOR_0876, imx318mipiraw_COLOR_0877, imx318mipiraw_COLOR_0878, imx318mipiraw_COLOR_0879,
    imx318mipiraw_COLOR_0880, imx318mipiraw_COLOR_0881, imx318mipiraw_COLOR_0882, imx318mipiraw_COLOR_0883, imx318mipiraw_COLOR_0884, imx318mipiraw_COLOR_0885, imx318mipiraw_COLOR_0886, imx318mipiraw_COLOR_0887, imx318mipiraw_COLOR_0888, imx318mipiraw_COLOR_0889,
    imx318mipiraw_COLOR_0890, imx318mipiraw_COLOR_0891, imx318mipiraw_COLOR_0892, imx318mipiraw_COLOR_0893, imx318mipiraw_COLOR_0894, imx318mipiraw_COLOR_0895, imx318mipiraw_COLOR_0896, imx318mipiraw_COLOR_0897, imx318mipiraw_COLOR_0898, imx318mipiraw_COLOR_0899,
    imx318mipiraw_COLOR_0900, imx318mipiraw_COLOR_0901, imx318mipiraw_COLOR_0902, imx318mipiraw_COLOR_0903, imx318mipiraw_COLOR_0904, imx318mipiraw_COLOR_0905, imx318mipiraw_COLOR_0906, imx318mipiraw_COLOR_0907, imx318mipiraw_COLOR_0908, imx318mipiraw_COLOR_0909,
    imx318mipiraw_COLOR_0910, imx318mipiraw_COLOR_0911, imx318mipiraw_COLOR_0912, imx318mipiraw_COLOR_0913, imx318mipiraw_COLOR_0914, imx318mipiraw_COLOR_0915, imx318mipiraw_COLOR_0916, imx318mipiraw_COLOR_0917, imx318mipiraw_COLOR_0918, imx318mipiraw_COLOR_0919,
    imx318mipiraw_COLOR_0920, imx318mipiraw_COLOR_0921, imx318mipiraw_COLOR_0922, imx318mipiraw_COLOR_0923, imx318mipiraw_COLOR_0924, imx318mipiraw_COLOR_0925, imx318mipiraw_COLOR_0926, imx318mipiraw_COLOR_0927, imx318mipiraw_COLOR_0928, imx318mipiraw_COLOR_0929,
    imx318mipiraw_COLOR_0930, imx318mipiraw_COLOR_0931, imx318mipiraw_COLOR_0932, imx318mipiraw_COLOR_0933, imx318mipiraw_COLOR_0934, imx318mipiraw_COLOR_0935, imx318mipiraw_COLOR_0936, imx318mipiraw_COLOR_0937, imx318mipiraw_COLOR_0938, imx318mipiraw_COLOR_0939,
    imx318mipiraw_COLOR_0940, imx318mipiraw_COLOR_0941, imx318mipiraw_COLOR_0942, imx318mipiraw_COLOR_0943, imx318mipiraw_COLOR_0944, imx318mipiraw_COLOR_0945, imx318mipiraw_COLOR_0946, imx318mipiraw_COLOR_0947, imx318mipiraw_COLOR_0948, imx318mipiraw_COLOR_0949,
    imx318mipiraw_COLOR_0950, imx318mipiraw_COLOR_0951, imx318mipiraw_COLOR_0952, imx318mipiraw_COLOR_0953, imx318mipiraw_COLOR_0954, imx318mipiraw_COLOR_0955, imx318mipiraw_COLOR_0956, imx318mipiraw_COLOR_0957, imx318mipiraw_COLOR_0958, imx318mipiraw_COLOR_0959,
},
.PCA = {
    imx318mipiraw_PCA_0000,
},
.COLOR_PARAM = {
    imx318mipiraw_COLOR_PARAM_0000, imx318mipiraw_COLOR_PARAM_0001, imx318mipiraw_COLOR_PARAM_0002, imx318mipiraw_COLOR_PARAM_0003, imx318mipiraw_COLOR_PARAM_0004, imx318mipiraw_COLOR_PARAM_0005, imx318mipiraw_COLOR_PARAM_0006, imx318mipiraw_COLOR_PARAM_0007, imx318mipiraw_COLOR_PARAM_0008, imx318mipiraw_COLOR_PARAM_0009,
    imx318mipiraw_COLOR_PARAM_0010, imx318mipiraw_COLOR_PARAM_0011, imx318mipiraw_COLOR_PARAM_0012, imx318mipiraw_COLOR_PARAM_0013, imx318mipiraw_COLOR_PARAM_0014, imx318mipiraw_COLOR_PARAM_0015,
},
