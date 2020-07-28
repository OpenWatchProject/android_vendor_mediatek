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
#include "vowAPI_SCP.h"
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

/*==============================================================================
 *                     MACRO
 *============================================================================*/

// #define RECOVERY_TEST



/*==============================================================================
 *                     private global members
 *============================================================================*/
vowserv_t vowserv;

short vowserv_buf_voice[VOW_LENGTH_VOICE] __attribute__((aligned(4)));
short vowserv_buf_speaker[VOW_LENGTH_SPEAKER] __attribute__((aligned(4)));

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

#if PHASE_DEBUG
unsigned int test_cnt = 0;
#endif  // #if PHASE_DEBUG

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
unsigned int swvad_flag = 0;
unsigned int total_recog_time = 0;
unsigned int avg_cnt = 0;

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
static void vow_setModel(vow_event_info_t type, int id, int addr, int size);
static void vow_loadModel(void);
static void vow_sync_data_ready(void);
#if PMIC_6337_SUPPORT
static void vow_ready_receive_fifo(void);
#endif  // #if PMIC_6337_SUPPORT
static void vow_getModel_speaker(void);
int vow_model_init(void *SModel);
int vow_keyword_recognize(void *sample_input, int num_sample, int *ret_info);
static void vow_resume_phase1_setting(void);
#if SMART_AAD_MIC_SUPPORT
static void vow_setSmartDevice(bool enable, unsigned int eint_num);
static void vow_SmartDeviceEnable(void);
static unsigned int vow_getEintNumber(unsigned int eint_num);
#endif  // #if SMART_AAD_MIC_SUPPORT
#ifdef CHECK_RECOG_TIME
static void vVOWTimerCallback(TimerHandle_t xTimer);
#endif  // #ifdef CHECK_RECOG_TIME

/*==============================================================================
 *                     class new/construct/destruct functions
 *============================================================================*/

