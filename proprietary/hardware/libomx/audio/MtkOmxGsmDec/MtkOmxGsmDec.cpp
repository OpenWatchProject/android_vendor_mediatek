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
*   MtkOmxGsmDec.cpp
*
* Project:
* --------
*   MT65xx
*
* Description:
* ------------
*   MTK OMX GSM Decoder component
*
* Author:
* -------
*   We Li(mtk81277)
*
****************************************************************************/
#define MTK_LOG_ENABLE 1

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "osal_utils.h"
#include <dlfcn.h>
#include <cutils/log.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <media/stagefright/foundation/ADebug.h>

#include "MtkOmxGsmDec.h"

#undef LOG_TAG
#define LOG_TAG "MtkOmxGsmDec"

MtkOmxGsmDec :: MtkOmxGsmDec()
{
    SLOGD("MtkOmxGsmDec Construct !!!");
    mInitFlag = OMX_FALSE;

    mFirstFrameFlag = OMX_TRUE;       // for compute time stamp
    mLastSampleCount = 0;

    lastTimeStamp = 0;
    isNewOutputBuffer = OMX_TRUE;

    maxNumFramesInOutputBuf = 1;
    numFramesInOutputBuf = 0;

    totalConsumedBytes = 0;
    mGsm = NULL;
    mSamplingRate = MTK_OMX_GSM_DEFAULT_SAMPLINGRATE;
    mNumChannels  = 1;
}

MtkOmxGsmDec :: ~MtkOmxGsmDec()
{
    SLOGD("MtkOmxGsmDec Deconstruct !!!");
}

//just decode 65bytes every time
int MtkOmxGsmDec :: DecodeGSM(gsm handle, int16_t *out, uint8_t *in, size_t inSize)
{
    int ret = 0;
    CHECK(handle);

    gsm_decode(handle, in, out);
    in += 33;
    inSize -= 33;
    out += 160;
    ret += 160;
    gsm_decode(handle, in, out);
    in += 32;
    inSize -= 32;
    out += 160;
    ret += 160;

    return ret;
}

