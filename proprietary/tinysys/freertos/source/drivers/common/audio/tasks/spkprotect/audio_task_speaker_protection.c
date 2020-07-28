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

#include "audio_task_speaker_protection.h"
#include "audio_task_speaker_protection_params.h"

#include <stdarg.h>
#include <audio_ringbuf.h>
#include <mt_gpt.h>

#include <interrupt.h>
#include <dvfs.h>
#include <dma.h>
#include <feature_manager.h>
#include <scp_ipi.h>
#include <wrapped_audio.h>
#include <wrapped_errors.h>
#include <wakelock.h>
#include "audio_messenger_ipi.h"
#include "audio_task_factory.h"
#include "arsi_api.h"
#include "audio_task_utility.h"

// for arsi api
#include <arsi_library_entry_points.h>
#include <arsi_call_type.h>

#include "audio_hw.h"
#include "audio_hw_reg.h"
#include "audio_ringbuf.h"

#include <cache_internal.h>
#ifdef MTK_SMARTPA_CACHE_SUPPORT
#include <cache_dram_management.h>
#endif
#include "audio_messenger_ipi.h"
#include <audio_memory_control.h>

#define MAX_MSG_QUEUE_SIZE (32)
#define LOCAL_TASK_STACK_SIZE (768)
#define LOCAL_TASK_NAME "speaker_protection"
#define LOCAL_TASK_PRIORITY (2)
#define SPK_TASK_LOG_NAME "[spk task]"
#ifdef GET_TIME_INTERVAL
#undef GET_TIME_INTERVAL
#endif
#define GET_TIME_INTERVAL(stop_time_ns, start_time_ns) \
    (((stop_time_ns) > (start_time_ns)) \
     ? (((stop_time_ns) - (start_time_ns))*77) \
     : (((0xFFFFFFFFFFFFFFFF - (start_time_ns)) + (stop_time_ns) + 1))*77)

#define SMARTPA_WAKELOCK
#ifdef SMARTPA_WAKELOCK
static wakelock_t spk_task_monitor_wakelock;
#endif

static uSpeaker_Protection_Info_t uSpeaker_Protection_Info;
Memory_Info_t *Memory_block;

/*define this to runtime print IV data*/
//#define Dump_IV_data
#ifdef Dump_IV_data
static int dumpdatacount = 0;
#endif

#define PROCESSINGFRAMEBYTES 0x2000
#define MAX_PARAM_SIZE 4096

SRAM_REGION_VARIABLE static uint8_t *g_msg_array;
SRAM_REGION_VARIABLE static uint32_t ipi_msg_size;
#define IPI_MSG_DMA_INFO_SIZE (sizeof(struct ipi_msg_dma_info_t))
#define LIGHT_IPI_MSG_PAYLOAD_SIZE (32)
#define GET_IPI_MSG(queue_idx) \
    ({ \
        ipi_msg_t *__ret = (ipi_msg_t *)(g_msg_array + \
                                         queue_idx * ipi_msg_size); \
        if (!g_msg_array) { \
            AUD_LOG_E("GET_IPI_MSG error, g_msg_array null"); \
        } \
        __ret; \
    })

#define PROCESS_TIME
#ifdef PROCESS_TIME
// Task
static uint64_t task_max_process_time_ns[5];
static uint64_t task_start_time_ns[5];
static uint64_t task_stop_time_ns[5];
static uint64_t task_time_diff_ns[5];
static uint64_t task_total_process_time[5];
static uint64_t task_avg_process_time[5];
static int task_avg_count[5];
static int task_time_idx;
// Lib
static uint64_t lib_max_process_time_ns;
static uint64_t lib_start_time_ns;
static uint64_t lib_stop_time_ns;
static uint64_t lib_time_diff_ns;
static uint64_t lib_total_process_time;
static uint64_t lib_avg_process_time;
static int lib_avg_count;
#define AVG_NUMBER 100
extern freq_enum get_cur_clk(void);

enum SPK_PROCESS_TIME {
    SCP_PROCESS_LIB = 0,
    SCP_PROCESS_TASK,
};

static void ProcessTime(uint8_t event, uint16_t msg_id,
                        uint8_t num_queue_element, bool is_start_stage);
#endif /* end of #ifdef PROCESS_TIME */

typedef struct tsask_debug_struct {
    uint16_t msg_id;
    uint64_t diff_ns;
} tsask_debug_struct_t;
static tsask_debug_struct_t task_debug_time[MAX_MSG_QUEUE_SIZE];

static arsi_task_config_t arsi_task_config;
static arsi_lib_config_t  arsi_lib_config;
static data_buf_t param_buf;
static data_buf_t working_buf;
static void InitArsiTaskConfig(void);
static void InitArsiLibConfig(void);
static void myprint(const char *message, ...);
static void UpdateArsiTaskconfig(void);
static void *tuning_key_value;

#ifdef MTK_SMARTPA_CACHE_SUPPORT
#define WORKING_BUFFER_SIZE (0xd000)
DRAM_REGION_BSS char lib_working_buf[WORKING_BUFFER_SIZE] __attribute__((aligned(MTK_CACHE_LINE_SIZE)));
DRAM_REGION_BSS char task_working_buf_d1_in[PROCESSINGFRAMEBYTES] __attribute__((aligned(MTK_CACHE_LINE_SIZE)));
DRAM_REGION_BSS char task_working_buf_d1_out[PROCESSINGFRAMEBYTES] __attribute__((aligned(MTK_CACHE_LINE_SIZE)));
DRAM_REGION_BSS char task_working_buf_d1_iv[PROCESSINGFRAMEBYTES] __attribute__((aligned(MTK_CACHE_LINE_SIZE)));
DRAM_REGION_BSS AurisysLibInterface the_lib;
DRAM_REGION_BSS char dram_param_buffer[MAX_PARAM_SIZE] __attribute__((aligned(MTK_CACHE_LINE_SIZE)));
DRAM_REGION_BSS char tuning_buffer[MAX_PARAM_SIZE] __attribute__((aligned(MTK_CACHE_LINE_SIZE)));
#else
char task_working_buf_d1_in[PROCESSINGFRAMEBYTES] __attribute__((aligned(MTK_CACHE_LINE_SIZE)));
char task_working_buf_d1_out[PROCESSINGFRAMEBYTES] __attribute__((aligned(MTK_CACHE_LINE_SIZE)));
char task_working_buf_d1_iv[PROCESSINGFRAMEBYTES] __attribute__((aligned(MTK_CACHE_LINE_SIZE)));
AurisysLibInterface the_lib;
char dram_param_buffer[MAX_PARAM_SIZE] __attribute__((aligned(MTK_CACHE_LINE_SIZE)));
char tuning_buffer[MAX_PARAM_SIZE] __attribute__((aligned(MTK_CACHE_LINE_SIZE)));
#endif /* end of #ifdef MTK_SMARTPA_CACHE_SUPPORT */

#if defined(CFG_MTK_SMARTPA_IV_BUF_IN_TCM)
#define IV_TCM_BUF_SIZE (16 * 1024)
static char iv_buffer[IV_TCM_BUF_SIZE] __attribute__((aligned(MTK_CACHE_LINE_SIZE)));
#endif /* end of #ifdef CFG_MTK_SMARTPA_IV_BUF_IN_TCM */

static audio_buf_t dl_buf_in;
static audio_buf_t dl_buf_out;
static audio_buf_t dl_buf_iv;

static int smart_pa_dump_pcm_flag;
static int smartpa_lib_log_flag;
static int bypass_lib_flag;
static const char *bypass_lib_string = "BYPASS_LIB";

/* aurisys relate */
static void *smartpa_arsi_handler;

// #define ProfileSpeechLog
#define DUMP_LOG_COUNT 50

/* dump relate*/
static struct RingBuf dram_dump;
static uint8_t dump_pcm_flag;
static int dataRemained;

#ifdef AUD_LOG_D
#undef  AUD_LOG_D
#define AUD_LOG_D PRINTF_E
#endif

typedef enum {
    DUMP_PCM_PRE = 0,
    DUMP_IV_DATA = 1,
    DUMP_DEBUG_DATA = 2,
    NUM_DUMP_DATA,
} dump_data_t;

inline int min(int a, int b) {
    if (a > b)
        return b;
    return a;
}
inline int max(int a, int b) {
    if (a < b)
        return b;
    return a;
}

static void spk_flush_cache(uint32_t address, uint32_t length)
{
#ifdef MTK_SMARTPA_CACHE_SUPPORT
    int ret;
    ret = hal_cache_flush_multiple_cache_lines(CACHE_DCACHE, address, length);
    if (ret != HAL_CACHE_STATUS_OK) {
        AUD_LOG_E("%s cache flush error", SPK_TASK_LOG_NAME);
    }
#endif
}

static void spk_invalidate_cache(uint32_t address, uint32_t length)
{
#ifdef MTK_SMARTPA_CACHE_SUPPORT
    int ret;
    ret = hal_cache_invalidate_multiple_cache_lines(CACHE_DCACHE, address, length);
    if (ret != HAL_CACHE_STATUS_OK) {
        AUD_LOG_E("%s cache invalidate error", SPK_TASK_LOG_NAME);
    }
#endif
}

static unsigned int getRingBufCount(struct RingBuf *ring_buf,
                                    unsigned int idx,
                                    bool isReadPtrUpdate)
{
    unsigned int current_idx;
    if (isReadPtrUpdate) {
        current_idx = (unsigned int)ring_buf->pRead;
    } else {
        current_idx = (unsigned int)ring_buf->pWrite;
    }

    unsigned int hw_cur_index =
            (unsigned int)ring_buf->pBufBase + idx;
    unsigned int count;
    if (hw_cur_index >= current_idx) {
        count = hw_cur_index - current_idx;
    } else {
        count = hw_cur_index - current_idx + ring_buf->bufLen;
    }

    return count;
}

static int bypass_lib_flag_set(char *tuning_key_value)
{
    int ret = 0;
    char bypass_on_str[32];
    char bypass_off_str[32];
    sprintf(bypass_on_str, "%s=on", bypass_lib_string);
    sprintf(bypass_off_str, "%s=off", bypass_lib_string);
    if (strncmp(tuning_key_value, bypass_on_str, strlen(bypass_on_str)) == 0) {
        AUD_LOG_D("BYPASS_LIB on");
        bypass_lib_flag = 1;
        ret = 1;
    } else if (strncmp(tuning_key_value, bypass_off_str, strlen(bypass_off_str)) == 0) {
        AUD_LOG_D("BYPASS_LIB off");
        bypass_lib_flag = 0;
        ret = 1;
    }

    return ret;
}

static int bypass_lib_flag_get(string_buf_t *key_value_pair)
{
    int ret = 0;

    if (strncmp(key_value_pair->p_string, bypass_lib_string, strlen(bypass_lib_string)) == 0) {
        char bypass_lib_str[8];
        sprintf(bypass_lib_str, "%s", bypass_lib_flag == 1 ? "=on" : "=off");
        strncat(key_value_pair->p_string, bypass_lib_str, strlen(bypass_lib_str));
        key_value_pair->string_size = strlen(bypass_lib_str);
        ret = 1;
    }

    return ret;
}

void bus_qos(uint32_t set) __attribute__((weak));

