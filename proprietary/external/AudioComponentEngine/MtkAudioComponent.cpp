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

#define LOG_TAG  "MtkAudioComponent"
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <utils/Log.h>
#include <utils/String8.h>
#include <assert.h>
#if defined(ENABLE_SRC)
#include "MtkAudioSrc.h"
#endif
#include "MtkAudioBitConverter.h"
#include "MtkAudioLoud.h"
#if defined(ENABLE_DC_REMOVE)
#include "MtkAudioDcRemoval.h"
#endif
namespace android {

extern "C" MtkAudioBitConverterBase *createMtkAudioBitConverter(uint32_t sampling_rate, uint32_t channel_num, BCV_PCM_FORMAT format) {
    return new MtkAudioBitConverter(sampling_rate, channel_num, format);
}

extern "C" MtkAudioSrcBase *createMtkAudioSrc(uint32_t input_SR, uint32_t input_channel_num, uint32_t output_SR, uint32_t output_channel_num, SRC_PCM_FORMAT format) {
#if defined(ENABLE_SRC)
    return new MtkAudioSrc(input_SR, input_channel_num, output_SR, output_channel_num, format);
#else
    (void) input_SR;
    (void) input_channel_num;
    (void) output_SR;
    (void) output_channel_num;
    (void) format;
    return NULL;
#endif
}

extern "C" MtkAudioLoudBase *createMtkAudioLoud(uint32_t eFLTtype, bool bFastTrack) {
    return new MtkAudioLoud(eFLTtype, bFastTrack);
}

extern "C" MtkAudioDcRemoveBase *createMtkDcRemove() {
#if defined(ENABLE_DC_REMOVE)
    return new MtkAudioDcRemove();
#else
    return NULL;
#endif
}

extern "C" MtkAudioCustParamCacheBase *createMtkAudioCustParamCache(void) {
    return MtkAudioCustParamCache::getInstance();
}

extern "C" bool MtkAudioLoudEnableXMLCallBack(void) {
    return MtkAudioLoud::initParser();
}

extern "C" void destroyMtkAudioBitConverter(MtkAudioBitConverterBase *pObject) {
    if (pObject != NULL) {
        delete pObject;
    }
}

extern "C" void destroyMtkAudioSrc(MtkAudioSrcBase *pObject) {
    if (pObject != NULL) {
        delete pObject;
    }
}

extern "C" void destroyMtkAudioLoud(MtkAudioLoudBase *pObject) {
    if (pObject != NULL) {
        delete pObject;
    }
}

extern "C" void destroyMtkAudioDcRemove(MtkAudioDcRemoveBase *pObject) {
    if (pObject != NULL) {
        delete pObject;
    }
}

extern "C" void destroyMtkAudioCustParamCache(MtkAudioCustParamCacheBase *pObject) {
    if (pObject != NULL) {
        // delete pObject;
        MtkAudioCustParamCache::destroyInstance();
    }
}


}//namespace android

