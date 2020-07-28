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
 *   MtkOmxVorbisEnc.cpp
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX Vorbis Encoder component
 *
 * Author:
 * -------
 *   Qigang Wu (mtk80721)
 *
 ****************************************************************************/

#define MTK_LOG_ENABLE 1
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <dlfcn.h>
#include <cutils/log.h>
#include "osal_utils.h"
#include "MtkOmxVorbisEnc.h"
#include <cutils/properties.h>

//#define LOG_NDEBUG 0
#undef LOG_TAG
#define LOG_TAG "MtkOmxVorbisEnc"

#define MTK_OMX_VORBIS_ENCODER "OMX.MTK.AUDIO.ENCODER.VORBIS"

#define GUARD_BYTES 512
#define LOGD SLOGD
#define LOGE SLOGE
#define LOGV SLOGV
#define LOGW SLOGW


void MtkOmxVorbisEnc::EncodeAudio(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    if (pInputBuf == NULL || pOutputBuf == NULL)
    {
        LOGE("ERROR, pInputBuf or pOutputBuf is NULL");
        return;
    }

    OMX_U8 *pPcmBuffer = pInputBuf->pBuffer + pInputBuf->nOffset;
    OMX_S32 nPcmBufSize = (OMX_S32)pInputBuf->nFilledLen;
    OMX_U8 *pBitStreamBuf = pOutputBuf->pBuffer + pOutputBuf->nOffset;

    LOGV("EncodeAudio>> pInputBuf->pBuffer=%p,pInputBuf->nOffset=%u, pInputBuf->nFilledLen=%u",
         pInputBuf->pBuffer, pInputBuf->nOffset, pInputBuf->nFilledLen);

    if (mVorbisEncInit == OMX_FALSE)
    {
        if (InitVorbisEncoder())
        {
            /*
                        mPortReconfigInProgress = OMX_TRUE;
                        LOGD("--- OMX_EventPortSettingsChanged ---");
                        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                                             mAppData,
                                                             OMX_EventPortSettingsChanged,
                                                             MTK_OMX_OUTPUT_PORT,
                                                             NULL,
                                                             NULL);
            */
            QueueInputBuffer(findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pInputBuf));
            QueueOutputBuffer(findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pOutputBuf));
        }
        // init false, eos
        else
        {
            fn_ErrHandle("MtkOmxVorbisEnc::EncodeAudio:Init error", OMX_ErrorBadParameter, pInputBuf, pOutputBuf);
        }

        return;
    }
    else
    {
        //Vorbis Header
        if (mFrameCount == -1)
        {
            mFrameCount++;
            memcpy(pBitStreamBuf, (void *)((OMX_U8 *)p_bs_out + GUARD_BYTES), mHeadSize);
            pOutputBuf->nFilledLen = mHeadSize;
            pOutputBuf->nTimeStamp = pInputBuf->nTimeStamp;
            HandleEmptyBufferDone(pInputBuf);
            HandleFillBufferDone(pOutputBuf);
            LOGV("VorbisEncoder::read:writehead");
            return;
        }
        else if (mFrameCount == 0)
        {
            m_firstTimeStamp = pInputBuf->nTimeStamp;
        }

        // encode frame
        int ret = -1;
        int finalize = 0;
        OMX_S32 packetSize = 0;
        OMX_U32 ulTotalOutputSize = 0;
        OMX_U32 ulNumSamplesOut = 0;
        OMX_U8 *pPcmDataRead = pPcmBuffer;
        OMX_U32 uRemainSize  = 0;
        //this EncodeAudio frame count, maybe: 0, 1, 2.used for calculate output timestamp
        OMX_U32 lFrameCount = 0;
        OMX_TICKS lFrameTime = 0;

        if (pInputBuf->nFlags & OMX_BUFFERFLAG_EOS)
            // lastest frame
        {
            mEosTimes++;
            LOGD("VORBIS EOS received, TS=%lld,times=%d", pInputBuf->nTimeStamp, mEosTimes);

            if (mEosTimes == 1)
            {
                //finalize = 1 encode
                OMX_U32 nOffset   = mVorbisTempBuf->write_ptr - mVorbisTempBuf->buffer_base;
                OMX_U32 nCopySize = nPcmBufSize > (mVorbisTempBuf->buffer_size - nOffset) ? (mVorbisTempBuf->buffer_size - nOffset) : nPcmBufSize;
                //last pcm data probabley size != mVorbisTempBuf->buffer_size
                OMX_U32 nTotleSize = nOffset + nCopySize;
                memcpy(mVorbisTempBuf->write_ptr, (void *)pPcmBuffer, nCopySize);
                //Dump File
                Dump_PCMData(mDumpFile, mVorbisTempBuf->buffer_base, nTotleSize);
                //memcpy((uint8_t *)p_pcm_in+GUARD_BYTES, mVorbisTempBuf->buffer_base, nTotleSize);
                finalize = 1;
                ret = minvorbis_encode_one_frame(h_minvorbis, (unsigned char *)mVorbisTempBuf->buffer_base, (unsigned char *)pBitStreamBuf, finalize, 0);

                if (ret < 0)
                {
                    fn_ErrHandle("minvorbis_encode_one_frame() error:eos", OMX_ErrorUndefined, pInputBuf, pOutputBuf);
                    return;
                }

                mFrameCount++;
                packetSize = ret;
                mVorbisTempBuf->write_ptr = mVorbisTempBuf->buffer_base;
                pBitStreamBuf += packetSize;
                ulTotalOutputSize += packetSize;
                pOutputBuf->nFilledLen = ulTotalOutputSize;
                LOGV("Deal eos,nTotleSize=%d,ret=%d,timestamp=%lld,pOutputBuf->nFilledLen=%d", nTotleSize, ret, pOutputBuf->nTimeStamp, pOutputBuf->nFilledLen);
                // flush encoder
                FlushAudioEncoder();
            }

            // return the EOS output buffer
            pOutputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
            HandleFillBufferDone(pOutputBuf);
            // return the EOS input buffer
            HandleEmptyBufferDone(pInputBuf);
            LOGV("mNumPendingInput(%d), mNumPendingOutput(%d)", (int)mNumPendingInput, (int)mNumPendingOutput);
            DumpFTBQ();
            return;
        }

        //   memcpy(pBitStreamBuf, pPcmBuffer, nPcmBufSize);
        //   pOutputBuf->nFilledLen = nPcmBufSize;
        //  mFrameCount++;

        if (mVorbisTempBuf->buffer_partial)
        {
            OMX_U32 nOffset   = mVorbisTempBuf->write_ptr - mVorbisTempBuf->buffer_base;
            CHECK(nOffset <= mVorbisTempBuf->buffer_size);
            OMX_U32 nCopySize = nPcmBufSize > (mVorbisTempBuf->buffer_size - nOffset) ? (mVorbisTempBuf->buffer_size - nOffset) : nPcmBufSize;
            memcpy(mVorbisTempBuf->write_ptr, (void *)pPcmBuffer, nCopySize);
            pPcmBuffer += nCopySize;
            nPcmBufSize -= nCopySize;

            mVorbisTempBuf->write_ptr += nCopySize;
            LOGV("buffer_partial:1,nCopySize=%d", nCopySize);

            if (mVorbisTempBuf->write_ptr >= mVorbisTempBuf->buffer_base + mVorbisTempBuf->buffer_size)
            {
                mVorbisTempBuf->write_ptr = mVorbisTempBuf->buffer_base;
                mVorbisTempBuf->buffer_partial = OMX_FALSE;
                mVorbisTempBuf->buffer_full = OMX_TRUE;
                LOGV("buffer_partial:2");
            }
        }

        if (mVorbisTempBuf->buffer_full)
        {
            //Dump File
            Dump_PCMData(mDumpFile, mVorbisTempBuf->buffer_base, mVorbisTempBuf->buffer_size);
            ret = minvorbis_encode_one_frame(h_minvorbis, mVorbisTempBuf->buffer_base, (unsigned char *)p_bs_out + GUARD_BYTES, finalize, 0);

            /*
                 ret = minvorbis_encode_one_frame(h_minvorbis,
                     mVorbisTempBuf->buffer_base,
                     pBitStreamBuf,
                     finalize,
                     0);
            */
            if (ret < 0)
            {
                fn_ErrHandle("minvorbis_encode_one_frame() error:tempbuf", OMX_ErrorUndefined, pInputBuf, pOutputBuf);
                return;
            }
            else if (ret > 0)
            {
                memcpy(pBitStreamBuf, (void *)((OMX_U8 *)p_bs_out + GUARD_BYTES), ret);
            }

            mFrameCount++;
            lFrameCount++;
            lFrameTime += m_FrameTime;
            packetSize = ret;
            mVorbisTempBuf->buffer_full = OMX_FALSE;
            mVorbisTempBuf->write_ptr   = mVorbisTempBuf->buffer_base;
            pBitStreamBuf += packetSize;
            ulTotalOutputSize += packetSize;
        }

        //kNumSamplesPerFrame * sizeof(OMX_S16) * mOutputVorbisParam.nChannels
        uRemainSize = nPcmBufSize;

        while (uRemainSize >= mVorbisTempBuf->buffer_size)
        {
            //Dump File
            Dump_PCMData(mDumpFile, pPcmBuffer, mVorbisTempBuf->buffer_size);
            //LOGD("nPcmBufSize=%d,pPcmBuffer=%p,pBitStreamBuf=%p",nPcmBufSize,pPcmBuffer,pBitStreamBuf);
            //ret = minvorbis_encode_one_frame(h_minvorbis, pPcmBuffer, pBitStreamBuf, finalize, 0);
            memcpy(mVorbisTempBuf->buffer_base, pPcmBuffer, mVorbisTempBuf->buffer_size);
            ret = minvorbis_encode_one_frame(h_minvorbis, mVorbisTempBuf->buffer_base, (unsigned char *)p_bs_out + GUARD_BYTES, finalize, 0);

            if (ret < 0)
            {
                fn_ErrHandle("minvorbis_encode_one_frame() error", OMX_ErrorUndefined, pInputBuf, pOutputBuf);
                return;
            }
            else if (ret > 0)
            {
                memcpy(pBitStreamBuf, (void *)((OMX_U8 *)p_bs_out + GUARD_BYTES), ret);
            }

            packetSize = ret;
            pPcmBuffer += mVorbisTempBuf->buffer_size;
            pBitStreamBuf += packetSize;
            ulTotalOutputSize += packetSize;
            lFrameCount++;
            lFrameTime += m_FrameTime;
            uRemainSize -= mVorbisTempBuf->buffer_size;
        }

        pOutputBuf->nFilledLen = ulTotalOutputSize;
        LOGV("MtkOmxVorbisEnc::EncodeAudio,pOutputBuf->nFilledLen=%d", pOutputBuf->nFilledLen);

        if (uRemainSize > 0)
        {
            memcpy((void *)mVorbisTempBuf->buffer_base, (void *)pPcmBuffer, uRemainSize);
            mVorbisTempBuf->write_ptr   = mVorbisTempBuf->buffer_base + uRemainSize;
            mVorbisTempBuf->buffer_partial = OMX_TRUE;
            mVorbisTempBuf->buffer_full = OMX_FALSE;
            LOGV("MtkOmxVorbisEnc::EncodeAudio:uReamainSize=%d", uRemainSize);
        }

        if (m_NextTimeStamp == 0)
        {
            pOutputBuf->nTimeStamp = m_firstTimeStamp;
        }
        else
        {
            pOutputBuf->nTimeStamp = m_NextTimeStamp;
        }

        //m_NextTimeStamp = pOutputBuf->nTimeStamp+ 1000000LL * lFrameCount * kNumSamplesPerFrame/m_samplerate ;
        m_NextTimeStamp = pOutputBuf->nTimeStamp + lFrameTime;

        LOGV("pInputBuf:nFilledLen=%u|pOutputBuf:nFilledLen=%u|lFrameCount=%d,pOutputBuf->nTimeStamp=%lli",
             pInputBuf->nFilledLen, pOutputBuf->nFilledLen,
             lFrameCount, pOutputBuf->nTimeStamp
            );
        HandleEmptyBufferDone(pInputBuf);
        HandleFillBufferDone(pOutputBuf);
    }

}