static void spk_dump_pcm(dump_data_t flag, uint32_t dump_buf_src,
                         uint32_t dump_buf_size)
{
    ipi_msg_t ipi_msg;
    uint32_t ipi_payload_buf[3];

    /* only dump on can do dump*/
    if (dump_pcm_flag != 0) {
        ipi_payload_buf[0] = flag;
        ipi_payload_buf[1] = dump_buf_size;
        ipi_payload_buf[2] = dram_dump.pWrite - dram_dump.pBufBase;

        spk_flush_cache(dump_buf_src, dump_buf_size);
        aud_dma_transaction_wrap((uint32_t)dram_dump.pWrite, dump_buf_src,
                                 dump_buf_size, true, SMART_PA_MEM_ID);

        if (dram_dump.pWrite + dump_buf_size <= dram_dump.pBufEnd) {
            dram_dump.pWrite += dump_buf_size;
        } else {
            dram_dump.pWrite = dram_dump.pBufBase + dump_buf_size;
        }

        audio_send_ipi_msg(&ipi_msg, TASK_SCENE_SPEAKER_PROTECTION,
                           AUDIO_IPI_LAYER_TO_KERNEL, AUDIO_IPI_PAYLOAD,
                           AUDIO_IPI_MSG_NEED_ACK,
                           SPK_PROTECT_PCMDUMP_OK, sizeof(uint32_t) * 3, 0,
                           (char *)ipi_payload_buf);
        AUD_LOG_V("flag = %d, buf size = %d, buf offset = %d",
                  ipi_payload_buf[0], ipi_payload_buf[1], ipi_payload_buf[2]);
    }
}

static void task_speaker_protection_start(AudioTask *this)
{
    if (AUDIO_TASK_INIT != this->state) {
        AUD_LOG_E("%s Unexpected status=0x%x", SPK_TASK_LOG_NAME, this->state);
        return;
    }

    AUD_LOG_D("%s start", SPK_TASK_LOG_NAME);
}

static void task_speaker_protection_stop(AudioTask *this)
{
    if (AUDIO_TASK_IDLE == this->state) {
        AUD_LOG_E("%s Unexpected status=0x%x", SPK_TASK_LOG_NAME, this->state);
        return;
    }

    if (smartpa_arsi_handler != NULL && !bypass_lib_flag) {
        if (the_lib.arsi_reset_handler(
                &arsi_task_config,
                &arsi_lib_config,
                &param_buf,
                smartpa_arsi_handler) != NO_ERROR) {
            AUD_LOG_D("arsi_reset_handler fail!!");
        }
    }
}

static void dumpRingBuf(char *appendstring, struct RingBuf buf)
{
    AUD_LOG_D("%s(), pBufBase = %p pBufEnd = %p  pread = %p p write = %p DataCount = %u freespace = %u",
              appendstring, buf.pBufBase, buf.pBufEnd, buf.pRead, buf.pWrite,
              RingBuf_getDataCount(&buf), RingBuf_getFreeSpace(&buf));
}

static void DumpSpeakerProteMsg(char *appendstring,
                                uSpeaker_Protection_Task_msg_t Msg)
{
    uint64_t time_tick = timer_get_global_timer_tick();
    AUD_LOG_D("%s(), Msg =0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, time %llu",
              appendstring, Msg.msg_id, Msg.param1,  Msg.param2, Msg.payload0,
              Msg.payload1, Msg.payload2, Msg.payload3, Msg.payload4, time_tick);
}

static void ResetMemIndex(Memory_Info_t *Memory_block)
{
    Memory_block->memory_buf.pRead =  Memory_block->memory_buf.pBufBase;
    Memory_block->memory_buf.pWrite = Memory_block->memory_buf.pBufBase;
    Memory_block->memory_buf.datacount = 0;
}

static unsigned int memalign(unsigned int bytesnum) {
    // 0xFFFFFFE0: 32 bytes alignment
    return (bytesnum & 0xFFFFFFE0);
}

#ifdef Dump_IV_data
static void DumpbufferData(char *IVbuffer)
{
    if (dumpdatacount % DUMP_LOG_COUNT == 0) {
        unsigned int *outbuf = (unsigned int *)IVbuffer;
        AUD_LOG_D("[IV Data] writebuf = %x %x %x %x %x %x %x %x",
                  *(outbuf), *(outbuf + 1), *(outbuf + 2), *(outbuf + 3), *(outbuf + 4),
                  *(outbuf + 5), *(outbuf + 6), *(outbuf + 7));
        AUD_LOG_D("[IV Data] writebuf = %x  %x %x %x %x %x %x %x",
                  *(outbuf + 8), *(outbuf + 9), *(outbuf + 10), *(outbuf + 11), *(outbuf + 12),
                  *(outbuf + 13), *(outbuf + 14), *(outbuf + 15));
    }
}
#endif

static void dlDataProcessing(unsigned int frameBytes,
                             int num_queue_element,
                             bool isStateWorking,
                             int msg_id)
{
    struct RingBuf *preProcessRingBuf;
    uint32_t memory_type;
    switch (msg_id) {
    case SPK_PROTECT_DLCOPY:
        preProcessRingBuf = &uSpeaker_Protection_Info.rMemPlatformDL.memory_buf;
        memory_type = uSpeaker_Protection_Info.rMemPlatformDL.memory_type;
        break;
    case SPK_PROTECT_SPEECH_IRQDL:
        preProcessRingBuf = &uSpeaker_Protection_Info.rMemMDDL.memory_buf;
        memory_type = uSpeaker_Protection_Info.rMemMDDL.memory_type;
        break;
    default:
        AUD_LOG_E("%s dl process: msg id error, return", SPK_TASK_LOG_NAME);
        return;
    }

    unsigned int DLBufCnt, IvBufCnt, spkBufFreeCnt, processBytes;

    do {
        DLBufCnt = RingBuf_getDataCount(preProcessRingBuf);
        IvBufCnt = RingBuf_getDataCount(&uSpeaker_Protection_Info.rMemIVSpk.memory_buf);
        spkBufFreeCnt = RingBuf_getFreeSpace(&uSpeaker_Protection_Info.rMemSpkDL.memory_buf);
        processBytes = frame_to_bytes(uSpeaker_Protection_Info.mdperiodsize,
                                      uSpeaker_Protection_Info.mdlchannel,
                                      uSpeaker_Protection_Info.mdlformat);

        AUD_LOG_V("+%s framebytes: %u DLBufCnt: %u IvBufCnt: %u, processBytes: %u",
                  msg_id == SPK_PROTECT_DLCOPY ? "SPK_DLCOPY" : "SPK_SPEECH_IRQDL",
                  frameBytes, DLBufCnt, IvBufCnt, processBytes);

        if (isStateWorking) {
            if (DLBufCnt < processBytes ||
                IvBufCnt < processBytes ||
                spkBufFreeCnt < processBytes) {
                return;
            }
        } else {
            if (DLBufCnt < processBytes ||
                spkBufFreeCnt < processBytes) {
                return;
            }
        }

        ProcessTime(SCP_PROCESS_TASK,
                    msg_id, num_queue_element, true);

        RingBuf_copyToLinear_dma((char *)dl_buf_in.data_buf.p_buffer,
                                 preProcessRingBuf,
                                 processBytes,
                                 memory_type,
                                 SMART_PA_MEM_ID);
        spk_invalidate_cache((uint32_t)dl_buf_in.data_buf.p_buffer, processBytes);

        dl_buf_in.data_buf.data_size = processBytes;
        dl_buf_out.data_buf.data_size = 0;
        if (isStateWorking) {
            RingBuf_copyToLinear_dma((char *)dl_buf_iv.data_buf.p_buffer,
                                     &uSpeaker_Protection_Info.rMemIVSpk.memory_buf,
                                     processBytes,
                                     uSpeaker_Protection_Info.rMemIVSpk.memory_type,
                                     SMART_PA_MEM_ID);
            spk_invalidate_cache((uint32_t)dl_buf_iv.data_buf.p_buffer, processBytes);
            dl_buf_iv.data_buf.data_size = processBytes;

            spk_dump_pcm(DUMP_IV_DATA, (uint32_t)dl_buf_iv.data_buf.p_buffer,
                         processBytes);
        } else {
            dl_buf_iv.data_buf.data_size = 0;
        }

        if (bypass_lib_flag) {
            spk_dump_pcm(DUMP_DEBUG_DATA, (uint32_t)dl_buf_in.data_buf.p_buffer,
                         processBytes);
            spk_flush_cache((uint32_t)dl_buf_in.data_buf.p_buffer,
                            processBytes);
            aud_dma_transaction_wrap((uint32_t)dl_buf_out.data_buf.p_buffer,
                                     (uint32_t)dl_buf_in.data_buf.p_buffer,
                                     dl_buf_in.data_buf.data_size,
                                     uSpeaker_Protection_Info.rMemSpkDL.memory_type,
                                     SMART_PA_MEM_ID);
            spk_invalidate_cache((uint32_t)dl_buf_out.data_buf.p_buffer,
                                 processBytes);
            dl_buf_out.data_buf.data_size = processBytes;
        } else {
            spk_dump_pcm(DUMP_DEBUG_DATA, (uint32_t)dl_buf_iv.data_buf.p_buffer,
                         processBytes);
            ProcessTime(SCP_PROCESS_LIB,
                        msg_id, num_queue_element, true);
            the_lib.arsi_process_dl_buf(&dl_buf_in, &dl_buf_out, &dl_buf_iv, NULL, smartpa_arsi_handler);
            ProcessTime(SCP_PROCESS_LIB,
                        msg_id, num_queue_element, false);
        }

        if (dl_buf_out.data_buf.data_size == 0) {
            AUD_LOG_D("DLCOPY [Warn] lib processing fail, process data is 0x%x",
                      dl_buf_out.data_buf.data_size);
        } else {
            if (processBytes != dl_buf_out.data_buf.data_size) {
                AUD_LOG_W("processBytes != dl_buf_out.data_buf.data_size");
            }
            processBytes = dl_buf_out.data_buf.data_size;

            spk_flush_cache((uint32_t)dl_buf_out.data_buf.p_buffer, processBytes);

            RingBuf_copyFromLinear_dma(&uSpeaker_Protection_Info.rMemSpkDL.memory_buf,
                                       (char *)dl_buf_out.data_buf.p_buffer,
                                       processBytes,
                                       uSpeaker_Protection_Info.rMemSpkDL.memory_type,
                                       SMART_PA_MEM_ID);
            spk_dump_pcm(DUMP_PCM_PRE, (uint32_t)dl_buf_out.data_buf.p_buffer,
                         processBytes);
        }
    } while (!isStateWorking); // Write all data in rMemPlatformDL before state working

    ProcessTime(SCP_PROCESS_TASK, msg_id, num_queue_element, false);
}

/*
    The task like HISR.
    Please do data moving inside the task.
*/

