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
#include "audio_task_offload_mp3.h"

#include <stdarg.h>

#include <interrupt.h>
#include <dvfs.h>
#include <dma.h>
#include <feature_manager.h>

#ifdef CFG_MTK_AUDIODSP_SUPPORT
#include <adsp_ipi.h>
#else
#include <scp_ipi.h>
#endif

#include <wrapped_errors.h>
#include <wakelock.h>
#ifdef MET_MP3
#include <met_tag.h> //for tag check
#endif
/* audio utility relate*/
#include <audio_ringbuf.h>
#include <audio_task_utility.h>
#include <audio_task_attr.h>
#include <audio_dsp_hw_hal.h>

#include "audio_messenger_ipi.h"
#include "audio_task_offload_mp3_params.h"
#include "arsi_api.h"


#include "audio_hw.h"
#include "audio_hw_reg.h"
#include "ADIF_Header.h"

#include <feature_manager.h>
#include <dvfs_config_parser.h>

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
#include <audio_task_aurisys.h>
#include <aurisys_config.h>
#endif

/*==============================================================================
 *                     MACRO
 *============================================================================*/
#define LOCAL_TASK_STACK_SIZE (8192)
#define LOCAL_TASK_NAME "mp3"
#define LOCAL_TASK_PRIORITY (2)

#define MAX_MSG_QUEUE_SIZE (40)
/* TODO: calculate it */
#define USE_PERIODS_MAX     (8192)
#define DRAM_REMAIN_THRESHOLD   (USE_PERIODS_MAX * 8) //64K
#define SRAM_PERIODS_SIZE   (16384)
#define POST_PROCESS
#define AudioDecoder

#define BliSrc
#define BLISRC_BUFFER_SIZE (4608<<2)
#define CACHE_LINE_SIZE (128)
#define LOCAL_TASK_OUPUT_BUFFER_SIZE (4096<<3)

//#define MCPS_PROFILING
//#define CYCLE
/*==============================================================================
 *                     private global members
 *============================================================================*/
/* Task buffer */
static struct audio_hw_buffer mtask_audio_buf;
static struct audio_hw_buffer mtask_audio_afebuf;
static struct RingBuf mtask_audio_ring_buf;
static struct audio_dsp_dram mtask_dsp_atod_sharemm;
static struct audio_dsp_dram mtask_dsp_dtoa_sharemm;
static int hw_buf_handle = -1;
static int pcmdump_enable = 0;

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
static struct audio_task_offload_aurisys *task_offload_aurisys  = NULL;
static struct data_buf_t aurisys_param_list;
static void *aurisys_param_data_buf = NULL;
static uint32_t aurisys_param_data_size = 0;
static bool initAurisysManagerflag = false;
#endif

#ifdef WAKELOCK
wakelock_t mp3_task_monitor_wakelock;
#endif
typedef struct {
    SCP_OFFLOAD_T afe_offload_block;
    struct RingBuf rMemDL;
    struct RingBuf rMemDRAM;
    DMA_BUFFER_T bs_buf;
    int pcm_timetsamp;
    uint32_t bs_adddata;
    unsigned char bDramPlayback;
} do_save_mp3_t;
/* TODO: put to derived class */
static DMA_BUFFER_T bs_buf;
static DMA_BUFFER_T out_buf;
static DMA_BUFFER_T pcm_buf;
static SCP_OFFLOAD_T afe_offload_block;
static void *pRemainedPCM_Buf;
static int *pBitConvertBuf;
static void *pOutputBuf;
static unsigned int mOuputsize = 0;
static void *pBSDump_BUf;
static unsigned int mBSDumpsize = 0;


//Ringbuf : DRAM, SRAM
static struct RingBuf rMemDL;
static struct RingBuf *rMemDRAM;
#ifdef AudioDecoder
static mp3DecEngine *mMp3Dec = NULL;
static uint8_t mMp3InitFlag = false;
static aacDecEngine *mAacDec = NULL;
static uint8_t mAacInitFlag = false;
#endif
#ifdef BliSrc
static blisrcEngine *mBliSrc = NULL;
static uint8_t mSrcInitFlag = false;
static void *pSrcOutBuf;
static uint32_t decoder_sameplerateidx[9] = {44100, 48000, 32000, 22050, 24000, 16000, 11025, 12000, 8000};
#endif
unsigned char bDramPlayback = 0;
unsigned char AFEenable = false;
unsigned char bfirsttime = true;
unsigned char bfirsdecode = true;

static uint32_t bsAddData;
static uint8_t dump_pcm_flag;
static uint32_t dump_total;
uint32_t dst_addr_sram;

static struct RingBuf dram_dump;
static uint8_t mCountForSrc = 2;
uint32_t RemainedPcmlen = 0;
static uint8_t bmp3_src_remained = false;
static uint8_t baac_src_remained = false;
static uint32_t mCurrentBSIdx = 0;
static uint8_t bDoBack = false;
static uint8_t birq = false;

#ifdef MET_MP3
uint32_t task_monitor_tag_id;
#endif
//#define CYCLE
#ifdef CYCLE
volatile int *MP3_DWT_CONTROL = (int *)0xE0001000;
volatile int *MP3_DWT_CYCCNT = (int *)0xE0001004;
volatile int *MP3_DEMCR = (int *)0xE000EDFC;

#define MP3_CPU_RESET_CYCLECOUNTER    do { *MP3_DEMCR = *MP3_DEMCR | 0x01000000;  \
        *MP3_DWT_CYCCNT = 0;              \
        *MP3_DWT_CONTROL = *MP3_DWT_CONTROL | 1 ; } while(0)
#endif
static task_offload_scene_t OFFLOAD_SCENE_TYPE;
static int pcm_buffer_size = 0;
static int decode_count = 0;
static bool decode_status = false;
static bool decode_unsupport_format = false;

static offload_codec_t offload_codec_info;
static dvfs_swip_config_t mp3_config;
static dvfs_swip_config_t aac_config;
static dvfs_swip_config_t src_config;

static uint32_t sbr_vaule = 0;
#ifdef MCPS_PROFILING
unsigned int start_time;
unsigned int end_time;
#endif

//extern freq_enum get_cur_opp(void);
/*==============================================================================
 *                     derived functions - declaration
 *============================================================================*/

static void     task_offload_constructor(struct AudioTask *this);
static void     task_offload_destructor(struct AudioTask *this);

static void     task_offload_create_task_loop(struct AudioTask *this);
static void     task_offload_destroy_task_loop(struct AudioTask *this);


static status_t task_offload_recv_message(
    struct AudioTask *this,
    struct ipi_msg_t *ipi_msg);

static void     task_offload_irq_hanlder(
    struct AudioTask *this,
    uint32_t irq_type);

static uint32_t task_offload_get_write_back_size(struct AudioTask *this);
static void     task_offload_copy_write_back_buffer(struct AudioTask *this,
                                                        data_buf_t *buf);
static void     task_offload_do_write_back(struct AudioTask *this,
                                               data_buf_t *buf);

static void     task_offload_hal_reboot_cbk(
    struct AudioTask *this,
    const uint8_t hal_type);



/*==============================================================================
 *                     private functions - declaration
 *============================================================================*/


static uint8_t task_offload_preparsing_message(struct AudioTask *this,
                                                   ipi_msg_t *ipi_msg);

static void           task_offload_loop(void *pvParameters);

static status_t task_offload_init(struct AudioTask *this);
static status_t task_offload_mp3_working(struct AudioTask *this);
static status_t task_offload_deinit(struct AudioTask *this);
static status_t task_offload_aac_working(struct AudioTask *this);

static int mp3_init_decoder(void);
static int offload_deinit_blisrc(void);
static void mp3_deinit_decoder(void);
static int aac_init_decoder(void);
static void aac_deinit_decoder(void);

static int offload_init_blisrc(int32_t inSampleRate, int32_t inChannelCount,
                           int32_t outSampleRate, int32_t outChannelCount);

static int offload_blisrc_process(void *pInBuffer, unsigned int inBytes,
                              void **ppOutBuffer, unsigned int *pOutBytes);
static int offload_volume_process(void *pInBuffer, unsigned int inBytes,
                              uint32_t *vc, uint32_t *vp,
                              void **ppOutBuffer, unsigned int *pOutBytes);
#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
static int offload_bitconvert_process(void *pInBuffer, unsigned int inBytes,
                                  void **ppOutBuffer, unsigned int *pOutBytes);
#endif
static int offload_init_bsbuffer(void);
static void flush_dram(unsigned int pBufAddr, int len);

static int offload_fillbs_fromDram(uint32_t bs_fill_size);
static int mp3_decode_process(void);
static int aac_decode_process(void);
static unsigned int offload_calc_datasize(unsigned int olddatasize, unsigned int newdatasize);
static void offload_get_timestamp(struct AudioTask *this);
static void offload_dump_pcm(DEBUG_DUMP_OFFLOAD flag, uint32_t dump_buf_src,
                         uint32_t dump_buf_size);
int offload_get_bs_buffersize(void);
int offload_get_pcm_buffersize(void);
void set_task_scene(task_offload_scene_t scene);
void offload_calc_dram(void);
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
int init_aurisys_task(struct audio_task_offload_aurisys **task_offload);
int deinit_aurisys_task(struct audio_task_offload_aurisys **task_offload);
#endif
typedef struct task_scene_op {
    status_t (*task_offload_working)(struct AudioTask *this);
    int (*task_init_decoder)(void);
    void (*task_deinit_decoder)(void);
    uint32_t BSBUFFER_SIZE;
} task_scene_op;
static task_scene_op offload_op;

//extern void scp_wakeup_src_setup(wakeup_src_id irq, uint32_t enable);

/*==============================================================================
 *                     class new/construct/destruct functions
 *============================================================================*/

AudioTask *task_offload_new()
{
    AUD_LOG_D("MP3 %s(+)\n", __func__);

    /* alloc object here */
    AudioTask *task = (AudioTask *)kal_pvPortMalloc(sizeof(AudioTask));

    if (task == NULL) {
        AUD_LOG_E("%s(), kal_pvPortMalloc fail!!\n", __func__);
        return NULL;
    }
    memset(task, 0, sizeof(AudioTask));
    OFFLOAD_SCENE_TYPE = TASK_SCENE_OFFLOAD_MP3;

    /* only assign methods, but not class members here */
    task->constructor       = task_offload_constructor;
    task->destructor        = task_offload_destructor;

    task->create_task_loop  = task_offload_create_task_loop;
    task->destroy_task_loop = task_offload_destroy_task_loop;

    task->recv_message      = task_offload_recv_message;

    task->irq_hanlder       = task_offload_irq_hanlder;

    task->get_write_back_size       = task_offload_get_write_back_size;
    task->copy_write_back_buffer    = task_offload_copy_write_back_buffer;
    task->do_write_back             = task_offload_do_write_back;

    task->hal_reboot_cbk            = task_offload_hal_reboot_cbk;

    AUD_LOG_D("MP3 %s(-), task = %p\n", __func__, task);
    return task;
}


void task_offload_delete(AudioTask *task)
{
    AUD_LOG_V("%s(+), task_scene = %d\n", __func__, task->scene);

    if (task == NULL) {
        AUD_LOG_E("%s(), task is NULL!!\n", __func__);
        return;
    }

    vPortFree(task);

    AUD_LOG_V("%s(-)\n", __func__);
}

static uint32_t task_offload_get_write_back_size(struct AudioTask *this)
{
    if (!afe_offload_block.bIsClose) {
        return sizeof(do_save_mp3_t);
    }
    else {
        return 0;
    }
}
static void task_offload_copy_write_back_buffer(struct AudioTask *this,
                                                    data_buf_t *buf)
{
    do_save_mp3_t wb_data;

    AUD_ASSERT(buf->memory_size >= sizeof(do_save_mp3_t));
    //save sturcture
    memset((void *)&wb_data, 0, sizeof(wb_data));
    memcpy((void *)&wb_data.bs_buf, (void *)&bs_buf, sizeof(bs_buf));
    memcpy((void *)&wb_data.afe_offload_block, (void *)&afe_offload_block,
           sizeof(afe_offload_block));
    memcpy((void *)&wb_data.rMemDL, (void *)&rMemDL, sizeof(rMemDL));
    memcpy((void *)&wb_data.rMemDRAM, (void *)rMemDRAM, sizeof(*rMemDRAM));
    wb_data.pcm_timetsamp = pcm_buf.u4DataRemained;
    wb_data.bDramPlayback = bDramPlayback;
    wb_data.bs_adddata = bsAddData;
    memcpy(buf->p_buffer, &wb_data, sizeof(do_save_mp3_t));
    buf->data_size = sizeof(do_save_mp3_t);
    // copy to dram
    mp3_dma_transaction_wrap((uint32_t)rMemDRAM->pBufEnd, (uint32_t)bs_buf.pBufAddr,
                             bs_buf.u4BufferSize, true);
    if (bsAddData) {
        mp3_dma_transaction_wrap((uint32_t)(rMemDRAM->pBufEnd + bs_buf.u4BufferSize),
                                 (uint32_t)(bs_buf.pBufAddr + bs_buf.u4BufferSize), bsAddData, true);

    }
    AUD_LOG_V("-- SRAM BufBase = %p, BufEnd = %p, Read = %p, Write = %p\n",
              rMemDL.pBufBase, rMemDL.pBufEnd, rMemDL.pRead, rMemDL.pWrite);

    AUD_LOG_V("-- DRAM BufBase = %p, BufEnd = %p, Read = %p, Write = %p\n",
              rMemDRAM->pBufBase, rMemDRAM->pBufEnd, rMemDRAM->pRead, rMemDRAM->pWrite);

}
static void task_offload_do_write_back(struct AudioTask *this,
                                           data_buf_t *buf)
{
    do_save_mp3_t wb_data;

    AUD_ASSERT(buf->memory_size >= sizeof(do_save_mp3_t));
    AUD_ASSERT(buf->data_size == sizeof(do_save_mp3_t));
    memcpy(&wb_data, buf->p_buffer, sizeof(do_save_mp3_t));

    memcpy((void *)&bs_buf, (void *)&wb_data.bs_buf, sizeof(bs_buf));
    memcpy((void *)&afe_offload_block, (void *)&wb_data.afe_offload_block,
           sizeof(afe_offload_block));
    memcpy((void *)&rMemDL, (void *)&wb_data.rMemDL, sizeof(rMemDL));
    memcpy((void *)rMemDRAM, (void *)&wb_data.rMemDRAM, sizeof(*rMemDRAM));
    pcm_buf.u4DataRemained = wb_data.pcm_timetsamp;
    bDramPlayback = wb_data.bDramPlayback;
    bsAddData = wb_data.bs_adddata;
    AUD_LOG_V("++ SRAM BufBase = %p, BufEnd = %p, Read = %p, Write = %p\n",
              rMemDL.pBufBase, rMemDL.pBufEnd, rMemDL.pRead, rMemDL.pWrite);

    AUD_LOG_V("++ DRAM BufBase = %p, BufEnd = %p, Read = %p, Write = %p\n",
              rMemDRAM->pBufBase, rMemDRAM->pBufEnd, rMemDRAM->pRead, rMemDRAM->pWrite);
    bDoBack = true;
    this->state = AUDIO_TASK_DEINIT;
}

static void task_offload_constructor(struct AudioTask *this)
{
    AUD_LOG_D("MP3 %s(+), task_scene = %d\n", __func__, this->scene);
    AUD_ASSERT(this != NULL);

    /* assign initial value for class members & alloc private memory here */
    this->scene = TASK_SCENE_PLAYBACK_MP3;
    this->state = AUDIO_TASK_IDLE;

    /* queue */
    this->queue_idx = 0;
    this->num_queue_element = 0;

    // this->msg_array = NULL;
    this->msg_array = (ipi_msg_t *)kal_pvPortMalloc(
                          MAX_MSG_QUEUE_SIZE * sizeof(ipi_msg_t));
    AUD_ASSERT(this->msg_array != NULL);
    memset(this->msg_array, 0, MAX_MSG_QUEUE_SIZE * sizeof(ipi_msg_t));

    this->msg_idx_queue = xQueueCreate(MAX_MSG_QUEUE_SIZE, sizeof(uint8_t));

    AUD_ASSERT(this->msg_idx_queue != NULL);
    memset(&afe_offload_block, 0, sizeof(afe_offload_block));
    dump_pcm_flag = 0;
    dump_total = 0;
    afe_offload_block.volume[0] = 0x0F;
    afe_offload_block.bIsClose = true;
    mOuputsize = 0;

    AUD_LOG_D("MP3 %s(-), task_scene = %d\n", __func__, this->scene);
}


