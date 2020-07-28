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
 *   MtkOmxMp3Dec.cpp
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX MP3 Decoder component
 *
 * Author:
 * -------
 *   Liwen Tan (mtk80556)
 *   Zhihui Zhang (mtk80712)
 ****************************************************************************/

#define MTK_LOG_ENABLE 1
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <dlfcn.h>
#include <cutils/log.h>
#include "osal_utils.h"
#include "MtkOmxMp3Dec.h"
#include <cutils/properties.h>

#undef LOG_TAG
#define LOG_TAG "MtkOmxMp3Dec"

#define MTK_OMX_MP3_DECODER "OMX.MTK.AUDIO.DECODER.MP3"

#define MTK_1ST_TIME_DECODE_MINIMUM_BUFFERSIZE 4096
#define MP3_MAX_FRAME_SIZE (4096)

//#define MTK_OMX_MP3_DEC_DUMP_PCM_DATA
//#define MTK_OMX_MP3_DEC_DUMP_BITSTREAM_DATA
#define FOR_CTS_TEST
#define ENABLE_XLOG_MtkOmxMp3Dec
#ifdef ENABLE_XLOG_MtkOmxMp3Dec
#undef LOGE
#undef LOGW
#undef LOGI
#undef LOGD
#undef LOGV
#define LOGE SLOGE
#define LOGW SLOGW
#define LOGI SLOGI
#define LOGD SLOGD
#define LOGV SLOGV
#endif


#if 0
#undef LOGV
#define LOGV SLOGD
#endif

#define UNUSED(x) (void)(x);

//MTK_OMX_MP3_LOW_POWER
#define MTKMP3_NUM_OF_FRAMES 1