static void task_speaker_protection_task_loop(void *pvParameters)
{
    AudioTask *this = (AudioTask *)pvParameters;
    uint8_t local_queue_idx = 0xFF;
    uint8_t debug_time_idx = 0;
    ipi_msg_t *ipi_msg;
    uSpeaker_Protection_Task_msg_t uSpeakerProctionMsg;
    const struct mtk_memif_reg *spk_memif;
    uint32_t data_size = 0;
    uint64_t task_end_ns, task_start_ns;

    AUD_LOG_D("%s loop start", SPK_TASK_LOG_NAME);

    while (1) {
        xQueueReceive(this->msg_idx_queue, &local_queue_idx, portMAX_DELAY);
        ipi_msg = GET_IPI_MSG(local_queue_idx);
        task_debug_time[debug_time_idx].msg_id = ipi_msg->msg_id;
        task_start_ns = timer_get_global_timer_tick();

        uint32_t *tempPayload = (uint32_t *)(ipi_msg->payload);
        uSpeakerProctionMsg.msg_id = ipi_msg->msg_id;
        uSpeakerProctionMsg.param1 = ipi_msg->param1;
        uSpeakerProctionMsg.param2 = ipi_msg->param2;
        uSpeakerProctionMsg.payload0 = *tempPayload;
        uSpeakerProctionMsg.payload1 = *(tempPayload + 1);
        uSpeakerProctionMsg.payload2 = *(tempPayload + 2);
        uSpeakerProctionMsg.payload3 = *(tempPayload + 3);
        uSpeakerProctionMsg.payload4 = *(tempPayload + 4);
        //DumpSpeakerProteMsg("SPK_PROTECT", uSpeakerProctionMsg);

        switch (uSpeakerProctionMsg.msg_id) {
        case SPK_PROTECT_GET_TCM_BUF: {
#if defined(CFG_MTK_SMARTPA_IV_BUF_IN_TCM)
            memset(iv_buffer, 0, IV_TCM_BUF_SIZE);
            ipi_msg->param1 = (uint32_t)iv_buffer;
            ipi_msg->param2 = IV_TCM_BUF_SIZE;
            AUD_LOG_D("SPK_PROTECT_GET_TCM_BUF, base_addr 0x%x, size %d",
                      ipi_msg->param1, ipi_msg->param2);
#else
            AUD_LOG_D("SPK_PROTECT_GET_TCM_BUF not support");
#endif
            break;
        }
        case SPK_PROTECT_HWPARAM: {
            DumpSpeakerProteMsg("SPK_PROTECT_HWPARAM", uSpeakerProctionMsg);
            break;
        }
        case SPK_PROTECT_OPEN: {
            DumpSpeakerProteMsg("SPK_PROTECT_OPEN", uSpeakerProctionMsg);
            scp_wakeup_src_setup(AUDIO_WAKE_CLK_CTRL, true);
            enable_clk_bus(SMART_PA_MEM_ID);
            memset(&uSpeaker_Protection_Info, 0, sizeof(uSpeaker_Protection_Info));
            uSpeaker_Protection_Info.mscenario = Speaker_protect_playback;
#ifdef SMARTPA_WAKELOCK
            wake_lock(&spk_task_monitor_wakelock);
#endif
            // enable ultra
            if (bus_qos) {
                bus_qos(1);
            }

            spk_irq_update(uSpeakerProctionMsg.param1, uSpeakerProctionMsg.param2);

#ifdef PROCESS_TIME
            memset(task_max_process_time_ns, 0, sizeof(task_max_process_time_ns));
            memset(task_total_process_time, 0, sizeof(task_total_process_time));
            memset(task_avg_process_time, 0, sizeof(task_avg_process_time));
            memset(task_avg_count, 0, sizeof(task_avg_count));
            lib_max_process_time_ns = 0;
            lib_total_process_time = 0;
            lib_avg_process_time = 0;
            lib_avg_count = 0;
            lib_start_time_ns = 0;
            lib_stop_time_ns  = 0;
            lib_time_diff_ns = 0;
#endif
            dataRemained = 0;
            break;
        }
        case SPK_PROTECT_CLOSE: {
            DumpSpeakerProteMsg("SPK_PROTECT_CLOSE", uSpeakerProctionMsg);

            // disable ultra
            if (bus_qos) {
                bus_qos(0);
            }

            if (uSpeaker_Protection_Info.mscenario != Speaker_protect_playback) {
                AUD_LOG_W("SPK_PROTECT_CLOSE, error state (%d)", this->state);
                break;
            }

            scp_wakeup_src_setup(AUDIO_WAKE_CLK_CTRL, false);
            disable_clk_bus(SMART_PA_MEM_ID);
#ifdef SMARTPA_WAKELOCK
            wake_unlock(&spk_task_monitor_wakelock);
#endif
            uSpeaker_Protection_Info.mscenario = Speaker_protect_idle;
            this->state = AUDIO_TASK_DEINIT;
#if defined(PROCESS_TIME)
            AUD_LOG_D("Playback mode, lib max process time = %llu usec, lib avg process time = %llu usec",
                      TIME_NS_TO_US(lib_max_process_time_ns), TIME_NS_TO_US(lib_avg_process_time));
#endif
            AUD_LOG_D("SCP CPU = %uMHz", get_cur_clk());
            break;
        }
        case SPK_PROTECT_PLATMEMPARAM: {
            DumpSpeakerProteMsg("SPK_PROTECT_PLATMEMPARAM", uSpeakerProctionMsg);
            Memory_block = &uSpeaker_Protection_Info.rMemPlatformDL;
            Memory_block->phys_addr = uSpeakerProctionMsg.payload0;
            Memory_block->virt_addr = uSpeakerProctionMsg.payload1;
            Memory_block->memory_buf.bufLen = uSpeakerProctionMsg.payload2;
            Memory_block->memory_type = uSpeakerProctionMsg.payload3;
            Memory_block->memory_id = SMART_PA_MEM_ID;

            if (Memory_block->memory_type == MEMORY_AUDIO_SRAM) {
                Memory_block->memory_buf.pBufBase = (char *)(Memory_block->phys_addr -
                                                             AFE_INTERNAL_SRAM_PHY_BASE +
                                                             AFE_INTERNAL_SRAM_CM4_MAP_BASE);
            } else if (Memory_block->memory_type == MEMORY_AUDIO_DRAM) {
                Memory_block->memory_buf.pBufBase = (char *)ap_to_scp(Memory_block->phys_addr);
            }
            Memory_block->memory_buf.pBufEnd = Memory_block->memory_buf.pBufBase +
                                               Memory_block->memory_buf.bufLen;
            Memory_block->memory_buf.pRead =  Memory_block->memory_buf.pBufBase;
            Memory_block->memory_buf.pWrite = Memory_block->memory_buf.pBufBase;
            //dumpRingBuf("SPK_PROTECT_PLATMEMPARAM", Memory_block->memory_buf);
            break;
        }
        case SPK_PROTECT_DLMEMPARAM: {
            DumpSpeakerProteMsg("SPK_PROTECT_DLMEMPARAM", uSpeakerProctionMsg);
            Memory_block = &uSpeaker_Protection_Info.rMemSpkDL;
            Memory_block->phys_addr = uSpeakerProctionMsg.payload0;
            Memory_block->virt_addr = uSpeakerProctionMsg.payload1;
            Memory_block->memory_buf.bufLen = uSpeakerProctionMsg.payload2;
            Memory_block->memory_type = uSpeakerProctionMsg.payload3;
            Memory_block->memory_id = uSpeakerProctionMsg.payload4;
            if (Memory_block->memory_type == MEMORY_AUDIO_SRAM) {
                Memory_block->memory_buf.pBufBase = (char *)(Memory_block->phys_addr -
                                                             AFE_INTERNAL_SRAM_PHY_BASE +
                                                             AFE_INTERNAL_SRAM_CM4_MAP_BASE);
            } else if (Memory_block->memory_type == MEMORY_AUDIO_DRAM) {
                Memory_block->memory_buf.pBufBase = (char *)ap_to_scp(Memory_block->phys_addr);
            }
            Memory_block->memory_buf.pBufEnd = Memory_block->memory_buf.pBufBase +
                                               Memory_block->memory_buf.bufLen;
            Memory_block->memory_buf.pRead =  Memory_block->memory_buf.pBufBase;
            Memory_block->memory_buf.pWrite = Memory_block->memory_buf.pBufBase;
            spk_memif_update(Memory_block->memory_id);
            //dumpRingBuf("SPK_PROTECT_DLMEMPARAM", Memory_block->memory_buf);
            break;
        }
        case SPK_PROTECT_PREPARE: {
            DumpSpeakerProteMsg("SPK_PROTECT_PREPARE", uSpeakerProctionMsg);
            uSpeaker_Protection_Info.mdlformat = uSpeakerProctionMsg.payload0;
            uSpeaker_Protection_Info.mdlsamplerate = uSpeakerProctionMsg.payload1;
            uSpeaker_Protection_Info.mdlchannel = uSpeakerProctionMsg.payload2;
            uSpeaker_Protection_Info.mdperiodsize = uSpeakerProctionMsg.payload3;

            UpdateArsiTaskconfig();

            Memory_block = &uSpeaker_Protection_Info.rMemSpkDL;
            ResetMemIndex(Memory_block);
            Memory_block = &uSpeaker_Protection_Info.rMemPlatformDL;
            ResetMemIndex(Memory_block);
            Memory_block = &uSpeaker_Protection_Info.rMemIVSpk;
            ResetMemIndex(Memory_block);
            this->state = AUDIO_TASK_INIT;
            break;
        }
        case SPK_PROTECT_IVMEMPARAM: {
            DumpSpeakerProteMsg("SPK_PROTECT_IVMEMPARAM", uSpeakerProctionMsg);
            Memory_block = &uSpeaker_Protection_Info.rMemIVSpk;
            Memory_block->phys_addr = uSpeakerProctionMsg.payload0;
            Memory_block->virt_addr = uSpeakerProctionMsg.payload1;
            Memory_block->memory_buf.bufLen = uSpeakerProctionMsg.payload2;
            Memory_block->memory_type = uSpeakerProctionMsg.payload3;
            Memory_block->memory_id = uSpeakerProctionMsg.payload4;
            if (Memory_block->memory_type == MEMORY_AUDIO_SRAM) {
                Memory_block->memory_buf.pBufBase = (char *)(Memory_block->phys_addr -
                                                             AFE_INTERNAL_SRAM_PHY_BASE +
                                                             AFE_INTERNAL_SRAM_CM4_MAP_BASE);
            } else if (Memory_block->memory_type == MEMORY_AUDIO_DRAM) {
                Memory_block->memory_buf.pBufBase = (char *)ap_to_scp(Memory_block->phys_addr);
            }
            Memory_block->memory_buf.pBufEnd = Memory_block->memory_buf.pBufBase +
                                               Memory_block->memory_buf.bufLen;
            Memory_block->memory_buf.pRead =  Memory_block->memory_buf.pBufBase;
            Memory_block->memory_buf.pWrite = Memory_block->memory_buf.pBufBase;
            spk_memif_update(Memory_block->memory_id);
            //dumpRingBuf("SPK_PROTECT_IVMEMPARAM", Memory_block->memory_buf);
            break;
        }
        case SPK_PROTECT_DLCOPY: {
            unsigned int IVread_index = 0;
            unsigned int read_index = 0;
            uint32_t count;

            /*
            AUD_LOG_D("SPK_PROTECT_DLCOPY uSndMsg=0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
                      uSpeakerProctionMsg.msg_id, uSpeakerProctionMsg.param1, uSpeakerProctionMsg.param2,
                      uSpeakerProctionMsg.payload0, uSpeakerProctionMsg.payload1);*/
            /* update for platform memory */

            if (this->state == AUDIO_TASK_WORKING) {
                spk_memif = spk_memif_get(SPK_MEMIF_DL);
                read_index = memalign(DRV_Reg32(spk_memif->reg_cur) -
                                      DRV_Reg32(spk_memif->reg_base));

                spk_memif = spk_memif_get(SPK_MEMIF_IV);
                IVread_index = memalign(DRV_Reg32(spk_memif->reg_cur) -
                                        DRV_Reg32(spk_memif->reg_base));
            } else {
                read_index = 0;
                IVread_index = 0;
            }

            /* get spk buffer bytes */
            count = getRingBufCount(&uSpeaker_Protection_Info.rMemSpkDL.memory_buf,
                                    read_index, true);
            RingBuf_update_readptr(&uSpeaker_Protection_Info.rMemSpkDL.memory_buf,
                                   count);

            /* get iv buffer bytes */
            count = getRingBufCount(&uSpeaker_Protection_Info.rMemIVSpk.memory_buf,
                                    IVread_index, false);
            RingBuf_update_writeptr(&uSpeaker_Protection_Info.rMemIVSpk.memory_buf,
                                    count);

            /* get DL buffer bytes */
            unsigned int framecount = uSpeakerProctionMsg.payload1;
            unsigned int framebytes = frame_to_bytes(framecount,
                                                     uSpeaker_Protection_Info.mdlchannel,
                                                     uSpeaker_Protection_Info.mdlformat);
            RingBuf_update_writeptr(&uSpeaker_Protection_Info.rMemPlatformDL.memory_buf,
                                    framebytes);

            /* process data */
            if (this->state == AUDIO_TASK_WORKING) {
                dlDataProcessing(framebytes, this->num_queue_element,
                                 true, SPK_PROTECT_DLCOPY);
            } else {
                dlDataProcessing(framebytes, this->num_queue_element,
                                 false, SPK_PROTECT_DLCOPY);
            }

            //dumpRingBuf("SPK_PROTECT_DLCOPY rMemIVSpk",uSpeaker_Protection_Info.rMemIVSpk.memory_buf);
            //dumpRingBuf("DLCOPY MemPlatformDL",uSpeaker_Protection_Info.rMemPlatformDL.memory_buf);
            //dumpRingBuf("DLCOPY MemSpkDL",uSpeaker_Protection_Info.rMemSpkDL.memory_buf);
            break;
        }
        case SPK_PROTECT_START: {
            DumpSpeakerProteMsg("SPK_PROTECT_START", uSpeakerProctionMsg);
            task_speaker_protection_start(this);
            this->state = AUDIO_TASK_WORKING;
            unmask_irq(AUDIO_IRQn);
            break;
        }
        case SPK_PROTECT_STOP: {
            DumpSpeakerProteMsg("SPK_PROTECT_STOP", uSpeakerProctionMsg);
            mask_irq(AUDIO_IRQn);
            task_speaker_protection_stop(this);
            dataRemained = 0;
            Memory_block = &uSpeaker_Protection_Info.rMemSpkDL;
            ResetMemIndex(Memory_block);
            Memory_block = &uSpeaker_Protection_Info.rMemPlatformDL;
            ResetMemIndex(Memory_block);
            Memory_block = &uSpeaker_Protection_Info.rMemIVSpk;
            ResetMemIndex(Memory_block);
            this->state = AUDIO_TASK_IDLE;
            break;
        }
        case SPK_PROTECT_IRQDL: {
            if (this->state == AUDIO_TASK_IDLE) {
                break;
            }

            spk_memif = spk_memif_get(SPK_MEMIF_DL);
            if (DRV_Reg32(spk_memif->reg_cur) == 0) {
                DumpSpeakerProteMsg("SPK_PROTECT_IRQDL AFE_DL1_CUR =0", uSpeakerProctionMsg);
                break;
            }

            unsigned int read_index = memalign(DRV_Reg32(spk_memif->reg_cur) -
                                               DRV_Reg32(spk_memif->reg_base));
            uint32_t count;
            count = getRingBufCount(&uSpeaker_Protection_Info.rMemSpkDL.memory_buf,
                                    read_index, true);
            RingBuf_update_readptr(&uSpeaker_Protection_Info.rMemSpkDL.memory_buf,
                                   count);

            unsigned int IVread_index = 0;
            spk_memif = spk_memif_get(SPK_MEMIF_IV);
            IVread_index = memalign(DRV_Reg32(spk_memif->reg_cur) -
                                    DRV_Reg32(spk_memif->reg_base));
            count = getRingBufCount(&uSpeaker_Protection_Info.rMemIVSpk.memory_buf,
                                    IVread_index, false);
            RingBuf_update_writeptr(&uSpeaker_Protection_Info.rMemIVSpk.memory_buf,
                                    count);
#ifdef Dump_IV_data
            dumpdatacount++;
            DumpbufferData(uSpeaker_Protection_Info.rMemIVSpk.memory_buf.pWrite);
#endif

            //dumpRingBuf("SPK_PROTECT_IRQDL rMemIVSpk",uSpeaker_Protection_Info.rMemIVSpk.memory_buf);
            //dumpRingBuf("IRQDL MemPlatformDL",uSpeaker_Protection_Info.rMemPlatformDL.memory_buf);
            //dumpRingBuf("IRQDL MemSpkDL",uSpeaker_Protection_Info.rMemSpkDL.memory_buf);
            ipi_msg_t ipi_msg_irqdl;
            int retrycount = 10, ret = 0;
            do {
                ret = audio_send_ipi_msg(&ipi_msg_irqdl, this->scene,
                                         AUDIO_IPI_LAYER_TO_KERNEL,
                                         AUDIO_IPI_MSG_ONLY, AUDIO_IPI_MSG_BYPASS_ACK,
                                         SPK_PROTECT_IRQDL, read_index,
                                         (unsigned int)(uSpeaker_Protection_Info.rMemPlatformDL.memory_buf.pRead), NULL);
                if (ret != 0) {
                    AUD_LOG_W("audio_send_ipi_msg fail retry count: %d, queue num: %d\n",
                              retrycount, this->num_queue_element);
                    vTaskDelay(1 / portTICK_RATE_MS);
                }
                retrycount--;
            } while ((ret != 0) && (retrycount >= 0));

            break;
        }
        case SPK_PROTECT_SPEECH_OPEN: {
            DumpSpeakerProteMsg("SPK_PROTECT_SPEECH_OPEN", uSpeakerProctionMsg);
            scp_wakeup_src_setup(AUDIO_WAKE_CLK_CTRL, true);
            enable_clk_bus(SMART_PA_MEM_ID);
#ifdef SMARTPA_WAKELOCK
            wake_lock(&spk_task_monitor_wakelock);
#endif
            // enable ultra
            if (bus_qos) {
                bus_qos(1);
            }

            spk_irq_update(uSpeakerProctionMsg.param1, uSpeakerProctionMsg.param2);

            memset(&uSpeaker_Protection_Info, 0, sizeof(uSpeaker_Protection_Info));
            memset(task_max_process_time_ns, 0, sizeof(task_max_process_time_ns));
            memset(task_total_process_time, 0, sizeof(task_total_process_time));
            memset(task_avg_process_time, 0, sizeof(task_avg_process_time));
            memset(task_avg_count, 0, sizeof(task_avg_count));
            uSpeaker_Protection_Info.mscenario = Speaker_protect_speech;
            lib_max_process_time_ns = 0;
            lib_total_process_time = 0;
            lib_avg_process_time = 0;
            lib_avg_count = 0;
            lib_start_time_ns = 0;
            lib_stop_time_ns  = 0;
            lib_time_diff_ns = 0;
            break;
        }
        case SPK_PROTECT_SPEECH_CLOSE: {
            DumpSpeakerProteMsg("SPK_PROTECT_SPEECH_CLOSE", uSpeakerProctionMsg);

            // disable ultra
            if (bus_qos) {
                bus_qos(0);
            }

            if (uSpeaker_Protection_Info.mscenario != Speaker_protect_speech) {
                AUD_LOG_W("SPK_PROTECT_SPEECH_CLOSE, error state (%d)", this->state);
                break;
            }

            this->state = AUDIO_TASK_DEINIT;
            scp_wakeup_src_setup(AUDIO_WAKE_CLK_CTRL, false);
            disable_clk_bus(SMART_PA_MEM_ID);

#ifdef SMARTPA_WAKELOCK
            wake_unlock(&spk_task_monitor_wakelock);
#endif
            uSpeaker_Protection_Info.mscenario = Speaker_protect_idle;
#if defined(PROCESS_TIME)
            AUD_LOG_D("PhoneCall mode, lib max time = %llu usec, lib avg time = %llu usec\n",
                      TIME_NS_TO_US(lib_max_process_time_ns), TIME_NS_TO_US(lib_avg_process_time));
#endif
            AUD_LOG_D("SCP CPU: %uM Hz\n", get_cur_clk());
            break;
        }
        case SPK_PROTECT_SPEECH_PREPARE: {
            DumpSpeakerProteMsg("SPK_PROTECT_SPEECH_PREPARE", uSpeakerProctionMsg);
            uSpeaker_Protection_Info.mdlformat = uSpeakerProctionMsg.payload0;
            uSpeaker_Protection_Info.mdlsamplerate = uSpeakerProctionMsg.payload1;
            uSpeaker_Protection_Info.mdlchannel = uSpeakerProctionMsg.payload2;
            uSpeaker_Protection_Info.mdperiodsize = uSpeakerProctionMsg.payload3;

            UpdateArsiTaskconfig();

            /* base on stop with interrrupt , reset when do prepare*/
            Memory_block = &uSpeaker_Protection_Info.rMemSpkDL;
            ResetMemIndex(Memory_block);
            Memory_block = &uSpeaker_Protection_Info.rMemMDDL;
            ResetMemIndex(Memory_block);
            Memory_block = &uSpeaker_Protection_Info.rMemIVSpk;
            ResetMemIndex(Memory_block);
            unsigned int processingframebytes = frame_to_bytes(
                                                    uSpeaker_Protection_Info.mdperiodsize,
                                                    uSpeaker_Protection_Info.mdlchannel,
                                                    uSpeaker_Protection_Info.mdlformat);
            AUD_LOG_D("SPK_PROTECT_SPEECH_PREPARE processingframebytes = %d\n",
                      processingframebytes);
            RingBuf_update_writeptr(&uSpeaker_Protection_Info.rMemSpkDL.memory_buf,
                                    processingframebytes * 2);
            dumpRingBuf("SPK_PROTECT_SPEECH_PREPARE rMemIVSpk",
                        uSpeaker_Protection_Info.rMemIVSpk.memory_buf);
            dumpRingBuf("SPK_PROTECT_SPEECH_PREPARE rMemMDDL",
                        uSpeaker_Protection_Info.rMemMDDL.memory_buf);
            dumpRingBuf("SPK_PROTECT_SPEECH_PREPARE MemSpkDL",
                        uSpeaker_Protection_Info.rMemSpkDL.memory_buf);
            this->state = AUDIO_TASK_INIT;
            break;
        }
        case SPK_PROTECT_SPEECH_IVMEMPARAM: {
            DumpSpeakerProteMsg("SPK_PROTECT_SPEECH_IVMEMPARAM", uSpeakerProctionMsg);
            /* speech path IV mem info*/
            Memory_block = &uSpeaker_Protection_Info.rMemIVSpk;
            Memory_block->phys_addr = uSpeakerProctionMsg.payload0;
            Memory_block->virt_addr = uSpeakerProctionMsg.payload1;
            Memory_block->memory_buf.bufLen = uSpeakerProctionMsg.payload2;
            Memory_block->memory_type = uSpeakerProctionMsg.payload3;
            Memory_block->memory_id = uSpeakerProctionMsg.payload4;

            if (Memory_block->memory_type == MEMORY_AUDIO_SRAM) {
                Memory_block->memory_buf.pBufBase = (char *)(Memory_block->phys_addr -
                                                             AFE_INTERNAL_SRAM_PHY_BASE +
                                                             AFE_INTERNAL_SRAM_CM4_MAP_BASE);
            } else if (Memory_block->memory_type == MEMORY_AUDIO_DRAM) {
                Memory_block->memory_buf.pBufBase = (char *)ap_to_scp(Memory_block->phys_addr);
            }
            Memory_block->memory_buf.pBufEnd = Memory_block->memory_buf.pBufBase +
                                               Memory_block->memory_buf.bufLen;
            Memory_block->memory_buf.pRead =  Memory_block->memory_buf.pBufBase;
            Memory_block->memory_buf.pWrite = Memory_block->memory_buf.pBufBase;
            spk_memif_update(Memory_block->memory_id);
            //dumpRingBuf("SPK_PROTECT_SPEECH_IVMEMPARAM", Memory_block->memory_buf);
            break;
        }
        case SPK_PROTECT_SPEECH_MDFEEDBACKPARAM: {
            DumpSpeakerProteMsg("SPK_PROTECT_SPEECH_MDFEEDBACKPARAM", uSpeakerProctionMsg);
            /* speech path MD dl mem info*/
            Memory_block = &uSpeaker_Protection_Info.rMemMDDL;
            Memory_block->phys_addr = uSpeakerProctionMsg.payload0;
            Memory_block->virt_addr = uSpeakerProctionMsg.payload1;
            Memory_block->memory_buf.bufLen = uSpeakerProctionMsg.payload2;
            Memory_block->memory_type = uSpeakerProctionMsg.payload3;
            Memory_block->memory_id = uSpeakerProctionMsg.payload4;

            if (Memory_block->memory_type == MEMORY_AUDIO_SRAM) {
                Memory_block->memory_buf.pBufBase = (char *)(Memory_block->phys_addr -
                                                             AFE_INTERNAL_SRAM_PHY_BASE +
                                                             AFE_INTERNAL_SRAM_CM4_MAP_BASE);
            } else if (Memory_block->memory_type == MEMORY_AUDIO_DRAM) {
                Memory_block->memory_buf.pBufBase = (char *)ap_to_scp(Memory_block->phys_addr);
            }
            Memory_block->memory_buf.pBufEnd = Memory_block->memory_buf.pBufBase +
                                               Memory_block->memory_buf.bufLen;
            Memory_block->memory_buf.pRead =  Memory_block->memory_buf.pBufBase;
            Memory_block->memory_buf.pWrite = Memory_block->memory_buf.pBufBase;
            spk_memif_update(Memory_block->memory_id);
            dumpRingBuf("SPK_PROTECT_SPEECH_MDFEEDBACKPARAM", Memory_block->memory_buf);
            break;
        }
        case SPK_PROTECT_SPEECH_DLMEMPARAM: {
            DumpSpeakerProteMsg("SPK_PROTECT_SPEECH_DLMEMPARAM", uSpeakerProctionMsg);
            Memory_block = &uSpeaker_Protection_Info.rMemSpkDL;
            Memory_block->phys_addr = uSpeakerProctionMsg.payload0;
            Memory_block->virt_addr = uSpeakerProctionMsg.payload1;
            Memory_block->memory_buf.bufLen = uSpeakerProctionMsg.payload2;
            Memory_block->memory_type = uSpeakerProctionMsg.payload3;
            Memory_block->memory_id = uSpeakerProctionMsg.payload4;

            if (Memory_block->memory_type == MEMORY_AUDIO_SRAM) {
                Memory_block->memory_buf.pBufBase = (char *)(Memory_block->phys_addr -
                                                             AFE_INTERNAL_SRAM_PHY_BASE +
                                                             AFE_INTERNAL_SRAM_CM4_MAP_BASE);
            } else if (Memory_block->memory_type == MEMORY_AUDIO_DRAM) {
                Memory_block->memory_buf.pBufBase = (char *)ap_to_scp(Memory_block->phys_addr);
            }
            Memory_block->memory_buf.pBufEnd = Memory_block->memory_buf.pBufBase +
                                               Memory_block->memory_buf.bufLen;
            Memory_block->memory_buf.bufLen = Memory_block->memory_buf.bufLen;
            Memory_block->memory_buf.pRead =  Memory_block->memory_buf.pBufBase;
            Memory_block->memory_buf.pWrite = Memory_block->memory_buf.pBufBase;
            spk_memif_update(Memory_block->memory_id);
            dumpRingBuf("SPK_PROTECT_SPEECH_DLMEMPARAM", Memory_block->memory_buf);
            break;
        }
        case SPK_PROTECT_SPEECH_START: {
            DumpSpeakerProteMsg("SPK_PROTECT_SPEECH_START", uSpeakerProctionMsg);
            task_speaker_protection_start(this);
            this->state = AUDIO_TASK_WORKING;
            unmask_irq(AUDIO_IRQn);
            break;
        }
        case SPK_PROTECT_SPEECH_STOP: {
            DumpSpeakerProteMsg("SPK_PROTECT_SPEECH_STOP", uSpeakerProctionMsg);
            mask_irq(AUDIO_IRQn);
            task_speaker_protection_stop(this);
            this->state = AUDIO_TASK_IDLE;
            break;
        }
        case SPK_PROTECT_SPEECH_IRQDL: {

            if (this->state == AUDIO_TASK_IDLE) {
                break;
            }

            spk_memif = spk_memif_get(SPK_MEMIF_DL);
            if (DRV_Reg32(spk_memif->reg_cur) == 0) {
                DumpSpeakerProteMsg("SPK_PROTECT_SPEECH_IRQDL AFE_DL1_CUR =0",
                                    uSpeakerProctionMsg);
                break;
            }

            /* get spk buffer data bytes */
            unsigned int read_index = memalign(DRV_Reg32(spk_memif->reg_cur) -
                                               DRV_Reg32(spk_memif->reg_base));
            uint32_t count;
            count = getRingBufCount(&uSpeaker_Protection_Info.rMemSpkDL.memory_buf,
                                    read_index, true);
            RingBuf_update_readptr(&uSpeaker_Protection_Info.rMemSpkDL.memory_buf,
                                   count);

            /* get iv data bytes */
            unsigned int IVread_index = 0;
            spk_memif = spk_memif_get(SPK_MEMIF_IV);
            IVread_index = memalign(DRV_Reg32(spk_memif->reg_cur) -
                                    DRV_Reg32(spk_memif->reg_base));
            count = getRingBufCount(&uSpeaker_Protection_Info.rMemIVSpk.memory_buf,
                                    IVread_index, false);
            RingBuf_update_writeptr(&uSpeaker_Protection_Info.rMemIVSpk.memory_buf,
                                    count);

            /* get MD data bytes */
            spk_memif = spk_memif_get(SPK_MEMIF_MD_UL);
            unsigned int  awbread_index = memalign(DRV_Reg32(spk_memif->reg_cur) -
                                                   DRV_Reg32(spk_memif->reg_base));
            count = getRingBufCount(&uSpeaker_Protection_Info.rMemMDDL.memory_buf,
                                    awbread_index, false);
            RingBuf_update_writeptr(&uSpeaker_Protection_Info.rMemMDDL.memory_buf,
                                    count);

            unsigned int processBytes = frame_to_bytes(
                                        uSpeaker_Protection_Info.mdperiodsize,
                                        uSpeaker_Protection_Info.mdlchannel,
                                        uSpeaker_Protection_Info.mdlformat);

            unsigned int DLfreebufcount = RingBuf_getFreeSpace(
                                              &uSpeaker_Protection_Info.rMemSpkDL.memory_buf);
            unsigned int DLbufcount = RingBuf_getDataCount(
                                              &uSpeaker_Protection_Info.rMemSpkDL.memory_buf);
            unsigned int IVbufcount = RingBuf_getDataCount(
                                          &uSpeaker_Protection_Info.rMemIVSpk.memory_buf);
            unsigned int MdDLbufcount = RingBuf_getDataCount(
                                            &uSpeaker_Protection_Info.rMemMDDL.memory_buf);

            AUD_LOG_V("DLbufcount:%d(0x%x), IVbufcount:%d(0x%x), MdDLbufcount:%d(0x%x)",
                      DLbufcount, DLbufcount, IVbufcount, IVbufcount, MdDLbufcount, MdDLbufcount);

            if (DLbufcount < 0 ||
                DLfreebufcount < 0 ||
                IVbufcount < processBytes ||
                MdDLbufcount < processBytes) {
                AUD_LOG_E("SPK_SPEECH_IRQDL, data error, DLfreebufcount:%d, IVbufcount:%d, MdDLbufcount:%d",
                          DLfreebufcount, IVbufcount, MdDLbufcount);
            }

            /* process all the data if data is enough */
            while (DLfreebufcount >= processBytes &&
                   IVbufcount >= processBytes &&
                   MdDLbufcount >= processBytes) {

                dlDataProcessing(count, this->num_queue_element,
                                 true, SPK_PROTECT_SPEECH_IRQDL);

                dl_buf_out.data_buf.data_size = 0;
                dl_buf_iv.data_buf.data_size = 0;
                DLfreebufcount -= processBytes;
                IVbufcount -= processBytes;
                MdDLbufcount -= processBytes;
            }

#ifdef Dump_IV_data
            dumpdatacount++;
            DumpbufferData(uSpeaker_Protection_Info.rMemIVSpk.memory_buf.pWrite);
#endif
            break;
        }
        case SPK_IPI_MSG_A2D_DL_ENHANCE_ON: {
            AUD_LOG_D("%s loop, SPK_IPI_MSG_A2D_DL_ENHANCE_ON = %d\n",
                      SPK_TASK_LOG_NAME, ipi_msg->param1);
            if (smartpa_arsi_handler != NULL) {
                the_lib.arsi_set_dl_enhance(ipi_msg->param1, smartpa_arsi_handler);
            }
            break;
        }
        case SPK_IPI_MSG_A2D_PARAM: {
            if (bypass_lib_flag) {
                ipi_msg->param1 = 1;
                ipi_msg->param2 = 0;
                AUD_LOG_D("SPK_IPI_MSG_A2D_PARAM bypass_lib_flag: %d\n",
                          bypass_lib_flag);
                break;
            }

            memset(tuning_key_value, '\0', MAX_PARAM_SIZE);
            spk_flush_cache((uint32_t)tuning_key_value, MAX_PARAM_SIZE);
            audio_get_dma_from_msg(ipi_msg, &tuning_key_value, &data_size);
            AUD_ASSERT(ipi_msg->param1 <= param_buf.memory_size);

            param_buf.data_size = ipi_msg->param1;
            param_buf.p_buffer = tuning_key_value;
            param_buf.data_size = ipi_msg->param1;
            if (smartpa_arsi_handler != NULL) {
                if (the_lib.arsi_update_param(
                            &arsi_task_config,
                            &arsi_lib_config,
                            &param_buf,
                            smartpa_arsi_handler) != NO_ERROR) {
                            AUD_LOG_D("SPK_IPI_MSG_A2D_PARAM, arsi_update_param fail!!\n");
                }
            }
            ipi_msg->param1 = 1;
            ipi_msg->param2 = param_buf.data_size;
            AUD_LOG_D("SPK_IPI_MSG_A2D_PARAM bypass_lib_flag: %d, param_buf = %p, str len: %d\n",
                      bypass_lib_flag,
                      param_buf.p_buffer,
                      (int) param_buf.data_size);
            break;
        }
        case SPK_IPI_MSG_A2D_SET_ADDR_VALUE: {
            AUD_LOG_D("1 addr = 0x%x, value = 0x%x\n", ipi_msg->param1, ipi_msg->param2);
            if (smartpa_arsi_handler != NULL) {
                status_t retval = the_lib.arsi_set_addr_value(ipi_msg->param1,
                                                              ipi_msg->param2,
                                                              smartpa_arsi_handler);
                if (retval != NO_ERROR) {
                    AUD_LOG_W("arsi_set_addr_value fail!! retval = %d\n", retval);
                    ipi_msg->param1 = 0;
                } else {
                    AUD_LOG_V("arsi_set_addr_value pass!! retval = %d\n", retval);
                    ipi_msg->param1 = 1;
                }
            } else {
                ipi_msg->param1 = 0;
            }
            break;
        }
        case SPK_IPI_MSG_A2D_GET_ADDR_VALUE: {
            AUD_LOG_D("1 addr = 0x%x, value = 0x%x\n", ipi_msg->param1, ipi_msg->param2);
            if (smartpa_arsi_handler != NULL) {
                status_t retval = the_lib.arsi_get_addr_value(ipi_msg->param1,
                                                              &ipi_msg->param2,
                                                              smartpa_arsi_handler);
                AUD_LOG_V("2 addr = 0x%x, value = 0x%x\n", ipi_msg->param1, ipi_msg->param2);

                if (retval != NO_ERROR) {
                    AUD_LOG_W("arsi_get_addr_value fail!! retval = %d\n", retval);
                    ipi_msg->param1 = 0;
                } else {
                    AUD_LOG_V("arsi_get_addr_value pass!! retval = %d\n", retval);
                    ipi_msg->param1 = 1;
                }
            } else {
                /* test code to do get addr value*/
                AUD_LOG_D("test code  SPK_IPI_MSG_A2D_GET_ADDR_VALUE addr = 0x%x, value = 0x%x\n",
                          ipi_msg->param1, ipi_msg->param2);
                switch (ipi_msg->param1) {
                case 0x1234:
                    ipi_msg->param1 = 1;
                    ipi_msg->param2 = 0x5678;
                    break;
                case 0x1235:
                    ipi_msg->param1 = 1;
                    ipi_msg->param2 = 0x5679;
                    break;
                default:
                    ipi_msg->param1 = 0;
                    ipi_msg->param2 = 0xebad;
                    AUD_LOG_D("test code return addr = 0x%x, value = 0x%x\n", ipi_msg->param1,
                              ipi_msg->param2);
                    break;
                }
            }
            break;
        }
        case SPK_IPI_MSG_A2D_SET_KEY_VALUE: {
            memset(tuning_key_value, '\0', MAX_PARAM_SIZE);
            spk_flush_cache((uint32_t)tuning_key_value, MAX_PARAM_SIZE);
            audio_get_dma_from_msg(ipi_msg, &tuning_key_value, &data_size);
            string_buf_t key_value_pair;
            key_value_pair.memory_size = ipi_msg->param2;
            key_value_pair.string_size = ipi_msg->param1;
            key_value_pair.p_string = tuning_key_value;
            AUD_LOG_D("set key_value memsize 0x%x, string size %d, string %s\n",
                      (unsigned int) key_value_pair.memory_size,
                      (int) key_value_pair.string_size,
                      key_value_pair.p_string);

            if (bypass_lib_flag_set(tuning_key_value)) {
                ipi_msg->param1 = 1;
                break;
            }

            if (smartpa_arsi_handler != NULL && !bypass_lib_flag) {
                status_t retval = the_lib.arsi_set_key_value_pair(&key_value_pair,
                                                                  smartpa_arsi_handler);
                if (retval != NO_ERROR) {
                    AUD_LOG_W("arsi_set_key_value_pair fail!! retval = %d\n", (int) retval);
                    ipi_msg->param1 = 0;
                } else {
                    AUD_LOG_D("arsi_set_key_value_pair pass!! retval = %d\n", (int) retval);
                    ipi_msg->param1 = 1;
                }
            } else {
                ipi_msg->param1 = 0;
                AUD_LOG_D("smartpa_arsi_handler NULL or bypass_lib_flag\n");
            }
            break;
        }
        case SPK_IPI_MSG_A2D_GET_KEY_VALUE: {
            status_t retval;
            memset(tuning_key_value, '\0', MAX_PARAM_SIZE);
            spk_flush_cache((uint32_t)tuning_key_value, MAX_PARAM_SIZE);
            audio_get_dma_from_msg(ipi_msg, &tuning_key_value, &data_size);
            string_buf_t key_value_pair;
            key_value_pair.memory_size = ipi_msg->param2;
            key_value_pair.string_size = ipi_msg->param1;
            key_value_pair.p_string = tuning_key_value;
            AUD_LOG_D("get key_value memsize 0x%x, string size %d, string %s\n",
                      (unsigned int) key_value_pair.memory_size,
                      (int) key_value_pair.string_size,
                      key_value_pair.p_string);

            if (smartpa_arsi_handler != NULL) {
                if (bypass_lib_flag_get(&key_value_pair)) {
                    retval = NO_ERROR;
                } else {
                    retval = the_lib.arsi_get_key_value_pair(&key_value_pair,
                                                             smartpa_arsi_handler);
                }
                if (retval != NO_ERROR) {
                    AUD_LOG_W("arsi_set_key_value_pair fail!! retval = %d\n", (int) retval);
                    ipi_msg->param1 = 0;
                } else {
                    key_value_pair.string_size = strlen(key_value_pair.p_string);
                    ipi_msg->param1 = 1;
                    ipi_msg->param2 = key_value_pair.string_size;

                    audio_ipi_dma_write(ipi_msg->dma_info.wb_dram.addr,
                                      key_value_pair.p_string,
                                      key_value_pair.string_size);
                    ipi_msg->dma_info.wb_dram.data_size = key_value_pair.string_size;
                    AUD_LOG_D("arsi_get_key_value_pair pass!! retval = %d, str:%s, len: %d, wb size: %d, wb addr:%p\n",
                              (int) retval, key_value_pair.p_string,
                              (int) key_value_pair.string_size,
                              (int) ipi_msg->dma_info.wb_dram.data_size,
                              ipi_msg->dma_info.wb_dram.addr);
                }
            }
            break;
        }
        case SPK_IPI_MSG_A2D_PCM_DUMP_ON:
            smart_pa_dump_pcm_flag = ipi_msg->param1;
            AUD_LOG_D("dump_pcm_flag: %d\n", smart_pa_dump_pcm_flag);
            break;
        case SPK_IPI_MSG_A2D_LIB_LOG_ON:
            smartpa_lib_log_flag = ipi_msg->param1;;
            AUD_LOG_D("smartpa_lib_log_flag: %d\n", smartpa_lib_log_flag);
            break;
        case SPK_PROTTCT_PCMDUMP_ON:
            dump_pcm_flag = true;
            dram_dump.bufLen = uSpeakerProctionMsg.payload0;
            dram_dump.pBufBase = (char *)ap_to_scp(uSpeakerProctionMsg.payload1);
            dram_dump.pBufEnd = dram_dump.pBufBase + dram_dump.bufLen;
            dram_dump.pRead = dram_dump.pBufBase;
            dram_dump.pWrite = dram_dump.pBufBase;
            AUD_LOG_D("SPK_PROTTCT_PCMDUMP_ON bufLen = %d pBufBase = %p\n",
                      dram_dump.bufLen, dram_dump.pBufBase);
            break;
        case SPK_PROTTCT_PCMDUMP_OFF:
            AUD_LOG_D("SPK_PROTTCT_PCMDUMP_Off");
            dump_pcm_flag = false;
            break;
        default:
            break;
        }

        /* send ack back if need */
        int retrycount = 10, ret = 0;
        do {
            ret = audio_send_ipi_msg_ack_back(ipi_msg);
            if (ret != 0) {
                AUD_LOG_W("audio_send_ipi_msg_ack_back fail retry count: %d, queue num: %d\n",
                          retrycount, this->num_queue_element);
                vTaskDelay(1 / portTICK_RATE_MS);
            }
            retrycount--;
        } while ((ret != 0) && (retrycount >= 0));

        task_end_ns = timer_get_global_timer_tick();
        task_debug_time[debug_time_idx].diff_ns = GET_TIME_INTERVAL(task_end_ns, task_start_ns);
        debug_time_idx = (debug_time_idx == MAX_MSG_QUEUE_SIZE - 1) ? 0 : debug_time_idx + 1;

        /* clean msg */
        memset(ipi_msg, 0, ipi_msg_size);
        this->num_queue_element--;
    }
}


