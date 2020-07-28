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
 *   MtkOmxApeDec.cpp
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX APE Decoder component
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
#include "MtkOmxApeDec.h"
#include <cutils/properties.h>
//#include <binder/Parcel.h>
//#include <binder/IMemory.h>
//#include <OMXNodeInstance.cpp>

#undef LOG_TAG
#define LOG_TAG "MtkOmxApeDec"

//#define MTK_OMX_APE_DEC_DUMP

#define MTK_OMX_APE_DECODER "OMX.MTK.AUDIO.DECODER.APE"

#define APE_ERR_EOS -1
#define APE_ERR_CRC -2

#define LOGD SLOGD
#define LOGE SLOGE
#define LOGV SLOGV

MtkOmxApeDec::MtkOmxApeDec()
{
    SLOGD("MtkOmxApeDec::MtkOmxApeDec(APE) this= %p", this);
    mApeInitFlag = OMX_FALSE;//init flag
    mInputCurrLength = 0; //Input Buffer 's current length
    mFrameCount = 0;//frame num

    mNewInBufferRequired = OMX_TRUE;
    mNewOutBufRequired = OMX_TRUE;

    mOutputFrameLength = 0;
    mSeekTimeUs = 0;
    mSeekRampup = 0;
    mSeekMute = 0;
    mCurrentTime = 0;

    mCRCError = OMX_FALSE;
    mSeekEnable = false;
    mStarted = false;
    mInputApeParam.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputApeParam.channels = 2;
    mInputApeParam.SampleRate = 48000;

    mInputApeParam.fileversion = 0;
    mInputApeParam.compressiontype = 0;
    mInputApeParam.blocksperframe = 0;
    mInputApeParam.finalframeblocks = 0;
    mInputApeParam.totalframes = 0;
    mInputApeParam.bps = 16;
    mInputApeParam.SourceBufferSize = 0;
    mInputApeParam.Bitrate = 0;

    mInputApeParam.seekfrm = 0;
    mInputApeParam.seekbyte = 0;
    in_size = 0;
    out_size = 0;
    //stand alone process+
    ALOGD("new BpMtkCodec");
    pMtkCodec = new BpMtkCodecService();
    if (pMtkCodec == NULL)
    {
        LOGE("error, can't new BpMtkCodec");
    }
    else
    {
        mLastTimeStamp = 0;
        apeCreateParams param;
        //const char *pMIME="audio/ape";
        //CODECTYPE ptype = DECODE;
        //param.writeCString(pMIME);
        //param.writeInt32(ptype);
        status_t iresult = pMtkCodec->Create(param);
        ALOGD("create:%d",iresult);
        if (iresult == OK)
        {
            in_size = param.in_size;
            out_size = param.out_size;
            LOGD("ctor:in_size:%d,out_size:%d",in_size,out_size);
        }
        else
        {
            LOGE("MtkCodec service create error");    
            
        }
    }
    //mStagefrightHandle = NULL;
#if 0
	//dlopen so
	mStagefrightHandle = dlopen("libstagefright_omx.so",RTLD_NOW);
	if(mStagefrightHandle == NULL)
	{
        dlclose(mStagefrightHandle);
	    mStagefrightHandle = NULL;
		ALOGD("connect OMXNodeInstance fail");
	}
	else
	{
		ALOGD("connect OMXNodeInstance success");
        //get function handle
        //extern "C" style
        getIMemoryFromBufferMeta = (sp<IMemory>(*)(OMX_PTR))dlsym(mStagefrightHandle,"getIMemoryFromBufferMeta");

        if(getIMemoryFromBufferMeta == NULL)
        {
           //not extern "C" style
           getIMemoryFromBufferMeta = (sp<IMemory>(*)(OMX_PTR))dlsym(mStagefrightHandle,"_ZN7android24getIMemoryFromBufferMetaEPv");
        }
        if(getIMemoryFromBufferMeta != NULL)
        {
            ALOGD("getIMemoryFromBufferMeta success");
        }
        else
        {
            ALOGD("getIMemoryFromBufferMeta fail err = %s",dlerror());
        }
	}
#endif
    //stand alone process-
}

