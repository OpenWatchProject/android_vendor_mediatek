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
 */

#include <string.h>
#include <stdio.h>

#include <driver_api.h>
#include <feature_manager.h>

#include <dvfs_common.h>

#define UART_EN_MASK     (0x1 << 2)
#define DMA_EN_MASK      (0x1 << 1)
#define TIMER_EN_MASK    (0x1 << 0)
#define CG_EN_MASK       (UART_EN_MASK | DMA_EN_MASK | TIMER_EN_MASK)
#define CG_EN_BITS       (3)

dvfs_swip_config_t config_8k_earpiece = {
    .dl_out.sample_rate = 8000,
    .output_devices = AUDIO_DEVICE_OUT_EARPIECE,
    .num_mics = 2
};  //160 MCPS
dvfs_swip_config_t config_32k_speaker = {
    .dl_out.sample_rate = 32000,
    .output_devices = AUDIO_DEVICE_OUT_SPEAKER,
    .num_mics = 2
};  //250 MCPS

void testsuite_dvfs_dispatcher(uint32_t id) //
{
    PRINTF_D("testsuite_dvfs_dispatcher id(%d)\n", id);
    switch (id) {
        case 0x0:         //deregister audplayback feature  ->320
            dvfs_deregister_feature(VOICE_CALL_FEATURE_ID);
            break;
        case 0x1:         //register audplayback feature  ->321
            dvfs_register_feature(VOICE_CALL_FEATURE_ID);
            break;
        case 0x2:         //reset table
            reset_feature_table();
            break;
        case 0xA:         //add 160 MCPS ->330
            dvfs_add_swip(VOICE_CALL_FEATURE_ID, SWIP_ID_PHONE_CALL_ENH, &config_8k_earpiece);
            break;
        case 0xB:         //add 250 MCPS ->331
            dvfs_add_swip(VOICE_CALL_FEATURE_ID, SWIP_ID_PHONE_CALL_ENH, &config_32k_speaker);
            break;
        case 0x14:        //delete 160 MCPS  ->340
            dvfs_delete_swip(VOICE_CALL_FEATURE_ID, SWIP_ID_PHONE_CALL_ENH, &config_8k_earpiece);
            break;
        case 0x15:        //delete 250 MCPS  ->341
            dvfs_delete_swip(VOICE_CALL_FEATURE_ID, SWIP_ID_PHONE_CALL_ENH, &config_32k_speaker);
            break;
        case 0x16:        // unforce adspppll -> 342
            g_force_adsppll_on = 0;
            break;
        case 0x17:        // force adspppll -> 343
            g_force_adsppll_on = 1;
            break;
        default:
            PRINTF_D("testsuite_dvfs_dispatcher no this testcase id(%d)\n", id);
            break;
    }
}
