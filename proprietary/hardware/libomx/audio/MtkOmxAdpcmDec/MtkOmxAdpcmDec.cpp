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
*   MtkOmxAdpcmDec.cpp
*
* Project:
* --------
*   MT65xx
*
* Description:
* ------------
*   MTK OMX ADPCM Decoder component
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

#include "MtkOmxAdpcmDec.h"

#undef LOG_TAG
#define LOG_TAG "MtkOmxAdpcmDec"

MtkOmxAdpcmDec :: MtkOmxAdpcmDec()
{
    SLOGD("MtkOmxAdpcmDec Construct !!!");
    mInitFlag = OMX_FALSE;
    mBsBufSize = MIN_BS_BUFFER_SIZE;
    mPcmBufSize = 0;

    mInterBufSize = 0;
    mpInterBuf = NULL;
    mpHandle = NULL;

    mFirstFrameFlag = OMX_TRUE;       // for compute time stamp
    mLastSampleCount = 0;

    lastTimeStamp = 0;
    isNewOutputBuffer = OMX_TRUE;

    maxNumFramesInOutputBuf = 5;
    numFramesInOutputBuf = 0;

    totalConsumedBytes = 0;
    memset(&mInputAdpcmParam, 0, sizeof(OMX_AUDIO_ADPCMPARAM));

    fgFlush = OMX_TRUE;
    mFormatChange = OMX_FALSE;

}

MtkOmxAdpcmDec :: ~MtkOmxAdpcmDec()
{
    SLOGD("MtkOmxAdpcmDec Deconstruct !!!");
}

