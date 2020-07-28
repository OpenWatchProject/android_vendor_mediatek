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
#include "audio_task_vow.h"
#include <stdarg.h>
#include <interrupt.h>
#include <dma.h>
#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif  // #ifdef CFG_VCORE_DVFS_SUPPORT
#include <feature_manager.h>
#include <wrapped_errors.h>
#include "audio_messenger_ipi.h"
#include "vow_hw.h"
#include "swVAD.h"
#include "VowEngine_DSP_testing.h"
#include "arsi_api.h"
#ifdef VOW_WAKELOCK
#include "wakelock.h"
#endif  // #ifdef VOW_WAKELOCK
#include "mt_gpt.h"
#if SMART_AAD_MIC_SUPPORT
#ifdef CFG_EINT_SUPPORT
#include "eint.h"
#endif  // #ifdef CFG_EINT_SUPPORT
#endif  // #if SMART_AAD_MIC_SUPPORT
#ifndef NORMAL_SRC_DIRECT_OUT
#include "blisrc_vow.h"
#endif  // #ifndef NORMAL_SRC_DIRECT_OUT
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
#include "blisrc_accurate_vow.h"
#include "BargeInProcessor.h"
#include "audio_hw.h"
#include "audio_hw_reg.h"
#include "cache_internal.h"

#ifdef VOW_ECHO_SW_SRC
#include "blisrc_echo_ref_vow.h"
#endif // #ifdef VOW_ECHO_SW_SRC

#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
#include "audio_task_utility.h"
#ifdef __VOW_AMAZON_SUPPORT__
#include "pryon_lite.h"
#endif
#ifdef __VOW_GOOGLE_SUPPORT__
#include "arm_math.h"
#include "hotword_dsp_api.h"
#endif

/*==============================================================================
 *                     MACRO
 *============================================================================*/

// #define RECOVERY_TEST


/*==============================================================================
 *                     private global members
 *============================================================================*/
vowserv_t vowserv;

short vowserv_buf_voice[VOW_LENGTH_VOICE] __attribute__((aligned(4)));

short vowserv_buf_speaker_A[VOW_LENGTH_SPEAKER] __attribute__((aligned(4)));
#ifdef CFG_MTK_VOW_2E2K_SUPPORT
short vowserv_buf_speaker_B[VOW_LENGTH_SPEAKER] __attribute__((aligned(4)));
#endif
#ifdef BLISRC_USE
short vowserv_buf_blic_out[BLISRC_OUTPUT_TEMP_BUFFER] __attribute__((aligned(4)));
unsigned int blisrc_internal_buf[BLISRC_INTERNAL_BUFFER_WORD_SIZE] __attribute__((aligned(4)));
void *blisrc_handle = NULL;
bool double_bli_output_next = false;
#endif  // #ifdef BLISRC_USE

#ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
short vowserv_buf_seamless_rec[SEAMLESS_RECORD_BUF_SIZE] __attribute__((aligned(4)));
#endif // #ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT


#define RECOG_DUMP_SIZE_DATA         (VOW_SAMPLE_NUM_FRAME * 16)    /* 160 * 16 =  0xA00 samples */
#define RECOG_DUMP_SIZE_BYTE_DATA    (RECOG_DUMP_SIZE_DATA * 2)     /* 160 * 16 *2 =  0x1400 bytes */
#define RECOG_DUMP_SIZE_DATA_R       (VOW_SAMPLE_NUM_FRAME * 16)    /* 160 * 16 =  0xA00 samples */
#define RECOG_DUMP_SIZE_BYTE_DATA_R  (RECOG_DUMP_SIZE_DATA_R * 2)   /* 160 * 16 *2 =  0x1400 bytes */

static int recog_dump_dram_len;
static uint32_t recog_dump_data_w;
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
static uint32_t recog_dump_data_w_R;
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT

#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
//#define DUMP_BARGEIN

#define BARGEIN_DUMP_SIZE_MIC        (VOW_SAMPLE_NUM_FRAME * 16)    /* 160 * 16 =  0xA00 samples */
#define BARGEIN_DUMP_SIZE_BYTE_MIC   (BARGEIN_DUMP_SIZE_MIC * 2)    /* 160 * 16 *2 =  0x1400 bytes */
#define BARGEIN_DUMP_SIZE_ECHO       (ECHO_REF_FRM_LEN * 16)        /* 160 * 16 =  0xA00 samples */
#define BARGEIN_DUMP_SIZE_BYTE_ECHO  (BARGEIN_DUMP_SIZE_ECHO * 2)   /* 160 * 16 *2 =  0x1400 bytes */
#define BARGEIN_DUMP_SIZE_MIC_R      (VOW_SAMPLE_NUM_FRAME * 16)    /* 160 * 16 =  0xA00 samples */
#define BARGEIN_DUMP_SIZE_BYTE_MIC_R (BARGEIN_DUMP_SIZE_MIC_R * 2)  /* 160 * 16 *2 =  0x1400 bytes */

#ifdef VOW_ECHO_SW_SRC
DRAM_REGION_VARIABLE short vowserv_buf_echo_ref_nonsrc[ECHO_REF_NONSRC_BUF_LEN] __attribute__((aligned(4)));
DRAM_REGION_VARIABLE unsigned int
    blisrc_echo_ref_internal_buf[BLISRC_ECHO_INTERNAL_BUFFER_WORD_SIZE] __attribute__((aligned(4)));
void *blisrc_echo_ref_handle = NULL;
#endif // #ifdef VOW_ECHO_SW_SRC
short vowserv_buf_echo_ref_in[ECHO_REF_BUF_LEN] __attribute__((aligned(4)));
short vowserv_buf_echo_ref_out[ECHO_REF_FRM_LEN] __attribute__((aligned(4)));
short vowserv_buf_echo_ref_sil[ECHO_REF_FRM_LEN] __attribute__((aligned(4)));
unsigned int blisrc_accurate_internal_buf[BLISRC_ACCURATE_INTERNAL_BUFFER_WORD_SIZE] __attribute__((aligned(4)));
#ifdef __VOW_AMAZON_SUPPORT__
PryonLiteDecoderConfig PryonConfig;
char pryon_lite_decoder_buf[18000] __attribute__((aligned(4)));
#endif

char *micConfPath = NULL;
int ec_out_byte = 0;
int *out_byte_ptr = &ec_out_byte;
void *blisrc_accurate_handle = NULL;
//static uint32_t dump_total;
//static uint32_t dump_total_count;
static int bargein_dump_dram_len;
static bool bargein_ipi_send_flag;
static uint32_t bargein_dump_echo_w;
static uint32_t bargein_dump_mic_w;
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
static uint32_t bargein_dump_mic_w_R;
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
static unsigned int echo_irq_id;

#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
static uint8_t dump_pcm_flag;
static pcm_dump_info_t vow_pcm_dump_info;

#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
short vowserv_buf_voice_R[VOW_LENGTH_VOICE] __attribute__((aligned(4)));

#ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
short vowserv_buf_seamless_rec_R[SEAMLESS_RECORD_BUF_SIZE] __attribute__((aligned(4)));
#endif // #ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT

#ifdef BLISRC_USE
short vowserv_buf_blic_out_R[BLISRC_OUTPUT_TEMP_BUFFER] __attribute__((aligned(4)));
unsigned int blisrc_internal_buf_R[BLISRC_INTERNAL_BUFFER_WORD_SIZE] __attribute__((aligned(4)));
void *blisrc_handle_R = NULL;
#endif  // #ifdef BLISRC_USE

#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
unsigned int blisrc_accurate_internal_buf_R[BLISRC_ACCURATE_INTERNAL_BUFFER_WORD_SIZE] __attribute__((aligned(4)));
short vowserv_buf_echo_ref_out_R[ECHO_REF_FRM_LEN] __attribute__((aligned(4)));
void *blisrc_accurate_handle_R = NULL;
//char *micConfPath_R = NULL;
int ec_out_byte_R = 0;
int *out_byte_ptr_R = &ec_out_byte_R;

#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
bool scp_vow_lch = true;

#if SW_FIX_METHOD
TaskHandle_t xCheckDataIrqHandle;
#endif  // #if SW_FIX_METHOD
#ifdef VOW_WAKELOCK
wakelock_t vow_wakelock;
#endif  // #ifdef VOW_WAKELOCK
#ifdef CHECK_RECOG_TIME
TimerHandle_t xVowTimer;
unsigned int vowTaskStruct;
unsigned int vow_cur_ipi_msg;
#endif  // #ifdef CHECK_RECOG_TIME
#ifdef RECOVERY_TEST
unsigned int bug_test_cnt;
#endif  // #ifdef RECOVERY_TEST

typedef void (*VOW_MODEL_HANDLER)(void);

#ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
int delay_count = 0;
unsigned int seamless_test = SEAMLESS_SEND_IDLE;
#endif

#ifdef __VOW_AMAZON_SUPPORT__
// decoder handle
static PryonLiteDecoderHandle sDecoder = NULL;
// initialize decoder
static PryonLiteSessionInfo sessionInfo;
static bool pryonLite_recog_ok = false;
static int pryonLite_vadState = -1;
static int pryonLite_confidence_lv = 0;
#endif

#if TIMES_DEBUG
volatile unsigned int *ITM_CONTROL_1 = (unsigned int *)0xE0000E80;
volatile unsigned int *DWT_CONTROL_1 = (unsigned int *)0xE0001000;
volatile unsigned int *DWT_CYCCNT_1 = (unsigned int *)0xE0001004;
volatile unsigned int *DEMCR_1 = (unsigned int *)0xE000EDFC;
unsigned int start_recog_time = 0;
unsigned int end_recog_time = 0;
unsigned int max_recog_time = 0;
unsigned int start_task_time = 0;
unsigned int end_task_time = 0;
unsigned int max_task_time = 0;
unsigned int start_blisrc_time = 0;
unsigned int end_blisrc_time = 0;
unsigned int total_recog_time = 0;
unsigned int avg_cnt = 0;
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
unsigned int start_ec_time = 0;
unsigned int end_ec_time = 0;
#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT

#define CPU_RESET_CYCLECOUNTER_1() \
do { \
*DEMCR_1 = *DEMCR_1 | 0x01000000; \
*DWT_CYCCNT_1 = 0; \
*DWT_CONTROL_1 = *DWT_CONTROL_1 | 1 ; \
} while (0)

// Test Method
// CPU_RESET_CYCLECOUNTER();
// start_time = *DWT_CYCCNT;
#endif  // #if TIMES_DEBUG
unsigned long long recognize_ok_cycle = 0;
unsigned long long pre_recognize_ok_cycle = 0;

static uint8_t *g_msg_array;
static uint32_t ipi_msg_size;
#define IPI_MSG_DMA_INFO_SIZE (sizeof(struct ipi_msg_dma_info_t))
#define LIGHT_IPI_MSG_PAYLOAD_SIZE (sizeof(vow_ipi_combined_info_t))
#define GET_IPI_MSG(queue_idx) \
    ((ipi_msg_t *)(g_msg_array + queue_idx * ipi_msg_size))

inline int max(int a, int b)
{
    if (a < b)
        return b;
    return a;
}

/*==============================================================================
 *                     derived functions - declaration
 *============================================================================*/

static void     task_vow_constructor(struct AudioTask *this);
static void     task_vow_destructor(struct AudioTask *this);

static void     task_vow_create_task_loop(struct AudioTask *this);
static void     task_vow_destroy_task_loop(struct AudioTask *this);

static status_t task_vow_recv_message(
    struct AudioTask *this,
    struct ipi_msg_t *ipi_msg);

static void     task_vow_irq_handler(
    struct AudioTask *this,
    uint32_t irq_type);

static uint32_t task_vow_get_write_back_size(struct AudioTask *this);
static void     task_vow_copy_write_back_buffer(struct AudioTask *this,
        data_buf_t *buf);
static void     task_vow_do_write_back(struct AudioTask *this,
                                       data_buf_t *buf);

static void     task_vow_hal_reboot_cbk(
    struct AudioTask *this,
    const uint8_t hal_type);



/*==============================================================================
 *                     private functions - declaration
 *============================================================================*/
static uint8_t task_vow_preparsing_message(struct AudioTask *this, ipi_msg_t *ipi_msg);
static void task_vow_task_loop(void *pvParameters);
#if SW_FIX_METHOD
static void task_vow_check_datairq(void *pvParameters);
#endif  // #if SW_FIX_METHOD
static void task_vow_init(struct AudioTask *this);
static uint8_t task_vow_working(struct AudioTask *this, struct ipi_msg_t *ipi_msg);
static void task_vow_deinit(struct AudioTask *this);
static void vow_enable(void);
static void vow_disable(void);
static void vow_setapreg_addr(unsigned int addr);
// static unsigned int vow_gettcmreg_addr(void);
static void vow_set_flag(vow_flag_t flag, short enable);
static int vow_findEmptyModel(void);
static int vow_fetchMatchModel(int id);
static void vow_clearModel(int slot, vow_event_info_t type);
static void vow_setModel(int slot, vow_event_info_t type, int id, int addr, int size);
static void vow_loadModel(int slot);
static void vow_ModelStart(int id, unsigned int confidence_level);
static void vow_ModelStop(int id, unsigned int confidence_level);
//static void vow_sync_data_ready(void);
#if PMIC_6337_SUPPORT
static void vow_ready_receive_fifo(void);
#endif  // #if PMIC_6337_SUPPORT
static void vow_getModel_speaker(int slot);
int vow_model_setconfig(vow_model_t *model_ptr);
int vow_model_init(vow_model_t *model_ptr);
int vow_model_deinit(vow_model_t *model_ptr);
int vow_keyword_recognize(void *sample_input, int num_sample, int *ret_info, int *recog_ok_id, int *confidence_lv);
static void vow_resume_phase1_setting(void);
#if SMART_AAD_MIC_SUPPORT
static void vow_setSmartDevice(bool enable, unsigned int eint_num);
static void vow_SmartDeviceEnable(void);
static unsigned int vow_getEintNumber(unsigned int eint_num);
#endif  // #if SMART_AAD_MIC_SUPPORT
#ifdef CHECK_RECOG_TIME
static void vVOWTimerCallback(TimerHandle_t xTimer);
#endif  // #ifdef CHECK_RECOG_TIME

#ifdef BLISRC_USE
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
static void vow_barge_in_start(void);
static void vow_barge_in_stop(void);
static bool echo_ref_is_sram(void);
static void echo_ref_address_map(bool is_sram);
static void task_echo_ref_getting(void);
//static void initdebugvar(void);
static int vow_bargein_buffer_data(int w_idx, uint32_t delay_sample, uint32_t buf_len);
#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
#endif // #ifdef BLISRC_USE
static void vow_pcm_dump_process(dump_data_t flag, uint32_t dump_buf_src, uint32_t dump_buf_size,
                                 vow_ipi_combined_info_t *ipi_ptr);

#ifdef NORMAL_SRC_DIRECT_OUT
typedef struct {
    uint32_t input_sampling_rate;
    uint32_t output_sampling_rate;
    uint32_t input_channel_number;
    uint32_t output_channel_number;
} blisrc_vow_init_param_t;

uint32_t blisrc_vow_get_buffer_size(void);

int32_t blisrc_vow_open(void **p_handle,
                        uint8_t *p_internal_buf,
                        blisrc_vow_init_param_t *p_param);

int32_t blisrc_vow_process(void *handle,
                           int16_t *p_in_buf,
                           uint32_t *available_in_size,
                           int16_t *p_ou_buf,
                           uint32_t *available_ou_size);
#endif // #ifdef NORMAL_SRC_DIRECT_OUT

BaseType_t vow_wake_lock(void *lock);
BaseType_t vow_wake_unlock(void *lock);

/*==============================================================================
 *                     class new/construct/destruct functions
 *============================================================================*/

AudioTask *task_vow_new()
{
    AUD_LOG_V("task_vow_create++\n");
    /* alloc object here */
    AudioTask *task = (AudioTask *)kal_pvPortMalloc(sizeof(AudioTask));
    if (task == NULL) {
        AUD_LOG_E("%s(), [err]kal_pvPortMalloc fail!!\n", __func__);
        return NULL;
    }

    memset(task, 0, sizeof(AudioTask));

    /* only assign methods, but not class members here */
    task->constructor               = task_vow_constructor;
    task->destructor                = task_vow_destructor;

    task->create_task_loop          = task_vow_create_task_loop;
    task->destroy_task_loop         = task_vow_destroy_task_loop;

    task->recv_message              = task_vow_recv_message;

    task->irq_hanlder               = task_vow_irq_handler;

    task->get_write_back_size       = task_vow_get_write_back_size;
    task->copy_write_back_buffer    = task_vow_copy_write_back_buffer;
    task->do_write_back             = task_vow_do_write_back;

    task->hal_reboot_cbk            = task_vow_hal_reboot_cbk;

    AUD_LOG_V("task_vow_create--\n");

    return task;
}

void task_vow_delete(AudioTask *task)
{
    AUD_LOG_V("%s(+)\n", __func__);

    if (task == NULL) {
        AUD_LOG_E("%s(), [err]task is NULL!!\n", __func__);
        return;
    }

    kal_vPortFree(task);

    AUD_LOG_V("%s(-)\n", __func__);
}

static uint32_t task_vow_get_write_back_size(struct AudioTask *this)
{
    return sizeof(do_save_vow_t);
}

static void task_vow_copy_write_back_buffer(struct AudioTask *this,
        data_buf_t *buf)
{
    do_save_vow_t wb_data;
    int I;

    AUD_ASSERT(buf->memory_size >= sizeof(do_save_vow_t));

    // save sturcture
    memset((void *)&wb_data, 0, sizeof(wb_data));
    wb_data.vow_status = vowserv.vow_status;
    wb_data.debug_P1_flag = vowserv.debug_P1_flag;
    wb_data.debug_P2_flag = vowserv.debug_P2_flag;
    wb_data.swip_log_anable = vowserv.swip_log_anable;
    wb_data.apreg_addr = vowserv.apreg_addr;
    for (I = 0; I < MAX_VOW_SPEAKER_MODEL; I++) {
        wb_data.vow_model_event[I].addr = vowserv.vow_model_event[I].addr;
        wb_data.vow_model_event[I].size = vowserv.vow_model_event[I].size;
        wb_data.vow_model_event[I].id = vowserv.vow_model_event[I].id;
        wb_data.vow_model_event[I].flag = vowserv.vow_model_event[I].flag;
    }
    // copy into *buf
    memcpy(buf->p_buffer, &wb_data, sizeof(do_save_vow_t));
    buf->data_size = sizeof(do_save_vow_t);
}

static void task_vow_do_write_back(struct AudioTask *this,
                                   data_buf_t *buf)
{
    do_save_vow_t wb_data;
    int I;

    AUD_ASSERT(buf->memory_size >= sizeof(do_save_vow_t));
    AUD_ASSERT(buf->data_size == sizeof(do_save_vow_t));
    memcpy(&wb_data, buf->p_buffer, sizeof(do_save_vow_t));
    AUD_LOG_V("%s(), apreg=0x%x, model=0x%x\n", __func__,
              wb_data.apreg_addr,
              wb_data.vow_model_event.addr);
    vowserv.debug_P1_flag = wb_data.debug_P1_flag;
    vowserv.debug_P2_flag = wb_data.debug_P2_flag;
    vowserv.swip_log_anable = wb_data.swip_log_anable;
    if (wb_data.apreg_addr != 0) {
        vowserv.apreg_addr = wb_data.apreg_addr;
    }

    for (I = 0; I < MAX_VOW_SPEAKER_MODEL; I++) {
        if (wb_data.vow_model_event[I].addr) {
            vowserv.vow_model_event[I].addr = wb_data.vow_model_event[I].addr;
            vowserv.vow_model_event[I].size = wb_data.vow_model_event[I].size;
            vowserv.vow_model_event[I].id = wb_data.vow_model_event[I].id;
            vowserv.vow_model_event[I].flag = wb_data.vow_model_event[I].flag;
            AUD_LOG_V("DO WB, model_addr=0x%x, size=0x%x, id=0x%d, flag=%d\n",
                       vowserv.vow_model_event[I].addr,
                       vowserv.vow_model_event[I].size,
                       vowserv.vow_model_event[I].id,
                       vowserv.vow_model_event[I].flag);
            vow_loadModel(I);
        }
    }
}


