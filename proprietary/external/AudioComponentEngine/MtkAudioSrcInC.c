/* MediaTek Inc. (C) 2016. All rights reserved.
 *
 * Copyright Statement:
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 */

#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG  "MtkAudioSrcInC"

#include <sys/ioctl.h>
#include <utils/Log.h>
#include <assert.h>
#include "MtkAudioSrcInC.h"
#ifdef FLT_PROFILING
#include <sys/time.h>
#endif

#define HAVE_SWIP

static int SrcOpen(struct MtkAudioSrcInC *self, uint32_t input_SR, uint32_t input_channel_num, uint32_t output_SR, uint32_t output_channel_num, SRC_PCM_FORMAT format) {
    ALOGV("+%s()self->mState = %d", __FUNCTION__, self->mState);

    if (self->mState != ACE_STATE_INIT) {
        return ACE_INVALIDE_OPERATION;
    }

    int ret;
    self->mBlisrcParam.PCM_Format = (uint32_t)format;
    self->mBlisrcParam.in_sampling_rate = input_SR;
    self->mBlisrcParam.ou_sampling_rate = output_SR;
    self->mBlisrcParam.in_channel = input_channel_num;
    self->mBlisrcParam.ou_channel = output_channel_num;

#if defined(HAVE_SWIP)
    ret = Blisrc_GetBufferSize(&self->mInternalBufSize,
                               &self->mTempBufSize,
                               &self->mBlisrcParam);

    if (ret < 0) {
        ALOGD("Blisrc_GetBufferSize return err %d\n", ret);
        return ACE_INVALIDE_OPERATION;
    }
#endif
    if (self->mInternalBufSize > 0) {
        self->mpInternalBuf = (char *)malloc(self->mInternalBufSize);
    }
    if (self->mTempBufSize > 0) {
        self->mpTempBuf = (char *)malloc(self->mTempBufSize);
    }
#if defined(HAVE_SWIP)
    Blisrc_Open(&self->mBlisrcHandler, self->mpInternalBuf, &self->mBlisrcParam);
#endif
    self->mState = ACE_STATE_OPEN;
    ALOGV("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;
}


static int SrcClose(struct MtkAudioSrcInC *self) {
    ALOGV("+%s ()self->mState = %d", __FUNCTION__, self->mState);

    if (self->mState != ACE_STATE_OPEN) {
        return ACE_INVALIDE_OPERATION;
    }

    if (self->mpTempBuf != NULL) {
        free(self->mpTempBuf);
        self->mpTempBuf = NULL;
    }
    if (self->mpInternalBuf != NULL) {
        free(self->mpInternalBuf);
        self->mpInternalBuf = NULL;
    }
    self->mState = ACE_STATE_INIT;
    ALOGV("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;
}

static int SrcSetParameter(struct MtkAudioSrcInC *self, uint32_t paramID, void *param) {
    ALOGD("+%s(), paramID %d, param 0x%x\n", __FUNCTION__, paramID, (uint32_t)((long)param));
    uint32_t Curparam = (uint32_t)((long)param);
    if (self->mState == ACE_STATE_OPEN && Curparam != SRC_PAR_SET_INPUT_SAMPLE_RATE) { //Only input sampling rate could be update during process.
        return ACE_INVALIDE_OPERATION;
    }
    switch (paramID) {
    case SRC_PAR_SET_PCM_FORMAT: {
        if (Curparam >= SRC_IN_END) {
            return ACE_INVALIDE_PARAMETER;
        }
        self->mBlisrcParam.PCM_Format = Curparam;
        break;
    }
    case SRC_PAR_SET_INPUT_SAMPLE_RATE: {
        self->mBlisrcParam.in_sampling_rate = Curparam;
        break;
    }
    case SRC_PAR_SET_OUTPUT_SAMPLE_RATE: {
        self->mBlisrcParam.ou_sampling_rate = Curparam;
        if (self->mState == ACE_STATE_OPEN) {
#if defined(HAVE_SWIP)
            Blisrc_SetSamplingRate(self->mBlisrcHandler, self->mBlisrcParam.ou_sampling_rate);
#endif
        }
        break;
    }
    case SRC_PAR_SET_INPUT_CHANNEL_NUMBER: {
        self->mBlisrcParam.in_channel = Curparam;
        break;
    }
    case SRC_PAR_SET_OUTPUT_CHANNEL_NUMBER: {
        self->mBlisrcParam.ou_channel = Curparam;
        break;
    }
    default:
        return ACE_INVALIDE_PARAMETER;
    }
    ALOGD("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;
}

static int SrcGetParameter(struct MtkAudioSrcInC *self, uint32_t paramID, void *param) {
    ALOGD("+%s(), paramID %d\n", __FUNCTION__, paramID);
    long Curparam = 0;
    switch (paramID) {
    case SRC_PAR_GET_PCM_FORMAT: {
        Curparam = (long)self->mBlisrcParam.PCM_Format;
        break;
    }
    case SRC_PAR_GET_INPUT_SAMPLE_RATE: {
        Curparam = (long)self->mBlisrcParam.in_sampling_rate;
        break;
    }
    case SRC_PAR_GET_OUTPUT_SAMPLE_RATE: {
        Curparam = (long)self->mBlisrcParam.ou_sampling_rate;
        break;
    }
    case SRC_PAR_GET_INPUT_CHANNEL_NUMBER: {
        Curparam = (long)self->mBlisrcParam.in_channel;
        break;
    }
    case SRC_PAR_GET_OUTPUT_CHANNEL_NUMBER: {
        Curparam = (long)self->mBlisrcParam.ou_channel;
        break;
    }
    default:
        return ACE_INVALIDE_PARAMETER;
    }
    param = (void *) Curparam;
    ALOGD("-%s(), paramID %d, param 0x%x\n", __FUNCTION__, paramID, (uint32_t)((long)param));
    return ACE_SUCCESS;
}

static int SrcReset(struct MtkAudioSrcInC *self) {
    ALOGD("+%s()\n", __FUNCTION__);
    if (self->mState != ACE_STATE_OPEN) {
        return ACE_INVALIDE_OPERATION;
    }
#if defined(HAVE_SWIP)
    Blisrc_Reset(self->mBlisrcHandler);
#endif
    ALOGD("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;
}

static int SrcProcess(struct MtkAudioSrcInC *self,
                      void *pInputBuffer,   /* Input, pointer to input buffer */
                      uint32_t *InputSampleCount,        /* Input, length(byte) of input buffer */
                      /* Output, length(byte) left in the input buffer after conversion */
                      void *pOutputBuffer,                        /* Input, pointer to output buffer */
                      uint32_t *OutputSampleCount) {
    ALOGV("+%s(), inputCnt %d, outputCnt %x\n", __FUNCTION__, *InputSampleCount, *OutputSampleCount);
    if (self->mState != ACE_STATE_OPEN) {
        return ACE_INVALIDE_OPERATION;
    }

    char *tmp = NULL;

    uint32_t bit_ratio = (self->mBlisrcParam.PCM_Format == SRC_IN_Q1P15_OUT_Q1P31) ? 2 : 1;

    uint32_t raw_count = *InputSampleCount;
    uint32_t src_count = (raw_count * bit_ratio * self->mBlisrcParam.ou_sampling_rate * self->mBlisrcParam.ou_channel) /
                         (self->mBlisrcParam.in_sampling_rate * self->mBlisrcParam.in_channel);

    uint32_t size_per_frame = 0;
    if (self->mBlisrcParam.PCM_Format == SRC_IN_Q1P15_OUT_Q1P15) {
        size_per_frame = self->mBlisrcParam.ou_channel * 2;
    }
    else if (self->mBlisrcParam.PCM_Format == SRC_IN_Q1P15_OUT_Q1P31 ||
             self->mBlisrcParam.PCM_Format == SRC_IN_Q9P23_OUT_Q1P31 ||
             self->mBlisrcParam.PCM_Format == SRC_IN_Q1P31_OUT_Q1P31) {
        size_per_frame = self->mBlisrcParam.ou_channel * 4;
    }
    else {
        ALOGE("unknown format %u!!", self->mBlisrcParam.PCM_Format);
        size_per_frame = 1;
    }

    if ((src_count % size_per_frame) != 0) { // alignment
        src_count = ((src_count / size_per_frame) + 1) * size_per_frame;
    }

    if (src_count > *OutputSampleCount) {
        ALOGE("OutputSampleCount %u < %u!!", *OutputSampleCount, src_count);
        src_count = *OutputSampleCount;
    }



#if defined(HAVE_SWIP)
    Blisrc_Process(self->mBlisrcHandler,
                   self->mpTempBuf,
                   pInputBuffer,
                   InputSampleCount,
                   pOutputBuffer,
                   OutputSampleCount);
#endif

    if (*OutputSampleCount != src_count) {
        if (self->compensated == 1) {
            //ALOGW("OutputSampleCount %u != %u", *OutputSampleCount, src_count);
        }
        else {
            ALOGD("OutputSampleCount %u => %u", *OutputSampleCount, src_count);
            if (src_count > *OutputSampleCount) {
                tmp = (char *)malloc(src_count);
                memset(tmp, 0, src_count);

                memcpy(tmp + (src_count - *OutputSampleCount),
                       pOutputBuffer,
                       *OutputSampleCount);
                memcpy(pOutputBuffer, tmp, src_count);
                *OutputSampleCount = src_count;

                free(tmp);
                tmp = NULL;
            }
        }
    }
    self->compensated = 1;

    ALOGV("-%s(), inputCnt %d, outputCnt %x\n", __FUNCTION__, *InputSampleCount, *OutputSampleCount);
    return ACE_SUCCESS;
}

static int MultiChannel_Open(struct MtkAudioSrcInC *self, uint32_t input_SR, uint32_t input_channel_num, uint32_t output_SR, uint32_t output_channel_num, SRC_PCM_FORMAT format) {
    ALOGD("+%s()\n", __FUNCTION__);
    if (self->mState != ACE_STATE_INIT) {
        return ACE_INVALIDE_OPERATION;
    }
    if (input_channel_num != output_channel_num) {
        ALOGE("in ch %u != out ch %u", input_channel_num, output_channel_num);
        return ACE_INVALIDE_OPERATION;
    }
    int ret;
    self->mBlisrcParam.PCM_Format = (uint32_t)format;
    self->mBlisrcParam.in_sampling_rate = input_SR;
    self->mBlisrcParam.ou_sampling_rate = output_SR;
    self->mBlisrcParam.in_channel = input_channel_num;
    self->mBlisrcParam.ou_channel = output_channel_num;

#if defined(HAVE_SWIP)
    ret = Blisrc_MultiChannel_GetBufferSize(&self->mInternalBufSize,
                                            &self->mTempBufSize,
                                            &self->mBlisrcParam);

    if (ret < 0) {
        ALOGD("Blisrc_GetBufferSize return err %d\n", ret);
        return ACE_INVALIDE_OPERATION;
    }
#endif
    if (self->mInternalBufSize > 0) {
        self->mpInternalBuf = (char *)malloc(self->mInternalBufSize);
    }
    if (self->mTempBufSize > 0) {
        self->mpTempBuf = (char *)malloc(self->mTempBufSize);
    }
#if defined(HAVE_SWIP)
    Blisrc_MultiChannel_Open(&self->mBlisrcHandler, self->mpInternalBuf, &self->mBlisrcParam);
#endif
    self->mState = ACE_STATE_OPEN;
    ALOGD("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;

}

static int MultiChannel_ResetBuffer(struct MtkAudioSrcInC *self) {
    ALOGD("+%s()\n", __FUNCTION__);
    if (self->mState != ACE_STATE_OPEN) {
        return ACE_INVALIDE_OPERATION;
    }
#if defined(HAVE_SWIP)
    Blisrc_MultiChannel_Reset(self->mBlisrcHandler);
#endif
    ALOGD("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;

}

static int MultiChannel_Process(struct MtkAudioSrcInC *self,
                                void *pInputBuffer,   /* Input, pointer to input buffer */
                                uint32_t *InputSampleCount,        /* Input, length(byte) of input buffer */
                                /* Output, length(byte) left in the input buffer after conversion */
                                void *pOutputBuffer,               /* Input, pointer to output buffer */
                                uint32_t *OutputSampleCount)      /* Input, length(byte) of output buffer */
/* Output, output data length(byte) */
{
    ALOGV("+%s(), inputCnt %d, outputCnt %x\n", __FUNCTION__, *InputSampleCount, *OutputSampleCount);
    if (self->mState != ACE_STATE_OPEN) {
        return ACE_INVALIDE_OPERATION;
    }

    char *tmp = NULL;

    uint32_t bit_ratio = (self->mBlisrcParam.PCM_Format == SRC_IN_Q1P15_OUT_Q1P31) ? 2 : 1;

    uint32_t raw_count = *InputSampleCount;
    uint32_t src_count = (raw_count * bit_ratio * self->mBlisrcParam.ou_sampling_rate * self->mBlisrcParam.ou_channel) /
                         (self->mBlisrcParam.in_sampling_rate * self->mBlisrcParam.in_channel);

    uint32_t size_per_frame = 0;
    if (self->mBlisrcParam.PCM_Format == SRC_IN_Q1P15_OUT_Q1P15) {
        size_per_frame = self->mBlisrcParam.ou_channel * 2;
    }
    else if (self->mBlisrcParam.PCM_Format == SRC_IN_Q1P15_OUT_Q1P31 ||
             self->mBlisrcParam.PCM_Format == SRC_IN_Q9P23_OUT_Q1P31 ||
             self->mBlisrcParam.PCM_Format == SRC_IN_Q1P31_OUT_Q1P31) {
        size_per_frame = self->mBlisrcParam.ou_channel * 4;
    }
    else {
        ALOGE("unknown format %u!!", self->mBlisrcParam.PCM_Format);
        size_per_frame = 1;
    }

    if ((src_count % size_per_frame) != 0) { // alignment
        src_count = ((src_count / size_per_frame) + 1) * size_per_frame;
    }

    if (src_count > *OutputSampleCount) {
        ALOGE("OutputSampleCount %u < %u!!", *OutputSampleCount, src_count);
        src_count = *OutputSampleCount;
    }


#if defined(HAVE_SWIP)
    Blisrc_MultiChannel_Process(self->mBlisrcHandler,
                                self->mpTempBuf,
                                pInputBuffer,
                                InputSampleCount,
                                pOutputBuffer,
                                OutputSampleCount);
#endif

    if (*OutputSampleCount != src_count) {
        if (self->compensated == 1) {
            //ALOGW("OutputSampleCount %u != %u", *OutputSampleCount, src_count);
        }
        else {
            ALOGD("OutputSampleCount %u => %u", *OutputSampleCount, src_count);
            if (src_count > *OutputSampleCount) {
                tmp = (char *)malloc(src_count);
                memset(tmp, 0, src_count);

                memcpy(tmp + (src_count - *OutputSampleCount),
                       pOutputBuffer,
                       *OutputSampleCount);
                memcpy(pOutputBuffer, tmp, src_count);
                *OutputSampleCount = src_count;

                free(tmp);
                tmp = NULL;
            }
        }
    }
    self->compensated = 1;

    ALOGV("-%s(), inputCnt %d, outputCnt %x\n", __FUNCTION__, *InputSampleCount, *OutputSampleCount);
    return ACE_SUCCESS;
}

void InitMtkAudioSrcInC(struct MtkAudioSrcInC *self) {
    ALOGD("%s", __FUNCTION__);
    self->mpTempBuf = NULL;
    self->mpInternalBuf = NULL;
    self->mBlisrcHandler = NULL;
    self->mTempBufSize = 0;
    self->mInternalBufSize = 0;
    self->mState = ACE_STATE_INIT;
    memset(&self->mBlisrcParam, 0, sizeof(Blisrc_Param));

    /* assign function*/
    self->open = SrcOpen;
    self->close = SrcClose;
    self->SetParameter = SrcSetParameter;
    self->GetParameter = SrcGetParameter;
    self->Reset = SrcReset;
    self->Process = SrcProcess;
    self->MultiChannel_Open = MultiChannel_Open;
    self->MultiChannel_ResetBuffer = MultiChannel_ResetBuffer;
    self->MultiChannel_Process = MultiChannel_Process;

    self->compensated = 0;
}