// ============================================================================

static void task_speaker_protection_constructor(struct AudioTask *this)
{
    AUD_ASSERT(this != NULL);

    AUD_LOG_D("+%s, Heap free = %d", SPK_TASK_LOG_NAME, xPortGetFreeHeapSize());

    /* Mask irq to scp first. Prevent irq handler triggered when reset. */
    Afe_Set_Reg(SCP_IRQ_EN_REG, 0x0, SCP_IRQ_EN_REG_MASK);

    /* assign initial value for class members & alloc private memory here */
    this->scene = TASK_SCENE_SPEAKER_PROTECTION;
    this->state = AUDIO_TASK_IDLE;

    /* queue */
    this->queue_idx = 0;
    this->num_queue_element = 0;
    ipi_msg_size = max(LIGHT_IPI_MSG_PAYLOAD_SIZE, IPI_MSG_DMA_INFO_SIZE) +
                   IPI_MSG_HEADER_SIZE;
    g_msg_array = (uint8_t *)kal_pvPortMalloc(MAX_MSG_QUEUE_SIZE * ipi_msg_size);
    AUD_ASSERT(g_msg_array != NULL);
    memset(g_msg_array, 0, MAX_MSG_QUEUE_SIZE * ipi_msg_size);

    this->msg_idx_queue = xQueueCreate(MAX_MSG_QUEUE_SIZE,
                                       sizeof(uSpeaker_Protection_Task_msg_t));
    AUD_ASSERT(this->msg_idx_queue != NULL);

#ifdef SMARTPA_WAKELOCK
    wake_lock_init(&spk_task_monitor_wakelock, "spkwl");
#endif

#if defined(MTK_SMARTPA_BYPASS_LIB)
    bypass_lib_flag = 1;
#endif

    smartpa_arsi_assign_lib_fp(&the_lib);
    AUD_ASSERT(the_lib.arsi_create_handler != NULL); /* TODO: check all functions */

    string_buf_t lib_version;
    char lib_ver_str[128];
    lib_version.memory_size = 128;
    lib_version.string_size = 0;
    lib_version.p_string = (char *)lib_ver_str;
    the_lib.arsi_get_lib_version(&lib_version);
    AUD_LOG_D("lib_version: \"%s\"\n\n", lib_version.p_string);

    /* init lib */
    param_buf.memory_size = MAX_PARAM_SIZE;
    param_buf.data_size = 0;
    param_buf.p_buffer = (void *)dram_param_buffer;

    InitArsiLibConfig();
    InitArsiTaskConfig();

#ifdef MTK_SMARTPA_CACHE_SUPPORT
    working_buf.memory_size = WORKING_BUFFER_SIZE;
    working_buf.p_buffer = (void *)(lib_working_buf);
    memset(working_buf.p_buffer, 0, WORKING_BUFFER_SIZE);
#else
    the_lib.arsi_query_working_buf_size(&arsi_task_config, &arsi_lib_config, &working_buf.memory_size, myprint);
    working_buf.p_buffer = kal_pvPortMalloc(working_buf.memory_size);
    memset(working_buf.p_buffer, 0, working_buf.memory_size);
#endif
    tuning_key_value = (void *)tuning_buffer;
    memset(tuning_key_value, '\0', MAX_PARAM_SIZE);
    memset(param_buf.p_buffer, 0, MAX_PARAM_SIZE);
    spk_flush_cache((uint32_t)tuning_key_value, MAX_PARAM_SIZE);
    spk_flush_cache((uint32_t)param_buf.p_buffer, MAX_PARAM_SIZE);
    the_lib.arsi_create_handler(&arsi_task_config, &arsi_lib_config, &param_buf, &working_buf, &smartpa_arsi_handler, myprint);
    the_lib.arsi_set_debug_log_fp(myprint, smartpa_arsi_handler);

    // allocate memory
    dl_buf_in.data_buf.memory_size = PROCESSINGFRAMEBYTES;
    dl_buf_in.data_buf.data_size = 0;
    dl_buf_in.audio_format = AUDIO_FORMAT_PCM_32_BIT;
    dl_buf_in.num_channels = 2;

    dl_buf_out.data_buf.memory_size = PROCESSINGFRAMEBYTES;
    dl_buf_out.data_buf.data_size = 0;
    dl_buf_out.audio_format = AUDIO_FORMAT_PCM_32_BIT;
    dl_buf_out.num_channels = 2;

    dl_buf_iv.data_buf.memory_size = PROCESSINGFRAMEBYTES;
    dl_buf_iv.data_buf.data_size = 0;
    dl_buf_iv.audio_format = AUDIO_FORMAT_PCM_32_BIT;
    dl_buf_iv.num_channels = 2;
    dl_buf_iv.data_buf_type = DATA_BUF_IV_BUFFER;

    dl_buf_in.data_buf.p_buffer = (void *)task_working_buf_d1_in;
    dl_buf_out.data_buf.p_buffer = (void *)task_working_buf_d1_out;
    dl_buf_iv.data_buf.p_buffer = (void *)task_working_buf_d1_iv;

    memset(dl_buf_in.data_buf.p_buffer, 0, dl_buf_in.data_buf.memory_size);
    memset(dl_buf_out.data_buf.p_buffer, 0, dl_buf_out.data_buf.memory_size);
    memset(dl_buf_iv.data_buf.p_buffer, 0, dl_buf_iv.data_buf.memory_size);
    memset(&uSpeaker_Protection_Info, 0, sizeof(uSpeaker_Protection_Info));
    spk_flush_cache((uint32_t)dl_buf_in.data_buf.p_buffer,
                    dl_buf_in.data_buf.memory_size);
    spk_flush_cache((uint32_t)dl_buf_out.data_buf.p_buffer,
                    dl_buf_out.data_buf.memory_size);
    spk_flush_cache((uint32_t)dl_buf_iv.data_buf.p_buffer,
                    dl_buf_iv.data_buf.memory_size);

    AUD_LOG_D("dl_buf_in size: %u, dl_buf_out size: %u, dl_buf_iv size: %u, working_buf size: %u, msg queue size: %u, bypass_lib_flag: %d",
              dl_buf_in.data_buf.memory_size, dl_buf_out.data_buf.memory_size,
              dl_buf_iv.data_buf.memory_size, working_buf.memory_size,
              MAX_MSG_QUEUE_SIZE * ipi_msg_size, bypass_lib_flag);

    AUD_LOG_D("-%s, Heap free = %d",
              SPK_TASK_LOG_NAME, xPortGetFreeHeapSize());

    dump_pcm_flag = 0;
}