static void task_offload_destructor(struct AudioTask *this)
{
    if (this == NULL) {
        AUD_LOG_E("%s(), this is NULL!!\n", __func__);
        return;
    }

    AUD_LOG_V("%s(+), task_scene = %d\n", __func__, this->scene);

    /* dealloc private memory & dealloc object here */
    task_offload_deinit(this);

    if (this->msg_array != NULL) {
        kal_vPortFree(this->msg_array);
    }
    if (this->msg_idx_queue != NULL) {
        vQueueDelete(this->msg_idx_queue);
    }
    AUD_LOG_V("%s(-), task_scene = %d\n", __func__, this->scene);
}


static void task_offload_create_task_loop(struct AudioTask *this)
{
    /* Note: you can also bypass this function,
             and do kal_xTaskCreate until you really need it.
             Ex: create task after you do get the enable message. */
    AUD_LOG_D("MP3 %s(+)\n", __func__);

    BaseType_t xReturn = pdFAIL;

    xReturn = kal_xTaskCreate(
                  task_offload_loop,
                  LOCAL_TASK_NAME,
                  LOCAL_TASK_STACK_SIZE,
                  (void *)this,
                  LOCAL_TASK_PRIORITY,
                  &this->freertos_task);

    AUD_ASSERT(xReturn == pdPASS);
    AUD_ASSERT(this->freertos_task);
#ifdef MET_MP3
    task_monitor_tag_id = met_tag_create("mp3_decode");
#endif
    AUD_LOG_D("MP3 %s(-)\n", __func__);

}


static void task_offload_destroy_task_loop(struct AudioTask *this)
{
#if (INCLUDE_vTaskDelete == 1)
    if (this->freertos_task) {
        vTaskDelete(this->freertos_task);
    }
#endif
}

static uint8_t offload_get_queue_idx(struct AudioTask *this)
{
    uint8_t queue_idx = this->queue_idx;

    this->queue_idx++;
    if (this->queue_idx == MAX_MSG_QUEUE_SIZE) {
        this->queue_idx = 0;
    }

    return queue_idx;
}


static status_t task_offload_recv_message(
    struct AudioTask *this,
    struct ipi_msg_t *ipi_msg)
{
    static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    uint8_t ret = false;
    uint8_t queue_idx = 0, i = 0;
    this->num_queue_element++;
    queue_idx = offload_get_queue_idx(this);
    // check message
    if (this->msg_array[queue_idx].magic != 0) {
        AUD_LOG_E("queue_idx = %d\n", queue_idx);
        for (i = 0; i < MAX_MSG_QUEUE_SIZE; i++) {
            AUD_LOG_E("[%d] id = 0x%x\n", i, this->msg_array[i].msg_id);
        }
        AUD_ASSERT(this->msg_array[queue_idx].magic == 0); /* item is clean */
    }
    // copy to array
    memcpy(&this->msg_array[queue_idx], ipi_msg, sizeof(ipi_msg_t));

    ret = task_offload_preparsing_message(this, &this->msg_array[queue_idx]);
    if (ret) {
        if (xQueueSendToBackFromISR(this->msg_idx_queue, &queue_idx,
                                    &xHigherPriorityTaskWoken) != pdTRUE) {
            return UNKNOWN_ERROR;
        }
    }
    else {
        /* clean msg */
        memset(&this->msg_array[queue_idx], 0, sizeof(ipi_msg_t));
        this->num_queue_element--;
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    return NO_ERROR;
}


static void task_offload_irq_hanlder(
    struct AudioTask *this,
    uint32_t irq_type)
{
    if (hw_buf_handle < 0) {
        AUD_LOG_D("hw_buf_handle < 0");
        return;
    }

    audio_dsp_hw_irq(this, hw_buf_handle);
    birq = true;
    if (this->state == AUDIO_TASK_WORKING) {
        AUD_LOG_V("task_offload_mp3_irq_hanlder");
        uint8_t queue_idx = 0, i = 0;
        ipi_msg_t *ipi_msg = NULL;
        static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
        this->num_queue_element++;
        queue_idx = offload_get_queue_idx(this);
        ipi_msg = &this->msg_array[queue_idx];
        if (ipi_msg->magic != 0) {
            AUD_LOG_E("irq queue_idx = %d\n", queue_idx);
            for (i = 0; i < MAX_MSG_QUEUE_SIZE; i++) {
                AUD_LOG_E("[%d] id = 0x%x\n", i, this->msg_array[i].msg_id);
            }
            AUD_ASSERT(ipi_msg->magic == 0); /* item is clean */
        }
        ipi_msg->magic          = IPI_MSG_MAGIC_NUMBER;
        ipi_msg->task_scene     = TASK_SCENE_PLAYBACK_MP3;
        ipi_msg->target_layer   = AUDIO_IPI_LAYER_TO_DSP;
        ipi_msg->data_type      = AUDIO_IPI_MSG_ONLY;
        ipi_msg->ack_type       = AUDIO_IPI_MSG_BYPASS_ACK;
        ipi_msg->msg_id         = AUDIO_DSP_TASK_DLCOPY;
        ipi_msg->param1         = 0;
        ipi_msg->param2         = 0;
        if (xQueueSendToBackFromISR(this->msg_idx_queue, &queue_idx,
                                    &xHigherPriorityTaskWoken) != pdTRUE) {
            return;
        }
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}


static void task_offload_hal_reboot_cbk(struct AudioTask *this, const uint8_t hal_type) {
    AUD_LOG_D("%s(), scene %d\n", __func__, this->scene);
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    if (task_offload_aurisys != NULL) {
        if (task_offload_aurisys->task_aurisys && task_offload_aurisys->task_aurisys->manager) {
            DestroyAurisysLibManager(task_offload_aurisys->task_aurisys, task_offload_aurisys->task_scene);
            initAurisysManagerflag = false;
            AUDIO_FREE_POINTER(task_offload_aurisys->task_aurisys->dsp_config);
            FREE_ALOCK(task_offload_aurisys->task_aurisys->mAurisysLibManagerLock);
        }
        AUDIO_FREE(task_offload_aurisys->mtask_processing_buf);
        int ret = deinit_aurisys_task(&task_offload_aurisys);
        AUD_LOG_D("%s(), AUDIO_DSP_TASK_STOP: deinit_aurisys_task\n", __func__);
        if (ret) {
            AUD_LOG_E("%s(), AUDIO_DSP_TASK_STOP: deinit_aurisys_task ret[%d]!!\n", __func__, ret);
        }
        task_offload_aurisys = NULL;
    }
#endif
}


void set_task_scene(task_offload_scene_t scene)
{
    AUD_LOG_D("%s(), scene = %d\n", __func__, scene);
    switch(scene) {
        case TASK_SCENE_OFFLOAD_MP3: {
            offload_op.task_offload_working = task_offload_mp3_working;
            offload_op.task_init_decoder = mp3_init_decoder;
            offload_op.task_deinit_decoder = mp3_deinit_decoder;
            offload_op.BSBUFFER_SIZE = 4096;
            }
        break;
        case TASK_SCENE_OFFLOAD_AAC: {
            offload_op.task_offload_working = task_offload_aac_working;
            offload_op.task_init_decoder = aac_init_decoder;
            offload_op.task_deinit_decoder = aac_deinit_decoder;
            offload_op.BSBUFFER_SIZE = 3192;
       }
        break;
       }
    AUD_LOG_D("%s(), scene = %d, offload_op.BSBUFFER_SIZE = %d\n", __func__, scene, offload_op.BSBUFFER_SIZE);
}
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
int init_aurisys_task(struct audio_task_offload_aurisys **task_offload)
{
    struct audio_task_offload_aurisys *task_offload_inst = NULL;

    if (*task_offload != NULL) {
        AUD_LOG_E("%s\n", __FUNCTION__);
        return -1;
    }

    AUD_LOG_D("%s\n", __FUNCTION__);
    task_offload_inst = (struct audio_task_offload_aurisys *)AUDIO_MALLOC(sizeof(
                                                                    struct audio_task_offload_aurisys));
    if (task_offload_inst == NULL) {
        AUD_LOG_D("%s AUDIO_MALLOC task_common_inst fail\n", __FUNCTION__);
        return -1;
    }
    memset((void *)task_offload_inst, 0, sizeof(struct audio_task_offload_aurisys));

    task_offload_inst->task_aurisys = (struct audio_aurisys_handle_t *)AUDIO_MALLOC(
                                         sizeof(
                                             struct audio_aurisys_handle_t));
    if (task_offload_inst->task_aurisys == NULL) {
        AUD_LOG_D("%s AUDIO_MALLOC task_aurisys fail\n", __FUNCTION__);
        if (task_offload_inst) {
            AUDIO_FREE(task_offload_inst);
        }
        return -1;
    }
    memset((void *)task_offload_inst->task_aurisys, 0,
           sizeof(struct audio_aurisys_handle_t));

    *task_offload = task_offload_inst;

    return 0;
}

int deinit_aurisys_task(struct audio_task_offload_aurisys **task_offload)
{
    struct audio_task_offload_aurisys *task_offload_inst = *task_offload;

    if (task_offload_inst == NULL) {
        AUD_LOG_E("%s task_common = NULL \n", __FUNCTION__);
        return -1;
    }
    if (task_offload_inst->task_aurisys) {
        AUDIO_FREE(task_offload_inst->task_aurisys);
        task_offload_inst->task_aurisys = NULL;
    }
    AUDIO_FREE(task_offload_inst);
    *task_offload = NULL;
    return 0;
}
#endif

static void task_offload_loop(void *pvParameters)
{
    AUD_LOG_D("task_offload_loop start \n");

    AudioTask *this = (AudioTask *)pvParameters;
    uint8_t local_queue_idx = 0xFF;
    ipi_msg_t *ipi_msg;
    struct Audio_Task_Msg_t uAudioTaskMsg;
    unsigned int pa_addr = 0;

    while (1) {
        if (xQueueReceive(this->msg_idx_queue, &local_queue_idx,
                          portMAX_DELAY) == pdTRUE) {
            AUD_LOG_D("vOffloadTaskProject received = %d\n", local_queue_idx);
        }
        ipi_msg = &this->msg_array[local_queue_idx];

        uAudioTaskMsg.msg_id = ipi_msg->msg_id;
        uAudioTaskMsg.param1 = ipi_msg->param1;
        uAudioTaskMsg.param2 = ipi_msg->param2;
        uAudioTaskMsg.dma_addr = ipi_msg->dma_addr;
#if 1
        switch (uAudioTaskMsg.msg_id) {
            case AUDIO_DSP_TASK_OPEN :
                AUD_LOG_D("%s() OFFLOAD DRAIN AUDIO_DSP_TASK_OPEN \n", __FUNCTION__);
                this->state = AUDIO_TASK_INIT;
                break;
            case AUDIO_DSP_TASK_HWPARAM : {
                AUD_LOG_D("%s() OFFLOAD AUDIO_DSP_TASK_HWPARAM \n", __FUNCTION__);
                pa_addr = (unsigned int)mtask_dsp_atod_sharemm.phy_addr;
                hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                           HAL_CACHE_OPERATION_INVALIDATE,
                                           (void *)pa_addr,
                                           mtask_dsp_atod_sharemm.size
                                          );
                get_audiobuf_from_msg(&uAudioTaskMsg, &mtask_audio_buf);
                RingBuf_Map_RingBuf_bridge(&mtask_audio_buf.aud_buffer.buf_bridge,
                                           &mtask_audio_ring_buf);
                this->state = AUDIO_TASK_INIT;
#ifdef DEBUG_VERBOSE
                dump_audio_hwbuffer(&mtask_audio_buf);
                dump_rbuf(&mtask_audio_ring_buf);
#endif
            }
            break;
            case OFFLOAD_DRAIN: {
                afe_offload_block.bIsDrain = true;
                rMemDRAM->pWrite = (char *)ipi_msg->param1;
                pa_addr = (unsigned int)rMemDRAM->pBufBase;
                hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                           HAL_CACHE_OPERATION_FLUSH_INVALIDATE,
                                           (void *)pa_addr,
                                           rMemDRAM->bufLen
                                           );
                AUD_LOG_D("%s() OFFLOAD DRAIN IDX = %p,  afe_offload_block.bIsDrain = %d\n", __FUNCTION__, rMemDRAM->pWrite, afe_offload_block.bIsDrain);
            }
            break;
            case OFFLOAD_RESUME : {
                AUD_LOG_D("%s() OFFLOAD_RESUME \n", __FUNCTION__);
                afe_offload_block.bIsPause = false;
                if (this->state == AUDIO_TASK_WORKING && offload_op.task_offload_working != NULL) {
                    offload_op.task_offload_working(this);
                }
            }
            break;
            case OFFLOAD_PAUSE : {
                AUD_LOG_D("%s() OFFLOAD_PAUSE \n", __FUNCTION__);
                audio_dsp_hw_stop(hw_buf_handle);
                afe_offload_block.bIsPause = true;
                mOuputsize = 0;
                decode_count = 0;
            }
            break;
            case OFFLOAD_SCENE : {
                OFFLOAD_SCENE_TYPE = (task_offload_scene_t)ipi_msg->param1;
                AUD_LOG_D("%s()Offload_SCENE = %d \n", __FUNCTION__,OFFLOAD_SCENE_TYPE);
                set_task_scene(OFFLOAD_SCENE_TYPE);
            }
            break;
            case AUDIO_DSP_TASK_DLCOPY: {
                AUD_LOG_V("%s() AUDIO_DSP_TASK_DLCOPY \n", __FUNCTION__);
                pa_addr = (unsigned int)mtask_dsp_atod_sharemm.phy_addr;
                hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                           HAL_CACHE_OPERATION_INVALIDATE,
                                           (void *)pa_addr,
                                           mtask_dsp_atod_sharemm.size
                                          );

                if (this->state == AUDIO_TASK_INIT ||
                    this->state == AUDIO_TASK_IDLE) {
                    /* get message status and sync write point*/
                    if (uAudioTaskMsg.dma_addr == 0) {
                        break;
                    }
                    get_audiobuf_from_msg(&uAudioTaskMsg, &mtask_audio_buf);
                    /* sync with buf_bridge*/
                    sync_ringbuf_writeidx(&mtask_audio_ring_buf,
                                          &mtask_audio_buf.aud_buffer.buf_bridge);
                }
                if (this->state == AUDIO_TASK_IDLE) {
                    this->state = AUDIO_TASK_INIT;
                    task_offload_init(this);
                }
                if (this->state != AUDIO_TASK_DEINIT) {
                    this->state = AUDIO_TASK_WORKING;
                    offload_op.task_offload_working(this);
                    pa_addr = (unsigned int)mtask_dsp_dtoa_sharemm.phy_addr;
                    hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                               HAL_CACHE_OPERATION_INVALIDATE,
                                               (void *)pa_addr,
                                               mtask_dsp_dtoa_sharemm.size
                                              );
                }
            }
            break;
            case OFFLOAD_PCMDUMP_ON: {
                AUD_LOG_D("%s, OFFLOAD_PCMDUMP_ON\n", __func__);
                dram_dump.bufLen = ipi_msg->param1;
                dump_pcm_flag = ipi_msg->param2;
                if (dump_pcm_flag) {
                    dram_dump.pBufBase = (char *)ap_to_scp((uint32_t)ipi_msg->dma_addr);
                    dram_dump.pBufEnd = dram_dump.pBufBase + dram_dump.bufLen;
                    dram_dump.pRead = dram_dump.pBufBase;
                    dram_dump.pWrite = dram_dump.pBufBase;
                }
                dump_total = 0;
                break;
            }
            case AUDIO_DSP_TASK_MSGA2DSHAREMEM: {
                AUD_LOG_D("%s()MAUDIO_DSP_TASK_MSGA2DSHAREMEM \n", __FUNCTION__);
                //DumpTaskMsg(" AUDIO_DSP_TASK_MSGA2DSHAREMEM", &uAudioTaskMsg);
                memcpy((void *)&mtask_dsp_atod_sharemm, ipi_msg->payload,
                       ipi_msg->param1);
                dump_audio_dsp_dram(&mtask_dsp_atod_sharemm);
                break;
            }
            case AUDIO_DSP_TASK_MSGD2ASHAREMEM: {
                AUD_LOG_D("%s()AUDIO_DSP_TASK_MSGD2ASHAREMEM \n", __FUNCTION__);
                //DumpTaskMsg("AUDIO_DSP_TASK_MSGA2DSHAREMEM", &uAudioTaskMsg);
                memcpy((void *)&mtask_dsp_dtoa_sharemm, ipi_msg->payload,
                       ipi_msg->param1);
                dump_audio_dsp_dram(&mtask_dsp_dtoa_sharemm);
                break;
            }
            case AUDIO_DSP_TASK_PCM_HWPARAM: {
                AUD_LOG_D("%s()+ AUDIO_DSP_TASK_PCM_HWPARAM \n", __FUNCTION__);
                /* get message status and sync write point*/
                //DumpTaskMsg("AUDIO_DSP_TASK_HWPARAM", &uAudioTaskMsg);
                pa_addr = (unsigned int)mtask_dsp_atod_sharemm.phy_addr;
                hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                           HAL_CACHE_OPERATION_INVALIDATE,
                                           (void *)pa_addr,
                                           mtask_dsp_atod_sharemm.size
                                          );
                get_audiobuf_from_msg(&uAudioTaskMsg, &mtask_audio_afebuf);
                dump_audio_hwbuffer(&mtask_audio_afebuf);
                AUD_LOG_D("%s()- AUDIO_DSP_TASK_PCM_HWPARAM \n", __FUNCTION__);
                break;
            }
            case AUDIO_DSP_TASK_STOP: {
                AUD_LOG_D("%s()AUDIO_DSP_TASK_STOP \n", __FUNCTION__);
                this->state = AUDIO_TASK_IDLE;
                clear_audiobuffer_hw(&mtask_audio_buf);
                /* only reset pointer, not content, content reset by AP */
                mtask_audio_ring_buf.pRead = mtask_audio_ring_buf.pBufBase;
                mtask_audio_ring_buf.pWrite = mtask_audio_ring_buf.pBufBase;
#ifndef FAKE_WRITE
                audio_dsp_hw_stop(hw_buf_handle);
#else
                audio_dsp_dummy_stop(hw_buf_handle);
#endif
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT

                if (initAurisysManagerflag == true) {
                    DestroyAurisysLibManager(task_offload_aurisys->task_aurisys, task_offload_aurisys->task_scene);
                    initAurisysManagerflag = false;
                    AUD_LOG_D("%s(), AUDIO_DSP_TASK_STOP: initAurisysManagerflag = %d \n", __FUNCTION__, initAurisysManagerflag);
                }
                if (task_offload_aurisys != NULL) {
                    AUDIO_FREE(task_offload_aurisys->mtask_processing_buf);
                    AUDIO_FREE_POINTER(task_offload_aurisys->task_aurisys->dsp_config);
                    FREE_ALOCK(task_offload_aurisys->task_aurisys->mAurisysLibManagerLock);
                    int ret = deinit_aurisys_task(&task_offload_aurisys);
                    AUD_LOG_D("%s(), AUDIO_DSP_TASK_STOP: deinit_aurisys_task\n", __func__);
                    if (ret) {
                        AUD_LOG_E("%s(), AUDIO_DSP_TASK_STOP: deinit_aurisys_task ret[%d]!!\n", __func__, ret);
                    }
                    task_offload_aurisys = NULL;
                }
#endif
                offload_deinit_blisrc();
                if (offload_op.task_deinit_decoder != NULL) {
                    offload_op.task_deinit_decoder();
                }
                bfirsttime = true;
                bfirsdecode = true;
                bmp3_src_remained = false;
                birq = false;
                mCurrentBSIdx = 0;
                decode_unsupport_format = false;
                decode_status = false;
                break;
            }
            case AUDIO_DSP_TASK_PCM_HWFREE: {
                AUD_LOG_D("%s()AUDIO_DSP_TASK_PCM_HWFREE \n", __FUNCTION__);
                pa_addr = (unsigned int)mtask_dsp_atod_sharemm.phy_addr;
                hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                           HAL_CACHE_OPERATION_INVALIDATE,
                                           (void *)pa_addr,
                                           mtask_dsp_atod_sharemm.size
                                          );