static void task_vow_hal_reboot_cbk(struct AudioTask *this, const uint8_t hal_type)
{
    AUD_LOG_D("%s(), scene %d\n", __func__, this->scene);
}


static void task_vow_constructor(struct AudioTask *this)
{
    AUD_ASSERT(this != NULL);
    AUD_LOG_V("%s()\n", __func__);

    /* assign initial value for class members & alloc private memory here */
    this->scene = TASK_SCENE_VOW;
    this->state = AUDIO_TASK_IDLE;

    /* queue */
    this->queue_idx = 0;
    this->num_queue_element = 0;

    ipi_msg_size = max(LIGHT_IPI_MSG_PAYLOAD_SIZE, IPI_MSG_DMA_INFO_SIZE) +
                   IPI_MSG_HEADER_SIZE;
    g_msg_array = (uint8_t *)kal_pvPortMalloc(MAX_MSG_QUEUE_SIZE * ipi_msg_size);
    AUD_ASSERT(g_msg_array != NULL);
    memset(g_msg_array, 0, MAX_MSG_QUEUE_SIZE * ipi_msg_size);

    this->msg_idx_queue = xQueueCreate(MAX_MSG_QUEUE_SIZE, sizeof(uint8_t));
    AUD_ASSERT(this->msg_idx_queue != NULL);

    task_vow_init(this);
}

static void task_vow_destructor(struct AudioTask *this)
{
    if (this == NULL) {
        AUD_LOG_E("%s(), [err]this is NULL!!\n", __func__);
        return;
    }

    AUD_LOG_V("%s(+), task_scene = %d\n", __func__, this->scene);
    task_vow_deinit(this);
    /* dealloc private memory & dealloc object here */
    if (g_msg_array != NULL) {
        kal_vPortFree(g_msg_array);
    }

    AUD_LOG_V("%s(-), task_scene = %d\n", __func__, this->scene);
    if (this->msg_idx_queue != NULL) {
        vQueueDelete(this->msg_idx_queue);
    }
#ifdef VOW_WAKELOCK
    wake_lock_deinit(&vow_wakelock);
#endif  // #ifdef VOW_WAKELOCK
}

static void task_vow_create_task_loop(struct AudioTask *this)
{
    /* Note: you can also bypass this function,
             and do kal_xTaskCreate until you really need it.
             Ex: create task after you do get the enable message. */

    BaseType_t xReturn = pdFAIL;

    xReturn = kal_xTaskCreate(
                  task_vow_task_loop,
                  LOCAL_TASK_NAME,
                  LOCAL_TASK_STACK_SIZE,
                  (void *)this,
                  LOCAL_TASK_PRIORITY,
                  &this->freertos_task);
    AUD_ASSERT(xReturn == pdPASS);
    AUD_ASSERT(this->freertos_task);

#if SW_FIX_METHOD
    xReturn = pdFAIL;
    xReturn = kal_xTaskCreate(
                  task_vow_check_datairq,
                  "vow_check_datairq_task",
                  100,
                  (void *)0,
                  LOCAL_TASK_PRIORITY,
                  &xCheckDataIrqHandle);
    AUD_ASSERT(xReturn == pdPASS);
    AUD_ASSERT(xCheckDataIrqHandle);
#endif  // #if SW_FIX_METHOD

#ifdef CHECK_RECOG_TIME
    vowTaskStruct = (unsigned int)this;
    AUD_LOG_D("this = %p, vowTaskStruct = %d\n", this, vowTaskStruct);
    xVowTimer = xTimerCreate(  /* Just a text name, not used by the RTOS kernel. */
                    "VowCheckTimer",
                    /* The timer period in ticks */
                    ((portTickType) 15 / portTICK_RATE_MS),
                    /* The timers will auto-reload themselveswhen they expire. */
                    pdFALSE,
                    /* The ID is used to store a count of the number of times the timer
                    has expired, which is initialised to 0. */
                    (void *) 0,
                    /* Each timer calls the same callback when it expires. */
                    vVOWTimerCallback);
    if (xVowTimer == NULL) {
        /* The timer was not created. malloc fail? */
        configASSERT(0);
    } else {
        /* stop the timer because it's not use yet */
        xTimerStop(xVowTimer, 0);
    }
#endif  // #ifdef CHECK_RECOG_TIME
}

static void task_vow_destroy_task_loop(struct AudioTask *this)
{
#if (INCLUDE_vTaskDelete == 1)
    if (this->freertos_task) {
        vTaskDelete(this->freertos_task);
    }
#if SW_FIX_METHOD
    if (xCheckDataIrqHandle) {
        vTaskDelete(xCheckDataIrqHandle);
    }
#endif  // #if SW_FIX_METHOD
#endif  // #if (INCLUDE_vTaskDelete == 1)
}



static status_t task_vow_recv_message(
    struct AudioTask *this,
    struct ipi_msg_t *ipi_msg)
{
    static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    uint8_t ret = false;
    uint8_t i = 0;
    unsigned int queue_idx;
    ipi_msg_t *p_ipi_msg;
    mask_irq(MAD_FIFO_IRQn);  /* prevent nested interrupt: MD_IRQn has higher prior */
    this->num_queue_element++;
    queue_idx = get_queue_idx(this, MAX_MSG_QUEUE_SIZE);
    p_ipi_msg = GET_IPI_MSG(queue_idx);

    // check message
    if (p_ipi_msg->magic != 0) {
        AUD_LOG_D("queue_idx = %d\n", queue_idx);
        for (i = 0; i < MAX_MSG_QUEUE_SIZE; i++) {
            AUD_LOG_E("[%d] id = 0x%x\n", i, p_ipi_msg->msg_id);
        }
        AUD_ASSERT(p_ipi_msg->magic == 0);  /* item is clean */
    }
    // copy to array
    memcpy(p_ipi_msg, ipi_msg, ipi_msg_size);
    unmask_irq(MAD_FIFO_IRQn);

