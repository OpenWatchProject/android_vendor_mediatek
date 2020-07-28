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
*   MtkOmxRawDec.cpp
*
* Project:
* --------
*   MT65xx
*
* Description:
* ------------
*   MTK OMX RAW Decoder component
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

#include "MtkOmxRawDec.h"

#undef LOG_TAG
#define LOG_TAG "MtkOmxRawDec"

#define LOGD_IF ALOGD_IF

#define swap16(x)                                                            \
    (__uint16_t)(((__uint16_t)(x) & 0xff) << 8 | ((__uint16_t)(x) & 0xff00) >> 8)

#define swap32(x)                                                            \
     (__uint32_t)(((__uint32_t)(x) & 0xff) << 24 |                            \
     ((__uint32_t)(x) & 0xff00) << 8 | ((__uint32_t)(x) & 0xff0000) >> 8 |    \
     ((__uint32_t)(x) & 0xff000000) >> 24)

#define UNUSED_VARIABLE(x)  (void)(x)

MtkOmxRawDec :: MtkOmxRawDec()
{
    SLOGD("MtkOmxRawDec Construct !!!");
    mInitFlag = OMX_FALSE;

    mFirstFrameFlag = OMX_TRUE;       // for compute time stamp
    mLastSampleCount = 0;

    lastTimeStamp = 0;
    memset(&tempBuffer, 0, sizeof(tempBuffer));
    memset(&tempBufferFor24Bit, 0, sizeof(tempBufferFor24Bit));
    b24OptionFlag = OMX_FALSE;

    mChannelAssign = NONE;
    mValue = -1;

    isNewOutputBuffer = OMX_TRUE;

    maxNumFramesInOutputBuf = 4;
    numFramesInOutputBuf = 0;


    totalConsumedBytes = 0;
    memset(&mRawParamForInput, 0, sizeof(OMX_AUDIO_PARAM_RAWTYPE));

#ifdef ENHANCE_CHANNEL_ASSIGNMENT
// Default sequence is  L_R_C_Lfe_Bl_Br_Bc_Sl_Sr_Ls_Rs_Rls_Rrs
    /*
        ChnAssignFunc[3] =
        {
            {L_R_Ls_Rs_C,     &MtkOmxRawDec::HandleChnAssignFunc1},                 //DVD_AOB
            {L_R_Ls_Rs_C_Lfe, &MtkOmxRawDec::HandleChnAssignFunc2},                 //DVD_AOB
            {L_R_C_Ls_Rs_Lfe, &MtkOmxRawDec::HandleChnAssignFunc3},                 //BD File
            {L_R_C_Ls_Rls_Rrs_Rs_Z,     &MtkOmxRawDec::HandleChnAssignFunc4},       //BD File
            {L_R_C_Ls_Rls_Rrs_Rs_Lfe,     &MtkOmxRawDec::HandleChnAssignFunc5},     //BD File
            {L_R_Ls_Rs_Lfe,     &MtkOmxRawDec::HandleChnAssignFunc6}                //DVD_AOB
        };
    */
    ChnAssignFunc[0].chnAssign = L_R_Ls_Rs_C;
    ChnAssignFunc[0].func = &MtkOmxRawDec::HandleChnAssignFunc1;
    ChnAssignFunc[1].chnAssign = L_R_Ls_Rs_C_Lfe;
    ChnAssignFunc[1].func = &MtkOmxRawDec::HandleChnAssignFunc2;
    ChnAssignFunc[2].chnAssign = L_R_C_Ls_Rs_Lfe;
    ChnAssignFunc[2].func = &MtkOmxRawDec::HandleChnAssignFunc3;
    ChnAssignFunc[3].chnAssign = L_R_C_Ls_Rls_Rrs_Rs_Z;
    ChnAssignFunc[3].func = &MtkOmxRawDec::HandleChnAssignFunc4;
    ChnAssignFunc[4].chnAssign = L_R_C_Ls_Rls_Rrs_Rs_Lfe;
    ChnAssignFunc[4].func = &MtkOmxRawDec::HandleChnAssignFunc4;
    ChnAssignFunc[5].chnAssign = L_R_Ls_Rs_Lfe;
    ChnAssignFunc[5].func = &MtkOmxRawDec::HandleChnAssignFunc1;

    initmChannelAssign();
    registerChnAssignHandler();
#endif

#ifdef CONFIG_MT_ENG_BUILD
    mlog_enable = 1;
#else
    mlog_enable = __android_log_is_loggable(ANDROID_LOG_DEBUG,"MTK_AUDIO_LOG", ANDROID_LOG_INFO);
#endif
}

MtkOmxRawDec :: ~MtkOmxRawDec()
{
    SLOGD("MtkOmxRawDec Deconstruct !!!");

#ifdef ENHANCE_CHANNEL_ASSIGNMENT
    mChnAssignment.clear();
    mHandleChnAssign.clear();
#endif
    b24OptionFlag = OMX_FALSE;
}