int MtkOmxVorbisEnc::EncodeAudio()
{
    int input_idx = 0, output_idx = 0;
    int iResult= 0;
    OMX_BUFFERHEADERTYPE *pInputBuf =NULL, *pOutputBuf = NULL;
    OMX_U8 *pPcmBuffer = NULL, *pBitStreamBuf = NULL;
    OMX_S32 nPcmBufSize = 0;
    OMX_S32 packetSize = 0;
    OMX_U32 uRemainSize  = 0;
    OMX_U32 nOffset = 0;
    OMX_U32 nCopySize = 0;
    int finalize = 0;
    int ret = -1;
//init
    if (mVorbisEncInit == OMX_FALSE)
    {
        if (!InitVorbisEncoder())
        {
            LOGE("MtkOmxVorbisEnc::EncodeAudio:Init error");
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle, mAppData, OMX_EventError, OMX_ErrorBadParameter, 0, NULL);
            return INITENC_FAIL;
        }
    }
    if (mFrameCount == -1)
    {
        //output
        iResult = GetOutputBuffer();
        if (iResult < 0)
            return iResult;
        else
            output_idx = iResult;
        pOutputBuf = mOutputBufferHdrs[output_idx];
        pBitStreamBuf = pOutputBuf->pBuffer + pOutputBuf->nOffset;
        memcpy(pBitStreamBuf, (OMX_U8*)p_bs_out + GUARD_BYTES, mHeadSize);
        pOutputBuf->nFilledLen = mHeadSize;
        pOutputBuf->nTimeStamp = 0;
        HandleFillBufferDone(pOutputBuf);
        LOGV("writehead,done");
        mFrameCount++;
    }
    while (true)
    {
        iResult=GetInputBuffer();
        if (iResult < 0)
            return iResult;
        else
            input_idx = iResult;
         //
        pInputBuf = mInputBufferHdrs[input_idx];
	LOGV("input hdr:%p,index:%d", pInputBuf,input_idx);
        pPcmBuffer = pInputBuf->pBuffer + pInputBuf->nOffset;
        nPcmBufSize = pInputBuf->nFilledLen - pInputBuf->nOffset;
        LOGV("input buf:%p,bufsize=%d",pPcmBuffer,nPcmBufSize);
        if (pInputBuf->nFlags & OMX_BUFFERFLAG_EOS)
        {
            iResult = GetOutputBuffer(input_idx);
            if (iResult < 0)
                return iResult;
            else
                output_idx = iResult;
            OMX_BUFFERHEADERTYPE *pOutputBuf = mOutputBufferHdrs[output_idx];
            pBitStreamBuf = pOutputBuf->pBuffer + pOutputBuf->nOffset;
            mEosTimes++;
            LOGD("VORBIS EOS received, TS=%lld,times=%d", pInputBuf->nTimeStamp, mEosTimes);

            if (mEosTimes == 1)
            {
                //finalize = 1 encode
                OMX_U32 nOffset   = mVorbisTempBuf->write_ptr - mVorbisTempBuf->buffer_base;
                OMX_U32 nCopySize = nPcmBufSize > (mVorbisTempBuf->buffer_size - nOffset) ? (mVorbisTempBuf->buffer_size - nOffset) : nPcmBufSize;
                //last pcm data probabley size != mVorbisTempBuf->buffer_size
                OMX_U32 nTotleSize = nOffset + nCopySize;
                memcpy(mVorbisTempBuf->write_ptr, (void *)pPcmBuffer, nCopySize);
                //Dump File
                Dump_PCMData(mDumpFile, mVorbisTempBuf->buffer_base, nTotleSize);
                finalize = 1;
                ret = minvorbis_encode_one_frame(h_minvorbis, (unsigned char *)mVorbisTempBuf->buffer_base, (unsigned char *)pBitStreamBuf, finalize, 0);
                if (ret < 0)
                {
                    fn_ErrHandle("minvorbis_encode_one_frame() error:eos", OMX_ErrorUndefined, pInputBuf, pOutputBuf);
                    return ENC_ERR;
                }
                mFrameCount++;
                packetSize = ret;
                mVorbisTempBuf->write_ptr = mVorbisTempBuf->buffer_base;
                pOutputBuf->nFilledLen = packetSize;
                LOGV("Deal eos,nTotleSize=%d,ret=%d,timestamp=%lld,pOutputBuf->nFilledLen=%d", nTotleSize, ret, pOutputBuf->nTimeStamp, pOutputBuf->nFilledLen);
                // flush encoder
                FlushAudioEncoder();
            }

            // return the EOS output buffer
            pOutputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
            HandleFillBufferDone(pOutputBuf);
            // return the EOS input buffer
            HandleEmptyBufferDone(pInputBuf);
            LOGV("mNumPendingInput(%d), mNumPendingOutput(%d)", (int)mNumPendingInput, (int)mNumPendingOutput);
            DumpFTBQ();
            return ENC_NOERR;
        }

        if (mVorbisTempBuf->buffer_partial)
        {
            nOffset   = mVorbisTempBuf->write_ptr - mVorbisTempBuf->buffer_base;
            CHECK(nOffset <= mVorbisTempBuf->buffer_size);
            nCopySize = nPcmBufSize > (mVorbisTempBuf->buffer_size - nOffset) ? (mVorbisTempBuf->buffer_size - nOffset) : nPcmBufSize;
            memcpy(mVorbisTempBuf->write_ptr, (void *)pPcmBuffer, nCopySize);
            mVorbisTempBuf->write_ptr += nCopySize;
	        mdealsize = nCopySize;
            pInputBuf->nOffset += nCopySize;
            if (mVorbisTempBuf->write_ptr >= mVorbisTempBuf->buffer_base + mVorbisTempBuf->buffer_size)
            {
                mVorbisTempBuf->write_ptr = mVorbisTempBuf->buffer_base;
                mVorbisTempBuf->buffer_partial = OMX_FALSE;
                mVorbisTempBuf->buffer_full = OMX_TRUE;
            }
            LOGV("tmpbuf partial:%d,copysize=%d",mVorbisTempBuf->buffer_partial,nCopySize);
        }
        if (mVorbisTempBuf->buffer_full)
        {
            LOGV("tempbuf full");
            iResult = GetOutputBuffer(input_idx);
            if (iResult < 0)
                return iResult;
            else
                output_idx = iResult;
            pOutputBuf = mOutputBufferHdrs[output_idx];
            pBitStreamBuf = pOutputBuf->pBuffer + pOutputBuf->nOffset;
            LOGV("encode tmpbuf:%p",mVorbisTempBuf->buffer_base);
            //Dump File
            Dump_PCMData(mDumpFile, mVorbisTempBuf->buffer_base, mVorbisTempBuf->buffer_size);
            ret = minvorbis_encode_one_frame(h_minvorbis, mVorbisTempBuf->buffer_base, (unsigned char *)p_bs_out + GUARD_BYTES, finalize, 0);
            if (ret < 0)
            {
                fn_ErrHandle("minvorbis_encode_one_frame() error:tempbuf", OMX_ErrorUndefined, pInputBuf, pOutputBuf);
                return ENC_ERR;
            }
            else if (ret > 0)
            {
                memcpy(pBitStreamBuf, (OMX_U8*)p_bs_out + GUARD_BYTES, ret);
            }

            mFrameCount++;
            packetSize = ret;
            mVorbisTempBuf->buffer_full = OMX_FALSE;
            mVorbisTempBuf->write_ptr   = mVorbisTempBuf->buffer_base;
            mdealsize = nCopySize;
            pOutputBuf->nTimeStamp = m_NextTimeStamp == 0 ? pInputBuf->nTimeStamp : m_NextTimeStamp;
            m_NextTimeStamp = pOutputBuf->nTimeStamp+m_FrameTime;
            pOutputBuf->nFilledLen=packetSize;
            LOGV("outbuf:%p,timestamp=%lld",pOutputBuf->pBuffer,pOutputBuf->nTimeStamp);
            HandleFillBufferDone(pOutputBuf);
        }

        pPcmBuffer += mdealsize;
        nPcmBufSize -= mdealsize;
        uRemainSize = nPcmBufSize;

        while (uRemainSize >= mVorbisTempBuf->buffer_size)
        {
            LOGV("encode input buf,remainsize=%d",uRemainSize);
            iResult = GetOutputBuffer(input_idx);
            if (iResult < 0)
                return iResult;
            else
                output_idx = iResult;
            pOutputBuf = mOutputBufferHdrs[output_idx];
            pBitStreamBuf = pOutputBuf->pBuffer + pOutputBuf->nOffset;
            //Dump File
            Dump_PCMData(mDumpFile, pPcmBuffer, mVorbisTempBuf->buffer_size);
            memcpy(mVorbisTempBuf->buffer_base, pPcmBuffer, mVorbisTempBuf->buffer_size);
            ret = minvorbis_encode_one_frame(h_minvorbis, mVorbisTempBuf->buffer_base, (unsigned char *)p_bs_out + GUARD_BYTES, finalize, 0);
            if (ret < 0)
            {
                fn_ErrHandle("minvorbis_encode_one_frame() error", OMX_ErrorUndefined, pInputBuf, pOutputBuf);
                return ENC_ERR;
            }
            else if (ret > 0)
            {
                memcpy(pBitStreamBuf, (OMX_U8*)p_bs_out + GUARD_BYTES, ret);
            }
            packetSize = ret;
            uRemainSize -= mVorbisTempBuf->buffer_size;
            pPcmBuffer += mVorbisTempBuf->buffer_size;
            mdealsize += mVorbisTempBuf->buffer_size;
	        pInputBuf->nOffset += mVorbisTempBuf->buffer_size;
            pOutputBuf->nTimeStamp = m_NextTimeStamp == 0 ? pInputBuf->nTimeStamp : m_NextTimeStamp;
            m_NextTimeStamp = pOutputBuf->nTimeStamp+m_FrameTime;
            pOutputBuf->nFilledLen=packetSize;
            LOGV("outbuf:%p,timestamp=%lld",pOutputBuf->pBuffer,pOutputBuf->nTimeStamp);
            HandleFillBufferDone(pOutputBuf);
        }

        if (uRemainSize > 0)
        {
            memcpy((void *)mVorbisTempBuf->buffer_base, (void *)pPcmBuffer, uRemainSize);
            mVorbisTempBuf->write_ptr   = mVorbisTempBuf->buffer_base + uRemainSize;
            mVorbisTempBuf->buffer_partial = OMX_TRUE;
            mVorbisTempBuf->buffer_full = OMX_FALSE;
            LOGV("EncodeAudio:uReamainSize=%d", uRemainSize);
        }
        LOGV("empty buf done hdr:%p",pInputBuf);
        HandleEmptyBufferDone(pInputBuf);
        if (mdealsize > 0)
            mdealsize = 0;
    }

    return ENC_NOERR;
}
void MtkOmxVorbisEnc::fn_ErrHandle(const char *pErrMsg, OMX_ERRORTYPE pErrType,
                                   OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    LOGE("%s", pErrMsg);
    mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                           mAppData,
                           OMX_EventError,
                           pErrType,
                           0,
                           NULL);
    HandleEmptyBufferDone(pInputBuf);
    HandleFillBufferDone(pOutputBuf);
}