    //AUD_LOG_V("Recv id = 0x%x\n", ipi_msg->msg_id);
    AUD_LOG_V("ele:%d, idx: %d, msg:%d\n", this->num_queue_element, queue_idx, p_ipi_msg->msg_id);
    ret = task_vow_preparsing_message(this, p_ipi_msg);
    if (ret) {
        if (xQueueSendToBackFromISR(this->msg_idx_queue, &queue_idx,
                                    &xHigherPriorityTaskWoken) != pdTRUE) {
            AUD_LOG_E("[err]vow queue UNKNOW ERROR\n");
            return UNKNOWN_ERROR;
        }
    } else {
        /* clean msg */
        memset(p_ipi_msg, 0, ipi_msg_size);
        this->num_queue_element--;
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    return NO_ERROR;
}

static void task_vow_irq_handler(
    struct AudioTask *this,
    uint32_t irq_type)
{
#if PMIC_6337_SUPPORT
    if (irq_type == MAD_DATA_IRQn) {
        mask_irq(MAD_DATA_IRQn);

        vow_wake_lock(&vow_wakelock);
        vowserv.scpwakelock = SCP_LOCK;

        vow_datairq_clear();
        vow_ready_receive_fifo();
    } else if (irq_type == MAD_FIFO_IRQn) {
#else  // #if PMIC_6337_SUPPORT
    if (irq_type == MAD_FIFO_IRQn) {
#endif  // #if PMIC_6337_SUPPORT
        ipi_msg_t *ipi_msg = NULL;
        uint8_t queue_idx = 0;
        static BaseType_t xHigherPriorityTaskWoken;
        xHigherPriorityTaskWoken = pdFALSE;

        /*
        #if !PMIC_6337_SUPPORT
                if (vowserv.scpwakelock == SCP_UNLOCK) {
                    vow_wake_lock(&vow_wakelock);
                    vowserv.scpwakelock = SCP_LOCK;
                }
        #endif  // #if !PMIC_6337_SUPPORT
        */
        /*----------------------------------------------------------------------
        //  Check Status
        ----------------------------------------------------------------------*/
        if (vowserv.vow_status == VOW_STATUS_IDLE) {
            // AUD_LOG_D("mic_idle\n");
#if PMIC_6359_SUPPORT
            vow_read_fifo_data(NULL, NULL, VOW_SAMPLE_NUM_IRQ, &vowserv);
#else
            vow_read_fifo_data(NULL, VOW_SAMPLE_NUM_IRQ);
#endif
            vow_reset_fifo_irq();
            return;
        }
        if (vowserv.drop_count < vowserv.drop_frame) {
            // AUD_LOG_D("drop\n");
#if PMIC_6359_SUPPORT
            vow_read_fifo_data(NULL, NULL, VOW_SAMPLE_NUM_IRQ, &vowserv);
#else
            vow_read_fifo_data(NULL, VOW_SAMPLE_NUM_IRQ);
#endif
            vowserv.drop_count++;
            vow_reset_fifo_irq();
            return;
        }

        if ((vowserv.write_idx == vowserv.read_idx) && (vowserv.data_length == VOW_LENGTH_VOICE)) {
            AUD_LOG_E("[err]Buffer full: w:%d\n", vowserv.write_idx);
#if PMIC_6359_SUPPORT
            vow_read_fifo_data(NULL, NULL, VOW_SAMPLE_NUM_IRQ, &vowserv);
#else
            vow_read_fifo_data(NULL, VOW_SAMPLE_NUM_IRQ);
#endif
            vowserv.read_idx = VOW_LENGTH_VOICE + 1;
            vowserv.write_idx = 0;
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
            vowserv.vowTimeFlag &= ~(BIT_VOICE_R_IDX_UPDATE);
#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT

#ifdef BLISRC_USE
            vowserv.blisrc_r_idx = 0;
            vowserv.blisrc_w_idx = 0;
#endif  // #ifdef BLISRC_USE
            vowserv.data_length = 0;

            vow_reset_fifo_irq();
            return;
        }
        AUD_LOG_V("iV, r: %d, w: %d\n", vowserv.read_idx, vowserv.write_idx);
        /*----------------------------------------------------------------------
        //  Read From VOW FIFO and do memcpy
        ----------------------------------------------------------------------*/
#if PMIC_6359_SUPPORT
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
        vow_read_fifo_data(&vowserv_buf_voice[vowserv.write_idx], &vowserv_buf_voice_R[vowserv.write_idx], VOW_SAMPLE_NUM_IRQ,
                           &vowserv);
#else
        vow_read_fifo_data(&vowserv_buf_voice[vowserv.write_idx], NULL, VOW_SAMPLE_NUM_IRQ, &vowserv);
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
#else
        vow_read_fifo_data(&vowserv_buf_voice[vowserv.write_idx], VOW_SAMPLE_NUM_IRQ);
#endif
        vowserv.write_idx += VOW_SAMPLE_NUM_IRQ;
        // AUD_LOG_D("w_idx=0x%x r:0x%x\n", vowserv.write_idx, vowserv.read_idx);
        if (vowserv.write_idx >= VOW_LENGTH_VOICE) {
            vowserv.write_idx = 0;
        }
        vowserv.data_length += VOW_SAMPLE_NUM_IRQ;

        /*----------------------------------------------------------------------
        //  Calculate data amount and trigger VOW task if necessary
        ----------------------------------------------------------------------*/
        AUD_LOG_V("10ms\n");

        this->num_queue_element++;
        queue_idx = get_queue_idx(this, MAX_MSG_QUEUE_SIZE);
        ipi_msg = GET_IPI_MSG(queue_idx);
        ipi_msg->magic      = IPI_MSG_MAGIC_NUMBER;
        ipi_msg->task_scene = TASK_SCENE_VOW;
        ipi_msg->source_layer = AUDIO_IPI_LAYER_FROM_DSP;
        ipi_msg->target_layer = AUDIO_IPI_LAYER_TO_DSP;
        ipi_msg->data_type  = AUDIO_IPI_MSG_ONLY;
        ipi_msg->ack_type   = AUDIO_IPI_MSG_BYPASS_ACK;
        ipi_msg->msg_id     = VOW_RUN;
        ipi_msg->param1     = 0;
        ipi_msg->param2     = 0;
        if (xQueueSendToBackFromISR(this->msg_idx_queue, &queue_idx,
                                    &xHigherPriorityTaskWoken) != pdTRUE) {
            AUD_LOG_E("[err]Queue Full\n");
        }

        vow_reset_fifo_irq();
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        if (vowserv.bargeInStatus == BARGEIN_START) {
            vow_set_bargein_int(true, echo_irq_id);
            vow_set_mem_awb(true);
            vowserv.bargeInStatus = BARGEIN_WORKING1;
        }
#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    if (irq_type == AUDIO_IRQn) {
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        int IRQSource;
        unsigned int delay_smpl;

        IRQSource = DRV_Reg32(AFE_IRQ_MCU_STATUS);
        DRV_WriteReg32(AFE_IRQ_MCU_CLR, (IRQSource & (1 << echo_irq_id)));
        AUD_LOG_V("[Bargein] %s(+) %d %d\n", __func__, irq_type, IRQSource);
        AUD_LOG_V("[Bargein] iA, r: %d, w: %d\n", vowserv.read_idx, vowserv.write_idx);

        if (IRQSource & (1 << echo_irq_id)) {
            if ((AUDIO_TASK_INIT == this->state || AUDIO_TASK_WORKING == this->state) &&
                    ((vowserv.bargeInStatus == BARGEIN_WORKING1) || (vowserv.bargeInStatus == BARGEIN_WORKING2))) {
                if (vowserv.isAduioFirstIrq == 0) { // decide echo ref first Read ptr, Echo ref is stereo
#ifdef VOW_ECHO_SW_SRC
                    delay_smpl = ECHO_REF_NONSRC_FRM_LEN;
#else
                    delay_smpl = ECHO_REF_FRM_BYTE_LEN;
#endif // #ifdef VOW_ECHO_SW_SRC
                    vowserv.echoRead = DRV_Reg32(AFE_AWB_CUR);
                    // Rptr = (AFE_AWB_CUR - one stereo frame bytes), todo: stereo case
                    if (vowserv.echoRead >= (DRV_Reg32(AFE_AWB_BASE) + (delay_smpl << 1))) {
                        vowserv.echoRead -= (delay_smpl << 1);
                    } else {
                        vowserv.echoRead = vowserv.echoRead - (delay_smpl << 1)
                                           + (DRV_Reg32(AFE_AWB_END) - DRV_Reg32(AFE_AWB_BASE) + 1);
                    }
                    //vowserv.isAduioFirstIrq++;
                    AUD_LOG_D("[Bargein] first get r_idx: 0x%x, cur: 0x%x\n", vowserv.echoRead, DRV_Reg32(AFE_AWB_CUR));
                    AUD_LOG_D("[Bargein] b: 0x%x, e: 0x%x\n", DRV_Reg32(AFE_AWB_BASE), DRV_Reg32(AFE_AWB_END));
                }
                vowserv.echoReadNowInIRQ = DRV_Reg32(AFE_AWB_CUR);
                vowserv.echoRefRamEnd = DRV_Reg32(AFE_AWB_END);
                vowserv.echoRefRamBase = DRV_Reg32(AFE_AWB_BASE);
                echo_ref_address_map(vowserv.echo_is_sram);
                task_echo_ref_getting();
            }
        }
#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
    }
}

/***********************************************************************************
** vow_smartdevice_eint_isr - Smart device EINT handler
************************************************************************************/
#if SMART_AAD_MIC_SUPPORT
void vow_smartdevice_eint_isr(int index)
{
    AUD_LOG_V("eint isr\n");
    vow_smartdevice_eint_handler(vowserv.smart_device_flag);
}
#endif  // #if SMART_AAD_MIC_SUPPORT

#if SW_FIX_METHOD
#ifdef CFG_VCORE_DVFS_SUPPORT
#define VowCheckDataIrqTask_DELAY                  ((portTickType) 10 / portTICK_RATE_MS)
static void task_vow_check_datairq(void *pvParameters)
{
    while (1) {
        if (vowserv.checkDataClean == true) {
            if (NVIC_GetPendingIRQ(MAD_DATA_WAKE_CLK_CTRL) == 0) {
                // MTKIF data pin become clear, then re-open MAD_DATA irq
                unmask_irq(MAD_DATA_IRQn);
                vowserv.checkDataClean = false;
                AUD_LOG_V("wait...\n");
            } else {
                AUD_LOG_V("PendingIRQ:0x%x\n", NVIC_GetPendingIRQ(MAD_DATA_WAKE_CLK_CTRL));
                NVIC_ClearPendingIRQ(MAD_DATA_WAKE_CLK_CTRL);
            }
            vTaskDelay(VowCheckDataIrqTask_DELAY);
        } else {
            vTaskSuspend(xCheckDataIrqHandle);
        }
    }
}
#endif  // #ifdef CFG_VCORE_DVFS_SUPPORT
#endif  // #if SW_FIX_METHOD

static void task_vow_task_loop(void *pvParameters)
{
    AudioTask *this = (AudioTask *)pvParameters;
    uint8_t local_queue_idx = 0xFF;
    ipi_msg_t *ipi_msg;
    unsigned int *item;
    int ret;

    while (1) {
        AUD_LOG_V("vow task is running...\n");
        if (xQueueReceive(this->msg_idx_queue, &local_queue_idx,
                          portMAX_DELAY) == pdTRUE) {
            AUD_LOG_V("vow TaskProject received = %d\n", local_queue_idx);
        }
        ipi_msg = GET_IPI_MSG(local_queue_idx);
#ifdef CHECK_RECOG_TIME
        vow_cur_ipi_msg = (unsigned int)ipi_msg;
#endif  // #ifdef CHECK_RECOG_TIME
        /* wakeup on state change */
        AUD_LOG_V("task_loop: 0x%x\n", ipi_msg->msg_id);
        switch (ipi_msg->msg_id) {
            case IPIMSG_VOW_ENABLE:
                this->state = AUDIO_TASK_INIT;
                vow_enable();
                break;
            case IPIMSG_VOW_DISABLE:
                this->state = AUDIO_TASK_DEINIT;
                vow_disable();
                break;
            case IPIMSG_VOW_APREGDATA_ADDR:
                this->state = AUDIO_TASK_INIT;
                ipi_msg->param1 = VOW_IPI_SUCCESS;
                item = (unsigned int *)ipi_msg->payload;
                vow_setapreg_addr(item[0]);
                break;
            case IPIMSG_VOW_SET_MODEL:
                this->state = AUDIO_TASK_INIT;
                item = (unsigned int *)ipi_msg->payload;
                if ((vow_event_info_t)item[0] == VOW_MODEL_CLEAR) {
                    int slot = vow_fetchMatchModel(item[1]);
                    vow_clearModel(slot, (vow_event_info_t)item[0]);
                } else if ((vow_event_info_t)item[0] == VOW_MODEL_SPEAKER) {
                    int slot = vow_findEmptyModel(); //find a empty slot
                    vow_setModel(slot,(vow_event_info_t)item[0], item[1], item[2], item[3]);
                    vow_loadModel(slot);
                }
                break;
            case IPIMSG_VOW_SET_SMART_DEVICE:
                this->state = AUDIO_TASK_INIT;
#if SMART_AAD_MIC_SUPPORT
                item = (unsigned int *)ipi_msg->payload;
                vow_setSmartDevice((bool)item[0], (unsigned int)item[1]);
                vow_SmartDeviceEnable();
#endif  // #if SMART_AAD_MIC_SUPPORT
                break;
            case IPIMSG_VOW_SET_FLAG:
                this->state = AUDIO_TASK_INIT;
                item = (unsigned int *)ipi_msg->payload;
                vow_set_flag((vow_flag_t)item[0], (short)item[1]);
                // For return value
                switch ((vow_flag_t)item[0]) {
                    case VOW_FLAG_FORCE_PHASE1_DEBUG:
                    case VOW_FLAG_FORCE_PHASE2_DEBUG:
                        if ((vowserv.debug_P1_flag == true) && (vowserv.debug_P2_flag == false)) {
                            ipi_msg->param2 = (item[0] << WORD_H) | FORCE_PHASE1;
                        } else if ((vowserv.debug_P1_flag == false) && (vowserv.debug_P2_flag == true)) {
                            ipi_msg->param2 = (item[0] << WORD_H) | FORCE_PHASE2;
                        } else {
                            ipi_msg->param2 = (item[0] << WORD_H) | NO_FORCE;
                        }
                        break;
                    case VOW_FLAG_SWIP_LOG_PRINT:
                        ipi_msg->param2 = (item[0] << WORD_H) | vowserv.swip_log_anable;
                        break;
                    default:
                        break;
                }
                break;
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
            case IPIMSG_VOW_SET_BARGEIN_ON:
                if (vowserv.bypass_barge_in == true) {
                    AUD_LOG_E("[BargeIn]: bypass bargein start once\n");
                    break;
                }
                item = (unsigned int *)ipi_msg->payload;
                echo_irq_id = item[0];
                ret = vow_bargein_irqid_check(echo_irq_id);
                if (ret != 0)
                    break;
                vowserv.vowTimeFlag |= (BIT_BARGEIN_PERIOD);
                if ((vowserv.vowTimeFlag & (BIT_BARGEIN_PERIOD | BIT_VOW_PERIOD)) == (BIT_BARGEIN_PERIOD | BIT_VOW_PERIOD)) {
                    AUD_LOG_E("[BargeIn] VOW enable ahead Bargein start, irq: %d\n", echo_irq_id);
                    vow_barge_in_start();
                }
                break;
            case IPIMSG_VOW_SET_BARGEIN_OFF:
                item = (unsigned int *)ipi_msg->payload;
                echo_irq_id = item[0];
                ret = vow_bargein_irqid_check(echo_irq_id);
                if (ret != 0)
                    break;
                if ((vowserv.vowTimeFlag & (BIT_BARGEIN_PERIOD | BIT_VOW_PERIOD)) == (BIT_BARGEIN_PERIOD | BIT_VOW_PERIOD)) {
                    AUD_LOG_E("[BargeIn] Bargein stop ahead VOW disable, irq: %d\n", echo_irq_id);
                    vow_barge_in_stop();
                }
                vowserv.vowTimeFlag &= ~(BIT_BARGEIN_PERIOD);
                break;
#endif  // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
            case IPIMSG_VOW_PCM_DUMP_ON:
                item = (unsigned int *)ipi_msg->payload;
                dump_pcm_flag = item[1];
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
                bargein_dump_dram_len = item[0];
                AUD_LOG_D("[Bargein] VOW_PCM_DUMP_ON(%d) ipi 0 = %d, 2 = %x\n",
                          item[1], item[0], item[2]);
#endif  // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
                recog_dump_dram_len = item[3];
                AUD_LOG_D("[Recog] VOW_PCM_DUMP_ON(%d) ipi 3 = %d, 4 = %x\n",
                          item[1], item[3], item[4]);
                if (dump_pcm_flag) {
                    vowserv.PcmDumpFlag = true;
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
                    vowserv.bargein_dump_dram_addr = ap_to_scp(item[2]); //reserved dram addr for bargein dump
                    vow_pcm_dump_info.mic_dump_size = BARGEIN_DUMP_SIZE_BYTE_MIC >> 1;
                    vow_pcm_dump_info.echo_dump_size = BARGEIN_DUMP_SIZE_BYTE_ECHO >> 1;
                    vow_pcm_dump_info.mic_offset = 0;
                    bargein_dump_mic_w = vowserv.bargein_dump_dram_addr;
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                    vow_pcm_dump_info.mic_dump_size_R = BARGEIN_DUMP_SIZE_BYTE_MIC_R >> 1;
                    vow_pcm_dump_info.mic_offset_R = BARGEIN_DUMP_SIZE_BYTE_MIC;
                    bargein_dump_mic_w_R = vowserv.bargein_dump_dram_addr + BARGEIN_DUMP_SIZE_BYTE_MIC;
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                    vow_pcm_dump_info.echo_offset = BARGEIN_DUMP_SIZE_BYTE_MIC + BARGEIN_DUMP_SIZE_BYTE_MIC_R;
                    bargein_dump_echo_w = vowserv.bargein_dump_dram_addr + BARGEIN_DUMP_SIZE_BYTE_MIC + BARGEIN_DUMP_SIZE_BYTE_MIC_R;

#endif  // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
                    vowserv.recog_dump_dram_addr = ap_to_scp(item[4]); //reserved dram addr for recog dump
                    vow_pcm_dump_info.recog_dump_size = RECOG_DUMP_SIZE_BYTE_DATA >> 1;
                    vow_pcm_dump_info.recog_dump_offset = 0;
                    recog_dump_data_w = vowserv.recog_dump_dram_addr;
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                    vow_pcm_dump_info.recog_dump_size_R = RECOG_DUMP_SIZE_BYTE_DATA_R >> 1;
                    vow_pcm_dump_info.recog_dump_offset_R = RECOG_DUMP_SIZE_BYTE_DATA;
                    recog_dump_data_w_R = vowserv.recog_dump_dram_addr + RECOG_DUMP_SIZE_BYTE_DATA;
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                }
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
                AUD_LOG_D("[Bargein] VOW_BARGEIN_DUMP_ON flag = %d MEMORY = %d(byte) dump addr = 0x%x\n",
                          dump_pcm_flag, bargein_dump_dram_len, vowserv.bargein_dump_dram_addr);

                AUD_ASSERT(bargein_dump_dram_len ==
                           BARGEIN_DUMP_SIZE_BYTE_MIC + BARGEIN_DUMP_SIZE_BYTE_MIC_R + BARGEIN_DUMP_SIZE_BYTE_ECHO);
                //initdebugvar();
#endif  // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT

                AUD_ASSERT(recog_dump_dram_len == RECOG_DUMP_SIZE_BYTE_DATA + RECOG_DUMP_SIZE_BYTE_DATA_R);
                break;
            case IPIMSG_VOW_PCM_DUMP_OFF:
                vowserv.PcmDumpFlag = false;
                dump_pcm_flag = false;
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
                bargein_dump_mic_w = 0;
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                bargein_dump_mic_w_R = 0;
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                bargein_dump_echo_w = 0;
                memset(&vow_pcm_dump_info, 0, sizeof(pcm_dump_info_t));
                //AUD_LOG_D("[Bargein] VOW_BARGEIN_DUMP_OFF ipi cnt: %d, size: %d\n", dump_total_count, dump_total);
                //initdebugvar();
#endif  // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
                recog_dump_data_w = 0;
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                recog_dump_data_w_R = 0;
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                break;
            case IPIMSG_VOW_MODEL_START:
                item = (unsigned int *)ipi_msg->payload;
                vow_ModelStart(item[0], item[1]);
                break;
            case IPIMSG_VOW_MODEL_STOP:
                item = (unsigned int *)ipi_msg->payload;
                vow_ModelStop(item[0], item[1]);
                break;
            case VOW_RUN:
                this->state = AUDIO_TASK_WORKING;
                task_vow_working(this, ipi_msg);
                break;
            default:
                AUD_LOG_E("[err]Not support id: 0x%x\n", ipi_msg->msg_id);
                break;
        }
        (void) ret;
        /* send ack back if need */
        AUD_LOG_V("%s(), id:%x, ack:%x\n", __func__, ipi_msg->msg_id, ipi_msg->ack_type);
        audio_send_ipi_msg_ack_back(ipi_msg);  // call this for need ack msg
        /* clean msg */
        memset(ipi_msg, 0, ipi_msg_size);
        this->num_queue_element--;

        if (vowserv.can_unlock_scp_flag == true) {
            AUD_LOG_V("unlock_scp\n");

            vow_wake_unlock(&vow_wakelock);
            vowserv.scpwakelock = SCP_UNLOCK;
            AUD_LOG_D("<VOW UNLOCK SCP>\n");
            if (vowserv.vow_status != VOW_STATUS_IDLE) {  // if is disable, then bypass
#if PMIC_6337_SUPPORT
#if SW_FIX_METHOD
                vowserv.checkDataClean = true;  // must do this after vow_datairq_reset()
                vTaskResume(xCheckDataIrqHandle);  // must do this after vow_datairq_reset()
#else
                unmask_irq(MAD_DATA_IRQn);
#endif  // #if SW_FIX_METHOD
#endif  // #if PMIC_6337_SUPPORT
#if SMART_AAD_MIC_SUPPORT
                if (vowserv.smart_device_flag == true) {
#ifdef CFG_EINT_SUPPORT
                    mt_eint_ack(vowserv.smart_device_eint_id);
                    mt_eint_unmask(vowserv.smart_device_eint_id);
#endif  // #ifdef CFG_EINT_SUPPORT
                }
#endif  // #if SMART_AAD_MIC_SUPPORT
            }
            vowserv.can_unlock_scp_flag = false;
            // extern clk_enum get_cur_clk(void);
            // AUD_LOG_D("cur_clk:%x\n", get_cur_clk());
        }
    }
}

uint8_t task_vow_preparsing_message(struct AudioTask *this, ipi_msg_t *ipi_msg)
{
    uint8_t ret = false;

    // AUD_LOG_V("%s(), id:%x\n", __func__, ipi_msg->msg_id);
    switch (ipi_msg->msg_id) {
        case IPIMSG_VOW_ENABLE:
        case IPIMSG_VOW_DISABLE:
        case IPIMSG_VOW_APREGDATA_ADDR:
        case IPIMSG_VOW_SET_MODEL:
        case IPIMSG_VOW_SET_SMART_DEVICE:
        case IPIMSG_VOW_SET_FLAG:
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        case IPIMSG_VOW_SET_BARGEIN_ON:
        case IPIMSG_VOW_SET_BARGEIN_OFF:
#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        case IPIMSG_VOW_PCM_DUMP_ON:
        case IPIMSG_VOW_PCM_DUMP_OFF:
        case IPIMSG_VOW_MODEL_START:
        case IPIMSG_VOW_MODEL_STOP:
            ipi_msg->data_type = AUDIO_IPI_MSG_ONLY;
            ipi_msg->param1 = VOW_IPI_SUCCESS;
            ret = true;
            break;
        /*
        case IPIMSG_VOW_DATAREADY_ACK:
            ipi_msg->data_type = AUDIO_IPI_MSG_ONLY;
            ipi_msg->param1 = VOW_IPI_SUCCESS;
            vow_sync_data_ready();
            ret = false;
            break;
        */
        default:
            AUD_LOG_E("[err]Not support id: 0x%x\n", ipi_msg->msg_id);
            break;
    }
    return ret;
}


/***********************************************************************************
** task_vow_init - Initial funciton in scp init
************************************************************************************/
static void task_vow_init(struct AudioTask *this)
{
    unsigned int I;

    AUD_LOG_V("vow init\n");
#ifdef VOW_WAKELOCK
    wake_lock_init(&vow_wakelock, "vow1");
#endif  // #ifdef VOW_WAKELOCK
    vowserv.vow_status      = VOW_STATUS_IDLE;
    vowserv.allowRecog_flag = false;
    vowserv.read_idx        = VOW_LENGTH_VOICE + 1;
    vowserv.write_idx       = 0;
#ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
    vowserv.seamless_buf_w_ptr = 0;
    memset(vowserv_buf_seamless_rec,   0, SEAMLESS_RECORD_BUF_SIZE * sizeof(short));
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
    memset(vowserv_buf_seamless_rec_R, 0, SEAMLESS_RECORD_BUF_SIZE * sizeof(short));
    vowserv.is_headset      = false;
#endif
#endif  // #ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
#ifdef BLISRC_USE
    vowserv.blisrc_r_idx    = 0;
    vowserv.blisrc_w_idx    = 0;
#endif  // #ifdef BLISRC_USE
    vowserv.pre_learn       = false;
    vowserv.record_flag     = false;
    vowserv.debug_P1_flag   = false;
    vowserv.debug_P2_flag   = false;
    vowserv.swip_log_anable = VOW_SWIP_HAS_LOG;
    vowserv.drop_frame      = 0;
    vowserv.apreg_addr      = 0;
    vowserv.dmic_low_power  = false;
    vowserv.mtkif_type      = 0;
    vowserv.samplerate      = VOW_SAMPLERATE_16K;
    vowserv.dc_removal_enable = true;
    vowserv.dc_removal_order = 5;  // default
    vowserv.can_unlock_scp_flag  = false;
    vowserv.scpwakelock     = SCP_UNLOCK;
#if PMIC_6337_SUPPORT
    vowserv.flr_enable      = 0;
    vowserv.dvfs_mode       = VOW_DVFS_SW_MODE;
    vowserv.open_periodic_flag   = false;
    vowserv.periodic_need_enable = false;
#endif  // #if PMIC_6337_SUPPORT
#if SW_FIX_METHOD
    vowserv.checkDataClean       = false;
#endif  // #if SW_FIX_METHOD
    for (I = 0; I < MAX_VOW_SPEAKER_MODEL; I++) {
        vowserv.vow_model_event[I].func = vow_getModel_speaker;
        vowserv.vow_model_event[I].addr = 0;
        vowserv.vow_model_event[I].size = 0;
        vowserv.vow_model_event[I].flag = false;
        vowserv.vow_model_event[I].id   = -1;
        vowserv.vow_model_event[I].enable = false;
    }
    memset(vowserv_buf_speaker_A, 0, VOW_LENGTH_SPEAKER * sizeof(short));
#ifdef CFG_MTK_VOW_2E2K_SUPPORT
    memset(vowserv_buf_speaker_B, 0, VOW_LENGTH_SPEAKER * sizeof(short));
#endif
    memset(vowserv_buf_voice, 0, VOW_LENGTH_VOICE * sizeof(short));
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
    vowserv.mic_num = 2;
    memset(vowserv_buf_voice_R, 0, VOW_LENGTH_VOICE * sizeof(short));
#else
    vowserv.mic_num = 1;
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
#ifdef BLISRC_USE
    memset(blisrc_internal_buf, 0, BLISRC_INTERNAL_BUFFER_WORD_SIZE * sizeof(unsigned int));
    memset(vowserv_buf_blic_out, 0, BLISRC_OUTPUT_TEMP_BUFFER * sizeof(short));
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
    memset(vowserv_buf_blic_out_R, 0, BLISRC_OUTPUT_TEMP_BUFFER * sizeof(short));
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
#endif  // #ifdef BLISRC_USE
    vowserv.vow_model_event[0].data = (short*)vowserv_buf_speaker_A;
#ifdef CFG_MTK_VOW_2E2K_SUPPORT
    vowserv.vow_model_event[1].data = (short*)vowserv_buf_speaker_B;
#endif
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
    vowserv.bargeInStatus = BARGEIN_OFF;
    vowserv.vowTimeFlag = 0;
    vowserv.bypass_barge_in = false;
    memset(vowserv_buf_echo_ref_in, 0, ECHO_REF_BUF_LEN * sizeof(short));
    memset(vowserv_buf_echo_ref_out, 0, ECHO_REF_FRM_LEN * sizeof(short));
    memset(vowserv_buf_echo_ref_sil, 1, ECHO_REF_FRM_LEN * sizeof(short));
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
    memset(vowserv_buf_echo_ref_out_R, 0, ECHO_REF_FRM_LEN * sizeof(short));
#endif  // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT

#ifdef VOW_ECHO_SW_SRC
    vowserv.echo_nonsrc_w_idx = ECHO_REF_NONSRC_FRM_LEN * 2;
    vowserv.echo_nonsrc_r_idx = 0;
    vowserv.echo_nonsrc_data_len = ECHO_REF_NONSRC_FRM_LEN * 2;
    memset(blisrc_echo_ref_internal_buf, 0, BLISRC_ECHO_INTERNAL_BUFFER_WORD_SIZE * sizeof(short));
    memset(vowserv_buf_echo_ref_nonsrc, 0, ECHO_REF_NONSRC_BUF_LEN * sizeof(short));
#endif // #ifdef VOW_ECHO_SW_SRC

    vowserv.echo_delay_samples = ECHO_DELAY_SAMPLE;
    dvfs_enable_DRAM_resource(VOW_MEM_ID);
    blisrc_accurate_vow_open(NULL, NULL, NULL);
    AEC_init(vowserv.mic_num, EC_REF_NUMBER, micConfPath, ECHO_REF_FRM_LEN, vowserv.echo_delay_samples);
    dvfs_disable_DRAM_resource(VOW_MEM_ID);
    AUD_LOG_D("AEC Init echo delay: %d mic num: %d\n", vowserv.echo_delay_samples, vowserv.mic_num);
#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
}

/***********************************************************************************
** task_vow_deinit - Deinitial funciton in scp
************************************************************************************/
void task_vow_deinit(struct AudioTask *this)
{
    vow_disable();
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
    dvfs_enable_DRAM_resource(VOW_MEM_ID);
    AEC_release();
    dvfs_disable_DRAM_resource(VOW_MEM_ID);
#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
    vow_wake_unlock(&vow_wakelock);
    vowserv.scpwakelock = SCP_UNLOCK;

}

/***********************************************************************************
** vow_enable - enable VOW
************************************************************************************/
static void vow_enable(void)
{
    int ret = 0;
#ifdef BLISRC_USE
    blisrc_vow_init_param_t param;
    uint32_t byte_cnt;
#endif  // #ifdef BLISRC_USE
    AUD_LOG_D("+Enable_%x\n", vowserv.vow_status);
    AUD_LOG_E("SCP_VOW_TASK_VER: %s\n", SCP_VOW_TASK_VER);
    if (vowserv.vow_status == VOW_STATUS_IDLE) {

        // Enable VOW FIFO
        vow_enable_fifo(&vowserv);

        vowserv.read_idx = VOW_LENGTH_VOICE + 1;
        vowserv.write_idx = 0;
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        vowserv.bargein_dump_frm = 0;
        vowserv.vowTimeFlag &= ~(BIT_VOICE_R_IDX_UPDATE);
#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
#ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
        vowserv.seamless_buf_w_ptr = 0;
#endif  // #ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
#ifdef BLISRC_USE
        vowserv.blisrc_r_idx = 0;
        vowserv.blisrc_w_idx = 0;
#endif  // #ifdef BLISRC_USE
        vowserv.data_length = 0;
        vowserv.first_irq_flag = true;
        vowserv.frm_cnt = 0;
#if PMIC_6337_SUPPORT
        if (!vowserv.debug_P1_flag) {
            vow_lock_vow_event();
        }
#endif  // #if PMIC_6337_SUPPORT
        vow_dc_removal(vowserv.dc_removal_enable, vowserv.dc_removal_order);
#if PMIC_6337_SUPPORT
        vowserv.flr_enable = vow_flr_setting();
        AUD_LOG_D("Setting:DVFS_mode=%x, FLR_Enable=%x, PeriodicOnOff=%x\n",
                  vowserv.dvfs_mode,
                  vowserv.flr_enable,
                  vowserv.periodic_need_enable);
#endif  // #if PMIC_6337_SUPPORT
        /* Inintail settings for 6358 low power mode*/
        vow_pmic_low_power_control(&vowserv, VOW_PMIC_LOW_POWER_INIT);
        vow_pmic_idleTophase1(&vowserv);

#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        vowserv.vowTimeFlag |= (BIT_VOW_PERIOD);
        AUD_LOG_V("[Bargein] vowserv.vowTimeFlag: %d\n", vowserv.vowTimeFlag);
        if ((vowserv.vowTimeFlag & (BIT_BARGEIN_PERIOD | BIT_VOW_PERIOD)) == (BIT_BARGEIN_PERIOD | BIT_VOW_PERIOD)) {
            AUD_LOG_E("[BargeIn] Bargein start ahead VOW enable\n");
            vow_barge_in_start();
        }
#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        AUD_LOG_D("debug P1:%d, P2:%d\n", vowserv.debug_P1_flag, vowserv.debug_P2_flag);
        if (vowserv.debug_P1_flag) {
            vow_force_phase1();
        }
        if (((vowserv.pre_learn) || (vowserv.record_flag) || (vowserv.debug_P2_flag)) && (!vowserv.debug_P1_flag)) {
            // Force Phase2
            AUD_LOG_V("Enable, prelearn\n");
            vow_force_phase2(&vowserv);

#if PMIC_6337_SUPPORT
            vow_datairq_clear();
            if (vowserv.flr_enable) {
                vow_flr_suspend();
            }
            vowserv.open_periodic_flag = true;
#endif  // #if PMIC_6337_SUPPORT
            // for phase2 setting, need lock scp
            if (vowserv.record_flag) {
                vow_wake_lock(&vow_wakelock);
                vowserv.scpwakelock = SCP_LOCK;
                AUD_LOG_E("<VOW LOCK SCP>\n");
            }
        } else {
#if PMIC_6337_SUPPORT
            AUD_LOG_D("enable Periodic: %x\n", vowserv.periodic_need_enable);
            // Start form Phase1
            if (!vowserv.debug_P1_flag) {
                vow_datairq_reset();
            }
            vow_set_periodic_onoff(vowserv.periodic_need_enable);
#endif  // #if PMIC_6337_SUPPORT
        }
#if PMIC_6337_SUPPORT
        if (!vowserv.debug_P1_flag) {
            vow_unlock_vow_event();
        }
#endif  // #if PMIC_6337_SUPPORT
        vowserv.drop_count = 0;
        if (vowserv.pre_learn) {
            vowserv.drop_frame = VOW_DROP_FRAME_FOR_DC;
        }
        vowserv.vow_status = VOW_STATUS_SCP_REG;
        vowserv.pcmdump_cnt = 0;
#if SMART_AAD_MIC_SUPPORT
        if (vowserv.smart_device_flag == true) {
#ifdef CFG_EINT_SUPPORT
            // mt_eint_dump_all_config();
            mt_eint_ack(vowserv.smart_device_eint_id);
            mt_eint_unmask(vowserv.smart_device_eint_id);
#endif  // #ifdef CFG_EINT_SUPPORT
        }
#endif  // #if SMART_AAD_MIC_SUPPORT

        // For SW VAD init
        initSwVad();
        vowserv.noKeywordCount = 0;
#ifdef BLISRC_USE
        param.input_sampling_rate   = BLISRC_INPUT_SAMPLING_RATE_IN_HZ;
        param.output_sampling_rate  = BLISRC_OUTPUT_SAMPLING_RATE_IN_HZ;
        param.input_channel_number  = BLISRC_INPUT_CHANNEL_NUBMER;
        param.output_channel_number = BLISRC_OUTPUT_CHANNEL_NUBMER;
        blisrc_vow_open(&blisrc_handle, (unsigned char *)blisrc_internal_buf, &param);
        double_bli_output_next = false;
        byte_cnt = blisrc_vow_get_buffer_size();
        if (byte_cnt != sizeof(blisrc_internal_buf)) {
            AUD_LOG_E("[BargeIn] normal blisrc buf size error, Query byte cnt: %ld, buf byte cnt: %d\n", byte_cnt,
                      sizeof(blisrc_internal_buf));
        }
#endif  // #ifdef BLISRC_USE
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
        vow_headset_detection(&vowserv);
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT

#if TIMES_DEBUG
        total_recog_time = 0;
        avg_cnt = 0;
        start_recog_time = 0;
        end_recog_time = 0;
        max_recog_time = 0;
        start_task_time = 0;
        end_task_time = 0;
        max_task_time = 0;
        start_blisrc_time = 0;
        end_blisrc_time = 0;
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        start_ec_time = 0;
        end_ec_time = 0;
#endif  // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        CPU_RESET_CYCLECOUNTER_1();
#endif  // #if TIMES_DEBUG
        // set wakeup source
#if PMIC_6337_SUPPORT
#ifdef CFG_VCORE_DVFS_SUPPORT
        scp_wakeup_src_setup(MAD_DATA_WAKE_CLK_CTRL, 1);
#endif  // #ifdef CFG_VCORE_DVFS_SUPPORT
        unmask_irq(MAD_DATA_IRQn);
#else  // #if PMIC_6337_SUPPORT
#ifdef CFG_VCORE_DVFS_SUPPORT
        scp_wakeup_src_setup(MAD_FIFO_WAKE_CLK_CTRL, 1);
#endif  // #ifdef CFG_VCORE_DVFS_SUPPORT
#endif  // #if PMIC_6337_SUPPORT
        unmask_irq(MAD_FIFO_IRQn);

        AUD_LOG_D("-Enable_%x_%x\n", vowserv.vow_status, ret);
    }
}

/***********************************************************************************
s** vow_disable - disable VOW
************************************************************************************/
static void vow_disable(void)
{
    AUD_LOG_D("+Disable_%x\n", vowserv.vow_status);
    if (vowserv.vow_status != VOW_STATUS_IDLE) {
#ifdef CHECK_RECOG_TIME
        /* stop checking SWIP operating time when disable vow */
        xTimerStop(xVowTimer, 0);
#endif  // #ifdef CHECK_RECOG_TIME
        // Clear wakeup source
        vowserv.can_unlock_scp_flag = true;
#if PMIC_6337_SUPPORT
#ifdef CFG_VCORE_DVFS_SUPPORT
        scp_wakeup_src_setup(MAD_DATA_WAKE_CLK_CTRL, 0);
#endif  // #ifdef CFG_VCORE_DVFS_SUPPORT
        mask_irq(MAD_DATA_IRQn);
        vowserv.open_periodic_flag = false;
        vow_datairq_clear();
#else  // #if PMIC_6337_SUPPORT
#ifdef CFG_VCORE_DVFS_SUPPORT
        scp_wakeup_src_setup(MAD_FIFO_WAKE_CLK_CTRL, 0);
#endif  // #ifdef CFG_VCORE_DVFS_SUPPORT
#endif  // #if PMIC_6337_SUPPORT
        mask_irq(MAD_FIFO_IRQn);
#if SMART_AAD_MIC_SUPPORT
        if (vowserv.smart_device_flag == true) {
            // Clear wakeup source
            vowserv.smart_device_flag = false;
#ifdef CFG_VCORE_DVFS_SUPPORT
            scp_wakeup_src_setup(EINT_WAKE_CLK_CTRL, 0);
#endif  // #ifdef CFG_VCORE_DVFS_SUPPORT
#ifdef CFG_EINT_SUPPORT
            mt_eint_mask(vowserv.smart_device_eint_id);
#endif  // #ifdef CFG_EINT_SUPPORT
        }
#endif  // #if SMART_AAD_MIC_SUPPORT
        vow_disable_fifo();

        vow_pmic_low_power_control(&vowserv, VOW_PMIC_LOW_POWER_EXIT);
        vowserv.first_irq_flag = false;

#if PMIC_6337_SUPPORT
        vow_lock_vow_event();
#endif  // #if PMIC_6337_SUPPORT

#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        if ((vowserv.vowTimeFlag & (BIT_BARGEIN_PERIOD | BIT_VOW_PERIOD)) == (BIT_BARGEIN_PERIOD | BIT_VOW_PERIOD)) {
            vow_barge_in_stop();
            AUD_LOG_E("[BargeIn] ahead VOW disable ahead Bargein stop\n");
        }
        vowserv.bypass_barge_in = false;
        vowserv.vowTimeFlag = 0;
#endif  // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
        vowserv.is_headset = false;
#endif  // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
        vow_pmic_idle(&vowserv);
        vow_dc_removal(false, 0);

#if PMIC_6337_SUPPORT
        if (vowserv.flr_enable) {
            vow_flr_suspend();
        }
        vowserv.flr_enable = 0;
        vow_unlock_vow_event();
#endif  // #if PMIC_6337_SUPPORT
        vowserv.vow_status = VOW_STATUS_IDLE;
        vowserv.read_idx = VOW_LENGTH_VOICE + 1;
        vowserv.write_idx = 0;
#ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
        vowserv.seamless_buf_w_ptr = 0;
#endif  // #ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
#ifdef BLISRC_USE
        vowserv.blisrc_r_idx = 0;
        vowserv.blisrc_w_idx = 0;
#endif  // #ifdef BLISRC_USE
        vowserv.data_length = 0;

        // For SW VAD
        vowserv.noKeywordCount = 0;
        AUD_LOG_D("-Disable_%x\n", vowserv.vow_status);
    }
}

/***********************************************************************************
** vow_ready_receive_fifo - for  Mode Switch
************************************************************************************/
#if PMIC_6337_SUPPORT
static void vow_ready_receive_fifo(void)
{
    vow_lock_vow_event();
    vow_pmic_phase1Tophase2(VOW_DVFS_SW_MODE_DONE);
    vow_unlock_vow_event();
}
#endif  // #if PMIC_6337_SUPPORT

/***********************************************************************************
** vow_sync_data_ready - for sync every pcm dump package
************************************************************************************/
/*
static void vow_sync_data_ready(void)
{
#ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
    vowserv.sync_data_ready = true;
#endif  // #ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
}
*/
/***********************************************************************************
** vow_getModel_speaker - Get speaker model informations through DMA
************************************************************************************/
static void vow_getModel_speaker(int slot)
{
    char *ptr8;
    unsigned int ret;

    if ((slot < 0) || (slot >= MAX_VOW_SPEAKER_MODEL)) {
        AUD_LOG_D("%s, slot err:%d\n", __func__, slot);
        return;
    }
    if (vowserv.vow_model_event[slot].size == 0) {
        AUD_LOG_D("%s, size err\n", __func__);
        return;
    }
    if (vowserv.vow_model_event[slot].addr == 0) {
        AUD_LOG_D("%s, addr err\n", __func__);
        return;
    }
    AUD_LOG_D("GetModel S_%x %x %x\n",
              vowserv.vow_model_event[slot].addr,
              vowserv.vow_model_event[slot].size,
              (unsigned int)vowserv.vow_model_event[slot].data);
    ret = vow_dma_transaction((uint32_t)vowserv.vow_model_event[slot].data,
                              (uint32_t)vowserv.vow_model_event[slot].addr,
                              (uint32_t)vowserv.vow_model_event[slot].size);
    if (ret != DMA_RESULT_DONE) {
        AUD_LOG_E("%s(), [err]DMA fail:%x\n", __func__, ret);
    }
    // To do: DMA Speaker Model into vowserv_buf_speaker
    ptr8 = (char*)vowserv.vow_model_event[slot].data;

    AUD_LOG_D("check_%x %x %x %x %x %x\n", *(char*)&ptr8[0], *(char*)&ptr8[1],
              *(char*)&ptr8[2], *(char*)&ptr8[3],
              *(short*)&ptr8[160], *(int*)&ptr8[7960]);
}

/***********************************************************************************
** vow_setapreg_addr - Get noise model informations through DMA
************************************************************************************/
static void vow_setapreg_addr(unsigned int addr)
{
    AUD_LOG_D("SetAPREG_%x_%x\n", addr, vowserv.apreg_addr);
    if (vowserv.apreg_addr == 0) {
        vowserv.apreg_addr = ap_to_scp(addr);  // AP's address
        AUD_LOG_V("SetAPREG_done\n");
    }
}

/***********************************************************************************
** vow_set_flag - Enable/Disable specific operations
************************************************************************************/
static void vow_set_flag(vow_flag_t flag, short enable)
{
    AUD_LOG_D("%s(), %x, %x\n", __func__, flag, enable);
    switch (flag) {
        case VOW_FLAG_SEAMLESS:
#ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
            AUD_LOG_D("seamless_record_%x\n", enable);
            vowserv.seamless_record = enable;
#endif  // #ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
            break;
        case VOW_FLAG_DEBUG:
            AUD_LOG_D("FgDebug_%x\n", enable);
            vowserv.record_flag = enable;
            break;
        case VOW_FLAG_PRE_LEARN:
            AUD_LOG_V("FgPreL_%x\n", enable);
            vowserv.pre_learn = enable;
            break;
        case VOW_FLAG_DMIC_LOWPOWER:
            AUD_LOG_V("FgDmic_%x\n", enable);
            vowserv.dmic_low_power = enable;
            break;
        case VOW_FLAG_PERIODIC_ENABLE:
#if PMIC_6337_SUPPORT
            AUD_LOG_V("PeriodicEn_%x\n", enable);
            vowserv.periodic_need_enable = enable;
#endif  // #if PMIC_6337_SUPPORT
            break;
        case VOW_FLAG_FORCE_PHASE1_DEBUG:
            AUD_LOG_V("Force Phase1 Debug:%x\n", enable);
            vowserv.debug_P1_flag = enable;
            if ((vowserv.debug_P1_flag == true) && (vowserv.debug_P2_flag == true)) {
                AUD_LOG_V("clear Phase2 Debug\n");
                vowserv.debug_P2_flag = false;
            }
            break;
        case VOW_FLAG_FORCE_PHASE2_DEBUG:
            AUD_LOG_V("Force Phase2 Debug:%x\n", enable);
            vowserv.debug_P2_flag = enable;
            if ((vowserv.debug_P1_flag == true) && (vowserv.debug_P2_flag == true)) {
                AUD_LOG_V("clear Phase1 Debug\n");
                vowserv.debug_P1_flag = false;
            }
            break;
        case VOW_FLAG_SWIP_LOG_PRINT:
#ifdef __VOW_MTK_SUPPORT__
            AUD_LOG_V("SWIP show log:%x\n", enable);
            vowserv.swip_log_anable = enable;
            VOWE_DSP_testing_setArgument(vowe_argid_printLog, vowserv.swip_log_anable);
#endif
            break;
        case VOW_FLAG_MTKIF_TYPE:
            AUD_LOG_V("MTKIF type:%x\n", enable);
            vowserv.channel = (enable >> 4);
            vowserv.mtkif_type = enable & 0x0F;
            break;
        case VOW_FLAG_DUAL_MIC_LCH:
            AUD_LOG_D("Dual mic L channel flag:%x\n", enable);
            scp_vow_lch = (bool) enable;
            break;
        default:
            break;
    }
}

/***********************************************************************************
** vow_getEintNumber - Indicate using smart external device
************************************************************************************/
#if SMART_AAD_MIC_SUPPORT
static unsigned int vow_getEintNumber(unsigned int eint_num)
{
    return (EINT_NUMBER_0 + eint_num);
}
#endif  // #if SMART_AAD_MIC_SUPPORT

/***********************************************************************************
** vow_setSmartDevice - Indicate using smart external device
************************************************************************************/
#if SMART_AAD_MIC_SUPPORT
static void vow_setSmartDevice(bool enable, unsigned int eint_num)
{
    AUD_LOG_D("vow_setSmartDevice, need open:%x\n", enable);
    vowserv.smart_device_need_enable = enable;
    if (enable == true) {
        vowserv.smart_device_eint_id = vow_getEintNumber(eint_num);
    }
}
#endif  // #if SMART_AAD_MIC_SUPPORT

/***********************************************************************************
** vow_SmartDeviceEnable - Indicate using smart external device
************************************************************************************/
#if SMART_AAD_MIC_SUPPORT
static void vow_SmartDeviceEnable(void)
{
    if (vowserv.smart_device_need_enable == true) {
        AUD_LOG_D("smart device enable, flag:%x\n", vowserv.smart_device_flag);
        if (vowserv.smart_device_flag == false) {
            vowserv.smart_device_flag = true;

            if (vowserv.smart_device_eint_id > EINT_NUMBER_19) {
                AUD_LOG_E("eint id(0x%x) is out of order!\n", vowserv.smart_device_eint_id);
            }
#ifdef CFG_EINT_SUPPORT
            mt_eint_registration(vowserv.smart_device_eint_id,
                                 EDGE_SENSITIVE,
                                 HIGH_LEVEL_TRIGGER,
                                 vow_smartdevice_eint_isr,
                                 EINT_INT_MASK,
                                 EINT_INT_AUTO_UNMASK_OFF);
            mt_eint_dis_hw_debounce(vowserv.smart_device_eint_id);
#endif  // #ifdef CFG_EINT_SUPPORT
#ifdef CFG_VCORE_DVFS_SUPPORT
            scp_wakeup_src_setup(EINT_WAKE_CLK_CTRL, 1);
#endif  // #ifdef CFG_VCORE_DVFS_SUPPORT
            vowserv.samplerate = VOW_SAMPLERATE_16K;
            AUD_LOG_D("[VOW]mt_eint_read_status %x\n", vowserv.smart_device_eint_id);
        }
    } else if (vowserv.smart_device_need_enable == false) {
        AUD_LOG_D("smart device disable\n");
        vowserv.smart_device_flag = false;
        vowserv.samplerate = VOW_SAMPLERATE_16K;
    }
    AUD_LOG_D("Set smart_device_flag %x, id=%d\n", vowserv.smart_device_flag, vowserv.smart_device_eint_id);
}
#endif  // #if SMART_AAD_MIC_SUPPORT


/***********************************************************************************
** vow_findEmptyModel - find out empty member, and return slot number
************************************************************************************/
static int vow_findEmptyModel(void)
{
    int I;
    for (I = 0; I < MAX_VOW_SPEAKER_MODEL; I++) {
        if (vowserv.vow_model_event[I].flag == false) {
            return I;
        }
    }
    AUD_LOG_D("[VOW]No empty model\n");
    return -1;
}

/***********************************************************************************
** vow_fetchMatchModel - fetch id match model, and return slot number
************************************************************************************/
static int vow_fetchMatchModel(int id)
{
    int I;

    for (I = 0; I < MAX_VOW_SPEAKER_MODEL; I++) {
        if ((vowserv.vow_model_event[I].id == id) &&
            (vowserv.vow_model_event[I].flag == true)) {
                return I;
        }
    }
    AUD_LOG_D("[VOW]No Match model, id:%d\n", id);
    return -1;
}

/***********************************************************************************
** vow_clearModel - clear model information
************************************************************************************/
static void vow_clearModel(int slot, vow_event_info_t type)
{
    int ret;

    AUD_LOG_D("[vow]ClearModel+, slot:%x\n", slot);

    AUD_ASSERT(type == VOW_MODEL_CLEAR);
    AUD_ASSERT((slot >= 0) && (slot < MAX_VOW_SPEAKER_MODEL));

    ret = vow_model_deinit(&vowserv.vow_model_event[slot]);
    if (ret == -1)
        AUD_LOG_D("%s() err\n", __func__);

    vowserv.vow_model_event[slot].id = -1;
    vowserv.vow_model_event[slot].addr = 0;
    vowserv.vow_model_event[slot].size = 0;
    vowserv.vow_model_event[slot].flag = false;
    vowserv.vow_model_event[slot].enable = false;

    return;
}

/***********************************************************************************
** vow_setModel - set model information
************************************************************************************/
static void vow_setModel(int slot, vow_event_info_t type, int id, int addr, int size)
{
    AUD_LOG_D("[vow]SetModel+, slot:%d, %x %x %x %x\n", slot, type, id, addr, size);

    AUD_ASSERT(type == VOW_MODEL_SPEAKER);
    AUD_ASSERT((size >> 1) <= VOW_LENGTH_SPEAKER);
    AUD_ASSERT((slot >= 0) && (slot < MAX_VOW_SPEAKER_MODEL));

    AUD_LOG_D("cur model addr:0x%x\n", vowserv.vow_model_event[slot].addr);

    vowserv.vow_model_event[slot].id = id;
    vowserv.vow_model_event[slot].addr = ap_to_scp(addr);
    vowserv.vow_model_event[slot].size = size;
    vowserv.vow_model_event[slot].flag = true;

    return;
}

/***********************************************************************************
** vow_LoadModel - Load Model Infomation from AP
************************************************************************************/
static void vow_loadModel(int slot)
{
    int ret;

    AUD_ASSERT((slot >= 0) && (slot < MAX_VOW_SPEAKER_MODEL));
    vowserv.vow_model_event[slot].func(slot);
    ret = vow_model_init(&vowserv.vow_model_event[slot]);
    if (ret == -1)
        AUD_LOG_D("%s() err\n", __func__);
}

/***********************************************************************************
** vow_ModelStart - enable the id model and allow to recognize
************************************************************************************/
static void vow_ModelStart(int id, unsigned int confidence_level)
{
    int slot;
    int ret;

    slot = vow_fetchMatchModel(id);
    if (slot == -1)
        return;

    // dato todo: add condition to avoid mutiple init
    if (vowserv.vow_model_event[slot].enable == false) {
        vowserv.vow_model_event[slot].confidence_lv = confidence_level;
        ret = vow_model_setconfig(&vowserv.vow_model_event[slot]);
        if (ret == -1)
            AUD_LOG_D("%s() err\n", __func__);

        vowserv.vow_model_event[slot].enable = true;

        vowserv.allowRecog_flag = true;

        if (vowserv.bypass_barge_in == true) {
            vowserv.bypass_barge_in = false;
        }
        AUD_LOG_D("[vow]%s, slot:%d, id:%d, enable:%d, confidence_lv:%d\n",
                  __func__, slot, id,
                  vowserv.vow_model_event[slot].enable,
                  vowserv.vow_model_event[slot].confidence_lv);
    }
}

/***********************************************************************************
** vow_ModelStop - disable the id model and stop to recognize
************************************************************************************/
static void vow_ModelStop(int id, unsigned int confidence_level)
{
    int slot;
    int I;
    bool enable_flag = false;

    slot = vow_fetchMatchModel(id);
    if (slot == -1)
        return;

    if (vowserv.vow_model_event[slot].enable == true) {
        vowserv.vow_model_event[slot].enable = false;
        vowserv.vow_model_event[slot].confidence_lv = confidence_level;

        AUD_LOG_D("[vow]%s, slot:%d, id:%d, enable:%d, confidence_lv:%d\n",
                  __func__, slot, id,
                  vowserv.vow_model_event[slot].enable,
                  vowserv.vow_model_event[slot].confidence_lv);
        enable_flag = false;
        for (I = 0; I < MAX_VOW_SPEAKER_MODEL; I++) {
            AUD_LOG_D("[vow]%s, model[%d].en = %d\n",
                  __func__, I,
                  vowserv.vow_model_event[I].enable);
            if (vowserv.vow_model_event[I].enable == true) {
                enable_flag = true;
            }
        }
        vowserv.allowRecog_flag = enable_flag;

#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        // if enable_flag = 0, means no model is running, bypass barge-in start
        if (enable_flag == false) {
            vowserv.bypass_barge_in = true;
            AUD_LOG_D("[vow]no model, bypass start bargein\n");
        }
#endif  // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
    }
}

#ifdef __VOW_AMAZON_SUPPORT__
// keyword detection callback
static void detectionCallback(PryonLiteDecoderHandle handle, const PryonLiteResult* result)
{
    AUD_LOG_D("[AMAZON]Detected keyword '%s', condifence '%d'\n", result->keyword, result->confidence);
    AUD_LOG_D("[AMAZON]begin '%d', end '%d'\n", result->beginSampleIndex, result->endSampleIndex);
    pryonLite_recog_ok = true;
    pryonLite_confidence_lv = result->confidence;
}

// VAD event callback
static void vadCallback(PryonLiteDecoderHandle handle, const PryonLiteVadEvent* vadEvent)
{
    AUD_LOG_D("[AMAZON]VAD state %d\n", (int) vadEvent->vadState);
    pryonLite_vadState = vadEvent->vadState;
}
#endif

/***********************************************************************************
** vow_model_setconfig - Initialize SWIP with different model_type
************************************************************************************/
int vow_model_setconfig(vow_model_t *model_ptr) {
    int ret = 1;

    switch (model_ptr->id) {
        case VENDOR_ID_OTHERS:
            break;
        case VENDOR_ID_MTK:
            break;
        case VENDOR_ID_AMAZON: {
#ifdef __VOW_AMAZON_SUPPORT__
                PryonLiteError err;
                int confidence_lv = 0;

                // Set detection threshold for all keywords
                // (this function can be called any time after decoder initialization)
                // confidence level: 0~100 mapping to 0~1000
                if ((model_ptr->confidence_lv >= 0) && (model_ptr->confidence_lv <= 100)) {
                    confidence_lv = 10 * model_ptr->confidence_lv;
                    if (confidence_lv > 1000) {
                        confidence_lv = 1000;
                    }
                    AUD_LOG_D("[AMAZON]confidence lv=%d(%d)\n", confidence_lv, model_ptr->confidence_lv);
                    err = PryonLiteDecoder_SetDetectionThreshold(sDecoder, NULL, confidence_lv);
                    if (err != PRYON_LITE_ERROR_OK) {
                        AUD_LOG_D("[AMAZON]PryonLiteDecoder_SetDetectionThreshold err=%d\n", err);
                        return -1;
                    }
                } else {
                    AUD_LOG_D("[AMAZON]condifence level not in range: %d\n", model_ptr->confidence_lv);
                }
#endif
            }
            break;
        default:
            break;
    }
    return ret;
}

/***********************************************************************************
** vow_model_deinit - Initialize SWIP with different model_type
************************************************************************************/
int vow_model_deinit(vow_model_t *model_ptr) {
    int ret = 1;

    switch (model_ptr->id) {
        case VENDOR_ID_OTHERS:
            break;
        case VENDOR_ID_MTK:
            break;
        case VENDOR_ID_AMAZON: {
#ifdef __VOW_AMAZON_SUPPORT__
                PryonLiteError err;

                AUD_LOG_D("Amazon_Model init\n");
                // Calling Destroy() will flush any unprocessed audio that has been
                // pushed and allows for calling Initialize() again if desired.
                err = PryonLiteDecoder_Destroy(&sDecoder);
                if (err != PRYON_LITE_ERROR_OK) {
                     AUD_LOG_D("[AMAZON]PryonLiteDecoder_Destroy err=%d\n", err);
                     ret = -1;
                }
#endif
            }
            break;
        default:
            break;
    }
    return ret;
}

/***********************************************************************************
** vow_model_init - Initialize SWIP with different model_type
************************************************************************************/
int vow_model_init(vow_model_t *model_ptr)
{
    int ret = 1;

    switch (model_ptr->id) {
        case VENDOR_ID_OTHERS:
#ifdef __VOW_GOOGLE_SUPPORT__
            AUD_LOG_D("Google_Model init\n");
            ret = GoogleHotwordDspInit((void *)model_ptr->data);
#endif
            break;
        case VENDOR_ID_MTK: {
#ifdef __VOW_MTK_SUPPORT__
                const char *version_info;
                VOWE_DSP_testing_init_parameters model_param;

                AUD_LOG_D("MTK_Model init\n");
                model_param.dspParsedModelData.length = model_ptr->size;
                model_param.dspParsedModelData.beginPtr = (char *)model_ptr->data;
                ret = VOWE_DSP_testing_init(&model_param);
                if (ret != vowe_ok) {
                    AUD_LOG_D("[MTK]testing_init err\n");
                    ret = -1;
                }
                ret = VOWE_DSP_testing_reset();
                if (ret != vowe_ok) {
                    AUD_LOG_D("[MTK]testing_reset err\n");
                    ret = -1;
                }
                version_info = VOWE_DSP_testing_version();
                AUD_LOG_D("[MTK]VOW lib ver %s\n", version_info);
#endif
            }
            break;
        case VENDOR_ID_AMAZON: {
#ifdef __VOW_AMAZON_SUPPORT__
                char *temp;
                // Query for the size of instance memory required by the decoder
                PryonLiteModelAttributes modelAttributes;
                PryonLiteError err;

                AUD_LOG_D("AMAZON_Model init\n");
                PryonConfig = (PryonLiteDecoderConfig)PryonLiteDecoderConfig_Default;
                PryonConfig.sizeofModel = model_ptr->size;
                PryonConfig.model = (void *)model_ptr->data;
                err = PryonLite_GetModelAttributes(PryonConfig.model,
                                                   PryonConfig.sizeofModel,
                                                   &modelAttributes);
                if (err != PRYON_LITE_ERROR_OK) {
                    AUD_LOG_D("[AMAZON]PryonLite_GetModelAttributes err=%d\n", err);
                    return -1;
                }
                temp = (char *)PryonConfig.model;
                AUD_LOG_D("[AMAZON]%d, %d, %d, %d\n", temp[0], temp[1], temp[2], temp[3]);
                AUD_LOG_D("[AMAZON]requiredDecoderMem = %d\n", modelAttributes.requiredDecoderMem);

                PryonConfig.decoderMem = pryon_lite_decoder_buf;
                if (modelAttributes.requiredDecoderMem > sizeof(pryon_lite_decoder_buf)) {
                    // handle error
                    return -1;
                }

                PryonConfig.sizeofDecoderMem = modelAttributes.requiredDecoderMem;
                PryonConfig.detectThreshold = 400; //default: 400
                PryonConfig.resultCallback = detectionCallback; // register detection handler
                PryonConfig.vadCallback = vadCallback; // register VAD handler
                PryonConfig.useVad = 1;  // enable voice activity detector  //DATO must 1, otherwise initialize err will be 2

                err = PryonLiteDecoder_Initialize(&PryonConfig, &sessionInfo, &sDecoder);
                AUD_LOG_D("[AMAZON]sessionInfo.samplesPerFrame=%d\n", sessionInfo.samplesPerFrame);
                if (err != PRYON_LITE_ERROR_OK) {
                    AUD_LOG_D("[AMAZON]PryonLiteDecoder_Initialize err=%d\n", err);
                    return -1;
                }

                // initialize variable
                pryonLite_recog_ok = false;
                pryonLite_confidence_lv = 0;
                pryonLite_vadState = -1;
#endif
            }
            break;
        default:
            AUD_LOG_D("Not Support\n");
            break;
    }
    return ret;
}

/***********************************************************************************
** vow_keyword_Recognize - use SWIP to recognize keyword with different model_type
************************************************************************************/
int vow_keyword_recognize(void *sample_input,
                          int num_sample,
                          int *ret_info,
                          int *recog_ok_id,
                          int *confidence_lv)
{
    int ret = -1;
    int I;

    for (I = 0; I < MAX_VOW_SPEAKER_MODEL; I++) {
        if (vowserv.vow_model_event[I].enable == true) {
#ifdef CHECK_RECOG_TIME
            /* start to check SWIP operating time, if > 15ms,
            then recreate vow task */
            xTimerStart(xVowTimer, 0);
#endif  // #ifdef CHECK_RECOG_TIME
#ifdef RECOVERY_TEST
            if (bug_test_cnt > 100) {
                bug_test_cnt = 0;
                vTaskDelay((portTickType) 25 / portTICK_RATE_MS);
            } else {
                bug_test_cnt++;
            }
#endif  // #ifdef RECOVERY_TEST

            *recog_ok_id = 0;
            switch (vowserv.vow_model_event[I].id) {
                case VENDOR_ID_OTHERS:
#ifdef __VOW_GOOGLE_SUPPORT__
                    ret = GoogleHotwordDspProcess(sample_input,
                                                  num_sample,
                                                  ret_info);
                    if (ret == 1) {
                        // Reset in order to resume detection.
                        GoogleHotwordDspReset();
                        *recog_ok_id = vowserv.vow_model_event[I].id;
                        AUD_LOG_D("OTHERS detect!\n");
                        // if recognize ok, then leave
                        break;
                    }
#endif
                    break;
                case VENDOR_ID_MTK: {
#ifdef __VOW_MTK_SUPPORT__
                        RecognitionResults results;
                        (void)num_sample;
                        // 0: not thing, 1: pass, 2: fail(no-sound)
                        results = VOWE_DSP_testing_recognize((short *)sample_input, 160);
                        if (results.status == vowe_accept) {
                            ret = 1;
                            *recog_ok_id = vowserv.vow_model_event[I].id;
                            *confidence_lv = (int)(results.score / 65536);
                            AUD_LOG_D("results.score = %lf, confidence_lv = %d\n", (double)results.score, *confidence_lv);
                            AUD_LOG_D("MTK detect!\n");
                            // if recognize ok, then leave
                            break;
                        } else if (results.status == vowe_hold_on) {
                            ret = 0;
                        } else if (results.status == vowe_bad) {
                            /* need to check ontesting input size, must be 160 sample */
                            AUD_ASSERT(0);
                        }
#endif
                    }
                    break;
                case VENDOR_ID_AMAZON: {
#ifdef __VOW_AMAZON_SUPPORT__
                        // pass samples to decoder
                        PryonLiteError err;

                        err = PryonLiteDecoder_PushAudioSamples(sDecoder,
                                                                sample_input,
                                                                sessionInfo.samplesPerFrame);
                        if (err != PRYON_LITE_ERROR_OK) {
                            // handle error
                            AUD_LOG_D("[AMAZON]recog err=%d\n", err);
                            return -1;
                        }
                        if (pryonLite_recog_ok == true) {
                            pryonLite_recog_ok = false;
                            ret = 1;
                            *recog_ok_id = vowserv.vow_model_event[I].id;
                            *confidence_lv = pryonLite_confidence_lv / 10;
                            AUD_LOG_D("AMAZON detect!\n");
                            // if recognize ok, then leave
                            break;
                        } else {
                            ret = 0;
                            if (pryonLite_vadState == 0) {
                                ret = 2;  // Become no sound, back to phase1
                                pryonLite_vadState = -1;
                                AUD_LOG_D("no sound\n");
                            }
                        }
#endif
                    }
                    break;
                default:
                    AUD_LOG_D("Not Support\n");
                    break;
            }
            //SWVAD handling
            if (ret == 0) {
                // call MTK SW VAD, check if in "no-sound" place
                vowserv.noKeywordCount++;
                if (returnToPhase1BySwVad((short *)sample_input,
                                          vowserv.noKeywordCount)) {
                    ret = 2;  // Become no sound, back to phase1
                }
            }
            if ((ret == 1) || (ret == 2)) {
                resetSwVadPara();
                vowserv.noKeywordCount = 0;
            }
            if (ret == 1) {
                // one is recog pass, then break
                break;
            }

#ifdef CHECK_RECOG_TIME
            /* stop checking SWIP operating time.
            if it doesn't cause timer handler,
            then means operating time is lower than 15ms */
            xTimerStop(xVowTimer, 0);
#endif  // #ifdef CHECK_RECOG_TIME
        }
    }
    return ret;
}

/***********************************************************************************
** vow_resume_phase1 - handle new control flow when P2 to P1
************************************************************************************/
static void vow_resume_phase1_setting(void)
{
    AUD_LOG_V("resume_phase1\n");
    mask_irq(MAD_FIFO_IRQn);
    vowserv.read_idx = VOW_LENGTH_VOICE + 1;
    vowserv.write_idx = 0;
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
    vowserv.vowTimeFlag &= ~(BIT_VOICE_R_IDX_UPDATE);
#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
//#ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
//    vowserv.seamless_buf_w_ptr = 0;
//#endif  // #ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
#ifdef BLISRC_USE
    vowserv.blisrc_r_idx = 0;
    vowserv.blisrc_w_idx = 0;
#endif  // #ifdef BLISRC_USE
    vowserv.data_length = 0;
    unmask_irq(MAD_FIFO_IRQn);

#if PMIC_6337_SUPPORT
    // already turn on, then clear periodic On/Off flag
    if ((vowserv.open_periodic_flag == true) && (vowserv.periodic_need_enable == true)) {
        vowserv.open_periodic_flag = false;
    }
    vow_datairq_reset();

#endif  // #if PMIC_6337_SUPPORT
//    vowserv.can_unlock_scp_flag = true;  // flag means can back to phase1
}

/***********************************************************************************
** task_vow_working - main task content
************************************************************************************/
static uint8_t task_vow_working(
    struct AudioTask *this,
    struct ipi_msg_t *ipi_msg)
{
    int ret = 0;
    int RetInfo = 0;
    int recog_ok_id = 0;
    int confidence_lv = 0;
    int src_fifo_r_idx;  // this is index for catching from FIFO buffer
    // int data_amount;
    ipi_msg_t ipi_msg_send;
    // uint8_t queue_idx = 0;
    uint8_t LoopAg = 0;
    vow_pcmdump_info_t pcmdump_info;
#ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
    unsigned int Offset_Size;
    //int delay_count = 0;
    unsigned int Rdy2Send_Offset = 0;
    unsigned int Rdy2Send_Size = 0;
    short *seamless_buf_ptr;
    short *seamless_src_ptr;
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
    short *seamless_src_ptr_R;
    short *seamless_buf_ptr_R;
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
#else  // #ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
    bool pcmdump_start_remind_kernel = false;
    unsigned int pingpong_offset;
#endif  // #ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
#if TIMES_DEBUG
    unsigned int Temp_recog_time = 0;
    unsigned int Temp_task_time = 0;
#endif  // #if TIMES_DEBUG
#ifdef BLISRC_USE
    bool bli_working_loop_again = false;
    bool double_bli_output = false;
    uint32_t bli_available_in_size;
    uint32_t bli_available_ou_size;
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
    uint32_t bli_available_in_size_R;
    uint32_t bli_available_ou_size_R;
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
#endif  // #ifdef BLISRC_USE
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
    short *ec_in_buf_ptr;
    short *ec_out_buf_ptr;
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
    short *ec_out_buf_ptr_R;
#endif  // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
#ifdef VOW_ECHO_SW_SRC
    uint32_t bli_echo_available_in_size;
    uint32_t bli_echo_available_ou_size;
#endif // #ifdef VOW_ECHO_SW_SRC
#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
    short *recog_buf_ptr;
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
    short *recog_buf_ptr_R;
#endif  // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
    bool bypass_flag = false;
    vow_ipi_combined_info_t vow_ipi_info;


