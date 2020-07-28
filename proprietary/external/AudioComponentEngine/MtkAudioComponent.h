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
*   This file implements virtual class function for supporting dlopen to use audio
*   post processing.
*/

#ifndef __AUDIO_COMPONENT_ENGINE_EXP_H__
#define __AUDIO_COMPONENT_ENGINE_EXP_H__
#include <utils/String8.h>
#include "CFG_AUDIO_File.h"
#define BCV_UP_BIT           0
#define BCV_DOWN_BIT         0x100
#define BCV_SIMPLE_SHIFT_BIT 0x200

typedef enum {
    BCV_PAR_SET_PCM_FORMAT,
    BCV_PAR_GET_PCM_FORMAT,
    BCV_PAR_SET_SAMPLE_RATE,
    BCV_PAR_GET_SAMPLE_RATE,
    BCV_PAR_SET_CHANNEL_NUMBER,
    BCV_PAR_GET_CHANNEL_NUMBER,
} BCV_PARAMETER;

typedef enum {
    BCV_IN_Q1P15_OUT_Q1P31    = BCV_UP_BIT + 0,     // 16-bit Q1.15 input, 32-bit Q1.31 output
    BCV_IN_Q1P31_OUT_Q1P15    = BCV_UP_BIT + 1,     // 32-bit Q1.31 input, 16-bit Q1.15 output
    BCV_IN_Q9P23_OUT_Q1P31    = BCV_UP_BIT + 2,     // 32-bit Q9.23 input, 32-bit Q1.31 output
    BCV_IN_Q1P31_OUT_Q9P23    = BCV_UP_BIT + 3,     // 32-bit Q1.31 input, 32-bit Q9.23 output
    BCV_IN_Q1P15_OUT_Q9P23    = BCV_UP_BIT + 4,     // 16-bit Q1.15 input, 32-bit Q9.23 output
    BCV_IN_Q9P23_OUT_Q1P15    = BCV_UP_BIT + 5,     // 32-bit Q9.23 input, 16-bit Q1.15 output
    BCV_IN_Q1P15_OUT_Q1P23    = BCV_UP_BIT + 6,     // 16-bit Q1.15 input, 24-bit Q1.23 output
    BCV_IN_Q1P23_OUT_Q1P15    = BCV_UP_BIT + 7,     // 24-bit Q1.23 input, 16-bit Q1.15 output
    BCV_IN_Q1P31_OUT_Q1P23    = BCV_UP_BIT + 8,     // 32-bit Q1.31 input, 24-bit Q1.23 output
    BCV_IN_Q1P23_OUT_Q1P31    = BCV_UP_BIT + 9,     // 24-bit Q1.23 input, 32-bit Q1.31 output
    BCV_IN_Q9P23_OUT_Q1P23    = BCV_UP_BIT + 10,    // 32-bit Q9.23 input, 24-bit Q1.23 output
    BCV_IN_Q1P23_OUT_Q9P23    = BCV_UP_BIT + 11,    // 24-bit Q1.23 input, 32-bit Q9.23 output
    BCV_UP_BIT_END            = BCV_UP_BIT + 12,    // End of up-bits. Do Not use this

    BCV_IN_Q33P31_OUT_Q1P31   = BCV_DOWN_BIT + 0,   // 64-bit Q33.31 input, 32-bit Q1.31 output
    BCV_DOWN_BIT_END          = BCV_DOWN_BIT + 1,   // End of down-bits. Do Not use this

    BCV_SIMPLE_SHIFT_BIT_END  = BCV_SIMPLE_SHIFT_BIT + 0,   // End of simple down-bits. Do Not use this
} BCV_PCM_FORMAT;

typedef enum {
    SRC_IN_Q1P15_OUT_Q1P15 = 0,   // 16-bit Q1.15 input, 16-bit Q1.15 output
    SRC_IN_Q1P15_OUT_Q1P31 = 1,   // 16-bit Q1.15 input, 32-bit Q1.31 output
    SRC_IN_Q9P23_OUT_Q1P31 = 2,   // 32-bit Q9.23 input, 32-bit Q1.31 output
    SRC_IN_Q1P31_OUT_Q1P31 = 3,   // 32-bit Q1.31 input, 32-bit Q1.31 output
    SRC_IN_END,
} SRC_PCM_FORMAT;