void MtkOmxMp3Dec::DecodeAudio(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    LOGV("In DecodeAudio -- pInputBuf:%p , pOutputBuf :%p", pInputBuf, pOutputBuf);
    LOGV("In DecodeAudio -- pInputBuf->pBuffer:%p , pOutputBuf->pBuffer :%p", pInputBuf->pBuffer, pOutputBuf->pBuffer);
    LOGV("In DecodeAudio -- pInputBuf->nOffset =%lu,pOutputBuf->nOffset =%lu", pInputBuf->nOffset, pOutputBuf->nOffset);
    LOGV("In DecodeAudio -- pInputBuf->nFilledLen =%lu,pOutputBuf->nFilledLen =%lu", pInputBuf->nFilledLen, pOutputBuf->nFilledLen);
    LOGV("JB In DecodeAudio -- pInputBuf->nTimeStamp =%lld,pOutputBuf->nTimeStamp =%lld", pInputBuf->nTimeStamp, pOutputBuf->nTimeStamp);

    OMX_U8 *pBitStreamBuf = pInputBuf->pBuffer + pInputBuf->nOffset;
    OMX_U32 nBitStreamSize = pInputBuf->nFilledLen;
    OMX_U8 *pPcmBuffer = pOutputBuf->pBuffer + pOutputBuf->nOffset;

    if (mMp3InitFlag == OMX_FALSE)
    {
        if (InitMp3Decoder(pBitStreamBuf))
        {
            // push the input buffer back to the head of the input queue
            QueueInputBuffer(findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pInputBuf));
            QueueOutputBuffer(findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pOutputBuf));


        }
        else
        {

            FlushAudioDecoder();//
            mSeekFlagFrameCount = 0;//Seek Flag set 0
            OMX_U32 error = OMX_ErrorBadParameter;
            QueueInputBuffer(findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pInputBuf));
            QueueOutputBuffer(findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pOutputBuf));
            mSignalledError = OMX_TRUE;
            //bitstream error, discard the current data as it can't be decoded further
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventError,
                                   error,
                                   0, NULL);

            LOGE("--- Init Mp3 Decoder Fail! ---");
        }

        return;
    }
    else
    {
        if (pInputBuf->nFlags & OMX_BUFFERFLAG_EOS)
        {
            mEndFlag = OMX_TRUE;//EOS Flag
        }

        int i = 0;

        if (mEndFlag && pInputBuf->nFilledLen <= 0)
        {

            if(isFirst)
            {
                mIsEndOfStream = OMX_TRUE;
            }
            else if ((pOutputBuf->nAllocLen - pOutputBuf->nOffset - pOutputBuf->nFilledLen) >= (kPVMP3DecoderDelay * mInputMp3Param.nChannels * sizeof(int16_t)))
            {
                memset(pOutputBuf->pBuffer + pOutputBuf->nOffset + pOutputBuf->nFilledLen, 0, (kPVMP3DecoderDelay * mInputMp3Param.nChannels * sizeof(int16_t)));
                pOutputBuf->nFilledLen += (kPVMP3DecoderDelay * mInputMp3Param.nChannels * sizeof(int16_t));
                mIsEndOfStream = OMX_TRUE;
            }
            else
            {
                mNewOutBufferRequired = OMX_TRUE;
                mNewInBufferRequired = OMX_FALSE;
            }
        }
        else
        {

            OMX_S32 ulBytesConsumed = 0;
            OMX_U32 ulTotalSamplesOut = 0;
            OMX_U32 ulNumSamplesOut = 0;
            OMX_U8 *pBitStreamRead ;
            OMX_BOOL DecodeReturn = OMX_TRUE;
            mEOSAfterSeek = OMX_FALSE;


            //          pPcmBuffer = pOutputBuf->pBuffer + pOutputBuf->nFilledLen;
#ifdef FOR_CTS_TEST
            pPcmBuffer = pOutputBuf->pBuffer +  pOutputBuf->nOffset + pOutputBuf->nFilledLen;
#endif

            if (mNewInBufferRequired) //new input buffer ,set position and size again
            {
                mBufferLength = nBitStreamSize;
                pBitStreamRead = pBitStreamBuf;
                mInSampleCount = 0;
                LOGV("new input buffer pointer %p length %lu !", pBitStreamRead, mBufferLength);
            }
            else  //
            {
                pBitStreamRead = pLastBitStreamRead;//old input buffer,set last decode position
                LOGV("old input buffer,set last decode position pLastBitStreamRead %p", pLastBitStreamRead);
            }

            //set output buffer timestamp
            //mLastSampleCount is sample number contained in last output buffer;
            if (mNewOutBufferRequired || pOutputBuf->nFilledLen == 0)
            {
                if(OMX_TRUE == mFirstFrameFlag)
                {
                    pOutputBuf->nTimeStamp = pInputBuf->nTimeStamp;
                    lastInputTimeStamp = pInputBuf->nTimeStamp;
                    lastTimeStamp = pOutputBuf->nTimeStamp;
                    mFirstFrameFlag = OMX_FALSE;
                }
                else if(lastInputTimeStamp != pInputBuf->nTimeStamp)
                {
                    LOGV("pOutputBuf->nTimeStamp %lld, lastInputTimeStamp %lld, pInputBuf->nTimeStamp %lld",
                            pOutputBuf->nTimeStamp,lastInputTimeStamp, pInputBuf->nTimeStamp);
                    pOutputBuf->nTimeStamp = pInputBuf->nTimeStamp;
                    lastInputTimeStamp = pInputBuf->nTimeStamp;
                    lastTimeStamp = pOutputBuf->nTimeStamp;
                    mLastSampleCount = 0;
                }
                else
                {
                    lastInputTimeStamp = pInputBuf->nTimeStamp;
                    pOutputBuf->nTimeStamp = lastTimeStamp + (mLastSampleCount) * 1000000LL / (mOutputPcmMode.nSamplingRate);
                    LOGV("pOutputBuf->nTimeStamp %lld, lastTimeStamp %lld, mLastSampleCount %u, mOutputPcmMode.nSamplingRate %d",
                            pOutputBuf->nTimeStamp, lastTimeStamp,mLastSampleCount, mOutputPcmMode.nSamplingRate);
                    lastTimeStamp = pOutputBuf->nTimeStamp;
                    mLastSampleCount = 0;
                }
            }
            /*
            // set output buffer timestamp to be the same as input buffer timestamp
            if (mNewOutBufferRequired || pOutputBuf->nFilledLen == 0)
            {
                pOutputBuf->nTimeStamp = pInputBuf->nTimeStamp + ((OMX_TICKS)mInSampleCount * 1000000LL) / mOutputPcmMode.nSamplingRate;
                LOGV("New pOutputBuf->nTimeStamp = %lld ,pInputBuf->nTimeStamp=%lld ,mInSampleCount=%d mOutputPcmMode.nSamplingRate=%d", pOutputBuf->nTimeStamp, pInputBuf->nTimeStamp, mInSampleCount, mOutputPcmMode.nSamplingRate);
                mLastTimeStamp = pOutputBuf->nTimeStamp;
            }
*/
            OMX_U32 header = U32_AT(pBitStreamRead);
            if ((header & 0xffe00000) != 0xffe00000) {
                ALOGD("invalid header sync word");
                QueueInputBuffer(findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pInputBuf));
                QueueOutputBuffer(findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pOutputBuf));
                mSignalledError = OMX_TRUE;
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                    mAppData,
                                    OMX_EventError,
                                    OMX_ErrorUndefined,
                                    0, NULL);
                return;
            }

            while (nBitStreamSize > 0)
            {
                LOGV("pBitStreamBuf=%p,pBitStreamRead=%p,pBitStreamRead[0]=0x%x,pBitStreamRead[1]=0x%x,pBitStreamRead[2]=0x%x,pBitStreamRead[3]=0x%x,pBitStreamRead[4]=0x%x", pBitStreamBuf, pBitStreamRead, pBitStreamRead[0], pBitStreamRead[1], pBitStreamRead[2], pBitStreamRead[3], pBitStreamRead[4]);
                //to handle Format change case
                int sample_rate, num_channels;
                bool isFormatChange = isMp3FormatChanged(pBitStreamRead, &sample_rate, &num_channels);
                if (isFormatChange) {
                    handleFormatChanged(pBitStreamRead, sample_rate, num_channels);
                    break;
                }

                //to decode mp3 frame to pcm
                ulBytesConsumed = MP3Dec_Decode(mMp3Dec->handle, (void *)pPcmBuffer, (void *)pBitStreamBuf, (int)mBufferLength, pBitStreamRead);
                LOGV("i=%d,ulBytesConsumed=%ld,mBufferLength=%lu,mMp3Dec->handle->PCMSamplesPerCH=%lu mMp3Dec->handle->CHNumber=%d", i++, ulBytesConsumed, mBufferLength, mMp3Dec->handle->PCMSamplesPerCH, mMp3Dec->handle->CHNumber);
                nBitStreamSize -= ulBytesConsumed;

                if (ulBytesConsumed <= 0 ||  nBitStreamSize > 0x7fffffff)
                {
                    ErrorDeclare(ulBytesConsumed);
                    OMX_U32 SilenceLength = (mMp3Dec->handle->PCMSamplesPerCH) << (mMp3Dec->handle->CHNumber - 1);
                    MTK_OMX_MEMSET(pPcmBuffer, 0, SilenceLength * (sizeof(OMX_U16)));
                    pInputBuf->nFilledLen = 0;
                    nBitStreamSize = pInputBuf->nFilledLen;
                    DecodeReturn = OMX_FALSE;
                }
                else
                {
                    pBitStreamRead += ulBytesConsumed;
                    pLastBitStreamRead = pBitStreamRead;
                }

                if ((mMp3Dec->handle->sampleRateIndex == -1) || (mMp3Dec->handle->CHNumber == -1) || (mMp3Dec->handle->PCMSamplesPerCH == 0))
                {
                    LOGV("MP3DEC_SKIP_ERROR_FRAME");

                    if (OMX_FALSE == DecodeReturn)
                    {
                        mNewOutBufferRequired = OMX_TRUE;
                        continue;
                    }
                    else
                    {
                        mNewOutBufferRequired = OMX_FALSE;
                        break;
                    }
                }
                else
                {
                    mOutputPcmMode.nSamplingRate = Get_sample_rate(mMp3Dec->handle->sampleRateIndex);
                    mOutputNum++;
                }

                mInSampleCount += mMp3Dec->handle->PCMSamplesPerCH;
                ulNumSamplesOut = (mMp3Dec->handle->PCMSamplesPerCH) << (mMp3Dec->handle->CHNumber - 1);
                mOutputFrameLength = ulNumSamplesOut << 1;
                //Output length for a buffer of OMX_U8* will be double as that of OMX_S16*
                pOutputBuf->nFilledLen += (ulNumSamplesOut << 1);
                //offset not required in our case, set it to zero
                //              pOutputBuf->nOffset = 0;

#ifdef FOR_CTS_TEST

                if (isOffsetFirst)
                {
                    isOffsetFirst = false;
                    pOutputBuf->nOffset = kPVMP3DecoderDelay * mInputMp3Param.nChannels * sizeof(int16_t);
                }

                if (isFirst)
                {
                    ALOGD("isFirst = true");
                    isFirst = false;
                    if (pOutputBuf->nFilledLen > pOutputBuf->nOffset) {
                        pOutputBuf->nFilledLen = pOutputBuf->nFilledLen - pOutputBuf->nOffset;
                    } else {
                        pOutputBuf->nFilledLen = 0;
                    }
                }

#endif

                LOGV("mOutputNum=%ld,pOutputBuf->nAllocLen =%lu, pOutputBuf->nFilledLen=%lu ,mOutputFrameLength=%lu", mOutputNum, pOutputBuf->nAllocLen , pOutputBuf->nFilledLen , mOutputFrameLength);

                //Send the output buffer back when it has become full
                if ((pOutputBuf->nAllocLen - (pOutputBuf->nFilledLen + pOutputBuf->nOffset) < mOutputFrameLength) || mOutputNum >= mMP3Config.nU32)
                {
                    mNewOutBufferRequired = OMX_TRUE;
                    mOutputNum = 0;
                    LOGV("ulBytesConsumed=%ld, nBitStreamSize=%lu, ulNumSamplesOut=%lu", ulBytesConsumed, nBitStreamSize, ulNumSamplesOut);
                    break;
                }
                else
                {
                    pPcmBuffer = pOutputBuf->pBuffer + pOutputBuf->nFilledLen + pOutputBuf->nOffset; //outputbuffer move next position

                    if (OMX_FALSE == DecodeReturn)
                    {
                        mOutputNum = 0;
                        mNewOutBufferRequired = OMX_TRUE;
                    }
                    else
                    {
                        mNewOutBufferRequired = OMX_FALSE;
                    }
                }

                LOGV("ulBytesConsumed=%ld, nBitStreamSize=%lu, ulNumSamplesOut=%lu", ulBytesConsumed, nBitStreamSize, ulNumSamplesOut);
            }

            if (nBitStreamSize <= 0 || nBitStreamSize > 0x7fffffff)
            {
                pInputBuf->nFilledLen = 0;

                if (mEndFlag)
                {
                    mNewOutBufferRequired = OMX_TRUE;
                    mNewInBufferRequired = OMX_FALSE;
                }
                else
                {
                    mNewInBufferRequired = OMX_TRUE;
                }
            }
            else
            {
                pInputBuf->nFilledLen = nBitStreamSize;
                mNewInBufferRequired = OMX_FALSE;
                lastInputTimeStamp = pInputBuf->nTimeStamp;
                lastTimeStamp = pOutputBuf->nTimeStamp;
                LOGV("pInputBuf->nFilledLen = %lu", pInputBuf->nFilledLen);
            }
        }

        if (mIsEndOfStream) //endofstream
        {
            LOGD("MP3 EOS received, TS=%lld", pInputBuf->nTimeStamp);
            mSeekFlagFrameCount = 0;//Seek Flag set 0
            // return the EOS output buffer
            pOutputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
            pOutputBuf->nTimeStamp = lastTimeStamp;//pInputBuf->nTimeStamp;

            if (OMX_TRUE == mEOSAfterSeek)
            {
                pOutputBuf->nFilledLen = 0;
            }


            HandleFillBufferDone(pOutputBuf);
            HandleEmptyBufferDone(pInputBuf);
            LOGV("mNumPendingInput(%d), mNumPendingOutput(%d)", (int)mNumPendingInput, (int)mNumPendingOutput);
            mEndFlag = OMX_FALSE;
            mIsEndOfStream = OMX_FALSE;
            // flush decoder
            FlushAudioDecoder();//if lowpower ,don't init decoder again
            return;
        }



        if (mNewOutBufferRequired)
        {
            LOGV("DecodeAudio ---- HandleFillBufferDone: %lld", pOutputBuf->nTimeStamp);

            mLastSampleCount = pOutputBuf->nFilledLen / (mMp3Dec->handle->CHNumber * 2);
            LOGV("MP3 mLastSampleCount is %d, ChannelCount is %d", mLastSampleCount, mMp3Dec->handle->CHNumber);

            HandleFillBufferDone(pOutputBuf);
        }
        else
        {
            QueueOutputBuffer(findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pOutputBuf));
            LOGV("DecodeAudio ----QueueOutputBuffer");
        }

        if (mNewInBufferRequired)
        {
            LOGV("DecodeAudio ---- HandleEmptyBufferDone");
            HandleEmptyBufferDone(pInputBuf);
        }
        else
        {
            LOGV("DecodeAudio ----QueueInputBuffer");
            QueueInputBuffer(findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pInputBuf));
        }
    }


}