bool MtkOmxVorbisEnc::InitVorbisEncoder()
{

    int ret;

    if (mVorbisEncInit == OMX_TRUE)
    {
        LOGW("Call InitVorbisEncoder when encoder already Init");
        return OMX_TRUE;
    }

    minvorbis_get_mem_size_for_encoding(&sz_rt_tab, &sz_shared, &sz_encoder, &sz_parser, &sz_pcm_in, &sz_bs_out , 0);
    LOGV("minvorbis_get_mem_size_for_encoding,sz_rt_tab=%d,sz_shared=%d,sz_encoder=%d,sz_parser=%d,sz_pcm_in=%d,sz_bs_out=%d",
         sz_rt_tab, sz_shared, sz_encoder, sz_parser, sz_pcm_in, sz_bs_out);

    p_shared        = malloc(sz_shared  + 2 * GUARD_BYTES);
    p_encoder       = malloc(sz_encoder + 2 * GUARD_BYTES);
    p_parser        = malloc(sz_parser  + 2 * GUARD_BYTES);
    p_pcm_in        = malloc(sz_pcm_in  + 2 * GUARD_BYTES);
    p_bs_out        = malloc(sz_bs_out  + 2 * GUARD_BYTES);
    p_rt_tab        = malloc(sz_rt_tab  + 2 * GUARD_BYTES);

    memset(p_shared, 0xff, sz_shared    + 2 * GUARD_BYTES);
    memset(p_encoder, 0xff, sz_encoder   + 2 * GUARD_BYTES);
    memset(p_parser, 0xff, sz_parser    + 2 * GUARD_BYTES);
    memset(p_pcm_in, 0xff, sz_pcm_in    + 2 * GUARD_BYTES);
    memset(p_bs_out, 0xff, sz_bs_out    + 2 * GUARD_BYTES);
    memset(p_rt_tab, 0xff, sz_rt_tab    + 2 * GUARD_BYTES);

    m_bitrate = mOutputVorbisParam.nBitRate;
    m_samplerate = mOutputVorbisParam.nSampleRate;
    m_ChannelCount = mOutputVorbisParam.nChannels;

    int pBitRate = fn_GetBitRate(m_samplerate);

    if (pBitRate == 0)
    {
        return OMX_FALSE;
    }

    h_minvorbis = minvorbis_init_encoder((char *)p_rt_tab + GUARD_BYTES, (char *)p_shared + GUARD_BYTES,
                                         (char *)p_encoder + GUARD_BYTES, (char *)p_parser + GUARD_BYTES, m_ChannelCount,
                                         m_samplerate, pBitRate, (unsigned char *)p_bs_out + GUARD_BYTES, &ret, 0);

    LOGD("VorbisEncoder::start:minvorbis_init_encoder");

    CHECK(h_minvorbis != 0);
    mFrameCount = -1;
    mHeadSize = ret;

    // Temp buffer for keep the remain data from input buffer
    if (NULL == mVorbisTempBuf)
    {
        mVorbisTempBuf = (vorbisEncTempBuffer *)MTK_OMX_ALLOC(sizeof(vorbisEncTempBuffer));
        MTK_OMX_MEMSET(mVorbisTempBuf, 0, sizeof(vorbisEncTempBuffer));

        mVorbisTempBuf->buffer_size = kNumSamplesPerFrame * mOutputVorbisParam.nChannels * sizeof(OMX_S16);
        mVorbisTempBuf->buffer_base = (OMX_U8 *)MTK_OMX_ALLOC(mVorbisTempBuf->buffer_size);
        MTK_OMX_MEMSET(mVorbisTempBuf->buffer_base, 0, mVorbisTempBuf->buffer_size);

        mVorbisTempBuf->buffer_partial = OMX_FALSE;
        mVorbisTempBuf->buffer_full = OMX_FALSE;
        mVorbisTempBuf->write_ptr   = mVorbisTempBuf->buffer_base;
        m_FrameTime = 1000000LL * kNumSamplesPerFrame / m_samplerate ;
    }

    //Dump File
    Dump_Access(mDumpProp, mDumpFile);

    mVorbisEncInit = OMX_TRUE;
    return OMX_TRUE;
}


