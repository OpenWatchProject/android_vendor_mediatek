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

#include <FreeRTOS.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <driver_api.h>
#include "audio_hw_reg.h"
#include "audio_hw_type.h"
#include "audio_task.h"
#include "audio_log.h"
#include <interrupt.h>
#include <audio_type.h>

struct mtk_afe_memif_data {
    int reg_ofs_cur;
    int enable_reg;
    int enable_shift;
    bool is_dl;
};

struct mtk_afe_irq_data {
    int irq_en_reg;
    int irq_en_shift;
};

static const struct mtk_afe_irq_data irq_data[MT6779_IRQ_NUM] = {
    [MT6779_IRQ_0] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 0,
    },
    [MT6779_IRQ_1] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 1,
    },
    [MT6779_IRQ_2] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 2,
    },
    [MT6779_IRQ_3] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 3,
    },
    [MT6779_IRQ_4] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 4,
    },
    [MT6779_IRQ_5] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 5,
    },
    [MT6779_IRQ_6] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 6,
    },
    [MT6779_IRQ_7] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 7,
    },
    [MT6779_IRQ_8] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 8,
    },
    [MT6779_IRQ_9] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 9,
    },
    [MT6779_IRQ_10] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 10,
    },
    [MT6779_IRQ_11] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 11,
    },
    [MT6779_IRQ_12] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 12,
    },
    [MT6779_IRQ_13] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 13,
    },
    [MT6779_IRQ_14] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 14,
    },
    [MT6779_IRQ_15] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 15,
    },
    [MT6779_IRQ_16] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 16,
    },
    [MT6779_IRQ_17] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 17,
    },
    [MT6779_IRQ_18] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 18,
    },
    [MT6779_IRQ_19] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 19,
    },
    [MT6779_IRQ_20] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 20,
    },
    [MT6779_IRQ_21] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 21,
    },
    [MT6779_IRQ_22] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 22,
    },
    [MT6779_IRQ_23] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 23,
    },
    [MT6779_IRQ_24] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 24,
    },
    [MT6779_IRQ_25] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 25,
    },
    [MT6779_IRQ_26] = {
        .irq_en_reg = AFE_IRQ_MCU_CON0,
        .irq_en_shift = 26,
    },
};

static const struct mtk_afe_memif_data memif_data[MT6779_MEMIF_NUM] = {
    [MT6779_MEMIF_DL1] = {
        .reg_ofs_cur = AFE_DL1_CUR,
        .enable_reg = AFE_DAC_CON0,
        .enable_shift = DL1_ON_SFT,
        .is_dl = true,
    },
    [MT6779_MEMIF_DL12] = {
        .reg_ofs_cur = AFE_DL12_CUR,
        .enable_reg = AFE_DAC_CON0,
        .enable_shift = DL12_ON_SFT,
        .is_dl = true,
    },
    [MT6779_MEMIF_DL2] = {
        .reg_ofs_cur = AFE_DL2_CUR,
        .enable_reg = AFE_DAC_CON0,
        .enable_shift = DL2_ON_SFT,
        .is_dl = true,
    },
    [MT6779_MEMIF_DL3] = {
        .reg_ofs_cur = AFE_DL3_CUR,
        .enable_reg = AFE_DAC_CON0,
        .enable_shift = DL3_ON_SFT,
        .is_dl = true,
    },
    [MT6779_MEMIF_DL4] = {
        .reg_ofs_cur = AFE_DL4_CUR,
        .enable_reg = AFE_DAC_CON0,
        .enable_shift = DL4_ON_SFT,
        .is_dl = true,
    },
    [MT6779_MEMIF_DL5] = {
        .reg_ofs_cur = AFE_DL5_CUR,
        .enable_reg = AFE_DAC_CON0,
        .enable_shift = DL5_ON_SFT,
        .is_dl = true,
    },
    [MT6779_MEMIF_DL6] = {
        .reg_ofs_cur = AFE_DL6_CUR,
        .enable_reg = AFE_DAC_CON0,
        .enable_shift = DL6_ON_SFT,
        .is_dl = true,
    },
    [MT6779_MEMIF_DL7] = {
        .reg_ofs_cur = AFE_DL7_CUR,
        .enable_reg = AFE_DAC_CON0,
        .enable_shift = DL7_ON_SFT,
        .is_dl = true,
    },
    [MT6779_MEMIF_DL8] = {
        .reg_ofs_cur = AFE_DL8_CUR,
        .enable_reg = AFE_DAC_CON0,
        .enable_shift = DL8_ON_SFT,
        .is_dl = true,
    },
    [MT6779_MEMIF_VUL12] = {
        .reg_ofs_cur = AFE_VUL12_CUR,
        .enable_reg = AFE_DAC_CON0,
        .enable_shift = VUL12_ON_SFT,
        .is_dl = false,
    },
    [MT6779_MEMIF_VUL2] = {
        .reg_ofs_cur = AFE_VUL2_CUR,
        .enable_reg = AFE_DAC_CON0,
        .enable_shift = VUL2_ON_SFT,
        .is_dl = false,
    },
    [MT6779_MEMIF_VUL3] = {
        .reg_ofs_cur = AFE_VUL3_CUR,
        .enable_reg = AFE_DAC_CON0,
        .enable_shift = VUL3_ON_SFT,
        .is_dl = false,
    },
    [MT6779_MEMIF_VUL4] = {
        .reg_ofs_cur = AFE_VUL4_CUR,
        .enable_reg = AFE_DAC_CON0,
        .enable_shift = VUL4_ON_SFT,
        .is_dl = false,
    },
    [MT6779_MEMIF_VUL5] = {
        .reg_ofs_cur = AFE_VUL5_CUR,
        .enable_reg = AFE_DAC_CON0,
        .enable_shift = VUL5_ON_SFT,
        .is_dl = false,
    },
    [MT6779_MEMIF_VUL6] = {
        .reg_ofs_cur = AFE_VUL6_CUR,
        .enable_reg = AFE_DAC_CON0,
        .enable_shift = VUL6_ON_SFT,
        .is_dl = false,
    },
    [MT6779_MEMIF_AWB] = {
        .reg_ofs_cur = AFE_AWB_CUR,
        .enable_reg = AFE_DAC_CON0,
        .enable_shift = AWB_ON_SFT,
        .is_dl = false,
    },
    [MT6779_MEMIF_AWB2] = {
        .reg_ofs_cur = AFE_AWB2_CUR,
        .enable_reg = AFE_DAC_CON0,
        .enable_shift = AWB2_ON_SFT,
        .is_dl = false,
    },
};