void MtkOmxAdpcmDec::DecodeAudio(OMX_BUFFERHEADERTYPE* pInputBuf, OMX_BUFFERHEADERTYPE* pOutputBuf)
{
    if(pInputBuf == NULL || pOutputBuf == NULL) {
        SLOGE("pInputBuf == NULL or pOutputBuf == NULL !");

        OMX_ERRORTYPE err = OMX_ErrorBadParameter;
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle, mAppData, OMX_EventError, err, 0, NULL);
    } else {
        SLOGV("decode ADPCM bitstream !!!");
        SLOGV("Input Buffer data length is %u, timeStamp is %lld",pInputBuf->nFilledLen, pInputBuf->nTimeStamp);
        SLOGV("Output Buffer data length is %u, timeStamp is %lld",pOutputBuf->nFilledLen, pOutputBuf->nTimeStamp);
        SLOGV("pInputBuf is %p, pOutputBuf is %p, pInputBuf->pBuffer is %p, pOutputBuf->pBuffer is %p",pInputBuf, pOutputBuf, pInputBuf->pBuffer, pOutputBuf->pBuffer);

        if(pInputBuf->nFlags & OMX_BUFFERFLAG_CODECCONFIG) {
            if (initAdpcmDecoder(pInputBuf)) {
                SLOGV("Init ADPCM decoder success !!!");
                handleBuffers(pInputBuf, pOutputBuf, OMX_TRUE, OMX_FALSE);
                if((mOutputPcmMode.nChannels != mInputAdpcmParam.nChannelCount) || (mOutputPcmMode.nSamplingRate != mInputAdpcmParam.nSamplesPerSec)) {
                    SLOGV("ADPCM handle changing output port format !!!");
                    handleFormatChanged(pInputBuf, pOutputBuf);
                }
                isNewOutputBuffer = OMX_TRUE;
            } else {
                SLOGE("Init ADPCM decoder failed !!!");
                FlushAudioDecoder();
                handleBuffers(pInputBuf, pOutputBuf, OMX_TRUE, OMX_TRUE);
                OMX_ERRORTYPE err = OMX_ErrorBadParameter;
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle, mAppData, OMX_EventError, err, 0, NULL);
            }
            return ;
        } else {
            if(fgFlush == OMX_TRUE) {
                fgFlush = OMX_FALSE;
                mFormatChange = OMX_TRUE;
                mPortReconfigInProgress = OMX_TRUE;
                SLOGV("--- OMX_EventPortSettingsChanged ---");
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventPortSettingsChanged,
                                       MTK_OMX_OUTPUT_PORT,
                                       NULL,
                                       NULL);

                // push the input buffer back to the head of the input queue
                SLOGV("[ADPMCDec]push the input buffer back to the head of the input queue!");
                QueueInputBuffer(findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pInputBuf));
                QueueOutputBuffer(findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pOutputBuf));
                return;
            } else {
                mFormatChange = OMX_FALSE;
            }

            if((pInputBuf->nFlags & OMX_BUFFERFLAG_EOS) && (pInputBuf->nFilledLen == 0)) { // with data ?
                SLOGV("ADPCM EOS without data received !!!");

                pOutputBuf->nFlags = OMX_BUFFERFLAG_EOS;
                pOutputBuf->nTimeStamp = lastTimeStamp + (mLastSampleCount + mLastSampleCount >> 1) * 1000000LL / (mOutputPcmMode.nSamplingRate);

                FlushAudioDecoder();
                SLOGV("mNumPendingInput(%d), mNumPendingOutput(%d)", (int)mNumPendingInput, (int)mNumPendingOutput);
                handleBuffers(pInputBuf, pOutputBuf, OMX_TRUE, OMX_TRUE);

                return;
            } else {
                if(isNewOutputBuffer == OMX_TRUE) {
                    ALOGV("isNewOutputBuffer == OMX_TRUE !");

                    //set output buffer timestamp
                    if(OMX_TRUE == mFirstFrameFlag) {
                        pOutputBuf->nTimeStamp = pInputBuf->nTimeStamp;
                        lastTimeStamp = pOutputBuf->nTimeStamp;
                        mFirstFrameFlag = OMX_FALSE;
                    } else {
                        pOutputBuf->nTimeStamp = lastTimeStamp + (mLastSampleCount + mLastSampleCount >> 1) * 1000000LL / (mOutputPcmMode.nSamplingRate);
                        SLOGV("pOutputBuf->nTimeStamp %0.2f, mLastSampleCount %d, mOutputPcmMode.nSamplingRate %d",pOutputBuf->nTimeStamp / 1E6, mLastSampleCount, mOutputPcmMode.nSamplingRate);
                        lastTimeStamp = pOutputBuf->nTimeStamp;
                        mLastSampleCount = 0;
                    }
                }

                void* pBitStreamBufferRead = pInputBuf->pBuffer + pInputBuf->nOffset + totalConsumedBytes;
                void* pPCMBufferWrite = pOutputBuf->pBuffer + pOutputBuf->nOffset + pOutputBuf->nFilledLen;

                OMX_U32 remainingInputBytes = pInputBuf->nFilledLen;

                while((remainingInputBytes > 0) && (numFramesInOutputBuf < maxNumFramesInOutputBuf)
                        && ((pOutputBuf->nAllocLen - pOutputBuf->nOffset - pOutputBuf->nFilledLen) >= MIN_BS_BUFFER_SIZE * 4)) {
                    OMX_U32 mConsumedBytes = 0;
                    unsigned int inputBytes = MIN_BS_BUFFER_SIZE;
                    unsigned int outputBytes = pOutputBuf->nAllocLen - pOutputBuf->nOffset - pOutputBuf->nFilledLen;

                    if(remainingInputBytes < inputBytes) {
                        inputBytes = remainingInputBytes;
                        SLOGV("The rest input bytes of this input buffer is %d", inputBytes);
                    }

                    SLOGV("ADPCM_Decode +++, inputBytes %d, outputBytes %d, pBitStreamBufferRead is 0x%x, pPCMBufferWrite is 0x%x", inputBytes, outputBytes, pBitStreamBufferRead, pPCMBufferWrite);
                    mConsumedBytes = ADPCM_Decode(mpHandle, pBitStreamBufferRead, inputBytes, pPCMBufferWrite, &outputBytes);
                    SLOGV("ADPCM_Decode ---, inputBytes %d, outputBytes %d, mConsumedBytes is %d", inputBytes, outputBytes, mConsumedBytes);

                    pBitStreamBufferRead = (void *)((OMX_U8 *)pBitStreamBufferRead + mConsumedBytes);
                    pPCMBufferWrite = (void *)((OMX_U8 *)pPCMBufferWrite + outputBytes);
                    remainingInputBytes -= mConsumedBytes;

                    pInputBuf->nFilledLen -= mConsumedBytes;
                    pOutputBuf->nFilledLen += outputBytes;
                    numFramesInOutputBuf++;

                    totalConsumedBytes += mConsumedBytes;
                }

                if((numFramesInOutputBuf < maxNumFramesInOutputBuf)
                        && ((pOutputBuf->nAllocLen - pOutputBuf->nOffset - pOutputBuf->nFilledLen) >= MIN_BS_BUFFER_SIZE * 4)) {
                    SLOGV("bitstream size is zero, Output buffer is available !");

                    if(pInputBuf->nFlags & OMX_BUFFERFLAG_EOS) {
                        SLOGV("ADPCM EOS with data received, data has been consumed !");
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
                        mLastSampleCount = (pOutputBuf->nFilledLen / (mOutputPcmMode.nBitPerSample / 8)) >> (mOutputPcmMode.nChannels - 1);
                        handleBuffers(pInputBuf, pOutputBuf, OMX_FALSE, OMX_TRUE);
                    } else {
                        if(pInputBuf->nFlags & OMX_BUFFERFLAG_EOS) {
                            SLOGV("ADPCM EOS with data received, data has been consumed !");
                            FlushAudioDecoder();
                            pOutputBuf->nFlags = OMX_BUFFERFLAG_EOS;
                        }

                        mLastSampleCount = (pOutputBuf->nFilledLen / (mOutputPcmMode.nBitPerSample / 8)) >> (mOutputPcmMode.nChannels - 1);
                        handleBuffers(pInputBuf, pOutputBuf, OMX_TRUE, OMX_TRUE);
                    }
                }
            }
        }
    }
}