MtkOmxApeDec::~MtkOmxApeDec()
{
    ALOGD("~dtor+ this= %p", this);
    mApeInitFlag = OMX_FALSE;
    mSeekEnable = false;

	//close handle of libstagefright_omx.so
	//dlclose(mStagefrightHandle);
	//mStagefrightHandle = NULL;
	ALOGD("close handle");

    if (pMtkCodec != NULL)
    {
        //Parcel param;
        //param.writeInt32(pMtkCodec->GetCodecId());
        ALOGD("call MtkCodec destroy+");
        pMtkCodec->Destroy();
        ALOGD("call MtkCodec destroy-");
    }
//stand alone process-
    LOGD("~dtor- this= %p", this);
}

void MtkOmxApeDec::DecodeAudio(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf)
{

    if (pInputBuf == NULL  || pOutputBuf==NULL)
    {
        fn_ErrHandle("input or output is NULL", OMX_ErrorBadParameter, 0, pInputBuf, pOutputBuf, true);
        return;
    }

    if (pInputBuf->nFlags & OMX_BUFFERFLAG_CODECCONFIG)
    {
        if (mApeInitFlag == OMX_FALSE)  // just  init decoder only once
        {
            if (OmxApeDecInit(pInputBuf))
            {
                mApeInitFlag = OMX_TRUE;
                mOutputPcmMode.nSamplingRate = mInputApeParam.SampleRate;
                mOutputPcmMode.nChannels    =   mInputApeParam.channels;
            }
            else
            {
                fn_ErrHandle("DecodeAudio Init Failure!", OMX_ErrorBadParameter, 0,
                    pInputBuf, pOutputBuf, true);
                return;
            }
        }
        //empty csd buffer
        HandleEmptyBufferDone(pInputBuf);
        QueueOutputBuffer(findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pOutputBuf));
        return;
    }
    else
    {
        // If EOS flag has come from the client & there are no more
        // input buffers to decode, send the callback to the client
        if (mInputApeParam.seekfrm != 0)
        {
            mSeekEnable = true;
        }
       
        LOGV("decode+");
        if (mSignalledError == OMX_TRUE)
        {
            QueueInputBuffer(findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pInputBuf));
            QueueOutputBuffer(findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pOutputBuf));
            LOGD("Error exit mNumPendingInput(%d), mNumPendingOutput(%d)", (int)mNumPendingInput, (int)mNumPendingOutput);
            return;
        }

        if (mSeekEnable == true)
        {
            int32_t newframe = 255, seekbyte = 255;
            mSeekEnable = false;
            newframe = mInputApeParam.seekfrm;
            seekbyte = mInputApeParam.seekbyte;
            pOutputBuf->nTimeStamp = pInputBuf->nTimeStamp;
            mSeekTimeUs = 0;
            mCurrentTime = pInputBuf->nTimeStamp;
            LOGD("reset seekbyte=%d, newfrm=%d", seekbyte, newframe);
            //ape_decoder_reset(apeHandle, seekbyte, newframe);
            //Parcel param;
            //param.writeInt32(pMtkCodec->GetCodecId());
            //param.writeInt32(seekbyte);
            //param.writeInt32(newframe);
            apeResetParams param;
            param.seekbyte = seekbyte;
            param.newframe = newframe;
            pMtkCodec->Reset(param);
            mSeekMute = 0;///ape_param.blocksperframe* 2 * ape_param.channels;
            mSeekRampup = 0;
            mInputApeParam.seekfrm = 0;
            mInputApeParam.seekbyte = 0;
        }

        if (pInputBuf->nFlags & OMX_BUFFERFLAG_EOS)
        {
            LOGD("APE EOS received, TS=%lld", pInputBuf->nTimeStamp);
        }
        
        //input param order:codecid, input buf offset, input flag, input buf, output buf
        //Parcel data, reply;
        //int pcodecid = pMtkCodec->GetCodecId();
        //int inputOffset = pInputBuf->nOffset;
        //OMX_U32 uInputFlags = pInputBuf->nFlags;
        //data.writeInt32(pcodecid);
        //data.writeInt32(inputOffset);
        //data.writeInt32(uInputFlags);
        //LOGV("pcodecid:%d,inputOffset:%d,uInputFlags:%d",pcodecid, inputOffset, uInputFlags);
		
        //sp<IMemory> inputmem = getIMemoryFromBufferMeta(pInputBuf->pAppPrivate);
        //data.writeStrongBinder(IInterface::asBinder(inputmem));
        //sp<IMemory> outputmem = getIMemoryFromBufferMeta(pOutputBuf->pAppPrivate);
        //data.writeStrongBinder(IInterface::asBinder(outputmem));

        apeDecParams params_in;
        params_in.inOffset = pInputBuf->nOffset;
        params_in.inFlag = pInputBuf->nFlags;
        apeDecRetParams params_out;

        /*****  copy input data to from input buffer to input hidl_mem   *****/
        mapped_memory_input->update();
        uint8_t *data_input= static_cast<uint8_t*>(static_cast<void*>(mapped_memory_input->getPointer()));
        memcpy(data_input,pInputBuf->pBuffer,pInputBuf->nFilledLen);
        mapped_memory_input->commit();
        params_in.in_mem = m_hidl_memory_in;

        /*****  Decode   *****/
        status_t iresult = pMtkCodec->DoCodec(params_in, params_out,(uint8_t*)pOutputBuf->pBuffer);

        if (iresult != OK)
        {
            char pmsg[64]="";
            sprintf(pmsg, "BnCodec error, result:%x", iresult);
            fn_ErrHandle(pmsg, OMX_ErrorNotImplemented, 0, pInputBuf, pOutputBuf, false);
            return;
        }