typedef enum {
    SRC_PAR_SET_PCM_FORMAT,
    SRC_PAR_GET_PCM_FORMAT,
    SRC_PAR_SET_INPUT_SAMPLE_RATE,
    SRC_PAR_GET_INPUT_SAMPLE_RATE,
    SRC_PAR_SET_OUTPUT_SAMPLE_RATE,
    SRC_PAR_GET_OUTPUT_SAMPLE_RATE,
    SRC_PAR_SET_INPUT_CHANNEL_NUMBER,
    SRC_PAR_GET_INPUT_CHANNEL_NUMBER,
    SRC_PAR_SET_OUTPUT_CHANNEL_NUMBER,
    SRC_PAR_GET_OUTPUT_CHANNEL_NUMBER,
} SRC_PARAMETER;

typedef enum {
    BLOUD_PAR_SET_FILTER_TYPE,
    BLOUD_PAR_SET_WORK_MODE,
    BLOUD_PAR_SET_CHANNEL_NUMBER,
    BLOUD_PAR_SET_SAMPLE_RATE,
    BLOUD_PAR_SET_PCM_FORMAT,
    BLOUD_PAR_SET_USE_DEFAULT_PARAM,
    BLOUD_PAR_SET_PREVIEW_PARAM,
    BLOUD_PAR_SET_USE_DEFAULT_PARAM_SUB,
    BLOUD_PAR_SET_PREVIEW_PARAM_SUB,
    BLOUD_PAR_SET_UPDATE_PARAM_TO_SWIP,
    BLOUD_PAR_SET_SEP_LR_FILTER,    //10
    BLOUD_PAR_SET_STEREO_TO_MONO_MODE,
    BLOUD_PAR_SET_RAMP_UP,
    BLOUD_PAR_SET_USE_DEFAULT_PARAM_FORCE_RELOAD,//bypass cache to load parameter
    BLOUD_PAR_SET_NOISE_FILTER,
    BLOUD_PAR_SET_LOAD_CACHE_PARAM,
    BLOUD_PAR_SET_FRAME_LENGTH,
} BLOUD_PARAMETER;

typedef enum {
    BLOUD_IN_Q1P15_OUT_Q1P15  = 0,   // 16-bit Q1.15  input, 16-bit Q1.15 output
    BLOUD_IN_Q1P31_OUT_Q1P31  = 1,   // 32-bit Q1.31  input, 32-bit Q1.31 output
} BLOUD_PCM_FORMAT;

typedef enum {
    BLOUD_S2M_MODE_NONE      = 0,  // None
    BLOUD_S2M_MODE_ST2MO2ST  = 1,  // Stereo to mono to stereo
} BLOUD_S2M_MODE_ENUM;

typedef enum {
    DCR_MODE_1 = 0,
    DCR_MODE_2,
    DCR_MODE_3
} DCR_MODE;

typedef enum {
    DCREMOVE_BIT16 = 0,
    DCREMOVE_BIT24,
} DCR_BITDEPTH;

