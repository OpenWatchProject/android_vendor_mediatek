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

#ifndef AUDIO_TASK_VOW_H
#define AUDIO_TASK_VOW_H

#include <stdbool.h>
#include "audio_task_vow_params.h"

#define VOW_DEBUG 0
#define VOW_ERR(fmt, args...)    PRINTF_I("[vow] ERR: "fmt, ##args)
#define VOW_DBG(fmt, args...)    PRINTF_D("[vow] DBG: "fmt, ##args)
#if VOW_DEBUG
#define VOW_LOG(fmt, args...)    PRINTF_I("[vow] LOG: "fmt, ##args)
#else
#define VOW_LOG(fmt, args...)
#endif

void vow_init(void);
void vow_enable(void);
void vow_disable(void);
void vow_sync_data_ready(void);
void vow_setapreg_addr(unsigned int addr);
unsigned int vow_gettcmreg_addr(void);
void vow_set_flag(vow_flag_t flag, short enable);
void vow_setmode(vow_mode_t mode);
void vow_setModel(vow_event_info_t type, int id, int addr, int size);
uint32_t vow_dma_transaction(uint32_t dst_addr, uint32_t src_addr, uint32_t len);
void vow_setSmartDevice(bool enable, unsigned int eint_num);
#if SMART_AAD_MIC_SUPPORT
unsigned int vow_getEintNumber(unsigned int eint_pin);
void vow_smartdevice_eint_isr(int index);
#endif
#endif  // end of AUDIO_TASK_VOW_H