static void task_speaker_protection_destructor(struct AudioTask *this)
{
    AUD_LOG_D("+%s destruct, Heap free = %d, scene = %d\n",
              SPK_TASK_LOG_NAME, xPortGetFreeHeapSize(), this->scene);
    AUD_ASSERT(this != NULL);
    if (g_msg_array != NULL) {
        kal_vPortFree(g_msg_array);
    }
    if (dl_buf_in.data_buf.p_buffer != NULL) {
        dl_buf_in.data_buf.memory_size = 0;
        dl_buf_in.data_buf.p_buffer = NULL;
    }
    if (dl_buf_out.data_buf.p_buffer != NULL) {
        dl_buf_out.data_buf.memory_size = 0;
        dl_buf_out.data_buf.p_buffer = NULL;
    }
    if (dl_buf_iv.data_buf.p_buffer != NULL) {
        dl_buf_iv.data_buf.memory_size = 0;
        dl_buf_iv.data_buf.p_buffer = NULL;
    }

    if (param_buf.p_buffer != NULL) {
        param_buf.memory_size = 0;
        param_buf.p_buffer = NULL;
    }
    the_lib.arsi_destroy_handler(smartpa_arsi_handler);
    smartpa_arsi_handler = NULL;

    if (working_buf.p_buffer != NULL) {
#ifndef MTK_SMARTPA_CACHE_SUPPORT
        kal_vPortFree(working_buf.p_buffer);
#endif
        working_buf.memory_size = 0;
        working_buf.p_buffer = NULL;
    }

#ifdef SMARTPA_WAKELOCK
    wake_lock_deinit(&spk_task_monitor_wakelock);
#endif
    AUD_LOG_D("-%s destruct, Heap free = %d\n",
              SPK_TASK_LOG_NAME, xPortGetFreeHeapSize());
}

