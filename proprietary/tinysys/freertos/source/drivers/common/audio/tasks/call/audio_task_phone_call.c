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


#include "audio_task_phone_call.h"

#include <stdarg.h> /* va_list, va_start, va_arg, va_end */
#include <stdint.h> /* va_list, va_start, va_arg, va_end */

#include <interrupt.h>
#include <dma.h>
#include <feature_manager.h>
#include <dvfs.h>
#include <mt_gpt.h>

#include <wrapped_audio.h>
#include <wrapped_errors.h>

#include "audio_messenger_ipi.h"
#include "audio_speech_msg_id.h"

#include "audio_shared_tcm.h"

#include <arsi_api.h>
#include <audio_task.h>
#include <arsi_library_entry_points.h>
#include <arsi_call_type.h>

#include <stdio.h>
#include "wakelock.h"
#include "hal_cache.h"

#include <wrapped_audio.h>
#include <audio_ringbuf_pure.h>
#include <audio_task_aurisys.h>
#include <aurisys_controller.h>
#include <aurisys_lib_manager.h>
#include <aurisys_utility.h>
#include <aurisys_config.h>

#include <audio_task_aurisys.h>
#ifdef MCPS_PROF_SUPPORT
#include "mcps_prof.h"
#endif
#include <mpu_mtk.h>

/*==============================================================================
 *                     CCIF3 Registers
 *============================================================================*/

#define CCIF_BASE_AP (0x1023E000)   //AP->MD
#define CCIF_AP_BUSY   (CCIF_BASE_AP + 0x0004)
#define CCIF_AP_TCHNUM (CCIF_BASE_AP + 0x000C)
#define CCIF_AP_RCHNUM (CCIF_BASE_AP + 0x0010)
#define CCIF_AP_ACK (CCIF_BASE_AP + 0x0014)

#define CCIF_BASE_MD (0x1023F000)   //MD-> AP
#define CCIF_MD_BUSY   (CCIF_BASE_MD + 0x0004)
#define CCIF_MD_TCHNUM (CCIF_BASE_MD + 0x000C)
#define CCIF_MD_RCHNUM (CCIF_BASE_MD + 0x0010)
#define CCIF_MD_ACK (CCIF_BASE_MD + 0x0014)

/*==============================================================================
 *                     Band Info
 *============================================================================*/
#define SPH_BAND_MODE_NB 0
#define SPH_BAND_MODE_WB 1
#define SPH_BAND_MODE_SWB 2
#define SPH_BAND_MODE_FB 3
#define SPH_BAND_MODE_UNDEF 4

/*==============================================================================
 *                     EMI Physical Address and Offsets
 *============================================================================*/
#define KB 1024 //1024Bytes
#define FB_PCM_BUF_SIZE 1920 // 20*48*2 = 1920
static uint64_t emi_phy_addr;
static uint64_t emi_phy_size;
bool receive_emi_addr = 0;
volatile unsigned long  *p_ul_src1;
volatile unsigned long  *p_ul_src2;
volatile unsigned long  *p_ul_src3;
volatile unsigned long  *p_ul_ref1;
volatile unsigned long  *p_ul_ref2;
volatile unsigned long  *p_dl_src;
volatile unsigned long  *p_ul_dst;
volatile unsigned long  *p_dl_dst;
volatile unsigned long  *p_band_info;
volatile unsigned long  *p_rchnum;

volatile unsigned long  *p_valid;
/*==============================================================================
 *                     Types
 *============================================================================*/

enum tty_mode_t {
    AUD_TTY_OFF  = 0,
    AUD_TTY_FULL = 1,
    AUD_TTY_VCO  = 2,
    AUD_TTY_HCO  = 4,
    AUD_TTY_ERR  = -1
};

enum pcm_dump_t {
    UL_IN1,
    UL_IN2,
    UL_IN3,
    UL_REF1,
    UL_REF2,
    UL_OUT,
    DL_IN,
    DL_OUT
};

enum valid_bitmask_t {
    VALID_BITMASK_MIC1=1<<0,
    VALID_BITMASK_MIC2=1<<1,
    VALID_BITMASK_MIC3=1<<2,
    VALID_BITMASK_REF1=1<<3,
    VALID_BITMASK_REF2=1<<4,
    VALID_BITMASK_DL=1<<5,
};

/*==============================================================================
 *                     MACRO
 *============================================================================*/

#define LOCAL_TASK_STACK_SIZE (256*8)
#define LOCAL_TASK_NAME "call"
#define LOCAL_TASK_PRIORITY (3)

#define MAX_MSG_QUEUE_SIZE (8)

/* TODO: calculate it */
// #define FRAME_BUF_SIZE   (320)
#define MAX_FRAME_BUF_SIZE   (1920)
#define MAX_PARAM_SIZE   (8192)

#define EMI_PHY_SIZE     (0x8000)
/*==============================================================================
 *                     private global members
 *============================================================================*/
#if MCPS_PROF_SUPPORT
static struct mcps_profile_t sp_mcps_profile;
#endif

static struct alock_t *g_phone_call_task_lock;
static uint8_t            dump_pcm_flag;

typedef struct pcm_dump_ul_t {
    char *ul_in_ch1;
    char *ul_in_ch2;
    char *ul_in_ch3;
    char *aec_in1;
    char *aec_in2;
    char *ul_out;
    uint32_t frame_buf_size;
} pcm_dump_ul_t;

typedef struct pcm_dump_dl_t {
    char *dl_in;
    char *dl_out;
    uint32_t frame_buf_size;
} pcm_dump_dl_t;

pcm_dump_ul_t pcm_dump_ul;
pcm_dump_dl_t pcm_dump_dl;

/*==============================================================================
 *                     derived functions - declaration
 *============================================================================*/

static void     task_phone_call_constructor(struct AudioTask *this);
static void     task_phone_call_destructor(struct AudioTask *this);

static void     task_phone_call_create_task_loop(struct AudioTask *this);
static void     task_phone_call_destroy_task_loop(struct AudioTask *this);

static status_t task_phone_call_recv_message(struct AudioTask *this, struct ipi_msg_t *ipi_msg);
static void     task_phone_call_irq_hanlder(struct AudioTask *this, uint32_t irq_type);
static void     task_phone_call_hal_reboot_cbk(struct AudioTask *this, const uint8_t hal_type);


/*==============================================================================
 *                     private functions - declaration
 *============================================================================*/

static void     task_phone_call_parsing_message(
    struct AudioTask *this,
    ipi_msg_t *ipi_msg);

static void     task_phone_call_task_loop(void *pvParameters);

static status_t task_phone_call_init(struct AudioTask *this);
static status_t task_phone_call_working(
    struct AudioTask *this,
    ipi_msg_t *ipi_msg);

static status_t task_phone_call_deinit(struct AudioTask *this);

#ifdef OPENDSP_DUMP_PARAM
static void dump_param(uint32_t data_size, void *param);
#endif

typedef enum {
    MODEM_UL_DATA     = 0x1,
    MODEM_DL_DATA     = 0x2,
    SCP_UL_ACK        = 0x4,
    SCP_DL_ACK        = 0x8,
    MODEM_UL_HANDOVER = 0x10,
    MODEM_DL_HANDOVER = 0x20,
    BYPASS_DATA       = 0xFF,
} modem_data_handshake_t;



static audio_buf_t ul_buf_in;
static audio_buf_t ul_buf_out;
static audio_buf_t aec_buf_in;
static audio_buf_t dl_buf_in;
static audio_buf_t dl_buf_out;


