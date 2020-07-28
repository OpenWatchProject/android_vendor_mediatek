/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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
 */

#ifndef _VOW_HW_REG_H
#define _VOW_HW_REG_H

/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Header Files
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
#include "audio_task_vow_params.h"


#define PMIC_6359_SUPPORT                   (1)
#define SW_FIX_METHOD                       (0)
#define PMIC_ADD_DELAY                      (0)  // Only for PMIC6337 & PMIC6355

#define LOCAL_TASK_PRIORITY                 (2)
#define VOW_WAKELOCK  // need to use sw method to lock SCP

/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Defination
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/

#define VOW_SAMPLE_NUM_IRQ                (0xA0)
#define VOW_LENGTH_VOICE                  (0x320)  // 0x320 => 800 / 16kSampleRate = 50msec, 5*160=800
#define BLISRC_INTERNAL_BUFFER_WORD_SIZE            (528)
#define BLISRC_ACCURATE_INTERNAL_BUFFER_WORD_SIZE   (529)
#define BLISRC_INPUT_BUFFER_SIZE_IN_HALF_WORD       (160)
#define BLISRC_OUTPUT_BUFFER_SIZE_IN_HALF_WORD      (160)  // 10msec voice data
#define BLISRC_INPUT_SAMPLING_RATE_IN_HZ            (16000)
#define BLISRC_OUTPUT_SAMPLING_RATE_IN_HZ           (16000)
#define BLISRC_ACCURATE_INPUT_SAMPLING_RATE_IN_HZ   (1625)
#define BLISRC_ACCURATE_OUTPUT_SAMPLING_RATE_IN_HZ  (1632)
#define BLISRC_INPUT_CHANNEL_NUBMER                 (1)
#define BLISRC_OUTPUT_CHANNEL_NUBMER                (1)
#define BLISRC_OUTPUT_TEMP_BUFFER               (BLISRC_OUTPUT_BUFFER_SIZE_IN_HALF_WORD * 2)
#define AFE_IRQ_SOURCE                          (AFE_IRQ14_SOURCE)

#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
#define VOICE_DELAY_FRM                 (3)
#define ECHO_DELAY_FRM                  (3)
#define ECHO_DELAY_SAMPLE               (0)
#endif

//=============================================================================================
//                 SCP Registers
//=============================================================================================
#define VOW_FIFO_BASE                       (0x405C1000)
#define VOW_FIFO_EN                         (VOW_FIFO_BASE + 0x00)  // VIF_FIFO_EN
#define VOW_FIFO_STATUS                     (VOW_FIFO_BASE + 0x04)  // VIF_FIFO_STATUS
#define VOW_FIFO_DATA                       (VOW_FIFO_BASE + 0x08)  // VIF_RXDATA
#define VOW_FIFO_DATA_THRES                 (VOW_FIFO_BASE + 0x0C)  // VIF_FIFO_THRESHOLD
#define VOW_FIFO_IRQ_ACK                    (VOW_FIFO_BASE + 0x10)  // VIF_IRQ_STATUS
#define VOW_RXIF_CFG0                       (VOW_FIFO_BASE + 0x14)  // VIF_RXIF_CFG0
#define VOW_RXIF_CFG1                       (VOW_FIFO_BASE + 0x18)  // VIF_RXIF_CFG1
#define VOW_RXIF_CFG2                       (VOW_FIFO_BASE + 0x1C)  // VIF_RXIF_CFG2
#define VOW_RXIF_OUT                        (VOW_FIFO_BASE + 0x20)
#define VOW_RXIF_STATUS                     (VOW_FIFO_BASE + 0x24)
#define VOW_R_DATAIN_IRQ_EN                 (VOW_FIFO_BASE + 0x28)  // For MTKIF DATA PIN INT

