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

#ifndef _AUDIO_HW_H
#define _AUDIO_HW_H

/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Header Files
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
#include <stdint.h>
#include <stdbool.h>
#include <audio_task_interface.h>

/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Definitions
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/

#define SetRegBit(variable,bit) variable =   ((variable) | (1<<(bit)))
#define ResetRegBit(variable,bit) variable = ((variable) & (~(1<<(bit))))

#define AFE_SRAM_SIZE    (0xD000)
#define SRAM_ADDR_MAPPING(phyAdd) ((phyAdd)- AFE_INTERNAL_SRAM_PHY_BASE + AFE_INTERNAL_SRAM_MAP_BASE)

enum {
    IRQ_TO_MCU = 0,
    IRQ_TO_SCP = 1,
    IRQ_TO_HIFI3 = 2
};

/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Functions
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
void Afe_Set_Reg(uint32_t offset, uint32_t value, uint32_t mask);
uint32_t Afe_Get_Reg(uint32_t offset);
uint32_t AudDrv_IRQ_handler(uint32_t irq_mode);
void Audio_Enable_Dl(unsigned enable, unsigned char dl_num);

void afe_memif_start(int audio_memtype);
void afe_memif_stop(int audio_memtype);
bool afe_memif_is_dl(int audio_memtype);
unsigned int afe_memif_current(int audio_memtype);
void afe_irq_start(int irq_num);
void afe_irq_stop(int irq_num);
void afe_irq_set_target(int irq_num, int target);

/*
 * reg audio reg and
 * retrun irq num
 */
int adsp_audio_irq_handler();
int get_audio_scene_by_irqline(int irq);
int register_dsp_irq(int irq, AudioTask* task);
void spk_memif_update(uint32_t id);
const struct mtk_memif_reg *spk_memif_get(int id);
void spk_irq_update(int irq_num, int scp_irq_reg_offset);
const struct mtk_irq_info *spk_irq_get(void);
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
int vow_bargein_irqid_check(unsigned int irq_id);
void vow_set_bargein_int(bool enable, unsigned int irq_id);
void vow_set_mem_awb(bool enable);
#endif

#endif //_AUDIO_HW_H