namespace android {

typedef enum {
    ACE_WARN_MISMATCH_SCENE_NAME = 1,
    ACE_SUCCESS = 0,
    ACE_INVALIDE_PARAMETER = -1,
    ACE_INVALIDE_OPERATION = -2,
    ACE_NOT_INIT = -3,
    ACE_NOT_OPEN = -4,
} ACE_ERRID;

typedef enum {
    ACE_STATE_NONE,
    ACE_STATE_INIT,
    ACE_STATE_OPEN,
} ACE_STATE;

class AudioComponentEngineBase {
public:
    //AudioComponentEngineBase() {};
    virtual ACE_ERRID setParameter(uint32_t paramID, void *param) = 0;
    virtual ACE_ERRID getParameter(uint32_t paramID, void *param) = 0;
    virtual ACE_ERRID open(void) = 0;
    virtual ACE_ERRID close(void) = 0;
    virtual ACE_ERRID resetBuffer(void) = 0;
    /* Return: consumed input buffer size(byte)                             */
    virtual ACE_ERRID process(void *pInputBuffer,   /* Input, pointer to input buffer */
                              uint32_t *InputSampleCount,        /* Input, length(byte) of input buffer */
                              /* Output, length(byte) left in the input buffer after conversion */
                              void *pOutputBuffer,               /* Input, pointer to output buffer */
                              uint32_t *OutputSampleCount) = 0;  /* Input, length(byte) of output buffer */
    /* Output, output data length(byte) */
    virtual ~AudioComponentEngineBase() {};
};

class MtkAudioBitConverterBase : public AudioComponentEngineBase {
public:
    virtual ACE_ERRID setParameter(uint32_t paramID, void *param) = 0;
    virtual ACE_ERRID getParameter(uint32_t paramID, void *param) = 0;
    virtual ACE_ERRID open(void) = 0;
    virtual ACE_ERRID close(void) = 0;
    virtual ACE_ERRID resetBuffer(void) = 0;
    virtual ACE_ERRID process(void *pInputBuffer,   /* Input, pointer to input buffer */
                              uint32_t *InputSampleCount,        /* Input, length(byte) of input buffer */
                              /* Output, length(byte) left in the input buffer after conversion */
                              void *pOutputBuffer,               /* Input, pointer to output buffer */
                              uint32_t *OutputSampleCount) = 0;  /* Input, length(byte) of output buffer */
    /* Output, output data length(byte) */
    virtual ~MtkAudioBitConverterBase() {};
};

class MtkAudioSrcBase : public AudioComponentEngineBase {
public:
    virtual ACE_ERRID setParameter(uint32_t paramID, void *param) = 0;
    virtual ACE_ERRID getParameter(uint32_t paramID, void *param) = 0;
    virtual ACE_ERRID open(void) = 0;
    virtual ACE_ERRID close(void) = 0;
    virtual ACE_ERRID resetBuffer(void) = 0;
    virtual ACE_ERRID process(void *pInputBuffer,   /* Input, pointer to input buffer */
                              uint32_t *InputSampleCount,        /* Input, length(byte) of input buffer */
                              /* Output, length(byte) left in the input buffer after conversion */
                              void *pOutputBuffer,               /* Input, pointer to output buffer */
                              uint32_t *OutputSampleCount) = 0;  /* Input, length(byte) of output buffer */
    /* Output, output data length(byte) */