#ifndef FAKE_WRITE
                audio_dsp_hw_close(hw_buf_handle);
#else
                audio_dsp_dummy_close(hw_buf_handle);
#endif
                hw_buf_handle = -1;
                this->state = AUDIO_TASK_DEINIT;
                afe_offload_block.bIsClose = true;
                afe_offload_block.volume[1] = afe_offload_block.volume[0];
                task_offload_deinit(this);
                break;
            }
            case AUDIO_DSP_TASK_PCM_PREPARE: {
                AUD_LOG_D("%s()AUDIO_DSP_TASK_PCM_PREPARE \n", __FUNCTION__);
                pa_addr = (unsigned int)mtask_dsp_atod_sharemm.phy_addr;
                hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                           HAL_CACHE_OPERATION_INVALIDATE,
                                           (void *)pa_addr,
                                           mtask_dsp_atod_sharemm.size
                                          );
                get_audiobuf_from_msg(&uAudioTaskMsg, &mtask_audio_afebuf);
                dump_audio_hwbuffer(&mtask_audio_afebuf);
                task_offload_init(this);
                AUD_LOG_D("%s()task_offload_init done \n", __FUNCTION__);
#ifndef FAKE_WRITE
                hw_buf_handle = audio_dsp_hw_open(this, &mtask_audio_afebuf);
#else
                hw_buf_handle = audio_dsp_dummy_open(this, &mtask_audio_afebuf);
#endif
                this->state = AUDIO_TASK_INIT;
                break;
            case AUDIO_DSP_TASK_PCMDUMP_ON:
                pcmdump_enable = uAudioTaskMsg.param1;
                AUD_LOG_D("%s() AUDIO_DSP_TASK_PCMDUMP_ON, pcmdump_enable=%d \n", __FUNCTION__, pcmdump_enable);
                break;
           }
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
            case AUDIO_DSP_TASK_AURISYS_INIT:
                AUD_LOG_D("%s() AUDIO_DSP_TASK_AURISYS_INIT\n", __FUNCTION__);

                audio_get_dma_from_msg(ipi_msg, &aurisys_param_data_buf, &aurisys_param_data_size);
                if (aurisys_param_data_size < sizeof(struct aurisys_dsp_config_t)) {
                                       AUD_WARNING("dsp config fail");
                                       AUDIO_FREE_POINTER(aurisys_param_data_buf);
                break;
                }
                if (aurisys_param_data_size == sizeof(struct aurisys_dsp_config_t)) {
                                        AUD_WARNING("param fail");
                                        AUDIO_FREE_POINTER(aurisys_param_data_buf);
                break;
                }
                aurisys_param_list.data_size = aurisys_param_data_size - sizeof(struct aurisys_dsp_config_t);
                aurisys_param_list.memory_size = aurisys_param_list.data_size;
                aurisys_param_list.p_buffer = (uint8_t *)aurisys_param_data_buf + sizeof(struct aurisys_dsp_config_t);
                break;
            case AUDIO_DSP_TASK_AURISYS_DEINIT:
                AUD_LOG_D("%s() AUDIO_DSP_TASK_AURISYS_DEINIT\n", __FUNCTION__);
                AUDIO_FREE_POINTER(aurisys_param_data_buf);
                aurisys_param_data_buf = NULL;
                aurisys_param_list.p_buffer = NULL;
                aurisys_param_list.data_size = 0;
                aurisys_param_list.memory_size = 0;
                aurisys_param_data_size = 0;

                break;
            case AUDIO_DSP_TASK_AURISYS_LIB_PARAM:
                AUD_LOG_D("%s()offload :  AUDIO_DSP_TASK_AURISYS_LIB_PARAM\n", __FUNCTION__);
                audio_get_dma_from_msg(ipi_msg, &aurisys_param_data_buf, &aurisys_param_data_size);
                if (task_offload_aurisys->task_aurisys && task_offload_aurisys->task_aurisys->manager) {
                    ipi_msg->param1 = UpdateAurisysLibParam(task_offload_aurisys->task_aurisys,
                                                            aurisys_param_data_buf,
                                                            aurisys_param_data_size);
                }
                break;
#endif
        }
#endif
        audio_send_ipi_msg_ack_back(ipi_msg); //call this for need ack msg
        /* clean msg */
        AUD_LOG_D("memset id : %x\n", ipi_msg->msg_id);
        memset(ipi_msg, 0, sizeof(ipi_msg_t));
        this->num_queue_element--;
    }
}


uint8_t task_offload_preparsing_message(struct AudioTask *this,
                                            ipi_msg_t *ipi_msg)
{
    uint8_t ret = false;
    unsigned int pa_addr = 0;
    switch (ipi_msg->msg_id) {
        case OFFLOAD_SETWRITEBLOCK: {
             rMemDRAM->pWrite = (char *)ipi_msg->param1;
             pa_addr = (unsigned int)rMemDRAM->pBufBase;
             hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                        HAL_CACHE_OPERATION_INVALIDATE,
                                        (void *)pa_addr,
                                        rMemDRAM->bufLen
                                       );
             AUD_LOG_D("update OFFLOAD_SETWRITEBLOCK IDX = %p\n", rMemDRAM->pWrite);
        }
        break;
        case OFFLOAD_VOLUME:
             afe_offload_block.volume[1] =
             afe_offload_block.volume[0]; // record prev v.s. current volume
             afe_offload_block.volume[0] = ipi_msg->param1;
             AUD_LOG_D("update OFFLOAD_VOLUME = %d\n", afe_offload_block.volume[0]);
            break;
        case OFFLOAD_WRITEIDX:
             rMemDRAM->pWrite = (char *)ipi_msg->param1;
             AUD_LOG_D("update OFFLOAD_WRITEIDX IDX = %p\n", rMemDRAM->pWrite);
             pa_addr = (unsigned int)rMemDRAM->pBufBase;
             hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                        HAL_CACHE_OPERATION_INVALIDATE,
                                        (void *)pa_addr,
                                        rMemDRAM->bufLen
                                       );
            break;
        case OFFLOAD_TSTAMP:
            afe_offload_block.bgetTime = true;
            break;

        case OFFLOAD_CODEC_INFO:
            offload_codec_info.codec_bitrate = ipi_msg->param1;
            offload_codec_info.codec_samplerate = ipi_msg->param2;
            if (offload_codec_info.codec_bitrate == 0) {
                offload_codec_info.codec_bitrate = 128000;
            }
            AUD_LOG_D("update offload_codec_info.codec_bitrate = %d, offload_codec_info.codec_samplerate = %d\n",
                offload_codec_info.codec_bitrate, offload_codec_info.codec_samplerate);
            break;
        default:
            ret = true;
    }
    return ret;
}
static status_t task_offload_init(struct AudioTask *this)
{
    AUD_LOG_D("%s(+)\n", __func__);

    if (pBitConvertBuf == NULL) {
        pBitConvertBuf = kal_pvPortMalloc(BLISRC_BUFFER_SIZE);
        pOutputBuf = kal_pvPortMalloc(BLISRC_BUFFER_SIZE);
        pBSDump_BUf = kal_pvPortMalloc(BLISRC_BUFFER_SIZE);
        mOuputsize = 0;
    }
    //init decoder ---------------------------------------------------------
    offload_op.task_init_decoder();

    //init Sram addr & buffer-----------------------------------------------
    if (!bDoBack) {
        afe_offload_block.channels = mtask_audio_buf.aud_buffer.buffer_attr.channel;
        afe_offload_block.samplerate = mtask_audio_buf.aud_buffer.buffer_attr.rate;
        afe_offload_block.format = mtask_audio_buf.aud_buffer.buffer_attr.format;
        bDramPlayback = mtask_audio_afebuf.memory_type; //0: Audio Sram. 1: Audio Dram
        AUD_LOG_D("[process_init] channels = %d, samplerate = %d,format = %d, bDramPlayback %d\n",
                  afe_offload_block.channels, afe_offload_block.samplerate,
                  afe_offload_block.format,
                  bDramPlayback);

        RingBuf_Map_RingBuf_bridge(&mtask_audio_afebuf.aud_buffer.buf_bridge,
                                   &rMemDL);
        //DRAM setting ------------------------------------------------------------------
        rMemDRAM = &mtask_audio_ring_buf;
        AUD_LOG_D("[process_init] SRAM BufBase = %p, BufEnd = %p, Read = %p, Write = %p\n",
                  rMemDL.pBufBase, rMemDL.pBufEnd, rMemDL.pRead, rMemDL.pWrite);

        AUD_LOG_D("[process_init] DRAM BufBase = %p, BufEnd = %p, Read = %p, Write = %p\n",
                  rMemDRAM->pBufBase, rMemDRAM->pBufEnd, rMemDRAM->pRead, rMemDRAM->pWrite);
        AUD_LOG_D("[process_init] mtask_audio_ring_buf BufBase = %p, BufEnd = %p, Read = %p, Write = %p\n",
                  mtask_audio_ring_buf.pBufBase, mtask_audio_ring_buf.pBufEnd, mtask_audio_ring_buf.pRead, mtask_audio_ring_buf.pWrite);
        afe_offload_block.bIsDrain = false;
        afe_offload_block.bIsClose = false;
        afe_offload_block.bIsPause = false;
    }
#ifdef WAKELOCK
    wake_lock_init(&mp3_task_monitor_wakelock, "mp3wl");
    wake_lock(&mp3_task_monitor_wakelock);
#endif


    scp_wakeup_src_setup(AUDIO_WAKE_CLK_CTRL, true);
    // always enable dram
    if (bDramPlayback) {
        dvfs_enable_DRAM_resource(OPENDSP_MEM_ID);
    }
    dvfs_register_feature(OFFLOAD_FEATURE_ID);

    //AUD_LOG_V("%s(-) Freq = %d\n", __func__,get_cur_opp());
    return NO_ERROR;
}