void MtkOmxGsmDec::DecodeAudio(OMX_BUFFERHEADERTYPE* pInputBuf, OMX_BUFFERHEADERTYPE* pOutputBuf)
{
    if(pInputBuf == NULL || pOutputBuf == NULL) {
        SLOGE("pInputBuf == NULL or pOutputBuf == NULL !");
        handleBuffers(pInputBuf, pOutputBuf, OMX_TRUE, OMX_TRUE);

        OMX_ERRORTYPE err = OMX_ErrorBadParameter;
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle, mAppData, OMX_EventError, err, 0, NULL);
    } else {
        SLOGV("decode GSM bitstream !!!");
        SLOGV("Input Buffer data length is %u, timeStamp is %lld",pInputBuf->nFilledLen, pInputBuf->nTimeStamp);
        SLOGV("Output Buffer data length is %u, timeStamp is %lld",pOutputBuf->nFilledLen, pOutputBuf->nTimeStamp);
        SLOGV("pInputBuf is %p, pOutputBuf is %p, pInputBuf->pBuffer is %p, pOutputBuf->pBuffer is %p",pInputBuf, pOutputBuf, pInputBuf->pBuffer, pOutputBuf->pBuffer);

        if(OMX_FALSE == mInitFlag) {
            if(initGSMDecoder(pInputBuf)) {
                SLOGD("Init GSM decoder success !!!");
                mInitFlag = OMX_TRUE;

                handleBuffers(pInputBuf, pOutputBuf, OMX_FALSE, OMX_FALSE);
                isNewOutputBuffer = OMX_TRUE;
            } else {
                SLOGE("Init GSM decoder failed !!!");
                mInitFlag = OMX_FALSE;

                FlushAudioDecoder();
                handleBuffers(pInputBuf, pOutputBuf, OMX_TRUE, OMX_TRUE);


                OMX_ERRORTYPE err = OMX_ErrorBadParameter;
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle, mAppData, OMX_EventError, err, 0, NULL);
            }

            return ;
        } else {
            if((pInputBuf->nFlags & OMX_BUFFERFLAG_EOS) && (pInputBuf->nFilledLen == 0)) { // with data ?
                SLOGD("GSM EOS without data received !!!");

                pOutputBuf->nFlags = OMX_BUFFERFLAG_EOS;
                pOutputBuf->nTimeStamp = lastTimeStamp + (mLastSampleCount + mLastSampleCount >> 1) * 1000000LL / mSamplingRate;

                FlushAudioDecoder();
                SLOGD("mNumPendingInput(%d), mNumPendingOutput(%d)", (int)mNumPendingInput, (int)mNumPendingOutput);
                handleBuffers(pInputBuf, pOutputBuf, OMX_TRUE, OMX_TRUE);

                return;
            } else {
                if(isNewOutputBuffer == OMX_TRUE) {
                    SLOGV("isNewOutputBuffer == OMX_TRUE !");

                    //set output buffer timestamp
                    if(OMX_TRUE == mFirstFrameFlag) {
                        pOutputBuf->nTimeStamp = pInputBuf->nTimeStamp;
                        lastTimeStamp = pOutputBuf->nTimeStamp;
                        mFirstFrameFlag = OMX_FALSE;
                    } else {
                        pOutputBuf->nTimeStamp = lastTimeStamp + (mLastSampleCount + mLastSampleCount >> 1) * 1000000LL / mSamplingRate;
                        SLOGV("pOutputBuf->nTimeStamp %0.2f, mLastSampleCount %d, nSamplingRate %d",pOutputBuf->nTimeStamp / 1E6, mLastSampleCount, mSamplingRate);
                        lastTimeStamp = pOutputBuf->nTimeStamp;
                        mLastSampleCount = 0;
                    }
                }

                if(((pInputBuf->nFilledLen / 65) * 65) != pInputBuf->nFilledLen) {
                    ALOGE("input buffer not multiple of 65 (%ld).", pInputBuf->nFilledLen);
                }

                void* pBitStreamBufferRead = pInputBuf->pBuffer + pInputBuf->nOffset + totalConsumedBytes;
                void* pPCMBufferWrite = pOutputBuf->pBuffer + pOutputBuf->nOffset + pOutputBuf->nFilledLen;

                OMX_U32 remainingInputBytes = pInputBuf->nFilledLen;

                //65Byte -> 320 samples for 8KHZ
                while((remainingInputBytes > 0) && (numFramesInOutputBuf < maxNumFramesInOutputBuf)
                        && ((pOutputBuf->nAllocLen - pOutputBuf->nOffset - pOutputBuf->nFilledLen) >= PCM_SIZE_FOR_TWO_FRAME)) {
                    OMX_U32 mConsumedHalfWords = 0;

                    SLOGV("GSM_Decode +++, pBitStreamBufferRead is 0x%x, pPCMBufferWrite is 0x%x", pBitStreamBufferRead, pPCMBufferWrite);
                    mConsumedHalfWords = DecodeGSM(mGsm, reinterpret_cast<int16_t *>(pPCMBufferWrite), (uint8_t *)pBitStreamBufferRead, remainingInputBytes);
                    SLOGV("GSM_Decode ---, mConsumedHalfWords is %d", mConsumedHalfWords);

                    pBitStreamBufferRead = (void *)((OMX_U8 *)pBitStreamBufferRead + 65);
                    pPCMBufferWrite       = (void *)((OMX_U8 *)pPCMBufferWrite + PCM_SIZE_FOR_TWO_FRAME);
                    remainingInputBytes -= 65;

                    pInputBuf->nFilledLen -= 65;
                    pOutputBuf->nFilledLen += PCM_SIZE_FOR_TWO_FRAME;
                    numFramesInOutputBuf++;

                    totalConsumedBytes += 65;
                }

                if((numFramesInOutputBuf < maxNumFramesInOutputBuf)
                        && ((pOutputBuf->nAllocLen - pOutputBuf->nOffset - pOutputBuf->nFilledLen) >= PCM_SIZE_FOR_TWO_FRAME)) {
                    SLOGV("bitstream size is zero, Output buffer is available !");

                    if(pInputBuf->nFlags & OMX_BUFFERFLAG_EOS) {
                        SLOGV("GSM EOS with data received, data has been consumed !");
                        FlushAudioDecoder();
                        pOutputBuf->nFlags = OMX_BUFFERFLAG_EOS;

                        handleBuffers(pInputBuf, pOutputBuf, OMX_TRUE, OMX_TRUE);
                    } else {
                        SLOGV("Input buffer data has been consumed !");
                        handleBuffers(pInputBuf, pOutputBuf, OMX_TRUE, OMX_FALSE);
                    }
                } else {
                    SLOGV("Output buffer is full or frame in output buffer is full !");

                    if(remainingInputBytes > 0) {
                        SLOGV("There is stll bitstream size in input buffer");
                        mLastSampleCount = pOutputBuf->nFilledLen / 2;
                        handleBuffers(pInputBuf, pOutputBuf, OMX_FALSE, OMX_TRUE);
                    } else {
                        if(pInputBuf->nFlags & OMX_BUFFERFLAG_EOS) {
                            SLOGV("GSM EOS with data received, data has been consumed !");
                            FlushAudioDecoder();
                            pOutputBuf->nFlags = OMX_BUFFERFLAG_EOS;
                        }

                        mLastSampleCount = pOutputBuf->nFilledLen / 2;
                        handleBuffers(pInputBuf, pOutputBuf, OMX_TRUE, OMX_TRUE);
                    }
                }
            }
        }
    }
}

