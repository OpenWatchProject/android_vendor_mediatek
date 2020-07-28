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
#ifndef AUDIO_TASK_OFFLOAD_MP3_PARAMS_H
#define AUDIO_TASK_OFFLOAD_MP3_PARAMS_H
//#include "mp3dec_common.h"
#include "mp3dec_exp.h"
#include "heaacdec_exp.h"
#include "blisrc_exp.h"
#include "audio_ringbuf.h"


#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

typedef enum {
    OFFLOAD_NEEDDATA = 21,
    OFFLOAD_PCMCONSUMED = 22,
    OFFLOAD_DRAINDONE = 23,
    OFFLOAD_PCMDUMP_OK = 24,
    OFFLOAD_DECODE_ERROR = 25,
} IPI_MSG_ID;

typedef enum {
    OFFLOAD_RESUME = 0x300,
    OFFLOAD_PAUSE,
    OFFLOAD_SETWRITEBLOCK,
    OFFLOAD_DRAIN,
    OFFLOAD_VOLUME,
    OFFLOAD_WRITEIDX,
    OFFLOAD_TSTAMP,
    OFFLOAD_PCMDUMP_ON,
    OFFLOAD_SCENE,
    OFFLOAD_CODEC_INFO,
} IPI_RECEIVED_OFFLOAD;

typedef enum {
    MP3_DECODE_DUMP = 1,
    MP3_AUDIOSRAM_DUMP = 2,
    /* MP3_BLISRC_DUMP = 3,
     MP3_VOLUME_DUMP = 4,
     MP3_BITCONVERT_DUMP = 5,*/
} DEBUG_DUMP_OFFLOAD;
typedef struct {
    uint32_t samplerate;
    uint32_t channels;
    uint32_t format;
    uint32_t volume[2];
    uint32_t u4DataRemained;
    uint8_t bIsDrain;
    uint8_t bIsPause;
    uint8_t bIsClose;
    uint8_t bgetTime;
} SCP_OFFLOAD_T;

typedef struct {
    void *pBufAddr;
    int u4BufferSize;
    char *pWriteIdx;//  Write Index.
    char *pReadIdx;   //  Read Index,update by scp
    int u4DataRemained;
    int consumedBS;
    unsigned char   bneedFill;
} DMA_BUFFER_T;


typedef struct {
    Mp3dec_handle *handle;
    int workingbuf_size1;
    int workingbuf_size2;
    int min_bs_size;
    int pcm_size;
    void *working_buf1;
    void *working_buf2;
    void *pcm_buf;
} mp3DecEngine;

typedef struct {
    HEAACDEC_HANDLE*handle;
    int workingbuf_size1;
    int workingbuf_size2;
    int sbrbuf_size;
    int min_bs_size;
    int pcm_size;
    void *working_buf1;
    void *working_buf2;
    void *sbr_buf;
    void *pcm_buf;
    SBR_SIGNALING sbrSignaling;
    SBR_MODE sbrMode;
    int fForce2Stereo;
} aacDecEngine;

typedef struct {

    Blisrc_Handle *mHandle;
    Blisrc_Param mBliParam;
    unsigned int  mInternalBufSize;
    unsigned int  mTempBufSize;
    void *mInternalBuf;
    void *mTempBuf;
} blisrcEngine;

typedef struct {
    unsigned int codec_samplerate;
    unsigned int codec_bitrate;
}offload_codec_t ;

int mp3_dma_transaction_wrap(uint32_t dst_addr, uint32_t src_addr,
                             uint32_t len, uint8_t IsDram);
void mp3_RingBuf_copyFromLinear_dma(struct RingBuf *RingBuf1, const char *buf,
                                uint32_t count, const char bDramPlayback);

#endif // end of AUDIO_TASK_OFFLOAD_MP3_PARAMS_H