void MtkOmxMp3Dec::RampUp(OMX_S16 *RampBuff, OMX_U32 HalfRampSample)
{
    LOGV("RampUp HalfRampSample=%lu", HalfRampSample);
    static const unsigned int RAMP_SHIFT_BIT =  16;
    static const unsigned int RAMP_FRACTION = (65536 / HalfRampSample);
    unsigned int mRamp_Fraction = 0, i = 0;

    for (i = 0; i < HalfRampSample; i++)
    {
        RampBuff[i << 1] = RampBuff[i << 1] * mRamp_Fraction >> RAMP_SHIFT_BIT;
        RampBuff[(i << 1) + 1] = RampBuff[(i << 1) + 1] * mRamp_Fraction >> RAMP_SHIFT_BIT;
        mRamp_Fraction +=  RAMP_FRACTION;
    }
}

void MtkOmxMp3Dec::ErrorDeclare(OMX_S32 ulBytesConsumed)
{
    mp3DecErrorCode errCode = (mp3DecErrorCode)(ulBytesConsumed & 0x7);

    switch (errCode)
    {
        case MP3DEC_HANDLE_NULL:
            LOGE("MP3Dec_Decode>> Input pointer handle is NULL pointer!!!");
            break;

        case MP3DEC_PCMBUF_NULL:
            LOGE("MP3Dec_Decode>> Input pointer pPCM_BUF is NULL pointer!!!");
            break;

        case MP3DEC_PCMBUF_NOTALIGN:
            LOGE("MP3Dec_Decode>> Input pointer pPCM_BUF is not 4-byte aligned!!!");
            break;

        //case MP3DEC_BSBUF_NULL:
            //LOGE("MP3Dec_Decode>> Input pointer pBS_BUF is NULL pointer!!!");
            //break;

        case MP3DEC_BSREAD_NULL:
            LOGE("MP3Dec_Decode>> Input pointer pBS_Read is NULL pointer!!!");
            break;

        case MP3DEC_BSBUF_SIZE_INVALID:
            LOGE("MP3Dec_Decode>> BS_BUF_size must be larger than 0!!!");
            break;

        default:
            break;
    }
}

