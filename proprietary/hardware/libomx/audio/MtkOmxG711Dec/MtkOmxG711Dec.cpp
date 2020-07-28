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
 *   MtkOmxAacDec.cpp
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX G711 Decoder component
 *
 * Author:
 * -------
 *   Zhihui Zhang (mtk80712)
 *
 ****************************************************************************/

#define MTK_LOG_ENABLE 1
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <dlfcn.h>
#include <cutils/log.h>
#include "osal_utils.h"
#include "MtkOmxG711Dec.h"

#include <cutils/properties.h>

#undef LOG_TAG
#define LOG_TAG "MtkOmxG711Dec"

//#define MTK_OMX_G711_DEC_DUMP

//#define ENABLE_XLOG_MtkOmxG711Dec
#ifdef ENABLE_XLOG_MtkOmxG711Dec
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
#else
#define LOGE ALOGE
#define LOGW ALOGW
#define LOGI ALOGI
#define LOGD ALOGD
#define LOGV ALOGV
#endif





MtkOmxG711Dec::MtkOmxG711Dec()
{
    LOGV("MtkOmxG711Dec::MtkOmxG711Dec(G711) this= %p", this);
    mG711InitFlag = OMX_FALSE;
    mIsMLaw = OMX_FALSE;
}

MtkOmxG711Dec::~MtkOmxG711Dec()
{
    LOGV("MtkOmxG711Dec::~MtkOmxG711Dec(G711) this= %p", this);
}


OMX_BOOL  MtkOmxG711Dec::OmxG711DecInit()
{
    LOGV("MtkOmxG711Dec::MtkG711DecInit In !");

    if (mInputG711Mode.ePCMMode == OMX_AUDIO_PCMModeMULaw)
    {
        mIsMLaw = OMX_TRUE;
        LOGV("MtkOmxG711Dec-----MULaw");
    }
    else if (mInputG711Mode.ePCMMode == OMX_AUDIO_PCMModeALaw)
    {
        mIsMLaw = OMX_FALSE;
        LOGV("MtkOmxG711Dec-----ALaw");
    }
    else
    {
        LOGE("OmxG711DecInit Failure!");
        return OMX_FALSE;
    }

    return OMX_TRUE;
}

void MtkOmxG711Dec::DecodeAudio(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    LOGV("In DecodeAudio -- pInputBuf:%p , pOutputBuf :%p", pInputBuf, pOutputBuf);
    LOGV("In DecodeAudio -- pInputBuf->pBuffer:%p , pOutputBuf->pBuffer :%p", pInputBuf->pBuffer, pOutputBuf->pBuffer);

    if (OMX_FALSE == mG711InitFlag)
    {
        if (OmxG711DecInit())
        {
            mG711InitFlag = OMX_TRUE;
            LOGV("Init,so push the input buffer back to the head of the input queue!");
            // push the input buffer back to the head of the input queue
            QueueInputBuffer(findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pInputBuf));
            QueueOutputBuffer(findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pOutputBuf));

        }
        else
        {
            OMX_U32 error = OMX_ErrorBadParameter;
            QueueInputBuffer(findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pInputBuf));
            QueueOutputBuffer(findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pOutputBuf));
            mSignalledError = true;
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventError,
                                   error,
                                   0, NULL);

            LOGE("MtkOmxG711Dec::DecodeAudio Init Failure!");
        }

        return;
    }
    else
    {
        // If EOS flag has come from the client & there are no more
        // input buffers to decode, send the callback to the client
        if ((pInputBuf->nFlags & OMX_BUFFERFLAG_EOS) && (pInputBuf->nFilledLen == 0))    // lastest frame
        {
            LOGV("G711 EOS received, TS=%lld", pInputBuf->nTimeStamp);
            // flush decoder
            FlushAudioDecoder();
            // return the EOS output buffer
            pOutputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
            pOutputBuf->nTimeStamp = pInputBuf->nTimeStamp;
            HandleFillBufferDone(pOutputBuf);
            // return the EOS input buffer
            HandleEmptyBufferDone(pInputBuf);
            LOGV("mNumPendingInput(%d), mNumPendingOutput(%d)", (int)mNumPendingInput, (int)mNumPendingOutput);
            //DumpFTBQ();
            return;
        }

        if (pInputBuf != NULL)
        {
            //Decode the buffer
            OMX_U8 *pOutBuffer = pOutputBuf->pBuffer + pOutputBuf->nOffset;
            OMX_U8 *pInBuffer = pInputBuf->pBuffer + pInputBuf->nOffset;

            if (pInputBuf->nFilledLen > KMaxNumSamplesPerFrame)  //A/MLaw 8byte no linear ,and input buffer size <= 16384
            {
                LOGE("input buffer too large (%lu).", pInputBuf->nFilledLen);
                HandleEmptyBufferDone(pInputBuf);
                QueueOutputBuffer(findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pOutputBuf));
                return ;
            }

            // set output buffer timestamp to be the same as input buffer timestamp
            pOutputBuf->nTimeStamp = pInputBuf ->nTimeStamp;

            OMX_U32 OutputLength = 0;
            OMX_U32 InSize = pInputBuf->nFilledLen;

            if (mIsMLaw)
            {
                DecodeMLaw((OMX_S16 *)pOutBuffer, pInBuffer, InSize);
            }
            else
            {
                DecodeALaw((OMX_S16 *)pOutBuffer, pInBuffer, InSize);
            }

            // Each 8-bit byte is converted into a 16-bit sample.
            pOutputBuf ->nFilledLen = pInputBuf ->nFilledLen * 2;
#ifdef MTK_OMX_G711_DEC_DUMP
            G711Dump(pOutputBuf);
#endif
            LOGV("HandleFillBufferDone pOutputBuf->nTimeStamp=%lld", pOutputBuf->nTimeStamp);

            if (pInputBuf->nFlags & OMX_BUFFERFLAG_EOS) {
                FlushAudioDecoder();
                pOutputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
            }
            HandleFillBufferDone(pOutputBuf);
            HandleEmptyBufferDone(pInputBuf);

        }
    }

    return;
}

