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
#ifndef AUDIO_TASK_A2DP_H
#define AUDIO_TASK_A2DP_H

#include "audio_task_interface.h"
#include "timers.h"

#define A2DPBUF_MAX_ELEMENT (20)
#define A2DPBUF_MIN_ELEMENT (8)
//#define A2DP_TIMER_START_THRESHOLD (A2DPBUF_MAX_ELEMENT>>1)

#define A2DP_EENCODED_DATA_SIZE (2048)
#define SBC_MAX_FRAME_LENGTH (119)


typedef struct {
    void *pBufAddr;
    int u4BufferSize;
    char *pWriteIdx;
    char *pReadIdx;
    int u4DataRemained;
    int consumedBS;
} BS_BUFFER_T;

typedef struct {
    uint32_t codec_type; // codec types ex: SBC/AAC/LDAC/APTx
    uint32_t sample_rate;    // sample rate, ex: 44.1/48.88.2/96 KHz
    uint32_t encoded_audio_bitrate;  // encoder audio bitrates
    uint16_t max_latency;    // maximum latency
    uint16_t scms_t_enable;  // content protection enable
    uint16_t acl_hdl;    // connection handle
    uint16_t l2c_rcid;   // l2cap channel id
    uint16_t mtu;        // mtu size
    unsigned char bits_per_sample; // bits per sample, ex: 16/24/32
    unsigned char ch_mode;         // None:0, Left:1, Right:2
    unsigned char codec_info[32];  //codec specific information
} FORMAT_INFO_T;

typedef struct {
    uint32_t frame_num;
    uint32_t frame_length;
    uint32_t encoded_data_length;
    uint32_t mtu_num;
    uint8_t reserved[112]; // for cache 128 byte align
    uint8_t encoded_data[A2DP_EENCODED_DATA_SIZE];
} A2DPBUF_DATA_T;

typedef struct {
    unsigned char r_index; // 0 ~ 7
    unsigned char w_index; // 0 ~ 7
    unsigned char count; // 1 ~ 8
} A2DPBUF_INDEX_T;

typedef struct {
    unsigned int input_length;
    unsigned int output_length;
    unsigned int frame_num;
    unsigned int frame_length;
} ENCODE_INFO_T;

/* for Codec Specific Information Element */
#define A2DP_SBC_IE_SAMP_FREQ_MSK 0xF0 /* b7-b4 sampling frequency */
#define A2DP_SBC_IE_SAMP_FREQ_16 0x80  /* b7:16  kHz */
#define A2DP_SBC_IE_SAMP_FREQ_32 0x40  /* b6:32  kHz */
#define A2DP_SBC_IE_SAMP_FREQ_44 0x20  /* b5:44.1kHz */
#define A2DP_SBC_IE_SAMP_FREQ_48 0x10  /* b4:48  kHz */

#define A2DP_SBC_IE_CH_MD_MSK 0x0F    /* b3-b0 channel mode */
#define A2DP_SBC_IE_CH_MD_MONO 0x08   /* b3: mono */
#define A2DP_SBC_IE_CH_MD_DUAL 0x04   /* b2: dual */
#define A2DP_SBC_IE_CH_MD_STEREO 0x02 /* b1: stereo */
#define A2DP_SBC_IE_CH_MD_JOINT 0x01  /* b0: joint stereo */

#define A2DP_SBC_IE_BLOCKS_MSK 0xF0 /* b7-b4 number of blocks */
#define A2DP_SBC_IE_BLOCKS_4 0x80   /* 4 blocks */
#define A2DP_SBC_IE_BLOCKS_8 0x40   /* 8 blocks */
#define A2DP_SBC_IE_BLOCKS_12 0x20  /* 12blocks */
#define A2DP_SBC_IE_BLOCKS_16 0x10  /* 16blocks */

#define A2DP_SBC_IE_SUBBAND_MSK 0x0C /* b3-b2 number of subbands */
#define A2DP_SBC_IE_SUBBAND_4 0x08   /* b3: 4 */
#define A2DP_SBC_IE_SUBBAND_8 0x04   /* b2: 8 */

#define A2DP_SBC_IE_ALLOC_MD_MSK 0x03 /* b1-b0 allocation mode */
#define A2DP_SBC_IE_ALLOC_MD_S 0x02   /* b1: SNR */
#define A2DP_SBC_IE_ALLOC_MD_L 0x01   /* b0: loundess */

#define A2DP_SBC_IE_MIN_BITPOOL 2
#define A2DP_SBC_IE_MAX_BITPOOL 250

#define A2DP_CODEC_TYPE_SBC 0x01
#define A2DP_CODEC_TYPE_AAC 0x02
#define A2DP_CODEC_TYPE_APTX 0x04
#define A2DP_CODEC_TYPE_APTX_HD 0x08
#define A2DP_CODEC_LDAC 0x10

#define A2DP_CODEC_SAMP_FREQ_44 0x01
#define A2DP_CODEC_SAMP_FREQ_48 0x02
#define A2DP_CODEC_SAMP_FREQ_88 0x04
#define A2DP_CODEC_SAMP_FREQ_96 0x08
#define A2DP_CODEC_SAMP_FREQ_176 0x10
#define A2DP_CODEC_SAMP_FREQ_192 0x20

#define A2DP_CODEC_SAMP_BIT_16 0x01
#define A2DP_CODEC_SAMP_BIT_24 0x02
#define A2DP_CODEC_SAMP_BIT_32 0x04

#define A2DP_CODEC_CH_MD_MONO 0x01
#define A2DP_CODEC_CH_MD_STEREO 0x02

struct audio_a2dp_struct {
    struct RingBuf *pcm_ring_buf;
    SemaphoreHandle_t xPCMRingBufSemphr;
    SemaphoreHandle_t xPCMRingBufSemphr2;
    SemaphoreHandle_t xA2DPSemphrTX;
    bool bt_first_wakeup;
};

void task_a2dp_init(void);
void task_a2dp_deinit(void);
void task_a2dp_encode_init(void);
void task_a2dp_encode_deinit(void);
unsigned int task_a2dp_encode_process(unsigned int input_length);
int task_a2dp_get_encode_input_length(void);
void task_a2dp_timer_start(void);
void task_a2dp_timer_stop(void);
void task_a2dp_write_process(void);

void a2dp_structure_init(void);
void a2dp_structure_deinit(void);
void *get_a2dp_structure(void);
void task_copyfrom_pcmringbuf(unsigned int length, char *buf);
void task_a2dp_set_codecinfo(uint32_t size_info, void *info, uint32_t sample_rate);

AudioTask *task_a2dp_new(void);
void task_a2dp_delete(AudioTask *task);

/* task relate variable.*/

#endif  // end of AUDIO_TASK_A2DP_H