    if ((vowserv.vow_status != VOW_STATUS_IDLE)
            && (vowserv.data_length >= VOW_SAMPLE_NUM_IRQ)) {

        if (vowserv.first_irq_flag) {
            vowserv.first_irq_flag = false;
            vow_pmic_low_power_control(&vowserv, VOW_PMIC_LOW_POWER_ENTRY);
        }
#if TIMES_DEBUG
        start_task_time = *DWT_CYCCNT_1;
#endif  // #if TIMES_DEBUG
        mask_irq(MAD_FIFO_IRQn);
        if (vowserv.read_idx >= VOW_LENGTH_VOICE) {
            vowserv.read_idx = 0;
        }
        vowserv.frm_cnt++;
        // [head]: Determine Blisrc output two frame or not
#ifdef BLISRC_USE
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        if ((vowserv.bargeInStatus == BARGEIN_WORKING1) || (vowserv.bargeInStatus == BARGEIN_WORKING2)) {
            double_bli_output = is_accurate_src_additional_frame_accumulated((uint32_t *)&vowserv.frm_cnt);
        } else {
            double_bli_output = is_normal_src_additional_frame_accumulated((uint32_t *)&vowserv.frm_cnt);
        }
#else // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        double_bli_output = is_normal_src_additional_frame_accumulated((uint32_t *)&vowserv.frm_cnt);
#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        bli_available_in_size = BLISRC_INPUT_BUFFER_SIZE_IN_HALF_WORD * sizeof(short);
        bli_available_ou_size = BLISRC_OUTPUT_BUFFER_SIZE_IN_HALF_WORD * sizeof(short);
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
        bli_available_in_size_R = BLISRC_INPUT_BUFFER_SIZE_IN_HALF_WORD * sizeof(short);
        bli_available_ou_size_R = BLISRC_OUTPUT_BUFFER_SIZE_IN_HALF_WORD * sizeof(short);
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
        if (double_bli_output || double_bli_output_next) {
            if ((BLISRC_OUTPUT_TEMP_BUFFER - vowserv.blisrc_w_idx) < BLISRC_OUTPUT_TEMP_BUFFER) {
                double_bli_output_next = true;
                AUD_LOG_V("Space of output buf(Wptr to tail) is not enough, output twice next\n");
            } else {
                bli_available_ou_size <<= 1;
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                bli_available_ou_size_R <<= 1;
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                bli_working_loop_again = true;
                double_bli_output_next = false;
                AUD_LOG_V("bli_available_ou_size: %d, vowserv.frm_cnt: %d\n",
                          bli_available_ou_size, vowserv.frm_cnt);
            }
        }
        // [tail]: Determine Blisrc output two frame or not
#endif // #ifdef BLISRC_USE
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
#ifdef BLISRC_USE
        // [head]: MIC voice data delay because echo data must to be ahead.
        if ((vowserv.vowTimeFlag & (BIT_VOICE_R_IDX_UPDATE | BIT_VOW_PERIOD)) == BIT_VOW_PERIOD) {
            src_fifo_r_idx = vow_bargein_buffer_data(vowserv.write_idx, (VOW_SAMPLE_NUM_IRQ * VOICE_DELAY_FRM),
                             VOW_LENGTH_VOICE);
            vowserv.read_idx = src_fifo_r_idx;
            vowserv.vowTimeFlag |= (BIT_VOICE_R_IDX_UPDATE);
            vowserv.data_length = VOW_SAMPLE_NUM_IRQ * (VOICE_DELAY_FRM);
            //AUD_LOG_D("[BargeIn] voice buffer delay %d frame! Before task read, voice len: %d samples\n",
            //          VOICE_DELAY_FRM, vowserv.data_length);
            AUD_LOG_D("[BargeIn] delay %d frame,len: %d samples\n",
                      VOICE_DELAY_FRM, vowserv.data_length);
            AUD_LOG_V("[BargeIn] After modify voice idx, read_idx:%d, write_idx:%d\n",
                      vowserv.read_idx, vowserv.write_idx);
        } else {
            src_fifo_r_idx = vowserv.read_idx;
        }
        // [tail]: MIC voice data delay because echo data must to be ahead.

        unmask_irq(MAD_FIFO_IRQn);

        // [head]: Excute Blisrc process, it may be accurate blisrc or normal one
        if ((vowserv.bargeInStatus == BARGEIN_WORKING1) || (vowserv.bargeInStatus == BARGEIN_WORKING2)) {
#if TIMES_DEBUG
            //CPU_RESET_CYCLECOUNTER_1();
            start_blisrc_time = *DWT_CYCCNT_1;
#endif
            blisrc_accurate_vow_process(blisrc_accurate_handle,
                                        (int16_t *)&vowserv_buf_voice[src_fifo_r_idx],
                                        &bli_available_in_size,
                                        (int16_t *)&vowserv_buf_blic_out[vowserv.blisrc_w_idx],
                                        &bli_available_ou_size);
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
            blisrc_accurate_vow_process(blisrc_accurate_handle_R,
                                        (int16_t *)&vowserv_buf_voice_R[src_fifo_r_idx],
                                        &bli_available_in_size_R,
                                        (int16_t *)&vowserv_buf_blic_out_R[vowserv.blisrc_w_idx],
                                        &bli_available_ou_size_R);
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
#if TIMES_DEBUG
            end_blisrc_time = *DWT_CYCCNT_1;
            AUD_LOG_V("[Bargein] acc_src: out_size end start: %d\t%d\t%d \n",
                      bli_available_ou_size, end_blisrc_time, start_blisrc_time);
#endif
        } else {
#if TIMES_DEBUG
//            CPU_RESET_CYCLECOUNTER_1();
            start_blisrc_time = *DWT_CYCCNT_1;
#endif
            blisrc_vow_process(blisrc_handle,
                               (int16_t *)&vowserv_buf_voice[src_fifo_r_idx],
                               &bli_available_in_size,
                               (int16_t *)&vowserv_buf_blic_out[vowserv.blisrc_w_idx],
                               &bli_available_ou_size);
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
            blisrc_vow_process(blisrc_handle_R,
                               (int16_t *)&vowserv_buf_voice_R[src_fifo_r_idx],
                               &bli_available_in_size_R,
                               (int16_t *)&vowserv_buf_blic_out_R[vowserv.blisrc_w_idx],
                               &bli_available_ou_size_R);
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
#if TIMES_DEBUG
            end_blisrc_time = *DWT_CYCCNT_1;
            AUD_LOG_V("[Bargein] nor_src: out_size end start: %d\t%d\t%d \n",
                      bli_available_ou_size, end_blisrc_time, start_blisrc_time);
#endif
        }
#endif // #ifdef BLISRC_USE
#else // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        // Get current read_index
        src_fifo_r_idx = vowserv.read_idx;
        unmask_irq(MAD_FIFO_IRQn);

#ifdef BLISRC_USE
#if TIMES_DEBUG
        //CPU_RESET_CYCLECOUNTER_1();
        start_blisrc_time = *DWT_CYCCNT_1;
#endif
        blisrc_vow_process(blisrc_handle,
                           (int16_t *)&vowserv_buf_voice[src_fifo_r_idx],
                           &bli_available_in_size,
                           (int16_t *)&vowserv_buf_blic_out[vowserv.blisrc_w_idx],
                           &bli_available_ou_size);
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
        blisrc_vow_process(blisrc_handle,
                           (int16_t *)&vowserv_buf_voice_R[src_fifo_r_idx],
                           &bli_available_in_size_R,
                           (int16_t *)&vowserv_buf_blic_out_R[vowserv.blisrc_w_idx],
                           &bli_available_ou_size_R);
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
#if TIMES_DEBUG
        end_blisrc_time = *DWT_CYCCNT_1;
        AUD_LOG_V("[Bargein] nor_src: out_size end start: %d\t%d\t%d \n",
                  bli_available_ou_size, end_blisrc_time, start_blisrc_time);
#endif
#endif // #ifdef BLISRC_USE

#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        // [tail]: Excute Blisrc process, it may be accurate blisrc or normal one

#ifdef BLISRC_USE
WORKING_LOOP_AGAIN:

        /* reset ipi value */
        vow_ipi_info.ipi_type_flag = 0;

        // Do AEC/recognition twice if BLISRC output 2 frames
        /* Update blisrc output buffer index, Use BLISRC out second buffer when working_loop_again is true */
        vowserv.blisrc_w_idx += BLISRC_OUTPUT_BUFFER_SIZE_IN_HALF_WORD;
        if (vowserv.blisrc_w_idx >= BLISRC_OUTPUT_TEMP_BUFFER) {
            vowserv.blisrc_w_idx = 0;
        }
        AUD_LOG_V("[BargeIn] Read Bli out: %d, Write Bli out: %d\n",
                  vowserv.blisrc_r_idx, vowserv.blisrc_w_idx);

        recog_buf_ptr = &vowserv_buf_blic_out[vowserv.blisrc_r_idx];
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
        recog_buf_ptr_R = &vowserv_buf_blic_out_R[vowserv.blisrc_r_idx];
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT

        AUD_ASSERT(vowserv.blisrc_r_idx < BLISRC_OUTPUT_TEMP_BUFFER);
#else  // #ifdef BLISRC_USE
        recog_buf_ptr = &vowserv_buf_voice[src_fifo_r_idx];
        AUD_ASSERT(src_fifo_r_idx < VOW_LENGTH_VOICE);
#endif  // #ifdef BLISRC_USE

#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        // [head] : AEC process
        if (((vowserv.echoFirstRamReadDone == ECHO_FIRST_READ_DONE) || (vowserv.echoFirstRamReadDone == ECHO_FIRST_READ_TASK)) &&
            (vowserv.vowTimeFlag & BIT_BARGEIN_PERIOD)) {
            AUD_LOG_V("[BargeIn] Barge In working\n");
#ifdef VOW_ECHO_SW_SRC
            bli_echo_available_in_size = BLISRC_ECHO_INPUT_BUFFER_SIZE_IN_HALF_WORD * sizeof(short);
            bli_echo_available_ou_size = BLISRC_ECHO_OUTPUT_BUFFER_SIZE_IN_HALF_WORD * sizeof(short);
            AUD_LOG_V("Echo Bli out r: %d, Echo Bli out w: %d\n",
                      vowserv.echo_r_idx, vowserv.echo_w_idx);
            AUD_LOG_V("Echo Bli in r: %d, Echo Bli in w: %d\n",
                      vowserv.echo_nonsrc_r_idx, vowserv.echo_nonsrc_w_idx);
            if (vowserv.echo_nonsrc_data_len < ECHO_REF_NONSRC_FRM_LEN) {
                AUD_LOG_E("Echo nonsrc udf: %d\n", vowserv.echo_nonsrc_data_len);
            }
#if TIMES_DEBUG
//            CPU_RESET_CYCLECOUNTER_1();
            start_blisrc_time = *DWT_CYCCNT_1;
#endif
            blisrc_echo_ref_vow_process(blisrc_echo_ref_handle,
                                        (int16_t *)&vowserv_buf_echo_ref_nonsrc[vowserv.echo_nonsrc_r_idx],
                                        &bli_echo_available_in_size,
                                        (int16_t *)&vowserv_buf_echo_ref_in[vowserv.echo_w_idx],
                                        &bli_echo_available_ou_size);
            AUD_LOG_V("echo src: in consumed %04d  out %04d \n",
                      bli_echo_available_in_size, bli_echo_available_ou_size);
#if TIMES_DEBUG
            end_blisrc_time = *DWT_CYCCNT_1;
            AUD_LOG_D("echo: out_size end start: %d\t%d \n",
                      end_blisrc_time, start_blisrc_time);
#endif
            vowserv.echo_w_idx += BLISRC_ECHO_OUTPUT_BUFFER_SIZE_IN_HALF_WORD;
            vowserv.echo_data_len += BLISRC_ECHO_OUTPUT_BUFFER_SIZE_IN_HALF_WORD;
            if (vowserv.echo_w_idx >= ECHO_REF_BUF_LEN) {
                vowserv.echo_w_idx = 0;
            }

            vowserv.echo_nonsrc_r_idx += BLISRC_ECHO_INPUT_BUFFER_SIZE_IN_HALF_WORD;
            if (vowserv.echo_nonsrc_r_idx >= ECHO_REF_NONSRC_BUF_LEN) {
                vowserv.echo_nonsrc_r_idx = 0;
            }
            vowserv.echo_nonsrc_data_len -= ECHO_REF_NONSRC_FRM_LEN;

#endif // #ifdef VOW_ECHO_SW_SRC
            if (vowserv.echoFirstRamReadDone == ECHO_FIRST_READ_DONE) {
                AUD_LOG_D("[BargeIn] Delay echo ref, echo len: %d\n", vowserv.echo_data_len);
                mask_irq(AUDIO_IRQn);
                if (vowserv.echo_w_idx >= (ECHO_REF_FRM_LEN * ECHO_DELAY_FRM)) {
                    vowserv.echo_r_idx = vowserv.echo_w_idx - (ECHO_REF_FRM_LEN * ECHO_DELAY_FRM);
                } else {
                    vowserv.echo_r_idx = ECHO_REF_BUF_LEN - (ECHO_REF_FRM_LEN * ECHO_DELAY_FRM) + vowserv.echo_w_idx;
                }
                vowserv.echoFirstRamReadDone = ECHO_FIRST_READ_TASK;
                vowserv.echo_data_len = (ECHO_REF_FRM_LEN * ECHO_DELAY_FRM);
                unmask_irq(AUDIO_IRQn);
                // send IPI to kernel to inform delay parameter in of EC lib
                if ((vowserv.bargeInStatus == BARGEIN_WORKING1) && dump_pcm_flag) {
                    AUD_LOG_D("[BargeIn] dump info send\n");
                    /* IPIMSG_VOW_BARGEIN_DUMP_INFO */
                    vow_ipi_info.ipi_type_flag |= BARGEIN_DUMP_INFO_IDX_MASK;
                    vow_ipi_info.dump_frm_cnt = vowserv.bargein_dump_frm;
                    vow_ipi_info.voice_sample_delay = vowserv.echo_delay_samples;

                    vowserv.bargeInStatus = BARGEIN_WORKING2;
                }
            }
            if ((vowserv.echo_data_len < ECHO_REF_FRM_LEN) && (vowserv.echoFirstRamReadDone >= ECHO_FIRST_READ_DONE)) {
                AUD_LOG_E("[BargeIn] echo ref buffer underflow, echo_data_len: %d\n", vowserv.echo_data_len);
            }
            ec_in_buf_ptr = &vowserv_buf_echo_ref_in[vowserv.echo_r_idx];
            vowserv.echo_r_idx += ECHO_REF_FRM_LEN;
            if (vowserv.echo_r_idx >= ECHO_REF_BUF_LEN) {
                vowserv.echo_r_idx = 0;
            }
            mask_irq(AUDIO_IRQn);
            vowserv.echo_data_len -= ECHO_REF_FRM_LEN;
            unmask_irq(AUDIO_IRQn);
            ec_out_buf_ptr = vowserv_buf_echo_ref_out;
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
            ec_out_buf_ptr_R = vowserv_buf_echo_ref_out_R;
#endif //#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
            // [head]: Bargein dump when there is music playback
            if (vowserv.PcmDumpFlag) {
                vowserv.bargein_dump_frm++;
                vow_pcm_dump_process(DUMP_VOW_VOICE, (uint32_t)recog_buf_ptr, (VOW_SAMPLE_NUM_FRAME << 1), &vow_ipi_info);
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                vow_pcm_dump_process(DUMP_VOW_VOICE_R, (uint32_t)recog_buf_ptr_R, (VOW_SAMPLE_NUM_FRAME << 1), &vow_ipi_info);
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                vow_pcm_dump_process(DUMP_ECHO_REF, (uint32_t)ec_in_buf_ptr, ECHO_REF_FRM_BYTE_LEN, &vow_ipi_info);
            }
            // [tail]: Bargein dump when there is music playback

#if TIMES_DEBUG
            start_ec_time = *DWT_CYCCNT_1;
#endif  // #if (TIMES_DEBUG)

                AEC_writeMicInput(EC_MIC_L_IDX, (void *) recog_buf_ptr, ECHO_REF_FRM_BYTE_LEN);
                AEC_writeRefInput(0, (void *) ec_in_buf_ptr, ECHO_REF_FRM_BYTE_LEN);
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                AEC_writeMicInput(EC_MIC_R_IDX, (void *) recog_buf_ptr_R, ECHO_REF_FRM_BYTE_LEN);
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                AEC_process();
                AEC_readMicOutput(EC_MIC_L_IDX, (void *) ec_out_buf_ptr, out_byte_ptr);
                if (*out_byte_ptr != ECHO_REF_FRM_BYTE_LEN)
                    AUD_LOG_E("[BargeIn] Fail, AEC_process output num: %d \n", *out_byte_ptr);
                recog_buf_ptr = ec_out_buf_ptr;
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                AEC_readMicOutput(EC_MIC_R_IDX, (void *) ec_out_buf_ptr_R, out_byte_ptr_R);
                if (*out_byte_ptr_R != ECHO_REF_FRM_BYTE_LEN)
                    AUD_LOG_E("[BargeIn] Fail, AEC_process output num: %d \n", *out_byte_ptr_R);
                recog_buf_ptr_R = ec_out_buf_ptr_R;
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT

#if TIMES_DEBUG
            end_ec_time = *DWT_CYCCNT_1;
            AUD_LOG_V("[BargeIn] EC end start: %d\t%d \n", end_ec_time, start_ec_time);
#endif  // #if (TIMES_DEBUG)

            // [tail] : AEC process
            if (vowserv.PcmDumpFlag) {
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                vow_pcm_dump_process(DUMP_VOW_RECOG_R, (uint32_t)recog_buf_ptr_R, (VOW_SAMPLE_NUM_FRAME << 1), &vow_ipi_info);
#endif  // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                vow_pcm_dump_process(DUMP_VOW_RECOG, (uint32_t)recog_buf_ptr, (VOW_SAMPLE_NUM_FRAME << 1), &vow_ipi_info);
            }
            // [head]: Bargein dump when there is no music playback, fill DC value
        } else if ((vowserv.echoFirstRamReadDone == ECHO_FIRST_READ_RESYNC) ||
                   (vowserv.PcmDumpFlag && !(vowserv.vowTimeFlag & BIT_BARGEIN_PERIOD))) {
            ec_in_buf_ptr = &vowserv_buf_echo_ref_sil[0];
            vowserv.bargein_dump_frm++;
            vow_pcm_dump_process(DUMP_VOW_VOICE, (uint32_t)recog_buf_ptr, (VOW_SAMPLE_NUM_FRAME << 1), &vow_ipi_info);
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
            vow_pcm_dump_process(DUMP_VOW_VOICE_R, (uint32_t)recog_buf_ptr_R, (VOW_SAMPLE_NUM_FRAME << 1), &vow_ipi_info);
            vow_pcm_dump_process(DUMP_VOW_RECOG_R, (uint32_t)recog_buf_ptr_R, (VOW_SAMPLE_NUM_FRAME << 1), &vow_ipi_info);
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
            vow_pcm_dump_process(DUMP_VOW_RECOG, (uint32_t)recog_buf_ptr, (VOW_SAMPLE_NUM_FRAME << 1), &vow_ipi_info);
            vow_pcm_dump_process(DUMP_ECHO_REF, (uint32_t)ec_in_buf_ptr, ECHO_REF_FRM_BYTE_LEN, &vow_ipi_info);
        }
        // [tail]: Bargein dump when there is no music playback, fill DC value

#else  // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT

        // No bargein case
        if (vowserv.PcmDumpFlag) {
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
            vow_pcm_dump_process(DUMP_VOW_RECOG_R, (uint32_t)recog_buf_ptr_R, (VOW_SAMPLE_NUM_FRAME << 1), &vow_ipi_info);
#endif  // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
            vow_pcm_dump_process(DUMP_VOW_RECOG, (uint32_t)recog_buf_ptr, (VOW_SAMPLE_NUM_FRAME << 1), &vow_ipi_info);
        }
        // [tail]: Bargein dump when there is no music playback, fill DC value
#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT

#ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
        // Data source of seamless record buffer is "SCP recognition data"
        seamless_buf_ptr = vowserv_buf_seamless_rec;
        seamless_src_ptr = recog_buf_ptr;

        // Copy Data to Seamless record buffer
        memcpy(&seamless_buf_ptr[vowserv.seamless_buf_w_ptr], seamless_src_ptr,
               VOW_SAMPLE_NUM_FRAME * sizeof(short));
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
        // Data source of seamless record buffer is "SCP recognition data"
        seamless_buf_ptr_R = vowserv_buf_seamless_rec_R;
        seamless_src_ptr_R = recog_buf_ptr_R;

        // Copy Data to Seamless record buffer
        memcpy(&seamless_buf_ptr_R[vowserv.seamless_buf_w_ptr], seamless_src_ptr_R,
               VOW_SAMPLE_NUM_FRAME * sizeof(short));
#endif// #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
        // update seamless record index, let the last 10msec can combine into 64kB buffer
        vowserv.seamless_buf_w_ptr += VOW_SAMPLE_NUM_FRAME;
        if (vowserv.seamless_buf_w_ptr >= SEAMLESS_RECORD_BUF_SIZE) {
            vowserv.seamless_buf_w_ptr = 0;
        }
#endif // #ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT

        if (vowserv.record_flag) {
#ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
            Rdy2Send_Size = 0;
            Rdy2Send_Offset = 0;
            Offset_Size = 0;
            if (vowserv.seamless_record == false) {
                // upload pcm dump data into Kernel
                    ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr,
                                              (uint32_t)seamless_src_ptr,
                                              (VOW_SAMPLE_NUM_FRAME << 1));
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                    ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr + VOW_VOICEDATA_SIZE,
                                              (uint32_t)seamless_src_ptr_R,
                                              (VOW_SAMPLE_NUM_FRAME << 1));
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                if (ret != DMA_RESULT_DONE) {
                    AUD_LOG_E("%s(), [err]DMA fail:%x\n", __func__, ret);
                }

                Rdy2Send_Size = (VOW_SAMPLE_NUM_FRAME << 1);
                Rdy2Send_Offset = 0;

            } else if ((vowserv.seamless_record == true) && (delay_count >= SEAMLESS_DELAY)) {
                vowserv.seamless_record = false;
                // Transfer the previous voice data in seamless record
                delay_count = 0;
                Rdy2Send_Offset = 0;
                Offset_Size = (SEAMLESS_RECORD_LENGTH - vowserv.seamless_buf_w_ptr) << 1;
                // DMA from vowserv.seamless_buf_w_ptr to bottom
                ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr,
                                          (uint32_t)&seamless_buf_ptr[vowserv.seamless_buf_w_ptr],
                                          Offset_Size);
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr + VOW_VOICEDATA_SIZE,
                                          (uint32_t)&seamless_buf_ptr_R[vowserv.seamless_buf_w_ptr],
                                          Offset_Size);
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                if (ret != DMA_RESULT_DONE) {
                    AUD_LOG_E("%s(), [err]DMA bottom fail:%x\n", __func__, ret);
                }
                // DMA from top to vowserv.seamless_buf_w_ptr
                if (vowserv.seamless_buf_w_ptr > 0) {
                    ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr + Offset_Size,
                                              (uint32_t)&seamless_buf_ptr[0],
                                              (vowserv.seamless_buf_w_ptr << 1));
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                    ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr +
                                              VOW_VOICEDATA_SIZE + Offset_Size,
                                              (uint32_t)&seamless_buf_ptr_R[0],
                                              (vowserv.seamless_buf_w_ptr << 1));
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
                    if (ret != DMA_RESULT_DONE) {
                        AUD_LOG_E("%s(), [err]DMA top fail:%x\n", __func__, ret);
                    }
                }
                Rdy2Send_Size = (SEAMLESS_RECORD_LENGTH << 1);

            } else {
                AUD_LOG_V("delay_count=%d\n", delay_count);
                delay_count++;
                Rdy2Send_Size = 0;
                Rdy2Send_Offset = 0;
            }
            if (Rdy2Send_Size > 0) {
                if (Rdy2Send_Size > 0xF000) {
                    if (seamless_test >= SEAMLESS_SEND_LARGE) {
                        AUD_LOG_E("[err]two continue 64KB buffer, need check flow\n");
                        seamless_test = SEAMLESS_SEND_CONTINUE_LARGE;
                    } else {
                        seamless_test = SEAMLESS_SEND_LARGE;
                    }
                } else {
                    seamless_test = SEAMLESS_SEND_SMALL;
                }
                if (Rdy2Send_Size > (SEAMLESS_RECORD_LENGTH << 1)) {
                    AUD_LOG_E("[err]Rdy2Send_Size > 64kB, need check flow: w=%d\n", vowserv.seamless_buf_w_ptr);
                    AUD_ASSERT(0);
                }
                if (seamless_test != SEAMLESS_SEND_CONTINUE_LARGE) {
                    pcmdump_info.dump_buf_offset = Rdy2Send_Offset;  // DMA offset
                    pcmdump_info.dump_size = Rdy2Send_Size;  // DMA size (Byte)

                    /* IPIMSG_VOW_BARGEIN_DUMP_INFO */
                    vow_ipi_info.ipi_type_flag |= DEBUG_DUMP_IDX_MASK;
                    vow_ipi_info.voice_buf_offset = pcmdump_info.dump_buf_offset;
                    vow_ipi_info.voice_length = pcmdump_info.dump_size;
                }
            }
