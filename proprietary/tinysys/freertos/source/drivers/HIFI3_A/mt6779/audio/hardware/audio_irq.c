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
#include "audio_irq.h"

#include <interrupt.h>

#include <audio_task_interface.h>
#include <audio_task_factory.h>

/*==============================================================================
 *                     private functions - declaration
 *============================================================================*/

#ifdef CFG_MTK_AURISYS_PHONE_CALL_SUPPORT
static void modem_irq_hanlder_md1(void);
#endif
#ifdef CFG_MTK_AUDIO_FRAMEWORK_SUPPORT
static void adsp_audio_common_irq_hanlder(void);
#endif
#ifdef CFG_A2DP_OFFLOAD_SUPPORT
static void adsp_audio_a2dp_irq_hanlder(void);
#endif

/*==============================================================================
 *                     public functions - implementation
 *============================================================================*/

void audio_irq_init(void)
{
#ifdef CFG_MTK_AURISYS_PHONE_CALL_SUPPORT
    request_irq(CCIF3_IRQn, modem_irq_hanlder_md1, "MD1");
#endif
#ifdef CFG_MTK_AUDIO_FRAMEWORK_SUPPORT
    request_irq(AUDIO_IRQn, adsp_audio_common_irq_hanlder, "AUDIO_COMMON");
#endif
#ifdef CFG_A2DP_OFFLOAD_SUPPORT
    request_irq(CONNSYS2_IRQn, adsp_audio_a2dp_irq_hanlder, "AUDIO_A2DP");
#endif
}


/*==============================================================================
 *                     private functions - implementation
 *============================================================================*/

#ifdef CFG_MTK_AURISYS_PHONE_CALL_SUPPORT
static void modem_irq_hanlder_md1(void)
{
    AudioTask *task = get_task_by_scene(TASK_SCENE_PHONE_CALL);
    if (task != NULL) {
        task->irq_hanlder(task, (uint32_t)CCIF3_IRQn);
    }
    AUD_ASSERT(task != NULL);
}

#endif

#ifdef CFG_MTK_AUDIO_FRAMEWORK_SUPPORT
static void adsp_audio_common_irq_hanlder(void)
{
    AudioTask *task = NULL;

    /* get task irq interrut*/
    task = get_task_by_irq((unsigned int)AUDIO_IRQn);
    if (task != NULL) {
        task->irq_hanlder(task, (uint32_t)AUDIO_IRQn);
        return;
    }
    else {
        AUD_LOG_W("%s() no task", __func__);
    }
}

#ifdef CFG_A2DP_OFFLOAD_SUPPORT
static void adsp_audio_a2dp_irq_hanlder(void) {
    AudioTask *task = NULL;
    task = get_task_by_scene(TASK_SCENE_A2DP);
    if (task != NULL) {
        task->irq_hanlder(task, (uint32_t)CONNSYS2_IRQn);
    }
}
#endif
#endif

