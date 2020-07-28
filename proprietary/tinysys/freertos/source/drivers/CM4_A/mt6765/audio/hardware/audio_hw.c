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
/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Header Files
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
#include "audio_hw.h"
#include "audio_hw_reg.h"
#include <FreeRTOS.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <driver_api.h>
#include "audio_hw_reg.h"
#if defined(CFG_MTK_SPEAKER_PROTECTION_SUPPORT)
#include "audio_task_speaker_protection_params.h"
#endif
/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Static Functions
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/

static uint32_t CheckOffset(uint32_t offset)
{
    if (offset > AFE_MAXLENGTH || !offset) {
        return 0;
    }

    return 1;
}
static uint32_t Auddrv_DL3_Interrupt_Handler(void)
{
    uint32_t HW_Cur_ReadIdx = 0;
    HW_Cur_ReadIdx = Afe_Get_Reg(AFE_DL3_CUR);
    return HW_Cur_ReadIdx;
}

/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        AFE Reg Access API
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/


void Afe_Set_Reg(uint32_t offset, uint32_t value, uint32_t mask)
{
    volatile uint32_t *AFE_Register;
    volatile uint32_t val_tmp;

    if (CheckOffset(offset) == 0) {
        AUD_LOG_E("Afe_Set_Reg error:%d", offset);
        return;
    }

    AFE_Register = (volatile uint32_t *)offset;
    val_tmp = Afe_Get_Reg(offset);
    val_tmp &= (~mask);
    val_tmp |= (value & mask);
    drv_write_reg32(AFE_Register, val_tmp);
}

uint32_t Afe_Get_Reg(uint32_t offset)
{
    if (CheckOffset(offset) == 0) {
        AUD_LOG_E("Afe_Get_Reg error:%d", offset);
        return 0xffffffff;
    }
    return (*(volatile unsigned int *)(offset));
}

void Audio_Enable_Dl(unsigned enable, unsigned char dl_num)
{
    if (enable) {
        Afe_Set_Reg(AFE_DAC_CON0, 1 << dl_num, 1 << dl_num);
    }
    else {
        Afe_Set_Reg(AFE_DAC_CON0, 0 << dl_num, 1 << dl_num);
    }
}



uint32_t AudDrv_IRQ_handler(uint32_t irq_mode)
{
    uint32_t IRQSource, irq_mcu_clr = 0, irq_scp_en = 0;
    uint32_t HW_Cur_ReadIdx = 0;
    IRQSource = Afe_Get_Reg(AFE_IRQ_MCU_STATUS);
    IRQSource &= AFE_IRQ_MASK;

    if ((IRQSource & AFE_IRQ_MASK) == 0) {
        irq_scp_en = Afe_Get_Reg(AFE_IRQ_MCU_EN1);
        irq_scp_en &= 0x7fff0000;
        irq_mcu_clr = (irq_scp_en)>>16;
        Afe_Set_Reg(AFE_IRQ_MCU_CLR, irq_mcu_clr, irq_mcu_clr);
    }
    /*check IRQ 7*/
    Afe_Set_Reg(AFE_IRQ_MCU_CLR, IRQSource, AFE_IRQ_MASK);

    if (IRQSource & (0x1 << irq_mode)) {
        /*update DL read pointer*/
        HW_Cur_ReadIdx = Auddrv_DL3_Interrupt_Handler();
    }
    return HW_Cur_ReadIdx;
}

#if defined(CFG_MTK_SPEAKER_PROTECTION_SUPPORT)
static struct mtk_memif_reg memif_reg[SPK_MEMIF_NUM];
static struct mtk_irq_info irq_info;

void spk_memif_update(uint32_t id)
{
    PRINTF("%s id = %d", __FUNCTION__, id);
    switch (id) {
    case Soc_Aud_Digital_Block_MEM_DL1:
        memif_reg[SPK_MEMIF_DL].reg_base = AFE_DL1_BASE;
        memif_reg[SPK_MEMIF_DL].reg_end = AFE_DL1_END;
        memif_reg[SPK_MEMIF_DL].reg_cur = AFE_DL1_CUR;
        break;
    case Soc_Aud_Digital_Block_MEM_AWB:
        memif_reg[SPK_MEMIF_MD_UL].reg_base = AFE_AWB_BASE;
        memif_reg[SPK_MEMIF_MD_UL].reg_end = AFE_AWB_END;
        memif_reg[SPK_MEMIF_MD_UL].reg_cur = AFE_AWB_CUR;
        break;
    case Soc_Aud_Digital_Block_MEM_AWB2:
        memif_reg[SPK_MEMIF_IV].reg_base = AFE_AWB2_BASE;
        memif_reg[SPK_MEMIF_IV].reg_end = AFE_AWB2_END;
        memif_reg[SPK_MEMIF_IV].reg_cur = AFE_AWB2_CUR;
        break;
    case Soc_Aud_Digital_Block_MEM_VUL_DATA2:
        memif_reg[SPK_MEMIF_IV].reg_base = AFE_VUL_D2_BASE;
        memif_reg[SPK_MEMIF_IV].reg_end = AFE_VUL_D2_END;
        memif_reg[SPK_MEMIF_IV].reg_cur = AFE_VUL_D2_CUR;
        break;
    default:
        PRINTF("memif not support");
        break;
    };
}

const struct mtk_memif_reg *spk_memif_get(int id)
{
    if (id >= SPK_MEMIF_NUM || id < 0) {
        PRINTF("out of SPK_MEMIF_NUM, return NULL");
        return NULL;
    }
    return &memif_reg[id];
}

void spk_irq_update(int irq_num, int scp_irq_reg_offset)
{
    if (irq_num >= Soc_Aud_IRQ_MCU_MODE_NUM ||
        irq_num < 0) {
        PRINTF("irq num not support");
        return;
    }

    irq_info.irq_mode = irq_num;
    irq_info.irq_shift_bit = 0x1 << irq_num;

    AUD_ASSERT(scp_irq_reg_offset != 0);
    irq_info.scp_irq_reg = AFE_BASE + scp_irq_reg_offset;
}

const struct mtk_irq_info *spk_irq_get(void)
{
    return &irq_info;
}

#endif

