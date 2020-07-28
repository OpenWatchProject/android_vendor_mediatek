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

#ifndef _AUDIO_HW_REG_H
#define _AUDIO_HW_REG_H


#define AFE_INTERNAL_SRAM_CM4_MAP_BASE (0xD1221000)
#define AFE_INTERNAL_SRAM_PHY_BASE  (0x11221000)
#define AFE_BASE                (0xD1220000)

#define AFE_IRQ1_FLAG 0x1
#define AFE_IRQ1_SOURCE 0x01
#define AFE_IRQ2_FLAG 0x2
#define AFE_IRQ2_SOURCE 0x02
#define AFE_IRQ7_FLAG 0x80
#define AFE_IRQ7_SOURCE 0x80

#define DL3_ON       5

#define AFE_IRQ_MASK 0x7fff

#define AFE_DAC_CON0            (AFE_BASE + 0x0010)
#define AFE_DAC_CON1            (AFE_BASE + 0x0014)
#define AFE_DL1_BASE            (AFE_BASE + 0x040)
#define AFE_DL1_CUR             (AFE_BASE + 0x044)
#define AFE_DL1_END             (AFE_BASE + 0x048)
#define AFE_AWB_BASE            (AFE_BASE + 0x0070)
#define AFE_AWB_END             (AFE_BASE + 0x0078)
#define AFE_AWB_CUR             (AFE_BASE + 0x007C)
#define AFE_VUL_D2_BASE         (AFE_BASE + 0x0350)
#define AFE_VUL_D2_END          (AFE_BASE + 0x0358)
#define AFE_VUL_D2_CUR          (AFE_BASE + 0x035C)
#define AFE_AWB2_BASE           (AFE_BASE + 0x0bd0)
#define AFE_AWB2_END            (AFE_BASE + 0x0bd8)
#define AFE_AWB2_CUR            (AFE_BASE + 0x0bdc)
#define AFE_AWB2_BASE_MSB       (AFE_BASE + 0x0be0)
#define AFE_AWB2_END_MSB        (AFE_BASE + 0x0be8)
#define AFE_AWB2_CUR_MSB        (AFE_BASE + 0x0bec)
#define AFE_DL3_BASE            (AFE_BASE + 0x0360)
#define AFE_DL3_END             (AFE_BASE + 0x0368)
#define AFE_DL3_CUR             (AFE_BASE + 0x0364)
#define AFE_DAI2_BASE           (AFE_BASE + 0x08b0)
#define AFE_DAI2_END            (AFE_BASE + 0x08b8)
#define AFE_DAI2_CUR            (AFE_BASE + 0x08bc)
#define AFE_IRQ_MCU_CON         (AFE_BASE + 0x03A0)
#define AFE_IRQ_MCU_CON1        (AFE_BASE + 0x02E4)
#define AFE_IRQ_MCU_STATUS      (AFE_BASE + 0x03A4)
#define AFE_IRQ_MCU_EN          (AFE_BASE + 0x03B4)
#define AFE_IRQ_MCU_EN1         (AFE_BASE + 0x030C)
#define AFE_IRQ_MCU_CLR         (AFE_BASE + 0x03A8)
#define AFE_IRQ_MCU_CNT4        (AFE_BASE + 0x03E8)

#define AFE_MAXLENGTH           (AFE_BASE + 0x0da0)  // mt6799

#define SCP_IRQ_EN_REG          AFE_IRQ_MCU_EN1
#define SCP_IRQ_EN_REG_MASK     (0xffff0000)
#define SCP_IRQ_SHIFT_BIT       (16)