static status_t task_offload_mp3_working(struct AudioTask *this)
{
    AUD_LOG_D("%s()\n", __func__);
    ipi_msg_t ipi_msg;
    unsigned int pa_addr ;

    unsigned int copysize = 0;
    int written = 0;
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    uint32_t expect_out_size = 0;
#endif

    offload_init_bsbuffer();
    offload_get_timestamp(this);
    offload_calc_dram();

    AUD_LOG_V(" task_offload_mp3_working: bs_buf.u4DataRemained = %d, mtask_audio_ring_buf.datacount = %d,rMemDRAM->datacount = %d, rMemDRAM->pWrite = %p, rMemDRAM->pRead=%p\n",
                  bs_buf.u4DataRemained, mtask_audio_ring_buf.datacount, rMemDRAM->datacount, rMemDRAM->pWrite, rMemDRAM->pRead);

    // check dram data
    if (!afe_offload_block.bIsDrain) {
        if ((rMemDRAM->datacount) < (DRAM_REMAIN_THRESHOLD >> 1)) {
            sync_bridge_ringbuf_readidx(&mtask_audio_buf.aud_buffer.buf_bridge,
                                        rMemDRAM);
            sync_bridge_ringbuf_writeidx(&mtask_audio_buf.aud_buffer.buf_bridge,
                                        rMemDRAM);

            AUD_LOG_D("SCP Need DATA!!! R : %p, W: %p, DRAM_REMAIN_THRESHOLD = %d, afe_offload_block.u4DataRemained = %d, rMemDRAM->datacount = %d\n",
                       rMemDRAM->pRead, rMemDRAM->pWrite, DRAM_REMAIN_THRESHOLD>>1, afe_offload_block.u4DataRemained, rMemDRAM->datacount);
            memcpy((void *)ap_to_scp(mtask_dsp_dtoa_sharemm.phy_addr),
                   (void *)&mtask_audio_buf, sizeof(struct audio_hw_buffer));


            pa_addr = (unsigned int)mtask_dsp_dtoa_sharemm.phy_addr;
            hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                       HAL_CACHE_OPERATION_FLUSH_INVALIDATE,
                                       (void *)pa_addr,
                                       sizeof(struct audio_hw_buffer)
                                      );

            ipi_msg_t ipi_msg_irqdl;
            audio_send_ipi_msg(&ipi_msg_irqdl,
                               this->scene,
                               AUDIO_IPI_LAYER_TO_KERNEL,
                               AUDIO_IPI_PAYLOAD,
                               AUDIO_IPI_MSG_BYPASS_ACK,
                               AUDIO_DSP_TASK_IRQDL,
                               sizeof(struct audio_hw_buffer),
                               0,
                               (char *)ap_to_scp(mtask_dsp_dtoa_sharemm.phy_addr));
        }
    }

    while (!afe_offload_block.bIsPause) {
        int bs_buf_needfill = 0;
        bs_buf_needfill = bs_buf.u4BufferSize - bs_buf.u4DataRemained;
        offload_fillbs_fromDram(bs_buf_needfill);

        if (birq == true) {//no space
            AUD_LOG_D(" birq = %d\n", birq);
            birq = false;
            break;
        }
        mp3_init_decoder();

        if (!bmp3_src_remained) {    //need to decode data
            mp3_decode_process();
        }

        AUD_LOG_V("MP3 DRAIN IDX = %p,  afe_offload_block.bIsDrain = %d, afe_offload_block.u4DataRemained = %d, SRAM_PERIODS_SIZE = %d\n",
            rMemDRAM->pWrite, afe_offload_block.bIsDrain,afe_offload_block.u4DataRemained,SRAM_PERIODS_SIZE);
        if ((decode_unsupport_format == true) || ((bs_buf.consumedBS == 1) && afe_offload_block.bIsDrain &&
            (afe_offload_block.u4DataRemained <= SRAM_PERIODS_SIZE))) {
            AUD_LOG_D("[DRAIN or ERROR] decode_unsupport_format = %d, bs_buf.consumedBS = %d, afe_offload_block.u4DataRemained = %d\n",
                      decode_unsupport_format, bs_buf.consumedBS, afe_offload_block.u4DataRemained);
            break;
        }

        unsigned int PCMDataSize = mMp3Dec->handle->PCMSamplesPerCH <<
                                   (mMp3Dec->handle->CHNumber);
        if (PCMDataSize <= 0) {
            PCMDataSize = offload_get_pcm_buffersize();
        }

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
         if (task_offload_aurisys == NULL) {
             int ret = init_aurisys_task(&task_offload_aurisys);
             if (ret) {
                 AUD_LOG_E("%s(), init_aurisys_task fail!!\n", __func__);
             }
             NEW_ALOCK(task_offload_aurisys->task_aurisys->mAurisysLibManagerLock);
             AUDIO_ALLOC_STRUCT(struct aurisys_dsp_config_t,
                                task_offload_aurisys->task_aurisys->dsp_config);
             memcpy(task_offload_aurisys->task_aurisys->dsp_config, aurisys_param_data_buf,
                    sizeof(struct aurisys_dsp_config_t));
             task_offload_aurisys->mtask_processing_buf_out_size = LOCAL_TASK_OUPUT_BUFFER_SIZE;
             task_offload_aurisys->mtask_processing_buf = (char *)AUDIO_MALLOC(LOCAL_TASK_OUPUT_BUFFER_SIZE);
             AUD_LOG_D("%s(),init_aurisys_task: LOCAL_TASK_OUPUT_BUFFER_SIZE = %d, task_offload_aurisys->mtask_processing_buf_out_size = %d\n", __func__, LOCAL_TASK_OUPUT_BUFFER_SIZE, task_offload_aurisys->mtask_processing_buf_out_size);
         }
         if (initAurisysManagerflag == false) {
            CreateAurisysLibManager(task_offload_aurisys->task_aurisys, &aurisys_param_list, task_offload_aurisys->task_scene);
            initAurisysManagerflag = true;
            AUD_LOG_D("%s()initAurisysManagerflag = %d \n", __FUNCTION__, initAurisysManagerflag);
         }
#endif

        if (!bmp3_src_remained) {
            offload_dump_pcm(MP3_DECODE_DUMP, (uint32_t)mMp3Dec->pcm_buf,
                         (uint32_t)PCMDataSize);
        }

        // pcm dump for debug
        if (pcmdump_enable == 1 && !bmp3_src_remained) {
            struct ipi_msg_t ipi_msg;
            char *data_buf = mMp3Dec->pcm_buf;
            uint32_t data_size = PCMDataSize;
            AUD_LOG_V("[MP3 DUMP2!]\n");

            audio_send_ipi_msg(&ipi_msg, this->scene,
                               AUDIO_IPI_LAYER_TO_HAL, AUDIO_IPI_DMA,
                               AUDIO_IPI_MSG_BYPASS_ACK,
                               AUDIO_DSP_TASK_PCMDUMP_DATA,
                               data_size,
                               DEBUG_PCMDUMP_OUT, //dump point serial number
                               data_buf);

        }

        void *pBufferBeforeBliSrc = mMp3Dec->pcm_buf;
        while (mCurrentBSIdx < PCMDataSize) {
            if (birq == true) {
                bmp3_src_remained = true;
                break;
            }
            char *data_buf = NULL;
            offload_init_blisrc(mMp3Dec->handle->sampleRateIndex, mMp3Dec->handle->CHNumber,
                            afe_offload_block.samplerate, afe_offload_block.channels);

            unsigned int DataPerSrc = PCMDataSize / (mCountForSrc);
            AUD_LOG_V("DataPerSrc = %d\n", DataPerSrc);
            pcm_buf.u4DataRemained += DataPerSrc;

            /* do blisrc 16in 16out*/
            void *pBufferAfterBliSrc = NULL;
            unsigned int bytesAfterBliSrc = 0;
#ifdef CYCLE
            volatile uint32_t mycount = 0;
            MP3_CPU_RESET_CYCLECOUNTER;
#endif
            offload_blisrc_process(pBufferBeforeBliSrc + mCurrentBSIdx, DataPerSrc,
                               &pBufferAfterBliSrc,
                               &bytesAfterBliSrc);
            AUD_LOG_V("offload_blisrc_process Done\n");

#ifdef CYCLE
            mycount = *MP3_DWT_CYCCNT;
            AUD_LOG_D("\n\r SRC Cycle %d %d\n", mycount, DataPerSrc);
#endif
            mCurrentBSIdx += DataPerSrc;

            /* do Volume Gain */
            void *pBufferAfterSetVolume = NULL;
            unsigned int  bytesAfterSetVolume = 0;
            offload_volume_process(pBufferAfterBliSrc, bytesAfterBliSrc,
                               &afe_offload_block.volume[0], &afe_offload_block.volume[1],
                               &pBufferAfterSetVolume, &bytesAfterSetVolume);

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
            memcpy((void *)task_offload_aurisys->mtask_processing_buf, pBufferAfterSetVolume, bytesAfterSetVolume);
            if (!get_aurisys_on()) {
                task_offload_aurisys->mtask_output_buf = (char *)task_offload_aurisys->mtask_processing_buf;
                task_offload_aurisys->mtask_output_buf_size = bytesAfterSetVolume;
            }
            else {
                task_offload_aurisys->mtask_output_buf = pOutputBuf;
                cal_output_buffer_size(&task_offload_aurisys->task_aurisys->dsp_config->attribute[DATA_BUF_DOWNLINK_IN],
                                       &task_offload_aurisys->task_aurisys->dsp_config->attribute[DATA_BUF_DOWNLINK_OUT],
                                       bytesAfterSetVolume, &expect_out_size);
                AUD_ASSERT(expect_out_size != 0);

                audio_pool_buf_copy_from_linear(task_offload_aurisys->task_aurisys->mAudioPoolBufDlIn,
                                                task_offload_aurisys->mtask_processing_buf,
                                                bytesAfterSetVolume);

                // post processing + SRC + Bit conversion
                aurisys_process_dl_only(task_offload_aurisys->task_aurisys->manager,
                                        task_offload_aurisys->task_aurisys->mAudioPoolBufDlIn,
                                        task_offload_aurisys->task_aurisys->mAudioPoolBufDlOut);

                // data alignment
                uint32_t data_size_align = audio_ringbuf_count(&task_offload_aurisys->task_aurisys->mAudioPoolBufDlOut->ringbuf);
                AUD_LOG_V("PCMDataSize = %d, DataPerSrc = %d, bytesAfterBliSrc = %d, bytesAfterSetVolume = %d, expect_out_size = %d, data_size_align = %d\n",
                           PCMDataSize, DataPerSrc, bytesAfterBliSrc, bytesAfterSetVolume, expect_out_size, data_size_align);

                if (data_size_align != expect_out_size) {
                    AUD_LOG_W("data2_size_align = %d, expect_out_size = %d", data_size_align,
                               expect_out_size);
                }
                data_size_align &= (~0x7F);
                audio_pool_buf_copy_to_linear(&task_offload_aurisys->task_aurisys->mLinearOut->p_buffer,
                                              &task_offload_aurisys->task_aurisys->mLinearOut->memory_size,
                                              task_offload_aurisys->task_aurisys->mAudioPoolBufDlOut,
                                              data_size_align);
                task_offload_aurisys->mtask_output_buf = task_offload_aurisys->task_aurisys->mLinearOut->p_buffer;
                task_offload_aurisys->mtask_output_buf_size = data_size_align;

                AUDIO_CHECK(task_offload_aurisys->mtask_processing_buf);
                AUDIO_CHECK(task_offload_aurisys->mtask_output_buf);
                AUDIO_CHECK(task_offload_aurisys->task_aurisys->mLinearOut->p_buffer);
                AUDIO_CHECK(task_offload_aurisys->task_aurisys->mAudioPoolBufDlOut->ringbuf.base);
            }

#ifndef FAKE_WRITE
            AUD_LOG_V("MP3 write to SRAM copysize=%d\n",copysize);
            written = audio_dsp_hw_write(hw_buf_handle, (void*)task_offload_aurisys->mtask_output_buf, task_offload_aurisys->mtask_output_buf_size);
            copysize = task_offload_aurisys->mtask_output_buf_size;
            data_buf = task_offload_aurisys->mtask_output_buf;
#else
            written = audio_dsp_dummy_write(hw_buf_handle, (void*)task_offload_aurisys->mtask_output_buf,task_offload_aurisys->mtask_output_buf_size);
#endif
#else

            void *pBufferAfterBitConvertion = NULL;
            unsigned int  bytesAfterBitConvertion = 0;

            offload_bitconvert_process(pBufferAfterSetVolume, bytesAfterSetVolume,
                                   &pBufferAfterBitConvertion, &bytesAfterBitConvertion);

            copysize = offload_calc_datasize(mOuputsize, bytesAfterBitConvertion);
            unsigned int remainsize = (mOuputsize + bytesAfterBitConvertion) - copysize;
            memcpy(pOutputBuf + mOuputsize , pBufferAfterBitConvertion , bytesAfterBitConvertion);

            AUD_LOG_V("MP3 output_buffer process , bytesAfterBitConvertion = %d, mOuputsize = %d, copysize = %d, remainsize = %d\n",
                       bytesAfterBitConvertion, mOuputsize, copysize, remainsize);

#ifndef FAKE_WRITE
            AUD_LOG_V("MP3 write to SRAM bytesAfterBitConvertion=%d\n",bytesAfterBitConvertion);
            written = audio_dsp_hw_write(hw_buf_handle, (void*)pOutputBuf, copysize);
            AUD_LOG_V("MP3 write to SRAM Done, written=%d\n", written);
            data_buf = pOutputBuf;

#else
            written = audio_dsp_dummy_write(hw_buf_handle, (void*)pOutputBuf, copysize);
#endif
#endif
            if (pcmdump_enable == 1) {
                struct ipi_msg_t ipi_msg;
                uint32_t data_size = written;
                AUD_LOG_V("[MP3 DUMP3!]\n");
                audio_send_ipi_msg(&ipi_msg, this->scene,
                                    AUDIO_IPI_LAYER_TO_HAL, AUDIO_IPI_DMA,
                                    AUDIO_IPI_MSG_BYPASS_ACK,
                                    AUDIO_DSP_TASK_PCMDUMP_DATA,
                                    data_size,
                                    DEBUG_PCMDUMP_RESERVED, //dump point serial number
                                    data_buf);
            }

#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
            memcpy(pOutputBuf, pOutputBuf + copysize , remainsize);
            mOuputsize = remainsize;
#endif

        if (written != copysize) {
                AUD_LOG_E("%s AUDIO_DSP_TASK_DLCOPY written = %d\n", __func__, written);
            }
        }
        if (mCurrentBSIdx == PCMDataSize) {
            mCurrentBSIdx = 0;
            bmp3_src_remained = false;
        }
    }
    if ((decode_unsupport_format == true) || ((bs_buf.consumedBS == 1) && afe_offload_block.bIsDrain &&
        (afe_offload_block.u4DataRemained <= SRAM_PERIODS_SIZE))) {
        AUD_LOG_D("DECODE FINISHED!! Send DARIN DONE IPI %d!!!\n", this->scene);

        audio_send_ipi_msg(&ipi_msg, this->scene, AUDIO_IPI_LAYER_TO_KERNEL,
                           AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                           OFFLOAD_DRAINDONE, 0, 0, NULL);
        audio_dsp_hw_stop(hw_buf_handle); //stop irq
    }
    return NO_ERROR;
}

