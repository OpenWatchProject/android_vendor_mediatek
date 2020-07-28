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
#define LOG_TAG  "MtkAudioBitConverterc"

#include <sys/ioctl.h>
#include <utils/Log.h>
#include <assert.h>
#include "MtkAudioBitConverterc.h"
#ifdef FLT_PROFILING
#include <sys/time.h>
#endif

#define HAVE_SWIP

static int BitConvertionOpen(struct MtkAudioBitConverterC *self, uint32_t sampling_rate, uint32_t channel_num, BCV_PCM_FORMAT format) {
    ALOGV("+%s() self->mPcmFormat = %d", __FUNCTION__, self->mPcmFormat);
    if (self->mState != ACE_STATE_INIT) {
        return ACE_INVALIDE_OPERATION;
    }

    self->mSampleRate = sampling_rate;
    self->mChannelNum = channel_num;
    self->mPcmFormat = format;

    if (self->mPcmFormat != 0xFFFF && self->mChannelNum != 0 && self->mSampleRate != 0) {
        if (self->mPcmFormat >= BCV_DOWN_BIT && self->mPcmFormat < BCV_DOWN_BIT_END) { // Use Limiter
#if defined(HAVE_SWIP)
            uint32_t transformPcmFormat; // transform to Limiter's PcmFormat
            transformPcmFormat = self->mPcmFormat - BCV_DOWN_BIT;
            Limiter_GetBufferSize(&self->mInternalBufSize, &self->mTempBufSize, self->mPcmFormat);
            if (self->mInternalBufSize > 0) {
                self->mpInternalBuf = (char *)malloc(self->mInternalBufSize);
            }
            if (self->mTempBufSize > 0) {
                self->mpTempBuf = (char *)malloc(self->mTempBufSize);
            }
            self->mLimiterInitPar.Channel = self->mChannelNum;
            self->mLimiterInitPar.Sampling_Rate = self->mSampleRate;
            self->mLimiterInitPar.PCM_Format = transformPcmFormat;
            Limiter_Open(&self->mLimiterHandler, self->mpInternalBuf, &(self->mLimiterInitPar));
#endif
        } else { // Use Shifter

            // Do nothing
        }
    }
    self->mState = ACE_STATE_OPEN;
    ALOGV("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;

}


static int BitConvertionClose(struct MtkAudioBitConverterC *self) {
    ALOGV("+%s()\n", __FUNCTION__);
    if (self->mpTempBuf != NULL) {
        free(self->mpTempBuf);
        self->mpTempBuf = NULL;
    }
    if (self->mpInternalBuf != NULL) {
        free(self->mpInternalBuf);
        self->mpInternalBuf = NULL;
    }
    ALOGV("-%s()\n", __FUNCTION__);
    return 0;
}

static int BitConvertionSetParameter(struct MtkAudioBitConverterC *self, uint32_t paramID, void *param) {
    ALOGD("+%s(), paramID %d, param 0x%x\n", __FUNCTION__, paramID, (uint32_t)((long)param));
    if (self->mState == ACE_STATE_OPEN) {
        return ACE_INVALIDE_OPERATION;
    }
    uint32_t Curparam = (uint32_t)((long)param);
    switch (paramID) {
    case BCV_PAR_SET_PCM_FORMAT: {
        uint32_t pcm_format = Curparam;
        if (((pcm_format >= BCV_UP_BIT) && (pcm_format <= BCV_UP_BIT_END)) ||
            ((pcm_format >= BCV_DOWN_BIT) && (pcm_format <= BCV_DOWN_BIT_END))) {
            self->mPcmFormat = pcm_format;
        } else {
            return ACE_INVALIDE_PARAMETER;
        }
        break;
    }
    case BCV_PAR_SET_SAMPLE_RATE: {
        self->mSampleRate = Curparam;
        break;
    }
    case BCV_PAR_SET_CHANNEL_NUMBER: {
        if (Curparam == 2 || Curparam == 1) {
            self->mChannelNum = Curparam;
        } else {
            return ACE_INVALIDE_PARAMETER;
        }
        break;
    }
    default:
        return ACE_INVALIDE_PARAMETER;
    }
    ALOGV("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;

    return 0;
}

static int BitConvertionGetParameter(struct MtkAudioBitConverterC *self, uint32_t paramID, void *param) {
    ALOGD("+%s(), paramID %d, param %p\n", __FUNCTION__, paramID, param);
    long Curparam;
    switch (paramID) {
    case BCV_PAR_GET_PCM_FORMAT: {
        Curparam = (long) self->mPcmFormat;
        break;
    }
    case BCV_PAR_SET_SAMPLE_RATE: {
        Curparam = (long) self->mSampleRate;
        break;
    }
    case BCV_PAR_SET_CHANNEL_NUMBER: {
        Curparam = (long) self->mChannelNum;
        break;
    }
    default:
        return ACE_INVALIDE_PARAMETER;
    }
    ALOGV("-%s(), paramID %d, param %p\n", __FUNCTION__, paramID, param);

    param = (void *) Curparam;
    return ACE_SUCCESS;

}

static int BitConvertionReset(struct MtkAudioBitConverterC *self) {
    ALOGV("+%s()\n", __FUNCTION__);
    if (self->mPcmFormat >= BCV_DOWN_BIT && self->mState == ACE_STATE_OPEN) { // Use Limiter
#if defined(HAVE_SWIP)
        Limiter_Reset(self->mLimiterHandler);
#endif
    }
    ALOGV("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;
}

static int BitConvertionProcess(struct MtkAudioBitConverterC *self,
                                void *pInputBuffer,   /* Input, pointer to input buffer */
                                uint32_t *InputSampleCount,        /* Input, length(byte) of input buffer */
                                /* Output, length(byte) left in the input buffer after conversion */
                                void *pOutputBuffer,                        /* Input, pointer to output buffer */
                                uint32_t *OutputSampleCount) {
    ALOGV("+%s(), inputCnt %d, outputCnt %x\n", __FUNCTION__, *InputSampleCount, *OutputSampleCount);

    if (self->mState != ACE_STATE_OPEN) {
        return ACE_INVALIDE_OPERATION;
    }
#if defined(HAVE_SWIP)
    if (self->mPcmFormat >= BCV_DOWN_BIT && self->mPcmFormat < BCV_DOWN_BIT_END) { // Use Limiter
        Limiter_Process(self->mLimiterHandler,
                        self->mpTempBuf,
                        pInputBuffer,
                        InputSampleCount,
                        pOutputBuffer,
                        OutputSampleCount);
    } else if (self->mPcmFormat < BCV_UP_BIT_END) { // Use shifter
        Shifter_Process(pInputBuffer, InputSampleCount, pOutputBuffer, OutputSampleCount, self->mPcmFormat);
    }
#endif

    ALOGV("-%s(), inputCnt %d, outputCnt %x\n", __FUNCTION__, *InputSampleCount, *OutputSampleCount);
    return ACE_SUCCESS;

}


void InitMtkAudioBitConverterC(struct MtkAudioBitConverterC *self) {
    ALOGV("%s", __FUNCTION__);
    self->mPcmFormat = 0xFFFF;
    self->mChannelNum = 0;
    self->mSampleRate = 0;
    self->mpTempBuf = NULL;
    self->mpInternalBuf = NULL;
    self->mTempBufSize = 0;
    self->mInternalBufSize = 0;
    self->mLimiterHandler = NULL;
    memset(&self->mLimiterInitPar, 0, sizeof(Limiter_InitParam));

    self->mState = ACE_STATE_INIT;
    self->open = BitConvertionOpen;
    self->close = BitConvertionClose;
    self->SetParameter = BitConvertionSetParameter;
    self->GetParameter = BitConvertionGetParameter;
    self->Reset = BitConvertionReset;
    self->Process = BitConvertionProcess;
}


