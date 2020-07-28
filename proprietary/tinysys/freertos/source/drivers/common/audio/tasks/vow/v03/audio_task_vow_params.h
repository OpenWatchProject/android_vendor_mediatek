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

#ifdef CFG_MTK_VOW_VENDOR_GOOGLE_SUPPORT
#define __VOW_GOOGLE_SUPPORT__
#endif
#ifdef CFG_MTK_VOW_VENDOR_MTK_SUPPORT
#define __VOW_MTK_SUPPORT__
#endif
#ifdef CFG_MTK_VOW_VENDOR_AMAZON_SUPPORT
#define __VOW_AMAZON_SUPPORT__
#endif
#define SMART_AAD_MIC_SUPPORT               (0)


#define CHECK_RECOG_TIME

#define BLISRC_USE

#ifdef PMIC_6359_SUPPORT
#define NORMAL_SRC_DIRECT_OUT
#endif

#ifndef DRV_WriteReg32
#define DRV_WriteReg32(addr,data)   ((*(volatile unsigned int *)(addr)) = (unsigned int)data)
#endif
#ifndef DRV_Reg32
#define DRV_Reg32(addr)             (*(volatile unsigned int *)(addr))
#endif

/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Definitions
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
// #define LOCAL_TASK_STACK_SIZE (384)
#define LOCAL_TASK_STACK_SIZE (768)
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

#ifdef CFG_MTK_VOW_2E2K_SUPPORT
#define MAX_VOW_SPEAKER_MODEL             (2)
#else
#define MAX_VOW_SPEAKER_MODEL             (1)
#endif

#define VOW_LENGTH_INIT                   (0x2)  // 0x3800

#ifdef CFG_MTK_VOW_VENDOR_MTK_SUPPORT
#define VOW_SPKMODEL_SIZE                 (0xB000)  // current model size:0xAD64
#else
#define VOW_SPKMODEL_SIZE                 (0x11000)
#endif
//UBM_V1: 0xA000,UBM_V2: 0xDC00, UBM_V3: 2*0x10000
//#define VOW_VOICEDATA_OFFSET              (VOW_SPKMODEL_SIZE * MAX_VOW_SPEAKER_MODEL)

#define VOW_VOICEDATA_SIZE                (0x12500) /* 74880, need over 2.3sec */

#define VOW_LENGTH_SPEAKER                (VOW_SPKMODEL_SIZE >> 1) //cannot over 0x10000/2

#define SEAMLESS_RECORD_LENGTH            (37440) // 74880/2

#ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
#define SEAMLESS_RECORD_BUF_SIZE          (SEAMLESS_RECORD_LENGTH)
#endif // #ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT

#define VOW_DROP_FRAME_FOR_DC             (0x1E)  // wait for dc stable in 30(0x1E) * 10msec = 300msec

#define VOW_SAMPLE_NUM_FRAME              (160)

#define VOW_RESULT_NOTHING                (0x0)
#define VOW_RESULT_PASS                   (0x1)
#define VOW_RESULT_FAIL                   (0x2)
#define VOW_RESULT_CHANGE_MODEL           (0x3)

// After receiving SEAMLESS_RECORD flag, let accumulate 20msec
// then start send voice after receiving SEAMLESS_RECORD flag
#define SEAMLESS_DELAY                    (0)//(2)

#define PCMDUMP_BUF_CNT                   (16)
#define WORD_H                            (8)
#define WORD_L                            (0)

/* For BARGE IN*/
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
#define ECHO_REF_BUF_LEN                (0xA0 * 4)   // 0x280    => 640 / 16kSampleRate = 40msec, 4*160=640
#define ECHO_REF_FRM_LEN                (0xA0)       // 0xA0     => 160 samples
#define ECHO_REF_FRM_BYTE_LEN           (0xA0 * 2)   // 0xA0 * sizeof(int16_t) => 320 bytes
#define EC_SUCCESS                      (1)
#define EC_MIC_NUMBER                   (1)
#define EC_REF_NUMBER                   (1)
#define EC_MIC_L_IDX                    (0)
#define EC_MIC_R_IDX                    (1)
#define IRQ4_SOURCE                     (0x10)
#define IRQ6_SOURCE                     (0x40)
#define IRQ7_SOURCE                     (0x80)
#define BIT_BARGEIN_PERIOD              (0x1)
#define BIT_VOW_PERIOD                  (0x2)
#define BIT_VOICE_R_IDX_UPDATE          (0x4)