static status_t task_offload_aac_working(struct AudioTask *this)
{
    AUD_LOG_D("%s()\n", __func__);
    ipi_msg_t ipi_msg;
    unsigned int pa_addr ;
    unsigned int copysize = 0;
    int written = 0;
#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
    uint32_t expect_out_size = 0;
#endif

    pa_addr = (unsigned int)rMemDRAM->pBufBase;
    hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                               HAL_CACHE_OPERATION_INVALIDATE,
                               (void *)pa_addr,
                               rMemDRAM->bufLen
                              );

    offload_init_bsbuffer();
    offload_get_timestamp(this);
    offload_calc_dram();

    // check dram data
    if (!afe_offload_block.bIsDrain) {
        if ((rMemDRAM->datacount) < (DRAM_REMAIN_THRESHOLD >> 1)) {
            /* sync with buf_bridge*/
            sync_bridge_ringbuf_readidx(&mtask_audio_buf.aud_buffer.buf_bridge,
                                        rMemDRAM);
            sync_bridge_ringbuf_writeidx(&mtask_audio_buf.aud_buffer.buf_bridge,
                                         rMemDRAM);
            AUD_LOG_D("SCP Need DATA!!! R : %p, W: %p\n",
                       rMemDRAM->pRead, rMemDRAM->pWrite);
            memcpy((void *)ap_to_scp(mtask_dsp_dtoa_sharemm.phy_addr),
                   (void *)&mtask_audio_buf, sizeof(struct audio_hw_buffer));

            pa_addr = (unsigned int)mtask_dsp_dtoa_sharemm.phy_addr;
            hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                                       HAL_CACHE_OPERATION_FLUSH_INVALIDATE,
                                       (void *)pa_addr,
                                       sizeof(struct audio_hw_buffer)
                                       );
            ipi_msg_t ipi_msg_irqdl;
            audio_send_ipi_msg(&ipi_msg_irqdl,
                               this->scene,
                               AUDIO_IPI_LAYER_TO_KERNEL,
                               AUDIO_IPI_PAYLOAD,
                               AUDIO_IPI_MSG_BYPASS_ACK,
                               AUDIO_DSP_TASK_IRQDL,
                               sizeof(struct audio_hw_buffer),
                               0,
                               (char *)ap_to_scp(mtask_dsp_dtoa_sharemm.phy_addr));
        }
    }

    while (!afe_offload_block.bIsPause) {
        int bs_buf_needfill = 0;

        bs_buf_needfill = bs_buf.u4BufferSize - bs_buf.u4DataRemained;
        offload_fillbs_fromDram(bs_buf_needfill);

        if (birq == true) {//no space
            birq = false;
            break;
        }

        aac_init_decoder();

        if (!baac_src_remained) {    //need to decode data
            aac_decode_process();
        }

        if  (decode_unsupport_format == true || (((bs_buf.u4DataRemained <= 0) || (decode_status == false)) &&
              ((afe_offload_block.bIsDrain == true) && (afe_offload_block.u4DataRemained <= SRAM_PERIODS_SIZE)))) {
            AUD_LOG_D("[DRAIN or ERROR]afe_offload_block.bIsDrain == %d afe_offload_block.u4DataRemained = %d, decode_unsupport_format = %d\n",
                      afe_offload_block.bIsDrain, afe_offload_block.u4DataRemained, decode_unsupport_format);
            break;
        }

        unsigned int PCMDataSize = (mAacDec->handle->FrameSize) << (mAacDec->handle->ChannelNum);

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
        if (task_offload_aurisys == NULL) {
            int ret = init_aurisys_task(&task_offload_aurisys);
            if (ret) {
                AUD_LOG_E("%s(), init_aurisys_task fail!!\n", __func__);
            }
            NEW_ALOCK(task_offload_aurisys->task_aurisys->mAurisysLibManagerLock);
            AUDIO_ALLOC_STRUCT(struct aurisys_dsp_config_t,
                               task_offload_aurisys->task_aurisys->dsp_config);
            memcpy(task_offload_aurisys->task_aurisys->dsp_config, aurisys_param_data_buf,
                   sizeof(struct aurisys_dsp_config_t));
            task_offload_aurisys->mtask_processing_buf_out_size = LOCAL_TASK_OUPUT_BUFFER_SIZE;
            task_offload_aurisys->mtask_processing_buf = (char *)AUDIO_MALLOC(LOCAL_TASK_OUPUT_BUFFER_SIZE);
            AUD_LOG_D("%s(),init_aurisys_task: LOCAL_TASK_OUPUT_BUFFER_SIZE = %d, task_offload_aurisys->mtask_processing_buf_out_size = %d\n", __func__, LOCAL_TASK_OUPUT_BUFFER_SIZE, task_offload_aurisys->mtask_processing_buf_out_size);
        }
        if (initAurisysManagerflag == false) {
           CreateAurisysLibManager(task_offload_aurisys->task_aurisys, &aurisys_param_list, task_offload_aurisys->task_scene);
           initAurisysManagerflag = true;
           AUD_LOG_D("%s()initAurisysManagerflag = %d \n", __FUNCTION__, initAurisysManagerflag);
        }
#endif

        if (!baac_src_remained) {
            offload_dump_pcm(MP3_DECODE_DUMP, (uint32_t)mAacDec->pcm_buf,
                         (uint32_t)PCMDataSize);
        }

        if (pcmdump_enable == 1 && !baac_src_remained) {
            struct ipi_msg_t ipi_msg;
            char *data_buf = mAacDec->pcm_buf;
            uint32_t data_size = PCMDataSize;
            AUD_LOG_V("[AAC DUMP2!]\n");

            audio_send_ipi_msg(&ipi_msg, this->scene,
                               AUDIO_IPI_LAYER_TO_HAL, AUDIO_IPI_DMA,
                               AUDIO_IPI_MSG_BYPASS_ACK,
                               AUDIO_DSP_TASK_PCMDUMP_DATA,
                               data_size,
                               DEBUG_PCMDUMP_OUT, //dump point serial number
                               data_buf);

        }

        void *pBufferBeforeBliSrc = mAacDec->pcm_buf;
        while (mCurrentBSIdx < PCMDataSize) {
            if (birq == true) {
                baac_src_remained = true;
                break;
            }
            char *data_buf = NULL;
            offload_init_blisrc(mAacDec->handle->SampleRate, mAacDec->handle->ChannelNum,
                            afe_offload_block.samplerate, afe_offload_block.channels);

            while (PCMDataSize % mCountForSrc != 0)
            {
                mCountForSrc++;
            }

            unsigned int DataPerSrc = PCMDataSize / (mCountForSrc);
            AUD_LOG_V("DataPerSrc = %d\n", DataPerSrc);
            pcm_buf.u4DataRemained += DataPerSrc;

            /* do blisrc 16in 16out*/
            void *pBufferAfterBliSrc = NULL;
            unsigned int bytesAfterBliSrc = 0;
#ifdef CYCLE
            volatile uint32_t mycount = 0;
            MP3_CPU_RESET_CYCLECOUNTER;
#endif
            offload_blisrc_process(pBufferBeforeBliSrc + mCurrentBSIdx, DataPerSrc,
                               &pBufferAfterBliSrc,
                               &bytesAfterBliSrc);
#ifdef CYCLE
            mycount = *MP3_DWT_CYCCNT;
            AUD_LOG_D("SRC Cycle %d %d\n", mycount, DataPerSrc);
#endif
            mCurrentBSIdx += DataPerSrc;

            /* do Volume Gain */
            void *pBufferAfterSetVolume = NULL;
            unsigned int  bytesAfterSetVolume = 0;
            offload_volume_process(pBufferAfterBliSrc, bytesAfterBliSrc,
                               &afe_offload_block.volume[0], &afe_offload_block.volume[1],
                               &pBufferAfterSetVolume, &bytesAfterSetVolume);

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
            memcpy((void *)task_offload_aurisys->mtask_processing_buf, pBufferAfterSetVolume, bytesAfterSetVolume);
            if (!get_aurisys_on()) {
                task_offload_aurisys->mtask_output_buf = (char *)task_offload_aurisys->mtask_processing_buf;
                task_offload_aurisys->mtask_output_buf_size = bytesAfterSetVolume;
            }
            else {
                task_offload_aurisys->mtask_output_buf = pOutputBuf;
                cal_output_buffer_size(&task_offload_aurisys->task_aurisys->dsp_config->attribute[DATA_BUF_DOWNLINK_IN],
                                       &task_offload_aurisys->task_aurisys->dsp_config->attribute[DATA_BUF_DOWNLINK_OUT],
                                       bytesAfterSetVolume, &expect_out_size);
                AUD_ASSERT(expect_out_size != 0);

                audio_pool_buf_copy_from_linear(task_offload_aurisys->task_aurisys->mAudioPoolBufDlIn,
                                                task_offload_aurisys->mtask_processing_buf,
                                                bytesAfterSetVolume);

                // post processing + SRC + Bit conversion
                aurisys_process_dl_only(task_offload_aurisys->task_aurisys->manager,
                                        task_offload_aurisys->task_aurisys->mAudioPoolBufDlIn,
                                        task_offload_aurisys->task_aurisys->mAudioPoolBufDlOut);

                // data alignment
                uint32_t data_size_align = audio_ringbuf_count(&task_offload_aurisys->task_aurisys->mAudioPoolBufDlOut->ringbuf);
                AUD_LOG_V("PCMDataSize = %d, DataPerSrc = %d, bytesAfterBliSrc = %d, bytesAfterSetVolume = %d, expect_out_size = %d, data_size_align = %d\n",
                           PCMDataSize, DataPerSrc, bytesAfterBliSrc, bytesAfterSetVolume, expect_out_size, data_size_align);

                if (data_size_align != expect_out_size) {
                    AUD_LOG_W("data_size_align = %d, expect_out_size = %d", data_size_align,
                               expect_out_size);
                }
                data_size_align &= (~0x7F);
                audio_pool_buf_copy_to_linear(&task_offload_aurisys->task_aurisys->mLinearOut->p_buffer,
                                              &task_offload_aurisys->task_aurisys->mLinearOut->memory_size,
                                              task_offload_aurisys->task_aurisys->mAudioPoolBufDlOut,
                                              data_size_align);
                task_offload_aurisys->mtask_output_buf = task_offload_aurisys->task_aurisys->mLinearOut->p_buffer;
                task_offload_aurisys->mtask_output_buf_size = data_size_align;

                AUDIO_CHECK(task_offload_aurisys->mtask_processing_buf);
                AUDIO_CHECK(task_offload_aurisys->mtask_output_buf);
                AUDIO_CHECK(task_offload_aurisys->task_aurisys->mLinearOut->p_buffer);
                AUDIO_CHECK(task_offload_aurisys->task_aurisys->mAudioPoolBufDlOut->ringbuf.base);
            }
            //copysize = task_offload_aurisys->mtask_output_buf_size;
#ifndef FAKE_WRITE
            AUD_LOG_V("AAC write to SRAM copysize=%d\n",copysize);
            written = audio_dsp_hw_write(hw_buf_handle, (void*)task_offload_aurisys->mtask_output_buf, task_offload_aurisys->mtask_output_buf_size);
            copysize = task_offload_aurisys->mtask_output_buf_size;
            data_buf = task_offload_aurisys->mtask_output_buf;
#else
            written = audio_dsp_dummy_write(hw_buf_handle, (void*)task_offload_aurisys->mtask_output_buf,task_offload_aurisys->mtask_output_buf_size);
#endif
#else

            void *pBufferAfterBitConvertion = NULL;
            unsigned int  bytesAfterBitConvertion = 0;
            offload_bitconvert_process(pBufferAfterSetVolume, bytesAfterSetVolume,
                                   &pBufferAfterBitConvertion, &bytesAfterBitConvertion);

            copysize = offload_calc_datasize(mOuputsize, bytesAfterBitConvertion);
            unsigned int remainsize = (mOuputsize + bytesAfterBitConvertion) - copysize;
            memcpy(pOutputBuf + mOuputsize , pBufferAfterBitConvertion , bytesAfterBitConvertion);


            AUD_LOG_V("AAC output_buffer process , bytesAfterBitConvertion = %d, mOuputsize = %d, copysize = %d, remainsize = %d\n",
                       bytesAfterBitConvertion, mOuputsize, copysize, remainsize);

#ifndef FAKE_WRITE
            AUD_LOG_D("AAC write to SRAM bytesAfterBitConvertion=%d\n",bytesAfterBitConvertion);
            written = audio_dsp_hw_write(hw_buf_handle, (void*)pOutputBuf, copysize);
            AUD_LOG_V("AAC write to SRAM Done, written=%d\n", written);
            data_buf = pOutputBuf;

#else
            written = audio_dsp_dummy_write(hw_buf_handle, (void*)pOutputBuf, copysize);
#endif
#endif

            if (written != copysize) {
                AUD_LOG_E("%s AUDIO_DSP_TASK_DLCOPY written = %d\n", __func__, written);
            }
            if (pcmdump_enable == 1) {
                struct ipi_msg_t ipi_msg;
                uint32_t data_size = written;
                AUD_LOG_V("[MP3 DUMP3!]\n");
                audio_send_ipi_msg(&ipi_msg, this->scene,
                                    AUDIO_IPI_LAYER_TO_HAL, AUDIO_IPI_DMA,
                                    AUDIO_IPI_MSG_BYPASS_ACK,
                                    AUDIO_DSP_TASK_PCMDUMP_DATA,
                                    data_size,
                                    DEBUG_PCMDUMP_RESERVED, //dump point serial number
                                    data_buf);
            }

#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
            memcpy(pOutputBuf, pOutputBuf + copysize , remainsize);
            mOuputsize = remainsize;
#endif
        }
        if (mCurrentBSIdx >= PCMDataSize) {
            mCurrentBSIdx = 0;
            baac_src_remained = false;
        }
        if (afe_offload_block.bIsDrain && (((bs_buf.u4DataRemained <= 0) || (decode_status == false)) &&
            (afe_offload_block.u4DataRemained <= SRAM_PERIODS_SIZE))) {
            AUD_LOG_D("2.SRAM_PERIODS_SIZE  = %d, afe_offload_block.u4DataRemained = %d, rMemDRAM->datacount = %d, afe_offload_block.bIsDrain = %d",
                      SRAM_PERIODS_SIZE, afe_offload_block.u4DataRemained, rMemDRAM->datacount, afe_offload_block.bIsDrain);
            break;
        }
    }

    if (decode_unsupport_format == true || (((bs_buf.u4DataRemained <= 0) || (decode_status == false))
        && ((afe_offload_block.bIsDrain == true) && (afe_offload_block.u4DataRemained <= SRAM_PERIODS_SIZE)))) {

        AUD_LOG_D("DECODE FINISHED!! Send DARIN DONE IPI !!!\n");
        audio_send_ipi_msg(&ipi_msg, this->scene, AUDIO_IPI_LAYER_TO_KERNEL,
                           AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                           OFFLOAD_DRAINDONE, 0, 0, NULL);
        audio_dsp_hw_stop(hw_buf_handle); //stop irq
    }
    return NO_ERROR;
}

static status_t task_offload_deinit(struct AudioTask *this)
{
    AUD_LOG_D("%s()+\n", __func__);
    bfirsttime = true;
    bfirsdecode = true;
    bmp3_src_remained = false;
    birq = false;
    mCurrentBSIdx = 0;
    // free working memory..
    if (offload_op.task_deinit_decoder != NULL) {
        offload_op.task_deinit_decoder();
    }
    // free malloc memory
    if (pBitConvertBuf != NULL) {
        kal_vPortFree(pBitConvertBuf);
        kal_vPortFree(pOutputBuf);
        kal_vPortFree(pBSDump_BUf);

        pBitConvertBuf = NULL;
        pOutputBuf = NULL;
        pBSDump_BUf = NULL;
        mOuputsize = 0;
    }
    if (pRemainedPCM_Buf != NULL) {
        kal_vPortFree(pRemainedPCM_Buf);
        pRemainedPCM_Buf = NULL;
    }

    dvfs_deregister_feature(OFFLOAD_FEATURE_ID);
    if (bDramPlayback) {
        dvfs_disable_DRAM_resource(OPENDSP_MEM_ID);
    }
    scp_wakeup_src_setup(AUDIO_WAKE_CLK_CTRL, false);
#ifdef WAKELOCK
    wake_unlock(&mp3_task_monitor_wakelock);
    wake_lock_deinit(&mp3_task_monitor_wakelock);
#endif
    clear_audiobuffer_hw(&mtask_audio_buf);
    RingBuf_Reset(&mtask_audio_ring_buf);
    bDoBack = false;
    AUD_LOG_D("%s()-\n", __func__);
    return NO_ERROR;
}

static void *get_resv_dram_buf(const uint32_t len)
{
    // TODO: ring buf
    void *retval = (void *)dram_dump.pWrite;

    if (retval + len < (void *)dram_dump.pBufEnd) {
        dram_dump.pWrite += len;
    }
    else {
        retval = (void *)dram_dump.pBufBase;
        dram_dump.pWrite = dram_dump.pBufBase + len;
    }

    return retval;
}

static unsigned int offload_calc_datasize(unsigned int olddatasize, unsigned int newdatasize)
{
    unsigned int  availsize = (olddatasize + newdatasize) / CACHE_LINE_SIZE;
    unsigned int  copysize = 0;

    if ((olddatasize + newdatasize) % CACHE_LINE_SIZE) {
        copysize = (availsize * CACHE_LINE_SIZE) > (olddatasize + newdatasize) ?
                    ((availsize - 1) * CACHE_LINE_SIZE) : (availsize * CACHE_LINE_SIZE);
    } else {
        copysize = olddatasize + newdatasize;
    }

    return copysize;
}


static void offload_dump_pcm(DEBUG_DUMP_OFFLOAD flag, uint32_t dump_buf_src,
                         uint32_t dump_buf_size)
{
    ipi_msg_t ipi_msg;


    if (flag == dump_pcm_flag) {

        dump_total += dump_buf_size;

        uint32_t dst_addr = (uint32_t)get_resv_dram_buf(dump_buf_size);
        mp3_dma_transaction_wrap((uint32_t)dst_addr, dump_buf_src,
                                 dump_buf_size, bDramPlayback);

        if (dump_buf_size != offload_get_pcm_buffersize() && (dump_total % offload_get_pcm_buffersize()) != 0) {
            dst_addr_sram  = dst_addr;
        }
        else {
            dst_addr_sram  = dst_addr;
            dump_total = offload_get_pcm_buffersize();
        }

        if (flag == MP3_AUDIOSRAM_DUMP) {
            if ((dump_total % offload_get_pcm_buffersize()) == 0) {

                audio_send_ipi_msg(&ipi_msg, TASK_SCENE_PLAYBACK_MP3,
                                   AUDIO_IPI_LAYER_TO_KERNEL, AUDIO_IPI_DMA,
                                   AUDIO_IPI_MSG_NEED_ACK,
                                   OFFLOAD_PCMDUMP_OK, offload_get_pcm_buffersize(), 0,
                                   (char *)scp_to_ap((uint32_t)dst_addr_sram));

                dump_total = offload_get_pcm_buffersize();

            }

        }
        else {
            audio_send_ipi_msg(&ipi_msg, TASK_SCENE_PLAYBACK_MP3,
                               AUDIO_IPI_LAYER_TO_KERNEL, AUDIO_IPI_DMA,
                               AUDIO_IPI_MSG_NEED_ACK,
                               OFFLOAD_PCMDUMP_OK, dump_buf_size, 0, (char *)scp_to_ap((uint32_t)dst_addr));
        }

        AUD_LOG_V("dump_pcm_flag = %d dump_total = %d datasize = %d, pwrite = %p\n",
                  dump_pcm_flag, dump_total, dump_buf_size,
                  scp_to_ap((uint32_t)dst_addr));
    }
}

