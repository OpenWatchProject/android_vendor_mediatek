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
#ifndef AUDIO_TASK_VOW_PARAMS_H
#define AUDIO_TASK_VOW_PARAMS_H


/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Header Files
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
#include <stdbool.h>

/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Options
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/

#define VOW_HAS_SWIP 1
#define TIMES_DEBUG 1

#if VOW_DEBUG
#define PHASE_DEBUG 1
#else
#define PHASE_DEBUG 0
#endif

#define __VOW_GOOGLE_SUPPORT__ 0
#define __VOW_MTK_SUPPORT__ 1
#define __SEAMLESS_RECORD_SUPPORT__ 0

#define SMART_AAD_MIC_SUPPORT 0

/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Definitions
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
#ifndef false
#define false   0
#endif

#ifndef true
#define true    1
#endif

#ifndef NULL
#define NULL    0
#endif

#ifndef bool
typedef unsigned char  bool;
#endif

// #define VOW_MAX_SPEAKER_MODEL             0xA
#define VOW_LENGTH_INIT                   0x2  // 0x3800
#if (__VOW_GOOGLE_SUPPORT__)
#define VOW_LENGTH_SPEAKER                0x5000  // 0x9E54 / 2 = 4F2A
#else
#define VOW_LENGTH_SPEAKER                0x4A00
#endif
// #define VOW_LENGTH_GOOGLE_SPEAKER         0xA8E4
#define VOW_LENGTH_NOISE                  0x2
#define VOW_LENGTH_FIR                    0x2
#if (__VOW_GOOGLE_SUPPORT__)
#define VOW_LENGTH_VOICE                  0x7D00  // 0x7D00 => 32000 / 16kSampleRate = 2sec => 200*160word
#else
#define VOW_LENGTH_VOICE                  0x280  // 0x280 => 640 / 16kSampleRate = 40msec
#endif
#define VOW_LENGTH_VOICE_HALF             (VOW_LENGTH_VOICE >> 1)
#define VOW_LENGTH_VOWREG                 0x00A0
#define VOW_PRELEARN_FRAME                0x80
#define VOW_DC_CALIBRATION_FRAME          0xC8
#define VOW_DROP_FRAME                    0x28
#define VOW_DROP_FRAME_FOR_DC             0xC8

#define VOW_SAMPLE_NUM_IRQ                (VOW_LENGTH_VOWREG)  // (VOW_LENGTH_VOWREG >> 1)
#define VOW_SAMPLE_NUM_FRAME              (VOW_LENGTH_VOWREG)

#define VOW_RESULT_NOTHING                0x0
#define VOW_RESULT_PASS                   0x1
#define VOW_RESULT_FAIL                   0x2
#define VOW_RESULT_CHANGE_MODEL           0x3

// After receiving SEAMLESS_RECORD flag, let accumulate 20msec
// then start send voice after receiving SEAMLESS_RECORD flag
#define SEAMLESS_DELAY                    2

#define PCMDUMP_BUF_CNT                   16

#define VOW_IPI_TIMEOUT                   100  // 1ms


/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Enumrations & Data Structures
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
typedef struct {
    void   (*func)(void);
    int    addr;
    int    size;
    short  *data;
    bool   flag;
} vow_model_t;

typedef struct {
    int     id;
    int     addr;
    int     size;
    bool    enable;
} vow_key_model_t;

typedef struct {
    unsigned int     dump_buf_offset;
    unsigned int     dump_size;
} vow_pcmdump_info_t;

typedef enum vow_event_info_t {
    VOW_MODEL_INIT = 0,
    VOW_MODEL_SPEAKER,
    VOW_MODEL_NOISE,
    VOW_MODEL_FIR,
    // Add before this
    VOW_MODEL_SIZE
} vow_event_info_t;

typedef enum vow_status_t {
    VOW_STATUS_IDLE = 0,
    VOW_STATUS_SCP_REG,
    VOW_STATUS_AP_REG
} vow_status_t;

typedef enum vow_mode_t {
    VOW_MODE_SCP_VOW = 0,
    VOW_MODE_VOICECOMMAND,
    VOW_MODE_MULTIPLE_KEY,
    VOW_MODE_MULTIPLE_KEY_VOICECOMMAND
} vow_mode_t;

typedef enum pmic_status_t {
    PMIC_STATUS_OFF = 0,
    PMIC_STATUS_PHASE1,
    PMIC_STATUS_PHASE2
} pmic_status_t;

typedef enum vow_flag_t {
    VOW_FLAG_DEBUG = 0,
    VOW_FLAG_PRE_LEARN,
    VOW_FLAG_DMIC_LOWPOWER,
    VOW_FLAG_PERIODIC_ENABLE,
    VOW_FLAG_FORCE_PHASE1_DEBUG,
    VOW_FLAG_FORCE_PHASE2_DEBUG
} vow_flag_t;

typedef enum samplerate_t {
    VOW_SAMPLERATE_16K = 0,
    VOW_SAMPLERATE_32K
} samplerate_t;




#endif  // end of AUDIO_TASK_VOW_PARAMS_H