void MtkOmxRawDec::DecodeAudio(OMX_BUFFERHEADERTYPE* pInputBuf, OMX_BUFFERHEADERTYPE* pOutputBuf)
{
    OMX_U32 nCopyDataSize = 0;
    OMX_U8 *pCopyData = NULL;

    if(pInputBuf == NULL || pOutputBuf == NULL) {
        SLOGE("pInputBuf == NULL or pOutputBuf == NULL !");
        handleBuffers(pInputBuf, pOutputBuf, OMX_TRUE, OMX_TRUE);

        OMX_ERRORTYPE err = OMX_ErrorBadParameter;
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle, mAppData, OMX_EventError, err, 0, NULL);
    } else {
        SLOGV("decode Raw Pcm data !!!");
        SLOGV("Input Buffer data length is %u, timeStamp is %lld",pInputBuf->nFilledLen, pInputBuf->nTimeStamp);
        SLOGV("Output Buffer data length is %u, timeStamp is %lld",pOutputBuf->nFilledLen, pOutputBuf->nTimeStamp);
        SLOGV("pInputBuf is %p, pOutputBuf is %p, pInputBuf->pBuffer is %p, pOutputBuf->pBuffer is %p", pInputBuf, pOutputBuf, pInputBuf->pBuffer, pOutputBuf->pBuffer);

        if(OMX_FALSE == mInitFlag) {
            initRawDecoder(pInputBuf);
        }

        OMX_U32 bytesPerSample = (mRawParamForInput.nBitPerSample >> 3) * mRawParamForInput.nChannels;

        if(pInputBuf->nFilledLen % bytesPerSample != 0) {
            mSignalledError = OMX_TRUE;
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventError,
                                   OMX_ErrorBadParameter,
                                   0, NULL);
            SLOGE("MtkOmxAacDec::DecodeAudio RawPCM_INVALID_BITSTREAM!");
        }


        if((pInputBuf->nFlags & OMX_BUFFERFLAG_EOS) && (pInputBuf->nFilledLen == 0)) { // without data
            SLOGV("RAW PCM EOS without data received !!!");

            pOutputBuf->nFlags = OMX_BUFFERFLAG_EOS;
            pOutputBuf->nTimeStamp = lastTimeStamp + (mLastSampleCount + mLastSampleCount >> 1) * 1000000LL / (mOutputPcmMode.nSamplingRate);
            FlushAudioDecoder();

            SLOGV("mNumPendingInput(%d), mNumPendingOutput(%d)", (int)mNumPendingInput, (int)mNumPendingOutput);
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
                    pOutputBuf->nTimeStamp = lastTimeStamp + (mLastSampleCount + mLastSampleCount >> 1) * 1000000LL / (mOutputPcmMode.nSamplingRate);
                    SLOGV("pOutputBuf->nTimeStamp %0.2f, mLastSampleCount %d, mOutputPcmMode.nSamplingRate %d",pOutputBuf->nTimeStamp / 1E6, mLastSampleCount, mOutputPcmMode.nSamplingRate);
                    lastTimeStamp = pOutputBuf->nTimeStamp;
                    mLastSampleCount = 0;
                }
            }

            void* pBitStreamBufferRead = pInputBuf->pBuffer + pInputBuf->nOffset + totalConsumedBytes;
            void* pPCMBufferWrite = pOutputBuf->pBuffer + pOutputBuf->nOffset + pOutputBuf->nFilledLen;
            OMX_U32 ninputSize = SAMPLE_NUM_EVERY_DECODE * (mRawParamForInput.nBitPerSample / 8) * mRawParamForInput.nChannels;

            OMX_U32 remainingInputBytes = pInputBuf->nFilledLen;
            OMX_U32 bytesEveryTime = SAMPLE_NUM_EVERY_DECODE * 2 * mOutputPcmMode.nChannels;

            if(OMX_TRUE == b24OptionFlag) {
                if(mRawParamForInput.nBitPerSample == 24) {
                    if(pOutputBuf->nAllocLen / 2 >= bytesEveryTime)
                        bytesEveryTime *= 2;
                }
            }

            while((remainingInputBytes > 0) && (numFramesInOutputBuf < maxNumFramesInOutputBuf)
                    && ((pOutputBuf->nAllocLen - pOutputBuf->nOffset - pOutputBuf->nFilledLen) >= bytesEveryTime)) {
                if(remainingInputBytes < ninputSize) {
                    ninputSize = remainingInputBytes;
                    SLOGV("The rest input bytes of this input buffer is %d", ninputSize);
                }

                //For Big to Little Endian change
                if(mRawParamForInput.nBitPerSample == 8 || mRawParamForInput.eEndian == OMX_EndianLittle) {
                    SLOGV("Do not need to change endian !");
                } else if(mRawParamForInput.eEndian == OMX_EndianBig) {
                    SLOGV("Need to change endian !");
                    BigEndianToLittleEndian((uint8_t *)pBitStreamBufferRead, ninputSize);
                } else {
                    SLOGE("Unknow !");
                }

                //For PCM data unsign -> signed
                if(mRawParamForInput.eNumData == OMX_NumericalDataSigned) {
                    SLOGV("OMX_NumericalDataSigned, Don't need to do unsigned -> signed convert!");
                } else {
                    SLOGV("Do unsigned -> signed convert!");
                    UnsignedToSignedConverter((uint8_t *)pBitStreamBufferRead, ninputSize, mRawParamForInput.nBitPerSample);
                }

                //For Bit Width Change(8/24/32 -> 16)
                OMX_U32 bufferSize = 0;

                if(OMX_TRUE == b24OptionFlag) {
                    if(mRawParamForInput.nBitPerSample == 16 || mRawParamForInput.nBitPerSample == 24) {
                        SLOGV("Not need to change bit width, nBitPerSample is %d", mRawParamForInput.nBitPerSample);
                        bufferSize = ninputSize;

                        if(OMX_TRUE == b24OptionFlag) {
                            if(mRawParamForInput.nBitPerSample == 24) {
                                bufferSize = PCM24ToPCM32((uint8_t *)pBitStreamBufferRead,  ninputSize);
                            }

                            mOutputPcmMode.nBitPerSample = 32;
                        }
                    } else {
                        if(mRawParamForInput.nPcmFormat == 0x3) {
                            SLOGV("Have Changed bit width in extractor, skip."); //only for pcm float
                            bufferSize = ninputSize;
                        } else {
                            SLOGV("Need to change bit width, nBitPerSample is %d, ninputSize is %d line %d", mRawParamForInput.nBitPerSample, ninputSize, __LINE__);
                            bufferSize = BitWidthToPCM16((uint8_t *)pBitStreamBufferRead, ninputSize);
                        }


                        mOutputPcmMode.nBitPerSample = 16;
                    }
                } else {
                    if(mRawParamForInput.nBitPerSample == 16) {
                        SLOGV("Not need to change bit width, nBitPerSample is %d", mRawParamForInput.nBitPerSample);
                        bufferSize = ninputSize;

                        if(OMX_TRUE == b24OptionFlag) {
                            if(mRawParamForInput.nBitPerSample == 24) {
                                bufferSize = PCM24ToPCM32((uint8_t *)pBitStreamBufferRead,  ninputSize);
                            }

                            mOutputPcmMode.nBitPerSample = 32;
                        }
                    } else {
                        if(mRawParamForInput.nPcmFormat == 0x3) {
                            SLOGV("Have Changed bit width in extractor, skip."); //only for pcm float
                            bufferSize = ninputSize;
                        } else {
                            SLOGV("Need to change bit width, nBitPerSample is %d, ninputSize is %d line %d", mRawParamForInput.nBitPerSample, ninputSize, __LINE__);
                            bufferSize = BitWidthToPCM16((uint8_t *)pBitStreamBufferRead, ninputSize);
                        }

                        mOutputPcmMode.nBitPerSample = 16;
                    }
                }

                SLOGV("bufferSize is %d", bufferSize);

                //For channel reassignment
                if(mRawParamForInput.eRawType == PCM_WAVE) {
                    SLOGV("WAVE, Don't need to do channel assignment !");
                } else {
                    SLOGV("Do channel assignment !");

                    if(OMX_TRUE == b24OptionFlag) {
                        if(mRawParamForInput.nBitPerSample == 8) {
                            ChannelAssignment((uint8_t *)tempBuffer.tempBufPointer, bufferSize);
                        }

                        if(mRawParamForInput.nBitPerSample == 24) {
                            ChannelAssignment((uint8_t *)tempBufferFor24Bit.tempBufPointer, bufferSize);
                        } else
                            ChannelAssignment((uint8_t *)pBitStreamBufferRead, bufferSize);
                    } else {
                        if(mRawParamForInput.nBitPerSample == 8) {
                            ChannelAssignment((uint8_t *)tempBuffer.tempBufPointer, bufferSize);
                        } else
                            ChannelAssignment((uint8_t *)pBitStreamBufferRead, bufferSize);

                    }
                }

                //For PCM interleaved or block data
                if(OMX_TRUE == b24OptionFlag) {
                    if(mRawParamForInput.nBitPerSample == 8) {
                        nCopyDataSize = bufferSize;
                        pCopyData = (OMX_U8 *)tempBuffer.tempBufPointer;
                    }

                    if(mRawParamForInput.nBitPerSample == 24) {
                        nCopyDataSize = bufferSize;
                        pCopyData = (OMX_U8 *)tempBufferFor24Bit.tempBufPointer;
                    } else {
                        nCopyDataSize = bufferSize;
                        pCopyData = (uint8_t *)pBitStreamBufferRead;
                    }

                } else {
                    if(mRawParamForInput.nBitPerSample == 8) {
                        nCopyDataSize = bufferSize;
                        pCopyData = (OMX_U8 *)tempBuffer.tempBufPointer;
                    } else {
                        nCopyDataSize = bufferSize;
                        pCopyData = (uint8_t *)pBitStreamBufferRead;
                    }
                }

                memcpy(pPCMBufferWrite, pCopyData, nCopyDataSize);

                pBitStreamBufferRead =
                    (void *)((char *)pBitStreamBufferRead + ninputSize);
                pPCMBufferWrite =
                    (void *)((char *)pPCMBufferWrite + nCopyDataSize);
                remainingInputBytes -= ninputSize;

                pInputBuf->nFilledLen -= ninputSize;
                pOutputBuf->nFilledLen += nCopyDataSize;
                numFramesInOutputBuf++;

                totalConsumedBytes += ninputSize;
            }

            if((numFramesInOutputBuf < maxNumFramesInOutputBuf)
                    && ((pOutputBuf->nAllocLen - pOutputBuf->nOffset - pOutputBuf->nFilledLen) >=  bytesEveryTime)) {
                SLOGV("bitstream size is zero, Output buffer is available !");

                if(pInputBuf->nFlags & OMX_BUFFERFLAG_EOS) {
                    SLOGV("Raw EOS with data received, data has been consumed !");
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
                    mLastSampleCount = (pOutputBuf->nFilledLen / (mOutputPcmMode.nBitPerSample / 8)) / mOutputPcmMode.nChannels;
                    handleBuffers(pInputBuf, pOutputBuf, OMX_FALSE, OMX_TRUE);
                } else {
                    if(pInputBuf->nFlags & OMX_BUFFERFLAG_EOS) {
                        SLOGV("Raw EOS with data received, data has been consumed !");
                        FlushAudioDecoder();
                        pOutputBuf->nFlags = OMX_BUFFERFLAG_EOS;
                    }

                    mLastSampleCount = (pOutputBuf->nFilledLen / (mOutputPcmMode.nBitPerSample / 8)) / mOutputPcmMode.nChannels;
                    handleBuffers(pInputBuf, pOutputBuf, OMX_TRUE, OMX_TRUE);
                }
            }
        }
    }
}