/*******************************************************
*                             MP3 Decoder API
*
*********************************************************/

int offload_get_bs_buffersize(void)
{
    int buffer_size = 0;
#ifdef AudioDecoder
    switch(OFFLOAD_SCENE_TYPE) {
        case TASK_SCENE_OFFLOAD_MP3: {
             buffer_size = mMp3Dec->min_bs_size;
            }
             break;
        case TASK_SCENE_OFFLOAD_AAC: {
             buffer_size = mAacDec->min_bs_size;
            }
             break;
        }
#endif
    return  buffer_size;

}

int offload_get_pcm_buffersize(void)
{
    return  pcm_buffer_size;
}

void offload_calc_dram(void)
{

    if (rMemDRAM->pWrite > rMemDRAM->pRead) {
        rMemDRAM->datacount = (uint32_t) (rMemDRAM->pWrite - rMemDRAM->pRead);
        AUD_LOG_V("offload_calc_dram(), 1. mtask_audio_ring_buf.datacount = %d,rMemDRAM->datacount = %d, rMemDRAM->pWrite = %p, rMemDRAM->pRead=%p, rMemDRAM->pBufBase = %p, rMemDRAM->pBufEnd = %p\n",
                   mtask_audio_ring_buf.datacount, rMemDRAM->datacount, rMemDRAM->pWrite, rMemDRAM->pRead, rMemDRAM->pBufBase, rMemDRAM->pBufEnd);
    }
    else if (rMemDRAM->pWrite < rMemDRAM->pRead){
        rMemDRAM->datacount = (uint32_t)(rMemDRAM->pBufEnd -  rMemDRAM->pRead + rMemDRAM->pWrite - rMemDRAM->pBufBase);
        AUD_LOG_V("offload_calc_dram(), 2. mtask_audio_ring_buf.datacount = %d,rMemDRAM->datacount = %d, rMemDRAM->pWrite = %p, rMemDRAM->pRead=%p, rMemDRAM->pBufBase = %p, rMemDRAM->pBufEnd = %p\n",
                   mtask_audio_ring_buf.datacount, rMemDRAM->datacount, rMemDRAM->pWrite, rMemDRAM->pRead, rMemDRAM->pBufBase, rMemDRAM->pBufEnd);
    } else {
        rMemDRAM->datacount = 0;
    }
    afe_offload_block.u4DataRemained = RingBuf_getDataCount(rMemDRAM);

}


static int offload_init_bsbuffer()
{
#ifdef AudioDecoder

    uint32_t bs_addr_old = (uint32_t)bs_buf.pBufAddr;
    uint32_t bs_addr_new = 0;
    uint32_t bs_addr_offset = 0;

    offload_calc_dram();

    if (bDoBack && bfirsttime) {
        bs_buf.pBufAddr = NULL;
        bs_buf.pBufAddr = kal_pvPortMalloc(bs_buf.u4BufferSize + 3);   // add backup bit
        AUD_ASSERT(bs_buf.pBufAddr != NULL);
        bs_addr_new = (uint32_t) bs_buf.pBufAddr;
        bs_addr_offset = (bs_addr_old > bs_addr_new) ? bs_addr_old - bs_addr_new :
                         bs_addr_new - bs_addr_old;
        bs_buf.pReadIdx += bs_addr_offset;
        mp3_dma_transaction_wrap((uint32_t)bs_buf.pBufAddr, (uint32_t)rMemDRAM->pBufEnd,
                                 bs_buf.u4BufferSize, true);
        if (bsAddData)
            mp3_dma_transaction_wrap((uint32_t)(bs_buf.pBufAddr + bs_buf.u4BufferSize),
                                     (uint32_t)(rMemDRAM->pBufEnd + bs_buf.u4BufferSize), bsAddData, true);
        bfirsttime = false;
        afe_offload_block.bIsPause = false;
    }
    else {
        if (bfirsttime) {
            memset(&pcm_buf, 0, sizeof(pcm_buf));
            bs_buf.consumedBS = 0;
            // copy data from dram to bs buffer
            if (afe_offload_block.u4DataRemained > bs_buf.u4BufferSize) {
                bs_buf.u4BufferSize = offload_op.BSBUFFER_SIZE * 2;
                bs_buf.bneedFill = true;
            }
            else {
                bs_buf.u4BufferSize = afe_offload_block.u4DataRemained;
                bs_buf.bneedFill = false;
            }
            if (bs_buf.pBufAddr == NULL) {
                bs_buf.pBufAddr = kal_pvPortMalloc(bs_buf.u4BufferSize + 3);   // add backup bit
                out_buf.u4BufferSize = bs_buf.u4BufferSize + ADIF_HEADER_TEMPLATE_LEN;
                out_buf.pBufAddr = kal_pvPortMalloc(out_buf.u4BufferSize + 3);
                AUD_ASSERT(bs_buf.pBufAddr != NULL);
                memset(bs_buf.pBufAddr, 0, bs_buf.u4BufferSize + 3);
                memset(out_buf.pBufAddr, 0, out_buf.u4BufferSize + 3);
            }
            else {
                memset(bs_buf.pBufAddr, 0, bs_buf.u4BufferSize + 3);
            }
            flush_dram((unsigned int)rMemDRAM->pBufBase, bs_buf.u4BufferSize);
            mp3_dma_transaction_wrap((uint32_t)bs_buf.pBufAddr,
                                     (uint32_t)rMemDRAM->pBufBase,
                                     bs_buf.u4BufferSize, true);

            bs_buf.u4DataRemained = bs_buf.u4BufferSize;
            bs_buf.pReadIdx = bs_buf.pBufAddr;
            out_buf.pReadIdx = out_buf.pBufAddr;

            rMemDRAM->pRead = rMemDRAM->pRead + bs_buf.u4BufferSize;
            bfirsttime = false;
            AUD_LOG_D("initBsBuffer :bs_buf.pBufAddr =%p bs_buf.u4BufferSize = %d\n",
                      bs_buf.pBufAddr, bs_buf.u4BufferSize);
        }
    }
    AUD_LOG_V("initBsBuffer2 :bs_buf.pBufAddr =%p, bs_buf.u4BufferSize = %d\n",
              bs_buf.pBufAddr, bs_buf.u4BufferSize);
#endif
    return NO_ERROR;
}

static void flush_dram(unsigned int pBufAddr, int len)
{
    unsigned int pa_addr = pBufAddr;
    hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                               HAL_CACHE_OPERATION_INVALIDATE,
                               (void *)pa_addr,
                               len
                              );

}

static int offload_fillbs_fromDram(uint32_t bs_fill_size)
{


    int ret = 0;
    uint32_t need_add = 0;
    uint32_t bs_threshold = 0;
    bs_threshold = (offload_get_bs_buffersize() >> 2);

#ifdef AudioDecoder
    /*Copy DRAM data to decode buffer*/
    if (bs_fill_size % 4 != 0) {
         bs_fill_size -= (bs_fill_size % 4);
    }
    if (bs_fill_size == 0) {
        AUD_LOG_D(" offload_fillbs_fromDram no need fill bs_threshold = %d, return\n", bs_threshold);
        return 0;
    }

    offload_calc_dram();

    if (bs_buf.bneedFill &&
        (bs_buf.u4DataRemained <= bs_threshold)) {

        AUD_LOG_D(" + offload_fillbs_fromDram, bs_fill_size = %d, bs_buf.u4DataRemained = %d, bs_threshold = %d, BsbufferSize = %d\n",
            bs_fill_size, bs_buf.u4DataRemained, bs_threshold, offload_get_bs_buffersize());
        AUD_LOG_D("offload_fillbs_fromDram :bs_buf.pBufAddr =%p bs_buf.pReadIdx = %p, bs_buf.u4BufferSize = %d\n",
                      bs_buf.pBufAddr, bs_buf.pReadIdx, bs_buf.u4BufferSize);
        memmove(bs_buf.pBufAddr, bs_buf.pReadIdx, bs_buf.u4DataRemained);
        bs_buf.pReadIdx = bs_buf.pBufAddr;

        /*should only happened in Drain state, check is drain*/
        if (afe_offload_block.u4DataRemained <= bs_fill_size) {
            if (afe_offload_block.bIsDrain) {
                if (rMemDRAM->pWrite > rMemDRAM->pRead) {
                    flush_dram((unsigned int)rMemDRAM->pRead, ((afe_offload_block.u4DataRemained >> 2) << 2));
                    ret = mp3_dma_transaction_wrap((uint32_t)(bs_buf.pBufAddr + bs_buf.u4DataRemained),
                                                   (uint32_t)rMemDRAM->pRead,
                                                   ((afe_offload_block.u4DataRemained >> 2) << 2), true);
                    AUD_LOG_V("[offload_fillbs_fromDram] COPY01. DRAM DATA: afe_offload_block.u4DataRemained = %d < bs_buf_needfill = %d \n",
                              afe_offload_block.u4DataRemained, bs_fill_size);
                }
                else {
                    int data_length =  rMemDRAM->pBufEnd -  rMemDRAM->pRead;
                    flush_dram((unsigned int)rMemDRAM->pRead, data_length);
                    ret = mp3_dma_transaction_wrap((uint32_t)(bs_buf.pBufAddr + bs_buf.u4DataRemained),
                                                   (uint32_t)rMemDRAM->pRead,
                                                   data_length, true);

                    data_length = (uint32_t)(rMemDRAM->pWrite - rMemDRAM->pBufBase);
                    flush_dram((unsigned int)rMemDRAM->pBufBase, data_length);
                    ret = mp3_dma_transaction_wrap((uint32_t)(bs_buf.pBufAddr + bs_buf.u4DataRemained + data_length),
                                                   (uint32_t)rMemDRAM->pBufBase,
                                                   (uint32_t)(rMemDRAM->pWrite - rMemDRAM->pBufBase), true);
                     AUD_LOG_V("[offload_fillbs_fromDram] COPY02. DRAM DATA: afe_offload_block.u4DataRemained = %d < bs_buf_needfill = %d \n",
                    afe_offload_block.u4DataRemained, bs_fill_size);
                }
                bs_buf.u4DataRemained += afe_offload_block.u4DataRemained;
                rMemDRAM->pRead += afe_offload_block.u4DataRemained;
                bs_buf.bneedFill = false;   /*copy done*/
            }
        }
        else {
            if (rMemDRAM->pWrite >= rMemDRAM->pRead) {
                flush_dram((unsigned int)rMemDRAM->pRead, bs_fill_size);
                ret = mp3_dma_transaction_wrap((uint32_t)(bs_buf.pBufAddr + bs_buf.u4DataRemained),
                                               (uint32_t)rMemDRAM->pRead, bs_fill_size, true);

                AUD_LOG_V("[offload_fillbs_fromDram] COPY03 BSADDR = %p, rMemDRAM->pRead = %p, rMemDRAM->pWrite = %p, size = %d\n",
                          (bs_buf.pBufAddr + bs_buf.u4DataRemained), rMemDRAM->pRead, rMemDRAM->pWrite,
                          bs_fill_size);

            }
            else {
                int data_length =  rMemDRAM->pBufEnd -  rMemDRAM->pRead;
                if (bs_fill_size <= data_length) {
                    flush_dram((unsigned int)rMemDRAM->pRead, bs_fill_size);
                    ret = mp3_dma_transaction_wrap((uint32_t)(bs_buf.pBufAddr + bs_buf.u4DataRemained),
                                                   (uint32_t)rMemDRAM->pRead,
                                                   bs_fill_size, true);

                    AUD_LOG_V("[offload_fillbs_fromDram] COPY04 data_length = %d needfill = %d\n", data_length, bs_fill_size);
                }
                else {
                    flush_dram((unsigned int)rMemDRAM->pRead, data_length);
                    ret = mp3_dma_transaction_wrap((uint32_t)(bs_buf.pBufAddr + bs_buf.u4DataRemained),
                                                   (uint32_t)rMemDRAM->pRead,
                                                   data_length, true);

                    AUD_LOG_V("[offload_fillbs_fromDram] COPY05 data_length = %d needfill = %d\n", data_length, bs_fill_size);
                    flush_dram((unsigned int)rMemDRAM->pBufBase, (bs_fill_size - data_length));
                    ret = mp3_dma_transaction_wrap((uint32_t)(bs_buf.pBufAddr + bs_buf.u4DataRemained + data_length),
                                                   (uint32_t)rMemDRAM->pBufBase,
                                                   (uint32_t)(bs_fill_size - data_length), true);

                    AUD_LOG_V("[offload_fillbs_fromDram] COPY06 data_length = %d needfill = %d\n", data_length, bs_fill_size);
                }
                AUD_LOG_V("[process_run] COPY02 BSADDR = %p, rMemDRAM->pRead = %p,rMemDRAM->pWrite = %p, size = %d\n",
                          (bs_buf.pBufAddr + bs_buf.u4DataRemained), rMemDRAM->pRead, rMemDRAM->pWrite,
                          bs_fill_size);
            }
            bs_buf.u4DataRemained += bs_fill_size;
            rMemDRAM->pRead += bs_fill_size;
        }
        if (rMemDRAM->pRead == rMemDRAM->pBufEnd) {
            rMemDRAM->pRead = rMemDRAM->pBufBase;
        }
        else if (rMemDRAM->pRead > rMemDRAM->pBufEnd) {
            rMemDRAM->pRead = rMemDRAM->pBufBase + (rMemDRAM->pRead - rMemDRAM->pBufEnd);
        }

        offload_calc_dram();
        bsAddData = need_add;
        AUD_LOG_D("[- offload_fillbs_fromDram] bs_fill_size = %d, bs_buf.u4DataRemained = %d, mtask_audio_ring_buf.datacount = %d,rMemDRAM->datacount = %d, rMemDRAM->pBufBase = %p, rMemDRAM->pBufEnd = %p, rMemDRAM->pRead = %p, rMemDRAM->pWrite = %p\n",
                    bs_fill_size, bs_buf.u4DataRemained, mtask_audio_ring_buf.datacount, rMemDRAM->datacount, rMemDRAM->pBufBase, rMemDRAM->pBufEnd, rMemDRAM->pRead, rMemDRAM->pWrite);
    }
#endif

    return ret;
}

static int mp3_decode_process(void)
{
#ifdef AudioDecoder
    AUD_LOG_V(" + Decoderprocess()\n");
    unsigned int pa_addr = (unsigned int)bs_buf.pBufAddr;
    ipi_msg_t ipi_msg;
    hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                               HAL_CACHE_OPERATION_FLUSH_INVALIDATE,
                               (void *)pa_addr,
                               bs_buf.u4BufferSize
                               );

#ifdef CYCLE
    volatile uint32_t mycount = 0;
    volatile uint32_t offset = 0;
    MP3_CPU_RESET_CYCLECOUNTER;
    __asm volatile("nop");
    mycount = *MP3_DWT_CYCCNT;
    offset = mycount - 1;
    MP3_CPU_RESET_CYCLECOUNTER;
    /*STEP 4 : Start to decode */
    bs_buf.consumedBS = MP3Dec_Decode(mMp3Dec->handle, mMp3Dec->pcm_buf,
                                      bs_buf.pBufAddr, bs_buf.u4BufferSize,
                                      bs_buf.pReadIdx);
    mycount = *MP3_DWT_CYCCNT - offset;
    AUD_LOG_D("\n\r MP3 Cycle %d BS %d\n", mycount, bs_buf.consumedBS);
#else
    AUD_LOG_V("+bs_buf.u4DataRemained = %d, bs_buf.consumedBS = %d, bs_buf.pBufAddr = 0x%x, bs_buf.u4BufferSize = %d, bs_buf.pReadIdx = 0x%x \n",
               bs_buf.u4DataRemained, bs_buf.consumedBS, bs_buf.pBufAddr, bs_buf.u4BufferSize, bs_buf.pReadIdx);

#ifdef MCPS_PROFILING
    start_time = read_ccount();
    bs_buf.consumedBS = MP3Dec_Decode(mMp3Dec->handle, mMp3Dec->pcm_buf,
                                      bs_buf.pBufAddr, bs_buf.u4BufferSize,
                                      bs_buf.pReadIdx);


    end_time = read_ccount();
    unsigned int diff = end_time - start_time;
    AUD_LOG_D("diff = %d\n",diff);
#else

    bs_buf.consumedBS = MP3Dec_Decode(mMp3Dec->handle, mMp3Dec->pcm_buf,
                                      bs_buf.pBufAddr, bs_buf.u4BufferSize,
                                      bs_buf.pReadIdx);