bool MtkOmxMp3Dec::InitMp3Decoder(OMX_U8 *pInputBuffer)
{
    UNUSED(pInputBuffer);

    if (mMp3InitFlag == OMX_FALSE)
    {
        LOGD("JB +InitMp3Decoder");
        mMp3Dec = (mp3DecEngine *)MTK_OMX_ALLOC(sizeof(mp3DecEngine));
        MTK_OMX_MEMSET(mMp3Dec, 0, sizeof(mp3DecEngine));

        MP3Dec_GetMemSize(&mMp3Dec->min_bs_size, &mMp3Dec->pcm_size, &mMp3Dec->workingbuf_size1, &mMp3Dec->workingbuf_size2);
        LOGV("InitMp3Decoder>> min_bs_size=%u, pcm_size=%u, workingbuf_size1=%u,workingbuf_size2=%u",
             mMp3Dec->min_bs_size, mMp3Dec->pcm_size, mMp3Dec->workingbuf_size1, mMp3Dec->workingbuf_size2);

        mMp3Dec->working_buf1 = MTK_OMX_ALLOC(mMp3Dec->workingbuf_size1);
        mMp3Dec->working_buf2 = MTK_OMX_ALLOC(mMp3Dec->workingbuf_size2);

        if ((NULL == mMp3Dec->working_buf1) || (NULL == mMp3Dec->working_buf2))
        {
            LOGE("InitMp3Decoder>> Get working buffer for mp3 decoder failed!!!");
            return OMX_FALSE;
        }

        MTK_OMX_MEMSET(mMp3Dec->working_buf1, 0, mMp3Dec->workingbuf_size1);
        MTK_OMX_MEMSET(mMp3Dec->working_buf2, 0, mMp3Dec->workingbuf_size2);

        LOGV("Init MP3 Decoder");

        if (mMp3Dec->handle == NULL)
        {
            mMp3Dec->handle = MP3Dec_Init(mMp3Dec->working_buf1, mMp3Dec->working_buf2);

            if (mMp3Dec->handle == NULL)
            {
                LOGD("Create mp3 decoder handle error");

                if (mMp3Dec->working_buf1)
                {
                    MTK_OMX_FREE(mMp3Dec->working_buf1);
                    mMp3Dec->working_buf1 = NULL;
                }

                if (mMp3Dec->working_buf2)
                {
                    MTK_OMX_FREE(mMp3Dec->working_buf2);
                    mMp3Dec->working_buf2 = NULL;
                }

                MTK_OMX_FREE(mMp3Dec);
                mMp3Dec = NULL;
                return OMX_FALSE;
            }
        }

        mMp3InitFlag = OMX_TRUE;
        mEndFlag = OMX_FALSE;
        //MTK_OMX_MP3_LOW_POWER
#ifndef MTK_CODEC_NEW
        mPreBSBuffSize = MP3_MAX_FRAME_SIZE * (mMP3Config.nU32 + 1);
        pPreBS = (OMX_U8 *)MTK_OMX_ALLOC(mPreBSBuffSize);
        pRead = pPreBS;
        pWrite = pPreBS;
        mBSBuffSize = 0;
#endif
        LOGD("-InitMp3Decoder");
    }

#ifdef MTK_CODEC_NEW
    LOGV("-------------------New Decoder!------------------------");
#endif
    return OMX_TRUE;
}