#else  // #ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
            ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr +
                                      (vowserv.pcmdump_cnt * (VOW_SAMPLE_NUM_FRAME << 1)),
                                      (uint32_t)recog_buf_ptr,
                                      (VOW_SAMPLE_NUM_FRAME << 1));
            if (ret != DMA_RESULT_DONE) {
                AUD_LOG_E("%s(), [err]DMA fail:%x\n", __func__, ret);
            }

            vowserv.pcmdump_cnt++;
            pcmdump_start_remind_kernel = false;
            // if pcmdump_cnt is half or full, then send IPI to kernel
            if ((vowserv.pcmdump_cnt == (PCMDUMP_BUF_CNT >> 1))
                    || (vowserv.pcmdump_cnt == PCMDUMP_BUF_CNT)) {
                pingpong_offset = (vowserv.pcmdump_cnt - (PCMDUMP_BUF_CNT >> 1)) * (VOW_SAMPLE_NUM_FRAME << 1);
                pcmdump_start_remind_kernel = true;
            }

            if (vowserv.pcmdump_cnt == PCMDUMP_BUF_CNT) {
                vowserv.pcmdump_cnt = 0;
            }

            if (pcmdump_start_remind_kernel == true) {
                pcmdump_info.dump_buf_offset = pingpong_offset;  // DMA offset
                pcmdump_info.dump_size = (PCMDUMP_BUF_CNT >> 1) * (VOW_SAMPLE_NUM_FRAME << 1);  // DMA size (Byte)
                AUD_LOG_V("offset: 0x%x, leng: 0x%x\n\r", pcmdump_info.dump_buf_offset, pcmdump_info.dump_size);

                /* IPIMSG_VOW_DATAREADY */
                vow_ipi_info.ipi_type_flag |= DEBUG_DUMP_IDX_MASK;
                vow_ipi_info.voice_buf_offset = pcmdump_info.dump_buf_offset;
                vow_ipi_info.voice_length = pcmdump_info.dump_size;
            }