#endif

    if (bfirsdecode == true) {
        memset(&mp3_config, 0, sizeof(dvfs_swip_config_t));
        mp3_config.dl_out.sample_rate = decoder_sameplerateidx[mMp3Dec->handle->sampleRateIndex];
        mp3_config.dl_out.channels = mMp3Dec->handle->CHNumber;
        dvfs_add_swip(OFFLOAD_FEATURE_ID, SWIP_ID_CODEC_MP3_DECODER, &mp3_config);
        bfirsdecode = false;
    }
#endif
    if (bs_buf.u4DataRemained >= bs_buf.consumedBS) {
        bs_buf.u4DataRemained -= bs_buf.consumedBS;
    } else {
        AUD_LOG_D("Warning : bs_buf.consumedBS %d, bs_buf.u4DataRemained %d, drain state = %d\n",
                   bs_buf.consumedBS, bs_buf.u4DataRemained, afe_offload_block.bIsDrain);
        bs_buf.u4DataRemained = 0;
    }

    AUD_LOG_V("bs_buf.consumedBS %d\n", bs_buf.consumedBS);

    // pcm dump for debug

    if (pcmdump_enable == 1) {
        unsigned int copysize = offload_calc_datasize(mBSDumpsize, bs_buf.consumedBS);
        unsigned int remainsize = (mBSDumpsize + bs_buf.consumedBS) - copysize;

        memcpy(pBSDump_BUf + mBSDumpsize , bs_buf.pReadIdx , bs_buf.consumedBS);
        struct ipi_msg_t ipi_msg;
        char *data_buf = pBSDump_BUf;
        uint32_t data_size = copysize;
        AUD_LOG_V("[MP3 DUMP2!]\n");

        audio_send_ipi_msg(&ipi_msg, TASK_SCENE_PLAYBACK_MP3,
                           AUDIO_IPI_LAYER_TO_HAL, AUDIO_IPI_DMA,
                           AUDIO_IPI_MSG_BYPASS_ACK,
                           AUDIO_DSP_TASK_PCMDUMP_DATA,
                           data_size,
                           DEBUG_PCMDUMP_IN, //dump point serial number
                           data_buf);

        memcpy(pBSDump_BUf, pBSDump_BUf + copysize , remainsize);
        mBSDumpsize = remainsize;

    }

    if (bs_buf.consumedBS == 1) {
        AUD_LOG_E("bs_buf.consumedBS == 1,BS remained = %d, afe_offload_block.u4DataRemained = %d\n", bs_buf.u4DataRemained, afe_offload_block.u4DataRemained);
    }
    if (bs_buf.consumedBS < 0)
    {
        AUD_LOG_D("Warning : mp3_decode_process(), bs_buf.consumedBS %d, afe_offload_block.bIsDrain = %d, afe_offload_block.u4DataRemained = %d\n", bs_buf.consumedBS, afe_offload_block.bIsDrain, afe_offload_block.u4DataRemained);
        decode_unsupport_format = true;
        audio_send_ipi_msg(&ipi_msg, TASK_SCENE_PLAYBACK_MP3, AUDIO_IPI_LAYER_TO_KERNEL,
                            AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                            OFFLOAD_DECODE_ERROR, 0, 0, NULL);
    }
    bs_buf.pReadIdx += bs_buf.consumedBS;
#ifdef MET_MP3
    met_tag_call(task_monitor_tag_id, bs_buf.u4DataRemained, CTX_TASK,
                 MID_TAG_ONESHOT);
#endif
#endif
    AUD_LOG_V("mp3_decode_process decode, bs_buf.u4DataRemained = %d, bs_buf.consumedBS = %d, pcm_buf.u4DataRemained = %d \n",
               bs_buf.u4DataRemained, bs_buf.consumedBS, pcm_buf.u4DataRemained);

    return 0;
}

static int aac_decode_process(void)
{
#ifdef AudioDecoder
    AUD_LOG_V(" + Decoderprocess()\n");
    int ret = 0;
    ipi_msg_t ipi_msg;

#ifdef CYCLE
    volatile uint32_t mycount = 0;
    volatile uint32_t offset = 0;
    MP3_CPU_RESET_CYCLECOUNTER;
    __asm volatile("nop");
    mycount = *MP3_DWT_CYCCNT;
    offset = mycount - 1;
    MP3_CPU_RESET_CYCLECOUNTER;
    /*STEP 4 : Start to decode */

    ret = HEAACDec_DecodeFrame(mAacDec->handle, mAacDec->pcm_buf, mAacDec->working_buf2,
                                      bs_buf.pBufAddr, bs_buf.u4BufferSize,
                                      bs_buf.pReadIdx, &bs_buf.consumedBS);
    if (ret != 0)
    {
        AUD_LOG_D("ret = %d BS %d\n", ret, bs_buf.consumedBS);

    }
    mycount = *MP3_DWT_CYCCNT - offset;
    //AUD_LOG_D("\n\r MP3 Cycle %d BS %d\n", mycount, bs_buf.consumedBS);
#else
    int OutBufLen = 0;
    if (bfirsdecode == true) {

        unsigned char adif_header_array[ADIF_HEADER_TEMPLATE_LEN];
        AUD_LOG_D("offload_codec_info.codec_bitrate = %d, offload_codec_info.codec_samplerate = %d\n",
                   offload_codec_info.codec_bitrate, offload_codec_info.codec_samplerate);
        addAdifHeader(offload_codec_info.codec_bitrate ,offload_codec_info.codec_samplerate, adif_header_array, bs_buf.pBufAddr, bs_buf.u4BufferSize, out_buf.pBufAddr, &OutBufLen);

        int i;
        for(i=0; i<(ADIF_HEADER_TEMPLATE_LEN); i++)
        {
            AUD_LOG_D("adif_header_array[%d] = 0x%x\n", i, adif_header_array[i]);
        }

        bfirsdecode = false;
        AUD_LOG_V("MP3 after addAdifHeader\n");
        ret = HEAACDec_DecodeFrame(mAacDec->handle, mAacDec->pcm_buf, mAacDec->working_buf2,
                                   out_buf.pBufAddr, OutBufLen,
                                   out_buf.pReadIdx, &bs_buf.consumedBS);
        AUD_LOG_V("MP3 after HEAACDec_DecodeFrame, bs_buf.consumedBS = %d, OutBufLen=%d\n", bs_buf.consumedBS, OutBufLen);
        if (bs_buf.consumedBS >= ADIF_HEADER_TEMPLATE_LEN) {
            bs_buf.consumedBS -= ADIF_HEADER_TEMPLATE_LEN;
        }

        memset(&aac_config, 0, sizeof(dvfs_swip_config_t));
        aac_config.dl_out.sample_rate = mAacDec->handle->SampleRate;
        aac_config.dl_out.channels = mAacDec->handle->ChannelNum;
        sbr_vaule = (uint32_t)mAacDec->handle->sbrFlag;
        aac_config.p_config = &sbr_vaule;
        dvfs_add_swip(OFFLOAD_FEATURE_ID, SWIP_ID_CODEC_HEAAC_DECODER, &aac_config);

    } else {

        AUD_LOG_V("before HEAACDec_DecodeFrame :bs_buf.pBufAddr =%p bs_buf.pReadIdx = %p, bs_buf.u4BufferSize = %d,*bs_buf.pReadIdx = 0x%x, *bs_buf.pReadIdx+1 = 0x%x, mAacDec->pcm_buf = %p, mAacDec->working_buf2 = %p, mAacDec->handle = %p  \n",
                      bs_buf.pBufAddr, bs_buf.pReadIdx, bs_buf.u4BufferSize, *bs_buf.pReadIdx, *(bs_buf.pReadIdx+1), mAacDec->pcm_buf, mAacDec->working_buf2, mAacDec->handle );
#ifdef MCPS_PROFILING
        start_time = read_ccount();
        ret = HEAACDec_DecodeFrame(mAacDec->handle, mAacDec->pcm_buf, mAacDec->working_buf2,
                                          bs_buf.pBufAddr, bs_buf.u4BufferSize,
                                          bs_buf.pReadIdx, &bs_buf.consumedBS);

        end_time = read_ccount();
        unsigned int diff = end_time - start_time;
        AUD_LOG_D("diff = %d\n",diff);
#else

        ret = HEAACDec_DecodeFrame(mAacDec->handle, mAacDec->pcm_buf, mAacDec->working_buf2,
                                          bs_buf.pBufAddr, bs_buf.u4BufferSize,
                                          bs_buf.pReadIdx, &bs_buf.consumedBS);
#endif

    }
    decode_status = true;
    //ret = 17; //not support format
    if (ret != 0)
    {
        AUD_LOG_D("Warning : aac_decode_process(), ret = %d bs_buf.consumedBS %d, afe_offload_block.bIsDrain = %d\n", ret, bs_buf.consumedBS, afe_offload_block.bIsDrain);
        decode_status = false;
        decode_unsupport_format = true;
        audio_send_ipi_msg(&ipi_msg, TASK_SCENE_PLAYBACK_MP3, AUDIO_IPI_LAYER_TO_KERNEL,
                            AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                            OFFLOAD_DECODE_ERROR, 0, 0, NULL);
    }

#endif
    if (bs_buf.u4DataRemained >= bs_buf.consumedBS) {
        bs_buf.u4DataRemained -= bs_buf.consumedBS;
    } else {
        AUD_LOG_D("Warning : aac_decode_process(), bs_buf.consumedBS %d, bs_buf.u4DataRemained %d, drain state = %d\n",
                   bs_buf.consumedBS, bs_buf.u4DataRemained, afe_offload_block.bIsDrain);
        bs_buf.u4DataRemained = 0;
    }

    AUD_LOG_V("bs_buf.consumedBS %d, bs_buf.u4DataRemained %d\n", bs_buf.consumedBS, bs_buf.u4DataRemained);

    if (pcmdump_enable == 1) {
        unsigned int copysize = offload_calc_datasize(mBSDumpsize, bs_buf.consumedBS);
        unsigned int remainsize = (mBSDumpsize + bs_buf.consumedBS) - copysize;
        memcpy(pBSDump_BUf + mBSDumpsize , bs_buf.pReadIdx , bs_buf.consumedBS);
        struct ipi_msg_t ipi_msg;
        char *data_buf = pBSDump_BUf;
        uint32_t data_size = copysize;
        AUD_LOG_V("[MP3 DUMP2!]\n");

        audio_send_ipi_msg(&ipi_msg, TASK_SCENE_PLAYBACK_MP3,
                           AUDIO_IPI_LAYER_TO_HAL, AUDIO_IPI_DMA,
                           AUDIO_IPI_MSG_BYPASS_ACK,
                           AUDIO_DSP_TASK_PCMDUMP_DATA,
                           data_size,
                           DEBUG_PCMDUMP_IN, //dump point serial number
                           data_buf);

        memcpy(pBSDump_BUf, pBSDump_BUf + copysize , remainsize);
        mBSDumpsize = remainsize;

    }

    bs_buf.pReadIdx += bs_buf.consumedBS;
#ifdef MET_MP3
    met_tag_call(task_monitor_tag_id, bs_buf.u4DataRemained, CTX_TASK,
                 MID_TAG_ONESHOT);
#endif
#endif
    AUD_LOG_V(" - Decoderprocess()\n");
    return 0;
}

static int mp3_init_decoder(void)
{
#ifdef AudioDecoder
    if (!mMp3InitFlag) {
        AUD_LOG_D("%s(+)\n", __func__);
        mMp3Dec = (mp3DecEngine *)kal_pvPortMalloc(sizeof(mp3DecEngine));
        if (mMp3Dec == NULL) {
            AUD_LOG_E("%s() allocate engine fail", __func__);
            return false;
        }
        memset(mMp3Dec, 0, sizeof(mp3DecEngine));
        MP3Dec_GetMemSize(&mMp3Dec->min_bs_size, &mMp3Dec->pcm_size,
                          &mMp3Dec->workingbuf_size1, &mMp3Dec->workingbuf_size2);
        AUD_LOG_D("%s >>mp3DecEngine=%u min_bs_size=%u, pcm_size=%u, workingbuf_size1=%u,workingbuf_size2=%u\n",
                  __func__, sizeof(mp3DecEngine),
                  mMp3Dec->min_bs_size, mMp3Dec->pcm_size, mMp3Dec->workingbuf_size1,
                  mMp3Dec->workingbuf_size2);
        pcm_buffer_size = mMp3Dec->pcm_size;

        mMp3Dec->working_buf1 = kal_pvPortMalloc(mMp3Dec->workingbuf_size1);
        mMp3Dec->working_buf2 = kal_pvPortMalloc(mMp3Dec->workingbuf_size2);
        mMp3Dec->pcm_buf      = kal_pvPortMalloc(mMp3Dec->pcm_size);

        if ((NULL == mMp3Dec->working_buf1) || (NULL == mMp3Dec->working_buf2)) {
            AUD_LOG_E("%s() allocate working buf fail", __func__);
            return false;
        }

        memset(mMp3Dec->working_buf1, 0, mMp3Dec->workingbuf_size1);
        memset(mMp3Dec->working_buf2, 0, mMp3Dec->workingbuf_size2);
        memset(mMp3Dec->pcm_buf, 0, mMp3Dec->pcm_size);

        if (mMp3Dec->handle == NULL) {
            mMp3Dec->handle = MP3Dec_Init(mMp3Dec->working_buf1, mMp3Dec->working_buf2);
            if (mMp3Dec->handle == NULL) {

                AUD_LOG_E("%s() Init Decoder Fail", __func__);

                if (mMp3Dec->working_buf1) {
                    kal_vPortFree(mMp3Dec->working_buf1);
                    mMp3Dec->working_buf1 = NULL;
                }

                if (mMp3Dec->working_buf2) {
                    kal_vPortFree(mMp3Dec->working_buf2);
                    mMp3Dec->working_buf2 = NULL;
                }

                if (mMp3Dec->pcm_buf) {
                    kal_vPortFree(mMp3Dec->pcm_buf);
                    mMp3Dec->pcm_buf = NULL;
                }

                kal_vPortFree(mMp3Dec);
                mMp3Dec = NULL;
                return false;
            }
        }
        mMp3InitFlag = true;
    }
#endif
    return 0;

}

static void mp3_deinit_decoder(void)
{
    // PRINTF("+%s()", __func__);
#ifdef AudioDecoder
    if ((mMp3InitFlag == true) && (mMp3Dec != NULL)) {
        AUD_LOG_D("%s()+\n", __func__);
        dvfs_delete_swip(OFFLOAD_FEATURE_ID, SWIP_ID_CODEC_MP3_DECODER, &mp3_config);
        if (mMp3Dec->working_buf1) {
            kal_vPortFree(mMp3Dec->working_buf1);
            mMp3Dec->working_buf1 = NULL;
        }

        if (mMp3Dec->working_buf2) {
            kal_vPortFree(mMp3Dec->working_buf2);
            mMp3Dec->working_buf2 = NULL;
        }

        if (mMp3Dec->pcm_buf) {
            kal_vPortFree(mMp3Dec->pcm_buf);
            mMp3Dec->pcm_buf = NULL;
        }
        kal_vPortFree(mMp3Dec);
        mMp3Dec = NULL;
        mMp3InitFlag = false;
    }

    if (bs_buf.pBufAddr) {
        kal_vPortFree(bs_buf.pBufAddr);
        bs_buf.pBufAddr = NULL;
    }
    if (out_buf.pBufAddr) {
        kal_vPortFree(out_buf.pBufAddr);
        out_buf.pBufAddr = NULL;
    }

#endif
}