static int16_t updated_ul_digital_gain;
static int16_t updated_ul_analog_gain_ref;
static int16_t updated_dl_digital_gain;
static int16_t updated_dl_analog_gain_ref;
static uint32_t ref_channel_num;

static uint8_t enh_on_ul;
static uint8_t enh_on_dl;

static uint8_t enh_on_tty_ul;
static uint8_t enh_on_tty_dl;

static uint32_t frame_buf_size;
static uint32_t ul_in_buf_size;
static uint32_t ul_out_buf_size;
static uint32_t aec_buf_size;
static uint32_t dl_in_buf_size;
static uint32_t dl_out_buf_size;

static uint32_t mask_adsp2md_irq;

// unsigned char dl_in[FRAME_BUF_SIZE];
// unsigned char dl_out[FRAME_BUF_SIZE];

#define WAKELOCK
#ifdef WAKELOCK
wakelock_t phone_call_monitor_wakelock;
#endif

//default sampling rate
static uint16_t mModemDspSampleRate[] = {8000, 16000, 32000, 48000, 0};
static uint16_t mFrameBufferSize[] = {320, 640, 1280, 1920, 0};
static uint16_t mDspBandInfo = 0;
static uint16_t mPrevDspBandInfo = 0;
static uint32_t mInputDeviceInfo = 0;
static uint32_t mOutputDeviceInfo = 0;




struct audio_aurisys_handle_t *aurisys_handle;
struct data_buf_t param_list;



/*==============================================================================
 *                     class new/construct/destruct functions
 *============================================================================*/

AudioTask *task_phone_call_new() {

    AUD_LOG_V("%s(+)\n", __func__);

    /* alloc object here */
    AudioTask *task = (AudioTask *)kal_pvPortMalloc(sizeof(AudioTask));
    if (task == NULL) {
        AUD_LOG_E("%s(), kal_pvPortMalloc fail!!\n", __func__);
        return NULL;
    }
    memset(task, 0, sizeof(AudioTask));


    /* only assign methods, but not class members here */
    task->constructor       = task_phone_call_constructor;
    task->destructor        = task_phone_call_destructor;

    task->create_task_loop  = task_phone_call_create_task_loop;
    task->destroy_task_loop = task_phone_call_destroy_task_loop;

    task->recv_message      = task_phone_call_recv_message;

    task->irq_hanlder       = task_phone_call_irq_hanlder;

    task->hal_reboot_cbk    = task_phone_call_hal_reboot_cbk;

    aurisys_handle = (struct audio_aurisys_handle_t *)
                     AUDIO_MALLOC(sizeof(struct audio_aurisys_handle_t));
    NEW_ALOCK(aurisys_handle->mAurisysLibManagerLock);
    NEW_ALOCK(g_phone_call_task_lock);

    param_list.memory_size = 0;
    param_list.data_size = 0;
    param_list.p_buffer = NULL;

    AUD_LOG_V("%s(-), task = %p\n", __func__, task);
    return task;
}


void task_phone_call_delete(AudioTask *task) {
    AUD_LOG_V("%s(+), task_scene = %d\n", __func__, task->scene);

    if (task == NULL) {
        AUD_LOG_E("%s(), task is NULL!!\n", __func__);
        return;
    }

    FREE_ALOCK(g_phone_call_task_lock);
    FREE_ALOCK(aurisys_handle->mAurisysLibManagerLock);
    AUDIO_FREE_POINTER(aurisys_handle);

    vPortFree(task);

    AUD_LOG_V("%s(-)\n", __func__);
}

static void task_phone_call_constructor(struct AudioTask *this) {
    AUD_LOG_D("%s(+), task_scene = %d\n", __func__, this->scene);

    AUD_ASSERT(this != NULL);

    /* assign initial value for class members & alloc private memory here */
    this->scene = TASK_SCENE_PHONE_CALL;
    this->state = AUDIO_TASK_IDLE;


#ifdef WAKELOCK
    wake_lock_init(&phone_call_monitor_wakelock, "callw");
#endif

    /* queue */
    this->queue_idx = 0;
    this->num_queue_element = 0;

    this->msg_array = (ipi_msg_t *)kal_pvPortMalloc(
                          MAX_MSG_QUEUE_SIZE * sizeof(ipi_msg_t));
    AUD_ASSERT(this->msg_array != NULL);
    memset(this->msg_array, 0, MAX_MSG_QUEUE_SIZE * sizeof(ipi_msg_t));

    this->msg_idx_queue = xQueueCreate(MAX_MSG_QUEUE_SIZE, sizeof(uint8_t));
    AUD_ASSERT(this->msg_idx_queue != NULL);


    /* speech enhancement parameters */
    dump_pcm_flag = 0;

    enh_on_ul = 1;
    enh_on_dl = 1;
    enh_on_tty_ul = 1;
    enh_on_tty_dl = 1;
    updated_ul_digital_gain = 0;
    updated_ul_analog_gain_ref = 0;
    updated_dl_digital_gain = 0;
    updated_dl_analog_gain_ref = 0;


    AUD_LOG_V("%s(-), task_scene = %d\n", __func__, this->scene);
}


static void task_phone_call_destructor(struct AudioTask *this) {
    if (this == NULL) {
        AUD_LOG_E("%s(), this is NULL!!\n", __func__);
        return;
    }

    AUD_LOG_D("%s(+), task_scene = %d\n", __func__, this->scene);

    AUD_ASSERT(this->state == AUDIO_TASK_IDLE);

    /* dealloc private memory & dealloc object here */
    if (this->msg_array != NULL) {
        kal_vPortFree(this->msg_array);
    }
    if (this->msg_idx_queue != NULL) {
        vQueueDelete(this->msg_idx_queue);
    }
#ifdef WAKELOCK
    //wake_lock_deinit(&phone_call_monitor_wakelock);
#endif
    AUD_LOG_V("%s(-), task_scene = %d\n", __func__, this->scene);
}


static void task_phone_call_create_task_loop(struct AudioTask *this) {
    /* Note: you can also bypass this function,
             and do kal_xTaskCreate until you really need it.
             Ex: create task after you do get the enable message. */

    BaseType_t xReturn = pdFAIL;

    xReturn = kal_xTaskCreate(
                  task_phone_call_task_loop,
                  LOCAL_TASK_NAME,
                  LOCAL_TASK_STACK_SIZE,
                  (void *)this,
                  LOCAL_TASK_PRIORITY,
                  &this->freertos_task);


    AUD_ASSERT(xReturn == pdPASS);
    AUD_ASSERT(this->freertos_task);
}


static void task_phone_call_destroy_task_loop(struct AudioTask *this) {
#if (INCLUDE_vTaskDelete == 1)
    if (this->freertos_task) {
        vTaskDelete(this->freertos_task);
    }
#endif
}


static uint8_t get_queue_idx(struct AudioTask *this) {
    uint8_t queue_idx = this->queue_idx;

    this->queue_idx++;
    if (this->queue_idx == MAX_MSG_QUEUE_SIZE) {
        this->queue_idx = 0;
    }

    return queue_idx;
}

inline void mask_modem_irq() {
    mask_irq(CCIF3_IRQn);
}


inline void unmask_modem_irq() {
    unmask_irq(CCIF3_IRQn);
}

