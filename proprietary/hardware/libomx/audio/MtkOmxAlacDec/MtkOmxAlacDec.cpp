/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   MtkOmxAlacDec.cpp
 *
 * Project:
 * --------
 *
 *
 * Description:
 * ------------
 *   MTK OMX ALAC Decoder component
 *
 * Author:
 * -------
 *
 *
 ****************************************************************************/

#define MTK_LOG_ENABLE 1
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <dlfcn.h>
#include <cutils/log.h>
#include "osal_utils.h"
#include "MtkOmxAlacDec.h"
#include <cutils/properties.h>

#undef LOG_TAG
#define LOG_TAG "MtkOmxAlacDec"

#define MTK_OMX_ALAC_DECODER "OMX.MTK.AUDIO.DECODER.ALAC"

// Convert 8-bit signed samples to 16-bit signed.
static int PCM8ToPCM16(uint8_t* src, int length)
{
    int16_t* dst = (int16_t*)src;
    int numSamples = length;

    for(int i = numSamples - 1; i >= 0; i--) {
        int16_t x = (int16_t)src[i];
        x = (x <<8) >> 8;
        dst[i] = x;
    }

    return numSamples * 2;
}

// Convert 24-bit signed samples to 16-bit signed.
static int PCM24ToPCM16(uint8_t* src, int length)
{
    if(length % 3) {
        ALOGE("length cannot be divisible by 3");
        return 0;
    }

    int16_t* dst = (int16_t *)src;
    int numSamples = length / 3;

    for(int i = 0; i < numSamples; i++) {
        int32_t x = (int32_t)(src[3 * i] | (src[3 * i + 1] << 8) | (src[3 * i + 2] << 16));
        x = (x << 8) >> 8;  // sign extension

        x = x >> 8;
        dst[i] = (int16_t)x;
    }

    return numSamples * 2;
}

static int PCM32ToPCM16(uint8_t* src, int length)
{
    if(length % 4) {
        ALOGE("length cannot be divisible by 4");
        return 0;
    }

    int numSamples = length / 4;

    for(int i = 0; i < numSamples; i++) {
        src[2*i + 0] = src[4*i + 2];
        src[2*i + 1] = src[4*i + 3];
    }

    return numSamples * 2;
}

// Convert 24-bit signed samples to 32-bit signed.
static int PCM24ToPCM32(uint8_t* src, int length)
{
    if(length % 3) {
        ALOGE("length cannot be divisible by 3");
        return 0;
    }

    int32_t* dst = (int32_t *)src;
    int numSamples = length / 3;

    for(int i = numSamples - 1; i >= 0; i--) {
        int32_t x = (int32_t)(src[3 * i] | (src[3 * i + 1] << 8) | (src[3 * i + 2] << 16));
        x = (x << 8) >> 8;  // sign extension

        dst[i] = x;
    }

    return numSamples * 4;
}

MtkOmxAlacDec::MtkOmxAlacDec()
    : mAlacDecHandle(NULL)
{
    ALOGD("MtkOmxAlacDec::MtkOmxAlacDec this= %p", this);
}

MtkOmxAlacDec::~MtkOmxAlacDec()
{

    ALOGD("+MtkOmxAlacDec::~MtkOmxAlacDec this= %p", this);

    if(mAlacDecHandle != NULL) {
        alac_deinit(mAlacDecHandle);
        MTK_OMX_FREE(mAlacDecHandle);
        mAlacDecHandle = NULL;
    }

    ALOGD("-MtkOmxAlacDec::~MtkOmxAlacDec this= %p", this);
}


OMX_BOOL MtkOmxAlacDec::InitAlacDecoder(OMX_BUFFERHEADERTYPE* pInputBuf)
{

    ALOGD("+InitAlacDecoder");

    if(mAlacDecHandle == NULL) {
        mAlacDecHandle = (alac_file *)MTK_OMX_ALLOC(sizeof(alac_file));
        alac_init(mAlacDecHandle, (uint8_t *)(pInputBuf->pBuffer + pInputBuf->nOffset));
    }

    ALOGD("-InitAlacDecoder");

    return OMX_TRUE;
}

void MtkOmxAlacDec::FlushAudioDecoder()
{
    if(NULL != mAlacDecHandle) {
        ALOGD("FlushAudioDecoder, reset alac decoder");
        alac_reset(mAlacDecHandle);
    }
}