/*******************************************************
*                             Blisrc API
*
*********************************************************/
static int offload_init_blisrc(int32_t inSampleRate, int32_t inChannelCount,
                           int32_t outSampleRate, int32_t outChannelCount)
{
#ifdef BliSrc
    int32_t result = 0;
    if (!mSrcInitFlag) {
        if (OFFLOAD_SCENE_TYPE == TASK_SCENE_OFFLOAD_MP3) {
            if ((inSampleRate > 8) || (inSampleRate < 0) || inChannelCount < 0) {
                inSampleRate = outSampleRate;
                inChannelCount = outChannelCount;
                AUD_LOG_E("MP3: Blisrc failed ins = %d outs = %d inch = %d outch = %d\n",
                      inSampleRate, outSampleRate, inChannelCount, outChannelCount);
            }
            else {
                inSampleRate = decoder_sameplerateidx[inSampleRate];
                AUD_LOG_D("MP3: Blisrc ins = %d outs = %d inch = %d outch = %d\n",
                          inSampleRate, outSampleRate, inChannelCount, outChannelCount);
            }
        } else if(OFFLOAD_SCENE_TYPE == TASK_SCENE_OFFLOAD_AAC) {
            if ((inSampleRate < 0) || inChannelCount < 0) {
                AUD_LOG_E("AAC: Blisrc failed ins = %d outs = %d inch = %d outch = %d\n",
                      inSampleRate, outSampleRate, inChannelCount, outChannelCount);
                inSampleRate = outSampleRate;
                inChannelCount = outChannelCount;
                }
            else {
                AUD_LOG_D("AAC: Blisrc ins = %d outs = %d inch = %d outch = %d\n",
                          inSampleRate, outSampleRate, inChannelCount, outChannelCount);
                }
        }

        mBliSrc = (blisrcEngine *)kal_pvPortMalloc(sizeof(blisrcEngine));
        if (mBliSrc == NULL) {
            AUD_LOG_E("%s() allocate engine fail", __func__);
            AUD_ASSERT(mBliSrc != NULL);
            return false;
        }
        memset(mBliSrc, 0, sizeof(blisrcEngine));
        // set params
        mBliSrc->mBliParam.in_channel = inChannelCount;
        mBliSrc->mBliParam.in_sampling_rate = inSampleRate;
        mBliSrc->mBliParam.ou_channel = outChannelCount;
        mBliSrc->mBliParam.ou_sampling_rate = outSampleRate;
        mBliSrc->mBliParam.PCM_Format = BLISRC_IN_Q1P15_OUT_Q1P15;
        result = Blisrc_GetBufferSize(&mBliSrc->mInternalBufSize,
                                      &mBliSrc->mTempBufSize, &mBliSrc->mBliParam);
        if (result < 0) {
            AUD_LOG_E("Blisrc_GetBufferSize error %d", result);
        }
        mBliSrc->mInternalBuf = kal_pvPortMalloc(mBliSrc->mInternalBufSize);
        mBliSrc->mTempBuf = kal_pvPortMalloc(mBliSrc->mTempBufSize);

        AUD_LOG_D("sizeof(blisrcEngine) %d, Internal Buf = %d, mBliSrc->mTempBufSize = %d\n", sizeof(blisrcEngine),
                  mBliSrc->mInternalBufSize, mBliSrc->mTempBufSize);
        if (NULL == mBliSrc->mInternalBuf) {
            AUD_LOG_E("%s() allocate working buf fail", __func__);
            AUD_ASSERT(mBliSrc->mInternalBuf != NULL);
            return false;
        }
        memset(&src_config, 0, sizeof(dvfs_swip_config_t));
        src_config.dl_in.format = AUDIO_FORMAT_PCM_16_BIT;
        src_config.dl_out.format = AUDIO_FORMAT_PCM_16_BIT;
        src_config.dl_in.channels = inChannelCount;
        src_config.dl_out.channels = outChannelCount;
        src_config.dl_in.sample_rate = inSampleRate;
        src_config.dl_out.sample_rate = outSampleRate;
        dvfs_add_swip(FORMATTER_FEATURE_ID, SWIP_ID_PROCESSING_SRC, &src_config);

        if (Blisrc_Open(&mBliSrc->mHandle, (void *)mBliSrc->mInternalBuf,
                        &mBliSrc->mBliParam) != 0) {
            AUD_LOG_E("Blisrc_Open error \n");
            if (mBliSrc->mInternalBuf) {
                kal_vPortFree(mBliSrc->mInternalBuf);
                mBliSrc->mInternalBuf = NULL;
            }
            kal_vPortFree(mBliSrc);
            mBliSrc = NULL;
            result = false;
        }
        else {
            Blisrc_Reset(mBliSrc->mHandle);
            result = true;
            mSrcInitFlag = true;
            if (outSampleRate >= inSampleRate) {
                mCountForSrc = outSampleRate / inSampleRate;
                if (outSampleRate % inSampleRate) {
                    mCountForSrc += 1 ;
                }
            }
            AUD_LOG_D("Blisrc_Open Success ins = %d outs = %d inch = %d outch = %d mCountForSrc = %d\n",
                      inSampleRate, outSampleRate, inChannelCount, outChannelCount, mCountForSrc);
        }
    }
    return result;
#else
    return 0;
#endif
}

// wait for complete
static int aac_init_decoder(void)
{
#ifdef AudioDecoder
    if (!mAacInitFlag) {
        // PRINTF("+%s()", __func__);
        mAacDec = (aacDecEngine *)kal_pvPortMalloc(sizeof(aacDecEngine));
        if (mAacDec == NULL) {
            AUD_LOG_E("%s() allocate engine fail", __func__);
            return false;
        }
        memset(mAacDec, 0, sizeof(aacDecEngine));
        mAacDec->sbrSignaling = SBR_AUTO;
        mAacDec->sbrMode = SBR_MODE_AUTO;
        HEAACDec_GetMemSize(&mAacDec->workingbuf_size1, &mAacDec->workingbuf_size2,
                          &mAacDec->pcm_size, &mAacDec->min_bs_size);

        AUD_LOG_D("%s >>aacDecEngine=%u min_bs_size=%u, pcm_size=%u, workingbuf_size1=%u,workingbuf_size2=%u\n",
                  __func__, sizeof(aacDecEngine),
                  mAacDec->min_bs_size, mAacDec->pcm_size, mAacDec->workingbuf_size1, mAacDec->workingbuf_size2);
        pcm_buffer_size = mAacDec->pcm_size;
        mAacDec->working_buf1 = kal_pvPortMalloc(mAacDec->workingbuf_size1);
        mAacDec->working_buf2 = kal_pvPortMalloc(mAacDec->workingbuf_size2);
        mAacDec->pcm_buf      = kal_pvPortMalloc(mAacDec->pcm_size);

        if ((NULL == mAacDec->working_buf1) || (NULL == mAacDec->working_buf2)) {
            AUD_LOG_E("%s() allocate working buf fail", __func__);
            return false;
        }

        memset(mAacDec->working_buf1, 0, mAacDec->workingbuf_size1);
        memset(mAacDec->working_buf2, 0, mAacDec->workingbuf_size2);
        memset(mAacDec->pcm_buf, 0, mAacDec->pcm_size);

        if (mAacDec->handle == NULL) {
            mAacDec->handle = HEAACDec_Init(mAacDec->working_buf1, mAacDec->sbrSignaling, mAacDec->sbrMode, mAacDec->fForce2Stereo);
            if (mAacDec->handle == NULL) {
                AUD_LOG_E("%s() Init Decoder Fail", __func__);

                if (mAacDec->working_buf1) {
                    kal_vPortFree(mAacDec->working_buf1);
                    mAacDec->working_buf1 = NULL;
                }

                if (mAacDec->working_buf2) {
                    kal_vPortFree(mAacDec->working_buf2);
                    mAacDec->working_buf2 = NULL;
                }

                if (mAacDec->pcm_buf) {
                    kal_vPortFree(mAacDec->pcm_buf);
                    mAacDec->pcm_buf = NULL;
                }

                kal_vPortFree(mAacDec);
                mAacDec = NULL;
                return false;
            }
            AUD_LOG_D("%s() Init Decoder Success", __func__);
        }
        mAacInitFlag = true;
    }
#endif
    return 0;
}

static void aac_deinit_decoder(void)
{
#ifdef AudioDecoder
    if ((mAacInitFlag == true) && (mAacDec != NULL)) {
        AUD_LOG_D("%s()+\n", __func__);
        dvfs_delete_swip(OFFLOAD_FEATURE_ID, SWIP_ID_CODEC_HEAAC_DECODER, &aac_config);
        if (mAacDec->working_buf1) {
            kal_vPortFree(mAacDec->working_buf1);
            mAacDec->working_buf1 = NULL;
        }

        if (mAacDec->working_buf2) {
            kal_vPortFree(mAacDec->working_buf2);
            mAacDec->working_buf2 = NULL;
        }

        if (mAacDec->pcm_buf) {
            kal_vPortFree(mAacDec->pcm_buf);
            mAacDec->pcm_buf = NULL;
        }
        kal_vPortFree(mAacDec);
        mAacDec = NULL;
        mAacInitFlag = false;
    }

    if (bs_buf.pBufAddr) {
        kal_vPortFree(bs_buf.pBufAddr);
        bs_buf.pBufAddr = NULL;
    }

    if (out_buf.pBufAddr) {
        kal_vPortFree(out_buf.pBufAddr);
        out_buf.pBufAddr = NULL;
    }
#endif
    AUD_LOG_V("-%s()", __func__);
}

static int offload_blisrc_process(void *pInBuffer, unsigned int inBytes,
                              void **ppOutBuffer, unsigned int *pOutBytes)
{
#ifdef BliSrc
    if ((mBliSrc->mBliParam.in_channel ==  mBliSrc->mBliParam.ou_channel) &&
         (mBliSrc->mBliParam.in_sampling_rate == mBliSrc->mBliParam.ou_sampling_rate)) {
        *ppOutBuffer = pInBuffer;
        *pOutBytes = inBytes;

    } else {
        unsigned int outputIndex = 0;
        unsigned int outBuffSize = offload_get_pcm_buffersize();
        int totalPutSize = 0;
        int putSize = inBytes ;
        unsigned int ouputlen = 0;
        unsigned int inLen = 0;
        unsigned int remained = 0;

        if (pSrcOutBuf == NULL) {
            pSrcOutBuf = kal_pvPortMalloc(outBuffSize);
            AUD_LOG_D("- BlisrcProcess() pSrcOutBuf addr = %p\n", pSrcOutBuf);
            AUD_ASSERT(pSrcOutBuf != NULL);
        }

        while (totalPutSize < putSize) {

            remained = (putSize - totalPutSize);
            inLen = remained;
            int ret = Blisrc_Process(mBliSrc->mHandle, mBliSrc->mTempBuf, (void *)pInBuffer + totalPutSize,
                                     &remained,
                                     (void *)pSrcOutBuf + ouputlen, &outBuffSize);
            if (ret != 0)
            {
                AUD_LOG_D("BlisrcProcess() ret = %d, remained = %d, outBuffSize = %d\n", ret, remained, outBuffSize);
            }
            AUD_ASSERT(pSrcOutBuf != NULL);
            totalPutSize += (inLen - remained);
            outputIndex += outBuffSize;
            ouputlen = outputIndex;

        }

        *ppOutBuffer = pSrcOutBuf;
        *pOutBytes = outputIndex;
    }
#else
    *ppOutBuffer = pInBuffer;
    *pOutBytes = inBytes;
#endif
    return 0;
}


static int offload_deinit_blisrc()
{
#ifdef BliSrc
    AUD_LOG_V("+%s()", __func__);
    if ((mSrcInitFlag == true) && (mBliSrc != NULL)) {
        if (mBliSrc->mInternalBuf) {
            kal_vPortFree(mBliSrc->mInternalBuf);
            mBliSrc->mInternalBuf = NULL;
        }
        if (mBliSrc->mTempBuf) {
            kal_vPortFree(mBliSrc->mTempBuf);
            mBliSrc->mTempBuf = NULL;
        }
        if (pSrcOutBuf != NULL) {
            kal_vPortFree(pSrcOutBuf);
            pSrcOutBuf = NULL;
        }
        if (mBliSrc != NULL) {
            kal_vPortFree(mBliSrc);
            mBliSrc = NULL;
        }
        mSrcInitFlag = false;
        dvfs_delete_swip(FORMATTER_FEATURE_ID, SWIP_ID_PROCESSING_SRC, &src_config);

    }

    AUD_LOG_V("-%s()", __func__);
#endif
    return 0;
}

/*******************************************************
*                             bit convert API
*
*********************************************************/
#ifdef POST_PROCESS
static int offload_volume_process(void *pInBuffer, unsigned int inBytes,
                              uint32_t *vc, uint32_t *vp, void **ppOutBuffer, unsigned int *pOutBytes)
{
    // change to AUDIO_FORMAT_PCM_8_24_BIT
    int16_t *ptr16 ;
    ptr16 = (int16_t *)pInBuffer;
    int16_t data16 = 0;
    int data32 = 0;
    uint32_t volume_cur = *vc;
    uint32_t volume_prev = *vp;
    int32_t diff = (volume_cur - volume_prev);
    uint32_t count = inBytes >> 1;

    if (volume_prev == 0xF || diff == 0) {    // initial value no need to ramp
        while (count) {
            data16 = *ptr16;
            if (0x1000000 == volume_cur) {
                break;
            }
            data32 = data16 * (uint16_t)(volume_cur >> 8);
            *ptr16 = (int16_t)(data32 >> 16);
            ptr16++;
            count--;
        }
    }
    else if (diff != 0) {                  // need ramp
        if (volume_cur > volume_prev) {    // do ramp up
            int Volume_inverse = (diff / count);
            int ConsumeSample = 0;
            while ((count - 1)) {
                data16 = *ptr16;
                data32 = data16 * (uint16_t)((volume_prev + (Volume_inverse * ConsumeSample)) >>
                                            8);
                *ptr16 = (int16_t)(data32 >> 16);
                ptr16++;
                count--;
                ConsumeSample++;
            }
            data16 = *ptr16;
            data32 = data16 * (uint16_t)(volume_cur >> 8);
            *ptr16 = (int16_t)(data32 >> 16);
            AUD_LOG_V("[ramp up] Volume_inverse = %d volume_cur = %d volume_prev = %d\n",
                      Volume_inverse
                      , volume_cur, volume_prev);
        }
        else {                             //do ramp down
            diff = (volume_prev - volume_cur);
            int Volume_inverse = (diff / count);
            int ConsumeSample = 0;
            while ((count - 1)) {
                data16 = *ptr16;
                data32 = data16 * (uint16_t)((volume_prev - (Volume_inverse * ConsumeSample)) >>
                                            8);
                *ptr16 = (int16_t)(data32 >> 16);
                ptr16++;
                count--;
                ConsumeSample++;
            }
            data16 = *ptr16;
            data32 = data16 * (uint16_t)(volume_cur >> 8);
            *ptr16 = (int16_t)(data32 >> 16);
            AUD_LOG_V("[ramp down] Volume_inverse = %d volume_cur = %d volume_prev = %d\n",
                      Volume_inverse
                      , volume_cur, volume_prev);
        }
    }
    *ppOutBuffer = pInBuffer;
    *pOutBytes = inBytes;
    *vp = *vc;                           // only process one frame
    return 0;
}
#endif

#ifndef MTK_AURISYS_FRAMEWORK_SUPPORT
static int offload_bitconvert_process(void *pInBuffer, unsigned int inBytes,
                                  void **ppOutBuffer, unsigned int *pOutBytes)
{
    int16_t *ptr16;
    int *ptr32 = pBitConvertBuf;
    int data32 = 0;
    int ptr32_cnt;
    ptr16 = (int16_t *)pInBuffer;
    for (ptr32_cnt = 0; ptr32_cnt < (inBytes >> 1); ptr32_cnt++) {
        data32 = *ptr16;
        data32 = (data32 & 0xFFFF) << 8;
        *ptr32 = data32;
        ptr16++;
        ptr32++;
    }
    *ppOutBuffer = pBitConvertBuf;
    *pOutBytes = inBytes * 2;
    return 0;
}
#endif


//DMA don not need data pending function since HW  dont need 64byte align

static void offload_get_timestamp(struct AudioTask *this)
{
    ipi_msg_t ipi_msg;
    uint32_t pa_addr = (unsigned int)pcm_buf.pBufAddr;

    hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                               HAL_CACHE_OPERATION_FLUSH_INVALIDATE,
                               (void *)pa_addr,
                               pcm_buf.u4BufferSize
                              );

    if (afe_offload_block.bgetTime == true) {
        AUD_LOG_V("offload_get_timestamp, pcm_buf.u4DataRemained = %d\n", pcm_buf.u4DataRemained);
        audio_send_ipi_msg(&ipi_msg, this->scene, AUDIO_IPI_LAYER_TO_KERNEL,
                           AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                           OFFLOAD_PCMCONSUMED,
                           (unsigned int)pcm_buf.u4DataRemained, 0, NULL);
        afe_offload_block.bgetTime = false;
    }
}


