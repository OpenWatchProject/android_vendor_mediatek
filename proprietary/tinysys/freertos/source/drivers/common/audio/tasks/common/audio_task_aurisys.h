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
#ifndef AUDIO_TASK_COMMON_AURYSYS_H
#define AUDIO_TASK_COMMON_AURYSYS_H

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT

#include <stdint.h>
#include <stdbool.h>
#include "aurisys_lib_handler.h"

struct aurisys_dsp_config_t;
struct aurisys_lib_manager_t;

struct audio_pool_buf_t;
struct data_buf_t;
struct alock_t;


struct audio_aurisys_handle_t {
    struct aurisys_dsp_config_t *dsp_config;

    struct aurisys_lib_manager_t *manager;

    struct audio_pool_buf_t *mAudioPoolBufUlIn;
    struct audio_pool_buf_t *mAudioPoolBufUlOut;
    struct audio_pool_buf_t *mAudioPoolBufUlAec;
    struct audio_pool_buf_t *mAudioPoolBufDlIn;
    struct audio_pool_buf_t *mAudioPoolBufDlOut;
    struct audio_pool_buf_t *mAudioPoolBufDlIV;
    struct data_buf_t *mLinearOut;
    struct alock_t *mAurisysLibManagerLock;
};



/* aurisys related API */
void CreateAurisysLibManager(
    struct audio_aurisys_handle_t *aurisys_handle,
    struct data_buf_t *param_list,
    unsigned char task_scene);

void DestroyAurisysLibManager(
    struct audio_aurisys_handle_t *aurisys_handle,
    unsigned char task_scene);


bool UpdateAurisysLibParam(
    struct audio_aurisys_handle_t *aurisys_handle,
    void *lib_name_param,
    uint32_t lib_name_param_sz);


bool ApplyAurisysLibParamList(
    struct audio_aurisys_handle_t *aurisys_handle,
    struct data_buf_t *param_list,
    unsigned char task_scene);



#endif /* end of MTK_AURISYS_FRAMEWORK_SUPPORT */

#endif /* end of AUDIO_TASK_COMMON_AURYSYS_H */

