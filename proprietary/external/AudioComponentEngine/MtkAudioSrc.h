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

#ifndef __MTK_AUDIO_SRC_EXP_H__
#define __MTK_AUDIO_SRC_EXP_H__

#include <utils/threads.h>
#include "MtkAudioComponent.h"
extern "C" {
#include "Blisrc_exp.h"
}
namespace android {

class MtkAudioSrc : public MtkAudioSrcBase {
public:
    MtkAudioSrc();
    MtkAudioSrc(uint32_t input_SR, uint32_t input_channel_num, uint32_t output_SR, uint32_t output_channel_num, SRC_PCM_FORMAT format);
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

    virtual ACE_ERRID multiChannelOpen(void);
    virtual ACE_ERRID multiChannelResetBuffer(void);
    virtual ACE_ERRID multiChannelProcess(void *pInputBuffer,   /* Input, pointer to input buffer */
                                          uint32_t *InputSampleCount,        /* Input, length(byte) of input buffer */
                                          /* Output, length(byte) left in the input buffer after conversion */
                                          void *pOutputBuffer,               /* Input, pointer to output buffer */
                                          uint32_t *OutputSampleCount);      /* Input, length(byte) of output buffer */
    /* Output, output data length(byte) */
    virtual ~MtkAudioSrc();
private:
    ACE_STATE mState;
    uint32_t mTempBufSize; // in byte
    uint32_t mInternalBufSize; // in byte
    char *mpTempBuf;
    char *mpInternalBuf;
    Blisrc_Param mBlisrcParam;
    Blisrc_Handle *mBlisrcHandler;
    Mutex mLock;
};

}; // namespace android

#endif // __MTK_AUDIO_SRC_EXP_H__