AudioTask *task_vow_new()
{
    AUD_LOG_V("task_vow_create++\n");
    /* alloc object here */
    AudioTask *task = (AudioTask *)kal_pvPortMalloc(sizeof(AudioTask));
    if (task == NULL) {
        AUD_LOG_E("%s(), kal_pvPortMalloc fail!!\n", __func__);
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
        AUD_LOG_E("%s(), task is NULL!!\n", __func__);
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

    AUD_ASSERT(buf->memory_size >= sizeof(do_save_vow_t));
    AUD_LOG_V("%s(), apreg=0x%x, model=0x%x, size=0x%x\n", __func__,
              vowserv.apreg_addr,
              vowserv.vow_model_event.addr,
              vowserv.vow_model_event.size);
    // save sturcture
    memset((void *)&wb_data, 0, sizeof(wb_data));
    wb_data.vow_status = vowserv.vow_status;
    wb_data.debug_P1_flag = vowserv.debug_P1_flag;
    wb_data.debug_P2_flag = vowserv.debug_P2_flag;
    wb_data.swip_log_anable = vowserv.swip_log_anable;
    wb_data.apreg_addr = vowserv.apreg_addr;
    wb_data.vow_model_event.addr = vowserv.vow_model_event.addr;
    wb_data.vow_model_event.size = vowserv.vow_model_event.size;
    if ((vowserv.vow_model_event.addr != 0) && (vowserv.vow_model_event.size != 0)) {
        wb_data.vow_model_event.flag = true;
    } else {
        wb_data.vow_model_event.flag = false;
    }

    // copy into *buf
    memcpy(buf->p_buffer, &wb_data, sizeof(do_save_vow_t));
    buf->data_size = sizeof(do_save_vow_t);
}

static void task_vow_do_write_back(struct AudioTask *this,
                                   data_buf_t *buf)
{
    do_save_vow_t wb_data;

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
    if (wb_data.vow_model_event.addr) {
        vowserv.vow_model_event.addr = wb_data.vow_model_event.addr;
        vowserv.vow_model_event.size = wb_data.vow_model_event.size;
        vowserv.vow_model_event.flag = true;
        AUD_LOG_V("DO WB, model_addr=0x%x, size=0x%x\n", vowserv.vow_model_event.addr, vowserv.vow_model_event.size);
        vow_loadModel();
    }
}


static void task_vow_hal_reboot_cbk(struct AudioTask *this, const uint8_t hal_type) {
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
    this->msg_array = (ipi_msg_t *)kal_pvPortMalloc(MAX_MSG_QUEUE_SIZE * sizeof(ipi_msg_t));
    AUD_ASSERT(this->msg_array != NULL);
    memset(this->msg_array, 0, MAX_MSG_QUEUE_SIZE * sizeof(ipi_msg_t));

    this->msg_idx_queue = xQueueCreate(MAX_MSG_QUEUE_SIZE, sizeof(uint8_t));
    AUD_ASSERT(this->msg_idx_queue != NULL);

    task_vow_init(this);
}

static void task_vow_destructor(struct AudioTask *this)
{
    if (this == NULL) {
        AUD_LOG_E("%s(), this is NULL!!\n", __func__);
        return;
    }

    AUD_LOG_V("%s(+), task_scene = %d\n", __func__, this->scene);
    task_vow_deinit(this);
    /* dealloc private memory & dealloc object here */
    if (this->msg_array != NULL) {
        kal_vPortFree(this->msg_array);
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
    AUD_LOG_D("this = %d, vowTaskStruct = %d\n", this, vowTaskStruct);
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

static uint8_t get_queue_idx(struct AudioTask *this)
{
    uint8_t queue_idx = this->queue_idx;

    this->queue_idx++;
    if (this->queue_idx == MAX_MSG_QUEUE_SIZE) {
        this->queue_idx = 0;
    }

    return queue_idx;
}



static status_t task_vow_recv_message(
    struct AudioTask *this,
    struct ipi_msg_t *ipi_msg)
{
    static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    uint8_t ret = false;
    uint8_t queue_idx = 0, i = 0;
    mask_irq(MAD_FIFO_IRQn);  /* prevent nested interrupt: MD_IRQn has higher prior */
    this->num_queue_element++;
    queue_idx = get_queue_idx(this);

    // check message
    if (this->msg_array[queue_idx].magic != 0) {
        AUD_LOG_D("queue_idx = %d\n", queue_idx);
        for (i = 0; i < MAX_MSG_QUEUE_SIZE; i++) {
            AUD_LOG_E("[%d] id = 0x%x\n", i, this->msg_array[i].msg_id);
        }
        AUD_ASSERT(this->msg_array[queue_idx].magic == 0);  /* item is clean */
    }
    // copy to array
    memcpy(&this->msg_array[queue_idx], ipi_msg, sizeof(ipi_msg_t));
    unmask_irq(MAD_FIFO_IRQn);

    // AUD_LOG_V("Recv id = 0x%x\n", ipi_msg->msg_id);
    AUD_LOG_V("ele:%d, idx: %d, msg:%d\n", this->num_queue_element, queue_idx, this->msg_array[queue_idx].msg_id);
    ret = task_vow_preparsing_message(this, &this->msg_array[queue_idx]);
    if (ret) {
        if (xQueueSendToBackFromISR(this->msg_idx_queue, &queue_idx,
                                    &xHigherPriorityTaskWoken) != pdTRUE) {
            AUD_LOG_D("vow queue UNKNOW ERROR\n");
            return UNKNOWN_ERROR;
        }
    } else {
        /* clean msg */
        memset(&this->msg_array[queue_idx], 0, sizeof(ipi_msg_t));
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
#ifdef VOW_WAKELOCK
        wake_lock_FromISR(&vow_wakelock);
        vowserv.scpwakelock = SCP_LOCK;
#endif  // #ifdef VOW_WAKELOCK
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
#if PMIC_6358_SUPPORT
        if (vowserv.first_irq_flag) {
            vow_pmic_low_power_control(&vowserv, VOW_PMIC_LOW_POWER_ENTRY);
        }
#endif  // #if PMIC_6358_SUPPORT

#if !PMIC_6337_SUPPORT
        if (vowserv.scpwakelock == SCP_UNLOCK) {
#ifdef VOW_WAKELOCK
            wake_lock_FromISR(&vow_wakelock);
            vowserv.scpwakelock = SCP_LOCK;
#endif  // #ifdef VOW_WAKELOCK
        }
#endif  // #if !PMIC_6337_SUPPORT
        /*----------------------------------------------------------------------
        //  Check Status
        ----------------------------------------------------------------------*/
        if (vowserv.vow_status == VOW_STATUS_IDLE) {
            // AUD_LOG_D("mic_idle\n");
            vow_read_fifo_data(NULL, VOW_SAMPLE_NUM_IRQ);
            vow_reset_fifo_irq();
            return;
        }
        if (vowserv.drop_count < vowserv.drop_frame) {
            // AUD_LOG_D("drop\n");
            vow_read_fifo_data(NULL, VOW_SAMPLE_NUM_IRQ);
            vowserv.drop_count++;
            vow_reset_fifo_irq();
            return;
        }

        if ((vowserv.write_idx == vowserv.read_idx) && (vowserv.data_length == VOW_LENGTH_VOICE)) {
            AUD_LOG_D("Buffer full: w:%d\n", vowserv.write_idx);
            vow_read_fifo_data(NULL, VOW_SAMPLE_NUM_IRQ);

            vowserv.read_idx = VOW_LENGTH_VOICE + 1;
            vowserv.write_idx = 0;
#ifdef __SEAMLESS_RECORD_SUPPORT__
            vowserv.read_buf_idx = SEAMLESS_RECORD_LENGTH + 1;
#endif  // #ifdef __SEAMLESS_RECORD_SUPPORT__
            vowserv.data_length = 0;

            vow_reset_fifo_irq();
            return;
        }

        /*----------------------------------------------------------------------
        //  Read From VOW FIFO and do memcpy
        ----------------------------------------------------------------------*/
        vow_read_fifo_data(&vowserv_buf_voice[vowserv.write_idx], VOW_SAMPLE_NUM_IRQ);

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
        queue_idx = get_queue_idx(this);
        ipi_msg = &this->msg_array[queue_idx];
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

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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

    while (1) {
        AUD_LOG_V("vow task is running...\n");
        if (xQueueReceive(this->msg_idx_queue, &local_queue_idx,
                          portMAX_DELAY) == pdTRUE) {
               AUD_LOG_V("vow TaskProject received = %d\n", local_queue_idx);
        }
        ipi_msg = &this->msg_array[local_queue_idx];
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
                vow_setModel((vow_event_info_t)item[0], item[1], item[2], item[3]);
                vow_loadModel();
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
            case VOW_RUN:
                this->state = AUDIO_TASK_WORKING;
                task_vow_working(this, ipi_msg);
                break;
            default:
                // AUD_LOG_E("Not support id: 0x%x\n", ipi_msg->msg_id);
                break;
        }
        /* send ack back if need */
        AUD_LOG_V("%s(), id:%x, ack:%x\n", __func__, ipi_msg->msg_id, ipi_msg->ack_type);
        audio_send_ipi_msg_ack_back(ipi_msg);  // call this for need ack msg
        /* clean msg */
        memset(ipi_msg, 0, sizeof(ipi_msg_t));
        this->num_queue_element--;

        if (vowserv.can_unlock_scp_flag == true) {
            AUD_LOG_V("unlock_scp\n");
#ifdef VOW_WAKELOCK
            wake_unlock(&vow_wakelock);
            vowserv.scpwakelock = SCP_UNLOCK;
#endif  // #ifdef VOW_WAKELOCK
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
            ipi_msg->data_type = AUDIO_IPI_MSG_ONLY;
            ipi_msg->param1 = VOW_IPI_SUCCESS;
            ret = true;
            break;
        case IPIMSG_VOW_DATAREADY_ACK:
            ipi_msg->data_type = AUDIO_IPI_MSG_ONLY;
            ipi_msg->param1 = VOW_IPI_SUCCESS;
            vow_sync_data_ready();
            ret = false;
            break;
        default:
            AUD_LOG_E("Not support id: 0x%x\n", ipi_msg->msg_id);
            break;
    }
    return ret;
}


/***********************************************************************************
** task_vow_init - Initial funciton in scp init
************************************************************************************/
static void task_vow_init(struct AudioTask *this)
{
    AUD_LOG_V("vow init\n");
#ifdef VOW_WAKELOCK
    wake_lock_init(&vow_wakelock, "vow1");
#endif  // #ifdef VOW_WAKELOCK
    vowserv.vow_status      = VOW_STATUS_IDLE;
    vowserv.read_idx        = VOW_LENGTH_VOICE + 1;
    vowserv.write_idx       = 0;
#ifdef __SEAMLESS_RECORD_SUPPORT__
    vowserv.read_buf_idx    = SEAMLESS_RECORD_LENGTH + 1;
#endif  // #ifdef __SEAMLESS_RECORD_SUPPORT__
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
    vowserv.vow_model_event.func = vow_getModel_speaker;
    vowserv.vow_model_event.addr = 0;
    vowserv.vow_model_event.size = 0;
    vowserv.vow_model_event.flag = false;

    memset(vowserv_buf_speaker, 0, VOW_LENGTH_SPEAKER * sizeof(short));
    memset(vowserv_buf_voice, 0, VOW_LENGTH_VOICE * sizeof(short));

    vowserv.vow_model_event.data = (short*)vowserv_buf_speaker;
}

/***********************************************************************************
** task_vow_deinit - Deinitial funciton in scp
************************************************************************************/
void task_vow_deinit(struct AudioTask *this)
{
    vow_disable();
#ifdef VOW_WAKELOCK
    wake_unlock(&vow_wakelock);
    vowserv.scpwakelock = SCP_UNLOCK;
#endif  // #ifdef VOW_WAKELOCK
}

/***********************************************************************************
** vow_enable - enable VOW
************************************************************************************/
static void vow_enable(void)
{
    int ret = 0;
#ifdef __SEAMLESS_RECORD_SUPPORT__
    unsigned int rdata;
#endif  // #ifdef __SEAMLESS_RECORD_SUPPORT__

    AUD_LOG_D("+Enable_%x\n", vowserv.vow_status);
    if (vowserv.vow_status == VOW_STATUS_IDLE) {
        ret = vow_model_init(vowserv.vow_model_event.data);

        // Enable VOW FIFO
        vow_enable_fifo(&vowserv);

        vowserv.read_idx = VOW_LENGTH_VOICE + 1;
        vowserv.write_idx = 0;
#ifdef __SEAMLESS_RECORD_SUPPORT__
        vowserv.read_buf_idx = SEAMLESS_RECORD_LENGTH + 1;
#endif  // #ifdef __SEAMLESS_RECORD_SUPPORT__
        vowserv.data_length = 0;

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
#if PMIC_6358_SUPPORT
        /* Inintail settings for 6358 low power mode*/
        vow_pmic_low_power_control(&vowserv, VOW_PMIC_LOW_POWER_INIT);
#endif  // #if PMIC_6358_SUPPORT
#ifdef __SEAMLESS_RECORD_SUPPORT__
        rdata = vow_pwrapper_read(AFE_VOW_TOP);
        if ((rdata & 0x0800) == 0x0000) {
            vow_pmic_idleTophase1(&vowserv);
        } else {
            vow_pmic_phase2Tophase1(&vowserv);
        }
        vowserv.sync_data_ready = true;
#else  // #ifdef __SEAMLESS_RECORD_SUPPORT__
        vow_pmic_idleTophase1(&vowserv);
#endif  // #ifdef __SEAMLESS_RECORD_SUPPORT__
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
#ifdef VOW_WAKELOCK
            wake_lock(&vow_wakelock);
            vowserv.scpwakelock = SCP_LOCK;
#endif  // #ifdef VOW_WAKELOCK
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
#if PHASE_DEBUG
        test_cnt = 0;
#endif  // #if PHASE_DEBUG

#if TIMES_DEBUG
        total_recog_time = 0;
        avg_cnt = 0;
        start_recog_time = 0;
        end_recog_time = 0;
        max_recog_time = 0;
        start_task_time = 0;
        end_task_time = 0;
        max_task_time = 0;
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
    } else {
#ifdef __SEAMLESS_RECORD_SUPPORT__
#if PMIC_6337_SUPPORT
        vow_lock_vow_event();
#endif  // #if PMIC_6337_SUPPORT
        rdata = vow_pwrapper_read(AFE_VOW_TOP);
        AUD_LOG_D("vow_status not in idle, AFE_VOW_TOP:%x\n", rdata);
        if ((rdata & 0x0800) == 0x0000) {
            AUD_LOG_D("ENABLE: IDLE to P1\n");
            vow_pmic_idleTophase1(&vowserv);
        } else {
            if (vowserv.seamless_record == false) {
                AUD_LOG_D("ENABLE: P2 to Idle To Phase1\n");
                vow_pmic_idle(&vowserv);
                vow_pmic_idleTophase1(&vowserv);
            }
        }
#if PMIC_6337_SUPPORT
        vow_unlock_vow_event();
#endif  // #if PMIC_6337_SUPPORT
#endif  // #ifdef __SEAMLESS_RECORD_SUPPORT__
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
#if PMIC_6337_SUPPORT
        vow_lock_vow_event();
#endif  // #if PMIC_6337_SUPPORT
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
#ifdef __SEAMLESS_RECORD_SUPPORT__
        vowserv.read_buf_idx = SEAMLESS_RECORD_LENGTH + 1;
#endif  // #ifdef __SEAMLESS_RECORD_SUPPORT__
        vowserv.data_length = 0;

        // For SW VAD
        vowserv.noKeywordCount = 0;

#if PHASE_DEBUG
        test_cnt = 0;
#endif  // #if PHASE_DEBUG

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
static void vow_sync_data_ready(void)
{
#ifdef __SEAMLESS_RECORD_SUPPORT__
    vowserv.sync_data_ready = true;
#endif  // #ifdef __SEAMLESS_RECORD_SUPPORT__
}

/***********************************************************************************
** vow_getModel_speaker - Get speaker model informations through DMA
************************************************************************************/
static void vow_getModel_speaker(void)
{
    char *ptr8;
    unsigned int ret;

    AUD_LOG_D("GetModel S_%x %x %x\n", vowserv.vow_model_event.addr,
                                       vowserv.vow_model_event.size,
                                       (int)&vowserv_buf_speaker[0]);
    ret = vow_dma_transaction((uint32_t)&vowserv_buf_speaker[0],
                              (uint32_t)vowserv.vow_model_event.addr,
                              (uint32_t)vowserv.vow_model_event.size);
    if (ret != DMA_RESULT_DONE) {
         AUD_LOG_E("%s(), DMA fail:%x\n", __func__, ret);
    }
    // To do: DMA Speaker Model into vowserv_buf_speaker
    ptr8 = (char*)vowserv_buf_speaker;

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
        case VOW_FLAG_DEBUG:
            AUD_LOG_V("FgDebug_%x\n", enable);
            vowserv.record_flag = enable;
#ifdef __SEAMLESS_RECORD_SUPPORT__
            vowserv.seamless_record = enable;
#endif  // #ifdef __SEAMLESS_RECORD_SUPPORT__
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
            AUD_LOG_V("SWIP show log:%x\n", enable);
            vowserv.swip_log_anable = enable;
            setPrint(vowserv.swip_log_anable);
            break;
        case VOW_FLAG_MTKIF_TYPE:
            AUD_LOG_V("MTKIF type:%x\n", enable);
            vowserv.mtkif_type = enable;
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
static void vow_SmartDeviceEnable(void) {
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
** vow_setModel - set model information
************************************************************************************/
static void vow_setModel(vow_event_info_t type, int id, int addr, int size)
{
    AUD_LOG_D("SetModel+ %x %x %x %x\n", type, id, addr, size);

    AUD_ASSERT(type == VOW_MODEL_SPEAKER);
    AUD_ASSERT((size >> 1) <= VOW_LENGTH_SPEAKER);

    AUD_LOG_D("cur model addr:0x%x\n", vowserv.vow_model_event.addr);

    vowserv.vow_model_event.addr = ap_to_scp(addr);
    vowserv.vow_model_event.size = size;
    vowserv.vow_model_event.flag = true;

    return;
}

/***********************************************************************************
** vow_LoadModel - Load Model Infomation from AP
************************************************************************************/
static void vow_loadModel(void)
{
    // Set vow_model
    if (vowserv.vow_model_event.flag) {
        vowserv.vow_model_event.func();
        vowserv.vow_model_event.flag = false;
    } else {
        AUD_LOG_D("model event is Empty\n");
    }
}

/***********************************************************************************
** vow_model_init - Initialize SWIP with different model_type
************************************************************************************/
int vow_model_init(void *SModel)
{
    int ret = -1;

#if (__VOW_GOOGLE_SUPPORT__)
    AUD_LOG_D("Google_Model init\n");
    ret = GoogleHotwordDspInit((void *)SModel);
#elif(__VOW_MTK_SUPPORT__)
    AUD_LOG_D("MTK_Model init\n");
    ret = TestingInit_Model((char *)SModel);
    TestingInit_Table(0);
#else  // #if (__VOW_GOOGLE_SUPPORT__)
    AUD_LOG_D("Not Support\n");
#endif  // #if (__VOW_GOOGLE_SUPPORT__)
    return ret;
}

/***********************************************************************************
** vow_keyword_Recognize - use SWIP to recognize keyword with different model_type
************************************************************************************/
int vow_keyword_recognize(void *sample_input, int num_sample, int *ret_info)
{
    int ret = -1;
#ifdef CHECK_RECOG_TIME
    /* start to check SWIP operating time, if > 15ms, then recreate vow task */
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

#if (__VOW_GOOGLE_SUPPORT__)
    ret = GoogleHotwordDspProcess(sample_input, num_sample, ret_info);
    if (ret == 1) {
        // Reset in order to resume detection.
        GoogleHotwordDspReset();
    } else {  // ret = 0
        // call MTK SW VAD, check if in "no-sound" place
        vowserv.noKeywordCount++;
#if TIMES_DEBUG
        swvad_flag = 1;
#endif  // #if TIMES_DEBUG
        if (returnToPhase1BySwVad((short *)sample_input, vowserv.noKeywordCount)) {
            ret = 2;  // Become no sound, back to phase1
        }
#if TIMES_DEBUG
        swvad_flag = 0;
#endif  // #if TIMES_DEBUG
    }
#elif(__VOW_MTK_SUPPORT__)
    (void)num_sample;
    ret = onTesting((short *)sample_input, ret_info);  // 0: not thing, 1: pass, 2: fail(no-sound)
    if (ret == 0) {
        vowserv.noKeywordCount++;
#if TIMES_DEBUG
        swvad_flag = 1;
#endif  // #if TIMES_DEBUG
        if (returnToPhase1BySwVad((short *)sample_input, vowserv.noKeywordCount)) {
            ret = 2;  // Become no sound, back to phase1
        }
#if TIMES_DEBUG
        swvad_flag = 0;
#endif  // #if TIMES_DEBUG
    }
    if ((ret == 1) || (ret == 2)) {
        resetSwVadPara();
        vowserv.noKeywordCount = 0;
    }
#else  // #if (__VOW_GOOGLE_SUPPORT__)
    AUD_LOG_D("Not Support\n");
#endif  // #if (__VOW_GOOGLE_SUPPORT__)

#ifdef CHECK_RECOG_TIME
    /* stop checking SWIP operating time. if it doesn't cause timer handler,
    then means operating time is lower than 15ms */
    xTimerStop(xVowTimer, 0);
#endif  // #ifdef CHECK_RECOG_TIME
    return ret;
}

/***********************************************************************************
** vow_resume_phase1 - handle new control flow when P2 to P1
************************************************************************************/
static void vow_resume_phase1_setting(void) {
    mask_irq(MAD_FIFO_IRQn);
    vowserv.read_idx = VOW_LENGTH_VOICE + 1;
    vowserv.write_idx = 0;
#ifdef __SEAMLESS_RECORD_SUPPORT__
    vowserv.read_buf_idx = SEAMLESS_RECORD_LENGTH + 1;
#endif  // #ifdef __SEAMLESS_RECORD_SUPPORT__
    vowserv.data_length = 0;
    unmask_irq(MAD_FIFO_IRQn);

#if PMIC_6337_SUPPORT
    // already turn on, then clear periodic On/Off flag
    if ((vowserv.open_periodic_flag == true) && (vowserv.periodic_need_enable == true)) {
        vowserv.open_periodic_flag = false;
    }
    vow_datairq_reset();

#endif  // #if PMIC_6337_SUPPORT
    vowserv.can_unlock_scp_flag = true;  // flag means can back to phase1
}

/***********************************************************************************
** task_vow_working - main task content
************************************************************************************/
static uint8_t task_vow_working(
    struct AudioTask *this,
    struct ipi_msg_t *ipi_msg)
{
    int ret, RetInfo;
    int src_fifo_r_idx;  // this is index for catching from FIFO buffer
    // int data_amount;
    ipi_msg_t ipi_msg_send;
    // uint8_t queue_idx = 0;
    uint8_t LoopAg = 0;
    vow_pcmdump_info_t pcmdump_info;
#ifdef __SEAMLESS_RECORD_SUPPORT__
    int w_idx;
    int r_idx;
    unsigned int Offset_Size;
    unsigned int Tx_Size;
    int delay_count = 0;
    unsigned int Rdy2Send_Offset = 0;
    unsigned int Rdy2Send_Size = 0;
    short *seamless_buf_ptr;
#else  // #ifdef __SEAMLESS_RECORD_SUPPORT__
    bool pcmdump_start_remind_kernel = false;
    unsigned int pingpong_offset;
#endif  // #ifdef __SEAMLESS_RECORD_SUPPORT__
#if TIMES_DEBUG
    unsigned int Temp_recog_time = 0;
    unsigned int Temp_task_time = 0;
#endif  // #if TIMES_DEBUG
    short *recog_buf_ptr;
    unsigned long long recognize_ok_cycle = 0;

    if ((vowserv.vow_status != VOW_STATUS_IDLE)
     && (vowserv.data_length >= VOW_SAMPLE_NUM_IRQ)) {
#if TIMES_DEBUG
        start_task_time = *DWT_CYCCNT_1;
#endif  // #if TIMES_DEBUG
        mask_irq(MAD_FIFO_IRQn);
        if (vowserv.read_idx >= VOW_LENGTH_VOICE) {
            vowserv.read_idx = 0;
        }
#ifdef __SEAMLESS_RECORD_SUPPORT__
        if (vowserv.read_buf_idx >= SEAMLESS_RECORD_LENGTH) {
            vowserv.read_buf_idx = 0;
        }
#endif  // #ifdef __SEAMLESS_RECORD_SUPPORT__

        // Get current read_index
        src_fifo_r_idx = vowserv.read_idx;
        unmask_irq(MAD_FIFO_IRQn);



        recog_buf_ptr = &vowserv_buf_voice[src_fifo_r_idx];
        AUD_ASSERT(src_fifo_r_idx < VOW_LENGTH_VOICE);
        if (vowserv.record_flag) {
#ifdef __SEAMLESS_RECORD_SUPPORT__
            seamless_buf_ptr = vowserv_buf_voice;
            w_idx = vowserv.write_idx;
            r_idx = src_fifo_r_idx;  // this is vowserv.read_idx

            if ((vowserv.seamless_record == true) && (delay_count >= SEAMLESS_DELAY)) {
                vowserv.seamless_record = false;
                if (vowserv.sync_data_ready) {  // seamless record don't need this flag help
                    vowserv.sync_data_ready = false;
                }
                delay_count = 0;
                // Transfer the previous voice data in seamless record
                Rdy2Send_Offset = 0;
                if (w_idx >= r_idx) {
                    Offset_Size = (SEAMLESS_RECORD_LENGTH - w_idx) << 1;
                    // DMA from W_idx to bottom
                    ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr,
                                              (uint32_t)&seamless_buf_ptr[w_idx],
                                              Offset_Size);
                    if (ret != DMA_RESULT_DONE) {
                        AUD_LOG_E("%s(), DMA bottom fail:%x\n", __func__, ret);
                    }
                    // DMA from top to r_idx
                    ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr + Offset_Size,
                                              (uint32_t)&seamless_buf_ptr[0],
                                              ((r_idx + VOW_SAMPLE_NUM_FRAME) << 1));
                    if (ret != DMA_RESULT_DONE) {
                        AUD_LOG_E("%s(), DMA top fail:%x\n", __func__, ret);
                    }

                    Rdy2Send_Size = Offset_Size + ((r_idx + VOW_SAMPLE_NUM_FRAME) << 1);
                } else {
                    Offset_Size = ((r_idx - w_idx + VOW_SAMPLE_NUM_FRAME) << 1);
                    ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr,
                                              (uint32_t)&seamless_buf_ptr[w_idx],
                                              Offset_Size);
                    if (ret != DMA_RESULT_DONE) {
                        AUD_LOG_E("%s(), DMA fail:%x\n", __func__, ret);
                    }
                    Rdy2Send_Size = Offset_Size;
                }
                vowserv.read_buf_idx = r_idx + VOW_SAMPLE_NUM_FRAME;
                AUD_LOG_V("W:0x%x, R:0x%x, Total:0x%xByte\n", w_idx, r_idx, Rdy2Send_Size);
            } else if (vowserv.seamless_record == true) {
                delay_count++;
                Rdy2Send_Size = 0;
                Rdy2Send_Offset = 0;
            } else {
                // upload pcm dump data into Kernel
                if (vowserv.sync_data_ready) {
                    vowserv.sync_data_ready = false;
                    if (r_idx >= vowserv.read_buf_idx) {
                        Tx_Size = r_idx - vowserv.read_buf_idx + VOW_SAMPLE_NUM_FRAME;
                        ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr,
                                                  (uint32_t)&seamless_buf_ptr[vowserv.read_buf_idx],
                                                  (Tx_Size << 1));
                        if (ret != DMA_RESULT_DONE) {
                            AUD_LOG_E("%s(), DMA fail:%x\n", __func__, ret);
                        }
                    } else {
                        Tx_Size = (SEAMLESS_RECORD_LENGTH - vowserv.read_buf_idx) + r_idx + VOW_SAMPLE_NUM_FRAME;
                        Offset_Size = ((SEAMLESS_RECORD_LENGTH - vowserv.read_buf_idx) << 1);
                        ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr,
                                                  (uint32_t)&seamless_buf_ptr[vowserv.read_buf_idx],
                                                  Offset_Size);
                        if (ret != DMA_RESULT_DONE) {
                            AUD_LOG_E("%s(), DMA bottom fail:%x\n", __func__, ret);
                        }
                        ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr + Offset_Size,
                                                  (uint32_t)&seamless_buf_ptr[0],
                                                  ((r_idx + VOW_SAMPLE_NUM_FRAME) << 1));
                        if (ret != DMA_RESULT_DONE) {
                            AUD_LOG_E("%s(), DMA bottom fail:%x\n", __func__, ret);
                        }
                    }
                    Rdy2Send_Size = (Tx_Size << 1);
                    Rdy2Send_Offset = 0;
                    vowserv.read_buf_idx += Tx_Size;
                    AUD_LOG_V("R:0x%x, R_buf:0x%x, Total:0x%xByte\n", r_idx, vowserv.read_buf_idx, Rdy2Send_Size);
                }
            }
            if (Rdy2Send_Size > 0) {
                pcmdump_info.dump_buf_offset = Rdy2Send_Offset;  // DMA offset
                pcmdump_info.dump_size = Rdy2Send_Size;  // DMA size (Byte)
                audio_send_ipi_msg(&ipi_msg_send, this->scene, AUDIO_IPI_LAYER_TO_KERNEL,
                                   AUDIO_IPI_PAYLOAD, AUDIO_IPI_MSG_BYPASS_ACK,
                                   IPIMSG_VOW_DATAREADY,
                                   sizeof(vow_pcmdump_info_t), 0, (char *)&pcmdump_info);
            }
#else  // #ifdef __SEAMLESS_RECORD_SUPPORT__
            ret = vow_dma_transaction((uint32_t)vowserv.apreg_addr +
                                      (vowserv.pcmdump_cnt * (VOW_SAMPLE_NUM_FRAME << 1)),
                                      (uint32_t)recog_buf_ptr,
                                      (VOW_SAMPLE_NUM_FRAME << 1));
            if (ret != DMA_RESULT_DONE) {
                AUD_LOG_E("%s(), DMA fail:%x\n", __func__, ret);
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
                audio_send_ipi_msg(&ipi_msg_send, this->scene, AUDIO_IPI_LAYER_TO_KERNEL,
                                   AUDIO_IPI_PAYLOAD, AUDIO_IPI_MSG_BYPASS_ACK,
                                   IPIMSG_VOW_DATAREADY,
                                   sizeof(vow_pcmdump_info_t), 0, (char *)&pcmdump_info);
            }
#endif  // #ifdef __SEAMLESS_RECORD_SUPPORT__
        }

#if VOW_HAS_SWIP

#if TIMES_DEBUG
        start_recog_time = *DWT_CYCCNT_1;
#endif  // #if TIMES_DEBUG
        ret = VOW_RESULT_NOTHING;

        ret = vow_keyword_recognize((void *)recog_buf_ptr, VOW_SAMPLE_NUM_FRAME, &RetInfo);

#if TIMES_DEBUG
        end_recog_time = *DWT_CYCCNT_1;
#endif  // #if TIMES_DEBUG
        if (ret == VOW_RESULT_PASS) {
            AUD_LOG_D("Recog Ok: %x\n", RetInfo);
            recognize_ok_cycle = timer_get_global_timer_tick();
            if ((vowserv.record_flag == false) && (vowserv.debug_P2_flag == false)) {
#if PHASE_DEBUG
                AUD_LOG_D("P2_cnt:%d\n", test_cnt);
                test_cnt = 0;
#endif  // #if PHASE_DEBUG

#if !defined(__SEAMLESS_RECORD_SUPPORT__)
#if PMIC_6337_SUPPORT
                vow_lock_vow_event();
#endif  // #if PMIC_6337_SUPPORT
                vow_pmic_phase2Tophase1(&vowserv);
#if PMIC_6358_SUPPORT
                vow_pmic_low_power_control(&vowserv, VOW_PMIC_LOW_POWER_EXIT);
#endif  // #if PMIC_6358_SUPPORT
#if PMIC_6337_SUPPORT
                vow_unlock_vow_event();
#endif  // #if PMIC_6337_SUPPORT
                vow_resume_phase1_setting();
#endif  // #if !defined(__SEAMLESS_RECORD_SUPPORT__)
            }

            audio_send_ipi_msg(&ipi_msg_send, this->scene, AUDIO_IPI_LAYER_TO_KERNEL,
                               AUDIO_IPI_PAYLOAD, AUDIO_IPI_MSG_BYPASS_ACK,
                               IPIMSG_VOW_RECOGNIZE_OK,
                               sizeof(unsigned long long), RetInfo, (char *)&recognize_ok_cycle);

        } else if (ret == VOW_RESULT_FAIL) {
            if ((vowserv.record_flag == false) && (vowserv.debug_P2_flag == false)) {
#if PHASE_DEBUG
                AUD_LOG_D("P2_cnt:%d\n", test_cnt);
                test_cnt = 0;
#endif  // #if PHASE_DEBUG
#if PMIC_6337_SUPPORT
                vow_lock_vow_event();
#endif  // #if PMIC_6337_SUPPORT
                vow_pmic_phase2Tophase1(&vowserv);
#if PMIC_6358_SUPPORT
                vow_pmic_low_power_control(&vowserv, VOW_PMIC_LOW_POWER_EXIT);
#endif  // #if PMIC_6358_SUPPORT
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
        audio_send_ipi_msg(&ipi_msg_send, this->scene, AUDIO_IPI_LAYER_TO_KERNEL,
                           AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                           IPIMSG_VOW_RECOGNIZE_OK,
                           0, RetInfo, NULL);
#endif  // #if VOW_HAS_SWIP
        if (vowserv.can_unlock_scp_flag == false) {
            mask_irq(MAD_FIFO_IRQn);
            /* Update read_index */
            if (vowserv.read_idx < VOW_LENGTH_VOICE) {
                vowserv.read_idx += VOW_SAMPLE_NUM_IRQ;
            }
            if (vowserv.data_length >= VOW_SAMPLE_NUM_IRQ) {
                vowserv.data_length -= VOW_SAMPLE_NUM_IRQ;
            }
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
    }
    return LoopAg;
}

#ifdef CHECK_RECOG_TIME
static void vVOWTimerCallback(TimerHandle_t xTimer) {
    BaseType_t xReturn = pdFAIL;
    struct AudioTask *taskPtr = (struct AudioTask *)vowTaskStruct;

    AUD_LOG_D("==VOW SWIP STUCK, RESET TASK==\n");
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
}
#endif  // #ifdef CHECK_RECOG_TIME

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