static void task_speaker_protection_create_task_loop(struct AudioTask *this)
{
    /* Note: you can also bypass this function,
             and do kal_xTaskCreate until you really need it.
             Ex: create task after you do get the enable message. */
    AUD_LOG_D("+%s() create, Heap free = %d\n",
              SPK_TASK_LOG_NAME, xPortGetFreeHeapSize());

    BaseType_t xReturn = pdFAIL;
    xReturn = kal_xTaskCreate(
                  task_speaker_protection_task_loop,
                  LOCAL_TASK_NAME,
                  LOCAL_TASK_STACK_SIZE,
                  (void *)this,
                  LOCAL_TASK_PRIORITY,
                  &this->freertos_task);

    AUD_LOG_D("-%s() create, Heap free = %d\n",
              SPK_TASK_LOG_NAME, xPortGetFreeHeapSize());
    AUD_ASSERT(xReturn == pdPASS);
}

SRAM_REGION_FUNCTION static status_t task_speaker_protection_recv_message(struct AudioTask *this,
                                                                          struct ipi_msg_t *ipi_msg)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    uint8_t i = 0;
    unsigned int queue_idx;
    ipi_msg_t *p_ipi_msg;
    this->num_queue_element++;
    queue_idx = get_queue_idx(this, MAX_MSG_QUEUE_SIZE);
    p_ipi_msg = GET_IPI_MSG(queue_idx);

    // check message
    if (p_ipi_msg->magic != 0) {
        ipi_msg_t *p_ipi_msg_error;
        uint64_t time_tick = timer_get_global_timer_tick();
        AUD_LOG_E("queue_idx: %d tick:%llu\n", queue_idx, time_tick);
        for (i = 0; i < MAX_MSG_QUEUE_SIZE; i++) {
            p_ipi_msg_error = GET_IPI_MSG(i);
            AUD_LOG_E("[%d] id = 0x%x\n", i, p_ipi_msg_error->msg_id);
        }
        AUD_ASSERT(p_ipi_msg->magic == 0); /* item is clean */
    }
    // copy to array
    memcpy(p_ipi_msg, ipi_msg, ipi_msg_size);
    //AUD_LOG_E("%s [%d] id = 0x%x\n", __FUNCTION__,i, p_ipi_msg->msg_id);

    if (xQueueSendToBackFromISR(this->msg_idx_queue, &queue_idx,
                                &xHigherPriorityTaskWoken) != pdTRUE) {
        return UNKNOWN_ERROR;
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    return NO_ERROR;
}

