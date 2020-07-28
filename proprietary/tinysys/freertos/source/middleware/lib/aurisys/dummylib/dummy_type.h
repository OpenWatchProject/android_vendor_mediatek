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
*
* The following software/firmware and/or related documentation ("MediaTek Software")
* have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
* applicable license agreements with MediaTek Inc.
*/
#ifndef DUMMY_TYPE_H
#define DUMMY_TYPE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif



/* dummy soft data structures */
struct dummy_sph_param_t{
    uint32_t param1[16];
    uint16_t param2[32];
};

struct dummy_epl_buf_t{
    uint32_t frame_index;

    int8_t process_type; // 0: uplink, 1: downlink
    char pcm_debug_buf[1280];

    int16_t  analog_gain;
    int16_t  digital_gain;
    int16_t  mute_on;
    int16_t  enhance_on;
};

struct dummy_lib_handle_t{
    arsi_task_config_t task_config;
    arsi_lib_config_t  lib_config;

    debug_log_fp_t print_log;
    struct dummy_sph_param_t sph_param;

    uint32_t tmp_buf_size;
    uint32_t my_private_var;

    int16_t  ul_analog_gain;
    int16_t  ul_digital_gain;
    int16_t  dl_analog_gain;
    int16_t  dl_digital_gain;

    int16_t  b_ul_mute_on;
    int16_t  b_dl_mute_on;

    int16_t  b_ul_enhance_on;
    int16_t  b_dl_enhance_on;

    uint32_t value_at_addr_0x1234;

    uint32_t frame_index;
};



#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* end of DUMMY_TYPE_H */

