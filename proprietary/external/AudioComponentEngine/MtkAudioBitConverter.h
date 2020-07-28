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
#ifndef __MTK_AUDIO_BIT_CONVERTER_EXP_H__
#define __MTK_AUDIO_BIT_CONVERTER_EXP_H__
#include <utils/threads.h>

#include "MtkAudioComponent.h"
extern "C" {
#include "Shifter_exp.h"
#include "Limiter_exp.h"
}

namespace android {

class MtkAudioBitConverter : public MtkAudioBitConverterBase {
public:
    MtkAudioBitConverter();
    MtkAudioBitConverter(uint32_t sampling_rate, uint32_t channel_num, BCV_PCM_FORMAT format);
    virtual ACE_ERRID setParameter(uint32_t paramID, void *param);
    virtual ACE_ERRID getParameter(uint32_t paramID, void *param);
    virtual ACE_ERRID open(void);
    virtual ACE_ERRID close(void);
    virtual ACE_ERRID resetBuffer(void);
    virtual ACE_ERRID process(void *pInputBuffer,   /* Input, pointer to input buffer */
                              uint32_t *InputSampleCount,        /* Input, length(byte) of input buffer */
                              /* Output, length(byte) left in the input buffer after conversion */
                              void *pOutputBuffer,               /* Input, pointer to output buffer */
                              uint32_t *OutputSampleCount);      /* Input, length(byte) of output buffer */
    /* Output, output data length(byte) */
    virtual ~MtkAudioBitConverter();
private:
    ACE_STATE mState;
    uint32_t mPcmFormat;
    uint32_t mChannelNum;
    uint32_t mSampleRate;
    char *mpTempBuf;
    char *mpInternalBuf;
    uint32_t mTempBufSize; // in byte
    uint32_t mInternalBufSize; // in byte
    Limiter_InitParam mLimiterInitPar;
    Limiter_Handle *mLimiterHandler;
    Mutex mLock;
};

}; // namespace android

#endif // __MTK_AUDIO_BIT_CONVERTER_EXP_H__