//=============================================================================================
//                 PMIC Registers
//=============================================================================================
#define AFE_VOW_TOP_CON0                     (0x240A)
#define AFE_VOW_TOP_CON1                     (0x240C)
#define AFE_VOW_TOP_CON2                     (0x240E)
#define AFE_VOW_TOP_CON3                     (0x2410)
#define AFE_VOW_TOP_CON4                     (0x2412)
#define AFE_VOW_VAD_CFG0                     (0x2416)
#define AFE_VOW_VAD_CFG1                     (0x2418)
#define AFE_VOW_VAD_CFG2                     (0x241A)
#define AFE_VOW_VAD_CFG3                     (0x241C)
#define AFE_VOW_VAD_CFG4                     (0x241E)
#define AFE_VOW_VAD_CFG5                     (0x2420)
#define AFE_VOW_VAD_CFG6                     (0x2422)
#define AFE_VOW_VAD_CFG7                     (0x2424)
#define AFE_VOW_VAD_CFG8                     (0x2426)
#define AFE_VOW_VAD_CFG9                     (0x2428)
#define AFE_VOW_VAD_CFG10                    (0x242A)
#define AFE_VOW_VAD_CFG11                    (0x242C)
#define AFE_VOW_VAD_CFG12                    (0x242E)
#define AFE_VOW_TGEN_CFG0                    (0x2448)
#define AFE_VOW_TGEN_CFG1                    (0x244A)
#define AFE_VOW_HPF_CFG0                     (0x244C)
#define AFE_VOW_HPF_CFG1                     (0x244E)
#define AFE_VOW_PERIODIC_CFG0                (0x2488)
#define AFE_VOW_PERIODIC_CFG1                (0x248A)
#define AFE_VOW_PERIODIC_CFG2                (0x248C)
#define AFE_VOW_PERIODIC_CFG3                (0x248E)
#define AFE_VOW_PERIODIC_CFG4                (0x2490)
#define AFE_VOW_PERIODIC_CFG5                (0x2492)
#define AFE_VOW_PERIODIC_CFG6                (0x2494)
#define AFE_VOW_PERIODIC_CFG7                (0x2496)
#define AFE_VOW_PERIODIC_CFG8                (0x2498)
#define AFE_VOW_PERIODIC_CFG9                (0x249A)
#define AFE_VOW_PERIODIC_CFG10               (0x249C)
#define AFE_VOW_PERIODIC_CFG11               (0x249E)
#define AFE_VOW_PERIODIC_CFG12               (0x24A0)
#define AFE_VOW_PERIODIC_CFG13               (0x24A2)
#define AFE_VOW_PERIODIC_CFG14               (0x24A4)
#define AFE_VOW_PERIODIC_CFG15               (0x24A6)
#define AFE_VOW_PERIODIC_CFG16               (0x24A8)
#define AFE_VOW_PERIODIC_CFG17               (0x24AA)
#define AFE_VOW_PERIODIC_CFG18               (0x24AC)
#define AFE_VOW_PERIODIC_CFG19               (0x24AE)
#define AFE_VOW_PERIODIC_CFG20               (0x24B0)
#define AFE_VOW_PERIODIC_CFG21               (0x24B2)
#define AFE_VOW_PERIODIC_CFG22               (0x24B4)
#define AFE_VOW_PERIODIC_CFG23               (0x24B6)
#define AFE_VOW_PERIODIC_CFG24               (0x24B8)
#define AFE_VOW_PERIODIC_CFG25               (0x24BA)
#define AFE_VOW_PERIODIC_CFG26               (0x24BC)
#define AFE_VOW_PERIODIC_CFG27               (0x24BE)
#define AFE_VOW_PERIODIC_CFG28               (0x24C0)
#define AFE_VOW_PERIODIC_CFG29               (0x24C2)
#define AFE_VOW_PERIODIC_CFG30               (0x24C4)
#define AFE_VOW_PERIODIC_CFG31               (0x24C6)
#define AFE_VOW_PERIODIC_CFG32               (0x24C8)
#define AFE_VOW_PERIODIC_CFG33               (0x24CA)
#define AFE_VOW_PERIODIC_CFG34               (0x24CC)
#define AFE_VOW_PERIODIC_CFG35               (0x24CE)
#define AFE_VOW_PERIODIC_CFG36               (0x24D0)
#define AFE_VOW_PERIODIC_CFG37               (0x24D2)
#define AFE_VOW_PERIODIC_CFG38               (0x24D4)
#define AFE_VOW_PERIODIC_CFG39               (0x24D6)
#define TOP_CKSEL_CON1                       (0x011E)
#define TOP_CKSEL_CON1_SET                   (0x0120)
#define TOP_CKSEL_CON1_CLR                   (0x0122)
#define BUCK_VCORE_SSHUB_CON0                (0x14A4)
#define LDO_VSRAM_OTHERS_SSHUB_CON0          (0x1BC4)

// For smart device use
#define AUDENC_ANA_CON9                      (0x3630)
#define AUDENC_ANA_CON14                     (0x363A)
#define AUDENC_ANA_CON15                     (0x363C)
#define AUDENC_ANA_CON16                     (0x363E)


#endif  // _VOW_HW_REG_H