/* global for debug*/
uint32_t IRQSource;
SRAM_REGION_FUNCTION static void task_speaker_protection_irq_hanlder(struct AudioTask *this,
                                                                     uint32_t irq_type)
{
    if (irq_type == AUDIO_IRQn) {
        if (this->state != AUDIO_TASK_WORKING) {
            AUD_LOG_E("irq state err: %d", this->state);
            mask_irq(AUDIO_IRQn);
            NVIC_ClearPendingIRQ(AUDIO_IRQn);
            portYIELD_FROM_ISR(pdFALSE);
            return;
        }

        /* handler for last interrupt with irq status is all zero*/
        uint32_t IRQSource, IRQMask, irq_scp_en;
        const struct mtk_irq_info *irq_info = spk_irq_get();
        IRQSource = Afe_Get_Reg(AFE_IRQ_MCU_STATUS);
        IRQMask = IRQSource;
        irq_scp_en = Afe_Get_Reg(irq_info->scp_irq_reg);
        if ((IRQSource & AFE_IRQ_MASK) == 0 && (irq_scp_en & (irq_info->irq_shift_bit << SCP_IRQ_SHIFT_BIT)) == 0) {
            AUD_LOG_E("irq with state: %d IRQSource: %d irq_scp_en: 0x%x, irq num: %d ClearPendingIRQ",
                      this->state, IRQSource, irq_scp_en, irq_info->irq_mode);
            Afe_Set_Reg(irq_info->scp_irq_reg, irq_info->irq_shift_bit << SCP_IRQ_SHIFT_BIT,
                        irq_info->irq_shift_bit << SCP_IRQ_SHIFT_BIT);
            Afe_Set_Reg(AFE_IRQ_MCU_CLR, 0xffff, 0xffff);
            Afe_Set_Reg(irq_info->scp_irq_reg, 0, irq_info->irq_shift_bit << SCP_IRQ_SHIFT_BIT);
            NVIC_ClearPendingIRQ(AUDIO_IRQn);
            portYIELD_FROM_ISR(pdFALSE);
            return;
        }

        /* only clear IRQ which is sent to SCP */
        IRQMask &= irq_info->irq_shift_bit;
        Afe_Set_Reg(AFE_IRQ_MCU_CLR, IRQMask, IRQMask);

        /*
        AUD_LOG_V("irq normal case state = 0x%x IRQSource = 0x%x IRQMask = 0x%x\n",
        this->state,IRQSource,IRQMask);*/

        // check IRQ, DL InterruptSample
        if (IRQSource & (0x1 << irq_info->irq_mode)) {
            uint8_t i = 0;
            unsigned int queue_idx = 0;
            ipi_msg_t *ipi_msg = NULL;
            static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
            this->num_queue_element++;
            queue_idx = get_queue_idx(this, MAX_MSG_QUEUE_SIZE);
            ipi_msg = GET_IPI_MSG(queue_idx);
            if (ipi_msg->magic != 0) {
                ipi_msg_t *p_ipi_msg_error;
                uint64_t time_tick = timer_get_global_timer_tick();
                AUD_LOG_E("queue_idx: %d tick:%llu\n", queue_idx, time_tick);
                for (i = 0; i < MAX_MSG_QUEUE_SIZE; i++) {
                    p_ipi_msg_error = GET_IPI_MSG(i);
                    AUD_LOG_E("[%d] id = 0x%x\n", i, p_ipi_msg_error->msg_id);
                }
                AUD_ASSERT(ipi_msg->magic == 0); /* item is clean */
            }
            ipi_msg->magic    = IPI_MSG_MAGIC_NUMBER;
            ipi_msg->task_scene = TASK_SCENE_SPEAKER_PROTECTION;
            ipi_msg->source_layer  = AUDIO_IPI_LAYER_FROM_DSP;
            ipi_msg->target_layer  = AUDIO_IPI_LAYER_TO_KERNEL;
            ipi_msg->data_type  = AUDIO_IPI_MSG_ONLY;
            ipi_msg->ack_type   = AUDIO_IPI_MSG_BYPASS_ACK;
            if (uSpeaker_Protection_Info.mscenario == Speaker_protect_playback) {
                ipi_msg->msg_id     = SPK_PROTECT_IRQDL;
            }
            if (uSpeaker_Protection_Info.mscenario == Speaker_protect_speech) {
                ipi_msg->msg_id     = SPK_PROTECT_SPEECH_IRQDL;
            }
            ipi_msg->param1     = 0;
            ipi_msg->param2     = 0;
            if (xQueueSendToBackFromISR(this->msg_idx_queue, &queue_idx,
                                        &xHigherPriorityTaskWoken) != pdTRUE) {
                return;
            }
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        } else {

        }
    }
}


