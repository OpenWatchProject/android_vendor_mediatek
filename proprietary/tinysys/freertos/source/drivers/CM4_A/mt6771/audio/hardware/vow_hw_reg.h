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


#define PMIC_6358_SUPPORT                   (1)
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

#define VOW_SAMPLE_NUM_IRQ                (0xAA)
#define VOW_LENGTH_VOICE                  (0x2A8)  // 0x2A8 => 680 / 16.93kSampleRate = 40.165msec, 4*170=680
#define BLISRC_INTERNAL_BUFFER_WORD_SIZE            (528)
#define BLISRC_ACCURATE_INTERNAL_BUFFER_WORD_SIZE   (529)
#define BLISRC_INPUT_BUFFER_SIZE_IN_HALF_WORD       (170)
#define BLISRC_OUTPUT_BUFFER_SIZE_IN_HALF_WORD      (160)  // 10msec voice data
#define BLISRC_INPUT_SAMPLING_RATE_IN_HZ            (18000)
#define BLISRC_OUTPUT_SAMPLING_RATE_IN_HZ           (17000)
#define BLISRC_ACCURATE_INPUT_SAMPLING_RATE_IN_HZ   (1625)
#define BLISRC_ACCURATE_OUTPUT_SAMPLING_RATE_IN_HZ  (1536)
#define BLISRC_INPUT_CHANNEL_NUBMER                 (1)
#define BLISRC_OUTPUT_CHANNEL_NUBMER                (1)
#define BLISRC_OUTPUT_TEMP_BUFFER               (BLISRC_OUTPUT_BUFFER_SIZE_IN_HALF_WORD * 2)
#define AFE_IRQ_SOURCE                          (0x80)

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
#define PMIC_SYS_TOP_REG_BASE                (0x0000)
#define PMIC_AUDIO_SYS_TOP_REG_BASE          (0x6000)

#define AFE_VOW_TOP                          (0x230A)
#define AFE_VOW_TGEN_CFG0                    (0x2328)
#define AFE_VOW_POSDIV_CFG0                  (0x232A)
#define AFE_VOW_HPF_CFG0                     (0x232C)
#define AFE_VOW_CFG0                         (0x230C)
#define AFE_VOW_CFG1                         (0x230E)
#define AFE_VOW_CFG2                         (0x2310)
#define AFE_VOW_CFG3                         (0x2312)
#define AFE_VOW_CFG4                         (0x2314)
#define AFE_VOW_CFG5                         (0x2316)
#define AFE_VOW_CFG6                         (0x2318)
#define AFE_VOW_PERIODIC_CFG0                (0x232E)
#define AFE_VOW_PERIODIC_CFG1                (0x2330)
#define AFE_VOW_PERIODIC_CFG2                (0x2332)
#define AFE_VOW_PERIODIC_CFG3                (0x2334)
#define AFE_VOW_PERIODIC_CFG4                (0x2336)
#define AFE_VOW_PERIODIC_CFG5                (0x2338)
#define AFE_VOW_PERIODIC_CFG6                (0x233A)
#define AFE_VOW_PERIODIC_CFG7                (0x233C)
#define AFE_VOW_PERIODIC_CFG8                (0x233E)
#define AFE_VOW_PERIODIC_CFG9                (0x2340)
#define AFE_VOW_PERIODIC_CFG10               (0x2342)
#define AFE_VOW_PERIODIC_CFG11               (0x2344)
#define AFE_VOW_PERIODIC_CFG12               (0x2346)
#define AFE_VOW_PERIODIC_CFG13               (0x2348)
#define AFE_VOW_PERIODIC_CFG14               (0x234A)
#define AFE_VOW_PERIODIC_CFG15               (0x234C)
#define AFE_VOW_PERIODIC_CFG16               (0x234E)
#define AFE_VOW_PERIODIC_CFG17               (0x2350)
#define AFE_VOW_PERIODIC_CFG18               (0x2352)
#define AFE_VOW_PERIODIC_CFG19               (0x2354)
#define AFE_VOW_PERIODIC_CFG20               (0x2356)
#define AFE_VOW_PERIODIC_CFG21               (0x2358)
#define AFE_VOW_PERIODIC_CFG22               (0x235A)
#define AFE_VOW_PERIODIC_CFG23               (0x235C)
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
