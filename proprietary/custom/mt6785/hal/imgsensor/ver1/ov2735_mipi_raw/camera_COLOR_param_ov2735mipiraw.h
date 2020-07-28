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
    ov2735mipiraw_CCM_0000, ov2735mipiraw_CCM_0001, ov2735mipiraw_CCM_0002, ov2735mipiraw_CCM_0003, ov2735mipiraw_CCM_0004, ov2735mipiraw_CCM_0005, ov2735mipiraw_CCM_0006, ov2735mipiraw_CCM_0007, ov2735mipiraw_CCM_0008, ov2735mipiraw_CCM_0009,
    ov2735mipiraw_CCM_0010, ov2735mipiraw_CCM_0011, ov2735mipiraw_CCM_0012, ov2735mipiraw_CCM_0013, ov2735mipiraw_CCM_0014, ov2735mipiraw_CCM_0015, ov2735mipiraw_CCM_0016, ov2735mipiraw_CCM_0017, ov2735mipiraw_CCM_0018, ov2735mipiraw_CCM_0019,
    ov2735mipiraw_CCM_0020, ov2735mipiraw_CCM_0021, ov2735mipiraw_CCM_0022, ov2735mipiraw_CCM_0023, ov2735mipiraw_CCM_0024, ov2735mipiraw_CCM_0025, ov2735mipiraw_CCM_0026, ov2735mipiraw_CCM_0027, ov2735mipiraw_CCM_0028, ov2735mipiraw_CCM_0029,
    ov2735mipiraw_CCM_0030, ov2735mipiraw_CCM_0031, ov2735mipiraw_CCM_0032, ov2735mipiraw_CCM_0033, ov2735mipiraw_CCM_0034, ov2735mipiraw_CCM_0035, ov2735mipiraw_CCM_0036, ov2735mipiraw_CCM_0037, ov2735mipiraw_CCM_0038, ov2735mipiraw_CCM_0039,
    ov2735mipiraw_CCM_0040, ov2735mipiraw_CCM_0041, ov2735mipiraw_CCM_0042, ov2735mipiraw_CCM_0043, ov2735mipiraw_CCM_0044, ov2735mipiraw_CCM_0045, ov2735mipiraw_CCM_0046, ov2735mipiraw_CCM_0047, ov2735mipiraw_CCM_0048, ov2735mipiraw_CCM_0049,
    ov2735mipiraw_CCM_0050, ov2735mipiraw_CCM_0051, ov2735mipiraw_CCM_0052, ov2735mipiraw_CCM_0053, ov2735mipiraw_CCM_0054, ov2735mipiraw_CCM_0055, ov2735mipiraw_CCM_0056, ov2735mipiraw_CCM_0057, ov2735mipiraw_CCM_0058, ov2735mipiraw_CCM_0059,
    ov2735mipiraw_CCM_0060, ov2735mipiraw_CCM_0061, ov2735mipiraw_CCM_0062, ov2735mipiraw_CCM_0063, ov2735mipiraw_CCM_0064, ov2735mipiraw_CCM_0065, ov2735mipiraw_CCM_0066, ov2735mipiraw_CCM_0067, ov2735mipiraw_CCM_0068, ov2735mipiraw_CCM_0069,
    ov2735mipiraw_CCM_0070, ov2735mipiraw_CCM_0071, ov2735mipiraw_CCM_0072, ov2735mipiraw_CCM_0073, ov2735mipiraw_CCM_0074, ov2735mipiraw_CCM_0075, ov2735mipiraw_CCM_0076, ov2735mipiraw_CCM_0077, ov2735mipiraw_CCM_0078, ov2735mipiraw_CCM_0079,
    ov2735mipiraw_CCM_0080, ov2735mipiraw_CCM_0081, ov2735mipiraw_CCM_0082, ov2735mipiraw_CCM_0083, ov2735mipiraw_CCM_0084, ov2735mipiraw_CCM_0085, ov2735mipiraw_CCM_0086, ov2735mipiraw_CCM_0087, ov2735mipiraw_CCM_0088, ov2735mipiraw_CCM_0089,
    ov2735mipiraw_CCM_0090, ov2735mipiraw_CCM_0091, ov2735mipiraw_CCM_0092, ov2735mipiraw_CCM_0093, ov2735mipiraw_CCM_0094, ov2735mipiraw_CCM_0095, ov2735mipiraw_CCM_0096, ov2735mipiraw_CCM_0097, ov2735mipiraw_CCM_0098, ov2735mipiraw_CCM_0099,
    ov2735mipiraw_CCM_0100, ov2735mipiraw_CCM_0101, ov2735mipiraw_CCM_0102, ov2735mipiraw_CCM_0103, ov2735mipiraw_CCM_0104, ov2735mipiraw_CCM_0105, ov2735mipiraw_CCM_0106, ov2735mipiraw_CCM_0107, ov2735mipiraw_CCM_0108, ov2735mipiraw_CCM_0109,
    ov2735mipiraw_CCM_0110, ov2735mipiraw_CCM_0111, ov2735mipiraw_CCM_0112, ov2735mipiraw_CCM_0113, ov2735mipiraw_CCM_0114, ov2735mipiraw_CCM_0115, ov2735mipiraw_CCM_0116, ov2735mipiraw_CCM_0117, ov2735mipiraw_CCM_0118, ov2735mipiraw_CCM_0119,
    ov2735mipiraw_CCM_0120, ov2735mipiraw_CCM_0121, ov2735mipiraw_CCM_0122, ov2735mipiraw_CCM_0123, ov2735mipiraw_CCM_0124, ov2735mipiraw_CCM_0125, ov2735mipiraw_CCM_0126, ov2735mipiraw_CCM_0127, ov2735mipiraw_CCM_0128, ov2735mipiraw_CCM_0129,
    ov2735mipiraw_CCM_0130, ov2735mipiraw_CCM_0131, ov2735mipiraw_CCM_0132, ov2735mipiraw_CCM_0133, ov2735mipiraw_CCM_0134, ov2735mipiraw_CCM_0135, ov2735mipiraw_CCM_0136, ov2735mipiraw_CCM_0137, ov2735mipiraw_CCM_0138, ov2735mipiraw_CCM_0139,
    ov2735mipiraw_CCM_0140, ov2735mipiraw_CCM_0141, ov2735mipiraw_CCM_0142, ov2735mipiraw_CCM_0143, ov2735mipiraw_CCM_0144, ov2735mipiraw_CCM_0145, ov2735mipiraw_CCM_0146, ov2735mipiraw_CCM_0147, ov2735mipiraw_CCM_0148, ov2735mipiraw_CCM_0149,
    ov2735mipiraw_CCM_0150, ov2735mipiraw_CCM_0151, ov2735mipiraw_CCM_0152, ov2735mipiraw_CCM_0153, ov2735mipiraw_CCM_0154, ov2735mipiraw_CCM_0155, ov2735mipiraw_CCM_0156, ov2735mipiraw_CCM_0157, ov2735mipiraw_CCM_0158, ov2735mipiraw_CCM_0159,
    ov2735mipiraw_CCM_0160, ov2735mipiraw_CCM_0161, ov2735mipiraw_CCM_0162, ov2735mipiraw_CCM_0163, ov2735mipiraw_CCM_0164, ov2735mipiraw_CCM_0165, ov2735mipiraw_CCM_0166, ov2735mipiraw_CCM_0167, ov2735mipiraw_CCM_0168, ov2735mipiraw_CCM_0169,
    ov2735mipiraw_CCM_0170, ov2735mipiraw_CCM_0171, ov2735mipiraw_CCM_0172, ov2735mipiraw_CCM_0173, ov2735mipiraw_CCM_0174, ov2735mipiraw_CCM_0175, ov2735mipiraw_CCM_0176, ov2735mipiraw_CCM_0177, ov2735mipiraw_CCM_0178, ov2735mipiraw_CCM_0179,
    ov2735mipiraw_CCM_0180, ov2735mipiraw_CCM_0181, ov2735mipiraw_CCM_0182, ov2735mipiraw_CCM_0183, ov2735mipiraw_CCM_0184, ov2735mipiraw_CCM_0185, ov2735mipiraw_CCM_0186, ov2735mipiraw_CCM_0187, ov2735mipiraw_CCM_0188, ov2735mipiraw_CCM_0189,
    ov2735mipiraw_CCM_0190, ov2735mipiraw_CCM_0191, ov2735mipiraw_CCM_0192, ov2735mipiraw_CCM_0193, ov2735mipiraw_CCM_0194, ov2735mipiraw_CCM_0195, ov2735mipiraw_CCM_0196, ov2735mipiraw_CCM_0197, ov2735mipiraw_CCM_0198, ov2735mipiraw_CCM_0199,
    ov2735mipiraw_CCM_0200, ov2735mipiraw_CCM_0201, ov2735mipiraw_CCM_0202, ov2735mipiraw_CCM_0203, ov2735mipiraw_CCM_0204, ov2735mipiraw_CCM_0205, ov2735mipiraw_CCM_0206, ov2735mipiraw_CCM_0207, ov2735mipiraw_CCM_0208, ov2735mipiraw_CCM_0209,
    ov2735mipiraw_CCM_0210, ov2735mipiraw_CCM_0211, ov2735mipiraw_CCM_0212, ov2735mipiraw_CCM_0213, ov2735mipiraw_CCM_0214, ov2735mipiraw_CCM_0215, ov2735mipiraw_CCM_0216, ov2735mipiraw_CCM_0217, ov2735mipiraw_CCM_0218, ov2735mipiraw_CCM_0219,
    ov2735mipiraw_CCM_0220, ov2735mipiraw_CCM_0221, ov2735mipiraw_CCM_0222, ov2735mipiraw_CCM_0223, ov2735mipiraw_CCM_0224, ov2735mipiraw_CCM_0225, ov2735mipiraw_CCM_0226, ov2735mipiraw_CCM_0227, ov2735mipiraw_CCM_0228, ov2735mipiraw_CCM_0229,
    ov2735mipiraw_CCM_0230, ov2735mipiraw_CCM_0231, ov2735mipiraw_CCM_0232, ov2735mipiraw_CCM_0233, ov2735mipiraw_CCM_0234, ov2735mipiraw_CCM_0235, ov2735mipiraw_CCM_0236, ov2735mipiraw_CCM_0237, ov2735mipiraw_CCM_0238, ov2735mipiraw_CCM_0239,
},
.COLOR = {
    ov2735mipiraw_COLOR_0000, ov2735mipiraw_COLOR_0001, ov2735mipiraw_COLOR_0002, ov2735mipiraw_COLOR_0003, ov2735mipiraw_COLOR_0004, ov2735mipiraw_COLOR_0005, ov2735mipiraw_COLOR_0006, ov2735mipiraw_COLOR_0007, ov2735mipiraw_COLOR_0008, ov2735mipiraw_COLOR_0009,
    ov2735mipiraw_COLOR_0010, ov2735mipiraw_COLOR_0011, ov2735mipiraw_COLOR_0012, ov2735mipiraw_COLOR_0013, ov2735mipiraw_COLOR_0014, ov2735mipiraw_COLOR_0015, ov2735mipiraw_COLOR_0016, ov2735mipiraw_COLOR_0017, ov2735mipiraw_COLOR_0018, ov2735mipiraw_COLOR_0019,
    ov2735mipiraw_COLOR_0020, ov2735mipiraw_COLOR_0021, ov2735mipiraw_COLOR_0022, ov2735mipiraw_COLOR_0023, ov2735mipiraw_COLOR_0024, ov2735mipiraw_COLOR_0025, ov2735mipiraw_COLOR_0026, ov2735mipiraw_COLOR_0027, ov2735mipiraw_COLOR_0028, ov2735mipiraw_COLOR_0029,
    ov2735mipiraw_COLOR_0030, ov2735mipiraw_COLOR_0031, ov2735mipiraw_COLOR_0032, ov2735mipiraw_COLOR_0033, ov2735mipiraw_COLOR_0034, ov2735mipiraw_COLOR_0035, ov2735mipiraw_COLOR_0036, ov2735mipiraw_COLOR_0037, ov2735mipiraw_COLOR_0038, ov2735mipiraw_COLOR_0039,
    ov2735mipiraw_COLOR_0040, ov2735mipiraw_COLOR_0041, ov2735mipiraw_COLOR_0042, ov2735mipiraw_COLOR_0043, ov2735mipiraw_COLOR_0044, ov2735mipiraw_COLOR_0045, ov2735mipiraw_COLOR_0046, ov2735mipiraw_COLOR_0047, ov2735mipiraw_COLOR_0048, ov2735mipiraw_COLOR_0049,
    ov2735mipiraw_COLOR_0050, ov2735mipiraw_COLOR_0051, ov2735mipiraw_COLOR_0052, ov2735mipiraw_COLOR_0053, ov2735mipiraw_COLOR_0054, ov2735mipiraw_COLOR_0055, ov2735mipiraw_COLOR_0056, ov2735mipiraw_COLOR_0057, ov2735mipiraw_COLOR_0058, ov2735mipiraw_COLOR_0059,
    ov2735mipiraw_COLOR_0060, ov2735mipiraw_COLOR_0061, ov2735mipiraw_COLOR_0062, ov2735mipiraw_COLOR_0063, ov2735mipiraw_COLOR_0064, ov2735mipiraw_COLOR_0065, ov2735mipiraw_COLOR_0066, ov2735mipiraw_COLOR_0067, ov2735mipiraw_COLOR_0068, ov2735mipiraw_COLOR_0069,
    ov2735mipiraw_COLOR_0070, ov2735mipiraw_COLOR_0071, ov2735mipiraw_COLOR_0072, ov2735mipiraw_COLOR_0073, ov2735mipiraw_COLOR_0074, ov2735mipiraw_COLOR_0075, ov2735mipiraw_COLOR_0076, ov2735mipiraw_COLOR_0077, ov2735mipiraw_COLOR_0078, ov2735mipiraw_COLOR_0079,
    ov2735mipiraw_COLOR_0080, ov2735mipiraw_COLOR_0081, ov2735mipiraw_COLOR_0082, ov2735mipiraw_COLOR_0083, ov2735mipiraw_COLOR_0084, ov2735mipiraw_COLOR_0085, ov2735mipiraw_COLOR_0086, ov2735mipiraw_COLOR_0087, ov2735mipiraw_COLOR_0088, ov2735mipiraw_COLOR_0089,
    ov2735mipiraw_COLOR_0090, ov2735mipiraw_COLOR_0091, ov2735mipiraw_COLOR_0092, ov2735mipiraw_COLOR_0093, ov2735mipiraw_COLOR_0094, ov2735mipiraw_COLOR_0095, ov2735mipiraw_COLOR_0096, ov2735mipiraw_COLOR_0097, ov2735mipiraw_COLOR_0098, ov2735mipiraw_COLOR_0099,
    ov2735mipiraw_COLOR_0100, ov2735mipiraw_COLOR_0101, ov2735mipiraw_COLOR_0102, ov2735mipiraw_COLOR_0103, ov2735mipiraw_COLOR_0104, ov2735mipiraw_COLOR_0105, ov2735mipiraw_COLOR_0106, ov2735mipiraw_COLOR_0107, ov2735mipiraw_COLOR_0108, ov2735mipiraw_COLOR_0109,
    ov2735mipiraw_COLOR_0110, ov2735mipiraw_COLOR_0111, ov2735mipiraw_COLOR_0112, ov2735mipiraw_COLOR_0113, ov2735mipiraw_COLOR_0114, ov2735mipiraw_COLOR_0115, ov2735mipiraw_COLOR_0116, ov2735mipiraw_COLOR_0117, ov2735mipiraw_COLOR_0118, ov2735mipiraw_COLOR_0119,
    ov2735mipiraw_COLOR_0120, ov2735mipiraw_COLOR_0121, ov2735mipiraw_COLOR_0122, ov2735mipiraw_COLOR_0123, ov2735mipiraw_COLOR_0124, ov2735mipiraw_COLOR_0125, ov2735mipiraw_COLOR_0126, ov2735mipiraw_COLOR_0127, ov2735mipiraw_COLOR_0128, ov2735mipiraw_COLOR_0129,
    ov2735mipiraw_COLOR_0130, ov2735mipiraw_COLOR_0131, ov2735mipiraw_COLOR_0132, ov2735mipiraw_COLOR_0133, ov2735mipiraw_COLOR_0134, ov2735mipiraw_COLOR_0135, ov2735mipiraw_COLOR_0136, ov2735mipiraw_COLOR_0137, ov2735mipiraw_COLOR_0138, ov2735mipiraw_COLOR_0139,
    ov2735mipiraw_COLOR_0140, ov2735mipiraw_COLOR_0141, ov2735mipiraw_COLOR_0142, ov2735mipiraw_COLOR_0143, ov2735mipiraw_COLOR_0144, ov2735mipiraw_COLOR_0145, ov2735mipiraw_COLOR_0146, ov2735mipiraw_COLOR_0147, ov2735mipiraw_COLOR_0148, ov2735mipiraw_COLOR_0149,
    ov2735mipiraw_COLOR_0150, ov2735mipiraw_COLOR_0151, ov2735mipiraw_COLOR_0152, ov2735mipiraw_COLOR_0153, ov2735mipiraw_COLOR_0154, ov2735mipiraw_COLOR_0155, ov2735mipiraw_COLOR_0156, ov2735mipiraw_COLOR_0157, ov2735mipiraw_COLOR_0158, ov2735mipiraw_COLOR_0159,
    ov2735mipiraw_COLOR_0160, ov2735mipiraw_COLOR_0161, ov2735mipiraw_COLOR_0162, ov2735mipiraw_COLOR_0163, ov2735mipiraw_COLOR_0164, ov2735mipiraw_COLOR_0165, ov2735mipiraw_COLOR_0166, ov2735mipiraw_COLOR_0167, ov2735mipiraw_COLOR_0168, ov2735mipiraw_COLOR_0169,
    ov2735mipiraw_COLOR_0170, ov2735mipiraw_COLOR_0171, ov2735mipiraw_COLOR_0172, ov2735mipiraw_COLOR_0173, ov2735mipiraw_COLOR_0174, ov2735mipiraw_COLOR_0175, ov2735mipiraw_COLOR_0176, ov2735mipiraw_COLOR_0177, ov2735mipiraw_COLOR_0178, ov2735mipiraw_COLOR_0179,
    ov2735mipiraw_COLOR_0180, ov2735mipiraw_COLOR_0181, ov2735mipiraw_COLOR_0182, ov2735mipiraw_COLOR_0183, ov2735mipiraw_COLOR_0184, ov2735mipiraw_COLOR_0185, ov2735mipiraw_COLOR_0186, ov2735mipiraw_COLOR_0187, ov2735mipiraw_COLOR_0188, ov2735mipiraw_COLOR_0189,
    ov2735mipiraw_COLOR_0190, ov2735mipiraw_COLOR_0191, ov2735mipiraw_COLOR_0192, ov2735mipiraw_COLOR_0193, ov2735mipiraw_COLOR_0194, ov2735mipiraw_COLOR_0195, ov2735mipiraw_COLOR_0196, ov2735mipiraw_COLOR_0197, ov2735mipiraw_COLOR_0198, ov2735mipiraw_COLOR_0199,
    ov2735mipiraw_COLOR_0200, ov2735mipiraw_COLOR_0201, ov2735mipiraw_COLOR_0202, ov2735mipiraw_COLOR_0203, ov2735mipiraw_COLOR_0204, ov2735mipiraw_COLOR_0205, ov2735mipiraw_COLOR_0206, ov2735mipiraw_COLOR_0207, ov2735mipiraw_COLOR_0208, ov2735mipiraw_COLOR_0209,
    ov2735mipiraw_COLOR_0210, ov2735mipiraw_COLOR_0211, ov2735mipiraw_COLOR_0212, ov2735mipiraw_COLOR_0213, ov2735mipiraw_COLOR_0214, ov2735mipiraw_COLOR_0215, ov2735mipiraw_COLOR_0216, ov2735mipiraw_COLOR_0217, ov2735mipiraw_COLOR_0218, ov2735mipiraw_COLOR_0219,
    ov2735mipiraw_COLOR_0220, ov2735mipiraw_COLOR_0221, ov2735mipiraw_COLOR_0222, ov2735mipiraw_COLOR_0223, ov2735mipiraw_COLOR_0224, ov2735mipiraw_COLOR_0225, ov2735mipiraw_COLOR_0226, ov2735mipiraw_COLOR_0227, ov2735mipiraw_COLOR_0228, ov2735mipiraw_COLOR_0229,
    ov2735mipiraw_COLOR_0230, ov2735mipiraw_COLOR_0231, ov2735mipiraw_COLOR_0232, ov2735mipiraw_COLOR_0233, ov2735mipiraw_COLOR_0234, ov2735mipiraw_COLOR_0235, ov2735mipiraw_COLOR_0236, ov2735mipiraw_COLOR_0237, ov2735mipiraw_COLOR_0238, ov2735mipiraw_COLOR_0239,
    ov2735mipiraw_COLOR_0240, ov2735mipiraw_COLOR_0241, ov2735mipiraw_COLOR_0242, ov2735mipiraw_COLOR_0243, ov2735mipiraw_COLOR_0244, ov2735mipiraw_COLOR_0245, ov2735mipiraw_COLOR_0246, ov2735mipiraw_COLOR_0247, ov2735mipiraw_COLOR_0248, ov2735mipiraw_COLOR_0249,
    ov2735mipiraw_COLOR_0250, ov2735mipiraw_COLOR_0251, ov2735mipiraw_COLOR_0252, ov2735mipiraw_COLOR_0253, ov2735mipiraw_COLOR_0254, ov2735mipiraw_COLOR_0255, ov2735mipiraw_COLOR_0256, ov2735mipiraw_COLOR_0257, ov2735mipiraw_COLOR_0258, ov2735mipiraw_COLOR_0259,
    ov2735mipiraw_COLOR_0260, ov2735mipiraw_COLOR_0261, ov2735mipiraw_COLOR_0262, ov2735mipiraw_COLOR_0263, ov2735mipiraw_COLOR_0264, ov2735mipiraw_COLOR_0265, ov2735mipiraw_COLOR_0266, ov2735mipiraw_COLOR_0267, ov2735mipiraw_COLOR_0268, ov2735mipiraw_COLOR_0269,
    ov2735mipiraw_COLOR_0270, ov2735mipiraw_COLOR_0271, ov2735mipiraw_COLOR_0272, ov2735mipiraw_COLOR_0273, ov2735mipiraw_COLOR_0274, ov2735mipiraw_COLOR_0275, ov2735mipiraw_COLOR_0276, ov2735mipiraw_COLOR_0277, ov2735mipiraw_COLOR_0278, ov2735mipiraw_COLOR_0279,
    ov2735mipiraw_COLOR_0280, ov2735mipiraw_COLOR_0281, ov2735mipiraw_COLOR_0282, ov2735mipiraw_COLOR_0283, ov2735mipiraw_COLOR_0284, ov2735mipiraw_COLOR_0285, ov2735mipiraw_COLOR_0286, ov2735mipiraw_COLOR_0287, ov2735mipiraw_COLOR_0288, ov2735mipiraw_COLOR_0289,
    ov2735mipiraw_COLOR_0290, ov2735mipiraw_COLOR_0291, ov2735mipiraw_COLOR_0292, ov2735mipiraw_COLOR_0293, ov2735mipiraw_COLOR_0294, ov2735mipiraw_COLOR_0295, ov2735mipiraw_COLOR_0296, ov2735mipiraw_COLOR_0297, ov2735mipiraw_COLOR_0298, ov2735mipiraw_COLOR_0299,
    ov2735mipiraw_COLOR_0300, ov2735mipiraw_COLOR_0301, ov2735mipiraw_COLOR_0302, ov2735mipiraw_COLOR_0303, ov2735mipiraw_COLOR_0304, ov2735mipiraw_COLOR_0305, ov2735mipiraw_COLOR_0306, ov2735mipiraw_COLOR_0307, ov2735mipiraw_COLOR_0308, ov2735mipiraw_COLOR_0309,
    ov2735mipiraw_COLOR_0310, ov2735mipiraw_COLOR_0311, ov2735mipiraw_COLOR_0312, ov2735mipiraw_COLOR_0313, ov2735mipiraw_COLOR_0314, ov2735mipiraw_COLOR_0315, ov2735mipiraw_COLOR_0316, ov2735mipiraw_COLOR_0317, ov2735mipiraw_COLOR_0318, ov2735mipiraw_COLOR_0319,
    ov2735mipiraw_COLOR_0320, ov2735mipiraw_COLOR_0321, ov2735mipiraw_COLOR_0322, ov2735mipiraw_COLOR_0323, ov2735mipiraw_COLOR_0324, ov2735mipiraw_COLOR_0325, ov2735mipiraw_COLOR_0326, ov2735mipiraw_COLOR_0327, ov2735mipiraw_COLOR_0328, ov2735mipiraw_COLOR_0329,
    ov2735mipiraw_COLOR_0330, ov2735mipiraw_COLOR_0331, ov2735mipiraw_COLOR_0332, ov2735mipiraw_COLOR_0333, ov2735mipiraw_COLOR_0334, ov2735mipiraw_COLOR_0335, ov2735mipiraw_COLOR_0336, ov2735mipiraw_COLOR_0337, ov2735mipiraw_COLOR_0338, ov2735mipiraw_COLOR_0339,
    ov2735mipiraw_COLOR_0340, ov2735mipiraw_COLOR_0341, ov2735mipiraw_COLOR_0342, ov2735mipiraw_COLOR_0343, ov2735mipiraw_COLOR_0344, ov2735mipiraw_COLOR_0345, ov2735mipiraw_COLOR_0346, ov2735mipiraw_COLOR_0347, ov2735mipiraw_COLOR_0348, ov2735mipiraw_COLOR_0349,
    ov2735mipiraw_COLOR_0350, ov2735mipiraw_COLOR_0351, ov2735mipiraw_COLOR_0352, ov2735mipiraw_COLOR_0353, ov2735mipiraw_COLOR_0354, ov2735mipiraw_COLOR_0355, ov2735mipiraw_COLOR_0356, ov2735mipiraw_COLOR_0357, ov2735mipiraw_COLOR_0358, ov2735mipiraw_COLOR_0359,
    ov2735mipiraw_COLOR_0360, ov2735mipiraw_COLOR_0361, ov2735mipiraw_COLOR_0362, ov2735mipiraw_COLOR_0363, ov2735mipiraw_COLOR_0364, ov2735mipiraw_COLOR_0365, ov2735mipiraw_COLOR_0366, ov2735mipiraw_COLOR_0367, ov2735mipiraw_COLOR_0368, ov2735mipiraw_COLOR_0369,
    ov2735mipiraw_COLOR_0370, ov2735mipiraw_COLOR_0371, ov2735mipiraw_COLOR_0372, ov2735mipiraw_COLOR_0373, ov2735mipiraw_COLOR_0374, ov2735mipiraw_COLOR_0375, ov2735mipiraw_COLOR_0376, ov2735mipiraw_COLOR_0377, ov2735mipiraw_COLOR_0378, ov2735mipiraw_COLOR_0379,
    ov2735mipiraw_COLOR_0380, ov2735mipiraw_COLOR_0381, ov2735mipiraw_COLOR_0382, ov2735mipiraw_COLOR_0383, ov2735mipiraw_COLOR_0384, ov2735mipiraw_COLOR_0385, ov2735mipiraw_COLOR_0386, ov2735mipiraw_COLOR_0387, ov2735mipiraw_COLOR_0388, ov2735mipiraw_COLOR_0389,
    ov2735mipiraw_COLOR_0390, ov2735mipiraw_COLOR_0391, ov2735mipiraw_COLOR_0392, ov2735mipiraw_COLOR_0393, ov2735mipiraw_COLOR_0394, ov2735mipiraw_COLOR_0395, ov2735mipiraw_COLOR_0396, ov2735mipiraw_COLOR_0397, ov2735mipiraw_COLOR_0398, ov2735mipiraw_COLOR_0399,
    ov2735mipiraw_COLOR_0400, ov2735mipiraw_COLOR_0401, ov2735mipiraw_COLOR_0402, ov2735mipiraw_COLOR_0403, ov2735mipiraw_COLOR_0404, ov2735mipiraw_COLOR_0405, ov2735mipiraw_COLOR_0406, ov2735mipiraw_COLOR_0407, ov2735mipiraw_COLOR_0408, ov2735mipiraw_COLOR_0409,
    ov2735mipiraw_COLOR_0410, ov2735mipiraw_COLOR_0411, ov2735mipiraw_COLOR_0412, ov2735mipiraw_COLOR_0413, ov2735mipiraw_COLOR_0414, ov2735mipiraw_COLOR_0415, ov2735mipiraw_COLOR_0416, ov2735mipiraw_COLOR_0417, ov2735mipiraw_COLOR_0418, ov2735mipiraw_COLOR_0419,
    ov2735mipiraw_COLOR_0420, ov2735mipiraw_COLOR_0421, ov2735mipiraw_COLOR_0422, ov2735mipiraw_COLOR_0423, ov2735mipiraw_COLOR_0424, ov2735mipiraw_COLOR_0425, ov2735mipiraw_COLOR_0426, ov2735mipiraw_COLOR_0427, ov2735mipiraw_COLOR_0428, ov2735mipiraw_COLOR_0429,
    ov2735mipiraw_COLOR_0430, ov2735mipiraw_COLOR_0431, ov2735mipiraw_COLOR_0432, ov2735mipiraw_COLOR_0433, ov2735mipiraw_COLOR_0434, ov2735mipiraw_COLOR_0435, ov2735mipiraw_COLOR_0436, ov2735mipiraw_COLOR_0437, ov2735mipiraw_COLOR_0438, ov2735mipiraw_COLOR_0439,
    ov2735mipiraw_COLOR_0440, ov2735mipiraw_COLOR_0441, ov2735mipiraw_COLOR_0442, ov2735mipiraw_COLOR_0443, ov2735mipiraw_COLOR_0444, ov2735mipiraw_COLOR_0445, ov2735mipiraw_COLOR_0446, ov2735mipiraw_COLOR_0447, ov2735mipiraw_COLOR_0448, ov2735mipiraw_COLOR_0449,
    ov2735mipiraw_COLOR_0450, ov2735mipiraw_COLOR_0451, ov2735mipiraw_COLOR_0452, ov2735mipiraw_COLOR_0453, ov2735mipiraw_COLOR_0454, ov2735mipiraw_COLOR_0455, ov2735mipiraw_COLOR_0456, ov2735mipiraw_COLOR_0457, ov2735mipiraw_COLOR_0458, ov2735mipiraw_COLOR_0459,
    ov2735mipiraw_COLOR_0460, ov2735mipiraw_COLOR_0461, ov2735mipiraw_COLOR_0462, ov2735mipiraw_COLOR_0463, ov2735mipiraw_COLOR_0464, ov2735mipiraw_COLOR_0465, ov2735mipiraw_COLOR_0466, ov2735mipiraw_COLOR_0467, ov2735mipiraw_COLOR_0468, ov2735mipiraw_COLOR_0469,
    ov2735mipiraw_COLOR_0470, ov2735mipiraw_COLOR_0471, ov2735mipiraw_COLOR_0472, ov2735mipiraw_COLOR_0473, ov2735mipiraw_COLOR_0474, ov2735mipiraw_COLOR_0475, ov2735mipiraw_COLOR_0476, ov2735mipiraw_COLOR_0477, ov2735mipiraw_COLOR_0478, ov2735mipiraw_COLOR_0479,
    ov2735mipiraw_COLOR_0480, ov2735mipiraw_COLOR_0481, ov2735mipiraw_COLOR_0482, ov2735mipiraw_COLOR_0483, ov2735mipiraw_COLOR_0484, ov2735mipiraw_COLOR_0485, ov2735mipiraw_COLOR_0486, ov2735mipiraw_COLOR_0487, ov2735mipiraw_COLOR_0488, ov2735mipiraw_COLOR_0489,
    ov2735mipiraw_COLOR_0490, ov2735mipiraw_COLOR_0491, ov2735mipiraw_COLOR_0492, ov2735mipiraw_COLOR_0493, ov2735mipiraw_COLOR_0494, ov2735mipiraw_COLOR_0495, ov2735mipiraw_COLOR_0496, ov2735mipiraw_COLOR_0497, ov2735mipiraw_COLOR_0498, ov2735mipiraw_COLOR_0499,
    ov2735mipiraw_COLOR_0500, ov2735mipiraw_COLOR_0501, ov2735mipiraw_COLOR_0502, ov2735mipiraw_COLOR_0503, ov2735mipiraw_COLOR_0504, ov2735mipiraw_COLOR_0505, ov2735mipiraw_COLOR_0506, ov2735mipiraw_COLOR_0507, ov2735mipiraw_COLOR_0508, ov2735mipiraw_COLOR_0509,
    ov2735mipiraw_COLOR_0510, ov2735mipiraw_COLOR_0511, ov2735mipiraw_COLOR_0512, ov2735mipiraw_COLOR_0513, ov2735mipiraw_COLOR_0514, ov2735mipiraw_COLOR_0515, ov2735mipiraw_COLOR_0516, ov2735mipiraw_COLOR_0517, ov2735mipiraw_COLOR_0518, ov2735mipiraw_COLOR_0519,
    ov2735mipiraw_COLOR_0520, ov2735mipiraw_COLOR_0521, ov2735mipiraw_COLOR_0522, ov2735mipiraw_COLOR_0523, ov2735mipiraw_COLOR_0524, ov2735mipiraw_COLOR_0525, ov2735mipiraw_COLOR_0526, ov2735mipiraw_COLOR_0527, ov2735mipiraw_COLOR_0528, ov2735mipiraw_COLOR_0529,
    ov2735mipiraw_COLOR_0530, ov2735mipiraw_COLOR_0531, ov2735mipiraw_COLOR_0532, ov2735mipiraw_COLOR_0533, ov2735mipiraw_COLOR_0534, ov2735mipiraw_COLOR_0535, ov2735mipiraw_COLOR_0536, ov2735mipiraw_COLOR_0537, ov2735mipiraw_COLOR_0538, ov2735mipiraw_COLOR_0539,
    ov2735mipiraw_COLOR_0540, ov2735mipiraw_COLOR_0541, ov2735mipiraw_COLOR_0542, ov2735mipiraw_COLOR_0543, ov2735mipiraw_COLOR_0544, ov2735mipiraw_COLOR_0545, ov2735mipiraw_COLOR_0546, ov2735mipiraw_COLOR_0547, ov2735mipiraw_COLOR_0548, ov2735mipiraw_COLOR_0549,
    ov2735mipiraw_COLOR_0550, ov2735mipiraw_COLOR_0551, ov2735mipiraw_COLOR_0552, ov2735mipiraw_COLOR_0553, ov2735mipiraw_COLOR_0554, ov2735mipiraw_COLOR_0555, ov2735mipiraw_COLOR_0556, ov2735mipiraw_COLOR_0557, ov2735mipiraw_COLOR_0558, ov2735mipiraw_COLOR_0559,
    ov2735mipiraw_COLOR_0560, ov2735mipiraw_COLOR_0561, ov2735mipiraw_COLOR_0562, ov2735mipiraw_COLOR_0563, ov2735mipiraw_COLOR_0564, ov2735mipiraw_COLOR_0565, ov2735mipiraw_COLOR_0566, ov2735mipiraw_COLOR_0567, ov2735mipiraw_COLOR_0568, ov2735mipiraw_COLOR_0569,
    ov2735mipiraw_COLOR_0570, ov2735mipiraw_COLOR_0571, ov2735mipiraw_COLOR_0572, ov2735mipiraw_COLOR_0573, ov2735mipiraw_COLOR_0574, ov2735mipiraw_COLOR_0575, ov2735mipiraw_COLOR_0576, ov2735mipiraw_COLOR_0577, ov2735mipiraw_COLOR_0578, ov2735mipiraw_COLOR_0579,
    ov2735mipiraw_COLOR_0580, ov2735mipiraw_COLOR_0581, ov2735mipiraw_COLOR_0582, ov2735mipiraw_COLOR_0583, ov2735mipiraw_COLOR_0584, ov2735mipiraw_COLOR_0585, ov2735mipiraw_COLOR_0586, ov2735mipiraw_COLOR_0587, ov2735mipiraw_COLOR_0588, ov2735mipiraw_COLOR_0589,
    ov2735mipiraw_COLOR_0590, ov2735mipiraw_COLOR_0591, ov2735mipiraw_COLOR_0592, ov2735mipiraw_COLOR_0593, ov2735mipiraw_COLOR_0594, ov2735mipiraw_COLOR_0595, ov2735mipiraw_COLOR_0596, ov2735mipiraw_COLOR_0597, ov2735mipiraw_COLOR_0598, ov2735mipiraw_COLOR_0599,
    ov2735mipiraw_COLOR_0600, ov2735mipiraw_COLOR_0601, ov2735mipiraw_COLOR_0602, ov2735mipiraw_COLOR_0603, ov2735mipiraw_COLOR_0604, ov2735mipiraw_COLOR_0605, ov2735mipiraw_COLOR_0606, ov2735mipiraw_COLOR_0607, ov2735mipiraw_COLOR_0608, ov2735mipiraw_COLOR_0609,
    ov2735mipiraw_COLOR_0610, ov2735mipiraw_COLOR_0611, ov2735mipiraw_COLOR_0612, ov2735mipiraw_COLOR_0613, ov2735mipiraw_COLOR_0614, ov2735mipiraw_COLOR_0615, ov2735mipiraw_COLOR_0616, ov2735mipiraw_COLOR_0617, ov2735mipiraw_COLOR_0618, ov2735mipiraw_COLOR_0619,
    ov2735mipiraw_COLOR_0620, ov2735mipiraw_COLOR_0621, ov2735mipiraw_COLOR_0622, ov2735mipiraw_COLOR_0623, ov2735mipiraw_COLOR_0624, ov2735mipiraw_COLOR_0625, ov2735mipiraw_COLOR_0626, ov2735mipiraw_COLOR_0627, ov2735mipiraw_COLOR_0628, ov2735mipiraw_COLOR_0629,
    ov2735mipiraw_COLOR_0630, ov2735mipiraw_COLOR_0631, ov2735mipiraw_COLOR_0632, ov2735mipiraw_COLOR_0633, ov2735mipiraw_COLOR_0634, ov2735mipiraw_COLOR_0635, ov2735mipiraw_COLOR_0636, ov2735mipiraw_COLOR_0637, ov2735mipiraw_COLOR_0638, ov2735mipiraw_COLOR_0639,
    ov2735mipiraw_COLOR_0640, ov2735mipiraw_COLOR_0641, ov2735mipiraw_COLOR_0642, ov2735mipiraw_COLOR_0643, ov2735mipiraw_COLOR_0644, ov2735mipiraw_COLOR_0645, ov2735mipiraw_COLOR_0646, ov2735mipiraw_COLOR_0647, ov2735mipiraw_COLOR_0648, ov2735mipiraw_COLOR_0649,
    ov2735mipiraw_COLOR_0650, ov2735mipiraw_COLOR_0651, ov2735mipiraw_COLOR_0652, ov2735mipiraw_COLOR_0653, ov2735mipiraw_COLOR_0654, ov2735mipiraw_COLOR_0655, ov2735mipiraw_COLOR_0656, ov2735mipiraw_COLOR_0657, ov2735mipiraw_COLOR_0658, ov2735mipiraw_COLOR_0659,
    ov2735mipiraw_COLOR_0660, ov2735mipiraw_COLOR_0661, ov2735mipiraw_COLOR_0662, ov2735mipiraw_COLOR_0663, ov2735mipiraw_COLOR_0664, ov2735mipiraw_COLOR_0665, ov2735mipiraw_COLOR_0666, ov2735mipiraw_COLOR_0667, ov2735mipiraw_COLOR_0668, ov2735mipiraw_COLOR_0669,
    ov2735mipiraw_COLOR_0670, ov2735mipiraw_COLOR_0671, ov2735mipiraw_COLOR_0672, ov2735mipiraw_COLOR_0673, ov2735mipiraw_COLOR_0674, ov2735mipiraw_COLOR_0675, ov2735mipiraw_COLOR_0676, ov2735mipiraw_COLOR_0677, ov2735mipiraw_COLOR_0678, ov2735mipiraw_COLOR_0679,
    ov2735mipiraw_COLOR_0680, ov2735mipiraw_COLOR_0681, ov2735mipiraw_COLOR_0682, ov2735mipiraw_COLOR_0683, ov2735mipiraw_COLOR_0684, ov2735mipiraw_COLOR_0685, ov2735mipiraw_COLOR_0686, ov2735mipiraw_COLOR_0687, ov2735mipiraw_COLOR_0688, ov2735mipiraw_COLOR_0689,
    ov2735mipiraw_COLOR_0690, ov2735mipiraw_COLOR_0691, ov2735mipiraw_COLOR_0692, ov2735mipiraw_COLOR_0693, ov2735mipiraw_COLOR_0694, ov2735mipiraw_COLOR_0695, ov2735mipiraw_COLOR_0696, ov2735mipiraw_COLOR_0697, ov2735mipiraw_COLOR_0698, ov2735mipiraw_COLOR_0699,
    ov2735mipiraw_COLOR_0700, ov2735mipiraw_COLOR_0701, ov2735mipiraw_COLOR_0702, ov2735mipiraw_COLOR_0703, ov2735mipiraw_COLOR_0704, ov2735mipiraw_COLOR_0705, ov2735mipiraw_COLOR_0706, ov2735mipiraw_COLOR_0707, ov2735mipiraw_COLOR_0708, ov2735mipiraw_COLOR_0709,
    ov2735mipiraw_COLOR_0710, ov2735mipiraw_COLOR_0711, ov2735mipiraw_COLOR_0712, ov2735mipiraw_COLOR_0713, ov2735mipiraw_COLOR_0714, ov2735mipiraw_COLOR_0715, ov2735mipiraw_COLOR_0716, ov2735mipiraw_COLOR_0717, ov2735mipiraw_COLOR_0718, ov2735mipiraw_COLOR_0719,
    ov2735mipiraw_COLOR_0720, ov2735mipiraw_COLOR_0721, ov2735mipiraw_COLOR_0722, ov2735mipiraw_COLOR_0723, ov2735mipiraw_COLOR_0724, ov2735mipiraw_COLOR_0725, ov2735mipiraw_COLOR_0726, ov2735mipiraw_COLOR_0727, ov2735mipiraw_COLOR_0728, ov2735mipiraw_COLOR_0729,
    ov2735mipiraw_COLOR_0730, ov2735mipiraw_COLOR_0731, ov2735mipiraw_COLOR_0732, ov2735mipiraw_COLOR_0733, ov2735mipiraw_COLOR_0734, ov2735mipiraw_COLOR_0735, ov2735mipiraw_COLOR_0736, ov2735mipiraw_COLOR_0737, ov2735mipiraw_COLOR_0738, ov2735mipiraw_COLOR_0739,
    ov2735mipiraw_COLOR_0740, ov2735mipiraw_COLOR_0741, ov2735mipiraw_COLOR_0742, ov2735mipiraw_COLOR_0743, ov2735mipiraw_COLOR_0744, ov2735mipiraw_COLOR_0745, ov2735mipiraw_COLOR_0746, ov2735mipiraw_COLOR_0747, ov2735mipiraw_COLOR_0748, ov2735mipiraw_COLOR_0749,
    ov2735mipiraw_COLOR_0750, ov2735mipiraw_COLOR_0751, ov2735mipiraw_COLOR_0752, ov2735mipiraw_COLOR_0753, ov2735mipiraw_COLOR_0754, ov2735mipiraw_COLOR_0755, ov2735mipiraw_COLOR_0756, ov2735mipiraw_COLOR_0757, ov2735mipiraw_COLOR_0758, ov2735mipiraw_COLOR_0759,
    ov2735mipiraw_COLOR_0760, ov2735mipiraw_COLOR_0761, ov2735mipiraw_COLOR_0762, ov2735mipiraw_COLOR_0763, ov2735mipiraw_COLOR_0764, ov2735mipiraw_COLOR_0765, ov2735mipiraw_COLOR_0766, ov2735mipiraw_COLOR_0767, ov2735mipiraw_COLOR_0768, ov2735mipiraw_COLOR_0769,
    ov2735mipiraw_COLOR_0770, ov2735mipiraw_COLOR_0771, ov2735mipiraw_COLOR_0772, ov2735mipiraw_COLOR_0773, ov2735mipiraw_COLOR_0774, ov2735mipiraw_COLOR_0775, ov2735mipiraw_COLOR_0776, ov2735mipiraw_COLOR_0777, ov2735mipiraw_COLOR_0778, ov2735mipiraw_COLOR_0779,
    ov2735mipiraw_COLOR_0780, ov2735mipiraw_COLOR_0781, ov2735mipiraw_COLOR_0782, ov2735mipiraw_COLOR_0783, ov2735mipiraw_COLOR_0784, ov2735mipiraw_COLOR_0785, ov2735mipiraw_COLOR_0786, ov2735mipiraw_COLOR_0787, ov2735mipiraw_COLOR_0788, ov2735mipiraw_COLOR_0789,
    ov2735mipiraw_COLOR_0790, ov2735mipiraw_COLOR_0791, ov2735mipiraw_COLOR_0792, ov2735mipiraw_COLOR_0793, ov2735mipiraw_COLOR_0794, ov2735mipiraw_COLOR_0795, ov2735mipiraw_COLOR_0796, ov2735mipiraw_COLOR_0797, ov2735mipiraw_COLOR_0798, ov2735mipiraw_COLOR_0799,
    ov2735mipiraw_COLOR_0800, ov2735mipiraw_COLOR_0801, ov2735mipiraw_COLOR_0802, ov2735mipiraw_COLOR_0803, ov2735mipiraw_COLOR_0804, ov2735mipiraw_COLOR_0805, ov2735mipiraw_COLOR_0806, ov2735mipiraw_COLOR_0807, ov2735mipiraw_COLOR_0808, ov2735mipiraw_COLOR_0809,
    ov2735mipiraw_COLOR_0810, ov2735mipiraw_COLOR_0811, ov2735mipiraw_COLOR_0812, ov2735mipiraw_COLOR_0813, ov2735mipiraw_COLOR_0814, ov2735mipiraw_COLOR_0815, ov2735mipiraw_COLOR_0816, ov2735mipiraw_COLOR_0817, ov2735mipiraw_COLOR_0818, ov2735mipiraw_COLOR_0819,
    ov2735mipiraw_COLOR_0820, ov2735mipiraw_COLOR_0821, ov2735mipiraw_COLOR_0822, ov2735mipiraw_COLOR_0823, ov2735mipiraw_COLOR_0824, ov2735mipiraw_COLOR_0825, ov2735mipiraw_COLOR_0826, ov2735mipiraw_COLOR_0827, ov2735mipiraw_COLOR_0828, ov2735mipiraw_COLOR_0829,
    ov2735mipiraw_COLOR_0830, ov2735mipiraw_COLOR_0831, ov2735mipiraw_COLOR_0832, ov2735mipiraw_COLOR_0833, ov2735mipiraw_COLOR_0834, ov2735mipiraw_COLOR_0835, ov2735mipiraw_COLOR_0836, ov2735mipiraw_COLOR_0837, ov2735mipiraw_COLOR_0838, ov2735mipiraw_COLOR_0839,
    ov2735mipiraw_COLOR_0840, ov2735mipiraw_COLOR_0841, ov2735mipiraw_COLOR_0842, ov2735mipiraw_COLOR_0843, ov2735mipiraw_COLOR_0844, ov2735mipiraw_COLOR_0845, ov2735mipiraw_COLOR_0846, ov2735mipiraw_COLOR_0847, ov2735mipiraw_COLOR_0848, ov2735mipiraw_COLOR_0849,
    ov2735mipiraw_COLOR_0850, ov2735mipiraw_COLOR_0851, ov2735mipiraw_COLOR_0852, ov2735mipiraw_COLOR_0853, ov2735mipiraw_COLOR_0854, ov2735mipiraw_COLOR_0855, ov2735mipiraw_COLOR_0856, ov2735mipiraw_COLOR_0857, ov2735mipiraw_COLOR_0858, ov2735mipiraw_COLOR_0859,
    ov2735mipiraw_COLOR_0860, ov2735mipiraw_COLOR_0861, ov2735mipiraw_COLOR_0862, ov2735mipiraw_COLOR_0863, ov2735mipiraw_COLOR_0864, ov2735mipiraw_COLOR_0865, ov2735mipiraw_COLOR_0866, ov2735mipiraw_COLOR_0867, ov2735mipiraw_COLOR_0868, ov2735mipiraw_COLOR_0869,
    ov2735mipiraw_COLOR_0870, ov2735mipiraw_COLOR_0871, ov2735mipiraw_COLOR_0872, ov2735mipiraw_COLOR_0873, ov2735mipiraw_COLOR_0874, ov2735mipiraw_COLOR_0875, ov2735mipiraw_COLOR_0876, ov2735mipiraw_COLOR_0877, ov2735mipiraw_COLOR_0878, ov2735mipiraw_COLOR_0879,
    ov2735mipiraw_COLOR_0880, ov2735mipiraw_COLOR_0881, ov2735mipiraw_COLOR_0882, ov2735mipiraw_COLOR_0883, ov2735mipiraw_COLOR_0884, ov2735mipiraw_COLOR_0885, ov2735mipiraw_COLOR_0886, ov2735mipiraw_COLOR_0887, ov2735mipiraw_COLOR_0888, ov2735mipiraw_COLOR_0889,
    ov2735mipiraw_COLOR_0890, ov2735mipiraw_COLOR_0891, ov2735mipiraw_COLOR_0892, ov2735mipiraw_COLOR_0893, ov2735mipiraw_COLOR_0894, ov2735mipiraw_COLOR_0895, ov2735mipiraw_COLOR_0896, ov2735mipiraw_COLOR_0897, ov2735mipiraw_COLOR_0898, ov2735mipiraw_COLOR_0899,
    ov2735mipiraw_COLOR_0900, ov2735mipiraw_COLOR_0901, ov2735mipiraw_COLOR_0902, ov2735mipiraw_COLOR_0903, ov2735mipiraw_COLOR_0904, ov2735mipiraw_COLOR_0905, ov2735mipiraw_COLOR_0906, ov2735mipiraw_COLOR_0907, ov2735mipiraw_COLOR_0908, ov2735mipiraw_COLOR_0909,
    ov2735mipiraw_COLOR_0910, ov2735mipiraw_COLOR_0911, ov2735mipiraw_COLOR_0912, ov2735mipiraw_COLOR_0913, ov2735mipiraw_COLOR_0914, ov2735mipiraw_COLOR_0915, ov2735mipiraw_COLOR_0916, ov2735mipiraw_COLOR_0917, ov2735mipiraw_COLOR_0918, ov2735mipiraw_COLOR_0919,
    ov2735mipiraw_COLOR_0920, ov2735mipiraw_COLOR_0921, ov2735mipiraw_COLOR_0922, ov2735mipiraw_COLOR_0923, ov2735mipiraw_COLOR_0924, ov2735mipiraw_COLOR_0925, ov2735mipiraw_COLOR_0926, ov2735mipiraw_COLOR_0927, ov2735mipiraw_COLOR_0928, ov2735mipiraw_COLOR_0929,
    ov2735mipiraw_COLOR_0930, ov2735mipiraw_COLOR_0931, ov2735mipiraw_COLOR_0932, ov2735mipiraw_COLOR_0933, ov2735mipiraw_COLOR_0934, ov2735mipiraw_COLOR_0935, ov2735mipiraw_COLOR_0936, ov2735mipiraw_COLOR_0937, ov2735mipiraw_COLOR_0938, ov2735mipiraw_COLOR_0939,
    ov2735mipiraw_COLOR_0940, ov2735mipiraw_COLOR_0941, ov2735mipiraw_COLOR_0942, ov2735mipiraw_COLOR_0943, ov2735mipiraw_COLOR_0944, ov2735mipiraw_COLOR_0945, ov2735mipiraw_COLOR_0946, ov2735mipiraw_COLOR_0947, ov2735mipiraw_COLOR_0948, ov2735mipiraw_COLOR_0949,
    ov2735mipiraw_COLOR_0950, ov2735mipiraw_COLOR_0951, ov2735mipiraw_COLOR_0952, ov2735mipiraw_COLOR_0953, ov2735mipiraw_COLOR_0954, ov2735mipiraw_COLOR_0955, ov2735mipiraw_COLOR_0956, ov2735mipiraw_COLOR_0957, ov2735mipiraw_COLOR_0958, ov2735mipiraw_COLOR_0959,
},
.PCA = {
    ov2735mipiraw_PCA_0000,
},
.COLOR_PARAM = {
    ov2735mipiraw_COLOR_PARAM_0000, ov2735mipiraw_COLOR_PARAM_0001, ov2735mipiraw_COLOR_PARAM_0002, ov2735mipiraw_COLOR_PARAM_0003, ov2735mipiraw_COLOR_PARAM_0004, ov2735mipiraw_COLOR_PARAM_0005, ov2735mipiraw_COLOR_PARAM_0006, ov2735mipiraw_COLOR_PARAM_0007, ov2735mipiraw_COLOR_PARAM_0008, ov2735mipiraw_COLOR_PARAM_0009,
    ov2735mipiraw_COLOR_PARAM_0010, ov2735mipiraw_COLOR_PARAM_0011, ov2735mipiraw_COLOR_PARAM_0012, ov2735mipiraw_COLOR_PARAM_0013, ov2735mipiraw_COLOR_PARAM_0014, ov2735mipiraw_COLOR_PARAM_0015,
},