void MtkOmxVorbisEnc::FlushAudioEncoder()
{
    if (mVorbisEncInit == OMX_TRUE)
    {
        LOGD("FlushAudioEncoder(VORBIS)");
        MTK_OMX_MEMSET(mVorbisTempBuf->buffer_base, 0, mVorbisTempBuf->buffer_size);
        mVorbisTempBuf->buffer_partial = OMX_FALSE;
        mVorbisTempBuf->buffer_full = OMX_FALSE;
        mVorbisTempBuf->write_ptr   = mVorbisTempBuf->buffer_base;
    }

}

void MtkOmxVorbisEnc::DeinitAudioEncoder()
{
    LOGD("+DeinitAudioDecoder(VORBIS)");

    if (mVorbisEncInit == OMX_TRUE)
    {
        if (NULL != mVorbisTempBuf)
        {
            MTK_OMX_FREE(mVorbisTempBuf->buffer_base);
            MTK_OMX_FREE(mVorbisTempBuf);
            mVorbisTempBuf = NULL;
        }

        mVorbisEncInit = OMX_FALSE;

    }

    LOGD("-DeinitAudioEncoder(VORBIS)");
}



MtkOmxVorbisEnc::MtkOmxVorbisEnc():
    mVorbisTempBuf(NULL),
    mVorbisEncInit(OMX_FALSE),
    mFrameCount(0),
    mEosTimes(0),
    p_pcm_in(NULL),
    p_bs_out(NULL),
    p_shared(NULL),
    p_encoder(NULL),
    p_parser(NULL),
    p_rt_tab(NULL),
    m_NextTimeStamp(0),
    mDumpProp("vendor.audio.dumpenc.vorbis"),
    mDumpFile("sdcard/vorbisenc_out.pcm"),
    mdealsize(0)
{
    LOGD("MtkOmxVorbisEnc::MtkOmxVorbisEnc this= %p", this);
    mEncodePlus = true;

}


