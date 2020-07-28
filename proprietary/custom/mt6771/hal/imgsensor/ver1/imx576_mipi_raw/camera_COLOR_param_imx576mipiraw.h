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
    imx576mipiraw_CCM_0000, imx576mipiraw_CCM_0001, imx576mipiraw_CCM_0002, imx576mipiraw_CCM_0003, imx576mipiraw_CCM_0004, imx576mipiraw_CCM_0005, imx576mipiraw_CCM_0006, imx576mipiraw_CCM_0007, imx576mipiraw_CCM_0008, imx576mipiraw_CCM_0009,
    imx576mipiraw_CCM_0010, imx576mipiraw_CCM_0011, imx576mipiraw_CCM_0012, imx576mipiraw_CCM_0013, imx576mipiraw_CCM_0014, imx576mipiraw_CCM_0015, imx576mipiraw_CCM_0016, imx576mipiraw_CCM_0017, imx576mipiraw_CCM_0018, imx576mipiraw_CCM_0019,
    imx576mipiraw_CCM_0020, imx576mipiraw_CCM_0021, imx576mipiraw_CCM_0022, imx576mipiraw_CCM_0023, imx576mipiraw_CCM_0024, imx576mipiraw_CCM_0025, imx576mipiraw_CCM_0026, imx576mipiraw_CCM_0027, imx576mipiraw_CCM_0028, imx576mipiraw_CCM_0029,
    imx576mipiraw_CCM_0030, imx576mipiraw_CCM_0031, imx576mipiraw_CCM_0032, imx576mipiraw_CCM_0033, imx576mipiraw_CCM_0034, imx576mipiraw_CCM_0035, imx576mipiraw_CCM_0036, imx576mipiraw_CCM_0037, imx576mipiraw_CCM_0038, imx576mipiraw_CCM_0039,
    imx576mipiraw_CCM_0040, imx576mipiraw_CCM_0041, imx576mipiraw_CCM_0042, imx576mipiraw_CCM_0043, imx576mipiraw_CCM_0044, imx576mipiraw_CCM_0045, imx576mipiraw_CCM_0046, imx576mipiraw_CCM_0047, imx576mipiraw_CCM_0048, imx576mipiraw_CCM_0049,
    imx576mipiraw_CCM_0050, imx576mipiraw_CCM_0051, imx576mipiraw_CCM_0052, imx576mipiraw_CCM_0053, imx576mipiraw_CCM_0054, imx576mipiraw_CCM_0055, imx576mipiraw_CCM_0056, imx576mipiraw_CCM_0057, imx576mipiraw_CCM_0058, imx576mipiraw_CCM_0059,
    imx576mipiraw_CCM_0060, imx576mipiraw_CCM_0061, imx576mipiraw_CCM_0062, imx576mipiraw_CCM_0063, imx576mipiraw_CCM_0064, imx576mipiraw_CCM_0065, imx576mipiraw_CCM_0066, imx576mipiraw_CCM_0067, imx576mipiraw_CCM_0068, imx576mipiraw_CCM_0069,
    imx576mipiraw_CCM_0070, imx576mipiraw_CCM_0071, imx576mipiraw_CCM_0072, imx576mipiraw_CCM_0073, imx576mipiraw_CCM_0074, imx576mipiraw_CCM_0075, imx576mipiraw_CCM_0076, imx576mipiraw_CCM_0077, imx576mipiraw_CCM_0078, imx576mipiraw_CCM_0079,
    imx576mipiraw_CCM_0080, imx576mipiraw_CCM_0081, imx576mipiraw_CCM_0082, imx576mipiraw_CCM_0083, imx576mipiraw_CCM_0084, imx576mipiraw_CCM_0085, imx576mipiraw_CCM_0086, imx576mipiraw_CCM_0087, imx576mipiraw_CCM_0088, imx576mipiraw_CCM_0089,
    imx576mipiraw_CCM_0090, imx576mipiraw_CCM_0091, imx576mipiraw_CCM_0092, imx576mipiraw_CCM_0093, imx576mipiraw_CCM_0094, imx576mipiraw_CCM_0095, imx576mipiraw_CCM_0096, imx576mipiraw_CCM_0097, imx576mipiraw_CCM_0098, imx576mipiraw_CCM_0099,
    imx576mipiraw_CCM_0100, imx576mipiraw_CCM_0101, imx576mipiraw_CCM_0102, imx576mipiraw_CCM_0103, imx576mipiraw_CCM_0104, imx576mipiraw_CCM_0105, imx576mipiraw_CCM_0106, imx576mipiraw_CCM_0107, imx576mipiraw_CCM_0108, imx576mipiraw_CCM_0109,
    imx576mipiraw_CCM_0110, imx576mipiraw_CCM_0111, imx576mipiraw_CCM_0112, imx576mipiraw_CCM_0113, imx576mipiraw_CCM_0114, imx576mipiraw_CCM_0115, imx576mipiraw_CCM_0116, imx576mipiraw_CCM_0117, imx576mipiraw_CCM_0118, imx576mipiraw_CCM_0119,
    imx576mipiraw_CCM_0120, imx576mipiraw_CCM_0121, imx576mipiraw_CCM_0122, imx576mipiraw_CCM_0123, imx576mipiraw_CCM_0124, imx576mipiraw_CCM_0125, imx576mipiraw_CCM_0126, imx576mipiraw_CCM_0127, imx576mipiraw_CCM_0128, imx576mipiraw_CCM_0129,
    imx576mipiraw_CCM_0130, imx576mipiraw_CCM_0131, imx576mipiraw_CCM_0132, imx576mipiraw_CCM_0133, imx576mipiraw_CCM_0134, imx576mipiraw_CCM_0135, imx576mipiraw_CCM_0136, imx576mipiraw_CCM_0137, imx576mipiraw_CCM_0138, imx576mipiraw_CCM_0139,
    imx576mipiraw_CCM_0140, imx576mipiraw_CCM_0141, imx576mipiraw_CCM_0142, imx576mipiraw_CCM_0143, imx576mipiraw_CCM_0144, imx576mipiraw_CCM_0145, imx576mipiraw_CCM_0146, imx576mipiraw_CCM_0147, imx576mipiraw_CCM_0148, imx576mipiraw_CCM_0149,
    imx576mipiraw_CCM_0150, imx576mipiraw_CCM_0151, imx576mipiraw_CCM_0152, imx576mipiraw_CCM_0153, imx576mipiraw_CCM_0154, imx576mipiraw_CCM_0155, imx576mipiraw_CCM_0156, imx576mipiraw_CCM_0157, imx576mipiraw_CCM_0158, imx576mipiraw_CCM_0159,
    imx576mipiraw_CCM_0160, imx576mipiraw_CCM_0161, imx576mipiraw_CCM_0162, imx576mipiraw_CCM_0163, imx576mipiraw_CCM_0164, imx576mipiraw_CCM_0165, imx576mipiraw_CCM_0166, imx576mipiraw_CCM_0167, imx576mipiraw_CCM_0168, imx576mipiraw_CCM_0169,
    imx576mipiraw_CCM_0170, imx576mipiraw_CCM_0171, imx576mipiraw_CCM_0172, imx576mipiraw_CCM_0173, imx576mipiraw_CCM_0174, imx576mipiraw_CCM_0175, imx576mipiraw_CCM_0176, imx576mipiraw_CCM_0177, imx576mipiraw_CCM_0178, imx576mipiraw_CCM_0179,
    imx576mipiraw_CCM_0180, imx576mipiraw_CCM_0181, imx576mipiraw_CCM_0182, imx576mipiraw_CCM_0183, imx576mipiraw_CCM_0184, imx576mipiraw_CCM_0185, imx576mipiraw_CCM_0186, imx576mipiraw_CCM_0187, imx576mipiraw_CCM_0188, imx576mipiraw_CCM_0189,
    imx576mipiraw_CCM_0190, imx576mipiraw_CCM_0191, imx576mipiraw_CCM_0192, imx576mipiraw_CCM_0193, imx576mipiraw_CCM_0194, imx576mipiraw_CCM_0195, imx576mipiraw_CCM_0196, imx576mipiraw_CCM_0197, imx576mipiraw_CCM_0198, imx576mipiraw_CCM_0199,
    imx576mipiraw_CCM_0200, imx576mipiraw_CCM_0201, imx576mipiraw_CCM_0202, imx576mipiraw_CCM_0203, imx576mipiraw_CCM_0204, imx576mipiraw_CCM_0205, imx576mipiraw_CCM_0206, imx576mipiraw_CCM_0207, imx576mipiraw_CCM_0208, imx576mipiraw_CCM_0209,
    imx576mipiraw_CCM_0210, imx576mipiraw_CCM_0211, imx576mipiraw_CCM_0212, imx576mipiraw_CCM_0213, imx576mipiraw_CCM_0214, imx576mipiraw_CCM_0215, imx576mipiraw_CCM_0216, imx576mipiraw_CCM_0217, imx576mipiraw_CCM_0218, imx576mipiraw_CCM_0219,
    imx576mipiraw_CCM_0220, imx576mipiraw_CCM_0221, imx576mipiraw_CCM_0222, imx576mipiraw_CCM_0223, imx576mipiraw_CCM_0224, imx576mipiraw_CCM_0225, imx576mipiraw_CCM_0226, imx576mipiraw_CCM_0227, imx576mipiraw_CCM_0228, imx576mipiraw_CCM_0229,
    imx576mipiraw_CCM_0230, imx576mipiraw_CCM_0231, imx576mipiraw_CCM_0232, imx576mipiraw_CCM_0233, imx576mipiraw_CCM_0234, imx576mipiraw_CCM_0235, imx576mipiraw_CCM_0236, imx576mipiraw_CCM_0237, imx576mipiraw_CCM_0238, imx576mipiraw_CCM_0239,
},
.COLOR = {
    imx576mipiraw_COLOR_0000, imx576mipiraw_COLOR_0001, imx576mipiraw_COLOR_0002, imx576mipiraw_COLOR_0003, imx576mipiraw_COLOR_0004, imx576mipiraw_COLOR_0005, imx576mipiraw_COLOR_0006, imx576mipiraw_COLOR_0007, imx576mipiraw_COLOR_0008, imx576mipiraw_COLOR_0009,
    imx576mipiraw_COLOR_0010, imx576mipiraw_COLOR_0011, imx576mipiraw_COLOR_0012, imx576mipiraw_COLOR_0013, imx576mipiraw_COLOR_0014, imx576mipiraw_COLOR_0015, imx576mipiraw_COLOR_0016, imx576mipiraw_COLOR_0017, imx576mipiraw_COLOR_0018, imx576mipiraw_COLOR_0019,
    imx576mipiraw_COLOR_0020, imx576mipiraw_COLOR_0021, imx576mipiraw_COLOR_0022, imx576mipiraw_COLOR_0023, imx576mipiraw_COLOR_0024, imx576mipiraw_COLOR_0025, imx576mipiraw_COLOR_0026, imx576mipiraw_COLOR_0027, imx576mipiraw_COLOR_0028, imx576mipiraw_COLOR_0029,
    imx576mipiraw_COLOR_0030, imx576mipiraw_COLOR_0031, imx576mipiraw_COLOR_0032, imx576mipiraw_COLOR_0033, imx576mipiraw_COLOR_0034, imx576mipiraw_COLOR_0035, imx576mipiraw_COLOR_0036, imx576mipiraw_COLOR_0037, imx576mipiraw_COLOR_0038, imx576mipiraw_COLOR_0039,
    imx576mipiraw_COLOR_0040, imx576mipiraw_COLOR_0041, imx576mipiraw_COLOR_0042, imx576mipiraw_COLOR_0043, imx576mipiraw_COLOR_0044, imx576mipiraw_COLOR_0045, imx576mipiraw_COLOR_0046, imx576mipiraw_COLOR_0047, imx576mipiraw_COLOR_0048, imx576mipiraw_COLOR_0049,
    imx576mipiraw_COLOR_0050, imx576mipiraw_COLOR_0051, imx576mipiraw_COLOR_0052, imx576mipiraw_COLOR_0053, imx576mipiraw_COLOR_0054, imx576mipiraw_COLOR_0055, imx576mipiraw_COLOR_0056, imx576mipiraw_COLOR_0057, imx576mipiraw_COLOR_0058, imx576mipiraw_COLOR_0059,
    imx576mipiraw_COLOR_0060, imx576mipiraw_COLOR_0061, imx576mipiraw_COLOR_0062, imx576mipiraw_COLOR_0063, imx576mipiraw_COLOR_0064, imx576mipiraw_COLOR_0065, imx576mipiraw_COLOR_0066, imx576mipiraw_COLOR_0067, imx576mipiraw_COLOR_0068, imx576mipiraw_COLOR_0069,
    imx576mipiraw_COLOR_0070, imx576mipiraw_COLOR_0071, imx576mipiraw_COLOR_0072, imx576mipiraw_COLOR_0073, imx576mipiraw_COLOR_0074, imx576mipiraw_COLOR_0075, imx576mipiraw_COLOR_0076, imx576mipiraw_COLOR_0077, imx576mipiraw_COLOR_0078, imx576mipiraw_COLOR_0079,
    imx576mipiraw_COLOR_0080, imx576mipiraw_COLOR_0081, imx576mipiraw_COLOR_0082, imx576mipiraw_COLOR_0083, imx576mipiraw_COLOR_0084, imx576mipiraw_COLOR_0085, imx576mipiraw_COLOR_0086, imx576mipiraw_COLOR_0087, imx576mipiraw_COLOR_0088, imx576mipiraw_COLOR_0089,
    imx576mipiraw_COLOR_0090, imx576mipiraw_COLOR_0091, imx576mipiraw_COLOR_0092, imx576mipiraw_COLOR_0093, imx576mipiraw_COLOR_0094, imx576mipiraw_COLOR_0095, imx576mipiraw_COLOR_0096, imx576mipiraw_COLOR_0097, imx576mipiraw_COLOR_0098, imx576mipiraw_COLOR_0099,
    imx576mipiraw_COLOR_0100, imx576mipiraw_COLOR_0101, imx576mipiraw_COLOR_0102, imx576mipiraw_COLOR_0103, imx576mipiraw_COLOR_0104, imx576mipiraw_COLOR_0105, imx576mipiraw_COLOR_0106, imx576mipiraw_COLOR_0107, imx576mipiraw_COLOR_0108, imx576mipiraw_COLOR_0109,
    imx576mipiraw_COLOR_0110, imx576mipiraw_COLOR_0111, imx576mipiraw_COLOR_0112, imx576mipiraw_COLOR_0113, imx576mipiraw_COLOR_0114, imx576mipiraw_COLOR_0115, imx576mipiraw_COLOR_0116, imx576mipiraw_COLOR_0117, imx576mipiraw_COLOR_0118, imx576mipiraw_COLOR_0119,
    imx576mipiraw_COLOR_0120, imx576mipiraw_COLOR_0121, imx576mipiraw_COLOR_0122, imx576mipiraw_COLOR_0123, imx576mipiraw_COLOR_0124, imx576mipiraw_COLOR_0125, imx576mipiraw_COLOR_0126, imx576mipiraw_COLOR_0127, imx576mipiraw_COLOR_0128, imx576mipiraw_COLOR_0129,
    imx576mipiraw_COLOR_0130, imx576mipiraw_COLOR_0131, imx576mipiraw_COLOR_0132, imx576mipiraw_COLOR_0133, imx576mipiraw_COLOR_0134, imx576mipiraw_COLOR_0135, imx576mipiraw_COLOR_0136, imx576mipiraw_COLOR_0137, imx576mipiraw_COLOR_0138, imx576mipiraw_COLOR_0139,
    imx576mipiraw_COLOR_0140, imx576mipiraw_COLOR_0141, imx576mipiraw_COLOR_0142, imx576mipiraw_COLOR_0143, imx576mipiraw_COLOR_0144, imx576mipiraw_COLOR_0145, imx576mipiraw_COLOR_0146, imx576mipiraw_COLOR_0147, imx576mipiraw_COLOR_0148, imx576mipiraw_COLOR_0149,
    imx576mipiraw_COLOR_0150, imx576mipiraw_COLOR_0151, imx576mipiraw_COLOR_0152, imx576mipiraw_COLOR_0153, imx576mipiraw_COLOR_0154, imx576mipiraw_COLOR_0155, imx576mipiraw_COLOR_0156, imx576mipiraw_COLOR_0157, imx576mipiraw_COLOR_0158, imx576mipiraw_COLOR_0159,
    imx576mipiraw_COLOR_0160, imx576mipiraw_COLOR_0161, imx576mipiraw_COLOR_0162, imx576mipiraw_COLOR_0163, imx576mipiraw_COLOR_0164, imx576mipiraw_COLOR_0165, imx576mipiraw_COLOR_0166, imx576mipiraw_COLOR_0167, imx576mipiraw_COLOR_0168, imx576mipiraw_COLOR_0169,
    imx576mipiraw_COLOR_0170, imx576mipiraw_COLOR_0171, imx576mipiraw_COLOR_0172, imx576mipiraw_COLOR_0173, imx576mipiraw_COLOR_0174, imx576mipiraw_COLOR_0175, imx576mipiraw_COLOR_0176, imx576mipiraw_COLOR_0177, imx576mipiraw_COLOR_0178, imx576mipiraw_COLOR_0179,
    imx576mipiraw_COLOR_0180, imx576mipiraw_COLOR_0181, imx576mipiraw_COLOR_0182, imx576mipiraw_COLOR_0183, imx576mipiraw_COLOR_0184, imx576mipiraw_COLOR_0185, imx576mipiraw_COLOR_0186, imx576mipiraw_COLOR_0187, imx576mipiraw_COLOR_0188, imx576mipiraw_COLOR_0189,
    imx576mipiraw_COLOR_0190, imx576mipiraw_COLOR_0191, imx576mipiraw_COLOR_0192, imx576mipiraw_COLOR_0193, imx576mipiraw_COLOR_0194, imx576mipiraw_COLOR_0195, imx576mipiraw_COLOR_0196, imx576mipiraw_COLOR_0197, imx576mipiraw_COLOR_0198, imx576mipiraw_COLOR_0199,
    imx576mipiraw_COLOR_0200, imx576mipiraw_COLOR_0201, imx576mipiraw_COLOR_0202, imx576mipiraw_COLOR_0203, imx576mipiraw_COLOR_0204, imx576mipiraw_COLOR_0205, imx576mipiraw_COLOR_0206, imx576mipiraw_COLOR_0207, imx576mipiraw_COLOR_0208, imx576mipiraw_COLOR_0209,
    imx576mipiraw_COLOR_0210, imx576mipiraw_COLOR_0211, imx576mipiraw_COLOR_0212, imx576mipiraw_COLOR_0213, imx576mipiraw_COLOR_0214, imx576mipiraw_COLOR_0215, imx576mipiraw_COLOR_0216, imx576mipiraw_COLOR_0217, imx576mipiraw_COLOR_0218, imx576mipiraw_COLOR_0219,
    imx576mipiraw_COLOR_0220, imx576mipiraw_COLOR_0221, imx576mipiraw_COLOR_0222, imx576mipiraw_COLOR_0223, imx576mipiraw_COLOR_0224, imx576mipiraw_COLOR_0225, imx576mipiraw_COLOR_0226, imx576mipiraw_COLOR_0227, imx576mipiraw_COLOR_0228, imx576mipiraw_COLOR_0229,
    imx576mipiraw_COLOR_0230, imx576mipiraw_COLOR_0231, imx576mipiraw_COLOR_0232, imx576mipiraw_COLOR_0233, imx576mipiraw_COLOR_0234, imx576mipiraw_COLOR_0235, imx576mipiraw_COLOR_0236, imx576mipiraw_COLOR_0237, imx576mipiraw_COLOR_0238, imx576mipiraw_COLOR_0239,
    imx576mipiraw_COLOR_0240, imx576mipiraw_COLOR_0241, imx576mipiraw_COLOR_0242, imx576mipiraw_COLOR_0243, imx576mipiraw_COLOR_0244, imx576mipiraw_COLOR_0245, imx576mipiraw_COLOR_0246, imx576mipiraw_COLOR_0247, imx576mipiraw_COLOR_0248, imx576mipiraw_COLOR_0249,
    imx576mipiraw_COLOR_0250, imx576mipiraw_COLOR_0251, imx576mipiraw_COLOR_0252, imx576mipiraw_COLOR_0253, imx576mipiraw_COLOR_0254, imx576mipiraw_COLOR_0255, imx576mipiraw_COLOR_0256, imx576mipiraw_COLOR_0257, imx576mipiraw_COLOR_0258, imx576mipiraw_COLOR_0259,
    imx576mipiraw_COLOR_0260, imx576mipiraw_COLOR_0261, imx576mipiraw_COLOR_0262, imx576mipiraw_COLOR_0263, imx576mipiraw_COLOR_0264, imx576mipiraw_COLOR_0265, imx576mipiraw_COLOR_0266, imx576mipiraw_COLOR_0267, imx576mipiraw_COLOR_0268, imx576mipiraw_COLOR_0269,
    imx576mipiraw_COLOR_0270, imx576mipiraw_COLOR_0271, imx576mipiraw_COLOR_0272, imx576mipiraw_COLOR_0273, imx576mipiraw_COLOR_0274, imx576mipiraw_COLOR_0275, imx576mipiraw_COLOR_0276, imx576mipiraw_COLOR_0277, imx576mipiraw_COLOR_0278, imx576mipiraw_COLOR_0279,
    imx576mipiraw_COLOR_0280, imx576mipiraw_COLOR_0281, imx576mipiraw_COLOR_0282, imx576mipiraw_COLOR_0283, imx576mipiraw_COLOR_0284, imx576mipiraw_COLOR_0285, imx576mipiraw_COLOR_0286, imx576mipiraw_COLOR_0287, imx576mipiraw_COLOR_0288, imx576mipiraw_COLOR_0289,
    imx576mipiraw_COLOR_0290, imx576mipiraw_COLOR_0291, imx576mipiraw_COLOR_0292, imx576mipiraw_COLOR_0293, imx576mipiraw_COLOR_0294, imx576mipiraw_COLOR_0295, imx576mipiraw_COLOR_0296, imx576mipiraw_COLOR_0297, imx576mipiraw_COLOR_0298, imx576mipiraw_COLOR_0299,
    imx576mipiraw_COLOR_0300, imx576mipiraw_COLOR_0301, imx576mipiraw_COLOR_0302, imx576mipiraw_COLOR_0303, imx576mipiraw_COLOR_0304, imx576mipiraw_COLOR_0305, imx576mipiraw_COLOR_0306, imx576mipiraw_COLOR_0307, imx576mipiraw_COLOR_0308, imx576mipiraw_COLOR_0309,
    imx576mipiraw_COLOR_0310, imx576mipiraw_COLOR_0311, imx576mipiraw_COLOR_0312, imx576mipiraw_COLOR_0313, imx576mipiraw_COLOR_0314, imx576mipiraw_COLOR_0315, imx576mipiraw_COLOR_0316, imx576mipiraw_COLOR_0317, imx576mipiraw_COLOR_0318, imx576mipiraw_COLOR_0319,
    imx576mipiraw_COLOR_0320, imx576mipiraw_COLOR_0321, imx576mipiraw_COLOR_0322, imx576mipiraw_COLOR_0323, imx576mipiraw_COLOR_0324, imx576mipiraw_COLOR_0325, imx576mipiraw_COLOR_0326, imx576mipiraw_COLOR_0327, imx576mipiraw_COLOR_0328, imx576mipiraw_COLOR_0329,
    imx576mipiraw_COLOR_0330, imx576mipiraw_COLOR_0331, imx576mipiraw_COLOR_0332, imx576mipiraw_COLOR_0333, imx576mipiraw_COLOR_0334, imx576mipiraw_COLOR_0335, imx576mipiraw_COLOR_0336, imx576mipiraw_COLOR_0337, imx576mipiraw_COLOR_0338, imx576mipiraw_COLOR_0339,
    imx576mipiraw_COLOR_0340, imx576mipiraw_COLOR_0341, imx576mipiraw_COLOR_0342, imx576mipiraw_COLOR_0343, imx576mipiraw_COLOR_0344, imx576mipiraw_COLOR_0345, imx576mipiraw_COLOR_0346, imx576mipiraw_COLOR_0347, imx576mipiraw_COLOR_0348, imx576mipiraw_COLOR_0349,
    imx576mipiraw_COLOR_0350, imx576mipiraw_COLOR_0351, imx576mipiraw_COLOR_0352, imx576mipiraw_COLOR_0353, imx576mipiraw_COLOR_0354, imx576mipiraw_COLOR_0355, imx576mipiraw_COLOR_0356, imx576mipiraw_COLOR_0357, imx576mipiraw_COLOR_0358, imx576mipiraw_COLOR_0359,
    imx576mipiraw_COLOR_0360, imx576mipiraw_COLOR_0361, imx576mipiraw_COLOR_0362, imx576mipiraw_COLOR_0363, imx576mipiraw_COLOR_0364, imx576mipiraw_COLOR_0365, imx576mipiraw_COLOR_0366, imx576mipiraw_COLOR_0367, imx576mipiraw_COLOR_0368, imx576mipiraw_COLOR_0369,
    imx576mipiraw_COLOR_0370, imx576mipiraw_COLOR_0371, imx576mipiraw_COLOR_0372, imx576mipiraw_COLOR_0373, imx576mipiraw_COLOR_0374, imx576mipiraw_COLOR_0375, imx576mipiraw_COLOR_0376, imx576mipiraw_COLOR_0377, imx576mipiraw_COLOR_0378, imx576mipiraw_COLOR_0379,
    imx576mipiraw_COLOR_0380, imx576mipiraw_COLOR_0381, imx576mipiraw_COLOR_0382, imx576mipiraw_COLOR_0383, imx576mipiraw_COLOR_0384, imx576mipiraw_COLOR_0385, imx576mipiraw_COLOR_0386, imx576mipiraw_COLOR_0387, imx576mipiraw_COLOR_0388, imx576mipiraw_COLOR_0389,
    imx576mipiraw_COLOR_0390, imx576mipiraw_COLOR_0391, imx576mipiraw_COLOR_0392, imx576mipiraw_COLOR_0393, imx576mipiraw_COLOR_0394, imx576mipiraw_COLOR_0395, imx576mipiraw_COLOR_0396, imx576mipiraw_COLOR_0397, imx576mipiraw_COLOR_0398, imx576mipiraw_COLOR_0399,
    imx576mipiraw_COLOR_0400, imx576mipiraw_COLOR_0401, imx576mipiraw_COLOR_0402, imx576mipiraw_COLOR_0403, imx576mipiraw_COLOR_0404, imx576mipiraw_COLOR_0405, imx576mipiraw_COLOR_0406, imx576mipiraw_COLOR_0407, imx576mipiraw_COLOR_0408, imx576mipiraw_COLOR_0409,
    imx576mipiraw_COLOR_0410, imx576mipiraw_COLOR_0411, imx576mipiraw_COLOR_0412, imx576mipiraw_COLOR_0413, imx576mipiraw_COLOR_0414, imx576mipiraw_COLOR_0415, imx576mipiraw_COLOR_0416, imx576mipiraw_COLOR_0417, imx576mipiraw_COLOR_0418, imx576mipiraw_COLOR_0419,
    imx576mipiraw_COLOR_0420, imx576mipiraw_COLOR_0421, imx576mipiraw_COLOR_0422, imx576mipiraw_COLOR_0423, imx576mipiraw_COLOR_0424, imx576mipiraw_COLOR_0425, imx576mipiraw_COLOR_0426, imx576mipiraw_COLOR_0427, imx576mipiraw_COLOR_0428, imx576mipiraw_COLOR_0429,
    imx576mipiraw_COLOR_0430, imx576mipiraw_COLOR_0431, imx576mipiraw_COLOR_0432, imx576mipiraw_COLOR_0433, imx576mipiraw_COLOR_0434, imx576mipiraw_COLOR_0435, imx576mipiraw_COLOR_0436, imx576mipiraw_COLOR_0437, imx576mipiraw_COLOR_0438, imx576mipiraw_COLOR_0439,
    imx576mipiraw_COLOR_0440, imx576mipiraw_COLOR_0441, imx576mipiraw_COLOR_0442, imx576mipiraw_COLOR_0443, imx576mipiraw_COLOR_0444, imx576mipiraw_COLOR_0445, imx576mipiraw_COLOR_0446, imx576mipiraw_COLOR_0447, imx576mipiraw_COLOR_0448, imx576mipiraw_COLOR_0449,
    imx576mipiraw_COLOR_0450, imx576mipiraw_COLOR_0451, imx576mipiraw_COLOR_0452, imx576mipiraw_COLOR_0453, imx576mipiraw_COLOR_0454, imx576mipiraw_COLOR_0455, imx576mipiraw_COLOR_0456, imx576mipiraw_COLOR_0457, imx576mipiraw_COLOR_0458, imx576mipiraw_COLOR_0459,
    imx576mipiraw_COLOR_0460, imx576mipiraw_COLOR_0461, imx576mipiraw_COLOR_0462, imx576mipiraw_COLOR_0463, imx576mipiraw_COLOR_0464, imx576mipiraw_COLOR_0465, imx576mipiraw_COLOR_0466, imx576mipiraw_COLOR_0467, imx576mipiraw_COLOR_0468, imx576mipiraw_COLOR_0469,
    imx576mipiraw_COLOR_0470, imx576mipiraw_COLOR_0471, imx576mipiraw_COLOR_0472, imx576mipiraw_COLOR_0473, imx576mipiraw_COLOR_0474, imx576mipiraw_COLOR_0475, imx576mipiraw_COLOR_0476, imx576mipiraw_COLOR_0477, imx576mipiraw_COLOR_0478, imx576mipiraw_COLOR_0479,
    imx576mipiraw_COLOR_0480, imx576mipiraw_COLOR_0481, imx576mipiraw_COLOR_0482, imx576mipiraw_COLOR_0483, imx576mipiraw_COLOR_0484, imx576mipiraw_COLOR_0485, imx576mipiraw_COLOR_0486, imx576mipiraw_COLOR_0487, imx576mipiraw_COLOR_0488, imx576mipiraw_COLOR_0489,
    imx576mipiraw_COLOR_0490, imx576mipiraw_COLOR_0491, imx576mipiraw_COLOR_0492, imx576mipiraw_COLOR_0493, imx576mipiraw_COLOR_0494, imx576mipiraw_COLOR_0495, imx576mipiraw_COLOR_0496, imx576mipiraw_COLOR_0497, imx576mipiraw_COLOR_0498, imx576mipiraw_COLOR_0499,
    imx576mipiraw_COLOR_0500, imx576mipiraw_COLOR_0501, imx576mipiraw_COLOR_0502, imx576mipiraw_COLOR_0503, imx576mipiraw_COLOR_0504, imx576mipiraw_COLOR_0505, imx576mipiraw_COLOR_0506, imx576mipiraw_COLOR_0507, imx576mipiraw_COLOR_0508, imx576mipiraw_COLOR_0509,
    imx576mipiraw_COLOR_0510, imx576mipiraw_COLOR_0511, imx576mipiraw_COLOR_0512, imx576mipiraw_COLOR_0513, imx576mipiraw_COLOR_0514, imx576mipiraw_COLOR_0515, imx576mipiraw_COLOR_0516, imx576mipiraw_COLOR_0517, imx576mipiraw_COLOR_0518, imx576mipiraw_COLOR_0519,
    imx576mipiraw_COLOR_0520, imx576mipiraw_COLOR_0521, imx576mipiraw_COLOR_0522, imx576mipiraw_COLOR_0523, imx576mipiraw_COLOR_0524, imx576mipiraw_COLOR_0525, imx576mipiraw_COLOR_0526, imx576mipiraw_COLOR_0527, imx576mipiraw_COLOR_0528, imx576mipiraw_COLOR_0529,
    imx576mipiraw_COLOR_0530, imx576mipiraw_COLOR_0531, imx576mipiraw_COLOR_0532, imx576mipiraw_COLOR_0533, imx576mipiraw_COLOR_0534, imx576mipiraw_COLOR_0535, imx576mipiraw_COLOR_0536, imx576mipiraw_COLOR_0537, imx576mipiraw_COLOR_0538, imx576mipiraw_COLOR_0539,
    imx576mipiraw_COLOR_0540, imx576mipiraw_COLOR_0541, imx576mipiraw_COLOR_0542, imx576mipiraw_COLOR_0543, imx576mipiraw_COLOR_0544, imx576mipiraw_COLOR_0545, imx576mipiraw_COLOR_0546, imx576mipiraw_COLOR_0547, imx576mipiraw_COLOR_0548, imx576mipiraw_COLOR_0549,
    imx576mipiraw_COLOR_0550, imx576mipiraw_COLOR_0551, imx576mipiraw_COLOR_0552, imx576mipiraw_COLOR_0553, imx576mipiraw_COLOR_0554, imx576mipiraw_COLOR_0555, imx576mipiraw_COLOR_0556, imx576mipiraw_COLOR_0557, imx576mipiraw_COLOR_0558, imx576mipiraw_COLOR_0559,
    imx576mipiraw_COLOR_0560, imx576mipiraw_COLOR_0561, imx576mipiraw_COLOR_0562, imx576mipiraw_COLOR_0563, imx576mipiraw_COLOR_0564, imx576mipiraw_COLOR_0565, imx576mipiraw_COLOR_0566, imx576mipiraw_COLOR_0567, imx576mipiraw_COLOR_0568, imx576mipiraw_COLOR_0569,
    imx576mipiraw_COLOR_0570, imx576mipiraw_COLOR_0571, imx576mipiraw_COLOR_0572, imx576mipiraw_COLOR_0573, imx576mipiraw_COLOR_0574, imx576mipiraw_COLOR_0575, imx576mipiraw_COLOR_0576, imx576mipiraw_COLOR_0577, imx576mipiraw_COLOR_0578, imx576mipiraw_COLOR_0579,
    imx576mipiraw_COLOR_0580, imx576mipiraw_COLOR_0581, imx576mipiraw_COLOR_0582, imx576mipiraw_COLOR_0583, imx576mipiraw_COLOR_0584, imx576mipiraw_COLOR_0585, imx576mipiraw_COLOR_0586, imx576mipiraw_COLOR_0587, imx576mipiraw_COLOR_0588, imx576mipiraw_COLOR_0589,
    imx576mipiraw_COLOR_0590, imx576mipiraw_COLOR_0591, imx576mipiraw_COLOR_0592, imx576mipiraw_COLOR_0593, imx576mipiraw_COLOR_0594, imx576mipiraw_COLOR_0595, imx576mipiraw_COLOR_0596, imx576mipiraw_COLOR_0597, imx576mipiraw_COLOR_0598, imx576mipiraw_COLOR_0599,
    imx576mipiraw_COLOR_0600, imx576mipiraw_COLOR_0601, imx576mipiraw_COLOR_0602, imx576mipiraw_COLOR_0603, imx576mipiraw_COLOR_0604, imx576mipiraw_COLOR_0605, imx576mipiraw_COLOR_0606, imx576mipiraw_COLOR_0607, imx576mipiraw_COLOR_0608, imx576mipiraw_COLOR_0609,
    imx576mipiraw_COLOR_0610, imx576mipiraw_COLOR_0611, imx576mipiraw_COLOR_0612, imx576mipiraw_COLOR_0613, imx576mipiraw_COLOR_0614, imx576mipiraw_COLOR_0615, imx576mipiraw_COLOR_0616, imx576mipiraw_COLOR_0617, imx576mipiraw_COLOR_0618, imx576mipiraw_COLOR_0619,
    imx576mipiraw_COLOR_0620, imx576mipiraw_COLOR_0621, imx576mipiraw_COLOR_0622, imx576mipiraw_COLOR_0623, imx576mipiraw_COLOR_0624, imx576mipiraw_COLOR_0625, imx576mipiraw_COLOR_0626, imx576mipiraw_COLOR_0627, imx576mipiraw_COLOR_0628, imx576mipiraw_COLOR_0629,
    imx576mipiraw_COLOR_0630, imx576mipiraw_COLOR_0631, imx576mipiraw_COLOR_0632, imx576mipiraw_COLOR_0633, imx576mipiraw_COLOR_0634, imx576mipiraw_COLOR_0635, imx576mipiraw_COLOR_0636, imx576mipiraw_COLOR_0637, imx576mipiraw_COLOR_0638, imx576mipiraw_COLOR_0639,
    imx576mipiraw_COLOR_0640, imx576mipiraw_COLOR_0641, imx576mipiraw_COLOR_0642, imx576mipiraw_COLOR_0643, imx576mipiraw_COLOR_0644, imx576mipiraw_COLOR_0645, imx576mipiraw_COLOR_0646, imx576mipiraw_COLOR_0647, imx576mipiraw_COLOR_0648, imx576mipiraw_COLOR_0649,
    imx576mipiraw_COLOR_0650, imx576mipiraw_COLOR_0651, imx576mipiraw_COLOR_0652, imx576mipiraw_COLOR_0653, imx576mipiraw_COLOR_0654, imx576mipiraw_COLOR_0655, imx576mipiraw_COLOR_0656, imx576mipiraw_COLOR_0657, imx576mipiraw_COLOR_0658, imx576mipiraw_COLOR_0659,
    imx576mipiraw_COLOR_0660, imx576mipiraw_COLOR_0661, imx576mipiraw_COLOR_0662, imx576mipiraw_COLOR_0663, imx576mipiraw_COLOR_0664, imx576mipiraw_COLOR_0665, imx576mipiraw_COLOR_0666, imx576mipiraw_COLOR_0667, imx576mipiraw_COLOR_0668, imx576mipiraw_COLOR_0669,
    imx576mipiraw_COLOR_0670, imx576mipiraw_COLOR_0671, imx576mipiraw_COLOR_0672, imx576mipiraw_COLOR_0673, imx576mipiraw_COLOR_0674, imx576mipiraw_COLOR_0675, imx576mipiraw_COLOR_0676, imx576mipiraw_COLOR_0677, imx576mipiraw_COLOR_0678, imx576mipiraw_COLOR_0679,
    imx576mipiraw_COLOR_0680, imx576mipiraw_COLOR_0681, imx576mipiraw_COLOR_0682, imx576mipiraw_COLOR_0683, imx576mipiraw_COLOR_0684, imx576mipiraw_COLOR_0685, imx576mipiraw_COLOR_0686, imx576mipiraw_COLOR_0687, imx576mipiraw_COLOR_0688, imx576mipiraw_COLOR_0689,
    imx576mipiraw_COLOR_0690, imx576mipiraw_COLOR_0691, imx576mipiraw_COLOR_0692, imx576mipiraw_COLOR_0693, imx576mipiraw_COLOR_0694, imx576mipiraw_COLOR_0695, imx576mipiraw_COLOR_0696, imx576mipiraw_COLOR_0697, imx576mipiraw_COLOR_0698, imx576mipiraw_COLOR_0699,
    imx576mipiraw_COLOR_0700, imx576mipiraw_COLOR_0701, imx576mipiraw_COLOR_0702, imx576mipiraw_COLOR_0703, imx576mipiraw_COLOR_0704, imx576mipiraw_COLOR_0705, imx576mipiraw_COLOR_0706, imx576mipiraw_COLOR_0707, imx576mipiraw_COLOR_0708, imx576mipiraw_COLOR_0709,
    imx576mipiraw_COLOR_0710, imx576mipiraw_COLOR_0711, imx576mipiraw_COLOR_0712, imx576mipiraw_COLOR_0713, imx576mipiraw_COLOR_0714, imx576mipiraw_COLOR_0715, imx576mipiraw_COLOR_0716, imx576mipiraw_COLOR_0717, imx576mipiraw_COLOR_0718, imx576mipiraw_COLOR_0719,
    imx576mipiraw_COLOR_0720, imx576mipiraw_COLOR_0721, imx576mipiraw_COLOR_0722, imx576mipiraw_COLOR_0723, imx576mipiraw_COLOR_0724, imx576mipiraw_COLOR_0725, imx576mipiraw_COLOR_0726, imx576mipiraw_COLOR_0727, imx576mipiraw_COLOR_0728, imx576mipiraw_COLOR_0729,
    imx576mipiraw_COLOR_0730, imx576mipiraw_COLOR_0731, imx576mipiraw_COLOR_0732, imx576mipiraw_COLOR_0733, imx576mipiraw_COLOR_0734, imx576mipiraw_COLOR_0735, imx576mipiraw_COLOR_0736, imx576mipiraw_COLOR_0737, imx576mipiraw_COLOR_0738, imx576mipiraw_COLOR_0739,
    imx576mipiraw_COLOR_0740, imx576mipiraw_COLOR_0741, imx576mipiraw_COLOR_0742, imx576mipiraw_COLOR_0743, imx576mipiraw_COLOR_0744, imx576mipiraw_COLOR_0745, imx576mipiraw_COLOR_0746, imx576mipiraw_COLOR_0747, imx576mipiraw_COLOR_0748, imx576mipiraw_COLOR_0749,
    imx576mipiraw_COLOR_0750, imx576mipiraw_COLOR_0751, imx576mipiraw_COLOR_0752, imx576mipiraw_COLOR_0753, imx576mipiraw_COLOR_0754, imx576mipiraw_COLOR_0755, imx576mipiraw_COLOR_0756, imx576mipiraw_COLOR_0757, imx576mipiraw_COLOR_0758, imx576mipiraw_COLOR_0759,
    imx576mipiraw_COLOR_0760, imx576mipiraw_COLOR_0761, imx576mipiraw_COLOR_0762, imx576mipiraw_COLOR_0763, imx576mipiraw_COLOR_0764, imx576mipiraw_COLOR_0765, imx576mipiraw_COLOR_0766, imx576mipiraw_COLOR_0767, imx576mipiraw_COLOR_0768, imx576mipiraw_COLOR_0769,
    imx576mipiraw_COLOR_0770, imx576mipiraw_COLOR_0771, imx576mipiraw_COLOR_0772, imx576mipiraw_COLOR_0773, imx576mipiraw_COLOR_0774, imx576mipiraw_COLOR_0775, imx576mipiraw_COLOR_0776, imx576mipiraw_COLOR_0777, imx576mipiraw_COLOR_0778, imx576mipiraw_COLOR_0779,
    imx576mipiraw_COLOR_0780, imx576mipiraw_COLOR_0781, imx576mipiraw_COLOR_0782, imx576mipiraw_COLOR_0783, imx576mipiraw_COLOR_0784, imx576mipiraw_COLOR_0785, imx576mipiraw_COLOR_0786, imx576mipiraw_COLOR_0787, imx576mipiraw_COLOR_0788, imx576mipiraw_COLOR_0789,
    imx576mipiraw_COLOR_0790, imx576mipiraw_COLOR_0791, imx576mipiraw_COLOR_0792, imx576mipiraw_COLOR_0793, imx576mipiraw_COLOR_0794, imx576mipiraw_COLOR_0795, imx576mipiraw_COLOR_0796, imx576mipiraw_COLOR_0797, imx576mipiraw_COLOR_0798, imx576mipiraw_COLOR_0799,
    imx576mipiraw_COLOR_0800, imx576mipiraw_COLOR_0801, imx576mipiraw_COLOR_0802, imx576mipiraw_COLOR_0803, imx576mipiraw_COLOR_0804, imx576mipiraw_COLOR_0805, imx576mipiraw_COLOR_0806, imx576mipiraw_COLOR_0807, imx576mipiraw_COLOR_0808, imx576mipiraw_COLOR_0809,
    imx576mipiraw_COLOR_0810, imx576mipiraw_COLOR_0811, imx576mipiraw_COLOR_0812, imx576mipiraw_COLOR_0813, imx576mipiraw_COLOR_0814, imx576mipiraw_COLOR_0815, imx576mipiraw_COLOR_0816, imx576mipiraw_COLOR_0817, imx576mipiraw_COLOR_0818, imx576mipiraw_COLOR_0819,
    imx576mipiraw_COLOR_0820, imx576mipiraw_COLOR_0821, imx576mipiraw_COLOR_0822, imx576mipiraw_COLOR_0823, imx576mipiraw_COLOR_0824, imx576mipiraw_COLOR_0825, imx576mipiraw_COLOR_0826, imx576mipiraw_COLOR_0827, imx576mipiraw_COLOR_0828, imx576mipiraw_COLOR_0829,
    imx576mipiraw_COLOR_0830, imx576mipiraw_COLOR_0831, imx576mipiraw_COLOR_0832, imx576mipiraw_COLOR_0833, imx576mipiraw_COLOR_0834, imx576mipiraw_COLOR_0835, imx576mipiraw_COLOR_0836, imx576mipiraw_COLOR_0837, imx576mipiraw_COLOR_0838, imx576mipiraw_COLOR_0839,
    imx576mipiraw_COLOR_0840, imx576mipiraw_COLOR_0841, imx576mipiraw_COLOR_0842, imx576mipiraw_COLOR_0843, imx576mipiraw_COLOR_0844, imx576mipiraw_COLOR_0845, imx576mipiraw_COLOR_0846, imx576mipiraw_COLOR_0847, imx576mipiraw_COLOR_0848, imx576mipiraw_COLOR_0849,
    imx576mipiraw_COLOR_0850, imx576mipiraw_COLOR_0851, imx576mipiraw_COLOR_0852, imx576mipiraw_COLOR_0853, imx576mipiraw_COLOR_0854, imx576mipiraw_COLOR_0855, imx576mipiraw_COLOR_0856, imx576mipiraw_COLOR_0857, imx576mipiraw_COLOR_0858, imx576mipiraw_COLOR_0859,
    imx576mipiraw_COLOR_0860, imx576mipiraw_COLOR_0861, imx576mipiraw_COLOR_0862, imx576mipiraw_COLOR_0863, imx576mipiraw_COLOR_0864, imx576mipiraw_COLOR_0865, imx576mipiraw_COLOR_0866, imx576mipiraw_COLOR_0867, imx576mipiraw_COLOR_0868, imx576mipiraw_COLOR_0869,
    imx576mipiraw_COLOR_0870, imx576mipiraw_COLOR_0871, imx576mipiraw_COLOR_0872, imx576mipiraw_COLOR_0873, imx576mipiraw_COLOR_0874, imx576mipiraw_COLOR_0875, imx576mipiraw_COLOR_0876, imx576mipiraw_COLOR_0877, imx576mipiraw_COLOR_0878, imx576mipiraw_COLOR_0879,
    imx576mipiraw_COLOR_0880, imx576mipiraw_COLOR_0881, imx576mipiraw_COLOR_0882, imx576mipiraw_COLOR_0883, imx576mipiraw_COLOR_0884, imx576mipiraw_COLOR_0885, imx576mipiraw_COLOR_0886, imx576mipiraw_COLOR_0887, imx576mipiraw_COLOR_0888, imx576mipiraw_COLOR_0889,
    imx576mipiraw_COLOR_0890, imx576mipiraw_COLOR_0891, imx576mipiraw_COLOR_0892, imx576mipiraw_COLOR_0893, imx576mipiraw_COLOR_0894, imx576mipiraw_COLOR_0895, imx576mipiraw_COLOR_0896, imx576mipiraw_COLOR_0897, imx576mipiraw_COLOR_0898, imx576mipiraw_COLOR_0899,
    imx576mipiraw_COLOR_0900, imx576mipiraw_COLOR_0901, imx576mipiraw_COLOR_0902, imx576mipiraw_COLOR_0903, imx576mipiraw_COLOR_0904, imx576mipiraw_COLOR_0905, imx576mipiraw_COLOR_0906, imx576mipiraw_COLOR_0907, imx576mipiraw_COLOR_0908, imx576mipiraw_COLOR_0909,
    imx576mipiraw_COLOR_0910, imx576mipiraw_COLOR_0911, imx576mipiraw_COLOR_0912, imx576mipiraw_COLOR_0913, imx576mipiraw_COLOR_0914, imx576mipiraw_COLOR_0915, imx576mipiraw_COLOR_0916, imx576mipiraw_COLOR_0917, imx576mipiraw_COLOR_0918, imx576mipiraw_COLOR_0919,
    imx576mipiraw_COLOR_0920, imx576mipiraw_COLOR_0921, imx576mipiraw_COLOR_0922, imx576mipiraw_COLOR_0923, imx576mipiraw_COLOR_0924, imx576mipiraw_COLOR_0925, imx576mipiraw_COLOR_0926, imx576mipiraw_COLOR_0927, imx576mipiraw_COLOR_0928, imx576mipiraw_COLOR_0929,
    imx576mipiraw_COLOR_0930, imx576mipiraw_COLOR_0931, imx576mipiraw_COLOR_0932, imx576mipiraw_COLOR_0933, imx576mipiraw_COLOR_0934, imx576mipiraw_COLOR_0935, imx576mipiraw_COLOR_0936, imx576mipiraw_COLOR_0937, imx576mipiraw_COLOR_0938, imx576mipiraw_COLOR_0939,
    imx576mipiraw_COLOR_0940, imx576mipiraw_COLOR_0941, imx576mipiraw_COLOR_0942, imx576mipiraw_COLOR_0943, imx576mipiraw_COLOR_0944, imx576mipiraw_COLOR_0945, imx576mipiraw_COLOR_0946, imx576mipiraw_COLOR_0947, imx576mipiraw_COLOR_0948, imx576mipiraw_COLOR_0949,
    imx576mipiraw_COLOR_0950, imx576mipiraw_COLOR_0951, imx576mipiraw_COLOR_0952, imx576mipiraw_COLOR_0953, imx576mipiraw_COLOR_0954, imx576mipiraw_COLOR_0955, imx576mipiraw_COLOR_0956, imx576mipiraw_COLOR_0957, imx576mipiraw_COLOR_0958, imx576mipiraw_COLOR_0959,
},
.PCA = {
    imx576mipiraw_PCA_0000,
},
.COLOR_PARAM = {
    imx576mipiraw_COLOR_PARAM_0000, imx576mipiraw_COLOR_PARAM_0001, imx576mipiraw_COLOR_PARAM_0002, imx576mipiraw_COLOR_PARAM_0003, imx576mipiraw_COLOR_PARAM_0004, imx576mipiraw_COLOR_PARAM_0005, imx576mipiraw_COLOR_PARAM_0006, imx576mipiraw_COLOR_PARAM_0007, imx576mipiraw_COLOR_PARAM_0008, imx576mipiraw_COLOR_PARAM_0009,
    imx576mipiraw_COLOR_PARAM_0010, imx576mipiraw_COLOR_PARAM_0011, imx576mipiraw_COLOR_PARAM_0012, imx576mipiraw_COLOR_PARAM_0013, imx576mipiraw_COLOR_PARAM_0014, imx576mipiraw_COLOR_PARAM_0015,
},