    virtual ACE_ERRID multiChannelOpen(void) = 0;
    virtual ACE_ERRID multiChannelResetBuffer(void) = 0;
    virtual ACE_ERRID multiChannelProcess(void *pInputBuffer,   /* Input, pointer to input buffer */
                                           uint32_t *InputSampleCount,        /* Input, length(byte) of input buffer */
                                           /* Output, length(byte) left in the input buffer after conversion */
                                           void *pOutputBuffer,               /* Input, pointer to output buffer */
                                           uint32_t *OutputSampleCount) = 0;  /* Input, length(byte) of output buffer */
    /* Output, output data length(byte) */
    virtual ~MtkAudioSrcBase() {};
};

class MtkAudioLoudBase : public AudioComponentEngineBase {
public:
    virtual ACE_ERRID setParameter(uint32_t paramID, void *param) = 0;
    virtual ACE_ERRID getParameter(uint32_t paramID, void *param) = 0;
    virtual ACE_ERRID open(void) = 0;
    virtual ACE_ERRID close(void) = 0;
    virtual ACE_ERRID resetBuffer(void) = 0;
    virtual ACE_ERRID change2ByPass(void) = 0;
    virtual ACE_ERRID change2Normal(void) = 0;
    virtual ACE_ERRID process(void *pInputBuffer,   /* Input, pointer to input buffer */
                              uint32_t *InputSampleCount,        /* Input, length(byte) of input buffer */
                              /* Output, length(byte) left in the input buffer after conversion */
                              void *pOutputBuffer,               /* Input, pointer to output buffer */
                              uint32_t *OutputSampleCount) = 0;  /* Input, length(byte) of output buffer */
    /* Output, output data length(byte) */
    virtual ACE_ERRID setWorkMode(uint32_t chNum, uint32_t smpRate, uint32_t workMode, bool bRampUpEnable) = 0;
    virtual ACE_ERRID setOutputGain(int32_t gain, uint32_t ramp_sample_cnt) = 0;
    virtual ACE_ERRID setNotchFilterParam(uint32_t fc, uint32_t bw, int32_t th) = 0;
    virtual bool isZeroCoeffFilter(void) = 0;
    virtual ~MtkAudioLoudBase() {};
    virtual ACE_ERRID setCustSceneName(const char* sceneName) = 0;
    virtual uint32_t getFilterType(void) = 0;
    virtual ACE_ERRID getCustSceneName(char* sceneName) = 0;
};

class MtkAudioDcRemoveBase : public AudioComponentEngineBase {
public:
    virtual ACE_ERRID setParameter(uint32_t paramID, void *param) = 0;
    virtual ACE_ERRID getParameter(uint32_t paramID, void *param) = 0;
    virtual ACE_ERRID open(void) = 0;
    virtual ACE_ERRID close(void) = 0;
    virtual ACE_ERRID resetBuffer(void) = 0;
    virtual ACE_ERRID process(void *pInputBuffer,   /* Input, pointer to input buffer */
                              uint32_t *InputSampleCount,        /* Input, length(byte) of input buffer */
                              /* Output, length(byte) left in the input buffer after conversion */
                              void *pOutputBuffer,               /* Input, pointer to output buffer */
                              uint32_t *OutputSampleCount) = 0;  /* Input, length(byte) of output buffer */
    virtual ACE_ERRID init(uint32_t channel, uint32_t samplerate, uint32_t dcrMode, uint32_t dBit) = 0;
    virtual ~MtkAudioDcRemoveBase() {};
};

class MtkAudioCustParamCacheBase {
public:
    virtual ACE_ERRID loadParameter(uint32_t FilterType, AUDIO_ACF_CUSTOM_PARAM_STRUCT *AudioParam, const char *custScene = NULL) = 0;
    virtual ACE_ERRID saveParameter(uint32_t FilterType, AUDIO_ACF_CUSTOM_PARAM_STRUCT *AudioParam, const char *custScene = NULL) = 0;
    virtual ACE_ERRID saveEncodedParameter(uint32_t FilterType, String8 keys, const char *custScene = NULL) = 0;
    virtual bool isCacheValid(uint32_t FilterType, const char *custScene = NULL) = 0;
    virtual ~MtkAudioCustParamCacheBase() {};
};

extern "C" MtkAudioBitConverterBase *createMtkAudioBitConverter(uint32_t sampling_rate, uint32_t channel_num, BCV_PCM_FORMAT format);
extern "C" MtkAudioSrcBase *createMtkAudioSrc(uint32_t input_SR, uint32_t input_channel_num, uint32_t output_SR, uint32_t output_channel_num, SRC_PCM_FORMAT format);
extern "C" MtkAudioLoudBase *createMtkAudioLoud(uint32_t eFLTtype, bool bFastTrack);
extern "C" MtkAudioDcRemoveBase *createMtkDcRemove(void);
extern "C" MtkAudioCustParamCacheBase *createMtkAudioCustParamCache(void);
extern "C" void destroyMtkAudioBitConverter(MtkAudioBitConverterBase *pObject);
extern "C" void destroyMtkAudioSrc(MtkAudioSrcBase *pObject);
extern "C" void destroyMtkAudioLoud(MtkAudioLoudBase *pObject);
extern "C" void destroyMtkAudioDcRemove(MtkAudioDcRemoveBase *pObject);
extern "C" void destroyMtkAudioCustParamCache(MtkAudioCustParamCacheBase *pObject);
typedef android::MtkAudioBitConverterBase *create_AudioBitConverter(uint32_t sampling_rate, uint32_t channel_num, BCV_PCM_FORMAT format);
typedef android::MtkAudioSrcBase *create_AudioSrc(uint32_t input_SR, uint32_t input_channel_num, uint32_t output_SR, uint32_t output_channel_num, SRC_PCM_FORMAT format);
typedef android::MtkAudioLoudBase *create_AudioLoud(uint32_t eFLTtype, bool bFastTrack);
typedef android::MtkAudioDcRemoveBase *create_DcRemove();
typedef android::MtkAudioCustParamCacheBase *create_AudioCustParamCache();
typedef void destroy_AudioBitConverter(MtkAudioBitConverterBase *pObject);
typedef void destroy_AudioSrc(MtkAudioSrcBase *pObject);
typedef void destroy_AudioLoud(MtkAudioLoudBase *pObject);
typedef void destroy_DcRemove(MtkAudioDcRemoveBase *pObject);
typedef void destroy_AudioCustParamCache(MtkAudioCustParamCacheBase *pObject);

extern "C" bool MtkAudioLoudEnableXMLCallBack(void);

}; // namespace android

#endif // __AUDIO_COMPONENT_ENGINE_EXP_H__