void MtkOmxMp3Dec::FlushAudioDecoder()
{
    LOGV("+FlushAudioDecoder(MP3)");
    //MTK_OMX_MP3_LOW_POWER

    if (mMp3InitFlag == OMX_TRUE)
    {
        mMp3Dec->handle = MP3Dec_Init(mMp3Dec->working_buf1, mMp3Dec->working_buf2);
    }

    mSeekFlagFrameCount = 0;//7
    mNewOutBufferRequired = OMX_TRUE;
    mNewInBufferRequired = OMX_TRUE ;
    mOutputNum = 0;

    mEOSAfterSeek = OMX_TRUE;
    LOGV("-FlushAudioDecoder(MP3)");

    mFirstFrameFlag = OMX_TRUE;		// for computing time stamp 
    mLastSampleCount = 0;

#ifdef FOR_CTS_TEST
    isFirst = true;
    isOffsetFirst = true;
#endif

    lastTimeStamp = 0;
    lastInputTimeStamp = 0;
    mSignalledError = OMX_FALSE;

}

void MtkOmxMp3Dec::DeinitAudioDecoder()
{
    LOGV("+DeinitAudioDecoder(MP3)");

    if ((mMp3InitFlag == OMX_TRUE) && (mMp3Dec != NULL))
    {
        if (mMp3Dec->working_buf1)
        {
            MTK_OMX_FREE(mMp3Dec->working_buf1);
            mMp3Dec->working_buf1 = NULL;
        }

        if (mMp3Dec->working_buf2)
        {
            MTK_OMX_FREE(mMp3Dec->working_buf2);
            mMp3Dec->working_buf2 = NULL;
        }

#ifndef MTK_CODEC_NEW

        if (pPreBS != NULL)
        {
            MTK_OMX_FREE(pPreBS);
        }

#endif
        MTK_OMX_FREE(mMp3Dec);
        mMp3Dec = NULL;
        mMp3InitFlag = OMX_FALSE;
    }

#ifndef MTK_CODEC_NEW
    size_PreBUF = 0;
    mBSBuffSize = 0;
    mPreBufferPrepareFlag = 0;
#endif

#ifdef FOR_CTS_TEST
    isFirst = true;
    isOffsetFirst = true;
#endif

    LOGV("-DeinitAudioDecoder(MP3)");
}