#endif  // #ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
        } else {
#ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
            seamless_test = SEAMLESS_SEND_IDLE;
#endif  // #ifdef CFG_MTK_VOW_SEAMLESS_SUPPORT
        }

#if VOW_HAS_SWIP

#if TIMES_DEBUG
        start_recog_time = *DWT_CYCCNT_1;
#endif  // #if TIMES_DEBUG
        ret = VOW_RESULT_NOTHING;
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
        if (vowserv.is_headset) {
            // use headset mic(recog_buf_ptr) to recognize, MONO case
            ret = vow_keyword_recognize((void *)recog_buf_ptr, VOW_SAMPLE_NUM_FRAME, &RetInfo, &recog_ok_id, &confidence_lv);
        } else {
            // use second mic(recog_buf_ptr_R) to recognize, set by HAL
            ret = vow_keyword_recognize((void *)recog_buf_ptr_R, VOW_SAMPLE_NUM_FRAME, &RetInfo, &recog_ok_id, &confidence_lv);
        }
#else
        // use second mic(recog_buf_ptr) to recognize, set by HAL.
        ret = vow_keyword_recognize((void *)recog_buf_ptr, VOW_SAMPLE_NUM_FRAME, &RetInfo, &recog_ok_id, &confidence_lv);
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT

#if TIMES_DEBUG
        end_recog_time = *DWT_CYCCNT_1;