void MtkOmxAlacDec::DeinitAudioDecoder()
{
    ALOGD("+DeinitAudioDecoder(ALAC)");

    if(mAlacDecHandle != NULL) {
        alac_deinit(mAlacDecHandle);
        MTK_OMX_FREE(mAlacDecHandle);
        mAlacDecHandle = NULL;
    }

    ALOGD("-DeinitAudioDecoder(ALAC)");
}

OMX_U32 MtkOmxAlacDec::BitsWidthAdaptTo16(uint8_t* src, int length)
{
    ALOGV("bitWidthAdaptTo16 !");
    OMX_U32 bufferSize = 0;

    if(NULL == src || length <= 0) {
        ALOGE("NULL == src || length <= 0");
        return 0;
    }

    switch(mInputAlacParam.nBitsWidth) {
    case 8:
        bufferSize = PCM8ToPCM16(src, length);
        break;
    case 16:
        bufferSize = length;
        SLOGV("nBitPerSample is %d, not need to handle", mInputAlacParam.nBitsWidth);
        break;
    case 20:
    case 24:
        bufferSize = PCM24ToPCM16(src, length);
        break;
    case 32:
        bufferSize = PCM32ToPCM16(src, length);
        break;
    default:
        ALOGE("nBitPerSample is %d, not supported now", mInputAlacParam.nBitsWidth);
        return 0;
    }

    return bufferSize;
}

void MtkOmxAlacDec::DecodeAudio(OMX_BUFFERHEADERTYPE* pInputBuf, OMX_BUFFERHEADERTYPE* pOutputBuf)
{
    if (pInputBuf == nullptr || pOutputBuf == nullptr) {
        SLOGE("pInputBuf or pOutputBuf is NULL !");
        HandleBuffers(pInputBuf, OMX_TRUE, pOutputBuf, OMX_TRUE);

        OMX_ERRORTYPE err = OMX_ErrorBadParameter;
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle, mAppData, OMX_EventError, err, 0, NULL);
        return;
    }

    ALOGV("In DecodeAudio -- pInputBuf:%p , pOutputBuf :%p", pInputBuf, pOutputBuf);
    ALOGV("In DecodeAudio -- pInputBuf->pBuffer:%p , pOutputBuf->pBuffer :%p", pInputBuf->pBuffer, pOutputBuf->pBuffer);
    ALOGV("In DecodeAudio -- pInputBuf->nOffset =%u,pOutputBuf->nOffset =%u", pInputBuf->nOffset, pOutputBuf->nOffset);
    ALOGV("In DecodeAudio -- pInputBuf->nFilledLen =%u,pOutputBuf->nFilledLen =%u", pInputBuf->nFilledLen, pOutputBuf->nFilledLen);
    ALOGV("In DecodeAudio -- pInputBuf->nTimeStamp =%lld pInputBuf->nFlags 0x%x", pInputBuf->nTimeStamp, pInputBuf->nFlags);

    if((pInputBuf != NULL) && (pInputBuf->nFlags & OMX_BUFFERFLAG_CODECCONFIG)) {
        if(InitAlacDecoder(pInputBuf)) {

            HandleBuffers(pInputBuf, OMX_TRUE, pOutputBuf, OMX_FALSE);

            mPortReconfigInProgress = OMX_TRUE;
            SLOGD("--- OMX_EventPortSettingsChanged ---");
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventPortSettingsChanged,
                                   MTK_OMX_OUTPUT_PORT,
                                   0,
                                   NULL);
        } else {
            OMX_U32 error = OMX_ErrorBadParameter;
            HandleBuffers(pInputBuf, OMX_FALSE, pOutputBuf, OMX_FALSE);
            mSignalledError = OMX_TRUE;
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventError,
                                   error,
                                   0,NULL);
            ALOGE("MtkOmxAlacDec::DecodeAudio Init Failure!");
        }

        return;
    } else { // decode frame

        // If EOS flag has come from the client & there are no more
        // input buffers to decode, send the callback to the client
        if((pInputBuf != NULL) && (pInputBuf->nFlags & OMX_BUFFERFLAG_EOS)) {   // lastest frame
            // flush decoder
            FlushAudioDecoder();
            // return the EOS output buffer
            pOutputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
            pOutputBuf->nTimeStamp = pInputBuf->nTimeStamp;
            HandleBuffers(pInputBuf, OMX_TRUE, pOutputBuf, OMX_TRUE);
            return;
        }

        if(pInputBuf != NULL)  {
            //Decode the buffer
            OMX_U8* pOutBuffer = pOutputBuf->pBuffer + pOutputBuf->nOffset;
            OMX_U8* pInBuffer = pInputBuf->pBuffer + pInputBuf->nOffset;

            uint32_t numBytes = 0;

            alac_decode_frame(mAlacDecHandle, pInBuffer, (void*)pOutBuffer, (int *)&numBytes);
            pOutputBuf->nFilledLen = numBytes;
            ALOGV("pOutputBuf->nFilledLen = %u", pOutputBuf->nFilledLen);

            /*24/32 bit*/
            if(mOutputPcmMode.nBitPerSample == 16) {
                numBytes = BitsWidthAdaptTo16(pOutputBuf->pBuffer + pOutputBuf->nOffset, pOutputBuf->nFilledLen);
                pOutputBuf->nFilledLen = numBytes;
                ALOGV("16bit, pOutputBuf->nFilledLen = %u", pOutputBuf->nFilledLen);
            } else if(mOutputPcmMode.nBitPerSample == 32) {
                if(mInputAlacParam.nBitsWidth == 24 ||
                        mInputAlacParam.nBitsWidth == 20) {
                    numBytes = PCM24ToPCM32(pOutputBuf->pBuffer + pOutputBuf->nOffset, pOutputBuf->nFilledLen);
                    pOutputBuf->nFilledLen = numBytes;
                    ALOGV("24bit, pOutputBuf->nFilledLen = %u", pOutputBuf->nFilledLen);
                }
            }

            // set output buffer timestamp to be the same as input buffer timestamp
            pOutputBuf->nTimeStamp = pInputBuf ->nTimeStamp;

            HandleBuffers(pInputBuf, OMX_TRUE, pOutputBuf, OMX_TRUE);
        }

    }
}