OMX_ERRORTYPE MtkOmxMp3Dec::SetParameter(OMX_IN OMX_HANDLETYPE hComp,
        OMX_IN OMX_INDEXTYPE nParamIndex, OMX_IN OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    UNUSED(hComp);

    if (NULL == pCompParam) {
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    if (mState == OMX_StateInvalid) {
        err = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    switch (nParamIndex) {
        case OMX_IndexParamAudioPcm:
        {
            OMX_AUDIO_PARAM_PCMMODETYPE *pPcmMode = (OMX_AUDIO_PARAM_PCMMODETYPE *)pCompParam;

            if (pPcmMode->nPortIndex == mOutputPcmMode.nPortIndex) {
                mOutputPcmMode.nChannels = pPcmMode->nChannels;
                mOutputPcmMode.nSamplingRate = pPcmMode->nSamplingRate;
                mInputMp3Param.nChannels = mOutputPcmMode.nChannels;
                mInputMp3Param.nSampleRate = mOutputPcmMode.nSamplingRate;
            } else {
                err = OMX_ErrorUnsupportedIndex;
            }
            break;
        }
        case OMX_IndexParamAudioMp3:
        {
            OMX_AUDIO_PARAM_MP3TYPE *pAudioMp3 = (OMX_AUDIO_PARAM_MP3TYPE *)pCompParam;

            if (pAudioMp3->nPortIndex == mInputMp3Param.nPortIndex) {
                memcpy(&mInputMp3Param, pCompParam, sizeof(OMX_AUDIO_PARAM_MP3TYPE));
            } else {
                err = OMX_ErrorUnsupportedIndex;
            }
            break;
        }
        case OMX_IndexVendorMtkMP3Decode:
        {
            memcpy(&mMP3Config, pCompParam, sizeof(OMX_PARAM_U32TYPE));
            if (mMP3Config.nU32 == 1 || mMP3Config.nU32 == 20) {
                ALOGD("config mp3 outbufer contain %u frames", mMP3Config.nU32);
                mOutputPortDef.nBufferSize = MTK_OMX_OUTPUT_BUFFER_SIZE_MP3 * mMP3Config.nU32;
            }
            break;
        }
        default:
        {
             err = MtkOmxAudioDecBase::SetParameter(hComp, nParamIndex, pCompParam);
        }
    }
EXIT:
    return err;
}

OMX_ERRORTYPE MtkOmxMp3Dec::GetParameter(OMX_IN OMX_HANDLETYPE hComp,
        OMX_IN  OMX_INDEXTYPE nParamIndex, OMX_INOUT OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    UNUSED(hComp);

    if (NULL == pCompParam) {
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    if (mState == OMX_StateInvalid) {
        err = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    switch (nParamIndex) {
        case OMX_IndexParamAudioPcm:
        {
            OMX_AUDIO_PARAM_PCMMODETYPE *pPcmMode = (OMX_AUDIO_PARAM_PCMMODETYPE *)pCompParam;

            if (pPcmMode->nPortIndex == mOutputPcmMode.nPortIndex) {
                memcpy(pCompParam, &mOutputPcmMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
            } else {
                err = OMX_ErrorUnsupportedIndex;
            }
            break;
        }
        case OMX_IndexParamAudioMp3:
        {
            OMX_AUDIO_PARAM_MP3TYPE *pAudioMp3 = (OMX_AUDIO_PARAM_MP3TYPE *)pCompParam;

            if (pAudioMp3->nPortIndex == mInputMp3Param.nPortIndex) {
                memcpy(pCompParam, &mInputMp3Param, sizeof(OMX_AUDIO_PARAM_MP3TYPE));
            } else {
                err = OMX_ErrorUnsupportedIndex;
            }
            break;
        }
        case OMX_IndexVendorMtkMP3Decode:
        {
            memcpy(pCompParam, &mMP3Config, sizeof(OMX_PARAM_U32TYPE));
            break;
        }
        default:
        {
             err = MtkOmxAudioDecBase::GetParameter(hComp, nParamIndex, pCompParam);
        }
    }

EXIT:
    return err;
}

MtkOmxMp3Dec::MtkOmxMp3Dec()
{
    LOGD("JB MtkOmxMp3Dec::MtkOmxMp3Dec this= %p", this);
    mMp3Dec = NULL;
    mMp3InitFlag = OMX_FALSE;
    mOutputFrameLength = MTK_OMX_OUTPUT_BUFFER_SIZE_MP3;
    mNewOutBufferRequired = OMX_TRUE;
    mNewInBufferRequired = OMX_TRUE;
    mSeekFlagFrameCount = 0;

#ifdef FOR_CTS_TEST
    isFirst = true;
    isOffsetFirst = true;
#endif

#ifndef MTK_CODEC_NEW
    //MTK_OMX_MP3_LOW_POWER
    mPreBufferPrepareFlag = 0;
    size_PreBUF = 0 ;
    mSampleRate = 0;
    mBSBuffSize = 0;
#endif
    mOutputNum = 0;
    mBufferLength = 0 ;
    pLastBitStreamRead = NULL;
    mEndFlag = OMX_FALSE;
    mIsEndOfStream = OMX_FALSE;
    mLastTimeStamp = 0;
    mInSampleCount = 0;
    mEOSAfterSeek = OMX_FALSE;

	mFirstFrameFlag = OMX_TRUE;		 
	mLastSampleCount = 0;
	
	lastTimeStamp = 0;
	lastInputTimeStamp = 0;
}


MtkOmxMp3Dec::~MtkOmxMp3Dec()
{
    LOGD("JB ~MtkOmxMp3Dec this= %p", this);

    if (mMp3Dec)
    {
        if (mMp3Dec->working_buf1)
        {
            MTK_OMX_FREE(mMp3Dec->working_buf1);
            mMp3Dec->working_buf1 = NULL;
        }

        if (mMp3Dec->working_buf2)
        {
            MTK_OMX_FREE(mMp3Dec->working_buf2);
            mMp3Dec->working_buf2 = NULL;
        }

        MTK_OMX_FREE(mMp3Dec);
        mMp3Dec = NULL;
    }
    clearComponetAndThread();
}

OMX_BOOL MtkOmxMp3Dec::InitAudioParams()
{
    LOGD("JB MtkOmxMp3Dec::InitAudioParams(MP3)");
    // init input port format
    strncpy((char *)mCompRole, "audio_decoder.mp3", sizeof(mCompRole));
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eEncoding = OMX_AUDIO_CodingMP3;

    // init output port format
    mOutputPortFormat.nPortIndex = MTK_OMX_OUTPUT_PORT;
    mOutputPortFormat.nIndex = 0;
    mOutputPortFormat.eEncoding = OMX_AUDIO_CodingPCM;

    // init input port definition
    mInputPortDef.nPortIndex                                     = MTK_OMX_INPUT_PORT;
    mInputPortDef.eDir                                           = OMX_DirInput;
    mInputPortDef.eDomain                                        = OMX_PortDomainAudio;
    mInputPortDef.format.audio.pNativeRender             = NULL;
    mInputPortDef.format.audio.cMIMEType                   = (OMX_STRING)"audio/mpeg";
    mInputPortDef.format.audio.bFlagErrorConcealment  = OMX_FALSE;
    mInputPortDef.format.audio.eEncoding    = OMX_AUDIO_CodingMP3;

    mInputPortDef.nBufferCountActual                         = MTK_OMX_NUMBER_INPUT_BUFFER_MP3;
    mInputPortDef.nBufferCountMin                             = 1;
    mInputPortDef.nBufferSize                                    = MTK_OMX_INPUT_BUFFER_SIZE_MP3;
    mInputPortDef.bEnabled                                        = OMX_TRUE;
    mInputPortDef.bPopulated                                     = OMX_FALSE;

    //<---Donglei for conformance test
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

    mOutputPortDef.nBufferCountActual = MTK_OMX_NUMBER_OUTPUT_BUFFER_MP3;
    mOutputPortDef.nBufferCountMin = 1;
    mOutputPortDef.nBufferSize = MTK_OMX_OUTPUT_BUFFER_SIZE_MP3 * MTKMP3_NUM_OF_FRAMES;
    mOutputPortDef.bEnabled = OMX_TRUE;
    mOutputPortDef.bPopulated = OMX_FALSE;

    //<---Donglei for conformance test
    mOutputPortDef.bBuffersContiguous = OMX_FALSE;
    mOutputPortDef.nBufferAlignment   = OMX_FALSE;
    //--->

    // init input mp3 format
    mInputMp3Param.nSize = sizeof(mInputMp3Param);
    mInputMp3Param.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputMp3Param.nChannels = 2;
    mInputMp3Param.nBitRate = 0;
    mInputMp3Param.nSampleRate = 48000;
    mInputMp3Param.nAudioBandWidth = 0;
    mInputMp3Param.eChannelMode = OMX_AUDIO_ChannelModeStereo;
    mInputMp3Param.eFormat = OMX_AUDIO_MP3StreamFormatMP1Layer3;


    // init output pcm format
    mOutputPcmMode.nSize = sizeof(mOutputPcmMode);
    mOutputPcmMode.nPortIndex = MTK_OMX_OUTPUT_PORT;
    mOutputPcmMode.nChannels = 2;
    mOutputPcmMode.eNumData = OMX_NumericalDataSigned;
    mOutputPcmMode.eEndian = OMX_EndianBig;
    mOutputPcmMode.bInterleaved = OMX_TRUE;
    mOutputPcmMode.nBitPerSample = 16;
    mOutputPcmMode.nSamplingRate = 48000;
    mOutputPcmMode.ePCMMode = OMX_AUDIO_PCMModeLinear;
    mOutputPcmMode.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
    mOutputPcmMode.eChannelMapping[1] = OMX_AUDIO_ChannelRF;

    // allocate input buffer headers address array
    mInputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE *)*mInputPortDef.nBufferCountActual);
    MTK_OMX_MEMSET(mInputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE *)*mInputPortDef.nBufferCountActual);

    // allocate output buffer headers address array
    mOutputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE *)*mOutputPortDef.nBufferCountActual);
    MTK_OMX_MEMSET(mOutputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE *)*mOutputPortDef.nBufferCountActual);
    // init mp3 outputbuffer include frame number,default
    mMP3Config.nU32 = MTKMP3_NUM_OF_FRAMES;
    mMP3Config.nSize = sizeof(mMP3Config);
    return OMX_TRUE;
}