//output param order: consumeBS, outputFrameSize,mNewInputBufferRequired,mNewOutBufRequired
        int32_t consumeBS = APE_ERR_EOS;
        consumeBS = params_out.consumeBS;//reply.readInt32();
        int32_t OutFrameSize = params_out.outputFrameSize;//reply.readInt32();
        int inputBufUsedLen = params_out.inputBufUsedLen;//reply.readInt32();
        mNewInBufferRequired = (OMX_BOOL)params_out.newInBufRequired;//(OMX_BOOL)reply.readInt32();
        mNewOutBufRequired = (OMX_BOOL)params_out.newOutBufRequired;//(OMX_BOOL)reply.readInt32();
        
        LOGV("consumeBS:%d,inputBufUsedLen:%d,OutFrameSize:%d,mNewInBufferRequired:%d,mNewOutBufRequired:%d",
            consumeBS,inputBufUsedLen,OutFrameSize,mNewInBufferRequired,mNewOutBufRequired);
           
        if (consumeBS == APE_ERR_CRC)
        {
            FlushAudioDecoder();
            char psMsg[128]="";
            sprintf(psMsg, "CRC mNumPendingInput(%d), mNumPendingOutput(%d)", (int)mNumPendingInput, (int)mNumPendingOutput);
            fn_ErrHandle(psMsg, OMX_ErrorStreamCorrupt, OMX_AUDIO_CodingAPE, pInputBuf, pOutputBuf, false);
            return;
        }
        else if (consumeBS == APE_ERR_EOS)
        {
            LOGD("Decode Frame ERROR EOS");
            // This is recoverable, just ignore the current frame and play silence instead.
            FlushAudioDecoder();
            // return the EOS output buffer
            pOutputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
            pOutputBuf->nTimeStamp = mCurrentTime;
            pInputBuf->nTimeStamp = mCurrentTime;
            pOutputBuf->nFilledLen = OutFrameSize;
            HandleFillBufferDone(pOutputBuf);
            HandleEmptyBufferDone(pInputBuf);
            //flush input port,avoid to the last Input buffer eos cause decoder error
            FlushInputPort();
            LOGD("File Send Eos mNumPendingInput(%d), mNumPendingOutput(%d)", (int)mNumPendingInput, (int)mNumPendingOutput);
            return;
        }
        else
        {
            pOutputBuf->nTimeStamp = mCurrentTime;
            if (mOutputPcmMode.nBitPerSample == 32)
                mCurrentTime += (OutFrameSize * 1000000LL) / (4 * mInputApeParam.channels* mInputApeParam.SampleRate);
            else
                mCurrentTime += (OutFrameSize * 1000000) / (2 * mInputApeParam.channels * mInputApeParam.SampleRate);
        }
        pOutputBuf->nFilledLen = OutFrameSize;
        pInputBuf->nOffset += inputBufUsedLen;
        if (mNewOutBufRequired)
        {
            LOGV("new out buf req");
            HandleFillBufferDone(pOutputBuf);
        }
        else
        {
            LOGV("que out buf");
            QueueOutputBuffer(findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pOutputBuf));
        }

        if (mNewInBufferRequired)
        {
            LOGV("new in buf req");
            HandleEmptyBufferDone(pInputBuf);
        }
        else
        {
            LOGV("que in buf");
            QueueInputBuffer(findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pInputBuf));
        }
        LOGV("decode-");
    }
    return ;
}

