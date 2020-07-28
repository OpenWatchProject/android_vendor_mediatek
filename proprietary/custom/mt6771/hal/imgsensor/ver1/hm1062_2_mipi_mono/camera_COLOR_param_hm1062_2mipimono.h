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
    hm1062_2mipimono_CCM_0000, hm1062_2mipimono_CCM_0001, hm1062_2mipimono_CCM_0002, hm1062_2mipimono_CCM_0003, hm1062_2mipimono_CCM_0004, hm1062_2mipimono_CCM_0005, hm1062_2mipimono_CCM_0006, hm1062_2mipimono_CCM_0007, hm1062_2mipimono_CCM_0008, hm1062_2mipimono_CCM_0009,
    hm1062_2mipimono_CCM_0010, hm1062_2mipimono_CCM_0011, hm1062_2mipimono_CCM_0012, hm1062_2mipimono_CCM_0013, hm1062_2mipimono_CCM_0014, hm1062_2mipimono_CCM_0015, hm1062_2mipimono_CCM_0016, hm1062_2mipimono_CCM_0017, hm1062_2mipimono_CCM_0018, hm1062_2mipimono_CCM_0019,
    hm1062_2mipimono_CCM_0020, hm1062_2mipimono_CCM_0021, hm1062_2mipimono_CCM_0022, hm1062_2mipimono_CCM_0023, hm1062_2mipimono_CCM_0024, hm1062_2mipimono_CCM_0025, hm1062_2mipimono_CCM_0026, hm1062_2mipimono_CCM_0027, hm1062_2mipimono_CCM_0028, hm1062_2mipimono_CCM_0029,
    hm1062_2mipimono_CCM_0030, hm1062_2mipimono_CCM_0031, hm1062_2mipimono_CCM_0032, hm1062_2mipimono_CCM_0033, hm1062_2mipimono_CCM_0034, hm1062_2mipimono_CCM_0035, hm1062_2mipimono_CCM_0036, hm1062_2mipimono_CCM_0037, hm1062_2mipimono_CCM_0038, hm1062_2mipimono_CCM_0039,
    hm1062_2mipimono_CCM_0040, hm1062_2mipimono_CCM_0041, hm1062_2mipimono_CCM_0042, hm1062_2mipimono_CCM_0043, hm1062_2mipimono_CCM_0044, hm1062_2mipimono_CCM_0045, hm1062_2mipimono_CCM_0046, hm1062_2mipimono_CCM_0047, hm1062_2mipimono_CCM_0048, hm1062_2mipimono_CCM_0049,
    hm1062_2mipimono_CCM_0050, hm1062_2mipimono_CCM_0051, hm1062_2mipimono_CCM_0052, hm1062_2mipimono_CCM_0053, hm1062_2mipimono_CCM_0054, hm1062_2mipimono_CCM_0055, hm1062_2mipimono_CCM_0056, hm1062_2mipimono_CCM_0057, hm1062_2mipimono_CCM_0058, hm1062_2mipimono_CCM_0059,
    hm1062_2mipimono_CCM_0060, hm1062_2mipimono_CCM_0061, hm1062_2mipimono_CCM_0062, hm1062_2mipimono_CCM_0063, hm1062_2mipimono_CCM_0064, hm1062_2mipimono_CCM_0065, hm1062_2mipimono_CCM_0066, hm1062_2mipimono_CCM_0067, hm1062_2mipimono_CCM_0068, hm1062_2mipimono_CCM_0069,
    hm1062_2mipimono_CCM_0070, hm1062_2mipimono_CCM_0071, hm1062_2mipimono_CCM_0072, hm1062_2mipimono_CCM_0073, hm1062_2mipimono_CCM_0074, hm1062_2mipimono_CCM_0075, hm1062_2mipimono_CCM_0076, hm1062_2mipimono_CCM_0077, hm1062_2mipimono_CCM_0078, hm1062_2mipimono_CCM_0079,
    hm1062_2mipimono_CCM_0080, hm1062_2mipimono_CCM_0081, hm1062_2mipimono_CCM_0082, hm1062_2mipimono_CCM_0083, hm1062_2mipimono_CCM_0084, hm1062_2mipimono_CCM_0085, hm1062_2mipimono_CCM_0086, hm1062_2mipimono_CCM_0087, hm1062_2mipimono_CCM_0088, hm1062_2mipimono_CCM_0089,
    hm1062_2mipimono_CCM_0090, hm1062_2mipimono_CCM_0091, hm1062_2mipimono_CCM_0092, hm1062_2mipimono_CCM_0093, hm1062_2mipimono_CCM_0094, hm1062_2mipimono_CCM_0095, hm1062_2mipimono_CCM_0096, hm1062_2mipimono_CCM_0097, hm1062_2mipimono_CCM_0098, hm1062_2mipimono_CCM_0099,
    hm1062_2mipimono_CCM_0100, hm1062_2mipimono_CCM_0101, hm1062_2mipimono_CCM_0102, hm1062_2mipimono_CCM_0103, hm1062_2mipimono_CCM_0104, hm1062_2mipimono_CCM_0105, hm1062_2mipimono_CCM_0106, hm1062_2mipimono_CCM_0107, hm1062_2mipimono_CCM_0108, hm1062_2mipimono_CCM_0109,
    hm1062_2mipimono_CCM_0110, hm1062_2mipimono_CCM_0111, hm1062_2mipimono_CCM_0112, hm1062_2mipimono_CCM_0113, hm1062_2mipimono_CCM_0114, hm1062_2mipimono_CCM_0115, hm1062_2mipimono_CCM_0116, hm1062_2mipimono_CCM_0117, hm1062_2mipimono_CCM_0118, hm1062_2mipimono_CCM_0119,
    hm1062_2mipimono_CCM_0120, hm1062_2mipimono_CCM_0121, hm1062_2mipimono_CCM_0122, hm1062_2mipimono_CCM_0123, hm1062_2mipimono_CCM_0124, hm1062_2mipimono_CCM_0125, hm1062_2mipimono_CCM_0126, hm1062_2mipimono_CCM_0127, hm1062_2mipimono_CCM_0128, hm1062_2mipimono_CCM_0129,
    hm1062_2mipimono_CCM_0130, hm1062_2mipimono_CCM_0131, hm1062_2mipimono_CCM_0132, hm1062_2mipimono_CCM_0133, hm1062_2mipimono_CCM_0134, hm1062_2mipimono_CCM_0135, hm1062_2mipimono_CCM_0136, hm1062_2mipimono_CCM_0137, hm1062_2mipimono_CCM_0138, hm1062_2mipimono_CCM_0139,
    hm1062_2mipimono_CCM_0140, hm1062_2mipimono_CCM_0141, hm1062_2mipimono_CCM_0142, hm1062_2mipimono_CCM_0143, hm1062_2mipimono_CCM_0144, hm1062_2mipimono_CCM_0145, hm1062_2mipimono_CCM_0146, hm1062_2mipimono_CCM_0147, hm1062_2mipimono_CCM_0148, hm1062_2mipimono_CCM_0149,
    hm1062_2mipimono_CCM_0150, hm1062_2mipimono_CCM_0151, hm1062_2mipimono_CCM_0152, hm1062_2mipimono_CCM_0153, hm1062_2mipimono_CCM_0154, hm1062_2mipimono_CCM_0155, hm1062_2mipimono_CCM_0156, hm1062_2mipimono_CCM_0157, hm1062_2mipimono_CCM_0158, hm1062_2mipimono_CCM_0159,
    hm1062_2mipimono_CCM_0160, hm1062_2mipimono_CCM_0161, hm1062_2mipimono_CCM_0162, hm1062_2mipimono_CCM_0163, hm1062_2mipimono_CCM_0164, hm1062_2mipimono_CCM_0165, hm1062_2mipimono_CCM_0166, hm1062_2mipimono_CCM_0167, hm1062_2mipimono_CCM_0168, hm1062_2mipimono_CCM_0169,
    hm1062_2mipimono_CCM_0170, hm1062_2mipimono_CCM_0171, hm1062_2mipimono_CCM_0172, hm1062_2mipimono_CCM_0173, hm1062_2mipimono_CCM_0174, hm1062_2mipimono_CCM_0175, hm1062_2mipimono_CCM_0176, hm1062_2mipimono_CCM_0177, hm1062_2mipimono_CCM_0178, hm1062_2mipimono_CCM_0179,
    hm1062_2mipimono_CCM_0180, hm1062_2mipimono_CCM_0181, hm1062_2mipimono_CCM_0182, hm1062_2mipimono_CCM_0183, hm1062_2mipimono_CCM_0184, hm1062_2mipimono_CCM_0185, hm1062_2mipimono_CCM_0186, hm1062_2mipimono_CCM_0187, hm1062_2mipimono_CCM_0188, hm1062_2mipimono_CCM_0189,
    hm1062_2mipimono_CCM_0190, hm1062_2mipimono_CCM_0191, hm1062_2mipimono_CCM_0192, hm1062_2mipimono_CCM_0193, hm1062_2mipimono_CCM_0194, hm1062_2mipimono_CCM_0195, hm1062_2mipimono_CCM_0196, hm1062_2mipimono_CCM_0197, hm1062_2mipimono_CCM_0198, hm1062_2mipimono_CCM_0199,
    hm1062_2mipimono_CCM_0200, hm1062_2mipimono_CCM_0201, hm1062_2mipimono_CCM_0202, hm1062_2mipimono_CCM_0203, hm1062_2mipimono_CCM_0204, hm1062_2mipimono_CCM_0205, hm1062_2mipimono_CCM_0206, hm1062_2mipimono_CCM_0207, hm1062_2mipimono_CCM_0208, hm1062_2mipimono_CCM_0209,
    hm1062_2mipimono_CCM_0210, hm1062_2mipimono_CCM_0211, hm1062_2mipimono_CCM_0212, hm1062_2mipimono_CCM_0213, hm1062_2mipimono_CCM_0214, hm1062_2mipimono_CCM_0215, hm1062_2mipimono_CCM_0216, hm1062_2mipimono_CCM_0217, hm1062_2mipimono_CCM_0218, hm1062_2mipimono_CCM_0219,
    hm1062_2mipimono_CCM_0220, hm1062_2mipimono_CCM_0221, hm1062_2mipimono_CCM_0222, hm1062_2mipimono_CCM_0223, hm1062_2mipimono_CCM_0224, hm1062_2mipimono_CCM_0225, hm1062_2mipimono_CCM_0226, hm1062_2mipimono_CCM_0227, hm1062_2mipimono_CCM_0228, hm1062_2mipimono_CCM_0229,
    hm1062_2mipimono_CCM_0230, hm1062_2mipimono_CCM_0231, hm1062_2mipimono_CCM_0232, hm1062_2mipimono_CCM_0233, hm1062_2mipimono_CCM_0234, hm1062_2mipimono_CCM_0235, hm1062_2mipimono_CCM_0236, hm1062_2mipimono_CCM_0237, hm1062_2mipimono_CCM_0238, hm1062_2mipimono_CCM_0239,
},
.COLOR = {
    hm1062_2mipimono_COLOR_0000, hm1062_2mipimono_COLOR_0001, hm1062_2mipimono_COLOR_0002, hm1062_2mipimono_COLOR_0003, hm1062_2mipimono_COLOR_0004, hm1062_2mipimono_COLOR_0005, hm1062_2mipimono_COLOR_0006, hm1062_2mipimono_COLOR_0007, hm1062_2mipimono_COLOR_0008, hm1062_2mipimono_COLOR_0009,
    hm1062_2mipimono_COLOR_0010, hm1062_2mipimono_COLOR_0011, hm1062_2mipimono_COLOR_0012, hm1062_2mipimono_COLOR_0013, hm1062_2mipimono_COLOR_0014, hm1062_2mipimono_COLOR_0015, hm1062_2mipimono_COLOR_0016, hm1062_2mipimono_COLOR_0017, hm1062_2mipimono_COLOR_0018, hm1062_2mipimono_COLOR_0019,
    hm1062_2mipimono_COLOR_0020, hm1062_2mipimono_COLOR_0021, hm1062_2mipimono_COLOR_0022, hm1062_2mipimono_COLOR_0023, hm1062_2mipimono_COLOR_0024, hm1062_2mipimono_COLOR_0025, hm1062_2mipimono_COLOR_0026, hm1062_2mipimono_COLOR_0027, hm1062_2mipimono_COLOR_0028, hm1062_2mipimono_COLOR_0029,
    hm1062_2mipimono_COLOR_0030, hm1062_2mipimono_COLOR_0031, hm1062_2mipimono_COLOR_0032, hm1062_2mipimono_COLOR_0033, hm1062_2mipimono_COLOR_0034, hm1062_2mipimono_COLOR_0035, hm1062_2mipimono_COLOR_0036, hm1062_2mipimono_COLOR_0037, hm1062_2mipimono_COLOR_0038, hm1062_2mipimono_COLOR_0039,
    hm1062_2mipimono_COLOR_0040, hm1062_2mipimono_COLOR_0041, hm1062_2mipimono_COLOR_0042, hm1062_2mipimono_COLOR_0043, hm1062_2mipimono_COLOR_0044, hm1062_2mipimono_COLOR_0045, hm1062_2mipimono_COLOR_0046, hm1062_2mipimono_COLOR_0047, hm1062_2mipimono_COLOR_0048, hm1062_2mipimono_COLOR_0049,
    hm1062_2mipimono_COLOR_0050, hm1062_2mipimono_COLOR_0051, hm1062_2mipimono_COLOR_0052, hm1062_2mipimono_COLOR_0053, hm1062_2mipimono_COLOR_0054, hm1062_2mipimono_COLOR_0055, hm1062_2mipimono_COLOR_0056, hm1062_2mipimono_COLOR_0057, hm1062_2mipimono_COLOR_0058, hm1062_2mipimono_COLOR_0059,
    hm1062_2mipimono_COLOR_0060, hm1062_2mipimono_COLOR_0061, hm1062_2mipimono_COLOR_0062, hm1062_2mipimono_COLOR_0063, hm1062_2mipimono_COLOR_0064, hm1062_2mipimono_COLOR_0065, hm1062_2mipimono_COLOR_0066, hm1062_2mipimono_COLOR_0067, hm1062_2mipimono_COLOR_0068, hm1062_2mipimono_COLOR_0069,
    hm1062_2mipimono_COLOR_0070, hm1062_2mipimono_COLOR_0071, hm1062_2mipimono_COLOR_0072, hm1062_2mipimono_COLOR_0073, hm1062_2mipimono_COLOR_0074, hm1062_2mipimono_COLOR_0075, hm1062_2mipimono_COLOR_0076, hm1062_2mipimono_COLOR_0077, hm1062_2mipimono_COLOR_0078, hm1062_2mipimono_COLOR_0079,
    hm1062_2mipimono_COLOR_0080, hm1062_2mipimono_COLOR_0081, hm1062_2mipimono_COLOR_0082, hm1062_2mipimono_COLOR_0083, hm1062_2mipimono_COLOR_0084, hm1062_2mipimono_COLOR_0085, hm1062_2mipimono_COLOR_0086, hm1062_2mipimono_COLOR_0087, hm1062_2mipimono_COLOR_0088, hm1062_2mipimono_COLOR_0089,
    hm1062_2mipimono_COLOR_0090, hm1062_2mipimono_COLOR_0091, hm1062_2mipimono_COLOR_0092, hm1062_2mipimono_COLOR_0093, hm1062_2mipimono_COLOR_0094, hm1062_2mipimono_COLOR_0095, hm1062_2mipimono_COLOR_0096, hm1062_2mipimono_COLOR_0097, hm1062_2mipimono_COLOR_0098, hm1062_2mipimono_COLOR_0099,
    hm1062_2mipimono_COLOR_0100, hm1062_2mipimono_COLOR_0101, hm1062_2mipimono_COLOR_0102, hm1062_2mipimono_COLOR_0103, hm1062_2mipimono_COLOR_0104, hm1062_2mipimono_COLOR_0105, hm1062_2mipimono_COLOR_0106, hm1062_2mipimono_COLOR_0107, hm1062_2mipimono_COLOR_0108, hm1062_2mipimono_COLOR_0109,
    hm1062_2mipimono_COLOR_0110, hm1062_2mipimono_COLOR_0111, hm1062_2mipimono_COLOR_0112, hm1062_2mipimono_COLOR_0113, hm1062_2mipimono_COLOR_0114, hm1062_2mipimono_COLOR_0115, hm1062_2mipimono_COLOR_0116, hm1062_2mipimono_COLOR_0117, hm1062_2mipimono_COLOR_0118, hm1062_2mipimono_COLOR_0119,
    hm1062_2mipimono_COLOR_0120, hm1062_2mipimono_COLOR_0121, hm1062_2mipimono_COLOR_0122, hm1062_2mipimono_COLOR_0123, hm1062_2mipimono_COLOR_0124, hm1062_2mipimono_COLOR_0125, hm1062_2mipimono_COLOR_0126, hm1062_2mipimono_COLOR_0127, hm1062_2mipimono_COLOR_0128, hm1062_2mipimono_COLOR_0129,
    hm1062_2mipimono_COLOR_0130, hm1062_2mipimono_COLOR_0131, hm1062_2mipimono_COLOR_0132, hm1062_2mipimono_COLOR_0133, hm1062_2mipimono_COLOR_0134, hm1062_2mipimono_COLOR_0135, hm1062_2mipimono_COLOR_0136, hm1062_2mipimono_COLOR_0137, hm1062_2mipimono_COLOR_0138, hm1062_2mipimono_COLOR_0139,
    hm1062_2mipimono_COLOR_0140, hm1062_2mipimono_COLOR_0141, hm1062_2mipimono_COLOR_0142, hm1062_2mipimono_COLOR_0143, hm1062_2mipimono_COLOR_0144, hm1062_2mipimono_COLOR_0145, hm1062_2mipimono_COLOR_0146, hm1062_2mipimono_COLOR_0147, hm1062_2mipimono_COLOR_0148, hm1062_2mipimono_COLOR_0149,
    hm1062_2mipimono_COLOR_0150, hm1062_2mipimono_COLOR_0151, hm1062_2mipimono_COLOR_0152, hm1062_2mipimono_COLOR_0153, hm1062_2mipimono_COLOR_0154, hm1062_2mipimono_COLOR_0155, hm1062_2mipimono_COLOR_0156, hm1062_2mipimono_COLOR_0157, hm1062_2mipimono_COLOR_0158, hm1062_2mipimono_COLOR_0159,
    hm1062_2mipimono_COLOR_0160, hm1062_2mipimono_COLOR_0161, hm1062_2mipimono_COLOR_0162, hm1062_2mipimono_COLOR_0163, hm1062_2mipimono_COLOR_0164, hm1062_2mipimono_COLOR_0165, hm1062_2mipimono_COLOR_0166, hm1062_2mipimono_COLOR_0167, hm1062_2mipimono_COLOR_0168, hm1062_2mipimono_COLOR_0169,
    hm1062_2mipimono_COLOR_0170, hm1062_2mipimono_COLOR_0171, hm1062_2mipimono_COLOR_0172, hm1062_2mipimono_COLOR_0173, hm1062_2mipimono_COLOR_0174, hm1062_2mipimono_COLOR_0175, hm1062_2mipimono_COLOR_0176, hm1062_2mipimono_COLOR_0177, hm1062_2mipimono_COLOR_0178, hm1062_2mipimono_COLOR_0179,
    hm1062_2mipimono_COLOR_0180, hm1062_2mipimono_COLOR_0181, hm1062_2mipimono_COLOR_0182, hm1062_2mipimono_COLOR_0183, hm1062_2mipimono_COLOR_0184, hm1062_2mipimono_COLOR_0185, hm1062_2mipimono_COLOR_0186, hm1062_2mipimono_COLOR_0187, hm1062_2mipimono_COLOR_0188, hm1062_2mipimono_COLOR_0189,
    hm1062_2mipimono_COLOR_0190, hm1062_2mipimono_COLOR_0191, hm1062_2mipimono_COLOR_0192, hm1062_2mipimono_COLOR_0193, hm1062_2mipimono_COLOR_0194, hm1062_2mipimono_COLOR_0195, hm1062_2mipimono_COLOR_0196, hm1062_2mipimono_COLOR_0197, hm1062_2mipimono_COLOR_0198, hm1062_2mipimono_COLOR_0199,
    hm1062_2mipimono_COLOR_0200, hm1062_2mipimono_COLOR_0201, hm1062_2mipimono_COLOR_0202, hm1062_2mipimono_COLOR_0203, hm1062_2mipimono_COLOR_0204, hm1062_2mipimono_COLOR_0205, hm1062_2mipimono_COLOR_0206, hm1062_2mipimono_COLOR_0207, hm1062_2mipimono_COLOR_0208, hm1062_2mipimono_COLOR_0209,
    hm1062_2mipimono_COLOR_0210, hm1062_2mipimono_COLOR_0211, hm1062_2mipimono_COLOR_0212, hm1062_2mipimono_COLOR_0213, hm1062_2mipimono_COLOR_0214, hm1062_2mipimono_COLOR_0215, hm1062_2mipimono_COLOR_0216, hm1062_2mipimono_COLOR_0217, hm1062_2mipimono_COLOR_0218, hm1062_2mipimono_COLOR_0219,
    hm1062_2mipimono_COLOR_0220, hm1062_2mipimono_COLOR_0221, hm1062_2mipimono_COLOR_0222, hm1062_2mipimono_COLOR_0223, hm1062_2mipimono_COLOR_0224, hm1062_2mipimono_COLOR_0225, hm1062_2mipimono_COLOR_0226, hm1062_2mipimono_COLOR_0227, hm1062_2mipimono_COLOR_0228, hm1062_2mipimono_COLOR_0229,
    hm1062_2mipimono_COLOR_0230, hm1062_2mipimono_COLOR_0231, hm1062_2mipimono_COLOR_0232, hm1062_2mipimono_COLOR_0233, hm1062_2mipimono_COLOR_0234, hm1062_2mipimono_COLOR_0235, hm1062_2mipimono_COLOR_0236, hm1062_2mipimono_COLOR_0237, hm1062_2mipimono_COLOR_0238, hm1062_2mipimono_COLOR_0239,
    hm1062_2mipimono_COLOR_0240, hm1062_2mipimono_COLOR_0241, hm1062_2mipimono_COLOR_0242, hm1062_2mipimono_COLOR_0243, hm1062_2mipimono_COLOR_0244, hm1062_2mipimono_COLOR_0245, hm1062_2mipimono_COLOR_0246, hm1062_2mipimono_COLOR_0247, hm1062_2mipimono_COLOR_0248, hm1062_2mipimono_COLOR_0249,
    hm1062_2mipimono_COLOR_0250, hm1062_2mipimono_COLOR_0251, hm1062_2mipimono_COLOR_0252, hm1062_2mipimono_COLOR_0253, hm1062_2mipimono_COLOR_0254, hm1062_2mipimono_COLOR_0255, hm1062_2mipimono_COLOR_0256, hm1062_2mipimono_COLOR_0257, hm1062_2mipimono_COLOR_0258, hm1062_2mipimono_COLOR_0259,
    hm1062_2mipimono_COLOR_0260, hm1062_2mipimono_COLOR_0261, hm1062_2mipimono_COLOR_0262, hm1062_2mipimono_COLOR_0263, hm1062_2mipimono_COLOR_0264, hm1062_2mipimono_COLOR_0265, hm1062_2mipimono_COLOR_0266, hm1062_2mipimono_COLOR_0267, hm1062_2mipimono_COLOR_0268, hm1062_2mipimono_COLOR_0269,
    hm1062_2mipimono_COLOR_0270, hm1062_2mipimono_COLOR_0271, hm1062_2mipimono_COLOR_0272, hm1062_2mipimono_COLOR_0273, hm1062_2mipimono_COLOR_0274, hm1062_2mipimono_COLOR_0275, hm1062_2mipimono_COLOR_0276, hm1062_2mipimono_COLOR_0277, hm1062_2mipimono_COLOR_0278, hm1062_2mipimono_COLOR_0279,
    hm1062_2mipimono_COLOR_0280, hm1062_2mipimono_COLOR_0281, hm1062_2mipimono_COLOR_0282, hm1062_2mipimono_COLOR_0283, hm1062_2mipimono_COLOR_0284, hm1062_2mipimono_COLOR_0285, hm1062_2mipimono_COLOR_0286, hm1062_2mipimono_COLOR_0287, hm1062_2mipimono_COLOR_0288, hm1062_2mipimono_COLOR_0289,
    hm1062_2mipimono_COLOR_0290, hm1062_2mipimono_COLOR_0291, hm1062_2mipimono_COLOR_0292, hm1062_2mipimono_COLOR_0293, hm1062_2mipimono_COLOR_0294, hm1062_2mipimono_COLOR_0295, hm1062_2mipimono_COLOR_0296, hm1062_2mipimono_COLOR_0297, hm1062_2mipimono_COLOR_0298, hm1062_2mipimono_COLOR_0299,
    hm1062_2mipimono_COLOR_0300, hm1062_2mipimono_COLOR_0301, hm1062_2mipimono_COLOR_0302, hm1062_2mipimono_COLOR_0303, hm1062_2mipimono_COLOR_0304, hm1062_2mipimono_COLOR_0305, hm1062_2mipimono_COLOR_0306, hm1062_2mipimono_COLOR_0307, hm1062_2mipimono_COLOR_0308, hm1062_2mipimono_COLOR_0309,
    hm1062_2mipimono_COLOR_0310, hm1062_2mipimono_COLOR_0311, hm1062_2mipimono_COLOR_0312, hm1062_2mipimono_COLOR_0313, hm1062_2mipimono_COLOR_0314, hm1062_2mipimono_COLOR_0315, hm1062_2mipimono_COLOR_0316, hm1062_2mipimono_COLOR_0317, hm1062_2mipimono_COLOR_0318, hm1062_2mipimono_COLOR_0319,
    hm1062_2mipimono_COLOR_0320, hm1062_2mipimono_COLOR_0321, hm1062_2mipimono_COLOR_0322, hm1062_2mipimono_COLOR_0323, hm1062_2mipimono_COLOR_0324, hm1062_2mipimono_COLOR_0325, hm1062_2mipimono_COLOR_0326, hm1062_2mipimono_COLOR_0327, hm1062_2mipimono_COLOR_0328, hm1062_2mipimono_COLOR_0329,
    hm1062_2mipimono_COLOR_0330, hm1062_2mipimono_COLOR_0331, hm1062_2mipimono_COLOR_0332, hm1062_2mipimono_COLOR_0333, hm1062_2mipimono_COLOR_0334, hm1062_2mipimono_COLOR_0335, hm1062_2mipimono_COLOR_0336, hm1062_2mipimono_COLOR_0337, hm1062_2mipimono_COLOR_0338, hm1062_2mipimono_COLOR_0339,
    hm1062_2mipimono_COLOR_0340, hm1062_2mipimono_COLOR_0341, hm1062_2mipimono_COLOR_0342, hm1062_2mipimono_COLOR_0343, hm1062_2mipimono_COLOR_0344, hm1062_2mipimono_COLOR_0345, hm1062_2mipimono_COLOR_0346, hm1062_2mipimono_COLOR_0347, hm1062_2mipimono_COLOR_0348, hm1062_2mipimono_COLOR_0349,
    hm1062_2mipimono_COLOR_0350, hm1062_2mipimono_COLOR_0351, hm1062_2mipimono_COLOR_0352, hm1062_2mipimono_COLOR_0353, hm1062_2mipimono_COLOR_0354, hm1062_2mipimono_COLOR_0355, hm1062_2mipimono_COLOR_0356, hm1062_2mipimono_COLOR_0357, hm1062_2mipimono_COLOR_0358, hm1062_2mipimono_COLOR_0359,
    hm1062_2mipimono_COLOR_0360, hm1062_2mipimono_COLOR_0361, hm1062_2mipimono_COLOR_0362, hm1062_2mipimono_COLOR_0363, hm1062_2mipimono_COLOR_0364, hm1062_2mipimono_COLOR_0365, hm1062_2mipimono_COLOR_0366, hm1062_2mipimono_COLOR_0367, hm1062_2mipimono_COLOR_0368, hm1062_2mipimono_COLOR_0369,
    hm1062_2mipimono_COLOR_0370, hm1062_2mipimono_COLOR_0371, hm1062_2mipimono_COLOR_0372, hm1062_2mipimono_COLOR_0373, hm1062_2mipimono_COLOR_0374, hm1062_2mipimono_COLOR_0375, hm1062_2mipimono_COLOR_0376, hm1062_2mipimono_COLOR_0377, hm1062_2mipimono_COLOR_0378, hm1062_2mipimono_COLOR_0379,
    hm1062_2mipimono_COLOR_0380, hm1062_2mipimono_COLOR_0381, hm1062_2mipimono_COLOR_0382, hm1062_2mipimono_COLOR_0383, hm1062_2mipimono_COLOR_0384, hm1062_2mipimono_COLOR_0385, hm1062_2mipimono_COLOR_0386, hm1062_2mipimono_COLOR_0387, hm1062_2mipimono_COLOR_0388, hm1062_2mipimono_COLOR_0389,
    hm1062_2mipimono_COLOR_0390, hm1062_2mipimono_COLOR_0391, hm1062_2mipimono_COLOR_0392, hm1062_2mipimono_COLOR_0393, hm1062_2mipimono_COLOR_0394, hm1062_2mipimono_COLOR_0395, hm1062_2mipimono_COLOR_0396, hm1062_2mipimono_COLOR_0397, hm1062_2mipimono_COLOR_0398, hm1062_2mipimono_COLOR_0399,
    hm1062_2mipimono_COLOR_0400, hm1062_2mipimono_COLOR_0401, hm1062_2mipimono_COLOR_0402, hm1062_2mipimono_COLOR_0403, hm1062_2mipimono_COLOR_0404, hm1062_2mipimono_COLOR_0405, hm1062_2mipimono_COLOR_0406, hm1062_2mipimono_COLOR_0407, hm1062_2mipimono_COLOR_0408, hm1062_2mipimono_COLOR_0409,
    hm1062_2mipimono_COLOR_0410, hm1062_2mipimono_COLOR_0411, hm1062_2mipimono_COLOR_0412, hm1062_2mipimono_COLOR_0413, hm1062_2mipimono_COLOR_0414, hm1062_2mipimono_COLOR_0415, hm1062_2mipimono_COLOR_0416, hm1062_2mipimono_COLOR_0417, hm1062_2mipimono_COLOR_0418, hm1062_2mipimono_COLOR_0419,
    hm1062_2mipimono_COLOR_0420, hm1062_2mipimono_COLOR_0421, hm1062_2mipimono_COLOR_0422, hm1062_2mipimono_COLOR_0423, hm1062_2mipimono_COLOR_0424, hm1062_2mipimono_COLOR_0425, hm1062_2mipimono_COLOR_0426, hm1062_2mipimono_COLOR_0427, hm1062_2mipimono_COLOR_0428, hm1062_2mipimono_COLOR_0429,
    hm1062_2mipimono_COLOR_0430, hm1062_2mipimono_COLOR_0431, hm1062_2mipimono_COLOR_0432, hm1062_2mipimono_COLOR_0433, hm1062_2mipimono_COLOR_0434, hm1062_2mipimono_COLOR_0435, hm1062_2mipimono_COLOR_0436, hm1062_2mipimono_COLOR_0437, hm1062_2mipimono_COLOR_0438, hm1062_2mipimono_COLOR_0439,
    hm1062_2mipimono_COLOR_0440, hm1062_2mipimono_COLOR_0441, hm1062_2mipimono_COLOR_0442, hm1062_2mipimono_COLOR_0443, hm1062_2mipimono_COLOR_0444, hm1062_2mipimono_COLOR_0445, hm1062_2mipimono_COLOR_0446, hm1062_2mipimono_COLOR_0447, hm1062_2mipimono_COLOR_0448, hm1062_2mipimono_COLOR_0449,
    hm1062_2mipimono_COLOR_0450, hm1062_2mipimono_COLOR_0451, hm1062_2mipimono_COLOR_0452, hm1062_2mipimono_COLOR_0453, hm1062_2mipimono_COLOR_0454, hm1062_2mipimono_COLOR_0455, hm1062_2mipimono_COLOR_0456, hm1062_2mipimono_COLOR_0457, hm1062_2mipimono_COLOR_0458, hm1062_2mipimono_COLOR_0459,
    hm1062_2mipimono_COLOR_0460, hm1062_2mipimono_COLOR_0461, hm1062_2mipimono_COLOR_0462, hm1062_2mipimono_COLOR_0463, hm1062_2mipimono_COLOR_0464, hm1062_2mipimono_COLOR_0465, hm1062_2mipimono_COLOR_0466, hm1062_2mipimono_COLOR_0467, hm1062_2mipimono_COLOR_0468, hm1062_2mipimono_COLOR_0469,
    hm1062_2mipimono_COLOR_0470, hm1062_2mipimono_COLOR_0471, hm1062_2mipimono_COLOR_0472, hm1062_2mipimono_COLOR_0473, hm1062_2mipimono_COLOR_0474, hm1062_2mipimono_COLOR_0475, hm1062_2mipimono_COLOR_0476, hm1062_2mipimono_COLOR_0477, hm1062_2mipimono_COLOR_0478, hm1062_2mipimono_COLOR_0479,
    hm1062_2mipimono_COLOR_0480, hm1062_2mipimono_COLOR_0481, hm1062_2mipimono_COLOR_0482, hm1062_2mipimono_COLOR_0483, hm1062_2mipimono_COLOR_0484, hm1062_2mipimono_COLOR_0485, hm1062_2mipimono_COLOR_0486, hm1062_2mipimono_COLOR_0487, hm1062_2mipimono_COLOR_0488, hm1062_2mipimono_COLOR_0489,
    hm1062_2mipimono_COLOR_0490, hm1062_2mipimono_COLOR_0491, hm1062_2mipimono_COLOR_0492, hm1062_2mipimono_COLOR_0493, hm1062_2mipimono_COLOR_0494, hm1062_2mipimono_COLOR_0495, hm1062_2mipimono_COLOR_0496, hm1062_2mipimono_COLOR_0497, hm1062_2mipimono_COLOR_0498, hm1062_2mipimono_COLOR_0499,
    hm1062_2mipimono_COLOR_0500, hm1062_2mipimono_COLOR_0501, hm1062_2mipimono_COLOR_0502, hm1062_2mipimono_COLOR_0503, hm1062_2mipimono_COLOR_0504, hm1062_2mipimono_COLOR_0505, hm1062_2mipimono_COLOR_0506, hm1062_2mipimono_COLOR_0507, hm1062_2mipimono_COLOR_0508, hm1062_2mipimono_COLOR_0509,
    hm1062_2mipimono_COLOR_0510, hm1062_2mipimono_COLOR_0511, hm1062_2mipimono_COLOR_0512, hm1062_2mipimono_COLOR_0513, hm1062_2mipimono_COLOR_0514, hm1062_2mipimono_COLOR_0515, hm1062_2mipimono_COLOR_0516, hm1062_2mipimono_COLOR_0517, hm1062_2mipimono_COLOR_0518, hm1062_2mipimono_COLOR_0519,
    hm1062_2mipimono_COLOR_0520, hm1062_2mipimono_COLOR_0521, hm1062_2mipimono_COLOR_0522, hm1062_2mipimono_COLOR_0523, hm1062_2mipimono_COLOR_0524, hm1062_2mipimono_COLOR_0525, hm1062_2mipimono_COLOR_0526, hm1062_2mipimono_COLOR_0527, hm1062_2mipimono_COLOR_0528, hm1062_2mipimono_COLOR_0529,
    hm1062_2mipimono_COLOR_0530, hm1062_2mipimono_COLOR_0531, hm1062_2mipimono_COLOR_0532, hm1062_2mipimono_COLOR_0533, hm1062_2mipimono_COLOR_0534, hm1062_2mipimono_COLOR_0535, hm1062_2mipimono_COLOR_0536, hm1062_2mipimono_COLOR_0537, hm1062_2mipimono_COLOR_0538, hm1062_2mipimono_COLOR_0539,
    hm1062_2mipimono_COLOR_0540, hm1062_2mipimono_COLOR_0541, hm1062_2mipimono_COLOR_0542, hm1062_2mipimono_COLOR_0543, hm1062_2mipimono_COLOR_0544, hm1062_2mipimono_COLOR_0545, hm1062_2mipimono_COLOR_0546, hm1062_2mipimono_COLOR_0547, hm1062_2mipimono_COLOR_0548, hm1062_2mipimono_COLOR_0549,
    hm1062_2mipimono_COLOR_0550, hm1062_2mipimono_COLOR_0551, hm1062_2mipimono_COLOR_0552, hm1062_2mipimono_COLOR_0553, hm1062_2mipimono_COLOR_0554, hm1062_2mipimono_COLOR_0555, hm1062_2mipimono_COLOR_0556, hm1062_2mipimono_COLOR_0557, hm1062_2mipimono_COLOR_0558, hm1062_2mipimono_COLOR_0559,
    hm1062_2mipimono_COLOR_0560, hm1062_2mipimono_COLOR_0561, hm1062_2mipimono_COLOR_0562, hm1062_2mipimono_COLOR_0563, hm1062_2mipimono_COLOR_0564, hm1062_2mipimono_COLOR_0565, hm1062_2mipimono_COLOR_0566, hm1062_2mipimono_COLOR_0567, hm1062_2mipimono_COLOR_0568, hm1062_2mipimono_COLOR_0569,
    hm1062_2mipimono_COLOR_0570, hm1062_2mipimono_COLOR_0571, hm1062_2mipimono_COLOR_0572, hm1062_2mipimono_COLOR_0573, hm1062_2mipimono_COLOR_0574, hm1062_2mipimono_COLOR_0575, hm1062_2mipimono_COLOR_0576, hm1062_2mipimono_COLOR_0577, hm1062_2mipimono_COLOR_0578, hm1062_2mipimono_COLOR_0579,
    hm1062_2mipimono_COLOR_0580, hm1062_2mipimono_COLOR_0581, hm1062_2mipimono_COLOR_0582, hm1062_2mipimono_COLOR_0583, hm1062_2mipimono_COLOR_0584, hm1062_2mipimono_COLOR_0585, hm1062_2mipimono_COLOR_0586, hm1062_2mipimono_COLOR_0587, hm1062_2mipimono_COLOR_0588, hm1062_2mipimono_COLOR_0589,
    hm1062_2mipimono_COLOR_0590, hm1062_2mipimono_COLOR_0591, hm1062_2mipimono_COLOR_0592, hm1062_2mipimono_COLOR_0593, hm1062_2mipimono_COLOR_0594, hm1062_2mipimono_COLOR_0595, hm1062_2mipimono_COLOR_0596, hm1062_2mipimono_COLOR_0597, hm1062_2mipimono_COLOR_0598, hm1062_2mipimono_COLOR_0599,
    hm1062_2mipimono_COLOR_0600, hm1062_2mipimono_COLOR_0601, hm1062_2mipimono_COLOR_0602, hm1062_2mipimono_COLOR_0603, hm1062_2mipimono_COLOR_0604, hm1062_2mipimono_COLOR_0605, hm1062_2mipimono_COLOR_0606, hm1062_2mipimono_COLOR_0607, hm1062_2mipimono_COLOR_0608, hm1062_2mipimono_COLOR_0609,
    hm1062_2mipimono_COLOR_0610, hm1062_2mipimono_COLOR_0611, hm1062_2mipimono_COLOR_0612, hm1062_2mipimono_COLOR_0613, hm1062_2mipimono_COLOR_0614, hm1062_2mipimono_COLOR_0615, hm1062_2mipimono_COLOR_0616, hm1062_2mipimono_COLOR_0617, hm1062_2mipimono_COLOR_0618, hm1062_2mipimono_COLOR_0619,
    hm1062_2mipimono_COLOR_0620, hm1062_2mipimono_COLOR_0621, hm1062_2mipimono_COLOR_0622, hm1062_2mipimono_COLOR_0623, hm1062_2mipimono_COLOR_0624, hm1062_2mipimono_COLOR_0625, hm1062_2mipimono_COLOR_0626, hm1062_2mipimono_COLOR_0627, hm1062_2mipimono_COLOR_0628, hm1062_2mipimono_COLOR_0629,
    hm1062_2mipimono_COLOR_0630, hm1062_2mipimono_COLOR_0631, hm1062_2mipimono_COLOR_0632, hm1062_2mipimono_COLOR_0633, hm1062_2mipimono_COLOR_0634, hm1062_2mipimono_COLOR_0635, hm1062_2mipimono_COLOR_0636, hm1062_2mipimono_COLOR_0637, hm1062_2mipimono_COLOR_0638, hm1062_2mipimono_COLOR_0639,
    hm1062_2mipimono_COLOR_0640, hm1062_2mipimono_COLOR_0641, hm1062_2mipimono_COLOR_0642, hm1062_2mipimono_COLOR_0643, hm1062_2mipimono_COLOR_0644, hm1062_2mipimono_COLOR_0645, hm1062_2mipimono_COLOR_0646, hm1062_2mipimono_COLOR_0647, hm1062_2mipimono_COLOR_0648, hm1062_2mipimono_COLOR_0649,
    hm1062_2mipimono_COLOR_0650, hm1062_2mipimono_COLOR_0651, hm1062_2mipimono_COLOR_0652, hm1062_2mipimono_COLOR_0653, hm1062_2mipimono_COLOR_0654, hm1062_2mipimono_COLOR_0655, hm1062_2mipimono_COLOR_0656, hm1062_2mipimono_COLOR_0657, hm1062_2mipimono_COLOR_0658, hm1062_2mipimono_COLOR_0659,
    hm1062_2mipimono_COLOR_0660, hm1062_2mipimono_COLOR_0661, hm1062_2mipimono_COLOR_0662, hm1062_2mipimono_COLOR_0663, hm1062_2mipimono_COLOR_0664, hm1062_2mipimono_COLOR_0665, hm1062_2mipimono_COLOR_0666, hm1062_2mipimono_COLOR_0667, hm1062_2mipimono_COLOR_0668, hm1062_2mipimono_COLOR_0669,
    hm1062_2mipimono_COLOR_0670, hm1062_2mipimono_COLOR_0671, hm1062_2mipimono_COLOR_0672, hm1062_2mipimono_COLOR_0673, hm1062_2mipimono_COLOR_0674, hm1062_2mipimono_COLOR_0675, hm1062_2mipimono_COLOR_0676, hm1062_2mipimono_COLOR_0677, hm1062_2mipimono_COLOR_0678, hm1062_2mipimono_COLOR_0679,
    hm1062_2mipimono_COLOR_0680, hm1062_2mipimono_COLOR_0681, hm1062_2mipimono_COLOR_0682, hm1062_2mipimono_COLOR_0683, hm1062_2mipimono_COLOR_0684, hm1062_2mipimono_COLOR_0685, hm1062_2mipimono_COLOR_0686, hm1062_2mipimono_COLOR_0687, hm1062_2mipimono_COLOR_0688, hm1062_2mipimono_COLOR_0689,
    hm1062_2mipimono_COLOR_0690, hm1062_2mipimono_COLOR_0691, hm1062_2mipimono_COLOR_0692, hm1062_2mipimono_COLOR_0693, hm1062_2mipimono_COLOR_0694, hm1062_2mipimono_COLOR_0695, hm1062_2mipimono_COLOR_0696, hm1062_2mipimono_COLOR_0697, hm1062_2mipimono_COLOR_0698, hm1062_2mipimono_COLOR_0699,
    hm1062_2mipimono_COLOR_0700, hm1062_2mipimono_COLOR_0701, hm1062_2mipimono_COLOR_0702, hm1062_2mipimono_COLOR_0703, hm1062_2mipimono_COLOR_0704, hm1062_2mipimono_COLOR_0705, hm1062_2mipimono_COLOR_0706, hm1062_2mipimono_COLOR_0707, hm1062_2mipimono_COLOR_0708, hm1062_2mipimono_COLOR_0709,
    hm1062_2mipimono_COLOR_0710, hm1062_2mipimono_COLOR_0711, hm1062_2mipimono_COLOR_0712, hm1062_2mipimono_COLOR_0713, hm1062_2mipimono_COLOR_0714, hm1062_2mipimono_COLOR_0715, hm1062_2mipimono_COLOR_0716, hm1062_2mipimono_COLOR_0717, hm1062_2mipimono_COLOR_0718, hm1062_2mipimono_COLOR_0719,
    hm1062_2mipimono_COLOR_0720, hm1062_2mipimono_COLOR_0721, hm1062_2mipimono_COLOR_0722, hm1062_2mipimono_COLOR_0723, hm1062_2mipimono_COLOR_0724, hm1062_2mipimono_COLOR_0725, hm1062_2mipimono_COLOR_0726, hm1062_2mipimono_COLOR_0727, hm1062_2mipimono_COLOR_0728, hm1062_2mipimono_COLOR_0729,
    hm1062_2mipimono_COLOR_0730, hm1062_2mipimono_COLOR_0731, hm1062_2mipimono_COLOR_0732, hm1062_2mipimono_COLOR_0733, hm1062_2mipimono_COLOR_0734, hm1062_2mipimono_COLOR_0735, hm1062_2mipimono_COLOR_0736, hm1062_2mipimono_COLOR_0737, hm1062_2mipimono_COLOR_0738, hm1062_2mipimono_COLOR_0739,
    hm1062_2mipimono_COLOR_0740, hm1062_2mipimono_COLOR_0741, hm1062_2mipimono_COLOR_0742, hm1062_2mipimono_COLOR_0743, hm1062_2mipimono_COLOR_0744, hm1062_2mipimono_COLOR_0745, hm1062_2mipimono_COLOR_0746, hm1062_2mipimono_COLOR_0747, hm1062_2mipimono_COLOR_0748, hm1062_2mipimono_COLOR_0749,
    hm1062_2mipimono_COLOR_0750, hm1062_2mipimono_COLOR_0751, hm1062_2mipimono_COLOR_0752, hm1062_2mipimono_COLOR_0753, hm1062_2mipimono_COLOR_0754, hm1062_2mipimono_COLOR_0755, hm1062_2mipimono_COLOR_0756, hm1062_2mipimono_COLOR_0757, hm1062_2mipimono_COLOR_0758, hm1062_2mipimono_COLOR_0759,
    hm1062_2mipimono_COLOR_0760, hm1062_2mipimono_COLOR_0761, hm1062_2mipimono_COLOR_0762, hm1062_2mipimono_COLOR_0763, hm1062_2mipimono_COLOR_0764, hm1062_2mipimono_COLOR_0765, hm1062_2mipimono_COLOR_0766, hm1062_2mipimono_COLOR_0767, hm1062_2mipimono_COLOR_0768, hm1062_2mipimono_COLOR_0769,
    hm1062_2mipimono_COLOR_0770, hm1062_2mipimono_COLOR_0771, hm1062_2mipimono_COLOR_0772, hm1062_2mipimono_COLOR_0773, hm1062_2mipimono_COLOR_0774, hm1062_2mipimono_COLOR_0775, hm1062_2mipimono_COLOR_0776, hm1062_2mipimono_COLOR_0777, hm1062_2mipimono_COLOR_0778, hm1062_2mipimono_COLOR_0779,
    hm1062_2mipimono_COLOR_0780, hm1062_2mipimono_COLOR_0781, hm1062_2mipimono_COLOR_0782, hm1062_2mipimono_COLOR_0783, hm1062_2mipimono_COLOR_0784, hm1062_2mipimono_COLOR_0785, hm1062_2mipimono_COLOR_0786, hm1062_2mipimono_COLOR_0787, hm1062_2mipimono_COLOR_0788, hm1062_2mipimono_COLOR_0789,
    hm1062_2mipimono_COLOR_0790, hm1062_2mipimono_COLOR_0791, hm1062_2mipimono_COLOR_0792, hm1062_2mipimono_COLOR_0793, hm1062_2mipimono_COLOR_0794, hm1062_2mipimono_COLOR_0795, hm1062_2mipimono_COLOR_0796, hm1062_2mipimono_COLOR_0797, hm1062_2mipimono_COLOR_0798, hm1062_2mipimono_COLOR_0799,
    hm1062_2mipimono_COLOR_0800, hm1062_2mipimono_COLOR_0801, hm1062_2mipimono_COLOR_0802, hm1062_2mipimono_COLOR_0803, hm1062_2mipimono_COLOR_0804, hm1062_2mipimono_COLOR_0805, hm1062_2mipimono_COLOR_0806, hm1062_2mipimono_COLOR_0807, hm1062_2mipimono_COLOR_0808, hm1062_2mipimono_COLOR_0809,
    hm1062_2mipimono_COLOR_0810, hm1062_2mipimono_COLOR_0811, hm1062_2mipimono_COLOR_0812, hm1062_2mipimono_COLOR_0813, hm1062_2mipimono_COLOR_0814, hm1062_2mipimono_COLOR_0815, hm1062_2mipimono_COLOR_0816, hm1062_2mipimono_COLOR_0817, hm1062_2mipimono_COLOR_0818, hm1062_2mipimono_COLOR_0819,
    hm1062_2mipimono_COLOR_0820, hm1062_2mipimono_COLOR_0821, hm1062_2mipimono_COLOR_0822, hm1062_2mipimono_COLOR_0823, hm1062_2mipimono_COLOR_0824, hm1062_2mipimono_COLOR_0825, hm1062_2mipimono_COLOR_0826, hm1062_2mipimono_COLOR_0827, hm1062_2mipimono_COLOR_0828, hm1062_2mipimono_COLOR_0829,
    hm1062_2mipimono_COLOR_0830, hm1062_2mipimono_COLOR_0831, hm1062_2mipimono_COLOR_0832, hm1062_2mipimono_COLOR_0833, hm1062_2mipimono_COLOR_0834, hm1062_2mipimono_COLOR_0835, hm1062_2mipimono_COLOR_0836, hm1062_2mipimono_COLOR_0837, hm1062_2mipimono_COLOR_0838, hm1062_2mipimono_COLOR_0839,
    hm1062_2mipimono_COLOR_0840, hm1062_2mipimono_COLOR_0841, hm1062_2mipimono_COLOR_0842, hm1062_2mipimono_COLOR_0843, hm1062_2mipimono_COLOR_0844, hm1062_2mipimono_COLOR_0845, hm1062_2mipimono_COLOR_0846, hm1062_2mipimono_COLOR_0847, hm1062_2mipimono_COLOR_0848, hm1062_2mipimono_COLOR_0849,
    hm1062_2mipimono_COLOR_0850, hm1062_2mipimono_COLOR_0851, hm1062_2mipimono_COLOR_0852, hm1062_2mipimono_COLOR_0853, hm1062_2mipimono_COLOR_0854, hm1062_2mipimono_COLOR_0855, hm1062_2mipimono_COLOR_0856, hm1062_2mipimono_COLOR_0857, hm1062_2mipimono_COLOR_0858, hm1062_2mipimono_COLOR_0859,
    hm1062_2mipimono_COLOR_0860, hm1062_2mipimono_COLOR_0861, hm1062_2mipimono_COLOR_0862, hm1062_2mipimono_COLOR_0863, hm1062_2mipimono_COLOR_0864, hm1062_2mipimono_COLOR_0865, hm1062_2mipimono_COLOR_0866, hm1062_2mipimono_COLOR_0867, hm1062_2mipimono_COLOR_0868, hm1062_2mipimono_COLOR_0869,
    hm1062_2mipimono_COLOR_0870, hm1062_2mipimono_COLOR_0871, hm1062_2mipimono_COLOR_0872, hm1062_2mipimono_COLOR_0873, hm1062_2mipimono_COLOR_0874, hm1062_2mipimono_COLOR_0875, hm1062_2mipimono_COLOR_0876, hm1062_2mipimono_COLOR_0877, hm1062_2mipimono_COLOR_0878, hm1062_2mipimono_COLOR_0879,
    hm1062_2mipimono_COLOR_0880, hm1062_2mipimono_COLOR_0881, hm1062_2mipimono_COLOR_0882, hm1062_2mipimono_COLOR_0883, hm1062_2mipimono_COLOR_0884, hm1062_2mipimono_COLOR_0885, hm1062_2mipimono_COLOR_0886, hm1062_2mipimono_COLOR_0887, hm1062_2mipimono_COLOR_0888, hm1062_2mipimono_COLOR_0889,
    hm1062_2mipimono_COLOR_0890, hm1062_2mipimono_COLOR_0891, hm1062_2mipimono_COLOR_0892, hm1062_2mipimono_COLOR_0893, hm1062_2mipimono_COLOR_0894, hm1062_2mipimono_COLOR_0895, hm1062_2mipimono_COLOR_0896, hm1062_2mipimono_COLOR_0897, hm1062_2mipimono_COLOR_0898, hm1062_2mipimono_COLOR_0899,
    hm1062_2mipimono_COLOR_0900, hm1062_2mipimono_COLOR_0901, hm1062_2mipimono_COLOR_0902, hm1062_2mipimono_COLOR_0903, hm1062_2mipimono_COLOR_0904, hm1062_2mipimono_COLOR_0905, hm1062_2mipimono_COLOR_0906, hm1062_2mipimono_COLOR_0907, hm1062_2mipimono_COLOR_0908, hm1062_2mipimono_COLOR_0909,
    hm1062_2mipimono_COLOR_0910, hm1062_2mipimono_COLOR_0911, hm1062_2mipimono_COLOR_0912, hm1062_2mipimono_COLOR_0913, hm1062_2mipimono_COLOR_0914, hm1062_2mipimono_COLOR_0915, hm1062_2mipimono_COLOR_0916, hm1062_2mipimono_COLOR_0917, hm1062_2mipimono_COLOR_0918, hm1062_2mipimono_COLOR_0919,
    hm1062_2mipimono_COLOR_0920, hm1062_2mipimono_COLOR_0921, hm1062_2mipimono_COLOR_0922, hm1062_2mipimono_COLOR_0923, hm1062_2mipimono_COLOR_0924, hm1062_2mipimono_COLOR_0925, hm1062_2mipimono_COLOR_0926, hm1062_2mipimono_COLOR_0927, hm1062_2mipimono_COLOR_0928, hm1062_2mipimono_COLOR_0929,
    hm1062_2mipimono_COLOR_0930, hm1062_2mipimono_COLOR_0931, hm1062_2mipimono_COLOR_0932, hm1062_2mipimono_COLOR_0933, hm1062_2mipimono_COLOR_0934, hm1062_2mipimono_COLOR_0935, hm1062_2mipimono_COLOR_0936, hm1062_2mipimono_COLOR_0937, hm1062_2mipimono_COLOR_0938, hm1062_2mipimono_COLOR_0939,
    hm1062_2mipimono_COLOR_0940, hm1062_2mipimono_COLOR_0941, hm1062_2mipimono_COLOR_0942, hm1062_2mipimono_COLOR_0943, hm1062_2mipimono_COLOR_0944, hm1062_2mipimono_COLOR_0945, hm1062_2mipimono_COLOR_0946, hm1062_2mipimono_COLOR_0947, hm1062_2mipimono_COLOR_0948, hm1062_2mipimono_COLOR_0949,
    hm1062_2mipimono_COLOR_0950, hm1062_2mipimono_COLOR_0951, hm1062_2mipimono_COLOR_0952, hm1062_2mipimono_COLOR_0953, hm1062_2mipimono_COLOR_0954, hm1062_2mipimono_COLOR_0955, hm1062_2mipimono_COLOR_0956, hm1062_2mipimono_COLOR_0957, hm1062_2mipimono_COLOR_0958, hm1062_2mipimono_COLOR_0959,
},
.PCA = {
    hm1062_2mipimono_PCA_0000,
},
.COLOR_PARAM = {
    hm1062_2mipimono_COLOR_PARAM_0000, hm1062_2mipimono_COLOR_PARAM_0001, hm1062_2mipimono_COLOR_PARAM_0002, hm1062_2mipimono_COLOR_PARAM_0003, hm1062_2mipimono_COLOR_PARAM_0004, hm1062_2mipimono_COLOR_PARAM_0005, hm1062_2mipimono_COLOR_PARAM_0006, hm1062_2mipimono_COLOR_PARAM_0007, hm1062_2mipimono_COLOR_PARAM_0008, hm1062_2mipimono_COLOR_PARAM_0009,
    hm1062_2mipimono_COLOR_PARAM_0010, hm1062_2mipimono_COLOR_PARAM_0011, hm1062_2mipimono_COLOR_PARAM_0012, hm1062_2mipimono_COLOR_PARAM_0013, hm1062_2mipimono_COLOR_PARAM_0014, hm1062_2mipimono_COLOR_PARAM_0015,
},