#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Enumrations & Data Structures
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
typedef struct {
    void (*func)(int);
    int    addr;
    int    size;
    int    id;
    short  *data;
    bool   flag;   // load model = 1, unload load = 0
    bool   enable; // start recog = 1, stop recog = 0
    unsigned int confidence_lv;
} vow_model_t;

typedef struct {
    unsigned int     dump_buf_offset;
    unsigned int     dump_size;
} vow_pcmdump_info_t;

typedef enum vow_event_info_t {
    VOW_MODEL_INIT = 0,
    VOW_MODEL_SPEAKER,
    VOW_MODEL_NOISE,
    VOW_MODEL_FIR,
    VOW_MODEL_CLEAR,
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
    VOW_FLAG_SEAMLESS,
    VOW_FLAG_DUAL_MIC_LCH,
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
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
    IPIMSG_VOW_SET_BARGEIN_ON = 10,
    IPIMSG_VOW_SET_BARGEIN_OFF = 11,
#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
    IPIMSG_VOW_PCM_DUMP_ON = 12,
    IPIMSG_VOW_PCM_DUMP_OFF = 13,
    VOW_RUN = 15,
    IPIMSG_VOW_COMBINED_INFO = 17,
    IPIMSG_VOW_MODEL_START = 18,
    IPIMSG_VOW_MODEL_STOP = 19,
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

typedef enum vow_channel {
    VOW_MONO = 0,
    VOW_STEREO,
} vow_channel;

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
#ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
    bool                     seamless_record;
    int                      seamless_buf_w_ptr;
#endif  // #ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
    int                      pcmdump_cnt;
    vow_model_t              vow_model_event[MAX_VOW_SPEAKER_MODEL];
    bool                     first_irq_flag;
    char                     mtkif_type;
    char                     channel;
#ifdef BLISRC_USE
    short                    blisrc_w_idx;
    short                    blisrc_r_idx;
#endif  // #ifdef BLISRC_USE
    unsigned int             frm_cnt;
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
    unsigned int             echo_w_idx;
    unsigned int             echo_r_idx;
    unsigned int             echoRead;
    unsigned int             echoReadNowInIRQ;
    unsigned int             echoRefRamEnd;
    unsigned int             echoRefRamBase;
    unsigned int             isAduioFirstIrq;
    unsigned char            echoFirstRamReadDone;
    unsigned char            bargeInStatus;
    unsigned char            vowTimeFlag;
    unsigned int             echo_data_len;
    int                      echo_delay_samples;
    unsigned int             bargein_dump_frm;
    bool                     echo_is_sram;
    bool                     bypass_barge_in;
    unsigned int             bargein_dump_dram_addr;
#ifdef VOW_ECHO_SW_SRC
    unsigned int             echo_nonsrc_w_idx;
    unsigned int             echo_nonsrc_r_idx;
    unsigned int             echo_nonsrc_data_len;
#endif // #ifdef VOW_ECHO_SW_SRC
#endif  // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
    bool                     is_headset;
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
    bool                     PcmDumpFlag;
    unsigned int             recog_dump_dram_addr;
    int                      mic_num;
    bool                     allowRecog_flag;   //now is no used, just reserved
} vowserv_t;

typedef enum dump_data_t {
    DUMP_VOW_VOICE = 0,
    DUMP_VOW_RECOG,
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
    DUMP_VOW_VOICE_R,
    DUMP_VOW_RECOG_R,
#endif
    DUMP_ECHO_REF,
    NUM_DUMP_DATA,
} dump_data_t;