void MtkOmxApeDec::fn_ErrHandle(const char *pErrMsg, int pErrType, int pCodingType,
                      OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf, bool isBufQue)
{
    LOGE("%s",pErrMsg);
    mSignalledError = OMX_TRUE;
    mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle, mAppData, OMX_EventError, pErrType, pCodingType, NULL);
    if (isBufQue)
    {
        QueueInputBuffer(findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pInputBuf));
        QueueOutputBuffer(findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pOutputBuf));       
    }
    else
    {
        HandleEmptyBufferDone(pInputBuf);
        HandleFillBufferDone(pOutputBuf);
    }
}

void MtkOmxApeDec::FlushAudioDecoder()
{
    mSeekEnable = true;
    mSignalledError = OMX_FALSE;
    SLOGD("MtkOmxApeDec::FlushAudioDecoder()");
}
//Dump data send to AudioPlayer 's Buffer
void MtkOmxApeDec::APEDump(OMX_BUFFERHEADERTYPE *pBuf)
{
    //SXLOGE("APEDump----mFrameCount = %ld",mFrameCount);
    if (pBuf->nFilledLen != 0)
    {
        FILE *fp = fopen("/sdcard/apedec.pcm", "ab");

        if (fp)
        {
            fwrite(pBuf->pBuffer, 1, pBuf->nFilledLen, fp);
            fclose(fp);
        }
    }
}


void MtkOmxApeDec::DeinitAudioDecoder()
{
    SLOGV("+DeinitAudioDecoder (APE)");

    mNewOutBufRequired = OMX_FALSE;
    mSeekEnable = false;
    SLOGV("-DeinitAudioDecoder (APE)");
}