//maybe before normal ComponentDeInit,MtkOmxMp3Dec was destroyed
//because lowmemory killer or other reason,
//and ComponentDeInit is not processed,cmd thread and decode thread
//maybe meet NULL pointer of MtkOmxMp3Dec,then NE.
void MtkOmxMp3Dec::clearComponetAndThread()
{
    if (OMX_FALSE == mIsComponentAlive) {
        LOGD("component has been DeInit before,return.");
        return;
    }
    OMX_COMPONENTTYPE *pHandle = GetComponentHandle();
    if (pHandle) {
        LOGD("clearComponetAndThread ComponentDeInit");
        ComponentDeInit((OMX_HANDLETYPE)pHandle);
        pHandle = NULL;
    }
}

// Note: each MTK OMX component must export 'MtkOmxComponentCreate" to MtkOmxCore
extern "C" OMX_COMPONENTTYPE *MtkOmxComponentCreate(OMX_STRING componentName)
{

    // create component instance
    MtkOmxBase *pOmxMp3Dec  = new MtkOmxMp3Dec();

    if (NULL == pOmxMp3Dec)
    {
        LOGE("MtkOmxComponentCreate out of memory!!!");
        return NULL;
    }

    // get OMX component handle
    OMX_COMPONENTTYPE *pHandle = pOmxMp3Dec->GetComponentHandle();
    LOGV("MtkOmxComponentCreate mCompHandle(%p)", pOmxMp3Dec);

    // init component
    pOmxMp3Dec->ComponentInit(pHandle, componentName);

    return pHandle;
}

void  MtkOmxMp3Dec::QueueInputBuffer(int index)
{
    LOCK(mEmptyThisBufQLock);

    LOGV("MP3 @@ QueueInputBuffer (%d)", index);

#if CPP_STL_SUPPORT
    //mEmptyThisBufQ.push_front(index);
#endif

#if ANDROID
    mEmptyThisBufQ.insertAt(index, 0);
#endif

    UNLOCK(mEmptyThisBufQLock);
}
void  MtkOmxMp3Dec::QueueOutputBuffer(int index)
{
    LOCK(mFillThisBufQLock);

    LOGV("MP3 @@ QueueOutputBuffer (%d)", index);

#if CPP_STL_SUPPORT
    //mFillThisBufQ.push_front(index);
#endif

#if ANDROID
    mFillThisBufQ.insertAt(index, 0);
#endif
    SIGNAL(mDecodeSem);
    UNLOCK(mFillThisBufQLock);
}
// Returns the sample rate based on the sampling frequency index
int MtkOmxMp3Dec::Get_sample_rate(const int sf_index)
{
    static const int sample_rates[] =
    {
        44100, 48000, 32000, 22050,
        24000, 16000, 11025, 12000, 8000
    };

    if (sf_index < sizeof(sample_rates) / sizeof(sample_rates[0]))
    {
        return sample_rates[sf_index];
    }

    LOGD("mp3 decoder sample rate index is %d  %d", sf_index, mOutputPcmMode.nSamplingRate);
    return mOutputPcmMode.nSamplingRate;
}

bool MtkOmxMp3Dec :: isMp3FormatChanged(OMX_U8* pBitStreamRead,
                        int* sample_rate, int* num_channels)
{
    if (pBitStreamRead) {
        OMX_U32 header = U32_AT(pBitStreamRead);
        int frame_size;
        int bitrate;
        bool isGetOK = GetMPEGAudioFrameSize(
                header, &frame_size, sample_rate, num_channels, &bitrate);
        if (!isGetOK) {
            LOGW("isMp3FormatChanged isGetOK = false.");
            return false;
        }
        if (*num_channels != mOutputPcmMode.nChannels ||
            *sample_rate != mOutputPcmMode.nSamplingRate) {
            LOGD("Mp3FormatChanged!!!!!!!!");
            return true;
        }
    }
    return false;
}