void MtkOmxGsmDec :: handleBuffers(OMX_BUFFERHEADERTYPE* pInputBuf,
                                   OMX_BUFFERHEADERTYPE* pOutputBuf,
                                   OMX_BOOL emptyInput, OMX_BOOL fillOutput)
{
    SLOGV("pInputBuf size is %d, pOutputBuf size is %d, timestamp of output buffer is %0.4f s, numFramesInOutputBuf is %d",
          pInputBuf->nFilledLen, pOutputBuf->nFilledLen, pOutputBuf->nTimeStamp / 1E6, numFramesInOutputBuf);
    SLOGV("pInputBuf is 0x%x, pOutputBuf is 0x%x, pInputBuf->pBuffer is 0x%x, pOutputBuf->pBuffer is 0x%x",
          (unsigned int)pInputBuf, (unsigned int)pOutputBuf, (unsigned int)pInputBuf->pBuffer, (unsigned int)pOutputBuf->pBuffer);

    if(emptyInput == OMX_TRUE) {
        totalConsumedBytes = 0;
        HandleEmptyBufferDone(pInputBuf);
    } else
        QueueInputBuffer(findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pInputBuf));

    if(fillOutput == OMX_TRUE) {
        isNewOutputBuffer = OMX_TRUE;
        numFramesInOutputBuf = 0;
        HandleFillBufferDone(pOutputBuf);
    } else {
        isNewOutputBuffer = OMX_FALSE;
        QueueOutputBuffer(findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pOutputBuf));
    }
}

void MtkOmxGsmDec :: FlushAudioDecoder()
{
    SLOGD("GSM FlushAudioDecoder !!!");

    mFirstFrameFlag = OMX_TRUE;
    mLastSampleCount = 0;

    lastTimeStamp = 0;
    isNewOutputBuffer = OMX_TRUE;

    numFramesInOutputBuf = 0;
    totalConsumedBytes = 0;

    if(mGsm) {
        gsm_destroy(mGsm);
        mGsm = gsm_create();
        CHECK(mGsm);
        int msopt = 1;
        gsm_option(mGsm, GSM_OPT_WAV49, &msopt);
    } else {
        SLOGD("FlushAudioDecoder: mGsm is NULL");
    }
}

void MtkOmxGsmDec :: DeinitAudioDecoder()
{
    SLOGD("GSM DeinitAudioDecoder");

    //CHECK(mGsm);
    if(mGsm) {
        SLOGD("mGsm is not NULL");
        gsm_destroy(mGsm);
        mGsm = NULL;
    } else
        SLOGD("mGsm is NULL");


    mInitFlag= OMX_FALSE;
    mFirstFrameFlag = OMX_TRUE;
    mLastSampleCount = 0;
    isNewOutputBuffer = OMX_TRUE;

    numFramesInOutputBuf = 0;
    totalConsumedBytes = 0;
}

