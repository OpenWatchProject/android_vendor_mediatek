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

/*
* Description:
*   This file implements Audio Bit Converter
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
#define LOG_TAG  "MtkAudioBitConverter"

#include <sys/ioctl.h>
#include <utils/Log.h>
#include <utils/String8.h>
#include <assert.h>
#include "MtkAudioBitConverter.h"
//#define ENABLE_LOG_AudioCompensationFilter
#ifdef MtkAudioBitConverter
#undef ALOGV
#define ALOGV(...) ALOGD(__VA_ARGS__)
#endif

#define HAVE_SWIP

namespace android {

MtkAudioBitConverter::MtkAudioBitConverter() :
    mPcmFormat(0xFFFF), mChannelNum(0), mSampleRate(0), mpTempBuf(NULL), mpInternalBuf(NULL),
    mTempBufSize(0), mInternalBufSize(0), mLimiterHandler(NULL) {
    memset(&mLimiterInitPar, 0, sizeof(Limiter_InitParam));
    mState = ACE_STATE_INIT;
    ALOGV("MtkAudioBitConverter Constructor\n");
}

MtkAudioBitConverter::MtkAudioBitConverter(uint32_t sampling_rate, uint32_t channel_num, BCV_PCM_FORMAT format) :
    mPcmFormat(format), mChannelNum(channel_num), mSampleRate(sampling_rate), mpTempBuf(NULL), mpInternalBuf(NULL),
    mTempBufSize(0), mInternalBufSize(0), mLimiterHandler(NULL) {
    memset(&mLimiterInitPar, 0, sizeof(Limiter_InitParam));
    mState = ACE_STATE_INIT;
    ALOGV("MtkAudioBitConverter Constructor, SR %d, CH %d, format %d\n", sampling_rate, channel_num, format);
}

MtkAudioBitConverter::~MtkAudioBitConverter() {
    ALOGV("+%s()\n", __FUNCTION__);
    if (mpTempBuf != NULL) {
        delete mpTempBuf;
        mpTempBuf = NULL;
    }
    if (mpInternalBuf != NULL) {
        delete mpInternalBuf;
        mpInternalBuf = NULL;
    }
    ALOGV("-%s()\n", __FUNCTION__);
}

ACE_ERRID MtkAudioBitConverter::setParameter(uint32_t paramID, void *param) {
    ALOGD("+%s(), paramID %d, param 0x%x\n", __FUNCTION__, paramID, (uint32_t)((long)param));
    Mutex::Autolock _l(mLock);
    if (mState == ACE_STATE_OPEN) {
        return ACE_INVALIDE_OPERATION;
    }
    uint32_t Curparam = (uint32_t)((long)param);
    switch (paramID) {
    case BCV_PAR_SET_PCM_FORMAT: {
        uint32_t pcm_format = Curparam;
        if (((pcm_format >= BCV_UP_BIT) && (pcm_format <= BCV_UP_BIT_END)) ||
            ((pcm_format >= BCV_DOWN_BIT) && (pcm_format <= BCV_DOWN_BIT_END))) {
            mPcmFormat = pcm_format;
        } else {
            return ACE_INVALIDE_PARAMETER;
        }
        break;
    }
    case BCV_PAR_SET_SAMPLE_RATE: {
        //To do : Add Check
        mSampleRate = Curparam;
        break;
    }
    case BCV_PAR_SET_CHANNEL_NUMBER: {
        if (Curparam == 2 || Curparam == 1) {
            mChannelNum = Curparam;
        } else {
            return ACE_INVALIDE_PARAMETER;
        }
        break;
    }
    default:
        return ACE_INVALIDE_PARAMETER;
    }
    ALOGD("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioBitConverter::getParameter(uint32_t paramID, void *param) {
    ALOGD("+%s(), paramID %d, param %p\n", __FUNCTION__, paramID, param);
    Mutex::Autolock _l(mLock);
    long Curparam;
    switch (paramID) {
    case BCV_PAR_GET_PCM_FORMAT: {
        Curparam = (long) mPcmFormat;
        break;
    }
    case BCV_PAR_SET_SAMPLE_RATE: {
        Curparam = (long) mSampleRate;
        break;
    }
    case BCV_PAR_SET_CHANNEL_NUMBER: {
        Curparam = (long) mChannelNum;
        break;
    }
    default:
        return ACE_INVALIDE_PARAMETER;
    }
    ALOGD("-%s(), paramID %d, param %p\n", __FUNCTION__, paramID, param);

    param = (void *) Curparam;
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioBitConverter::open(void) {
    ALOGD("+%s(), format %d, CH %d, SR %d\n", __FUNCTION__, mPcmFormat, mChannelNum, mSampleRate);
    Mutex::Autolock _l(mLock);
    if (mState != ACE_STATE_INIT) {
        return ACE_INVALIDE_OPERATION;
    }
    if (mPcmFormat != 0xFFFF && mChannelNum != 0 && mSampleRate != 0) {
        if (mPcmFormat >= BCV_DOWN_BIT && mPcmFormat < BCV_DOWN_BIT_END) { // Use Limiter
#if defined(HAVE_SWIP)
            uint32_t transformPcmFormat; // transform to Limiter's PcmFormat
            transformPcmFormat = mPcmFormat - BCV_DOWN_BIT;
            Limiter_GetBufferSize(&mInternalBufSize, &mTempBufSize, mPcmFormat);
            if (mInternalBufSize > 0) {
                mpInternalBuf = new char[mInternalBufSize];
            }
            if (mTempBufSize > 0) {
                mpTempBuf = new char[mTempBufSize];
            }
            mLimiterInitPar.Channel = mChannelNum;
            mLimiterInitPar.Sampling_Rate = mSampleRate;
            mLimiterInitPar.PCM_Format = transformPcmFormat;
            Limiter_Open(&mLimiterHandler, mpInternalBuf, &mLimiterInitPar);
#endif
        } else { // Use Shifter
            // Do nothing
        }
    }
    mState = ACE_STATE_OPEN;
    ALOGV("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioBitConverter::close(void) {
    ALOGD("+%s()\n", __FUNCTION__);
    Mutex::Autolock _l(mLock);
    if (mState != ACE_STATE_OPEN) {
        return ACE_INVALIDE_OPERATION;
    }
    if (mpTempBuf != NULL) {
        delete mpTempBuf;
        mpTempBuf = NULL;
    }
    if (mpInternalBuf != NULL) {
        delete mpInternalBuf;
        mpInternalBuf = NULL;
    }
    mState = ACE_STATE_INIT;
    ALOGV("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioBitConverter::resetBuffer(void) {
    ALOGD("+%s()\n", __FUNCTION__);
    Mutex::Autolock _l(mLock);
    if (mPcmFormat >= BCV_DOWN_BIT && mState == ACE_STATE_OPEN) { // Use Limiter
#if defined(HAVE_SWIP)
        Limiter_Reset(mLimiterHandler);
#endif
    }
    ALOGV("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;
}
/* Return: consumed input buffer size(byte)                             */
ACE_ERRID MtkAudioBitConverter::process(void *pInputBuffer,   /* Input, pointer to input buffer */
                                        uint32_t *InputSampleCount,        /* Input, length(byte) of input buffer */
                                        /* Output, length(byte) left in the input buffer after conversion */
                                        void *pOutputBuffer,               /* Input, pointer to output buffer */
                                        uint32_t *OutputSampleCount)       /* Input, length(byte) of output buffer */
