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
*   This file implements Mtk Audio Ssampl Rate Converter
*/

#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>

#define HAVE_SWIP

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG  "MtkAudioSrc"

#include <sys/ioctl.h>
#include <utils/Log.h>
#include <utils/String8.h>
#include <assert.h>
#include "MtkAudioSrc.h"
//#define ENABLE_LOG_AUDIO_SRC
#ifdef ENABLE_LOG_AUDIO_SRC
#undef ALOGV
#define ALOGV(...) ALOGD(__VA_ARGS__)
#endif

namespace android {

MtkAudioSrc::MtkAudioSrc() {
    mpTempBuf      = NULL;
    mpInternalBuf  = NULL;
    mBlisrcHandler = NULL;
    mTempBufSize   = 0;
    mInternalBufSize = 0;
    mState = ACE_STATE_INIT;
    memset(&mBlisrcParam, 0, sizeof(mBlisrcParam));
    ALOGV("MtkAudioSrc Constructor\n");
}

MtkAudioSrc::MtkAudioSrc(uint32_t input_SR, uint32_t input_channel_num, uint32_t output_SR, uint32_t output_channel_num, SRC_PCM_FORMAT format) {
    mpTempBuf      = NULL;
    mpInternalBuf  = NULL;
    mBlisrcHandler = NULL;
    mTempBufSize   = 0;
    mInternalBufSize = 0;
    mBlisrcParam.PCM_Format = (uint32_t)format;
    mBlisrcParam.in_sampling_rate = input_SR;
    mBlisrcParam.ou_sampling_rate = output_SR;
    mBlisrcParam.in_channel = input_channel_num;
    mBlisrcParam.ou_channel = output_channel_num;
    mState = ACE_STATE_INIT;
    ALOGV("MtkAudioSrc Constructor in SR %d, CH %d; out SR %d, CH %d; format %d\n", input_SR, input_channel_num, output_SR, output_channel_num, format);
}

MtkAudioSrc::~MtkAudioSrc() {
    ALOGV("+%s()\n", __FUNCTION__);
    if (mpTempBuf != NULL) {
        delete[] mpTempBuf;
        mpTempBuf = NULL;
    }
    if (mpInternalBuf != NULL) {
        delete[] mpInternalBuf;
        mpInternalBuf = NULL;
    }
    ALOGV("-%s()\n", __FUNCTION__);
}

ACE_ERRID MtkAudioSrc::setParameter(uint32_t paramID, void *param) {
    ALOGV("+%s(), paramID %d, param 0x%x\n", __FUNCTION__, paramID, (uint32_t)((long)param));
    Mutex::Autolock _l(mLock);
    uint32_t currentParam = (uint32_t)((long)param);
    if (mState == ACE_STATE_OPEN && paramID != SRC_PAR_SET_INPUT_SAMPLE_RATE) { //Only input sampling rate could be update during process.
        return ACE_INVALIDE_OPERATION;
    }
    switch (paramID) {
    case SRC_PAR_SET_PCM_FORMAT: {
        if (currentParam >= SRC_IN_END) {
            return ACE_INVALIDE_PARAMETER;
        }
        mBlisrcParam.PCM_Format = currentParam;
        break;
    }
    case SRC_PAR_SET_INPUT_SAMPLE_RATE: {
        mBlisrcParam.in_sampling_rate = currentParam;
        if (mState == ACE_STATE_OPEN) {
#if defined(HAVE_SWIP)
            Blisrc_SetSamplingRate(mBlisrcHandler, mBlisrcParam.in_sampling_rate);
#endif
        }
        break;
    }
    case SRC_PAR_SET_OUTPUT_SAMPLE_RATE: {
        mBlisrcParam.ou_sampling_rate = currentParam;
        break;
    }
    case SRC_PAR_SET_INPUT_CHANNEL_NUMBER: {
        mBlisrcParam.in_channel = currentParam;
        break;
    }
    case SRC_PAR_SET_OUTPUT_CHANNEL_NUMBER: {
        mBlisrcParam.ou_channel = currentParam;
        break;
    }
    default:
        return ACE_INVALIDE_PARAMETER;
    }
    ALOGV("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;
}


ACE_ERRID MtkAudioSrc::getParameter(uint32_t paramID, void *param) {
    ALOGD("+%s(), paramID %d\n", __FUNCTION__, paramID);
    Mutex::Autolock _l(mLock);
    long currentParam = 0;
    switch (paramID) {
    case SRC_PAR_GET_PCM_FORMAT: {
        currentParam = (long)mBlisrcParam.PCM_Format;
        break;
    }
    case SRC_PAR_GET_INPUT_SAMPLE_RATE: {
        currentParam = (long)mBlisrcParam.in_sampling_rate;
        break;
    }
    case SRC_PAR_GET_OUTPUT_SAMPLE_RATE: {
        currentParam = (long)mBlisrcParam.ou_sampling_rate;
        break;
    }
    case SRC_PAR_GET_INPUT_CHANNEL_NUMBER: {
        currentParam = (long)mBlisrcParam.in_channel;
        break;
    }
    case SRC_PAR_GET_OUTPUT_CHANNEL_NUMBER: {
        currentParam = (long)mBlisrcParam.ou_channel;
        break;
    }
    default:
        return ACE_INVALIDE_PARAMETER;
    }
    param = (void *) currentParam;
    ALOGD("-%s(), paramID %d, param 0x%x\n", __FUNCTION__, paramID, (uint32_t)((long)param));
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioSrc::open(void) {
    Mutex::Autolock _l(mLock);
    if (mState != ACE_STATE_INIT) {
        ALOGD("%s(), mState != ACE_STATE_INIT\n", __FUNCTION__);
        return ACE_INVALIDE_OPERATION;
    }
    int returnValue;
#if defined(HAVE_SWIP)
    returnValue = Blisrc_GetBufferSize(&mInternalBufSize,
                               &mTempBufSize,
                               &mBlisrcParam);

    if (returnValue < 0) {
        ALOGD("Blisrc_GetBufferSize return err %d\n", returnValue);
        return ACE_INVALIDE_OPERATION;
    }
#endif
    if (mInternalBufSize > 0) {
        mpInternalBuf = new char[mInternalBufSize];
    }
    if (mTempBufSize > 0) {
        mpTempBuf = new char[mTempBufSize];
    }
#if defined(HAVE_SWIP)
    Blisrc_Open(&mBlisrcHandler, mpInternalBuf, &mBlisrcParam);
#endif
    mState = ACE_STATE_OPEN;
    ALOGD("%s(), SR %d, CH %d; out SR %d, CH %d; format %d, mState = %d\n",
          __FUNCTION__, mBlisrcParam.in_sampling_rate, mBlisrcParam.in_channel,
          mBlisrcParam.ou_sampling_rate, mBlisrcParam.ou_channel,
          mBlisrcParam.PCM_Format, mState);

    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioSrc::close(void) {
    Mutex::Autolock _l(mLock);
    if (mState != ACE_STATE_OPEN) {
        ALOGD("%s(), mState != ACE_STATE_OPEN\n", __FUNCTION__);
        return ACE_INVALIDE_OPERATION;
    }
    if (mpTempBuf != NULL) {
        delete[] mpTempBuf;
        mpTempBuf = NULL;
    }
    if (mpInternalBuf != NULL) {
        delete[] mpInternalBuf;
        mpInternalBuf = NULL;
    }
    mState = ACE_STATE_INIT;
    ALOGD("%s(), mState = %d\n", __FUNCTION__, mState);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioSrc::resetBuffer(void) {
    ALOGD("+%s()\n", __FUNCTION__);
    Mutex::Autolock _l(mLock);
    if (mState != ACE_STATE_OPEN) {
        return ACE_INVALIDE_OPERATION;
    }
#if defined(HAVE_SWIP)
    Blisrc_Reset(mBlisrcHandler);
#endif
    ALOGD("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;
}

ACE_ERRID MtkAudioSrc::process(void *pInputBuffer,      /* Input, pointer to input buffer */
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
    Blisrc_Process(mBlisrcHandler,
                   mpTempBuf,
                   pInputBuffer,
                   InputSampleCount,
                   pOutputBuffer,
                   OutputSampleCount);
#endif
    ALOGV("-%s(), inputCnt %d, outputCnt %x\n", __FUNCTION__, *InputSampleCount, *OutputSampleCount);
    return ACE_SUCCESS;
}


ACE_ERRID MtkAudioSrc::multiChannelOpen(void) {
    ALOGD("+%s()\n", __FUNCTION__);
    Mutex::Autolock _l(mLock);
    if (mState != ACE_STATE_INIT) {
        return ACE_INVALIDE_OPERATION;
    }
    int returnValue;
#if defined(HAVE_SWIP)
    returnValue = Blisrc_MultiChannel_GetBufferSize(&mInternalBufSize,
                                            &mTempBufSize,
                                            &mBlisrcParam);

    if (returnValue < 0) {
        ALOGD("Blisrc_GetBufferSize return err %d\n", returnValue);
        return ACE_INVALIDE_OPERATION;
    }
#endif
    if (mInternalBufSize > 0) {
        mpInternalBuf = new char[mInternalBufSize];
    }
    if (mTempBufSize > 0) {
        mpTempBuf = new char[mTempBufSize];
    }
#if defined(HAVE_SWIP)
    Blisrc_MultiChannel_Open(&mBlisrcHandler, mpInternalBuf, &mBlisrcParam);
#endif
    mState = ACE_STATE_OPEN;
    ALOGD("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;
}


ACE_ERRID MtkAudioSrc::multiChannelResetBuffer(void) {
    ALOGD("+%s()\n", __FUNCTION__);
    Mutex::Autolock _l(mLock);
    if (mState != ACE_STATE_OPEN) {
        return ACE_INVALIDE_OPERATION;
    }
#if defined(HAVE_SWIP)
    Blisrc_MultiChannel_Reset(mBlisrcHandler);
#endif
    ALOGD("-%s()\n", __FUNCTION__);
    return ACE_SUCCESS;
}


ACE_ERRID MtkAudioSrc::multiChannelProcess(void *pInputBuffer,      /* Input, pointer to input buffer */
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
    Blisrc_MultiChannel_Process(mBlisrcHandler,
                                mpTempBuf,
                                pInputBuffer,
                                InputSampleCount,
                                pOutputBuffer,
                                OutputSampleCount);
#endif
    ALOGV("-%s(), inputCnt %d, outputCnt %x\n", __FUNCTION__, *InputSampleCount, *OutputSampleCount);
    return ACE_SUCCESS;
}

}//namespace android