MtkOmxVorbisEnc::~MtkOmxVorbisEnc()
{
    LOGD("~MtkOmxVorbisEnc this= %p", this);

    if (mVorbisTempBuf)
    {
        MTK_OMX_FREE(mVorbisTempBuf->buffer_base);
        MTK_OMX_FREE(mVorbisTempBuf);
        mVorbisTempBuf = NULL;
    }

    if (p_shared != NULL)
    {
        free(p_shared);
        p_shared = NULL;
    }

    if (p_encoder != NULL)
    {
        free(p_encoder);
        p_encoder = NULL;
    }

    if (p_parser != NULL)
    {
        free(p_parser);
        p_parser = NULL;
    }

    if (p_pcm_in != NULL)
    {
        free(p_pcm_in);
        p_pcm_in = NULL;
    }

    if (p_bs_out != NULL)
    {
        free(p_bs_out);
        p_bs_out = NULL;
    }

    if (p_rt_tab != NULL)
    {
        free(p_rt_tab);
        p_rt_tab = NULL;

    }
}

OMX_BOOL MtkOmxVorbisEnc::InitAudioParams()
{
    LOGD("MtkOmxVorbisEnc::InitAudioParams(VORBIS)");
    // init input port format
    strncpy((char *)mCompRole, "audio_encoder.vorbis", OMX_MAX_STRINGNAME_SIZE -1);
    mCompRole[OMX_MAX_STRINGNAME_SIZE -1] = '\0';
    mInputPortFormat.nSize = sizeof(mInputPortFormat);
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eEncoding = OMX_AUDIO_CodingPCM;

    // init output port format
    mOutputPortFormat.nSize = sizeof(mOutputPortFormat);
    mOutputPortFormat.nPortIndex = MTK_OMX_OUTPUT_PORT;
    mOutputPortFormat.nIndex = 0;
    mOutputPortFormat.eEncoding = OMX_AUDIO_CodingVORBIS;

    // init input port definition
    mInputPortDef.nSize = sizeof(mInputPortDef);
    mInputPortDef.nPortIndex                                     = MTK_OMX_INPUT_PORT;
    mInputPortDef.eDir                                           = OMX_DirInput;
    mInputPortDef.eDomain                                        = OMX_PortDomainAudio;
    mInputPortDef.format.audio.pNativeRender             = NULL;
    mInputPortDef.format.audio.cMIMEType                   = (OMX_STRING)"raw";
    mInputPortDef.format.audio.bFlagErrorConcealment  = OMX_FALSE;
    mInputPortDef.format.audio.eEncoding    = OMX_AUDIO_CodingPCM;

    mInputPortDef.nBufferCountActual                         = MTK_OMX_NUMBER_INPUT_BUFFER_VORBIS_ENC;
    mInputPortDef.nBufferCountMin                             = 1;
    mInputPortDef.nBufferSize                                    = MTK_OMX_INPUT_BUFFER_SIZE_VORBIS_ENC;
    mInputPortDef.bEnabled                                        = OMX_TRUE;
    mInputPortDef.bPopulated                                     = OMX_FALSE;

    // init output port definition
    mOutputPortDef.nSize = sizeof(mOutputPortDef);
    mOutputPortDef.nPortIndex = MTK_OMX_OUTPUT_PORT;
    mOutputPortDef.eDomain = OMX_PortDomainAudio;
    mOutputPortDef.format.audio.cMIMEType = (OMX_STRING)"audio/ogg";
    mOutputPortDef.format.audio.pNativeRender = 0;
    mOutputPortDef.format.audio.bFlagErrorConcealment = OMX_FALSE;
    mOutputPortDef.format.audio.eEncoding = OMX_AUDIO_CodingVORBIS;
    mOutputPortDef.eDir = OMX_DirOutput;

    mOutputPortDef.nBufferCountActual = MTK_OMX_NUMBER_OUTPUT_BUFFER_VORBIS_ENC;
    mOutputPortDef.nBufferCountMin = 1;
    mOutputPortDef.nBufferSize = MTK_OMX_OUTPUT_BUFFER_SIZE_VORBIS_ENC;
    mOutputPortDef.bEnabled = OMX_TRUE;
    mOutputPortDef.bPopulated = OMX_FALSE;

    //Default values for PCM input audio param port
    mInputPcmMode.nSize = sizeof(mInputPcmMode);
    mInputPcmMode.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPcmMode.nChannels = 2;
    mInputPcmMode.eNumData = OMX_NumericalDataSigned;
    mInputPcmMode.bInterleaved = OMX_TRUE;
    mInputPcmMode.nBitPerSample = 16;
    mInputPcmMode.nSamplingRate = 16000;
    mInputPcmMode.ePCMMode = OMX_AUDIO_PCMModeLinear;
    mInputPcmMode.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
    mInputPcmMode.eChannelMapping[1] = OMX_AUDIO_ChannelRF;

    //Default values for Vorbis output audio param port
    mOutputVorbisParam.nSize = sizeof(mOutputVorbisParam);
    mOutputVorbisParam.nPortIndex = MTK_OMX_OUTPUT_PORT;
    mOutputVorbisParam.nChannels = 2;
    mOutputVorbisParam.nBitRate = 0;

    // allocate input buffer headers address array
    mInputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE *)*mInputPortDef.nBufferCountActual);
    MTK_OMX_MEMSET(mInputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE *)*mInputPortDef.nBufferCountActual);

    // allocate output buffer headers address array
    mOutputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE *)*mOutputPortDef.nBufferCountActual);
    MTK_OMX_MEMSET(mOutputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE *)*mOutputPortDef.nBufferCountActual);
    return OMX_TRUE;
}

