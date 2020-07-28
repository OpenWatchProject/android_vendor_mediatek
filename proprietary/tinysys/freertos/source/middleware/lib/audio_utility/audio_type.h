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

#ifndef AUDIO_TYPE_H
#define AUDIO_TYPE_H

#include <tinysys_config.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include <driver_api.h>

#include <audio_log.h>
#include <audio_assert.h>
#include <audio_ringbuf.h>

struct RingBuf;

struct Audio_Task_Msg_t {
    uint16_t msg_id;     /* defined by user */
    uint32_t param1;     /* see audio_ipi_msg_data_t */
    uint32_t param2;     /* see audio_ipi_msg_data_t */
    union {
        unsigned char payload[256];
        char *dma_addr;  /* for AUDIO_IPI_DMA only */
    };
} ;

enum {
    BITS_PER_SAMPLE_16,
    BITS_PER_SAMPLE_32,
} ;

enum {
    TASK_DL_SR_8K,
    TASK_DL_SR_16K,
    TASK_DL_SR_32K,
    TASK_DL_SR_44K,
    TASK_DL_SR_48K,
    TASK_DL_SR_96K,
    TASK_DL_SR_192K,
};

enum {
    MEMORY_AUDIO_SRAM,
    MEMORY_AUDIO_DRAM,
    MEMORY_SYSTEM_TCM,
};

/* audio mem define*/
enum {
    AUDIO_MEMIF_DL1,
    AUDIO_MEMIF_DL2,
    AUDIO_MEMIF_DL3,
    AUDIO_MEMIF_DL1_2,
    AUDIO_MEMIF_DAI,
    AUDIO_MEMIF_MOD_DAI,
    AUDIO_MEMIF_VUL_2,
    AUDIO_MEMIF_VUL2,
    AUDIO_MEMIF_AWB,
    AUDIO_MEMIF_AWB2,
    AUDIO_MEMIF_NUM,
};

enum {
    AUDIO_IRQ_0,
    AUDIO_IRQ_1,
    AUDIO_IRQ_2,
    AUDIO_IRQ_3,
    AUDIO_IRQ_4,
    AUDIO_IRQ_5,
    AUDIO_IRQ_6,
    AUDIO_IRQ_7,
    AUDIO_IRQ_11,
    AUDIO_IRQ_12,
    AUDIO_IRQ_NUM,
};

enum {
    SNDRV_PCM_FORMAT_S8,
    SNDRV_PCM_FORMAT_U8,
    SNDRV_PCM_FORMAT_S16_LE,
    SNDRV_PCM_FORMAT_S16_BE,
    SNDRV_PCM_FORMAT_U16_LE,
    SNDRV_PCM_FORMAT_U16_BE,
    SNDRV_PCM_FORMAT_S24_LE,
    SNDRV_PCM_FORMAT_S24_BE,
    SNDRV_PCM_FORMAT_U24_LE,
    SNDRV_PCM_FORMAT_U24_BE,
    SNDRV_PCM_FORMAT_S32_LE,
    SNDRV_PCM_FORMAT_S32_BE,
    SNDRV_PCM_FORMAT_U32_LE,
    SNDRV_PCM_FORMAT_U32_BE,
    SNDRV_PCM_FORMAT_FLOAT_LE,
    SNDRV_PCM_FORMAT_FLOAT_BE,
    SNDRV_PCM_FORMAT_FLOAT64_LE,
    SNDRV_PCM_FORMAT_FLOAT64_BE,
};


#endif // end of AUDIO_TYPE_H