OMX_BOOL MtkOmxGsmDec :: initGSMDecoder(OMX_BUFFERHEADERTYPE* pInputBuffer)
{
    SLOGD("Init GSM Decoder+++++++++++++++++++!!!");

    mGsm = gsm_create();
    CHECK(mGsm);
    int msopt = 1;
    gsm_option(mGsm, GSM_OPT_WAV49, &msopt);

    mInitFlag= OMX_TRUE;

    mFirstFrameFlag = OMX_TRUE;
    mLastSampleCount = 0;
    isNewOutputBuffer = OMX_TRUE;
    numFramesInOutputBuf = 0;

    SLOGD("Init ADPCM Decoder----------------------!!!");
    return OMX_TRUE;

}


OMX_ERRORTYPE MtkOmxGsmDec :: GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_INDEXTYPE nParamIndex,
        OMX_INOUT OMX_PTR ComponentParameterStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if(NULL == ComponentParameterStructure) {
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    if(mState == OMX_StateInvalid) {
        err = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    switch(nParamIndex) {
    case OMX_IndexParamAudioPcm: {
        OMX_AUDIO_PARAM_PCMMODETYPE *pcmParams =
            (OMX_AUDIO_PARAM_PCMMODETYPE *)ComponentParameterStructure;

        if(pcmParams->nPortIndex > 1) {
            return OMX_ErrorUndefined;
        }

        pcmParams->eNumData = OMX_NumericalDataSigned;
        pcmParams->eEndian = OMX_EndianBig;
        pcmParams->bInterleaved = OMX_TRUE;
        pcmParams->nBitPerSample = 16;
        pcmParams->ePCMMode = OMX_AUDIO_PCMModeLinear;
        pcmParams->eChannelMapping[0] = OMX_AUDIO_ChannelLF;
        pcmParams->eChannelMapping[1] = OMX_AUDIO_ChannelRF;

        pcmParams->nChannels = mNumChannels;
        pcmParams->nSamplingRate = mSamplingRate;

        return OMX_ErrorNone;
    }
    default: {
        err = MtkOmxAudioDecBase :: GetParameter(hComponent, nParamIndex, ComponentParameterStructure);
        break;
    }
    }

EXIT:
    return err;
}
OMX_ERRORTYPE MtkOmxGsmDec :: SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_INDEXTYPE nParamIndex,
        OMX_IN OMX_PTR ComponentParameterStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if(NULL == ComponentParameterStructure) {
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    if(mState == OMX_StateInvalid) {
        err = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    switch(nParamIndex) {
    case OMX_IndexParamAudioPcm: {
        OMX_AUDIO_PARAM_PCMMODETYPE *pcmParams =
            (OMX_AUDIO_PARAM_PCMMODETYPE *)ComponentParameterStructure;

        if(pcmParams->nPortIndex != 0 && pcmParams->nPortIndex != 1) {
            return OMX_ErrorUndefined;
        }

        mSamplingRate = pcmParams->nSamplingRate;
        mNumChannels  = pcmParams->nChannels;

        return OMX_ErrorNone;
    }
    default: {
        err = MtkOmxAudioDecBase :: SetParameter(hComponent, nParamIndex, ComponentParameterStructure);
        break;
    }
    }

EXIT:
    return err;
}



OMX_BOOL MtkOmxGsmDec :: InitAudioParams()
{
    SLOGD("Init Audio(GSM) Params!!!");

    //Init input port format
    strncpy((char*)mCompRole, "audio_decoder_gsm", OMX_MAX_STRINGNAME_SIZE -1);
    mCompRole[OMX_MAX_STRINGNAME_SIZE -1] = '\0';
    mInputPortFormat.nSize = sizeof(mInputPortFormat);
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eEncoding = OMX_AUDIO_CodingGSMFR;

    //Init output port format
    mOutputPortFormat.nSize = sizeof(mOutputPortFormat);
    mOutputPortFormat.nPortIndex = MTK_OMX_OUTPUT_PORT;
    mOutputPortFormat.nIndex = 0;
    mOutputPortFormat.eEncoding = OMX_AUDIO_CodingPCM;

    //Init input port definition
    mInputPortDef.nSize = sizeof(mInputPortDef);
    mInputPortDef.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortDef.eDir = OMX_DirInput;
    mInputPortDef.eDomain = OMX_PortDomainAudio;
    mInputPortDef.format.audio.pNativeRender = NULL;
    mInputPortDef.format.audio.cMIMEType = (OMX_STRING)"audio/gsm";
    mInputPortDef.format.audio.bFlagErrorConcealment = OMX_FALSE;
    mInputPortDef.format.audio.eEncoding = OMX_AUDIO_CodingGSMFR;

    mInputPortDef.nBufferCountActual = MTK_OMX_NUMBER_INPUT_BUFFER_GSM;
    mInputPortDef.nBufferCountMin = 1;
    mInputPortDef.nBufferSize = MTK_OMX_INPUT_BUFFER_SIZE_GSM;
    mInputPortDef.bEnabled = OMX_TRUE;
    mInputPortDef.bPopulated = OMX_FALSE;

    //Init output port definition
    mOutputPortDef.nSize = sizeof(mOutputPortDef);
    mOutputPortDef.nPortIndex = MTK_OMX_OUTPUT_PORT;
    mOutputPortDef.eDir = OMX_DirOutput;
    mOutputPortDef.eDomain = OMX_PortDomainAudio;
    mOutputPortDef.format.audio.pNativeRender = NULL;
    mOutputPortDef.format.audio.cMIMEType = (OMX_STRING)"raw";
    mOutputPortDef.format.audio.bFlagErrorConcealment = OMX_FALSE;
    mOutputPortDef.format.audio.eEncoding = OMX_AUDIO_CodingPCM;

    mOutputPortDef.nBufferCountActual = MTK_OMX_NUMBER_OUTPUT_BUFFER_GSM;
    mOutputPortDef.nBufferCountMin = 1;
    mOutputPortDef.nBufferSize = MTK_OMX_OUTPUT_BUFFER_SIZE_GSM;
    mOutputPortDef.bEnabled = OMX_TRUE;
    mOutputPortDef.bPopulated = OMX_FALSE;


    mInputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC((sizeof(OMX_BUFFERHEADERTYPE *))*mInputPortDef.nBufferCountActual);

    if(NULL == mInputBufferHdrs) {
        SLOGE("malloc input buffer headers failed!!!");
        return OMX_FALSE;
    }

    MTK_OMX_MEMSET(mInputBufferHdrs, 0x00, (sizeof(OMX_BUFFERHEADERTYPE *))*mInputPortDef.nBufferCountActual);
    mOutputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC((sizeof(OMX_BUFFERHEADERTYPE *))*mInputPortDef.nBufferCountActual);

    if(NULL == mOutputBufferHdrs) {
        SLOGE("malloc output buffer headers failed!!!");
        return OMX_FALSE;
    }

    MTK_OMX_MEMSET(mOutputBufferHdrs, 0x00, (sizeof(OMX_BUFFERHEADERTYPE *))*mInputPortDef.nBufferCountActual);

    return OMX_TRUE;
}

extern "C" OMX_COMPONENTTYPE* MtkOmxComponentCreate(OMX_STRING componentName)
{
    SLOGD("MtkOmxComponentCreate +++ !!!");

    MtkOmxBase* pGsmDec = new MtkOmxGsmDec();

    if(NULL == pGsmDec) {
        SLOGE("new MtkOmxGsmDec failed!!!");
        return NULL;
    }

    OMX_COMPONENTTYPE* pGsmHandle = pGsmDec->GetComponentHandle();

    if(NULL == pGsmHandle) {
        SLOGE("GetComponentHandle failed!!!");
        return NULL;
    }

    OMX_ERRORTYPE err = pGsmDec->ComponentInit(pGsmHandle, componentName);

    if(OMX_ErrorNone != err) {
        SLOGE("ComponentInit failed!!!");
        return NULL;
    }

    return pGsmHandle;
}

