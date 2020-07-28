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
#include "vow_hw_reg.h"

/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Options
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/

#define VOW_HAS_SWIP (1)
#define TIMES_DEBUG (0)

#if VOW_DEBUG
#define PHASE_DEBUG (1)
#else
#define PHASE_DEBUG (0)
#endif

#define __VOW_GOOGLE_SUPPORT__              (0)
#define __VOW_MTK_SUPPORT__                 (1)
#define SMART_AAD_MIC_SUPPORT               (0)

// #define __SEAMLESS_RECORD_SUPPORT__
#define CHECK_RECOG_TIME

/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Definitions
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
#define LOCAL_TASK_STACK_SIZE (384)
#define LOCAL_TASK_NAME "vow"

#define MAX_MSG_QUEUE_SIZE (20)


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
#define VOW_LENGTH_INIT                   (0x2)  // 0x3800
#define VOW_VOICEDATA_OFFSET              (0xA000)  /* VOW VOICE DATA DRAM OFFSET */
#if (__VOW_GOOGLE_SUPPORT__)
#define VOW_LENGTH_SPEAKER                (0x5000)  // 0x9E54 / 2 = 4F2A
#else
#define VOW_LENGTH_SPEAKER                (0x4A00)
#endif

#define SEAMLESS_RECORD_LENGTH            (0x7D00)
#if defined(__SEAMLESS_RECORD_SUPPORT__)
#define VOW_LENGTH_VOICE                  (SEAMLESS_RECORD_LENGTH)  // 0x7D00 => 32000 / 16kSampleRate = 2sec
#else  // #if defined(__SEAMLESS_RECORD_SUPPORT__)
#define VOW_LENGTH_VOICE                  (0x280)  // 0x280 => 640 / 16kSampleRate = 40msec, 4*160=640
#endif  // #if defined(__SEAMLESS_RECORD_SUPPORT__)

#define VOW_DROP_FRAME_FOR_DC             (0x1E)  // wait for dc stable in 30(0x1E) * 10msec = 300msec

#define VOW_SAMPLE_NUM_IRQ                (0xA0)
#define VOW_SAMPLE_NUM_FRAME              (160)

#define VOW_RESULT_NOTHING                (0x0)
#define VOW_RESULT_PASS                   (0x1)
#define VOW_RESULT_FAIL                   (0x2)
#define VOW_RESULT_CHANGE_MODEL           (0x3)

// After receiving SEAMLESS_RECORD flag, let accumulate 20msec
// then start send voiceafter receiving SEAMLESS_RECORD flag
#define SEAMLESS_DELAY                    (2)

#define PCMDUMP_BUF_CNT                   (16)
#define WORD_H                            (8)
#define WORD_L                            (0)

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
    VOW_FLAG_FORCE_PHASE2_DEBUG,
    VOW_FLAG_SWIP_LOG_PRINT,
    VOW_FLAG_MTKIF_TYPE,
} vow_flag_t;

typedef enum samplerate_t {
    VOW_SAMPLERATE_16K = 0,
    VOW_SAMPLERATE_32K
} samplerate_t;

typedef enum vow_dvfs_mode_t {
    VOW_DVFS_HW_MODE = 0,
    VOW_DVFS_SW_MODE,
    VOW_DVFS_SW_MODE_DONE
} vow_dvfs_mode_t;

typedef enum vow_ipi_result {
    VOW_IPI_SUCCESS = 0,
    VOW_IPI_CLR_SMODEL_ID_NOTMATCH,
    VOW_IPI_SET_SMODEL_NO_FREE_SLOT,
} vow_ipi_result;

typedef enum vow_ipi_msgid_t {
    IPIMSG_VOW_ENABLE = 0,
    IPIMSG_VOW_DISABLE = 1,
    IPIMSG_VOW_SETMODE = 2,
    IPIMSG_VOW_APREGDATA_ADDR = 3,
    IPIMSG_VOW_SET_MODEL = 4,
    IPIMSG_VOW_SET_FLAG = 5,
    IPIMSG_VOW_SET_SMART_DEVICE = 6,
    IPIMSG_VOW_DATAREADY_ACK = 7,

    IPIMSG_VOW_DATAREADY = 8,
    IPIMSG_VOW_RECOGNIZE_OK = 9,

    VOW_RUN = 15,
} vow_ipi_msgid_t;

typedef enum vow_scplock_t {
    SCP_UNLOCK = 0,
    SCP_LOCK,
} vow_scplock_t;

typedef enum vow_force_phase {
    NO_FORCE = 0,
    FORCE_PHASE1,
    FORCE_PHASE2,
} vow_force_phase;

typedef enum vow_swip_log {
    VOW_SWIP_NO_LOG = 0,
    VOW_SWIP_HAS_LOG,
} vow_swip_log;

typedef enum vow_pmic_low_power_t {
    VOW_PMIC_LOW_POWER_INIT = 0,
    VOW_PMIC_LOW_POWER_ENTRY,
    VOW_PMIC_LOW_POWER_EXIT,
} vow_pmic_low_power_t;

typedef enum vow_mtkif_type {
    VOW_MTKIF_NONE = 0,
    VOW_MTKIF_AMIC,
    VOW_MTKIF_DMIC,
    VOW_MTKIF_DMIC_LP,
} vow_mtkif_type;


typedef struct vowserv_t {
    vow_status_t             vow_status;
    short                    read_idx;
    short                    write_idx;
    bool                     pre_learn;
    bool                     record_flag;
    bool                     debug_P1_flag;
    bool                     debug_P2_flag;
    bool                     swip_log_anable;
    bool                     dmic_low_power;
    bool                     dc_removal_enable;
    char                     dc_removal_order;
    bool                     can_unlock_scp_flag;
    vow_scplock_t            scpwakelock;
#if SW_FIX_METHOD
    bool                     checkDataClean;
#endif  // #if SW_FIX_METHOD
#if PMIC_6337_SUPPORT
    bool                     flr_enable;
    bool                     periodic_need_enable;
    bool                     open_periodic_flag;
#endif  // #if PMIC_6337_SUPPORT
    char                     dvfs_mode;
    int                      drop_count;
    int                      drop_frame;
    int                      apreg_addr;
    int                      samplerate;
    int                      data_length;
    int                      noKeywordCount;
#if SMART_AAD_MIC_SUPPORT
    bool                     smart_device_need_enable;
    bool                     smart_device_flag;
    int                      smart_device_eint_id;
#endif
#ifdef __SEAMLESS_RECORD_SUPPORT__
    short                    read_buf_idx;
    bool                     seamless_record;
    bool                     sync_data_ready;
#endif  // #ifdef __SEAMLESS_RECORD_SUPPORT__
    int                      pcmdump_cnt;
    vow_model_t              vow_model_event;
#if PMIC_6358_SUPPORT
    bool                     first_irq_flag;
#endif  // #if PMIC_6358_SUPPORT
    char                     mtkif_type;
} vowserv_t;

typedef struct {
    vow_status_t             vow_status;
    bool                     debug_P1_flag;
    bool                     debug_P2_flag;
    bool                     swip_log_anable;
    int                      apreg_addr;
    vow_model_t              vow_model_event;
} do_save_vow_t;

#endif  // end of AUDIO_TASK_VOW_PARAMS_H