/* task can register their irq */
static int mtk_irq_data[MT6779_IRQ_NUM];

/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Static Functions
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/

static uint32_t CheckOffset(uint32_t offset)
{
    if (offset > AFE_MAXLENGTH) {
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

bool afe_memif_is_dl(int audio_memtype)
{
    const struct mtk_afe_memif_data *memif = &memif_data[audio_memtype];

    return memif->is_dl;
}

void afe_memif_start(int audio_memtype)
{
    const struct mtk_afe_memif_data *memif = &memif_data[audio_memtype];

    if (memif->enable_shift < 0) {
        AUD_LOG_E("%s(), error, id %d, enable_shift < 0\n", __func__, audio_memtype);
        return;
    }
    unsigned int memif_reg_value;
    memif_reg_value = Afe_Get_Reg(memif->enable_reg);

    Afe_Set_Reg(memif->enable_reg,
                1 << memif->enable_shift,
                1 << memif->enable_shift);
    AUD_LOG_D("%s() offset %x = 0x%x memif_reg_value = 0x%x audio_memtype = %d\n", __func__,
              memif->enable_reg,
              Afe_Get_Reg(memif->enable_reg), memif_reg_value, audio_memtype);
}

void afe_memif_stop(int audio_memtype)
{
    const struct mtk_afe_memif_data *memif = &memif_data[audio_memtype];

    if (memif->enable_shift < 0) {
        AUD_LOG_E("%s(), error, id %d, enable_shift < 0\n", __func__, audio_memtype);
        return;
    }

    Afe_Set_Reg(memif->enable_reg,
                0 << memif->enable_shift,
                1 << memif->enable_shift);

    AUD_LOG_D("%s() offset %x = 0x%x\n", __func__, memif->enable_reg,
              Afe_Get_Reg(memif->enable_reg));
}

unsigned int afe_memif_current(int audio_memtype)
{
    const struct mtk_afe_memif_data *memif = &memif_data[audio_memtype];

    return (unsigned int)Afe_Get_Reg(memif->reg_ofs_cur);
}

void afe_irq_start(int irq_num)
{
    const struct mtk_afe_irq_data *irq = &irq_data[irq_num];
    unsigned int irq_en_reg_value;
    irq_en_reg_value = Afe_Get_Reg(irq->irq_en_reg);

    /* turn on irq */
    Afe_Set_Reg(irq->irq_en_reg,
                1 << irq->irq_en_shift,
                1 << irq->irq_en_shift);
    AUD_LOG_D("%s() offset %x = 0x%x pre_irq_en_reg_value = 0x%x\n", __func__,
              irq->irq_en_reg,
              Afe_Get_Reg(irq->irq_en_reg), irq_en_reg_value);
}

void afe_irq_stop(int irq_num)
{
    const struct mtk_afe_irq_data *irq = &irq_data[irq_num];

    /* turn off irq */
    Afe_Set_Reg(irq->irq_en_reg,
                0 << irq->irq_en_shift,
                1 << irq->irq_en_shift);
    AUD_LOG_D("%s() offset %x = 0x%x\n", __func__, irq->irq_en_reg,
              Afe_Get_Reg(irq->irq_en_reg));
}

void afe_irq_set_target(int irq_num, int target)
{
    if (target == IRQ_TO_HIFI3) {
        Afe_Set_Reg(AFE_IRQ_MCU_EN, 0x0 << irq_num, 0x1 << irq_num);
        Afe_Set_Reg(AFE_IRQ_MCU_DSP_EN, 0x1 << (irq_num + AFE_IRQ_DSP_EN_SHIFT),
                    0x1 << (irq_num + AFE_IRQ_DSP_EN_SHIFT));
    }
    else {
        Afe_Set_Reg(AFE_IRQ_MCU_DSP_EN, 0x0 << (irq_num + AFE_IRQ_DSP_EN_SHIFT),
                    0x1 << (irq_num + AFE_IRQ_DSP_EN_SHIFT));
        Afe_Set_Reg(AFE_IRQ_MCU_EN, 0x1 << irq_num, 0x1 << irq_num);
    }
    AUD_LOG_V("%s() AFE_IRQ_MCU_EN = 0x%x AFE_IRQ_MCU_DSP_EN = 0x%x\n",
              __func__, Afe_Get_Reg(AFE_IRQ_MCU_EN), Afe_Get_Reg(AFE_IRQ_MCU_DSP_EN));
}

uint32_t AudDrv_IRQ_handler(uint32_t irq_mode)
{
    uint32_t HW_Cur_ReadIdx = 0;

    uint32_t IRQSource, IRQMask;// irq_scp_en, irq_mcu_en = 0;
    IRQSource = Afe_Get_Reg(AFE_IRQ_MCU_STATUS);
    IRQMask = IRQSource;


    /* handler for lat interrupt with irq status is all zero*/
    if ((IRQSource & AFE_IRQ_MASK) == 0) {
        Afe_Set_Reg(AFE_IRQ_MCU_DSP_EN, AFE_IRQ7_FLAG << AFE_IRQ_DSP_EN_SHIFT,
                    AFE_IRQ7_FLAG << AFE_IRQ_DSP_EN_SHIFT);
        Afe_Set_Reg(AFE_IRQ_MCU_CLR, 0xffffffff, 0xffffffff);
        Afe_Set_Reg(AFE_IRQ_MCU_DSP_EN, 0, AFE_IRQ7_FLAG << AFE_IRQ_DSP_EN_SHIFT);
        return 0;
    }

    /*check IRQ 7*/
    IRQMask &= AFE_IRQ7_FLAG;
    Afe_Set_Reg(AFE_IRQ_MCU_CLR, IRQMask, AFE_IRQ_MASK);

    if (IRQSource & (0x1 << irq_mode)) {
        /*update DL read pointer*/
        HW_Cur_ReadIdx = Auddrv_DL3_Interrupt_Handler();
    }
    return HW_Cur_ReadIdx;
}

/* hardware implemetation of irq handler*/
int adsp_audio_irq_handler(void)
{
    uint32_t IRQSource, irq_scp_en , irq_mcu_en = 0;
    IRQSource = Afe_Get_Reg(AFE_IRQ_MCU_STATUS);
    int pos = 0;

    /* handler for lat interrupt with irq status is all zero*/
    if ((IRQSource & AFE_IRQ_MASK) == 0) {
        irq_scp_en = Afe_Get_Reg(AFE_IRQ_MCU_DSP_EN);
        irq_mcu_en = Afe_Get_Reg(AFE_IRQ_MCU_EN);
        AUD_LOG_E("IRQSource = 0x%x irq_scp_en = 0x%x irq_mcu_en = 0x%x ClearPendingIRQ\n",
                  IRQSource, irq_scp_en, irq_mcu_en);
        /* enable all irq en, clear , disable en*/
        Afe_Set_Reg(AFE_IRQ_MCU_DSP_EN, AFE_IRQ_MASK, AFE_IRQ_MASK);
        Afe_Set_Reg(AFE_IRQ_MCU_CLR, 0xffffffff, 0xffffffff);
        Afe_Set_Reg(AFE_IRQ_MCU_DSP_EN, irq_scp_en, AFE_IRQ_MASK);

#ifdef CONFIG_MTK_TINYSYS_SCP_SUPPORT
        NVIC_ClearPendingIRQ(AUDIO_IRQn);
#endif
        return -1;
    }

    IRQSource &= Afe_Get_Reg(AFE_IRQ_MCU_DSP_EN);

    /* check IRQ , clear irq and return irq num*/
    for (pos = 0 ; pos <= MT6779_IRQ_26 ; pos ++ ) {
        if (IRQSource & (1 << pos) ) {
            Afe_Set_Reg(AFE_IRQ_MCU_CLR, (1 << pos), AFE_IRQ_MASK);
            return pos;
        }
    }
    AUD_LOG_D("%s() return -1 \n", __func__);

    return -1;
}

int register_dsp_irq(int irq, AudioTask *task)
{
    if (irq < 0) {
        AUD_LOG_W("%s() irq = 0x%x, EN1 = 0x%x\n", __func__, irq,
                  Afe_Get_Reg(AFE_IRQ_MCU_DSP_EN));
        return -1;
    }
    mtk_irq_data[irq] = task->scene;
    return 0;
}

/* wrapper between irq num and task scene*/
/* todo : register this by task enable*/
int get_audio_scene_by_irqline(int irq)
{
    if (irq < 0 || irq >= MT6779_IRQ_NUM) {
        return -1;
    }
    return mtk_irq_data[irq];
}