void MtkOmxRawDec :: FlushAudioDecoder()
{
    SLOGD_IF(mlog_enable, "RAW PCM FlushAudioDecoder !!!");

    if(NULL != tempBuffer.tempBufPointer) {
        memset((OMX_U8 *)tempBuffer.tempBufPointer, 0, tempBuffer.bufferSize);
    }

    if(NULL != tempBufferFor24Bit.tempBufPointer) {
        memset((OMX_U8 *)tempBufferFor24Bit.tempBufPointer, 0, tempBufferFor24Bit.bufferSize);
    }


    mFirstFrameFlag = OMX_TRUE;
    mLastSampleCount = 0;
    lastTimeStamp = 0;

    isNewOutputBuffer = OMX_TRUE;
    numFramesInOutputBuf = 0;

    totalConsumedBytes = 0;
}

void MtkOmxRawDec :: DeinitAudioDecoder()
{
    SLOGD_IF(mlog_enable,"RAW PCM DeinitAudioDecoder");

    mInitFlag= OMX_FALSE;
    mFirstFrameFlag = OMX_TRUE;
    mLastSampleCount = 0;
    isNewOutputBuffer = OMX_TRUE;

    numFramesInOutputBuf = 0;
    totalConsumedBytes = 0;

    if(NULL != tempBuffer.tempBufPointer) {
        free(tempBuffer.tempBufPointer);
        tempBuffer.tempBufPointer = NULL;
    }

    if(NULL != tempBufferFor24Bit.tempBufPointer) {
        free(tempBufferFor24Bit.tempBufPointer);
        tempBufferFor24Bit.tempBufPointer = NULL;
    }
}

OMX_BOOL MtkOmxRawDec :: initRawDecoder(OMX_BUFFERHEADERTYPE* pInputBuffer)
{
    SLOGD_IF(mlog_enable,"Init RAW PCM Decoder !!!");
    UNUSED_VARIABLE(pInputBuffer);
    mInitFlag= OMX_TRUE;
    return OMX_TRUE;
}