#endif  // #if TIMES_DEBUG
        if (ret == VOW_RESULT_PASS) {
            AUD_LOG_E("Recog Ok: %x, id:%d\n", RetInfo, recog_ok_id);
            /* reset VOW IP status */
#ifdef __VOW_MTK_SUPPORT__
            VOWE_DSP_testing_reset();
#endif
            pre_recognize_ok_cycle = recognize_ok_cycle;
            recognize_ok_cycle = timer_get_global_timer_tick();
            bypass_flag = false;
            if ((recognize_ok_cycle - pre_recognize_ok_cycle) < 12987012) {
                AUD_LOG_E("bypass this RECOGNIZE_OK\n");
                recognize_ok_cycle = pre_recognize_ok_cycle;
                bypass_flag = true;
            }
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
            if ((vowserv.PcmDumpFlag == false) &&
                (vowserv.record_flag == false) &&
                (vowserv.debug_P2_flag == false) &&
                !(vowserv.vowTimeFlag & BIT_BARGEIN_PERIOD)) {
#else // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
            if ((vowserv.PcmDumpFlag == false) &&
                (vowserv.record_flag == false) &&
                (vowserv.debug_P2_flag == false)) {
#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT

#if !defined(CFG_MTK_VOW_SEAMLESS_SUPPORT)
                vow_pmic_low_power_control(&vowserv, VOW_PMIC_LOW_POWER_EXIT);
                vowserv.first_irq_flag = true;
#if PMIC_6337_SUPPORT
                vow_lock_vow_event();
#endif  // #if PMIC_6337_SUPPORT
                vow_pmic_phase2Tophase1(&vowserv);
#if PMIC_6337_SUPPORT
                vow_unlock_vow_event();
#endif  // #if PMIC_6337_SUPPORT
                vow_resume_phase1_setting();
#endif  // #if !defined(CFG_MTK_VOW_SEAMLESS_SUPPORT)
            }
            if (!bypass_flag) {
                AUD_LOG_V("send: IPIMSG_VOW_RECOGNIZE_OK\n");

                /* IPIMSG_VOW_RECOGNIZE_OK */
                vow_ipi_info.ipi_type_flag |= RECOG_OK_IDX_MASK;
                vow_ipi_info.recog_ret_info = RetInfo;
                vow_ipi_info.recog_ok_uuid = recog_ok_id;
                vow_ipi_info.confidence_lv = confidence_lv;
                vow_ipi_info.recog_ok_os_timer = recognize_ok_cycle;
            }

        } else if (ret == VOW_RESULT_FAIL) {
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
            if ((vowserv.PcmDumpFlag == false) &&
                (vowserv.record_flag == false) &&
                (vowserv.debug_P2_flag == false) &&
                !(vowserv.vowTimeFlag & BIT_BARGEIN_PERIOD)) {
#else // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
            if ((vowserv.PcmDumpFlag == false) &&
                (vowserv.record_flag == false) &&
                (vowserv.debug_P2_flag == false)) {
#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
                vow_pmic_low_power_control(&vowserv, VOW_PMIC_LOW_POWER_EXIT);
                vowserv.first_irq_flag = true;
#if PMIC_6337_SUPPORT
                vow_lock_vow_event();
#endif  // #if PMIC_6337_SUPPORT
                vow_pmic_phase2Tophase1(&vowserv);
#if PMIC_6337_SUPPORT
                vow_unlock_vow_event();
#endif  // #if PMIC_6337_SUPPORT
                vow_resume_phase1_setting();
            }
        } else {
            // AUD_LOG_D("not thing: %x\n\r", RetInfo);
        }
#else  // #if VOW_HAS_SWIP
        (void)ret;
        AUD_LOG_D("Bypass\n");
        RetInfo = 0;  // reg ok

        /* IPIMSG_VOW_RECOGNIZE_OK */
        vow_ipi_info.ipi_type_flag |= RECOG_OK_IDX_MASK;
        vow_ipi_info.recog_ret_info = RetInfo;
        vow_ipi_info.recog_ok_uuid = recog_ok_id;
        vow_ipi_info.recog_ok_os_timer = recognize_ok_cycle;

#endif  // #if VOW_HAS_SWIP

        /* send COMBINED INFO IPI to kernel */
        if (vow_ipi_info.ipi_type_flag) {
            int ipi_ret = 0;
            AUD_LOG_V("[vow]send IPI\n");
            ipi_ret = audio_send_ipi_msg(&ipi_msg_send, this->scene, AUDIO_IPI_LAYER_TO_KERNEL,
                               AUDIO_IPI_PAYLOAD, AUDIO_IPI_MSG_BYPASS_ACK,
                               IPIMSG_VOW_COMBINED_INFO,
                                         sizeof(vow_ipi_combined_info_t), 0,
                                         (char *)&vow_ipi_info);
            if (ipi_ret != 0) {
                AUD_LOG_D("[Err]VOW IPI send fail\n");
            }
        }
        if (vowserv.can_unlock_scp_flag == false) {
            mask_irq(MAD_FIFO_IRQn);
            /* Update read_index */
#ifdef BLISRC_USE
            vowserv.blisrc_r_idx += BLISRC_OUTPUT_BUFFER_SIZE_IN_HALF_WORD;
            if (vowserv.blisrc_r_idx >= BLISRC_OUTPUT_TEMP_BUFFER) {
                vowserv.blisrc_r_idx = 0;
            }
            if (bli_working_loop_again == false) {
                if (vowserv.read_idx < VOW_LENGTH_VOICE) {
                    vowserv.read_idx += VOW_SAMPLE_NUM_IRQ;
                }
                if (vowserv.data_length >= VOW_SAMPLE_NUM_IRQ) {
                    vowserv.data_length -= VOW_SAMPLE_NUM_IRQ;
                }
            }
#else  // #ifdef BLISRC_USE
            if (vowserv.read_idx < VOW_LENGTH_VOICE) {
                vowserv.read_idx += VOW_SAMPLE_NUM_IRQ;
            }
            if (vowserv.data_length >= VOW_SAMPLE_NUM_IRQ) {
                vowserv.data_length -= VOW_SAMPLE_NUM_IRQ;
            }
#endif  // #ifdef BLISRC_USE
            unmask_irq(MAD_FIFO_IRQn);
        }

#if TIMES_DEBUG
        end_task_time = *DWT_CYCCNT_1;
        if (end_recog_time > start_recog_time) {
            Temp_recog_time = (end_recog_time - start_recog_time);
        } else {
            Temp_recog_time = max_recog_time;  // for DWT_CYCCNT overflow, giveup this time
        }
        if (Temp_recog_time > max_recog_time) {
            max_recog_time = Temp_recog_time;
        }
        if (Temp_recog_time > 5600000) {  // about 50msec = 50000 * 112
            AUD_LOG_D("ERROR!!! Recognition too long: %d\n", Temp_recog_time);
        }
        if (end_task_time > start_task_time) {
            Temp_task_time = (end_task_time - start_task_time);
        } else {
            Temp_task_time = max_task_time;  // for DWT_CYCCNT overflow, giveup this time
        }
        AUD_LOG_V("[0x%x], <0x%x>\n", Temp_recog_time, Temp_task_time);
        if (avg_cnt == 1100) {
            avg_cnt++;
            total_recog_time /= 1000;
        } else if (avg_cnt > 1100) {
            AUD_LOG_V("AVG:0x%x\n", total_recog_time);
        } else if (avg_cnt >= 100) {
            avg_cnt++;
            total_recog_time += Temp_recog_time;
            // AUD_LOG_D("avg_cnt:0x%x\n", avg_cnt);
        } else {
            avg_cnt++;
        }
        if (Temp_task_time > max_task_time) {
            max_task_time = Temp_task_time;
        }
#endif  // #if TIMES_DEBUG
#ifdef BLISRC_USE
        if (bli_working_loop_again == true) {
            bli_working_loop_again = false;
            AUD_LOG_V("LpAg\n");
            goto WORKING_LOOP_AGAIN;
        }
#endif  // #ifdef BLISRC_USE
    }
    return LoopAg;
}

#ifdef BLISRC_USE

#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT

static void vow_barge_in_start(void)
{
    uint32_t byte_cnt;
    blisrc_accurate_vow_init_param_t accurate_param;
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
    blisrc_accurate_vow_init_param_t accurate_param_R;
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
#ifdef VOW_ECHO_SW_SRC
    blisrc_echo_ref_vow_init_param_t echo_ref_param;
#endif // #ifdef VOW_ECHO_SW_SRC

    if (vowserv.bargeInStatus != BARGEIN_OFF) {
        AUD_LOG_E("[BargeIn] ignore bargein on\n");
        return;
    }

    dvfs_enable_DRAM_resource(VOW_MEM_ID);
    AUD_LOG_E("[BargeIn] vow_barge_in_start\n");

    vowserv.echo_w_idx = 0;
    vowserv.echo_r_idx = 0;
    vowserv.echoRead = 0;
    vowserv.echoReadNowInIRQ = 0;
    vowserv.isAduioFirstIrq = 0;
    vowserv.echoFirstRamReadDone = ECHO_FIRST_READ_OFF;
    vowserv.echo_data_len = 0;
    bargein_ipi_send_flag = false;
    vowserv.frm_cnt = 0;
    memset(vowserv_buf_echo_ref_in, 0, ECHO_REF_BUF_LEN * sizeof(short));
    memset(vowserv_buf_echo_ref_out, 0, ECHO_REF_FRM_LEN * sizeof(short));
    memset(vowserv_buf_echo_ref_sil, 1, ECHO_REF_FRM_LEN * sizeof(short));

    vow_force_phase2(&vowserv);

    accurate_param.input_sampling_rate   = BLISRC_ACCURATE_INPUT_SAMPLING_RATE_IN_HZ;
    accurate_param.output_sampling_rate  = BLISRC_ACCURATE_OUTPUT_SAMPLING_RATE_IN_HZ;
    accurate_param.input_channel_number  = BLISRC_INPUT_CHANNEL_NUBMER;
    accurate_param.output_channel_number = BLISRC_OUTPUT_CHANNEL_NUBMER;
    blisrc_accurate_vow_open(&blisrc_accurate_handle,
                             (unsigned char *)blisrc_accurate_internal_buf,
                             &accurate_param);

    byte_cnt = blisrc_accurate_vow_get_buffer_size();
    double_bli_output_next = false;
    if (byte_cnt != sizeof(blisrc_accurate_internal_buf)) {
        AUD_LOG_E("[BargeIn][err] blisrc buf size error, Query byte cnt: %ld, buf byte cnt: %d\n",
                  byte_cnt, sizeof(blisrc_accurate_internal_buf));
    }

#ifdef VOW_ECHO_SW_SRC
    vowserv.echo_nonsrc_w_idx = ECHO_REF_NONSRC_FRM_LEN * 2;
    vowserv.echo_nonsrc_r_idx = 0;
    vowserv.echo_nonsrc_data_len = ECHO_REF_NONSRC_FRM_LEN * 2;

    memset(vowserv_buf_echo_ref_nonsrc, 0, ECHO_REF_NONSRC_BUF_LEN * sizeof(short));

    echo_ref_param.input_sampling_rate   = BLISRC_ECHO_INPUT_SAMPLING_RATE_IN_HZ;
    echo_ref_param.output_sampling_rate  = BLISRC_ECHO_OUTPUT_SAMPLING_RATE_IN_HZ;
    echo_ref_param.input_channel_number  = BLISRC_INPUT_CHANNEL_NUBMER;
    echo_ref_param.output_channel_number = BLISRC_OUTPUT_CHANNEL_NUBMER;

    blisrc_echo_ref_vow_open(&blisrc_echo_ref_handle,
                             (unsigned char *)blisrc_echo_ref_internal_buf,
                             &echo_ref_param);
    byte_cnt = blisrc_echo_ref_vow_get_buffer_size();
    if (byte_cnt != sizeof(blisrc_echo_ref_internal_buf)) {
        AUD_LOG_E("[BargeIn][err] echo buf size error, Query byte cnt: %ld, buf byte cnt: %d\n",
                  byte_cnt, sizeof(blisrc_echo_ref_internal_buf));
    }
#endif // #ifdef VOW_ECHO_SW_SRC

#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
    memset(vowserv_buf_echo_ref_out_R, 0, ECHO_REF_FRM_LEN * sizeof(short));

    accurate_param_R.input_sampling_rate   = BLISRC_ACCURATE_INPUT_SAMPLING_RATE_IN_HZ;
    accurate_param_R.output_sampling_rate  = BLISRC_ACCURATE_OUTPUT_SAMPLING_RATE_IN_HZ;
    accurate_param_R.input_channel_number  = BLISRC_INPUT_CHANNEL_NUBMER;
    accurate_param_R.output_channel_number = BLISRC_OUTPUT_CHANNEL_NUBMER;
    blisrc_accurate_vow_open(&blisrc_accurate_handle_R,
                             (unsigned char *)blisrc_accurate_internal_buf_R,
                             &accurate_param_R);

    byte_cnt = blisrc_accurate_vow_get_buffer_size();
    if (byte_cnt != sizeof(blisrc_accurate_internal_buf_R)) {
        AUD_LOG_E("[BargeIn][err] blisrc buf size error, Query byte cnt: %ld, buf byte cnt: %d\n",
                  byte_cnt, sizeof(blisrc_accurate_internal_buf_R));
    }
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT

    vowserv.echo_is_sram = echo_ref_is_sram();
    vowserv.bargeInStatus = BARGEIN_START;
    scp_wakeup_src_setup(AUDIO_WAKE_CLK_CTRL, true);
}