void MtkOmxAdpcmDec :: handleFormatChanged(OMX_BUFFERHEADERTYPE* pInputBuffer,OMX_BUFFERHEADERTYPE* pOutputBuffer)
{
    SLOGV("ADPCM handleFormatChanged !!!");
    SLOGV("ADPCM Param channel number is %d, samplingRate is %d", mInputAdpcmParam.nChannelCount, mInputAdpcmParam.nSamplesPerSec);

    mPortReconfigInProgress = OMX_TRUE;
    mOutputPcmMode.nChannels = mInputAdpcmParam.nChannelCount;
    mOutputPcmMode.nSamplingRate = mInputAdpcmParam.nSamplesPerSec;
    mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle, mAppData, OMX_EventPortSettingsChanged, MTK_OMX_OUTPUT_PORT, NULL, NULL);
}

void MtkOmxAdpcmDec :: handleBuffers(OMX_BUFFERHEADERTYPE* pInputBuf,
                                     OMX_BUFFERHEADERTYPE* pOutputBuf,
                                     OMX_BOOL emptyInput, OMX_BOOL fillOutput)
{
    SLOGV("pInputBuf size is %d, pOutputBuf size is %d, timestamp of output buffer is %0.4f s, numFramesInOutputBuf is %d",
          pInputBuf->nFilledLen, pOutputBuf->nFilledLen, pOutputBuf->nTimeStamp / 1E6, numFramesInOutputBuf);
    SLOGV("pInputBuf is %p, pOutputBuf is %p, pInputBuf->pBuffer is %p, pOutputBuf->pBuffer is %p",
          pInputBuf, pOutputBuf, pInputBuf->pBuffer, pOutputBuf->pBuffer);

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

void MtkOmxAdpcmDec :: FlushAudioDecoder()
{
    SLOGV("ADPCM FlushAudioDecoder !!!");

    if(NULL != mpHandle) {
        SLOGV("ADPCM_FlushDecoder !!!");
        ADPCM_FlushDecoder(mpHandle);

    }

    mFirstFrameFlag = OMX_TRUE;
    mLastSampleCount = 0;

    lastTimeStamp = 0;
    isNewOutputBuffer = OMX_TRUE;

    numFramesInOutputBuf = 0;
    totalConsumedBytes = 0;

    if(mFormatChange == OMX_FALSE) {
        fgFlush = OMX_TRUE;
    }
}

void MtkOmxAdpcmDec :: DeinitAudioDecoder()
{
    SLOGD("ADPCM DeinitAudioDecoder");

    if(NULL != mpInterBuf) {
        MTK_OMX_FREE(mpInterBuf);
        mpInterBuf = NULL;
    }

    mpHandle = NULL;

    mPcmBufSize = 0;
    mInterBufSize = 0;

    mInitFlag= OMX_FALSE;
    mFirstFrameFlag = OMX_TRUE;
    mLastSampleCount = 0;
    isNewOutputBuffer = OMX_TRUE;

    numFramesInOutputBuf = 0;
    totalConsumedBytes = 0;

    fgFlush = OMX_TRUE;
}

OMX_BOOL MtkOmxAdpcmDec :: initAdpcmDecoder(OMX_BUFFERHEADERTYPE* pInputBuffer)
{
    SLOGV("Init ADPCM Decoder+++++++++++++++++++!!!");
    OMX_U8* csdBuffer = pInputBuffer->pBuffer + pInputBuffer->nOffset;
    OMX_U32 blockAlign;
    memcpy(&blockAlign, csdBuffer, 4);
    mInputAdpcmParam.nBlockAlign = blockAlign;
    SLOGV("initAdpcmDecoder, blockAlign = %d", blockAlign);
    OMX_U32 versionID = 0;
    versionID = ADPCM_GetDecVersion();
    SLOGV("ADPCM Decode Version is Ox%x", versionID);
    SLOGV("projectType is %d, compilerMajorVersion is %d, compileMinorVersion is %d, ReleaseVersion is %d",
          (versionID & 0xFF000000)>>24,(versionID & 0x00FF0000)>>16,(versionID & 0x0000FF00)<<8,(versionID & 0x000000FF));

    if(mBsBufSize < 256) {
        SLOGE("bitstream size < 256Byte !!!");
        return OMX_FALSE;
    }

    ADPCM_DEC_STATUS err = ADPCM_GetDecBufferSize(MIN_BS_BUFFER_SIZE, (unsigned int*)&mPcmBufSize, (unsigned int*)&mInterBufSize);

    if(ADPCM_DEC_SUCCESS != err) {
        SLOGE("ADPCM get decoder buffer failed !!!");
        return OMX_FALSE;
    }

    mpInterBuf = (OMX_U8 *)MTK_OMX_ALLOC(mInterBufSize);

    if(NULL == mpInterBuf) {
        SLOGE("malloc internel buffer size failed !!!");
        return  OMX_FALSE;
    }

    SLOGV("Init ADPCM Decoder adpcmParam +++!!!");
    ADPCM_DEC_PARAM adpcmParam;
    adpcmParam.format_tag = (mInputAdpcmParam.nFormatTag == WAVE_FORMAT_MS_ADPCM ? FORMAT_DEC_MSADPCM : FORMAT_DEC_DVI_IMAADPCM);
    adpcmParam.channel_num = mInputAdpcmParam.nChannelCount;
    adpcmParam.sample_rate = mInputAdpcmParam.nSamplesPerSec;
    adpcmParam.block_align = mInputAdpcmParam.nBlockAlign;
    adpcmParam.bits_per_sample = mInputAdpcmParam.nBitsPerSample;
    adpcmParam.extra_data_length = mInputAdpcmParam.nExtendDataSize;
    adpcmParam.extra_data = mInputAdpcmParam.pExtendData;

    SLOGV("ADPCM mInputAdpcmParam.nFormatTag is %d", mInputAdpcmParam.nFormatTag);
    SLOGV("ADPCM mInputAdpcmParam.nChannelCount is %d", mInputAdpcmParam.nChannelCount);
    SLOGV("ADPCM mInputAdpcmParam.nSamplesPerSec is %d", mInputAdpcmParam.nSamplesPerSec);
    SLOGV("ADPCM mInputAdpcmParam.nBlockAlign is %d", mInputAdpcmParam.nBlockAlign);
    SLOGV("ADPCM mInputAdpcmParam.nBitsPerSample is %d", mInputAdpcmParam.nBitsPerSample);
    SLOGV("ADPCM mInputAdpcmParam.nExtendDataSize is %d", mInputAdpcmParam.nExtendDataSize);
    SLOGV("ADPCM mInputAdpcmParam.pExtendData is %p", mInputAdpcmParam.pExtendData);

    SLOGV("Init ADPCM Decoder ADPCM_InitDecoder +++!!!");
    mpHandle = ADPCM_InitDecoder((void *)mpInterBuf, &adpcmParam);
    SLOGV("Init ADPCM Decoder ADPCM_InitDecoder ---!!!");

    SLOGV("ADPCM mpHandle is 0x%x", mpHandle);

    if(NULL == mpHandle) {
        SLOGE("ADPCM Init Decoder failed !!!");
        return OMX_FALSE;
    }

    mInitFlag= OMX_TRUE;

    mFirstFrameFlag = OMX_TRUE;
    mLastSampleCount = 0;
    isNewOutputBuffer = OMX_TRUE;
    numFramesInOutputBuf = 0;

    SLOGV("Init ADPCM Decoder----------------------!!!");
    return OMX_TRUE;

}


OMX_ERRORTYPE MtkOmxAdpcmDec :: GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
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
    case OMX_IndexParamAudioAdpcm: {
        SLOGV("GetParameter OMX_IndexParamAudioAdpcm !!!");
        OMX_AUDIO_ADPCMPARAM *pParam = (OMX_AUDIO_ADPCMPARAM *)ComponentParameterStructure;

        if(mInputPortDef.nPortIndex == pParam->nPortIndex) {
            memcpy(pParam, &mInputAdpcmParam, sizeof(OMX_AUDIO_ADPCMPARAM));
        } else {
            err = OMX_ErrorBadPortIndex;
            goto EXIT;
        }

        break;
    }
    case OMX_IndexVendorMtkOmxPartialFrameQuerySupported: {
        OMX_PARAM_U32TYPE *pSupportPartialFrame = (OMX_PARAM_U32TYPE *)ComponentParameterStructure;
        pSupportPartialFrame->nU32 = (OMX_U32)OMX_TRUE;
        SLOGV("ADPCM Support Partial Frame !");
        break;
    }
    default: {
        err = MtkOmxAudioDecBase :: GetParameter(hComponent, nParamIndex, ComponentParameterStructure);
        break;
    }
    }