int MtkOmxVorbisEnc::fn_GetBitRate(int pSampleRate, int pQuality)
{
    int rBitRate = 0;

    if (pSampleRate >= 44000)
    {
        rBitRate = 2;
    }
    else if (pSampleRate == 32000 || pSampleRate == 22050 || pSampleRate == 16000)
    {
        rBitRate = (pQuality == 1 ? 3 : 5);
    }
    else if (pSampleRate == 8000 || pSampleRate == 11025 || pSampleRate == 12000)
    {
        rBitRate = 5;
    }
    else
    {
        LOGE("MtkOmxVorbisEnc::GetBitRate error, samplerate=%d,Quality=%d", pSampleRate, pQuality);
    }

    LOGV("MtkOmxVorbisEnc::GetBitRate samplerate=%d,rBitRate=%d", pSampleRate, rBitRate);
    return rBitRate;
}

//The code has benn implemented in StagefrightRecorder::startOGGRecording(), so it is not necessary to run again
OMX_ERRORTYPE MtkOmxVorbisEnc::CheckParams(OMX_PTR params)
{
    //LOGE("MtkOmxVorbisEnc::CheckParams");
    OMX_ERRORTYPE err = OMX_ErrorNone;

    return err;
}

// Note: each MTK OMX component must export 'MtkOmxComponentCreate" to MtkOmxCore
extern "C" OMX_COMPONENTTYPE *MtkOmxComponentCreate(OMX_STRING componentName)
{

    // create component instance
    MtkOmxBase *pOmxVorbisEnc  = new MtkOmxVorbisEnc();

    if (NULL == pOmxVorbisEnc)
    {
        LOGE("MtkOmxComponentCreate out of memory!!!");
        return NULL;
    }

    // get OMX component handle
    OMX_COMPONENTTYPE *pHandle = pOmxVorbisEnc->GetComponentHandle();
    LOGD("MtkOmxComponentCreate mCompHandle(%p)", pOmxVorbisEnc);

    // init component
    pOmxVorbisEnc->ComponentInit(pHandle, componentName);

    return pHandle;
}