void vow_barge_in_stop(void)
{
    if (vowserv.bargeInStatus == BARGEIN_OFF) {
        AUD_LOG_E("[BargeIn] ignore bargein off\n");
        return;
    }
    AUD_LOG_E("[BargeIn] vow_barge_in_stop\n");

    // Only clear audio interrupt if VOW is enabled.
    AUD_LOG_V("[BargeIn] audio INT off in vow_barge_in_stop\n");

    vow_set_mem_awb(false);
    vow_set_bargein_int(false, echo_irq_id);
    scp_wakeup_src_setup(AUDIO_WAKE_CLK_CTRL, false);
    vowserv.bargeInStatus = BARGEIN_OFF;

    dvfs_disable_DRAM_resource(VOW_MEM_ID);

}

static bool echo_ref_is_sram(void)
{
    uint32_t echo_ref_base = DRV_Reg32(AFE_AWB_BASE);
    if ((echo_ref_base & 0xFFFF0000) == (AFE_INTERNAL_SRAM_PHY_BASE & 0xFFFF0000)) {
        AUD_LOG_D("[BargeIn] echo ref is using SRAM\n");
        return true;
    } else {
        AUD_LOG_D("[BargeIn] echo ref is using DRAM\n");
        return false;
    }
}

static void echo_ref_address_map(bool is_sram)
{
    if (is_sram) {
        vowserv.echoReadNowInIRQ = ((vowserv.echoReadNowInIRQ & (~0xF0000000)) + (AFE_INTERNAL_SRAM_CM4_MAP_BASE & 0xF0000000));
        vowserv.echoRefRamEnd = ((vowserv.echoRefRamEnd & (~0xF0000000)) + (AFE_INTERNAL_SRAM_CM4_MAP_BASE & 0xF0000000));
        vowserv.echoRefRamBase = ((vowserv.echoRefRamBase & (~0xF0000000)) + (AFE_INTERNAL_SRAM_CM4_MAP_BASE & 0xF0000000));
        if (vowserv.isAduioFirstIrq == 0) {
            vowserv.echoRead = ((vowserv.echoRead & (~0xF0000000)) + (AFE_INTERNAL_SRAM_CM4_MAP_BASE & 0xF0000000));
            vowserv.isAduioFirstIrq++;
            AUD_LOG_D("S-remap, cur: 0x%x, e: 0x%x, b: 0x%x\n", vowserv.echoReadNowInIRQ, vowserv.echoRefRamEnd,
                      vowserv.echoRefRamBase);
        }
    } else {
        vowserv.echoReadNowInIRQ = ap_to_scp(vowserv.echoReadNowInIRQ);
        vowserv.echoRefRamEnd = ap_to_scp(vowserv.echoRefRamEnd);
        vowserv.echoRefRamBase = ap_to_scp(vowserv.echoRefRamBase);
        if (vowserv.isAduioFirstIrq == 0) {
            vowserv.echoRead = ap_to_scp(vowserv.echoRead); // do address map once only
            vowserv.isAduioFirstIrq++;
            AUD_LOG_D("D-remap, cur: 0x%x, e: 0x%x, b: 0x%x\n", vowserv.echoReadNowInIRQ, vowserv.echoRefRamEnd,
                      vowserv.echoRefRamBase);
        }
    }
    AUD_LOG_V("remap, cur: 0x%x, e: 0x%x, b: 0x%x\n", vowserv.echoReadNowInIRQ, vowserv.echoRefRamEnd,
              vowserv.echoRefRamBase);
}
static void task_echo_ref_getting(void)
{
    uint32_t i;
    bool stereo_flag = false;
    uint32_t echoRefReadNow = vowserv.echoReadNowInIRQ;
    uint32_t echoRefRamEnd = vowserv.echoRefRamEnd;
    uint32_t bytesRead;
    uint32_t samplesRead;
    AUD_LOG_V("a: 0x%x, b: 0x%x, c: 0x%x\n", vowserv.echoRead, echoRefReadNow, DRV_Reg32(AFE_AWB_CUR));
    if (echoRefReadNow > vowserv.echoRead) {
        bytesRead = echoRefReadNow - vowserv.echoRead;
        //AUD_ASSERT(bytesRead >= ECHO_REF_FRM_BYTE_LEN);
    } else {
        bytesRead = echoRefReadNow - vowserv.echoRefRamBase + (echoRefRamEnd + 1) - vowserv.echoRead;
        //AUD_ASSERT(bytesRead >= ECHO_REF_FRM_BYTE_LEN);
    }

    if (bytesRead < ECHO_REF_FRM_BYTE_LEN) {
        AUD_LOG_V("[BargeIn] MEMIF underflow, %d\n", bytesRead);
    }

    echoRefReadNow = vowserv.echoRead;
    if (stereo_flag)
        samplesRead = bytesRead >> 1;
    else
        samplesRead = bytesRead >> 2;

#ifdef VOW_ECHO_SW_SRC
    if (samplesRead > ECHO_REF_NONSRC_FRM_LEN) {
        samplesRead = ECHO_REF_NONSRC_FRM_LEN;
    }
    AUD_LOG_V("test: %x\n", echoRefReadNow);

    for (i = 0; i < samplesRead; i++) {
        vowserv_buf_echo_ref_nonsrc[vowserv.echo_nonsrc_w_idx] = *(short *)echoRefReadNow;

        if (stereo_flag) // todo: stereo case
            echoRefReadNow += (sizeof(short));  // 2 bytes per sample
        else
            echoRefReadNow += (sizeof(int)); // 2 bytes per sample, skip right channel.

        if (echoRefReadNow > echoRefRamEnd) {
            echoRefReadNow = vowserv.echoRefRamBase;
        }

        vowserv.echo_nonsrc_w_idx++;
        if ((vowserv.echo_nonsrc_w_idx) >= ECHO_REF_NONSRC_BUF_LEN) {
            vowserv.echo_nonsrc_w_idx = vowserv.echo_nonsrc_w_idx - ECHO_REF_NONSRC_BUF_LEN;
        }
    }
    vowserv.echo_nonsrc_data_len += samplesRead;
    AUD_LOG_V("ew: %d\n", vowserv.echo_nonsrc_w_idx);
#else
    if (samplesRead > ECHO_REF_FRM_LEN) {
        samplesRead = ECHO_REF_FRM_LEN;
    }
    AUD_LOG_V("test: %x\n", echoRefReadNow);

    for (i = 0; i < samplesRead; i++) {
        vowserv_buf_echo_ref_in[vowserv.echo_w_idx] = *(short *)echoRefReadNow;

        if (stereo_flag) // todo: stereo case
            echoRefReadNow += (sizeof(short));  // 2 bytes per sample
        else
            echoRefReadNow += (sizeof(int)); // 2 bytes per sample, skip right channel.

        if (echoRefReadNow > echoRefRamEnd) {
            echoRefReadNow = vowserv.echoRefRamBase;
        }

        vowserv.echo_w_idx++;
        if ((vowserv.echo_w_idx) >= ECHO_REF_BUF_LEN) {
            vowserv.echo_w_idx = vowserv.echo_w_idx - ECHO_REF_BUF_LEN;
        }
    }
    vowserv.echo_data_len += samplesRead;
    if (vowserv.echoFirstRamReadDone == ECHO_FIRST_READ_RESYNC) {
        vowserv.echoFirstRamReadDone = ECHO_FIRST_READ_OFF;
    }
    if (vowserv.echo_data_len > ECHO_REF_BUF_LEN)  {
        AUD_LOG_E("[BargeIn][err] echo ref buffer overflow, echo_data_len: %d\n", vowserv.echo_data_len);
        vowserv.echoFirstRamReadDone = ECHO_FIRST_READ_RESYNC;
        vowserv.isAduioFirstIrq = 0;
        vowserv.echo_data_len = 0;
    }
#endif // #ifdef VOW_ECHO_SW_SRC
    /* Fill missing samples with zeros
    for ( ; i < ECHO_REF_FRM_LEN; i++ ) {
        vowserv_buf_echo_ref_in[vowserv.echo_w_idx] = 0;

        vowserv.echo_w_idx++;
        if ((vowserv.echo_w_idx) >= ECHO_REF_BUF_LEN) {
            vowserv.echo_w_idx = vowserv.echo_w_idx - ECHO_REF_BUF_LEN;
        }
    }
    */
    if (vowserv.echoFirstRamReadDone == ECHO_FIRST_READ_OFF) {
        vowserv.echoFirstRamReadDone = ECHO_FIRST_READ_DONE;
        AUD_LOG_D("[BargeIn] Echo first MEMIF read done\n");
    }
    vowserv.echoRead = echoRefReadNow;
}

static int vow_bargein_buffer_data(int w_idx, uint32_t delay_sample, uint32_t buf_len)
{
    int r_idx = 0;

    if (w_idx >= delay_sample) {
        r_idx = w_idx - delay_sample;
    } else {
        r_idx = buf_len + w_idx - delay_sample;
    }
    return r_idx;
}
/*
static void initdebugvar()
{
    dump_total = 0;
    dump_total_count = 0;
}
*/
#endif // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT

static bool get_dump_offset(const uint32_t data_len, uint32_t base, uint32_t *dump_offset, uint32_t *w_idx)
{
    uint32_t buf_len = RECOG_DUMP_SIZE_BYTE_DATA;
    bool flag = false;

    *w_idx += data_len;
    if (*w_idx == (buf_len >> 1) + base) {
        *dump_offset = 0;
        flag = true;
    } else if (*w_idx == buf_len + base) {
        *dump_offset = buf_len >> 1;
        *w_idx = base;
        flag = true;
    }

    return flag;
}

static void vow_pcm_dump_process(dump_data_t flag, uint32_t dump_buf_src,
                                 uint32_t dump_buf_size, vow_ipi_combined_info_t *ipi_ptr)
{
//#ifdef DUMP_BARGEIN
    bool ready_to_send = false;
    unsigned int ret = 0;
    uint32_t offset = 0;
    uint32_t buf_base = 0;
    uint32_t pinpon_head = 0;
    //ipi_msg_t ipi_msg_bargein;

    /* only dump on can do dump*/
    if (dump_pcm_flag != 0) {

        //dump_total += dump_buf_size;
        //dump_total_count++;

        if (flag == DUMP_VOW_VOICE) {
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
            pinpon_head = 0;
            AUD_LOG_V("dump_pcm_flag = %d dump_total = %d dump_total_count = %d datasize = %d, addr = 0x%x\n",
                      flag, dump_total, dump_total_count, dump_buf_size, bargein_dump_mic_w);
            ret = vow_dma_transaction(bargein_dump_mic_w, dump_buf_src, dump_buf_size);

            buf_base = vowserv.bargein_dump_dram_addr + pinpon_head;
            get_dump_offset(dump_buf_size, buf_base, &offset, &bargein_dump_mic_w);
            vow_pcm_dump_info.mic_offset = offset + pinpon_head;
#endif  // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        }
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
        else if (flag == DUMP_VOW_VOICE_R) {
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
            pinpon_head = BARGEIN_DUMP_SIZE_BYTE_MIC;
            AUD_LOG_V("dump_pcm_flag = %d dump_total = %d dump_total_count = %d datasize = %d, addr = 0x%x\n",
                      flag, dump_total, dump_total_count, dump_buf_size, bargein_dump_echo_w);
            ret = vow_dma_transaction(bargein_dump_mic_w_R, dump_buf_src, dump_buf_size);
            //get_offset_dram_mic(dump_buf_size);

            buf_base = vowserv.bargein_dump_dram_addr + pinpon_head;
            get_dump_offset(dump_buf_size, buf_base, &offset, &bargein_dump_mic_w_R);
            vow_pcm_dump_info.mic_offset_R = offset + pinpon_head;
#endif  // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        } else if (flag == DUMP_VOW_RECOG_R) {
            pinpon_head = RECOG_DUMP_SIZE_BYTE_DATA;
            ret = vow_dma_transaction(recog_dump_data_w_R, dump_buf_src, dump_buf_size);

            buf_base = vowserv.recog_dump_dram_addr + pinpon_head;
            get_dump_offset(dump_buf_size, buf_base, &offset, &recog_dump_data_w_R);
            vow_pcm_dump_info.recog_dump_offset_R = offset + pinpon_head;
        }
#endif  // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
        else if (flag == DUMP_VOW_RECOG) {
            pinpon_head = 0;
            ret = vow_dma_transaction(recog_dump_data_w, dump_buf_src, dump_buf_size);

            buf_base = vowserv.recog_dump_dram_addr + pinpon_head;
            ready_to_send = get_dump_offset(dump_buf_size, buf_base, &offset, &recog_dump_data_w);
            vow_pcm_dump_info.recog_dump_offset = offset + pinpon_head;
        } else if (flag == DUMP_ECHO_REF) {
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
            pinpon_head = (BARGEIN_DUMP_SIZE_BYTE_MIC << 1);
            AUD_LOG_V("dump_pcm_flag = %d dump_total = %d dump_total_count = %d datasize = %d, addr = 0x%x\n",
                      flag, dump_total, dump_total_count, dump_buf_size, bargein_dump_echo_w);
            ret = vow_dma_transaction(bargein_dump_echo_w, dump_buf_src, dump_buf_size);
            //get_offset_dram_echo(dump_buf_size);

            buf_base = vowserv.bargein_dump_dram_addr + pinpon_head;
            bargein_ipi_send_flag = get_dump_offset(dump_buf_size, buf_base, &offset, &bargein_dump_echo_w);
            vow_pcm_dump_info.echo_offset = offset + pinpon_head;
#endif  // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        }

        if (ret != DMA_RESULT_DONE) {
            AUD_LOG_E("%s(), [err]DMA fail:%x\n", __func__, ret);
        }
#ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        if (bargein_ipi_send_flag) {
            AUD_LOG_V("[Barge]size %d, mic = 0x%x, mic_R = 0x%x, echo = 0x%x\n",
                      vow_pcm_dump_info.mic_dump_size, vow_pcm_dump_info.mic_offset,
                      vow_pcm_dump_info.mic_offset_R, vow_pcm_dump_info.echo_offset);
            /* IPIMSG_VOW_BARGEIN_PCMDUMP_OK */
            ipi_ptr->ipi_type_flag |= BARGEIN_DUMP_IDX_MASK;
            ipi_ptr->bargein_mic_dump_size = vow_pcm_dump_info.mic_dump_size;
            ipi_ptr->bargein_mic_offset = vow_pcm_dump_info.mic_offset;
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
            ipi_ptr->bargein_mic_dump_size_R = vow_pcm_dump_info.mic_dump_size_R;
            ipi_ptr->bargein_mic_offset_R = vow_pcm_dump_info.mic_offset_R;
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
            ipi_ptr->bargein_echo_dump_size = vow_pcm_dump_info.echo_dump_size;
            ipi_ptr->bargein_echo_offset = vow_pcm_dump_info.echo_offset;

            bargein_ipi_send_flag = false;
        }
#endif  // #ifdef CFG_MTK_VOW_BARGE_IN_SUPPORT
        if (ready_to_send) {
            AUD_LOG_V("[Recog]size %d, recog = 0x%x, recog_R = 0x%x\n",
                      vow_pcm_dump_info.recog_dump_size, vow_pcm_dump_info.recog_dump_offset,
                      vow_pcm_dump_info.recog_dump_offset_R);
            /* IPIMSG_VOW_BARGEIN_PCMDUMP_OK */
            //ipi_ptr->recog_dump_flag = 1;
            ipi_ptr->ipi_type_flag |= RECOG_DUMP_IDX_MASK;
            ipi_ptr->recog_dump_size = vow_pcm_dump_info.recog_dump_size;
            ipi_ptr->recog_dump_offset = vow_pcm_dump_info.recog_dump_offset;
#ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
            ipi_ptr->recog_dump_size_R = vow_pcm_dump_info.recog_dump_size_R;
            ipi_ptr->recog_dump_offset_R = vow_pcm_dump_info.recog_dump_offset_R;
#endif // #ifdef CFG_MTK_VOW_DUAL_MIC_SUPPORT
        }
    }
//#endif // #ifdef DUMP_BARGEIN
}
#endif // #ifdef BLISRC_USE

#ifdef NORMAL_SRC_DIRECT_OUT
uint32_t blisrc_vow_get_buffer_size(void)
{
    return sizeof(blisrc_internal_buf);
}

int32_t blisrc_vow_open(void **p_handle,
                        uint8_t *p_internal_buf,
                        blisrc_vow_init_param_t *p_param)
{
    return 0;
}

int32_t blisrc_vow_process(void *handle,
                           int16_t *p_in_buf,
                           uint32_t *available_in_size,
                           int16_t *p_ou_buf,
                           uint32_t *available_ou_size)
{

    uint32_t out_size;
    out_size = *available_ou_size;
    memcpy(p_ou_buf, p_in_buf, out_size);
    return 0;
}
#endif // #ifdef NORMAL_SRC_DIRECT_OUT

#ifdef CHECK_RECOG_TIME
static void vVOWTimerCallback(TimerHandle_t xTimer)
{
#if 0
    BaseType_t xReturn = pdFAIL;
    struct AudioTask *taskPtr = (struct AudioTask *)vowTaskStruct;
#endif
    AUD_LOG_D("==VOW SWIP STUCK, RESET TASK==\n");
#if 0
    /* vow task stuck!. need to restart vow teak */
    if (taskPtr->freertos_task != NULL) {
        vTaskDelete(taskPtr->freertos_task);
    }
    /* end of this current ipi_msg */
    audio_send_ipi_msg_ack_back((ipi_msg_t *)vow_cur_ipi_msg);
    /* clean msg */
    memset((ipi_msg_t *)vow_cur_ipi_msg, 0, sizeof(ipi_msg_t));
    taskPtr->num_queue_element--;

    xReturn = kal_xTaskCreate(
                  task_vow_task_loop,
                  LOCAL_TASK_NAME,
                  LOCAL_TASK_STACK_SIZE,
                  (void *)taskPtr,
                  LOCAL_TASK_PRIORITY,
                  &taskPtr->freertos_task);

    AUD_ASSERT(xReturn == pdPASS);
    AUD_ASSERT(taskPtr->freertos_task);
#endif
}
#endif  // #ifdef CHECK_RECOG_TIME

BaseType_t vow_wake_lock(void *lock)
{
    BaseType_t ret;
#ifdef VOW_WAKELOCK
    if (is_in_isr()) {
        ret = wake_lock_FromISR((wakelock_t *)lock);
    } else {
        ret = wake_lock((wakelock_t *)lock);
    }
#endif
    return ret;
}

BaseType_t vow_wake_unlock(void *lock)
{
    BaseType_t ret;
#ifdef VOW_WAKELOCK
    if (is_in_isr()) {
        ret = wake_unlock_FromISR((wakelock_t *)lock);
    } else {
        ret = wake_unlock((wakelock_t *)lock);
    }
#endif
    return ret;
}

#if 0
static void myprint(const char *message, ...)
{
    va_list args;
    va_start(args, message);

#ifdef DEBUG_KERNEL
    vprintk(message, ##args);
#endif  // #ifdef DEBUG_KERNEL

#ifdef ANDROID_HAL
    __android_log_vprint(ANDROID_LOG_DEBUG, "TAG", message, args);
#endif  // #ifdef ANDROID_HAL

#ifdef DEBUG_CM4
    AUD_LOG_D("[Setlog]");
    vAUD_LOG_D(message, args);
#endif  // #ifdef DEBUG_CM4
    va_end(args);
}
#endif  // #if 0