void MtkOmxG711Dec::DecodeALaw(OMX_S16 *out, OMX_U8 *in, OMX_U32 inSize)
{
    while (inSize-- > 0)
    {
        OMX_S32 x = *in++;

        OMX_S32 ix = x ^ 0x55;
        ix &= 0x7f;

        OMX_S32 iexp = ix >> 4;
        OMX_S32 mant = ix & 0x0f;

        if (iexp > 0)
        {
            mant += 16;
        }

        mant = (mant << 4) + 8;

        if (iexp > 1)
        {
            mant = mant << (iexp - 1);
        }

        *out++ = (x > 127) ? mant : -mant;
    }


}
void MtkOmxG711Dec::DecodeMLaw(OMX_S16 *out, OMX_U8 *in, OMX_U32 inSize)
{
    while (inSize-- > 0)
    {
        OMX_S32 x = *in++;

        OMX_S32 mantissa = ~x;
        OMX_S32 exponent = (mantissa >> 4) & 7;
        OMX_S32 segment = exponent + 1;
        mantissa &= 0x0f;

        OMX_S32 step = 4 << segment;

        OMX_S32 abs = (0x80l << exponent) + step * mantissa + step / 2 - 4 * 33;

        *out++ = (x < 0x80) ? -abs : abs;
    }

}

void MtkOmxG711Dec::FlushAudioDecoder()
{
    LOGV("MtkOmxG711Dec::FlushAudioDecoder()");

}
void MtkOmxG711Dec::DeinitAudioDecoder()
{
    LOGV("MtkOmxG711Dec::DeinitAudioDecoder()");

}

void  MtkOmxG711Dec::QueueInputBuffer(int index)
{
    LOCK(mEmptyThisBufQLock);

    LOGV("@@ QueueInputBuffer (%d)", index);

#if CPP_STL_SUPPORT
    //mEmptyThisBufQ.push_front(index);
#endif

#if ANDROID
    mEmptyThisBufQ.insertAt(index, 0);
#endif
    SIGNAL(mDecodeSem);
    UNLOCK(mEmptyThisBufQLock);

}
void  MtkOmxG711Dec::QueueOutputBuffer(int index)
{
    LOCK(mFillThisBufQLock);
    LOGV("@@ QueueOutputBuffer (%d)", index);
#if CPP_STL_SUPPORT
    //mFillThisBufQ.push_front(index);
#endif

#if ANDROID
    mFillThisBufQ.insertAt(index, 0);
#endif
    SIGNAL(mDecodeSem);
    UNLOCK(mFillThisBufQLock);

}

// Note: each MTK OMX component must export 'MtkOmxComponentCreate" to MtkOmxCore
extern "C" OMX_COMPONENTTYPE *MtkOmxComponentCreate(OMX_STRING componentName)
{
    // create component instance
    MtkOmxBase *pOmxG711Dec  = new MtkOmxG711Dec();

    if (NULL == pOmxG711Dec)
    {
        LOGE("MtkOmxComponentCreate out of memory!!!");
        return NULL;
    }

    // get OMX component handle
    OMX_COMPONENTTYPE *pHandle = pOmxG711Dec->GetComponentHandle();
    LOGV("MtkOmxComponentCreate mCompHandle(%p)", pOmxG711Dec);

    // init component
    pOmxG711Dec->ComponentInit(pHandle, componentName);

    return pHandle;

}