OMX_BOOL MtkOmxAlacDec::HandleBuffers(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BOOL NewInputBufRequired,
                                      OMX_BUFFERHEADERTYPE *pOutputBuf, OMX_BOOL NewOutputBufRequired)
{
    if(pOutputBuf != NULL) {
        if(NewOutputBufRequired) {
            ALOGV("proceedBuffers ---- HandleFillBufferDone: %lld size %d", pOutputBuf->nTimeStamp, pOutputBuf->nFilledLen);
            HandleFillBufferDone(pOutputBuf);
        } else {
            ALOGV("proceedBuffers ----QueueOutputBuffer");
            QueueOutputBuffer(findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pOutputBuf));
        }
    } else {
        return OMX_FALSE;
    }

    if(pInputBuf != NULL) {
        if(NewInputBufRequired) {
            ALOGV("proceedBuffers ---- HandleEmptyBufferDone");
            HandleEmptyBufferDone(pInputBuf);
        } else {
            ALOGV("proceedBuffers ----QueueInputBuffer");
            QueueInputBuffer(findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pInputBuf));
        }
    } else {
        return OMX_FALSE;
    }

    return OMX_TRUE;
}

OMX_BOOL MtkOmxAlacDec::InitAudioParams()
{

    ALOGD("MtkOmxAlacDec::InitAudioParams(ALAC)");
    // init input port format
    strncpy((char*)mCompRole, "audio_decoder.alac", OMX_MAX_STRINGNAME_SIZE - 1);
    mCompRole[OMX_MAX_STRINGNAME_SIZE - 1] = '\0';
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eEncoding = OMX_AUDIO_CodingALAC;

    // init output port format
    mOutputPortFormat.nPortIndex = MTK_OMX_OUTPUT_PORT;
    mOutputPortFormat.nIndex = 0;
    mOutputPortFormat.eEncoding = OMX_AUDIO_CodingPCM;

    // init input port definition
    mInputPortDef.nPortIndex                           = MTK_OMX_INPUT_PORT;
    mInputPortDef.eDir                                 = OMX_DirInput;
    mInputPortDef.eDomain                              = OMX_PortDomainAudio;
    mInputPortDef.format.audio.pNativeRender           = NULL;
    mInputPortDef.format.audio.cMIMEType               = (OMX_STRING)"audio/alac";
    mInputPortDef.format.audio.bFlagErrorConcealment   = OMX_FALSE;
    mInputPortDef.format.audio.eEncoding               = OMX_AUDIO_CodingALAC;

    mInputPortDef.nBufferCountActual                   = MTK_OMX_NUMBER_INPUT_BUFFER_ALAC;
    mInputPortDef.nBufferCountMin                      = 1;
    mInputPortDef.nBufferSize                          = MTK_OMX_INPUT_BUFFER_SIZE_ALAC;
    mInputPortDef.bEnabled                             = OMX_TRUE;
    mInputPortDef.bPopulated                           = OMX_FALSE;

    //<---for conformance test
    mInputPortDef.bBuffersContiguous = OMX_FALSE;
    mInputPortDef.nBufferAlignment   = OMX_FALSE;
    //--->

    // init output port definition
    mOutputPortDef.nPortIndex = MTK_OMX_OUTPUT_PORT;
    mOutputPortDef.eDomain = OMX_PortDomainAudio;
    mOutputPortDef.format.audio.cMIMEType = (OMX_STRING)"raw";
    mOutputPortDef.format.audio.pNativeRender = 0;
    mOutputPortDef.format.audio.bFlagErrorConcealment = OMX_FALSE;
    mOutputPortDef.format.audio.eEncoding = OMX_AUDIO_CodingPCM;
    mOutputPortDef.eDir = OMX_DirOutput;

    mOutputPortDef.nBufferCountActual = MTK_OMX_NUMBER_OUTPUT_BUFFER_ALAC;
    mOutputPortDef.nBufferCountMin = 1;
    mOutputPortDef.nBufferSize = MTK_OMX_OUTPUT_BUFFER_SIZE_ALAC;
    mOutputPortDef.bEnabled = OMX_TRUE;
    mOutputPortDef.bPopulated = OMX_FALSE;

    //<---Donglei for conformance test
    mOutputPortDef.bBuffersContiguous = OMX_FALSE;
    mOutputPortDef.nBufferAlignment   = OMX_FALSE;
    //--->

    // init input alac format
    mInputAlacParam.nPortIndex  = MTK_OMX_INPUT_PORT;
    mInputAlacParam.nChannels   = 2;
    mInputAlacParam.nSampleRate = 48000;
    mInputAlacParam.nBitsWidth  = 16;
    mInputAlacParam.nSamplesPerPakt = 4096;

    // init output pcm format
    mOutputPcmMode.nPortIndex = MTK_OMX_OUTPUT_PORT;
    mOutputPcmMode.nChannels = 2;
    mOutputPcmMode.eNumData = OMX_NumericalDataSigned;
    mOutputPcmMode.bInterleaved = OMX_TRUE;
    mOutputPcmMode.nBitPerSample = 16;
    mOutputPcmMode.nSamplingRate = 48000;
    mOutputPcmMode.ePCMMode = OMX_AUDIO_PCMModeLinear;
    mOutputPcmMode.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
    mOutputPcmMode.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
    mOutputPcmMode.eChannelMapping[2] = OMX_AUDIO_ChannelCF;
    mOutputPcmMode.eChannelMapping[3] = OMX_AUDIO_ChannelLFE;
    mOutputPcmMode.eChannelMapping[4] = OMX_AUDIO_ChannelLS;
    mOutputPcmMode.eChannelMapping[5] = OMX_AUDIO_ChannelRS;
    mOutputPcmMode.eChannelMapping[6] = OMX_AUDIO_ChannelLR;
    mOutputPcmMode.eChannelMapping[7] = OMX_AUDIO_ChannelRR;

    // allocate input buffer headers address array
    mInputBufferHdrs = (OMX_BUFFERHEADERTYPE**)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE*)*mInputPortDef.nBufferCountActual);
    MTK_OMX_MEMSET(mInputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE*)*mInputPortDef.nBufferCountActual);

    // allocate output buffer headers address array
    mOutputBufferHdrs = (OMX_BUFFERHEADERTYPE**)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE*)*mOutputPortDef.nBufferCountActual);
    MTK_OMX_MEMSET(mOutputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE*)*mOutputPortDef.nBufferCountActual);
    return OMX_TRUE;
}


// Note: each MTK OMX component must export 'MtkOmxComponentCreate" to MtkOmxCore
extern "C" OMX_COMPONENTTYPE* MtkOmxComponentCreate(OMX_STRING componentName)
{

    // create component instance
    MtkOmxBase* pOmxAlacDec  = new MtkOmxAlacDec();

    if(NULL == pOmxAlacDec) {
        ALOGE("MtkOmxComponentCreate out of memory!!!");
        return NULL;
    }

    // get OMX component handle
    OMX_COMPONENTTYPE *pHandle = pOmxAlacDec->GetComponentHandle();
    ALOGD("MtkOmxComponentCreate mCompHandle(%p)", pOmxAlacDec);

    // init component
    OMX_ERRORTYPE err = pOmxAlacDec->ComponentInit(pHandle, componentName);

    if(err != OMX_ErrorNone) {
        ALOGE("MtkOmxComponentCreate init failed, error = 0x%x", err);
        pOmxAlacDec->ComponentDeInit((OMX_HANDLETYPE)pHandle);
        pHandle = NULL;
    }

    return pHandle;
}