static void print_time_from_param(const uint32_t param)
{
    /* param[24:31] => hour
     * param[16:23] => minute
     * param[8:15]  => second
     * param[0:7]   => enable */

    uint8_t time_value_h = (param & 0xFF000000) >> 24;
    uint8_t time_value_m = (param & 0x00FF0000) >> 16;
    uint8_t time_value_s = (param & 0x0000FF00) >> 8;

    AUD_LOG_D("HAL Time %02d:%02d:%02d\n", time_value_h, time_value_m, time_value_s);
}


static status_t task_phone_call_recv_message(
    struct AudioTask *this,
    struct ipi_msg_t *ipi_msg) {
    uint8_t queue_idx = 0;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    int i = 0;

    AUD_LOG_D("%s()\n", __func__);

    mask_modem_irq(); /* prevent nested interrupt: MD_IRQn has higher prior */
    this->num_queue_element++;
    queue_idx = get_queue_idx(this);

    if (this->msg_array[queue_idx].magic != 0) {
        AUD_LOG_E("queue_idx = %d\n", queue_idx);

        for (i = 0; i < MAX_MSG_QUEUE_SIZE; i++) {
            AUD_LOG_E("[%d] id = 0x%x\n", i, this->msg_array[i].msg_id);
        }
        AUD_ASSERT(this->msg_array[queue_idx].magic == 0); /* item is clean */
    }
    memcpy(&this->msg_array[queue_idx], ipi_msg, sizeof(ipi_msg_t));
    unmask_modem_irq();

    xQueueSendToBackFromISR(
        this->msg_idx_queue,
        &queue_idx,
        &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    return NO_ERROR;
}

static uint64_t callIrqReadStart;
static uint64_t callIrqReadStop;
static uint64_t callIrqWriteStart;
static uint64_t callIrqWriteStop;

#define OSCYC2US(cyc) (((cyc) * 315) >> 12)

static void task_phone_call_irq_hanlder(
    struct AudioTask *this,
    uint32_t irq_type) {

    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    uint8_t queue_idx = 0;
    ipi_msg_t *ipi_msg = NULL;
    int i = 0;
    uint32_t rchnum = 0;
    uint16_t rchnum_emi = 0;
    bool isReadFromEmi = false;

    callIrqReadStart = timer_get_global_timer_tick();
#if CALL_CH_NUM_FROM_EMI
    if (receive_emi_addr) {
        hal_cache_region_operation (HAL_CACHE_TYPE_DATA , HAL_CACHE_OPERATION_INVALIDATE, (void *)p_rchnum, 128);

        if(((*p_rchnum) & 0xffff0000) == 0x67790000) {
            isReadFromEmi = true;
            rchnum_emi = (uint16_t)(*p_rchnum);
        }
    }
#endif
    if(false == isReadFromEmi) {
        rchnum = DRV_Reg32(CCIF_AP_RCHNUM);
    }
    callIrqReadStop = timer_get_global_timer_tick();
    AUD_LOG_D("+%s() %d:%d; %d\n", __func__, rchnum, rchnum_emi, isReadFromEmi);

    if ((NULL == this) || (AUDIO_TASK_WORKING != this->state)) {
        callIrqWriteStart = timer_get_global_timer_tick();
        DRV_WriteReg32(CCIF_AP_ACK, (rchnum | rchnum_emi));
        callIrqWriteStop = timer_get_global_timer_tick();
        AUD_LOG_D("+%s() skipIRQ, read time:%lld, write time:%lld\n", __func__, OSCYC2US((callIrqReadStop-callIrqReadStart)), OSCYC2US((callIrqWriteStop-callIrqWriteStart)));
        return;
    }

    /* put DL/UL data into queue to process */
    this->num_queue_element++;
    queue_idx = get_queue_idx(this);
    ipi_msg = &this->msg_array[queue_idx];

    if (ipi_msg->magic != 0) {
        AUD_LOG_E("queue_idx = %d\n", queue_idx);

        for (i = 0; i < MAX_MSG_QUEUE_SIZE; i++) {
            AUD_LOG_E("[%d] id = 0x%x\n", i, this->msg_array[i].msg_id);
        }
        AUD_ASSERT(ipi_msg->magic == 0); /* item is clean */
    }

    ipi_msg->magic      = IPI_MSG_MAGIC_NUMBER;
    ipi_msg->task_scene = TASK_SCENE_PHONE_CALL;
    ipi_msg->source_layer  = AUDIO_IPI_LAYER_FROM_DSP;
    ipi_msg->target_layer  = AUDIO_IPI_LAYER_TO_DSP;
    ipi_msg->data_type  = AUDIO_IPI_MSG_ONLY;
    ipi_msg->ack_type   = AUDIO_IPI_MSG_BYPASS_ACK;
    ipi_msg->msg_id     = IPI_MSG_M2D_CALL_DATA_READY;
    ipi_msg->param1     = 0;
    ipi_msg->param2     = (rchnum | rchnum_emi);

    callIrqWriteStart = timer_get_global_timer_tick();
    DRV_WriteReg32(CCIF_AP_ACK, (rchnum | rchnum_emi));
    callIrqWriteStop = timer_get_global_timer_tick();

    AUD_LOG_D("+%s() read time:%lld, write time:%lld \n", __func__, OSCYC2US((callIrqReadStop-callIrqReadStart)), OSCYC2US((callIrqWriteStop-callIrqWriteStart)));

    xQueueSendToBackFromISR(
        this->msg_idx_queue,
        &queue_idx,
        &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}


static void task_phone_call_hal_reboot_cbk(struct AudioTask *this, const uint8_t hal_type) {
    if (this == NULL || hal_type != 0) {
        AUD_LOG_W("%s(), this %p type %d fail!!\n", __func__, this, hal_type);
        return;
    }

    LOCK_ALOCK_MS(g_phone_call_task_lock, 1000);
    AUD_LOG_D("%s(), scene %d", __func__, this->scene);

    /* close adsp speech driver */
    if (this->state == AUDIO_TASK_WORKING) {
        this->state = AUDIO_TASK_DEINIT;
        task_phone_call_deinit(this);
        this->state = AUDIO_TASK_IDLE;
    }

    /* close aurisys */
    if (aurisys_handle->manager) {
        DestroyAurisysLibManager(aurisys_handle, TASK_SCENE_PHONE_CALL);
        AUDIO_FREE_POINTER(aurisys_handle->dsp_config);
        AUDIO_FREE_POINTER(param_list.p_buffer);
        param_list.memory_size = 0;
        param_list.data_size = 0;
        param_list.p_buffer = NULL;
    }

    UNLOCK_ALOCK(g_phone_call_task_lock);
}


static void task_phone_call_task_loop(void *pvParameters) {
    AUD_LOG_D("+%s()\n", __func__); // address = 2349072384

    AudioTask *this = (AudioTask *)pvParameters;
    uint8_t local_queue_idx = 0xFF;

    while (1) {
        xQueueReceive(this->msg_idx_queue, &local_queue_idx, portMAX_DELAY);
        LOCK_ALOCK_MS(g_phone_call_task_lock, 1000);
        task_phone_call_parsing_message(this, &this->msg_array[local_queue_idx]);
        UNLOCK_ALOCK(g_phone_call_task_lock);
    }

}


static void update_cfg_by_sample_rare(
    struct aurisys_dsp_config_t *dsp_config,
    const uint32_t sample_rate)
{
    struct aurisys_lib_manager_config_t *manager_config = NULL;
    struct arsi_task_config_t *task_config = NULL;

    if (!dsp_config || !sample_rate) {
        AUD_ASSERT(0);
    }

    /* prefer config */
    manager_config = &dsp_config->manager_config;
    manager_config->sample_rate = sample_rate;

    /* task config */
    task_config = &manager_config->task_config;
    task_config->input_device_info.sample_rate  = sample_rate;
    task_config->output_device_info.sample_rate = sample_rate;
    task_config->max_input_device_sample_rate   = sample_rate;
    task_config->max_output_device_sample_rate  = sample_rate;
    //dump_task_config(task_config);

    /* audio buffer config */
    dsp_config->attribute[DATA_BUF_UPLINK_IN].sample_rate    = sample_rate;
    dsp_config->attribute[DATA_BUF_UPLINK_OUT].sample_rate   = sample_rate;
    dsp_config->attribute[DATA_BUF_ECHO_REF].sample_rate     = sample_rate;
    dsp_config->attribute[DATA_BUF_DOWNLINK_IN].sample_rate  = sample_rate;
    dsp_config->attribute[DATA_BUF_DOWNLINK_OUT].sample_rate = sample_rate;
}


void task_phone_call_parsing_message(struct AudioTask *this, ipi_msg_t *ipi_msg) {
    void *data_buf = NULL;
    uint32_t data_size = 0;

    switch (ipi_msg -> msg_id) {

    case IPI_MSG_A2D_GET_EMI_ADDRESS: {
        if (!receive_emi_addr) {
            emi_phy_addr = *((uint64_t *)ipi_msg->payload);
            emi_phy_size = *((uint64_t *)ipi_msg->payload+1);
            AUD_LOG_D("emi_phy_addr: = %llx %lld, size=%llx\n", emi_phy_addr, emi_phy_addr, emi_phy_size);
            p_ul_src1 = (volatile unsigned long *)((unsigned long)emi_phy_addr + 3 * KB);
            p_ul_src2 = (volatile unsigned long *)((unsigned long)emi_phy_addr + 3 * KB + FB_PCM_BUF_SIZE);
            p_ul_src3 = (volatile unsigned long *)((unsigned long)emi_phy_addr + 3 * KB + FB_PCM_BUF_SIZE * 2);
            p_ul_ref1 = (volatile unsigned long *)((unsigned long)emi_phy_addr + 3 * KB + FB_PCM_BUF_SIZE * 3);

            p_ul_ref2 = (volatile unsigned long *)((unsigned long)emi_phy_addr +  3 * KB + FB_PCM_BUF_SIZE * 4);
            p_dl_src = (volatile unsigned long *)((unsigned long)emi_phy_addr +  3 * KB + FB_PCM_BUF_SIZE * 5);

            p_band_info = (volatile unsigned long *)((unsigned long)emi_phy_addr + 3 * KB + FB_PCM_BUF_SIZE * 6);
            p_rchnum = (volatile unsigned long *)((unsigned long)emi_phy_addr + 3 * KB + FB_PCM_BUF_SIZE * 6 + 128);

            p_valid = (volatile unsigned long *)((unsigned long)emi_phy_addr + 3 * KB + FB_PCM_BUF_SIZE * 6 + 128);

            p_ul_dst = (volatile unsigned long *)((unsigned long)emi_phy_addr + 19 * KB);
            p_dl_dst = (volatile unsigned long *)((unsigned long)emi_phy_addr + 19 * KB + FB_PCM_BUF_SIZE * 1);

            disable_mpu_region(MPU_ID_MD_SHARED_RW);
            mpu_setup_region_address(MPU_ID_MD_SHARED_RW, emi_phy_addr, emi_phy_size);
            enable_mpu_region(MPU_ID_MD_SHARED_RW);

        }
        receive_emi_addr = 1;
        break;
    }
    case IPI_MSG_A2D_UL_GAIN: {
        AUD_LOG_D("UL gain 0x%x, 0x%x\n", ipi_msg->param2, ipi_msg->param1);
        updated_ul_digital_gain = ipi_msg->param1;
        updated_ul_analog_gain_ref = ipi_msg->param2;
        aurisys_set_ul_digital_gain(aurisys_handle->manager,
            updated_ul_analog_gain_ref,
            updated_ul_digital_gain);
        break;
    }
    case IPI_MSG_A2D_DL_GAIN: {
        AUD_LOG_D("DL gain 0x%x, 0x%x\n", ipi_msg->param2, ipi_msg->param1);
        updated_dl_digital_gain = ipi_msg->param1;
        updated_dl_analog_gain_ref = ipi_msg->param2;
        aurisys_set_dl_digital_gain(aurisys_handle->manager,
            updated_dl_analog_gain_ref,
            updated_dl_digital_gain);
        break;
    }



    case IPI_MSG_M2D_CALL_DATA_READY: {
        if (AUDIO_TASK_WORKING == this->state) {
            if (!receive_emi_addr) {
                AUD_LOG_D("%s(): not received emi addr\n", __func__);
                AUD_ASSERT(0);
            }
            task_phone_call_working(this, ipi_msg);
        }
        break;
    }

    case IPI_MSG_A2D_TASK_CFG:
    case IPI_MSG_A2D_LIB_CFG:
    case IPI_MSG_A2D_SPH_PARAM:
        break;

    case IPI_MSG_A2D_SPH_ON: {
        if ((ipi_msg->param1 & 0xFF) == 1) {  //Speech On
            AUD_LOG_D("speech on\n");
            print_time_from_param(ipi_msg->param1);
            this->state = AUDIO_TASK_INIT;
            task_phone_call_init(this);
            dvfs_register_feature(VOICE_CALL_FEATURE_ID);
#if MCPS_PROF_SUPPORT
            mcps_prof_init(&sp_mcps_profile);
#endif
            mask_adsp2md_irq = 0;
            this->state = AUDIO_TASK_WORKING;

        } else if ((ipi_msg->param1 & 0xFF) == 0) { //Speech Off
            AUD_LOG_D("speech off\n");
            print_time_from_param(ipi_msg->param1);
            if (AUDIO_TASK_WORKING == this->state) {
                this->state = AUDIO_TASK_DEINIT;
                dvfs_deregister_feature(VOICE_CALL_FEATURE_ID);
#if MCPS_PROF_SUPPORT
            mcps_prof_show(&sp_mcps_profile);
#endif
                task_phone_call_deinit(this);
                this->state = AUDIO_TASK_IDLE;
            } else {
                AUD_LOG_E("speech off under state %d\n", this->state);
            }
        } else {
            AUD_LOG_E("IPI_MSG_A2D_SPH_ON invalid param1: 0x%x\n", ipi_msg->param1);
        }
        break;
    }

    case IPI_MSG_A2D_MASK_MD_IRQ: {
        AUD_LOG_D("speech closing\n");
        print_time_from_param(ipi_msg->param1);
        mask_adsp2md_irq = 1;
        break;
    }

    case IPI_MSG_A2D_TTY_ON: {
        if (ipi_msg->param1 == 1) {
            AUD_LOG_V("tty on\n");
            switch (ipi_msg->param2) { // tty_mode_t
            case AUD_TTY_FULL: {
                enh_on_tty_ul = 0;
                enh_on_tty_dl = 0;
                break;
            }
            case AUD_TTY_VCO: {
                enh_on_tty_ul = 1;
                enh_on_tty_dl = 0;
                break;
            }
            case AUD_TTY_HCO: {
                enh_on_tty_ul = 0;
                enh_on_tty_dl = 1;
                break;
            }
            default: {
                AUD_LOG_W("TTY_ON: mode %d err\n", ipi_msg->param2);
                enh_on_tty_ul = 1;
                enh_on_tty_dl = 1;
                break;
            }
            }
        } else if (ipi_msg->param1 == 0) {
            AUD_LOG_V("tty off\n");
            switch (ipi_msg->param2) { // tty_mode_t
            case AUD_TTY_OFF: {
                enh_on_tty_ul = 1;
                enh_on_tty_dl = 1;
                break;
            }
            default: {
                AUD_LOG_W("TTY_OFF: mode %d err\n", ipi_msg->param2);
                enh_on_tty_ul = 1;
                enh_on_tty_dl = 1;
                break;
            }
            }
        }

        // apply enhancement state
        aurisys_set_ul_enhance(aurisys_handle->manager, enh_on_ul & enh_on_tty_ul);
        aurisys_set_dl_enhance(aurisys_handle->manager, enh_on_dl & enh_on_tty_dl);
        break;
    }
    case IPI_MSG_A2D_UL_MUTE_ON: {
        AUD_LOG_D("UL mute on %d\n", ipi_msg->param1);
        aurisys_set_ul_mute(aurisys_handle->manager, ipi_msg->param1);
        break;
    }
    case IPI_MSG_A2D_DL_MUTE_ON: {
        AUD_LOG_D("DL mute on %d\n", ipi_msg->param1);
        aurisys_set_dl_mute(aurisys_handle->manager, ipi_msg->param1);
        break;
    }
    case IPI_MSG_A2D_UL_ENHANCE_ON: {
        AUD_LOG_D("UL enh on %d\n", ipi_msg->param1);
        enh_on_ul = ipi_msg->param1;
        aurisys_set_ul_enhance(aurisys_handle->manager, enh_on_ul & enh_on_tty_ul);
        break;
    }
    case IPI_MSG_A2D_DL_ENHANCE_ON: {
        AUD_LOG_D("DL enh on %d\n", ipi_msg->param1);
        enh_on_dl = ipi_msg->param1;
        aurisys_set_dl_enhance(aurisys_handle->manager, enh_on_dl & enh_on_tty_dl);
        break;
    }

    case IPI_MSG_A2D_BT_NREC_ON:
    case IPI_MSG_A2D_SET_ADDR_VALUE:
    case IPI_MSG_A2D_GET_ADDR_VALUE:
    case IPI_MSG_A2D_SET_KEY_VALUE:
    case IPI_MSG_A2D_GET_KEY_VALUE:
        break;
    case IPI_MSG_A2D_PCM_DUMP_ON: {
        dump_pcm_flag = ipi_msg->param1;
        AUD_LOG_D("dump_pcm_flag: %d\n", dump_pcm_flag);
        break;
    }
    case IPI_MSG_A2D_LIB_LOG_ON: {
        break;
    }

    case IPI_MSG_A2D_AURISYS_INIT: {
        audio_get_dma_from_msg(ipi_msg, &data_buf, &data_size);
        if (data_size < sizeof(struct aurisys_dsp_config_t)) {
            AUD_WARNING("dsp config fail");
            AUDIO_FREE_POINTER(data_buf);
            break;
        }
        if (data_size == sizeof(struct aurisys_dsp_config_t)) {
            AUD_WARNING("param fail");
            AUDIO_FREE_POINTER(data_buf);
            break;
        }

        /* config */
        AUDIO_ALLOC_STRUCT(struct aurisys_dsp_config_t, aurisys_handle->dsp_config);
        memcpy(aurisys_handle->dsp_config,
               data_buf,
               sizeof(struct aurisys_dsp_config_t));
        update_cfg_by_sample_rare(
            aurisys_handle->dsp_config,
            mModemDspSampleRate[mDspBandInfo]);
        mInputDeviceInfo  = aurisys_handle->dsp_config->manager_config.task_config.input_device_info.devices;
        mOutputDeviceInfo = aurisys_handle->dsp_config->manager_config.task_config.output_device_info.devices;

        /* param */
        param_list.data_size = data_size - sizeof(struct aurisys_dsp_config_t);
        dynamic_change_buf_size(
            &param_list.p_buffer,
            &param_list.memory_size,
            param_list.data_size);
        memcpy(param_list.p_buffer,
               ((uint8_t *)data_buf) + sizeof(struct aurisys_dsp_config_t),
               param_list.data_size);
        AUD_LOG_D("param_list.data_size: %u\n", param_list.data_size);

        CreateAurisysLibManager(aurisys_handle, &param_list, TASK_SCENE_PHONE_CALL);
        aurisys_set_ul_digital_gain(aurisys_handle->manager,
            updated_ul_analog_gain_ref,
            updated_ul_digital_gain);
        aurisys_set_dl_digital_gain(aurisys_handle->manager,
            updated_dl_analog_gain_ref,
            updated_dl_digital_gain);
        AUDIO_FREE_POINTER(data_buf);
        break;
    }
    case IPI_MSG_A2D_AURISYS_ROUTING: {
        audio_get_dma_from_msg(ipi_msg, &data_buf, &data_size);
        if (data_size < sizeof(struct aurisys_dsp_config_t)) {
            AUD_WARNING("dsp config fail");
            AUDIO_FREE_POINTER(data_buf);
            break;
        }
        if (data_size == sizeof(struct aurisys_dsp_config_t)) {
            AUD_WARNING("param fail");
            AUDIO_FREE_POINTER(data_buf);
            break;
        }

        /* config */
        if (!aurisys_handle->dsp_config) {
            AUD_LOG_W("config null!!");
            AUDIO_FREE_POINTER(data_buf);
            break;
        }
        DestroyAurisysLibManager(aurisys_handle, TASK_SCENE_PHONE_CALL);
        memcpy(aurisys_handle->dsp_config,
               data_buf,
               sizeof(struct aurisys_dsp_config_t));
        update_cfg_by_sample_rare(
            aurisys_handle->dsp_config,
            mModemDspSampleRate[mDspBandInfo]);
        mInputDeviceInfo  = aurisys_handle->dsp_config->manager_config.task_config.input_device_info.devices;
        mOutputDeviceInfo = aurisys_handle->dsp_config->manager_config.task_config.output_device_info.devices;

        /* param */
        param_list.data_size = data_size - sizeof(struct aurisys_dsp_config_t);
        dynamic_change_buf_size(
            &param_list.p_buffer,
            &param_list.memory_size,
            param_list.data_size);
        memcpy(param_list.p_buffer,
               ((uint8_t *)data_buf) + sizeof(struct aurisys_dsp_config_t),
               param_list.data_size);
        AUD_LOG_D("param_list.data_size: %u\n", param_list.data_size);

        CreateAurisysLibManager(aurisys_handle, &param_list, TASK_SCENE_PHONE_CALL);
        aurisys_set_ul_digital_gain(aurisys_handle->manager,
            updated_ul_analog_gain_ref,
            updated_ul_digital_gain);
        aurisys_set_dl_digital_gain(aurisys_handle->manager,
            updated_dl_analog_gain_ref,
            updated_dl_digital_gain);
        AUDIO_FREE_POINTER(data_buf);
        break;
    }
    case IPI_MSG_A2D_AURISYS_PARAM: {
        audio_get_dma_from_msg(ipi_msg, &data_buf, &data_size);
        ipi_msg->param1 = UpdateAurisysLibParam(
            aurisys_handle,
            data_buf,
            data_size);
        AUDIO_FREE_POINTER(data_buf);
        break;
    }
    case IPI_MSG_A2D_AURISYS_PARAM_LIST: {
        audio_get_dma_from_msg(ipi_msg, &data_buf, &data_size);

        /* param */
        param_list.data_size = data_size;
        dynamic_change_buf_size(
            &param_list.p_buffer,
            &param_list.memory_size,
            param_list.data_size);

        memcpy(param_list.p_buffer,
               ((uint8_t *)data_buf),
               param_list.data_size);

        ipi_msg->param1 = ApplyAurisysLibParamList(
            aurisys_handle,
            &param_list,
            TASK_SCENE_PHONE_CALL);
        AUDIO_FREE_POINTER(data_buf);
        break;
    }
    case IPI_MSG_A2D_AURISYS_DEINIT: {
        if ( NULL == aurisys_handle->manager ) {
            AUD_LOG_D("IPI_MSG_A2D_AURISYS_DEINIT without init\n");
        } else {
            DestroyAurisysLibManager(aurisys_handle, TASK_SCENE_PHONE_CALL);
            AUDIO_FREE_POINTER(aurisys_handle->dsp_config);
            AUDIO_FREE_POINTER(param_list.p_buffer);
            param_list.memory_size = 0;
            param_list.data_size = 0;
            param_list.p_buffer = NULL;
        }
        break;
    }
    default:
        break;
    }

    /* send ack back if need */
    audio_send_ipi_msg_ack_back(ipi_msg);

    memset(ipi_msg, 0, sizeof(ipi_msg_t));
    this->num_queue_element--;

}



static status_t task_phone_call_init(struct AudioTask *this) {
    AUD_LOG_D("task_phone_call_init\n");
    /* debug dump */

    struct arsi_task_config_t *task_config = NULL;

    if (aurisys_handle == NULL) {
        AUD_ASSERT(aurisys_handle != NULL);
        return NO_INIT;
    }
    task_config = &aurisys_handle->dsp_config->manager_config.task_config;

    frame_buf_size = mFrameBufferSize[mDspBandInfo]; //32k = 32*20*2 = 1280 Bytes
    ul_in_buf_size = (frame_buf_size * 2);
    ul_out_buf_size = frame_buf_size;
    aec_buf_size = (frame_buf_size * 2);
    dl_in_buf_size = frame_buf_size;
    dl_out_buf_size = frame_buf_size;

    ref_channel_num = aurisys_handle->dsp_config->attribute[DATA_BUF_ECHO_REF].num_channels;
    AUD_LOG_D("ref.num_channel :%d", ref_channel_num);
    /* init global var */
#ifdef WAKELOCK
    //wake_lock(&phone_call_monitor_wakelock);
#endif

    enh_on_ul = 1;
    enh_on_dl = 1;
    enh_on_tty_ul = 1;
    enh_on_tty_dl = 1;

    /* dump current heap size before malloc */
    AUD_LOG_D("+Heap free/total = %d/%d\n",
              xPortGetFreeHeapSize(), configTOTAL_HEAP_SIZE);

    /* ul_buf_in */
    ul_buf_in.data_buf.memory_size = ul_in_buf_size;
    ul_buf_in.data_buf.data_size = 0;
    ul_buf_in.data_buf.p_buffer = kal_pvPortMalloc(ul_buf_in.data_buf.memory_size);
    AUD_LOG_D("ul_buf_in.data_buf.p_buffer = %p\n",  ul_buf_in.data_buf.p_buffer);

    /* ul_buf_out */
    ul_buf_out.data_buf.memory_size = ul_out_buf_size;
    ul_buf_out.data_buf.data_size = 0;
    ul_buf_out.data_buf.p_buffer = kal_pvPortMalloc(ul_buf_out.data_buf.memory_size);
    AUD_LOG_D("ul_buf_out.data_buf.p_buffer = %p\n", ul_buf_out.data_buf.p_buffer);

    /* aec */
    aec_buf_in.data_buf.memory_size = aec_buf_size;
    aec_buf_in.data_buf.data_size = 0;
    aec_buf_in.data_buf.p_buffer = kal_pvPortMalloc(aec_buf_in.data_buf.memory_size);
    AUD_LOG_D("aec_buf_in.data_buf.p_buffer = %p\n", aec_buf_in.data_buf.p_buffer);

    /* dl_buf_in */
    dl_buf_in.data_buf.memory_size = dl_in_buf_size;
    dl_buf_in.data_buf.data_size = 0;
    dl_buf_in.data_buf.p_buffer = kal_pvPortMalloc(dl_buf_in.data_buf.memory_size);
    AUD_LOG_D("dl_buf_in.data_buf.p_buffer = %p\n",  dl_buf_in.data_buf.p_buffer);

    /* dl_buf_out */
    dl_buf_out.data_buf.memory_size = dl_out_buf_size;
    dl_buf_out.data_buf.data_size = 0;
    dl_buf_out.data_buf.p_buffer = kal_pvPortMalloc(dl_buf_out.data_buf.memory_size);
    AUD_LOG_D("dl_buf_out.data_buf.p_buffer = %p\n", dl_buf_out.data_buf.p_buffer);

    AUD_LOG_D("enh_on_ul & enh_on_tty_ul = %d\n", enh_on_ul & enh_on_tty_ul);
    AUD_LOG_D("enh_on_dl & enh_on_tty_dl = %d\n", enh_on_dl & enh_on_tty_dl);

    /* pcm dump */
    pcm_dump_ul.ul_in_ch1 = (char *)kal_pvPortMalloc(MAX_FRAME_BUF_SIZE);
    pcm_dump_ul.ul_in_ch2 = (char *)kal_pvPortMalloc(MAX_FRAME_BUF_SIZE);
    pcm_dump_ul.aec_in1 = (char *)kal_pvPortMalloc(MAX_FRAME_BUF_SIZE);
    pcm_dump_ul.aec_in2 = (char *)kal_pvPortMalloc(MAX_FRAME_BUF_SIZE);
    pcm_dump_ul.ul_out = (char *)kal_pvPortMalloc(MAX_FRAME_BUF_SIZE);
    pcm_dump_dl.dl_in  = (char *)kal_pvPortMalloc(MAX_FRAME_BUF_SIZE);
    pcm_dump_dl.dl_out = (char *)kal_pvPortMalloc(MAX_FRAME_BUF_SIZE);

    AUD_LOG_D("-Heap free/total = %d/%d\n",
              xPortGetFreeHeapSize(), configTOTAL_HEAP_SIZE);
    AUD_LOG_V("%s(-)\n", __func__);
    return NO_ERROR;
}

static void send_pcm_dump(struct AudioTask *this,
                          ipi_msg_t *ipi_msg, int size, int id, void *buf) {

    audio_send_ipi_msg(ipi_msg, this->scene,
                       AUDIO_IPI_LAYER_TO_HAL, AUDIO_IPI_DMA,
                       AUDIO_IPI_MSG_BYPASS_ACK,
                       IPI_MSG_D2A_PCM_DUMP_DATA_NOTIFY,
                       size,
                       id,
                       buf);
}

static status_t task_phone_call_working(

    struct AudioTask *this,
    ipi_msg_t *ipi_msg) {
    int channel = 0;
    uint16_t mddsp_valid_bit;
    struct arsi_task_config_t *task_config = NULL;
    uint8_t num_channels = 0;

    if (aurisys_handle == NULL) {
        AUD_ASSERT(aurisys_handle != NULL);
        return NO_INIT;
    }
    task_config = &aurisys_handle->dsp_config->manager_config.task_config;
    num_channels = task_config->input_device_info.num_channels;

    /* set_call_band */
    hal_cache_region_operation (HAL_CACHE_TYPE_DATA , HAL_CACHE_OPERATION_INVALIDATE, (void *)p_band_info, 128);
    mDspBandInfo = (uint16_t)(*p_band_info);

    if (mDspBandInfo >= SPH_BAND_MODE_UNDEF) {
        AUD_LOG_E("Invalid mDspBandInfo: %d", mDspBandInfo);
        mDspBandInfo = SPH_BAND_MODE_NB;
    }

    frame_buf_size = mFrameBufferSize[mDspBandInfo];

    AUD_LOG_D("%s(): mDspBandInfo:%d, sample_rate:%d, frame_buf_size:%d\n", __func__, mDspBandInfo, mModemDspSampleRate[mDspBandInfo], frame_buf_size);

    if (mDspBandInfo != mPrevDspBandInfo) {
        AUD_LOG_D("%s(): change band; mDspBandInfo=%d mPrevDspBandInfo=%d\n", __func__, mDspBandInfo, mPrevDspBandInfo);

        task_phone_call_deinit(this);
        DestroyAurisysLibManager(aurisys_handle, TASK_SCENE_PHONE_CALL);
        update_cfg_by_sample_rare(
            aurisys_handle->dsp_config,
            mModemDspSampleRate[mDspBandInfo]);
        CreateAurisysLibManager(aurisys_handle, &param_list, TASK_SCENE_PHONE_CALL);
        aurisys_set_ul_digital_gain(aurisys_handle->manager,
            updated_ul_analog_gain_ref,
            updated_ul_digital_gain);
        aurisys_set_dl_digital_gain(aurisys_handle->manager,
            updated_dl_analog_gain_ref,
            updated_dl_digital_gain);
        task_phone_call_init(this);
        mPrevDspBandInfo = mDspBandInfo;
    }

    channel = ipi_msg -> param2;
    hal_cache_region_operation (HAL_CACHE_TYPE_DATA , HAL_CACHE_OPERATION_INVALIDATE, (void *)p_valid, 128);

    mddsp_valid_bit = (uint16_t)(*p_valid);
    AUD_LOG_D("mddsp_valid_bit =%x", mddsp_valid_bit);
    if (1 == channel) { /* UL ch1 */

        /* invalidate cache before read */
        hal_cache_region_operation (HAL_CACHE_TYPE_DATA , HAL_CACHE_OPERATION_INVALIDATE, (void *)p_ul_src1, FB_PCM_BUF_SIZE*5);
        ul_buf_in.data_buf.data_size = frame_buf_size;

        AUD_LOG_D("working num_channels = %d\n", num_channels);

        if ( NULL == ul_buf_in.data_buf.p_buffer ) {
            AUD_LOG_E("ul_buf_in.data_buf.p_buffer is NULL with mem_size=%d\n", ul_buf_in.data_buf.memory_size);
        } else {
            if (1 == num_channels) {
                memcpy((void *)ul_buf_in.data_buf.p_buffer, (void *)p_ul_src1, frame_buf_size);
            } else if (2 == num_channels) { /* UL ch2 */
                memcpy((void *)ul_buf_in.data_buf.p_buffer, (void *)p_ul_src1, frame_buf_size);
                memcpy((void *)(ul_buf_in.data_buf.p_buffer + frame_buf_size), (void *)p_ul_src2, frame_buf_size);
                ul_buf_in.data_buf.data_size += frame_buf_size;
            } else if (3 == num_channels) { /* UL ch3 */
                memcpy((void *)ul_buf_in.data_buf.p_buffer, (void *)p_ul_src1, frame_buf_size);
                memcpy((void *)(ul_buf_in.data_buf.p_buffer + frame_buf_size), (void *)p_ul_src2, frame_buf_size);
                memcpy((void *)(ul_buf_in.data_buf.p_buffer + 2 * frame_buf_size), (void *)p_ul_src3, frame_buf_size);
                ul_buf_in.data_buf.data_size += (2 * frame_buf_size);
            }
        }

        AUD_LOG_D("ul_buf_in.data_buf.data_size =%d", ul_buf_in.data_buf.data_size);

        /* Echo Ref */
        if( NULL == aec_buf_in.data_buf.p_buffer ) {
            AUD_LOG_E("aec_buf_in.data_buf.p_buffer is NULL with mem_size=%d\n", aec_buf_in.data_buf.memory_size);
        } else {
            memcpy((void *)aec_buf_in.data_buf.p_buffer, (void *)p_ul_ref1, frame_buf_size);
            aec_buf_in.data_buf.data_size = frame_buf_size;
            if (2 == ref_channel_num) {
                if(VALID_BITMASK_REF2 & mddsp_valid_bit) {
                    memcpy((void *)(aec_buf_in.data_buf.p_buffer + frame_buf_size), (void *)p_ul_ref2, frame_buf_size);
                } else {
                    AUD_LOG_D("copy ref1 to ref2");
                    memcpy((void *)(aec_buf_in.data_buf.p_buffer + frame_buf_size), (void *)p_ul_ref1, frame_buf_size);
                }
                aec_buf_in.data_buf.data_size += frame_buf_size;
            } else if(ref_channel_num > 2) {
                AUD_LOG_E("ref.num_channels errors\n");
            }
        }

#if MCPS_PROF_SUPPORT
        mcps_prof_start(&sp_mcps_profile);
#endif

        audio_pool_buf_copy_from_linear(
            aurisys_handle->mAudioPoolBufUlIn,
            ul_buf_in.data_buf.p_buffer,
            ul_buf_in.data_buf.data_size);

        audio_pool_buf_copy_from_linear(
            aurisys_handle->mAudioPoolBufUlAec,
            aec_buf_in.data_buf.p_buffer,
            aec_buf_in.data_buf.data_size);

        aurisys_process_ul_only(
            aurisys_handle->manager,
            aurisys_handle->mAudioPoolBufUlIn,
            aurisys_handle->mAudioPoolBufUlOut,
            aurisys_handle->mAudioPoolBufUlAec);

        audio_pool_buf_copy_to_linear(
            &aurisys_handle->mLinearOut->p_buffer,
            &aurisys_handle->mLinearOut->memory_size,
            aurisys_handle->mAudioPoolBufUlOut,
            frame_buf_size);

#if MCPS_PROF_SUPPORT
        mcps_prof_stop(&sp_mcps_profile, SCENE_UL);
#endif

        if (dump_pcm_flag) {

            memcpy((void *)pcm_dump_ul.ul_in_ch1, (void *)ul_buf_in.data_buf.p_buffer, frame_buf_size);
            memcpy((void *)pcm_dump_ul.ul_in_ch2, (void *)(ul_buf_in.data_buf.p_buffer + frame_buf_size), frame_buf_size);
            memcpy((void *)pcm_dump_ul.aec_in1, (void *)(aec_buf_in.data_buf.p_buffer), frame_buf_size);
            memcpy((void *)pcm_dump_ul.aec_in2, (void *)(aec_buf_in.data_buf.p_buffer + frame_buf_size), frame_buf_size);
            memcpy((void *)pcm_dump_ul.ul_out, (void *)(aurisys_handle->mLinearOut->p_buffer), frame_buf_size);

            send_pcm_dump(this, ipi_msg, frame_buf_size, UL_IN1, pcm_dump_ul.ul_in_ch1);
            send_pcm_dump(this, ipi_msg, frame_buf_size, UL_IN2, pcm_dump_ul.ul_in_ch2);
            send_pcm_dump(this, ipi_msg, frame_buf_size, UL_REF1, pcm_dump_ul.aec_in1);
            if (2 == aurisys_handle->dsp_config->attribute[DATA_BUF_ECHO_REF].num_channels) {
                send_pcm_dump(this, ipi_msg, frame_buf_size, UL_REF2, pcm_dump_ul.aec_in2);
            }
            send_pcm_dump(this, ipi_msg, frame_buf_size, UL_OUT, aurisys_handle->mLinearOut->p_buffer);
        }

        if (NULL == aurisys_handle->mLinearOut->p_buffer) {
            AUD_LOG_E("p_buffer is NULL with mem_size = %u\n", aurisys_handle->mLinearOut->memory_size);
            memset((void *)p_ul_dst, 0, frame_buf_size);
        } else {
            memcpy((void *)p_ul_dst, aurisys_handle->mLinearOut->p_buffer, frame_buf_size);
        }
        /* flush after write cache*/
        hal_cache_region_operation (HAL_CACHE_TYPE_DATA , HAL_CACHE_OPERATION_FLUSH, (void *)p_ul_dst, FB_PCM_BUF_SIZE);

        if (mask_adsp2md_irq) {
            AUD_LOG_D("SKIP UL IRQ ADSP2MD");
        } else {
            DRV_WriteReg32(CCIF_AP_BUSY, 1 << 2);
            DRV_WriteReg32(CCIF_AP_TCHNUM, 0x2);
        }
    } else if (2 == channel) {

        /* invalidate cache before read */
        hal_cache_region_operation (HAL_CACHE_TYPE_DATA , HAL_CACHE_OPERATION_INVALIDATE, (void *)p_dl_src, FB_PCM_BUF_SIZE);

        memcpy((void *)dl_buf_in.data_buf.p_buffer, (void *)p_dl_src, frame_buf_size);

        if (mDspBandInfo != mPrevDspBandInfo)
            AUD_LOG_D("emi_phy_addr: = %llx %p\n", emi_phy_addr, p_dl_src);

        dl_buf_in.data_buf.data_size = frame_buf_size;
        dl_buf_out.data_buf.data_size = 0;


#if MCPS_PROF_SUPPORT
        mcps_prof_start(&sp_mcps_profile);
#endif
        audio_pool_buf_copy_from_linear(
            aurisys_handle->mAudioPoolBufDlIn,
            dl_buf_in.data_buf.p_buffer,
            dl_buf_in.data_buf.data_size);

        aurisys_process_dl_only(
            aurisys_handle->manager,
            aurisys_handle->mAudioPoolBufDlIn,
            aurisys_handle->mAudioPoolBufDlOut);

        audio_pool_buf_copy_to_linear(
            &aurisys_handle->mLinearOut->p_buffer,
            &aurisys_handle->mLinearOut->memory_size,
            aurisys_handle->mAudioPoolBufDlOut,
            frame_buf_size);

#if MCPS_PROF_SUPPORT
        mcps_prof_stop(&sp_mcps_profile, SCENE_DL);
#endif
        if (dump_pcm_flag) {
            memcpy((void *)pcm_dump_dl.dl_in, (void *)(dl_buf_in.data_buf.p_buffer), frame_buf_size);
            memcpy((void *)pcm_dump_dl.dl_out, (void *)(dl_buf_out.data_buf.p_buffer), frame_buf_size);

            send_pcm_dump(this, ipi_msg, frame_buf_size, DL_IN, pcm_dump_dl.dl_in);
            send_pcm_dump(this, ipi_msg, frame_buf_size, DL_OUT, aurisys_handle->mLinearOut->p_buffer);
        }

        memcpy((void *)p_dl_dst, aurisys_handle->mLinearOut->p_buffer, frame_buf_size);

        /* flush after write cache*/
        hal_cache_region_operation (HAL_CACHE_TYPE_DATA , HAL_CACHE_OPERATION_FLUSH, (void *)p_dl_dst, FB_PCM_BUF_SIZE);

        if (mask_adsp2md_irq) {
            AUD_LOG_D("SKIP DL IRQ ADSP2MD");
        } else {
            DRV_WriteReg32(CCIF_AP_BUSY, 1 << 3);
            DRV_WriteReg32(CCIF_AP_TCHNUM, 0x3);
        }
    } else if (16 == channel) {
        AUD_LOG_D("%s(): band change. mDspBandInfo: = %d\n", __func__, mDspBandInfo);
        /* process inside set_call_band */
    }

    return NO_ERROR;
}


static status_t task_phone_call_deinit(struct AudioTask *this) {
    AUD_LOG_D("%s()\n", __func__);

    AUD_LOG_D("+Heap free/total = %d/%d\n",
              xPortGetFreeHeapSize(), configTOTAL_HEAP_SIZE);


    AUD_LOG_D("Free = ul_buf_in.data_buf.p_buffer\n");
    kal_vPortFree(ul_buf_in.data_buf.p_buffer);
    ul_buf_in.data_buf.memory_size = 0;
    ul_buf_in.data_buf.p_buffer = NULL;

    AUD_LOG_D("Free = ul_buf_out.data_buf.p_buffer\n");
    kal_vPortFree(ul_buf_out.data_buf.p_buffer);
    ul_buf_out.data_buf.memory_size = 0;
    ul_buf_out.data_buf.p_buffer = NULL;

    AUD_LOG_D("Free = dl_buf_in.data_buf.p_buffer\n");
    kal_vPortFree(dl_buf_in.data_buf.p_buffer);
    dl_buf_in.data_buf.memory_size = 0;
    dl_buf_in.data_buf.p_buffer = NULL;

    AUD_LOG_D("Free = dl_buf_out.data_buf.p_buffer\n");
    kal_vPortFree(dl_buf_out.data_buf.p_buffer);
    dl_buf_out.data_buf.memory_size = 0;
    dl_buf_out.data_buf.p_buffer = NULL;

    AUD_LOG_D("Free = aec_buf_in.data_buf.p_buffer %d\n", aec_buf_in.data_buf.memory_size);
    AUD_LOG_D("aec_buf_in.data_buf.p_buffer = %p\n", aec_buf_in.data_buf.p_buffer);
    kal_vPortFree(aec_buf_in.data_buf.p_buffer);
    aec_buf_in.data_buf.memory_size = 0;
    aec_buf_in.data_buf.p_buffer = NULL;


    kal_vPortFree(pcm_dump_ul.ul_in_ch1);
    kal_vPortFree(pcm_dump_ul.ul_in_ch2);
    kal_vPortFree(pcm_dump_ul.aec_in1);
    kal_vPortFree(pcm_dump_ul.aec_in2);
    kal_vPortFree(pcm_dump_ul.ul_out);
    kal_vPortFree(pcm_dump_dl.dl_in);
    kal_vPortFree(pcm_dump_dl.dl_out);

    AUD_LOG_D("Free = End\n");

    AUD_LOG_D("-Heap free/total = %d/%d\n",
              xPortGetFreeHeapSize(), configTOTAL_HEAP_SIZE);
#ifdef WAKELOCK
    //wake_unlock(&phone_call_monitor_wakelock);
#endif

#if PHONECALL_PROFILING
    AUD_LOG_D("Phone call profiling, band info %d, input_device 0x%x, output_device = 0x%x, \
              lib max UL process = %d mcps, lib max DL process = %d mcps, \
              lib avg UL process = %f mcps, lib avg DL process = %f mcps\n",
              mDspBandInfo, mInputDeviceInfo, mOutputDeviceInfo, lib_max_ul_mcps, lib_max_dl_mcps,
              lib_avg_ul_mcps, lib_avg_dl_mcps);
#endif
    return NO_ERROR;
}