OMX_BOOL MtkOmxApeDec::OmxApeDecInit(OMX_BUFFERHEADERTYPE * pInputBuf)
{
    OMX_U8* pInBuffer = pInputBuf->pBuffer + pInputBuf->nOffset;
    mInputApeParam.fileversion = (OMX_S16)*((OMX_S16 *) (pInBuffer + 16));
    mInputApeParam.compressiontype = (OMX_U16)*((OMX_U16 *) (pInBuffer + 64));
    mInputApeParam.blocksperframe = (OMX_U32)*((OMX_U32 *) (pInBuffer + 68));
    mInputApeParam.finalframeblocks = (OMX_U32)*((OMX_U32 *) (pInBuffer + 72));
    mInputApeParam.totalframes = (OMX_U32)*((OMX_U32 *) (pInBuffer + 76));

    if (mInputApeParam.fileversion < 3950
            || mInputApeParam.fileversion > 4120
            || mInputApeParam.compressiontype > 4000
            || mInputApeParam.blocksperframe <= 0
            || mInputApeParam.totalframes <= 0
            || mInputApeParam.bps <= 0
            || mInputApeParam.SampleRate <= 0
            || mInputApeParam.SampleRate > 192000
            || mInputApeParam.channels <= 0
            || mInputApeParam.channels > 2)
    {
        SLOGD("APE header error: fileversion:%d,compressiontype:%d,blocksperframe %d,totalframes %d, bps %d,samplerate %d,channels:%d",
               mInputApeParam.fileversion,
               mInputApeParam.compressiontype, 
               mInputApeParam.blocksperframe,
               mInputApeParam.totalframes,
               mInputApeParam.bps,
               mInputApeParam.SampleRate,
               mInputApeParam.channels);
        return OMX_FALSE;
    }

//order:mimetype, opr type, blocksperframe, bps, channels, compressiontype, fileversion, finalframeblocks, totlframes
    //Parcel param;
    //param.writeInt32(pMtkCodec->GetCodecId());
    //param.writeInt32(mInputApeParam.blocksperframe);
    //param.writeInt32(mInputApeParam.bps);
    //param.writeInt32(mInputApeParam.channels);
    //param.writeInt32(mInputApeParam.compressiontype);
    //param.writeInt32(mInputApeParam.fileversion);
    //param.writeInt32(mInputApeParam.finalframeblocks);
    //param.writeInt32(mInputApeParam.totalframes);
    apeInitParams param;
    param.blocksperframe   = mInputApeParam.blocksperframe;
    param.bps              = mInputApeParam.bps;
    param.channels         = mInputApeParam.channels;
    param.compressiontype  = mInputApeParam.compressiontype;
    param.fileversion      = mInputApeParam.fileversion;
    param.finalframeblocks = mInputApeParam.finalframeblocks;
    param.totalframes      = mInputApeParam.totalframes;
    LOGD("call BpMtkCodec::Init,blocksperframe:%d,bps:%d,channels:%d,compressiontype:%d,fileversion:%d,finalframeblocks:%d,totalframes:%d",
        mInputApeParam.blocksperframe,mInputApeParam.bps,mInputApeParam.channels,mInputApeParam.compressiontype,
        mInputApeParam.fileversion,mInputApeParam.finalframeblocks,mInputApeParam.totalframes);
    status_t iresult = pMtkCodec->Init(param);
    if (iresult != OK)
        return OMX_FALSE;
    else
    {
        mNewOutBufRequired = OMX_TRUE;
        mSeekEnable = false;
        mCRCError = OMX_FALSE;
        LOGV("-MtkOmxApeDecInit sucess!");
        return OMX_TRUE;
    }

}

void  MtkOmxApeDec::QueueInputBuffer(int index)
{
    LOCK(mEmptyThisBufQLock);

    SLOGV("@@ QueueInputBuffer (%d)", index);

#if ANDROID
    mEmptyThisBufQ.insertAt(index, 0);
#endif

    UNLOCK(mEmptyThisBufQLock);
}

void  MtkOmxApeDec::QueueOutputBuffer(int index)
{
    LOCK(mFillThisBufQLock);

    SLOGV("@@ QueueOutputBuffer (%d)", index);

#if ANDROID
    mFillThisBufQ.insertAt(index, 0);
#endif
    SIGNAL(mDecodeSem);
    UNLOCK(mFillThisBufQLock);
}

void MtkOmxApeDec::RampUp(int16_t *aRampBuff, uint32_t aHalfRampSample)
{
    SLOGV("RampUp 16bit aHalfRampSample=%d", aHalfRampSample);
    static const unsigned int RAMP_SHIFT_BIT =  16;
    static const unsigned int RAMP_FRACTION = (65536 / aHalfRampSample);
    unsigned int mRamp_Fraction = 0, i = 0;

    for (i = 0; i < aHalfRampSample; i++)
    {
        aRampBuff[i << 1] = aRampBuff[i << 1] * mRamp_Fraction >> RAMP_SHIFT_BIT;
        aRampBuff[(i << 1) + 1] = aRampBuff[(i << 1) + 1] * mRamp_Fraction >> RAMP_SHIFT_BIT;
        mRamp_Fraction +=  RAMP_FRACTION;
    }
}