EXIT:
    return err;
}
OMX_ERRORTYPE MtkOmxAdpcmDec :: SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
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
    case OMX_IndexParamAudioAdpcm: {
        SLOGV("SetParameter OMX_IndexParamAudioAdpcm !!!");
        OMX_AUDIO_ADPCMPARAM *pParam = (OMX_AUDIO_ADPCMPARAM *)ComponentParameterStructure;

        if(mInputPortDef.nPortIndex == pParam->nPortIndex) {
            memcpy(&mInputAdpcmParam, pParam, sizeof(OMX_AUDIO_ADPCMPARAM));
        } else {
            err = OMX_ErrorBadPortIndex;
            goto EXIT;
        }

        break;
    }

    default: {
        err = MtkOmxAudioDecBase :: SetParameter(hComponent, nParamIndex, ComponentParameterStructure);
        break;
    }
    }

EXIT:
    return err;
}



OMX_BOOL MtkOmxAdpcmDec :: InitAudioParams()
{
    SLOGV("Init Audio(ADPCM) Params!!!");

    //Init input port format
    strncpy((char*)mCompRole, "audio_decoder_adpcm", OMX_MAX_STRINGNAME_SIZE -1);
    mCompRole[OMX_MAX_STRINGNAME_SIZE -1] = '\0';
    mInputPortFormat.nSize = sizeof(mInputPortFormat);
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eEncoding = OMX_AUDIO_CodingADPCM;

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
    mInputPortDef.format.audio.cMIMEType = (OMX_STRING)"audio/";
    mInputPortDef.format.audio.bFlagErrorConcealment = OMX_FALSE;
    mInputPortDef.format.audio.eEncoding = OMX_AUDIO_CodingADPCM;

    mInputPortDef.nBufferCountActual = MTK_OMX_NUMBER_INPUT_BUFFER_ADPCM;
    mInputPortDef.nBufferCountMin = 1;
    mInputPortDef.nBufferSize = MTK_OMX_INPUT_BUFFER_SIZE_ADPCM;
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

    mOutputPortDef.nBufferCountActual = MTK_OMX_NUMBER_OUTPUT_BUFFER_ADPCM;
    mOutputPortDef.nBufferCountMin = 1;
    mOutputPortDef.nBufferSize = MTK_OMX_OUTPUT_BUFFER_SIZE_ADPCM;
    mOutputPortDef.bEnabled = OMX_TRUE;
    mOutputPortDef.bPopulated = OMX_FALSE;

    //Init output pcm format
    mOutputPcmMode.nSize = sizeof(mOutputPcmMode);
    mOutputPcmMode.nPortIndex = MTK_OMX_OUTPUT_PORT;
    mOutputPcmMode.nChannels = MTK_OMX_AUDIO_DEFAULT_CHANNEL_NUMBERS;
    mOutputPcmMode.eNumData = OMX_NumericalDataSigned;
    mOutputPcmMode.bInterleaved = OMX_TRUE;
    mOutputPcmMode.nBitPerSample = MTK_OMX_AUDIO_DEFAULT_BITS_PER_SAMPLE;
    mOutputPcmMode.nSamplingRate = MTK_OMX_AUDIO_DEFAULT_SAMPLINGRATE;
    mOutputPcmMode.ePCMMode = OMX_AUDIO_PCMModeLinear;
    mOutputPcmMode.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
    mOutputPcmMode.eChannelMapping[1] = OMX_AUDIO_ChannelRF;

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
    SLOGV("MtkOmxComponentCreate +++ !!!");

    MtkOmxBase* pAdpcmDec = new MtkOmxAdpcmDec();

    if(NULL == pAdpcmDec) {
        SLOGE("new MtkOmxAdpcmDec failed!!!");
        return NULL;
    }

    OMX_COMPONENTTYPE* pAdpcmHandle = pAdpcmDec->GetComponentHandle();

    if(NULL == pAdpcmHandle) {
        SLOGE("GetComponentHandle failed!!!");
        return NULL;
    }

    OMX_ERRORTYPE err = pAdpcmDec->ComponentInit(pAdpcmHandle, componentName);

    if(OMX_ErrorNone != err) {
        SLOGE("ComponentInit failed!!!");
        return NULL;
    }

    return pAdpcmHandle;
}