static void task_speaker_protection_hal_reboot_cbk(struct AudioTask *this, const uint8_t hal_type) {
    AUD_LOG_D("%s(), scene %d", __func__, this->scene);
}


AudioTask *task_speaker_protection_new(void)
{
    /* alloc object here */
    AudioTask *task = (AudioTask *)kal_pvPortMalloc(sizeof(AudioTask));
    if (task == NULL) {
        AUD_LOG_E("%s new, kal_pvPortMalloc fail!!", SPK_TASK_LOG_NAME);
        return NULL;
    }

    /* only assign methods, but not class members here */
    task->constructor       = task_speaker_protection_constructor;
    task->destructor        = task_speaker_protection_destructor;

    task->create_task_loop  = task_speaker_protection_create_task_loop;

    task->recv_message      = task_speaker_protection_recv_message;

    task->irq_hanlder       = task_speaker_protection_irq_hanlder;

    task->hal_reboot_cbk    = task_speaker_protection_hal_reboot_cbk;

    return task;
}

void task_speaker_protection_delete(AudioTask *task)
{
    AUD_LOG_V("%s delete, task_scene = %d",
              SPK_TASK_LOG_NAME, task->scene);

    if (task == NULL) {
        AUD_LOG_E("%s delete, task is NULL!!", SPK_TASK_LOG_NAME);
        return;
    }
    vPortFree(task);
}

static void myprint(const char *message, ...)
{
    static char printf_msg[256];

    va_list args;
    va_start(args, message);

    vsnprintf(printf_msg, sizeof(printf_msg), message, args);
    AUD_LOG_D("%s", printf_msg);

    va_end(args);
}

static void UpdateArsiTaskconfig()
{
    if (bypass_lib_flag) {
        return;
    }

    arsi_task_config.output_device_info.sample_rate = uSpeaker_Protection_Info.mdlsamplerate;
    arsi_lib_config.sample_rate = uSpeaker_Protection_Info.mdlsamplerate;
    arsi_lib_config.audio_format = uSpeaker_Protection_Info.mdlformat;
    arsi_lib_config.frame_size_ms = uSpeaker_Protection_Info.mdperiodsize * 1000 / uSpeaker_Protection_Info.mdlsamplerate;

    if (smartpa_arsi_handler != NULL) {
        if (the_lib.arsi_update_device(
                &arsi_task_config,
                &arsi_lib_config,
                &param_buf,
                smartpa_arsi_handler) != NO_ERROR) {
            AUD_LOG_E("arsi_update_device fail!!");
        }
        AUD_LOG_D("aurisys update, sample rate = %d, audio_format = %d, frame ms = %d",
                  arsi_lib_config.sample_rate,
                  arsi_lib_config.audio_format, arsi_lib_config.frame_size_ms);
    }
}

static void InitArsiTaskConfig(void)
{
    memset(&arsi_task_config, 0, sizeof(arsi_task_config));
    /* input device */
    arsi_task_config.input_device_info.devices = AUDIO_DEVICE_NONE;
    arsi_task_config.input_device_info.audio_format = AUDIO_FORMAT_DEFAULT;
    arsi_task_config.input_device_info.sample_rate = 0;
    arsi_task_config.input_device_info.channel_mask = AUDIO_CHANNEL_NONE;
    arsi_task_config.input_device_info.num_channels = 0;
    arsi_task_config.input_device_info.hw_info_mask = 0;

    /* output device */
    arsi_task_config.output_device_info.devices = AUDIO_DEVICE_OUT_SPEAKER;
    arsi_task_config.output_device_info.audio_format = AUDIO_FORMAT_PCM_32_BIT;
    arsi_task_config.output_device_info.sample_rate = 48000;
    arsi_task_config.output_device_info.channel_mask = AUDIO_CHANNEL_NONE;
    arsi_task_config.output_device_info.num_channels = 2;
    arsi_task_config.output_device_info.hw_info_mask = 0;

    /* task scene */
    arsi_task_config.task_scene = TASK_SCENE_SPEAKER_PROTECTION;

    /* audio mode */
    arsi_task_config.audio_mode = AUDIO_MODE_RINGTONE;

    /* max device capability for allocating memory */
    arsi_task_config.max_input_device_sample_rate  = 48000; /* TODO */
    arsi_task_config.max_output_device_sample_rate = 48000; /* TODO */

    arsi_task_config.max_input_device_num_channels = 0;
    arsi_task_config.max_output_device_num_channels = 2; /* TODO */

    /* max device capability for allocating memory */
    arsi_task_config.input_source = AUDIO_SOURCE_VOICE_DOWNLINK ;
}

static void InitArsiLibConfig(void)
{
    memset(&arsi_lib_config, 0, sizeof(arsi_lib_config));
    arsi_lib_config.sample_rate = 48000;
    arsi_lib_config.audio_format = AUDIO_FORMAT_PCM_32_BIT;
    arsi_lib_config.frame_size_ms = 32;
    arsi_lib_config.b_interleave = 1;

    arsi_lib_config.p_dl_buf_in = &dl_buf_in;
    arsi_lib_config.p_dl_buf_out = &dl_buf_out;
    arsi_lib_config.p_dl_ref_bufs = &dl_buf_iv;

    arsi_lib_config.num_dl_ref_buf_array = 1;
}


static void ProcessTime(uint8_t event, uint16_t msg_id,
                        uint8_t num_queue_element, bool is_start_stage)
{
#ifdef PROCESS_TIME
    char scene_str[16] = {'\0'};

    if (uSpeaker_Protection_Info.mscenario == Speaker_protect_playback) {
        strncpy(scene_str, "Playback", 8);
    } else if (uSpeaker_Protection_Info.mscenario == Speaker_protect_speech) {
        strncpy(scene_str, "Phonecall", 9);
    } else {
        AUD_LOG_E("wrong scene in measure time, return");
        return;
    }

    if (event == SCP_PROCESS_LIB) {
        if (is_start_stage) {
            lib_start_time_ns = timer_get_global_timer_tick();
            return;
        }

        lib_stop_time_ns = timer_get_global_timer_tick();
        lib_time_diff_ns = GET_TIME_INTERVAL(lib_stop_time_ns, lib_start_time_ns);
        if (lib_time_diff_ns > lib_max_process_time_ns) {
            lib_max_process_time_ns = lib_time_diff_ns;
            AUD_LOG_D("%s, lib max time: %llu, queue element num: %d",
                      scene_str, TIME_NS_TO_US(lib_max_process_time_ns),
                      num_queue_element);
        }
        lib_avg_count++;
        lib_total_process_time += lib_time_diff_ns;
        if (lib_avg_count == AVG_NUMBER) {
            lib_avg_process_time = lib_total_process_time / AVG_NUMBER;
            AUD_LOG_D("%s, lib avg time: %llu usec, queue element num: %d",
                      scene_str, TIME_NS_TO_US(lib_avg_process_time), num_queue_element);
            lib_total_process_time = 0;
            lib_avg_count = 0;
        }
    } else if (event == SCP_PROCESS_TASK) {
        switch (msg_id) {
        case SPK_PROTECT_DLCOPY:
            task_time_idx = 0;
            break;
        case SPK_PROTECT_SPEECH_IRQDL:
            task_time_idx = 1;
            break;
        default:
            return;
        }

        if (is_start_stage) {
            task_start_time_ns[task_time_idx] = timer_get_global_timer_tick();
            return;
        }

        task_stop_time_ns[task_time_idx] = timer_get_global_timer_tick();
        task_time_diff_ns[task_time_idx] = GET_TIME_INTERVAL(task_stop_time_ns[task_time_idx],
                                                             task_start_time_ns[task_time_idx]);

        if (task_time_diff_ns[task_time_idx] > task_max_process_time_ns[task_time_idx]) {
            task_max_process_time_ns[task_time_idx] = task_time_diff_ns[task_time_idx];
            AUD_LOG_D("%s, task max time: %llu usec, queue num: %d, task_time_idx: %d",
                      scene_str, TIME_NS_TO_US(task_max_process_time_ns[task_time_idx]),
                      num_queue_element, task_time_idx);
        }
        task_avg_count[task_time_idx]++;
        task_total_process_time[task_time_idx] += task_time_diff_ns[task_time_idx];
        if (task_avg_count[task_time_idx] == AVG_NUMBER) {
            task_avg_process_time[task_time_idx] = task_total_process_time[task_time_idx] / AVG_NUMBER;
            AUD_LOG_D("%s, task avg time: %llu usec, queue num: %d, task_time_idx: %d",
                      scene_str, TIME_NS_TO_US(task_avg_process_time[task_time_idx]), num_queue_element, task_time_idx);
            task_total_process_time[task_time_idx] = 0;
            task_avg_count[task_time_idx] = 0;
        }
    } else {
        AUD_LOG_D("wrong event(lib/task) in measure time");
    }
#endif
}