// ComponentInit ==> InitAudioParams
OMX_BOOL MtkOmxG711Dec::InitAudioParams()
{
    LOGV("MtkOmxAacDec::InitAudioParams(G711)");

    // init input port format
    strncpy((char *)mCompRole, "audio_decoder.g711", OMX_MAX_STRINGNAME_SIZE -1);
    mCompRole[OMX_MAX_STRINGNAME_SIZE -1] = '\0';
    mInputPortFormat.nSize = sizeof(mInputPortFormat);
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eEncoding = OMX_AUDIO_CodingG711;

    // init output port format
    mOutputPortFormat.nSize = sizeof(mOutputPortFormat);
    mOutputPortFormat.nPortIndex = MTK_OMX_OUTPUT_PORT;
    mOutputPortFormat.nIndex = 0;
    mOutputPortFormat.eEncoding = OMX_AUDIO_CodingPCM;

    // init input port definition
    mInputPortDef.nSize = sizeof(mInputPortDef);
    mInputPortDef.nPortIndex                                     = MTK_OMX_INPUT_PORT;
    mInputPortDef.eDir                                           = OMX_DirInput;
    mInputPortDef.eDomain                                        = OMX_PortDomainAudio;
    mInputPortDef.format.audio.pNativeRender             = NULL;
    mInputPortDef.format.audio.cMIMEType                   = (OMX_STRING)"audio/mpeg";
    mInputPortDef.format.audio.bFlagErrorConcealment  = OMX_FALSE;
    mInputPortDef.format.audio.eEncoding    = OMX_AUDIO_CodingG711;

    mInputPortDef.nBufferCountActual                         = MTK_OMX_NUMBER_INPUT_BUFFER_G711;
    mInputPortDef.nBufferCountMin                            = 1;
    mInputPortDef.nBufferSize                                 = MTK_OMX_INPUT_BUFFER_SIZE_G711;
    mInputPortDef.bEnabled                                     = OMX_TRUE;
    mInputPortDef.bPopulated                                   = OMX_FALSE;

    //<---Donglei for conformance test
    mInputPortDef.bBuffersContiguous = OMX_FALSE;
    mInputPortDef.nBufferAlignment   = OMX_FALSE;
    //--->

    // init output port definition
    mOutputPortDef.nSize = sizeof(mOutputPortDef);
    mOutputPortDef.nPortIndex = MTK_OMX_OUTPUT_PORT;
    mOutputPortDef.eDomain = OMX_PortDomainAudio;
    mOutputPortDef.format.audio.cMIMEType = (OMX_STRING)"raw";
    mOutputPortDef.format.audio.pNativeRender = 0;
    mOutputPortDef.format.audio.bFlagErrorConcealment = OMX_FALSE;
    mOutputPortDef.format.audio.eEncoding = OMX_AUDIO_CodingPCM;
    mOutputPortDef.eDir = OMX_DirOutput;

    mOutputPortDef.nBufferCountActual = MTK_OMX_NUMBER_OUTPUT_BUFFER_G711;
    mOutputPortDef.nBufferCountMin = 1;
    mOutputPortDef.nBufferSize = MTK_OMX_OUTPUT_BUFFER_SIZE_G711;
    mOutputPortDef.bEnabled = OMX_TRUE;
    mOutputPortDef.bPopulated = OMX_FALSE;

    //<---Donglei for conformance test
    mOutputPortDef.bBuffersContiguous = OMX_FALSE;
    mOutputPortDef.nBufferAlignment   = OMX_FALSE;
    //--->


    // init input g711 format
    mInputG711Mode.nSize = sizeof(mInputG711Mode);
    mInputG711Mode.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputG711Mode.nChannels = 2;
    mInputG711Mode.eNumData = OMX_NumericalDataSigned;
    mInputG711Mode.bInterleaved = OMX_TRUE;
    mInputG711Mode.nBitPerSample = 16;
    mInputG711Mode.nSamplingRate = 44100;
    mInputG711Mode.ePCMMode = OMX_AUDIO_PCMModeALaw;//or OMX_AUDIO_PCMModeMULaw
    mInputG711Mode.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
    mInputG711Mode.eChannelMapping[1] = OMX_AUDIO_ChannelRF;


    // init output pcm format
    mOutputPcmMode.nSize = sizeof(mOutputPcmMode);
    mOutputPcmMode.nPortIndex = MTK_OMX_OUTPUT_PORT;
    mOutputPcmMode.nChannels = 2;
    mOutputPcmMode.eNumData = OMX_NumericalDataSigned;
    mOutputPcmMode.bInterleaved = OMX_TRUE;
    mOutputPcmMode.nBitPerSample = 16;
    mOutputPcmMode.nSamplingRate = 44100;
    mOutputPcmMode.ePCMMode = OMX_AUDIO_PCMModeLinear;
    mOutputPcmMode.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
    mOutputPcmMode.eChannelMapping[1] = OMX_AUDIO_ChannelRF;

    // allocate input buffer headers address array
    mInputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE *)*mInputPortDef.nBufferCountActual);
    MTK_OMX_MEMSET(mInputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE *)*mInputPortDef.nBufferCountActual);

    // allocate output buffer headers address array
    mOutputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE *)*mOutputPortDef.nBufferCountActual);
    MTK_OMX_MEMSET(mOutputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE *)*mOutputPortDef.nBufferCountActual);



    return OMX_TRUE;

}


void MtkOmxG711Dec::G711Dump(OMX_BUFFERHEADERTYPE *pBuf)
{
    if (pBuf->nFilledLen != 0)
    {
        FILE *fp = fopen("/sdcard/g711dec.pcm", "ab");

        if (fp)
        {
            fwrite(pBuf->pBuffer, 1, pBuf->nFilledLen, fp);
            fclose(fp);
        }
    }
}