void MtkOmxApeDec::RampUp(int8_t *aRampBuff, uint32_t aHalfRampSample)
{
    SLOGV("RampUp 8bit aHalfRampSample=%d", aHalfRampSample);
    static const unsigned int RAMP_SHIFT_BIT =  8;
    static const unsigned int RAMP_FRACTION = (256 / aHalfRampSample);
    unsigned int mRamp_Fraction = 0, i = 0;

    for (i = 0; i < aHalfRampSample; i++)
    {
        aRampBuff[i << 1] = aRampBuff[i << 1] * mRamp_Fraction >> RAMP_SHIFT_BIT;
        aRampBuff[(i << 1) + 1] = aRampBuff[(i << 1) + 1] * mRamp_Fraction >> RAMP_SHIFT_BIT;
        mRamp_Fraction +=  RAMP_FRACTION;
    }
}


// ComponentInit ==> InitAudioParams
OMX_BOOL MtkOmxApeDec::InitAudioParams()
{
    SLOGD("MtkOmxApeDec::InitAudioParams(APE)");
    // init input port format
    strncpy((char *)mCompRole, "audio_decoder.ape", OMX_MAX_STRINGNAME_SIZE -1);
    mCompRole[OMX_MAX_STRINGNAME_SIZE -1] = '\0';
    mInputPortFormat.nPortIndex = MTK_OMX_INPUT_PORT;
    mInputPortFormat.nIndex = 0;
    mInputPortFormat.eEncoding = OMX_AUDIO_CodingAPE;

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
    mInputPortDef.format.audio.eEncoding    = OMX_AUDIO_CodingAPE;

    mInputPortDef.nBufferCountActual                         = MTK_OMX_NUMBER_INPUT_BUFFER_APE;
    mInputPortDef.nBufferCountMin                             = 2;
    mInputPortDef.nBufferSize                                    = in_size * 7;

    if (mInputPortDef.nBufferSize > 12288)
    {
        mInputPortDef.nBufferSize = 12288;    ///12k
    }

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

    mOutputPortDef.nBufferCountActual = MTK_OMX_NUMBER_OUTPUT_BUFFER_APE;
    mOutputPortDef.nBufferCountMin = 1;
    ALOGD("InitAudioParams:out_size:%d",out_size);
    mOutputPortDef.nBufferSize = out_size;
    mOutputPortDef.bEnabled = OMX_TRUE;
    mOutputPortDef.bPopulated = OMX_FALSE;

    //<---Donglei for conformance test
    mOutputPortDef.bBuffersContiguous = OMX_FALSE;
    mOutputPortDef.nBufferAlignment   = OMX_FALSE;
    //--->


    // init output pcm format
    mOutputPcmMode.nPortIndex = MTK_OMX_OUTPUT_PORT;
    mOutputPcmMode.nChannels = 2;
    mOutputPcmMode.eNumData = OMX_NumericalDataSigned;
    mOutputPcmMode.bInterleaved = OMX_TRUE;
    mOutputPcmMode.nBitPerSample = 16;
    mOutputPcmMode.nSamplingRate = 48000;//44100;by Changqing
    mOutputPcmMode.ePCMMode = OMX_AUDIO_PCMModeLinear;
    mOutputPcmMode.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
    mOutputPcmMode.eChannelMapping[1] = OMX_AUDIO_ChannelRF;

    // allocate input buffer headers address array
    mInputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE *)*mInputPortDef.nBufferCountActual);
    MTK_OMX_MEMSET(mInputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE *)*mInputPortDef.nBufferCountActual);

    // allocate output buffer headers address array
    mOutputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE *)*mOutputPortDef.nBufferCountActual);
    MTK_OMX_MEMSET(mOutputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE *)*mOutputPortDef.nBufferCountActual);

    //allocate hidl_memory for input port
    do {
        sp<IAllocator> ashmem = IAllocator::getService("ashmem");
        if (ashmem == NULL) {
            ALOGD("[MtkOmxApeDec] failed to get IAllocator HW service");
            break;
        }
        ashmem->allocate(mInputPortDef.nBufferSize,
            [&](bool success, const hidl_memory &memory_in) {
            if (success == true) {
                m_hidl_memory_in = memory_in;
                mapped_memory_input = mapMemory(m_hidl_memory_in);
                ALOGD("[MtkOmxApeDec] Get HIDL input memory success.");
            }
       });
    }while (0);

    return OMX_TRUE;
}


