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
#ifndef AUDIO_TASK_UTILITY_H
#define AUDIO_TASK_UTILITY_H

#include <stdio.h>
#include <audio_type.h>

#ifdef CFG_MTK_AUDIODSP_SUPPORT
#include <hal_cache.h>
#endif

#define GET_TIME_INTERVAL(stop_time_ns, start_time_ns) \
    (((stop_time_ns) > (start_time_ns)) \
     ? (((stop_time_ns) - (start_time_ns))*77) \
     : (((0xFFFFFFFFFFFFFFFF - (start_time_ns)) + (stop_time_ns) + 1))*77)

#define TIME_NS_TO_US(time_ns) (((uint64_t)(time_ns)) / 1000)
#define TIME_NS_TO_MS(time_ns) (((uint64_t)(time_ns)) / 1000000)

#define TIME_US_TO_MS(time_ms) (((uint64_t)(time_ms)) / 1000)
#define TIME_MS_TO_US(time_ms) (((uint64_t)(time_ms)) * 1000)


enum {
    TASK_PRIMARY_TIME_ID1 = 0,
    TASK_PRIMARY_TIME_ID2,
    TASK_AUDPLAYUBACK_TIME_ID1,
    TASK_AUDPLAYUBACK_TIME_ID2,
    TASK_DEEPBUFFER_TIME_ID1,
    TASK_DEEPBUFFER_TIME_ID2,
    TASK_VOIP_TIME_ID1,
    TASK_VOIP_TIME_ID2,
    TASK_CAPTURE_UL1_TIME_ID1,
    TASK_CAPTURE_UL1_TIME_ID2,
    TASK_A2DP_TIME_ID1,
    TASK_A2DP_TIME_ID2,
    TASK_DATAPROVIDER_TIME_ID1,
    TASK_DATAPROVIDER_TIME_ID2,
    TASK_CALL_FINAL_TIME_ID1,
    TASK_KTV_TIME_ID1,
    TASK_KTV_TIME_ID2,
    NUM_OF_TASK_TIME_ID,
};

enum {
    TASK_DEBUG_ERROR,
    TASK_DEBUG_WARNING,
    TASK_DEBUG_INFO,
    TASK_DEBUG_DEBUG,
    TASK_DEBUG_VERBOSE
};

struct AudioTask;
struct Audio_Task_Msg_t;
struct audio_hw_buffer;
struct stream_attribute_dsp;
struct buf_attr;

/* utility function */
int aud_task_check_state(struct AudioTask *task, unsigned int state);
unsigned int frame_to_bytes(unsigned int framecount, unsigned int channels,
                            unsigned int format);
void aud_task_delay(int delayms);
unsigned int get_peroid_mstime(unsigned int rate, unsigned int period);

/* time relate */
int init_time_interval(int id, unsigned long long thresholdns);
int record_time_interval(int id);
int stop_time_interval(int id);
unsigned long long get_time_stamp(void);

unsigned int getframesize(unsigned int channel, unsigned int format);

/* get audio hw buffer API */
int get_audiobuf_from_msg(struct Audio_Task_Msg_t *AudioTaskMsg,
                          struct audio_hw_buffer *task_audio_buf);
int get_audioattr_from_msg(struct Audio_Task_Msg_t *AudioTaskMsg,
                           struct buf_attr *task_audio_attr);
unsigned int get_audio_irq_size(struct audio_hw_buffer *audio_hwbuf);

int get_audio_loop_count(struct audio_hw_buffer *audio_hwbuf,
                         int process_size,
                         int* count);

/* debug */
void DumpTaskMsg(char *appendstring, struct Audio_Task_Msg_t *Msg);


/* enter point of irq handler*/
int adsp_audio_irq_entry(void);

/* condition enter write loop */
int enter_write_cond(int handle,
                     struct AudioTask *task,
                     struct audio_hw_buffer *audio_hwbuf,
                     struct audio_hw_buffer *audio_afebuf,
                     struct RingBuf *pRingBuf,
                     unsigned int written_size);


int enter_read_cond(int handle,
                     struct AudioTask *task,
                     struct audio_hw_buffer *audio_hwbuf,
                     struct audio_hw_buffer *audio_afebuf,
                     struct RingBuf *pRingBuf,
                     unsigned int processing_in_size,
                     unsigned int processing_out_size);

/* condition enter first write */
int first_write_cond(int handle,
                     struct AudioTask *task,
                     struct audio_hw_buffer *audio_hwbuf,
                     struct audio_hw_buffer *audio_afebuf,
                     struct RingBuf *pRingBuf,
                     unsigned int written_size);

/* *send message to task */
unsigned int get_queue_idx(struct AudioTask *this,
                           unsigned int max_queue_size);
int send_task_message(struct AudioTask *this, struct ipi_msg_t *ipi_msg,
                      unsigned int max_queue_size);
int send_task_message_fromisr(struct AudioTask *this, struct ipi_msg_t *ipi_msg,
                              unsigned int max_queue_size);

int task_do_processing(char *inputbuffer, int inputbuffersize,
                       char *outputbuffer, int *outputbuffersize,
                       char *referencebuffer, int referencebuffersize);

#ifdef MTK_AURISYS_FRAMEWORK_SUPPORT
void cal_output_buffer_size(struct stream_attribute_dsp *attr_in,
                            struct stream_attribute_dsp *attr_out,
                            uint32_t size_in,
                            uint32_t *size_out);

void cal_buffer_size(struct stream_attribute_dsp *attr, int *size);
#endif

bool task_is_dl(int task_scene);
bool task_is_ul(int task_scene);

#endif // end of AUDIO_TASK_UTILITY_H