OMX_ERRORTYPE MtkOmxRawDec :: GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
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
    case OMX_IndexParamAudioRaw: {
        SLOGD_IF(mlog_enable,"GetParameter OMX_IndexParamAudioRaw !!!");

        OMX_AUDIO_PARAM_RAWTYPE *rawParams = (OMX_AUDIO_PARAM_RAWTYPE *)ComponentParameterStructure;

        if(rawParams->nPortIndex != 0 && rawParams->nPortIndex != 1) {
            return OMX_ErrorUndefined;
        }

        rawParams->eNumData = OMX_NumericalDataSigned;
        rawParams->eEndian = OMX_EndianLittle;
        rawParams->bInterleaved = OMX_TRUE;
        rawParams->nBitPerSample = 16;
        rawParams->ePCMMode = OMX_AUDIO_PCMModeLinear;

        rawParams->nChannels = mRawParamForInput.nChannels;
        rawParams->nSamplingRate = mRawParamForInput.nSamplingRate;

        break;
    }
    case OMX_IndexParamAudioPcm: {
        SLOGD_IF(mlog_enable,"GetParameter OMX_IndexParamAudioPcm !!!");

        OMX_AUDIO_PARAM_PCMMODETYPE *pcmParams = (OMX_AUDIO_PARAM_PCMMODETYPE *)ComponentParameterStructure;

        if(mRawParamForInput.nPortIndex == pcmParams->nPortIndex) {
            pcmParams->nChannels = mRawParamForInput.nChannels;
            pcmParams->nSamplingRate = mRawParamForInput.nSamplingRate;
            pcmParams->nBitPerSample = mRawParamForInput.nBitPerSample;
        } else if(mOutputPcmMode.nPortIndex == pcmParams->nPortIndex) {
            memcpy(pcmParams, &mOutputPcmMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
        } else {
            return OMX_ErrorUndefined;
        }

        pcmParams->eNumData = OMX_NumericalDataSigned;
        pcmParams->eEndian = OMX_EndianLittle;
        pcmParams->bInterleaved = OMX_TRUE;
        pcmParams->ePCMMode = OMX_AUDIO_PCMModeLinear;

        break;
    }
    case OMX_IndexVendorMtkOmxPartialFrameQuerySupported: {
        OMX_PARAM_U32TYPE *pSupportPartialFrame = (OMX_PARAM_U32TYPE *)ComponentParameterStructure;
        pSupportPartialFrame->nU32 = (OMX_U32)OMX_TRUE;
        SLOGD_IF(mlog_enable,"RAW Support Partial Frame !");
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
OMX_ERRORTYPE MtkOmxRawDec :: SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
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
    case OMX_IndexParamAudioRaw: {
        SLOGD_IF(mlog_enable,"SetParameter OMX_IndexParamAudioRaw !!!");
        OMX_AUDIO_PARAM_RAWTYPE *pParam = (OMX_AUDIO_PARAM_RAWTYPE *)ComponentParameterStructure;

        if(mInputPortDef.nPortIndex == pParam->nPortIndex) {
            memcpy(&mRawParamForInput, pParam, sizeof(OMX_AUDIO_PARAM_RAWTYPE));

            if(mlog_enable) {
                SLOGD("RAW mRawParamForInput.nChannels is %d", mRawParamForInput.nChannels);
                SLOGD("RAW mRawParamForInput.eEndian is %d, if 0,big endian, if 1,little endian", mRawParamForInput.eEndian == OMX_EndianBig ? 0 : 1);
                SLOGD("RAW mRawParamForInput.nBitPerSample is %d", mRawParamForInput.nBitPerSample);
                SLOGD("RAW mRawParamForInput.nSamplingRate is %d", mRawParamForInput.nSamplingRate);
                SLOGD("RAW mRawParamForInput.eRawType is 0x%x,", (int)mRawParamForInput.eRawType);
                SLOGD("RAW mRawParamForInput.nChannelAssignment is 0x%x", mRawParamForInput.nChannelAssignment);
                SLOGD("RAW mRawParamForInput.eNumData is %s", (mRawParamForInput.eNumData == OMX_NumericalDataSigned) ? "OMX_NumericalDataSigned" : "OMX_NumericalDataUnsigned");
                SLOGD("RAW mRawParamForInput.nPcmFormat is 0x%x", mRawParamForInput.nPcmFormat);
            }

            if(!checkInputParameter(mRawParamForInput.nChannels, mRawParamForInput.eEndian, mRawParamForInput.nBitPerSample, mRawParamForInput.eRawType, mRawParamForInput.eNumData)) {
                SLOGE("input parameters is invailed !");
            }

            mOutputPcmMode.nChannels = mRawParamForInput.nChannels;
            mOutputPcmMode.nBitPerSample = mRawParamForInput.nBitPerSample;
            mOutputPcmMode.nSamplingRate = mRawParamForInput.nSamplingRate;
        } else {
            err = OMX_ErrorBadPortIndex;
            goto EXIT;
        }

        break;
    }

    case OMX_IndexParamAudioPcm: {
        SLOGD_IF(mlog_enable,"SetParameter OMX_IndexParamAudioPcm !!!");
        OMX_AUDIO_PARAM_PCMMODETYPE *pParam = (OMX_AUDIO_PARAM_PCMMODETYPE *)ComponentParameterStructure;

        if(mInputPortDef.nPortIndex == pParam->nPortIndex) {
            memcpy(&mRawParamForInput, pParam, sizeof(OMX_AUDIO_PARAM_RAWTYPE));

            if(mlog_enable) {
                SLOGD("RAW mRawParamForInput.nChannels is %d", mRawParamForInput.nChannels);
                SLOGD("RAW mRawParamForInput.eEndian is %d, if 0,big endian, if 1,little endian", mRawParamForInput.eEndian == OMX_EndianBig ? 0 : 1);
                SLOGD("RAW mRawParamForInput.nBitPerSample is %d", mRawParamForInput.nBitPerSample);
                SLOGD("RAW mRawParamForInput.nSamplingRate is %d", mRawParamForInput.nSamplingRate);
            }

            mRawParamForInput.eRawType = PCM_WAVE;
            mRawParamForInput.nChannelAssignment = 0;
            mRawParamForInput.eNumData = OMX_NumericalDataSigned;
            SLOGD_IF(mlog_enable,"RAW mRawParamForInput.eRawType is 0x%x,", (int)mRawParamForInput.eRawType);
            SLOGD_IF(mlog_enable,"RAW mRawParamForInput.nChannelAssignment is 0x%x", mRawParamForInput.nChannelAssignment);
            SLOGD_IF(mlog_enable,"RAW mRawParamForInput.eNumData is %s", (mRawParamForInput.eNumData == OMX_NumericalDataSigned) ? "OMX_NumericalDataSigned" : "OMX_NumericalDataUnsigned");

            if(!checkInputParameter(mRawParamForInput.nChannels, mRawParamForInput.eEndian, mRawParamForInput.nBitPerSample, mRawParamForInput.eRawType, mRawParamForInput.eNumData)) {
                SLOGE("input parameters is invailed !");
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }

            mOutputPcmMode.nChannels = mRawParamForInput.nChannels;
            mOutputPcmMode.nBitPerSample = mRawParamForInput.nBitPerSample;
            mOutputPcmMode.nSamplingRate = mRawParamForInput.nSamplingRate;
        } else if(mOutputPortDef.nPortIndex == pParam->nPortIndex) {
            err = MtkOmxAudioDecBase :: SetParameter(hComponent, nParamIndex, ComponentParameterStructure);

            if(32 == mOutputPcmMode.nBitPerSample) {
                SLOGD_IF(mlog_enable,"24bit option is Opened");
                b24OptionFlag = OMX_TRUE;
            } else {
                SLOGD_IF(mlog_enable,"24bit option is not Opened");
            }

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

OMX_BOOL MtkOmxRawDec :: checkInputParameter(OMX_U32 channelCount, OMX_ENDIANTYPE eEndian, OMX_U32 bitWidth, OMX_AUDIO_PCMTYPE fileType, OMX_NUMERICALDATATYPE numerical)
{
    SLOGD("checkInputParameter !");
    OMX_BOOL ret = OMX_TRUE;

    if(channelCount < 1 || channelCount > 8) {
        ret = OMX_FALSE;
        return ret;
    }

    if(eEndian != OMX_EndianBig && eEndian != OMX_EndianLittle) {
        ret = OMX_FALSE;
        return ret;
    }

    if(bitWidth != 8 && bitWidth != 16 && bitWidth != 20 && bitWidth != 24 && bitWidth != 32) {
        ret = OMX_FALSE;
        return ret;
    }

    if(fileType != PCM_WAVE && fileType != PCM_DVD_AOB && fileType != PCM_DVD_VOB && fileType != PCM_BD) {
        ret = OMX_FALSE;
        return ret;
    }

    if(numerical != OMX_NumericalDataSigned && numerical != OMX_NumericalDataUnsigned) {
        ret = OMX_FALSE;
        return ret;
    }

    return ret;
}

//mRawParamForInput.nBitPerSample = 16 will not call this function
OMX_U32 MtkOmxRawDec :: BitWidthToPCM16(uint8_t* src, int length)
{
    SLOGV("BitWidthToPCM16 !");
    OMX_U32 bufferSize = 0;

    if(mRawParamForInput.nBitPerSample != 8 && mRawParamForInput.nBitPerSample != 16 && mRawParamForInput.nBitPerSample != 20
            && mRawParamForInput.nBitPerSample != 24 && mRawParamForInput.nBitPerSample != 32) {
        SLOGE("nBitPerSample is %d, not supported now", mRawParamForInput.nBitPerSample);
        return 0;
    }

    if(NULL == src || length <= 0) {
        SLOGE("NULL == src || length <= 0");
        return 0;
    }

    switch(mRawParamForInput.nBitPerSample) {
    case 8:

        if(NULL == tempBuffer.tempBufPointer || tempBuffer.bufferSize < (OMX_U32)length*2) {
            SLOGD("Here need a temp buffer !");

            if(NULL != tempBuffer.tempBufPointer) {
                SLOGD("Free old smaller temp buffer !");
                free(tempBuffer.tempBufPointer);
                tempBuffer.tempBufPointer = NULL;
            }

            tempBuffer.tempBufPointer = (int16_t *)malloc(length * 2);

            if(NULL == tempBuffer.tempBufPointer) {
                SLOGE("Malloc the temp buffer failed !");
                return 0;
            }

            tempBuffer.bufferSize = length*2;
        }

        bufferSize = PCM8ToPCM16(src, tempBuffer.tempBufPointer, length);  //bufferSize is temp buffer data size
        break;
    case 20:
    case 24:
        bufferSize = PCM24ToPCM16(src, length);
        break;
    case 32:
        bufferSize = PCM32ToPCM16(src, length);
        break;
    default:
        SLOGD("nBitPerSample is %d, not need to handle", mRawParamForInput.nBitPerSample);
    }

    return bufferSize;
}

// Convert 8-bit signed samples to 16-bit signed.
OMX_U32 MtkOmxRawDec :: PCM8ToPCM16(uint8_t* src, int16_t* dst, int length)
{
    int temp = length;

    if(NULL == dst) {
        SLOGE("Temp buffer is not exist !");
        return 0;
    }

    while(temp-- > 0) {
//      *dst++ = ((int16_t)(*src) - 128) * 256;   // 8 unsigned -> 8 signed -> 16 signed
        *dst++ = ((int16_t)(*src)) * 256;
        ++src;
    }

    return length * 2;
}

// Convert 24-bit signed samples to 16-bit signed.
OMX_U32 MtkOmxRawDec :: PCM24ToPCM16(uint8_t* src, int length)
{
    if(length % 3) {
        SLOGE("length % 3 is not zero");
        return 0;
    }

    int16_t* dst = (int16_t *)src;
    size_t numSamples = length / 3;

    for(size_t i = 0; i < numSamples; ++i) {
        int32_t x = (int32_t)(src[0] | src[1] << 8 | src[2] << 16);
        x = (x << 8) >> 8;  // sign extension

        x = x >> 8;
        *dst++ = (int16_t)x;
        src += 3;
    }

    return numSamples * 2;
}

OMX_U32 MtkOmxRawDec :: PCM32ToPCM16(uint8_t* src, int length)
{
    if(length % 4) {
        SLOGE("length % 4 is not zero");
        return 0;
    }

    for(int i=0; i<length/4; i++) {
        src[2*i + 0] = src[4*i + 2];
        src[2*i + 1] = src[4*i + 3];
    }

    return length / 2;
}

OMX_U32 MtkOmxRawDec :: PCM24ToPCM32(uint8_t* src, int length)
{
    if(length % 3) {
        SLOGE("length % 3 is not zero");
        return 0;
    }

    if(NULL == tempBufferFor24Bit.tempBufPointer || tempBufferFor24Bit.bufferSize < (OMX_U32)length/3*4) {
        SLOGV("Here need a temp buffer for 24->32 bit !");

        if(NULL != tempBufferFor24Bit.tempBufPointer) {
            SLOGV("Free old smaller temp buffer !");
            free(tempBufferFor24Bit.tempBufPointer);
            tempBufferFor24Bit.tempBufPointer = NULL;
        }

        tempBufferFor24Bit.tempBufPointer = (int16_t *)malloc(length/3*4);

        if(NULL == tempBufferFor24Bit.tempBufPointer) {
            SLOGE("Malloc the temp buffer failed !");
            return 0;
        }

        tempBufferFor24Bit.bufferSize = length/3*4;
        memset(tempBufferFor24Bit.tempBufPointer, 0, tempBufferFor24Bit.bufferSize);
    }

    int32_t *dst = (int32_t *)tempBufferFor24Bit.tempBufPointer;

    for(int i=0; i<length/3; i++) {

        int32_t x = (int32_t)(src[3*i+0] | src[3*i+1] << 8 | src[3*i+2] << 16);
        x = (x << 8) >> 8;  // sign extension
        *dst++ = x;
        /*
                    dst[4*i + 1] = src[3*i + 0];
                    dst[4*i + 2] = src[3*i + 1];
                    dst[4*i + 3] = src[3*i + 2];
        */
    }


    return length/3*4;
}

void MtkOmxRawDec :: BigEndianToLittleEndian(uint8_t* src, int length)
{
    SLOGD_IF(mlog_enable,"BigEndianToLittleEndian !");

    switch(mRawParamForInput.nBitPerSample) {
    case 16: {
        char16_t *framedata16 = (char16_t *) src;
        char16_t *dstdata16 = (char16_t *) src;

        for(int i=0; i<length/2; i++) {
            dstdata16[i] = swap16(framedata16[i]);
        }

        break;
    }
    case 20:
    case 24: {
        uint8_t *framedata24 = src;
        char32_t data = 0;

        for(int i=0; i<length/3; i++) {
            data = 0;
            data = ((char32_t)*(framedata24 + 3*i)) << 16 | ((char32_t)*(framedata24 + 3*i + 1)) << 8 | ((char32_t)*(framedata24 + 3*i + 2));
            *(src + 3*i) = (uint8_t)(data & 0x000000ff);
            *(src + 3*i + 1) = (uint8_t)((data & 0x0000ff00) >> 8);
            *(src + 3*i + 2) = (uint8_t)((data & 0x00ff0000) >> 16);
        }

        break;
    }
    case 32: {
        char32_t *framedata32 = (char32_t *) src;
        char32_t *dstdata32 = (char32_t *) src;

        for(int i=0; i<length/4; i++) {
            dstdata32[i] = swap32(framedata32[i]);
        }

        break;
    }
    default:
        SLOGV("Other bitsPerSample, Don't do big endian to little endian");
    }
}

void MtkOmxRawDec :: UnsignedToSignedConverter(uint8_t* src, int length, int bitWidth)
{
    OMX_U32 bytesPerSample = bitWidth / 8;
    uint8_t *p = src;
    OMX_U32 valueFor24 = 0;
    SLOGV("2^(bitWidth - 1) is %d", 2^(bitWidth - 1));

    switch(bitWidth) {
    case 8:
        SLOGV("bitWidth is 8 !");

        for(int i = 0; i < length/(int)bytesPerSample; i++) {
            *(p + bytesPerSample*i) = *(p + bytesPerSample*i) - 128;
        }

        break;
    case 16:
        SLOGV("bitWidth is 16 !");

        for(int i = 0; i < length /(int)bytesPerSample; i++) {
            *(char16_t *)(p + bytesPerSample*i) = *(char16_t *)(p + bytesPerSample*i) - 32768;
        }

        break;
    case 32:
        SLOGV("bitWidth is 32 !");

        for(int i = 0; i < length /(int)bytesPerSample; i++) {
            *(char32_t *)(p + bytesPerSample*i) = *(char32_t *)(p + bytesPerSample*i) - 2147483648;
        }

        break;
    case 20:
    case 24: {
        SLOGV("bitWidth is 20 or 24 !");

        for(int i = 0; i < length /(int)bytesPerSample; i++) {
            valueFor24 = src[3*i] | src[3*i +1] << 8 | src[3*i + 2] << 16;
            valueFor24 -= 8388608;
            src[3*i] = valueFor24 & 0x00ff;
            src[3*i + 1] = valueFor24 &  0xff00;
            src[3*i + 2] = valueFor24 &  0xff0000;
        }

        break;
    }
    default:
        SLOGE("Unknow bitWidth !");
    }

    return;
}

void MtkOmxRawDec :: ChannelAssignment(uint8_t* src, int length)
{
    uint16_t pointerSize = 0;
    OMX_U32 channelNum = mOutputPcmMode.nChannels;
    OMX_U32 bytesPerSample = mOutputPcmMode.nBitPerSample / 8;
    uint16_t step = channelNum * bytesPerSample;
    uint8_t temp[bytesPerSample*2];

    OMX_BOOL ret = getChannelAssignment();

    if(OMX_FALSE == ret) {
        SLOGE("getChannelAssignment failed !");
        return ;
    }

#ifdef ENHANCE_CHANNEL_ASSIGNMENT
    ssize_t index = mHandleChnAssign.indexOfKey(mChannelAssign);

    if((size_t)index >= mHandleChnAssign.size() || index < 0) {
        SLOGD_IF(mlog_enable,"Not find handler, Do not do channelAssignment !");
        return ;
    } else {
        SLOGD_IF(mlog_enable,"index of mChannelAssign is %zd", index);
        HandleChnAssignFunc handleFunc = mHandleChnAssign.valueAt(index);
        (this->*handleFunc)(src, length);
    }

#else

    switch(mOutputPcmMode.nChannels) {
    case 3:

        switch(mChannelAssign) {
        case L_R_C:
        case L_R_Bc:
        case L_R_Lfe:
            SLOGD("Is already default channel assignment !");
            break;
        case C_L_R:
            //TODO
            break;
        case L_C_R:
            //TODO
            break;
        default:
            SLOGD("");
        }

        break;
    case 4:

        switch(mChannelAssign) {
        case L_R_C_Bc:
        case L_R_Bl_Br:
        case L_R_Sl_Sr:
        case L_R_Ls_Rs:
        case L_R_Lfe_Bc:
        case L_R_C_Lfe:
        case L_R_C_Z:
        case L_R_Bc_Z:
            SLOGD("Is already default channel assignment !");
            break;
        case C_L_R_Ls:
            //TODO
            break;
        case L_C_R_Bc:
            //TODO
            break;
        default:
            SLOGD("");
        }

        break;
    case 5:

        switch(mChannelAssign) {
        case L_R_C_Bl_Br:
        case L_R_C_Sl_Sr:
        case L_R_Lfe_Ls_Rs:
        case L_R_C_Ls_Rs:
        case L_R_C_Lfe_Bc:
            SLOGD("Is already default channel assignment !");
            break;
        case C_L_R_Ls_Rs:
            //TODO
            break;
        case L_C_R_Ls_Rs:
            //TODO
            break;
        case L_R_Ls_Rs_Lfe:  //must do
        case L_R_Ls_Rs_C:

            while(pointerSize < length) {
                memcpy(temp, src + bytesPerSample*2, bytesPerSample*2);
                memcpy(src + bytesPerSample*2, src + bytesPerSample*4, bytesPerSample);
                memcpy(src + bytesPerSample*3, temp, bytesPerSample*2);

                pointerSize += step;
            }

            //TODO
            break;
        default:
            SLOGD("");
        }

        break;
    case 6:

        switch(mChannelAssign) {
        case L_R_C_Lfe_Bl_Br:
        case L_R_C_Lfe_Sl_Sr:
        case L_R_C_Bl_Br_Bc:
        case L_R_C_Bc_Sl_Sr:
        case L_R_C_Ls_Rs_Z:
        case L_R_C_Lfe_Ls_Rs:
            SLOGD("Is already default channel assignment !");
            break;
        case C_L_R_Ls_Rs_Lfe:
            //TODO
            break;
        case L_C_R_Ls_Rs_Lfe:
            //TODO
            break;
        case L_R_Ls_Rs_C_Lfe:  //must do

            while(pointerSize < length) {
                memcpy(temp, src + bytesPerSample*2, bytesPerSample*2);
                memcpy(src + bytesPerSample*2, src + bytesPerSample*4, bytesPerSample*2);
                memcpy(src + bytesPerSample*3, temp, bytesPerSample*2);

                pointerSize += step;
            }

            break;
        case L_R_C_Ls_Rs_Lfe:  //must do

            while(pointerSize < length) {
                memcpy(temp, src + bytesPerSample*3, bytesPerSample*2);
                memcpy(src + bytesPerSample*3, src + bytesPerSample*5, bytesPerSample);
                memcpy(src + bytesPerSample*4, temp, bytesPerSample*2);

                pointerSize += step;
            }

            break;
        default:
            SLOGD("");
        }

        break;
    case 7:

        switch(mChannelAssign) {
        case L_R_C_Bl_Br_Sl_Sr:
            SLOGD("Is already default channel assignment !");
            break;
        default:
            SLOGD("");
        }

        break;
    case 8:

        switch(mChannelAssign) {
        case L_R_C_Lfe_Bl_Br_Sl_Sr:
            SLOGD("Is already default channel assignment !");
            break;
        case C_L_R_Ls_Rs_Rls_Rrs_Lfe:
            //TODO
            break;
        case L_C_R_Ls_Rs_Rls_Rrs_Lfe:
            //TODO
            break;
        case L_R_C_Ls_Rls_Rrs_Rs_Z:
            //TODO
            break;
        case L_R_C_Ls_Rls_Rrs_Rs_Lfe:
            //TODO
            break;
        default:
            SLOGD("");
        }

        break;
    default:
        SLOGE("");
    }

#endif
}

//can use keyedVector to store [value, AUDIO_CHANNEL_ASSIGNMENT]
OMX_BOOL MtkOmxRawDec :: getChannelAssignment()
{
#ifdef ENHANCE_CHANNEL_ASSIGNMENT
    size_t index = findChnAssign(mRawParamForInput.eRawType, mRawParamForInput.nChannelAssignment);

    if(index >= mChnAssignment.size()) {
        SLOGE("Not find available channel assignment !");
        return OMX_FALSE;
    }

    mChannelAssign = getChnAssign(index);
    SLOGD("mChannelAssign is 0x%x", mChannelAssign);
#else

    switch(mRawParamForInput.eRawType) {
    case PCM_DVD_AOB:

        switch(mRawParamForInput.nChannelAssignment) {
        case 0x00010b:
            mChannelAssign = L_R_Bc;
            break;
        case 0x00011b:
            mChannelAssign = L_R_Ls_Rs;
            break;
        case 0x00100b:
            mChannelAssign = L_R_Lfe;
            break;
        case 0x00101b:
            mChannelAssign = L_R_Lfe_Bc;
            break;
        case 0x00110b:
            mChannelAssign = L_R_Lfe_Ls_Rs;
            break;
        case 0x00111b:
            mChannelAssign = L_R_C;
            break;
        case 0x01000b:
            mChannelAssign = L_R_C_Bc;
            break;
        case 0x01001b:
            mChannelAssign = L_R_C_Ls_Rs;
            break;
        case 0x01010b:
            mChannelAssign = L_R_C_Lfe;
            break;
        case 0x01011b:
            mChannelAssign = L_R_Lfe_Bc;
            break;
        case 0x01100b:
            mChannelAssign = L_R_C_Lfe_Ls_Rs;
            break;
        case 0x01101b:
            mChannelAssign = L_R_C_Bc;
            break;
        case 0x01110b:
            mChannelAssign = L_R_C_Ls_Rs;
            break;
        case 0x01111b:
            mChannelAssign = L_R_C_Lfe;
            break;
        case 0x10000b:
            mChannelAssign = L_R_C_Lfe_Bc;
            break;
        case 0x10001b:
            mChannelAssign = L_R_Lfe_Ls_Rs;
            break;
        case 0x10010b:
            mChannelAssign = L_R_Ls_Rs_Lfe;
            break;
        case 0x10011b:
            mChannelAssign = L_R_Ls_Rs_C;
            break;
        case 0x10100b:
            mChannelAssign = L_R_Ls_Rs_C_Lfe;
            break;
        default:
            SLOGD("Not defined in Spec !");
        }

        break;
    case PCM_DVD_VOB:

        switch(mRawParamForInput.nChannelAssignment) {
        case 0x00010b:
            mChannelAssign = L_R_C;
            break;
        case 0x00011b:
            mChannelAssign = L_R_Ls_Rs;
            break;
        case 0x00100b:
            mChannelAssign = L_R_Lfe;
            break;
        case 0x00101b:
            mChannelAssign = L_R_Lfe_Bc;
            break;
        case 0x00110b:
            mChannelAssign = L_R_Lfe_Ls_Rs;
            break;
        case 0x00111b:
            mChannelAssign = L_R_C;
            break;
        case 0x01000b:
            mChannelAssign = L_R_C_Bc;
            break;
        case 0x01001b:
            mChannelAssign = L_R_C_Ls_Rs;
            break;
        case 0x01010b:
            mChannelAssign = L_R_C_Lfe;
            break;
        case 0x01011b:
            mChannelAssign = L_R_C_Lfe_Bc;
            break;
        case 0x01100b:
            mChannelAssign = L_R_C_Lfe_Ls_Rs;
            break;
            //Be Carefull
        case 0x01101b:
            mChannelAssign = L_R_Bc;
            break;
        case 0x01110b:
            mChannelAssign = L_R_Ls_Rs;
            break;
        case 0x01111b:
            mChannelAssign = L_R_Lfe;
            break;
        case 0x10000b:
            mChannelAssign = L_R_Lfe_Bc;
            break;
        case 0x10001b:
            mChannelAssign = L_R_Lfe_Ls_Rs;
            break;
        case 0x10010b:
            mChannelAssign = L_R_C;
            break;
        case 0x10011b:
            mChannelAssign = L_R_C_Bc;
            break;
        case 0x10100b:
            mChannelAssign = L_R_C_Ls_Rs;
            break;
        case 0x10101b:
            mChannelAssign = L_R_C_Lfe;
            break;
        case 0x10110b:
            mChannelAssign = L_R_C_Lfe_Bc;
            break;
        case 0x10111b:
            mChannelAssign = L_R_C_Lfe_Ls_Rs;
            break;
        default:
            SLOGD("Not defined in Spec !");
        }

        break;
    case PCM_BD:

        switch(mRawParamForInput.nChannelAssignment) {
        case 4:
            mChannelAssign = L_R_C_Z;
            break;
        case 5:
            mChannelAssign = L_R_Bc_Z;
            break;
        case 6:
            mChannelAssign = L_R_C_Bc;
            break;
        case 7:
            mChannelAssign = L_R_Ls_Rs;
            break;
        case 8:
            mChannelAssign = L_R_C_Ls_Rs_Z;
            break;
        case 9:
            mChannelAssign = L_R_C_Ls_Rs_Lfe;
            break;
        case 10:
            mChannelAssign = L_R_C_Ls_Rls_Rrs_Rs_Z;
            break;
        case 11:
            mChannelAssign = L_R_C_Ls_Rls_Rrs_Rs_Lfe;
            break;
        default:
            SLOGD("Not defined in Spec !");
        }

        break;
    default:
        SLOGD("Not Supported");
        return OMX_FALSE;
    }

    SLOGD_IF(mlog_enable,"mChannelAssign is 0x%x", mChannelAssign);
#endif
    return OMX_TRUE;
}

#ifdef ENHANCE_CHANNEL_ASSIGNMENT
void MtkOmxRawDec :: addChannelAssign(OMX_AUDIO_PCMTYPE rawType, OMX_U32 value, AUDIO_CHANNEL_ASSIGNMENT chnAssign)
{
    mChnAssignment.push();
    ChannelAssign *chnAss = &mChnAssignment.editItemAt(mChnAssignment.size() - 1);
    chnAss->rawType = rawType;
    chnAss->value = value;
    chnAss->chnAssign = chnAssign;
}

size_t MtkOmxRawDec :: findChnAssign(OMX_AUDIO_PCMTYPE rawType, OMX_U32 value)
{
    SLOGD_IF(mlog_enable,"findChnAssign");

    for(size_t i = 0; i < mChnAssignment.size(); ++i) {
        const ChannelAssign &info = mChnAssignment.itemAt(i);

        if(info.rawType == rawType && info.value == value) {
            return i;
        }
    }

    return -ENOENT;
}

AUDIO_CHANNEL_ASSIGNMENT MtkOmxRawDec :: getChnAssign(size_t index)
{
    if(index >= mChnAssignment.size()) {
        SLOGE("return (AUDIO_CHANNEL_ASSIGNMENT)NULL !");
        return (AUDIO_CHANNEL_ASSIGNMENT)NULL;
    }

    const ChannelAssign &info = mChnAssignment.itemAt(index);

    return info.chnAssign;
}

size_t MtkOmxRawDec :: countAssignNum()
{
    return  mChnAssignment.size();
}

void MtkOmxRawDec :: initmChannelAssign()
{
    SLOGD_IF(mlog_enable,"initmChannelAssign");
    int fileTableSize = sizeof(FileChnAssign) / sizeof(FileChnAssign[0]);
    SLOGD("fileTableSize is %d", fileTableSize);

    for(int i=0; i<fileTableSize; i++) {
        addChannelAssign(FileChnAssign[i].rawType, FileChnAssign[i].value, FileChnAssign[i].chnAssign);
    }

    SLOGD_IF(mlog_enable,"mChnAssignment.size is %d", countAssignNum());
}

void MtkOmxRawDec :: registerChnAssignHandler()
{
    SLOGD_IF(mlog_enable,"registerChnAssignHandler");
    int FuncTableSize = sizeof(ChnAssignFunc) / sizeof(ChnAssignFunc[0]);
    SLOGD_IF(mlog_enable,"FuncTableSize is %d", FuncTableSize);

    for(int i=0; i<FuncTableSize; i++) {
        mHandleChnAssign.add(ChnAssignFunc[i].chnAssign, ChnAssignFunc[i].func);
    }

    SLOGD_IF(mlog_enable,"mHandleChnAssign size is %d", mHandleChnAssign.size());
}

void MtkOmxRawDec :: addChnAssignFunc(AUDIO_CHANNEL_ASSIGNMENT chnAssign, HandleChnAssignFunc func)
{
    SLOGD_IF(mlog_enable,"mHandleChnAssign.size is %d", mHandleChnAssign.size());
    mHandleChnAssign.add(chnAssign, func);

    SLOGD_IF(mlog_enable,"mHandleChnAssign.size is %d", mHandleChnAssign.size());
}

OMX_BOOL MtkOmxRawDec :: HandleChnAssignFunc1(uint8_t* src, int length)
{
    uint16_t pointerSize = 0;
    OMX_U32 channelNum = mOutputPcmMode.nChannels;
    OMX_U32 bytesPerSample = mOutputPcmMode.nBitPerSample / 8;
    uint16_t step = channelNum * bytesPerSample;
    uint8_t temp[bytesPerSample*2];

    while(pointerSize < length) {
        memcpy(temp, src + bytesPerSample*2, bytesPerSample*2);
        memcpy(src + bytesPerSample*2, src + bytesPerSample*4, bytesPerSample);
        memcpy(src + bytesPerSample*3, temp, bytesPerSample*2);

        pointerSize += step;
    }

    return OMX_TRUE;
}

OMX_BOOL MtkOmxRawDec :: HandleChnAssignFunc2(uint8_t* src, int length)
{
    uint16_t pointerSize = 0;
    OMX_U32 channelNum = mOutputPcmMode.nChannels;
    OMX_U32 bytesPerSample = mOutputPcmMode.nBitPerSample / 8;
    uint16_t step = channelNum * bytesPerSample;
    uint8_t temp[bytesPerSample*2];

    while(pointerSize < length) {
        memcpy(temp, src + bytesPerSample*2, bytesPerSample*2);
        memcpy(src + bytesPerSample*2, src + bytesPerSample*4, bytesPerSample*2);
        memcpy(src + bytesPerSample*3, temp, bytesPerSample*2);

        pointerSize += step;
    }

    return OMX_TRUE;
}

OMX_BOOL MtkOmxRawDec :: HandleChnAssignFunc3(uint8_t* src, int length)
{
    uint16_t pointerSize = 0;
    OMX_U32 channelNum = mOutputPcmMode.nChannels;
    OMX_U32 bytesPerSample = mOutputPcmMode.nBitPerSample / 8;
    uint16_t step = channelNum * bytesPerSample;
    uint8_t temp[bytesPerSample*2];

    while(pointerSize < length) {
        memcpy(temp, src + bytesPerSample*3, bytesPerSample*2);
        memcpy(src + bytesPerSample*3, src + bytesPerSample*5, bytesPerSample);
        memcpy(src + bytesPerSample*4, temp, bytesPerSample*2);

        pointerSize += step;
    }

    return OMX_TRUE;
}

OMX_BOOL MtkOmxRawDec :: HandleChnAssignFunc4(uint8_t* src, int length)
{
    uint16_t pointerSize = 0;
    OMX_U32 channelNum = mOutputPcmMode.nChannels;
    OMX_U32 bytesPerSample = mOutputPcmMode.nBitPerSample / 8;
    uint16_t step = channelNum * bytesPerSample;
    uint8_t temp[bytesPerSample*4];

    while(pointerSize < length) {
        memcpy(temp, src + bytesPerSample*3, bytesPerSample*4);
        memcpy(src + bytesPerSample*3, src + bytesPerSample*7, bytesPerSample);
        memcpy(src + bytesPerSample*4, temp, bytesPerSample*4);

        memcpy(temp, src + bytesPerSample*5, bytesPerSample*2);
        memcpy(src + bytesPerSample*5, src + bytesPerSample*7, bytesPerSample);
        memcpy(src + bytesPerSample*6, temp, bytesPerSample*2);

        pointerSize += step;
    }

    return OMX_TRUE;
}
#endif

void MtkOmxRawDec :: handleBuffers(OMX_BUFFERHEADERTYPE* pInputBuf,
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

OMX_BOOL MtkOmxRawDec :: InitAudioParams()
{
    SLOGD("Init Audio(Raw PCM) Params!!!");

    //Init input port format
    strncpy((char*)mCompRole, "audio_decoder_pcm", OMX_MAX_STRINGNAME_SIZE -1);
    mCompRole[OMX_MAX_STRINGNAME_SIZE -1] = '\0';
    mInputPortFormat.nSize = sizeof(mInputPortFormat);
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eEncoding = OMX_AUDIO_CodingPCM;

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
    mInputPortDef.format.audio.cMIMEType = (OMX_STRING)"raw";
    mInputPortDef.format.audio.bFlagErrorConcealment = OMX_FALSE;
    mInputPortDef.format.audio.eEncoding = OMX_AUDIO_CodingPCM;

    mInputPortDef.nBufferCountActual = MTK_OMX_NUMBER_INPUT_BUFFER_RAW;
    mInputPortDef.nBufferCountMin = 1;
    mInputPortDef.nBufferSize = MTK_OMX_INPUT_BUFFER_SIZE_RAW;
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

    mOutputPortDef.nBufferCountActual = MTK_OMX_NUMBER_OUTPUT_BUFFER_RAW;
    mOutputPortDef.nBufferCountMin = 1;
    mOutputPortDef.nBufferSize = MTK_OMX_OUTPUT_BUFFER_SIZE_RAW;
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
    SLOGD("MtkOmxComponentCreate +++ !!!");

    MtkOmxBase* pAdpcmDec = new MtkOmxRawDec();

    if(NULL == pAdpcmDec) {
        SLOGE("new MtkOmxRawDec failed!!!");
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