// Note: each MTK OMX component must export 'MtkOmxComponentCreate" to MtkOmxCore
extern "C" OMX_COMPONENTTYPE *MtkOmxComponentCreate(OMX_STRING componentName)
{
    ALOGD("APE component create");
    // create component instance
    MtkOmxBase *pOmxApeDec  = new MtkOmxApeDec();

    if (NULL == pOmxApeDec)
    {
        SLOGE("MtkOmxComponentCreate out of memory!!!");
        return NULL;
    }
    else
    {
        if (!((MtkOmxApeDec*)pOmxApeDec)->IsMtkCodecSrvReady())
        {
            LOGE("MtkCodecService not run");
            delete pOmxApeDec;
            pOmxApeDec = NULL;
            return NULL;
        }
    }
    // get OMX component handle
    OMX_COMPONENTTYPE *pHandle = pOmxApeDec->GetComponentHandle();
    SLOGV("MtkOmxComponentCreate mCompHandle(%p)", pOmxApeDec);

    // init component
    pOmxApeDec->ComponentInit(pHandle, componentName);

    return pHandle;
}

bool MtkOmxApeDec::IsMtkCodecSrvReady()
{
    bool blnResult = false;
    if (pMtkCodec != NULL)
    {
        blnResult = pMtkCodec->IsSrvReady();
    }
    return blnResult;
}

OMX_ERRORTYPE MtkOmxApeDec::SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_INDEXTYPE nParamIndex,
        OMX_IN OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if(NULL == pCompParam) {
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    if(mState == OMX_StateInvalid) {
        err = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    switch(nParamIndex) {
    case OMX_IndexParamAudioApe: {
        LOGV("SetParameter OMX_IndexParamAudioApe !!!");
        OMX_AUDIO_PARAM_APETYPE *pAudioApe = (OMX_AUDIO_PARAM_APETYPE *)pCompParam;
        if (pAudioApe->nPortIndex == mInputApeParam.nPortIndex)
        {
            memcpy(&mInputApeParam, pCompParam, sizeof(OMX_AUDIO_PARAM_APETYPE));
            LOGV("setApeAudioParameter, channels(%d), sampleRate(%d), bps(%d)",
                mInputApeParam.channels, mInputApeParam.SampleRate,
                mInputApeParam.bps);
            mOutputPcmMode.nSamplingRate = mInputApeParam.SampleRate;
            mOutputPcmMode.nChannels    =   mInputApeParam.channels;
            if (mInputApeParam.bps == 24) {
                mOutputPcmMode.nBitPerSample = 32u;
                mOutputPcmMode.eNumData = OMX_NumericalDataFloat;
                mOutputPortDef.nBufferSize = mOutputPortDef.nBufferSize * sizeof (float) / 3;
                LOGD("24bit--update ouput buffer size: %lld", (long long)mOutputPortDef.nBufferSize);
            }
        }
        else
        {
            err = OMX_ErrorUnsupportedIndex;
        }
        break;
        }

    default: {
        err = MtkOmxAudioDecBase::SetParameter(hComponent, nParamIndex, pCompParam);
        break;
        }
    }

EXIT:
    LOGV("setParam, err = %ld", (long) err);
    return err;
}