void MtkOmxMp3Dec :: handleFormatChanged(OMX_U8* pBitStreamRead,
                        int sample_rate, int num_channels)
{
    if (pBitStreamRead) {
        LOGD("handleFormatChanged: num_channels(%d->%d), sample_rate(%d->%d)",
                mOutputPcmMode.nChannels, num_channels,
                mOutputPcmMode.nSamplingRate, sample_rate);
        mOutputPcmMode.nChannels = num_channels;
        mOutputPcmMode.nSamplingRate = sample_rate;
        mInputMp3Param.nChannels = mOutputPcmMode.nChannels;
        mInputMp3Param.nSampleRate = mOutputPcmMode.nSamplingRate;

        if (!isFirst) {
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle, mAppData, OMX_EventPortSettingsChanged, MTK_OMX_OUTPUT_PORT, NULL, NULL);
        }
        //remember the input position for next decode
        pLastBitStreamRead = pBitStreamRead;

        //HandleFillBufferDone will be called at the end of DecodeAudio to clear output
        mNewOutBufferRequired = OMX_FALSE;
    }
}

OMX_U32 MtkOmxMp3Dec::U32_AT(const OMX_U8 *ptr) {
    return ptr[0] << 24 | ptr[1] << 16 | ptr[2] << 8 | ptr[3];
}

//copy from avc_utils
bool MtkOmxMp3Dec::GetMPEGAudioFrameSize(
        OMX_U32 header, int *frame_size,
        int *out_sampling_rate, int *out_channels,
        int *out_bitrate, int *out_num_samples) {
    *frame_size = 0;

    if (out_sampling_rate) {
        *out_sampling_rate = 0;
    }

    if (out_channels) {
        *out_channels = 0;
    }

    if (out_bitrate) {
        *out_bitrate = 0;
    }

    if (out_num_samples) {
        *out_num_samples = 1152;
    }

    if ((header & 0xffe00000) != 0xffe00000) {
        return false;
    }

    unsigned version = (header >> 19) & 3;

    if (version == 0x01) {
        return false;
    }

    unsigned layer = (header >> 17) & 3;

    if (layer == 0x00) {
        return false;
    }

    unsigned protection __unused = (header >> 16) & 1;

    unsigned bitrate_index = (header >> 12) & 0x0f;

    if (bitrate_index == 0 || bitrate_index == 0x0f) {
        // Disallow "free" bitrate.
        return false;
    }

    unsigned sampling_rate_index = (header >> 10) & 3;

    if (sampling_rate_index == 3) {
        return false;
    }

    static const int kSamplingRateV1[] = { 44100, 48000, 32000 };
    int sampling_rate = kSamplingRateV1[sampling_rate_index];
    if (version == 2 /* V2 */) {
        sampling_rate /= 2;
    } else if (version == 0 /* V2.5 */) {
        sampling_rate /= 4;
    }

    unsigned padding = (header >> 9) & 1;

    if (layer == 3) {
        // layer I

        static const int kBitrateV1[] = {
            32, 64, 96, 128, 160, 192, 224, 256,
            288, 320, 352, 384, 416, 448
        };

        static const int kBitrateV2[] = {
            32, 48, 56, 64, 80, 96, 112, 128,
            144, 160, 176, 192, 224, 256
        };

        int bitrate =
            (version == 3 /* V1 */)
                ? kBitrateV1[bitrate_index - 1]
                : kBitrateV2[bitrate_index - 1];

        if (out_bitrate) {
            *out_bitrate = bitrate;
        }

        *frame_size = (12000 * bitrate / sampling_rate + padding) * 4;

        if (out_num_samples) {
            *out_num_samples = 384;
        }
    } else {
        // layer II or III

        static const int kBitrateV1L2[] = {
            32, 48, 56, 64, 80, 96, 112, 128,
            160, 192, 224, 256, 320, 384
        };

        static const int kBitrateV1L3[] = {
            32, 40, 48, 56, 64, 80, 96, 112,
            128, 160, 192, 224, 256, 320
        };

        static const int kBitrateV2[] = {
            8, 16, 24, 32, 40, 48, 56, 64,
            80, 96, 112, 128, 144, 160
        };

        int bitrate;
        if (version == 3 /* V1 */) {
            bitrate = (layer == 2 /* L2 */)
                ? kBitrateV1L2[bitrate_index - 1]
                : kBitrateV1L3[bitrate_index - 1];

            if (out_num_samples) {
                *out_num_samples = 1152;
            }
        } else {
            // V2 (or 2.5)

            bitrate = kBitrateV2[bitrate_index - 1];
            if (out_num_samples) {
                *out_num_samples = (layer == 1 /* L3 */) ? 576 : 1152;
            }
        }

        if (out_bitrate) {
            *out_bitrate = bitrate;
        }

        if (version == 3 /* V1 */) {
            *frame_size = 144000 * bitrate / sampling_rate + padding;
        } else {
            // V2 or V2.5
            size_t tmp = (layer == 1 /* L3 */) ? 72000 : 144000;
            *frame_size = tmp * bitrate / sampling_rate + padding;
        }
    }

    if (out_sampling_rate) {
        *out_sampling_rate = sampling_rate;
    }

    if (out_channels) {
        int channel_mode = (header >> 6) & 3;

        *out_channels = (channel_mode == 3) ? 1 : 2;
    }

    return true;
}