/* Output, output data length(byte) */
{
    ALOGV("+%s(), inputCnt %d, outputCnt %x\n", __FUNCTION__, *InputSampleCount, *OutputSampleCount);
    Mutex::Autolock _l(mLock);
    if (mState != ACE_STATE_OPEN) {
        return ACE_INVALIDE_OPERATION;
    }
#if defined(HAVE_SWIP)
    if (mPcmFormat >= BCV_DOWN_BIT && mPcmFormat < BCV_DOWN_BIT_END) { // Use Limiter
        Limiter_Process(mLimiterHandler,
                        mpTempBuf,
                        pInputBuffer,
                        InputSampleCount,
                        pOutputBuffer,
                        OutputSampleCount);
    } else if (mPcmFormat < BCV_UP_BIT_END) { // Use shifter
        Shifter_Process(pInputBuffer, InputSampleCount, pOutputBuffer, OutputSampleCount, mPcmFormat);
    }
#endif
#if 0
    if (mPcmFormat >= BCV_SIMPLE_SHIFT_BIT && mPcmFormat < BCV_SIMPLE_SHIFT_BIT_END) { // Simply shift bit
        uint32_t in_cnt, out_cnt, cnt;
        if (mPcmFormat == BCV_IN_Q1P31_OUT_Q1P15) {
            int16_t *outBuf;
            int32_t *inBuf;
            in_cnt = (*InputSampleCount) >> 2;
            out_cnt = (*OutputSampleCount) >> 1;
            outBuf = (int16_t *)pOutputBuffer;
            inBuf = (int32_t *)pInputBuffer;
            if (in_cnt >= out_cnt) {
                cnt = out_cnt;
            } else {
                cnt = in_cnt;
            }
            for (in_cnt = 0; in_cnt < cnt ; in_cnt ++) {
                *outBuf = (int16_t)(*inBuf >> 16);
                outBuf++;
                inBuf++;
            }
            *InputSampleCount = *InputSampleCount - (cnt << 2);
            *OutputSampleCount = (cnt << 1);
        } else if (mPcmFormat == BCV_IN_Q1P31_OUT_Q9P23) {
            int32_t *inBuf, *outBuf;
            in_cnt = (*InputSampleCount) >> 2;
            out_cnt = (*OutputSampleCount) >> 2;
            outBuf = (int32_t *)pOutputBuffer;
            inBuf = (int32_t *)pInputBuffer;
            if (in_cnt >= out_cnt) {
                cnt = out_cnt;
            } else {
                cnt = in_cnt;
            }
            for (in_cnt = 0; in_cnt < cnt ; in_cnt ++) {
                *outBuf = (*inBuf >> 8);
                outBuf++;
                inBuf++;
            }
            *InputSampleCount = *InputSampleCount - (cnt << 2);
            *OutputSampleCount = (cnt << 2);
        }
    }
#endif

    ALOGV("-%s(), inputCnt %d, outputCnt %x\n", __FUNCTION__, *InputSampleCount, *OutputSampleCount);
    return ACE_SUCCESS;
}

}//namespace android