enum Soc_Aud_IRQ_MCU_MODE {
    Soc_Aud_IRQ_MCU_MODE_IRQ0_MCU_MODE = 0,
    Soc_Aud_IRQ_MCU_MODE_IRQ1_MCU_MODE,
    Soc_Aud_IRQ_MCU_MODE_IRQ2_MCU_MODE,
    Soc_Aud_IRQ_MCU_MODE_IRQ3_MCU_MODE,
    Soc_Aud_IRQ_MCU_MODE_IRQ4_MCU_MODE,
    Soc_Aud_IRQ_MCU_MODE_IRQ5_MCU_MODE,
    Soc_Aud_IRQ_MCU_MODE_IRQ6_MCU_MODE,
    Soc_Aud_IRQ_MCU_MODE_IRQ7_MCU_MODE,
    Soc_Aud_IRQ_MCU_MODE_IRQ8_MCU_MODE,
    Soc_Aud_IRQ_MCU_MODE_IRQ9_MCU_MODE,
    Soc_Aud_IRQ_MCU_MODE_IRQ10_MCU_MODE,
    Soc_Aud_IRQ_MCU_MODE_IRQ11_MCU_MODE,
    Soc_Aud_IRQ_MCU_MODE_IRQ12_MCU_MODE,
    Soc_Aud_IRQ_MCU_MODE_IRQ_ACC1_MCU_MODE,
    Soc_Aud_IRQ_MCU_MODE_IRQ_ACC2_MCU_MODE,
    Soc_Aud_IRQ_MCU_MODE_NUM
};

enum soc_aud_digital_block {
    /* memmory interfrace */
    Soc_Aud_Digital_Block_MEM_DL1 = 0,
    Soc_Aud_Digital_Block_MEM_DL1_DATA2,
    Soc_Aud_Digital_Block_MEM_DL2,
    Soc_Aud_Digital_Block_MEM_VUL,
    Soc_Aud_Digital_Block_MEM_VUL2,
    Soc_Aud_Digital_Block_MEM_DAI,
    Soc_Aud_Digital_Block_MEM_DL3,
    Soc_Aud_Digital_Block_MEM_AWB,
    Soc_Aud_Digital_Block_MEM_MOD_DAI,
    /* currently no use */
    Soc_Aud_Digital_Block_MEM_VUL_DATA2,
    Soc_Aud_Digital_Block_MEM_DAI2,
    Soc_Aud_Digital_Block_MEM_AWB2,
    Soc_Aud_Digital_Block_MEM_I2S, /* this is not actually a mem if... */
    Soc_Aud_Digital_Block_MEM_HDMI,
    /* ADDA */
    Soc_Aud_Digital_Block_ADDA_DL,
    Soc_Aud_Digital_Block_ADDA_DL2,
    Soc_Aud_Digital_Block_ADDA_UL,
    Soc_Aud_Digital_Block_ADDA_UL2,
    Soc_Aud_Digital_Block_ADDA_VOW,
    Soc_Aud_Digital_Block_ADDA_ANC,
    Soc_Aud_Digital_Block_ADDA_ALL,
    /* connection to int main modem */
    Soc_Aud_Digital_Block_MODEM_PCM_1_O,
    /* connection to extrt/int modem */
    Soc_Aud_Digital_Block_MODEM_PCM_2_O,
    /* 1st I2S for DAC and ADC */
    Soc_Aud_Digital_Block_I2S_OUT_DAC,
    Soc_Aud_Digital_Block_I2S_OUT_DAC_2, /* 4 channel */
    Soc_Aud_Digital_Block_I2S_IN_ADC,
    Soc_Aud_Digital_Block_I2S_IN_ADC_2, /* 4 channel */
    /*  I2S2 input    */
    Soc_Aud_Digital_Block_I2S_IN,
    /* 2nd I2S */
    Soc_Aud_Digital_Block_I2S_OUT_2,
    Soc_Aud_Digital_Block_I2S_IN_2,
    /* CONNSYS I2S */
    Soc_Aud_Digital_Block_I2S_IN_CONNSYS,
    /* HW gain contorl */
    Soc_Aud_Digital_Block_HW_GAIN1,
    Soc_Aud_Digital_Block_HW_GAIN2,
    /* megrge interface */
    Soc_Aud_Digital_Block_MRG_I2S_OUT,
    Soc_Aud_Digital_Block_MRG_I2S_IN,
    Soc_Aud_Digital_Block_DAI_BT,
    Soc_Aud_Digital_Block_NUM_OF_DIGITAL_BLOCK,
    Soc_Aud_Digital_Block_NUM_OF_MEM_INTERFACE = Soc_Aud_Digital_Block_MEM_HDMI + 1
};

#endif  // _AUDIO_HW_REG_H