typedef struct {
    uint32_t mic_dump_size;
    uint32_t mic_offset;
    uint32_t recog_dump_size;
    uint32_t recog_dump_offset;
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
    uint32_t mic_dump_size_R;
    uint32_t mic_offset_R;
    uint32_t recog_dump_size_R;
    uint32_t recog_dump_offset_R;
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
    uint32_t echo_dump_size;
    uint32_t echo_offset;
} pcm_dump_info_t;

#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT

typedef enum bargein_status_t {
    BARGEIN_OFF = 0,
    BARGEIN_START = 1,
    BARGEIN_WORKING1 = 2,
    BARGEIN_WORKING2 = 3,
} bargein_status_t;

typedef enum echofirstread_status_t {
    ECHO_FIRST_READ_OFF = 0,
    ECHO_FIRST_READ_DONE = 1,
    ECHO_FIRST_READ_TASK = 2,
    ECHO_FIRST_READ_RESYNC = 3,
} echofirstread_status_t;

#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT

#ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT

typedef enum seamless_send_status_t {
    SEAMLESS_SEND_IDLE = 0,
    SEAMLESS_SEND_SMALL = 1,
    SEAMLESS_SEND_LARGE = 2,
    SEAMLESS_SEND_CONTINUE_LARGE = 3,
} seamless_send_status_t;

#endif // #ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
typedef struct {
    vow_status_t             vow_status;
    bool                     debug_P1_flag;
    bool                     debug_P2_flag;
    bool                     swip_log_anable;
    int                      apreg_addr;
    vow_model_t              vow_model_event[MAX_VOW_SPEAKER_MODEL];
} do_save_vow_t;

typedef enum ipi_type_flag_t {
    RECOG_OK_IDX = 0,
    DEBUG_DUMP_IDX = 1,
    RECOG_DUMP_IDX = 2,
    BARGEIN_DUMP_INFO_IDX = 3,
    BARGEIN_DUMP_IDX = 4
} ipi_type_flag_t;

typedef enum vendor_id_t{
    VENDOR_ID_MTK = 77,     //'M'
    VENDOR_ID_AMAZON = 65,  //'A'
    VENDOR_ID_OTHERS = 71,  //'G'
    VENDOR_ID_NONE = 0
} vendor_id_t;

#define RECOG_OK_IDX_MASK           (0x01 << RECOG_OK_IDX)
#define DEBUG_DUMP_IDX_MASK         (0x01 << DEBUG_DUMP_IDX)
#define RECOG_DUMP_IDX_MASK         (0x01 << RECOG_DUMP_IDX)
#define BARGEIN_DUMP_INFO_IDX_MASK  (0x01 << BARGEIN_DUMP_INFO_IDX)
#define BARGEIN_DUMP_IDX_MASK       (0x01 << BARGEIN_DUMP_IDX)


typedef struct vow_ipi_combined_info {
    unsigned int ipi_type_flag;
    /* IPIMSG_VOW_RECOGNIZE_OK */
    unsigned int recog_ret_info;
    unsigned int recog_ok_uuid;
    unsigned int confidence_lv;
    unsigned long long recog_ok_os_timer;
    /* IPIMSG_VOW_DATAREADY */
    unsigned int voice_buf_offset;
    unsigned int voice_length;
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
    /* IPIMSG_VOW_BARGEIN_DUMP_INFO */
    unsigned int dump_frm_cnt;
    unsigned int voice_sample_delay;
    /* IPIMSG_VOW_BARGEIN_PCMDUMP_OK */
    unsigned int bargein_mic_dump_size;
    unsigned int bargein_mic_offset;
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
    unsigned int bargein_mic_dump_size_R;
    unsigned int bargein_mic_offset_R;
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
    unsigned int bargein_echo_dump_size;
    unsigned int bargein_echo_offset;
#endif  // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
    unsigned int recog_dump_size;
    unsigned int recog_dump_offset;
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
    unsigned int recog_dump_size_R;
    unsigned int recog_dump_offset_R;
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
} vow_ipi_combined_info_t;

#endif  // end of AUDIO_TASK_VOW_PARAMS_H
