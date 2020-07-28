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

#ifndef _VOW_HW_H
#define _VOW_HW_H

/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Header Files
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
#include <stdbool.h>

#include "audio_task_vow_params.h"


/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Extern
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
void vow_enable_fifo(vowserv_t *vow);
void vow_disable_fifo(void);
void vow_pmic_idleTophase1(vowserv_t *vow);
void vow_pmic_phase2Tophase1(vowserv_t *vow);
void vow_force_phase2(vowserv_t *vow);
void vow_force_phase1(void);
void vow_pmic_idle(vowserv_t *vow);
void vow_dc_removal(char enable, char order);
unsigned short vow_pwrapper_read(unsigned short addr);
void vow_pwrapper_write(unsigned short addr, unsigned short data);
unsigned short vow_pwrapper_read_bits(unsigned short addr, char bits, char len);
void vow_pwrapper_write_bits(unsigned short addr, unsigned short value, char bits, char len);
#if PMIC_6359_SUPPORT
void vow_read_fifo_data(short *voice_buf_L, short *voice_buf_R, unsigned short read_length, vowserv_t *vow);
#else
void vow_read_fifo_data(short *voice_buf, unsigned short read_length);
#endif
void vow_reset_fifo_irq(void);
#if PMIC_6337_SUPPORT
bool vow_flr_setting(void);
void vow_flr_suspend(void);
void vow_flr_resume(void);
void vow_dvfs_mode_setting(char mode);
void vow_datairq_reset(void);
void vow_datairq_clear(void);
void vow_set_periodic_onoff(char enable);
void vow_pmic_phase1Tophase2(char dvfs_mode);
void vow_lock_vow_event(void);
void vow_unlock_vow_event(void);
#endif  // #if PMIC_6337_SUPPORT
#if SMART_AAD_MIC_SUPPORT
void vow_smartdevice_eint_handler(bool smart_device_flag);
#endif  // #if SMART_AAD_MIC_SUPPORT
void vow_clear_force_phase2(void);
#if (PMIC_6358_SUPPORT || PMIC_6359_SUPPORT)
void vow_pmic_low_power_control(vowserv_t *vow, vow_pmic_low_power_t lp_control);
void vow_dynamic_modify_phase1_vad(int value);
#endif  // #if PMIC_6358_SUPPORT
int32_t is_normal_src_additional_frame_accumulated(uint32_t *p_frm_cnt);
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
int32_t is_accurate_src_additional_frame_accumulated(uint32_t *p_frm_cnt);
#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
void vow_headset_detection(vowserv_t *vow);
#endif // #ifdef #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
#endif  // _VOW_HW_H
